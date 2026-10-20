# Summary of `string_heldout.csv` (STAGE-2 string-comparator experiment)

Cells: 18.  Statistic: median over repetitions of the paired ratio variant/base (bootstrap 95 % CI, 10000 resamples, seed 12345).  Primary extension: `sagc` (SAG-C); `sag` = round-1 SAG, descriptive.

| cell | H*/log2 n | class | cmp SAG-C/base (incl. calibration) | cmp SAG/base | time SAG-C/base [95% CI] | time SAG/base [95% CI] | A/A base_b/base [95% CI] | naive/base | tim_base/base | stdstable/base | topdown/base | base ns/elem | base wall-ns per cmp | SAG-C measured c (ns) |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| STRHELD_few16_n17 | 0.24 | low-H* | 0.668 | 0.759 | 0.823 [0.796, 0.847] | 0.815 [0.808, 0.839] | 0.998 [0.964, 1.024] | 1.574 | 0.969 | 1.802 | 1.470 | 319.4 | 26.5 | 124.8 |
| STRHELD_few16_n19 | 0.21 | low-H* | 0.657 | 0.755 | 0.812 [0.716, 0.883] | 0.794 [0.737, 0.836] | 0.993 [0.952, 1.003] | 1.968 | 0.971 | 2.324 | 1.825 | 312.9 | 25.8 | 236.2 |
| STRHELD_few256_n17 | 0.47 | low-H* | 0.762 | 0.834 | 0.725 [0.704, 0.755] | 0.725 [0.714, 0.744] | 0.976 [0.921, 0.995] | 0.987 | 0.985 | 1.049 | 0.924 | 500.1 | 31.8 | 117.9 |
| STRHELD_few256_n19 | 0.42 | low-H* | 0.714 | 0.786 | 0.630 [0.617, 0.639] | 0.650 [0.636, 0.657] | 0.992 [0.974, 1.004] | 1.153 | 0.978 | 1.398 | 1.096 | 546.1 | 32.6 | 178.2 |
| STRHELD_few2_n17 | 0.06 | low-H* | 0.761 | 0.851 | 0.979 [0.944, 0.998] | 0.907 [0.896, 0.958] | 1.011 [0.985, 1.050] | 1.827 | 1.019 | 2.193 | 1.769 | 149.9 | 21.1 | 123.7 |
| STRHELD_few2_n19 | 0.05 | low-H* | 0.748 | 0.851 | 0.921 [0.895, 0.951] | 0.935 [0.926, 0.952] | 0.998 [0.984, 1.042] | 2.054 | 1.007 | 2.615 | 2.045 | 174.4 | 24.6 | 185.6 |
| STRHELD_few4096_n17 | 0.70 | high-H* | 0.972 | 1.000 | 0.975 [0.961, 1.000] | 1.012 [0.986, 1.036] | 0.999 [0.983, 1.020] | 0.990 | 0.990 | 1.038 | 0.921 | 480.6 | 30.6 | 213.3 |
| STRHELD_few4096_n19 | 0.63 | high-H* | 0.887 | 0.955 | 0.805 [0.798, 0.819] | 0.918 [0.910, 0.931] | 1.011 [0.998, 1.012] | 1.017 | 1.001 | 1.210 | 0.968 | 639.6 | 36.1 | 175.8 |
| STRHELD_mixture64_n17 | 0.67 | high-H* | 0.863 | 0.897 | 0.935 [0.928, 0.951] | 0.892 [0.878, 0.913] | 0.990 [0.966, 1.022] | 1.034 | 0.974 | 1.202 | 1.016 | 391.1 | 27.1 | 105.5 |
| STRHELD_mixture64_n19 | 0.65 | high-H* | 0.855 | 0.891 | 0.931 [0.911, 0.951] | 0.912 [0.892, 0.933] | 1.009 [0.988, 1.015] | 1.167 | 0.996 | 1.496 | 1.180 | 482.6 | 30.8 | 227.6 |
| STRHELD_nearly0p01_n17 | 0.63 | high-H* | 0.515 | 0.534 | 0.745 [0.728, 0.750] | 0.667 [0.633, 0.683] | 0.994 [0.977, 1.016] | 1.618 | 1.015 | 2.201 | 1.849 | 126.9 | 20.2 | 111.7 |
| STRHELD_nearly0p01_n19 | 0.67 | high-H* | 0.484 | 0.518 | 0.674 [0.643, 0.715] | 0.658 [0.627, 0.678] | 1.034 [0.931, 1.071] | 1.780 | 1.003 | 2.312 | 1.921 | 150.2 | 22.8 | 169.1 |
| STRHELD_perm0_n17 | 0.92 | high-H* | 1.011 | 1.000 | 1.039 [0.997, 1.075] | 0.994 [0.980, 1.021] | 0.984 [0.977, 1.009] | 0.983 | 0.987 | 1.058 | 0.920 | 629.2 | 40.0 | 159.8 |
| STRHELD_perm0_n19 | 0.93 | high-H* | 1.005 | 1.000 | 1.037 [1.019, 1.060] | 1.027 [1.014, 1.074] | 1.005 [0.985, 1.019] | 0.995 | 1.002 | 1.127 | 0.948 | 823.8 | 46.5 | 190.4 |
| STRHELD_runs64_n17 | 0.92 | high-H* | 1.007 | 0.993 | 1.114 [1.104, 1.150] | 1.007 [0.990, 1.043] | 0.991 [0.984, 1.035] | 0.965 | 0.995 | 1.495 | 1.469 | 148.4 | 22.2 | 123.6 |
| STRHELD_runs64_n19 | 0.93 | high-H* | 0.987 | 0.988 | 1.092 [1.061, 1.112] | 1.014 [0.983, 1.045] | 0.996 [0.961, 1.005] | 0.992 | 1.009 | 1.749 | 1.544 | 155.5 | 23.6 | 169.4 |
| STRHELD_zipf1024_n17 | 0.44 | low-H* | 0.808 | 0.815 | 0.869 [0.848, 0.888] | 0.853 [0.834, 0.867] | 0.990 [0.986, 1.010] | 1.134 | 0.988 | 1.205 | 1.058 | 406.9 | 28.3 | 129.2 |
| STRHELD_zipf1024_n19 | 0.40 | low-H* | 0.760 | 0.773 | 0.745 [0.737, 0.754] | 0.731 [0.727, 0.747] | 0.992 [0.986, 1.006] | 1.267 | 0.994 | 1.533 | 1.197 | 484.5 | 31.5 | 221.9 |

## Frozen decision rule (docs/STAGE2_REAL_COMPARATOR_PLAN.md section 8)

| criterion | SAG-C (PRIMARY) | SAG (descriptive) |
|---|---|---|
| P1 correctness: every output verified (idx sequence == reference AND order+stability scan) | **PASS** failures: none | same data |
| P2 gain: >= 10 % faster (median ratio <= 0.90 AND CI upper < 1) in >= 2/3 of the low-H* cells | **PASS** 6 of 8 (need 6) | pass: 6 of 8 |
| P3 guard: no severe regression (ratio >= 1.10 AND CI lower > 1) in any control (high-H*) cell | **FAIL** ['STRHELD_runs64_n17'] | pass: none |
| P3b guard, all cells (low-H* too): no severe regression | FAIL ['STRHELD_runs64_n17'] | none |
| P4 comparisons counted by the sorter never > 1.03 x base (as R2-D; SAG-C calibration calls are reported separately in the table) | **PASS** none | pass: none |
| A/A noise: largest abs(median(base_b/base)-1) | 0.034 | |

**Verdict under the frozen rule - PRIMARY (SAG-C): GAIN WITHOUT SAFETY (guard failed)**

**Verdict under the same rule - secondary (SAG; own verdict, not pooled): SUCCESS**
