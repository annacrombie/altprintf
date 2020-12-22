#!/usr/bin/env python3

import sys


sys.path.append("../test")

mode = sys.argv[1]

with open(sys.argv[2]) as f:
    src_text = f.read()


def insert(text, split_on, insert):
    split = text.split(split_on)
    assert len(split) == 2

    return split[0] + insert + split[1]


if mode == "grammar":
    from apf_grammar import cfg
    from examples import examples

    res = insert(src_text, "@GRAMMAR@", str(cfg))
    res = insert(res, "@EXAMPLES@", str(examples))
elif mode == "example":
    inserts = []
    with open(sys.argv[3]) as f:
        inserts += [f.read()]

    from examples import ExampleTesterCmd

    sys.path.append("../doc")
    from example import examples

    tester = ExampleTesterCmd("example", None)
    inserts += [tester.format(examples)]

    res = insert(src_text, "@EXAMPLE@", "\n---\n\n".join(inserts))
else:
    assert False

print(res, end="")
