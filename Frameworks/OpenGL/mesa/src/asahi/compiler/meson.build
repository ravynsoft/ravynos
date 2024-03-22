# Copyright 2018 Rob Clark
# Copyright 2019 Collabora, Ltd.
# SPDX-License-Identifier: MIT

libasahi_agx_files = files(
  'agx_compile.c',
  'agx_dce.c',
  'agx_liveness.c',
  'agx_insert_waits.c',
  'agx_nir_fence_images.c',
  'agx_nir_lower_address.c',
  'agx_nir_lower_cull_distance.c',
  'agx_nir_lower_frag_sidefx.c',
  'agx_nir_lower_sample_mask.c',
  'agx_nir_lower_discard_zs_emit.c',
  'agx_nir_lower_texture.c',
  'agx_nir_lower_interpolation.c',
  'agx_nir_lower_layer.c',
  'agx_nir_lower_load_mask.c',
  'agx_nir_lower_shared_bitsize.c',
  'agx_nir_lower_subgroups.c',
  'agx_nir_opt_preamble.c',
  'agx_lower_64bit.c',
  'agx_lower_parallel_copy.c',
  'agx_lower_pseudo.c',
  'agx_lower_uniform_sources.c',
  'agx_pack.c',
  'agx_performance.c',
  'agx_print.c',
  'agx_pressure_schedule.c',
  'agx_ir.c',
  'agx_opt_cse.c',
  'agx_opt_break_if.c',
  'agx_opt_empty_else.c',
  'agx_opt_jmp_none.c',
  'agx_optimizer.c',
  'agx_register_allocate.c',
  'agx_validate.c',
)

agx_nir_algebraic_c = custom_target(
  'agx_nir_algebraic.c',
  input : 'agx_nir_algebraic.py',
  output : 'agx_nir_algebraic.c',
  command : [
    prog_python, '@INPUT@', '-p', dir_compiler_nir,
  ],
  capture : true,
  depend_files : nir_algebraic_depends,
)

agx_opcodes_h = custom_target(
  'agx_opcodes.h',
  input : ['agx_opcodes.h.py'],
  output : 'agx_opcodes.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('agx_opcodes.py'),
)

idep_agx_opcodes_h = declare_dependency(
  sources : [agx_opcodes_h],
  include_directories : include_directories('.'),
)

agx_opcodes_c = custom_target(
  'agx_opcodes.c',
  input : ['agx_opcodes.c.py'],
  output : 'agx_opcodes.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('agx_opcodes.py'),
)

agx_builder_h = custom_target(
  'agx_builder.h',
  input : ['agx_builder.h.py'],
  output : 'agx_builder.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('agx_opcodes.py'),
)

idep_agx_builder_h = declare_dependency(
  sources : [agx_builder_h],
  include_directories : include_directories('.'),
)

libasahi_compiler = static_library(
  'asahi_compiler',
  [libasahi_agx_files, libagx_shaders, agx_opcodes_c, agx_nir_algebraic_c],
  dependencies: [idep_nir, idep_agx_opcodes_h, idep_agx_builder_h, idep_agx_pack],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

if with_tests
  test(
    'agx_tests',
    executable(
      'agx_tests',
      files(
         'test/test-minifloat.cpp',
         'test/test-optimizer.cpp',
         'test/test-lower-pseudo.cpp',
         'test/test-lower-parallel-copy.cpp',
      ),
      c_args : [c_msvc_compat_args, no_override_init_args],
      gnu_symbol_visibility : 'hidden',
      dependencies: [idep_gtest, idep_nir, idep_agx_opcodes_h, idep_agx_builder_h],
      link_with : [libasahi_compiler],
    ),
    suite : ['asahi'],
    protocol : 'gtest',
  )
endif
