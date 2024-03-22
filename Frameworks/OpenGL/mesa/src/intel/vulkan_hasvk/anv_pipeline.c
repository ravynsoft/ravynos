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

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "util/mesa-sha1.h"
#include "util/os_time.h"
#include "common/intel_l3_config.h"
#include "common/intel_disasm.h"
#include "common/intel_sample_positions.h"
#include "anv_private.h"
#include "compiler/brw_nir.h"
#include "compiler/brw_nir_rt.h"
#include "anv_nir.h"
#include "nir/nir_xfb_info.h"
#include "spirv/nir_spirv.h"
#include "vk_pipeline.h"
#include "vk_render_pass.h"
#include "vk_util.h"

/* Eventually, this will become part of anv_CreateShader.  Unfortunately,
 * we can't do that yet because we don't have the ability to copy nir.
 */
static nir_shader *
anv_shader_stage_to_nir(struct anv_device *device,
                        const VkPipelineShaderStageCreateInfo *stage_info,
                        enum brw_robustness_flags robust_flags,
                        void *mem_ctx)
{
   const struct anv_physical_device *pdevice = device->physical;
   const struct anv_instance *instance = pdevice->instance;
   const struct brw_compiler *compiler = pdevice->compiler;
   gl_shader_stage stage = vk_to_mesa_shader_stage(stage_info->stage);
   const nir_shader_compiler_options *nir_options =
      compiler->nir_options[stage];

   const struct spirv_to_nir_options spirv_options = {
      .caps = {
         .demote_to_helper_invocation = true,
         .derivative_group = true,
         .descriptor_array_dynamic_indexing = true,
         .descriptor_array_non_uniform_indexing = true,
         .descriptor_indexing = true,
         .device_group = true,
         .draw_parameters = true,
         .float16 = pdevice->info.ver >= 8,
         .float32_atomic_add = pdevice->info.has_lsc,
         .float32_atomic_min_max = pdevice->info.ver >= 9,
         .float64 = pdevice->info.ver >= 8,
         .float64_atomic_min_max = pdevice->info.has_lsc,
         .fragment_shader_sample_interlock = pdevice->info.ver >= 9,
         .fragment_shader_pixel_interlock = pdevice->info.ver >= 9,
         .geometry_streams = true,
         /* When using Vulkan 1.3 or KHR_format_feature_flags2 is enabled, the
          * read/write without format is per format, so just report true. It's
          * up to the application to check.
          */
         .image_read_without_format = instance->vk.app_info.api_version >= VK_API_VERSION_1_3 || device->vk.enabled_extensions.KHR_format_feature_flags2,
         .image_write_without_format = true,
         .int8 = pdevice->info.ver >= 8,
         .int16 = pdevice->info.ver >= 8,
         .int64 = pdevice->info.ver >= 8,
         .int64_atomics = pdevice->info.ver >= 9 && pdevice->use_softpin,
         .integer_functions2 = pdevice->info.ver >= 8,
         .min_lod = true,
         .multiview = true,
         .physical_storage_buffer_address = pdevice->has_a64_buffer_access,
         .post_depth_coverage = pdevice->info.ver >= 9,
         .runtime_descriptor_array = true,
         .float_controls = true,
         .shader_clock = true,
         .shader_viewport_index_layer = true,
         .stencil_export = pdevice->info.ver >= 9,
         .storage_8bit = pdevice->info.ver >= 8,
         .storage_16bit = pdevice->info.ver >= 8,
         .subgroup_arithmetic = true,
         .subgroup_basic = true,
         .subgroup_ballot = true,
         .subgroup_dispatch = true,
         .subgroup_quad = true,
         .subgroup_uniform_control_flow = true,
         .subgroup_shuffle = true,
         .subgroup_vote = true,
         .tessellation = true,
         .transform_feedback = true,
         .variable_pointers = true,
         .vk_memory_model = true,
         .vk_memory_model_device_scope = true,
         .workgroup_memory_explicit_layout = true,
      },
      .ubo_addr_format = anv_nir_ubo_addr_format(pdevice, robust_flags),
      .ssbo_addr_format = anv_nir_ssbo_addr_format(pdevice, robust_flags),
      .phys_ssbo_addr_format = nir_address_format_64bit_global,
      .push_const_addr_format = nir_address_format_logical,

      /* TODO: Consider changing this to an address format that has the NULL
       * pointer equals to 0.  That might be a better format to play nice
       * with certain code / code generators.
       */
      .shared_addr_format = nir_address_format_32bit_offset,

      .min_ubo_alignment = ANV_UBO_ALIGNMENT,
      .min_ssbo_alignment = ANV_SSBO_ALIGNMENT,
   };

   nir_shader *nir;
   VkResult result =
      vk_pipeline_shader_stage_to_nir(&device->vk, stage_info,
                                      &spirv_options, nir_options,
                                      mem_ctx, &nir);
   if (result != VK_SUCCESS)
      return NULL;

   if (INTEL_DEBUG(intel_debug_flag_for_shader_stage(stage))) {
      fprintf(stderr, "NIR (from SPIR-V) for %s shader:\n",
              gl_shader_stage_name(stage));
      nir_print_shader(nir, stderr);
   }

   NIR_PASS_V(nir, nir_lower_io_to_temporaries,
              nir_shader_get_entrypoint(nir), true, false);

   const struct nir_lower_sysvals_to_varyings_options sysvals_to_varyings = {
      .point_coord = true,
   };
   NIR_PASS(_, nir, nir_lower_sysvals_to_varyings, &sysvals_to_varyings);

   const nir_opt_access_options opt_access_options = {
      .is_vulkan = true,
   };
   NIR_PASS(_, nir, nir_opt_access, &opt_access_options);

   /* Vulkan uses the separate-shader linking model */
   nir->info.separate_shader = true;

   struct brw_nir_compiler_opts opts = {};

   brw_preprocess_nir(compiler, nir, &opts);

   return nir;
}

VkResult
anv_pipeline_init(struct anv_pipeline *pipeline,
                  struct anv_device *device,
                  enum anv_pipeline_type type,
                  VkPipelineCreateFlags flags,
                  const VkAllocationCallbacks *pAllocator)
{
   VkResult result;

   memset(pipeline, 0, sizeof(*pipeline));

   vk_object_base_init(&device->vk, &pipeline->base,
                       VK_OBJECT_TYPE_PIPELINE);
   pipeline->device = device;

   /* It's the job of the child class to provide actual backing storage for
    * the batch by setting batch.start, batch.next, and batch.end.
    */
   pipeline->batch.alloc = pAllocator ? pAllocator : &device->vk.alloc;
   pipeline->batch.relocs = &pipeline->batch_relocs;
   pipeline->batch.status = VK_SUCCESS;

   result = anv_reloc_list_init(&pipeline->batch_relocs,
                                pipeline->batch.alloc);
   if (result != VK_SUCCESS)
      return result;

   pipeline->mem_ctx = ralloc_context(NULL);

   pipeline->type = type;
   pipeline->flags = flags;

   util_dynarray_init(&pipeline->executables, pipeline->mem_ctx);

   return VK_SUCCESS;
}

void
anv_pipeline_finish(struct anv_pipeline *pipeline,
                    struct anv_device *device,
                    const VkAllocationCallbacks *pAllocator)
{
   anv_reloc_list_finish(&pipeline->batch_relocs,
                         pAllocator ? pAllocator : &device->vk.alloc);
   ralloc_free(pipeline->mem_ctx);
   vk_object_base_finish(&pipeline->base);
}

void anv_DestroyPipeline(
    VkDevice                                    _device,
    VkPipeline                                  _pipeline,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_pipeline, pipeline, _pipeline);

   if (!pipeline)
      return;

   switch (pipeline->type) {
   case ANV_PIPELINE_GRAPHICS: {
      struct anv_graphics_pipeline *gfx_pipeline =
         anv_pipeline_to_graphics(pipeline);

      for (unsigned s = 0; s < ARRAY_SIZE(gfx_pipeline->shaders); s++) {
         if (gfx_pipeline->shaders[s])
            anv_shader_bin_unref(device, gfx_pipeline->shaders[s]);
      }
      break;
   }

   case ANV_PIPELINE_COMPUTE: {
      struct anv_compute_pipeline *compute_pipeline =
         anv_pipeline_to_compute(pipeline);

      if (compute_pipeline->cs)
         anv_shader_bin_unref(device, compute_pipeline->cs);

      break;
   }

   default:
      unreachable("invalid pipeline type");
   }

   anv_pipeline_finish(pipeline, device, pAllocator);
   vk_free2(&device->vk.alloc, pAllocator, pipeline);
}

static void
populate_sampler_prog_key(const struct intel_device_info *devinfo,
                          struct brw_sampler_prog_key_data *key)
{
   /* XXX: Handle texture swizzle Pre-HSW */
}

static void
populate_base_prog_key(const struct anv_device *device,
                       enum brw_robustness_flags robust_flags,
                       struct brw_base_prog_key *key)
{
   key->robust_flags = robust_flags;
   key->limit_trig_input_range =
      device->physical->instance->limit_trig_input_range;

   populate_sampler_prog_key(device->info, &key->tex);
}

static void
populate_vs_prog_key(const struct anv_device *device,
                     enum brw_robustness_flags robust_flags,
                     struct brw_vs_prog_key *key)
{
   memset(key, 0, sizeof(*key));

   populate_base_prog_key(device, robust_flags, &key->base);

   /* XXX: Handle vertex input work-arounds */

   /* XXX: Handle sampler_prog_key */
}

static void
populate_tcs_prog_key(const struct anv_device *device,
                      enum brw_robustness_flags robust_flags,
                      unsigned input_vertices,
                      struct brw_tcs_prog_key *key)
{
   memset(key, 0, sizeof(*key));

   populate_base_prog_key(device, robust_flags, &key->base);

   key->input_vertices = input_vertices;
}

static void
populate_tes_prog_key(const struct anv_device *device,
                      enum brw_robustness_flags robust_flags,
                      struct brw_tes_prog_key *key)
{
   memset(key, 0, sizeof(*key));

   populate_base_prog_key(device, robust_flags, &key->base);
}

static void
populate_gs_prog_key(const struct anv_device *device,
                     bool robust_flags,
                     struct brw_gs_prog_key *key)
{
   memset(key, 0, sizeof(*key));

   populate_base_prog_key(device, robust_flags, &key->base);
}

static void
populate_wm_prog_key(const struct anv_graphics_pipeline *pipeline,
                     enum brw_robustness_flags robust_flags,
                     const BITSET_WORD *dynamic,
                     const struct vk_multisample_state *ms,
                     const struct vk_render_pass_state *rp,
                     struct brw_wm_prog_key *key)
{
   const struct anv_device *device = pipeline->base.device;

   memset(key, 0, sizeof(*key));

   populate_base_prog_key(device, robust_flags, &key->base);

   /* We set this to 0 here and set to the actual value before we call
    * brw_compile_fs.
    */
   key->input_slots_valid = 0;

   /* XXX Vulkan doesn't appear to specify */
   key->clamp_fragment_color = false;

   key->ignore_sample_mask_out = false;

   assert(rp->color_attachment_count <= MAX_RTS);
   /* Consider all inputs as valid until look at the NIR variables. */
   key->color_outputs_valid = (1u << rp->color_attachment_count) - 1;
   key->nr_color_regions = rp->color_attachment_count;

   /* To reduce possible shader recompilations we would need to know if
    * there is a SampleMask output variable to compute if we should emit
    * code to workaround the issue that hardware disables alpha to coverage
    * when there is SampleMask output.
    */
   key->alpha_to_coverage = ms != NULL && ms->alpha_to_coverage_enable ?
      BRW_ALWAYS : BRW_NEVER;

   /* Vulkan doesn't support fixed-function alpha test */
   key->alpha_test_replicate_alpha = false;

   if (ms != NULL) {
      /* We should probably pull this out of the shader, but it's fairly
       * harmless to compute it and then let dead-code take care of it.
       */
      if (ms->rasterization_samples > 1) {
         key->persample_interp =
            (ms->sample_shading_enable &&
             (ms->min_sample_shading * ms->rasterization_samples) > 1) ?
            BRW_ALWAYS : BRW_NEVER;
         key->multisample_fbo = BRW_ALWAYS;
      }

      if (device->physical->instance->sample_mask_out_opengl_behaviour)
         key->ignore_sample_mask_out = !key->multisample_fbo;
   }
}

static void
populate_cs_prog_key(const struct anv_device *device,
                     enum brw_robustness_flags robust_flags,
                     struct brw_cs_prog_key *key)
{
   memset(key, 0, sizeof(*key));

   populate_base_prog_key(device, robust_flags, &key->base);
}

static void
populate_bs_prog_key(const struct anv_device *device,
                     enum brw_robustness_flags robust_flags,
                     struct brw_bs_prog_key *key)
{
   memset(key, 0, sizeof(*key));

   populate_base_prog_key(device, robust_flags, &key->base);
}

struct anv_pipeline_stage {
   gl_shader_stage stage;

   const VkPipelineShaderStageCreateInfo *info;

   unsigned char shader_sha1[20];

   union brw_any_prog_key key;

   struct {
      gl_shader_stage stage;
      unsigned char sha1[20];
   } cache_key;

   nir_shader *nir;

   struct anv_pipeline_binding surface_to_descriptor[256];
   struct anv_pipeline_binding sampler_to_descriptor[256];
   struct anv_pipeline_bind_map bind_map;

   union brw_any_prog_data prog_data;

   uint32_t num_stats;
   struct brw_compile_stats stats[3];
   char *disasm[3];

   VkPipelineCreationFeedback feedback;

   const unsigned *code;

   struct anv_shader_bin *bin;
};

static void
anv_pipeline_hash_graphics(struct anv_graphics_pipeline *pipeline,
                           struct anv_pipeline_layout *layout,
                           struct anv_pipeline_stage *stages,
                           unsigned char *sha1_out)
{
   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);

   _mesa_sha1_update(&ctx, &pipeline->view_mask,
                     sizeof(pipeline->view_mask));

   if (layout)
      _mesa_sha1_update(&ctx, layout->sha1, sizeof(layout->sha1));

   for (uint32_t s = 0; s < ANV_GRAPHICS_SHADER_STAGE_COUNT; s++) {
      if (stages[s].info) {
         _mesa_sha1_update(&ctx, stages[s].shader_sha1,
                           sizeof(stages[s].shader_sha1));
         _mesa_sha1_update(&ctx, &stages[s].key, brw_prog_key_size(s));
      }
   }

   _mesa_sha1_final(&ctx, sha1_out);
}

static void
anv_pipeline_hash_compute(struct anv_compute_pipeline *pipeline,
                          struct anv_pipeline_layout *layout,
                          struct anv_pipeline_stage *stage,
                          unsigned char *sha1_out)
{
   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);

   if (layout)
      _mesa_sha1_update(&ctx, layout->sha1, sizeof(layout->sha1));

   const struct anv_device *device = pipeline->base.device;

   const bool rba = device->vk.enabled_features.robustBufferAccess;
   _mesa_sha1_update(&ctx, &rba, sizeof(rba));

   const uint8_t afs = device->physical->instance->assume_full_subgroups;
   _mesa_sha1_update(&ctx, &afs, sizeof(afs));

   _mesa_sha1_update(&ctx, stage->shader_sha1,
                     sizeof(stage->shader_sha1));
   _mesa_sha1_update(&ctx, &stage->key.cs, sizeof(stage->key.cs));

   _mesa_sha1_final(&ctx, sha1_out);
}

static nir_shader *
anv_pipeline_stage_get_nir(struct anv_pipeline *pipeline,
                           struct vk_pipeline_cache *cache,
                           void *mem_ctx,
                           struct anv_pipeline_stage *stage)
{
   const struct brw_compiler *compiler =
      pipeline->device->physical->compiler;
   const nir_shader_compiler_options *nir_options =
      compiler->nir_options[stage->stage];
   nir_shader *nir;

   nir = anv_device_search_for_nir(pipeline->device, cache,
                                   nir_options,
                                   stage->shader_sha1,
                                   mem_ctx);
   if (nir) {
      assert(nir->info.stage == stage->stage);
      return nir;
   }

   nir = anv_shader_stage_to_nir(pipeline->device, stage->info,
                                 stage->key.base.robust_flags, mem_ctx);
   if (nir) {
      anv_device_upload_nir(pipeline->device, cache, nir, stage->shader_sha1);
      return nir;
   }

   return NULL;
}

static void
shared_type_info(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   assert(glsl_type_is_vector_or_scalar(type));

   uint32_t comp_size = glsl_type_is_boolean(type)
      ? 4 : glsl_get_bit_size(type) / 8;
   unsigned length = glsl_get_vector_elements(type);
   *size = comp_size * length,
   *align = comp_size * (length == 3 ? 4 : length);
}

static void
anv_pipeline_lower_nir(struct anv_pipeline *pipeline,
                       void *mem_ctx,
                       struct anv_pipeline_stage *stage,
                       struct anv_pipeline_layout *layout)
{
   const struct anv_physical_device *pdevice = pipeline->device->physical;
   const struct brw_compiler *compiler = pdevice->compiler;

   struct brw_stage_prog_data *prog_data = &stage->prog_data.base;
   nir_shader *nir = stage->nir;

   if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS(_, nir, nir_lower_wpos_center);
      NIR_PASS(_, nir, nir_lower_input_attachments,
               &(nir_input_attachment_options) {
                   .use_fragcoord_sysval = true,
                   .use_layer_id_sysval = true,
               });
   }

   NIR_PASS(_, nir, anv_nir_lower_ycbcr_textures, layout);

   if (pipeline->type == ANV_PIPELINE_GRAPHICS) {
      struct anv_graphics_pipeline *gfx_pipeline =
         anv_pipeline_to_graphics(pipeline);
      NIR_PASS(_, nir, anv_nir_lower_multiview, gfx_pipeline->view_mask);
   }

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   NIR_PASS(_, nir, brw_nir_lower_storage_image,
            &(struct brw_nir_lower_storage_image_opts) {
               .devinfo = compiler->devinfo,
               .lower_loads = true,
               .lower_stores = true,
               .lower_atomics = true,
               .lower_get_size = true,
            });

   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_global,
            nir_address_format_64bit_global);
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_push_const,
            nir_address_format_32bit_offset);

   /* Apply the actual pipeline layout to UBOs, SSBOs, and textures */
   NIR_PASS_V(nir, anv_nir_apply_pipeline_layout,
              pdevice, stage->key.base.robust_flags,
              layout, &stage->bind_map);

   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_ubo,
            anv_nir_ubo_addr_format(pdevice, stage->key.base.robust_flags));
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_ssbo,
            anv_nir_ssbo_addr_format(pdevice, stage->key.base.robust_flags));

   /* First run copy-prop to get rid of all of the vec() that address
    * calculations often create and then constant-fold so that, when we
    * get to anv_nir_lower_ubo_loads, we can detect constant offsets.
    */
   NIR_PASS(_, nir, nir_copy_prop);
   NIR_PASS(_, nir, nir_opt_constant_folding);

   NIR_PASS(_, nir, anv_nir_lower_ubo_loads);

   enum nir_lower_non_uniform_access_type lower_non_uniform_access_types =
      nir_lower_non_uniform_texture_access | nir_lower_non_uniform_image_access;

   /* In practice, most shaders do not have non-uniform-qualified
    * accesses (see
    * https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/17558#note_1475069)
    * thus a cheaper and likely to fail check is run first.
    */
   if (nir_has_non_uniform_access(nir, lower_non_uniform_access_types)) {
      NIR_PASS(_, nir, nir_opt_non_uniform_access);

      /* We don't support non-uniform UBOs and non-uniform SSBO access is
      * handled naturally by falling back to A64 messages.
      */
      NIR_PASS(_, nir, nir_lower_non_uniform_access,
               &(nir_lower_non_uniform_access_options) {
                  .types = lower_non_uniform_access_types,
                  .callback = NULL,
               });
   }

   NIR_PASS_V(nir, anv_nir_compute_push_layout,
              pdevice, stage->key.base.robust_flags,
              prog_data, &stage->bind_map, mem_ctx);

   if (gl_shader_stage_uses_workgroup(nir->info.stage)) {
      if (!nir->info.shared_memory_explicit_layout) {
         NIR_PASS(_, nir, nir_lower_vars_to_explicit_types,
                  nir_var_mem_shared, shared_type_info);
      }

      NIR_PASS(_, nir, nir_lower_explicit_io,
               nir_var_mem_shared, nir_address_format_32bit_offset);

      if (nir->info.zero_initialize_shared_memory &&
          nir->info.shared_size > 0) {
         /* The effective Shared Local Memory size is at least 1024 bytes and
          * is always rounded to a power of two, so it is OK to align the size
          * used by the shader to chunk_size -- which does simplify the logic.
          */
         const unsigned chunk_size = 16;
         const unsigned shared_size = ALIGN(nir->info.shared_size, chunk_size);
         assert(shared_size <=
                intel_calculate_slm_size(compiler->devinfo->ver, nir->info.shared_size));

         NIR_PASS(_, nir, nir_zero_initialize_shared_memory,
                  shared_size, chunk_size);
      }
   }

   if (gl_shader_stage_is_compute(nir->info.stage))
      NIR_PASS(_, nir, brw_nir_lower_cs_intrinsics);

   stage->nir = nir;
}

static void
anv_pipeline_link_vs(const struct brw_compiler *compiler,
                     struct anv_pipeline_stage *vs_stage,
                     struct anv_pipeline_stage *next_stage)
{
   if (next_stage)
      brw_nir_link_shaders(compiler, vs_stage->nir, next_stage->nir);
}

static void
anv_pipeline_compile_vs(const struct brw_compiler *compiler,
                        void *mem_ctx,
                        struct anv_graphics_pipeline *pipeline,
                        struct anv_pipeline_stage *vs_stage)
{
   /* When using Primitive Replication for multiview, each view gets its own
    * position slot.
    */
   uint32_t pos_slots =
      (vs_stage->nir->info.per_view_outputs & VARYING_BIT_POS) ?
      MAX2(1, util_bitcount(pipeline->view_mask)) : 1;

   /* Only position is allowed to be per-view */
   assert(!(vs_stage->nir->info.per_view_outputs & ~VARYING_BIT_POS));

   brw_compute_vue_map(compiler->devinfo,
                       &vs_stage->prog_data.vs.base.vue_map,
                       vs_stage->nir->info.outputs_written,
                       vs_stage->nir->info.separate_shader,
                       pos_slots);

   vs_stage->num_stats = 1;

   struct brw_compile_vs_params params = {
      .base = {
         .nir = vs_stage->nir,
         .stats = vs_stage->stats,
         .log_data = pipeline->base.device,
         .mem_ctx = mem_ctx,
      },
      .key = &vs_stage->key.vs,
      .prog_data = &vs_stage->prog_data.vs,
   };

   vs_stage->code = brw_compile_vs(compiler, &params);
}

static void
merge_tess_info(struct shader_info *tes_info,
                const struct shader_info *tcs_info)
{
   /* The Vulkan 1.0.38 spec, section 21.1 Tessellator says:
    *
    *    "PointMode. Controls generation of points rather than triangles
    *     or lines. This functionality defaults to disabled, and is
    *     enabled if either shader stage includes the execution mode.
    *
    * and about Triangles, Quads, IsoLines, VertexOrderCw, VertexOrderCcw,
    * PointMode, SpacingEqual, SpacingFractionalEven, SpacingFractionalOdd,
    * and OutputVertices, it says:
    *
    *    "One mode must be set in at least one of the tessellation
    *     shader stages."
    *
    * So, the fields can be set in either the TCS or TES, but they must
    * agree if set in both.  Our backend looks at TES, so bitwise-or in
    * the values from the TCS.
    */
   assert(tcs_info->tess.tcs_vertices_out == 0 ||
          tes_info->tess.tcs_vertices_out == 0 ||
          tcs_info->tess.tcs_vertices_out == tes_info->tess.tcs_vertices_out);
   tes_info->tess.tcs_vertices_out |= tcs_info->tess.tcs_vertices_out;

   assert(tcs_info->tess.spacing == TESS_SPACING_UNSPECIFIED ||
          tes_info->tess.spacing == TESS_SPACING_UNSPECIFIED ||
          tcs_info->tess.spacing == tes_info->tess.spacing);
   tes_info->tess.spacing |= tcs_info->tess.spacing;

   assert(tcs_info->tess._primitive_mode == 0 ||
          tes_info->tess._primitive_mode == 0 ||
          tcs_info->tess._primitive_mode == tes_info->tess._primitive_mode);
   tes_info->tess._primitive_mode |= tcs_info->tess._primitive_mode;
   tes_info->tess.ccw |= tcs_info->tess.ccw;
   tes_info->tess.point_mode |= tcs_info->tess.point_mode;
}

static void
anv_pipeline_link_tcs(const struct brw_compiler *compiler,
                      struct anv_pipeline_stage *tcs_stage,
                      struct anv_pipeline_stage *tes_stage)
{
   assert(tes_stage && tes_stage->stage == MESA_SHADER_TESS_EVAL);

   brw_nir_link_shaders(compiler, tcs_stage->nir, tes_stage->nir);

   nir_lower_patch_vertices(tes_stage->nir,
                            tcs_stage->nir->info.tess.tcs_vertices_out,
                            NULL);

   /* Copy TCS info into the TES info */
   merge_tess_info(&tes_stage->nir->info, &tcs_stage->nir->info);

   /* Whacking the key after cache lookup is a bit sketchy, but all of
    * this comes from the SPIR-V, which is part of the hash used for the
    * pipeline cache.  So it should be safe.
    */
   tcs_stage->key.tcs._tes_primitive_mode =
      tes_stage->nir->info.tess._primitive_mode;
   tcs_stage->key.tcs.quads_workaround =
      compiler->devinfo->ver < 9 &&
      tes_stage->nir->info.tess._primitive_mode == TESS_PRIMITIVE_QUADS &&
      tes_stage->nir->info.tess.spacing == TESS_SPACING_EQUAL;
}

static void
anv_pipeline_compile_tcs(const struct brw_compiler *compiler,
                         void *mem_ctx,
                         struct anv_device *device,
                         struct anv_pipeline_stage *tcs_stage,
                         struct anv_pipeline_stage *prev_stage)
{
   tcs_stage->key.tcs.outputs_written =
      tcs_stage->nir->info.outputs_written;
   tcs_stage->key.tcs.patch_outputs_written =
      tcs_stage->nir->info.patch_outputs_written;

   tcs_stage->num_stats = 1;

   struct brw_compile_tcs_params params = {
      .base = {
         .nir = tcs_stage->nir,
         .stats = tcs_stage->stats,
         .log_data = device,
         .mem_ctx = mem_ctx,
      },
      .key = &tcs_stage->key.tcs,
      .prog_data = &tcs_stage->prog_data.tcs,
   };

   tcs_stage->code = brw_compile_tcs(compiler, &params);
}

static void
anv_pipeline_link_tes(const struct brw_compiler *compiler,
                      struct anv_pipeline_stage *tes_stage,
                      struct anv_pipeline_stage *next_stage)
{
   if (next_stage)
      brw_nir_link_shaders(compiler, tes_stage->nir, next_stage->nir);
}

static void
anv_pipeline_compile_tes(const struct brw_compiler *compiler,
                         void *mem_ctx,
                         struct anv_device *device,
                         struct anv_pipeline_stage *tes_stage,
                         struct anv_pipeline_stage *tcs_stage)
{
   tes_stage->key.tes.inputs_read =
      tcs_stage->nir->info.outputs_written;
   tes_stage->key.tes.patch_inputs_read =
      tcs_stage->nir->info.patch_outputs_written;

   tes_stage->num_stats = 1;

   struct brw_compile_tes_params params = {
      .base = {
         .nir = tes_stage->nir,
         .stats = tes_stage->stats,
         .log_data = device,
         .mem_ctx = mem_ctx,
      },
      .key = &tes_stage->key.tes,
      .prog_data = &tes_stage->prog_data.tes,
      .input_vue_map = &tcs_stage->prog_data.tcs.base.vue_map,
   };

   tes_stage->code = brw_compile_tes(compiler, &params);
}

static void
anv_pipeline_link_gs(const struct brw_compiler *compiler,
                     struct anv_pipeline_stage *gs_stage,
                     struct anv_pipeline_stage *next_stage)
{
   if (next_stage)
      brw_nir_link_shaders(compiler, gs_stage->nir, next_stage->nir);
}

static void
anv_pipeline_compile_gs(const struct brw_compiler *compiler,
                        void *mem_ctx,
                        struct anv_device *device,
                        struct anv_pipeline_stage *gs_stage,
                        struct anv_pipeline_stage *prev_stage)
{
   brw_compute_vue_map(compiler->devinfo,
                       &gs_stage->prog_data.gs.base.vue_map,
                       gs_stage->nir->info.outputs_written,
                       gs_stage->nir->info.separate_shader, 1);

   gs_stage->num_stats = 1;

   struct brw_compile_gs_params params = {
      .base = {
         .nir = gs_stage->nir,
         .stats = gs_stage->stats,
         .log_data = device,
         .mem_ctx = mem_ctx,
      },
      .key = &gs_stage->key.gs,
      .prog_data = &gs_stage->prog_data.gs,
   };

   gs_stage->code = brw_compile_gs(compiler, &params);
}

static void
anv_pipeline_link_fs(const struct brw_compiler *compiler,
                     struct anv_pipeline_stage *stage,
                     const struct vk_render_pass_state *rp)
{
   /* Initially the valid outputs value is set to all possible render targets
    * valid (see populate_wm_prog_key()), before we look at the shader
    * variables. Here we look at the output variables of the shader an compute
    * a correct number of render target outputs.
    */
   stage->key.wm.color_outputs_valid = 0;
   nir_foreach_shader_out_variable_safe(var, stage->nir) {
      if (var->data.location < FRAG_RESULT_DATA0)
         continue;

      const unsigned rt = var->data.location - FRAG_RESULT_DATA0;
      const unsigned array_len =
         glsl_type_is_array(var->type) ? glsl_get_length(var->type) : 1;
      assert(rt + array_len <= MAX_RTS);

      stage->key.wm.color_outputs_valid |= BITFIELD_RANGE(rt, array_len);
   }
   stage->key.wm.color_outputs_valid &=
      (1u << rp->color_attachment_count) - 1;
   stage->key.wm.nr_color_regions =
      util_last_bit(stage->key.wm.color_outputs_valid);

   unsigned num_rt_bindings;
   struct anv_pipeline_binding rt_bindings[MAX_RTS];
   if (stage->key.wm.nr_color_regions > 0) {
      assert(stage->key.wm.nr_color_regions <= MAX_RTS);
      for (unsigned rt = 0; rt < stage->key.wm.nr_color_regions; rt++) {
         if (stage->key.wm.color_outputs_valid & BITFIELD_BIT(rt)) {
            rt_bindings[rt] = (struct anv_pipeline_binding) {
               .set = ANV_DESCRIPTOR_SET_COLOR_ATTACHMENTS,
               .index = rt,
            };
         } else {
            /* Setup a null render target */
            rt_bindings[rt] = (struct anv_pipeline_binding) {
               .set = ANV_DESCRIPTOR_SET_COLOR_ATTACHMENTS,
               .index = UINT32_MAX,
            };
         }
      }
      num_rt_bindings = stage->key.wm.nr_color_regions;
   } else {
      /* Setup a null render target */
      rt_bindings[0] = (struct anv_pipeline_binding) {
         .set = ANV_DESCRIPTOR_SET_COLOR_ATTACHMENTS,
         .index = UINT32_MAX,
      };
      num_rt_bindings = 1;
   }

   assert(num_rt_bindings <= MAX_RTS);
   assert(stage->bind_map.surface_count == 0);
   typed_memcpy(stage->bind_map.surface_to_descriptor,
                rt_bindings, num_rt_bindings);
   stage->bind_map.surface_count += num_rt_bindings;
}

static void
anv_pipeline_compile_fs(const struct brw_compiler *compiler,
                        void *mem_ctx,
                        struct anv_device *device,
                        struct anv_pipeline_stage *fs_stage,
                        struct anv_pipeline_stage *prev_stage)
{
   /* TODO: we could set this to 0 based on the information in nir_shader, but
    * we need this before we call spirv_to_nir.
    */
   assert(prev_stage);

   struct brw_compile_fs_params params = {
      .base = {
         .nir = fs_stage->nir,
         .stats = fs_stage->stats,
         .log_data = device,
         .mem_ctx = mem_ctx,
      },
      .key = &fs_stage->key.wm,
      .prog_data = &fs_stage->prog_data.wm,

      .allow_spilling = true,
   };

   fs_stage->key.wm.input_slots_valid =
      prev_stage->prog_data.vue.vue_map.slots_valid;

   fs_stage->code = brw_compile_fs(compiler, &params);

   fs_stage->num_stats = (uint32_t)fs_stage->prog_data.wm.dispatch_8 +
                         (uint32_t)fs_stage->prog_data.wm.dispatch_16 +
                         (uint32_t)fs_stage->prog_data.wm.dispatch_32;
}

static void
anv_pipeline_add_executable(struct anv_pipeline *pipeline,
                            struct anv_pipeline_stage *stage,
                            struct brw_compile_stats *stats,
                            uint32_t code_offset)
{
   char *nir = NULL;
   if (stage->nir &&
       (pipeline->flags &
        VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)) {
      nir = nir_shader_as_str(stage->nir, pipeline->mem_ctx);
   }

   char *disasm = NULL;
   if (stage->code &&
       (pipeline->flags &
        VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)) {
      char *stream_data = NULL;
      size_t stream_size = 0;
      FILE *stream = open_memstream(&stream_data, &stream_size);

      uint32_t push_size = 0;
      for (unsigned i = 0; i < 4; i++)
         push_size += stage->bind_map.push_ranges[i].length;
      if (push_size > 0) {
         fprintf(stream, "Push constant ranges:\n");
         for (unsigned i = 0; i < 4; i++) {
            if (stage->bind_map.push_ranges[i].length == 0)
               continue;

            fprintf(stream, "    RANGE%d (%dB): ", i,
                    stage->bind_map.push_ranges[i].length * 32);

            switch (stage->bind_map.push_ranges[i].set) {
            case ANV_DESCRIPTOR_SET_NULL:
               fprintf(stream, "NULL");
               break;

            case ANV_DESCRIPTOR_SET_PUSH_CONSTANTS:
               fprintf(stream, "Vulkan push constants and API params");
               break;

            case ANV_DESCRIPTOR_SET_DESCRIPTORS:
               fprintf(stream, "Descriptor buffer for set %d (start=%dB)",
                       stage->bind_map.push_ranges[i].index,
                       stage->bind_map.push_ranges[i].start * 32);
               break;

            case ANV_DESCRIPTOR_SET_NUM_WORK_GROUPS:
               unreachable("gl_NumWorkgroups is never pushed");

            case ANV_DESCRIPTOR_SET_SHADER_CONSTANTS:
               fprintf(stream, "Inline shader constant data (start=%dB)",
                       stage->bind_map.push_ranges[i].start * 32);
               break;

            case ANV_DESCRIPTOR_SET_COLOR_ATTACHMENTS:
               unreachable("Color attachments can't be pushed");

            default:
               fprintf(stream, "UBO (set=%d binding=%d start=%dB)",
                       stage->bind_map.push_ranges[i].set,
                       stage->bind_map.push_ranges[i].index,
                       stage->bind_map.push_ranges[i].start * 32);
               break;
            }
            fprintf(stream, "\n");
         }
         fprintf(stream, "\n");
      }

      /* Creating this is far cheaper than it looks.  It's perfectly fine to
       * do it for every binary.
       */
      intel_disassemble(&pipeline->device->physical->compiler->isa,
                        stage->code, code_offset, stream);

      fclose(stream);

      /* Copy it to a ralloc'd thing */
      disasm = ralloc_size(pipeline->mem_ctx, stream_size + 1);
      memcpy(disasm, stream_data, stream_size);
      disasm[stream_size] = 0;

      free(stream_data);
   }

   const struct anv_pipeline_executable exe = {
      .stage = stage->stage,
      .stats = *stats,
      .nir = nir,
      .disasm = disasm,
   };
   util_dynarray_append(&pipeline->executables,
                        struct anv_pipeline_executable, exe);
}

static void
anv_pipeline_add_executables(struct anv_pipeline *pipeline,
                             struct anv_pipeline_stage *stage,
                             struct anv_shader_bin *bin)
{
   if (stage->stage == MESA_SHADER_FRAGMENT) {
      /* We pull the prog data and stats out of the anv_shader_bin because
       * the anv_pipeline_stage may not be fully populated if we successfully
       * looked up the shader in a cache.
       */
      const struct brw_wm_prog_data *wm_prog_data =
         (const struct brw_wm_prog_data *)bin->prog_data;
      struct brw_compile_stats *stats = bin->stats;

      if (wm_prog_data->dispatch_8) {
         anv_pipeline_add_executable(pipeline, stage, stats++, 0);
      }

      if (wm_prog_data->dispatch_16) {
         anv_pipeline_add_executable(pipeline, stage, stats++,
                                     wm_prog_data->prog_offset_16);
      }

      if (wm_prog_data->dispatch_32) {
         anv_pipeline_add_executable(pipeline, stage, stats++,
                                     wm_prog_data->prog_offset_32);
      }
   } else {
      anv_pipeline_add_executable(pipeline, stage, bin->stats, 0);
   }
}

static enum brw_robustness_flags
anv_device_get_robust_flags(const struct anv_device *device)
{
   return device->robust_buffer_access ?
          (BRW_ROBUSTNESS_UBO | BRW_ROBUSTNESS_SSBO) : 0;
}

static void
anv_graphics_pipeline_init_keys(struct anv_graphics_pipeline *pipeline,
                                const struct vk_graphics_pipeline_state *state,
                                struct anv_pipeline_stage *stages)
{
   for (uint32_t s = 0; s < ANV_GRAPHICS_SHADER_STAGE_COUNT; s++) {
      if (!stages[s].info)
         continue;

      int64_t stage_start = os_time_get_nano();

      vk_pipeline_hash_shader_stage(stages[s].info, NULL, stages[s].shader_sha1);

      const struct anv_device *device = pipeline->base.device;
      enum brw_robustness_flags robust_flags = anv_device_get_robust_flags(device);
      switch (stages[s].stage) {
      case MESA_SHADER_VERTEX:
         populate_vs_prog_key(device,
                              robust_flags,
                              &stages[s].key.vs);
         break;
      case MESA_SHADER_TESS_CTRL:
         populate_tcs_prog_key(device,
                               robust_flags,
                               state->ts->patch_control_points,
                               &stages[s].key.tcs);
         break;
      case MESA_SHADER_TESS_EVAL:
         populate_tes_prog_key(device,
                               robust_flags,
                               &stages[s].key.tes);
         break;
      case MESA_SHADER_GEOMETRY:
         populate_gs_prog_key(device,
                              robust_flags,
                              &stages[s].key.gs);
         break;
      case MESA_SHADER_FRAGMENT: {
         populate_wm_prog_key(pipeline,
                              robust_flags,
                              state->dynamic, state->ms, state->rp,
                              &stages[s].key.wm);
         break;
      }
      default:
         unreachable("Invalid graphics shader stage");
      }

      stages[s].feedback.duration += os_time_get_nano() - stage_start;
      stages[s].feedback.flags |= VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT;
   }

   assert(pipeline->active_stages & VK_SHADER_STAGE_VERTEX_BIT);
}

static bool
anv_graphics_pipeline_load_cached_shaders(struct anv_graphics_pipeline *pipeline,
                                          struct vk_pipeline_cache *cache,
                                          struct anv_pipeline_stage *stages,
                                          VkPipelineCreationFeedback *pipeline_feedback)
{
   unsigned found = 0;
   unsigned cache_hits = 0;
   for (unsigned s = 0; s < ANV_GRAPHICS_SHADER_STAGE_COUNT; s++) {
      if (!stages[s].info)
         continue;

      int64_t stage_start = os_time_get_nano();

      bool cache_hit;
      struct anv_shader_bin *bin =
         anv_device_search_for_kernel(pipeline->base.device, cache,
                                      &stages[s].cache_key,
                                      sizeof(stages[s].cache_key), &cache_hit);
      if (bin) {
         found++;
         pipeline->shaders[s] = bin;
      }

      if (cache_hit) {
         cache_hits++;
         stages[s].feedback.flags |=
            VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
      }
      stages[s].feedback.duration += os_time_get_nano() - stage_start;
   }

   if (found == __builtin_popcount(pipeline->active_stages)) {
      if (cache_hits == found) {
         pipeline_feedback->flags |=
            VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
      }
      /* We found all our shaders in the cache.  We're done. */
      for (unsigned s = 0; s < ARRAY_SIZE(pipeline->shaders); s++) {
         if (!stages[s].info)
            continue;

         anv_pipeline_add_executables(&pipeline->base, &stages[s],
                                      pipeline->shaders[s]);
      }
      return true;
   } else if (found > 0) {
      /* We found some but not all of our shaders. This shouldn't happen most
       * of the time but it can if we have a partially populated pipeline
       * cache.
       */
      assert(found < __builtin_popcount(pipeline->active_stages));

      vk_perf(VK_LOG_OBJS(cache ? &cache->base :
                                  &pipeline->base.device->vk.base),
              "Found a partial pipeline in the cache.  This is "
              "most likely caused by an incomplete pipeline cache "
              "import or export");

      /* We're going to have to recompile anyway, so just throw away our
       * references to the shaders in the cache.  We'll get them out of the
       * cache again as part of the compilation process.
       */
      for (unsigned s = 0; s < ARRAY_SIZE(pipeline->shaders); s++) {
         stages[s].feedback.flags = 0;
         if (pipeline->shaders[s]) {
            anv_shader_bin_unref(pipeline->base.device, pipeline->shaders[s]);
            pipeline->shaders[s] = NULL;
         }
      }
   }

   return false;
}

static const gl_shader_stage graphics_shader_order[] = {
   MESA_SHADER_VERTEX,
   MESA_SHADER_TESS_CTRL,
   MESA_SHADER_TESS_EVAL,
   MESA_SHADER_GEOMETRY,

   MESA_SHADER_FRAGMENT,
};

static VkResult
anv_graphics_pipeline_load_nir(struct anv_graphics_pipeline *pipeline,
                               struct vk_pipeline_cache *cache,
                               struct anv_pipeline_stage *stages,
                               void *pipeline_ctx)
{
   for (unsigned i = 0; i < ARRAY_SIZE(graphics_shader_order); i++) {
      gl_shader_stage s = graphics_shader_order[i];
      if (!stages[s].info)
         continue;

      int64_t stage_start = os_time_get_nano();

      assert(stages[s].stage == s);
      assert(pipeline->shaders[s] == NULL);

      stages[s].bind_map = (struct anv_pipeline_bind_map) {
         .surface_to_descriptor = stages[s].surface_to_descriptor,
         .sampler_to_descriptor = stages[s].sampler_to_descriptor
      };

      stages[s].nir = anv_pipeline_stage_get_nir(&pipeline->base, cache,
                                                 pipeline_ctx,
                                                 &stages[s]);
      if (stages[s].nir == NULL) {
         return vk_error(pipeline, VK_ERROR_UNKNOWN);
      }

      stages[s].feedback.duration += os_time_get_nano() - stage_start;
   }

   return VK_SUCCESS;
}

static VkResult
anv_graphics_pipeline_compile(struct anv_graphics_pipeline *pipeline,
                              struct vk_pipeline_cache *cache,
                              const VkGraphicsPipelineCreateInfo *info,
                              const struct vk_graphics_pipeline_state *state)
{
   ANV_FROM_HANDLE(anv_pipeline_layout, layout, info->layout);
   VkResult result;

   VkPipelineCreationFeedback pipeline_feedback = {
      .flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT,
   };
   int64_t pipeline_start = os_time_get_nano();

   const struct brw_compiler *compiler = pipeline->base.device->physical->compiler;
   struct anv_pipeline_stage stages[ANV_GRAPHICS_SHADER_STAGE_COUNT] = {};
   for (uint32_t i = 0; i < info->stageCount; i++) {
      gl_shader_stage stage = vk_to_mesa_shader_stage(info->pStages[i].stage);
      stages[stage].stage = stage;
      stages[stage].info = &info->pStages[i];
   }

   anv_graphics_pipeline_init_keys(pipeline, state, stages);

   unsigned char sha1[20];
   anv_pipeline_hash_graphics(pipeline, layout, stages, sha1);

   for (unsigned s = 0; s < ARRAY_SIZE(stages); s++) {
      if (!stages[s].info)
         continue;

      stages[s].cache_key.stage = s;
      memcpy(stages[s].cache_key.sha1, sha1, sizeof(sha1));
   }

   const bool skip_cache_lookup =
      (pipeline->base.flags & VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR);
   if (!skip_cache_lookup) {
      bool found_all_shaders =
         anv_graphics_pipeline_load_cached_shaders(pipeline, cache, stages,
                                                   &pipeline_feedback);
      if (found_all_shaders)
         goto done;
   }

   if (info->flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT)
      return VK_PIPELINE_COMPILE_REQUIRED;

   void *pipeline_ctx = ralloc_context(NULL);

   result = anv_graphics_pipeline_load_nir(pipeline, cache, stages,
                                           pipeline_ctx);
   if (result != VK_SUCCESS)
      goto fail;

   /* Walk backwards to link */
   struct anv_pipeline_stage *next_stage = NULL;
   for (int i = ARRAY_SIZE(graphics_shader_order) - 1; i >= 0; i--) {
      gl_shader_stage s = graphics_shader_order[i];
      if (!stages[s].info)
         continue;

      switch (s) {
      case MESA_SHADER_VERTEX:
         anv_pipeline_link_vs(compiler, &stages[s], next_stage);
         break;
      case MESA_SHADER_TESS_CTRL:
         anv_pipeline_link_tcs(compiler, &stages[s], next_stage);
         break;
      case MESA_SHADER_TESS_EVAL:
         anv_pipeline_link_tes(compiler, &stages[s], next_stage);
         break;
      case MESA_SHADER_GEOMETRY:
         anv_pipeline_link_gs(compiler, &stages[s], next_stage);
         break;
      case MESA_SHADER_FRAGMENT:
         anv_pipeline_link_fs(compiler, &stages[s], state->rp);
         break;
      default:
         unreachable("Invalid graphics shader stage");
      }

      next_stage = &stages[s];
   }

   struct anv_pipeline_stage *prev_stage = NULL;
   for (unsigned i = 0; i < ARRAY_SIZE(graphics_shader_order); i++) {
      gl_shader_stage s = graphics_shader_order[i];
      if (!stages[s].info)
         continue;

      int64_t stage_start = os_time_get_nano();

      void *stage_ctx = ralloc_context(NULL);

      anv_pipeline_lower_nir(&pipeline->base, stage_ctx, &stages[s], layout);

      if (prev_stage && compiler->nir_options[s]->unify_interfaces) {
         prev_stage->nir->info.outputs_written |= stages[s].nir->info.inputs_read &
                  ~(VARYING_BIT_TESS_LEVEL_INNER | VARYING_BIT_TESS_LEVEL_OUTER);
         stages[s].nir->info.inputs_read |= prev_stage->nir->info.outputs_written &
                  ~(VARYING_BIT_TESS_LEVEL_INNER | VARYING_BIT_TESS_LEVEL_OUTER);
         prev_stage->nir->info.patch_outputs_written |= stages[s].nir->info.patch_inputs_read;
         stages[s].nir->info.patch_inputs_read |= prev_stage->nir->info.patch_outputs_written;
      }

      ralloc_free(stage_ctx);

      stages[s].feedback.duration += os_time_get_nano() - stage_start;

      prev_stage = &stages[s];
   }

   prev_stage = NULL;
   for (unsigned i = 0; i < ARRAY_SIZE(graphics_shader_order); i++) {
      gl_shader_stage s = graphics_shader_order[i];
      if (!stages[s].info)
         continue;

      int64_t stage_start = os_time_get_nano();

      void *stage_ctx = ralloc_context(NULL);

      switch (s) {
      case MESA_SHADER_VERTEX:
         anv_pipeline_compile_vs(compiler, stage_ctx, pipeline,
                                 &stages[s]);
         break;
      case MESA_SHADER_TESS_CTRL:
         anv_pipeline_compile_tcs(compiler, stage_ctx, pipeline->base.device,
                                  &stages[s], prev_stage);
         break;
      case MESA_SHADER_TESS_EVAL:
         anv_pipeline_compile_tes(compiler, stage_ctx, pipeline->base.device,
                                  &stages[s], prev_stage);
         break;
      case MESA_SHADER_GEOMETRY:
         anv_pipeline_compile_gs(compiler, stage_ctx, pipeline->base.device,
                                 &stages[s], prev_stage);
         break;
      case MESA_SHADER_FRAGMENT:
         anv_pipeline_compile_fs(compiler, stage_ctx, pipeline->base.device,
                                 &stages[s], prev_stage);
         break;
      default:
         unreachable("Invalid graphics shader stage");
      }
      if (stages[s].code == NULL) {
         ralloc_free(stage_ctx);
         result = vk_error(pipeline->base.device, VK_ERROR_OUT_OF_HOST_MEMORY);
         goto fail;
      }

      anv_nir_validate_push_layout(&stages[s].prog_data.base,
                                   &stages[s].bind_map);

      struct anv_shader_bin *bin =
         anv_device_upload_kernel(pipeline->base.device, cache, s,
                                  &stages[s].cache_key,
                                  sizeof(stages[s].cache_key),
                                  stages[s].code,
                                  stages[s].prog_data.base.program_size,
                                  &stages[s].prog_data.base,
                                  brw_prog_data_size(s),
                                  stages[s].stats, stages[s].num_stats,
                                  stages[s].nir->xfb_info,
                                  &stages[s].bind_map);
      if (!bin) {
         ralloc_free(stage_ctx);
         result = vk_error(pipeline, VK_ERROR_OUT_OF_HOST_MEMORY);
         goto fail;
      }

      anv_pipeline_add_executables(&pipeline->base, &stages[s], bin);

      pipeline->shaders[s] = bin;
      ralloc_free(stage_ctx);

      stages[s].feedback.duration += os_time_get_nano() - stage_start;

      prev_stage = &stages[s];
   }

   ralloc_free(pipeline_ctx);

done:

   pipeline_feedback.duration = os_time_get_nano() - pipeline_start;

   const VkPipelineCreationFeedbackCreateInfo *create_feedback =
      vk_find_struct_const(info->pNext, PIPELINE_CREATION_FEEDBACK_CREATE_INFO);
   if (create_feedback) {
      *create_feedback->pPipelineCreationFeedback = pipeline_feedback;

      uint32_t stage_count = create_feedback->pipelineStageCreationFeedbackCount;
      assert(stage_count == 0 || info->stageCount == stage_count);
      for (uint32_t i = 0; i < stage_count; i++) {
         gl_shader_stage s = vk_to_mesa_shader_stage(info->pStages[i].stage);
         create_feedback->pPipelineStageCreationFeedbacks[i] = stages[s].feedback;
      }
   }

   return VK_SUCCESS;

fail:
   ralloc_free(pipeline_ctx);

   for (unsigned s = 0; s < ARRAY_SIZE(pipeline->shaders); s++) {
      if (pipeline->shaders[s])
         anv_shader_bin_unref(pipeline->base.device, pipeline->shaders[s]);
   }

   return result;
}

static VkResult
anv_pipeline_compile_cs(struct anv_compute_pipeline *pipeline,
                        struct vk_pipeline_cache *cache,
                        const VkComputePipelineCreateInfo *info)
{
   const VkPipelineShaderStageCreateInfo *sinfo = &info->stage;
   assert(sinfo->stage == VK_SHADER_STAGE_COMPUTE_BIT);

   VkPipelineCreationFeedback pipeline_feedback = {
      .flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT,
   };
   int64_t pipeline_start = os_time_get_nano();

   struct anv_device *device = pipeline->base.device;
   const struct brw_compiler *compiler = device->physical->compiler;

   struct anv_pipeline_stage stage = {
      .stage = MESA_SHADER_COMPUTE,
      .info = &info->stage,
      .cache_key = {
         .stage = MESA_SHADER_COMPUTE,
      },
      .feedback = {
         .flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT,
      },
   };
   vk_pipeline_hash_shader_stage(&info->stage, NULL, stage.shader_sha1);

   struct anv_shader_bin *bin = NULL;

   populate_cs_prog_key(device,
                        anv_device_get_robust_flags(device),
                        &stage.key.cs);

   ANV_FROM_HANDLE(anv_pipeline_layout, layout, info->layout);

   const bool skip_cache_lookup =
      (pipeline->base.flags & VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR);

   anv_pipeline_hash_compute(pipeline, layout, &stage, stage.cache_key.sha1);

   bool cache_hit = false;
   if (!skip_cache_lookup) {
      bin = anv_device_search_for_kernel(device, cache,
                                         &stage.cache_key,
                                         sizeof(stage.cache_key),
                                         &cache_hit);
   }

   if (bin == NULL &&
       (info->flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT))
      return VK_PIPELINE_COMPILE_REQUIRED;

   void *mem_ctx = ralloc_context(NULL);
   if (bin == NULL) {
      int64_t stage_start = os_time_get_nano();

      stage.bind_map = (struct anv_pipeline_bind_map) {
         .surface_to_descriptor = stage.surface_to_descriptor,
         .sampler_to_descriptor = stage.sampler_to_descriptor
      };

      /* Set up a binding for the gl_NumWorkGroups */
      stage.bind_map.surface_count = 1;
      stage.bind_map.surface_to_descriptor[0] = (struct anv_pipeline_binding) {
         .set = ANV_DESCRIPTOR_SET_NUM_WORK_GROUPS,
      };

      stage.nir = anv_pipeline_stage_get_nir(&pipeline->base, cache, mem_ctx, &stage);
      if (stage.nir == NULL) {
         ralloc_free(mem_ctx);
         return vk_error(pipeline, VK_ERROR_UNKNOWN);
      }

      anv_pipeline_lower_nir(&pipeline->base, mem_ctx, &stage, layout);

      unsigned local_size = stage.nir->info.workgroup_size[0] *
                            stage.nir->info.workgroup_size[1] *
                            stage.nir->info.workgroup_size[2];

      /* Games don't always request full subgroups when they should,
       * which can cause bugs, as they may expect bigger size of the
       * subgroup than we choose for the execution.
       */
      if (device->physical->instance->assume_full_subgroups &&
          stage.nir->info.uses_wide_subgroup_intrinsics &&
          stage.nir->info.subgroup_size == SUBGROUP_SIZE_API_CONSTANT &&
          local_size &&
          local_size % BRW_SUBGROUP_SIZE == 0)
         stage.nir->info.subgroup_size = SUBGROUP_SIZE_FULL_SUBGROUPS;

      /* If the client requests that we dispatch full subgroups but doesn't
       * allow us to pick a subgroup size, we have to smash it to the API
       * value of 32.  Performance will likely be terrible in this case but
       * there's nothing we can do about that.  The client should have chosen
       * a size.
       */
      if (stage.nir->info.subgroup_size == SUBGROUP_SIZE_FULL_SUBGROUPS)
         stage.nir->info.subgroup_size =
            device->physical->instance->assume_full_subgroups != 0 ?
            device->physical->instance->assume_full_subgroups : BRW_SUBGROUP_SIZE;

      stage.num_stats = 1;

      struct brw_compile_cs_params params = {
         .base = {
            .nir = stage.nir,
            .stats = stage.stats,
            .log_data = device,
            .mem_ctx = mem_ctx,
         },
         .key = &stage.key.cs,
         .prog_data = &stage.prog_data.cs,
      };

      stage.code = brw_compile_cs(compiler, &params);
      if (stage.code == NULL) {
         ralloc_free(mem_ctx);
         return vk_error(pipeline, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      anv_nir_validate_push_layout(&stage.prog_data.base, &stage.bind_map);

      if (!stage.prog_data.cs.uses_num_work_groups) {
         assert(stage.bind_map.surface_to_descriptor[0].set ==
                ANV_DESCRIPTOR_SET_NUM_WORK_GROUPS);
         stage.bind_map.surface_to_descriptor[0].set = ANV_DESCRIPTOR_SET_NULL;
      }

      const unsigned code_size = stage.prog_data.base.program_size;
      bin = anv_device_upload_kernel(device, cache,
                                     MESA_SHADER_COMPUTE,
                                     &stage.cache_key, sizeof(stage.cache_key),
                                     stage.code, code_size,
                                     &stage.prog_data.base,
                                     sizeof(stage.prog_data.cs),
                                     stage.stats, stage.num_stats,
                                     NULL, &stage.bind_map);
      if (!bin) {
         ralloc_free(mem_ctx);
         return vk_error(pipeline, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      stage.feedback.duration = os_time_get_nano() - stage_start;
   }

   anv_pipeline_add_executables(&pipeline->base, &stage, bin);

   ralloc_free(mem_ctx);

   if (cache_hit) {
      stage.feedback.flags |=
         VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
      pipeline_feedback.flags |=
         VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
   }
   pipeline_feedback.duration = os_time_get_nano() - pipeline_start;

   const VkPipelineCreationFeedbackCreateInfo *create_feedback =
      vk_find_struct_const(info->pNext, PIPELINE_CREATION_FEEDBACK_CREATE_INFO);
   if (create_feedback) {
      *create_feedback->pPipelineCreationFeedback = pipeline_feedback;

      if (create_feedback->pipelineStageCreationFeedbackCount) {
         assert(create_feedback->pipelineStageCreationFeedbackCount == 1);
         create_feedback->pPipelineStageCreationFeedbacks[0] = stage.feedback;
      }
   }

   pipeline->cs = bin;

   return VK_SUCCESS;
}

static VkResult
anv_compute_pipeline_create(struct anv_device *device,
                            struct vk_pipeline_cache *cache,
                            const VkComputePipelineCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator,
                            VkPipeline *pPipeline)
{
   struct anv_compute_pipeline *pipeline;
   VkResult result;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);

   pipeline = vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*pipeline), 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (pipeline == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = anv_pipeline_init(&pipeline->base, device,
                              ANV_PIPELINE_COMPUTE, pCreateInfo->flags,
                              pAllocator);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, pAllocator, pipeline);
      return result;
   }

   anv_batch_set_storage(&pipeline->base.batch, ANV_NULL_ADDRESS,
                         pipeline->batch_data, sizeof(pipeline->batch_data));

   result = anv_pipeline_compile_cs(pipeline, cache, pCreateInfo);
   if (result != VK_SUCCESS) {
      anv_pipeline_finish(&pipeline->base, device, pAllocator);
      vk_free2(&device->vk.alloc, pAllocator, pipeline);
      return result;
   }

   anv_genX(device->info, compute_pipeline_emit)(pipeline);

   *pPipeline = anv_pipeline_to_handle(&pipeline->base);

   return pipeline->base.batch.status;
}

VkResult anv_CreateComputePipelines(
    VkDevice                                    _device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    count,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(vk_pipeline_cache, pipeline_cache, pipelineCache);

   VkResult result = VK_SUCCESS;

   unsigned i;
   for (i = 0; i < count; i++) {
      VkResult res = anv_compute_pipeline_create(device, pipeline_cache,
                                                 &pCreateInfos[i],
                                                 pAllocator, &pPipelines[i]);

      if (res == VK_SUCCESS)
         continue;

      /* Bail out on the first error != VK_PIPELINE_COMPILE_REQUIRED as it
       * is not obvious what error should be report upon 2 different failures.
       * */
      result = res;
      if (res != VK_PIPELINE_COMPILE_REQUIRED)
         break;

      pPipelines[i] = VK_NULL_HANDLE;

      if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT)
         break;
   }

   for (; i < count; i++)
      pPipelines[i] = VK_NULL_HANDLE;

   return result;
}

/**
 * Calculate the desired L3 partitioning based on the current state of the
 * pipeline.  For now this simply returns the conservative defaults calculated
 * by get_default_l3_weights(), but we could probably do better by gathering
 * more statistics from the pipeline state (e.g. guess of expected URB usage
 * and bound surfaces), or by using feed-back from performance counters.
 */
void
anv_pipeline_setup_l3_config(struct anv_pipeline *pipeline, bool needs_slm)
{
   const struct intel_device_info *devinfo = pipeline->device->info;

   const struct intel_l3_weights w =
      intel_get_default_l3_weights(devinfo, true, needs_slm);

   pipeline->l3_config = intel_get_l3_config(devinfo, w);
}

static VkResult
anv_graphics_pipeline_init(struct anv_graphics_pipeline *pipeline,
                           struct anv_device *device,
                           struct vk_pipeline_cache *cache,
                           const struct VkGraphicsPipelineCreateInfo *pCreateInfo,
                           const struct vk_graphics_pipeline_state *state,
                           const VkAllocationCallbacks *alloc)
{
   VkResult result;

   result = anv_pipeline_init(&pipeline->base, device,
                              ANV_PIPELINE_GRAPHICS, pCreateInfo->flags,
                              alloc);
   if (result != VK_SUCCESS)
      return result;

   anv_batch_set_storage(&pipeline->base.batch, ANV_NULL_ADDRESS,
                         pipeline->batch_data, sizeof(pipeline->batch_data));

   pipeline->active_stages = 0;
   for (uint32_t i = 0; i < pCreateInfo->stageCount; i++)
      pipeline->active_stages |= pCreateInfo->pStages[i].stage;

   if (pipeline->active_stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
      pipeline->active_stages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

   pipeline->dynamic_state.ms.sample_locations = &pipeline->sample_locations;
   vk_dynamic_graphics_state_fill(&pipeline->dynamic_state, state);

   pipeline->depth_clamp_enable = state->rs->depth_clamp_enable;
   pipeline->depth_clip_enable =
      vk_rasterization_state_depth_clip_enable(state->rs);
   pipeline->view_mask = state->rp->view_mask;

   result = anv_graphics_pipeline_compile(pipeline, cache, pCreateInfo, state);
   if (result != VK_SUCCESS) {
      anv_pipeline_finish(&pipeline->base, device, alloc);
      return result;
   }

   anv_pipeline_setup_l3_config(&pipeline->base, false);

   const uint64_t inputs_read = get_vs_prog_data(pipeline)->inputs_read;

   u_foreach_bit(a, state->vi->attributes_valid) {
      if (inputs_read & BITFIELD64_BIT(VERT_ATTRIB_GENERIC0 + a))
         pipeline->vb_used |= BITFIELD64_BIT(state->vi->attributes[a].binding);
   }

   u_foreach_bit(b, state->vi->bindings_valid) {
      pipeline->vb[b].stride = state->vi->bindings[b].stride;
      pipeline->vb[b].instanced = state->vi->bindings[b].input_rate ==
         VK_VERTEX_INPUT_RATE_INSTANCE;
      pipeline->vb[b].instance_divisor = state->vi->bindings[b].divisor;
   }

   pipeline->instance_multiplier = 1;
   if (pipeline->view_mask)
      pipeline->instance_multiplier = util_bitcount(pipeline->view_mask);

   pipeline->negative_one_to_one =
      state->vp != NULL && state->vp->depth_clip_negative_one_to_one;

   /* Store line mode, polygon mode and rasterization samples, these are used
    * for dynamic primitive topology.
    */
   pipeline->polygon_mode = state->rs->polygon_mode;
   pipeline->rasterization_samples =
      state->ms != NULL ? state->ms->rasterization_samples : 1;
   pipeline->line_mode = state->rs->line.mode;
   if (pipeline->line_mode == VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT) {
      if (pipeline->rasterization_samples > 1) {
         pipeline->line_mode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT;
      } else {
         pipeline->line_mode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
      }
   }
   pipeline->patch_control_points =
      state->ts != NULL ? state->ts->patch_control_points : 0;

   /* Store the color write masks, to be merged with color write enable if
    * dynamic.
    */
   if (state->cb != NULL) {
      for (unsigned i = 0; i < state->cb->attachment_count; i++)
         pipeline->color_comp_writes[i] = state->cb->attachments[i].write_mask;
   }

   return VK_SUCCESS;
}

static VkResult
anv_graphics_pipeline_create(struct anv_device *device,
                             struct vk_pipeline_cache *cache,
                             const VkGraphicsPipelineCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkPipeline *pPipeline)
{
   struct anv_graphics_pipeline *pipeline;
   VkResult result;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);

   pipeline = vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*pipeline), 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (pipeline == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_graphics_pipeline_all_state all;
   struct vk_graphics_pipeline_state state = { };
   result = vk_graphics_pipeline_state_fill(&device->vk, &state, pCreateInfo,
                                            NULL /* driver_rp */,
                                            0 /* driver_rp_flags */,
                                            &all, NULL, 0, NULL);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, pAllocator, pipeline);
      return result;
   }

   result = anv_graphics_pipeline_init(pipeline, device, cache,
                                       pCreateInfo, &state, pAllocator);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, pAllocator, pipeline);
      return result;
   }

   anv_genX(device->info, graphics_pipeline_emit)(pipeline, &state);

   *pPipeline = anv_pipeline_to_handle(&pipeline->base);

   return pipeline->base.batch.status;
}

VkResult anv_CreateGraphicsPipelines(
    VkDevice                                    _device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    count,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(vk_pipeline_cache, pipeline_cache, pipelineCache);

   VkResult result = VK_SUCCESS;

   unsigned i;
   for (i = 0; i < count; i++) {
      VkResult res = anv_graphics_pipeline_create(device,
                                                  pipeline_cache,
                                                  &pCreateInfos[i],
                                                  pAllocator, &pPipelines[i]);

      if (res == VK_SUCCESS)
         continue;

      /* Bail out on the first error != VK_PIPELINE_COMPILE_REQUIRED as it
       * is not obvious what error should be report upon 2 different failures.
       * */
      result = res;
      if (res != VK_PIPELINE_COMPILE_REQUIRED)
         break;

      pPipelines[i] = VK_NULL_HANDLE;

      if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT)
         break;
   }

   for (; i < count; i++)
      pPipelines[i] = VK_NULL_HANDLE;

   return result;
}

#define WRITE_STR(field, ...) ({                               \
   memset(field, 0, sizeof(field));                            \
   UNUSED int i = snprintf(field, sizeof(field), __VA_ARGS__); \
   assert(i > 0 && i < sizeof(field));                         \
})

VkResult anv_GetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties)
{
   ANV_FROM_HANDLE(anv_pipeline, pipeline, pPipelineInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutablePropertiesKHR, out,
                          pProperties, pExecutableCount);

   util_dynarray_foreach (&pipeline->executables, struct anv_pipeline_executable, exe) {
      vk_outarray_append_typed(VkPipelineExecutablePropertiesKHR, &out, props) {
         gl_shader_stage stage = exe->stage;
         props->stages = mesa_to_vk_shader_stage(stage);

         unsigned simd_width = exe->stats.dispatch_width;
         if (stage == MESA_SHADER_FRAGMENT) {
            WRITE_STR(props->name, "%s%d %s",
                      simd_width ? "SIMD" : "vec",
                      simd_width ? simd_width : 4,
                      _mesa_shader_stage_to_string(stage));
         } else {
            WRITE_STR(props->name, "%s", _mesa_shader_stage_to_string(stage));
         }
         WRITE_STR(props->description, "%s%d %s shader",
                   simd_width ? "SIMD" : "vec",
                   simd_width ? simd_width : 4,
                   _mesa_shader_stage_to_string(stage));

         /* The compiler gives us a dispatch width of 0 for vec4 but Vulkan
          * wants a subgroup size of 1.
          */
         props->subgroupSize = MAX2(simd_width, 1);
      }
   }

   return vk_outarray_status(&out);
}

static const struct anv_pipeline_executable *
anv_pipeline_get_executable(struct anv_pipeline *pipeline, uint32_t index)
{
   assert(index < util_dynarray_num_elements(&pipeline->executables,
                                             struct anv_pipeline_executable));
   return util_dynarray_element(
      &pipeline->executables, struct anv_pipeline_executable, index);
}

VkResult anv_GetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics)
{
   ANV_FROM_HANDLE(anv_pipeline, pipeline, pExecutableInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutableStatisticKHR, out,
                          pStatistics, pStatisticCount);

   const struct anv_pipeline_executable *exe =
      anv_pipeline_get_executable(pipeline, pExecutableInfo->executableIndex);

   const struct brw_stage_prog_data *prog_data;
   switch (pipeline->type) {
   case ANV_PIPELINE_GRAPHICS: {
      prog_data = anv_pipeline_to_graphics(pipeline)->shaders[exe->stage]->prog_data;
      break;
   }
   case ANV_PIPELINE_COMPUTE: {
      prog_data = anv_pipeline_to_compute(pipeline)->cs->prog_data;
      break;
   }
   default:
      unreachable("invalid pipeline type");
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Instruction Count");
      WRITE_STR(stat->description,
                "Number of GEN instructions in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.instructions;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "SEND Count");
      WRITE_STR(stat->description,
                "Number of instructions in the final generated shader "
                "executable which access external units such as the "
                "constant cache or the sampler.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.sends;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Loop Count");
      WRITE_STR(stat->description,
                "Number of loops (not unrolled) in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.loops;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Cycle Count");
      WRITE_STR(stat->description,
                "Estimate of the number of EU cycles required to execute "
                "the final generated executable.  This is an estimate only "
                "and may vary greatly from actual run-time performance.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.cycles;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Spill Count");
      WRITE_STR(stat->description,
                "Number of scratch spill operations.  This gives a rough "
                "estimate of the cost incurred due to spilling temporary "
                "values to memory.  If this is non-zero, you may want to "
                "adjust your shader to reduce register pressure.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.spills;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Fill Count");
      WRITE_STR(stat->description,
                "Number of scratch fill operations.  This gives a rough "
                "estimate of the cost incurred due to spilling temporary "
                "values to memory.  If this is non-zero, you may want to "
                "adjust your shader to reduce register pressure.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.fills;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Scratch Memory Size");
      WRITE_STR(stat->description,
                "Number of bytes of scratch memory required by the "
                "generated shader executable.  If this is non-zero, you "
                "may want to adjust your shader to reduce register "
                "pressure.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = prog_data->total_scratch;
   }

   if (gl_shader_stage_uses_workgroup(exe->stage)) {
      vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
         WRITE_STR(stat->name, "Workgroup Memory Size");
         WRITE_STR(stat->description,
                   "Number of bytes of workgroup shared memory used by this "
                   "shader including any padding.");
         stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
         stat->value.u64 = prog_data->total_shared;
      }
   }

   return vk_outarray_status(&out);
}

static bool
write_ir_text(VkPipelineExecutableInternalRepresentationKHR* ir,
              const char *data)
{
   ir->isText = VK_TRUE;

   size_t data_len = strlen(data) + 1;

   if (ir->pData == NULL) {
      ir->dataSize = data_len;
      return true;
   }

   strncpy(ir->pData, data, ir->dataSize);
   if (ir->dataSize < data_len)
      return false;

   ir->dataSize = data_len;
   return true;
}

VkResult anv_GetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations)
{
   ANV_FROM_HANDLE(anv_pipeline, pipeline, pExecutableInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutableInternalRepresentationKHR, out,
                          pInternalRepresentations, pInternalRepresentationCount);
   bool incomplete_text = false;

   const struct anv_pipeline_executable *exe =
      anv_pipeline_get_executable(pipeline, pExecutableInfo->executableIndex);

   if (exe->nir) {
      vk_outarray_append_typed(VkPipelineExecutableInternalRepresentationKHR, &out, ir) {
         WRITE_STR(ir->name, "Final NIR");
         WRITE_STR(ir->description,
                   "Final NIR before going into the back-end compiler");

         if (!write_ir_text(ir, exe->nir))
            incomplete_text = true;
      }
   }

   if (exe->disasm) {
      vk_outarray_append_typed(VkPipelineExecutableInternalRepresentationKHR, &out, ir) {
         WRITE_STR(ir->name, "GEN Assembly");
         WRITE_STR(ir->description,
                   "Final GEN assembly for the generated shader binary");

         if (!write_ir_text(ir, exe->disasm))
            incomplete_text = true;
      }
   }

   return incomplete_text ? VK_INCOMPLETE : vk_outarray_status(&out);
}
