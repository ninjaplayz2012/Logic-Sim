import re, sys
from StressTest import equivalent

# lines = open("Log.txt", encoding="utf-8").read().splitlines()
# steps = []
# for l in lines:
#     m = re.match(r"\((Pre|Post)-(\w+)\) Equation: (.*)", l)
#     if m: steps.append((m[1], m[2], m[3]))

# for (k1, n1, e1), (k2, n2, e2) in zip(steps, steps[1:]):
#     if k1 == "Pre" and k2 == "Post" and n1 == n2 and e1 != e2:
#         if not equivalent(e1, e2):
#             print(f"FIRST BAD LAW: {n1}\nBEFORE: {e1}\nAFTER : {e2}")
#             sys.exit()
# print("every law step preserved equivalence")

for n, line in enumerate(open("Log.txt", encoding="utf-8"), 1):
    m = re.match(r"\s*New Term/Equation: (.*) -> (.*)$", line.rstrip("\n"))
    if m and not equivalent(m[1], m[2]):
        print(f"BAD PAIR (line {n})\nIN : {m[1]}\nOUT: {m[2]}")
        sys.exit()
print("all accepted pairs are equivalent")