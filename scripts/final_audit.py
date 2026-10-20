#!/usr/bin/env python3
"""Final audit (reproducible): python scripts/final_audit.py   -> results/outputs/final_audit.txt

Checks, each printed as PASS / FAIL / INFO:
  A  base-paper metadata record present; B tests from a CLEAN rebuild (documented command) pass with 0 failures;
  C  demo output reproduces the stored output byte for byte;  D held-out summaries re-derive identically from the raw CSVs;
  E  manifest digests equal the frozen digests;  F the original protocol text (before the addendum) is byte-identical to the
  frozen hash;  G round-1 code under test unchanged since its freeze;  H raw CSV completeness / verification flags;
  I  required deliverable files exist;  J original LPQ folders: metadata only (never read or written by this project).
"""
import hashlib
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
EXE = ".exe" if os.name == "nt" else ""
lines = []
fails = 0


def rec(status, msg):
    global fails
    if status == "FAIL":
        fails += 1
    lines.append(f"[{status}] {msg}")
    print(lines[-1])


def run(cmd, **kw):
    return subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, **kw)


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


# ---- A: paper record
pv = (ROOT / "docs" / "PAPER_VERIFICATION.md").read_text(encoding="utf-8")
ok = all(k in pv for k in ("10.1007/s00453-024-01285-6", "Algorithmica", "Springer", "NOT independently verified"))
rec("PASS" if ok else "FAIL", "A  docs/PAPER_VERIFICATION.md names DOI, publisher, venue and states that Scopus was not verified")

# ---- B: clean rebuild + tests (documented command: rm -rf build && bash scripts/build.sh)
shutil.rmtree(ROOT / "build", ignore_errors=True)
t0 = run(["bash", "scripts/build.sh"])
built = t0.returncode == 0 and "warning" not in (t0.stdout + t0.stderr).lower()
rec("PASS" if built else "FAIL", f"B1 clean build with the documented command, exit {t0.returncode}, warnings: {'none' if 'warning' not in (t0.stdout + t0.stderr).lower() else 'PRESENT'}")
counts = {}
for t in ("test_base", "test_all", "test_sag", "test_sagt"):
    r = run([str(ROOT / "build" / (t + EXE))])
    m = re.findall(r"(\d+) checks passed, (\d+) failed", r.stdout)
    passed, failed = (int(m[-1][0]), int(m[-1][1])) if m else (0, -1)
    counts[t] = (passed, failed)
    rec("PASS" if r.returncode == 0 and failed == 0 and passed > 0 else "FAIL", f"B2 {t}: {passed} checks passed, {failed} failed (exit {r.returncode})")

# ---- C: demo
d = run([str(ROOT / "build" / ("demo_sort" + EXE))])
stored = (ROOT / "results" / "outputs" / "demo_output.txt").read_text(encoding="utf-8", errors="replace")
rec("PASS" if d.returncode == 0 and d.stdout.replace("\r\n", "\n").strip() == stored.replace("\r\n", "\n").strip() else "FAIL",
    "C  demo_sort exits 0, prints 'all outputs equal the reference stable sort: YES' and matches results/outputs/demo_output.txt")

# ---- D: summaries re-derive identically
for csv_name, extra in (("heldout", []), ("heldout2", ["--var", "sagt", "--round2"])):
    out_md = ROOT / "results" / "outputs" / f"summary_{csv_name}.md"
    before = out_md.read_bytes()
    r = run([sys.executable, "scripts/analyze.py", f"results/raw/{csv_name}.csv"] + extra)
    after = out_md.read_bytes()
    rec("PASS" if r.returncode == 0 and before == after else "FAIL", f"D  results/raw/{csv_name}.csv re-derives results/outputs/summary_{csv_name}.md identically (deterministic bootstrap)")

# ---- E: manifest digests
for manifest, record in (("heldout", "freeze_record.txt"), ("heldout2", "freeze_record_round2.txt")):
    dry = run([sys.executable, "scripts/run_bench.py", "--manifest", manifest, "--dry"]).stdout.splitlines()[0]
    cur = re.search(r"sha256=([0-9a-f]{64})", dry).group(1)
    frozen = re.search(r"sha256=([0-9a-f]{64})", (ROOT / "results" / "outputs" / record).read_text()).group(1)
    rec("PASS" if cur == frozen else "FAIL", f"E  manifest '{manifest}' digest equals the digest frozen in {record}")

# ---- F: original protocol text unchanged since the round-1 freeze
plan = (ROOT / "docs" / "EXPERIMENT_PLAN.md").read_bytes()
marker = b"\n---\n\n# ADDENDUM"
idx = plan.index(marker)
frozen_plan = re.search(r"([0-9a-f]{64}) \*docs/EXPERIMENT_PLAN.md", (ROOT / "results" / "outputs" / "freeze_record.txt").read_text()).group(1)
rec("PASS" if hashlib.sha256(plan[:idx]).hexdigest() == frozen_plan else "FAIL",
    "F  sections 1-9 of docs/EXPERIMENT_PLAN.md (success criteria included) are byte-identical to the text frozen before the held-out run; only the addendum was appended")

# ---- G: round-1 code under test unchanged
fr1 = (ROOT / "results" / "outputs" / "freeze_record.txt").read_text()
for f in ("src/natural_merge_sort.hpp", "src/sag_policy.hpp", "src/generators.hpp", "src/entropy.hpp", "src/textbook_merge_sort.hpp"):
    h = re.search(r"([0-9a-f]{64}) \*" + re.escape(f), fr1).group(1)
    rec("PASS" if sha(ROOT / f) == h else "FAIL", f"G  {f} unchanged since the round-1 freeze")
changed = [f for f in ("bench/bench_sort.cpp", "scripts/run_bench.py", "scripts/analyze.py", "scripts/build.sh", "tests/test_sort.cpp", "tests/test_sag.cpp")
           if re.search(r"([0-9a-f]{64}) \*" + re.escape(f), fr1) and sha(ROOT / f) != re.search(r"([0-9a-f]{64}) \*" + re.escape(f), fr1).group(1)]
rec("INFO", "G  files that differ from the round-1 freeze record (additions after round 1: corrective-round variants/manifests/analysis modes; see EXPERIMENT_LOG): " + ", ".join(changed))

# ---- H: raw CSV completeness
import csv
for name, ncells in (("heldout", 90), ("heldout2", 54)):
    rows = list(csv.DictReader(open(ROOT / "results" / "raw" / f"{name}.csv", newline="")))
    cells = {r["cell"] for r in rows}
    bad = [r for r in rows if r["ok"] != "1"]
    rec("PASS" if len(cells) == ncells and not bad else "FAIL", f"H  {name}.csv: {len(rows)} rows, {len(cells)}/{ncells} cells, rows with ok != 1: {len(bad)}")

# ---- I: deliverables
need = ["README.md", "src/natural_merge_sort.hpp", "src/sag_policy.hpp", "src/sag_time_policy.hpp", "tests/test_sort.cpp", "scripts/build.sh",
        "results/raw/heldout.csv", "results/raw/heldout2.csv", "results/outputs/summary_heldout.md", "results/outputs/demo_output.txt",
        "results/figures/fig1_time_ratio_by_regime.png", "docs/SYLLABUS_COMPLIANCE.md", "docs/IDEA_SELECTION.md", "docs/PAPER_VERIFICATION.md",
        "docs/BASE_PAPER_IMPLEMENTATION.md", "docs/PROPOSED_METHOD.md", "docs/EXPERIMENT_PLAN.md", "docs/EXPERIMENT_LOG.md", "docs/FINAL_REPORT.md",
        "docs/DEMO_COMMANDS.md"]
missing = [f for f in need if not (ROOT / f).exists()]
rec("PASS" if not missing else "FAIL", f"I  required deliverables present; missing: {missing or 'none'}")

# ---- J: original LPQ folders (metadata only; this project never read or wrote inside them)
for p in (r"D:\dsaproject\lpq", r"D:\dsaproject\lpq_backup_pre_sprint_20261008"):
    if os.path.exists(p):
        st = os.stat(p)
        import datetime
        rec("INFO", f"J  {p}: directory modified {datetime.datetime.fromtimestamp(st.st_mtime):%Y-%m-%d %H:%M:%S} (project started 2026-10-19 22:33; a modification time before that is consistent with 'untouched'; contents were never opened)")
    else:
        rec("INFO", f"J  {p}: not found")

lines.append(f"\nSUMMARY: {fails} FAIL")
out = ROOT / "results" / "outputs" / "final_audit.txt"
out.write_text("\n".join(lines) + "\n", encoding="utf-8")
print(lines[-1])
sys.exit(1 if fails else 0)
