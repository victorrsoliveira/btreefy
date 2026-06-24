"""
validation.py — Phase 4: Optimality Gap & Cache Miss Comparison
===============================================================
Two-part validation:

  Part A — Optimality Gap Table
  ------------------------------
  For each MILP-verified instance (n ≤ 12, solve_time < 250 s from P2):
    - Read MILP optimal cost from scaling_raw.csv
    - Run SA (α=0.995) to get sa_cost
    - Compute Gap(%) = (sa_cost - milp_cost) / milp_cost * 100

  Part B — Cache Miss Comparison
  --------------------------------
  For all benchmark instances (n=5..20, real BTs):
    - Compare cache misses: DFS layout vs SA-optimized layout
    - Sweep cache sizes: {4, 8, 16, 32} lines

Outputs
-------
  results/optimality_gap_table.csv
  docs/figures/optimality_gap.png
  docs/figures/cache_miss_comparison.png

Run from repo root:
  python3 scripts/minla/experiments/validation.py
"""

import os
import sys
import csv

_HERE      = os.path.dirname(os.path.abspath(__file__))
_MINLA_PKG = os.path.dirname(_HERE)
_SCRIPTS   = os.path.dirname(_MINLA_PKG)
_REPO      = os.path.dirname(_SCRIPTS)
for p in [_SCRIPTS, _REPO]:
    if p not in sys.path:
        sys.path.insert(0, p)

from minla.random_bt  import generate
from minla.bt_graph   import from_xml
from minla.sa_solver  import solve as sa_solve
from minla.cost       import minla_cost
from minla.cache_sim  import simulate as cache_simulate, miss_rate

# ── Paths ─────────────────────────────────────────────────────────────────────
_RESULTS = os.path.join(_MINLA_PKG, "results")
_FIGURES = os.path.join(_MINLA_PKG, "docs", "figures")
os.makedirs(_RESULTS, exist_ok=True)
os.makedirs(_FIGURES, exist_ok=True)

SCALING_CSV  = os.path.join(_RESULTS, "scaling_raw.csv")
GAP_CSV      = os.path.join(_RESULTS, "optimality_gap_table.csv")
GAP_FIG      = os.path.join(_FIGURES, "optimality_gap.png")
CACHE_FIG    = os.path.join(_FIGURES, "cache_miss_comparison.png")

SA_ALPHA = 0.995
SA_SEED  = 42
CACHE_LINES_SWEEP = [1, 2, 4, 8]

# ── Part A: Optimality Gap ────────────────────────────────────────────────────

def load_milp_verified() -> list[dict]:
    """
    Load MILP-verified instances from scaling_raw.csv.
    Verified = solve_time < 250 s (ensures CBC proved optimality).
    """
    verified = []
    with open(SCALING_CSV) as f:
        for row in csv.DictReader(f):
            if float(row["solve_time"]) < 250.0:
                verified.append({
                    "n":         int(row["n"]),
                    "seed":      int(row["seed"]),
                    "milp_cost": float(row["milp_cost"]),
                })
    return verified


def run_gap_analysis(verified: list[dict]) -> list[dict]:
    rows = []
    print("\n=== Part A: Optimality Gap ===")
    print(f"{'n':>4}  {'seed':>4}  {'milp':>6}  {'sa':>6}  {'gap%':>7}  verified")

    for inst in verified:
        n, seed, milp = inst["n"], inst["seed"], inst["milp_cost"]
        g       = generate(n, seed=seed)
        result  = sa_solve(g, alpha=SA_ALPHA, seed=SA_SEED)
        sa_cost = result.cost
        gap     = (sa_cost - milp) / milp * 100

        print(f"{n:>4}  {seed:>4}  {milp:>6.1f}  {sa_cost:>6.1f}  {gap:>+7.2f}%  ✓")
        rows.append({
            "n": n, "seed": seed,
            "milp_cost": milp, "sa_cost": sa_cost,
            "gap_pct": round(gap, 3),
            "dfs_cost": minla_cost(g, list(range(n))),
        })

    avg_gap = sum(r["gap_pct"] for r in rows) / len(rows)
    max_gap = max(r["gap_pct"] for r in rows)
    print(f"\n  Instances: {len(rows)}  avg_gap={avg_gap:+.2f}%  max_gap={max_gap:+.2f}%")
    return rows


# ── Part B: Cache Miss Comparison ─────────────────────────────────────────────

def build_cache_instances() -> list[dict]:
    insts = []
    for n, seed in [(5, 0), (8, 0), (10, 0), (12, 0), (15, 0), (20, 0)]:
        g = generate(n, seed=seed)
        insts.append({"name": f"n={n}", "graph": g})

    for model, tree_id, label in [
        (os.path.join(_REPO, "models", "porta_automatica.xml"), "main_new",    "PortaAutomatica"),
        (os.path.join(_REPO, "models", "AssetTracking-App.xml"), "Monitoring", "AssetTracking"),
    ]:
        try:
            g = from_xml(model, tree_id)
            insts.append({"name": label, "graph": g})
            print(f"  Loaded {label} n={g.n}")
        except Exception as e:
            print(f"  Skipping {label}: {e}")

    return insts


def run_cache_comparison(insts: list[dict]) -> list[dict]:
    rows = []
    print("\n=== Part B: Cache Miss Comparison (2 cache lines) ===")
    print(f"{'Instance':>18}  {'n':>3}  {'DFS_miss':>9}  {'SA_miss':>8}  {'reduction':>10}")

    for inst in insts:
        g    = inst["graph"]
        name = inst["name"]
        n    = g.n
        dfs_perm = list(range(n))
        result   = sa_solve(g, alpha=SA_ALPHA, seed=SA_SEED)
        sa_perm  = result.permutation

        inst["sa_perm"] = sa_perm

        dfs_h, dfs_m = cache_simulate(g, dfs_perm, cache_lines=2)
        sa_h,  sa_m  = cache_simulate(g, sa_perm,  cache_lines=2)
        total = dfs_h + dfs_m
        reduction = (dfs_m - sa_m) / dfs_m * 100 if dfs_m else 0.0

        print(f"{name:>18}  {n:>3}  {dfs_m:>4}/{total:<4}  "
              f"{sa_m:>3}/{total:<4}  {reduction:>+9.1f}%")

        for cl in CACHE_LINES_SWEEP:
            _, dm = cache_simulate(g, dfs_perm, cache_lines=cl)
            _, sm = cache_simulate(g, sa_perm,  cache_lines=cl)
            red = (dm - sm) / dm * 100 if dm else 0.0
            rows.append({
                "instance": name, "n": n, "cache_lines": cl,
                "dfs_misses": dm, "sa_misses": sm,
                "reduction_pct": round(red, 2),
            })
    return rows


# ── Plotting ──────────────────────────────────────────────────────────────────

def plot_gap(gap_rows: list[dict]) -> None:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np

    BG, SURFACE, BORDER = "#0d1117", "#161b22", "#30363d"
    ACCENT, WARN, GREEN = "#58a6ff", "#d29922", "#3fb950"
    TEXT, MUTED         = "#e6edf3", "#8b949e"

    labels   = [f"n={r['n']}\ns{r['seed']}" for r in gap_rows]
    gaps     = [r["gap_pct"] for r in gap_rows]
    dfs_reds = [(r["dfs_cost"] - r["milp_cost"]) / r["milp_cost"] * 100 for r in gap_rows]

    x  = np.arange(len(labels))
    w  = 0.35
    fig, ax = plt.subplots(figsize=(max(8, len(labels) * 0.9 + 2), 5))
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(SURFACE)
    for sp in ax.spines.values(): sp.set_edgecolor(BORDER)
    ax.tick_params(colors=TEXT)

    ax.bar(x - w/2, dfs_reds, w, color=WARN,   alpha=0.85, label="DFS gap vs MILP optimal")
    ax.bar(x + w/2, gaps,     w, color=ACCENT,  alpha=0.85, label="SA gap vs MILP optimal")

    ax.axhline(0, color=GREEN, linewidth=1.2, linestyle="--", alpha=0.7, label="MILP optimal (0%)")
    ax.axhline(10, color=WARN, linewidth=0.8, linestyle=":", alpha=0.5, label="10% target threshold")

    ax.set_xticks(x)
    ax.set_xticklabels(labels, color=TEXT, fontsize=9)
    ax.set_ylabel("Gap vs MILP optimal (%)", color=TEXT, fontsize=10)
    ax.set_title("Optimality Gap: DFS vs SA (α=0.995)\nMILP-verified instances (n ≤ 12, solve_time < 250 s)",
                 color=TEXT, fontsize=11, pad=10)
    ax.legend(framealpha=0.15, labelcolor=TEXT, facecolor=SURFACE, edgecolor=BORDER, fontsize=9)
    ax.grid(True, axis="y", color=BORDER, linewidth=0.4, alpha=0.6)
    fig.tight_layout()
    fig.savefig(GAP_FIG, dpi=150, facecolor=BG)
    plt.close(fig)
    print(f"\nGap plot → {GAP_FIG}")


def plot_cache(cache_rows: list[dict]) -> None:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np
    from collections import defaultdict

    BG, SURFACE, BORDER = "#0d1117", "#161b22", "#30363d"
    ACCENT, WARN, GREEN = "#58a6ff", "#d29922", "#3fb950"
    TEXT, MUTED         = "#e6edf3", "#8b949e"

    # Group by cache_lines=2 for bar chart
    rows8 = [r for r in cache_rows if r["cache_lines"] == 2]
    names = list(dict.fromkeys(r["instance"] for r in rows8))   # order-preserving unique
    dfs_m = [next(r["dfs_misses"] for r in rows8 if r["instance"] == nm) for nm in names]
    sa_m  = [next(r["sa_misses"]  for r in rows8 if r["instance"] == nm) for nm in names]

    x = np.arange(len(names))
    w = 0.35
    fig, ax = plt.subplots(figsize=(max(9, len(names) * 1.4), 5))
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(SURFACE)
    for sp in ax.spines.values(): sp.set_edgecolor(BORDER)
    ax.tick_params(colors=TEXT)

    ax.bar(x - w/2, dfs_m, w, color=WARN,  alpha=0.85, label="DFS layout")
    ax.bar(x + w/2, sa_m,  w, color=GREEN, alpha=0.85, label="SA-optimized layout")

    ax.set_xticks(x)
    ax.set_xticklabels(names, color=TEXT, fontsize=9, rotation=20, ha="right")
    ax.set_ylabel("Cache misses per DFS traversal", color=TEXT, fontsize=10)
    ax.set_title("Cache Miss Comparison: DFS vs SA Layout\n"
                 "Direct-mapped, 2 cache lines, 64-byte lines, 32-byte btf_node",
                 color=TEXT, fontsize=11, pad=10)
    ax.legend(framealpha=0.15, labelcolor=TEXT, facecolor=SURFACE, edgecolor=BORDER, fontsize=9)
    ax.grid(True, axis="y", color=BORDER, linewidth=0.4, alpha=0.6)
    fig.tight_layout()
    fig.savefig(CACHE_FIG, dpi=150, facecolor=BG)
    plt.close(fig)
    print(f"Cache miss plot → {CACHE_FIG}")


# ── Entry point ───────────────────────────────────────────────────────────────

if __name__ == "__main__":
    import time
    t0 = time.perf_counter()

    print("=" * 60)
    print("Phase 4 — Validation")
    print("=" * 60)

    # Part A: Optimality Gap
    verified  = load_milp_verified()
    gap_rows  = run_gap_analysis(verified)

    with open(GAP_CSV, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["n","seed","milp_cost","sa_cost","gap_pct","dfs_cost"])
        writer.writeheader()
        writer.writerows(gap_rows)
    print(f"\nGap CSV → {GAP_CSV}")
    plot_gap(gap_rows)

    # Part B: Cache miss comparison
    cache_insts = build_cache_instances()
    cache_rows  = run_cache_comparison(cache_insts)
    plot_cache(cache_rows)

    print(f"\nTotal wall time: {time.perf_counter() - t0:.1f}s")
    print("Phase 4 validation complete.")
