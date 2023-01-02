scdoc_prog = find_program(scdoc.get_pkgconfig_variable('scdoc'), native: true)
mandir = get_option('mandir')

sh = find_program('sh', native: true)

example = executable(
    'example',
    install: false,
    sources: files('example.c'),
    dependencies: altprintf_dep,
)

test(
    'example.c',
    find_program('./example_test.py'),
    args: [example.full_path()],
    depends: [example],
)

inserter = find_program('./insert.py')

man_files = [
    [
        'altprintf',
        '3',
        custom_target(
            'altprintf.3.scd',
            input: ['altprintf.3.scd.in', 'example.c'],
            output: 'altprintf.3.scd',
            command: [inserter, 'example', '@INPUT0@', '@INPUT1@'],
            capture: true,
        ),
    ],
    [
        'altprintf',
        '7',
        custom_target(
            'altprintf.7.scd',
            input: ['altprintf.7.scd.in', '../test/apf_grammar.py'],
            output: 'altprintf.7.scd',
            command: [inserter, 'grammar', '@INPUT@'],
            capture: true,
        ),
    ],
]

foreach page_spec : man_files
    topic = page_spec[0]
    section = page_spec[1]
    source = page_spec[2]
    output = '@0@.@1@'.format(topic, section)

    custom_target(
        output,
        input: source,
        output: output,
        command: [sh, '-c', '@0@ < @INPUT@'.format(scdoc_prog.path())],
        capture: true,
        install: true,
        install_dir: '@0@/man@1@'.format(mandir, section),
    )
endforeach
