threads = dependency('threads')
wayland_egl = dependency('wayland-egl')
wayland_cursor = dependency('wayland-cursor')
wayland_client = dependency('wayland-client')
libpng = dependency('libpng', required: false, disabler: true)
egl = dependency('egl', required: false, disabler: true)
glesv2 = dependency('glesv2', required: false, disabler: true)
# These versions correspond to ffmpeg 4.0
libavutil = dependency('libavutil', version: '>=56.14.100', required: false, disabler: true)
libavcodec = dependency('libavcodec', version: '>=58.18.100', required: false, disabler: true)
libavformat = dependency('libavformat', version: '>=58.12.100', required: false, disabler: true)
# Only needed for drm_fourcc.h
libdrm = dependency('libdrm').partial_dependency(compile_args: true, includes: true)

# epoll is a separate library in FreeBSD
if host_machine.system() == 'freebsd'
	libepoll = dependency('epoll-shim')
else
	libepoll = dependency('', required: false)
endif

if not cc.has_header('libavutil/hwcontext_drm.h', dependencies: libavutil)
	libavutil = disabler()
endif

compositors = {
	'simple': {
		'src': 'simple.c',
	},
	'pointer': {
		'src': 'pointer.c',
	},
	'touch': {
		'src': ['touch.c', 'cat.c'],
	},
	'tablet': {
		'src': 'tablet.c',
	},
	'rotation': {
		'src': ['rotation.c', 'cat.c'],
	},
	'multi-pointer': {
		'src': 'multi-pointer.c',
	},
	'output-layout': {
		'src': ['output-layout.c', 'cat.c'],
	},
	'fullscreen-shell': {
		'src': 'fullscreen-shell.c',
		'proto': ['fullscreen-shell-unstable-v1'],
	},
	'quads': {
		'src': 'quads.c',
	},
	'scene-graph': {
		'src': 'scene-graph.c',
		'proto': ['xdg-shell'],
	},
}

clients = {
	'idle': {
		'src': 'idle.c',
		'dep': threads,
		'proto': ['kde-idle'],
	},
	'idle-inhibit': {
		'src': ['idle-inhibit.c', 'egl_common.c'],
		'dep': [wayland_egl, egl, glesv2],
		'proto': [
			'idle-inhibit-unstable-v1',
			'xdg-shell',
		],
	},
	'keyboard-shortcuts-inhibit': {
		'src': ['keyboard-shortcuts-inhibit.c', 'egl_common.c'],
		'dep': [wayland_egl, wayland_cursor, egl, glesv2],
		'proto': [
			'keyboard-shortcuts-inhibit-unstable-v1',
			'xdg-shell',
		],
	},
	'layer-shell': {
		'src': ['layer-shell.c', 'egl_common.c'],
		'dep': [wayland_egl, wayland_cursor, egl, glesv2],
		'proto': [
			'wlr-layer-shell-unstable-v1',
			'xdg-shell',
		],
	},
	'input-inhibitor': {
		'src': ['input-inhibitor.c', 'egl_common.c'],
		'dep': [wayland_egl, wayland_cursor, egl, glesv2],
		'proto': [
			'wlr-input-inhibitor-unstable-v1',
			'xdg-shell',
		],
	},
	'gamma-control': {
		'src': 'gamma-control.c',
		'dep': [wayland_cursor, math],
		'proto': ['wlr-gamma-control-unstable-v1'],
	},
	'output-power-management': {
		'src': 'output-power-management.c',
		'dep': [wayland_client],
		'proto': ['wlr-output-power-management-unstable-v1'],
	},
	'pointer-constraints': {
		'src': ['pointer-constraints.c', 'egl_common.c'],
		'dep': [wayland_egl, egl, glesv2],
		'proto': [
			'pointer-constraints-unstable-v1',
			'xdg-shell',
		],
	},
	'relative-pointer': {
		'src': ['relative-pointer-unstable-v1.c', 'egl_common.c'],
		'dep': [wayland_egl, egl, glesv2],
		'proto': [
			'pointer-constraints-unstable-v1',
			'relative-pointer-unstable-v1',
			'xdg-shell',
		],
	},
	'dmabuf-capture': {
		'src': 'dmabuf-capture.c',
		'dep': [
			libavcodec,
			libavformat,
			libavutil,
			drm,
			threads,
		],
		'proto': ['wlr-export-dmabuf-unstable-v1'],
	},
	'screencopy': {
		'src': 'screencopy.c',
		'dep': [libpng, rt],
		'proto': ['wlr-screencopy-unstable-v1'],
	},
	'screencopy-dmabuf': {
		'src': 'screencopy-dmabuf.c',
		'dep': [libpng, rt, gbm, drm],
		'proto': [
			'wlr-screencopy-unstable-v1',
			'linux-dmabuf-unstable-v1',
		],
	},
	'toplevel-decoration': {
		'src': ['toplevel-decoration.c', 'egl_common.c'],
		'dep': [wayland_egl, egl, glesv2],
		'proto': [
			'xdg-decoration-unstable-v1',
			'xdg-shell',
		],
	},
	'input-method': {
		'src': 'input-method.c',
		'dep': [wayland_egl, libepoll],
		'proto': [
			'input-method-unstable-v2',
			'text-input-unstable-v3',
			'xdg-shell',
		],
	},
	'text-input': {
		'src': ['text-input.c', 'egl_common.c'],
		'dep': [wayland_egl, wayland_cursor, egl, glesv2],
		'proto': [
			'text-input-unstable-v3',
			'xdg-shell',
		],
	},
	'foreign-toplevel': {
		'src': 'foreign-toplevel.c',
		'proto': ['wlr-foreign-toplevel-management-unstable-v1'],
	},
	'virtual-pointer': {
		'src': 'virtual-pointer.c',
		'proto': ['wlr-virtual-pointer-unstable-v1'],
	},
	'input-method-keyboard-grab': {
		'src': 'input-method-keyboard-grab.c',
		'dep': xkbcommon,
		'proto': [
			'input-method-unstable-v2',
		],
	},
}

foreach name, info : compositors
	extra_src = []
	foreach p : info.get('proto', [])
		extra_src += protocols_server_header[p]
	endforeach

	executable(
		name,
		[info.get('src'), extra_src],
		dependencies: [wlroots, libdrm],
		include_directories: [wlr_inc, proto_inc],
		build_by_default: get_option('examples'),
	)
endforeach

foreach name, info : clients
	extra_src = []
	foreach p : info.get('proto')
		extra_src += protocols_code[p]
		extra_src += protocols_client_header[p]
	endforeach

	executable(
		name,
		[info.get('src'), extra_src],
		dependencies: [wayland_client, info.get('dep', [])],
		build_by_default: get_option('examples'),
	)
endforeach
