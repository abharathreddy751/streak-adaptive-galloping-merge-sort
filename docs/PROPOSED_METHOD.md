# PROPOSED_METHOD — Streak-Adaptive Galloping (SAG)

Code: [src/sag_policy.hpp](../src/sag_policy.hpp) (a single policy struct; the base sorter in `src/natural_merge_sort.hpp` is unchanged and unaware of it). Syllabus topic: **Merge Sort** (Module 3) — the proposal changes the *merge step*.

## 1. What the base paper does
Stable natural merge sorts merge adjacent runs by finding *blocks* (streaks of consecutive output elements coming from the same run). Each block is found by `t`-galloping: `t` one-by-one probes, then exponential + binary search. A block of length m costs `cost_t(m) = m` if `m ≤ t+2`, else `t + 2⌈log₂(m−t)⌉`. The paper's safe choice is `t = ⌈log₂(a+b)⌉²`, fixed for the whole merge; it proves comparison bounds of O(n + nH*) (H* = dual-run entropy) and "never more than O(n) above naive merging".

## 2. The remaining limitation
`t` is chosen from the merge size alone. At the top merge levels it is in the hundreds (e.g. 484 for n = 2²²), so **every block shorter than ≈ t is still found linearly**, although for long blocks `t = 0` costs only `2⌈log₂ m⌉` comparisons. Worked example (σ = 1024 distinct values, n = 2²²): at the top level blocks have ≈ 2048 elements; base policy pays `484 + 2 + 22 ≈ 508` comparisons per block, `t = 0` pays 22 — a 23× difference — and the gap closes only 3–4 levels down. Theorem 46 vs 51 in the paper show the same gap analytically (`log₂(H*+1)·n` vs `2·log₂(H*+1)·n` additive terms, with `t = Θ(H*)` optimal). TimSort's own adaptive rule (Algorithm 6 of the paper) is the classical fix and the paper shows it can be fooled (Props. 42–43).

## 3. The proposed modification
Start every merge at the paper's `t_max = ⌈log₂(a+b)⌉²` (keeps merges independent and the safe behaviour as the starting point). After each discovered block of length `n` that cost `tests` comparisons, with `len` run elements remaining:

```
observe(t, t_max, n, tests, len):
    if n < MinStreak and tests <= n:  return t           # short block found linearly: nothing to learn   (fast path)
    k := n - 1;  tail := len - 1                         # the tail search saw k "false" elements out of tail
    best := tests;  best_t := t
    for cand in { floor(t/2),  min(t_max, 2t+2),  0 (if JumpToZero) }:
        c := gallop_tests_model(cand, k, tail)           # exact cost of this block under cand (paper's cost_t, no key comparisons)
        if c < best:  best := c;  best_t := cand
    return best_t
```
Final configuration `SagTT<64, true>` (MinStreak 64, jump to 0 allowed). Properties:
* **No extra key comparisons**, no extra memory (state = two integers per merge), counterfactuals are arithmetic.
* **Fast path:** blocks shorter than 64 found linearly return immediately; on data without long blocks SAG executes the same comparisons as the base (verified: exactly equal for the halving ablation on random permutations, within +1 % for the final jump configuration).
* **Recovery:** after a jump to `t = 0`, a block whose search wasted comparisons (`tests > n`) triggers `t ← 2t+2`, so a mis-set `t` is repaired in O(log t) blocks; `t` never exceeds `t_max` and never drops below 0.
* **Per-merge reset** to `t_max`: no global state (the weakness exploited by Prop. 42–43 of the paper).

## 4. Complexity
| | base policy | SAG |
|---|---|---|
| Time per block (fast path) | O(1) extra | O(1) extra (two comparisons + a predictable branch) |
| Time per adaptation | — | O(log m) (three model evaluations); happens on blocks ≥ 64 or wasteful blocks only |
| Extra space | 0 | 0 (two integers) |
| Worst-case comparisons | ≤ naive + O(n) (paper, Prop. 49) | **≤ (4/3)·naive + O(n)**: for any `t ≥ 0`, `cost_t(m) ≤ (1+1/(t+3))·m ≤ (4/3)m` (paper Lemma 1), so each merge costs ≤ 2 + (4/3)(a+b) comparisons — checked on 65,459 merges in `tests/test_sag.cpp` |
| Average comparisons, structured inputs | O(n + nH*) | not proven; empirically fewer than the base (see results). **I do not claim a better worst-case bound than the paper's.** |

The weaker worst-case guarantee (4/3 instead of additive O(n)) is the price of adapting; the experiment measures how often it matters.

## 5. Why it should help, and when it should not
* **Helps** when blocks are long relative to `t_max`, i.e. low dual-run entropy / few distinct values / skewed duplicates, and when **comparisons cost much more than moves** (the paper's own condition for its cost model: "relevant only if element comparisons are significantly more expensive than element moves").
* **Neutral** on random permutations and on inputs with only short blocks (same comparisons as the base; ≈ +3–8 % bookkeeping time in cheap regimes).
* **Expected to fail** when comparisons are cheap (plain integers): galloping's probes are data-dependent branches that mispredict, so saved comparisons cost more than they gain; this already holds for the *base* galloping versus naive merging. On a long block followed by many short ones, a jump to `t=0` costs a few wasted comparisons before `t` recovers; the 4/3 bound limits the damage.

## 6. How the experiments separate a real improvement from noise
Paired same-repetition ratios SAG/base on identical inputs, randomised variant order, bootstrap CIs, an A/A control (`base_b`) measuring the noise floor (≈ 1 % in development), structural classification of cells (dual-run entropy, not timings), exact comparison counts from a separate pass, and criteria frozen in `docs/EXPERIMENT_PLAN.md` before the held-out run.

## 7. Design history (so the choice of constants is auditable)
* Round 1 (`results/raw/dev_counts.csv`, deterministic counts on 30 dev cells): halving-only variants save ~15 % comparisons on low-entropy cells; jump variants ~23 %; jump with MinStreak 16 looked best (mean 0.774 of base, max 1.008 on high-entropy cells).
* Round 2 (`results/raw/dev_selection.csv`, timing on dev data): MinStreak 16 costs +29 % (plain) / +5 % (heavy) on a structure-free input with rare long blocks because one outlier block pins `t` near 0 for the rest of the merge. Selection rule revised on *development* data to include timing guards; MinStreak 64 qualified with the best remaining gain. The first selection rule is kept in the log as a mistake caught before the held-out run.
* Earlier engineering fixes (shared by base and extension): forced inlining of `gallop_count`/`blit`; `adapt` returns `t` by value so the state does not escape.

## 8. Corrective round: SAG-C (time-aware, self-calibrating) - added after the round-1 result
**Why.** Round 1 showed SAG loses time with cheap comparisons: it decides from comparison counts, so after a jump to t = 0 it keeps paying for slow non-linear probes on short blocks without noticing (no comparisons are wasted). Code: `src/sag_time_policy.hpp`.

**Method (final, SAG-C).** The sorter measures the cost c of one comparison on 16,384 pseudo-random pairs of its own input (arrays shorter than 4096 use the base policy). With an architecture constant m = 16 ns for an unpredictable non-linear probe it defines the break-even block length B(c, m), the shortest block for which a pure exponential search beats a linear scan, and keeps a credit:
```
observe(n):                                   # n = length of the block just found
    if credit <= 0 and n < B:  return         # fast path: one compare, t stays t_max
    credit := clamp(credit + c*n - (c+m)*probes0(n), 0, cap)      # cap = 50*m
    t      := 0 if credit > 0 else t_max
```
A long block earns the time a pure exponential search would have saved; each short block spends its extra cost (about m); t = 0 persists while credit remains, so long blocks separated by one-element blocks (nearly-sorted data) stay cheap, while on structure-free data the credit never accumulates. No loops and no calls in the hot path.

**Complexity / guarantees.** Same as SAG: any t >= 0 costs <= (4/3)*block length in comparisons (tests: 79,434 merges in `tests/test_sagt.cpp`); extra space O(1); the only extra work per sort is the calibration (16,384 comparator calls), included in all timings. No better worst-case bound than the paper's is claimed.

**Development history (kept as evidence).** SAG-T v1 evaluated four candidates with the probe model after every long block: its adaptation call cost more than a block, giving up to +60 % time in cheap regimes (`results/raw/dev_sagt_v1_timemodel_NEGATIVE.csv`). SAG-B replaced the evaluation by the memoryless rule "t = 0 if the last block >= B else t_max": cheap regimes were fixed (0.99-1.09), but the nearly-sorted gain was lost (0.91 vs 0.69) because one short block resets t (`results/raw/dev_sagt_v2_breakeven.csv`). SAG-C (credit) is the third attempt; the selection rule (<= +5 % time vs base on every dev cell of the cheap regimes and on structure-free heavy cells, maximal heavy-regime gain) picked m = 16 ns, cap = 50*m (`results/raw/dev_sagt.csv`).

**Held-out result (fresh seeds, 54 cells).** Heavy regime, low-entropy cells: 0.725-0.863 of the base time (8 of 8 pass), nearly-sorted 0.53-0.56; cheap regimes: median 1.015 (plain) / 1.029 (indirect), 2 of 36 cells severe (>= 1.10 with CI above 1): corrective verdict **PARTIAL**. See `docs/FINAL_REPORT.md` section 7.
