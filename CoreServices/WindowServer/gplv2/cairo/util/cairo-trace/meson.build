cairo_trace_sources = [
  'trace.c',
]

if conf.get('CAIRO_HAS_SYMBOL_LOOKUP', 0) == 1
  cairo_trace_sources += ['lookup-symbol.c']
endif

shared_lib_ext = libcairo.full_path().split('.')[-1]

libcairotrace = library('cairo-trace', cairo_trace_sources,
  include_directories: [incbase, incsrc],
  dependencies: deps,
  c_args: ['-DSHARED_LIB_EXT="@0@"'.format(shared_lib_ext),] + pthread_c_args,
  link_args: extra_link_args,
  gnu_symbol_visibility: 'hidden',
  install: true,
  install_dir: get_option('libdir') / 'cairo',
)

trace_conf = configuration_data()
trace_conf.set('prefix', get_option('prefix'))
trace_conf.set('exec_prefix', get_option('prefix'))
trace_conf.set('libdir', get_option('prefix') / get_option('libdir'))
trace_conf.set('CAIRO_VERSION_MAJOR', meson.project_version().split('.')[0])
trace_conf.set('CAIRO_VERSION_MINOR', meson.project_version().split('.')[1])
trace_conf.set('CAIRO_VERSION_MICRO', meson.project_version().split('.')[2])
trace_conf.set('SHLIB_EXT', shared_lib_ext)

configure_file(input: 'cairo-trace.in',
  output: 'cairo-trace',
  configuration: trace_conf,
  install: true,
  install_dir: join_paths(get_option('prefix'), get_option('bindir')),
  install_mode: 'rwxr-xr-x',
)
