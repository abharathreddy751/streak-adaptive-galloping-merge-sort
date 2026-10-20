// bench_string.cpp -- STAGE-2 real-comparator experiment.  One process = one workload cell; all variants sort the IDENTICAL input.
//
// Elements are 16-byte records {const char* p; uint32 len; uint32 idx} pointing into a contiguous arena of SYNTHETIC strings
//     key string = <PREFIX: P identical characters> + <8 lower-case hex digits of the integer key v>      (v from src/generators.hpp)
// The comparator is a genuine lexicographic string comparison (std::string_view::compare = memcmp over up to P+8 bytes, preceded by
// two pointer dereferences into a large arena -> real cache misses).  No sleeps, no dummy arithmetic, no volatile loops.
// The integer structure (few-valued, zipf, nearly sorted, ...) is carried over exactly because hex-digit order == numeric order.
// `idx` is the original position, so EQUAL keys are distinguishable and stability is verified from the output.
//
// Per repetition the variants run in a freshly randomised order on a pristine copy; time = steady_clock around the sort call only
// (SAG-C's self-calibration is INSIDE its call).  A separate un-timed counting pass records exact comparison counts.
// Every output is verified (idx sequence == std::stable_sort reference AND an independent order+stability scan).
//
//   bench_string --family few --param 16 --n 131072 --seed 7000012 --reps 15 --variants naive,base,base_b,sag,sagc
//                --cell STR_few16_n17 --csv stage2/results/x.csv --pin auto [--prefix 240]
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
using Clock = std::chrono::steady_clock;

struct SRec { const char* p; std::uint32_t len; std::uint32_t idx; };
static_assert(std::is_trivially_copyable_v<SRec>);

struct StrLess {
  bool operator()(const SRec& a, const SRec& b) const {
    return std::string_view(a.p, a.len).compare(std::string_view(b.p, b.len)) < 0;
  }
};
struct CountLess {
  u64* c;
  bool operator()(const SRec& a, const SRec& b) const { ++*c; return StrLess{}(a, b); }
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

// SAG-C: the frozen round-2 configuration (m = 16 ns, cap = 50 m) -- NOT retuned for strings.
static void run_sagc(SRec* a, std::size_t n) { nms::sag_credit_sort<false>(a, n, StrLess{}, nullptr, 32, 16.0, 50.0); }
static nms::Stats cnt_sagc(SRec* a, std::size_t n, u64& cc, double& cn) {
  nms::Stats s;
  nms::sag_credit_sort<true>(a, n, StrLess{}, &s, 32, 16.0, 50.0);
  cc = n >= 4096 ? 16384 : 0;  // calibration comparisons (measure_comparison_ns, S = 16384) are not in Stats
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
      {"sag", run_nms<Order::Power, SagT>, cnt_nms<Order::Power, SagT>},
      {"sagc", run_sagc, cnt_sagc},
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
          const int eff = reinterpret_cast<const unsigned char*>(&p->Processor)[1];  // EfficiencyClass (byte after Flags)
          const KAFFINITY m = p->Processor.GroupMask[0].Mask;
          int first = -1;
          for (int b = 0; b < 64; ++b) if ((m >> b) & 1) { first = b; break; }
          if (first >= 2 && (eff > bestEff || (eff == bestEff && first < lp))) { bestEff = eff; lp = first; }  // skip logical CPUs 0,1 (interrupt-heavy)
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

struct Args {
  std::string family = "few", variants = "naive,base,base_b,sag,sagc", csv, cell = "cell", pin = "auto";
  double param = 0;
  std::size_t n = 1u << 17, prefix = 240;
  u64 seed = 1;
  int reps = 11, warmup = 1;
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
    if (k == "--family") a.family = v; else if (k == "--param") a.param = std::atof(v.c_str());
    else if (k == "--n") a.n = static_cast<std::size_t>(std::atoll(v.c_str())); else if (k == "--seed") a.seed = std::strtoull(v.c_str(), nullptr, 10);
    else if (k == "--reps") a.reps = std::atoi(v.c_str()); else if (k == "--warmup") a.warmup = std::atoi(v.c_str());
    else if (k == "--variants") a.variants = v; else if (k == "--csv") a.csv = v; else if (k == "--cell") a.cell = v;
    else if (k == "--pin") a.pin = v; else if (k == "--prefix") a.prefix = static_cast<std::size_t>(std::atoll(v.c_str()));
    else { std::fprintf(stderr, "unknown option %s\n", k.c_str()); return 2; }
  }
  int pin_cpu = -1;
  const std::string pin_info = pin_thread(a.pin, pin_cpu);
  std::fprintf(stderr, "[bench_string] cell=%s family=%s param=%g n=%zu seed=%llu prefix=%zu reps=%d | %s\n", a.cell.c_str(), a.family.c_str(), a.param,
               a.n, (unsigned long long)a.seed, a.prefix, a.reps, pin_info.c_str());

  const std::vector<std::uint32_t> keys = gen::make(a.family, a.n, a.param, a.seed);
  if (keys.size() != a.n) { std::fprintf(stderr, "unknown family %s\n", a.family.c_str()); return 2; }
  const double H = nms::run_entropy(keys), Hs = nms::dual_run_entropy(keys);
  const std::size_t nruns = nms::natural_run_lengths(keys).size();

  // ---- build the string arena (generation order = memory order; sorting permutes only the records)
  const std::size_t stride = (a.prefix + 8 + 1 + 63) / 64 * 64;
  std::vector<char> arena(a.n * stride);
  {
    gen::Rng r(a.seed ^ 0x5555AAAA5555AAAAull);
    std::string prefix(a.prefix, 'a');
    for (auto& ch : prefix) ch = static_cast<char>('a' + r.bounded(26));
    static const char hex[] = "0123456789abcdef";
    for (std::size_t i = 0; i < a.n; ++i) {
      char* s = arena.data() + i * stride;
      std::memcpy(s, prefix.data(), a.prefix);
      for (int d = 0; d < 8; ++d) s[a.prefix + d] = hex[(keys[i] >> (28 - 4 * d)) & 15];
      s[a.prefix + 8] = 0;
    }
  }
  const std::size_t slen = a.prefix + 8;
  std::vector<SRec> pristine(a.n);
  for (std::size_t i = 0; i < a.n; ++i) pristine[i] = {arena.data() + i * stride, static_cast<std::uint32_t>(slen), static_cast<std::uint32_t>(i)};
  std::vector<SRec> ref = pristine;
  std::stable_sort(ref.begin(), ref.end(), StrLess{});
  std::vector<SRec> work(a.n);

  auto verify = [&](const std::vector<SRec>& out) {
    for (std::size_t i = 0; i < a.n; ++i) if (out[i].idx != ref[i].idx || out[i].p != ref[i].p) return 0;
    for (std::size_t i = 0; i + 1 < a.n; ++i) {  // independent invariant scan (order + stability)
      if (StrLess{}(out[i + 1], out[i])) return 0;
      if (!StrLess{}(out[i], out[i + 1]) && out[i].idx > out[i + 1].idx) return 0;
    }
    return 1;
  };

  auto all = all_variants();
  std::vector<Var> vars;
  for (const std::string& want : split(a.variants))
    for (const auto& v : all) if (want == v.name) vars.push_back(v);
  if (vars.empty()) { std::fprintf(stderr, "no valid variants\n"); return 2; }

  std::ofstream csv;
  if (!a.csv.empty()) {
    bool fresh = true;
    { std::ifstream t(a.csv, std::ios::binary); fresh = !t || t.peek() == std::ifstream::traits_type::eof(); }
    csv.open(a.csv, std::ios::app);
    if (fresh) csv << "kind,cell,family,param,n,seed,elem,variant,rep,order_pos,ns_total,ns_per_elem,ok,comparisons,run_cmp,merge_cmp,merges,trivial_merges,blocks,moves,max_buffer,H,Hstar,nruns,pin_cpu,calib_cmp,c_ns\n";
  }
  std::ofstream null_sink;
  std::ofstream& out = a.csv.empty() ? null_sink : csv;
  int failures = 0;
  const auto prefix_s = [&](const char* kind, const char* variant) {
    char buf[512];
    std::snprintf(buf, sizeof buf, "%s,%s,%s,%g,%zu,%llu,string,%s", kind, a.cell.c_str(), a.family.c_str(), a.param, a.n, (unsigned long long)a.seed, variant);
    return std::string(buf);
  };

  // counting pass (not timed)
  for (const auto& v : vars) {
    work = pristine;
    u64 cc = 0; double cn = 0;
    const nms::Stats s = v.count(work.data(), a.n, cc, cn);
    const int ok = verify(work);
    if (!ok) { ++failures; std::fprintf(stderr, "VERIFY FAIL (count) %s %s\n", a.cell.c_str(), v.name); }
    out << prefix_s("count", v.name) << ",-1,-1,,," << ok << "," << s.comparisons() << "," << s.run_comparisons << "," << s.merge_comparisons << "," << s.merges << ","
        << s.trivial_merges << "," << s.blocks << "," << s.element_moves << "," << s.max_buffer << "," << H << "," << Hs << "," << nruns << "," << pin_cpu << "," << cc << "," << cn << "\n";
  }
  // warm-up (not recorded)
  for (int w = 0; w < a.warmup; ++w)
    for (const auto& v : vars) { work = pristine; v.run(work.data(), a.n); }
  // timed repetitions, variant order randomised per repetition
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
      out << prefix_s("time", v.name) << "," << rep << "," << pos << "," << static_cast<long long>(ns) << "," << ns / static_cast<double>(a.n) << "," << ok << ",,,,,,,,," << H << "," << Hs << "," << nruns << "," << pin_cpu << ",,\n";
    }
  }
  std::fprintf(stderr, "[bench_string] cell=%s done rc=%d (H=%.3f H*=%.3f runs=%zu)\n", a.cell.c_str(), failures ? 1 : 0, H, Hs, nruns);
  return failures ? 1 : 0;
}
