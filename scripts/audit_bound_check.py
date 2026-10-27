"""Independent finite-range check of the report's SAG bound: every streak costs at most 4/3 of its length (cost model of the galloping search).
Re-implements the search as the paper/README describe it: t linear probes, then probes at t+1, t+2, t+4, ... until a true, then binary search.
A streak found by the tail search that saw k 'false' elements out of `tail` has length k+1 (the first element is known without a comparison)."""
import sys

def tests(t, k, ln):
    """number of predicate evaluations when the first true is at 0-based index k (k == ln: never true), tail length ln."""
    lin = min(t, ln)
    if k < lin:
        return k + 1
    n = lin
    if lin == ln:
        return n
    lo, step = t, 1
    while True:
        p = min(t + step, ln)
        n += 1
        if k + 1 <= p:
            hi = p
            break
        lo = p
        if p == ln:
            return n
        step <<= 1
    while hi - lo > 1:
        mid = lo + (hi - lo) // 2
        n += 1
        if k + 1 <= mid:
            hi = mid
        else:
            lo = mid
    return n

worst, arg = 0.0, None
cnt = 0
for t in range(0, 71):
    for ln in range(0, 161):
        for k in range(0, ln + 1):
            c = tests(t, k, ln)
            r = c / (k + 1)
            cnt += 1
            if r > worst:
                worst, arg = r, (t, k, ln, c)
print(f"cases {cnt}; worst cost/streak-length = {worst:.4f} at (t,k,tail,tests) = {arg}; bound 4/3 = {4/3:.4f}; holds = {worst <= 4/3 + 1e-12}")
sys.exit(0 if worst <= 4 / 3 + 1e-12 else 1)
