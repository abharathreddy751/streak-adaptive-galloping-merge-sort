#!/usr/bin/env bash
# test_cli.sh -- checks of the terminal tool build/nms_tool: exit codes, argument validation, determinism, required output lines.
#   bash tests/test_cli.sh      (run after scripts/build.sh; exit 0 iff every check passes)
set -u
cd "$(dirname "$0")/.." || exit 2
if ! command -v g++ >/dev/null 2>&1 && [ -x /c/msys64/usr/bin/g++ ]; then export PATH="/c/msys64/usr/bin:$PATH"; fi
exe=build/nms_tool; [ -x build/nms_tool.exe ] && exe=build/nms_tool.exe
[ -x "$exe" ] || { echo "test_cli: $exe missing"; exit 1; }
pass=0; fail=0
ok()  { pass=$((pass+1)); }
bad() { fail=$((fail+1)); echo "FAIL: $1"; }
# expect_exit <code> <description> <args...>
expect_exit() { local want=$1 d=$2; shift 2; "./$exe" "$@" >/dev/null 2>&1; local rc=$?; [ "$rc" -eq "$want" ] && ok || bad "$d (exit $rc, wanted $want)"; }
# expect_line <description> <fixed text> <args...>   (stdout must contain the text)
expect_line() { local d=$1 t=$2; shift 2; "./$exe" "$@" 2>&1 | tr -d '\r' | grep -F -q -- "$t" && ok || bad "$d (missing: $t)"; }

KEYS=5,6,7,8,9,1,1,1,2,2,2,2,3,3,3,9,9,9,1,2,3,4,5,6
expect_exit 0 "help" help
expect_exit 0 "trace default" trace
expect_exit 0 "trace explicit keys, base policy" trace --keys $KEYS --policy base
expect_exit 0 "trace n=1" trace --keys 5
expect_exit 0 "profile" profile --family few --param 3 --n 5000
expect_exit 0 "compare small" compare --family nearly --param 0.02 --n 3000
expect_exit 0 "compare strings" compare --key str --keylen 20 --family zipf --n 3000
expect_exit 0 "strings (small, timed)" strings --n 2000 --reps 2
expect_exit 0 "compare n=1" compare --n 1
expect_exit 0 "compare reversed" compare --family reversed --n 1000
# invalid input: usage errors must exit 2 and say why on stderr
expect_exit 2 "no command"
expect_exit 2 "unknown command" frobnicate
expect_exit 2 "unknown option" compare --nonsense 1
expect_exit 2 "missing value" compare --n
expect_exit 2 "n = 0" compare --n 0
expect_exit 2 "n not a number" compare --n 12x
expect_exit 2 "n too large" compare --n 99999999999
expect_exit 2 "negative n" compare --n -5
expect_exit 2 "unknown family" compare --family bogus
expect_exit 2 "bad key list" trace --keys 1,,2
expect_exit 2 "key too large" trace --keys 4294967296
expect_exit 2 "keys with generator options" profile --keys 1,2,3 --n 10
expect_exit 2 "trace too large" trace --n 129
expect_exit 2 "bad policy" trace --policy fast
expect_exit 2 "bad keylen" compare --key str --keylen 7
expect_exit 2 "bad minrun" compare --minrun 0
expect_exit 2 "bad reps" compare --reps 0
expect_exit 2 "strings with int key" strings --key int
"./$exe" compare --n 0 2>&1 >/dev/null | grep -q "error" && ok || bad "usage error message on stderr"

# required content
expect_line "trace verifies output" "output identical to the reference stable order (keys and ids): YES" trace
expect_line "trace shows run detection" "[run 1] elements 0..4: run detection found 5" trace
expect_line "trace shows merge" "[merge] at 0: left run 5 + right run 13" trace
expect_line "trace reports counters agree" "counters agree with external counter: YES" trace
expect_line "trace total of the stored demo (61)" "= 61 |" trace
expect_line "profile measured block" "measured by the profiler" profile --n 100
expect_line "profile sorted label" "already sorted             : yes" profile --family sorted --n 100
expect_line "profile reverse label" "reverse sorted             : yes" profile --family reversed --n 100
expect_line "compare result line" "RESULT: OK" compare --family few --param 5 --n 4000
expect_line "compare has SAG row" "SAG " compare --family few --param 5 --n 4000
expect_line "strings explains timing" "wall-clock time" strings --n 2000 --reps 2

# determinism: the same command twice gives byte-identical output (no timing in trace/profile/compare without --reps)
for args in "trace" "profile --family zipf --param 100 --n 4000" "compare --family runs --param 9 --n 6000" "compare --key str --family few --param 7 --n 3000"; do
  a=$("./$exe" $args 2>&1); b=$("./$exe" $args 2>&1)
  [ "$a" = "$b" ] && ok || bad "non-deterministic output for: $args"
done
# a different seed changes the data (and so the profile), the same seed does not
a=$("./$exe" profile --family perm --n 3000 --seed 1 | grep "adjacent descents"); b=$("./$exe" profile --family perm --n 3000 --seed 2 | grep "adjacent descents")
[ "$a" != "$b" ] && ok || bad "different seeds gave identical descents"

echo "test_cli: $pass checks passed, $fail failed"
[ "$fail" -eq 0 ]
