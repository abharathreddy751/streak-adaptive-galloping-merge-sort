# Known limitations

These limits belong to the result. Read them together with `docs/FINAL_PROJECT_REPORT.md`.

## 1. The improvement is conditional, not universal
SAG is a conditional improvement. In Stage 3 it reached the preregistered gain only in the gain set: low-entropy inputs whose key comparisons examine at least 32 bytes on average (long variable-length strings). For short synthetic keys (about 10 characters) the time ratios were 0.938-1.076, i.e. little gain and, on `zipf(1024)`, 5.6 and 7.6 percent slower. Stage 1 showed the same pattern with integer keys: SAG was slower than the base in all 54 cheap-comparison cells. No claim is made that SAG is faster for every input, every key type or every machine.

## 2. Narrow margin against the preregistered criterion
The criterion needed 11 of 16 gain cells; SAG reached 12. Four gain cells did not reach a 10 percent gain (L few(2) n19, M few(2) n17, M few(2) n19, M zipf(1024) n17). The result is a pass of a frozen rule, not a large safety margin.

## 3. Mostly synthetic data; the real-string sample is small
48 of the 50 Stage 3 cells are generated variable-length strings with an invented structure. The only real strings are two lists of the same 19,611 file names from one local software installation (natural order and shuffled); each sort takes a few milliseconds, so those cells are noisy and show only a 4.3 percent gain (natural order) and no change (shuffled). They do not demonstrate a practical benefit on real-world string workloads.

## 4. One machine and one toolchain in Stage 3
Stage 3 ran on one laptop (Intel i5-13500HX, hybrid cores, Windows 11, AC power, Balanced power plan, thread pinned to a performance core) with one compiler (GCC 15.3.0, MSYS2, Cygwin target, `-O2`). Turbo behaviour and thermal state could not be controlled. Other CPUs, compilers (for example a MinGW or Linux build), memory systems and standard libraries may behave differently. Stage 3 timings are not comparable in absolute terms with Stage 1.

## 5. Comparison counts are not execution time
SAG always used fewer or equal comparisons than the base (Stage 3 comparison ratio at most 1.000), but elapsed time fell only when a comparison was expensive. With cheap keys the saved comparisons were outweighed by less predictable branches and bookkeeping. Counts are reported as a mechanism; time conclusions rest only on the paired timing measurements.

## 6. Stage 1 and Stage 2 timing-audit limits
The Stage 1 timings were measured on a different computer and compiler (Intel i5-1235U, g++ 14.2.0 MinGW) that were not available later. The later audits re-analysed the saved raw data and reproduced every table number, but did not re-measure the timings. Stage 1/2 results are therefore verified as data, not reproduced as experiments. One round-2 report number was rounded wrongly (1.015, data give 1.016) and one plan file was edited 11 seconds after the round-2 run started (a typo fix, documented; the pre-edit text was not kept, so "no criterion changed" is the log's statement and cannot be proven by hash).

## 7. The audits are internal
The Stage 2 and Stage 3 checkers were written by the same team and tooling that produced the code and results. They use separate code and recompute results from raw data, but they are internal audits, not an external peer or independent third-party review.

## 8. Sanitizer runtimes were not available
AddressSanitizer and UndefinedBehaviorSanitizer runtimes could not be run on the Stage 2/3 machine (no `libasan`/`libubsan`; the Linux subsystem had no compiler installed, and nothing was installed). Substitutes that were actually run: builds with checked standard containers and assertions, undefined-behaviour trap-on-error and integer-overflow trapping, guard-zone (canary) checks in the fuzz test, and exception tests. These are weaker than the sanitizer runtimes for memory errors. The Stage 1 sanitizer logs came from an earlier machine and were not repeated.

## 9. Paper and indexing
The code is an independent re-implementation of the relevant method, not the authors' code. The paper's proofs, other stack rules, adversarial constructions and its Algorithm 6 (TimSort's `min_gallop` state machine) are not reproduced. Only the arXiv v3 preprint text was read; the journal version is behind a paywall. The publisher's journal page lists Scopus, but Scopus itself requires a login, so **individual-article Scopus indexing is unverified**.

## 10. Exception safety is the basic guarantee
If a comparator throws, the array stays a permutation of its input, but its order and stability are not guaranteed and counters may be partial. The strong guarantee (original order restored) is not provided. The regression check covers throws at every comparator call for 4 configurations and 20 inputs, not every possible input. For comparators declared `noexcept` the guard is compiled out.

## 11. Further limitations found in the code and evidence
* **Worst-case bound.** SAG's per-streak bound is 4/3 of the streak length (checked on a finite range, not proven); the paper's stronger "naive plus O(n)" guarantee is not claimed for SAG.
* **No recovery rule for cost-neutral short streaks.** After a jump to `t = 0`, SAG returns to a larger `t` only when a streak wastes comparisons; streaks that cost exactly their length leave `t` unchanged until the merge ends. This is a design property, measured by the fuzz test.
* **SAG-C** (not the candidate) measures comparison cost on random pairs, which on strings was 3.7-9.1 times the cost inside the sort, and its 16,384 calibration comparisons are a fixed cost that matters for small sorts. It failed its safety criteria in Stage 2 and had a severe regression (ratio 1.276) on the natural-order file-name cell in Stage 3. Its constants were not retuned.
* **Element type.** Items must be trivially copyable (blocks are moved with `memmove`); strings are handled through `{pointer, length, index}` records.
* **Generated code.** For `noexcept` comparators the exception-guard change gives identical comparison counts and outputs and no systematic speed change in a micro-benchmark, but the compiled code for the plain merge routine is not byte-identical.
* **Old audit script.** `scripts/final_audit.py` reports one expected FAIL on the final source (check G, the frozen-header hash), because the exception guard changed `src/natural_merge_sort.hpp` on purpose; the original header is kept in `stage3/artifacts/orig_src`.
* **Tested platform.** Build and tests were run on Windows 11 with GCC 15.3.0 (MSYS2, Cygwin target). Linux, MinGW and other compilers are expected to work but were not tested for this package. The build script quirk: with `CXX` set to a non-existent path inside MSYS bash the build still used the default compiler (cause not investigated); with a compiler command that fails, the scripts correctly report failure (`validation/negative_test_failing_compiler.txt`).

## 12. v4 additions (terminal diagnostics)
* **Demonstration, not evidence.** `nms_tool` timings (`--reps`) come from one machine, one input and a few runs; they are descriptive and are not the preregistered analysis. The scientific conclusions rest only on the frozen experiments.
* **Instrumented versus timed sorters.** Counters and traces come from `Count = true` sorters; timings from uncounted ones. They have the same logic (tests cross-check outputs and counters; assembly of the uncounted sorters is identical to v3 at `-O2` for the instantiations in `validation/v4/asm_check/asm_instances.cpp`), but "identical assembly" was shown for those instantiations and `g++ -O2` only, not for every possible type or compiler.
* **Header hash.** `src/natural_merge_sort.hpp` changed again (trace hook only), so the Stage 3 independent audit reports one FAIL for its frozen-header hash on the live tree; it passes completely (25 PASS) on a copy with the v3 header. This is expected and checked by the gate.
* **Trace scope.** The trace is limited to n <= 128; the last block of each merge has no `t` change event because no discovery follows it; records must be trivially copyable, so strings are 64-byte fixed records in the tool.
* **Profile labels** (`nearly sorted`, `duplicate-heavy`, `low-entropy`) follow fixed thresholds printed with the output (1 percent of pairs, 10 percent distinct, H* <= 0.5 log2 n); other thresholds would label some inputs differently.
* **Not verified by v4:** sanitizers, Linux, MinGW, other compilers, real-world string workloads beyond the Stage 3 file-name lists.
