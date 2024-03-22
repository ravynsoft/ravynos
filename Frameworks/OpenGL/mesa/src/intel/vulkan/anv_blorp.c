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

#include "anv_private.h"
#include "genxml/gen8_pack.h"

static bool
lookup_blorp_shader(struct blorp_batch *batch,
                    const void *key, uint32_t key_size,
                    uint32_t *kernel_out, void *prog_data_out)
{
   struct blorp_context *blorp = batch->blorp;
   struct anv_device *device = blorp->driver_ctx;

   struct anv_shader_bin *bin =
      anv_device_search_for_kernel(device, device->internal_cache,
                                   key, key_size, NULL);
   if (!bin)
      return false;

   /* The cache already has a reference and it's not going anywhere so there
    * is no need to hold a second reference.
    */
   anv_shader_bin_unref(device, bin);

   *kernel_out = bin->kernel.offset;
   *(const struct brw_stage_prog_data **)prog_data_out = bin->prog_data;

   return true;
}

static bool
upload_blorp_shader(struct blorp_batch *batch, uint32_t stage,
                    const void *key, uint32_t key_size,
                    const void *kernel, uint32_t kernel_size,
                    const struct brw_stage_prog_data *prog_data,
                    uint32_t prog_data_size,
                    uint32_t *kernel_out, void *prog_data_out)
{
   struct blorp_context *blorp = batch->blorp;
   struct anv_device *device = blorp->driver_ctx;

   struct anv_pipeline_bind_map bind_map = {
      .surface_count = 0,
      .sampler_count = 0,
   };
   struct anv_push_descriptor_info push_desc_info = {};

   struct anv_shader_bin *bin =
      anv_device_upload_kernel(device, device->internal_cache, stage,
                               key, key_size, kernel, kernel_size,
                               prog_data, prog_data_size,
                               NULL, 0, NULL, &bind_map,
                               &push_desc_info,
                               0 /* dynamic_push_values */);

   if (!bin)
      return false;

   /* The cache already has a reference and it's not going anywhere so there
    * is no need to hold a second reference.
    */
   anv_shader_bin_unref(device, bin);

   *kernel_out = bin->kernel.offset;
   *(const struct brw_stage_prog_data **)prog_data_out = bin->prog_data;

   return true;
}

void
anv_device_init_blorp(struct anv_device *device)
{
   const struct blorp_config config = {
      .use_mesh_shading = device->vk.enabled_extensions.EXT_mesh_shader,
      .use_unrestricted_depth_range =
         device->vk.enabled_extensions.EXT_depth_range_unrestricted,
   };

   blorp_init(&device->blorp, device, &device->isl_dev, &config);
   device->blorp.compiler = device->physical->compiler;
   device->blorp.lookup_shader = lookup_blorp_shader;
   device->blorp.upload_shader = upload_blorp_shader;
   device->blorp.enable_tbimr = device->physical->instance->enable_tbimr;
   device->blorp.exec = anv_genX(device->info, blorp_exec);
}

void
anv_device_finish_blorp(struct anv_device *device)
{
   blorp_finish(&device->blorp);
}

static void
anv_blorp_batch_init(struct anv_cmd_buffer *cmd_buffer,
                     struct blorp_batch *batch, enum blorp_batch_flags flags)
{
   VkQueueFlags queue_flags = cmd_buffer->queue_family->queueFlags;

   if (queue_flags & VK_QUEUE_GRAPHICS_BIT) {
      /* blorp runs on render engine by default */
   } else if (queue_flags & VK_QUEUE_COMPUTE_BIT) {
      flags |= BLORP_BATCH_USE_COMPUTE;
   } else if (queue_flags & VK_QUEUE_TRANSFER_BIT) {
      flags |= BLORP_BATCH_USE_BLITTER;
   } else {
      unreachable("unknown queue family");
   }

   blorp_batch_init(&cmd_buffer->device->blorp, batch, cmd_buffer, flags);
}

static void
anv_blorp_batch_finish(struct blorp_batch *batch)
{
   blorp_batch_finish(batch);
}

static isl_surf_usage_flags_t
get_usage_flag_for_cmd_buffer(const struct anv_cmd_buffer *cmd_buffer,
                              bool is_dest)
{
   isl_surf_usage_flags_t usage;

   switch (cmd_buffer->queue_family->engine_class) {
   case INTEL_ENGINE_CLASS_RENDER:
      usage = is_dest ? ISL_SURF_USAGE_RENDER_TARGET_BIT :
                        ISL_SURF_USAGE_TEXTURE_BIT;
      break;
   case INTEL_ENGINE_CLASS_COMPUTE:
      usage = is_dest ? ISL_SURF_USAGE_STORAGE_BIT :
                        ISL_SURF_USAGE_TEXTURE_BIT;
      break;
   case INTEL_ENGINE_CLASS_COPY:
      usage = is_dest ? ISL_SURF_USAGE_BLITTER_DST_BIT :
                        ISL_SURF_USAGE_BLITTER_SRC_BIT;
      break;
   default:
      unreachable("Unhandled engine class");
   }

   return usage;
}

static void
get_blorp_surf_for_anv_address(struct anv_cmd_buffer *cmd_buffer,
                               struct anv_address address,
                               uint32_t width, uint32_t height,
                               uint32_t row_pitch, enum isl_format format,
                               bool is_dest,
                               struct blorp_surf *blorp_surf,
                               struct isl_surf *isl_surf)
{
   bool ok UNUSED;
   isl_surf_usage_flags_t usage =
      get_usage_flag_for_cmd_buffer(cmd_buffer, is_dest);

   *blorp_surf = (struct blorp_surf) {
      .surf = isl_surf,
      .addr = {
         .buffer = address.bo,
         .offset = address.offset,
         .mocs = anv_mocs(cmd_buffer->device, address.bo, usage),
      },
   };

   ok = isl_surf_init(&cmd_buffer->device->isl_dev, isl_surf,
                     .dim = ISL_SURF_DIM_2D,
                     .format = format,
                     .width = width,
                     .height = height,
                     .depth = 1,
                     .levels = 1,
                     .array_len = 1,
                     .samples = 1,
                     .row_pitch_B = row_pitch,
                     .usage = usage,
                     .tiling_flags = ISL_TILING_LINEAR_BIT);
   assert(ok);
}

static void
get_blorp_surf_for_anv_buffer(struct anv_cmd_buffer *cmd_buffer,
                              struct anv_buffer *buffer, uint64_t offset,
                              uint32_t width, uint32_t height,
                              uint32_t row_pitch, enum isl_format format,
                              bool is_dest,
                              struct blorp_surf *blorp_surf,
                              struct isl_surf *isl_surf)
{
   get_blorp_surf_for_anv_address(cmd_buffer,
                                  anv_address_add(buffer->address, offset),
                                  width, height, row_pitch, format,
                                  is_dest, blorp_surf, isl_surf);
}

/* Pick something high enough that it won't be used in core and low enough it
 * will never map to an extension.
 */
#define ANV_IMAGE_LAYOUT_EXPLICIT_AUX (VkImageLayout)10000000

static struct blorp_address
anv_to_blorp_address(struct anv_address addr)
{
   return (struct blorp_address) {
      .buffer = addr.bo,
      .offset = addr.offset,
   };
}

static void
get_blorp_surf_for_anv_image(const struct anv_cmd_buffer *cmd_buffer,
                             const struct anv_image *image,
                             VkImageAspectFlags aspect,
                             VkImageUsageFlags usage,
                             VkImageLayout layout,
                             enum isl_aux_usage aux_usage,
                             struct blorp_surf *blorp_surf)
{
   const struct anv_device *device = cmd_buffer->device;
   const uint32_t plane = anv_image_aspect_to_plane(image, aspect);

   if (layout != ANV_IMAGE_LAYOUT_EXPLICIT_AUX) {
      assert(usage != 0);
      aux_usage = anv_layout_to_aux_usage(device->info, image,
                                          aspect, usage, layout,
                                          cmd_buffer->queue_family->queueFlags);
   }

   isl_surf_usage_flags_t isl_usage =
      get_usage_flag_for_cmd_buffer(cmd_buffer,
                                    usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT);
   const struct anv_surface *surface = &image->planes[plane].primary_surface;
   const struct anv_address address =
      anv_image_address(image, &surface->memory_range);

   *blorp_surf = (struct blorp_surf) {
      .surf = &surface->isl,
      .addr = {
         .buffer = address.bo,
         .offset = address.offset,
         .mocs = anv_mocs(device, address.bo, isl_usage),
      },
   };

   if (aux_usage != ISL_AUX_USAGE_NONE) {
      const struct anv_surface *aux_surface = &image->planes[plane].aux_surface;
      const struct anv_address aux_address =
         anv_image_address(image, &aux_surface->memory_range);

      blorp_surf->aux_usage = aux_usage;
      blorp_surf->aux_surf = &aux_surface->isl;

      if (!anv_address_is_null(aux_address)) {
         blorp_surf->aux_addr = (struct blorp_address) {
            .buffer = aux_address.bo,
            .offset = aux_address.offset,
            .mocs = anv_mocs(device, aux_address.bo, isl_usage),
         };
      }

      /* If we're doing a partial resolve, then we need the indirect clear
       * color.  If we are doing a fast clear and want to store/update the
       * clear color, we also pass the address to blorp, otherwise it will only
       * stomp the CCS to a particular value and won't care about format or
       * clear value
       */
      if (aspect & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV) {
         const struct anv_address clear_color_addr =
            anv_image_get_clear_color_addr(device, image, aspect);
         blorp_surf->clear_color_addr = anv_to_blorp_address(clear_color_addr);
      } else if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
         const struct anv_address clear_color_addr =
            anv_image_get_clear_color_addr(device, image, aspect);
         blorp_surf->clear_color_addr = anv_to_blorp_address(clear_color_addr);
         blorp_surf->clear_color = (union isl_color_value) {
            .f32 = { ANV_HZ_FC_VAL },
         };
      }
   }
}

static void
copy_image(struct anv_cmd_buffer *cmd_buffer,
           struct blorp_batch *batch,
           struct anv_image *src_image,
           VkImageLayout src_image_layout,
           struct anv_image *dst_image,
           VkImageLayout dst_image_layout,
           const VkImageCopy2 *region)
{
   VkOffset3D srcOffset =
      vk_image_sanitize_offset(&src_image->vk, region->srcOffset);
   VkOffset3D dstOffset =
      vk_image_sanitize_offset(&dst_image->vk, region->dstOffset);
   VkExtent3D extent =
      vk_image_sanitize_extent(&src_image->vk, region->extent);

   const uint32_t dst_level = region->dstSubresource.mipLevel;
   unsigned dst_base_layer, layer_count;
   if (dst_image->vk.image_type == VK_IMAGE_TYPE_3D) {
      dst_base_layer = region->dstOffset.z;
      layer_count = region->extent.depth;
   } else {
      dst_base_layer = region->dstSubresource.baseArrayLayer;
      layer_count = vk_image_subresource_layer_count(&dst_image->vk,
                                                     &region->dstSubresource);
   }

   const uint32_t src_level = region->srcSubresource.mipLevel;
   unsigned src_base_layer;
   if (src_image->vk.image_type == VK_IMAGE_TYPE_3D) {
      src_base_layer = region->srcOffset.z;
   } else {
      src_base_layer = region->srcSubresource.baseArrayLayer;
      assert(layer_count ==
             vk_image_subresource_layer_count(&src_image->vk,
                                              &region->srcSubresource));
   }

   VkImageAspectFlags src_mask = region->srcSubresource.aspectMask,
      dst_mask = region->dstSubresource.aspectMask;

   assert(anv_image_aspects_compatible(src_mask, dst_mask));

   if (util_bitcount(src_mask) > 1) {
      anv_foreach_image_aspect_bit(aspect_bit, src_image, src_mask) {
         struct blorp_surf src_surf, dst_surf;
         get_blorp_surf_for_anv_image(cmd_buffer,
                                      src_image, 1UL << aspect_bit,
                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                      src_image_layout, ISL_AUX_USAGE_NONE,
                                      &src_surf);
         get_blorp_surf_for_anv_image(cmd_buffer,
                                      dst_image, 1UL << aspect_bit,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                      dst_image_layout, ISL_AUX_USAGE_NONE,
                                      &dst_surf);
         anv_cmd_buffer_mark_image_written(cmd_buffer, dst_image,
                                           1UL << aspect_bit,
                                           dst_surf.aux_usage, dst_level,
                                           dst_base_layer, layer_count);

         for (unsigned i = 0; i < layer_count; i++) {
            blorp_copy(batch, &src_surf, src_level, src_base_layer + i,
                       &dst_surf, dst_level, dst_base_layer + i,
                       srcOffset.x, srcOffset.y,
                       dstOffset.x, dstOffset.y,
                       extent.width, extent.height);
         }
      }
   } else {
      /* This case handles the ycbcr images, aspect mask are compatible but
       * don't need to be the same.
       */
      struct blorp_surf src_surf, dst_surf;
      get_blorp_surf_for_anv_image(cmd_buffer, src_image, src_mask,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                   src_image_layout, ISL_AUX_USAGE_NONE,
                                   &src_surf);
      get_blorp_surf_for_anv_image(cmd_buffer, dst_image, dst_mask,
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                   dst_image_layout, ISL_AUX_USAGE_NONE,
                                   &dst_surf);
      anv_cmd_buffer_mark_image_written(cmd_buffer, dst_image, dst_mask,
                                        dst_surf.aux_usage, dst_level,
                                        dst_base_layer, layer_count);

      for (unsigned i = 0; i < layer_count; i++) {
         blorp_copy(batch, &src_surf, src_level, src_base_layer + i,
                    &dst_surf, dst_level, dst_base_layer + i,
                    srcOffset.x, srcOffset.y,
                    dstOffset.x, dstOffset.y,
                    extent.width, extent.height);
      }
   }
}

static struct anv_state
record_main_rcs_cmd_buffer_done(struct anv_cmd_buffer *cmd_buffer)
{
   const struct intel_device_info *info = cmd_buffer->device->info;

   const VkResult result = anv_cmd_buffer_ensure_rcs_companion(cmd_buffer);
   if (result != VK_SUCCESS) {
      anv_batch_set_error(&cmd_buffer->batch, result);
      return ANV_STATE_NULL;
   }

   assert(cmd_buffer->companion_rcs_cmd_buffer != NULL);

   /* Re-emit the aux table register in every command buffer.  This way we're
    * ensured that we have the table even if this command buffer doesn't
    * initialize any images.
    */
   if (cmd_buffer->device->info->has_aux_map) {
      anv_add_pending_pipe_bits(cmd_buffer->companion_rcs_cmd_buffer,
                                 ANV_PIPE_AUX_TABLE_INVALIDATE_BIT,
                                 "new cmd buffer with aux-tt");
   }

   return anv_genX(info, cmd_buffer_begin_companion_rcs_syncpoint)(cmd_buffer);
}

static void
end_main_rcs_cmd_buffer_done(struct anv_cmd_buffer *cmd_buffer,
                             struct anv_state syncpoint)
{
   const struct intel_device_info *info = cmd_buffer->device->info;
   anv_genX(info, cmd_buffer_end_companion_rcs_syncpoint)(cmd_buffer,
                                                          syncpoint);
}

static bool
anv_blorp_blitter_execute_on_companion(struct anv_cmd_buffer *cmd_buffer,
                                       struct anv_image *image,
                                       const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                       const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo)
{
   if (!anv_cmd_buffer_is_blitter_queue(cmd_buffer))
      return false;

   assert((pCopyBufferToImageInfo && !pCopyImageToBufferInfo) ||
          (pCopyImageToBufferInfo && !pCopyBufferToImageInfo));

   bool blorp_execute_on_companion = false;
   VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_NONE;
   const uint32_t region_count = pCopyBufferToImageInfo ?
                                 pCopyBufferToImageInfo->regionCount :
                                 pCopyImageToBufferInfo->regionCount;

   for (unsigned r = 0; r < region_count &&
                            !blorp_execute_on_companion; r++) {
      if (pCopyBufferToImageInfo) {
         aspect_mask =
            pCopyBufferToImageInfo->pRegions[r].imageSubresource.aspectMask;
      } else {
         aspect_mask =
            pCopyImageToBufferInfo->pRegions[r].imageSubresource.aspectMask;
      }

      enum isl_format linear_format =
         anv_get_isl_format(cmd_buffer->device->info, image->vk.format,
                            aspect_mask, VK_IMAGE_TILING_LINEAR);
      const struct isl_format_layout *linear_fmtl =
         isl_format_get_layout(linear_format);

      switch (linear_fmtl->bpb) {
      case 96:
         /* We can only support linear mode for 96bpp on blitter engine. */
         blorp_execute_on_companion |=
            image->vk.tiling != VK_IMAGE_TILING_LINEAR;
         break;
      default:
         blorp_execute_on_companion |= linear_fmtl->bpb % 3 == 0;
         break;
      }
   }

   return blorp_execute_on_companion;
}

static bool
anv_blorp_execute_on_companion(struct anv_cmd_buffer *cmd_buffer,
                               struct anv_image *dst_image)
{
   /* MSAA images have to be dealt with on the companion RCS command buffer
    * for both CCS && BCS engines.
    */
   if ((anv_cmd_buffer_is_blitter_queue(cmd_buffer) ||
        anv_cmd_buffer_is_compute_queue(cmd_buffer)) &&
       dst_image->vk.samples > 1)
      return true;

   /* Emulation of formats is done through a compute shader, so we need
    * the companion command buffer for the BCS engine.
    */
   if (anv_cmd_buffer_is_blitter_queue(cmd_buffer) &&
       dst_image->emu_plane_format != VK_FORMAT_UNDEFINED)
      return true;

   return false;
}

void anv_CmdCopyImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_image, src_image, pCopyImageInfo->srcImage);
   ANV_FROM_HANDLE(anv_image, dst_image, pCopyImageInfo->dstImage);

   struct anv_cmd_buffer *main_cmd_buffer = cmd_buffer;
   UNUSED struct anv_state rcs_done = ANV_STATE_NULL;

   if (anv_blorp_execute_on_companion(cmd_buffer, dst_image)) {
      rcs_done = record_main_rcs_cmd_buffer_done(cmd_buffer);
      cmd_buffer = cmd_buffer->companion_rcs_cmd_buffer;
   }

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   for (unsigned r = 0; r < pCopyImageInfo->regionCount; r++) {
      copy_image(cmd_buffer, &batch,
                 src_image, pCopyImageInfo->srcImageLayout,
                 dst_image, pCopyImageInfo->dstImageLayout,
                 &pCopyImageInfo->pRegions[r]);
   }

   anv_blorp_batch_finish(&batch);

   if (dst_image->emu_plane_format != VK_FORMAT_UNDEFINED) {
      assert(!anv_cmd_buffer_is_blitter_queue(cmd_buffer));
      const enum anv_pipe_bits pipe_bits =
         anv_cmd_buffer_is_compute_queue(cmd_buffer) ?
         ANV_PIPE_HDC_PIPELINE_FLUSH_BIT :
         ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT;
      anv_add_pending_pipe_bits(cmd_buffer, pipe_bits,
                                "Copy flush before astc emu");

      for (unsigned r = 0; r < pCopyImageInfo->regionCount; r++) {
         const VkImageCopy2 *region = &pCopyImageInfo->pRegions[r];
         const VkOffset3D block_offset = vk_image_offset_to_elements(
               &dst_image->vk, region->dstOffset);
         const VkExtent3D block_extent = vk_image_extent_to_elements(
               &src_image->vk, region->extent);
         anv_astc_emu_process(cmd_buffer, dst_image,
                              pCopyImageInfo->dstImageLayout,
                              &region->dstSubresource,
                              block_offset, block_extent);
      }
   }

   if (rcs_done.alloc_size)
      end_main_rcs_cmd_buffer_done(main_cmd_buffer, rcs_done);
}

static enum isl_format
isl_format_for_size(unsigned size_B)
{
   /* Prefer 32-bit per component formats for CmdFillBuffer */
   switch (size_B) {
   case 1:  return ISL_FORMAT_R8_UINT;
   case 2:  return ISL_FORMAT_R16_UINT;
   case 3:  return ISL_FORMAT_R8G8B8_UINT;
   case 4:  return ISL_FORMAT_R32_UINT;
   case 6:  return ISL_FORMAT_R16G16B16_UINT;
   case 8:  return ISL_FORMAT_R32G32_UINT;
   case 12: return ISL_FORMAT_R32G32B32_UINT;
   case 16: return ISL_FORMAT_R32G32B32A32_UINT;
   default:
      unreachable("Unknown format size");
   }
}

static void
copy_buffer_to_image(struct anv_cmd_buffer *cmd_buffer,
                     struct blorp_batch *batch,
                     struct anv_buffer *anv_buffer,
                     struct anv_image *anv_image,
                     VkImageLayout image_layout,
                     const VkBufferImageCopy2* region,
                     bool buffer_to_image)
{
   struct {
      struct blorp_surf surf;
      uint32_t level;
      VkOffset3D offset;
   } image, buffer, *src, *dst;

   buffer.level = 0;
   buffer.offset = (VkOffset3D) { 0, 0, 0 };

   if (buffer_to_image) {
      src = &buffer;
      dst = &image;
   } else {
      src = &image;
      dst = &buffer;
   }

   const VkImageAspectFlags aspect = region->imageSubresource.aspectMask;

   get_blorp_surf_for_anv_image(cmd_buffer, anv_image, aspect,
                                buffer_to_image ?
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT :
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                image_layout, ISL_AUX_USAGE_NONE,
                                &image.surf);
   image.offset =
      vk_image_sanitize_offset(&anv_image->vk, region->imageOffset);
   image.level = region->imageSubresource.mipLevel;

   VkExtent3D extent =
      vk_image_sanitize_extent(&anv_image->vk, region->imageExtent);
   if (anv_image->vk.image_type != VK_IMAGE_TYPE_3D) {
      image.offset.z = region->imageSubresource.baseArrayLayer;
      extent.depth =
         vk_image_subresource_layer_count(&anv_image->vk,
                                          &region->imageSubresource);
   }

   const enum isl_format linear_format =
      anv_get_isl_format(cmd_buffer->device->info, anv_image->vk.format,
                         aspect, VK_IMAGE_TILING_LINEAR);
   const struct isl_format_layout *linear_fmtl =
      isl_format_get_layout(linear_format);

   const struct vk_image_buffer_layout buffer_layout =
      vk_image_buffer_copy_layout(&anv_image->vk, region);

   /* Some formats have additional restrictions which may cause ISL to
    * fail to create a surface for us.  For example, YCbCr formats
    * have to have 2-pixel aligned strides.
    *
    * To avoid these issues, we always bind the buffer as if it's a
    * "normal" format like RGBA32_UINT.  Since we're using blorp_copy,
    * the format doesn't matter as long as it has the right bpb.
    */
   const VkExtent2D buffer_extent = {
      .width = DIV_ROUND_UP(extent.width, linear_fmtl->bw),
      .height = DIV_ROUND_UP(extent.height, linear_fmtl->bh),
   };
   const enum isl_format buffer_format =
      isl_format_for_size(linear_fmtl->bpb / 8);

   struct isl_surf buffer_isl_surf;
   get_blorp_surf_for_anv_buffer(cmd_buffer,
                                 anv_buffer, region->bufferOffset,
                                 buffer_extent.width, buffer_extent.height,
                                 buffer_layout.row_stride_B, buffer_format,
                                 false, &buffer.surf, &buffer_isl_surf);

   if (&image == dst) {
      /* In this case, the source is the buffer and, since blorp takes its
       * copy dimensions in terms of the source format, we have to use the
       * scaled down version for compressed textures because the source
       * format is an RGB format.
       */
      extent.width = buffer_extent.width;
      extent.height = buffer_extent.height;

      anv_cmd_buffer_mark_image_written(cmd_buffer, anv_image,
                                        aspect, dst->surf.aux_usage,
                                        dst->level,
                                        dst->offset.z, extent.depth);
   }

   for (unsigned z = 0; z < extent.depth; z++) {
      blorp_copy(batch, &src->surf, src->level, src->offset.z,
                 &dst->surf, dst->level, dst->offset.z,
                 src->offset.x, src->offset.y, dst->offset.x, dst->offset.y,
                 extent.width, extent.height);

      image.offset.z++;
      buffer.surf.addr.offset += buffer_layout.image_stride_B;
   }
}

void anv_CmdCopyBufferToImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, src_buffer, pCopyBufferToImageInfo->srcBuffer);
   ANV_FROM_HANDLE(anv_image, dst_image, pCopyBufferToImageInfo->dstImage);

   struct anv_cmd_buffer *main_cmd_buffer = cmd_buffer;
   UNUSED struct anv_state rcs_done = ANV_STATE_NULL;

   bool blorp_execute_on_companion =
      anv_blorp_execute_on_companion(cmd_buffer, dst_image);

   /* Check if any one of the aspects is incompatible with the blitter engine,
    * if true, use the companion RCS command buffer for blit operation since 3
    * component formats are not supported natively except 96bpb on the blitter.
    */
   blorp_execute_on_companion |=
      anv_blorp_blitter_execute_on_companion(cmd_buffer, dst_image,
                                             pCopyBufferToImageInfo, NULL);

   if (blorp_execute_on_companion) {
      rcs_done = record_main_rcs_cmd_buffer_done(cmd_buffer);
      cmd_buffer = cmd_buffer->companion_rcs_cmd_buffer;
   }

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   for (unsigned r = 0; r < pCopyBufferToImageInfo->regionCount; r++) {
      copy_buffer_to_image(cmd_buffer, &batch, src_buffer, dst_image,
                           pCopyBufferToImageInfo->dstImageLayout,
                           &pCopyBufferToImageInfo->pRegions[r], true);
   }

   anv_blorp_batch_finish(&batch);

   if (dst_image->emu_plane_format != VK_FORMAT_UNDEFINED) {
      assert(!anv_cmd_buffer_is_blitter_queue(cmd_buffer));
      const enum anv_pipe_bits pipe_bits =
         anv_cmd_buffer_is_compute_queue(cmd_buffer) ?
         ANV_PIPE_HDC_PIPELINE_FLUSH_BIT :
         ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT;
      anv_add_pending_pipe_bits(cmd_buffer, pipe_bits,
                                "Copy flush before astc emu");

      for (unsigned r = 0; r < pCopyBufferToImageInfo->regionCount; r++) {
         const VkBufferImageCopy2 *region =
            &pCopyBufferToImageInfo->pRegions[r];
         const VkOffset3D block_offset = vk_image_offset_to_elements(
               &dst_image->vk, region->imageOffset);
         const VkExtent3D block_extent = vk_image_extent_to_elements(
               &dst_image->vk, region->imageExtent);
         anv_astc_emu_process(cmd_buffer, dst_image,
                              pCopyBufferToImageInfo->dstImageLayout,
                              &region->imageSubresource,
                              block_offset, block_extent);
      }
   }

   if (rcs_done.alloc_size)
      end_main_rcs_cmd_buffer_done(main_cmd_buffer, rcs_done);
}

static void
anv_add_buffer_write_pending_bits(struct anv_cmd_buffer *cmd_buffer,
                                  const char *reason)
{
   const struct intel_device_info *devinfo = cmd_buffer->device->info;

   cmd_buffer->state.queries.buffer_write_bits |=
      (cmd_buffer->queue_family->queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 ?
      ANV_QUERY_COMPUTE_WRITES_PENDING_BITS :
      ANV_QUERY_RENDER_TARGET_WRITES_PENDING_BITS(devinfo);
}

void anv_CmdCopyImageToBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_image, src_image, pCopyImageToBufferInfo->srcImage);
   ANV_FROM_HANDLE(anv_buffer, dst_buffer, pCopyImageToBufferInfo->dstBuffer);

   UNUSED struct anv_cmd_buffer *main_cmd_buffer = cmd_buffer;
   UNUSED struct anv_state rcs_done = ANV_STATE_NULL;

   bool blorp_execute_on_companion =
      anv_blorp_execute_on_companion(cmd_buffer, src_image);

   /* Check if any one of the aspects is incompatible with the blitter engine,
    * if true, use the companion RCS command buffer for blit operation since 3
    * component formats are not supported natively except 96bpb on the blitter.
    */
   blorp_execute_on_companion |=
      anv_blorp_blitter_execute_on_companion(cmd_buffer, src_image, NULL,
                                             pCopyImageToBufferInfo);

   if (blorp_execute_on_companion) {
      rcs_done = record_main_rcs_cmd_buffer_done(cmd_buffer);
      cmd_buffer = cmd_buffer->companion_rcs_cmd_buffer;
   }

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   for (unsigned r = 0; r < pCopyImageToBufferInfo->regionCount; r++) {
      copy_buffer_to_image(cmd_buffer, &batch, dst_buffer, src_image,
                           pCopyImageToBufferInfo->srcImageLayout,
                           &pCopyImageToBufferInfo->pRegions[r], false);
   }

   anv_add_buffer_write_pending_bits(cmd_buffer, "after copy image to buffer");

   anv_blorp_batch_finish(&batch);

   if (rcs_done.alloc_size)
      end_main_rcs_cmd_buffer_done(main_cmd_buffer, rcs_done);
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
blit_image(struct anv_cmd_buffer *cmd_buffer,
           struct blorp_batch *batch,
           struct anv_image *src_image,
           VkImageLayout src_image_layout,
           struct anv_image *dst_image,
           VkImageLayout dst_image_layout,
           const VkImageBlit2 *region,
           VkFilter filter)
{
   const VkImageSubresourceLayers *src_res = &region->srcSubresource;
   const VkImageSubresourceLayers *dst_res = &region->dstSubresource;

   struct blorp_surf src, dst;

   enum blorp_filter blorp_filter;
   switch (filter) {
   case VK_FILTER_NEAREST:
      blorp_filter = BLORP_FILTER_NEAREST;
      break;
   case VK_FILTER_LINEAR:
      blorp_filter = BLORP_FILTER_BILINEAR;
      break;
   default:
      unreachable("Invalid filter");
   }

   assert(anv_image_aspects_compatible(src_res->aspectMask,
                                       dst_res->aspectMask));

   anv_foreach_image_aspect_bit(aspect_bit, src_image, src_res->aspectMask) {
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   src_image, 1U << aspect_bit,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                   src_image_layout, ISL_AUX_USAGE_NONE, &src);
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   dst_image, 1U << aspect_bit,
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                   dst_image_layout, ISL_AUX_USAGE_NONE, &dst);

      VkFormat src_vk_format = src_image->vk.format;

      if (src_image->emu_plane_format != VK_FORMAT_UNDEFINED) {
         /* redirect src to the hidden plane */
         const uint32_t plane = src_image->n_planes;
         const struct anv_surface *surface =
            &src_image->planes[plane].primary_surface;
         const struct anv_address address =
            anv_image_address(src_image, &surface->memory_range);
         src.surf = &surface->isl,
         src.addr.offset = address.offset;

         src_vk_format = src_image->emu_plane_format;
      }

      struct anv_format_plane src_format =
         anv_get_format_aspect(cmd_buffer->device->info, src_vk_format,
                               1U << aspect_bit, src_image->vk.tiling);
      struct anv_format_plane dst_format =
         anv_get_format_aspect(cmd_buffer->device->info, dst_image->vk.format,
                               1U << aspect_bit, dst_image->vk.tiling);

      unsigned dst_start, dst_end;
      if (dst_image->vk.image_type == VK_IMAGE_TYPE_3D) {
         assert(dst_res->baseArrayLayer == 0);
         dst_start = region->dstOffsets[0].z;
         dst_end = region->dstOffsets[1].z;
      } else {
         dst_start = dst_res->baseArrayLayer;
         dst_end = dst_start +
            vk_image_subresource_layer_count(&dst_image->vk, dst_res);
      }

      unsigned src_start, src_end;
      if (src_image->vk.image_type == VK_IMAGE_TYPE_3D) {
         assert(src_res->baseArrayLayer == 0);
         src_start = region->srcOffsets[0].z;
         src_end = region->srcOffsets[1].z;
      } else {
         src_start = src_res->baseArrayLayer;
         src_end = src_start +
            vk_image_subresource_layer_count(&src_image->vk, src_res);
      }

      bool flip_z = flip_coords(&src_start, &src_end, &dst_start, &dst_end);
      const unsigned num_layers = dst_end - dst_start;
      float src_z_step = (float)(src_end - src_start) / (float)num_layers;

      /* There is no interpolation to the pixel center during rendering, so
       * add the 0.5 offset ourselves here. */
      float depth_center_offset = 0;
      if (src_image->vk.image_type == VK_IMAGE_TYPE_3D)
         depth_center_offset = 0.5 / num_layers * (src_end - src_start);

      if (flip_z) {
         src_start = src_end;
         src_z_step *= -1;
         depth_center_offset *= -1;
      }

      unsigned src_x0 = region->srcOffsets[0].x;
      unsigned src_x1 = region->srcOffsets[1].x;
      unsigned dst_x0 = region->dstOffsets[0].x;
      unsigned dst_x1 = region->dstOffsets[1].x;
      bool flip_x = flip_coords(&src_x0, &src_x1, &dst_x0, &dst_x1);

      unsigned src_y0 = region->srcOffsets[0].y;
      unsigned src_y1 = region->srcOffsets[1].y;
      unsigned dst_y0 = region->dstOffsets[0].y;
      unsigned dst_y1 = region->dstOffsets[1].y;
      bool flip_y = flip_coords(&src_y0, &src_y1, &dst_y0, &dst_y1);

      anv_cmd_buffer_mark_image_written(cmd_buffer, dst_image,
                                        1U << aspect_bit,
                                        dst.aux_usage,
                                        dst_res->mipLevel,
                                        dst_start, num_layers);

      for (unsigned i = 0; i < num_layers; i++) {
         unsigned dst_z = dst_start + i;
         float src_z = src_start + i * src_z_step + depth_center_offset;

         blorp_blit(batch, &src, src_res->mipLevel, src_z,
                    src_format.isl_format, src_format.swizzle,
                    &dst, dst_res->mipLevel, dst_z,
                    dst_format.isl_format, dst_format.swizzle,
                    src_x0, src_y0, src_x1, src_y1,
                    dst_x0, dst_y0, dst_x1, dst_y1,
                    blorp_filter, flip_x, flip_y);
      }
   }
}

void anv_CmdBlitImage2(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_image, src_image, pBlitImageInfo->srcImage);
   ANV_FROM_HANDLE(anv_image, dst_image, pBlitImageInfo->dstImage);

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   for (unsigned r = 0; r < pBlitImageInfo->regionCount; r++) {
      blit_image(cmd_buffer, &batch,
                 src_image, pBlitImageInfo->srcImageLayout,
                 dst_image, pBlitImageInfo->dstImageLayout,
                 &pBlitImageInfo->pRegions[r], pBlitImageInfo->filter);
   }

   anv_blorp_batch_finish(&batch);
}

/**
 * Returns the greatest common divisor of a and b that is a power of two.
 */
static uint64_t
gcd_pow2_u64(uint64_t a, uint64_t b)
{
   assert(a > 0 || b > 0);

   unsigned a_log2 = ffsll(a) - 1;
   unsigned b_log2 = ffsll(b) - 1;

   /* If either a or b is 0, then a_log2 or b_log2 till be UINT_MAX in which
    * case, the MIN2() will take the other one.  If both are 0 then we will
    * hit the assert above.
    */
   return 1 << MIN2(a_log2, b_log2);
}

/* This is maximum possible width/height our HW can handle */
#define MAX_SURFACE_DIM (1ull << 14)

static void
copy_buffer(struct anv_device *device,
            struct blorp_batch *batch,
            struct anv_buffer *src_buffer,
            struct anv_buffer *dst_buffer,
            const VkBufferCopy2 *region)
{
   struct blorp_address src = {
      .buffer = src_buffer->address.bo,
      .offset = src_buffer->address.offset + region->srcOffset,
      .mocs = anv_mocs(device, src_buffer->address.bo,
                       blorp_batch_isl_copy_usage(batch, false /* is_dest */)),
   };
   struct blorp_address dst = {
      .buffer = dst_buffer->address.bo,
      .offset = dst_buffer->address.offset + region->dstOffset,
      .mocs = anv_mocs(device, dst_buffer->address.bo,
                       blorp_batch_isl_copy_usage(batch, true /* is_dest */)),
   };

   blorp_buffer_copy(batch, src, dst, region->size);
}

void anv_CmdCopyBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, src_buffer, pCopyBufferInfo->srcBuffer);
   ANV_FROM_HANDLE(anv_buffer, dst_buffer, pCopyBufferInfo->dstBuffer);

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   for (unsigned r = 0; r < pCopyBufferInfo->regionCount; r++) {
      copy_buffer(cmd_buffer->device, &batch, src_buffer, dst_buffer,
                  &pCopyBufferInfo->pRegions[r]);
   }

   anv_add_buffer_write_pending_bits(cmd_buffer, "after copy buffer");

   anv_blorp_batch_finish(&batch);
}


void anv_CmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, dst_buffer, dstBuffer);

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   /* We can't quite grab a full block because the state stream needs a
    * little data at the top to build its linked list.
    */
   const uint32_t max_update_size =
      cmd_buffer->device->dynamic_state_pool.block_size - 64;

   assert(max_update_size < MAX_SURFACE_DIM * 4);

   /* We're about to read data that was written from the CPU.  Flush the
    * texture cache so we don't get anything stale.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT,
                             "before UpdateBuffer");

   while (dataSize) {
      const uint32_t copy_size = MIN2(dataSize, max_update_size);

      struct anv_state tmp_data =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, copy_size, 64);

      memcpy(tmp_data.map, pData, copy_size);

      struct blorp_address src = {
         .buffer = cmd_buffer->device->dynamic_state_pool.block_pool.bo,
         .offset = tmp_data.offset,
         .mocs = anv_mocs(cmd_buffer->device, NULL,
                          get_usage_flag_for_cmd_buffer(cmd_buffer,
                                                        false /* is_dest */))
      };
      struct blorp_address dst = {
         .buffer = dst_buffer->address.bo,
         .offset = dst_buffer->address.offset + dstOffset,
         .mocs = anv_mocs(cmd_buffer->device, dst_buffer->address.bo,
                          get_usage_flag_for_cmd_buffer(cmd_buffer,
                                                        true /* is_dest */)),
      };

      blorp_buffer_copy(&batch, src, dst, copy_size);

      dataSize -= copy_size;
      dstOffset += copy_size;
      pData = (void *)pData + copy_size;
   }

   anv_add_buffer_write_pending_bits(cmd_buffer, "update buffer");

   anv_blorp_batch_finish(&batch);
}

void
anv_cmd_buffer_fill_area(struct anv_cmd_buffer *cmd_buffer,
                         struct anv_address address,
                         VkDeviceSize size,
                         uint32_t data)
{
   struct blorp_surf surf;
   struct isl_surf isl_surf;

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   /* First, we compute the biggest format that can be used with the
    * given offsets and size.
    */
   int bs = 16;
   uint64_t offset = address.offset;
   bs = gcd_pow2_u64(bs, offset);
   bs = gcd_pow2_u64(bs, size);
   enum isl_format isl_format = isl_format_for_size(bs);

   union isl_color_value color = {
      .u32 = { data, data, data, data },
   };

   const uint64_t max_fill_size = MAX_SURFACE_DIM * MAX_SURFACE_DIM * bs;
   while (size >= max_fill_size) {
      get_blorp_surf_for_anv_address(cmd_buffer,
                                     (struct anv_address) {
                                        .bo = address.bo, .offset = offset,
                                     },
                                     MAX_SURFACE_DIM, MAX_SURFACE_DIM,
                                     MAX_SURFACE_DIM * bs, isl_format,
                                     true /* is_dest */,
                                     &surf, &isl_surf);

      blorp_clear(&batch, &surf, isl_format, ISL_SWIZZLE_IDENTITY,
                  0, 0, 1, 0, 0, MAX_SURFACE_DIM, MAX_SURFACE_DIM,
                  color, 0 /* color_write_disable */);
      size -= max_fill_size;
      offset += max_fill_size;
   }

   uint64_t height = size / (MAX_SURFACE_DIM * bs);
   assert(height < MAX_SURFACE_DIM);
   if (height != 0) {
      const uint64_t rect_fill_size = height * MAX_SURFACE_DIM * bs;
      get_blorp_surf_for_anv_address(cmd_buffer,
                                     (struct anv_address) {
                                        .bo = address.bo, .offset = offset,
                                     },
                                     MAX_SURFACE_DIM, height,
                                     MAX_SURFACE_DIM * bs, isl_format,
                                     true /* is_dest */,
                                     &surf, &isl_surf);

      blorp_clear(&batch, &surf, isl_format, ISL_SWIZZLE_IDENTITY,
                  0, 0, 1, 0, 0, MAX_SURFACE_DIM, height,
                  color, 0 /* color_write_disable */);
      size -= rect_fill_size;
      offset += rect_fill_size;
   }

   if (size != 0) {
      const uint32_t width = size / bs;
      get_blorp_surf_for_anv_address(cmd_buffer,
                                     (struct anv_address) {
                                        .bo = address.bo, .offset = offset,
                                     },
                                     width, 1,
                                     width * bs, isl_format,
                                     true /* is_dest */,
                                     &surf, &isl_surf);

      blorp_clear(&batch, &surf, isl_format, ISL_SWIZZLE_IDENTITY,
                  0, 0, 1, 0, 0, width, 1,
                  color, 0 /* color_write_disable */);
   }

   anv_blorp_batch_finish(&batch);
}

void anv_CmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                fillSize,
    uint32_t                                    data)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_buffer, dst_buffer, dstBuffer);

   fillSize = vk_buffer_range(&dst_buffer->vk, dstOffset, fillSize);

   /* From the Vulkan spec:
    *
    *    "size is the number of bytes to fill, and must be either a multiple
    *    of 4, or VK_WHOLE_SIZE to fill the range from offset to the end of
    *    the buffer. If VK_WHOLE_SIZE is used and the remaining size of the
    *    buffer is not a multiple of 4, then the nearest smaller multiple is
    *    used."
    */
   fillSize &= ~3ull;

   anv_cmd_buffer_fill_area(cmd_buffer,
                            anv_address_add(dst_buffer->address, dstOffset),
                            fillSize, data);

   anv_add_buffer_write_pending_bits(cmd_buffer, "after fill buffer");
}

void anv_CmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     _image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_image, image, _image);

   struct anv_cmd_buffer *main_cmd_buffer = cmd_buffer;
   UNUSED struct anv_state rcs_done = ANV_STATE_NULL;

   if (anv_blorp_execute_on_companion(cmd_buffer, image)) {
      rcs_done = record_main_rcs_cmd_buffer_done(cmd_buffer);
      cmd_buffer = cmd_buffer->companion_rcs_cmd_buffer;
   }

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   for (unsigned r = 0; r < rangeCount; r++) {
      if (pRanges[r].aspectMask == 0)
         continue;

      assert(pRanges[r].aspectMask & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV);

      struct blorp_surf surf;
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   image, pRanges[r].aspectMask,
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                   imageLayout, ISL_AUX_USAGE_NONE, &surf);

      struct anv_format_plane src_format =
         anv_get_format_aspect(cmd_buffer->device->info, image->vk.format,
                               VK_IMAGE_ASPECT_COLOR_BIT, image->vk.tiling);

      unsigned base_layer = pRanges[r].baseArrayLayer;
      uint32_t layer_count =
         vk_image_subresource_layer_count(&image->vk, &pRanges[r]);
      uint32_t level_count =
         vk_image_subresource_level_count(&image->vk, &pRanges[r]);

      for (uint32_t i = 0; i < level_count; i++) {
         const unsigned level = pRanges[r].baseMipLevel + i;
         const unsigned level_width = u_minify(image->vk.extent.width, level);
         const unsigned level_height = u_minify(image->vk.extent.height, level);

         if (image->vk.image_type == VK_IMAGE_TYPE_3D) {
            base_layer = 0;
            layer_count = u_minify(image->vk.extent.depth, level);
         }

         anv_cmd_buffer_mark_image_written(cmd_buffer, image,
                                           pRanges[r].aspectMask,
                                           surf.aux_usage, level,
                                           base_layer, layer_count);

         blorp_clear(&batch, &surf,
                     src_format.isl_format, src_format.swizzle,
                     level, base_layer, layer_count,
                     0, 0, level_width, level_height,
                     vk_to_isl_color(*pColor), 0 /* color_write_disable */);
      }
   }

   anv_blorp_batch_finish(&batch);

   if (rcs_done.alloc_size)
      end_main_rcs_cmd_buffer_done(main_cmd_buffer, rcs_done);
}

void anv_CmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image_h,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_image, image, image_h);

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);
   assert((batch.flags & BLORP_BATCH_USE_COMPUTE) == 0);

   struct blorp_surf depth, stencil;
   if (image->vk.aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   image, VK_IMAGE_ASPECT_DEPTH_BIT,
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                   imageLayout, ISL_AUX_USAGE_NONE, &depth);
   } else {
      memset(&depth, 0, sizeof(depth));
   }

   if (image->vk.aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   image, VK_IMAGE_ASPECT_STENCIL_BIT,
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                   imageLayout, ISL_AUX_USAGE_NONE, &stencil);
   } else {
      memset(&stencil, 0, sizeof(stencil));
   }

   for (unsigned r = 0; r < rangeCount; r++) {
      if (pRanges[r].aspectMask == 0)
         continue;

      bool clear_depth = pRanges[r].aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT;
      bool clear_stencil = pRanges[r].aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT;

      unsigned base_layer = pRanges[r].baseArrayLayer;
      uint32_t layer_count =
         vk_image_subresource_layer_count(&image->vk, &pRanges[r]);
      uint32_t level_count =
         vk_image_subresource_level_count(&image->vk, &pRanges[r]);

      for (uint32_t i = 0; i < level_count; i++) {
         const unsigned level = pRanges[r].baseMipLevel + i;
         const unsigned level_width = u_minify(image->vk.extent.width, level);
         const unsigned level_height = u_minify(image->vk.extent.height, level);

         if (image->vk.image_type == VK_IMAGE_TYPE_3D)
            layer_count = u_minify(image->vk.extent.depth, level);

         blorp_clear_depth_stencil(&batch, &depth, &stencil,
                                   level, base_layer, layer_count,
                                   0, 0, level_width, level_height,
                                   clear_depth, pDepthStencil->depth,
                                   clear_stencil ? 0xff : 0,
                                   pDepthStencil->stencil);
      }
   }

   anv_blorp_batch_finish(&batch);
}

VkResult
anv_cmd_buffer_alloc_blorp_binding_table(struct anv_cmd_buffer *cmd_buffer,
                                         uint32_t num_entries,
                                         uint32_t *state_offset,
                                         struct anv_state *bt_state)
{
   *bt_state = anv_cmd_buffer_alloc_binding_table(cmd_buffer, num_entries,
                                                  state_offset);
   if (bt_state->map == NULL) {
      /* We ran out of space.  Grab a new binding table block. */
      VkResult result = anv_cmd_buffer_new_binding_table_block(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;

      /* Re-emit state base addresses so we get the new surface state base
       * address before we start emitting binding tables etc.
       */
      anv_cmd_buffer_emit_state_base_address(cmd_buffer);

      *bt_state = anv_cmd_buffer_alloc_binding_table(cmd_buffer, num_entries,
                                                     state_offset);
      assert(bt_state->map != NULL);
   }

   return VK_SUCCESS;
}

static VkResult
binding_table_for_surface_state(struct anv_cmd_buffer *cmd_buffer,
                                struct anv_state surface_state,
                                uint32_t *bt_offset)
{
   uint32_t state_offset;
   struct anv_state bt_state;

   VkResult result =
      anv_cmd_buffer_alloc_blorp_binding_table(cmd_buffer, 1, &state_offset,
                                               &bt_state);
   if (result != VK_SUCCESS)
      return result;

   uint32_t *bt_map = bt_state.map;
   bt_map[0] = surface_state.offset + state_offset;

   *bt_offset = bt_state.offset;
   return VK_SUCCESS;
}

static bool
can_fast_clear_color_att(struct anv_cmd_buffer *cmd_buffer,
                         struct blorp_batch *batch,
                         const struct anv_attachment *att,
                         const VkClearAttachment *attachment,
                         uint32_t rectCount, const VkClearRect *pRects)
{
   union isl_color_value clear_color =
      vk_to_isl_color(attachment->clearValue.color);

   if (INTEL_DEBUG(DEBUG_NO_FAST_CLEAR))
      return false;

   /* We don't support fast clearing with conditional rendering at the
    * moment. All the tracking done around fast clears (clear color updates
    * and fast-clear type updates) happens unconditionally.
    */
   if (batch->flags & BLORP_BATCH_PREDICATE_ENABLE)
      return false;

   if (rectCount > 1) {
      anv_perf_warn(VK_LOG_OBJS(&cmd_buffer->device->vk.base),
                    "Fast clears for vkCmdClearAttachments supported only for rectCount == 1");
      return false;
   }

   /* We only support fast-clears on the first layer */
   if (pRects[0].layerCount > 1 || pRects[0].baseArrayLayer > 0)
      return false;

   bool is_multiview = cmd_buffer->state.gfx.view_mask != 0;
   if (is_multiview && (cmd_buffer->state.gfx.view_mask != 1))
      return false;

   return anv_can_fast_clear_color_view(cmd_buffer->device,
                                        (struct anv_image_view *)att->iview,
                                        att->layout,
                                        clear_color,
                                        pRects->layerCount,
                                        pRects->rect,
                                        cmd_buffer->queue_family->queueFlags);
}

static void
exec_ccs_op(struct anv_cmd_buffer *cmd_buffer,
            struct blorp_batch *batch,
            const struct anv_image *image,
            enum isl_format format, struct isl_swizzle swizzle,
            VkImageAspectFlagBits aspect, uint32_t level,
            uint32_t base_layer, uint32_t layer_count,
            enum isl_aux_op ccs_op, union isl_color_value *clear_value)
{
   assert(image->vk.aspects & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV);
   assert(image->vk.samples == 1);
   assert(level < anv_image_aux_levels(image, aspect));
   /* Multi-LOD YcBcR is not allowed */
   assert(image->n_planes == 1 || level == 0);
   assert(base_layer + layer_count <=
          anv_image_aux_layers(image, aspect, level));

   const uint32_t plane = anv_image_aspect_to_plane(image, aspect);
   const struct intel_device_info *devinfo = cmd_buffer->device->info;

   struct blorp_surf surf;
   get_blorp_surf_for_anv_image(cmd_buffer, image, aspect,
                                0, ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                image->planes[plane].aux_usage,
                                &surf);

   uint32_t level_width = u_minify(surf.surf->logical_level0_px.w, level);
   uint32_t level_height = u_minify(surf.surf->logical_level0_px.h, level);

   /* Blorp will store the clear color for us if we provide the clear color
    * address and we are doing a fast clear. So we save the clear value into
    * the blorp surface.
    */
   if (clear_value)
      surf.clear_color = *clear_value;

   char flush_reason[64];
   int ret =
      snprintf(flush_reason, sizeof(flush_reason),
               "ccs op start: %s", isl_aux_op_to_name(ccs_op));
   assert(ret < sizeof(flush_reason));

   /* From the Sky Lake PRM Vol. 7, "Render Target Fast Clear":
    *
    *    "After Render target fast clear, pipe-control with color cache
    *    write-flush must be issued before sending any DRAW commands on
    *    that render target."
    *
    * This comment is a bit cryptic and doesn't really tell you what's going
    * or what's really needed.  It appears that fast clear ops are not
    * properly synchronized with other drawing.  This means that we cannot
    * have a fast clear operation in the pipe at the same time as other
    * regular drawing operations.  We need to use a PIPE_CONTROL to ensure
    * that the contents of the previous draw hit the render target before we
    * resolve and then use a second PIPE_CONTROL after the resolve to ensure
    * that it is completed before any additional drawing occurs.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                             ANV_PIPE_TILE_CACHE_FLUSH_BIT |
                             (devinfo->verx10 == 120 ?
                                ANV_PIPE_DEPTH_STALL_BIT : 0) |
                             (devinfo->verx10 == 125 ?
                                ANV_PIPE_HDC_PIPELINE_FLUSH_BIT |
                                ANV_PIPE_DATA_CACHE_FLUSH_BIT : 0) |
                             ANV_PIPE_PSS_STALL_SYNC_BIT |
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             flush_reason);

   switch (ccs_op) {
   case ISL_AUX_OP_FAST_CLEAR:
      /* From the ICL PRMs, Volume 9: Render Engine, State Caching :
       *
       *    "Any values referenced by pointers within the RENDER_SURFACE_STATE
       *     or SAMPLER_STATE (e.g. Clear Color Pointer, Border Color or
       *     Indirect State Pointer) are considered to be part of that state
       *     and any changes to these referenced values requires an
       *     invalidation of the L1 state cache to ensure the new values are
       *     being used as part of the state. In the case of surface data
       *     pointed to by the Surface Base Address in RENDER SURFACE STATE,
       *     the Texture Cache must be invalidated if the surface data
       *     changes."
       *
       * and From the Render Target Fast Clear section,
       *
       *   "HwManaged FastClear allows SW to store FastClearValue in separate
       *   graphics allocation, instead of keeping them in
       *   RENDER_SURFACE_STATE. This behavior can be enabled by setting
       *   ClearValueAddressEnable in RENDER_SURFACE_STATE.
       *
       *    Proper sequence of commands is as follows:
       *
       *       1. Storing clear color to allocation
       *       2. Ensuring that step 1. is finished and visible for TextureCache
       *       3. Performing FastClear
       *
       *    Step 2. is required on products with ClearColorConversion feature.
       *    This feature is enabled by setting ClearColorConversionEnable.
       *    This causes HW to read stored color from ClearColorAllocation and
       *    write back with the native format or RenderTarget - and clear
       *    color needs to be present and visible. Reading is done from
       *    TextureCache, writing is done to RenderCache."
       *
       * We're going to change the clear color. Invalidate the texture cache
       * now to ensure the clear color conversion feature works properly.
       * Although the docs seem to require invalidating the texture cache
       * after updating the clear color allocation, we can do this beforehand
       * so long as we ensure:
       *
       *    1. Step 1 is complete before the texture cache is accessed in step 3
       *    2. We don't access the texture cache between invalidation and step 3
       *
       * The second requirement is satisfied because we'll be performing step
       * 1 and 3 right after invalidating. The first is satisfied because
       * BLORP updates the clear color before performing the fast clear and it
       * performs the synchronizations suggested by the Render Target Fast
       * Clear section (not quoted here) to ensure its completion.
       *
       * While we're here, also invalidate the state cache as suggested.
       */
      if (devinfo->ver >= 11) {
         anv_add_pending_pipe_bits(cmd_buffer,
                                   ANV_PIPE_STATE_CACHE_INVALIDATE_BIT |
                                   ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT,
                                   "before blorp clear color update");
      }

      blorp_fast_clear(batch, &surf, format, swizzle,
                       level, base_layer, layer_count,
                       0, 0, level_width, level_height);
      break;
   case ISL_AUX_OP_FULL_RESOLVE:
   case ISL_AUX_OP_PARTIAL_RESOLVE: {
      /* Wa_1508744258: Enable RHWO optimization for resolves */
      const bool enable_rhwo_opt =
         intel_needs_workaround(cmd_buffer->device->info, 1508744258);

      if (enable_rhwo_opt)
         cmd_buffer->state.pending_rhwo_optimization_enabled = true;

      blorp_ccs_resolve(batch, &surf, level, base_layer, layer_count,
                        format, ccs_op);

      if (enable_rhwo_opt)
         cmd_buffer->state.pending_rhwo_optimization_enabled = false;
      break;
   }
   case ISL_AUX_OP_AMBIGUATE:
      for (uint32_t a = 0; a < layer_count; a++) {
         const uint32_t layer = base_layer + a;
         blorp_ccs_ambiguate(batch, &surf, level, layer);
      }
      break;
   default:
      unreachable("Unsupported CCS operation");
   }

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                             (devinfo->verx10 == 120 ?
                                ANV_PIPE_TILE_CACHE_FLUSH_BIT |
                                ANV_PIPE_DEPTH_STALL_BIT : 0) |
                             ANV_PIPE_PSS_STALL_SYNC_BIT |
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "ccs op finish");
}

static void
exec_mcs_op(struct anv_cmd_buffer *cmd_buffer,
            struct blorp_batch *batch,
            const struct anv_image *image,
            enum isl_format format, struct isl_swizzle swizzle,
            VkImageAspectFlagBits aspect,
            uint32_t base_layer, uint32_t layer_count,
            enum isl_aux_op mcs_op, union isl_color_value *clear_value)
{
   assert(image->vk.aspects == VK_IMAGE_ASPECT_COLOR_BIT);
   assert(image->vk.samples > 1);
   assert(base_layer + layer_count <= anv_image_aux_layers(image, aspect, 0));

   /* Multisampling with multi-planar formats is not supported */
   assert(image->n_planes == 1);

   const struct intel_device_info *devinfo = cmd_buffer->device->info;
   struct blorp_surf surf;
   get_blorp_surf_for_anv_image(cmd_buffer, image, aspect,
                                0, ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                ISL_AUX_USAGE_MCS, &surf);

   /* Blorp will store the clear color for us if we provide the clear color
    * address and we are doing a fast clear. So we save the clear value into
    * the blorp surface.
    */
   if (clear_value)
      surf.clear_color = *clear_value;

   /* From the Sky Lake PRM Vol. 7, "Render Target Fast Clear":
    *
    *    "After Render target fast clear, pipe-control with color cache
    *    write-flush must be issued before sending any DRAW commands on
    *    that render target."
    *
    * This comment is a bit cryptic and doesn't really tell you what's going
    * or what's really needed.  It appears that fast clear ops are not
    * properly synchronized with other drawing.  This means that we cannot
    * have a fast clear operation in the pipe at the same time as other
    * regular drawing operations.  We need to use a PIPE_CONTROL to ensure
    * that the contents of the previous draw hit the render target before we
    * resolve and then use a second PIPE_CONTROL after the resolve to ensure
    * that it is completed before any additional drawing occurs.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                             ANV_PIPE_TILE_CACHE_FLUSH_BIT |
                             (devinfo->verx10 == 120 ?
                                ANV_PIPE_DEPTH_STALL_BIT : 0) |
                             (devinfo->verx10 == 125 ?
                                ANV_PIPE_HDC_PIPELINE_FLUSH_BIT |
                                ANV_PIPE_DATA_CACHE_FLUSH_BIT : 0) |
                             ANV_PIPE_PSS_STALL_SYNC_BIT |
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "before fast clear mcs");

   switch (mcs_op) {
   case ISL_AUX_OP_FAST_CLEAR:
      /* From the ICL PRMs, Volume 9: Render Engine, State Caching :
       *
       *    "Any values referenced by pointers within the RENDER_SURFACE_STATE
       *     or SAMPLER_STATE (e.g. Clear Color Pointer, Border Color or
       *     Indirect State Pointer) are considered to be part of that state
       *     and any changes to these referenced values requires an
       *     invalidation of the L1 state cache to ensure the new values are
       *     being used as part of the state. In the case of surface data
       *     pointed to by the Surface Base Address in RENDER SURFACE STATE,
       *     the Texture Cache must be invalidated if the surface data
       *     changes."
       *
       * and From the Render Target Fast Clear section,
       *
       *   "HwManaged FastClear allows SW to store FastClearValue in separate
       *   graphics allocation, instead of keeping them in
       *   RENDER_SURFACE_STATE. This behavior can be enabled by setting
       *   ClearValueAddressEnable in RENDER_SURFACE_STATE.
       *
       *    Proper sequence of commands is as follows:
       *
       *       1. Storing clear color to allocation
       *       2. Ensuring that step 1. is finished and visible for TextureCache
       *       3. Performing FastClear
       *
       *    Step 2. is required on products with ClearColorConversion feature.
       *    This feature is enabled by setting ClearColorConversionEnable.
       *    This causes HW to read stored color from ClearColorAllocation and
       *    write back with the native format or RenderTarget - and clear
       *    color needs to be present and visible. Reading is done from
       *    TextureCache, writing is done to RenderCache."
       *
       * We're going to change the clear color. Invalidate the texture cache
       * now to ensure the clear color conversion feature works properly.
       * Although the docs seem to require invalidating the texture cache
       * after updating the clear color allocation, we can do this beforehand
       * so long as we ensure:
       *
       *    1. Step 1 is complete before the texture cache is accessed in step 3
       *    2. We don't access the texture cache between invalidation and step 3
       *
       * The second requirement is satisfied because we'll be performing step
       * 1 and 3 right after invalidating. The first is satisfied because
       * BLORP updates the clear color before performing the fast clear and it
       * performs the synchronizations suggested by the Render Target Fast
       * Clear section (not quoted here) to ensure its completion.
       *
       * While we're here, also invalidate the state cache as suggested.
       */
      if (devinfo->ver >= 11) {
         anv_add_pending_pipe_bits(cmd_buffer,
                                   ANV_PIPE_STATE_CACHE_INVALIDATE_BIT |
                                   ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT,
                                   "before blorp clear color update");
      }

      blorp_fast_clear(batch, &surf, format, swizzle,
                       0, base_layer, layer_count,
                       0, 0, image->vk.extent.width, image->vk.extent.height);
      break;
   case ISL_AUX_OP_PARTIAL_RESOLVE:
      blorp_mcs_partial_resolve(batch, &surf, format,
                                base_layer, layer_count);
      break;
   case ISL_AUX_OP_AMBIGUATE:
      blorp_mcs_ambiguate(batch, &surf, base_layer, layer_count);
      break;
   case ISL_AUX_OP_FULL_RESOLVE:
   default:
      unreachable("Unsupported MCS operation");
   }

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                             (devinfo->verx10 == 120 ?
                                ANV_PIPE_TILE_CACHE_FLUSH_BIT |
                                ANV_PIPE_DEPTH_STALL_BIT : 0) |
                             ANV_PIPE_PSS_STALL_SYNC_BIT |
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "after fast clear mcs");
}

static void
clear_color_attachment(struct anv_cmd_buffer *cmd_buffer,
                       struct blorp_batch *batch,
                       const VkClearAttachment *attachment,
                       uint32_t rectCount, const VkClearRect *pRects)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const uint32_t att_idx = attachment->colorAttachment;
   assert(att_idx < gfx->color_att_count);
   const struct anv_attachment *att = &gfx->color_att[att_idx];

   if (att->vk_format == VK_FORMAT_UNDEFINED)
      return;

   union isl_color_value clear_color =
      vk_to_isl_color(attachment->clearValue.color);

   const struct anv_image_view *iview = att->iview;
   if (iview &&
       can_fast_clear_color_att(cmd_buffer, batch, att,
                                attachment, rectCount, pRects)) {
      if (iview->image->vk.samples == 1) {
         exec_ccs_op(cmd_buffer, batch, iview->image,
                     iview->planes[0].isl.format,
                     iview->planes[0].isl.swizzle,
                     VK_IMAGE_ASPECT_COLOR_BIT,
                     0, 0, 1, ISL_AUX_OP_FAST_CLEAR,
                     &clear_color);
      } else {
         exec_mcs_op(cmd_buffer, batch, iview->image,
                     iview->planes[0].isl.format,
                     iview->planes[0].isl.swizzle,
                     VK_IMAGE_ASPECT_COLOR_BIT,
                     0, 1, ISL_AUX_OP_FAST_CLEAR,
                     &clear_color);
      }

      anv_cmd_buffer_mark_image_fast_cleared(cmd_buffer, iview->image,
                                             iview->planes[0].isl.format,
                                             clear_color);
      anv_cmd_buffer_load_clear_color_from_image(cmd_buffer,
                                                 att->surface_state.state,
                                                 iview->image);
      return;
   }

   uint32_t binding_table;
   VkResult result =
      binding_table_for_surface_state(cmd_buffer, att->surface_state.state,
                                      &binding_table);
   if (result != VK_SUCCESS)
      return;

   /* If multiview is enabled we ignore baseArrayLayer and layerCount */
   if (gfx->view_mask) {
      u_foreach_bit(view_idx, gfx->view_mask) {
         for (uint32_t r = 0; r < rectCount; ++r) {
            const VkOffset2D offset = pRects[r].rect.offset;
            const VkExtent2D extent = pRects[r].rect.extent;
            blorp_clear_attachments(batch, binding_table,
                                    ISL_FORMAT_UNSUPPORTED,
                                    gfx->samples,
                                    view_idx, 1,
                                    offset.x, offset.y,
                                    offset.x + extent.width,
                                    offset.y + extent.height,
                                    true, clear_color, false, 0.0f, 0, 0);
         }
      }
      return;
   }

   for (uint32_t r = 0; r < rectCount; ++r) {
      const VkOffset2D offset = pRects[r].rect.offset;
      const VkExtent2D extent = pRects[r].rect.extent;
      assert(pRects[r].layerCount != VK_REMAINING_ARRAY_LAYERS);
      blorp_clear_attachments(batch, binding_table,
                              ISL_FORMAT_UNSUPPORTED,
                              gfx->samples,
                              pRects[r].baseArrayLayer,
                              pRects[r].layerCount,
                              offset.x, offset.y,
                              offset.x + extent.width, offset.y + extent.height,
                              true, clear_color, false, 0.0f, 0, 0);
   }
}

static void
anv_fast_clear_depth_stencil(struct anv_cmd_buffer *cmd_buffer,
                             struct blorp_batch *batch,
                             const struct anv_image *image,
                             VkImageAspectFlags aspects,
                             uint32_t level,
                             uint32_t base_layer, uint32_t layer_count,
                             VkRect2D area, uint8_t stencil_value)
{
   assert(image->vk.aspects & (VK_IMAGE_ASPECT_DEPTH_BIT |
                               VK_IMAGE_ASPECT_STENCIL_BIT));

   struct blorp_surf depth = {};
   if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
      const uint32_t plane =
         anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_DEPTH_BIT);
      assert(base_layer + layer_count <=
             anv_image_aux_layers(image, VK_IMAGE_ASPECT_DEPTH_BIT, level));
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   image, VK_IMAGE_ASPECT_DEPTH_BIT,
                                   0, ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                   image->planes[plane].aux_usage, &depth);
   }

   struct blorp_surf stencil = {};
   if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      const uint32_t plane =
         anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_STENCIL_BIT);
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   image, VK_IMAGE_ASPECT_STENCIL_BIT,
                                   0, ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                   image->planes[plane].aux_usage, &stencil);
   }

   /* From the Sky Lake PRM Volume 7, "Depth Buffer Clear":
    *
    *    "The following is required when performing a depth buffer clear with
    *    using the WM_STATE or 3DSTATE_WM:
    *
    *       * If other rendering operations have preceded this clear, a
    *         PIPE_CONTROL with depth cache flush enabled, Depth Stall bit
    *         enabled must be issued before the rectangle primitive used for
    *         the depth buffer clear operation.
    *       * [...]"
    *
    * Even though the PRM only says that this is required if using 3DSTATE_WM
    * and a 3DPRIMITIVE, the GPU appears to also need this to avoid occasional
    * hangs when doing a clear with WM_HZ_OP.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_DEPTH_CACHE_FLUSH_BIT |
                             ANV_PIPE_DEPTH_STALL_BIT,
                             "before clear hiz");

   if ((aspects & VK_IMAGE_ASPECT_DEPTH_BIT) &&
       depth.aux_usage == ISL_AUX_USAGE_HIZ_CCS_WT) {
      /* From Bspec 47010 (Depth Buffer Clear):
       *
       *    Since the fast clear cycles to CCS are not cached in TileCache,
       *    any previous depth buffer writes to overlapping pixels must be
       *    flushed out of TileCache before a succeeding Depth Buffer Clear.
       *    This restriction only applies to Depth Buffer with write-thru
       *    enabled, since fast clears to CCS only occur for write-thru mode.
       *
       * There may have been a write to this depth buffer. Flush it from the
       * tile cache just in case.
       *
       * Set CS stall bit to guarantee that the fast clear starts the execution
       * after the tile cache flush completed.
       *
       * There is no Bspec requirement to flush the data cache but the
       * experiment shows that flusing the data cache helps to resolve the
       * corruption.
       */
      unsigned wa_flush = cmd_buffer->device->info->verx10 >= 125 ?
                          ANV_PIPE_DATA_CACHE_FLUSH_BIT : 0;
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_DEPTH_CACHE_FLUSH_BIT |
                                ANV_PIPE_CS_STALL_BIT |
                                ANV_PIPE_TILE_CACHE_FLUSH_BIT |
                                wa_flush,
                                "before clear hiz_ccs_wt");
   }

   blorp_hiz_clear_depth_stencil(batch, &depth, &stencil,
                                 level, base_layer, layer_count,
                                 area.offset.x, area.offset.y,
                                 area.offset.x + area.extent.width,
                                 area.offset.y + area.extent.height,
                                 aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                                 ANV_HZ_FC_VAL,
                                 aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
                                 stencil_value);

   /* From the SKL PRM, Depth Buffer Clear:
    *
    *    "Depth Buffer Clear Workaround
    *
    *    Depth buffer clear pass using any of the methods (WM_STATE,
    *    3DSTATE_WM or 3DSTATE_WM_HZ_OP) must be followed by a PIPE_CONTROL
    *    command with DEPTH_STALL bit and Depth FLUSH bits “set” before
    *    starting to render.  DepthStall and DepthFlush are not needed between
    *    consecutive depth clear passes nor is it required if the depth-clear
    *    pass was done with “full_surf_clear” bit set in the
    *    3DSTATE_WM_HZ_OP."
    *
    * Even though the PRM provides a bunch of conditions under which this is
    * supposedly unnecessary, we choose to perform the flush unconditionally
    * just to be safe.
    *
    * From Bspec 46959, a programming note applicable to Gfx12+:
    *
    *    "Since HZ_OP has to be sent twice (first time set the clear/resolve state
    *    and 2nd time to clear the state), and HW internally flushes the depth
    *    cache on HZ_OP, there is no need to explicitly send a Depth Cache flush
    *    after Clear or Resolve."
    */
   if (cmd_buffer->device->info->verx10 < 120) {
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_DEPTH_CACHE_FLUSH_BIT |
                                ANV_PIPE_DEPTH_STALL_BIT,
                                "after clear hiz");
   }
}

static bool
can_hiz_clear_att(struct anv_cmd_buffer *cmd_buffer,
                  struct blorp_batch *batch,
                  const struct anv_attachment *ds_att,
                  const VkClearAttachment *attachment,
                  uint32_t rectCount, const VkClearRect *pRects)
{
   if (INTEL_DEBUG(DEBUG_NO_FAST_CLEAR))
      return false;

   /* From Bspec's section MI_PREDICATE:
    *
    *    "The MI_PREDICATE command is used to control the Predicate state bit,
    *    which in turn can be used to enable/disable the processing of
    *    3DPRIMITIVE commands."
    *
    * Also from BDW/CHV Bspec's 3DSTATE_WM_HZ_OP programming notes:
    *
    *    "This command does NOT support predication from the use of the
    *    MI_PREDICATE register. To predicate depth clears and resolves on you
    *    must fall back to using the 3D_PRIMITIVE or GPGPU_WALKER commands."
    *
    * Since BLORP's predication is currently dependent on MI_PREDICATE, fall
    * back to the slow depth clear path when the BLORP_BATCH_PREDICATE_ENABLE
    * flag is set.
    */
   if (batch->flags & BLORP_BATCH_PREDICATE_ENABLE)
      return false;

   if (rectCount > 1) {
      anv_perf_warn(VK_LOG_OBJS(&cmd_buffer->device->vk.base),
                    "Fast clears for vkCmdClearAttachments supported only for rectCount == 1");
      return false;
   }

   /* When the BLORP_BATCH_NO_EMIT_DEPTH_STENCIL flag is set, BLORP can only
    * clear the first slice of the currently configured depth/stencil view.
    */
   assert(batch->flags & BLORP_BATCH_NO_EMIT_DEPTH_STENCIL);
   if (pRects[0].layerCount > 1 || pRects[0].baseArrayLayer > 0)
      return false;

   return anv_can_hiz_clear_ds_view(cmd_buffer->device, ds_att->iview,
                                    ds_att->layout,
                                    attachment->aspectMask,
                                    attachment->clearValue.depthStencil.depth,
                                    pRects->rect,
                                    cmd_buffer->queue_family->queueFlags);
}

static void
clear_depth_stencil_attachment(struct anv_cmd_buffer *cmd_buffer,
                               struct blorp_batch *batch,
                               const VkClearAttachment *attachment,
                               uint32_t rectCount, const VkClearRect *pRects)
{
   static const union isl_color_value color_value = { .u32 = { 0, } };
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const struct anv_attachment *d_att = &gfx->depth_att;
   const struct anv_attachment *s_att = &gfx->stencil_att;
   if (d_att->vk_format == VK_FORMAT_UNDEFINED &&
       s_att->vk_format == VK_FORMAT_UNDEFINED)
      return;

   const struct anv_attachment *ds_att = d_att->iview ? d_att : s_att;
   if (ds_att->iview &&
       can_hiz_clear_att(cmd_buffer, batch, ds_att, attachment, rectCount, pRects)) {
      anv_fast_clear_depth_stencil(cmd_buffer, batch, ds_att->iview->image,
                                   attachment->aspectMask,
                                   ds_att->iview->planes[0].isl.base_level,
                                   ds_att->iview->planes[0].isl.base_array_layer,
                                   pRects[0].layerCount, pRects->rect,
                                   attachment->clearValue.depthStencil.stencil);
      return;
   }

   bool clear_depth = attachment->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT;
   bool clear_stencil = attachment->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT;

   enum isl_format depth_format = ISL_FORMAT_UNSUPPORTED;
   if (d_att->vk_format != VK_FORMAT_UNDEFINED) {
      depth_format = anv_get_isl_format(cmd_buffer->device->info,
                                        d_att->vk_format,
                                        VK_IMAGE_ASPECT_DEPTH_BIT,
                                        VK_IMAGE_TILING_OPTIMAL);
   }

   uint32_t binding_table;
   VkResult result =
      binding_table_for_surface_state(cmd_buffer,
                                      gfx->null_surface_state,
                                      &binding_table);
   if (result != VK_SUCCESS)
      return;

   /* If multiview is enabled we ignore baseArrayLayer and layerCount */
   if (gfx->view_mask) {
      u_foreach_bit(view_idx, gfx->view_mask) {
         for (uint32_t r = 0; r < rectCount; ++r) {
            const VkOffset2D offset = pRects[r].rect.offset;
            const VkExtent2D extent = pRects[r].rect.extent;
            VkClearDepthStencilValue value = attachment->clearValue.depthStencil;
            blorp_clear_attachments(batch, binding_table,
                                    depth_format,
                                    gfx->samples,
                                    view_idx, 1,
                                    offset.x, offset.y,
                                    offset.x + extent.width,
                                    offset.y + extent.height,
                                    false, color_value,
                                    clear_depth, value.depth,
                                    clear_stencil ? 0xff : 0, value.stencil);
         }
      }
      return;
   }

   for (uint32_t r = 0; r < rectCount; ++r) {
      const VkOffset2D offset = pRects[r].rect.offset;
      const VkExtent2D extent = pRects[r].rect.extent;
      VkClearDepthStencilValue value = attachment->clearValue.depthStencil;
      assert(pRects[r].layerCount != VK_REMAINING_ARRAY_LAYERS);
      blorp_clear_attachments(batch, binding_table,
                              depth_format,
                              gfx->samples,
                              pRects[r].baseArrayLayer,
                              pRects[r].layerCount,
                              offset.x, offset.y,
                              offset.x + extent.width, offset.y + extent.height,
                              false, color_value,
                              clear_depth, value.depth,
                              clear_stencil ? 0xff : 0, value.stencil);
   }
}

void anv_CmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   /* Because this gets called within a render pass, we tell blorp not to
    * trash our depth and stencil buffers.
    */
   struct blorp_batch batch;
   enum blorp_batch_flags flags = BLORP_BATCH_NO_EMIT_DEPTH_STENCIL;
   if (cmd_buffer->state.conditional_render_enabled) {
      anv_cmd_emit_conditional_render_predicate(cmd_buffer);
      flags |= BLORP_BATCH_PREDICATE_ENABLE;
   }
   anv_blorp_batch_init(cmd_buffer, &batch, flags);

   for (uint32_t a = 0; a < attachmentCount; ++a) {
      if (pAttachments[a].aspectMask & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV) {
         assert(pAttachments[a].aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);
         clear_color_attachment(cmd_buffer, &batch,
                                &pAttachments[a],
                                rectCount, pRects);
      } else {
         clear_depth_stencil_attachment(cmd_buffer, &batch,
                                        &pAttachments[a],
                                        rectCount, pRects);
      }
   }

   anv_blorp_batch_finish(&batch);
}

static void
anv_image_msaa_resolve(struct anv_cmd_buffer *cmd_buffer,
                       const struct anv_image *src_image,
                       enum isl_format src_format_override,
                       enum isl_aux_usage src_aux_usage,
                       uint32_t src_level, uint32_t src_base_layer,
                       const struct anv_image *dst_image,
                       enum isl_format dst_format_override,
                       enum isl_aux_usage dst_aux_usage,
                       uint32_t dst_level, uint32_t dst_base_layer,
                       VkImageAspectFlagBits aspect,
                       uint32_t src_x, uint32_t src_y,
                       uint32_t dst_x, uint32_t dst_y,
                       uint32_t width, uint32_t height,
                       uint32_t layer_count,
                       enum blorp_filter filter)
{
   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);
   assert((batch.flags & BLORP_BATCH_USE_COMPUTE) == 0);

   assert(src_image->vk.image_type == VK_IMAGE_TYPE_2D);
   assert(src_image->vk.samples > 1);
   assert(dst_image->vk.image_type == VK_IMAGE_TYPE_2D);
   assert(dst_image->vk.samples == 1);

   struct blorp_surf src_surf, dst_surf;
   get_blorp_surf_for_anv_image(cmd_buffer, src_image, aspect,
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                src_aux_usage, &src_surf);
   if (src_aux_usage == ISL_AUX_USAGE_MCS) {
      src_surf.clear_color_addr = anv_to_blorp_address(
         anv_image_get_clear_color_addr(cmd_buffer->device, src_image,
                                        VK_IMAGE_ASPECT_COLOR_BIT));
   }
   get_blorp_surf_for_anv_image(cmd_buffer, dst_image, aspect,
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                dst_aux_usage, &dst_surf);
   anv_cmd_buffer_mark_image_written(cmd_buffer, dst_image,
                                     aspect, dst_aux_usage,
                                     dst_level, dst_base_layer, layer_count);

   if (filter == BLORP_FILTER_NONE) {
      /* If no explicit filter is provided, then it's implied by the type of
       * the source image.
       */
      if ((src_surf.surf->usage & ISL_SURF_USAGE_DEPTH_BIT) ||
          (src_surf.surf->usage & ISL_SURF_USAGE_STENCIL_BIT) ||
          isl_format_has_int_channel(src_surf.surf->format)) {
         filter = BLORP_FILTER_SAMPLE_0;
      } else {
         filter = BLORP_FILTER_AVERAGE;
      }
   }

   for (uint32_t l = 0; l < layer_count; l++) {
      blorp_blit(&batch,
                 &src_surf, src_level, src_base_layer + l,
                 src_format_override, ISL_SWIZZLE_IDENTITY,
                 &dst_surf, dst_level, dst_base_layer + l,
                 dst_format_override, ISL_SWIZZLE_IDENTITY,
                 src_x, src_y, src_x + width, src_y + height,
                 dst_x, dst_y, dst_x + width, dst_y + height,
                 filter, false, false);
   }

   anv_blorp_batch_finish(&batch);
}

static enum blorp_filter
vk_to_blorp_resolve_mode(VkResolveModeFlagBits vk_mode)
{
   switch (vk_mode) {
   case VK_RESOLVE_MODE_SAMPLE_ZERO_BIT:
      return BLORP_FILTER_SAMPLE_0;
   case VK_RESOLVE_MODE_AVERAGE_BIT:
      return BLORP_FILTER_AVERAGE;
   case VK_RESOLVE_MODE_MIN_BIT:
      return BLORP_FILTER_MIN_SAMPLE;
   case VK_RESOLVE_MODE_MAX_BIT:
      return BLORP_FILTER_MAX_SAMPLE;
   default:
      return BLORP_FILTER_NONE;
   }
}

void
anv_attachment_msaa_resolve(struct anv_cmd_buffer *cmd_buffer,
                            const struct anv_attachment *att,
                            VkImageLayout layout,
                            VkImageAspectFlagBits aspect)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const struct anv_image_view *src_iview = att->iview;
   const struct anv_image_view *dst_iview = att->resolve_iview;

   enum isl_aux_usage src_aux_usage =
      anv_layout_to_aux_usage(cmd_buffer->device->info,
                              src_iview->image, aspect,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                              layout,
                              cmd_buffer->queue_family->queueFlags);

   enum isl_aux_usage dst_aux_usage =
      anv_layout_to_aux_usage(cmd_buffer->device->info,
                              dst_iview->image, aspect,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                              att->resolve_layout,
                              cmd_buffer->queue_family->queueFlags);

   enum blorp_filter filter = vk_to_blorp_resolve_mode(att->resolve_mode);

   /* Depth/stencil should not use their view format for resolve because they
    * go in pairs.
    */
   enum isl_format src_format = ISL_FORMAT_UNSUPPORTED;
   enum isl_format dst_format = ISL_FORMAT_UNSUPPORTED;
   if (!(aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
      src_format = src_iview->planes[0].isl.format;
      dst_format = dst_iview->planes[0].isl.format;
   }

   const VkRect2D render_area = gfx->render_area;
   if (gfx->view_mask == 0) {
      anv_image_msaa_resolve(cmd_buffer,
                             src_iview->image, src_format, src_aux_usage,
                             src_iview->planes[0].isl.base_level,
                             src_iview->planes[0].isl.base_array_layer,
                             dst_iview->image, dst_format, dst_aux_usage,
                             dst_iview->planes[0].isl.base_level,
                             dst_iview->planes[0].isl.base_array_layer,
                             aspect,
                             render_area.offset.x, render_area.offset.y,
                             render_area.offset.x, render_area.offset.y,
                             render_area.extent.width,
                             render_area.extent.height,
                             gfx->layer_count, filter);
   } else {
      uint32_t res_view_mask = gfx->view_mask;
      while (res_view_mask) {
         int i = u_bit_scan(&res_view_mask);

         anv_image_msaa_resolve(cmd_buffer,
                                src_iview->image, src_format, src_aux_usage,
                                src_iview->planes[0].isl.base_level,
                                src_iview->planes[0].isl.base_array_layer + i,
                                dst_iview->image, dst_format, dst_aux_usage,
                                dst_iview->planes[0].isl.base_level,
                                dst_iview->planes[0].isl.base_array_layer + i,
                                aspect,
                                render_area.offset.x, render_area.offset.y,
                                render_area.offset.x, render_area.offset.y,
                                render_area.extent.width,
                                render_area.extent.height,
                                1, filter);
      }
   }
}

static void
resolve_image(struct anv_cmd_buffer *cmd_buffer,
              struct anv_image *src_image,
              VkImageLayout src_image_layout,
              struct anv_image *dst_image,
              VkImageLayout dst_image_layout,
              const VkImageResolve2 *region)
{
   assert(region->srcSubresource.aspectMask == region->dstSubresource.aspectMask);
   assert(vk_image_subresource_layer_count(&src_image->vk, &region->srcSubresource) ==
          vk_image_subresource_layer_count(&dst_image->vk, &region->dstSubresource));

   const uint32_t layer_count =
      vk_image_subresource_layer_count(&dst_image->vk, &region->dstSubresource);

   anv_foreach_image_aspect_bit(aspect_bit, src_image,
                                region->srcSubresource.aspectMask) {
      enum isl_aux_usage src_aux_usage =
         anv_layout_to_aux_usage(cmd_buffer->device->info, src_image,
                                 (1 << aspect_bit),
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 src_image_layout,
                                 cmd_buffer->queue_family->queueFlags);
      enum isl_aux_usage dst_aux_usage =
         anv_layout_to_aux_usage(cmd_buffer->device->info, dst_image,
                                 (1 << aspect_bit),
                                 VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                 dst_image_layout,
                                 cmd_buffer->queue_family->queueFlags);

      anv_image_msaa_resolve(cmd_buffer,
                             src_image, ISL_FORMAT_UNSUPPORTED, src_aux_usage,
                             region->srcSubresource.mipLevel,
                             region->srcSubresource.baseArrayLayer,
                             dst_image, ISL_FORMAT_UNSUPPORTED, dst_aux_usage,
                             region->dstSubresource.mipLevel,
                             region->dstSubresource.baseArrayLayer,
                             (1 << aspect_bit),
                             region->srcOffset.x,
                             region->srcOffset.y,
                             region->dstOffset.x,
                             region->dstOffset.y,
                             region->extent.width,
                             region->extent.height,
                             layer_count, BLORP_FILTER_NONE);
   }
}

void anv_CmdResolveImage2(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_image, src_image, pResolveImageInfo->srcImage);
   ANV_FROM_HANDLE(anv_image, dst_image, pResolveImageInfo->dstImage);

   for (uint32_t r = 0; r < pResolveImageInfo->regionCount; r++) {
      resolve_image(cmd_buffer,
                    src_image, pResolveImageInfo->srcImageLayout,
                    dst_image, pResolveImageInfo->dstImageLayout,
                    &pResolveImageInfo->pRegions[r]);
   }
}

void
anv_image_clear_color(struct anv_cmd_buffer *cmd_buffer,
                      const struct anv_image *image,
                      VkImageAspectFlagBits aspect,
                      enum isl_aux_usage aux_usage,
                      enum isl_format format, struct isl_swizzle swizzle,
                      uint32_t level, uint32_t base_layer, uint32_t layer_count,
                      VkRect2D area, union isl_color_value clear_color)
{
   assert(image->vk.aspects == VK_IMAGE_ASPECT_COLOR_BIT);

   /* We don't support planar images with multisampling yet */
   assert(image->n_planes == 1);

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);

   struct blorp_surf surf;
   get_blorp_surf_for_anv_image(cmd_buffer, image, aspect,
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                aux_usage, &surf);
   anv_cmd_buffer_mark_image_written(cmd_buffer, image, aspect, aux_usage,
                                     level, base_layer, layer_count);

   blorp_clear(&batch, &surf, format, anv_swizzle_for_render(swizzle),
               level, base_layer, layer_count,
               area.offset.x, area.offset.y,
               area.offset.x + area.extent.width,
               area.offset.y + area.extent.height,
               clear_color, 0 /* color_write_disable */);

   anv_blorp_batch_finish(&batch);
}

void
anv_image_clear_depth_stencil(struct anv_cmd_buffer *cmd_buffer,
                              const struct anv_image *image,
                              VkImageAspectFlags aspects,
                              enum isl_aux_usage depth_aux_usage,
                              uint32_t level,
                              uint32_t base_layer, uint32_t layer_count,
                              VkRect2D area,
                              float depth_value, uint8_t stencil_value)
{
   assert(image->vk.aspects & (VK_IMAGE_ASPECT_DEPTH_BIT |
                               VK_IMAGE_ASPECT_STENCIL_BIT));

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);
   assert((batch.flags & BLORP_BATCH_USE_COMPUTE) == 0);

   struct blorp_surf depth = {};
   if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   image, VK_IMAGE_ASPECT_DEPTH_BIT,
                                   0, ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                   depth_aux_usage, &depth);
   }

   struct blorp_surf stencil = {};
   if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      const uint32_t plane =
         anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_STENCIL_BIT);
      get_blorp_surf_for_anv_image(cmd_buffer,
                                   image, VK_IMAGE_ASPECT_STENCIL_BIT,
                                   0, ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                   image->planes[plane].aux_usage, &stencil);
   }

   /* Blorp may choose to clear stencil using RGBA32_UINT for better
    * performance.  If it does this, we need to flush it out of the depth
    * cache before rendering to it.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_DEPTH_CACHE_FLUSH_BIT |
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "before clear DS");

   blorp_clear_depth_stencil(&batch, &depth, &stencil,
                             level, base_layer, layer_count,
                             area.offset.x, area.offset.y,
                             area.offset.x + area.extent.width,
                             area.offset.y + area.extent.height,
                             aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                             depth_value,
                             (aspects & VK_IMAGE_ASPECT_STENCIL_BIT) ? 0xff : 0,
                             stencil_value);

   /* Blorp may choose to clear stencil using RGBA32_UINT for better
    * performance.  If it does this, we need to flush it out of the render
    * cache before someone starts trying to do stencil on it.
    */
   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "after clear DS");

   anv_blorp_batch_finish(&batch);
}

void
anv_image_hiz_op(struct anv_cmd_buffer *cmd_buffer,
                 const struct anv_image *image,
                 VkImageAspectFlagBits aspect, uint32_t level,
                 uint32_t base_layer, uint32_t layer_count,
                 enum isl_aux_op hiz_op)
{
   assert(aspect == VK_IMAGE_ASPECT_DEPTH_BIT);
   assert(base_layer + layer_count <= anv_image_aux_layers(image, aspect, level));
   const uint32_t plane = anv_image_aspect_to_plane(image, aspect);
   assert(plane == 0);

   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);
   assert((batch.flags & BLORP_BATCH_USE_COMPUTE) == 0);

   struct blorp_surf surf;
   get_blorp_surf_for_anv_image(cmd_buffer,
                                image, VK_IMAGE_ASPECT_DEPTH_BIT,
                                0, ANV_IMAGE_LAYOUT_EXPLICIT_AUX,
                                image->planes[plane].aux_usage, &surf);

   blorp_hiz_op(&batch, &surf, level, base_layer, layer_count, hiz_op);

   anv_blorp_batch_finish(&batch);
}

void
anv_image_hiz_clear(struct anv_cmd_buffer *cmd_buffer,
                    const struct anv_image *image,
                    VkImageAspectFlags aspects,
                    uint32_t level,
                    uint32_t base_layer, uint32_t layer_count,
                    VkRect2D area, uint8_t stencil_value)
{
   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch, 0);
   assert((batch.flags & BLORP_BATCH_USE_COMPUTE) == 0);

   anv_fast_clear_depth_stencil(cmd_buffer, &batch, image, aspects, level,
                                base_layer, layer_count, area, stencil_value);

   anv_blorp_batch_finish(&batch);
}

void
anv_image_mcs_op(struct anv_cmd_buffer *cmd_buffer,
                 const struct anv_image *image,
                 enum isl_format format, struct isl_swizzle swizzle,
                 VkImageAspectFlagBits aspect,
                 uint32_t base_layer, uint32_t layer_count,
                 enum isl_aux_op mcs_op, union isl_color_value *clear_value,
                 bool predicate)
{
   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch,
                        BLORP_BATCH_PREDICATE_ENABLE * predicate +
                        BLORP_BATCH_NO_UPDATE_CLEAR_COLOR * !clear_value);
   assert((batch.flags & BLORP_BATCH_USE_COMPUTE) == 0);

   exec_mcs_op(cmd_buffer, &batch, image, format, swizzle, aspect,
               base_layer, layer_count, mcs_op, clear_value);

   anv_blorp_batch_finish(&batch);
}

void
anv_image_ccs_op(struct anv_cmd_buffer *cmd_buffer,
                 const struct anv_image *image,
                 enum isl_format format, struct isl_swizzle swizzle,
                 VkImageAspectFlagBits aspect, uint32_t level,
                 uint32_t base_layer, uint32_t layer_count,
                 enum isl_aux_op ccs_op, union isl_color_value *clear_value,
                 bool predicate)
{
   struct blorp_batch batch;
   anv_blorp_batch_init(cmd_buffer, &batch,
                        BLORP_BATCH_PREDICATE_ENABLE * predicate +
                        BLORP_BATCH_NO_UPDATE_CLEAR_COLOR * !clear_value);
   assert((batch.flags & BLORP_BATCH_USE_COMPUTE) == 0);

   exec_ccs_op(cmd_buffer, &batch, image, format, swizzle, aspect, level,
               base_layer, layer_count, ccs_op, clear_value);

   anv_blorp_batch_finish(&batch);
}
