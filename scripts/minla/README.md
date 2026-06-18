# BT MinLA — Minimum Linear Arrangement for BTreeFy

Optimizes the memory layout of BTreeFy node arrays to minimize instruction
cache misses during DFS traversal by solving the Minimum Linear Arrangement
(MinLA) problem.

## Quick Start

```bash
# Install dependencies (from repo root)
pip install -r scripts/minla/requirements.txt

# Run P1 self-tests
cd scripts && python -m minla.milp_solver --selftest
cd scripts && python -m minla.cost

# Run experiments
python scripts/minla/experiments/scaling_study.py
python scripts/minla/experiments/sa_sensitivity.py
python scripts/minla/experiments/validation.py

# End-to-end pipeline (porta_automatica)
python scripts/minla/experiments/end_to_end.py \
    --model models/porta_automatica.xml --treename main_new

# Optimized C array output
python scripts/btf_layout_emitter.py \
    --model models/porta_automatica.xml \
    --treename main_new \
    --layout 4,0,1,2,3 \
    --source-output-dir src/generated \
    --include-output-dir include/generated
```

## Structure

```
minla/
├── bt_graph.py         Graph extraction from Groot XML (LCRS → G=(V,E,W))
├── cost.py             MinLA cost: full O(|E|) + incremental O(deg) delta
├── milp_solver.py      PuLP/CBC MILP formulation (exact, n≤12)
├── random_bt.py        Random BT generator for scaling experiments
├── sa_solver.py        Simulated Annealing + Fiedler spectral init
├── cache_sim.py        Direct-mapped cache miss simulator
├── experiments/
│   ├── scaling_study.py    P2: Exponential Wall characterization
│   ├── sa_sensitivity.py   P3: α cooling schedule sensitivity sweep
│   ├── validation.py       P4: Optimality Gap table + cache miss comparison
│   └── end_to_end.py       P4: Full XML→optimized-C pipeline
└── docs/
    ├── preamble.typ        Shared Typst theme
    ├── figures/            Auto-generated plots (embedded in reports)
    └── report_p{1-4}_*.typ/pdf   Phase reports
```

## Mathematical Background

**MinLA objective:**

  min_y  Σ_{(i,j)∈E}  W_ij · |y_i − y_j|

where y: V→{0,…,n−1} is a bijection (node→slot). Edges E are derived from
the LCRS `parent`/`child`/`sibling` pointer fields.

**Complexity:** NP-Hard in general (Garey & Johnson 1979). Polynomial on
unweighted trees, but weighted/cache-model extensions re-introduce hardness.

## References

- Garey & Johnson (1979) — MinLA NP-Hardness
- Pettis & Hansen (1990) — Spatial locality, Fiedler vector initialization
- Petit (2011) — MinLA benchmark experiments and scaling characterization
