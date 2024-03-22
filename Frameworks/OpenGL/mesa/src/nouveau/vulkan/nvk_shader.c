/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_shader.h"

#include "nvk_descriptor_set_layout.h"
#include "nvk_device.h"
#include "nvk_physical_device.h"
#include "nvk_pipeline.h"
#include "nvk_sampler.h"

#include "vk_nir_convert_ycbcr.h"
#include "vk_pipeline.h"
#include "vk_pipeline_cache.h"
#include "vk_pipeline_layout.h"
#include "vk_shader_module.h"
#include "vk_ycbcr_conversion.h"

#include "nak.h"
#include "nir.h"
#include "nir_builder.h"
#include "compiler/spirv/nir_spirv.h"

#include "nv50_ir_driver.h"

#include "util/mesa-sha1.h"
#include "util/u_debug.h"

#include "cla097.h"
#include "clb097.h"
#include "clc397.h"
#include "clc597.h"

static void
shared_var_info(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   assert(glsl_type_is_vector_or_scalar(type));

   uint32_t comp_size = glsl_type_is_boolean(type) ? 4 : glsl_get_bit_size(type) / 8;
   unsigned length = glsl_get_vector_elements(type);
   *size = comp_size * length, *align = comp_size;
}

VkShaderStageFlags
nvk_nak_stages(const struct nv_device_info *info)
{
   const VkShaderStageFlags all =
      VK_SHADER_STAGE_VERTEX_BIT |
      VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
      VK_SHADER_STAGE_GEOMETRY_BIT |
      VK_SHADER_STAGE_FRAGMENT_BIT |
      VK_SHADER_STAGE_COMPUTE_BIT;

   const struct debug_control flags[] = {
      { "vs", BITFIELD64_BIT(MESA_SHADER_VERTEX) },
      { "tcs", BITFIELD64_BIT(MESA_SHADER_TESS_CTRL) },
      { "tes", BITFIELD64_BIT(MESA_SHADER_TESS_EVAL) },
      { "gs", BITFIELD64_BIT(MESA_SHADER_GEOMETRY) },
      { "fs", BITFIELD64_BIT(MESA_SHADER_FRAGMENT) },
      { "cs", BITFIELD64_BIT(MESA_SHADER_COMPUTE) },
      { "all", all },
      { NULL, 0 },
   };

   const char *env_str = getenv("NVK_USE_NAK");
   if (env_str == NULL)
      return info->cls_eng3d >= VOLTA_A ? all : 0;
   else
      return parse_debug_string(env_str, flags);
}

static bool
use_nak(const struct nvk_physical_device *pdev, gl_shader_stage stage)
{
   return nvk_nak_stages(&pdev->info) & mesa_to_vk_shader_stage(stage);
}

uint64_t
nvk_physical_device_compiler_flags(const struct nvk_physical_device *pdev)
{
   bool no_cbufs = pdev->debug_flags & NVK_DEBUG_NO_CBUF;
   uint64_t prog_debug = nvk_cg_get_prog_debug();
   uint64_t prog_optimize = nvk_cg_get_prog_optimize();
   uint64_t nak_stages = nvk_nak_stages(&pdev->info);
   uint64_t nak_flags = nak_debug_flags(pdev->nak);

   assert(prog_debug <= UINT8_MAX);
   assert(prog_optimize < 16);
   assert(nak_stages <= UINT32_MAX);
   assert(nak_flags <= UINT16_MAX);

   return prog_debug
      | (prog_optimize << 8)
      | ((uint64_t)no_cbufs << 12)
      | (nak_stages << 16)
      | (nak_flags << 48);
}

const nir_shader_compiler_options *
nvk_physical_device_nir_options(const struct nvk_physical_device *pdev,
                                gl_shader_stage stage)
{
   if (use_nak(pdev, stage))
      return nak_nir_options(pdev->nak);
   else
      return nvk_cg_nir_options(pdev, stage);
}

struct spirv_to_nir_options
nvk_physical_device_spirv_options(const struct nvk_physical_device *pdev,
                                  const struct vk_pipeline_robustness_state *rs)
{
   return (struct spirv_to_nir_options) {
      .caps = {
         .demote_to_helper_invocation = true,
         .descriptor_array_dynamic_indexing = true,
         .descriptor_array_non_uniform_indexing = true,
         .descriptor_indexing = true,
         .device_group = true,
         .draw_parameters = true,
         .float_controls = true,
         .float64 = true,
         .fragment_barycentric = true,
         .geometry_streams = true,
         .image_atomic_int64 = true,
         .image_read_without_format = true,
         .image_write_without_format = true,
         .int8 = true,
         .int16 = true,
         .int64 = true,
         .int64_atomics = true,
         .min_lod = true,
         .multiview = true,
         .physical_storage_buffer_address = true,
         .runtime_descriptor_array = true,
         .shader_clock = true,
         .shader_viewport_index_layer = true,
         .storage_8bit = true,
         .storage_16bit = true,
         .subgroup_arithmetic = true,
         .subgroup_ballot = true,
         .subgroup_basic = true,
         .subgroup_quad = true,
         .subgroup_shuffle = true,
         .subgroup_vote = true,
         .tessellation = true,
         .transform_feedback = true,
         .variable_pointers = true,
         .vk_memory_model_device_scope = true,
         .vk_memory_model = true,
         .workgroup_memory_explicit_layout = true,
      },
      .ssbo_addr_format = nvk_buffer_addr_format(rs->storage_buffers),
      .phys_ssbo_addr_format = nir_address_format_64bit_global,
      .ubo_addr_format = nvk_buffer_addr_format(rs->uniform_buffers),
      .shared_addr_format = nir_address_format_32bit_offset,
      .min_ssbo_alignment = NVK_MIN_SSBO_ALIGNMENT,
      .min_ubo_alignment = nvk_min_cbuf_alignment(&pdev->info),
   };
}

static bool
lower_load_global_constant_offset_instr(nir_builder *b,
                                        nir_intrinsic_instr *intrin,
                                        UNUSED void *_data)
{
   if (intrin->intrinsic != nir_intrinsic_load_global_constant_offset &&
       intrin->intrinsic != nir_intrinsic_load_global_constant_bounded)
      return false;

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *base_addr = intrin->src[0].ssa;
   nir_def *offset = intrin->src[1].ssa;

   nir_def *zero = NULL;
   if (intrin->intrinsic == nir_intrinsic_load_global_constant_bounded) {
      nir_def *bound = intrin->src[2].ssa;

      unsigned bit_size = intrin->def.bit_size;
      assert(bit_size >= 8 && bit_size % 8 == 0);
      unsigned byte_size = bit_size / 8;

      zero = nir_imm_zero(b, intrin->num_components, bit_size);

      unsigned load_size = byte_size * intrin->num_components;

      nir_def *sat_offset =
         nir_umin(b, offset, nir_imm_int(b, UINT32_MAX - (load_size - 1)));
      nir_def *in_bounds =
         nir_ilt(b, nir_iadd_imm(b, sat_offset, load_size - 1), bound);

      nir_push_if(b, in_bounds);
   }

   nir_def *val =
      nir_build_load_global_constant(b, intrin->def.num_components,
                                     intrin->def.bit_size,
                                     nir_iadd(b, base_addr, nir_u2u64(b, offset)),
                                     .align_mul = nir_intrinsic_align_mul(intrin),
                                     .align_offset = nir_intrinsic_align_offset(intrin));

   if (intrin->intrinsic == nir_intrinsic_load_global_constant_bounded) {
      nir_pop_if(b, NULL);
      val = nir_if_phi(b, val, zero);
   }

   nir_def_rewrite_uses(&intrin->def, val);

   return true;
}

static const struct vk_ycbcr_conversion_state *
lookup_ycbcr_conversion(const void *_layout, uint32_t set,
                        uint32_t binding, uint32_t array_index)
{
   const struct vk_pipeline_layout *pipeline_layout = _layout;
   assert(set < pipeline_layout->set_count);
   const struct nvk_descriptor_set_layout *set_layout =
      vk_to_nvk_descriptor_set_layout(pipeline_layout->set_layouts[set]);
   assert(binding < set_layout->binding_count);

   const struct nvk_descriptor_set_binding_layout *bind_layout =
      &set_layout->binding[binding];

   if (bind_layout->immutable_samplers == NULL)
      return NULL;

   array_index = MIN2(array_index, bind_layout->array_size - 1);

   const struct nvk_sampler *sampler =
      bind_layout->immutable_samplers[array_index];

   return sampler && sampler->vk.ycbcr_conversion ?
          &sampler->vk.ycbcr_conversion->state : NULL;
}

VkResult
nvk_shader_stage_to_nir(struct nvk_device *dev,
                        const VkPipelineShaderStageCreateInfo *sinfo,
                        const struct vk_pipeline_robustness_state *rstate,
                        struct vk_pipeline_cache *cache,
                        void *mem_ctx, struct nir_shader **nir_out)
{
   struct nvk_physical_device *pdev = nvk_device_physical(dev);
   const gl_shader_stage stage = vk_to_mesa_shader_stage(sinfo->stage);
   const nir_shader_compiler_options *nir_options =
      nvk_physical_device_nir_options(pdev, stage);

   unsigned char stage_sha1[SHA1_DIGEST_LENGTH];
   vk_pipeline_hash_shader_stage(sinfo, rstate, stage_sha1);

   if (cache == NULL)
      cache = dev->mem_cache;

   nir_shader *nir = vk_pipeline_cache_lookup_nir(cache, stage_sha1,
                                                  sizeof(stage_sha1),
                                                  nir_options, NULL,
                                                  mem_ctx);
   if (nir != NULL) {
      *nir_out = nir;
      return VK_SUCCESS;
   }

   const struct spirv_to_nir_options spirv_options =
      nvk_physical_device_spirv_options(pdev, rstate);

   VkResult result = vk_pipeline_shader_stage_to_nir(&dev->vk, sinfo,
                                                     &spirv_options,
                                                     nir_options,
                                                     mem_ctx, &nir);
   if (result != VK_SUCCESS)
      return result;

   NIR_PASS_V(nir, nir_lower_io_to_temporaries,
              nir_shader_get_entrypoint(nir), true, false);

   if (use_nak(dev->pdev, nir->info.stage))
      nak_preprocess_nir(nir, NULL);
   else
      nvk_cg_preprocess_nir(nir);

   vk_pipeline_cache_add_nir(cache, stage_sha1, sizeof(stage_sha1), nir);

   *nir_out = nir;

   return VK_SUCCESS;
}

static inline bool
nir_has_image_var(nir_shader *nir)
{
   nir_foreach_image_variable(_, nir)
      return true;

   return false;
}

void
nvk_lower_nir(struct nvk_device *dev, nir_shader *nir,
              const struct vk_pipeline_robustness_state *rs,
              bool is_multiview,
              const struct vk_pipeline_layout *layout,
              struct nvk_shader *shader)
{
   struct nvk_physical_device *pdev = nvk_device_physical(dev);

   if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS(_, nir, nir_lower_input_attachments,
               &(nir_input_attachment_options) {
                  .use_fragcoord_sysval = use_nak(pdev, nir->info.stage),
                  .use_layer_id_sysval = use_nak(pdev, nir->info.stage) ||
                                         is_multiview,
                  .use_view_id_for_layer = is_multiview,
               });
   }

   NIR_PASS(_, nir, nir_vk_lower_ycbcr_tex, lookup_ycbcr_conversion, layout);

   nir_lower_compute_system_values_options csv_options = {
      .has_base_workgroup_id = true,
   };
   NIR_PASS(_, nir, nir_lower_compute_system_values, &csv_options);

   /* Lower push constants before lower_descriptors */
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_push_const,
            nir_address_format_32bit_offset);

   /* Lower non-uniform access before lower_descriptors */
   enum nir_lower_non_uniform_access_type lower_non_uniform_access_types =
      nir_lower_non_uniform_ubo_access;

   if (pdev->info.cls_eng3d < TURING_A) {
      lower_non_uniform_access_types |= nir_lower_non_uniform_texture_access |
                                        nir_lower_non_uniform_image_access;
   }

   /* In practice, most shaders do not have non-uniform-qualified accesses
    * thus a cheaper and likely to fail check is run first.
    */
   if (nir_has_non_uniform_access(nir, lower_non_uniform_access_types)) {
      struct nir_lower_non_uniform_access_options opts = {
         .types = lower_non_uniform_access_types,
         .callback = NULL,
      };
      NIR_PASS(_, nir, nir_opt_non_uniform_access);
      NIR_PASS(_, nir, nir_lower_non_uniform_access, &opts);
   }

   /* TODO: Kepler image lowering requires image params to be loaded from the
    * descriptor set which we don't currently support.
    */
   assert(dev->pdev->info.cls_eng3d >= MAXWELL_A || !nir_has_image_var(nir));

   struct nvk_cbuf_map *cbuf_map = NULL;
   if (use_nak(pdev, nir->info.stage) &&
       !(pdev->debug_flags & NVK_DEBUG_NO_CBUF)) {
      cbuf_map = &shader->cbuf_map;
   } else {
      /* Codegen sometimes puts stuff in cbuf 1 and adds 1 to our cbuf indices
       * so we can't really rely on it for lowering to cbufs and instead place
       * the root descriptors in both cbuf 0 and cbuf 1.
       */
      shader->cbuf_map = (struct nvk_cbuf_map) {
         .cbuf_count = 2,
         .cbufs = {
            { .type = NVK_CBUF_TYPE_ROOT_DESC },
            { .type = NVK_CBUF_TYPE_ROOT_DESC },
         }
      };
   }

   NIR_PASS(_, nir, nvk_nir_lower_descriptors, rs, layout, cbuf_map);
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_global,
            nir_address_format_64bit_global);
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_ssbo,
            nvk_buffer_addr_format(rs->storage_buffers));
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_ubo,
            nvk_buffer_addr_format(rs->uniform_buffers));
   NIR_PASS(_, nir, nir_shader_intrinsics_pass,
            lower_load_global_constant_offset_instr, nir_metadata_none, NULL);

   if (!nir->info.shared_memory_explicit_layout) {
      NIR_PASS(_, nir, nir_lower_vars_to_explicit_types,
               nir_var_mem_shared, shared_var_info);
   }
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_shared,
            nir_address_format_32bit_offset);
}

#ifndef NDEBUG
static void
nvk_shader_dump(struct nvk_shader *shader)
{
   unsigned pos;

   if (shader->info.stage != MESA_SHADER_COMPUTE) {
      _debug_printf("dumping HDR for %s shader\n",
                    _mesa_shader_stage_to_string(shader->info.stage));
      for (pos = 0; pos < ARRAY_SIZE(shader->info.hdr); ++pos)
         _debug_printf("HDR[%02"PRIxPTR"] = 0x%08x\n",
                      pos * sizeof(shader->info.hdr[0]), shader->info.hdr[pos]);
   }
   _debug_printf("shader binary code (0x%x bytes):", shader->code_size);
   for (pos = 0; pos < shader->code_size / 4; ++pos) {
      if ((pos % 8) == 0)
         _debug_printf("\n");
      _debug_printf("%08x ", ((const uint32_t *)shader->code_ptr)[pos]);
   }
   _debug_printf("\n");
}
#endif

static VkResult
nvk_compile_nir_with_nak(struct nvk_physical_device *pdev,
                         nir_shader *nir,
                         VkPipelineCreateFlagBits2KHR pipeline_flags,
                         const struct vk_pipeline_robustness_state *rs,
                         const struct nak_fs_key *fs_key,
                         struct nvk_shader *shader)
{
   const bool dump_asm =
      pipeline_flags & VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;

   nir_variable_mode robust2_modes = 0;
   if (rs->uniform_buffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT)
      robust2_modes |= nir_var_mem_ubo;
   if (rs->storage_buffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT)
      robust2_modes |= nir_var_mem_ssbo;

   shader->nak = nak_compile_shader(nir, dump_asm, pdev->nak, robust2_modes, fs_key);
   shader->info = shader->nak->info;
   shader->code_ptr = shader->nak->code;
   shader->code_size = shader->nak->code_size;

   return VK_SUCCESS;
}

struct nvk_shader *
nvk_shader_init(struct nvk_device *dev, const void *key_data, size_t key_size)
{
   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct nvk_shader, shader, 1);
   VK_MULTIALLOC_DECL_SIZE(&ma, char, obj_key_data, key_size);

   if (!vk_multialloc_zalloc(&ma, &dev->vk.alloc,
                             VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
      return NULL;

   memcpy(obj_key_data, key_data, key_size);

   vk_pipeline_cache_object_init(&dev->vk, &shader->base,
                                 &nvk_shader_ops, obj_key_data, key_size);

   return shader;
}

VkResult
nvk_compile_nir(struct nvk_device *dev, nir_shader *nir,
                VkPipelineCreateFlagBits2KHR pipeline_flags,
                const struct vk_pipeline_robustness_state *rs,
                const struct nak_fs_key *fs_key,
                struct vk_pipeline_cache *cache,
                struct nvk_shader *shader)
{
   struct nvk_physical_device *pdev = nvk_device_physical(dev);

   if (use_nak(pdev, nir->info.stage)) {
      return nvk_compile_nir_with_nak(pdev, nir, pipeline_flags, rs,
                                      fs_key, shader);
   } else {
      return nvk_cg_compile_nir(pdev, nir, fs_key, shader);
   }
}

VkResult
nvk_shader_upload(struct nvk_device *dev, struct nvk_shader *shader)
{
   uint32_t hdr_size = 0;
   if (shader->info.stage != MESA_SHADER_COMPUTE) {
      if (dev->pdev->info.cls_eng3d >= TURING_A)
         hdr_size = TU102_SHADER_HEADER_SIZE;
      else
         hdr_size = GF100_SHADER_HEADER_SIZE;
   }

   /* Fermi   needs 0x40 alignment
    * Kepler+ needs the first instruction to be 0x80 aligned, so we waste 0x30 bytes
    */
   int alignment = dev->pdev->info.cls_eng3d >= KEPLER_A ? 0x80 : 0x40;
   int offset = 0;

   if (dev->pdev->info.cls_eng3d >= KEPLER_A &&
       dev->pdev->info.cls_eng3d < TURING_A &&
       hdr_size > 0) {
      /* offset will be 0x30 */
      offset = alignment - hdr_size;
   }

   uint32_t total_size = shader->code_size + hdr_size + offset;
   char *data = malloc(total_size);
   if (data == NULL)
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   assert(hdr_size <= sizeof(shader->info.hdr));
   memcpy(data + offset, shader->info.hdr, hdr_size);
   memcpy(data + offset + hdr_size, shader->code_ptr, shader->code_size);

#ifndef NDEBUG
   if (debug_get_bool_option("NV50_PROG_DEBUG", false))
      nvk_shader_dump(shader);
#endif

   VkResult result = nvk_heap_upload(dev, &dev->shader_heap, data,
                                     total_size, alignment, &shader->upload_addr);
   if (result == VK_SUCCESS) {
      shader->upload_size = total_size;
      shader->upload_padding = offset;
   }
   free(data);

   return result;
}

void
nvk_shader_finish(struct nvk_device *dev, struct nvk_shader *shader)
{
   if (shader == NULL)
      return;

   if (shader->upload_size > 0) {
      nvk_heap_free(dev, &dev->shader_heap,
                    shader->upload_addr,
                    shader->upload_size);
   }

   if (shader->nak) {
      nak_shader_bin_destroy(shader->nak);
   } else {
      /* This came from codegen or deserialize, just free it */
      free((void *)shader->code_ptr);
   }

   vk_free(&dev->vk.alloc, shader);
}

void
nvk_hash_shader(unsigned char *hash,
                const VkPipelineShaderStageCreateInfo *sinfo,
                const struct vk_pipeline_robustness_state *rs,
                bool is_multiview,
                const struct vk_pipeline_layout *layout,
                const struct nak_fs_key *fs_key)
{
   struct mesa_sha1 ctx;

   _mesa_sha1_init(&ctx);

   unsigned char stage_sha1[SHA1_DIGEST_LENGTH];
   vk_pipeline_hash_shader_stage(sinfo, rs, stage_sha1);

   _mesa_sha1_update(&ctx, stage_sha1, sizeof(stage_sha1));

   _mesa_sha1_update(&ctx, &is_multiview, sizeof(is_multiview));

   if (layout) {
      _mesa_sha1_update(&ctx, &layout->create_flags,
                        sizeof(layout->create_flags));
      _mesa_sha1_update(&ctx, &layout->set_count, sizeof(layout->set_count));
      for (int i = 0; i < layout->set_count; i++) {
         struct nvk_descriptor_set_layout *set =
            vk_to_nvk_descriptor_set_layout(layout->set_layouts[i]);
         _mesa_sha1_update(&ctx, &set->sha1, sizeof(set->sha1));
      }
   }

   if(fs_key)
      _mesa_sha1_update(&ctx, fs_key, sizeof(*fs_key));

   _mesa_sha1_final(&ctx, hash);
}

static bool
nvk_shader_serialize(struct vk_pipeline_cache_object *object,
                     struct blob *blob);

static struct vk_pipeline_cache_object *
nvk_shader_deserialize(struct vk_pipeline_cache *cache,
                       const void *key_data,
                       size_t key_size,
                       struct blob_reader *blob);

void
nvk_shader_destroy(struct vk_device *_dev,
                   struct vk_pipeline_cache_object *object)
{
   struct nvk_device *dev =
      container_of(_dev, struct nvk_device, vk);
   struct nvk_shader *shader =
      container_of(object, struct nvk_shader, base);

   nvk_shader_finish(dev, shader);
}

const struct vk_pipeline_cache_object_ops nvk_shader_ops = {
   .serialize = nvk_shader_serialize,
   .deserialize = nvk_shader_deserialize,
   .destroy = nvk_shader_destroy,
};

static bool
nvk_shader_serialize(struct vk_pipeline_cache_object *object,
                     struct blob *blob)
{
   struct nvk_shader *shader =
      container_of(object, struct nvk_shader, base);

   blob_write_bytes(blob, &shader->info, sizeof(shader->info));
   blob_write_bytes(blob, &shader->cbuf_map, sizeof(shader->cbuf_map));
   blob_write_uint32(blob, shader->code_size);
   blob_write_bytes(blob, shader->code_ptr, shader->code_size);

   return true;
}

static struct vk_pipeline_cache_object *
nvk_shader_deserialize(struct vk_pipeline_cache *cache,
                       const void *key_data,
                       size_t key_size,
                       struct blob_reader *blob)
{
   struct nvk_device *dev =
      container_of(cache->base.device, struct nvk_device, vk);
   struct nvk_shader *shader =
      nvk_shader_init(dev, key_data, key_size);

   if (!shader)
      return NULL;

   blob_copy_bytes(blob, &shader->info, sizeof(shader->info));
   blob_copy_bytes(blob, &shader->cbuf_map, sizeof(shader->cbuf_map));

   shader->code_size = blob_read_uint32(blob);
   void *code_ptr = malloc(shader->code_size);
   if (!code_ptr)
      goto fail;

   blob_copy_bytes(blob, code_ptr, shader->code_size);
   shader->code_ptr = code_ptr;

   return &shader->base;

fail:
   /* nvk_shader_destroy frees both shader and shader->xfb */
   nvk_shader_destroy(cache->base.device, &shader->base);
   return NULL;
}
