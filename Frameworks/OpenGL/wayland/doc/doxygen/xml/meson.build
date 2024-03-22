# dot_png: list of PNG targets
# dot_map: list of MAP targets
foreach name, infile: dot_gv
	dot_png += custom_target(
		name + '.png',
		command: [ dot, '-Tpng', '-o@OUTPUT@', '@INPUT@' ],
		input: infile,
		output: name + '.png',
		install: true,
		install_dir: join_paths(publican_install_prefix, publican_html_dir, 'images')
	)

	dot_map += custom_target(
		name + '.map',
		command: [ dot, '-Tcmapx_np', '-o@OUTPUT@', '@INPUT@' ],
		input: infile,
		output: name + '.map',
	)
endforeach

subdir('Client')
subdir('Server')
