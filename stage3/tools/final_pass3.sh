#!/usr/bin/env bash
# Stage-3 final verification gate.  Run from anywhere:  bash stage3/tools/final_pass3.sh
#
# Submission-package revision (the Stage-3 original is preserved in the original project and in checkpoint CP3):
#   * works from the project root wherever it is extracted (no absolute machine paths; the project path may contain spaces);
#   * log and artifact directories can be redirected so a re-run never overwrites historical evidence:
#         NMS_LOG_DIR=<dir>   (default stage3/logs)      NMS_ART_DIR=<dir>   (default stage3/artifacts)
#   * the "pristine tree" audit uses checkpoint CP2 when NMS_CP2_ZIP points to it, otherwise a temporary copy of this project whose
#     src/natural_merge_sort.hpp is replaced by the ORIGINAL header kept in stage3/artifacts/orig_src;
#   * every audit-output file that older scripts overwrite is backed up and restored (including stage3/audit/audit_stage3_output.txt);
#   * it ends with an explicit GATE line and a non-zero exit status if any expectation is not met.
#
# Expected results, decided by the Stage-3 design (the frozen-header hash checks of the OLD audits fail by design on the live tree because
# src/natural_merge_sort.hpp carries the Stage-3 exception guard):
#   clean build exit 0, no warnings; test_base, test_all, test_sag, test_sagt, test_exception exit 0; demo exit 0 and identical to the stored output;
#   scripts/final_audit.py on the live tree: exactly one FAIL, check G for src/natural_merge_sort.hpp;
#   scripts/final_audit.py on the pristine-header tree: exit 0 (0 FAIL);
#   stage2 audits on the live tree: failures only on natural_merge_sort.hpp hash checks (2 + 2);
#   stage3 independent audit: exit 0.
set -u
cd "$(dirname "$0")/../.." || exit 2
ROOT="$(pwd)"
if ! command -v g++ >/dev/null 2>&1 && [ -x /c/msys64/usr/bin/g++ ]; then export PATH="/c/msys64/usr/bin:$PATH"; fi
export CXX="${CXX:-g++}"
L="${NMS_LOG_DIR:-stage3/logs}"
ART="${NMS_ART_DIR:-stage3/artifacts}"
mkdir -p "$L" "$ART/backup_audit_outputs"
L="$(cd "$L" && pwd)"; ART="$(cd "$ART" && pwd)"   # absolute from here on, so relative and absolute NMS_* values both work
# NOTE: MSYS2 grep 3.0 mis-handles the pattern ^\[FAIL\] (matches nothing), so fixed-string searches are used throughout.
TMP="$(python -I -c "import tempfile;print(tempfile.mkdtemp().replace(chr(92),chr(47)))")"
GATE=0
bad() { echo "GATE-CHECK FAILED: $1"; GATE=1; }

echo "== environment"
{ echo "date: $(date)"; echo "root: $ROOT"; echo "compiler: $($CXX --version | head -1)  target: $($CXX -dumpmachine)"; echo "python: $(python -V 2>&1)"; uname -a; } | tee "$L/final3_environment.txt"

cp results/outputs/final_audit.txt "$ART/backup_audit_outputs/final_audit.txt"
cp stage2/audit/independent_audit_output.txt stage2/audit/independent_audit_output.json stage2/audit/string_audit_output.txt stage3/audit/audit_stage3_output.txt "$ART/backup_audit_outputs/"

echo "== clean build + all tests + demo (live tree, patched header)"
rm -rf build
bash scripts/build.sh > "$L/final3_build.txt" 2>&1; rc=$?; w=$(grep -ci warning "$L/final3_build.txt")
echo "build exit=$rc warnings: $w"; [ "$rc" -eq 0 ] && [ "$w" -eq 0 ] || bad "clean build"
for t in test_base test_all test_sag test_sagt test_exception; do
  if [ -x "./build/$t.exe" ]; then ./build/$t.exe > "$L/final3_$t.txt" 2>&1; rc=$?; else echo "missing build/$t.exe" > "$L/final3_$t.txt"; rc=127; fi
  echo "$t exit=$rc $(tail -1 "$L/final3_$t.txt")"; [ "$rc" -eq 0 ] || bad "$t"
done
./build/demo_sort.exe > "$L/final3_demo.txt" 2>&1; rc=$?
echo "demo exit=$rc $(tail -1 "$L/final3_demo.txt")"; [ "$rc" -eq 0 ] || bad "demo exit"
if diff <(tr -d '\r' < "$L/final3_demo.txt") <(tr -d '\r' < results/outputs/demo_output.txt) > /dev/null; then echo "demo output identical to results/outputs/demo_output.txt"; else bad "demo output differs from the stored output"; fi

echo "== original audit script on the LIVE tree (expected: exactly one FAIL, check G for natural_merge_sort.hpp)"
python -I scripts/final_audit.py > "$L/final3_final_audit_live.txt" 2>&1; rc=$?
cp results/outputs/final_audit.txt "$ART/final_audit_LIVE_stage3.txt"
echo "final_audit.py live exit=$rc"; grep -F -e "[FAIL]" -e "SUMMARY" "$ART/final_audit_LIVE_stage3.txt"
nf=$(grep -c -F "[FAIL]" "$ART/final_audit_LIVE_stage3.txt"); gf=$(grep -c -F "[FAIL] G  src/natural_merge_sort.hpp" "$ART/final_audit_LIVE_stage3.txt")
[ "$nf" -eq 1 ] && [ "$gf" -eq 1 ] || bad "live-tree final_audit.py did not show exactly the expected single FAIL"
cp "$ART/backup_audit_outputs/final_audit.txt" results/outputs/final_audit.txt

echo "== original audit script on a pristine-header tree (state before the Stage-3 header change)"
PT="$TMP/pristine tree"
if [ -n "${NMS_CP2_ZIP:-}" ] && [ -f "$NMS_CP2_ZIP" ]; then
  mkdir -p "$PT" && python -I -c "import sys,zipfile;zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])" "$NMS_CP2_ZIP" "$PT"; echo "(using checkpoint $NMS_CP2_ZIP)"
else
  python -I -c "import shutil,sys;shutil.copytree('.',sys.argv[1],ignore=shutil.ignore_patterns('build'))" "$PT" && cp stage3/artifacts/orig_src/natural_merge_sort.hpp "$PT/src/natural_merge_sort.hpp"
  echo "(using a temporary copy of this project with the ORIGINAL header from stage3/artifacts/orig_src)"
fi
( cd "$PT" && python -I scripts/final_audit.py > "$L/final3_final_audit_pristine_tree.txt" 2>&1; echo "final_audit.py on pristine-header tree exit=$?" > "$TMP/pristine_rc.txt"; cp results/outputs/final_audit.txt "$ART/final_audit_PRISTINE_TREE_stage3.txt" )
cat "$TMP/pristine_rc.txt"; tail -1 "$ART/final_audit_PRISTINE_TREE_stage3.txt"
grep -q "exit=0" "$TMP/pristine_rc.txt" || bad "pristine-header tree final_audit.py"

echo "== Stage-2 audits on the LIVE tree (expected: failures only on natural_merge_sort.hpp hash checks)"
python -I stage2/audit/independent_audit.py > "$L/final3_stage2_independent_audit_live.txt" 2>&1; echo "stage2 independent_audit exit=$? $(tail -1 "$L/final3_stage2_independent_audit_live.txt")"
grep -F "[FAIL]" "$L/final3_stage2_independent_audit_live.txt" | cut -c1-200
python -I stage2/audit/audit_string_experiment.py > "$L/final3_stage2_string_audit_live.txt" 2>&1; echo "stage2 string audit exit=$? $(tail -1 "$L/final3_stage2_string_audit_live.txt")"
grep -F "[FAIL]" "$L/final3_stage2_string_audit_live.txt" | cut -c1-200
a=$(grep -c -F "[FAIL]" "$L/final3_stage2_independent_audit_live.txt"); b=$(grep -c -F "[FAIL]" "$L/final3_stage2_string_audit_live.txt")
[ "$a" -eq 2 ] && [ "$b" -eq 2 ] || bad "stage2 audits: expected exactly 2 + 2 header-hash FAILs, got $a + $b"
cp "$ART/backup_audit_outputs/independent_audit_output.txt" "$ART/backup_audit_outputs/independent_audit_output.json" "$ART/backup_audit_outputs/string_audit_output.txt" stage2/audit/

echo "== Stage-3 independent audit"
python -I stage3/audit/audit_stage3.py > "$L/final3_audit_stage3.txt" 2>&1; rc=$?
cp stage3/audit/audit_stage3_output.txt "$ART/audit_stage3_output_this_run.txt"
cp "$ART/backup_audit_outputs/audit_stage3_output.txt" stage3/audit/audit_stage3_output.txt
echo "audit_stage3 exit=$rc $(tail -1 "$L/final3_audit_stage3.txt")"
grep -F -e "[FAIL]" -e "[DISCREPANCY]" "$L/final3_audit_stage3.txt" | cut -c1-200
[ "$rc" -eq 0 ] || bad "stage3 independent audit"

echo "== restored historical audit outputs"
cmp -s results/outputs/final_audit.txt "$ART/backup_audit_outputs/final_audit.txt" && echo "results/outputs/final_audit.txt restored" || bad "final_audit.txt not restored"
rm -rf "$TMP"
if [ "$GATE" -eq 0 ]; then echo "GATE: PASS"; else echo "GATE: FAIL"; fi
exit "$GATE"
