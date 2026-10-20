#!/usr/bin/env python3
"""Analysis of raw benchmark CSVs -> markdown summary + pre-registered criteria evaluation.

    python scripts/analyze.py results/raw/heldout.csv                 # writes results/outputs/summary_heldout.md
    python scripts/analyze.py results/raw/dev_time.csv --no-criteria  # descriptive table only

Definitions (frozen in docs/EXPERIMENT_PLAN.md):
  time ratio of a cell  = median over repetitions of [ time(SAG) / time(base) ] measured in the SAME repetition
                          (paired; the order of variants is randomised per repetition)
  95 % CI               = percentile bootstrap, 10,000 resamples of the repetition indices, RNG seed 12345
  A/A noise control     = the same statistic for base_b / base (identical code run twice)
  comparison ratio      = comparisons(SAG) / comparisons(base) from the un-timed counting pass (deterministic)
  low-H* cell           = dual-run entropy H* <= 0.5 * log2(n)   (a structural property of the input, not of timings)
Uses only the Python standard library (+ optional numpy if present is NOT required).
"""
import csv
import math
import random
import statistics as st
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BOOT = 10000
SEED = 12345


def median_ratio_ci(a, b):
    """a, b: paired lists. Returns (median ratio, lo, hi)."""
    r = [x / y for x, y in zip(a, b)]
    rng = random.Random(SEED)
    k = len(r)
    meds = []
    for _ in range(BOOT):
        meds.append(st.median(r[rng.randrange(k)] for _ in range(k)))
    meds.sort()
    return st.median(r), meds[int(0.025 * BOOT)], meds[int(0.975 * BOOT) - 1]


def load(path):
    cells = {}
    for row in csv.DictReader(open(path, newline="")):
        c = cells.setdefault(row["cell"], dict(meta={}, time=defaultdict(dict), count={}, ok=True))
        c["meta"] = dict(family=row["family"], param=float(row["param"]), n=int(row["n"]), elem=row["elem"], H=float(row["H"]),
                         Hs=float(row["Hstar"]), seed=int(row["seed"]))
        if row["ok"] != "1":
            c["ok"] = False
        if row["kind"] == "time":
            c["time"][row["variant"]][int(row["rep"])] = float(row["ns_per_elem"])
        else:
            c["count"][row["variant"]] = int(row["comparisons"])
    return cells


def fmt_ci(m, lo, hi):
    return f"{m:.3f} [{lo:.3f}, {hi:.3f}]"


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    criteria = "--no-criteria" not in sys.argv
    path = Path(args[0])
    name = path.stem
    cells = load(path)
    beven = {cid: c for cid, c in cells.items() if cid.startswith("BEVEN_")}  # descriptive sweep, excluded from criteria
    cells = {cid: c for cid, c in cells.items() if not cid.startswith("BEVEN_")}
    var = next((sys.argv[i + 1] for i, a in enumerate(sys.argv) if a == "--var"), None)
    round2 = "--round2" in sys.argv
    if var:  # analyse another variant in the 'sag' slot; the original 'sag' stays available as 'sag_r1'
        for c in cells.values():
            for d in (c["time"], c["count"]):
                if "sag" in d:
                    d["sag_r1"] = d.pop("sag")
                if var in d:
                    d["sag"] = d.pop(var)
    rows = []
    for cid in sorted(cells):
        c = cells[cid]
        m = c["meta"]
        lowH = m["Hs"] <= 0.5 * math.log2(m["n"])
        t = c["time"]
        rec = dict(cell=cid, regime=m["elem"], family=m["family"], param=m["param"], n=m["n"], H=m["H"], Hs=m["Hs"], low=lowH, ok=c["ok"])
        if "base" in c["count"] and "sag" in c["count"]:
            rec["cmp_ratio"] = c["count"]["sag"] / c["count"]["base"]
            rec["cmp_naive_ratio"] = c["count"]["naive"] / c["count"]["base"] if "naive" in c["count"] else None
        for v in ("sag", "sag_r1", "base_b", "naive", "tim_sag", "stdstable", "topdown", "sag_h8"):
            if v in t and "base" in t:
                reps = sorted(set(t[v]) & set(t["base"]))
                if reps:
                    base_ref = "base" if v != "tim_sag" else "tim_base"
                    if base_ref not in t:
                        continue
                    reps = sorted(set(t[v]) & set(t[base_ref]))
                    rec["t_" + v] = median_ratio_ci([t[v][r] for r in reps], [t[base_ref][r] for r in reps])
        if "base" in t:
            rec["base_ns"] = st.median(t["base"].values())
        rows.append(rec)

    out = []
    out.append(f"# Summary of `{path.name}`\n")
    out.append(f"Cells: {len(rows)}.  Statistic: median over repetitions of the paired ratio variant/base (bootstrap 95 % CI, {BOOT} resamples, seed {SEED}).\n")
    if var:
        out.append(f"Variant under test in the 'SAG' columns: `{var}`; the round-1 SAG is shown as 'round-1 SAG'.\n")
    out.append("| cell | regime | family | n | H*/log2 n | class | cmp SAG/base | time SAG/base [95% CI] | round-1 SAG time/base | A/A base_b/base [95% CI] | time naive/base | base ns/elem |")
    out.append("|---|---|---|---|---|---|---|---|---|---|---|---|")
    for r in rows:
        ts = r.get("t_sag"); aa = r.get("t_base_b"); nv = r.get("t_naive"); r1 = r.get("t_sag_r1")
        out.append("| {cell} | {regime} | {fam}{p} | 2^{ln} | {h:.2f} | {cls} | {cr} | {ts} | {r1} | {aa} | {nv} | {bn} |".format(
            r1=(f"{r1[0]:.3f}" if r1 else "-"),
            cell=r["cell"], regime=r["regime"], fam=r["family"], p=("" if r["family"] in ("perm", "sorted") else f"({r['param']:g})"),
            ln=r["n"].bit_length() - 1, h=r["Hs"] / math.log2(r["n"]), cls="low-H*" if r["low"] else "high-H*",
            cr=(f"{r['cmp_ratio']:.3f}" if "cmp_ratio" in r else "-"), ts=(fmt_ci(*ts) if ts else "-"), aa=(fmt_ci(*aa) if aa else "-"),
            nv=(f"{nv[0]:.3f}" if nv else "-"), bn=(f"{r['base_ns']:.1f}" if "base_ns" in r else "-")))
    out.append("")

    if criteria:
        low = [r for r in rows if r["low"]]
        failed_verif = [r["cell"] for r in rows if not r["ok"]]
        # H1: comparisons, distinct (family, param, n) low cells (counts are identical across regimes)
        seen, h1cells = set(), []
        for r in low:
            key = (r["family"], r["param"], r["n"])
            if key in seen or "cmp_ratio" not in r:
                continue
            seen.add(key)
            h1cells.append(r)
        h1_pass_n = sum(1 for r in h1cells if r["cmp_ratio"] <= 0.90)
        # H2: heavy-regime time on low-H* cells
        h2cells = [r for r in low if r["regime"] == "heavy" and "t_sag" in r]
        h2_pass_n = sum(1 for r in h2cells if r["t_sag"][0] <= 0.90 and r["t_sag"][2] < 1.0)
        # H3: guards, evaluated per regime class exactly as written in docs/EXPERIMENT_PLAN.md section 7
        def severe_in(regimes):
            return [r["cell"] for r in rows if r["regime"] in regimes and "t_sag" in r and r["t_sag"][0] >= 1.10 and r["t_sag"][1] > 1.0]
        sev_heavy = severe_in(("heavy",))
        sev_cheap = severe_in(("plain", "indirect"))
        cmp_bad = [r["cell"] for r in rows if "cmp_ratio" in r and r["cmp_ratio"] > 1.03]
        # secondary "default" criterion: >= 10 % faster (CI excludes 1) on a majority of ALL cells with timing
        timed = [r for r in rows if "t_sag" in r]
        dflt_n = sum(1 for r in timed if r["t_sag"][0] <= 0.90 and r["t_sag"][2] < 1.0)
        # exploratory: low-H* cells in plain / indirect regimes
        expl = {}
        for reg in ("plain", "indirect"):
            cs = [r for r in low if r["regime"] == reg and "t_sag" in r]
            expl[reg] = (sum(1 for r in cs if r["t_sag"][0] <= 0.90 and r["t_sag"][2] < 1.0), len(cs))
        aa_max = max((abs(r["t_base_b"][0] - 1) for r in rows if "t_base_b" in r), default=float("nan"))

        h1 = len(h1cells) > 0 and h1_pass_n >= math.ceil(2 * len(h1cells) / 3)
        h2 = len(h2cells) > 0 and h2_pass_n >= math.ceil(2 * len(h2cells) / 3)
        h3_heavy = not sev_heavy and not cmp_bad
        h3_cheap = not sev_cheap
        h4 = not failed_verif
        dflt = len(timed) > 0 and dflt_n > len(timed) / 2
        out.append("## Pre-registered criteria (docs/EXPERIMENT_PLAN.md)\n")
        out.append("| criterion | result | detail |")
        out.append("|---|---|---|")
        out.append(f"| H1  comparisons: >= 10 % fewer than base in >= 2/3 of low-H* cells | **{'PASS' if h1 else 'FAIL'}** | {h1_pass_n} of {len(h1cells)} distinct low-H* (family,size) cells |")
        out.append(f"| H2  heavy-comparator time: >= 10 % faster (CI < 1) in >= 2/3 of low-H* cells | **{'PASS' if h2 else 'FAIL'}** | {h2_pass_n} of {len(h2cells)} cells |")
        n_cheap = sum(1 for r in rows if r["regime"] in ("plain", "indirect") and "t_sag" in r)
        out.append(f"| H3a guard, heavy regime: no severe time regression (ratio >= 1.10 with CI lower bound > 1); comparisons never > 1.03x base (all cells) | **{'PASS' if h3_heavy else 'FAIL'}** | severe: {sev_heavy or 'none'}; comparison excess: {cmp_bad or 'none'} |")
        out.append(f"| H3b guard, cheap regimes (plain, indirect): no severe time regression | **{'PASS' if h3_cheap else 'FAIL'}** | {len(sev_cheap)} severe cells of {n_cheap}: {sev_cheap or 'none'} |")
        out.append(f"| H4  correctness: every output matched the reference | **{'PASS' if h4 else 'FAIL'}** | failures: {failed_verif or 'none'} |")
        out.append(f"| Secondary (default brief criterion): >= 10 % faster on a majority of ALL timed cells | **{'PASS' if dflt else 'FAIL'}** | {dflt_n} of {len(timed)} cells |")
        out.append(f"| Exploratory (not gating): low-H* time wins, plain keys | {expl['plain'][0]} of {expl['plain'][1]} | cheap comparisons |")
        out.append(f"| Exploratory (not gating): low-H* time wins, indirect keys | {expl['indirect'][0]} of {expl['indirect'][1]} | cache-missing comparisons |")
        out.append(f"| A/A noise: largest |median(base_b/base) - 1| over cells | {aa_max:.3f} | identical code run twice |")
        verdict = "NO-GO"
        if h1 and h2 and h4 and h3_heavy:
            verdict = "SUCCESS" if (h3_cheap and dflt) else "PARTIAL SUCCESS (conditional on comparator cost)"
        out.append(f"\n**Verdict under the round-1 rules: {verdict}**\n")
        if round2:
            # Corrective-round rules (docs/EXPERIMENT_PLAN.md, addendum): A gain retained, B no regret anywhere, C correctness, D comparisons
            ra, rb, rc, rd = h2, (not sev_heavy and not sev_cheap), h4, not cmp_bad
            out.append("## Corrective-round criteria (addendum to docs/EXPERIMENT_PLAN.md; variant under test in the 'SAG' columns)\n")
            out.append("| criterion | result | detail |")
            out.append("|---|---|---|")
            out.append(f"| R2-A gain retained: heavy regime, low-H* cells >= 10 % faster (CI < 1) in >= 2/3 | **{'PASS' if ra else 'FAIL'}** | {h2_pass_n} of {len(h2cells)} |")
            out.append(f"| R2-B no regret: no severe regression (ratio >= 1.10, CI lower > 1) in ANY cell of ANY regime | **{'PASS' if rb else 'FAIL'}** | severe: {(sev_heavy + sev_cheap) or 'none'} |")
            out.append(f"| R2-C correctness | **{'PASS' if rc else 'FAIL'}** | failures: {failed_verif or 'none'} |")
            out.append(f"| R2-D comparisons never > 1.03x base | **{'PASS' if rd else 'FAIL'}** | excess: {cmp_bad or 'none'} |")
            v2 = "CORRECTIVE NO-GO"
            if ra and rc and rd:
                v2 = "CORRECTIVE SUCCESS (no regret + gain when comparisons dominate)" if rb else "CORRECTIVE PARTIAL (regressions remain in some cells)"
            out.append(f"\n**Corrective-round verdict under its frozen rules: {v2}**\n")
    if beven:
        out.append("## Break-even sweep (descriptive, not part of the criteria): SAG/base time ratio vs comparator cost K (heavy chain of K steps), n = 2^19\n")
        out.append("| cell | K | cmp SAG/base | time SAG/base [95% CI] | time naive/base | A/A base_b/base |")
        out.append("|---|---|---|---|---|---|")
        for cid in sorted(beven, key=lambda x: (x.split("_")[2], int(x.split("_")[1][1:]))):
            c = beven[cid]
            t = c["time"]
            reps = sorted(set(t["sag"]) & set(t["base"]))
            ts = median_ratio_ci([t["sag"][r] for r in reps], [t["base"][r] for r in reps])
            nv = st.median(t["naive"][r] / t["base"][r] for r in reps)
            aa = st.median(t["base_b"][r] / t["base"][r] for r in reps)
            out.append("| {} | {} | {:.3f} | {} | {:.3f} | {:.3f} |".format(cid, cid.split("_")[1][1:], c["count"]["sag"] / c["count"]["base"], fmt_ci(*ts), nv, aa))
        out.append("")
    text = "\n".join(out)
    outp = ROOT / "results" / "outputs" / f"summary_{name}.md"
    outp.parent.mkdir(parents=True, exist_ok=True)
    outp.write_text(text, encoding="utf-8")
    print(text)


if __name__ == "__main__":
    main()
