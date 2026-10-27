# Viva preparation

Short, accurate answers. Numbers come from the files named in `docs/FINAL_PROJECT_REPORT.md`. If an examiner asks something that is not covered here, say what was and was not tested.

## The project
**1. What problem does the project solve?**
Natural merge sorts (the family of TimSort) spend most of their time in the merge step. If comparing two keys is expensive, the number of comparisons matters. A 2024 journal paper gives a safe rule for the galloping parameter `t` in that merge step but fixes `t` per merge. We ask whether `t` can be adapted while merging, and whether fewer comparisons really mean less time.

**2. Which syllabus topic does it implement?**
Merge Sort (Module 3), in its natural, run-based form, with a stable merge and a temporary buffer. The project changes and evaluates the merge step. A textbook top-down Merge Sort is also included as a reference.

**3. What is Merge Sort, and what is natural merge sort?**
Merge Sort sorts two halves and merges them; it takes `O(n log n)` comparisons and is stable. A natural merge sort does not cut the array in halves blindly: it first finds the runs that are already sorted and merges those, so nearly sorted data is cheaper.

**4. What is the base paper?**
Ghasemi, Jugé, Khalighinejad, Yazdanyar, "Galloping in Fast-Growth Natural Merge Sorts", Algorithmica 87(2), 242-291, Springer, online 9 December 2024 (issue 2025), DOI 10.1007/s00453-024-01285-6. It is theory only (no code, no experiments). Our code is an independent re-implementation of the relevant method, not the authors' code, and we did not reproduce all proofs.

**5. What are runs, merging, stability, galloping, minrun and the stack policy?**
- A *run* is a maximal already-ordered part of the array. Falling runs are reversed (strictly falling only, which keeps stability).
- *Merging* joins two sorted runs into one by repeatedly taking the smaller front item.
- *Stable* means equal items keep their original order; ties go to the left run.
- *Galloping* finds how long a streak from one run is: check `t` items one by one, then jump 1, 2, 4, 8 positions and binary-search back.
- *minrun* (32 here) is the smallest run length; shorter runs are extended by binary insertion sort.
- The *stack policy* decides which neighbouring runs to merge next; we implement PowerSort and TimSort's rule (the paper's Algorithm 2).

## SAG and the experiments
**6. What is SAG and how does it differ from the base?**
SAG changes one thing: the rule for `t`. The base uses `t = ceil(log2(a+b))^2` for the whole merge. SAG starts with that value and, after each long streak, uses the paper's cost formula to pick the cheapest of `t/2`, `t`, `2t+2` and `0`; it resets at the next merge. It uses no extra memory and no extra key comparisons. Everything else is identical code.

**7. What are expensive-key comparisons, and why can fewer comparisons fail to make the sort faster?**
Comparing long strings reads many bytes and often waits for memory, so one comparison costs tens of nanoseconds. Comparing small integers costs about a nanosecond. A smaller galloping parameter replaces predictable one-by-one checks by jumps and binary search, whose branches are hard to predict. With cheap keys that extra cost can exceed the comparisons saved; with expensive keys the saved comparisons dominate. Our data show this: gains for long keys, little or no gain for short keys.

**8. How were the experiments designed, and how was the gain criterion set?**
Same input for every method, random method order in every repetition, many repetitions, an A/A control (the base run twice) for the noise level, paired time ratios with 95 percent bootstrap confidence intervals, and every output checked against a stable-sort reference. Rules, data, seeds and file hashes were written down and frozen before each held-out run. For Stage 3 the gain set was defined from the data alone: low entropy (`H* <= 0.5 log2 n`) and expensive keys (comparisons that examine at least 32 bytes). Success required a time ratio of at most 0.90 with the interval's upper bound below 1.0 in at least 11 of 16 such cells, no severe regression in any of 50 cells, and comparison counts at most 1.03 times the base.

**9. What do the time ratios and confidence intervals mean?**
A ratio is SAG time divided by base time in the same repetition (median over repetitions). 0.80 means 20 percent faster; 1.10 means 10 percent slower. The 95 percent interval says how uncertain the median is: if its upper end is below 1.0, SAG is faster with good confidence. A *severe regression* is a ratio of at least 1.10 with the interval's lower end above 1.0.

**10. Why is the result called a conditional confirmation success?**
Because all frozen criteria were met (12 of 16 gain cells, no severe regression in 50 cells, correct outputs, counts within limits, checker agreement), but only for the stated kind of workload: expensive-key, low-entropy strings. The margin is narrow (12 against 11), 48 of 50 cells are synthetic, the two real file-name cells are tiny, and it was one machine and one compiler.

**11. Why is it wrong to say SAG is always faster?**
Because it is not: with short keys the ratios were 0.938-1.076 (up to 7.6 percent slower), with integer keys in Stage 1 it was slower in all 54 cheap-comparison cells, four expensive-key cells did not reach 10 percent, and random permutations show no gain.

**12. What happened with SAG-C?**
SAG-C is a time-aware variant that measures the cost of one comparison at the start. In Stage 2 it failed the severe-regression guard (ratio 1.114 on one cell), because its measurement was 3.7-9.1 times the real cost inside the sort. In Stage 3 it was only a diagnostic and had a severe regression (1.276) on the small natural-order file-name case. We did not retune it, and it is not the final candidate.

## Correctness and limits
**13. What was the comparator-exception bug, and what does the fix guarantee?**
If the comparator threw in the middle of a merge, part of the smaller run existed only in the temporary buffer, so items could be lost or duplicated (32,069 of 84,623 reproduced cases). A small guard now copies the unmerged part back while the exception unwinds. The guarantee is the *basic* guarantee: the array stays a permutation of the input (0 of 84,623 cases fail after the fix). Order and stability are not guaranteed after an exception, and it is not the strong guarantee. For `noexcept` comparators the guard generates no code.

**14. What are the limitations?**
Conditional result; narrow margin; mostly synthetic data and small real samples; one machine and compiler in Stage 3; comparison counts are not time; Stage 1-2 timings audited but not re-measured; audits are internal; sanitizer runtimes unavailable (we used checked-container, undefined-behaviour-trap and overflow-trap builds); article-level Scopus indexing unverified. Full list: `docs/KNOWN_LIMITATIONS.md`.

**15. Which files and commands reproduce the final result?**
Code: `src/natural_merge_sort.hpp` (base and exception guard), `src/sag_policy.hpp` (SAG). Commands: `bash scripts/run_all_tests.sh` (clean build and all tests) and `bash stage3/tools/final_pass3.sh` (final gate). Evidence: `stage3/results/s3_heldout.csv`, `stage3/results/summary_s3_heldout.md`, `docs/STAGE3_CONFIRMATION_RESULTS.md`, frozen plan `docs/STAGE3_CONFIRMATION_PLAN.md`. The timing experiment itself is not re-run by the gate (it was run once, as designed, and its raw data are included).

**16. What can and cannot be claimed about the paper's indexing status?**
Can say: it was published in Algorithmica (Springer), and the publisher's journal page lists Scopus among the journal's indexing services. Cannot say: that the individual article is verified as indexed in Scopus. Scopus itself needs a login and we did not verify it there.

## Extra questions that are likely
**17. Is the Stage 3 audit an external review?** No. It is an internal audit with separate code that recomputes the results from raw data.
**18. Is SAG new?** We do not claim novelty. TimSort already adapts its own galloping threshold; our contribution is a cost-model-based rule and a careful, pre-registered test of when it helps. No full literature search for priority was done.
**19. What is the worst case of SAG?** Each streak costs at most 4/3 of its length (checked on a finite range, not proven). We do not claim the paper's stronger "naive plus linear" bound for SAG.
**20. Did the exception fix slow the sorter?** Counts and outputs were identical in 216 differential runs and a micro-benchmark showed no systematic change, but the compiled code of one routine is not byte-identical, so we say "no measured effect", not "identical code".

## Questions about the v4 terminal tool
**21. What did v4 add?** A terminal tool (`build/nms_tool`) and an optional execution trace: `profile` measures an input, `trace` shows every event of a small sort, `compare` runs textbook, naive, base, SAG and `std::stable_sort` on identical data and cross-checks the counters. No algorithm and no result changed.
**22. Is the trace a made-up story?** No. The sorter emits events (run found, merge begin, merge skipped, block found, SAG `t` change, merge end) to a callback while it runs; the tool only prints them. A test checks that the events agree with the exact counters, and that attaching a trace changes neither the output nor any counter.
**23. Does the trace slow the timed sorters?** No: the hook exists only in the counting sorters; for the uncounted ones the compiled code is identical to v3 at `-O2` (`validation/v4/asm_check`).
**24. Why does `compare` show fewer comparisons but no speed-up for integers?** A comparison of two integers costs about one nanosecond, so the saved comparisons are outweighed by extra bookkeeping. With 64-byte string keys that share a prefix (`strings`), a comparison costs much more and the saving can show up in time. This is the same conclusion as the frozen experiments; the tool's timings are only demonstrations.
**25. Is `nearly sorted` or `low-entropy` just a name?** No. The profiler prints fixed definitions (for example low-entropy means H* <= 0.5 log2 n) and the measured numbers behind each label.
