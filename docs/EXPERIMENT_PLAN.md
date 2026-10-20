# EXPERIMENT_PLAN — frozen protocol (written BEFORE any held-out measurement)

Frozen on 2026-10-19 at about 23:40 IST (machine clock) after the base implementation passed Gate A and after the extension's two constants were chosen on development data. The SHA-256 of every source file and of the held-out manifest are recorded in `results/outputs/freeze_record.txt`; any later change to the manifest, the criteria, or the code under test must be reported as a deviation in `docs/FINAL_REPORT.md`.

## 1. Question and hypotheses
**Background.** The base paper's merge routine fixes the galloping parameter `t = ⌈log₂(a+b)⌉²` for a whole merge (proven safe, workload-oblivious). **Proposed extension (SAG, Streak-Adaptive Galloping)** adapts `t` within each merge from the lengths of the blocks it has just discovered (`src/sag_policy.hpp`, final config `SagTT<64, true>`; `docs/PROPOSED_METHOD.md`).

* **H1 (comparison count — the paper's cost model).** On inputs with low dual-run entropy, SAG performs ≥ 10 % fewer comparisons than the base policy.
* **H2 (time when comparisons dominate).** In the *heavy-comparator regime* (a comparison costs ≈ 22 ns of dependent arithmetic, see §3) the comparison saving becomes a ≥ 10 % wall-clock saving on low-entropy inputs.
* **H3 (no harm).** SAG does not cause severe regressions anywhere it is applied; it never uses materially more comparisons than the base.
* **H4 (correctness).** Every sorted output equals the reference (stable) sort.
* **Not claimed:** that SAG is faster with cheap comparisons (plain integers, cache-missing indices). Development data already showed it is *slower* there; those regimes are measured and reported, and they decide the verdict tier (§7), not the existence of a hypothesis.

## 2. Algorithms compared (all use the same merge core, buffer discipline, `minrun = 32`, PowerSort stack rule unless stated)
| name | what it is |
|---|---|
| `base` | **Base paper**: polylogarithmic galloping, `t = ⌈log₂(a+b)⌉²` per merge. **Reference for every ratio.** |
| `base_b` | identical code to `base`, run a second time per repetition → A/A noise control |
| `sag` | **Proposed extension** (`SagTT<64,true>`) |
| `naive` | same sorter with a plain two-pointer merge (no galloping) |
| `tim_base`, `tim_sag` | `base` / `sag` under TimSort's stack rule (paper Alg. 2): tests that the result does not depend on the merge-order policy |
| `stdstable` | `std::stable_sort` (libstdc++), context only |
| `topdown` | textbook top-down Merge Sort (syllabus form), context only |

## 3. Comparison regimes (an input is the same array in every regime)
| regime | element / comparator | meaning |
|---|---|---|
| `plain` | `uint32` keys, `a<b` | cheap comparisons (≈ 1 ns); moves comparable to comparisons |
| `indirect` | `uint32` indices compared via `keys[i]<keys[j]` (argsort) | comparisons are cache-missing loads, independent so the CPU overlaps them |
| `heavy` | `uint32` keys + a dependent multiply-add chain of **K = 40** steps (≈ 22 ns here) per comparison | **SYNTHETIC** stand-in for expensive comparators (long strings, collation, computed keys): exactly the condition under which the paper's comparison-only cost model is meaningful |

Break-even sweep (descriptive, not part of any criterion): K ∈ {0, 2, 5, 10, 20, 40} for `few(16)`, `few(256)`, `perm` at n = 2^19.

## 4. Workloads (synthetic; deterministic generators in `src/generators.hpp`)
Families and parameters, **held-out**: `perm`; `few(σ)` with σ ∈ {2, 16, 256, 4096} (uniform random values); `zipf(1024)`; `mixture(64)` (half `few(64)`, half random permutation); `runs(64)` (64 sorted runs of random lengths with random values); `nearly(0.01)` (sorted with 1 % random swaps). Sizes n ∈ {2^17, 2^20, 2^22} (heavy regime: 2^17 and 2^20 only). Seeds 9001 + 10·family + size index (`scripts/run_bench.py`, `HELD_SEED_BASE`).
**Development** (disjoint seeds 101…, sizes 2^16/2^19/2^21, different parameters: `few(3,32,512,2048)`, `zipf(256)`, `mixture(16)`, `runs(16)`, `nearly(0.05)`, `perm`, `sorted`).
**Low-entropy cell** (structural, from the data, not from timings): dual-run entropy `H* ≤ 0.5·log₂ n`. For the held-out set this is computed by the bench and written to the CSV (`Hstar`).

### What was tuned on development data (and nothing else)
The two constants of SAG — `MinStreak` ∈ {4,8,16,32,64,128} and `JumpToZero` ∈ {no, yes}. Round 1 (comparison counts only, `results/raw/dev_counts.csv`) preferred MinStreak 16 with jump. Round 2 (dev timing, `results/raw/dev_selection.csv`) showed that MinStreak 16 costs up to +29 % plain-regime time and +5 % heavy-regime time on structure-free inputs (`runs(16)`), so the selection rule was **revised during development** to: *maximise the heavy-regime time gain on low-entropy dev cells subject to ≤ +1 % comparisons on high-entropy cells, ≤ +3 % heavy-regime time and ≤ +10 % plain-regime time on structure-free cells.* Only MinStreak ≥ 64 qualified; 64 has the larger gain → **`SagTT<64,true>`**. The held-out data did not influence this.

## 5. Measurement protocol
* One process per cell (`bench/bench_sort.cpp`); all variants of a cell see the **identical** input, copied from a pristine array before every timed run (copy not timed); variant order is **randomised in every repetition** (seeded); 1 warm-up round per variant.
* Repetitions: 15 (11 for n = 2^22; heavy regime 9 at n = 2^20, 15 at 2^17). Time = `std::chrono::steady_clock` around the sort call only.
* Every output is compared with a reference (`std::stable_sort`) → column `ok`; a mismatch is recorded, never discarded.
* A separate un-timed counting pass (counters compiled in; timed builds have none) records exact comparisons, merges, blocks and moves per variant.
* Build: `g++ 14.2.0`, `-std=c++17 -O2 -DNDEBUG -Wall -Wextra`, no `-march=native`. Sanitizer builds are separate and never timed.
* Environment: Windows 11 (build 26100), Intel Core i5-1235U (2 P-cores + 8 E-cores), on AC power, Balanced power plan (not changed), one thread pinned to the P-core logical CPU 0 (the harness verifies 1000/1000 samples on that CPU and records it), high process priority. Turbo/thermal behaviour cannot be controlled from user space and is a stated limitation. No other heavy process is started during a run.
* Raw rows go to `results/raw/heldout.csv` unedited; the analysis reads only that file.

## 6. Metrics and statistics
Primary: **median wall-clock ns/element** and **exact comparison counts**. Secondary: moves, merges, blocks, extra memory (`max_buffer`, identical by construction), A/A ratio.
For each cell: `time ratio = median over repetitions of time(SAG)/time(base)` measured in the same repetition (paired); 95 % percentile-bootstrap CI over repetitions (10,000 resamples, seed 12345); `comparison ratio = comparisons(SAG)/comparisons(base)` (deterministic). `scripts/analyze.py` produces the tables.

## 7. Success criteria and verdict rules (frozen)
* **H1 passes** if SAG's comparison ratio ≤ 0.90 in ≥ 2/3 of the distinct low-entropy (family, parameter, size) cells.
* **H2 passes** if, among low-entropy cells of the `heavy` regime, the time ratio ≤ 0.90 **and** its CI upper bound < 1.0 in ≥ 2/3 of the cells.
* **H3 (guard)** — a *severe regression* is a cell with time ratio ≥ 1.10 and CI lower bound > 1.0; a *comparison excess* is a comparison ratio > 1.03. H3 is evaluated separately for the heavy regime and for the cheap regimes (`plain`, `indirect`).
* **H4 passes** if no verification failed.
* **Secondary (the generic criterion from the project brief):** ≥ 10 % median improvement with CI excluding 1 on a majority of *all* timed cells (all regimes). Reported, not required for PARTIAL SUCCESS.
* **Verdict tiers**
  * **SUCCESS** — H1, H2, H4 pass, H3 passes in *all* regimes, and the secondary criterion passes.
  * **PARTIAL SUCCESS (conditional on comparator cost)** — H1, H2, H4 pass and H3 passes in the heavy regime, but H3 (or the secondary criterion) fails in the cheap regimes. Reading: SAG helps only when comparisons dominate and hurts otherwise.
  * **NO-GO** — H1 or H2 or H4 fails, or H3 fails in the heavy regime.
* **Conditions that make the contribution a no-go regardless of aggregate numbers:** a verification failure; a severe regression in the heavy regime; no gain in the heavy regime.
* These rules are not changed after seeing held-out results. If the verdict is NO-GO or PARTIAL, one corrective experiment is allowed **only** with a justified code change and **fresh seeds** (`HELD_SEED_BASE` + 100000); original held-out results are kept and reported side by side.

## 8. Known threats to validity (stated in advance)
Synthetic data only; the heavy regime is a model of expensive comparisons, not a measurement of a real comparator; one machine with a hybrid CPU and uncontrolled turbo; one compiler; the base is my re-implementation (not the authors' code, which is theoretical); block moves use `memmove`/small loops that favour galloping less than a tuned library would; `minrun = 32` insertion-sort time is included in every number and dilutes merge-phase differences.

## 9. Pre-run record
The manifest digest (SHA-256 of the canonical JSON of the 90 cells printed by `python scripts/run_bench.py --manifest heldout --dry`) is **93710ed639dae8963ce0d96319a53b78c1fd512374597395677661e6f7bf72f0**. File hashes: `results/outputs/freeze_record.txt`.

---

# ADDENDUM - corrective round (frozen BEFORE the round-2 held-out run)

Written 2026-10-20 at about 00:20 IST (freeze record: 00:23), after the round-1 held-out result (PARTIAL SUCCESS, section 7) and before any round-2 held-out measurement. Section 7 allows exactly one corrective experiment with a justified code change and fresh seeds; this is it. **The round-1 results and verdict are unchanged and remain the primary result; round 2 is reported next to them.**

## A1. Why a correction is justified
Round 1 showed that SAG helps when a comparison is expensive but is slower than the base in the cheap regimes (12 of 54 cells with a severe regression, worst +24 %). Diagnosis from the development data: SAG decides from comparison COUNTS; after a jump to t = 0 each short block still takes the exponential-search path, which wastes time (unpredictable branches) without wasting comparisons, so SAG never notices.

## A2. What was built (new file src/sag_time_policy.hpp; base and round-1 SAG files are untouched)
Three attempts, all kept on record:
1. **SAG-T v1** (time-model candidate evaluation after every long block). Failed on development data: up to +60 % time in the cheap regimes because the adaptation call itself costs more than a block (results/raw/dev_sagt_v1_timemodel_NEGATIVE.csv).
2. **SAG-B** (memoryless two-state rule with a break-even block length). Fixed the cheap regimes but lost the nearly-sorted gain (heavy time ratio 0.91 vs 0.69 for round-1 SAG) because one short block resets t (results/raw/dev_sagt_v2_breakeven.csv).
3. **SAG-C (final)**: credit rule. A block of length n changes a credit by c*n - (c+m)*probes0(n) (the time a pure exponential search saves over a linear scan; negative for short blocks); credit is clamped to [0, cap]; t = 0 while credit > 0, else t_max. Fast path: credit = 0 and n < B (one compare), where B is the break-even block length B(c, m). No loops, no calls in the hot path.
   * c = cost of one comparison, **measured by the sorter itself** at the start of each sort (16,384 comparator calls on pseudo-random pairs of the input, included in its timings); arrays shorter than 4096 elements use the base policy.
   * m = extra cost of an unpredictable non-linear probe: **16 ns**, an architecture constant fixed on development data; cap = 50 * m.
   * Worst case unchanged: any t >= 0 costs <= (4/3) x block length in comparisons (tests: tests/test_sagt.cpp).

## A3. Development selection rule (applied to the candidates in results/raw/dev_sagt.csv, dev seeds only)
Maximise the mean heavy-regime time gain on low-entropy dev cells subject to <= +5 % time versus the base on EVERY dev cell of the plain and indirect regimes and on the structure-free heavy cells (perm, runs). Only **sagc_m16_c50** (m = 16 ns, cap = 50 m) qualified (worst plain +3.8 %, worst indirect +4.4 %, heavy perm +4.4 %); it is the variant named `sagt` in the benchmark.

## A4. Fairness
The extension receives no information from outside: the comparison cost is measured by the sorter on the input itself. The base policy needs no such information. The calibration cost is part of the timed region.

## A5. Round-2 held-out experiment (manifest `heldout2`)
54 cells = the same 9 families (section 4) x sizes 2^17 and 2^20 x regimes plain / indirect / heavy (K = 40); **fresh seeds 109001 + 10*family + size index**, disjoint from round 1 (9001...) and development (101...). Variants: naive, base, base_b (A/A control), sag (round-1 SAG, for comparison), **sagt** (SAG-C). Repetitions: 15 (heavy regime at 2^20: 9). Same protocol as section 5. Held-out manifest digest: see results/outputs/freeze_record_round2.txt.

## A6. Round-2 criteria and verdicts (frozen)
Variant under test: **sagt** versus **base**; low-entropy cell as in section 4 (H* <= 0.5 log2 n).
* **R2-A gain retained:** in heavy-regime low-entropy cells, time ratio <= 0.90 with CI upper bound < 1.0 in >= 2/3 of the cells.
* **R2-B no regret:** NO cell of ANY regime with a severe regression (time ratio >= 1.10 and CI lower bound > 1.0).
* **R2-C correctness:** every output equals the reference.
* **R2-D comparisons:** comparison ratio never above 1.03 (counted in a separate un-timed pass; the calibration makes the count slightly non-deterministic).
* **Verdict:** CORRECTIVE SUCCESS (no regret + gain when comparisons dominate) if R2-A..D pass; CORRECTIVE PARTIAL if R2-A, C, D pass but R2-B fails; CORRECTIVE NO-GO otherwise. The generic "10 % faster on a majority of ALL cells" criterion is reported but cannot hold by construction (cheap regimes give no gain) and is not required.
* No further correction rounds will be run; if round 2 fails it is reported as a failure.

## A7. Extra threats to validity
The calibration depends on the CPU clock state at the moment of measurement (unpinned test runs showed a 2x spread between runs); m is machine specific; the credit rule's constants (m, cap) were tuned on 7 development families at one size.
