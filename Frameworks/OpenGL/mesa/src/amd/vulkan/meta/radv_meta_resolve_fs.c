/*
 * Copyright © 2016 Dave Airlie
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

#include "nir/nir_builder.h"
#include "radv_meta.h"
#include "radv_private.h"
#include "sid.h"
#include "vk_common_entrypoints.h"
#include "vk_format.h"

static nir_shader *
build_resolve_fragment_shader(struct radv_device *dev, bool is_integer, int samples)
{
   enum glsl_base_type img_base_type = is_integer ? GLSL_TYPE_UINT : GLSL_TYPE_FLOAT;
   const struct glsl_type *vec4 = glsl_vec4_type();
   const struct glsl_type *sampler_type = glsl_sampler_type(GLSL_SAMPLER_DIM_MS, false, false, img_base_type);

   nir_builder b =
      radv_meta_init_shader(dev, MESA_SHADER_FRAGMENT, "meta_resolve_fs-%d-%s", samples, is_integer ? "int" : "float");

   nir_variable *input_img = nir_variable_create(b.shader, nir_var_uniform, sampler_type, "s_tex");
   input_img->data.descriptor_set = 0;
   input_img->data.binding = 0;

   nir_variable *color_out = nir_variable_create(b.shader, nir_var_shader_out, vec4, "f_color");
   color_out->data.location = FRAG_RESULT_DATA0;

   nir_def *pos_in = nir_trim_vector(&b, nir_load_frag_coord(&b), 2);
   nir_def *src_offset = nir_load_push_constant(&b, 2, 32, nir_imm_int(&b, 0), .range = 8);

   nir_def *pos_int = nir_f2i32(&b, pos_in);

   nir_def *img_coord = nir_trim_vector(&b, nir_iadd(&b, pos_int, src_offset), 2);
   nir_variable *color = nir_local_variable_create(b.impl, glsl_vec4_type(), "color");

   radv_meta_build_resolve_shader_core(dev, &b, is_integer, samples, input_img, color, img_coord);

   nir_def *outval = nir_load_var(&b, color);
   nir_store_var(&b, color_out, outval, 0xf);
   return b.shader;
}

static VkResult
create_layout(struct radv_device *device)
{
   VkResult result;
   /*
    * one descriptors for the image being sampled
    */
   VkDescriptorSetLayoutCreateInfo ds_create_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                     .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
                                                     .bindingCount = 1,
                                                     .pBindings = (VkDescriptorSetLayoutBinding[]){
                                                        {.binding = 0,
                                                         .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                         .descriptorCount = 1,
                                                         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                         .pImmutableSamplers = NULL},
                                                     }};

   result = radv_CreateDescriptorSetLayout(radv_device_to_handle(device), &ds_create_info, &device->meta_state.alloc,
                                           &device->meta_state.resolve_fragment.ds_layout);
   if (result != VK_SUCCESS)
      goto fail;

   VkPipelineLayoutCreateInfo pl_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &device->meta_state.resolve_fragment.ds_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &(VkPushConstantRange){VK_SHADER_STAGE_FRAGMENT_BIT, 0, 8},
   };

   result = radv_CreatePipelineLayout(radv_device_to_handle(device), &pl_create_info, &device->meta_state.alloc,
                                      &device->meta_state.resolve_fragment.p_layout);
   if (result != VK_SUCCESS)
      goto fail;
   return VK_SUCCESS;
fail:
   return result;
}

static const VkPipelineVertexInputStateCreateInfo normal_vi_create_info = {
   .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
   .vertexBindingDescriptionCount = 0,
   .vertexAttributeDescriptionCount = 0,
};

static VkResult
create_resolve_pipeline(struct radv_device *device, int samples_log2, VkFormat format)
{
   mtx_lock(&device->meta_state.mtx);

   unsigned fs_key = radv_format_meta_fs_key(device, format);
   VkPipeline *pipeline = &device->meta_state.resolve_fragment.rc[samples_log2].pipeline[fs_key];
   if (*pipeline) {
      mtx_unlock(&device->meta_state.mtx);
      return VK_SUCCESS;
   }

   VkResult result;
   bool is_integer = false;
   uint32_t samples = 1 << samples_log2;
   const VkPipelineVertexInputStateCreateInfo *vi_create_info;
   vi_create_info = &normal_vi_create_info;
   if (vk_format_is_int(format))
      is_integer = true;

   nir_shader *fs = build_resolve_fragment_shader(device, is_integer, samples);
   nir_shader *vs = radv_meta_build_nir_vs_generate_vertices(device);

   VkPipelineShaderStageCreateInfo pipeline_shader_stages[] = {
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = VK_SHADER_STAGE_VERTEX_BIT,
       .module = vk_shader_module_handle_from_nir(vs),
       .pName = "main",
       .pSpecializationInfo = NULL},
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = vk_shader_module_handle_from_nir(fs),
       .pName = "main",
       .pSpecializationInfo = NULL},
   };

   const VkPipelineRenderingCreateInfo rendering_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &format,
   };

   const VkGraphicsPipelineCreateInfo vk_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &rendering_create_info,
      .stageCount = ARRAY_SIZE(pipeline_shader_stages),
      .pStages = pipeline_shader_stages,
      .pVertexInputState = vi_create_info,
      .pInputAssemblyState =
         &(VkPipelineInputAssemblyStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .primitiveRestartEnable = false,
         },
      .pViewportState =
         &(VkPipelineViewportStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
         },
      .pRasterizationState =
         &(VkPipelineRasterizationStateCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                                   .rasterizerDiscardEnable = false,
                                                   .polygonMode = VK_POLYGON_MODE_FILL,
                                                   .cullMode = VK_CULL_MODE_NONE,
                                                   .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                                   .depthBiasConstantFactor = 0.0f,
                                                   .depthBiasClamp = 0.0f,
                                                   .depthBiasSlopeFactor = 0.0f,
                                                   .lineWidth = 1.0f},
      .pMultisampleState =
         &(VkPipelineMultisampleStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = 1,
            .sampleShadingEnable = false,
            .pSampleMask = (VkSampleMask[]){UINT32_MAX},
         },
      .pColorBlendState =
         &(VkPipelineColorBlendStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments =
               (VkPipelineColorBlendAttachmentState[]){
                  {.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT},
               },
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}},
      .pDynamicState =
         &(VkPipelineDynamicStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates =
               (VkDynamicState[]){
                  VK_DYNAMIC_STATE_VIEWPORT,
                  VK_DYNAMIC_STATE_SCISSOR,
               },
         },
      .flags = 0,
      .layout = device->meta_state.resolve_fragment.p_layout,
      .renderPass = VK_NULL_HANDLE,
      .subpass = 0,
   };

   const struct radv_graphics_pipeline_create_info radv_pipeline_info = {.use_rectlist = true};

   result = radv_graphics_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &vk_pipeline_info,
                                          &radv_pipeline_info, &device->meta_state.alloc, pipeline);
   ralloc_free(vs);
   ralloc_free(fs);

   mtx_unlock(&device->meta_state.mtx);
   return result;
}

enum { DEPTH_RESOLVE, STENCIL_RESOLVE };

static const char *
get_resolve_mode_str(VkResolveModeFlagBits resolve_mode)
{
   switch (resolve_mode) {
   case VK_RESOLVE_MODE_SAMPLE_ZERO_BIT:
      return "zero";
   case VK_RESOLVE_MODE_AVERAGE_BIT:
      return "average";
   case VK_RESOLVE_MODE_MIN_BIT:
      return "min";
   case VK_RESOLVE_MODE_MAX_BIT:
      return "max";
   default:
      unreachable("invalid resolve mode");
   }
}

static nir_shader *
build_depth_stencil_resolve_fragment_shader(struct radv_device *dev, int samples, int index,
                                            VkResolveModeFlagBits resolve_mode)
{
   enum glsl_base_type img_base_type = index == DEPTH_RESOLVE ? GLSL_TYPE_FLOAT : GLSL_TYPE_UINT;
   const struct glsl_type *vec4 = glsl_vec4_type();
   const struct glsl_type *sampler_type = glsl_sampler_type(GLSL_SAMPLER_DIM_MS, false, false, img_base_type);

   nir_builder b =
      radv_meta_init_shader(dev, MESA_SHADER_FRAGMENT, "meta_resolve_fs_%s-%s-%d",
                            index == DEPTH_RESOLVE ? "depth" : "stencil", get_resolve_mode_str(resolve_mode), samples);

   nir_variable *input_img = nir_variable_create(b.shader, nir_var_uniform, sampler_type, "s_tex");
   input_img->data.descriptor_set = 0;
   input_img->data.binding = 0;

   nir_variable *fs_out = nir_variable_create(b.shader, nir_var_shader_out, vec4, "f_out");
   fs_out->data.location = index == DEPTH_RESOLVE ? FRAG_RESULT_DEPTH : FRAG_RESULT_STENCIL;

   nir_def *pos_in = nir_trim_vector(&b, nir_load_frag_coord(&b), 2);

   nir_def *pos_int = nir_f2i32(&b, pos_in);

   nir_def *img_coord = nir_trim_vector(&b, pos_int, 2);

   nir_deref_instr *input_img_deref = nir_build_deref_var(&b, input_img);
   nir_def *outval = nir_txf_ms_deref(&b, input_img_deref, img_coord, nir_imm_int(&b, 0));

   if (resolve_mode != VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) {
      for (int i = 1; i < samples; i++) {
         nir_def *si = nir_txf_ms_deref(&b, input_img_deref, img_coord, nir_imm_int(&b, i));

         switch (resolve_mode) {
         case VK_RESOLVE_MODE_AVERAGE_BIT:
            assert(index == DEPTH_RESOLVE);
            outval = nir_fadd(&b, outval, si);
            break;
         case VK_RESOLVE_MODE_MIN_BIT:
            if (index == DEPTH_RESOLVE)
               outval = nir_fmin(&b, outval, si);
            else
               outval = nir_umin(&b, outval, si);
            break;
         case VK_RESOLVE_MODE_MAX_BIT:
            if (index == DEPTH_RESOLVE)
               outval = nir_fmax(&b, outval, si);
            else
               outval = nir_umax(&b, outval, si);
            break;
         default:
            unreachable("invalid resolve mode");
         }
      }

      if (resolve_mode == VK_RESOLVE_MODE_AVERAGE_BIT)
         outval = nir_fdiv_imm(&b, outval, samples);
   }

   nir_store_var(&b, fs_out, outval, 0x1);

   return b.shader;
}

static VkResult
create_depth_stencil_resolve_pipeline(struct radv_device *device, int samples_log2, int index,
                                      VkResolveModeFlagBits resolve_mode)
{
   VkPipeline *pipeline;
   VkResult result;

   mtx_lock(&device->meta_state.mtx);

   switch (resolve_mode) {
   case VK_RESOLVE_MODE_SAMPLE_ZERO_BIT:
      if (index == DEPTH_RESOLVE)
         pipeline = &device->meta_state.resolve_fragment.depth_zero_pipeline;
      else
         pipeline = &device->meta_state.resolve_fragment.stencil_zero_pipeline;
      break;
   case VK_RESOLVE_MODE_AVERAGE_BIT:
      assert(index == DEPTH_RESOLVE);
      pipeline = &device->meta_state.resolve_fragment.depth[samples_log2].average_pipeline;
      break;
   case VK_RESOLVE_MODE_MIN_BIT:
      if (index == DEPTH_RESOLVE)
         pipeline = &device->meta_state.resolve_fragment.depth[samples_log2].min_pipeline;
      else
         pipeline = &device->meta_state.resolve_fragment.stencil[samples_log2].min_pipeline;
      break;
   case VK_RESOLVE_MODE_MAX_BIT:
      if (index == DEPTH_RESOLVE)
         pipeline = &device->meta_state.resolve_fragment.depth[samples_log2].max_pipeline;
      else
         pipeline = &device->meta_state.resolve_fragment.stencil[samples_log2].max_pipeline;
      break;
   default:
      unreachable("invalid resolve mode");
   }

   if (*pipeline) {
      mtx_unlock(&device->meta_state.mtx);
      return VK_SUCCESS;
   }

   uint32_t samples = 1 << samples_log2;
   nir_shader *fs = build_depth_stencil_resolve_fragment_shader(device, samples, index, resolve_mode);
   nir_shader *vs = radv_meta_build_nir_vs_generate_vertices(device);

   VkPipelineShaderStageCreateInfo pipeline_shader_stages[] = {
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = VK_SHADER_STAGE_VERTEX_BIT,
       .module = vk_shader_module_handle_from_nir(vs),
       .pName = "main",
       .pSpecializationInfo = NULL},
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = vk_shader_module_handle_from_nir(fs),
       .pName = "main",
       .pSpecializationInfo = NULL},
   };

   VkStencilOp stencil_op = index == DEPTH_RESOLVE ? VK_STENCIL_OP_KEEP : VK_STENCIL_OP_REPLACE;

   VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = true,
      .depthWriteEnable = index == DEPTH_RESOLVE,
      .stencilTestEnable = index == STENCIL_RESOLVE,
      .depthCompareOp = VK_COMPARE_OP_ALWAYS,
      .front =
         {
            .failOp = stencil_op,
            .passOp = stencil_op,
            .depthFailOp = stencil_op,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = UINT32_MAX,
            .writeMask = UINT32_MAX,
            .reference = 0u,
         },
      .back =
         {
            .failOp = stencil_op,
            .passOp = stencil_op,
            .depthFailOp = stencil_op,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = UINT32_MAX,
            .writeMask = UINT32_MAX,
            .reference = 0u,
         },
      .minDepthBounds = 0.0f,
      .maxDepthBounds = 1.0f};

   const VkPipelineVertexInputStateCreateInfo *vi_create_info;
   vi_create_info = &normal_vi_create_info;

   const VkPipelineRenderingCreateInfo rendering_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .depthAttachmentFormat = index == DEPTH_RESOLVE ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_UNDEFINED,
      .stencilAttachmentFormat = index == STENCIL_RESOLVE ? VK_FORMAT_S8_UINT : VK_FORMAT_UNDEFINED,
   };

   const VkGraphicsPipelineCreateInfo vk_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &rendering_create_info,
      .stageCount = ARRAY_SIZE(pipeline_shader_stages),
      .pStages = pipeline_shader_stages,
      .pVertexInputState = vi_create_info,
      .pInputAssemblyState =
         &(VkPipelineInputAssemblyStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .primitiveRestartEnable = false,
         },
      .pViewportState =
         &(VkPipelineViewportStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
         },
      .pDepthStencilState = &depth_stencil_state,
      .pRasterizationState =
         &(VkPipelineRasterizationStateCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                                   .rasterizerDiscardEnable = false,
                                                   .polygonMode = VK_POLYGON_MODE_FILL,
                                                   .cullMode = VK_CULL_MODE_NONE,
                                                   .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                                   .depthBiasConstantFactor = 0.0f,
                                                   .depthBiasClamp = 0.0f,
                                                   .depthBiasSlopeFactor = 0.0f,
                                                   .lineWidth = 1.0f},
      .pMultisampleState =
         &(VkPipelineMultisampleStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = 1,
            .sampleShadingEnable = false,
            .pSampleMask = (VkSampleMask[]){UINT32_MAX},
         },
      .pColorBlendState =
         &(VkPipelineColorBlendStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 0,
            .pAttachments =
               (VkPipelineColorBlendAttachmentState[]){
                  {.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT},
               },
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}},
      .pDynamicState =
         &(VkPipelineDynamicStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates =
               (VkDynamicState[]){
                  VK_DYNAMIC_STATE_VIEWPORT,
                  VK_DYNAMIC_STATE_SCISSOR,
               },
         },
      .flags = 0,
      .layout = device->meta_state.resolve_fragment.p_layout,
      .renderPass = VK_NULL_HANDLE,
      .subpass = 0,
   };

   const struct radv_graphics_pipeline_create_info radv_pipeline_info = {.use_rectlist = true};

   result = radv_graphics_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &vk_pipeline_info,
                                          &radv_pipeline_info, &device->meta_state.alloc, pipeline);

   ralloc_free(vs);
   ralloc_free(fs);

   mtx_unlock(&device->meta_state.mtx);
   return result;
}

VkResult
radv_device_init_meta_resolve_fragment_state(struct radv_device *device, bool on_demand)
{
   VkResult res;

   res = create_layout(device);
   if (res != VK_SUCCESS)
      return res;

   if (on_demand)
      return VK_SUCCESS;

   for (uint32_t i = 0; i < MAX_SAMPLES_LOG2; ++i) {
      for (unsigned j = 0; j < NUM_META_FS_KEYS; ++j) {
         res = create_resolve_pipeline(device, i, radv_fs_key_format_exemplars[j]);
         if (res != VK_SUCCESS)
            return res;
      }

      res = create_depth_stencil_resolve_pipeline(device, i, DEPTH_RESOLVE, VK_RESOLVE_MODE_AVERAGE_BIT);
      if (res != VK_SUCCESS)
         return res;

      res = create_depth_stencil_resolve_pipeline(device, i, DEPTH_RESOLVE, VK_RESOLVE_MODE_MIN_BIT);
      if (res != VK_SUCCESS)
         return res;

      res = create_depth_stencil_resolve_pipeline(device, i, DEPTH_RESOLVE, VK_RESOLVE_MODE_MAX_BIT);
      if (res != VK_SUCCESS)
         return res;

      res = create_depth_stencil_resolve_pipeline(device, i, STENCIL_RESOLVE, VK_RESOLVE_MODE_MIN_BIT);
      if (res != VK_SUCCESS)
         return res;

      res = create_depth_stencil_resolve_pipeline(device, i, STENCIL_RESOLVE, VK_RESOLVE_MODE_MAX_BIT);
      if (res != VK_SUCCESS)
         return res;
   }

   res = create_depth_stencil_resolve_pipeline(device, 0, DEPTH_RESOLVE, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
   if (res != VK_SUCCESS)
      return res;

   return create_depth_stencil_resolve_pipeline(device, 0, STENCIL_RESOLVE, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
}

void
radv_device_finish_meta_resolve_fragment_state(struct radv_device *device)
{
   struct radv_meta_state *state = &device->meta_state;
   for (uint32_t i = 0; i < MAX_SAMPLES_LOG2; ++i) {
      for (unsigned j = 0; j < NUM_META_FS_KEYS; ++j) {
         radv_DestroyPipeline(radv_device_to_handle(device), state->resolve_fragment.rc[i].pipeline[j], &state->alloc);
      }

      radv_DestroyPipeline(radv_device_to_handle(device), state->resolve_fragment.depth[i].average_pipeline,
                           &state->alloc);

      radv_DestroyPipeline(radv_device_to_handle(device), state->resolve_fragment.depth[i].max_pipeline, &state->alloc);

      radv_DestroyPipeline(radv_device_to_handle(device), state->resolve_fragment.depth[i].min_pipeline, &state->alloc);

      radv_DestroyPipeline(radv_device_to_handle(device), state->resolve_fragment.stencil[i].max_pipeline,
                           &state->alloc);

      radv_DestroyPipeline(radv_device_to_handle(device), state->resolve_fragment.stencil[i].min_pipeline,
                           &state->alloc);
   }

   radv_DestroyPipeline(radv_device_to_handle(device), state->resolve_fragment.depth_zero_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->resolve_fragment.stencil_zero_pipeline, &state->alloc);

   device->vk.dispatch_table.DestroyDescriptorSetLayout(radv_device_to_handle(device),
                                                        state->resolve_fragment.ds_layout, &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->resolve_fragment.p_layout, &state->alloc);
}

static VkPipeline *
radv_get_resolve_pipeline(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *src_iview,
                          struct radv_image_view *dst_iview)
{
   struct radv_device *device = cmd_buffer->device;
   unsigned fs_key = radv_format_meta_fs_key(cmd_buffer->device, dst_iview->vk.format);
   const uint32_t samples = src_iview->image->vk.samples;
   const uint32_t samples_log2 = ffs(samples) - 1;
   VkPipeline *pipeline;

   pipeline = &device->meta_state.resolve_fragment.rc[samples_log2].pipeline[fs_key];
   if (!*pipeline) {
      VkResult ret;

      ret = create_resolve_pipeline(device, samples_log2, radv_fs_key_format_exemplars[fs_key]);
      if (ret != VK_SUCCESS) {
         vk_command_buffer_set_error(&cmd_buffer->vk, ret);
         return NULL;
      }
   }

   return pipeline;
}

static void
emit_resolve(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *src_iview, struct radv_image_view *dst_iview,
             const VkOffset2D *src_offset, const VkOffset2D *dst_offset)
{
   struct radv_device *device = cmd_buffer->device;
   VkCommandBuffer cmd_buffer_h = radv_cmd_buffer_to_handle(cmd_buffer);
   VkPipeline *pipeline;

   radv_meta_push_descriptor_set(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 cmd_buffer->device->meta_state.resolve_fragment.p_layout, 0, /* set */
                                 1,                                                           /* descriptorWriteCount */
                                 (VkWriteDescriptorSet[]){
                                    {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                     .dstBinding = 0,
                                     .dstArrayElement = 0,
                                     .descriptorCount = 1,
                                     .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                     .pImageInfo =
                                        (VkDescriptorImageInfo[]){
                                           {
                                              .sampler = VK_NULL_HANDLE,
                                              .imageView = radv_image_view_to_handle(src_iview),
                                              .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                                           },
                                        }},
                                 });

   cmd_buffer->state.flush_bits |=
      radv_dst_access_flush(cmd_buffer, VK_ACCESS_2_SHADER_READ_BIT, src_iview->image) |
      radv_dst_access_flush(cmd_buffer, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, dst_iview->image);

   unsigned push_constants[2] = {
      src_offset->x - dst_offset->x,
      src_offset->y - dst_offset->y,
   };
   vk_common_CmdPushConstants(radv_cmd_buffer_to_handle(cmd_buffer), device->meta_state.resolve_fragment.p_layout,
                              VK_SHADER_STAGE_FRAGMENT_BIT, 0, 8, push_constants);

   pipeline = radv_get_resolve_pipeline(cmd_buffer, src_iview, dst_iview);

   radv_CmdBindPipeline(cmd_buffer_h, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

   radv_CmdDraw(cmd_buffer_h, 3, 1, 0, 0);
   cmd_buffer->state.flush_bits |=
      radv_src_access_flush(cmd_buffer, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, dst_iview->image);
}

static void
emit_depth_stencil_resolve(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *src_iview,
                           struct radv_image_view *dst_iview, const VkOffset2D *resolve_offset,
                           const VkExtent2D *resolve_extent, VkImageAspectFlags aspects,
                           VkResolveModeFlagBits resolve_mode)
{
   struct radv_device *device = cmd_buffer->device;
   const uint32_t samples = src_iview->image->vk.samples;
   const uint32_t samples_log2 = ffs(samples) - 1;
   VkPipeline *pipeline;

   radv_meta_push_descriptor_set(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 cmd_buffer->device->meta_state.resolve_fragment.p_layout, 0, /* set */
                                 1,                                                           /* descriptorWriteCount */
                                 (VkWriteDescriptorSet[]){
                                    {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                     .dstBinding = 0,
                                     .dstArrayElement = 0,
                                     .descriptorCount = 1,
                                     .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                     .pImageInfo =
                                        (VkDescriptorImageInfo[]){
                                           {
                                              .sampler = VK_NULL_HANDLE,
                                              .imageView = radv_image_view_to_handle(src_iview),
                                              .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                                           },
                                        }},
                                 });

   switch (resolve_mode) {
   case VK_RESOLVE_MODE_SAMPLE_ZERO_BIT:
      if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT)
         pipeline = &device->meta_state.resolve_fragment.depth_zero_pipeline;
      else
         pipeline = &device->meta_state.resolve_fragment.stencil_zero_pipeline;
      break;
   case VK_RESOLVE_MODE_AVERAGE_BIT:
      assert(aspects == VK_IMAGE_ASPECT_DEPTH_BIT);
      pipeline = &device->meta_state.resolve_fragment.depth[samples_log2].average_pipeline;
      break;
   case VK_RESOLVE_MODE_MIN_BIT:
      if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT)
         pipeline = &device->meta_state.resolve_fragment.depth[samples_log2].min_pipeline;
      else
         pipeline = &device->meta_state.resolve_fragment.stencil[samples_log2].min_pipeline;
      break;
   case VK_RESOLVE_MODE_MAX_BIT:
      if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT)
         pipeline = &device->meta_state.resolve_fragment.depth[samples_log2].max_pipeline;
      else
         pipeline = &device->meta_state.resolve_fragment.stencil[samples_log2].max_pipeline;
      break;
   default:
      unreachable("invalid resolve mode");
   }

   if (!*pipeline) {
      int index = aspects == VK_IMAGE_ASPECT_DEPTH_BIT ? DEPTH_RESOLVE : STENCIL_RESOLVE;
      VkResult ret;

      ret = create_depth_stencil_resolve_pipeline(device, samples_log2, index, resolve_mode);
      if (ret != VK_SUCCESS) {
         vk_command_buffer_set_error(&cmd_buffer->vk, ret);
         return;
      }
   }

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

   radv_CmdSetViewport(radv_cmd_buffer_to_handle(cmd_buffer), 0, 1,
                       &(VkViewport){.x = resolve_offset->x,
                                     .y = resolve_offset->y,
                                     .width = resolve_extent->width,
                                     .height = resolve_extent->height,
                                     .minDepth = 0.0f,
                                     .maxDepth = 1.0f});

   radv_CmdSetScissor(radv_cmd_buffer_to_handle(cmd_buffer), 0, 1,
                      &(VkRect2D){
                         .offset = *resolve_offset,
                         .extent = *resolve_extent,
                      });

   radv_CmdDraw(radv_cmd_buffer_to_handle(cmd_buffer), 3, 1, 0, 0);
}

void
radv_meta_resolve_fragment_image(struct radv_cmd_buffer *cmd_buffer, struct radv_image *src_image,
                                 VkImageLayout src_image_layout, struct radv_image *dst_image,
                                 VkImageLayout dst_image_layout, const VkImageResolve2 *region)
{
   struct radv_meta_saved_state saved_state;
   unsigned dst_layout = radv_meta_dst_layout_from_layout(dst_image_layout);
   VkImageLayout layout = radv_meta_dst_layout_to_layout(dst_layout);

   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_GRAPHICS_PIPELINE | RADV_META_SAVE_CONSTANTS | RADV_META_SAVE_DESCRIPTORS);

   assert(region->srcSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);
   assert(region->dstSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);
   /* Multi-layer resolves are handled by compute */
   assert(vk_image_subresource_layer_count(&src_image->vk, &region->srcSubresource) == 1 &&
          vk_image_subresource_layer_count(&dst_image->vk, &region->dstSubresource) == 1);

   const struct VkExtent3D extent = vk_image_sanitize_extent(&src_image->vk, region->extent);
   const struct VkOffset3D srcOffset = vk_image_sanitize_offset(&src_image->vk, region->srcOffset);
   const struct VkOffset3D dstOffset = vk_image_sanitize_offset(&dst_image->vk, region->dstOffset);

   VkRect2D resolve_area = {
      .offset = {dstOffset.x, dstOffset.y},
      .extent = {extent.width, extent.height},
   };

   radv_CmdSetViewport(radv_cmd_buffer_to_handle(cmd_buffer), 0, 1,
                       &(VkViewport){.x = resolve_area.offset.x,
                                     .y = resolve_area.offset.y,
                                     .width = resolve_area.extent.width,
                                     .height = resolve_area.extent.height,
                                     .minDepth = 0.0f,
                                     .maxDepth = 1.0f});

   radv_CmdSetScissor(radv_cmd_buffer_to_handle(cmd_buffer), 0, 1, &resolve_area);

   struct radv_image_view src_iview;
   radv_image_view_init(&src_iview, cmd_buffer->device,
                        &(VkImageViewCreateInfo){
                           .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                           .image = radv_image_to_handle(src_image),
                           .viewType = VK_IMAGE_VIEW_TYPE_2D,
                           .format = src_image->vk.format,
                           .subresourceRange =
                              {
                                 .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1,
                              },
                        },
                        0, NULL);

   struct radv_image_view dst_iview;
   radv_image_view_init(&dst_iview, cmd_buffer->device,
                        &(VkImageViewCreateInfo){
                           .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                           .image = radv_image_to_handle(dst_image),
                           .viewType = radv_meta_get_view_type(dst_image),
                           .format = dst_image->vk.format,
                           .subresourceRange =
                              {
                                 .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = region->dstSubresource.mipLevel,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1,
                              },
                        },
                        0, NULL);

   const VkRenderingAttachmentInfo color_att = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = radv_image_view_to_handle(&dst_iview),
      .imageLayout = layout,
      .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
   };

   const VkRenderingInfo rendering_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = resolve_area,
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_att,
   };

   radv_CmdBeginRendering(radv_cmd_buffer_to_handle(cmd_buffer), &rendering_info);

   emit_resolve(cmd_buffer, &src_iview, &dst_iview, &(VkOffset2D){srcOffset.x, srcOffset.y},
                &(VkOffset2D){dstOffset.x, dstOffset.y});

   radv_CmdEndRendering(radv_cmd_buffer_to_handle(cmd_buffer));

   radv_image_view_finish(&src_iview);
   radv_image_view_finish(&dst_iview);

   radv_meta_restore(&saved_state, cmd_buffer);
}

void
radv_cmd_buffer_resolve_rendering_fs(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *src_iview,
                                     VkImageLayout src_layout, struct radv_image_view *dst_iview,
                                     VkImageLayout dst_layout)
{
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   struct radv_meta_saved_state saved_state;
   VkRect2D resolve_area = render->area;

   radv_meta_save(
      &saved_state, cmd_buffer,
      RADV_META_SAVE_GRAPHICS_PIPELINE | RADV_META_SAVE_CONSTANTS | RADV_META_SAVE_DESCRIPTORS | RADV_META_SAVE_RENDER);

   radv_CmdSetViewport(radv_cmd_buffer_to_handle(cmd_buffer), 0, 1,
                       &(VkViewport){.x = resolve_area.offset.x,
                                     .y = resolve_area.offset.y,
                                     .width = resolve_area.extent.width,
                                     .height = resolve_area.extent.height,
                                     .minDepth = 0.0f,
                                     .maxDepth = 1.0f});

   radv_CmdSetScissor(radv_cmd_buffer_to_handle(cmd_buffer), 0, 1, &resolve_area);

   const VkRenderingAttachmentInfo color_att = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = radv_image_view_to_handle(dst_iview),
      .imageLayout = dst_layout,
      .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
   };

   const VkRenderingInfo rendering_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = saved_state.render.area,
      .layerCount = 1,
      .viewMask = saved_state.render.view_mask,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_att,
   };

   radv_CmdBeginRendering(radv_cmd_buffer_to_handle(cmd_buffer), &rendering_info);

   emit_resolve(cmd_buffer, src_iview, dst_iview, &resolve_area.offset, &resolve_area.offset);

   radv_CmdEndRendering(radv_cmd_buffer_to_handle(cmd_buffer));

   radv_meta_restore(&saved_state, cmd_buffer);
}

/**
 * Depth/stencil resolves for the current rendering.
 */
void
radv_depth_stencil_resolve_rendering_fs(struct radv_cmd_buffer *cmd_buffer, VkImageAspectFlags aspects,
                                        VkResolveModeFlagBits resolve_mode)
{
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   VkRect2D resolve_area = render->area;
   struct radv_meta_saved_state saved_state;
   struct radv_resolve_barrier barrier;

   /* Resolves happen before rendering ends, so we have to make the attachment shader-readable */
   barrier.src_stage_mask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
   barrier.dst_stage_mask = VK_PIPELINE_STAGE_2_RESOLVE_BIT;
   barrier.src_access_mask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
   barrier.dst_access_mask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
   radv_emit_resolve_barrier(cmd_buffer, &barrier);

   struct radv_image_view *src_iview = cmd_buffer->state.render.ds_att.iview;
   VkImageLayout src_layout =
      aspects & VK_IMAGE_ASPECT_DEPTH_BIT ? render->ds_att.layout : render->ds_att.stencil_layout;
   struct radv_image *src_image = src_iview->image;

   VkImageResolve2 region = {0};
   region.sType = VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2;
   region.srcSubresource.aspectMask = aspects;
   region.srcSubresource.mipLevel = 0;
   region.srcSubresource.baseArrayLayer = 0;
   region.srcSubresource.layerCount = 1;

   radv_decompress_resolve_src(cmd_buffer, src_image, src_layout, &region);

   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_GRAPHICS_PIPELINE | RADV_META_SAVE_DESCRIPTORS | RADV_META_SAVE_RENDER);

   struct radv_image_view *dst_iview = saved_state.render.ds_att.resolve_iview;

   const VkRenderingAttachmentInfo depth_att = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = radv_image_view_to_handle(dst_iview),
      .imageLayout = saved_state.render.ds_att.resolve_layout,
      .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
   };

   const VkRenderingAttachmentInfo stencil_att = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = radv_image_view_to_handle(dst_iview),
      .imageLayout = saved_state.render.ds_att.stencil_resolve_layout,
      .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
   };

   const VkRenderingInfo rendering_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = saved_state.render.area,
      .layerCount = 1,
      .viewMask = saved_state.render.view_mask,
      .pDepthAttachment = (dst_iview->image->vk.aspects & VK_IMAGE_ASPECT_DEPTH_BIT) ? &depth_att : NULL,
      .pStencilAttachment = (dst_iview->image->vk.aspects & VK_IMAGE_ASPECT_STENCIL_BIT) ? &stencil_att : NULL,
   };

   radv_CmdBeginRendering(radv_cmd_buffer_to_handle(cmd_buffer), &rendering_info);

   struct radv_image_view tsrc_iview;
   radv_image_view_init(&tsrc_iview, cmd_buffer->device,
                        &(VkImageViewCreateInfo){
                           .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                           .image = radv_image_to_handle(src_image),
                           .viewType = VK_IMAGE_VIEW_TYPE_2D,
                           .format = src_iview->vk.format,
                           .subresourceRange =
                              {
                                 .aspectMask = aspects,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1,
                              },
                        },
                        0, NULL);

   emit_depth_stencil_resolve(cmd_buffer, &tsrc_iview, dst_iview, &resolve_area.offset, &resolve_area.extent, aspects,
                              resolve_mode);

   radv_CmdEndRendering(radv_cmd_buffer_to_handle(cmd_buffer));

   radv_image_view_finish(&tsrc_iview);

   radv_meta_restore(&saved_state, cmd_buffer);
}
