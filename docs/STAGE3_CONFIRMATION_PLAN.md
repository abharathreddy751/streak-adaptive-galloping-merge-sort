# STAGE3_CONFIRMATION_PLAN - one fresh confirmatory experiment (frozen BEFORE the held-out run)

Status: **FROZEN** at the time recorded in `stage3/string3/freeze_record.txt` (written after this file; it contains the SHA-256 of this plan, of the benchmark source/binary, runner, analyzer, all library sources, the dataset file and the held-out manifest digest).
Nothing from rounds 1-2 or Stage 2 is touched; Stage 3 results live in `stage3/` and are never pooled with earlier stages (different data, different generators, and the Stage-3 numbers were taken in a separate session).

## 1. Question
Stage 2 found that SAG (round-1 policy `SagTT<64,true>`) met the declared secondary rule on string keys that all share one 240-character prefix, while SAG-C failed the guard. **Does SAG's result generalise to more varied string-key workloads** - variable string lengths, variable shared-prefix lengths, different comparison costs, skewed and low-cardinality distributions, nearly sorted data, structure-free controls, and a small set of real strings?

## 2. Candidate and comparators (all parameters frozen; no tuning on held-out results)
| name | what | role |
|---|---|---|
| `base` | paper's polylogarithmic galloping, PowerSort order | reference for every ratio |
| `base_b` | identical code to `base` | A/A noise control |
| **`sag`** | **`SagTT<64,true>` (original SAG)** | **PRIMARY candidate; the only variant that can produce the Stage-3 verdict** |
| `sagc` | SAG-C, frozen round-2 settings (m = 16 ns, cap = 50 m), **not retuned** | secondary diagnostic only, no verdict |
| `naive` | plain two-pointer merge in the same sorter | context |
| `tim_base` | `base` under the TimSort stack rule | context |
| `stdstable` | `std::stable_sort` (libstdc++) | reference |
| `topdown` | textbook top-down merge sort | reference |
The sorter source used is the **Stage-3 header** `src/natural_merge_sort.hpp` (an exception guard was added, see `docs/STAGE3_CHANGELOG.md`; for noexcept comparators it generates no code, differential tests show identical comparison counts and outputs, and a micro-benchmark shows no systematic speed change). The benchmark comparator is declared `noexcept`. SAG policy header unchanged since round 1 (hash recorded).

## 3. Verification of the string representation (done before this plan, evidence in `stage3/string3/` and `docs/STAGE3_AUDIT_REPORT.md`)
Element = 16-byte `{const char* p; u32 len; u32 idx}` (trivially copyable, `static_assert`), every element owns a private NUL-terminated copy of its string in a packed arena. Comparator = `std::string_view::compare` (bytewise lexicographic; shorter string first on equal prefixes), strict weak order, `noexcept` (`static_assert(is_nothrow_invocable)`). The benchmark checks at start that every record points at exactly `len` bytes followed by a NUL with no embedded NUL. Verification of each sorted output: (a) equal to the `std::stable_sort` reference as a record sequence (pointer, length and original index), which includes stability, and (b) an independent scan (non-decreasing; equivalent neighbours have ascending original index).

## 4. Data (all **synthetic** except the two FILE cells, which are labelled with their source)
Structure comes from the frozen integer generators (`src/generators.hpp`): the integer value of every element is replaced by the string of the same rank in a sorted pool of distinct random strings (order-isomorphic), so the structure of the string array is exactly that of the integer array.
Key styles (pool generator in `bench_string3.cpp`, seeded):
| style | strings | typical length | comparison cost |
|---|---|---|---|
| `S` | random words from [a-z0-9], 4-16 chars, no shared structure | ~10 | cheap |
| `M` | hierarchical paths `/w1/w2/.../leaf`, 2-4 components from small random vocabularies, component lengths 3-30, leaf 6-28 | ~55-65 | medium; shared prefixes vary from 0 to ~60 chars |
| `L` | hierarchical paths with long components (first 60-140, second 30-90, ...), leaf 10-40 | ~210-250 | high; shared prefixes of 40-250 chars, variable |
Structures: `perm`; `few(2)`, `few(16)`, `few(256)` (low cardinality); `zipf(1024)` (skewed); `mixture(64)` (half few-valued, half random); `runs(8)` (concatenation of 8 sorted runs); `nearly(0.01)` (sorted with 1 % random swaps).
**Cells:** 3 styles x 8 structures x sizes {2^17, 2^19} = **48 synthetic cells**, plus **2 real-string cells** (`FILE`): the 19,611 file names (relative paths, e.g. `usr/bin/addr2line.exe`; names only, no file contents) of the MSYS2 installation on this machine, saved as `stage3/data/msys2_paths.txt` (SHA-256 in the freeze record). The names are of files from free-software packages; the list is derived data, used here only as test strings; it is not redistributed beyond this project. `natural` = directory-walk order with sorted names, `shuffled` = random permutation of the same strings. **Total 50 cells.**
Seeds: held-out `7300000 + 100*style_index + 10*structure_index + size_index` (FILE: `7300900 + k`); development `6500000 + ...` with different parameters (`few(4)`, `few(64)`, `zipf(256)`, `runs(16)`, `nearly(0.05)`, sizes 2^16/2^18); seeds, parameters and sizes are disjoint from every earlier stage. Manifest: `python -I stage3/string3/run_string3.py --manifest heldout --dry` (digest in the freeze record).

## 5. Structural classes (defined from the generated data, before any timing)
* **low-entropy cell:** dual-run entropy of the rank array `H* <= 0.5 log2 n` (same definition as every earlier stage).
* **expensive-key cell:** `cost_B >= 32`, where `cost_B` = mean over adjacent pairs of the sorted array of `min(LCP + 1, shorter length)` = the number of bytes a comparison of neighbouring keys examines (equal keys: their full length). The threshold 32 bytes (half a cache line pair) was fixed during design, before any structural or timing pass.
* **gain set G = low-entropy AND expensive-key.** A deterministic **un-timed structural pass** (`--manifest structure_heldout`, no sorting variant run; output `stage3/results/s3_structure_heldout.csv`) was run before this plan was frozen. It gives exactly: **G = the 16 cells of styles `M` and `L` with `few(2)`, `few(16)`, `few(256)`, `zipf(1024)` at both sizes.** The low-entropy cells outside G (diagnostic, still covered by the guard): the 8 `S`-style cells (`cost_B` 9-12 bytes: cheap keys) and `FILE natural` (`H* = 4.86`, `cost_B = 30.8`, just under 32: **cheap by the pre-declared rule**). `runs(8)` is high-entropy (`H*` about 0.9 log2 n) and is a control. All other cells are high-entropy controls.

## 6. Measurement protocol
* One `bench_string3.exe` process per cell; all variants on the identical input (pristine copy per run, copy not timed); variant order re-randomised in every repetition (seeded); 1 warm-up per variant; `std::chrono::steady_clock` around the sort call only (SAG-C's calibration is inside its call).
* Repetitions: **15** (2^17), **11** (2^19), **31** (FILE cells). Un-timed counting pass with counters compiled in (timed builds contain none): exact comparisons per variant; for `sagc` the 16,384 calibration calls are recorded separately.
* Build: `g++ (GCC) 15.3.0` (MSYS2, target x86_64-pc-cygwin), `-std=c++17 -O2 -DNDEBUG -Wall -Wextra -Isrc`, no `-march=native`.
* Machine: Windows 11, Intel Core i5-13500HX (6P+8E), HP Victus 16, AC power, Balanced plan (unchanged); the thread is pinned to a P-core logical CPU (efficiency class 1, CPU >= 2) with 1000/1000 verification and high process priority; nothing else heavy runs during the run. RAM gate: a cell is not started with < 1,200 MB free.
* Raw rows are appended unedited to `stage3/results/s3_heldout.csv`; every command, exit status and free-RAM reading goes to `s3_heldout_runlog.txt`; failed or missing cells are reported, never dropped. Results are **not** compared with Stage 1-2 numbers as if measured under the same conditions.

## 7. Statistics
Per cell: time ratio = median over repetitions of `time(variant)/time(base)` from the same repetition; 95 % percentile bootstrap CI (10,000 resamples, `random.Random(12345)`, sorted medians, indices 250 and 9749). Comparison ratio = comparisons(variant)/comparisons(base) from the counting pass. Software: `stage3/string3/analyze_string3.py` (hash frozen).

## 8. Decision rule (frozen) - applies to the primary candidate `sag` only
A **Stage-3 confirmation success** requires ALL of:
1. **P1 correctness/completeness:** all 50 cells x 8 variants x all repetitions present; every output matches the independent stable reference including stability; every process exit status 0.
2. **P2 gain:** in at least **ceil(2/3 x |G|) = 11 of the 16** gain-set cells: time ratio <= 0.90 **and** CI upper bound < 1.0.
3. **P3 guard:** **no cell of the 50** (gain set, cheap-key, controls, real strings) with time ratio >= 1.10 **and** CI lower bound > 1.0 (severe regression; same as Stage 2).
4. **P4 counts and manifests:** SAG comparisons counted by the sorter never above 1.03 x `base` in any cell, **and** the independent Stage-3 checker (`stage3/audit/audit_stage3.py`) re-derives the counts, ratios, CIs and verdict from the raw rows, finds the manifest digest, run log and source hashes identical to the freeze record, and reports no unexplained discrepancy.
Any failed criterion = **confirmation not met**. The A/A control is reported as the noise floor. SAG-C and all reference variants are descriptive. No threshold, cell, size, repetition count or variant changes after the held-out run starts; **the held-out set is run once**; no tuning against it. If the result is not a success it is reported as such and the base implementation remains the safe default.

## 9. What a success / failure would mean (decided now)
Success: SAG finalised as a **conditional** improvement for the tested expensive-key string workloads (not universally faster; cheap keys and structure-free data not claimed). Failure: generalisation not demonstrated; keep the base implementation; report the most defensible remaining result (the Stage-2 single-prefix result stands only for that workload). No new algorithm is introduced in Stage 3.

## 10. Threats to validity (stated in advance)
Synthetic strings except two small real-string cells (n = 19,611, tiny sorts of about 2-4 ms, noisier); one machine and compiler; hybrid CPU with uncontrolled turbo; `memcmp` is vectorised; the key-cost threshold (32 bytes) and the cell list are design choices fixed before timing; the cheap-key (`S`) cells are expected, from round 1, to be unfavourable for SAG and are deliberately included in the guard.

## 11. Development observations (harness check only; development seeds)
`stage3/results/s3_dev.csv`, `summary_s3_dev.md` (36 cells: 3 styles x 6 structures x sizes 2^16/2^18, dev seeds 6500000+, 7 repetitions): the harness works (2,278 records, all verified `ok = 1`, every process exit 0), sorts take 5-400 ms, wall time per comparison 9-95 ns. The structural classes behave as designed: `S` keys have `cost_B` 3.5-12.7 (cheap), `M` 33-66, `L` 170-250. Applying the frozen rule to the development data (for information only; no criterion, threshold, cell or constant was changed because of it) gives 11 of 12 gain-set cells with a >= 10 % gain for SAG, no severe regression, A/A noise up to 0.057. The cheap-key `S` cells show small or no gain (ratios 0.92-1.09, one 1.087 [1.020, 1.301] on `zipf(256)` at 2^16), which is why they are deliberately part of the guard in the held-out set. Development data are not reused for any held-out claim.
