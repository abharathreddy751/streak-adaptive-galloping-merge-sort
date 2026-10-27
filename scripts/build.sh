#!/usr/bin/env bash
# Build script (no CMake needed).  Works in Git-Bash/MSYS2 on Windows and in Linux/WSL.
#   scripts/build.sh            build everything that exists
#   scripts/build.sh tests      only the test executables
#   scripts/build.sh bench      only the benchmark executable
# Requires g++ with C++17 (tested: g++ 14.2.0 MinGW-w64).  Optimised builds only; sanitizer builds are in
# scripts/sanitize_wsl.sh.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
CXX="${CXX:-g++}"
EXT=""
case "$(uname -s)" in MINGW*|MSYS*|CYGWIN*) EXT=".exe" ;; esac
WARN="-Wall -Wextra"
INC="-Isrc -Itests"
TARGET="${1:-all}"

if [ "$TARGET" = "all" ] || [ "$TARGET" = "tests" ]; then
  echo "[build] test_base (base paper implementation only)"
  $CXX -std=c++17 -O2 $WARN $INC tests/test_sort.cpp -o "build/test_base$EXT"
  if grep -q "SagTT" src/sag_policy.hpp 2>/dev/null; then
    echo "[build] test_all (base + proposed extension)"
    $CXX -std=c++17 -O2 $WARN $INC -DWITH_SAG tests/test_sort.cpp -o "build/test_all$EXT"
    $CXX -std=c++17 -O2 $WARN $INC tests/test_sag.cpp -o "build/test_sag$EXT"
  fi
  if [ -s tests/test_exception.cpp ]; then
    echo "[build] test_exception (stage 3: comparator-exception contract)"
    $CXX -std=c++17 -O2 $WARN $INC tests/test_exception.cpp -o "build/test_exception$EXT"
  fi
  if [ -s tests/test_diag.cpp ]; then
    echo "[build] test_diag (v4: terminal diagnostics, trace consistency, profiler)"
    $CXX -std=c++17 -O2 $WARN $INC tests/test_diag.cpp -o "build/test_diag$EXT"
  fi
  if [ -s src/sag_time_policy.hpp ] && grep -q "SagTimeTT" src/sag_time_policy.hpp; then
    echo "[build] test_sagt (corrective-round extension SAG-T)"
    $CXX -std=c++17 -O2 $WARN $INC tests/test_sagt.cpp -o "build/test_sagt$EXT"
  fi
fi

if [ "$TARGET" = "all" ] || [ "$TARGET" = "bench" ]; then
  if [ -s bench/bench_sort.cpp ] && grep -q "int main" bench/bench_sort.cpp; then
    echo "[build] bench_sort (optimised: -O2 -DNDEBUG)"
    $CXX -std=c++17 -O2 -DNDEBUG $WARN $INC bench/bench_sort.cpp -o "build/bench_sort$EXT"
  fi
fi
if [ "$TARGET" = "all" ] || [ "$TARGET" = "demo" ]; then
  if [ -s bench/demo_sort.cpp ]; then
    echo "[build] demo_sort"
    $CXX -std=c++17 -O2 -DNDEBUG $WARN $INC bench/demo_sort.cpp -o "build/demo_sort$EXT"
  fi
fi
if [ "$TARGET" = "all" ] || [ "$TARGET" = "demo" ]; then
  if [ -s bench/nms_tool.cpp ]; then
    echo "[build] nms_tool (v4 terminal tool)"
    $CXX -std=c++17 -O2 -DNDEBUG '-DNMS_BUILD_FLAGS="-std=c++17 -O2 -DNDEBUG"' $WARN $INC bench/nms_tool.cpp -o "build/nms_tool$EXT"
  fi
fi
echo "[build] done"
