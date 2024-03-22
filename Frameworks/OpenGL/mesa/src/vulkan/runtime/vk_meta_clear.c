/*
 * Copyright © 2022 Collabora Ltd
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

#include "vk_meta_private.h"

#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_format.h"
#include "vk_image.h"
#include "vk_pipeline.h"
#include "vk_util.h"

#include "nir_builder.h"

struct vk_meta_clear_key {
   enum vk_meta_object_key_type key_type;
   struct vk_meta_rendering_info render;
   uint8_t color_attachments_cleared;
   bool clear_depth;
   bool clear_stencil;
};

struct vk_meta_clear_push_data {
   VkClearColorValue color_values[MESA_VK_MAX_COLOR_ATTACHMENTS];
};

static nir_shader *
build_clear_shader(const struct vk_meta_clear_key *key)
{
   nir_builder build = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,
                                                      NULL, "vk-meta-clear");
   nir_builder *b = &build;

   struct glsl_struct_field push_field = {
      .type = glsl_array_type(glsl_vec4_type(),
                              MESA_VK_MAX_COLOR_ATTACHMENTS,
                              16 /* explicit_stride */),
      .name = "color_values",
   };
   const struct glsl_type *push_iface_type =
      glsl_interface_type(&push_field, 1, GLSL_INTERFACE_PACKING_STD140,
                          false /* row_major */, "push");

   nir_variable *push = nir_variable_create(b->shader, nir_var_mem_push_const,
                                            push_iface_type, "push");
   nir_deref_instr *push_arr =
      nir_build_deref_struct(b, nir_build_deref_var(b, push), 0);

   u_foreach_bit(a, key->color_attachments_cleared) {
      nir_def *color_value =
         nir_load_deref(b, nir_build_deref_array_imm(b, push_arr, a));

      const struct glsl_type *out_type;
      if (vk_format_is_int(key->render.color_attachment_formats[a]))
         out_type = glsl_ivec4_type();
      else if (vk_format_is_uint(key->render.color_attachment_formats[a]))
         out_type = glsl_uvec4_type();
      else
         out_type = glsl_vec4_type();

      char out_name[8];
      snprintf(out_name, sizeof(out_name), "color%u", a);

      nir_variable *out = nir_variable_create(b->shader, nir_var_shader_out,
                                              out_type, out_name);
      out->data.location = FRAG_RESULT_DATA0 + a;

      nir_store_var(b, out, color_value, 0xf);
   }

   return b->shader;
}

static VkResult
get_clear_pipeline_layout(struct vk_device *device,
                          struct vk_meta_device *meta,
                          VkPipelineLayout *layout_out)
{
   const char key[] = "vk-meta-clear-pipeline-layout";

   VkPipelineLayout from_cache =
      vk_meta_lookup_pipeline_layout(meta, key, sizeof(key));
   if (from_cache != VK_NULL_HANDLE) {
      *layout_out = from_cache;
      return VK_SUCCESS;
   }

   const VkPushConstantRange push_range = {
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(struct vk_meta_clear_push_data),
   };

   const VkPipelineLayoutCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_range,
   };

   return vk_meta_create_pipeline_layout(device, meta, &info,
                                         key, sizeof(key), layout_out);
}

static VkResult
get_clear_pipeline(struct vk_device *device,
                   struct vk_meta_device *meta,
                   const struct vk_meta_clear_key *key,
                   VkPipelineLayout layout,
                   VkPipeline *pipeline_out)
{
   VkPipeline from_cache = vk_meta_lookup_pipeline(meta, key, sizeof(*key));
   if (from_cache != VK_NULL_HANDLE) {
      *pipeline_out = from_cache;
      return VK_SUCCESS;
   }

   const VkPipelineShaderStageNirCreateInfoMESA fs_nir_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NIR_CREATE_INFO_MESA,
      .nir = build_clear_shader(key),
   };
   const VkPipelineShaderStageCreateInfo fs_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = &fs_nir_info,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pName = "main",
   };

   VkPipelineDepthStencilStateCreateInfo ds_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
   };
   const VkDynamicState dyn_stencil_ref = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
   VkPipelineDynamicStateCreateInfo dyn_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
   };
   if (key->clear_depth) {
      ds_info.depthTestEnable = VK_TRUE;
      ds_info.depthWriteEnable = VK_TRUE;
      ds_info.depthCompareOp = VK_COMPARE_OP_ALWAYS;
   }
   if (key->clear_stencil) {
      ds_info.stencilTestEnable = VK_TRUE;
      ds_info.front.compareOp = VK_COMPARE_OP_ALWAYS;
      ds_info.front.passOp = VK_STENCIL_OP_REPLACE;
      ds_info.front.compareMask = ~0u;
      ds_info.front.writeMask = ~0u;
      ds_info.back = ds_info.front;
      dyn_info.dynamicStateCount = 1;
      dyn_info.pDynamicStates = &dyn_stencil_ref;
   }

   const VkGraphicsPipelineCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 1,
      .pStages = &fs_info,
      .pDepthStencilState = &ds_info,
      .pDynamicState = &dyn_info,
      .layout = layout,
   };

   VkResult result = vk_meta_create_graphics_pipeline(device, meta, &info,
                                                      &key->render,
                                                      key, sizeof(*key),
                                                      pipeline_out);
   ralloc_free(fs_nir_info.nir);

   return result;
}

static int
vk_meta_rect_cmp_layer(const void *_a, const void *_b)
{
   const struct vk_meta_rect *a = _a, *b = _b;
   assert(a->layer <= INT_MAX && b->layer <= INT_MAX);
   return a->layer - b->layer;
}

void
vk_meta_clear_attachments(struct vk_command_buffer *cmd,
                          struct vk_meta_device *meta,
                          const struct vk_meta_rendering_info *render,
                          uint32_t attachment_count,
                          const VkClearAttachment *attachments,
                          uint32_t clear_rect_count,
                          const VkClearRect *clear_rects)
{
   struct vk_device *device = cmd->base.device;
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkResult result;

   struct vk_meta_clear_key key;
   memset(&key, 0, sizeof(key));
   key.key_type = VK_META_OBJECT_KEY_CLEAR_PIPELINE;
   vk_meta_rendering_info_copy(&key.render, render);

   struct vk_meta_clear_push_data push = {0};
   float depth_value = 1.0f;
   uint32_t stencil_value = 0;

   for (uint32_t i = 0; i < attachment_count; i++) {
      if (attachments[i].aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
         const uint32_t a = attachments[i].colorAttachment;
         if (a == VK_ATTACHMENT_UNUSED)
            continue;

         assert(a < MESA_VK_MAX_COLOR_ATTACHMENTS);
         if (render->color_attachment_formats[a] == VK_FORMAT_UNDEFINED)
            continue;

         key.color_attachments_cleared |= BITFIELD_BIT(a);
         push.color_values[a] = attachments[i].clearValue.color;
      }
      if (attachments[i].aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
         key.clear_depth = true;
         depth_value = attachments[i].clearValue.depthStencil.depth;
      }
      if (attachments[i].aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
         key.clear_stencil = true;
         stencil_value = attachments[i].clearValue.depthStencil.stencil;
      }
   }

   VkPipelineLayout layout;
   result = get_clear_pipeline_layout(device, meta, &layout);
   if (unlikely(result != VK_SUCCESS)) {
      /* TODO: Report error */
      return;
   }

   VkPipeline pipeline;
   result = get_clear_pipeline(device, meta, &key, layout, &pipeline);
   if (unlikely(result != VK_SUCCESS)) {
      /* TODO: Report error */
      return;
   }

   disp->CmdBindPipeline(vk_command_buffer_to_handle(cmd),
                         VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

   if (key.clear_stencil) {
      disp->CmdSetStencilReference(vk_command_buffer_to_handle(cmd),
                                   VK_STENCIL_FACE_FRONT_AND_BACK,
                                   stencil_value);
   }

   disp->CmdPushConstants(vk_command_buffer_to_handle(cmd),
                          layout, VK_SHADER_STAGE_FRAGMENT_BIT,
                          0, sizeof(push), &push);

   if (render->view_mask == 0) {
      if (clear_rect_count == 1 && clear_rects[0].layerCount > 1) {
         struct vk_meta_rect rect = {
            .x0 = clear_rects[0].rect.offset.x,
            .x1 = clear_rects[0].rect.offset.x +
                  clear_rects[0].rect.extent.width,
            .y0 = clear_rects[0].rect.offset.y,
            .y1 = clear_rects[0].rect.offset.y +
                  clear_rects[0].rect.extent.height,
            .z = depth_value,
            .layer = clear_rects[0].baseArrayLayer,
         };

         meta->cmd_draw_volume(cmd, meta, &rect, clear_rects[0].layerCount);
      } else {
         uint32_t max_rect_count = 0;
         for (uint32_t r = 0; r < clear_rect_count; r++)
            max_rect_count += clear_rects[r].layerCount;

         STACK_ARRAY(struct vk_meta_rect, rects, max_rect_count);

         uint32_t rect_count = 0;
         for (uint32_t r = 0; r < clear_rect_count; r++) {
            struct vk_meta_rect rect = {
               .x0 = clear_rects[r].rect.offset.x,
               .x1 = clear_rects[r].rect.offset.x +
                     clear_rects[r].rect.extent.width,
               .y0 = clear_rects[r].rect.offset.y,
               .y1 = clear_rects[r].rect.offset.y +
                     clear_rects[r].rect.extent.height,
               .z = depth_value,
            };
            for (uint32_t a = 0; a < clear_rects[r].layerCount; a++) {
               rect.layer = clear_rects[r].baseArrayLayer + a;
               rects[rect_count++] = rect;
            }
         }
         assert(rect_count <= max_rect_count);

         /* If we have more than one clear rect, sort by layer in the hopes
          * the hardware more or less does all the clears for one layer before
          * moving on to the next, thus reducing cache thrashing.
          */
         qsort(rects, rect_count, sizeof(*rects), vk_meta_rect_cmp_layer);

         meta->cmd_draw_rects(cmd, meta, rect_count, rects);

         STACK_ARRAY_FINISH(rects);
      }
   } else {
      const uint32_t rect_count = clear_rect_count *
                                  util_bitcount(render->view_mask);
      STACK_ARRAY(struct vk_meta_rect, rects, rect_count);

      uint32_t rect_idx = 0;
      u_foreach_bit(v, render->view_mask) {
         for (uint32_t r = 0; r < clear_rect_count; r++) {
            assert(clear_rects[r].baseArrayLayer == 0);
            assert(clear_rects[r].layerCount == 1);
            rects[rect_idx++] = (struct vk_meta_rect) {
               .x0 = clear_rects[r].rect.offset.x,
               .x1 = clear_rects[r].rect.offset.x +
                     clear_rects[r].rect.extent.width,
               .y0 = clear_rects[r].rect.offset.y,
               .y1 = clear_rects[r].rect.offset.y +
                     clear_rects[r].rect.extent.height,
               .z = depth_value,
               .layer = v,
            };
         }
      }
      assert(rect_idx == rect_count);

      meta->cmd_draw_rects(cmd, meta, rect_count, rects);

      STACK_ARRAY_FINISH(rects);
   }
}

void
vk_meta_clear_rendering(struct vk_meta_device *meta,
                        struct vk_command_buffer *cmd,
                        const VkRenderingInfo *pRenderingInfo)
{
   assert(!(pRenderingInfo->flags & VK_RENDERING_RESUMING_BIT));

   struct vk_meta_rendering_info render = {
      .view_mask = pRenderingInfo->viewMask,
      .color_attachment_count = pRenderingInfo->colorAttachmentCount,
   };

   uint32_t clear_count = 0;
   VkClearAttachment clear_att[MESA_VK_MAX_COLOR_ATTACHMENTS + 1];
   for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; i++) {
      const VkRenderingAttachmentInfo *att_info =
         &pRenderingInfo->pColorAttachments[i];
      if (att_info->imageView == VK_NULL_HANDLE ||
          att_info->loadOp != VK_ATTACHMENT_LOAD_OP_CLEAR)
         continue;

      VK_FROM_HANDLE(vk_image_view, iview, att_info->imageView);
      render.color_attachment_formats[i] = iview->format;
      assert(render.samples == 0 || render.samples == iview->image->samples);
      render.samples = MAX2(render.samples, iview->image->samples);

      clear_att[clear_count++] = (VkClearAttachment) {
         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .colorAttachment = i,
         .clearValue = att_info->clearValue,
      };
   }

   /* One more for depth/stencil, if needed */
   clear_att[clear_count] = (VkClearAttachment) { .aspectMask = 0, };

   const VkRenderingAttachmentInfo *d_att_info =
      pRenderingInfo->pDepthAttachment;
   if (d_att_info != NULL && d_att_info->imageView != VK_NULL_HANDLE &&
       d_att_info->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
      VK_FROM_HANDLE(vk_image_view, iview, d_att_info->imageView);
      render.depth_attachment_format = iview->format;
      render.samples = MAX2(render.samples, iview->image->samples);

      clear_att[clear_count].aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
      clear_att[clear_count].clearValue.depthStencil.depth =
         d_att_info->clearValue.depthStencil.depth;
   }

   const VkRenderingAttachmentInfo *s_att_info =
      pRenderingInfo->pStencilAttachment;
   if (s_att_info != NULL && s_att_info->imageView != VK_NULL_HANDLE &&
       s_att_info->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
      VK_FROM_HANDLE(vk_image_view, iview, s_att_info->imageView);
      render.stencil_attachment_format = iview->format;
      render.samples = MAX2(render.samples, iview->image->samples);

      clear_att[clear_count].aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
      clear_att[clear_count].clearValue.depthStencil.stencil =
         s_att_info->clearValue.depthStencil.depth;
   }
   if (clear_att[clear_count].aspectMask != 0)
      clear_count++;

   if (clear_count > 0) {
      const VkClearRect clear_rect = {
         .rect = pRenderingInfo->renderArea,
         .baseArrayLayer = 0,
         .layerCount = pRenderingInfo->viewMask ?
                       1 : pRenderingInfo->layerCount,
      };
      vk_meta_clear_attachments(cmd, meta, &render,
                                clear_count, clear_att,
                                1, &clear_rect);
   }
}

static void
clear_image_level_layers(struct vk_command_buffer *cmd,
                         struct vk_meta_device *meta,
                         struct vk_image *image,
                         VkImageLayout image_layout,
                         VkFormat format,
                         const VkClearValue *clear_value,
                         VkImageAspectFlags aspects,
                         uint32_t level,
                         uint32_t base_array_layer,
                         uint32_t layer_count)
{
   struct vk_device *device = cmd->base.device;
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkCommandBuffer _cmd = vk_command_buffer_to_handle(cmd);
   VkResult result;

   const VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = vk_image_to_handle(image),
      .viewType = vk_image_render_view_type(image, layer_count),
      .format = format,
      .subresourceRange = {
         .aspectMask = aspects,
         .baseMipLevel = level,
         .levelCount = 1,
         .baseArrayLayer = base_array_layer,
         .layerCount = layer_count,
      }
   };

   VkImageView image_view;
   result = vk_meta_create_image_view(cmd, meta, &view_info, &image_view);
   if (unlikely(result != VK_SUCCESS)) {
      /* TODO: Report error */
      return;
   }

   const VkExtent3D level_extent = vk_image_mip_level_extent(image, level);

   VkRenderingAttachmentInfo vk_att = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = image_view,
      .imageLayout = image_layout,
      .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
   };
   VkRenderingInfo vk_render = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {
         .offset = { 0, 0 },
         .extent = { level_extent.width, level_extent.height },
      },
      .layerCount = layer_count,
   };
   struct vk_meta_rendering_info meta_render = {
      .samples = image->samples,
   };

   if (image->aspects == VK_IMAGE_ASPECT_COLOR_BIT) {
      vk_render.colorAttachmentCount = 1;
      vk_render.pColorAttachments = &vk_att;
      meta_render.color_attachment_count = 1;
      meta_render.color_attachment_formats[0] = format;
   }

   if (image->aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
      vk_render.pDepthAttachment = &vk_att;
      meta_render.depth_attachment_format = format;
   }

   if (image->aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      vk_render.pStencilAttachment = &vk_att;
      meta_render.stencil_attachment_format = format;
   }

   const VkClearAttachment clear_att = {
      .aspectMask = aspects,
      .colorAttachment = 0,
      .clearValue = *clear_value,
   };

   const VkClearRect clear_rect = {
      .rect = {
         .offset = { 0, 0 },
         .extent = { level_extent.width, level_extent.height },
      },
      .baseArrayLayer = 0,
      .layerCount = layer_count,
   };

   disp->CmdBeginRendering(_cmd, &vk_render);

   vk_meta_clear_attachments(cmd, meta, &meta_render,
                             1, &clear_att, 1, &clear_rect);

   disp->CmdEndRendering(_cmd);
}

static void
clear_image_level(struct vk_command_buffer *cmd,
                  struct vk_meta_device *meta,
                  struct vk_image *image,
                  VkImageLayout image_layout,
                  VkFormat format,
                  const VkClearValue *clear_value,
                  uint32_t level,
                  const VkImageSubresourceRange *range)
{
   const VkExtent3D level_extent = vk_image_mip_level_extent(image, level);

   uint32_t base_array_layer, layer_count;
   if (image->image_type == VK_IMAGE_TYPE_3D) {
      base_array_layer = 0;
      layer_count = level_extent.depth;
   } else {
      base_array_layer = range->baseArrayLayer;
      layer_count = vk_image_subresource_layer_count(image, range);
   }

   if (layer_count > 1 && !meta->use_layered_rendering) {
      for (uint32_t a = 0; a < layer_count; a++) {
         clear_image_level_layers(cmd, meta, image, image_layout,
                                  format, clear_value,
                                  range->aspectMask, level,
                                  base_array_layer + a, 1);
      }
   } else {
      clear_image_level_layers(cmd, meta, image, image_layout,
                               format, clear_value,
                               range->aspectMask, level,
                               base_array_layer, layer_count);
   }
}

void
vk_meta_clear_color_image(struct vk_command_buffer *cmd,
                          struct vk_meta_device *meta,
                          struct vk_image *image,
                          VkImageLayout image_layout,
                          VkFormat format,
                          const VkClearColorValue *color,
                          uint32_t range_count,
                          const VkImageSubresourceRange *ranges)
{
   const VkClearValue clear_value = {
      .color = *color,
   };
   for (uint32_t r = 0; r < range_count; r++) {
      const uint32_t level_count =
         vk_image_subresource_level_count(image, &ranges[r]);

      for (uint32_t l = 0; l < level_count; l++) {
         clear_image_level(cmd, meta, image, image_layout,
                           format, &clear_value,
                           ranges[r].baseMipLevel + l,
                           &ranges[r]);
      }
   }
}

void
vk_meta_clear_depth_stencil_image(struct vk_command_buffer *cmd,
                                  struct vk_meta_device *meta,
                                  struct vk_image *image,
                                  VkImageLayout image_layout,
                                  const VkClearDepthStencilValue *depth_stencil,
                                  uint32_t range_count,
                                  const VkImageSubresourceRange *ranges)
{
   const VkClearValue clear_value = {
      .depthStencil = *depth_stencil,
   };
   for (uint32_t r = 0; r < range_count; r++) {
      const uint32_t level_count =
         vk_image_subresource_level_count(image, &ranges[r]);

      for (uint32_t l = 0; l < level_count; l++) {
         clear_image_level(cmd, meta, image, image_layout,
                           image->format, &clear_value,
                           ranges[r].baseMipLevel + l,
                           &ranges[r]);
      }
   }
}
