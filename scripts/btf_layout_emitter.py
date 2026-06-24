"""
btf_layout_emitter.py — Optimized C Array Emitter
===================================================
A standalone script that takes a BTreeFy XML model, computes the optimal
node layout using SA (α=0.995), and re-emits the btf_node[] C array with
nodes in the optimized slot order.

This is a *separate script* — it does NOT modify btf_groot_parser.py.
It imports BTFNode from the parser as a read-only library.

Usage
-----
  python3 scripts/btf_layout_emitter.py \\
      -m models/porta_automatica.xml \\
      -tn main_new \\
      --source-output-dir src/generated \\
      --include-output-dir include/generated

Output files (same names as btf_groot_parser.py, drop-in replacement):
  btf_nodes_generated.c
  btf_action_functions_generated.h

Algorithm
---------
  1. Parse XML with BTFNode (from btf_groot_parser.py) — DFS pre-order.
  2. Build BTGraph from the parsed LCRS arrays.
  3. Run SA (Fiedler init, α=0.995) to find optimal permutation y*.
  4. Re-index: new_index[node] = y*[node]  (slot the node lands in).
  5. Emit C array with nodes in slot order, with parent/child/sibling
     indices remapped through y*.
"""

import sys
import os
import argparse

# ── Path bootstrap ─────────────────────────────────────────────────────────────
_SCRIPTS = os.path.dirname(os.path.abspath(__file__))
_REPO    = os.path.dirname(_SCRIPTS)
for p in [_SCRIPTS, _REPO]:
    if p not in sys.path:
        sys.path.insert(0, p)

import importlib.util
import xml.etree.ElementTree as ET

from minla.bt_graph   import BTGraph, BTF_NULL_NODE, from_lcrs_arrays
from minla.sa_solver  import solve as sa_solve
from minla.cost       import minla_cost

# ── Re-use parser templates verbatim ──────────────────────────────────────────
ACTION_FUNCTION_DATA_FILE_NO_EXT = "btf_action_functions_generated"
NODES_DATA_FILE                  = "btf_nodes_generated.c"

BTF_NODES_DATA_FILE_INCLUDE = f"""\
#include "btreefy/btreefy_policies.h"
#include "btreefy/btreefy_always_actions.h"
#include "{ACTION_FUNCTION_DATA_FILE_NO_EXT}.h"

"""

BTF_ACTION_FN_FILE_INCLUDE = """\
#include "btreefy/btreefy_objs.h"

"""

BTF_NODE_STRUCT_CTYPE   = "struct btf_node"
BTF_NODES_CARRAY_NAME   = "nodes"
BT_NODES_CARRAY_DECL_START = f"{BTF_NODE_STRUCT_CTYPE} {BTF_NODES_CARRAY_NAME}[] = {{"
BT_NODES_CARRAY_DECL_END   = "};"
BT_NODES_SIZE_DECL = (
    f"size_t {BTF_NODES_CARRAY_NAME}_size = "
    f"sizeof({BTF_NODES_CARRAY_NAME})/sizeof({BTF_NODES_CARRAY_NAME}[0]);"
)
BTF_ACTION_FUNCTION_NAME_CTYPE = "enum btf_node_status {name}(struct btf_tree *tree)"

BTF_CTYPE_POLICY_FUNCTIONS_MAP = {
    "Sequence":     "btf_sequence_policy_fn",
    "Fallback":     "btf_fallback_policy_fn",
    "ForceSuccess": "btf_success_policy_fn",
    "ForceFailure": "btf_fail_policy_fn",
}
BTF_CTYPE_ALWAYS_ACTION_FUNCTIONS_MAP = {
    "AlwaysSuccess": "btf_always_success_action",
    "AlwaysFailure": "btf_always_failure_action",
}

NODE_TEMPLATE = """\

    [{index}] = {{.status   = BTF_UNDEF_STATUS,
            .parent   = {parent},
            .child    =  {child},
            .sibling = {sibling},
            .action   = {action},
            .control  = {control},
            .name     = {name}}},
"""


# ── Parsing (reimplemented without BTFNode class state mutation) ───────────────

def _parse_tree(bt_root):
    """
    Walk the XML subtree and build plain lists:
      tags[i], attribs[i], parents[i], children[i], siblings[i]
    in DFS pre-order (same traversal as btf_groot_parser.py).
    Returns (tags, attribs, parents, children, siblings).
    """
    tags    = []
    attribs = []
    parents_  = []
    children_ = []
    siblings_ = []

    # Assign indices in DFS pre-order
    index_map = {}   # xml.Element → int index

    def _dfs(elem, parent_elem, sibling_elem):
        idx = len(tags)
        index_map[id(elem)] = idx
        tags.append(elem.tag)
        attribs.append(dict(elem.attrib))
        parents_.append(index_map[id(parent_elem)] if parent_elem is not None else BTF_NULL_NODE)
        children_.append(BTF_NULL_NODE)   # filled below
        siblings_.append(BTF_NULL_NODE)   # filled below

        child_elems = list(elem)
        for k, child in enumerate(child_elems):
            sib = child_elems[k + 1] if k + 1 < len(child_elems) else None
            _dfs(child, elem, sib)

        # Fill child/sibling of current node after recursion
        if child_elems:
            children_[idx] = index_map[id(child_elems[0])]
        if sibling_elem is not None:
            # sibling is filled by parent after sibling's index is known
            pass

    # Build sibling links via a second pass
    # Simpler: do the traversal iteratively to build sibling links directly
    order = []  # (elem, parent_elem, sibling_elem)

    def _collect(elem, parent, sibling):
        order.append((elem, parent, sibling))
        ch = list(elem)
        for k, c in enumerate(ch):
            _collect(c, elem, ch[k+1] if k+1 < len(ch) else None)

    _collect(bt_root, None, None)

    for i, (elem, parent, sibling) in enumerate(order):
        index_map[id(elem)] = i
        tags.append(elem.tag)
        attribs.append(dict(elem.attrib))

    # Reset and rebuild with known indices
    n = len(tags)
    parents_  = [BTF_NULL_NODE] * n
    children_ = [BTF_NULL_NODE] * n
    siblings_ = [BTF_NULL_NODE] * n

    for elem, parent, sibling in order:
        idx = index_map[id(elem)]
        if parent is not None:
            parents_[idx] = index_map[id(parent)]
        ch = list(elem)
        if ch:
            children_[idx] = index_map[id(ch[0])]
        if sibling is not None:
            siblings_[idx] = index_map[id(sibling)]

    return tags, attribs, parents_, children_, siblings_


# ── Optimized emitter ─────────────────────────────────────────────────────────

def emit(args):
    print(f"[btf_layout_emitter] Loading: {args.model}  tree={args.treename}")

    tree = ET.parse(args.model)
    root = tree.getroot()
    bt_root = root.find(f"BehaviorTree[@ID='{args.treename}']")[0]

    tags, attribs, parents, children, siblings = _parse_tree(bt_root)
    n = len(tags)

    # Build BTGraph for SA
    labels = [attribs[i].get("name", tags[i]) for i in range(n)]
    graph  = from_lcrs_arrays(
        parents=parents, children=children, siblings=siblings, labels=labels
    )

    # Baseline (DFS pre-order) cost
    dfs_perm  = list(range(n))
    dfs_cost  = minla_cost(graph, dfs_perm)

    # SA optimization
    print(f"  n={n}  |E|={len(graph.edges)}  DFS cost={dfs_cost:.0f}")
    print(f"  Running SA (α={args.alpha}) …")
    result    = sa_solve(graph, alpha=args.alpha, seed=args.seed)
    sa_cost   = result.cost
    perm      = result.permutation    # perm[node] = new_slot

    improvement = (dfs_cost - sa_cost) / dfs_cost * 100
    print(f"  SA cost={sa_cost:.0f}  improvement={improvement:+.1f}% vs DFS")

    # Remap parent/child/sibling through perm
    def remap(idx):
        return perm[idx] if idx != BTF_NULL_NODE else -1

    # slot_order[new_slot] = original_node_index
    slot_order = [0] * n
    for node, slot in enumerate(perm):
        slot_order[slot] = node

    # ── Emit C source ─────────────────────────────────────────────────────
    os.makedirs(args.source_output_dir, exist_ok=True)
    os.makedirs(args.include_output_dir, exist_ok=True)

    nodes_file = os.path.join(args.source_output_dir, NODES_DATA_FILE)
    actions_fn_set = set()

    with open(nodes_file, "w") as f:
        f.write(BTF_NODES_DATA_FILE_INCLUDE)
        f.write(f"/* Layout optimized by btf_layout_emitter.py  "
                f"(SA α={args.alpha}, cost={sa_cost:.0f} vs DFS {dfs_cost:.0f}, "
                f"{improvement:+.1f}%) */\n")
        f.write(BT_NODES_CARRAY_DECL_START)

        for new_slot in range(n):
            orig = slot_order[new_slot]
            tag  = tags[orig]
            attr = attribs[orig]

            action_fn  = "NULL"
            control_fn = "NULL"
            node_name  = '"' + attr.get("name", tag) + '"'

            if tag in BTF_CTYPE_POLICY_FUNCTIONS_MAP:
                control_fn = BTF_CTYPE_POLICY_FUNCTIONS_MAP[tag]
            elif tag in BTF_CTYPE_ALWAYS_ACTION_FUNCTIONS_MAP:
                action_fn = BTF_CTYPE_ALWAYS_ACTION_FUNCTIONS_MAP[tag]
            elif tag in ("Script", "ScriptCondition"):
                action_fn = attr["code"]
                actions_fn_set.add(action_fn)

            f.write(NODE_TEMPLATE.format(
                index   = new_slot,
                parent  = remap(parents[orig]),
                child   = remap(children[orig]),
                sibling = remap(siblings[orig]),
                action  = action_fn,
                control = control_fn,
                name    = node_name,
            ))

        f.write(BT_NODES_CARRAY_DECL_END + "\n\n")
        f.write(BT_NODES_SIZE_DECL + "\n")

    print(f"  Written: {nodes_file}")

    # ── Emit action function header ────────────────────────────────────────
    header_file = os.path.join(
        args.include_output_dir, ACTION_FUNCTION_DATA_FILE_NO_EXT + ".h"
    )
    with open(header_file, "w") as f:
        f.write(BTF_ACTION_FN_FILE_INCLUDE)
        for fn in actions_fn_set:
            f.write(BTF_ACTION_FUNCTION_NAME_CTYPE.format(name=fn) + ";\n\n")

    print(f"  Written: {header_file}")
    print("[btf_layout_emitter] Done.")
    return result


# ── CLI ───────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="btf_layout_emitter",
        description=(
            "Parse a Groot model, optimize node layout with SA+Fiedler, "
            "and emit a btf_node[] C array in the optimized slot order."
        ),
    )
    parser.add_argument("-m",  "--model",             required=True,  help="Groot XML model file")
    parser.add_argument("-tn", "--treename",           required=True,  help="BehaviorTree ID inside the model")
    parser.add_argument("--source-output-dir",         default=".",    help="Directory for .c output")
    parser.add_argument("--include-output-dir",        default=".",    help="Directory for .h output")
    parser.add_argument("--alpha",  type=float,        default=0.995,  help="SA cooling rate (default 0.995)")
    parser.add_argument("--seed",   type=int,          default=42,     help="SA RNG seed (default 42)")
    args = parser.parse_args()
    emit(args)
