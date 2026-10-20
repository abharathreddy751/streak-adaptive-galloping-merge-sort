// sag_time_policy.hpp -- CORRECTIVE-ROUND EXTENSION (SAG-T), added after the round-1 held-out result.
//
// Diagnosis (round 1, docs/FINAL_REPORT.md): SAG chooses t by comparison COUNTS only.  After a jump to t = 0 every
// later short block is found through the exponential/binary path: the comparison count is unchanged (so SAG sees no
// waste) but each such probe is a data-dependent, poorly predicted branch, which costs TIME.  With cheap comparisons
// that time penalty is not repaid (up to +24 % vs the base); with expensive comparisons it is.
//
// SAG-T keeps the same controller but evaluates its candidates with a two-parameter TIME model
//        time(t | block) = c * (all probes) + m * (non-linear probes)
// where  c = cost of one key comparison  -- MEASURED BY THE SORTER ITSELF on a small sample of the input at start
//                                            (so no information is supplied from outside), and
//        m = extra cost of a non-linear (unpredictable) probe -- an architecture constant fixed on development data.
// Candidates: t/2, min(t_max, 2t+2), 0 and t_max (fast recovery).  The block's probe counts come from the paper's
// own cost model (gallop_tests_model), split into linear and non-linear probes; no extra key comparisons.
//
// Worst case is unchanged from SAG: any t >= 0 costs <= (4/3)*block length in comparisons.
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>

#include "natural_merge_sort.hpp"

namespace nms {

struct SagTimeParams {
  double c_ns = 1.0;  // measured comparison cost (ns); set by sag_time_sort()
  double m_ns = 4.0;  // extra cost of a non-linear probe (ns)
  size_t breakeven = 64;  // break-even block length B(c, m); set by sag_be_sort()/sag_credit_sort()
  double cap_ns = 400.0;  // credit cap (SAG-C)
};
inline SagTimeParams& sag_time_params() {
  static thread_local SagTimeParams p;
  return p;
}

struct ProbeSplit { u64 linear, nonlinear; };

// Probes of gallop_count() for a block whose tail has k "false" elements out of `len`: phase C1 probes are linear,
// everything after (exponential + binary search) is non-linear.
inline ProbeSplit gallop_probe_split(size_t t, size_t k, size_t len) {
  const size_t lin = t < len ? t : len;
  const u64 total = gallop_tests_model(t, k, len);
  const u64 L = (k < lin) ? static_cast<u64>(k) + 1 : static_cast<u64>(lin);
  return {L, total - L};
}

template <unsigned MinStreak>
struct SagTimeTT {
  static constexpr bool kNaive = false;
  struct State { size_t t, tmax; double c, m; };

  static State start(size_t total) {
    const size_t l = ceil_log2(total);
    const SagTimeParams& p = sag_time_params();
    return {l * l, l * l, p.c_ns, p.m_ns};
  }
  static size_t t(const State& s) { return s.t; }

  NMS_INLINE static void observe(State& s, size_t n, u64 tests, size_t len) {
    // fast path: every probe of this block was linear (tests <= t) and the block is short -> nothing to learn
    if (__builtin_expect(tests <= s.t && n < MinStreak, 1)) return;
    s.t = adapt(s.t, s.tmax, s.c, s.m, n, len);
  }

  NMS_NOINLINE static size_t adapt(size_t t, size_t tmax, double c, double m, size_t n, size_t len) {
    const size_t k = n - 1, tail = len - 1;
    auto cost = [&](size_t cand) {
      const ProbeSplit p = gallop_probe_split(cand, k, tail);
      return c * static_cast<double>(p.linear + p.nonlinear) + m * static_cast<double>(p.nonlinear);
    };
    size_t best_t = t;
    double best = cost(t);
    const size_t cands[4] = {t / 2, std::min(tmax, 2 * t + 2), size_t(0), tmax};
    for (size_t cand : cands) {
      if (cand == best_t) continue;
      const double cc = cost(cand);
      if (cc < best) { best = cc; best_t = cand; }
    }
    return best_t;
  }
};

// Measures the throughput cost (ns) of one comparison on pseudo-random pairs of the input (S = 16384 calls).
template <class T, class Less>
double measure_comparison_ns(const T* a, size_t n, Less less) {
  using clk = std::chrono::steady_clock;
  const size_t S = 16384;
  size_t si = (7919 % n) | 1, sj = (104729 % n) | 1;
  size_t i = 0, j = n / 2;
  std::uint64_t acc = 0;
  const auto t0 = clk::now();
  for (size_t s = 0; s < S; ++s) {
    acc += less(a[i], a[j]) ? 1u : 0u;
    i += si; if (i >= n) i -= n;
    j += sj; if (j >= n) j -= n;
  }
  const auto t1 = clk::now();
#if defined(__GNUC__)
  asm volatile("" : : "r"(acc));
#endif
  const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(S);
  return std::min(1000.0, std::max(0.2, ns));
}

// Sorter entry point for SAG-T.  Arrays shorter than 4096 elements use the paper's base policy (t_max is tiny there
// and a calibration would cost more than the sort).
template <unsigned MinStreak, bool Count = false, class T, class Less>
void sag_time_sort(T* data, size_t n, Less less, Stats* stats = nullptr, size_t minrun = 32, double m_ns = 4.0) {
  if (n < 4096) {
    natural_merge_sort<Order::Power, PolylogT, Count>(data, n, less, stats, minrun);
    return;
  }
  sag_time_params() = {measure_comparison_ns(data, n, less), m_ns, 64, 400.0};
  natural_merge_sort<Order::Power, SagTimeTT<MinStreak>, Count>(data, n, less, stats, minrun);
}

// ---------------------------------------------------------------------------------------------------------------
// SAG-B ("break-even"): the policy that replaced SAG-T v1 in the corrective round.
// SAG-T v1 evaluated its four candidates through the probe model after every block of >= MinStreak elements; on dev data
// that adaptation overhead (hundreds of ns per call) exceeded what it saved whenever comparisons were cheap (up to +60 %
// time vs the base).  The same time model implies ONE number per sort: the break-even block length
//        B(c, m) = min { k : (c + m) * probes_t0(k) < c * (k + 1) }
// (the shortest streak for which a pure exponential search beats a linear scan).  SAG-B is then memoryless:
//        after a block of length n:   t := 0 if n >= B else t_max
// i.e. one compare per block, no calls, no state besides (t, t_max, B).  c is measured by the sorter, m is a dev constant.
// ---------------------------------------------------------------------------------------------------------------
inline size_t breakeven_block_length(double c, double m) {
  for (size_t k = 2; k <= (size_t(1) << 20); ++k) {
    const double gallop = (c + m) * static_cast<double>(gallop_tests_model(0, k - 1, size_t(1) << 40));
    if (gallop < c * static_cast<double>(k)) return k;
  }
  return size_t(1) << 20;
}

struct SagBreakEvenT {
  static constexpr bool kNaive = false;
  struct State { size_t t, tmax, B; };
  static State start(size_t total) {
    const size_t l = ceil_log2(total);
    return {l * l, l * l, sag_time_params().breakeven};
  }
  static size_t t(const State& s) { return s.t; }
  NMS_INLINE static void observe(State& s, size_t n, u64, size_t) { s.t = (n >= s.B) ? 0 : s.tmax; }
};

template <bool Count = false, class T, class Less>
void sag_be_sort(T* data, size_t n, Less less, Stats* stats = nullptr, size_t minrun = 32, double m_ns = 8.0) {
  if (n < 4096) {
    natural_merge_sort<Order::Power, PolylogT, Count>(data, n, less, stats, minrun);
    return;
  }
  const double c = measure_comparison_ns(data, n, less);
  sag_time_params() = {c, m_ns, breakeven_block_length(c, m_ns), 400.0};
  natural_merge_sort<Order::Power, SagBreakEvenT, Count>(data, n, less, stats, minrun);
}

// ---------------------------------------------------------------------------------------------------------------
// SAG-C ("credit"): SAG-B forgets a long streak at the first short block, so nearly-sorted inputs (long streaks
// separated by one-element blocks) fall back to linear scans again and again.  SAG-C keeps t = 0 while there is credit:
//   a block of length n changes the credit by   c*n - (c+m)*probes0(n)        (time a pure exponential search saves
//                                                                              over a linear scan; negative for short blocks)
//   credit is clamped to [0, cap];  t = 0 while credit > 0, else t_max.      cap = capmult * m   (tolerates about
//                                                                              capmult consecutive short blocks).
// Fast path (credit = 0 and n < B): one compare.  Per block otherwise: one clz and a few flops; no calls, no loops.
// ---------------------------------------------------------------------------------------------------------------
struct SagCreditT {
  static constexpr bool kNaive = false;
  struct State { size_t t, tmax, B; double c, m, credit, cap; };
  static State start(size_t total) {
    const size_t l = ceil_log2(total);
    const SagTimeParams& p = sag_time_params();
    return {l * l, l * l, p.breakeven, p.c_ns, p.m_ns, 0.0, p.cap_ns};
  }
  static size_t t(const State& s) { return s.t; }
  NMS_INLINE static void observe(State& s, size_t n, u64, size_t) {
    if (s.credit <= 0.0 && n < s.B) return;
    const double probes = n == 1 ? 1.0 : 2.0 * static_cast<double>(ceil_log2(n));  // t = 0 search for a block of length n
    double cr = s.credit + s.c * static_cast<double>(n) - (s.c + s.m) * probes;
    if (cr > s.cap) cr = s.cap;
    if (cr < 0.0) cr = 0.0;
    s.credit = cr;
    s.t = cr > 0.0 ? 0 : s.tmax;
  }
};

template <bool Count = false, class T, class Less>
void sag_credit_sort(T* data, size_t n, Less less, Stats* stats = nullptr, size_t minrun = 32, double m_ns = 8.0, double capmult = 50.0) {
  if (n < 4096) {
    natural_merge_sort<Order::Power, PolylogT, Count>(data, n, less, stats, minrun);
    return;
  }
  const double c = measure_comparison_ns(data, n, less);
  sag_time_params() = {c, m_ns, breakeven_block_length(c, m_ns), capmult * m_ns};
  natural_merge_sort<Order::Power, SagCreditT, Count>(data, n, less, stats, minrun);
}

}  // namespace nms
