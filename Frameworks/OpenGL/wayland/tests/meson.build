if not get_option('libraries')
	error('-Dtests=true requires -Dlibraries=true')
endif

test_runner = static_library(
	'test-runner',
	sources: [
		'test-runner.c',
		'test-helpers.c',
		'test-compositor.c'
	],
	include_directories: [ root_inc, src_inc ],
	dependencies: [
		cc.find_library('dl', required: false),
		dependency('threads'),
		epoll_dep,
		ffi_dep,
		wayland_util_dep,
		wayland_private_dep,
		wayland_client_dep,
		wayland_server_dep
	]
)

test_runner_dep = declare_dependency(
	link_with: test_runner,
	include_directories: [ src_inc ],
	dependencies: [
		dependency('threads'),
		cc.find_library('dl', required: false)
	]
)

tests_protocol_xml = files('../protocol/tests.xml')

tests_server_protocol_h = custom_target(
	'test server protocol header',
	command: [ wayland_scanner_for_build, '-s', 'server-header', '@INPUT@', '@OUTPUT@' ],
	input: tests_protocol_xml,
	output: 'tests-server-protocol.h'
)

tests_client_protocol_c = custom_target(
	'test client protocol header',
	command: [ wayland_scanner_for_build, '-s', 'client-header', '@INPUT@', '@OUTPUT@' ],
	input: tests_protocol_xml,
	output: 'tests-client-protocol.h'
)

tests_protocol_c = custom_target(
	'test protocol source',
	command: [ wayland_scanner_for_build, '-s', 'public-code', '@INPUT@', '@OUTPUT@' ],
	input: tests_protocol_xml,
	output: 'tests-protocol.c'
)

benchmark(
	'fixed-benchmark',
	executable(
		'fixed-benchmark',
		'fixed-benchmark.c',
		dependencies: [ test_runner_dep, rt_dep ]
	)
)

executable(
	'exec-fd-leak-checker',
	'exec-fd-leak-checker.c',
	dependencies: test_runner_dep
)

if add_languages('cpp', native: false)
	test(
		'cpp-compile-test',
		executable(
			'cpp-compile-test',
			'cpp-compile-test.cpp',
			wayland_server_protocol_h,
			include_directories: src_inc
		)
	)
endif

sed_path = find_program('sed').full_path()

if get_option('scanner')
	test(
		'scanner-test',
		find_program('scanner-test.sh'),
		env: [
			'TEST_DATA_DIR=@0@/data'.format(meson.current_source_dir()),
			'TEST_OUTPUT_DIR=@0@/output'.format(meson.current_build_dir()),
			'SED=@0@'.format(sed_path),
			'WAYLAND_SCANNER=@0@'.format(wayland_scanner.full_path()),
		],
	)
endif

tests = {
	'array-test': [],
	'client-test': [ wayland_server_protocol_h ],
	'display-test': [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
		tests_server_protocol_h,
		tests_client_protocol_c,
		tests_protocol_c,
	],
	'connection-test': [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
	],
	'event-loop-test': [ wayland_server_protocol_h ],
	'fixed-test': [],
	'interface-test': [ wayland_client_protocol_h ],
	'list-test': [],
	'map-test': [],
	'sanity-test' : [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
	],
	'socket-test': [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
	],
	'queue-test': [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
	],
	'signal-test': [ wayland_server_protocol_h ],
	'newsignal-test': [
		# wayland-server.c is needed here to access wl_priv_* functions
		files('../src/wayland-server.c'),
		wayland_server_protocol_h,
	],
	'resources-test': [ wayland_server_protocol_h ],
	'message-test': [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
	],
	'compositor-introspection-test': [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
	],
	'protocol-logger-test': [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
	],
	'headers-test': [
		wayland_client_protocol_h,
		wayland_server_protocol_h,
		'headers-protocol-test.c',
		wayland_client_protocol_core_h,
		wayland_server_protocol_core_h,
		'headers-protocol-core-test.c',
	],
	'os-wrappers-test': [],
}

foreach test_name, test_extra_sources: tests
	test_sources = [ test_name + '.c' ] + test_extra_sources
	test_deps = [test_runner_dep, epoll_dep]
	bin = executable(test_name, test_sources, dependencies: test_deps)
	test(
		test_name,
		bin,
		env: [
			'TEST_SRC_DIR=@0@'.format(meson.current_source_dir()),
			'TEST_BUILD_DIR=@0@'.format(meson.current_build_dir()),
		],
	)
endforeach
