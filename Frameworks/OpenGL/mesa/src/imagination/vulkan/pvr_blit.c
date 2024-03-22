/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "pvr_blit.h"
#include "pvr_clear.h"
#include "pvr_csb.h"
#include "pvr_formats.h"
#include "pvr_job_transfer.h"
#include "pvr_private.h"
#include "pvr_shader_factory.h"
#include "pvr_static_shaders.h"
#include "pvr_types.h"
#include "util/bitscan.h"
#include "util/list.h"
#include "util/macros.h"
#include "util/u_math.h"
#include "vk_alloc.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_format.h"
#include "vk_log.h"

/* TODO: Investigate where this limit comes from. */
#define PVR_MAX_TRANSFER_SIZE_IN_TEXELS 2048U

static struct pvr_transfer_cmd *
pvr_transfer_cmd_alloc(struct pvr_cmd_buffer *cmd_buffer)
{
   struct pvr_transfer_cmd *transfer_cmd;

   transfer_cmd = vk_zalloc(&cmd_buffer->vk.pool->alloc,
                            sizeof(*transfer_cmd),
                            8U,
                            VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!transfer_cmd) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
      return NULL;
   }

   /* transfer_cmd->mapping_count is already set to zero. */
   transfer_cmd->sources[0].filter = PVR_FILTER_POINT;
   transfer_cmd->sources[0].resolve_op = PVR_RESOLVE_BLEND;
   transfer_cmd->sources[0].addr_mode = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
   transfer_cmd->cmd_buffer = cmd_buffer;

   return transfer_cmd;
}

static void pvr_setup_buffer_surface(struct pvr_transfer_cmd_surface *surface,
                                     VkRect2D *rect,
                                     pvr_dev_addr_t dev_addr,
                                     VkDeviceSize offset,
                                     VkFormat vk_format,
                                     VkFormat image_format,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t stride)
{
   enum pipe_format pformat = vk_format_to_pipe_format(image_format);

   surface->dev_addr = PVR_DEV_ADDR_OFFSET(dev_addr, offset);
   surface->width = width;
   surface->height = height;
   surface->stride = stride;
   surface->vk_format = vk_format;
   surface->mem_layout = PVR_MEMLAYOUT_LINEAR;
   surface->sample_count = 1;

   /* Initialize rectangle extent. Also, rectangle.offset should be set to
    * zero, as the offset is already adjusted in the device address above. We
    * don't explicitly set offset to zero as transfer_cmd is zero allocated.
    */
   rect->extent.width = width;
   rect->extent.height = height;

   if (util_format_is_compressed(pformat)) {
      uint32_t block_width = util_format_get_blockwidth(pformat);
      uint32_t block_height = util_format_get_blockheight(pformat);

      surface->width = MAX2(1U, DIV_ROUND_UP(surface->width, block_width));
      surface->height = MAX2(1U, DIV_ROUND_UP(surface->height, block_height));
      surface->stride = MAX2(1U, DIV_ROUND_UP(surface->stride, block_width));

      rect->offset.x /= block_width;
      rect->offset.y /= block_height;
      rect->extent.width =
         MAX2(1U, DIV_ROUND_UP(rect->extent.width, block_width));
      rect->extent.height =
         MAX2(1U, DIV_ROUND_UP(rect->extent.height, block_height));
   }
}

VkFormat pvr_get_raw_copy_format(VkFormat format)
{
   switch (vk_format_get_blocksize(format)) {
   case 1:
      return VK_FORMAT_R8_UINT;
   case 2:
      return VK_FORMAT_R8G8_UINT;
   case 3:
      return VK_FORMAT_R8G8B8_UINT;
   case 4:
      return VK_FORMAT_R32_UINT;
   case 6:
      return VK_FORMAT_R16G16B16_UINT;
   case 8:
      return VK_FORMAT_R32G32_UINT;
   case 12:
      return VK_FORMAT_R32G32B32_UINT;
   case 16:
      return VK_FORMAT_R32G32B32A32_UINT;
   default:
      unreachable("Unhandled copy block size.");
   }
}

static void pvr_setup_transfer_surface(struct pvr_device *device,
                                       struct pvr_transfer_cmd_surface *surface,
                                       VkRect2D *rect,
                                       const struct pvr_image *image,
                                       uint32_t array_layer,
                                       uint32_t mip_level,
                                       const VkOffset3D *offset,
                                       const VkExtent3D *extent,
                                       float fdepth,
                                       VkFormat format,
                                       VkImageAspectFlags aspect_mask)
{
   const uint32_t height = MAX2(image->vk.extent.height >> mip_level, 1U);
   const uint32_t width = MAX2(image->vk.extent.width >> mip_level, 1U);
   enum pipe_format image_pformat = vk_format_to_pipe_format(image->vk.format);
   enum pipe_format pformat = vk_format_to_pipe_format(format);
   const VkImageSubresource sub_resource = {
      .aspectMask = aspect_mask,
      .mipLevel = mip_level,
      .arrayLayer = array_layer,
   };
   VkSubresourceLayout info;
   uint32_t depth;

   if (image->memlayout == PVR_MEMLAYOUT_3DTWIDDLED)
      depth = MAX2(image->vk.extent.depth >> mip_level, 1U);
   else
      depth = 1U;

   pvr_get_image_subresource_layout(image, &sub_resource, &info);

   surface->dev_addr = PVR_DEV_ADDR_OFFSET(image->dev_addr, info.offset);
   surface->width = width;
   surface->height = height;
   surface->depth = depth;

   assert(info.rowPitch % vk_format_get_blocksize(format) == 0);
   surface->stride = info.rowPitch / vk_format_get_blocksize(format);

   surface->vk_format = format;
   surface->mem_layout = image->memlayout;
   surface->sample_count = image->vk.samples;

   if (image->memlayout == PVR_MEMLAYOUT_3DTWIDDLED)
      surface->z_position = fdepth;
   else
      surface->dev_addr.addr += info.depthPitch * ((uint32_t)fdepth);

   rect->offset.x = offset->x;
   rect->offset.y = offset->y;
   rect->extent.width = extent->width;
   rect->extent.height = extent->height;

   if (util_format_is_compressed(image_pformat) &&
       !util_format_is_compressed(pformat)) {
      uint32_t block_width = util_format_get_blockwidth(image_pformat);
      uint32_t block_height = util_format_get_blockheight(image_pformat);

      surface->width = MAX2(1U, DIV_ROUND_UP(surface->width, block_width));
      surface->height = MAX2(1U, DIV_ROUND_UP(surface->height, block_height));
      surface->stride = MAX2(1U, DIV_ROUND_UP(surface->stride, block_width));

      rect->offset.x /= block_width;
      rect->offset.y /= block_height;
      rect->extent.width =
         MAX2(1U, DIV_ROUND_UP(rect->extent.width, block_width));
      rect->extent.height =
         MAX2(1U, DIV_ROUND_UP(rect->extent.height, block_height));
   }
}

void pvr_CmdBlitImage2(VkCommandBuffer commandBuffer,
                          const VkBlitImageInfo2 *pBlitImageInfo)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_image, src, pBlitImageInfo->srcImage);
   PVR_FROM_HANDLE(pvr_image, dst, pBlitImageInfo->dstImage);
   struct pvr_device *device = cmd_buffer->device;
   enum pvr_filter filter = PVR_FILTER_DONTCARE;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   if (pBlitImageInfo->filter == VK_FILTER_LINEAR)
      filter = PVR_FILTER_LINEAR;

   for (uint32_t i = 0U; i < pBlitImageInfo->regionCount; i++) {
      const VkImageBlit2 *region = &pBlitImageInfo->pRegions[i];

      assert(region->srcSubresource.layerCount ==
             region->dstSubresource.layerCount);
      const bool inverted_dst_z =
         (region->dstOffsets[1].z < region->dstOffsets[0].z);
      const bool inverted_src_z =
         (region->srcOffsets[1].z < region->srcOffsets[0].z);
      const uint32_t min_src_z = inverted_src_z ? region->srcOffsets[1].z
                                                : region->srcOffsets[0].z;
      const uint32_t max_src_z = inverted_src_z ? region->srcOffsets[0].z
                                                : region->srcOffsets[1].z;
      const uint32_t min_dst_z = inverted_dst_z ? region->dstOffsets[1].z
                                                : region->dstOffsets[0].z;
      const uint32_t max_dst_z = inverted_dst_z ? region->dstOffsets[0].z
                                                : region->dstOffsets[1].z;

      const uint32_t src_width =
         region->srcOffsets[1].x - region->srcOffsets[0].x;
      const uint32_t src_height =
         region->srcOffsets[1].y - region->srcOffsets[0].y;
      uint32_t dst_width;
      uint32_t dst_height;

      float initial_depth_offset;
      VkExtent3D src_extent;
      VkExtent3D dst_extent;
      VkOffset3D dst_offset = region->dstOffsets[0];
      float z_slice_stride;
      bool flip_x;
      bool flip_y;

      if (region->dstOffsets[1].x > region->dstOffsets[0].x) {
         dst_width = region->dstOffsets[1].x - region->dstOffsets[0].x;
         flip_x = false;
      } else {
         dst_width = region->dstOffsets[0].x - region->dstOffsets[1].x;
         flip_x = true;
         dst_offset.x = region->dstOffsets[1].x;
      }

      if (region->dstOffsets[1].y > region->dstOffsets[0].y) {
         dst_height = region->dstOffsets[1].y - region->dstOffsets[0].y;
         flip_y = false;
      } else {
         dst_height = region->dstOffsets[0].y - region->dstOffsets[1].y;
         flip_y = true;
         dst_offset.y = region->dstOffsets[1].y;
      }

      /* If any of the extent regions is zero, then reject the blit and
       * continue.
       */
      if (!src_width || !src_height || !dst_width || !dst_height ||
          !(max_dst_z - min_dst_z) || !(max_src_z - min_src_z)) {
         mesa_loge("BlitImage: Region %i has an area of zero", i);
         continue;
      }

      src_extent = (VkExtent3D){
         .width = src_width,
         .height = src_height,
         .depth = 0U,
      };

      dst_extent = (VkExtent3D){
         .width = dst_width,
         .height = dst_height,
         .depth = 0U,
      };

      /* The z_position of a transfer surface is intended to be in the range
       * of 0.0f <= z_position <= depth. It will be used as a texture coordinate
       * in the source surface for cases where linear filtering is enabled, so
       * the fractional part will need to represent the exact midpoint of a z
       * slice range in the source texture, as it maps to each destination
       * slice.
       *
       * For destination surfaces, the fractional part is discarded, so
       * we can safely pass the slice index.
       */

      /* Calculate the ratio of z slices in our source region to that of our
       * destination region, to get the number of z slices in our source region
       * to iterate over for each destination slice.
       *
       * If our destination region is inverted, we iterate backwards.
       */
      z_slice_stride =
         (inverted_dst_z ? -1.0f : 1.0f) *
         ((float)(max_src_z - min_src_z) / (float)(max_dst_z - min_dst_z));

      /* Offset the initial depth offset by half of the z slice stride, into the
       * blit region's z range.
       */
      initial_depth_offset =
         (inverted_dst_z ? max_src_z : min_src_z) + (0.5f * z_slice_stride);

      for (uint32_t j = 0U; j < region->srcSubresource.layerCount; j++) {
         struct pvr_transfer_cmd_surface src_surface = { 0 };
         struct pvr_transfer_cmd_surface dst_surface = { 0 };
         VkRect2D src_rect;
         VkRect2D dst_rect;

         /* Get the subresource info for the src and dst images, this is
          * required when incrementing the address of the depth slice used by
          * the transfer surface.
          */
         VkSubresourceLayout src_info, dst_info;
         const VkImageSubresource src_sub_resource = {
            .aspectMask = region->srcSubresource.aspectMask,
            .mipLevel = region->srcSubresource.mipLevel,
            .arrayLayer = region->srcSubresource.baseArrayLayer + j,
         };
         const VkImageSubresource dst_sub_resource = {
            .aspectMask = region->dstSubresource.aspectMask,
            .mipLevel = region->dstSubresource.mipLevel,
            .arrayLayer = region->dstSubresource.baseArrayLayer + j,
         };

         pvr_get_image_subresource_layout(src, &src_sub_resource, &src_info);
         pvr_get_image_subresource_layout(dst, &dst_sub_resource, &dst_info);

         /* Setup the transfer surfaces once per image layer, which saves us
          * from repeating subresource queries by manually incrementing the
          * depth slices.
          */
         pvr_setup_transfer_surface(device,
                                    &src_surface,
                                    &src_rect,
                                    src,
                                    region->srcSubresource.baseArrayLayer + j,
                                    region->srcSubresource.mipLevel,
                                    &region->srcOffsets[0],
                                    &src_extent,
                                    initial_depth_offset,
                                    src->vk.format,
                                    region->srcSubresource.aspectMask);

         pvr_setup_transfer_surface(device,
                                    &dst_surface,
                                    &dst_rect,
                                    dst,
                                    region->dstSubresource.baseArrayLayer + j,
                                    region->dstSubresource.mipLevel,
                                    &dst_offset,
                                    &dst_extent,
                                    min_dst_z,
                                    dst->vk.format,
                                    region->dstSubresource.aspectMask);

         for (uint32_t dst_z = min_dst_z; dst_z < max_dst_z; dst_z++) {
            struct pvr_transfer_cmd *transfer_cmd;
            VkResult result;

            /* TODO: See if we can allocate all the transfer cmds in one go. */
            transfer_cmd = pvr_transfer_cmd_alloc(cmd_buffer);
            if (!transfer_cmd)
               return;

            transfer_cmd->sources[0].mappings[0].src_rect = src_rect;
            transfer_cmd->sources[0].mappings[0].dst_rect = dst_rect;
            transfer_cmd->sources[0].mappings[0].flip_x = flip_x;
            transfer_cmd->sources[0].mappings[0].flip_y = flip_y;
            transfer_cmd->sources[0].mapping_count++;

            transfer_cmd->sources[0].surface = src_surface;
            transfer_cmd->sources[0].filter = filter;
            transfer_cmd->source_count = 1;

            transfer_cmd->dst = dst_surface;
            transfer_cmd->scissor = dst_rect;

            result = pvr_cmd_buffer_add_transfer_cmd(cmd_buffer, transfer_cmd);
            if (result != VK_SUCCESS) {
               vk_free(&cmd_buffer->vk.pool->alloc, transfer_cmd);
               return;
            }

            if (src_surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED) {
               src_surface.z_position += z_slice_stride;
            } else {
               src_surface.dev_addr.addr +=
                  src_info.depthPitch * ((uint32_t)z_slice_stride);
            }

            if (dst_surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED)
               dst_surface.z_position += 1.0f;
            else
               dst_surface.dev_addr.addr += dst_info.depthPitch;
         }
      }
   }
}

static VkFormat pvr_get_copy_format(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_R8_SNORM:
      return VK_FORMAT_R8_SINT;
   case VK_FORMAT_R8G8_SNORM:
      return VK_FORMAT_R8G8_SINT;
   case VK_FORMAT_R8G8B8_SNORM:
      return VK_FORMAT_R8G8B8_SINT;
   case VK_FORMAT_R8G8B8A8_SNORM:
      return VK_FORMAT_R8G8B8A8_SINT;
   case VK_FORMAT_B8G8R8A8_SNORM:
      return VK_FORMAT_B8G8R8A8_SINT;
   default:
      return format;
   }
}

static void
pvr_setup_surface_for_image(struct pvr_device *device,
                            struct pvr_transfer_cmd_surface *surface,
                            VkRect2D *rect,
                            const struct pvr_image *image,
                            uint32_t array_layer,
                            uint32_t array_offset,
                            uint32_t mip_level,
                            const VkOffset3D *offset,
                            const VkExtent3D *extent,
                            uint32_t depth,
                            VkFormat format,
                            const VkImageAspectFlags aspect_mask)
{
   if (image->vk.image_type != VK_IMAGE_TYPE_3D) {
      pvr_setup_transfer_surface(device,
                                 surface,
                                 rect,
                                 image,
                                 array_layer + array_offset,
                                 mip_level,
                                 offset,
                                 extent,
                                 0.0f,
                                 format,
                                 aspect_mask);
   } else {
      pvr_setup_transfer_surface(device,
                                 surface,
                                 rect,
                                 image,
                                 array_layer,
                                 mip_level,
                                 offset,
                                 extent,
                                 (float)depth,
                                 format,
                                 aspect_mask);
   }
}

static VkResult
pvr_copy_or_resolve_image_region(struct pvr_cmd_buffer *cmd_buffer,
                                 enum pvr_resolve_op resolve_op,
                                 const struct pvr_image *src,
                                 const struct pvr_image *dst,
                                 const VkImageCopy2 *region)
{
   enum pipe_format src_pformat = vk_format_to_pipe_format(src->vk.format);
   enum pipe_format dst_pformat = vk_format_to_pipe_format(dst->vk.format);
   bool src_block_compressed = util_format_is_compressed(src_pformat);
   bool dst_block_compressed = util_format_is_compressed(dst_pformat);
   VkExtent3D src_extent;
   VkExtent3D dst_extent;
   VkFormat dst_format;
   VkFormat src_format;
   uint32_t dst_layers;
   uint32_t src_layers;
   uint32_t max_slices;
   uint32_t flags = 0U;

   if (src->vk.format == VK_FORMAT_D24_UNORM_S8_UINT &&
       region->srcSubresource.aspectMask !=
          (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
      /* Takes the stencil of the source and the depth of the destination and
       * combines the two interleaved.
       */
      flags |= PVR_TRANSFER_CMD_FLAGS_DSMERGE;

      if (region->srcSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
         /* Takes the depth of the source and the stencil of the destination and
          * combines the two interleaved.
          */
         flags |= PVR_TRANSFER_CMD_FLAGS_PICKD;
      }
   }

   src_extent = region->extent;
   dst_extent = region->extent;

   if (src_block_compressed && !dst_block_compressed) {
      uint32_t block_width = util_format_get_blockwidth(src_pformat);
      uint32_t block_height = util_format_get_blockheight(src_pformat);

      dst_extent.width = MAX2(1U, DIV_ROUND_UP(src_extent.width, block_width));
      dst_extent.height =
         MAX2(1U, DIV_ROUND_UP(src_extent.height, block_height));
   } else if (!src_block_compressed && dst_block_compressed) {
      uint32_t block_width = util_format_get_blockwidth(dst_pformat);
      uint32_t block_height = util_format_get_blockheight(dst_pformat);

      dst_extent.width = MAX2(1U, src_extent.width * block_width);
      dst_extent.height = MAX2(1U, src_extent.height * block_height);
   }

   /* We don't care what format dst is as it's guaranteed to be size compatible
    * with src.
    */
   dst_format = pvr_get_raw_copy_format(src->vk.format);
   src_format = dst_format;

   src_layers =
      vk_image_subresource_layer_count(&src->vk, &region->srcSubresource);
   dst_layers =
      vk_image_subresource_layer_count(&dst->vk, &region->dstSubresource);

   /* srcSubresource.layerCount must match layerCount of dstSubresource in
    * copies not involving 3D images. In copies involving 3D images, if there is
    * a 2D image it's layerCount.
    */
   max_slices = MAX3(src_layers, dst_layers, region->extent.depth);

   for (uint32_t i = 0U; i < max_slices; i++) {
      struct pvr_transfer_cmd *transfer_cmd;
      VkResult result;

      transfer_cmd = pvr_transfer_cmd_alloc(cmd_buffer);
      if (!transfer_cmd)
         return VK_ERROR_OUT_OF_HOST_MEMORY;

      transfer_cmd->flags |= flags;
      transfer_cmd->sources[0].resolve_op = resolve_op;

      pvr_setup_surface_for_image(
         cmd_buffer->device,
         &transfer_cmd->sources[0].surface,
         &transfer_cmd->sources[0].mappings[0U].src_rect,
         src,
         region->srcSubresource.baseArrayLayer,
         i,
         region->srcSubresource.mipLevel,
         &region->srcOffset,
         &src_extent,
         region->srcOffset.z + i,
         src_format,
         region->srcSubresource.aspectMask);

      pvr_setup_surface_for_image(cmd_buffer->device,
                                  &transfer_cmd->dst,
                                  &transfer_cmd->scissor,
                                  dst,
                                  region->dstSubresource.baseArrayLayer,
                                  i,
                                  region->dstSubresource.mipLevel,
                                  &region->dstOffset,
                                  &dst_extent,
                                  region->dstOffset.z + i,
                                  dst_format,
                                  region->dstSubresource.aspectMask);

      transfer_cmd->sources[0].mappings[0U].dst_rect = transfer_cmd->scissor;
      transfer_cmd->sources[0].mapping_count++;
      transfer_cmd->source_count = 1;

      result = pvr_cmd_buffer_add_transfer_cmd(cmd_buffer, transfer_cmd);
      if (result != VK_SUCCESS) {
         vk_free(&cmd_buffer->vk.pool->alloc, transfer_cmd);
         return result;
      }
   }

   return VK_SUCCESS;
}

VkResult
pvr_copy_or_resolve_color_image_region(struct pvr_cmd_buffer *cmd_buffer,
                                       const struct pvr_image *src,
                                       const struct pvr_image *dst,
                                       const VkImageCopy2 *region)
{
   enum pvr_resolve_op resolve_op = PVR_RESOLVE_BLEND;

   if (src->vk.samples > 1U && dst->vk.samples < 2U) {
      /* Integer resolve picks a single sample. */
      if (vk_format_is_int(src->vk.format))
         resolve_op = PVR_RESOLVE_SAMPLE0;
   }

   return pvr_copy_or_resolve_image_region(cmd_buffer,
                                           resolve_op,
                                           src,
                                           dst,
                                           region);
}

static bool pvr_can_merge_ds_regions(const VkImageCopy2 *pRegionA,
                                     const VkImageCopy2 *pRegionB)
{
   assert(pRegionA->srcSubresource.aspectMask != 0U);
   assert(pRegionB->srcSubresource.aspectMask != 0U);

   if (!((pRegionA->srcSubresource.aspectMask ^
          pRegionB->srcSubresource.aspectMask) &
         (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
      return false;
   }

   /* Assert if aspectMask mismatch between src and dst, given it's a depth and
    * stencil image so not multi-planar and from the Vulkan 1.0.223 spec:
    *
    *    If neither srcImage nor dstImage has a multi-planar image format then
    *    for each element of pRegions, srcSubresource.aspectMask and
    *    dstSubresource.aspectMask must match.
    */
   assert(pRegionA->srcSubresource.aspectMask ==
          pRegionA->dstSubresource.aspectMask);
   assert(pRegionB->srcSubresource.aspectMask ==
          pRegionB->dstSubresource.aspectMask);

   if (!(pRegionA->srcSubresource.mipLevel ==
            pRegionB->srcSubresource.mipLevel &&
         pRegionA->srcSubresource.baseArrayLayer ==
            pRegionB->srcSubresource.baseArrayLayer &&
         pRegionA->srcSubresource.layerCount ==
            pRegionB->srcSubresource.layerCount)) {
      return false;
   }

   if (!(pRegionA->dstSubresource.mipLevel ==
            pRegionB->dstSubresource.mipLevel &&
         pRegionA->dstSubresource.baseArrayLayer ==
            pRegionB->dstSubresource.baseArrayLayer &&
         pRegionA->dstSubresource.layerCount ==
            pRegionB->dstSubresource.layerCount)) {
      return false;
   }

   if (!(pRegionA->srcOffset.x == pRegionB->srcOffset.x &&
         pRegionA->srcOffset.y == pRegionB->srcOffset.y &&
         pRegionA->srcOffset.z == pRegionB->srcOffset.z)) {
      return false;
   }

   if (!(pRegionA->dstOffset.x == pRegionB->dstOffset.x &&
         pRegionA->dstOffset.y == pRegionB->dstOffset.y &&
         pRegionA->dstOffset.z == pRegionB->dstOffset.z)) {
      return false;
   }

   if (!(pRegionA->extent.width == pRegionB->extent.width &&
         pRegionA->extent.height == pRegionB->extent.height &&
         pRegionA->extent.depth == pRegionB->extent.depth)) {
      return false;
   }

   return true;
}

void pvr_CmdCopyImage2(VkCommandBuffer commandBuffer,
                          const VkCopyImageInfo2 *pCopyImageInfo)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_image, src, pCopyImageInfo->srcImage);
   PVR_FROM_HANDLE(pvr_image, dst, pCopyImageInfo->dstImage);

   const bool can_merge_ds = src->vk.format == VK_FORMAT_D24_UNORM_S8_UINT &&
                             dst->vk.format == VK_FORMAT_D24_UNORM_S8_UINT;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   for (uint32_t i = 0U; i < pCopyImageInfo->regionCount; i++) {
      VkResult result;

      /* If an application has split a copy between D24S8 images into two
       * separate copy regions (one for the depth aspect and one for the
       * stencil aspect) attempt to merge the two regions back into one blit.
       *
       * This can only be merged if both regions are identical apart from the
       * aspectMask, one of which has to be depth and the other has to be
       * stencil.
       *
       * Only attempt to merge consecutive regions, ignore the case of merging
       * non-consecutive regions.
       */
      if (can_merge_ds && i != (pCopyImageInfo->regionCount - 1)) {
         const bool ret =
            pvr_can_merge_ds_regions(&pCopyImageInfo->pRegions[i],
                                     &pCopyImageInfo->pRegions[i + 1]);
         if (ret) {
            VkImageCopy2 region = pCopyImageInfo->pRegions[i];

            region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT |
                                               VK_IMAGE_ASPECT_STENCIL_BIT;
            region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT |
                                               VK_IMAGE_ASPECT_STENCIL_BIT;

            result = pvr_copy_or_resolve_color_image_region(cmd_buffer,
                                                            src,
                                                            dst,
                                                            &region);
            if (result != VK_SUCCESS)
               return;

            /* Skip the next region as it has been processed with the last
             * region.
             */
            i++;

            continue;
         }
      }

      result =
         pvr_copy_or_resolve_color_image_region(cmd_buffer,
                                                src,
                                                dst,
                                                &pCopyImageInfo->pRegions[i]);
      if (result != VK_SUCCESS)
         return;
   }
}

VkResult
pvr_copy_buffer_to_image_region_format(struct pvr_cmd_buffer *const cmd_buffer,
                                       const pvr_dev_addr_t buffer_dev_addr,
                                       const struct pvr_image *const image,
                                       const VkBufferImageCopy2 *const region,
                                       const VkFormat src_format,
                                       const VkFormat dst_format,
                                       const uint32_t flags)
{
   enum pipe_format pformat = vk_format_to_pipe_format(dst_format);
   uint32_t row_length_in_texels;
   uint32_t buffer_slice_size;
   uint32_t buffer_layer_size;
   uint32_t height_in_blks;
   uint32_t row_length;

   if (region->bufferRowLength == 0)
      row_length_in_texels = region->imageExtent.width;
   else
      row_length_in_texels = region->bufferRowLength;

   if (region->bufferImageHeight == 0)
      height_in_blks = region->imageExtent.height;
   else
      height_in_blks = region->bufferImageHeight;

   if (util_format_is_compressed(pformat)) {
      uint32_t block_width = util_format_get_blockwidth(pformat);
      uint32_t block_height = util_format_get_blockheight(pformat);
      uint32_t block_size = util_format_get_blocksize(pformat);

      height_in_blks = DIV_ROUND_UP(height_in_blks, block_height);
      row_length_in_texels =
         DIV_ROUND_UP(row_length_in_texels, block_width) * block_size;
   }

   row_length = row_length_in_texels * vk_format_get_blocksize(src_format);

   buffer_slice_size = height_in_blks * row_length;
   buffer_layer_size = buffer_slice_size * region->imageExtent.depth;

   for (uint32_t i = 0; i < region->imageExtent.depth; i++) {
      const uint32_t depth = i + (uint32_t)region->imageOffset.z;

      for (uint32_t j = 0; j < region->imageSubresource.layerCount; j++) {
         const VkDeviceSize buffer_offset = region->bufferOffset +
                                            (j * buffer_layer_size) +
                                            (i * buffer_slice_size);
         struct pvr_transfer_cmd *transfer_cmd;
         VkResult result;

         transfer_cmd = pvr_transfer_cmd_alloc(cmd_buffer);
         if (!transfer_cmd)
            return VK_ERROR_OUT_OF_HOST_MEMORY;

         transfer_cmd->flags = flags;

         pvr_setup_buffer_surface(
            &transfer_cmd->sources[0].surface,
            &transfer_cmd->sources[0].mappings[0].src_rect,
            buffer_dev_addr,
            buffer_offset,
            src_format,
            image->vk.format,
            region->imageExtent.width,
            region->imageExtent.height,
            row_length_in_texels);

         transfer_cmd->sources[0].surface.depth = 1;
         transfer_cmd->source_count = 1;

         pvr_setup_transfer_surface(cmd_buffer->device,
                                    &transfer_cmd->dst,
                                    &transfer_cmd->scissor,
                                    image,
                                    region->imageSubresource.baseArrayLayer + j,
                                    region->imageSubresource.mipLevel,
                                    &region->imageOffset,
                                    &region->imageExtent,
                                    depth,
                                    dst_format,
                                    region->imageSubresource.aspectMask);

         transfer_cmd->sources[0].mappings[0].dst_rect = transfer_cmd->scissor;
         transfer_cmd->sources[0].mapping_count++;

         result = pvr_cmd_buffer_add_transfer_cmd(cmd_buffer, transfer_cmd);
         if (result != VK_SUCCESS) {
            vk_free(&cmd_buffer->vk.pool->alloc, transfer_cmd);
            return result;
         }
      }
   }

   return VK_SUCCESS;
}

VkResult
pvr_copy_buffer_to_image_region(struct pvr_cmd_buffer *const cmd_buffer,
                                const pvr_dev_addr_t buffer_dev_addr,
                                const struct pvr_image *const image,
                                const VkBufferImageCopy2 *const region)
{
   const VkImageAspectFlags aspect_mask = region->imageSubresource.aspectMask;
   VkFormat src_format;
   VkFormat dst_format;
   uint32_t flags = 0;

   if (vk_format_has_depth(image->vk.format) &&
       vk_format_has_stencil(image->vk.format)) {
      flags |= PVR_TRANSFER_CMD_FLAGS_DSMERGE;

      if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
         src_format = vk_format_stencil_only(image->vk.format);
      } else {
         src_format = vk_format_depth_only(image->vk.format);
         flags |= PVR_TRANSFER_CMD_FLAGS_PICKD;
      }

      dst_format = image->vk.format;
   } else {
      src_format = pvr_get_raw_copy_format(image->vk.format);
      dst_format = src_format;
   }

   return pvr_copy_buffer_to_image_region_format(cmd_buffer,
                                                 buffer_dev_addr,
                                                 image,
                                                 region,
                                                 src_format,
                                                 dst_format,
                                                 flags);
}

void pvr_CmdCopyBufferToImage2(
   VkCommandBuffer commandBuffer,
   const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo)
{
   PVR_FROM_HANDLE(pvr_buffer, src, pCopyBufferToImageInfo->srcBuffer);
   PVR_FROM_HANDLE(pvr_image, dst, pCopyBufferToImageInfo->dstImage);
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   for (uint32_t i = 0; i < pCopyBufferToImageInfo->regionCount; i++) {
      const VkResult result =
         pvr_copy_buffer_to_image_region(cmd_buffer,
                                         src->dev_addr,
                                         dst,
                                         &pCopyBufferToImageInfo->pRegions[i]);
      if (result != VK_SUCCESS)
         return;
   }
}

VkResult
pvr_copy_image_to_buffer_region_format(struct pvr_cmd_buffer *const cmd_buffer,
                                       const struct pvr_image *const image,
                                       const pvr_dev_addr_t buffer_dev_addr,
                                       const VkBufferImageCopy2 *const region,
                                       const VkFormat src_format,
                                       const VkFormat dst_format)
{
   enum pipe_format pformat = vk_format_to_pipe_format(image->vk.format);
   struct pvr_transfer_cmd_surface dst_surface = { 0 };
   VkImageSubresource sub_resource;
   uint32_t buffer_image_height;
   uint32_t buffer_row_length;
   uint32_t buffer_slice_size;
   uint32_t max_array_layers;
   VkRect2D dst_rect = { 0 };
   uint32_t max_depth_slice;
   VkSubresourceLayout info;

   /* Only images with VK_SAMPLE_COUNT_1_BIT can be copied to buffer. */
   assert(image->vk.samples == 1);

   if (region->bufferRowLength == 0)
      buffer_row_length = region->imageExtent.width;
   else
      buffer_row_length = region->bufferRowLength;

   if (region->bufferImageHeight == 0)
      buffer_image_height = region->imageExtent.height;
   else
      buffer_image_height = region->bufferImageHeight;

   max_array_layers =
      region->imageSubresource.baseArrayLayer +
      vk_image_subresource_layer_count(&image->vk, &region->imageSubresource);

   buffer_slice_size = buffer_image_height * buffer_row_length *
                       vk_format_get_blocksize(dst_format);

   max_depth_slice = region->imageExtent.depth + region->imageOffset.z;

   pvr_setup_buffer_surface(&dst_surface,
                            &dst_rect,
                            buffer_dev_addr,
                            region->bufferOffset,
                            dst_format,
                            image->vk.format,
                            buffer_row_length,
                            buffer_image_height,
                            buffer_row_length);

   dst_rect.extent.width = region->imageExtent.width;
   dst_rect.extent.height = region->imageExtent.height;

   if (util_format_is_compressed(pformat)) {
      uint32_t block_width = util_format_get_blockwidth(pformat);
      uint32_t block_height = util_format_get_blockheight(pformat);

      dst_rect.extent.width =
         MAX2(1U, DIV_ROUND_UP(dst_rect.extent.width, block_width));
      dst_rect.extent.height =
         MAX2(1U, DIV_ROUND_UP(dst_rect.extent.height, block_height));
   }

   sub_resource = (VkImageSubresource){
      .aspectMask = region->imageSubresource.aspectMask,
      .mipLevel = region->imageSubresource.mipLevel,
      .arrayLayer = region->imageSubresource.baseArrayLayer,
   };

   pvr_get_image_subresource_layout(image, &sub_resource, &info);

   for (uint32_t i = region->imageSubresource.baseArrayLayer;
        i < max_array_layers;
        i++) {
      struct pvr_transfer_cmd_surface src_surface = { 0 };
      VkRect2D src_rect = { 0 };

      /* Note: Set the depth to the initial depth offset, the memory address (or
       * the z_position) for the depth slice will be incremented manually in the
       * loop below.
       */
      pvr_setup_transfer_surface(cmd_buffer->device,
                                 &src_surface,
                                 &src_rect,
                                 image,
                                 i,
                                 region->imageSubresource.mipLevel,
                                 &region->imageOffset,
                                 &region->imageExtent,
                                 region->imageOffset.z,
                                 src_format,
                                 region->imageSubresource.aspectMask);

      for (uint32_t j = region->imageOffset.z; j < max_depth_slice; j++) {
         struct pvr_transfer_cmd *transfer_cmd;
         VkResult result;

         /* TODO: See if we can allocate all the transfer cmds in one go. */
         transfer_cmd = pvr_transfer_cmd_alloc(cmd_buffer);
         if (!transfer_cmd)
            return vk_error(cmd_buffer->device, VK_ERROR_OUT_OF_HOST_MEMORY);

         transfer_cmd->sources[0].mappings[0].src_rect = src_rect;
         transfer_cmd->sources[0].mappings[0].dst_rect = dst_rect;
         transfer_cmd->sources[0].mapping_count++;

         transfer_cmd->sources[0].surface = src_surface;
         transfer_cmd->source_count = 1;

         transfer_cmd->dst = dst_surface;
         transfer_cmd->scissor = dst_rect;

         result = pvr_cmd_buffer_add_transfer_cmd(cmd_buffer, transfer_cmd);
         if (result != VK_SUCCESS) {
            vk_free(&cmd_buffer->vk.pool->alloc, transfer_cmd);
            return result;
         }

         dst_surface.dev_addr.addr += buffer_slice_size;

         if (src_surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED)
            src_surface.z_position += 1.0f;
         else
            src_surface.dev_addr.addr += info.depthPitch;
      }
   }

   return VK_SUCCESS;
}

VkResult
pvr_copy_image_to_buffer_region(struct pvr_cmd_buffer *const cmd_buffer,
                                const struct pvr_image *const image,
                                const pvr_dev_addr_t buffer_dev_addr,
                                const VkBufferImageCopy2 *const region)
{
   const VkImageAspectFlags aspect_mask = region->imageSubresource.aspectMask;

   VkFormat src_format = pvr_get_copy_format(image->vk.format);
   VkFormat dst_format;

   /* Color and depth aspect copies can be done using an appropriate raw format.
    */
   if (aspect_mask & (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT)) {
      src_format = pvr_get_raw_copy_format(src_format);
      dst_format = src_format;
   } else if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
      /* From the Vulkan spec:
       *
       *    Data copied to or from the stencil aspect of any depth/stencil
       *    format is tightly packed with one VK_FORMAT_S8_UINT value per texel.
       */
      dst_format = VK_FORMAT_S8_UINT;
   } else {
      /* YUV Planes require specific formats. */
      dst_format = src_format;
   }

   return pvr_copy_image_to_buffer_region_format(cmd_buffer,
                                                 image,
                                                 buffer_dev_addr,
                                                 region,
                                                 src_format,
                                                 dst_format);
}

void pvr_CmdCopyImageToBuffer2(
   VkCommandBuffer commandBuffer,
   const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo)
{
   PVR_FROM_HANDLE(pvr_buffer, dst, pCopyImageToBufferInfo->dstBuffer);
   PVR_FROM_HANDLE(pvr_image, src, pCopyImageToBufferInfo->srcImage);
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   for (uint32_t i = 0U; i < pCopyImageToBufferInfo->regionCount; i++) {
      const VkBufferImageCopy2 *region = &pCopyImageToBufferInfo->pRegions[i];

      const VkResult result = pvr_copy_image_to_buffer_region(cmd_buffer,
                                                              src,
                                                              dst->dev_addr,
                                                              region);
      if (result != VK_SUCCESS)
         return;
   }
}

static void pvr_calc_mip_level_extents(const struct pvr_image *image,
                                       uint16_t mip_level,
                                       VkExtent3D *extent_out)
{
   /* 3D textures are clamped to 4x4x4. */
   const uint32_t clamp = (image->vk.image_type == VK_IMAGE_TYPE_3D) ? 4 : 1;
   const VkExtent3D *extent = &image->vk.extent;

   extent_out->width = MAX2(extent->width >> mip_level, clamp);
   extent_out->height = MAX2(extent->height >> mip_level, clamp);
   extent_out->depth = MAX2(extent->depth >> mip_level, clamp);
}

static VkResult pvr_clear_image_range(struct pvr_cmd_buffer *cmd_buffer,
                                      const struct pvr_image *image,
                                      const VkClearColorValue *pColor,
                                      const VkImageSubresourceRange *psRange,
                                      uint32_t flags)
{
   const uint32_t layer_count =
      vk_image_subresource_layer_count(&image->vk, psRange);
   const uint32_t max_layers = psRange->baseArrayLayer + layer_count;
   VkFormat format = image->vk.format;
   const VkOffset3D offset = { 0 };
   VkExtent3D mip_extent;

   assert((psRange->baseArrayLayer + layer_count) <= image->vk.array_layers);

   for (uint32_t layer = psRange->baseArrayLayer; layer < max_layers; layer++) {
      const uint32_t level_count =
         vk_image_subresource_level_count(&image->vk, psRange);
      const uint32_t max_level = psRange->baseMipLevel + level_count;

      assert((psRange->baseMipLevel + level_count) <= image->vk.mip_levels);

      for (uint32_t level = psRange->baseMipLevel; level < max_level; level++) {
         pvr_calc_mip_level_extents(image, level, &mip_extent);

         for (uint32_t depth = 0; depth < mip_extent.depth; depth++) {
            struct pvr_transfer_cmd *transfer_cmd;
            VkResult result;

            transfer_cmd = pvr_transfer_cmd_alloc(cmd_buffer);
            if (!transfer_cmd)
               return VK_ERROR_OUT_OF_HOST_MEMORY;

            transfer_cmd->flags |= flags;
            transfer_cmd->flags |= PVR_TRANSFER_CMD_FLAGS_FILL;

            for (uint32_t i = 0; i < ARRAY_SIZE(transfer_cmd->clear_color); i++)
               transfer_cmd->clear_color[i].ui = pColor->uint32[i];

            pvr_setup_transfer_surface(cmd_buffer->device,
                                       &transfer_cmd->dst,
                                       &transfer_cmd->scissor,
                                       image,
                                       layer,
                                       level,
                                       &offset,
                                       &mip_extent,
                                       depth,
                                       format,
                                       psRange->aspectMask);

            result = pvr_cmd_buffer_add_transfer_cmd(cmd_buffer, transfer_cmd);
            if (result != VK_SUCCESS) {
               vk_free(&cmd_buffer->vk.pool->alloc, transfer_cmd);
               return result;
            }
         }
      }
   }

   return VK_SUCCESS;
}

void pvr_CmdClearColorImage(VkCommandBuffer commandBuffer,
                            VkImage _image,
                            VkImageLayout imageLayout,
                            const VkClearColorValue *pColor,
                            uint32_t rangeCount,
                            const VkImageSubresourceRange *pRanges)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_image, image, _image);

   for (uint32_t i = 0; i < rangeCount; i++) {
      const VkResult result =
         pvr_clear_image_range(cmd_buffer, image, pColor, &pRanges[i], 0);
      if (result != VK_SUCCESS)
         return;
   }
}

void pvr_CmdClearDepthStencilImage(VkCommandBuffer commandBuffer,
                                   VkImage _image,
                                   VkImageLayout imageLayout,
                                   const VkClearDepthStencilValue *pDepthStencil,
                                   uint32_t rangeCount,
                                   const VkImageSubresourceRange *pRanges)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_image, image, _image);

   for (uint32_t i = 0; i < rangeCount; i++) {
      const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT |
                                           VK_IMAGE_ASPECT_STENCIL_BIT;
      VkClearColorValue clear_ds = { 0 };
      uint32_t flags = 0U;
      VkResult result;

      if (image->vk.format == VK_FORMAT_D24_UNORM_S8_UINT &&
          pRanges[i].aspectMask != ds_aspect) {
         /* A depth or stencil blit to a packed_depth_stencil requires a merge
          * operation.
          */
         flags |= PVR_TRANSFER_CMD_FLAGS_DSMERGE;

         if (pRanges[i].aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
            flags |= PVR_TRANSFER_CMD_FLAGS_PICKD;
      }

      clear_ds.float32[0] = pDepthStencil->depth;
      clear_ds.uint32[1] = pDepthStencil->stencil;

      result =
         pvr_clear_image_range(cmd_buffer, image, &clear_ds, pRanges + i, flags);
      if (result != VK_SUCCESS)
         return;
   }
}

static VkResult pvr_cmd_copy_buffer_region(struct pvr_cmd_buffer *cmd_buffer,
                                           pvr_dev_addr_t src_addr,
                                           VkDeviceSize src_offset,
                                           pvr_dev_addr_t dst_addr,
                                           VkDeviceSize dst_offset,
                                           VkDeviceSize size,
                                           uint32_t fill_data,
                                           bool is_fill)
{
   VkDeviceSize offset = 0;

   while (offset < size) {
      const VkDeviceSize remaining_size = size - offset;
      struct pvr_transfer_cmd *transfer_cmd;
      uint32_t texel_width;
      VkDeviceSize texels;
      VkFormat vk_format;
      VkResult result;
      uint32_t height;
      uint32_t width;

      if (is_fill) {
         vk_format = VK_FORMAT_R32_UINT;
         texel_width = 4U;
      } else if (remaining_size >= 16U) {
         vk_format = VK_FORMAT_R32G32B32A32_UINT;
         texel_width = 16U;
      } else if (remaining_size >= 4U) {
         vk_format = VK_FORMAT_R32_UINT;
         texel_width = 4U;
      } else {
         vk_format = VK_FORMAT_R8_UINT;
         texel_width = 1U;
      }

      texels = remaining_size / texel_width;

      /* Try to do max-width rects, fall back to a 1-height rect for the
       * remainder.
       */
      if (texels > PVR_MAX_TRANSFER_SIZE_IN_TEXELS) {
         width = PVR_MAX_TRANSFER_SIZE_IN_TEXELS;
         height = texels / PVR_MAX_TRANSFER_SIZE_IN_TEXELS;
         height = MIN2(height, PVR_MAX_TRANSFER_SIZE_IN_TEXELS);
      } else {
         width = texels;
         height = 1;
      }

      transfer_cmd = pvr_transfer_cmd_alloc(cmd_buffer);
      if (!transfer_cmd)
         return VK_ERROR_OUT_OF_HOST_MEMORY;

      if (!is_fill) {
         pvr_setup_buffer_surface(
            &transfer_cmd->sources[0].surface,
            &transfer_cmd->sources[0].mappings[0].src_rect,
            src_addr,
            offset + src_offset,
            vk_format,
            vk_format,
            width,
            height,
            width);
         transfer_cmd->source_count = 1;
      } else {
         transfer_cmd->flags |= PVR_TRANSFER_CMD_FLAGS_FILL;

         for (uint32_t i = 0; i < ARRAY_SIZE(transfer_cmd->clear_color); i++)
            transfer_cmd->clear_color[i].ui = fill_data;
      }

      pvr_setup_buffer_surface(&transfer_cmd->dst,
                               &transfer_cmd->scissor,
                               dst_addr,
                               offset + dst_offset,
                               vk_format,
                               vk_format,
                               width,
                               height,
                               width);

      if (transfer_cmd->source_count > 0) {
         transfer_cmd->sources[0].mappings[0].dst_rect = transfer_cmd->scissor;

         transfer_cmd->sources[0].mapping_count++;
      }

      result = pvr_cmd_buffer_add_transfer_cmd(cmd_buffer, transfer_cmd);
      if (result != VK_SUCCESS) {
         vk_free(&cmd_buffer->vk.pool->alloc, transfer_cmd);
         return result;
      }

      offset += width * height * texel_width;
   }

   return VK_SUCCESS;
}

void pvr_CmdUpdateBuffer(VkCommandBuffer commandBuffer,
                         VkBuffer dstBuffer,
                         VkDeviceSize dstOffset,
                         VkDeviceSize dataSize,
                         const void *pData)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_buffer, dst, dstBuffer);
   struct pvr_suballoc_bo *pvr_bo;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   result = pvr_cmd_buffer_upload_general(cmd_buffer, pData, dataSize, &pvr_bo);
   if (result != VK_SUCCESS)
      return;

   pvr_cmd_copy_buffer_region(cmd_buffer,
                              pvr_bo->dev_addr,
                              0,
                              dst->dev_addr,
                              dstOffset,
                              dataSize,
                              0U,
                              false);
}

void pvr_CmdCopyBuffer2(VkCommandBuffer commandBuffer,
                           const VkCopyBufferInfo2 *pCopyBufferInfo)
{
   PVR_FROM_HANDLE(pvr_buffer, src, pCopyBufferInfo->srcBuffer);
   PVR_FROM_HANDLE(pvr_buffer, dst, pCopyBufferInfo->dstBuffer);
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; i++) {
      const VkResult result =
         pvr_cmd_copy_buffer_region(cmd_buffer,
                                    src->dev_addr,
                                    pCopyBufferInfo->pRegions[i].srcOffset,
                                    dst->dev_addr,
                                    pCopyBufferInfo->pRegions[i].dstOffset,
                                    pCopyBufferInfo->pRegions[i].size,
                                    0U,
                                    false);
      if (result != VK_SUCCESS)
         return;
   }
}

void pvr_CmdFillBuffer(VkCommandBuffer commandBuffer,
                       VkBuffer dstBuffer,
                       VkDeviceSize dstOffset,
                       VkDeviceSize fillSize,
                       uint32_t data)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_buffer, dst, dstBuffer);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   fillSize = vk_buffer_range(&dst->vk, dstOffset, fillSize);

   /* From the Vulkan spec:
    *
    *    "size is the number of bytes to fill, and must be either a multiple
    *    of 4, or VK_WHOLE_SIZE to fill the range from offset to the end of
    *    the buffer. If VK_WHOLE_SIZE is used and the remaining size of the
    *    buffer is not a multiple of 4, then the nearest smaller multiple is
    *    used."
    */
   fillSize &= ~3ULL;

   pvr_cmd_copy_buffer_region(cmd_buffer,
                              PVR_DEV_ADDR_INVALID,
                              0,
                              dst->dev_addr,
                              dstOffset,
                              fillSize,
                              data,
                              true);
}

/**
 * \brief Returns the maximum number of layers to clear starting from base_layer
 * that contain or match the target rectangle.
 *
 * \param[in] target_rect      The region which the clear should contain or
 *                             match.
 * \param[in] base_layer       The layer index to start at.
 * \param[in] clear_rect_count Amount of clear_rects
 * \param[in] clear_rects      Array of clear rects.
 *
 * \return Max number of layers that cover or match the target region.
 */
static uint32_t
pvr_get_max_layers_covering_target(VkRect2D target_rect,
                                   uint32_t base_layer,
                                   uint32_t clear_rect_count,
                                   const VkClearRect *clear_rects)
{
   const int32_t target_x0 = target_rect.offset.x;
   const int32_t target_x1 = target_x0 + (int32_t)target_rect.extent.width;
   const int32_t target_y0 = target_rect.offset.y;
   const int32_t target_y1 = target_y0 + (int32_t)target_rect.extent.height;

   uint32_t layer_count = 0;

   assert((int64_t)target_x0 + (int64_t)target_rect.extent.width <= INT32_MAX);
   assert((int64_t)target_y0 + (int64_t)target_rect.extent.height <= INT32_MAX);

   for (uint32_t i = 0; i < clear_rect_count; i++) {
      const VkClearRect *clear_rect = &clear_rects[i];
      const uint32_t max_layer =
         clear_rect->baseArrayLayer + clear_rect->layerCount;
      bool target_is_covered;
      int32_t x0, x1;
      int32_t y0, y1;

      if (clear_rect->baseArrayLayer == 0)
         continue;

      assert((uint64_t)clear_rect->baseArrayLayer + clear_rect->layerCount <=
             UINT32_MAX);

      /* Check for layer intersection. */
      if (clear_rect->baseArrayLayer > base_layer || max_layer <= base_layer)
         continue;

      x0 = clear_rect->rect.offset.x;
      x1 = x0 + (int32_t)clear_rect->rect.extent.width;
      y0 = clear_rect->rect.offset.y;
      y1 = y0 + (int32_t)clear_rect->rect.extent.height;

      assert((int64_t)x0 + (int64_t)clear_rect->rect.extent.width <= INT32_MAX);
      assert((int64_t)y0 + (int64_t)clear_rect->rect.extent.height <=
             INT32_MAX);

      target_is_covered = x0 <= target_x0 && x1 >= target_x1;
      target_is_covered &= y0 <= target_y0 && y1 >= target_y1;

      if (target_is_covered)
         layer_count = MAX2(layer_count, max_layer - base_layer);
   }

   return layer_count;
}

/* Return true if vertex shader is required to output render target id to pick
 * the texture array layer.
 */
static inline bool
pvr_clear_needs_rt_id_output(struct pvr_device_info *dev_info,
                             uint32_t rect_count,
                             const VkClearRect *rects)
{
   if (!PVR_HAS_FEATURE(dev_info, gs_rta_support))
      return false;

   for (uint32_t i = 0; i < rect_count; i++) {
      if (rects[i].baseArrayLayer != 0 || rects[i].layerCount > 1)
         return true;
   }

   return false;
}

static VkResult pvr_clear_color_attachment_static_create_consts_buffer(
   struct pvr_cmd_buffer *cmd_buffer,
   const struct pvr_shader_factory_info *shader_info,
   const uint32_t clear_color[static const PVR_CLEAR_COLOR_ARRAY_SIZE],
   ASSERTED bool uses_tile_buffer,
   uint32_t tile_buffer_idx,
   struct pvr_suballoc_bo **const const_shareds_buffer_out)
{
   struct pvr_device *device = cmd_buffer->device;
   struct pvr_suballoc_bo *const_shareds_buffer;
   struct pvr_bo *tile_buffer;
   uint64_t tile_dev_addr;
   uint32_t *buffer;
   VkResult result;

   /* TODO: This doesn't need to be aligned to slc size. Alignment to 4 is fine.
    * Change pvr_cmd_buffer_alloc_mem() to take in an alignment?
    */
   result =
      pvr_cmd_buffer_alloc_mem(cmd_buffer,
                               device->heaps.general_heap,
                               PVR_DW_TO_BYTES(shader_info->const_shared_regs),
                               &const_shareds_buffer);
   if (result != VK_SUCCESS)
      return result;

   buffer = pvr_bo_suballoc_get_map_addr(const_shareds_buffer);

   for (uint32_t i = 0; i < PVR_CLEAR_ATTACHMENT_CONST_COUNT; i++) {
      uint32_t dest_idx = shader_info->driver_const_location_map[i];

      if (dest_idx == PVR_CLEAR_ATTACHMENT_DEST_ID_UNUSED)
         continue;

      assert(dest_idx < shader_info->const_shared_regs);

      switch (i) {
      case PVR_CLEAR_ATTACHMENT_CONST_COMPONENT_0:
      case PVR_CLEAR_ATTACHMENT_CONST_COMPONENT_1:
      case PVR_CLEAR_ATTACHMENT_CONST_COMPONENT_2:
      case PVR_CLEAR_ATTACHMENT_CONST_COMPONENT_3:
         buffer[dest_idx] = clear_color[i];
         break;

      case PVR_CLEAR_ATTACHMENT_CONST_TILE_BUFFER_UPPER:
         assert(uses_tile_buffer);
         tile_buffer = device->tile_buffer_state.buffers[tile_buffer_idx];
         tile_dev_addr = tile_buffer->vma->dev_addr.addr;
         buffer[dest_idx] = (uint32_t)(tile_dev_addr >> 32);
         break;

      case PVR_CLEAR_ATTACHMENT_CONST_TILE_BUFFER_LOWER:
         assert(uses_tile_buffer);
         tile_buffer = device->tile_buffer_state.buffers[tile_buffer_idx];
         tile_dev_addr = tile_buffer->vma->dev_addr.addr;
         buffer[dest_idx] = (uint32_t)tile_dev_addr;
         break;

      default:
         unreachable("Unsupported clear attachment const type.");
      }
   }

   for (uint32_t i = 0; i < shader_info->num_static_const; i++) {
      const struct pvr_static_buffer *static_buff =
         &shader_info->static_const_buffer[i];

      assert(static_buff->dst_idx < shader_info->const_shared_regs);

      buffer[static_buff->dst_idx] = static_buff->value;
   }

   *const_shareds_buffer_out = const_shareds_buffer;

   return VK_SUCCESS;
}

static VkResult pvr_clear_color_attachment_static(
   struct pvr_cmd_buffer *cmd_buffer,
   const struct usc_mrt_resource *mrt_resource,
   VkFormat format,
   uint32_t clear_color[static const PVR_CLEAR_COLOR_ARRAY_SIZE],
   uint32_t template_idx,
   uint32_t stencil,
   bool vs_has_rt_id_output)
{
   struct pvr_device *device = cmd_buffer->device;
   ASSERTED const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   ASSERTED const bool has_eight_output_registers =
      PVR_HAS_FEATURE(dev_info, eight_output_registers);
   const struct pvr_device_static_clear_state *dev_clear_state =
      &device->static_clear_state;
   const bool uses_tile_buffer = mrt_resource->type ==
                                 USC_MRT_RESOURCE_TYPE_MEMORY;
   const struct pvr_pds_clear_attachment_program_info *clear_attachment_program;
   struct pvr_pds_pixel_shader_sa_program texture_program;
   uint32_t pds_state[PVR_STATIC_CLEAR_PDS_STATE_COUNT];
   const struct pvr_shader_factory_info *shader_info;
   struct pvr_suballoc_bo *pds_texture_program_bo;
   struct pvr_static_clear_ppp_template template;
   struct pvr_suballoc_bo *const_shareds_buffer;
   uint64_t pds_texture_program_addr;
   struct pvr_suballoc_bo *pvr_bo;
   uint32_t tile_buffer_idx = 0;
   uint32_t out_reg_count;
   uint32_t output_offset;
   uint32_t program_idx;
   uint32_t *buffer;
   VkResult result;

   out_reg_count =
      DIV_ROUND_UP(pvr_get_pbe_accum_format_size_in_bytes(format), 4U);

   if (uses_tile_buffer) {
      tile_buffer_idx = mrt_resource->mem.tile_buffer;
      output_offset = mrt_resource->mem.offset_dw;
   } else {
      output_offset = mrt_resource->reg.output_reg;
   }

   assert(has_eight_output_registers || out_reg_count + output_offset <= 4);

   program_idx = pvr_get_clear_attachment_program_index(out_reg_count,
                                                        output_offset,
                                                        uses_tile_buffer);

   shader_info = clear_attachment_collection[program_idx].info;

   result = pvr_clear_color_attachment_static_create_consts_buffer(
      cmd_buffer,
      shader_info,
      clear_color,
      uses_tile_buffer,
      tile_buffer_idx,
      &const_shareds_buffer);
   if (result != VK_SUCCESS)
      return result;

   /* clang-format off */
   texture_program = (struct pvr_pds_pixel_shader_sa_program){
      .num_texture_dma_kicks = 1,
      .texture_dma_address = {
         [0] = const_shareds_buffer->dev_addr.addr,
      }
   };
   /* clang-format on */

   pvr_csb_pack (&texture_program.texture_dma_control[0],
                 PDSINST_DOUT_FIELDS_DOUTD_SRC1,
                 doutd_src1) {
      doutd_src1.dest = PVRX(PDSINST_DOUTD_DEST_COMMON_STORE);
      doutd_src1.bsize = shader_info->const_shared_regs;
   }

   clear_attachment_program =
      &dev_clear_state->pds_clear_attachment_program_info[program_idx];

   /* TODO: This doesn't need to be aligned to slc size. Alignment to 4 is fine.
    * Change pvr_cmd_buffer_alloc_mem() to take in an alignment?
    */
   result = pvr_cmd_buffer_alloc_mem(
      cmd_buffer,
      device->heaps.pds_heap,
      clear_attachment_program->texture_program_data_size,
      &pds_texture_program_bo);
   if (result != VK_SUCCESS) {
      list_del(&const_shareds_buffer->link);
      pvr_bo_suballoc_free(const_shareds_buffer);

      return result;
   }

   buffer = pvr_bo_suballoc_get_map_addr(pds_texture_program_bo);
   pds_texture_program_addr = pds_texture_program_bo->dev_addr.addr -
                              device->heaps.pds_heap->base_addr.addr;

   pvr_pds_generate_pixel_shader_sa_texture_state_data(
      &texture_program,
      buffer,
      &device->pdevice->dev_info);

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_SHADERBASE],
                 TA_STATE_PDS_SHADERBASE,
                 shaderbase) {
      shaderbase.addr = clear_attachment_program->pixel_program_offset;
   }

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_TEXUNICODEBASE],
                 TA_STATE_PDS_TEXUNICODEBASE,
                 texunicodebase) {
      texunicodebase.addr = clear_attachment_program->texture_program_offset;
   }

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_SIZEINFO1],
                 TA_STATE_PDS_SIZEINFO1,
                 sizeinfo1) {
      sizeinfo1.pds_texturestatesize = DIV_ROUND_UP(
         clear_attachment_program->texture_program_data_size,
         PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEXTURESTATESIZE_UNIT_SIZE));

      sizeinfo1.pds_tempsize =
         DIV_ROUND_UP(clear_attachment_program->texture_program_pds_temps_count,
                      PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEMPSIZE_UNIT_SIZE));
   }

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_SIZEINFO2],
                 TA_STATE_PDS_SIZEINFO2,
                 sizeinfo2) {
      sizeinfo2.usc_sharedsize =
         DIV_ROUND_UP(shader_info->const_shared_regs,
                      PVRX(TA_STATE_PDS_SIZEINFO2_USC_SHAREDSIZE_UNIT_SIZE));
   }

   /* Dummy coefficient loading program. */
   pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_VARYINGBASE] = 0;

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_TEXTUREDATABASE],
                 TA_STATE_PDS_TEXTUREDATABASE,
                 texturedatabase) {
      texturedatabase.addr = PVR_DEV_ADDR(pds_texture_program_addr);
   }

   assert(template_idx < PVR_STATIC_CLEAR_VARIANT_COUNT);
   template =
      cmd_buffer->device->static_clear_state.ppp_templates[template_idx];

   template.config.pds_state = &pds_state;

   template.config.ispctl.upass =
      cmd_buffer->state.render_pass_info.isp_userpass;

   if (template_idx & VK_IMAGE_ASPECT_STENCIL_BIT)
      template.config.ispa.sref = stencil;

   if (vs_has_rt_id_output) {
      template.config.output_sel.rhw_pres = true;
      template.config.output_sel.render_tgt_pres = true;
      template.config.output_sel.vtxsize = 4 + 1;
   }

   result = pvr_emit_ppp_from_template(
      &cmd_buffer->state.current_sub_cmd->gfx.control_stream,
      &template,
      &pvr_bo);
   if (result != VK_SUCCESS) {
      list_del(&pds_texture_program_bo->link);
      pvr_bo_suballoc_free(pds_texture_program_bo);

      list_del(&const_shareds_buffer->link);
      pvr_bo_suballoc_free(const_shareds_buffer);

      return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
   }

   list_add(&pvr_bo->link, &cmd_buffer->bo_list);

   return VK_SUCCESS;
}

/**
 * \brief Record a deferred clear operation into the command buffer.
 *
 * Devices which don't have gs_rta_support require extra handling for RTA
 * clears. We setup a list of deferred clear transfer commands which will be
 * processed at the end of the graphics sub command to account for the missing
 * feature.
 */
static VkResult pvr_add_deferred_rta_clear(struct pvr_cmd_buffer *cmd_buffer,
                                           const VkClearAttachment *attachment,
                                           const VkClearRect *rect,
                                           bool is_render_init)
{
   struct pvr_render_pass_info *pass_info = &cmd_buffer->state.render_pass_info;
   struct pvr_sub_cmd_gfx *sub_cmd = &cmd_buffer->state.current_sub_cmd->gfx;
   const struct pvr_renderpass_hwsetup_render *hw_render =
      &pass_info->pass->hw_setup->renders[sub_cmd->hw_render_idx];
   struct pvr_transfer_cmd *transfer_cmd_list;
   const struct pvr_image_view *image_view;
   const struct pvr_image *image;
   uint32_t base_layer;

   const VkOffset3D offset = {
      .x = rect->rect.offset.x,
      .y = rect->rect.offset.y,
      .z = 1,
   };
   const VkExtent3D extent = {
      .width = rect->rect.extent.width,
      .height = rect->rect.extent.height,
      .depth = 1,
   };

   assert(
      !PVR_HAS_FEATURE(&cmd_buffer->device->pdevice->dev_info, gs_rta_support));

   transfer_cmd_list = util_dynarray_grow(&cmd_buffer->deferred_clears,
                                          struct pvr_transfer_cmd,
                                          rect->layerCount);
   if (!transfer_cmd_list) {
      return vk_command_buffer_set_error(&cmd_buffer->vk,
                                         VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   /* From the Vulkan 1.3.229 spec VUID-VkClearAttachment-aspectMask-00019:
    *
    *    "If aspectMask includes VK_IMAGE_ASPECT_COLOR_BIT, it must not
    *    include VK_IMAGE_ASPECT_DEPTH_BIT or VK_IMAGE_ASPECT_STENCIL_BIT"
    *
    */
   if (attachment->aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) {
      assert(attachment->aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT ||
             attachment->aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT ||
             attachment->aspectMask ==
                (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));

      image_view = pass_info->attachments[hw_render->ds_attach_idx];
   } else if (is_render_init) {
      uint32_t index;

      assert(attachment->colorAttachment < hw_render->color_init_count);
      index = hw_render->color_init[attachment->colorAttachment].index;

      image_view = pass_info->attachments[index];
   } else {
      const struct pvr_renderpass_hwsetup_subpass *hw_pass =
         pvr_get_hw_subpass(pass_info->pass, pass_info->subpass_idx);
      const struct pvr_render_subpass *sub_pass =
         &pass_info->pass->subpasses[hw_pass->index];
      const uint32_t attachment_idx =
         sub_pass->color_attachments[attachment->colorAttachment];

      assert(attachment->colorAttachment < sub_pass->color_count);

      image_view = pass_info->attachments[attachment_idx];
   }

   base_layer = image_view->vk.base_array_layer + rect->baseArrayLayer;
   image = vk_to_pvr_image(image_view->vk.image);

   for (uint32_t i = 0; i < rect->layerCount; i++) {
      struct pvr_transfer_cmd *transfer_cmd = &transfer_cmd_list[i];

      /* TODO: Add an init function for when we don't want to use
       * pvr_transfer_cmd_alloc()? And use it here.
       */
      *transfer_cmd = (struct pvr_transfer_cmd){
         .flags = PVR_TRANSFER_CMD_FLAGS_FILL,
         .cmd_buffer = cmd_buffer,
         .is_deferred_clear = true,
      };

      if (attachment->aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) {
         for (uint32_t j = 0; j < ARRAY_SIZE(transfer_cmd->clear_color); j++) {
            transfer_cmd->clear_color[j].ui =
               attachment->clearValue.color.uint32[j];
         }
      } else {
         transfer_cmd->clear_color[0].f =
            attachment->clearValue.depthStencil.depth;
         transfer_cmd->clear_color[1].ui =
            attachment->clearValue.depthStencil.stencil;
      }

      pvr_setup_transfer_surface(cmd_buffer->device,
                                 &transfer_cmd->dst,
                                 &transfer_cmd->scissor,
                                 image,
                                 base_layer + i,
                                 0,
                                 &offset,
                                 &extent,
                                 0.0f,
                                 image->vk.format,
                                 attachment->aspectMask);
   }

   return VK_SUCCESS;
}

static void pvr_clear_attachments(struct pvr_cmd_buffer *cmd_buffer,
                                  uint32_t attachment_count,
                                  const VkClearAttachment *attachments,
                                  uint32_t rect_count,
                                  const VkClearRect *rects,
                                  bool is_render_init)
{
   const struct pvr_render_pass *pass = cmd_buffer->state.render_pass_info.pass;
   struct pvr_render_pass_info *pass_info = &cmd_buffer->state.render_pass_info;
   const struct pvr_renderpass_hwsetup_subpass *hw_pass =
      pvr_get_hw_subpass(pass, pass_info->subpass_idx);
   struct pvr_sub_cmd_gfx *sub_cmd = &cmd_buffer->state.current_sub_cmd->gfx;
   struct pvr_device_info *dev_info = &cmd_buffer->device->pdevice->dev_info;
   struct pvr_render_subpass *sub_pass = &pass->subpasses[hw_pass->index];
   uint32_t vs_output_size_in_bytes;
   bool vs_has_rt_id_output;

   /* TODO: This function can be optimized so that most of the device memory
    * gets allocated together in one go and then filled as needed. There might
    * also be opportunities to reuse pds code and data segments.
    */

   assert(cmd_buffer->state.current_sub_cmd->type == PVR_SUB_CMD_TYPE_GRAPHICS);

   pvr_reset_graphics_dirty_state(cmd_buffer, false);

   /* We'll be emitting to the control stream. */
   sub_cmd->empty_cmd = false;

   vs_has_rt_id_output =
      pvr_clear_needs_rt_id_output(dev_info, rect_count, rects);

   /* 4 because we're expecting the USC to output X, Y, Z, and W. */
   vs_output_size_in_bytes = PVR_DW_TO_BYTES(4);
   if (vs_has_rt_id_output)
      vs_output_size_in_bytes += PVR_DW_TO_BYTES(1);

   for (uint32_t i = 0; i < attachment_count; i++) {
      const VkClearAttachment *attachment = &attachments[i];
      struct pvr_pds_vertex_shader_program pds_program;
      struct pvr_pds_upload pds_program_upload = { 0 };
      uint64_t current_base_array_layer = ~0;
      VkResult result;
      float depth;

      if (attachment->aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) {
         uint32_t packed_clear_color[PVR_CLEAR_COLOR_ARRAY_SIZE];
         const struct usc_mrt_resource *mrt_resource;
         uint32_t global_attachment_idx;
         uint32_t local_attachment_idx;
         VkFormat format;

         local_attachment_idx = attachment->colorAttachment;

         if (is_render_init) {
            struct pvr_renderpass_hwsetup_render *hw_render;

            assert(pass->hw_setup->render_count > 0);
            hw_render = &pass->hw_setup->renders[0];

            mrt_resource =
               &hw_render->init_setup.mrt_resources[local_attachment_idx];

            assert(local_attachment_idx < hw_render->color_init_count);
            global_attachment_idx =
               hw_render->color_init[local_attachment_idx].index;
         } else {
            mrt_resource = &hw_pass->setup.mrt_resources[local_attachment_idx];

            assert(local_attachment_idx < sub_pass->color_count);
            global_attachment_idx =
               sub_pass->color_attachments[local_attachment_idx];
         }

         if (global_attachment_idx == VK_ATTACHMENT_UNUSED)
            continue;

         assert(global_attachment_idx < pass->attachment_count);
         format = pass->attachments[global_attachment_idx].vk_format;

         assert(format != VK_FORMAT_UNDEFINED);

         pvr_get_hw_clear_color(format,
                                attachment->clearValue.color,
                                packed_clear_color);

         result = pvr_clear_color_attachment_static(cmd_buffer,
                                                    mrt_resource,
                                                    format,
                                                    packed_clear_color,
                                                    VK_IMAGE_ASPECT_COLOR_BIT,
                                                    0,
                                                    vs_has_rt_id_output);
         if (result != VK_SUCCESS)
            return;
      } else if (hw_pass->z_replicate != -1 &&
                 attachment->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
         const VkClearColorValue clear_color = {
            .float32 = { [0] = attachment->clearValue.depthStencil.depth, },
         };
         const uint32_t template_idx = attachment->aspectMask |
                                       VK_IMAGE_ASPECT_COLOR_BIT;
         const uint32_t stencil = attachment->clearValue.depthStencil.stencil;
         uint32_t packed_clear_color[PVR_CLEAR_COLOR_ARRAY_SIZE];
         const struct usc_mrt_resource *mrt_resource;

         mrt_resource = &hw_pass->setup.mrt_resources[hw_pass->z_replicate];

         pvr_get_hw_clear_color(VK_FORMAT_R32_SFLOAT,
                                clear_color,
                                packed_clear_color);

         result = pvr_clear_color_attachment_static(cmd_buffer,
                                                    mrt_resource,
                                                    VK_FORMAT_R32_SFLOAT,
                                                    packed_clear_color,
                                                    template_idx,
                                                    stencil,
                                                    vs_has_rt_id_output);
         if (result != VK_SUCCESS)
            return;
      } else {
         const uint32_t template_idx = attachment->aspectMask;
         struct pvr_static_clear_ppp_template template;
         struct pvr_suballoc_bo *pvr_bo;

         assert(template_idx < PVR_STATIC_CLEAR_VARIANT_COUNT);
         template =
            cmd_buffer->device->static_clear_state.ppp_templates[template_idx];

         if (attachment->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            template.config.ispa.sref =
               attachment->clearValue.depthStencil.stencil;
         }

         if (vs_has_rt_id_output) {
            template.config.output_sel.rhw_pres = true;
            template.config.output_sel.render_tgt_pres = true;
            template.config.output_sel.vtxsize = 4 + 1;
         }

         result = pvr_emit_ppp_from_template(&sub_cmd->control_stream,
                                             &template,
                                             &pvr_bo);
         if (result != VK_SUCCESS) {
            pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
            return;
         }

         list_add(&pvr_bo->link, &cmd_buffer->bo_list);
      }

      if (attachment->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
         depth = attachment->clearValue.depthStencil.depth;
      else
         depth = 1.0f;

      if (vs_has_rt_id_output) {
         const struct pvr_device_static_clear_state *dev_clear_state =
            &cmd_buffer->device->static_clear_state;
         const struct pvr_suballoc_bo *multi_layer_vert_bo =
            dev_clear_state->usc_multi_layer_vertex_shader_bo;

         /* We can't use the device's passthrough pds program since it doesn't
          * have iterate_instance_id enabled. We'll be uploading code sections
          * per each clear rect.
          */

         /* TODO: See if we can allocate all the code section memory in one go.
          * We'd need to make sure that changing instance_id_modifier doesn't
          * change the code section size.
          * Also check if we can reuse the same code segment for each rect.
          * Seems like the instance_id_modifier is written into the data section
          * and used by the pds ADD instruction that way instead of it being
          * embedded into the code section.
          */

         pvr_pds_clear_rta_vertex_shader_program_init_base(&pds_program,
                                                           multi_layer_vert_bo);
      } else {
         /* We can reuse the device's code section but we'll need to upload data
          * sections so initialize the program.
          */
         pvr_pds_clear_vertex_shader_program_init_base(
            &pds_program,
            cmd_buffer->device->static_clear_state.usc_vertex_shader_bo);

         pds_program_upload.code_offset =
            cmd_buffer->device->static_clear_state.pds.code_offset;
         /* TODO: The code size doesn't get used by pvr_clear_vdm_state() maybe
          * let's change its interface to make that clear and not set this?
          */
         pds_program_upload.code_size =
            cmd_buffer->device->static_clear_state.pds.code_size;
      }

      for (uint32_t j = 0; j < rect_count; j++) {
         struct pvr_pds_upload pds_program_data_upload;
         const VkClearRect *clear_rect = &rects[j];
         struct pvr_suballoc_bo *vertices_bo;
         uint32_t vdm_cs_size_in_dw;
         uint32_t *vdm_cs_buffer;
         VkResult result;

         if (!PVR_HAS_FEATURE(dev_info, gs_rta_support) &&
             (clear_rect->baseArrayLayer != 0 || clear_rect->layerCount > 1)) {
            result = pvr_add_deferred_rta_clear(cmd_buffer,
                                                attachment,
                                                clear_rect,
                                                is_render_init);
            if (result != VK_SUCCESS)
               return;

            if (clear_rect->baseArrayLayer != 0)
               continue;
         }

         /* TODO: Allocate all the buffers in one go before the loop, and add
          * support to multi-alloc bo.
          */
         result = pvr_clear_vertices_upload(cmd_buffer->device,
                                            &clear_rect->rect,
                                            depth,
                                            &vertices_bo);
         if (result != VK_SUCCESS) {
            pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
            return;
         }

         list_add(&vertices_bo->link, &cmd_buffer->bo_list);

         if (vs_has_rt_id_output) {
            if (current_base_array_layer != clear_rect->baseArrayLayer) {
               const uint32_t base_array_layer = clear_rect->baseArrayLayer;
               struct pvr_pds_upload pds_program_code_upload;

               result =
                  pvr_pds_clear_rta_vertex_shader_program_create_and_upload_code(
                     &pds_program,
                     cmd_buffer,
                     base_array_layer,
                     &pds_program_code_upload);
               if (result != VK_SUCCESS) {
                  pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
                  return;
               }

               pds_program_upload.code_offset =
                  pds_program_code_upload.code_offset;
               /* TODO: The code size doesn't get used by pvr_clear_vdm_state()
                * maybe let's change its interface to make that clear and not
                * set this?
                */
               pds_program_upload.code_size = pds_program_code_upload.code_size;

               current_base_array_layer = base_array_layer;
            }

            result =
               pvr_pds_clear_rta_vertex_shader_program_create_and_upload_data(
                  &pds_program,
                  cmd_buffer,
                  vertices_bo,
                  &pds_program_data_upload);
            if (result != VK_SUCCESS)
               return;
         } else {
            result = pvr_pds_clear_vertex_shader_program_create_and_upload_data(
               &pds_program,
               cmd_buffer,
               vertices_bo,
               &pds_program_data_upload);
            if (result != VK_SUCCESS)
               return;
         }

         pds_program_upload.data_offset = pds_program_data_upload.data_offset;
         pds_program_upload.data_size = pds_program_data_upload.data_size;

         vdm_cs_size_in_dw =
            pvr_clear_vdm_state_get_size_in_dw(dev_info,
                                               clear_rect->layerCount);

         pvr_csb_set_relocation_mark(&sub_cmd->control_stream);

         vdm_cs_buffer =
            pvr_csb_alloc_dwords(&sub_cmd->control_stream, vdm_cs_size_in_dw);
         if (!vdm_cs_buffer) {
            pvr_cmd_buffer_set_error_unwarned(cmd_buffer,
                                              sub_cmd->control_stream.status);
            return;
         }

         pvr_pack_clear_vdm_state(dev_info,
                                  &pds_program_upload,
                                  pds_program.temps_used,
                                  4,
                                  vs_output_size_in_bytes,
                                  clear_rect->layerCount,
                                  vdm_cs_buffer);

         pvr_csb_clear_relocation_mark(&sub_cmd->control_stream);
      }
   }
}

void pvr_clear_attachments_render_init(struct pvr_cmd_buffer *cmd_buffer,
                                       const VkClearAttachment *attachment,
                                       const VkClearRect *rect)
{
   pvr_clear_attachments(cmd_buffer, 1, attachment, 1, rect, true);
}

void pvr_CmdClearAttachments(VkCommandBuffer commandBuffer,
                             uint32_t attachmentCount,
                             const VkClearAttachment *pAttachments,
                             uint32_t rectCount,
                             const VkClearRect *pRects)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_sub_cmd_gfx *sub_cmd = &state->current_sub_cmd->gfx;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);
   assert(state->current_sub_cmd->type == PVR_SUB_CMD_TYPE_GRAPHICS);

   /* TODO: There are some optimizations that can be made here:
    *  - For a full screen clear, update the clear values for the corresponding
    *    attachment index.
    *  - For a full screen color attachment clear, add its index to a load op
    *    override to add it to the background shader. This will elide any load
    *    op loads currently in the background shader as well as the usual
    *    frag kick for geometry clear.
    */

   /* If we have any depth/stencil clears, update the sub command depth/stencil
    * modification and usage flags.
    */
   if (state->depth_format != VK_FORMAT_UNDEFINED) {
      uint32_t full_screen_clear_count;
      bool has_stencil_clear = false;
      bool has_depth_clear = false;

      for (uint32_t i = 0; i < attachmentCount; i++) {
         const VkImageAspectFlags aspect_mask = pAttachments[i].aspectMask;

         if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT)
            has_stencil_clear = true;

         if (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT)
            has_depth_clear = true;

         if (has_stencil_clear && has_depth_clear)
            break;
      }

      sub_cmd->modifies_stencil |= has_stencil_clear;
      sub_cmd->modifies_depth |= has_depth_clear;

      /* We only care about clears that have a baseArrayLayer of 0 as any
       * attachment clears we move to the background shader must apply to all of
       * the attachment's sub resources.
       */
      full_screen_clear_count =
         pvr_get_max_layers_covering_target(state->render_pass_info.render_area,
                                            0,
                                            rectCount,
                                            pRects);

      if (full_screen_clear_count > 0) {
         if (has_stencil_clear &&
             sub_cmd->stencil_usage == PVR_DEPTH_STENCIL_USAGE_UNDEFINED) {
            sub_cmd->stencil_usage = PVR_DEPTH_STENCIL_USAGE_NEVER;
         }

         if (has_depth_clear &&
             sub_cmd->depth_usage == PVR_DEPTH_STENCIL_USAGE_UNDEFINED) {
            sub_cmd->depth_usage = PVR_DEPTH_STENCIL_USAGE_NEVER;
         }
      }
   }

   pvr_clear_attachments(cmd_buffer,
                         attachmentCount,
                         pAttachments,
                         rectCount,
                         pRects,
                         false);
}

void pvr_CmdResolveImage2(VkCommandBuffer commandBuffer,
                             const VkResolveImageInfo2 *pResolveImageInfo)
{
   PVR_FROM_HANDLE(pvr_image, src, pResolveImageInfo->srcImage);
   PVR_FROM_HANDLE(pvr_image, dst, pResolveImageInfo->dstImage);
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   for (uint32_t i = 0U; i < pResolveImageInfo->regionCount; i++) {
      VkImageCopy2 region = {
         .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
         .srcSubresource = pResolveImageInfo->pRegions[i].srcSubresource,
         .srcOffset = pResolveImageInfo->pRegions[i].srcOffset,
         .dstSubresource = pResolveImageInfo->pRegions[i].dstSubresource,
         .dstOffset = pResolveImageInfo->pRegions[i].dstOffset,
         .extent = pResolveImageInfo->pRegions[i].extent,
      };

      VkResult result =
         pvr_copy_or_resolve_color_image_region(cmd_buffer, src, dst, &region);
      if (result != VK_SUCCESS)
         return;
   }
}
