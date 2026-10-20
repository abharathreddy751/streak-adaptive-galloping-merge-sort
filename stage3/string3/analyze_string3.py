#!/usr/bin/env python3
"""Analysis of the STAGE-3 confirmatory experiment.  Implements the decision rule frozen in docs/STAGE3_CONFIRMATION_PLAN.md.

    python -I stage3/string3/analyze_string3.py stage3/results/s3_heldout.csv [--criteria]

Statistics (identical to earlier stages): per cell, time ratio = median over repetitions of time(variant)/time(base) of the SAME repetition;
95 % percentile bootstrap CI (10,000 resamples, random.Random(12345), sorted-median indices 250 and 9749).
Cell classes (structural, from the generated data - never from timings):
  low-entropy  : dual-run entropy of the rank array  H* <= 0.5*log2(n)
  expensive-key: cost_B >= 32  (mean over adjacent pairs of the sorted array of min(LCP+1, shorter length) = bytes examined by a comparison)
  gain set G   : low-entropy AND expensive-key
Primary candidate: SAG (SagTT<64,true>).  SAG-C (frozen, not retuned) is shown as a secondary diagnostic only; it gets no verdict.
"""
import csv, math, random, sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BOOT, SEED = 10000, 12345
COST_B_MIN = 32.0


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
        c = cells.setdefault(r["cell"], dict(style=r["style"], family=r["family"], param=float(r["param"]), n=int(r["n"]), Hs=float(r["Hstar"]), cost_B=float(r["cost_B"]),
                                              mean_len=float(r["mean_len"]), ok=True, time=defaultdict(dict), cnt={}, calib={}, cns={}))
        if r["ok"] != "1":
            c["ok"] = False
        if r["kind"] == "time":
            c["time"][r["variant"]][int(r["rep"])] = float(r["ns_per_elem"])
        elif r["kind"] == "count":
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
        r = dict(cell=cid, c=c, low=c["Hs"] <= 0.5 * math.log2(c["n"]), exp=c["cost_B"] >= COST_B_MIN)
        r["gate"] = r["low"] and r["exp"]
        for v in ("sag", "sagc", "base_b", "naive", "tim_base", "stdstable", "topdown"):
            if v in c["time"]:
                r["t_" + v] = boot_ci(ratios(c, v))
        for v in ("sag", "sagc"):
            r["cmpx_" + v] = c["cnt"][v] / c["cnt"]["base"]
            r["cmp_" + v] = (c["cnt"][v] + c["calib"][v]) / c["cnt"]["base"]
        r["base_ns"] = med(list(c["time"]["base"].values()))
        r["ns_per_cmp"] = r["base_ns"] / (c["cnt"]["base"] / c["n"])
        rows.append(r)
    out = [f"# Summary of `{path.name}` (STAGE-3 confirmatory string experiment)\n",
           f"Cells: {len(rows)}.  Statistic: median over repetitions of the paired ratio variant/base (bootstrap 95 % CI, {BOOT} resamples, seed {SEED}). Primary candidate: `sag` (SagTT<64,true>); `sagc` = SAG-C frozen settings, secondary diagnostic only.\n",
           "| cell | class (low-H* / key cost) | H*/log2 n | mean len | cost_B | cmp SAG/base | time SAG/base [95% CI] | A/A base_b/base [95% CI] | naive/base | tim_base/base | stdstable/base | topdown/base | cmp SAG-C/base (incl. calib) | time SAG-C/base [95% CI] (diagnostic) | base ns/elem | base ns per cmp |",
           "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for r in rows:
        c = r["c"]
        g = lambda k: fmt(r[k]) if k in r else "-"
        m = lambda k: f"{r[k][0]:.3f}" if k in r else "-"
        cls = ("low" if r["low"] else "high") + "/" + ("EXP" if r["exp"] else "cheap") + (" GATE" if r["gate"] else "")
        out.append(f"| {r['cell']} | {cls} | {c['Hs'] / math.log2(c['n']):.2f} | {c['mean_len']:.1f} | {c['cost_B']:.1f} | {r['cmpx_sag']:.3f} | {g('t_sag')} | {g('t_base_b')} | {m('t_naive')} | {m('t_tim_base')} | {m('t_stdstable')} | {m('t_topdown')} | {r['cmp_sagc']:.3f} | {g('t_sagc')} | {r['base_ns']:.1f} | {r['ns_per_cmp']:.1f} |")
    out.append("")
    if criteria:
        gate = [r for r in rows if r["gate"]]
        low_all = [r for r in rows if r["low"]]
        bad_ok = [r["cell"] for r in rows if not r["c"]["ok"]]
        win = lambda t: t[0] <= 0.90 and t[2] < 1.0
        sev = lambda t: t[0] >= 1.10 and t[1] > 1.0
        nwin = [r["cell"] for r in gate if win(r["t_sag"])]
        need = math.ceil(2 * len(gate) / 3)
        p2 = len(gate) > 0 and len(nwin) >= need
        sev_all = [r["cell"] for r in rows if sev(r["t_sag"])]
        cmp_bad = [r["cell"] for r in rows if r["cmpx_sag"] > 1.03]
        out.append("## Frozen decision rule (docs/STAGE3_CONFIRMATION_PLAN.md section 8) - primary candidate SAG\n")
        out.append("| criterion | result | detail |")
        out.append("|---|---|---|")
        out.append(f"| P1 correctness: every output verified (record sequence == reference AND order+stability scan) | **{'PASS' if not bad_ok else 'FAIL'}** | failures: {bad_ok or 'none'} |")
        out.append(f"| P2 gain: >= 10 % faster (median ratio <= 0.90 AND CI upper < 1) in >= 2/3 of the gain-set cells (low-entropy AND expensive-key) | **{'PASS' if p2 else 'FAIL'}** | {len(nwin)} of {len(gate)} (need {need}); non-wins: {sorted(set(r['cell'] for r in gate) - set(nwin))} |")
        out.append(f"| P3 guard: no severe regression (ratio >= 1.10 AND CI lower > 1) in ANY of the {len(rows)} cells | **{'PASS' if not sev_all else 'FAIL'}** | {sev_all or 'none'} |")
        out.append(f"| P4 comparisons counted by the sorter never > 1.03 x base in any cell | **{'PASS' if not cmp_bad else 'FAIL'}** | {cmp_bad or 'none'} |")
        out.append(f"| (P4 independent verification of counts and manifests) | done by `stage3/audit/audit_stage3.py` | not decided here |")
        aa = max(abs(r["t_base_b"][0] - 1) for r in rows)
        out.append(f"| A/A noise: largest abs(median(base_b/base)-1) | {aa:.3f} | identical code run twice |")
        out.append(f"| descriptive: low-entropy cells (all key costs) with >= 10 % gain | {sum(1 for r in low_all if win(r['t_sag']))} of {len(low_all)} | |")
        out.append(f"| descriptive: SAG-C diagnostic (frozen, no verdict): gain-set wins / severe cells | {sum(1 for r in gate if win(r['t_sagc']))} of {len(gate)} / {[r['cell'] for r in rows if sev(r['t_sagc'])] or 'none'} | |")
        ok_all = not bad_ok and p2 and not sev_all and not cmp_bad
        verdict = "CONFIRMATION SUCCESS (candidate criteria P1-P4 met; independent verification pending)" if ok_all else (
            "CONFIRMATION NOT MET - gain without safety (P3 or P4 failed)" if (not bad_ok and p2) else "CONFIRMATION NOT MET")
        out.append("")
        out.append(f"**Verdict under the frozen rule (SAG): {verdict}**")
        out.append("")
    text = "\n".join(out)
    outp = ROOT / "stage3" / "results" / f"summary_{path.stem}.md"
    outp.parent.mkdir(parents=True, exist_ok=True)
    outp.write_text(text, encoding="utf-8")
    print(text)


if __name__ == "__main__":
    main()
