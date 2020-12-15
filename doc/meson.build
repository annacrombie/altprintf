# Taken from https://git.sr.ht/~sircmpwn/ctools/tree/master/doc/meson.build

scdoc_prog = find_program(scdoc.get_pkgconfig_variable('scdoc'), native: true)
mandir = get_option('mandir')

sh = find_program('sh', native: true)

man_files = [
	['altprintf', '3', 'altprintf.3.scd'],
	['altprintf', '7', custom_target(
		'altprintf.7.scd',
		input: 'altprintf.7.scd.in',
		output: 'altprintf.7.scd',
		command: [find_program('./insert_grammar.py'), '@INPUT@'],
		capture: true,
	)]
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
		command: [
			sh, '-c', '@0@ < @INPUT@'.format(scdoc_prog.path())
		],
		capture: true,
		install: true,
		install_dir: '@0@/man@1@'.format(mandir, section)
	)
endforeach
