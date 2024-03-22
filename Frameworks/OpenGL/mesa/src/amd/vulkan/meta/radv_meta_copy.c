/*
 * Copyright © 2016 Intel Corporation
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

#include "radv_meta.h"
#include "radv_private.h"
#include "radv_sdma.h"
#include "vk_format.h"

static VkFormat
vk_format_for_size(int bs)
{
   switch (bs) {
   case 1:
      return VK_FORMAT_R8_UINT;
   case 2:
      return VK_FORMAT_R8G8_UINT;
   case 4:
      return VK_FORMAT_R8G8B8A8_UINT;
   case 8:
      return VK_FORMAT_R16G16B16A16_UINT;
   case 12:
      return VK_FORMAT_R32G32B32_UINT;
   case 16:
      return VK_FORMAT_R32G32B32A32_UINT;
   default:
      unreachable("Invalid format block size");
   }
}

static struct radv_meta_blit2d_surf
blit_surf_for_image_level_layer(struct radv_image *image, VkImageLayout layout, const VkImageSubresourceLayers *subres,
                                VkImageAspectFlags aspect_mask)
{
   VkFormat format = radv_get_aspect_format(image, aspect_mask);

   if (!radv_dcc_enabled(image, subres->mipLevel) && !(radv_image_is_tc_compat_htile(image)))
      format = vk_format_for_size(vk_format_get_blocksize(format));

   format = vk_format_no_srgb(format);

   return (struct radv_meta_blit2d_surf){
      .format = format,
      .bs = vk_format_get_blocksize(format),
      .level = subres->mipLevel,
      .layer = subres->baseArrayLayer,
      .image = image,
      .aspect_mask = aspect_mask,
      .current_layout = layout,
   };
}

static bool
alloc_transfer_temp_bo(struct radv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->transfer.copy_temp)
      return true;

   const struct radv_device *const device = cmd_buffer->device;
   const VkResult r = device->ws->buffer_create(device->ws, RADV_SDMA_TRANSFER_TEMP_BYTES, 4096, RADEON_DOMAIN_VRAM,
                                                RADEON_FLAG_NO_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING,
                                                RADV_BO_PRIORITY_SCRATCH, 0, &cmd_buffer->transfer.copy_temp);

   if (r != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd_buffer->vk, r);
      return false;
   }

   radv_cs_add_buffer(device->ws, cmd_buffer->cs, cmd_buffer->transfer.copy_temp);
   return true;
}

static void
transfer_copy_buffer_image(struct radv_cmd_buffer *cmd_buffer, struct radv_buffer *buffer, struct radv_image *image,
                           const VkBufferImageCopy2 *region, bool to_image)
{
   const struct radv_device *device = cmd_buffer->device;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const VkImageAspectFlags aspect_mask = region->imageSubresource.aspectMask;
   const unsigned binding_idx = image->disjoint ? radv_plane_from_aspect(aspect_mask) : 0;

   radv_cs_add_buffer(device->ws, cs, image->bindings[binding_idx].bo);
   radv_cs_add_buffer(device->ws, cs, buffer->bo);

   struct radv_sdma_surf buf = radv_sdma_get_buf_surf(buffer, image, region, aspect_mask);
   const struct radv_sdma_surf img =
      radv_sdma_get_surf(device, image, region->imageSubresource, region->imageOffset, aspect_mask);
   const VkExtent3D extent = radv_sdma_get_copy_extent(image, region->imageSubresource, region->imageExtent);

   if (radv_sdma_use_unaligned_buffer_image_copy(device, &buf, &img, extent)) {
      if (!alloc_transfer_temp_bo(cmd_buffer))
         return;

      radv_sdma_copy_buffer_image_unaligned(device, cs, &buf, &img, extent, cmd_buffer->transfer.copy_temp, to_image);
      return;
   }

   radv_sdma_copy_buffer_image(device, cs, &buf, &img, extent, to_image);
}

static void
copy_buffer_to_image(struct radv_cmd_buffer *cmd_buffer, struct radv_buffer *buffer, struct radv_image *image,
                     VkImageLayout layout, const VkBufferImageCopy2 *region)
{
   if (cmd_buffer->qf == RADV_QUEUE_TRANSFER) {
      transfer_copy_buffer_image(cmd_buffer, buffer, image, region, true);
      return;
   }

   struct radv_meta_saved_state saved_state;
   bool cs;

   /* The Vulkan 1.0 spec says "dstImage must have a sample count equal to
    * VK_SAMPLE_COUNT_1_BIT."
    */
   assert(image->vk.samples == 1);

   cs = cmd_buffer->qf == RADV_QUEUE_COMPUTE || !radv_image_is_renderable(cmd_buffer->device, image);

   /* VK_EXT_conditional_rendering says that copy commands should not be
    * affected by conditional rendering.
    */
   radv_meta_save(&saved_state, cmd_buffer,
                  (cs ? RADV_META_SAVE_COMPUTE_PIPELINE : RADV_META_SAVE_GRAPHICS_PIPELINE) | RADV_META_SAVE_CONSTANTS |
                     RADV_META_SAVE_DESCRIPTORS | RADV_META_SUSPEND_PREDICATING);

   /**
    * From the Vulkan 1.0.6 spec: 18.3 Copying Data Between Images
    *    extent is the size in texels of the source image to copy in width,
    *    height and depth. 1D images use only x and width. 2D images use x, y,
    *    width and height. 3D images use x, y, z, width, height and depth.
    *
    *
    * Also, convert the offsets and extent from units of texels to units of
    * blocks - which is the highest resolution accessible in this command.
    */
   const VkOffset3D img_offset_el = vk_image_offset_to_elements(&image->vk, region->imageOffset);

   /* Start creating blit rect */
   const VkExtent3D img_extent_el = vk_image_extent_to_elements(&image->vk, region->imageExtent);
   struct radv_meta_blit2d_rect rect = {
      .width = img_extent_el.width,
      .height = img_extent_el.height,
   };

   /* Create blit surfaces */
   struct radv_meta_blit2d_surf img_bsurf =
      blit_surf_for_image_level_layer(image, layout, &region->imageSubresource, region->imageSubresource.aspectMask);

   if (!radv_is_buffer_format_supported(img_bsurf.format, NULL)) {
      uint32_t queue_mask = radv_image_queue_family_mask(image, cmd_buffer->qf, cmd_buffer->qf);
      bool compressed =
         radv_layout_dcc_compressed(cmd_buffer->device, image, region->imageSubresource.mipLevel, layout, queue_mask);
      if (compressed) {
         radv_describe_barrier_start(cmd_buffer, RGP_BARRIER_UNKNOWN_REASON);

         radv_decompress_dcc(cmd_buffer, image,
                             &(VkImageSubresourceRange){
                                .aspectMask = region->imageSubresource.aspectMask,
                                .baseMipLevel = region->imageSubresource.mipLevel,
                                .levelCount = 1,
                                .baseArrayLayer = region->imageSubresource.baseArrayLayer,
                                .layerCount = vk_image_subresource_layer_count(&image->vk, &region->imageSubresource),
                             });
         img_bsurf.disable_compression = true;

         radv_describe_barrier_end(cmd_buffer);
      }
      img_bsurf.format = vk_format_for_size(vk_format_get_blocksize(img_bsurf.format));
   }

   const struct vk_image_buffer_layout buf_layout = vk_image_buffer_copy_layout(&image->vk, region);
   struct radv_meta_blit2d_buffer buf_bsurf = {
      .bs = img_bsurf.bs,
      .format = img_bsurf.format,
      .buffer = buffer,
      .offset = region->bufferOffset,
      .pitch = buf_layout.row_stride_B / buf_layout.element_size_B,
   };

   if (image->vk.image_type == VK_IMAGE_TYPE_3D)
      img_bsurf.layer = img_offset_el.z;
   /* Loop through each 3D or array slice */
   unsigned num_slices_3d = img_extent_el.depth;
   unsigned num_slices_array = vk_image_subresource_layer_count(&image->vk, &region->imageSubresource);
   unsigned slice_3d = 0;
   unsigned slice_array = 0;
   while (slice_3d < num_slices_3d && slice_array < num_slices_array) {

      rect.dst_x = img_offset_el.x;
      rect.dst_y = img_offset_el.y;

      /* Perform Blit */
      if (cs) {
         radv_meta_buffer_to_image_cs(cmd_buffer, &buf_bsurf, &img_bsurf, 1, &rect);
      } else {
         radv_meta_blit2d(cmd_buffer, NULL, &buf_bsurf, &img_bsurf, 1, &rect);
      }

      /* Once we've done the blit, all of the actual information about
       * the image is embedded in the command buffer so we can just
       * increment the offset directly in the image effectively
       * re-binding it to different backing memory.
       */
      buf_bsurf.offset += buf_layout.image_stride_B;
      img_bsurf.layer++;
      if (image->vk.image_type == VK_IMAGE_TYPE_3D)
         slice_3d++;
      else
         slice_array++;
   }

   radv_meta_restore(&saved_state, cmd_buffer);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, src_buffer, pCopyBufferToImageInfo->srcBuffer);
   RADV_FROM_HANDLE(radv_image, dst_image, pCopyBufferToImageInfo->dstImage);

   for (unsigned r = 0; r < pCopyBufferToImageInfo->regionCount; r++) {
      copy_buffer_to_image(cmd_buffer, src_buffer, dst_image, pCopyBufferToImageInfo->dstImageLayout,
                           &pCopyBufferToImageInfo->pRegions[r]);
   }

   if (radv_is_format_emulated(cmd_buffer->device->physical_device, dst_image->vk.format)) {
      cmd_buffer->state.flush_bits |=
         RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_PS_PARTIAL_FLUSH |
         radv_src_access_flush(cmd_buffer, VK_ACCESS_TRANSFER_WRITE_BIT, dst_image) |
         radv_dst_access_flush(cmd_buffer, VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, dst_image);

      const enum util_format_layout format_layout = vk_format_description(dst_image->vk.format)->layout;
      for (unsigned r = 0; r < pCopyBufferToImageInfo->regionCount; r++) {
         if (format_layout == UTIL_FORMAT_LAYOUT_ASTC) {
            radv_meta_decode_astc(cmd_buffer, dst_image, pCopyBufferToImageInfo->dstImageLayout,
                                  &pCopyBufferToImageInfo->pRegions[r].imageSubresource,
                                  pCopyBufferToImageInfo->pRegions[r].imageOffset,
                                  pCopyBufferToImageInfo->pRegions[r].imageExtent);
         } else {
            radv_meta_decode_etc(cmd_buffer, dst_image, pCopyBufferToImageInfo->dstImageLayout,
                                 &pCopyBufferToImageInfo->pRegions[r].imageSubresource,
                                 pCopyBufferToImageInfo->pRegions[r].imageOffset,
                                 pCopyBufferToImageInfo->pRegions[r].imageExtent);
         }
      }
   }
}

static void
copy_image_to_buffer(struct radv_cmd_buffer *cmd_buffer, struct radv_buffer *buffer, struct radv_image *image,
                     VkImageLayout layout, const VkBufferImageCopy2 *region)
{
   struct radv_device *device = cmd_buffer->device;
   if (cmd_buffer->qf == RADV_QUEUE_TRANSFER) {
      transfer_copy_buffer_image(cmd_buffer, buffer, image, region, false);
      return;
   }

   struct radv_meta_saved_state saved_state;

   /* VK_EXT_conditional_rendering says that copy commands should not be
    * affected by conditional rendering.
    */
   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_COMPUTE_PIPELINE | RADV_META_SAVE_CONSTANTS | RADV_META_SAVE_DESCRIPTORS |
                     RADV_META_SUSPEND_PREDICATING);

   /**
    * From the Vulkan 1.0.6 spec: 18.3 Copying Data Between Images
    *    extent is the size in texels of the source image to copy in width,
    *    height and depth. 1D images use only x and width. 2D images use x, y,
    *    width and height. 3D images use x, y, z, width, height and depth.
    *
    *
    * Also, convert the offsets and extent from units of texels to units of
    * blocks - which is the highest resolution accessible in this command.
    */
   const VkOffset3D img_offset_el = vk_image_offset_to_elements(&image->vk, region->imageOffset);
   const VkExtent3D bufferExtent = {
      .width = region->bufferRowLength ? region->bufferRowLength : region->imageExtent.width,
      .height = region->bufferImageHeight ? region->bufferImageHeight : region->imageExtent.height,
   };
   const VkExtent3D buf_extent_el = vk_image_extent_to_elements(&image->vk, bufferExtent);

   /* Start creating blit rect */
   const VkExtent3D img_extent_el = vk_image_extent_to_elements(&image->vk, region->imageExtent);
   struct radv_meta_blit2d_rect rect = {
      .width = img_extent_el.width,
      .height = img_extent_el.height,
   };

   /* Create blit surfaces */
   struct radv_meta_blit2d_surf img_info =
      blit_surf_for_image_level_layer(image, layout, &region->imageSubresource, region->imageSubresource.aspectMask);

   if (!radv_is_buffer_format_supported(img_info.format, NULL)) {
      uint32_t queue_mask = radv_image_queue_family_mask(image, cmd_buffer->qf, cmd_buffer->qf);
      bool compressed =
         radv_layout_dcc_compressed(device, image, region->imageSubresource.mipLevel, layout, queue_mask);
      if (compressed) {
         radv_describe_barrier_start(cmd_buffer, RGP_BARRIER_UNKNOWN_REASON);

         radv_decompress_dcc(cmd_buffer, image,
                             &(VkImageSubresourceRange){
                                .aspectMask = region->imageSubresource.aspectMask,
                                .baseMipLevel = region->imageSubresource.mipLevel,
                                .levelCount = 1,
                                .baseArrayLayer = region->imageSubresource.baseArrayLayer,
                                .layerCount = vk_image_subresource_layer_count(&image->vk, &region->imageSubresource),
                             });
         img_info.disable_compression = true;

         radv_describe_barrier_end(cmd_buffer);
      }
      img_info.format = vk_format_for_size(vk_format_get_blocksize(img_info.format));
   }

   struct radv_meta_blit2d_buffer buf_info = {
      .bs = img_info.bs,
      .format = img_info.format,
      .buffer = buffer,
      .offset = region->bufferOffset,
      .pitch = buf_extent_el.width,
   };

   if (image->vk.image_type == VK_IMAGE_TYPE_3D)
      img_info.layer = img_offset_el.z;
   /* Loop through each 3D or array slice */
   unsigned num_slices_3d = img_extent_el.depth;
   unsigned num_slices_array = vk_image_subresource_layer_count(&image->vk, &region->imageSubresource);
   unsigned slice_3d = 0;
   unsigned slice_array = 0;
   while (slice_3d < num_slices_3d && slice_array < num_slices_array) {

      rect.src_x = img_offset_el.x;
      rect.src_y = img_offset_el.y;

      /* Perform Blit */
      radv_meta_image_to_buffer(cmd_buffer, &img_info, &buf_info, 1, &rect);

      buf_info.offset += buf_extent_el.width * buf_extent_el.height * buf_info.bs;
      img_info.layer++;
      if (image->vk.image_type == VK_IMAGE_TYPE_3D)
         slice_3d++;
      else
         slice_array++;
   }

   radv_meta_restore(&saved_state, cmd_buffer);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_image, src_image, pCopyImageToBufferInfo->srcImage);
   RADV_FROM_HANDLE(radv_buffer, dst_buffer, pCopyImageToBufferInfo->dstBuffer);

   for (unsigned r = 0; r < pCopyImageToBufferInfo->regionCount; r++) {
      copy_image_to_buffer(cmd_buffer, dst_buffer, src_image, pCopyImageToBufferInfo->srcImageLayout,
                           &pCopyImageToBufferInfo->pRegions[r]);
   }
}

static void
transfer_copy_image(struct radv_cmd_buffer *cmd_buffer, struct radv_image *src_image, VkImageLayout src_image_layout,
                    struct radv_image *dst_image, VkImageLayout dst_image_layout, const VkImageCopy2 *region)
{
   const struct radv_device *device = cmd_buffer->device;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   unsigned int dst_aspect_mask_remaining = region->dstSubresource.aspectMask;

   u_foreach_bit (b, region->srcSubresource.aspectMask) {
      const VkImageAspectFlags src_aspect_mask = BITFIELD_BIT(b);
      const VkImageAspectFlags dst_aspect_mask = BITFIELD_BIT(u_bit_scan(&dst_aspect_mask_remaining));
      const unsigned src_binding_idx = src_image->disjoint ? radv_plane_from_aspect(src_aspect_mask) : 0;
      const unsigned dst_binding_idx = dst_image->disjoint ? radv_plane_from_aspect(dst_aspect_mask) : 0;

      radv_cs_add_buffer(device->ws, cs, src_image->bindings[src_binding_idx].bo);
      radv_cs_add_buffer(device->ws, cs, dst_image->bindings[dst_binding_idx].bo);

      const struct radv_sdma_surf src =
         radv_sdma_get_surf(device, src_image, region->srcSubresource, region->srcOffset, src_aspect_mask);
      const struct radv_sdma_surf dst =
         radv_sdma_get_surf(device, dst_image, region->dstSubresource, region->dstOffset, dst_aspect_mask);
      const VkExtent3D extent = radv_sdma_get_copy_extent(src_image, region->srcSubresource, region->extent);

      if (radv_sdma_use_t2t_scanline_copy(device, &src, &dst, extent)) {
         if (!alloc_transfer_temp_bo(cmd_buffer))
            return;

         radv_sdma_copy_image_t2t_scanline(device, cs, &src, &dst, extent, cmd_buffer->transfer.copy_temp);
      } else {
         radv_sdma_copy_image(device, cs, &src, &dst, extent);
      }
   }
}

static void
copy_image(struct radv_cmd_buffer *cmd_buffer, struct radv_image *src_image, VkImageLayout src_image_layout,
           struct radv_image *dst_image, VkImageLayout dst_image_layout, const VkImageCopy2 *region)
{
   if (cmd_buffer->qf == RADV_QUEUE_TRANSFER) {
      transfer_copy_image(cmd_buffer, src_image, src_image_layout, dst_image, dst_image_layout, region);
      return;
   }

   struct radv_meta_saved_state saved_state;
   bool cs;

   /* From the Vulkan 1.0 spec:
    *
    *    vkCmdCopyImage can be used to copy image data between multisample images, but both images must have the same
    *    number of samples.
    */
   assert(src_image->vk.samples == dst_image->vk.samples);
   /* From the Vulkan 1.3 spec:
    *
    *    Multi-planar images can only be copied on a per-plane basis, and the subresources used in each region when
    *    copying to or from such images must specify only one plane, though different regions can specify different
    *    planes.
    */
   assert(src_image->plane_count == 1 || util_is_power_of_two_nonzero(region->srcSubresource.aspectMask));
   assert(dst_image->plane_count == 1 || util_is_power_of_two_nonzero(region->dstSubresource.aspectMask));

   cs = cmd_buffer->qf == RADV_QUEUE_COMPUTE || !radv_image_is_renderable(cmd_buffer->device, dst_image);

   /* VK_EXT_conditional_rendering says that copy commands should not be
    * affected by conditional rendering.
    */
   radv_meta_save(&saved_state, cmd_buffer,
                  (cs ? RADV_META_SAVE_COMPUTE_PIPELINE : RADV_META_SAVE_GRAPHICS_PIPELINE) | RADV_META_SAVE_CONSTANTS |
                     RADV_META_SAVE_DESCRIPTORS | RADV_META_SUSPEND_PREDICATING);

   if (cs) {
      /* For partial copies, HTILE should be decompressed before copying because the metadata is
       * re-initialized to the uncompressed state after.
       */
      uint32_t queue_mask = radv_image_queue_family_mask(dst_image, cmd_buffer->qf, cmd_buffer->qf);

      if (radv_layout_is_htile_compressed(cmd_buffer->device, dst_image, dst_image_layout, queue_mask) &&
          (region->dstOffset.x || region->dstOffset.y || region->dstOffset.z ||
           region->extent.width != dst_image->vk.extent.width || region->extent.height != dst_image->vk.extent.height ||
           region->extent.depth != dst_image->vk.extent.depth)) {
         radv_describe_barrier_start(cmd_buffer, RGP_BARRIER_UNKNOWN_REASON);

         u_foreach_bit (i, region->dstSubresource.aspectMask) {
            unsigned aspect_mask = 1u << i;
            radv_expand_depth_stencil(
               cmd_buffer, dst_image,
               &(VkImageSubresourceRange){
                  .aspectMask = aspect_mask,
                  .baseMipLevel = region->dstSubresource.mipLevel,
                  .levelCount = 1,
                  .baseArrayLayer = region->dstSubresource.baseArrayLayer,
                  .layerCount = vk_image_subresource_layer_count(&dst_image->vk, &region->dstSubresource),
               },
               NULL);
         }

         radv_describe_barrier_end(cmd_buffer);
      }
   }

   /* Create blit surfaces */
   struct radv_meta_blit2d_surf b_src = blit_surf_for_image_level_layer(
      src_image, src_image_layout, &region->srcSubresource, region->srcSubresource.aspectMask);

   struct radv_meta_blit2d_surf b_dst = blit_surf_for_image_level_layer(
      dst_image, dst_image_layout, &region->dstSubresource, region->dstSubresource.aspectMask);

   uint32_t dst_queue_mask = radv_image_queue_family_mask(dst_image, cmd_buffer->qf, cmd_buffer->qf);
   bool dst_compressed = radv_layout_dcc_compressed(cmd_buffer->device, dst_image, region->dstSubresource.mipLevel,
                                                    dst_image_layout, dst_queue_mask);
   uint32_t src_queue_mask = radv_image_queue_family_mask(src_image, cmd_buffer->qf, cmd_buffer->qf);
   bool src_compressed = radv_layout_dcc_compressed(cmd_buffer->device, src_image, region->srcSubresource.mipLevel,
                                                    src_image_layout, src_queue_mask);
   bool need_dcc_sign_reinterpret = false;

   if (!src_compressed || (radv_dcc_formats_compatible(cmd_buffer->device->physical_device->rad_info.gfx_level,
                                                       b_src.format, b_dst.format, &need_dcc_sign_reinterpret) &&
                           !need_dcc_sign_reinterpret)) {
      b_src.format = b_dst.format;
   } else if (!dst_compressed) {
      b_dst.format = b_src.format;
   } else {
      radv_describe_barrier_start(cmd_buffer, RGP_BARRIER_UNKNOWN_REASON);

      radv_decompress_dcc(cmd_buffer, dst_image,
                          &(VkImageSubresourceRange){
                             .aspectMask = region->dstSubresource.aspectMask,
                             .baseMipLevel = region->dstSubresource.mipLevel,
                             .levelCount = 1,
                             .baseArrayLayer = region->dstSubresource.baseArrayLayer,
                             .layerCount = vk_image_subresource_layer_count(&dst_image->vk, &region->dstSubresource),
                          });
      b_dst.format = b_src.format;
      b_dst.disable_compression = true;

      radv_describe_barrier_end(cmd_buffer);
   }

   /**
    * From the Vulkan 1.0.6 spec: 18.4 Copying Data Between Buffers and Images
    *    imageExtent is the size in texels of the image to copy in width, height
    *    and depth. 1D images use only x and width. 2D images use x, y, width
    *    and height. 3D images use x, y, z, width, height and depth.
    *
    * Also, convert the offsets and extent from units of texels to units of
    * blocks - which is the highest resolution accessible in this command.
    */
   const VkOffset3D dst_offset_el = vk_image_offset_to_elements(&dst_image->vk, region->dstOffset);
   const VkOffset3D src_offset_el = vk_image_offset_to_elements(&src_image->vk, region->srcOffset);

   /*
    * From Vulkan 1.0.68, "Copying Data Between Images":
    *    "When copying between compressed and uncompressed formats
    *     the extent members represent the texel dimensions of the
    *     source image and not the destination."
    * However, we must use the destination image type to avoid
    * clamping depth when copying multiple layers of a 2D image to
    * a 3D image.
    */
   const VkExtent3D img_extent_el = vk_image_extent_to_elements(&src_image->vk, region->extent);

   /* Start creating blit rect */
   struct radv_meta_blit2d_rect rect = {
      .width = img_extent_el.width,
      .height = img_extent_el.height,
   };

   unsigned num_slices = vk_image_subresource_layer_count(&src_image->vk, &region->srcSubresource);

   if (src_image->vk.image_type == VK_IMAGE_TYPE_3D) {
      b_src.layer = src_offset_el.z;
      num_slices = img_extent_el.depth;
   }

   if (dst_image->vk.image_type == VK_IMAGE_TYPE_3D)
      b_dst.layer = dst_offset_el.z;

   for (unsigned slice = 0; slice < num_slices; slice++) {
      /* Finish creating blit rect */
      rect.dst_x = dst_offset_el.x;
      rect.dst_y = dst_offset_el.y;
      rect.src_x = src_offset_el.x;
      rect.src_y = src_offset_el.y;

      /* Perform Blit */
      if (cs) {
         radv_meta_image_to_image_cs(cmd_buffer, &b_src, &b_dst, 1, &rect);
      } else {
         if (radv_can_use_fmask_copy(cmd_buffer, b_src.image, b_dst.image, 1, &rect)) {
            radv_fmask_copy(cmd_buffer, &b_src, &b_dst);
         } else {
            radv_meta_blit2d(cmd_buffer, &b_src, NULL, &b_dst, 1, &rect);
         }
      }

      b_src.layer++;
      b_dst.layer++;
   }

   if (cs) {
      /* Fixup HTILE after a copy on compute. */
      uint32_t queue_mask = radv_image_queue_family_mask(dst_image, cmd_buffer->qf, cmd_buffer->qf);

      if (radv_layout_is_htile_compressed(cmd_buffer->device, dst_image, dst_image_layout, queue_mask)) {
         cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_INV_VCACHE;

         VkImageSubresourceRange range = {
            .aspectMask = region->dstSubresource.aspectMask,
            .baseMipLevel = region->dstSubresource.mipLevel,
            .levelCount = 1,
            .baseArrayLayer = region->dstSubresource.baseArrayLayer,
            .layerCount = vk_image_subresource_layer_count(&dst_image->vk, &region->dstSubresource),
         };

         uint32_t htile_value = radv_get_htile_initial_value(cmd_buffer->device, dst_image);

         cmd_buffer->state.flush_bits |= radv_clear_htile(cmd_buffer, dst_image, &range, htile_value);
      }
   }

   radv_meta_restore(&saved_state, cmd_buffer);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_image, src_image, pCopyImageInfo->srcImage);
   RADV_FROM_HANDLE(radv_image, dst_image, pCopyImageInfo->dstImage);

   for (unsigned r = 0; r < pCopyImageInfo->regionCount; r++) {
      copy_image(cmd_buffer, src_image, pCopyImageInfo->srcImageLayout, dst_image, pCopyImageInfo->dstImageLayout,
                 &pCopyImageInfo->pRegions[r]);
   }

   if (radv_is_format_emulated(cmd_buffer->device->physical_device, dst_image->vk.format)) {
      cmd_buffer->state.flush_bits |=
         RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_PS_PARTIAL_FLUSH |
         radv_src_access_flush(cmd_buffer, VK_ACCESS_TRANSFER_WRITE_BIT, dst_image) |
         radv_dst_access_flush(cmd_buffer, VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, dst_image);

      const enum util_format_layout format_layout = vk_format_description(dst_image->vk.format)->layout;
      for (unsigned r = 0; r < pCopyImageInfo->regionCount; r++) {
         VkExtent3D dst_extent = pCopyImageInfo->pRegions[r].extent;
         if (src_image->vk.format != dst_image->vk.format) {
            dst_extent.width = dst_extent.width / vk_format_get_blockwidth(src_image->vk.format) *
                               vk_format_get_blockwidth(dst_image->vk.format);
            dst_extent.height = dst_extent.height / vk_format_get_blockheight(src_image->vk.format) *
                                vk_format_get_blockheight(dst_image->vk.format);
         }
         if (format_layout == UTIL_FORMAT_LAYOUT_ASTC) {
            radv_meta_decode_astc(cmd_buffer, dst_image, pCopyImageInfo->dstImageLayout,
                                  &pCopyImageInfo->pRegions[r].dstSubresource, pCopyImageInfo->pRegions[r].dstOffset,
                                  dst_extent);
         } else {
            radv_meta_decode_etc(cmd_buffer, dst_image, pCopyImageInfo->dstImageLayout,
                                 &pCopyImageInfo->pRegions[r].dstSubresource, pCopyImageInfo->pRegions[r].dstOffset,
                                 dst_extent);
         }
      }
   }
}
