#!/usr/bin/env python3
"""STAGE-2 INDEPENDENT AUDIT CHECKER.

Deliberately does NOT import or call scripts/final_audit.py, scripts/analyze.py or scripts/run_bench.py.
Everything is re-derived from (a) the raw CSV files, (b) the run logs, (c) the freeze records and (d) the written protocol
(docs/EXPERIMENT_PLAN.md).  The expected experimental design is TRANSCRIBED here from the plan; the transcription is checked
against the frozen manifest digest, so a transcription error would show up as a digest mismatch.

    python -I stage2/audit/independent_audit.py            -> stage2/audit/independent_audit_output.txt (+ .json)

Result levels: PASS / FAIL / DISCREPANCY (a documented difference needing explanation) / INFO.
"""
import csv, hashlib, json, math, random, re, sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "stage2" / "audit"
lines, tally = [], Counter()


def rec(level, msg):
    tally[level] += 1
    lines.append(f"[{level}] {msg}")
    print(lines[-1], flush=True)


def sha_file(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()


# ------------------------------------------------------------------------------------------------ expected designs
def cell_name(prefix, fam, p, n, elem):
    return f"{prefix}_{elem}_{fam}{('%g' % p).replace('.', 'p')}_n{n.bit_length() - 1}"


FAMS = [("perm", 0), ("few", 2), ("few", 16), ("few", 256), ("few", 4096), ("zipf", 1024), ("mixture", 64), ("runs", 64), ("nearly", 0.01)]


def expected_heldout():
    cells = []
    for fi, (fam, p) in enumerate(FAMS):
        for si, n in enumerate([1 << 17, 1 << 20, 1 << 22]):
            seed = 9001 + 10 * fi + si
            big = n >= (1 << 22)
            cells.append(dict(cell=cell_name("HELD", fam, p, n, "plain"), family=fam, param=p, n=n, seed=seed, elem="plain",
                              variants="naive,base,base_b,sag,tim_base,tim_sag,stdstable,topdown", reps=11 if big else 15, warmup=1))
            cells.append(dict(cell=cell_name("HELD", fam, p, n, "indirect"), family=fam, param=p, n=n, seed=seed, elem="indirect",
                              variants="naive,base,base_b,sag,stdstable", reps=11 if big else 15, warmup=1))
            if not big:
                cells.append(dict(cell=cell_name("HELD", fam, p, n, "heavy"), family=fam, param=p, n=n, seed=seed, elem="heavy",
                                  variants="naive,base,base_b,sag,tim_base,tim_sag", reps=9 if n >= (1 << 20) else 15, warmup=1))
    for fi, (fam, p) in enumerate([("few", 16), ("few", 256), ("perm", 0)]):
        for k in [0, 2, 5, 10, 20, 40]:
            cells.append(dict(cell=f"BEVEN_k{k}_{fam}{('%g' % p)}_n19", family=fam, param=p, n=1 << 19, seed=9001 + 500 + fi, elem="heavy",
                              variants="naive,base,base_b,sag", reps=11, warmup=1, heavy_k=k))
    return cells


def expected_heldout2():
    cells = []
    for fi, (fam, p) in enumerate(FAMS):
        for si, n in enumerate((1 << 17, 1 << 20)):
            seed = 109001 + 10 * fi + si
            for elem in ("plain", "indirect", "heavy"):
                reps = 15 if (n == (1 << 17) or elem != "heavy") else 9
                cells.append(dict(cell=cell_name("HELD2", fam, p, n, elem), family=fam, param=p, n=n, seed=seed, elem=elem,
                                  variants="naive,base,base_b,sag,sagt", reps=reps, warmup=1))
    return cells


def digest(cells):
    return hashlib.sha256(json.dumps(cells, sort_keys=True).encode()).hexdigest()


# ------------------------------------------------------------------------------------------------ CSV loading
HEADER = "kind,cell,family,param,n,seed,elem,variant,rep,order_pos,ns_total,ns_per_elem,ok,comparisons,run_cmp,merge_cmp,merges,trivial_merges,blocks,moves,max_buffer,H,Hstar,nruns,pin_cpu".split(",")


def load_csv(path):
    raw = Path(path).read_bytes()
    text = raw.decode("utf-8")
    physical = text.count("\n")
    rdr = list(csv.reader(text.splitlines()))  # tolerant of CRLF
    return raw, physical, rdr


def audit_raw(name, expected, var_slot):
    """Structural audit of results/raw/<name>.csv against the expected design.  Returns parsed data."""
    path = ROOT / "results" / "raw" / f"{name}.csv"
    raw, physical, grid = load_csv(path)
    hdr, body = grid[0], grid[1:]
    rec("PASS" if hdr == HEADER else "FAIL", f"{name}: header has the 25 documented columns")
    rec("PASS" if physical - 1 == len(body) else "FAIL", f"{name}: physical lines minus header ({physical - 1}) == parsed data records ({len(body)}); blank/partial lines: none" if physical - 1 == len(body) else f"{name}: line/record mismatch {physical - 1} vs {len(body)}")
    bad_w = [i for i, r in enumerate(body) if len(r) != 25]
    rec("PASS" if not bad_w else "FAIL", f"{name}: every record has 25 fields (bad: {len(bad_w)})")
    rows = [dict(zip(hdr, r)) for r in body]
    exp = {c["cell"]: c for c in expected}
    got_cells = Counter(r["cell"] for r in rows)
    missing = sorted(set(exp) - set(got_cells))
    extra = sorted(set(got_cells) - set(exp))
    rec("PASS" if not missing and not extra else "FAIL", f"{name}: cells present {len(got_cells)} / expected {len(exp)}; missing {missing}; unexpected {extra}")
    # duplicates
    keys = Counter((r["kind"], r["cell"], r["variant"], r["rep"]) for r in rows)
    dup = [k for k, v in keys.items() if v > 1]
    rec("PASS" if not dup else "FAIL", f"{name}: duplicate (kind,cell,variant,rep) keys: {len(dup)}")
    # expected row counts
    exp_count = sum(len(c["variants"].split(",")) for c in expected)
    exp_time = sum(len(c["variants"].split(",")) * c["reps"] for c in expected)
    kinds = Counter(r["kind"] for r in rows)
    rec("PASS" if (kinds["count"], kinds["time"]) == (exp_count, exp_time) else "FAIL",
        f"{name}: count rows {kinds['count']} (expected {exp_count}), time rows {kinds['time']} (expected {exp_time}), total {len(rows)} (header excluded)")
    # per-cell exhaustive check
    per_cell_problems = []
    by_cell = defaultdict(list)
    for r in rows:
        by_cell[r["cell"]].append(r)
    for cid, c in exp.items():
        rs = by_cell.get(cid, [])
        vs = c["variants"].split(",")
        for r in rs:
            for f, e in (("family", c["family"]), ("n", str(c["n"])), ("seed", str(c["seed"])), ("elem", c["elem"])):
                if r[f] != e:
                    per_cell_problems.append(f"{cid}: {f}={r[f]} expected {e}")
            if float(r["param"]) != float(c["param"]):
                per_cell_problems.append(f"{cid}: param {r['param']} vs {c['param']}")
        # one count row per variant, reps 0..R-1 per variant, order_pos a permutation per rep
        cr = [r for r in rs if r["kind"] == "count"]
        if sorted(r["variant"] for r in cr) != sorted(vs):
            per_cell_problems.append(f"{cid}: count rows variants {sorted(r['variant'] for r in cr)} != {sorted(vs)}")
        tr = [r for r in rs if r["kind"] == "time"]
        for v in vs:
            reps = sorted(int(r["rep"]) for r in tr if r["variant"] == v)
            if reps != list(range(c["reps"])):
                per_cell_problems.append(f"{cid}/{v}: reps {reps} != 0..{c['reps'] - 1}")
        for rep in range(c["reps"]):
            pos = sorted(int(r["order_pos"]) for r in tr if int(r["rep"]) == rep)
            if pos != list(range(len(vs))):
                per_cell_problems.append(f"{cid} rep {rep}: order_pos {pos} not a permutation of 0..{len(vs) - 1}")
        # meta constant within a cell
        for f in ("H", "Hstar", "nruns", "pin_cpu"):
            if len({r[f] for r in rs}) != 1:
                per_cell_problems.append(f"{cid}: column {f} not constant within cell")
    rec("PASS" if not per_cell_problems else "FAIL", f"{name}: per-cell design check (family/param/n/seed/elem, variants, reps 0..R-1 exactly once, order_pos permutations, constant meta): {len(per_cell_problems)} problems" + ("" if not per_cell_problems else " e.g. " + "; ".join(per_cell_problems[:5])))
    # correctness flags
    notok = [r for r in rows if r["ok"] != "1"]
    rec("PASS" if not notok else "FAIL", f"{name}: rows with ok != 1: {len(notok)} of {len(rows)}")
    # numeric sanity
    bad_num = []
    for r in rows:
        if r["kind"] == "time":
            ns, nspe, n = float(r["ns_total"]), float(r["ns_per_elem"]), int(r["n"])
            if not (ns > 0 and abs(ns / n - nspe) <= 1e-5 * nspe):
                bad_num.append((r["cell"], r["variant"], r["rep"]))
        else:
            cm, rc, mc = int(r["comparisons"]), int(r["run_cmp"]), int(r["merge_cmp"])
            if cm != rc + mc:
                bad_num.append((r["cell"], r["variant"], "cmp!=run+merge"))
    rec("PASS" if not bad_num else "FAIL", f"{name}: time rows have ns_total>0 and ns_per_elem == ns_total/n; count rows comparisons == run_cmp + merge_cmp (violations: {len(bad_num)})")
    # A/A determinism: base and base_b counts identical
    cnt = {(r["cell"], r["variant"]): int(r["comparisons"]) for r in rows if r["kind"] == "count"}
    aa_bad = [cid for cid in exp if (cid, "base") in cnt and (cid, "base_b") in cnt and cnt[(cid, "base")] != cnt[(cid, "base_b")]]
    rec("PASS" if not aa_bad else "FAIL", f"{name}: base and base_b (identical code) have identical comparison counts in every cell (violations: {len(aa_bad)})")
    return rows, exp


# ------------------------------------------------------------------------------------------------ statistics (re-implemented)
BOOT, SEED = 10000, 12345


def med(xs):
    s = sorted(xs)
    k = len(s)
    return s[k // 2] if k % 2 else 0.5 * (s[k // 2 - 1] + s[k // 2])


def boot_ci(ratios):
    """Percentile bootstrap of the median: 10,000 resamples, Python Random(12345), index 250 and 9749 of the sorted medians."""
    rng = random.Random(SEED)
    k = len(ratios)
    meds = sorted(med([ratios[rng.randrange(k)] for _ in range(k)]) for _ in range(BOOT))
    return med(ratios), meds[int(0.025 * BOOT)], meds[int(0.975 * BOOT) - 1]


def sign_ci(ratios, conf=0.95):
    """Exact distribution-free CI for the median (order statistics), the narrowest symmetric interval with coverage >= conf."""
    s = sorted(ratios)
    k = len(s)
    best = (1, k)  # 1-based ranks
    for r in range(1, k // 2 + 1):
        cover = sum(math.comb(k, j) for j in range(r, k - r + 1)) / 2 ** k  # P(X_(r) <= median <= X_(k-r+1))
        if cover >= conf:
            best = (r, k - r + 1)
        else:
            break
    return med(s), s[best[0] - 1], s[best[1] - 1]


def fmt(t):
    return f"{t[0]:.3f} [{t[1]:.3f}, {t[2]:.3f}]"


def build_cells(rows):
    cells = {}
    for r in rows:
        c = cells.setdefault(r["cell"], dict(family=r["family"], param=float(r["param"]), n=int(r["n"]), elem=r["elem"], Hs=float(r["Hstar"]),
                                              H=float(r["H"]), time=defaultdict(dict), cnt={}, ok=True))
        if r["ok"] != "1":
            c["ok"] = False
        if r["kind"] == "time":
            c["time"][r["variant"]][int(r["rep"])] = float(r["ns_per_elem"])
        else:
            c["cnt"][r["variant"]] = int(r["comparisons"])
    return cells


def ratio_stats(c, v, base="base"):
    reps = sorted(set(c["time"][v]) & set(c["time"][base]))
    return [c["time"][v][r] / c["time"][base][r] for r in reps]


def parse_summary_table(path):
    out = {}
    for ln in Path(path).read_text(encoding="utf-8").splitlines():
        if ln.startswith("| HELD"):
            f = [x.strip() for x in ln.strip().strip("|").split("|")]
            out[f[0]] = f
    return out


def ci_str(t):
    return fmt(t)


def audit_summary(name, cells, summary_md, sag_slot, r1_slot):
    """Recompute every number in the summary table (cell rows) and compare to the stored markdown."""
    stored = parse_summary_table(summary_md)
    mism, checked = [], 0
    results = {}
    for cid in sorted(cells):
        if cid.startswith("BEVEN_"):
            continue
        c = cells[cid]
        row = stored.get(cid)
        if row is None:
            mism.append(f"{cid}: not in stored summary")
            continue
        cmp_ratio = c["cnt"][sag_slot] / c["cnt"]["base"]
        ts = boot_ci(ratio_stats(c, sag_slot))
        aa = boot_ci(ratio_stats(c, "base_b"))
        nv = med(ratio_stats(c, "naive"))
        r1 = med(ratio_stats(c, r1_slot)) if r1_slot else None
        bns = med(list(c["time"]["base"].values()))
        results[cid] = dict(cmp=cmp_ratio, ts=ts, aa=aa, nv=nv, r1=r1, bns=bns, c=c)
        exp_cells = [f"{cmp_ratio:.3f}", ci_str(ts), (f"{r1:.3f}" if r1 is not None else "-"), ci_str(aa), f"{nv:.3f}", f"{bns:.1f}"]
        got_cells = [row[6], row[7], row[8], row[9], row[10], row[11]]
        for lab, e, g in zip(("cmp", "time", "r1", "A/A", "naive", "base_ns"), exp_cells, got_cells):
            checked += 1
            if e != g:
                mism.append(f"{cid} {lab}: recomputed '{e}' vs stored '{g}'")
    extra = sorted(set(stored) - {k for k in cells if not k.startswith("BEVEN_")})
    rec("PASS" if not mism and not extra else "FAIL",
        f"{name}: {checked} printed numbers in {summary_md.name} (cmp ratio, time ratio+95% CI, round-1 SAG, A/A+CI, naive, base ns/elem) re-derived from raw CSV; mismatches {len(mism)}; stored-only cells {extra}" + ("" if not mism else " e.g. " + "; ".join(mism[:4])))
    return results


def classify(results, slot_label):
    """Independent evaluation of the frozen criteria.  Returns dict of facts."""
    facts = {}
    low = {cid: r for cid, r in results.items() if r["c"]["Hs"] <= 0.5 * math.log2(r["c"]["n"])}
    # H1: distinct low (family,param,n) cells; counts must not depend on the regime
    by_key = defaultdict(set)
    for cid, r in low.items():
        by_key[(r["c"]["family"], r["c"]["param"], r["c"]["n"])].add(round(r["cmp"], 9))
    facts["cmp_regime_dependent_keys"] = [k for k, v in by_key.items() if len(v) > 1]
    h1_cells = {k: min(v) for k, v in by_key.items()}
    facts["H1"] = (sum(1 for v in h1_cells.values() if v <= 0.90), len(h1_cells))
    heavy_low = {cid: r for cid, r in low.items() if r["c"]["elem"] == "heavy"}
    facts["H2"] = (sum(1 for r in heavy_low.values() if r["ts"][0] <= 0.90 and r["ts"][2] < 1.0), len(heavy_low))
    sev = lambda regs: sorted(cid for cid, r in results.items() if r["c"]["elem"] in regs and r["ts"][0] >= 1.10 and r["ts"][1] > 1.0)
    facts["sev_heavy"], facts["sev_cheap"] = sev(("heavy",)), sev(("plain", "indirect"))
    facts["cmp_excess"] = sorted(cid for cid, r in results.items() if r["cmp"] > 1.03)
    facts["H4"] = [cid for cid, r in results.items() if not r["c"]["ok"]]
    timed = list(results.values())
    facts["default"] = (sum(1 for r in timed if r["ts"][0] <= 0.90 and r["ts"][2] < 1.0), len(timed))
    return facts


def need(n_pass, n_total):
    return n_total > 0 and n_pass >= math.ceil(2 * n_total / 3)


# ------------------------------------------------------------------------------------------------ main
def main():
    # ---------- 0. freeze records vs current files
    rec("INFO", "audit checker runs with: python " + sys.version.split()[0])
    for fr_name, label in (("freeze_record.txt", "round 1"), ("freeze_record_round2.txt", "round 2")):
        fr = (ROOT / "results" / "outputs" / fr_name).read_text()
        same, changed, missing = [], [], []
        for h, f in re.findall(r"([0-9a-f]{64}) \*(\S+)", fr):
            p = ROOT / f
            if not p.exists():
                missing.append(f)
            elif sha_file(p) == h:
                same.append(f)
            else:
                changed.append(f)
        rec("INFO", f"{label} freeze record: {len(same)} files byte-identical to the frozen hash; changed since freeze: {changed or 'none'}; missing: {missing or 'none'}")
        if label == "round 1":
            core = ("src/natural_merge_sort.hpp", "src/sag_policy.hpp", "src/generators.hpp", "src/entropy.hpp", "src/textbook_merge_sort.hpp")
            rec("PASS" if all(f in same for f in core) else "FAIL", "round 1: the library code under test (base, SAG, generators, entropy, textbook) is byte-identical to the round-1 freeze")
            r1_changed = changed
        else:
            rec("PASS" if "src/sag_time_policy.hpp" in same and "src/natural_merge_sort.hpp" in same else "FAIL", "round 2: SAG-C code (sag_time_policy.hpp) and the base are byte-identical to the round-2 freeze")
            rec("PASS" if not changed else "DISCREPANCY", f"round 2: all hashed files unchanged since freeze (changed: {changed or 'none'})")
    # original protocol text frozen
    plan = (ROOT / "docs" / "EXPERIMENT_PLAN.md").read_bytes()
    idx = plan.index(b"\n---\n\n# ADDENDUM")
    fr1 = (ROOT / "results" / "outputs" / "freeze_record.txt").read_text()
    h_plan1 = re.search(r"([0-9a-f]{64}) \*docs/EXPERIMENT_PLAN.md", fr1).group(1)
    rec("PASS" if hashlib.sha256(plan[:idx]).hexdigest() == h_plan1 else "FAIL", "round-1 protocol text (everything before the ADDENDUM) is byte-identical to the text frozen before the round-1 run")
    fr2 = (ROOT / "results" / "outputs" / "freeze_record_round2.txt").read_text()
    h_plan2 = re.search(r"([0-9a-f]{64}) \*docs/EXPERIMENT_PLAN.md", fr2).group(1)
    if hashlib.sha256(plan).hexdigest() == h_plan2:
        rec("PASS", "complete EXPERIMENT_PLAN.md (with addendum) is byte-identical to the text frozen before the round-2 run")
    else:
        log = (ROOT / "docs" / "EXPERIMENT_LOG.md").read_text(encoding="utf-8")
        documented = "Typo fix in EXPERIMENT_PLAN addendum" in log
        rec("DISCREPANCY", "complete EXPERIMENT_PLAN.md (with addendum) is NOT byte-identical to the text hashed in freeze_record_round2.txt; "
            + ("the EXPERIMENT_LOG (00:23:47) documents a text-only typo fix made 11 s AFTER the round-2 run started (run start 00:23:36); the pre-edit text was not retained, so 'no criterion changed' cannot be proven by hash. "
               "Mitigation: the round-2 criteria R2-A..R2-D are also encoded in scripts/analyze.py whose hash IS frozen and unchanged, and this audit re-derives the same verdict from the current plan text." if documented else "and NO log entry explains it"))

    # ---------- 1. expected design vs frozen digests
    e1, e2 = expected_heldout(), expected_heldout2()
    d1, d2 = digest(e1), digest(e2)
    f1 = re.search(r"sha256=([0-9a-f]{64})", fr1).group(1)
    f2 = re.search(r"sha256=([0-9a-f]{64})", fr2).group(1)
    rec("PASS" if d1 == f1 else "FAIL", f"design transcribed from the plan (round 1: {len(e1)} cells) has digest {d1[:16]}... == frozen digest {f1[:16]}...")
    rec("PASS" if d2 == f2 else "FAIL", f"design transcribed from the plan addendum (round 2: {len(e2)} cells) has digest {d2[:16]}... == frozen digest {f2[:16]}...")

    # ---------- 2. raw data structure
    rows1, exp1 = audit_raw("heldout", e1, "sag")
    rows2, exp2 = audit_raw("heldout2", e2, "sagt")

    # ---------- 3. run logs
    for name, exp in (("heldout", exp1), ("heldout2", exp2)):
        txt = (ROOT / "results" / "outputs" / f"{name}_runlog.txt").read_text(errors="replace")
        starts = re.findall(r"# run started (\S+ \S+) manifest=\S+ sha256=([0-9a-f]{64})", txt)
        fins = re.findall(r"# finished (\S+ \S+) failures=(\d+)", txt)
        fr_t = re.search(r"FREEZE RECORD(?: ROUND 2)? (\d{4}-\d\d-\d\d \d\d:\d\d:\d\d)", fr1 if name == "heldout" else fr2).group(1)
        first_start = starts[0][0]
        rec("PASS" if first_start > fr_t else "FAIL", f"{name}: first run start ({first_start}) is after the freeze record time ({fr_t})")
        rec("PASS" if all(s[1] == (d1 if name == 'heldout' else d2) for s in starts) else "FAIL", f"{name}: every run segment ({len(starts)}) logged the frozen manifest digest")
        rec("INFO", f"{name}: run segments {len(starts)}, finished markers {fins}")
        cmds = re.findall(r"--family (\S+) --param (\S+) --n (\d+) --seed (\d+) --reps (\d+) --warmup (\d+) --elem (\S+) --variants (\S+) --cell (\S+) --csv .*?--pin (\S+)(?: --heavy_k (\d+))?\n\s+exit=(-?\d+) wall=([\d.]+)s", txt)
        seen = Counter(c[8] for c in cmds)
        probs = []
        for fam, par, n, seed, reps, warm, elem, vs, cell, pin, hk, ex, wall in cmds:
            e = exp.get(cell)
            if e is None:
                probs.append(f"{cell}: not in design")
                continue
            if (fam, float(par), int(n), int(seed), int(reps), int(warm), elem, vs) != (e["family"], float(e["param"]), e["n"], e["seed"], e["reps"], e["warmup"], e["elem"], e["variants"]):
                probs.append(f"{cell}: logged command differs from design")
            if int(ex) != 0:
                probs.append(f"{cell}: exit={ex}")
            if e.get("heavy_k") is not None and (hk == "" or int(hk) != e["heavy_k"]):
                probs.append(f"{cell}: heavy_k mismatch")
        rec("PASS" if not probs and set(seen) == set(exp) else "FAIL", f"{name}: run log lists {len(cmds)} bench invocations covering {len(seen)}/{len(exp)} cells; every logged command equals the design and every exit status is 0; repeated cells: {[c for c, v in seen.items() if v > 1] or 'none'}; problems: {probs[:4] or 'none'}")

    # ---------- 4. summaries re-derived (independent bootstrap implementation)
    cells1 = build_cells(rows1)
    cells2 = build_cells(rows2)
    main1 = {k: v for k, v in cells1.items() if not k.startswith("BEVEN_")}
    res1 = audit_summary("round 1", main1, ROOT / "results" / "outputs" / "summary_heldout.md", "sag", None)
    res2 = audit_summary("round 2", cells2, ROOT / "results" / "outputs" / "summary_heldout2.md", "sagt", "sag")

    be = {}
    for ln in (ROOT / "results" / "outputs" / "summary_heldout.md").read_text(encoding="utf-8").splitlines():
        if ln.startswith("| BEVEN"):
            f = [x.strip() for x in ln.strip().strip("|").split("|")]
            be[f[0]] = f
    be_mis = []
    for cid, c in cells1.items():
        if not cid.startswith("BEVEN_"):
            continue
        f = be.get(cid)
        e = [cid.split("_")[1][1:], f"{c['cnt']['sag'] / c['cnt']['base']:.3f}", fmt(boot_ci(ratio_stats(c, 'sag'))), f"{med(ratio_stats(c, 'naive')):.3f}", f"{med(ratio_stats(c, 'base_b')):.3f}"]
        if f is None or f[1:6] != e:
            be_mis.append((cid, e, f))
    rec("PASS" if not be_mis and len(be) == 18 else "FAIL", f"round 1 break-even sweep table: {len(be)} of 18 rows re-derived from raw CSV; mismatches {len(be_mis)} {be_mis[:2]}")
    # ---------- 5. criteria, independently
    f1c = classify(res1, "sag")
    h1 = need(*f1c["H1"]); h2 = need(*f1c["H2"])
    h3h = not f1c["sev_heavy"] and not f1c["cmp_excess"]
    h3c = not f1c["sev_cheap"]
    h4 = not f1c["H4"]
    dflt = f1c["default"][0] > f1c["default"][1] / 2
    verdict1 = "NO-GO"
    if h1 and h2 and h4 and h3h:
        verdict1 = "SUCCESS" if (h3c and dflt) else "PARTIAL SUCCESS (conditional on comparator cost)"
    rec("INFO", f"round 1 (independent): H1 {f1c['H1']} -> {h1}; H2 {f1c['H2']} -> {h2}; H3a heavy severe {f1c['sev_heavy']} cmp excess {f1c['cmp_excess']} -> {h3h}; H3b cheap severe {len(f1c['sev_cheap'])}: {f1c['sev_cheap']} -> {h3c}; H4 {h4}; default criterion {f1c['default']} -> {dflt}")
    rec("PASS" if f1c["cmp_regime_dependent_keys"] == [] else "FAIL", f"round 1: comparison counts for a (family,size) are identical across plain/indirect/heavy regimes (assumption behind H1 de-duplication); violations {f1c['cmp_regime_dependent_keys']}")
    rec("PASS" if verdict1.startswith("PARTIAL") else "DISCREPANCY", f"round 1 verdict re-derived independently: {verdict1}  (stored verdict: PARTIAL SUCCESS, conditional on comparator cost)")
    f2c = classify(res2, "sagt")
    ra = need(*f2c["H2"]); rb = not f2c["sev_heavy"] and not f2c["sev_cheap"]; rd = not f2c["cmp_excess"]; rc = not f2c["H4"]
    v2 = "CORRECTIVE NO-GO"
    if ra and rc and rd:
        v2 = "CORRECTIVE SUCCESS" if rb else "CORRECTIVE PARTIAL"
    rec("INFO", f"round 2 (independent): R2-A {f2c['H2']} -> {ra}; R2-B severe {f2c['sev_heavy'] + f2c['sev_cheap']} -> {rb}; R2-C {rc}; R2-D excess {f2c['cmp_excess']} -> {rd}")
    rec("PASS" if v2 == "CORRECTIVE PARTIAL" else "DISCREPANCY", f"round 2 verdict re-derived independently: {v2}  (stored verdict: CORRECTIVE PARTIAL)")

    # ---------- 6. sensitivity of the verdicts to the CI method (distribution-free sign interval instead of bootstrap)
    def sens(results, label, heavy_win=True):
        flips = []
        for cid, r in results.items():
            c = r["c"]
            sg = sign_ci(ratio_stats(c, label))
            boot_win = r["ts"][0] <= 0.90 and r["ts"][2] < 1.0
            sign_win = sg[0] <= 0.90 and sg[2] < 1.0
            boot_sev = r["ts"][0] >= 1.10 and r["ts"][1] > 1.0
            sign_sev = sg[0] >= 1.10 and sg[1] > 1.0
            if boot_win != sign_win or boot_sev != sign_sev:
                flips.append((cid, "win" if boot_win != sign_win else "severe", fmt(r["ts"]), fmt(sg)))
        return flips
    for lbl, res, slot in (("round 1", res1, "sag"), ("round 2", res2, "sagt")):
        fl = sens(res, slot)
        rec("INFO", f"{lbl}: classification (win / severe regression) under the exact sign-interval CI instead of the bootstrap changes in {len(fl)} of {len(res)} cells: " + "; ".join(f"{a} [{b}] boot {c} vs sign {d}" for a, b, c, d in fl[:12]))

    # ---------- 7. numeric claims in docs/FINAL_REPORT.md
    claims = []
    def claim(text, computed, stored):
        ok = computed == stored
        claims.append((ok, text, computed, stored))
        rec("PASS" if ok else "DISCREPANCY", f"claim: {text}: recomputed {computed} | reported {stored}")
    heavy_tbl = {}
    for ln in (ROOT / "docs" / "FINAL_REPORT.md").read_text(encoding="utf-8").splitlines():
        m = re.match(r"\| (few\(\d+\)|zipf\(1024\)|few\(4096\) \(high H\*\)|mixture\(64\)|nearly\(0\.01\)|perm|runs\(64\)) \| ([\d.]+ \[[\d., ]+\]) \| ([\d.]+ \[[\d., ]+\]) \|", ln)
        if m:
            heavy_tbl[m.group(1).replace(" (high H*)", "")] = (m.group(2), m.group(3))
    fam_key = lambda fam, p: (f"{fam}({'%g' % p})" if fam not in ("perm",) else "perm")
    n_heavy_ok = 0
    for fam, p in FAMS:
        k = fam_key(fam, p)
        for ni, n in enumerate((17, 20)):
            cid = cell_name("HELD", fam, p, 1 << n, "heavy")
            comp = fmt(res1[cid]["ts"])
            rep = heavy_tbl.get(k, ("?", "?"))[ni]
            if comp == rep:
                n_heavy_ok += 1
            else:
                claims.append((False, f"heavy table {k} 2^{n}", comp, rep))
                rec("DISCREPANCY", f"claim: FINAL_REPORT heavy-regime table {k} 2^{n}: recomputed {comp} | reported {rep}")
    rec("PASS" if n_heavy_ok == 18 else "DISCREPANCY", f"FINAL_REPORT section 6 heavy-regime table: {n_heavy_ok} of 18 entries (ratio and CI) reproduced exactly")
    cheap1 = {cid: r for cid, r in res1.items() if r["c"]["elem"] in ("plain", "indirect")}
    claim("round 1: cells (plain+indirect) with SAG/base median > 1.0", sum(1 for r in cheap1.values() if r["ts"][0] > 1.0), 54)
    for reg, (mm, lo, hi) in (("plain", (1.066, 1.007, 1.133)), ("indirect", (1.079, 1.005, 1.240))):
        v = [r["ts"][0] for r in cheap1.values() if r["c"]["elem"] == reg]
        claim(f"round 1 {reg}: median / min / max of SAG/base", (round(med(v), 3), round(min(v), 3), round(max(v), 3)), (mm, lo, hi))
    claim("round 1: severe cells in plain+indirect", len(f1c["sev_cheap"]), 12)
    heavy_ratios = [r["ts"][0] for r in res1.values() if r["c"]["elem"] == "heavy"]
    claim("round 1: worst heavy-regime SAG/base (reported 'worst +2.2%')", round(max(heavy_ratios), 3), 1.022)
    low_cmp = sorted(round(v, 2) for v in {(r['c']['family'], r['c']['param'], r['c']['n']): r['cmp'] for r in res1.values() if r['c']['Hs'] <= 0.5 * math.log2(r['c']['n'])}.values())
    claim("round 1: low-H* comparison ratios min..max (reported 0.73-0.85)", (low_cmp[0], low_cmp[-1], len(low_cmp)), (0.73, 0.85, 12))
    cheap2 = {cid: r for cid, r in res2.items() if r["c"]["elem"] in ("plain", "indirect")}
    for reg, mm in (("plain", 1.015), ("indirect", 1.029)):
        v = [r["ts"][0] for r in cheap2.values() if r["c"]["elem"] == reg]
        claim(f"round 2 {reg}: median SAG-C/base", round(med(v), 3), mm)
    for reg, share in (("plain", 0.11), ("indirect", 0.17)):
        v = [r["ts"][0] for r in cheap2.values() if r["c"]["elem"] == reg]
        claim(f"round 2 {reg}: share of cells with ratio above 1.05 (rounded)", round(sum(1 for x in v if x > 1.05) / len(v), 2), share)
    for reg, mm in (("plain", 1.042), ("indirect", 1.047)):
        v = [med(ratio_stats(r["c"], "sag")) for r in cheap2.values() if r["c"]["elem"] == reg]
        claim(f"round 2 {reg}: round-1 SAG median ratio on the fresh seeds", round(med(v), 3), mm)
    claim("round 2: max round-1 SAG ratio on cheap cells", round(max(med(ratio_stats(r["c"], "sag")) for r in cheap2.values()), 3), 1.163)
    claim("round 2: severe cells", sorted(f2c["sev_heavy"] + f2c["sev_cheap"]), ["HELD2_indirect_few2_n17", "HELD2_indirect_nearly0p01_n17"])
    sev_vals = {cid: fmt(res2[cid]["ts"]) for cid in ("HELD2_indirect_few2_n17", "HELD2_indirect_nearly0p01_n17")}
    claim("round 2: severe cell few(2)/2^17 ratio+CI", sev_vals["HELD2_indirect_few2_n17"], "1.103 [1.032, 1.166]")
    claim("round 2: severe cell nearly(0.01)/2^17 ratio+CI", sev_vals["HELD2_indirect_nearly0p01_n17"], "1.120 [1.063, 1.190]")
    hv2 = sorted(round(r["ts"][0], 3) for r in res2.values() if r["c"]["elem"] == "heavy" and r["c"]["Hs"] <= 0.5 * math.log2(r["c"]["n"]))
    claim("round 2: heavy low-H* SAG-C/base min..max (reported 0.725-0.863, 8 cells)", (hv2[0], hv2[-1], len(hv2)), (0.725, 0.863, 8))
    claim("round 2: max comparison ratio SAG-C/base (reported never above 1.001)", round(max(r["cmp"] for r in res2.values()), 3), 1.001)
    claim("generic criterion round 2: cells >= 10% faster with CI<1 (reported 12 of 54)", f2c["default"], (12, 54))
    aa = re.search(r"0\.986 \[0\.850, 1\.042\]", (ROOT / "docs" / "FINAL_REPORT.md").read_text(encoding="utf-8"))
    claim("round 2: A/A control for indirect nearly(0.01) 2^17 as quoted in the report", fmt(res2["HELD2_indirect_nearly0p01_n17"]["aa"]), "0.986 [0.850, 1.042]")

    # ---------- 8. A/A noise level (identical code run twice): how large can a null ratio look?
    for lbl, res in (("round 1", res1), ("round 2", res2)):
        spread = [(abs(r["aa"][0] - 1), cid) for cid, r in res.items()]
        spread.sort(reverse=True)
        fp = [cid for cid, r in res.items() if (r["aa"][0] >= 1.10 and r["aa"][1] > 1.0) or (r["aa"][0] <= 0.90 and r["aa"][2] < 1.0)]
        rec("INFO", f"{lbl}: A/A control (base_b/base): largest |median-1| = {spread[0][0]:.3f} in {spread[0][1]}; cells where the SAME code would be classed 'severe' or 'win' by the frozen rule: {fp or 'none'}")

    # ---------- 9. other CSVs: ok flags and shape only
    for n in ("dev_counts", "dev_time", "dev_sagt", "dev_selection", "dev_sagt_v1_timemodel_NEGATIVE", "dev_sagt_v2_breakeven"):
        raw, physical, grid = load_csv(ROOT / "results" / "raw" / f"{n}.csv")
        body = grid[1:]
        rec("PASS" if all(len(r) == 25 for r in body) and all(r[12] == "1" for r in body) and physical - 1 == len(body) else "FAIL",
            f"{n}.csv: {len(body)} records == {physical - 1} lines, all 25 fields, all ok == 1")

    # ---------- summary
    lines.append("")
    lines.append(f"SUMMARY: PASS {tally['PASS']}  FAIL {tally['FAIL']}  DISCREPANCY {tally['DISCREPANCY']}  INFO {tally['INFO']}")
    print(lines[-1])
    (OUT / "independent_audit_output.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
    (OUT / "independent_audit_output.json").write_text(json.dumps(dict(tally=tally, claims=[dict(ok=a, claim=b, recomputed=str(c), reported=str(d)) for a, b, c, d in claims]), indent=1), encoding="utf-8")
    return 1 if tally["FAIL"] else 0


if __name__ == "__main__":
    sys.exit(main())
