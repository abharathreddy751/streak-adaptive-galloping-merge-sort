// natural_merge_sort.hpp  --  BASE PAPER IMPLEMENTATION (no extension code in this file)
//
// Re-implementation, from the paper text, of the stable natural merge sorts and the
// galloping merge routine analysed in
//   E. Ghasemi, V. Juge, G. Khalighinejad, H. Yazdanyar,
//   "Galloping in Fast-Growth Natural Merge Sorts", Algorithmica 87(2):242-291 (2025),
//   DOI 10.1007/s00453-024-01285-6   (preprint: arXiv:2012.03996).
//
// MACSE501 topic: Merge Sort (Module 3).  This is Merge Sort with (i) run detection,
// (ii) a stack policy choosing which adjacent runs to merge, and (iii) a merge step that
// finds block boundaries by "galloping".
//
// What is here
//   * count_run / binary insertion (run detection, minimum run length)
//   * merge-order policies: PowerSort (Munro-Wild) and TimSort's stack rule (paper Alg. 2)
//   * gallop_count():    McIlroy's mixed strategy "t linear probes, then exponential+binary search"
//   * merge_core<D>():   stable galloping merge, forward (D=+1) or backward (D=-1)
//   * t-policies:        NaiveT (plain merge), FixedT<K>, PolylogT  (t = ceil(log2(a+b))^2, the paper's variant)
//   * Stats:             exact comparison / move counters (compile-time switch, zero cost when off)
//
// Deliberate deviations from the paper (also in docs/BASE_PAPER_IMPLEMENTATION.md):
//   * runs shorter than `minrun` are extended with binary insertion sort (as real TimSort/PowerSort do);
//   * one extra comparison per merge checks whether the two runs are already in order (skips the merge);
//   * the merge copies the SMALLER run to a buffer (as TimSort does) and merges forward or backward;
//   * no TimSort-specific low-level shortcuts (prefix/suffix trimming, 7-element min_gallop state).
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

namespace nms {

using std::size_t;
using u64 = std::uint64_t;

// Hot-path helpers are force-inlined for ALL variants (base and extension alike); without this GCC leaves
// small per-block helpers as calls, which dominates the cost of short blocks.
#if defined(__GNUC__) || defined(__clang__)
#define NMS_INLINE inline __attribute__((always_inline))
#define NMS_NOINLINE __attribute__((noinline))
#else
#define NMS_INLINE inline
#define NMS_NOINLINE
#endif

// ceil(log2(x)); ceil_log2(0)=ceil_log2(1)=0.
inline unsigned ceil_log2(size_t x) {
  return x <= 1 ? 0u : static_cast<unsigned>(64 - __builtin_clzll(static_cast<unsigned long long>(x - 1)));
}

// ----------------------------------------------------------------------------------------
// Instrumentation
// ----------------------------------------------------------------------------------------
struct MergeRecord {  // one entry per non-trivial merge (tests only)
  size_t a, b, t_start;
  u64 comparisons;
  u64 blocks;
};

struct Stats {
  u64 run_comparisons = 0;    // comparisons spent on run detection + insertion sort
  u64 merge_comparisons = 0;  // comparisons spent inside merges (incl. the 1 'already ordered?' check)
  u64 merges = 0;             // merge_at() calls
  u64 trivial_merges = 0;     // merges skipped because the runs were already in order
  u64 blocks = 0;             // blocks (streaks) discovered by galloping
  u64 element_moves = 0;      // elements copied into/out of the buffer and between positions
  size_t max_buffer = 0;      // max buffer length (elements) allocated
  std::vector<MergeRecord>* log = nullptr;
  u64 comparisons() const { return run_comparisons + merge_comparisons; }
};

template <class Less, bool Count>
struct CountingLess {
  Less less;
  mutable u64* counter = nullptr;
  template <class T>
  bool operator()(const T& x, const T& y) const {
    if constexpr (Count) ++*counter;
    return less(x, y);
  }
};

// Comparator with swapped arguments (used for backward merging).
template <class C>
struct SwappedLess {
  const C& c;
  template <class T>
  bool operator()(const T& x, const T& y) const { return c(y, x); }
};

// ----------------------------------------------------------------------------------------
// Galloping search (McIlroy's mixed strategy, paper Section 2)
// ----------------------------------------------------------------------------------------
// base[D*0], base[D*1], ... is a sequence of `len` elements; pred is monotone along it
// (false ... false true ... true).  Returns the number of leading elements for which pred is false.
// `tests` is incremented by the number of pred evaluations.
//
// Strategy (secret integer m = 1 + #false elements; test(x) <=> pred(element at 1-based position x)):
//   C1: positions 1..t tested one by one;
//   C2: if still undecided, B1 translated by t: positions t+1, t+2, t+4, ... until pred is true,
//       then binary search inside the last interval.
// With t = 0 this uses 2*ceil(log2 m) tests for m >= 2 (1 test for m = 1), cf. paper Table 1.
template <int D, class T, class Pred>
NMS_INLINE size_t gallop_count(const T* base, size_t len, size_t t, Pred&& pred, u64& tests) {
  const size_t lin = t < len ? t : len;
  size_t x = 0;
  for (; x < lin; ++x) {
    ++tests;
    if (pred(base[D * static_cast<std::ptrdiff_t>(x)])) return x;
  }
  if (x == len) return len;
  size_t lo = t;  // positions 1..lo are known to be false
  size_t step = 1;
  size_t hi;
  for (;;) {
    size_t p = t + step;
    if (p >= len) p = len;
    ++tests;
    if (pred(base[D * static_cast<std::ptrdiff_t>(p - 1)])) { hi = p; break; }
    lo = p;
    if (p == len) return len;
    step <<= 1;
  }
  while (hi - lo > 1) {
    const size_t mid = lo + (hi - lo) / 2;
    ++tests;
    if (pred(base[D * static_cast<std::ptrdiff_t>(mid - 1)])) hi = mid; else lo = mid;
  }
  return hi - 1;
}

// Number of tests gallop_count() would perform for a monotone predicate whose first `true` is at
// 0-based index k (k == len means "never true").  Pure arithmetic; used for analysis and by extensions.
inline u64 gallop_tests_model(size_t t, size_t k, size_t len) {
  const size_t lin = t < len ? t : len;
  if (k < lin) return k + 1;
  u64 tests = lin;
  if (lin == len) return tests;
  size_t lo = t, step = 1, hi;
  for (;;) {
    size_t p = t + step;
    if (p >= len) p = len;
    ++tests;
    if (k + 1 <= p) { hi = p; break; }
    lo = p;
    if (p == len) return tests;
    step <<= 1;
  }
  while (hi - lo > 1) {
    const size_t mid = lo + (hi - lo) / 2;
    ++tests;
    if (k + 1 <= mid) hi = mid; else lo = mid;
  }
  return tests;
}

// ----------------------------------------------------------------------------------------
// t-policies.  A policy provides a per-merge State, the initial t, and an observe() hook that is
// called after every discovered block (the base policies ignore it).  observe(state, n, tests, len):
// n = block length, tests = comparisons spent discovering it, len = run length remaining when the
// discovery started (including the block's first element, which is known without a comparison).
// ----------------------------------------------------------------------------------------
struct NaiveT {  // plain two-pointer merge, no galloping
  static constexpr bool kNaive = true;
  struct State { size_t t; };
  static State start(size_t) { return {0}; }
  static size_t t(const State& s) { return s.t; }
  static void observe(State&, size_t, u64, size_t) {}
};

template <size_t K>
struct FixedT {  // t = K for every merge
  static constexpr bool kNaive = false;
  struct State { size_t t; };
  static State start(size_t) { return {K}; }
  static size_t t(const State& s) { return s.t; }
  static void observe(State&, size_t, u64, size_t) {}
};

struct PolylogT {  // paper Definition 47: t = ceil(log2(a+b))^2
  static constexpr bool kNaive = false;
  struct State { size_t t; };
  static State start(size_t total) {
    const size_t l = ceil_log2(total);
    return {l * l};
  }
  static size_t t(const State& s) { return s.t; }
  static void observe(State&, size_t, u64, size_t) {}
};

// ----------------------------------------------------------------------------------------
// Merge-order policies
// ----------------------------------------------------------------------------------------
enum class Order { Power, Tim };

// PowerSort node power (Munro & Wild); runs [s1, s1+n1) and [s1+n1, s1+n1+n2) inside an array of n.
inline unsigned node_power(size_t s1, size_t n1, size_t n2, size_t n) {
  unsigned result = 0;
  size_t a = 2 * s1 + n1;       // twice the midpoint of run 1
  size_t b = a + n1 + n2;       // twice the midpoint of run 2
  for (;;) {
    ++result;
    if (a >= n) { a -= n; b -= n; }
    else if (b >= n) break;
    a <<= 1;
    b <<= 1;
  }
  return result;
}

// ----------------------------------------------------------------------------------------
// The sorter
// ----------------------------------------------------------------------------------------
template <class T, class Less, class TPol, Order Ord, bool Count>
class Sorter {
  static_assert(std::is_trivially_copyable_v<T>, "block moves use memmove");
  using Cmp = CountingLess<Less, Count>;

 public:
  Sorter(T* data, size_t n, Less less, Stats* stats, size_t minrun)
      : a_(data), n_(n), cmp_{less, nullptr}, st_(stats), minrun_(minrun < 1 ? 1 : minrun) {
    if (!st_) st_ = &local_;
  }

  void run() {
    if (n_ < 2) return;
    if constexpr (Ord == Order::Power) run_power(); else run_tim();
  }

 private:
  struct Rn { size_t start, len; };

  // ---- run detection -------------------------------------------------------------------
  size_t count_run(size_t pos) {
    T* p = a_ + pos;
    const size_t rem = n_ - pos;
    if (rem == 1) return 1;
    size_t r = 2;
    if (cmp_(p[1], p[0])) {                      // strictly descending
      while (r < rem && cmp_(p[r], p[r - 1])) ++r;
      std::reverse(p, p + r);                    // strict => stable
      if constexpr (Count) st_->element_moves += r;
    } else {                                     // non-descending
      while (r < rem && !cmp_(p[r], p[r - 1])) ++r;
    }
    return r;
  }

  void binary_insertion(size_t pos, size_t total, size_t sorted) {  // a_[pos, pos+total), first `sorted` already ordered
    T* p = a_ + pos;
    for (size_t i = sorted; i < total; ++i) {
      const T pivot = p[i];
      size_t lo = 0, hi = i;
      while (lo < hi) {
        const size_t mid = lo + (hi - lo) / 2;
        if (cmp_(pivot, p[mid])) hi = mid; else lo = mid + 1;
      }
      if (lo < i) std::memmove(p + lo + 1, p + lo, (i - lo) * sizeof(T));
      p[lo] = pivot;
      if constexpr (Count) st_->element_moves += (i - lo) + 1;
    }
  }

  Rn next_run(size_t pos) {
    if constexpr (Count) cmp_.counter = &st_->run_comparisons;
    size_t len = count_run(pos);
    if (len < minrun_) {
      const size_t force = (n_ - pos) < minrun_ ? (n_ - pos) : minrun_;
      binary_insertion(pos, force, len);
      len = force;
    }
    return {pos, len};
  }

  // ---- merge-order policies ------------------------------------------------------------
  void run_power() {
    Rn runs[128];
    unsigned pw[128];
    int sp = 0;
    size_t pos = 0;
    Rn x = next_run(pos);
    pos += x.len;
    while (pos < n_) {
      const Rn y = next_run(pos);
      pos += y.len;
      const unsigned p = node_power(x.start, x.len, y.len, n_);
      while (sp > 0 && pw[sp - 1] >= p) {
        --sp;
        merge_at(runs[sp].start, runs[sp].len, x.len);
        x = {runs[sp].start, runs[sp].len + x.len};
      }
      runs[sp] = x;
      pw[sp] = p;
      ++sp;
      x = y;
    }
    while (sp > 0) {
      --sp;
      merge_at(runs[sp].start, runs[sp].len, x.len);
      x = {runs[sp].start, runs[sp].len + x.len};
    }
  }

  // TimSort's stack rule as printed in the paper (Algorithm 2).  The comparison signs were lost in the
  // PDF-to-text conversion; they are reconstructed from the (fixed) Java TimSort rule.
  void run_tim() {
    std::vector<Rn> s;
    size_t pos = 0;
    auto len = [&](size_t i) { return s[s.size() - i].len; };  // i = 1 is the top (R_h)
    auto merge_top = [&](size_t below_top) {                   // merge runs at positions h-below_top and h-below_top+1 (1-based from top)
      const size_t h = s.size();
      const size_t i = h - below_top - 1;                      // 0-based index of the left run
      merge_at(s[i].start, s[i].len, s[i + 1].len);
      s[i].len += s[i + 1].len;
      s.erase(s.begin() + static_cast<std::ptrdiff_t>(i) + 1);
    };
    for (;;) {
      const size_t h = s.size();
      if (h >= 3 && len(3) < len(1)) merge_top(2);                       // merge R_{h-2}, R_{h-1}
      else if (h >= 2 && len(2) <= len(1)) merge_top(1);                 // merge R_{h-1}, R_h
      else if (h >= 3 && len(3) <= len(2) + len(1)) merge_top(1);
      else if (h >= 4 && len(4) <= len(3) + len(2)) merge_top(1);
      else if (pos < n_) { const Rn r = next_run(pos); pos += r.len; s.push_back(r); }
      else break;
    }
    while (s.size() >= 2) merge_top(1);
  }

  // ---- merging -------------------------------------------------------------------------
  void ensure_buffer(size_t m) {
    if (buf_.size() < m) {
      buf_.resize(m);
      if (m > st_->max_buffer) st_->max_buffer = m;
    }
  }

  template <int D>
  NMS_INLINE static void blit(T* dst, const T* src, size_t cnt) {  // dst/src point at logical element 0 of the block
    if (cnt <= 8) {
      for (size_t i = 0; i < cnt; ++i) dst[D * static_cast<std::ptrdiff_t>(i)] = src[D * static_cast<std::ptrdiff_t>(i)];
    } else if constexpr (D > 0) {
      std::memmove(dst, src, cnt * sizeof(T));
    } else {
      std::memmove(dst - (cnt - 1), src - (cnt - 1), cnt * sizeof(T));
    }
  }

  // Stable merge.  Logical sequences: L[i]=Lp[D*i] (buffered, la elements), R[j]=Rp[D*j] (in place, lr
  // elements), output out[k]=Op[D*k].  `c(x,y)` means "x must be output strictly before y"; ties go to L.
  // Precondition: both sequences are sorted w.r.t. c and R[0] precedes-or-equals ordering was not checked.
  template <int D, class C>
  void merge_core(const T* Lp, size_t la, T* Rp, size_t lr, T* Op, const C& c, size_t total) {
    typename TPol::State ts = TPol::start(total);
    size_t i = 0, j = 0, k = 0;
    u64 tests = 0;
    bool cur_L = !c(Rp[0], Lp[0]);  // 1 comparison: who owns the first block?  (ties -> L)
    for (;;) {
      if (cur_L) {
        const T rj = Rp[D * static_cast<std::ptrdiff_t>(j)];
        const size_t len = la - i;
        const u64 t0 = tests;
        // L[i] is already known to precede R[j] (previous comparison), so only the tail is searched.
        const size_t n = 1 + gallop_count<D>(Lp + D * static_cast<std::ptrdiff_t>(i + 1), len - 1, TPol::t(ts),
                                             [&](const T& e) { return c(rj, e); }, tests);
        if constexpr (Count) ++st_->blocks;
        blit<D>(Op + D * static_cast<std::ptrdiff_t>(k), Lp + D * static_cast<std::ptrdiff_t>(i), n);
        if constexpr (Count) st_->element_moves += n;
        i += n; k += n;
        if (i == la) break;                       // remaining R elements are already in place
        TPol::observe(ts, n, tests - t0, len);
        cur_L = false;
      } else {
        const T li = Lp[D * static_cast<std::ptrdiff_t>(i)];
        const size_t len = lr - j;
        const u64 t0 = tests;
        // R[j] is already known to precede L[i] (previous comparison), so only the tail is searched.
        const size_t n = 1 + gallop_count<D>(Rp + D * static_cast<std::ptrdiff_t>(j + 1), len - 1, TPol::t(ts),
                                             [&](const T& e) { return !c(e, li); }, tests);
        if constexpr (Count) ++st_->blocks;
        blit<D>(Op + D * static_cast<std::ptrdiff_t>(k), Rp + D * static_cast<std::ptrdiff_t>(j), n);
        if constexpr (Count) st_->element_moves += n;
        j += n; k += n;
        if (j == lr) {                            // R exhausted: flush the rest of L
          blit<D>(Op + D * static_cast<std::ptrdiff_t>(k), Lp + D * static_cast<std::ptrdiff_t>(i), la - i);
          if constexpr (Count) st_->element_moves += la - i;
          break;
        }
        TPol::observe(ts, n, tests - t0, len);
        cur_L = true;
      }
    }
  }

  // Plain two-pointer stable merge (NaiveT): same buffer discipline, no galloping.
  template <int D, class C>
  void merge_plain(const T* Lp, size_t la, T* Rp, size_t lr, T* Op, const C& c) {
    size_t i = 0, j = 0, k = 0;
    while (i < la && j < lr) {
      if (c(Rp[D * static_cast<std::ptrdiff_t>(j)], Lp[D * static_cast<std::ptrdiff_t>(i)])) {
        Op[D * static_cast<std::ptrdiff_t>(k++)] = Rp[D * static_cast<std::ptrdiff_t>(j++)];
      } else {
        Op[D * static_cast<std::ptrdiff_t>(k++)] = Lp[D * static_cast<std::ptrdiff_t>(i++)];
      }
    }
    if (i < la) blit<D>(Op + D * static_cast<std::ptrdiff_t>(k), Lp + D * static_cast<std::ptrdiff_t>(i), la - i);
    if constexpr (Count) st_->element_moves += k + (la - i);
  }

  void merge_at(size_t start, size_t a, size_t b) {
    T* A = a_ + start;
    T* B = A + a;
    if constexpr (Count) { cmp_.counter = &st_->merge_comparisons; ++st_->merges; }
    const u64 c0 = st_->merge_comparisons;
    // Already in order?  (1 comparison)  -- deviation: not part of the paper's cost model.
    if (!cmp_(*B, *(B - 1))) { if constexpr (Count) ++st_->trivial_merges; return; }
    const size_t total = a + b;
    const u64 blocks0 = st_->blocks;
    size_t t_start = 0;
    if (a <= b) {
      ensure_buffer(a);
      std::memcpy(buf_.data(), A, a * sizeof(T));
      if constexpr (Count) st_->element_moves += a;
      if constexpr (TPol::kNaive) merge_plain<+1>(buf_.data(), a, B, b, A, cmp_);
      else { t_start = TPol::t(TPol::start(total)); merge_core<+1>(buf_.data(), a, B, b, A, cmp_, total); }
    } else {
      ensure_buffer(b);
      std::memcpy(buf_.data(), B, b * sizeof(T));
      if constexpr (Count) st_->element_moves += b;
      const SwappedLess<Cmp> sw{cmp_};
      if constexpr (TPol::kNaive) merge_plain<-1>(buf_.data() + (b - 1), b, A + (a - 1), a, A + (total - 1), sw);
      else { t_start = TPol::t(TPol::start(total)); merge_core<-1>(buf_.data() + (b - 1), b, A + (a - 1), a, A + (total - 1), sw, total); }
    }
    if constexpr (Count) {
      if (st_->log) st_->log->push_back({a, b, t_start, st_->merge_comparisons - c0, st_->blocks - blocks0});
    }
  }

  T* a_;
  size_t n_;
  Cmp cmp_;
  Stats* st_;
  Stats local_;
  size_t minrun_;
  std::vector<T> buf_;
};

// Convenience entry point.
//   Ord    : Order::Power (PowerSort stack rule) or Order::Tim (TimSort stack rule, paper Alg. 2)
//   TPol   : NaiveT | FixedT<K> | PolylogT | (extension policies)
//   Count  : true => fill `stats` with exact counters
template <Order Ord, class TPol, bool Count = false, class T, class Less>
void natural_merge_sort(T* data, size_t n, Less less, Stats* stats = nullptr, size_t minrun = 32) {
  Sorter<T, Less, TPol, Ord, Count> s(data, n, less, stats, minrun);
  s.run();
}

}  // namespace nms
