# STAGE-2 paper verification (2026-10-20)

Method: official publisher page opened in the built-in browser (ordinary page load, no login, no bypass), Crossref REST API, OpenAlex API, the
arXiv v3 PDF (downloaded, SHA-256 in `arxiv_2012.03996v3.pdf.sha256`, text via `pdftotext`, key pages rendered to PNG and read visually).

## Verified against the publisher record (link.springer.com/article/10.1007/s00453-024-01285-6)
| field | value on the Springer page | Crossref | OpenAlex |
|---|---|---|---|
| title | Galloping in Fast-Growth Natural Merge Sorts | same | same |
| authors | Elahe Ghasemi, Vincent Jugé, Ghazal Khalighinejad, Helia Yazdanyar | same 4, same order | same 4, same order |
| journal / publisher | Algorithmica / Springer Nature | Algorithmica / "Springer Science and Business Media LLC" | Algorithmica |
| volume, pages | vol. 87, pp. 242-291 (2025) | vol 87, issue 2, pp 242-291 | vol 87, issue 2, 242-291 |
| dates | Published 09 December 2024 | online 2024-12-09, print 2025-02 | 2024-12-09 (year 2024) |
| DOI | 10.1007/s00453-024-01285-6 | same | same |
| type | Article | journal-article | article, `is_retracted: false` |

Both citation years are legitimate: **online 2024, issue year 2025** (cite "Algorithmica 87(2):242-291, 2025").

### New finding (missing from the stage-1 record): a published correction
The Springer page states "A Correction to this article was published on 29 January 2025"; Crossref lists the correction as
DOI **10.1007/s00453-025-01297-w** (*Correction: Galloping in Fast-Growth Natural Merge Sorts*, Algorithmica 87(5):812, online 2025-01-29,
type `correction`). The change history on the article page says: "The original version of the article is corrected: Affiliation of
coauthor Helia Yazdanyar is corrected" (30 Jan 2025). So the correction concerns an **author affiliation only**; nothing in the page
indicates a change to the algorithms or theorems. The stage-1 record listed Yazdanyar's affiliation as "CNRS" (Crossref/OpenAlex era data);
the affiliations are irrelevant to the project's claims, but the correction should be cited.

## Indexing
| claim | status | evidence |
|---|---|---|
| Journal *Algorithmica* is listed as "Abstracted and indexed in ... SCOPUS" | **verified at journal level, from the publisher's own journal page** (link.springer.com/journal/453, "Abstracted and indexed in": ... SCImago, SCOPUS, Science Citation Index Expanded (SCIE), zbMATH, DBLP, ...) | page text captured 2026-10-20 |
| Journal is covered by Scopus according to Elsevier's Source list / Scopus itself | **NOT verified** | Scopus requires an institutional session; the Scimago page answered with a Cloudflare bot challenge ("Just a moment...") and was not bypassed |
| **This individual article** is indexed in Scopus | **NOT verified** (cannot be, without Scopus access) | journal-level coverage does not prove article-level indexing; a 2024-12 article may or may not yet be indexed |
| DBLP record | not retrievable (Anubis bot protection, access denied; not bypassed) | |
Conclusion: say "published in *Algorithmica* (Springer), a journal the publisher lists as indexed in Scopus and SCIE; article-level Scopus indexing was not verified."

## Relationship of the implementation to the paper (checked against the arXiv v3 text, not the paywalled version of record)
* **TimSort stack rule (paper Algorithm 2, page 15 rendered):** lines 2-9 read `h>=3 and r_{h-2} < r_h` -> merge R_{h-2},R_{h-1}; `h>=2 and r_{h-1} <= r_h` -> merge R_{h-1},R_h; `h>=3 and r_{h-2} <= r_{h-1}+r_h` -> merge R_{h-1},R_h; `h>=4 and r_{h-3} <= r_{h-2}+r_{h-1}` -> merge R_{h-1},R_h; push a run; final loop merges the top two. `src/natural_merge_sort.hpp::run_tim` matches case for case (`<`, `<=`, `<=`, `<=`). The stage-1 note that these were "reconstructed" is **resolved: the reconstruction equals the printed algorithm.**
* **Polylogarithmic parameter (Definition 47, page 29 rendered):** "t = ceil(log2(a+b))^2". The implementation's `PolylogT` computes exactly `ceil_log2(a+b)^2`. The stage-1 "interpretation" caveat is **resolved**.
* **PowerSort** is analysed in the paper (Theorem 10: tight middle-growth), but its pseudo-code is not reproduced in the paper; the implementation follows Munro & Wild's node-power definition (independent source, not re-verified here).
* **Still true (do not claim otherwise):** this is an *independent implementation of the paper's method, not the authors' code*; the paper's own Algorithm 6 (TimSort's real merging routine with `min_gallop` state) is **not** implemented; the adversarial constructions (Props. 42-43) and all proofs are **not** reproduced; only the arXiv v3 preprint was read, not the Springer version of record (paywalled: "preview of subscription content").
* The paper itself contains **no experiments**; there is nothing to reproduce numerically.
