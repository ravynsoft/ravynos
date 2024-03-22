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

#include "nir/nir_builder.h"
#include "radv_meta.h"
#include "vk_common_entrypoints.h"

struct blit_region {
   VkOffset3D src_offset;
   VkExtent3D src_extent;
   VkOffset3D dst_offset;
   VkExtent3D dst_extent;
};

static VkResult build_pipeline(struct radv_device *device, VkImageAspectFlagBits aspect, enum glsl_sampler_dim tex_dim,
                               VkFormat format, VkPipeline *pipeline);

static nir_shader *
build_nir_vertex_shader(struct radv_device *dev)
{
   const struct glsl_type *vec4 = glsl_vec4_type();
   nir_builder b = radv_meta_init_shader(dev, MESA_SHADER_VERTEX, "meta_blit_vs");

   nir_variable *pos_out = nir_variable_create(b.shader, nir_var_shader_out, vec4, "gl_Position");
   pos_out->data.location = VARYING_SLOT_POS;

   nir_variable *tex_pos_out = nir_variable_create(b.shader, nir_var_shader_out, vec4, "v_tex_pos");
   tex_pos_out->data.location = VARYING_SLOT_VAR0;
   tex_pos_out->data.interpolation = INTERP_MODE_SMOOTH;

   nir_def *outvec = nir_gen_rect_vertices(&b, NULL, NULL);

   nir_store_var(&b, pos_out, outvec, 0xf);

   nir_def *src_box = nir_load_push_constant(&b, 4, 32, nir_imm_int(&b, 0), .range = 16);
   nir_def *src0_z = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 16, .range = 4);

   nir_def *vertex_id = nir_load_vertex_id_zero_base(&b);

   /* vertex 0 - src0_x, src0_y, src0_z */
   /* vertex 1 - src0_x, src1_y, src0_z*/
   /* vertex 2 - src1_x, src0_y, src0_z */
   /* so channel 0 is vertex_id != 2 ? src_x : src_x + w
      channel 1 is vertex id != 1 ? src_y : src_y + w */

   nir_def *c0cmp = nir_ine_imm(&b, vertex_id, 2);
   nir_def *c1cmp = nir_ine_imm(&b, vertex_id, 1);

   nir_def *comp[4];
   comp[0] = nir_bcsel(&b, c0cmp, nir_channel(&b, src_box, 0), nir_channel(&b, src_box, 2));

   comp[1] = nir_bcsel(&b, c1cmp, nir_channel(&b, src_box, 1), nir_channel(&b, src_box, 3));
   comp[2] = src0_z;
   comp[3] = nir_imm_float(&b, 1.0);
   nir_def *out_tex_vec = nir_vec(&b, comp, 4);
   nir_store_var(&b, tex_pos_out, out_tex_vec, 0xf);
   return b.shader;
}

static nir_shader *
build_nir_copy_fragment_shader(struct radv_device *dev, enum glsl_sampler_dim tex_dim)
{
   const struct glsl_type *vec4 = glsl_vec4_type();
   nir_builder b = radv_meta_init_shader(dev, MESA_SHADER_FRAGMENT, "meta_blit_fs.%d", tex_dim);

   nir_variable *tex_pos_in = nir_variable_create(b.shader, nir_var_shader_in, vec4, "v_tex_pos");
   tex_pos_in->data.location = VARYING_SLOT_VAR0;

   /* Swizzle the array index which comes in as Z coordinate into the right
    * position.
    */
   unsigned swz[] = {0, (tex_dim == GLSL_SAMPLER_DIM_1D ? 2 : 1), 2};
   nir_def *const tex_pos =
      nir_swizzle(&b, nir_load_var(&b, tex_pos_in), swz, (tex_dim == GLSL_SAMPLER_DIM_1D ? 2 : 3));

   const struct glsl_type *sampler_type =
      glsl_sampler_type(tex_dim, false, tex_dim != GLSL_SAMPLER_DIM_3D, glsl_get_base_type(vec4));
   nir_variable *sampler = nir_variable_create(b.shader, nir_var_uniform, sampler_type, "s_tex");
   sampler->data.descriptor_set = 0;
   sampler->data.binding = 0;

   nir_deref_instr *tex_deref = nir_build_deref_var(&b, sampler);
   nir_def *color = nir_tex_deref(&b, tex_deref, tex_deref, tex_pos);

   nir_variable *color_out = nir_variable_create(b.shader, nir_var_shader_out, vec4, "f_color");
   color_out->data.location = FRAG_RESULT_DATA0;
   nir_store_var(&b, color_out, color, 0xf);

   return b.shader;
}

static nir_shader *
build_nir_copy_fragment_shader_depth(struct radv_device *dev, enum glsl_sampler_dim tex_dim)
{
   const struct glsl_type *vec4 = glsl_vec4_type();
   nir_builder b = radv_meta_init_shader(dev, MESA_SHADER_FRAGMENT, "meta_blit_depth_fs.%d", tex_dim);

   nir_variable *tex_pos_in = nir_variable_create(b.shader, nir_var_shader_in, vec4, "v_tex_pos");
   tex_pos_in->data.location = VARYING_SLOT_VAR0;

   /* Swizzle the array index which comes in as Z coordinate into the right
    * position.
    */
   unsigned swz[] = {0, (tex_dim == GLSL_SAMPLER_DIM_1D ? 2 : 1), 2};
   nir_def *const tex_pos =
      nir_swizzle(&b, nir_load_var(&b, tex_pos_in), swz, (tex_dim == GLSL_SAMPLER_DIM_1D ? 2 : 3));

   const struct glsl_type *sampler_type =
      glsl_sampler_type(tex_dim, false, tex_dim != GLSL_SAMPLER_DIM_3D, glsl_get_base_type(vec4));
   nir_variable *sampler = nir_variable_create(b.shader, nir_var_uniform, sampler_type, "s_tex");
   sampler->data.descriptor_set = 0;
   sampler->data.binding = 0;

   nir_deref_instr *tex_deref = nir_build_deref_var(&b, sampler);
   nir_def *color = nir_tex_deref(&b, tex_deref, tex_deref, tex_pos);

   nir_variable *color_out = nir_variable_create(b.shader, nir_var_shader_out, vec4, "f_color");
   color_out->data.location = FRAG_RESULT_DEPTH;
   nir_store_var(&b, color_out, color, 0x1);

   return b.shader;
}

static nir_shader *
build_nir_copy_fragment_shader_stencil(struct radv_device *dev, enum glsl_sampler_dim tex_dim)
{
   const struct glsl_type *vec4 = glsl_vec4_type();
   nir_builder b = radv_meta_init_shader(dev, MESA_SHADER_FRAGMENT, "meta_blit_stencil_fs.%d", tex_dim);

   nir_variable *tex_pos_in = nir_variable_create(b.shader, nir_var_shader_in, vec4, "v_tex_pos");
   tex_pos_in->data.location = VARYING_SLOT_VAR0;

   /* Swizzle the array index which comes in as Z coordinate into the right
    * position.
    */
   unsigned swz[] = {0, (tex_dim == GLSL_SAMPLER_DIM_1D ? 2 : 1), 2};
   nir_def *const tex_pos =
      nir_swizzle(&b, nir_load_var(&b, tex_pos_in), swz, (tex_dim == GLSL_SAMPLER_DIM_1D ? 2 : 3));

   const struct glsl_type *sampler_type =
      glsl_sampler_type(tex_dim, false, tex_dim != GLSL_SAMPLER_DIM_3D, glsl_get_base_type(vec4));
   nir_variable *sampler = nir_variable_create(b.shader, nir_var_uniform, sampler_type, "s_tex");
   sampler->data.descriptor_set = 0;
   sampler->data.binding = 0;

   nir_deref_instr *tex_deref = nir_build_deref_var(&b, sampler);
   nir_def *color = nir_tex_deref(&b, tex_deref, tex_deref, tex_pos);

   nir_variable *color_out = nir_variable_create(b.shader, nir_var_shader_out, vec4, "f_color");
   color_out->data.location = FRAG_RESULT_STENCIL;
   nir_store_var(&b, color_out, color, 0x1);

   return b.shader;
}

static enum glsl_sampler_dim
translate_sampler_dim(VkImageType type)
{
   switch (type) {
   case VK_IMAGE_TYPE_1D:
      return GLSL_SAMPLER_DIM_1D;
   case VK_IMAGE_TYPE_2D:
      return GLSL_SAMPLER_DIM_2D;
   case VK_IMAGE_TYPE_3D:
      return GLSL_SAMPLER_DIM_3D;
   default:
      unreachable("Unhandled image type");
   }
}

static void
meta_emit_blit(struct radv_cmd_buffer *cmd_buffer, struct radv_image *src_image, struct radv_image_view *src_iview,
               VkImageLayout src_image_layout, float src_offset_0[3], float src_offset_1[3],
               struct radv_image *dst_image, struct radv_image_view *dst_iview, VkImageLayout dst_image_layout,
               VkRect2D dst_box, VkSampler sampler)
{
   struct radv_device *device = cmd_buffer->device;
   uint32_t src_width = radv_minify(src_iview->image->vk.extent.width, src_iview->vk.base_mip_level);
   uint32_t src_height = radv_minify(src_iview->image->vk.extent.height, src_iview->vk.base_mip_level);
   uint32_t src_depth = radv_minify(src_iview->image->vk.extent.depth, src_iview->vk.base_mip_level);
   uint32_t dst_width = radv_minify(dst_iview->image->vk.extent.width, dst_iview->vk.base_mip_level);
   uint32_t dst_height = radv_minify(dst_iview->image->vk.extent.height, dst_iview->vk.base_mip_level);

   assert(src_image->vk.samples == dst_image->vk.samples);

   float vertex_push_constants[5] = {
      src_offset_0[0] / (float)src_width,  src_offset_0[1] / (float)src_height, src_offset_1[0] / (float)src_width,
      src_offset_1[1] / (float)src_height, src_offset_0[2] / (float)src_depth,
   };

   vk_common_CmdPushConstants(radv_cmd_buffer_to_handle(cmd_buffer), device->meta_state.blit.pipeline_layout,
                              VK_SHADER_STAGE_VERTEX_BIT, 0, 20, vertex_push_constants);

   VkPipeline *pipeline = NULL;
   unsigned fs_key = 0;
   VkFormat format = VK_FORMAT_UNDEFINED;

   switch (src_iview->vk.aspects) {
   case VK_IMAGE_ASPECT_COLOR_BIT: {
      fs_key = radv_format_meta_fs_key(device, dst_image->vk.format);
      format = radv_fs_key_format_exemplars[fs_key];

      switch (src_image->vk.image_type) {
      case VK_IMAGE_TYPE_1D:
         pipeline = &device->meta_state.blit.pipeline_1d_src[fs_key];
         break;
      case VK_IMAGE_TYPE_2D:
         pipeline = &device->meta_state.blit.pipeline_2d_src[fs_key];
         break;
      case VK_IMAGE_TYPE_3D:
         pipeline = &device->meta_state.blit.pipeline_3d_src[fs_key];
         break;
      default:
         unreachable("bad VkImageType");
      }
      break;
   }
   case VK_IMAGE_ASPECT_DEPTH_BIT: {
      format = VK_FORMAT_D32_SFLOAT;

      switch (src_image->vk.image_type) {
      case VK_IMAGE_TYPE_1D:
         pipeline = &device->meta_state.blit.depth_only_1d_pipeline;
         break;
      case VK_IMAGE_TYPE_2D:
         pipeline = &device->meta_state.blit.depth_only_2d_pipeline;
         break;
      case VK_IMAGE_TYPE_3D:
         pipeline = &device->meta_state.blit.depth_only_3d_pipeline;
         break;
      default:
         unreachable("bad VkImageType");
      }
      break;
   }
   case VK_IMAGE_ASPECT_STENCIL_BIT: {
      format = VK_FORMAT_S8_UINT;

      switch (src_image->vk.image_type) {
      case VK_IMAGE_TYPE_1D:
         pipeline = &device->meta_state.blit.stencil_only_1d_pipeline;
         break;
      case VK_IMAGE_TYPE_2D:
         pipeline = &device->meta_state.blit.stencil_only_2d_pipeline;
         break;
      case VK_IMAGE_TYPE_3D:
         pipeline = &device->meta_state.blit.stencil_only_3d_pipeline;
         break;
      default:
         unreachable("bad VkImageType");
      }
      break;
   }
   default:
      unreachable("bad VkImageType");
   }

   if (!*pipeline) {
      VkResult ret = build_pipeline(device, src_iview->vk.aspects, translate_sampler_dim(src_image->vk.image_type),
                                    format, pipeline);
      if (ret != VK_SUCCESS) {
         vk_command_buffer_set_error(&cmd_buffer->vk, ret);
         return;
      }
   }

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

   radv_meta_push_descriptor_set(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, device->meta_state.blit.pipeline_layout,
                                 0, /* set */
                                 1, /* descriptorWriteCount */
                                 (VkWriteDescriptorSet[]){{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                           .dstBinding = 0,
                                                           .dstArrayElement = 0,
                                                           .descriptorCount = 1,
                                                           .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                           .pImageInfo = (VkDescriptorImageInfo[]){
                                                              {
                                                                 .sampler = sampler,
                                                                 .imageView = radv_image_view_to_handle(src_iview),
                                                                 .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                                                              },
                                                           }}});

   VkRenderingInfo rendering_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea =
         {
            .offset = {0, 0},
            .extent = {dst_width, dst_height},
         },
      .layerCount = 1,
   };

   VkRenderingAttachmentInfo color_att;
   if (src_iview->image->vk.aspects == VK_IMAGE_ASPECT_COLOR_BIT) {
      unsigned dst_layout = radv_meta_dst_layout_from_layout(dst_image_layout);
      VkImageLayout layout = radv_meta_dst_layout_to_layout(dst_layout);

      color_att = (VkRenderingAttachmentInfo){
         .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
         .imageView = radv_image_view_to_handle(dst_iview),
         .imageLayout = layout,
         .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      };
      rendering_info.colorAttachmentCount = 1;
      rendering_info.pColorAttachments = &color_att;
   }

   VkRenderingAttachmentInfo depth_att;
   if (src_iview->image->vk.aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
      enum radv_blit_ds_layout ds_layout = radv_meta_blit_ds_to_type(dst_image_layout);
      VkImageLayout layout = radv_meta_blit_ds_to_layout(ds_layout);

      depth_att = (VkRenderingAttachmentInfo){
         .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
         .imageView = radv_image_view_to_handle(dst_iview),
         .imageLayout = layout,
         .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      };
      rendering_info.pDepthAttachment = &depth_att;
   }

   VkRenderingAttachmentInfo stencil_att;
   if (src_iview->image->vk.aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      enum radv_blit_ds_layout ds_layout = radv_meta_blit_ds_to_type(dst_image_layout);
      VkImageLayout layout = radv_meta_blit_ds_to_layout(ds_layout);

      stencil_att = (VkRenderingAttachmentInfo){
         .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
         .imageView = radv_image_view_to_handle(dst_iview),
         .imageLayout = layout,
         .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      };
      rendering_info.pStencilAttachment = &stencil_att;
   }

   radv_CmdBeginRendering(radv_cmd_buffer_to_handle(cmd_buffer), &rendering_info);

   radv_CmdDraw(radv_cmd_buffer_to_handle(cmd_buffer), 3, 1, 0, 0);

   radv_CmdEndRendering(radv_cmd_buffer_to_handle(cmd_buffer));
}

static bool
flip_coords(unsigned *src0, unsigned *src1, unsigned *dst0, unsigned *dst1)
{
   bool flip = false;
   if (*src0 > *src1) {
      unsigned tmp = *src0;
      *src0 = *src1;
      *src1 = tmp;
      flip = !flip;
   }

   if (*dst0 > *dst1) {
      unsigned tmp = *dst0;
      *dst0 = *dst1;
      *dst1 = tmp;
      flip = !flip;
   }
   return flip;
}

static void
blit_image(struct radv_cmd_buffer *cmd_buffer, struct radv_image *src_image, VkImageLayout src_image_layout,
           struct radv_image *dst_image, VkImageLayout dst_image_layout, const VkImageBlit2 *region, VkFilter filter)
{
   const VkImageSubresourceLayers *src_res = &region->srcSubresource;
   const VkImageSubresourceLayers *dst_res = &region->dstSubresource;
   struct radv_device *device = cmd_buffer->device;
   struct radv_meta_saved_state saved_state;
   VkSampler sampler;

   /* From the Vulkan 1.0 spec:
    *
    *    vkCmdBlitImage must not be used for multisampled source or
    *    destination images. Use vkCmdResolveImage for this purpose.
    */
   assert(src_image->vk.samples == 1);
   assert(dst_image->vk.samples == 1);

   radv_CreateSampler(radv_device_to_handle(device),
                      &(VkSamplerCreateInfo){
                         .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                         .magFilter = filter,
                         .minFilter = filter,
                         .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                         .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                         .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                      },
                      &cmd_buffer->vk.pool->alloc, &sampler);

   /* VK_EXT_conditional_rendering says that blit commands should not be
    * affected by conditional rendering.
    */
   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_GRAPHICS_PIPELINE | RADV_META_SAVE_CONSTANTS | RADV_META_SAVE_DESCRIPTORS |
                     RADV_META_SUSPEND_PREDICATING);

   unsigned dst_start, dst_end;
   if (dst_image->vk.image_type == VK_IMAGE_TYPE_3D) {
      assert(dst_res->baseArrayLayer == 0);
      dst_start = region->dstOffsets[0].z;
      dst_end = region->dstOffsets[1].z;
   } else {
      dst_start = dst_res->baseArrayLayer;
      dst_end = dst_start + vk_image_subresource_layer_count(&dst_image->vk, dst_res);
   }

   unsigned src_start, src_end;
   if (src_image->vk.image_type == VK_IMAGE_TYPE_3D) {
      assert(src_res->baseArrayLayer == 0);
      src_start = region->srcOffsets[0].z;
      src_end = region->srcOffsets[1].z;
   } else {
      src_start = src_res->baseArrayLayer;
      src_end = src_start + vk_image_subresource_layer_count(&src_image->vk, src_res);
   }

   bool flip_z = flip_coords(&src_start, &src_end, &dst_start, &dst_end);
   float src_z_step = (float)(src_end - src_start) / (float)(dst_end - dst_start);

   /* There is no interpolation to the pixel center during
    * rendering, so add the 0.5 offset ourselves here. */
   float depth_center_offset = 0;
   if (src_image->vk.image_type == VK_IMAGE_TYPE_3D)
      depth_center_offset = 0.5 / (dst_end - dst_start) * (src_end - src_start);

   if (flip_z) {
      src_start = src_end;
      src_z_step *= -1;
      depth_center_offset *= -1;
   }

   unsigned src_x0 = region->srcOffsets[0].x;
   unsigned src_x1 = region->srcOffsets[1].x;
   unsigned dst_x0 = region->dstOffsets[0].x;
   unsigned dst_x1 = region->dstOffsets[1].x;

   unsigned src_y0 = region->srcOffsets[0].y;
   unsigned src_y1 = region->srcOffsets[1].y;
   unsigned dst_y0 = region->dstOffsets[0].y;
   unsigned dst_y1 = region->dstOffsets[1].y;

   VkRect2D dst_box;
   dst_box.offset.x = MIN2(dst_x0, dst_x1);
   dst_box.offset.y = MIN2(dst_y0, dst_y1);
   dst_box.extent.width = dst_x1 - dst_x0;
   dst_box.extent.height = dst_y1 - dst_y0;

   const VkOffset2D dst_offset_0 = {
      .x = dst_x0,
      .y = dst_y0,
   };
   const VkOffset2D dst_offset_1 = {
      .x = dst_x1,
      .y = dst_y1,
   };

   radv_CmdSetViewport(radv_cmd_buffer_to_handle(cmd_buffer), 0, 1,
                       &(VkViewport){.x = dst_offset_0.x,
                                     .y = dst_offset_0.y,
                                     .width = dst_offset_1.x - dst_offset_0.x,
                                     .height = dst_offset_1.y - dst_offset_0.y,
                                     .minDepth = 0.0f,
                                     .maxDepth = 1.0f});

   radv_CmdSetScissor(
      radv_cmd_buffer_to_handle(cmd_buffer), 0, 1,
      &(VkRect2D){
         .offset = (VkOffset2D){MIN2(dst_offset_0.x, dst_offset_1.x), MIN2(dst_offset_0.y, dst_offset_1.y)},
         .extent = (VkExtent2D){abs(dst_offset_1.x - dst_offset_0.x), abs(dst_offset_1.y - dst_offset_0.y)},
      });

   const unsigned num_layers = dst_end - dst_start;
   for (unsigned i = 0; i < num_layers; i++) {
      struct radv_image_view dst_iview, src_iview;

      float src_offset_0[3] = {
         src_x0,
         src_y0,
         src_start + i * src_z_step + depth_center_offset,
      };
      float src_offset_1[3] = {
         src_x1,
         src_y1,
         src_start + i * src_z_step + depth_center_offset,
      };
      const uint32_t dst_array_slice = dst_start + i;

      /* 3D images have just 1 layer */
      const uint32_t src_array_slice = src_image->vk.image_type == VK_IMAGE_TYPE_3D ? 0 : src_start + i;

      radv_image_view_init(&dst_iview, cmd_buffer->device,
                           &(VkImageViewCreateInfo){
                              .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                              .image = radv_image_to_handle(dst_image),
                              .viewType = radv_meta_get_view_type(dst_image),
                              .format = dst_image->vk.format,
                              .subresourceRange = {.aspectMask = dst_res->aspectMask,
                                                   .baseMipLevel = dst_res->mipLevel,
                                                   .levelCount = 1,
                                                   .baseArrayLayer = dst_array_slice,
                                                   .layerCount = 1},
                           },
                           0, NULL);
      radv_image_view_init(&src_iview, cmd_buffer->device,
                           &(VkImageViewCreateInfo){
                              .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                              .image = radv_image_to_handle(src_image),
                              .viewType = radv_meta_get_view_type(src_image),
                              .format = src_image->vk.format,
                              .subresourceRange = {.aspectMask = src_res->aspectMask,
                                                   .baseMipLevel = src_res->mipLevel,
                                                   .levelCount = 1,
                                                   .baseArrayLayer = src_array_slice,
                                                   .layerCount = 1},
                           },
                           0, NULL);
      meta_emit_blit(cmd_buffer, src_image, &src_iview, src_image_layout, src_offset_0, src_offset_1, dst_image,
                     &dst_iview, dst_image_layout, dst_box, sampler);

      radv_image_view_finish(&dst_iview);
      radv_image_view_finish(&src_iview);
   }

   radv_meta_restore(&saved_state, cmd_buffer);

   radv_DestroySampler(radv_device_to_handle(device), sampler, &cmd_buffer->vk.pool->alloc);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_image, src_image, pBlitImageInfo->srcImage);
   RADV_FROM_HANDLE(radv_image, dst_image, pBlitImageInfo->dstImage);

   for (unsigned r = 0; r < pBlitImageInfo->regionCount; r++) {
      blit_image(cmd_buffer, src_image, pBlitImageInfo->srcImageLayout, dst_image, pBlitImageInfo->dstImageLayout,
                 &pBlitImageInfo->pRegions[r], pBlitImageInfo->filter);
   }
}

void
radv_device_finish_meta_blit_state(struct radv_device *device)
{
   struct radv_meta_state *state = &device->meta_state;

   for (unsigned i = 0; i < NUM_META_FS_KEYS; ++i) {
      radv_DestroyPipeline(radv_device_to_handle(device), state->blit.pipeline_1d_src[i], &state->alloc);
      radv_DestroyPipeline(radv_device_to_handle(device), state->blit.pipeline_2d_src[i], &state->alloc);
      radv_DestroyPipeline(radv_device_to_handle(device), state->blit.pipeline_3d_src[i], &state->alloc);
   }

   radv_DestroyPipeline(radv_device_to_handle(device), state->blit.depth_only_1d_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->blit.depth_only_2d_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->blit.depth_only_3d_pipeline, &state->alloc);

   radv_DestroyPipeline(radv_device_to_handle(device), state->blit.stencil_only_1d_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->blit.stencil_only_2d_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->blit.stencil_only_3d_pipeline, &state->alloc);

   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->blit.pipeline_layout, &state->alloc);
   device->vk.dispatch_table.DestroyDescriptorSetLayout(radv_device_to_handle(device), state->blit.ds_layout,
                                                        &state->alloc);
}

static VkResult
build_pipeline(struct radv_device *device, VkImageAspectFlagBits aspect, enum glsl_sampler_dim tex_dim, VkFormat format,
               VkPipeline *pipeline)
{
   VkResult result = VK_SUCCESS;

   mtx_lock(&device->meta_state.mtx);

   if (*pipeline) {
      mtx_unlock(&device->meta_state.mtx);
      return VK_SUCCESS;
   }

   nir_shader *fs;
   nir_shader *vs = build_nir_vertex_shader(device);

   VkPipelineRenderingCreateInfo rendering_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
   };

   switch (aspect) {
   case VK_IMAGE_ASPECT_COLOR_BIT:
      fs = build_nir_copy_fragment_shader(device, tex_dim);
      rendering_create_info.colorAttachmentCount = 1;
      rendering_create_info.pColorAttachmentFormats = &format;
      break;
   case VK_IMAGE_ASPECT_DEPTH_BIT:
      fs = build_nir_copy_fragment_shader_depth(device, tex_dim);
      rendering_create_info.depthAttachmentFormat = format;
      break;
   case VK_IMAGE_ASPECT_STENCIL_BIT:
      fs = build_nir_copy_fragment_shader_stencil(device, tex_dim);
      rendering_create_info.stencilAttachmentFormat = format;
      break;
   default:
      unreachable("Unhandled aspect");
   }
   VkPipelineVertexInputStateCreateInfo vi_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
   };

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

   VkGraphicsPipelineCreateInfo vk_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &rendering_create_info,
      .stageCount = ARRAY_SIZE(pipeline_shader_stages),
      .pStages = pipeline_shader_stages,
      .pVertexInputState = &vi_create_info,
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
                                                   .lineWidth = 1.0f},
      .pMultisampleState =
         &(VkPipelineMultisampleStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = 1,
            .sampleShadingEnable = false,
            .pSampleMask = (VkSampleMask[]){UINT32_MAX},
         },
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
      .layout = device->meta_state.blit.pipeline_layout,
      .renderPass = VK_NULL_HANDLE,
      .subpass = 0,
   };

   VkPipelineColorBlendStateCreateInfo color_blend_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments =
         (VkPipelineColorBlendAttachmentState[]){
            {.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                               VK_COLOR_COMPONENT_B_BIT},
         },
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

   VkPipelineDepthStencilStateCreateInfo depth_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = true,
      .depthWriteEnable = true,
      .depthCompareOp = VK_COMPARE_OP_ALWAYS,
   };

   VkPipelineDepthStencilStateCreateInfo stencil_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = false,
      .depthWriteEnable = false,
      .stencilTestEnable = true,
      .front = {.failOp = VK_STENCIL_OP_REPLACE,
                .passOp = VK_STENCIL_OP_REPLACE,
                .depthFailOp = VK_STENCIL_OP_REPLACE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .compareMask = 0xff,
                .writeMask = 0xff,
                .reference = 0},
      .back = {.failOp = VK_STENCIL_OP_REPLACE,
               .passOp = VK_STENCIL_OP_REPLACE,
               .depthFailOp = VK_STENCIL_OP_REPLACE,
               .compareOp = VK_COMPARE_OP_ALWAYS,
               .compareMask = 0xff,
               .writeMask = 0xff,
               .reference = 0},
      .depthCompareOp = VK_COMPARE_OP_ALWAYS,
   };

   switch (aspect) {
   case VK_IMAGE_ASPECT_COLOR_BIT:
      vk_pipeline_info.pColorBlendState = &color_blend_info;
      break;
   case VK_IMAGE_ASPECT_DEPTH_BIT:
      vk_pipeline_info.pDepthStencilState = &depth_info;
      break;
   case VK_IMAGE_ASPECT_STENCIL_BIT:
      vk_pipeline_info.pDepthStencilState = &stencil_info;
      break;
   default:
      unreachable("Unhandled aspect");
   }

   const struct radv_graphics_pipeline_create_info radv_pipeline_info = {.use_rectlist = true};

   result = radv_graphics_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &vk_pipeline_info,
                                          &radv_pipeline_info, &device->meta_state.alloc, pipeline);
   ralloc_free(vs);
   ralloc_free(fs);
   mtx_unlock(&device->meta_state.mtx);
   return result;
}

static VkResult
radv_device_init_meta_blit_color(struct radv_device *device, bool on_demand)
{
   VkResult result;

   for (unsigned i = 0; i < NUM_META_FS_KEYS; ++i) {
      VkFormat format = radv_fs_key_format_exemplars[i];
      unsigned key = radv_format_meta_fs_key(device, format);

      if (on_demand)
         continue;

      result = build_pipeline(device, VK_IMAGE_ASPECT_COLOR_BIT, GLSL_SAMPLER_DIM_1D, format,
                              &device->meta_state.blit.pipeline_1d_src[key]);
      if (result != VK_SUCCESS)
         goto fail;

      result = build_pipeline(device, VK_IMAGE_ASPECT_COLOR_BIT, GLSL_SAMPLER_DIM_2D, format,
                              &device->meta_state.blit.pipeline_2d_src[key]);
      if (result != VK_SUCCESS)
         goto fail;

      result = build_pipeline(device, VK_IMAGE_ASPECT_COLOR_BIT, GLSL_SAMPLER_DIM_3D, format,
                              &device->meta_state.blit.pipeline_3d_src[key]);
      if (result != VK_SUCCESS)
         goto fail;
   }

   result = VK_SUCCESS;
fail:
   return result;
}

static VkResult
radv_device_init_meta_blit_depth(struct radv_device *device, bool on_demand)
{
   VkResult result;

   if (on_demand)
      return VK_SUCCESS;

   result = build_pipeline(device, VK_IMAGE_ASPECT_DEPTH_BIT, GLSL_SAMPLER_DIM_1D, VK_FORMAT_D32_SFLOAT,
                           &device->meta_state.blit.depth_only_1d_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   result = build_pipeline(device, VK_IMAGE_ASPECT_DEPTH_BIT, GLSL_SAMPLER_DIM_2D, VK_FORMAT_D32_SFLOAT,
                           &device->meta_state.blit.depth_only_2d_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   result = build_pipeline(device, VK_IMAGE_ASPECT_DEPTH_BIT, GLSL_SAMPLER_DIM_3D, VK_FORMAT_D32_SFLOAT,
                           &device->meta_state.blit.depth_only_3d_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

fail:
   return result;
}

static VkResult
radv_device_init_meta_blit_stencil(struct radv_device *device, bool on_demand)
{
   VkResult result;

   if (on_demand)
      return VK_SUCCESS;

   result = build_pipeline(device, VK_IMAGE_ASPECT_STENCIL_BIT, GLSL_SAMPLER_DIM_1D, VK_FORMAT_S8_UINT,
                           &device->meta_state.blit.stencil_only_1d_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   result = build_pipeline(device, VK_IMAGE_ASPECT_STENCIL_BIT, GLSL_SAMPLER_DIM_2D, VK_FORMAT_S8_UINT,
                           &device->meta_state.blit.stencil_only_2d_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   result = build_pipeline(device, VK_IMAGE_ASPECT_STENCIL_BIT, GLSL_SAMPLER_DIM_3D, VK_FORMAT_S8_UINT,
                           &device->meta_state.blit.stencil_only_3d_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

fail:
   return result;
}

VkResult
radv_device_init_meta_blit_state(struct radv_device *device, bool on_demand)
{
   VkResult result;

   VkDescriptorSetLayoutCreateInfo ds_layout_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                     .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
                                                     .bindingCount = 1,
                                                     .pBindings = (VkDescriptorSetLayoutBinding[]){
                                                        {.binding = 0,
                                                         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         .descriptorCount = 1,
                                                         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                         .pImmutableSamplers = NULL},
                                                     }};
   result = radv_CreateDescriptorSetLayout(radv_device_to_handle(device), &ds_layout_info, &device->meta_state.alloc,
                                           &device->meta_state.blit.ds_layout);
   if (result != VK_SUCCESS)
      return result;

   const VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 20};

   result = radv_CreatePipelineLayout(radv_device_to_handle(device),
                                      &(VkPipelineLayoutCreateInfo){
                                         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                         .setLayoutCount = 1,
                                         .pSetLayouts = &device->meta_state.blit.ds_layout,
                                         .pushConstantRangeCount = 1,
                                         .pPushConstantRanges = &push_constant_range,
                                      },
                                      &device->meta_state.alloc, &device->meta_state.blit.pipeline_layout);
   if (result != VK_SUCCESS)
      return result;

   result = radv_device_init_meta_blit_color(device, on_demand);
   if (result != VK_SUCCESS)
      return result;

   result = radv_device_init_meta_blit_depth(device, on_demand);
   if (result != VK_SUCCESS)
      return result;

   return radv_device_init_meta_blit_stencil(device, on_demand);
}
