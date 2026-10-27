# Release record: v4 (terminal diagnostics)

Date 2026-10-27. This repository holds the package folder `new dsa project_SUBMISSION_V4` (without the ZIP). What changed and why: `docs/RELEASE_V4.md`.

## Current release: v4 (delivered separately, not stored in this repository)
* File: `new dsa project_SUBMISSION_V4.zip` (one root folder: `new dsa project_SUBMISSION_V4`, 390 entries)
* SHA-256: `99cd44d5d15b38f9525018c78a231dbc3b5e5dd6a0e21db8600a9749e46367b0`

## Previous verified release: v3 (immutable, unchanged)
* File: `new dsa project_SUBMISSION_20261021_v3.zip`, SHA-256 `50f90609de8087ed8e5f7de5456510d41effb9c106c3752730e55b5711ca543f`
* Git tag `v3-submission` = commit `3a46350`. The first ZIP and `_v2` are obsolete evidence.

## Verified results (v4)
* `bash scripts/run_all_tests.sh` -> `RESULT: PASS`, exit 0: `test_base` 1502829, `test_all` 2100389, `test_sag` 265609, `test_sagt` 760686, `test_exception` 84624, `test_diag` 1686861 checks, all 0 failed; `tests/test_cli.sh` 45 checks, 0 failed; demo output identical to the stored output. The suites overlap, do not add them.
* `bash stage3/tools/final_pass3.sh` -> `GATE: PASS`, exit 0. Expected by design: `scripts/final_audit.py` shows exactly one FAIL on the live tree (frozen-header hash) and 0 FAIL on the pristine-header copy; the Stage 3 audit shows exactly one FAIL on the live tree (frozen-header hash) and 25 PASS / 0 FAIL / 0 DISCREPANCY on a copy with the v3 header.
* Both were run on the source tree (`validation/v4/`) and again from a fresh extraction of the ZIP in a path with spaces (clean build, tests, gate, saved demo outputs reproduced).
* `SUBMISSION_MANIFEST.sha256`: 389 listed files, 0 mismatches, 0 duplicates, none missing or extra. Scan for secrets and personal-data patterns: 0 hits.
* `validation/v4/asm_check`: the uncounted (timed) sorters compile to identical assembly with the v3 and the v4 header (`-O2`, 78 functions).

## How to verify
1. Compare the ZIP hash with the value above (`sha256sum` or `Get-FileHash`).
2. Extract, then from the package root: `grep -v '^#' SUBMISSION_MANIFEST.sha256 | awk '{print $1"  "$3}' | sha256sum -c -`
3. Run `bash scripts/run_all_tests.sh` and `bash stage3/tools/final_pass3.sh` (g++ with C++17, Bash, Python 3). Terminal demo: `docs/TERMINAL_DEMO.md`.

## Not verified / limits
Sanitizer runtimes unavailable; Linux and MinGW not tested; article-level Scopus check needs a login; the review deck could not be inspected on the real artifact. Scientific conclusions are unchanged by v4. Details: `docs/KNOWN_LIMITATIONS.md`, `docs/RELEASE_V4.md`, `docs/SUBMISSION_HANDOVER.md`.
