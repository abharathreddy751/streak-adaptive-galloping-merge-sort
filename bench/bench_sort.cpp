// bench_sort.cpp -- benchmark driver: one process = one workload cell, all requested variants on IDENTICAL input.
//
//   bench_sort --family few --param 64 --n 1048576 --seed 9001 --reps 15 --elem plain
//              --variants naive,base,base_b,sag --cell HELD_few64_n1M --csv results/raw/heldout.csv --pin auto
//
// For every repetition the variants are run in a freshly randomised order on a pristine copy of the input.
// Every output is verified against a reference (std::stable_sort) -- a failed verification is recorded (ok=0).
// A separate, un-timed "counting" pass records exact comparison / move counters for each variant.
// Timed builds contain NO counters (Count=false); counting builds are never timed.
//
// elem=plain    : sort uint32 keys directly (cheap comparisons)
// elem=indirect : sort uint32 indices by keys[index]  (an "argsort": comparisons are cache-missing loads)
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <numeric>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <windows.h>
#endif

#include "entropy.hpp"
#include "generators.hpp"
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
#include "sag_time_policy.hpp"
#include "textbook_merge_sort.hpp"

using Elem = std::uint32_t;
using u64 = std::uint64_t;
using Clock = std::chrono::steady_clock;

// ---------------------------------------------------------------------------------------------
// comparators
struct PlainLess {
  bool operator()(Elem a, Elem b) const { return a < b; }
};
struct IndirectLess {
  const Elem* keys;
  bool operator()(Elem a, Elem b) const { return keys[a] < keys[b]; }
};
// SYNTHETIC expensive comparator: a dependent multiply-add chain of `g_heavy_k` steps (~1 ns per step on the test
// machine) before the key comparison.  It stands in for comparators whose cost dwarfs data movement (long-string
// compares, collation, computed keys).  The chain cannot be removed by the optimiser (asm volatile consumer).
static unsigned g_heavy_k = 40;
struct HeavyLess {
  bool operator()(Elem a, Elem b) const {
    std::uint64_t x = static_cast<std::uint64_t>(a) * 0x9E3779B97F4A7C15ull + b;
    for (unsigned i = 0; i < g_heavy_k; ++i) x = x * 6364136223846793005ull + 1442695040888963407ull;
#if defined(__GNUC__)
    asm volatile("" : : "r"(x));
#endif
    return a < b;
  }
};
template <class Less>
struct CountLess {
  Less l;
  u64* c;
  bool operator()(Elem a, Elem b) const { ++*c; return l(a, b); }
};

// ---------------------------------------------------------------------------------------------
// variants
template <class Less>
struct Var {
  const char* name;
  void (*run)(Elem*, std::size_t, Less);
  nms::Stats (*count)(Elem*, std::size_t, Less);
};
template <nms::Order O, class TP, class Less>
void run_nms(Elem* a, std::size_t n, Less l) { nms::natural_merge_sort<O, TP, false>(a, n, l, nullptr, 32); }
template <nms::Order O, class TP, class Less>
nms::Stats cnt_nms(Elem* a, std::size_t n, Less l) { nms::Stats s; nms::natural_merge_sort<O, TP, true>(a, n, l, &s, 32); return s; }

template <class Less> void run_stdstable(Elem* a, std::size_t n, Less l) { std::stable_sort(a, a + n, l); }
template <class Less> nms::Stats cnt_stdstable(Elem* a, std::size_t n, Less l) { nms::Stats s; u64 c = 0; std::stable_sort(a, a + n, CountLess<Less>{l, &c}); s.merge_comparisons = c; return s; }
template <class Less> void run_topdown(Elem* a, std::size_t n, Less l) { nms::merge_sort_topdown(a, n, l); }
template <class Less> nms::Stats cnt_topdown(Elem* a, std::size_t n, Less l) { nms::Stats s; u64 c = 0; nms::merge_sort_topdown(a, n, CountLess<Less>{l, &c}); s.merge_comparisons = c; return s; }

template <unsigned MS, int M10, class Less>
void run_sagt(Elem* a, std::size_t n, Less l) { nms::sag_time_sort<MS, false>(a, n, l, nullptr, 32, M10 / 10.0); }
template <unsigned MS, int M10, class Less>
nms::Stats cnt_sagt(Elem* a, std::size_t n, Less l) { nms::Stats s; nms::sag_time_sort<MS, true>(a, n, l, &s, 32, M10 / 10.0); return s; }

template <int M10, class Less>
void run_sagbe(Elem* a, std::size_t n, Less l) { nms::sag_be_sort<false>(a, n, l, nullptr, 32, M10 / 10.0); }
template <int M10, class Less>
nms::Stats cnt_sagbe(Elem* a, std::size_t n, Less l) { nms::Stats s; nms::sag_be_sort<true>(a, n, l, &s, 32, M10 / 10.0); return s; }

template <int M10, int CAP, class Less>
void run_sagc(Elem* a, std::size_t n, Less l) { nms::sag_credit_sort<false>(a, n, l, nullptr, 32, M10 / 10.0, CAP); }
template <int M10, int CAP, class Less>
nms::Stats cnt_sagc(Elem* a, std::size_t n, Less l) { nms::Stats s; nms::sag_credit_sort<true>(a, n, l, &s, 32, M10 / 10.0, CAP); return s; }

using SagH4 = nms::SagTT<4, false>;  // development variants: MinStreak x {halving only, jump to 0}
using SagH8 = nms::SagTT<8, false>;
using SagH16 = nms::SagTT<16, false>;
using SagH32 = nms::SagTT<32, false>;
using SagJ4 = nms::SagTT<4, true>;
using SagJ8 = nms::SagTT<8, true>;
using SagJ16 = nms::SagTT<16, true>;
using SagJ32 = nms::SagTT<32, true>;
using SagJ64 = nms::SagTT<64, true>;
using SagJ128 = nms::SagTT<128, true>;

template <class Less>
std::vector<Var<Less>> all_variants() {
  using namespace nms;
#define V(NAME, ORD, TP) {NAME, run_nms<ORD, TP, Less>, cnt_nms<ORD, TP, Less>}
  return {
      V("naive", Order::Power, NaiveT),
      V("base", Order::Power, PolylogT),      // BASE: paper's polylogarithmic galloping, PowerSort order
      V("base_b", Order::Power, PolylogT),    // identical code run a second time = A/A noise control
      V("sag", Order::Power, SagT),           // EXTENSION (final: MinStreak 16, jump to t=0 allowed)
      V("sag_h4", Order::Power, SagH4),       // ablations / development variants (halving only)
      V("sag_h8", Order::Power, SagH8),
      V("sag_h16", Order::Power, SagH16),
      V("sag_h32", Order::Power, SagH32),
      V("sag_j4", Order::Power, SagJ4),       // jump variants
      V("sag_j8", Order::Power, SagJ8),
      V("sag_j16", Order::Power, SagJ16),
      V("sag_j32", Order::Power, SagJ32),
      V("sag_j64", Order::Power, SagJ64),
      V("sag_j128", Order::Power, SagJ128),
      V("t0", Order::Power, FixedT<0>),
      // corrective round: time-aware SAG-T (self-calibrating comparison cost c; m = 0.1 ns * M10)
      {"sagt16_m2", run_sagt<16, 20, Less>, cnt_sagt<16, 20, Less>},
      {"sagt16_m4", run_sagt<16, 40, Less>, cnt_sagt<16, 40, Less>},
      {"sagt16_m8", run_sagt<16, 80, Less>, cnt_sagt<16, 80, Less>},
      {"sagt16_m16", run_sagt<16, 160, Less>, cnt_sagt<16, 160, Less>},
      {"sagt64_m4", run_sagt<64, 40, Less>, cnt_sagt<64, 40, Less>},
      {"sagt64_m8", run_sagt<64, 80, Less>, cnt_sagt<64, 80, Less>},
      {"sagbe_m2", run_sagbe<20, Less>, cnt_sagbe<20, Less>},
      {"sagbe_m4", run_sagbe<40, Less>, cnt_sagbe<40, Less>},
      {"sagbe_m8", run_sagbe<80, Less>, cnt_sagbe<80, Less>},
      {"sagbe_m16", run_sagbe<160, Less>, cnt_sagbe<160, Less>},
      {"sagbe_m32", run_sagbe<320, Less>, cnt_sagbe<320, Less>},
      {"sagc_m8_c10", run_sagc<80, 10, Less>, cnt_sagc<80, 10, Less>},
      {"sagc_m8_c50", run_sagc<80, 50, Less>, cnt_sagc<80, 50, Less>},
      {"sagc_m8_c200", run_sagc<80, 200, Less>, cnt_sagc<80, 200, Less>},
      {"sagc_m4_c50", run_sagc<40, 50, Less>, cnt_sagc<40, 50, Less>},
      {"sagc_m16_c50", run_sagc<160, 50, Less>, cnt_sagc<160, 50, Less>},
      {"sagt", run_sagc<160, 50, Less>, cnt_sagc<160, 50, Less>},  // FINAL corrective-round extension SAG-C (m = 16 ns, cap = 50*m); chosen on dev data  // placeholder alias for the round-2 candidate; fixed on dev data before the freeze
      V("tim_base", Order::Tim, PolylogT),
      V("tim_sag", Order::Tim, SagT),
      {"stdstable", run_stdstable<Less>, cnt_stdstable<Less>},
      {"topdown", run_topdown<Less>, cnt_topdown<Less>},
  };
#undef V
}

// ---------------------------------------------------------------------------------------------
// pinning (Windows: prefer a logical CPU of the highest efficiency class = a P-core)
static std::string pin_thread(const std::string& mode, int& cpu_out) {
  cpu_out = -1;
  if (mode == "none") return "no pinning requested";
#ifdef _WIN32
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
          // Old MinGW headers lack the field; in the Windows SDK layout it is the byte right after `Flags`.
          const int eff = reinterpret_cast<const unsigned char*>(&p->Processor)[1];
          const KAFFINITY m = p->Processor.GroupMask[0].Mask;
          int first = -1;
          for (int b = 0; b < 64; ++b) if ((m >> b) & 1) { first = b; break; }
          if (first >= 0 && (eff > bestEff || (eff == bestEff && first < lp))) { bestEff = eff; lp = first; }
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
  (void)mode;
  return "pinning not implemented on this platform";
#endif
}

// ---------------------------------------------------------------------------------------------
struct Args {
  std::string family = "few", elem = "plain", variants = "naive,base,base_b,sag", csv, cell = "cell", pin = "auto";
  double param = 0;
  std::size_t n = 1u << 20;
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

struct Meta { double H = 0, Hs = 0; std::size_t nruns = 0; };

template <class Less>
static int run_cell(const Args& a, const std::vector<Elem>& keys, const Elem* keytable, Less less, const Meta& meta,
                    const std::string& pin_info, int pin_cpu, std::ofstream& csv) {
  const std::size_t n = keys.size();
  std::vector<Elem> pristine(n);
  if (keytable) std::iota(pristine.begin(), pristine.end(), 0u); else pristine = keys;
  std::vector<Elem> ref = pristine;
  std::stable_sort(ref.begin(), ref.end(), less);
  std::vector<Elem> work(n);

  auto all = all_variants<Less>();
  std::vector<Var<Less>> vars;
  for (const std::string& want : split(a.variants))
    for (const auto& v : all) if (want == v.name) vars.push_back(v);
  if (vars.empty()) { std::fprintf(stderr, "no valid variants\n"); return 2; }

  int failures = 0;
  const auto prefix = [&](const char* kind, const char* variant) {
    char buf[512];
    std::snprintf(buf, sizeof buf, "%s,%s,%s,%g,%zu,%llu,%s,%s", kind, a.cell.c_str(), a.family.c_str(), a.param, n,
                  (unsigned long long)a.seed, a.elem.c_str(), variant);
    return std::string(buf);
  };

  // counting pass (not timed)
  for (const auto& v : vars) {
    work = pristine;
    const nms::Stats s = v.count(work.data(), n, less);
    const int ok = std::memcmp(work.data(), ref.data(), n * sizeof(Elem)) == 0;
    if (!ok) { ++failures; std::fprintf(stderr, "VERIFY FAIL (count) %s %s\n", a.cell.c_str(), v.name); }
    csv << prefix("count", v.name) << ",-1,-1,,," << ok << "," << s.comparisons() << "," << s.run_comparisons << ","
        << s.merge_comparisons << "," << s.merges << "," << s.trivial_merges << "," << s.blocks << "," << s.element_moves << ","
        << s.max_buffer << "," << meta.H << "," << meta.Hs << "," << meta.nruns << "," << pin_cpu << "\n";
  }

  // warm-up (not recorded)
  for (int w = 0; w < a.warmup; ++w)
    for (const auto& v : vars) { work = pristine; v.run(work.data(), n, less); }

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
      v.run(work.data(), n, less);
      const auto t1 = Clock::now();
      const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
      const int ok = std::memcmp(work.data(), ref.data(), n * sizeof(Elem)) == 0;
      if (!ok) { ++failures; std::fprintf(stderr, "VERIFY FAIL %s %s rep %d\n", a.cell.c_str(), v.name, rep); }
      csv << prefix("time", v.name) << "," << rep << "," << pos << "," << static_cast<long long>(ns) << "," << ns / static_cast<double>(n)
          << "," << ok << ",,,,,,,,," << meta.H << "," << meta.Hs << "," << meta.nruns << "," << pin_cpu << "\n";
    }
  }
  (void)pin_info;
  return failures ? 1 : 0;
}

int main(int argc, char** argv) {
  Args a;
  for (int i = 1; i + 1 < argc; i += 2) {
    const std::string k = argv[i], v = argv[i + 1];
    if (k == "--family") a.family = v; else if (k == "--param") a.param = std::atof(v.c_str());
    else if (k == "--n") a.n = static_cast<std::size_t>(std::atoll(v.c_str())); else if (k == "--seed") a.seed = std::strtoull(v.c_str(), nullptr, 10);
    else if (k == "--reps") a.reps = std::atoi(v.c_str()); else if (k == "--warmup") a.warmup = std::atoi(v.c_str());
    else if (k == "--elem") a.elem = v; else if (k == "--variants") a.variants = v; else if (k == "--csv") a.csv = v;
    else if (k == "--cell") a.cell = v; else if (k == "--pin") a.pin = v;
    else if (k == "--heavy_k") g_heavy_k = static_cast<unsigned>(std::atoi(v.c_str()));
    else { std::fprintf(stderr, "unknown option %s\n", k.c_str()); return 2; }
  }
  int pin_cpu = -1;
  const std::string pin_info = pin_thread(a.pin, pin_cpu);
  std::fprintf(stderr, "[bench] cell=%s family=%s param=%g n=%zu seed=%llu elem=%s reps=%d | %s\n", a.cell.c_str(), a.family.c_str(), a.param,
               a.n, (unsigned long long)a.seed, a.elem.c_str(), a.reps, pin_info.c_str());

  const std::vector<Elem> keys = gen::make(a.family, a.n, a.param, a.seed);
  if (keys.size() != a.n) { std::fprintf(stderr, "unknown family %s\n", a.family.c_str()); return 2; }
  Meta meta;
  meta.H = nms::run_entropy(keys);
  meta.Hs = nms::dual_run_entropy(keys);
  meta.nruns = nms::natural_run_lengths(keys).size();

  std::ofstream csv;
  if (!a.csv.empty()) {
    bool fresh = true;
    { std::ifstream t(a.csv, std::ios::binary); fresh = !t || t.peek() == std::ifstream::traits_type::eof(); }
    csv.open(a.csv, std::ios::app);
    if (fresh)
      csv << "kind,cell,family,param,n,seed,elem,variant,rep,order_pos,ns_total,ns_per_elem,ok,comparisons,run_cmp,merge_cmp,merges,trivial_merges,blocks,moves,max_buffer,H,Hstar,nruns,pin_cpu\n";
  }
  std::ofstream null_sink;
  std::ofstream& out = a.csv.empty() ? null_sink : csv;
  int rc;
  if (a.elem == "indirect") rc = run_cell(a, keys, keys.data(), IndirectLess{keys.data()}, meta, pin_info, pin_cpu, out);
  else if (a.elem == "heavy") rc = run_cell(a, keys, nullptr, HeavyLess{}, meta, pin_info, pin_cpu, out);
  else rc = run_cell(a, keys, nullptr, PlainLess{}, meta, pin_info, pin_cpu, out);
  std::fprintf(stderr, "[bench] cell=%s done rc=%d (H=%.3f H*=%.3f runs=%zu)\n", a.cell.c_str(), rc, meta.H, meta.Hs, meta.nruns);
  return rc;
}
