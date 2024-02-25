# Here be dragons

dot_gv = {
	'wayland-architecture': files('dot/wayland-architecture.gv'),
	'x-architecture': files('dot/x-architecture.gv'),
}

# This is a workaround for Meson's custom_target() directive, which
# currently does not support outputs pointing to a sub-directory
# XXX: try turning these into maps, so they can be indexed with picture name
dot_png = []
dot_map = []

doxygen_conf = configuration_data()
doxygen_conf.set('VERSION', meson.project_version())
doxygen_conf.set('top_builddir', meson.project_build_root())
wayland_doxygen = configure_file(
	input: 'wayland.doxygen.in',
	output: 'wayland.doxygen',
	configuration: doxygen_conf,
)

shared_files = files([
	'../../src/wayland-util.h',
])

client_files = files([
	'../../src/wayland-client.c',
	'../../src/wayland-client.h',
	'../../src/wayland-client-core.h',
])

server_files = files([
	'../../src/event-loop.c',
	'../../src/wayland-server.c',
	'../../src/wayland-server.h',
	'../../src/wayland-server-core.h',
	'../../src/wayland-shm.c',
])

cursor_files = files([
	'../../cursor/wayland-cursor.c',
	'../../cursor/wayland-cursor.h',
])

extra_client_files = [
	'mainpage.dox',
	wayland_client_protocol_h,
]

extra_server_files = [
	'mainpage.dox',
	wayland_server_protocol_h,
]

extra_cursor_files = [
	'mainpage.dox',
]

gen_doxygen = find_program('gen-doxygen.py')

subdir('xml')

formats = {
	'html': {
		'Client': shared_files + client_files + extra_client_files,
		'Server': shared_files + server_files + extra_server_files,
		'Cursor': shared_files + cursor_files + extra_cursor_files,
	},
}

foreach f_name, sections: formats
	foreach s_name, s_files: sections
		t_name = '@0@-@1@-doc'.format(f_name, s_name)

		# We do not really need an output file, but Meson
		# will complain if one is not set, so we use a
		# dummy 'stamp' file
		stamp = join_paths(meson.current_build_dir(), '@0@.stamp'.format(t_name))
		custom_target(
			t_name,
			command: [
				gen_doxygen,
				# XXX pass doxygen path as argument
				'--builddir=@OUTDIR@',
				'--section=@0@'.format(s_name),
				'--output-format=@0@'.format(f_name),
				'--stamp=@0@'.format(stamp),
				wayland_doxygen,
				'@INPUT@',
			],
			input: s_files,
			output: '@0@.stamp'.format(t_name),
			depends: [dot_png, dot_map],
			build_by_default: true,
		)
	endforeach
endforeach

man_files = shared_files + server_files + client_files + cursor_files
stamp = join_paths(meson.current_build_dir(), 'man3.stamp')
custom_target(
	'man-pages-3',
	command: [
		gen_doxygen,
		'--builddir=@OUTDIR@',
		'--output-format=man3',
		'--stamp=@0@'.format(stamp),
		wayland_doxygen,
		'@INPUT@',
	],
	input: man_files,
	output: 'man3',
	build_by_default: true,
	install: true,
	install_dir: join_paths(get_option('prefix'), get_option('mandir')),
)
