# Taken from https://git.sr.ht/~sircmpwn/ctools/tree/master/doc/meson.build

scdoc_prog = find_program(scdoc.get_pkgconfig_variable('scdoc'), native: true)
sh = find_program('sh', native: true)
mandir = get_option('mandir')

man_files = [
	'altprintf.1',
	'altprintf.3',
	'altprintf.7'
]

foreach page : man_files
	filename = page + '.scd'
	topic = filename.split('.')[-3].split('/')[-1]
	section = filename.split('.')[-2]
	output = '@0@.@1@'.format(topic, section)

	custom_target(
		output,
		input: filename,
		output: output,
		command: [
			sh, '-c', '@0@ < @INPUT@ > @1@'.format(
				scdoc_prog.path(),
				join_paths(meson.current_build_dir(), output))
		],
		install: true,
		install_dir: '@0@/man@1@'.format(mandir, section)
	)
endforeach
