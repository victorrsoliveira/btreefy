"""
sa_sensitivity.py — Phase 3: α Cooling Schedule Sensitivity Analysis
=====================================================================
Sweeps α ∈ {0.99, 0.995, 0.999} across a set of benchmark instances and
records convergence curves (MinLA cost vs. iteration) for each combination.

Benchmark instances
-------------------
  - Random BTs: n ∈ {5, 12, 20}, seed=0 (covers toy, MILP-verified, intractable)
  - Real BTs: porta_automatica (main_new) and AssetTracking (Overview) — if
    parseable; skipped gracefully if XML subtrees (SubTree nodes) block parsing.

Outputs
-------
  docs/figures/sa_sensitivity.png  — 3×2 grid of convergence plots
  results/sa_sensitivity_summary.csv — final cost per (instance, α) combination

Run from repo root:
  python3 scripts/minla/experiments/sa_sensitivity.py
"""

import os
import sys
import csv

# ── Path setup ────────────────────────────────────────────────────────────────
_HERE        = os.path.dirname(os.path.abspath(__file__))   # .../experiments/
_MINLA_PKG   = os.path.dirname(_HERE)                        # .../minla/
_SCRIPTS     = os.path.dirname(_MINLA_PKG)                   # .../scripts/
_REPO        = os.path.dirname(_SCRIPTS)                     # .../btreefy-repo/
for p in [_SCRIPTS, _REPO]:
    if p not in sys.path:
        sys.path.insert(0, p)

from minla.random_bt   import generate
from minla.bt_graph    import from_xml, BTGraph
from minla.sa_solver   import solve, solve_random_init, fiedler_permutation
from minla.cost        import minla_cost

# ── Output paths ──────────────────────────────────────────────────────────────
_RESULTS_DIR = os.path.join(_MINLA_PKG, "results")
_FIGURES_DIR = os.path.join(_MINLA_PKG, "docs", "figures")

os.makedirs(_RESULTS_DIR, exist_ok=True)
os.makedirs(_FIGURES_DIR, exist_ok=True)

FIG_PATH     = os.path.join(_FIGURES_DIR, "sa_sensitivity.png")
FIG_INIT_PATH = os.path.join(_FIGURES_DIR, "sa_fiedler_vs_random.png")
CSV_PATH     = os.path.join(_RESULTS_DIR, "sa_sensitivity_summary.csv")

# ── Parameters ────────────────────────────────────────────────────────────────
ALPHAS  = [0.99, 0.995, 0.999]
SA_SEED = 42

# ── Instance registry ─────────────────────────────────────────────────────────

def build_instances() -> list[dict]:
    """Return list of {name, graph, milp_cost_or_None} dicts."""
    instances = []

    # Random BTs (matching scaling study seeds)
    for n, seed, milp_cost in [
        (5,  0, 8.0),    # toy example — MILP optimal known
        (12, 0, 31.0),   # MILP-verified optimal from scaling study
        (20, 0, None),   # MILP intractable
    ]:
        g = generate(n, seed=seed)
        instances.append({
            "name":      f"random_n{n}_s{seed}",
            "label":     f"Random BT  n={n}",
            "graph":     g,
            "milp_cost": milp_cost,
        })

    # Real BTs — try parsing; skip on failure (SubTree nodes unsupported)
    real_bts = [
        (os.path.join(_REPO, "models", "porta_automatica.xml"), "main_new",    "PortaAutomatica"),
        (os.path.join(_REPO, "models", "AssetTracking-App.xml"), "Monitoring", "AssetTracking"),
    ]
    for model_path, tree_id, label in real_bts:
        try:
            g = from_xml(model_path, tree_id)
            instances.append({
                "name":      f"real_{label.lower()}",
                "label":     f"{label}  n={g.n}",
                "graph":     g,
                "milp_cost": None,
            })
            print(f"  Loaded real BT: {label} (n={g.n}, |E|={len(g.edges)})")
        except Exception as e:
            print(f"  Skipping {label}: {e}")

    return instances


# ── Run sweep ─────────────────────────────────────────────────────────────────

def run_sweep(instances: list[dict]) -> list[dict]:
    summary_rows = []

    for inst in instances:
        g    = inst["graph"]
        name = inst["name"]
        print(f"\n[{name}]  n={g.n}  |E|={len(g.edges)}")

        # Fiedler init quality (once per instance)
        fiedler_perm = fiedler_permutation(g)
        fiedler_cost = minla_cost(g, fiedler_perm)
        dfs_perm     = list(range(g.n))          # DFS pre-order baseline
        dfs_cost     = minla_cost(g, dfs_perm)
        print(f"  DFS baseline cost : {dfs_cost:.1f}")
        print(f"  Fiedler init cost : {fiedler_cost:.1f}  "
              f"({(fiedler_cost - dfs_cost) / dfs_cost * 100:+.1f}% vs DFS)")
        if inst["milp_cost"]:
            milp = inst["milp_cost"]
            print(f"  MILP optimal cost : {milp:.1f}")

        # SA sweep over alpha values
        for alpha in ALPHAS:
            result = solve(g, alpha=alpha, seed=SA_SEED)
            gap_str = ""
            if inst["milp_cost"]:
                gap = (result.cost - inst["milp_cost"]) / inst["milp_cost"] * 100
                gap_str = f"  gap={gap:+.1f}%"
            print(
                f"  α={alpha}  cost={result.cost:.1f}  "
                f"iters={result.iterations}{gap_str}"
            )
            inst.setdefault("results", {})[alpha] = result

            row = {
                "instance":  name,
                "n":         g.n,
                "alpha":     alpha,
                "dfs_cost":  dfs_cost,
                "fiedler_init_cost": fiedler_cost,
                "sa_cost":   result.cost,
                "iters":     result.iterations,
                "milp_cost": inst["milp_cost"] if inst["milp_cost"] else "",
                "gap_pct":   round((result.cost - inst["milp_cost"]) /
                                   inst["milp_cost"] * 100, 2)
                             if inst["milp_cost"] else "",
            }
            summary_rows.append(row)

    return summary_rows


# ── Plot ──────────────────────────────────────────────────────────────────────

def plot_convergence(instances: list[dict]) -> None:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.gridspec as gridspec

    BG      = "#0d1117"
    SURFACE = "#161b22"
    BORDER  = "#30363d"
    ACCENT  = "#58a6ff"
    WARN    = "#d29922"
    GREEN   = "#3fb950"
    TEXT    = "#e6edf3"
    MUTED   = "#8b949e"
    ALPHA_COLORS = {0.99: WARN, 0.995: ACCENT, 0.999: GREEN}

    n_inst = len(instances)
    ncols = 3
    nrows = (n_inst + ncols - 1) // ncols

    fig, axes = plt.subplots(nrows, ncols,
                             figsize=(5.5 * ncols, 4 * nrows))
    fig.patch.set_facecolor(BG)
    axes = axes.flatten() if n_inst > 1 else [axes]

    for ax_idx, inst in enumerate(instances):
        ax = axes[ax_idx]
        ax.set_facecolor(SURFACE)
        for spine in ax.spines.values():
            spine.set_edgecolor(BORDER)
        ax.tick_params(colors=TEXT, labelsize=8)
        ax.xaxis.label.set_color(TEXT)
        ax.yaxis.label.set_color(TEXT)

        g    = inst["graph"]
        results = inst.get("results", {})

        # DFS baseline (horizontal reference)
        dfs_cost = minla_cost(g, list(range(g.n)))
        ax.axhline(dfs_cost, color=MUTED, linestyle=":", linewidth=1.2,
                   label=f"DFS baseline ({dfs_cost:.0f})", alpha=0.7)

        # MILP optimal if known
        if inst["milp_cost"]:
            ax.axhline(inst["milp_cost"], color=WARN, linestyle="--",
                       linewidth=1.2,
                       label=f"MILP optimal ({inst['milp_cost']:.0f})",
                       alpha=0.85)

        for alpha in ALPHAS:
            if alpha not in results:
                continue
            r = results[alpha]
            iters = [h[0] for h in r.history]
            costs = [h[1] for h in r.history]
            ax.plot(iters, costs, color=ALPHA_COLORS[alpha],
                    linewidth=1.6, label=f"α={alpha}  final={r.cost:.0f}")

        ax.set_title(inst["label"], color=TEXT, fontsize=10, pad=6)
        ax.set_xlabel("Iteration", fontsize=8)
        ax.set_ylabel("MinLA cost", fontsize=8)
        ax.legend(framealpha=0.15, labelcolor=TEXT, facecolor=SURFACE,
                  edgecolor=BORDER, fontsize=7)
        ax.grid(True, color=BORDER, linewidth=0.4, alpha=0.6)

    # Hide unused axes
    for ax in axes[n_inst:]:
        ax.set_visible(False)

    fig.suptitle("SA Cooling Schedule Sensitivity  ·  α ∈ {0.99, 0.995, 0.999}",
                 color=TEXT, fontsize=13, y=1.01)
    fig.tight_layout()
    fig.savefig(FIG_PATH, dpi=150, facecolor=BG, bbox_inches="tight")
    plt.close(fig)
    print(f"\nConvergence plot saved → {FIG_PATH}")


def plot_fiedler_vs_random(instances: list[dict]) -> None:
    """Bar chart: Fiedler init cost vs random init cost vs DFS baseline."""
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np

    BG      = "#0d1117"
    SURFACE = "#161b22"
    BORDER  = "#30363d"
    ACCENT  = "#58a6ff"
    WARN    = "#d29922"
    GREEN   = "#3fb950"
    TEXT    = "#e6edf3"
    MUTED   = "#8b949e"

    labels, dfs_vals, fiedler_vals, rand_vals, sa_vals = [], [], [], [], []

    for inst in instances:
        g    = inst["graph"]
        dfs  = minla_cost(g, list(range(g.n)))
        fied = minla_cost(g, fiedler_permutation(g))
        rand = solve_random_init(g, alpha=0.995, seed=SA_SEED).cost
        sa   = inst.get("results", {}).get(0.995)
        sa_c = sa.cost if sa else None

        labels.append(inst["label"])
        dfs_vals.append(dfs)
        fiedler_vals.append(fied)
        rand_vals.append(rand)
        sa_vals.append(sa_c if sa_c else 0)

    x = np.arange(len(labels))
    w = 0.2

    fig, ax = plt.subplots(figsize=(max(8, len(labels) * 2.2), 5))
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(SURFACE)
    for spine in ax.spines.values():
        spine.set_edgecolor(BORDER)
    ax.tick_params(colors=TEXT)

    ax.bar(x - 1.5*w, dfs_vals,     w, label="DFS baseline",   color=MUTED,   alpha=0.8)
    ax.bar(x - 0.5*w, fiedler_vals, w, label="Fiedler init",   color=ACCENT,  alpha=0.9)
    ax.bar(x + 0.5*w, rand_vals,    w, label="Random init+SA", color=WARN,    alpha=0.8)
    ax.bar(x + 1.5*w, sa_vals,      w, label="Fiedler+SA (α=0.995)", color=GREEN, alpha=0.9)

    ax.set_xticks(x)
    ax.set_xticklabels(labels, color=TEXT, fontsize=9, rotation=15, ha="right")
    ax.set_ylabel("MinLA cost", color=TEXT, fontsize=10)
    ax.set_title("Initialization Quality: DFS vs Fiedler vs SA",
                 color=TEXT, fontsize=12, pad=10)
    ax.legend(framealpha=0.15, labelcolor=TEXT, facecolor=SURFACE,
              edgecolor=BORDER, fontsize=9)
    ax.grid(True, axis="y", color=BORDER, linewidth=0.4, alpha=0.6)

    fig.tight_layout()
    fig.savefig(FIG_INIT_PATH, dpi=150, facecolor=BG)
    plt.close(fig)
    print(f"Init comparison plot saved → {FIG_INIT_PATH}")


# ── Entry point ───────────────────────────────────────────────────────────────

if __name__ == "__main__":
    import time

    print("=" * 60)
    print("Phase 3 — SA Sensitivity Analysis")
    print(f"α values : {ALPHAS}")
    print(f"SA seed  : {SA_SEED}")
    print("=" * 60)

    t0        = time.perf_counter()
    instances = build_instances()
    rows      = run_sweep(instances)
    plot_convergence(instances)
    plot_fiedler_vs_random(instances)

    # Write summary CSV
    fieldnames = ["instance", "n", "alpha", "dfs_cost",
                  "fiedler_init_cost", "sa_cost", "iters",
                  "milp_cost", "gap_pct"]
    with open(CSV_PATH, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    print(f"Summary CSV  → {CSV_PATH}")
    print(f"\nTotal wall time: {time.perf_counter() - t0:.1f}s")
    print("Phase 3 complete.")
