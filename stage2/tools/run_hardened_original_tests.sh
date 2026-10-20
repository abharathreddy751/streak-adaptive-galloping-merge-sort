#!/usr/bin/env bash
# Substitute for the ASan/UBSan runs that this machine cannot do (no libasan/libubsan, no WSL compiler):
# the ORIGINAL four test programs built with checked containers + UBSan trap-on-error (no runtime library needed) + -ftrapv.
set -u
export PATH="/c/msys64/usr/bin:$PATH"
F="-std=c++17 -O1 -g -Wall -Wextra -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -fsanitize=undefined -fsanitize-undefined-trap-on-error -ftrapv -Isrc -Itests"
mkdir -p stage2/artifacts/hardened
g++ $F tests/test_sort.cpp -o stage2/artifacts/hardened/test_base_h.exe
g++ $F -DWITH_SAG tests/test_sort.cpp -o stage2/artifacts/hardened/test_all_h.exe
g++ $F tests/test_sag.cpp -o stage2/artifacts/hardened/test_sag_h.exe
g++ $F tests/test_sagt.cpp -o stage2/artifacts/hardened/test_sagt_h.exe
for t in test_base test_all test_sag test_sagt; do
  ./stage2/artifacts/hardened/${t}_h.exe > stage2/logs/hardened_${t}.txt 2>&1; echo "exit=$?" >> stage2/logs/hardened_${t}.txt
done
echo ALLDONE > stage2/logs/hardened_tests_done.txt
