// Differential check: same program built against the ORIGINAL and the patched header must print identical lines.
#include <cstdint>
#include <cstdio>
#include <vector>
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
#include "generators.hpp"
using u32 = std::uint32_t;
struct Plain { bool operator()(u32 a, u32 b) const { return a < b; } };                // not declared noexcept
struct Nx { bool operator()(u32 a, u32 b) const noexcept { return a < b; } };          // noexcept -> guard compiled out
template <class L, nms::Order O, class P>
static void run(const char* tag, const std::vector<u32>& in, size_t mr) {
  std::vector<u32> a = in; nms::Stats s;
  nms::natural_merge_sort<O, P, true>(a.data(), a.size(), L{}, &s, mr);
  std::uint64_t h = 1469598103934665603ull;
  for (u32 x : a) { h ^= x; h *= 1099511628211ull; }
  std::printf("%s n=%zu mr=%zu cmp=%llu run=%llu merges=%llu blocks=%llu moves=%llu buf=%zu hash=%016llx\n", tag, in.size(), mr, (unsigned long long)s.comparisons(),
              (unsigned long long)s.run_comparisons, (unsigned long long)s.merges, (unsigned long long)s.blocks, (unsigned long long)s.element_moves, s.max_buffer, (unsigned long long)h);
}
int main() {
  const char* fams[] = {"perm", "few", "zipf", "runs", "mixture", "nearly"};
  const double pars[] = {0, 5, 64, 20, 16, 0.02};
  for (int f = 0; f < 6; ++f) for (size_t n : {100u, 5000u, 200000u}) {
    auto in = gen::make(fams[f], n, pars[f], 777 + f);
    for (size_t mr : {1u, 32u}) {
      run<Plain, nms::Order::Power, nms::PolylogT>("plain/power/polylog", in, mr);
      run<Nx, nms::Order::Power, nms::PolylogT>("nx/power/polylog", in, mr);
      run<Plain, nms::Order::Tim, nms::NaiveT>("plain/tim/naive", in, mr);
      run<Nx, nms::Order::Power, nms::SagT>("nx/power/sag", in, mr);
      run<Plain, nms::Order::Tim, nms::SagT>("plain/tim/sag", in, mr);
      run<Nx, nms::Order::Power, nms::FixedT<0>>("nx/power/fixed0", in, mr);
    }
  }
}
