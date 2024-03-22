# Copyright © 2020-2021 Collabora, Ltd.
# Author: Antonio Caggiano <antonio.caggiano@collabora.com>
#
# SPDX-License-Identifier: MIT

intel_tracepoint_files = custom_target(
  'intel_tracepoints.[ch]',
  input : 'intel_tracepoints.py',
  output : ['intel_tracepoints.h',
            'intel_tracepoints_perfetto.h',
            'intel_tracepoints.c'],
  command : [
    prog_python, '@INPUT@',
    '-p', join_paths(dir_source_root, 'src/util/perf/'),
    '--utrace-hdr', '@OUTPUT0@',
    '--perfetto-hdr', '@OUTPUT1@',
    '--utrace-src', '@OUTPUT2@',
  ],
  depend_files : u_trace_py,
)

libintel_driver_ds_deps = [
  idep_mesautil,
  idep_nir_headers,
]

if with_perfetto
  libintel_driver_ds_deps += dep_perfetto
endif

idep_intel_driver_ds_headers = declare_dependency(
  sources : intel_tracepoint_files[0],
  include_directories : [inc_include, inc_intel],
)

libintel_driver_ds = static_library(
  'intel-driver-ds',
  sources : ['intel_driver_ds.cc', intel_tracepoint_files],
  include_directories : [inc_src, inc_include, inc_intel],
  link_with : [libintel_perf, libintel_common],
  dependencies : [libintel_driver_ds_deps, idep_intel_dev],
  gnu_symbol_visibility : 'hidden',
)

idep_intel_driver_ds = declare_dependency(
  link_with : libintel_driver_ds,
  include_directories : [inc_include, inc_intel],
  dependencies : idep_intel_driver_ds_headers,
)

if with_perfetto and (with_datasources.contains('intel') or with_datasources.contains('auto'))
  pps_intel_sources = files(
    'intel_pps_perf.cc',
    'intel_pps_driver.cc',
  )

  pps_intel_lib = static_library(
    'pps-intel',
    sources: pps_intel_sources,
    include_directories: [inc_tool, inc_src, inc_include, inc_intel],
    link_with: [libintel_perf, libintel_common],
    dependencies: [dep_perfetto, dep_libdrm, idep_mesautil, idep_intel_dev],
  )

  compile_args_pps_intel = ['-DPPS_INTEL']

  pps_intel_dep = declare_dependency(
    link_with: pps_intel_lib,
    include_directories: [inc_tool, inc_include],
    compile_args: compile_args_pps_intel,
  )

  pps_datasources += pps_intel_dep
  if not with_datasources.contains('intel')
    with_datasources += 'intel'
  endif
endif
