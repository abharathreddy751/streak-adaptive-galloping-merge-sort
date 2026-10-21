# Streak-Adaptive Galloping for Natural Merge Sort

**MACSE501 topic:** Merge Sort (Module 3) — the project re-implements a 2025 research paper on the *merge step* of natural merge sorts (TimSort family), proposes a change to it, and evaluates both on identical inputs.
**Platform:** C++17, header-only, no dependencies. Tested with g++ 14.2.0 (Windows/MinGW) and, for sanitizers, g++ 15.2.0 (WSL Ubuntu).

## Base paper (verified)
E. Ghasemi, V. Jugé, G. Khalighinejad, H. Yazdanyar, **"Galloping in Fast-Growth Natural Merge Sorts"**, *Algorithmica* 87(2):242–291, Springer, online 2024-12-09 (issue Feb 2025). DOI [10.1007/s00453-024-01285-6](https://doi.org/10.1007/s00453-024-01285-6); preprint arXiv:2012.03996 (read in full-text form). Metadata cross-checked in Crossref and OpenAlex. **Scopus indexing was not independently verified** (see `docs/PAPER_VERIFICATION.md`). An earlier idea (succinct rank/select bit vectors) was *rejected as outside the MACSE501 syllabus* — see `docs/SYLLABUS_COMPLIANCE.md`.

## What was built
| part | where |
|---|---|
| **Base paper algorithm**: natural merge sorts (PowerSort and TimSort stack rules), run detection + binary-insertion `minrun`, McIlroy's mixed t-galloping merge, polylogarithmic `t = ⌈log₂(a+b)⌉²` | [src/natural_merge_sort.hpp](src/natural_merge_sort.hpp) |
| **Proposed extension**: Streak-Adaptive Galloping (SAG) — adapts `t` inside each merge from observed block lengths using the paper's own cost model; no extra memory or key comparisons | [src/sag_policy.hpp](src/sag_policy.hpp) |
| **Corrective-round extensions** SAG-T v1 (failed), SAG-B (failed on nearly-sorted), **SAG-C** (credit rule with self-measured comparison cost) | [src/sag_time_policy.hpp](src/sag_time_policy.hpp) |
| Textbook top-down Merge Sort (syllabus baseline / oracle), entropy tools (H, H*), deterministic generators | `src/textbook_merge_sort.hpp`, `src/entropy.hpp`, `src/generators.hpp` |
| Tests (exhaustive small cases, randomised families, paper-derived invariants) | `tests/` |
| Benchmark harness, manifests, analysis, figures | `bench/`, `scripts/` |

## Result in one paragraph
All outputs on all held-out cells (90 + 54) matched the reference sort. **Round 1 (primary):** SAG uses **15–27 % fewer comparisons** than the paper's policy on low-entropy inputs (up to 51 % on nearly-sorted data) and never more than the base. When a comparison is expensive (synthetic ≈ 22 ns comparator) this becomes **11–21 % less time on all 8 low-entropy cells** (44 % on nearly-sorted data), with no severe regression (worst +2.2 %). **With cheap comparisons (plain integers, cache-missing indices) SAG is slower than the base in all 54 cells (median +7–8 %, up to +24 %; 12 cells severe)** — the paper's cost model counts comparisons only, and the experiment confirms that it matters only when comparisons dominate. Verdict under the frozen protocol: **PARTIAL SUCCESS, conditional on comparator cost.** **Round 2 (one corrective experiment, fresh seeds):** a time-aware variant that measures the comparison cost itself (SAG-C) keeps and increases the gain when comparisons dominate (14–28 % on low-entropy cells, 47 % nearly-sorted) and is nearly neutral otherwise (median +1.5 % / +2.9 %), but 2 of 36 cheap-comparison cells still regress ≥ 10 %: **CORRECTIVE PARTIAL.** Details: `docs/FINAL_REPORT.md`.

## Stage 2 (2026-10-20): independent audit + real string comparator - additional evidence, verdicts above unchanged
* **Audit** (`docs/STAGE2_AUDIT_REPORT.md`, checker `stage2/audit/independent_audit.py`): the raw CSVs, manifests, run logs, freeze hashes and every number in the summaries/report were re-derived with separate code; results agree (one 0.001 rounding slip in the report and one documented plan-file edit are listed). A fresh-seed red-team fuzz (986,580 sorts, 108 configurations, stability tags, canary guard zones, bad comparators) found **no defect in the sorting code**. Known limitation found: no exception safety if a comparator throws.
* **New experiment** (`docs/STAGE2_REAL_COMPARATOR_PLAN.md` frozen first, results in `docs/STAGE2_REAL_COMPARATOR_RESULTS.md`): real lexicographic string comparison (240-character shared prefixes), 18 held-out cells. **SAG-C (primary): GAIN WITHOUT SAFETY** (6 of 8 low-entropy cells >= 10 % faster, but one control cell +11.4 %); round-1 SAG under the same rule: SUCCESS (6 of 8, no severe regression). Synthetic strings, one machine; not a general performance win.
* **Paper** (`stage2/paper/STAGE2_PAPER_VERIFICATION.md`): publisher page, Crossref and OpenAlex agree; a publisher *correction* (affiliation only, DOI 10.1007/s00453-025-01297-w) exists; TimSort rule and `t = ceil(log2(a+b))^2` verified against the printed paper; journal-level Scopus listing seen on the publisher's page, article-level Scopus indexing still **not** verified.
* **Environment note:** the stage-1 numbers were measured on an i5-1235U with g++ 14.2 (MinGW); stage 2 ran on an i5-13500HX with the Cygwin-target g++ 15.3.0 (sanitizer libraries unavailable; checked/UBSan-trap builds used instead). See `docs/STAGE2_CHANGELOG.md`, `docs/EVALUATION_PREP.md`.

## Stage 3 (2026-10-21): exception contract fixed + one fresh confirmation of SAG - earlier verdicts unchanged
* **Exception safety:** a comparator that throws could lose or duplicate elements (reproduced). Now: basic guarantee (array stays a permutation; order unspecified), zero code for `noexcept` comparators; test `build/test_exception`.
* **Confirmation (`docs/STAGE3_CONFIRMATION_PLAN.md` frozen first; results `docs/STAGE3_CONFIRMATION_RESULTS.md`):** 50 string-key cells (variable lengths/prefixes, 3 comparison-cost classes, 2 real file-name cells). **SAG: CONFIRMATION SUCCESS** (12 of 16 expensive-key low-entropy cells >= 10 % faster, no severe regression anywhere, independently re-derived) - a *conditional* improvement for expensive keys; cheap short keys gain little or lose. SAG-C (diagnostic) is not recommended. Synthetic strings, one machine.
* Audit: `docs/STAGE3_AUDIT_REPORT.md`, checker `stage3/audit/audit_stage3.py`; changelog `docs/STAGE3_CHANGELOG.md`; viva material `docs/EVALUATION_PREP.md`.

## Prerequisites
`g++` with C++17 (no CMake/Ninja needed), Python 3 (standard library only; `matplotlib` only for figures). WSL with g++ only for the optional sanitizer run.

## Build, test, demo (copy-paste; Git-Bash or Linux shell, from the project root)
```bash
bash scripts/build.sh                 # builds build/test_base, test_all, test_sag, bench_sort, demo_sort
./build/test_base.exe                 # base paper implementation only          -> expect "1502829 checks passed, 0 failed"
./build/test_all.exe                  # base + extension                        -> expect "2100389 checks passed, 0 failed"
./build/test_sag.exe                  # extension-specific tests                -> expect "265609 checks passed, 0 failed"
./build/test_sagt.exe                 # corrective-round extensions (SAG-C etc.) -> expect "760686 checks passed, 0 failed"
python scripts/final_audit.py         # full audit (clean rebuild + tests + re-derivation of results) -> "SUMMARY: 0 FAIL"
./build/demo_sort.exe                 # readable demonstration (see below)
```
(On Linux drop `.exe`.) Sanitizers: `MSYS_NO_PATHCONV=1 wsl.exe -e bash "/mnt/d/new dsa project/scripts/sanitize_wsl.sh"` (adapt the path).

Sample `demo_sort` output (actual, abridged):
```
few(3) values         16.28   1.58 |   14.454    8.272    6.776 |     15.583     14.406
few(64) values        16.67   6.00 |   16.639   14.577   11.166 |     17.744     16.658
   columns: H  H*  | naive  base(paper)  SAG(proposed) | std::stable_sort  textbook top-down   (comparisons per element)
all outputs equal the reference stable sort: YES
```

## Reproduce the experiments
```bash
python scripts/run_bench.py --manifest dev_counts   # development: deterministic comparison counts (seconds)
python scripts/run_bench.py --manifest dev_time     # development timing (~1 min)
python scripts/run_bench.py --manifest heldout      # FROZEN held-out run, 90 cells, ~25 min, needs an otherwise idle machine
python scripts/analyze.py results/raw/heldout.csv   # tables + pre-registered criteria -> results/outputs/summary_heldout.md
python scripts/run_bench.py --manifest heldout2     # corrective round, fresh seeds, 54 cells (~10 min)
python scripts/analyze.py results/raw/heldout2.csv --var sagt --round2   # -> results/outputs/summary_heldout2.md
python scripts/make_figures.py results/raw/heldout.csv results/raw/heldout2.csv   # results/figures/*.png
```
Protocol, seeds, success criteria and the SHA-256 of every file under test were frozen **before** each held-out run: `docs/EXPERIMENT_PLAN.md` (round 1 + addendum for round 2), `results/outputs/freeze_record.txt`, `results/outputs/freeze_record_round2.txt`.

## Where to look first
1. `docs/FINAL_REPORT.md` — question, evidence, verdict, limitations.
2. `docs/SYLLABUS_COMPLIANCE.md`, `docs/IDEA_SELECTION.md`, `docs/PAPER_VERIFICATION.md`.
3. `docs/BASE_PAPER_IMPLEMENTATION.md` (what was implemented, deviations) and `docs/PROPOSED_METHOD.md` (the extension).
4. `results/outputs/summary_heldout.md`, `results/figures/`, raw data `results/raw/heldout.csv`.
5. `docs/EXPERIMENT_LOG.md` — chronological log, including mistakes found and fixed.
6. `docs/DEMO_COMMANDS.md` — exact commands with expected output.

## Honest limitations (short)
Synthetic data only; the "heavy comparator" regime is a model; one hybrid-core laptop with uncontrolled turbo; the base is a re-implementation of a theoretical paper (no reference code), the TimSort stack-rule inequalities were reconstructed from the Java rule because the PDF text lost the symbols; the paper's proofs and adversarial constructions were not reproduced; Scopus status unverified. No claim of novelty beyond "proposed extension".
