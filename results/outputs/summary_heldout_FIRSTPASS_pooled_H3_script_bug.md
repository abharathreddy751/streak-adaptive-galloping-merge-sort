# Summary of `heldout.csv`

Cells: 72.  Statistic: median over repetitions of the paired ratio variant/base (bootstrap 95 % CI, 10000 resamples, seed 12345).

| cell | regime | family | n | H*/log2 n | class | cmp SAG/base | time SAG/base [95% CI] | A/A base_b/base [95% CI] | time naive/base | base ns/elem |
|---|---|---|---|---|---|---|---|---|---|---|
| HELD_heavy_few16_n17 | heavy | few(16) | 2^17 | 0.24 | low-H* | 0.759 | 0.794 [0.790, 0.819] | 0.999 [0.994, 1.022] | 1.231 | 272.2 |
| HELD_heavy_few16_n20 | heavy | few(16) | 2^20 | 0.20 | low-H* | 0.754 | 0.800 [0.778, 0.821] | 1.006 [0.985, 1.036] | 1.447 | 275.4 |
| HELD_heavy_few256_n17 | heavy | few(256) | 2^17 | 0.47 | low-H* | 0.834 | 0.887 [0.824, 0.951] | 1.005 [0.957, 1.040] | 1.001 | 562.6 |
| HELD_heavy_few256_n20 | heavy | few(256) | 2^20 | 0.40 | low-H* | 0.777 | 0.822 [0.801, 0.843] | 1.005 [0.986, 1.019] | 1.072 | 375.4 |
| HELD_heavy_few2_n17 | heavy | few(2) | 2^17 | 0.06 | low-H* | 0.852 | 0.880 [0.868, 0.888] | 1.003 [0.976, 1.024] | 1.690 | 157.6 |
| HELD_heavy_few2_n20 | heavy | few(2) | 2^20 | 0.05 | low-H* | 0.851 | 0.866 [0.854, 0.886] | 0.990 [0.959, 0.998] | 1.956 | 161.1 |
| HELD_heavy_few4096_n17 | heavy | few(4096) | 2^17 | 0.70 | high-H* | 1.000 | 1.022 [1.013, 1.038] | 0.996 [0.989, 1.022] | 0.981 | 359.8 |
| HELD_heavy_few4096_n20 | heavy | few(4096) | 2^20 | 0.60 | high-H* | 0.910 | 0.934 [0.905, 0.947] | 1.004 [0.964, 1.030] | 0.973 | 421.8 |
| HELD_heavy_mixture64_n17 | heavy | mixture(64) | 2^17 | 0.67 | high-H* | 0.897 | 0.931 [0.920, 0.941] | 1.009 [0.987, 1.015] | 0.992 | 337.2 |
| HELD_heavy_mixture64_n20 | heavy | mixture(64) | 2^20 | 0.64 | high-H* | 0.892 | 0.923 [0.901, 0.935] | 0.997 [0.983, 1.013] | 1.058 | 367.9 |
| HELD_heavy_nearly0p01_n17 | heavy | nearly(0.01) | 2^17 | 0.63 | high-H* | 0.540 | 0.581 [0.568, 0.594] | 1.000 [0.988, 1.013] | 1.826 | 132.2 |
| HELD_heavy_nearly0p01_n20 | heavy | nearly(0.01) | 2^20 | 0.69 | high-H* | 0.506 | 0.563 [0.547, 0.569] | 0.998 [0.971, 1.017] | 2.110 | 144.7 |
| HELD_heavy_perm0_n17 | heavy | perm | 2^17 | 0.92 | high-H* | 1.000 | 1.015 [0.982, 1.028] | 0.996 [0.977, 1.014] | 0.975 | 388.7 |
| HELD_heavy_perm0_n20 | heavy | perm | 2^20 | 0.93 | high-H* | 1.000 | 1.010 [0.977, 1.042] | 0.999 [0.968, 1.014] | 0.965 | 459.8 |
| HELD_heavy_runs64_n17 | heavy | runs(64) | 2^17 | 0.92 | high-H* | 0.992 | 1.002 [0.993, 1.024] | 0.998 [0.982, 1.013] | 0.954 | 154.8 |
| HELD_heavy_runs64_n20 | heavy | runs(64) | 2^20 | 0.93 | high-H* | 0.984 | 1.009 [0.975, 1.031] | 0.993 [0.977, 1.007] | 0.956 | 152.9 |
| HELD_heavy_zipf1024_n17 | heavy | zipf(1024) | 2^17 | 0.44 | low-H* | 0.815 | 0.871 [0.866, 0.873] | 0.995 [0.987, 1.002] | 1.048 | 322.7 |
| HELD_heavy_zipf1024_n20 | heavy | zipf(1024) | 2^20 | 0.38 | low-H* | 0.756 | 0.823 [0.792, 0.842] | 0.999 [0.962, 1.031] | 1.131 | 352.1 |
| HELD_indirect_few16_n17 | indirect | few(16) | 2^17 | 0.24 | low-H* | 0.759 | 1.095 [1.038, 1.142] | 1.006 [0.969, 1.042] | 1.096 | 52.8 |
| HELD_indirect_few16_n20 | indirect | few(16) | 2^20 | 0.20 | low-H* | 0.754 | 1.076 [1.039, 1.091] | 1.020 [1.016, 1.033] | 1.167 | 55.7 |
| HELD_indirect_few16_n22 | indirect | few(16) | 2^22 | 0.18 | low-H* | 0.754 | 1.077 [1.044, 1.094] | 1.023 [0.987, 1.026] | 1.306 | 72.6 |
| HELD_indirect_few256_n17 | indirect | few(256) | 2^17 | 0.47 | low-H* | 0.834 | 1.095 [1.054, 1.157] | 0.985 [0.951, 1.065] | 0.976 | 80.3 |
| HELD_indirect_few256_n20 | indirect | few(256) | 2^20 | 0.40 | low-H* | 0.777 | 1.072 [1.032, 1.095] | 0.994 [0.965, 1.026] | 1.030 | 79.5 |
| HELD_indirect_few256_n22 | indirect | few(256) | 2^22 | 0.36 | low-H* | 0.770 | 1.044 [1.016, 1.082] | 1.010 [0.990, 1.027] | 1.119 | 86.1 |
| HELD_indirect_few2_n17 | indirect | few(2) | 2^17 | 0.06 | low-H* | 0.852 | 1.066 [1.025, 1.137] | 1.085 [0.995, 1.101] | 1.159 | 21.6 |
| HELD_indirect_few2_n20 | indirect | few(2) | 2^20 | 0.05 | low-H* | 0.851 | 1.025 [0.978, 1.065] | 1.055 [0.964, 1.108] | 1.182 | 23.5 |
| HELD_indirect_few2_n22 | indirect | few(2) | 2^22 | 0.05 | low-H* | 0.851 | 1.041 [0.999, 1.059] | 0.980 [0.927, 1.012] | 1.200 | 26.2 |
| HELD_indirect_few4096_n17 | indirect | few(4096) | 2^17 | 0.70 | high-H* | 1.000 | 1.052 [1.022, 1.068] | 1.002 [0.970, 1.016] | 0.925 | 101.4 |
| HELD_indirect_few4096_n20 | indirect | few(4096) | 2^20 | 0.60 | high-H* | 0.910 | 1.052 [1.042, 1.071] | 1.003 [0.971, 1.026] | 0.953 | 114.0 |
| HELD_indirect_few4096_n22 | indirect | few(4096) | 2^22 | 0.55 | high-H* | 0.829 | 1.005 [0.989, 1.008] | 1.009 [0.998, 1.055] | 0.973 | 119.8 |
| HELD_indirect_mixture64_n17 | indirect | mixture(64) | 2^17 | 0.67 | high-H* | 0.897 | 1.086 [0.997, 1.155] | 0.994 [0.944, 1.063] | 0.935 | 89.1 |
| HELD_indirect_mixture64_n20 | indirect | mixture(64) | 2^20 | 0.64 | high-H* | 0.892 | 1.060 [1.037, 1.080] | 1.014 [0.977, 1.055] | 0.922 | 99.5 |
| HELD_indirect_mixture64_n22 | indirect | mixture(64) | 2^22 | 0.63 | high-H* | 0.896 | 1.069 [1.053, 1.078] | 1.007 [0.993, 1.017] | 0.925 | 114.1 |
| HELD_indirect_nearly0p01_n17 | indirect | nearly(0.01) | 2^17 | 0.63 | high-H* | 0.540 | 1.240 [1.146, 1.348] | 1.003 [0.920, 1.036] | 1.324 | 9.8 |
| HELD_indirect_nearly0p01_n20 | indirect | nearly(0.01) | 2^20 | 0.69 | high-H* | 0.506 | 1.146 [1.103, 1.223] | 0.988 [0.947, 1.050] | 1.427 | 12.0 |
| HELD_indirect_nearly0p01_n22 | indirect | nearly(0.01) | 2^22 | 0.72 | high-H* | 0.492 | 1.089 [1.038, 1.132] | 0.991 [0.956, 1.039] | 1.341 | 17.1 |
| HELD_indirect_perm0_n17 | indirect | perm | 2^17 | 0.92 | high-H* | 1.000 | 1.070 [1.047, 1.118] | 0.991 [0.975, 1.076] | 0.862 | 220.8 |
| HELD_indirect_perm0_n20 | indirect | perm | 2^20 | 0.93 | high-H* | 1.000 | 1.099 [1.032, 1.116] | 0.988 [0.954, 1.027] | 0.858 | 233.0 |
| HELD_indirect_perm0_n22 | indirect | perm | 2^22 | 0.94 | high-H* | 1.000 | 1.079 [1.047, 1.099] | 0.985 [0.970, 1.015] | 0.815 | 178.6 |
| HELD_indirect_runs64_n17 | indirect | runs(64) | 2^17 | 0.92 | high-H* | 0.992 | 1.114 [1.069, 1.161] | 0.997 [0.984, 1.018] | 0.827 | 34.9 |
| HELD_indirect_runs64_n20 | indirect | runs(64) | 2^20 | 0.93 | high-H* | 0.984 | 1.094 [1.075, 1.103] | 1.009 [0.990, 1.015] | 0.837 | 37.8 |
| HELD_indirect_runs64_n22 | indirect | runs(64) | 2^22 | 0.94 | high-H* | 0.988 | 1.079 [1.055, 1.087] | 0.994 [0.986, 1.004] | 0.835 | 37.1 |
| HELD_indirect_zipf1024_n17 | indirect | zipf(1024) | 2^17 | 0.44 | low-H* | 0.815 | 1.138 [1.110, 1.188] | 0.979 [0.969, 1.017] | 0.988 | 72.6 |
| HELD_indirect_zipf1024_n20 | indirect | zipf(1024) | 2^20 | 0.38 | low-H* | 0.756 | 1.135 [1.090, 1.144] | 0.996 [0.977, 1.003] | 1.020 | 79.1 |
| HELD_indirect_zipf1024_n22 | indirect | zipf(1024) | 2^22 | 0.34 | low-H* | 0.734 | 1.107 [1.081, 1.116] | 1.009 [0.983, 1.025] | 1.087 | 81.2 |
| HELD_plain_few16_n17 | plain | few(16) | 2^17 | 0.24 | low-H* | 0.759 | 1.032 [0.989, 1.071] | 1.022 [0.949, 1.044] | 0.954 | 49.0 |
| HELD_plain_few16_n20 | plain | few(16) | 2^20 | 0.20 | low-H* | 0.754 | 1.034 [1.017, 1.051] | 1.009 [0.982, 1.027] | 0.988 | 50.2 |
| HELD_plain_few16_n22 | plain | few(16) | 2^22 | 0.18 | low-H* | 0.754 | 1.020 [1.004, 1.070] | 0.989 [0.982, 1.021] | 0.996 | 52.2 |
| HELD_plain_few256_n17 | plain | few(256) | 2^17 | 0.47 | low-H* | 0.834 | 1.048 [1.029, 1.095] | 1.021 [0.982, 1.064] | 0.945 | 69.3 |
| HELD_plain_few256_n20 | plain | few(256) | 2^20 | 0.40 | low-H* | 0.777 | 1.034 [1.008, 1.064] | 0.995 [0.963, 1.039] | 0.903 | 104.7 |
| HELD_plain_few256_n22 | plain | few(256) | 2^22 | 0.36 | low-H* | 0.770 | 1.036 [1.006, 1.051] | 0.997 [0.973, 1.010] | 0.910 | 73.9 |
| HELD_plain_few2_n17 | plain | few(2) | 2^17 | 0.06 | low-H* | 0.852 | 1.007 [0.973, 1.037] | 0.979 [0.957, 1.127] | 1.112 | 21.7 |
| HELD_plain_few2_n20 | plain | few(2) | 2^20 | 0.05 | low-H* | 0.851 | 1.040 [0.975, 1.124] | 1.008 [0.977, 1.044] | 1.046 | 22.2 |
| HELD_plain_few2_n22 | plain | few(2) | 2^22 | 0.05 | low-H* | 0.851 | 1.031 [1.002, 1.060] | 1.018 [0.999, 1.031] | 1.107 | 24.0 |
| HELD_plain_few4096_n17 | plain | few(4096) | 2^17 | 0.70 | high-H* | 1.000 | 1.051 [1.037, 1.088] | 1.005 [0.978, 1.042] | 0.902 | 86.8 |
| HELD_plain_few4096_n20 | plain | few(4096) | 2^20 | 0.60 | high-H* | 0.910 | 1.070 [1.049, 1.087] | 1.004 [0.997, 1.031] | 0.900 | 88.3 |
| HELD_plain_few4096_n22 | plain | few(4096) | 2^22 | 0.55 | high-H* | 0.829 | 1.066 [1.054, 1.086] | 0.986 [0.975, 0.995] | 0.881 | 90.0 |
| HELD_plain_mixture64_n17 | plain | mixture(64) | 2^17 | 0.67 | high-H* | 0.897 | 1.061 [1.035, 1.089] | 1.001 [0.987, 1.014] | 0.903 | 71.0 |
| HELD_plain_mixture64_n20 | plain | mixture(64) | 2^20 | 0.64 | high-H* | 0.892 | 1.061 [1.039, 1.085] | 0.998 [0.973, 1.005] | 0.914 | 84.8 |
| HELD_plain_mixture64_n22 | plain | mixture(64) | 2^22 | 0.63 | high-H* | 0.896 | 1.074 [1.042, 1.099] | 1.012 [1.006, 1.029] | 0.918 | 86.2 |
| HELD_plain_nearly0p01_n17 | plain | nearly(0.01) | 2^17 | 0.63 | high-H* | 0.540 | 1.126 [1.015, 1.165] | 1.016 [0.952, 1.046] | 1.190 | 9.8 |
| HELD_plain_nearly0p01_n20 | plain | nearly(0.01) | 2^20 | 0.69 | high-H* | 0.506 | 1.117 [1.083, 1.134] | 1.002 [0.996, 1.019] | 1.303 | 10.5 |
| HELD_plain_nearly0p01_n22 | plain | nearly(0.01) | 2^22 | 0.72 | high-H* | 0.492 | 1.133 [1.087, 1.150] | 1.003 [0.954, 1.042] | 1.272 | 13.6 |
| HELD_plain_perm0_n17 | plain | perm | 2^17 | 0.92 | high-H* | 1.000 | 1.079 [1.024, 1.099] | 1.005 [0.958, 1.035] | 0.889 | 172.4 |
| HELD_plain_perm0_n20 | plain | perm | 2^20 | 0.93 | high-H* | 1.000 | 1.056 [1.035, 1.066] | 0.982 [0.974, 0.994] | 0.878 | 116.6 |
| HELD_plain_perm0_n22 | plain | perm | 2^22 | 0.94 | high-H* | 1.000 | 1.067 [1.056, 1.074] | 0.993 [0.991, 1.015] | 0.878 | 126.9 |
| HELD_plain_runs64_n17 | plain | runs(64) | 2^17 | 0.92 | high-H* | 0.992 | 1.080 [1.075, 1.111] | 1.026 [0.962, 1.071] | 0.827 | 30.2 |
| HELD_plain_runs64_n20 | plain | runs(64) | 2^20 | 0.93 | high-H* | 0.984 | 1.079 [1.067, 1.112] | 1.004 [0.997, 1.028] | 0.840 | 31.9 |
| HELD_plain_runs64_n22 | plain | runs(64) | 2^22 | 0.94 | high-H* | 0.988 | 1.087 [1.051, 1.120] | 0.994 [0.960, 1.018] | 0.846 | 31.4 |
| HELD_plain_zipf1024_n17 | plain | zipf(1024) | 2^17 | 0.44 | low-H* | 0.815 | 1.127 [1.102, 1.162] | 0.982 [0.965, 1.017] | 0.912 | 66.3 |
| HELD_plain_zipf1024_n20 | plain | zipf(1024) | 2^20 | 0.38 | low-H* | 0.756 | 1.131 [1.117, 1.140] | 1.009 [0.975, 1.027] | 0.930 | 69.1 |
| HELD_plain_zipf1024_n22 | plain | zipf(1024) | 2^22 | 0.34 | low-H* | 0.734 | 1.127 [1.088, 1.147] | 0.993 [0.983, 1.012] | 0.936 | 68.3 |

## Pre-registered criteria (docs/EXPERIMENT_PLAN.md)

| criterion | result | detail |
|---|---|---|
| H1  comparisons: >= 10 % fewer than base in >= 2/3 of low-H* cells | **PASS** | 12 of 12 distinct low-H* (family,size) cells |
| H2  heavy-comparator time: >= 10 % faster (CI < 1) in >= 2/3 of low-H* cells | **PASS** | 8 of 8 cells |
| H3  guards: no severe time regression (ratio >= 1.10 with CI lower bound > 1) in ANY cell; comparisons never > 1.03x base | **FAIL** | severe: ['HELD_indirect_nearly0p01_n17', 'HELD_indirect_nearly0p01_n20', 'HELD_indirect_runs64_n17', 'HELD_indirect_zipf1024_n17', 'HELD_indirect_zipf1024_n20', 'HELD_indirect_zipf1024_n22', 'HELD_plain_nearly0p01_n17', 'HELD_plain_nearly0p01_n20', 'HELD_plain_nearly0p01_n22', 'HELD_plain_zipf1024_n17', 'HELD_plain_zipf1024_n20', 'HELD_plain_zipf1024_n22']; comparison excess: none |
| H4  correctness: every output matched the reference | **PASS** | failures: none |
| Secondary (default brief criterion): >= 10 % faster on a majority of ALL timed cells | **FAIL** | 10 of 72 cells |
| Exploratory (not gating): low-H* time wins, plain keys | 0 of 12 | cheap comparisons |
| Exploratory (not gating): low-H* time wins, indirect keys | 0 of 12 | cache-missing comparisons |
| A/A noise: largest |median(base_b/base) - 1| over cells | 0.085 | identical code run twice |

**Verdict under the frozen rules: NO-GO**
