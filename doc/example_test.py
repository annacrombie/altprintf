#!/usr/bin/env python3

import sys

sys.path.append("..")

from test.examples import ExampleTesterCmd, examples
from doc.example import examples

tester = ExampleTesterCmd("example", sys.argv[1])
examples.test(tester)
