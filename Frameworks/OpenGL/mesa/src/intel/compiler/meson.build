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

libintel_compiler_files = files(
  'brw_cfg.cpp',
  'brw_cfg.h',
  'brw_clip.h',
  'brw_clip_line.c',
  'brw_clip_point.c',
  'brw_clip_tri.c',
  'brw_clip_unfilled.c',
  'brw_clip_util.c',
  'brw_compile_clip.c',
  'brw_compile_ff_gs.c',
  'brw_compile_sf.c',
  'brw_compiler.c',
  'brw_compiler.h',
  'brw_dead_control_flow.cpp',
  'brw_dead_control_flow.h',
  'brw_debug_recompile.c',
  'brw_disasm.c',
  'brw_disasm_info.c',
  'brw_disasm_info.h',
  'brw_eu.c',
  'brw_eu_compact.c',
  'brw_eu_defines.h',
  'brw_eu_emit.c',
  'brw_eu.h',
  'brw_eu_util.c',
  'brw_eu_validate.c',
  'brw_fs_bank_conflicts.cpp',
  'brw_fs_builder.h',
  'brw_fs_cmod_propagation.cpp',
  'brw_fs_combine_constants.cpp',
  'brw_fs_copy_propagation.cpp',
  'brw_fs.cpp',
  'brw_fs_cse.cpp',
  'brw_fs_dead_code_eliminate.cpp',
  'brw_fs_generator.cpp',
  'brw_fs.h',
  'brw_fs_live_variables.cpp',
  'brw_fs_live_variables.h',
  'brw_fs_lower_dpas.cpp',
  'brw_fs_lower_pack.cpp',
  'brw_fs_lower_regioning.cpp',
  'brw_fs_nir.cpp',
  'brw_fs_reg_allocate.cpp',
  'brw_fs_register_coalesce.cpp',
  'brw_fs_saturate_propagation.cpp',
  'brw_fs_scoreboard.cpp',
  'brw_fs_sel_peephole.cpp',
  'brw_fs_thread_payload.cpp',
  'brw_fs_validate.cpp',
  'brw_fs_visitor.cpp',
  'brw_inst.h',
  'brw_interpolation_map.c',
  'brw_ir.h',
  'brw_ir_allocator.h',
  'brw_ir_analysis.h',
  'brw_ir_fs.h',
  'brw_ir_performance.h',
  'brw_ir_performance.cpp',
  'brw_ir_vec4.h',
  'brw_isa_info.h',
  'brw_lower_logical_sends.cpp',
  'brw_mesh.cpp',
  'brw_nir.h',
  'brw_nir.c',
  'brw_nir_analyze_boolean_resolves.c',
  'brw_nir_analyze_ubo_ranges.c',
  'brw_nir_attribute_workarounds.c',
  'brw_nir_blockify_uniform_loads.c',
  'brw_nir_clamp_per_vertex_loads.c',
  'brw_nir_lower_conversions.c',
  'brw_nir_lower_cooperative_matrix.c',
  'brw_nir_lower_cs_intrinsics.c',
  'brw_nir_lower_alpha_to_coverage.c',
  'brw_nir_lower_intersection_shader.c',
  'brw_nir_lower_non_uniform_resource_intel.c',
  'brw_nir_lower_non_uniform_barycentric_at_sample.c',
  'brw_nir_lower_ray_queries.c',
  'brw_nir_lower_rt_intrinsics.c',
  'brw_nir_lower_shader_calls.c',
  'brw_nir_lower_shading_rate_output.c',
  'brw_nir_lower_sparse.c',
  'brw_nir_lower_storage_image.c',
  'brw_nir_opt_peephole_ffma.c',
  'brw_nir_opt_peephole_imul32x16.c',
  'brw_nir_rt.h',
  'brw_nir_rt.c',
  'brw_nir_rt_builder.h',
  'brw_nir_tcs_workarounds.c',
  'brw_nir_clamp_image_1d_2d_array_sizes.c',
  'brw_packed_float.c',
  'brw_predicated_break.cpp',
  'brw_prim.h',
  'brw_private.h',
  'brw_reg.h',
  'brw_reg_type.c',
  'brw_reg_type.h',
  'brw_rt.h',
  'brw_schedule_instructions.cpp',
  'brw_shader.cpp',
  'brw_shader.h',
  'brw_simd_selection.cpp',
  'brw_vec4_builder.h',
  'brw_vec4_cmod_propagation.cpp',
  'brw_vec4_copy_propagation.cpp',
  'brw_vec4.cpp',
  'brw_vec4_cse.cpp',
  'brw_vec4_dead_code_eliminate.cpp',
  'brw_vec4_generator.cpp',
  'brw_vec4_gs_visitor.cpp',
  'brw_vec4_gs_visitor.h',
  'brw_vec4.h',
  'brw_vec4_live_variables.cpp',
  'brw_vec4_live_variables.h',
  'brw_vec4_nir.cpp',
  'brw_vec4_gs_nir.cpp',
  'brw_vec4_reg_allocate.cpp',
  'brw_vec4_surface_builder.cpp',
  'brw_vec4_surface_builder.h',
  'brw_vec4_tcs.cpp',
  'brw_vec4_tcs.h',
  'brw_vec4_tes.cpp',
  'brw_vec4_tes.h',
  'brw_vec4_visitor.cpp',
  'brw_vec4_vs_visitor.cpp',
  'brw_vec4_vs.h',
  'brw_vue_map.c',
  'gfx6_gs_visitor.cpp',
  'gfx6_gs_visitor.h',
)

brw_nir_trig = custom_target(
  'brw_nir_trig_workarounds.c',
  input : 'brw_nir_trig_workarounds.py',
  output : 'brw_nir_trig_workarounds.c',
  command : [
    prog_python, '@INPUT@', '-p', dir_compiler_nir,
  ],
  depend_files : nir_algebraic_depends,
  capture : true,
)

libintel_compiler = static_library(
  'intel_compiler',
  [libintel_compiler_files, brw_nir_trig, ir_expression_operation_h],
  include_directories : [inc_include, inc_src, inc_intel],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [idep_nir_headers, idep_mesautil, idep_intel_dev],
  build_by_default : false,
)

# For now this tool is only going to be used by Anv
if get_option('intel-clc') == 'system'
  prog_intel_clc = find_program('intel_clc', native : true)
  dep_prog_intel_clc = []
elif with_intel_clc
  prog_intel_clc = executable(
    'intel_clc',
    ['intel_clc.c', 'brw_kernel.c'],
    link_with : [
      libintel_compiler, libintel_common,libisl,
    ],
    include_directories : [inc_include, inc_src, inc_intel],
    c_args : [pre_args, no_override_init_args],
    link_args : [ld_args_build_id],
    dependencies : [idep_nir, idep_vtn, idep_mesaclc, idep_mesautil, idep_intel_dev],
    native : true,
  )
  dep_prog_intel_clc = [prog_intel_clc]
endif

if with_tests
  test(
    'intel_compiler_tests',
    executable(
      'intel_compiler_tests',
      files(
        'test_predicated_break.cpp',
        'test_eu_compact.cpp',
        'test_eu_validate.cpp',
        'test_fs_cmod_propagation.cpp',
        'test_fs_copy_propagation.cpp',
        'test_fs_saturate_propagation.cpp',
        'test_fs_scoreboard.cpp',
        'test_simd_selection.cpp',
        'test_vec4_cmod_propagation.cpp',
        'test_vec4_copy_propagation.cpp',
        'test_vec4_dead_code_eliminate.cpp',
        'test_vec4_register_coalesce.cpp',
        'test_vf_float_conversions.cpp',
      ),
      ir_expression_operation_h,
      include_directories : [inc_include, inc_src, inc_intel],
      link_with : [
        libintel_compiler, libintel_common, libisl
      ],
      dependencies : [idep_gtest, idep_nir, idep_mesautil, idep_intel_dev],
    ),
    suite : ['intel'],
    protocol : 'gtest',
  )
endif
