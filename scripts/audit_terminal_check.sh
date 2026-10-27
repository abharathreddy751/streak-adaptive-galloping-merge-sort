#!/usr/bin/env bash
# v4 code audit: run every documented terminal example plus boundary cases; print  verdict | exit code | command.  Exit status = number of problems.
#   bash scripts/audit_terminal_check.sh      (after bash scripts/build.sh; on Windows with the MSYS2 compiler the script puts /c/msys64/usr/bin on PATH itself)
cd "$(dirname "$0")/.." || exit 2
A="."   # temp files in the project root (mktemp paths differ between Git-Bash and MSYS2 tools)
[ -d /c/msys64/usr/bin ] && export PATH="/c/msys64/usr/bin:$PATH"
exe=./build/nms_tool.exe; [ -x "$exe" ] || exe=./build/nms_tool
bad=0
t() {  # t <expected exit> <expected text or -> <args...>
  local want=$1 txt=$2; shift 2
  local out rc
  out="$("$exe" "$@" 2>&1)"; rc=$?
  local v=OK
  [ "$rc" -eq "$want" ] || v="BAD-EXIT"
  if [ "$txt" != "-" ] && ! printf '%s\n' "$out" | tr -d '\r' | grep -F -q -- "$txt"; then v="BAD-TEXT"; fi
  [ "$v" = OK ] || bad=$((bad+1))
  printf '%-9s exit=%s want=%s | nms_tool %s\n' "$v" "$rc" "$want" "$*"
}
echo "== commands of docs/TERMINAL_DEMO.md section 3"
t 0 "RESULT: OK" trace
t 0 "RESULT: OK" trace --keys 3,1,2,9,8,7,4,5,6 --policy base --minrun 2
t 0 "measured by the profiler" profile --family few --param 3 --n 262144
t 0 "RESULT: OK" compare --family few --param 3 --n 262144
t 0 "wall-clock time" compare --family nearly --param 0.01 --n 100000 --reps 5
t 0 "string-key comparison" strings
t 0 "RESULT: OK" compare --key str --keylen 63 --family zipf --param 1024 --n 100000
t 0 "nms_tool -- terminal demonstration" help
echo "== boundary and special inputs"
t 0 "RESULT: OK" trace --keys 5
t 0 "RESULT: OK" trace --keys 5,5
t 0 "RESULT: OK" trace --keys 2,1
t 0 "RESULT: OK" trace --keys 7,7,7,7,7,7,7
t 0 "RESULT: OK" trace --keys 1,2,3,4,5,6,7,8,9 --minrun 2
t 0 "RESULT: OK" trace --keys 9,8,7,6,5,4,3,2,1 --minrun 1
t 0 "RESULT: OK" trace --keys 4,4,1,1,3,3,2,2,5 --minrun 1
t 0 "RESULT: OK" compare --n 1
t 0 "RESULT: OK" compare --n 2
t 0 "RESULT: OK" compare --n 3 --family reversed
t 0 "RESULT: OK" compare --n 997 --family perm
t 0 "RESULT: OK" compare --n 1001 --family few --param 2
t 0 "RESULT: OK" compare --family sorted --n 1000
t 0 "RESULT: OK" compare --family reversed --n 1000
t 0 "RESULT: OK" compare --family pipe --n 1001
t 0 "RESULT: OK" compare --family mixture --n 5000
t 0 "RESULT: OK" compare --family runs --param 7 --n 5003
t 0 "RESULT: OK" compare --key str --keys 5,5,5,1,1,9,9,9,9
t 0 "RESULT: OK" compare --keys 4294967295,0,4294967295,0 --minrun 1
t 0 "already sorted             : yes" profile --keys 3,3,3,3
t 0 "reverse sorted             : yes" profile --keys 9,7,7,1
t 0 "distinct keys              : 1 " profile --keys 6,6,6
t 0 "elements n                 : 1" profile --keys 42
t 0 "RESULT: OK" trace --n 128 --family perm
t 2 "limited to n <= 128" trace --n 129
echo "== invalid input (exit 2, message on stderr)"
t 2 "missing command"
t 2 "unknown command" nonsense
t 2 "unknown option" compare --bogus
t 2 "needs a value" compare --n
t 2 "--n must be" compare --n 0
t 2 "--n must be" compare --n -1
t 2 "--n must be" compare --n 20000001
t 2 "--n must be" compare --n 1e3
t 2 "unknown --family" compare --family x
t 2 "--param must be" compare --param abc
t 2 "--seed must be" compare --seed -4
t 2 "--keys must be" trace --keys ""
t 2 "--keys must be" trace --keys 1,2,
t 2 "--keys must be" trace --keys 1,a
t 2 "cannot be combined" trace --keys 1,2 --family perm
t 2 "--keylen must be" compare --key str --keylen 64
t 2 "--key must be" compare --key float
t 2 "--policy must be" trace --policy x
t 2 "--minrun must be" compare --minrun 1025
t 2 "--reps must be" compare --reps 1001
t 2 "needs --key str" strings --key int
echo "== determinism and seeds"
a=$("$exe" compare --family zipf --param 100 --n 20000 | tr -d '\r'); b=$("$exe" compare --family zipf --param 100 --n 20000 | tr -d '\r')
[ "$a" = "$b" ] && echo "OK        compare output is byte-identical on repeat" || { echo "BAD       compare output differs on repeat"; bad=$((bad+1)); }
a=$("$exe" trace --keys 3,1,2,9,8,7,4,5,6 | tr -d '\r'); b=$("$exe" trace --keys 3,1,2,9,8,7,4,5,6 | tr -d '\r')
[ "$a" = "$b" ] && echo "OK        trace output is byte-identical on repeat" || { echo "BAD       trace differs"; bad=$((bad+1)); }
echo "== stdout/stderr split for a usage error"
"$exe" compare --n 0 >"$A/_audit_o.txt" 2>"$A/_audit_e.txt"; echo "stdout bytes=$(wc -c <"$A/_audit_o.txt") stderr: $(head -1 "$A/_audit_e.txt")"; rm -f "$A/_audit_o.txt" "$A/_audit_e.txt"
rm -f "$A/_audit_o.txt" "$A/_audit_e.txt"
echo "TERMINAL AUDIT: $([ $bad -eq 0 ] && echo ALL OK || echo "$bad PROBLEMS")"
exit $bad
