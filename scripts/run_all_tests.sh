#!/usr/bin/env bash
# Supported quick entry point: clean build from source, run every test program and the demo, exit non-zero on ANY failure.
#   bash scripts/run_all_tests.sh
# Needs g++ with C++17 on PATH (or CXX set). On Windows run it from Git-Bash/MSYS2; if g++ is the MSYS2 one it is found automatically.
# The project folder may contain spaces. Nothing outside the project folder is read or written (except the compiler).
set -u
cd "$(dirname "$0")/.." || exit 2
if ! command -v "${CXX:-g++}" >/dev/null 2>&1 && [ -x /c/msys64/usr/bin/g++ ]; then export PATH="/c/msys64/usr/bin:$PATH"; fi
export CXX="${CXX:-g++}"
echo "compiler: $($CXX --version | head -1)"
rm -rf build                      # never trust stale executables
if ! bash scripts/build.sh; then echo "RESULT: FAIL (build)"; exit 1; fi
fail=0
for t in test_base test_all test_sag test_sagt test_exception test_diag; do
  if [ ! -x "build/$t.exe" ] && [ ! -x "build/$t" ]; then echo "$t: executable missing"; fail=1; continue; fi
  exe="build/$t"; [ -x "build/$t.exe" ] && exe="build/$t.exe"
  out="$("./$exe" 2>&1)"; rc=$?
  echo "$t exit=$rc :: $(echo "$out" | tail -1)"
  [ "$rc" -eq 0 ] || fail=1
done
exe=build/demo_sort; [ -x build/demo_sort.exe ] && exe=build/demo_sort.exe
out="$("./$exe" 2>&1)"; rc=$?
echo "demo_sort exit=$rc :: $(echo "$out" | tail -1)"
[ "$rc" -eq 0 ] || fail=1
if diff <(echo "$out" | tr -d '\r') <(tr -d '\r' < results/outputs/demo_output.txt) >/dev/null; then echo "demo output identical to results/outputs/demo_output.txt"; else echo "demo output DIFFERS from results/outputs/demo_output.txt"; fail=1; fi
out="$(bash tests/test_cli.sh 2>&1)"; rc=$?
echo "test_cli exit=$rc :: $(echo "$out" | tail -1)"
[ "$rc" -eq 0 ] || { echo "$out" | head -20; fail=1; }
if [ "$fail" -eq 0 ]; then echo "RESULT: PASS"; else echo "RESULT: FAIL"; fi
exit "$fail"
