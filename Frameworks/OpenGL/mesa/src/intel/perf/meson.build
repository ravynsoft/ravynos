intel_hw_metrics = [
  'hsw',
  'bdw', 'chv',
  'sklgt2', 'sklgt3', 'sklgt4',
  'kblgt2', 'kblgt3',
  'cflgt2', 'cflgt3',
  'bxt', 'glk',
  'icl', 'ehl',
  'tglgt1', 'tglgt2', 'rkl', 'dg1', 'adl',
  'acmgt1', 'acmgt2', 'acmgt3',
  'mtlgt2', 'mtlgt3',
]

intel_hw_metrics_xml_files = []
foreach hw : intel_hw_metrics
  intel_hw_metrics_xml_files += 'oa-@0@.xml'.format(hw)
endforeach

intel_perf_sources = [
  'intel_perf.c',
  'intel_perf_query.c',
  'intel_perf_mdapi.c',
]

intel_perf_sources += custom_target(
  'intel-perf-sources',
  input : intel_hw_metrics_xml_files,
  output : [ 'intel_perf_metrics.c', 'intel_perf_metrics.h' ],
  command : [
    prog_python, files('gen_perf.py'),
    '--code', '@OUTPUT0@', '--header', '@OUTPUT1@',
    '@INPUT@',
  ],
)

libintel_perf = static_library(
  'intel_perf',
  intel_perf_sources,
  include_directories : [
    inc_include, inc_src, inc_intel,
  ],
  c_args : [no_override_init_args, sse2_arg],
  cpp_args : [sse2_arg],
  gnu_symbol_visibility : 'hidden',
  dependencies : [idep_intel_dev],
)

if with_intel_tools
  intel_perf_query_layout = executable(
    'intel_perf_query_layout',
    files('intel_perf_query_layout.c'),
    dependencies : [idep_mesautil, dep_dl, dep_thread, dep_m, idep_intel_dev],
    include_directories : [inc_include, inc_src, inc_intel],
    link_with : [libintel_common, libintel_perf],
    c_args : [no_override_init_args],
    gnu_symbol_visibility : 'hidden',
    install : false
  )
endif
