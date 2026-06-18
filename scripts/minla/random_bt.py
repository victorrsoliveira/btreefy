"""
random_bt.py — Random BT Instance Generator
=============================================
Generates structurally valid BT graphs for the scaling experiment (Phase 2).
No XML files required — produces BTGraph objects directly from LCRS arrays.

Design
------
A BT is a rooted tree where each internal node is a control node (Sequence or
Fallback) and each leaf is an action node. We build the tree top-down:

  1. Start with a single root (control node if n > 1).
  2. While unplaced nodes remain, pick an existing control node at random and
     attach 2–max_children children to it. The last children placed exhaust
     the budget; each becomes a leaf unless more nodes remain.
  3. Assign LCRS fields (parent / first-child / right-sibling) from the tree.

LCRS edges are then extracted the same way as the real parser:
  For each node i, add (i, parent[i]), (i, child[i]), (i, sibling[i]) if ≠ NULL.

Parameters
----------
n    : int   Number of nodes (total tree size).
seed : int   RNG seed for reproducibility.

Returns
-------
BTGraph  ready to pass to milp_solver.solve() or sa_solver.solve().
"""

import random
from typing import List, Optional, Tuple
from .bt_graph import BTGraph, from_lcrs_arrays, BTF_NULL_NODE


def generate(n: int, seed: int = 0, max_children: int = 4) -> BTGraph:
    """
    Generate a random BT with exactly n nodes.

    Parameters
    ----------
    n            : int   Total node count (>= 2).
    seed         : int   RNG seed.
    max_children : int   Maximum children per control node (default 4).

    Returns
    -------
    BTGraph
    """
    if n < 1:
        raise ValueError("n must be >= 1")
    if n == 1:
        # Trivial: single leaf
        return from_lcrs_arrays(
            parents=[BTF_NULL_NODE],
            children=[BTF_NULL_NODE],
            siblings=[BTF_NULL_NODE],
            labels=["Action_0"],
        )

    rng = random.Random(seed)

    # ── Build adjacency tree (parent_of[i] = parent index, -1 for root) ──
    parent_of: List[int] = [-1] * n
    children_of: List[List[int]] = [[] for _ in range(n)]

    # Nodes that can still receive children (control nodes)
    control_nodes: List[int] = [0]   # root is always control
    next_idx = 1                      # next node to place

    while next_idx < n and control_nodes:
        # Pick a control node to expand
        parent_idx = rng.choice(control_nodes)

        # How many children to attach (at least 2 for a control node, up to max_children)
        remaining = n - next_idx
        num_children = min(rng.randint(2, max_children), remaining)

        for _ in range(num_children):
            if next_idx >= n:
                break
            child_idx = next_idx
            parent_of[child_idx] = parent_idx
            children_of[parent_idx].append(child_idx)
            next_idx += 1

        # Remove parent from control pool (avoid attaching twice;
        # already has children)
        control_nodes = [c for c in control_nodes if c != parent_idx]

        # New children become control nodes only if there are still nodes left
        # and we need more attachment points
        for child_idx in children_of[parent_idx]:
            if next_idx < n:
                control_nodes.append(child_idx)

    # If we still have unplaced nodes (can happen if control_nodes ran dry),
    # attach remaining nodes as children of the root.
    while next_idx < n:
        child_idx = next_idx
        parent_of[child_idx] = 0
        children_of[0].append(child_idx)
        next_idx += 1

    # ── Build LCRS arrays from adjacency ──────────────────────────────────
    # DFS pre-order index assignment (matches btf_groot_parser behaviour)
    dfs_order: List[int] = []

    def _dfs(node: int):
        dfs_order.append(node)
        for child in children_of[node]:
            _dfs(child)

    _dfs(0)

    # Map old indices to new DFS-order indices
    old_to_new = [0] * n
    for new_idx, old_idx in enumerate(dfs_order):
        old_to_new[old_idx] = new_idx

    # Rebuild children and parents in new indexing
    new_children_of: List[List[int]] = [[] for _ in range(n)]
    new_parent_of: List[int] = [-1] * n
    for old_idx in range(n):
        new_idx = old_to_new[old_idx]
        for old_child in children_of[old_idx]:
            new_child = old_to_new[old_child]
            new_children_of[new_idx].append(new_child)
        if parent_of[old_idx] >= 0:
            new_parent_of[new_idx] = old_to_new[parent_of[old_idx]]

    # Derive LCRS fields
    parents_arr  = [BTF_NULL_NODE] * n
    children_arr = [BTF_NULL_NODE] * n
    siblings_arr = [BTF_NULL_NODE] * n

    for i in range(n):
        if new_parent_of[i] >= 0:
            parents_arr[i] = new_parent_of[i]
        ch = new_children_of[i]
        if ch:
            children_arr[i] = ch[0]
        for k in range(len(ch) - 1):
            siblings_arr[ch[k]] = ch[k + 1]

    labels = [f"Node_{i}" for i in range(n)]

    return from_lcrs_arrays(
        parents=parents_arr,
        children=children_arr,
        siblings=siblings_arr,
        labels=labels,
    )


# ---------------------------------------------------------------------------
# Self-test
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    for n in [5, 8, 10, 12, 15, 20]:
        g = generate(n, seed=42)
        assert g.n == n, f"Expected n={n}, got {g.n}"
        # Every graph must be connected: |E| >= n-1
        assert len(g.edges) >= n - 1, f"n={n}: too few edges ({len(g.edges)})"
        print(f"n={n:3d}  |E|={len(g.edges):3d}  OK")
    print("random_bt.py self-test PASSED")
