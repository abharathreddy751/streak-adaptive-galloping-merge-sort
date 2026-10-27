#!/usr/bin/env python3
"""Independent check of `nms_tool trace` and `nms_tool profile` against a separate Python model (v4 audit).

    python -I scripts/audit_cli_check.py [path-to-nms_tool] [number-of-random-inputs]     # exit 0 iff every check agrees

trace  : for random small inputs (many ties) a Python model of natural merge sort derives, from the algorithm's definition, the runs found, the
         merge sequence (PowerSort order), whether each merge is skipped, the galloping blocks (maximal streaks of output elements coming from one
         run; the streak at the end where the merge finishes is the tail that is not searched: last for forward merges, first for backward merges) and the sorted result; they must equal the events printed by
         the tool.  (Comparison counts are not modelled here: they are cross-checked inside the tool against an external counter.)
profile: distinct keys, descents, runs, run lengths, H and H* are recomputed from their definitions and compared with the printed values.
"""
import math, random, re, subprocess, sys, os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
exe = sys.argv[1] if len(sys.argv) > 1 else str(ROOT / "build" / ("nms_tool.exe" if os.name == "nt" else "nms_tool"))
N = int(sys.argv[2]) if len(sys.argv) > 2 else 300
exe = str(Path(exe).resolve())
env = dict(os.environ)
env["PATH"] = "/c/msys64/usr/bin;C:\\msys64\\usr\\bin;" + env.get("PATH", "")
fails = []


def run(args):
    p = subprocess.run([exe] + args, capture_output=True, text=True, env=env)
    return p.returncode, p.stdout.replace("\r", "")


# ---------------------------------------------------------------- model of the natural merge sort (stable, ties to the left run)
def node_power(s1, n1, n2, n):
    result, a, b = 0, 2 * s1 + n1, 2 * s1 + n1 + n1 + n2
    while True:
        result += 1
        if a >= n:
            a -= n; b -= n
        elif b >= n:
            break
        a <<= 1; b <<= 1
    return result


def model(keys, minrun):
    """returns (events, sorted_keys); records (key, id) so stability holds."""
    n = len(keys)
    a = [(k, i) for i, k in enumerate(keys)]
    ev = []

    def next_run(pos):
        rem = n - pos
        if rem == 1:
            r = 1
        else:
            r = 2
            if a[pos + 1][0] < a[pos][0]:
                while r < rem and a[pos + r][0] < a[pos + r - 1][0]:
                    r += 1
                a[pos:pos + r] = a[pos:pos + r][::-1]
            else:
                while r < rem and not (a[pos + r][0] < a[pos + r - 1][0]):
                    r += 1
        found = r
        if r < minrun:
            force = min(n - pos, minrun)
            a[pos:pos + force] = sorted(a[pos:pos + force], key=lambda t: t[0])  # stable: equals binary insertion of the unsorted tail
            r = force
        ev.append(("run", pos, found, r))
        return pos, r

    def merge_at(start, la, lb):
        A, B = a[start:start + la], a[start + la:start + la + lb]
        if not (B[0][0] < A[-1][0]):
            ev.append(("skip", start, la, lb))
            return
        ev.append(("merge", start, la, lb))
        out, origin, i, j = [], [], 0, 0
        while i < la and j < lb:
            if B[j][0] < A[i][0]:
                out.append(B[j]); origin.append("R"); j += 1
            else:
                out.append(A[i]); origin.append("L"); i += 1
        out += A[i:] + B[j:]; origin += ["L"] * (la - i) + ["R"] * (lb - j)
        streaks = []
        for o in origin:
            if streaks and streaks[-1][0] == o:
                streaks[-1][1] += 1
            else:
                streaks.append([o, 1])
        # forward merge (la <= lb, left run buffered) discovers streaks from the front and does not search the last one;
        # backward merge (la > lb, right run buffered) works from the end of the array: it discovers them in reverse order and does not search the first
        found = streaks[:-1] if la <= lb else streaks[:0:-1]
        for o, ln in found:
            ev.append(("block", ln, "left" if o == "L" else "right"))
        ev.append(("end", start))
        a[start:start + la + lb] = out

    if n < 2:
        return ev, [k for k, _ in a]
    runs, pw = [], []
    pos = 0
    x = next_run(pos); pos += x[1]
    x = list(x)
    while pos < n:
        y = list(next_run(pos)); pos += y[1]
        p = node_power(x[0], x[1], y[1], n)
        while runs and pw[-1] >= p:
            r = runs.pop(); pw.pop()
            merge_at(r[0], r[1], x[1]); x = [r[0], r[1] + x[1]]
        runs.append(x); pw.append(p)
        x = y
    while runs:
        r = runs.pop(); pw.pop()
        merge_at(r[0], r[1], x[1]); x = [r[0], r[1] + x[1]]
    return ev, [k for k, _ in a]


def parse_trace(text):
    ev = []
    for line in text.split("\n"):
        m = re.match(r"\[run \d+\] elements (\d+)\.\.(\d+): run detection found (\d+)", line)
        if m:
            s, e, f = int(m[1]), int(m[2]), int(m[3]); ev.append(("run", s, f, e - s + 1)); continue
        m = re.match(r"\[merge\] at (\d+): left run (\d+) \+ right run (\d+)", line)
        if m: ev.append(("merge", int(m[1]), int(m[2]), int(m[3]))); continue
        m = re.match(r"\[merge skipped\] at (\d+): left (\d+) \+ right (\d+)", line)
        if m: ev.append(("skip", int(m[1]), int(m[2]), int(m[3]))); continue
        m = re.match(r"\s+block of (\d+) element\(s\) from the (left|right) run", line)
        if m: ev.append(("block", int(m[1]), m[2])); continue
        m = re.match(r"\[merge done\] at (\d+):", line)
        if m: ev.append(("end", int(m[1]))); continue
    res = re.search(r"\[result\] sorted: (.*)", text)
    return ev, [int(t) for t in res[1].split()] if res else None


rng = random.Random(20261027)
n_trace = 0
for it in range(N):
    n = rng.randint(1, 60)
    sigma = rng.choice([2, 3, 5, 10, 1000])
    mode = rng.choice(["rand", "runs", "rev", "sorted"])
    if mode == "rand": keys = [rng.randrange(sigma) for _ in range(n)]
    elif mode == "sorted": keys = sorted(rng.randrange(sigma) for _ in range(n))
    elif mode == "rev": keys = sorted((rng.randrange(sigma) for _ in range(n)), reverse=True)
    else:
        keys = []
        while len(keys) < n:
            keys += sorted(rng.randrange(sigma) for _ in range(rng.randint(1, 15)))
        keys = keys[:n]
    minrun = rng.choice([1, 2, 4, 8])
    exp_ev, exp_sorted = model(keys, minrun)
    rc, out = run(["trace", "--keys", ",".join(map(str, keys)), "--minrun", str(minrun), "--policy", rng.choice(["base", "sag"])])
    got_ev, got_sorted = parse_trace(out)
    n_trace += 1
    if rc != 0 or got_ev != exp_ev or got_sorted != exp_sorted:
        fails.append(("trace", keys, minrun, rc))
        if len(fails) <= 3:
            print("TRACE MISMATCH", keys, "minrun", minrun, "rc", rc); print(" model :", exp_ev[:8]); print(" tool  :", got_ev[:8])
print(f"trace: {n_trace} random inputs compared with the Python model; mismatches so far {len(fails)}")


# ---------------------------------------------------------------- profile
def runs_of(a):
    lens, i, n = [], 0, len(a)
    while i < n:
        if i + 1 == n: lens.append(1); break
        r = 2
        if a[i + 1] < a[i]:
            while i + r < n and a[i + r] < a[i + r - 1]: r += 1
        else:
            while i + r < n and not (a[i + r] < a[i + r - 1]): r += 1
        lens.append(r); i += r
    return lens


def ent(lens, n):
    return -sum(l / n * math.log2(l / n) for l in lens)


def dual_runs(a):
    n = len(a)
    pos = sorted(range(n), key=lambda i: (a[i], i))  # stable rank order: pos[r] = original index of the element with rank r
    lens, i = [], 0
    while i < n:
        if i + 1 == n: lens.append(1); break
        r = 2
        if pos[i + 1] > pos[i]:
            while i + r < n and pos[i + r] > pos[i + r - 1]: r += 1
        else:
            while i + r < n and pos[i + r] < pos[i + r - 1]: r += 1
        lens.append(r); i += r
    return lens


n_prof = 0
for it in range(N):
    n = rng.randint(1, 120)
    sigma = rng.choice([1, 2, 4, 20, 100000])
    keys = [rng.randrange(sigma) for _ in range(n)]
    if rng.random() < 0.3: keys.sort()
    if rng.random() < 0.2: keys.sort(reverse=True)
    rc, out = run(["profile", "--keys", ",".join(map(str, keys))])
    g = lambda pat: re.search(pat, out)
    lens = runs_of(keys)
    asc = sum(1 for k, l in zip(range(len(lens)), lens))  # placeholder, replaced below
    pos, a_runs, d_runs = 0, 0, 0
    for l in lens:
        if l >= 2 and keys[pos + 1] < keys[pos]: d_runs += 1
        else: a_runs += 1
        pos += l
    exp = dict(n=n, distinct=len(set(keys)), desc=sum(1 for i in range(1, n) if keys[i] < keys[i - 1]), runs=len(lens), asc=a_runs, dr=d_runs,
               rmin=min(lens), rmax=max(lens), H=ent(lens, n), Hs=ent(dual_runs(keys), n))
    got = dict(n=int(g(r"elements n\s+: (\d+)")[1]), distinct=int(g(r"distinct keys\s+: (\d+)")[1]), desc=int(g(r"adjacent descents\s+: (\d+)")[1]),
               runs=int(g(r"natural runs\s+: (\d+)")[1]), asc=int(g(r"\((\d+) non-decreasing")[1]), dr=int(g(r"(\d+) strictly decreasing")[1]),
               rmin=int(g(r"min/mean/max\s+: (\d+) /")[1]), rmax=int(g(r"/ ([\d.]+) / (\d+)\n")[2]),
               H=float(g(r"run entropy H\s+: ([\d.]+) bits")[1]), Hs=float(g(r"H\* : ([\d.]+) bits")[1]))
    lab_sorted = g(r"already sorted\s+: (yes|no)")[1] == "yes"
    lab_rev = g(r"reverse sorted\s+: (yes|no)")[1] == "yes"
    ok = rc == 0 and all(exp[k] == got[k] for k in ("n", "distinct", "desc", "runs", "asc", "dr", "rmin", "rmax")) \
        and abs(exp["H"] - got["H"]) < 6e-4 and abs(exp["Hs"] - got["Hs"]) < 6e-4 \
        and lab_sorted == (exp["desc"] == 0) \
        and lab_rev == (n > 1 and all(not (keys[i - 1] < keys[i]) for i in range(1, n)))
    n_prof += 1
    if not ok:
        fails.append(("profile", keys))
        if len(fails) <= 3: print("PROFILE MISMATCH", keys, exp, got)
print(f"profile: {n_prof} random inputs compared with independent recomputation")
print("CLI AUDIT:", "ALL AGREE" if not fails else f"{len(fails)} MISMATCH")
sys.exit(1 if fails else 0)
