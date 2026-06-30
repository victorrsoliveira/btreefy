"""
gen_figures.py — Generate clean presentation figures for the MinLA/SA slides.

Outputs (all to ../figures/):
  - scaling_wall.png       (already exists, regenerated clean)
  - sa_sensitivity.png     (already exists, regenerated clean)
  - optimality_gap.png     (new)

Style: minimal, data-focused, no decorative elements.
Run from: scripts/minla/docs/final_report/
  python3 gen_figures.py
"""

import csv
import math
from pathlib import Path
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# ── Paths ─────────────────────────────────────────────────────────────────────
HERE   = Path(__file__).parent
FIGS   = HERE.parent / "figures"
FIGS.mkdir(exist_ok=True)
RESULTS = HERE.parent.parent / "results"

# ── Shared style ──────────────────────────────────────────────────────────────
plt.rcParams.update({
    "font.family":      "sans-serif",
    "font.size":        11,
    "axes.spines.top":  False,
    "axes.spines.right":False,
    "axes.linewidth":   0.8,
    "grid.color":       "#dddddd",
    "grid.linewidth":   0.6,
    "figure.dpi":       150,
    "savefig.dpi":      150,
    "savefig.bbox":     "tight",
    "savefig.pad_inches": 0.15,
})

GRAY   = "#555555"
BLUE   = "#2166ac"
RED    = "#d6604d"
GREEN  = "#4dac26"
ORANGE = "#f4a582"

# ══════════════════════════════════════════════════════════════════════════════
# Figure 1 — Scaling Wall
# ══════════════════════════════════════════════════════════════════════════════
def fig_scaling_wall():
    rows = []
    with open(RESULTS / "scaling_raw.csv", newline="") as f:
        for r in csv.DictReader(f):
            rows.append({"n": int(r["n"]), "t": float(r["solve_time"])})

    by_n = defaultdict(list)
    for r in rows:
        by_n[r["n"]].append(r["t"])

    ns   = sorted(by_n)
    avg  = [np.mean(by_n[n]) for n in ns]
    mx   = [np.max(by_n[n])  for n in ns]

    fig, ax = plt.subplots(figsize=(6.5, 4))

    # timeout band
    ax.axhspan(250, 320, color="#f7e6e6", zorder=0)
    ax.axhline(300, color=RED, linewidth=0.9, linestyle="--", label="5 min timeout")

    ax.plot(ns, avg, marker="o", color=BLUE, linewidth=1.8,
            markersize=5, label="Avg solve time")
    ax.plot(ns, mx,  marker="s", color=GRAY, linewidth=1.2, linestyle="--",
            markersize=4, label="Max solve time")

    ax.set_yscale("log")
    ax.set_xlabel("Número de nós  $n$")
    ax.set_ylabel("Tempo (s) — escala log")
    ax.set_xticks(ns)
    ax.set_ylim(0.05, 400)
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(
        lambda v, _: f"{v:.0f}s" if v >= 1 else f"{v:.2f}s"
    ))
    ax.legend(frameon=False, fontsize=10)
    ax.grid(axis="y")

    # annotate failure zone
    ax.text(15, 330, "timeout", color=RED, fontsize=9, va="bottom")

    fig.tight_layout()
    out = FIGS / "scaling_wall.png"
    fig.savefig(out)
    plt.close(fig)
    print(f"  ✓ {out}")


# ══════════════════════════════════════════════════════════════════════════════
# Figure 2 — SA sensitivity (bar chart: DFS / Fiedler / α variants)
# ══════════════════════════════════════════════════════════════════════════════
def fig_sa_sensitivity():
    rows = []
    with open(RESULTS / "sa_sensitivity_summary.csv", newline="") as f:
        for r in csv.DictReader(f):
            rows.append({
                "instance": r["instance"],
                "n":        int(r["n"]),
                "alpha":    float(r["alpha"]),
                "dfs":      float(r["dfs_cost"]),
                "fiedler":  float(r["fiedler_init_cost"]),
                "sa":       float(r["sa_cost"]),
            })

    # pivot: one entry per (instance, alpha)
    instances = []
    seen = set()
    for r in rows:
        if r["instance"] not in seen:
            seen.add(r["instance"])
            instances.append(r["instance"])

    alphas = [0.99, 0.995, 0.999]
    alpha_colors = {0.99: "#6baed6", 0.995: BLUE, 0.999: "#08519c"}

    # For each instance grab DFS, Fiedler, and SA per alpha
    data = {}
    for r in rows:
        key = (r["instance"], r["alpha"])
        data[key] = r

    labels = [inst.replace("random_", "rand_").replace("real_", "") for inst in instances]
    x = np.arange(len(instances))
    n_alphas = len(alphas)
    w = 0.18

    fig, ax = plt.subplots(figsize=(8, 4.5))

    # DFS baseline
    dfs_vals = [data[(inst, 0.995)]["dfs"] for inst in instances]
    ax.bar(x - w*(n_alphas/2 + 0.5), dfs_vals, w, label="DFS baseline",
           color="#cccccc", edgecolor="white")

    # Fiedler init
    fiedler_vals = [data[(inst, 0.995)]["fiedler"] for inst in instances]
    ax.bar(x - w*(n_alphas/2 - 0.5), fiedler_vals, w, label="Fiedler init",
           color=ORANGE, edgecolor="white")

    # SA per alpha
    for i, alpha in enumerate(alphas):
        sa_vals = [data[(inst, alpha)]["sa"] for inst in instances]
        ax.bar(x + w*(i - n_alphas/2 + 1.5), sa_vals, w,
               label=f"SA  α={alpha}", color=alpha_colors[alpha], edgecolor="white")

    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=20, ha="right", fontsize=9)
    ax.set_ylabel("Custo MinLA")
    ax.legend(frameon=False, fontsize=9, ncol=2)
    ax.grid(axis="y")

    fig.tight_layout()
    out = FIGS / "sa_sensitivity.png"
    fig.savefig(out)
    plt.close(fig)
    print(f"  ✓ {out}")


# ══════════════════════════════════════════════════════════════════════════════
# Figure 3 — Optimality Gap (new)
# ══════════════════════════════════════════════════════════════════════════════
def fig_optimality_gap():
    """
    Grouped bar chart: MILP cost vs SA cost per instance.
    Highlights the cost difference visually; annotates gap % where SA > MILP.
    """
    records = [
        # (label,      n, milp, sa,   dfs)
        ("n5 s0",  5,   8,   8,  12),
        ("n5 s1",  5,  10,  10,  12),
        ("n5 s2",  5,   8,   8,  10),
        ("n5 s3",  5,  10,  10,  12),
        ("n5 s4",  5,   8,   8,  10),
        ("n8 s0",  8,  16,  16,  26),
        ("n8 s1",  8,  18,  18,  28),
        ("n8 s2",  8,  16,  16,  28),
        ("n8 s3",  8,  19,  19,  30),
        ("n8 s4",  8,  20,  20,  32),
        ("n10 s0", 10,  23,  23,  41),
        ("n10 s1", 10,  25,  26,  45),
        ("n10 s2", 10,  25,  25,  41),
        ("n10 s3", 10,  24,  24,  38),
        ("n10 s4", 10,  25,  25,  45),
        ("n12 s1", 12,  30,  30,  56),
        ("n12 s2", 12,  29,  29,  52),
        ("n12 s3", 12,  30,  30,  54),
    ]

    labels = [r[0] for r in records]
    milp   = np.array([r[2] for r in records], dtype=float)
    sa     = np.array([r[3] for r in records], dtype=float)

    x = np.arange(len(labels))
    w = 0.38

    fig, ax = plt.subplots(figsize=(10, 4.5))

    # MILP bars (reference — darker blue)
    ax.bar(x - w/2, milp, w, label="MILP (ótimo)", color="#2166ac", edgecolor="white", zorder=3)

    # SA bars — green where equal, orange/red where worse
    sa_colors = []
    for m, s in zip(milp, sa):
        if s == m:
            sa_colors.append("#4dac26")   # green = match
        else:
            sa_colors.append("#d6604d")   # red   = gap

    sa_bars = ax.bar(x + w/2, sa, w, label="SA  (α=0.995)",
                     color=sa_colors, edgecolor="white", zorder=3)

    # Annotate the one instance where SA > MILP
    for i, (bar, m, s) in enumerate(zip(sa_bars, milp, sa)):
        if s > m:
            gap_pct = (s - m) / m * 100
            bx = bar.get_x() + bar.get_width() / 2
            # bracket showing the extra cost
            ax.annotate(
                "",
                xy=(bx, s), xytext=(bx, m),
                arrowprops=dict(arrowstyle="<->", color=RED, lw=1.4),
                zorder=5,
            )
            ax.text(bx + 0.22, (s + m) / 2, f"+{gap_pct:.0f}%\n(Δ={s-m:.0f})",
                    ha="left", va="center", fontsize=8, color=RED, zorder=6)

    # legend patch for "exact match"
    import matplotlib.patches as mpatches
    patch_match = mpatches.Patch(color="#4dac26", label="SA = ótimo (17/18)")
    patch_gap   = mpatches.Patch(color=RED,       label="SA > ótimo (1/18)")
    ax.legend(handles=[
        mpatches.Patch(color="#2166ac", label="MILP (ótimo)"),
        patch_match,
        patch_gap,
    ], frameon=False, fontsize=9)

    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=40, ha="right", fontsize=8)
    ax.set_ylabel("Custo MinLA")
    ax.set_ylim(0, max(sa.max(), milp.max()) * 1.18)
    ax.grid(axis="y", zorder=0)

    # avg gap annotation
    gap_sa = (sa - milp) / milp * 100
    avg = gap_sa.mean()
    ax.text(0.98, 0.97, f"avg gap = {avg:.2f}%", transform=ax.transAxes,
            ha="right", va="top", fontsize=9, color=GRAY)

    fig.tight_layout()
    out = FIGS / "optimality_gap.png"
    fig.savefig(out)
    plt.close(fig)
    print(f"  ✓ {out}")



# ══════════════════════════════════════════════════════════════════════════════
if __name__ == "__main__":
    print("Generating presentation figures …")
    fig_scaling_wall()
    fig_sa_sensitivity()
    fig_optimality_gap()
    print("Done.")
