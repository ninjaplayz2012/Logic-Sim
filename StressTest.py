import argparse, os, random, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor, as_completed

sys.setrecursionlimit(10000)

BINARY_OPERATORS = ["∧", "∨", "↑", "↓", "⊕", "⊙"]
ALL_OPERATORS = BINARY_OPERATORS + ["¬"]

LAWS = [
    "Distributivity",
    "Identity",
    "Annihilator",
    "Idempotence",
    "Absorption",
    "Complementation",
    "DoubleNegation",
    "DeMorgan"
]

# ---------- generation ----------
def gen(n_leaves, vars_, rng, p_not, p_const, operators):
    if n_leaves == 1:
        leaf = rng.choice("01") if rng.random() < p_const else rng.choice(vars_)
        return f"(¬{leaf})" if rng.random() < p_not else leaf

    left = rng.randint(1, n_leaves - 1)
    op = rng.choice(operators)

    node = f"({gen(left, vars_, rng, p_not, p_const, operators)}{op}{gen(n_leaves - left, vars_, rng, p_not, p_const, operators)})"

    return f"(¬{node})" if rng.random() < p_not * 0.5 else node


# ---------- parsing / evaluation ----------
class Parser:
    PRECEDENCE = {
        "∨": 1,
        "↓": 1,
        "⊕": 2,
        "⊙": 2,
        "∧": 3,
        "↑": 3
    }

    def __init__(self, s):
        self.t = [c for c in s if not c.isspace()]
        self.i = 0

    def peek(self):
        return self.t[self.i] if self.i < len(self.t) else None

    def eat(self, c=None):
        tok = self.peek()

        if tok is None or (c and tok != c):
            raise ValueError(f"expected {c!r} at {self.i}, got {tok!r}")

        self.i += 1
        return tok

    def parse(self):
        node = self.expression()

        if self.peek() is not None:
            raise ValueError("trailing input")

        return node

    def expression(self, min_precedence=0):
        node = self.not_()

        while True:
            op = self.peek()
            precedence = self.PRECEDENCE.get(op, -1)

            if precedence < min_precedence:
                break

            self.eat()
            right = self.expression(precedence + 1)
            node = (op, node, right)

        return node

    def not_(self):
        if self.peek() == "¬":
            self.eat()
            return ("¬", self.not_())

        return self.atom()

    def atom(self):
        tok = self.peek()

        if tok == "(":
            self.eat("(")
            node = self.expression()
            self.eat(")")
            return node

        if tok is not None and (tok.isalpha() or tok in "01"):
            self.eat()
            return ("v", tok)

        raise ValueError(f"unexpected {tok!r} at {self.i}")


def variables(ast, acc):
    if ast[0] == "v":
        if ast[1].isalpha():
            acc.add(ast[1])
    else:
        for child in ast[1:]:
            variables(child, acc)

    return acc


def table(ast, masks, full):
    k = ast[0]

    if k == "v":
        if ast[1] == "0":
            return 0
        if ast[1] == "1":
            return full
        return masks[ast[1]]

    if k == "¬":
        return full ^ table(ast[1], masks, full)

    a = table(ast[1], masks, full)
    b = table(ast[2], masks, full)

    if k == "∧":
        return a & b
    if k == "∨":
        return a | b
    if k == "↑":
        return full ^ (a & b)
    if k == "↓":
        return full ^ (a | b)
    if k == "⊕":
        return a ^ b
    if k == "⊙":
        return full ^ (a ^ b)

    raise ValueError(f"unknown operator {k!r}")


def equivalent(orig, simp):
    ao = Parser(orig).parse()
    ass = Parser(simp).parse()

    vo = variables(ao, set())
    vs = variables(ass, set())

    if not vs <= vo:
        return False

    vl = sorted(vo)
    rows = 1 << len(vl)
    full = (1 << rows) - 1

    masks = {}

    for idx, v in enumerate(vl):
        m = 0

        for r in range(rows):
            if (r >> idx) & 1:
                m |= 1 << r

        masks[v] = m

    return table(ao, masks, full) == table(ass, masks, full)


def count_ops(s):
    return sum(s.count(op) for op in ALL_OPERATORS)


# ---------- running ----------
def run_batch(exe, eqs, timeout, law_args):
    try:
        p = subprocess.run(
            [exe, "--logic", *law_args, *eqs],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout
        )
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


# ---------- main ----------
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

    ap.add_argument(
        "--operators",
        nargs="+",
        choices=ALL_OPERATORS,
        default=BINARY_OPERATORS,
        help="operators to generate"
    )

    for law in LAWS:
        aliases = [f"--{law}"]

        if law == "Annihilator":
            aliases.append("--Annihlation")

        ap.add_argument(
            *aliases,
            dest=law,
            action="store_true",
            help=f"enable the {law} law"
        )

    a = ap.parse_args()

    # If no law flags were provided, enable every law.
    selected_laws = [
        law for law in LAWS if getattr(a, law)
    ]

    if not selected_laws:
        selected_laws = LAWS.copy()

    law_args = [f"--{law}" for law in selected_laws]

    rng = random.Random(a.seed)
    vars_ = [chr(ord("A") + i) for i in range(a.vars)]

    eqs = []

    for _ in range(a.count):
        terms = rng.randint(a.min_terms, a.max_terms)

        eqs.append((
            terms,
            gen(
                terms,
                vars_,
                rng,
                a.p_not,
                a.p_const,
                a.operators
            )
        ))

    terms_of = {eq: t for t, eq in eqs}

    batches = [
        [eq for _, eq in eqs[i:i + a.batch]]
        for i in range(0, len(eqs), a.batch)
    ]

    stats = dict(
        pass_=0,
        wrong=0,
        malformed=0,
        crash=0,
        timeout=0
    )

    in_ops = out_ops = done = 0
    fails = []
    t0 = time.time()

    def work(batch):
        res = run_batch(
            a.exe,
            batch,
            a.timeout * len(batch),
            law_args
        )

        if res is None:
            res = {}

            for eq in batch:
                r = run_batch(
                    a.exe,
                    [eq],
                    a.timeout,
                    law_args
                )

                res[eq] = "TIMEOUT" if r is None else r[eq]

        return res

    with ThreadPoolExecutor(max_workers=a.workers) as pool:
        futures = [
            pool.submit(work, batch)
            for batch in batches
        ]

        for fut in as_completed(futures):
            for eq, out in fut.result().items():
                terms = terms_of[eq]
                done += 1

                if out == "TIMEOUT":
                    stats["timeout"] += 1
                    fails.append(("TIMEOUT", terms, eq, ""))
                    continue

                if out is None:
                    stats["crash"] += 1
                    fails.append(("CRASH", terms, eq, ""))
                    continue

                try:
                    ok = equivalent(eq, out)

                except ValueError as e:
                    stats["malformed"] += 1
                    fails.append((f"MALFORMED({e})", terms, eq, out))
                    continue

                if ok:
                    stats["pass_"] += 1
                    in_ops += count_ops(eq)
                    out_ops += count_ops(out)
                else:
                    stats["wrong"] += 1
                    fails.append(("NOT EQUIVALENT", terms, eq, out))

            print(
                f"[{done}/{a.count}] {stats}  "
                f"{time.time() - t0:.0f}s",
                flush=True
            )

    print("\n=== Configuration ===")
    print(f"Laws: {', '.join(selected_laws)}")
    print(f"Operators: {' '.join(a.operators)} + ¬")

    print("\n=== Results ===")

    for k, v in stats.items():
        print(f"{k:10s} {v}")

    if stats["pass_"]:
        print(
            f"avg operators: "
            f"{in_ops / stats['pass_']:.1f} -> "
            f"{out_ops / stats['pass_']:.1f}"
        )

    if fails:
        with open(a.out, "w", encoding="utf-8") as f:
            for kind, terms, eq, out in fails:
                f.write(
                    f"{kind}  terms={terms}\n"
                    f"IN : {eq}\n"
                    f"OUT: {out}\n\n"
                )

        print(f"{len(fails)} failures written to {a.out}")


if __name__ == "__main__":
    main()