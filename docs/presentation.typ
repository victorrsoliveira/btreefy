// ─────────────────────────────────────────────────────────────────────────────
// Cache-Aware BT Memory Layout — Research Presentation
// Victor Oliveira — UFAL PPGI — 2026
// ─────────────────────────────────────────────────────────────────────────────

#import "@preview/polylux:0.4.0": *

// ── Theme ────────────────────────────────────────────────────────────────────
#let bg      = rgb("#0d1117")
#let surface = rgb("#161b22")
#let border  = rgb("#30363d")
#let accent  = rgb("#58a6ff")
#let accent2 = rgb("#3fb950")
#let warn    = rgb("#d29922")
#let clr_text  = rgb("#e6edf3")
#let muted   = rgb("#8b949e")

#set page(
  paper: "presentation-16-9",
  fill: bg,
  margin: (x: 2.2cm, y: 1.6cm),
)

#set text(font: "Libertinus Serif", fill: clr_text, size: 18pt)
#show heading: set text(fill: accent, )
#show strong:  set text(fill: accent)
#show emph:    set text(fill: accent2, style: "italic")

// ── Helper components ─────────────────────────────────────────────────────────
#let pill(content, color: accent) = box(
  fill: color.transparentize(80%),
  stroke: 0.6pt + color,
  radius: 4pt,
  inset: (x: 6pt, y: 3pt),
  text(fill: color, size: 14pt, content)
)

#let card(content, title: none, color: accent) = block(
  fill: surface,
  stroke: 0.8pt + color.transparentize(60%),
  radius: 6pt,
  inset: 14pt,
  width: 100%,
  {
    if title != none {
      text(fill: color, size: 14pt, weight: "bold", title)
      v(6pt)
    }
    content
  }
)

#let badge(t, color: accent) = box(
  fill: color,
  radius: 3pt,
  inset: (x: 5pt, y: 2pt),
  text(fill: bg, size: 13pt, weight: "bold", t)
)

#let divider = line(length: 100%, stroke: 0.6pt + border)

#let slide-title(t) = {
  text(fill: accent, size: 26pt, weight: "bold", t)
  v(2pt)
  divider
  v(10pt)
}

#let bullet(content) = pad(left: 8pt, {
  grid(columns: (12pt, 1fr), gutter: 6pt,
    text(fill: accent, "›"), content)
})

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 1 — Title
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #v(1fr)
  #align(center)[
    #text(fill: muted, size: 14pt)[UFAL · PPGI · Master's Research · 2026]
    #v(14pt)
    #text(fill: clr_text, size: 32pt, weight: "bold")[
      Cache-Aware Memory Layout\
      for Behavior Trees
    ]
    #v(10pt)
    #text(fill: accent, size: 20pt)[
      Minimizing Instruction Cache Misses\
      via Minimum Linear Arrangement
    ]
    #v(20pt)
    #divider
    #v(8pt)
    #grid(columns: (1fr, 1fr, 1fr),
      align(center)[#pill("MinLA / NP-Hard")],
      align(center)[#pill("MILP + Metaheuristics", color: accent2)],
      align(center)[#pill("Embedded Systems", color: warn)],
    )
    #v(8pt)
    #text(fill: muted, size: 14pt)[Victor Oliveira · victor.rsoliveira\@gmail.com]
  ]
  #v(1fr)
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 2 — Context: BTreeFy
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[Context: BTreeFy Runtime]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "What is BTreeFy?", color: accent)[
      #set text(size: 16pt)
      #bullet[C library for *Behavior Trees* on embedded targets (bare-metal, POSIX, Zephyr)]
      #v(4pt)
      #bullet[Nodes stored as a *flat 1D C array* of `btf_node` structs]
      #v(4pt)
      #bullet[Traversal is DFS — pointer jumps follow `child`, `sibling`, `parent` index fields]
    ]
    #card(title: "The btf_node layout in memory", color: accent2)[
      #set text(size: 14pt, font: "Fira Code")
      ```c
      struct btf_node {
        uint32_t parent;
        uint32_t child;
        uint32_t sibling;
        btf_action_fn_t  action;
        btf_control_fn_t control;
        char *name;
      };
      ```
      #set text(size: 14pt, font: "Libertinus Serif")
      #text(fill: muted)[Array index → memory address]
    ]
  ]
  #v(10pt)
  #card(color: warn)[
    #set text(size: 16pt)
    #grid(columns: (auto, 1fr), gutter: 10pt,
      badge("KEY", color: warn),
      [The *order* in which nodes appear in the array directly determines *spatial locality* during DFS traversal.]
    )
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 3 — BT Structure
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[Behavior Trees — Quick Primer]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "Two node types", color: accent)[
      #set text(size: 15pt)
      *Control nodes* (internal) — define policy:
      #v(4pt)
      #bullet[*Sequence* ①②③: run children left→right; stop on first FAILURE]
      #v(2pt)
      #bullet[*Fallback* ①②③: run children left→right; stop on first SUCCESS]
      #v(8pt)
      *Leaf nodes* — actual work:
      #v(4pt)
      #bullet[*Action*: executes something; returns SUCCESS / FAILURE / RUNNING]
      #v(2pt)
      #bullet[*Condition*: evaluates a predicate; no side-effects]
    ]
    #card(title: "5-node example (BTreeFy)", color: accent2)[
      #set text(size: 14pt, font: "Fira Code")
      ```
      Fallback [idx 0]
      ├── Sequence  [idx 1]
      │   ├── Action A [idx 2]
      │   └── Action B [idx 3]
      └── Action C  [idx 4]
      ```
      #set text(size: 14pt, font: "Libertinus Serif")
      #v(6pt)
      In BTreeFy each node stores *three integer indices*:
      `parent`, `child`, `sibling` — pointers into the flat C array.
    ]
  ]
  #v(8pt)
  #card(color: warn)[
    #set text(size: 15pt)
    #grid(columns: (auto, 1fr), gutter: 10pt,
      badge("KEY", color: warn),
      [The order in which nodes appear in the flat C array directly determines *spatial locality* during execution.]
    )
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 4 — DFS Execution
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[BT Execution — The DFS Tick]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "Node access order (one tick)", color: accent)[
      #set text(size: 14pt, font: "Fira Code")
      ```
      1. load [0]  ← Fallback: enter
      2. load [1]  ← Sequence: enter
      3. load [2]  ← Action A → SUCCESS
      4. load [1]  ← Sequence: re-entered!
      5. load [3]  ← Action B → SUCCESS
      6. load [1]  ← Sequence: re-entered!
         → returns SUCCESS
      7. load [0]  ← Fallback: re-entered!
         → done
      ```
    ]
    #card(title: "The locality problem", color: accent2)[
      #set text(size: 15pt)
      Access sequence as array indices:
      #v(6pt)
      $ [0, 1, 2, #text(fill: warn)[1], 3, #text(fill: warn)[1], #text(fill: warn)[0]] $
      #v(8pt)
      #bullet[Control nodes are *re-accessed repeatedly*]
      #v(4pt)
      #bullet[Each access loads the node's cache line]
      #v(4pt)
      #bullet[If node and its *child or sibling are far in the array* → cache eviction → *reload cost*]
    ]
  ]
  #v(8pt)
  #card(color: border)[
    #set text(size: 15pt)
    *Insight:* the CPU pays for distance — not just traversal. Nodes connected by an edge should sit *close together in the array*.
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 5 — Toy Example: Layout Comparison
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[Toy Example — Why Layout Matters]
  #set text(size: 14pt)
  Graph edges — LCRS: collect all non-null `parent`, `child`, `sibling` fields across every node:
  $E = {(0,1),(0,4),(1,2),(1,3),(1,4),(2,3)}$ — *6 edges*, all weights $W=1$
  #v(8pt)
  #grid(columns: (1fr, 1fr), gutter: 14pt)[
    #card(title: "Layout A — DFS pre-order (parser output)", color: warn)[
      #set text(size: 14pt, font: "Fira Code")
      Array: [F, S, A, B, C]  →  y = [0, 1, 2, 3, 4]
      #set text(size: 14pt, font: "Libertinus Serif")
      #v(6pt)
      $C_A = |0-1|+|0-4|+|1-2|+|1-3|+|1-4|+|2-3|$
      #v(4pt)
      $quad = 1+4+1+2+3+1 = #text(fill: warn, weight: "bold")[12]$
      #v(6pt)
      $S$ has degree 4 but sits at index 1 — large distances to $C$ (dist 3) and $B$ (dist 2).
    ]
    #card(title: "Layout B — optimised  ✦ provably optimal", color: accent2)[
      #set text(size: 14pt, font: "Fira Code")
      Array: [C, F, S, A, B]  →  y = [0, 1, 2, 3, 4]
      #set text(size: 14pt, font: "Libertinus Serif")
      #v(6pt)
      $C_B = |1-2|+|1-0|+|2-3|+|2-4|+|2-0|+|3-4|$
      #v(4pt)
      $quad = 1+1+1+2+2+1 = #text(fill: accent2, weight: "bold")[8]$
      #v(6pt)
      $S$ centred at index 2; pairs $C$–$F$ and $A$–$B$ are adjacent.
    ]
  ]
  #v(6pt)
  #card(color: accent)[
    #grid(columns: (auto, 1fr), gutter: 10pt,
      badge("RESULT"),
      [Layout B achieves a *33% cost reduction*. Cost 8 is *provably optimal*: $S$ has degree 4, so $>=2$ of its edges must span $>=2$ slots, giving lower bound $=8$.]
    )
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 6 — Graph Model
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[Modelling the BT as a Graph $G = (V, E, W)$]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "Vertex set", color: accent)[
      #set text(size: 15pt)
      $V = {0, 1, dots.h, n-1}$ — one vertex per BT node
      #v(8pt)
      Directly the index of the node in the `btf_node[]` array.
    ]
    #card(title: "Edge set", color: accent2)[
      #set text(size: 15pt)
      For each node $i$, add an undirected edge for each non-null pointer field:
      #v(6pt)
      #bullet[$(i, " node"."parent")$ if `parent` ≠ NULL]
      #v(2pt)
      #bullet[$(i, " node"."child")$ if `child` ≠ NULL]
      #v(2pt)
      #bullet[$(i, " node"."sibling")$ if `sibling` ≠ NULL]
      #v(6pt)
      Duplicates removed. Result: $|E| = O(n)$ for a tree.
    ]
  ]
  #v(10pt)
  #card(title: "Edge weight", color: border)[
    #set text(size: 15pt)
    $W_(i j)$ = *traversal frequency* of edge $(i,j)$ — how many times per tick the CPU moves between nodes $i$ and $j$. Uniform $W=1$ for the baseline (Week 1); profile-guided weights for later phases.
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 7 — MinLA Problem Statement
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[Minimum Linear Arrangement (MinLA)]
  #set text(size: 15pt)
  #card(title: "Problem statement", color: accent)[
    Find a bijection $y : V -> {0, 1, dots.h, n-1}$ that minimises:
    #v(8pt)
    $ min_(y) quad sum_((i,j) in E) W_(i j) dot |y_i - y_j| $
    #v(8pt)
    subject to: $y_i eq.not y_j quad forall i eq.not j$ #h(1fr) *(all slots distinct)*
  ]
  #v(10pt)
  #grid(columns: (1fr, 1fr, 1fr), gutter: 12pt)[
    #card(title: [$y_i$], color: accent2)[
      Memory slot assigned to node $i$. The *decision variable*: what position in the C array does node $i$ occupy?
    ]
    #card(title: [$|y_i - y_j|$], color: accent2)[
      *Distance* between nodes $i$ and $j$ in the array. Proxy for cache line separation.
    ]
    #card(title: [$W_(i j)$], color: accent2)[
      *Weight* of edge $(i,j)$ — how often the CPU traverses this link. High weight → must be close.
    ]
  ]
  #v(8pt)
  #card(color: warn)[
    #grid(columns: (auto, 1fr), gutter: 10pt,
      badge("COMPLEXITY", color: warn),
      [MinLA is *NP-Hard* for general graphs (Garey & Johnson, 1979). BT graphs are trees — MinLA on trees is polynomial, but weighted and with cache-model extensions it re-introduces hardness.]
    )
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 8 — MILP: From Abs Value to Linear Constraints
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[MILP Formulation — Linearising the Problem]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "Step 1 — Eliminate the absolute value", color: accent)[
      #set text(size: 14pt)
      Replace $|y_i - y_j|$ with auxiliary variable $d_(i j) >= 0$:
      #v(6pt)
      $ d_(i j) >= y_i - y_j quad forall (i,j) in E $
      $ d_(i j) >= y_j - y_i quad forall (i,j) in E $
      #v(6pt)
      A minimising solver will drive $d_(i j)$ to *exactly* $|y_i - y_j|$.
      Objective becomes: $min sum W_(i j) d_(i j)$ — now *linear*.
    ]
    #card(title: "Step 2 — Enforce unique slots (Big-M)", color: accent2)[
      #set text(size: 14pt)
      Introduce binary $z_(i j) in {0,1}$ encoding which of $i,j$ comes first:
      #v(6pt)
      $ y_i - y_j >= 1 - M z_(i j) quad forall i eq.not j $
      $ y_j - y_i >= 1 - M(1 - z_(i j)) quad forall i eq.not j $
      #v(6pt)
      If $z_(i j)=0$: forces $y_j - y_i >= 1$ #h(2pt) (i.e. $i$ comes before $j$).
      If $z_(i j)=1$: forces $y_i - y_j >= 1$ #h(2pt) (i.e. $j$ before $i$).
      $M = n$ suffices as Big-M.
    ]
  ]
  #v(8pt)
  #card(color: warn)[
    #set text(size: 14pt)
    #grid(columns: (auto, 1fr), gutter: 10pt,
      badge("COMPLEXITY", color: warn),
      [$O(n)$ continuous vars ($y_i, d_(i j)$) + $O(n^2)$ binaries ($z_(i j)$). Binary count is the *exponential bottleneck* — motivates heuristics for large $n$.]
    )
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 6 — Scaling Wall
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[The Exponential Wall (Week 2)]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "Why MILP alone is not enough", color: warn)[
      #set text(size: 15pt)
      #bullet[No-overlap constraints introduce $O(n^2)$ binary variables $z_(i j)$]
      #v(4pt)
      #bullet[Branch-and-bound search space grows *exponentially* with $n$]
      #v(4pt)
      #bullet[CBC hits intractability near $n approx 15$–$20$]
      #v(4pt)
      #bullet[Real BTs: 20–200 nodes → *MILP alone infeasible*]
    ]
    #card(title: "Week 2 deliverable", color: accent)[
      #set text(size: 15pt)
      Empirical scaling study:
      #v(6pt)
      #bullet[$n in {5, 8, 10, 12, 15, 20}$, 5 random BTs each]
      #v(4pt)
      #bullet[Record: solver time, optimality gap at timeout, memory]
      #v(4pt)
      #bullet[Plot *solver time vs. $n$* to locate the exact failure point]
      #v(4pt)
      #bullet[This *motivates heuristics* — the core scientific contribution]
    ]
  ]
  #v(10pt)
  #align(center)[
    #text(fill: muted, size: 14pt)[Reference: Petit (2011) — MinLA benchmark experiments]
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 7 — Heuristic: Simulated Annealing
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[Heuristic Solution: Simulated Annealing (Week 3)]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "Algorithm Design", color: accent)[
      #set text(size: 15pt)
      #bullet[*Neighbourhood*: swap positions $y_a <-> y_b$ for random pair $(a,b)$]
      #v(4pt)
      #bullet[*Cooling*: geometric schedule $T_(k+1) = alpha dot T_k$, $alpha = 0.995$]
      #v(4pt)
      #bullet[*Stop*: $T < T_min = 10^(-4)$ or iteration budget]
      #v(4pt)
      #bullet[*Incremental cost*: only recompute edges incident to swapped nodes — $O(k)$ per step]
    ]
    #card(title: "Initialisation: Spectral Sequencing", color: accent2)[
      #set text(size: 15pt)
      #bullet[Compute *Fiedler vector* (2nd eigenvector of graph Laplacian $L$)]
      #v(4pt)
      #bullet[Sort nodes by Fiedler value → initial permutation]
      #v(4pt)
      #bullet[Proven to approximate MinLA (Pettis & Hansen, 1990 — spatial locality principle)]
      #v(4pt)
      #bullet[Gives SA a *much warmer start* than random]
    ]
  ]
  #v(8pt)
  #card(color: warn)[
    #set text(size: 14pt)
    #grid(columns: (auto, 1fr), gutter: 10pt,
      badge("TUNING", color: warn),
      [Sensitivity analysis on $alpha in {0.99, 0.995, 0.999}$ — must be included in Week 3 to justify the final schedule choice.]
    )
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 8 — Validation
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[Validation: Optimality Gap (Week 4)]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "Optimality Gap metric", color: accent)[
      #set text(size: 15pt)
      $ "Gap"(%) = (C_"SA" - C_"MILP") / C_"MILP" times 100 $
      #v(8pt)
      - Compare SA vs MILP on *all instances where MILP terminates* (time limit 5 min)
      - Target: Gap $< 10%$ on benchmark BTs
    ]
    #card(title: "Cache simulation", color: accent2)[
      #set text(size: 15pt)
      #bullet[Translate layout permutation to *memory access sequence* under DFS]
      #v(4pt)
      #bullet[Count misses using a *direct-mapped cache model* (parameterised by line size)]
      #v(4pt)
      #bullet[Validate: lower MinLA cost $=>$ fewer simulated cache misses]
    ]
  ]
  #v(10pt)
  #card(title: "Parser extension", color: border)[
    #set text(size: 15pt)
    `btf_groot_parser.py --layout <permutation>` — re-emits the `btf_node` C array *in the optimised slot order*, closing the loop from algorithm to deployed code.
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 9 — 30-Day Sprint
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[30-Day Implementation Sprint]
  #set text(size: 15pt)
  #grid(columns: (auto, 1fr, 1fr), gutter: 8pt,
    ..([*Week*], [*Milestone*], [*Critical Deliverable*],
       [#badge("W1")], [MILP Formulation], [Correct $A x <= b$ structure; optimal layout for $n <= 12$],
       [#badge("W2", color: warn)], [Scaling Analysis], [Empirical "Exponential Wall" plot; documented failure point],
       [#badge("W3", color: accent2)], [SA Heuristic], [SA with spectral init; $alpha$ sensitivity analysis],
       [#badge("W4", color: muted)], [Validation], [Optimality Gap table; cache miss simulation; parser extension],
    )
  )
  #v(14pt)
  #card(color: accent)[
    #set text(size: 15pt)
    #grid(columns: (auto, 1fr), gutter: 10pt,
      badge("STACK"),
      [*Python* (PuLP/CBC) for MILP · *NumPy/SciPy* for spectral init · *Matplotlib* for plots · *C/BTreeFy* for end-to-end validation]
    )
  ]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 10 — Expected Contributions
// ─────────────────────────────────────────────────────────────────────────────
#slide[
  #slide-title[Expected Contributions]
  #grid(columns: (1fr, 1fr), gutter: 18pt)[
    #card(title: "Scientific", color: accent)[
      #set text(size: 15pt)
      #bullet[First application of *MinLA* to embedded Behavior Tree layout]
      #v(4pt)
      #bullet[Empirical characterisation of the *Exponential Wall* for BT-structured graphs]
      #v(4pt)
      #bullet[Quantified *Optimality Gap* for spectral-seeded SA on BT graphs]
      #v(4pt)
      #bullet[Formal link between MinLA cost and *instruction cache miss count*]
    ]
    #card(title: "Practical", color: accent2)[
      #set text(size: 15pt)
      #bullet[Extended `btf_groot_parser.py` → *optimised C array output*]
      #v(4pt)
      #bullet[Drop-in layout optimiser for any *BTreeFy* application]
      #v(4pt)
      #bullet[Reproducible benchmarks on `porta_automatica` and `AssetTracking` BT models]
      #v(4pt)
      #bullet[Open-source under the existing BTreeFy repository]
    ]
  ]
  #v(14pt)
  #align(center)[
    #text(fill: muted, size: 14pt)[
      Pettis & Hansen (1990) · Petit (2011) · Garey & Johnson (1979)
    ]
  ]
]
