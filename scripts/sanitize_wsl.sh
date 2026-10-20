#!/usr/bin/env bash
# AddressSanitizer + UndefinedBehaviorSanitizer run of the full test suite.
# Windows g++ (MinGW) cannot link libasan/libubsan, so this runs inside WSL (Ubuntu g++).
#   From Git-Bash:    MSYS_NO_PATHCONV=1 wsl.exe -e bash "/mnt/d/new dsa project/scripts/sanitize_wsl.sh"
#   From PowerShell:  wsl.exe -e bash "/mnt/d/new dsa project/scripts/sanitize_wsl.sh"
# Sanitizer builds are NOT used for any timing.   (The project path contains a space, hence the quoting.)
set -euo pipefail
SRC="$(cd "$(dirname "$0")/.." && pwd)"
OUT=/tmp/nms_sanitize
mkdir -p "$OUT" "$SRC/results/outputs"
FLAGS=(-std=c++17 -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -Wall -Wextra "-I$SRC/src" "-I$SRC/tests")
CC_VER="$(g++ --version | head -1)"
echo "compiler: $CC_VER"
g++ "${FLAGS[@]}" "$SRC/tests/test_sort.cpp" -o "$OUT/test_base_san"
{ echo "# ASan+UBSan: test_base ($CC_VER)"; "$OUT/test_base_san"; } | tee "$SRC/results/outputs/sanitizer_test_base.txt"
if grep -q "SagTT" "$SRC/src/sag_policy.hpp" 2>/dev/null; then
  g++ "${FLAGS[@]}" -DWITH_SAG "$SRC/tests/test_sort.cpp" -o "$OUT/test_all_san"
  { echo "# ASan+UBSan: test_all ($CC_VER)"; "$OUT/test_all_san"; } | tee "$SRC/results/outputs/sanitizer_test_all.txt"
  g++ "${FLAGS[@]}" "$SRC/tests/test_sag.cpp" -o "$OUT/test_sag_san"
  { echo "# ASan+UBSan: test_sag ($CC_VER)"; "$OUT/test_sag_san"; } | tee "$SRC/results/outputs/sanitizer_test_sag.txt"
fi
if grep -q "SagTimeTT" "$SRC/src/sag_time_policy.hpp" 2>/dev/null; then
  g++ "${FLAGS[@]}" "$SRC/tests/test_sagt.cpp" -o "$OUT/test_sagt_san"
  { echo "# ASan+UBSan: test_sagt ($CC_VER)"; "$OUT/test_sagt_san"; } | tee "$SRC/results/outputs/sanitizer_test_sagt.txt"
fi
echo "sanitizer run finished OK"
