# Summary of `string_dev.csv` (STAGE-2 string-comparator experiment)

Cells: 16.  Statistic: median over repetitions of the paired ratio variant/base (bootstrap 95 % CI, 10000 resamples, seed 12345).  Primary extension: `sagc` (SAG-C); `sag` = round-1 SAG, descriptive.

| cell | H*/log2 n | class | cmp SAG-C/base (incl. calibration) | cmp SAG/base | time SAG-C/base [95% CI] | time SAG/base [95% CI] | A/A base_b/base [95% CI] | naive/base | tim_base/base | stdstable/base | topdown/base | base ns/elem | base wall-ns per cmp | SAG-C measured c (ns) |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| STRDEV_few32_n16 | 0.31 | low-H* | 0.701 | 0.772 | 0.851 [0.811, 0.859] | 0.806 [0.779, 0.827] | 1.007 [0.982, 1.020] | 1.199 | 0.989 | 1.038 | 1.085 | 297.6 | 22.6 | 55.5 |
| STRDEV_few32_n18 | 0.28 | low-H* | 0.671 | 0.760 | 0.793 [0.786, 0.799] | 0.790 [0.777, 0.829] | 0.994 [0.973, 1.015] | 1.562 | 0.975 | 1.948 | 1.455 | 323.8 | 24.2 | 139.7 |
| STRDEV_few3_n16 | 0.10 | low-H* | 0.718 | 0.821 | 1.004 [0.936, 1.030] | 0.877 [0.860, 1.022] | 1.029 [1.006, 1.079] | 1.630 | 1.090 | 1.422 | 1.561 | 165.1 | 20.0 | 111.0 |
| STRDEV_few3_n18 | 0.09 | low-H* | 0.694 | 0.819 | 0.910 [0.878, 0.921] | 0.888 [0.855, 0.909] | 0.999 [0.969, 1.021] | 1.867 | 0.978 | 2.487 | 1.792 | 175.3 | 21.2 | 149.8 |
| STRDEV_few512_n16 | 0.56 | high-H* | 0.880 | 0.946 | 0.923 [0.919, 0.971] | 0.943 [0.861, 0.951] | 1.013 [0.984, 1.042] | 1.007 | 1.018 | 0.874 | 0.900 | 374.6 | 25.5 | 59.5 |
| STRDEV_few512_n18 | 0.50 | low-H* | 0.773 | 0.844 | 0.739 [0.691, 0.765] | 0.736 [0.672, 0.785] | 1.022 [0.944, 1.124] | 1.025 | 0.990 | 1.180 | 0.993 | 622.7 | 37.3 | 150.0 |
| STRDEV_mixture16_n16 | 0.61 | high-H* | 0.866 | 0.893 | 0.977 [0.850, 1.000] | 0.941 [0.852, 0.952] | 0.980 [0.929, 1.025] | 1.055 | 0.983 | 1.037 | 1.030 | 327.0 | 25.6 | 110.4 |
| STRDEV_mixture16_n18 | 0.60 | high-H* | 0.857 | 0.895 | 0.966 [0.951, 0.980] | 0.950 [0.937, 0.987] | 1.000 [0.979, 1.031] | 1.205 | 0.976 | 1.547 | 1.195 | 406.2 | 29.2 | 148.6 |
| STRDEV_nearly0p05_n16 | 0.75 | high-H* | 0.636 | 0.668 | 0.874 [0.828, 0.903] | 0.792 [0.750, 0.843] | 0.987 [0.964, 1.025] | 1.158 | 1.021 | 1.097 | 1.118 | 227.1 | 20.7 | 65.2 |
| STRDEV_nearly0p05_n18 | 0.78 | high-H* | 0.603 | 0.648 | 0.812 [0.802, 0.842] | 0.807 [0.785, 0.821] | 0.983 [0.975, 1.021] | 1.206 | 1.022 | 1.447 | 1.144 | 275.6 | 23.8 | 127.5 |
| STRDEV_perm0_n16 | 0.92 | high-H* | 1.018 | 1.000 | 1.076 [1.031, 1.122] | 1.002 [0.969, 1.022] | 0.967 [0.954, 1.037] | 0.989 | 1.005 | 0.902 | 0.880 | 434.3 | 29.5 | 64.2 |
| STRDEV_perm0_n18 | 0.93 | high-H* | 1.006 | 1.000 | 1.049 [1.029, 1.081] | 1.008 [0.999, 1.016] | 1.005 [0.991, 1.024] | 0.989 | 0.992 | 1.152 | 0.942 | 580.1 | 34.7 | 156.4 |
| STRDEV_runs16_n16 | 0.91 | high-H* | 1.036 | 1.000 | 1.365 [1.104, 1.437] | 1.120 [1.030, 1.273] | 1.075 [0.895, 1.179] | 0.994 | 1.272 | 1.925 | 2.072 | 91.5 | 19.5 | 50.4 |
| STRDEV_runs16_n18 | 0.92 | high-H* | 0.978 | 0.972 | 1.132 [1.115, 1.142] | 0.983 [0.976, 1.049] | 1.008 [0.953, 1.022] | 0.965 | 1.107 | 2.065 | 1.774 | 118.2 | 23.9 | 128.1 |
| STRDEV_zipf256_n16 | 0.39 | low-H* | 0.790 | 0.791 | 0.845 [0.832, 0.873] | 0.834 [0.808, 0.887] | 1.021 [0.932, 1.159] | 1.091 | 1.008 | 0.978 | 0.957 | 378.5 | 28.2 | 83.6 |
| STRDEV_zipf256_n18 | 0.35 | low-H* | 0.735 | 0.749 | 0.781 [0.746, 0.794] | 0.755 [0.716, 0.776] | 1.006 [0.960, 1.033] | 1.290 | 1.006 | 1.573 | 1.231 | 415.3 | 29.0 | 141.5 |

## Frozen decision rule (docs/STAGE2_REAL_COMPARATOR_PLAN.md section 8)

| criterion | SAG-C (PRIMARY) | SAG (descriptive) |
|---|---|---|
| P1 correctness: every output verified (idx sequence == reference AND order+stability scan) | **PASS** failures: none | same data |
| P2 gain: >= 10 % faster (median ratio <= 0.90 AND CI upper < 1) in >= 2/3 of the low-H* cells | **PASS** 5 of 7 (need 5) | pass: 6 of 7 |
| P3 guard: no severe regression (ratio >= 1.10 AND CI lower > 1) in any control (high-H*) cell | **FAIL** ['STRDEV_runs16_n16', 'STRDEV_runs16_n18'] | fail: ['STRDEV_runs16_n16'] |
| P3b guard, all cells (low-H* too): no severe regression | FAIL ['STRDEV_runs16_n16', 'STRDEV_runs16_n18'] | ['STRDEV_runs16_n16'] |
| P4 comparisons counted by the sorter never > 1.03 x base (as R2-D; SAG-C calibration calls are reported separately in the table) | **PASS** none | pass: none |
| A/A noise: largest abs(median(base_b/base)-1) | 0.075 | |

**Verdict under the frozen rule - PRIMARY (SAG-C): GAIN WITHOUT SAFETY (guard failed)**

**Verdict under the same rule - secondary (SAG; own verdict, not pooled): GAIN WITHOUT SAFETY (guard failed)**
