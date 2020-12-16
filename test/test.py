import subprocess


def apf_test(cmd, in_, out, *args):
    args = list(map(lambda x: str(x), args))
    in_ = in_.encode("utf-8")

    result = subprocess.run(
        [cmd, in_, *args],
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

    if out is None:
        if result.returncode != 0:
            print(f"test failed: '{in_}'")
            print(stderr)
            return False
    elif res != out:
        print(f"test failed: '{in_}'")
        print(stderr)
        print(f"got:      '{res}'")
        print(f"expected: '{out}'")

        return False

    return True
