incmicro = include_directories('.')

subdir('micro')

gtk2_dep = dependency('gtk+-2.0', required: false)

libcairoperf = static_library('cairoperf',
  [
    'cairo-perf.c',
    'cairo-perf-report.c',
    'cairo-stats.c',
    '../src/cairo-time.c',
    'cairo-perf.h',
    'cairo-stats.h',
  ],
  include_directories: [incbase, incsrc],
  dependencies: [pixman_dep, libcairomissing_dep, cairoboilerplate_dep],
)
libcairoperf_dep = declare_dependency(
  link_with: libcairoperf,
)


analyse_trace = executable('cairo-analyse-trace',
  [
    'cairo-analyse-trace.c',
    '../src/cairo-error.c',
  ],
  include_directories: [incbase],
  dependencies: [pixman_dep, fontconfig_dep, libcairo_dep, cairoboilerplate_dep,
    libcairoscript_dep, libcairomissing_dep],
)

perf_trace = executable('cairo-perf-trace',
  [
    'cairo-perf-trace.c',
    '../src/cairo-error.c',
    '../src/cairo-hash.c',
  ],
  include_directories: [incbase, incsrc],
  dependencies: [pixman_dep, fontconfig_dep, libcairoperf_dep, libcairoscript_dep,
    cairoboilerplate_dep, libcairomissing_dep],
)

micro = executable('cairo-perf-micro',
  [ 'cairo-perf-micro.c', ],
  include_directories: [incbase, incsrc],
  dependencies: [fontconfig_dep, libcairoperf_dep, libcairoperfmicro_dep,
    cairoboilerplate_dep, libcairomissing_dep],
)

diff_files = executable('cairo-perf-diff-files',
  [
    'cairo-perf-diff-files.c',
  ],
  include_directories: [incbase, incsrc],
  dependencies: [libcairoperf_dep, cairoboilerplate_dep],
)

print = executable('cairo-perf-print',
  [
    'cairo-perf-print.c',
  ],
  include_directories: [incbase, incsrc],
  dependencies: [libcairoperf_dep, cairoboilerplate_dep],
)

chart = executable('cairo-perf-chart',
  [
    'cairo-perf-chart.c',
  ],
  include_directories: [incbase, incsrc],
  dependencies: [libcairoperf_dep, cairoboilerplate_dep],
)

compare_backends = executable('cairo-perf-compare-backends',
  [
    'cairo-perf-compare-backends.c',
  ],
  include_directories: [incbase, incsrc],
  dependencies: [libcairoperf_dep, cairoboilerplate_dep],
)

if gtk2_dep.found()
  graph_files = executable('cairo-perf-graph',
    [
      'cairo-perf-graph-files.c',
      'cairo-perf-graph-widget.c',
      'cairo-perf-graph.h',
    ],
    include_directories: [incbase, incsrc],
    dependencies: [gtk2_dep, libcairo_dep, cairoboilerplate_dep, libcairoperf_dep],
  )
endif
