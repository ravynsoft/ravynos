# Copyright © 2017 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

nir_depends = files('nir_opcodes.py', 'nir_intrinsics.py')

nir_algebraic_depends = files('nir_opcodes.py', 'nir_algebraic.py')

nir_builder_opcodes_h = custom_target(
  'nir_builder_opcodes.h',
  input : 'nir_builder_opcodes_h.py',
  output : 'nir_builder_opcodes.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : nir_depends,
)

nir_constant_expressions_c = custom_target(
  'nir_constant_expressions.c',
  input : 'nir_constant_expressions.py',
  output : 'nir_constant_expressions.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : nir_depends,
)

nir_opcodes_h = custom_target(
  'nir_opcodes.h',
  input : 'nir_opcodes_h.py',
  output : 'nir_opcodes.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : nir_depends,
)

nir_opcodes_c = custom_target(
  'nir_opcodes.c',
  input : 'nir_opcodes_c.py',
  output : 'nir_opcodes.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : nir_depends,
)

nir_opt_algebraic_c = custom_target(
  'nir_opt_algebraic.c',
  input : 'nir_opt_algebraic.py',
  output : 'nir_opt_algebraic.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : nir_algebraic_depends,
)

nir_intrinsics_h = custom_target(
  'nir_intrinsics.h',
  input : 'nir_intrinsics_h.py',
  output : 'nir_intrinsics.h',
  command : [prog_python, '@INPUT@', '--outdir', meson.current_build_dir()],
  capture : false,
  depend_files : files('nir_intrinsics.py'),
)

nir_intrinsics_indices_h = custom_target(
  'nir_intrinsics_indices.h',
  input : 'nir_intrinsics_indices_h.py',
  output : 'nir_intrinsics_indices.h',
  command : [prog_python, '@INPUT@', '--outdir', meson.current_build_dir()],
  capture : false,
  depend_files : files('nir_intrinsics.py'),
)

nir_intrinsics_c = custom_target(
  'nir_intrinsic.c',
  input : 'nir_intrinsics_c.py',
  output : 'nir_intrinsics.c',
  command : [prog_python, '@INPUT@', '--outdir', meson.current_build_dir()],
  capture: false,
  depend_files : files('nir_intrinsics.py'),
)

files_libnir = files(
  'nir.c',
  'nir.h',
  'nir_builder.c',
  'nir_builder.h',
  'nir_builtin_builder.c',
  'nir_builtin_builder.h',
  'nir_conversion_builder.h',
  'nir_clone.c',
  'nir_constant_expressions.h',
  'nir_control_flow.c',
  'nir_control_flow.h',
  'nir_control_flow_private.h',
  'nir_deref.c',
  'nir_deref.h',
  'nir_divergence_analysis.c',
  'nir_dominance.c',
  'nir_format_convert.h',
  'nir_from_ssa.c',
  'nir_functions.c',
  'nir_gather_info.c',
  'nir_gather_types.c',
  'nir_gather_xfb_info.c',
  'nir_group_loads.c',
  'nir_gs_count_vertices.c',
  'nir_inline_uniforms.c',
  'nir_instr_set.c',
  'nir_instr_set.h',
  'nir_legacy.c',
  'nir_legacy.h',
  'nir_linking_helpers.c',
  'nir_liveness.c',
  'nir_loop_analyze.c',
  'nir_loop_analyze.h',
  'nir_lower_alu.c',
  'nir_lower_alu_width.c',
  'nir_lower_alpha_test.c',
  'nir_lower_amul.c',
  'nir_lower_array_deref_of_vec.c',
  'nir_lower_atomics_to_ssbo.c',
  'nir_lower_bitmap.c',
  'nir_lower_blend.c',
  'nir_lower_bool_to_bitsize.c',
  'nir_lower_bool_to_float.c',
  'nir_lower_bool_to_int32.c',
  'nir_lower_cl_images.c',
  'nir_lower_clamp_color_outputs.c',
  'nir_lower_clip.c',
  'nir_lower_clip_cull_distance_arrays.c',
  'nir_lower_clip_disable.c',
  'nir_lower_clip_halfz.c',
  'nir_lower_const_arrays_to_uniforms.c',
  'nir_lower_continue_constructs.c',
  'nir_lower_convert_alu_types.c',
  'nir_lower_variable_initializers.c',
  'nir_lower_discard_if.c',
  'nir_lower_discard_or_demote.c',
  'nir_lower_double_ops.c',
  'nir_lower_drawpixels.c',
  'nir_lower_fb_read.c',
  'nir_lower_flatshade.c',
  'nir_lower_flrp.c',
  'nir_lower_fp16_conv.c',
  'nir_lower_fragcoord_wtrans.c',
  'nir_lower_frag_coord_to_pixel_coord.c',
  'nir_lower_fragcolor.c',
  'nir_lower_frexp.c',
  'nir_lower_global_vars_to_local.c',
  'nir_lower_goto_ifs.c',
  'nir_lower_gs_intrinsics.c',
  'nir_lower_helper_writes.c',
  'nir_lower_load_const_to_scalar.c',
  'nir_lower_locals_to_regs.c',
  'nir_lower_idiv.c',
  'nir_lower_image.c',
  'nir_lower_image_atomics_to_global.c',
  'nir_lower_indirect_derefs.c',
  'nir_lower_input_attachments.c',
  'nir_lower_int64.c',
  'nir_lower_interpolation.c',
  'nir_lower_int_to_float.c',
  'nir_lower_io.c',
  'nir_lower_io_arrays_to_elements.c',
  'nir_lower_io_to_temporaries.c',
  'nir_lower_io_to_scalar.c',
  'nir_lower_io_to_vector.c',
  'nir_lower_is_helper_invocation.c',
  'nir_lower_multiview.c',
  'nir_lower_mediump.c',
  'nir_lower_mem_access_bit_sizes.c',
  'nir_lower_memcpy.c',
  'nir_lower_memory_model.c',
  'nir_lower_non_uniform_access.c',
  'nir_lower_packing.c',
  'nir_lower_passthrough_edgeflags.c',
  'nir_lower_patch_vertices.c',
  'nir_lower_phis_to_scalar.c',
  'nir_lower_pntc_ytransform.c',
  'nir_lower_point_size.c',
  'nir_lower_point_size_mov.c',
  'nir_lower_point_smooth.c',
  'nir_lower_poly_line_smooth.c',
  'nir_lower_printf.c',
  'nir_lower_reg_intrinsics_to_ssa.c',
  'nir_lower_readonly_images_to_tex.c',
  'nir_lower_returns.c',
  'nir_lower_robust_access.c',
  'nir_lower_samplers.c',
  'nir_lower_scratch.c',
  'nir_lower_shader_calls.c',
  'nir_lower_single_sampled.c',
  'nir_lower_ssbo.c',
  'nir_lower_subgroups.c',
  'nir_lower_system_values.c',
  'nir_lower_task_shader.c',
  'nir_lower_tess_coord_z.c',
  'nir_lower_tex_shadow.c',
  'nir_lower_tex.c',
  'nir_lower_texcoord_replace.c',
  'nir_lower_texcoord_replace_late.c',
  'nir_lower_two_sided_color.c',
  'nir_lower_undef_to_zero.c',
  'nir_lower_vars_to_ssa.c',
  'nir_lower_var_copies.c',
  'nir_lower_vec_to_regs.c',
  'nir_lower_vec3_to_vec4.c',
  'nir_lower_viewport_transform.c',
  'nir_lower_wpos_center.c',
  'nir_lower_wpos_ytransform.c',
  'nir_lower_wrmasks.c',
  'nir_lower_bit_size.c',
  'nir_lower_ubo_vec4.c',
  'nir_lower_uniforms_to_ubo.c',
  'nir_lower_sysvals_to_varyings.c',
  'nir_metadata.c',
  'nir_mod_analysis.c',
  'nir_move_vec_src_uses_to_dest.c',
  'nir_normalize_cubemap_coords.c',
  'nir_opt_access.c',
  'nir_opt_barriers.c',
  'nir_opt_combine_stores.c',
  'nir_opt_comparison_pre.c',
  'nir_opt_conditional_discard.c',
  'nir_opt_constant_folding.c',
  'nir_opt_copy_prop_vars.c',
  'nir_opt_copy_propagate.c',
  'nir_opt_cse.c',
  'nir_opt_dce.c',
  'nir_opt_dead_cf.c',
  'nir_opt_dead_write_vars.c',
  'nir_opt_find_array_copies.c',
  'nir_opt_fragdepth.c',
  'nir_opt_gcm.c',
  'nir_opt_idiv_const.c',
  'nir_opt_if.c',
  'nir_opt_intrinsics.c',
  'nir_opt_large_constants.c',
  'nir_opt_load_store_vectorize.c',
  'nir_opt_loop.c',
  'nir_opt_loop_unroll.c',
  'nir_opt_memcpy.c',
  'nir_opt_move.c',
  'nir_opt_move_discards_to_top.c',
  'nir_opt_non_uniform_access.c',
  'nir_opt_offsets.c',
  'nir_opt_peephole_select.c',
  'nir_opt_phi_precision.c',
  'nir_opt_preamble.c',
  'nir_opt_ray_queries.c',
  'nir_opt_reassociate_bfi.c',
  'nir_opt_rematerialize_compares.c',
  'nir_opt_remove_phis.c',
  'nir_opt_reuse_constants.c',
  'nir_opt_shrink_stores.c',
  'nir_opt_shrink_vectors.c',
  'nir_opt_sink.c',
  'nir_opt_undef.c',
  'nir_opt_uniform_atomics.c',
  'nir_opt_vectorize.c',
  'nir_passthrough_gs.c',
  'nir_passthrough_tcs.c',
  'nir_phi_builder.c',
  'nir_phi_builder.h',
  'nir_print.c',
  'nir_propagate_invariant.c',
  'nir_range_analysis.c',
  'nir_range_analysis.h',
  'nir_remove_dead_variables.c',
  'nir_remove_tex_shadow.c',
  'nir_repair_ssa.c',
  'nir_scale_fdiv.c',
  'nir_schedule.c',
  'nir_schedule.h',
  'nir_search.c',
  'nir_search.h',
  'nir_search_helpers.h',
  'nir_serialize.c',
  'nir_serialize.h',
  'nir_split_64bit_vec3_and_vec4.c',
  'nir_split_per_member_structs.c',
  'nir_split_var_copies.c',
  'nir_split_vars.c',
  'nir_sweep.c',
  'nir_to_lcssa.c',
  'nir_trivialize_registers.c',
  'nir_validate.c',
  'nir_vla.h',
  'nir_worklist.c',
  'nir_worklist.h',
  'nir_xfb_info.h',
)

# When a file is too large, -Wmisleading-indentation will give a note about
# not being able to process it, however that is not suppressable by a #pragma
# in GCC.  This happens with the generated code in nir_opt_algebraic.c.
#
# As a workaround, drop the warning for GCC.  Clang builds should cover this.
no_misleading_indentation = []
if cc.get_id() == 'gcc'
  no_misleading_indentation += cc.get_supported_arguments('-Wno-misleading-indentation')
endif

_libnir = static_library(
  'nir',
  [files_libnir, nir_opt_algebraic_c, nir_opcodes_c,
   nir_opcodes_h, nir_constant_expressions_c, nir_builder_opcodes_h,
   nir_intrinsics_c, nir_intrinsics_h, nir_intrinsics_indices_h],
  include_directories : [inc_include, inc_src],
  c_args : [c_msvc_compat_args, no_override_init_args, no_misleading_indentation],
  gnu_symbol_visibility : 'hidden',
  dependencies : [idep_compiler, dep_valgrind],
  build_by_default : false,
)

inc_nir = include_directories('.')

# Headers-only dependency
idep_nir_headers = declare_dependency(
  sources : [nir_opcodes_h, nir_builder_opcodes_h, nir_intrinsics_h, nir_intrinsics_indices_h,
             ir_expression_operation_h, builtin_types_h],
  include_directories : [inc_nir, inc_compiler],
)

# Also link with nir
idep_nir = declare_dependency(
  dependencies : [idep_nir_headers, idep_mesautil, idep_compiler],
  link_with : _libnir,
)

if with_tests
  if cc.get_id() == 'msvc' and cc.version().version_compare('< 19.29')
    msvc_designated_initializer = 'cpp_std=c++latest'
  else
    msvc_designated_initializer = 'cpp_std=c++20'
  endif

  test(
    'nir_tests',
    executable(
      'nir_tests',
      files(
        'tests/algebraic_tests.cpp',
        'tests/builder_tests.cpp',
        'tests/comparison_pre_tests.cpp',
        'tests/control_flow_tests.cpp',
        'tests/core_tests.cpp',
        'tests/dce_tests.cpp',
        'tests/load_store_vectorizer_tests.cpp',
        'tests/loop_analyze_tests.cpp',
        'tests/loop_unroll_tests.cpp',
        'tests/lower_alu_width_tests.cpp',
        'tests/mod_analysis_tests.cpp',
        'tests/negative_equal_tests.cpp',
        'tests/opt_if_tests.cpp',
        'tests/opt_peephole_select.cpp',
        'tests/opt_shrink_vectors_tests.cpp',
        'tests/serialize_tests.cpp',
        'tests/range_analysis_tests.cpp',
        'tests/vars_tests.cpp',
      ),
      cpp_args : [cpp_msvc_compat_args],
      override_options: [msvc_designated_initializer],
      gnu_symbol_visibility : 'hidden',
      include_directories : [inc_include, inc_src],
      dependencies : [dep_thread, idep_gtest, idep_nir, idep_mesautil],
    ),
    suite : ['compiler', 'nir'],
    protocol : 'gtest',
  )

  test(
    'nir_algebraic_parser',
    prog_python,
    args : [
      files('tests/algebraic_parser_test.py')
    ],
    suite : ['compiler', 'nir'],
  )
endif
