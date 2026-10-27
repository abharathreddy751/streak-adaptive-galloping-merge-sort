"""Compare two g++ -S outputs function by function.  Functions are matched by section/symbol name, so the ORDER in which the
compiler emitted them does not matter; local labels (.L123, .LLSDA45 ...) are renumbered per function by first appearance, because
their global numbers depend on how many labels were generated earlier in the translation unit.  The file-level symbol directives ('.def' on Windows; '.hidden'/'.weak' on Linux) are
compared as one set for the whole file (where g++ places them between functions is arbitrary).  Anything else must be identical."""
import re, sys

DEF = re.compile(r"\s*\.(def|hidden|weak)\b")


def defs(path):
    return sorted(set(l.strip() for l in open(path, encoding="utf-8") if DEF.match(l)))


def chunks(path):
    text = open(path, encoding="utf-8").read().replace("\r", "")
    parts = re.split(r"(?m)^(?=\t\.section\t)", text)
    out = {}
    for i, part in enumerate(parts):
        lines = [l for l in part.split("\n") if not re.match(r"\s*\.(file|ident)\b", l) and not DEF.match(l)]
        m = re.match(r"\t\.section\t([^,\n]+)", part)
        key = m.group(1) if m else "#preamble%d" % i
        seen = {}

        def lab(mm):
            return seen.setdefault(mm.group(0), ".LL%d" % len(seen))

        body = re.sub(r"\.L[A-Za-z]*\d+", lab, "\n".join(lines))
        out.setdefault(key, []).append(body)
    return out


a, b = chunks(sys.argv[1]), chunks(sys.argv[2])
only_a, only_b = sorted(set(a) - set(b)), sorted(set(b) - set(a))
diff = [k for k in a if k in b and sorted(a[k]) != sorted(b[k])]
if defs(sys.argv[1]) != defs(sys.argv[2]):
    diff.append("(file-level symbol directives differ)")
print("functions/sections: v3 %d, v4 %d; only in v3: %d; only in v4: %d; different body: %d" % (len(a), len(b), len(only_a), len(only_b), len(diff)))
for k in only_a[:5]:
    print("  only in v3:", k)
for k in only_b[:5]:
    print("  only in v4:", k)
for k in diff[:5]:
    print("  different:", k)
sys.exit(0 if not (only_a or only_b or diff) else 1)
