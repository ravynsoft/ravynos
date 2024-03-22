/*
 * Copyright © 2021 Raspberry Pi Ltd
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

#include "v3dv_private.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/compiler/v3d_compiler.h"

static uint8_t
blend_factor(VkBlendFactor factor, bool dst_alpha_one, bool *needs_constants)
{
   switch (factor) {
   case VK_BLEND_FACTOR_ZERO:
   case VK_BLEND_FACTOR_ONE:
   case VK_BLEND_FACTOR_SRC_COLOR:
   case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
   case VK_BLEND_FACTOR_DST_COLOR:
   case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
   case VK_BLEND_FACTOR_SRC_ALPHA:
   case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
   case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
      return factor;
   case VK_BLEND_FACTOR_CONSTANT_COLOR:
   case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
   case VK_BLEND_FACTOR_CONSTANT_ALPHA:
   case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
      *needs_constants = true;
      return factor;
   case VK_BLEND_FACTOR_DST_ALPHA:
      return dst_alpha_one ? V3D_BLEND_FACTOR_ONE :
                             V3D_BLEND_FACTOR_DST_ALPHA;
   case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
      return dst_alpha_one ? V3D_BLEND_FACTOR_ZERO :
                             V3D_BLEND_FACTOR_INV_DST_ALPHA;
   case VK_BLEND_FACTOR_SRC1_COLOR:
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
   case VK_BLEND_FACTOR_SRC1_ALPHA:
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
      unreachable("Invalid blend factor: dual source blending not supported.");
   default:
      unreachable("Unknown blend factor.");
   }
}

static void
pack_blend(struct v3dv_pipeline *pipeline,
           const VkPipelineColorBlendStateCreateInfo *cb_info)
{
   /* By default, we are not enabling blending and all color channel writes are
    * enabled. Color write enables are independent of whether blending is
    * enabled or not.
    *
    * Vulkan specifies color write masks so that bits set correspond to
    * enabled channels. Our hardware does it the other way around.
    */
   pipeline->blend.enables = 0;
   pipeline->blend.color_write_masks = 0; /* All channels enabled */

   if (!cb_info)
      return;

   assert(pipeline->subpass);
   if (pipeline->subpass->color_count == 0)
      return;

   assert(pipeline->subpass->color_count == cb_info->attachmentCount);
   pipeline->blend.needs_color_constants = false;
   uint32_t color_write_masks = 0;
   for (uint32_t i = 0; i < pipeline->subpass->color_count; i++) {
      const VkPipelineColorBlendAttachmentState *b_state =
         &cb_info->pAttachments[i];

      uint32_t attachment_idx =
         pipeline->subpass->color_attachments[i].attachment;
      if (attachment_idx == VK_ATTACHMENT_UNUSED)
         continue;

      color_write_masks |= (~b_state->colorWriteMask & 0xf) << (4 * i);

      if (!b_state->blendEnable)
         continue;

      VkAttachmentDescription2 *desc =
         &pipeline->pass->attachments[attachment_idx].desc;
      const struct v3dv_format *format = v3dX(get_format)(desc->format);

      /* We only do blending with render pass attachments, so we should not have
       * multiplanar images here
       */
      assert(format->plane_count == 1);
      bool dst_alpha_one = (format->planes[0].swizzle[3] == PIPE_SWIZZLE_1);

      uint8_t rt_mask = 1 << i;
      pipeline->blend.enables |= rt_mask;

      v3dvx_pack(pipeline->blend.cfg[i], BLEND_CFG, config) {
         config.render_target_mask = rt_mask;

         config.color_blend_mode = b_state->colorBlendOp;
         config.color_blend_dst_factor =
            blend_factor(b_state->dstColorBlendFactor, dst_alpha_one,
                         &pipeline->blend.needs_color_constants);
         config.color_blend_src_factor =
            blend_factor(b_state->srcColorBlendFactor, dst_alpha_one,
                         &pipeline->blend.needs_color_constants);

         config.alpha_blend_mode = b_state->alphaBlendOp;
         config.alpha_blend_dst_factor =
            blend_factor(b_state->dstAlphaBlendFactor, dst_alpha_one,
                         &pipeline->blend.needs_color_constants);
         config.alpha_blend_src_factor =
            blend_factor(b_state->srcAlphaBlendFactor, dst_alpha_one,
                         &pipeline->blend.needs_color_constants);
      }
   }

   pipeline->blend.color_write_masks = color_write_masks;
}

/* This requires that pack_blend() had been called before so we can set
 * the overall blend enable bit in the CFG_BITS packet.
 */
static void
pack_cfg_bits(struct v3dv_pipeline *pipeline,
              const VkPipelineDepthStencilStateCreateInfo *ds_info,
              const VkPipelineRasterizationStateCreateInfo *rs_info,
              const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *pv_info,
              const VkPipelineRasterizationLineStateCreateInfoEXT *ls_info,
              const VkPipelineMultisampleStateCreateInfo *ms_info)
{
   assert(sizeof(pipeline->cfg_bits) == cl_packet_length(CFG_BITS));

   pipeline->msaa =
      ms_info && ms_info->rasterizationSamples > VK_SAMPLE_COUNT_1_BIT;

   v3dvx_pack(pipeline->cfg_bits, CFG_BITS, config) {
      config.enable_forward_facing_primitive =
         rs_info ? !(rs_info->cullMode & VK_CULL_MODE_FRONT_BIT) : false;

      config.enable_reverse_facing_primitive =
         rs_info ? !(rs_info->cullMode & VK_CULL_MODE_BACK_BIT) : false;

      /* Seems like the hardware is backwards regarding this setting... */
      config.clockwise_primitives =
         rs_info ? rs_info->frontFace == VK_FRONT_FACE_COUNTER_CLOCKWISE : false;

      /* Even if rs_info->depthBiasEnabled is true, we can decide to not
       * enable it, like if there isn't a depth/stencil attachment with the
       * pipeline.
       */
      config.enable_depth_offset = pipeline->depth_bias.enabled;

      /* This is required to pass line rasterization tests in CTS while
       * exposing, at least, a minimum of 4-bits of subpixel precision
       * (the minimum requirement).
       */
      if (ls_info &&
          ls_info->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT)
         config.line_rasterization = V3D_LINE_RASTERIZATION_DIAMOND_EXIT;
      else
         config.line_rasterization = V3D_LINE_RASTERIZATION_PERP_END_CAPS;

      if (rs_info && rs_info->polygonMode != VK_POLYGON_MODE_FILL) {
         config.direct3d_wireframe_triangles_mode = true;
         config.direct3d_point_fill_mode =
            rs_info->polygonMode == VK_POLYGON_MODE_POINT;
      }

      /* diamond-exit rasterization does not support oversample */
      config.rasterizer_oversample_mode =
         (config.line_rasterization == V3D_LINE_RASTERIZATION_PERP_END_CAPS &&
          pipeline->msaa) ? 1 : 0;

      /* From the Vulkan spec:
       *
       *   "Provoking Vertex:
       *
       *       The vertex in a primitive from which flat shaded attribute
       *       values are taken. This is generally the “first” vertex in the
       *       primitive, and depends on the primitive topology."
       *
       * First vertex is the Direct3D style for provoking vertex. OpenGL uses
       * the last vertex by default.
       */
      if (pv_info) {
         config.direct3d_provoking_vertex =
            pv_info->provokingVertexMode ==
               VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT;
      } else {
         config.direct3d_provoking_vertex = true;
      }

      config.blend_enable = pipeline->blend.enables != 0;

      /* Disable depth/stencil if we don't have a D/S attachment */
      bool has_ds_attachment =
         pipeline->subpass->ds_attachment.attachment != VK_ATTACHMENT_UNUSED;

      if (ds_info && ds_info->depthTestEnable && has_ds_attachment) {
         config.z_updates_enable = ds_info->depthWriteEnable;
         config.depth_test_function = ds_info->depthCompareOp;
      } else {
         config.depth_test_function = VK_COMPARE_OP_ALWAYS;
      }

      config.stencil_enable =
         ds_info ? ds_info->stencilTestEnable && has_ds_attachment: false;

      pipeline->z_updates_enable = config.z_updates_enable;

#if V3D_VERSION >= 71
      /* From the Vulkan spec:
       *
       *    "depthClampEnable controls whether to clamp the fragment’s depth
       *     values as described in Depth Test. If the pipeline is not created
       *     with VkPipelineRasterizationDepthClipStateCreateInfoEXT present
       *     then enabling depth clamp will also disable clipping primitives to
       *     the z planes of the frustrum as described in Primitive Clipping.
       *     Otherwise depth clipping is controlled by the state set in
       *     VkPipelineRasterizationDepthClipStateCreateInfoEXT."
       *
       * Note: neither depth clamping nor VK_EXT_depth_clip_enable are actually
       * supported in the driver yet, so in practice we are always enabling Z
       * clipping for now.
       */
      bool z_clamp_enable = rs_info && rs_info->depthClampEnable;
      bool z_clip_enable = false;
      const VkPipelineRasterizationDepthClipStateCreateInfoEXT *clip_info =
         ds_info ? vk_find_struct_const(ds_info->pNext,
                                        PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT) :
                   NULL;
      if (clip_info)
         z_clip_enable = clip_info->depthClipEnable;
      else if (!z_clamp_enable)
         z_clip_enable = true;

      if (z_clip_enable) {
         config.z_clipping_mode = pipeline->negative_one_to_one ?
	    V3D_Z_CLIP_MODE_MIN_ONE_TO_ONE : V3D_Z_CLIP_MODE_ZERO_TO_ONE;
      } else {
         config.z_clipping_mode = V3D_Z_CLIP_MODE_NONE;
      }

      config.z_clamp_mode = z_clamp_enable;

      config.depth_bounds_test_enable =
              ds_info && ds_info->depthBoundsTestEnable && has_ds_attachment;
#endif
   };
}

static uint32_t
translate_stencil_op(VkStencilOp op)
{
   switch (op) {
   case VK_STENCIL_OP_KEEP:
      return V3D_STENCIL_OP_KEEP;
   case VK_STENCIL_OP_ZERO:
      return V3D_STENCIL_OP_ZERO;
   case VK_STENCIL_OP_REPLACE:
      return V3D_STENCIL_OP_REPLACE;
   case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
      return V3D_STENCIL_OP_INCR;
   case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
      return V3D_STENCIL_OP_DECR;
   case VK_STENCIL_OP_INVERT:
      return V3D_STENCIL_OP_INVERT;
   case VK_STENCIL_OP_INCREMENT_AND_WRAP:
      return V3D_STENCIL_OP_INCWRAP;
   case VK_STENCIL_OP_DECREMENT_AND_WRAP:
      return V3D_STENCIL_OP_DECWRAP;
   default:
      unreachable("bad stencil op");
   }
}

static void
pack_single_stencil_cfg(struct v3dv_pipeline *pipeline,
                        uint8_t *stencil_cfg,
                        bool is_front,
                        bool is_back,
                        const VkStencilOpState *stencil_state)
{
   /* From the Vulkan spec:
    *
    *   "Reference is an integer reference value that is used in the unsigned
    *    stencil comparison. The reference value used by stencil comparison
    *    must be within the range [0,2^s-1] , where s is the number of bits in
    *    the stencil framebuffer attachment, otherwise the reference value is
    *    considered undefined."
    *
    * In our case, 's' is always 8, so we clamp to that to prevent our packing
    * functions to assert in debug mode if they see larger values.
    *
    * If we have dynamic state we need to make sure we set the corresponding
    * state bits to 0, since cl_emit_with_prepacked ORs the new value with
    * the old.
    */
   const uint8_t write_mask =
      pipeline->dynamic_state.mask & V3DV_DYNAMIC_STENCIL_WRITE_MASK ?
         0 : stencil_state->writeMask & 0xff;

   const uint8_t compare_mask =
      pipeline->dynamic_state.mask & V3DV_DYNAMIC_STENCIL_COMPARE_MASK ?
         0 : stencil_state->compareMask & 0xff;

   const uint8_t reference =
      pipeline->dynamic_state.mask & V3DV_DYNAMIC_STENCIL_COMPARE_MASK ?
         0 : stencil_state->reference & 0xff;

   v3dvx_pack(stencil_cfg, STENCIL_CFG, config) {
      config.front_config = is_front;
      config.back_config = is_back;
      config.stencil_write_mask = write_mask;
      config.stencil_test_mask = compare_mask;
      config.stencil_test_function = stencil_state->compareOp;
      config.stencil_pass_op = translate_stencil_op(stencil_state->passOp);
      config.depth_test_fail_op = translate_stencil_op(stencil_state->depthFailOp);
      config.stencil_test_fail_op = translate_stencil_op(stencil_state->failOp);
      config.stencil_ref_value = reference;
   }
}

static void
pack_stencil_cfg(struct v3dv_pipeline *pipeline,
                 const VkPipelineDepthStencilStateCreateInfo *ds_info)
{
   assert(sizeof(pipeline->stencil_cfg) == 2 * cl_packet_length(STENCIL_CFG));

   if (!ds_info || !ds_info->stencilTestEnable)
      return;

   if (pipeline->subpass->ds_attachment.attachment == VK_ATTACHMENT_UNUSED)
      return;

   const uint32_t dynamic_stencil_states = V3DV_DYNAMIC_STENCIL_COMPARE_MASK |
                                           V3DV_DYNAMIC_STENCIL_WRITE_MASK |
                                           V3DV_DYNAMIC_STENCIL_REFERENCE;


   /* If front != back or we have dynamic stencil state we can't emit a single
    * packet for both faces.
    */
   bool needs_front_and_back = false;
   if ((pipeline->dynamic_state.mask & dynamic_stencil_states) ||
       memcmp(&ds_info->front, &ds_info->back, sizeof(ds_info->front)))
      needs_front_and_back = true;

   /* If the front and back configurations are the same we can emit both with
    * a single packet.
    */
   pipeline->emit_stencil_cfg[0] = true;
   if (!needs_front_and_back) {
      pack_single_stencil_cfg(pipeline, pipeline->stencil_cfg[0],
                              true, true, &ds_info->front);
   } else {
      pipeline->emit_stencil_cfg[1] = true;
      pack_single_stencil_cfg(pipeline, pipeline->stencil_cfg[0],
                              true, false, &ds_info->front);
      pack_single_stencil_cfg(pipeline, pipeline->stencil_cfg[1],
                              false, true, &ds_info->back);
   }
}

void
v3dX(pipeline_pack_state)(struct v3dv_pipeline *pipeline,
                          const VkPipelineColorBlendStateCreateInfo *cb_info,
                          const VkPipelineDepthStencilStateCreateInfo *ds_info,
                          const VkPipelineRasterizationStateCreateInfo *rs_info,
                          const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *pv_info,
                          const VkPipelineRasterizationLineStateCreateInfoEXT *ls_info,
                          const VkPipelineMultisampleStateCreateInfo *ms_info)
{
   pack_blend(pipeline, cb_info);
   pack_cfg_bits(pipeline, ds_info, rs_info, pv_info, ls_info, ms_info);
   pack_stencil_cfg(pipeline, ds_info);
}

static void
pack_shader_state_record(struct v3dv_pipeline *pipeline)
{
   assert(sizeof(pipeline->shader_state_record) >=
          cl_packet_length(GL_SHADER_STATE_RECORD));

   struct v3d_fs_prog_data *prog_data_fs =
      pipeline->shared_data->variants[BROADCOM_SHADER_FRAGMENT]->prog_data.fs;

   struct v3d_vs_prog_data *prog_data_vs =
      pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX]->prog_data.vs;

   struct v3d_vs_prog_data *prog_data_vs_bin =
      pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX_BIN]->prog_data.vs;


   /* Note: we are not packing addresses, as we need the job (see
    * cl_pack_emit_reloc). Additionally uniforms can't be filled up at this
    * point as they depend on dynamic info that can be set after create the
    * pipeline (like viewport), . Would need to be filled later, so we are
    * doing a partial prepacking.
    */
   v3dvx_pack(pipeline->shader_state_record, GL_SHADER_STATE_RECORD, shader) {
      shader.enable_clipping = true;

      if (!pipeline->has_gs) {
         shader.point_size_in_shaded_vertex_data =
            pipeline->topology == MESA_PRIM_POINTS;
      } else {
         struct v3d_gs_prog_data *prog_data_gs =
            pipeline->shared_data->variants[BROADCOM_SHADER_GEOMETRY]->prog_data.gs;
         shader.point_size_in_shaded_vertex_data = prog_data_gs->writes_psiz;
      }

      /* Must be set if the shader modifies Z, discards, or modifies
       * the sample mask.  For any of these cases, the fragment
       * shader needs to write the Z value (even just discards).
       */
      shader.fragment_shader_does_z_writes = prog_data_fs->writes_z;

      /* Set if the EZ test must be disabled (due to shader side
       * effects and the early_z flag not being present in the
       * shader).
       */
      shader.turn_off_early_z_test = prog_data_fs->disable_ez;

      shader.fragment_shader_uses_real_pixel_centre_w_in_addition_to_centroid_w2 =
         prog_data_fs->uses_center_w;

      /* The description for gl_SampleID states that if a fragment shader reads
       * it, then we should automatically activate per-sample shading. However,
       * the Vulkan spec also states that if a framebuffer has no attachments:
       *
       *    "The subpass continues to use the width, height, and layers of the
       *     framebuffer to define the dimensions of the rendering area, and the
       *     rasterizationSamples from each pipeline’s
       *     VkPipelineMultisampleStateCreateInfo to define the number of
       *     samples used in rasterization multisample rasterization."
       *
       * So in this scenario, if the pipeline doesn't enable multiple samples
       * but the fragment shader accesses gl_SampleID we would be requested
       * to do per-sample shading in single sample rasterization mode, which
       * is pointless, so just disable it in that case.
       */
      shader.enable_sample_rate_shading =
         pipeline->sample_rate_shading ||
         (pipeline->msaa && prog_data_fs->force_per_sample_msaa);

      shader.any_shader_reads_hardware_written_primitive_id = false;

      shader.do_scoreboard_wait_on_first_thread_switch =
         prog_data_fs->lock_scoreboard_on_first_thrsw;
      shader.disable_implicit_point_line_varyings =
         !prog_data_fs->uses_implicit_point_line_varyings;

      shader.number_of_varyings_in_fragment_shader =
         prog_data_fs->num_inputs;

      /* Note: see previous note about addresses */
      /* shader.coordinate_shader_code_address */
      /* shader.vertex_shader_code_address */
      /* shader.fragment_shader_code_address */

#if V3D_VERSION == 42
      shader.coordinate_shader_propagate_nans = true;
      shader.vertex_shader_propagate_nans = true;
      shader.fragment_shader_propagate_nans = true;

      /* FIXME: Use combined input/output size flag in the common case (also
       * on v3d, see v3dx_draw).
       */
      shader.coordinate_shader_has_separate_input_and_output_vpm_blocks =
         prog_data_vs_bin->separate_segments;
      shader.vertex_shader_has_separate_input_and_output_vpm_blocks =
         prog_data_vs->separate_segments;
      shader.coordinate_shader_input_vpm_segment_size =
         prog_data_vs_bin->separate_segments ?
         prog_data_vs_bin->vpm_input_size : 1;
      shader.vertex_shader_input_vpm_segment_size =
         prog_data_vs->separate_segments ?
         prog_data_vs->vpm_input_size : 1;
#endif

      /* On V3D 7.1 there isn't a specific flag to set if we are using
       * shared/separate segments or not. We just set the value of
       * vpm_input_size to 0, and set output to the max needed. That should be
       * already properly set on prog_data_vs_bin
       */
#if V3D_VERSION == 71
      shader.coordinate_shader_input_vpm_segment_size =
         prog_data_vs_bin->vpm_input_size;
      shader.vertex_shader_input_vpm_segment_size =
         prog_data_vs->vpm_input_size;
#endif

      shader.coordinate_shader_output_vpm_segment_size =
         prog_data_vs_bin->vpm_output_size;
      shader.vertex_shader_output_vpm_segment_size =
         prog_data_vs->vpm_output_size;

      /* Note: see previous note about addresses */
      /* shader.coordinate_shader_uniforms_address */
      /* shader.vertex_shader_uniforms_address */
      /* shader.fragment_shader_uniforms_address */

      shader.min_coord_shader_input_segments_required_in_play =
         pipeline->vpm_cfg_bin.As;
      shader.min_vertex_shader_input_segments_required_in_play =
         pipeline->vpm_cfg.As;

      shader.min_coord_shader_output_segments_required_in_play_in_addition_to_vcm_cache_size =
         pipeline->vpm_cfg_bin.Ve;
      shader.min_vertex_shader_output_segments_required_in_play_in_addition_to_vcm_cache_size =
         pipeline->vpm_cfg.Ve;

      shader.coordinate_shader_4_way_threadable =
         prog_data_vs_bin->base.threads == 4;
      shader.vertex_shader_4_way_threadable =
         prog_data_vs->base.threads == 4;
      shader.fragment_shader_4_way_threadable =
         prog_data_fs->base.threads == 4;

      shader.coordinate_shader_start_in_final_thread_section =
         prog_data_vs_bin->base.single_seg;
      shader.vertex_shader_start_in_final_thread_section =
         prog_data_vs->base.single_seg;
      shader.fragment_shader_start_in_final_thread_section =
         prog_data_fs->base.single_seg;

      shader.vertex_id_read_by_coordinate_shader =
         prog_data_vs_bin->uses_vid;
      shader.base_instance_id_read_by_coordinate_shader =
         prog_data_vs_bin->uses_biid;
      shader.instance_id_read_by_coordinate_shader =
         prog_data_vs_bin->uses_iid;
      shader.vertex_id_read_by_vertex_shader =
         prog_data_vs->uses_vid;
      shader.base_instance_id_read_by_vertex_shader =
         prog_data_vs->uses_biid;
      shader.instance_id_read_by_vertex_shader =
         prog_data_vs->uses_iid;

      /* Note: see previous note about addresses */
      /* shader.address_of_default_attribute_values */
   }
}

static void
pack_vcm_cache_size(struct v3dv_pipeline *pipeline)
{
   assert(sizeof(pipeline->vcm_cache_size) ==
          cl_packet_length(VCM_CACHE_SIZE));

   v3dvx_pack(pipeline->vcm_cache_size, VCM_CACHE_SIZE, vcm) {
      vcm.number_of_16_vertex_batches_for_binning = pipeline->vpm_cfg_bin.Vc;
      vcm.number_of_16_vertex_batches_for_rendering = pipeline->vpm_cfg.Vc;
   }
}

/* As defined on the GL_SHADER_STATE_ATTRIBUTE_RECORD */
static uint8_t
get_attr_type(const struct util_format_description *desc)
{
   uint32_t r_size = desc->channel[0].size;
   uint8_t attr_type = ATTRIBUTE_FLOAT;

   switch (desc->channel[0].type) {
   case UTIL_FORMAT_TYPE_FLOAT:
      if (r_size == 32) {
         attr_type = ATTRIBUTE_FLOAT;
      } else {
         assert(r_size == 16);
         attr_type = ATTRIBUTE_HALF_FLOAT;
      }
      break;

   case UTIL_FORMAT_TYPE_SIGNED:
   case UTIL_FORMAT_TYPE_UNSIGNED:
      switch (r_size) {
      case 32:
         attr_type = ATTRIBUTE_INT;
         break;
      case 16:
         attr_type = ATTRIBUTE_SHORT;
         break;
      case 10:
         attr_type = ATTRIBUTE_INT2_10_10_10;
         break;
      case 8:
         attr_type = ATTRIBUTE_BYTE;
         break;
      default:
         fprintf(stderr,
                 "format %s unsupported\n",
                 desc->name);
         attr_type = ATTRIBUTE_BYTE;
         abort();
      }
      break;

   default:
      fprintf(stderr,
              "format %s unsupported\n",
              desc->name);
      abort();
   }

   return attr_type;
}

static void
pack_shader_state_attribute_record(struct v3dv_pipeline *pipeline,
                                   uint32_t index,
                                   const VkVertexInputAttributeDescription *vi_desc)
{
   const uint32_t packet_length =
      cl_packet_length(GL_SHADER_STATE_ATTRIBUTE_RECORD);

   const struct util_format_description *desc =
      vk_format_description(vi_desc->format);

   uint32_t binding = vi_desc->binding;

   v3dvx_pack(&pipeline->vertex_attrs[index * packet_length],
             GL_SHADER_STATE_ATTRIBUTE_RECORD, attr) {

      /* vec_size == 0 means 4 */
      attr.vec_size = desc->nr_channels & 3;
      attr.signed_int_type = (desc->channel[0].type ==
                              UTIL_FORMAT_TYPE_SIGNED);
      attr.normalized_int_type = desc->channel[0].normalized;
      attr.read_as_int_uint = desc->channel[0].pure_integer;

      attr.instance_divisor = MIN2(pipeline->vb[binding].instance_divisor,
                                   0xffff);
      attr.stride = pipeline->vb[binding].stride;
      attr.type = get_attr_type(desc);
   }
}

void
v3dX(pipeline_pack_compile_state)(struct v3dv_pipeline *pipeline,
                                  const VkPipelineVertexInputStateCreateInfo *vi_info,
                                  const VkPipelineVertexInputDivisorStateCreateInfoEXT *vd_info)
{
   pack_shader_state_record(pipeline);
   pack_vcm_cache_size(pipeline);

   pipeline->vb_count = vi_info->vertexBindingDescriptionCount;
   for (uint32_t i = 0; i < vi_info->vertexBindingDescriptionCount; i++) {
      const VkVertexInputBindingDescription *desc =
         &vi_info->pVertexBindingDescriptions[i];

      pipeline->vb[desc->binding].stride = desc->stride;
      pipeline->vb[desc->binding].instance_divisor = desc->inputRate;
   }

   if (vd_info) {
      for (uint32_t i = 0; i < vd_info->vertexBindingDivisorCount; i++) {
         const VkVertexInputBindingDivisorDescriptionEXT *desc =
            &vd_info->pVertexBindingDivisors[i];

         pipeline->vb[desc->binding].instance_divisor = desc->divisor;
      }
   }

   pipeline->va_count = 0;
   struct v3d_vs_prog_data *prog_data_vs =
      pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX]->prog_data.vs;

   for (uint32_t i = 0; i < vi_info->vertexAttributeDescriptionCount; i++) {
      const VkVertexInputAttributeDescription *desc =
         &vi_info->pVertexAttributeDescriptions[i];
      uint32_t location = desc->location + VERT_ATTRIB_GENERIC0;

      /* We use a custom driver_location_map instead of
       * nir_find_variable_with_location because if we were able to get the
       * shader variant from the cache, we would not have the nir shader
       * available.
       */
      uint32_t driver_location =
         prog_data_vs->driver_location_map[location];

      if (driver_location != -1) {
         assert(driver_location < MAX_VERTEX_ATTRIBS);
         pipeline->va[driver_location].offset = desc->offset;
         pipeline->va[driver_location].binding = desc->binding;
         pipeline->va[driver_location].vk_format = desc->format;

         pack_shader_state_attribute_record(pipeline, driver_location, desc);

         pipeline->va_count++;
      }
   }
}

#if V3D_VERSION == 42
static bool
pipeline_has_integer_vertex_attrib(struct v3dv_pipeline *pipeline)
{
   for (uint8_t i = 0; i < pipeline->va_count; i++) {
      if (vk_format_is_int(pipeline->va[i].vk_format))
         return true;
   }
   return false;
}
#endif

bool
v3dX(pipeline_needs_default_attribute_values)(struct v3dv_pipeline *pipeline)
{
#if V3D_VERSION == 42
   return pipeline_has_integer_vertex_attrib(pipeline);
#endif

   return false;
}

/* @pipeline can be NULL. In that case we assume the most common case. For
 * example, for v42 we assume in that case that all the attributes have a
 * float format (we only create an all-float BO once and we reuse it with all
 * float pipelines), otherwise we look at the actual type of each attribute
 * used with the specific pipeline passed in.
 */
struct v3dv_bo *
v3dX(create_default_attribute_values)(struct v3dv_device *device,
                                      struct v3dv_pipeline *pipeline)
{
#if V3D_VERSION >= 71
   return NULL;
#endif

   uint32_t size = MAX_VERTEX_ATTRIBS * sizeof(float) * 4;
   struct v3dv_bo *bo;

   bo = v3dv_bo_alloc(device, size, "default_vi_attributes", true);

   if (!bo) {
      fprintf(stderr, "failed to allocate memory for the default "
              "attribute values\n");
      return NULL;
   }

   bool ok = v3dv_bo_map(device, bo, size);
   if (!ok) {
      fprintf(stderr, "failed to map default attribute values buffer\n");
      return NULL;
   }

   uint32_t *attrs = bo->map;
   uint8_t va_count = pipeline != NULL ? pipeline->va_count : 0;
   for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++) {
      attrs[i * 4 + 0] = 0;
      attrs[i * 4 + 1] = 0;
      attrs[i * 4 + 2] = 0;
      VkFormat attr_format =
         pipeline != NULL ? pipeline->va[i].vk_format : VK_FORMAT_UNDEFINED;
      if (i < va_count && vk_format_is_int(attr_format)) {
         attrs[i * 4 + 3] = 1;
      } else {
         attrs[i * 4 + 3] = fui(1.0);
      }
   }

   v3dv_bo_unmap(device, bo);

   return bo;
}
