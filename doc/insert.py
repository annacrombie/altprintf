#!/usr/bin/env python3

import sys

sys.path.append("../test")

from apf_grammar import cfg
from test import apf_test

mode = sys.argv[1]
src = sys.argv[2]

if mode == "grammar":
    split_at = "@GRAMMAR@"
    insert = cfg
elif mode == "example":
    cmd = sys.argv[3]
    cmd_src = sys.argv[4]
    assert apf_test(cmd, "{}", "test", "test")
    split_at = "@EXAMPLE@"
    with open(cmd_src) as f:
        insert = f.read()
else:
    assert False

with open(src) as f:
    text = f.read().split(split_at)
    assert len(text) == 2

print(text[0], end="")
print(insert, end="")
print(text[1], end="")
