#!/usr/bin/env python3
"""Analysis of the STAGE-2 string experiment.  Implements the decision rule frozen in docs/STAGE2_REAL_COMPARATOR_PLAN.md.

    python -I stage2/string_exp/analyze_string.py stage2/results/string_heldout.csv [--criteria]

Statistics (identical to the earlier rounds): per cell, time ratio = median over repetitions of time(variant)/time(base) of the SAME
repetition; 95 % percentile bootstrap CI (10,000 resamples, random.Random(12345), sorted-median indices 250 and 9749).
low-H* cell: dual-run entropy H* <= 0.5*log2(n)   (structural; read from the CSV, computed by the benchmark from the key array).
"""
import csv, math, random, sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BOOT, SEED = 10000, 12345
PRIMARY = "sagc"  # the single primary extension under test (frozen in the plan); "sag" is reported descriptively


def med(xs):
    s = sorted(xs)
    k = len(s)
    return s[k // 2] if k % 2 else 0.5 * (s[k // 2 - 1] + s[k // 2])


def boot_ci(r):
    rng = random.Random(SEED)
    k = len(r)
    meds = sorted(med([r[rng.randrange(k)] for _ in range(k)]) for _ in range(BOOT))
    return med(r), meds[int(0.025 * BOOT)], meds[int(0.975 * BOOT) - 1]


def fmt(t):
    return f"{t[0]:.3f} [{t[1]:.3f}, {t[2]:.3f}]"


def load(path):
    cells = {}
    for r in csv.DictReader(open(path, newline="")):
        c = cells.setdefault(r["cell"], dict(family=r["family"], param=float(r["param"]), n=int(r["n"]), Hs=float(r["Hstar"]), H=float(r["H"]), ok=True,
                                              time=defaultdict(dict), cnt={}, calib={}, cns={}))
        if r["ok"] != "1":
            c["ok"] = False
        if r["kind"] == "time":
            c["time"][r["variant"]][int(r["rep"])] = float(r["ns_per_elem"])
        else:
            c["cnt"][r["variant"]] = int(r["comparisons"])
            c["calib"][r["variant"]] = int(r["calib_cmp"] or 0)
            c["cns"][r["variant"]] = float(r["c_ns"] or 0)
    return cells


def ratios(c, v, base="base"):
    reps = sorted(set(c["time"][v]) & set(c["time"][base]))
    return [c["time"][v][r] / c["time"][base][r] for r in reps]


def main():
    path = Path(sys.argv[1])
    criteria = "--criteria" in sys.argv
    cells = load(path)
    rows = []
    for cid in sorted(cells):
        c = cells[cid]
        low = c["Hs"] <= 0.5 * math.log2(c["n"])
        r = dict(cell=cid, c=c, low=low)
        for v in ("sagc", "sag", "base_b", "naive", "tim_base", "stdstable", "topdown"):
            if v in c["time"]:
                r["t_" + v] = boot_ci(ratios(c, v))
        for v in ("sagc", "sag"):
            if v in c["cnt"] and "base" in c["cnt"]:
                r["cmp_" + v] = (c["cnt"][v] + c["calib"][v]) / c["cnt"]["base"]
                r["cmpx_" + v] = c["cnt"][v] / c["cnt"]["base"]
        r["base_ns"] = med(list(c["time"]["base"].values()))
        base_cmp = c["cnt"]["base"] / c["n"]
        r["ns_per_cmp"] = r["base_ns"] / base_cmp  # wall ns per comparison INCLUDING moves/overhead: an upper bound on comparator cost
        r["c_meas"] = c["cns"].get("sagc", 0.0)
        rows.append(r)

    out = [f"# Summary of `{path.name}` (STAGE-2 string-comparator experiment)\n",
           f"Cells: {len(rows)}.  Statistic: median over repetitions of the paired ratio variant/base (bootstrap 95 % CI, {BOOT} resamples, seed {SEED}).  Primary extension: `{PRIMARY}` (SAG-C); `sag` = round-1 SAG, descriptive.\n",
           "| cell | H*/log2 n | class | cmp SAG-C/base (incl. calibration) | cmp SAG/base | time SAG-C/base [95% CI] | time SAG/base [95% CI] | A/A base_b/base [95% CI] | naive/base | tim_base/base | stdstable/base | topdown/base | base ns/elem | base wall-ns per cmp | SAG-C measured c (ns) |",
           "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for r in rows:
        c = r["c"]
        g = lambda k: fmt(r[k]) if k in r else "-"
        m = lambda k: f"{r[k][0]:.3f}" if k in r else "-"
        out.append(f"| {r['cell']} | {c['Hs'] / math.log2(c['n']):.2f} | {'low-H*' if r['low'] else 'high-H*'} | {r.get('cmp_sagc', float('nan')):.3f} | {r.get('cmp_sag', float('nan')):.3f} | {g('t_sagc')} | {g('t_sag')} | {g('t_base_b')} | {m('t_naive')} | {m('t_tim_base')} | {m('t_stdstable')} | {m('t_topdown')} | {r['base_ns']:.1f} | {r['ns_per_cmp']:.1f} | {r['c_meas']:.1f} |")
    out.append("")
    if criteria:
        low = [r for r in rows if r["low"]]
        high = [r for r in rows if not r["low"]]
        bad_ok = [r["cell"] for r in rows if not r["c"]["ok"]]
        win = lambda t: t[0] <= 0.90 and t[2] < 1.0
        sev = lambda t: t[0] >= 1.10 and t[1] > 1.0
        res = {}
        for v in ("sagc", "sag"):
            nwin = sum(1 for r in low if win(r["t_" + v]))
            need = math.ceil(2 * len(low) / 3)
            sev_all = [r["cell"] for r in rows if sev(r["t_" + v])]
            sev_ctrl = [r["cell"] for r in high if sev(r["t_" + v])]
            cmp_bad = [r["cell"] for r in rows if r["cmpx_" + v] > 1.03]  # sorter-counted comparisons, as in round-2 R2-D; calibration calls are shown separately
            res[v] = dict(nwin=nwin, nlow=len(low), need=need, p2=len(low) > 0 and nwin >= need, sev_all=sev_all, sev_ctrl=sev_ctrl, cmp_bad=cmp_bad)
        out.append("## Frozen decision rule (docs/STAGE2_REAL_COMPARATOR_PLAN.md section 8)\n")
        out.append("| criterion | SAG-C (PRIMARY) | SAG (descriptive) |")
        out.append("|---|---|---|")
        for lab, key in (("P1 correctness: every output verified (idx sequence == reference AND order+stability scan)", None),):
            out.append(f"| {lab} | **{'PASS' if not bad_ok else 'FAIL'}** failures: {bad_ok or 'none'} | same data |")
        P, S = res["sagc"], res["sag"]
        out.append(f"| P2 gain: >= 10 % faster (median ratio <= 0.90 AND CI upper < 1) in >= 2/3 of the low-H* cells | **{'PASS' if P['p2'] else 'FAIL'}** {P['nwin']} of {P['nlow']} (need {P['need']}) | {'pass' if S['p2'] else 'fail'}: {S['nwin']} of {S['nlow']} |")
        out.append(f"| P3 guard: no severe regression (ratio >= 1.10 AND CI lower > 1) in any control (high-H*) cell | **{'PASS' if not P['sev_ctrl'] else 'FAIL'}** {P['sev_ctrl'] or 'none'} | {'pass' if not S['sev_ctrl'] else 'fail'}: {S['sev_ctrl'] or 'none'} |")
        out.append(f"| P3b guard, all cells (low-H* too): no severe regression | {'PASS' if not P['sev_all'] else 'FAIL'} {P['sev_all'] or 'none'} | {S['sev_all'] or 'none'} |")
        out.append(f"| P4 comparisons counted by the sorter never > 1.03 x base (as R2-D; SAG-C calibration calls are reported separately in the table) | **{'PASS' if not P['cmp_bad'] else 'FAIL'}** {P['cmp_bad'] or 'none'} | {'pass' if not S['cmp_bad'] else 'fail'}: {S['cmp_bad'] or 'none'} |")
        aa = max(abs(r["t_base_b"][0] - 1) for r in rows)
        out.append(f"| A/A noise: largest abs(median(base_b/base)-1) | {aa:.3f} | |")
        def verdict_for(R):
            ok_all = not bad_ok and R["p2"] and not R["sev_all"] and not R["cmp_bad"]
            return "SUCCESS" if ok_all else ("NO-GO (hypothesis not supported)" if (bad_ok or not R["p2"]) else "GAIN WITHOUT SAFETY (guard failed)")
        out.append("")
        out.append(f"**Verdict under the frozen rule - PRIMARY (SAG-C): {verdict_for(P)}**")
        out.append("")
        out.append(f"**Verdict under the same rule - secondary (SAG; own verdict, not pooled): {verdict_for(S)}**")
        out.append("")
    text = "\n".join(out)
    outp = ROOT / "stage2" / "results" / f"summary_{path.stem}.md"
    outp.parent.mkdir(parents=True, exist_ok=True)
    outp.write_text(text, encoding="utf-8")
    print(text)


if __name__ == "__main__":
    main()
