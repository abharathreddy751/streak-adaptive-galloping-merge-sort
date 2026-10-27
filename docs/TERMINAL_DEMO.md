# Terminal demonstration guide (v4)

`build/nms_tool` is a command-line program for showing and measuring the algorithms of this project in a terminal. It has no GUI, writes no files and uses no network. It adds **no new sorting algorithm**: it runs the existing textbook Merge Sort, the naive-merge natural sort, the paper's base policy, SAG and `std::stable_sort` and reports what they did. Source: `bench/nms_tool.cpp` (command line), `src/diag.hpp` (measurement helpers), `src/natural_merge_sort.hpp` (the optional trace hook).

## 1. Build and run
```bash
cd "<project folder>"
bash scripts/build.sh            # builds everything, including build/nms_tool(.exe)
./build/nms_tool.exe help        # on Linux/macOS: ./build/nms_tool help
```
With the MSYS2 compiler on Windows, put `/c/msys64/usr/bin` on `PATH` first (`export PATH="/c/msys64/usr/bin:$PATH"`), otherwise the program cannot find its runtime DLL.

## 2. Commands
| Command | What it does |
| --- | --- |
| `profile` | measured description of an input (section 4) |
| `trace` | every event of one sort of a small input (n <= 128): runs, merges, blocks, SAG adaptations (section 5) |
| `compare` | the five algorithms on the same input: exact counters, correctness and stability, optional timing (section 6) |
| `strings` | `compare` with 64-byte string records, a stability check and timing (defaults: `--key str --family few --param 16 --n 200000 --reps 5`) |
| `help` | usage |

Input options (all commands): `--keys 5,6,7` (explicit integers) **or** `--family F --n N --param P --seed S`; `--key int|str`; `--keylen L` (string key length 8-63, default 48); `--minrun M` (default 32, trace default 4); `--policy base|sag` (trace only, default sag); `--reps R` (compare/strings: also time every variant). Families: `perm few zipf runs mixture nearly sorted reversed pipe`. Bad input gives a one-line message on stderr and exit code 2. Exit code 1 means a correctness or counter check failed; 0 means everything checked passed.

## 3. Commands for a demonstration (all were run on this package)
```bash
./build/nms_tool.exe trace                                   # the 24-element array of demo_sort, SAG policy
./build/nms_tool.exe trace --keys 3,1,2,9,8,7,4,5,6 --policy base --minrun 2
./build/nms_tool.exe profile --family few --param 3 --n 262144
./build/nms_tool.exe compare --family few --param 3 --n 262144
./build/nms_tool.exe compare --family nearly --param 0.01 --n 100000 --reps 5
./build/nms_tool.exe strings                                 # 200,000 records with 64-byte string keys, timed
./build/nms_tool.exe compare --key str --keylen 63 --family zipf --param 1024 --n 100000
```
Saved outputs of the deterministic commands (no timing lines) are in `results/outputs/v4_*.txt`; `bash scripts/v4_demo_outputs.sh` regenerates them.

## 4. `profile`: what is measured
The profiler first prints the **generator metadata** (what was asked for) and then **measurements** taken from the data itself. Nothing is concluded from a generator's name.

| Line | Definition |
| --- | --- |
| distinct keys | number of different keys (by the key order) |
| adjacent descents | pairs with `a[i+1] < a[i]` |
| natural runs | the sorter's own greedy decomposition: maximal non-decreasing or strictly decreasing blocks, left to right |
| run length min/mean/max | over those runs |
| H, H* | run entropy and dual-run entropy in bits (`src/entropy.hpp`, the paper's measures) |

Labels use fixed, printed definitions: **already sorted** = no descent; **reverse sorted** = no strict ascent (non-increasing); **nearly sorted** = not sorted and descents <= 1 percent of the n-1 pairs; **duplicate-heavy** = distinct keys <= 10 percent of n; **low-entropy** = H* <= 0.5 log2 n (the definition used by the preregistered gain set). A label is only a statement about these numbers.

## 5. `trace`: event legend
Every line is one event emitted by the sorter while it ran (the hook is described in `docs/BASE_PAPER_IMPLEMENTATION.md`); nothing is added by the printer except wording.

| Line | Event |
| --- | --- |
| `[run k] elements a..b: run detection found L` | a natural run of length `L` was found; if it was shorter than `minrun` it is extended by binary insertion (the line says so) and `a..b` is the final range |
| `[merge] at s: left run x + right run y ...` | a merge starts; which run is copied to the buffer (the smaller one); `t` = the galloping parameter at the start |
| `[merge skipped] ...` | the two runs were already in order; one comparison was spent, nothing moved |
| `block of m element(s) from the left/right run ...` | galloping found a streak of `m` consecutive output elements from one run, how many comparisons that discovery cost, and `t` used |
| `SAG adapted: t a -> b after a block of m` | SAG changed `t` (only for `--policy sag`) |
| `[merge done] ... c comparisons, k block(s)` | comparisons of this merge = 1 (already in order?) + 1 (who owns the first block?) + the block discoveries |
| `[verify]` | the output is identical (keys and ids) to the reference stable order **and** passes an independent sorted-and-stable test **and** the sorter's own comparison counter equals an external counter |

Notes. The streak at the end where a merge finishes (the last one in a forward merge, the first one in a backward merge) is the part that is already in place or is flushed; it is **not searched**, so it is not reported as a block and the printed blocks do not add up to the merge size. For the same reason the last printed block of a merge has no `SAG adapted` line. In a backward merge (right run buffered) the roles are mirrored internally; the printout always says left/right in array terms.

Example (real output of `trace`, SAG policy; the blocks are short, so SAG does not change `t` here and equals the base):
```
[run 1] elements 0..4: run detection found 5
[run 2] elements 5..17: run detection found 13
[run 3] elements 18..23: run detection found 6
[merge] at 0: left run 5 + right run 13 | left run copied to the buffer | t = 25
    block of 10 element(s) from the right run | found with t = 25 | 10 comparison(s) | 13 left in that run
    block of 5 element(s) from the left run | found with t = 25 | 4 comparison(s) | 5 left in that run
[merge done] at 0: 16 comparisons, 2 block(s)
```
(16 = 1 + 1 + 10 + 4.)

## 6. `compare` and `strings`: counters
All five algorithms sort an identical copy of the same records (`key`, original `id`).

| Column | Meaning |
| --- | --- |
| comparisons | key comparisons counted by a wrapper around the comparator (independent of the sorter); for the natural sorts this equals the sorter's own `run + merge` counters (checked; a mismatch prints `COUNTER MISMATCH` and fails) |
| cmp/elem | comparisons divided by n |
| moves | the sorter's own exact `element_moves` counter (elements copied into/out of the buffer, block copies, run reversals and insertion shifts); `n/a` for the textbook sort and `std::stable_sort`, which have no such counter |
| merges / skipped | merges performed / merges skipped because the runs were already in order |
| blocks | streaks found by galloping (0 for `naive`, which does not gallop) |
| correct | output byte-identical to `std::stable_sort` of the input |
| stable | independent test: sorted by key and equal keys in original order |

Below the table, `detail` lines give, for base and SAG: run counts, run-detection versus merge comparisons, block length min/mean/max and a histogram (`1`, `2-3`, `4-15`, `16-63`, `>=64`), and for SAG the number of `t` changes and the number of merges in which SAG adapted. `SAG / base comparisons` is an exact ratio of counts.

**Timing (`--reps R`).** Uncounted builds (no counters) are timed on fresh copies of the same input: one untimed warm-up, then `R` timed runs; the table shows median, minimum and maximum in milliseconds. The ratio of medians is descriptive for this one machine, one input and `R` runs. It is **not** a significance test, it is **not** the preregistered analysis, and it must not be mixed with the frozen results in `stage3/results/`. Fewer comparisons do not by themselves mean less time: with cheap integer keys SAG usually saves comparisons and no time; the benefit seen in the frozen experiments appears when a comparison is expensive (long string keys), which `strings` imitates with 64-byte keys that share a prefix. The 64-byte records are synthetic (`k...k` prefix plus 8 hex digits of a generated integer), not real-world strings.

## 7. Limits of the tool
* Counts and traces are produced by instrumented (`Count = true`) sorters; timings use the uncounted ones. The instrumented and uncounted sorters have the same logic; `validation/v4/asm_check` shows that the uncounted sorters compile to identical assembly with the v3 and v4 headers, and `tests/test_diag.cpp` shows that attaching a trace changes neither output nor counters.
* The trace is limited to n <= 128 on purpose; use `compare` for large inputs.
* Items must be trivially copyable (blocks are moved with `memmove`), hence fixed-size records for strings.
* Results from this tool are demonstrations on generated data, not evidence of general performance. The evidence is in `docs/STAGE3_CONFIRMATION_RESULTS.md`.
