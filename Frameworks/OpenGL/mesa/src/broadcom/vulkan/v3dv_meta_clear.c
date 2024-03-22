/*
 * Copyright © 2020 Raspberry Pi Ltd
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
#include "v3dv_meta_common.h"

#include "compiler/nir/nir_builder.h"
#include "util/u_pack_color.h"

static void
get_hw_clear_color(struct v3dv_device *device,
                   const VkClearColorValue *color,
                   VkFormat fb_format,
                   VkFormat image_format,
                   uint32_t internal_type,
                   uint32_t internal_bpp,
                   uint32_t *hw_color)
{
   const uint32_t internal_size = 4 << internal_bpp;

   /* If the image format doesn't match the framebuffer format, then we are
    * trying to clear an unsupported tlb format using a compatible
    * format for the framebuffer. In this case, we want to make sure that
    * we pack the clear value according to the original format semantics,
    * not the compatible format.
    */
   if (fb_format == image_format) {
      v3dv_X(device, get_hw_clear_color)(color, internal_type, internal_size,
                                         hw_color);
   } else {
      union util_color uc;
      enum pipe_format pipe_image_format =
         vk_format_to_pipe_format(image_format);
      util_pack_color(color->float32, pipe_image_format, &uc);
      memcpy(hw_color, uc.ui, internal_size);
   }
}

/* Returns true if the implementation is able to handle the case, false
 * otherwise.
*/
static bool
clear_image_tlb(struct v3dv_cmd_buffer *cmd_buffer,
                struct v3dv_image *image,
                const VkClearValue *clear_value,
                const VkImageSubresourceRange *range)
{
   const VkOffset3D origin = { 0, 0, 0 };
   VkFormat fb_format;

   /* From vkCmdClearColorImage spec:
    *  "image must not use any of the formats that require a sampler YCBCR
    *   conversion"
    */
   assert(image->plane_count == 1);
   if (!v3dv_meta_can_use_tlb(image, 0, 0, &origin, NULL, &fb_format))
      return false;

   uint32_t internal_type, internal_bpp;
   v3dv_X(cmd_buffer->device, get_internal_type_bpp_for_image_aspects)
      (fb_format, range->aspectMask,
       &internal_type, &internal_bpp);

   union v3dv_clear_value hw_clear_value = { 0 };
   if (range->aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
      get_hw_clear_color(cmd_buffer->device, &clear_value->color, fb_format,
                         image->vk.format, internal_type, internal_bpp,
                         &hw_clear_value.color[0]);
   } else {
      assert((range->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) ||
             (range->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT));
      hw_clear_value.z = clear_value->depthStencil.depth;
      hw_clear_value.s = clear_value->depthStencil.stencil;
   }

   uint32_t level_count = vk_image_subresource_level_count(&image->vk, range);
   uint32_t min_level = range->baseMipLevel;
   uint32_t max_level = range->baseMipLevel + level_count;

   /* For 3D images baseArrayLayer and layerCount must be 0 and 1 respectively.
    * Instead, we need to consider the full depth dimension of the image, which
    * goes from 0 up to the level's depth extent.
    */
   uint32_t min_layer;
   uint32_t max_layer;
   if (image->vk.image_type != VK_IMAGE_TYPE_3D) {
      min_layer = range->baseArrayLayer;
      max_layer = range->baseArrayLayer +
                  vk_image_subresource_layer_count(&image->vk, range);
   } else {
      min_layer = 0;
      max_layer = 0;
   }

   for (uint32_t level = min_level; level < max_level; level++) {
      if (image->vk.image_type == VK_IMAGE_TYPE_3D)
         max_layer = u_minify(image->vk.extent.depth, level);

      uint32_t width = u_minify(image->vk.extent.width, level);
      uint32_t height = u_minify(image->vk.extent.height, level);

      struct v3dv_job *job =
         v3dv_cmd_buffer_start_job(cmd_buffer, -1, V3DV_JOB_TYPE_GPU_CL);

      if (!job)
         return true;

      v3dv_job_start_frame(job, width, height, max_layer,
                           false, true, 1, internal_bpp,
                           4 * v3d_internal_bpp_words(internal_bpp),
                           image->vk.samples > VK_SAMPLE_COUNT_1_BIT);

      struct v3dv_meta_framebuffer framebuffer;
      v3dv_X(job->device, meta_framebuffer_init)(&framebuffer, fb_format,
                                                 internal_type,
                                                 &job->frame_tiling);

      v3dv_X(job->device, job_emit_binning_flush)(job);

      /* If this triggers it is an application bug: the spec requires
       * that any aspects to clear are present in the image.
       */
      assert(range->aspectMask & image->vk.aspects);

      v3dv_X(job->device, meta_emit_clear_image_rcl)
         (job, image, &framebuffer, &hw_clear_value,
          range->aspectMask, min_layer, max_layer, level);

      v3dv_cmd_buffer_finish_job(cmd_buffer);
   }

   return true;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdClearColorImage(VkCommandBuffer commandBuffer,
                        VkImage _image,
                        VkImageLayout imageLayout,
                        const VkClearColorValue *pColor,
                        uint32_t rangeCount,
                        const VkImageSubresourceRange *pRanges)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_image, image, _image);

   const VkClearValue clear_value = {
      .color = *pColor,
   };

   cmd_buffer->state.is_transfer = true;

   for (uint32_t i = 0; i < rangeCount; i++) {
      if (clear_image_tlb(cmd_buffer, image, &clear_value, &pRanges[i]))
         continue;
      unreachable("Unsupported color clear.");
   }

   cmd_buffer->state.is_transfer = false;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdClearDepthStencilImage(VkCommandBuffer commandBuffer,
                               VkImage _image,
                               VkImageLayout imageLayout,
                               const VkClearDepthStencilValue *pDepthStencil,
                               uint32_t rangeCount,
                               const VkImageSubresourceRange *pRanges)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_image, image, _image);

   const VkClearValue clear_value = {
      .depthStencil = *pDepthStencil,
   };

   cmd_buffer->state.is_transfer = true;

   for (uint32_t i = 0; i < rangeCount; i++) {
      if (clear_image_tlb(cmd_buffer, image, &clear_value, &pRanges[i]))
         continue;
      unreachable("Unsupported depth/stencil clear.");
   }

   cmd_buffer->state.is_transfer = false;
}

static void
destroy_color_clear_pipeline(VkDevice _device,
                             uint64_t pipeline,
                             VkAllocationCallbacks *alloc)
{
   struct v3dv_meta_color_clear_pipeline *p =
      (struct v3dv_meta_color_clear_pipeline *) (uintptr_t) pipeline;
   v3dv_DestroyPipeline(_device, p->pipeline, alloc);
   if (p->cached)
      v3dv_DestroyRenderPass(_device, p->pass, alloc);
   vk_free(alloc, p);
}

static void
destroy_depth_clear_pipeline(VkDevice _device,
                             struct v3dv_meta_depth_clear_pipeline *p,
                             VkAllocationCallbacks *alloc)
{
   v3dv_DestroyPipeline(_device, p->pipeline, alloc);
   vk_free(alloc, p);
}

static VkResult
create_color_clear_pipeline_layout(struct v3dv_device *device,
                                   VkPipelineLayout *pipeline_layout)
{
   /* FIXME: this is abusing a bit the API, since not all of our clear
    * pipelines have a geometry shader. We could create 2 different pipeline
    * layouts, but this works for us for now.
    */
   VkPushConstantRange ranges[2] = {
      { VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16 },
      { VK_SHADER_STAGE_GEOMETRY_BIT, 16, 4 },
   };

   VkPipelineLayoutCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 2,
      .pPushConstantRanges = ranges,
   };

   return v3dv_CreatePipelineLayout(v3dv_device_to_handle(device),
                                    &info, &device->vk.alloc, pipeline_layout);
}

static VkResult
create_depth_clear_pipeline_layout(struct v3dv_device *device,
                                   VkPipelineLayout *pipeline_layout)
{
   /* FIXME: this is abusing a bit the API, since not all of our clear
    * pipelines have a geometry shader. We could create 2 different pipeline
    * layouts, but this works for us for now.
    */
   VkPushConstantRange ranges[2] = {
      { VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4 },
      { VK_SHADER_STAGE_GEOMETRY_BIT, 4, 4 },
   };

   VkPipelineLayoutCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 2,
      .pPushConstantRanges = ranges
   };

   return v3dv_CreatePipelineLayout(v3dv_device_to_handle(device),
                                    &info, &device->vk.alloc, pipeline_layout);
}

void
v3dv_meta_clear_init(struct v3dv_device *device)
{
   device->meta.color_clear.cache =
      _mesa_hash_table_create(NULL, u64_hash, u64_compare);

   create_color_clear_pipeline_layout(device,
                                      &device->meta.color_clear.p_layout);

   device->meta.depth_clear.cache =
      _mesa_hash_table_create(NULL, u64_hash, u64_compare);

   create_depth_clear_pipeline_layout(device,
                                      &device->meta.depth_clear.p_layout);
}

void
v3dv_meta_clear_finish(struct v3dv_device *device)
{
   VkDevice _device = v3dv_device_to_handle(device);

   hash_table_foreach(device->meta.color_clear.cache, entry) {
      struct v3dv_meta_color_clear_pipeline *item = entry->data;
      destroy_color_clear_pipeline(_device, (uintptr_t)item, &device->vk.alloc);
   }
   _mesa_hash_table_destroy(device->meta.color_clear.cache, NULL);

   if (device->meta.color_clear.p_layout) {
      v3dv_DestroyPipelineLayout(_device, device->meta.color_clear.p_layout,
                                 &device->vk.alloc);
   }

   hash_table_foreach(device->meta.depth_clear.cache, entry) {
      struct v3dv_meta_depth_clear_pipeline *item = entry->data;
      destroy_depth_clear_pipeline(_device, item, &device->vk.alloc);
   }
   _mesa_hash_table_destroy(device->meta.depth_clear.cache, NULL);

   if (device->meta.depth_clear.p_layout) {
      v3dv_DestroyPipelineLayout(_device, device->meta.depth_clear.p_layout,
                                 &device->vk.alloc);
   }
}

static nir_shader *
get_clear_rect_vs()
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_VERTEX, options,
                                                  "meta clear vs");

   const struct glsl_type *vec4 = glsl_vec4_type();
   nir_variable *vs_out_pos =
      nir_variable_create(b.shader, nir_var_shader_out, vec4, "gl_Position");
   vs_out_pos->data.location = VARYING_SLOT_POS;

   nir_def *pos = nir_gen_rect_vertices(&b, NULL, NULL);
   nir_store_var(&b, vs_out_pos, pos, 0xf);

   return b.shader;
}

static nir_shader *
get_clear_rect_gs(uint32_t push_constant_layer_base)
{
   /* FIXME: this creates a geometry shader that takes the index of a single
    * layer to clear from push constants, so we need to emit a draw call for
    * each layer that we want to clear. We could actually do better and have it
    * take a range of layers and then emit one triangle per layer to clear,
    * however, if we were to do this we would need to be careful not to exceed
    * the maximum number of output vertices allowed in a geometry shader.
    */
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_GEOMETRY, options,
                                                  "meta clear gs");
   nir_shader *nir = b.shader;
   nir->info.inputs_read = 1ull << VARYING_SLOT_POS;
   nir->info.outputs_written = (1ull << VARYING_SLOT_POS) |
                               (1ull << VARYING_SLOT_LAYER);
   nir->info.gs.input_primitive = MESA_PRIM_TRIANGLES;
   nir->info.gs.output_primitive = MESA_PRIM_TRIANGLE_STRIP;
   nir->info.gs.vertices_in = 3;
   nir->info.gs.vertices_out = 3;
   nir->info.gs.invocations = 1;
   nir->info.gs.active_stream_mask = 0x1;

   /* in vec4 gl_Position[3] */
   nir_variable *gs_in_pos =
      nir_variable_create(b.shader, nir_var_shader_in,
                          glsl_array_type(glsl_vec4_type(), 3, 0),
                          "in_gl_Position");
   gs_in_pos->data.location = VARYING_SLOT_POS;

   /* out vec4 gl_Position */
   nir_variable *gs_out_pos =
      nir_variable_create(b.shader, nir_var_shader_out, glsl_vec4_type(),
                          "out_gl_Position");
   gs_out_pos->data.location = VARYING_SLOT_POS;

   /* out float gl_Layer */
   nir_variable *gs_out_layer =
      nir_variable_create(b.shader, nir_var_shader_out, glsl_float_type(),
                          "out_gl_Layer");
   gs_out_layer->data.location = VARYING_SLOT_LAYER;

   /* Emit output triangle */
   for (uint32_t i = 0; i < 3; i++) {
      /* gl_Position from shader input */
      nir_deref_instr *in_pos_i =
         nir_build_deref_array_imm(&b, nir_build_deref_var(&b, gs_in_pos), i);
      nir_copy_deref(&b, nir_build_deref_var(&b, gs_out_pos), in_pos_i);

      /* gl_Layer from push constants */
      nir_def *layer =
         nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0),
                                .base = push_constant_layer_base, .range = 4);
      nir_store_var(&b, gs_out_layer, layer, 0x1);

      nir_emit_vertex(&b, 0);
   }

   nir_end_primitive(&b, 0);

   return nir;
}

static nir_shader *
get_color_clear_rect_fs(uint32_t rt_idx, VkFormat format)
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT, options,
                                                  "meta clear fs");

   enum pipe_format pformat = vk_format_to_pipe_format(format);
   const struct glsl_type *fs_out_type =
      util_format_is_float(pformat) ? glsl_vec4_type() : glsl_uvec4_type();

   nir_variable *fs_out_color =
      nir_variable_create(b.shader, nir_var_shader_out, fs_out_type, "out_color");
   fs_out_color->data.location = FRAG_RESULT_DATA0 + rt_idx;

   nir_def *color_load = nir_load_push_constant(&b, 4, 32, nir_imm_int(&b, 0), .base = 0, .range = 16);
   nir_store_var(&b, fs_out_color, color_load, 0xf);

   return b.shader;
}

static nir_shader *
get_depth_clear_rect_fs()
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT, options,
                                                  "meta depth clear fs");

   nir_variable *fs_out_depth =
      nir_variable_create(b.shader, nir_var_shader_out, glsl_float_type(),
                          "out_depth");
   fs_out_depth->data.location = FRAG_RESULT_DEPTH;

   nir_def *depth_load =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 0, .range = 4);

   nir_store_var(&b, fs_out_depth, depth_load, 0x1);

   return b.shader;
}

static VkResult
create_pipeline(struct v3dv_device *device,
                struct v3dv_render_pass *pass,
                uint32_t subpass_idx,
                uint32_t samples,
                struct nir_shader *vs_nir,
                struct nir_shader *gs_nir,
                struct nir_shader *fs_nir,
                const VkPipelineVertexInputStateCreateInfo *vi_state,
                const VkPipelineDepthStencilStateCreateInfo *ds_state,
                const VkPipelineColorBlendStateCreateInfo *cb_state,
                const VkPipelineLayout layout,
                VkPipeline *pipeline)
{
   VkPipelineShaderStageCreateInfo stages[3] = { 0 };
   struct vk_shader_module vs_m = vk_shader_module_from_nir(vs_nir);
   struct vk_shader_module gs_m;
   struct vk_shader_module fs_m;

   uint32_t stage_count = 0;
   stages[stage_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   stages[stage_count].stage = VK_SHADER_STAGE_VERTEX_BIT;
   stages[stage_count].module = vk_shader_module_to_handle(&vs_m);
   stages[stage_count].pName = "main";
   stage_count++;

   if (gs_nir) {
      gs_m = vk_shader_module_from_nir(gs_nir);
      stages[stage_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stages[stage_count].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
      stages[stage_count].module = vk_shader_module_to_handle(&gs_m);
      stages[stage_count].pName = "main";
      stage_count++;
   }

   if (fs_nir) {
      fs_m = vk_shader_module_from_nir(fs_nir);
      stages[stage_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stages[stage_count].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      stages[stage_count].module = vk_shader_module_to_handle(&fs_m);
      stages[stage_count].pName = "main";
      stage_count++;
   }

   VkGraphicsPipelineCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

      .stageCount = stage_count,
      .pStages = stages,

      .pVertexInputState = vi_state,

      .pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
         .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
         .primitiveRestartEnable = false,
      },

      .pViewportState = &(VkPipelineViewportStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .viewportCount = 1,
         .scissorCount = 1,
      },

      .pRasterizationState = &(VkPipelineRasterizationStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
         .rasterizerDiscardEnable = false,
         .polygonMode = VK_POLYGON_MODE_FILL,
         .cullMode = VK_CULL_MODE_NONE,
         .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
         .depthBiasEnable = false,
      },

      .pMultisampleState = &(VkPipelineMultisampleStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .rasterizationSamples = samples,
         .sampleShadingEnable = false,
         .pSampleMask = NULL,
         .alphaToCoverageEnable = false,
         .alphaToOneEnable = false,
      },

      .pDepthStencilState = ds_state,

      .pColorBlendState = cb_state,

      /* The meta clear pipeline declares all state as dynamic.
       * As a consequence, vkCmdBindPipeline writes no dynamic state
       * to the cmd buffer. Therefore, at the end of the meta clear,
       * we need only restore dynamic state that was vkCmdSet.
       */
      .pDynamicState = &(VkPipelineDynamicStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
         .dynamicStateCount = 6,
         .pDynamicStates = (VkDynamicState[]) {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
            VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE,
            VK_DYNAMIC_STATE_BLEND_CONSTANTS,
            VK_DYNAMIC_STATE_DEPTH_BIAS,
            VK_DYNAMIC_STATE_LINE_WIDTH,
         },
      },

      .flags = 0,
      .layout = layout,
      .renderPass = v3dv_render_pass_to_handle(pass),
      .subpass = subpass_idx,
   };

   VkResult result =
      v3dv_CreateGraphicsPipelines(v3dv_device_to_handle(device),
                                   VK_NULL_HANDLE,
                                   1, &info,
                                   &device->vk.alloc,
                                   pipeline);

   ralloc_free(vs_nir);
   ralloc_free(gs_nir);
   ralloc_free(fs_nir);

   return result;
}

static VkResult
create_color_clear_pipeline(struct v3dv_device *device,
                            struct v3dv_render_pass *pass,
                            uint32_t subpass_idx,
                            uint32_t rt_idx,
                            VkFormat format,
                            VkSampleCountFlagBits samples,
                            uint32_t components,
                            bool is_layered,
                            VkPipelineLayout pipeline_layout,
                            VkPipeline *pipeline)
{
   nir_shader *vs_nir = get_clear_rect_vs();
   nir_shader *fs_nir = get_color_clear_rect_fs(rt_idx, format);
   nir_shader *gs_nir = is_layered ? get_clear_rect_gs(16) : NULL;

   const VkPipelineVertexInputStateCreateInfo vi_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
   };

   const VkPipelineDepthStencilStateCreateInfo ds_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = false,
      .depthWriteEnable = false,
      .depthBoundsTestEnable = false,
      .stencilTestEnable = false,
   };

   assert(subpass_idx < pass->subpass_count);
   const uint32_t color_count = pass->subpasses[subpass_idx].color_count;
   assert(rt_idx < color_count);

   VkPipelineColorBlendAttachmentState blend_att_state[V3D_MAX_DRAW_BUFFERS];
   for (uint32_t i = 0; i < color_count; i++) {
      blend_att_state[i] = (VkPipelineColorBlendAttachmentState) {
         .blendEnable = false,
         .colorWriteMask = i == rt_idx ? components : 0,
      };
   }

   const VkPipelineColorBlendStateCreateInfo cb_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = false,
      .attachmentCount = color_count,
      .pAttachments = blend_att_state
   };

   return create_pipeline(device,
                          pass, subpass_idx,
                          samples,
                          vs_nir, gs_nir, fs_nir,
                          &vi_state,
                          &ds_state,
                          &cb_state,
                          pipeline_layout,
                          pipeline);
}

static VkResult
create_depth_clear_pipeline(struct v3dv_device *device,
                            VkImageAspectFlags aspects,
                            struct v3dv_render_pass *pass,
                            uint32_t subpass_idx,
                            uint32_t samples,
                            bool is_layered,
                            VkPipelineLayout pipeline_layout,
                            VkPipeline *pipeline)
{
   const bool has_depth = aspects & VK_IMAGE_ASPECT_DEPTH_BIT;
   const bool has_stencil = aspects & VK_IMAGE_ASPECT_STENCIL_BIT;
   assert(has_depth || has_stencil);

   nir_shader *vs_nir = get_clear_rect_vs();
   nir_shader *fs_nir = has_depth ? get_depth_clear_rect_fs() : NULL;
   nir_shader *gs_nir = is_layered ? get_clear_rect_gs(4) : NULL;

   const VkPipelineVertexInputStateCreateInfo vi_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
   };

   const VkPipelineDepthStencilStateCreateInfo ds_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = has_depth,
      .depthWriteEnable = has_depth,
      .depthCompareOp = VK_COMPARE_OP_ALWAYS,
      .depthBoundsTestEnable = false,
      .stencilTestEnable = has_stencil,
      .front = {
         .passOp = VK_STENCIL_OP_REPLACE,
         .compareOp = VK_COMPARE_OP_ALWAYS,
         /* compareMask, writeMask and reference are dynamic state */
      },
      .back = { 0 },
   };

   assert(subpass_idx < pass->subpass_count);
   VkPipelineColorBlendAttachmentState blend_att_state[V3D_MAX_DRAW_BUFFERS] = { 0 };
   const VkPipelineColorBlendStateCreateInfo cb_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = false,
      .attachmentCount = pass->subpasses[subpass_idx].color_count,
      .pAttachments = blend_att_state,
   };

   return create_pipeline(device,
                          pass, subpass_idx,
                          samples,
                          vs_nir, gs_nir, fs_nir,
                          &vi_state,
                          &ds_state,
                          &cb_state,
                          pipeline_layout,
                          pipeline);
}

static VkResult
create_color_clear_render_pass(struct v3dv_device *device,
                               uint32_t rt_idx,
                               VkFormat format,
                               VkSampleCountFlagBits samples,
                               VkRenderPass *pass)
{
   VkAttachmentDescription2 att = {
      .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
      .format = format,
      .samples = samples,
      .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
      .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
   };

   VkAttachmentReference2 att_ref = {
      .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
      .attachment = rt_idx,
      .layout = VK_IMAGE_LAYOUT_GENERAL,
   };

   VkSubpassDescription2 subpass = {
      .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .colorAttachmentCount = 1,
      .pColorAttachments = &att_ref,
      .pResolveAttachments = NULL,
      .pDepthStencilAttachment = NULL,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = NULL,
   };

   VkRenderPassCreateInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
      .attachmentCount = 1,
      .pAttachments = &att,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 0,
      .pDependencies = NULL,
   };

   return v3dv_CreateRenderPass2(v3dv_device_to_handle(device),
                                 &info, &device->vk.alloc, pass);
}

static inline uint64_t
get_color_clear_pipeline_cache_key(uint32_t rt_idx,
                                   VkFormat format,
                                   VkSampleCountFlagBits samples,
                                   uint32_t components,
                                   bool is_layered)
{
   assert(rt_idx < V3D_MAX_DRAW_BUFFERS);

   uint64_t key = 0;
   uint32_t bit_offset = 0;

   key |= rt_idx;
   bit_offset += 3;

   key |= ((uint64_t) format) << bit_offset;
   bit_offset += 32;

   key |= ((uint64_t) samples) << bit_offset;
   bit_offset += 4;

   key |= ((uint64_t) components) << bit_offset;
   bit_offset += 4;

   key |= (is_layered ? 1ull : 0ull) << bit_offset;
   bit_offset += 1;

   assert(bit_offset <= 64);
   return key;
}

static inline uint64_t
get_depth_clear_pipeline_cache_key(VkImageAspectFlags aspects,
                                   VkFormat format,
                                   uint32_t samples,
                                   bool is_layered)
{
   uint64_t key = 0;
   uint32_t bit_offset = 0;

   key |= format;
   bit_offset += 32;

   key |= ((uint64_t) samples) << bit_offset;
   bit_offset += 4;

   const bool has_depth = (aspects & VK_IMAGE_ASPECT_DEPTH_BIT) ? 1 : 0;
   key |= ((uint64_t) has_depth) << bit_offset;
   bit_offset++;

   const bool has_stencil = (aspects & VK_IMAGE_ASPECT_STENCIL_BIT) ? 1 : 0;
   key |= ((uint64_t) has_stencil) << bit_offset;
   bit_offset++;;

   key |= (is_layered ? 1ull : 0ull) << bit_offset;
   bit_offset += 1;

   assert(bit_offset <= 64);
   return key;
}

static VkResult
get_color_clear_pipeline(struct v3dv_device *device,
                         struct v3dv_render_pass *pass,
                         uint32_t subpass_idx,
                         uint32_t rt_idx,
                         uint32_t attachment_idx,
                         VkFormat format,
                         VkSampleCountFlagBits samples,
                         uint32_t components,
                         bool is_layered,
                         struct v3dv_meta_color_clear_pipeline **pipeline)
{
   assert(vk_format_is_color(format));

   VkResult result = VK_SUCCESS;

   /* If pass != NULL it means that we are emitting the clear as a draw call
    * in the current pass bound by the application. In that case, we can't
    * cache the pipeline, since it will be referencing that pass and the
    * application could be destroying it at any point. Hopefully, the perf
    * impact is not too big since we still have the device pipeline cache
    * around and we won't end up re-compiling the clear shader.
    *
    * FIXME: alternatively, we could refcount (or maybe clone) the render pass
    * provided by the application and include it in the pipeline key setup
    * to make caching safe in this scenario, however, based on tests with
    * vkQuake3, the fact that we are not caching here doesn't seem to have
    * any significant impact in performance, so it might not be worth it.
    */
   const bool can_cache_pipeline = (pass == NULL);

   uint64_t key;
   if (can_cache_pipeline) {
      key = get_color_clear_pipeline_cache_key(rt_idx, format, samples,
                                               components, is_layered);
      mtx_lock(&device->meta.mtx);
      struct hash_entry *entry =
         _mesa_hash_table_search(device->meta.color_clear.cache, &key);
      if (entry) {
         mtx_unlock(&device->meta.mtx);
         *pipeline = entry->data;
         return VK_SUCCESS;
      }
   }

   *pipeline = vk_zalloc2(&device->vk.alloc, NULL, sizeof(**pipeline), 8,
                          VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

   if (*pipeline == NULL) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail;
   }

   if (!pass) {
      result = create_color_clear_render_pass(device,
                                              rt_idx,
                                              format,
                                              samples,
                                              &(*pipeline)->pass);
      if (result != VK_SUCCESS)
         goto fail;

      pass = v3dv_render_pass_from_handle((*pipeline)->pass);
   } else {
      (*pipeline)->pass = v3dv_render_pass_to_handle(pass);
   }

   result = create_color_clear_pipeline(device,
                                        pass,
                                        subpass_idx,
                                        rt_idx,
                                        format,
                                        samples,
                                        components,
                                        is_layered,
                                        device->meta.color_clear.p_layout,
                                        &(*pipeline)->pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   if (can_cache_pipeline) {
      (*pipeline)->key = key;
      (*pipeline)->cached = true;
      _mesa_hash_table_insert(device->meta.color_clear.cache,
                              &(*pipeline)->key, *pipeline);

      mtx_unlock(&device->meta.mtx);
   }

   return VK_SUCCESS;

fail:
   if (can_cache_pipeline)
      mtx_unlock(&device->meta.mtx);

   VkDevice _device = v3dv_device_to_handle(device);
   if (*pipeline) {
      if ((*pipeline)->cached)
         v3dv_DestroyRenderPass(_device, (*pipeline)->pass, &device->vk.alloc);
      if ((*pipeline)->pipeline)
         v3dv_DestroyPipeline(_device, (*pipeline)->pipeline, &device->vk.alloc);
      vk_free(&device->vk.alloc, *pipeline);
      *pipeline = NULL;
   }

   return result;
}

static VkResult
get_depth_clear_pipeline(struct v3dv_device *device,
                         VkImageAspectFlags aspects,
                         struct v3dv_render_pass *pass,
                         uint32_t subpass_idx,
                         uint32_t attachment_idx,
                         bool is_layered,
                         struct v3dv_meta_depth_clear_pipeline **pipeline)
{
   assert(subpass_idx < pass->subpass_count);
   assert(attachment_idx != VK_ATTACHMENT_UNUSED);
   assert(attachment_idx < pass->attachment_count);

   VkResult result = VK_SUCCESS;

   const uint32_t samples = pass->attachments[attachment_idx].desc.samples;
   const VkFormat format = pass->attachments[attachment_idx].desc.format;
   assert(vk_format_is_depth_or_stencil(format));

   const uint64_t key =
      get_depth_clear_pipeline_cache_key(aspects, format, samples, is_layered);
   mtx_lock(&device->meta.mtx);
   struct hash_entry *entry =
      _mesa_hash_table_search(device->meta.depth_clear.cache, &key);
   if (entry) {
      mtx_unlock(&device->meta.mtx);
      *pipeline = entry->data;
      return VK_SUCCESS;
   }

   *pipeline = vk_zalloc2(&device->vk.alloc, NULL, sizeof(**pipeline), 8,
                          VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

   if (*pipeline == NULL) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail;
   }

   result = create_depth_clear_pipeline(device,
                                        aspects,
                                        pass,
                                        subpass_idx,
                                        samples,
                                        is_layered,
                                        device->meta.depth_clear.p_layout,
                                        &(*pipeline)->pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   (*pipeline)->key = key;
   _mesa_hash_table_insert(device->meta.depth_clear.cache,
                           &(*pipeline)->key, *pipeline);

   mtx_unlock(&device->meta.mtx);
   return VK_SUCCESS;

fail:
   mtx_unlock(&device->meta.mtx);

   VkDevice _device = v3dv_device_to_handle(device);
   if (*pipeline) {
      if ((*pipeline)->pipeline)
         v3dv_DestroyPipeline(_device, (*pipeline)->pipeline, &device->vk.alloc);
      vk_free(&device->vk.alloc, *pipeline);
      *pipeline = NULL;
   }

   return result;
}

/* Emits a scissored quad in the clear color */
static void
emit_subpass_color_clear_rects(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_render_pass *pass,
                               struct v3dv_subpass *subpass,
                               uint32_t rt_idx,
                               const VkClearColorValue *clear_color,
                               bool is_layered,
                               bool all_rects_same_layers,
                               uint32_t rect_count,
                               const VkClearRect *rects)
{
   /* Skip if attachment is unused in the current subpass */
   assert(rt_idx < subpass->color_count);
   const uint32_t attachment_idx = subpass->color_attachments[rt_idx].attachment;
   if (attachment_idx == VK_ATTACHMENT_UNUSED)
      return;

   /* Obtain a pipeline for this clear */
   assert(attachment_idx < cmd_buffer->state.pass->attachment_count);
   const VkFormat format =
      cmd_buffer->state.pass->attachments[attachment_idx].desc.format;
   const VkSampleCountFlagBits samples =
      cmd_buffer->state.pass->attachments[attachment_idx].desc.samples;
   const uint32_t components = VK_COLOR_COMPONENT_R_BIT |
                               VK_COLOR_COMPONENT_G_BIT |
                               VK_COLOR_COMPONENT_B_BIT |
                               VK_COLOR_COMPONENT_A_BIT;
   struct v3dv_meta_color_clear_pipeline *pipeline = NULL;
   VkResult result = get_color_clear_pipeline(cmd_buffer->device,
                                              pass,
                                              cmd_buffer->state.subpass_idx,
                                              rt_idx,
                                              attachment_idx,
                                              format,
                                              samples,
                                              components,
                                              is_layered,
                                              &pipeline);
   if (result != VK_SUCCESS) {
      if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
         v3dv_flag_oom(cmd_buffer, NULL);
      return;
   }
   assert(pipeline && pipeline->pipeline);

   /* Emit clear rects */
   v3dv_cmd_buffer_meta_state_push(cmd_buffer, false);

   VkCommandBuffer cmd_buffer_handle = v3dv_cmd_buffer_to_handle(cmd_buffer);
   v3dv_CmdPushConstants(cmd_buffer_handle,
                         cmd_buffer->device->meta.depth_clear.p_layout,
                         VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16,
                         clear_color->float32);

   v3dv_CmdBindPipeline(cmd_buffer_handle,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->pipeline);

   for (uint32_t i = 0; i < rect_count; i++) {
      const VkViewport viewport = {
         .x = rects[i].rect.offset.x,
         .y = rects[i].rect.offset.y,
         .width = rects[i].rect.extent.width,
         .height = rects[i].rect.extent.height,
         .minDepth = 0.0f,
         .maxDepth = 1.0f
      };
      v3dv_CmdSetViewport(cmd_buffer_handle, 0, 1, &viewport);
      v3dv_CmdSetScissor(cmd_buffer_handle, 0, 1, &rects[i].rect);

      if (is_layered) {
         for (uint32_t layer_offset = 0; layer_offset < rects[i].layerCount;
              layer_offset++) {
            uint32_t layer = rects[i].baseArrayLayer + layer_offset;
            v3dv_CmdPushConstants(cmd_buffer_handle,
                                  cmd_buffer->device->meta.depth_clear.p_layout,
                                  VK_SHADER_STAGE_GEOMETRY_BIT, 16, 4, &layer);
            v3dv_CmdDraw(cmd_buffer_handle, 4, 1, 0, 0);
         }
      } else {
         assert(rects[i].baseArrayLayer == 0 && rects[i].layerCount == 1);
         v3dv_CmdDraw(cmd_buffer_handle, 4, 1, 0, 0);
      }
   }

   /* Subpass pipelines can't be cached because they include a reference to the
    * render pass currently bound by the application, which means that we need
    * to destroy them manually here.
    */
   assert(!pipeline->cached);
   v3dv_cmd_buffer_add_private_obj(
      cmd_buffer, (uintptr_t)pipeline,
      (v3dv_cmd_buffer_private_obj_destroy_cb) destroy_color_clear_pipeline);

   v3dv_cmd_buffer_meta_state_pop(cmd_buffer, false);
}

/* Emits a scissored quad, clearing the depth aspect by writing to gl_FragDepth
 * and the stencil aspect by using stencil testing.
 */
static void
emit_subpass_ds_clear_rects(struct v3dv_cmd_buffer *cmd_buffer,
                            struct v3dv_render_pass *pass,
                            struct v3dv_subpass *subpass,
                            VkImageAspectFlags aspects,
                            const VkClearDepthStencilValue *clear_ds,
                            bool is_layered,
                            bool all_rects_same_layers,
                            uint32_t rect_count,
                            const VkClearRect *rects)
{
   /* Skip if attachment is unused in the current subpass */
   const uint32_t attachment_idx = subpass->ds_attachment.attachment;
   if (attachment_idx == VK_ATTACHMENT_UNUSED)
      return;

   /* Obtain a pipeline for this clear */
   assert(attachment_idx < cmd_buffer->state.pass->attachment_count);
   struct v3dv_meta_depth_clear_pipeline *pipeline = NULL;
   VkResult result = get_depth_clear_pipeline(cmd_buffer->device,
                                              aspects,
                                              pass,
                                              cmd_buffer->state.subpass_idx,
                                              attachment_idx,
                                              is_layered,
                                              &pipeline);
   if (result != VK_SUCCESS) {
      if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
         v3dv_flag_oom(cmd_buffer, NULL);
      return;
   }
   assert(pipeline && pipeline->pipeline);

   /* Emit clear rects */
   v3dv_cmd_buffer_meta_state_push(cmd_buffer, false);

   VkCommandBuffer cmd_buffer_handle = v3dv_cmd_buffer_to_handle(cmd_buffer);
   v3dv_CmdPushConstants(cmd_buffer_handle,
                         cmd_buffer->device->meta.depth_clear.p_layout,
                         VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4,
                         &clear_ds->depth);

   v3dv_CmdBindPipeline(cmd_buffer_handle,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->pipeline);

   if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      v3dv_CmdSetStencilReference(cmd_buffer_handle,
                                  VK_STENCIL_FACE_FRONT_AND_BACK,
                                  clear_ds->stencil);
      v3dv_CmdSetStencilWriteMask(cmd_buffer_handle,
                                  VK_STENCIL_FACE_FRONT_AND_BACK, 0xff);
      v3dv_CmdSetStencilCompareMask(cmd_buffer_handle,
                                    VK_STENCIL_FACE_FRONT_AND_BACK, 0xff);
   }

   for (uint32_t i = 0; i < rect_count; i++) {
      const VkViewport viewport = {
         .x = rects[i].rect.offset.x,
         .y = rects[i].rect.offset.y,
         .width = rects[i].rect.extent.width,
         .height = rects[i].rect.extent.height,
         .minDepth = 0.0f,
         .maxDepth = 1.0f
      };
      v3dv_CmdSetViewport(cmd_buffer_handle, 0, 1, &viewport);
      v3dv_CmdSetScissor(cmd_buffer_handle, 0, 1, &rects[i].rect);
      if (is_layered) {
         for (uint32_t layer_offset = 0; layer_offset < rects[i].layerCount;
              layer_offset++) {
            uint32_t layer = rects[i].baseArrayLayer + layer_offset;
            v3dv_CmdPushConstants(cmd_buffer_handle,
                                  cmd_buffer->device->meta.depth_clear.p_layout,
                                  VK_SHADER_STAGE_GEOMETRY_BIT, 4, 4, &layer);
            v3dv_CmdDraw(cmd_buffer_handle, 4, 1, 0, 0);
         }
      } else {
         assert(rects[i].baseArrayLayer == 0 && rects[i].layerCount == 1);
         v3dv_CmdDraw(cmd_buffer_handle, 4, 1, 0, 0);
      }
   }

   v3dv_cmd_buffer_meta_state_pop(cmd_buffer, false);
}

static void
gather_layering_info(uint32_t rect_count, const VkClearRect *rects,
                     bool *is_layered, bool *all_rects_same_layers)
{
   *all_rects_same_layers = true;

   uint32_t min_layer = rects[0].baseArrayLayer;
   uint32_t max_layer = rects[0].baseArrayLayer + rects[0].layerCount - 1;
   for (uint32_t i = 1; i < rect_count; i++) {
      if (rects[i].baseArrayLayer != rects[i - 1].baseArrayLayer ||
          rects[i].layerCount != rects[i - 1].layerCount) {
         *all_rects_same_layers = false;
         min_layer = MIN2(min_layer, rects[i].baseArrayLayer);
         max_layer = MAX2(max_layer, rects[i].baseArrayLayer +
                                     rects[i].layerCount - 1);
      }
   }

   *is_layered = !(min_layer == 0 && max_layer == 0);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdClearAttachments(VkCommandBuffer commandBuffer,
                         uint32_t attachmentCount,
                         const VkClearAttachment *pAttachments,
                         uint32_t rectCount,
                         const VkClearRect *pRects)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   /* We can have at most max_color_RTs + 1 D/S attachments */
   assert(attachmentCount <=
          V3D_MAX_RENDER_TARGETS(cmd_buffer->device->devinfo.ver) + 1);

   /* We can only clear attachments in the current subpass */
   struct v3dv_render_pass *pass = cmd_buffer->state.pass;

   assert(cmd_buffer->state.subpass_idx < pass->subpass_count);
   struct v3dv_subpass *subpass =
      &cmd_buffer->state.pass->subpasses[cmd_buffer->state.subpass_idx];

   /* Emit a clear rect inside the current job for this subpass. For layered
    * framebuffers, we use a geometry shader to redirect clears to the
    * appropriate layers.
    */

   v3dv_cmd_buffer_pause_occlusion_query(cmd_buffer);

   bool is_layered, all_rects_same_layers;
   gather_layering_info(rectCount, pRects, &is_layered, &all_rects_same_layers);
   for (uint32_t i = 0; i < attachmentCount; i++) {
      if (pAttachments[i].aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
         emit_subpass_color_clear_rects(cmd_buffer, pass, subpass,
                                        pAttachments[i].colorAttachment,
                                        &pAttachments[i].clearValue.color,
                                        is_layered, all_rects_same_layers,
                                        rectCount, pRects);
      } else {
         emit_subpass_ds_clear_rects(cmd_buffer, pass, subpass,
                                     pAttachments[i].aspectMask,
                                     &pAttachments[i].clearValue.depthStencil,
                                     is_layered, all_rects_same_layers,
                                     rectCount, pRects);
      }
   }

   v3dv_cmd_buffer_resume_occlusion_query(cmd_buffer);
}
