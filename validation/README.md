# Validation logs of this submission package

All runs were made on this package folder (not on the original project) on 2026-10-21, with GCC 15.3.0 (MSYS2, Cygwin target), Python 3.11.9, Windows 11 build 26200, from a Git-Bash shell. The folder name contains a space; the commands handled it.

| File or folder | What it shows |
| --- | --- |
| `run_all_tests_run1.txt`, `run_all_tests_run2.txt` | two consecutive clean builds (`rm -rf build` first) of `bash scripts/run_all_tests.sh`; both `RESULT: PASS`, exit 0 |
| `negative_test_failing_compiler.txt` | the same script with `CXX=false`: `RESULT: FAIL (build)`, exit 1 (a failed build is reported as failure) |
| `gate_run1/` | first run of `bash stage3/tools/final_pass3.sh`: **GATE: FAIL**. Cause: two defects in the gate script's own checks, found by this run and fixed (MSYS2 `grep` 3.0 matched nothing for the pattern `^\[FAIL\]`, and the temporary-directory step mixed two `/tmp` locations). The build, five tests, demo and Stage 3 audit passed in this run. Kept as evidence, not as the final result |
| `gate_run2/` | second run with the fixed script: **GATE: PASS**, exit 0. `gate_console.txt` is the complete output; `logs/` and `artifacts/` hold the individual logs and audit outputs of that run |
| `gate_run3/` | third run after one more script fix: **GATE: PASS**, exit 0 (`gate_console.txt`). The fix came from the first ZIP-extraction test (below): the gate failed there only because the log directories were given as absolute paths and the script prefixed the project root to them. The script now converts `NMS_LOG_DIR` and `NMS_ART_DIR` to absolute paths first |
| first ZIP (`new dsa project_SUBMISSION_20261021.zip`) and its verification folder | **superseded**. Its extraction test (clean build and five tests passed, manifest 283/283 OK) exposed the absolute-path defect above in the gate, so a corrected package `..._v2.zip` was built and tested. The first ZIP and its logs were kept, not overwritten |
The historical logs and audit outputs elsewhere in the package were not overwritten (the gate was run with redirected log directories and restores every audit-output file it temporarily changes).
The extracted-ZIP verification (clean build and gate run from an extracted copy of the ZIP) cannot be stored inside the ZIP it verifies; its logs are in the separate folder `new dsa project_SUBMISSION_20261021_v2_ZIP_VERIFICATION` placed beside the final ZIP `new dsa project_SUBMISSION_20261021_v2.zip`.
