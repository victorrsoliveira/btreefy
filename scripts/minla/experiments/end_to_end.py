"""
end_to_end.py — Phase 4: Full Pipeline Verification
=====================================================
Runs the complete pipeline on benchmark BTs:
  parse XML → SA optimize → emit C → build → run tests

Steps:
  1. Run btf_layout_emitter.py on each benchmark BT.
  2. Call ./compile-execute.sh --with-tests to verify the library
     still compiles and all tests pass with the new layout.
  3. Report MinLA cost reduction for each benchmark.

Run from repo root:
  python3 scripts/minla/experiments/end_to_end.py
"""

import os
import sys
import subprocess

_HERE      = os.path.dirname(os.path.abspath(__file__))
_MINLA_PKG = os.path.dirname(_HERE)
_SCRIPTS   = os.path.dirname(_MINLA_PKG)
_REPO      = os.path.dirname(_SCRIPTS)
for p in [_SCRIPTS, _REPO]:
    if p not in sys.path:
        sys.path.insert(0, p)

from minla.bt_graph  import from_xml
from minla.cost      import minla_cost
from minla.sa_solver import solve as sa_solve

BENCHMARKS = [
    {
        "model":   os.path.join(_REPO, "models", "porta_automatica.xml"),
        "treename": "main_new",
        "label":   "PortaAutomatica",
    },
    {
        "model":   os.path.join(_REPO, "models", "AssetTracking-App.xml"),
        "treename": "Monitoring",
        "label":   "AssetTracking",
    },
]

SA_ALPHA = 0.995
SA_SEED  = 42

# Emitter writes to the library's generated sources
SRC_OUT = os.path.join(_REPO, "src", "generated")
INC_OUT = os.path.join(_REPO, "include", "generated")


def run_emitter(bm: dict) -> dict:
    """Run btf_layout_emitter.py for one benchmark; return cost info."""
    print(f"\n[{bm['label']}]")

    # Compute costs directly (for reporting) without touching the build
    try:
        g = from_xml(bm["model"], bm["treename"])
    except Exception as e:
        print(f"  Skipping (parse error): {e}")
        return {}

    n          = g.n
    dfs_perm   = list(range(n))
    dfs_cost   = minla_cost(g, dfs_perm)
    result     = sa_solve(g, alpha=SA_ALPHA, seed=SA_SEED)
    sa_cost    = result.cost
    improvement = (dfs_cost - sa_cost) / dfs_cost * 100

    print(f"  n={n}  |E|={len(g.edges)}")
    print(f"  DFS cost : {dfs_cost:.0f}")
    print(f"  SA  cost : {sa_cost:.0f}  ({improvement:+.1f}% vs DFS)")

    # Run emitter subprocess
    emitter_cmd = [
        sys.executable,
        os.path.join(_SCRIPTS, "btf_layout_emitter.py"),
        "-m",  bm["model"],
        "-tn", bm["treename"],
        "--source-output-dir", SRC_OUT,
        "--include-output-dir", INC_OUT,
        "--alpha", str(SA_ALPHA),
        "--seed",  str(SA_SEED),
    ]
    proc = subprocess.run(emitter_cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        print(f"  Emitter FAILED:\n{proc.stderr}")
        return {"label": bm["label"], "n": n, "status": "EMIT_FAIL"}

    print(f"  Emitter OK → {SRC_OUT}/btf_nodes_generated.c")
    return {
        "label":       bm["label"],
        "n":           n,
        "dfs_cost":    dfs_cost,
        "sa_cost":     sa_cost,
        "improvement": improvement,
        "status":      "OK",
    }


def run_build() -> bool:
    """Invoke ./compile-execute.sh --with-tests and return True on success."""
    print("\n[Build check] ./compile-execute.sh --with-tests")
    proc = subprocess.run(
        ["./compile-execute.sh", "--with-tests"],
        cwd=_REPO,
        capture_output=True,
        text=True,
        timeout=300,
    )
    if proc.returncode == 0:
        print("  BUILD & TESTS PASSED ✓")
        # Print last 20 lines of output for evidence
        lines = (proc.stdout + proc.stderr).strip().splitlines()
        for line in lines[-20:]:
            print(f"    {line}")
        return True
    else:
        print("  BUILD/TESTS FAILED ✗")
        lines = (proc.stdout + proc.stderr).strip().splitlines()
        for line in lines[-30:]:
            print(f"    {line}")
        return False


if __name__ == "__main__":
    import time
    t0 = time.perf_counter()

    print("=" * 60)
    print("Phase 4 — End-to-End Pipeline")
    print("=" * 60)

    results = []
    for bm in BENCHMARKS:
        r = run_emitter(bm)
        if r:
            results.append(r)

    # Only attempt build if at least one emitter succeeded
    build_ok = False
    if any(r.get("status") == "OK" for r in results):
        try:
            build_ok = run_build()
        except subprocess.TimeoutExpired:
            print("  Build timed out after 5 minutes.")
    else:
        print("\n[Build check] Skipped — no emitter succeeded.")

    # Summary
    print("\n" + "=" * 60)
    print("End-to-End Summary")
    print("=" * 60)
    for r in results:
        status = "✓" if r.get("status") == "OK" else "✗"
        print(f"  {status} {r['label']:20s}  "
              f"n={r.get('n','?'):>3}  "
              f"DFS={r.get('dfs_cost','?'):>5.0f}  "
              f"SA={r.get('sa_cost','?'):>5.0f}  "
              f"({r.get('improvement', 0):+.1f}%)")
    print(f"  Build: {'PASS' if build_ok else 'FAIL (see above)'}")
    print(f"\nTotal wall time: {time.perf_counter() - t0:.1f}s")
