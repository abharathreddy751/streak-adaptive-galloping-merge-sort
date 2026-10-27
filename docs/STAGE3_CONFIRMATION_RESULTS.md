# STAGE3_CONFIRMATION_RESULTS - one fresh confirmatory experiment, frozen protocol, SAG as primary candidate

**Verdict under the frozen rule (`docs/STAGE3_CONFIRMATION_PLAN.md` section 8): CONFIRMATION SUCCESS for SAG (`SagTT<64,true>`).**
P1 correctness/completeness PASS; P2 gain PASS (**12 of 16** gain-set cells >= 10 % faster, threshold 11 - a thin margin); P3 guard PASS (no severe regression in any of 50 cells); P4 comparisons PASS, and the independent Stage-3 checker re-derived counts, ratios, CIs, manifest digest, run log, hashes and the verdict from the raw rows (`stage3/audit/audit_stage3_output.txt`).
**What this licenses:** SAG is a *conditional* improvement for **expensive-key, low-entropy synthetic string workloads** on this machine and compiler. It is **not** a universally faster sort: cheap short keys gain little or lose, and several expensive-key cells missed the 10 % bar. Stage-1/2 verdicts are unchanged and SAG-C's Stage-2 result is unchanged.

## 1. Protocol (frozen before the run)
Plan hash and all source/binary/data hashes: `stage3/string3/freeze_record.txt` (written 23:23:13; held-out run 23:23:27 - 23:36:59, one run, 50 cells, every process exit 0). 3 key styles (`S` short ~10 chars, `M` hierarchical paths ~55-65 chars, `L` long hierarchical paths ~210-250 chars with variable shared prefixes) x 8 structures (`perm`, `few(2)`, `few(16)`, `few(256)`, `zipf(1024)`, `mixture(64)`, `runs(8)`, `nearly(0.01)`) x sizes {2^17, 2^19} = 48 synthetic cells, + 2 real-string cells (19,611 file names from a local MSYS2 installation: names only, `natural` order and `shuffled`). 8 variants, 15 / 11 / 31 repetitions, randomised variant order, A/A control, pinned P-core (50/50 cells verified 1000/1000), `g++ 15.3.0 -O2`, noexcept comparator, patched sorter header. Gain set G = low-entropy (`H* <= 0.5 log2 n`) AND expensive-key (`cost_B >= 32` bytes), fixed by an un-timed structural pass before the freeze: M and L styles x `few(2)`, `few(16)`, `few(256)`, `zipf(1024)` x both sizes = **16 cells**.
Raw data `stage3/results/s3_heldout.csv` (5,888 records: 400 counting + 5,488 timed), run log `s3_heldout_runlog.txt`, table `summary_s3_heldout.md`. Development data (harness check only): `s3_dev.csv`.

## 2. Decision-rule evaluation (SAG, primary)
| criterion | result |
|---|---|
| P1 all cells/variants/repetitions present; every output equals the independent stable reference incl. stability; exit codes 0 | **PASS** (5,888 / 5,888 rows `ok = 1`, 50 / 50 processes exit 0) |
| P2 >= 10 % faster (ratio <= 0.90 and CI upper < 1) in >= 11 of 16 gain-set cells | **PASS, 12 of 16** |
| P3 no severe regression (ratio >= 1.10 and CI lower > 1) in any cell | **PASS** (worst: `S3H_S_zipf1024_n17` 1.076 [1.053, 1.085]; no cell >= 1.10) |
| P4 sorter-counted comparisons <= 1.03 x base in every cell; independent verification | **PASS** (max 1.000); independent checker: 0 mismatches |
Gain-set cells that **missed** the 10 % bar (reported, not hidden): `L few(2)` 2^19 = 0.928 [0.911, 0.937]; `M few(2)` 2^17 = 0.916 [0.880, 0.956]; `M few(2)` 2^19 = 0.934 [0.926, 0.943]; `M zipf(1024)` 2^17 = 0.943 [0.923, 0.949]. (`L few(2)` 2^17 = 0.895 [0.888, 0.906] passed narrowly.) A/A noise floor: largest |median - 1| = 0.021; identical code was never flagged as a win or a severe regression. Replacing the bootstrap CI by an exact sign-interval changes no cell's classification (0 of 50).

## 3. Results by key class (SAG / base time ratio; CI in `summary_s3_heldout.md`)
| class | cells | SAG time ratio range | comparisons SAG/base |
|---|---|---|---|
| gain set, style `L` (long keys) | 8 | 0.660 - 0.928 (7 of 8 wins) | 0.755 - 0.851 |
| gain set, style `M` (medium keys) | 8 | 0.827 - 0.943 (5 of 8 wins) | 0.755 - 0.852 |
| cheap-key low-entropy `S` | 8 | 0.938 - 1.076 | 0.755 - 0.853 |
| `nearly(0.01)` (high-entropy control) | 6 | `L` 0.602-0.626, `M` 0.788-0.799, `S` 0.832-0.838 | 0.517 - 0.541 |
| `perm` / `runs(8)` / `mixture(64)` controls | 18 | 0.871 - 1.044 | 0.891 - 1.000 |
| real strings, `natural` (low-entropy by the rule, `cost_B` 30.8 = cheap) | 1 | 0.957 [0.944, 0.966] | 0.845 |
| real strings, `shuffled` | 1 | 1.007 [1.005, 1.011] | 1.000 |
Reading: the gain follows the **cost of a comparison** (base wall time per comparison: `L` 17-71 ns, `M` 11-28 ns, `S` 7-17 ns). For long variable-prefix keys SAG saves 7 %-34 % of the time on low-entropy data and 37 %-40 % on nearly-sorted data; for medium keys the saving is 6 %-17 % (low entropy) and 20 % (nearly sorted); for cheap 10-character keys the comparison saving (15-25 %) does not translate into time (0.94-1.08) - on `zipf(1024)` with cheap keys SAG is 6-8 % *slower*. SAG never uses more comparisons than the base (0.517-1.000), and is neutral on random permutations (0.990-1.044).
`naive` merging is 1.0-1.9x slower than the paper's galloping on low-entropy long keys; `std::stable_sort` is slower than `base` in 24 of the 32 expensive-key synthetic cells (up to 2.5x); on cheap `S` keys it is 0.64-0.88x of `base` for `few`/`zipf`/`mixture`/`perm` but 1.6-1.75x slower on `nearly`/`runs` (reference medians, no CIs computed).

## 4. Secondary diagnostic (frozen SAG-C, no verdict)
SAG-C (frozen Stage-2 constants) had 12 of 16 gain-set wins as well, but a severe regression on the real-string cell `FILE natural` (1.276 [1.266, 1.288]; its calibration costs about 16,384 comparisons on a 19,611-element sort) - consistent with Stage 2, where its guard also failed. Not used for any claim.

## 5. Limitations
Synthetic strings except two small real-string cells (n = 19,611: 2-6 ms sorts); one machine, one compiler, one session; the 32-byte cost threshold and the gain-set definition are design choices fixed before timing - with them, cheap-key cells are guarded but not required to gain, so the success statement is *conditional on comparison cost*; margins are thin (12 of 16 vs the 11 needed; 4 expensive-key cells under 10 %); `few(2)` and `M zipf(1024)` 2^17 did not reach 10 %; results do not transfer to other string workloads or machines without testing; the Stage-3 numbers are not comparable in absolute terms with Stages 1-2; no sanitizer runtime was available (hardened builds only).

## 6. v4 audit addendum (2026-10-27; the sections above are unchanged)
An independent recomputation of the numbers of this report from `stage3/results/s3_heldout.csv` (`scripts/audit_recompute.py`, `docs/AUDIT_V4_CODE.md`) agreed with all of them except one range: the comparison ratio SAG/base of the six `nearly(0.01)` cells is **0.517-0.547**, not 0.517-0.541 as printed in the table of section 3 (the largest value is `S3H_L_nearly0p01_n17` = 0.547, see `stage3/results/summary_s3_heldout.md`). The criteria outcome (P1-P4), the 12 of 16 gain cells and every other number are unaffected. The consolidated `docs/FINAL_PROJECT_REPORT.md` carries the corrected range.
