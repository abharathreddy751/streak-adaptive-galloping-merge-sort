# Submission handover

## Package
* Folder: `new dsa project_SUBMISSION_20261021` (one project root). **Final ZIP: `new dsa project_SUBMISSION_20261021_v3.zip`**, containing exactly one root folder, `new dsa project_SUBMISSION_20261021`. Its SHA-256 is in the separate file `new dsa project_SUBMISSION_20261021_v3.zip.sha256` beside the ZIP (a checksum cannot be stored inside the archive it describes); the GitHub repository's `RELEASE.md` repeats it.
* **Obsolete ZIPs, kept only as historical evidence, not for submission:** `new dsa project_SUBMISSION_20261021.zip` (first package; its extraction test found a gate-script defect with absolute log paths) and `new dsa project_SUBMISSION_20261021_v2.zip` (corrected, but built before the final release audit recorded in `docs/RELEASE_AUDIT_V3.md`).
* The original project folder `D:\new dsa project` was not modified. Checkpoints taken before packaging: CP2 (before Stage 3), CP3 (Stage 3 final), CP4 (just before packaging), in `D:\new dsa project_CHECKPOINTS` (outside the package).

## Required tools
`g++` with C++17 support; Bash (Git-Bash or MSYS2); Python 3 standard library. Tested: GCC 15.3.0 (MSYS2, target x86_64-pc-cygwin), Python 3.11.9, Windows 11 build 26200, Intel i5-13500HX. Not tested: Linux, MinGW, other compilers. No network, database or extra libraries are needed. On this machine the scripts find the MSYS2 `g++` at `/c/msys64/usr/bin` automatically if `g++` is not on the PATH.

## Commands that were run on the package, and their results
| Command | Result |
| --- | --- |
| `bash scripts/run_all_tests.sh` (twice, each from a clean `build/`) | both `RESULT: PASS`, exit 0: `test_base` 1502829, `test_all` 2100389, `test_sag` 265609, `test_sagt` 760686, `test_exception` 84624 checks, all 0 failed; `demo_sort` exit 0 and its output equals `results/outputs/demo_output.txt` (`validation/run_all_tests_run1.txt`, `run2`). These suites overlap; the counts must not be added. |
| `CXX=false bash scripts/run_all_tests.sh` | `RESULT: FAIL (build)`, exit 1: a failed build is not reported as success (`validation/negative_test_failing_compiler.txt`) |
| `bash stage3/tools/final_pass3.sh` run 1 | GATE: FAIL because of two defects in the gate script's own checks (found and fixed, see `validation/README.md`) |
| `bash stage3/tools/final_pass3.sh` runs 2 and 3 (corrected script; run 3 after the absolute-path fix) | **GATE: PASS**, exit 0 (`validation/gate_run2/gate_console.txt`, `gate_run3/gate_console.txt`): clean build exit 0 with no warnings; the five tests exit 0; demo identical; `scripts/final_audit.py` on the final source shows exactly one FAIL (check G, the frozen-header hash of `src/natural_merge_sort.hpp`, expected because of the Stage 3 exception guard) and 0 FAIL on a pristine-header copy; the two Stage 2 audits fail only on `natural_merge_sort.hpp` hash checks (2 + 2, as expected); Stage 3 independent audit 25 PASS, 0 FAIL, 0 DISCREPANCY; historical audit outputs restored |
| Release-audit run on the final source (v3 candidate) | `bash scripts/run_all_tests.sh`: `RESULT: PASS`, exit 0; `bash stage3/tools/final_pass3.sh`: `GATE: PASS`, exit 0; five tests exit 0 with 1502829 / 2100389 / 265609 / 760686 / 84624 checks, 0 failed (`validation/v3/run_all_tests.txt`, `validation/v3/gate_console.txt`, `validation/v3/gate_logs/`) |
| Extraction test of the final ZIP (`_v3`) | clean build, `run_all_tests.sh` and the gate run from a fresh extraction in a path with spaces, independent of the original folder; logs in `new dsa project_SUBMISSION_20261021_v3_ZIP_VERIFICATION` beside the ZIP |
The gate does not re-run the timing experiments (they were run once, by design; raw data are included).

## Important files
`README.md`; `docs/FINAL_PROJECT_REPORT.md` (consolidated report); `docs/KNOWN_LIMITATIONS.md`; `docs/VIVA_PREP.md`; `docs/FINAL_DEMO_CHECKLIST.md`; source `src/`; tests `tests/`; Stage 3 evidence `stage3/` (plan `docs/STAGE3_CONFIRMATION_PLAN.md`, results `docs/STAGE3_CONFIRMATION_RESULTS.md`, audit `docs/STAGE3_AUDIT_REPORT.md`, raw data `stage3/results/s3_heldout.csv`, freeze record `stage3/string3/freeze_record.txt`, checker `stage3/audit/audit_stage3.py`); Stage 1-2 evidence `results/`, `stage2/`.

## Manifests
* `SUBMISSION_MANIFEST.sha256` (package root): SHA-256 and size of every file in the package except the manifest itself. It was written after the last change to the package. Verify it from the package root in Git-Bash: `grep -v '^#' SUBMISSION_MANIFEST.sha256 | awk '{print $1"  "$3}' | sha256sum -c -` (every line should print OK).
* Historical hash records (`results/outputs/freeze_record*.txt`, `stage2/FINAL_HASHES.txt`, `stage3/FINAL_HASHES.txt`, `stage3/string3/freeze_record.txt`) are unchanged. They record the hashes of the files at the time of each stage and are not the final-package manifest. `stage3/FINAL_HASHES.txt` lists the files of the original project at the end of Stage 3 (the README and `final_pass3.sh` in this package differ from those hashes by design; see `docs/SUBMISSION_CHANGES.txt`).

## Excluded from the package, and why
Listed with reasons in `docs/SUBMISSION_EXCLUDED.txt`: the nested duplicate folder `new dsa project/`, `.git` metadata, generated executables and the `build/` folder (rebuild from source), old and hardened test binaries (their hashes are kept), the third-party arXiv PDF and its page images (the SHA-256 and verification notes are kept), Python caches. Two frozen benchmark binaries (`stage2/string_exp/bench_string.exe`, `stage3/string3/bench_string3.exe`) are kept as evidence because the freeze records and checkers hash them; the build does not use them.

## Known limitations
See `docs/KNOWN_LIMITATIONS.md`. The conclusion is a conditional confirmation success for SAG (12 of 16 gain cells against 11 required; no severe regression in 50 cells), not a general speed-up.

## Validation steps that passed or remain blocked
* Passed: clean builds and the five test programs (twice); demo output; failing-build detection; Stage 3 gate (run 2); Stage 3 independent audit 25 PASS.
* Not run, with reasons: AddressSanitizer/UBSan runtimes (not installed, nothing was installed; hardened-build substitutes were run in Stage 3 and are described in the audit report); Linux and MinGW builds (not available here); re-runs of the Stage 1-3 timing experiments (not required; they were run once under frozen plans); article-level Scopus verification (Scopus needs a login).
* Blockers for the code package: none known. External items outside the package, recorded in `docs/RELEASE_AUDIT_V3.md`: the review deck (`https://claude.ai/artifact/6GjZ4xAX55LWVnJmNL68gj`, artifact id `2ab012f2-065e-499e-b671-af9633845c94`, source kept in `docs/presentation_source/`) is private and needs the owner to share it from its Share menu; it could not be opened by the audit because the page requires a Claude sign-in, so its rendering was checked only with an approximate local HTML render.
