#!/usr/bin/env bash
# Final verification pass (run from project root).  Restores the ORIGINAL results/outputs/final_audit.txt afterwards.
set -u
export PATH="/c/msys64/usr/bin:$PATH" CXX=/c/msys64/usr/bin/g++
orig_sha=$(sha256sum stage2/artifacts/final_audit_ORIGINAL.txt | cut -d' ' -f1)
python -I scripts/final_audit.py > stage2/logs/final_pass_final_audit.txt 2>&1; echo "existing final_audit.py exit=$?"
cp results/outputs/final_audit.txt stage2/artifacts/final_audit_RERUN_final_pass.txt
cp stage2/artifacts/final_audit_ORIGINAL.txt results/outputs/final_audit.txt
[ "$(sha256sum results/outputs/final_audit.txt | cut -d' ' -f1)" = "$orig_sha" ] && echo "original final_audit.txt restored (sha ok)"
tail -1 stage2/artifacts/final_audit_RERUN_final_pass.txt
python -I stage2/audit/independent_audit.py > stage2/logs/final_pass_independent_audit.txt 2>&1; echo "independent_audit exit=$? $(tail -1 stage2/logs/final_pass_independent_audit.txt)"
python -I stage2/audit/audit_string_experiment.py > stage2/logs/final_pass_string_audit.txt 2>&1; echo "string audit exit=$? $(tail -1 stage2/logs/final_pass_string_audit.txt)"
for t in test_base test_all test_sag test_sagt; do ./build/$t.exe > stage2/logs/final_pass_$t.txt 2>&1; echo "$t exit=$? $(tail -1 stage2/logs/final_pass_$t.txt)"; done
./build/demo_sort.exe > stage2/logs/final_pass_demo.txt 2>&1; echo "demo exit=$? $(tail -1 stage2/logs/final_pass_demo.txt)"
