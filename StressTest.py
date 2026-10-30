#!/usr/bin/env python3
import argparse, os, random, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor, as_completed

sys.setrecursionlimit(10000)

# ---------- generation ----------
def gen(n_leaves, vars_, rng, p_not, p_const):
    if n_leaves == 1:
        leaf = rng.choice("01") if rng.random() < p_const else rng.choice(vars_)
        return f"(¬{leaf})" if rng.random() < p_not else leaf
    left = rng.randint(1, n_leaves - 1)
    op = rng.choice("∧∨")
    node = f"({gen(left, vars_, rng, p_not, p_const)}{op}{gen(n_leaves - left, vars_, rng, p_not, p_const)})"
    return f"(¬{node})" if rng.random() < p_not * 0.5 else node

# ---------- parsing / evaluation ----------
class Parser:
    def __init__(self, s):
        self.t, self.i = [c for c in s if not c.isspace()], 0
    def peek(self): return self.t[self.i] if self.i < len(self.t) else None
    def eat(self, c=None):
        tok = self.peek()
        if tok is None or (c and tok != c): raise ValueError(f"expected {c!r} at {self.i}, got {tok!r}")
        self.i += 1; return tok
    def parse(self):
        node = self.or_()
        if self.peek() is not None: raise ValueError("trailing input")
        return node
    def or_(self):
        n = self.and_()
        while self.peek() == "∨": self.eat(); n = ("∨", n, self.and_())
        return n
    def and_(self):
        n = self.not_()
        while self.peek() == "∧": self.eat(); n = ("∧", n, self.not_())
        return n
    def not_(self):
        if self.peek() == "¬": self.eat(); return ("¬", self.not_())
        return self.atom()
    def atom(self):
        tok = self.peek()
        if tok == "(":
            self.eat("("); n = self.or_(); self.eat(")"); return n
        if tok is not None and (tok.isalpha() or tok in "01"):
            self.eat(); return ("v", tok)
        raise ValueError(f"unexpected {tok!r}")

def variables(ast, acc):
    if ast[0] == "v":
        if ast[1].isalpha(): acc.add(ast[1])
    else:
        for c in ast[1:]: variables(c, acc)
    return acc

def table(ast, masks, full):
    k = ast[0]
    if k == "v":
        if ast[1] == "0": return 0
        if ast[1] == "1": return full
        return masks[ast[1]]
    if k == "¬": return full ^ table(ast[1], masks, full)
    a, b = table(ast[1], masks, full), table(ast[2], masks, full)
    return a & b if k == "∧" else a | b

def equivalent(orig, simp):
    ao, as_ = Parser(orig).parse(), Parser(simp).parse()
    vo, vs = variables(ao, set()), variables(as_, set())
    if not vs <= vo: return False
    vl = sorted(vo)
    rows = 1 << len(vl)
    full = (1 << rows) - 1
    masks = {}
    for idx, v in enumerate(vl):
        m = 0
        for r in range(rows):
            if (r >> idx) & 1: m |= 1 << r
        masks[v] = m
    return table(ao, masks, full) == table(as_, masks, full)

def count_ops(s): return s.count("∧") + s.count("∨") + s.count("¬")

# ---------- running ----------
def run_batch(exe, eqs, timeout):
    try:
        p = subprocess.run([exe, *eqs], capture_output=True, text=True,
                           encoding="utf-8", errors="replace", timeout=timeout)
    except subprocess.TimeoutExpired:
        return None
    results = {eq: None for eq in eqs}
    for line in p.stdout.splitlines():
        for eq in eqs:
            prefix = eq + " → "
            if line.startswith(prefix):
                results[eq] = line[len(prefix):].strip()
                break
    return results

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", default="./build/LogicSim.exe")
    ap.add_argument("--count", type=int, default=1000)
    ap.add_argument("--min-terms", type=int, default=10)
    ap.add_argument("--max-terms", type=int, default=100)
    ap.add_argument("--vars", type=int, default=6)
    ap.add_argument("--p-not", type=float, default=0.25)
    ap.add_argument("--p-const", type=float, default=0.08)
    ap.add_argument("--timeout", type=float, default=10, help="per equation")
    ap.add_argument("--batch", type=int, default=8)
    ap.add_argument("--workers", type=int, default=os.cpu_count())
    ap.add_argument("--seed", type=int, default=12345)
    ap.add_argument("--out", default="stress_failures.txt")
    a = ap.parse_args()

    rng = random.Random(a.seed)
    vars_ = [chr(ord("A") + i) for i in range(a.vars)]
    eqs = []
    for _ in range(a.count):
        t = rng.randint(a.min_terms, a.max_terms)
        eqs.append((t, gen(t, vars_, rng, a.p_not, a.p_const)))
    terms_of = {eq: t for t, eq in eqs}
    batches = [[eq for _, eq in eqs[i:i + a.batch]] for i in range(0, len(eqs), a.batch)]

    stats = dict(pass_=0, wrong=0, malformed=0, crash=0, timeout=0)
    in_ops = out_ops = done = 0
    fails = []
    t0 = time.time()

    def work(batch):
        res = run_batch(a.exe, batch, a.timeout * len(batch))
        if res is None:
            res = {}
            for eq in batch:
                r = run_batch(a.exe, [eq], a.timeout)
                res[eq] = "TIMEOUT" if r is None else r[eq]
        return res

    with ThreadPoolExecutor(max_workers=a.workers) as pool:
        for fut in as_completed([pool.submit(work, b) for b in batches]):
            for eq, out in fut.result().items():
                terms = terms_of[eq]
                done += 1
                if out == "TIMEOUT":
                    stats["timeout"] += 1; fails.append(("TIMEOUT", terms, eq, "")); continue
                if out is None:
                    stats["crash"] += 1; fails.append(("CRASH", terms, eq, "")); continue
                try:
                    ok = equivalent(eq, out)
                except ValueError as e:
                    stats["malformed"] += 1; fails.append((f"MALFORMED({e})", terms, eq, out)); continue
                if ok:
                    stats["pass_"] += 1; in_ops += count_ops(eq); out_ops += count_ops(out)
                else:
                    stats["wrong"] += 1; fails.append(("NOT EQUIVALENT", terms, eq, out))
            print(f"[{done}/{a.count}] {stats}  {time.time()-t0:.0f}s", flush=True)

    print("\n=== Results ===")
    for k, v in stats.items(): print(f"{k:10s} {v}")
    if stats["pass_"]: print(f"avg operators: {in_ops/stats['pass_']:.1f} -> {out_ops/stats['pass_']:.1f}")
    if fails:
        with open(a.out, "w", encoding="utf-8") as f:
            for kind, terms, eq, out in fails:
                f.write(f"{kind}  terms={terms}\nIN : {eq}\nOUT: {out}\n\n")
        print(f"{len(fails)} failures written to {a.out}")

if __name__ == "__main__":
    main()