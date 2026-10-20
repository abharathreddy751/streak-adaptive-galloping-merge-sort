#!/usr/bin/env python3
"""STAGE-3 independent checker.  Does NOT import or call scripts/final_audit.py, scripts/analyze.py, scripts/run_bench.py,
stage2/string_exp/* or stage3/string3/analyze_string3.py / run_string3.py.  Everything is re-derived from the raw CSV rows, the run log,
the freeze record and the plan text; the experimental design is TRANSCRIBED here from docs/STAGE3_CONFIRMATION_PLAN.md and is
checked against the frozen manifest digest.

    python -I stage3/audit/audit_stage3.py   ->  stage3/audit/audit_stage3_output.txt
"""
import csv, hashlib, json, math, random, re, sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
lines, tally = [], Counter()


def rec(level, msg):
    tally[level] += 1
    lines.append(f"[{level}] {msg}")
    print(lines[-1], flush=True)


def sha(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()


VARS = "naive,base,base_b,sag,sagc,tim_base,stdstable,topdown".split(",")
STRUCTS = [("perm", 0), ("few", 2), ("few", 16), ("few", 256), ("zipf", 1024), ("mixture", 64), ("runs", 8), ("nearly", 0.01)]


def design():
    cells = []
    for sti, style in enumerate("SML"):
        for fi, (fam, p) in enumerate(STRUCTS):
            for si, n in enumerate((1 << 17, 1 << 19)):
                cells.append(dict(cell=f"S3H_{style}_{fam}{('%g' % p).replace('.', 'p')}_n{n.bit_length() - 1}", style=style, family=fam, param=p, n=n,
                                  seed=7300000 + 100 * sti + 10 * fi + si, variants=",".join(VARS), reps=15 if n <= (1 << 17) else 11, warmup=1, file=""))
    for k, fam in enumerate(("natural", "shuffled")):
        cells.append(dict(cell=f"S3H_FILE_{fam}_n15", style="FILE", family=fam, param=0, n=19611, seed=7300900 + k, variants=",".join(VARS), reps=31, warmup=1, file="stage3/data/msys2_paths.txt"))
    return cells


HEADER = ("kind,cell,family,param,n,seed,elem,variant,rep,order_pos,ns_total,ns_per_elem,ok,comparisons,run_cmp,merge_cmp,merges,trivial_merges,blocks,moves,max_buffer,H,Hstar,nruns,pin_cpu,calib_cmp,c_ns,style,mean_len,cost_B,total_bytes").split(",")
BOOT, SEED = 10000, 12345


def med(xs):
    s = sorted(xs)
    k = len(s)
    return s[k // 2] if k % 2 else 0.5 * (s[k // 2 - 1] + s[k // 2])


def boot_ci(r):
    rng = random.Random(SEED)
    k = len(r)
    m = sorted(med([r[rng.randrange(k)] for _ in range(k)]) for _ in range(BOOT))
    return med(r), m[int(0.025 * BOOT)], m[int(0.975 * BOOT) - 1]


def sign_ci(r, conf=0.95):
    s = sorted(r)
    k = len(s)
    best = (1, k)
    for a in range(1, k // 2 + 1):
        if sum(math.comb(k, j) for j in range(a, k - a + 1)) / 2 ** k >= conf:
            best = (a, k - a + 1)
        else:
            break
    return med(s), s[best[0] - 1], s[best[1] - 1]


def fmt(t):
    return f"{t[0]:.3f} [{t[1]:.3f}, {t[2]:.3f}]"


def main():
    fr = (ROOT / "stage3" / "string3" / "freeze_record.txt").read_text()
    # ---- 1. freeze integrity
    bad = [f for h, f in re.findall(r"^([0-9a-f]{64}) \*(\S+)$", fr, re.M) if sha(ROOT / f) != h]
    rec("PASS" if not bad else "FAIL", f"freeze record: every hashed file (library sources, bench source+binary, runner, analyzer, dataset, plan, structure CSV) is byte-identical now (changed: {bad or 'none'})")
    r2 = (ROOT / "results" / "outputs" / "freeze_record_round2.txt").read_text()
    same = {f: f"{sha(ROOT / f)} *{f}" in r2 for f in ("src/sag_policy.hpp", "src/sag_time_policy.hpp", "src/generators.hpp", "src/entropy.hpp", "src/textbook_merge_sort.hpp", "src/natural_merge_sort.hpp")}
    rec("PASS" if all(v for k, v in same.items() if k != "src/natural_merge_sort.hpp") else "FAIL", f"SAG / SAG-C / generators / entropy / textbook sources equal the round-2 frozen code byte for byte")
    rec("INFO", f"src/natural_merge_sort.hpp equals its round-2 freeze hash: {same['src/natural_merge_sort.hpp']} (expected False: Stage-3 exception guard; the ORIGINAL is kept in stage3/artifacts/orig_src with hash {sha(ROOT / 'stage3/artifacts/orig_src/natural_merge_sort.hpp')[:16]}...)")
    orig_ok = f"{sha(ROOT / 'stage3/artifacts/orig_src/natural_merge_sort.hpp')} *src/natural_merge_sort.hpp" in r2
    rec("PASS" if orig_ok else "FAIL", "the preserved ORIGINAL natural_merge_sort.hpp equals the round-2 freeze hash")
    exp = design()
    dg = hashlib.sha256(json.dumps(exp, sort_keys=True).encode()).hexdigest()
    fz = re.search(r"held-out manifest=heldout cells=50 sha256=([0-9a-f]{64})", fr).group(1)
    rec("PASS" if dg == fz else "FAIL", f"design transcribed from the plan (50 cells) digest {dg[:16]}.. == frozen {fz[:16]}..")
    # ---- 2. raw data
    raw = (ROOT / "stage3" / "results" / "s3_heldout.csv").read_bytes().decode()
    grid = list(csv.reader(raw.splitlines()))
    hdr, body = grid[0], grid[1:]
    rec("PASS" if hdr == HEADER else "FAIL", "header = the 31 documented columns")
    rec("PASS" if raw.count("\n") - 1 == len(body) and all(len(r) == 31 for r in body) else "FAIL", f"physical lines minus header ({raw.count(chr(10)) - 1}) == records ({len(body)}); all records have 31 fields")
    rows = [dict(zip(hdr, r)) for r in body]
    e = {c["cell"]: c for c in exp}
    got = Counter(r["cell"] for r in rows)
    rec("PASS" if set(got) == set(e) else "FAIL", f"50/50 expected cells present; missing/unexpected: {sorted(set(got) ^ set(e)) or 'none'}")
    keys = Counter((r["kind"], r["cell"], r["variant"], r["rep"]) for r in rows)
    rec("PASS" if all(v == 1 for v in keys.values()) else "FAIL", f"no duplicate (kind, cell, variant, rep) keys ({sum(1 for v in keys.values() if v > 1)} duplicates)")
    exp_time = sum(c["reps"] * 8 for c in exp)
    kinds = Counter(r["kind"] for r in rows)
    rec("PASS" if (kinds["count"], kinds["time"], len(kinds)) == (400, exp_time, 2) else "FAIL", f"count rows {kinds['count']} (expected 400), time rows {kinds['time']} (expected {exp_time}), other kinds: {set(kinds) - {'count', 'time'} or 'none'}")
    byc = defaultdict(list)
    for r in rows:
        byc[r["cell"]].append(r)
    probs = []
    for cid, c in e.items():
        rs = byc[cid]
        for r in rs:
            if (r["family"], int(r["n"]), int(r["seed"]), r["elem"], r["style"]) != (c["family"], c["n"], c["seed"], "string", c["style"]) or float(r["param"]) != float(c["param"]):
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
        for f in ("H", "Hstar", "nruns", "pin_cpu", "style", "mean_len", "cost_B", "total_bytes"):
            if len({r[f] for r in rs}) != 1:
                probs.append(f"{cid}: {f} not constant")
    rec("PASS" if not probs else "FAIL", f"per-cell design (8 variants x R repetitions exactly once, order_pos permutations, constant structural columns): {len(probs)} problems {probs[:3]}")
    nok = sum(r["ok"] != "1" for r in rows)
    rec("PASS" if nok == 0 else "FAIL", f"rows with ok != 1: {nok} of {len(rows)}")
    nums = []
    for r in rows:
        n = int(r["n"])
        if r["kind"] == "time":
            if not (float(r["ns_total"]) > 0 and abs(float(r["ns_total"]) / n - float(r["ns_per_elem"])) <= 1e-5 * float(r["ns_per_elem"])):
                nums.append((r["cell"], r["variant"], r["rep"], "time units"))
        else:
            cm = int(r["comparisons"])
            if cm != int(r["run_cmp"]) + int(r["merge_cmp"]) and r["variant"] not in ("stdstable", "topdown"):
                nums.append((r["cell"], r["variant"], "cmp != run+merge"))
            if not (n - 1 <= cm <= 3 * n * math.log2(n)):
                nums.append((r["cell"], r["variant"], f"cmp {cm} outside [n-1, 3 n log2 n]"))
    rec("PASS" if not nums else "FAIL", f"timing units (ns_total == ns_per_elem*n, > 0) and comparison-count sanity (n-1 <= cmp <= 3 n log2 n; run+merge == total): violations {len(nums)} {nums[:3]}")
    cnt = {(r["cell"], r["variant"]): r for r in rows if r["kind"] == "count"}
    rec("PASS" if all(cnt[(c, "base")]["comparisons"] == cnt[(c, "base_b")]["comparisons"] for c in e) else "FAIL", "base and base_b (identical code) have identical comparison counts in all 50 cells")
    rec("PASS" if all(int(cnt[(c, "sagc")]["calib_cmp"]) == 16384 and float(cnt[(c, "sagc")]["c_ns"]) > 0 for c in e if int(e[c]["n"]) >= 4096) else "FAIL", "SAG-C count rows record 16,384 calibration calls and a positive measured c in every cell")
    # ---- 3. run log: exit codes agree with correctness flags
    log = (ROOT / "stage3" / "results" / "s3_heldout_runlog.txt").read_text(errors="replace")
    starts = re.findall(r"# run started (\S+ \S+) manifest=heldout sha256=([0-9a-f]{64})", log)
    ft = re.search(r"FREEZE RECORD (\d{4}-\d\d-\d\d \d\d:\d\d:\d\d)", fr).group(1)
    rec("PASS" if len(starts) == 1 and starts[0][0] > ft and starts[0][1] == fz else "FAIL", f"{len(starts)} run segment(s); first start {starts[0][0] if starts else '-'} is after the freeze record ({ft}) and logged the frozen digest")
    cmds = re.findall(r"--style (\S+) --family (\S+) --param (\S+) --n (\d+) --seed (\d+) --reps (\d+) --warmup (\d+) --variants (\S+) --cell (\S+) --csv .*? --pin (\S+)( --file .*?)?\n\s+exit=(-?\d+)", log)
    cp, exits = [], {}
    for style, fam, par, n, seed, reps, warm, vs, cell, pin, fil, ex in cmds:
        c = e.get(cell)
        exits[cell] = int(ex)
        if not c or (style, fam, float(par), int(seed), int(reps), int(warm), vs) != (c["style"], c["family"], float(c["param"]), c["seed"], c["reps"], c["warmup"], c["variants"]) or (c["style"] != "FILE" and int(n) != c["n"]):
            cp.append(cell)
    allok = {c: all(r["ok"] == "1" for r in byc[c]) for c in e}
    agree = all((exits.get(c) == 0) == allok[c] for c in e)
    rec("PASS" if len(cmds) == 50 and not cp and agree and re.search(r"# finished \S+ \S+ failures=0", log) else "FAIL",
        f"run log: {len(cmds)} invocations equal the design; process exit status == 0 exactly for the cells whose rows all have ok == 1 ({agree}); 'failures=0'; problems {cp or 'none'}")
    pin_pat = r"pinned to logical CPU (\d+) \(highest core efficiency class=(\d+)\), verified (\d+)/1000"
    pins = dict(Counter(re.findall(pin_pat, log)))
    rec("INFO", f"pinning records (cpu, class, samples/1000): {pins}")
    # ---- 4. structural classes and the frozen gain set
    low = lambda c: float(cnt[(c, "base")]["Hstar"]) <= 0.5 * math.log2(e[c]["n"])
    expn = lambda c: float(cnt[(c, "base")]["cost_B"]) >= 32.0
    G = sorted(c for c in e if low(c) and expn(c))
    planned = sorted(f"S3H_{st}_{fam}{('%g' % p).replace('.', 'p')}_n{n}" for st in "ML" for fam, p in (("few", 2), ("few", 16), ("few", 256), ("zipf", 1024)) for n in (17, 19))
    rec("PASS" if G == planned else "FAIL", f"gain set G recomputed from the CSV (low-entropy AND cost_B >= 32) has {len(G)} cells and equals the pre-declared list of 16 (M and L styles x few2/few16/few256/zipf1024 x both sizes)")
    sp = ROOT / "stage3" / "results" / "s3_structure_heldout.csv"
    srows = {r["cell"]: r for r in csv.DictReader(open(sp, newline=""))}
    same_struct = all(srows[c]["Hstar"] == cnt[(c, "base")]["Hstar"] and srows[c]["cost_B"] == cnt[(c, "base")]["cost_B"] and srows[c]["total_bytes"] == cnt[(c, "base")]["total_bytes"] for c in e)
    rec("PASS" if same_struct else "FAIL", "the un-timed structural pass (frozen before the run) and the timed run report identical H*, cost_B and total_bytes for every cell (the input data are the pre-declared inputs)")
    # ---- 5. independent recomputation of ratios, CIs, verdict
    T = defaultdict(lambda: defaultdict(dict))
    for r in rows:
        if r["kind"] == "time":
            T[r["cell"]][r["variant"]][int(r["rep"])] = float(r["ns_per_elem"])
    ratio = lambda c, v: [T[c][v][i] / T[c]["base"][i] for i in sorted(T[c][v])]
    res = {c: dict(sag=boot_ci(ratio(c, "sag")), sagc=boot_ci(ratio(c, "sagc")), aa=boot_ci(ratio(c, "base_b"))) for c in e}
    stored = {}
    for ln in (ROOT / "stage3" / "results" / "summary_s3_heldout.md").read_text(encoding="utf-8").splitlines():
        if ln.startswith("| S3H_"):
            f = [x.strip() for x in ln.strip().strip("|").split("|")]
            stored[f[0]] = f
    mism = []
    for c in e:
        f = stored.get(c)
        exp_cols = [f"{int(cnt[(c, 'sag')]['comparisons']) / int(cnt[(c, 'base')]['comparisons']):.3f}", fmt(res[c]["sag"]), fmt(res[c]["aa"]),
                    f"{med(ratio(c, 'naive')):.3f}", f"{med(ratio(c, 'tim_base')):.3f}", f"{med(ratio(c, 'stdstable')):.3f}", f"{med(ratio(c, 'topdown')):.3f}",
                    f"{(int(cnt[(c, 'sagc')]['comparisons']) + int(cnt[(c, 'sagc')]['calib_cmp'])) / int(cnt[(c, 'base')]['comparisons']):.3f}", fmt(res[c]["sagc"]),
                    f"{med(list(T[c]['base'].values())):.1f}"]
        if f is None or [f[5], f[6], f[7], f[8], f[9], f[10], f[11], f[12], f[13], f[14]] != exp_cols:
            mism.append((c, exp_cols, f))
    rec("PASS" if not mism and len(stored) == 50 else "FAIL", f"summary_s3_heldout.md: 50 rows x 10 numbers (comparison ratio, SAG ratio+CI, A/A+CI, naive, tim, std, topdown, SAG-C comparison + ratio+CI, base ns/elem) re-derived from raw rows with an independent bootstrap; mismatches {len(mism)} {mism[:1]}")
    win = lambda t: t[0] <= 0.90 and t[2] < 1.0
    sev = lambda t: t[0] >= 1.10 and t[1] > 1.0
    wins = [c for c in G if win(res[c]["sag"])]
    need = math.ceil(2 * len(G) / 3)
    p2 = len(wins) >= need
    sevs = sorted(c for c in e if sev(res[c]["sag"]))
    cmpbad = sorted(c for c in e if int(cnt[(c, "sag")]["comparisons"]) / int(cnt[(c, "base")]["comparisons"]) > 1.03)
    p1 = nok == 0 and agree and not probs
    verdict = "SUCCESS" if (p1 and p2 and not sevs and not cmpbad) else ("NOT MET (gain without safety)" if (p1 and p2) else "NOT MET")
    rec("INFO", f"P1 {p1}; P2 wins {len(wins)}/{len(G)} (need {need}) non-wins {sorted(set(G) - set(wins))}; P3 severe cells {sevs or 'none'}; P4 comparison excess {cmpbad or 'none'} -> independent verdict: {verdict}")
    sv = (ROOT / "stage3" / "results" / "summary_s3_heldout.md").read_text(encoding="utf-8")
    sm = re.search(r"\*\*Verdict under the frozen rule \(SAG\): (.*?)\*\*", sv).group(1)
    consistent = (verdict == "SUCCESS") == sm.startswith("CONFIRMATION SUCCESS")
    rec("PASS" if consistent else "DISCREPANCY", f"stored verdict text '{sm}' is consistent with the independently derived verdict '{verdict}'")
    flips = []
    for c in e:
        sg = sign_ci(ratio(c, "sag"))
        b = res[c]["sag"]
        if win(b) != win(sg) or sev(b) != sev(sg):
            flips.append(f"{c}: boot {fmt(b)} vs sign {fmt(sg)}")
    rec("INFO", f"sensitivity: win/severe classification under the exact sign-interval CI differs in {len(flips)} of 50 cells: {flips}")
    aa = sorted(((abs(res[c]['aa'][0] - 1), c) for c in e), reverse=True)[:3]
    rec("INFO", f"A/A control: largest |median-1| cells {[(round(a, 3), c) for a, c in aa]}; cells where identical code is flagged win/severe by the frozen rule: {[c for c in e if win(res[c]['aa']) or sev(res[c]['aa'])] or 'none'}")
    # ---- 6. dev data and consistency of project artefacts
    d = list(csv.reader((ROOT / "stage3" / "results" / "s3_dev.csv").read_text().splitlines()))[1:]
    rec("PASS" if all(len(r) == 31 and r[12] == "1" for r in d) else "FAIL", f"s3_dev.csv: {len(d)} records, all 31 fields, all ok == 1")
    for p, pat in (("README.md", "Stage 3"), ("docs/FINAL_REPORT.md", "Stage-3"), ("docs/STAGE3_CONFIRMATION_RESULTS.md", sm.split(" (")[0].split(" - ")[0]), ("docs/STAGE3_AUDIT_REPORT.md", "Stage 3")):
        t = (ROOT / p).read_text(encoding="utf-8") if (ROOT / p).exists() else ""
        rec("PASS" if pat in t else "FAIL", f"{p} exists and mentions '{pat}'")
    lines.append("")
    lines.append(f"SUMMARY: PASS {tally['PASS']}  FAIL {tally['FAIL']}  DISCREPANCY {tally['DISCREPANCY']}  INFO {tally['INFO']}")
    print(lines[-1])
    (ROOT / "stage3" / "audit" / "audit_stage3_output.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return 1 if tally["FAIL"] else 0


if __name__ == "__main__":
    sys.exit(main())
