#!/usr/bin/env bash
LOG="/mnt/d/new dsa project_V4_AUDIT/linux"
EXT=/tmp/nms_v4_linux2; rm -rf "$EXT"; mkdir -p "$EXT"
python3 -I -c "import sys,zipfile;zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])" "/mnt/d/new dsa project_SUBMISSION_V4.zip" "$EXT"
cd "$EXT/new dsa project_SUBMISSION_V4" || exit 4
bash scripts/build.sh > "$LOG/06_build2.txt" 2>&1; echo "build exit=$? warnings: $(grep -ci warning "$LOG/06_build2.txt")"
mkdir -p /tmp/pyshim && ln -sf "$(command -v python3)" /tmp/pyshim/python
echo "== asm check on Linux (g++ $(g++ -dumpversion), -O2)"
PATH="/tmp/pyshim:$PATH" bash validation/v4/asm_check/asm_check.sh > "$LOG/03_asm_check.txt" 2>&1; echo "asm_check exit=$?"; tail -4 "$LOG/03_asm_check.txt"
rm -f validation/v4/asm_check/asm_v*_O2.s
echo "== CLI cross-check against the Python model (Linux binary, 300 random inputs each)"
python3 -I "/mnt/d/new dsa project_V4_WORK/scripts/audit_cli_check.py" "$PWD/build/nms_tool" 300 > "$LOG/04_cli_check.txt" 2>&1; echo "cli check exit=$?"; tail -3 "$LOG/04_cli_check.txt"
echo "== saved v4 demo outputs reproduced on Linux?"
for c in "v4_trace_default.txt:trace" "v4_trace_base_small.txt:trace --keys 3,1,2,9,8,7,4,5,6 --policy base --minrun 2" "v4_profile_few3.txt:profile --family few --param 3 --n 262144" "v4_compare_few3.txt:compare --family few --param 3 --n 262144" "v4_compare_nearly.txt:compare --family nearly --param 0.01 --n 100000" "v4_compare_strings_zipf.txt:compare --key str --keylen 63 --family zipf --param 1024 --n 100000"; do
  f=${c%%:*}; a=${c#*:}
  { echo "# ./nms_tool $a"; ./build/nms_tool $a; echo "# exit=$?"; } 2>&1 | tr -d '\r' > /tmp/_cur.txt
  tr -d '\r' < "results/outputs/$f" | diff - /tmp/_cur.txt > /tmp/_d.txt && echo "same   $f" || { echo "DIFFER $f ($(wc -l < /tmp/_d.txt) lines)"; head -4 /tmp/_d.txt | cut -c1-200; }
done
