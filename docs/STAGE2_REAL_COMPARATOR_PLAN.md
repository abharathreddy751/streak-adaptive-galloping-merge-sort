# STAGE2_REAL_COMPARATOR_PLAN - frozen protocol (written BEFORE the held-out run)

Status: **FROZEN** at the time recorded in `stage2/string_exp/freeze_record.txt` (written after this file; the file hashes, compiler,
manifest digest and the SHA-256 of this plan are in that record). Round-1 and round-2 results, code and verdicts are **not** touched.
This is an *additional* validation experiment; it cannot overwrite the earlier verdicts (PARTIAL SUCCESS; CORRECTIVE PARTIAL).

## 1. Question and hypothesis
Rounds 1-2 used a *synthetic* expensive comparator (a dependent multiply-add chain, `heavy`). Does the effect survive a **genuine** expensive comparison?

* **H-S (primary).** When keys are real strings with long common prefixes (comparison = lexicographic `memcmp` over ~248 bytes behind two pointer
  dereferences), the extension reduces wall-clock time by >= 10 % relative to the base paper's policy on low-entropy inputs, with no severe regression elsewhere.
* Not hypothesised: any gain on structure-free inputs; any claim about real-world data (all strings are **synthetic**).
* Context from development data (see section 11): a string comparison here costs about 20-37 ns of wall time per comparison and is dominated by cache misses, i.e.
  it is in the regime where rounds 1-2 saw gains.

## 2. Algorithms compared (identical merge core, buffer discipline, `minrun = 32`, PowerSort stack rule unless stated)
| name | what it is |
|---|---|
| `base` | **base paper**: polylogarithmic galloping `t = ceil(log2(a+b))^2`. Reference for every ratio. |
| `base_b` | identical code to `base`, run a second time per repetition: A/A noise control |
| `sag` | proposed extension SAG, round-1 configuration `SagTT<64,true>` (frozen code, unchanged) |
| `sagc` | **PRIMARY extension under test**: SAG-C credit policy, round-2 final configuration `sag_credit_sort(m = 16 ns, cap = 50 m)` (frozen code, **not retuned**); calibration (16,384 comparator calls on pseudo-random pairs) happens inside the timed sort call |
| `naive` | same sorter with a plain two-pointer merge |
| `tim_base` | `base` under TimSort's stack rule (paper Algorithm 2) |
| `stdstable` | `std::stable_sort` (libstdc++), reference |
| `topdown` | textbook top-down Merge Sort (syllabus form), reference |
Why `sagc` is primary: it is the project's final corrective extension and was designed for exactly this situation (self-measured comparison cost). `sag` is evaluated
under the **same rule with its own verdict**; the two verdicts are never pooled and no "best of two" is reported as the result.

## 3. Genericity (feasibility check, done before any code was written for the experiment)
`natural_merge_sort<...>` and the SAG policies are templates over the element type `T` (requirement: `std::is_trivially_copyable_v<T>`, because blocks are moved with `memmove`)
and over an arbitrary comparator. A 16-byte record `{const char* p; uint32 len; uint32 idx}` is trivially copyable and its comparator performs a true string comparison, so the frozen
sources are used **unchanged** (hashes in the freeze record). No genericity change was needed. `std::string` elements directly would **not** compile (not trivially copyable) and are not used.

## 4. Data (synthetic string datasets - not real-world data)
Key array `v` from the frozen generators `src/generators.hpp` (`perm`, `few(sigma)`, `zipf`, `mixture`, `runs`, `nearly`); string for element i =
`PREFIX(240 identical pseudo-random lower-case letters, same for the whole cell) + 8 lower-case hex digits of v[i]` (hex order == numeric order, so the structure is exactly that of `v`).
Every element owns its own string in one contiguous arena (generation order); sorting permutes only the records, so comparisons read scattered memory (arena = 32 MB at 2^17, 128 MB at 2^19).
`idx` = original position -> equal keys are distinguishable and stability is verified. Datasets and generation parameters are identical for all variants of a cell.
| family | meaning | parameter(s) |
|---|---|---|
| `perm` | random permutation (structure-free control) | - |
| `few(sigma)` | low-cardinality: repeated long-prefix strings | sigma in {2, 16, 256, 4096} |
| `zipf(1024)` | skewed low-cardinality | s = 1 |
| `mixture(64)` | half `few(64)`, half random | - |
| `runs(64)` | 64 sorted runs of random strings | - |
| `nearly(0.01)` | sorted strings with 1 % random swaps (controlled perturbation) | 0.01 |
**Held-out cells: 9 families x sizes {2^17, 2^19} = 18 cells**, seeds `7100000 + 10*family_index + size_index`. **Development**: seeds `6000000+`, different parameters
(`few(3,32,512)`, `zipf(256)`, `mixture(16)`, `runs(16)`, `nearly(0.05)`, `perm`) and sizes {2^16, 2^18}; disjoint from held-out and from rounds 1-2.
**Low-entropy cell** = dual-run entropy `H* <= 0.5 log2 n` (structural; computed from `v`). Computed from the generators before any timing, the held-out low cells are exactly:
`few(2)`, `few(16)`, `few(256)`, `zipf(1024)` at both sizes (**8 cells**, H* = 1.00, 4.00, 8.00, 7.50 vs thresholds 8.5 / 9.5); the other 10 cells are high-H* **control cells**.

## 5. Measurement protocol
* One `bench_string.exe` process per cell; all variants on the identical input (pristine copy, copy not timed); variant order **re-randomised in every repetition** (seeded); 1 warm-up round per variant.
* Repetitions: 15 at 2^17, 11 at 2^19. Time = `std::chrono::steady_clock` around the sort call only (calibration of `sagc` is inside the call; the sorters' internal buffer allocation is inside every call).
* Un-timed counting pass with counters compiled in (timed builds contain no counters): exact comparisons per variant; for `sagc` the 16,384 calibration comparisons are recorded in a separate column.
* Build: `g++ (GCC) 15.3.0` (MSYS2, target x86_64-pc-cygwin), `-std=c++17 -O2 -DNDEBUG -Wall -Wextra -Isrc`, no `-march=native`.
* Environment: Windows 11 build 26200, Intel Core i5-13500HX (6P+8E), HP Victus 16, on AC power, Balanced power plan (unchanged), thread pinned to a P-core logical CPU (efficiency class 1, CPU >= 2; harness verifies 1000/1000 samples), high process priority, no other heavy process started during the run (including no concurrent fuzz/compile jobs).
* RAM: harness refuses to start a cell if free RAM < 1,200 MB (laptop has 15.7 GB total, ~2.6 GB free at the start); peak use per cell is below 400 MB.
* No sleeps, no volatile busy-loops, no arithmetic padding: the cost is the real comparator.
* Raw rows are appended unedited to `stage2/results/string_heldout.csv`; failed/non-zero cells are logged in `string_heldout_runlog.txt`, never dropped; missing cells are reported.

## 6. Metrics and statistics (identical to rounds 1-2)
Per cell: time ratio = median over repetitions of `time(variant)/time(base)` of the same repetition (paired); 95 % percentile bootstrap CI over repetitions (10,000 resamples, `random.Random(12345)`, medians sorted, indices 250 and 9749). Comparison ratio = comparisons(variant)/comparisons(base) (deterministic for `sag`; `sagc` has a calibration-dependent count). Software: `stage2/string_exp/analyze_string.py` (hash frozen).

## 7. Correctness requirements (P1)
Every timed and counting run must (a) reproduce the `std::stable_sort` reference **exactly** (same record sequence, hence same `idx` order, i.e. stable) and (b) pass an independent scan (non-decreasing, and equivalent neighbours have ascending `idx`). A single failure or a missing (cell, variant, repetition) = P1 fails.

## 8. Decision rule (frozen; applied to `sagc` as PRIMARY and, separately, to `sag`)
* **P1** correctness: all outputs verified, all 18 cells x 8 variants x all repetitions present.
* **P2** gain: **time ratio <= 0.90 AND CI upper bound < 1.0** in at least ceil(2/3 x 8) = **6 of the 8** low-H* cells.
* **P3** guard (severe regression): **no cell at all (low or control)** with time ratio >= 1.10 AND CI lower bound > 1.0.
* **P4** comparisons counted by the sorter never above 1.03 x `base` in any cell (as R2-D; calibration calls reported separately).
* **Verdict:** **SUCCESS** = P1 and P2 and P3 and P4. **GAIN WITHOUT SAFETY** = P1 and P2 hold but P3 or P4 fails. **NO-GO (hypothesis not supported)** = P1 or P2 fails.
* The A/A control (`base_b/base`) is reported: its spread is the noise floor against which a 10 % effect must be read.
* **No-go conditions regardless of aggregates:** a verification failure; fewer than 6 of 8 low cells with a >= 10 % gain.
* No threshold, family, size, rep count, variant, or success criterion may change after the held-out run starts. No repeated held-out runs or parameter searches against held-out cells. If the result is not SUCCESS, it is reported as such; a different idea needs its own plan and fresh seeds.

## 9. What is explicitly not done
No retuning of `m`, cap, MinStreak or any constant (development data shows `sagc`'s calibration reads 50-156 ns per comparison, 3-5 x the wall time per comparison in the sort; this is a property of the frozen design and is reported, not repaired). No new algorithm. No claim about real-world datasets.

## 10. Threats to validity (stated in advance)
Synthetic strings (single shared prefix; fixed length); one machine, one compiler (Cygwin-target g++ 15.3.0, different from the g++ 14.2 MinGW used in rounds 1-2 and a different CPU: results are not comparable to rounds 1-2 in absolute terms); hybrid CPU with uncontrolled turbo; `sagc`'s calibration pays cache-cold comparisons, which inflates its cost for small/fast sorts; `memcmp` is vectorised; A/A noise up to ~8 % was seen in earlier rounds, so a 10 % target is close to the noise floor for small cells.

## 11. Development observations used to check the harness (not used to change any criterion)
`stage2/results/string_dev.csv` / `summary_string_dev.md` (16 cells, dev seeds): the harness verifies every output (0 failures), sorts take about 6 ms - 165 ms, wall time per comparison 20-37 ns, A/A spread up to 0.075.
On development data `sagc` and `sag` both showed >= 10 % gains in most low-H* cells and `sagc` a severe regression on `runs(16)` (1.37 / 1.13) where the small sort time makes its 16,384-call calibration expensive. This is a *predicted risk* for the held-out controls, recorded here before the held-out run.

## 12. Freeze record
`stage2/string_exp/freeze_record.txt`: SHA-256 of `bench_string.cpp`, `bench_string.exe`, `run_string_bench.py`, `analyze_string.py`, the frozen `src/*.hpp`, this plan; compiler version; held-out manifest digest (`python -I stage2/string_exp/run_string_bench.py --manifest heldout --dry`).
