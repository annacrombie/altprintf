pymod = import('python')
python = pymod.find_installation('python3')
pyapf = python.extension_module(
    'pyapf',
    install: false,
    sources: files('pyapf.c'),
    dependencies: [altprintf_dep, python.dependency()],
)

test(
    'tests',
    find_program('./test'),
    args: [pyapf.full_path()],
    depends: [pyapf],
)
