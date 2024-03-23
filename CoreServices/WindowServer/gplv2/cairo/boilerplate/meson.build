cairo_boilerplate_sources = [
  'cairo-boilerplate-getopt.c',
  'cairo-boilerplate-system.c',
  'cairo-boilerplate.c',
]

cairo_boilerplate_feature_sources = {
  'cairo-xlib': ['cairo-boilerplate-xlib.c'],
  'cairo-quartz': ['cairo-boilerplate-quartz.c'],
  'cairo-xcb': ['cairo-boilerplate-xcb.c'],
  'cairo-win32': ['cairo-boilerplate-win32.c', 'cairo-boilerplate-win32-printing.c'],
  'cairo-pdf': ['cairo-boilerplate-pdf.c'],
  'cairo-ps': ['cairo-boilerplate-ps.c'],
  'cairo-svg': ['cairo-boilerplate-svg.c'],
  'cairo-script': ['cairo-boilerplate-script.c'],
}

foreach feature: built_features
  source_key = feature.get('source-key', feature.get('name'))
  cairo_boilerplate_sources += cairo_boilerplate_feature_sources.get(source_key, [])
endforeach

cairo_boilerplate_constructors = custom_target('cairo-boilerplate-constructors.c',
  input: files(cairo_boilerplate_sources),
  output: 'cairo-boilerplate-constructors.c',
  command: [python3, files('make-cairo-boilerplate-constructors.py')[0], '@OUTPUT@', '@INPUT@'])

libcairoboilerplate = static_library('cairoboilerplate', cairo_boilerplate_sources + [cairo_boilerplate_constructors],
  include_directories: [incbase],
  dependencies: deps + [libcairo_dep],
  c_args: ['-DCAIRO_COMPILATION'],
  install: false,
  build_by_default: false,
)

cairoboilerplate_dep = declare_dependency(
  link_with: [libcairoboilerplate],
  include_directories: include_directories('.'),
)
