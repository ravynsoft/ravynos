cairo_fdr_sources = [
  'fdr.c',
]

libcairofdr = library('cairo-fdr', cairo_fdr_sources,
  include_directories: [incbase, incsrc],
  dependencies: deps,
  gnu_symbol_visibility: 'hidden',
  link_args: extra_link_args,
  install: true,
  install_dir: get_option('libdir') / 'cairo',
)
