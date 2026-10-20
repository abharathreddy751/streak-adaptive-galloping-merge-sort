// textbook_merge_sort.hpp -- the plain MACSE501 Module-3 Merge Sort (top-down, divide and conquer, one auxiliary
// array).  Used as the syllabus baseline and as an independent cross-check; no runs, no galloping.
//   T(n) = 2 T(n/2) + Theta(n)  =>  Theta(n log n) comparisons and moves, Theta(n) extra space.
#pragma once

#include <cstddef>
#include <vector>

namespace nms {

namespace detail {
template <class T, class Less>
void ms_rec(T* a, T* aux, std::size_t lo, std::size_t hi, Less& less) {  // sorts a[lo, hi)
  if (hi - lo < 2) return;
  const std::size_t mid = lo + (hi - lo) / 2;
  ms_rec(a, aux, lo, mid, less);
  ms_rec(a, aux, mid, hi, less);
  for (std::size_t k = lo; k < hi; ++k) aux[k] = a[k];
  std::size_t i = lo, j = mid;
  for (std::size_t k = lo; k < hi; ++k) {
    if (i == mid) a[k] = aux[j++];
    else if (j == hi) a[k] = aux[i++];
    else if (less(aux[j], aux[i])) a[k] = aux[j++];   // strict: equal keys keep left-run first (stable)
    else a[k] = aux[i++];
  }
}
}  // namespace detail

template <class T, class Less>
void merge_sort_topdown(T* a, std::size_t n, Less less) {
  if (n < 2) return;
  std::vector<T> aux(n);
  detail::ms_rec(a, aux.data(), 0, n, less);
}

}  // namespace nms
