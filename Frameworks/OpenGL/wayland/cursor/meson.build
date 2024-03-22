icondir = get_option('icon_directory')
if icondir == ''
	icondir = join_paths(get_option('prefix'), get_option('datadir'), 'icons')
endif

if wayland_version[0] != '1'
	# The versioning used for the shared libraries assumes that the major
	# version of Wayland as a whole will increase to 2 if and only if there
	# is an ABI break, at which point we should probably bump the SONAME of
	# all libraries to .so.2. For more details see
	# https://gitlab.freedesktop.org/wayland/wayland/-/merge_requests/177
	error('We probably need to bump the SONAME of libwayland-cursor')
endif

wayland_cursor = library(
	'wayland-cursor',
	sources: [
		'wayland-cursor.c',
		'os-compatibility.c',
		'xcursor.c',
	],
	# To avoid an unnecessary SONAME bump, wayland 1.x.y produces
	# libwayland-cursor.so.0.x.y.
	version: '.'.join(['0', wayland_version[1], wayland_version[2]]),
	dependencies: [ wayland_client_dep ],
	c_args: [ '-DICONDIR="@0@"'.format(icondir) ],
	install: true,
)

install_headers('wayland-cursor.h')

pkgconfig.generate(
	name: 'Wayland Cursor',
	description: 'Wayland cursor helper library',
	version: meson.project_version(),
	libraries: wayland_cursor,
	filebase: 'wayland-cursor',
)

wayland_cursor_dep = declare_dependency(
	link_with: wayland_cursor,
	include_directories: [ root_inc, include_directories('.') ],
)

if meson.version().version_compare('>= 0.54.0')
	meson.override_dependency('wayland-cursor', wayland_cursor_dep)
endif
