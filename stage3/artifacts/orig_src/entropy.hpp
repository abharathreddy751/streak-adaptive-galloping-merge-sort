// entropy.hpp -- run-length entropy H and dual run-length entropy H* of an array (paper, Sections 1 and 3).
//
//   runs of A      : greedy decomposition into maximal non-decreasing or strictly decreasing blocks
//                    (the same rule the sorter uses to detect runs).
//   dual runs of A : take the stable rank permutation pi of A, then decompose the INVERSE permutation
//                    into maximal monotone (increasing or decreasing) runs; their lengths s_i give
//                    H* = -sum (s_i/n) log2(s_i/n).   An array with sigma distinct values has at most
//                    sigma dual runs, so H* <= log2(sigma).
//
// These are offline analysis helpers (they sort); they are never used inside a timed region.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

namespace nms {

inline double entropy_of_lengths(const std::vector<std::size_t>& lens, std::size_t n) {
  double h = 0.0;
  for (std::size_t l : lens) {
    const double p = static_cast<double>(l) / static_cast<double>(n);
    h -= p * std::log2(p);
  }
  return h;
}

// Natural runs: non-decreasing, or strictly decreasing (as count_run in natural_merge_sort.hpp).
template <class T>
std::vector<std::size_t> natural_run_lengths(const std::vector<T>& a) {
  std::vector<std::size_t> lens;
  std::size_t i = 0, n = a.size();
  while (i < n) {
    if (i + 1 == n) { lens.push_back(1); break; }
    std::size_t r = 2;
    if (a[i + 1] < a[i]) { while (i + r < n && a[i + r] < a[i + r - 1]) ++r; }
    else { while (i + r < n && !(a[i + r] < a[i + r - 1])) ++r; }
    lens.push_back(r);
    i += r;
  }
  return lens;
}

template <class T>
double run_entropy(const std::vector<T>& a) {
  if (a.empty()) return 0.0;
  return entropy_of_lengths(natural_run_lengths(a), a.size());
}

// Dual runs: monotone runs (either direction) of the inverse stable-rank permutation.
template <class T>
std::vector<std::size_t> dual_run_lengths(const std::vector<T>& a) {
  const std::size_t n = a.size();
  std::vector<std::uint32_t> pos(n);  // pos[r] = original index of the element with stable rank r
  std::iota(pos.begin(), pos.end(), 0u);
  std::stable_sort(pos.begin(), pos.end(), [&](std::uint32_t x, std::uint32_t y) { return a[x] < a[y]; });
  std::vector<std::size_t> lens;
  std::size_t i = 0;
  while (i < n) {
    if (i + 1 == n) { lens.push_back(1); break; }
    std::size_t r = 2;
    if (pos[i + 1] > pos[i]) { while (i + r < n && pos[i + r] > pos[i + r - 1]) ++r; }
    else { while (i + r < n && pos[i + r] < pos[i + r - 1]) ++r; }
    lens.push_back(r);
    i += r;
  }
  return lens;
}

template <class T>
double dual_run_entropy(const std::vector<T>& a) {
  if (a.empty()) return 0.0;
  return entropy_of_lengths(dual_run_lengths(a), a.size());
}

}  // namespace nms
