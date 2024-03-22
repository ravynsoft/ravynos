/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "radv_shader.h"
#include "meta/radv_meta.h"
#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "nir/nir_xfb_info.h"
#include "nir/radv_nir.h"
#include "spirv/nir_spirv.h"
#include "util/memstream.h"
#include "util/mesa-sha1.h"
#include "util/streaming-load-memcpy.h"
#include "util/u_atomic.h"
#include "radv_cs.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "radv_sdma.h"
#include "radv_shader_args.h"

#include "util/u_debug.h"
#include "ac_binary.h"
#include "ac_nir.h"
#if defined(USE_LIBELF)
#include "ac_rtld.h"
#endif
#include "aco_interface.h"
#include "sid.h"
#include "vk_format.h"
#include "vk_nir.h"
#include "vk_semaphore.h"
#include "vk_sync.h"

#include "aco_shader_info.h"
#include "radv_aco_shader_info.h"
#if LLVM_AVAILABLE
#include "ac_llvm_util.h"
#endif

static void
get_nir_options_for_stage(struct radv_physical_device *device, gl_shader_stage stage)
{
   bool split_fma = (stage <= MESA_SHADER_GEOMETRY || stage == MESA_SHADER_MESH) &&
                    device->instance->debug_flags & RADV_DEBUG_SPLIT_FMA;
   device->nir_options[stage] = (nir_shader_compiler_options){
      .vertex_id_zero_based = true,
      .lower_scmp = true,
      .lower_flrp16 = true,
      .lower_flrp32 = true,
      .lower_flrp64 = true,
      .lower_device_index_to_zero = true,
      .lower_fdiv = true,
      .lower_fmod = true,
      .lower_ineg = true,
      .lower_bitfield_insert = true,
      .lower_bitfield_extract = true,
      .lower_pack_snorm_4x8 = true,
      .lower_pack_unorm_4x8 = true,
      .lower_pack_half_2x16 = true,
      .lower_pack_64_2x32 = true,
      .lower_pack_64_4x16 = true,
      .lower_pack_32_2x16 = true,
      .lower_unpack_snorm_2x16 = true,
      .lower_unpack_snorm_4x8 = true,
      .lower_unpack_unorm_2x16 = true,
      .lower_unpack_unorm_4x8 = true,
      .lower_unpack_half_2x16 = true,
      .lower_ffma16 = split_fma || device->rad_info.gfx_level < GFX9,
      .lower_ffma32 = split_fma || device->rad_info.gfx_level < GFX10_3,
      .lower_ffma64 = split_fma,
      .lower_fpow = true,
      .lower_mul_2x32_64 = true,
      .lower_iadd_sat = device->rad_info.gfx_level <= GFX8,
      .lower_hadd = true,
      .lower_mul_32x16 = true,
      .has_bfe = true,
      .has_bfm = true,
      .has_bitfield_select = true,
      .has_fsub = true,
      .has_isub = true,
      .has_sdot_4x8 = device->rad_info.has_accelerated_dot_product,
      .has_sudot_4x8 = device->rad_info.has_accelerated_dot_product && device->rad_info.gfx_level >= GFX11,
      .has_udot_4x8 = device->rad_info.has_accelerated_dot_product,
      .has_sdot_4x8_sat = device->rad_info.has_accelerated_dot_product,
      .has_sudot_4x8_sat = device->rad_info.has_accelerated_dot_product && device->rad_info.gfx_level >= GFX11,
      .has_udot_4x8_sat = device->rad_info.has_accelerated_dot_product,
      .has_dot_2x16 = device->rad_info.has_accelerated_dot_product && device->rad_info.gfx_level < GFX11,
      .has_find_msb_rev = true,
      .has_pack_half_2x16_rtz = true,
      .has_bit_test = !device->use_llvm,
      .has_fmulz = true,
      .has_msad = true,
      .max_unroll_iterations = 32,
      .max_unroll_iterations_aggressive = 128,
      .use_interpolated_input_intrinsics = true,
      .vectorize_vec2_16bit = true,
      .lower_int64_options = nir_lower_imul64 | nir_lower_imul_high64 | nir_lower_imul_2x32_64 | nir_lower_divmod64 |
                             nir_lower_minmax64 | nir_lower_iabs64 | nir_lower_iadd_sat64 | nir_lower_conv64,
      .lower_doubles_options = nir_lower_drcp | nir_lower_dsqrt | nir_lower_drsq | nir_lower_ddiv,
      .divergence_analysis_options = nir_divergence_view_index_uniform,
      .optimize_quad_vote_to_reduce = true,
   };
}

void
radv_get_nir_options(struct radv_physical_device *device)
{
   for (gl_shader_stage stage = MESA_SHADER_VERTEX; stage < MESA_VULKAN_SHADER_STAGES; stage++)
      get_nir_options_for_stage(device, stage);
}

static uint8_t
vectorize_vec2_16bit(const nir_instr *instr, const void *_)
{
   if (instr->type != nir_instr_type_alu)
      return 0;

   const nir_alu_instr *alu = nir_instr_as_alu(instr);
   const unsigned bit_size = alu->def.bit_size;
   if (bit_size == 16)
      return 2;
   else
      return 1;
}

static bool
is_meta_shader(nir_shader *nir)
{
   return nir && nir->info.internal;
}

bool
radv_can_dump_shader(struct radv_device *device, nir_shader *nir, bool meta_shader)
{
   if (!(device->instance->debug_flags & RADV_DEBUG_DUMP_SHADERS))
      return false;

   if ((is_meta_shader(nir) || meta_shader) && !(device->instance->debug_flags & RADV_DEBUG_DUMP_META_SHADERS))
      return false;

   return true;
}

bool
radv_can_dump_shader_stats(struct radv_device *device, nir_shader *nir)
{
   /* Only dump non-meta shader stats. */
   return device->instance->debug_flags & RADV_DEBUG_DUMP_SHADER_STATS && !is_meta_shader(nir);
}

void
radv_optimize_nir(struct nir_shader *shader, bool optimize_conservatively)
{
   bool progress;

   struct set *skip = _mesa_pointer_set_create(NULL);
   do {
      progress = false;

      NIR_LOOP_PASS(progress, skip, shader, nir_split_array_vars, nir_var_function_temp);
      NIR_LOOP_PASS(progress, skip, shader, nir_shrink_vec_array_vars, nir_var_function_temp);

      if (!shader->info.var_copies_lowered) {
         /* Only run this pass if nir_lower_var_copies was not called
          * yet. That would lower away any copy_deref instructions and we
          * don't want to introduce any more.
          */
         NIR_LOOP_PASS(progress, skip, shader, nir_opt_find_array_copies);
      }

      NIR_LOOP_PASS(progress, skip, shader, nir_opt_copy_prop_vars);
      NIR_LOOP_PASS(progress, skip, shader, nir_opt_dead_write_vars);
      NIR_LOOP_PASS(_, skip, shader, nir_lower_vars_to_ssa);

      NIR_LOOP_PASS(_, skip, shader, nir_lower_alu_width, vectorize_vec2_16bit, NULL);
      NIR_LOOP_PASS(_, skip, shader, nir_lower_phis_to_scalar, true);

      NIR_LOOP_PASS(progress, skip, shader, nir_copy_prop);
      NIR_LOOP_PASS(progress, skip, shader, nir_opt_remove_phis);
      NIR_LOOP_PASS(progress, skip, shader, nir_opt_dce);
      bool opt_loop_progress = false;
      NIR_LOOP_PASS(opt_loop_progress, skip, shader, nir_opt_loop);
      if (opt_loop_progress) {
         progress = true;
         NIR_LOOP_PASS(progress, skip, shader, nir_copy_prop);
         NIR_LOOP_PASS(progress, skip, shader, nir_opt_remove_phis);
         NIR_LOOP_PASS(progress, skip, shader, nir_opt_dce);
      }
      NIR_LOOP_PASS_NOT_IDEMPOTENT(progress, skip, shader, nir_opt_if, nir_opt_if_optimize_phi_true_false);
      NIR_LOOP_PASS(progress, skip, shader, nir_opt_dead_cf);
      NIR_LOOP_PASS(progress, skip, shader, nir_opt_cse);
      NIR_LOOP_PASS(progress, skip, shader, nir_opt_peephole_select, 8, true, true);
      NIR_LOOP_PASS(progress, skip, shader, nir_opt_constant_folding);
      NIR_LOOP_PASS(progress, skip, shader, nir_opt_intrinsics);
      NIR_LOOP_PASS_NOT_IDEMPOTENT(progress, skip, shader, nir_opt_algebraic);

      NIR_LOOP_PASS(progress, skip, shader, nir_opt_undef);

      if (shader->options->max_unroll_iterations) {
         NIR_LOOP_PASS_NOT_IDEMPOTENT(progress, skip, shader, nir_opt_loop_unroll);
      }
   } while (progress && !optimize_conservatively);
   _mesa_set_destroy(skip, NULL);

   NIR_PASS(progress, shader, nir_opt_shrink_vectors);
   NIR_PASS(progress, shader, nir_remove_dead_variables, nir_var_function_temp | nir_var_shader_in | nir_var_shader_out,
            NULL);

   if (shader->info.stage == MESA_SHADER_FRAGMENT && (shader->info.fs.uses_discard || shader->info.fs.uses_demote)) {
      NIR_PASS(progress, shader, nir_opt_conditional_discard);
      NIR_PASS(progress, shader, nir_opt_move_discards_to_top);
   }

   NIR_PASS(progress, shader, nir_opt_move, nir_move_load_ubo);
}

void
radv_optimize_nir_algebraic(nir_shader *nir, bool opt_offsets)
{
   bool more_algebraic = true;
   while (more_algebraic) {
      more_algebraic = false;
      NIR_PASS(_, nir, nir_copy_prop);
      NIR_PASS(_, nir, nir_opt_dce);
      NIR_PASS(_, nir, nir_opt_constant_folding);
      NIR_PASS(_, nir, nir_opt_cse);
      NIR_PASS(more_algebraic, nir, nir_opt_algebraic);
   }

   if (opt_offsets) {
      static const nir_opt_offsets_options offset_options = {
         .uniform_max = 0,
         .buffer_max = ~0,
         .shared_max = ~0,
      };
      NIR_PASS(_, nir, nir_opt_offsets, &offset_options);
   }

   /* Do late algebraic optimization to turn add(a,
    * neg(b)) back into subs, then the mandatory cleanup
    * after algebraic.  Note that it may produce fnegs,
    * and if so then we need to keep running to squash
    * fneg(fneg(a)).
    */
   bool more_late_algebraic = true;
   struct set *skip = _mesa_pointer_set_create(NULL);
   while (more_late_algebraic) {
      more_late_algebraic = false;
      NIR_LOOP_PASS_NOT_IDEMPOTENT(more_late_algebraic, skip, nir, nir_opt_algebraic_late);
      NIR_LOOP_PASS(_, skip, nir, nir_opt_constant_folding);
      NIR_LOOP_PASS(_, skip, nir, nir_copy_prop);
      NIR_LOOP_PASS(_, skip, nir, nir_opt_dce);
      NIR_LOOP_PASS(_, skip, nir, nir_opt_cse);
   }
   _mesa_set_destroy(skip, NULL);
}

static void
shared_var_info(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   assert(glsl_type_is_vector_or_scalar(type));

   uint32_t comp_size = glsl_type_is_boolean(type) ? 4 : glsl_get_bit_size(type) / 8;
   unsigned length = glsl_get_vector_elements(type);
   *size = comp_size * length, *align = comp_size;
}

struct radv_shader_debug_data {
   struct radv_device *device;
   const struct vk_object_base *object;
};

static void
radv_spirv_nir_debug(void *private_data, enum nir_spirv_debug_level level, size_t spirv_offset, const char *message)
{
   struct radv_shader_debug_data *debug_data = private_data;
   struct radv_instance *instance = debug_data->device->instance;

   static const VkDebugReportFlagsEXT vk_flags[] = {
      [NIR_SPIRV_DEBUG_LEVEL_INFO] = VK_DEBUG_REPORT_INFORMATION_BIT_EXT,
      [NIR_SPIRV_DEBUG_LEVEL_WARNING] = VK_DEBUG_REPORT_WARNING_BIT_EXT,
      [NIR_SPIRV_DEBUG_LEVEL_ERROR] = VK_DEBUG_REPORT_ERROR_BIT_EXT,
   };
   char buffer[256];

   snprintf(buffer, sizeof(buffer), "SPIR-V offset %lu: %s", (unsigned long)spirv_offset, message);

   vk_debug_report(&instance->vk, vk_flags[level], debug_data->object, 0, 0, "radv", buffer);
}

static void
radv_compiler_debug(void *private_data, enum aco_compiler_debug_level level, const char *message)
{
   struct radv_shader_debug_data *debug_data = private_data;
   struct radv_instance *instance = debug_data->device->instance;

   static const VkDebugReportFlagsEXT vk_flags[] = {
      [ACO_COMPILER_DEBUG_LEVEL_PERFWARN] = VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
      [ACO_COMPILER_DEBUG_LEVEL_ERROR] = VK_DEBUG_REPORT_ERROR_BIT_EXT,
   };

   /* VK_DEBUG_REPORT_DEBUG_BIT_EXT specifies diagnostic information
    * from the implementation and layers.
    */
   vk_debug_report(&instance->vk, vk_flags[level] | VK_DEBUG_REPORT_DEBUG_BIT_EXT, NULL, 0, 0, "radv", message);
}

/* If the shader doesn't have an index=1 output, then assume that it meant for a location=1 to be used. This works on
 * some older hardware because the MRT1 target is used for both location=1 and index=1, but GFX11 works differently.
 */
static void
fix_dual_src_mrt1_export(nir_shader *nir)
{
   nir_foreach_shader_out_variable (var, nir) {
      if (var->data.location == FRAG_RESULT_DATA0 && var->data.index == 1)
         return;
   }

   nir_variable *loc1_var = nir_find_variable_with_location(nir, nir_var_shader_out, FRAG_RESULT_DATA1);
   if (loc1_var) {
      loc1_var->data.location = FRAG_RESULT_DATA0;
      loc1_var->data.index = 1;
   }
}

nir_shader *
radv_shader_spirv_to_nir(struct radv_device *device, const struct radv_shader_stage *stage,
                         const struct radv_pipeline_key *key, bool is_internal)
{
   unsigned subgroup_size = 64, ballot_bit_size = 64;
   const unsigned required_subgroup_size = key->stage_info[stage->stage].subgroup_required_size * 32;
   if (required_subgroup_size) {
      /* Only compute/mesh/task shaders currently support requiring a
       * specific subgroup size.
       */
      assert(stage->stage >= MESA_SHADER_COMPUTE);
      subgroup_size = required_subgroup_size;
      ballot_bit_size = required_subgroup_size;
   }

   nir_shader *nir;

   if (stage->internal_nir) {
      /* Some things such as our meta clear/blit code will give us a NIR
       * shader directly.  In that case, we just ignore the SPIR-V entirely
       * and just use the NIR shader.  We don't want to alter meta and RT
       * shaders IR directly, so clone it first. */
      nir = nir_shader_clone(NULL, stage->internal_nir);
      nir_validate_shader(nir, "in internal shader");

      assert(exec_list_length(&nir->functions) == 1);
   } else {
      uint32_t *spirv = (uint32_t *)stage->spirv.data;
      assert(stage->spirv.size % 4 == 0);

      bool dump_meta = device->instance->debug_flags & RADV_DEBUG_DUMP_META_SHADERS;
      if ((device->instance->debug_flags & RADV_DEBUG_DUMP_SPIRV) && (!is_internal || dump_meta))
         radv_print_spirv(stage->spirv.data, stage->spirv.size, stderr);

      uint32_t num_spec_entries = 0;
      struct nir_spirv_specialization *spec_entries = vk_spec_info_to_nir_spirv(stage->spec_info, &num_spec_entries);
      struct radv_shader_debug_data spirv_debug_data = {
         .device = device,
         .object = stage->spirv.object,
      };
      const bool has_fragment_shader_interlock = radv_has_pops(device->physical_device);
      const struct spirv_to_nir_options spirv_options = {
         .caps =
            {
               .amd_fragment_mask = true,
               .amd_gcn_shader = true,
               .amd_image_gather_bias_lod = true,
               .amd_image_read_write_lod = true,
               .amd_shader_ballot = true,
               .amd_shader_explicit_vertex_parameter = true,
               .amd_trinary_minmax = true,
               .demote_to_helper_invocation = true,
               .derivative_group = true,
               .descriptor_array_dynamic_indexing = true,
               .descriptor_array_non_uniform_indexing = true,
               .descriptor_indexing = true,
               .device_group = true,
               .draw_parameters = true,
               .float_controls = true,
               .float16 = device->physical_device->rad_info.has_packed_math_16bit,
               .float32_atomic_add = true,
               .float32_atomic_min_max = true,
               .float64 = true,
               .float64_atomic_min_max = true,
               .fragment_barycentric = true,
               .fragment_fully_covered = true,
               .fragment_shader_pixel_interlock = has_fragment_shader_interlock,
               .fragment_shader_sample_interlock = has_fragment_shader_interlock,
               .geometry_streams = true,
               .groups = true,
               .image_atomic_int64 = true,
               .image_ms_array = true,
               .image_read_without_format = true,
               .image_write_without_format = true,
               .int8 = true,
               .int16 = true,
               .int64 = true,
               .int64_atomics = true,
               .integer_functions2 = true,
               .mesh_shading = true,
               .min_lod = true,
               .multiview = true,
               .physical_storage_buffer_address = true,
               .post_depth_coverage = true,
               .ray_cull_mask = true,
               .ray_query = true,
               .ray_tracing = true,
               .ray_tracing_position_fetch = true,
               .ray_traversal_primitive_culling = true,
               .runtime_descriptor_array = true,
               .shader_clock = true,
               .shader_viewport_index_layer = true,
               .sparse_residency = true,
               .stencil_export = true,
               .storage_8bit = true,
               .storage_16bit = true,
               .storage_image_ms = true,
               .subgroup_arithmetic = true,
               .subgroup_ballot = true,
               .subgroup_basic = true,
               .subgroup_quad = true,
               .subgroup_shuffle = true,
               .subgroup_uniform_control_flow = true,
               .subgroup_vote = true,
               .tessellation = true,
               .transform_feedback = true,
               .variable_pointers = true,
               .vk_memory_model = true,
               .vk_memory_model_device_scope = true,
               .fragment_shading_rate = device->physical_device->rad_info.gfx_level >= GFX10_3,
               .workgroup_memory_explicit_layout = true,
               .cooperative_matrix = true,
            },
         .ubo_addr_format = nir_address_format_vec2_index_32bit_offset,
         .ssbo_addr_format = nir_address_format_vec2_index_32bit_offset,
         .phys_ssbo_addr_format = nir_address_format_64bit_global,
         .push_const_addr_format = nir_address_format_logical,
         .shared_addr_format = nir_address_format_32bit_offset,
         .constant_addr_format = nir_address_format_64bit_global,
         .debug =
            {
               .func = radv_spirv_nir_debug,
               .private_data = &spirv_debug_data,
            },
         .force_tex_non_uniform = device->cache_key.tex_non_uniform,
         .force_ssbo_non_uniform = device->cache_key.ssbo_non_uniform,
      };
      nir = spirv_to_nir(spirv, stage->spirv.size / 4, spec_entries, num_spec_entries, stage->stage, stage->entrypoint,
                         &spirv_options, &device->physical_device->nir_options[stage->stage]);
      nir->info.internal |= is_internal;
      assert(nir->info.stage == stage->stage);
      nir_validate_shader(nir, "after spirv_to_nir");

      free(spec_entries);

      /* TODO: This can be removed once GCM (which is more general) is used. */
      NIR_PASS(_, nir, nir_opt_reuse_constants);

      const struct nir_lower_sysvals_to_varyings_options sysvals_to_varyings = {
         .point_coord = true,
      };
      NIR_PASS_V(nir, nir_lower_sysvals_to_varyings, &sysvals_to_varyings);

      /* We have to lower away local constant initializers right before we
       * inline functions.  That way they get properly initialized at the top
       * of the function and not at the top of its caller.
       */
      NIR_PASS(_, nir, nir_lower_variable_initializers, nir_var_function_temp);
      NIR_PASS(_, nir, nir_lower_returns);
      bool progress = false;
      NIR_PASS(progress, nir, nir_inline_functions);
      if (progress) {
         NIR_PASS(_, nir, nir_opt_copy_prop_vars);
         NIR_PASS(_, nir, nir_copy_prop);
      }
      NIR_PASS(_, nir, nir_opt_deref);

      /* Pick off the single entrypoint that we want */
      nir_remove_non_entrypoints(nir);

      /* Make sure we lower constant initializers on output variables so that
       * nir_remove_dead_variables below sees the corresponding stores
       */
      NIR_PASS(_, nir, nir_lower_variable_initializers, nir_var_shader_out);

      /* Now that we've deleted all but the main function, we can go ahead and
       * lower the rest of the constant initializers.
       */
      NIR_PASS(_, nir, nir_lower_variable_initializers, ~0);

      NIR_PASS(_, nir, radv_nir_lower_cooperative_matrix, subgroup_size);

      /* Split member structs.  We do this before lower_io_to_temporaries so that
       * it doesn't lower system values to temporaries by accident.
       */
      NIR_PASS(_, nir, nir_split_var_copies);
      NIR_PASS(_, nir, nir_split_per_member_structs);

      if (nir->info.stage == MESA_SHADER_FRAGMENT)
         NIR_PASS(_, nir, nir_lower_io_to_vector, nir_var_shader_out);
      if (nir->info.stage == MESA_SHADER_FRAGMENT)
         NIR_PASS(_, nir, nir_lower_input_attachments,
                  &(nir_input_attachment_options){
                     .use_fragcoord_sysval = true,
                     .use_layer_id_sysval = false,
                  });

      nir_remove_dead_variables_options dead_vars_opts = {
         .can_remove_var = nir_vk_is_not_xfb_output,
      };
      NIR_PASS(_, nir, nir_remove_dead_variables,
               nir_var_shader_in | nir_var_shader_out | nir_var_system_value | nir_var_mem_shared, &dead_vars_opts);

      if (nir->info.stage == MESA_SHADER_FRAGMENT && key->ps.epilog.mrt0_is_dual_src &&
          device->instance->drirc.dual_color_blend_by_location)
         fix_dual_src_mrt1_export(nir);

      /* Variables can make nir_propagate_invariant more conservative
       * than it needs to be.
       */
      NIR_PASS(_, nir, nir_lower_global_vars_to_local);

      NIR_PASS(_, nir, nir_lower_vars_to_ssa);

      NIR_PASS(_, nir, nir_propagate_invariant, device->cache_key.invariant_geom);

      NIR_PASS(_, nir, nir_lower_clip_cull_distance_arrays);

      if (nir->info.stage == MESA_SHADER_VERTEX || nir->info.stage == MESA_SHADER_TESS_EVAL ||
          nir->info.stage == MESA_SHADER_GEOMETRY)
         NIR_PASS_V(nir, nir_shader_gather_xfb_info);

      NIR_PASS(_, nir, nir_lower_discard_or_demote, device->cache_key.lower_discard_to_demote);

      nir_lower_doubles_options lower_doubles = nir->options->lower_doubles_options;

      if (device->physical_device->rad_info.gfx_level == GFX6) {
         /* GFX6 doesn't support v_floor_f64 and the precision
          * of v_fract_f64 which is used to implement 64-bit
          * floor is less than what Vulkan requires.
          */
         lower_doubles |= nir_lower_dfloor;
      }

      NIR_PASS(_, nir, nir_lower_doubles, NULL, lower_doubles);

      NIR_PASS(_, nir, ac_nir_lower_sin_cos);
   }

   NIR_PASS(_, nir, nir_lower_system_values);
   nir_lower_compute_system_values_options csv_options = {
      /* Mesh shaders run as NGG which can implement local_invocation_index from
       * the wave ID in merged_wave_info, but they don't have local_invocation_ids on GFX10.3.
       */
      .lower_cs_local_id_to_index = nir->info.stage == MESA_SHADER_MESH && !device->mesh_fast_launch_2,
      .lower_local_invocation_index = nir->info.stage == MESA_SHADER_COMPUTE &&
                                      ((nir->info.workgroup_size[0] == 1) + (nir->info.workgroup_size[1] == 1) +
                                       (nir->info.workgroup_size[2] == 1)) == 2,
   };
   NIR_PASS(_, nir, nir_lower_compute_system_values, &csv_options);

   /* Vulkan uses the separate-shader linking model */
   nir->info.separate_shader = true;

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   if (nir->info.ray_queries > 0) {
      /* Lower shared variables early to prevent the over allocation of shared memory in
       * radv_nir_lower_ray_queries.  */
      if (nir->info.stage == MESA_SHADER_COMPUTE) {
         if (!nir->info.shared_memory_explicit_layout)
            NIR_PASS(_, nir, nir_lower_vars_to_explicit_types, nir_var_mem_shared, shared_var_info);

         NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_shared, nir_address_format_32bit_offset);
      }

      NIR_PASS(_, nir, nir_opt_ray_queries);
      NIR_PASS(_, nir, nir_opt_ray_query_ranges);
      NIR_PASS(_, nir, radv_nir_lower_ray_queries, device);
   }

   nir_lower_tex_options tex_options = {
      .lower_txp = ~0,
      .lower_txf_offset = true,
      .lower_tg4_offsets = true,
      .lower_txs_cube_array = true,
      .lower_to_fragment_fetch_amd = device->physical_device->use_fmask,
      .lower_lod_zero_width = true,
      .lower_invalid_implicit_lod = true,
      .lower_1d = device->physical_device->rad_info.gfx_level == GFX9,
   };

   NIR_PASS(_, nir, nir_lower_tex, &tex_options);

   static const nir_lower_image_options image_options = {
      .lower_cube_size = true,
   };

   NIR_PASS(_, nir, nir_lower_image, &image_options);

   NIR_PASS(_, nir, nir_lower_vars_to_ssa);

   if (nir->info.stage == MESA_SHADER_VERTEX || nir->info.stage == MESA_SHADER_GEOMETRY ||
       nir->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(nir, nir_lower_io_to_temporaries, nir_shader_get_entrypoint(nir), true, true);
   } else if (nir->info.stage == MESA_SHADER_TESS_EVAL) {
      NIR_PASS_V(nir, nir_lower_io_to_temporaries, nir_shader_get_entrypoint(nir), true, false);
   }

   NIR_PASS(_, nir, nir_split_var_copies);

   NIR_PASS(_, nir, nir_lower_global_vars_to_local);
   NIR_PASS(_, nir, nir_remove_dead_variables, nir_var_function_temp, NULL);

   bool gfx7minus = device->physical_device->rad_info.gfx_level <= GFX7;
   bool has_inverse_ballot = true;
#if LLVM_AVAILABLE
   has_inverse_ballot = !radv_use_llvm_for_stage(device, nir->info.stage) || LLVM_VERSION_MAJOR >= 17;
#endif

   NIR_PASS(_, nir, nir_lower_subgroups,
            &(struct nir_lower_subgroups_options){
               .subgroup_size = subgroup_size,
               .ballot_bit_size = ballot_bit_size,
               .ballot_components = 1,
               .lower_to_scalar = 1,
               .lower_subgroup_masks = 1,
               .lower_relative_shuffle = 1,
               .lower_shuffle_to_32bit = 1,
               .lower_vote_eq = 1,
               .lower_vote_bool_eq = 1,
               .lower_quad_broadcast_dynamic = 1,
               .lower_quad_broadcast_dynamic_to_const = gfx7minus,
               .lower_shuffle_to_swizzle_amd = 1,
               .lower_ballot_bit_count_to_mbcnt_amd = 1,
               .lower_inverse_ballot = !has_inverse_ballot,
               .lower_boolean_reduce = 1,
            });

   NIR_PASS(_, nir, nir_lower_load_const_to_scalar);
   NIR_PASS(_, nir, nir_opt_shrink_stores, !device->instance->drirc.disable_shrink_image_store);

   if (!key->optimisations_disabled)
      radv_optimize_nir(nir, false);

   /* We call nir_lower_var_copies() after the first radv_optimize_nir()
    * to remove any copies introduced by nir_opt_find_array_copies().
    */
   NIR_PASS(_, nir, nir_lower_var_copies);

   unsigned lower_flrp = (nir->options->lower_flrp16 ? 16 : 0) | (nir->options->lower_flrp32 ? 32 : 0) |
                         (nir->options->lower_flrp64 ? 64 : 0);
   if (lower_flrp != 0) {
      bool progress = false;
      NIR_PASS(progress, nir, nir_lower_flrp, lower_flrp, false /* always precise */);
      if (progress)
         NIR_PASS(_, nir, nir_opt_constant_folding);
   }

   const nir_opt_access_options opt_access_options = {
      .is_vulkan = true,
   };
   NIR_PASS(_, nir, nir_opt_access, &opt_access_options);

   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_push_const, nir_address_format_32bit_offset);

   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_ubo | nir_var_mem_ssbo,
            nir_address_format_vec2_index_32bit_offset);

   NIR_PASS(_, nir, radv_nir_lower_intrinsics_early, key);

   /* Lower deref operations for compute shared memory. */
   if (nir->info.stage == MESA_SHADER_COMPUTE || nir->info.stage == MESA_SHADER_TASK ||
       nir->info.stage == MESA_SHADER_MESH) {
      nir_variable_mode var_modes = nir_var_mem_shared;

      if (nir->info.stage == MESA_SHADER_TASK || nir->info.stage == MESA_SHADER_MESH)
         var_modes |= nir_var_mem_task_payload;

      if (!nir->info.shared_memory_explicit_layout)
         NIR_PASS(_, nir, nir_lower_vars_to_explicit_types, var_modes, shared_var_info);
      else if (var_modes & ~nir_var_mem_shared)
         NIR_PASS(_, nir, nir_lower_vars_to_explicit_types, var_modes & ~nir_var_mem_shared, shared_var_info);
      NIR_PASS(_, nir, nir_lower_explicit_io, var_modes, nir_address_format_32bit_offset);

      if (nir->info.zero_initialize_shared_memory && nir->info.shared_size > 0) {
         const unsigned chunk_size = 16; /* max single store size */
         const unsigned shared_size = ALIGN(nir->info.shared_size, chunk_size);
         NIR_PASS(_, nir, nir_zero_initialize_shared_memory, shared_size, chunk_size);
      }
   }

   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_global | nir_var_mem_constant, nir_address_format_64bit_global);

   /* Lower large variables that are always constant with load_constant
    * intrinsics, which get turned into PC-relative loads from a data
    * section next to the shader.
    */
   NIR_PASS(_, nir, nir_opt_large_constants, glsl_get_natural_size_align_bytes, 16);

   /* Lower primitive shading rate to match HW requirements. */
   if ((nir->info.stage == MESA_SHADER_VERTEX || nir->info.stage == MESA_SHADER_GEOMETRY ||
        nir->info.stage == MESA_SHADER_MESH) &&
       nir->info.outputs_written & BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_SHADING_RATE)) {
      /* Lower primitive shading rate to match HW requirements. */
      NIR_PASS(_, nir, radv_nir_lower_primitive_shading_rate, device->physical_device->rad_info.gfx_level);
   }

   /* Indirect lowering must be called after the radv_optimize_nir() loop
    * has been called at least once. Otherwise indirect lowering can
    * bloat the instruction count of the loop and cause it to be
    * considered too large for unrolling.
    */
   if (ac_nir_lower_indirect_derefs(nir, device->physical_device->rad_info.gfx_level) && !key->optimisations_disabled &&
       nir->info.stage != MESA_SHADER_COMPUTE) {
      /* Optimize the lowered code before the linking optimizations. */
      radv_optimize_nir(nir, false);
   }

   return nir;
}

bool
radv_consider_culling(const struct radv_physical_device *pdevice, struct nir_shader *nir, uint64_t ps_inputs_read,
                      unsigned num_vertices_per_primitive, const struct radv_shader_info *info)
{
   /* Culling doesn't make sense for meta shaders. */
   if (is_meta_shader(nir))
      return false;

   /* We don't support culling with multiple viewports yet. */
   if (nir->info.outputs_written & (VARYING_BIT_VIEWPORT | VARYING_BIT_VIEWPORT_MASK))
      return false;

   /* We don't support culling with vertex shader prologs. */
   if (info->vs.has_prolog)
      return false;

   if (!pdevice->use_ngg_culling)
      return false;

   /* Shader based culling efficiency can depend on PS throughput.
    * Estimate an upper limit for PS input param count based on GPU info.
    */
   unsigned max_ps_params;
   unsigned max_render_backends = pdevice->rad_info.max_render_backends;
   unsigned max_se = pdevice->rad_info.max_se;

   if (max_render_backends / max_se == 4)
      max_ps_params = 6; /* Navi21 and other GFX10.3 dGPUs. */
   else
      max_ps_params = 4; /* Navi 1x. */

   /* TODO: consider other heuristics here, such as PS execution time */
   if (util_bitcount64(ps_inputs_read & ~VARYING_BIT_POS) > max_ps_params)
      return false;

   /* Only triangle culling is supported. */
   if (num_vertices_per_primitive != 3)
      return false;

   /* When the shader writes memory, it is difficult to guarantee correctness.
    * Future work:
    * - if only write-only SSBOs are used
    * - if we can prove that non-position outputs don't rely on memory stores
    * then may be okay to keep the memory stores in the 1st shader part, and delete them from the 2nd.
    */
   if (nir->info.writes_memory)
      return false;

   /* When the shader relies on the subgroup invocation ID, we'd break it, because the ID changes after the culling.
    * Future work: try to save this to LDS and reload, but it can still be broken in subtle ways.
    */
   if (BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_SUBGROUP_INVOCATION))
      return false;

   return true;
}

static void
setup_ngg_lds_layout(struct radv_device *device, nir_shader *nir, struct radv_shader_info *info, unsigned max_vtx_in)
{
   unsigned scratch_lds_base = 0;
   gl_shader_stage stage = nir->info.stage;

   if (stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_TESS_EVAL) {
      /* Get pervertex LDS usage. */
      bool uses_instanceid = BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_INSTANCE_ID);
      bool uses_primitive_id = BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_PRIMITIVE_ID);
      bool streamout_enabled = nir->xfb_info && device->physical_device->use_ngg_streamout;
      unsigned pervertex_lds_bytes = ac_ngg_nogs_get_pervertex_lds_size(
         stage, nir->num_outputs, streamout_enabled, info->outinfo.export_prim_id, false, /* user edge flag */
         info->has_ngg_culling, uses_instanceid, uses_primitive_id);

      unsigned total_es_lds_bytes = pervertex_lds_bytes * max_vtx_in;
      scratch_lds_base = ALIGN(total_es_lds_bytes, 8u);
   } else if (stage == MESA_SHADER_GEOMETRY) {
      unsigned esgs_ring_lds_bytes = info->ngg_info.esgs_ring_size;
      unsigned gs_total_out_vtx_bytes = info->ngg_info.ngg_emit_size * 4u;
      scratch_lds_base = ALIGN(esgs_ring_lds_bytes + gs_total_out_vtx_bytes, 8u /* for the repacking code */);
   } else {
      /* not handled here */
      return;
   }

   /* Get scratch LDS usage. */
   unsigned scratch_lds_size = ac_ngg_get_scratch_lds_size(
      stage, info->workgroup_size, info->wave_size, device->physical_device->use_ngg_streamout, info->has_ngg_culling);

   /* Get total LDS usage. */
   nir->info.shared_size = scratch_lds_base + scratch_lds_size;

   /* Record scratch base for abi lower of nir_load_lds_ngg_scratch_base_amd. */
   info->ngg_info.scratch_lds_base = scratch_lds_base;
}

void
radv_lower_ngg(struct radv_device *device, struct radv_shader_stage *ngg_stage, const struct radv_pipeline_key *pl_key)
{
   const struct radv_shader_info *info = &ngg_stage->info;
   nir_shader *nir = ngg_stage->nir;

   assert(nir->info.stage == MESA_SHADER_VERTEX || nir->info.stage == MESA_SHADER_TESS_EVAL ||
          nir->info.stage == MESA_SHADER_GEOMETRY || nir->info.stage == MESA_SHADER_MESH);

   const struct gfx10_ngg_info *ngg_info = &info->ngg_info;
   unsigned num_vertices_per_prim = 3;

   /* Get the number of vertices per input primitive */
   if (nir->info.stage == MESA_SHADER_TESS_EVAL) {
      if (nir->info.tess.point_mode)
         num_vertices_per_prim = 1;
      else if (nir->info.tess._primitive_mode == TESS_PRIMITIVE_ISOLINES)
         num_vertices_per_prim = 2;

      /* Manually mark the primitive ID used, so the shader can repack it. */
      if (info->outinfo.export_prim_id)
         BITSET_SET(nir->info.system_values_read, SYSTEM_VALUE_PRIMITIVE_ID);

   } else if (nir->info.stage == MESA_SHADER_VERTEX) {
      num_vertices_per_prim = radv_get_num_vertices_per_prim(pl_key);

      /* Manually mark the instance ID used, so the shader can repack it. */
      if (pl_key->vs.instance_rate_inputs)
         BITSET_SET(nir->info.system_values_read, SYSTEM_VALUE_INSTANCE_ID);

   } else if (nir->info.stage == MESA_SHADER_GEOMETRY) {
      num_vertices_per_prim = nir->info.gs.vertices_in;
   } else if (nir->info.stage == MESA_SHADER_MESH) {
      if (nir->info.mesh.primitive_type == MESA_PRIM_POINTS)
         num_vertices_per_prim = 1;
      else if (nir->info.mesh.primitive_type == MESA_PRIM_LINES)
         num_vertices_per_prim = 2;
      else
         assert(nir->info.mesh.primitive_type == MESA_PRIM_TRIANGLES);
   } else {
      unreachable("NGG needs to be VS, TES or GS.");
   }

   /* Invocations that process an input vertex */
   unsigned max_vtx_in = MIN2(256, ngg_info->hw_max_esverts);

   setup_ngg_lds_layout(device, nir, &ngg_stage->info, max_vtx_in);

   ac_nir_lower_ngg_options options = {0};
   options.family = device->physical_device->rad_info.family;
   options.gfx_level = device->physical_device->rad_info.gfx_level;
   options.max_workgroup_size = info->workgroup_size;
   options.wave_size = info->wave_size;
   options.clipdist_enable_mask = info->outinfo.clip_dist_mask | info->outinfo.cull_dist_mask;
   options.vs_output_param_offset = info->outinfo.vs_output_param_offset;
   options.has_param_exports = info->outinfo.param_exports || info->outinfo.prim_param_exports;
   options.can_cull = nir->info.stage != MESA_SHADER_GEOMETRY && info->has_ngg_culling;
   options.disable_streamout = !device->physical_device->use_ngg_streamout;
   options.has_gen_prim_query = info->has_prim_query;
   options.has_xfb_prim_query = info->has_xfb_query;
   options.force_vrs = info->force_vrs_per_vertex;

   if (nir->info.stage == MESA_SHADER_VERTEX || nir->info.stage == MESA_SHADER_TESS_EVAL) {
      assert(info->is_ngg);

      if (info->has_ngg_culling)
         radv_optimize_nir_algebraic(nir, false);

      options.num_vertices_per_primitive = num_vertices_per_prim;
      options.early_prim_export = info->has_ngg_early_prim_export;
      options.passthrough = info->is_ngg_passthrough;
      options.export_primitive_id = info->outinfo.export_prim_id;
      options.instance_rate_inputs = pl_key->vs.instance_rate_inputs << VERT_ATTRIB_GENERIC0;

      NIR_PASS_V(nir, ac_nir_lower_ngg_nogs, &options);

      /* Increase ESGS ring size so the LLVM binary contains the correct LDS size. */
      ngg_stage->info.ngg_info.esgs_ring_size = nir->info.shared_size;
   } else if (nir->info.stage == MESA_SHADER_GEOMETRY) {
      assert(info->is_ngg);

      options.gs_out_vtx_bytes = info->gs.gsvs_vertex_size;

      NIR_PASS_V(nir, ac_nir_lower_ngg_gs, &options);
   } else if (nir->info.stage == MESA_SHADER_MESH) {
      /* ACO aligns the workgroup size to the wave size. */
      unsigned hw_workgroup_size = ALIGN(info->workgroup_size, info->wave_size);

      bool scratch_ring = false;
      NIR_PASS_V(nir, ac_nir_lower_ngg_ms, options.gfx_level, options.clipdist_enable_mask,
                 options.vs_output_param_offset, options.has_param_exports, &scratch_ring, info->wave_size,
                 hw_workgroup_size, pl_key->has_multiview_view_index, info->ms.has_query, device->mesh_fast_launch_2);
      ngg_stage->info.ms.needs_ms_scratch_ring = scratch_ring;
   } else {
      unreachable("invalid SW stage passed to radv_lower_ngg");
   }
}

static unsigned
get_size_class(unsigned size, bool round_up)
{
   size = round_up ? util_logbase2_ceil(size) : util_logbase2(size);
   unsigned size_class = MAX2(size, RADV_SHADER_ALLOC_MIN_SIZE_CLASS) - RADV_SHADER_ALLOC_MIN_SIZE_CLASS;
   return MIN2(size_class, RADV_SHADER_ALLOC_NUM_FREE_LISTS - 1);
}

static void
remove_hole(struct radv_shader_free_list *free_list, union radv_shader_arena_block *hole)
{
   unsigned size_class = get_size_class(hole->size, false);
   list_del(&hole->freelist);
   if (list_is_empty(&free_list->free_lists[size_class]))
      free_list->size_mask &= ~(1u << size_class);
}

static void
add_hole(struct radv_shader_free_list *free_list, union radv_shader_arena_block *hole)
{
   unsigned size_class = get_size_class(hole->size, false);
   list_addtail(&hole->freelist, &free_list->free_lists[size_class]);
   free_list->size_mask |= 1u << size_class;
}

static union radv_shader_arena_block *
alloc_block_obj(struct radv_device *device)
{
   if (!list_is_empty(&device->shader_block_obj_pool)) {
      union radv_shader_arena_block *block =
         list_first_entry(&device->shader_block_obj_pool, union radv_shader_arena_block, pool);
      list_del(&block->pool);
      return block;
   }

   return malloc(sizeof(union radv_shader_arena_block));
}

static void
free_block_obj(struct radv_device *device, union radv_shader_arena_block *block)
{
   list_add(&block->pool, &device->shader_block_obj_pool);
}

VkResult
radv_shader_wait_for_upload(struct radv_device *device, uint64_t seq)
{
   if (!seq)
      return VK_SUCCESS;

   const VkSemaphoreWaitInfo wait_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
      .pSemaphores = &device->shader_upload_sem,
      .semaphoreCount = 1,
      .pValues = &seq,
   };
   return device->vk.dispatch_table.WaitSemaphores(radv_device_to_handle(device), &wait_info, UINT64_MAX);
}

static struct radv_shader_arena *
radv_create_shader_arena(struct radv_device *device, struct radv_shader_free_list *free_list, unsigned min_size,
                         unsigned arena_size, bool replayable, uint64_t replay_va)
{
   union radv_shader_arena_block *alloc = NULL;
   struct radv_shader_arena *arena = calloc(1, sizeof(struct radv_shader_arena));
   if (!arena)
      goto fail;

   if (!arena_size)
      arena_size = MAX2(
         RADV_SHADER_ALLOC_MIN_ARENA_SIZE << MIN2(RADV_SHADER_ALLOC_MAX_ARENA_SIZE_SHIFT, device->shader_arena_shift),
         min_size);
   arena->size = arena_size;

   enum radeon_bo_flag flags = RADEON_FLAG_NO_INTERPROCESS_SHARING | RADEON_FLAG_32BIT;
   if (device->shader_use_invisible_vram)
      flags |= RADEON_FLAG_NO_CPU_ACCESS;
   else
      flags |= (device->physical_device->rad_info.cpdma_prefetch_writes_memory ? 0 : RADEON_FLAG_READ_ONLY);

   if (replayable)
      flags |= RADEON_FLAG_REPLAYABLE;

   VkResult result;
   result = device->ws->buffer_create(device->ws, arena_size, RADV_SHADER_ALLOC_ALIGNMENT, RADEON_DOMAIN_VRAM, flags,
                                      RADV_BO_PRIORITY_SHADER, replay_va, &arena->bo);
   if (result != VK_SUCCESS)
      goto fail;

   radv_rmv_log_bo_allocate(device, arena->bo, arena_size, true);

   list_inithead(&arena->entries);
   alloc = alloc_block_obj(device);
   if (!alloc)
      goto fail;

   list_inithead(&alloc->freelist);
   alloc->arena = arena;
   alloc->offset = 0;
   alloc->size = arena_size;
   list_addtail(&alloc->list, &arena->entries);
   if (free_list)
      add_hole(free_list, alloc);

   if (!(flags & RADEON_FLAG_NO_CPU_ACCESS)) {
      arena->ptr = (char *)device->ws->buffer_map(arena->bo);
      if (!arena->ptr)
         goto fail;
   }

   if (replay_va)
      arena->type = RADV_SHADER_ARENA_REPLAYED;
   else if (replayable)
      arena->type = RADV_SHADER_ARENA_REPLAYABLE;
   else
      arena->type = RADV_SHADER_ARENA_DEFAULT;

   return arena;

fail:
   if (alloc)
      free_block_obj(device, alloc);
   if (arena && arena->bo) {
      radv_rmv_log_bo_destroy(device, arena->bo);
      device->ws->buffer_destroy(device->ws, arena->bo);
   }
   free(arena);
   return NULL;
}

/* Inserts a block at an arbitrary place into a hole, splitting the hole as needed */
static union radv_shader_arena_block *
insert_block(struct radv_device *device, union radv_shader_arena_block *hole, uint32_t offset_in_hole, uint32_t size,
             struct radv_shader_free_list *free_list)
{
   uint32_t hole_begin = hole->offset;
   uint32_t hole_end = hole->offset + hole->size;

   /* The block might not lie exactly at the beginning or end
    * of the hole. Resize the hole to fit the block exactly,
    * and insert new holes before (left_hole) or after (right_hole) as needed.
    * left_hole or right_hole are skipped if the allocation lies exactly at the
    * beginning or end of the hole to avoid 0-sized holes. */
   union radv_shader_arena_block *left_hole = NULL;
   union radv_shader_arena_block *right_hole = NULL;

   if (offset_in_hole) {
      left_hole = alloc_block_obj(device);
      if (!left_hole)
         return NULL;
      list_inithead(&left_hole->freelist);
      left_hole->arena = hole->arena;
      left_hole->offset = hole->offset;
      left_hole->size = offset_in_hole;

      if (free_list)
         add_hole(free_list, left_hole);
   }

   if (hole->size > offset_in_hole + size) {
      right_hole = alloc_block_obj(device);
      if (!right_hole) {
         free(left_hole);
         return NULL;
      }
      list_inithead(&right_hole->freelist);
      right_hole->arena = hole->arena;
      right_hole->offset = hole_begin + offset_in_hole + size;
      right_hole->size = hole_end - right_hole->offset;

      if (free_list)
         add_hole(free_list, right_hole);
   }

   if (left_hole) {
      hole->offset += left_hole->size;
      hole->size -= left_hole->size;

      list_addtail(&left_hole->list, &hole->list);
   }
   if (right_hole) {
      hole->size -= right_hole->size;

      list_add(&right_hole->list, &hole->list);
   }

   if (free_list)
      remove_hole(free_list, hole);
   return hole;
}

/* Segregated fit allocator, implementing a good-fit allocation policy.
 *
 * This is an variation of sequential fit allocation with several lists of free blocks ("holes")
 * instead of one. Each list of holes only contains holes of a certain range of sizes, so holes that
 * are too small can easily be ignored while allocating. Because this also ignores holes that are
 * larger than necessary (approximating best-fit allocation), this could be described as a
 * "good-fit" allocator.
 *
 * Typically, shaders are allocated and only free'd when the device is destroyed. For this pattern,
 * this should allocate blocks for shaders fast and with no fragmentation, while still allowing
 * free'd memory to be re-used.
 */
union radv_shader_arena_block *
radv_alloc_shader_memory(struct radv_device *device, uint32_t size, bool replayable, void *ptr)
{
   size = ac_align_shader_binary_for_prefetch(&device->physical_device->rad_info, size);
   size = align(size, RADV_SHADER_ALLOC_ALIGNMENT);

   mtx_lock(&device->shader_arena_mutex);

   struct radv_shader_free_list *free_list = replayable ? &device->capture_replay_free_list : &device->shader_free_list;

   /* Try to use an existing hole. Unless the shader is very large, this should only have to look
    * at the first one available.
    */
   unsigned free_list_mask = BITFIELD_MASK(RADV_SHADER_ALLOC_NUM_FREE_LISTS);
   unsigned size_class = ffs(free_list->size_mask & (free_list_mask << get_size_class(size, true)));
   if (size_class) {
      size_class--;

      list_for_each_entry (union radv_shader_arena_block, hole, &free_list->free_lists[size_class], freelist) {
         if (hole->size < size)
            continue;

         assert(hole->offset % RADV_SHADER_ALLOC_ALIGNMENT == 0);

         if (size == hole->size) {
            remove_hole(free_list, hole);
            hole->freelist.next = ptr;
            mtx_unlock(&device->shader_arena_mutex);
            return hole;
         } else {
            union radv_shader_arena_block *alloc = alloc_block_obj(device);
            if (!alloc) {
               mtx_unlock(&device->shader_arena_mutex);
               return NULL;
            }
            list_addtail(&alloc->list, &hole->list);
            alloc->freelist.prev = NULL;
            alloc->freelist.next = ptr;
            alloc->arena = hole->arena;
            alloc->offset = hole->offset;
            alloc->size = size;

            remove_hole(free_list, hole);
            hole->offset += size;
            hole->size -= size;
            add_hole(free_list, hole);

            mtx_unlock(&device->shader_arena_mutex);
            return alloc;
         }
      }
   }

   struct radv_shader_arena *arena = radv_create_shader_arena(device, free_list, size, 0, replayable, 0);
   union radv_shader_arena_block *alloc = NULL;
   if (!arena)
      goto fail;

   alloc =
      insert_block(device, list_entry(arena->entries.next, union radv_shader_arena_block, list), 0, size, free_list);
   alloc->freelist.prev = NULL;
   alloc->freelist.next = ptr;

   ++device->shader_arena_shift;
   list_addtail(&arena->list, &device->shader_arenas);

   mtx_unlock(&device->shader_arena_mutex);
   return alloc;

fail:
   mtx_unlock(&device->shader_arena_mutex);
   free(alloc);
   if (arena) {
      free(arena->list.next);
      radv_rmv_log_bo_destroy(device, arena->bo);
      device->ws->buffer_destroy(device->ws, arena->bo);
   }
   free(arena);
   return NULL;
}

static union radv_shader_arena_block *
get_hole(struct radv_shader_arena *arena, struct list_head *head)
{
   if (head == &arena->entries)
      return NULL;

   union radv_shader_arena_block *hole = list_entry(head, union radv_shader_arena_block, list);
   return hole->freelist.prev ? hole : NULL;
}

void
radv_free_shader_memory(struct radv_device *device, union radv_shader_arena_block *alloc)
{
   mtx_lock(&device->shader_arena_mutex);

   union radv_shader_arena_block *hole_prev = get_hole(alloc->arena, alloc->list.prev);
   union radv_shader_arena_block *hole_next = get_hole(alloc->arena, alloc->list.next);

   union radv_shader_arena_block *hole = alloc;

   struct radv_shader_free_list *free_list;

   switch (alloc->arena->type) {
   case RADV_SHADER_ARENA_DEFAULT:
      free_list = &device->shader_free_list;
      break;
   case RADV_SHADER_ARENA_REPLAYABLE:
      free_list = &device->capture_replay_free_list;
      break;
   case RADV_SHADER_ARENA_REPLAYED:
      free_list = NULL;
      break;
   default:
      unreachable("invalid shader arena type");
   }

   /* merge with previous hole */
   if (hole_prev) {
      if (free_list)
         remove_hole(free_list, hole_prev);

      hole_prev->size += hole->size;
      list_del(&hole->list);
      free_block_obj(device, hole);

      hole = hole_prev;
   }

   /* merge with next hole */
   if (hole_next) {
      if (free_list)
         remove_hole(free_list, hole_next);

      hole_next->offset -= hole->size;
      hole_next->size += hole->size;
      list_del(&hole->list);
      free_block_obj(device, hole);

      hole = hole_next;
   }

   if (list_is_singular(&hole->list)) {
      struct radv_shader_arena *arena = hole->arena;
      free_block_obj(device, hole);

      radv_rmv_log_bo_destroy(device, arena->bo);
      device->ws->buffer_destroy(device->ws, arena->bo);
      list_del(&arena->list);
      free(arena);
   } else if (free_list) {
      add_hole(free_list, hole);
   }

   mtx_unlock(&device->shader_arena_mutex);
}

struct radv_serialized_shader_arena_block
radv_serialize_shader_arena_block(union radv_shader_arena_block *block)
{
   struct radv_serialized_shader_arena_block serialized_block = {
      .offset = block->offset,
      .size = block->size,
      .arena_va = block->arena->bo->va,
      .arena_size = block->arena->size,
   };
   return serialized_block;
}

union radv_shader_arena_block *
radv_replay_shader_arena_block(struct radv_device *device, const struct radv_serialized_shader_arena_block *src,
                               void *ptr)
{
   mtx_lock(&device->shader_arena_mutex);
   uint64_t va = src->arena_va;
   void *data = _mesa_hash_table_u64_search(device->capture_replay_arena_vas, va);

   if (!data) {
      struct radv_shader_arena *arena = radv_create_shader_arena(device, NULL, 0, src->arena_size, true, src->arena_va);
      if (!arena) {
         mtx_unlock(&device->shader_arena_mutex);
         return NULL;
      }

      _mesa_hash_table_u64_insert(device->capture_replay_arena_vas, src->arena_va, arena);
      list_addtail(&arena->list, &device->shader_arenas);
      data = arena;
   }
   mtx_unlock(&device->shader_arena_mutex);

   uint32_t block_begin = src->offset;
   uint32_t block_end = src->offset + src->size;

   struct radv_shader_arena *arena = data;
   list_for_each_entry (union radv_shader_arena_block, hole, &arena->entries, list) {
      /* Only consider holes, not allocated shaders */
      if (!hole->freelist.prev)
         continue;

      uint32_t hole_begin = hole->offset;
      uint32_t hole_end = hole->offset + hole->size;

      if (hole_end < block_end)
         continue;

      /* If another allocated block overlaps the current replay block, allocation is impossible */
      if (hole_begin > block_begin)
         return NULL;

      union radv_shader_arena_block *block = insert_block(device, hole, block_begin - hole_begin, src->size, NULL);
      if (!block)
         return NULL;

      block->freelist.prev = NULL;
      block->freelist.next = ptr;
      return hole;
   }
   return NULL;
}

void
radv_init_shader_arenas(struct radv_device *device)
{
   mtx_init(&device->shader_arena_mutex, mtx_plain);

   device->shader_free_list.size_mask = 0;
   device->capture_replay_free_list.size_mask = 0;

   list_inithead(&device->shader_arenas);
   list_inithead(&device->shader_block_obj_pool);
   for (unsigned i = 0; i < RADV_SHADER_ALLOC_NUM_FREE_LISTS; i++) {
      list_inithead(&device->shader_free_list.free_lists[i]);
      list_inithead(&device->capture_replay_free_list.free_lists[i]);
   }
}

void
radv_destroy_shader_arenas(struct radv_device *device)
{
   list_for_each_entry_safe (union radv_shader_arena_block, block, &device->shader_block_obj_pool, pool)
      free(block);

   list_for_each_entry_safe (struct radv_shader_arena, arena, &device->shader_arenas, list) {
      radv_rmv_log_bo_destroy(device, arena->bo);
      device->ws->buffer_destroy(device->ws, arena->bo);
      free(arena);
   }
   mtx_destroy(&device->shader_arena_mutex);
}

VkResult
radv_init_shader_upload_queue(struct radv_device *device)
{
   if (!device->shader_use_invisible_vram)
      return VK_SUCCESS;

   VkDevice vk_device = radv_device_to_handle(device);
   struct radeon_winsys *ws = device->ws;

   const struct vk_device_dispatch_table *disp = &device->vk.dispatch_table;
   VkResult result = VK_SUCCESS;

   result = ws->ctx_create(ws, RADEON_CTX_PRIORITY_MEDIUM, &device->shader_upload_hw_ctx);
   if (result != VK_SUCCESS)
      return result;
   mtx_init(&device->shader_upload_hw_ctx_mutex, mtx_plain);

   mtx_init(&device->shader_dma_submission_list_mutex, mtx_plain);
   cnd_init(&device->shader_dma_submission_list_cond);
   list_inithead(&device->shader_dma_submissions);

   for (unsigned i = 0; i < RADV_SHADER_UPLOAD_CS_COUNT; i++) {
      struct radv_shader_dma_submission *submission = calloc(1, sizeof(struct radv_shader_dma_submission));
      submission->cs = ws->cs_create(ws, AMD_IP_SDMA, false);
      if (!submission->cs)
         return VK_ERROR_OUT_OF_DEVICE_MEMORY;
      list_addtail(&submission->list, &device->shader_dma_submissions);
   }

   const VkSemaphoreTypeCreateInfo sem_type = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
      .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
      .initialValue = 0,
   };
   const VkSemaphoreCreateInfo sem_create = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = &sem_type,
   };
   result = disp->CreateSemaphore(vk_device, &sem_create, NULL, &device->shader_upload_sem);
   if (result != VK_SUCCESS)
      return result;

   return VK_SUCCESS;
}

void
radv_destroy_shader_upload_queue(struct radv_device *device)
{
   if (!device->shader_use_invisible_vram)
      return;

   struct vk_device_dispatch_table *disp = &device->vk.dispatch_table;
   struct radeon_winsys *ws = device->ws;

   /* Upload queue should be idle assuming that pipelines are not leaked */
   if (device->shader_upload_sem)
      disp->DestroySemaphore(radv_device_to_handle(device), device->shader_upload_sem, NULL);

   list_for_each_entry_safe (struct radv_shader_dma_submission, submission, &device->shader_dma_submissions, list) {
      if (submission->cs)
         ws->cs_destroy(submission->cs);
      if (submission->bo)
         ws->buffer_destroy(ws, submission->bo);
      list_del(&submission->list);
      free(submission);
   }

   cnd_destroy(&device->shader_dma_submission_list_cond);
   mtx_destroy(&device->shader_dma_submission_list_mutex);

   if (device->shader_upload_hw_ctx) {
      mtx_destroy(&device->shader_upload_hw_ctx_mutex);
      ws->ctx_destroy(device->shader_upload_hw_ctx);
   }
}

static bool
radv_should_use_wgp_mode(const struct radv_device *device, gl_shader_stage stage, const struct radv_shader_info *info)
{
   enum amd_gfx_level chip = device->physical_device->rad_info.gfx_level;
   switch (stage) {
   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_TESS_CTRL:
      return chip >= GFX10;
   case MESA_SHADER_GEOMETRY:
      return chip == GFX10 || (chip >= GFX10_3 && !info->is_ngg);
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_TESS_EVAL:
      return chip == GFX10 && info->is_ngg;
   default:
      return false;
   }
}

#if defined(USE_LIBELF)
static bool
radv_open_rtld_binary(struct radv_device *device, const struct radv_shader_binary *binary,
                      struct ac_rtld_binary *rtld_binary)
{
   const char *elf_data = (const char *)((struct radv_shader_binary_rtld *)binary)->data;
   size_t elf_size = ((struct radv_shader_binary_rtld *)binary)->elf_size;
   struct ac_rtld_symbol lds_symbols[3];
   unsigned num_lds_symbols = 0;

   if (device->physical_device->rad_info.gfx_level >= GFX9 &&
       (binary->info.stage == MESA_SHADER_GEOMETRY || binary->info.is_ngg)) {
      struct ac_rtld_symbol *sym = &lds_symbols[num_lds_symbols++];
      sym->name = "esgs_ring";
      sym->size = binary->info.ngg_info.esgs_ring_size;
      sym->align = 64 * 1024;
   }

   if (binary->info.is_ngg && binary->info.stage == MESA_SHADER_GEOMETRY) {
      struct ac_rtld_symbol *sym = &lds_symbols[num_lds_symbols++];
      sym->name = "ngg_emit";
      sym->size = binary->info.ngg_info.ngg_emit_size * 4;
      sym->align = 4;

      sym = &lds_symbols[num_lds_symbols++];
      sym->name = "ngg_scratch";
      sym->size = 8;
      sym->align = 4;
   }

   struct ac_rtld_open_info open_info = {
      .info = &device->physical_device->rad_info,
      .shader_type = binary->info.stage,
      .wave_size = binary->info.wave_size,
      .num_parts = 1,
      .elf_ptrs = &elf_data,
      .elf_sizes = &elf_size,
      .num_shared_lds_symbols = num_lds_symbols,
      .shared_lds_symbols = lds_symbols,
   };

   return ac_rtld_open(rtld_binary, open_info);
}
#endif

static bool
radv_postprocess_binary_config(struct radv_device *device, struct radv_shader_binary *binary,
                               const struct radv_shader_args *args)
{
   struct ac_shader_config *config = &binary->config;

   if (binary->type == RADV_BINARY_TYPE_RTLD) {
#if !defined(USE_LIBELF)
      return false;
#else
      struct ac_rtld_binary rtld_binary = {0};

      if (!radv_open_rtld_binary(device, binary, &rtld_binary)) {
         return false;
      }

      if (!ac_rtld_read_config(&device->physical_device->rad_info, &rtld_binary, config)) {
         ac_rtld_close(&rtld_binary);
         return false;
      }

      if (rtld_binary.lds_size > 0) {
         unsigned encode_granularity = device->physical_device->rad_info.lds_encode_granularity;
         config->lds_size = DIV_ROUND_UP(rtld_binary.lds_size, encode_granularity);
      }
      if (!config->lds_size && binary->info.stage == MESA_SHADER_TESS_CTRL) {
         /* This is used for reporting LDS statistics */
         config->lds_size = binary->info.tcs.num_lds_blocks;
      }

      assert(!binary->info.has_ngg_culling || config->lds_size);
      ac_rtld_close(&rtld_binary);
#endif
   }

   const struct radv_shader_info *info = &binary->info;
   gl_shader_stage stage = binary->info.stage;
   const struct radv_physical_device *pdevice = device->physical_device;
   bool scratch_enabled = config->scratch_bytes_per_wave > 0;
   bool trap_enabled = !!device->trap_handler_shader;
   unsigned vgpr_comp_cnt = 0;
   unsigned num_input_vgprs = args->ac.num_vgprs_used;

   if (stage == MESA_SHADER_FRAGMENT) {
      num_input_vgprs = ac_get_fs_input_vgpr_cnt(config, NULL);
   }

   unsigned num_vgprs = MAX2(config->num_vgprs, num_input_vgprs);
   /* +2 for the ring offsets, +3 for scratch wave offset and VCC */
   unsigned num_sgprs = MAX2(config->num_sgprs, args->ac.num_sgprs_used + 2 + 3);
   unsigned num_shared_vgprs = config->num_shared_vgprs;
   /* shared VGPRs are introduced in Navi and are allocated in blocks of 8 (RDNA ref 3.6.5) */
   assert((pdevice->rad_info.gfx_level >= GFX10 && num_shared_vgprs % 8 == 0) ||
          (pdevice->rad_info.gfx_level < GFX10 && num_shared_vgprs == 0));
   unsigned num_shared_vgpr_blocks = num_shared_vgprs / 8;
   unsigned excp_en = 0;

   config->num_vgprs = num_vgprs;
   config->num_sgprs = num_sgprs;
   config->num_shared_vgprs = num_shared_vgprs;

   config->rsrc2 = S_00B12C_USER_SGPR(args->num_user_sgprs) | S_00B12C_SCRATCH_EN(scratch_enabled) |
                   S_00B12C_TRAP_PRESENT(trap_enabled);

   if (trap_enabled) {
      /* Configure the shader exceptions like memory violation, etc.
       * TODO: Enable (and validate) more exceptions.
       */
      excp_en = 1 << 8; /* mem_viol */
   }

   if (!pdevice->use_ngg_streamout) {
      config->rsrc2 |= S_00B12C_SO_BASE0_EN(!!info->so.strides[0]) | S_00B12C_SO_BASE1_EN(!!info->so.strides[1]) |
                       S_00B12C_SO_BASE2_EN(!!info->so.strides[2]) | S_00B12C_SO_BASE3_EN(!!info->so.strides[3]) |
                       S_00B12C_SO_EN(!!info->so.num_outputs);
   }

   config->rsrc1 = S_00B848_VGPRS((num_vgprs - 1) / (info->wave_size == 32 ? 8 : 4)) | S_00B848_DX10_CLAMP(1) |
                   S_00B848_FLOAT_MODE(config->float_mode);

   if (pdevice->rad_info.gfx_level >= GFX10) {
      config->rsrc2 |= S_00B22C_USER_SGPR_MSB_GFX10(args->num_user_sgprs >> 5);
   } else {
      config->rsrc1 |= S_00B228_SGPRS((num_sgprs - 1) / 8);
      config->rsrc2 |= S_00B22C_USER_SGPR_MSB_GFX9(args->num_user_sgprs >> 5);
   }

   bool wgp_mode = radv_should_use_wgp_mode(device, stage, info);

   switch (stage) {
   case MESA_SHADER_TESS_EVAL:
      if (info->is_ngg) {
         config->rsrc1 |= S_00B228_MEM_ORDERED(pdevice->rad_info.gfx_level >= GFX10);
         config->rsrc2 |= S_00B22C_OC_LDS_EN(1) | S_00B22C_EXCP_EN(excp_en);
      } else if (info->tes.as_es) {
         assert(pdevice->rad_info.gfx_level <= GFX8);
         vgpr_comp_cnt = info->uses_prim_id ? 3 : 2;

         config->rsrc2 |= S_00B12C_OC_LDS_EN(1) | S_00B12C_EXCP_EN(excp_en);
      } else {
         bool enable_prim_id = info->outinfo.export_prim_id || info->uses_prim_id;
         vgpr_comp_cnt = enable_prim_id ? 3 : 2;

         config->rsrc1 |= S_00B128_MEM_ORDERED(pdevice->rad_info.gfx_level >= GFX10);
         config->rsrc2 |= S_00B12C_OC_LDS_EN(1) | S_00B12C_EXCP_EN(excp_en);
      }
      config->rsrc2 |= S_00B22C_SHARED_VGPR_CNT(num_shared_vgpr_blocks);
      break;
   case MESA_SHADER_TESS_CTRL:
      if (pdevice->rad_info.gfx_level >= GFX9) {
         /* We need at least 2 components for LS.
          * VGPR0-3: (VertexID, RelAutoindex, InstanceID / StepRate0, InstanceID).
          * StepRate0 is set to 1. so that VGPR3 doesn't have to be loaded.
          */
         if (pdevice->rad_info.gfx_level >= GFX10) {
            if (info->vs.needs_instance_id) {
               vgpr_comp_cnt = 3;
            } else if (pdevice->rad_info.gfx_level <= GFX10_3) {
               vgpr_comp_cnt = 1;
            }
            config->rsrc2 |= S_00B42C_EXCP_EN_GFX6(excp_en);
         } else {
            vgpr_comp_cnt = info->vs.needs_instance_id ? 2 : 1;
            config->rsrc2 |= S_00B42C_EXCP_EN_GFX9(excp_en);
         }
      } else {
         config->rsrc2 |= S_00B12C_OC_LDS_EN(1) | S_00B12C_EXCP_EN(excp_en);
      }
      config->rsrc1 |= S_00B428_MEM_ORDERED(pdevice->rad_info.gfx_level >= GFX10) | S_00B428_WGP_MODE(wgp_mode);
      config->rsrc2 |= S_00B42C_SHARED_VGPR_CNT(num_shared_vgpr_blocks);
      break;
   case MESA_SHADER_VERTEX:
      if (info->is_ngg) {
         config->rsrc1 |= S_00B228_MEM_ORDERED(pdevice->rad_info.gfx_level >= GFX10);
      } else if (info->vs.as_ls) {
         assert(pdevice->rad_info.gfx_level <= GFX8);
         /* We need at least 2 components for LS.
          * VGPR0-3: (VertexID, RelAutoindex, InstanceID / StepRate0, InstanceID).
          * StepRate0 is set to 1. so that VGPR3 doesn't have to be loaded.
          */
         vgpr_comp_cnt = info->vs.needs_instance_id ? 2 : 1;
      } else if (info->vs.as_es) {
         assert(pdevice->rad_info.gfx_level <= GFX8);
         /* VGPR0-3: (VertexID, InstanceID / StepRate0, ...) */
         vgpr_comp_cnt = info->vs.needs_instance_id ? 1 : 0;
      } else {
         /* VGPR0-3: (VertexID, InstanceID / StepRate0, PrimID, InstanceID)
          * If PrimID is disabled. InstanceID / StepRate1 is loaded instead.
          * StepRate0 is set to 1. so that VGPR3 doesn't have to be loaded.
          */
         if (info->vs.needs_instance_id && pdevice->rad_info.gfx_level >= GFX10) {
            vgpr_comp_cnt = 3;
         } else if (info->outinfo.export_prim_id) {
            vgpr_comp_cnt = 2;
         } else if (info->vs.needs_instance_id) {
            vgpr_comp_cnt = 1;
         } else {
            vgpr_comp_cnt = 0;
         }

         config->rsrc1 |= S_00B128_MEM_ORDERED(pdevice->rad_info.gfx_level >= GFX10);
      }
      config->rsrc2 |= S_00B12C_SHARED_VGPR_CNT(num_shared_vgpr_blocks) | S_00B12C_EXCP_EN(excp_en);
      break;
   case MESA_SHADER_MESH:
      config->rsrc1 |= S_00B228_MEM_ORDERED(1);
      config->rsrc2 |= S_00B12C_SHARED_VGPR_CNT(num_shared_vgpr_blocks) | S_00B12C_EXCP_EN(excp_en);
      break;
   case MESA_SHADER_FRAGMENT:
      config->rsrc1 |= S_00B028_MEM_ORDERED(pdevice->rad_info.gfx_level >= GFX10) |
                       S_00B028_LOAD_PROVOKING_VTX(info->ps.load_provoking_vtx);
      config->rsrc2 |= S_00B02C_SHARED_VGPR_CNT(num_shared_vgpr_blocks) | S_00B02C_EXCP_EN(excp_en) |
                       S_00B02C_LOAD_COLLISION_WAVEID(info->ps.pops && pdevice->rad_info.gfx_level < GFX11);
      break;
   case MESA_SHADER_GEOMETRY:
      config->rsrc1 |= S_00B228_MEM_ORDERED(pdevice->rad_info.gfx_level >= GFX10);
      config->rsrc2 |= S_00B22C_SHARED_VGPR_CNT(num_shared_vgpr_blocks) | S_00B22C_EXCP_EN(excp_en);
      break;
   case MESA_SHADER_RAYGEN:
   case MESA_SHADER_CLOSEST_HIT:
   case MESA_SHADER_MISS:
   case MESA_SHADER_CALLABLE:
   case MESA_SHADER_INTERSECTION:
   case MESA_SHADER_ANY_HIT:
   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_TASK:
      config->rsrc1 |= S_00B848_MEM_ORDERED(pdevice->rad_info.gfx_level >= GFX10) | S_00B848_WGP_MODE(wgp_mode);
      config->rsrc2 |= S_00B84C_TGID_X_EN(info->cs.uses_block_id[0]) | S_00B84C_TGID_Y_EN(info->cs.uses_block_id[1]) |
                       S_00B84C_TGID_Z_EN(info->cs.uses_block_id[2]) |
                       S_00B84C_TIDIG_COMP_CNT(info->cs.uses_thread_id[2]   ? 2
                                               : info->cs.uses_thread_id[1] ? 1
                                                                            : 0) |
                       S_00B84C_TG_SIZE_EN(info->cs.uses_local_invocation_idx) | S_00B84C_LDS_SIZE(config->lds_size) |
                       S_00B84C_EXCP_EN(excp_en);
      config->rsrc3 |= S_00B8A0_SHARED_VGPR_CNT(num_shared_vgpr_blocks);

      break;
   default:
      unreachable("unsupported shader type");
      break;
   }

   if (pdevice->rad_info.gfx_level >= GFX10 && info->is_ngg &&
       (stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_TESS_EVAL || stage == MESA_SHADER_GEOMETRY ||
        stage == MESA_SHADER_MESH)) {
      unsigned gs_vgpr_comp_cnt, es_vgpr_comp_cnt;
      gl_shader_stage es_stage = stage;
      if (stage == MESA_SHADER_GEOMETRY)
         es_stage = info->gs.es_type;

      /* VGPR5-8: (VertexID, UserVGPR0, UserVGPR1, UserVGPR2 / InstanceID) */
      if (es_stage == MESA_SHADER_VERTEX) {
         es_vgpr_comp_cnt = info->vs.needs_instance_id ? 3 : 0;
      } else if (es_stage == MESA_SHADER_TESS_EVAL) {
         bool enable_prim_id = info->outinfo.export_prim_id || info->uses_prim_id;
         es_vgpr_comp_cnt = enable_prim_id ? 3 : 2;
      } else if (es_stage == MESA_SHADER_MESH) {
         es_vgpr_comp_cnt = 0;
      } else {
         unreachable("Unexpected ES shader stage");
      }

      /* GS vertex offsets in NGG:
       * - in passthrough mode, they are all packed into VGPR0
       * - in the default mode: VGPR0: offsets 0, 1; VGPR1: offsets 2, 3
       *
       * The vertex offset 2 is always needed when NGG isn't in passthrough mode
       * and uses triangle input primitives, including with NGG culling.
       */
      bool need_gs_vtx_offset2 = !info->is_ngg_passthrough || info->gs.vertices_in >= 3;

      /* TES only needs vertex offset 2 for triangles or quads. */
      if (stage == MESA_SHADER_TESS_EVAL)
         need_gs_vtx_offset2 &=
            info->tes._primitive_mode == TESS_PRIMITIVE_TRIANGLES || info->tes._primitive_mode == TESS_PRIMITIVE_QUADS;

      if (info->uses_invocation_id) {
         gs_vgpr_comp_cnt = 3; /* VGPR3 contains InvocationID. */
      } else if (info->uses_prim_id || (es_stage == MESA_SHADER_VERTEX && info->outinfo.export_prim_id)) {
         gs_vgpr_comp_cnt = 2; /* VGPR2 contains PrimitiveID. */
      } else if (need_gs_vtx_offset2) {
         gs_vgpr_comp_cnt = 1; /* VGPR1 contains offsets 2, 3 */
      } else {
         gs_vgpr_comp_cnt = 0; /* VGPR0 contains offsets 0, 1 (or passthrough prim) */
      }

      /* Disable the WGP mode on gfx10.3 because it can hang. (it
       * happened on VanGogh) Let's disable it on all chips that
       * disable exactly 1 CU per SA for GS.
       */
      config->rsrc1 |= S_00B228_GS_VGPR_COMP_CNT(gs_vgpr_comp_cnt) | S_00B228_WGP_MODE(wgp_mode);
      config->rsrc2 |= S_00B22C_ES_VGPR_COMP_CNT(es_vgpr_comp_cnt) | S_00B22C_LDS_SIZE(config->lds_size) |
                       S_00B22C_OC_LDS_EN(es_stage == MESA_SHADER_TESS_EVAL);
   } else if (pdevice->rad_info.gfx_level >= GFX9 && stage == MESA_SHADER_GEOMETRY) {
      unsigned es_type = info->gs.es_type;
      unsigned gs_vgpr_comp_cnt, es_vgpr_comp_cnt;

      if (es_type == MESA_SHADER_VERTEX) {
         /* VGPR0-3: (VertexID, InstanceID / StepRate0, ...) */
         if (info->vs.needs_instance_id) {
            es_vgpr_comp_cnt = pdevice->rad_info.gfx_level >= GFX10 ? 3 : 1;
         } else {
            es_vgpr_comp_cnt = 0;
         }
      } else if (es_type == MESA_SHADER_TESS_EVAL) {
         es_vgpr_comp_cnt = info->uses_prim_id ? 3 : 2;
      } else {
         unreachable("invalid shader ES type");
      }

      /* If offsets 4, 5 are used, GS_VGPR_COMP_CNT is ignored and
       * VGPR[0:4] are always loaded.
       */
      if (info->uses_invocation_id) {
         gs_vgpr_comp_cnt = 3; /* VGPR3 contains InvocationID. */
      } else if (info->uses_prim_id) {
         gs_vgpr_comp_cnt = 2; /* VGPR2 contains PrimitiveID. */
      } else if (info->gs.vertices_in >= 3) {
         gs_vgpr_comp_cnt = 1; /* VGPR1 contains offsets 2, 3 */
      } else {
         gs_vgpr_comp_cnt = 0; /* VGPR0 contains offsets 0, 1 */
      }

      config->rsrc1 |= S_00B228_GS_VGPR_COMP_CNT(gs_vgpr_comp_cnt) | S_00B228_WGP_MODE(wgp_mode);
      config->rsrc2 |=
         S_00B22C_ES_VGPR_COMP_CNT(es_vgpr_comp_cnt) | S_00B22C_OC_LDS_EN(es_type == MESA_SHADER_TESS_EVAL);
   } else if (pdevice->rad_info.gfx_level >= GFX9 && stage == MESA_SHADER_TESS_CTRL) {
      config->rsrc1 |= S_00B428_LS_VGPR_COMP_CNT(vgpr_comp_cnt);
   } else {
      config->rsrc1 |= S_00B128_VGPR_COMP_CNT(vgpr_comp_cnt);
   }

   return true;
}

static bool
radv_shader_binary_upload(struct radv_device *device, const struct radv_shader_binary *binary,
                          struct radv_shader *shader, void *dest_ptr)
{
   shader->code = calloc(shader->code_size, 1);
   if (!shader->code) {
      radv_shader_unref(device, shader);
      return false;
   }

   if (binary->type == RADV_BINARY_TYPE_RTLD) {
#if !defined(USE_LIBELF)
      return false;
#else
      struct ac_rtld_binary rtld_binary = {0};

      if (!radv_open_rtld_binary(device, binary, &rtld_binary)) {
         free(shader);
         return false;
      }

      struct ac_rtld_upload_info info = {
         .binary = &rtld_binary,
         .rx_va = radv_shader_get_va(shader),
         .rx_ptr = dest_ptr,
      };

      if (!ac_rtld_upload(&info)) {
         radv_shader_unref(device, shader);
         ac_rtld_close(&rtld_binary);
         return false;
      }

      ac_rtld_close(&rtld_binary);

      if (shader->code) {
         /* Instead of running RTLD twice, just copy the relocated binary back from VRAM.
          * Use streaming memcpy to reduce penalty of copying from uncachable memory.
          */
         util_streaming_load_memcpy(shader->code, dest_ptr, shader->code_size);
      }
#endif
   } else {
      struct radv_shader_binary_legacy *bin = (struct radv_shader_binary_legacy *)binary;
      memcpy(dest_ptr, bin->data + bin->stats_size, bin->code_size);

      if (shader->code) {
         memcpy(shader->code, bin->data + bin->stats_size, bin->code_size);
      }
   }

   return true;
}

static VkResult
radv_shader_dma_resize_upload_buf(struct radv_shader_dma_submission *submission, struct radeon_winsys *ws,
                                  uint64_t size)
{
   if (submission->bo)
      ws->buffer_destroy(ws, submission->bo);

   VkResult result = ws->buffer_create(
      ws, size, RADV_SHADER_ALLOC_ALIGNMENT, RADEON_DOMAIN_GTT,
      RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING | RADEON_FLAG_32BIT | RADEON_FLAG_GTT_WC,
      RADV_BO_PRIORITY_UPLOAD_BUFFER, 0, &submission->bo);
   if (result != VK_SUCCESS)
      return result;

   submission->ptr = ws->buffer_map(submission->bo);
   submission->bo_size = size;

   return VK_SUCCESS;
}

struct radv_shader_dma_submission *
radv_shader_dma_pop_submission(struct radv_device *device)
{
   struct radv_shader_dma_submission *submission;

   mtx_lock(&device->shader_dma_submission_list_mutex);

   while (list_is_empty(&device->shader_dma_submissions))
      cnd_wait(&device->shader_dma_submission_list_cond, &device->shader_dma_submission_list_mutex);

   submission = list_first_entry(&device->shader_dma_submissions, struct radv_shader_dma_submission, list);
   list_del(&submission->list);

   mtx_unlock(&device->shader_dma_submission_list_mutex);

   return submission;
}

void
radv_shader_dma_push_submission(struct radv_device *device, struct radv_shader_dma_submission *submission, uint64_t seq)
{
   submission->seq = seq;

   mtx_lock(&device->shader_dma_submission_list_mutex);

   list_addtail(&submission->list, &device->shader_dma_submissions);
   cnd_signal(&device->shader_dma_submission_list_cond);

   mtx_unlock(&device->shader_dma_submission_list_mutex);
}

struct radv_shader_dma_submission *
radv_shader_dma_get_submission(struct radv_device *device, struct radeon_winsys_bo *bo, uint64_t va, uint64_t size)
{
   struct radv_shader_dma_submission *submission = radv_shader_dma_pop_submission(device);
   struct radeon_cmdbuf *cs = submission->cs;
   struct radeon_winsys *ws = device->ws;
   VkResult result;

   /* Wait for potentially in-flight submission to settle */
   result = radv_shader_wait_for_upload(device, submission->seq);
   if (result != VK_SUCCESS)
      goto fail;

   ws->cs_reset(cs);

   if (submission->bo_size < size) {
      result = radv_shader_dma_resize_upload_buf(submission, ws, size);
      if (result != VK_SUCCESS)
         goto fail;
   }

   radv_sdma_copy_buffer(device, cs, radv_buffer_get_va(submission->bo), va, size);
   radv_cs_add_buffer(ws, cs, submission->bo);
   radv_cs_add_buffer(ws, cs, bo);

   result = ws->cs_finalize(cs);
   if (result != VK_SUCCESS)
      goto fail;

   return submission;

fail:
   radv_shader_dma_push_submission(device, submission, 0);

   return NULL;
}

/*
 * If upload_seq_out is NULL, this function blocks until the DMA is complete. Otherwise, the
 * semaphore value to wait on device->shader_upload_sem is stored in *upload_seq_out.
 */
bool
radv_shader_dma_submit(struct radv_device *device, struct radv_shader_dma_submission *submission,
                       uint64_t *upload_seq_out)
{
   struct radeon_cmdbuf *cs = submission->cs;
   struct radeon_winsys *ws = device->ws;
   VkResult result;

   mtx_lock(&device->shader_upload_hw_ctx_mutex);

   uint64_t upload_seq = device->shader_upload_seq + 1;

   struct vk_semaphore *semaphore = vk_semaphore_from_handle(device->shader_upload_sem);
   struct vk_sync *sync = vk_semaphore_get_active_sync(semaphore);
   const struct vk_sync_signal signal_info = {
      .sync = sync,
      .signal_value = upload_seq,
      .stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
   };

   struct radv_winsys_submit_info submit = {
      .ip_type = AMD_IP_SDMA,
      .queue_index = 0,
      .cs_array = &cs,
      .cs_count = 1,
   };

   result = ws->cs_submit(device->shader_upload_hw_ctx, &submit, 0, NULL, 1, &signal_info);
   if (result != VK_SUCCESS) {
      mtx_unlock(&device->shader_upload_hw_ctx_mutex);
      radv_shader_dma_push_submission(device, submission, 0);
      return false;
   }
   device->shader_upload_seq = upload_seq;
   mtx_unlock(&device->shader_upload_hw_ctx_mutex);

   radv_shader_dma_push_submission(device, submission, upload_seq);

   if (upload_seq_out) {
      *upload_seq_out = upload_seq;
   } else {
      result = radv_shader_wait_for_upload(device, upload_seq);
      if (result != VK_SUCCESS)
         return false;
   }

   return true;
}

static bool
radv_shader_upload(struct radv_device *device, struct radv_shader *shader, const struct radv_shader_binary *binary)
{
   if (device->shader_use_invisible_vram) {
      struct radv_shader_dma_submission *submission =
         radv_shader_dma_get_submission(device, shader->bo, shader->va, shader->code_size);
      if (!submission)
         return false;

      if (!radv_shader_binary_upload(device, binary, shader, submission->ptr)) {
         radv_shader_dma_push_submission(device, submission, 0);
         return false;
      }

      if (!radv_shader_dma_submit(device, submission, &shader->upload_seq))
         return false;
   } else {
      void *dest_ptr = shader->alloc->arena->ptr + shader->alloc->offset;

      if (!radv_shader_binary_upload(device, binary, shader, dest_ptr))
         return false;
   }
   return true;
}

unsigned
radv_get_max_waves(const struct radv_device *device, const struct ac_shader_config *conf,
                   const struct radv_shader_info *info)
{
   const struct radeon_info *rad_info = &device->physical_device->rad_info;
   const enum amd_gfx_level gfx_level = rad_info->gfx_level;
   const uint8_t wave_size = info->wave_size;
   gl_shader_stage stage = info->stage;
   unsigned max_simd_waves = rad_info->max_waves_per_simd;
   unsigned lds_per_wave = 0;

   if (stage == MESA_SHADER_FRAGMENT) {
      lds_per_wave = conf->lds_size * rad_info->lds_encode_granularity + info->ps.num_interp * 48;
      lds_per_wave = align(lds_per_wave, rad_info->lds_alloc_granularity);
   } else if (stage == MESA_SHADER_COMPUTE || stage == MESA_SHADER_TASK) {
      unsigned max_workgroup_size = info->workgroup_size;
      lds_per_wave = align(conf->lds_size * rad_info->lds_encode_granularity, rad_info->lds_alloc_granularity);
      lds_per_wave /= DIV_ROUND_UP(max_workgroup_size, wave_size);
   }

   if (conf->num_sgprs && gfx_level < GFX10) {
      unsigned sgprs = align(conf->num_sgprs, gfx_level >= GFX8 ? 16 : 8);
      max_simd_waves = MIN2(max_simd_waves, rad_info->num_physical_sgprs_per_simd / sgprs);
   }

   if (conf->num_vgprs) {
      unsigned physical_vgprs = rad_info->num_physical_wave64_vgprs_per_simd * (64 / wave_size);
      unsigned vgprs = align(conf->num_vgprs, wave_size == 32 ? 8 : 4);
      if (gfx_level >= GFX10_3) {
         unsigned real_vgpr_gran = rad_info->num_physical_wave64_vgprs_per_simd / 64;
         vgprs = util_align_npot(vgprs, real_vgpr_gran * (wave_size == 32 ? 2 : 1));
      }
      max_simd_waves = MIN2(max_simd_waves, physical_vgprs / vgprs);
   }

   unsigned simd_per_workgroup = rad_info->num_simd_per_compute_unit;
   if (gfx_level >= GFX10)
      simd_per_workgroup *= 2; /* like lds_size_per_workgroup, assume WGP on GFX10+ */

   unsigned max_lds_per_simd = rad_info->lds_size_per_workgroup / simd_per_workgroup;
   if (lds_per_wave)
      max_simd_waves = MIN2(max_simd_waves, DIV_ROUND_UP(max_lds_per_simd, lds_per_wave));

   return gfx_level >= GFX10 ? max_simd_waves * (wave_size / 32) : max_simd_waves;
}

unsigned
radv_get_max_scratch_waves(const struct radv_device *device, struct radv_shader *shader)
{
   const unsigned num_cu = device->physical_device->rad_info.num_cu;

   return MIN2(device->scratch_waves, 4 * num_cu * shader->max_waves);
}

VkResult
radv_shader_create_uncached(struct radv_device *device, const struct radv_shader_binary *binary, bool replayable,
                            struct radv_serialized_shader_arena_block *replay_block, struct radv_shader **out_shader)
{
   VkResult result = VK_SUCCESS;
   struct radv_shader *shader = calloc(1, sizeof(struct radv_shader));
   if (!shader) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto out;
   }
   simple_mtx_init(&shader->replay_mtx, mtx_plain);

   vk_pipeline_cache_object_init(&device->vk, &shader->base, &radv_shader_ops, shader->hash, sizeof(shader->hash));

   shader->info = binary->info;
   shader->config = binary->config;
   shader->max_waves = radv_get_max_waves(device, &shader->config, &shader->info);

   if (binary->type == RADV_BINARY_TYPE_RTLD) {
#if !defined(USE_LIBELF)
      goto out;
#else
      struct ac_rtld_binary rtld_binary = {0};

      if (!radv_open_rtld_binary(device, binary, &rtld_binary)) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto out;
      }

      shader->code_size = rtld_binary.rx_size;
      shader->exec_size = rtld_binary.exec_size;
      ac_rtld_close(&rtld_binary);
#endif
   } else {
      struct radv_shader_binary_legacy *bin = (struct radv_shader_binary_legacy *)binary;

      shader->code_size = bin->code_size;
      shader->exec_size = bin->exec_size;

      if (bin->stats_size) {
         shader->statistics = calloc(bin->stats_size, 1);
         memcpy(shader->statistics, bin->data, bin->stats_size);
      }
   }

   if (replay_block) {
      shader->alloc = radv_replay_shader_arena_block(device, replay_block, shader);
      if (!shader->alloc) {
         result = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS;
         goto out;
      }

      shader->has_replay_alloc = true;
   } else {
      shader->alloc = radv_alloc_shader_memory(device, shader->code_size, replayable, shader);
      if (!shader->alloc) {
         result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
         goto out;
      }
   }

   shader->bo = shader->alloc->arena->bo;
   shader->va = radv_buffer_get_va(shader->bo) + shader->alloc->offset;

   if (!radv_shader_upload(device, shader, binary)) {
      result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
      goto out;
   }

   *out_shader = shader;

out:
   if (result != VK_SUCCESS) {
      free(shader);
      *out_shader = NULL;
   }

   return result;
}

bool
radv_shader_reupload(struct radv_device *device, struct radv_shader *shader)
{
   if (device->shader_use_invisible_vram) {
      struct radv_shader_dma_submission *submission =
         radv_shader_dma_get_submission(device, shader->bo, shader->va, shader->code_size);
      if (!submission)
         return false;

      memcpy(submission->ptr, shader->code, shader->code_size);

      if (!radv_shader_dma_submit(device, submission, &shader->upload_seq))
         return false;
   } else {
      void *dest_ptr = shader->alloc->arena->ptr + shader->alloc->offset;
      memcpy(dest_ptr, shader->code, shader->code_size);
   }
   return true;
}

static bool
radv_shader_part_binary_upload(struct radv_device *device, const struct radv_shader_part_binary *bin,
                               struct radv_shader_part *shader_part)
{
   struct radv_shader_dma_submission *submission = NULL;
   void *dest_ptr;

   if (device->shader_use_invisible_vram) {
      uint64_t va = radv_buffer_get_va(shader_part->alloc->arena->bo) + shader_part->alloc->offset;
      submission = radv_shader_dma_get_submission(device, shader_part->alloc->arena->bo, va, bin->code_size);
      if (!submission)
         return false;

      dest_ptr = submission->ptr;
   } else {
      dest_ptr = shader_part->alloc->arena->ptr + shader_part->alloc->offset;
   }

   memcpy(dest_ptr, bin->data, bin->code_size);

   if (device->shader_use_invisible_vram) {
      if (!radv_shader_dma_submit(device, submission, &shader_part->upload_seq))
         return false;
   }

   return true;
}

struct radv_shader_part *
radv_shader_part_create(struct radv_device *device, struct radv_shader_part_binary *binary, unsigned wave_size)
{
   struct radv_shader_part *shader_part;

   shader_part = calloc(1, sizeof(struct radv_shader_part));
   if (!shader_part)
      return NULL;

   shader_part->ref_count = 1;
   shader_part->code_size = binary->code_size;
   shader_part->rsrc1 =
      S_00B848_VGPRS((binary->num_vgprs - 1) / (wave_size == 32 ? 8 : 4)) | S_00B228_SGPRS((binary->num_sgprs - 1) / 8);
   shader_part->disasm_string = binary->disasm_size ? strdup((const char *)(binary->data + binary->code_size)) : NULL;

   shader_part->spi_shader_col_format = binary->info.spi_shader_col_format;
   shader_part->spi_shader_z_format = binary->info.spi_shader_z_format;

   /* Allocate memory and upload. */
   shader_part->alloc = radv_alloc_shader_memory(device, shader_part->code_size, false, NULL);
   if (!shader_part->alloc)
      goto fail;

   shader_part->bo = shader_part->alloc->arena->bo;
   shader_part->va = radv_buffer_get_va(shader_part->bo) + shader_part->alloc->offset;

   if (!radv_shader_part_binary_upload(device, binary, shader_part))
      goto fail;

   return shader_part;

fail:
   radv_shader_part_destroy(device, shader_part);
   return NULL;
}

bool
radv_shader_part_cache_init(struct radv_shader_part_cache *cache, struct radv_shader_part_cache_ops *ops)
{
   cache->ops = ops;
   if (!_mesa_set_init(&cache->entries, NULL, cache->ops->hash, cache->ops->equals))
      return false;
   simple_mtx_init(&cache->lock, mtx_plain);
   return true;
}

void
radv_shader_part_cache_finish(struct radv_device *device, struct radv_shader_part_cache *cache)
{
   set_foreach (&cache->entries, entry)
      radv_shader_part_unref(device, radv_shader_part_from_cache_entry(entry->key));
   simple_mtx_destroy(&cache->lock);
   ralloc_free(cache->entries.table);
}

/*
 * A cache with atomics-free fast path for prolog / epilog lookups.
 *
 * VS prologs and PS/TCS epilogs are used to support dynamic states. In
 * particular dynamic blend state is heavily used by Zink. These are called
 * every frame as a part of command buffer building, so these functions are
 * on the hot path.
 *
 * Originally this was implemented with a rwlock, but this lead to high
 * overhead. To avoid locking altogether in the hot path, the cache is done
 * at two levels: one at device level, and another at each CS. Access to the
 * CS cache is externally synchronized and do not require a lock.
 */
struct radv_shader_part *
radv_shader_part_cache_get(struct radv_device *device, struct radv_shader_part_cache *cache, struct set *local_entries,
                           const void *key)
{
   struct set_entry *local, *global;
   bool local_found, global_found;
   uint32_t hash = cache->ops->hash(key);

   local = _mesa_set_search_or_add_pre_hashed(local_entries, hash, key, &local_found);
   if (local_found)
      return radv_shader_part_from_cache_entry(local->key);

   simple_mtx_lock(&cache->lock);
   global = _mesa_set_search_or_add_pre_hashed(&cache->entries, hash, key, &global_found);
   if (global_found) {
      simple_mtx_unlock(&cache->lock);
      local->key = global->key;
      return radv_shader_part_from_cache_entry(global->key);
   }

   struct radv_shader_part *shader_part = cache->ops->create(device, key);
   if (!shader_part) {
      _mesa_set_remove(&cache->entries, global);
      simple_mtx_unlock(&cache->lock);
      _mesa_set_remove(local_entries, local);
      return NULL;
   }

   /* Make the set entry a pointer to the key, so that the hash and equals
    * functions from radv_shader_part_cache_ops can be directly used.
    */
   global->key = &shader_part->key;
   simple_mtx_unlock(&cache->lock);
   local->key = &shader_part->key;
   return shader_part;
}

static char *
radv_dump_nir_shaders(struct nir_shader *const *shaders, int shader_count)
{
   char *data = NULL;
   char *ret = NULL;
   size_t size = 0;
   struct u_memstream mem;
   if (u_memstream_open(&mem, &data, &size)) {
      FILE *const memf = u_memstream_get(&mem);
      for (int i = 0; i < shader_count; ++i)
         nir_print_shader(shaders[i], memf);
      u_memstream_close(&mem);
   }

   ret = malloc(size + 1);
   if (ret) {
      memcpy(ret, data, size);
      ret[size] = 0;
   }
   free(data);
   return ret;
}

static void
radv_aco_build_shader_binary(void **bin, const struct ac_shader_config *config, const char *llvm_ir_str,
                             unsigned llvm_ir_size, const char *disasm_str, unsigned disasm_size, uint32_t *statistics,
                             uint32_t stats_size, uint32_t exec_size, const uint32_t *code, uint32_t code_dw,
                             const struct aco_symbol *symbols, unsigned num_symbols)
{
   struct radv_shader_binary **binary = (struct radv_shader_binary **)bin;
   size_t size = llvm_ir_size;

   size += disasm_size;
   size += stats_size;

   size += code_dw * sizeof(uint32_t) + sizeof(struct radv_shader_binary_legacy);

   /* We need to calloc to prevent uninitialized data because this will be used
    * directly for the disk cache. Uninitialized data can appear because of
    * padding in the struct or because legacy_binary->data can be at an offset
    * from the start less than sizeof(radv_shader_binary_legacy). */
   struct radv_shader_binary_legacy *legacy_binary = (struct radv_shader_binary_legacy *)calloc(size, 1);
   legacy_binary->base.type = RADV_BINARY_TYPE_LEGACY;
   legacy_binary->base.total_size = size;
   legacy_binary->base.config = *config;

   if (stats_size)
      memcpy(legacy_binary->data, statistics, stats_size);
   legacy_binary->stats_size = stats_size;

   memcpy(legacy_binary->data + legacy_binary->stats_size, code, code_dw * sizeof(uint32_t));
   legacy_binary->exec_size = exec_size;
   legacy_binary->code_size = code_dw * sizeof(uint32_t);

   legacy_binary->disasm_size = 0;
   legacy_binary->ir_size = llvm_ir_size;

   if (llvm_ir_size) {
      memcpy((char *)legacy_binary->data + legacy_binary->stats_size + legacy_binary->code_size, llvm_ir_str,
             llvm_ir_size);
   }

   legacy_binary->disasm_size = disasm_size;
   if (disasm_size) {
      memcpy((char *)legacy_binary->data + legacy_binary->stats_size + legacy_binary->code_size + llvm_ir_size,
             disasm_str, disasm_size);
   }
   *binary = (struct radv_shader_binary *)legacy_binary;
}

static void
radv_fill_nir_compiler_options(struct radv_nir_compiler_options *options, struct radv_device *device,
                               const struct radv_pipeline_key *key, bool should_use_wgp, bool can_dump_shader,
                               bool is_meta_shader, bool keep_shader_info, bool keep_statistic_info)
{
   if (key)
      options->key = *key;

   /* robust_buffer_access_llvm here used by LLVM only, pipeline robustness is not exposed there. */
   options->robust_buffer_access_llvm = device->buffer_robustness >= RADV_BUFFER_ROBUSTNESS_1;
   options->wgp_mode = should_use_wgp;
   options->info = &device->physical_device->rad_info;
   options->dump_shader = can_dump_shader;
   options->dump_preoptir = options->dump_shader && device->instance->debug_flags & RADV_DEBUG_PREOPTIR;
   options->record_ir = keep_shader_info;
   options->record_stats = keep_statistic_info;
   options->check_ir = device->instance->debug_flags & RADV_DEBUG_CHECKIR;
   options->enable_mrt_output_nan_fixup = options->key.ps.epilog.enable_mrt_output_nan_fixup;
}

static void
radv_capture_shader_executable_info(struct radv_device *device, struct radv_shader *shader,
                                    struct nir_shader *const *shaders, int shader_count,
                                    const struct radv_shader_binary *binary)
{
   shader->nir_string = radv_dump_nir_shaders(shaders, shader_count);

   if (binary->type == RADV_BINARY_TYPE_RTLD) {
#if !defined(USE_LIBELF)
      return;
#else
      struct radv_shader_binary_rtld *bin = (struct radv_shader_binary_rtld *)binary;
      struct ac_rtld_binary rtld_binary = {0};

      if (!radv_open_rtld_binary(device, binary, &rtld_binary)) {
         return;
      }

      const char *disasm_data;
      size_t disasm_size;
      if (!ac_rtld_get_section_by_name(&rtld_binary, ".AMDGPU.disasm", &disasm_data, &disasm_size)) {
         return;
      }

      shader->ir_string = bin->llvm_ir_size ? strdup((const char *)(bin->data + bin->elf_size)) : NULL;
      shader->disasm_string = malloc(disasm_size + 1);
      memcpy(shader->disasm_string, disasm_data, disasm_size);
      shader->disasm_string[disasm_size] = 0;

      ac_rtld_close(&rtld_binary);
#endif
   } else {
      struct radv_shader_binary_legacy *bin = (struct radv_shader_binary_legacy *)binary;

      shader->ir_string = bin->ir_size ? strdup((const char *)(bin->data + bin->stats_size + bin->code_size)) : NULL;
      shader->disasm_string =
         bin->disasm_size ? strdup((const char *)(bin->data + bin->stats_size + bin->code_size + bin->ir_size)) : NULL;
   }
}

static struct radv_shader_binary *
shader_compile(struct radv_device *device, struct nir_shader *const *shaders, int shader_count, gl_shader_stage stage,
               const struct radv_shader_info *info, const struct radv_shader_args *args,
               struct radv_nir_compiler_options *options)
{
   struct radv_shader_debug_data debug_data = {
      .device = device,
      .object = NULL,
   };
   options->debug.func = radv_compiler_debug;
   options->debug.private_data = &debug_data;

   struct radv_shader_binary *binary = NULL;

#if LLVM_AVAILABLE
   if (radv_use_llvm_for_stage(device, stage) || options->dump_shader || options->record_ir)
      ac_init_llvm_once();

   if (radv_use_llvm_for_stage(device, stage)) {
      llvm_compile_shader(options, info, shader_count, shaders, &binary, args);
#else
   if (false) {
#endif
   } else {
      struct aco_shader_info ac_info;
      struct aco_compiler_options ac_opts;
      radv_aco_convert_opts(&ac_opts, options, args);
      radv_aco_convert_shader_info(&ac_info, info, args, &device->cache_key, options->info->gfx_level);
      aco_compile_shader(&ac_opts, &ac_info, shader_count, shaders, &args->ac, &radv_aco_build_shader_binary,
                         (void **)&binary);
   }

   binary->info = *info;

   if (!radv_postprocess_binary_config(device, binary, args)) {
      free(binary);
      return NULL;
   }

   return binary;
}

struct radv_shader_binary *
radv_shader_nir_to_asm(struct radv_device *device, struct radv_shader_stage *pl_stage,
                       struct nir_shader *const *shaders, int shader_count, const struct radv_pipeline_key *key,
                       bool keep_shader_info, bool keep_statistic_info)
{
   gl_shader_stage stage = shaders[shader_count - 1]->info.stage;
   struct radv_shader_info *info = &pl_stage->info;

   struct radv_nir_compiler_options options = {0};
   radv_fill_nir_compiler_options(&options, device, key, radv_should_use_wgp_mode(device, stage, info),
                                  radv_can_dump_shader(device, shaders[0], false), is_meta_shader(shaders[0]),
                                  keep_shader_info, keep_statistic_info);

   struct radv_shader_binary *binary =
      shader_compile(device, shaders, shader_count, stage, info, &pl_stage->args, &options);

   return binary;
}

void
radv_shader_generate_debug_info(struct radv_device *device, bool dump_shader, bool keep_shader_info,
                                struct radv_shader_binary *binary, struct radv_shader *shader,
                                struct nir_shader *const *shaders, int shader_count, struct radv_shader_info *info)
{
   if (dump_shader || keep_shader_info)
      radv_capture_shader_executable_info(device, shader, shaders, shader_count, binary);

   if (dump_shader) {
      fprintf(stderr, "%s", radv_get_shader_name(info, shaders[0]->info.stage));
      for (int i = 1; i < shader_count; ++i)
         fprintf(stderr, " + %s", radv_get_shader_name(info, shaders[i]->info.stage));

      fprintf(stderr, "\ndisasm:\n%s\n", shader->disasm_string);
   }
}

struct radv_shader *
radv_create_trap_handler_shader(struct radv_device *device)
{
   gl_shader_stage stage = MESA_SHADER_COMPUTE;
   struct radv_shader_info info = {0};
   struct radv_pipeline_key key = {0};
   struct radv_nir_compiler_options options = {0};
   radv_fill_nir_compiler_options(&options, device, &key, radv_should_use_wgp_mode(device, stage, &info), false, false,
                                  false, false);

   nir_builder b = radv_meta_init_shader(device, stage, "meta_trap_handler");

   info.wave_size = 64;
   info.type = RADV_SHADER_TYPE_TRAP_HANDLER;

   struct radv_shader_args args;
   radv_declare_shader_args(device, &key, &info, stage, MESA_SHADER_NONE, &args);

   struct radv_shader_binary *binary = shader_compile(device, &b.shader, 1, stage, &info, &args, &options);
   struct radv_shader *shader;
   radv_shader_create_uncached(device, binary, false, NULL, &shader);

   ralloc_free(b.shader);
   free(binary);

   return shader;
}

static void
radv_aco_build_shader_part(void **bin, uint32_t num_sgprs, uint32_t num_vgprs, const uint32_t *code, uint32_t code_size,
                           const char *disasm_str, uint32_t disasm_size)
{
   struct radv_shader_part_binary **binary = (struct radv_shader_part_binary **)bin;
   size_t size = code_size * sizeof(uint32_t) + sizeof(struct radv_shader_part_binary);

   size += disasm_size;
   struct radv_shader_part_binary *part_binary = (struct radv_shader_part_binary *)calloc(size, 1);

   part_binary->num_sgprs = num_sgprs;
   part_binary->num_vgprs = num_vgprs;
   part_binary->total_size = size;
   part_binary->code_size = code_size * sizeof(uint32_t);
   memcpy(part_binary->data, code, part_binary->code_size);
   if (disasm_size) {
      memcpy((char *)part_binary->data + part_binary->code_size, disasm_str, disasm_size);
      part_binary->disasm_size = disasm_size;
   }

   *binary = part_binary;
}

struct radv_shader *
radv_create_rt_prolog(struct radv_device *device)
{
   struct radv_shader *prolog;
   struct radv_shader_args in_args = {0};
   struct radv_shader_args out_args = {0};
   struct radv_nir_compiler_options options = {0};
   radv_fill_nir_compiler_options(&options, device, NULL, false,
                                  device->instance->debug_flags & RADV_DEBUG_DUMP_PROLOGS, false,
                                  device->instance->debug_flags & RADV_DEBUG_HANG, false);
   struct radv_shader_info info = {0};
   info.stage = MESA_SHADER_COMPUTE;
   info.loads_push_constants = true;
   info.desc_set_used_mask = -1; /* just to force indirection */
   info.wave_size = device->physical_device->rt_wave_size;
   info.workgroup_size = info.wave_size;
   info.user_data_0 = R_00B900_COMPUTE_USER_DATA_0;
   info.cs.is_rt_shader = true;
   info.cs.uses_ray_launch_size = true;
   info.cs.uses_dynamic_rt_callable_stack = true;
   info.cs.block_size[0] = 8;
   info.cs.block_size[1] = device->physical_device->rt_wave_size == 64 ? 8 : 4;
   info.cs.block_size[2] = 1;
   info.cs.uses_thread_id[0] = true;
   info.cs.uses_thread_id[1] = true;
   for (unsigned i = 0; i < 3; i++)
      info.cs.uses_block_id[i] = true;

   struct radv_pipeline_key pipeline_key = {0};

   radv_declare_shader_args(device, &pipeline_key, &info, MESA_SHADER_COMPUTE, MESA_SHADER_NONE, &in_args);
   radv_declare_rt_shader_args(options.info->gfx_level, &out_args);
   info.user_sgprs_locs = in_args.user_sgprs_locs;

#if LLVM_AVAILABLE
   if (options.dump_shader || options.record_ir)
      ac_init_llvm_once();
#endif

   struct radv_shader_binary *binary = NULL;
   struct aco_shader_info ac_info;
   struct aco_compiler_options ac_opts;
   radv_aco_convert_shader_info(&ac_info, &info, &in_args, &device->cache_key, options.info->gfx_level);
   radv_aco_convert_opts(&ac_opts, &options, &in_args);
   aco_compile_rt_prolog(&ac_opts, &ac_info, &in_args.ac, &out_args.ac, &radv_aco_build_shader_binary,
                         (void **)&binary);
   binary->info = info;

   radv_postprocess_binary_config(device, binary, &in_args);
   radv_shader_create_uncached(device, binary, false, NULL, &prolog);
   if (!prolog)
      goto done;

   if (device->keep_shader_info || options.dump_shader) {
      radv_capture_shader_executable_info(device, prolog, NULL, 0, binary);
   }

   if (options.dump_shader) {
      fprintf(stderr, "Raytracing prolog");
      fprintf(stderr, "\ndisasm:\n%s\n", prolog->disasm_string);
   }

done:
   free(binary);
   return prolog;
}

struct radv_shader_part *
radv_create_vs_prolog(struct radv_device *device, const struct radv_vs_prolog_key *key)
{
   struct radv_shader_part *prolog;
   struct radv_shader_args args = {0};
   struct radv_nir_compiler_options options = {0};
   radv_fill_nir_compiler_options(&options, device, NULL, false,
                                  device->instance->debug_flags & RADV_DEBUG_DUMP_PROLOGS, false,
                                  device->instance->debug_flags & RADV_DEBUG_HANG, false);

   struct radv_shader_info info = {0};
   info.stage = MESA_SHADER_VERTEX;
   info.wave_size = key->wave32 ? 32 : 64;
   info.vs.needs_instance_id = true;
   info.vs.needs_base_instance = true;
   info.vs.needs_draw_id = true;
   info.vs.use_per_attribute_vb_descs = true;
   info.vs.vb_desc_usage_mask = BITFIELD_MASK(key->num_attributes);
   info.vs.has_prolog = true;
   info.vs.as_ls = key->as_ls;
   info.is_ngg = key->is_ngg;

   struct radv_pipeline_key pipeline_key = {0};

   radv_declare_shader_args(device, &pipeline_key, &info, key->next_stage,
                            key->next_stage != MESA_SHADER_VERTEX ? MESA_SHADER_VERTEX : MESA_SHADER_NONE, &args);

   info.user_sgprs_locs = args.user_sgprs_locs;
   info.inline_push_constant_mask = args.ac.inline_push_const_mask;

#if LLVM_AVAILABLE
   if (options.dump_shader || options.record_ir)
      ac_init_llvm_once();
#endif

   struct radv_shader_part_binary *binary = NULL;
   struct aco_shader_info ac_info;
   struct aco_vs_prolog_info ac_prolog_info;
   struct aco_compiler_options ac_opts;
   radv_aco_convert_shader_info(&ac_info, &info, &args, &device->cache_key, options.info->gfx_level);
   radv_aco_convert_opts(&ac_opts, &options, &args);
   radv_aco_convert_vs_prolog_key(&ac_prolog_info, key, &args);
   aco_compile_vs_prolog(&ac_opts, &ac_info, &ac_prolog_info, &args.ac, &radv_aco_build_shader_part, (void **)&binary);

   prolog = radv_shader_part_create(device, binary, info.wave_size);
   if (!prolog)
      goto fail;

   prolog->key.vs = *key;
   prolog->nontrivial_divisors = key->nontrivial_divisors;

   if (options.dump_shader) {
      fprintf(stderr, "Vertex prolog");
      fprintf(stderr, "\ndisasm:\n%s\n", prolog->disasm_string);
   }

   free(binary);

   return prolog;

fail:
   free(binary);
   return NULL;
}

struct radv_shader_part *
radv_create_ps_epilog(struct radv_device *device, const struct radv_ps_epilog_key *key,
                      struct radv_shader_part_binary **binary_out)
{
   struct radv_shader_part *epilog;
   struct radv_shader_args args = {0};
   struct radv_nir_compiler_options options = {0};
   radv_fill_nir_compiler_options(&options, device, NULL, false,
                                  device->instance->debug_flags & RADV_DEBUG_DUMP_EPILOGS, false,
                                  device->instance->debug_flags & RADV_DEBUG_HANG, false);

   struct radv_shader_info info = {0};
   info.stage = MESA_SHADER_FRAGMENT;
   info.wave_size = device->physical_device->ps_wave_size;
   info.workgroup_size = 64;

   radv_declare_ps_epilog_args(device, key, &args);

#if LLVM_AVAILABLE
   if (options.dump_shader || options.record_ir)
      ac_init_llvm_once();
#endif

   struct radv_shader_part_binary *binary = NULL;
   struct aco_shader_info ac_info;
   struct aco_ps_epilog_info ac_epilog_info = {0};
   struct aco_compiler_options ac_opts;
   radv_aco_convert_shader_info(&ac_info, &info, &args, &device->cache_key, options.info->gfx_level);
   radv_aco_convert_opts(&ac_opts, &options, &args);
   radv_aco_convert_ps_epilog_key(&ac_epilog_info, key, &args);
   aco_compile_ps_epilog(&ac_opts, &ac_info, &ac_epilog_info, &args.ac, &radv_aco_build_shader_part, (void **)&binary);

   binary->info.spi_shader_col_format = key->spi_shader_col_format;
   binary->info.spi_shader_z_format = key->spi_shader_z_format;

   epilog = radv_shader_part_create(device, binary, info.wave_size);
   if (!epilog)
      goto fail;

   epilog->key.ps = *key;

   if (options.dump_shader) {
      fprintf(stderr, "Fragment epilog");
      fprintf(stderr, "\ndisasm:\n%s\n", epilog->disasm_string);
   }

   if (binary_out) {
      *binary_out = binary;
   } else {
      free(binary);
   }

   return epilog;

fail:
   free(binary);
   return NULL;
}

struct radv_shader_part *
radv_create_tcs_epilog(struct radv_device *device, const struct radv_tcs_epilog_key *key)
{
   struct radv_shader_part *epilog;
   struct radv_shader_args args = {0};
   struct radv_nir_compiler_options options = {0};
   radv_fill_nir_compiler_options(&options, device, NULL, false,
                                  device->instance->debug_flags & RADV_DEBUG_DUMP_EPILOGS, false,
                                  device->instance->debug_flags & RADV_DEBUG_HANG, false);

   struct radv_shader_info info = {0};
   info.stage = MESA_SHADER_TESS_CTRL;
   info.wave_size = device->physical_device->ge_wave_size;
   info.workgroup_size = 256;

   radv_declare_tcs_epilog_args(device, key, &args);

#if LLVM_AVAILABLE
   if (options.dump_shader || options.record_ir)
      ac_init_llvm_once();
#endif

   struct radv_shader_part_binary *binary = NULL;
   struct aco_shader_info ac_info;
   struct aco_tcs_epilog_info ac_epilog_info;
   struct aco_compiler_options ac_opts;
   radv_aco_convert_shader_info(&ac_info, &info, &args, &device->cache_key, options.info->gfx_level);
   radv_aco_convert_opts(&ac_opts, &options, &args);
   radv_aco_convert_tcs_epilog_key(&ac_epilog_info, key, &args);
   aco_compile_tcs_epilog(&ac_opts, &ac_info, &ac_epilog_info, &args.ac, &radv_aco_build_shader_part, (void **)&binary);

   epilog = radv_shader_part_create(device, binary, info.wave_size);
   if (!epilog)
      goto fail;

   epilog->key.tcs = *key;

   if (options.dump_shader) {
      fprintf(stderr, "TCS epilog");
      fprintf(stderr, "\ndisasm:\n%s\n", epilog->disasm_string);
   }

   free(binary);
   return epilog;

fail:
   free(binary);
   return NULL;
}

void
radv_shader_part_destroy(struct radv_device *device, struct radv_shader_part *shader_part)
{
   assert(shader_part->ref_count == 0);

   if (device->shader_use_invisible_vram) {
      /* Wait for any pending upload to complete, or we'll be writing into freed shader memory. */
      radv_shader_wait_for_upload(device, shader_part->upload_seq);
   }

   if (shader_part->alloc)
      radv_free_shader_memory(device, shader_part->alloc);
   free(shader_part->disasm_string);
   free(shader_part);
}

uint64_t
radv_shader_get_va(const struct radv_shader *shader)
{
   return shader->va;
}

struct radv_shader *
radv_find_shader(struct radv_device *device, uint64_t pc)
{
   mtx_lock(&device->shader_arena_mutex);
   list_for_each_entry (struct radv_shader_arena, arena, &device->shader_arenas, list) {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
      list_for_each_entry (union radv_shader_arena_block, block, &arena->entries, list) {
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
         uint64_t start = radv_buffer_get_va(block->arena->bo) + block->offset;
         if (!block->freelist.prev && pc >= start && pc < start + block->size) {
            mtx_unlock(&device->shader_arena_mutex);
            return (struct radv_shader *)block->freelist.next;
         }
      }
   }

   mtx_unlock(&device->shader_arena_mutex);
   return NULL;
}

const char *
radv_get_shader_name(const struct radv_shader_info *info, gl_shader_stage stage)
{
   switch (stage) {
   case MESA_SHADER_VERTEX:
      if (info->vs.as_ls)
         return "Vertex Shader as LS";
      else if (info->vs.as_es)
         return "Vertex Shader as ES";
      else if (info->is_ngg)
         return "Vertex Shader as ESGS";
      else
         return "Vertex Shader as VS";
   case MESA_SHADER_TESS_CTRL:
      return "Tessellation Control Shader";
   case MESA_SHADER_TESS_EVAL:
      if (info->tes.as_es)
         return "Tessellation Evaluation Shader as ES";
      else if (info->is_ngg)
         return "Tessellation Evaluation Shader as ESGS";
      else
         return "Tessellation Evaluation Shader as VS";
   case MESA_SHADER_GEOMETRY:
      return "Geometry Shader";
   case MESA_SHADER_FRAGMENT:
      return "Pixel Shader";
   case MESA_SHADER_COMPUTE:
      return "Compute Shader";
   case MESA_SHADER_MESH:
      return "Mesh Shader as NGG";
   case MESA_SHADER_TASK:
      return "Task Shader as CS";
   case MESA_SHADER_RAYGEN:
      return "Ray Generation Shader as CS Function";
   case MESA_SHADER_CLOSEST_HIT:
      return "Closest Hit Shader as CS Function";
   case MESA_SHADER_INTERSECTION:
      return "Intersection Shader as CS Function";
   case MESA_SHADER_ANY_HIT:
      return "Any Hit Shader as CS Function";
   case MESA_SHADER_MISS:
      return "Miss Shader as CS Function";
   case MESA_SHADER_CALLABLE:
      return "Callable Shader as CS Function";
   default:
      return "Unknown shader";
   };
}

unsigned
radv_compute_spi_ps_input(const struct radv_pipeline_key *pipeline_key, const struct radv_shader_info *info)
{
   unsigned spi_ps_input;

   spi_ps_input = S_0286CC_PERSP_CENTER_ENA(info->ps.reads_persp_center) |
                  S_0286CC_PERSP_CENTROID_ENA(info->ps.reads_persp_centroid) |
                  S_0286CC_PERSP_SAMPLE_ENA(info->ps.reads_persp_sample) |
                  S_0286CC_LINEAR_CENTER_ENA(info->ps.reads_linear_center) |
                  S_0286CC_LINEAR_CENTROID_ENA(info->ps.reads_linear_centroid) |
                  S_0286CC_LINEAR_SAMPLE_ENA(info->ps.reads_linear_sample) |
                  S_0286CC_PERSP_PULL_MODEL_ENA(info->ps.reads_barycentric_model) |
                  S_0286CC_FRONT_FACE_ENA(info->ps.reads_front_face);

   if (info->ps.reads_frag_coord_mask || info->ps.reads_sample_pos_mask) {
      uint8_t mask = info->ps.reads_frag_coord_mask | info->ps.reads_sample_pos_mask;

      for (unsigned i = 0; i < 4; i++) {
         if (mask & (1 << i))
            spi_ps_input |= S_0286CC_POS_X_FLOAT_ENA(1) << i;
      }

      if (pipeline_key->adjust_frag_coord_z && info->ps.reads_frag_coord_mask & (1 << 2)) {
         spi_ps_input |= S_0286CC_ANCILLARY_ENA(1);
      }
   }

   if (info->ps.reads_sample_id || info->ps.reads_frag_shading_rate || info->ps.reads_sample_mask_in) {
      spi_ps_input |= S_0286CC_ANCILLARY_ENA(1);
   }

   if (info->ps.reads_sample_mask_in || info->ps.reads_fully_covered) {
      spi_ps_input |= S_0286CC_SAMPLE_COVERAGE_ENA(1);
   }

   if (G_0286CC_POS_W_FLOAT_ENA(spi_ps_input)) {
      /* If POS_W_FLOAT (11) is enabled, at least one of PERSP_* must be enabled too */
      spi_ps_input |= S_0286CC_PERSP_CENTER_ENA(1);
   }

   if (!(spi_ps_input & 0x7F)) {
      /* At least one of PERSP_* (0xF) or LINEAR_* (0x70) must be enabled */
      spi_ps_input |= S_0286CC_PERSP_CENTER_ENA(1);
   }

   return spi_ps_input;
}

VkResult
radv_dump_shader_stats(struct radv_device *device, struct radv_pipeline *pipeline, struct radv_shader *shader,
                       gl_shader_stage stage, FILE *output)
{
   VkPipelineExecutablePropertiesKHR *props = NULL;
   uint32_t prop_count = 0;
   VkResult result;

   VkPipelineInfoKHR pipeline_info = {0};
   pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR;
   pipeline_info.pipeline = radv_pipeline_to_handle(pipeline);

   result = radv_GetPipelineExecutablePropertiesKHR(radv_device_to_handle(device), &pipeline_info, &prop_count, NULL);
   if (result != VK_SUCCESS)
      return result;

   props = calloc(prop_count, sizeof(*props));
   if (!props)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   result = radv_GetPipelineExecutablePropertiesKHR(radv_device_to_handle(device), &pipeline_info, &prop_count, props);
   if (result != VK_SUCCESS)
      goto fail;

   for (unsigned exec_idx = 0; exec_idx < prop_count; exec_idx++) {
      if (!(props[exec_idx].stages & mesa_to_vk_shader_stage(stage)))
         continue;

      VkPipelineExecutableStatisticKHR *stats = NULL;
      uint32_t stat_count = 0;

      VkPipelineExecutableInfoKHR exec_info = {0};
      exec_info.pipeline = radv_pipeline_to_handle(pipeline);
      exec_info.executableIndex = exec_idx;

      result = radv_GetPipelineExecutableStatisticsKHR(radv_device_to_handle(device), &exec_info, &stat_count, NULL);
      if (result != VK_SUCCESS)
         goto fail;

      stats = calloc(stat_count, sizeof(*stats));
      if (!stats) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto fail;
      }

      result = radv_GetPipelineExecutableStatisticsKHR(radv_device_to_handle(device), &exec_info, &stat_count, stats);
      if (result != VK_SUCCESS) {
         free(stats);
         goto fail;
      }

      fprintf(output, "\n%s:\n", radv_get_shader_name(&shader->info, stage));
      fprintf(output, "*** SHADER STATS ***\n");

      for (unsigned i = 0; i < stat_count; i++) {
         fprintf(output, "%s: ", stats[i].name);
         switch (stats[i].format) {
         case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR:
            fprintf(output, "%s", stats[i].value.b32 == VK_TRUE ? "true" : "false");
            break;
         case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR:
            fprintf(output, "%" PRIi64, stats[i].value.i64);
            break;
         case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR:
            fprintf(output, "%" PRIu64, stats[i].value.u64);
            break;
         case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
            fprintf(output, "%f", stats[i].value.f64);
            break;
         default:
            unreachable("Invalid pipeline statistic format");
         }
         fprintf(output, "\n");
      }

      fprintf(output, "********************\n\n\n");

      free(stats);
   }

fail:
   free(props);
   return result;
}
