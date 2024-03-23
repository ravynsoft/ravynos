project(
	'seatd',
	'c',
	version: '0.8.0',
	license: 'MIT',
	meson_version: '>=0.60.0',
	default_options: [
		'c_std=c11',
		'warning_level=3',
		'werror=true',
	],
)

# Bump whenever ABI-breaking changes occur.
libseat_soversion = 1

defaultpath = get_option('defaultpath')
if defaultpath == ''
	defaultpath = '/var/run/seatd.sock'
	if target_machine.system() == 'linux'
	   defaultpath = '/run/seatd.sock'
	endif
endif

seatdpath = get_option('prefix') / get_option('bindir') / 'seatd'

cc = meson.get_compiler('c')

add_project_arguments(
	[
		'-D_XOPEN_SOURCE=700',
		'-D__BSD_VISIBLE',
		'-D_NETBSD_SOURCE',
		'-DSEATD_VERSION="@0@"'.format(meson.project_version()),
		'-DSEATD_DEFAULTPATH="@0@"'.format(defaultpath),
		'-DSEATD_INSTALLPATH="@0@"'.format(seatdpath),
	],
	language: 'c',
)

add_project_arguments(cc.get_supported_arguments(
	[
		'-Wundef',
		'-Wunused',
		'-Wlogical-op',
		'-Wmissing-include-dirs',
		'-Wold-style-definition', # nop
		'-Wpointer-arith',
		'-Wstrict-prototypes',
		'-Wimplicit-fallthrough',
		'-Wmissing-prototypes',
		'-Wno-unknown-warning-option',
		'-Wno-unused-command-line-argument',
		'-Wvla',
	]),
	language: 'c',
)

add_project_arguments(cc.get_supported_link_arguments(
	[
		'-Wl,--exclude-libs=ALL',
	]),
	language: 'c',
)

if ['debugoptimized', 'release', 'minsize'].contains(get_option('buildtype'))
	add_project_arguments('-D_FORTIFY_SOURCE=2', language: 'c')
endif

if get_option('buildtype').startswith('debug')
	add_project_arguments('-DDEBUG', language : 'c')
endif

# Hacks
source_root = meson.current_source_dir().split('/')
build_root = meson.global_build_root().split('/')
relative_dir_parts = []
i = 0
in_prefix = true
foreach p : build_root
	if i >= source_root.length() or not in_prefix or p != source_root[i]
		in_prefix = false
		relative_dir_parts += '..'
	endif
	i += 1
endforeach
i = 0
in_prefix = true
foreach p : source_root
	if i >= build_root.length() or not in_prefix or build_root[i] != p
		in_prefix = false
		relative_dir_parts += p
	endif
	i += 1
endforeach

add_project_arguments(
	'-DREL_SRC_DIR="@0@"'.format(join_paths(relative_dir_parts) + '/'),
	language: 'c',
)

private_files = [
	'common/connection.c',
	'common/linked_list.c',
	'common/log.c',
]

private_deps = []

server_files = [
	'common/log.c',
	'common/linked_list.c',
	'common/terminal.c',
	'common/connection.c',
	'common/evdev.c',
	'common/drm.c',
	'common/wscons.c',
	'seatd/poller.c',
	'seatd/seat.c',
	'seatd/client.c',
	'seatd/server.c',
]

with_seatd = get_option('libseat-seatd') == 'enabled'
with_builtin = get_option('libseat-builtin') == 'enabled'
with_server = get_option('server') == 'enabled'

if with_seatd or with_builtin
	private_files += 'libseat/backend/seatd.c'
endif

libseat_c_args = ['-DLIBSEAT=1']

if with_seatd
	libseat_c_args += '-DSEATD_ENABLED=1'
endif

logind = disabler()
if get_option('libseat-logind') != 'disabled'
	if get_option('libseat-logind') == 'auto' and get_option('auto_features').disabled()
		# Disable logind
	elif get_option('libseat-logind') == 'auto'
		assert(get_option('auto_features').auto(), '-Dlibseat-logind must be set to systemd or elogind since auto_features != auto')
		logind = dependency(['libelogind', 'libsystemd'], required: false)
	else
		logind = dependency('lib@0@'.format(get_option('libseat-logind')))
	endif

	if logind.found()
		libseat_c_args += '-DLOGIND_ENABLED=1'
		libseat_c_args += '-DHAVE_@0@=1'.format(logind.name().to_upper())
		private_files += [
			'libseat/backend/logind.c',
			'common/drm.c',
		]
		private_deps += logind
	endif
endif

# needed for cross-compilation
realtime = meson.get_compiler('c').find_library('rt')
private_deps += realtime

if with_builtin
	libseat_c_args += '-DBUILTIN_ENABLED=1'
	private_files += server_files
endif

private_lib = static_library(
	'seat-private',
	private_files,
	dependencies: private_deps,
	include_directories: [include_directories('.', 'include')],
	c_args: libseat_c_args,
)

symbols_file = 'libseat/libseat.syms'
symbols_flag = '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), symbols_file)
lib = library(
	'seat', # This results in the library being called 'libseat'
	[ 'libseat/libseat.c', 'libseat/backend/noop.c' ],
	soversion: '@0@'.format(libseat_soversion),
	link_with: private_lib,
	include_directories: [include_directories('.', 'include')],
	install: true,
	link_args: symbols_flag,
	link_depends: symbols_file,
	c_args: libseat_c_args,
)

install_headers('include/libseat.h')

libseat_vars = {
	'have_seatd': with_seatd.to_string(),
	'have_logind': logind.found().to_string(),
	'have_builtin': with_builtin.to_string(),
}

pkgconfig = import('pkgconfig')
pkgconfig.generate(lib,
	version: meson.project_version(),
	filebase: 'libseat',
	name: 'libseat',
	description: 'Seat management library',
	variables: libseat_vars,
)

libseat = declare_dependency(
	link_with: lib,
	dependencies: private_deps,
	include_directories: include_directories('include', is_system: true),
	variables: libseat_vars,
)

meson.override_dependency('libseat', libseat)

if with_server
	executable(
		'seatd',
		[ server_files, 'seatd/seatd.c' ],
		include_directories: [include_directories('.', 'include')],
		install: true,
		dependencies: [realtime],
	)
	executable(
		'seatd-launch',
		[ 'seatd-launch/seatd-launch.c' ],
		include_directories: [include_directories('.', 'include')],
		install: true,
		dependencies: [realtime],
	)
endif

if get_option('examples') == 'enabled'
	executable(
		'simpletest',
		['examples/simpletest/main.c'],
		link_with: [lib],
		include_directories: [include_directories('.', 'include')],
		install: false,
	)
endif

tests = {
	'linked_list': ['common/linked_list.c'],
	'connection': ['common/connection.c'],
	'poller': ['common/linked_list.c', 'seatd/poller.c'],
}

foreach name, value : tests
	test(name, executable(
		'@0@_test'.format(name),
		['tests/@0@.c'.format(name), value],
		include_directories: [include_directories('.', 'include')]))
endforeach

if with_server
	scdoc = dependency('scdoc', required: get_option('man-pages'), version: '>= 1.9.7', native: true)
else
	scdoc = disabler()
endif

if scdoc.found()
	mandir = get_option('mandir')

	foreach src : ['seatd.1.scd', 'seatd-launch.1.scd']
		topic = src.split('.')[0]
		section = src.split('.')[1]
		output = '@0@.@1@'.format(topic, section)

		custom_target(
			output,
			input: 'man/' + src,
			output: output,
			command: scdoc.get_variable(pkgconfig: 'scdoc'),
			feed: true,
			capture: true,
			install: true,
			install_dir: '@0@/man@1@'.format(mandir, section)
		)
	endforeach
endif

summary({
	'libseat-seatd': with_seatd,
	'libseat-builtin': with_builtin,
	'libseat-systemd': logind.found() and logind.name() == 'libsystemd',
	'libseat-elogind': logind.found() and logind.name() == 'libelogind',
	'server': with_server,
}, bool_yn: true)
