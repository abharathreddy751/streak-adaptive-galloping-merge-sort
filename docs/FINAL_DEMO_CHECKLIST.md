# Final demo checklist

Order of the live demonstration (about 10 minutes). Run all commands from the project folder in Git-Bash or MSYS2 (`cd "<project folder>"`). Status of each command: **tested** = executed on this package with the result shown; **not run in the demo environment** = explain only.

1. **Show the layout.** `ls` and open `README.md`. Point out `src/`, `tests/`, `bench/`, `results/`, `stage2/`, `stage3/`, `docs/`, `validation/`. (no command to verify)
2. **Identify the files.** Base paper method: `src/natural_merge_sort.hpp`. SAG: `src/sag_policy.hpp` (configuration `SagTT<64, true>`). SAG-C (diagnostic, not the candidate): `src/sag_time_policy.hpp`. Reference sort: `src/textbook_merge_sort.hpp`.
3. **Explain base versus SAG.** Same code for runs, stack rule, merge and galloping. Only the rule for `t` differs: base fixes `t = ceil(log2(a+b))^2` per merge, SAG adapts it after long streaks using the paper's cost formula (details: `docs/PROPOSED_METHOD.md`, README table).
4. **Build and run the tests and demo (tested).**
   ```bash
   bash scripts/run_all_tests.sh
   ```
   Expected (about 1.5 minutes, from `validation/run_all_tests_run1.txt` and `run2`): `test_base exit=0 :: test_sort: 1502829 checks passed, 0 failed`; `test_all` 2100389; `test_sag` 265609; `test_sagt` 760686; `test_exception` 84624, all `0 failed`; `demo_sort exit=0 :: all outputs equal the reference stable sort: YES`; `demo output identical to results/outputs/demo_output.txt`; `RESULT: PASS`.
   To show only the demo after the build (tested): `./build/demo_sort.exe` - it prints comparisons per element, for example for 3 distinct values: naive 14.454, base 8.272, SAG 6.776.
5. **Show the reproducible results and verification evidence.**
   - `docs/STAGE3_CONFIRMATION_PLAN.md` (frozen plan) and `stage3/string3/freeze_record.txt` (hashes, written before the run).
   - `stage3/results/summary_s3_heldout.md` (all 50 cells) and `docs/STAGE3_CONFIRMATION_RESULTS.md`.
   - `stage3/audit/audit_stage3_output.txt` (independent internal checker: 25 PASS, 0 FAIL, 0 DISCREPANCY; shows the recomputation) (tested: shipped output; it is re-run by the gate).
   - Optional (tested, about 8 minutes, prints `GATE: PASS`): `bash stage3/tools/final_pass3.sh`. The timing experiment itself is not re-run.
6. **Explain the key result, the limitation and the exception-safety fix.**
   - Result: 12 of 16 gain cells at least 10 percent faster (11 required); no severe regression in 50 cells; worst ratio 1.076.
   - Limitation: conditional on expensive keys and low entropy; narrow margin; mostly synthetic data; one machine and compiler (`docs/KNOWN_LIMITATIONS.md`).
   - Exception fix: show `GapGuard` in `src/natural_merge_sort.hpp` and the test `tests/test_exception.cpp` (84,623 throw-at-every-call cases, 0 lost items; before the fix 32,069 of them lost items); basic guarantee only.
7. **End with the conclusion.** "SAG is a conditional improvement for expensive-key, low-entropy string workloads, confirmed under a frozen rule on one machine. It is not a faster sort in general. SAG-C is not recommended."

Not run in the demo environment: builds on Linux or with MinGW or other compilers; AddressSanitizer/UBSan runtimes; any re-run of the Stage 1-3 timing experiments (their raw data are included).

## v4 terminal demonstration (about 5 minutes; commands and output legend: `docs/TERMINAL_DEMO.md`)
1. `bash scripts/build.sh` (or `bash scripts/run_all_tests.sh`). On Windows with the MSYS2 compiler: `export PATH="/c/msys64/usr/bin:$PATH"`.
2. `./build/nms_tool.exe trace` : walk through runs, the two merges, the blocks and the final `[verify]` line.
3. `./build/nms_tool.exe compare --family few --param 3 --n 262144` : read the profile (generator metadata versus measured values), then the table; point out that all variants are `correct` and `stable` and that SAG uses fewer comparisons.
4. `./build/nms_tool.exe strings` : 64-byte string keys, stability check, timing; say clearly that the timing is a demonstration on one machine, not the experiment.
5. Say what the tool does not prove: the evidence is the frozen Stage 3 experiment.
