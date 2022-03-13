project(
  'Waybox',
  'c',
  version: '0.2.0',
  license: 'MIT',
  meson_version: '>=0.52.0',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    'werror=true',
  ],
)

add_project_arguments(
  '-g', '-O0',
  '-Wno-unused-parameter',
  '-D_DEFAULT_SOURCE',
  '-D_POSIX_C_SOURCE=200112L',
  '-DWLR_USE_UNSTABLE',
  '-DPACKAGE_NAME="' + meson.project_name() + '"',
  '-DPACKAGE_VERSION="' + meson.project_version() + '"',
  language: 'c')

cc = meson.get_compiler('c')

# Adding include directory
inc_dir = include_directories('include')

libxml2 = dependency('libxml-2.0')
wlroots = dependency('wlroots', version: '>=0.15.0')
wayland_server = dependency('wayland-server', version: '>=1.15')
wayland_protos  = dependency('wayland-protocols', version: '>=1.17')
xkbcommon  = dependency('xkbcommon')

msgfmt = find_program('msgfmt', required: false)
if msgfmt.found()
  source_root = meson.current_source_dir()
  add_project_arguments('-DUSE_NLS=1', language: 'c')
  subdir('po')
endif

subdir('data')
subdir('protocol')
subdir('waybox')
