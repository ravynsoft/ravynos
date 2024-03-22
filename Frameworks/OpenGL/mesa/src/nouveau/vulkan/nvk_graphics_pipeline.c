/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_pipeline.h"

#include "nvk_cmd_buffer.h"
#include "nvk_device.h"
#include "nvk_mme.h"
#include "nvk_physical_device.h"
#include "nvk_shader.h"

#include "vk_nir.h"
#include "vk_pipeline.h"
#include "vk_pipeline_layout.h"

#include "nv_push.h"

#include "nouveau_context.h"

#include "compiler/spirv/nir_spirv.h"

#include "nvk_cl9097.h"
#include "nvk_clb197.h"
#include "nvk_clc397.h"

static void
nvk_populate_fs_key(struct nak_fs_key *key,
                    const struct vk_multisample_state *ms,
                    const struct vk_graphics_pipeline_state *state)
{
   memset(key, 0, sizeof(*key));

   key->sample_locations_cb = 0;
   key->sample_locations_offset = nvk_root_descriptor_offset(draw.sample_locations);

   if (state->pipeline_flags &
       VK_PIPELINE_CREATE_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)
      key->zs_self_dep = true;

   if (ms == NULL || ms->rasterization_samples <= 1)
      return;

   if (ms->sample_shading_enable &&
       (ms->rasterization_samples * ms->min_sample_shading) > 1.0)
      key->force_sample_shading = true;
}

static void
emit_pipeline_ms_state(struct nv_push *p,
                       const struct vk_multisample_state *ms,
                       bool force_max_samples)
{
   const float min_sample_shading = force_max_samples ? 1 :
      (ms->sample_shading_enable ? CLAMP(ms->min_sample_shading, 0, 1) : 0);
   uint32_t min_samples = ceilf(ms->rasterization_samples * min_sample_shading);
   min_samples = util_next_power_of_two(MAX2(1, min_samples));

   P_IMMD(p, NV9097, SET_HYBRID_ANTI_ALIAS_CONTROL, {
      .passes = min_samples,
      .centroid = min_samples > 1 ? CENTROID_PER_PASS : CENTROID_PER_FRAGMENT,
   });
}

static void
emit_pipeline_ct_write_state(struct nv_push *p,
                             const struct vk_color_blend_state *cb,
                             const struct vk_render_pass_state *rp)
{
   uint32_t write_mask = 0;
   uint32_t att_count = 0;

   if (rp != NULL) {
      att_count = rp->color_attachment_count;
      for (uint32_t a = 0; a < rp->color_attachment_count; a++) {
         VkFormat att_format = rp->color_attachment_formats[a];
         if (att_format != VK_FORMAT_UNDEFINED)
            write_mask |= 0xf << (4 * a);
      }
   }

   P_IMMD(p, NV9097, SET_MME_SHADOW_SCRATCH(NVK_MME_SCRATCH_WRITE_MASK_PIPELINE),
          write_mask);

   P_1INC(p, NV9097, CALL_MME_MACRO(NVK_MME_SET_WRITE_MASK));
   P_INLINE_DATA(p, att_count);
}

static void
emit_pipeline_xfb_state(struct nv_push *p, const struct nak_xfb_info *xfb)
{
   for (uint8_t b = 0; b < ARRAY_SIZE(xfb->attr_count); b++) {
      const uint8_t attr_count = xfb->attr_count[b];
      P_MTHD(p, NV9097, SET_STREAM_OUT_CONTROL_STREAM(b));
      P_NV9097_SET_STREAM_OUT_CONTROL_STREAM(p, b, xfb->stream[b]);
      P_NV9097_SET_STREAM_OUT_CONTROL_COMPONENT_COUNT(p, b, attr_count);
      P_NV9097_SET_STREAM_OUT_CONTROL_STRIDE(p, b, xfb->stride[b]);

      /* upload packed varying indices in multiples of 4 bytes */
      const uint32_t n = DIV_ROUND_UP(attr_count, 4);
      if (n > 0) {
         P_MTHD(p, NV9097, SET_STREAM_OUT_LAYOUT_SELECT(b, 0));
         P_INLINE_ARRAY(p, (const uint32_t*)xfb->attr_index[b], n);
      }
   }
}

static const uint32_t mesa_to_nv9097_shader_type[] = {
   [MESA_SHADER_VERTEX]    = NV9097_SET_PIPELINE_SHADER_TYPE_VERTEX,
   [MESA_SHADER_TESS_CTRL] = NV9097_SET_PIPELINE_SHADER_TYPE_TESSELLATION_INIT,
   [MESA_SHADER_TESS_EVAL] = NV9097_SET_PIPELINE_SHADER_TYPE_TESSELLATION,
   [MESA_SHADER_GEOMETRY]  = NV9097_SET_PIPELINE_SHADER_TYPE_GEOMETRY,
   [MESA_SHADER_FRAGMENT]  = NV9097_SET_PIPELINE_SHADER_TYPE_PIXEL,
};

static void
merge_tess_info(struct shader_info *tes_info, struct shader_info *tcs_info)
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
   assert(tcs_info->tess.tcs_vertices_out == 0 || tes_info->tess.tcs_vertices_out == 0 ||
          tcs_info->tess.tcs_vertices_out == tes_info->tess.tcs_vertices_out);
   tes_info->tess.tcs_vertices_out |= tcs_info->tess.tcs_vertices_out;

   assert(tcs_info->tess.spacing == TESS_SPACING_UNSPECIFIED ||
          tes_info->tess.spacing == TESS_SPACING_UNSPECIFIED ||
          tcs_info->tess.spacing == tes_info->tess.spacing);
   tes_info->tess.spacing |= tcs_info->tess.spacing;

   assert(tcs_info->tess._primitive_mode == TESS_PRIMITIVE_UNSPECIFIED ||
          tes_info->tess._primitive_mode == TESS_PRIMITIVE_UNSPECIFIED ||
          tcs_info->tess._primitive_mode == tes_info->tess._primitive_mode);
   tes_info->tess._primitive_mode |= tcs_info->tess._primitive_mode;
   tes_info->tess.ccw |= tcs_info->tess.ccw;
   tes_info->tess.point_mode |= tcs_info->tess.point_mode;

   /* Copy the merged info back to the TCS */
   tcs_info->tess.tcs_vertices_out = tes_info->tess.tcs_vertices_out;
   tcs_info->tess.spacing = tes_info->tess.spacing;
   tcs_info->tess._primitive_mode = tes_info->tess._primitive_mode;
   tcs_info->tess.ccw = tes_info->tess.ccw;
   tcs_info->tess.point_mode = tes_info->tess.point_mode;
}

VkResult
nvk_graphics_pipeline_create(struct nvk_device *dev,
                             struct vk_pipeline_cache *cache,
                             const VkGraphicsPipelineCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkPipeline *pPipeline)
{
   VK_FROM_HANDLE(vk_pipeline_layout, pipeline_layout, pCreateInfo->layout);
   struct nvk_graphics_pipeline *pipeline;
   VkResult result = VK_SUCCESS;

   pipeline = (void *)nvk_pipeline_zalloc(dev, NVK_PIPELINE_GRAPHICS,
                                          sizeof(*pipeline), pAllocator);
   if (pipeline == NULL)
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkPipelineCreateFlags2KHR pipeline_flags =
      vk_graphics_pipeline_create_flags(pCreateInfo);

   if (pipeline_flags &
       VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)
      cache = NULL;

   struct vk_graphics_pipeline_all_state all;
   struct vk_graphics_pipeline_state state = {};
   result = vk_graphics_pipeline_state_fill(&dev->vk, &state, pCreateInfo,
                                            NULL, 0, &all, NULL, 0, NULL);
   assert(result == VK_SUCCESS);

   VkPipelineCreationFeedbackEXT pipeline_feedback = {
      .flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT,
   };
   VkPipelineCreationFeedbackEXT stage_feedbacks[MESA_SHADER_STAGES] = { 0 };

   int64_t pipeline_start = os_time_get_nano();

   const VkPipelineCreationFeedbackCreateInfo *creation_feedback =
         vk_find_struct_const(pCreateInfo->pNext,
                              PIPELINE_CREATION_FEEDBACK_CREATE_INFO);

   const VkPipelineShaderStageCreateInfo *infos[MESA_SHADER_STAGES] = {};
   nir_shader *nir[MESA_SHADER_STAGES] = {};
   struct vk_pipeline_robustness_state robustness[MESA_SHADER_STAGES];

   struct vk_pipeline_cache_object *cache_objs[MESA_SHADER_STAGES] = {};

   struct nak_fs_key fs_key_tmp, *fs_key = NULL;
   nvk_populate_fs_key(&fs_key_tmp, state.ms, &state);
   fs_key = &fs_key_tmp;

   for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
      const VkPipelineShaderStageCreateInfo *sinfo = &pCreateInfo->pStages[i];
      gl_shader_stage stage = vk_to_mesa_shader_stage(sinfo->stage);
      infos[stage] = sinfo;
   }

   for (gl_shader_stage stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      const VkPipelineShaderStageCreateInfo *sinfo = infos[stage];
      if (sinfo == NULL)
         continue;

      vk_pipeline_robustness_state_fill(&dev->vk, &robustness[stage],
                                        pCreateInfo->pNext, sinfo->pNext);
   }

   for (gl_shader_stage stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      const VkPipelineShaderStageCreateInfo *sinfo = infos[stage];
      if (sinfo == NULL)
         continue;

      unsigned char sha1[SHA1_DIGEST_LENGTH];
      nvk_hash_shader(sha1, sinfo, &robustness[stage],
                      state.rp->view_mask != 0, pipeline_layout,
                      stage == MESA_SHADER_FRAGMENT ? fs_key : NULL);

      if (cache) {
         bool cache_hit = false;
         cache_objs[stage] = vk_pipeline_cache_lookup_object(cache, &sha1, sizeof(sha1),
                                                             &nvk_shader_ops, &cache_hit);
         pipeline->base.shaders[stage] =
            container_of(cache_objs[stage], struct nvk_shader, base);

         if (cache_hit && cache != dev->mem_cache)
            pipeline_feedback.flags |=
               VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
      }

      if (!cache_objs[stage] &&
          pCreateInfo->flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT) {
         result = VK_PIPELINE_COMPILE_REQUIRED;
         goto fail;
      }
   }

   for (gl_shader_stage stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      const VkPipelineShaderStageCreateInfo *sinfo = infos[stage];
      if (sinfo == NULL || cache_objs[stage])
         continue;

      result = nvk_shader_stage_to_nir(dev, sinfo, &robustness[stage],
                                       cache, NULL, &nir[stage]);
      if (result != VK_SUCCESS)
         goto fail;
   }

   if (nir[MESA_SHADER_TESS_CTRL] && nir[MESA_SHADER_TESS_EVAL]) {
      nir_lower_patch_vertices(nir[MESA_SHADER_TESS_EVAL], nir[MESA_SHADER_TESS_CTRL]->info.tess.tcs_vertices_out, NULL);
      merge_tess_info(&nir[MESA_SHADER_TESS_EVAL]->info, &nir[MESA_SHADER_TESS_CTRL]->info);
   }

   for (gl_shader_stage stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      const VkPipelineShaderStageCreateInfo *sinfo = infos[stage];
      if (sinfo == NULL)
         continue;

      if (!cache_objs[stage]) {
         int64_t stage_start = os_time_get_nano();

         unsigned char sha1[SHA1_DIGEST_LENGTH];
         nvk_hash_shader(sha1, sinfo, &robustness[stage],
                         state.rp->view_mask != 0, pipeline_layout,
                         stage == MESA_SHADER_FRAGMENT ? fs_key : NULL);

         struct nvk_shader *shader = nvk_shader_init(dev, sha1, SHA1_DIGEST_LENGTH);
         if(shader == NULL) {
            result = vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);
            goto fail;
         }

         nvk_lower_nir(dev, nir[stage], &robustness[stage],
                       state.rp->view_mask != 0, pipeline_layout, shader);

         result = nvk_compile_nir(dev, nir[stage],
                                  pipeline_flags, &robustness[stage],
                                  stage == MESA_SHADER_FRAGMENT ? fs_key : NULL,
                                  cache, shader);

         if (result == VK_SUCCESS) {
            cache_objs[stage] = &shader->base;

            if (cache)
               cache_objs[stage] = vk_pipeline_cache_add_object(cache,
                                                                cache_objs[stage]);

            stage_feedbacks[stage].flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT;
            pipeline->base.shaders[stage] =
               container_of(cache_objs[stage], struct nvk_shader, base);
         }

         stage_feedbacks[stage].duration += os_time_get_nano() - stage_start;
         ralloc_free(nir[stage]);
      }

      if (result != VK_SUCCESS)
         goto fail;

      result = nvk_shader_upload(dev, pipeline->base.shaders[stage]);
      if (result != VK_SUCCESS)
         goto fail;
   }

   struct nv_push push;
   nv_push_init(&push, pipeline->push_data, ARRAY_SIZE(pipeline->push_data));
   struct nv_push *p = &push;

   bool force_max_samples = false;

   struct nvk_shader *last_geom = NULL;
   for (gl_shader_stage stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      struct nvk_shader *shader = pipeline->base.shaders[stage];
      uint32_t idx = mesa_to_nv9097_shader_type[stage];

      P_IMMD(p, NV9097, SET_PIPELINE_SHADER(idx), {
         .enable  = nvk_shader_is_enabled(shader),
         .type    = mesa_to_nv9097_shader_type[stage],
      });

      if (!nvk_shader_is_enabled(shader))
         continue;

      if (stage != MESA_SHADER_FRAGMENT)
         last_geom = shader;

      uint64_t addr = nvk_shader_address(shader);
      if (dev->pdev->info.cls_eng3d >= VOLTA_A) {
         P_MTHD(p, NVC397, SET_PIPELINE_PROGRAM_ADDRESS_A(idx));
         P_NVC397_SET_PIPELINE_PROGRAM_ADDRESS_A(p, idx, addr >> 32);
         P_NVC397_SET_PIPELINE_PROGRAM_ADDRESS_B(p, idx, addr);
      } else {
         assert(addr < 0xffffffff);
         P_IMMD(p, NV9097, SET_PIPELINE_PROGRAM(idx), addr);
      }

      P_MTHD(p, NVC397, SET_PIPELINE_REGISTER_COUNT(idx));
      P_NVC397_SET_PIPELINE_REGISTER_COUNT(p, idx, shader->info.num_gprs);
      P_NVC397_SET_PIPELINE_BINDING(p, idx, stage);

      switch (stage) {
      case MESA_SHADER_VERTEX:
      case MESA_SHADER_GEOMETRY:
      case MESA_SHADER_TESS_CTRL:
      case MESA_SHADER_TESS_EVAL:
         break;

      case MESA_SHADER_FRAGMENT:
         P_IMMD(p, NV9097, SET_SUBTILING_PERF_KNOB_A, {
            .fraction_of_spm_register_file_per_subtile         = 0x10,
            .fraction_of_spm_pixel_output_buffer_per_subtile   = 0x40,
            .fraction_of_spm_triangle_ram_per_subtile          = 0x16,
            .fraction_of_max_quads_per_subtile                 = 0x20,
         });
         P_NV9097_SET_SUBTILING_PERF_KNOB_B(p, 0x20);

         P_IMMD(p, NV9097, SET_API_MANDATED_EARLY_Z,
                shader->info.fs.early_fragment_tests);

         if (dev->pdev->info.cls_eng3d >= MAXWELL_B) {
            P_IMMD(p, NVB197, SET_POST_Z_PS_IMASK,
                   shader->info.fs.post_depth_coverage);
         } else {
            assert(!shader->info.fs.post_depth_coverage);
         }

         P_IMMD(p, NV9097, SET_ZCULL_BOUNDS, {
            .z_min_unbounded_enable = shader->info.fs.writes_depth,
            .z_max_unbounded_enable = shader->info.fs.writes_depth,
         });

         /* If we're using the incoming sample mask and doing sample shading,
          * we have to do sample shading "to the max", otherwise there's no
          * way to tell which sets of samples are covered by the current
          * invocation.
          */
         force_max_samples = shader->info.fs.reads_sample_mask ||
                             shader->info.fs.uses_sample_shading;
         break;

      default:
         unreachable("Unsupported shader stage");
      }
   }

   const uint8_t clip_cull = last_geom->info.vtg.clip_enable |
                             last_geom->info.vtg.cull_enable;
   if (clip_cull) {
      P_IMMD(p, NV9097, SET_USER_CLIP_ENABLE, {
         .plane0 = (clip_cull >> 0) & 1,
         .plane1 = (clip_cull >> 1) & 1,
         .plane2 = (clip_cull >> 2) & 1,
         .plane3 = (clip_cull >> 3) & 1,
         .plane4 = (clip_cull >> 4) & 1,
         .plane5 = (clip_cull >> 5) & 1,
         .plane6 = (clip_cull >> 6) & 1,
         .plane7 = (clip_cull >> 7) & 1,
      });
      P_IMMD(p, NV9097, SET_USER_CLIP_OP, {
         .plane0 = (last_geom->info.vtg.cull_enable >> 0) & 1,
         .plane1 = (last_geom->info.vtg.cull_enable >> 1) & 1,
         .plane2 = (last_geom->info.vtg.cull_enable >> 2) & 1,
         .plane3 = (last_geom->info.vtg.cull_enable >> 3) & 1,
         .plane4 = (last_geom->info.vtg.cull_enable >> 4) & 1,
         .plane5 = (last_geom->info.vtg.cull_enable >> 5) & 1,
         .plane6 = (last_geom->info.vtg.cull_enable >> 6) & 1,
         .plane7 = (last_geom->info.vtg.cull_enable >> 7) & 1,
      });
   }

   /* TODO: prog_selects_layer */
   P_IMMD(p, NV9097, SET_RT_LAYER, {
      .v       = 0,
      .control = last_geom->info.vtg.writes_layer ?
                 CONTROL_GEOMETRY_SHADER_SELECTS_LAYER :
                 CONTROL_V_SELECTS_LAYER,
   });

   emit_pipeline_xfb_state(&push, &last_geom->info.vtg.xfb);

   if (state.ms) emit_pipeline_ms_state(&push, state.ms, force_max_samples);
   emit_pipeline_ct_write_state(&push, state.cb, state.rp);

   pipeline->push_dw_count = nv_push_dw_count(&push);

   pipeline->dynamic.vi = &pipeline->_dynamic_vi;
   pipeline->dynamic.ms.sample_locations = &pipeline->_dynamic_sl;
   vk_dynamic_graphics_state_fill(&pipeline->dynamic, &state);

   pipeline_feedback.duration = os_time_get_nano() - pipeline_start;
   if (creation_feedback) {
      *creation_feedback->pPipelineCreationFeedback = pipeline_feedback;

      int fb_count = creation_feedback->pipelineStageCreationFeedbackCount;
      if (pCreateInfo->stageCount == fb_count) {
         for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
            const VkPipelineShaderStageCreateInfo *sinfo =
               &pCreateInfo->pStages[i];
            gl_shader_stage stage = vk_to_mesa_shader_stage(sinfo->stage);
            creation_feedback->pPipelineStageCreationFeedbacks[i] =
               stage_feedbacks[stage];
         }
      }
   }

   *pPipeline = nvk_pipeline_to_handle(&pipeline->base);

   return VK_SUCCESS;

fail:
   vk_object_free(&dev->vk, pAllocator, pipeline);
   return result;
}
