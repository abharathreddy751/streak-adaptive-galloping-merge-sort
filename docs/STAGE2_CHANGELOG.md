# STAGE2_CHANGELOG

Stage 2 = independent audit + real-comparator experiment. **Nothing from round 1 / round 2 was modified** (sources, scripts, raw CSVs,
manifests, freeze records, plans, summaries, verdicts). All new material lives under `stage2/`, `docs/STAGE2_*.md` and `docs/EVALUATION_PREP.md`.
Times are machine-local (IST), 2026-10-20.

## 1. Project root and frozen starting state
* Verified working directory: `D:\new dsa project` (no nested project created). The folder already contains a sub-folder
  `D:\new dsa project\new dsa project\` - a **byte-identical duplicate** of the whole project (74 files; compared by SHA-256: 0 differences,
  0 files only on one side). It was left untouched; it is not used by any script. (Origin unknown; probably an accidental copy.)
* The folder is **not a Git repository** -> checkpoint = zip archive + SHA-256 manifest, outside the project tree, read back and verified.
* **Checkpoint CP0 (before any stage-2 change), 09:38:06:**
  * `D:\new dsa project_CHECKPOINTS\CP0_pre_stage2_20261020_093806.zip`  (148 files = 74 x 2, including the old `build/` and the duplicate folder)
  * zip SHA-256 `640fa9a83de9f475e8dfdc454de5d07c82a7523c867ca0322403fcdfd6b81eb9`
  * per-file manifest `...CP0_pre_stage2_20261020_093806.manifest.sha256`; verification: zip test OK, every member re-hashed == manifest == live file (0 mismatches).
  * Tool: `stage2/tools/checkpoint.py`.
* The original binaries built on the previous machine (`build/*.exe`, g++ 14.2.0 MinGW) were copied to `stage2/artifacts/build_original_gcc14/` (hashes in `build_original_gcc14.sha256`) before the clean rebuild replaced `build/`.
* The previous LPQ project / backup (`D:\dsaproject\lpq*`) was not opened, listed or modified by stage 2. (`final_audit.py` check J only stats them; the stage-2 audit does not touch them.)

## 2. Environment drift found (important for the audit)
| | recorded in round 1/2 docs | this machine (stage 2) |
|---|---|---|
| CPU | Intel Core i5-1235U (2P+8E) | Intel Core i5-13500HX (6P+8E, 20 logical), HP Victus 16 |
| OS | Windows 11 build 26100 | Windows 11 build 26200 |
| Python | 3.9.13 (numpy/scipy/matplotlib) | 3.11.9 (stdlib only used) |
| Compiler | g++ 14.2.0, MinGW-w64, `C:\gcc\bin` | **absent**; available: MSYS2 `g++ (GCC) 15.3.0`, target `x86_64-pc-cygwin` |
| Sanitizers | WSL Ubuntu g++ 15.2.0 | WSL Ubuntu 26.04 present but **no compiler installed**; Cygwin g++ has no libasan/libubsan |
Consequences: (a) the round-1/2 timings were produced on different hardware/compiler and **cannot be reproduced bit-for-bit here**; their raw data were audited, not re-measured; (b) all stage-2 builds use the Cygwin-target g++ 15.3.0 (`-O2`); (c) AddressSanitizer / the UBSan runtime could **not** be re-run (see audit report for the substitute checks). No toolchain was installed or downloaded for this.

## 3. Stage-2 deliverables (all new; nothing pre-existing was edited except README.md and docs/FINAL_REPORT.md, which only received appended stage-2 addenda after the audit)
| path | what |
|---|---|
| `stage2/tools/checkpoint.py`, `run_redteam_variants.sh`, `run_hardened_original_tests.sh` | checkpoint tool; fuzz/hardened-build drivers |
| `stage2/audit/independent_audit.py` (+ `independent_audit_output.txt/.json`) | independent checker for rounds 1-2 (63 PASS, 0 FAIL, 3 DISCREPANCY) |
| `stage2/audit/audit_string_experiment.py` (+ `string_audit_output.txt`) | independent checker for the string experiment (19 PASS, 0 FAIL) |
| `stage2/redteam/redteam_fuzz.cpp` | fresh-seed red-team fuzz (0 failures in 3 builds) |
| `stage2/paper/` | paper verification note, arXiv v3 PDF (SHA-256 recorded), extracted text, rendered pages 15 and 29 |
| `stage2/string_exp/` | `bench_string.cpp`, runner, analyzer, freeze record |
| `stage2/results/` | `string_dev.csv`, `string_heldout.csv` + run logs + summaries |
| `stage2/logs/`, `stage2/artifacts/` | command outputs; original g++14 binaries; original/rerun `final_audit` outputs; hardened binaries |
| `docs/STAGE2_AUDIT_REPORT.md`, `STAGE2_REAL_COMPARATOR_PLAN.md`, `STAGE2_REAL_COMPARATOR_RESULTS.md`, `EVALUATION_PREP.md` | reports |

## 4. Chronology (IST)
09:38 checkpoint CP0 -> 09:39 original binaries run -> 09:41 clean rebuild + tests + demo -> sanitizer probe (unsupported) -> independent audit checker written/run (first run: 3 FAIL caused by checker bugs - CSV rounding tolerance, break-even table parsing, wrong A/A cell - fixed and re-run; the genuine discrepancies D1/D2 were kept) -> red-team fuzz (all builds pass) -> paper verification (publisher page, Crossref, OpenAlex, arXiv PDF) -> feasibility check of generic types (no source change needed) -> development runs of the string harness (16 cells) -> plan written -> freeze record 10:14:19 -> held-out run 10:14:43-10:20:32 (18 cells, 0 failures) -> single analysis -> string audit -> hardened original tests -> existing `final_audit.py` rerun -> reports.
Mistakes made during stage 2 (kept on record): the first background launch of the hardened fuzz ran in the wrong directory and produced nothing (relaunched); a shell-quoting problem corrupted one analyzer edit (repaired before any held-out analysis); my first draft of the results text contained a muddled bullet about `std::stable_sort` (corrected against the table before the final audit).

## 5. Code defects
None found in `src/` (no stage-2 change to any frozen source). Limitation recorded: no exception safety when a comparator throws.

## 6. Deviations from the stage-2 brief
* Sanitizer (ASan/UBSan) runtime runs were **not possible** (no libasan/libubsan, no WSL compiler; installing a toolchain was not done without your approval): substitute builds used (see audit report).
* No MinGW build was possible; the Cygwin-target g++ 15.3.0 was used for everything in stage 2.
* Article-level Scopus indexing could not be checked (login / bot challenge, not bypassed).

## 7. Held-out string experiment verdict (frozen rule)
SAG-C (primary): **GAIN WITHOUT SAFETY**; SAG (secondary, same rule): **SUCCESS**. Details `docs/STAGE2_REAL_COMPARATOR_RESULTS.md`.

## 8. Final state (see `stage2/FINAL_HASHES.txt` for the SHA-256 of every source, script, raw CSV, summary, report and audit output)
* Final pass (`stage2/tools/final_pass.sh`, output `stage2/logs/final_pass_summary.txt`): existing `final_audit.py` exit 0, "0 FAIL"; stage-2 independent audit exit 0 (63 PASS / 0 FAIL / 3 documented DISCREPANCY); string audit exit 0 (19 PASS / 0 FAIL); the four test binaries exit 0 with 1,502,829 / 2,100,389 / 265,609 / 760,686 checks, 0 failed; demo exit 0, "all outputs equal the reference stable sort: YES".
* Comparison with CP0 (by SHA-256): of the 148 files in CP0, 140 are byte-identical; the 8 that differ are the six regenerated `build/*.exe` and the two appended-to documents `README.md`, `docs/FINAL_REPORT.md`. No raw CSV, manifest, freeze record, plan, summary, figure, source or script from rounds 1-2 changed. Original `results/outputs/final_audit.txt` unchanged (sha `42eba6df...27157`; the audit script overwrites it, so it was backed up and restored).
* LPQ: `D:\dsaproject\lpq` and its backup were never opened or modified by stage 2; they do not exist on this machine (`D:\dsaproject` absent), so their integrity cannot be re-checked here.
* Final checkpoint CP1: see `stage2/CHECKPOINT_CP1.txt` (written after the checkpoint, so not inside it).
