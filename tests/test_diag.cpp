// test_diag.cpp -- tests of the v4 terminal diagnostics (src/diag.hpp, the TraceEvent hook in natural_merge_sort.hpp).
// Checks: attaching a trace changes nothing; the event stream is consistent with the exact counters; the profiler measures what
// it claims; string records order like their numeric keys; edge cases (n = 1, 2, 3, all equal, sorted, reversed, odd lengths).
#include <set>
#include <vector>

#include "check.hpp"
#include "diag.hpp"

using namespace diag;

static std::vector<TraceEvent> g_ev;
static void collect(void*, const TraceEvent& e) { g_ev.push_back(e); }

template <class R>
static void check_one(const std::vector<R>& in, std::size_t minrun) {
  std::vector<R> ref = in;
  std::stable_sort(ref.begin(), ref.end(), [](const R& a, const R& b) { return a < b; });
  const V vs[] = {V::Textbook, V::Naive, V::Base, V::Sag, V::StdStable};
  for (V v : vs) {
    const Outcome o = run_counted<R>(v, in, ref, minrun);
    CHECKF(o.matches_ref, "%s n=%zu minrun=%zu", name(v), in.size(), minrun);
    CHECKF(o.stable, "%s n=%zu", name(v), in.size());
    CHECKF(o.counters_agree, "%s n=%zu stats=%llu ext=%llu", name(v), in.size(), (unsigned long long)o.st.comparisons(), (unsigned long long)o.ext_cmp);
    if (!is_natural(v)) continue;

    // Attaching a trace must not change the output or any counter.
    g_ev.clear();
    const Outcome t = run_counted<R>(v, in, ref, minrun, collect, nullptr);
    CHECK(t.matches_ref);
    CHECK(t.st.run_comparisons == o.st.run_comparisons && t.st.merge_comparisons == o.st.merge_comparisons);
    CHECK(t.st.merges == o.st.merges && t.st.trivial_merges == o.st.trivial_merges && t.st.blocks == o.st.blocks);
    CHECK(t.st.element_moves == o.st.element_moves && t.st.max_buffer == o.st.max_buffer);

    // The event stream agrees with the counters.
    u64 runs = 0, run_total = 0, begins = 0, ends = 0, trivials = 0, blocks = 0, end_cmp = 0, block_tests = 0, end_blocks = 0, in_merge_blocks = 0;
    std::size_t next_start = 0;
    bool in_merge = false;
    for (const TraceEvent& e : g_ev) {
      switch (e.kind) {
        case nms::Ev::Run: ++runs; run_total += e.c; CHECK(e.a == next_start); next_start += e.c; CHECK(e.c >= e.b); break;
        case nms::Ev::MergeBegin: ++begins; in_merge = true; in_merge_blocks = 0; CHECK(e.b >= 1 && e.c >= 1); break;
        case nms::Ev::MergeTrivial: ++trivials; break;
        case nms::Ev::Block: ++blocks; ++in_merge_blocks; block_tests += e.d; CHECK(in_merge); CHECK(e.a >= 1); break;
        case nms::Ev::TChange: CHECK(in_merge); CHECK(e.a != e.b); break;
        case nms::Ev::MergeEnd: ++ends; end_cmp += e.d; end_blocks += e.e; CHECK(in_merge); CHECK(in_merge_blocks == e.e); in_merge = false; break;
      }
    }
    CHECK(run_total == (in.size() < 2 ? 0u : in.size()));  // n < 2: the sorter returns before detecting runs
    CHECK(begins == ends);
    CHECK(blocks == o.st.blocks && end_blocks == o.st.blocks);
    CHECK(trivials == o.st.trivial_merges);
    CHECK(begins + trivials == o.st.merges);
    CHECK(end_cmp + trivials == o.st.merge_comparisons);  // every skipped merge costs exactly one comparison
    if (v != V::Naive) CHECKF(end_cmp == 2 * begins + block_tests, "end=%llu begins=%llu tests=%llu", (unsigned long long)end_cmp, (unsigned long long)begins, (unsigned long long)block_tests);
    if (v == V::Naive) CHECK(blocks == 0);
    if (v != V::Sag) for (const TraceEvent& e : g_ev) CHECK(e.kind != nms::Ev::TChange);
    Agg ag;
    for (const TraceEvent& e : g_ev) ag.add(e);
    CHECK(ag.runs == runs && ag.blocks == blocks && ag.merges == begins && ag.trivial == trivials);
    CHECK(ag.bins[0] + ag.bins[1] + ag.bins[2] + ag.bins[3] + ag.bins[4] == ag.blocks);
  }
}

static void test_equivalence_matrix() {
  const char* fams[] = {"perm", "few", "zipf", "runs", "mixture", "nearly", "sorted", "reversed", "pipe"};
  const std::size_t sizes[] = {1, 2, 3, 4, 5, 7, 31, 32, 33, 100, 257, 1000, 4097};
  const std::size_t minruns[] = {1, 4, 32};
  for (const char* f : fams)
    for (std::size_t n : sizes) {
      const double p = std::string(f) == "few" ? 3 : std::string(f) == "zipf" ? 64 : std::string(f) == "runs" ? 5 : std::string(f) == "mixture" ? 16 : std::string(f) == "nearly" ? 0.02 : 0;
      const std::vector<std::uint32_t> k = gen::make(f, n, p, 99 + n);
      for (std::size_t mr : minruns) {
        check_one<IntRec>(int_records(k), mr);
        if (n <= 1000) check_one<StrRec>(str_records(k, 24), mr);
      }
    }
  // all equal keys: stability is the only thing that can fail
  for (std::size_t n : {2u, 17u, 500u}) check_one<IntRec>(int_records(std::vector<std::uint32_t>(n, 7)), 4);
}

static void test_profile() {
  {
    const Profile p = profile_of(int_records({1, 2, 3, 4, 5}));
    CHECK(p.sorted && !p.non_increasing && p.runs == 1 && p.asc_runs == 1 && p.desc_runs == 0 && p.descents == 0 && p.distinct == 5);
    CHECK(p.run_min == 5 && p.run_max == 5 && p.H == 0.0);
  }
  {
    const Profile p = profile_of(int_records({5, 4, 3, 2, 1}));
    CHECK(!p.sorted && p.non_increasing && p.runs == 1 && p.desc_runs == 1 && p.descents == 4 && p.distinct == 5);
  }
  {
    const Profile p = profile_of(int_records({3, 1, 2}));  // runs: [3,1] strictly decreasing, [2]
    CHECK(p.runs == 2 && p.desc_runs == 1 && p.asc_runs == 1 && p.descents == 1 && p.distinct == 3 && !p.sorted && !p.non_increasing);
  }
  {
    const Profile p = profile_of(int_records({7, 7, 7, 7}));
    CHECK(p.sorted && p.non_increasing && p.distinct == 1 && p.runs == 1);
  }
  {
    const Profile p = profile_of(int_records({4}));
    CHECK(p.n == 1 && p.sorted && p.runs == 1 && p.distinct == 1);
  }
  for (const char* f : {"perm", "few", "zipf", "runs", "nearly"}) {
    const std::vector<std::uint32_t> k = gen::make(f, 3000, std::string(f) == "few" ? 9 : std::string(f) == "zipf" ? 40 : std::string(f) == "runs" ? 7 : 0.03, 5);
    const Profile p = profile_of(int_records(k));
    CHECK(p.distinct == std::set<std::uint32_t>(k.begin(), k.end()).size());
    CHECK(p.runs == nms::natural_run_lengths(int_records(k)).size());
    CHECK(p.asc_runs + p.desc_runs == p.runs);
    CHECK(std::abs(p.H - nms::run_entropy(k)) < 1e-9 && std::abs(p.Hstar - nms::dual_run_entropy(k)) < 1e-9);  // same numbers as the research code
    const Profile ps = profile_of(str_records(k, 30));  // string keys of the same ordering have the same structure
    CHECK(ps.runs == p.runs && ps.distinct == p.distinct && ps.descents == p.descents);
  }
}

static void test_records() {
  const std::vector<std::uint32_t> k = {0, 1, 15, 16, 255, 4096, 0xFFFFFFFFu, 123456789, 2};
  const std::vector<StrRec> s = str_records(k, 20);
  for (std::size_t i = 0; i < k.size(); ++i)
    for (std::size_t j = 0; j < k.size(); ++j) CHECK((s[i] < s[j]) == (k[i] < k[j]));
  CHECK(label(s[2]) == "0000000f" && label(s[6]) == "ffffffff");
  CHECK(std::strlen(s[0].key) == 20 && s[0].key[0] == 'k' && s[0].key[11] == 'k' && s[0].key[12] == '0');
  CHECK(sorted_and_stable(std::vector<IntRec>{{1, 0}, {1, 1}, {2, 0}}));
  CHECK(!sorted_and_stable(std::vector<IntRec>{{1, 1}, {1, 0}}));  // equal keys out of original order
  CHECK(!sorted_and_stable(std::vector<IntRec>{{2, 0}, {1, 1}}));  // not sorted
}

static void test_trace_known_case() {
  // The 24-element array of demo_sort (minrun 4): three runs 5 / 13 / 6, two merges (left run buffered in the first, right run in the second).
  const std::vector<IntRec> in = int_records({5, 6, 7, 8, 9, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 9, 9, 9, 1, 2, 3, 4, 5, 6});
  std::vector<IntRec> ref = in;
  std::stable_sort(ref.begin(), ref.end(), [](const IntRec& a, const IntRec& b) { return a < b; });
  g_ev.clear();
  const Outcome o = run_counted<IntRec>(V::Sag, in, ref, 4, collect, nullptr);
  CHECK(o.matches_ref && o.stable);
  std::vector<std::size_t> run_len;
  std::vector<const TraceEvent*> begins;
  for (const TraceEvent& e : g_ev) {
    if (e.kind == nms::Ev::Run) run_len.push_back(e.b);
    if (e.kind == nms::Ev::MergeBegin) begins.push_back(&e);
  }
  CHECK(run_len == (std::vector<std::size_t>{5, 13, 6}));
  CHECK(begins.size() == 2);
  if (begins.size() == 2) {
    CHECK(begins[0]->b == 5 && begins[0]->c == 13 && begins[0]->e == 1);   // 5 <= 13: left run buffered
    CHECK(begins[1]->b == 18 && begins[1]->c == 6 && begins[1]->e == 0);   // 18 > 6: right run buffered
    CHECK(begins[0]->d == 25);                                             // t = ceil(log2 24)^2 = 5^2
  }
  CHECK(o.st.comparisons() == 61);  // matches the stored demo output of the base paper policy (identical here: blocks are short)
}

int main() {
  test_equivalence_matrix();
  test_profile();
  test_records();
  test_trace_known_case();
  return chk_report("test_diag");
}
