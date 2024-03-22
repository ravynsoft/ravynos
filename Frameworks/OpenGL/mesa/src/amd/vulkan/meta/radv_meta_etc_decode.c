/*
 * Copyright © 2021 Google
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

VkResult
radv_device_init_meta_etc_decode_state(struct radv_device *device, bool on_demand)
{
   struct radv_meta_state *state = &device->meta_state;

   if (!device->physical_device->emulate_etc2)
      return VK_SUCCESS;

   state->etc_decode.allocator = &state->alloc;
   state->etc_decode.nir_options = &device->physical_device->nir_options[MESA_SHADER_COMPUTE];
   state->etc_decode.pipeline_cache = state->cache;
   vk_texcompress_etc2_init(&device->vk, &state->etc_decode);

   if (on_demand)
      return VK_SUCCESS;

   return vk_texcompress_etc2_late_init(&device->vk, &state->etc_decode);
}

void
radv_device_finish_meta_etc_decode_state(struct radv_device *device)
{
   struct radv_meta_state *state = &device->meta_state;
   vk_texcompress_etc2_finish(&device->vk, &state->etc_decode);
}

static VkPipeline
radv_get_etc_decode_pipeline(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_device *device = cmd_buffer->device;
   struct radv_meta_state *state = &device->meta_state;
   VkResult ret;

   ret = vk_texcompress_etc2_late_init(&device->vk, &state->etc_decode);
   if (ret != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd_buffer->vk, ret);
      return VK_NULL_HANDLE;
   }

   return state->etc_decode.pipeline;
}

static void
decode_etc(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *src_iview, struct radv_image_view *dst_iview,
           const VkOffset3D *offset, const VkExtent3D *extent)
{
   struct radv_device *device = cmd_buffer->device;
   VkPipeline pipeline = radv_get_etc_decode_pipeline(cmd_buffer);

   radv_meta_push_descriptor_set(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                 device->meta_state.etc_decode.pipeline_layout, 0, /* set */
                                 2,                                                /* descriptorWriteCount */
                                 (VkWriteDescriptorSet[]){{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                           .dstBinding = 0,
                                                           .dstArrayElement = 0,
                                                           .descriptorCount = 1,
                                                           .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                           .pImageInfo =
                                                              (VkDescriptorImageInfo[]){
                                                                 {.sampler = VK_NULL_HANDLE,
                                                                  .imageView = radv_image_view_to_handle(src_iview),
                                                                  .imageLayout = VK_IMAGE_LAYOUT_GENERAL},
                                                              }},
                                                          {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                           .dstBinding = 1,
                                                           .dstArrayElement = 0,
                                                           .descriptorCount = 1,
                                                           .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                           .pImageInfo = (VkDescriptorImageInfo[]){
                                                              {
                                                                 .sampler = VK_NULL_HANDLE,
                                                                 .imageView = radv_image_view_to_handle(dst_iview),
                                                                 .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                                                              },
                                                           }}});

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

   unsigned push_constants[5] = {
      offset->x, offset->y, offset->z, src_iview->image->vk.format, src_iview->image->vk.image_type,
   };

   vk_common_CmdPushConstants(radv_cmd_buffer_to_handle(cmd_buffer), device->meta_state.etc_decode.pipeline_layout,
                              VK_SHADER_STAGE_COMPUTE_BIT, 0, 20, push_constants);
   radv_unaligned_dispatch(cmd_buffer, extent->width, extent->height, extent->depth);
}

void
radv_meta_decode_etc(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, VkImageLayout layout,
                     const VkImageSubresourceLayers *subresource, VkOffset3D offset, VkExtent3D extent)
{
   struct radv_meta_saved_state saved_state;
   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_COMPUTE_PIPELINE | RADV_META_SAVE_CONSTANTS | RADV_META_SAVE_DESCRIPTORS |
                     RADV_META_SUSPEND_PREDICATING);

   uint32_t base_slice = radv_meta_get_iview_layer(image, subresource, &offset);
   uint32_t slice_count = image->vk.image_type == VK_IMAGE_TYPE_3D
                             ? extent.depth
                             : vk_image_subresource_layer_count(&image->vk, subresource);

   extent = vk_image_sanitize_extent(&image->vk, extent);
   offset = vk_image_sanitize_offset(&image->vk, offset);

   VkFormat load_format = vk_texcompress_etc2_load_format(image->vk.format);
   struct radv_image_view src_iview;
   radv_image_view_init(
      &src_iview, cmd_buffer->device,
      &(VkImageViewCreateInfo){
         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image = radv_image_to_handle(image),
         .viewType = vk_texcompress_etc2_image_view_type(image->vk.image_type),
         .format = load_format,
         .subresourceRange =
            {
               .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
               .baseMipLevel = subresource->mipLevel,
               .levelCount = 1,
               .baseArrayLayer = 0,
               .layerCount = subresource->baseArrayLayer + vk_image_subresource_layer_count(&image->vk, subresource),
            },
      },
      0, NULL);

   VkFormat store_format = vk_texcompress_etc2_store_format(image->vk.format);
   struct radv_image_view dst_iview;
   radv_image_view_init(
      &dst_iview, cmd_buffer->device,
      &(VkImageViewCreateInfo){
         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image = radv_image_to_handle(image),
         .viewType = vk_texcompress_etc2_image_view_type(image->vk.image_type),
         .format = store_format,
         .subresourceRange =
            {
               .aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT,
               .baseMipLevel = subresource->mipLevel,
               .levelCount = 1,
               .baseArrayLayer = 0,
               .layerCount = subresource->baseArrayLayer + vk_image_subresource_layer_count(&image->vk, subresource),
            },
      },
      0, NULL);

   decode_etc(cmd_buffer, &src_iview, &dst_iview, &(VkOffset3D){offset.x, offset.y, base_slice},
              &(VkExtent3D){extent.width, extent.height, slice_count});

   radv_image_view_finish(&src_iview);
   radv_image_view_finish(&dst_iview);

   radv_meta_restore(&saved_state, cmd_buffer);
}
