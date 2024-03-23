cairo_gobject_sources = [
  'cairo-gobject-enums.c',
  'cairo-gobject-structs.c',
]

cairo_gobject_headers = [
  'cairo-gobject.h',
]

libcairogobject = library('cairo-gobject', cairo_gobject_sources,
  include_directories: [incbase],
  dependencies: [glib_dep, gobject_dep, libcairo_dep],
  soversion: cairo_version_sonum,
  version: cairo_libversion,
  gnu_symbol_visibility: 'hidden',
  c_args: ['-DCAIRO_COMPILATION'],
  link_args: extra_link_args,
  install: true,
)

libcairogobject_dep = declare_dependency(link_with: libcairogobject,
  include_directories: include_directories('.'),
  dependencies: libcairo_dep)

pkgmod.generate(libcairogobject,
  libraries: [libcairo, glib_dep, gobject_dep],
  description: 'cairo-gobject for cairo graphics library',
  subdirs: [meson.project_name()],
)

meson.override_dependency('cairo-gobject', libcairogobject_dep)

install_headers(cairo_gobject_headers, subdir: 'cairo')
