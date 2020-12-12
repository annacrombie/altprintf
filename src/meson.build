src = files([
	'args.c',
	'cswidth.c',
	'debug.c', #TODO: only for debug builds
	'err.c',
	'fmt.c',
	'parse.c',
])

libapf = library('apf', src, include_directories: inc, install: true)
install_headers(files(['../include/apf.h']))

apf = executable(
	'apf',
	link_with: [libapf],
	include_directories: inc,
	sources: files(['main.c']),
	install: false
)
