// exc_repro.cpp -- deterministic reproduction / regression test for "comparator throws mid-sort".
// For every configuration and every k, the comparator throws on its k-th call (k = 1 .. total calls of an unthrottled run).
// After the exception the array must still be a PERMUTATION of the input with payload intact (basic guarantee).
// Not promised: sortedness or stability after an exception.   Exit 1 if any case leaves lost/duplicated elements AND
// the program is run with --require (regression mode); without --require it only reports (reproduction mode).
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
#include "sag_time_policy.hpp"

struct Rec { std::int64_t key; std::uint32_t idx; std::uint32_t tag; };
struct Asc { bool operator()(const Rec& a, const Rec& b) const { return a.key < b.key; } };
struct ThrowAt {  // NOT noexcept: may throw
  std::uint64_t* left;
  bool operator()(const Rec& a, const Rec& b) const { if (--*left == 0) throw 7; return a.key < b.key; }
};
struct CountAsc { std::uint64_t* c; bool operator()(const Rec& a, const Rec& b) const { ++*c; return a.key < b.key; } };

static std::uint64_t rs = 12345;
static std::uint64_t rnd() { rs = rs * 6364136223846793005ull + 1442695040888963407ull; return rs >> 33; }

template <nms::Order O, class P>
static void one(const char* name, size_t mr, const std::vector<Rec>& orig, std::uint64_t& cases, std::uint64_t& bad, std::uint64_t& thrown) {
  const size_t n = orig.size();
  std::uint64_t calls = 0;
  { std::vector<Rec> a = orig; nms::natural_merge_sort<O, P, false>(a.data(), n, CountAsc{&calls}, nullptr, mr); }
  for (std::uint64_t k = 1; k <= calls + 1; ++k) {
    std::vector<Rec> a = orig;
    std::uint64_t left = k;
    bool threw = false;
    try { nms::natural_merge_sort<O, P, false>(a.data(), n, ThrowAt{&left}, nullptr, mr); } catch (int) { threw = true; }
    ++cases;
    if (threw) ++thrown;
    std::vector<unsigned char> seen(n, 0);
    bool perm = true;
    for (size_t i = 0; i < n; ++i) { const Rec& r = a[i]; if (r.idx >= n || seen[r.idx] || r.key != orig[r.idx].key || r.tag != orig[r.idx].tag) { perm = false; break; } seen[r.idx] = 1; }
    if (!perm) { ++bad; if (bad <= 3) std::printf("  LOST/DUPLICATED: %s mr=%zu n=%zu throw-at-call %llu of %llu\n", name, mr, n, (unsigned long long)k, (unsigned long long)calls); }
  }
}

int main(int argc, char** argv) {
  const bool require = argc > 1 && std::strcmp(argv[1], "--require") == 0;
  std::uint64_t cases = 0, bad = 0, thrown = 0;
  for (int fam = 0; fam < 4; ++fam)
    for (size_t n : {2u, 3u, 17u, 64u, 300u}) {
      std::vector<Rec> orig(n);
      for (size_t i = 0; i < n; ++i) {
        std::int64_t key = fam == 0 ? (std::int64_t)(rnd() % 1000000) : fam == 1 ? (std::int64_t)(rnd() % 3) : fam == 2 ? (std::int64_t)i : (std::int64_t)((i / 5 % 2) ? i : n * 2 - i);
        orig[i] = {key, (std::uint32_t)i, (std::uint32_t)(rnd() & 0xFFFFF)};
      }
      for (size_t mr : {1u, 4u, 32u}) {
        one<nms::Order::Power, nms::PolylogT>("power/polylog", mr, orig, cases, bad, thrown);
        one<nms::Order::Tim, nms::NaiveT>("tim/naive", mr, orig, cases, bad, thrown);
        one<nms::Order::Power, nms::SagT>("power/sag", mr, orig, cases, bad, thrown);
        one<nms::Order::Tim, nms::FixedT<0>>("tim/fixed0", mr, orig, cases, bad, thrown);
      }
    }
  std::printf("throw-at-every-call test: %llu cases, %llu aborted by the exception, %llu left the array NOT a permutation (lost/duplicated elements)\n",
              (unsigned long long)cases, (unsigned long long)thrown, (unsigned long long)bad);
  if (require) { std::printf(bad ? "RESULT: FAIL (basic guarantee violated)\n" : "RESULT: PASS (basic guarantee holds in every case)\n"); return bad ? 1 : 0; }
  return 0;
}
