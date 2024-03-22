/*
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

#include "util/blob.h"
#include "util/hash_table.h"
#include "util/u_debug.h"
#include "util/disk_cache.h"
#include "util/mesa-sha1.h"
#include "nir/nir_serialize.h"
#include "anv_private.h"
#include "nir/nir_xfb_info.h"
#include "vulkan/util/vk_util.h"
#include "compiler/spirv/nir_spirv.h"
#include "shaders/float64_spv.h"

static bool
anv_shader_bin_serialize(struct vk_pipeline_cache_object *object,
                         struct blob *blob);

struct vk_pipeline_cache_object *
anv_shader_bin_deserialize(struct vk_pipeline_cache *cache,
                           const void *key_data, size_t key_size,
                           struct blob_reader *blob);

static void
anv_shader_bin_destroy(struct vk_device *_device,
                       struct vk_pipeline_cache_object *object)
{
   struct anv_device *device =
      container_of(_device, struct anv_device, vk);

   struct anv_shader_bin *shader =
      container_of(object, struct anv_shader_bin, base);

   anv_state_pool_free(&device->instruction_state_pool, shader->kernel);
   vk_pipeline_cache_object_finish(&shader->base);
   vk_free(&device->vk.alloc, shader);
}

static const struct vk_pipeline_cache_object_ops anv_shader_bin_ops = {
   .serialize = anv_shader_bin_serialize,
   .deserialize = anv_shader_bin_deserialize,
   .destroy = anv_shader_bin_destroy,
};

const struct vk_pipeline_cache_object_ops *const anv_cache_import_ops[2] = {
   &anv_shader_bin_ops,
   NULL
};

struct anv_shader_bin *
anv_shader_bin_create(struct anv_device *device,
                      gl_shader_stage stage,
                      const void *key_data, uint32_t key_size,
                      const void *kernel_data, uint32_t kernel_size,
                      const struct brw_stage_prog_data *prog_data_in,
                      uint32_t prog_data_size,
                      const struct brw_compile_stats *stats, uint32_t num_stats,
                      const nir_xfb_info *xfb_info_in,
                      const struct anv_pipeline_bind_map *bind_map,
                      const struct anv_push_descriptor_info *push_desc_info,
                      enum anv_dynamic_push_bits dynamic_push_values)
{
   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct anv_shader_bin, shader, 1);
   VK_MULTIALLOC_DECL_SIZE(&ma, void, obj_key_data, key_size);
   VK_MULTIALLOC_DECL_SIZE(&ma, struct brw_stage_prog_data, prog_data,
                                prog_data_size);
   VK_MULTIALLOC_DECL(&ma, struct brw_shader_reloc, prog_data_relocs,
                           prog_data_in->num_relocs);
   VK_MULTIALLOC_DECL(&ma, uint32_t, prog_data_param, prog_data_in->nr_params);

   VK_MULTIALLOC_DECL_SIZE(&ma, nir_xfb_info, xfb_info,
                                xfb_info_in == NULL ? 0 :
                                nir_xfb_info_size(xfb_info_in->output_count));

   VK_MULTIALLOC_DECL(&ma, struct anv_pipeline_binding, surface_to_descriptor,
                           bind_map->surface_count);
   VK_MULTIALLOC_DECL(&ma, struct anv_pipeline_binding, sampler_to_descriptor,
                      bind_map->sampler_count);
   VK_MULTIALLOC_DECL(&ma, struct brw_kernel_arg_desc, kernel_args,
                      bind_map->kernel_arg_count);

   if (!vk_multialloc_alloc(&ma, &device->vk.alloc,
                            VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
      return NULL;

   memcpy(obj_key_data, key_data, key_size);
   vk_pipeline_cache_object_init(&device->vk, &shader->base,
                                 &anv_shader_bin_ops, obj_key_data, key_size);

   shader->stage = stage;

   shader->kernel =
      anv_state_pool_alloc(&device->instruction_state_pool, kernel_size, 64);
   memcpy(shader->kernel.map, kernel_data, kernel_size);
   shader->kernel_size = kernel_size;

   uint64_t shader_data_addr =
      device->physical->va.instruction_state_pool.addr +
      shader->kernel.offset +
      prog_data_in->const_data_offset;

   int rv_count = 0;
   struct brw_shader_reloc_value reloc_values[6];
   assert((device->physical->va.indirect_descriptor_pool.addr & 0xffffffff) == 0);
   assert((device->physical->va.internal_surface_state_pool.addr & 0xffffffff) == 0);
   reloc_values[rv_count++] = (struct brw_shader_reloc_value) {
      .id = BRW_SHADER_RELOC_DESCRIPTORS_ADDR_HIGH,
      .value = device->physical->indirect_descriptors ?
               (device->physical->va.indirect_descriptor_pool.addr >> 32) :
               (device->physical->va.internal_surface_state_pool.addr >> 32),
   };
   assert((device->physical->va.instruction_state_pool.addr & 0xffffffff) == 0);
   reloc_values[rv_count++] = (struct brw_shader_reloc_value) {
      .id = BRW_SHADER_RELOC_CONST_DATA_ADDR_LOW,
      .value = shader_data_addr,
   };
   assert(shader_data_addr >> 32 == device->physical->va.instruction_state_pool.addr >> 32);
   reloc_values[rv_count++] = (struct brw_shader_reloc_value) {
      .id = BRW_SHADER_RELOC_CONST_DATA_ADDR_HIGH,
      .value = device->physical->va.instruction_state_pool.addr >> 32,
   };
   reloc_values[rv_count++] = (struct brw_shader_reloc_value) {
      .id = BRW_SHADER_RELOC_SHADER_START_OFFSET,
      .value = shader->kernel.offset,
   };
   if (brw_shader_stage_is_bindless(stage)) {
      const struct brw_bs_prog_data *bs_prog_data =
         brw_bs_prog_data_const(prog_data_in);
      uint64_t resume_sbt_addr =
         device->physical->va.instruction_state_pool.addr +
         shader->kernel.offset +
         bs_prog_data->resume_sbt_offset;
      reloc_values[rv_count++] = (struct brw_shader_reloc_value) {
         .id = BRW_SHADER_RELOC_RESUME_SBT_ADDR_LOW,
         .value = resume_sbt_addr,
      };
      reloc_values[rv_count++] = (struct brw_shader_reloc_value) {
         .id = BRW_SHADER_RELOC_RESUME_SBT_ADDR_HIGH,
         .value = resume_sbt_addr >> 32,
      };
   }

   brw_write_shader_relocs(&device->physical->compiler->isa,
                           shader->kernel.map, prog_data_in,
                           reloc_values, rv_count);

   memcpy(prog_data, prog_data_in, prog_data_size);
   typed_memcpy(prog_data_relocs, prog_data_in->relocs,
                prog_data_in->num_relocs);
   prog_data->relocs = prog_data_relocs;
   memset(prog_data_param, 0,
          prog_data->nr_params * sizeof(*prog_data_param));
   prog_data->param = prog_data_param;
   shader->prog_data = prog_data;
   shader->prog_data_size = prog_data_size;

   assert(num_stats <= ARRAY_SIZE(shader->stats));
   typed_memcpy(shader->stats, stats, num_stats);
   shader->num_stats = num_stats;

   if (xfb_info_in) {
      *xfb_info = *xfb_info_in;
      typed_memcpy(xfb_info->outputs, xfb_info_in->outputs,
                   xfb_info_in->output_count);
      shader->xfb_info = xfb_info;
   } else {
      shader->xfb_info = NULL;
   }

   shader->dynamic_push_values = dynamic_push_values;

   typed_memcpy(&shader->push_desc_info, push_desc_info, 1);

   shader->bind_map = *bind_map;
   typed_memcpy(surface_to_descriptor, bind_map->surface_to_descriptor,
                bind_map->surface_count);
   shader->bind_map.surface_to_descriptor = surface_to_descriptor;
   typed_memcpy(sampler_to_descriptor, bind_map->sampler_to_descriptor,
                bind_map->sampler_count);
   shader->bind_map.sampler_to_descriptor = sampler_to_descriptor;
   typed_memcpy(kernel_args, bind_map->kernel_args,
                bind_map->kernel_arg_count);
   shader->bind_map.kernel_args = kernel_args;

   return shader;
}

static bool
anv_shader_bin_serialize(struct vk_pipeline_cache_object *object,
                         struct blob *blob)
{
   struct anv_shader_bin *shader =
      container_of(object, struct anv_shader_bin, base);

   blob_write_uint32(blob, shader->stage);

   blob_write_uint32(blob, shader->kernel_size);
   blob_write_bytes(blob, shader->kernel.map, shader->kernel_size);

   blob_write_uint32(blob, shader->prog_data_size);

   union brw_any_prog_data prog_data;
   assert(shader->prog_data_size <= sizeof(prog_data));
   memcpy(&prog_data, shader->prog_data, shader->prog_data_size);
   prog_data.base.relocs = NULL;
   prog_data.base.param = NULL;
   blob_write_bytes(blob, &prog_data, shader->prog_data_size);

   blob_write_bytes(blob, shader->prog_data->relocs,
                    shader->prog_data->num_relocs *
                    sizeof(shader->prog_data->relocs[0]));

   blob_write_uint32(blob, shader->num_stats);
   blob_write_bytes(blob, shader->stats,
                    shader->num_stats * sizeof(shader->stats[0]));

   if (shader->xfb_info) {
      uint32_t xfb_info_size =
         nir_xfb_info_size(shader->xfb_info->output_count);
      blob_write_uint32(blob, xfb_info_size);
      blob_write_bytes(blob, shader->xfb_info, xfb_info_size);
   } else {
      blob_write_uint32(blob, 0);
   }

   blob_write_uint32(blob, shader->dynamic_push_values);

   blob_write_uint32(blob, shader->push_desc_info.used_descriptors);
   blob_write_uint32(blob, shader->push_desc_info.fully_promoted_ubo_descriptors);
   blob_write_uint8(blob, shader->push_desc_info.used_set_buffer);

   blob_write_bytes(blob, shader->bind_map.surface_sha1,
                    sizeof(shader->bind_map.surface_sha1));
   blob_write_bytes(blob, shader->bind_map.sampler_sha1,
                    sizeof(shader->bind_map.sampler_sha1));
   blob_write_bytes(blob, shader->bind_map.push_sha1,
                    sizeof(shader->bind_map.push_sha1));
   blob_write_uint32(blob, shader->bind_map.surface_count);
   blob_write_uint32(blob, shader->bind_map.sampler_count);
   if (shader->stage == MESA_SHADER_KERNEL) {
      uint32_t packed = (uint32_t)shader->bind_map.kernel_args_size << 16 |
                        (uint32_t)shader->bind_map.kernel_arg_count;
      blob_write_uint32(blob, packed);
   }
   blob_write_bytes(blob, shader->bind_map.surface_to_descriptor,
                    shader->bind_map.surface_count *
                    sizeof(*shader->bind_map.surface_to_descriptor));
   blob_write_bytes(blob, shader->bind_map.sampler_to_descriptor,
                    shader->bind_map.sampler_count *
                    sizeof(*shader->bind_map.sampler_to_descriptor));
   blob_write_bytes(blob, shader->bind_map.kernel_args,
                    shader->bind_map.kernel_arg_count *
                    sizeof(*shader->bind_map.kernel_args));
   blob_write_bytes(blob, shader->bind_map.push_ranges,
                    sizeof(shader->bind_map.push_ranges));

   return !blob->out_of_memory;
}

struct vk_pipeline_cache_object *
anv_shader_bin_deserialize(struct vk_pipeline_cache *cache,
                           const void *key_data, size_t key_size,
                           struct blob_reader *blob)
{
   struct anv_device *device =
      container_of(cache->base.device, struct anv_device, vk);

   gl_shader_stage stage = blob_read_uint32(blob);

   uint32_t kernel_size = blob_read_uint32(blob);
   const void *kernel_data = blob_read_bytes(blob, kernel_size);

   uint32_t prog_data_size = blob_read_uint32(blob);
   const void *prog_data_bytes = blob_read_bytes(blob, prog_data_size);
   if (blob->overrun)
      return NULL;

   union brw_any_prog_data prog_data;
   memcpy(&prog_data, prog_data_bytes,
          MIN2(sizeof(prog_data), prog_data_size));
   prog_data.base.relocs =
      blob_read_bytes(blob, prog_data.base.num_relocs *
                            sizeof(prog_data.base.relocs[0]));

   uint32_t num_stats = blob_read_uint32(blob);
   const struct brw_compile_stats *stats =
      blob_read_bytes(blob, num_stats * sizeof(stats[0]));

   const nir_xfb_info *xfb_info = NULL;
   uint32_t xfb_size = blob_read_uint32(blob);
   if (xfb_size)
      xfb_info = blob_read_bytes(blob, xfb_size);

   enum anv_dynamic_push_bits dynamic_push_values = blob_read_uint32(blob);

   struct anv_push_descriptor_info push_desc_info = {};
   push_desc_info.used_descriptors = blob_read_uint32(blob);
   push_desc_info.fully_promoted_ubo_descriptors = blob_read_uint32(blob);
   push_desc_info.used_set_buffer = blob_read_uint8(blob);

   struct anv_pipeline_bind_map bind_map = {};
   blob_copy_bytes(blob, bind_map.surface_sha1, sizeof(bind_map.surface_sha1));
   blob_copy_bytes(blob, bind_map.sampler_sha1, sizeof(bind_map.sampler_sha1));
   blob_copy_bytes(blob, bind_map.push_sha1, sizeof(bind_map.push_sha1));
   bind_map.surface_count = blob_read_uint32(blob);
   bind_map.sampler_count = blob_read_uint32(blob);
   if (stage == MESA_SHADER_KERNEL) {
      uint32_t packed = blob_read_uint32(blob);
      bind_map.kernel_args_size = (uint16_t)(packed >> 16);
      bind_map.kernel_arg_count = (uint16_t)packed;
   }
   bind_map.surface_to_descriptor = (void *)
      blob_read_bytes(blob, bind_map.surface_count *
                            sizeof(*bind_map.surface_to_descriptor));
   bind_map.sampler_to_descriptor = (void *)
      blob_read_bytes(blob, bind_map.sampler_count *
                            sizeof(*bind_map.sampler_to_descriptor));
   bind_map.kernel_args = (void *)
      blob_read_bytes(blob, bind_map.kernel_arg_count *
                            sizeof(*bind_map.kernel_args));
   blob_copy_bytes(blob, bind_map.push_ranges, sizeof(bind_map.push_ranges));

   if (blob->overrun)
      return NULL;

   struct anv_shader_bin *shader =
      anv_shader_bin_create(device, stage,
                            key_data, key_size,
                            kernel_data, kernel_size,
                            &prog_data.base, prog_data_size,
                            stats, num_stats, xfb_info, &bind_map,
                            &push_desc_info,
                            dynamic_push_values);
   if (shader == NULL)
      return NULL;

   return &shader->base;
}

struct anv_shader_bin *
anv_device_search_for_kernel(struct anv_device *device,
                             struct vk_pipeline_cache *cache,
                             const void *key_data, uint32_t key_size,
                             bool *user_cache_hit)
{
   /* Use the default pipeline cache if none is specified */
   if (cache == NULL)
      cache = device->default_pipeline_cache;

   bool cache_hit = false;
   struct vk_pipeline_cache_object *object =
      vk_pipeline_cache_lookup_object(cache, key_data, key_size,
                                      &anv_shader_bin_ops, &cache_hit);
   if (user_cache_hit != NULL) {
      *user_cache_hit = object != NULL && cache_hit &&
                        cache != device->default_pipeline_cache;
   }
   if (object == NULL)
      return NULL;

   return container_of(object, struct anv_shader_bin, base);
}

struct anv_shader_bin *
anv_device_upload_kernel(struct anv_device *device,
                         struct vk_pipeline_cache *cache,
                         gl_shader_stage stage,
                         const void *key_data, uint32_t key_size,
                         const void *kernel_data, uint32_t kernel_size,
                         const struct brw_stage_prog_data *prog_data,
                         uint32_t prog_data_size,
                         const struct brw_compile_stats *stats,
                         uint32_t num_stats,
                         const nir_xfb_info *xfb_info,
                         const struct anv_pipeline_bind_map *bind_map,
                         const struct anv_push_descriptor_info *push_desc_info,
                         enum anv_dynamic_push_bits dynamic_push_values)
{
   /* Use the default pipeline cache if none is specified */
   if (cache == NULL)
      cache = device->default_pipeline_cache;

   struct anv_shader_bin *shader =
      anv_shader_bin_create(device, stage,
                            key_data, key_size,
                            kernel_data, kernel_size,
                            prog_data, prog_data_size,
                            stats, num_stats,
                            xfb_info, bind_map,
                            push_desc_info,
                            dynamic_push_values);
   if (shader == NULL)
      return NULL;

   struct vk_pipeline_cache_object *cached =
      vk_pipeline_cache_add_object(cache, &shader->base);

   return container_of(cached, struct anv_shader_bin, base);
}

#define SHA1_KEY_SIZE 20

struct nir_shader *
anv_device_search_for_nir(struct anv_device *device,
                          struct vk_pipeline_cache *cache,
                          const nir_shader_compiler_options *nir_options,
                          unsigned char sha1_key[SHA1_KEY_SIZE],
                          void *mem_ctx)
{
   if (cache == NULL)
      cache = device->default_pipeline_cache;

   return vk_pipeline_cache_lookup_nir(cache, sha1_key, SHA1_KEY_SIZE,
                                       nir_options, NULL, mem_ctx);
}

void
anv_device_upload_nir(struct anv_device *device,
                      struct vk_pipeline_cache *cache,
                      const struct nir_shader *nir,
                      unsigned char sha1_key[SHA1_KEY_SIZE])
{
   if (cache == NULL)
      cache = device->default_pipeline_cache;

   vk_pipeline_cache_add_nir(cache, sha1_key, SHA1_KEY_SIZE, nir);
}

void
anv_load_fp64_shader(struct anv_device *device)
{
   const nir_shader_compiler_options *nir_options =
      device->physical->compiler->nir_options[MESA_SHADER_VERTEX];

   const char* shader_name = "float64_spv_lib";
   struct mesa_sha1 sha1_ctx;
   uint8_t sha1[20];
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx, shader_name, strlen(shader_name));
   _mesa_sha1_final(&sha1_ctx, sha1);

   device->fp64_nir =
      anv_device_search_for_nir(device, device->internal_cache,
                                   nir_options, sha1, NULL);

   /* The shader found, no need to call spirv_to_nir() again. */
   if (device->fp64_nir)
      return;

   struct spirv_to_nir_options spirv_options = {
      .caps = {
         .address = true,
         .float64 = true,
         .int8 = true,
         .int16 = true,
         .int64 = true,
      },
      .environment = NIR_SPIRV_VULKAN,
      .create_library = true
   };

   nir_shader* nir =
      spirv_to_nir(float64_spv_source, sizeof(float64_spv_source) / 4,
                   NULL, 0, MESA_SHADER_VERTEX, "main",
                   &spirv_options, nir_options);

   assert(nir != NULL);

   nir_validate_shader(nir, "after spirv_to_nir");
   nir_validate_ssa_dominance(nir, "after spirv_to_nir");

   NIR_PASS_V(nir, nir_lower_variable_initializers, nir_var_function_temp);
   NIR_PASS_V(nir, nir_lower_returns);
   NIR_PASS_V(nir, nir_inline_functions);
   NIR_PASS_V(nir, nir_opt_deref);

   NIR_PASS_V(nir, nir_lower_vars_to_ssa);
   NIR_PASS_V(nir, nir_copy_prop);
   NIR_PASS_V(nir, nir_opt_dce);
   NIR_PASS_V(nir, nir_opt_cse);
   NIR_PASS_V(nir, nir_opt_gcm, true);
   NIR_PASS_V(nir, nir_opt_peephole_select, 1, false, false);
   NIR_PASS_V(nir, nir_opt_dce);

   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_function_temp,
              nir_address_format_62bit_generic);

   anv_device_upload_nir(device, device->internal_cache,
                         nir, sha1);

   device->fp64_nir = nir;
}
