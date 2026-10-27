// diag.hpp -- v4 terminal diagnostics for the Merge Sort project (no new algorithm: it only RUNS and MEASURES the existing ones).
//
//   * record types (integer key + id, and a 64-byte string key + id) so that stability can be checked from the ids;
//   * measured input profile (run structure, distinct keys, descents, entropies) -- never inferred from a generator name;
//   * run_counted(): runs one variant (textbook / naive / base / SAG / std::stable_sort) on a COPY of the input and checks the result
//     against a reference stable order and against an independent stability test; the comparisons are counted twice (by the sorter's
//     own Stats and by an external counting comparator) so a counter bug shows up as a mismatch;
//   * Agg: folds the sorter's TraceEvents into block / merge / adaptation statistics without storing them;
//   * time_variant(): wall-clock timing of the UNCOUNTED instantiation (Count=false) on identical copies of the same input.
#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "entropy.hpp"
#include "generators.hpp"
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
#include "textbook_merge_sort.hpp"

namespace diag {

using nms::u64;
using nms::Stats;
using nms::TraceEvent;

// ---- records ---------------------------------------------------------------------------------------------------------------
struct IntRec { std::uint32_t key, id; };
struct StrRec { char key[64]; std::uint32_t id; };  // 64-byte key: shared prefix + 8 hex digits, zero padded; id = original position

inline bool operator<(const IntRec& a, const IntRec& b) { return a.key < b.key; }
inline bool operator<(const StrRec& a, const StrRec& b) { return std::memcmp(a.key, b.key, sizeof a.key) < 0; }

struct IntLess { bool operator()(const IntRec& a, const IntRec& b) const noexcept { return a.key < b.key; } };
struct StrLess { bool operator()(const StrRec& a, const StrRec& b) const noexcept { return std::memcmp(a.key, b.key, sizeof a.key) < 0; } };

template <class R> struct LessOf;
template <> struct LessOf<IntRec> { using type = IntLess; };
template <> struct LessOf<StrRec> { using type = StrLess; };

inline std::vector<IntRec> int_records(const std::vector<std::uint32_t>& k) {
  std::vector<IntRec> v(k.size());
  for (std::size_t i = 0; i < k.size(); ++i) v[i] = {k[i], static_cast<std::uint32_t>(i)};
  return v;
}

// keylen in [8, 63]: key = (keylen-8) times 'k', then the value as 8 lower-case hex digits (fixed width, so string order == numeric order).
inline std::vector<StrRec> str_records(const std::vector<std::uint32_t>& k, unsigned keylen) {
  std::vector<StrRec> v(k.size());
  static const char hex[] = "0123456789abcdef";
  for (std::size_t i = 0; i < k.size(); ++i) {
    std::memset(v[i].key, 0, sizeof v[i].key);
    std::memset(v[i].key, 'k', keylen - 8);
    for (int d = 0; d < 8; ++d) v[i].key[keylen - 8 + d] = hex[(k[i] >> (28 - 4 * d)) & 15u];
    v[i].id = static_cast<std::uint32_t>(i);
  }
  return v;
}

inline std::string label(const IntRec& r) { return std::to_string(r.key); }
inline std::string label(const StrRec& r) {  // the 8 varying digits (the shared prefix is not printed)
  const std::size_t len = std::strlen(r.key);
  return std::string(r.key + (len > 8 ? len - 8 : 0));
}

template <class R>
bool same_records(const std::vector<R>& a, const std::vector<R>& b) {
  return a.size() == b.size() && (a.empty() || std::memcmp(a.data(), b.data(), a.size() * sizeof(R)) == 0);
}

// Independent stability test: output is sorted by key AND equal keys appear in increasing original id (ids were assigned in input order).
template <class R>
bool sorted_and_stable(const std::vector<R>& v) {
  for (std::size_t i = 1; i < v.size(); ++i) {
    if (v[i] < v[i - 1]) return false;
    if (!(v[i - 1] < v[i]) && v[i].id < v[i - 1].id) return false;
  }
  return true;
}

// ---- measured input profile -------------------------------------------------------------------------------------------------
struct Profile {
  std::size_t n = 0, distinct = 0, runs = 0, asc_runs = 0, desc_runs = 0, run_min = 0, run_max = 0, descents = 0;
  double run_mean = 0, H = 0, Hstar = 0, log2n = 0;
  bool sorted = false, non_increasing = false;
};

// Everything here is measured from the data with the definitions in docs/TERMINAL_DEMO.md.  Runs are the sorter's own:
// maximal non-decreasing or strictly decreasing blocks (greedy, left to right).
template <class R>
Profile profile_of(const std::vector<R>& a) {
  Profile p;
  p.n = a.size();
  if (a.empty()) return p;
  std::size_t asc_pairs = 0;
  for (std::size_t i = 1; i < a.size(); ++i) {
    if (a[i] < a[i - 1]) ++p.descents;
    if (a[i - 1] < a[i]) ++asc_pairs;
  }
  p.sorted = p.descents == 0;
  p.non_increasing = asc_pairs == 0;
  const std::vector<std::size_t> lens = nms::natural_run_lengths(a);
  p.runs = lens.size();
  p.run_min = *std::min_element(lens.begin(), lens.end());
  p.run_max = *std::max_element(lens.begin(), lens.end());
  p.run_mean = static_cast<double>(p.n) / static_cast<double>(p.runs);
  std::size_t pos = 0;
  for (std::size_t l : lens) {
    if (l >= 2 && a[pos + 1] < a[pos]) ++p.desc_runs; else ++p.asc_runs;
    pos += l;
  }
  std::vector<R> s = a;
  std::sort(s.begin(), s.end(), [](const R& x, const R& y) { return x < y; });
  p.distinct = 1;
  for (std::size_t i = 1; i < s.size(); ++i) if (s[i - 1] < s[i]) ++p.distinct;
  p.H = nms::entropy_of_lengths(lens, p.n);
  p.Hstar = nms::dual_run_entropy(a);
  p.log2n = std::log2(static_cast<double>(p.n));
  return p;
}

// ---- trace aggregation -------------------------------------------------------------------------------------------------------
struct Agg {
  u64 runs = 0, runs_extended = 0, merges = 0, trivial = 0, blocks = 0, block_sum = 0, block_min = ~0ull, block_max = 0;
  u64 t_changes = 0, merges_adapted = 0;
  u64 bins[5] = {0, 0, 0, 0, 0};  // block length 1 | 2-3 | 4-15 | 16-63 | >= 64
  bool cur_adapted = false;
  static void cb(void* self, const TraceEvent& e) { static_cast<Agg*>(self)->add(e); }
  void add(const TraceEvent& e) {
    switch (e.kind) {
      case nms::Ev::Run: ++runs; if (e.c > e.b) ++runs_extended; break;
      case nms::Ev::MergeBegin: ++merges; cur_adapted = false; break;
      case nms::Ev::MergeTrivial: ++trivial; break;
      case nms::Ev::Block: {
        ++blocks; block_sum += e.a; block_min = std::min<u64>(block_min, e.a); block_max = std::max<u64>(block_max, e.a);
        ++bins[e.a == 1 ? 0 : e.a < 4 ? 1 : e.a < 16 ? 2 : e.a < 64 ? 3 : 4];
        break;
      }
      case nms::Ev::TChange: ++t_changes; if (!cur_adapted) { cur_adapted = true; ++merges_adapted; } break;
      case nms::Ev::MergeEnd: break;
    }
  }
};

// ---- running one variant -----------------------------------------------------------------------------------------------------
enum class V { Textbook, Naive, Base, Sag, StdStable };
inline const char* name(V v) {
  switch (v) {
    case V::Textbook: return "textbook";
    case V::Naive: return "naive";
    case V::Base: return "base";
    case V::Sag: return "SAG";
    case V::StdStable: return "std::stable_sort";
  }
  return "?";
}
inline bool is_natural(V v) { return v == V::Naive || v == V::Base || v == V::Sag; }

template <class Less>
struct ExtCount {  // independent comparison counter wrapped around the user's comparator
  Less less;
  u64* c;
  template <class T>
  bool operator()(const T& x, const T& y) const { ++*c; return less(x, y); }
};

struct Outcome {
  V v = V::Base;
  u64 ext_cmp = 0;        // comparisons counted by the external wrapper (all variants)
  Stats st;               // the sorter's own exact counters (natural variants only)
  bool has_stats = false;
  bool matches_ref = false;  // byte-identical to the reference stable order
  bool stable = false;       // independent sorted+stable test
  bool counters_agree = true;  // natural variants: Stats::comparisons() == ext_cmp
};

template <class TP, class R, class C>
void natural(std::vector<R>& a, C& cl, Stats& st, std::size_t minrun, const nms::TraceSink& sink) {
  if (sink.fn) nms::natural_merge_sort_traced<nms::Order::Power, TP>(a.data(), a.size(), cl, &st, minrun, sink);
  else nms::natural_merge_sort<nms::Order::Power, TP, true>(a.data(), a.size(), cl, &st, minrun);
}

// Runs `v` on a copy of `in`.  `ref` = std::stable_sort of the same input.  trace/ctx (optional) receive TraceEvents (natural variants).
template <class R>
Outcome run_counted(V v, const std::vector<R>& in, const std::vector<R>& ref, std::size_t minrun,
                    void (*trace)(void*, const TraceEvent&) = nullptr, void* ctx = nullptr) {
  using L = typename LessOf<R>::type;
  Outcome o;
  o.v = v;
  std::vector<R> a = in;
  ExtCount<L> cl{L{}, &o.ext_cmp};
  const nms::TraceSink sink{trace, ctx};
  switch (v) {
    case V::Textbook: nms::merge_sort_topdown(a.data(), a.size(), cl); break;
    case V::StdStable: std::stable_sort(a.begin(), a.end(), cl); break;
    case V::Naive: natural<nms::NaiveT>(a, cl, o.st, minrun, sink); break;
    case V::Base: natural<nms::PolylogT>(a, cl, o.st, minrun, sink); break;
    case V::Sag: natural<nms::SagT>(a, cl, o.st, minrun, sink); break;
  }
  o.has_stats = is_natural(v);
  o.matches_ref = same_records(a, ref);
  o.stable = sorted_and_stable(a);
  if (o.has_stats) o.counters_agree = o.st.comparisons() == o.ext_cmp;
  return o;
}

// ---- timing ------------------------------------------------------------------------------------------------------------------
struct TimeSummary { double median_ms = 0, min_ms = 0, max_ms = 0; unsigned reps = 0; bool correct = true; };

// One untimed warm-up, then `reps` timed runs; every run sorts a fresh copy made OUTSIDE the timed region; Count=false instantiations.
template <class R>
TimeSummary time_variant(V v, const std::vector<R>& in, const std::vector<R>& ref, std::size_t minrun, unsigned reps) {
  using L = typename LessOf<R>::type;
  TimeSummary ts;
  ts.reps = reps;
  std::vector<double> ms;
  for (unsigned r = 0; r <= reps; ++r) {
    std::vector<R> a = in;
    const auto t0 = std::chrono::steady_clock::now();
    switch (v) {
      case V::Textbook: nms::merge_sort_topdown(a.data(), a.size(), L{}); break;
      case V::StdStable: std::stable_sort(a.begin(), a.end(), L{}); break;
      case V::Naive: nms::natural_merge_sort<nms::Order::Power, nms::NaiveT, false>(a.data(), a.size(), L{}, nullptr, minrun); break;
      case V::Base: nms::natural_merge_sort<nms::Order::Power, nms::PolylogT, false>(a.data(), a.size(), L{}, nullptr, minrun); break;
      case V::Sag: nms::natural_merge_sort<nms::Order::Power, nms::SagT, false>(a.data(), a.size(), L{}, nullptr, minrun); break;
    }
    const auto t1 = std::chrono::steady_clock::now();
    if (!same_records(a, ref)) ts.correct = false;
    if (r > 0) ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
  }
  std::sort(ms.begin(), ms.end());
  if (!ms.empty()) {
    ts.min_ms = ms.front();
    ts.max_ms = ms.back();
    ts.median_ms = ms.size() % 2 ? ms[ms.size() / 2] : 0.5 * (ms[ms.size() / 2 - 1] + ms[ms.size() / 2]);
  }
  return ts;
}

}  // namespace diag
