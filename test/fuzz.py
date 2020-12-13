from collections import defaultdict
import random


def weighted_choice(weights):
    rnd = random.random() * sum(weights)
    for i, w in enumerate(weights):
        rnd -= w
        if rnd < 0:
            return i


class CFG(object):
    def __init__(self):
        self.prod = defaultdict(list)

    def add_prod(self, lhs, *rhs):
        for prod in rhs:
            self.prod[lhs].append(tuple(prod))

    def gen_random_convergent(
        self,
        symbol,
        cfactor=0.25,
        pcount=defaultdict(int),
        curlen=0,
        restrict=0,
        depth=0,
    ):
        sentence = ""

        # The possible productions of this symbol are weighted
        # by their appearance in the branch that has led to this
        # symbol in the derivation
        #
        weights = []
        for prod in self.prod[symbol]:
            if prod in pcount:
                weights.append(cfactor ** (pcount[prod]))
            else:
                weights.append(1.0)

        rand_prod = self.prod[symbol][weighted_choice(weights)]

        pcount[rand_prod] += 1

        # TODO: restrict feature currently unused, consider removal
        set_restrict = False

        for sym in rand_prod:
            # for non-terminals, recurse
            if sym in self.prod:
                thing = self.gen_random_convergent(
                    sym,
                    cfactor=cfactor,
                    pcount=pcount,
                    curlen=curlen,
                    restrict=restrict,
                    depth=depth + 1,
                )
            else:
                if callable(sym):
                    thing = sym()
                else:
                    thing = str(sym)

            curlen += len(thing)

            if restrict == 0:
                sentence += thing
            elif len(thing) + curlen < restrict:
                sentence += thing

        # backtracking: clear the modification to pcount
        pcount[rand_prod] -= 1
        return sentence


class Sym:
    s = 0
    ele = 1
    raw = 2
    expr = 3
    id_ = 4
    arg = 5
    optional_sep = 6
    align = 7
    num = 8
    prec = 9
    transform = 10
    align_char = 11
    id_or_lit = 12

    names = [
        "s",
        "ele",
        "raw",
        "expr",
        "id_",
        "arg",
        "optional_sep",
        "align",
        "num",
        "prec",
        "transform",
        "align_char",
        "id_or_lit",
    ]


random_strings = [
    "hello!",
    "world",
    "string",
    "format",
    "cool",
    "yeah",
    "groovy",
    "dig",
    "really really long",
]


def random_string():
    return random_strings[random.randint(0, len(random_strings) - 1)]


def random_char():
    return "~"


def random_num():
    return str(random.randint(1, 15))


class Fuzzer:
    def __init__(self):
        self.cfg = CFG()
        self.cfg.add_prod(Sym.s, [Sym.ele, Sym.s], [Sym.ele])
        self.cfg.add_prod(Sym.ele, [Sym.raw], ["{", Sym.expr, "}"])
        self.cfg.add_prod(
            Sym.expr,
            [Sym.id_or_lit, ":", Sym.arg],
            [Sym.id_or_lit, Sym.optional_sep],
            [":", Sym.arg],
            [Sym.id_, "?", Sym.s, Sym.optional_sep],
            [Sym.id_, "?", Sym.s, ":", Sym.s],
        )
        self.cfg.add_prod(Sym.raw, [random_string])
        self.cfg.add_prod(Sym.optional_sep, [":"], [""])
        self.cfg.add_prod(Sym.id_or_lit, [Sym.id_], ["=", Sym.s])
        self.cfg.add_prod(Sym.id_, [random_string], [""])
        self.cfg.add_prod(Sym.arg, [Sym.align, Sym.num, Sym.prec, Sym.transform])
        self.cfg.add_prod(
            Sym.align, [random_char, Sym.align_char], [Sym.align_char], [""]
        )
        self.cfg.add_prod(Sym.align_char, [">"], ["<"])
        self.cfg.add_prod(Sym.num, [random_num], [""])
        self.cfg.add_prod(Sym.prec, [".", Sym.num])
        self.cfg.add_prod(Sym.transform, ["b"], ["x"], [""])

    def gen(self):
        return self.cfg.gen_random_convergent(Sym.s, cfactor=0.7)

    def args(self):
        r = []
        for x in range(20):
            if random.random() > 0.5:
                v = random.randint(0, 1)
            else:
                v = random_string()

            r.append(v)

        return r
