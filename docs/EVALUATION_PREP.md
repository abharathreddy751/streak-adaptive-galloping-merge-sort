# EVALUATION_PREP - MACSE501 project "Streak-Adaptive Galloping for Natural Merge Sort"

(Plain-English preparation for the viva. Every number below comes from files in this project; nothing is claimed that the evidence does not support.)

## 1. The problem and why it matters
Merge Sort splits an array in halves, sorts them and merges. Real data is rarely random: it often contains stretches that are already sorted (**runs**) or has few distinct values. *Natural* merge sorts (TimSort - used in Python, Java, Android, V8 - and PowerSort) find those runs and merge them smartly. The expensive part of each merge is **finding where the output switches from one run to the other**. If a comparison is expensive (long strings, database keys), the number of comparisons dominates the running time, so we want to find those switch points with as few comparisons as possible.

## 2. The base paper (what it contributes)
Ghasemi, Jugé, Khalighinejad, Yazdanyar, "Galloping in Fast-Growth Natural Merge Sorts", *Algorithmica* 87(2):242-291 (online 9 Dec 2024; issue 2025), DOI 10.1007/s00453-024-01285-6 (a correction notice, DOI 10.1007/s00453-025-01297-w, fixes one author's affiliation only).
* TimSort uses **galloping**: to find the end of a streak ("block") of elements that come from the same run it probes `t` positions one by one, then jumps 1, 2, 4, 8... positions and finishes with a binary search.
* The paper proves TimSort's way of choosing `t` can be tricked: with 3 distinct values it needs Theta(n log n) comparisons although linear is possible. It introduces a **polylogarithmic rule `t = ceil(log2(a+b))^2`** (a, b = lengths of the two runs) and proves it never costs more than plain merging plus O(n) comparisons, and sorts arrays with sigma distinct values in O(n + n log sigma) comparisons (via "dual-run entropy" H*).
* The paper is theoretical: **it contains no experiments**. The implementation here is *my independent implementation of the paper's method*, not the authors' code.

## 3. The base algorithm, step by step (`src/natural_merge_sort.hpp`)
1. **Run detection**: cut the array into maximal non-decreasing or strictly decreasing runs (decreasing runs are reversed; strictness keeps the sort stable). Runs shorter than `minrun = 32` are extended by binary insertion sort.
2. **Merge order** (stack rule): PowerSort (node power of the boundary) or TimSort's rule (paper Algorithm 2). Both keep merge trees balanced enough to give O(n + nH) moves.
3. **Merge**: copy the *smaller* run to a buffer, merge forward or backward, discovering each block by galloping with `t = ceil(log2(a+b))^2` fixed for the whole merge.
Complexity: O(n log n) comparisons worst case; O(n + nH*) with the polylogarithmic rule; extra space min(a,b) <= n/2; stable.

## 4. The proposed modification and its exact difference from the baseline
* **Baseline:** `t` is fixed per merge at `ceil(log2(a+b))^2` (hundreds for big merges), so every streak shorter than about `t` is discovered one comparison per element, even when all streaks are long and `t = 0` (pure exponential search, 2*ceil(log2 m) comparisons for a block of m) would be far cheaper.
* **SAG (Streak-Adaptive Galloping, `src/sag_policy.hpp`):** after each block, evaluate the paper's own cost model for t/2, t, 2t+2 (and 0) and keep the cheapest; `t` resets to the paper's value at the start of every merge; blocks shorter than 64 found linearly teach nothing (fast path). No extra memory, no extra key comparisons. Worst case per block <= 4/3 of its length (the paper's guarantee "naive + O(n)" is stronger; **I do not claim a better worst case**).
* **SAG-C (`src/sag_time_policy.hpp`):** a time-aware version: measures the cost `c` of one comparison at the start of the sort (16,384 comparator calls) and keeps `t = 0` while a "credit" earned by long blocks lasts (credit = time saved by exponential search minus its overhead; capped, so about 50 short blocks cancel it).
Everything else (run detection, stack rule, merge, buffer) is identical code for all variants.

## 5. Pseudocode of the central operation (merge with galloping)
```
merge(A, B):                                   # adjacent sorted runs; ties keep A first (stable)
    if B[0] >= A[last]: return                 # already in order (1 comparison)
    buffer := the smaller run; merge forward if A is smaller else backward
    t := policy.start(a + b)                   # base: ceil(log2(a+b))^2   SAG/SAG-C: same, adapted below
    owner := A if A[0] <= B[0] else B          # 1 comparison
    loop:
        # the first element of the block is already known to belong to `owner`
        n := 1 + gallop_count(rest of owner's elements, t, "element belongs after the other run's head")
        move n elements of owner to the output (memmove for long blocks)
        policy.observe(n, comparisons spent, elements left)      # <- ONLY place where SAG / SAG-C differ
        owner := the other run
        stop when a run is exhausted; flush the remainder

gallop_count(seq, t, pred):                    # pred is false...false true...true; returns # of false
    for x in 1..t: if pred(seq[x]) return x-1  # linear probes
    step := 1; probe seq[t+1], seq[t+2], seq[t+4], ... until pred true   # exponential
    binary search inside the last interval
```
Cost of a block of m elements: `m` if `m <= t+2`, else `t + 2*ceil(log2(m-t))` comparisons.

## 6. How it was tested and how to run the demonstration
* **Correctness tests** (4 programs; **1,502,829 / 2,100,389 / 265,609 / 760,686 checks, 0 failed**): exhaustive small cases (all sorted-pair sequences, all arrays of length <= 9 over 3 values, all permutations of length <= 7), 2,445 random arrays from 15 families, the galloping routine against the paper's Table 1 and its cost formula, per-merge cost bounds, edge cases. Stage 2 added an independent **red-team fuzz** with fresh seeds (986,580 sorts over 108 configurations incl. stability tags, canary guard zones, inconsistent and throwing comparators): **0 failures** at -O2, at -O3 -march=native and in a checked/UBSan-trap build. These are tests, **not proofs**.
* **Demonstration** (from a Git-Bash shell in the project root; needs `g++` with C++17; the Cygwin-target `g++` of MSYS2 needs `C:\msys64\usr\bin` on PATH):
```bash
export PATH="/c/msys64/usr/bin:$PATH"; export CXX=g++      # only if g++ is not already on PATH
bash scripts/build.sh && ./build/test_base.exe && ./build/test_all.exe && ./build/test_sag.exe && ./build/test_sagt.exe
./build/demo_sort.exe      # prints comparisons per element for naive / paper (base) / SAG / std::stable_sort / textbook merge sort,
                           # ends with "all outputs equal the reference stable sort: YES"
```
Expected demo line for 3 distinct values (comparisons per element): naive 14.5, paper's policy 8.3, SAG 6.8.

## 7. Results (all synthetic data; verdicts fixed before each held-out run)
| experiment | cells | verdict |
|---|---|---|
| Round 1 (integer keys, 3 comparator regimes incl. a *synthetic* heavy comparator) | 72 + 18 sweep | **PARTIAL SUCCESS**: 15-27 % fewer comparisons on low-entropy inputs; 11-21 % less time when comparisons are expensive; SAG **slower** than the base in all 54 cheap-comparison cells (+0.5-24 %) |
| Round 2 (SAG-C, fresh seeds) | 54 | **CORRECTIVE PARTIAL**: gain kept/increased (heavy regime), but 2 of 36 cheap cells still regress >= 10 % |
| **Stage 2 (real string comparator, 240-char shared prefixes, `memcmp`)** | 18 | **SAG-C (primary): GAIN WITHOUT SAFETY** - 6 of 8 low-entropy cells >= 10 % faster (13-37 %), but `runs(64)` at 2^17 is **+11.4 %** (guard failed). **SAG (secondary): SUCCESS** under the same rule (6 of 8 cells; worst cell +2.7 %). |
| **Stage 3 (confirmation, SAG primary; variable-length keys, 3 key-cost classes, 8 structures, 2 real-string cells)** | 50 | **CONFIRMATION SUCCESS for SAG** under the frozen rule: 12 of 16 expensive-key low-entropy cells >= 10 % faster (threshold 11), no severe regression in any of 50 cells, 0 correctness failures; independently re-derived. Conditional: cheap 10-character keys gain little or lose (up to +7.6 % on `zipf(1024)`), 4 expensive-key cells stayed under 10 %. SAG-C (diagnostic) regressed +27.6 % on the real-string cell. |
* The honest summary: *when comparisons are expensive (here real strings), adapting `t` saves 13-37 % of the time on few-valued/skewed inputs and about 25-34 % on nearly sorted inputs; with cheap comparisons it does not pay; the self-calibrating variant SAG-C is not safe on string data because its calibration over-reads the comparison cost 4-9x.* Stage 3 supports the same conclusion for a wider set of string workloads: SAG is a **conditional** improvement - it pays when a comparison examines tens to hundreds of bytes (here 17-70 ns per comparison), not for short keys. It is **not** a general performance win.

## 8. Honest limitations
Synthetic data only (never describe it as production or real-world data); one laptop, one compiler per stage (rounds 1-2: i5-1235U, g++ 14.2 MinGW; stage 2: i5-13500HX, g++ 15.3 Cygwin-target, so absolute timings differ); round-1/2 results could not be re-run here (previous toolchain absent) - their raw data were re-analysed independently, not re-measured; A/A noise reached 8.5 % in some cheap cells, so a 10 % threshold is close to the noise floor there; margins in the string experiment are thin (6 of 8 cells = the threshold); proofs of the paper and its adversarial constructions are **not** reproduced; paper read in its arXiv v3 form (journal version paywalled); article-level Scopus indexing **not verified**; exception contract is only the *basic* guarantee (Stage 3: if a comparator throws, no element is lost or duplicated, but order is unspecified; for `noexcept` comparators the guard is compiled out); `T` must be trivially copyable (strings are handled through `{pointer, length, index}` records).

## 9. Likely viva questions (with accurate answers)
1. **Why fewer comparisons but sometimes *more* time?** A comparison count ignores what each step costs. Galloping with `t = 0` replaces predictable linear scans by exponential/binary search whose branches are data-dependent and poorly predicted; for 1-4 ns integer compares the branch mispredictions and extra bookkeeping cost more than the comparisons saved (round 1: +0.5 to +24 % in all 54 cheap cells). When one comparison costs 20-46 ns (strings, cache misses) the same saving dominates and time falls 13-37 %.
2. **What exactly did you implement from the paper?** Natural merge sort with PowerSort and TimSort stack rules, McIlroy's t-galloping merge and the polylogarithmic `t`, all from the paper's text; the TimSort rule and the `t` formula were checked against the printed Algorithm 2 and Definition 47. Not implemented: the paper's Algorithm 6 (TimSort's real `min_gallop` machine), other stack rules, adversarial arrays, proofs.
3. **Is it the authors' code?** No. It is an independent implementation; the authors' paper has no code or experiments.
4. **Is your extension novel?** Not claimed. TimSort already adapts `min_gallop` during a merge; SAG differs by using the paper's exact cost model and resetting per merge. No literature search for priority was done.
5. **Is the sort stable?** Yes by construction (strict comparisons for descending runs, ties go to the left run) and verified directly with tagged records (equal keys keep their original order) in all test programs and in the fuzz.
6. **Worst case?** Base: O(n log n); with polylog galloping at most naive-merge cost + O(n) (paper). SAG/SAG-C: at most 4/3 of the block length per block (checked exhaustively for t <= 70, tails <= 160, observed 1.3317 on real merges). I do not claim the paper's stronger "+O(n)" for SAG.
7. **Why a held-out set and a frozen plan?** To stop me tuning thresholds to the results; criteria, seeds, code hashes and the manifest digest were frozen before each run. Where a rule failed (e.g. the guard) it is reported as failed.
8. **Why does SAG-C fail on strings?** It measures the comparison cost on random pairs, which are cache misses (106-236 ns), but inside the sort comparisons cost 20-46 ns; so it adopts `t = 0` too eagerly and pays a fixed calibration cost. Evidence: calibration vs in-sort cost 3.7-9.1x; regression on `runs(64)` (+11 %, +9 %). The exact mechanism split (calibration cost vs eager `t = 0`) was not isolated experimentally.
9. **Why is `nearly(0.01)` not a 'low-entropy' cell although SAG helps most there?** The frozen rule defines low-entropy as H* <= 0.5 log2 n; `nearly(0.01)` has H* about 0.63-0.69 log2 n, so it is a control cell. Its gains are reported but did not count towards the gain criterion.
10. **How do you know your audit is independent?** The stage-2 checker never imports the project's analysis or audit scripts; it recomputes every table entry (432 + 324 printed numbers, the 18-row sweep table, the report's heavy-regime table and its other numeric claims) from the raw CSVs with its own bootstrap and compares them; the design digest is rebuilt from the plan text. It agrees with the stored results except for one 0.001 rounding slip in the report (1.015 vs 1.016) and a documented plan-file edit (see audit report).
11. **What happens if the comparator throws?** Originally elements could be lost or duplicated (reproduced: 32,069 of 84,623 throw-at-every-call cases). Stage 3 added a guard that copies the unmerged part of the temporary buffer back while the exception unwinds: the array stays a permutation of the input (basic guarantee, regression test `tests/test_exception.cpp`), but sortedness is not promised and it is not the strong guarantee. For `noexcept` comparators the guard generates no code; counts and outputs were identical to the original and no systematic speed change was measured (generated code is not byte-identical).
12. **Why is SAG, not SAG-C, the final candidate?** SAG-C failed its guard in Stage 2 and showed a severe regression on a small real-string cell in Stage 3 (calibration cost); SAG has no calibration and passed the frozen Stage-3 rule. Neither is claimed to be better than the base in general.
13. **What are the Scopus facts?** The publisher's journal page lists SCOPUS (and SCIE); article-level indexing could not be checked (Scopus/Scimago blocked automated access and were not bypassed).

## 10. Implemented / not implemented / claims to avoid
| implemented | not implemented |
|---|---|
| natural merge sort (PowerSort, TimSort rules), galloping merge, polylog `t`; SAG, SAG-C; basic exception guarantee; tests; benchmark harness; string experiments (Stage 2 and Stage 3) | Algorithm 6 (`min_gallop` state machine), other stack rules, paper's proofs and adversarial inputs, strong exception guarantee, non-trivially-copyable element types |
**Claims to avoid:** "faster than TimSort / std::sort in general"; "reproduces the paper"; "real-world data"; "novel / first"; "article is Scopus-indexed"; "all tests prove correctness"; "SAG-C is safe"; "SAG is faster for all string keys" (cheap keys are not covered); "strong exception safety"; any statement that stage 2 changed the round-1/2 verdicts.
