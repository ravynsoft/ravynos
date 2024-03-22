/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_cmd_buffer.h"

#include "nvk_buffer.h"
#include "nvk_device.h"
#include "nvk_device_memory.h"
#include "nvk_entrypoints.h"
#include "nvk_format.h"
#include "nvk_image.h"
#include "nvk_physical_device.h"

#include "vk_format.h"

#include "nouveau_bo.h"
#include "nouveau_context.h"

#include "nvtypes.h"
#include "nvk_cl902d.h"
#include "nvk_cl90b5.h"
#include "nvk_clc1b5.h"

struct nouveau_copy_buffer {
   uint64_t base_addr;
   VkImageType image_type;
   struct nil_offset4d offset_el;
   struct nil_extent4d extent_el;
   uint32_t bpp;
   uint32_t row_stride;
   uint32_t array_stride;
   struct nil_tiling tiling;
};

struct nouveau_copy {
   struct nouveau_copy_buffer src;
   struct nouveau_copy_buffer dst;
   struct nouveau_copy_remap {
      uint8_t comp_size;
      uint8_t dst[4];
   } remap;
   struct nil_extent4d extent_el;
};

static struct nouveau_copy_buffer
nouveau_copy_rect_buffer(struct nvk_buffer *buf,
                         VkDeviceSize offset,
                         struct vk_image_buffer_layout buffer_layout)
{
   return (struct nouveau_copy_buffer) {
      .base_addr = nvk_buffer_address(buf, offset),
      .image_type = VK_IMAGE_TYPE_2D,
      .bpp = buffer_layout.element_size_B,
      .row_stride = buffer_layout.row_stride_B,
      .array_stride = buffer_layout.image_stride_B,
   };
}

static struct nil_offset4d
vk_to_nil_offset(VkOffset3D offset, uint32_t base_array_layer)
{
   return nil_offset4d(offset.x, offset.y, offset.z, base_array_layer);
}

static struct nil_extent4d
vk_to_nil_extent(VkExtent3D extent, uint32_t array_layers)
{
   return nil_extent4d(extent.width, extent.height, extent.depth, array_layers);
}

static struct nouveau_copy_buffer
nouveau_copy_rect_image(struct nvk_image *img,
                        struct nvk_image_plane *plane,
                        VkOffset3D offset_px,
                        const VkImageSubresourceLayers *sub_res)
{
   const struct nil_extent4d lvl_extent4d_px =
      nil_image_level_extent_px(&plane->nil, sub_res->mipLevel);

   offset_px = vk_image_sanitize_offset(&img->vk, offset_px);
   const struct nil_offset4d offset4d_px =
      vk_to_nil_offset(offset_px, sub_res->baseArrayLayer);

   struct nouveau_copy_buffer buf = {
      .base_addr = nvk_image_plane_base_address(plane) +
                   plane->nil.levels[sub_res->mipLevel].offset_B,
      .image_type = img->vk.image_type,
      .offset_el = nil_offset4d_px_to_el(offset4d_px, plane->nil.format,
                                         plane->nil.sample_layout),
      .extent_el = nil_extent4d_px_to_el(lvl_extent4d_px, plane->nil.format,
                                         plane->nil.sample_layout),
      .bpp = util_format_get_blocksize(plane->nil.format),
      .row_stride = plane->nil.levels[sub_res->mipLevel].row_stride_B,
      .array_stride = plane->nil.array_stride_B,
      .tiling = plane->nil.levels[sub_res->mipLevel].tiling,
   };

   return buf;
}

static struct nouveau_copy_remap
nouveau_copy_remap_format(VkFormat format)
{
   /* Pick an arbitrary component size.  It doesn't matter what size we
    * pick since we're just doing a copy, as long as it's no more than 4B
    * and divides the format size.
    */
   unsigned comp_size = vk_format_get_blocksize(format);
   if (comp_size % 3 == 0) {
      comp_size /= 3;
      assert(util_is_power_of_two_nonzero(comp_size) && comp_size <= 4);
   } else {
      assert(util_is_power_of_two_nonzero(comp_size) && comp_size <= 16);
      comp_size = MIN2(comp_size, 4);
   }

   return (struct nouveau_copy_remap) {
      .comp_size = comp_size,
      .dst = { 0, 1, 2, 3 },
   };
}

static uint32_t
to_90b5_remap_comp_size(uint8_t comp_size)
{
   static const uint8_t to_90b5[] = {
      [1] = NV90B5_SET_REMAP_COMPONENTS_COMPONENT_SIZE_ONE,
      [2] = NV90B5_SET_REMAP_COMPONENTS_COMPONENT_SIZE_TWO,
      [3] = NV90B5_SET_REMAP_COMPONENTS_COMPONENT_SIZE_THREE,
      [4] = NV90B5_SET_REMAP_COMPONENTS_COMPONENT_SIZE_FOUR,
   };
   assert(comp_size > 0 && comp_size < ARRAY_SIZE(to_90b5));

   uint32_t size_90b5 = comp_size - 1;
   assert(size_90b5 == to_90b5[comp_size]);
   return size_90b5;
}

static uint32_t
to_90b5_remap_num_comps(uint8_t num_comps)
{
   static const uint8_t to_90b5[] = {
      [1] = NV90B5_SET_REMAP_COMPONENTS_NUM_SRC_COMPONENTS_ONE,
      [2] = NV90B5_SET_REMAP_COMPONENTS_NUM_SRC_COMPONENTS_TWO,
      [3] = NV90B5_SET_REMAP_COMPONENTS_NUM_SRC_COMPONENTS_THREE,
      [4] = NV90B5_SET_REMAP_COMPONENTS_NUM_SRC_COMPONENTS_FOUR,
   };
   assert(num_comps > 0 && num_comps < ARRAY_SIZE(to_90b5));

   uint32_t num_comps_90b5 = num_comps - 1;
   assert(num_comps_90b5 == to_90b5[num_comps]);
   return num_comps_90b5;
}

static void
nouveau_copy_rect(struct nvk_cmd_buffer *cmd, struct nouveau_copy *copy)
{
   uint32_t src_bw, dst_bw;
   if (copy->remap.comp_size > 0) {
      struct nv_push *p = nvk_cmd_buffer_push(cmd, 2);

      assert(copy->src.bpp % copy->remap.comp_size == 0);
      assert(copy->dst.bpp % copy->remap.comp_size == 0);
      uint32_t num_src_comps = copy->src.bpp / copy->remap.comp_size;
      uint32_t num_dst_comps = copy->dst.bpp / copy->remap.comp_size;

      /* When running with component remapping enabled, most X/Y dimensions
       * are in units of blocks.
       */
      src_bw = dst_bw = 1;

      P_IMMD(p, NV90B5, SET_REMAP_COMPONENTS, {
         .dst_x = copy->remap.dst[0],
         .dst_y = copy->remap.dst[1],
         .dst_z = copy->remap.dst[2],
         .dst_w = copy->remap.dst[3],
         .component_size = to_90b5_remap_comp_size(copy->remap.comp_size),
         .num_src_components = to_90b5_remap_comp_size(num_src_comps),
         .num_dst_components = to_90b5_remap_comp_size(num_dst_comps),
      });
   } else {
      /* When component remapping is disabled, dimensions are in units of
       * bytes (an implicit block widht of 1B).
       */
      assert(copy->src.bpp == copy->dst.bpp);
      src_bw = copy->src.bpp;
      dst_bw = copy->dst.bpp;
   }

   assert(copy->extent_el.depth == 1 || copy->extent_el.array_len == 1);
   for (unsigned z = 0; z < MAX2(copy->extent_el.d, copy->extent_el.a); z++) {
      VkDeviceSize src_addr = copy->src.base_addr;
      VkDeviceSize dst_addr = copy->dst.base_addr;

      if (copy->src.image_type != VK_IMAGE_TYPE_3D)
         src_addr += (z + copy->src.offset_el.a) * copy->src.array_stride;

      if (copy->dst.image_type != VK_IMAGE_TYPE_3D)
         dst_addr += (z + copy->dst.offset_el.a) * copy->dst.array_stride;

      if (!copy->src.tiling.is_tiled) {
         src_addr += copy->src.offset_el.x * copy->src.bpp +
                     copy->src.offset_el.y * copy->src.row_stride;
      }

      if (!copy->dst.tiling.is_tiled) {
         dst_addr += copy->dst.offset_el.x * copy->dst.bpp +
                     copy->dst.offset_el.y * copy->dst.row_stride;
      }

      struct nv_push *p = nvk_cmd_buffer_push(cmd, 31);

      P_MTHD(p, NV90B5, OFFSET_IN_UPPER);
      P_NV90B5_OFFSET_IN_UPPER(p, src_addr >> 32);
      P_NV90B5_OFFSET_IN_LOWER(p, src_addr & 0xffffffff);
      P_NV90B5_OFFSET_OUT_UPPER(p, dst_addr >> 32);
      P_NV90B5_OFFSET_OUT_LOWER(p, dst_addr & 0xffffffff);
      P_NV90B5_PITCH_IN(p, copy->src.row_stride);
      P_NV90B5_PITCH_OUT(p, copy->dst.row_stride);
      P_NV90B5_LINE_LENGTH_IN(p, copy->extent_el.width * src_bw);
      P_NV90B5_LINE_COUNT(p, copy->extent_el.height);

      uint32_t src_layout = 0, dst_layout = 0;
      if (copy->src.tiling.is_tiled) {
         P_MTHD(p, NV90B5, SET_SRC_BLOCK_SIZE);
         P_NV90B5_SET_SRC_BLOCK_SIZE(p, {
            .width = 0, /* Tiles are always 1 GOB wide */
            .height = copy->src.tiling.y_log2,
            .depth = copy->src.tiling.z_log2,
            .gob_height = copy->src.tiling.gob_height_8 ?
                          GOB_HEIGHT_GOB_HEIGHT_FERMI_8 :
                          GOB_HEIGHT_GOB_HEIGHT_TESLA_4,
         });
         P_NV90B5_SET_SRC_WIDTH(p, copy->src.extent_el.width * src_bw);
         P_NV90B5_SET_SRC_HEIGHT(p, copy->src.extent_el.height);
         P_NV90B5_SET_SRC_DEPTH(p, copy->src.extent_el.depth);
         if (copy->src.image_type == VK_IMAGE_TYPE_3D)
            P_NV90B5_SET_SRC_LAYER(p, z + copy->src.offset_el.z);
         else
            P_NV90B5_SET_SRC_LAYER(p, 0);

         if (nvk_cmd_buffer_device(cmd)->pdev->info.cls_copy >= 0xc1b5) {
            P_MTHD(p, NVC1B5, SRC_ORIGIN_X);
            P_NVC1B5_SRC_ORIGIN_X(p, copy->src.offset_el.x * src_bw);
            P_NVC1B5_SRC_ORIGIN_Y(p, copy->src.offset_el.y);
         } else {
            P_MTHD(p, NV90B5, SET_SRC_ORIGIN);
            P_NV90B5_SET_SRC_ORIGIN(p, {
               .x = copy->src.offset_el.x * src_bw,
               .y = copy->src.offset_el.y
            });
         }

         src_layout = NV90B5_LAUNCH_DMA_SRC_MEMORY_LAYOUT_BLOCKLINEAR;
      } else {
         src_addr += copy->src.array_stride;
         src_layout = NV90B5_LAUNCH_DMA_SRC_MEMORY_LAYOUT_PITCH;
      }

      if (copy->dst.tiling.is_tiled) {
         P_MTHD(p, NV90B5, SET_DST_BLOCK_SIZE);
         P_NV90B5_SET_DST_BLOCK_SIZE(p, {
            .width = 0, /* Tiles are always 1 GOB wide */
            .height = copy->dst.tiling.y_log2,
            .depth = copy->dst.tiling.z_log2,
            .gob_height = copy->dst.tiling.gob_height_8 ?
                          GOB_HEIGHT_GOB_HEIGHT_FERMI_8 :
                          GOB_HEIGHT_GOB_HEIGHT_TESLA_4,
         });
         P_NV90B5_SET_DST_WIDTH(p, copy->dst.extent_el.width * dst_bw);
         P_NV90B5_SET_DST_HEIGHT(p, copy->dst.extent_el.height);
         P_NV90B5_SET_DST_DEPTH(p, copy->dst.extent_el.depth);
         if (copy->dst.image_type == VK_IMAGE_TYPE_3D)
            P_NV90B5_SET_DST_LAYER(p, z + copy->dst.offset_el.z);
         else
            P_NV90B5_SET_DST_LAYER(p, 0);

         if (nvk_cmd_buffer_device(cmd)->pdev->info.cls_copy >= 0xc1b5) {
            P_MTHD(p, NVC1B5, DST_ORIGIN_X);
            P_NVC1B5_DST_ORIGIN_X(p, copy->dst.offset_el.x * dst_bw);
            P_NVC1B5_DST_ORIGIN_Y(p, copy->dst.offset_el.y);
         } else {
            P_MTHD(p, NV90B5, SET_DST_ORIGIN);
            P_NV90B5_SET_DST_ORIGIN(p, {
               .x = copy->dst.offset_el.x * dst_bw,
               .y = copy->dst.offset_el.y
            });
         }

         dst_layout = NV90B5_LAUNCH_DMA_DST_MEMORY_LAYOUT_BLOCKLINEAR;
      } else {
         dst_addr += copy->dst.array_stride;
         dst_layout = NV90B5_LAUNCH_DMA_DST_MEMORY_LAYOUT_PITCH;
      }

      P_IMMD(p, NV90B5, LAUNCH_DMA, {
         .data_transfer_type = DATA_TRANSFER_TYPE_NON_PIPELINED,
         .multi_line_enable = MULTI_LINE_ENABLE_TRUE,
         .flush_enable = FLUSH_ENABLE_TRUE,
         .src_memory_layout = src_layout,
         .dst_memory_layout = dst_layout,
         .remap_enable = copy->remap.comp_size > 0,
      });
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdCopyBuffer2(VkCommandBuffer commandBuffer,
                   const VkCopyBufferInfo2 *pCopyBufferInfo)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_buffer, src, pCopyBufferInfo->srcBuffer);
   VK_FROM_HANDLE(nvk_buffer, dst, pCopyBufferInfo->dstBuffer);

   for (unsigned r = 0; r < pCopyBufferInfo->regionCount; r++) {
      const VkBufferCopy2 *region = &pCopyBufferInfo->pRegions[r];

      uint64_t src_addr = nvk_buffer_address(src, region->srcOffset);
      uint64_t dst_addr = nvk_buffer_address(dst, region->dstOffset);
      uint64_t size = region->size;

      while (size) {
         struct nv_push *p = nvk_cmd_buffer_push(cmd, 10);

         P_MTHD(p, NV90B5, OFFSET_IN_UPPER);
         P_NV90B5_OFFSET_IN_UPPER(p, src_addr >> 32);
         P_NV90B5_OFFSET_IN_LOWER(p, src_addr & 0xffffffff);
         P_NV90B5_OFFSET_OUT_UPPER(p, dst_addr >> 32);
         P_NV90B5_OFFSET_OUT_LOWER(p, dst_addr & 0xffffffff);

         unsigned bytes = MIN2(size, 1 << 17);

         P_MTHD(p, NV90B5, LINE_LENGTH_IN);
         P_NV90B5_LINE_LENGTH_IN(p, bytes);
         P_NV90B5_LINE_COUNT(p, 1);

         P_IMMD(p, NV90B5, LAUNCH_DMA, {
                .data_transfer_type = DATA_TRANSFER_TYPE_NON_PIPELINED,
                .multi_line_enable = MULTI_LINE_ENABLE_TRUE,
                .flush_enable = FLUSH_ENABLE_TRUE,
                .src_memory_layout = SRC_MEMORY_LAYOUT_PITCH,
                .dst_memory_layout = DST_MEMORY_LAYOUT_PITCH,
         });

         src_addr += bytes;
         dst_addr += bytes;
         size -= bytes;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                          const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_buffer, src, pCopyBufferToImageInfo->srcBuffer);
   VK_FROM_HANDLE(nvk_image, dst, pCopyBufferToImageInfo->dstImage);

   for (unsigned r = 0; r < pCopyBufferToImageInfo->regionCount; r++) {
      const VkBufferImageCopy2 *region = &pCopyBufferToImageInfo->pRegions[r];
      struct vk_image_buffer_layout buffer_layout =
         vk_image_buffer_copy_layout(&dst->vk, region);

      const VkExtent3D extent_px =
         vk_image_sanitize_extent(&dst->vk, region->imageExtent);
      const struct nil_extent4d extent4d_px =
         vk_to_nil_extent(extent_px, region->imageSubresource.layerCount);

      const VkImageAspectFlagBits aspects = region->imageSubresource.aspectMask;
      uint8_t plane = nvk_image_aspects_to_plane(dst, aspects);

      struct nouveau_copy copy = {
         .src = nouveau_copy_rect_buffer(src, region->bufferOffset,
                                         buffer_layout),
         .dst = nouveau_copy_rect_image(dst, &dst->planes[plane],
                                        region->imageOffset,
                                        &region->imageSubresource),
         .extent_el = nil_extent4d_px_to_el(extent4d_px, dst->planes[plane].nil.format,
                                            dst->planes[plane].nil.sample_layout),
      };
      struct nouveau_copy copy2 = { 0 };

      switch (dst->vk.format) {
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
         if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT) {
            copy.remap.comp_size = 4;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_SRC_X;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_NO_WRITE;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;
         } else {
            assert(aspects == VK_IMAGE_ASPECT_STENCIL_BIT);
            copy2.dst = copy.dst;
            copy2.extent_el = copy.extent_el;
            copy.dst = copy2.src =
               nouveau_copy_rect_image(dst, &dst->stencil_copy_temp,
                                       region->imageOffset,
                                       &region->imageSubresource);

            copy.remap.comp_size = 1;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_SRC_X;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_NO_WRITE;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;

            copy2.remap.comp_size = 2;
            copy2.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_NO_WRITE;
            copy2.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy2.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_SRC_X;
            copy2.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;
         }
         break;
      case VK_FORMAT_D24_UNORM_S8_UINT:
         if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT) {
            copy.remap.comp_size = 1;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_SRC_X;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_SRC_Y;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_SRC_Z;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;
         } else {
            assert(aspects == VK_IMAGE_ASPECT_STENCIL_BIT);
            copy.remap.comp_size = 1;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_NO_WRITE;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_NO_WRITE;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_SRC_X;
         }
         break;
      default:
         copy.remap = nouveau_copy_remap_format(dst->vk.format);
         break;
      }

      nouveau_copy_rect(cmd, &copy);
      if (copy2.extent_el.w > 0)
         nouveau_copy_rect(cmd, &copy2);

      vk_foreach_struct_const(ext, region->pNext) {
         switch (ext->sType) {
         default:
            nvk_debug_ignored_stype(ext->sType);
            break;
         }
      }
   }

   vk_foreach_struct_const(ext, pCopyBufferToImageInfo->pNext) {
      switch (ext->sType) {
      default:
         nvk_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                          const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_image, src, pCopyImageToBufferInfo->srcImage);
   VK_FROM_HANDLE(nvk_buffer, dst, pCopyImageToBufferInfo->dstBuffer);

   for (unsigned r = 0; r < pCopyImageToBufferInfo->regionCount; r++) {
      const VkBufferImageCopy2 *region = &pCopyImageToBufferInfo->pRegions[r];
      struct vk_image_buffer_layout buffer_layout =
         vk_image_buffer_copy_layout(&src->vk, region);

      const VkExtent3D extent_px =
         vk_image_sanitize_extent(&src->vk, region->imageExtent);
      const struct nil_extent4d extent4d_px =
         vk_to_nil_extent(extent_px, region->imageSubresource.layerCount);

      const VkImageAspectFlagBits aspects = region->imageSubresource.aspectMask;
      uint8_t plane = nvk_image_aspects_to_plane(src, aspects);

      struct nouveau_copy copy = {
         .src = nouveau_copy_rect_image(src, &src->planes[plane],
                                        region->imageOffset,
                                        &region->imageSubresource),
         .dst = nouveau_copy_rect_buffer(dst, region->bufferOffset,
                                         buffer_layout),
         .extent_el = nil_extent4d_px_to_el(extent4d_px, src->planes[plane].nil.format,
                                            src->planes[plane].nil.sample_layout),
      };
      struct nouveau_copy copy2 = { 0 };

      switch (src->vk.format) {
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
         if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT) {
            copy.remap.comp_size = 4;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_SRC_X;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_NO_WRITE;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;
         } else {
            assert(aspects == VK_IMAGE_ASPECT_STENCIL_BIT);
            copy2.dst = copy.dst;
            copy2.extent_el = copy.extent_el;
            copy.dst = copy2.src =
               nouveau_copy_rect_image(src, &src->stencil_copy_temp,
                                       region->imageOffset,
                                       &region->imageSubresource);

            copy.remap.comp_size = 2;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_SRC_Z;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_NO_WRITE;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;

            copy2.remap.comp_size = 1;
            copy2.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_SRC_X;
            copy2.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy2.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_NO_WRITE;
            copy2.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;
         }
         break;
      case VK_FORMAT_D24_UNORM_S8_UINT:
         if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT) {
            copy.remap.comp_size = 1;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_SRC_X;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_SRC_Y;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_SRC_Z;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;
         } else {
            assert(aspects == VK_IMAGE_ASPECT_STENCIL_BIT);
            copy.remap.comp_size = 1;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_SRC_W;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_NO_WRITE;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;
         }
         break;
      default:
         copy.remap = nouveau_copy_remap_format(src->vk.format);
         break;
      }

      nouveau_copy_rect(cmd, &copy);
      if (copy2.extent_el.w > 0)
         nouveau_copy_rect(cmd, &copy2);

      vk_foreach_struct_const(ext, region->pNext) {
         switch (ext->sType) {
         default:
            nvk_debug_ignored_stype(ext->sType);
            break;
         }
      }
   }

   vk_foreach_struct_const(ext, pCopyImageToBufferInfo->pNext) {
      switch (ext->sType) {
      default:
         nvk_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdCopyImage2(VkCommandBuffer commandBuffer,
                  const VkCopyImageInfo2 *pCopyImageInfo)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_image, src, pCopyImageInfo->srcImage);
   VK_FROM_HANDLE(nvk_image, dst, pCopyImageInfo->dstImage);

   for (unsigned r = 0; r < pCopyImageInfo->regionCount; r++) {
      const VkImageCopy2 *region = &pCopyImageInfo->pRegions[r];

      /* From the Vulkan 1.3.217 spec:
       *
       *    "When copying between compressed and uncompressed formats the
       *    extent members represent the texel dimensions of the source image
       *    and not the destination."
       */
      const VkExtent3D extent_px =
         vk_image_sanitize_extent(&src->vk, region->extent);
      const struct nil_extent4d extent4d_px =
         vk_to_nil_extent(extent_px, region->srcSubresource.layerCount);

      const VkImageAspectFlagBits src_aspects =
         region->srcSubresource.aspectMask;
      uint8_t src_plane = nvk_image_aspects_to_plane(src, src_aspects);

      const VkImageAspectFlagBits dst_aspects =
         region->dstSubresource.aspectMask;
      uint8_t dst_plane = nvk_image_aspects_to_plane(dst, dst_aspects);

      struct nouveau_copy copy = {
         .src = nouveau_copy_rect_image(src, &src->planes[src_plane],
                                        region->srcOffset,
                                        &region->srcSubresource),
         .dst = nouveau_copy_rect_image(dst, &dst->planes[dst_plane],
                                        region->dstOffset,
                                        &region->dstSubresource),
         .extent_el = nil_extent4d_px_to_el(extent4d_px, src->planes[src_plane].nil.format,
                                            src->planes[src_plane].nil.sample_layout),
      };

      assert(src_aspects == region->srcSubresource.aspectMask);
      switch (src->vk.format) {
      case VK_FORMAT_D24_UNORM_S8_UINT:
         if (src_aspects == VK_IMAGE_ASPECT_DEPTH_BIT) {
            copy.remap.comp_size = 1;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_W_SRC_X;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_SRC_Y;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_SRC_Z;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_NO_WRITE;
         } else if (src_aspects == VK_IMAGE_ASPECT_STENCIL_BIT) {
            copy.remap.comp_size = 1;
            copy.remap.dst[0] = NV90B5_SET_REMAP_COMPONENTS_DST_X_NO_WRITE;
            copy.remap.dst[1] = NV90B5_SET_REMAP_COMPONENTS_DST_Y_NO_WRITE;
            copy.remap.dst[2] = NV90B5_SET_REMAP_COMPONENTS_DST_Z_NO_WRITE;
            copy.remap.dst[3] = NV90B5_SET_REMAP_COMPONENTS_DST_W_SRC_W;
         } else {
            /* If we're copying both, there's nothing special to do */
            assert(src_aspects == (VK_IMAGE_ASPECT_DEPTH_BIT |
                               VK_IMAGE_ASPECT_STENCIL_BIT));
         }
         break;
      default:
         copy.remap = nouveau_copy_remap_format(src->vk.format);
         break;
      }

      nouveau_copy_rect(cmd, &copy);
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdFillBuffer(VkCommandBuffer commandBuffer,
                  VkBuffer dstBuffer,
                  VkDeviceSize dstOffset,
                  VkDeviceSize size,
                  uint32_t data)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_buffer, dst_buffer, dstBuffer);

   uint64_t dst_addr = nvk_buffer_address(dst_buffer, dstOffset);
   size = vk_buffer_range(&dst_buffer->vk, dstOffset, size);

   uint32_t max_dim = 1 << 15;

   struct nv_push *p = nvk_cmd_buffer_push(cmd, 7);

   P_IMMD(p, NV90B5, SET_REMAP_CONST_A, data);
   P_IMMD(p, NV90B5, SET_REMAP_COMPONENTS, {
      .dst_x = DST_X_CONST_A,
      .dst_y = DST_Y_CONST_A,
      .dst_z = DST_Z_CONST_A,
      .dst_w = DST_W_CONST_A,
      .component_size = COMPONENT_SIZE_FOUR,
      .num_src_components = NUM_SRC_COMPONENTS_ONE,
      .num_dst_components = NUM_DST_COMPONENTS_ONE,
   });

   P_MTHD(p, NV90B5, PITCH_IN);
   P_NV90B5_PITCH_IN(p, max_dim * 4);
   P_NV90B5_PITCH_OUT(p, max_dim * 4);

   while (size >= 4) {
      struct nv_push *p = nvk_cmd_buffer_push(cmd, 8);

      P_MTHD(p, NV90B5, OFFSET_OUT_UPPER);
      P_NV90B5_OFFSET_OUT_UPPER(p, dst_addr >> 32);
      P_NV90B5_OFFSET_OUT_LOWER(p, dst_addr & 0xffffffff);

      uint64_t width, height;
      if (size >= (uint64_t)max_dim * (uint64_t)max_dim * 4) {
         width = height = max_dim;
      } else if (size >= max_dim * 4) {
         width = max_dim;
         height = size / (max_dim * 4);
      } else {
         width = size / 4;
         height = 1;
      }

      uint64_t dma_size = (uint64_t)width * (uint64_t)height * 4;
      assert(dma_size <= size);

      P_MTHD(p, NV90B5, LINE_LENGTH_IN);
      P_NV90B5_LINE_LENGTH_IN(p, width);
      P_NV90B5_LINE_COUNT(p, height);

      P_IMMD(p, NV90B5, LAUNCH_DMA, {
         .data_transfer_type = DATA_TRANSFER_TYPE_NON_PIPELINED,
         .multi_line_enable = height > 1,
         .flush_enable = FLUSH_ENABLE_TRUE,
         .src_memory_layout = SRC_MEMORY_LAYOUT_PITCH,
         .dst_memory_layout = DST_MEMORY_LAYOUT_PITCH,
         .remap_enable = REMAP_ENABLE_TRUE,
      });

      dst_addr += dma_size;
      size -= dma_size;
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdUpdateBuffer(VkCommandBuffer commandBuffer,
                    VkBuffer dstBuffer,
                    VkDeviceSize dstOffset,
                    VkDeviceSize dataSize,
                    const void *pData)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_buffer, dst, dstBuffer);

   uint64_t dst_addr = nvk_buffer_address(dst, dstOffset);

   uint64_t data_addr;
   nvk_cmd_buffer_upload_data(cmd, pData, dataSize, 64, &data_addr);

   struct nv_push *p = nvk_cmd_buffer_push(cmd, 10);

   P_MTHD(p, NV90B5, OFFSET_IN_UPPER);
   P_NV90B5_OFFSET_IN_UPPER(p, data_addr >> 32);
   P_NV90B5_OFFSET_IN_LOWER(p, data_addr & 0xffffffff);
   P_NV90B5_OFFSET_OUT_UPPER(p, dst_addr >> 32);
   P_NV90B5_OFFSET_OUT_LOWER(p, dst_addr & 0xffffffff);

   P_MTHD(p, NV90B5, LINE_LENGTH_IN);
   P_NV90B5_LINE_LENGTH_IN(p, dataSize);
   P_NV90B5_LINE_COUNT(p, 1);

   P_IMMD(p, NV90B5, LAUNCH_DMA, {
      .data_transfer_type = DATA_TRANSFER_TYPE_NON_PIPELINED,
      .multi_line_enable = MULTI_LINE_ENABLE_TRUE,
      .flush_enable = FLUSH_ENABLE_TRUE,
      .src_memory_layout = SRC_MEMORY_LAYOUT_PITCH,
      .dst_memory_layout = DST_MEMORY_LAYOUT_PITCH,
   });
}
