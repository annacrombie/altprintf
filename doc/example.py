import sys

sys.path.append("..")

from test.examples import Examples

examples = Examples().ex("hello {}", "hello world", "world")
