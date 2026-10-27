# Code-side audit of v4 (2026-10-27)

Scope: implementation, terminal tool, tests, headline metrics, experiment evidence, documentation and code package. Not covered: the review deck (handled separately). No sorting logic, SAG policy, constant, merge code, accepted criterion, frozen plan or raw datum was changed by this audit. Evidence files are in `validation/v4_audit/`; the audit scripts are `scripts/audit_*.{py,sh}`.

## 1. Verdict: PASS WITH DOCUMENTED LIMITATIONS
All required checks that could be run passed. One numerical error in the consolidated report was found and corrected; one platform limitation of the historical audit scripts (Linux) was found and documented; the sanitizer and Linux limitations of earlier documents were retired for the build, the six test programs and the tool (not for the gate). Remaining limitations are listed in section 10.

## 2. Baseline
Starting point: branch `v4-terminal-diagnostics` = `origin/main` = `a709d97` (working tree clean); tag `v3-submission` = `3a46350`; `new dsa project_SUBMISSION_V4.zip` SHA-256 `99cd44d5…67b0` (equals its sidecar and an independent `Get-FileHash`); v3 ZIP `50f90609…543f` unchanged. File-by-file comparison of the v3 and v4 ZIPs (`validation/v4_audit/zip_diff_output.txt`): 330 files identical, 15 changed, 45 added, 0 removed; every raw data file, frozen record, historical report and Stage 1-3 result file is byte-identical to v3.

## 3. Terminal tool (`nms_tool`)
| Function | How it was checked | Result |
| --- | --- | --- |
| `trace` | `scripts/audit_cli_check.py`: for 400 random inputs (n 1-60, many ties, sorted/reversed/run-structured, minrun 1/2/4/8, both policies) a separate Python model of natural merge sort derives the runs found, the merge sequence (PowerSort order), skipped merges, the galloping blocks (maximal streaks; forward merges leave the last streak unsearched, backward merges the first) and the sorted result; they are compared with the events printed by the tool | 400 of 400 identical. The model first disagreed on backward merges because it assumed forward order; the tool was right (streaks are discovered from the end of the array) and the model was corrected. Internally `trace` also checks output against a reference stable order, an independent sorted-and-stable test and the external comparison counter |
| `profile` | same script: distinct keys, descents, runs, asc/desc runs, run lengths, H and H* recomputed from their definitions for 400 random inputs | 400 of 400 agree (H and H* to 6e-4, the printed precision). Generator metadata and measured values are printed in separate blocks |
| `compare` | `tests/test_diag.cpp` (matrix of 9 families x 13 sizes x minrun 1/4/32, integer and string records) and the terminal checks | textbook, naive, base, SAG, `std::stable_sort` all `correct` and `stable`; sorter counters equal the external counter; attaching a trace changes neither output nor counters |
| `strings` | terminal checks; `docs/TERMINAL_DEMO.md` section 6 | runs, verifies stability, prints timing labelled descriptive (one machine, one input, not a significance test) |
| CLI | `scripts/audit_terminal_check.sh`: 8 documented examples, 25 boundary cases (n = 1, 2, 3; all-equal keys; sorted; reversed; pipe-organ; repeated keys; odd sizes 997/1001/5003; `--keys` extremes 0 and 4294967295; n = 128 and 129), 21 invalid-input cases, 2 determinism checks | 56 of 56 as expected: valid input exits 0, invalid input exits 2 with one message on stderr and nothing on stdout, repeated runs are byte-identical |
| Saved demo outputs | `bash scripts/v4_demo_outputs.sh` regenerated `results/outputs/v4_*.txt` | `git diff` empty (byte-identical) on Windows; on Linux identical except the compiler-version line |
| Timed variants unchanged | `validation/v4/asm_check` | Windows g++ 15.3.0: 78 functions identical; Linux g++ 15.2.0: 111 identical (after the normaliser fix of section 9) |

## 4. Tests and checks (final source, Windows, GCC 15.3.0, Python 3.11.9)
| Command | Exit | Result | Log |
| --- | --- | --- | --- |
| `bash scripts/run_all_tests.sh` | 0 | `RESULT: PASS`; `test_base` 1502829, `test_all` 2100389, `test_sag` 265609, `test_sagt` 760686, `test_exception` 84624, `test_diag` 1686861 checks, all 0 failed (overlapping suites: do not add); demo output identical; `tests/test_cli.sh` 45 checks, 0 failed | `validation/v4_audit/run_all_tests.txt` |
| `bash stage3/tools/final_pass3.sh` | 0 | `GATE: PASS` (clean build, 0 warnings; the tests above; `final_audit.py`: exactly one FAIL on the live tree, 0 on the pristine-header copy; Stage 2 audits: 2 + 2 FAILs naming exactly the expected changed files; Stage 3 audit: one FAIL (header hash) on the live tree and 25 PASS / 0 FAIL / 0 DISCREPANCY on a copy with the v3 header) | `validation/v4_audit/gate_console.txt`, `gate_logs/`, `gate_artifacts/` |
| `bash scripts/audit_terminal_check.sh` | 0 | 56 OK | `terminal_check.txt` |
| `python -I scripts/audit_cli_check.py build/nms_tool.exe 400` | 0 | trace 400 / profile 400 agree | `cli_check_windows.txt` |
| `python -I scripts/audit_recompute.py` | 0 | 90 checks, ALL MATCH | `recompute_windows_python3.11.txt` |
| `python -I scripts/audit_bound_check.py` | 0 | 925,911 cases, worst cost per streak length 1.3333 = 4/3 (reached, not exceeded) | `bound_check.txt` |
| Gate mutation test (copy of the v4 package with `SagT` changed to `SagTT<63,true>`) | 1 | `GATE: FAIL`: build fails and the Stage 3 audit reports a second FAIL; the Stage 2 audits alone still show 2 + 2, so the Stage 2 expectation was made file-specific (section 9) | `mutation_gate_console.txt` |

Linux (WSL Ubuntu, g++ 15.2.0, Python 3.14.4; extraction of the v4 ZIP into `/tmp`, nothing installed): build 0 warnings; `bash scripts/run_all_tests.sh` `RESULT: PASS` with the same counts; ASan + UBSan (`-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined`) on the six test programs and nine `nms_tool` runs: all exit 0, no report; `audit_cli_check.py` 300 + 300 agree; saved demo outputs identical except the compiler line (`validation/v4_audit/linux_v4_zip/`, `linux_run_console.txt`, `linux_rest_console.txt`). The gate and `final_audit.py` do **not** run on Linux (see section 10). The v4.1 ZIP was extracted and tested again on both platforms; those logs are beside the ZIP (`new dsa project_SUBMISSION_V4_1_ZIP_VERIFICATION`).

## 5. Metrics audit
`scripts/audit_recompute.py` re-implements the frozen statistics from the plans (per-cell median of the paired ratio time(variant)/time(base) of the same repetition; 95 % percentile bootstrap, 10,000 resamples, `random.Random(12345)`, indices 250 and 9749) without importing the project's analysers, plus an exact distribution-free sign-interval CI as a second check. It reads only the retained raw CSV files. "Recomputed" below means this script (or the named test run) reproduced the value.
| Metric / claim | Value | Source of the claim | Evidence recomputed from | Recomputed | Verdict |
| --- | --- | --- | --- | --- | --- |
| Stage 3 rows, cells, completeness | 5,888 rows; 50 cells x 8 variants; 15/11/31 repetitions; every `ok` = 1 | report 9.4 | `stage3/results/s3_heldout.csv` | yes | agrees |
| Gain set (H* <= 0.5 log2 n and cost_B >= 32) | 16 cells; equals the pre-freeze structural pass | report 8 | `s3_heldout.csv`, `s3_structure_heldout.csv` | yes | agrees |
| P2 gain criterion | 12 of 16 cells (<= 0.90, CI upper < 1); 11 required; non-wins L_few2_n19, M_few2_n17, M_few2_n19, M_zipf1024_n17 | report 9.4 | `s3_heldout.csv` | yes | agrees; unchanged under the exact sign-interval CI |
| P3 severe regressions | 0 of 50; largest ratio 1.076 (S zipf1024 n17; n19 = 1.056) | report 9.4, README | same | yes | agrees |
| P4 comparison ratio SAG/base | maximum 1.000; 0 cells above 1.03; gain-set range 0.755-0.852 | report 9.4 | same | yes | agrees |
| Comparison savings, low-entropy expensive-key | 0.755-0.852 of the base (15-25 percent fewer) | report 1, 9.4 | same | yes | agrees |
| A/A control | largest abs(median(base_b/base) - 1) = 0.021 | report 9.4 | same | yes | agrees |
| 16 gain-cell table | comparison ratio, time ratio + CI, A/A + CI, base ns/element, win flag | report 9.4 | same | yes (every cell) | agrees (CI to 0.001) |
| Other cell groups | cheap `S` low-entropy 0.938-1.076; nearly sorted time ratios L/M/S; perm/runs/mixture 0.871-1.044 (comparison 0.891-1.000); FILE cells 0.957 [0.944, 0.966] and 1.007 | report 9.4 | same | yes | agrees |
| **Nearly sorted comparison ratio** | **0.517-0.547** (report said 0.517-0.541) | report 9.4 table; `STAGE3_CONFIRMATION_RESULTS.md` section 3 | `summary_s3_heldout.md` rows and `s3_heldout.csv` | yes | **error found and corrected** (cell L_nearly_n17 = 0.547) |
| Time per comparison | L 17-71 ns, M 11-28 ns, S 7-17 ns | report 9.4 | `s3_heldout.csv` | yes | agrees |
| SAG-C diagnostic | 12 of 16 gain-set wins; FILE natural 1.276, the only severe cell | report 9.4 | same | yes | agrees |
| Randomisation of variant order | `order_pos` is a permutation of 0..7 in all 686 (cell, repetition) groups; mean position of base 3.57, of SAG 3.48 | report 8 | `s3_heldout.csv` | yes | agrees |
| Stage 2 | 18 cells; SAG-C 6 of 8 low-entropy gains but severe at runs(64) n17: 1.114 [1.104, 1.150]; round-1 SAG 6 of 8 and no severe cell | report 9.2 | `stage2/results/string_heldout.csv` | yes | agrees |
| Stage 2 SAG-C cost factor | 3.7-9.1 times the cost inside the sort | report 9.2 | `stage2/` audit report | no | taken from the saved Stage 2 audit; not re-derived here |
| Stage 1 round 1 | 72 held-out cells; heavy low-entropy 8 of 8 wins, 0.794-0.887, worst heavy 1.022; 54 cheap cells all slower (1.005-1.240), 12 severe; comparison ratio never above 1.000 | report 9.1 | `results/raw/heldout.csv` | yes | agrees |
| Stage 1 round 2 (SAG-C) | heavy low-entropy 0.725-0.863; 2 of 36 cheap cells >= 1.10 (1.103, 1.120) | report 9.1 | `results/raw/heldout2.csv` | yes | agrees |
| Stage 1/2 timings as measurements | different machine / toolchain | `KNOWN_LIMITATIONS.md` 6 | - | no (not re-measured) | stated limitation |
| Correctness, stability, counters | counts in section 4; `ok` = 1 in every raw row | tests, CSV | tests run, CSV read | yes | agrees |
| Exception safety | 84,623 throw-at-every-call cases, 0 lost items; before the fix 32,069 | report 10 | `test_exception` run (84,624 checks); the 32,069 figure is from the saved Stage 3 reproduction | partly | after-fix number reproduced; before-fix number from the saved log |
| SAG per-streak bound | at most 4/3 of the streak length | report 6 | finite range t <= 70, tails <= 160: maximum exactly 4/3 (925,911 cases) | yes (finite range) | agrees; **a finite-range check, not a proof**; the "1.3317 observed in fuzz runs" figure comes from a saved log |
| H and H* definitions and values | H over the greedy run decomposition, H* over the monotone runs of the inverse stable-rank permutation | `src/entropy.hpp`, TERMINAL_DEMO 4 | 800 random inputs against definitions from scratch | yes | agrees |
| Data provenance | 48 synthetic cells (generators with seeds in `bench_string3.cpp`); 2 FILE cells = 19,611 relative file names of a local MSYS2 installation (names only), natural and shuffled | plan 4 | plan, `stage3/data/` | no (provenance is documentation) | consistent with the plan and the freeze record |

Direction and units: ratios are variant over base throughout, so a value below 1 favours SAG; time is nanoseconds per element; "10 percent faster" is a ratio <= 0.90. Claim types are kept apart in the documents: bounds taken from the paper (`O(n + n H*)`, "naive + O(n)"), our own bound (4/3, finite range), and measured results (counts, times).

## 6. Research protocol
The frozen Stage 3 evidence is untouched (byte-identical files, section 2; the Stage 3 audit passes completely on a tree with the v3 header). No new scientific timing experiment was run. The timings printed by `nms_tool` are demonstration output, labelled as such and never mixed with the frozen results. No evidence gap was found that requires an additional experiment, so no exploratory plan was written.

## 7. Syllabus gate
Allowed: instrumentation, profiling and comparison of the existing Merge Sort variants; documentation. Rejected: parallel or heap-based merging, new sorting algorithms, changes to SAG constants or the merge. Residual risk (unchanged): natural runs, PowerSort order, galloping and the SAG rule are Merge Sort refinements that are not individually named in the syllabus; the syllabus text itself is not in the repository; faculty approval is not claimed.

## 8. Reproducibility
Fresh extraction in paths with spaces (Windows: `D:\new dsa project_ZIPTEST...`; Linux: `/tmp/.../new dsa project_SUBMISSION_V4`), clean build with 0 warnings, tests, manifest and hash checks: see section 4 and the `..._ZIP_VERIFICATION` folders beside the ZIPs. To reproduce the tables: `python -I scripts/audit_recompute.py` (reads only the retained CSV files); the original tables are produced by `python -I stage3/string3/analyze_string3.py stage3/results/s3_heldout.csv --criteria` (writes into `stage3/results/`; run on a copy to keep the historical file).

## 9. Corrections made (release v4.1; v3 and v4 ZIPs untouched)
| File | Change | Why |
| --- | --- | --- |
| `docs/FINAL_PROJECT_REPORT.md` | nearly sorted comparison range 0.541 -> 0.547; addendum; limitation text on sanitizers | metric error found by recomputation |
| `docs/STAGE3_CONFIRMATION_RESULTS.md` | appended addendum (history untouched) | the same error appears in this historical report |
| `stage3/tools/final_pass3.sh` | the Stage 2 audits must fail with exactly the expected changed-file lists, not just 2 + 2 FAIL lines | a mutation test showed the count-only check alone would not notice a different changed file |
| `validation/v4/asm_check/asm_compare.py` | file-level symbol directives (`.def`, `.hidden`, `.weak`) compared as one set | on Linux g++ places two such directives in different sections; no code differed |
| `docs/TERMINAL_DEMO.md` | explains that the unsearched tail streak of a merge is not a block | printed blocks do not add up to the merge size, which could be misread |
| `README.md`, `docs/KNOWN_LIMITATIONS.md` (section 13), `docs/SUBMISSION_HANDOVER.md`, `docs/FINAL_DEMO_CHECKLIST.md` | Linux and sanitizer status, platform support of the gate | earlier statements were out of date for the build and tests |
| new: `docs/AUDIT_V4_CODE.md`, `scripts/audit_*`, `validation/v4_audit/` | audit record and re-runnable checks | evidence |

## 10. Remaining limitations
* The Stage 3 gate and `scripts/final_audit.py` are Windows-only: the gate looks for `build/*.exe`, and `final_audit.py` check D compares re-derived summaries byte for byte with files stored with CRLF line endings (verified: identical after removing `\r`). The historical scripts were not changed. Linux support is for the build, `run_all_tests.sh`, the sanitizer builds and the tool.
* ASan/UBSan covered the six test programs and nine tool invocations; the Stage 3 red-team fuzz was not re-run under them. MinGW and other compilers: untested.
* Stage 1/2 timings were re-derived from the saved rows, not re-measured; SAG-C's cost factor and the fuzz-run figure 1.3317 come from saved logs; the 4/3 bound is checked on a finite range only.
* Result is conditional (12 of 16, margin 1), mostly synthetic strings, one machine; all audits (including this one) are internal; article-level Scopus indexing is unverified; the syllabus-approval risk of section 7.
* The review deck was outside this audit.
