src = files(
    'args.c',
    'compile.c',
    'cswidth.c',
    'err.c',
    'fmt.c',
)
flags = []

if get_option('debug')
    src += files('debug.c')
else
    flags += '-DNDEBUG'
endif

install_headers(files('../include/apf.h'))

altprintf_dep = declare_dependency(
    link_with: library(
        'apf',
        c_args: flags,
        include_directories: inc,
        install: true,
        sources: src,
    ),
    include_directories: inc,
)

executable(
    'apf',
    c_args: flags,
    include_directories: inc,
    install: false,
    sources: files('main.c'),
    dependencies: altprintf_dep,
)
