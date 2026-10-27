# Streak-Adaptive Galloping for Natural Merge Sort

Data Structures and Algorithm Analysis - postgraduate project (CSE-AIML). Syllabus topic: **Merge Sort** (Module 3).

## Abstract
Merge Sort is the standard divide-and-conquer sort. Production sorts such as TimSort (Python, Java) are *natural merge sorts*: they find the sorted runs that already exist in the data and merge those. Inside each merge, the sorter finds the end of every "streak" of items coming from the same run by *galloping* (probe a few items one by one, then jump 1, 2, 4, 8 ... places and binary-search back). This project re-implements the relevant method of the paper below in C++17 and evaluates one proposed extension, **SAG (Streak-Adaptive Galloping)**, which adapts the galloping parameter `t` while a merge is running instead of fixing it per merge.
The evidence is conditional: SAG reduces comparisons on low-entropy inputs, and it reduces elapsed time only when a key comparison is expensive (long strings in this study). It is **not** presented as a faster sort in general.

## Base paper
E. Ghasemi, V. Jugé, G. Khalighinejad, H. Yazdanyar, "Galloping in Fast-Growth Natural Merge Sorts", *Algorithmica* 87(2), 242-291, Springer; published online 9 December 2024 (issue year 2025). DOI: [10.1007/s00453-024-01285-6](https://doi.org/10.1007/s00453-024-01285-6). A correction notice (DOI 10.1007/s00453-025-01297-w) concerns one author's affiliation only.
This code is an **independent re-implementation of the relevant method, not the authors' code**. It does not reproduce the paper's proofs or all of its constructions. Individual-article Scopus indexing is **not verified**.

## Base algorithm versus SAG
| | Base (paper's policy) | SAG (primary extension) |
| --- | --- | --- |
| Runs and merge order | detect runs (minrun 32, binary insertion), PowerSort or TimSort stack rule | identical |
| Merge | copy the smaller run to a buffer, forward or backward stable merge, galloping | identical |
| Galloping parameter `t` | fixed per merge: `t = ceil(log2(a+b))^2` | starts at the same value, then adapted after each long streak using the paper's own cost formula |
| Extra memory / extra key comparisons | none | none |

SAG-C (`src/sag_time_policy.hpp`) is a corrective, time-aware variant. It failed its safety criteria in Stage 2 and had a severe regression in a Stage 3 diagnostic. It is kept as a documented alternative, not as the final candidate.

## Workflow in short
1. Find the natural runs. 2. A stack rule decides which two neighbouring runs to merge. 3. Merge them using a temporary buffer. 4. Inside the merge, locate each streak end by galloping with parameter `t`. 5. Base: `t` stays fixed. SAG: after a long streak, `t` moves to whichever of `t/2`, `t`, `2t+2` or `0` the cost formula says is cheapest; it resets at the next merge.
Details: `docs/BASE_PAPER_IMPLEMENTATION.md`, `docs/PROPOSED_METHOD.md`, `docs/FINAL_PROJECT_REPORT.md`.

## Directory map
| Path | Content |
| --- | --- |
| `src/` | header-only C++17 code: `natural_merge_sort.hpp` (base paper method + exception guard + optional trace hook), `sag_policy.hpp` (SAG), `sag_time_policy.hpp` (SAG-C), `textbook_merge_sort.hpp`, `generators.hpp`, `entropy.hpp`, `diag.hpp` (v4 diagnostics) |
| `tests/` | `test_sort.cpp` (built as `test_base` and `test_all`), `test_sag.cpp`, `test_sagt.cpp`, `test_exception.cpp`, `test_diag.cpp` and `test_cli.sh` (v4) |
| `bench/` | `demo_sort.cpp` (demonstration), `nms_tool.cpp` (v4 terminal tool), `bench_sort.cpp` (Stage 1-2 benchmark driver) |
| `scripts/` | `build.sh`, `run_all_tests.sh` (clean build + all tests), Stage 1-2 runners and analysis |
| `results/` | Stage 1-2 raw CSV data, frozen manifests and freeze records, summaries, figures |
| `stage2/`, `stage3/` | Stage 2 and Stage 3 evidence: benchmark sources, raw data, logs, independent checkers, freeze records |
| `docs/` | report, plans, audit reports, limitations, viva preparation, demo checklist, handover |
| `validation/` | logs of the clean-build and test runs performed on this submission package |

## Requirements
`g++` with C++17 (tested: GCC 15.3.0, MSYS2, target x86_64-pc-cygwin, Windows 11), Bash (Git-Bash or MSYS2), Python 3 standard library (tested 3.11.9). No other dependency, no network, no database. The project folder may contain spaces. Linux (WSL Ubuntu, g++ 15.2.0, Python 3.14.4) was tested in the v4 code audit for the build, `scripts/run_all_tests.sh` and sanitizer builds, but not for the Stage 3 gate; MinGW and other compilers were **not tested**.

## Build, test, demo (commands that were run on this package)
```bash
cd "<project folder>"
bash scripts/run_all_tests.sh        # clean build, six test programs, demo, CLI tests; exits non-zero on any failure
```
Verified output of the five test programs (see `validation/`): `test_base` 1502829 checks passed, 0 failed; `test_all` 2100389 checks passed, 0 failed; `test_sag` 265609 checks passed, 0 failed; `test_sagt` 760686 checks passed, 0 failed; `test_exception` 84624 checks passed, 0 failed. `test_diag` (v4) 1686861 checks passed, 0 failed; `tests/test_cli.sh` 45 checks passed, 0 failed. These suites overlap in coverage; do not add the numbers. The demo (`./build/demo_sort.exe`) ends with `all outputs equal the reference stable sort: YES` and its output equals `results/outputs/demo_output.txt`.

Stage 3 final gate (about 8 minutes; builds the project three times and re-runs the audits):
```bash
bash stage3/tools/final_pass3.sh     # prints GATE: PASS or GATE: FAIL and sets the exit status
```
`scripts/final_audit.py` (the older audit) is expected to report one FAIL (check G) on this tree, because `src/natural_merge_sort.hpp` was deliberately changed in Stage 3 (exception guard) and in v4 (trace hook); the Stage 3 audit likewise shows one header-hash FAIL on the live tree and passes completely on a copy with the v3 header. The gate script runs it on the live tree, expects exactly that FAIL, and also runs it on a pristine-header copy, where it reports 0 FAIL.

## Terminal demonstration (v4)
```bash
./build/nms_tool.exe trace                                    # step-by-step events of a 24-element sort (SAG)
./build/nms_tool.exe profile --family few --param 3 --n 262144 # measured description of an input
./build/nms_tool.exe compare --family few --param 3 --n 262144 # textbook / naive / base / SAG / std::stable_sort, identical input
./build/nms_tool.exe strings                                  # 64-byte string keys, stability check, timing
```
On Windows with the MSYS2 compiler put `/c/msys64/usr/bin` on `PATH` first. Commands, options, output legend and limits: `docs/TERMINAL_DEMO.md`; change record: `docs/RELEASE_V4.md`. The tool adds no algorithm and changes no result.

## Results in brief (details and caveats: `docs/FINAL_PROJECT_REPORT.md`)
* **Stage 1 (integer keys, synthetic comparator cost):** partial success. 15-27 percent fewer comparisons on low-entropy inputs; time gain only when comparisons are expensive; SAG slower in all 54 cheap-comparison cells.
* **Stage 1 corrective round (SAG-C):** corrective partial; 2 of 36 cheap cells still regressed by 10 percent or more.
* **Stage 2 (string comparator, one 240-character shared prefix):** SAG-C failed its severe-regression guard; SAG met the same rule as a secondary candidate.
* **Stage 3 (frozen confirmation of SAG, 50 cells):** SAG reached a time ratio of 0.90 or better (confidence-interval upper bound below 1.0) in **12 of the 16** expensive-key, low-entropy gain cells (11 were required), no severe regression in any of the 50 cells, comparison ratio at most 1.000, all 5,888 rows correct, independent internal checker 25 PASS / 0 FAIL / 0 DISCREPANCY. **Conditional confirmation success**: 48 of the 50 cells are synthetic variable-length strings, the other two are small file-name lists, one machine and one toolchain. Worst Stage 3 ratio 1.076 (short synthetic keys, `zipf(1024)`, n = 2^17).
* **Exception safety (Stage 3):** a comparator that throws could lose or duplicate items (32,069 of 84,623 reproduced cases). The fix gives the **basic guarantee** (the array stays a permutation; order and stability are not guaranteed after the exception); 0 of 84,623 after the fix.

## Limitations
See `docs/KNOWN_LIMITATIONS.md`. Main points: conditional result, narrow margin (12 against 11), mostly synthetic data, one machine and compiler for Stage 3, comparison counts are not time, the audits are internal, sanitizer runtimes were not available on the Windows toolchain (they passed in WSL Ubuntu in the v4 code audit), the Stage 3 gate is Windows-only, article-level Scopus indexing is unverified.

## Reading order
`docs/FINAL_PROJECT_REPORT.md`, `docs/KNOWN_LIMITATIONS.md`, `docs/TERMINAL_DEMO.md`, `docs/VIVA_PREP.md`, `docs/FINAL_DEMO_CHECKLIST.md`, `docs/SUBMISSION_HANDOVER.md`. The historical stage reports are kept unchanged: `docs/FINAL_REPORT.md`, `docs/STAGE2_*.md`, `docs/STAGE3_*.md`; the original Stage 1-2 README is `docs/README_STAGE1_2_HISTORICAL.md`.
