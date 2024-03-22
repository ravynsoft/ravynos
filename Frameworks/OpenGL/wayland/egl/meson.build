wayland_egl = library(
	'wayland-egl',
	sources: [
		'wayland-egl.c',
		wayland_client_protocol_h
	],
	include_directories: src_inc,
	version: meson.project_version(),
	install: true
)

executable('wayland-egl-abi-check', 'wayland-egl-abi-check.c')

nm_path = find_program('nm').full_path()

test(
	'wayland-egl symbols check',
	find_program('wayland-egl-symbols-check'),
	env: [
		'WAYLAND_EGL_LIB=@0@'.format(wayland_egl.full_path()),
		'NM=@0@'.format(nm_path)
	]
)

install_headers([
	'wayland-egl.h',
	'wayland-egl-core.h',
	'wayland-egl-backend.h'
])

pkgconfig.generate(
	name: 'wayland-egl',
	description: 'Frontend wayland-egl library',
	version: '18.1.0',
	requires: 'wayland-client',
	libraries: wayland_egl
)

pkgconfig.generate(
	name: 'wayland-egl-backend',
	description: 'Backend wayland-egl interface',
	version: '3'
)

wayland_egl_dep = declare_dependency(
	link_with: wayland_egl,
	include_directories: [ root_inc, include_directories('.') ],
)

if meson.version().version_compare('>= 0.54.0')
	meson.override_dependency('wayland-egl', wayland_egl_dep)
endif
