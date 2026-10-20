# STAGE3_AUDIT_REPORT - Stage 3 audit (exception contract + confirmation experiment)

Date 2026-10-20/21. The Stage-3 checker `stage3/audit/audit_stage3.py` imports none of `scripts/final_audit.py`, `scripts/analyze.py`, `scripts/run_bench.py`, `stage2/string_exp/*`, `stage3/string3/analyze_string3.py` or `run_string3.py`; the design is transcribed from the plan and checked against the frozen manifest digest. It is an internal audit (same author family as the code), not an external one.

## 0. Verdict
| question | result |
|---|---|
| Was the exception-contract defect real, and is it resolved? | Yes, reproduced (32,069 / 84,623 cases); resolved by a basic guarantee (0 / 84,623 after the fix); strong guarantee not provided. |
| Did the fix change behaviour or speed for non-throwing comparators? | Counts and outputs identical (216 differential runs); no systematic speed change; generated code not byte-identical. |
| Is the Stage-3 confirmation result reproducible from the raw data? | Yes: checker re-derived every number, the manifest digest, run log and the verdict (SUCCESS) independently; 0 mismatches, 0 discrepancies. |
| Did earlier stages' evidence stay intact? | Yes, except `src/natural_merge_sort.hpp`, deliberately patched (original preserved and hash-verified). Old audits fail exactly on that file (section 4). |

## 1. Checkpoints
CP1 (Stage 2 final) re-verified (zip SHA-256 `84517a4a...ad66d`, 235 members, 0 mismatches). CP2 (before Stage 3) `...CP2_pre_stage3_20261020_183106.zip` SHA-256 `e37c7d81...f1539`. Final checkpoint: `stage3/CHECKPOINT_CP3.txt`. Root `D:\new dsa project`; the nested duplicate was not touched; LPQ folders do not exist on this machine and were not accessed.

## 2. Exception-contract change (commands, evidence)
| check | result |
|---|---|
| `exc_repro` on ORIGINAL headers (`stage3/artifacts/orig_src`) | 84,623 cases, 84,383 aborted by the exception, **32,069 left a non-permutation**; smallest: n = 3, TimSort order + naive merge, throw at call 5 of 5 |
| same program on the PATCHED header | **0 of 84,623** non-permutations (`stage3/logs/exc_repro_FIXED.txt`) |
| `tests/test_exception.cpp` (built by `scripts/build.sh`) | exit 0, 84,624 checks, 0 failed; hardened build also 0 failed |
| differential `diff_counts` original vs patched (6 policies x noexcept / non-noexcept comparator x 6 families x 3 sizes x 2 minrun = 216 runs) | all 216 lines identical (comparisons, run/merge/block/move counters, buffer, output hash) |
| assembly for a noexcept comparator | **not byte-identical** (labels; one routine, the plain two-pointer merge, has a different instruction schedule) - stated, not hidden |
| micro-benchmark (1M integers, 3 inputs, 6 configurations, 5 alternating runs) | patched/original 0.86-1.02; no systematic effect (noise about +-5-10 % on the shortest sorts) |
| suites on the patched header, -O2 | `test_base` 1,502,829; `test_all` 2,100,389; `test_sag` 265,609; `test_sagt` 760,686; `test_exception` 84,624; all 0 failed, exit 0 |
| hardened builds (checked containers, UBSan trap-on-error, `-ftrapv`) of the same four + `test_exception` | all exit 0, same counts, 0 failed |
| red-team fuzz on the patched header | -O2 seed 20281020 and hardened seed 20291020: 986,580 sorts, 10,057,802 checks, 0 failed each; throwing-comparator probe: 0 of 15 aborted sorts lose elements (was 4 of 15) |
Guarantee now documented in the header: if the comparator throws, the exception propagates and the array is a permutation of the input; order and stability are unspecified; counters are partial. Sanitizer runtimes (ASan/UBSan) remain unavailable on this machine (no `libasan`/`libubsan`, WSL has no compiler); nothing was installed.
Why Option A: Option B (`static_assert` on `noexcept`) would have broken the frozen benchmark/test comparators, which are not declared `noexcept`, forcing edits to frozen files.

## 3. Verification of the string-key representation (before the confirmatory runner)
`SRec = {const char*, u32 len, u32 idx}` is trivially copyable (`static_assert`); the comparator is `std::string_view::compare` (bytewise lexicographic, length breaks ties; strict weak order) and `noexcept` (`static_assert is_nothrow_invocable`); every record owns a NUL-terminated private copy in a packed arena and the harness aborts if any record is inconsistent (embedded NUL / missing terminator); each sorted output is compared with the `std::stable_sort` reference as a record sequence (pointer, length, original index) and with an independent order + stability scan. Structural properties (`H*`, `cost_B`) are computed from the data, never from timings.

## 4. Old audits after the deliberate header change
| audit | on the pristine CP2 tree | on the live tree | explanation |
|---|---|---|---|
| `scripts/final_audit.py` | **0 FAIL** (clean rebuild, all tests, demo, D, E, F, G, H, I) | 1 FAIL: `G src/natural_merge_sort.hpp unchanged since the round-1 freeze` | the Stage-3 guard; all other checks pass |
| `stage2/audit/independent_audit.py` | (unchanged Stage-2 result: 63 PASS, 0 FAIL, 3 DISCREPANCY) | 61 PASS, 2 FAIL, 3 DISCREPANCY | the 2 FAILs are the "library code identical to the freeze" checks for `natural_merge_sort.hpp` |
| `stage2/audit/audit_string_experiment.py` | (unchanged: 19 PASS) | 17 PASS, 2 FAIL | same file hash checks |
The Stage-3 checker verifies that the preserved ORIGINAL `natural_merge_sort.hpp` equals the round-2 freeze hash (so the history is intact) and that SAG, SAG-C, generators, entropy and textbook sources are still byte-identical. Every audit-output file that these scripts overwrite was backed up first and restored afterwards (`stage3/artifacts/backup_audit_outputs/`; hashes equal); the live-tree runs are kept as `stage3/artifacts/final_audit_LIVE_stage3.txt` and `stage3/logs/final3_*`. No earlier report, CSV, manifest, freeze record or verdict was edited.

## 5. Confirmation experiment: independent verification (`stage3/audit/audit_stage3_output.txt`)
* freeze record: every hashed file (library sources, bench source and binary, runner, analyzer, dataset, plan, structure CSV) byte-identical after the run; design transcribed from the plan has the frozen manifest digest `aecefbd7...b85`.
* raw data: 31 columns; 5,888 lines = 5,888 records; 50 / 50 cells; no duplicates; 400 counting rows and 5,488 timed rows exactly as designed; every (cell, variant) has repetitions 0..R-1 once; `order_pos` permutations; constant structural columns; `ok == 1` in all rows; `ns_total == ns_per_elem * n`; `n-1 <= comparisons <= 3 n log2 n`; run + merge == total; `base` and `base_b` comparison counts identical; SAG-C calibration recorded in all cells.
* run log: one segment started (23:23:27) after the freeze record (23:23:13), logged the frozen digest; 50 invocations equal the design; process exit status 0 exactly for the cells whose rows are all `ok`; "failures=0"; pinning 50 / 50 on logical CPU 2, efficiency class 1, 1000 / 1000.
* gain set G recomputed from the CSV equals the pre-declared 16 cells; the timed run's `H*`, `cost_B` and `total_bytes` equal the un-timed structural pass frozen earlier.
* all 500 printed numbers of `summary_s3_heldout.md` (50 cells x 10) re-derived with an independent bootstrap: 0 mismatches; independently derived verdict **SUCCESS** (P1 true; P2 12 / 16, need 11; P3 none; P4 none), consistent with the stored verdict; exact sign-interval CI changes no classification (0 / 50); A/A control largest deviation 0.021, identical code never flagged.
* documents: README, `FINAL_REPORT.md` (Stage-3 section), `STAGE3_CONFIRMATION_RESULTS.md` and this report exist and mention the Stage-3 verdict; the demo output equals the stored `results/outputs/demo_output.txt` (final pass).
No discrepancy was found in the Stage-3 data. During drafting, four numeric statements in my own results text (e.g. "17-67 ns per comparison", "5-34 %") disagreed with the table; they were checked against the raw data by script and corrected before the final audit - recorded here, not hidden.

## 6. Paper and indexing
The correction notice (DOI 10.1007/s00453-025-01297-w) was opened on the publisher page: it states that one co-author's affiliation (University of Waterloo, studies began after publication) was incorrect and was removed. It does **not** alter the method. Scopus: the publisher's journal page lists SCOPUS; Scopus's own source search answered "you must be a registered user" -> official Scopus verification unavailable (not bypassed); journal-level status is publisher-reported, individual-article indexing unverified.

## 7. Unresolved limitations
Internal audit only; one machine and compiler; no sanitizer runtime; generated code differs slightly for the plain merge (no measured effect); basic (not strong) exception guarantee; confirmation margin thin (12 of 16, threshold 11) and conditional on key cost; synthetic data except two small file-name cells; Scopus unverified; the Stage-1/2 timings were never re-measured.
