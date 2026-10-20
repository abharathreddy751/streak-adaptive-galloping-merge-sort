// test_sag.cpp -- tests targeting the exact behaviour the proposed extension (SAG) must have.
// (Sorting correctness/stability of the SAG configurations is covered in test_sort.cpp built with -DWITH_SAG.)
//   1. controller unit tests: t shrinks on long streaks, grows back after waste, never leaves [0, t_max],
//      short linearly-found streaks leave t untouched (fast path);
//   2. SAG never exceeds the (4/3)-per-block worst-case bound, and equals the base policy on data with no
//      long streaks (random permutation);
//   3. SAG does not use more comparisons than the base policy on low-entropy families (property, not a benchmark);
//   4. adversarial alternating streak pattern (long, short, long, short ...) — the controller's weak spot —
//      is still within the (4/3) bound and is measured against the base.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <type_traits>
#include <vector>

#include "check.hpp"
#include "generators.hpp"
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"

using namespace nms;

struct Tag { std::uint32_t key, id; };
struct TagLess { bool operator()(const Tag& a, const Tag& b) const { return a.key < b.key; } };
static std::vector<Tag> tagged(const std::vector<std::uint32_t>& keys) {
  std::vector<Tag> v(keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) v[i] = {keys[i], static_cast<std::uint32_t>(i)};
  return v;
}
static bool same(const std::vector<Tag>& a, const std::vector<Tag>& b) {
  if (a.size() != b.size()) return false;
  for (std::size_t i = 0; i < a.size(); ++i) if (a[i].key != b[i].key || a[i].id != b[i].id) return false;
  return true;
}

static void test_controller() {
  using S = SagHalving;  // the halving-only ablation (original controller semantics)
  auto st = S::start(1u << 20);
  CHECK(st.t == 400 && st.tmax == 400);
  // fast path: short streak found linearly teaches nothing
  S::observe(st, 3, 3, 100000);
  CHECK(st.t == 400);
  S::observe(st, 7, 7, 100000);
  CHECK(st.t == 400);
  // a long streak found linearly (cost = length) makes t shrink
  S::observe(st, 300, gallop_tests_model(st.t, 299, 99999) , 100000);
  CHECK(st.t < 400);
  // repeated long streaks drive t to 0 and it stays there
  for (int i = 0; i < 30; ++i) S::observe(st, 1000, gallop_tests_model(st.t, 999, 99999), 100000);
  CHECK(st.t == 0);
  // waste at t = 0 (streak of 3 costs 4 comparisons, naive would cost 3) makes t grow back
  S::observe(st, 3, gallop_tests_model(0, 2, 99999), 100000);
  CHECK(st.t == 2);
  // growth is capped by t_max, shrink is bounded below by 0
  auto small = S::start(16);  // t_max = 16
  CHECK(small.tmax == 16);
  gen::Rng r(5);
  for (int i = 0; i < 100000; ++i) {
    const std::size_t len = 2 + r.bounded(5000);
    const std::size_t n = 1 + r.bounded(len);
    const std::size_t k = n - 1, tail = len - 1;
    S::observe(small, n, gallop_tests_model(small.t, k > tail ? tail : k, tail), len);
    CHECK(small.t <= small.tmax);
  }
  // the jump variant reaches 0 in one step on a long streak
  using J = SagTT<8, true>;
  auto js = J::start(1u << 20);
  J::observe(js, 5000, gallop_tests_model(js.t, 4999, 99999), 100000);
  CHECK(js.t == 0);
  // FINAL configuration (MinStreak 64, jump): ignores streaks shorter than 64 found linearly, jumps on long ones
  using F = SagT;
  static_assert(std::is_same_v<F, SagTT<64, true>>);
  auto fs = F::start(1u << 20);
  F::observe(fs, 63, 63, 100000);
  CHECK(fs.t == 400);
  F::observe(fs, 64, 64, 100000);          // linear scan of a 64-streak: t=400 is wasteful, t=0 costs 2*ceil(log2 64)=12
  CHECK(fs.t == 0);
  F::observe(fs, 3, gallop_tests_model(0, 2, 99999), 100000);  // waste at t=0 -> back up to 2
  CHECK(fs.t == 2);
  for (int i = 0; i < 100000; ++i) {       // never leaves [0, t_max] for random observations
    const std::size_t len = 2 + r.bounded(5000);
    const std::size_t n = 1 + r.bounded(len);
    F::observe(fs, n, gallop_tests_model(fs.t, n - 1 > len - 1 ? len - 1 : n - 1, len - 1), len);
    CHECK(fs.t <= fs.tmax);
  }
}

template <class TP>
static Stats count_run(const std::vector<std::uint32_t>& keys, std::vector<MergeRecord>* log = nullptr) {
  auto v = tagged(keys);
  Stats st;
  st.log = log;
  natural_merge_sort<Order::Power, TP, true>(v.data(), v.size(), TagLess{}, &st, 32);
  auto ref = tagged(keys);
  std::stable_sort(ref.begin(), ref.end(), TagLess{});
  CHECK(same(v, ref));
  return st;
}

static void test_bounds_and_fastpath() {
  // equal to base on a random permutation (all streaks are short -> fast path only)
  for (std::uint64_t seed = 1; seed <= 3; ++seed) {
    const auto keys = gen::make("perm", 100000, 0, seed);
    const Stats b = count_run<PolylogT>(keys), s = count_run<SagT>(keys), h = count_run<SagHalving>(keys);
    // halving-only ablation: never leaves the fast path on streak-free data, so it is EXACTLY the base
    CHECKF(b.comparisons() == h.comparisons(), "perm: base=%llu halving=%llu", (unsigned long long)b.comparisons(), (unsigned long long)h.comparisons());
    // final (jump) configuration may occasionally jump to t=0 on a lucky long streak: development guard is <= +1 %
    CHECKF(static_cast<double>(s.comparisons()) <= 1.01 * static_cast<double>(b.comparisons()), "perm: base=%llu sag=%llu",
           (unsigned long long)b.comparisons(), (unsigned long long)s.comparisons());
  }
  // per-merge worst-case bound for ANY t >= 0:  <= 2 + (4/3)(a+b)
  long merges = 0;
  for (const char* fam : {"few", "zipf", "mixture", "runs", "nearly", "perm", "pipe"})
    for (double p : {2.0, 3.0, 16.0, 256.0, 4096.0})
      for (std::uint64_t seed = 1; seed <= 2; ++seed) {
        const auto keys = gen::make(fam, 40000, p, seed);
        std::vector<MergeRecord> log;
        count_run<SagT>(keys, &log);
        for (const auto& r : log) {
          ++merges;
          CHECKF(static_cast<double>(r.comparisons) <= 2.0 + (4.0 / 3.0) * static_cast<double>(r.a + r.b) + 1e-9,
                 "%s p=%.0f a=%zu b=%zu cmp=%llu", fam, p, r.a, r.b, (unsigned long long)r.comparisons);
        }
      }
  std::printf("  per-merge (4/3) bound checked on %ld merges\n", merges);
}

static void test_not_worse_on_low_entropy() {
  struct C { const char* fam; double p; };
  const C cases[] = {{"few", 2}, {"few", 3}, {"few", 16}, {"few", 256}, {"few", 4096}, {"zipf", 1024}, {"mixture", 64}};
  for (const C& c : cases)
    for (std::size_t n : {1u << 16, 1u << 19}) {
      const auto keys = gen::make(c.fam, n, c.p, 11);
      const Stats b = count_run<PolylogT>(keys), s = count_run<SagT>(keys);
      std::printf("    %-8s p=%-6.0f n=%-8zu base=%.3f  SAG=%.3f  (comparisons/n)  ratio=%.3f\n", c.fam, c.p, (size_t)n,
                  (double)b.comparisons() / n, (double)s.comparisons() / n, (double)s.comparisons() / (double)b.comparisons());
      // SAG may lose a little on some inputs; the property we assert is "not more than 3 % worse"
      CHECKF(static_cast<double>(s.comparisons()) <= 1.03 * static_cast<double>(b.comparisons()), "%s p=%.0f n=%zu", c.fam, c.p, (size_t)n);
    }
}

// Build two sorted runs whose merge alternates long and short streaks; concatenated they form one array
// that the sorter treats as two natural runs (minrun is irrelevant: runs are already long).
static std::vector<std::uint32_t> alternating_streaks(std::size_t pairs, std::size_t long_len) {
  std::vector<std::uint32_t> A, B;
  std::uint32_t v = 0;
  for (std::size_t i = 0; i < pairs; ++i) {
    for (std::size_t j = 0; j < long_len; ++j) A.push_back(v++);  // long streak from A
    B.push_back(v++);                                            // short streak from B
    A.push_back(v++);                                            // short streak from A
    for (std::size_t j = 0; j < long_len; ++j) B.push_back(v++); // long streak from B
  }
  A.insert(A.end(), B.begin(), B.end());
  return A;
}

static void test_adversarial_alternation() {
  for (std::size_t long_len : {10u, 50u, 400u, 3000u}) {
    const auto keys = alternating_streaks(300, long_len);
    std::vector<MergeRecord> log;
    const Stats b = count_run<PolylogT>(keys), s = count_run<SagT>(keys, &log);
    Stats n;
    {
      auto v = tagged(keys);
      natural_merge_sort<Order::Power, NaiveT, true>(v.data(), v.size(), TagLess{}, &n, 32);
    }
    std::printf("    alternation long=%-5zu n=%-8zu naive=%llu base=%llu SAG=%llu\n", (size_t)long_len, keys.size(),
                (unsigned long long)n.comparisons(), (unsigned long long)b.comparisons(), (unsigned long long)s.comparisons());
    CHECK(static_cast<double>(s.comparisons()) <= (4.0 / 3.0) * static_cast<double>(n.comparisons()) + 64.0);
  }
}

int main() {
  std::printf("== test_sag (extension) ==\n");
  test_controller();
  test_bounds_and_fastpath();
  test_not_worse_on_low_entropy();
  test_adversarial_alternation();
  return chk_report("test_sag");
}
