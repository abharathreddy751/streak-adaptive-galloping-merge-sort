// test_sort.cpp -- correctness + paper-conformance tests for the BASE natural merge sort.
//   build (base only):        scripts/build.sh            -> build/test_base.exe
//   build (with extension):   scripts/build.sh            -> build/test_all.exe  (-DWITH_SAG)
//
// Oracles: std::stable_sort on (key, original-index) tags (so stability is checked exactly), the textbook
// top-down Merge Sort, and closed-form facts taken from the paper (Table 1, cost_t, Lemma 1, Prop. 49, Thm 51).
// Everything is deterministic (fixed seeds).  No test is sampled silently: counts are printed.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#include "check.hpp"
#include "entropy.hpp"
#include "generators.hpp"
#include "natural_merge_sort.hpp"
#include "textbook_merge_sort.hpp"
#ifdef WITH_SAG
#include "sag_policy.hpp"
#endif

using namespace nms;

struct Tag { std::uint32_t key, id; };
static_assert(std::is_trivially_copyable_v<Tag>);
struct TagLess { bool operator()(const Tag& a, const Tag& b) const { return a.key < b.key; } };

static std::vector<Tag> tagged(const std::vector<std::uint32_t>& keys) {
  std::vector<Tag> v(keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) v[i] = {keys[i], static_cast<std::uint32_t>(i)};
  return v;
}
static bool same(const std::vector<Tag>& a, const std::vector<Tag>& b) {
  return a.size() == b.size() && (a.empty() || std::memcmp(a.data(), b.data(), a.size() * sizeof(Tag)) == 0);
}
static std::vector<Tag> reference(std::vector<Tag> v) {
  std::stable_sort(v.begin(), v.end(), TagLess{});
  return v;
}

// ---- configuration list ----------------------------------------------------------------------
#ifdef WITH_SAG
using SagJ2 = SagTT<2, true>;  // tiny MinStreak: exercises the adaptation path on small inputs
#define EXTRA_CFG(X) X(Order::Power, SagT, "power/SAG") X(Order::Tim, SagT, "tim/SAG") X(Order::Power, SagHalving, "power/SAGh") X(Order::Power, SagJ2, "power/SAGj2")
#else
#define EXTRA_CFG(X)
#endif
#define ALL_CFG(X)                                                                                      \
  X(Order::Power, NaiveT, "power/naive") X(Order::Power, FixedT<0>, "power/t0")                         \
  X(Order::Power, FixedT<3>, "power/t3") X(Order::Power, PolylogT, "power/polylog")                     \
  X(Order::Tim, NaiveT, "tim/naive") X(Order::Tim, FixedT<0>, "tim/t0")                                 \
  X(Order::Tim, FixedT<3>, "tim/t3") X(Order::Tim, PolylogT, "tim/polylog") EXTRA_CFG(X)

template <class F>
static void for_cfg(F&& f) {
#define X(O, TP, NM) f(std::integral_constant<Order, O>{}, TP{}, NM);
  ALL_CFG(X)
#undef X
}
static int num_cfg() {
  int c = 0;
  for_cfg([&](auto, auto, const char*) { ++c; });
  return c;
}

// Sort `keys` with one configuration and compare against the stable reference.
template <Order O, class TP>
static bool sort_matches(const std::vector<std::uint32_t>& keys, std::size_t minrun) {
  std::vector<Tag> v = tagged(keys);
  const std::vector<Tag> ref = reference(v);
  natural_merge_sort<O, TP, false>(v.data(), v.size(), TagLess{}, nullptr, minrun);
  return same(v, ref);
}

// ---- 1. galloping routine vs paper Table 1 and cost model ------------------------------------
static void test_gallop() {
  const unsigned B1[18] = {0, 1, 2, 4, 4, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 10};  // paper Table 1, index = m
  for (unsigned m = 1; m <= 17; ++m) {
    std::vector<int> arr(200, 1);
    for (unsigned i = 0; i + 1 < m; ++i) arr[i] = 0;  // m-1 false elements, then true
    u64 tests = 0;
    const size_t r = gallop_count<+1>(arr.data(), arr.size(), 0, [](int e) { return e == 1; }, tests);
    CHECKF(r == m - 1, "m=%u r=%zu", m, r);
    CHECKF(tests == B1[m], "Table1 m=%u tests=%llu expected %u", m, (unsigned long long)tests, B1[m]);
  }
  // result correctness + agreement with the arithmetic model, all small (t, len, k)
  const size_t ts[] = {0, 1, 2, 3, 5, 10, 31};
  for (size_t t : ts)
    for (size_t len = 0; len <= 70; ++len)
      for (size_t k = 0; k <= len; ++k) {
        std::vector<int> arr(len + 1, 1);
        for (size_t i = 0; i < k; ++i) arr[i] = 0;
        u64 tests = 0;
        const size_t r = gallop_count<+1>(arr.data(), len, t, [](int e) { return e == 1; }, tests);
        CHECKF(r == k, "t=%zu len=%zu k=%zu r=%zu", t, len, k, r);
        CHECKF(tests == gallop_tests_model(t, k, len), "model t=%zu len=%zu k=%zu", t, len, k);
        CHECKF(tests <= len, "tests>len t=%zu len=%zu k=%zu", t, len, k);
      }
  // mixed strategy cost: cost_t(m) = m if m <= t+2, else t + 2*ceil(log2(m-t))     (paper Sec. 2)
  for (size_t t : ts)
    for (size_t m = 1; m <= 300; ++m) {
      std::vector<int> arr(6000, 1);
      for (size_t i = 0; i + 1 < m; ++i) arr[i] = 0;
      u64 tests = 0;
      gallop_count<+1>(arr.data(), arr.size(), t, [](int e) { return e == 1; }, tests);
      const u64 expect = m <= t + 2 ? m : t + 2ull * ceil_log2(m - t);
      CHECKF(tests == expect, "cost_t t=%zu m=%zu tests=%llu expect=%llu", t, m, (unsigned long long)tests, (unsigned long long)expect);
    }
  // backward direction (D = -1) finds the same counts
  for (size_t len = 0; len <= 40; ++len)
    for (size_t k = 0; k <= len; ++k) {
      std::vector<int> arr(len + 1, 1);
      for (size_t i = 0; i < k; ++i) arr[len - 1 - i] = 0;  // logical element x is arr[len-1-x]
      u64 tests = 0;
      const size_t r = gallop_count<-1>(arr.data() + (len ? len - 1 : 0), len, 2, [](int e) { return e == 1; }, tests);
      CHECKF(r == k, "backward len=%zu k=%zu r=%zu", len, k, r);
    }
}

static void test_node_power() {
  CHECK(node_power(0, 50, 50, 100) == 1);
  CHECK(node_power(0, 25, 25, 100) == 2);
  CHECK(node_power(0, 1, 1, 1ull << 40) >= 1);
  for (std::size_t n = 2; n <= 300; ++n)
    for (std::size_t n1 = 1; n1 < n; ++n1) {
      const std::size_t n2 = std::min<std::size_t>(n - n1, 7);
      const unsigned p = node_power(0, n1, n2, n);
      CHECKF(p >= 1 && p <= 2 * ceil_log2(n) + 2, "node_power n=%zu n1=%zu n2=%zu p=%u", n, n1, n2, p);
    }
}

// ---- 2. exhaustive small inputs ---------------------------------------------------------------
static void test_two_run_pairs() {
  std::vector<std::vector<std::uint32_t>> seqs;  // all non-decreasing sequences over {0..3}, length 0..6
  for (int len = 0; len <= 6; ++len) {
    std::vector<std::uint32_t> s(len, 0);
    for (;;) {
      seqs.push_back(s);
      int i = len - 1;
      while (i >= 0 && s[i] == 3) --i;
      if (i < 0) break;
      ++s[i];
      for (int j = i + 1; j < len; ++j) s[j] = s[i];
    }
  }
  long pairs = 0;
  for (const auto& a : seqs)
    for (const auto& b : seqs) {
      std::vector<std::uint32_t> keys = a;
      keys.insert(keys.end(), b.begin(), b.end());
      ++pairs;
      for_cfg([&](auto ord, auto tp, const char* nm) {
        constexpr Order O = decltype(ord)::value;
        using TP = decltype(tp);
        CHECKF((sort_matches<O, TP>(keys, 1)), "two-run %s", nm);
      });
    }
  std::printf("  two-run pairs: %ld pairs x %d configs\n", pairs, num_cfg());
}

static void test_exhaustive_alphabet3() {
  long arrays = 0;
  for (int len = 0; len <= 9; ++len) {
    long total = 1;
    for (int i = 0; i < len; ++i) total *= 3;
    for (long code = 0; code < total; ++code) {
      std::vector<std::uint32_t> keys(len);
      long c = code;
      for (int i = 0; i < len; ++i) { keys[i] = static_cast<std::uint32_t>(c % 3); c /= 3; }
      ++arrays;
      for (std::size_t minrun : {1u, 2u, 4u})
        for_cfg([&](auto ord, auto tp, const char* nm) {
          constexpr Order O = decltype(ord)::value;
          using TP = decltype(tp);
          CHECKF((sort_matches<O, TP>(keys, minrun)), "alphabet3 %s minrun=%zu code=%ld len=%d", nm, (size_t)minrun, code, len);
        });
    }
  }
  std::printf("  alphabet-3 arrays (len<=9): %ld arrays x 3 minrun x %d configs\n", arrays, num_cfg());
}

static void test_all_permutations() {
  long perms = 0;
  for (int len = 0; len <= 7; ++len) {
    std::vector<std::uint32_t> p(len);
    std::iota(p.begin(), p.end(), 0u);
    do {
      ++perms;
      for (std::size_t minrun : {1u, 2u})
        for_cfg([&](auto ord, auto tp, const char* nm) {
          constexpr Order O = decltype(ord)::value;
          using TP = decltype(tp);
          CHECKF((sort_matches<O, TP>(p, minrun)), "perm %s minrun=%zu len=%d", nm, (size_t)minrun, len);
        });
    } while (std::next_permutation(p.begin(), p.end()));
  }
  std::printf("  permutations (len<=7): %ld perms x 2 minrun x %d configs\n", perms, num_cfg());
}

// ---- 3. randomized family sweep ----------------------------------------------------------------
static void test_random_families() {
  struct Fam { const char* name; double p; };
  const Fam fams[] = {{"perm", 0}, {"few", 2}, {"few", 3}, {"few", 16}, {"few", 1000}, {"zipf", 256}, {"runs", 1},
                      {"runs", 2}, {"runs", 5}, {"runs", 50}, {"mixture", 8}, {"nearly", 0.01}, {"sorted", 0},
                      {"reversed", 0}, {"pipe", 0}};
  std::vector<std::size_t> sizes;
  for (std::size_t n = 0; n <= 40; ++n) sizes.push_back(n);
  for (std::size_t n : {63, 64, 65, 100, 127, 128, 129, 255, 256, 257, 1000, 4096, 10007, 100000}) sizes.push_back(n);
  long arrays = 0;
  for (const Fam& f : fams)
    for (std::size_t n : sizes)
      for (std::uint64_t seed = 1; seed <= (n >= 100000 ? 1u : 3u); ++seed) {
        const std::vector<std::uint32_t> keys = gen::make(f.name, n, f.p, seed * 7919 + n);
        ++arrays;
        for (std::size_t minrun : {2u, 32u})
          for_cfg([&](auto ord, auto tp, const char* nm) {
            constexpr Order O = decltype(ord)::value;
            using TP = decltype(tp);
            CHECKF((sort_matches<O, TP>(keys, minrun)), "random %s %s n=%zu seed=%llu minrun=%zu", f.name, nm, n, (unsigned long long)seed, (size_t)minrun);
          });
        // independent oracle #2: the textbook top-down Merge Sort
        std::vector<Tag> t = tagged(keys);
        merge_sort_topdown(t.data(), t.size(), TagLess{});
        CHECKF(same(t, reference(tagged(keys))), "textbook %s n=%zu", f.name, n);
      }
  std::printf("  random arrays: %ld arrays x 2 minrun x %d configs (+ textbook oracle)\n", arrays, num_cfg());
}

// ---- 4. edge cases ------------------------------------------------------------------------------
static void test_edges() {
  natural_merge_sort<Order::Power, PolylogT>(static_cast<Tag*>(nullptr), 0, TagLess{});  // n = 0, nullptr: must not crash
  CHECK(true);
  for (std::size_t n : {1u, 2u, 3u}) {  // equal keys keep order
    std::vector<std::uint32_t> keys(n, 7);
    CHECK((sort_matches<Order::Power, PolylogT>(keys, 32)));
    CHECK((sort_matches<Order::Tim, PolylogT>(keys, 32)));
  }
  CHECK((sort_matches<Order::Power, PolylogT>(gen::make("few", 5000, 1, 3), 32)));     // all equal
  CHECK((sort_matches<Order::Power, PolylogT>(gen::make("few", 5000, 2, 3), 0)));      // minrun = 0 is clamped to 1
  CHECK((sort_matches<Order::Power, PolylogT>(gen::make("perm", 50, 0, 3), 1000000))); // minrun larger than n: pure insertion
  std::vector<std::uint32_t> ext = {0xFFFFFFFFu, 0, 0xFFFFFFFFu, 1, 0x80000000u, 0x7FFFFFFFu};  // extreme key values
  CHECK((sort_matches<Order::Power, PolylogT>(ext, 1)));
  // t-policy extremes cannot overflow
  CHECK(PolylogT::start(1).t == 0);
  CHECK(PolylogT::start(2).t == 1);
  CHECK(PolylogT::start(std::size_t(1) << 40).t == 40u * 40u);
  CHECK(ceil_log2(0) == 0 && ceil_log2(1) == 0 && ceil_log2(2) == 1 && ceil_log2(3) == 2 && ceil_log2(1025) == 11);
}

// ---- 5. paper-derived invariants (counting build) -------------------------------------------------
template <class TP>
static void check_merge_bound(const std::vector<std::uint32_t>& keys, const char* label) {
  std::vector<Tag> v = tagged(keys);
  std::vector<MergeRecord> log;
  Stats st;
  st.log = &log;
  natural_merge_sort<Order::Power, TP, true>(v.data(), v.size(), TagLess{}, &st, 2);
  CHECK(same(v, reference(tagged(keys))));
  for (const MergeRecord& r : log) {
    // 1 (already-ordered check) + 1 (first-block owner) + sum of block costs <= (1+1/(t+3))(a+b)   [paper Lemma 1 / Prop 2]
    const double bound = 2.0 + (1.0 + 1.0 / (static_cast<double>(r.t_start) + 3.0)) * static_cast<double>(r.a + r.b) + 1e-9;
    CHECKF(static_cast<double>(r.comparisons) <= bound, "%s a=%zu b=%zu t=%zu cmp=%llu bound=%.1f", label, r.a, r.b, r.t_start,
           (unsigned long long)r.comparisons, bound);
  }
}

static void test_invariants() {
  // (a) per-merge cost bound
  for (std::uint64_t seed = 1; seed <= 4; ++seed) {
    check_merge_bound<PolylogT>(gen::make("few", 20000, 2, seed), "polylog few2");
    check_merge_bound<PolylogT>(gen::make("few", 20000, 3, seed), "polylog few3");
    check_merge_bound<PolylogT>(gen::make("few", 20000, 64, seed), "polylog few64");
    check_merge_bound<PolylogT>(gen::make("perm", 20000, 0, seed), "polylog perm");
    check_merge_bound<PolylogT>(gen::make("mixture", 20000, 8, seed), "polylog mixture");
    check_merge_bound<FixedT<0>>(gen::make("few", 20000, 16, seed), "t0 few16");
    check_merge_bound<FixedT<3>>(gen::make("zipf", 20000, 256, seed), "t3 zipf");
  }
  // (b) Prop. 49: polylog galloping never costs more than naive merging by more than O(n)
  double worst = -1e9;
  for (const char* fam : {"perm", "few", "zipf", "runs", "mixture", "nearly", "pipe"})
    for (double p : {2.0, 3.0, 64.0, 5000.0}) {
      const std::size_t n = 30000;
      const auto keys = gen::make(fam, n, p, 99);
      Stats su, sv;
      auto a = tagged(keys), b = tagged(keys);
      natural_merge_sort<Order::Power, NaiveT, true>(a.data(), n, TagLess{}, &su, 2);
      natural_merge_sort<Order::Power, PolylogT, true>(b.data(), n, TagLess{}, &sv, 2);
      CHECK(same(a, b));
      const double over = (static_cast<double>(sv.comparisons()) - static_cast<double>(su.comparisons())) / static_cast<double>(n);
      worst = std::max(worst, over);
      CHECKF(over <= 3.0, "polylog overhead %s p=%.0f (V-U)/n=%.3f", fam, p, over);
    }
  std::printf("  max (V_polylog - U_naive)/n over the sweep = %.3f   (Prop. 49 promises O(n); sanity bound 3)\n", worst);
  // (c) Theorem 51 shape: V <= n*H* + 2*n*log2(H*+1) + C*n.  C is unknown in the paper; report the observed value.
  double cmax = -1e9;
  for (unsigned sigma : {2u, 3u, 8u, 64u, 1024u}) {
    for (std::size_t n : {1u << 16, 1u << 19}) {
      const auto keys = gen::make("few", n, sigma, 5);
      const double hs = dual_run_entropy(keys);
      auto v = tagged(keys);
      Stats st;
      natural_merge_sort<Order::Power, PolylogT, true>(v.data(), n, TagLess{}, &st, 32);
      CHECK(same(v, reference(tagged(keys))));
      const double c = (static_cast<double>(st.comparisons()) - n * hs - 2.0 * n * std::log2(hs + 1.0)) / static_cast<double>(n);
      cmax = std::max(cmax, c);
      std::printf("    sigma=%-5u n=%-8zu H*=%.3f  cmp/n=%.3f  (n H*+2n log2(H*+1))/n=%.3f  C_obs=%.3f\n", sigma, (size_t)n, hs,
                  static_cast<double>(st.comparisons()) / n, hs + 2.0 * std::log2(hs + 1.0), c);
      CHECKF(c <= 10.0, "Thm51 sanity sigma=%u n=%zu C_obs=%.3f", sigma, (size_t)n, c);
    }
  }
  std::printf("  max C_obs (Theorem 51 constant, empirical) = %.3f\n", cmax);
  // (d) information-theoretic lower bound: any comparison sort needs >= log2(n!) on some inputs; our counter must be real
  for (std::uint64_t seed = 1; seed <= 5; ++seed) {
    const std::size_t n = 1000;
    auto v = tagged(gen::make("perm", n, 0, seed));
    Stats st;
    natural_merge_sort<Order::Power, PolylogT, true>(v.data(), n, TagLess{}, &st, 32);
    const double lb = std::lgamma(static_cast<double>(n) + 1.0) / std::log(2.0);
    CHECKF(static_cast<double>(st.comparisons()) >= lb * 0.9, "random perm cmp=%llu lb=%.0f", (unsigned long long)st.comparisons(), lb);
  }
  // (e) sorted / reversed inputs: exactly n-1 comparisons (single run, no merges), buffer never above n/2
  for (std::size_t n : {2u, 10u, 1000u, 100000u}) {
    for (const char* fam : {"sorted", "reversed"}) {
      auto v = tagged(gen::make(fam, n, 0, 1));
      Stats st;
      natural_merge_sort<Order::Power, PolylogT, true>(v.data(), n, TagLess{}, &st, 32);
      CHECKF(st.comparisons() == n - 1, "%s n=%zu cmp=%llu", fam, (size_t)n, (unsigned long long)st.comparisons());
    }
  }
  for (std::size_t n : {1000u, 65536u, 100001u}) {
    auto v = tagged(gen::make("perm", n, 0, 3));
    Stats st;
    natural_merge_sort<Order::Power, PolylogT, true>(v.data(), n, TagLess{}, &st, 32);
    CHECKF(st.max_buffer <= n / 2, "buffer %zu > n/2", st.max_buffer);
    CHECK(st.merges >= 1);
  }
  // (f) every config is a pure function of its input (determinism)
  {
    const auto keys = gen::make("few", 10000, 17, 8);
    Stats s1, s2;
    auto a = tagged(keys), b = tagged(keys);
    natural_merge_sort<Order::Power, PolylogT, true>(a.data(), a.size(), TagLess{}, &s1, 32);
    natural_merge_sort<Order::Power, PolylogT, true>(b.data(), b.size(), TagLess{}, &s2, 32);
    CHECK(s1.comparisons() == s2.comparisons() && same(a, b));
  }
}

int main() {
  std::printf("== test_sort (%s) ==\n",
#ifdef WITH_SAG
              "base + extension"
#else
              "BASE only"
#endif
  );
  std::printf("configs under test: %d\n", num_cfg());
  test_gallop();
  test_node_power();
  test_two_run_pairs();
  test_exhaustive_alphabet3();
  test_all_permutations();
  test_random_families();
  test_edges();
  test_invariants();
  return chk_report("test_sort");
}
