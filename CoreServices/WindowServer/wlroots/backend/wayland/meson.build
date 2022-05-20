wayland_client = dependency('wayland-client',
	fallback: 'wayland',
	default_options: wayland_project_options,
)
wlr_deps += wayland_client

wlr_files += files(
	'backend.c',
	'output.c',
	'seat.c',
	'tablet_v2.c',
)

client_protos = [
	'drm',
	'linux-dmabuf-unstable-v1',
	'pointer-gestures-unstable-v1',
	'presentation-time',
	'relative-pointer-unstable-v1',
	'tablet-unstable-v2',
	'xdg-activation-v1',
	'xdg-decoration-unstable-v1',
	'xdg-shell',
]

foreach proto : client_protos
	wlr_files += protocols_client_header[proto]
endforeach
