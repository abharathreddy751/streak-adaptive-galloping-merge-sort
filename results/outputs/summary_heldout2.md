# Summary of `heldout2.csv`

Cells: 54.  Statistic: median over repetitions of the paired ratio variant/base (bootstrap 95 % CI, 10000 resamples, seed 12345).

Variant under test in the 'SAG' columns: `sagt`; the round-1 SAG is shown as 'round-1 SAG'.

| cell | regime | family | n | H*/log2 n | class | cmp SAG/base | time SAG/base [95% CI] | round-1 SAG time/base | A/A base_b/base [95% CI] | time naive/base | base ns/elem |
|---|---|---|---|---|---|---|---|---|---|---|---|
| HELD2_heavy_few16_n17 | heavy | few(16) | 2^17 | 0.24 | low-H* | 0.668 | 0.734 [0.710, 0.758] | 0.792 | 0.999 [0.969, 1.008] | 1.205 | 281.7 |
| HELD2_heavy_few16_n20 | heavy | few(16) | 2^20 | 0.20 | low-H* | 0.656 | 0.725 [0.719, 0.738] | 0.795 | 1.003 [0.986, 1.017] | 1.422 | 276.8 |
| HELD2_heavy_few256_n17 | heavy | few(256) | 2^17 | 0.47 | low-H* | 0.762 | 0.811 [0.806, 0.836] | 0.874 | 1.013 [0.995, 1.023] | 0.959 | 344.1 |
| HELD2_heavy_few256_n20 | heavy | few(256) | 2^20 | 0.40 | low-H* | 0.704 | 0.746 [0.741, 0.776] | 0.807 | 0.993 [0.973, 1.006] | 1.050 | 376.7 |
| HELD2_heavy_few2_n17 | heavy | few(2) | 2^17 | 0.06 | low-H* | 0.744 | 0.819 [0.797, 0.828] | 0.886 | 1.028 [1.011, 1.042] | 1.685 | 152.3 |
| HELD2_heavy_few2_n20 | heavy | few(2) | 2^20 | 0.05 | low-H* | 0.744 | 0.790 [0.770, 0.796] | 0.857 | 1.004 [0.983, 1.012] | 1.956 | 154.1 |
| HELD2_heavy_few4096_n17 | heavy | few(4096) | 2^17 | 0.70 | high-H* | 0.970 | 0.998 [0.985, 1.008] | 1.008 | 0.988 [0.977, 1.005] | 0.948 | 375.7 |
| HELD2_heavy_few4096_n20 | heavy | few(4096) | 2^20 | 0.60 | high-H* | 0.843 | 0.881 [0.868, 0.887] | 0.939 | 0.995 [0.980, 1.019] | 0.942 | 420.9 |
| HELD2_heavy_mixture64_n17 | heavy | mixture(64) | 2^17 | 0.67 | high-H* | 0.858 | 0.907 [0.881, 0.931] | 0.932 | 1.000 [0.992, 1.038] | 0.984 | 327.5 |
| HELD2_heavy_mixture64_n20 | heavy | mixture(64) | 2^20 | 0.64 | high-H* | 0.854 | 0.900 [0.874, 0.919] | 0.919 | 1.008 [0.994, 1.020] | 1.031 | 383.0 |
| HELD2_heavy_nearly0p01_n17 | heavy | nearly(0.01) | 2^17 | 0.63 | high-H* | 0.500 | 0.558 [0.550, 0.571] | 0.590 | 1.004 [0.990, 1.020] | 1.843 | 131.9 |
| HELD2_heavy_nearly0p01_n20 | heavy | nearly(0.01) | 2^20 | 0.69 | high-H* | 0.474 | 0.529 [0.508, 0.569] | 0.563 | 1.021 [0.968, 1.034] | 2.114 | 139.1 |
| HELD2_heavy_perm0_n17 | heavy | perm | 2^17 | 0.92 | high-H* | 1.001 | 1.022 [0.973, 1.058] | 1.000 | 1.006 [0.985, 1.027] | 0.954 | 633.4 |
| HELD2_heavy_perm0_n20 | heavy | perm | 2^20 | 0.93 | high-H* | 1.001 | 1.000 [0.980, 1.023] | 0.998 | 0.996 [0.985, 1.024] | 0.924 | 438.4 |
| HELD2_heavy_runs64_n17 | heavy | runs(64) | 2^17 | 0.92 | high-H* | 0.980 | 1.010 [0.991, 1.029] | 0.990 | 0.985 [0.959, 1.018] | 0.934 | 151.3 |
| HELD2_heavy_runs64_n20 | heavy | runs(64) | 2^20 | 0.93 | high-H* | 0.987 | 1.031 [0.999, 1.047] | 1.014 | 0.995 [0.980, 1.021] | 0.939 | 153.0 |
| HELD2_heavy_zipf1024_n17 | heavy | zipf(1024) | 2^17 | 0.44 | low-H* | 0.800 | 0.863 [0.847, 0.886] | 0.886 | 1.013 [0.997, 1.030] | 1.046 | 319.4 |
| HELD2_heavy_zipf1024_n20 | heavy | zipf(1024) | 2^20 | 0.38 | low-H* | 0.741 | 0.799 [0.775, 0.813] | 0.834 | 1.003 [0.982, 1.018] | 1.130 | 347.4 |
| HELD2_indirect_few16_n17 | indirect | few(16) | 2^17 | 0.24 | low-H* | 0.729 | 1.029 [0.980, 1.074] | 1.017 | 0.994 [0.957, 1.047] | 1.071 | 54.4 |
| HELD2_indirect_few16_n20 | indirect | few(16) | 2^20 | 0.20 | low-H* | 0.666 | 1.041 [1.018, 1.049] | 1.024 | 1.005 [0.963, 1.018] | 1.130 | 57.1 |
| HELD2_indirect_few256_n17 | indirect | few(256) | 2^17 | 0.47 | low-H* | 0.834 | 1.000 [0.980, 1.016] | 1.039 | 1.000 [0.983, 1.030] | 0.978 | 75.6 |
| HELD2_indirect_few256_n20 | indirect | few(256) | 2^20 | 0.40 | low-H* | 0.711 | 1.001 [0.980, 1.029] | 1.026 | 1.010 [0.940, 1.019] | 1.022 | 80.6 |
| HELD2_indirect_few2_n17 | indirect | few(2) | 2^17 | 0.06 | low-H* | 0.831 | 1.103 [1.032, 1.166] | 1.031 | 1.002 [0.918, 1.062] | 1.147 | 22.1 |
| HELD2_indirect_few2_n20 | indirect | few(2) | 2^20 | 0.05 | low-H* | 0.748 | 1.030 [1.006, 1.040] | 1.038 | 0.985 [0.961, 1.012] | 1.168 | 22.8 |
| HELD2_indirect_few4096_n17 | indirect | few(4096) | 2^17 | 0.70 | high-H* | 1.000 | 1.003 [0.999, 1.039] | 1.031 | 0.993 [0.981, 1.011] | 0.910 | 103.8 |
| HELD2_indirect_few4096_n20 | indirect | few(4096) | 2^20 | 0.60 | high-H* | 0.852 | 1.011 [0.998, 1.046] | 1.047 | 1.007 [0.981, 1.024] | 0.953 | 110.9 |
| HELD2_indirect_mixture64_n17 | indirect | mixture(64) | 2^17 | 0.67 | high-H* | 0.896 | 1.024 [1.013, 1.041] | 1.047 | 0.998 [0.986, 1.020] | 0.916 | 82.6 |
| HELD2_indirect_mixture64_n20 | indirect | mixture(64) | 2^20 | 0.64 | high-H* | 0.858 | 1.009 [0.991, 1.044] | 1.039 | 0.987 [0.965, 1.003] | 0.910 | 102.8 |
| HELD2_indirect_nearly0p01_n17 | indirect | nearly(0.01) | 2^17 | 0.63 | high-H* | 0.527 | 1.120 [1.063, 1.190] | 1.145 | 0.986 [0.850, 1.042] | 1.331 | 9.8 |
| HELD2_indirect_nearly0p01_n20 | indirect | nearly(0.01) | 2^20 | 0.69 | high-H* | 0.474 | 1.029 [1.014, 1.073] | 1.130 | 1.018 [0.990, 1.047] | 1.412 | 11.6 |
| HELD2_indirect_perm0_n17 | indirect | perm | 2^17 | 0.92 | high-H* | 1.000 | 1.039 [0.978, 1.081] | 1.062 | 1.013 [0.950, 1.030] | 0.845 | 206.8 |
| HELD2_indirect_perm0_n20 | indirect | perm | 2^20 | 0.93 | high-H* | 1.000 | 1.016 [1.002, 1.062] | 1.081 | 0.995 [0.987, 1.037] | 0.844 | 138.9 |
| HELD2_indirect_runs64_n17 | indirect | runs(64) | 2^17 | 0.92 | high-H* | 0.990 | 1.030 [1.006, 1.055] | 1.070 | 0.979 [0.944, 1.043] | 0.810 | 34.0 |
| HELD2_indirect_runs64_n20 | indirect | runs(64) | 2^20 | 0.93 | high-H* | 0.989 | 1.048 [0.986, 1.062] | 1.076 | 0.978 [0.961, 1.012] | 0.816 | 35.4 |
| HELD2_indirect_zipf1024_n17 | indirect | zipf(1024) | 2^17 | 0.44 | low-H* | 0.846 | 1.051 [1.006, 1.111] | 1.133 | 1.016 [0.972, 1.062] | 0.998 | 73.1 |
| HELD2_indirect_zipf1024_n20 | indirect | zipf(1024) | 2^20 | 0.38 | low-H* | 0.742 | 1.019 [1.008, 1.041] | 1.087 | 0.994 [0.968, 1.005] | 1.006 | 104.6 |
| HELD2_plain_few16_n17 | plain | few(16) | 2^17 | 0.24 | low-H* | 0.766 | 1.028 [0.998, 1.083] | 1.042 | 1.022 [1.003, 1.075] | 0.974 | 47.4 |
| HELD2_plain_few16_n20 | plain | few(16) | 2^20 | 0.20 | low-H* | 0.685 | 1.013 [0.972, 1.043] | 1.005 | 0.990 [0.977, 1.033] | 0.992 | 52.4 |
| HELD2_plain_few256_n17 | plain | few(256) | 2^17 | 0.47 | low-H* | 0.834 | 1.018 [0.979, 1.045] | 1.041 | 1.005 [0.984, 1.042] | 0.909 | 66.4 |
| HELD2_plain_few256_n20 | plain | few(256) | 2^20 | 0.40 | low-H* | 0.737 | 0.991 [0.969, 1.013] | 1.052 | 0.990 [0.977, 0.995] | 0.908 | 70.5 |
| HELD2_plain_few2_n17 | plain | few(2) | 2^17 | 0.06 | low-H* | 0.846 | 1.009 [0.948, 1.060] | 1.006 | 0.974 [0.926, 1.081] | 1.014 | 20.7 |
| HELD2_plain_few2_n20 | plain | few(2) | 2^20 | 0.05 | low-H* | 0.780 | 0.995 [0.932, 1.054] | 1.035 | 0.982 [0.950, 1.005] | 1.049 | 22.2 |
| HELD2_plain_few4096_n17 | plain | few(4096) | 2^17 | 0.70 | high-H* | 1.000 | 1.021 [0.999, 1.048] | 1.036 | 0.980 [0.955, 1.004] | 0.862 | 88.7 |
| HELD2_plain_few4096_n20 | plain | few(4096) | 2^20 | 0.60 | high-H* | 0.869 | 1.011 [0.999, 1.036] | 1.043 | 0.999 [0.975, 1.002] | 0.880 | 92.8 |
| HELD2_plain_mixture64_n17 | plain | mixture(64) | 2^17 | 0.67 | high-H* | 0.897 | 1.012 [0.999, 1.041] | 1.052 | 1.006 [0.969, 1.048] | 0.910 | 71.2 |
| HELD2_plain_mixture64_n20 | plain | mixture(64) | 2^20 | 0.64 | high-H* | 0.869 | 0.994 [0.963, 1.001] | 1.034 | 0.992 [0.988, 1.013] | 0.904 | 89.6 |
| HELD2_plain_nearly0p01_n17 | plain | nearly(0.01) | 2^17 | 0.63 | high-H* | 0.530 | 1.090 [1.066, 1.168] | 1.163 | 1.019 [1.000, 1.029] | 1.215 | 9.1 |
| HELD2_plain_nearly0p01_n20 | plain | nearly(0.01) | 2^20 | 0.69 | high-H* | 0.474 | 1.035 [0.912, 1.154] | 1.085 | 1.050 [0.874, 1.199] | 1.387 | 11.4 |
| HELD2_plain_perm0_n17 | plain | perm | 2^17 | 0.92 | high-H* | 1.000 | 1.024 [0.976, 1.079] | 1.056 | 1.040 [0.974, 1.079] | 0.882 | 169.0 |
| HELD2_plain_perm0_n20 | plain | perm | 2^20 | 0.93 | high-H* | 1.000 | 0.985 [0.971, 1.011] | 1.042 | 0.995 [0.975, 1.007] | 0.881 | 122.8 |
| HELD2_plain_runs64_n17 | plain | runs(64) | 2^17 | 0.92 | high-H* | 0.993 | 0.983 [0.961, 1.073] | 1.017 | 0.935 [0.915, 1.024] | 0.814 | 30.3 |
| HELD2_plain_runs64_n20 | plain | runs(64) | 2^20 | 0.93 | high-H* | 0.993 | 1.055 [1.010, 1.072] | 1.070 | 1.011 [0.987, 1.077] | 0.835 | 29.5 |
| HELD2_plain_zipf1024_n17 | plain | zipf(1024) | 2^17 | 0.44 | low-H* | 0.862 | 1.034 [0.996, 1.057] | 1.142 | 1.012 [0.988, 1.037] | 0.937 | 65.4 |
| HELD2_plain_zipf1024_n20 | plain | zipf(1024) | 2^20 | 0.38 | low-H* | 0.760 | 1.023 [1.008, 1.111] | 1.122 | 0.995 [0.979, 1.056] | 0.903 | 89.4 |

## Pre-registered criteria (docs/EXPERIMENT_PLAN.md)

| criterion | result | detail |
|---|---|---|
| H1  comparisons: >= 10 % fewer than base in >= 2/3 of low-H* cells | **PASS** | 8 of 8 distinct low-H* (family,size) cells |
| H2  heavy-comparator time: >= 10 % faster (CI < 1) in >= 2/3 of low-H* cells | **PASS** | 8 of 8 cells |
| H3a guard, heavy regime: no severe time regression (ratio >= 1.10 with CI lower bound > 1); comparisons never > 1.03x base (all cells) | **PASS** | severe: none; comparison excess: none |
| H3b guard, cheap regimes (plain, indirect): no severe time regression | **FAIL** | 2 severe cells of 36: ['HELD2_indirect_few2_n17', 'HELD2_indirect_nearly0p01_n17'] |
| H4  correctness: every output matched the reference | **PASS** | failures: none |
| Secondary (default brief criterion): >= 10 % faster on a majority of ALL timed cells | **FAIL** | 12 of 54 cells |
| Exploratory (not gating): low-H* time wins, plain keys | 0 of 8 | cheap comparisons |
| Exploratory (not gating): low-H* time wins, indirect keys | 0 of 8 | cache-missing comparisons |
| A/A noise: largest |median(base_b/base) - 1| over cells | 0.065 | identical code run twice |

**Verdict under the round-1 rules: PARTIAL SUCCESS (conditional on comparator cost)**

## Corrective-round criteria (addendum to docs/EXPERIMENT_PLAN.md; variant under test in the 'SAG' columns)

| criterion | result | detail |
|---|---|---|
| R2-A gain retained: heavy regime, low-H* cells >= 10 % faster (CI < 1) in >= 2/3 | **PASS** | 8 of 8 |
| R2-B no regret: no severe regression (ratio >= 1.10, CI lower > 1) in ANY cell of ANY regime | **FAIL** | severe: ['HELD2_indirect_few2_n17', 'HELD2_indirect_nearly0p01_n17'] |
| R2-C correctness | **PASS** | failures: none |
| R2-D comparisons never > 1.03x base | **PASS** | excess: none |

**Corrective-round verdict under its frozen rules: CORRECTIVE PARTIAL (regressions remain in some cells)**
