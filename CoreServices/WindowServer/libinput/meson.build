project('libinput', 'c',
	version : '1.25.0',
	license : 'MIT/Expat',
	default_options : [ 'c_std=gnu99', 'warning_level=2' ],
	meson_version : '>= 0.56.0')

libinput_version = meson.project_version().split('.')

dir_data        = get_option('prefix') / get_option('datadir') / 'libinput'
dir_etc         = get_option('prefix') / get_option('sysconfdir')
dir_overrides   = get_option('prefix') / get_option('sysconfdir') / 'libinput'
dir_libexec     = get_option('prefix') / get_option('libexecdir') / 'libinput'
dir_lib         = get_option('prefix') / get_option('libdir')
dir_man1        = get_option('prefix') / get_option('mandir') / 'man1'
dir_system_udev = get_option('prefix') / 'lib' / 'udev'
dir_src_quirks  = meson.current_source_dir() / 'quirks'
dir_src_test    = meson.current_source_dir() / 'test'
dir_src         = meson.current_source_dir() / 'src'
dir_gitlab_ci	= meson.current_source_dir() / '.gitlab-ci'

dir_udev = get_option('udev-dir')
if dir_udev == ''
	dir_udev = dir_system_udev
endif
dir_udev_callouts = dir_udev
dir_udev_rules    = dir_udev / 'rules.d'

# Collection of man pages, we'll append to that
src_man = files()

# We use libtool-version numbers because it's easier to understand.
# Before making a release, the libinput_so_*
# numbers should be modified. The components are of the form C:R:A.
# a) If binary compatibility has been broken (eg removed or changed interfaces)
#    change to C+1:0:0.
# b) If interfaces have been changed or added, but binary compatibility has
#    been preserved, change to C+1:0:A+1
# c) If the interface is the same as the previous version, change to C:R+1:A
libinput_lt_c=23
libinput_lt_r=0
libinput_lt_a=13

# convert to soname
libinput_so_version = '@0@.@1@.@2@'.format((libinput_lt_c - libinput_lt_a),
                                            libinput_lt_a, libinput_lt_r)

# Compiler setup
cc = meson.get_compiler('c')
cflags = [
	'-Wno-unused-parameter',
	'-Wmissing-prototypes',
	'-Wstrict-prototypes',
	'-Wundef',
	'-Wlogical-op',
	'-Wpointer-arith',
	'-Wuninitialized',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough',
	'-Wredundant-decls',
	'-Wincompatible-pointer-types',
	'-Wformat=2',
	'-Wno-missing-field-initializers',
	'-Wmissing-declarations',

	'-fvisibility=hidden',
]
add_project_arguments(cc.get_supported_arguments(cflags), language : 'c')

# config.h
config_h = configuration_data()

doc_url_base = 'https://wayland.freedesktop.org/libinput/doc'
if libinput_version[2].to_int() >= 90
	doc_url = '@0@/latest'.format(doc_url_base)
else
	doc_url = '@0@/@1@'.format(doc_url_base, meson.project_version())
endif
config_h.set_quoted('HTTP_DOC_LINK', doc_url)

config_h.set('_GNU_SOURCE', '1')
if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
	config_h.set_quoted('MESON_BUILD_ROOT', meson.current_build_dir())
else
	config_h.set_quoted('MESON_BUILD_ROOT', '')
endif

prefix = '''#define _GNU_SOURCE 1
#include <assert.h>
'''
if cc.get_define('static_assert', prefix : prefix) == ''
	config_h.set('static_assert(...)', '/* */')
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

if cc.has_header_symbol('dirent.h', 'versionsort', prefix : prefix)
	config_h.set('HAVE_VERSIONSORT', '1')
endif

if not cc.has_header_symbol('errno.h', 'program_invocation_short_name', prefix : prefix)
	if cc.has_header_symbol('stdlib.h', 'getprogname')
		config_h.set('program_invocation_short_name', 'getprogname()')
	endif
endif

if cc.has_header('xlocale.h')
	config_h.set('HAVE_XLOCALE_H', '1')
endif

code = '''
#include <locale.h>
void main(void) { newlocale(LC_NUMERIC_MASK, "C", (locale_t)0); }
'''
if cc.links(code, name : 'locale.h')
	config_h.set('HAVE_LOCALE_H', '1')
endif

if not cc.has_header_symbol('sys/ptrace.h', 'PTRACE_ATTACH', prefix : prefix)
	config_h.set('PTRACE_ATTACH', 'PT_ATTACH')
	config_h.set('PTRACE_CONT', 'PT_CONTINUE')
	config_h.set('PTRACE_DETACH', 'PT_DETACH')
endif

config_h.set10('HAVE_INSTALLED_TESTS', get_option('install-tests'))

# Dependencies
pkgconfig = import('pkgconfig')
#dep_udev = dependency('libudev')
dep_udev = []
#dep_mtdev = dependency('mtdev', version : '>= 1.1.0')
dep_mtdev= []
#dep_libevdev = dependency('libevdev')
dep_libevdev = []
#config_h.set10('HAVE_LIBEVDEV_DISABLE_PROPERTY',
#		dep_libevdev.version().version_compare('>= 1.9.902'))

dep_lm = cc.find_library('m', required : false)
dep_rt = cc.find_library('rt', required : false)

# Include directories
includes_include = include_directories('include')
includes_src = include_directories('src')

############ libwacom configuration ############

have_libwacom = false #get_option('libwacom')
config_h.set10('HAVE_LIBWACOM', have_libwacom)
if have_libwacom
	dep_libwacom = dependency('libwacom', version : '>= 0.27')
else
	dep_libwacom = declare_dependency()
endif

############ udev bits ############

executable('libinput-device-group',
	   'udev/libinput-device-group.c',
	   dependencies : [dep_udev, dep_libwacom],
	   include_directories : [includes_src, includes_include],
	   install : true,
	   install_dir : dir_udev_callouts)
executable('libinput-fuzz-extract',
	   'udev/libinput-fuzz-extract.c',
	   'src/util-strings.c',
	   'src/util-prop-parsers.c',
	   dependencies : [dep_udev, dep_libevdev, dep_lm],
	   include_directories : [includes_src, includes_include],
	   install : true,
	   install_dir : dir_udev_callouts)
executable('libinput-fuzz-to-zero',
	   'udev/libinput-fuzz-to-zero.c',
	   dependencies : [dep_udev, dep_libevdev],
	   include_directories : [includes_src, includes_include],
	   install : true,
	   install_dir : dir_udev_callouts)

udev_rules_config = configuration_data()
udev_rules_config.set('UDEV_TEST_PATH', '')
configure_file(input : 'udev/80-libinput-device-groups.rules.in',
	       output : '80-libinput-device-groups.rules',
	       install_dir : dir_udev_rules,
	       configuration : udev_rules_config)
configure_file(input : 'udev/90-libinput-fuzz-override.rules.in',
	       output : '90-libinput-fuzz-override.rules',
	       install_dir : dir_udev_rules,
	       configuration : udev_rules_config)

litest_udev_rules_config = configuration_data()
litest_udev_rules_config.set('UDEV_TEST_PATH', meson.current_build_dir() + '/')
litest_groups_rules_file = configure_file(input : 'udev/80-libinput-device-groups.rules.in',
	       output : '80-libinput-device-groups-litest.rules',
	       configuration : litest_udev_rules_config)
litest_fuzz_override_file = configure_file(input : 'udev/90-libinput-fuzz-override.rules.in',
					   output : '90-libinput-fuzz-override-litest.rules',
					   configuration : litest_udev_rules_config)

############ Check for leftover udev rules ########

# This test should be defined first so we don't waste time testing anything
# else if we're about to fail anyway. ninja test will execute tests in the
# order of them defined in meson.build

if get_option('tests')
	test('leftover-rules',
	     find_program('test/check-leftover-udev-rules.sh'),
	     is_parallel : false,
	     suite : ['all'])
endif

############ libepoll-shim (BSD) ############

if cc.has_header_symbol('sys/epoll.h', 'epoll_create1', prefix : prefix)
	# epoll is built-in (Linux, illumos)
	dep_libepoll = declare_dependency()
else
	# epoll is implemented in userspace by libepoll-shim (FreeBSD)
	dir_libepoll = get_option('epoll-dir')
	if dir_libepoll == ''
		dir_libepoll = get_option('prefix')
	endif
	includes_epoll = include_directories(dir_libepoll / 'include' / 'libepoll-shim')
	dep_libepoll = cc.find_library('epoll-shim', dirs : dir_libepoll / 'lib')
	code = '''
	#include <sys/epoll.h>
	int main(void) { epoll_create1(0); }
	'''
	#if not cc.links(code,
	#	name : 'libepoll-shim check',
	#	dependencies : [dep_libepoll, dep_rt],
	#	include_directories : includes_epoll) # note: wants an include_directories object
	#	error('No built-in epoll or libepoll-shim found.')
	#endif
	dep_libepoll = declare_dependency(
		include_directories : includes_epoll,
		dependencies : [dep_libepoll, dep_rt])
endif

############ libinput-util.a ############

# Basic compilation test to make sure the headers include and define all the
# necessary bits.
util_headers = [
		'util-bits.h',
		'util-input-event.h',
		'util-list.h',
		'util-macros.h',
		'util-matrix.h',
		'util-prop-parsers.h',
		'util-ratelimit.h',
		'util-strings.h',
		'util-time.h',
]
foreach h: util_headers
	c = configuration_data()
	c.set_quoted('FILE', h)
	testfile = configure_file(input : 'test/test-util-includes.c',
			          output : 'test-util-includes-@0@.c'.format(h),
				  configuration : c)
	executable('test-build-@0@'.format(h),
		   testfile,
		   include_directories : [includes_src, includes_include],
		   install : false)
endforeach

src_libinput_util = [
	'src/util-list.c',
	'src/util-ratelimit.c',
	'src/util-strings.c',
	'src/util-prop-parsers.c',
]
libinput_util = static_library('libinput-util',
			       src_libinput_util,
			       dependencies : [dep_udev, dep_libevdev, dep_libwacom],
			       include_directories : includes_include)
dep_libinput_util = declare_dependency(link_with : libinput_util)

############ libfilter.a ############
src_libfilter = [
		'src/filter.c',
		'src/filter-custom.c',
		'src/filter-flat.c',
		'src/filter-low-dpi.c',
		'src/filter-mouse.c',
		'src/filter-touchpad.c',
		'src/filter-touchpad-flat.c',
		'src/filter-touchpad-x230.c',
		'src/filter-tablet.c',
		'src/filter-trackpoint.c',
		'src/filter-trackpoint-flat.c',
]
libfilter = static_library('filter', src_libfilter,
			   dependencies : [dep_udev, dep_libwacom],
			   include_directories : includes_include)
dep_libfilter = declare_dependency(link_with : libfilter)

############ libquirks.a #############
libinput_data_path = dir_data
libinput_data_override_path = dir_overrides / 'local-overrides.quirks'
config_h.set_quoted('LIBINPUT_QUIRKS_DIR', dir_data)
config_h.set_quoted('LIBINPUT_QUIRKS_OVERRIDE_FILE', libinput_data_override_path)

config_h.set_quoted('LIBINPUT_QUIRKS_SRCDIR', dir_src_quirks)
install_subdir('quirks',
	       exclude_files: ['README.md'],
	       install_dir : dir_data,
	       strip_directory : true)

src_libquirks = [
	'src/quirks.c',
]

deps_libquirks = [dep_udev, dep_libwacom, dep_libinput_util]
libquirks = static_library('quirks', src_libquirks,
			   dependencies : deps_libquirks,
			   include_directories : includes_include)
dep_libquirks = declare_dependency(link_with : libquirks)

# Create /etc/libinput
if meson.version().version_compare('>= 0.60')
	install_emptydir(dir_etc / 'libinput')
else
	install_subdir('libinput', install_dir : dir_etc)
endif

############ libinput.so ############
install_headers('src/libinput.h')
src_libinput = src_libfilter + [
	'src/libinput.c',
	'src/libinput-private-config.c',
	'src/evdev.c',
	'src/evdev-debounce.c',
	'src/evdev-fallback.c',
	'src/evdev-totem.c',
	'src/evdev-middle-button.c',
	'src/evdev-mt-touchpad.c',
	'src/evdev-mt-touchpad-tap.c',
	'src/evdev-mt-touchpad-thumb.c',
	'src/evdev-mt-touchpad-buttons.c',
	'src/evdev-mt-touchpad-edge-scroll.c',
	'src/evdev-mt-touchpad-gestures.c',
	'src/evdev-tablet.c',
	'src/evdev-tablet-pad.c',
	'src/evdev-tablet-pad-leds.c',
	'src/evdev-wheel.c',
	'src/path-seat.c',
	'src/udev-seat.c',
	'src/timer.c',
]

deps_libinput = [
	dep_mtdev,
	dep_udev,
	dep_libevdev,
	dep_libepoll,
	dep_lm,
	dep_rt,
	dep_libwacom,
	dep_libinput_util,
	dep_libquirks
]

libinput_version_h_config = configuration_data()
libinput_version_h_config.set('LIBINPUT_VERSION_MAJOR', libinput_version[0])
libinput_version_h_config.set('LIBINPUT_VERSION_MINOR', libinput_version[1])
libinput_version_h_config.set('LIBINPUT_VERSION_MICRO', libinput_version[2])
libinput_version_h_config.set('LIBINPUT_VERSION', meson.project_version())

libinput_version_h = configure_file(
		input : 'src/libinput-version.h.in',
		output : 'libinput-version.h',
		configuration : libinput_version_h_config,
)

mapfile = dir_src / 'libinput.sym'
version_flag = '-Wl,--version-script,@0@'.format(mapfile)
lib_libinput = shared_library('input',
		src_libinput,
		include_directories : [include_directories('.'), includes_include],
		dependencies : deps_libinput,
		version : libinput_so_version,
		link_args : version_flag,
		link_depends : mapfile,
		install : true
		)

dep_libinput = declare_dependency(
		link_with : lib_libinput,
		dependencies : deps_libinput)

if meson.version().version_compare('>= 0.54.0')
	meson.override_dependency('libinput', dep_libinput)
endif

pkgconfig.generate(
	filebase : 'libinput',
	name : 'Libinput',
	description : 'Input device library',
	version : meson.project_version(),
	libraries : lib_libinput
)

git_version_h = vcs_tag(command : ['git', 'describe'],
			fallback : 'unknown',
			input : 'src/libinput-git-version.h.in',
			output :'libinput-git-version.h')

############ documentation ############

if get_option('documentation')
	subdir('doc/api')
	subdir('doc/user')
endif

############ shell completion #########

subdir('completion/zsh')

############ tools ############
libinput_tool_path = dir_libexec
config_h.set_quoted('LIBINPUT_TOOL_PATH', libinput_tool_path)
tools_shared_sources = [ 'tools/shared.c' ]
deps_tools_shared = [ dep_libinput, dep_libevdev ]
lib_tools_shared = static_library('tools_shared',
				  tools_shared_sources,
				  include_directories : [includes_src, includes_include],
				  dependencies : deps_tools_shared)
dep_tools_shared = declare_dependency(link_with : lib_tools_shared,
				      dependencies : deps_tools_shared)

deps_tools = [ dep_tools_shared, dep_libinput ]
libinput_debug_events_sources = [
	'tools/libinput-debug-events.c',
	libinput_version_h,
]
executable('libinput-debug-events',
	   libinput_debug_events_sources,
	   dependencies : deps_tools,
	   include_directories : [includes_src, includes_include],
	   install_dir : libinput_tool_path,
	   install : true
	   )

libinput_debug_tablet_sources = [ 'tools/libinput-debug-tablet.c' ]
executable('libinput-debug-tablet',
	   libinput_debug_tablet_sources,
	   dependencies : deps_tools,
	   include_directories : [includes_src, includes_include],
	   install_dir : libinput_tool_path,
	   install : true)


libinput_quirks_sources = [ 'tools/libinput-quirks.c' ]
libinput_quirks = executable('libinput-quirks',
			     libinput_quirks_sources,
			     dependencies : [dep_libquirks, dep_tools_shared, dep_libinput],
			     include_directories : [includes_src, includes_include],
			     install_dir : libinput_tool_path,
			     install : true
			    )
test('validate-quirks',
     libinput_quirks,
     args: ['validate', '--data-dir=@0@'.format(dir_src_quirks)],
     suite : ['all']
     )

quirks_file_tester = find_program('test/test_quirks_files.py')
test('validate-quirks-files',
     quirks_file_tester,
     suite : ['all'],
     env: ['MESON_SOURCE_ROOT=@0@'.format(meson.project_source_root())],
     )

libinput_list_devices_sources = [ 'tools/libinput-list-devices.c' ]
libinput_list_devices = executable('libinput-list-devices',
				   libinput_list_devices_sources,
				   dependencies : deps_tools,
				   include_directories : [includes_src, includes_include],
				   install_dir : libinput_tool_path,
				   install : true,
				  )
test('list-devices',
     libinput_list_devices,
     suite : ['all', 'root', 'hardware'])

libinput_measure_sources = [ 'tools/libinput-measure.c' ]
executable('libinput-measure',
	   libinput_measure_sources,
	   dependencies : deps_tools,
	   include_directories : [includes_src, includes_include],
	   install_dir : libinput_tool_path,
	   install : true,
	   )

libinput_analyze_sources = [ 'tools/libinput-analyze.c' ]
executable('libinput-analyze',
	   libinput_analyze_sources,
	   dependencies : deps_tools,
	   include_directories : [includes_src, includes_include],
	   install_dir : libinput_tool_path,
	   install : true,
	   )

src_python_tools = files(
	      'tools/libinput-analyze-per-slot-delta.py',
	      'tools/libinput-analyze-recording.py',
	      'tools/libinput-analyze-touch-down-state.py',
	      'tools/libinput-list-kernel-devices.py',
	      'tools/libinput-measure-fuzz.py',
	      'tools/libinput-measure-touchpad-size.py',
	      'tools/libinput-measure-touchpad-tap.py',
	      'tools/libinput-measure-touchpad-pressure.py',
	      'tools/libinput-measure-touch-size.py',
	      'tools/libinput-replay.py'
)

foreach t : src_python_tools
	configure_file(input: t,
		       output: '@BASENAME@',
		       copy: true,
		       install_dir : libinput_tool_path
		      )
endforeach

libinput_record_sources = [ 'tools/libinput-record.c', git_version_h ]
executable('libinput-record',
	   libinput_record_sources,
	   dependencies : deps_tools + [dep_udev],
	   include_directories : [includes_src, includes_include],
	   install_dir : libinput_tool_path,
	   install : true,
	   )

config_h.set10('HAVE_DEBUG_GUI', get_option('debug-gui'))
if get_option('debug-gui')
	dep_gtk = dependency('gtk4', version : '>= 4.0', required : false)
	config_h.set10('HAVE_GTK4', dep_gtk.found())
	if not dep_gtk.found()
		dep_gtk = dependency('gtk+-3.0', version : '>= 3.20')
		config_h.set10('HAVE_GTK3', dep_gtk.found())
	endif

	if meson.version().version_compare('>= 0.58')
		gtk_targets = dep_gtk.get_variable('targets')
	else
		gtk_targets = dep_gtk.get_pkgconfig_variable('targets')
	endif

	have_gtk_wayland = gtk_targets.contains('wayland')
	have_gtk_x11 = gtk_targets.contains('x11')

	dep_cairo = dependency('cairo')
	dep_glib = dependency('glib-2.0')
	dep_x11 = dependency('x11', required : false)
	dep_wayland_client = dependency('wayland-client', required : false)
	dep_wayland_protocols = dependency('wayland-protocols', required : false)

	config_h.set10('HAVE_GTK_X11', have_gtk_x11 and dep_x11.found())
	config_h.set10('HAVE_GTK_WAYLAND', false)

	debug_gui_sources = [ 'tools/libinput-debug-gui.c' ]

	if have_gtk_wayland and dep_wayland_client.found() and dep_wayland_protocols.found()
		wayland_scanner = find_program('wayland-scanner')
		if meson.version().version_compare('>= 0.58')
			wlproto_dir = dep_wayland_protocols.get_variable('pkgdatadir')
		else
			wlproto_dir = dep_wayland_protocols.get_pkgconfig_variable('pkgdatadir')
		endif

		proto_name = 'pointer-constraints-unstable-v1'
		input = files(wlproto_dir / 'unstable' / 'pointer-constraints' / '@0@.xml'.format(proto_name))

		wayland_headers = custom_target('@0@ client header'.format(proto_name),
			input: input,
			output: '@0@-client-protocol.h'.format(proto_name),
			command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
		)

		wayland_sources = custom_target('@0@ source'.format(proto_name),
			input: input,
			output: '@0@-protocol.c'.format(proto_name),
			command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
		)

		debug_gui_sources += [ wayland_headers, wayland_sources ]
		config_h.set10('HAVE_GTK_WAYLAND', true)
	endif

	deps_debug_gui = [
			dep_gtk,
			dep_cairo,
			dep_glib,
			dep_wayland_client,
			dep_wayland_protocols,
			dep_x11,
			] + deps_tools
	executable('libinput-debug-gui',
		   debug_gui_sources,
		   dependencies : deps_debug_gui,
		   include_directories : [includes_src, includes_include],
		   install_dir : libinput_tool_path,
		   install : true
		   )
	src_man += files('tools/libinput-debug-gui.man')
endif

libinput_sources = [ 'tools/libinput-tool.c' ]

libinput_tool = executable('libinput',
			   libinput_sources,
			   dependencies : deps_tools,
			   include_directories : [includes_src, includes_include],
			   install : true
			  )

ptraccel_debug_sources = [ 'tools/ptraccel-debug.c' ]
executable('ptraccel-debug',
	   ptraccel_debug_sources,
	   dependencies : [ dep_libfilter, dep_libinput ],
	   include_directories : [includes_src, includes_include],
	   install : false
	   )

# Don't run the test during a release build because we rely on the magic
# subtool lookup
if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
	config_tool_option_test = configuration_data()
	config_tool_option_test.set('DISABLE_WARNING', 'yes')
	config_tool_option_test.set('MESON_ENABLED_DEBUG_GUI', get_option('debug-gui'))
	config_tool_option_test.set('MESON_BUILD_ROOT', meson.current_build_dir())
	config_tool_option_test.set('TOOL_PATH', libinput_tool.full_path())
	tool_option_test = configure_file(input: 'tools/test_tool_option_parsing.py',
					  output: '@PLAINNAME@',
					  configuration : config_tool_option_test)
	test('tool-option-parsing',
	     tool_option_test,
	     args : [tool_option_test],
	     suite : ['all', 'root'],
	     timeout : 240)
endif

# the libinput tools check whether we execute from the builddir, this is
# the test to verify that lookup. We test twice, once as normal test
# run from the builddir, once after copying to /tmp
test_builddir_lookup = executable('test-builddir-lookup',
				  'test/test-builddir-lookup.c',
				  dependencies : [ dep_tools_shared],
				  include_directories : [includes_src, includes_include],
				  install : false)
test('tools-builddir-lookup',
     test_builddir_lookup,
     args : ['--builddir-is-set'],
     suite : ['all'])
test('tools-builddir-lookup-installed',
     find_program('test/helper-copy-and-exec-from-tmp.sh'),
     args : [test_builddir_lookup.full_path(), '--builddir-is-null'],
     env : ['LD_LIBRARY_PATH=@0@'.format(meson.current_build_dir())],
     suite : ['all'],
     workdir : '/tmp')

############ tests ############

test('symbols-leak-test',
     find_program('test/symbols-leak-test'),
     args : [ dir_src / 'libinput.sym', dir_src],
     suite : ['all'])

# build-test only
executable('test-build-pedantic',
	   'test/build-pedantic.c',
	   dependencies : [dep_udev],
	   include_directories : [includes_src, includes_include],
	   c_args : ['-std=c99', '-pedantic', '-Werror'],
	   install : false)
# build-test only
executable('test-build-std-gnuc90',
	   'test/build-pedantic.c',
	   dependencies : [dep_udev],
	   include_directories : [includes_src, includes_include],
	   c_args : ['-std=gnu89', '-Werror'],
	   install : false)
# test for linking with the minimal linker flags
executable('test-build-linker',
	   'test/build-pedantic.c',
	   include_directories : [includes_src, includes_include],
	   dependencies : [ dep_libinput, dep_libinput_util ],
	   install : false)
# test including from C++ (in case CPP compiler is available)
if add_languages('cpp', native: false, required: false)
	executable('test-build-cxx',
		   'test/build-cxx.cc',
		   dependencies : [dep_udev],
		   include_directories : [includes_src, includes_include],
		   install : false)
endif

libinput_test_sources = [ 'tools/libinput-test.c' ]
executable('libinput-test',
	   libinput_test_sources,
	   dependencies : deps_tools,
	   include_directories : [includes_src, includes_include],
	   install_dir : libinput_tool_path,
	   install : true,
	   )

# This is the test suite runner, we allow disabling that one because of
# dependencies
if get_option('tests')
	dep_check = dependency('check', version : '>= 0.9.10')

	gstack = find_program('gstack', required : false)
	config_h.set10('HAVE_GSTACK', gstack.found())

	# for inhibit support during test run
	dep_libsystemd = dependency('libsystemd', version : '>= 221', required : false)
	config_h.set10('HAVE_LIBSYSTEMD', dep_libsystemd.found())

	litest_sources = [
		'src/libinput-private-config.c',
		'test/litest-device-absinfo-override.c',
		'test/litest-device-acer-hawaii-keyboard.c',
		'test/litest-device-acer-hawaii-touchpad.c',
		'test/litest-device-aiptek-tablet.c',
		'test/litest-device-alps-3fg.c',
		'test/litest-device-alps-semi-mt.c',
		'test/litest-device-alps-dualpoint.c',
		'test/litest-device-anker-mouse-kbd.c',
		'test/litest-device-apple-appletouch.c',
		'test/litest-device-apple-internal-keyboard.c',
		'test/litest-device-apple-magicmouse.c',
		'test/litest-device-asus-rog-gladius.c',
		'test/litest-device-atmel-hover.c',
		'test/litest-device-bcm5974.c',
		'test/litest-device-calibrated-touchscreen.c',
		'test/litest-device-cyborg-rat-5.c',
		'test/litest-device-dell-canvas-totem.c',
		'test/litest-device-dell-canvas-totem-touch.c',
		'test/litest-device-elantech-touchpad.c',
		'test/litest-device-elan-tablet.c',
		'test/litest-device-format-string.c',
		'test/litest-device-generic-pressurepad.c',
		'test/litest-device-generic-singletouch.c',
		'test/litest-device-gpio-keys.c',
		'test/litest-device-huion-pentablet.c',
		'test/litest-device-hp-wmi-hotkeys.c',
		'test/litest-device-ignored-mouse.c',
		'test/litest-device-keyboard.c',
		'test/litest-device-keyboard-all-codes.c',
		'test/litest-device-keyboard-quirked.c',
		'test/litest-device-keyboard-razer-blackwidow.c',
		'test/litest-device-keyboard-razer-blade-stealth.c',
		'test/litest-device-keyboard-razer-blade-stealth-videoswitch.c',
		'test/litest-device-lenovo-scrollpoint.c',
		'test/litest-device-lid-switch.c',
		'test/litest-device-lid-switch-surface3.c',
		'test/litest-device-logitech-media-keyboard-elite.c',
		'test/litest-device-logitech-trackball.c',
		'test/litest-device-nexus4-touch-screen.c',
		'test/litest-device-magic-trackpad.c',
		'test/litest-device-mouse.c',
		'test/litest-device-mouse-wheel-tilt.c',
		'test/litest-device-mouse-roccat.c',
		'test/litest-device-mouse-low-dpi.c',
		'test/litest-device-mouse-wheel-click-angle.c',
		'test/litest-device-mouse-wheel-click-count.c',
		'test/litest-device-ms-nano-transceiver-mouse.c',
		'test/litest-device-ms-surface-cover.c',
		'test/litest-device-protocol-a-touch-screen.c',
		'test/litest-device-qemu-usb-tablet.c',
		'test/litest-device-sony-vaio-keys.c',
		'test/litest-device-synaptics-x220.c',
		'test/litest-device-synaptics-hover.c',
		'test/litest-device-synaptics-i2c.c',
		'test/litest-device-synaptics-pressurepad.c',
		'test/litest-device-synaptics-rmi4.c',
		'test/litest-device-synaptics-st.c',
		'test/litest-device-synaptics-t440.c',
		'test/litest-device-synaptics-x1-carbon-3rd.c',
		'test/litest-device-synaptics-phantomclicks.c',
		'test/litest-device-tablet-mode-switch.c',
		'test/litest-device-thinkpad-extrabuttons.c',
		'test/litest-device-trackpoint.c',
		'test/litest-device-touch-screen.c',
		'test/litest-device-touchpad-palm-threshold-zero.c',
		'test/litest-device-touchscreen-invalid-range.c',
		'test/litest-device-touchscreen-fuzz.c',
		'test/litest-device-touchscreen-mt-tool.c',
		'test/litest-device-uclogic-tablet.c',
		'test/litest-device-wacom-bamboo-2fg-finger.c',
		'test/litest-device-wacom-bamboo-2fg-pad.c',
		'test/litest-device-wacom-bamboo-2fg-pen.c',
		'test/litest-device-wacom-bamboo-16fg-pen.c',
		'test/litest-device-wacom-calibrated-tablet.c',
		'test/litest-device-wacom-cintiq-12wx-pen.c',
		'test/litest-device-wacom-cintiq-13hdt-finger.c',
		'test/litest-device-wacom-cintiq-13hdt-pad.c',
		'test/litest-device-wacom-cintiq-13hdt-pen.c',
		'test/litest-device-wacom-cintiq-24hd-pen.c',
		'test/litest-device-wacom-cintiq-24hdt-pad.c',
		'test/litest-device-wacom-cintiq-pro-16-finger.c',
		'test/litest-device-wacom-cintiq-pro-16-pad.c',
		'test/litest-device-wacom-cintiq-pro-16-pen.c',
		'test/litest-device-wacom-ekr.c',
		'test/litest-device-wacom-hid4800-pen.c',
		'test/litest-device-wacom-intuos3-pad.c',
		'test/litest-device-wacom-intuos5-finger.c',
		'test/litest-device-wacom-intuos5-pad.c',
		'test/litest-device-wacom-intuos5-pen.c',
		'test/litest-device-wacom-isdv4-4200-pen.c',
		'test/litest-device-wacom-isdv4-524c-pen.c',
		'test/litest-device-wacom-isdv4-e6-pen.c',
		'test/litest-device-wacom-isdv4-e6-finger.c',
		'test/litest-device-wacom-mobilestudio-pro-pad.c',
		'test/litest-device-waltop-tablet.c',
		'test/litest-device-wheel-only.c',
		'test/litest-device-xen-virtual-pointer.c',
		'test/litest-device-vmware-virtual-usb-mouse.c',
		'test/litest-device-yubikey.c',
		'test/litest.c',
	]

	dep_dl = cc.find_library('dl')
	deps_litest = [
		dep_libinput,
		dep_check,
		dep_udev,
		dep_libevdev,
		dep_dl,
		dep_lm,
		dep_libsystemd,
		dep_libquirks,
	]

	litest_config_h = configuration_data()
	litest_config_h.set_quoted('LIBINPUT_DEVICE_GROUPS_RULES_FILE',
				   meson.current_build_dir() /
				   '80-libinput-device-groups-litest.rules')
	litest_config_h.set_quoted('LIBINPUT_FUZZ_OVERRIDE_UDEV_RULES_FILE',
				   meson.current_build_dir() /
				   '90-libinput-fuzz-override-litest.rules')

	def_no_main = '-DLITEST_NO_MAIN'
	def_disable_backtrace = '-DLITEST_DISABLE_BACKTRACE_LOGGING'
	defs_litest_selftest = [
		def_no_main,
		def_disable_backtrace,
		'-Wno-unused',
	]
	test_litest_selftest_sources = [
		'test/litest-selftest.c',
		'test/litest.c',
	]
	test_litest_selftest = executable('test-litest-selftest',
					  test_litest_selftest_sources,
					  include_directories : [includes_src, includes_include],
					  dependencies : deps_litest,
					  c_args : defs_litest_selftest,
					  install : false)
	test('test-litest-selftest',
	     test_litest_selftest,
	     suite : ['all'],
	     timeout : 100)

	def_LT_VERSION = '-DLIBINPUT_LT_VERSION="@0@:@1@:@2@"'.format(libinput_lt_c, libinput_lt_r, libinput_lt_a)
	test_library_version = executable('test-library-version',
					  ['test/test-library-version.c'],
					  c_args : [ def_LT_VERSION ],
					  install : false)
	test('test-library-version',
	     test_library_version,
	     suite : ['all'])

	test_utils_sources = [
		'test/test-utils.c',
	]
	test_utils = executable('libinput-test-utils',
				test_utils_sources,
				include_directories : [includes_src, includes_include],
				dependencies : deps_litest,
				install_dir : libinput_tool_path,
				install : get_option('install-tests'))
	test('test-utils',
	     test_utils,
	     suite : ['all'])

	# When adding new files to this list, update the CI
	tests_sources = [
		'test/test-udev.c',
		'test/test-path.c',
		'test/test-pointer.c',
		'test/test-touch.c',
		'test/test-log.c',
		'test/test-tablet.c',
		'test/test-totem.c',
		'test/test-pad.c',
		'test/test-touchpad.c',
		'test/test-touchpad-tap.c',
		'test/test-touchpad-buttons.c',
		'test/test-trackpoint.c',
		'test/test-trackball.c',
		'test/test-misc.c',
		'test/test-keyboard.c',
		'test/test-device.c',
		'test/test-gestures.c',
		'test/test-switch.c',
		'test/test-quirks.c',
	]
	libinput_test_runner_sources = litest_sources + tests_sources
	libinput_test_runner = executable('libinput-test-suite',
					  libinput_test_runner_sources,
					  include_directories : [includes_src, includes_include],
					  dependencies : deps_litest,
					  install_dir : libinput_tool_path,
					  install : get_option('install-tests'))

	src_man += 'test/libinput-test-suite.man'

	foreach testfile : tests_sources
		tfile = testfile.split('test/test-')[1]
		group = tfile.split('.c')[0]
		test('libinput-test-suite-@0@'.format(group),
		     libinput_test_runner,
		     suite : ['all', 'valgrind', 'root', 'hardware'],
		     args : ['--filter-group=@0@'.format(group),
			     '--xml-output=junit-@0@-XXXXXX.xml'.format(group)],
		     is_parallel : false,
		     timeout : 1200)
        endforeach

	test('libinput-test-deviceless',
	     libinput_test_runner,
	     suite : ['all', 'valgrind'],
	     args: ['--filter-deviceless',
	            '--xml-output=junit-deviceless-XXXXXX.xml'])

	valgrind = find_program('valgrind', required : false)
	if valgrind.found()
		valgrind_env = environment()
		valgrind_suppressions_file = dir_src_test / 'valgrind.suppressions'
		add_test_setup('valgrind',
				exe_wrapper : [ valgrind,
						'--leak-check=full',
						'--gen-suppressions=all',
						'--error-exitcode=3',
						'--suppressions=' + valgrind_suppressions_file ],
				env :  valgrind_env,
				timeout_multiplier : 100)
	else
		message('valgrind not found, disabling valgrind test suite')
	endif
	configure_file(output : 'litest-config.h',
		       configuration : litest_config_h)
endif


############ man pages ############
man_config = configuration_data()
man_config.set('LIBINPUT_VERSION', meson.project_version())
man_config.set('LIBINPUT_DATA_DIR', dir_data)
if get_option('install-tests')
	man_config.set('HAVE_INSTALLED_TESTS', '.\"')
else
	man_config.set('HAVE_INSTALLED_TESTS', '')
endif
if get_option('debug-gui')
	man_config.set('HAVE_DEBUG_GUI', '')
else
	man_config.set('HAVE_DEBUG_GUI', '.\"')
endif

src_man += files(
	'tools/libinput.man',
	'tools/libinput-analyze.man',
	'tools/libinput-analyze-per-slot-delta.man',
	'tools/libinput-analyze-recording.man',
	'tools/libinput-analyze-touch-down-state.man',
	'tools/libinput-debug-events.man',
	'tools/libinput-debug-tablet.man',
	'tools/libinput-list-devices.man',
	'tools/libinput-list-kernel-devices.man',
	'tools/libinput-measure.man',
	'tools/libinput-measure-fuzz.man',
	'tools/libinput-measure-touchpad-size.man',
	'tools/libinput-measure-touchpad-tap.man',
	'tools/libinput-measure-touchpad-pressure.man',
	'tools/libinput-measure-touch-size.man',
	'tools/libinput-quirks.man',
	'tools/libinput-record.man',
	'tools/libinput-replay.man',
	'tools/libinput-test.man',
)

foreach m : src_man
	configure_file(input : m,
		       output : '@BASENAME@.1',
		       configuration : man_config,
		       install_dir : dir_man1)
endforeach

# Same man page for the subtools to stay consistent with the other tools
configure_file(input : 'tools/libinput-quirks.man',
	       output : 'libinput-quirks-list.1',
	       configuration : man_config,
	       install_dir : dir_man1,
	       )
configure_file(input : 'tools/libinput-quirks.man',
	       output : 'libinput-quirks-validate.1',
	       configuration : man_config,
	       install_dir : dir_man1,
	       )

############ output files ############
configure_file(output : 'config.h', configuration : config_h)
