// test_sagt.cpp -- tests for the corrective-round extension SAG-T (time-aware controller + self-calibration).
//   1. controller decisions under different (c, m) cost models;
//   2. calibration returns sane values;
//   3. sorting through sag_time_sort() (calibration path) is correct and stable for many sizes/families;
//   4. per-merge worst-case bound (4/3) holds for several fixed (c, m) settings (direct policy use).
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "check.hpp"
#include "generators.hpp"
#include "natural_merge_sort.hpp"
#include "sag_time_policy.hpp"

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
  using P = SagTimeTT<16>;
  // expensive comparisons: a 100-streak scanned linearly is far costlier than 14 non-linear probes -> jump to 0
  P::State s{400, 400, 22.0, 4.0};
  P::observe(s, 100, 100, 100000);
  CHECK(s.t == 0);
  // cheap comparisons (0.5 ns) with a 4 ns non-linear penalty: 100 elements scan = 50 ns, gallop = 14*4.5 = 63 ns -> keep t
  P::State cheap{400, 400, 0.5, 4.0};
  P::observe(cheap, 100, 100, 100000);
  CHECK(cheap.t == 400);
  // ... but a 2000-streak is worth galloping even then
  P::observe(cheap, 2000, 2000, 100000);
  CHECK(cheap.t == 0);
  // after the jump, short blocks make non-linear probes pure waste -> t recovers upward at once (cheap model)
  P::observe(cheap, 2, 2, 100000);
  CHECK(cheap.t > 0);
  // expensive model: a short block at t=0 wastes only m=4 of ~26 ns per probe, but recovery still prefers linear for k=1
  P::State e0{0, 400, 22.0, 4.0};
  P::observe(e0, 2, 2, 100000);
  CHECK(e0.t >= 2);
  // never leaves [0, t_max]
  gen::Rng r(9);
  P::State rs{16, 16, 3.0, 4.0};
  for (int i = 0; i < 200000; ++i) {
    const std::size_t len = 2 + r.bounded(4000);
    const std::size_t n = 1 + r.bounded(len);
    const std::size_t k = n - 1, tail = len - 1;
    const u64 tests = gallop_tests_model(rs.t, k > tail ? tail : k, tail);
    P::observe(rs, n, tests, len);
    CHECK(rs.t <= rs.tmax);
  }
  // probe split adds up
  for (std::size_t t : {0u, 1u, 5u, 40u})
    for (std::size_t len : {1u, 10u, 100u}) {
      for (std::size_t k = 0; k <= len; ++k) {
        const ProbeSplit p = gallop_probe_split(t, k, len);
        CHECK(p.linear + p.nonlinear == gallop_tests_model(t, k, len));
        CHECK(p.linear <= t);
      }
    }
}

static void test_calibration() {
  const auto keys = gen::make("perm", 100000, 0, 3);
  const double plain = measure_comparison_ns(keys.data(), keys.size(), [](std::uint32_t a, std::uint32_t b) { return a < b; });
  CHECKF(plain >= 0.2 && plain < 100.0, "plain comparison measured %.3f ns", plain);
  const double heavy = measure_comparison_ns(keys.data(), keys.size(), [](std::uint32_t a, std::uint32_t b) {
    std::uint64_t x = static_cast<std::uint64_t>(a) * 0x9E3779B97F4A7C15ull + b;
    for (int i = 0; i < 40; ++i) x = x * 6364136223846793005ull + 1442695040888963407ull;
    asm volatile("" : : "r"(x));
    return a < b;
  });
  CHECKF(heavy > plain, "heavy %.3f ns should exceed plain %.3f ns", heavy, plain);
  std::printf("  calibration: plain comparison %.2f ns, heavy comparison %.2f ns\n", plain, heavy);
}

static void test_sorting() {
  long arrays = 0;
  struct F { const char* n; double p; };
  const F fams[] = {{"perm", 0}, {"few", 2}, {"few", 16}, {"few", 1000}, {"zipf", 256}, {"runs", 5}, {"mixture", 8}, {"nearly", 0.01}, {"sorted", 0}, {"reversed", 0}};
  for (const F& f : fams)
    for (std::size_t n : {0u, 1u, 100u, 4095u, 4096u, 4097u, 5000u, 20000u, 100000u})
      for (std::uint64_t seed = 1; seed <= 2; ++seed) {
        const auto keys = gen::make(f.n, n, f.p, seed * 31 + n);
        auto v = tagged(keys), ref = tagged(keys);
        std::stable_sort(ref.begin(), ref.end(), TagLess{});
        sag_time_sort<16>(v.data(), v.size(), TagLess{}, nullptr, 32, 4.0);
        CHECKF(same(v, ref), "sag_time_sort %s n=%zu seed=%llu", f.n, (size_t)n, (unsigned long long)seed);
        auto v2 = tagged(keys);
        sag_time_sort<64>(v2.data(), v2.size(), TagLess{}, nullptr, 32, 16.0);
        CHECK(same(v2, ref));
        ++arrays;
      }
  std::printf("  sag_time_sort: %ld arrays x 2 configs\n", arrays);
}

template <unsigned MS>
static void check_bound(double c, double m, const char* label) {
  long merges = 0;
  for (const char* fam : {"few", "zipf", "mixture", "runs", "nearly", "perm"})
    for (double p : {2.0, 16.0, 256.0, 4096.0}) {
      const auto keys = gen::make(fam, 40000, p, 5);
      auto v = tagged(keys);
      auto ref = v;
      std::stable_sort(ref.begin(), ref.end(), TagLess{});
      std::vector<MergeRecord> log;
      Stats st;
      st.log = &log;
      sag_time_params() = {c, m};
      natural_merge_sort<Order::Power, SagTimeTT<MS>, true>(v.data(), v.size(), TagLess{}, &st, 32);
      CHECK(same(v, ref));
      for (const auto& r : log) {
        ++merges;
        CHECKF(static_cast<double>(r.comparisons) <= 2.0 + (4.0 / 3.0) * static_cast<double>(r.a + r.b) + 1e-9, "%s %s p=%.0f", label, fam, p);
      }
    }
  std::printf("  per-merge (4/3) bound, %s: %ld merges\n", label, merges);
}

static void test_credit_and_breakeven() {
  // break-even block length: shrinks when comparisons get dearer, grows with the non-linear penalty
  CHECK(breakeven_block_length(22.0, 16.0) < breakeven_block_length(1.0, 16.0));
  CHECK(breakeven_block_length(1.0, 4.0) < breakeven_block_length(1.0, 16.0));
  CHECK(breakeven_block_length(100.0, 1.0) >= 2);
  for (double c : {0.3, 1.0, 5.0, 22.0})
    for (double m : {2.0, 8.0, 16.0}) {
      const size_t B = breakeven_block_length(c, m);
      CHECKF(B >= 2, "B=%zu", B);
      // at B the pure exponential search is cheaper than the linear scan
      const double g = (c + m) * static_cast<double>(gallop_tests_model(0, B - 1, size_t(1) << 40));
      CHECK(g < c * static_cast<double>(B));
    }
  // credit mechanics
  using C = SagCreditT;
  const double c = 16.0, m = 16.0;
  C::State s{400, 400, breakeven_block_length(c, m), c, m, 0.0, 50 * m};
  C::observe(s, 1, 1, 1000);                      // short block, no credit: stays safe
  CHECK(s.t == 400 && s.credit == 0.0);
  C::observe(s, 300, 300, 100000);                // long block earns credit -> t = 0
  CHECK(s.t == 0 && s.credit > 0.0);
  const double earned = s.credit;
  CHECK(earned <= s.cap);
  C::observe(s, 1, 1, 100000);                    // a one-element block spends about m
  CHECK(s.t == 0 && s.credit < earned);
  for (int i = 0; i < 100000 && s.t == 0; ++i) C::observe(s, 1, 1, 100000);
  CHECK(s.t == 400 && s.credit == 0.0);           // drained: back to the safe value
  gen::Rng r(4);
  for (int i = 0; i < 200000; ++i) {              // invariants under random blocks
    const std::size_t n = 1 + r.bounded(2000);
    C::observe(s, n, n, 100000);
    CHECK(s.credit >= 0.0 && s.credit <= s.cap);
    CHECK(s.t == 0 || s.t == s.tmax);
  }
}

static void test_credit_sorting() {
  long arrays = 0;
  struct F { const char* n; double p; };
  const F fams[] = {{"perm", 0}, {"few", 2}, {"few", 16}, {"few", 1000}, {"zipf", 256}, {"runs", 5}, {"mixture", 8}, {"nearly", 0.01}, {"sorted", 0}, {"reversed", 0}};
  for (const F& f : fams)
    for (std::size_t n : {0u, 1u, 100u, 4095u, 4096u, 4097u, 5000u, 20000u, 100000u})
      for (std::uint64_t seed = 1; seed <= 2; ++seed) {
        const auto keys = gen::make(f.n, n, f.p, seed * 17 + n);
        auto ref = tagged(keys);
        std::stable_sort(ref.begin(), ref.end(), TagLess{});
        auto v = tagged(keys);
        sag_credit_sort(v.data(), v.size(), TagLess{}, nullptr, 32, 16.0, 50.0);
        CHECKF(same(v, ref), "sag_credit_sort %s n=%zu", f.n, (size_t)n);
        auto w = tagged(keys);
        sag_be_sort(w.data(), w.size(), TagLess{}, nullptr, 32, 8.0);
        CHECKF(same(w, ref), "sag_be_sort %s n=%zu", f.n, (size_t)n);
        ++arrays;
      }
  std::printf("  sag_credit_sort / sag_be_sort: %ld arrays each\n", arrays);
  // per-merge (4/3) bound for the credit policy under fixed (c, m, cap) settings
  long merges = 0;
  for (double cc : {0.5, 5.0, 22.0})
    for (const char* fam : {"few", "zipf", "mixture", "runs", "nearly", "perm"})
      for (double p : {2.0, 16.0, 256.0, 4096.0}) {
        const auto keys = gen::make(fam, 40000, p, 6);
        auto v = tagged(keys), ref = v;
        std::stable_sort(ref.begin(), ref.end(), TagLess{});
        std::vector<MergeRecord> log;
        Stats st;
        st.log = &log;
        sag_time_params() = {cc, 16.0, breakeven_block_length(cc, 16.0), 50 * 16.0};
        natural_merge_sort<Order::Power, SagCreditT, true>(v.data(), v.size(), TagLess{}, &st, 32);
        CHECK(same(v, ref));
        for (const auto& rec : log) {
          ++merges;
          CHECKF(static_cast<double>(rec.comparisons) <= 2.0 + (4.0 / 3.0) * static_cast<double>(rec.a + rec.b) + 1e-9, "credit bound %s c=%.1f", fam, cc);
        }
      }
  std::printf("  per-merge (4/3) bound, SAG-C: %ld merges\n", merges);
}

int main() {
  std::printf("== test_sagt (time-aware extensions SAG-T v1, SAG-B, SAG-C) ==\n");
  test_credit_and_breakeven();
  test_credit_sorting();
  test_controller();
  test_calibration();
  test_sorting();
  check_bound<16>(22.0, 4.0, "c=22 m=4 MinStreak 16");
  check_bound<16>(0.5, 4.0, "c=0.5 m=4 MinStreak 16");
  check_bound<64>(3.0, 16.0, "c=3 m=16 MinStreak 64");
  return chk_report("test_sagt");
}
