# Copyright © 2021 Collabora, Ltd.
# Copyright © 2021 Google, Inc
#
# SPDX-License-Identifier: MIT

pps_freedreno_lib = static_library(
  'freedreno-gpu',
  sources: [
    'fd_pps_driver.cc',
    'fd_pps_driver.h',
    freedreno_xml_header_files,
  ],
  include_directories: [
    inc_tool,
    inc_src,
    inc_freedreno,
    inc_include,
  ],
  dependencies: [
    dep_libdrm,
    dep_perfetto,
    dep_valgrind,
  ],
)

pps_freedreno_dep = declare_dependency(
  link_with: [
    pps_freedreno_lib,
    libfreedreno_common,
    libfreedreno_drm,
    libfreedreno_perfcntrs,
  ],
  dependencies: [
    idep_mesautil,
  ],
  include_directories: [
    inc_tool,
    inc_src,
  ],
  compile_args: [
    '-DPPS_FREEDRENO',
  ],
)

pps_datasources += pps_freedreno_dep
pps_includes += [
  inc_include,
  inc_freedreno,
]
