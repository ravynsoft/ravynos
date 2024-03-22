# Copyright © 2020-2021 Collabora, Ltd.
# Author: Antonio Caggiano <antonio.caggiano@collabora.com>
#
# SPDX-License-Identifier: MIT

pps_sources = [
  'pps.cc',
  'pps_device.cc',
  'pps_driver.cc',
  'pps_counter.cc',
]

include_pps = include_directories('../')

dep_drm = dependency('libdrm')
pps_deps = [dep_drm, dep_perfetto]
pps_deps += pps_datasources

lib_pps = static_library(
  'pps',
  sources: pps_sources,
  include_directories: [include_pps, inc_src, pps_includes],
  dependencies: pps_deps,
)

dep_pps = declare_dependency(
  link_with: lib_pps,
  include_directories: [include_pps, inc_src]
)

producer_sources = [
  'pps_datasource.cc',
  'pps_producer.cc'
]

executable(
  'pps-producer',
  sources: producer_sources,
  include_directories: [include_pps, inc_src],
  dependencies: [dep_pps, dep_perfetto],
  install: true
)

config_sources = [
  'pps_config.cc'
]

dep_docopt = dependency('docopt', required: false)

if dep_docopt.found()
  executable(
    'pps-config',
    sources: config_sources,
    include_directories: [include_pps, inc_src],
    dependencies: [dep_pps, dep_docopt],
    install: true
  )
endif
