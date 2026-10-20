// bench_string3.cpp -- STAGE-3 confirmatory string-key benchmark.  One process = one cell; all variants sort the IDENTICAL input.
//
// Elements: 16-byte records {const char* p; uint32 len; uint32 idx} pointing into a packed arena that holds ONE PRIVATE COPY of every
// element's string (generation order).  The comparator is a genuine lexicographic comparison (std::string_view::compare, length breaks ties),
// declared noexcept (API contract of src/natural_merge_sort.hpp).  idx = original position -> equal keys are distinguishable (stability).
//
// Key sources (all SYNTHETIC unless --style FILE):
//   style S : short random words, 4..16 chars, no shared directory structure (cheap comparisons)
//   style M : medium hierarchical "paths" /w1/w2/.../leaf, components drawn from small random vocabularies; 25..130 chars
//   style L : long hierarchical paths with long directory components (shared prefixes of 40..250 chars); 80..330 chars
//   style FILE: real strings read from a text file (one string per line); family "natural" keeps file order, "shuffled" permutes it
// For S/M/L the STRUCTURE comes from the frozen integer generators (src/generators.hpp): the integer value u of every element is replaced by
// the string of rank dense_rank(u) in a sorted pool of distinct random strings, so the string array is order-isomorphic to u.
// Structural properties (H, H*, natural runs of the RANK array; mean string length; mean adjacent-LCP+1 of the sorted array = "cost_B")
// are computed from the generated data, never from timings.
//
//   bench_string3 --style M --family few --param 16 --n 131072 --seed 7300012 --reps 15 --variants naive,base,base_b,sag,sagc,tim_base,stdstable,topdown
//                 --cell S3_M_few16_n17 --csv stage3/results/x.csv --pin auto [--file path] [--structure-only 1]
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <numeric>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#if defined(_WIN32) || defined(__CYGWIN__)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <windows.h>
#define HAVE_WINPIN 1
#endif

#include "entropy.hpp"
#include "generators.hpp"
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
#include "sag_time_policy.hpp"
#include "textbook_merge_sort.hpp"

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using Clock = std::chrono::steady_clock;

struct SRec { const char* p; u32 len; u32 idx; };
static_assert(std::is_trivially_copyable_v<SRec>);

struct StrLess {
  bool operator()(const SRec& a, const SRec& b) const noexcept {
    return std::string_view(a.p, a.len).compare(std::string_view(b.p, b.len)) < 0;
  }
};
static_assert(std::is_nothrow_invocable_r_v<bool, const StrLess&, const SRec&, const SRec&>, "comparator must be noexcept (API contract)");
struct CountLess {
  u64* c;
  bool operator()(const SRec& a, const SRec& b) const noexcept { ++*c; return StrLess{}(a, b); }
};

struct Var {
  const char* name;
  void (*run)(SRec*, std::size_t);
  nms::Stats (*count)(SRec*, std::size_t, u64& calib_cmp, double& c_ns);
};
template <nms::Order O, class TP>
void run_nms(SRec* a, std::size_t n) { nms::natural_merge_sort<O, TP, false>(a, n, StrLess{}, nullptr, 32); }
template <nms::Order O, class TP>
nms::Stats cnt_nms(SRec* a, std::size_t n, u64& cc, double& cn) { cc = 0; cn = 0; nms::Stats s; nms::natural_merge_sort<O, TP, true>(a, n, StrLess{}, &s, 32); return s; }
// SAG-C: frozen round-2 configuration (m = 16 ns, cap = 50 m), secondary diagnostic only -- NOT retuned.
static void run_sagc(SRec* a, std::size_t n) { nms::sag_credit_sort<false>(a, n, StrLess{}, nullptr, 32, 16.0, 50.0); }
static nms::Stats cnt_sagc(SRec* a, std::size_t n, u64& cc, double& cn) {
  nms::Stats s;
  nms::sag_credit_sort<true>(a, n, StrLess{}, &s, 32, 16.0, 50.0);
  cc = n >= 4096 ? 16384 : 0;
  cn = n >= 4096 ? nms::sag_time_params().c_ns : 0;
  return s;
}
static void run_std(SRec* a, std::size_t n) { std::stable_sort(a, a + n, StrLess{}); }
static nms::Stats cnt_std(SRec* a, std::size_t n, u64& cc, double& cn) { cc = 0; cn = 0; nms::Stats s; u64 c = 0; std::stable_sort(a, a + n, CountLess{&c}); s.merge_comparisons = c; return s; }
static void run_top(SRec* a, std::size_t n) { nms::merge_sort_topdown(a, n, StrLess{}); }
static nms::Stats cnt_top(SRec* a, std::size_t n, u64& cc, double& cn) { cc = 0; cn = 0; nms::Stats s; u64 c = 0; nms::merge_sort_topdown(a, n, CountLess{&c}); s.merge_comparisons = c; return s; }

static std::vector<Var> all_variants() {
  using namespace nms;
  return {
      {"naive", run_nms<Order::Power, NaiveT>, cnt_nms<Order::Power, NaiveT>},
      {"base", run_nms<Order::Power, PolylogT>, cnt_nms<Order::Power, PolylogT>},
      {"base_b", run_nms<Order::Power, PolylogT>, cnt_nms<Order::Power, PolylogT>},  // identical code: A/A noise control
      {"sag", run_nms<Order::Power, SagT>, cnt_nms<Order::Power, SagT>},              // PRIMARY candidate: SagTT<64,true>
      {"sagc", run_sagc, cnt_sagc},                                                     // secondary diagnostic (frozen)
      {"tim_base", run_nms<Order::Tim, PolylogT>, cnt_nms<Order::Tim, PolylogT>},
      {"stdstable", run_std, cnt_std},
      {"topdown", run_top, cnt_top},
  };
}

// ---------------------------------------------------------------------------------------------
static std::string pin_thread(const std::string& mode, int& cpu_out) {
  cpu_out = -1;
  if (mode == "none") return "no pinning requested";
#ifdef HAVE_WINPIN
  int lp = -1;
  std::string note;
  if (mode == "auto") {
    DWORD len = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &len);
    std::vector<unsigned char> buf(len ? len : 1);
    if (len && GetLogicalProcessorInformationEx(RelationProcessorCore, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buf.data()), &len)) {
      int bestEff = -1;
      for (DWORD off = 0; off < len;) {
        auto* p = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buf.data() + off);
        if (p->Relationship == RelationProcessorCore && p->Processor.GroupCount >= 1 && p->Processor.GroupMask[0].Group == 0) {
          const int eff = reinterpret_cast<const unsigned char*>(&p->Processor)[1];
          const KAFFINITY m = p->Processor.GroupMask[0].Mask;
          int first = -1;
          for (int b = 0; b < 64; ++b) if ((m >> b) & 1) { first = b; break; }
          if (first >= 2 && (eff > bestEff || (eff == bestEff && first < lp))) { bestEff = eff; lp = first; }
        }
        off += p->Size;
      }
      note = "highest core efficiency class=" + std::to_string(bestEff);
    } else note = "GetLogicalProcessorInformationEx failed";
  } else lp = std::atoi(mode.c_str());
  if (lp < 0 || lp >= 64) return "pin failed: no usable logical processor (" + note + ")";
  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
  const DWORD_PTR old = SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(1) << lp);
  if (!old) return "SetThreadAffinityMask failed";
  Sleep(20);
  int same = 0;
  for (int i = 0; i < 1000; ++i) { if (static_cast<int>(GetCurrentProcessorNumber()) == lp) ++same; }
  cpu_out = lp;
  return "pinned to logical CPU " + std::to_string(lp) + " (" + note + "), verified " + std::to_string(same) + "/1000 samples on that CPU";
#else
  return "pinning not implemented on this platform";
#endif
}

// ---------------------------------------------------------------------------------------------
// synthetic string pools
struct StyleCfg { int dmin, dmax; int lo[4], hi[4]; int vocab[4]; int leaf_lo, leaf_hi; };
static StyleCfg style_cfg(char s) {
  if (s == 'S') return {0, 0, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 4, 16};
  if (s == 'M') return {2, 4, {6, 4, 3, 3}, {30, 20, 16, 12}, {16, 48, 128, 256}, 6, 28};
  return {2, 4, {60, 30, 12, 8}, {140, 90, 60, 40}, {8, 24, 64, 128}, 10, 40};  // 'L'
}
static std::string rword(gen::Rng& r, int lo, int hi) {
  static const char al[] = "abcdefghijklmnopqrstuvwxyz0123456789";
  const int len = lo + static_cast<int>(r.bounded(static_cast<u64>(hi - lo + 1)));
  std::string w(static_cast<size_t>(len), 'a');
  for (auto& c : w) c = al[r.bounded(36)];
  return w;
}
static std::vector<std::string> make_pool(char style, size_t m, u64 seed) {
  const StyleCfg c = style_cfg(style);
  gen::Rng r(seed ^ 0xC0FFEE1234567ull);
  std::vector<std::vector<std::string>> vocab(4);
  for (int l = 0; l < 4; ++l) for (int v = 0; v < c.vocab[l]; ++v) vocab[l].push_back(rword(r, c.lo[l], c.hi[l]));
  std::unordered_set<std::string> seen;
  seen.reserve(m * 2);
  std::vector<std::string> pool;
  pool.reserve(m);
  while (pool.size() < m) {
    std::string s;
    const int depth = c.dmin + static_cast<int>(r.bounded(static_cast<u64>(c.dmax - c.dmin + 1)));
    for (int l = 0; l < depth; ++l) { s += '/'; s += vocab[l][r.bounded(vocab[l].size())]; }
    if (depth) s += '/';
    s += rword(r, c.leaf_lo, c.leaf_hi);
    if (seen.insert(s).second) pool.push_back(std::move(s));
  }
  std::sort(pool.begin(), pool.end());
  return pool;
}

struct Args {
  std::string style = "M", family = "few", variants = "naive,base,base_b,sag,sagc", csv, cell = "cell", pin = "auto", file;
  double param = 0;
  std::size_t n = 1u << 17;
  u64 seed = 1;
  int reps = 11, warmup = 1, structure_only = 0;
};
static std::vector<std::string> split(const std::string& s) {
  std::vector<std::string> out;
  std::size_t i = 0;
  while (i <= s.size()) {
    const std::size_t j = s.find(',', i);
    out.push_back(s.substr(i, j == std::string::npos ? std::string::npos : j - i));
    if (j == std::string::npos) break;
    i = j + 1;
  }
  return out;
}

int main(int argc, char** argv) {
  Args a;
  for (int i = 1; i + 1 < argc; i += 2) {
    const std::string k = argv[i], v = argv[i + 1];
    if (k == "--style") a.style = v; else if (k == "--family") a.family = v; else if (k == "--param") a.param = std::atof(v.c_str());
    else if (k == "--n") a.n = static_cast<std::size_t>(std::atoll(v.c_str())); else if (k == "--seed") a.seed = std::strtoull(v.c_str(), nullptr, 10);
    else if (k == "--reps") a.reps = std::atoi(v.c_str()); else if (k == "--warmup") a.warmup = std::atoi(v.c_str());
    else if (k == "--variants") a.variants = v; else if (k == "--csv") a.csv = v; else if (k == "--cell") a.cell = v;
    else if (k == "--pin") a.pin = v; else if (k == "--file") a.file = v; else if (k == "--structure-only") a.structure_only = std::atoi(v.c_str());
    else { std::fprintf(stderr, "unknown option %s\n", k.c_str()); return 2; }
  }
  int pin_cpu = -1;
  const std::string pin_info = a.structure_only ? "structure only" : pin_thread(a.pin, pin_cpu);
  std::fprintf(stderr, "[bench_string3] cell=%s style=%s family=%s param=%g n=%zu seed=%llu reps=%d | %s\n", a.cell.c_str(), a.style.c_str(), a.family.c_str(), a.param,
               a.n, (unsigned long long)a.seed, a.reps, pin_info.c_str());

  // ---- build the element strings (one private copy each, packed in generation order) and the rank array (structure)
  std::vector<std::string_view> src(0);
  std::vector<std::string> pool;       // keeps synthetic pool alive
  std::vector<std::string> filelines;  // keeps file lines alive
  std::vector<u32> ranks;
  if (a.style == "FILE") {
    std::ifstream f(a.file);
    std::string line;
    while (std::getline(f, line) && filelines.size() < a.n) { if (!line.empty() && line.back() == '\r') line.pop_back(); if (!line.empty()) filelines.push_back(line); }
    if (filelines.empty()) { std::fprintf(stderr, "cannot read %s\n", a.file.c_str()); return 2; }
    a.n = filelines.size();
    if (a.family == "shuffled") { gen::Rng r(a.seed); for (size_t i = filelines.size(); i > 1; --i) std::swap(filelines[i - 1], filelines[r.bounded(i)]); }
    else if (a.family != "natural") { std::fprintf(stderr, "FILE family must be natural|shuffled\n"); return 2; }
    std::vector<std::string> uniq = filelines;
    std::sort(uniq.begin(), uniq.end());
    uniq.erase(std::unique(uniq.begin(), uniq.end()), uniq.end());
    ranks.resize(a.n);
    for (size_t i = 0; i < a.n; ++i) ranks[i] = static_cast<u32>(std::lower_bound(uniq.begin(), uniq.end(), filelines[i]) - uniq.begin());
    for (auto& s : filelines) src.emplace_back(s);
  } else {
    const std::vector<u32> u = gen::make(a.family, a.n, a.param, a.seed);
    if (u.size() != a.n) { std::fprintf(stderr, "unknown family %s\n", a.family.c_str()); return 2; }
    std::vector<u32> d = u;
    std::sort(d.begin(), d.end());
    d.erase(std::unique(d.begin(), d.end()), d.end());
    ranks.resize(a.n);
    for (size_t i = 0; i < a.n; ++i) ranks[i] = static_cast<u32>(std::lower_bound(d.begin(), d.end(), u[i]) - d.begin());
    pool = make_pool(a.style[0], d.size(), a.seed);
    for (size_t i = 0; i < a.n; ++i) src.emplace_back(pool[ranks[i]]);
  }
  size_t total_bytes = 0;
  for (auto s : src) total_bytes += s.size() + 1;
  std::vector<char> arena(total_bytes);
  std::vector<SRec> pristine(a.n);
  {
    size_t off = 0;
    for (size_t i = 0; i < a.n; ++i) {
      std::memcpy(arena.data() + off, src[i].data(), src[i].size());
      arena[off + src[i].size()] = 0;
      pristine[i] = {arena.data() + off, static_cast<u32>(src[i].size()), static_cast<u32>(i)};
      off += src[i].size() + 1;
    }
  }
  src.clear(); pool.clear(); pool.shrink_to_fit(); filelines.clear(); filelines.shrink_to_fit();
  // structural properties (from data only)
  const double H = nms::run_entropy(ranks), Hs = nms::dual_run_entropy(ranks);
  const std::size_t nruns = nms::natural_run_lengths(ranks).size();
  std::vector<SRec> ref = pristine;
  std::stable_sort(ref.begin(), ref.end(), StrLess{});
  double sum_len = 0, sum_lcp = 0;
  for (size_t i = 0; i < a.n; ++i) sum_len += pristine[i].len;
  for (size_t i = 0; i + 1 < a.n; ++i) {
    const SRec &x = ref[i], &y = ref[i + 1];
    const u32 m = std::min(x.len, y.len);
    u32 l = 0;
    while (l < m && x.p[l] == y.p[l]) ++l;
    sum_lcp += std::min<u32>(l + 1, m);  // bytes a comparison of these two examines (at most the shorter length)
  }
  const double mean_len = sum_len / static_cast<double>(a.n), cost_B = a.n > 1 ? sum_lcp / static_cast<double>(a.n - 1) : 0;
  // integrity of the representation: every record points at its own NUL-terminated copy of exactly len bytes
  for (size_t i = 0; i < a.n; ++i) if (pristine[i].p[pristine[i].len] != 0 || std::memchr(pristine[i].p, 0, pristine[i].len)) { std::fprintf(stderr, "REPRESENTATION ERROR at %zu\n", i); return 3; }

  std::ofstream csv;
  if (!a.csv.empty()) {
    bool fresh = true;
    { std::ifstream t(a.csv, std::ios::binary); fresh = !t || t.peek() == std::ifstream::traits_type::eof(); }
    csv.open(a.csv, std::ios::app);
    if (fresh) csv << "kind,cell,family,param,n,seed,elem,variant,rep,order_pos,ns_total,ns_per_elem,ok,comparisons,run_cmp,merge_cmp,merges,trivial_merges,blocks,moves,max_buffer,H,Hstar,nruns,pin_cpu,calib_cmp,c_ns,style,mean_len,cost_B,total_bytes\n";
  }
  std::ofstream null_sink;
  std::ofstream& out = a.csv.empty() ? null_sink : csv;
  const std::string tail_meta = "," + a.style + "," + std::to_string(mean_len) + "," + std::to_string(cost_B) + "," + std::to_string(total_bytes);
  const auto prefix_s = [&](const char* kind, const char* variant) {
    char buf[512];
    std::snprintf(buf, sizeof buf, "%s,%s,%s,%g,%zu,%llu,string,%s", kind, a.cell.c_str(), a.family.c_str(), a.param, a.n, (unsigned long long)a.seed, variant);
    return std::string(buf);
  };
  if (a.structure_only) {  // classification dry run: one structural row, nothing is sorted or timed
    out << prefix_s("struct", "-") << ",-1,-1,,,1,,,,,,,,," << H << "," << Hs << "," << nruns << "," << pin_cpu << ",," << tail_meta << "\n";
    std::fprintf(stderr, "[bench_string3] structure H=%.3f H*=%.3f runs=%zu mean_len=%.1f cost_B=%.1f\n", H, Hs, nruns, mean_len, cost_B);
    return 0;
  }
  std::vector<SRec> work(a.n);
  auto verify = [&](const std::vector<SRec>& o) {
    for (std::size_t i = 0; i < a.n; ++i) if (o[i].idx != ref[i].idx || o[i].p != ref[i].p || o[i].len != ref[i].len) return 0;
    for (std::size_t i = 0; i + 1 < a.n; ++i) {  // independent invariant scan: order + stability
      if (StrLess{}(o[i + 1], o[i])) return 0;
      if (!StrLess{}(o[i], o[i + 1]) && o[i].idx > o[i + 1].idx) return 0;
    }
    return 1;
  };
  auto all = all_variants();
  std::vector<Var> vars;
  for (const std::string& want : split(a.variants))
    for (const auto& v : all) if (want == v.name) vars.push_back(v);
  if (vars.empty()) { std::fprintf(stderr, "no valid variants\n"); return 2; }
  int failures = 0;
  for (const auto& v : vars) {  // counting pass (not timed)
    work = pristine;
    u64 cc = 0; double cn = 0;
    const nms::Stats s = v.count(work.data(), a.n, cc, cn);
    const int ok = verify(work);
    if (!ok) { ++failures; std::fprintf(stderr, "VERIFY FAIL (count) %s %s\n", a.cell.c_str(), v.name); }
    out << prefix_s("count", v.name) << ",-1,-1,,," << ok << "," << s.comparisons() << "," << s.run_comparisons << "," << s.merge_comparisons << "," << s.merges << ","
        << s.trivial_merges << "," << s.blocks << "," << s.element_moves << "," << s.max_buffer << "," << H << "," << Hs << "," << nruns << "," << pin_cpu << "," << cc << "," << cn << tail_meta << "\n";
  }
  for (int w = 0; w < a.warmup; ++w) for (const auto& v : vars) { work = pristine; v.run(work.data(), a.n); }
  for (int rep = 0; rep < a.reps; ++rep) {
    std::vector<std::size_t> order(vars.size());
    std::iota(order.begin(), order.end(), 0);
    gen::Rng rng(a.seed * 0x9E3779B97F4A7C15ull + static_cast<u64>(rep) + 1);
    for (std::size_t i = order.size(); i > 1; --i) std::swap(order[i - 1], order[rng.bounded(i)]);
    for (std::size_t pos = 0; pos < order.size(); ++pos) {
      const auto& v = vars[order[pos]];
      work = pristine;
      const auto t0 = Clock::now();
      v.run(work.data(), a.n);
      const auto t1 = Clock::now();
      const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
      const int ok = verify(work);
      if (!ok) { ++failures; std::fprintf(stderr, "VERIFY FAIL %s %s rep %d\n", a.cell.c_str(), v.name, rep); }
      out << prefix_s("time", v.name) << "," << rep << "," << pos << "," << static_cast<long long>(ns) << "," << ns / static_cast<double>(a.n) << "," << ok << ",,,,,,,,," << H << "," << Hs << "," << nruns << "," << pin_cpu << ",," << tail_meta << "\n";
    }
  }
  std::fprintf(stderr, "[bench_string3] cell=%s done rc=%d (H=%.3f H*=%.3f runs=%zu mean_len=%.1f cost_B=%.1f)\n", a.cell.c_str(), failures ? 1 : 0, H, Hs, nruns, mean_len, cost_B);
  return failures ? 1 : 0;
}
