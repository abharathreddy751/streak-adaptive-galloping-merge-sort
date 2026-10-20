// generators.hpp -- deterministic synthetic workload generators (portable: no <random> distributions,
// so a given seed produces the same array on every compiler).
//
// Every generator takes (n, parameters..., seed) and returns std::vector<uint32_t>.
// All data are SYNTHETIC; they illustrate specific run / dual-run structures, nothing more.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

namespace gen {

using u32 = std::uint32_t;
using u64 = std::uint64_t;

struct Rng {  // xoshiro256**, seeded through splitmix64
  u64 s[4];
  static u64 splitmix(u64& x) {
    u64 z = (x += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
  }
  explicit Rng(u64 seed) { for (auto& v : s) v = splitmix(seed); }
  static u64 rotl(u64 x, int k) { return (x << k) | (x >> (64 - k)); }
  u64 next() {
    const u64 r = rotl(s[1] * 5, 7) * 9;
    const u64 t = s[1] << 17;
    s[2] ^= s[0]; s[3] ^= s[1]; s[1] ^= s[2]; s[0] ^= s[3];
    s[2] ^= t; s[3] = rotl(s[3], 45);
    return r;
  }
  u64 bounded(u64 n) {  // Lemire's multiply-shift (tiny bias for huge n, irrelevant here)
    return static_cast<u64>((static_cast<unsigned __int128>(next()) * n) >> 64);
  }
  double uniform01() { return (next() >> 11) * (1.0 / 9007199254740992.0); }
};

inline std::vector<u32> random_permutation(std::size_t n, u64 seed) {
  Rng r(seed);
  std::vector<u32> v(n);
  std::iota(v.begin(), v.end(), 0u);
  for (std::size_t i = n; i > 1; --i) std::swap(v[i - 1], v[r.bounded(i)]);
  return v;
}

inline std::vector<u32> few_values(std::size_t n, u32 sigma, u64 seed) {
  Rng r(seed);
  std::vector<u32> v(n);
  for (auto& x : v) x = static_cast<u32>(r.bounded(sigma));
  return v;
}

// Zipf(s) over `sigma` distinct values (value 0 most frequent), inverse-CDF sampling.
inline std::vector<u32> zipf_values(std::size_t n, u32 sigma, double s, u64 seed) {
  Rng r(seed);
  std::vector<double> cdf(sigma);
  double acc = 0.0;
  for (u32 i = 0; i < sigma; ++i) { acc += 1.0 / std::pow(static_cast<double>(i + 1), s); cdf[i] = acc; }
  std::vector<u32> v(n);
  for (auto& x : v) {
    const double u = r.uniform01() * acc;
    x = static_cast<u32>(std::lower_bound(cdf.begin(), cdf.end(), u) - cdf.begin());
    if (x >= sigma) x = sigma - 1;
  }
  return v;
}

// `runs` consecutive sorted blocks of random lengths (random cut points), random values inside.
inline std::vector<u32> sorted_runs(std::size_t n, std::size_t runs, u64 seed) {
  Rng r(seed);
  std::vector<std::size_t> cuts{0, n};
  for (std::size_t i = 1; i < runs; ++i) cuts.push_back(r.bounded(n + 1));
  std::sort(cuts.begin(), cuts.end());
  std::vector<u32> v(n);
  for (auto& x : v) x = static_cast<u32>(r.next() >> 32);
  for (std::size_t i = 0; i + 1 < cuts.size(); ++i) std::sort(v.begin() + cuts[i], v.begin() + cuts[i + 1]);
  return v;
}

// First half: few distinct values; second half: a random permutation of large values.
inline std::vector<u32> mixture(std::size_t n, u32 sigma, u64 seed) {
  std::vector<u32> a = few_values(n / 2, sigma, seed);
  std::vector<u32> b = random_permutation(n - n / 2, seed ^ 0xABCDEF);
  for (auto& x : b) x += sigma;
  a.insert(a.end(), b.begin(), b.end());
  return a;
}

inline std::vector<u32> nearly_sorted(std::size_t n, double swap_fraction, u64 seed) {
  Rng r(seed);
  std::vector<u32> v(n);
  std::iota(v.begin(), v.end(), 0u);
  const std::size_t swaps = static_cast<std::size_t>(swap_fraction * static_cast<double>(n));
  for (std::size_t i = 0; i < swaps && n > 1; ++i) std::swap(v[r.bounded(n)], v[r.bounded(n)]);
  return v;
}

inline std::vector<u32> sorted_array(std::size_t n) { std::vector<u32> v(n); std::iota(v.begin(), v.end(), 0u); return v; }
inline std::vector<u32> reversed_array(std::size_t n) { std::vector<u32> v = sorted_array(n); std::reverse(v.begin(), v.end()); return v; }
inline std::vector<u32> pipe_organ(std::size_t n) {
  std::vector<u32> v(n);
  for (std::size_t i = 0; i < n; ++i) v[i] = static_cast<u32>(i < n / 2 ? i : n - 1 - i);
  return v;
}

// Dispatch by name, used by the benchmark and tests.  `p` is the family parameter (sigma, run count, ...).
inline std::vector<u32> make(const std::string& family, std::size_t n, double p, u64 seed) {
  if (family == "perm") return random_permutation(n, seed);
  if (family == "few") return few_values(n, static_cast<u32>(p), seed);
  if (family == "zipf") return zipf_values(n, static_cast<u32>(p), 1.0, seed);
  if (family == "runs") return sorted_runs(n, static_cast<std::size_t>(p), seed);
  if (family == "mixture") return mixture(n, static_cast<u32>(p), seed);
  if (family == "nearly") return nearly_sorted(n, p, seed);
  if (family == "sorted") return sorted_array(n);
  if (family == "reversed") return reversed_array(n);
  if (family == "pipe") return pipe_organ(n);
  return {};
}

}  // namespace gen
