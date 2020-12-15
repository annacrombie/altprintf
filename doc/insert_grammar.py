#!/usr/bin/env python3

import sys

sys.path.append("../test")

from apf_grammar import cfg

src = sys.argv[1]

with open(src) as f:
    text = f.read().split("@GRAMMAR@")
    assert len(text) == 2

print(text[0], end="")
print(cfg, end="")
print(text[1], end="")
