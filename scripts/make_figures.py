#!/usr/bin/env python3
"""Figures from the raw held-out CSV (same statistics as scripts/analyze.py).

    python scripts/make_figures.py results/raw/heldout.csv        -> results/figures/*.png

fig1_time_ratio_by_regime.png : SAG/base time ratio per held-out cell (dot = median of paired ratios, bar = 95 % bootstrap CI),
                                three panels = comparison regimes; filled dot = low-entropy cell, hollow = high-entropy cell.
fig2_comparison_ratio.png     : SAG/base comparison ratio per (family, size).
fig3_break_even.png           : SAG/base time ratio as a function of comparison cost (heavy-chain length K).
Colours: reference categorical palette slots 1-3 (validated with the dataviz validator); identity is also carried by
marker shapes and direct labels; every number is also in results/outputs/summary_*.md.
"""
import math
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).resolve().parent))
import analyze  # noqa: E402  (reuses load() and median_ratio_ci())

ROOT = Path(__file__).resolve().parent.parent
BLUE, ORANGE, AQUA = "#2a78d6", "#eb6834", "#1baf7a"
INK, MUTED, GRID, SURFACE = "#0b0b0b", "#52514e", "#e5e4df", "#fcfcfb"


def style(ax):
    ax.set_facecolor(SURFACE)
    ax.grid(axis="y", color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    for s in ("left", "bottom"):
        ax.spines[s].set_color(MUTED)
    ax.tick_params(colors=MUTED, labelsize=8)


def label(r):
    p = "" if r["family"] in ("perm", "sorted") else f"({r['param']:g})"
    return f"{r['family']}{p} 2^{r['n'].bit_length() - 1}"


def figure_round2(csv2, out):
    """fig4: round-2 corrective extension (SAG-C, blue circles) vs round-1 SAG (orange squares), per held-out2 cell."""
    cells = analyze.load(csv2)
    fig, axes = plt.subplots(1, 3, figsize=(15, 5.2), sharey=True, facecolor=SURFACE)
    titles = {"plain": "plain keys", "indirect": "indirect keys", "heavy": "heavy comparator (~22 ns, synthetic)"}
    for ax, reg in zip(axes, ("plain", "indirect", "heavy")):
        rows = []
        for cid, c in sorted(cells.items()):
            m = c["meta"]
            t = c["time"]
            if m["elem"] != reg or "sagt" not in t or "sag" not in t or "base" not in t:
                continue
            reps = sorted(set(t["sagt"]) & set(t["base"]))
            a = analyze.median_ratio_ci([t["sagt"][r] for r in reps], [t["base"][r] for r in reps])
            b = analyze.median_ratio_ci([t["sag"][r] for r in reps], [t["base"][r] for r in reps])
            rows.append((m["Hs"] / math.log2(m["n"]), m["n"], m, a, b))
        rows.sort(key=lambda r: (r[0], r[1]))
        style(ax)
        for i, (h, n, m, a, b) in enumerate(rows):
            ax.vlines(i - 0.12, a[1], a[2], color=BLUE, linewidth=1.5)
            ax.plot(i - 0.12, a[0], marker="o", markersize=6, color=BLUE)
            ax.vlines(i + 0.12, b[1], b[2], color=ORANGE, linewidth=1.5)
            ax.plot(i + 0.12, b[0], marker="s", markersize=5.5, color=ORANGE)
        ax.axhline(1.0, color=MUTED, linewidth=1.0)
        ax.axhline(1.1, color=MUTED, linewidth=1.0, linestyle=":")
        ax.set_xticks(range(len(rows)))
        ax.set_xticklabels([label(dict(family=r[2]["family"], param=r[2]["param"], n=r[1])) for r in rows], rotation=90, fontsize=7, color=MUTED)
        ax.set_title(titles[reg], fontsize=10, color=INK, loc="left")
    axes[0].set_ylabel("time / time(base)   (paired median, 95% CI)", color=INK, fontsize=9)
    axes[0].set_ylim(0.45, 1.3)
    axes[2].text(0.0, 0.52, "blue circle = SAG-C (corrective round)    orange square = round-1 SAG", color=INK, fontsize=8)
    axes[0].text(0.0, 1.105, "severe-regression line (1.10)", color=MUTED, fontsize=7, va="bottom")
    fig.suptitle("Fresh held-out seeds: corrective SAG-C vs round-1 SAG, both relative to the base paper policy", fontsize=11, color=INK, x=0.01, ha="left")
    fig.tight_layout(rect=(0, 0, 1, 0.94))
    fig.savefig(out / "fig4_round2_corrective.png", dpi=140, facecolor=SURFACE)
    plt.close(fig)


def main():
    csv_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "results" / "raw" / "heldout.csv"
    cells = analyze.load(csv_path)
    beven = {k: v for k, v in cells.items() if k.startswith("BEVEN_")}
    cells = {k: v for k, v in cells.items() if not k.startswith("BEVEN_")}
    out = ROOT / "results" / "figures"
    out.mkdir(parents=True, exist_ok=True)

    # ---- fig 1
    fig, axes = plt.subplots(1, 3, figsize=(15, 5.2), sharey=True, facecolor=SURFACE)
    titles = {"plain": "plain keys (cheap comparisons)", "indirect": "indirect keys (cache-missing comparisons)",
              "heavy": "heavy comparator (~22 ns per comparison, synthetic)"}
    for ax, reg in zip(axes, ("plain", "indirect", "heavy")):
        rows = []
        for cid, c in sorted(cells.items()):
            m = c["meta"]
            if m["elem"] != reg or "sag" not in c["time"] or "base" not in c["time"]:
                continue
            reps = sorted(set(c["time"]["sag"]) & set(c["time"]["base"]))
            med, lo, hi = analyze.median_ratio_ci([c["time"]["sag"][r] for r in reps], [c["time"]["base"][r] for r in reps])
            rows.append((m["Hs"] / math.log2(m["n"]), m["n"], m, med, lo, hi))
        rows.sort(key=lambda t: (t[0], t[1]))
        style(ax)
        for i, (h, n, m, med, lo, hi) in enumerate(rows):
            lowH = m["Hs"] <= 0.5 * math.log2(m["n"])
            ax.vlines(i, lo, hi, color=BLUE, linewidth=1.6)
            ax.plot(i, med, marker="o", markersize=6.5, markerfacecolor=BLUE if lowH else SURFACE, markeredgecolor=BLUE, markeredgewidth=1.6)
        ax.axhline(1.0, color=MUTED, linewidth=1.0)
        ax.axhline(0.9, color=ORANGE, linewidth=1.0, linestyle="--")
        ax.set_xticks(range(len(rows)))
        ax.set_xticklabels([label(dict(family=t[2]["family"], param=t[2]["param"], n=t[1])) for t in rows], rotation=90, fontsize=7, color=MUTED)
        ax.set_title(titles[reg], fontsize=10, color=INK, loc="left")
    axes[0].set_ylabel("time(SAG) / time(base)   (paired median, 95% CI)", color=INK, fontsize=9)
    axes[0].set_ylim(0.5, 1.45)
    axes[2].text(len(rows) - 0.5, 0.905, "10 % faster target", color=ORANGE, fontsize=8, ha="right", va="bottom")
    fig.suptitle("Held-out: proposed SAG vs base paper policy. Filled = low-entropy input (H* <= 0.5 log2 n); sorted left to right by H*/log2 n", fontsize=11, color=INK, x=0.01, ha="left")
    fig.tight_layout(rect=(0, 0, 1, 0.94))
    fig.savefig(out / "fig1_time_ratio_by_regime.png", dpi=140, facecolor=SURFACE)
    plt.close(fig)

    # ---- fig 2
    seen = {}
    for cid, c in cells.items():
        m = c["meta"]
        if "sag" in c["count"] and "base" in c["count"]:
            seen[(m["family"], m["param"], m["n"])] = (c["count"]["sag"] / c["count"]["base"], m["Hs"] / math.log2(m["n"]))
    keys = sorted(seen, key=lambda k: (seen[k][1], k[2]))
    fig, ax = plt.subplots(figsize=(11, 4.6), facecolor=SURFACE)
    style(ax)
    ax.bar(range(len(keys)), [seen[k][0] for k in keys], color=BLUE, width=0.7)
    ax.axhline(1.0, color=MUTED, linewidth=1.0)
    ax.axhline(0.9, color=ORANGE, linewidth=1.0, linestyle="--")
    ax.set_xticks(range(len(keys)))
    ax.set_xticklabels([label(dict(family=k[0], param=k[1], n=k[2])) for k in keys], rotation=90, fontsize=7, color=MUTED)
    ax.set_ylabel("comparisons(SAG) / comparisons(base)", color=INK, fontsize=9)
    ax.set_title("Held-out comparison counts (deterministic): SAG never uses more than the base", fontsize=10, color=INK, loc="left")
    fig.tight_layout()
    fig.savefig(out / "fig2_comparison_ratio.png", dpi=140, facecolor=SURFACE)
    plt.close(fig)

    # ---- fig 3
    if beven:
        fig, ax = plt.subplots(figsize=(7.5, 4.6), facecolor=SURFACE)
        style(ax)
        series = {}
        for cid, c in beven.items():
            parts = cid.split("_")
            k = int(parts[1][1:])
            fam = parts[2]
            reps = sorted(set(c["time"]["sag"]) & set(c["time"]["base"]))
            med, lo, hi = analyze.median_ratio_ci([c["time"]["sag"][r] for r in reps], [c["time"]["base"][r] for r in reps])
            series.setdefault(fam, []).append((k, med, lo, hi))
        for (fam, pts), col, mk in zip(sorted(series.items()), (BLUE, ORANGE, AQUA), ("o", "s", "^")):
            pts.sort()
            xs = [p[0] for p in pts]
            ax.plot(xs, [p[1] for p in pts], color=col, marker=mk, markersize=6, linewidth=1.8)
            ax.fill_between(xs, [p[2] for p in pts], [p[3] for p in pts], color=col, alpha=0.15, linewidth=0)
            ax.text(xs[-1] + 0.8, pts[-1][1], fam.replace("few", "few(").replace("perm0", "perm") + (")" if "few" in fam else ""), color=INK, fontsize=8, va="center")
        ax.axhline(1.0, color=MUTED, linewidth=1.0)
        ax.set_xlabel("comparator cost: length K of the dependent arithmetic chain (K=40 is about 22 ns)", color=INK, fontsize=9)
        ax.set_ylabel("time(SAG) / time(base)", color=INK, fontsize=9)
        ax.set_title("Break-even: SAG pays off once a comparison is expensive (n = 2^19)", fontsize=10, color=INK, loc="left")
        ax.set_xlim(-1, 48)
        fig.tight_layout()
        fig.savefig(out / "fig3_break_even.png", dpi=140, facecolor=SURFACE)
        plt.close(fig)
    if len(sys.argv) > 2:
        figure_round2(Path(sys.argv[2]), out)
    print("figures written to", out)


if __name__ == "__main__":
    main()
