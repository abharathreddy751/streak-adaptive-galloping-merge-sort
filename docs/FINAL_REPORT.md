# FINAL_REPORT — Streak-Adaptive Galloping for Natural Merge Sort

## 0. Verdict (under the rules frozen before each held-out run)
| round | what was evaluated | verdict |
|---|---|---|
| **Round 1 (primary)** — 72 held-out cells + 18 sweep cells | SAG vs the base paper's policy | **PARTIAL SUCCESS, conditional on comparator cost.** H1 (comparisons), H2 (heavy-comparator time), H3a (no harm when comparisons dominate) and H4 (correctness) pass; H3b (no harm with cheap comparisons) and the generic "10 % faster on a majority of all cells" criterion **fail**. |
| **Round 2 (one corrective experiment)** — 54 fresh-seed cells | SAG-C (time-aware, self-calibrating) vs base | **CORRECTIVE PARTIAL.** Gain when comparisons dominate is larger than in round 1 (R2-A pass), correctness and comparison guard pass, but **2 of 36** cheap-comparison cells still show a severe regression (R2-B fail). |

In plain words: the proposed change is a real improvement when a key comparison is expensive (11–28 % less time on low-entropy inputs, up to 47 % on nearly-sorted data, with up to 34 % fewer comparisons), and it is neutral-to-slightly-harmful with cheap comparisons, where even the paper's own galloping is slower than plain merging. No claim is made that it is better in general.

## 1. Research question and syllabus link
*Can the galloping parameter of a natural merge sort be adapted online, using the paper's own cost model, to reduce the cost of the merge step — and does fewer comparisons mean less time?* The central algorithm is **Merge Sort** (MACSE501 Module 3); the project modifies and evaluates its merge step. An earlier idea (succinct rank/select bit vectors) was rejected as outside the syllabus (`docs/SYLLABUS_COMPLIANCE.md`; its documents are kept in `docs/rejected_rank_select_idea/`).

## 2. Base paper
E. Ghasemi, V. Jugé, G. Khalighinejad, H. Yazdanyar, "Galloping in Fast-Growth Natural Merge Sorts", *Algorithmica* 87(2):242–291, Springer, online 2024-12-09 (issue February 2025), DOI 10.1007/s00453-024-01285-6; preprint arXiv:2012.03996 (v3, read). Peer-reviewed journal article; publisher, venue, year and DOI verified in Crossref and OpenAlex. **Scopus indexing is not independently verified** (only reported by a web-search snippet of the publisher's journal page). The Springer page itself and DBLP/Scimago could not be opened (bot protection / HTTP 403) and were not bypassed. Details: `docs/PAPER_VERIFICATION.md`.

## 3. What was implemented
* **Base** (`src/natural_merge_sort.hpp`, ~400 lines): run detection with binary-insertion `minrun`; PowerSort and TimSort stack rules (the TimSort inequalities were reconstructed because the PDF text lost the symbols); McIlroy's t-galloping merge with the paper's polylogarithmic `t = ⌈log₂(a+b)⌉²`; stable forward/backward merge sharing one core. Deviations and omissions: `docs/BASE_PAPER_IMPLEMENTATION.md`.
* **Extension SAG** (`src/sag_policy.hpp`): adapts `t` inside each merge from the length of the block just found, evaluating candidates with the paper's own cost model; no extra memory or key comparisons; worst case ≤ (4/3)·naive comparisons per merge (the paper's guarantee is the stronger "naive + O(n)"; I do not claim a better worst case). Final constants (MinStreak 64, jump to t = 0 allowed) chosen on development data in two rounds. `docs/PROPOSED_METHOD.md`.
* **Corrective extension SAG-C** (`src/sag_time_policy.hpp`): the same idea driven by a time model `c·probes + m·non-linear probes` with the comparison cost `c` measured by the sorter itself and a credit rule that costs one compare per block. Two earlier attempts (SAG-T v1, SAG-B) are kept in the code and logs as documented failures.

## 4. Correctness evidence
| check | result |
|---|---|
| Base only (`test_base`): exhaustive and sampled sorting tests, galloping routine vs the paper's Table 1 and `cost_t(m)`, per-merge cost bound, Prop. 49, Thm 51 shape, information-theoretic bound, edge cases | **1,502,829 checks, 0 failed** |
| Base + SAG (`test_all`, 11 configurations) | **2,100,389 checks, 0 failed** |
| SAG controller and bounds (`test_sag`) | **265,609 checks, 0 failed** |
| SAG-T/B/C controllers, calibration, sorting, bounds (`test_sagt`) | **760,686 checks, 0 failed** |
| Same four binaries under ASan + UBSan (WSL, g++ 15.2.0) | same counts, **0 failed**, no sanitizer report |
| Held-out benchmark outputs vs `std::stable_sort` reference (every timed run and every counting run) | round 1: 7,416 rows (90 cells), **0 mismatches**; round 2: 54 cells, **0 mismatches** |
The test domains are *stated*, not "exhaustive": all 44,100 pairs of sorted sequences (len ≤ 6, alphabet 4); all 29,524 arrays of length ≤ 9 over {0,1,2}; all permutations of length ≤ 7; 2,445 deterministic random arrays from 15 families; plus invariants taken from the paper. They do not reproduce the paper's proofs or its adversarial constructions.

## 5. Experimental design (frozen: `docs/EXPERIMENT_PLAN.md`, `results/outputs/freeze_record*.txt`)
Identical inputs for all variants, randomised variant order per repetition, thread pinned to a P-core (verified 1000/1000 samples), un-timed counting pass for exact comparison counts, A/A control (`base_b`), paired per-repetition ratios with bootstrap 95 % CIs, low-entropy cells defined structurally (`H* ≤ 0.5·log₂ n`), three comparison regimes: **plain** keys, **indirect** (argsort) keys, and a **synthetic heavy comparator** (K = 40 dependent multiply-adds ≈ 22 ns). Held-out: 9 families × sizes 2^17/2^20/2^22 (heavy regime ≤ 2^20), seeds 9001+; round 2: fresh seeds 109001+. Development used disjoint seeds and different parameters. Everything is **synthetic**.

## 6. Results — round 1 (primary)
Raw: `results/raw/heldout.csv`; full table with CIs: `results/outputs/summary_heldout.md`; figures `results/figures/fig1…3`.

**Comparisons (deterministic):** SAG/base = 0.73–0.85 on all 12 low-entropy (family,size) cells; 0.49–0.54 on `nearly(0.01)`; 0.83–1.00 on `few(4096)`, 0.89 on `mixture`, 0.98–0.99 on `runs(64)`, 1.000 on random permutations. Never above 1.000.

**Time, heavy regime (comparisons ≈ 22 ns), SAG/base, 95 % CI:**
| family | 2^17 | 2^20 |
|---|---|---|
| few(2) | 0.880 [0.868, 0.888] | 0.866 [0.854, 0.886] |
| few(16) | 0.794 [0.790, 0.819] | 0.800 [0.778, 0.821] |
| few(256) | 0.887 [0.824, 0.951] | 0.822 [0.801, 0.843] |
| zipf(1024) | 0.871 [0.866, 0.873] | 0.823 [0.792, 0.842] |
| few(4096) (high H*) | 1.022 [1.013, 1.038] | 0.934 [0.905, 0.947] |
| mixture(64) | 0.931 [0.920, 0.941] | 0.923 [0.901, 0.935] |
| nearly(0.01) | 0.581 [0.568, 0.594] | 0.563 [0.547, 0.569] |
| perm | 1.015 [0.982, 1.028] | 1.010 [0.977, 1.042] |
| runs(64) | 1.002 [0.993, 1.024] | 1.009 [0.975, 1.031] |
All 8 low-entropy heavy cells are ≥ 11 % faster with the CI below 1 (H2 pass); no heavy cell is severely worse (worst +2.2 %, H3a pass).

**Time, cheap regimes (the failures):** SAG/base is above 1.0 in **all 54** plain and indirect cells (plain: median 1.066, range 1.007–1.133; indirect: median 1.079, range 1.005–1.240); **12 of 54 cells are severe** (≥ 1.10 with CI above 1: `nearly(0.01)` and `zipf(1024)` in both regimes, `runs(64)` indirect). Even where SAG saves 25 % of the comparisons, e.g. `few(16)` plain keys: 0.754× comparisons but 1.03× time.

**The paper's own galloping vs plain merging (context, round 1):** with heavy comparisons `base` is up to 2.1× faster than `naive` (nearly sorted, 2^20) and 1.2–2.0× faster on few-valued inputs (σ = 2, 16), but only 1.0–1.1× on `few(256)`/`zipf(1024)`; with plain keys `naive` is 8–17 % **faster** than `base` on random-like inputs (permutations, `runs(64)`, `mixture(64)`, `few(4096)`) while `base` is still 12–30 % faster on nearly-sorted data. The comparison-only cost model of the paper is therefore only predictive when comparisons dominate — the authors say so themselves.

**Break-even sweep (descriptive, n = 2^19, SAG/base time vs comparator cost K):**
| K (≈ ns per comparison, estimate) | few(16) | few(256) | perm |
|---|---|---|---|
| 0 (≈ 1) | 1.030 | 0.999 | 1.030 |
| 2 (≈ 2) | 0.987 | 0.989 | 1.012 |
| 5 (≈ 4) | 0.953 | 0.985 | 1.022 |
| 10 (≈ 7) | 0.872 | 0.905 | 1.012 |
| 20 (≈ 12) | 0.831 | 0.878 | 1.024 |
| 40 (≈ 22) | 0.789 | 0.833 | 1.015 |
SAG starts to pay at roughly 3–5 ns per comparison and reaches the 10 % target at about 7 ns on these inputs; on structure-free random data it never pays. (The ns-per-K figures are estimates from development timings, not measurements of K.)

## 7. Results — round 2 (corrective experiment, fresh seeds)
Raw `results/raw/heldout2.csv`; table `results/outputs/summary_heldout2.md`; figure `fig4_round2_corrective.png`.
* Heavy regime, 8 low-entropy cells: SAG-C/base = **0.725–0.863** (round-1 SAG on the same cells: 0.79–0.89); `nearly(0.01)`: 0.53–0.56; neutral on `perm`/`runs` (1.000–1.031). R2-A **pass** (8/8).
* Cheap regimes (36 cells): SAG-C/base median **1.015 (plain), 1.029 (indirect)**; share above 1.05: 11 % / 17 % (round-1 SAG on the same fresh seeds: medians 1.042 / 1.047, max 1.163). **2 severe cells** (R2-B **fail**): `indirect few(2) 2^17` = 1.103 [1.032, 1.166] and `indirect nearly(0.01) 2^17` = 1.120 [1.063, 1.190] — both are sorts of only 10–22 ns per element where the A/A control itself spreads widely (e.g. 0.986 [0.850, 1.042]); by the frozen rule they count.
* Comparisons: never above 1.001× base (R2-D pass); correctness R2-C pass.
* Generic criterion (≥ 10 % faster on a majority of all cells): fails (12 of 54), as it must when cheap regimes cannot gain.

## 8. Mistakes, failed attempts and corrections (all kept on record in `docs/EXPERIMENT_LOG.md`)
1. **Base bug** found by the smoke test: re-testing each block's first element cost +38 % comparisons on random data, contradicting the paper's Prop. 49; fixed before any benchmark.
2. **First selection rule for SAG's constants** (comparison counts only) picked MinStreak 16; development timing then showed up to +29 % time on structure-free inputs; the rule was revised on development data (before the frozen held-out run) and MinStreak 64 chosen.
3. **Analysis-script bug:** the first automatic held-out verdict was NO-GO because the script pooled the H3 guard over regimes, contradicting the written plan (heavy and cheap regimes separately). The script was corrected to follow the plan, no criterion was changed, and the first-pass output is kept (`results/outputs/summary_heldout_FIRSTPASS_pooled_H3_script_bug.md`).
4. **Corrective round, two failed variants:** SAG-T v1 (per-block model evaluation) added up to +60 % time in cheap regimes; SAG-B (memoryless break-even rule) lost the nearly-sorted gain. SAG-C is the third attempt; its remaining failures are reported.
5. **Tooling:** the shell tool collapsed double backslashes (corrupting a script, repaired), Python rewrote scripts with CRLF line endings (normalised), the editor gate blocked the first write of every new file (handled by retrying).
6. **Demo/overhead tuning** (forced inlining, passing `t` by value) was done on development data and benefits base and extension equally.

## 9. Limitations
Synthetic inputs only; the heavy comparator is a model, not a measured real comparator; one laptop with a hybrid CPU, uncontrolled turbo (unpinned calibration runs differed by 2×), power plan not changed, one compiler for timing; the base is my re-implementation of a theoretical paper (no authors' code), TimSort stack-rule inequalities and the exact rounding of `t` were reconstructed; the paper's proofs, adversarial arrays (Props. 42–43), other stack rules and TimSort's real `merge_lo/hi` were **not** implemented or reproduced; `minrun = 32` insertion-sort time dilutes merge-phase differences; held-out sizes ≤ 2^22 elements; the credit rule's constants (m = 16 ns, cap = 50·m) were tuned on seven development families at one size; Scopus status unverified; no formal worst-case bound better than the paper's is claimed; the earlier LPQ project and its backup were never read, so the stated lessons come from the task brief only. **Novelty is not claimed:** TimSort's own `min_gallop` adapts during a merge; SAG is a proposed extension that differs in using exact cost-model savings and a per-merge reset.

## 10. Completed vs unfinished
Completed: syllabus gate, verified base paper, base implementation + validation, extension + tests, frozen protocol, two held-out evaluations, sanitizer runs, figures, reproducible commands, audit. Unfinished / not claimed: reproduction of the paper's own experiments (it has none; its proofs were not reproduced), Scopus confirmation, tests on a real expensive comparator or another machine, any claim about general-purpose sorting speed.

## 11. Final audit
`python scripts/final_audit.py` → `results/outputs/final_audit.txt` (clean rebuild with the documented command, all four test binaries, demo reproduction, re-derivation of both summaries from raw CSVs, manifest digests, byte-identity of the original protocol text, unchanged round-1 code, CSV completeness, deliverables, LPQ metadata only). Its result is recorded in the log and repeated in the hand-over summary.

## 12. Stage-2 addendum (2026-10-20) - additional evidence; the verdicts in section 0 are unchanged
Full detail: `docs/STAGE2_AUDIT_REPORT.md`, `docs/STAGE2_REAL_COMPARATOR_PLAN.md`, `docs/STAGE2_REAL_COMPARATOR_RESULTS.md`, `docs/EVALUATION_PREP.md`.
* **Independent audit.** A separate checker (`stage2/audit/independent_audit.py`) re-derived all raw-data completeness checks, the frozen manifest digests, every printed number of `summary_heldout*.md`, the heavy-regime table of section 6 and the other numeric claims of sections 6-7, and both verdicts from the raw CSVs. Agreement was complete except: (a) section 7 quotes the plain-regime median of SAG-C/base as 1.015; the data give 1.0158, i.e. **1.016** (rounding slip, no effect on any verdict); (b) `docs/EXPERIMENT_PLAN.md` was edited (typo fix, logged at 00:23:47) 11 seconds after the round-2 run started, so its current hash differs from the one in `freeze_record_round2.txt` and the pre-edit text was not kept; the round-2 criteria are also encoded in `scripts/analyze.py`, whose frozen hash is intact. Section 4's wording "Same four binaries under ASan + UBSan" refers to the stage-1 machine; those sanitizer runs could not be repeated in stage 2 (no sanitizer libraries or WSL compiler) - checked-container + UBSan-trap builds of all four test programs and of a new red-team fuzz passed instead.
* **Environment.** The round-1/2 timings were produced on an Intel i5-1235U with g++ 14.2.0 (MinGW); the stage-2 machine is an i5-13500HX with a Cygwin-target g++ 15.3.0. Round-1/2 timings were audited, not re-measured.
* **Base paper.** The TimSort stack rule and `t = ceil(log2(a+b))^2` that section 3 and the limitations call "reconstructed" were checked against the rendered pages of the arXiv v3 PDF and match exactly. A published correction (affiliation only; DOI 10.1007/s00453-025-01297-w) exists. Journal-level Scopus listing is shown on the publisher's page; article-level Scopus indexing remains unverified.
* **Real string comparator (new, pre-registered, held-out).** 18 cells, 240-character shared prefixes, genuine `memcmp` comparison, frozen sources unchanged: **SAG-C (primary) = GAIN WITHOUT SAFETY** (>= 10 % faster in 6 of 8 low-entropy cells, but `runs(64)` at 2^17 is 1.114 [1.104, 1.150]); round-1 SAG evaluated separately under the same rule = SUCCESS. SAG-C's self-calibration reads 3.7-9.1x the in-sort comparison cost on this workload. Strings are synthetic; one machine.

## 13. Stage-3 outcome (2026-10-21) - additional evidence; all earlier verdicts unchanged
Detail: `docs/STAGE3_CONFIRMATION_PLAN.md`, `docs/STAGE3_CONFIRMATION_RESULTS.md`, `docs/STAGE3_AUDIT_REPORT.md`, `docs/STAGE3_CHANGELOG.md`.
* **Exception contract.** The Stage-2 finding was reproduced (comparator that throws mid-merge lost/duplicated elements: 32,069 of 84,623 throw-at-every-call cases). Fixed with a small RAII guard in `src/natural_merge_sort.hpp` (basic guarantee: the array stays a permutation; order unspecified; no code is generated for `noexcept` comparators). Regression test `tests/test_exception.cpp` (84,623 cases, 0 failures); all earlier suites, hardened builds and the red-team fuzz re-run on the patched header with 0 failures; comparison counts and outputs identical to the original header; no systematic speed change in a micro-benchmark (generated code is not byte-identical for the plain merge). The original header is preserved (`stage3/artifacts/orig_src/`); `scripts/final_audit.py` check G therefore fails on the live tree by design and passes on the pristine pre-Stage-3 tree.
* **Confirmation experiment (SAG primary, frozen before the run, run once).** 50 cells: 3 key styles (short / medium / long hierarchical keys with variable lengths and shared prefixes) x 8 structures x 2 sizes + 2 real-string cells. **CONFIRMATION SUCCESS for SAG**: 12 of 16 expensive-key low-entropy cells >= 10 % faster (needed 11), no severe regression in any cell, all 5,888 rows verified correct and stable; independent checker agrees. It is a **conditional** result: cheap 10-character keys give little or no gain (up to +7.6 % slower on `zipf(1024)`), four expensive-key cells missed 10 %, one is a thin margin. SAG-C (diagnostic only, not retuned) regressed +27.6 % on the real-string cell. Synthetic strings except two small file-name cells; one machine.
* **Paper.** The correction notice (10.1007/s00453-025-01297-w) only removes a wrong co-author affiliation; it does not touch the method. Scopus: publisher page lists SCOPUS; Scopus's own source search needs a login, so official verification remains unavailable (not bypassed); article-level indexing unverified.
