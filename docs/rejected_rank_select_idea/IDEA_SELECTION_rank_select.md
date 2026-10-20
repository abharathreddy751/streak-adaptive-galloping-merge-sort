# IDEA_SELECTION — candidate comparison and final decision

Time-boxed research (about 45 minutes). Evidence: Crossref/OpenAlex metadata, arXiv abstracts, and, for the chosen paper, the full open-access text. For the non-chosen candidates **only metadata and abstracts were read**, so their scores are judgements, not measurements.

Project-history constraints (taken from the task brief; the LPQ folders were not read because directory access was declined): no priority-queue / learned-PQ work, no dynamic-SSSP repair-vs-recompute, no fixed-budget hybrid. Lessons applied: measure memory and time separately, pre-freeze success criteria, test several workload families, report per-cell results.

## Candidates

### C1 — Kurpicz, *Engineering Compact Data Structures for Rank and Select Queries on Bit Vectors* (Springer LNCS 13617, SPIRE 2022) — DOI 10.1007/978-3-031-20643-6_19
- **Method:** "flat-popcount": a 3-level block index (L0 optional, L1 = 4096-bit blocks, L2 = 512-bit blocks) with absolute 12-bit L2 counts interleaved with a 44-bit L1 count in one 128-bit word; same ~3.1% rank-index overhead as cs-poppy. Select = a hint for every 8192-nd one, then a *scan of the L1 index*, then a search over 7 L2 entries (linear / binary / SSE), then an in-word select.
- **Limitation we can address (read in the text, Sec. 3.1–3.2):** after the sampled hint the L1 index is *scanned*; the paper itself says select needs "linear time in the worst-case". Its adversarial inputs (99% of the ones in the last k% of the bits) create very long hint intervals, and cs-poppy runs time out on some inputs.
- **To reproduce:** bit-vector builder, block index, rank, select1/select0 with hints, SSE L2 search, select-in-word. About 500 lines of C++, no dependencies.
- **Extension (proposed):** *span-adaptive L1 search* — use the next hint as an upper bound, binary-search the monotone L1 counts while the interval is wider than T blocks, and finish with the paper's linear scan. No extra space.
- **Data/hardware:** synthetic generators only (the paper itself uses synthetic uniform and adversarial vectors); fits in 8 GB RAM at ≤ 2^30 bits.
- **Comparison / metrics:** the paper's flat-popcount select vs the extension; ns/query, L1 probes/query, index bytes, correctness agreement.
- **Risks:** gain may vanish on uniform dense vectors (expected); hardware-prefetched linear scans are strong baselines; timing noise on a hybrid P/E-core laptop.

### C2 — Graf & Lemire, *Binary Fuse Filters* (ACM JEA 27, 2022) — DOI 10.1145/3510449
- **Method:** 3-wise segmented XOR-style approximate-membership filter built by hypergraph peeling; about 13% above the information-theoretic minimum (abstract).
- **Possible limit:** construction cost and retries at small sizes; a query is 3 probes.
- **To reproduce:** hashing, segment layout, peeling queue, fingerprint array — feasible (~300 lines), but only the abstract was read.
- **Extension idea:** batched / prefetched queries or cache-aware construction. Likely small, since the authors' library is already tuned; chance of an honest measurable win is low.

### C3 — Even, Even & Morrison, *Prefix Filter* (PVLDB 15(7), 2022) — DOI 10.14778/3523210.3523211
- **Method:** incremental approximate-membership filter: bin table of pocket dictionaries plus a spare structure for overflow.
- **To reproduce:** pocket dictionary with SIMD operations, spare filter, spilling analysis. Substantially more engineering (~800+ lines); only the abstract was read.
- **Extension idea:** adaptive spare sizing — plausible, but the evaluation needs space / false-positive / time sweeps that are costly.

### C4 — Axtmann, Witt, Ferizovic & Sanders, *Engineering In-place (Shared-memory) Sorting Algorithms* (ACM TOPC 9(1), 2022) — DOI 10.1145/3505286
- **Method:** IPS4o, in-place parallel super-scalar samplesort (branchless decision tree, block-based classification, in-place permutation).
- **To reproduce:** the sequential core is feasible but intricate (block permutation, buffers, equality buckets); a faithful port plus a convincing improvement over a mature design is unlikely in the time. Only metadata read.

### C5 — Henzinger, Noe & Schulz, *Practical Fully Dynamic Minimum Cut Algorithms* (SIAM, 2022) — DOI 10.1137/1.9781611977042.2
- **Rejected on project-history grounds:** a dynamic update-versus-recompute study repeats the direction that was already stopped, even though it concerns minimum cut rather than SSSP.

## Scoring (1 = poor, 5 = excellent; judgement)
| Factor | C1 rank/select | C2 binary fuse | C3 prefix filter | C4 IPS4o | C5 dyn. min-cut |
|---|---|---|---|---|---|
| DSA relevance | 5 | 5 | 5 | 5 | 4 |
| Paper credibility & verifiability | 4 (Springer; full text read; Scopus unverified) | 4 | 4 | 4 | 3 |
| Feasibility of a faithful base in the time | 5 | 4 | 3 | 2 | 2 |
| Clarity of the extension | 5 (weakness stated in the text) | 3 | 3 | 2 | 3 |
| Opportunity for measurable gain | 4 | 2 | 3 | 2 | 3 |
| Strength of the evaluation plan | 5 (clear metric, adversarial inputs already defined) | 3 | 3 | 4 | 3 |
| **Total / 30** | **28** | **21** | **21** | **19** | **18** (also rejected by history rule) |

## Decision
**Choose C1.** It scores highest on the factors that matter most for an unattended run: the method is fully specified in an open-access paper I read, the base can be validated against an independent brute-force oracle, the weakness (linear L1 scan after the sampled hint) is stated by the paper itself, and the extension is a precise, space-neutral modification whose expected benefit and expected failure region (dense/uniform vectors, where intervals are short) can be predicted and then tested.

**Honest expectations:** the paper's *random-rank* queries mostly land in dense regions, so improvement on the paper's own workloads may be small. The extension should help when ones (or zeros) are sparse or clustered — exactly where the linear scan is long. The experiment is designed to show where it helps, where it does not, and whether it ever hurts.

**No ML component** is used: nothing in the problem needs a learned model, and a model would add inference cost to a nanosecond-scale query.

**Novelty statement:** I make no first-of-its-kind claim. Binary search over block counts is classical, and Vigna's `simple-select` (cited in the paper) already handles sparse regions with adaptive inventories. The contribution is a *proposed extension* evaluated on the Kurpicz structure specifically.
