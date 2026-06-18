"""
bt_graph.py — BT Graph Extraction for MinLA
============================================
Parses a BTreeFy-compatible Groot XML model and produces an undirected
weighted graph G = (V, E, W) over which the MinLA objective is defined.

Graph derivation rules (LCRS representation):
  For each node i, add an undirected edge for each non-null pointer field:
    (i, node.parent)   if parent  != BTF_NULL_NODE
    (i, node.child)    if child   != BTF_NULL_NODE
    (i, node.sibling)  if sibling != BTF_NULL_NODE
  Duplicate edges are removed. Result: |E| = O(n) for a tree.

Edge weights W_ij:
  Baseline: uniform W = 1 for all edges.
  Profile-guided weights can be supplied via the `weights` argument (P4 extension).
"""

import sys
import os
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Tuple

# ---------------------------------------------------------------------------
# Allow importing btf_groot_parser from the parent scripts/ directory
# ---------------------------------------------------------------------------
_SCRIPTS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, _SCRIPTS_DIR)

BTF_NULL_NODE = 0xFFFFFFFF


@dataclass
class BTGraph:
    """Undirected weighted graph derived from a BTreeFy node array."""

    # Number of nodes
    n: int
    # Node labels (tag + name from XML, index-aligned)
    labels: List[str]
    # Edge list — each entry is (u, v) with u < v
    edges: List[Tuple[int, int]]
    # Edge weights keyed by (u, v) with u < v; default = 1.0
    weights: Dict[Tuple[int, int], float]

    # Raw LCRS fields for each node, index-aligned
    # parent[i], child[i], sibling[i] ∈ {0..n-1} or BTF_NULL_NODE
    parents: List[int] = field(default_factory=list)
    children: List[int] = field(default_factory=list)
    siblings: List[int] = field(default_factory=list)

    def degree(self, v: int) -> int:
        return sum(1 for (u, w) in self.edges if u == v or w == v)

    def neighbours(self, v: int) -> List[int]:
        result = []
        for (u, w) in self.edges:
            if u == v:
                result.append(w)
            elif w == v:
                result.append(u)
        return result

    def edge_weight(self, u: int, v: int) -> float:
        key = (min(u, v), max(u, v))
        return self.weights.get(key, 1.0)


def _label(elem) -> str:
    name = elem.attrib.get("name", "")
    return f"{elem.tag}:{name}" if name else elem.tag


def from_xml(
    model_path: str,
    tree_id: str,
    profile_weights: Optional[Dict[Tuple[int, int], float]] = None,
) -> BTGraph:
    """
    Parse a Groot XML model and return a BTGraph.

    Parameters
    ----------
    model_path : str
        Path to the .xml file (BTCPP format 4).
    tree_id : str
        The BehaviorTree/@ID attribute to load (e.g. 'main_new').
    profile_weights : dict, optional
        Override edge weights {(u,v): float} (u < v). If None, W = 1 uniform.

    Returns
    -------
    BTGraph
    """
    # ── Parse XML ──────────────────────────────────────────────────────────
    tree_xml = ET.parse(model_path)
    root_xml = tree_xml.getroot()
    bt_root = root_xml.find(f"BehaviorTree[@ID='{tree_id}']")
    if bt_root is None:
        raise ValueError(
            f"BehaviorTree with ID='{tree_id}' not found in {model_path}"
        )
    bt_root = bt_root[0]  # The single child is the actual root node

    # ── Assign indices in DFS pre-order ───────────────────────────────────
    index_map: Dict = {}   # elem -> index
    labels: List[str] = []

    def _assign_indices(elem):
        idx = len(index_map)
        index_map[elem] = idx
        labels.append(_label(elem))
        for child in elem:
            _assign_indices(child)

    _assign_indices(bt_root)
    n = len(index_map)

    # ── Compute LCRS fields ───────────────────────────────────────────────
    parents = [BTF_NULL_NODE] * n
    children = [BTF_NULL_NODE] * n
    siblings = [BTF_NULL_NODE] * n

    def _set_lcrs(elem, parent_elem, sibling_elem):
        i = index_map[elem]
        if parent_elem is not None:
            parents[i] = index_map[parent_elem]
        elem_children = list(elem)
        if elem_children:
            children[i] = index_map[elem_children[0]]
        if sibling_elem is not None:
            siblings[i] = index_map[sibling_elem]
        for k, child in enumerate(elem_children):
            sib = elem_children[k + 1] if k + 1 < len(elem_children) else None
            _set_lcrs(child, elem, sib)

    _set_lcrs(bt_root, None, None)

    # ── Build edge set (deduplicated) ─────────────────────────────────────
    edge_set: set = set()
    for i in range(n):
        for ptr in (parents[i], children[i], siblings[i]):
            if ptr != BTF_NULL_NODE:
                u, v = min(i, ptr), max(i, ptr)
                edge_set.add((u, v))

    edges = sorted(edge_set)

    # ── Weights ───────────────────────────────────────────────────────────
    weights: Dict[Tuple[int, int], float] = {}
    if profile_weights:
        for (u, v), w in profile_weights.items():
            weights[(min(u, v), max(u, v))] = float(w)
    # Missing entries default to 1.0 (handled in BTGraph.edge_weight)

    return BTGraph(
        n=n,
        labels=labels,
        edges=edges,
        weights=weights,
        parents=parents,
        children=children,
        siblings=siblings,
    )


def from_lcrs_arrays(
    parents: List[int],
    children: List[int],
    siblings: List[int],
    labels: Optional[List[str]] = None,
    profile_weights: Optional[Dict[Tuple[int, int], float]] = None,
) -> BTGraph:
    """
    Construct a BTGraph directly from raw LCRS index arrays.
    Useful for synthetic BT instances (Phase 2 random generator).
    """
    n = len(parents)
    if labels is None:
        labels = [str(i) for i in range(n)]

    edge_set: set = set()
    for i in range(n):
        for ptr in (parents[i], children[i], siblings[i]):
            if ptr != BTF_NULL_NODE:
                u, v = min(i, ptr), max(i, ptr)
                edge_set.add((u, v))

    edges = sorted(edge_set)
    weights: Dict[Tuple[int, int], float] = {}
    if profile_weights:
        for (u, v), w in profile_weights.items():
            weights[(min(u, v), max(u, v))] = float(w)

    return BTGraph(
        n=n,
        labels=labels,
        edges=edges,
        weights=weights,
        parents=list(parents),
        children=list(children),
        siblings=list(siblings),
    )


# ---------------------------------------------------------------------------
# Toy-example sanity check (5-node BT from the presentation)
# ---------------------------------------------------------------------------
def _toy_example() -> BTGraph:
    """
    Fallback[0] → Sequence[1] → {Action_A[2], Action_B[3]}, Action_C[4]

    LCRS:
      parent:  [-1,  0,  1,  1,  0]
      child:   [ 1,  2, -1, -1, -1]
      sibling: [-1,  4,  3, -1, -1]

    Expected edges: (0,1),(0,4),(1,2),(1,3),(1,4),(2,3)  →  MinLA optimal cost = 8
    """
    NULL = BTF_NULL_NODE
    return from_lcrs_arrays(
        parents  = [NULL, 0,    1,    1,    0   ],
        children = [1,    2,    NULL, NULL, NULL ],
        siblings = [NULL, 4,    3,    NULL, NULL ],
        labels   = ["Fallback", "Sequence", "Action_A", "Action_B", "Action_C"],
    )


if __name__ == "__main__":
    g = _toy_example()
    print(f"Toy example: n={g.n}, |E|={len(g.edges)}")
    print(f"Edges: {g.edges}")
    print(f"Labels: {g.labels}")
