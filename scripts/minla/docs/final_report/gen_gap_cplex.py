"""
gen_gap_cplex.py — Reproduz a Parte A do experimento de validação com CPLEX.

Critério de instâncias verificadas: n <= 15 e solve_time < 300 s
(CPLEX provou otimalidade para todas as instâncias nesse intervalo).

Lê:
  results/scaling_raw.csv   — custos ótimos MILP por instância
Gera:
  docs/figures/optimality_gap.png — figura 2 do artigo (estilo gen_figures.py)
  results/optimality_gap_table.csv

Executar de: scripts/minla/docs/final_report/
  python3 gen_gap_cplex.py
"""

import csv
import os
import sys
from pathlib import Path

# ── Paths ──────────────────────────────────────────────────────────────────────
HERE      = Path(__file__).parent                          # docs/final_report/
MINLA_PKG = HERE.parent.parent                             # scripts/minla/
SCRIPTS   = MINLA_PKG.parent                               # scripts/
REPO      = SCRIPTS.parent                                 # repo root

for p in [str(SCRIPTS), str(REPO)]:
    if p not in sys.path:
        sys.path.insert(0, p)

from minla.random_bt import generate
from minla.sa_solver import solve as sa_solve
from minla.cost      import minla_cost

RESULTS      = MINLA_PKG / "results"
FIGURES      = MINLA_PKG / "docs" / "figures"
SCALING_CSV  = RESULTS / "scaling_raw.csv"
GAP_CSV      = RESULTS / "optimality_gap_table.csv"
GAP_FIG      = FIGURES / "optimality_gap.png"

SA_ALPHA = 0.995
SA_SEED  = 42

# ── Shared style (igual a gen_figures.py) ─────────────────────────────────────
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

plt.rcParams.update({
    "font.family":        "sans-serif",
    "font.size":          11,
    "axes.spines.top":    False,
    "axes.spines.right":  False,
    "axes.linewidth":     0.8,
    "grid.color":         "#dddddd",
    "grid.linewidth":     0.6,
    "figure.dpi":         150,
    "savefig.dpi":        150,
    "savefig.bbox":       "tight",
    "savefig.pad_inches": 0.15,
})

GRAY  = "#555555"
BLUE  = "#2166ac"
RED   = "#d6604d"
GREEN = "#4dac26"

# ── Parte A: carregar instâncias verificadas pelo CPLEX ────────────────────────
verified = []
with open(SCALING_CSV) as f:
    for row in csv.DictReader(f):
        if int(row["n"]) <= 15 and float(row["solve_time"]) < 300.0:
            verified.append({
                "n":         int(row["n"]),
                "seed":      int(row["seed"]),
                "milp_cost": float(row["milp_cost"]),
            })

print(f"Instâncias verificadas (n<=15, CPLEX): {len(verified)}")

# ── Parte A: rodar SA e calcular gap ──────────────────────────────────────────
print(f"\n{'n':>4}  {'seed':>4}  {'milp':>6}  {'sa':>6}  {'gap%':>7}")
gap_rows = []
for inst in verified:
    n, seed, milp = inst["n"], inst["seed"], inst["milp_cost"]
    g       = generate(n, seed=seed)
    result  = sa_solve(g, alpha=SA_ALPHA, seed=SA_SEED)
    sa_cost = result.cost
    gap     = (sa_cost - milp) / milp * 100
    dfs     = minla_cost(g, list(range(n)))
    print(f"{n:>4}  {seed:>4}  {milp:>6.1f}  {sa_cost:>6.1f}  {gap:>+7.2f}%")
    gap_rows.append({
        "n": n, "seed": seed,
        "milp_cost": milp, "sa_cost": sa_cost,
        "gap_pct": round(gap, 3),
        "dfs_cost": dfs,
    })

avg_gap = sum(r["gap_pct"] for r in gap_rows) / len(gap_rows)
max_gap = max(r["gap_pct"] for r in gap_rows)
n_optimal = sum(1 for r in gap_rows if r["gap_pct"] == 0.0)
print(f"\nInstâncias: {len(gap_rows)}  avg_gap={avg_gap:+.3f}%  "
      f"max_gap={max_gap:+.2f}%  ótimas={n_optimal}/{len(gap_rows)}")

# ── Salvar CSV ─────────────────────────────────────────────────────────────────
with open(GAP_CSV, "w", newline="") as f:
    writer = csv.DictWriter(
        f, fieldnames=["n","seed","milp_cost","sa_cost","gap_pct","dfs_cost"])
    writer.writeheader()
    writer.writerows(gap_rows)
print(f"\nCSV → {GAP_CSV}")

# ── Gráfico (estilo gen_figures.py) divido em 2 partes ────────────────────────
def plot_gap_subset(subset_rows, filename):
    if not subset_rows: return
    labels = [f"n{r['n']} s{r['seed']}" for r in subset_rows]
    milp   = np.array([r["milp_cost"] for r in subset_rows], dtype=float)
    sa     = np.array([r["sa_cost"]   for r in subset_rows], dtype=float)

    x = np.arange(len(labels))
    w = 0.38

    fig, ax = plt.subplots(figsize=(max(8, len(labels) * 0.55 + 2), 4.5))

    # Barras MILP (referência)
    ax.bar(x - w/2, milp, w, label="MILP (ótimo)",
           color=BLUE, edgecolor="white", zorder=3)

    # Barras SA — verde se igual, vermelho se pior
    sa_colors = [GREEN if s == m else RED for m, s in zip(milp, sa)]
    sa_bars = ax.bar(x + w/2, sa, w, label="SA  (α=0.995)",
                     color=sa_colors, edgecolor="white", zorder=3)

    # Anotar instâncias com gap > 0
    for bar, m, s in zip(sa_bars, milp, sa):
        if s > m:
            gap_pct = (s - m) / m * 100
            bx = bar.get_x() + bar.get_width() / 2
            ax.annotate("",
                xy=(bx, s), xytext=(bx, m),
                arrowprops=dict(arrowstyle="<->", color=RED, lw=1.4),
                zorder=5)
            ax.text(bx + 0.22, (s + m) / 2,
                    f"+{gap_pct:.0f}%\n(Δ={s-m:.0f})",
                    ha="left", va="center", fontsize=8, color=RED, zorder=6)

    # Legenda
    n_opt = sum(1 for r in subset_rows if r["gap_pct"] == 0.0)
    ax.legend(handles=[
        mpatches.Patch(color=BLUE,  label="MILP (ótimo)"),
        mpatches.Patch(color=GREEN, label=f"SA = ótimo ({n_opt}/{len(subset_rows)})"),
        mpatches.Patch(color=RED,   label=f"SA > ótimo ({len(subset_rows)-n_opt}/{len(subset_rows)})"),
    ], frameon=False, fontsize=9)

    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=40, ha="right", fontsize=8)
    ax.set_ylabel("Custo MinLA")
    ax.set_ylim(0, max(sa.max(), milp.max()) * 1.20)
    ax.grid(axis="y", zorder=0)

    # Anotação avg gap para o subset
    avg_g = sum(r["gap_pct"] for r in subset_rows) / len(subset_rows)
    ax.text(0.98, 0.97, f"avg gap = {avg_g:.3f}%",
            transform=ax.transAxes, ha="right", va="top",
            fontsize=9, color=GRAY)

    fig.tight_layout()
    fig.savefig(filename)
    plt.close(fig)
    print(f"Figura → {filename}")

gap_rows_1 = [r for r in gap_rows if r['n'] <= 10]
gap_rows_2 = [r for r in gap_rows if r['n'] >= 12]

plot_gap_subset(gap_rows_1, FIGURES / "optimality_gap_1.png")
plot_gap_subset(gap_rows_2, FIGURES / "optimality_gap_2.png")
