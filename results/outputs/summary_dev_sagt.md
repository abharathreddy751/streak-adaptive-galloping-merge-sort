# Summary of `dev_sagt.csv`

Cells: 21.  Statistic: median over repetitions of the paired ratio variant/base (bootstrap 95 % CI, 10000 resamples, seed 12345).

Variant under test in the 'SAG' columns: `sagt`; the round-1 SAG is shown as 'round-1 SAG'.

| cell | regime | family | n | H*/log2 n | class | cmp SAG/base | time SAG/base [95% CI] | round-1 SAG time/base | A/A base_b/base [95% CI] | time naive/base | base ns/elem |
|---|---|---|---|---|---|---|---|---|---|---|---|
| DEVS_heavy_few32_n17 | heavy | few(32) | 2^17 | 0.29 | low-H* | - | - | 0.797 | 1.012 [0.966, 1.019] | - | 341.3 |
| DEVS_heavy_few512_n17 | heavy | few(512) | 2^17 | 0.53 | high-H* | - | - | 0.916 | 1.000 [0.991, 1.040] | - | 356.0 |
| DEVS_heavy_mixture16_n17 | heavy | mixture(16) | 2^17 | 0.61 | high-H* | - | - | 0.920 | 0.981 [0.961, 1.003] | - | 319.3 |
| DEVS_heavy_nearly0p05_n17 | heavy | nearly(0.05) | 2^17 | 0.77 | high-H* | - | - | 0.704 | 1.025 [0.993, 1.055] | - | 249.3 |
| DEVS_heavy_perm0_n17 | heavy | perm | 2^17 | 0.92 | high-H* | - | - | 1.015 | 0.981 [0.964, 1.024] | - | 448.6 |
| DEVS_heavy_runs16_n17 | heavy | runs(16) | 2^17 | 0.92 | high-H* | - | - | 1.017 | 0.977 [0.953, 1.033] | - | 124.1 |
| DEVS_heavy_zipf256_n17 | heavy | zipf(256) | 2^17 | 0.37 | low-H* | - | - | 0.830 | 0.990 [0.977, 1.004] | - | 323.9 |
| DEVS_indirect_few32_n19 | indirect | few(32) | 2^19 | 0.26 | low-H* | - | - | 1.021 | 0.974 [0.911, 1.077] | - | 67.2 |
| DEVS_indirect_few512_n19 | indirect | few(512) | 2^19 | 0.47 | low-H* | - | - | 1.056 | 0.981 [0.959, 1.052] | - | 98.0 |
| DEVS_indirect_mixture16_n19 | indirect | mixture(16) | 2^19 | 0.60 | high-H* | - | - | 1.069 | 1.007 [0.990, 1.034] | - | 88.3 |
| DEVS_indirect_nearly0p05_n19 | indirect | nearly(0.05) | 2^19 | 0.79 | high-H* | - | - | 1.137 | 0.972 [0.927, 1.103] | - | 31.5 |
| DEVS_indirect_perm0_n19 | indirect | perm | 2^19 | 0.93 | high-H* | - | - | 1.016 | 0.962 [0.901, 1.023] | - | 159.5 |
| DEVS_indirect_runs16_n19 | indirect | runs(16) | 2^19 | 0.93 | high-H* | - | - | 1.072 | 0.984 [0.860, 1.043] | - | 31.4 |
| DEVS_indirect_zipf256_n19 | indirect | zipf(256) | 2^19 | 0.33 | low-H* | - | - | 1.099 | 0.997 [0.971, 1.019] | - | 73.8 |
| DEVS_plain_few32_n19 | plain | few(32) | 2^19 | 0.26 | low-H* | - | - | 1.043 | 1.005 [0.951, 1.021] | - | 65.9 |
| DEVS_plain_few512_n19 | plain | few(512) | 2^19 | 0.47 | low-H* | - | - | 1.058 | 1.002 [0.970, 1.040] | - | 87.2 |
| DEVS_plain_mixture16_n19 | plain | mixture(16) | 2^19 | 0.60 | high-H* | - | - | 1.033 | 1.011 [0.940, 1.022] | - | 71.2 |
| DEVS_plain_nearly0p05_n19 | plain | nearly(0.05) | 2^19 | 0.79 | high-H* | - | - | 1.203 | 1.011 [0.979, 1.065] | - | 26.4 |
| DEVS_plain_perm0_n19 | plain | perm | 2^19 | 0.93 | high-H* | - | - | 1.022 | 0.985 [0.941, 1.017] | - | 126.9 |
| DEVS_plain_runs16_n19 | plain | runs(16) | 2^19 | 0.93 | high-H* | - | - | 1.084 | 1.010 [0.910, 1.134] | - | 23.9 |
| DEVS_plain_zipf256_n19 | plain | zipf(256) | 2^19 | 0.33 | low-H* | - | - | 1.094 | 0.991 [0.939, 1.019] | - | 59.2 |

## Pre-registered criteria (docs/EXPERIMENT_PLAN.md)

| criterion | result | detail |
|---|---|---|
| H1  comparisons: >= 10 % fewer than base in >= 2/3 of low-H* cells | **FAIL** | 0 of 0 distinct low-H* (family,size) cells |
| H2  heavy-comparator time: >= 10 % faster (CI < 1) in >= 2/3 of low-H* cells | **FAIL** | 0 of 0 cells |
| H3a guard, heavy regime: no severe time regression (ratio >= 1.10 with CI lower bound > 1); comparisons never > 1.03x base (all cells) | **PASS** | severe: none; comparison excess: none |
| H3b guard, cheap regimes (plain, indirect): no severe time regression | **PASS** | 0 severe cells of 0: none |
| H4  correctness: every output matched the reference | **PASS** | failures: none |
| Secondary (default brief criterion): >= 10 % faster on a majority of ALL timed cells | **FAIL** | 0 of 0 cells |
| Exploratory (not gating): low-H* time wins, plain keys | 0 of 0 | cheap comparisons |
| Exploratory (not gating): low-H* time wins, indirect keys | 0 of 0 | cache-missing comparisons |
| A/A noise: largest |median(base_b/base) - 1| over cells | 0.038 | identical code run twice |

**Verdict under the round-1 rules: NO-GO**

## Corrective-round criteria (addendum to docs/EXPERIMENT_PLAN.md; variant under test in the 'SAG' columns)

| criterion | result | detail |
|---|---|---|
| R2-A gain retained: heavy regime, low-H* cells >= 10 % faster (CI < 1) in >= 2/3 | **FAIL** | 0 of 0 |
| R2-B no regret: no severe regression (ratio >= 1.10, CI lower > 1) in ANY cell of ANY regime | **PASS** | severe: none |
| R2-C correctness | **PASS** | failures: none |
| R2-D comparisons never > 1.03x base | **PASS** | excess: none |

**Corrective-round verdict under its frozen rules: CORRECTIVE NO-GO**
