// sag_policy.hpp -- PROPOSED EXTENSION (not part of the base paper): Streak-Adaptive Galloping (SAG).
//
// Setting.  The base paper's merge step finds each block (streak of consecutive output elements from the same
// run) with the t-galloping routine; its proven policy fixes  t = ceil(log2(a+b))^2  for the whole merge,
// independently of the data.  At the top merge levels this t is in the hundreds, so every block shorter than
// about t is still discovered one comparison per element (cost_t(m) = m for m <= t+2), even when all blocks are
// long enough that t = 0 (pure exponential search, 2*ceil(log2 m) comparisons) would be far cheaper.
//
// Proposal.  Keep the paper's t as the *safe starting point and upper cap* of every merge, but adapt t while the
// merge runs, using only quantities the merge already has: the length n of the block it just discovered, the
// number of comparisons `tests` that discovery cost, and the remaining run length.  After a block, evaluate
// the exact cost model (gallop_tests_model, the paper's cost_t) for three candidate values of t:
//        t/2 ,   t ,   min(t_max, 2t+2)           (and, if Jump: also 0)
// and move to the cheapest one (ties keep the current t).  Multiplicative steps mean a mis-set t is repaired in
// O(log t) blocks; the cap t_max keeps the paper's no-regret overhead guarantee as the limiting behaviour; the
// per-merge reset to t_max keeps merges independent (TimSort's global state is what Prop. 42/43 exploit).
//
// Fast path.  Streaks shorter than MinStreak that were found linearly (tests <= n) teach nothing, so random
// data pays one comparison of a branch per block and behaves exactly like the base policy.
//
// Costs: no extra memory; O(1) extra arithmetic per long block; no extra comparisons of keys (all counterfactuals
// are evaluated analytically).  Worst case: t >= 0 always, so every block costs <= (4/3)*its length
// (cost_0(m) <= 4m/3), i.e. at most 33 % above naive merging on adversarial streak sequences — weaker than the
// base policy's O(n) additive guarantee; this is the price of adaptivity and is measured, not hidden.
#pragma once

#include <algorithm>

#include "natural_merge_sort.hpp"

namespace nms {

template <unsigned MinStreak, bool JumpToZero = false>
struct SagTT {
  static constexpr bool kNaive = false;
  struct State { size_t t, tmax; };

  static State start(size_t total) {
    const size_t l = ceil_log2(total);
    return {l * l, l * l};
  }
  static size_t t(const State& s) { return s.t; }

  // n = block length, tests = comparisons this discovery cost, len = run elements remaining (incl. first).
  // The fast path is force-inlined into the merge loop; the adaptation itself is out of line.
  NMS_INLINE static void observe(State& s, size_t n, u64 tests, size_t len) {
    if (__builtin_expect(n < MinStreak && tests <= n, 1)) return;  // short streak found linearly: nothing to learn
    s.t = adapt(s.t, s.tmax, n, tests, len);  // by value: the state never escapes into memory in the hot loop
  }

  // Returns the new t.  Pure function of its arguments.
  NMS_NOINLINE static size_t adapt(size_t t, size_t tmax, size_t n, u64 tests, size_t len) {
    const size_t k = n - 1;                   // the tail search saw k "false" elements ...
    const size_t tail = len - 1;              // ... out of `tail` elements
    size_t best_t = t;
    u64 best = tests;
    auto consider = [&](size_t cand) {
      if (cand == t) return;
      const u64 c = gallop_tests_model(cand, k, tail);
      if (c < best) { best = c; best_t = cand; }
    };
    consider(t / 2);
    consider(std::min(tmax, 2 * t + 2));
    if constexpr (JumpToZero) consider(0);
    return best_t;
  }
};

// Ablation: multiplicative halving/doubling only (never jumps to 0).
using SagHalving = SagTT<8, false>;
// FINAL proposed extension: MinStreak = 64, JumpToZero = true.  Chosen on DEVELOPMENT data only, in two rounds
// (both documented in docs/PROPOSED_METHOD.md and docs/EXPERIMENT_LOG.md):
//   round 1 (comparison counts, results/raw/dev_counts.csv) preferred MinStreak 16;
//   round 2 (dev timing, results/raw/dev_selection.csv) showed that 16 pays up to +29 % time on structure-free
//   inputs because one lucky long streak sends t to 0 for the rest of the merge, so the final rule is: maximise the
//   comparison-dominated (heavy) time gain subject to  <= +1 % comparisons on high-entropy cells,
//   <= +3 % heavy-regime time and <= +10 % plain-regime time on structure-free cells.  Only MinStreak >= 64 qualified.
// Held-out data were not consulted.
using SagT = SagTT<64, true>;

}  // namespace nms
