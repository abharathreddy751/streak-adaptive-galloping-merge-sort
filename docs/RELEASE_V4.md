# Release record v4: terminal diagnostics

Date 2026-10-27. Scope: make the verified v3 project easier to demonstrate and to inspect in a terminal, without changing any algorithm, any frozen experiment or any scientific conclusion. The v3 release (`new dsa project_SUBMISSION_20261021_v3.zip`, SHA-256 `50f90609de8087ed8e5f7de5456510d41effb9c106c3752730e55b5711ca543f`, GitHub commit `3a46350`) is kept unchanged as the historical release; v4 is a separate package built from a separate working copy and branch.

## 1. Syllabus-compliance audit of the v4 changes (MACSE501, Module 3: Merge Sort)
Result: **PASS for every v4 change; no new algorithm or data structure was introduced.** Details and the unchanged residual risk: `docs/SYLLABUS_COMPLIANCE.md`, section 6.

## 2. What v4 adds, and why
| Addition | Where | Why it is useful |
| --- | --- | --- |
| Optional **execution trace** of the sorter (six event kinds: run found, merge begin, merge skipped, block found, SAG `t` change, merge end) | `src/natural_merge_sort.hpp` (`TraceEvent`, `TraceSink`, `natural_merge_sort_traced`) | Lets a small sort be explained from what the code actually did (runs, merge order, galloping blocks, SAG adaptation), instead of from a hand-written narrative |
| **Input profiler** (distinct keys, descents, run structure, H and H*, fixed-definition labels) | `src/diag.hpp` | States what an input IS by measurement; keeps generator metadata and measured values apart |
| **Variant comparison** with identical input copies, independent correctness and stability checks, double-counted comparisons, block statistics, optional timing | `src/diag.hpp`, `bench/nms_tool.cpp` | One command shows textbook / naive / base / SAG / `std::stable_sort` side by side and cross-checks the counters |
| **String-record demo** (`strings`): 64-byte keys with a shared prefix, stability check, timing | `bench/nms_tool.cpp` | A practical example of the case where a comparison is expensive, tied to the merge step |
| **Terminal tool** `nms_tool` (`profile`, `trace`, `compare`, `strings`, `help`) with argument validation | `bench/nms_tool.cpp`, `docs/TERMINAL_DEMO.md` | Easy to demonstrate; no GUI |
| Tests: `tests/test_diag.cpp` (1,686,861 checks), `tests/test_cli.sh` (45 checks) | `tests/` | Trace does not change output or counters; event stream agrees with exact counters; profiler is correct on known cases; n = 1, 2, 3, all-equal, sorted, reversed, odd sizes, minrun 1/4/32; CLI exit codes, invalid input, determinism |
| Assembly identity check for the uncounted (timed) sorters | `validation/v4/asm_check/` | Shows that adding the hook did not change the code generated for timed builds (`-O2`) |

How the hook stays out of the way: events are produced only by counting (`Count = true`) sorters on which `set_trace()` was called; the pointer lives in a base class that is empty for `Count = false`, so the layout and the generated code of every timed sorter are those of v3 (`validation/v4/asm_check/asm_check_output.txt`: all 78 functions identical, 0 different). `Stats` is unchanged.

## 3. Considered and not done
| Idea | Decision and reason |
| --- | --- |
| Parallel merge sort, k-way merge by heap or tournament tree | Rejected: heap/priority-queue and parallel algorithms are outside the permitted scope |
| New adaptive rule for `minrun` or new SAG constants | Rejected: changes the algorithm and the frozen configuration; would need a new preregistered experiment, not justified by any observed defect |
| New timing experiment | Not run: the frozen Stage 3 experiment answers its question; `nms_tool --reps` is a demonstration and is labelled so. No scientific conclusion changed |
| Memory measurement | Not added: the buffer size is already a counter (`max_buffer`); process-level memory is not reliably measurable with the available tools |
| Strong exception guarantee | Not done: would change the core merge; the basic guarantee stays documented |
| Downloading real string datasets | Not done: no suitable data are already in the project and the downloads would add dependencies; the real file-name lists of Stage 3 remain the only real strings |
| Sanitizers, Linux, MinGW | Not available in this environment; still not claimed |

Evaluation gap analysis: the existing generators already cover sorted, reversed, nearly sorted, random, duplicate-heavy, Zipf-skewed, many-run and pipe-organ integer inputs, and the Stage 2/3 string experiments cover expensive keys. The only gap that mattered for demonstration was a per-input measured profile, which `profile` now provides.

## 4. Changes to existing files
* `src/natural_merge_sort.hpp`: trace hook only (see above); no change to any sorting logic. The v3 header is kept in `validation/v4/asm_check/orig_hdr/` and the Stage-3 original in `stage3/artifacts/orig_src/`.
* `scripts/build.sh`, `scripts/run_all_tests.sh`: build and run `test_diag`, `nms_tool` and `tests/test_cli.sh`. The five earlier test programs and the demo-output comparison are unchanged; their check counts are identical to v3.
* `stage3/tools/final_pass3.sh`: also runs `test_diag` and `test_cli.sh`. Because the header changed, the Stage 3 independent audit now shows exactly one FAIL on the live tree (its frozen-header hash, same kind of check as the Stage 2 and `final_audit.py` header checks that were already expected to fail) and the gate expects exactly that; the gate additionally runs the Stage 3 audit on a copy with the **v3 header** and requires 25 PASS / 0 FAIL / 0 DISCREPANCY, and runs `final_audit.py` on the pristine-header copy as before. No expectation was loosened for any other check.
* Documentation: `README.md`, `docs/TERMINAL_DEMO.md` (new), `docs/SYLLABUS_COMPLIANCE.md` (section 6 added), `docs/KNOWN_LIMITATIONS.md` (section 12 added), `docs/FINAL_PROJECT_REPORT.md` (addendum), `docs/BASE_PAPER_IMPLEMENTATION.md` (note), `docs/VIVA_PREP.md`, `docs/FINAL_DEMO_CHECKLIST.md`, `docs/SUBMISSION_HANDOVER.md`, `docs/SUBMISSION_CHANGES.txt`, `validation/README.md`. Historical reports, frozen plans, raw data and earlier results are untouched.

## 5. Scientific conclusions
Unchanged. SAG remains a conditional improvement (12 of 16 gain cells against 11 required; no severe regression in 50 cells; comparisons never higher than the base; no general speed-up). `nms_tool` outputs are demonstrations on generated data; for cheap integer keys they typically show fewer comparisons and no time gain, which agrees with the frozen results.

## 6. Review of the research claims
Checked against the saved evidence: the paper is identified as Ghasemi, Jugé, Khalighinejad, Yazdanyar, *Galloping in Fast-Growth Natural Merge Sorts*, Algorithmica 87(2), 2025, DOI 10.1007/s00453-024-01285-6 (`docs/PAPER_VERIFICATION.md`); the code is described everywhere as an independent re-implementation (not the authors' code, not their proofs); the worst-case bound for SAG (4/3 of a streak) is described as checked on a finite range, not proven; the exception guarantee is described as basic; the experimental claims match `docs/STAGE3_CONFIRMATION_RESULTS.md`; article-level Scopus indexing remains **unverified** (Scopus needs a login); all audits are internal.

## 7. Evidence
`validation/v4/` (logs of the final test run and gate run, `asm_check/`), `results/outputs/v4_*.txt` (saved outputs of the deterministic demo commands), the extraction test of the v4 ZIP (`..._v4_ZIP_VERIFICATION` beside the ZIP) and `docs/SUBMISSION_HANDOVER.md`.

## 8. Still open (unchanged by v4)
Sanitizer runtimes unavailable; Linux and MinGW untested; article-level Scopus unverified; the review deck could not be inspected on the real artifact without a sign-in and its sharing has not been changed; mostly synthetic data and one machine for the confirmation experiment; the Windows-only tool runs were not repeated on another platform.
