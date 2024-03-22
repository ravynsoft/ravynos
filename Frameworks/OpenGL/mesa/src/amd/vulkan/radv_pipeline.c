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

#include "meta/radv_meta.h"
#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "nir/nir_serialize.h"
#include "nir/radv_nir.h"
#include "spirv/nir_spirv.h"
#include "util/disk_cache.h"
#include "util/mesa-sha1.h"
#include "util/os_time.h"
#include "util/u_atomic.h"
#include "radv_cs.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "radv_shader.h"
#include "radv_shader_args.h"
#include "vk_pipeline.h"
#include "vk_render_pass.h"
#include "vk_util.h"

#include "util/u_debug.h"
#include "ac_binary.h"
#include "ac_nir.h"
#include "ac_shader_util.h"
#include "aco_interface.h"
#include "sid.h"
#include "vk_format.h"
#include "vk_nir_convert_ycbcr.h"

bool
radv_shader_need_indirect_descriptor_sets(const struct radv_shader *shader)
{
   const struct radv_userdata_info *loc = radv_get_user_sgpr(shader, AC_UD_INDIRECT_DESCRIPTOR_SETS);
   return loc->sgpr_idx != -1;
}

bool
radv_pipeline_capture_shaders(const struct radv_device *device, VkPipelineCreateFlags2KHR flags)
{
   return (flags & VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR) ||
          (device->instance->debug_flags & RADV_DEBUG_DUMP_SHADERS) || device->keep_shader_info;
}

bool
radv_pipeline_capture_shader_stats(const struct radv_device *device, VkPipelineCreateFlags2KHR flags)
{
   return (flags & VK_PIPELINE_CREATE_2_CAPTURE_STATISTICS_BIT_KHR) ||
          (device->instance->debug_flags & RADV_DEBUG_DUMP_SHADER_STATS) || device->keep_shader_info;
}

void
radv_pipeline_init(struct radv_device *device, struct radv_pipeline *pipeline, enum radv_pipeline_type type)
{
   vk_object_base_init(&device->vk, &pipeline->base, VK_OBJECT_TYPE_PIPELINE);

   pipeline->type = type;
}

void
radv_pipeline_destroy(struct radv_device *device, struct radv_pipeline *pipeline,
                      const VkAllocationCallbacks *allocator)
{
   if (pipeline->cache_object)
      vk_pipeline_cache_object_unref(&device->vk, pipeline->cache_object);

   switch (pipeline->type) {
   case RADV_PIPELINE_GRAPHICS:
      radv_destroy_graphics_pipeline(device, radv_pipeline_to_graphics(pipeline));
      break;
   case RADV_PIPELINE_GRAPHICS_LIB:
      radv_destroy_graphics_lib_pipeline(device, radv_pipeline_to_graphics_lib(pipeline));
      break;
   case RADV_PIPELINE_COMPUTE:
      radv_destroy_compute_pipeline(device, radv_pipeline_to_compute(pipeline));
      break;
   case RADV_PIPELINE_RAY_TRACING:
      radv_destroy_ray_tracing_pipeline(device, radv_pipeline_to_ray_tracing(pipeline));
      break;
   default:
      unreachable("invalid pipeline type");
   }

   if (pipeline->cs.buf)
      free(pipeline->cs.buf);

   radv_rmv_log_resource_destroy(device, (uint64_t)radv_pipeline_to_handle(pipeline));
   vk_object_base_finish(&pipeline->base);
   vk_free2(&device->vk.alloc, allocator, pipeline);
}

VKAPI_ATTR void VKAPI_CALL
radv_DestroyPipeline(VkDevice _device, VkPipeline _pipeline, const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_pipeline, pipeline, _pipeline);

   if (!_pipeline)
      return;

   radv_pipeline_destroy(device, pipeline, pAllocator);
}

static enum radv_buffer_robustness
radv_convert_buffer_robustness(const struct radv_device *device, VkPipelineRobustnessBufferBehaviorEXT behaviour)
{
   switch (behaviour) {
   case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT:
      return device->buffer_robustness;
   case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT:
      return RADV_BUFFER_ROBUSTNESS_DISABLED;
   case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT:
      return RADV_BUFFER_ROBUSTNESS_1;
   case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT:
      return RADV_BUFFER_ROBUSTNESS_2;
   default:
      unreachable("Invalid pipeline robustness behavior");
   }
}

struct radv_pipeline_key
radv_generate_pipeline_key(const struct radv_device *device, const VkPipelineShaderStageCreateInfo *stages,
                           const unsigned num_stages, VkPipelineCreateFlags2KHR flags, const void *pNext)
{
   struct radv_pipeline_key key;

   memset(&key, 0, sizeof(key));

   if (flags & VK_PIPELINE_CREATE_2_DISABLE_OPTIMIZATION_BIT_KHR)
      key.optimisations_disabled = 1;

   for (unsigned i = 0; i < num_stages; ++i) {
      const VkPipelineShaderStageCreateInfo *const stage = &stages[i];
      const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *const subgroup_size =
         vk_find_struct_const(stage->pNext, PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO);
      const gl_shader_stage s = vk_to_mesa_shader_stage(stage->stage);

      if (subgroup_size) {
         if (subgroup_size->requiredSubgroupSize == 32)
            key.stage_info[s].subgroup_required_size = RADV_REQUIRED_WAVE32;
         else if (subgroup_size->requiredSubgroupSize == 64)
            key.stage_info[s].subgroup_required_size = RADV_REQUIRED_WAVE64;
         else
            unreachable("Unsupported required subgroup size.");
      }

      if (stage->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT) {
         key.stage_info[s].subgroup_require_full = 1;
      }
   }

   const VkPipelineRobustnessCreateInfoEXT *pipeline_robust_info =
      vk_find_struct_const(pNext, PIPELINE_ROBUSTNESS_CREATE_INFO_EXT);

   for (uint32_t i = 0; i < num_stages; i++) {
      gl_shader_stage stage = vk_to_mesa_shader_stage(stages[i].stage);
      const VkPipelineRobustnessCreateInfoEXT *stage_robust_info =
         vk_find_struct_const(stages[i].pNext, PIPELINE_ROBUSTNESS_CREATE_INFO_EXT);

      /* map any hit to intersection as these shaders get merged */
      if (stage == MESA_SHADER_ANY_HIT)
         stage = MESA_SHADER_INTERSECTION;

      enum radv_buffer_robustness storage_robustness = device->buffer_robustness;
      enum radv_buffer_robustness uniform_robustness = device->buffer_robustness;
      enum radv_buffer_robustness vertex_robustness = device->buffer_robustness;

      const VkPipelineRobustnessCreateInfoEXT *robust_info =
         stage_robust_info ? stage_robust_info : pipeline_robust_info;

      if (robust_info) {
         storage_robustness = radv_convert_buffer_robustness(device, robust_info->storageBuffers);
         uniform_robustness = radv_convert_buffer_robustness(device, robust_info->uniformBuffers);
         vertex_robustness = radv_convert_buffer_robustness(device, robust_info->vertexInputs);
      }

      if (storage_robustness >= RADV_BUFFER_ROBUSTNESS_2)
         key.stage_info[stage].storage_robustness2 = 1;
      if (uniform_robustness >= RADV_BUFFER_ROBUSTNESS_2)
         key.stage_info[stage].uniform_robustness2 = 1;
      if (stage == MESA_SHADER_VERTEX && vertex_robustness >= RADV_BUFFER_ROBUSTNESS_1)
         key.vertex_robustness1 = 1u;
   }

   for (uint32_t i = 0; i < num_stages; i++) {
      if (stages[i].stage == VK_SHADER_STAGE_MESH_BIT_EXT && device->mesh_fast_launch_2)
         key.mesh_fast_launch_2 = 1u;
   }

   key.keep_statistic_info = radv_pipeline_capture_shader_stats(device, flags);

   return key;
}

void
radv_pipeline_stage_init(const VkPipelineShaderStageCreateInfo *sinfo,
                         const struct radv_pipeline_layout *pipeline_layout, struct radv_shader_stage *out_stage)
{
   const VkShaderModuleCreateInfo *minfo = vk_find_struct_const(sinfo->pNext, SHADER_MODULE_CREATE_INFO);
   const VkPipelineShaderStageModuleIdentifierCreateInfoEXT *iinfo =
      vk_find_struct_const(sinfo->pNext, PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT);

   if (sinfo->module == VK_NULL_HANDLE && !minfo && !iinfo)
      return;

   memset(out_stage, 0, sizeof(*out_stage));

   out_stage->stage = vk_to_mesa_shader_stage(sinfo->stage);
   out_stage->entrypoint = sinfo->pName;
   out_stage->spec_info = sinfo->pSpecializationInfo;
   out_stage->feedback.flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT;

   if (sinfo->module != VK_NULL_HANDLE) {
      struct vk_shader_module *module = vk_shader_module_from_handle(sinfo->module);

      out_stage->spirv.data = module->data;
      out_stage->spirv.size = module->size;
      out_stage->spirv.object = &module->base;

      if (module->nir)
         out_stage->internal_nir = module->nir;
   } else if (minfo) {
      out_stage->spirv.data = (const char *)minfo->pCode;
      out_stage->spirv.size = minfo->codeSize;
   }

   radv_shader_layout_init(pipeline_layout, out_stage->stage, &out_stage->layout);

   vk_pipeline_hash_shader_stage(sinfo, NULL, out_stage->shader_sha1);
}

void
radv_shader_layout_init(const struct radv_pipeline_layout *pipeline_layout, gl_shader_stage stage,
                        struct radv_shader_layout *layout)
{
   layout->num_sets = pipeline_layout->num_sets;
   for (unsigned i = 0; i < pipeline_layout->num_sets; i++) {
      layout->set[i].layout = pipeline_layout->set[i].layout;
      layout->set[i].dynamic_offset_start = pipeline_layout->set[i].dynamic_offset_start;
   }

   layout->push_constant_size = pipeline_layout->push_constant_size;
   layout->use_dynamic_descriptors = pipeline_layout->dynamic_offset_count &&
                                     (pipeline_layout->dynamic_shader_stages & mesa_to_vk_shader_stage(stage));
}

static const struct vk_ycbcr_conversion_state *
ycbcr_conversion_lookup(const void *data, uint32_t set, uint32_t binding, uint32_t array_index)
{
   const struct radv_shader_layout *layout = data;

   const struct radv_descriptor_set_layout *set_layout = layout->set[set].layout;
   const struct vk_ycbcr_conversion_state *ycbcr_samplers = radv_immutable_ycbcr_samplers(set_layout, binding);

   if (!ycbcr_samplers)
      return NULL;

   return ycbcr_samplers + array_index;
}

bool
radv_mem_vectorize_callback(unsigned align_mul, unsigned align_offset, unsigned bit_size, unsigned num_components,
                            nir_intrinsic_instr *low, nir_intrinsic_instr *high, void *data)
{
   if (num_components > 4)
      return false;

   bool is_scratch = false;
   switch (low->intrinsic) {
   case nir_intrinsic_load_stack:
   case nir_intrinsic_load_scratch:
   case nir_intrinsic_store_stack:
   case nir_intrinsic_store_scratch:
      is_scratch = true;
      break;
   default:
      break;
   }

   /* >128 bit loads are split except with SMEM. On GFX6-8, >32 bit scratch loads are split. */
   enum amd_gfx_level gfx_level = *(enum amd_gfx_level *)data;
   if (bit_size * num_components > (is_scratch && gfx_level <= GFX8 ? 32 : 128))
      return false;

   uint32_t align;
   if (align_offset)
      align = 1 << (ffs(align_offset) - 1);
   else
      align = align_mul;

   switch (low->intrinsic) {
   case nir_intrinsic_load_global:
   case nir_intrinsic_store_global:
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_push_constant:
   case nir_intrinsic_load_stack:
   case nir_intrinsic_load_scratch:
   case nir_intrinsic_store_stack:
   case nir_intrinsic_store_scratch: {
      unsigned max_components;
      if (align % 4 == 0)
         max_components = NIR_MAX_VEC_COMPONENTS;
      else if (align % 2 == 0)
         max_components = 16u / bit_size;
      else
         max_components = 8u / bit_size;
      return (align % (bit_size / 8u)) == 0 && num_components <= max_components;
   }
   case nir_intrinsic_load_deref:
   case nir_intrinsic_store_deref:
      assert(nir_deref_mode_is(nir_src_as_deref(low->src[0]), nir_var_mem_shared));
      FALLTHROUGH;
   case nir_intrinsic_load_shared:
   case nir_intrinsic_store_shared:
      if (bit_size * num_components == 96) { /* 96 bit loads require 128 bit alignment and are split otherwise */
         return align % 16 == 0;
      } else if (bit_size == 16 && (align % 4)) {
         /* AMD hardware can't do 2-byte aligned f16vec2 loads, but they are useful for ALU
          * vectorization, because our vectorizer requires the scalar IR to already contain vectors.
          */
         return (align % 2 == 0) && num_components <= 2;
      } else {
         if (num_components == 3) {
            /* AMD hardware can't do 3-component loads except for 96-bit loads, handled above. */
            return false;
         }
         unsigned req = bit_size * num_components;
         if (req == 64 || req == 128) /* 64-bit and 128-bit loads can use ds_read2_b{32,64} */
            req /= 2u;
         return align % (req / 8u) == 0;
      }
   default:
      return false;
   }
   return false;
}

static unsigned
lower_bit_size_callback(const nir_instr *instr, void *_)
{
   struct radv_device *device = _;
   enum amd_gfx_level chip = device->physical_device->rad_info.gfx_level;

   if (instr->type != nir_instr_type_alu)
      return 0;
   nir_alu_instr *alu = nir_instr_as_alu(instr);

   /* If an instruction is not scalarized by this point,
    * it can be emitted as packed instruction */
   if (alu->def.num_components > 1)
      return 0;

   if (alu->def.bit_size & (8 | 16)) {
      unsigned bit_size = alu->def.bit_size;
      switch (alu->op) {
      case nir_op_bitfield_select:
      case nir_op_imul_high:
      case nir_op_umul_high:
      case nir_op_uadd_carry:
      case nir_op_usub_borrow:
         return 32;
      case nir_op_iabs:
      case nir_op_imax:
      case nir_op_umax:
      case nir_op_imin:
      case nir_op_umin:
      case nir_op_ishr:
      case nir_op_ushr:
      case nir_op_ishl:
      case nir_op_isign:
      case nir_op_uadd_sat:
      case nir_op_usub_sat:
         return (bit_size == 8 || !(chip >= GFX8 && alu->def.divergent)) ? 32 : 0;
      case nir_op_iadd_sat:
      case nir_op_isub_sat:
         return bit_size == 8 || !alu->def.divergent ? 32 : 0;

      default:
         return 0;
      }
   }

   if (nir_src_bit_size(alu->src[0].src) & (8 | 16)) {
      unsigned bit_size = nir_src_bit_size(alu->src[0].src);
      switch (alu->op) {
      case nir_op_bit_count:
      case nir_op_find_lsb:
      case nir_op_ufind_msb:
         return 32;
      case nir_op_ilt:
      case nir_op_ige:
      case nir_op_ieq:
      case nir_op_ine:
      case nir_op_ult:
      case nir_op_uge:
      case nir_op_bitz:
      case nir_op_bitnz:
         return (bit_size == 8 || !(chip >= GFX8 && alu->def.divergent)) ? 32 : 0;
      default:
         return 0;
      }
   }

   return 0;
}

static uint8_t
opt_vectorize_callback(const nir_instr *instr, const void *_)
{
   if (instr->type != nir_instr_type_alu)
      return 0;

   const struct radv_device *device = _;
   enum amd_gfx_level chip = device->physical_device->rad_info.gfx_level;
   if (chip < GFX9)
      return 1;

   const nir_alu_instr *alu = nir_instr_as_alu(instr);
   const unsigned bit_size = alu->def.bit_size;
   if (bit_size != 16)
      return 1;

   return aco_nir_op_supports_packed_math_16bit(alu) ? 2 : 1;
}

static nir_component_mask_t
non_uniform_access_callback(const nir_src *src, void *_)
{
   if (src->ssa->num_components == 1)
      return 0x1;
   return nir_chase_binding(*src).success ? 0x2 : 0x3;
}

void
radv_postprocess_nir(struct radv_device *device, const struct radv_pipeline_key *pipeline_key,
                     struct radv_shader_stage *stage)
{
   enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;
   bool progress;

   /* Wave and workgroup size should already be filled. */
   assert(stage->info.wave_size && stage->info.workgroup_size);

   if (stage->stage == MESA_SHADER_FRAGMENT) {
      if (!pipeline_key->optimisations_disabled) {
         NIR_PASS(_, stage->nir, nir_opt_cse);
      }
      NIR_PASS(_, stage->nir, radv_nir_lower_fs_intrinsics, stage, pipeline_key);
   }

   enum nir_lower_non_uniform_access_type lower_non_uniform_access_types =
      nir_lower_non_uniform_ubo_access | nir_lower_non_uniform_ssbo_access | nir_lower_non_uniform_texture_access |
      nir_lower_non_uniform_image_access;

   /* In practice, most shaders do not have non-uniform-qualified
    * accesses (see
    * https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/17558#note_1475069)
    * thus a cheaper and likely to fail check is run first.
    */
   if (nir_has_non_uniform_access(stage->nir, lower_non_uniform_access_types)) {
      if (!pipeline_key->optimisations_disabled) {
         NIR_PASS(_, stage->nir, nir_opt_non_uniform_access);
      }

      if (!radv_use_llvm_for_stage(device, stage->stage)) {
         nir_lower_non_uniform_access_options options = {
            .types = lower_non_uniform_access_types,
            .callback = &non_uniform_access_callback,
            .callback_data = NULL,
         };
         NIR_PASS(_, stage->nir, nir_lower_non_uniform_access, &options);
      }
   }
   NIR_PASS(_, stage->nir, nir_lower_memory_model);

   nir_load_store_vectorize_options vectorize_opts = {
      .modes = nir_var_mem_ssbo | nir_var_mem_ubo | nir_var_mem_push_const | nir_var_mem_shared | nir_var_mem_global |
               nir_var_shader_temp,
      .callback = radv_mem_vectorize_callback,
      .cb_data = &gfx_level,
      .robust_modes = 0,
      /* On GFX6, read2/write2 is out-of-bounds if the offset register is negative, even if
       * the final offset is not.
       */
      .has_shared2_amd = gfx_level >= GFX7,
   };

   if (pipeline_key->stage_info[stage->stage].uniform_robustness2)
      vectorize_opts.robust_modes |= nir_var_mem_ubo;

   if (pipeline_key->stage_info[stage->stage].storage_robustness2)
      vectorize_opts.robust_modes |= nir_var_mem_ssbo;

   if (!pipeline_key->optimisations_disabled) {
      progress = false;
      NIR_PASS(progress, stage->nir, nir_opt_load_store_vectorize, &vectorize_opts);
      if (progress) {
         NIR_PASS(_, stage->nir, nir_copy_prop);
         NIR_PASS(_, stage->nir, nir_opt_shrink_stores, !device->instance->drirc.disable_shrink_image_store);

         /* Gather info again, to update whether 8/16-bit are used. */
         nir_shader_gather_info(stage->nir, nir_shader_get_entrypoint(stage->nir));
      }
   }

   NIR_PASS(_, stage->nir, ac_nir_lower_subdword_loads,
            (ac_nir_lower_subdword_options){.modes_1_comp = nir_var_mem_ubo,
                                            .modes_N_comps = nir_var_mem_ubo | nir_var_mem_ssbo});

   progress = false;
   NIR_PASS(progress, stage->nir, nir_vk_lower_ycbcr_tex, ycbcr_conversion_lookup, &stage->layout);
   /* Gather info in the case that nir_vk_lower_ycbcr_tex might have emitted resinfo instructions. */
   if (progress)
      nir_shader_gather_info(stage->nir, nir_shader_get_entrypoint(stage->nir));

   bool fix_derivs_in_divergent_cf =
      stage->stage == MESA_SHADER_FRAGMENT && !radv_use_llvm_for_stage(device, stage->stage);
   if (fix_derivs_in_divergent_cf) {
      NIR_PASS(_, stage->nir, nir_convert_to_lcssa, true, true);
      nir_divergence_analysis(stage->nir);
   }
   NIR_PASS(_, stage->nir, ac_nir_lower_tex,
            &(ac_nir_lower_tex_options){
               .gfx_level = gfx_level,
               .lower_array_layer_round_even =
                  !device->physical_device->rad_info.conformant_trunc_coord || device->disable_trunc_coord,
               .fix_derivs_in_divergent_cf = fix_derivs_in_divergent_cf,
               .max_wqm_vgprs = 64, // TODO: improve spiller and RA support for linear VGPRs
            });
   if (fix_derivs_in_divergent_cf)
      NIR_PASS(_, stage->nir, nir_opt_remove_phis); /* cleanup LCSSA phis */

   if (stage->nir->info.uses_resource_info_query)
      NIR_PASS(_, stage->nir, ac_nir_lower_resinfo, gfx_level);

   NIR_PASS_V(stage->nir, radv_nir_apply_pipeline_layout, device, &stage->info, &stage->args, &stage->layout);

   if (!pipeline_key->optimisations_disabled) {
      NIR_PASS(_, stage->nir, nir_opt_shrink_vectors);
   }

   NIR_PASS(_, stage->nir, nir_lower_alu_width, opt_vectorize_callback, device);

   nir_move_options sink_opts = nir_move_const_undef | nir_move_copies;

   if (!pipeline_key->optimisations_disabled) {
      if (stage->stage != MESA_SHADER_FRAGMENT || !device->cache_key.disable_sinking_load_input_fs)
         sink_opts |= nir_move_load_input;

      NIR_PASS(_, stage->nir, nir_opt_sink, sink_opts);
      NIR_PASS(_, stage->nir, nir_opt_move, nir_move_load_input | nir_move_const_undef | nir_move_copies);
   }

   /* Lower VS inputs. We need to do this after nir_opt_sink, because
    * load_input can be reordered, but buffer loads can't.
    */
   if (stage->stage == MESA_SHADER_VERTEX) {
      NIR_PASS(_, stage->nir, radv_nir_lower_vs_inputs, stage, pipeline_key, &device->physical_device->rad_info);
   }

   /* Lower I/O intrinsics to memory instructions. */
   bool is_last_vgt_stage = radv_is_last_vgt_stage(stage);
   bool io_to_mem = radv_nir_lower_io_to_mem(device, stage);
   bool lowered_ngg = stage->info.is_ngg && is_last_vgt_stage;
   if (lowered_ngg) {
      radv_lower_ngg(device, stage, pipeline_key);
   } else if (is_last_vgt_stage) {
      if (stage->stage != MESA_SHADER_GEOMETRY) {
         NIR_PASS_V(stage->nir, ac_nir_lower_legacy_vs, gfx_level,
                    stage->info.outinfo.clip_dist_mask | stage->info.outinfo.cull_dist_mask,
                    stage->info.outinfo.vs_output_param_offset, stage->info.outinfo.param_exports,
                    stage->info.outinfo.export_prim_id, false, false, false, stage->info.force_vrs_per_vertex);

      } else {
         bool emulate_ngg_gs_query_pipeline_stat = device->physical_device->emulate_ngg_gs_query_pipeline_stat;

         ac_nir_gs_output_info gs_out_info = {
            .streams = stage->info.gs.output_streams,
            .usage_mask = stage->info.gs.output_usage_mask,
         };
         NIR_PASS_V(stage->nir, ac_nir_lower_legacy_gs, false, emulate_ngg_gs_query_pipeline_stat, &gs_out_info);
      }
   } else if (stage->stage == MESA_SHADER_FRAGMENT) {
      ac_nir_lower_ps_options options = {
         .gfx_level = gfx_level,
         .family = device->physical_device->rad_info.family,
         .use_aco = !radv_use_llvm_for_stage(device, stage->stage),
         .uses_discard = true,
         .alpha_func = COMPARE_FUNC_ALWAYS,
         .no_color_export = stage->info.has_epilog,
         .no_depth_export = stage->info.ps.exports_mrtz_via_epilog,

         .bc_optimize_for_persp = G_0286CC_PERSP_CENTER_ENA(stage->info.ps.spi_ps_input) &&
                                  G_0286CC_PERSP_CENTROID_ENA(stage->info.ps.spi_ps_input),
         .bc_optimize_for_linear = G_0286CC_LINEAR_CENTER_ENA(stage->info.ps.spi_ps_input) &&
                                   G_0286CC_LINEAR_CENTROID_ENA(stage->info.ps.spi_ps_input),
      };

      if (!options.no_color_export) {
         options.dual_src_blend_swizzle = pipeline_key->ps.epilog.mrt0_is_dual_src && gfx_level >= GFX11;
         options.color_is_int8 = pipeline_key->ps.epilog.color_is_int8;
         options.color_is_int10 = pipeline_key->ps.epilog.color_is_int10;
         options.enable_mrt_output_nan_fixup =
            pipeline_key->ps.epilog.enable_mrt_output_nan_fixup && !stage->nir->info.internal;
         /* Need to filter out unwritten color slots. */
         options.spi_shader_col_format = pipeline_key->ps.epilog.spi_shader_col_format & stage->info.ps.colors_written;
      }

      if (!options.no_depth_export) {
         /* Compared to radv_pipeline_key.ps.alpha_to_coverage_via_mrtz,
          * radv_shader_info.ps.writes_mrt0_alpha need any depth/stencil/sample_mask exist.
          * ac_nir_lower_ps() require this field to reflect whether alpha via mrtz is really
          * present.
          */
         options.alpha_to_coverage_via_mrtz = stage->info.ps.writes_mrt0_alpha;
      }

      NIR_PASS_V(stage->nir, ac_nir_lower_ps, &options);
   }

   if (radv_shader_should_clear_lds(device, stage->nir)) {
      const unsigned chunk_size = 16; /* max single store size */
      const unsigned shared_size = ALIGN(stage->nir->info.shared_size, chunk_size);
      NIR_PASS(_, stage->nir, nir_clear_shared_memory, shared_size, chunk_size);
   }

   NIR_PASS(_, stage->nir, nir_lower_int64);

   NIR_PASS(_, stage->nir, nir_opt_idiv_const, 8);

   NIR_PASS(_, stage->nir, nir_lower_idiv,
            &(nir_lower_idiv_options){
               .allow_fp16 = gfx_level >= GFX9,
            });

   if (radv_use_llvm_for_stage(device, stage->stage))
      NIR_PASS_V(stage->nir, nir_lower_io_to_scalar, nir_var_mem_global, NULL, NULL);

   NIR_PASS(_, stage->nir, ac_nir_lower_global_access);
   NIR_PASS_V(stage->nir, ac_nir_lower_intrinsics_to_args, gfx_level, radv_select_hw_stage(&stage->info, gfx_level),
              &stage->args.ac);
   NIR_PASS_V(stage->nir, radv_nir_lower_abi, gfx_level, &stage->info, &stage->args, pipeline_key,
              device->physical_device->rad_info.address32_hi);
   radv_optimize_nir_algebraic(
      stage->nir, io_to_mem || lowered_ngg || stage->stage == MESA_SHADER_COMPUTE || stage->stage == MESA_SHADER_TASK);

   NIR_PASS(_, stage->nir, nir_lower_fp16_casts, nir_lower_fp16_split_fp64);

   if (stage->nir->info.bit_sizes_int & (8 | 16)) {
      if (gfx_level >= GFX8) {
         NIR_PASS(_, stage->nir, nir_convert_to_lcssa, true, true);
         nir_divergence_analysis(stage->nir);
      }

      if (nir_lower_bit_size(stage->nir, lower_bit_size_callback, device)) {
         NIR_PASS(_, stage->nir, nir_opt_constant_folding);
      }

      if (gfx_level >= GFX8)
         NIR_PASS(_, stage->nir, nir_opt_remove_phis); /* cleanup LCSSA phis */
   }
   if (((stage->nir->info.bit_sizes_int | stage->nir->info.bit_sizes_float) & 16) && gfx_level >= GFX9) {
      bool separate_g16 = gfx_level >= GFX10;
      struct nir_fold_tex_srcs_options fold_srcs_options[] = {
         {
            .sampler_dims = ~(BITFIELD_BIT(GLSL_SAMPLER_DIM_CUBE) | BITFIELD_BIT(GLSL_SAMPLER_DIM_BUF)),
            .src_types = (1 << nir_tex_src_coord) | (1 << nir_tex_src_lod) | (1 << nir_tex_src_bias) |
                         (1 << nir_tex_src_min_lod) | (1 << nir_tex_src_ms_index) |
                         (separate_g16 ? 0 : (1 << nir_tex_src_ddx) | (1 << nir_tex_src_ddy)),
         },
         {
            .sampler_dims = ~BITFIELD_BIT(GLSL_SAMPLER_DIM_CUBE),
            .src_types = (1 << nir_tex_src_ddx) | (1 << nir_tex_src_ddy),
         },
      };
      struct nir_fold_16bit_tex_image_options fold_16bit_options = {
         .rounding_mode = nir_rounding_mode_rtz,
         .fold_tex_dest_types = nir_type_float,
         .fold_image_dest_types = nir_type_float,
         .fold_image_store_data = true,
         .fold_image_srcs = !radv_use_llvm_for_stage(device, stage->stage),
         .fold_srcs_options_count = separate_g16 ? 2 : 1,
         .fold_srcs_options = fold_srcs_options,
      };
      NIR_PASS(_, stage->nir, nir_fold_16bit_tex_image, &fold_16bit_options);

      if (!pipeline_key->optimisations_disabled) {
         NIR_PASS(_, stage->nir, nir_opt_vectorize, opt_vectorize_callback, device);
      }
   }

   /* cleanup passes */
   NIR_PASS(_, stage->nir, nir_lower_alu_width, opt_vectorize_callback, device);
   NIR_PASS(_, stage->nir, nir_lower_load_const_to_scalar);
   NIR_PASS(_, stage->nir, nir_copy_prop);
   NIR_PASS(_, stage->nir, nir_opt_dce);

   if (!pipeline_key->optimisations_disabled) {
      sink_opts |= nir_move_comparisons | nir_move_load_ubo | nir_move_load_ssbo;
      NIR_PASS(_, stage->nir, nir_opt_sink, sink_opts);

      nir_move_options move_opts =
         nir_move_const_undef | nir_move_load_ubo | nir_move_load_input | nir_move_comparisons | nir_move_copies;
      NIR_PASS(_, stage->nir, nir_opt_move, move_opts);
   }
}

bool
radv_shader_should_clear_lds(const struct radv_device *device, const nir_shader *shader)
{
   return (shader->info.stage == MESA_SHADER_COMPUTE || shader->info.stage == MESA_SHADER_MESH ||
           shader->info.stage == MESA_SHADER_TASK) &&
          shader->info.shared_size > 0 && device->instance->drirc.clear_lds;
}

static uint32_t
radv_get_executable_count(struct radv_pipeline *pipeline)
{
   uint32_t ret = 0;

   if (pipeline->type == RADV_PIPELINE_RAY_TRACING) {
      struct radv_ray_tracing_pipeline *rt_pipeline = radv_pipeline_to_ray_tracing(pipeline);
      for (uint32_t i = 0; i < rt_pipeline->stage_count; i++)
         ret += rt_pipeline->stages[i].shader ? 1 : 0;
   }

   for (int i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
      if (!pipeline->shaders[i])
         continue;

      if (i == MESA_SHADER_GEOMETRY && !radv_pipeline_has_ngg(radv_pipeline_to_graphics(pipeline))) {
         ret += 2u;
      } else {
         ret += 1u;
      }
   }
   return ret;
}

static struct radv_shader *
radv_get_shader_from_executable_index(struct radv_pipeline *pipeline, int index, gl_shader_stage *stage)
{
   if (pipeline->type == RADV_PIPELINE_RAY_TRACING) {
      struct radv_ray_tracing_pipeline *rt_pipeline = radv_pipeline_to_ray_tracing(pipeline);
      for (uint32_t i = 0; i < rt_pipeline->stage_count; i++) {
         struct radv_ray_tracing_stage *rt_stage = &rt_pipeline->stages[i];
         if (!rt_stage->shader)
            continue;

         if (!index) {
            *stage = rt_stage->stage;
            return rt_stage->shader;
         }

         index--;
      }
   }

   for (int i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
      if (!pipeline->shaders[i])
         continue;
      if (!index) {
         *stage = i;
         return pipeline->shaders[i];
      }

      --index;

      if (i == MESA_SHADER_GEOMETRY && !radv_pipeline_has_ngg(radv_pipeline_to_graphics(pipeline))) {
         if (!index) {
            *stage = i;
            return pipeline->gs_copy_shader;
         }
         --index;
      }
   }

   *stage = -1;
   return NULL;
}

/* Basically strlcpy (which does not exist on linux) specialized for
 * descriptions. */
static void
desc_copy(char *desc, const char *src)
{
   int len = strlen(src);
   assert(len < VK_MAX_DESCRIPTION_SIZE);
   memcpy(desc, src, len);
   memset(desc + len, 0, VK_MAX_DESCRIPTION_SIZE - len);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_GetPipelineExecutablePropertiesKHR(VkDevice _device, const VkPipelineInfoKHR *pPipelineInfo,
                                        uint32_t *pExecutableCount, VkPipelineExecutablePropertiesKHR *pProperties)
{
   RADV_FROM_HANDLE(radv_pipeline, pipeline, pPipelineInfo->pipeline);
   const uint32_t total_count = radv_get_executable_count(pipeline);

   if (!pProperties) {
      *pExecutableCount = total_count;
      return VK_SUCCESS;
   }

   const uint32_t count = MIN2(total_count, *pExecutableCount);
   for (uint32_t executable_idx = 0; executable_idx < count; executable_idx++) {
      gl_shader_stage stage;
      struct radv_shader *shader = radv_get_shader_from_executable_index(pipeline, executable_idx, &stage);

      pProperties[executable_idx].stages = mesa_to_vk_shader_stage(stage);

      const char *name = _mesa_shader_stage_to_string(stage);
      const char *description = NULL;
      switch (stage) {
      case MESA_SHADER_VERTEX:
         description = "Vulkan Vertex Shader";
         break;
      case MESA_SHADER_TESS_CTRL:
         if (!pipeline->shaders[MESA_SHADER_VERTEX]) {
            pProperties[executable_idx].stages |= VK_SHADER_STAGE_VERTEX_BIT;
            name = "vertex + tessellation control";
            description = "Combined Vulkan Vertex and Tessellation Control Shaders";
         } else {
            description = "Vulkan Tessellation Control Shader";
         }
         break;
      case MESA_SHADER_TESS_EVAL:
         description = "Vulkan Tessellation Evaluation Shader";
         break;
      case MESA_SHADER_GEOMETRY:
         if (shader->info.type == RADV_SHADER_TYPE_GS_COPY) {
            name = "geometry copy";
            description = "Extra shader stage that loads the GS output ringbuffer into the rasterizer";
            break;
         }

         if (pipeline->shaders[MESA_SHADER_TESS_CTRL] && !pipeline->shaders[MESA_SHADER_TESS_EVAL]) {
            pProperties[executable_idx].stages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            name = "tessellation evaluation + geometry";
            description = "Combined Vulkan Tessellation Evaluation and Geometry Shaders";
         } else if (!pipeline->shaders[MESA_SHADER_TESS_CTRL] && !pipeline->shaders[MESA_SHADER_VERTEX]) {
            pProperties[executable_idx].stages |= VK_SHADER_STAGE_VERTEX_BIT;
            name = "vertex + geometry";
            description = "Combined Vulkan Vertex and Geometry Shaders";
         } else {
            description = "Vulkan Geometry Shader";
         }
         break;
      case MESA_SHADER_FRAGMENT:
         description = "Vulkan Fragment Shader";
         break;
      case MESA_SHADER_COMPUTE:
         description = "Vulkan Compute Shader";
         break;
      case MESA_SHADER_MESH:
         description = "Vulkan Mesh Shader";
         break;
      case MESA_SHADER_TASK:
         description = "Vulkan Task Shader";
         break;
      case MESA_SHADER_RAYGEN:
         description = "Vulkan Ray Generation Shader";
         break;
      case MESA_SHADER_ANY_HIT:
         description = "Vulkan Any-Hit Shader";
         break;
      case MESA_SHADER_CLOSEST_HIT:
         description = "Vulkan Closest-Hit Shader";
         break;
      case MESA_SHADER_MISS:
         description = "Vulkan Miss Shader";
         break;
      case MESA_SHADER_INTERSECTION:
         description = "Shader responsible for traversing the acceleration structure";
         break;
      case MESA_SHADER_CALLABLE:
         description = "Vulkan Callable Shader";
         break;
      default:
         unreachable("Unsupported shader stage");
      }

      pProperties[executable_idx].subgroupSize = shader->info.wave_size;
      desc_copy(pProperties[executable_idx].name, name);
      desc_copy(pProperties[executable_idx].description, description);
   }

   VkResult result = *pExecutableCount < total_count ? VK_INCOMPLETE : VK_SUCCESS;
   *pExecutableCount = count;
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_GetPipelineExecutableStatisticsKHR(VkDevice _device, const VkPipelineExecutableInfoKHR *pExecutableInfo,
                                        uint32_t *pStatisticCount, VkPipelineExecutableStatisticKHR *pStatistics)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_pipeline, pipeline, pExecutableInfo->pipeline);
   gl_shader_stage stage;
   struct radv_shader *shader =
      radv_get_shader_from_executable_index(pipeline, pExecutableInfo->executableIndex, &stage);

   const struct radv_physical_device *pdevice = device->physical_device;

   unsigned lds_increment = pdevice->rad_info.gfx_level >= GFX11 && stage == MESA_SHADER_FRAGMENT
                               ? 1024
                               : pdevice->rad_info.lds_encode_granularity;

   VkPipelineExecutableStatisticKHR *s = pStatistics;
   VkPipelineExecutableStatisticKHR *end = s + (pStatistics ? *pStatisticCount : 0);
   VkResult result = VK_SUCCESS;

   if (s < end) {
      desc_copy(s->name, "Driver pipeline hash");
      desc_copy(s->description, "Driver pipeline hash used by RGP");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = pipeline->pipeline_hash;
   }
   ++s;

   if (s < end) {
      desc_copy(s->name, "SGPRs");
      desc_copy(s->description, "Number of SGPR registers allocated per subgroup");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = shader->config.num_sgprs;
   }
   ++s;

   if (s < end) {
      desc_copy(s->name, "VGPRs");
      desc_copy(s->description, "Number of VGPR registers allocated per subgroup");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = shader->config.num_vgprs;
   }
   ++s;

   if (s < end) {
      desc_copy(s->name, "Spilled SGPRs");
      desc_copy(s->description, "Number of SGPR registers spilled per subgroup");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = shader->config.spilled_sgprs;
   }
   ++s;

   if (s < end) {
      desc_copy(s->name, "Spilled VGPRs");
      desc_copy(s->description, "Number of VGPR registers spilled per subgroup");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = shader->config.spilled_vgprs;
   }
   ++s;

   if (s < end) {
      desc_copy(s->name, "Code size");
      desc_copy(s->description, "Code size in bytes");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = shader->exec_size;
   }
   ++s;

   if (s < end) {
      desc_copy(s->name, "LDS size");
      desc_copy(s->description, "LDS size in bytes per workgroup");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = shader->config.lds_size * lds_increment;
   }
   ++s;

   if (s < end) {
      desc_copy(s->name, "Scratch size");
      desc_copy(s->description, "Private memory in bytes per subgroup");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = shader->config.scratch_bytes_per_wave;
   }
   ++s;

   if (s < end) {
      desc_copy(s->name, "Subgroups per SIMD");
      desc_copy(s->description, "The maximum number of subgroups in flight on a SIMD unit");
      s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      s->value.u64 = shader->max_waves;
   }
   ++s;

   if (shader->statistics) {
      for (unsigned i = 0; i < aco_num_statistics; i++) {
         const struct aco_compiler_statistic_info *info = &aco_statistic_infos[i];
         if (s < end) {
            desc_copy(s->name, info->name);
            desc_copy(s->description, info->desc);
            s->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
            s->value.u64 = shader->statistics[i];
         }
         ++s;
      }
   }

   if (!pStatistics)
      *pStatisticCount = s - pStatistics;
   else if (s > end) {
      *pStatisticCount = end - pStatistics;
      result = VK_INCOMPLETE;
   } else {
      *pStatisticCount = s - pStatistics;
   }

   return result;
}

static VkResult
radv_copy_representation(void *data, size_t *data_size, const char *src)
{
   size_t total_size = strlen(src) + 1;

   if (!data) {
      *data_size = total_size;
      return VK_SUCCESS;
   }

   size_t size = MIN2(total_size, *data_size);

   memcpy(data, src, size);
   if (size)
      *((char *)data + size - 1) = 0;
   return size < total_size ? VK_INCOMPLETE : VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_GetPipelineExecutableInternalRepresentationsKHR(
   VkDevice _device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pInternalRepresentationCount,
   VkPipelineExecutableInternalRepresentationKHR *pInternalRepresentations)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_pipeline, pipeline, pExecutableInfo->pipeline);
   gl_shader_stage stage;
   struct radv_shader *shader =
      radv_get_shader_from_executable_index(pipeline, pExecutableInfo->executableIndex, &stage);

   VkPipelineExecutableInternalRepresentationKHR *p = pInternalRepresentations;
   VkPipelineExecutableInternalRepresentationKHR *end =
      p + (pInternalRepresentations ? *pInternalRepresentationCount : 0);
   VkResult result = VK_SUCCESS;
   /* optimized NIR */
   if (p < end) {
      p->isText = true;
      desc_copy(p->name, "NIR Shader(s)");
      desc_copy(p->description, "The optimized NIR shader(s)");
      if (radv_copy_representation(p->pData, &p->dataSize, shader->nir_string) != VK_SUCCESS)
         result = VK_INCOMPLETE;
   }
   ++p;

   /* backend IR */
   if (p < end) {
      p->isText = true;
      if (radv_use_llvm_for_stage(device, stage)) {
         desc_copy(p->name, "LLVM IR");
         desc_copy(p->description, "The LLVM IR after some optimizations");
      } else {
         desc_copy(p->name, "ACO IR");
         desc_copy(p->description, "The ACO IR after some optimizations");
      }
      if (radv_copy_representation(p->pData, &p->dataSize, shader->ir_string) != VK_SUCCESS)
         result = VK_INCOMPLETE;
   }
   ++p;

   /* Disassembler */
   if (p < end && shader->disasm_string) {
      p->isText = true;
      desc_copy(p->name, "Assembly");
      desc_copy(p->description, "Final Assembly");
      if (radv_copy_representation(p->pData, &p->dataSize, shader->disasm_string) != VK_SUCCESS)
         result = VK_INCOMPLETE;
   }
   ++p;

   if (!pInternalRepresentations)
      *pInternalRepresentationCount = p - pInternalRepresentations;
   else if (p > end) {
      result = VK_INCOMPLETE;
      *pInternalRepresentationCount = end - pInternalRepresentations;
   } else {
      *pInternalRepresentationCount = p - pInternalRepresentations;
   }

   return result;
}

static void
vk_shader_module_finish(void *_module)
{
   struct vk_shader_module *module = _module;
   vk_object_base_finish(&module->base);
}

VkPipelineShaderStageCreateInfo *
radv_copy_shader_stage_create_info(struct radv_device *device, uint32_t stageCount,
                                   const VkPipelineShaderStageCreateInfo *pStages, void *mem_ctx)
{
   VkPipelineShaderStageCreateInfo *new_stages;

   size_t size = sizeof(VkPipelineShaderStageCreateInfo) * stageCount;
   new_stages = ralloc_size(mem_ctx, size);
   if (!new_stages)
      return NULL;

   if (size)
      memcpy(new_stages, pStages, size);

   for (uint32_t i = 0; i < stageCount; i++) {
      RADV_FROM_HANDLE(vk_shader_module, module, new_stages[i].module);

      const VkShaderModuleCreateInfo *minfo = vk_find_struct_const(pStages[i].pNext, SHADER_MODULE_CREATE_INFO);

      if (module) {
         struct vk_shader_module *new_module = ralloc_size(mem_ctx, sizeof(struct vk_shader_module) + module->size);
         if (!new_module)
            return NULL;

         ralloc_set_destructor(new_module, vk_shader_module_finish);
         vk_object_base_init(&device->vk, &new_module->base, VK_OBJECT_TYPE_SHADER_MODULE);

         new_module->nir = NULL;
         memcpy(new_module->hash, module->hash, sizeof(module->hash));
         new_module->size = module->size;
         memcpy(new_module->data, module->data, module->size);

         module = new_module;
      } else if (minfo) {
         module = ralloc_size(mem_ctx, sizeof(struct vk_shader_module) + minfo->codeSize);
         if (!module)
            return NULL;

         vk_shader_module_init(&device->vk, module, minfo);
      }

      if (module) {
         const VkSpecializationInfo *spec = new_stages[i].pSpecializationInfo;
         if (spec) {
            VkSpecializationInfo *new_spec = ralloc(mem_ctx, VkSpecializationInfo);
            if (!new_spec)
               return NULL;

            new_spec->mapEntryCount = spec->mapEntryCount;
            uint32_t map_entries_size = sizeof(VkSpecializationMapEntry) * spec->mapEntryCount;
            new_spec->pMapEntries = ralloc_size(mem_ctx, map_entries_size);
            if (!new_spec->pMapEntries)
               return NULL;
            memcpy((void *)new_spec->pMapEntries, spec->pMapEntries, map_entries_size);

            new_spec->dataSize = spec->dataSize;
            new_spec->pData = ralloc_size(mem_ctx, spec->dataSize);
            if (!new_spec->pData)
               return NULL;
            memcpy((void *)new_spec->pData, spec->pData, spec->dataSize);

            new_stages[i].pSpecializationInfo = new_spec;
         }

         new_stages[i].module = vk_shader_module_to_handle(module);
         new_stages[i].pName = ralloc_strdup(mem_ctx, new_stages[i].pName);
         if (!new_stages[i].pName)
            return NULL;
         new_stages[i].pNext = NULL;
      }
   }

   return new_stages;
}
