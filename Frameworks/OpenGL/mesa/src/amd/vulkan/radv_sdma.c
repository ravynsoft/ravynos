/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 * Copyright 2015-2021 Advanced Micro Devices, Inc.
 * Copyright 2023 Valve Corporation
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "radv_sdma.h"
#include "util/macros.h"
#include "util/u_memory.h"
#include "radv_cs.h"
#include "radv_private.h"

struct radv_sdma_chunked_copy_info {
   unsigned extent_horizontal_blocks;
   unsigned extent_vertical_blocks;
   unsigned aligned_row_pitch;
   unsigned num_rows_per_copy;
};

static const VkExtent3D radv_sdma_t2t_alignment_2d_and_planar[] = {
   {16, 16, 1}, /* 1 bpp */
   {16, 8, 1},  /* 2 bpp */
   {8, 8, 1},   /* 4 bpp */
   {8, 4, 1},   /* 8 bpp */
   {4, 4, 1},   /* 16 bpp */
};

static const VkExtent3D radv_sdma_t2t_alignment_3d[] = {
   {8, 4, 8}, /* 1 bpp */
   {4, 4, 8}, /* 2 bpp */
   {4, 4, 4}, /* 4 bpp */
   {4, 2, 4}, /* 8 bpp */
   {2, 2, 4}, /* 16 bpp */
};

ALWAYS_INLINE static unsigned
radv_sdma_pitch_alignment(const struct radv_device *device, const unsigned bpp)
{
   if (device->physical_device->rad_info.sdma_ip_version >= SDMA_5_0)
      return MAX2(1, 4 / bpp);

   return 4;
}

ALWAYS_INLINE static void
radv_sdma_check_pitches(const unsigned pitch, const unsigned slice_pitch, const unsigned bpp, const bool uses_depth)
{
   ASSERTED const unsigned pitch_alignment = MAX2(1, 4 / bpp);
   assert(pitch);
   assert(pitch <= (1 << 14));
   assert(radv_is_aligned(pitch, pitch_alignment));

   if (uses_depth) {
      ASSERTED const unsigned slice_pitch_alignment = 4;
      assert(slice_pitch);
      assert(slice_pitch <= (1 << 28));
      assert(radv_is_aligned(slice_pitch, slice_pitch_alignment));
   }
}

ALWAYS_INLINE static enum gfx9_resource_type
radv_sdma_surface_resource_type(const struct radv_device *const device, const struct radeon_surf *const surf)
{
   if (device->physical_device->rad_info.sdma_ip_version >= SDMA_5_0) {
      /* Use the 2D resource type for rotated or Z swizzles. */
      if ((surf->u.gfx9.resource_type == RADEON_RESOURCE_1D || surf->u.gfx9.resource_type == RADEON_RESOURCE_3D) &&
          (surf->micro_tile_mode == RADEON_MICRO_MODE_RENDER || surf->micro_tile_mode == RADEON_MICRO_MODE_DEPTH))
         return RADEON_RESOURCE_2D;
   }

   return surf->u.gfx9.resource_type;
}

ALWAYS_INLINE static uint32_t
radv_sdma_surface_type_from_aspect_mask(const VkImageAspectFlags aspectMask)
{
   if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
      return 1;
   else if (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
      return 2;

   return 0;
}

ALWAYS_INLINE static VkExtent3D
radv_sdma_pixel_extent_to_blocks(const VkExtent3D extent, const unsigned blk_w, const unsigned blk_h)
{
   const VkExtent3D r = {
      .width = DIV_ROUND_UP(extent.width, blk_w),
      .height = DIV_ROUND_UP(extent.height, blk_h),
      .depth = extent.depth,
   };

   return r;
}

ALWAYS_INLINE static VkOffset3D
radv_sdma_pixel_offset_to_blocks(const VkOffset3D offset, const unsigned blk_w, const unsigned blk_h)
{
   const VkOffset3D r = {
      .x = DIV_ROUND_UP(offset.x, blk_w),
      .y = DIV_ROUND_UP(offset.y, blk_h),
      .z = offset.z,
   };

   return r;
}

ALWAYS_INLINE static unsigned
radv_sdma_pixels_to_blocks(const unsigned linear_pitch, const unsigned blk_w)
{
   return DIV_ROUND_UP(linear_pitch, blk_w);
}

ALWAYS_INLINE static unsigned
radv_sdma_pixel_area_to_blocks(const unsigned linear_slice_pitch, const unsigned blk_w, const unsigned blk_h)
{
   return DIV_ROUND_UP(DIV_ROUND_UP(linear_slice_pitch, blk_w), blk_h);
}

static struct radv_sdma_chunked_copy_info
radv_sdma_get_chunked_copy_info(const struct radv_device *const device, const struct radv_sdma_surf *const img,
                                const VkExtent3D extent)
{
   const unsigned extent_horizontal_blocks = DIV_ROUND_UP(extent.width, img->blk_w);
   const unsigned extent_vertical_blocks = DIV_ROUND_UP(extent.height, img->blk_h);
   const unsigned aligned_row_pitch = ALIGN(extent_horizontal_blocks, 4);
   const unsigned aligned_row_bytes = aligned_row_pitch * img->bpp;

   /* Assume that we can always copy at least one full row at a time. */
   const unsigned max_num_rows_per_copy = MIN2(RADV_SDMA_TRANSFER_TEMP_BYTES / aligned_row_bytes, extent.height);
   assert(max_num_rows_per_copy);

   /* Ensure that the number of rows copied at a time is a power of two. */
   const unsigned num_rows_per_copy = MAX2(1, util_next_power_of_two(max_num_rows_per_copy + 1) / 2);

   const struct radv_sdma_chunked_copy_info r = {
      .extent_horizontal_blocks = extent_horizontal_blocks,
      .extent_vertical_blocks = extent_vertical_blocks,
      .aligned_row_pitch = aligned_row_pitch,
      .num_rows_per_copy = num_rows_per_copy,
   };

   return r;
}

struct radv_sdma_surf
radv_sdma_get_buf_surf(const struct radv_buffer *const buffer, const struct radv_image *const image,
                       const VkBufferImageCopy2 *const region, const VkImageAspectFlags aspect_mask)
{
   assert(util_bitcount(aspect_mask) == 1);

   const unsigned pitch = (region->bufferRowLength ? region->bufferRowLength : region->imageExtent.width);
   const unsigned slice_pitch =
      (region->bufferImageHeight ? region->bufferImageHeight : region->imageExtent.height) * pitch;

   const unsigned plane_idx = radv_plane_from_aspect(region->imageSubresource.aspectMask);
   const struct radeon_surf *surf = &image->planes[plane_idx].surface;
   const struct radv_sdma_surf info = {
      .va = radv_buffer_get_va(buffer->bo) + buffer->offset + region->bufferOffset,
      .pitch = pitch,
      .slice_pitch = slice_pitch,
      .bpp = surf->bpe,
      .blk_w = surf->blk_w,
      .blk_h = surf->blk_h,
      .is_linear = true,
   };

   return info;
}

static uint32_t
radv_sdma_get_metadata_config(const struct radv_device *const device, const struct radv_image *const image,
                              const struct radeon_surf *const surf, const VkImageSubresourceLayers subresource,
                              const VkImageAspectFlags aspect_mask)
{
   if (!device->physical_device->rad_info.sdma_supports_compression ||
       !(radv_dcc_enabled(image, subresource.mipLevel) || radv_image_has_htile(image))) {
      return 0;
   }

   const VkFormat format = vk_format_get_aspect_format(image->vk.format, aspect_mask);
   const struct util_format_description *desc = vk_format_description(format);

   const uint32_t data_format =
      ac_get_cb_format(device->physical_device->rad_info.gfx_level, vk_format_to_pipe_format(format));
   const uint32_t alpha_is_on_msb = vi_alpha_is_on_msb(device, format);
   const uint32_t number_type = radv_translate_buffer_numformat(desc, vk_format_get_first_non_void_channel(format));
   const uint32_t surface_type = radv_sdma_surface_type_from_aspect_mask(aspect_mask);
   const uint32_t max_comp_block_size = surf->u.gfx9.color.dcc.max_compressed_block_size;
   const uint32_t max_uncomp_block_size = radv_get_dcc_max_uncompressed_block_size(device, image);
   const uint32_t pipe_aligned = surf->u.gfx9.color.dcc.pipe_aligned;

   return data_format | alpha_is_on_msb << 8 | number_type << 9 | surface_type << 12 | max_comp_block_size << 24 |
          max_uncomp_block_size << 26 | pipe_aligned << 31;
}

static uint32_t
radv_sdma_get_tiled_info_dword(const struct radv_device *const device, const struct radv_image *const image,
                               const struct radeon_surf *const surf, const VkImageSubresourceLayers subresource)
{
   const uint32_t element_size = util_logbase2(surf->bpe);
   const uint32_t swizzle_mode = surf->has_stencil ? surf->u.gfx9.zs.stencil_swizzle_mode : surf->u.gfx9.swizzle_mode;
   const enum gfx9_resource_type dimension = radv_sdma_surface_resource_type(device, surf);
   const uint32_t info = element_size | swizzle_mode << 3 | dimension << 9;
   const enum sdma_version ver = device->physical_device->rad_info.sdma_ip_version;

   if (ver >= SDMA_5_0) {
      const uint32_t mip_max = MAX2(image->vk.mip_levels, 1);
      const uint32_t mip_id = subresource.mipLevel;

      return info | (mip_max - 1) << 16 | mip_id << 20;
   } else if (ver >= SDMA_4_0) {
      return info | surf->u.gfx9.epitch << 16;
   } else {
      unreachable("unsupported SDMA version");
   }
}

static uint32_t
radv_sdma_get_tiled_header_dword(const struct radv_device *const device, const struct radv_image *const image,
                                 const VkImageSubresourceLayers subresource)
{
   const enum sdma_version ver = device->physical_device->rad_info.sdma_ip_version;

   if (ver >= SDMA_5_0) {
      return 0;
   } else if (ver >= SDMA_4_0) {
      const uint32_t mip_max = MAX2(image->vk.mip_levels, 1);
      const uint32_t mip_id = subresource.mipLevel;
      return (mip_max - 1) << 20 | mip_id << 24;
   } else {
      unreachable("unsupported SDMA version");
   }
}

struct radv_sdma_surf
radv_sdma_get_surf(const struct radv_device *const device, const struct radv_image *const image,
                   const VkImageSubresourceLayers subresource, const VkOffset3D offset,
                   const VkImageAspectFlags aspect_mask)
{
   assert(util_bitcount(aspect_mask) == 1);

   const unsigned plane_idx = radv_plane_from_aspect(aspect_mask);
   const unsigned binding_idx = image->disjoint ? plane_idx : 0;
   const struct radv_image_binding *binding = &image->bindings[binding_idx];
   const struct radeon_surf *const surf = &image->planes[plane_idx].surface;
   struct radv_sdma_surf info = {
      .extent =
         {
            .width = vk_format_get_plane_width(image->vk.format, plane_idx, image->vk.extent.width),
            .height = vk_format_get_plane_height(image->vk.format, plane_idx, image->vk.extent.height),
            .depth = image->vk.image_type == VK_IMAGE_TYPE_3D ? image->vk.extent.depth : image->vk.array_layers,
         },
      .offset =
         {
            .x = offset.x,
            .y = offset.y,
            .z = image->vk.image_type == VK_IMAGE_TYPE_3D ? offset.z : subresource.baseArrayLayer,
         },
      .bpp = surf->bpe,
      .blk_w = surf->blk_w,
      .blk_h = surf->blk_h,
      .mip_levels = image->vk.mip_levels,
      .micro_tile_mode = surf->micro_tile_mode,
      .is_linear = surf->is_linear,
      .is_3d = surf->u.gfx9.resource_type == RADEON_RESOURCE_3D,
   };

   if (surf->is_linear) {
      info.va =
         binding->bo->va + binding->offset + surf->u.gfx9.surf_offset + surf->u.gfx9.offset[subresource.mipLevel];
      info.pitch = surf->u.gfx9.pitch[subresource.mipLevel];
      info.slice_pitch = surf->blk_w * surf->blk_h * surf->u.gfx9.surf_slice_size / surf->bpe;
   } else {
      /* 1D resources should be linear. */
      assert(surf->u.gfx9.resource_type != RADEON_RESOURCE_1D);

      info.va = (binding->bo->va + binding->offset + surf->u.gfx9.surf_offset) | surf->tile_swizzle << 8;
      info.info_dword = radv_sdma_get_tiled_info_dword(device, image, surf, subresource);
      info.header_dword = radv_sdma_get_tiled_header_dword(device, image, subresource);

      if (device->physical_device->rad_info.sdma_supports_compression &&
          (radv_dcc_enabled(image, subresource.mipLevel) || radv_image_has_htile(image))) {
         info.meta_va = binding->bo->va + binding->offset + surf->meta_offset;
         info.meta_config = radv_sdma_get_metadata_config(device, image, surf, subresource, aspect_mask);
      }
   }

   return info;
}

static void
radv_sdma_emit_nop(const struct radv_device *device, struct radeon_cmdbuf *cs)
{
   /* SDMA NOP acts as a fence command and causes the SDMA engine to wait for pending copy operations. */
   radeon_check_space(device->ws, cs, 1);
   radeon_emit(cs, SDMA_PACKET(SDMA_OPCODE_NOP, 0, 0));
}

void
radv_sdma_copy_buffer(const struct radv_device *device, struct radeon_cmdbuf *cs, uint64_t src_va, uint64_t dst_va,
                      uint64_t size)
{
   if (size == 0)
      return;

   const enum sdma_version ver = device->physical_device->rad_info.sdma_ip_version;
   const unsigned max_size_per_packet = ver >= SDMA_5_2 ? SDMA_V5_2_COPY_MAX_BYTES : SDMA_V2_0_COPY_MAX_BYTES;

   unsigned align = ~0u;
   unsigned ncopy = DIV_ROUND_UP(size, max_size_per_packet);

   assert(ver >= SDMA_2_0);

   /* SDMA FW automatically enables a faster dword copy mode when
    * source, destination and size are all dword-aligned.
    *
    * When source and destination are dword-aligned, round down the size to
    * take advantage of faster dword copy, and copy the remaining few bytes
    * with the last copy packet.
    */
   if ((src_va & 0x3) == 0 && (dst_va & 0x3) == 0 && size > 4 && (size & 0x3) != 0) {
      align = ~0x3u;
      ncopy++;
   }

   radeon_check_space(device->ws, cs, ncopy * 7);

   for (unsigned i = 0; i < ncopy; i++) {
      unsigned csize = size >= 4 ? MIN2(size & align, max_size_per_packet) : size;
      radeon_emit(cs, SDMA_PACKET(SDMA_OPCODE_COPY, SDMA_COPY_SUB_OPCODE_LINEAR, 0));
      radeon_emit(cs, ver >= SDMA_4_0 ? csize - 1 : csize);
      radeon_emit(cs, 0); /* src/dst endian swap */
      radeon_emit(cs, src_va);
      radeon_emit(cs, src_va >> 32);
      radeon_emit(cs, dst_va);
      radeon_emit(cs, dst_va >> 32);
      dst_va += csize;
      src_va += csize;
      size -= csize;
   }
}

void
radv_sdma_fill_buffer(const struct radv_device *device, struct radeon_cmdbuf *cs, const uint64_t va,
                      const uint64_t size, const uint32_t value)
{
   const uint32_t fill_size = 2; /* This means that the count is in dwords. */
   const uint32_t constant_fill_header = SDMA_PACKET(SDMA_OPCODE_CONSTANT_FILL, 0, 0) | (fill_size & 0x3) << 30;

   /* This packet is the same since SDMA v2.4, haven't bothered to check older versions. */
   const enum sdma_version ver = device->physical_device->rad_info.sdma_ip_version;
   assert(ver >= SDMA_2_4);

   /* Maximum allowed fill size depends on the GPU.
    * Emit as many packets as necessary to fill all the bytes we need.
    */
   const uint64_t max_fill_bytes = BITFIELD64_MASK(ver >= SDMA_6_0 ? 30 : 22) & ~0x3;
   const unsigned num_packets = DIV_ROUND_UP(size, max_fill_bytes);
   ASSERTED unsigned cdw_max = radeon_check_space(device->ws, cs, num_packets * 5);

   for (unsigned i = 0; i < num_packets; ++i) {
      const uint64_t offset = i * max_fill_bytes;
      const uint64_t fill_bytes = MIN2(size - offset, max_fill_bytes);
      const uint64_t fill_va = va + offset;

      radeon_emit(cs, constant_fill_header);
      radeon_emit(cs, fill_va);
      radeon_emit(cs, fill_va >> 32);
      radeon_emit(cs, value);
      radeon_emit(cs, fill_bytes - 1); /* Must be programmed in bytes, even if the fill is done in dwords. */
   }

   assert(cs->cdw <= cdw_max);
}

static void
radv_sdma_emit_copy_linear_sub_window(const struct radv_device *device, struct radeon_cmdbuf *cs,
                                      const struct radv_sdma_surf *const src, const struct radv_sdma_surf *const dst,
                                      const VkExtent3D pix_extent)
{
   /* This packet is the same since SDMA v2.4, haven't bothered to check older versions.
    * The main difference is the bitfield sizes:
    *
    * v2.4 - src/dst_pitch: 14 bits, rect_z: 11 bits
    * v4.0 - src/dst_pitch: 19 bits, rect_z: 11 bits
    * v5.0 - src/dst_pitch: 19 bits, rect_z: 13 bits
    *
    * We currently use the smallest limits (from SDMA v2.4).
    */

   const VkOffset3D src_off = radv_sdma_pixel_offset_to_blocks(src->offset, src->blk_w, src->blk_h);
   const VkOffset3D dst_off = radv_sdma_pixel_offset_to_blocks(dst->offset, dst->blk_w, dst->blk_h);
   const VkExtent3D ext = radv_sdma_pixel_extent_to_blocks(pix_extent, src->blk_w, src->blk_h);
   const unsigned src_pitch = radv_sdma_pixels_to_blocks(src->pitch, src->blk_w);
   const unsigned dst_pitch = radv_sdma_pixels_to_blocks(dst->pitch, dst->blk_w);
   const unsigned src_slice_pitch = radv_sdma_pixel_area_to_blocks(src->slice_pitch, src->blk_w, src->blk_h);
   const unsigned dst_slice_pitch = radv_sdma_pixel_area_to_blocks(dst->slice_pitch, dst->blk_w, dst->blk_h);

   assert(src->bpp == dst->bpp);
   assert(util_is_power_of_two_nonzero(src->bpp));
   radv_sdma_check_pitches(src->pitch, src->slice_pitch, src->bpp, false);
   radv_sdma_check_pitches(dst->pitch, dst->slice_pitch, dst->bpp, false);

   ASSERTED unsigned cdw_end = radeon_check_space(device->ws, cs, 13);

   radeon_emit(cs, SDMA_PACKET(SDMA_OPCODE_COPY, SDMA_COPY_SUB_OPCODE_LINEAR_SUB_WINDOW, 0) | util_logbase2(src->bpp)
                                                                                                 << 29);
   radeon_emit(cs, src->va);
   radeon_emit(cs, src->va >> 32);
   radeon_emit(cs, src_off.x | src_off.y << 16);
   radeon_emit(cs, src_off.z | (src_pitch - 1) << 13);
   radeon_emit(cs, src_slice_pitch - 1);
   radeon_emit(cs, dst->va);
   radeon_emit(cs, dst->va >> 32);
   radeon_emit(cs, dst_off.x | dst_off.y << 16);
   radeon_emit(cs, dst_off.z | (dst_pitch - 1) << 13);
   radeon_emit(cs, dst_slice_pitch - 1);
   radeon_emit(cs, (ext.width - 1) | (ext.height - 1) << 16);
   radeon_emit(cs, (ext.depth - 1));

   assert(cs->cdw == cdw_end);
}

static void
radv_sdma_emit_copy_tiled_sub_window(const struct radv_device *device, struct radeon_cmdbuf *cs,
                                     const struct radv_sdma_surf *const tiled,
                                     const struct radv_sdma_surf *const linear, const VkExtent3D pix_extent,
                                     const bool detile)
{
   if (!device->physical_device->rad_info.sdma_supports_compression) {
      assert(!tiled->meta_va);
   }

   const VkOffset3D linear_off = radv_sdma_pixel_offset_to_blocks(linear->offset, linear->blk_w, linear->blk_h);
   const VkOffset3D tiled_off = radv_sdma_pixel_offset_to_blocks(tiled->offset, tiled->blk_w, tiled->blk_h);
   const VkExtent3D tiled_ext = radv_sdma_pixel_extent_to_blocks(tiled->extent, tiled->blk_w, tiled->blk_h);
   const VkExtent3D ext = radv_sdma_pixel_extent_to_blocks(pix_extent, tiled->blk_w, tiled->blk_h);
   const unsigned linear_pitch = radv_sdma_pixels_to_blocks(linear->pitch, tiled->blk_w);
   const unsigned linear_slice_pitch = radv_sdma_pixel_area_to_blocks(linear->slice_pitch, tiled->blk_w, tiled->blk_h);
   const bool dcc = !!tiled->meta_va;
   const bool uses_depth = linear_off.z != 0 || tiled_off.z != 0 || ext.depth != 1;

   assert(util_is_power_of_two_nonzero(tiled->bpp));
   radv_sdma_check_pitches(linear_pitch, linear_slice_pitch, tiled->bpp, uses_depth);

   ASSERTED unsigned cdw_end = radeon_check_space(device->ws, cs, 14 + (dcc ? 3 : 0));

   radeon_emit(cs, SDMA_PACKET(SDMA_OPCODE_COPY, SDMA_COPY_SUB_OPCODE_TILED_SUB_WINDOW, 0) | dcc << 19 | detile << 31 |
                      tiled->header_dword);
   radeon_emit(cs, tiled->va);
   radeon_emit(cs, tiled->va >> 32);
   radeon_emit(cs, tiled_off.x | tiled_off.y << 16);
   radeon_emit(cs, tiled_off.z | (tiled_ext.width - 1) << 16);
   radeon_emit(cs, (tiled_ext.height - 1) | (tiled_ext.depth - 1) << 16);
   radeon_emit(cs, tiled->info_dword);
   radeon_emit(cs, linear->va);
   radeon_emit(cs, linear->va >> 32);
   radeon_emit(cs, linear_off.x | linear_off.y << 16);
   radeon_emit(cs, linear_off.z | (linear_pitch - 1) << 16);
   radeon_emit(cs, linear_slice_pitch - 1);
   radeon_emit(cs, (ext.width - 1) | (ext.height - 1) << 16);
   radeon_emit(cs, (ext.depth - 1));

   if (tiled->meta_va) {
      const unsigned write_compress_enable = !detile;
      radeon_emit(cs, tiled->meta_va);
      radeon_emit(cs, tiled->meta_va >> 32);
      radeon_emit(cs, tiled->meta_config | write_compress_enable << 28);
   }

   assert(cs->cdw == cdw_end);
}

static void
radv_sdma_emit_copy_t2t_sub_window(const struct radv_device *device, struct radeon_cmdbuf *cs,
                                   const struct radv_sdma_surf *const src, const struct radv_sdma_surf *const dst,
                                   const VkExtent3D px_extent)
{
   /* We currently only support the SDMA v4+ versions of this packet. */
   assert(device->physical_device->rad_info.sdma_ip_version >= SDMA_4_0);

   /* On GFX10+ this supports DCC, but cannot copy a compressed surface to another compressed surface. */
   assert(!src->meta_va || !dst->meta_va);

   if (device->physical_device->rad_info.sdma_ip_version >= SDMA_4_0 &&
       device->physical_device->rad_info.sdma_ip_version < SDMA_5_0) {
      /* SDMA v4 doesn't support mip_id selection in the T2T copy packet. */
      assert(src->header_dword >> 24 == 0);
      assert(dst->header_dword >> 24 == 0);
      /* SDMA v4 doesn't support any image metadata. */
      assert(!src->meta_va);
      assert(!dst->meta_va);
   }

   /* Despite the name, this can indicate DCC or HTILE metadata. */
   const uint32_t dcc = src->meta_va || dst->meta_va;
   /* 0 = compress (src is uncompressed), 1 = decompress (src is compressed). */
   const uint32_t dcc_dir = src->meta_va && !dst->meta_va;

   const VkOffset3D src_off = radv_sdma_pixel_offset_to_blocks(src->offset, src->blk_w, src->blk_h);
   const VkOffset3D dst_off = radv_sdma_pixel_offset_to_blocks(dst->offset, dst->blk_w, dst->blk_h);
   const VkExtent3D src_ext = radv_sdma_pixel_extent_to_blocks(src->extent, src->blk_w, src->blk_h);
   const VkExtent3D dst_ext = radv_sdma_pixel_extent_to_blocks(dst->extent, dst->blk_w, dst->blk_h);
   const VkExtent3D ext = radv_sdma_pixel_extent_to_blocks(px_extent, src->blk_w, src->blk_h);

   assert(util_is_power_of_two_nonzero(src->bpp));
   assert(util_is_power_of_two_nonzero(dst->bpp));

   ASSERTED unsigned cdw_end = radeon_check_space(device->ws, cs, 15 + (dcc ? 3 : 0));

   radeon_emit(cs, SDMA_PACKET(SDMA_OPCODE_COPY, SDMA_COPY_SUB_OPCODE_T2T_SUB_WINDOW, 0) | dcc << 19 | dcc_dir << 31 |
                      src->header_dword);
   radeon_emit(cs, src->va);
   radeon_emit(cs, src->va >> 32);
   radeon_emit(cs, src_off.x | src_off.y << 16);
   radeon_emit(cs, src_off.z | (src_ext.width - 1) << 16);
   radeon_emit(cs, (src_ext.height - 1) | (src_ext.depth - 1) << 16);
   radeon_emit(cs, src->info_dword);
   radeon_emit(cs, dst->va);
   radeon_emit(cs, dst->va >> 32);
   radeon_emit(cs, dst_off.x | dst_off.y << 16);
   radeon_emit(cs, dst_off.z | (dst_ext.width - 1) << 16);
   radeon_emit(cs, (dst_ext.height - 1) | (dst_ext.depth - 1) << 16);
   radeon_emit(cs, dst->info_dword);
   radeon_emit(cs, (ext.width - 1) | (ext.height - 1) << 16);
   radeon_emit(cs, (ext.depth - 1));

   if (dst->meta_va) {
      const uint32_t write_compress_enable = 1;
      radeon_emit(cs, dst->meta_va);
      radeon_emit(cs, dst->meta_va >> 32);
      radeon_emit(cs, dst->meta_config | write_compress_enable << 28);
   } else if (src->meta_va) {
      radeon_emit(cs, src->meta_va);
      radeon_emit(cs, src->meta_va >> 32);
      radeon_emit(cs, src->meta_config);
   }

   assert(cs->cdw == cdw_end);
}

void
radv_sdma_copy_buffer_image(const struct radv_device *device, struct radeon_cmdbuf *cs,
                            const struct radv_sdma_surf *buf, const struct radv_sdma_surf *img, const VkExtent3D extent,
                            bool to_image)
{
   if (img->is_linear) {
      if (to_image)
         radv_sdma_emit_copy_linear_sub_window(device, cs, buf, img, extent);
      else
         radv_sdma_emit_copy_linear_sub_window(device, cs, img, buf, extent);
   } else {
      radv_sdma_emit_copy_tiled_sub_window(device, cs, img, buf, extent, !to_image);
   }
}

bool
radv_sdma_use_unaligned_buffer_image_copy(const struct radv_device *device, const struct radv_sdma_surf *buf,
                                          const struct radv_sdma_surf *img, const VkExtent3D ext)
{
   const unsigned pitch_blocks = radv_sdma_pixels_to_blocks(buf->pitch, img->blk_w);
   if (!radv_is_aligned(pitch_blocks, radv_sdma_pitch_alignment(device, img->bpp)))
      return true;

   const bool uses_depth = img->offset.z != 0 || ext.depth != 1;
   if (!img->is_linear && uses_depth) {
      const unsigned slice_pitch_blocks = radv_sdma_pixel_area_to_blocks(buf->slice_pitch, img->blk_w, img->blk_h);
      if (!radv_is_aligned(slice_pitch_blocks, 4))
         return true;
   }

   return false;
}

void
radv_sdma_copy_buffer_image_unaligned(const struct radv_device *device, struct radeon_cmdbuf *cs,
                                      const struct radv_sdma_surf *buf, const struct radv_sdma_surf *img_in,
                                      const VkExtent3D base_extent, struct radeon_winsys_bo *temp_bo, bool to_image)
{
   const struct radv_sdma_chunked_copy_info info = radv_sdma_get_chunked_copy_info(device, img_in, base_extent);
   struct radv_sdma_surf img = *img_in;
   struct radv_sdma_surf tmp = {
      .va = temp_bo->va,
      .bpp = img.bpp,
      .blk_w = img.blk_w,
      .blk_h = img.blk_h,
      .pitch = info.aligned_row_pitch * img.blk_w,
      .slice_pitch = info.aligned_row_pitch * img.blk_w * info.extent_vertical_blocks * img.blk_h,
   };

   VkExtent3D extent = base_extent;
   const unsigned buf_pitch_blocks = DIV_ROUND_UP(buf->pitch, img.blk_w);
   const unsigned buf_slice_pitch_blocks = DIV_ROUND_UP(DIV_ROUND_UP(buf->slice_pitch, img.blk_w), img.blk_h);
   assert(buf_pitch_blocks);
   assert(buf_slice_pitch_blocks);
   extent.depth = 1;

   for (unsigned slice = 0; slice < base_extent.depth; ++slice) {
      for (unsigned row = 0; row < info.extent_vertical_blocks; row += info.num_rows_per_copy) {
         const unsigned rows = MIN2(info.extent_vertical_blocks - row, info.num_rows_per_copy);

         img.offset.y = img_in->offset.y + row * img.blk_h;
         img.offset.z = img_in->offset.z + slice;
         extent.height = rows * img.blk_h;
         tmp.slice_pitch = tmp.pitch * rows * img.blk_h;

         if (!to_image) {
            /* Copy the rows from the source image to the temporary buffer. */
            if (img.is_linear)
               radv_sdma_emit_copy_linear_sub_window(device, cs, &img, &tmp, extent);
            else
               radv_sdma_emit_copy_tiled_sub_window(device, cs, &img, &tmp, extent, true);

            /* Wait for the copy to finish. */
            radv_sdma_emit_nop(device, cs);
         }

         /* buffer to image: copy each row from source buffer to temporary buffer.
          * image to buffer: copy each row from temporary buffer to destination buffer.
          */
         for (unsigned r = 0; r < rows; ++r) {
            const uint64_t buf_va =
               buf->va + slice * buf_slice_pitch_blocks * img.bpp + (row + r) * buf_pitch_blocks * img.bpp;
            const uint64_t tmp_va = tmp.va + r * info.aligned_row_pitch * img.bpp;
            radv_sdma_copy_buffer(device, cs, to_image ? buf_va : tmp_va, to_image ? tmp_va : buf_va,
                                  info.extent_horizontal_blocks * img.bpp);
         }

         /* Wait for the copy to finish. */
         radv_sdma_emit_nop(device, cs);

         if (to_image) {
            /* Copy the rows from the temporary buffer to the destination image. */
            if (img.is_linear)
               radv_sdma_emit_copy_linear_sub_window(device, cs, &tmp, &img, extent);
            else
               radv_sdma_emit_copy_tiled_sub_window(device, cs, &img, &tmp, extent, false);

            /* Wait for the copy to finish. */
            radv_sdma_emit_nop(device, cs);
         }
      }
   }
}

void
radv_sdma_copy_image(const struct radv_device *device, struct radeon_cmdbuf *cs, const struct radv_sdma_surf *src,
                     const struct radv_sdma_surf *dst, const VkExtent3D extent)
{
   if (src->is_linear) {
      if (dst->is_linear) {
         radv_sdma_emit_copy_linear_sub_window(device, cs, src, dst, extent);
      } else {
         radv_sdma_emit_copy_tiled_sub_window(device, cs, dst, src, extent, false);
      }
   } else {
      if (dst->is_linear) {
         radv_sdma_emit_copy_tiled_sub_window(device, cs, src, dst, extent, true);
      } else {
         radv_sdma_emit_copy_t2t_sub_window(device, cs, src, dst, extent);
      }
   }
}

bool
radv_sdma_use_t2t_scanline_copy(const struct radv_device *device, const struct radv_sdma_surf *src,
                                const struct radv_sdma_surf *dst, const VkExtent3D extent)
{
   /* These need a linear-to-linear / linear-to-tiled copy. */
   if (src->is_linear || dst->is_linear)
      return false;

   /* SDMA can't do format conversion. */
   assert(src->bpp == dst->bpp);

   const enum sdma_version ver = device->physical_device->rad_info.sdma_ip_version;
   if (ver < SDMA_5_0) {
      /* SDMA v4.x and older doesn't support proper mip level selection. */
      if (src->mip_levels > 1 || dst->mip_levels > 1)
         return true;
   }

   /* The two images can have a different block size,
    * but must have the same swizzle mode.
    */
   if (src->micro_tile_mode != dst->micro_tile_mode)
      return true;

   /* The T2T subwindow copy packet only has fields for one metadata configuration.
    * It can either compress or decompress, or copy uncompressed images, but it
    * can't copy from a compressed image to another.
    */
   if (src->meta_va && dst->meta_va)
      return true;

   const bool needs_3d_alignment = src->is_3d && (src->micro_tile_mode == RADEON_MICRO_MODE_DISPLAY ||
                                                  src->micro_tile_mode == RADEON_MICRO_MODE_STANDARD);
   const unsigned log2bpp = util_logbase2(src->bpp);
   const VkExtent3D *const alignment =
      needs_3d_alignment ? &radv_sdma_t2t_alignment_3d[log2bpp] : &radv_sdma_t2t_alignment_2d_and_planar[log2bpp];

   const VkExtent3D copy_extent_blk = radv_sdma_pixel_extent_to_blocks(extent, src->blk_w, src->blk_h);
   const VkOffset3D src_offset_blk = radv_sdma_pixel_offset_to_blocks(src->offset, src->blk_w, src->blk_h);
   const VkOffset3D dst_offset_blk = radv_sdma_pixel_offset_to_blocks(dst->offset, dst->blk_w, dst->blk_h);

   if (!radv_is_aligned(copy_extent_blk.width, alignment->width) ||
       !radv_is_aligned(copy_extent_blk.height, alignment->height) ||
       !radv_is_aligned(copy_extent_blk.depth, alignment->depth))
      return true;

   if (!radv_is_aligned(src_offset_blk.x, alignment->width) || !radv_is_aligned(src_offset_blk.y, alignment->height) ||
       !radv_is_aligned(src_offset_blk.z, alignment->depth))
      return true;

   if (!radv_is_aligned(dst_offset_blk.x, alignment->width) || !radv_is_aligned(dst_offset_blk.y, alignment->height) ||
       !radv_is_aligned(dst_offset_blk.z, alignment->depth))
      return true;

   return false;
}

void
radv_sdma_copy_image_t2t_scanline(const struct radv_device *device, struct radeon_cmdbuf *cs,
                                  const struct radv_sdma_surf *src, const struct radv_sdma_surf *dst,
                                  const VkExtent3D extent, struct radeon_winsys_bo *temp_bo)
{
   const struct radv_sdma_chunked_copy_info info = radv_sdma_get_chunked_copy_info(device, src, extent);
   struct radv_sdma_surf t2l_src = *src;
   struct radv_sdma_surf t2l_dst = {
      .va = temp_bo->va,
      .bpp = src->bpp,
      .blk_w = src->blk_w,
      .blk_h = src->blk_h,
      .pitch = info.aligned_row_pitch * src->blk_w,
   };
   struct radv_sdma_surf l2t_dst = *dst;
   struct radv_sdma_surf l2t_src = {
      .va = temp_bo->va,
      .bpp = dst->bpp,
      .blk_w = dst->blk_w,
      .blk_h = dst->blk_h,
      .pitch = info.aligned_row_pitch * dst->blk_w,
   };

   for (unsigned slice = 0; slice < extent.depth; ++slice) {
      for (unsigned row = 0; row < info.extent_vertical_blocks; row += info.num_rows_per_copy) {
         const unsigned rows = MIN2(info.extent_vertical_blocks - row, info.num_rows_per_copy);

         const VkExtent3D t2l_extent = {
            .width = info.extent_horizontal_blocks * src->blk_w,
            .height = rows * src->blk_h,
            .depth = 1,
         };

         t2l_src.offset.y = src->offset.y + row * src->blk_h;
         t2l_src.offset.z = src->offset.z + slice;
         t2l_dst.slice_pitch = t2l_dst.pitch * t2l_extent.height;

         radv_sdma_emit_copy_tiled_sub_window(device, cs, &t2l_src, &t2l_dst, t2l_extent, true);
         radv_sdma_emit_nop(device, cs);

         const VkExtent3D l2t_extent = {
            .width = info.extent_horizontal_blocks * dst->blk_w,
            .height = rows * dst->blk_h,
            .depth = 1,
         };

         l2t_dst.offset.y = dst->offset.y + row * dst->blk_h;
         l2t_dst.offset.z = dst->offset.z + slice;
         l2t_src.slice_pitch = l2t_src.pitch * l2t_extent.height;

         radv_sdma_emit_copy_tiled_sub_window(device, cs, &l2t_dst, &l2t_src, l2t_extent, false);
         radv_sdma_emit_nop(device, cs);
      }
   }
}
