"""
cost.py — MinLA Cost Functions
================================
Two complementary cost evaluators shared by MILP verification and the SA
inner loop.

  minla_cost(graph, permutation)
      Full O(|E|) evaluation. Used to:
        - Cross-check the MILP objective value after solving.
        - Evaluate SA candidate solutions.

  incremental_cost_delta(graph, permutation, a, b)
      O(deg(a) + deg(b)) incremental update after swapping positions a <-> b.
      Critical for SA performance: avoids recomputing the full cost each step.

Permutation convention
----------------------
  permutation[i] = slot assigned to node i
  i.e. permutation is the bijection  y : V -> {0,..,n-1}
"""

from typing import List
from .bt_graph import BTGraph


def minla_cost(graph: BTGraph, permutation: List[int]) -> float:
    """
    Compute the full MinLA objective value.

      C = sum_{(i,j) in E} W_ij * |y_i - y_j|

    Parameters
    ----------
    graph : BTGraph
    permutation : list[int]
        permutation[i] = slot of node i  (length == graph.n)

    Returns
    -------
    float
    """
    cost = 0.0
    for (u, v) in graph.edges:
        w = graph.edge_weight(u, v)
        cost += w * abs(permutation[u] - permutation[v])
    return cost


def incremental_cost_delta(
    graph: BTGraph,
    permutation: List[int],
    a: int,
    b: int,
) -> float:
    """
    Compute the change in MinLA cost when swapping positions of nodes a and b.

    Returns delta = C_after_swap - C_before_swap

    Only edges incident to a or b can change cost; all others cancel.
    Complexity: O(deg(a) + deg(b)).

    Parameters
    ----------
    graph : BTGraph
    permutation : list[int]
        Current permutation (will NOT be modified).
    a, b : int
        Node indices whose slots are to be swapped.

    Returns
    -------
    float   (negative means the swap improves cost)
    """
    if a == b:
        return 0.0

    ya, yb = permutation[a], permutation[b]

    # Collect edges incident to a or b (excluding the edge (a,b) itself if
    # it exists, which is handled specially).
    incident_a = set(graph.neighbours(a))
    incident_b = set(graph.neighbours(b))

    delta = 0.0

    # Edges incident to a (excluding b)
    for nb in incident_a:
        if nb == b:
            continue
        w = graph.edge_weight(a, nb)
        y_nb = permutation[nb]
        delta += w * (abs(yb - y_nb) - abs(ya - y_nb))

    # Edges incident to b (excluding a)
    for nb in incident_b:
        if nb == a:
            continue
        w = graph.edge_weight(b, nb)
        y_nb = permutation[nb]
        delta += w * (abs(ya - y_nb) - abs(yb - y_nb))

    # Edge (a, b) if it exists — both endpoints swap, so distance is unchanged
    # No contribution to delta.

    return delta


# ---------------------------------------------------------------------------
# Self-test
# ---------------------------------------------------------------------------
def _selftest():
    from .bt_graph import _toy_example

    g = _toy_example()
    # DFS pre-order layout: [F=0, S=1, A=2, B=3, C=4]
    perm_dfs = [0, 1, 2, 3, 4]
    # Optimal layout:       [C=0, F=1, S=2, A=3, B=4]
    perm_opt = [1, 2, 3, 4, 0]  # node 0(F)->slot1, 1(S)->slot2, 2(A)->slot3, 3(B)->slot4, 4(C)->slot0

    c_dfs = minla_cost(g, perm_dfs)
    c_opt = minla_cost(g, perm_opt)

    assert c_dfs == 12.0, f"Expected 12, got {c_dfs}"
    assert c_opt == 8.0,  f"Expected 8, got {c_opt}"

    # Test incremental delta consistency
    import random
    random.seed(0)
    perm = list(range(g.n))
    for _ in range(200):
        a = random.randrange(g.n)
        b = random.randrange(g.n)
        c_before = minla_cost(g, perm)
        delta = incremental_cost_delta(g, perm, a, b)
        perm[a], perm[b] = perm[b], perm[a]
        c_after = minla_cost(g, perm)
        assert abs((c_after - c_before) - delta) < 1e-9, (
            f"Incremental mismatch at swap ({a},{b}): "
            f"delta={delta}, actual={c_after - c_before}"
        )

    print("cost.py self-test PASSED")
    print(f"  DFS pre-order cost : {c_dfs}")
    print(f"  Optimal cost       : {c_opt}")


if __name__ == "__main__":
    _selftest()
