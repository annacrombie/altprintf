project(
	'libaltprintf',
	'c',
	version: '0.1.0',
	license: 'GPL-3.0-only',
	meson_version: '>=0.48.0',
	default_options: [
		'c_std=c99',
		'warning_level=2',
		'default_library=static'
	],
)


cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wundef',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wold-style-definition',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',

	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
]), language: 'c')

math_lib = cc.find_library('m', required : true)

lib_src = files([
	'src/parsef.c',
	'src/strbuf.c',
	'src/fmt.c',
	'src/fmte.c'
])

inc = include_directories(['include'])

libaltprintf = library('altprintf', lib_src, dependencies: math_lib, include_directories: inc, install: true)
install_headers(
	files([
		'include/altprintf/altprintf.h',
		'include/altprintf/enums.h',
		'include/altprintf/fmt.h',
		'include/altprintf/fmte.h',
		'include/altprintf/log.h',
		'include/altprintf/parsef.h',
		'include/altprintf/strbuf.h',
		'include/altprintf/syntax.h',
	]),
	subdir: 'include'
)
altprintf_dep = declare_dependency(
	include_directories: inc,
	link_with: libaltprintf
)
