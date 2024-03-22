# Copyright 2017 Intel Corporation
# SPDX-License-Identifier: MIT

files_asahi = files(
  'driinfo_asahi.h',
  'agx_batch.c',
  'agx_blit.c',
  'agx_disk_cache.c',
  'agx_fence.c',
  'agx_pipe.c',
  'agx_nir_lower_sysvals.c',
  'agx_nir_lower_bindings.c',
  'agx_nir_lower_point_size.c',
  'agx_query.c',
  'agx_state.c',
  'agx_streamout.c',
  'agx_uniforms.c',
)

libasahi = static_library(
  'asahi',
  [files_asahi],
  include_directories : [inc_gallium_aux, inc_gallium, inc_include, inc_src],
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [idep_nir, idep_mesautil, idep_agx_pack, dep_libdrm, idep_mesaclc],
)

driver_asahi = declare_dependency(
  compile_args : '-DGALLIUM_ASAHI',
  link_with : [libasahi, libasahiwinsys, libasahi_compiler, libasahi_lib, libasahi_layout, libasahi_decode]
)
