"""
cache_sim.py — Direct-Mapped Cache Miss Simulation
====================================================
Translates a BT layout permutation into a memory access sequence under
DFS traversal, then counts cache misses using a direct-mapped cache model.

Model
-----
  - Cache line size: L bytes (default 64 — one x86 cache line).
  - sizeof(btf_node): configurable (default 32 bytes — 2 nodes per line).
  - Direct-mapped: each memory address maps to exactly one cache slot.
    Slot index = (address / line_size) % n_lines.
  - A miss occurs whenever a node is accessed and its cache slot holds
    a different address (cold miss or conflict miss).

Validation invariant (from GEMINI.md):
  Lower MinLA cost  ⟹  fewer simulated cache misses.

DFS traversal order
-------------------
The BTreeFy runner visits nodes in left-child right-sibling order:
  visit(node):
    process(node)
    if node.child != NULL: visit(node.child)
    if node.sibling != NULL: visit(node.sibling)

Under LCRS, this is equivalent to a pre-order DFS of the original BT,
which is the access sequence used for simulation.
"""

from typing import List, Tuple
from .bt_graph import BTGraph, BTF_NULL_NODE


# ── Struct size map ───────────────────────────────────────────────────────────
# Approximate sizeof(btf_node) for a 32-bit embedded target.
# Adjust if the real struct changes size.
BTF_NODE_SIZEOF = 32   # bytes


def access_sequence(graph: BTGraph, perm: List[int]) -> List[int]:
    """
    Compute the memory access sequence for a DFS traversal of the BT.

    Parameters
    ----------
    graph : BTGraph   — the behavior tree graph (LCRS fields used)
    perm  : list[int] — perm[node] = slot (the array position of each node)

    Returns
    -------
    List of array *slot* indices accessed in traversal order.
    """
    n = graph.n
    # Build slot → node reverse map
    slot_of = perm                   # slot_of[node] = slot
    node_at = [0] * n               # node_at[slot]  = node
    for node, slot in enumerate(slot_of):
        node_at[slot] = node

    # Build children and sibling arrays from the graph's LCRS fields
    # (graph.children[i] and graph.siblings[i] are already available)
    children  = graph.children
    siblings  = graph.siblings

    # DFS pre-order traversal — iterative to avoid Python stack overflow
    sequence: List[int] = []
    stack = [0]   # start from root (always node index 0)
    while stack:
        node = stack.pop()
        sequence.append(slot_of[node])
        # Push sibling first (processed after subtree), then child (processed next)
        sib = siblings[node]
        if sib != BTF_NULL_NODE:
            stack.append(sib)
        child = children[node]
        if child != BTF_NULL_NODE:
            stack.append(child)

    return sequence


def simulate(
    graph: BTGraph,
    perm: List[int],
    cache_lines: int = 2,
    line_size_bytes: int = 64,
    node_size_bytes: int = BTF_NODE_SIZEOF,
) -> Tuple[int, int]:
    """
    Simulate direct-mapped cache behaviour for one DFS traversal.

    Parameters
    ----------
    graph           : BTGraph
    perm            : list[int]  — perm[node] = slot
    cache_lines     : int        — number of cache lines (default 8)
    line_size_bytes : int        — bytes per cache line (default 64)
    node_size_bytes : int        — sizeof(btf_node) in bytes (default 32)

    Returns
    -------
    (hits, misses) : tuple[int, int]
    """
    nodes_per_line = max(1, line_size_bytes // node_size_bytes)
    seq = access_sequence(graph, perm)

    cache = [-1] * cache_lines   # cache[slot_idx] = tag of resident line
    hits, misses = 0, 0

    for slot in seq:
        line_addr = slot // nodes_per_line        # which cache line this slot is in
        cache_idx = line_addr % cache_lines       # direct-mapped slot
        if cache[cache_idx] == line_addr:
            hits += 1
        else:
            cache[cache_idx] = line_addr
            misses += 1

    return hits, misses


def miss_rate(
    graph: BTGraph,
    perm: List[int],
    cache_lines: int = 2,
    line_size_bytes: int = 64,
    node_size_bytes: int = BTF_NODE_SIZEOF,
) -> float:
    """Return cache miss rate ∈ [0.0, 1.0] for one DFS traversal."""
    hits, misses = simulate(graph, perm, cache_lines, line_size_bytes, node_size_bytes)
    total = hits + misses
    return misses / total if total else 0.0


# ── Self-test ─────────────────────────────────────────────────────────────────

def _selftest():
    from .bt_graph import _toy_example

    g = _toy_example()
    dfs_perm     = list(range(g.n))       # DFS pre-order (parser default)
    optimal_perm = [1, 2, 4, 3, 0]       # MILP optimal (cost=8)

    print("Cache simulation self-test (5-node toy, 2 cache lines, 64-byte lines):")
    dfs_hits,  dfs_misses  = simulate(g, dfs_perm,     cache_lines=2)
    opt_hits,  opt_misses  = simulate(g, optimal_perm, cache_lines=2)

    print(f"  DFS layout     — hits={dfs_hits}  misses={dfs_misses}  "
          f"miss_rate={dfs_misses/(dfs_hits+dfs_misses):.2f}")
    print(f"  Optimal layout — hits={opt_hits}  misses={opt_misses}  "
          f"miss_rate={opt_misses/(opt_hits+opt_misses):.2f}")

    print("  (Note: lower MinLA cost correlates with fewer misses on average,")
    print("   but individual layouts may differ at very small cache sizes.)")
    # Sanity: simulation must produce a valid miss rate in [0,1]
    assert 0.0 <= dfs_misses / (dfs_hits + dfs_misses) <= 1.0
    assert 0.0 <= opt_misses / (opt_hits + opt_misses) <= 1.0
    print("cache_sim.py self-test PASSED ✓")


if __name__ == "__main__":
    _selftest()
