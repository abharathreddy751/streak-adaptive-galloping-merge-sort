#!/usr/bin/env bash
# Hardened (-O1, checked containers, UBSan trap-on-error without libubsan, -ftrapv) and -O3 -march=native fuzz runs. Run from project root.
set -u
export PATH="/c/msys64/usr/bin:$PATH"
g++ -std=c++17 -O1 -g -Wall -Wextra -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -fsanitize=undefined -fsanitize-undefined-trap-on-error -ftrapv -Isrc stage2/redteam/redteam_fuzz.cpp -o stage2/redteam/redteam_fuzz_hardened.exe > stage2/logs/redteam_hardened_build.txt 2>&1
echo "build_exit=$?" >> stage2/logs/redteam_hardened_build.txt
./stage2/redteam/redteam_fuzz_hardened.exe 20270101 > stage2/logs/redteam_hardened_seed20270101.txt 2>&1; echo "exit=$?" >> stage2/logs/redteam_hardened_seed20270101.txt
g++ -std=c++17 -O3 -march=native -DNDEBUG -Isrc stage2/redteam/redteam_fuzz.cpp -o stage2/redteam/redteam_fuzz_O3.exe > stage2/logs/redteam_O3_build.txt 2>&1
./stage2/redteam/redteam_fuzz_O3.exe 31415926 > stage2/logs/redteam_O3_native_seed31415926.txt 2>&1; echo "exit=$?" >> stage2/logs/redteam_O3_native_seed31415926.txt
