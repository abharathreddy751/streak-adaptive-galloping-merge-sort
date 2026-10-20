# BASE_PAPER_IMPLEMENTATION — what was implemented from the paper, and how it was validated

Base paper: Ghasemi, Jugé, Khalighinejad, Yazdanyar, *Galloping in Fast-Growth Natural Merge Sorts*, Algorithmica 87(2), 2025, DOI 10.1007/s00453-024-01285-6 (read via arXiv:2012.03996 v3). Code: [src/natural_merge_sort.hpp](../src/natural_merge_sort.hpp) (contains **only** base-paper material; the extension lives in `src/sag_policy.hpp`). Syllabus topic: **Merge Sort** (Module 3).

## 1. The algorithm family (own words)
A *natural* merge sort turns Merge Sort's "split in halves" into "split at natural boundaries":
1. **Run detection** — scan left to right and cut the array into maximal monotone *runs* (non-decreasing, or strictly decreasing then reversed so that stability is kept). Runs shorter than `minrun` are extended with binary insertion sort (as TimSort/PowerSort implementations do).
2. **Merge policy** — keep a stack of pending runs; a rule decides which two *adjacent* runs to merge next. The paper's "fast-growth" policies (PowerSort, α-MergeSort, adaptive ShiversSort, TimSort, PeekSort) all keep the merge tree shallow for long runs, which yields O(n + nH) moves for run-length entropy H.
3. **Merge step** — merging runs A (length a) and B (length b) produces *blocks*: maximal streaks of consecutive output elements that come from the same run. The classic merge discovers a block of m elements with m comparisons. The **galloping** routine of TimSort discovers it with McIlroy's *mixed strategy with parameter t*: probe the first t positions one by one, then do an exponential search (positions t+1, t+2, t+4, …) followed by a binary search. Cost of discovering a block of m elements:

   `cost_t(m) = m` if `m ≤ t+2`, else `t + 2⌈log₂(m−t)⌉`   (always `≤ (1 + 1/(t+3))·m`)

4. **Policy for t.** The paper studies (i) fixed t, (ii) TimSort's own update rule (shown to be exploitable: Θ(n log n) comparisons on 3-valued arrays, Prop. 43; a Θ(n log n) overhead over naive merging, Prop. 42), (iii) the **polylogarithmic** rule `t = ⌈log₂(a+b)⌉²` that avoids both defects (Def. 47, Prop. 49: never more than O(n) comparisons above naive merging; Theorem 51: O(n + nH*) comparisons, and `nH* + 2n·log₂(H*+1) + O(n)` for tight-middle-growth sorts, where H* is the *dual-run* entropy; an array with σ distinct values has H* ≤ log₂ σ).

## 2. Pseudocode of what is implemented
```
natural_merge_sort(A, order, tpolicy, minrun):
    runs := detect runs (extend each to ≥ minrun by binary insertion)
    process runs left to right with the stack rule of `order`
        PowerSort : power p of the boundary between the top run X and the next run Y;
                    while stack.top.power ≥ p: X := merge(stack.pop(), X);   push (X, p);  X := Y
        TimSort   : paper Algorithm 2 (cases: r[h-2] < r[h]; r[h-1] ≤ r[h]; r[h-2] ≤ r[h-1]+r[h]; r[h-3] ≤ r[h-2]+r[h-1])
    merge remaining stack from the top

merge(A, B):                               # adjacent, both sorted
    if not (B[0] < A[last]): return        # already in order (1 comparison)
    copy the smaller run to a buffer; merge forward (buffer = A) or backward (buffer = B)
    t := tpolicy.start(a+b)
    owner := (B[0] < A[0]) ? B : A         # 1 comparison; ties go to A (stability)
    loop:
        block := 1 + gallop_count(tail of owner's remaining elements, t, predicate "belongs after the other run's head")
        move block (memmove for long blocks); owner := the other run;  tpolicy.observe(...)
        stop when either run is exhausted; flush the buffered remainder

gallop_count(seq, t, pred):   # pred monotone F…F T…T; returns #F
    test positions 1..t one by one                       # phase C1
    then positions t+1, t+2, t+4, … until pred = T       # phase C2, exponential
    then binary search in the last interval
```

## 3. Complexity (as stated or derived)
| Quantity | Value | Basis |
|---|---|---|
| Comparisons, any policy | O(n log n) worst case | middle-growth property (paper Thm 6) |
| Comparisons/moves, fast-growth + naive or galloping merge | O(n + nH), H = run-length entropy | paper Thm 5 |
| Comparisons, fast-growth + polylog galloping | O(n + nH*), H* ≤ log₂σ = dual-run entropy | paper Thm 7 / 51 |
| Per merge, t-galloping | ≤ 2 + (1+1/(t+3))(a+b) comparisons (1 extra for "already ordered?") | paper Lemma 1 / Prop 2 |
| Element moves | Θ(a+b) per merge in this model; memmove for long blocks | implementation |
| Extra space | min(a,b) ≤ n/2 elements buffer | TimSort-style |

## 4. Implementation decisions and deviations (all intentional, all identical for every variant compared)
1. **`minrun` extension by binary insertion sort** (default 32) — real TimSort/PowerSort do this; the paper's counterexamples also assume runs of length ≥ minrun (Sec. 5.1). Comparisons spent here are counted (`run_comparisons`).
2. **"Already ordered?" check** (1 comparison per merge) — not in the paper's cost model; it turns merging of already-ordered runs into O(1). Counted in `merge_comparisons`; the per-merge test allows exactly this +1.
3. **Block-first-element not re-tested.** The first element of a block is known to belong to it from the previous comparison, so only the tail is searched (the paper's cost model: a block of m costs `cost_t(m)`). *My first draft re-tested it; the smoke test showed +38 % comparisons vs naive merging on random permutations, contradicting Prop. 49; fixed and logged in `EXPERIMENT_LOG.md`.*
4. **Forward and backward merging** (smaller run buffered) share one `merge_core<D>` by pointer direction `D=±1` and a swapped comparator; ties always keep the left run first (stable).
5. **TimSort stack rule inequalities** (`<` vs `≤`) were lost in the PDF-to-text extraction; reconstructed from the fixed Java TimSort rule. Results (sortedness, stability, complexity bounds) were validated for this reconstruction, but it may differ in rare cases from the paper's exact Algorithm 2.
6. **t = ⌈log₂(a+b)⌉²** — floor/ceiling glyphs lost in extraction; this reading was used (the alternative ⌈log₂((a+b)²)⌉ differs by a constant factor 2 in rare cases and does not affect the asymptotic claims).
7. **Not implemented:** α-MergeSort, adaptive ShiversSort, PeekSort, ShiversSort, NaturalMergeSort stack rules; TimSort's real `merge_lo/merge_hi` (Algorithm 6 with its `min_gallop` state machine); the explicit counterexample arrays of Propositions 42–43; any of the proofs. PowerSort + TimSort stack rules cover the "fast-growth" class used in the theorems.
8. **Stability** is verified exactly (tags `(key, original index)` compared with `std::stable_sort`).

## 5. Validation evidence (Gate A) — `tests/test_sort.cpp`, base-only build
Command: `scripts/build.sh tests && build/test_base.exe`  (g++ 14.2.0, `-O2`).
Result: **1,502,829 checks passed, 0 failed**; the same suite under ASan+UBSan (WSL g++ 15.2.0, `scripts/sanitize_wsl.sh`): **1,502,829 passed, 0 failed**. Raw logs: `results/outputs/test_base_windows_gcc14.txt`, `results/outputs/sanitizer_test_base.txt`.

What the checks cover:
* `gallop_count`: reproduces the paper's **Table 1** for t=0 (m = 1…17) exactly; matches the closed form `cost_t(m)` for t ∈ {0,1,2,3,5,10,31} and m ≤ 300; forward and backward direction; agrees with an arithmetic model for all (t, len ≤ 70, k ≤ len).
* `node_power`: hand-computed cases plus range invariants.
* Sorting vs the stable reference (exact equality incl. stability) for **8 configurations** (2 stack rules × {naive, t=0, t=3, polylog}): all 44,100 pairs of sorted sequences (len ≤ 6, alphabet 4); all 29,524 arrays of length ≤ 9 over alphabet {0,1,2} × minrun {1,2,4}; all permutations of length ≤ 7 (5,914) × minrun {1,2}; 2,445 deterministic random arrays from 15 families (sizes 0…100,000, incl. all-equal, sorted, reversed, pipe-organ, few distinct values, Zipf, concatenated runs, mixture) × minrun {2,32}; textbook top-down Merge Sort as a second oracle.
* Edge cases: n=0 with null pointer, n=1,2,3 equal keys, minrun 0 and minrun > n, extreme key values, t-policy at 2⁴⁰ elements, `ceil_log2` boundaries.
* Paper-derived invariants (counting build): **per-merge comparison bound** `≤ 2 + (1+1/(t+3))(a+b)` held in every merge of 28 runs (7 families × 4 seeds); **Prop. 49** (polylog never exceeds naive by more than O(n)): observed (V−U)/n ≤ 0.000 over 28 workload/parameter pairs; **Thm 51 shape**: comparisons ≈ nH* + 2n·log₂(H*+1) + C·n with measured C ≤ 4.1 (this C includes the insertion-sort cost of minrun=32 runs); information-theoretic lower bound respected on random permutations (counter is real); sorted/reversed inputs cost exactly n−1 comparisons; buffer ≤ n/2; determinism.

**Scope of the validation.** These tests show that the implementation sorts correctly and stably, that the galloping routine has exactly the paper's cost function, and that the measured comparison counts obey the paper's bounds on the families tested. They do *not* reproduce the paper's proofs or its adversarial constructions, and they are sampled/exhaustive only over the stated domains, not exhaustive in general.
