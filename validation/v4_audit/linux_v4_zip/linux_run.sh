#!/usr/bin/env bash
# v4 audit, Linux (WSL Ubuntu, g++ 15.2): fresh extraction of the v4 ZIP, supported test script, sanitizer builds of every test program and of nms_tool.
# Nothing is installed; the only helper is a `python` -> python3 symlink in a temporary directory for the gate script (which calls `python`).
LOG="/mnt/d/new dsa project_V4_AUDIT/linux"
mkdir -p "$LOG"
ZIP="/mnt/d/new dsa project_SUBMISSION_V4.zip"
EXT=/tmp/nms_v4_linux
rm -rf "$EXT"; mkdir -p "$EXT"
echo "zip sha256: $(sha256sum "$ZIP" | cut -d' ' -f1)" | tee "$LOG/00_env.txt"
{ g++ --version | head -1; python3 --version; uname -a; } | tee -a "$LOG/00_env.txt"
python3 -I -c "import sys,zipfile;zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])" "$ZIP" "$EXT"
cd "$EXT/new dsa project_SUBMISSION_V4" || exit 4
echo "extracted to: $(pwd)" | tee -a "$LOG/00_env.txt"
echo "== run_all_tests.sh (Linux)"
bash scripts/run_all_tests.sh > "$LOG/01_run_all_tests.txt" 2>&1; echo "run_all_tests exit=$?" | tee -a "$LOG/01_run_all_tests.txt"
tail -14 "$LOG/01_run_all_tests.txt"
echo "== sanitizer builds (ASan + UBSan, -O1 -g, trap on UB)"
SAN=/tmp/nms_v4_san; mkdir -p "$SAN"
FL=(-std=c++17 -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -Wall -Wextra -Isrc -Itests)
rc_all=0
build_run() { # name, sources/flags...
  local n=$1; shift
  if ! g++ "${FL[@]}" "$@" -o "$SAN/$n" > "$LOG/san_build_$n.txt" 2>&1; then echo "san build FAILED: $n"; rc_all=1; return; fi
  "$SAN/$n" > "$LOG/san_run_$n.txt" 2>&1; local rc=$?
  echo "san $n exit=$rc :: $(tail -1 "$LOG/san_run_$n.txt")"; [ $rc -eq 0 ] || rc_all=1
  [ -s "$LOG/san_build_$n.txt" ] && echo "   build warnings/output: $(wc -l < "$LOG/san_build_$n.txt") lines"
}
build_run test_base tests/test_sort.cpp
build_run test_all -DWITH_SAG tests/test_sort.cpp
build_run test_sag tests/test_sag.cpp
build_run test_sagt tests/test_sagt.cpp
build_run test_exception tests/test_exception.cpp
build_run test_diag tests/test_diag.cpp
g++ "${FL[@]}" bench/nms_tool.cpp -o "$SAN/nms_tool" > "$LOG/san_build_nms_tool.txt" 2>&1 || { echo "san build FAILED: nms_tool"; rc_all=1; }
for a in "trace" "trace --keys 3,1,2,9,8,7,4,5,6 --policy base --minrun 2" "profile --family few --param 3 --n 20000" "compare --family few --param 3 --n 20000" "compare --family nearly --param 0.01 --n 20000" "compare --key str --family zipf --param 100 --n 5000" "strings --n 5000 --reps 1" "compare --n 1" "compare --keys 7,7,7,7,7"; do
  "$SAN/nms_tool" $a > "$LOG/san_nms_tool_$(echo "$a" | tr ' ,/' '___' | cut -c1-40).txt" 2>&1; rc=$?
  echo "san nms_tool $a  exit=$rc"; [ $rc -eq 0 ] || rc_all=1
done
echo "SANITIZER SUMMARY: $([ $rc_all -eq 0 ] && echo ALL PASS || echo FAILURES)" | tee "$LOG/02_sanitizer_summary.txt"
echo "== asm check on Linux (v3 vs v4 header, -O2)"
bash validation/v4/asm_check/asm_check.sh > "$LOG/03_asm_check.txt" 2>&1; echo "asm_check exit=$?"; tail -3 "$LOG/03_asm_check.txt"
echo "== CLI cross-check against the Python model on Linux"
python3 -I scripts/audit_cli_check.py "$PWD/build/nms_tool" 150 > "$LOG/04_cli_check.txt" 2>&1; echo "cli check exit=$?"; tail -3 "$LOG/04_cli_check.txt"
echo "== Stage 3 gate on Linux (python shim in a temp dir)"
mkdir -p /tmp/pyshim && ln -sf "$(command -v python3)" /tmp/pyshim/python
PATH="/tmp/pyshim:$PATH" NMS_LOG_DIR="$LOG/gate_logs" NMS_ART_DIR="$LOG/gate_artifacts" bash stage3/tools/final_pass3.sh > "$LOG/05_gate_console.txt" 2>&1; echo "gate exit=$?" | tee -a "$LOG/05_gate_console.txt"
grep -E "^GATE|GATE-CHECK FAILED|exit=" "$LOG/05_gate_console.txt" | cut -c1-170
echo "LINUX RUN FINISHED"
