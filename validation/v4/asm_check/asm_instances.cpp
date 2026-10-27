// Explicit instantiations of the UNCOUNTED (Count=false, timed) sorters for the record types used by the benchmarks and nms_tool.
// Compiled once against the v3 header and once against the v4 header; the generated assembly must be identical (see asm_check.sh).
#include <cstdint>
#include "natural_merge_sort.hpp"
#include "sag_policy.hpp"
struct IntRec { std::uint32_t key, id; };
struct IntLess { bool operator()(const IntRec& a, const IntRec& b) const noexcept { return a.key < b.key; } };
struct StrRec { char key[64]; std::uint32_t id; };
struct StrLess { bool operator()(const StrRec& a, const StrRec& b) const noexcept { return __builtin_memcmp(a.key, b.key, 64) < 0; } };
struct ThrowLess { bool operator()(const IntRec& a, const IntRec& b) const { return a.key < b.key; } };  // may-throw comparator: GapGuard armed
#define INST(T, L, ORD, TP) template void nms::natural_merge_sort<nms::Order::ORD, TP, false, T, L>(T*, std::size_t, L, nms::Stats*, std::size_t);
#define ALLTP(T, L, ORD) INST(T, L, ORD, nms::NaiveT) INST(T, L, ORD, nms::FixedT<8>) INST(T, L, ORD, nms::PolylogT) INST(T, L, ORD, nms::SagT)
ALLTP(IntRec, IntLess, Power) ALLTP(IntRec, IntLess, Tim) ALLTP(StrRec, StrLess, Power) ALLTP(StrRec, StrLess, Tim) ALLTP(IntRec, ThrowLess, Power)
