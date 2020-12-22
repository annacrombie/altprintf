pymod = import('python')
python = pymod.find_installation('python3')
pyapf = python.extension_module('pyapf',
	include_directories: inc,
	install: false,
	sources: files(['pyapf.c']),
	link_with: [libapf],
	dependencies: python.dependency(),
)

test(
	'tests',
	find_program('./test'),
	args: [pyapf.full_path()],
	depends: [pyapf],
)
