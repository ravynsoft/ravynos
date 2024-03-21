if not (get_option('scanner') or get_option('libraries'))
	error('Either -Dscanner=true or -Dlibraries=true is required')
endif

wayland_version_h = configuration_data()
wayland_version_h.set('WAYLAND_VERSION', meson.project_version())
wayland_version_h.set('WAYLAND_VERSION_MAJOR', wayland_version[0].to_int())
wayland_version_h.set('WAYLAND_VERSION_MINOR', wayland_version[1].to_int())
wayland_version_h.set('WAYLAND_VERSION_MICRO', wayland_version[2].to_int())
configure_file(
	input: 'wayland-version.h.in',
	output: 'wayland-version.h',
	configuration: wayland_version_h,
	install: get_option('libraries'),
	install_dir: join_paths(get_option('prefix'), get_option('includedir'))
)


wayland_util = static_library(
	'wayland-util',
	sources: 'wayland-util.c'
)

wayland_util_dep = declare_dependency(
	link_with: wayland_util,
	include_directories: include_directories('.')
)

if get_option('scanner')
	# wayland-scanner

	scanner_deps = [ dependency('expat') ]
	scanner_args = [ '-include', 'config.h' ]

	if get_option('dtd_validation')
		scanner_deps += dependency('libxml-2.0')
		scanner_args += '-DHAVE_LIBXML=1'
	endif

	prog_embed = find_program('embed.py', native: true)

	embed_dtd = custom_target(
		'wayland.dtd.h',
		input: '../protocol/wayland.dtd',
		output: 'wayland.dtd.h',
		command: [ prog_embed, '@INPUT@', 'wayland_dtd' ],
		capture: true
	)

	wayland_scanner_sources = [ 'scanner.c', embed_dtd ]
	wayland_scanner_includes = [ root_inc, protocol_inc ]

	wayland_scanner = executable(
		'wayland-scanner',
		wayland_scanner_sources,
		c_args: scanner_args,
		include_directories: wayland_scanner_includes,
		dependencies: [ scanner_deps, wayland_util_dep, ],
		install: true
	)

	pkgconfig.generate(
		name: 'Wayland Scanner',
		description: 'Wayland scanner',
		version: meson.project_version(),
		variables: [
			'datarootdir=' + join_paths('${prefix}', get_option('datadir')),
			'pkgdatadir=' + join_paths('${datarootdir}', meson.project_name()),
			'bindir=' + join_paths('${prefix}', get_option('bindir')),
			'wayland_scanner=${bindir}/wayland-scanner'
		],
		filebase: 'wayland-scanner'
	)

	if meson.can_run_host_binaries()
		meson.override_find_program('wayland-scanner', wayland_scanner)
	endif
endif

if meson.is_cross_build() or not get_option('scanner')
	scanner_dep = dependency('wayland-scanner', native: true, version: meson.project_version())
	wayland_scanner_for_build = find_program(scanner_dep.get_variable(pkgconfig: 'wayland_scanner'))
else
	wayland_scanner_for_build = wayland_scanner
endif

if get_option('libraries')
	# wayland libraries

	mathlib_dep = cc.find_library('m', required: false)
	threads_dep = dependency('threads', required: false)

	wayland_private = static_library(
		'wayland-private',
		sources: [
			'connection.c',
			'wayland-os.c'
		],
		dependencies: [ epoll_dep, ffi_dep, rt_dep ]
	)

	wayland_private_dep = declare_dependency(
		link_with: wayland_private,
		include_directories: include_directories('.')
	)

	generated_headers = [
		{
			'scanner_args': ['server-header'],
			'output': 'wayland-server-protocol.h',
			'install': true,
		},
		{
			'scanner_args': ['server-header', '-c'],
			'output': 'wayland-server-protocol-core.h',
			'install': false,
		},
		{
			'scanner_args': ['client-header'],
			'output': 'wayland-client-protocol.h',
			'install': true,
		},
		{
			'scanner_args': ['client-header', '-c'],
			'output': 'wayland-client-protocol-core.h',
			'install': false,
		},
	]

	foreach gen: generated_headers
		scanner_args = gen['scanner_args']
		output_file = gen['output']
		install_file = gen['install']
		install_dir = join_paths(get_option('prefix'), get_option('includedir'))
		target_name = output_file.underscorify()

		target = custom_target(
			target_name,
			command: [
				wayland_scanner_for_build, '-s', scanner_args,
				'@INPUT@', '@OUTPUT@'
			],
			input: wayland_protocol_xml,
			output: output_file,
			install: install_file,
			install_dir: install_dir
		)

		set_variable(target_name, target)
	endforeach

	wayland_protocol_c = custom_target(
		'protocol source',
		command: [
			wayland_scanner_for_build, '-s', 'public-code', '@INPUT@', '@OUTPUT@'
		],
		input: wayland_protocol_xml,
		output: 'wayland-protocol.c'
	)

	if wayland_version[0] != '1'
		# The versioning used for the shared libraries assumes that the major
		# version of Wayland as a whole will increase to 2 if and only if there
		# is an ABI break, at which point we should probably bump the SONAME of
		# all libraries to .so.2. For more details see
		# https://gitlab.freedesktop.org/wayland/wayland/-/merge_requests/177
		error('We probably need to bump the SONAME of libwayland-server and -client')
	endif

	wayland_server = library(
		'wayland-server',
		sources: [
			wayland_server_protocol_core_h,
			wayland_server_protocol_h,
			wayland_protocol_c,
			'wayland-server.c',
			'wayland-shm.c',
			'event-loop.c'
		],
		# To avoid an unnecessary SONAME bump, wayland 1.x.y produces
		# libwayland-server.so.0.x.y.
		version: '.'.join(['0', wayland_version[1], wayland_version[2]]),
		dependencies: [
			epoll_dep,
			ffi_dep,
			wayland_private_dep,
			wayland_util_dep,
			mathlib_dep,
			threads_dep,
			rt_dep
		],
		include_directories: root_inc,
		install: true
	)

	wayland_server_dep = declare_dependency(
		link_with: wayland_server,
		include_directories: [ root_inc, include_directories('.') ],
		dependencies: [ epoll_dep, ffi_dep, mathlib_dep, threads_dep ],
		sources: [
			wayland_server_protocol_core_h,
			wayland_server_protocol_h
		]
	)

	pkgconfig.generate(
		wayland_server,
		name: 'Wayland Server',
		description: 'Server side implementation of the Wayland protocol',
		version: meson.project_version(),
		filebase: 'wayland-server',
		variables: [
			'datarootdir=' + join_paths('${prefix}', get_option('datadir')),
			'pkgdatadir=' + join_paths('${datarootdir}', meson.project_name())
		]
	)

	if meson.version().version_compare('>= 0.54.0')
		meson.override_dependency('wayland-server', wayland_server_dep)
	endif

	wayland_client = library(
		'wayland-client',
		sources: [
			wayland_client_protocol_core_h,
			wayland_client_protocol_h,
			wayland_protocol_c,
			'wayland-client.c'
		],
		# To avoid an unnecessary SONAME bump, wayland 1.x.y produces
		# libwayland-client.so.0.x.y.
		version: '.'.join(['0', wayland_version[1], wayland_version[2]]),
		dependencies: [
			epoll_dep,
			ffi_dep,
			wayland_private_dep,
			wayland_util_dep,
			mathlib_dep,
			threads_dep
		],
		include_directories: root_inc,
		install: true
	)

	pkgconfig.generate(
		wayland_client,
		name: 'Wayland Client',
		description: 'Wayland client side library',
		version: meson.project_version(),
		filebase: 'wayland-client',
		variables: [
			'datarootdir=' + join_paths('${prefix}', get_option('datadir')),
			'pkgdatadir=' + join_paths('${datarootdir}', meson.project_name())
		]
	)

	wayland_client_dep = declare_dependency(
		link_with: wayland_client,
		include_directories: [ root_inc, include_directories('.') ],
		sources: [
			wayland_client_protocol_core_h,
			wayland_client_protocol_h
		]
	)

	if meson.version().version_compare('>= 0.54.0')
		meson.override_dependency('wayland-client', wayland_client_dep)
	endif

	install_headers([
		'wayland-util.h',
		'wayland-server.h',
		'wayland-server-core.h',
		'wayland-client.h',
		'wayland-client-core.h',
	])
endif
