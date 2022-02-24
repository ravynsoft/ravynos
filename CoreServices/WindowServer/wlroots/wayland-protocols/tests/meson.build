prog_scan_sh = find_program('scan.sh')
dep_scanner = dependency('wayland-scanner', native: true)
prog_scanner = find_program(dep_scanner.get_pkgconfig_variable('wayland_scanner'))

libwayland = [
	dependency('wayland-client'),
	dependency('wayland-server'),
]

# Check that each protocol passes through the scanner
foreach protocol_file : protocol_files
	protocol_path = join_paths(wayland_protocols_srcdir, protocol_file)
	test_name = 'scan-@0@'.format(protocol_file.underscorify())
	test(test_name, prog_scan_sh,
		args: protocol_path,
		env: [
			'SCANNER=@0@'.format(prog_scanner.path()),
		]
	)
endforeach

# Check buildability

add_languages('c', 'cpp', native: false)
replace = find_program('replace.py')

extra_linker_flags = meson.get_compiler('c').get_supported_link_arguments([
	'-Wl,--unresolved-symbols=ignore-all',
])

foreach protocol_file : protocol_files
	xml_file = fs.name(protocol_file)
	xml_components = xml_file.split('.')
	protocol_base_file_name = xml_components[0]

	protocol_path = files(join_paths(wayland_protocols_srcdir, protocol_file))
	client_header_path = '@0@-client.h'.format(protocol_base_file_name)
	server_header_path = '@0@-server.h'.format(protocol_base_file_name)
	code_path = '@0@-code.c'.format(protocol_base_file_name)
	client_header = custom_target(
		client_header_path,
		output: client_header_path,
		input: protocol_path,
		command: [
			prog_scanner,
			'--strict',
			'client-header',
			'@INPUT@',
			'@OUTPUT@',
		],
		install: false,
	)
	server_header = custom_target(
		server_header_path,
		output: server_header_path,
		input: protocol_path,
		command: [
			prog_scanner,
			'--strict',
			'server-header',
			'@INPUT@',
			'@OUTPUT@',
		],
		install: false,
	)
	code = custom_target(
		code_path,
		output: code_path,
		input: protocol_path,
		command: [
			prog_scanner,
			'--strict',
			'private-code',
			'@INPUT@',
			'@OUTPUT@',
		],
		install: false,
	)

	replace_command = [
		replace,
		'@INPUT@',
		'@OUTPUT@',
		'PROTOCOL_CLIENT_INCLUDE_FILE',
		client_header.full_path(),
		'PROTOCOL_SERVER_INCLUDE_FILE',
		server_header.full_path(),
	]

	# Check that header can be included by a pedantic C99 compiler
	test_name = 'test-build-pedantic-@0@'.format(protocol_file.underscorify())
	test_name_source = '@0@.c'.format(test_name)
	test_source = custom_target(
		test_name_source,
		input: 'build-pedantic.c.in',
		output: test_name_source,
		command: replace_command,
	)
	pedantic_test_executable = executable(
		test_name,
		[
			test_source,
			client_header,
			server_header,
			code
		],
		link_args: extra_linker_flags,
		dependencies: libwayland,
		c_args: [
			'-std=c99',
			'-pedantic',
			'-Wall',
			'-Werror' ],
		install: false,
	)
	test(test_name, pedantic_test_executable)

	# Check that the header
	if not protocol_file.contains('xdg-foreign-unstable-v1')
		test_name = 'test-build-cxx-@0@'.format(protocol_file.underscorify())
		test_name_source = '@0@.cc'.format(test_name)
		test_source = custom_target(
			test_name_source,
			input: 'build-cxx.cc.in',
			output: test_name_source,
			command: replace_command,
		)
		cxx_test_executable = executable(
			test_name,
			[
				test_source,
				client_header,
				server_header,
			],
			link_args: extra_linker_flags,
			dependencies: libwayland,
			cpp_args: [
				'-Wall',
				'-Werror',
			],
			install: false,
		)
		test(test_name, cxx_test_executable)
	endif
endforeach
