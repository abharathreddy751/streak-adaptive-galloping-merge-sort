// Micro-benchmark of the sorter (integer keys) to quantify any performance effect of the exception guard.  Prints median ns/element.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <windows.h>
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
#include "generators.hpp"
using u32 = std::uint32_t;
struct Plain { bool operator()(u32 a, u32 b) const { return a < b; } };
struct Nx { bool operator()(u32 a, u32 b) const noexcept { return a < b; } };
template <class L, nms::Order O, class P>
static double med(const char* tag, const std::vector<u32>& in) {
  std::vector<double> t;
  for (int r = 0; r < 7; ++r) {
    std::vector<u32> a = in;
    auto t0 = std::chrono::steady_clock::now();
    nms::natural_merge_sort<O, P, false>(a.data(), a.size(), L{}, nullptr, 32);
    auto t1 = std::chrono::steady_clock::now();
    t.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count() / a.size());
  }
  std::sort(t.begin(), t.end());
  std::printf("%s %.3f\n", tag, t[3]);
  return t[3];
}
int main() {
  SetThreadAffinityMask(GetCurrentThread(), 1u << 2); SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  auto few = gen::make("few", 1 << 20, 16, 5), perm = gen::make("perm", 1 << 20, 0, 6), nearly = gen::make("nearly", 1 << 20, 0.01, 7);
  const std::vector<u32>* ins[] = {&few, &perm, &nearly}; const char* nm[] = {"few16", "perm", "nearly"};
  for (int i = 0; i < 3; ++i) {
    char b[64];
    std::snprintf(b, sizeof b, "%s nx/polylog", nm[i]); med<Nx, nms::Order::Power, nms::PolylogT>(b, *ins[i]);
    std::snprintf(b, sizeof b, "%s nx/sag", nm[i]); med<Nx, nms::Order::Power, nms::SagT>(b, *ins[i]);
    std::snprintf(b, sizeof b, "%s nx/naive", nm[i]); med<Nx, nms::Order::Power, nms::NaiveT>(b, *ins[i]);
    std::snprintf(b, sizeof b, "%s plain/polylog", nm[i]); med<Plain, nms::Order::Power, nms::PolylogT>(b, *ins[i]);
    std::snprintf(b, sizeof b, "%s plain/sag", nm[i]); med<Plain, nms::Order::Power, nms::SagT>(b, *ins[i]);
    std::snprintf(b, sizeof b, "%s plain/naive", nm[i]); med<Plain, nms::Order::Power, nms::NaiveT>(b, *ins[i]);
  }
}
