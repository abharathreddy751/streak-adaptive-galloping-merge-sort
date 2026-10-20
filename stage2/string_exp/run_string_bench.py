#!/usr/bin/env python3
"""Manifest runner for the STAGE-2 real-comparator (string) experiment.

    python -I stage2/string_exp/run_string_bench.py --manifest dev --dry
    python -I stage2/string_exp/run_string_bench.py --manifest heldout          # FROZEN: see docs/STAGE2_REAL_COMPARATOR_PLAN.md

Seeds, sizes, families and repetition counts are plain data below; the SHA-256 of the canonical JSON of the held-out manifest is
frozen in docs/STAGE2_REAL_COMPARATOR_PLAN.md and stage2/string_exp/freeze_record.txt BEFORE the held-out run.
Every cell runs one bench_string process; raw rows are appended to stage2/results/string_<manifest>.csv; a command log with exit
statuses and free-RAM readings goes to stage2/results/string_<manifest>_runlog.txt.  Failed/non-zero cells are logged, never dropped.
"""
import argparse, csv, ctypes, hashlib, json, os, platform, subprocess, sys, time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXE = ROOT / "stage2" / "string_exp" / "bench_string.exe"
RES = ROOT / "stage2" / "results"
ALL_VARIANTS = "naive,base,base_b,sag,sagc,tim_base,stdstable,topdown"

DEV_FAMILIES = [("perm", 0), ("few", 3), ("few", 32), ("few", 512), ("zipf", 256), ("mixture", 16), ("runs", 16), ("nearly", 0.05)]
DEV_SIZES = [1 << 16, 1 << 18]
DEV_SEED_BASE = 6000000

HELD_FAMILIES = [("perm", 0), ("few", 2), ("few", 16), ("few", 256), ("few", 4096), ("zipf", 1024), ("mixture", 64), ("runs", 64), ("nearly", 0.01)]
HELD_SIZES = [1 << 17, 1 << 19]
HELD_SEED_BASE = 7100000
PREFIX = 240
MIN_FREE_MB = 1200


def cell_id(prefix, fam, p, n):
    return f"{prefix}_{fam}{('%g' % p).replace('.', 'p')}_n{n.bit_length() - 1}"


def manifest_dev():
    cells = []
    for fi, (fam, p) in enumerate(DEV_FAMILIES):
        for si, n in enumerate(DEV_SIZES):
            cells.append(dict(cell=cell_id("STRDEV", fam, p, n), family=fam, param=p, n=n, seed=DEV_SEED_BASE + 10 * fi + si,
                              variants=ALL_VARIANTS, reps=7, warmup=1, prefix=PREFIX))
    return cells


def manifest_heldout():
    cells = []
    for fi, (fam, p) in enumerate(HELD_FAMILIES):
        for si, n in enumerate(HELD_SIZES):
            cells.append(dict(cell=cell_id("STRHELD", fam, p, n), family=fam, param=p, n=n, seed=HELD_SEED_BASE + 10 * fi + si,
                              variants=ALL_VARIANTS, reps=15 if n <= (1 << 17) else 11, warmup=1, prefix=PREFIX))
    return cells


MANIFESTS = {"dev": manifest_dev, "heldout": manifest_heldout}


def digest(cells):
    return hashlib.sha256(json.dumps(cells, sort_keys=True).encode()).hexdigest()


def free_mb():
    class MS(ctypes.Structure):
        _fields_ = [("dwLength", ctypes.c_ulong), ("dwMemoryLoad", ctypes.c_ulong), ("ullTotalPhys", ctypes.c_ulonglong), ("ullAvailPhys", ctypes.c_ulonglong),
                    ("ullTotalPageFile", ctypes.c_ulonglong), ("ullAvailPageFile", ctypes.c_ulonglong), ("ullTotalVirtual", ctypes.c_ulonglong),
                    ("ullAvailVirtual", ctypes.c_ulonglong), ("ullAvailExtendedVirtual", ctypes.c_ulonglong)]
    m = MS()
    m.dwLength = ctypes.sizeof(MS)
    ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(m))
    return m.ullAvailPhys // (1 << 20)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True, choices=sorted(MANIFESTS))
    ap.add_argument("--dry", action="store_true")
    ap.add_argument("--only", default="")
    ap.add_argument("--pin", default="auto")
    ap.add_argument("--out", default="")
    args = ap.parse_args()
    cells = MANIFESTS[args.manifest]()
    dg = digest(cells)
    print(f"manifest={args.manifest} cells={len(cells)} sha256={dg}")
    if args.dry:
        for c in cells:
            print(" ", c["cell"], c["family"], c["param"], c["n"], c["seed"], "reps", c["reps"])
        return 0
    if not EXE.exists():
        print("bench_string.exe missing", file=sys.stderr)
        return 2
    RES.mkdir(parents=True, exist_ok=True)
    tag = args.out or f"string_{args.manifest}"
    csv_path, log_path = RES / f"{tag}.csv", RES / f"{tag}_runlog.txt"
    done = set()
    if csv_path.exists():
        with open(csv_path, newline="") as f:
            done = {r["cell"] for r in csv.DictReader(f) if r["kind"] == "time"}
    env = dict(os.environ)
    env["PATH"] = r"C:\msys64\usr\bin;" + env.get("PATH", "")  # Cygwin runtime DLL for the g++ 15.3 build
    failures = 0
    with open(log_path, "a") as log:
        log.write(f"\n# run started {time.strftime('%Y-%m-%d %H:%M:%S')} manifest={args.manifest} sha256={dg} free_mb={free_mb()}\n")
        log.write(f"# platform: {platform.platform()} python {platform.python_version()} exe: {EXE}\n")
        for i, c in enumerate(cells, 1):
            if args.only and args.only not in c["cell"]:
                continue
            if c["cell"] in done:
                print(f"[{i}/{len(cells)}] skip (complete) {c['cell']}")
                continue
            fm = free_mb()
            if fm < MIN_FREE_MB:
                log.write(f"ABORT before {c['cell']}: free RAM {fm} MB < {MIN_FREE_MB} MB\n")
                print(f"ABORT: free RAM {fm} MB < {MIN_FREE_MB}", file=sys.stderr)
                return 3
            cmd = [str(EXE), "--family", c["family"], "--param", str(c["param"]), "--n", str(c["n"]), "--seed", str(c["seed"]), "--reps", str(c["reps"]),
                   "--warmup", str(c["warmup"]), "--variants", c["variants"], "--cell", c["cell"], "--csv", str(csv_path), "--pin", args.pin, "--prefix", str(c["prefix"])]
            t0 = time.time()
            p = subprocess.run(cmd, capture_output=True, text=True, env=env)
            dt = time.time() - t0
            last = p.stderr.strip().splitlines()
            log.write(f"{' '.join(cmd)}\n  exit={p.returncode} wall={dt:.1f}s free_mb_before={fm}\n  {last[0] if last else ''}\n  {last[-1] if last else ''}\n")
            log.flush()
            print(f"[{i}/{len(cells)}] {c['cell']} exit={p.returncode} {dt:.1f}s", flush=True)
            if p.returncode != 0:
                failures += 1
                print(p.stderr[-600:], file=sys.stderr)
        log.write(f"# finished {time.strftime('%Y-%m-%d %H:%M:%S')} failures={failures}\n")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
