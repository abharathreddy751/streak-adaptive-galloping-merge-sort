"""Make a verifiable checkpoint: zip + SHA-256 manifest + read-back verification.
usage: checkpoint.py LABEL   (snapshots D:/new dsa project into D:/new dsa project_CHECKPOINTS/)
"""
import hashlib, os, sys, time, zipfile, json

ROOT = 'D:/new dsa project'
OUT = 'D:/new dsa project_CHECKPOINTS'
label = sys.argv[1]
ts = time.strftime('%Y%m%d_%H%M%S')
os.makedirs(OUT, exist_ok=True)

def sha(b):
    return hashlib.sha256(b).hexdigest()

entries = []
for d, _, fs in os.walk(ROOT):
    for f in sorted(fs):
        p = os.path.join(d, f)
        rel = os.path.relpath(p, ROOT).replace(chr(92), '/')
        data = open(p, 'rb').read()
        entries.append((rel, len(data), sha(data)))
entries.sort()

zpath = f'{OUT}/{label}_{ts}.zip'
with zipfile.ZipFile(zpath, 'w', zipfile.ZIP_DEFLATED) as z:
    for rel, _, _ in entries:
        z.write(f'{ROOT}/{rel}', rel)
mpath = f'{OUT}/{label}_{ts}.manifest.sha256'
with open(mpath, 'w', newline='\n') as m:
    for rel, n, h in entries:
        m.write(f'{h}  {n:>9}  {rel}\n')

# verify: read zip back, compare to manifest AND to live files
bad = 0
with zipfile.ZipFile(zpath) as z:
    assert z.testzip() is None
    names = set(z.namelist())
    for rel, n, h in entries:
        if rel not in names or sha(z.read(rel)) != h:
            bad += 1
            print('ZIP MISMATCH', rel)
        if sha(open(f'{ROOT}/{rel}', 'rb').read()) != h:
            bad += 1
            print('LIVE CHANGED', rel)
    if names != {e[0] for e in entries}:
        bad += 1
        print('NAME SET DIFFERS')
zh = sha(open(zpath, 'rb').read())
print(f'files={len(entries)} zip={zpath} zip_sha256={zh} verify_mismatches={bad}')
open(f'{OUT}/{label}_{ts}.zip.sha256', 'w').write(f'{zh}  {os.path.basename(zpath)}\n')
sys.exit(1 if bad else 0)
