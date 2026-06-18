"""
milp_solver.py — MILP Formulation of MinLA
===========================================
Exact solver using PuLP with the CBC backend.

Formulation (standard Ax <= b form):
--------------------------------------
Variables:
  y_i  ∈ [0, n-1]   continuous  — slot assigned to node i
  d_ij >= 0          continuous  — linearized |y_i - y_j|, one per edge (i,j)∈E
  z_ij ∈ {0,1}       binary      — ordering: z_ij=0 => i before j, one per pair i≠j

Constraints:
  [Abs-val linearization, for each (i,j) ∈ E]
    d_ij >= y_i - y_j
    d_ij >= y_j - y_i

  [Unique-slot Big-M, for each ordered pair i≠j, M=n]
    y_i - y_j >= 1 - n * z_ij        (if z_ij=0 → i before j)
    y_j - y_i >= 1 - n * (1-z_ij)   (if z_ij=1 → j before i)

Objective:
  min  sum_{(i,j)∈E}  W_ij * d_ij

Complexity note:
  |continuous vars| = n + |E|
  |binary vars|     = n*(n-1)   ← exponential bottleneck, CBC fails ~n=15-20

References:
  Garey & Johnson (1979) — MinLA NP-Hardness
  Petit (2011) — MinLA benchmark experiments
"""

import time
from dataclasses import dataclass
from typing import List, Optional

import pulp

from .bt_graph import BTGraph
from .cost import minla_cost


@dataclass
class MILPResult:
    permutation: List[int]     # permutation[node] = slot
    cost: float                # MinLA objective value
    status: str                # 'Optimal', 'Feasible', 'Infeasible', 'Timeout'
    solve_time: float          # wall-clock seconds
    solver_cost: float         # objective reported by CBC (cross-check vs cost.py)


def solve(
    graph: BTGraph,
    time_limit_s: float = 300.0,
    msg: bool = False,
) -> MILPResult:
    """
    Solve the MinLA MILP for the given BTGraph.

    Parameters
    ----------
    graph        : BTGraph
    time_limit_s : float   Solver wall-clock time limit (default 5 min).
    msg          : bool    If True, print CBC solver log.

    Returns
    -------
    MILPResult
        If CBC times out without proving optimality, status='Feasible' and
        the best incumbent is returned. If no feasible solution is found,
        permutation is the DFS pre-order identity and cost=inf.
    """
    n = graph.n
    M = n  # Big-M constant (sufficient for slot range [0, n-1])

    prob = pulp.LpProblem("BT_MinLA", pulp.LpMinimize)

    # ── Decision variables ────────────────────────────────────────────────
    y = [pulp.LpVariable(f"y_{i}", lowBound=0, upBound=n - 1, cat="Continuous")
         for i in range(n)]

    d = {
        (u, v): pulp.LpVariable(f"d_{u}_{v}", lowBound=0, cat="Continuous")
        for (u, v) in graph.edges
    }

    # z_ij for ALL ordered pairs i≠j (not just edges)
    z = {}
    for i in range(n):
        for j in range(n):
            if i != j:
                z[(i, j)] = pulp.LpVariable(f"z_{i}_{j}", cat="Binary")

    # ── Objective ─────────────────────────────────────────────────────────
    prob += pulp.lpSum(
        graph.edge_weight(u, v) * d[(u, v)]
        for (u, v) in graph.edges
    )

    # ── Abs-value linearization constraints ───────────────────────────────
    for (u, v) in graph.edges:
        prob += d[(u, v)] >= y[u] - y[v], f"abs_pos_{u}_{v}"
        prob += d[(u, v)] >= y[v] - y[u], f"abs_neg_{u}_{v}"

    # ── Unique-slot Big-M constraints ─────────────────────────────────────
    for i in range(n):
        for j in range(n):
            if i == j:
                continue
            # z[i,j]=0 → y_i before y_j → y_j - y_i >= 1
            prob += y[i] - y[j] >= 1 - M * z[(i, j)],         f"order_ij_{i}_{j}"
            prob += y[j] - y[i] >= 1 - M * (1 - z[(i, j)]),   f"order_ji_{i}_{j}"

    # ── Solve ─────────────────────────────────────────────────────────────
    solver = pulp.getSolver(
        "PULP_CBC_CMD",
        timeLimit=int(time_limit_s),
        msg=int(msg),
    )

    t0 = time.perf_counter()
    prob.solve(solver)
    elapsed = time.perf_counter() - t0

    status_str = pulp.LpStatus[prob.status]

    # ── Extract solution ──────────────────────────────────────────────────
    if prob.status in (pulp.LpStatusNotSolved,):
        # No incumbent available
        return MILPResult(
            permutation=list(range(n)),
            cost=float("inf"),
            status="Infeasible",
            solve_time=elapsed,
            solver_cost=float("inf"),
        )

    y_vals = [pulp.value(y[i]) for i in range(n)]

    if any(v is None for v in y_vals):
        return MILPResult(
            permutation=list(range(n)),
            cost=float("inf"),
            status="Infeasible",
            solve_time=elapsed,
            solver_cost=float("inf"),
        )

    # Round to nearest integer (CBC may leave tiny floating residuals)
    perm = [int(round(v)) for v in y_vals]

    # Resolve duplicates that can arise from floating-point rounding
    if len(set(perm)) < n:
        # Fall back to argsort of raw floats
        order = sorted(range(n), key=lambda i: y_vals[i])
        perm = [0] * n
        for slot, node in enumerate(order):
            perm[node] = slot

    solver_cost = pulp.value(prob.objective) or float("inf")
    actual_cost = minla_cost(graph, perm)

    # Map CBC status to our vocabulary
    if status_str == "Optimal":
        out_status = "Optimal"
    elif prob.status == pulp.LpStatusOptimal:
        out_status = "Optimal"
    else:
        out_status = "Feasible" if solver_cost < float("inf") else "Timeout"

    return MILPResult(
        permutation=perm,
        cost=actual_cost,
        status=out_status,
        solve_time=elapsed,
        solver_cost=solver_cost,
    )


# ---------------------------------------------------------------------------
# Self-test — toy 5-node example, expected optimal cost = 8
# ---------------------------------------------------------------------------
def _selftest():
    from .bt_graph import _toy_example

    print("Running MILP self-test on 5-node toy example …")
    g = _toy_example()
    result = solve(g, time_limit_s=60, msg=False)

    print(f"  Status      : {result.status}")
    print(f"  MinLA cost  : {result.cost}")
    print(f"  Permutation : {result.permutation}")
    print(f"  Solve time  : {result.solve_time:.3f}s")

    assert result.status == "Optimal", f"Expected Optimal, got {result.status}"
    assert result.cost == 8.0, f"Expected cost=8, got {result.cost}"
    print("MILP self-test PASSED ✓")


if __name__ == "__main__":
    import sys
    if "--selftest" in sys.argv:
        _selftest()
    else:
        print("Usage: python -m minla.milp_solver --selftest")
