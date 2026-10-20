#!/usr/bin/env python3
"""Independent audit of the STAGE-2 string experiment (does not import analyze_string.py or run_string_bench.py).

    python -I stage2/audit/audit_string_experiment.py   ->  stage2/audit/string_audit_output.txt

Re-derives from stage2/results/string_heldout.csv: structure/completeness, freeze-hash integrity, run-log vs design, every number in the
stored summary table, the frozen decision rule (P1-P4) for both extensions, and the sensitivity of the cell classification to the CI method.
"""
import csv, hashlib, importlib.util, json, math, re, sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("ia", ROOT / "stage2" / "audit" / "independent_audit.py")
ia = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ia)  # reuse only the generic statistics helpers (med, boot_ci, sign_ci, fmt)

lines, tally = [], Counter()


def rec(level, msg):
    tally[level] += 1
    lines.append(f"[{level}] {msg}")
    print(lines[-1], flush=True)


HEADER = ("kind,cell,family,param,n,seed,elem,variant,rep,order_pos,ns_total,ns_per_elem,ok,comparisons,run_cmp,merge_cmp,merges,trivial_merges,blocks,moves,max_buffer,H,Hstar,nruns,pin_cpu,calib_cmp,c_ns").split(",")
VARS = "naive,base,base_b,sag,sagc,tim_base,stdstable,topdown".split(",")
FAMS = [("perm", 0), ("few", 2), ("few", 16), ("few", 256), ("few", 4096), ("zipf", 1024), ("mixture", 64), ("runs", 64), ("nearly", 0.01)]


def expected():
    cells = []
    for fi, (fam, p) in enumerate(FAMS):
        for si, n in enumerate([1 << 17, 1 << 19]):
            cells.append(dict(cell=f"STRHELD_{fam}{('%g' % p).replace('.', 'p')}_n{n.bit_length() - 1}", family=fam, param=p, n=n, seed=7100000 + 10 * fi + si,
                              variants=",".join(VARS), reps=15 if n <= (1 << 17) else 11, warmup=1, prefix=240))
    return cells


def main():
    fr = (ROOT / "stage2" / "string_exp" / "freeze_record.txt").read_text()
    # 1 freeze integrity
    bad = []
    for h, f in re.findall(r"^([0-9a-f]{64}) \*(\S+)$", fr, re.M):
        if hashlib.sha256((ROOT / f).read_bytes()).hexdigest() != h:
            bad.append(f)
    rec("PASS" if not bad else "FAIL", f"freeze record: every hashed file (frozen src/*.hpp, bench_string.cpp/.exe, runner, analyzer, plan) is byte-identical now (changed: {bad or 'none'})")
    r2 = (ROOT / "results" / "outputs" / "freeze_record_round2.txt").read_text()
    same = all(f"{hashlib.sha256((ROOT / f).read_bytes()).hexdigest()} *{f}" in r2 for f in ("src/natural_merge_sort.hpp", "src/sag_policy.hpp", "src/sag_time_policy.hpp", "src/generators.hpp", "src/entropy.hpp", "src/textbook_merge_sort.hpp"))
    rec("PASS" if same else "FAIL", "the library code used by the string experiment equals the round-2 frozen code byte for byte (no source change was needed)")
    exp = expected()
    dg = hashlib.sha256(json.dumps(exp, sort_keys=True).encode()).hexdigest()
    fz = re.search(r"held-out manifest=heldout cells=18 sha256=([0-9a-f]{64})", fr).group(1)
    rec("PASS" if dg == fz else "FAIL", f"design transcribed from the plan (18 cells) digest {dg[:16]}.. == frozen {fz[:16]}..")
    # 2 raw data
    raw = (ROOT / "stage2" / "results" / "string_heldout.csv").read_bytes().decode()
    grid = list(csv.reader(raw.splitlines()))
    hdr, body = grid[0], grid[1:]
    rec("PASS" if hdr == HEADER else "FAIL", "header = 27 documented columns")
    rec("PASS" if raw.count("\n") - 1 == len(body) and all(len(r) == 27 for r in body) else "FAIL", f"lines minus header ({raw.count(chr(10)) - 1}) == records ({len(body)}); all records have 27 fields")
    rows = [dict(zip(hdr, r)) for r in body]
    e = {c["cell"]: c for c in exp}
    cells_got = Counter(r["cell"] for r in rows)
    rec("PASS" if set(cells_got) == set(e) else "FAIL", f"18/18 expected cells present; unexpected/missing: {sorted(set(cells_got) ^ set(e)) or 'none'}")
    keys = Counter((r["kind"], r["cell"], r["variant"], r["rep"]) for r in rows)
    rec("PASS" if all(v == 1 for v in keys.values()) else "FAIL", "no duplicate (kind, cell, variant, rep) keys")
    exp_time = sum(c["reps"] * 8 for c in exp)
    kinds = Counter(r["kind"] for r in rows)
    rec("PASS" if (kinds["count"], kinds["time"]) == (144, exp_time) else "FAIL", f"count rows {kinds['count']} (expected 144), time rows {kinds['time']} (expected {exp_time})")
    probs = []
    byc = defaultdict(list)
    for r in rows:
        byc[r["cell"]].append(r)
    for cid, c in e.items():
        rs = byc[cid]
        for r in rs:
            if (r["family"], int(r["n"]), int(r["seed"]), r["elem"]) != (c["family"], c["n"], c["seed"], "string") or float(r["param"]) != float(c["param"]):
                probs.append(f"{cid}: metadata")
                break
        tr = [r for r in rs if r["kind"] == "time"]
        for v in VARS:
            if sorted(int(r["rep"]) for r in tr if r["variant"] == v) != list(range(c["reps"])):
                probs.append(f"{cid}/{v}: repetitions")
        for rep in range(c["reps"]):
            if sorted(int(r["order_pos"]) for r in tr if int(r["rep"]) == rep) != list(range(8)):
                probs.append(f"{cid} rep {rep}: order_pos")
        if sorted(r["variant"] for r in rs if r["kind"] == "count") != sorted(VARS):
            probs.append(f"{cid}: count variants")
        for f in ("H", "Hstar", "nruns", "pin_cpu"):
            if len({r[f] for r in rs}) != 1:
                probs.append(f"{cid}: {f} not constant")
    rec("PASS" if not probs else "FAIL", f"per-cell design (8 variants x reps exactly once, order_pos permutations, constant meta): {len(probs)} problems {probs[:3]}")
    rec("PASS" if all(r["ok"] == "1" for r in rows) else "FAIL", f"rows with ok != 1: {sum(r['ok'] != '1' for r in rows)} of {len(rows)} (ok = idx sequence equals std::stable_sort reference AND independent order+stability scan)")
    nums = []
    for r in rows:
        if r["kind"] == "time":
            if not (float(r["ns_total"]) > 0 and abs(float(r["ns_total"]) / int(r["n"]) - float(r["ns_per_elem"])) <= 1e-5 * float(r["ns_per_elem"])):
                nums.append((r["cell"], r["variant"], r["rep"]))
        elif int(r["comparisons"]) != int(r["run_cmp"]) + int(r["merge_cmp"]):
            nums.append((r["cell"], r["variant"], "cmp"))
    rec("PASS" if not nums else "FAIL", f"time rows ns_total>0 and == ns_per_elem*n; count rows comparisons == run+merge (violations {len(nums)})")
    cnt = {(r["cell"], r["variant"]): r for r in rows if r["kind"] == "count"}
    rec("PASS" if all(cnt[(c, "base")]["comparisons"] == cnt[(c, "base_b")]["comparisons"] == cnt[(c, "tim_base")]["comparisons"] or True for c in e) and all(cnt[(c, "base")]["comparisons"] == cnt[(c, "base_b")]["comparisons"] for c in e) else "FAIL", "base and base_b (identical code) have identical comparison counts in all 18 cells")
    rec("PASS" if all(int(cnt[(c, "sagc")]["calib_cmp"]) == 16384 and float(cnt[(c, "sagc")]["c_ns"]) > 0 for c in e) else "FAIL", "SAG-C count rows record the 16,384 calibration comparisons and a positive measured c in all cells")
    # 3 run log
    log = (ROOT / "stage2" / "results" / "string_heldout_runlog.txt").read_text(errors="replace")
    starts = re.findall(r"# run started (\S+ \S+) manifest=heldout sha256=([0-9a-f]{64})", log)
    ft = re.search(r"FREEZE RECORD (\d{4}-\d\d-\d\d \d\d:\d\d:\d\d)", fr).group(1)
    rec("PASS" if len(starts) == 1 and starts[0][0] > ft and starts[0][1] == fz else "FAIL", f"one run segment, started {starts[0][0]} after the freeze record ({ft}), logged the frozen digest")
    cmds = re.findall(r"--family (\S+) --param (\S+) --n (\d+) --seed (\d+) --reps (\d+) --warmup (\d+) --variants (\S+) --cell (\S+) --csv .*? --pin (\S+) --prefix (\d+)\n\s+exit=(-?\d+)", log)
    cp = []
    for fam, par, n, seed, reps, warm, vs, cell, pin, pre, ex in cmds:
        c = e.get(cell)
        if not c or (fam, float(par), int(n), int(seed), int(reps), int(warm), vs, int(pre)) != (c["family"], float(c["param"]), c["n"], c["seed"], c["reps"], c["warmup"], c["variants"], c["prefix"]) or int(ex) != 0:
            cp.append(cell)
    rec("PASS" if len(cmds) == 18 and not cp and re.search(r"# finished \S+ \S+ failures=0", log) else "FAIL", f"run log: {len(cmds)} invocations, all equal to the design, all exit 0, 'failures=0'; problems {cp or 'none'}")
    pins = Counter(re.findall(r"pinned to logical CPU (\d+) \(highest core efficiency class=(\d+)\), verified (\d+)/1000", log))
    rec("INFO", f"pinning records (cpu, class, samples-on-cpu/1000): {dict(pins)}")
    rec("INFO", "free RAM before cells (MB): " + ", ".join(sorted(set(re.findall(r"free_mb_before=(\d+)", log)), key=int)[:3]) + " ...")
    # 4 recompute table + rule
    cells = ia.build_cells([r for r in rows])
    stored = {}
    for ln in (ROOT / "stage2" / "results" / "summary_string_heldout.md").read_text(encoding="utf-8").splitlines():
        if ln.startswith("| STRHELD"):
            f = [x.strip() for x in ln.strip().strip("|").split("|")]
            stored[f[0]] = f
    mism, res = [], {}
    for cid, c in cells.items():
        base_cnt = c["cnt"]["base"]
        calib = int(cnt[(cid, "sagc")]["calib_cmp"])
        t_c, t_s = ia.boot_ci(ia.ratio_stats(c, "sagc")), ia.boot_ci(ia.ratio_stats(c, "sag"))
        aa = ia.boot_ci(ia.ratio_stats(c, "base_b"))
        res[cid] = dict(c=c, tc=t_c, ts=t_s, aa=aa, cmp_c=c["cnt"]["sagc"] / base_cnt, cmp_s=c["cnt"]["sag"] / base_cnt)
        row = stored.get(cid)
        exp_cols = [f"{(c['cnt']['sagc'] + calib) / base_cnt:.3f}", f"{c['cnt']['sag'] / base_cnt:.3f}", ia.fmt(t_c), ia.fmt(t_s), ia.fmt(aa),
                    f"{ia.med(ia.ratio_stats(c, 'naive')):.3f}", f"{ia.med(ia.ratio_stats(c, 'tim_base')):.3f}", f"{ia.med(ia.ratio_stats(c, 'stdstable')):.3f}", f"{ia.med(ia.ratio_stats(c, 'topdown')):.3f}",
                    f"{ia.med(list(c['time']['base'].values())):.1f}"]
        if not row or row[3:13] != exp_cols:
            mism.append((cid, exp_cols, row[3:13] if row else None))
    rec("PASS" if not mism and len(stored) == 18 else "FAIL", f"summary_string_heldout.md: 18 rows x 10 numbers re-derived from the raw CSV with an independent bootstrap implementation; mismatches {len(mism)} {mism[:1]}")
    low = {k: v for k, v in res.items() if v["c"]["Hs"] <= 0.5 * math.log2(v["c"]["n"])}
    rec("PASS" if len(low) == 8 and all(k.split("_")[1].startswith(("few2", "few16", "few256", "zipf1024")) for k in low) else "FAIL", f"low-H* cells recomputed from the CSV's H*: {sorted(low)} (plan predicted exactly few2, few16, few256, zipf1024 x 2 sizes)")
    verdicts = {}
    for name, tk, ck in (("SAG-C (primary)", "tc", "cmp_c"), ("SAG (secondary)", "ts", "cmp_s")):
        wins = sorted(k for k, v in low.items() if v[tk][0] <= 0.90 and v[tk][2] < 1.0)
        sev = sorted(k for k, v in res.items() if v[tk][0] >= 1.10 and v[tk][1] > 1.0)
        cb = sorted(k for k, v in res.items() if v[ck] > 1.03)
        p2 = len(wins) >= math.ceil(2 * len(low) / 3)
        v = "SUCCESS" if (p2 and not sev and not cb) else ("GAIN WITHOUT SAFETY" if p2 else "NO-GO")
        verdicts[name] = v
        rec("INFO", f"{name}: P2 wins {len(wins)}/{len(low)} (need {math.ceil(2 * len(low) / 3)}); non-wins among low cells: {sorted(set(low) - set(wins))}; P3 severe cells {sev or 'none'}; P4 excess {cb or 'none'} -> {v}")
    rec("PASS" if verdicts["SAG-C (primary)"] == "GAIN WITHOUT SAFETY" and verdicts["SAG (secondary)"] == "SUCCESS" else "DISCREPANCY", f"independently re-derived verdicts {verdicts} vs stored 'GAIN WITHOUT SAFETY (guard failed)' / 'SUCCESS'")
    # 5 sensitivity
    flips = []
    for k, v in res.items():
        for tk, nm in (("tc", "sagc"), ("ts", "sag")):
            sg = ia.sign_ci(ia.ratio_stats(v["c"], nm))
            b = v[tk]
            if (b[0] <= 0.90 and b[2] < 1) != (sg[0] <= 0.90 and sg[2] < 1) or (b[0] >= 1.10 and b[1] > 1) != (sg[0] >= 1.10 and sg[1] > 1):
                flips.append(f"{k}/{nm}: bootstrap {ia.fmt(b)} vs sign-CI {ia.fmt(sg)}")
    rec("INFO", f"sensitivity: win/severe classification under the exact sign-interval CI instead of the bootstrap differs in {len(flips)} cell-variant pairs: {flips}")
    # margins
    for name, tk in (("SAG-C", "tc"), ("SAG", "ts")):
        thin = sorted((k, round(v[tk][0], 3), round(v[tk][2], 3)) for k, v in low.items() if 0.88 <= v[tk][0] <= 1.0)
        rec("INFO", f"{name}: low-H* cells that did NOT reach the 10 % target: {thin}")
    aa_max = max(abs(v["aa"][0] - 1) for v in res.values())
    rec("INFO", f"A/A control largest |median-1| = {aa_max:.3f}; any cell where identical code would be flagged win/severe by the frozen rule: " + (str([k for k, v in res.items() if (v['aa'][0] >= 1.10 and v['aa'][1] > 1) or (v['aa'][0] <= 0.90 and v['aa'][2] < 1)]) or "none"))
    cm = [(k, round(float(cnt[(k, 'sagc')]['c_ns']), 1)) for k in sorted(res)]
    ratio = [float(cnt[(k, 'sagc')]['c_ns']) / (ia.med(list(res[k]['c']['time']['base'].values())) / (res[k]['c']['cnt']['base'] / res[k]['c']['n'])) for k in res]
    rec("INFO", f"SAG-C calibration: measured c / (base wall-ns per comparison) ranges {min(ratio):.1f}x - {max(ratio):.1f}x over cells (median {ia.med(ratio):.1f}x): the calibration over-estimates the in-sort comparison cost")
    # dev data
    d = list(csv.reader((ROOT / "stage2" / "results" / "string_dev.csv").read_text().splitlines()))[1:]
    rec("PASS" if all(len(r) == 27 and r[12] == "1" for r in d) else "FAIL", f"string_dev.csv: {len(d)} records, all ok == 1")
    lines.append("")
    lines.append(f"SUMMARY: PASS {tally['PASS']}  FAIL {tally['FAIL']}  DISCREPANCY {tally['DISCREPANCY']}  INFO {tally['INFO']}")
    print(lines[-1])
    (ROOT / "stage2" / "audit" / "string_audit_output.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return 1 if tally["FAIL"] else 0


if __name__ == "__main__":
    sys.exit(main())
