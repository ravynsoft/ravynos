subdir('cairo-missing')

cairo_utils = []

if feature_conf.get('CAIRO_HAS_GOBJECT_FUNCTIONS', 0) == 1
  subdir('cairo-gobject')
endif

if conf.get('CAIRO_HAS_INTERPRETER', 0) == 1
  subdir('cairo-script')
else
  libcairoscript_dep = dependency('', required: false)
endif

if conf.get('CAIRO_HAS_TRACE', 0) == 1
  subdir('cairo-trace')

  if (conf.get('CAIRO_HAS_DLSYM', 0) == 1 and
      feature_conf.get('CAIRO_HAS_SCRIPT_SURFACE', 0) == 1 and
      feature_conf.get('CAIRO_HAS_TEE_SURFACE', 0) == 1)
    subdir('cairo-fdr')
  endif
endif

if gtk_dep.found()
  cairo_utils += [
    ['show-contour.c', {'deps': [gtk_dep]}],
    ['show-traps.c', {'deps': [gtk_dep]}],
    ['show-edges.c', {'deps': [gtk_dep]}],
    ['show-polygon.c', {'deps': [gtk_dep]}],
    ['show-events.c', {'deps': [gtk_dep]}],
    ['font-view.c', {'deps': [gtk_dep]}],
  ]
endif

foreach util : cairo_utils
  exe_name = util[0].split('.')[0]
  util_deps = util.get(1, {}).get('deps', [])
  executable(exe_name, util[0],
    include_directories: [incbase],
    dependencies: deps + util_deps + [libcairo_dep, libcairoscript_dep],
  )
endforeach

if conf.get('CAIRO_HAS_DLSYM', 0) == 1 and cc.has_header('execinfo.h')
  libmallocstats = library('malloc-stats', 'malloc-stats.c', dependencies : dl_dep)
endif
