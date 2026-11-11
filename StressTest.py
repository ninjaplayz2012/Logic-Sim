
import argparse
import os
import random
import shutil
import subprocess
import sys
import time

from collections import defaultdict, deque
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

def gen(n_leaves, variables, rng, p_not, p_const, operators):
    if n_leaves == 1:
        leaf = (
            rng.choice("01")
            if rng.random() < p_const
            else rng.choice(variables)
        )

        return f"(¬{leaf})" if rng.random() < p_not else leaf

    left_count = rng.randint(1, n_leaves - 1)

    left = gen(
        left_count, variables, rng,
        p_not, p_const, operators
    )

    right = gen(
        n_leaves - left_count, variables, rng,
        p_not, p_const, operators
    )

    node = f"({left}{rng.choice(operators)}{right})"

    return f"(¬{node})" if rng.random() < p_not * 0.5 else node


# ---------- parsing ----------

class Parser:
    PRECEDENCE = {
        "∨": 1,
        "↓": 1,
        "⊕": 2,
        "⊙": 2,
        "∧": 3,
        "↑": 3
    }

    def __init__(self, equation):
        self.tokens = [
            c for c in equation
            if not c.isspace()
        ]
        self.index = 0

    def peek(self):
        if self.index >= len(self.tokens):
            return None

        return self.tokens[self.index]

    def eat(self, expected=None):
        token = self.peek()

        if token is None or (
            expected is not None and token != expected
        ):
            raise ValueError(
                f"Expected {expected!r} at {self.index}, got {token!r}"
            )

        self.index += 1
        return token

    def parse(self):
        node = self.expression()

        if self.peek() is not None:
            raise ValueError("Trailing input")

        return node

    def expression(self, min_precedence=0):
        node = self.not_()

        while True:
            operator = self.peek()
            precedence = self.PRECEDENCE.get(operator, -1)

            if precedence < min_precedence:
                break

            self.eat()
            right = self.expression(precedence + 1)

            node = (operator, node, right)

        return node

    def not_(self):
        if self.peek() == "¬":
            self.eat()
            return ("¬", self.not_())

        return self.atom()

    def atom(self):
        token = self.peek()

        if token == "(":
            self.eat("(")
            node = self.expression()
            self.eat(")")
            return node

        if token is not None and (
            token.isalpha() or token in "01"
        ):
            self.eat()
            return ("v", token)

        raise ValueError(
            f"Unexpected {token!r} at {self.index}"
        )


# ---------- truth tables ----------

def variables(ast, result=None):
    if result is None:
        result = set()

    kind = ast[0]

    if kind == "v":
        if ast[1].isalpha():
            result.add(ast[1])
    else:
        for child in ast[1:]:
            variables(child, result)

    return result


def table(ast, masks, full):
    kind = ast[0]

    if kind == "v":
        if ast[1] == "0":
            return 0

        if ast[1] == "1":
            return full

        return masks[ast[1]]

    if kind == "¬":
        return full ^ table(ast[1], masks, full)

    left = table(ast[1], masks, full)
    right = table(ast[2], masks, full)

    if kind == "∧":
        return left & right

    if kind == "∨":
        return left | right

    if kind == "↑":
        return full ^ (left & right)

    if kind == "↓":
        return full ^ (left | right)

    if kind == "⊕":
        return left ^ right

    if kind == "⊙":
        return full ^ (left ^ right)

    raise ValueError(f"Unknown operator {kind!r}")


def equivalent(original, simplified):
    original_ast = Parser(original).parse()
    simplified_ast = Parser(simplified).parse()

    original_variables = variables(original_ast)
    simplified_variables = variables(simplified_ast)

    if not simplified_variables <= original_variables:
        return False

    ordered_variables = sorted(original_variables)

    rows = 1 << len(ordered_variables)
    full = (1 << rows) - 1

    masks = {}

    for index, variable in enumerate(ordered_variables):
        block_size = 1 << index
        mask = 0

        for start in range(block_size, rows, block_size * 2):
            mask |= ((1 << block_size) - 1) << start

        masks[variable] = mask

    return (
        table(original_ast, masks, full)
        == table(simplified_ast, masks, full)
    )


def count_ops(equation):
    return sum(equation.count(op) for op in ALL_OPERATORS)


# ---------- process execution ----------

def invoke(executable, equations, timeout, law_args):
    try:
        process = subprocess.run(
            [executable, "-l", *law_args, *equations],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout
        )

    except subprocess.TimeoutExpired:
        return "TIMEOUT", [], ""

    if process.returncode != 0:
        return "CRASH", [], process.stderr.strip()

    pending = defaultdict(deque)

    for line in process.stdout.splitlines():
        original, separator, result = line.partition(" → ")

        if separator:
            pending[original].append(result.strip())

    outputs = []

    for equation in equations:
        outputs.append(
            pending[equation].popleft()
            if pending[equation]
            else None
        )

    return "OK", outputs, process.stderr.strip()


def run_batch(executable, equations, timeout, law_args):
    status, outputs, detail = invoke(
        executable,
        equations,
        timeout * len(equations),
        law_args
    )

    if status == "OK":
        results = []

        for equation, output in zip(equations, outputs):
            if output is not None:
                results.append(("OK", output, ""))
                continue

            single_status, single_outputs, single_detail = invoke(
                executable,
                [equation],
                timeout,
                law_args
            )

            if single_status == "OK" and single_outputs[0] is not None:
                results.append(("OK", single_outputs[0], ""))
            else:
                results.append((
                    single_status if single_status != "OK" else "MISSING",
                    None,
                    single_detail
                ))

        return results

    if len(equations) == 1:
        return [(status, None, detail)]

    results = []

    for equation in equations:
        single_status, single_outputs, single_detail = invoke(
            executable,
            [equation],
            timeout,
            law_args
        )

        if single_status == "OK" and single_outputs[0] is not None:
            results.append(("OK", single_outputs[0], ""))
        else:
            results.append((
                single_status if single_status != "OK" else "MISSING",
                None,
                single_detail
            ))

    return results


# ---------- arguments ----------

def positive_int(value):
    number = int(value)

    if number < 1:
        raise argparse.ArgumentTypeError("Must be at least 1")

    return number


def positive_float(value):
    number = float(value)

    if number <= 0:
        raise argparse.ArgumentTypeError("Must be greater than 0")

    return number


def probability(value):
    number = float(value)

    if not 0 <= number <= 1:
        raise argparse.ArgumentTypeError("Must be between 0 and 1")

    return number


def create_parser():
    parser = argparse.ArgumentParser(
        description="Generate randomized Boolean expressions and stress-test LogicSim.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        epilog="""
Examples:

  python stresstest.py
  python stresstest.py --count 10000
  python stresstest.py --min-terms 50 --max-terms 200
  python stresstest.py --vars 8 --count 5000
  python stresstest.py --p-not 0.4 --p-const 0.1
  python stresstest.py --operators ∧ ∨
  python stresstest.py --Distributivity --Identity
  python stresstest.py --no-laws
  python stresstest.py --workers 4 --batch 16
  python stresstest.py --seed 42
  python stresstest.py --timeout 5 --out failures.txt
"""
    )

    general = parser.add_argument_group("General")
    general.add_argument(
        "--exe",
        default="./build/LogicSim.exe",
        help="Path to the LogicSim executable"
    )
    general.add_argument(
        "--count",
        type=positive_int,
        default=1000,
        help="Number of equations to generate"
    )
    general.add_argument(
        "--seed",
        type=int,
        default=12345,
        help="Random generation seed"
    )
    general.add_argument(
        "--out",
        default="stress_failures.txt",
        help="Failure report output path"
    )
    general.add_argument(
        "--no-report",
        action="store_true",
        help="Do not write a failure report"
    )

    generation = parser.add_argument_group("Generation")
    generation.add_argument(
        "--min-terms",
        type=positive_int,
        default=10,
        help="Minimum number of expression leaves"
    )
    generation.add_argument(
        "--max-terms",
        type=positive_int,
        default=100,
        help="Maximum number of expression leaves"
    )
    generation.add_argument(
        "--vars",
        type=positive_int,
        default=6,
        help="Number of distinct variables (A-Z)"
    )
    generation.add_argument(
        "--p-not",
        type=probability,
        default=0.25,
        help="Probability of generating a NOT operator"
    )
    generation.add_argument(
        "--p-const",
        type=probability,
        default=0.08,
        help="Probability of generating a constant instead of a variable"
    )
    generation.add_argument(
        "--operators",
        nargs="+",
        choices=ALL_OPERATORS,
        default=BINARY_OPERATORS,
        help="Binary operators to generate"
    )

    execution = parser.add_argument_group("Execution")
    execution.add_argument(
        "--timeout",
        type=positive_float,
        default=10,
        help="Timeout per equation, in seconds"
    )
    execution.add_argument(
        "--batch",
        type=positive_int,
        default=8,
        help="Number of equations per LogicSim process"
    )
    execution.add_argument(
        "--workers",
        type=positive_int,
        default=max(1, os.cpu_count() or 1),
        help="Number of concurrent LogicSim processes"
    )
    execution.add_argument(
        "--quiet",
        action="store_true",
        help="Disable progress updates"
    )

    laws = parser.add_argument_group("Boolean laws")
    laws.add_argument(
        "--no-laws",
        action="store_true",
        help="Disable all Boolean laws"
    )

    for law in LAWS:
        aliases = [f"--{law}"]

        if law == "Annihilator":
            aliases.append("--Annihlation")

        laws.add_argument(
            *aliases,
            dest=law,
            action="store_true",
            help=f"Enable {law}"
        )

    return parser


# ---------- main ----------

def main():
    parser = create_parser()
    args = parser.parse_args()

    if args.min_terms > args.max_terms:
        parser.error("--min-terms cannot exceed --max-terms")

    if args.vars > 26:
        parser.error("--vars cannot exceed 26")

    if args.no_laws and any(getattr(args, law) for law in LAWS):
        parser.error("--no-laws cannot be combined with law flags")

    selected_laws = [
        law for law in LAWS
        if getattr(args, law)
    ]

    if args.no_laws:
        selected_laws = []
    elif not selected_laws:
        selected_laws = LAWS.copy()

    law_args = [f"--{law}" for law in selected_laws]

    executable = args.exe

    if not os.path.isfile(executable):
        executable = shutil.which(executable)

    if not executable:
        parser.error(f"LogicSim executable not found: {args.exe}")

    rng = random.Random(args.seed)
    variable_names = [
        chr(ord("A") + i)
        for i in range(args.vars)
    ]

    equations = []

    for _ in range(args.count):
        terms = rng.randint(args.min_terms, args.max_terms)

        equations.append((
            terms,
            gen(
                terms,
                variable_names,
                rng,
                args.p_not,
                args.p_const,
                args.operators
            )
        ))

    batches = [
        equations[i:i + args.batch]
        for i in range(0, len(equations), args.batch)
    ]

    stats = {
        "passed": 0,
        "wrong": 0,
        "malformed": 0,
        "crash": 0,
        "timeout": 0,
        "missing": 0
    }

    input_ops = 0
    output_ops = 0
    done = 0
    failures = []

    start_time = time.perf_counter()

    def work(batch):
        return run_batch(
            executable,
            [equation for _, equation in batch],
            args.timeout,
            law_args
        )

    
    with ThreadPoolExecutor(max_workers=args.workers) as pool:
        futures = {
            pool.submit(work, batch): batch
            for batch in batches
        }

        for future in as_completed(futures):
            batch = futures[future]
            results = future.result()

            for (terms, original_equation), (status, output, detail) in zip(
                batch, results
            ):
                done += 1

                if status != "OK":
                    stats[status.lower()] += 1
                    failures.append((
                        status,
                        original_equation,
                        "",
                        detail
                    ))
                    continue

                try:
                    ok = equivalent(original_equation, output)
                except (ValueError, RecursionError) as error:
                    stats["malformed"] += 1
                    failures.append((
                        "MALFORMED",
                        original_equation,
                        output,
                        str(error)
                    ))
                    continue

                if ok:
                    stats["passed"] += 1
                    input_ops += count_ops(original_equation)
                    output_ops += count_ops(output)
                else:
                    stats["wrong"] += 1
                    failures.append((
                        "NOT EQUIVALENT",
                        original_equation,
                        output,
                        ""
                    ))

            if not args.quiet:
                elapsed = time.perf_counter() - start_time
                print(
                    f"[{done}/{args.count}] "
                    f"Passed: {stats['passed']} | "
                    f"Wrong: {stats['wrong']} | "
                    f"Malformed: {stats['malformed']} | "
                    f"Crashes: {stats['crash']} | "
                    f"Timeouts: {stats['timeout']} | "
                    f"{elapsed:.1f}s",
                    flush=True
                )

    elapsed = time.perf_counter() - start_time

    print("\n=== Configuration ===")
    print(f"Executable: {executable}")
    print(f"Equations: {args.count}")
    print(f"Terms: {args.min_terms}-{args.max_terms}")
    print(f"Variables: {args.vars}")
    print(f"Operators: {' '.join(args.operators)} + ¬")
    print(f"Laws: {', '.join(selected_laws) if selected_laws else 'None'}")
    print(f"Workers: {args.workers}")
    print(f"Batch size: {args.batch}")
    print(f"Seed: {args.seed}")

    print("\n=== Results ===")

    for name, value in stats.items():
        print(f"{name.capitalize():12s} {value}")

    if stats["passed"]:
        print(
            f"\nAverage operators: "
            f"{input_ops / stats['passed']:.2f} → "
            f"{output_ops / stats['passed']:.2f}"
        )

        if input_ops:
            reduction = (
                1 - output_ops / input_ops
            ) * 100

            print(f"Operator reduction: {reduction:.2f}%")

    print(f"Total time: {elapsed:.3f}s")

    if failures:
        if not args.no_report:
            with open(args.out, "w", encoding="utf-8") as file:
                for kind, equation, output, detail in failures:
                    file.write(f"{kind}\n")

                    if equation:
                        file.write(f"IN : {equation}\n")

                    if output:
                        file.write(f"OUT: {output}\n")

                    if detail:
                        file.write(f"DETAIL: {detail}\n")

                    file.write("\n")

            print(
                f"\n{len(failures)} failures written to {args.out}"
            )
        else:
            print(f"\n{len(failures)} failures (report disabled)")


if __name__ == "__main__":
    main()
