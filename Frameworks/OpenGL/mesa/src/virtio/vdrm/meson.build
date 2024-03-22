# Copyright 2023 Google LLC
# SPDX-License-Identifier: MIT

libvdrm = static_library(
  'libvdrm',
  [
    'vdrm.c',
    'vdrm.h',
    'vdrm_virtgpu.c',
  ],
  include_directories : [
    inc_include,
    inc_src,
    inc_virtio_gpu,
  ],
  c_args : [ no_override_init_args ],
  gnu_symbol_visibility: 'hidden',
  dependencies: [
    dep_libdrm,
    dep_valgrind,
  ],
  link_with: [
    _libmesa_util,
  ],
  build_by_default: false,
)
