from collections import defaultdict


class Syms:
    class Sym_:
        def __init__(self, name):
            self.name = name

        def __str__(self):
            return self.name

    def __init__(self):
        self.syms = {}

    def sym(self, name):
        if name not in self.syms:
            sym = Syms.Sym_(name)
            self.syms[name] = sym

        return self.syms[name]


def scdoc_escape(string):
    res = ""
    for c in string:
        if c == "\\":
            res += "\\\\"
        else:
            res += c

    return res


class Cfg(object):
    def __init__(self, root):
        self.prod = defaultdict(list)
        self.root = root

    def a(self, lhs, *rhs):
        for prod in rhs:
            self.prod[lhs].append(prod)

        return self

    def validate(self):
        unused = {}
        for lhs in self.prod:
            unused[lhs] = 1

        for lhs in self.prod:
            for rhs in self.prod[lhs]:
                for e in rhs:
                    if type(e) == Syms.Sym_:
                        if e not in self.prod:
                            raise Exception(f"unterminated symbol: {e.name}")
                        elif e in unused:
                            unused.pop(e)

        if unused:
            unused_names = []
            for x in unused:
                unused_names += [x.name]

            raise Exception(f"unused symbols: {unused_names}")

        return self

    def __str__(self):
        prod_rules = []
        for lhs in self.prod:
            s = f"*{lhs.name}* -> "
            rules = []
            for rhs in self.prod[lhs]:
                rhs_elems = []
                for e in list(rhs):
                    if type(e) == Syms.Sym_:
                        rhs_elems += [f"*{e}*"]
                    elif hasattr(e, "desc"):
                        rhs_elems += [f"_{e.desc()}_"]
                    else:
                        rhs_elems += [str(e)]

                rules += [" ".join(map(scdoc_escape, rhs_elems))]

            s += " | ".join(rules)
            prod_rules += [s]

        return "++\n".join(prod_rules)
