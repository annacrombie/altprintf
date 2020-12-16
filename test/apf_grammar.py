from fuzzer import Ascii, Utf8, Num
from grammar import Cfg, Syms

IdChar = Utf8.exclude("?", ":", "}").with_sym_name("IdChar")
IdStartChar = IdChar.exclude("=").with_sym_name("IdStartChar")
RawChar = Utf8.exclude("{", "\\").with_sym_name("RawChar")
ExprLitChar = RawChar.exclude(":", "}").with_sym_name("ExprLitChar")
AsciiChar = Ascii

s = Syms()
cfg = (
    Cfg(s.sym("S"))
    .a(s.sym("S"))
    .a(s.sym("S"), [s.sym("Elem"), s.sym("S")], [s.sym("Elem")])
    .a(s.sym("Elem"), [s.sym("Raw")], ["{", s.sym("Expr"), "}"])
    .a(s.sym("Expr"), [s.sym("BasicExpr")], [s.sym("CondExpr")])
    .a(
        s.sym("BasicExpr"),
        [s.sym("IdOrLit"), ":", s.sym("Arg")],
        [s.sym("IdOrLit"), s.sym("OptionalColon")],
        [":", s.sym("Arg")],
    )
    .a(
        s.sym("CondExpr"),
        [s.sym("Id"), "?", s.sym("ExprLiteral"), s.sym("OptionalColon")],
        [s.sym("Id"), "?", s.sym("ExprLiteral"), ":", s.sym("ExprLiteral")],
    )
    .a(
        s.sym("Raw"),
        [RawChar, s.sym("Raw")],
        [s.sym("EscapeSequence"), s.sym("Raw")],
        [s.sym("Nil")],
    )
    .a(s.sym("OptionalColon"), [":"], [s.sym("Nil")])
    .a(s.sym("IdOrLit"), [IdStartChar, s.sym("Id")], ["=", s.sym("ExprLiteral")])
    .a(
        s.sym("Id"),
        [IdStartChar, s.sym("RestOfId")],
        [s.sym("Nil")],
    )
    .a(s.sym("RestOfId"), [IdChar, s.sym("RestOfId")], [s.sym("Nil")])
    .a(
        s.sym("ExprLiteral"),
        [s.sym("EscapeSequence"), s.sym("ExprLiteral")],
        [ExprLitChar, s.sym("ExprLiteral")],
        ["{", s.sym("Expr"), "}", s.sym("ExprLiteral")],
        [s.sym("Nil")],
    )
    .a(
        s.sym("Arg"),
        [s.sym("Align"), s.sym("OptionalU8"), s.sym("Prec"), s.sym("Transform")],
    )
    .a(
        s.sym("Align"),
        [s.sym("AlignChar"), AsciiChar],
        [s.sym("Nil")],
    )
    .a(s.sym("AlignChar"), ["<"], [">"])
    .a(s.sym("Prec"), [".", s.sym("OptionalU8")], [s.sym("Nil")])
    .a(s.sym("OptionalU8"), [Num(255)], [s.sym("Nil")])
    .a(s.sym("Transform"), ["b"], ["x"], [s.sym("Nil")])
    .a(s.sym("EscapeSequence"), ["\\", AsciiChar])
    .a(s.sym("Nil"), [""])
    .validate()
)
