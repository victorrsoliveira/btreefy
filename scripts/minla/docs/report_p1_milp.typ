// ─────────────────────────────────────────────────────────────────────────────
// report_p1_milp.typ — Phase 1 Report: MILP Formulation
// BT MinLA Project · UFAL PPGI · 2026
// ─────────────────────────────────────────────────────────────────────────────
#import "preamble.typ": *
#show: report-setup

// ── Title block ───────────────────────────────────────────────────────────────
#align(center)[
  #text(fill: muted, size: 10pt)[BT MinLA Project · UFAL PPGI · 2026]
  #v(8pt)
  #text(fill: clr_text, size: 22pt, weight: "bold")[
    Phase 1 Report: MILP Formulation\
    of the Minimum Linear Arrangement Problem
  ]
  #v(6pt)
  #grid(columns: (1fr, 1fr, 1fr),
    align(center)[#pill("MinLA / NP-Hard")],
    align(center)[#pill("PuLP · CBC", color: accent2)],
    align(center)[#pill("Exact · n ≤ 12", color: warn)],
  )
  #v(4pt)
  #text(fill: muted, size: 10pt)[Victor Oliveira · victor.rsoliveira\@gmail.com]
]

#v(16pt)
#divider
#v(16pt)

// ─────────────────────────────────────────────────────────────────────────────
= Motivation
// ─────────────────────────────────────────────────────────────────────────────

BTreeFy stores Behavior Tree nodes as a flat 1D C array of `btf_node` structs.
DFS traversal follows integer index fields (`parent`, `child`, `sibling`) that
act as pointers into that array. Because modern CPUs load memory in cache lines,
*the relative positions of connected nodes in the array directly determine
instruction cache efficiency*: two nodes separated by many slots force a cache
eviction and reload every time the traversal edge between them is exercised.

The goal of Phase 1 is to establish an *exact* mathematical formulation of this
layout optimization problem and implement a solver capable of producing
*provably optimal* layouts for small instances ($n <= 12$). The exact solution
serves two purposes: (i) it provides ground-truth optimal costs against which
the Phase 3 heuristic will be benchmarked, and (ii) it exposes the structural
bottleneck that motivates the Phase 2 scaling study.

// ─────────────────────────────────────────────────────────────────────────────
= Problem Formulation
// ─────────────────────────────────────────────────────────────────────────────

== Graph Model

The BT is modelled as an undirected weighted graph $G = (V, E, W)$:

#card(title: "Graph construction rules", color: accent)[
  #set text(size: 10pt)
  - $V = {0, 1, ..., n-1}$ — one vertex per BT node (array index)
  - For each node $i$, add an undirected edge for each non-null LCRS field:
    - $(i, "node.parent")$ if `parent` ≠ `BTF_NULL_NODE`
    - $(i, "node.child")$ if `child` ≠ `BTF_NULL_NODE`
    - $(i, "node.sibling")$ if `sibling` ≠ `BTF_NULL_NODE`
  - Duplicate edges removed. Result: $|E| = O(n)$ for a tree.
  - $W_(i j) = 1$ (uniform baseline). Profile-guided weights are reserved for Phase 4.
]

== MinLA Objective

Find a bijection $y : V arrow.r {0, 1, ..., n-1}$ minimising:

$ min_(y) quad sum_((i,j) in E) W_(i j) dot |y_i - y_j| $

subject to $y_i != y_j quad forall i != j$ (all slots distinct).

#note-box[
  *Complexity:* MinLA is NP-Hard for general graphs #ref-cite("Garey & Johnson, 1979"). BT graphs are trees — MinLA on unweighted trees admits a polynomial algorithm, but the weighted formulation and cache-model extensions we target re-introduce hardness.
]

// ─────────────────────────────────────────────────────────────────────────────
= MILP Implementation
// ─────────────────────────────────────────────────────────────────────────────

== Linearization of the Absolute Value (Step 1)

The absolute value $|y_i - y_j|$ is non-linear and cannot appear directly in
a linear program. We introduce an auxiliary continuous variable
$d_(i j) >= 0$ per edge and replace the objective with:

$ min quad sum_((i,j) in E) W_(i j) dot d_(i j) $

subject to the envelope constraints:

$ d_(i j) >= y_i - y_j quad forall (i,j) in E $
$ d_(i j) >= y_j - y_i quad forall (i,j) in E $

A minimizing solver drives each $d_(i j)$ to exactly $|y_i - y_j|$.

== Unique-Slot Enforcement via Big-M (Step 2)

The integrality constraint $y_i != y_j$ must be linearized. We introduce
a binary variable $z_(i j) in {0, 1}$ for each *ordered* pair $i != j$,
encoding which node occupies the earlier slot:

$ y_i - y_j >= 1 - M z_(i j) quad forall i != j $
$ y_j - y_i >= 1 - M (1 - z_(i j)) quad forall i != j $

With $M = n$ (sufficient since slot indices lie in $[0, n-1]$):
- If $z_(i j) = 0$: forces $y_j - y_i >= 1$ (node $i$ precedes node $j$).
- If $z_(i j) = 1$: forces $y_i - y_j >= 1$ (node $j$ precedes node $i$).

== Variable Count

#card(title: "Variable census", color: warn)[
  #set text(size: 10pt)
  #grid(columns: (1fr, 1fr), gutter: 12pt,
    [
      *Continuous:*
      - $y_i$: $n$ variables (node positions)
      - $d_(i j)$: $|E|$ variables (edge distances)
      - Total: $n + |E| = O(n)$
    ],
    [
      *Binary:*
      - $z_(i j)$: $n(n-1)$ variables (ordering)
      - This is the *exponential bottleneck*
      - CBC hits intractability near $n approx 15$–$20$
    ]
  )
]

== Solver & Tooling

The formulation is implemented in `milp_solver.py` using *PuLP 2.x* with
the *CBC* backend. CBC is open-source and ships with PuLP — no licence
required for academic use.

Key implementation choices:

#algo(title: "milp_solver.py — design decisions")[
  #set text(size: 10pt)
  #bullet[`y_i` declared as `Continuous` with bounds `[0, n-1]`. CBC's branch-and-bound on the $z_(i j)$ binaries implicitly enforces integrality of the final slot assignments.]
  #bullet[Ordering variables $z_(i j)$ are created for *all* ordered pairs, not just edges. Uniqueness must hold globally, not only for adjacent nodes.]
  #bullet[Floating-point residuals from CBC are resolved by argsort of raw $y$ values, guarding against rounding-induced duplicate slots.]
  #bullet[`msg=False` by default; CBC verbosity is a parameter for debugging.]
]

// ─────────────────────────────────────────────────────────────────────────────
= Results — Toy Example Validation
// ─────────────────────────────────────────────────────────────────────────────

The canonical 5-node BT from the project specification:

```
Fallback [0]
├── Sequence  [1]
│   ├── Action A [2]
│   └── Action B [3]
└── Action C  [4]
```

LCRS edges: E = {(0,1), (0,4), (1,2), (1,3), (1,4), (2,3)} — 6 edges, W=1.

#grid(columns: (1fr, 1fr), gutter: 14pt)[
  #card(title: "Layout A — DFS pre-order (parser default)", color: warn)[
    #set text(size: 10pt)
    `y = [0, 1, 2, 3, 4]`  →  `[F, S, A, B, C]`

    $ C_A = 1+4+1+2+3+1 = bold(12) $

    Node $S$ at slot 1 is far from $C$ (dist 3) and $B$ (dist 2).
  ]
  #card(title: "Layout B — MILP optimal  ✦", color: accent2)[
    #set text(size: 10pt)
    `y = [1, 2, 4, 3, 0]`  →  `[C, F, S, B, A]`

    $ C_B = 1+1+2+1+2+1 = bold(8) $

    $S$ centred; pairs $C$–$F$ and $A$–$B$ adjacent.
  ]
]

#result-box[
  *MILP solver returned cost = 8 in 0.19 s — matches the provably optimal lower bound.*

  Lower bound argument: $S$ has degree 4, so at least 2 of its edges must span $>= 2$ slots. Minimum contribution = $2 times 2 = 4$; remaining 2 edges contribute at least 4. Total lower bound = 8. #ref-cite("Petit, 2011 — lower bound methodology")

  *33% cost reduction over DFS pre-order.* Both `cost.py` cross-check and MILP objective agree on cost = 8.
]

#v(8pt)
Self-test output (run via `python3 -m minla.milp_solver --selftest`):

```
Running MILP self-test on 5-node toy example …
  Status      : Optimal
  MinLA cost  : 8.0
  Permutation : [1, 2, 4, 3, 0]
  Solve time  : 0.190s
MILP self-test PASSED ✓
```

// ─────────────────────────────────────────────────────────────────────────────
= Implementation Notes & Alternatives Considered
// ─────────────────────────────────────────────────────────────────────────────

#card(title: "Why PuLP/CBC and not Gurobi or CPLEX?", color: border)[
  #set text(size: 10pt)
  Gurobi and CPLEX are significantly faster but require commercial licences. For the purposes of this research — where the MILP is used only up to $n approx 12$ and primarily to generate ground-truth for the optimality gap analysis — CBC's performance is entirely adequate. The formulation is solver-agnostic; switching backends requires only changing the `getSolver` call.
]

#card(title: "Why not use a pure tree algorithm?", color: border)[
  #set text(size: 10pt)
  MinLA on *unweighted* trees is polynomial #ref-cite("Shiloach, 1979") and could be solved in $O(n log n)$. However: (i) the weighted extension targeted in Phase 4 re-introduces hardness, (ii) the LCRS graph is not a simple tree (sibling edges create non-tree links), and (iii) the MILP formulation generalizes directly to arbitrary graphs. The added generality justifies the complexity cost.
]

// ─────────────────────────────────────────────────────────────────────────────
= References
// ─────────────────────────────────────────────────────────────────────────────

#divider
#v(6pt)
#set text(size: 10pt)

- *Garey, M.R. & Johnson, D.S. (1979).* Computers and Intractability: A Guide to the Theory of NP-Completeness. Freeman. — MinLA NP-Hardness proof.

- *Petit, J. (2011).* Experiments on the minimum linear arrangement problem. _Journal of Experimental Algorithmics_, 8. — Benchmark methodology and lower bound derivation.

- *Pettis, K. & Hansen, R.C. (1990).* Profile guided code positioning. _ACM SIGPLAN Notices_, 25(6), 16–27. — Spatial locality principle; motivates Fiedler vector initialization in Phase 3.

- *Shiloach, Y. (1979).* A minimum linear arrangement algorithm for undirected trees. _SIAM Journal on Computing_, 8(1), 15–32. — Polynomial-time algorithm for the unweighted tree case.
