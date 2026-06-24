"""
gen_random_instances.py — Random BT Instance Generator
=======================================================
Generates K random BehaviorTree instances of size N and emits, for each one:
  - btf_nodes_generated.c
  - btf_action_functions_generated.h

Each instance goes into its own numbered subdirectory inside --output-dir:
  output_dir/
    000/
      btf_nodes_generated.c
      btf_action_functions_generated.h
    001/
      ...

Node assignment convention
--------------------------
  Control nodes (those with at least one child):
    Alternating Sequence / Fallback in DFS pre-order encounter order.
  Leaf nodes:
    Each gets a unique action function: action_A, action_B, ... (wraps to AA, AB, ...)

Layout
------
  Default: DFS pre-order (same as btf_groot_parser.py).
  --optimize: run SA (Fiedler init, α=0.995) and re-emit in optimized slot order.

Usage
-----
  python3 scripts/minla/gen_random_instances.py --n 20 --k 10 --output-dir out/
  python3 scripts/minla/gen_random_instances.py --n 20 --k 10 --output-dir out/ --optimize
  python3 scripts/minla/gen_random_instances.py --n 20 --k 10 --output-dir out/ --seed 7
"""

import argparse
import os
import sys

# ── Path bootstrap ─────────────────────────────────────────────────────────────
_SCRIPTS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_REPO    = os.path.dirname(_SCRIPTS)
for p in [_SCRIPTS, _REPO]:
    if p not in sys.path:
        sys.path.insert(0, p)

from minla.random_bt  import generate
from minla.bt_graph   import BTF_NULL_NODE
from minla.cost       import minla_cost
from minla.sa_solver  import solve as sa_solve

# ── C template (mirrors btf_groot_parser.py) ──────────────────────────────────
_ACTION_FN_CTYPE  = "enum btf_node_status {name}(struct btf_tree *tree)"
_CTRL_POLICY_MAP  = {
    "Sequence": "btf_sequence_policy_fn",
    "Fallback":  "btf_fallback_policy_fn",
}

_NODES_HEADER = """\
#include "btreefy/btreefy_policies.h"
#include "btreefy/btreefy_always_actions.h"
#include "btf_action_functions_generated.h"

"""

_ACTION_HEADER = """\
#include "btreefy/btreefy_objs.h"

"""

_NODE_TEMPLATE = """\
    [{index}] = {{.status   = BTF_UNDEF_STATUS,
            .parent   = {parent},
            .child    =  {child},
            .sibling = {sibling},
            .action   = {action},
            .control  = {control},
            .name     = "{name}"}},
"""


# ── Helpers ───────────────────────────────────────────────────────────────────

def _leaf_label(n: int) -> str:
    """0→A, 1→B, …, 25→Z, 26→AA, …"""
    n += 1
    letters = []
    while n > 0:
        n, r = divmod(n - 1, 26)
        letters.append(chr(ord("A") + r))
    return "".join(reversed(letters))


def _classify_nodes(graph):
    """
    Returns:
      ctrl_type[i] : "Sequence" | "Fallback" (None for leaves)
      leaf_fn[i]   : action function name string (None for control nodes)
    """
    ctrl_type = [None] * graph.n
    leaf_fn   = [None] * graph.n
    ctrl_count = 0
    leaf_count = 0

    for i in range(graph.n):
        if graph.children[i] != BTF_NULL_NODE:
            ctrl_type[i] = "Sequence" if ctrl_count % 2 == 0 else "Fallback"
            ctrl_count += 1
        else:
            leaf_fn[i] = f"action_{_leaf_label(leaf_count)}"
            leaf_count += 1

    return ctrl_type, leaf_fn


def _null_str(v: int) -> str:
    return str(v) if v != BTF_NULL_NODE else "-1"


# ── Per-instance emitter ───────────────────────────────────────────────────────

def emit_instance(graph, out_dir: str, optimize: bool, sa_alpha: float, sa_seed: int) -> dict:
    """
    Emit btf_nodes_generated.c and btf_action_functions_generated.h
    into out_dir.  Returns a dict with cost metadata.
    """
    os.makedirs(out_dir, exist_ok=True)

    n = graph.n

    # Build permutation
    dfs_perm = list(range(n))
    dfs_cost = minla_cost(graph, dfs_perm)

    if optimize:
        result = sa_solve(graph, alpha=sa_alpha, seed=sa_seed)
        perm   = result.permutation
        sa_cost = result.cost
    else:
        perm    = dfs_perm
        sa_cost = None

    # slot_order[slot] = original_node_index
    slot_order = [0] * n
    for node, slot in enumerate(perm):
        slot_order[slot] = node

    # Classify nodes (in original index space)
    ctrl_type, leaf_fn = _classify_nodes(graph)

    def remap(idx: int) -> int:
        return perm[idx] if idx != BTF_NULL_NODE else -1

    # Collect action functions (in slot order)
    actions_set: set[str] = set()
    nodes_lines = []
    for new_slot in range(n):
        orig = slot_order[new_slot]

        if ctrl_type[orig]:
            ctrl_fn   = _CTRL_POLICY_MAP[ctrl_type[orig]]
            action_fn = "NULL"
            node_name = ctrl_type[orig]
        else:
            ctrl_fn   = "NULL"
            action_fn = leaf_fn[orig]
            node_name = leaf_fn[orig]
            actions_set.add(action_fn)

        nodes_lines.append(_NODE_TEMPLATE.format(
            index   = new_slot,
            parent  = remap(graph.parents[orig]),
            child   = remap(graph.children[orig]),
            sibling = remap(graph.siblings[orig]),
            action  = action_fn,
            control = ctrl_fn,
            name    = node_name,
        ))

    # ── Write btf_nodes_generated.c ───────────────────────────────────────
    c_path = os.path.join(out_dir, "btf_nodes_generated.c")
    with open(c_path, "w") as f:
        f.write(_NODES_HEADER)
        if optimize:
            f.write(
                f"/* Layout optimized by gen_random_instances.py  "
                f"SA α={sa_alpha}, sa_cost={sa_cost:.0f} vs dfs_cost={dfs_cost:.0f} "
                f"({(dfs_cost - sa_cost) / dfs_cost * 100:+.1f}%) */\n"
            )
        f.write("struct btf_node nodes[] = {\n")
        for line in nodes_lines:
            f.write(line)
        f.write("};\n\n")
        f.write(
            f"size_t nodes_size = sizeof(nodes)/sizeof(nodes[0]);\n"
        )

    # ── Write btf_action_functions_generated.h ────────────────────────────
    h_path = os.path.join(out_dir, "btf_action_functions_generated.h")
    with open(h_path, "w") as f:
        f.write(_ACTION_HEADER)
        for fn in sorted(actions_set):
            f.write(_ACTION_FN_CTYPE.format(name=fn) + ";\n\n")

    return {
        "dfs_cost": dfs_cost,
        "sa_cost":  sa_cost,
    }


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        prog="gen_random_instances",
        description="Generate K random BT instances of size N and emit C source files.",
    )
    parser.add_argument("--n",          type=int,   required=True,  help="Number of nodes per tree")
    parser.add_argument("--k",          type=int,   required=True,  help="Number of instances to generate")
    parser.add_argument("--output-dir", type=str,   required=True,  help="Root output directory")
    parser.add_argument("--seed",       type=int,   default=0,      help="Starting RNG seed (instance i uses seed+i, default 0)")
    parser.add_argument("--optimize",   action="store_true",        help="Run SA optimization before emitting")
    parser.add_argument("--alpha",      type=float, default=0.995,  help="SA cooling rate (default 0.995, only with --optimize)")
    parser.add_argument("--sa-seed",    type=int,   default=42,     help="SA RNG seed (default 42, only with --optimize)")
    args = parser.parse_args()

    width = len(str(args.k - 1))   # zero-padding width

    print(f"Generating {args.k} instances  n={args.n}  "
          f"{'[SA optimized]' if args.optimize else '[DFS layout]'}")
    print(f"Output → {os.path.abspath(args.output_dir)}/")
    print()

    for i in range(args.k):
        seed     = args.seed + i
        graph    = generate(args.n, seed=seed)
        inst_dir = os.path.join(args.output_dir, str(i).zfill(width))

        meta = emit_instance(
            graph    = graph,
            out_dir  = inst_dir,
            optimize = args.optimize,
            sa_alpha = args.alpha,
            sa_seed  = args.sa_seed,
        )

        status = f"dfs_cost={meta['dfs_cost']:.0f}"
        if args.optimize and meta["sa_cost"] is not None:
            improvement = (meta["dfs_cost"] - meta["sa_cost"]) / meta["dfs_cost"] * 100
            status += f"  sa_cost={meta['sa_cost']:.0f}  ({improvement:+.1f}%)"

        print(f"  [{str(i).zfill(width)}] seed={seed}  {status}  → {inst_dir}/")

    print(f"\nDone. {args.k} instances written.")


if __name__ == "__main__":
    main()
