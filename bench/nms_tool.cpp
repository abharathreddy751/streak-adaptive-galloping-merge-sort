// nms_tool.cpp -- terminal tool for demonstrating and measuring the Merge Sort project (v4).  No GUI, no files written, no network.
//   nms_tool help | profile | trace | compare | strings        (see docs/TERMINAL_DEMO.md)
// Exit codes: 0 ok, 1 a correctness check failed, 2 usage error.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "diag.hpp"

#ifndef NMS_BUILD_FLAGS
#define NMS_BUILD_FLAGS "(not recorded: build with scripts/build.sh)"
#endif

using namespace diag;

namespace {

struct Opts {
  std::string cmd, family = "perm", key = "int", policy = "sag";
  double param = 0;
  bool have_param = false, have_keys = false, have_n = false, have_family = false, have_key = false;
  std::size_t n = 100000, minrun = 0;
  u64 seed = 2024;
  unsigned keylen = 48, reps = 0;
  std::vector<std::uint32_t> keys;
};

[[noreturn]] void usage_error(const std::string& msg) {
  std::fprintf(stderr, "nms_tool: error: %s\nTry: nms_tool help\n", msg.c_str());
  std::exit(2);
}

bool parse_u64(const char* s, u64& out) {  // digits only, no overflow
  if (!*s) return false;
  u64 v = 0;
  for (; *s; ++s) {
    if (*s < '0' || *s > '9') return false;
    const u64 d = static_cast<u64>(*s - '0');
    if (v > (~0ull - d) / 10) return false;
    v = v * 10 + d;
  }
  out = v;
  return true;
}

bool known_family(const std::string& f) {
  static const char* ok[] = {"perm", "few", "zipf", "runs", "mixture", "nearly", "sorted", "reversed", "pipe"};
  for (const char* s : ok) if (f == s) return true;
  return false;
}

double default_param(const std::string& f) {
  if (f == "few") return 16;
  if (f == "zipf") return 1024;
  if (f == "runs") return 16;
  if (f == "mixture") return 64;
  if (f == "nearly") return 0.01;
  return 0;
}

Opts parse(int argc, char** argv) {
  Opts o;
  if (argc < 2) usage_error("missing command");
  o.cmd = argv[1];
  for (int i = 2; i < argc; ++i) {
    const std::string a = argv[i];
    auto val = [&]() -> const char* {
      if (i + 1 >= argc) usage_error("option " + a + " needs a value");
      return argv[++i];
    };
    u64 u = 0;
    if (a == "--family") { o.family = val(); o.have_family = true; if (!known_family(o.family)) usage_error("unknown --family '" + o.family + "' (perm few zipf runs mixture nearly sorted reversed pipe)"); }
    else if (a == "--n") { if (!parse_u64(val(), u) || u < 1 || u > 20000000) usage_error("--n must be an integer in [1, 20000000]"); o.n = static_cast<std::size_t>(u); o.have_n = true; }
    else if (a == "--param") { char* e = nullptr; const char* s = val(); o.param = std::strtod(s, &e); if (e == s || *e || !(o.param >= 0) || o.param > 1e9) usage_error("--param must be a number in [0, 1e9]"); o.have_param = true; }
    else if (a == "--seed") { if (!parse_u64(val(), u)) usage_error("--seed must be a non-negative integer"); o.seed = u; }
    else if (a == "--minrun") { if (!parse_u64(val(), u) || u < 1 || u > 1024) usage_error("--minrun must be an integer in [1, 1024]"); o.minrun = static_cast<std::size_t>(u); }
    else if (a == "--reps") { if (!parse_u64(val(), u) || u < 1 || u > 1000) usage_error("--reps must be an integer in [1, 1000]"); o.reps = static_cast<unsigned>(u); }
    else if (a == "--keylen") { if (!parse_u64(val(), u) || u < 8 || u > 63) usage_error("--keylen must be an integer in [8, 63]"); o.keylen = static_cast<unsigned>(u); }
    else if (a == "--key") { o.key = val(); o.have_key = true; if (o.key != "int" && o.key != "str") usage_error("--key must be int or str"); }
    else if (a == "--policy") { o.policy = val(); if (o.policy != "base" && o.policy != "sag") usage_error("--policy must be base or sag"); }
    else if (a == "--keys") {
      std::string s = val();
      std::size_t p = 0;
      while (p <= s.size()) {
        std::size_t q = s.find(',', p);
        if (q == std::string::npos) q = s.size();
        if (!parse_u64(s.substr(p, q - p).c_str(), u) || u > 0xFFFFFFFFull) usage_error("--keys must be a comma-separated list of integers in [0, 4294967295]");
        o.keys.push_back(static_cast<std::uint32_t>(u));
        p = q + 1;
      }
      o.have_keys = true;
    }
    else usage_error("unknown option '" + a + "'");
  }
  if (o.have_keys && (o.have_family || o.have_n || o.have_param)) usage_error("--keys cannot be combined with --family/--n/--param");
  if (!o.have_param) o.param = default_param(o.family);
  return o;
}

std::string meta(const Opts& o, std::size_t n) {
  char b[256];
  if (o.have_keys) std::snprintf(b, sizeof b, "explicit list (--keys), n=%zu", n);
  else std::snprintf(b, sizeof b, "generator family=%s param=%g seed=%llu n=%zu", o.family.c_str(), o.param, static_cast<unsigned long long>(o.seed), n);
  return b;
}

std::vector<std::uint32_t> make_keys(const Opts& o) {
  if (o.have_keys) return o.keys;
  std::vector<std::uint32_t> k = gen::make(o.family, o.n, o.param, o.seed);
  if (k.size() != o.n) usage_error("generator produced no data for these parameters");
  return k;
}

void print_env() {
  std::printf("environment: compiler %s | flags %s | pointer %zu bit | timing clock steady_clock\n", __VERSION__, NMS_BUILD_FLAGS,
              sizeof(void*) * 8);
}

template <class R>
void print_profile(const std::vector<R>& rec, const std::string& gen_meta, const char* keyname) {
  const Profile p = profile_of(rec);
  std::printf("== input profile ==\n");
  std::printf("generator metadata (what was ASKED for): %s, key type %s\n", gen_meta.c_str(), keyname);
  std::printf("measured by the profiler (what is IN the data):\n");
  std::printf("  elements n                 : %zu\n", p.n);
  std::printf("  distinct keys              : %zu (%.2f%% of n)\n", p.distinct, 100.0 * p.distinct / p.n);
  std::printf("  adjacent descents          : %zu (pairs with a[i+1] < a[i]; %.3f%% of the n-1 pairs)\n", p.descents,
              p.n > 1 ? 100.0 * p.descents / (p.n - 1) : 0.0);
  std::printf("  natural runs               : %zu (%zu non-decreasing, %zu strictly decreasing)\n", p.runs, p.asc_runs, p.desc_runs);
  std::printf("  run length min/mean/max    : %zu / %.1f / %zu\n", p.run_min, p.run_mean, p.run_max);
  std::printf("  run entropy H              : %.3f bits   dual-run entropy H* : %.3f bits   (log2 n = %.3f)\n", p.H, p.Hstar, p.log2n);
  const bool nearly = !p.sorted && p.n > 1 && p.descents * 100 <= p.n - 1;
  const bool dup = p.distinct * 10 <= p.n;
  const bool lowH = p.Hstar <= 0.5 * p.log2n;
  std::printf("labels (fixed definitions, see docs/TERMINAL_DEMO.md):\n");
  std::printf("  already sorted             : %s  (no adjacent descent)\n", p.sorted ? "yes" : "no");
  std::printf("  reverse sorted             : %s  (no adjacent ascent, i.e. non-increasing)\n", p.non_increasing && p.n > 1 ? "yes" : "no");
  std::printf("  nearly sorted              : %s  (not sorted and descents <= 1%% of pairs)\n", nearly ? "yes" : "no");
  std::printf("  duplicate-heavy            : %s  (distinct keys <= 10%% of n)\n", dup ? "yes" : "no");
  std::printf("  low-entropy (project def.) : %s  (H* <= 0.5 * log2 n)\n", lowH ? "yes" : "no");
}

template <class R>
bool run_trace(const Opts& o, std::vector<R> rec) {
  if (rec.size() > 128) usage_error("trace is limited to n <= 128 (it prints every event); use 'compare' for large inputs");
  const std::size_t minrun = o.minrun ? o.minrun : 4;
  std::vector<R> ref = rec;
  std::stable_sort(ref.begin(), ref.end(), [](const R& a, const R& b) { return a < b; });
  std::vector<TraceEvent> ev;
  const V v = o.policy == "base" ? V::Base : V::Sag;
  const Outcome out = run_counted<R>(v, rec, ref, minrun, [](void* c, const TraceEvent& e) { static_cast<std::vector<TraceEvent>*>(c)->push_back(e); }, &ev);
  std::printf("== execution trace: %s policy (%s merge order = PowerSort), minrun = %zu ==\n", out.v == V::Sag ? "SAG" : "base", out.v == V::Sag ? "SagT<64,true>" : "PolylogT", minrun);
  std::printf("[1] input received (%zu elements):\n    ", rec.size());
  for (const R& r : rec) std::printf("%s ", label(r).c_str());
  std::printf("\nEvery line below is one event emitted by the sorter while it ran.\n");
  std::size_t shown_run = 0;
  for (const TraceEvent& e : ev) {
    switch (e.kind) {
      case nms::Ev::Run:
        std::printf("[run %zu] elements %zu..%zu: run detection found %zu%s\n", ++shown_run, e.a, e.a + e.c - 1, e.b,
                    e.c > e.b ? " -> too short, extended to minrun by binary insertion" : "");
        break;
      case nms::Ev::MergeBegin:
        std::printf("[merge] at %zu: left run %zu + right run %zu | %s copied to the buffer | t = %llu\n", e.a, e.b, e.c,
                    e.e ? "left run" : "right run", static_cast<unsigned long long>(e.d));
        break;
      case nms::Ev::MergeTrivial:
        std::printf("[merge skipped] at %zu: left %zu + right %zu already in order (1 comparison)\n", e.a, e.b, e.c);
        break;
      case nms::Ev::Block:
        std::printf("    block of %zu element(s) from the %s run | found with t = %zu | %llu comparison(s) | %llu left in that run\n", e.a,
                    e.b ? "left" : "right", e.c, static_cast<unsigned long long>(e.d), static_cast<unsigned long long>(e.e));
        break;
      case nms::Ev::TChange:
        std::printf("    SAG adapted: t %zu -> %zu after a block of %zu\n", e.a, e.b, e.c);
        break;
      case nms::Ev::MergeEnd:
        std::printf("[merge done] at %zu: %llu comparisons, %llu block(s)\n", e.a, static_cast<unsigned long long>(e.d), static_cast<unsigned long long>(e.e));
        break;
    }
  }
  std::printf("[result] sorted: ");
  std::vector<R> sorted = ref;
  for (const R& r : sorted) std::printf("%s ", label(r).c_str());
  std::printf("\n[verify] output identical to the reference stable order (keys and ids): %s | independent sorted+stable test: %s | counters agree with external counter: %s\n",
              out.matches_ref ? "YES" : "NO", out.stable ? "YES" : "NO", out.counters_agree ? "YES" : "NO");
  std::printf("[totals] run-detection comparisons %llu + merge comparisons %llu = %llu | merges %llu (skipped %llu) | blocks %llu | element moves %llu\n",
              (unsigned long long)out.st.run_comparisons, (unsigned long long)out.st.merge_comparisons, (unsigned long long)out.st.comparisons(),
              (unsigned long long)out.st.merges, (unsigned long long)out.st.trivial_merges, (unsigned long long)out.st.blocks, (unsigned long long)out.st.element_moves);
  return out.matches_ref && out.stable && out.counters_agree;
}

template <class R>
bool run_compare(const Opts& o, const std::vector<R>& rec, const char* title) {
  const std::size_t minrun = o.minrun ? o.minrun : 32;
  std::vector<R> ref = rec;
  std::stable_sort(ref.begin(), ref.end(), [](const R& a, const R& b) { return a < b; });
  const double n = static_cast<double>(rec.size());
  std::printf("== %s: identical input copy for every variant, minrun = %zu ==\n", title, minrun);
  std::printf("%-17s %12s %9s %12s %8s %8s %10s %7s %7s\n", "variant", "comparisons", "cmp/elem", "moves", "merges", "skipped", "blocks", "correct", "stable");
  bool ok = true;
  Agg agg_base, agg_sag;
  Outcome base_o, sag_o;
  const V vs[] = {V::Textbook, V::Naive, V::Base, V::Sag, V::StdStable};
  for (V v : vs) {
    Agg* ag = v == V::Base ? &agg_base : v == V::Sag ? &agg_sag : nullptr;
    const Outcome out = run_counted<R>(v, rec, ref, minrun, ag ? Agg::cb : nullptr, ag);
    if (v == V::Base) base_o = out;
    if (v == V::Sag) sag_o = out;
    ok = ok && out.matches_ref && out.stable && out.counters_agree;
    if (out.has_stats)
      std::printf("%-17s %12llu %9.3f %12llu %8llu %8llu %10llu %7s %7s\n", name(v), (unsigned long long)out.ext_cmp, out.ext_cmp / n,
                  (unsigned long long)out.st.element_moves, (unsigned long long)out.st.merges, (unsigned long long)out.st.trivial_merges,
                  (unsigned long long)out.st.blocks, out.matches_ref ? "yes" : "NO", out.stable ? "yes" : "NO");
    else
      std::printf("%-17s %12llu %9.3f %12s %8s %8s %10s %7s %7s\n", name(v), (unsigned long long)out.ext_cmp, out.ext_cmp / n, "n/a", "n/a", "n/a", "n/a",
                  out.matches_ref ? "yes" : "NO", out.stable ? "yes" : "NO");
  }
  if (!base_o.counters_agree || !sag_o.counters_agree) std::printf("COUNTER MISMATCH between the sorter's Stats and the external counter\n");
  auto detail = [&](const char* nm, const Agg& a, const Outcome& oc) {
    std::printf("%-5s detail: runs %llu (%llu extended to minrun) | run-detection cmp %llu, merge cmp %llu | blocks %llu", nm, (unsigned long long)a.runs,
                (unsigned long long)a.runs_extended, (unsigned long long)oc.st.run_comparisons, (unsigned long long)oc.st.merge_comparisons, (unsigned long long)a.blocks);
    if (a.blocks) std::printf(" (length min/mean/max %llu / %.2f / %llu)", (unsigned long long)a.block_min, static_cast<double>(a.block_sum) / a.blocks, (unsigned long long)a.block_max);
    std::printf("\n      block length histogram: 1:%llu  2-3:%llu  4-15:%llu  16-63:%llu  >=64:%llu | t changes %llu in %llu of %llu non-skipped merges\n",
                (unsigned long long)a.bins[0], (unsigned long long)a.bins[1], (unsigned long long)a.bins[2], (unsigned long long)a.bins[3], (unsigned long long)a.bins[4],
                (unsigned long long)a.t_changes, (unsigned long long)a.merges_adapted, (unsigned long long)a.merges);
  };
  detail("base", agg_base, base_o);
  detail("SAG", agg_sag, sag_o);
  if (base_o.ext_cmp)
    std::printf("SAG / base comparisons: %.4f   (exact counts; fewer comparisons do not by themselves mean less time)\n",
                static_cast<double>(sag_o.ext_cmp) / static_cast<double>(base_o.ext_cmp));
  else
    std::printf("SAG / base comparisons: n/a (base used no comparison)\n");
  if (o.reps) {
    std::printf("-- wall-clock time, uncounted builds, %u timed runs after 1 warm-up, copy made outside the timer --\n", o.reps);
    std::printf("%-17s %12s %12s %12s %8s\n", "variant", "median ms", "min ms", "max ms", "correct");
    double base_med = 0, sag_med = 0;
    for (V v : vs) {
      const TimeSummary t = time_variant<R>(v, rec, ref, minrun, o.reps);
      ok = ok && t.correct;
      if (v == V::Base) base_med = t.median_ms;
      if (v == V::Sag) sag_med = t.median_ms;
      std::printf("%-17s %12.3f %12.3f %12.3f %8s\n", name(v), t.median_ms, t.min_ms, t.max_ms, t.correct ? "yes" : "NO");
    }
    if (base_med > 0) std::printf("SAG / base ratio of median times: %.3f (one machine, one input; descriptive only, not a significance test; see docs/TERMINAL_DEMO.md)\n", sag_med / base_med);
  }
  return ok;
}

void help() {
  std::printf(
      "nms_tool -- terminal demonstration of the Merge Sort project (natural merge sort, galloping base policy, SAG extension)\n\n"
      "commands:\n"
      "  profile   measured description of an input\n"
      "  trace     step-by-step execution events for a small input (n <= 128)\n"
      "  compare   textbook / naive / base / SAG / std::stable_sort on the same input: exact counters (+ optional timing)\n"
      "  strings   compare with expensive 64-byte string keys and a stability check (defaults: --key str --family few --param 16 --n 200000 --reps 5)\n"
      "  help      this text\n\n"
      "input options (profile, trace, compare, strings):\n"
      "  --keys 5,6,7,...        explicit integer keys (exclusive with the generator options)\n"
      "  --family F              perm few zipf runs mixture nearly sorted reversed pipe   (default perm)\n"
      "  --n N  --param P  --seed S   size [1, 20000000] (default 100000), family parameter, seed (default 2024)\n"
      "  --key int|str           integer records or 64-byte string records (default int)\n"
      "  --keylen L              string key length [8, 63] (default 48): L-8 identical prefix bytes + 8 varying hex digits\n"
      "  --minrun M              minimum run length [1, 1024] (default 32; trace default 4)\n"
      "  --policy base|sag       trace only (default sag)\n"
      "  --reps R                compare/strings: also time every variant, R timed runs [1, 1000]\n\n"
      "trace example : nms_tool trace --keys 5,6,7,8,9,1,1,1,2,2,2,2,3,3,3,9,9,9,1,2,3,4,5,6\n"
      "compare example: nms_tool compare --family few --param 3 --n 262144\n"
      "exit codes: 0 ok | 1 a correctness check failed | 2 usage error\n");
}

template <class R>
bool dispatch(const Opts& o, const std::vector<std::uint32_t>& keys, const std::vector<R>& rec) {
  (void)keys;
  const char* keyname = std::is_same<R, StrRec>::value ? "str (64-byte)" : "int (uint32)";
  if (o.cmd == "profile") { print_profile(rec, meta(o, rec.size()), keyname); return true; }
  if (o.cmd == "trace") return run_trace<R>(o, rec);
  std::string m = meta(o, rec.size());
  std::printf("input: %s, key type %s\n", m.c_str(), keyname);
  print_profile(rec, m, keyname);
  return run_compare<R>(o, rec, o.cmd == "strings" ? "string-key comparison" : "comparison");
}

}  // namespace

int main(int argc, char** argv) {
  if (argc >= 2 && (std::strcmp(argv[1], "help") == 0 || std::strcmp(argv[1], "--help") == 0 || std::strcmp(argv[1], "-h") == 0)) { help(); return 0; }
  Opts o = parse(argc, argv);
  if (o.cmd != "profile" && o.cmd != "trace" && o.cmd != "compare" && o.cmd != "strings") usage_error("unknown command '" + o.cmd + "'");
  if (o.cmd == "strings") {
    if (!o.have_key) o.key = "str";
    if (!o.have_keys && !o.have_family) { o.family = "few"; if (!o.have_param) o.param = 16; }
    if (!o.have_keys && !o.have_n) o.n = 200000;
    if (!o.reps) o.reps = 5;
    if (o.key != "str") usage_error("'strings' needs --key str");
  }
  if (o.cmd == "trace" && !o.have_keys && !o.have_n && !o.have_family) {
    o.keys = {5, 6, 7, 8, 9, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 9, 9, 9, 1, 2, 3, 4, 5, 6};  // the 24-element array of demo_sort
    o.have_keys = true;
  }
  const std::vector<std::uint32_t> keys = make_keys(o);
  if (o.cmd != "trace") print_env();
  bool ok;
  if (o.key == "str") ok = dispatch<StrRec>(o, keys, str_records(keys, o.keylen));
  else ok = dispatch<IntRec>(o, keys, int_records(keys));
  if (!ok) { std::printf("RESULT: FAIL (a correctness or counter check failed)\n"); return 1; }
  if (o.cmd != "profile") std::printf("RESULT: OK (all outputs equal the reference stable sort; all counters consistent)\n");
  return 0;
}
