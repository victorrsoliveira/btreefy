"""
gen_scaling_cplex.py — Regenera scaling_wall.png com dados do CPLEX_PY.

Mantém o mesmo estilo de gen_figures.py (fig_scaling_wall).
Lê:  scripts/minla/results/scaling_raw.csv
Gera: scripts/minla/docs/figures/scaling_wall.png

Executar de: scripts/minla/docs/final_report/
  python3 gen_scaling_cplex.py
"""

import csv
from pathlib import Path
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# ── Paths ─────────────────────────────────────────────────────────────────────
HERE    = Path(__file__).parent
FIGS    = HERE.parent / "figures"
FIGS.mkdir(exist_ok=True)
RESULTS = HERE.parent.parent / "results"

# ── Shared style (igual a gen_figures.py) ─────────────────────────────────────
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

GRAY    = "#555555"
BLUE    = "#2166ac"
RED     = "#d6604d"
TIMEOUT = 300.0

# ── Leitura dos dados ──────────────────────────────────────────────────────────
rows = []
with open(RESULTS / "scaling_raw.csv", newline="") as f:
    for r in csv.DictReader(f):
        rows.append({"n": int(r["n"]), "t": float(r["solve_time"])})

by_n = defaultdict(list)
for r in rows:
    by_n[r["n"]].append(r["t"])

ns  = sorted(by_n)
avg = [np.mean(by_n[n]) for n in ns]
mx  = [np.max(by_n[n])  for n in ns]

# n cujas instâncias estouraram timeout
timeout_ns = [n for n in ns if np.min(by_n[n]) >= TIMEOUT]

# ── Figura ────────────────────────────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(6.5, 4))

ax.axhspan(250, 400, color="#f7e6e6", zorder=0)
ax.axhline(TIMEOUT, color=RED, linewidth=0.9, linestyle="--",
           label=f"{int(TIMEOUT//60)} min timeout")

ax.plot(ns, avg, marker="o", color=BLUE, linewidth=1.8,
        markersize=5, label="Tempo médio (CPLEX)")
ax.plot(ns, mx,  marker="s", color=GRAY, linewidth=1.2, linestyle="--",
        markersize=4, label="Tempo máximo")

ax.set_yscale("log")
ax.set_xlabel("Número de nós  $n$")
ax.set_ylabel("Tempo (s) — escala log")
ax.set_xticks(ns)
ax.set_ylim(0.01, 500)
ax.yaxis.set_major_formatter(ticker.FuncFormatter(
    lambda v, _: f"{v:.0f}s" if v >= 1 else f"{v:.2f}s"
))
ax.legend(frameon=False, fontsize=10)
ax.grid(axis="y")

if timeout_ns:
    ax.text(min(timeout_ns), 340, "timeout", color=RED, fontsize=9,
            va="bottom", ha="center")

ax.text(0.02, 0.97, "Solver: CPLEX (PuLP)",
        transform=ax.transAxes, fontsize=8, color=GRAY, va="top", ha="left")

fig.tight_layout()
out = FIGS / "scaling_wall.png"
fig.savefig(out)
plt.close(fig)
print(f"  ✓ {out}")
