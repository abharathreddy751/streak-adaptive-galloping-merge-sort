# Release record: submission v3

Date 2026-10-27. This repository holds the package folder `new dsa project_SUBMISSION_20261021` (without the ZIP).

## Final ZIP (delivered separately, not stored in this repository)
* File: `new dsa project_SUBMISSION_20261021_v3.zip` (one root folder: `new dsa project_SUBMISSION_20261021`, 345 entries)
* SHA-256: `50f90609de8087ed8e5f7de5456510d41effb9c106c3752730e55b5711ca543f`
* The first ZIP and `_v2` are obsolete evidence and must not be submitted.

## Verified results
* `bash scripts/run_all_tests.sh` -> `RESULT: PASS`, exit 0 (checks: `test_base` 1502829, `test_all` 2100389, `test_sag` 265609, `test_sagt` 760686, `test_exception` 84624; all 0 failed; the suites overlap, do not add them).
* `bash stage3/tools/final_pass3.sh` -> `GATE: PASS`, exit 0 (Stage 3 independent audit: 25 PASS, 0 FAIL, 0 DISCREPANCY).
* Both were run on the source tree (`validation/v3/`) and again from a fresh extraction of the final ZIP in a path with spaces.
* `SUBMISSION_MANIFEST.sha256`: 344 listed files, 0 mismatches, 0 duplicates, none missing or extra.

## How to verify
1. Compare the ZIP hash with the value above (`sha256sum` or `Get-FileHash`).
2. Extract, then from the package root: `grep -v '^#' SUBMISSION_MANIFEST.sha256 | awk '{print $1"  "$3}' | sha256sum -c -`
3. Run `bash scripts/run_all_tests.sh` and `bash stage3/tools/final_pass3.sh` (g++ with C++17, Bash, Python 3).

## Not verified / limits
Sanitizer runtimes unavailable; Linux and MinGW not tested; article-level Scopus check needs a login; the review deck could not be viewed on the real artifact by the audit. Details: `docs/KNOWN_LIMITATIONS.md`, `docs/RELEASE_AUDIT_V3.md`, `docs/SUBMISSION_HANDOVER.md`.
