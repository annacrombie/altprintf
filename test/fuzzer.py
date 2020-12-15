from collections import defaultdict
import random


class Num:
    def __init__(self, maximum):
        self.maximum = maximum

    def desc(self):
        return f"0-{self.maximum}"

    def __str__(self):
        return str(random.randint(0, self.maximum))


class CharClass:
    def __init__(self, name, ranges):
        self.name = name
        self.sym_name = name
        self.excluded = []
        self.ranges = ranges
        self.alphabet = [
            chr(code_point)
            for current_range in ranges
            for code_point in range(current_range[0], current_range[1] + 1)
        ]

    def exclude(self, *excl):
        new = CharClass(self.name, self.ranges)
        new.excluded = self.excluded + list(excl)

        for c in new.excluded:
            new.alphabet.remove(c)

        return new

    def with_sym_name(self, name):
        self.sym_name = name
        return self

    def desc(self):
        if self.excluded:
            return self.name + " except " + "".join(self.excluded)
        else:
            return self.name

    def __str__(self):
        return random.choice(self.alphabet)


Ascii = CharClass("Ascii", [(9, 10), (32, 126)])

# Actually, this isn't all of unicode
Utf8 = CharClass(
    "Uft8",
    [
        (0x0021, 0x0021),
        (0x0023, 0x0026),
        (0x0028, 0x007E),
        (0x00A1, 0x00AC),
        (0x00AE, 0x00FF),
        (0x0100, 0x017F),
        (0x0180, 0x024F),
        (0x2C60, 0x2C7F),
        (0x16A0, 0x16F0),
        (0x0370, 0x0377),
        (0x037A, 0x037E),
        (0x0384, 0x038A),
        (0x038C, 0x038C),
        (0x3000, 0x303F),
        (0x3040, 0x309F),
        (0x30A0, 0x30FF),
        (0xFF00, 0xFFEF),
        (0x4E00, 0x9FAF),
    ],
)


def print_tree(tree, depth=0):
    for node in tree:
        if node == "":
            return
        elif type(node) == list:
            print_tree(node, depth=depth + 1)
        elif type(node) == CharClass:
            print("  " * depth, node.full_name())
        elif type(node) == str:
            print("  " * depth, node)
        else:
            print("  " * depth, node.__class__.__name__)


def str_tree(tree):
    s = []

    for node in tree:
        if type(node) == list:
            s += str_tree(node)
        else:
            s += str(node)

    return "".join(s)


class Fuzzer:
    def __init__(self, cfg):
        self.cfg = cfg

    def gen(self):
        return self.gen_random_convergent(self.cfg.root, cfactor=0.7)

    def args(self):
        r = []
        for x in range(20):
            if random.random() > 0.5:
                v = random.randint(0, 1)
            else:
                v = Utf8.__str__()

            r.append(v)

        return r

    def gen_random_convergent(self, symbol, cfactor=0.25, pcount=defaultdict(int)):
        sentence = []

        # The possible productions of this symbol are weighted
        # by their appearance in the branch that has led to this
        # symbol in the derivation
        #
        weights = []
        for prod in self.cfg.prod[symbol]:
            tprod = tuple(prod)
            if tprod in pcount:
                weights.append(cfactor ** (pcount[tprod]))
            else:
                weights.append(1.0)

        rand_prod = self.cfg.prod[symbol][self.weighted_choice(weights)]

        pcount[tuple(rand_prod)] += 1

        for sym in rand_prod:
            # for non-terminals, recurse
            if sym in self.cfg.prod:
                thing = self.gen_random_convergent(sym, cfactor=cfactor, pcount=pcount)
            else:
                if callable(sym):
                    thing = sym()
                else:
                    thing = sym

            sentence += [thing]

        # backtracking: clear the modification to pcount
        pcount[tuple(rand_prod)] -= 1
        return sentence

    def weighted_choice(self, weights):
        rnd = random.random() * sum(weights)
        for i, w in enumerate(weights):
            rnd -= w
            if rnd < 0:
                return i
