#!/usr/bin/env python3
"""Benchmark manifest runner.

    python scripts/run_bench.py --manifest dev_counts      # development: deterministic comparison counts only
    python scripts/run_bench.py --manifest dev_time        # development: small timing confirmation
    python scripts/run_bench.py --manifest heldout         # FROZEN held-out evaluation (see docs/EXPERIMENT_PLAN.md)
    python scripts/run_bench.py --manifest heldout --dry   # print the cells and the manifest digest only

Each cell = one bench_sort process (one workload, all variants on identical input, randomised variant order).
Raw CSV rows go to results/raw/<manifest>.csv (appended; the runner skips cells that are already complete so an
interrupted run can be resumed).  A command log goes to results/outputs/<manifest>_runlog.txt.

Seeds, sizes, families and repetition counts are defined HERE, in plain data, so that the exact experiment can be
audited and the digest of the held-out manifest can be pinned in the experiment plan before the run.
"""
import argparse
import csv
import hashlib
import json
import os
import platform
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
EXE = ROOT / "build" / ("bench_sort.exe" if os.name == "nt" or sys.platform.startswith(("win", "cygwin", "msys")) else "bench_sort")

# ------------------------------------------------------------------------------------------------------------
# Manifests
# ------------------------------------------------------------------------------------------------------------
DEV_FAMILIES = [("perm", 0), ("few", 3), ("few", 32), ("few", 512), ("few", 2048), ("zipf", 256), ("mixture", 16),
                ("runs", 16), ("nearly", 0.05), ("sorted", 0)]
DEV_SIZES = [1 << 16, 1 << 19, 1 << 21]
DEV_SEED_BASE = 101

HELD_FAMILIES = [("perm", 0), ("few", 2), ("few", 16), ("few", 256), ("few", 4096), ("zipf", 1024), ("mixture", 64),
                 ("runs", 64), ("nearly", 0.01)]
HELD_SIZES = [1 << 17, 1 << 20, 1 << 22]
HELD_SEED_BASE = 9001
BEVEN_K = [0, 2, 5, 10, 20, 40]


def cell_id(prefix, fam, param, n, elem):
    p = ("%g" % param).replace(".", "p")
    return f"{prefix}_{elem}_{fam}{p}_n{n.bit_length() - 1}"


def manifest_dev_counts():
    cells = []
    variants = "naive,base,sag,sag_h4,sag_h8,sag_h16,sag_h32,sag_j4,sag_j8,sag_j16,sag_j32,sag_j64,sag_j128"
    for fi, (fam, p) in enumerate(DEV_FAMILIES):
        for si, n in enumerate(DEV_SIZES):
            cells.append(dict(cell=cell_id("DEVC", fam, p, n, "plain"), family=fam, param=p, n=n, seed=DEV_SEED_BASE + 10 * fi + si,
                              elem="plain", variants=variants, reps=0, warmup=0))
    return cells


def manifest_dev_time(final_variant="sag"):
    cells = []
    variants = f"naive,base,base_b,{final_variant}"
    fams = [("perm", 0), ("few", 3), ("few", 32), ("few", 512), ("zipf", 256), ("mixture", 16), ("runs", 16)]
    for fi, (fam, p) in enumerate(fams):
        for n in (1 << 16, 1 << 19):
            for elem, reps in (("plain", 9), ("heavy", 5)):
                cells.append(dict(cell=cell_id("DEVT", fam, p, n, elem), family=fam, param=p, n=n, seed=DEV_SEED_BASE + 500 + fi,
                                  elem=elem, variants=variants, reps=reps, warmup=1))
    return cells


def manifest_heldout():
    cells = []
    for fi, (fam, p) in enumerate(HELD_FAMILIES):
        for si, n in enumerate(HELD_SIZES):
            seed = HELD_SEED_BASE + 10 * fi + si
            big = n >= (1 << 22)
            cells.append(dict(cell=cell_id("HELD", fam, p, n, "plain"), family=fam, param=p, n=n, seed=seed, elem="plain",
                              variants="naive,base,base_b,sag,tim_base,tim_sag,stdstable,topdown", reps=11 if big else 15, warmup=1))
            cells.append(dict(cell=cell_id("HELD", fam, p, n, "indirect"), family=fam, param=p, n=n, seed=seed, elem="indirect",
                              variants="naive,base,base_b,sag,stdstable", reps=11 if big else 15, warmup=1))
            if not big:  # heavy comparator regime: 2^17 and 2^20 only (a 2^22 sort takes seconds per variant)
                cells.append(dict(cell=cell_id("HELD", fam, p, n, "heavy"), family=fam, param=p, n=n, seed=seed, elem="heavy",
                                  variants="naive,base,base_b,sag,tim_base,tim_sag", reps=9 if n >= (1 << 20) else 15, warmup=1))
    # Break-even sweep (descriptive, NOT part of the pass/fail criteria): how expensive must a comparison be before the
    # extension pays?  Heavy comparator with a chain of K steps, K in BEVEN_K, on three families at n = 2^19.
    for fi, (fam, p) in enumerate([("few", 16), ("few", 256), ("perm", 0)]):
        for k in BEVEN_K:
            cells.append(dict(cell=f"BEVEN_k{k}_{fam}{('%g' % p)}_n19", family=fam, param=p, n=1 << 19, seed=HELD_SEED_BASE + 500 + fi,
                              elem="heavy", variants="naive,base,base_b,sag", reps=11, warmup=1, heavy_k=k))
    return cells


HELD2_SEED_BASE = 109001  # corrective round: FRESH seeds, disjoint from round 1 (9001...) and development (101...)


def manifest_dev_sagt():
    """Development tuning of the corrective-round extension SAG-T (dev seeds only)."""
    cells = []
    variants = "base,base_b,sag,sagbe_m8,sagc_m8_c10,sagc_m8_c50,sagc_m8_c200,sagc_m4_c50,sagc_m16_c50"
    fams = [("perm", 0), ("runs", 16), ("few", 32), ("few", 512), ("zipf", 256), ("mixture", 16), ("nearly", 0.05)]
    for fi, (fam, p) in enumerate(fams):
        for elem, n, reps in (("plain", 1 << 19, 9), ("indirect", 1 << 19, 9), ("heavy", 1 << 17, 7)):
            cells.append(dict(cell=cell_id("DEVS", fam, p, n, elem), family=fam, param=p, n=n, seed=DEV_SEED_BASE + 700 + fi,
                              elem=elem, variants=variants, reps=reps, warmup=1))
    return cells


def manifest_heldout2(final_variant="sagt"):
    """Corrective round: same 9 families, fresh seeds, sizes 2^17 and 2^20, three regimes (54 cells)."""
    cells = []
    variants = f"naive,base,base_b,sag,{final_variant}"
    for fi, (fam, p) in enumerate(HELD_FAMILIES):
        for si, n in enumerate((1 << 17, 1 << 20)):
            seed = HELD2_SEED_BASE + 10 * fi + si
            for elem in ("plain", "indirect", "heavy"):
                reps = 15 if (n == (1 << 17) or elem != "heavy") else 9
                cells.append(dict(cell=cell_id("HELD2", fam, p, n, elem), family=fam, param=p, n=n, seed=seed, elem=elem,
                                  variants=variants, reps=reps, warmup=1))
    return cells


MANIFESTS = {"dev_counts": manifest_dev_counts, "dev_time": manifest_dev_time, "heldout": manifest_heldout,
             "dev_sagt": manifest_dev_sagt, "heldout2": manifest_heldout2}


def digest(cells):
    return hashlib.sha256(json.dumps(cells, sort_keys=True).encode()).hexdigest()


# ------------------------------------------------------------------------------------------------------------
def done_cells(csv_path):
    """Returns (cells with timing rows, cells with counting rows)."""
    timed, counted = set(), set()
    if not csv_path.exists():
        return timed, counted
    with open(csv_path, newline="") as f:
        for r in csv.DictReader(f):
            (timed if r["kind"] == "time" else counted).add(r["cell"])
    return timed, counted


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True, choices=sorted(MANIFESTS))
    ap.add_argument("--dry", action="store_true")
    ap.add_argument("--only", default="", help="run only cells whose id contains this substring")
    ap.add_argument("--final-variant", default="sag", help="dev_time only: the SAG variant under test")
    ap.add_argument("--pin", default="auto")
    args = ap.parse_args()

    cells = manifest_dev_time(args.final_variant) if args.manifest == "dev_time" else MANIFESTS[args.manifest]()
    dg = digest(cells)
    print(f"manifest={args.manifest} cells={len(cells)} sha256={dg}")
    if args.dry:
        for c in cells:
            print(" ", c["cell"], c["family"], c["param"], c["n"], c["seed"], c["elem"], "reps", c["reps"])
        return 0
    if not EXE.exists():
        print("bench executable missing; run scripts/build.sh first", file=sys.stderr)
        return 2

    raw = ROOT / "results" / "raw"
    outd = ROOT / "results" / "outputs"
    raw.mkdir(parents=True, exist_ok=True)
    outd.mkdir(parents=True, exist_ok=True)
    csv_path = raw / f"{args.manifest}.csv"
    log_path = outd / f"{args.manifest}_runlog.txt"
    timed, counted = done_cells(csv_path)
    with open(log_path, "a") as log:
        log.write(f"\n# run started {time.strftime('%Y-%m-%d %H:%M:%S')} manifest={args.manifest} sha256={dg}\n")
        log.write(f"# platform: {platform.platform()}  python {platform.python_version()}  exe: {EXE}\n")
        failures = 0
        for i, c in enumerate(cells, 1):
            if args.only and args.only not in c["cell"]:
                continue
            if (c["cell"] in timed) if c["reps"] > 0 else (c["cell"] in counted):
                print(f"[{i}/{len(cells)}] skip (complete) {c['cell']}")
                continue
            cmd = [str(EXE), "--family", c["family"], "--param", str(c["param"]), "--n", str(c["n"]), "--seed", str(c["seed"]),
                   "--reps", str(c["reps"]), "--warmup", str(c["warmup"]), "--elem", c["elem"], "--variants", c["variants"],
                   "--cell", c["cell"], "--csv", str(csv_path), "--pin", args.pin]
            if "heavy_k" in c:
                cmd += ["--heavy_k", str(c["heavy_k"])]
            t0 = time.time()
            p = subprocess.run(cmd, capture_output=True, text=True)
            dt = time.time() - t0
            log.write(f"{' '.join(cmd)}\n  exit={p.returncode} wall={dt:.1f}s\n  {p.stderr.strip().splitlines()[-1] if p.stderr.strip() else ''}\n")
            log.flush()
            print(f"[{i}/{len(cells)}] {c['cell']} exit={p.returncode} {dt:.1f}s")
            if p.returncode != 0:
                failures += 1
                print(p.stderr[-500:], file=sys.stderr)
        log.write(f"# finished {time.strftime('%Y-%m-%d %H:%M:%S')} failures={failures}\n")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
