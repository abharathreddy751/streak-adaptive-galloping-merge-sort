# data/

This project stores **no input data files**. Every input is generated deterministically in memory by `src/generators.hpp`
from a family name, a size and a seed (xoshiro256** seeded through splitmix64; no `<random>` distributions, so a seed gives
the same array on every compiler). The exact cells, sizes and seeds are plain data in `scripts/run_bench.py`:

| manifest | seeds | purpose |
|---|---|---|
| `dev_counts`, `dev_time`, `dev_selection`, `dev_sagt` | 101 … 900 | development / tuning only |
| `heldout` | 9001 + 10·family + size index | frozen round-1 evaluation (90 cells) |
| `heldout2` | 109001 + 10·family + size index | frozen corrective round, fresh seeds (54 cells) |

Families: `perm` (random permutation), `few(σ)` (uniform values from σ distinct), `zipf(σ)` (Zipf s = 1), `runs(r)` (r sorted runs
with random values), `mixture(σ)` (half few-valued, half random permutation), `nearly(f)` (sorted array with a fraction f of
random swaps), `sorted`, `reversed`, `pipe`. **All inputs are synthetic**; no real data set was used, so no claim of real-world
superiority is made.
