/*
 * Copyright © 2022 Intel Corporation
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

#include "anv_private.h"

#include "compiler/brw_compiler.h"
#include "compiler/brw_nir.h"
#include "compiler/spirv/nir_spirv.h"
#include "dev/intel_debug.h"
#include "util/macros.h"

#include "vk_nir.h"

#include "anv_internal_kernels.h"

#include "shaders/generated_draws_spv.h"
#include "shaders/query_copy_compute_spv.h"
#include "shaders/query_copy_fragment_spv.h"
#include "shaders/memcpy_compute_spv.h"

static bool
lower_vulkan_descriptors_instr(nir_builder *b, nir_intrinsic_instr *intrin,
                               void *cb_data)
{
   if (intrin->intrinsic != nir_intrinsic_load_vulkan_descriptor)
      return false;

   nir_instr *res_index_instr = intrin->src[0].ssa->parent_instr;
   assert(res_index_instr->type == nir_instr_type_intrinsic);
   nir_intrinsic_instr *res_index_intrin =
      nir_instr_as_intrinsic(res_index_instr);
   assert(res_index_intrin->intrinsic == nir_intrinsic_vulkan_resource_index);

   b->cursor = nir_after_instr(&intrin->instr);

   const struct anv_internal_kernel_bind_map *bind_map = cb_data;
   uint32_t binding = nir_intrinsic_binding(res_index_intrin);
   assert(binding < bind_map->num_bindings);

   nir_def *desc_value = NULL;
   if (bind_map->bindings[binding].push_constant) {
      desc_value =
         nir_vec2(b,
                  nir_imm_int(b, binding),
                  nir_imm_int(b, 0));
   } else {
      int push_constant_binding = -1;
      for (uint32_t i = 0; i < bind_map->num_bindings; i++) {
         if (bind_map->bindings[i].push_constant) {
            push_constant_binding = i;
            break;
         }
      }
      assert(push_constant_binding != -1);

      desc_value =
         nir_load_ubo(b, 1, 64,
                      nir_imm_int(b, push_constant_binding),
                      nir_imm_int(b,
                                  bind_map->bindings[binding].address_offset),
                      .align_mul = 8,
                      .align_offset = 0,
                      .range_base = 0,
                      .range = ~0);
      desc_value =
         nir_vec4(b,
                  nir_unpack_64_2x32_split_x(b, desc_value),
                  nir_unpack_64_2x32_split_y(b, desc_value),
                  nir_imm_int(b, 0),
                  nir_imm_int(b, 0));
   }

   nir_def_rewrite_uses(&intrin->def, desc_value);

   return true;
}

static bool
lower_vulkan_descriptors(nir_shader *shader,
                         const struct anv_internal_kernel_bind_map *bind_map)
{
   return nir_shader_intrinsics_pass(shader, lower_vulkan_descriptors_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       (void *)bind_map);
}

static bool
lower_base_workgroup_id(nir_builder *b, nir_intrinsic_instr *intrin,
                        UNUSED void *data)
{
   if (intrin->intrinsic != nir_intrinsic_load_base_workgroup_id)
      return false;

   b->cursor = nir_instr_remove(&intrin->instr);
   nir_def_rewrite_uses(&intrin->def, nir_imm_zero(b, 3, 32));
   return true;
}

static bool
lower_load_ubo_to_uniforms(nir_builder *b, nir_intrinsic_instr *intrin,
                           void *cb_data)
{
   if (intrin->intrinsic != nir_intrinsic_load_ubo)
      return false;

   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def_rewrite_uses(
      &intrin->def,
      nir_load_uniform(b,
                       intrin->def.num_components,
                       intrin->def.bit_size,
                       intrin->src[1].ssa,
                       .base = 0,
                       .range = intrin->def.num_components *
                                intrin->def.bit_size / 8));

   return true;
}

static struct anv_shader_bin *
compile_upload_spirv(struct anv_device *device,
                     gl_shader_stage stage,
                     const char *name,
                     const void *hash_key,
                     uint32_t hash_key_size,
                     const struct anv_internal_kernel_bind_map *bind_map,
                     const uint32_t *spirv_source,
                     uint32_t spirv_source_size,
                     uint32_t sends_count_expectation)
{
   struct spirv_to_nir_options spirv_options = {
      .caps = {
         .int64 = true,
      },
      .ubo_addr_format = nir_address_format_32bit_index_offset,
      .ssbo_addr_format = nir_address_format_64bit_global_32bit_offset,
      .environment = NIR_SPIRV_VULKAN,
      .create_library = false,
   };
   const nir_shader_compiler_options *nir_options =
      device->physical->compiler->nir_options[stage];

   nir_shader* nir =
      vk_spirv_to_nir(&device->vk, spirv_source, spirv_source_size * 4,
                      stage, "main", 0, NULL, &spirv_options,
                      nir_options, true /* internal */,
                      NULL);

   assert(nir != NULL);

   nir->info.name = ralloc_strdup(nir, name);

   NIR_PASS_V(nir, nir_lower_vars_to_ssa);
   NIR_PASS_V(nir, nir_opt_cse);
   NIR_PASS_V(nir, nir_opt_gcm, true);
   NIR_PASS_V(nir, nir_opt_peephole_select, 1, false, false);

   NIR_PASS_V(nir, nir_lower_variable_initializers, ~0);

   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_split_per_member_structs);

   struct brw_compiler *compiler = device->physical->compiler;
   struct brw_nir_compiler_opts opts = {};
   brw_preprocess_nir(compiler, nir, &opts);

   NIR_PASS_V(nir, nir_propagate_invariant, false);

   if (stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(nir, nir_lower_input_attachments,
                 &(nir_input_attachment_options) {
                    .use_fragcoord_sysval = true,
                    .use_layer_id_sysval = true,
                 });
   } else {
      nir_lower_compute_system_values_options options = {
         .has_base_workgroup_id = true,
         .lower_cs_local_id_to_index = true,
         .lower_workgroup_id_to_index = true,
      };
      NIR_PASS_V(nir, nir_lower_compute_system_values, &options);
      NIR_PASS_V(nir, nir_shader_intrinsics_pass, lower_base_workgroup_id,
                 nir_metadata_block_index | nir_metadata_dominance, NULL);
   }

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   /* Do vectorizing here. For some reason when trying to do it in the back
    * this just isn't working.
    */
   nir_load_store_vectorize_options options = {
      .modes = nir_var_mem_ubo | nir_var_mem_ssbo,
      .callback = brw_nir_should_vectorize_mem,
      .robust_modes = (nir_variable_mode)0,
   };
   NIR_PASS_V(nir, nir_opt_load_store_vectorize, &options);

   NIR_PASS_V(nir, lower_vulkan_descriptors, bind_map);

   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_mem_ubo,
              nir_address_format_32bit_index_offset);
   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_mem_ssbo,
              nir_address_format_64bit_global_32bit_offset);

   NIR_PASS_V(nir, nir_copy_prop);
   NIR_PASS_V(nir, nir_opt_constant_folding);
   NIR_PASS_V(nir, nir_opt_dce);

   if (stage == MESA_SHADER_COMPUTE) {
      NIR_PASS_V(nir, nir_shader_intrinsics_pass, lower_load_ubo_to_uniforms,
                 nir_metadata_block_index | nir_metadata_dominance,
                 NULL);
      NIR_PASS_V(nir, brw_nir_lower_cs_intrinsics);
      nir->num_uniforms = bind_map->push_data_size;
   }

   union brw_any_prog_key key;
   memset(&key, 0, sizeof(key));

   union brw_any_prog_data prog_data;
   memset(&prog_data, 0, sizeof(prog_data));
   prog_data.base.nr_params = nir->num_uniforms / 4;

   brw_nir_analyze_ubo_ranges(compiler, nir, prog_data.base.ubo_ranges);

   void *temp_ctx = ralloc_context(NULL);

   const unsigned *program;
   if (stage == MESA_SHADER_FRAGMENT) {
      struct brw_compile_stats stats[3];
      struct brw_compile_fs_params params = {
         .base = {
            .nir = nir,
            .log_data = device,
            .debug_flag = DEBUG_WM,
            .stats = stats,
            .mem_ctx = temp_ctx,
         },
         .key = &key.wm,
         .prog_data = &prog_data.wm,
      };
      program = brw_compile_fs(compiler, &params);

      unsigned stat_idx = 0;
      if (prog_data.wm.dispatch_8) {
         assert(stats[stat_idx].spills == 0);
         assert(stats[stat_idx].fills == 0);
         assert(stats[stat_idx].sends == sends_count_expectation);
         stat_idx++;
      }
      if (prog_data.wm.dispatch_16) {
         assert(stats[stat_idx].spills == 0);
         assert(stats[stat_idx].fills == 0);
         assert(stats[stat_idx].sends == sends_count_expectation);
         stat_idx++;
      }
      if (prog_data.wm.dispatch_32) {
         assert(stats[stat_idx].spills == 0);
         assert(stats[stat_idx].fills == 0);
         assert(stats[stat_idx].sends == sends_count_expectation * 2);
         stat_idx++;
      }
   } else {
      struct brw_compile_stats stats;
      struct brw_compile_cs_params params = {
         .base = {
            .nir = nir,
            .stats = &stats,
            .log_data = device,
            .debug_flag = DEBUG_CS,
            .mem_ctx = temp_ctx,
         },
         .key = &key.cs,
         .prog_data = &prog_data.cs,
      };
      program = brw_compile_cs(compiler, &params);

      assert(stats.spills == 0);
      assert(stats.fills == 0);
      assert(stats.sends == sends_count_expectation);
   }

   struct anv_pipeline_bind_map dummy_bind_map;
   memset(&dummy_bind_map, 0, sizeof(dummy_bind_map));

   struct anv_push_descriptor_info push_desc_info = {};

   struct anv_shader_bin *kernel =
      anv_device_upload_kernel(device,
                               device->internal_cache,
                               nir->info.stage,
                               hash_key, hash_key_size, program,
                               prog_data.base.program_size,
                               &prog_data.base, sizeof(prog_data),
                               NULL, 0, NULL, &dummy_bind_map,
                               &push_desc_info,
                               0 /* dynamic_push_values */);

   ralloc_free(temp_ctx);
   ralloc_free(nir);

   return kernel;
}

VkResult
anv_device_init_internal_kernels(struct anv_device *device)
{
   const struct intel_l3_weights w =
      intel_get_default_l3_weights(device->info,
                                   true /* wants_dc_cache */,
                                   false /* needs_slm */);
   device->internal_kernels_l3_config = intel_get_l3_config(device->info, w);

   const struct {
      struct {
         char name[40];
      } key;

      gl_shader_stage stage;

      const uint32_t *spirv_data;
      uint32_t        spirv_size;

      uint32_t        send_count;

      struct anv_internal_kernel_bind_map bind_map;
   } internal_kernels[] = {
      [ANV_INTERNAL_KERNEL_GENERATED_DRAWS] = {
         .key        = {
            .name    = "anv-generated-indirect-draws",
         },
         .stage      = MESA_SHADER_FRAGMENT,
         .spirv_data = generated_draws_spv_source,
         .spirv_size = ARRAY_SIZE(generated_draws_spv_source),
         .send_count = /* 2 * (2 loads + 3 stores) +  ** gfx11 **
                        * 2 * (2 loads + 6 stores) +  ** gfx9  **
                        * 1 load + 3 store
                        */ 29,
         .bind_map   = {
            .num_bindings = 5,
            .bindings     = {
               {
                  .address_offset = offsetof(struct anv_generated_indirect_params,
                                             indirect_data_addr),
               },
               {
                  .address_offset = offsetof(struct anv_generated_indirect_params,
                                             generated_cmds_addr),
               },
               {
                  .address_offset = offsetof(struct anv_generated_indirect_params,
                                             draw_ids_addr),
               },
               {
                  .address_offset = offsetof(struct anv_generated_indirect_params,
                                             draw_count_addr),
               },
               {
                  .push_constant = true,
               },
            },
            .push_data_size = sizeof(struct anv_generated_indirect_params),
         },
      },
      [ANV_INTERNAL_KERNEL_COPY_QUERY_RESULTS_COMPUTE] = {
         .key        = {
            .name    = "anv-copy-query-compute",
         },
         .stage      = MESA_SHADER_COMPUTE,
         .spirv_data = query_copy_compute_spv_source,
         .spirv_size = ARRAY_SIZE(query_copy_compute_spv_source),
         .send_count = device->info->verx10 >= 125 ?
                       9 /* 4 loads + 4 stores + 1 EOT */ :
                       8 /* 3 loads + 4 stores + 1 EOT */,
         .bind_map   = {
            .num_bindings = 3,
            .bindings     = {
               {
                  .address_offset = offsetof(struct anv_query_copy_params,
                                             query_data_addr),
               },
               {
                  .address_offset = offsetof(struct anv_query_copy_params,
                                             destination_addr),
               },
               {
                  .push_constant = true,
               },
            },
            .push_data_size = sizeof(struct anv_query_copy_params),
         },
      },
      [ANV_INTERNAL_KERNEL_COPY_QUERY_RESULTS_FRAGMENT] = {
         .key        = {
            .name    = "anv-copy-query-fragment",
         },
         .stage      = MESA_SHADER_FRAGMENT,
         .spirv_data = query_copy_fragment_spv_source,
         .spirv_size = ARRAY_SIZE(query_copy_fragment_spv_source),
         .send_count = 8 /* 3 loads + 4 stores + 1 EOT */,
         .bind_map   = {
            .num_bindings = 3,
            .bindings     = {
               {
                  .address_offset = offsetof(struct anv_query_copy_params,
                                             query_data_addr),
               },
               {
                  .address_offset = offsetof(struct anv_query_copy_params,
                                             destination_addr),
               },
               {
                  .push_constant = true,
               },
            },
            .push_data_size = sizeof(struct anv_query_copy_params),
         },
      },
      [ANV_INTERNAL_KERNEL_MEMCPY_COMPUTE] = {
         .key        = {
            .name    = "anv-memcpy-compute",
         },
         .stage      = MESA_SHADER_COMPUTE,
         .spirv_data = memcpy_compute_spv_source,
         .spirv_size = ARRAY_SIZE(memcpy_compute_spv_source),
         .send_count = device->info->verx10 >= 125 ?
                       10 /* 5 loads (1 pull constants) + 4 stores + 1 EOT */ :
                       9 /* 4 loads + 4 stores + 1 EOT */,
         .bind_map   = {
            .num_bindings = 3,
            .bindings     = {
               {
                  .address_offset = offsetof(struct anv_memcpy_params,
                                             src_addr),
               },
               {
                  .address_offset = offsetof(struct anv_memcpy_params,
                                             dst_addr),
               },
               {
                  .push_constant = true,
               },
            },
            .push_data_size = sizeof(struct anv_memcpy_params),
         },
      },
   };

   for (uint32_t i = 0; i < ARRAY_SIZE(internal_kernels); i++) {
      device->internal_kernels[i] =
         anv_device_search_for_kernel(device,
                                      device->internal_cache,
                                      &internal_kernels[i].key,
                                      sizeof(internal_kernels[i].key),
                                      NULL);
      if (device->internal_kernels[i] == NULL) {
         device->internal_kernels[i] =
            compile_upload_spirv(device,
                                 internal_kernels[i].stage,
                                 internal_kernels[i].key.name,
                                 &internal_kernels[i].key,
                                 sizeof(internal_kernels[i].key),
                                 &internal_kernels[i].bind_map,
                                 internal_kernels[i].spirv_data,
                                 internal_kernels[i].spirv_size,
                                 internal_kernels[i].send_count);
      }
      if (device->internal_kernels[i] == NULL)
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

      /* The cache already has a reference and it's not going anywhere so
       * there is no need to hold a second reference.
       */
      anv_shader_bin_unref(device, device->internal_kernels[i]);
   }

   return VK_SUCCESS;
}

void
anv_device_finish_internal_kernels(struct anv_device *device)
{
}
