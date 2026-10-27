# Streak-Adaptive Galloping for Natural Merge Sort - Final Project Report

Data Structures and Algorithm Analysis (postgraduate CSE-AIML). Central syllabus topic: Merge Sort.
This is the consolidated submission report. It summarises, and does not replace, the historical stage reports kept unchanged in `docs/` (`FINAL_REPORT.md` for Stages 1-2 with short Stage 2 and Stage 3 addenda, `STAGE2_*.md`, `STAGE3_*.md`). Every number below is traceable to a file named next to it; where a number comes from a historical report that was independently re-derived in a later audit, this is stated.

## 1. Abstract
Natural merge sorts such as TimSort and PowerSort detect existing sorted runs and merge them; inside each merge, a *galloping* routine finds the end of each streak of items that come from one run. The base paper fixes the galloping parameter `t` once per merge. This project re-implements that method in C++17 (independently, not from the authors' code) and evaluates a proposed extension, SAG (Streak-Adaptive Galloping), that adapts `t` after each long streak using the paper's own cost model, with no extra memory and no extra key comparisons.
Across three stages with experiment rules written before each held-out run, the results are conditional. SAG needs 15-27 percent fewer comparisons on low-entropy inputs. Fewer comparisons became less elapsed time only when a comparison was expensive: Stage 1 was a partial success, Stage 2 a failure of the SAG-C variant's safety criterion, and the final frozen Stage 3 confirmation of SAG met all its criteria (12 of 16 gain cells against 11 required; no severe regression in 50 cells). Most Stage 3 cells are synthetic variable-length strings on one machine and one toolchain. SAG is therefore a conditional improvement for expensive-key, low-entropy workloads, not a faster sort in general. A comparator-exception defect was found and fixed (basic guarantee).

## 2. Problem statement and motivation
Sorting is used everywhere, and real library sorts are natural merge sorts. Their running time is dominated by key comparisons when keys are expensive to compare (long strings, file paths, composite keys). The merge step decides where one run stops "winning" and the other starts; galloping lets it skip ahead on long streaks. The base paper's rule for `t` is safe against adversarial inputs but does not use what the merge has already observed about streak lengths. The project question: can `t` be adapted online, using the paper's cost model only, to reduce comparisons, and when does that translate into less time?

## 3. Mapping to the syllabus (Merge Sort)
The algorithm studied is Merge Sort in its natural, run-based form: run detection, a merge-order policy, a stable merge with a temporary buffer, comparison counting, `O(n log n)` behaviour. The project modifies and evaluates the merge step. A textbook top-down Merge Sort (`src/textbook_merge_sort.hpp`) is included as a reference and as a second correct oracle in tests. Details: `docs/SYLLABUS_COMPLIANCE.md`.

## 4. Base paper
E. Ghasemi, V. Jugé, G. Khalighinejad, H. Yazdanyar, "Galloping in Fast-Growth Natural Merge Sorts", *Algorithmica* 87(2), 242-291, Springer; published online 9 December 2024, issue year 2025; DOI 10.1007/s00453-024-01285-6. Title, authors, journal, volume, pages and DOI were compared across the publisher page, Crossref and OpenAlex (`stage2/paper/STAGE2_PAPER_VERIFICATION.md`, `docs/PAPER_VERIFICATION.md`). A publisher correction (DOI 10.1007/s00453-025-01297-w, 29 January 2025) removes an incorrect affiliation of one co-author; it does not alter the method. Preprint: arXiv:2012.03996 (v3 read); conference version: ICALP 2022, LIPIcs vol. 229, 68:1-68:19.
Content used: the t-galloping merge (McIlroy's mixed strategy), the cost model `cost_t(m) = m` for `m <= t+2`, else `t + 2 ceil(log2(m - t))`; the polylogarithmic rule `t = ceil(log2(a+b))^2` (Definition 47, checked against the rendered page), the TimSort stack rule (Algorithm 2, checked against the rendered page), and the claimed bounds (`O(n + n H*)` comparisons; never more than naive merging plus `O(n)`). The paper has no experiments and no code.
Indexing: the publisher's journal page lists Scopus among the journal's abstracting and indexing services. Scopus itself needs a login, so this was not verified there, and **individual-article Scopus indexing is not verified**.
This project is an independent re-implementation of the relevant method. It does not reproduce all proofs, other stack rules, the paper's adversarial constructions (Propositions 42-43) or the paper's Algorithm 6 (TimSort's `min_gallop` state machine).

## 5. Base algorithm
1. **Runs.** Scan the array into maximal non-decreasing runs or strictly decreasing runs (reversed, which keeps stability). Runs shorter than `minrun = 32` are extended with binary insertion sort.
2. **Merge order.** A stack rule picks which adjacent runs to merge: PowerSort (node power) or TimSort (paper Algorithm 2). Both are "fast-growth" policies from the paper.
3. **Merge.** If the runs are already in order (one comparison) nothing is moved. Otherwise the smaller run is copied to a buffer and merged forward or backward; ties go to the left run (stable).
4. **Galloping.** After one element of a streak is known, the rest of the streak is found by checking `t` items one by one, then positions `t+1, t+2, t+4, ...`, then a binary search. A streak of `m` items costs `cost_t(m)` comparisons.
5. **Parameter.** Base policy: `t = ceil(log2(a+b))^2`, fixed for the merge (`a`, `b` the run lengths).
Complexity: `O(n log n)` comparisons worst case; the paper gives `O(n + n H*)` for fast-growth sorts with this policy (`H*` = dual-run entropy); extra space `min(a, b)`. These bounds are the paper's; the project's tests check them on finite domains and do not prove them.

## 6. The SAG extension
**What changes.** Only the rule for `t` (`src/sag_policy.hpp`, configuration `SagTT<64, true>`). Everything else (runs, stack rule, buffer, merge, galloping routine) is the same code for every variant.
**How it works.** Each merge starts at the paper's `t` (also the upper limit). After a streak of at least 64 items, or a streak whose search wasted comparisons, SAG evaluates the paper's cost formula analytically for `t/2`, `t`, `2t+2` and `0`, and moves to the cheapest (ties keep the current value). Short streaks found by plain walking change nothing. `t` resets at the start of the next merge. No extra memory and no extra key comparisons are used.
**Intended effect.** On inputs with long streaks (few distinct values, skewed duplicates, nearly sorted data) the base policy still finds streaks shorter than about `t` one comparison at a time; SAG reaches the cheap exponential search sooner.
**Safety bound.** For any `t >= 0` each streak costs at most 4/3 of its length (checked exhaustively for `t <= 70`, tails `<= 160`; observed 1.3317 per merge in fuzz runs). The paper's stronger "naive plus `O(n)`" guarantee is not claimed for SAG.
**SAG-C** (`src/sag_time_policy.hpp`) is a time-aware corrective variant that measures the cost of one comparison at start and keeps `t = 0` while a "credit" lasts. It is a documented alternative, not the final candidate; its constants were never retuned after its failures.

## 7. Implementation structure
Header-only C++17 templates: `src/natural_merge_sort.hpp` (runs, stack rules, galloping, merge, counters, exception guard), `src/sag_policy.hpp`, `src/sag_time_policy.hpp`, `src/textbook_merge_sort.hpp`, `src/generators.hpp` and `src/entropy.hpp` (deterministic test data and entropy measures). Elements must be trivially copyable (blocks are moved with `memmove`); strings are handled through `{pointer, length, index}` records. Counters are a compile-time switch; timed builds contain none. Tests: `tests/test_sort.cpp` (two builds), `test_sag.cpp`, `test_sagt.cpp`, `test_exception.cpp`. Demonstration: `bench/demo_sort.cpp`. See `README.md` for the file map and commands.

## 8. Experimental design
Common to all stages: identical input for every variant of a cell; variant order randomised in every repetition; one warm-up per variant; paired per-repetition time ratio (variant divided by the base policy) with its median and a 95 percent percentile-bootstrap confidence interval (10,000 resamples, fixed seed); an A/A control (the base run twice) as the noise floor; exact comparison counts from a separate un-timed pass; every output compared with an independent stable-sort reference; plan, seeds, manifest digest and file hashes frozen before each held-out run; development data and seeds disjoint from held-out ones. A *low-entropy* cell is defined from the data as dual-run entropy `H* <= 0.5 log2 n`. A *severe regression* is a cell with time ratio at least 1.10 and confidence-interval lower bound above 1.0.

| | Stage 1 (rounds 1-2) | Stage 2 | Stage 3 |
| --- | --- | --- | --- |
| Question | does SAG save time, and when | real string comparator | does SAG's result generalise to varied string keys |
| Keys | 32-bit integers (plain, indirect, synthetic heavy comparator) | strings, one 240-character shared prefix | 48 synthetic variable-length string cells (3 key-length classes x 8 structures x 2 sizes) + 2 file-name cells |
| Cells | 90 + 54 | 18 | 50 |
| Machine | Intel i5-1235U | Intel i5-13500HX | Intel i5-13500HX |
| Compiler | g++ 14.2.0 (MinGW-w64) | g++ 15.3.0 (MSYS2, Cygwin target) | g++ 15.3.0 (MSYS2, Cygwin target) |
Stage 2 and 3 machine: HP Victus 16, Windows 11 build 26200, AC power, Balanced power plan, benchmark thread pinned to a performance-core logical CPU (checked 1000 of 1000 samples), `-O2 -DNDEBUG`. Stage 1 timings were produced on different hardware and were not re-run later; stages are never compared in absolute time.
**Stage 3 design (`docs/STAGE3_CONFIRMATION_PLAN.md`, frozen 23:23:13; the single held-out run 23:23:27-23:36:59).** Key styles: `S` short random words (about 10 characters), `M` hierarchical paths (about 55-65 characters), `L` long hierarchical paths (about 210-250 characters, variable shared prefixes). Structures: random permutation, few distinct values (2, 16, 256), zipf(1024), mixture, 8 concatenated runs, nearly sorted (1 percent swaps). Sizes 2^17 and 2^19; repetitions 15, 11 and 31 (file-name cells). Two real-string cells: 19,611 file names of a local MSYS2 installation (`natural` order and `shuffled`). Variants: naive merge, base, base (A/A), SAG (**primary candidate**), SAG-C (frozen settings, diagnostic only), base under the TimSort rule, `std::stable_sort`, textbook top-down merge sort.
**Frozen classes and rule.** An *expensive-key* cell has `cost_B >= 32`, where `cost_B` is the mean number of bytes a comparison of neighbouring sorted keys examines. The *gain set* is the low-entropy, expensive-key cells: 16 cells (styles `M` and `L`; few(2), few(16), few(256), zipf(1024); both sizes), fixed by an un-timed structural pass before the freeze. Confirmation success required all of: (P1) every run complete and verified correct and stable; (P2) time ratio at most 0.90 with confidence-interval upper bound below 1.0 in at least 11 of the 16 gain cells; (P3) no severe regression in any of the 50 cells; (P4) sorter-counted comparisons at most 1.03 times the base in every cell and the independent checker re-deriving counts, ratios, intervals, manifest and verdict.

## 9. Results
### 9.1 Stage 1 (historical; independently re-derived from the raw data in the Stage 2 audit)
* **Round 1** (72 held-out + 18 sweep cells; `results/raw/heldout.csv`, `docs/FINAL_REPORT.md`): SAG used 15-27 percent fewer comparisons on the 12 low-entropy cells (ratio 0.73-0.85; about 0.5 on nearly sorted data) and never more than the base. With a synthetic heavy comparator (about 22 ns) it saved 11-21 percent time on all 8 low-entropy cells (ratios 0.794-0.887), worst heavy-regime cell 1.022. With cheap comparisons it was slower in all 54 plain/indirect cells (ratios 1.005-1.240, 12 severe). Verdict **partial success, conditional on comparator cost**.
* **Round 2** (SAG-C, fresh seeds, 54 cells; `results/raw/heldout2.csv`): gain kept (8 of 8 heavy low-entropy cells, ratios 0.725-0.863) but 2 of 36 cheap cells regressed by 10 percent or more (1.103 and 1.120). Verdict **corrective partial**.
### 9.2 Stage 2 (real string comparator; `stage2/results/string_heldout.csv`, `docs/STAGE2_REAL_COMPARATOR_RESULTS.md`)
18 cells with a 240-character shared prefix. SAG-C (primary) met the gain criterion (6 of 8 low-entropy cells) but failed the guard: `runs(64)` at 2^17 had ratio 1.114 [1.104, 1.150]. Round-1 SAG, evaluated separately under the same rule, met it (6 of 8 cells, no severe regression). SAG-C's self-measured comparison cost was 3.7-9.1 times the cost observed inside the sort. Verdict **gain without safety** for SAG-C.
### 9.3 Exception safety (Stage 3)
See section 10.
### 9.4 Stage 3 confirmation (`stage3/results/s3_heldout.csv`, `stage3/results/summary_s3_heldout.md`, `docs/STAGE3_CONFIRMATION_RESULTS.md`)
Criteria outcome: P1 pass (5,888 of 5,888 rows `ok = 1`; 50 of 50 processes exit 0); P2 pass (**12 of 16** gain cells; 11 required); P3 pass (largest ratio over all 50 cells 1.076, below the 1.10 threshold); P4 pass (maximum SAG/base comparison ratio 1.000; independent internal checker 25 PASS, 0 FAIL, 0 DISCREPANCY; zero summary-number mismatches; the exact sign-interval confidence interval changes no cell's classification). A/A noise: largest deviation 0.021.

Gain-set cells (time ratio SAG/base with 95 percent confidence interval; ratio at most 0.90 with upper bound below 1 counts as a win):

| Cell (style_structure_log2 n) | comparisons SAG/base | time SAG/base [95% CI] | A/A [95% CI] | base ns per element | win |
| --- | --- | --- | --- | --- | --- |
| L_few16_n17 | 0.759 | 0.762 [0.751, 0.772] | 1.007 [0.996, 1.014] | 564.9 | yes |
| L_few16_n19 | 0.755 | 0.741 [0.713, 0.752] | 0.992 [0.985, 0.997] | 563.3 | yes |
| L_few256_n17 | 0.834 | 0.747 [0.734, 0.752] | 1.002 [0.987, 1.005] | 876.5 | yes |
| L_few256_n19 | 0.786 | 0.660 [0.645, 0.664] | 0.995 [0.983, 1.000] | 967.7 | yes |
| L_few2_n17 | 0.851 | 0.895 [0.888, 0.906] | 0.996 [0.990, 1.014] | 371.6 | yes |
| L_few2_n19 | 0.851 | 0.928 [0.911, 0.937] | 0.993 [0.981, 1.006] | 117.0 | no |
| L_zipf1024_n17 | 0.815 | 0.779 [0.774, 0.791] | 1.000 [0.987, 1.021] | 853.1 | yes |
| L_zipf1024_n19 | 0.773 | 0.689 [0.686, 0.696] | 1.004 [0.997, 1.015] | 906.2 | yes |
| M_few16_n17 | 0.758 | 0.873 [0.862, 0.897] | 1.004 [0.996, 1.014] | 223.3 | yes |
| M_few16_n19 | 0.755 | 0.873 [0.857, 0.891] | 1.014 [0.984, 1.062] | 250.1 | yes |
| M_few256_n17 | 0.834 | 0.885 [0.841, 0.929] | 1.005 [0.984, 1.036] | 366.5 | yes |
| M_few256_n19 | 0.786 | 0.827 [0.816, 0.859] | 0.999 [0.992, 1.018] | 364.0 | yes |
| M_few2_n17 | 0.849 | 0.916 [0.880, 0.956] | 1.007 [0.969, 1.044] | 152.5 | no |
| M_few2_n19 | 0.852 | 0.934 [0.926, 0.943] | 0.993 [0.986, 1.012] | 134.1 | no |
| M_zipf1024_n17 | 0.815 | 0.943 [0.923, 0.949] | 1.009 [0.989, 1.023] | 347.1 | no |
| M_zipf1024_n19 | 0.773 | 0.870 [0.859, 0.878] | 1.003 [0.992, 1.021] | 382.1 | yes |

Other cells (SAG/base time ratio; full table with intervals in `stage3/results/summary_s3_heldout.md`):

| Group | cells | time ratio range | comparison ratio range |
| --- | --- | --- | --- |
| cheap-key low-entropy (`S`: few(2), few(16), few(256), zipf(1024)) | 8 | 0.938 - 1.076 | 0.755 - 0.853 |
| nearly sorted (`L`, `M`, `S`) | 6 | `L` 0.602-0.626; `M` 0.788-0.799; `S` 0.832-0.838 | 0.517 - 0.541 |
| permutation, 8 runs, mixture (all key styles) | 18 | 0.871 - 1.044 | 0.891 - 1.000 |
| real file names, natural order | 1 | 0.957 [0.944, 0.966] | 0.845 |
| real file names, shuffled | 1 | 1.007 [1.005, 1.011] | 1.000 |

Reading: the gain follows the cost of one comparison. Wall time per comparison (base time per element divided by comparisons per element) was about 17-71 ns for `L`, 11-28 ns for `M` and 7-17 ns for `S`. Four gain cells were not 10 percent faster (L few(2) n19; M few(2) n17 and n19; M zipf(1024) n17); the passing margin is 12 against 11. Short `S` keys gain little and the two `zipf(1024)` `S` cells were 5.6 and 7.6 percent slower (below the severe threshold). Random permutations are neutral. The two file-name cells are small (19,611 strings; sorts of a few milliseconds) and show only a 4.3 percent gain (natural order) and no change (shuffled order); they do not demonstrate a practical benefit on real data.
**SAG-C diagnostic (not a verdict).** With its frozen settings SAG-C also had 12 of 16 gain-cell wins but a severe regression on the natural-order file-name cell (1.276 [1.266, 1.288]); it is not the final candidate.
**Conclusion of the frozen rule: conditional confirmation success for SAG.**

## 10. Exception-safety defect, correction and regression validation
**Defect.** If the comparator throws in the middle of a merge, part of the smaller run exists only in the temporary buffer. In the original header, 32,069 of 84,623 reproduced cases (comparator throws on every possible call, 4 configurations, 3 `minrun` values, 20 inputs) left the array not a permutation of its input (elements lost and duplicated); smallest case: 3 elements, TimSort order with naive merge.
**Fix.** A small RAII guard `GapGuard` in `src/natural_merge_sort.hpp` copies the not-yet-merged part of the buffered run back into the output gap during unwinding. For comparators declared `noexcept` the guard is an empty type and generates no code.
**Guarantee provided (basic).** If the comparator throws, the exception propagates and the array is a permutation of the input. Order and stability after the exception are not guaranteed and counters may be partial. This is **not** the strong guarantee.
**Validation.** After the fix: 0 non-permutations in the same 84,623 cases (`tests/test_exception.cpp`, 84,624 checks); the four older suites are unchanged (1,502,829; 2,100,389; 265,609; 760,686 checks, 0 failed; these suites overlap in coverage and must not be added); differential runs of the original and the patched header (216 runs) gave identical comparison counts, counters and output hashes; a micro-benchmark showed no systematic speed change, but the generated code for a `noexcept` comparator is not byte-identical (one routine is scheduled differently); hardened builds (checked containers, undefined-behaviour trap-on-error and integer-overflow trapping, which need no sanitizer runtime) of the same programs and of a red-team fuzz (986,580 sorts per run, 10,057,802 checks per run) passed. AddressSanitizer/UBSan runtimes were **not** available on the Stage 3 machine and were not run. Evidence: `docs/STAGE3_AUDIT_REPORT.md`, `docs/STAGE3_CHANGELOG.md`, `stage3/logs/`.

## 11. Limitations
Summarised here, listed in full in `docs/KNOWN_LIMITATIONS.md`: the result is conditional and the margin narrow (12 against 11); 48 of 50 confirmation cells are synthetic and the two real cells are small; one machine and one toolchain in Stage 3; comparison counts are not time (the evidence for time is the paired timing, and cheap keys show comparisons falling while time does not); Stage 1-2 timings were audited but not re-measured; all audits are internal, not external review; sanitizer runtimes were unavailable; article-level Scopus indexing is unverified; exception safety is the basic guarantee only.

## 12. Conclusion
The project delivers a tested, reproducible C++17 re-implementation of a natural merge sort with the paper's galloping policy, the SAG extension, and a three-stage evaluation with pre-registered rules. SAG reduces comparisons on low-entropy inputs and, in the frozen Stage 3 experiment, reduced time by at least 10 percent in 12 of 16 expensive-key, low-entropy cells without any severe regression in 50 cells. This confirms SAG as a **conditional** improvement for the tested expensive-key, low-entropy string workloads. It does not show that SAG is faster for every input, for short or cheap keys, on real-world data in general, or on other machines. SAG-C remains a documented, unsuccessful corrective alternative. The Stage 1 and Stage 2 conclusions (partial and corrective-partial; SAG-C failing its guard) are preserved unchanged.

## 13. Reproducibility
Build and tests: `bash scripts/run_all_tests.sh`. Stage 3 final gate: `bash stage3/tools/final_pass3.sh`. Final package checks and file hashes: `docs/SUBMISSION_HANDOVER.md`, `SUBMISSION_MANIFEST.sha256`, `validation/`. Frozen Stage 3 records: `stage3/string3/freeze_record.txt`.

## 14. References
1. E. Ghasemi, V. Jugé, G. Khalighinejad, H. Yazdanyar, "Galloping in Fast-Growth Natural Merge Sorts", Algorithmica 87(2):242-291, 2025 (online 9 Dec 2024), Springer. DOI 10.1007/s00453-024-01285-6.
2. Correction to reference 1, Algorithmica 87:812, 2025. DOI 10.1007/s00453-025-01297-w.
3. E. Ghasemi, V. Jugé, G. Khalighinejad, "Galloping in Fast-Growth Natural Merge Sorts", ICALP 2022, LIPIcs vol. 229, 68:1-68:19, DOI 10.4230/lipics.icalp.2022.68 (conference version listed by OpenAlex).
4. arXiv:2012.03996 (preprint of reference 1; v3 was read for this project).
Other sources named in the paper (McIlroy's mixed strategy, PowerSort, TimSort) were used only through reference 1 and the project's own implementation notes; they were not independently re-verified.
