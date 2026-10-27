# Release record: v4.1 (v4 plus the code-side audit)

Date 2026-10-27. This repository holds the package folder `new dsa project_SUBMISSION_V4_1` (without the ZIP). Branch `v4-audit`. What changed relative to v4 and why: `docs/AUDIT_V4_CODE.md` section 9.

## Current release: v4.1 (delivered separately, not stored in this repository)
* File: `new dsa project_SUBMISSION_V4_1.zip` (one root folder: `new dsa project_SUBMISSION_V4_1`, 448 entries)
* SHA-256: `2910570fead48f02ae6ca978cda2443cd8187f763062df0197330083d8a76501`

## Earlier releases (immutable, unchanged)
* v4: `new dsa project_SUBMISSION_V4.zip`, SHA-256 `99cd44d5d15b38f9525018c78a231dbc3b5e5dd6a0e21db8600a9749e46367b0`, commit `a709d97` (`main`, branch `v4-terminal-diagnostics`).
* v3: `new dsa project_SUBMISSION_20261021_v3.zip`, SHA-256 `50f90609de8087ed8e5f7de5456510d41effb9c106c3752730e55b5711ca543f`, tag `v3-submission` = commit `3a46350`. The first ZIP and `_v2` are obsolete evidence.

## Verified results (v4.1)
* `bash scripts/run_all_tests.sh` -> `RESULT: PASS`, exit 0: `test_base` 1502829, `test_all` 2100389, `test_sag` 265609, `test_sagt` 760686, `test_exception` 84624, `test_diag` 1686861 checks, all 0 failed; `tests/test_cli.sh` 45 checks, 0 failed (overlapping suites, do not add).
* `bash stage3/tools/final_pass3.sh` -> `GATE: PASS`, exit 0, on the source tree (`validation/v4_audit/`) and in a fresh extraction of the ZIP. Expected by design: `final_audit.py` one FAIL on the live tree (header hash) and 0 on the pristine-header copy; Stage 2 audits 2 + 2 FAILs naming exactly the expected changed files; Stage 3 audit one FAIL (header hash) on the live tree and 25 PASS / 0 FAIL on a copy with the v3 header.
* Metrics: `python -I scripts/audit_recompute.py` recomputes the report's headline numbers from the raw CSV files: 90 checks, all match (one report error was found and corrected, see the audit). Terminal tool: `scripts/audit_terminal_check.sh` 56 of 56, `scripts/audit_cli_check.py` trace and profile cross-check against a separate Python model.
* Linux (WSL Ubuntu, g++ 15.2.0, Python 3.14.4), fresh extraction: build 0 warnings, `run_all_tests.sh` PASS, ASan + UBSan on the six test programs and nine tool runs PASS, audit scripts PASS. The gate is Windows-only.
* Manifest: 447 listed files, 0 mismatches, 0 duplicates, none missing or extra. Scan for secrets and personal-data patterns: 0 hits.

## How to verify
1. Compare the ZIP hash with the value above (`sha256sum` or `Get-FileHash`).
2. Extract, then from the package root: `grep -v '^#' SUBMISSION_MANIFEST.sha256 | awk '{print $1"  "$3}' | sha256sum -c -`
3. `bash scripts/run_all_tests.sh` and, on Windows, `bash stage3/tools/final_pass3.sh` (g++ with C++17, Bash, Python 3). Terminal demo: `docs/TERMINAL_DEMO.md`.

## Not verified / limits
MinGW and other compilers untested; the gate and `scripts/final_audit.py` do not run on Linux; Stage 1/2 timings not re-measured; article-level Scopus unverified; the review deck was outside this audit; faculty syllabus approval is not claimed. Details: `docs/KNOWN_LIMITATIONS.md` (section 13), `docs/AUDIT_V4_CODE.md` (section 10).
