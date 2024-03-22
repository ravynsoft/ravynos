# Copyright © 2020 Collabora, Ltd.
# Author: Antonio Caggiano <antonio.caggiano@collabora.com>
# Author: Robert Beckett <bob.beckett@collabora.com>
#
# SPDX-License-Identifier: MIT

pps_panfrost_sources = [
  'pan_pps_perf.cpp',
  'pan_pps_driver.cpp'
]

pps_panfrost_includes = [inc_include, inc_src, inc_tool]

pps_panfrost_lib = static_library(
  'pps-panfrost',
  sources: pps_panfrost_sources,
  include_directories: pps_panfrost_includes,
  dependencies: [dep_libdrm, dep_perfetto, libpanfrost_dep, dep_panfrost_perf],
)

compile_args_pps_panfrost = ['-DPPS_PANFROST']

pps_panfrost_dep = declare_dependency(
  link_with: pps_panfrost_lib,
  include_directories: pps_panfrost_includes,
  compile_args: compile_args_pps_panfrost
)

pps_datasources += pps_panfrost_dep
if not with_datasources.contains('panfrost')
  with_datasources += 'panfrost'
endif
