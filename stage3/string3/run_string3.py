#!/usr/bin/env python3
"""Manifest runner for the STAGE-3 confirmatory string experiment.

    python -I stage3/string3/run_string3.py --manifest heldout --dry
    python -I stage3/string3/run_string3.py --manifest structure_heldout     # un-timed structural classification pass (no sorting variants)
    python -I stage3/string3/run_string3.py --manifest dev                   # development / harness check
    python -I stage3/string3/run_string3.py --manifest heldout               # FROZEN held-out run (docs/STAGE3_CONFIRMATION_PLAN.md)

Seeds, sizes, families, styles and repetition counts are plain data below; the SHA-256 of the canonical JSON of each manifest is printed and frozen in the plan.
Raw rows are appended to stage3/results/<tag>.csv; the command log (with exit statuses and free RAM) to stage3/results/<tag>_runlog.txt.
Failed or non-zero cells are logged and never dropped.
"""
import argparse, csv, ctypes, hashlib, json, os, platform, subprocess, sys, time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXE = ROOT / "stage3" / "string3" / "bench_string3.exe"
RES = ROOT / "stage3" / "results"
DATAFILE = "stage3/data/msys2_paths.txt"
VARIANTS = "naive,base,base_b,sag,sagc,tim_base,stdstable,topdown"
MIN_FREE_MB = 1200

HELD_STRUCTS = [("perm", 0), ("few", 2), ("few", 16), ("few", 256), ("zipf", 1024), ("mixture", 64), ("runs", 8), ("nearly", 0.01)]
HELD_STYLES = ["S", "M", "L"]
HELD_SIZES = [1 << 17, 1 << 19]
HELD_SEED_BASE = 7300000
DEV_STRUCTS = [("perm", 0), ("few", 4), ("few", 64), ("zipf", 256), ("runs", 16), ("nearly", 0.05)]
DEV_SIZES = [1 << 16, 1 << 18]
DEV_SEED_BASE = 6500000


def cid(prefix, style, fam, p, n):
    return f"{prefix}_{style}_{fam}{('%g' % p).replace('.', 'p')}_n{n.bit_length() - 1}"


def manifest_heldout():
    cells = []
    for sti, style in enumerate(HELD_STYLES):
        for fi, (fam, p) in enumerate(HELD_STRUCTS):
            for si, n in enumerate(HELD_SIZES):
                cells.append(dict(cell=cid("S3H", style, fam, p, n), style=style, family=fam, param=p, n=n, seed=HELD_SEED_BASE + 100 * sti + 10 * fi + si,
                                  variants=VARIANTS, reps=15 if n <= (1 << 17) else 11, warmup=1, file=""))
    for k, fam in enumerate(("natural", "shuffled")):  # real strings: file names of a local MSYS2 installation (names only), non-gating controls
        cells.append(dict(cell=f"S3H_FILE_{fam}_n15", style="FILE", family=fam, param=0, n=19611, seed=HELD_SEED_BASE + 900 + k, variants=VARIANTS, reps=31, warmup=1, file=DATAFILE))
    return cells


def manifest_dev():
    cells = []
    for sti, style in enumerate(HELD_STYLES):
        for fi, (fam, p) in enumerate(DEV_STRUCTS):
            for si, n in enumerate(DEV_SIZES):
                cells.append(dict(cell=cid("S3D", style, fam, p, n), style=style, family=fam, param=p, n=n, seed=DEV_SEED_BASE + 100 * sti + 10 * fi + si,
                                  variants=VARIANTS, reps=7, warmup=1, file=""))
    return cells


def manifest_structure_heldout():
    return [dict(c, reps=0, variants="base") for c in manifest_heldout()]


MANIFESTS = {"dev": manifest_dev, "heldout": manifest_heldout, "structure_heldout": manifest_structure_heldout}


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
    args = ap.parse_args()
    cells = MANIFESTS[args.manifest]()
    dg = digest(cells)
    print(f"manifest={args.manifest} cells={len(cells)} sha256={dg}")
    if args.dry:
        for c in cells:
            print(" ", c["cell"], c["style"], c["family"], c["param"], c["n"], c["seed"], "reps", c["reps"])
        return 0
    if not EXE.exists():
        print("bench_string3.exe missing", file=sys.stderr)
        return 2
    RES.mkdir(parents=True, exist_ok=True)
    tag = f"s3_{args.manifest}"
    csv_path, log_path = RES / f"{tag}.csv", RES / f"{tag}_runlog.txt"
    done = set()
    if csv_path.exists():
        with open(csv_path, newline="") as f:
            done = {r["cell"] for r in csv.DictReader(f)}
    env = dict(os.environ)
    env["PATH"] = r"C:\msys64\usr\bin;" + env.get("PATH", "")
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
            cmd = [str(EXE), "--style", c["style"], "--family", c["family"], "--param", str(c["param"]), "--n", str(c["n"]), "--seed", str(c["seed"]),
                   "--reps", str(c["reps"]), "--warmup", str(c["warmup"]), "--variants", c["variants"], "--cell", c["cell"], "--csv", str(csv_path), "--pin", args.pin]
            if c["file"]:
                cmd += ["--file", str(ROOT / c["file"])]
            if args.manifest == "structure_heldout":
                cmd += ["--structure-only", "1"]
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
