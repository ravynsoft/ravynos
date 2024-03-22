# Copyright 2017 Broadcom
# SPDX-License-Identifier: MIT

libasahiwinsys = static_library(
  'asahiwinsys',
  files('asahi_drm_winsys.c'),
  include_directories : [
    inc_src, inc_include,
    inc_gallium, inc_gallium_aux, inc_gallium_drivers,
  ],
  gnu_symbol_visibility : 'hidden',
  dependencies: [dep_libdrm, idep_mesautil],
)
