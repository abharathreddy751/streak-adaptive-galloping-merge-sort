# SYLLABUS_COMPLIANCE — MACSE501 hard gate

Written 2026-10-19 after the override requirement. Syllabus compliance is treated as a hard gate: the project's *central* algorithm must be a named MACSE501 topic (list supplied by the student in the override message).

## 1. Audit of the ORIGINAL idea

| Item | Value |
|---|---|
| Original idea | Span-adaptive select for flat-popcount rank/select bit vectors (extension of Kurpicz, *Engineering Compact Data Structures for Rank and Select Queries on Bit Vectors*, Springer LNCS 13617, SPIRE 2022, DOI 10.1007/978-3-031-20643-6_19) |
| Central algorithm / data structure | Succinct rank/select index over a bit vector (3-level block counts, sampled select hints, SIMD block search, in-word select) |
| Closest syllabus entry | Module 1: "Arrays" |
| Why "Arrays" does not rescue it | The bit vector is an array only as a *storage substrate*. The research subject — succinct o(n)-space rank/select indexing — is not a listed topic. This is exactly the "NOT ACCEPTABLE" case in the override: a familiar structure used as an implementation detail while the contribution concerns an algorithm outside the syllabus. |
| Verdict | **FAIL — REJECTED — OUTSIDE MACSE501 SYLLABUS** |

Actions taken (per the override): implementation and benchmarking stopped. At the moment of rejection **no source code had been written** (only documentation drafts; every attempt to create source files had been blocked by an editor gate and none succeeded). The two rejection-era documents were *moved, not deleted* to `docs/rejected_rank_select_idea/` (`IDEA_SELECTION_rank_select.md`, `PAPER_VERIFICATION_rank_select.md`). The old LPQ project and its backup were never touched (access to them was never granted).

## 2. Replacement search — started from the syllabus, not from a paper

For each syllabus topic I searched Crossref and OpenAlex for peer-reviewed 2022+ papers from IEEE / Springer / Elsevier, then checked whether the full text was readable (a faithful base implementation needs the method text).

| Syllabus topic (module) | Candidate paper found | Publisher / venue (year) | Full text readable? | Outcome |
|---|---|---|---|---|
| **Merge Sort (M3)** | Ghasemi, Jugé, Khalighinejad, Yazdanyar, *Galloping in Fast-Growth Natural Merge Sorts* | Springer, *Algorithmica* 87(2), 2025 (online 2024-12-09), DOI 10.1007/s00453-024-01285-6 | **Yes** (arXiv:2012.03996 v3, pseudocode + proofs) | **SELECTED** |
| Quick Sort (M3) | Ketchaya & Rattanatranurak, *Parallel Multi-Deque Partition Dual-Deque Merge sorting algorithm using OpenMP* | Nature Portfolio/Springer Nature, *Sci. Reports* 13, 2023, DOI 10.1038/s41598-023-33583-4 | Yes (OA) | Not chosen: many-core OpenMP algorithm evaluated on 16-core Xeon; this hybrid 2P+8E laptop would give noisy, non-comparable parallel timings |
| Graph Coloring (M4) | Yakut, *Malatya Vertex Coloring Algorithm* | IEEE Access 13, 2025, DOI 10.1109/ACCESS.2025.3614296 | **No** (IEEE page not retrievable; only abstract) | Not chosen: cannot be reproduced faithfully from the abstract alone |
| Approx. Vertex Cover (M5) | Yakut et al., *Malatya vertex-cover algorithm* | Springer, J. Supercomputing 2023, DOI 10.1007/s11227-023-05397-8 | No (closed access) | Not chosen |
| Approx. Set Cover (M5) | Delgado, Ferrada, Navarro, *A succinct and approximate greedy algorithm for the Minimum Set Cover Problem* | Elsevier, J. Comput. Sci. 2024, DOI 10.1016/j.jocs.2024.102378 | No (closed access) | Not chosen |
| N-Queens (M4) | Bouneb, *Distributed algorithm for parallel computation of the n queens solutions* | Elsevier, Expert Syst. Appl. 2024, DOI 10.1016/j.eswa.2024.124677 | No (closed access) | Not chosen |
| DFS (M2) | Wan & Wang, *Efficient semi-external depth-first search* | Elsevier, Inf. Sci. 2022, DOI 10.1016/j.ins.2022.03.078 (arXiv:2006.03198) | Yes | Not chosen: semi-external-memory model needs disk-resident graphs; large datasets and I/O-bound measurement |
| Strassen (M3) | Bessant et al., *Improved parallel matrix multiplication using Strassen and Urdhvatiryagbhyam method* | Springer, CCF Trans. HPC 2023 | No (closed) | Not chosen |
| Karatsuba (M3) | Pogue & Nicolici, *Karatsuba matrix multiplication and its efficient custom hardware implementations* | IEEE Trans. Computers 2025 | No / hardware-centred | Not chosen |
| Heaps / Dijkstra-type priority queues | — | — | — | **Deliberately excluded**: priority-queue/shortest-path work is the previously stopped LPQ direction |
| Segment trees, B-trees, AVL/Red-Black, Kruskal/Prim, Huffman, LCS, Subset Sum, A*, BFS, MCM, Knight's tour, Bellman-Ford, Ford-Fulkerson, Push-Relabel | searched (Crossref + OpenAlex keyword queries) | — | — | No suitable 2022+ IEEE/Springer/Elsevier paper with readable method text surfaced in the time-boxed search; search results were dominated by application papers that only *use* the algorithm. This is a statement about my search, not a claim that none exist. |

## 3. Replacement decision

| Item | Value |
|---|---|
| Replacement central algorithm | **Merge Sort — specifically the merge step and the run-merging order of *natural* merge sorts** (TimSort-family: PowerSort / α-merge policy with a galloping merge routine) |
| Exact syllabus entry | **Module 3 — Divide and Conquer and Greedy Algorithms → "Merge Sort"** (supported by Module 1 "Recursion and performance analysis" and "Asymptotic notation" for the complexity analysis, and "Stacks and queues" for the run stack) |
| Relationship to the base paper | The base paper analyses merge-sort variants and proves comparison bounds for a *galloping merge routine* (McIlroy's mixed linear/exponential search). We re-implement those algorithms and propose a change to the merge routine's parameter policy. |
| Baselines kept in the project | plain top-down/bottom-up **Merge Sort** (syllabus form), naive-merge natural merge sort, paper's galloping variant, `std::stable_sort` as an independent reference |
| Verdict for replacement | **PASS** |

**Honest scope note.** "Natural merge sort", "galloping" and "PowerSort" are not named individually in the syllabus; they are variants of the syllabus Merge Sort (same merge step and divide-and-conquer structure; they differ in how runs are found, which adjacent runs are merged, and how the merge step searches for block boundaries). The override explicitly accepts a project that "modifies, improves, or evaluates a syllabus-listed algorithm". The central subject of the extension — the *merge step of Merge Sort* — is syllabus-listed, and the project includes the plain textbook Merge Sort as a baseline so the connection is demonstrable. A strict reader could still argue that galloping is beyond the syllabus; I judged that this risk is much lower than for any alternative with a readable, implementable method.

## 4. Base-paper status for the replacement
Metadata verified through Crossref + OpenAlex + arXiv (details in `docs/PAPER_VERIFICATION.md`): 2025 volume (online 2024), Springer, *Algorithmica*, peer-reviewed journal article, DOI 10.1007/s00453-024-01285-6. Full text read via the arXiv v3 preprint (the published version may differ in minor details). **Scopus: not independently verified** (a web-search snippet of Springer's journal page lists Scopus among the journal's indexing services; I did not confirm it in the Scopus source list). The base paper's algorithm itself (a merge-sort variant) is within permitted scope.

## 5. Gate result
- Original idea: **FAIL / REJECTED**.
- Replacement (natural merge sort with galloping merge, syllabus: Merge Sort): **PASS**, proceeding to implementation with the base algorithm first.
