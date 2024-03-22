# Copyright 2017 Intel Corporation
# SPDX-License-Identifier: MIT

prog_asahi_clc = executable(
  'asahi_clc',
  ['asahi_clc.c'],
  link_with : [],
  include_directories : [inc_include, inc_src],
  c_args : [pre_args, no_override_init_args],
  link_args : [ld_args_build_id],
  dependencies : [idep_mesaclc, dep_llvm, dep_spirv_tools, idep_nir],
  native : true,
)
