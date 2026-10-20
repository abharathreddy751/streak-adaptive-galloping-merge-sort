# IDEA_SELECTION — syllabus-first candidate comparison (replacement round)

This is the second selection round. Round 1 (rank/select bit vectors) was **rejected as outside the MACSE501 syllabus**; its documents are preserved in `docs/rejected_rank_select_idea/`. Round 2 started from the approved syllabus topics (see `docs/SYLLABUS_COMPLIANCE.md` for the full topic sweep) and only then looked for papers.

Evidence standard: for every candidate I record whether the **full method text was readable**. A candidate whose method I cannot read cannot be reproduced faithfully and is penalised heavily. Non-selected candidates were assessed from abstracts/metadata only; their scores are judgements.

Project-history constraints (from the task brief; the LPQ folders were not opened): no priority queues / learned PQ / dynamic-SSSP repair-vs-recompute / fixed-budget hybrids. Lessons applied: time and operation counts are measured separately, success criteria are frozen before held-out data, results are reported per workload.

## Candidates (each mapped to a syllabus topic)

### C1 — Natural merge sorts with galloping  ·  syllabus: **Merge Sort (Module 3)**
- **Paper:** E. Ghasemi, V. Jugé, G. Khalighinejad, H. Yazdanyar, "Galloping in Fast-Growth Natural Merge Sorts", *Algorithmica* 87(2):242–291, Springer, online 2024-12-09 (print Feb 2025), DOI 10.1007/s00453-024-01285-6. Preprint: arXiv:2012.03996 (ICALP 2022 conference version exists). Peer-reviewed journal; Scopus listing not independently verified.
- **Main algorithm:** stable natural merge sorts (PowerSort, α-MergeSort, adaptive ShiversSort, TimSort, …) that find monotone runs and merge adjacent runs by a stack policy; the *merge routine* uses McIlroy's mixed "t-galloping" (t linear probes, then exponential + binary search). Proves comparison bounds O(n + nH*) (H* = dual-run entropy) for a "polylogarithmic" t = ⌈log₂(a+b)⌉²; shows TimSort's own t-update rule can cost Θ(n log n) comparisons on 3-valued arrays and a super-linear overhead vs naive merging.
- **Limitation we can address:** the proven t policy is *workload-oblivious* (t depends only on a+b). At the top merge levels t ≈ log²n (hundreds), so every streak of length ≲ t is still discovered linearly: nominal bound nH* + 2n·log₂(H*+1) vs the nH* + n·log₂(H*+1) reachable with the right t (Thm 46 vs 51). Also the cost model counts comparisons only and the authors state it matters only when comparisons dominate moves — so "fewer comparisons" may not mean less time.
- **To reproduce:** run detection (+ min-run extension), PowerSort / α-merge stack policy, galloping merge (forward and backward), polylog t, comparison counter. About 500 lines of header-only C++17, no dependencies.
- **Proposed extension:** *Streak-Adaptive Galloping (SAG)* — adapt t **within each merge** from the exact saving/waste of the last discovered streak (multiplicative decrease when galloping paid off, multiplicative increase back toward the paper's safe polylog value when it wasted comparisons), starting each merge from the paper's t.
- **Data/hardware:** purely synthetic generators with controlled run / dual-run structure (the paper is theoretical and has no datasets); 8 GB RAM is enough up to ~2^24 elements.
- **Comparisons / metrics:** naive merge, paper's polylog-t galloping (the base), TimSort-style update rule (simplified), `std::stable_sort`, textbook Merge Sort; metrics = comparisons, wall-clock ns/element (int keys and expensive-comparison keys), moves, extra memory.
- **Risks:** cheap integer comparisons may make galloping slower than naive merging regardless of policy (a known reason Rust/Swift dropped it) — this would produce a no-go on the time metric, which is an acceptable honest outcome.

### C2 — Parallel Multi-Deque Partition / Dual-Deque Merge sort  ·  syllabus: **Quick Sort (Module 3)**
- Ketchaya & Rattanatranurak, *Scientific Reports* 13 (2023), Nature Portfolio/Springer Nature, DOI 10.1038/s41598-023-33583-4. Full text readable (OA).
- Limitation: parallel block partitioning with shared deques; speed-ups reported on 16-core Xeon / 8-core i7. Reproduction on a 2P+8E laptop would give non-comparable, noisy parallel numbers; heavy OpenMP-on-MinGW risk. Extension would also be parallel-tuning (block size), weakly algorithmic.

### C3 — Malatya Vertex Coloring  ·  syllabus: **Graph Coloring (Module 4)**
- Yakut, *IEEE Access* 13 (2025), DOI 10.1109/ACCESS.2025.3614296. Abstract: colour vertices in descending "Malatya centrality" (sum over neighbours of deg(v)/deg(u)). **Method text not retrievable** — only the abstract; tie-breaking and ordering details unknown. Rejected for unverifiable fidelity (would be an excellent project if the text were accessible).

### C4 — Malatya vertex-cover algorithm  ·  syllabus: **Approximation algorithms for Vertex Cover (Module 5)**
- Yakut, Öztemiz, Karcı, *J. Supercomputing* (2023), DOI 10.1007/s11227-023-05397-8. Closed access; abstract not available to me. Rejected.

### C5 — Succinct approximate greedy Set Cover  ·  syllabus: **Approximation algorithms for Set Cover (Module 5)**
- Delgado, Ferrada, Navarro, *J. Computational Science* (2024), Elsevier, DOI 10.1016/j.jocs.2024.102378. Closed access, no abstract retrieved. Rejected.

### C6 — Semi-external DFS  ·  syllabus: **DFS (Module 2)**
- Wan & Wang, *Information Sciences* (2022), Elsevier, DOI 10.1016/j.ins.2022.03.078 (arXiv:2006.03198). Readable, but the problem is the semi-external-memory model (graph on disk, O(n) memory): needs large on-disk graphs and I/O measurement. Rejected for feasibility in the environment.

## Scoring (1 = poor … 5 = excellent; judgement)
| Factor | C1 galloping merge | C2 MPDMSort | C3 Malatya colouring | C4 Malatya VC | C5 set cover | C6 semi-ext. DFS |
|---|---|---|---|---|---|---|
| Syllabus fit (hard gate) | pass (Merge Sort) | pass (Quick Sort) | pass | pass | pass | pass |
| DSA relevance | 5 | 4 | 4 | 4 | 4 | 4 |
| Paper credibility / verifiability | 5 | 4 | 4 | 3 | 3 | 4 |
| Feasibility of faithful base (method readable) | 5 | 3 | 1 | 1 | 1 | 3 |
| Clarity of extension | 4 | 2 | 3 | 2 | 2 | 2 |
| Opportunity for measurable gain | 3 | 2 | 4 | 3 | 3 | 2 |
| Strength of evaluation plan | 5 | 3 | 4 | 3 | 3 | 2 |
| **Total / 30** | **27** | **18** | **17** | **14** | **14** | **17** |

## Decision
**Choose C1.** It is the only candidate that combines (a) a syllabus-listed central algorithm, (b) a method I could read in full (pseudocode, proofs, cost model), (c) an exactly measurable quantity (comparison counts) with analytic bounds to check the base implementation against, and (d) a clean extension point (the t policy of the merge routine). Its main risk — that wall-clock time may not follow comparison counts — is itself a legitimate research question and is the reason the experiment plan measures both.

**No ML** is used: the adaptation rule is a two-line multiplicative controller on observed streak lengths. **No first-of-its-kind claim** is made: TimSort's own `min_gallop` already adapts t during a merge (the paper analyses that rule and its weaknesses); SAG is a *proposed extension* differing in using exact saving/waste, multiplicative steps, and a per-merge reset to the proven safe value.
