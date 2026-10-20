#include <cstdint>
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
struct Nx { bool operator()(std::uint32_t a, std::uint32_t b) const noexcept { return a < b; } };
void probe_poly(std::uint32_t* a, std::size_t n) { nms::natural_merge_sort<nms::Order::Power, nms::PolylogT, false>(a, n, Nx{}, nullptr, 32); }
void probe_sag(std::uint32_t* a, std::size_t n) { nms::natural_merge_sort<nms::Order::Power, nms::SagT, false>(a, n, Nx{}, nullptr, 32); }
void probe_naive(std::uint32_t* a, std::size_t n) { nms::natural_merge_sort<nms::Order::Tim, nms::NaiveT, false>(a, n, Nx{}, nullptr, 32); }
