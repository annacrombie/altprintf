import subprocess
import importlib.util


class ExampleTesterCExt:
    def __init__(self, extpath):
        spec = importlib.util.spec_from_file_location("apf", extpath)
        self.apf = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.apf)

    def test(self, example):
        try:
            res = self.apf.apf(example.in_, list(example.args))
        except RuntimeError as e:
            print(f"test failed: '{example.in_}' {example.args}")
            print(f"error: {e}")
            return False

        if example.out != None and res != example.out:
            print(f"test failed: '{in_}'")
            print(f"got:      '{res}'")
            print(f"expected: '{example.out}'")
            return False

        return True


class ExampleTesterCmd:
    def __init__(self, cmd_name, cmd_path):
        self.name = cmd_name
        self.cmd_path = cmd_path

    def test(self, ex):
        args = list(map(lambda x: str(x), ex.args))
        in_ = ex.in_.encode("utf-8")

        result = subprocess.run(
            [self.cmd_path, in_, *args],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        if result.stderr is not None:
            try:
                stderr = result.stderr.decode("utf-8")
            except UnicodeDecodeError:
                stderr = result.stderr
        else:
            stderr = ""

        try:
            res = result.stdout.decode("utf-8")
        except UnicodeDecodeError:
            print(f"test failed: '{in_}'")
            print(f"garbled results: {result.stdout}")
            return False

        if ex.out is None:
            if result.returncode != 0:
                print(f"test failed: '{in_}'")
                print(stderr)
                return False
        elif res != ex.out:
            print(f"test failed: '{ex.in_}'")
            print(stderr)
            print(f"got:      '{res}'")
            print(f"expected: '{ex.out}'")

            return False

        return True

    def format(self, examples):
        return "\n".join([f"$ ./{self.name} {e}" for e in examples.examples])


class Example:
    def __init__(self, in_, out, args):
        self.in_ = in_
        self.out = out
        self.args = args

    def __str__(self):
        strargs = []

        for a in self.args:
            if type(a) == str:
                strargs += [f'"{a}"']
            else:
                strargs += [str(a)]

        strargs = " ".join(strargs)

        return f'"{self.in_}" {strargs}\n{self.out}'


class Examples:
    def __init__(self):
        self.examples = []

    def __str__(self):
        return "\n\n".join([str(e) for e in self.examples])

    def ex(self, in_, out, *args):
        self.examples.append(Example(in_, out, args))
        return self

    def test(self, tester):
        for e in self.examples:
            assert tester.test(e)


examples = (
    Examples()
    .ex("hello {}", "hello world", "world")
    .ex("test", "test")
    .ex("test {}", "test string", "string")
    .ex("{:.3}", "1.500", 1.5)
    .ex("{}", "80", 80)
    .ex("{=:>#10}", "##########")
    .ex("{?a:b}", "a", 1)
    .ex("{?a:b}", "b", 0)
    .ex("a{=b:> 9}", "a        b")
    .ex("片{}", "片仮名", "仮名")
    .ex("{?{={?a:b{}}}}d", "bcd", 1, 0, "c")
    .ex("片{:> 5}", "片 仮名", "仮名")
    .ex("{=片{:> 5}:> 8}", " 片 仮名", "仮名")
)
