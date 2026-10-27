#!/usr/bin/env bash
# Is the percentile-bootstrap analysis reproducible across Python versions?  (Windows run: Python 3.11.9; WSL: Python 3.14.4)
python3 --version
W="/mnt/d/new dsa project_V4_WORK"
cd "$W" || exit 2
echo "== audit_recompute.py under $(python3 --version)"
python3 -I scripts/audit_recompute.py > "/mnt/d/new dsa project_V4_AUDIT/audit_recompute_python314_output.txt" 2>&1; echo "exit=$?"
grep -v "^OK" "/mnt/d/new dsa project_V4_AUDIT/audit_recompute_python314_output.txt" | cut -c1-200 | head -20
echo "== first differing digits of bootstrap values (analyze.py on results/raw/heldout.csv, copy of the tree in /tmp)"
rm -rf /tmp/pycheck && mkdir -p /tmp/pycheck && cp -r "$W/scripts" "$W/results" "$W/docs" /tmp/pycheck/ 2>/dev/null
cd /tmp/pycheck && python3 -I scripts/analyze.py results/raw/heldout.csv > /tmp/pycheck/out_314.txt 2>&1; echo "analyze exit=$?"
diff <(tr -d '\r' < results/outputs/summary_heldout.md) <(tr -d '\r' < "$W/results/outputs/summary_heldout.md") > /tmp/pycheck/d.txt; echo "diff lines between Linux-3.14 re-derivation and the stored summary: $(wc -l < /tmp/pycheck/d.txt)"
head -6 /tmp/pycheck/d.txt | cut -c1-260
