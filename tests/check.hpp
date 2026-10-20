// check.hpp -- minimal self-contained test harness (no external dependencies).
// Every CHECK is counted; the first failures are printed with file:line; report() returns a process exit code.
#pragma once
#include <cstdio>

namespace chk {
inline long passed = 0;
inline long failed = 0;
}  // namespace chk

#define CHECK(cond)                                                                   \
  do {                                                                                \
    if (cond) ++chk::passed;                                                          \
    else { ++chk::failed; if (chk::failed <= 30) std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); } \
  } while (0)

// CHECKF(cond, "format", args...) prints extra context on failure.
#define CHECKF(cond, ...)                                                             \
  do {                                                                                \
    if (cond) ++chk::passed;                                                          \
    else {                                                                            \
      ++chk::failed;                                                                  \
      if (chk::failed <= 30) { std::printf("FAIL %s:%d  %s  [", __FILE__, __LINE__, #cond); std::printf(__VA_ARGS__); std::printf("]\n"); } \
    }                                                                                 \
  } while (0)

inline int chk_report(const char* name) {
  std::printf("%s: %ld checks passed, %ld failed\n", name, chk::passed, chk::failed);
  return chk::failed == 0 ? 0 : 1;
}
