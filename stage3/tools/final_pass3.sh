#!/usr/bin/env bash
# Stage-3 final verification pass (run from the project root).  Backs up and restores every audit-output file that older audit scripts overwrite.
set -u
export PATH="/c/msys64/usr/bin:$PATH" CXX=/c/msys64/usr/bin/g++
L=stage3/logs
SP="C:/Users/VENKAT~1/AppData/Local/Temp/claude/D--new-dsa-project/11861b8b-a626-46d7-b89e-8a1d47605250/scratchpad"
mkdir -p stage3/artifacts/backup_audit_outputs
cp results/outputs/final_audit.txt stage3/artifacts/backup_audit_outputs/final_audit.txt
cp stage2/audit/independent_audit_output.txt stage2/audit/independent_audit_output.json stage2/audit/string_audit_output.txt stage3/artifacts/backup_audit_outputs/

echo "== clean build + all tests + demo (live tree, patched header)"
rm -rf build && bash scripts/build.sh > $L/final3_build.txt 2>&1; echo "build exit=$? warnings: $(grep -ci warning $L/final3_build.txt)"
for t in test_base test_all test_sag test_sagt test_exception; do ./build/$t.exe > $L/final3_$t.txt 2>&1; echo "$t exit=$? $(tail -1 $L/final3_$t.txt)"; done
./build/demo_sort.exe > $L/final3_demo.txt 2>&1; echo "demo exit=$? $(tail -1 $L/final3_demo.txt)"
if diff <(tr -d '\r' < $L/final3_demo.txt) <(tr -d '\r' < results/outputs/demo_output.txt) > /dev/null; then echo "demo output identical to results/outputs/demo_output.txt"; else echo "DEMO OUTPUT DIFFERS"; fi

echo "== original audit scripts on the LIVE tree (expected: check G fails for natural_merge_sort.hpp only)"
python -I scripts/final_audit.py > $L/final3_final_audit_live.txt 2>&1; echo "final_audit.py live exit=$?"
cp results/outputs/final_audit.txt stage3/artifacts/final_audit_LIVE_stage3.txt
grep -E "^\[FAIL\]|SUMMARY" stage3/artifacts/final_audit_LIVE_stage3.txt
cp stage3/artifacts/backup_audit_outputs/final_audit.txt results/outputs/final_audit.txt

echo "== original audit on the pristine CP2 tree (the state before Stage 3)"
rm -rf "$SP/cp2tree" && mkdir -p "$SP/cp2tree"
python -I -c "import zipfile,glob;zipfile.ZipFile(glob.glob('D:/new dsa project_CHECKPOINTS/CP2_pre_stage3_*.zip')[0]).extractall('$SP/cp2tree')"
( cd "$SP/cp2tree" && python -I scripts/final_audit.py > "/d/new dsa project/$L/final3_final_audit_cp2tree.txt" 2>&1; echo "final_audit.py on CP2 tree exit=$?"; tail -1 results/outputs/final_audit.txt; cp results/outputs/final_audit.txt "/d/new dsa project/stage3/artifacts/final_audit_CP2TREE_stage3.txt" )

echo "== Stage-2 independent audits on the LIVE tree (expected: failures only for the Stage-3-changed natural_merge_sort.hpp)"
python -I stage2/audit/independent_audit.py > $L/final3_stage2_independent_audit_live.txt 2>&1; echo "stage2 independent_audit exit=$? $(tail -1 $L/final3_stage2_independent_audit_live.txt)"
grep -E "^\[FAIL\]" $L/final3_stage2_independent_audit_live.txt | cut -c1-220
python -I stage2/audit/audit_string_experiment.py > $L/final3_stage2_string_audit_live.txt 2>&1; echo "stage2 string audit exit=$? $(tail -1 $L/final3_stage2_string_audit_live.txt)"
grep -E "^\[FAIL\]" $L/final3_stage2_string_audit_live.txt | cut -c1-220
cp stage3/artifacts/backup_audit_outputs/independent_audit_output.txt stage3/artifacts/backup_audit_outputs/independent_audit_output.json stage3/artifacts/backup_audit_outputs/string_audit_output.txt stage2/audit/
echo "== restored original audit outputs; sha check:"
sha256sum results/outputs/final_audit.txt stage3/artifacts/backup_audit_outputs/final_audit.txt | awk '{print $1}' | uniq | wc -l
echo "== Stage-3 independent audit"
python -I stage3/audit/audit_stage3.py > $L/final3_audit_stage3.txt 2>&1; echo "audit_stage3 exit=$? $(tail -1 $L/final3_audit_stage3.txt)"
grep -E "^\[(FAIL|DISCREPANCY)\]" $L/final3_audit_stage3.txt | cut -c1-220
