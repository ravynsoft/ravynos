project('libevdev', 'c',
	version: '1.11.0', # change autotools version too
	license: 'MIT/Expat',
	default_options: [ 'c_std=gnu99', 'warning_level=2' ],
	meson_version: '>= 0.47.0')

libevdev_version = meson.project_version().split('.')

dir_src         = join_paths(meson.source_root(), 'libevdev')
dir_src_test    = join_paths(meson.source_root(), 'test')

# Include directories
includes_include = include_directories('include')

# DO NOT MODIFY THIS
# Use symbol versioning instead.
libevdev_lt_c=5
libevdev_lt_r=0
libevdev_lt_a=3

# convert to soname
libevdev_so_version = '@0@.@1@.@2@'.format((libevdev_lt_c - libevdev_lt_a),
					   libevdev_lt_a, libevdev_lt_r)


# Compiler setup
cc = meson.get_compiler('c')
cppflags = ['-Wno-unused-parameter', '-fvisibility=hidden']
cflags = cppflags + ['-Wmissing-prototypes', '-Wstrict-prototypes']
add_project_arguments(cflags, language: 'c')
add_project_arguments(cppflags, language: 'cpp')

# config.h
config_h = configuration_data()
config_h.set('_GNU_SOURCE', '1')

# Dependencies
pkgconfig = import('pkgconfig')
dep_lm = cc.find_library('m')

input_h = join_paths(meson.source_root(), 'include', 'linux', host_machine.system(), 'input.h')
uinput_h = join_paths(meson.source_root(), 'include', 'linux', host_machine.system(), 'uinput.h')
input_event_codes_h = join_paths(meson.source_root(), 'include', 'linux', host_machine.system(), 'input-event-codes.h')

# event-names.h
make_event_names = find_program('libevdev/make-event-names.py')
event_names_h = configure_file(input: 'libevdev/libevdev.h',
			       output: 'event-names.h',
			       command: [make_event_names, input_h, input_event_codes_h],
			       capture: true)


# libevdev.so
install_headers('libevdev/libevdev.h',
		'libevdev/libevdev-uinput.h',
		subdir: 'libevdev-1.0/libevdev')
src_libevdev = [
	event_names_h,
	'libevdev/libevdev.h',
	'libevdev/libevdev-int.h',
	'libevdev/libevdev-util.h',
	'libevdev/libevdev-uinput.c',
	'libevdev/libevdev-uinput.h',
	'libevdev/libevdev-uinput-int.h',
	'libevdev/libevdev.c',
	'libevdev/libevdev-names.c',
	'include/linux/input.h',
	'include/linux/uinput.h',
	input_event_codes_h,
	input_h,
	uinput_h
]

mapfile = join_paths(dir_src, 'libevdev.sym')
version_flag = '-Wl,--version-script,@0@'.format(mapfile)
lib_libevdev = library('evdev',
	src_libevdev,
	include_directories: [includes_include],
	dependencies: [],
	version: libevdev_so_version,
	link_args: version_flag,
	link_depends: mapfile,
	install: true
)

dep_libevdev = declare_dependency(link_with: lib_libevdev)

pkgconfig.generate(
	filebase: 'libevdev',
	name: 'libevdev',
	description: 'Handler library for evdev events',
	version: meson.project_version(),
	libraries: lib_libevdev,
	subdirs: 'libevdev-1.0',
)

man_config = configuration_data()
man_config.set('PACKAGE_VERSION', meson.project_version())
manpage = configure_file(input: 'doc/libevdev.man.in',
                         output: 'libevdev.3',
                         configuration: man_config)
install_man(manpage)


# tools
executable('libevdev-events',
	   sources: ['tools/libevdev-events.c'],
	   include_directories: [includes_include],
	   dependencies: dep_libevdev,
	   install: false)
executable('touchpad-edge-detector',
	   sources: ['tools/touchpad-edge-detector.c'],
	   include_directories: [includes_include],
	   dependencies: [dep_libevdev, dep_lm],
           install: true)
executable('mouse-dpi-tool',
	   sources: ['tools/mouse-dpi-tool.c'],
	   include_directories: [includes_include],
	   dependencies: dep_libevdev,
           install: true)
executable('libevdev-tweak-device',
	   sources: ['tools/libevdev-tweak-device.c'],
	   include_directories: [includes_include],
	   dependencies: dep_libevdev,
	   install: true)
install_man('tools/libevdev-tweak-device.1',
            'tools/touchpad-edge-detector.1')

# tests
dep_check = dependency('check', version: '>= 0.9.9',
		       required: get_option('tests'))
if dep_check.found()
	executable('test-link',
		   sources: ['test/test-link.c'],
		   include_directories: [includes_include],
		   dependencies: dep_libevdev,
		   install: false)
	executable('test-compile-pedantic',
		   sources: ['test/test-compile-pedantic.c'],
		   c_args: ['-pedantic', '-Werror', '-std=c89'],
		   include_directories: [includes_include],
		   dependencies: dep_libevdev,
		   install: false)

	src_common = [
		'test/test-common-uinput.c',
		'test/test-common-uinput.h',
		'test/test-common.c',
		'test/test-common.h',
		'test/test-main.c',
	]
	test_event_codes = executable('test-event-codes',
				      sources: src_common + [
					'test/test-event-codes.c',
					'test/test-event-names.c',
					'test/test-context.c',
				      ],
				      include_directories: [includes_include],
				      dependencies: [dep_libevdev, dep_check],
				      install: false)
	test('test-event-codes', test_event_codes, suite: 'library')

	test_internals = executable('test-internals',
				    sources: src_common + [
					'test/test-int-queue.c',
				    ],
				    include_directories: [includes_include],
				    dependencies: [dep_libevdev, dep_check],
				    install: false)
	test('test-internals', test_internals, suite: 'library')

	test_uinput = executable('test-uinput',
				 sources: src_common + [
					'test/test-uinput.c',
				 ],
				 include_directories: [includes_include],
				 dependencies: [dep_libevdev, dep_check],
				 install: false)
	test('test-uinput', test_uinput, suite: 'library')

	test_libevdev = executable('test-libevdev',
				   sources: src_common + [
					'test/test-libevdev-init.c',
					'test/test-libevdev-has-event.c',
					'test/test-libevdev-events.c',
				   ],
				   include_directories: [includes_include],
				   dependencies: [dep_libevdev, dep_check],
				   install: false)
	test('test-libevdev', test_libevdev, suite: 'library')

	test_kernel = executable('test-kernel',
				 sources: src_common + [
					'test/test-kernel.c',
				 ],
				 include_directories: [includes_include],
				 dependencies: [dep_libevdev, dep_check],
				 install: false)
	test('test-kernel', test_kernel, suite: 'kernel')


	valgrind = find_program('valgrind', required: false)
	if valgrind.found()
		valgrind_env = environment()
		valgrind_env.set('CK_TIMEOUT_MULTIPLIER', '10')
		valgrind_env.set('CK_FORK', 'no')
		valgrind_env.set('RUNNING_ON_VALGRIND', '1')
		valgrind_suppressions_file = join_paths(dir_src_test, 'valgrind.suppressions')
		add_test_setup('valgrind',
				exe_wrapper: [ valgrind,
						'--leak-check=full',
						'--gen-suppressions=all',
						'--error-exitcode=3',
						'--suppressions=' + valgrind_suppressions_file ],
				env:  valgrind_env,
				timeout_multiplier: 100)
	else
		message('valgrind not found, disabling valgrind test suite')
	endif

	test_static_link = find_program('test/test-static-symbols-leak.sh')
	test('static-symbols-leak', test_static_link,
	     args: [meson.current_build_dir()],
	     suite: 'static')
endif

doxygen = find_program('doxygen', required: get_option('documentation'))
if doxygen.found()
	doxygen = find_program('doxygen')

	src_doxygen = files(
		# source files
		join_paths(dir_src, 'libevdev.h'),
		join_paths(dir_src, 'libevdev-uinput.h'),
		# style files
		'doc/style/bootstrap.css',
		'doc/style/customdoxygen.css',
		'doc/style/doxy-boot.js',
		'doc/style/dynsections.js',
		'doc/style/footer.html',
		'doc/style/header.html',
		'doc/style/layout.xml',
		'doc/style/libevdevdoxygen.css',
		'doc/style/LICENSE',
		'doc/style/README.md',
		'doc/style/style.css',
	)
	doxyfiles = []
	foreach f: src_doxygen
		df = configure_file(input: f,
				    output: '@PLAINNAME@',
				    copy: true)
		doxyfiles += [df]
	endforeach

	doc_config = configuration_data()
	doc_config.set('PACKAGE_NAME', meson.project_name())
	doc_config.set('PACKAGE_VERSION', meson.project_version())
	doc_config.set('builddir', meson.current_build_dir())
	doc_config.set('top_srcdir', meson.source_root())
	doc_config.set('srcdir', join_paths(meson.source_root(), 'doc'))

	doxyfile = configure_file(input: 'doc/libevdev.doxygen.in',
				  output: 'libevdev.doxygen',
				  configuration: doc_config)
	custom_target('doxygen',
		      input: [doxyfiles, doxyfile] + src_doxygen,
		      output: ['.'],
		      command: [doxygen, doxyfile],
		      install: false,
		      build_by_default: true)
endif

# Coverity breaks because it doesn't define _Float128 correctly, you'll end
# up with a bunch of messages in the form:
# "/usr/include/stdlib.h", line 133: error #20: identifier "_Float128" is
#           undefined
#   extern _Float128 strtof128 (const char *__restrict __nptr,
#          ^
# We don't use float128 ourselves, it gets pulled in from math.h or
# something, so let's just define it as uint128 and move on.
# Unfortunately we can't detect the coverity build at meson configure
# time, we only know it fails at runtime. So make this an option instead, to
# be removed when coverity fixes this again.
if get_option('coverity')
        config_h.set('_Float128', '__uint128_t')
        config_h.set('_Float32', 'int')
        config_h.set('_Float32x', 'int')
        config_h.set('_Float64', 'long')
        config_h.set('_Float64x', 'long')
endif

############ output files ############
configure_file(output: 'config.h', configuration: config_h)
