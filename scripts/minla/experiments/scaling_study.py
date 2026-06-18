"""
scaling_study.py — Phase 2: Exponential Wall Characterization
=============================================================
Empirically maps where CBC becomes intractable as n grows.

Protocol
--------
  - n in {5, 8, 10, 12, 15, 20}
  - 5 random BT instances per n (seeds 0..4), 30 runs total
  - Per run: solve_time (s), milp_cost, status, binary_count = n*(n-1)
  - Time limit: 5 min per instance
  - Outputs:
      results/scaling_raw.csv
      docs/figures/scaling_wall.png   (solver time vs. n, log-scale y-axis)

Run from repo root:
  python3 scripts/minla/experiments/scaling_study.py
"""

import os
import sys
import csv
import time

# ── Path setup ────────────────────────────────────────────────────────────────
_SCRIPTS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_REPO    = os.path.dirname(_SCRIPTS)
for p in [_SCRIPTS, _REPO]:
    if p not in sys.path:
        sys.path.insert(0, p)

from minla.random_bt   import generate
from minla.milp_solver import solve

# ── Output paths ──────────────────────────────────────────────────────────────
_HERE        = os.path.dirname(os.path.abspath(__file__))
_MINLA_DIR   = os.path.dirname(_HERE)
_RESULTS_DIR = os.path.join(_MINLA_DIR, "results")
_FIGURES_DIR = os.path.join(_MINLA_DIR, "docs", "figures")

os.makedirs(_RESULTS_DIR, exist_ok=True)
os.makedirs(_FIGURES_DIR, exist_ok=True)

CSV_PATH = os.path.join(_RESULTS_DIR, "scaling_raw.csv")
FIG_PATH = os.path.join(_FIGURES_DIR, "scaling_wall.png")

# ── Experiment parameters ─────────────────────────────────────────────────────
N_VALUES    = [5, 8, 10, 12, 15, 20]
N_SEEDS     = 5          # instances per n
TIME_LIMIT  = 300        # seconds per instance (5 min)

FIELDNAMES = ["n", "seed", "status", "solve_time", "milp_cost", "binary_vars"]


def run_experiment() -> list[dict]:
    rows = []
    total = len(N_VALUES) * N_SEEDS
    done  = 0

    with open(CSV_PATH, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=FIELDNAMES)
        writer.writeheader()

        for n in N_VALUES:
            times_for_n = []
            for seed in range(N_SEEDS):
                done += 1
                g = generate(n, seed=seed)
                binary_count = n * (n - 1)

                print(
                    f"[{done:2d}/{total}] n={n:2d} seed={seed} "
                    f"binary_vars={binary_count} ...",
                    end=" ", flush=True,
                )

                result = solve(g, time_limit_s=TIME_LIMIT, msg=False)

                print(
                    f"{result.status:<10} cost={result.cost:6.1f} "
                    f"time={result.solve_time:7.2f}s"
                )

                row = {
                    "n":           n,
                    "seed":        seed,
                    "status":      result.status,
                    "solve_time":  round(result.solve_time, 4),
                    "milp_cost":   result.cost,
                    "binary_vars": binary_count,
                }
                writer.writerow(row)
                f.flush()
                rows.append(row)
                times_for_n.append(result.solve_time)

            avg = sum(times_for_n) / len(times_for_n)
            print(f"  → n={n} avg_time={avg:.2f}s\n")

    print(f"Raw results saved to: {CSV_PATH}")
    return rows


def plot_results(rows: list[dict]) -> None:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np

    # Aggregate: mean and max solve time per n
    from collections import defaultdict
    times_by_n: dict = defaultdict(list)
    for r in rows:
        times_by_n[r["n"]].append(r["solve_time"])

    ns      = sorted(times_by_n.keys())
    means   = [float(np.mean(times_by_n[n])) for n in ns]
    maxes   = [float(np.max(times_by_n[n]))  for n in ns]

    # ── Style ─────────────────────────────────────────────────────────────
    BG      = "#0d1117"
    SURFACE = "#161b22"
    BORDER  = "#30363d"
    ACCENT  = "#58a6ff"
    WARN    = "#d29922"
    GREEN   = "#3fb950"
    TEXT    = "#e6edf3"
    MUTED   = "#8b949e"

    fig, ax = plt.subplots(figsize=(9, 5.5))
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(SURFACE)
    for spine in ax.spines.values():
        spine.set_edgecolor(BORDER)
    ax.tick_params(colors=TEXT)
    ax.xaxis.label.set_color(TEXT)
    ax.yaxis.label.set_color(TEXT)
    ax.title.set_color(TEXT)

    ax.plot(ns, means, "o-", color=ACCENT,  linewidth=2.2,
            markersize=7, label="Mean solve time")
    ax.fill_between(ns, means, maxes, alpha=0.18, color=ACCENT)
    ax.plot(ns, maxes,  "s--", color=WARN, linewidth=1.5,
            markersize=6, label="Max solve time")

    # Mark 5-min timeout line
    ax.axhline(y=TIME_LIMIT, color=WARN, linestyle=":", linewidth=1.4, alpha=0.8)
    ax.text(ns[0] + 0.2, TIME_LIMIT * 1.05, "5-min timeout",
            color=WARN, fontsize=9)

    # Shade region beyond first timeout
    timeout_ns = [n for n, t in zip(ns, maxes) if t >= TIME_LIMIT - 1]
    if timeout_ns:
        ax.axvspan(timeout_ns[0] - 0.5, ns[-1] + 0.5,
                   alpha=0.07, color=WARN, label="Intractable region")

    ax.set_yscale("log")
    ax.set_xlabel("Number of BT nodes  (n)", fontsize=11)
    ax.set_ylabel("Solver wall-clock time (s) — log scale", fontsize=11)
    ax.set_title("MILP Scaling Study: Exponential Wall\n"
                 "CBC / PuLP · 5 random BTs per n · 5-min time limit",
                 fontsize=12, pad=12)
    ax.set_xticks(ns)
    ax.legend(framealpha=0.15, labelcolor=TEXT,
              facecolor=SURFACE, edgecolor=BORDER, fontsize=9)

    ax.grid(True, which="both", color=BORDER, linewidth=0.5, alpha=0.6)

    fig.tight_layout()
    fig.savefig(FIG_PATH, dpi=150, facecolor=BG)
    plt.close(fig)
    print(f"Plot saved to: {FIG_PATH}")


if __name__ == "__main__":
    print("=" * 60)
    print("Phase 2 — MILP Scaling Study")
    print(f"n values : {N_VALUES}")
    print(f"Seeds    : 0..{N_SEEDS - 1}")
    print(f"Timeout  : {TIME_LIMIT}s per instance")
    print("=" * 60 + "\n")

    t0   = time.perf_counter()
    rows = run_experiment()
    plot_results(rows)
    total_time = time.perf_counter() - t0

    print(f"\nTotal wall time: {total_time:.1f}s")
    print("Phase 2 complete.")
