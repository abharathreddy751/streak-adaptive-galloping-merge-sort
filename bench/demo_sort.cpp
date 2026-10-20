// demo_sort.cpp -- small, human-readable demonstration of the project (no timing, fully deterministic).
//   Part 1: a 24-element array sorted step by step: per-merge trace for the BASE paper policy and for SAG.
//   Part 2: comparisons per element on five inputs (n = 262,144): naive merge, base (paper), SAG (proposed),
//           std::stable_sort and the textbook top-down Merge Sort, with the input's entropies H (runs) and H* (dual runs).
// Exit code 0 iff every output equals the reference stable sort.
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "entropy.hpp"
#include "generators.hpp"
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
#include "textbook_merge_sort.hpp"

using nms::Order;
using nms::Stats;

struct Tag { std::uint32_t key, id; };
struct TagLess { bool operator()(const Tag& a, const Tag& b) const { return a.key < b.key; } };

static std::vector<Tag> tagged(const std::vector<std::uint32_t>& k) {
  std::vector<Tag> v(k.size());
  for (std::size_t i = 0; i < k.size(); ++i) v[i] = {k[i], static_cast<std::uint32_t>(i)};
  return v;
}
static bool equal(const std::vector<Tag>& a, const std::vector<Tag>& b) {
  if (a.size() != b.size()) return false;
  for (std::size_t i = 0; i < a.size(); ++i) if (a[i].key != b[i].key || a[i].id != b[i].id) return false;
  return true;
}

template <class TP>
static Stats run_counted(const std::vector<std::uint32_t>& keys, std::size_t minrun, std::vector<nms::MergeRecord>* log, bool& ok) {
  std::vector<Tag> v = tagged(keys), ref = tagged(keys);
  std::stable_sort(ref.begin(), ref.end(), TagLess{});
  Stats st;
  st.log = log;
  nms::natural_merge_sort<Order::Power, TP, true>(v.data(), v.size(), TagLess{}, &st, minrun);
  ok = ok && equal(v, ref);
  return st;
}

static void part1(bool& ok) {
  const std::vector<std::uint32_t> keys = {5, 6, 7, 8, 9, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 9, 9, 9, 1, 2, 3, 4, 5, 6};
  std::printf("== Part 1: 24 elements, minrun = 4, PowerSort merge order ==\ninput : ");
  for (auto k : keys) std::printf("%u ", k);
  std::printf("\n");
  std::vector<Tag> v = tagged(keys);
  std::stable_sort(v.begin(), v.end(), TagLess{});
  std::printf("sorted: ");
  for (const Tag& t : v) std::printf("%u ", t.key);
  std::printf("\n\n");
  std::vector<nms::MergeRecord> lb, ls;
  const Stats b = run_counted<nms::PolylogT>(keys, 4, &lb, ok);
  const Stats s = run_counted<nms::SagT>(keys, 4, &ls, ok);
  std::printf("%-6s %-14s %-9s %-8s %-s\n", "policy", "merge (a+b)", "t at start", "blocks", "comparisons in merge");
  for (const auto& r : lb) std::printf("%-6s %4zu + %-7zu %-9zu %-8llu %llu\n", "base", r.a, r.b, r.t_start, (unsigned long long)r.blocks, (unsigned long long)r.comparisons);
  for (const auto& r : ls) std::printf("%-6s %4zu + %-7zu %-9zu %-8llu %llu\n", "SAG", r.a, r.b, r.t_start, (unsigned long long)r.blocks, (unsigned long long)r.comparisons);
  std::printf("totals: base %llu comparisons (run detection %llu + merges %llu), SAG %llu (identical here: blocks are short)\n\n",
              (unsigned long long)b.comparisons(), (unsigned long long)b.run_comparisons, (unsigned long long)b.merge_comparisons,
              (unsigned long long)s.comparisons());
}

static void part2(bool& ok) {
  struct W { const char* name; const char* fam; double p; };
  const W ws[] = {{"random permutation", "perm", 0}, {"few(3) values", "few", 3}, {"few(64) values", "few", 64},
                  {"zipf(1024) values", "zipf", 1024}, {"16 sorted runs", "runs", 16}};
  const std::size_t n = 1u << 18;
  std::printf("== Part 2: comparisons per element, n = %zu (deterministic counts, minrun = 32) ==\n", n);
  std::printf("%-20s %6s %6s | %8s %8s %8s | %10s %10s\n", "input", "H", "H*", "naive", "base", "SAG", "stable_sort", "topdown");
  for (const W& w : ws) {
    const auto keys = gen::make(w.fam, n, w.p, 2024);
    const double H = nms::run_entropy(keys), Hs = nms::dual_run_entropy(keys);
    const Stats sn = run_counted<nms::NaiveT>(keys, 32, nullptr, ok);
    const Stats sb = run_counted<nms::PolylogT>(keys, 32, nullptr, ok);
    const Stats ss = run_counted<nms::SagT>(keys, 32, nullptr, ok);
    std::vector<Tag> v1 = tagged(keys), v2 = tagged(keys), ref = tagged(keys);
    std::stable_sort(ref.begin(), ref.end(), TagLess{});
    unsigned long long c1 = 0, c2 = 0;
    std::stable_sort(v1.begin(), v1.end(), [&](const Tag& a, const Tag& b2) { ++c1; return a.key < b2.key; });
    nms::merge_sort_topdown(v2.data(), v2.size(), [&](const Tag& a, const Tag& b2) { ++c2; return a.key < b2.key; });
    ok = ok && equal(v1, ref) && equal(v2, ref);
    const double d = static_cast<double>(n);
    std::printf("%-20s %6.2f %6.2f | %8.3f %8.3f %8.3f | %10.3f %10.3f\n", w.name, H, Hs, sn.comparisons() / d, sb.comparisons() / d,
                ss.comparisons() / d, c1 / d, c2 / d);
  }
  std::printf("\n(Comparisons are not time: see results/ for wall-clock measurements, where SAG helps only if a comparison is expensive.)\n");
}

int main() {
  bool ok = true;
  part1(ok);
  part2(ok);
  std::printf("\nall outputs equal the reference stable sort: %s\n", ok ? "YES" : "NO");
  return ok ? 0 : 1;
}
