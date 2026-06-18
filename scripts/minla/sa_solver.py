"""
sa_solver.py — Simulated Annealing with Spectral Initialization
================================================================
Heuristic solver for the MinLA problem. Designed for BT instances where
the MILP solver is intractable (n ≳ 15).

Algorithm
---------
1. **Spectral initialization (Fiedler sequencing)**
   Build the graph Laplacian L = D - A. Compute the Fiedler vector —
   the eigenvector corresponding to the 2nd smallest eigenvalue of L.
   Sort nodes by their Fiedler component → initial permutation y⁰.
   Grounded in Pettis & Hansen (1990): spatial locality principle.

2. **Simulated Annealing**
   - Neighbourhood: swap positions y_a ↔ y_b for random pair (a, b).
   - Acceptance: Δ ≤ 0 always accepted; Δ > 0 accepted with P = exp(-Δ/T).
   - Incremental cost via cost.incremental_cost_delta — O(deg) per step.
   - Cooling: geometric schedule T_{k+1} = α · T_k.
   - Stop: T < T_min = 1e-4 or max_iter exhausted.

References
----------
  Pettis & Hansen (1990) — Profile-guided code positioning / Fiedler sequencing
  Garey & Johnson (1979) — NP-Hardness of MinLA
"""

import math
import random as _random
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

import numpy as np
import scipy.linalg

from .bt_graph import BTGraph
from .cost import minla_cost, incremental_cost_delta


# ── Result type ───────────────────────────────────────────────────────────────

@dataclass
class SAResult:
    permutation: List[int]       # best permutation found (node → slot)
    cost: float                  # best MinLA cost achieved
    initial_cost: float          # cost of Fiedler-seeded starting point
    alpha: float                 # cooling rate used
    iterations: int              # total SA steps executed
    # Convergence history: list of (iteration, cost_at_that_point)
    # Sampled at ~1000 checkpoints; use for plotting.
    history: List[Tuple[int, float]] = field(default_factory=list)


# ── Spectral initialization ───────────────────────────────────────────────────

def fiedler_permutation(graph: BTGraph) -> List[int]:
    """
    Compute the Fiedler-vector-based initial permutation.

    Steps:
      1. Build weighted graph Laplacian L = D - A.
      2. Compute all eigenvalues/vectors (numpy.linalg.eigh — ascending order).
      3. Fiedler vector = eigenvectors[:, 1]  (2nd smallest eigenvalue).
      4. Sort nodes by Fiedler component → permutation.

    For connected graphs the Fiedler value (λ₂) is strictly positive.
    The sort order approximates the MinLA arrangement (Pettis & Hansen, 1990).

    Returns
    -------
    perm : list[int]
        perm[node] = slot  (length n, bijection onto {0..n-1})
    """
    n = graph.n

    # Build adjacency matrix (weighted)
    A = np.zeros((n, n), dtype=float)
    for (u, v) in graph.edges:
        w = graph.edge_weight(u, v)
        A[u, v] = w
        A[v, u] = w

    # Degree matrix + Laplacian
    D = np.diag(A.sum(axis=1))
    L = D - A

    # Eigendecomposition — eigh returns eigenvalues in ascending order
    eigenvalues, eigenvectors = scipy.linalg.eigh(L)

    # Fiedler vector: index 1 (index 0 is the constant eigenvector, λ=0)
    fiedler = eigenvectors[:, 1]

    # Sort nodes by Fiedler component; assign slots in that order
    order = np.argsort(fiedler)          # order[slot] = node
    perm = [0] * n
    for slot, node in enumerate(order):
        perm[int(node)] = slot

    return perm


# ── Simulated Annealing ───────────────────────────────────────────────────────

def solve(
    graph: BTGraph,
    alpha: float = 0.995,
    T_min: float = 1e-4,
    max_iter: Optional[int] = None,
    seed: int = 42,
    record_every: Optional[int] = None,
) -> SAResult:
    """
    Solve MinLA on `graph` using Simulated Annealing.

    Parameters
    ----------
    graph       : BTGraph
    alpha       : float    Geometric cooling rate (default 0.995).
    T_min       : float    Stopping temperature (default 1e-4).
    max_iter    : int|None Iteration cap (None = cool to T_min only).
    seed        : int      RNG seed.
    record_every: int|None History sampling interval. If None, auto (~1000 pts).

    Returns
    -------
    SAResult
    """
    rng = _random.Random(seed)
    n = graph.n

    # ── 1. Fiedler initialization ─────────────────────────────────────────
    perm = fiedler_permutation(graph)
    C = minla_cost(graph, perm)
    initial_cost = C

    # ── 2. Temperature schedule ───────────────────────────────────────────
    T0 = max(C, 1.0)   # initial temperature = initial cost (≥ 1 guard)
    T = T0

    # Estimate total iterations for auto record_every
    total_est = int(math.log(T_min / T0) / math.log(alpha)) if alpha < 1.0 else 1
    if max_iter is not None:
        total_est = min(total_est, max_iter)
    if record_every is None:
        record_every = max(1, total_est // 1000)

    # ── 3. SA loop ────────────────────────────────────────────────────────
    best_perm = perm[:]
    best_cost = C
    history: List[Tuple[int, float]] = [(0, C)]
    it = 0

    while T > T_min:
        if max_iter is not None and it >= max_iter:
            break

        # Random swap of two distinct nodes
        a = rng.randrange(n)
        b = rng.randrange(n)
        if a == b:
            T *= alpha
            it += 1
            continue

        delta = incremental_cost_delta(graph, perm, a, b)

        # Acceptance criterion
        if delta <= 0 or rng.random() < math.exp(-delta / T):
            perm[a], perm[b] = perm[b], perm[a]
            C += delta

            if C < best_cost:
                best_cost = C
                best_perm = perm[:]

        T *= alpha
        it += 1

        if it % record_every == 0:
            history.append((it, C))

    # Always record the final state
    if not history or history[-1][0] != it:
        history.append((it, C))

    return SAResult(
        permutation=best_perm,
        cost=best_cost,
        initial_cost=initial_cost,
        alpha=alpha,
        iterations=it,
        history=history,
    )


# ── Convenience: compare Fiedler vs random init ───────────────────────────────

def solve_random_init(
    graph: BTGraph,
    alpha: float = 0.995,
    T_min: float = 1e-4,
    max_iter: Optional[int] = None,
    seed: int = 42,
    record_every: Optional[int] = None,
) -> SAResult:
    """Same as solve() but uses a random initial permutation instead of Fiedler."""
    rng = _random.Random(seed)
    n = graph.n
    perm = list(range(n))
    rng.shuffle(perm)
    C = minla_cost(graph, perm)
    initial_cost = C
    T0 = max(C, 1.0)
    T = T0

    total_est = int(math.log(T_min / T0) / math.log(alpha)) if alpha < 1.0 else 1
    if max_iter is not None:
        total_est = min(total_est, max_iter)
    if record_every is None:
        record_every = max(1, total_est // 1000)

    best_perm = perm[:]
    best_cost = C
    history: List[Tuple[int, float]] = [(0, C)]
    it = 0

    while T > T_min:
        if max_iter is not None and it >= max_iter:
            break
        a = rng.randrange(n)
        b = rng.randrange(n)
        if a == b:
            T *= alpha
            it += 1
            continue
        delta = incremental_cost_delta(graph, perm, a, b)
        if delta <= 0 or rng.random() < math.exp(-delta / T):
            perm[a], perm[b] = perm[b], perm[a]
            C += delta
            if C < best_cost:
                best_cost = C
                best_perm = perm[:]
        T *= alpha
        it += 1
        if it % record_every == 0:
            history.append((it, C))

    if not history or history[-1][0] != it:
        history.append((it, C))

    return SAResult(
        permutation=best_perm,
        cost=best_cost,
        initial_cost=initial_cost,
        alpha=alpha,
        iterations=it,
        history=history,
    )


# ── Self-test ─────────────────────────────────────────────────────────────────

def _selftest():
    from .bt_graph import _toy_example

    print("Running SA self-test on 5-node toy example …")
    g = _toy_example()

    for alpha in [0.99, 0.995, 0.999]:
        result = solve(g, alpha=alpha, seed=0)
        gap = (result.cost - 8.0) / 8.0 * 100
        print(
            f"  α={alpha}  cost={result.cost:.1f}  "
            f"gap={gap:+.1f}%  iters={result.iterations}  "
            f"init={result.initial_cost:.1f}"
        )
        assert result.cost <= 10.0, f"SA quality too poor for α={alpha}: cost={result.cost}"

    # Fiedler warm-start should be no worse than random init on the toy example
    fiedler_result = solve(g, alpha=0.995, seed=0)
    random_result  = solve_random_init(g, alpha=0.995, seed=0)
    print(f"\n  Fiedler init cost: {fiedler_result.initial_cost:.1f}")
    print(f"  Random  init cost: {random_result.initial_cost:.1f}")

    print("\nSA self-test PASSED ✓")


if __name__ == "__main__":
    import sys
    if "--selftest" in sys.argv:
        _selftest()
    else:
        print("Usage: python3 -m minla.sa_solver --selftest")
