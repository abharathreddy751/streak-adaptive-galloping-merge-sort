#!/usr/bin/env python3
"""Independent recomputation of the headline metrics of the final report from the retained raw CSV files (v4 audit).

    python -I scripts/audit_recompute.py            # prints a table of claim | claimed | recomputed | verdict; exit 1 on any mismatch

Written from the frozen protocol text (docs/EXPERIMENT_PLAN.md, docs/STAGE2_REAL_COMPARATOR_PLAN.md, docs/STAGE3_CONFIRMATION_PLAN.md), NOT by
importing the project's analyzers.  Statistics: per cell, median over repetitions of the paired ratio time(variant)/time(base) of the SAME
repetition; 95 % percentile bootstrap CI (10,000 resamples of the repetition indices, random.Random(12345), sorted medians at indices 250 and 9749,
as the plans specify).  As a second, distribution-free check it also computes an exact sign-based confidence interval for the median
(order statistics, binomial(k, 1/2), level >= 95 %) and reports whether any classification changes.  No raw data are modified.
Standard library only.
"""
import csv, math, random, re, statistics, sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOL = 0.0006  # 3-decimal reports: allow rounding of the last printed digit
bad = []


def check(name, claimed, got, tol=TOL, text=False):
    ok = (claimed == got) if text else abs(claimed - got) <= tol
    print(f"{'OK  ' if ok else 'MISMATCH'} | {name} | claimed {claimed} | recomputed {got}")
    if not ok:
        bad.append(name)


def boot_ci(r, seed=12345, B=10000):
    rng = random.Random(seed)
    k = len(r)
    meds = sorted(statistics.median([r[rng.randrange(k)] for _ in range(k)]) for _ in range(B))
    return statistics.median(r), meds[int(0.025 * B)], meds[int(0.975 * B) - 1]


def sign_ci(r):
    """Exact distribution-free CI for the median: [x_(j), x_(k+1-j)], j largest with P(Bin(k,1/2) <= j-1) <= 0.025."""
    s = sorted(r)
    k = len(s)
    j = 1
    cdf = lambda m: sum(math.comb(k, i) for i in range(m + 1)) / 2 ** k
    while j + 1 <= k // 2 and cdf(j) <= 0.025:
        j += 1
    if cdf(j - 1) > 0.025:
        return s[0], s[-1]
    return s[j - 1], s[k - j]


def load(path):
    cells = {}
    for r in csv.DictReader(open(path, newline="")):
        c = cells.setdefault(r["cell"], dict(row=r, time=defaultdict(dict), cnt={}, ok=True, nrows=0))
        c["nrows"] += 1
        if r["ok"] != "1":
            c["ok"] = False
        if r["kind"] == "time":
            c["time"][r["variant"]][int(r["rep"])] = float(r["ns_per_elem"])
        else:
            c["cnt"][r["variant"]] = int(r["comparisons"])
    return cells


def ratios(c, v, base="base"):
    reps = sorted(set(c["time"][v]) & set(c["time"][base]))
    return [c["time"][v][i] / c["time"][base][i] for i in reps]


def low(c):
    return float(c["row"]["Hstar"]) <= 0.5 * math.log2(int(c["row"]["n"]))


# ======================================================================================================================
print("=== STAGE 3 (stage3/results/s3_heldout.csv) ===")
rows_total = sum(1 for _ in open(ROOT / "stage3/results/s3_heldout.csv")) - 1
check("S3 rows in raw CSV", 5888, rows_total, 0)
S3 = load(ROOT / "stage3/results/s3_heldout.csv")
check("S3 cells", 50, len(S3), 0)
check("S3 every row ok=1", 1, int(all(c["ok"] for c in S3.values())), 0)
reps_expected = {}
for cid, c in S3.items():
    n = int(c["row"]["n"])
    exp = 31 if "FILE" in cid else (15 if n == 2 ** 17 else 11)
    got = {v: len(t) for v, t in c["time"].items()}
    if len(got) != 8 or set(got.values()) != {exp} or len(c["cnt"]) != 8:
        reps_expected[cid] = (exp, got)
check("S3 completeness: 8 variants x protocol repetitions (15 / 11 / 31) x count pass in every cell", 0, len(reps_expected), 0)

# randomisation of the variant order (protocol: re-randomised in every repetition): each (cell, rep) uses every position 0..7 exactly once,
# and the position of the base variant is not constant
pos_by = defaultdict(dict)
for r in csv.DictReader(open(ROOT / "stage3/results/s3_heldout.csv", newline="")):
    if r["kind"] == "time":
        pos_by[(r["cell"], r["rep"])][r["variant"]] = int(r["order_pos"])
perm_ok = all(sorted(v.values()) == list(range(8)) for v in pos_by.values())
base_pos = [v["base"] for v in pos_by.values()]
check("S3 order_pos is a permutation of 0..7 in every (cell, repetition)", 1, int(perm_ok and len(pos_by) == 686), 0)
check("S3 base variant occupies every position 0..7 over the repetitions (mean position about 3.5)", 1, int(set(base_pos) == set(range(8)) and abs(statistics.mean(base_pos) - 3.5) < 0.25), 0)
print(f"     (info) mean order position of base {statistics.mean(base_pos):.3f}; of sag {statistics.mean(v['sag'] for v in pos_by.values()):.3f}; repetition groups {len(pos_by)}")

gate = [cid for cid, c in S3.items() if low(c) and float(c["row"]["cost_B"]) >= 32.0]
check("S3 gain-set size (H* <= 0.5 log2 n AND cost_B >= 32)", 16, len(gate), 0)
# the gain set was fixed by an un-timed structural pass BEFORE the freeze: it must equal that file's classification
struct = {r["cell"]: r for r in csv.DictReader(open(ROOT / "stage3/results/s3_structure_heldout.csv", newline=""))}
sgate = sorted(cid for cid, r in struct.items() if float(r["Hstar"]) <= 0.5 * math.log2(int(r["n"])) and float(r["cost_B"]) >= 32.0)
check("S3 gain set equals the pre-freeze structural pass classification", 1, int(sorted(gate) == sgate), 0)
same_struct = all(abs(float(struct[c]["Hstar"]) - float(S3[c]["row"]["Hstar"])) < 1e-9 and abs(float(struct[c]["cost_B"]) - float(S3[c]["row"]["cost_B"])) < 1e-9 for c in S3 if c in struct)
check("S3 structure (H*, cost_B) identical in the structural pass and the timed run", 1, int(same_struct), 0)

res = {cid: {v: boot_ci(ratios(c, v)) for v in ("sag", "base_b")} for cid, c in S3.items()}
cmpx = {cid: c["cnt"]["sag"] / c["cnt"]["base"] for cid, c in S3.items()}
win = lambda t: t[0] <= 0.90 and t[2] < 1.0
sev = lambda t: t[0] >= 1.10 and t[1] > 1.0
wins = [cid for cid in gate if win(res[cid]["sag"])]
check("S3 P2: gain cells with ratio <= 0.90 and CI upper < 1 (bootstrap)", 12, len(wins), 0)
check("S3 P2: required (ceil(2/3 x 16))", 11, math.ceil(2 * 16 / 3), 0)
check("S3 non-wins are L_few2_n19, M_few2_n17, M_few2_n19, M_zipf1024_n17", 1,
      int(sorted(set(gate) - set(wins)) == ["S3H_L_few2_n19", "S3H_M_few2_n17", "S3H_M_few2_n19", "S3H_M_zipf1024_n17"]), 0)
check("S3 P3: cells with severe regression (ratio >= 1.10 and CI lower > 1)", 0, sum(sev(res[c]["sag"]) for c in S3), 0)
worst = max(res[c]["sag"][0] for c in S3)
check("S3 largest SAG/base median time ratio over all 50 cells", 1.076, round(worst, 3), 0.0006)
check("S3 P4: maximum SAG/base comparison ratio", 1.000, round(max(cmpx.values()), 3), 0.0006)
check("S3 P4: cells with comparison ratio > 1.03", 0, sum(v > 1.03 for v in cmpx.values()), 0)
check("S3 A/A: largest |median(base_b/base) - 1|", 0.021, round(max(abs(res[c]["base_b"][0] - 1) for c in S3), 3), 0.0006)
gc = [cmpx[c] for c in gate]
check("S3 gain-set comparison ratio min", 0.755, round(min(gc), 3))
check("S3 gain-set comparison ratio max", 0.852, round(max(gc), 3))  # report table: M_few2_n19 = 0.852

# exact sign-interval sensitivity
chg = []
for cid, c in S3.items():
    t = ratios(c, "sag")
    lo, hi = sign_ci(t)
    m = statistics.median(t)
    if (cid in gate and win((m, lo, hi)) != (cid in wins)) or sev((m, lo, hi)) != sev(res[cid]["sag"]):
        chg.append(cid)
check("S3 classifications that change under the exact sign-interval CI", 0, len(chg), 0)

# the 16-row table of the final report
rep = (ROOT / "docs/FINAL_PROJECT_REPORT.md").read_text(encoding="utf-8")
tbl = re.findall(r"^\| ([LM]_[a-z0-9]+_n\d+) \| ([\d.]+) \| ([\d.]+) \[([\d.]+), ([\d.]+)\] \| ([\d.]+) \[([\d.]+), ([\d.]+)\] \| ([\d.]+) \| (yes|no) \|$", rep, re.M)
check("report gain table rows found", 16, len(tbl), 0)
nbad = 0
for name, cm, tm, tl, th, am, al, ah, ns, w in tbl:
    cid = "S3H_" + name
    r = res[cid]
    base_ns = statistics.median(list(S3[cid]["time"]["base"].values()))
    got = (cmpx[cid], *r["sag"], *r["base_b"], base_ns)
    claimed = tuple(map(float, (cm, tm, tl, th, am, al, ah, ns)))
    tolv = (0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.06)
    if any(abs(a - b) > t for a, b, t in zip(claimed, (got[0], got[1], got[2], got[3], got[4], got[5], got[6], got[7]), tolv)) or (w == "yes") != (cid in wins):
        nbad += 1
        print("  row differs:", name, claimed, [round(x, 3) for x in got], w, cid in wins)
check("report gain-table rows that differ from the recomputation (cmp, time+CI, A/A+CI, base ns, win)", 0, nbad, 0)

# group ranges in the report
S = [c for c in S3 if "_S_" in c and low(S3[c]) and "FILE" not in c]
tr = [res[c]["sag"][0] for c in S]
check("report: cheap-key low-entropy cells (S) count", 8, len(S), 0)
check("report: cheap-key low-entropy time ratio min", 0.938, round(min(tr), 3))
check("report: cheap-key low-entropy time ratio max", 1.076, round(max(tr), 3))
check("report: cheap-key low-entropy comparison ratio min", 0.755, round(min(cmpx[c] for c in S), 3))
check("report: cheap-key low-entropy comparison ratio max", 0.853, round(max(cmpx[c] for c in S), 3))
for st_, lo_, hi_ in (("L", 0.602, 0.626), ("M", 0.788, 0.799), ("S", 0.832, 0.838)):
    cs = [c for c in S3 if f"_{st_}_nearly" in c]
    check(f"report: nearly sorted {st_} cells", 2, len(cs), 0)
    check(f"report: nearly sorted {st_} time ratio min", lo_, round(min(res[c]['sag'][0] for c in cs), 3))
    check(f"report: nearly sorted {st_} time ratio max", hi_, round(max(res[c]['sag'][0] for c in cs), 3))
ns_ = [c for c in S3 if "_nearly" in c]
check("report: nearly sorted comparison ratio min", 0.517, round(min(cmpx[c] for c in ns_), 3))
check("report: nearly sorted comparison ratio max", 0.547, round(max(cmpx[c] for c in ns_), 3))  # corrected in v4 audit: v3/v4 report said 0.541 (L_nearly_n17 = 0.547)
G3 = [c for c in S3 if any(f"_{f}_" in c for f in ("perm0", "runs8", "mixture64")) and "FILE" not in c]
check("report: permutation / 8 runs / mixture cells", 18, len(G3), 0)
check("report: perm/runs/mixture time ratio min", 0.871, round(min(res[c]['sag'][0] for c in G3), 3))
check("report: perm/runs/mixture time ratio max", 1.044, round(max(res[c]['sag'][0] for c in G3), 3))
check("report: perm/runs/mixture comparison ratio min", 0.891, round(min(cmpx[c] for c in G3), 3))
check("report: perm/runs/mixture comparison ratio max", 1.000, round(max(cmpx[c] for c in G3), 3))
for cid, claimed in (("S3H_S_zipf1024_n17", 1.076), ("S3H_S_zipf1024_n19", 1.056)):  # "5.6 and 7.6 percent slower" in the report
    check(f"report: {cid} time ratio", claimed, round(res[cid]["sag"][0], 3))
fn, fs = res["S3H_FILE_natural_n15"]["sag"], res["S3H_FILE_shuffled_n15"]["sag"]
check("report: FILE natural time ratio", 0.957, round(fn[0], 3)); check("report: FILE natural CI low", 0.944, round(fn[1], 3)); check("report: FILE natural CI high", 0.966, round(fn[2], 3))
check("report: FILE natural comparison ratio", 0.845, round(cmpx["S3H_FILE_natural_n15"], 3))
check("report: FILE shuffled time ratio", 1.007, round(fs[0], 3)); check("report: FILE shuffled comparison ratio", 1.000, round(cmpx["S3H_FILE_shuffled_n15"], 3))
# wall time per comparison by style (base ns per element / comparisons per element)
for st_, lo_, hi_ in (("L", 17, 71), ("M", 11, 28), ("S", 7, 17)):
    v = [statistics.median(list(S3[c]["time"]["base"].values())) / (S3[c]["cnt"]["base"] / int(S3[c]["row"]["n"])) for c in S3 if f"_{st_}_" in c and "FILE" not in c]
    check(f"report: ns per comparison style {st_} min (rounded)", lo_, round(min(v)), 1)
    check(f"report: ns per comparison style {st_} max (rounded)", hi_, round(max(v)), 1)
# SAG-C diagnostic
sc = {cid: boot_ci(ratios(c, "sagc")) for cid, c in S3.items()}
check("report: SAG-C diagnostic gain-set wins", 12, sum(win(sc[c]) for c in gate), 0)
check("report: SAG-C FILE natural ratio", 1.276, round(sc["S3H_FILE_natural_n15"][0], 3))
check("report: SAG-C severe cells == [FILE natural]", 1, int([c for c in S3 if sev(sc[c])] == ["S3H_FILE_natural_n15"]), 0)

# ======================================================================================================================
print("=== STAGE 2 (stage2/results/string_heldout.csv, SAG-C = 'sagc') ===")
S2 = load(ROOT / "stage2/results/string_heldout.csv")
check("S2 cells", 18, len(S2), 0)
low2 = [c for c in S2 if low(S2[c])]
check("S2 low-entropy cells", 8, len(low2), 0)
r2c = {c: boot_ci(ratios(S2[c], "sagc")) for c in S2}
r2s = {c: boot_ci(ratios(S2[c], "sag")) for c in S2}
check("S2 SAG-C gain: low-entropy cells <= 0.90 with CI upper < 1", 6, sum(win(r2c[c]) for c in low2), 0)
check("S2 SAG round-1 gain: low-entropy cells <= 0.90 with CI upper < 1", 6, sum(win(r2s[c]) for c in low2), 0)
sev_c = sorted(c for c in S2 if sev(r2c[c]))
check("S2 SAG-C severe cells == [STRHELD_runs64_n17]", 1, int(sev_c == ["STRHELD_runs64_n17"]), 0)
if "STRHELD_runs64_n17" in r2c:
    t = r2c["STRHELD_runs64_n17"]
    check("S2 SAG-C runs(64) n17 ratio", 1.114, round(t[0], 3)); check("S2 SAG-C runs(64) n17 CI low", 1.104, round(t[1], 3)); check("S2 SAG-C runs(64) n17 CI high", 1.150, round(t[2], 3))
check("S2 SAG round-1: severe cells", 0, sum(sev(r2s[c]) for c in S2), 0)

# ======================================================================================================================
print("=== STAGE 1 round 1 (results/raw/heldout.csv) ===")
R1 = load(ROOT / "results/raw/heldout.csv")
H = {c: v for c, v in R1.items() if c.startswith("HELD_")}
check("R1 held-out cells", 72, len(H), 0)
check("R1 sweep cells (BEVEN_)", 18, sum(c.startswith("BEVEN_") for c in R1), 0)
tt = {c: boot_ci(ratios(v, "sag")) for c, v in H.items()}
cm = {c: v["cnt"]["sag"] / v["cnt"]["base"] for c, v in H.items()}
lowH = [c for c in H if low(H[c])]
heavy_low = [c for c in lowH if "_heavy_" in c]
cheap = [c for c in H if "_plain_" in c or "_indirect_" in c]
check("R1 low-entropy heavy cells", 8, len(heavy_low), 0)
check("R1 cheap cells (plain + indirect)", 54, len(cheap), 0)
check("R1 heavy low-entropy time ratio min", 0.794, round(min(tt[c][0] for c in heavy_low), 3))
check("R1 heavy low-entropy time ratio max", 0.887, round(max(tt[c][0] for c in heavy_low), 3))
check("R1 heavy low-entropy cells with <= 0.90 and CI upper < 1", 8, sum(win(tt[c]) for c in heavy_low), 0)
check("R1 worst heavy cell (all 18 heavy held-out)", 1.022, round(max(tt[c][0] for c in H if "_heavy_" in c), 3))
check("R1 cheap time ratio min", 1.005, round(min(tt[c][0] for c in cheap), 3))
check("R1 cheap time ratio max", 1.240, round(max(tt[c][0] for c in cheap), 3))
check("R1 cheap cells slower than base (median ratio > 1)", 54, sum(tt[c][0] > 1 for c in cheap), 0)
check("R1 cheap severe cells", 12, sum(sev(tt[c]) for c in cheap), 0)
lowc = [c for c in lowH if "_plain_" in c]
check("R1 low-entropy cell shapes (plain regime, one per family/size)", 12, len(lowc), 0)
check("R1 max comparison ratio SAG/base (all held-out cells)", 1.0, round(max(cm.values()), 3), 0.0006)

print("=== STAGE 1 round 2 (results/raw/heldout2.csv, SAG-C = 'sagt') ===")
R2 = load(ROOT / "results/raw/heldout2.csv")
check("R2 cells", 54, len(R2), 0)
t2 = {c: boot_ci(ratios(v, "sagt")) for c, v in R2.items()}
h2 = [c for c in R2 if "_heavy_" in c and low(R2[c])]
c2 = [c for c in R2 if ("_plain_" in c or "_indirect_" in c)]
check("R2 heavy low-entropy cells", 8, len(h2), 0)
check("R2 heavy low-entropy ratio min", 0.725, round(min(t2[c][0] for c in h2), 3))
check("R2 heavy low-entropy ratio max", 0.863, round(max(t2[c][0] for c in h2), 3))
check("R2 cheap cells", 36, len(c2), 0)
check("R2 cheap cells with ratio >= 1.10", 2, sum(t2[c][0] >= 1.10 for c in c2), 0)
check("R2 cheap ratios >= 1.10 are 1.103 and 1.120", 1, int(sorted(round(t2[c][0], 3) for c in c2 if t2[c][0] >= 1.10) == [1.103, 1.120]), 0)

print()
print("RECOMPUTE SUMMARY:", "ALL MATCH" if not bad else f"{len(bad)} MISMATCH: {bad}")
sys.exit(1 if bad else 0)
