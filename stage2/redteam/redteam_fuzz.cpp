// redteam_fuzz.cpp -- STAGE-2 independent red-team of src/ (does NOT use tests/check.hpp, src/generators.hpp or the old test code).
//
//  * fresh deterministic seeds (below), own PRNG and own input families;
//  * records {int64 key, uint32 original index}: sorted with a comparator on the key only, so equal keys are distinguishable and
//    STABILITY is checked directly (equivalent neighbours must keep ascending original index);
//  * the output is checked by invariants (non-decreasing, stable, permutation, payload intact) AND compared with std::stable_sort;
//  * canary (guard) records around every array detect out-of-bounds writes without needing AddressSanitizer;
//  * every merge-order policy x t-policy x minrun, including the time-aware SAG-B / SAG-C entry points;
//  * abuse tests: inconsistent (random) comparator, throwing comparator (exception safety is NOT promised: reported, not failed);
//  * gallop_count vs gallop_tests_model vs the (4/3) per-block bound, checked exhaustively on a finite domain.
//
// Build:  g++ -std=c++17 -O2 -Wall -Wextra -Isrc stage2/redteam/redteam_fuzz.cpp -o stage2/redteam/redteam_fuzz
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <string>
#include <vector>

#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
#include "sag_time_policy.hpp"

using nms::Order;
struct Rec { std::int64_t key; std::uint32_t idx; std::uint32_t tag; };
static_assert(std::is_trivially_copyable_v<Rec>);

static std::uint64_t g_checks = 0, g_fail = 0, g_sorts = 0;
static void fail(const std::string& m) { ++g_fail; if (g_fail <= 25) std::printf("FAIL: %s\n", m.c_str()); }
#define CHECK(c, msg) do { ++g_checks; if (!(c)) fail(msg); } while (0)

// ---- own PRNG (splitmix64 seeding xoshiro-lite)
struct Rng {
  std::uint64_t s;
  explicit Rng(std::uint64_t seed) : s(seed * 0x9E3779B97F4A7C15ull + 0x1234567ull) {}
  std::uint64_t next() { s += 0x9E3779B97F4A7C15ull; std::uint64_t z = s; z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull; z = (z ^ (z >> 27)) * 0x94D049BB133111EBull; return z ^ (z >> 31); }
  std::uint64_t below(std::uint64_t n) { return n ? next() % n : 0; }
};

// ---- comparators (all strict weak orders on key only)
struct Asc { bool operator()(const Rec& a, const Rec& b) const { return a.key < b.key; } };
struct Desc { bool operator()(const Rec& a, const Rec& b) const { return a.key > b.key; } };
struct Mod7 { bool operator()(const Rec& a, const Rec& b) const { return ((a.key % 7) + 7) % 7 < ((b.key % 7) + 7) % 7; } };  // many equivalent, distinct keys

// ---- input families (key sequences); idx is assigned afterwards as the original position
static const char* kFam[] = {"alpha1", "alpha2", "alpha3", "alpha17", "alpha1000", "fullrange", "extremes", "sorted", "reversed", "sawtooth",
                             "organpipe", "randruns", "nearly", "streak_geo", "streak_1", "streak_big", "streak_alt", "desc_runs", "dups_sorted", "two_blocks", "streak_trap"};
static const int kNF = sizeof(kFam) / sizeof(kFam[0]);

static std::vector<std::int64_t> make_keys(int fam, size_t n, Rng& r) {
  std::vector<std::int64_t> k(n);
  auto fill_alpha = [&](std::uint64_t a) { for (auto& x : k) x = static_cast<std::int64_t>(r.below(a)); };
  const std::int64_t lo = std::numeric_limits<std::int64_t>::min(), hi = std::numeric_limits<std::int64_t>::max();
  switch (fam) {
    case 0: fill_alpha(1); break;
    case 1: fill_alpha(2); break;
    case 2: fill_alpha(3); break;
    case 3: fill_alpha(17); break;
    case 4: fill_alpha(1000); break;
    case 5: for (auto& x : k) x = static_cast<std::int64_t>(r.next()); break;
    case 6: for (auto& x : k) { auto v = r.below(6); x = v == 0 ? lo : v == 1 ? hi : v == 2 ? lo + 1 : v == 3 ? hi - 1 : v == 4 ? 0 : -1; } break;
    case 7: for (size_t i = 0; i < n; ++i) k[i] = static_cast<std::int64_t>(i); break;
    case 8: for (size_t i = 0; i < n; ++i) k[i] = static_cast<std::int64_t>(n - i); break;
    case 9: { size_t p = 1 + r.below(50); for (size_t i = 0; i < n; ++i) k[i] = static_cast<std::int64_t>(i % p); } break;
    case 10: for (size_t i = 0; i < n; ++i) k[i] = static_cast<std::int64_t>(i < n / 2 ? i : n - i); break;
    case 11: { size_t i = 0; while (i < n) { size_t len = 1 + r.below(200); std::int64_t v = static_cast<std::int64_t>(r.below(1000)); for (size_t j = 0; j < len && i < n; ++j, ++i) k[i] = v + static_cast<std::int64_t>(j / (1 + r.below(3))); } } break;
    case 12: { for (size_t i = 0; i < n; ++i) k[i] = static_cast<std::int64_t>(i); size_t sw = n / 100 + 1; for (size_t s = 0; s < sw && n > 1; ++s) std::swap(k[r.below(n)], k[r.below(n)]); } break;
    // streak families: two sorted interleaved sequences => a merge sees blocks of the prescribed length.  Realised as a sequence
    // that is one run of evens followed by one run of odds (two natural runs) with the interleave pattern controlled by the block lengths.
    case 13: case 14: case 15: case 16: {
      std::vector<std::int64_t> A, B;
      std::int64_t v = 0;
      bool left = true;
      size_t i = 0;
      while (i < n) {
        size_t len = fam == 13 ? 1 + static_cast<size_t>(-std::log(1.0 - (r.below(1u << 30) + 0.5) / (double)(1u << 30)) * 6.0)
                   : fam == 14 ? 1
                   : fam == 15 ? (r.below(8) == 0 ? 1 + r.below(n / 2 + 1) : 1 + r.below(3))
                   : (left ? 1 + r.below(2) : 40 + r.below(80));
        for (size_t j = 0; j < len && i < n; ++j, ++i, ++v) (left ? A : B).push_back(v);
        left = !left;
      }
      size_t p = 0;
      for (auto x : A) k[p++] = x;
      for (auto x : B) k[p++] = x;
    } break;
    case 17: { size_t i = 0; while (i < n) { size_t len = 2 + r.below(60); std::int64_t v = static_cast<std::int64_t>(r.below(100000)); for (size_t j = 0; j < len && i < n; ++j, ++i) k[i] = v - static_cast<std::int64_t>(j); } } break;
    case 18: { for (size_t i = 0; i < n; ++i) k[i] = static_cast<std::int64_t>(i / 7); } break;
    case 19: { for (size_t i = 0; i < n; ++i) k[i] = static_cast<std::int64_t>(i < n / 2 ? 2 * i : 2 * (i - n / 2) + 1); } break;  // perfectly interleaved halves
    // adversarial for SAG: one long streak (sends t to 0) followed by blocks of length 3 (cost 4 = 4/3 of the block at t = 0)
    case 20: {
      std::vector<std::int64_t> A, B;
      std::int64_t v = 0;
      bool left = true;
      size_t i = 0;
      size_t len = 120;
      while (i < n) { for (size_t j = 0; j < len && i < n; ++j, ++i, ++v) (left ? A : B).push_back(v); left = !left; len = 3; }
      size_t p = 0;
      for (auto x : A) k[p++] = x;
      for (auto x : B) k[p++] = x;
    } break;
  }
  return k;
}

// ---- configuration enumeration -----------------------------------------------------------------
template <class T> struct Tag { using type = T; };

template <class Less, class F>
void for_each_config(F&& f) {
  auto pol = [&](auto tag, const char* pname) {
    using P = typename decltype(tag)::type;
    for (size_t mr : {1, 2, 3, 5, 32}) {
      f(std::string(pname) + "/power/mr" + std::to_string(mr), [mr](Rec* a, size_t n, Less l) { nms::natural_merge_sort<Order::Power, P, false>(a, n, l, nullptr, mr); });
      f(std::string(pname) + "/tim/mr" + std::to_string(mr), [mr](Rec* a, size_t n, Less l) { nms::natural_merge_sort<Order::Tim, P, false>(a, n, l, nullptr, mr); });
    }
  };
  pol(Tag<nms::NaiveT>{}, "naive");
  pol(Tag<nms::FixedT<0>>{}, "fixed0");
  pol(Tag<nms::FixedT<1>>{}, "fixed1");
  pol(Tag<nms::FixedT<3>>{}, "fixed3");
  pol(Tag<nms::FixedT<64>>{}, "fixed64");
  pol(Tag<nms::PolylogT>{}, "polylog");
  pol(Tag<nms::SagT>{}, "sag64j");
  pol(Tag<nms::SagHalving>{}, "sag8h");
  pol(Tag<nms::SagTT<1, true>>{}, "sag1j");
  pol(Tag<nms::SagTT<2, false>>{}, "sag2h");
  for (size_t mr : {2, 32}) {
    f("sag_time16/mr" + std::to_string(mr), [mr](Rec* a, size_t n, Less l) { nms::sag_time_sort<16, false>(a, n, l, nullptr, mr, 4.0); });
    f("sag_be/mr" + std::to_string(mr), [mr](Rec* a, size_t n, Less l) { nms::sag_be_sort<false>(a, n, l, nullptr, mr, 8.0); });
    f("sag_credit/mr" + std::to_string(mr), [mr](Rec* a, size_t n, Less l) { nms::sag_credit_sort<false>(a, n, l, nullptr, mr, 16.0, 50.0); });
    f("sag_credit_m0/mr" + std::to_string(mr), [mr](Rec* a, size_t n, Less l) { nms::sag_credit_sort<false>(a, n, l, nullptr, mr, 0.0, 50.0); });  // degenerate m = 0
  }
}

// ---- verification ------------------------------------------------------------------------------
template <class Less>
static void verify(const char* cfg, const char* fam, size_t n, const std::vector<Rec>& orig, const Rec* out, Less less, bool compare_ref) {
  char tmp[256];
  bool ok = true;
  std::vector<unsigned char> seen(n, 0);
  for (size_t i = 0; i < n && ok; ++i) {
    const Rec& r = out[i];
    if (r.idx >= n || seen[r.idx] || orig[r.idx].key != r.key || r.tag != orig[r.idx].tag) { ok = false; std::snprintf(tmp, sizeof tmp, "%s %s n=%zu: element at %zu is not an original element (permutation/payload)", cfg, fam, n, i); fail(tmp); break; }
    seen[r.idx] = 1;
  }
  for (size_t i = 0; ok && i + 1 < n; ++i) {
    if (less(out[i + 1], out[i])) { ok = false; std::snprintf(tmp, sizeof tmp, "%s %s n=%zu: order violated at %zu", cfg, fam, n, i); fail(tmp); }
    else if (!less(out[i], out[i + 1]) && out[i].idx > out[i + 1].idx) { ok = false; std::snprintf(tmp, sizeof tmp, "%s %s n=%zu: STABILITY violated at %zu (equivalent keys, idx %u before %u)", cfg, fam, n, i, out[i].idx, out[i + 1].idx); fail(tmp); }
  }
  ++g_checks;
  if (compare_ref && ok) {
    std::vector<Rec> ref = orig;
    std::stable_sort(ref.begin(), ref.end(), less);
    ++g_checks;
    for (size_t i = 0; i < n; ++i) if (ref[i].idx != out[i].idx) { std::snprintf(tmp, sizeof tmp, "%s %s n=%zu: differs from std::stable_sort at %zu", cfg, fam, n, i); fail(tmp); break; }
  }
}

static const size_t kGuard = 33;
static const Rec kCanary = {0x5A5A5A5A5A5A5A5Ell, 0xFFFFFFFFu, 0xC0FFEEu};

template <class Less>
static void run_matrix(const char* lname, Less less, const std::vector<size_t>& sizes, std::uint64_t seed0, bool ref_all) {
  for (size_t n : sizes) {
    for (int fam = 0; fam < kNF; ++fam) {
      Rng r(seed0 + 1000003ull * n + 7919ull * fam);
      std::vector<std::int64_t> keys = make_keys(fam, n, r);
      std::vector<Rec> orig(n);
      for (size_t i = 0; i < n; ++i) orig[i] = {keys[i], static_cast<std::uint32_t>(i), static_cast<std::uint32_t>(r.next() & 0xFFFFFF)};
      for_each_config<Less>([&](const std::string& name, auto sorter) {
        std::vector<Rec> buf(n + 2 * kGuard, kCanary);
        std::copy(orig.begin(), orig.end(), buf.begin() + kGuard);
        ++g_sorts;
        sorter(buf.data() + kGuard, n, less);
        bool guards = true;
        for (size_t i = 0; i < kGuard; ++i) {
          const Rec& a = buf[i];
          const Rec& b = buf[kGuard + n + i];
          if (a.key != kCanary.key || a.idx != kCanary.idx || a.tag != kCanary.tag || b.key != kCanary.key || b.idx != kCanary.idx || b.tag != kCanary.tag) guards = false;
        }
        ++g_checks;
        if (!guards) { char t[200]; std::snprintf(t, sizeof t, "%s %s %s n=%zu: CANARY overwritten (out-of-bounds write)", name.c_str(), lname, kFam[fam], n); fail(t); }
        verify((name + "/" + lname).c_str(), kFam[fam], n, orig, buf.data() + kGuard, less, ref_all || n <= 300);
      });
    }
  }
}

// ---- exhaustive finite-domain checks of the galloping routine -----------------------------------
static void gallop_exhaustive() {
  size_t cases = 0;
  double worst = 0;
  size_t worst_t = 0, worst_k = 0, worst_tail = 0;
  for (size_t t = 0; t <= 70; ++t)
    for (size_t len = 0; len <= 160; ++len)
      for (size_t k = 0; k <= len; ++k) {  // k = index of first true (k == len: never true)
        std::vector<int> v(len);
        for (size_t i = 0; i < len; ++i) v[i] = i >= k ? 1 : 0;
        std::uint64_t tests = 0;
        const size_t got = nms::gallop_count<+1>(v.data(), len, t, [](int x) { return x != 0; }, tests);
        ++cases;
        ++g_checks;
        if (got != k) fail("gallop_count returned wrong block length");
        ++g_checks;
        if (tests != nms::gallop_tests_model(t, k, len)) fail("gallop_count tests != gallop_tests_model");
        // backward direction on the reversed array must agree
        std::vector<int> w(v.rbegin(), v.rend());
        std::uint64_t tb = 0;
        ++g_checks;
        if (len > 0 && nms::gallop_count<-1>(w.data() + (len - 1), len, t, [](int x) { return x != 0; }, tb) != k) fail("gallop_count<-1> wrong");
        // per-block bound claimed in sag_policy.hpp: cost <= (4/3) * block length where block length = k+1 (first element free)
        const double ratio = static_cast<double>(tests) / static_cast<double>(k + 1);
        if (ratio > worst) { worst = ratio; worst_t = t; worst_k = k; worst_tail = len; }
      }
  std::printf("gallop exhaustive domain: %zu cases (t<=70, tail<=160, every k); worst tests/(k+1) = %.4f at t=%zu tail=%zu k=%zu (claimed bound 4/3 = 1.3333)\n", cases, worst, worst_t, worst_tail, worst_k);
  ++g_checks;
  if (worst > 4.0 / 3.0 + 1e-12) fail("per-block (4/3) bound exceeded");
}

// ---- per-merge bound on real merges --------------------------------------------------------------
template <class P, bool Count = true>
static double worst_merge_ratio(size_t n, int fam, std::uint64_t seed, Order ord_unused = Order::Power) {
  (void)ord_unused;
  Rng r(seed);
  auto keys = make_keys(fam, n, r);
  std::vector<Rec> a(n);
  for (size_t i = 0; i < n; ++i) a[i] = {keys[i], static_cast<std::uint32_t>(i), 0};
  nms::Stats st;
  std::vector<nms::MergeRecord> log;
  st.log = &log;
  nms::natural_merge_sort<Order::Power, P, true>(a.data(), n, Asc{}, &st, 1);
  double w = 0;
  for (auto& m : log) w = std::max(w, static_cast<double>(m.comparisons) / static_cast<double>(m.a + m.b));
  return w;
}

// ---- SAG-C reset / recovery state machine ---------------------------------------------------------
static void sagc_state_machine() {
  nms::sag_time_params() = {1.0, 16.0, nms::breakeven_block_length(1.0, 16.0), 50.0 * 16.0};
  auto s = nms::SagCreditT::start(1u << 20);
  const size_t tmax = s.tmax;
  CHECK(s.t == tmax && s.credit == 0.0, "SAG-C: a new merge starts at t_max with zero credit (reset per merge)");
  nms::SagCreditT::observe(s, 10000, 0, 20000);
  CHECK(s.t == 0 && s.credit > 0.0, "SAG-C: one long block switches to t = 0");
  int blocks_to_recover = 0;
  while (s.t == 0 && blocks_to_recover < 100000) { nms::SagCreditT::observe(s, 1, 0, 50); ++blocks_to_recover; }
  std::printf("SAG-C recovery: after one 10000-block (c=1ns, m=16ns, cap=50m) t returns to t_max=%zu after %d single-element blocks (credit cap/m = 50)\n", tmax, blocks_to_recover);
  CHECK(s.t == tmax && blocks_to_recover <= 51 && blocks_to_recover >= 1, "SAG-C: credit cap bounds the number of short blocks before recovery to ~cap/m");
  // SAG (round 1) has NO such recovery for short blocks: characterise it.
  auto q = nms::SagT::start(1u << 20);
  nms::SagT::observe(q, 100000, 40, 200000);
  const size_t t_after_long = q.t;
  for (int i = 0; i < 100000; ++i) nms::SagT::observe(q, 1 + i % 5, 1 + i % 5, 50);
  std::printf("SAG(round-1) characterisation: after a long streak t=%zu; after 100000 short linear blocks t=%zu (tmax=%zu): %s\n", t_after_long, q.t, q.tmax,
              (q.t == t_after_long ? "NO recovery on short blocks (known design property, bounded by the 4/3 per-block cost)" : "recovered"));
}

// ---- abuse: inconsistent and throwing comparators ----------------------------------------------------
struct CoinLess {
  std::uint64_t* s;
  bool operator()(const Rec&, const Rec&) const { *s = *s * 6364136223846793005ull + 1442695040888963407ull; return (*s >> 33) & 1; }
};
struct ThrowAfter {
  std::uint64_t* left;
  bool operator()(const Rec& a, const Rec& b) const { if (*left == 0) throw 7; --*left; return a.key < b.key; }
};

static void abuse() {
  int bad_perm = 0, total = 0;
  for (size_t n : {5u, 37u, 300u, 5000u, 20000u}) {
    Rng r(777 + n);
    std::vector<Rec> orig(n);
    for (size_t i = 0; i < n; ++i) orig[i] = {static_cast<std::int64_t>(r.below(50)), static_cast<std::uint32_t>(i), 0};
    for_each_config<CoinLess>([&](const std::string& name, auto sorter) {
      std::uint64_t st = 99 + n;
      std::vector<Rec> buf(n + 2 * kGuard, kCanary);
      std::copy(orig.begin(), orig.end(), buf.begin() + kGuard);
      sorter(buf.data() + kGuard, n, CoinLess{&st});
      bool guards = true;
      for (size_t i = 0; i < kGuard; ++i) if (buf[i].idx != 0xFFFFFFFFu || buf[kGuard + n + i].idx != 0xFFFFFFFFu) guards = false;
      CHECK(guards, "inconsistent comparator: canary overwritten in " + name);
      std::vector<unsigned char> seen(n, 0);
      bool perm = true;
      for (size_t i = 0; i < n; ++i) { auto x = buf[kGuard + i].idx; if (x >= n || seen[x]) { perm = false; break; } seen[x] = 1; }
      ++total;
      if (!perm) ++bad_perm;
    });
  }
  std::printf("inconsistent (random-coin) comparator: %d sorts, out-of-bounds writes: none detected (canaries); output NOT a permutation in %d of them\n", total, bad_perm);
  CHECK(bad_perm == 0, "inconsistent comparator must still leave a permutation of the input");

  // exception safety: NOT promised by the implementation; measure what happens.
  int threw = 0, lost = 0;
  for (std::uint64_t budget : {10ull, 100ull, 1000ull, 5000ull, 20000ull}) {
    const size_t n = 4000;
    Rng r(4242 + budget);
    std::vector<Rec> orig(n);
    for (size_t i = 0; i < n; ++i) orig[i] = {static_cast<std::int64_t>(r.below(1000000)), static_cast<std::uint32_t>(i), 0};
    for (int pol = 0; pol < 3; ++pol) {
      std::vector<Rec> a = orig;
      std::uint64_t left = budget;
      try {
        if (pol == 0) nms::natural_merge_sort<Order::Power, nms::PolylogT, false>(a.data(), n, ThrowAfter{&left}, nullptr, 32);
        else if (pol == 1) nms::natural_merge_sort<Order::Tim, nms::NaiveT, false>(a.data(), n, ThrowAfter{&left}, nullptr, 32);
        else nms::natural_merge_sort<Order::Power, nms::SagT, false>(a.data(), n, ThrowAfter{&left}, nullptr, 32);
      } catch (int) {
        ++threw;
        std::vector<unsigned char> seen(n, 0);
        bool perm = true;
        for (size_t i = 0; i < n; ++i) { auto x = a[i].idx; if (x >= n || seen[x]) { perm = false; break; } seen[x] = 1; }
        if (!perm) ++lost;
      }
    }
  }
  std::printf("throwing comparator (exception safety is not a documented guarantee): %d sorts aborted by an exception; array no longer a permutation (element lost/duplicated) after %d of them\n", threw, lost);
}

// ---- instrumentation integrity ------------------------------------------------------------------------
struct CountingAsc { std::uint64_t* c; bool operator()(const Rec& a, const Rec& b) const { ++*c; return a.key < b.key; } };
static void count_integrity() {
  int bad = 0, total = 0;
  for (size_t n : {0u, 1u, 2u, 50u, 1000u, 33333u}) for (int fam = 0; fam < kNF; ++fam) {
    Rng r(31 * n + fam);
    auto keys = make_keys(fam, n, r);
    std::vector<Rec> o(n);
    for (size_t i = 0; i < n; ++i) o[i] = {keys[i], static_cast<std::uint32_t>(i), 0};
    auto go = [&](auto tag, Order ord, const char* nm) {
      (void)nm;
      using P = typename decltype(tag)::type;
      std::vector<Rec> a = o, b = o;
      std::uint64_t ext = 0;
      nms::Stats st;
      if (ord == Order::Power) {
        nms::natural_merge_sort<Order::Power, P, false>(a.data(), n, CountingAsc{&ext}, nullptr, 32);
        nms::natural_merge_sort<Order::Power, P, true>(b.data(), n, Asc{}, &st, 32);
      } else {
        nms::natural_merge_sort<Order::Tim, P, false>(a.data(), n, CountingAsc{&ext}, nullptr, 32);
        nms::natural_merge_sort<Order::Tim, P, true>(b.data(), n, Asc{}, &st, 32);
      }
      ++total;
      ++g_checks;
      if (st.comparisons() != ext) { ++bad; fail(std::string("Stats.comparisons() != externally counted comparator calls for ") + nm + " " + kFam[fam] + " n=" + std::to_string(n) + ": " + std::to_string(st.comparisons()) + " vs " + std::to_string(ext)); }
      for (size_t i = 0; i < n; ++i) { ++g_checks; if (a[i].idx != b[i].idx) { fail("counting and non-counting builds disagree"); break; } }
    };
    go(Tag<nms::NaiveT>{}, Order::Power, "naive"); go(Tag<nms::PolylogT>{}, Order::Power, "polylog"); go(Tag<nms::SagT>{}, Order::Power, "sag");
    go(Tag<nms::PolylogT>{}, Order::Tim, "tim-polylog"); go(Tag<nms::SagT>{}, Order::Tim, "tim-sag"); go(Tag<nms::FixedT<0>>{}, Order::Power, "fixed0");
  }
  std::printf("instrumentation: Stats.comparisons() equals the externally counted comparator calls in %d of %d sorts\n", total - bad, total);
}

int main(int argc, char** argv) {
  const std::uint64_t base = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20261020ull;
  std::printf("redteam_fuzz: base seed %llu\n", (unsigned long long)base);
  std::vector<size_t> small;
  for (size_t n = 0; n <= 130; ++n) small.push_back(n);
  std::vector<size_t> mid = {255, 256, 257, 511, 1000, 1023, 1024, 1025, 4095, 4096, 4097, 5000};
  std::vector<size_t> big = {20000, 65537, 100003};
  run_matrix("asc", Asc{}, small, base, true);
  run_matrix("desc", Desc{}, small, base + 1, true);
  run_matrix("mod7", Mod7{}, small, base + 2, true);
  run_matrix("asc", Asc{}, mid, base + 3, true);
  run_matrix("desc", Desc{}, mid, base + 4, true);
  run_matrix("mod7", Mod7{}, mid, base + 5, true);
  run_matrix("asc", Asc{}, big, base + 6, false);
  run_matrix("mod7", Mod7{}, big, base + 7, false);
  std::printf("matrix: %llu sorts, %llu checks so far, %llu failures\n", (unsigned long long)g_sorts, (unsigned long long)g_checks, (unsigned long long)g_fail);
  gallop_exhaustive();
  sagc_state_machine();
  count_integrity();
  abuse();
  std::printf("worst per-merge comparisons/(a+b) observed (SAG-style bound claims <= 4/3 + O(1/(a+b))):\n");
  double w_sag = 0, w_pol = 0, w_f0 = 0;
  for (int fam = 0; fam < kNF; ++fam) for (size_t n : {1000u, 20000u, 100000u}) {
    w_sag = std::max(w_sag, worst_merge_ratio<nms::SagT>(n, fam, base + 11 + fam));
    w_pol = std::max(w_pol, worst_merge_ratio<nms::PolylogT>(n, fam, base + 11 + fam));
    w_f0 = std::max(w_f0, worst_merge_ratio<nms::FixedT<0>>(n, fam, base + 11 + fam));
  }
  std::printf("  SagT %.4f   PolylogT %.4f   FixedT<0> %.4f\n", w_sag, w_pol, w_f0);
  std::printf("redteam_fuzz: %llu sorts, %llu checks, %llu failed\n", (unsigned long long)g_sorts, (unsigned long long)g_checks, (unsigned long long)g_fail);
  return g_fail ? 1 : 0;
}
