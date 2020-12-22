src = files([
	'args.c',
	'compile.c',
	'cswidth.c',
	'err.c',
	'fmt.c',
])
flags = []

if get_option('debug')
	src += files(['debug.c'])
else
	flags += '-DNDEBUG'
endif

install_headers(files(['../include/apf.h']))

libapf = library(
	'apf',
	c_args: flags,
	include_directories: inc,
	install: true,
	sources: src,
)

apf = executable(
	'apf',
	c_args: flags,
	include_directories: inc,
	install: false,
	link_with: [libapf],
	sources: files(['main.c']),
)
