# DEMO_COMMANDS — copy-paste to reproduce the demonstration

> **Historical Stage 1 record.** For the final release use `README.md` and `docs/FINAL_DEMO_CHECKLIST.md`; the supported entry points are `bash scripts/run_all_tests.sh` and `bash stage3/tools/final_pass3.sh`. Two statements below are superseded: (1) section 0 describes the Stage 1 machine (i5-1235U, g++ 14.2.0 MinGW); the final release was built and tested with GCC 15.3.0 (MSYS2, Cygwin target) on an i5-13500HX; (2) section 6 (`scripts/final_audit.py`, "SUMMARY: 0 FAIL") was true for the Stage 1/2 source. On the final source it reports exactly **one** expected FAIL (check G: `src/natural_merge_sort.hpp` was deliberately changed in Stage 3 to add the exception guard); `stage3/tools/final_pass3.sh` checks that expectation and also runs the audit on a pristine-header copy (0 FAIL). The final release also builds a fifth test, `build/test_exception` (84,624 checks).

Run everything from the project root in **Git-Bash** (Windows) or any Linux shell. On Linux drop the `.exe` suffix. Timings differ between machines; comparison counts are exact and identical everywhere.

## 0. Environment used for the recorded outputs
g++ 14.2.0 (MinGW-w64), Windows 11 (26100), Intel Core i5-1235U, Python 3.9.13. Sanitizer runs: WSL Ubuntu, g++ 15.2.0.

## 1. Clean build (no CMake needed)
```bash
rm -rf build && bash scripts/build.sh
```
Expected tail: `[build] test_base`, `[build] test_all`, `[build] bench_sort`, `[build] demo_sort`, `[build] done`, no warnings.

## 2. Tests
```bash
./build/test_base.exe     # base paper algorithm only
./build/test_all.exe      # base + proposed extension
./build/test_sag.exe      # extension-specific tests
./build/test_sagt.exe     # corrective-round extensions (SAG-T v1, SAG-B, SAG-C)
```
Expected last lines (actual from the recorded run; counts are deterministic):
```
test_sort: 1502829 checks passed, 0 failed
test_sort: 2100389 checks passed, 0 failed
test_sag: 265609 checks passed, 0 failed
test_sagt: 760686 checks passed, 0 failed
```
`test_base`/`test_all` also print the sweeps they ran (e.g. `44100 pairs x 8 configs`, `29524 arrays x 3 minrun x 8 configs`) and the measured Theorem-51 constant (`max C_obs = 4.097`). Saved logs: `results/outputs/test_*_windows_gcc14.txt`.

Sanitizers (optional, needs WSL; Git-Bash needs `MSYS_NO_PATHCONV=1` so the `/mnt/...` path is not rewritten):
```bash
MSYS_NO_PATHCONV=1 wsl.exe -e bash "/mnt/d/new dsa project/scripts/sanitize_wsl.sh"
```
Expected: the same three lines of check counts, 0 failed, then `sanitizer run finished OK` (about 2 minutes). Saved: `results/outputs/sanitizer_test_*.txt`.

## 3. The demonstration program
```bash
./build/demo_sort.exe
```
Expected output (actual; deterministic):
```
== Part 1: 24 elements, minrun = 4, PowerSort merge order ==
input : 5 6 7 8 9 1 1 1 2 2 2 2 3 3 3 9 9 9 1 2 3 4 5 6
sorted: 1 1 1 1 2 2 2 2 2 3 3 3 3 4 5 5 6 6 7 8 9 9 9 9

policy merge (a+b)    t at start blocks   comparisons in merge
base      5 + 13      25        2        16
base     18 + 6       25        10       22
SAG       5 + 13      25        2        16
SAG      18 + 6       25        10       22
totals: base 61 comparisons (run detection 23 + merges 38), SAG 61 (identical here: blocks are short)

== Part 2: comparisons per element, n = 262144 (deterministic counts, minrun = 32) ==
input                     H     H* |    naive     base      SAG | stable_sort    topdown
random permutation    16.67  16.67 |   16.719   16.719   16.719 |     17.809     16.734
few(3) values         16.28   1.58 |   14.454    8.272    6.776 |     15.583     14.406
few(64) values        16.67   6.00 |   16.639   14.577   11.166 |     17.744     16.658
zipf(1024) values     16.67   7.50 |   16.705   14.929   11.823 |     17.902     16.732
16 sorted runs         3.49  16.63 |    4.866    4.866    4.769 |     12.021     11.339

all outputs equal the reference stable sort: YES
```
Reading it: "base" is the paper's polylogarithmic galloping, "SAG" the proposed extension; they tie where blocks are short and SAG wins where blocks are long. **Comparisons are not time** — step 4.

## 4. One benchmark cell (heavy-comparator regime, ~10 s)
```bash
./build/bench_sort.exe --family few --param 16 --n 131072 --seed 5 --reps 7 --elem heavy \
    --variants naive,base,base_b,sag --cell DEMO_heavy_few16 --csv build/demo_cell.csv --pin auto
python scripts/analyze.py build/demo_cell.csv --no-criteria
```
Expected stderr: `pinned to logical CPU 0 (...)` (the harness reports whether pinning worked) and `done rc=0 (H=15.660 H*=4.000 runs=53790)`. Expected table row (timings vary, ratios should be close):
```
| DEMO_heavy_few16 | heavy | few(16) | 2^17 | 0.24 | low-H* | 0.759 | 0.798 [0.768, 0.857] | 1.019 [0.987, 1.032] | 1.256 | 264.7 |
```
= SAG uses 0.759× the comparisons of the base and takes 0.798× the time (95 % CI 0.77–0.86); the A/A control (`base_b`/`base`, identical code) is 1.02, i.e. the noise floor.

## 5. Re-derive the held-out results from the raw data (no benchmarking needed)
```bash
python scripts/analyze.py results/raw/heldout.csv            # -> results/outputs/summary_heldout.md   (ends with: PARTIAL SUCCESS (conditional on comparator cost))
python scripts/analyze.py results/raw/heldout2.csv --var sagt --round2   # -> summary_heldout2.md (ends with: CORRECTIVE PARTIAL)
python scripts/make_figures.py results/raw/heldout.csv results/raw/heldout2.csv   # -> results/figures/
```
The verdict block at the end of the summary is computed from `results/raw/heldout.csv` only. To re-run the measurements themselves (about 25 minutes, idle machine): `python scripts/run_bench.py --manifest heldout` after deleting (or moving) `results/raw/heldout.csv`; the run resumes automatically if interrupted.

## 6. Everything at once (about 2 minutes)
```bash
python scripts/final_audit.py
```
Expected: 24 lines `[PASS]`/`[INFO]` and `SUMMARY: 0 FAIL` (clean rebuild with the documented command, the four test binaries, demo reproduction, re-derivation of both summaries, manifest digests, byte-identity of the frozen protocol text, CSV completeness). The recorded run is `results/outputs/final_audit.txt`.
