#!/usr/bin/env python3
"""Create SUBMISSION_MANIFEST.sha256 (every file of this folder except the manifest itself) and the ZIP beside the folder.

    python -I scripts/package_submission.py [suffix]   # writes <folder>/SUBMISSION_MANIFEST.sha256, <folder>.zip, <folder>.zip.sha256
Refuses to overwrite an existing ZIP.  The ZIP has ONE root folder (the folder name) and contains exactly the files listed in the manifest
plus the manifest.  Used once, for the submitted package; kept for transparency.
"""
import hashlib, os, sys, zipfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
suffix = sys.argv[1] if len(sys.argv) > 1 else ""   # e.g. _v2 when a ZIP of the plain name already exists
zip_path = root.with_name(root.name + suffix + ".zip")
if zip_path.exists():
    sys.exit(f"refusing to overwrite {zip_path}")
files = sorted(p for p in root.rglob("*") if p.is_file() and p.name != "SUBMISSION_MANIFEST.sha256")
with open(root / "SUBMISSION_MANIFEST.sha256", "w", newline="\n", encoding="utf-8") as m:
    m.write(f"# SHA-256 manifest of every file in {root.name}/ except this manifest itself ({len(files)} files). Format: sha256  size  relative-path\n")
    for p in files:
        data = p.read_bytes()
        m.write(f"{hashlib.sha256(data).hexdigest()}  {len(data):>9}  {p.relative_to(root).as_posix()}\n")
with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
    for p in files + [root / "SUBMISSION_MANIFEST.sha256"]:
        z.write(p, f"{root.name}/{p.relative_to(root).as_posix()}")
h = hashlib.sha256(zip_path.read_bytes()).hexdigest()
Path(str(zip_path) + ".sha256").write_text(f"{h}  {zip_path.name}\n")
print(f"files={len(files) + 1} zip={zip_path} sha256={h}")
