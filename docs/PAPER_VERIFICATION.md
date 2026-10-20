# PAPER_VERIFICATION — base paper (replacement round)

Checked 2026-10-19 with the Crossref REST API, the OpenAlex API, the arXiv abstract page and a web search. Round-1 verification (rank/select paper) is preserved in `docs/rejected_rank_select_idea/`.

## Base paper

| Field | Value | Source |
|---|---|---|
| Title | Galloping in Fast-Growth Natural Merge Sorts | Crossref, OpenAlex, arXiv |
| Authors | Elahe Ghasemi (Northeastern University), Vincent Jugé (CNRS), Ghazal Khalighinejad (Sharif University of Technology), Helia Yazdanyar (CNRS) | Crossref author list; OpenAlex affiliations |
| Publisher | Springer Science and Business Media LLC (Springer Nature) | Crossref `publisher`; OpenAlex host "Springer Science+Business Media" |
| Venue | *Algorithmica*, vol. 87, issue 2, pp. 242–291; ISSN 0178-4617 / 1432-0541 | Crossref |
| Year | published online **2024-12-09**; print issue February 2025 | Crossref `published-online` / `published-print` |
| DOI | 10.1007/s00453-024-01285-6 | Crossref |
| Official link | https://doi.org/10.1007/s00453-024-01285-6 (Springer landing page not opened: Springer pages answer automated requests with a cookie-redirect) | — |
| Type | peer-reviewed journal article | Crossref `journal-article` |
| Preprint / full text read | arXiv:2012.03996 (v1 7 Dec 2020, v3 1 Dec 2023, 38 pages). OpenAlex also lists the ICALP 2022 conference version (LIPIcs vol. 229, DOI 10.4230/lipics.icalp.2022.68) and HAL hal-04323404. | arXiv abstract page; OpenAlex locations |
| Open access | green OA (arXiv); the Springer version is not open access | OpenAlex `oa_status: green` |

## Full text actually inspected
I downloaded the arXiv PDF and read the introduction, Section 2 (galloping routine, cost model, t-policies), Section 3 (fast-/middle-growth properties, Theorems 5–7), the Algorithm 1–3 pseudocode of Section 4, Section 5 (TimSort's update policy, Algorithm 6, Propositions 42–44) and Section 6.1–6.2 (Theorems 46 and 51, polylogarithmic galloping). Sections 4.1.x proofs, 6.3–6.4 and the appendix-level details were skimmed only. **The text is the arXiv v3 preprint, not the Springer version of record**; differences, if any, are unknown. The PDF-to-text conversion lost some floor/ceiling glyphs, so the exact rounding in the polylog parameter `t = ⌈log₂(a+b)⌉²` (vs ⌈log₂((a+b)²)⌉ or ⌊·⌋) is an interpretation (differences are immaterial for the claims).

## Indexing status
- **Scopus: NOT independently verified.** A web-search snippet quoting Springer's journal page for *Algorithmica* lists "SCOPUS" among the journal's abstracting/indexing services; a Korean university repository record shows Scopus coverage 2017–2020. I did not open the Scopus source list or Springer's page itself (Scimago returned HTTP 403, Springer pages cookie-redirect). So: **"reported by the publisher's journal page (secondary retrieval), not confirmed in Scopus."** The project does not depend on it.
- Science Citation Index Expanded is also listed in the same snippet; not verified either.

## Requirement check
| Requirement | Status |
|---|---|
| Published 2022 or later | **Yes** (2024-12-09 online; arXiv v1 is 2020 but the peer-reviewed version is 2024/25) |
| Peer-reviewed, reputed publisher (IEEE/Springer/Elsevier) | **Yes** — Springer, *Algorithmica* |
| Verifiable title, authors, year, venue, DOI | **Yes** (Crossref + OpenAlex agree) |
| Algorithm implementable with enough detail | **Yes** — pseudocode for the stack policies, explicit cost model for the galloping routine, explicit t-policies |
| Scopus | Unverified (not claimed) |

## Other candidates considered — metadata
| Paper | Publisher / venue | DOI | Method text readable? |
|---|---|---|---|
| Parallel Multi-Deque Partition Dual-Deque Merge sorting (Ketchaya, Rattanatranurak) | Nature Portfolio, Sci. Reports 13 (2023) | 10.1038/s41598-023-33583-4 | yes |
| Malatya Vertex Coloring Algorithm (Yakut) | IEEE Access 13 (2025) | 10.1109/access.2025.3614296 | no (abstract only) |
| Malatya vertex-cover algorithm (Yakut, Öztemiz, Karcı) | Springer J. Supercomputing (2023) | 10.1007/s11227-023-05397-8 | no |
| Succinct and approximate greedy for Minimum Set Cover (Delgado, Ferrada, Navarro) | Elsevier J. Comput. Sci. (2024) | 10.1016/j.jocs.2024.102378 | no |
| Efficient semi-external DFS (Wan, Wang) | Elsevier Inf. Sci. (2022) | 10.1016/j.ins.2022.03.078 | yes (arXiv) |
