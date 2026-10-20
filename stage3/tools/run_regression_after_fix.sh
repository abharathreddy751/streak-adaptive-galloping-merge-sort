#!/usr/bin/env bash
# Re-run the Stage-2 red-team fuzz and hardened builds against the PATCHED header (run from project root).
export PATH="/c/msys64/usr/bin:$PATH"
H="-std=c++17 -O1 -g -Wall -Wextra -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -fsanitize=undefined -fsanitize-undefined-trap-on-error -ftrapv -Isrc -Itests"
mkdir -p stage3/artifacts/hardened
g++ -std=c++17 -O2 -Wall -Wextra -Isrc stage2/redteam/redteam_fuzz.cpp -o stage3/artifacts/redteam_O2.exe && ./stage3/artifacts/redteam_O2.exe 20281020 > stage3/logs/redteam_after_fix_O2_seed20281020.txt 2>&1; echo "exit=$?" >> stage3/logs/redteam_after_fix_O2_seed20281020.txt
g++ $H stage2/redteam/redteam_fuzz.cpp -o stage3/artifacts/hardened/redteam_h.exe && ./stage3/artifacts/hardened/redteam_h.exe 20291020 > stage3/logs/redteam_after_fix_hardened_seed20291020.txt 2>&1; echo "exit=$?" >> stage3/logs/redteam_after_fix_hardened_seed20291020.txt
g++ $H tests/test_exception.cpp -o stage3/artifacts/hardened/test_exception_h.exe && ./stage3/artifacts/hardened/test_exception_h.exe > stage3/logs/hardened_test_exception.txt 2>&1; echo "exit=$?" >> stage3/logs/hardened_test_exception.txt
for spec in "test_base:tests/test_sort.cpp:" "test_all:tests/test_sort.cpp:-DWITH_SAG" "test_sag:tests/test_sag.cpp:" "test_sagt:tests/test_sagt.cpp:"; do
  IFS=: read n src def <<< "$spec"
  g++ $H $def $src -o stage3/artifacts/hardened/${n}_h.exe && ./stage3/artifacts/hardened/${n}_h.exe > stage3/logs/hardened_after_fix_$n.txt 2>&1; echo "exit=$?" >> stage3/logs/hardened_after_fix_$n.txt
done
echo done > stage3/logs/regression_after_fix_done.txt
