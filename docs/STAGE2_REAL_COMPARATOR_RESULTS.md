# STAGE2_REAL_COMPARATOR_RESULTS - real string comparator, frozen protocol, held-out run

**Verdict under the frozen rule (`docs/STAGE2_REAL_COMPARATOR_PLAN.md` section 8):**
* **PRIMARY extension SAG-C: GAIN WITHOUT SAFETY** - the gain criterion passed (6 of 8 low-entropy cells >= 10 % faster, exactly at the 2/3 threshold) but the guard failed: one cell shows a severe regression (`runs(64)`, 2^17: 1.114 [1.104, 1.150]). **The primary success criterion is NOT met.**
* Secondary, same rule applied separately to round-1 SAG: **SUCCESS** (6 of 8 low cells >= 10 % faster; no severe regression anywhere; comparisons never above base). This is reported with its own verdict, was declared as secondary before the run, and is **not** substituted for the primary result.
Both are synthetic strings, one machine, one compiler. This does not change the round-1 (PARTIAL SUCCESS) or round-2 (CORRECTIVE PARTIAL) verdicts.

## 1. Protocol (frozen before the run)
Plan `docs/STAGE2_REAL_COMPARATOR_PLAN.md` (SHA-256 in `stage2/string_exp/freeze_record.txt`, written 10:14:19; run started 10:14:43, finished 10:20:32). 18 cells = 9 families x n in {2^17, 2^19}, seeds 7100000+; 8 variants (naive, base, base_b = A/A control, sag, sagc, tim_base, std::stable_sort, textbook top-down); 15 / 11 repetitions, randomised variant order per repetition, 1 warm-up, thread pinned to P-core logical CPU 2 (verified 1000/1000 in all 18 cells), AC power, Balanced plan; `g++ 15.3.0 -O2` (Cygwin target). Keys: `240-char identical prefix + 8 hex digits` strings, real lexicographic compare (`std::string_view::compare`), element = 16-byte record {pointer, length, original index}; **unchanged frozen sorting sources** (hashes identical to the round-2 freeze).
Raw data: `stage2/results/string_heldout.csv` (2,016 records: 144 counting + 1,872 timed), run log `stage2/results/string_heldout_runlog.txt`, table `stage2/results/summary_string_heldout.md`. Development data (harness check only): `string_dev.csv`.

## 2. Correctness (P1): PASS
All 2,016 rows have `ok = 1`: every output equals the `std::stable_sort` reference exactly (including the original-index order of equal keys, i.e. stability) and passes a separate order + stability scan. 18/18 cells, 8 variants, every repetition present; every cell process exited 0; no failed or missing run.

## 3. Per-cell results (time ratios = median of paired per-repetition ratios, bootstrap 95 % CI)
| cell | class | cmp SAG-C/base | cmp SAG/base | time SAG-C/base [95% CI] | time SAG/base [95% CI] | A/A [95% CI] | naive/base | std::stable_sort/base | base ns/elem | SAG-C measured c (ns) |
|---|---|---|---|---|---|---|---|---|---|---|
| few16_n17 | low-H* | 0.668 | 0.759 | 0.823 [0.796, 0.847] | 0.815 [0.808, 0.839] | 0.998 [0.964, 1.024] | 1.574 | 1.802 | 319.4 | 124.8 |
| few16_n19 | low-H* | 0.657 | 0.755 | 0.812 [0.716, 0.883] | 0.794 [0.737, 0.836] | 0.993 [0.952, 1.003] | 1.968 | 2.324 | 312.9 | 236.2 |
| few256_n17 | low-H* | 0.762 | 0.834 | 0.725 [0.704, 0.755] | 0.725 [0.714, 0.744] | 0.976 [0.921, 0.995] | 0.987 | 1.049 | 500.1 | 117.9 |
| few256_n19 | low-H* | 0.714 | 0.786 | 0.630 [0.617, 0.639] | 0.650 [0.636, 0.657] | 0.992 [0.974, 1.004] | 1.153 | 1.398 | 546.1 | 178.2 |
| few2_n17 | low-H* | 0.761 | 0.851 | 0.979 [0.944, 0.998] | 0.907 [0.896, 0.958] | 1.011 [0.985, 1.050] | 1.827 | 2.193 | 149.9 | 123.7 |
| few2_n19 | low-H* | 0.748 | 0.851 | 0.921 [0.895, 0.951] | 0.935 [0.926, 0.952] | 0.998 [0.984, 1.042] | 2.054 | 2.615 | 174.4 | 185.6 |
| zipf1024_n17 | low-H* | 0.808 | 0.815 | 0.869 [0.848, 0.888] | 0.853 [0.834, 0.867] | 0.990 [0.986, 1.010] | 1.134 | 1.205 | 406.9 | 129.2 |
| zipf1024_n19 | low-H* | 0.760 | 0.773 | 0.745 [0.737, 0.754] | 0.731 [0.727, 0.747] | 0.992 [0.986, 1.006] | 1.267 | 1.533 | 484.5 | 221.9 |
| few4096_n17 | high-H* | 0.972 | 1.000 | 0.975 [0.961, 1.000] | 1.012 [0.986, 1.036] | 0.999 [0.983, 1.020] | 0.990 | 1.038 | 480.6 | 213.3 |
| few4096_n19 | high-H* | 0.887 | 0.955 | 0.805 [0.798, 0.819] | 0.918 [0.910, 0.931] | 1.011 [0.998, 1.012] | 1.017 | 1.210 | 639.6 | 175.8 |
| mixture64_n17 | high-H* | 0.863 | 0.897 | 0.935 [0.928, 0.951] | 0.892 [0.878, 0.913] | 0.990 [0.966, 1.022] | 1.034 | 1.202 | 391.1 | 105.5 |
| mixture64_n19 | high-H* | 0.855 | 0.891 | 0.931 [0.911, 0.951] | 0.912 [0.892, 0.933] | 1.009 [0.988, 1.015] | 1.167 | 1.496 | 482.6 | 227.6 |
| nearly0p01_n17 | high-H* | 0.515 | 0.534 | 0.745 [0.728, 0.750] | 0.667 [0.633, 0.683] | 0.994 [0.977, 1.016] | 1.618 | 2.201 | 126.9 | 111.7 |
| nearly0p01_n19 | high-H* | 0.484 | 0.518 | 0.674 [0.643, 0.715] | 0.658 [0.627, 0.678] | 1.034 [0.931, 1.071] | 1.780 | 2.312 | 150.2 | 169.1 |
| perm0_n17 | high-H* | 1.011 | 1.000 | 1.039 [0.997, 1.075] | 0.994 [0.980, 1.021] | 0.984 [0.977, 1.009] | 0.983 | 1.058 | 629.2 | 159.8 |
| perm0_n19 | high-H* | 1.005 | 1.000 | 1.037 [1.019, 1.060] | 1.027 [1.014, 1.074] | 1.005 [0.985, 1.019] | 0.995 | 1.127 | 823.8 | 190.4 |
| runs64_n17 | high-H* | 1.007 | 0.993 | **1.114 [1.104, 1.150]** | 1.007 [0.990, 1.043] | 0.991 [0.984, 1.035] | 0.965 | 1.495 | 148.4 | 123.6 |
| runs64_n19 | high-H* | 0.987 | 0.988 | 1.092 [1.061, 1.112] | 1.014 [0.983, 1.045] | 0.996 [0.961, 1.005] | 0.992 | 1.749 | 155.5 | 169.4 |
The first comparison column (SAG-C) includes its 16,384 calibration calls; P4 uses the sorter-counted comparisons (CSV column `comparisons`, excluding calibration), which is the same definition as R2-D. Base wall time per comparison was 20-46 ns (base ns/elem divided by base comparisons/elem).

## 4. Decision rule evaluation
| criterion | SAG-C (primary) | SAG (secondary) |
|---|---|---|
| P1 correctness | PASS | PASS |
| P2 gain >= 10 % (ratio <= 0.90, CI upper < 1) in >= 6 of 8 low cells | **PASS, 6 of 8** (`few2` at both sizes miss: 0.979 [0.944, 0.998], 0.921 [0.895, 0.951]) | PASS, 6 of 8 (`few2` misses: 0.907 [0.896, 0.958], 0.935 [0.926, 0.952]) |
| P3 no severe regression (ratio >= 1.10, CI lower > 1) in any cell | **FAIL**: `runs(64)` 2^17 = 1.114 [1.104, 1.150] (and 2^19 = 1.092 [1.061, 1.112] just under the threshold) | PASS (max 1.027) |
| P4 sorter-counted comparisons <= 1.03 x base | PASS | PASS |
| A/A noise (identical code) | largest |median - 1| = 0.034; identical code was never flagged as win/severe | |
| **Verdict** | **GAIN WITHOUT SAFETY** | **SUCCESS** |

## 5. Reading the result honestly
* **The base paper's galloping is itself a large win here on low-entropy data:** `naive` is 1.13x-2.05x slower than `base` on `few(2)`, `few(16)`, `zipf(1024)` and `nearly(0.01)` (about equal on `few(256)`: 0.987 and 1.153), and `std::stable_sort` is slower than `base` in every one of the 18 cells (1.04x-2.6x). These are context medians; no CIs were computed for the references.
* Compared with `base`, SAG-C needs 19-34 % fewer comparisons on the low-entropy cells (SAG: 15-25 %) and about half on `nearly(0.01)`. With a true string comparator (20-46 ns wall time per comparison in the sort, dominated by cache misses) this became 13-37 % less time (SAG-C) / 15-35 % (SAG) on the 6 low cells that passed, 25-34 % on `nearly(0.01)`, and 2-20 % on mid-entropy controls for SAG-C. It **did not reach 10 %** on `few(2)` (the paper's two-value case: long blocks, only 150-175 ns/element for the base).
* **SAG-C's failure is systematic, not noise; its cause is only partly demonstrated.** Its self-calibration reads 106-236 ns per comparison, 3.7-9.1x the wall time per comparison inside the sort (it times comparisons on random pairs, i.e. cold cache misses). Two consequences are consistent with the data but were not isolated by a separate experiment: (i) the 16,384 calibration calls are a fixed ~2-4 ms, about 10 % of a 19 ms sort (`runs(64)` 2^17) but only ~3 % of the 82 ms sort at 2^19 where the regression is still +9 %; (ii) an inflated `c` makes the credit rule adopt `t = 0` too eagerly. This matches the risk recorded in the plan from development data (`runs(16)`); the constants were deliberately not retuned. The round-1 SAG has no calibration and showed no such regression.
* The 2^17 `runs(64)` regression has a CI that does not cross 1.10 from below (lower bound 1.104), so it is "severe" by the rule; the 2^19 cell is not. The A/A control stays within +/-3.4 %, so these are not noise.
* Round-1 SAG's string result (SUCCESS) is consistent with its round-1 heavy-comparator result (11-21 % on low cells) and with its round-1 plain/indirect failures: it helps when comparisons are expensive. The string comparator here (20-46 ns) is in the range where the break-even sweep of round 1 predicted a gain (>= ~7 ns).
* Margins are thin: both extensions pass P2 with exactly 6 of 8 cells (the threshold). The classification is unchanged under an alternative distribution-free CI (exact sign interval): 0 of 36 cell-variant pairs change.

## 6. Limitations specific to this experiment
Synthetic strings with one shared prefix and fixed length (real data, e.g. URLs or words, have variable shared-prefix length, usually shorter); n <= 2^19; one machine (different CPU/compiler than rounds 1-2); `memcmp` is vectorised; results do not transfer to other string workloads without testing; the two extensions were evaluated under one rule (declared before the run) but are two comparisons from the same data; the primary verdict is not a general performance win.

## 7. Audit
`python -I stage2/audit/audit_string_experiment.py` -> `stage2/audit/string_audit_output.txt`: 19 PASS, 0 FAIL, 0 DISCREPANCY (freeze hashes intact, 2,016 records complete, run log equals design, every table number and both verdicts re-derived independently).
