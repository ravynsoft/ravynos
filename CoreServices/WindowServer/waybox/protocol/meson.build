wl_protocol_dir = wayland_protos.get_variable(pkgconfig: 'pkgdatadir')

wayland_scanner = find_program('wayland-scanner', version: '>= 1.15')

wayland_scanner_server = generator(
  wayland_scanner,
  output: '@BASENAME@-protocol.h',
  arguments: ['server-header', '@INPUT@', '@OUTPUT@'],
)

wayland_scanner_code = generator(
  wayland_scanner,
  output: '@BASENAME@-protocol.c',
  arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

wayland_scanner_client = generator(
  wayland_scanner,
  output: '@BASENAME@-client-protocol.h',
  arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)

protocols = [
  [wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'],
  [wl_protocol_dir, 'unstable/idle-inhibit/idle-inhibit-unstable-v1.xml'],
  [wl_protocol_dir, 'unstable/linux-dmabuf/linux-dmabuf-unstable-v1.xml'],
  [wl_protocol_dir, 'unstable/primary-selection/primary-selection-unstable-v1.xml'],
  [wl_protocol_dir, 'unstable/xdg-decoration/xdg-decoration-unstable-v1.xml'],
  [wl_protocol_dir, 'unstable/xdg-output/xdg-output-unstable-v1.xml'],
  'wlr-gamma-control-unstable-v1.xml',
  'idle.xml',
  'wlr-screencopy-unstable-v1.xml',
  'wlr-layer-shell-unstable-v1.xml',
]

client_protocols = [
  [wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'],
  [wl_protocol_dir, 'unstable/idle-inhibit/idle-inhibit-unstable-v1.xml'],
  'idle.xml',
  'wlr-screencopy-unstable-v1.xml',
  'wlr-layer-shell-unstable-v1.xml',
]

wl_protos_src = []
wl_protos_headers = []

foreach p : protocols
  xml = join_paths(p)
  wl_protos_src += wayland_scanner_code.process(xml)
  wl_protos_headers += wayland_scanner_server.process(xml)
endforeach

foreach p : client_protocols
  xml = join_paths(p)
  wl_protos_headers += wayland_scanner_client.process(xml)
endforeach

lib_wl_protos = static_library('wl_protos', wl_protos_src + wl_protos_headers,
  dependencies: [wayland_server]) # for the include directory

wlr_protos = declare_dependency(
  link_with: lib_wl_protos,
  sources: wl_protos_headers,
)
