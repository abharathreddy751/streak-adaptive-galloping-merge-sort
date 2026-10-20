# PAPER_VERIFICATION — base paper metadata and evidence

Checked on 2026-10-19 using programmatic queries (Crossref REST API, OpenAlex API) plus arXiv and a Springer volume description retrieved through web search.

## Base paper

| Field | Value | Source / how checked |
|---|---|---|
| Title | Engineering Compact Data Structures for Rank and Select Queries on Bit Vectors | Crossref record, OpenAlex record, arXiv:2206.01149, paper PDF text |
| Author | Florian Kurpicz (Karlsruhe Institute of Technology, Germany) | Crossref; paper PDF header |
| Year | 2022 (published online 2022-11-01) | Crossref `published-online` = 2022-11-01; `published-print` = 2022 |
| Publisher | Springer International Publishing (Springer Nature) | Crossref `publisher`; OpenAlex host organization "Springer Science+Business Media" |
| Venue | String Processing and Information Retrieval (SPIRE 2022), 29th International Symposium, Concepción, Chile, 8–10 Nov 2022; Lecture Notes in Computer Science vol. 13617 | Crossref `container-title`; web-search retrieval of the Springer volume description |
| Pages | 257–272 (chapter 19) | Crossref |
| DOI | 10.1007/978-3-031-20643-6_19 | Crossref (volume DOI 10.1007/978-3-031-20643-6) |
| Series ISSN / ISBN | 0302-9743 (print), 1611-3349 (online); ISBN 9783031206429 / 9783031206436 | Crossref |
| Official link | https://doi.org/10.1007/978-3-031-20643-6_19 ; OA PDF https://link.springer.com/content/pdf/10.1007/978-3-031-20643-6_19.pdf (licence CC BY 4.0 per Crossref) | Crossref / OpenAlex |
| Preprint | arXiv:2206.01149 (v1 2 Jun 2022, v2 6 Nov 2022), same title/author | arXiv abstract page |
| Open-source code of authors | https://github.com/pasta-toolbox/bit_vector (GPLv3; **not used by us** — we re-implemented from the paper text) | Paper "Supplementary Material" |

## Peer review
The Springer volume description (retrieved via web search) states the volume's 23 full papers were "carefully reviewed and selected from 43 submissions". This is a refereed conference proceedings in a Springer series. This is a *conference* paper, not a journal article.

## Full text actually inspected
The OA PDF (CC BY 4.0) was downloaded by the fetch tool and converted with `pdftotext`; I read the text of Sections 1–3 (design of cs-poppy, flat-popcount, wide-popcount, select procedures) and the start of Section 4 (experimental setup, input classes). The remaining experimental results (Figures/Tables) were taken from a tool-generated summary and **not** independently re-derived; this project does not try to reproduce the paper's absolute numbers.

## Indexing status
- **Scopus: NOT VERIFIED.** I do not claim Scopus indexing. Scopus has no public lookup I could use programmatically; Scimago (HTTP 403) and the Springer series page (cookie-redirect) were inaccessible, and I did not bypass the access controls. LNCS is widely reported to be indexed in Scopus, but I did not confirm it for this volume and I make no such claim in this project.
- DBLP: not accessible (bot-protection page, not bypassed). Search results list the SPIRE 2022 volume as LNCS 13617, but I did not inspect the DBLP record.

## Verified vs unverified summary
- Verified (two independent registries agree + paper text): title, author, year, DOI, publisher, series, pages, licence, arXiv twin.
- Verified via secondary retrieval only: conference city/dates, editors, 23-of-43 acceptance statement.
- Unverified: Scopus indexing; official publisher landing page content (blocked for automated access).

## Other candidate papers — metadata (from Crossref, checked the same day)
| Paper | Publisher / venue | DOI |
|---|---|---|
| Binary Fuse Filters: Fast and Smaller Than Xor Filters (Graf, Lemire) | ACM, ACM J. Exp. Algorithmics 27, 2022 | 10.1145/3510449 |
| Prefix Filter (Even, Even, Morrison) | ACM (VLDB Endowment), PVLDB 15(7), 2022 | 10.14778/3523210.3523211 |
| Engineering In-place (Shared-memory) Sorting Algorithms (Axtmann, Witt, Ferizovic, Sanders) | ACM, ACM Trans. Parallel Computing 9(1), 2022 | 10.1145/3505286 |
| Practical Fully Dynamic Minimum Cut Algorithms (Henzinger, Noe, Schulz) | SIAM, ALENEX/SEA 2022 proceedings | 10.1137/1.9781611977042.2 |
