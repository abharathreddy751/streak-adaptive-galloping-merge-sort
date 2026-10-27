#!/usr/bin/env bash
# Regenerates results/outputs/v4_*.txt: saved outputs of the deterministic nms_tool commands (no --reps, so no timing lines).
#   bash scripts/build.sh && bash scripts/v4_demo_outputs.sh
set -u
cd "$(dirname "$0")/.." || exit 2
if ! command -v g++ >/dev/null 2>&1 && [ -x /c/msys64/usr/bin/g++ ]; then export PATH="/c/msys64/usr/bin:$PATH"; fi
exe=build/nms_tool; [ -x build/nms_tool.exe ] && exe=build/nms_tool.exe
o=results/outputs
run() { local f=$1; shift; { echo "# ./nms_tool $*"; "./$exe" "$@"; echo "# exit=$?"; } 2>&1 | tr -d '\r' > "$o/$f"; }
run v4_trace_default.txt trace
run v4_trace_base_small.txt trace --keys 3,1,2,9,8,7,4,5,6 --policy base --minrun 2
run v4_profile_few3.txt profile --family few --param 3 --n 262144
run v4_compare_few3.txt compare --family few --param 3 --n 262144
run v4_compare_nearly.txt compare --family nearly --param 0.01 --n 100000
run v4_compare_strings_zipf.txt compare --key str --keylen 63 --family zipf --param 1024 --n 100000
echo "wrote $o/v4_*.txt"
