# Release audit record (v3)

Date 2026-10-27. Scope: final release audit of the submission package before the `_v3` ZIP and the GitHub synchronisation. Commands and logs are named so each statement can be checked.

## 1. What was audited first
* Files reported as "changed on disk" after the `_v2` ZIP (`KNOWN_LIMITATIONS.md`, `final_pass3.sh`, Stage 2 and Stage 3 documents, `package_submission.py`, `redteam_fuzz.cpp`, `independent_audit.py`). **Finding:** every file of the package matched `SUBMISSION_MANIFEST.sha256` (0 non-OK lines), so none of them had changed since the `_v2` ZIP was built. The "changed on disk" notices came from edits made earlier in the same working session. Comparison with the original project folder: only `README.md` and `stage3/tools/final_pass3.sh` differ (intended: rewritten README, corrected gate script); all other original files are byte-identical; the files left out are the intended exclusions (`docs/SUBMISSION_EXCLUDED.txt`).
* GitHub repository (`49700f0`): all 306 tracked files were byte-identical to the package; the package additionally held two text logs ignored by the repository's `.gitignore` (`stage3/logs/asm_orig.s`, `asm_new.s`; evidence of the assembly comparison) and stale untracked build leftovers (`build/test_*`, removed before packaging).

## 2. Changes made in this release audit
| Change | Why |
| --- | --- |
| `docs/DEMO_COMMANDS.md`: banner added | The Stage 1 file said `scripts/final_audit.py` ends with "SUMMARY: 0 FAIL" and described the old machine. On the final source that audit reports exactly one expected FAIL (check G: the Stage 3 exception-guard change to `src/natural_merge_sort.hpp`). The banner marks the file as historical and points to `README.md` and the final commands |
| `docs/FINAL_PROJECT_REPORT.md`: one clause | `FINAL_REPORT.md` was described as a Stage 1-2 report; it also carries short Stage 2 and 3 addenda |
| `docs/SUBMISSION_HANDOVER.md`, `validation/README.md`, `docs/SUBMISSION_CHANGES.txt` | Final ZIP is `_v3`; the first and `_v2` ZIPs are obsolete evidence; release-audit run recorded |
| `docs/presentation_source/` added | The review deck lives only in an online artifact; its source is kept with the project |
No source code, test, experiment data, frozen plan or earlier report was changed. No defect was found in the code during this audit, so no regression test was added.

## 3. Tests on the final source (evidence in `validation/v3/`)
* `bash scripts/run_all_tests.sh` -> `RESULT: PASS`, script exit 0 (`run_all_tests.txt`): `test_base` 1502829, `test_all` 2100389, `test_sag` 265609, `test_sagt` 760686, `test_exception` 84624 checks, all 0 failed, each exit 0; `demo_sort` exit 0 and identical to `results/outputs/demo_output.txt`. The suites overlap in coverage; the counts must not be added.
* `bash stage3/tools/final_pass3.sh` -> `GATE: PASS`, exit 0 (`gate_console.txt`, `gate_logs/`, `gate_artifacts/`): clean build exit 0 with no warnings; `final_audit.py` on the live tree shows exactly the expected single FAIL (check G); on the pristine-header copy 0 FAIL; the two Stage 2 audits fail only on the `natural_merge_sort.hpp` hash checks (2 + 2, expected); Stage 3 independent audit 25 PASS, 0 FAIL, 0 DISCREPANCY.
* Claims versus code: the documentation distinguishes measured results (timings, comparison counts, test counts) from guarantees taken from the paper (`O(n + n H*)`, "naive + O(n)") and from our own bound (at most 4/3 of a streak, checked exhaustively on a finite range, not proven). Exception safety is stated as the basic guarantee only. Sanitizer runtimes were not available and are not claimed.

## 4. Security and publication audit
* Working tree (package): searched for tokens, private keys, passwords, API-key patterns, e-mail addresses and the local user name: **no matches**.
* Published Git history (`a7e46a8`, `49700f0`): no credentials. One low-risk finding in the **first** commit: the old `stage3/tools/final_pass3.sh` contains an absolute local temporary path under the Windows user profile (an abbreviated user-name fragment and a session folder name). It is not a credential; published history is not rewritten (no force push), and the current files no longer contain it. Commit author metadata carries the university e-mail address that was supplied for the commits.
* Absolute local paths remaining in essential instructions: none. `docs/SUBMISSION_HANDOVER.md` names the original project folder and checkpoints for information; `scripts/sanitize_wsl.sh` (historical) shows an example `/mnt/d/...` path.
* Package contents: no executables except the two frozen benchmark binaries that the freeze records hash (`stage2/string_exp/bench_string.exe`, `stage3/string3/bench_string3.exe`), no caches, no nested duplicate folder, no `.git`, no third-party PDF.

## 5. Review deck
* Artifact `https://claude.ai/artifact/6GjZ4xAX55LWVnJmNL68gj` resolves to artifact id `2ab012f2-065e-499e-b671-af9633845c94`, 13 slides in the order of its `deck.json` (read through the artifact interface; source saved in `docs/presentation_source/`). A newer runtime version than the one last published by this project exists, but the slide files are unchanged from what was written.
* Text review: every number on the slides was checked against `docs/STAGE3_CONFIRMATION_RESULTS.md` / `stage3/results/summary_s3_heldout.md` (12 of 16, 0 severe cells, worst ratio 1.076, 5,888 rows, 15-25 percent fewer comparisons, per-cell ratios on the chart slide).
* **Visual inspection: not possible on the real artifact.** Opening the artifact in the browser pane requires a Claude sign-in; no credentials were entered. Instead the 13 slide sources were rendered locally in a plain browser (approximate: fallback font instead of DM Sans, placeholder boxes for the icon element) and measured: no horizontal overflow on any slide and all flow content ends above the footer zone (largest bottom edge 829 px of 1080). A low-resolution screenshot of the densest slide (chart) showed a clean two-panel layout. This does **not** replace viewing the real artifact.
* **Sharing: not changed.** The artifact is private; sharing can only be done by its owner from the page's Share menu.
