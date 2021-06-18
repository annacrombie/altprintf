project(
	'altprintf',
	'c',
	version: '0.5.0',
	license: 'MIT',
	meson_version: '>=0.54.0',
	default_options: [
		'c_std=c11',
		'warning_level=3',
		'default_library=static',
	],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wno-missing-braces',
	'-Wendif-labels',
	'-Wimplicit-fallthrough=2',
	'-Winit-self',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
	'-Wold-style-definition',
	'-Woverflow',
	'-Wpointer-arith',
	'-Wstrict-aliasing=2',
	'-Wstrict-prototypes',
	'-Wundef',
	'-fstrict-aliasing',
]), language: 'c')

inc = include_directories(['include'])

subdir('src')

#subdir('test')

scdoc = dependency('scdoc', native: true, required: get_option('man-pages'),
	version: '>=1.9.0')
if scdoc.found()
	subdir('doc')
endif
