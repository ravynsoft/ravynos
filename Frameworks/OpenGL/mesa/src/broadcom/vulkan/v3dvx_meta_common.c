/*
 * Copyright © 2021 Raspberry Pi Ltd
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

#include "broadcom/common/v3d_macros.h"
#include "broadcom/common/v3d_tfu.h"
#include "broadcom/common/v3d_util.h"
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/compiler/v3d_compiler.h"

struct rcl_clear_info {
   const union v3dv_clear_value *clear_value;
   struct v3dv_image *image;
   VkImageAspectFlags aspects;
   uint32_t level;
};

static struct v3dv_cl *
emit_rcl_prologue(struct v3dv_job *job,
                  struct v3dv_meta_framebuffer *fb,
                  const struct rcl_clear_info *clear_info)
{
   const struct v3dv_frame_tiling *tiling = &job->frame_tiling;

   struct v3dv_cl *rcl = &job->rcl;
   v3dv_cl_ensure_space_with_branch(rcl, 200 +
                                    tiling->layers * 256 *
                                    cl_packet_length(SUPERTILE_COORDINATES));
   if (job->cmd_buffer->state.oom)
      return NULL;

   assert(!tiling->msaa || !tiling->double_buffer);
   cl_emit(rcl, TILE_RENDERING_MODE_CFG_COMMON, config) {
      config.early_z_disable = true;
      config.image_width_pixels = tiling->width;
      config.image_height_pixels = tiling->height;
      config.number_of_render_targets = 1;
      config.multisample_mode_4x = tiling->msaa;
      config.double_buffer_in_non_ms_mode = tiling->double_buffer;
#if V3D_VERSION == 42
      config.maximum_bpp_of_all_render_targets = tiling->internal_bpp;
#endif
#if V3D_VERSION >= 71
      config.log2_tile_width = log2_tile_size(tiling->tile_width);
      config.log2_tile_height = log2_tile_size(tiling->tile_height);
      /* FIXME: ideallly we would like next assert on the packet header (as is
       * general, so also applies to GL). We would need to expand
       * gen_pack_header for that.
       */
      assert(config.log2_tile_width == config.log2_tile_height ||
             config.log2_tile_width == config.log2_tile_height + 1);
#endif
      config.internal_depth_type = fb->internal_depth_type;
   }

   const uint32_t *color = NULL;
   if (clear_info && (clear_info->aspects & VK_IMAGE_ASPECT_COLOR_BIT)) {
      UNUSED uint32_t clear_pad = 0;
      if (clear_info->image) {
         const struct v3dv_image *image = clear_info->image;

         /* From vkCmdClearColorImage:
          *   "image must not use any of the formats that require a sampler
          *    YCBCR conversion"
          */
         assert(image->plane_count == 1);
         const struct v3d_resource_slice *slice =
            &image->planes[0].slices[clear_info->level];
         if (slice->tiling == V3D_TILING_UIF_NO_XOR ||
             slice->tiling == V3D_TILING_UIF_XOR) {
            int uif_block_height = v3d_utile_height(image->planes[0].cpp) * 2;

            uint32_t implicit_padded_height =
               align(tiling->height, uif_block_height) / uif_block_height;

            if (slice->padded_height_of_output_image_in_uif_blocks -
                implicit_padded_height >= 15) {
               clear_pad = slice->padded_height_of_output_image_in_uif_blocks;
            }
         }
      }

      color = &clear_info->clear_value->color[0];

#if V3D_VERSION == 42
      cl_emit(rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART1, clear) {
         clear.clear_color_low_32_bits = color[0];
         clear.clear_color_next_24_bits = color[1] & 0x00ffffff;
         clear.render_target_number = 0;
      };

      if (tiling->internal_bpp >= V3D_INTERNAL_BPP_64) {
         cl_emit(rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART2, clear) {
            clear.clear_color_mid_low_32_bits =
              ((color[1] >> 24) | (color[2] << 8));
            clear.clear_color_mid_high_24_bits =
              ((color[2] >> 24) | ((color[3] & 0xffff) << 8));
            clear.render_target_number = 0;
         };
      }

      if (tiling->internal_bpp >= V3D_INTERNAL_BPP_128 || clear_pad) {
         cl_emit(rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART3, clear) {
            clear.uif_padded_height_in_uif_blocks = clear_pad;
            clear.clear_color_high_16_bits = color[3] >> 16;
            clear.render_target_number = 0;
         };
      }
#endif
   }

#if V3D_VERSION == 42
   cl_emit(rcl, TILE_RENDERING_MODE_CFG_COLOR, rt) {
      rt.render_target_0_internal_bpp = tiling->internal_bpp;
      rt.render_target_0_internal_type = fb->internal_type;
      rt.render_target_0_clamp = V3D_RENDER_TARGET_CLAMP_NONE;
   }
#endif

#if V3D_VERSION >= 71
   cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1, rt) {
      if (color)
         rt.clear_color_low_bits = color[0];
      rt.internal_bpp = tiling->internal_bpp;
      rt.internal_type_and_clamping = v3dX(clamp_for_format_and_type)(fb->internal_type,
                                                                      fb->vk_format);
      rt.stride =
         v3d_compute_rt_row_row_stride_128_bits(tiling->tile_width,
                                                v3d_internal_bpp_words(rt.internal_bpp));
      rt.base_address = 0;
      rt.render_target_number = 0;
   }

   if (color && tiling->internal_bpp >= V3D_INTERNAL_BPP_64) {
      cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART2, rt) {
         rt.clear_color_mid_bits = /* 40 bits (32 + 8)  */
            ((uint64_t) color[1]) |
            (((uint64_t) (color[2] & 0xff)) << 32);
         rt.render_target_number = 0;
      }
   }

   if (color && tiling->internal_bpp >= V3D_INTERNAL_BPP_128) {
      cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART3, rt) {
         rt.clear_color_top_bits = /* 56 bits (24 + 32) */
            (((uint64_t) (color[2] & 0xffffff00)) >> 8) |
            (((uint64_t) (color[3])) << 24);
         rt.render_target_number = 0;
      }
   }
#endif

   cl_emit(rcl, TILE_RENDERING_MODE_CFG_ZS_CLEAR_VALUES, clear) {
      clear.z_clear_value = clear_info ? clear_info->clear_value->z : 1.0f;
      clear.stencil_clear_value = clear_info ? clear_info->clear_value->s : 0;
   };

   cl_emit(rcl, TILE_LIST_INITIAL_BLOCK_SIZE, init) {
      init.use_auto_chained_tile_lists = true;
      init.size_of_first_block_in_chained_tile_lists =
         TILE_ALLOCATION_BLOCK_SIZE_64B;
   }

   return rcl;
}

static void
emit_frame_setup(struct v3dv_job *job,
                 uint32_t min_layer,
                 const union v3dv_clear_value *clear_value)
{
   v3dv_return_if_oom(NULL, job);

   const struct v3dv_frame_tiling *tiling = &job->frame_tiling;

   struct v3dv_cl *rcl = &job->rcl;

   const uint32_t tile_alloc_offset =
      64 * min_layer * tiling->draw_tiles_x * tiling->draw_tiles_y;
   cl_emit(rcl, MULTICORE_RENDERING_TILE_LIST_SET_BASE, list) {
      list.address = v3dv_cl_address(job->tile_alloc, tile_alloc_offset);
   }

   cl_emit(rcl, MULTICORE_RENDERING_SUPERTILE_CFG, config) {
      config.number_of_bin_tile_lists = 1;
      config.total_frame_width_in_tiles = tiling->draw_tiles_x;
      config.total_frame_height_in_tiles = tiling->draw_tiles_y;

      config.supertile_width_in_tiles = tiling->supertile_width;
      config.supertile_height_in_tiles = tiling->supertile_height;

      config.total_frame_width_in_supertiles =
         tiling->frame_width_in_supertiles;
      config.total_frame_height_in_supertiles =
         tiling->frame_height_in_supertiles;
   }

   /* Implement GFXH-1742 workaround. Also, if we are clearing we have to do
    * it here.
    */
   for (int i = 0; i < 2; i++) {
      cl_emit(rcl, TILE_COORDINATES, coords);
      cl_emit(rcl, END_OF_LOADS, end);
      cl_emit(rcl, STORE_TILE_BUFFER_GENERAL, store) {
         store.buffer_to_store = NONE;
      }
      /* When using double-buffering, we need to clear both buffers (unless
       * we only have a single tile to render).
       */
      if (clear_value &&
          (i == 0 || v3dv_do_double_initial_tile_clear(tiling))) {
#if V3D_VERSION == 42
         cl_emit(rcl, CLEAR_TILE_BUFFERS, clear) {
            clear.clear_z_stencil_buffer = true;
            clear.clear_all_render_targets = true;
         }
#endif
#if V3D_VERSION >= 71
         cl_emit(rcl, CLEAR_RENDER_TARGETS, clear);
#endif
      }
      cl_emit(rcl, END_OF_TILE_MARKER, end);
   }

   cl_emit(rcl, FLUSH_VCD_CACHE, flush);
}

static void
emit_supertile_coordinates(struct v3dv_job *job,
                           struct v3dv_meta_framebuffer *framebuffer)
{
   v3dv_return_if_oom(NULL, job);

   struct v3dv_cl *rcl = &job->rcl;

   const uint32_t min_y = framebuffer->min_y_supertile;
   const uint32_t max_y = framebuffer->max_y_supertile;
   const uint32_t min_x = framebuffer->min_x_supertile;
   const uint32_t max_x = framebuffer->max_x_supertile;

   for (int y = min_y; y <= max_y; y++) {
      for (int x = min_x; x <= max_x; x++) {
         cl_emit(rcl, SUPERTILE_COORDINATES, coords) {
            coords.column_number_in_supertiles = x;
            coords.row_number_in_supertiles = y;
         }
      }
   }
}

static void
emit_linear_load(struct v3dv_cl *cl,
                 uint32_t buffer,
                 struct v3dv_bo *bo,
                 uint32_t offset,
                 uint32_t stride,
                 uint32_t format)
{
   cl_emit(cl, LOAD_TILE_BUFFER_GENERAL, load) {
      load.buffer_to_load = buffer;
      load.address = v3dv_cl_address(bo, offset);
      load.input_image_format = format;
      load.memory_format = V3D_TILING_RASTER;
      load.height_in_ub_or_stride = stride;
      load.decimate_mode = V3D_DECIMATE_MODE_SAMPLE_0;
   }
}

static void
emit_linear_store(struct v3dv_cl *cl,
                  uint32_t buffer,
                  struct v3dv_bo *bo,
                  uint32_t offset,
                  uint32_t stride,
                  bool msaa,
                  uint32_t format)
{
   cl_emit(cl, STORE_TILE_BUFFER_GENERAL, store) {
      store.buffer_to_store = RENDER_TARGET_0;
      store.address = v3dv_cl_address(bo, offset);
      store.clear_buffer_being_stored = false;
      store.output_image_format = format;
      store.memory_format = V3D_TILING_RASTER;
      store.height_in_ub_or_stride = stride;
      store.decimate_mode = msaa ? V3D_DECIMATE_MODE_ALL_SAMPLES :
                                   V3D_DECIMATE_MODE_SAMPLE_0;
   }
}

/* This chooses a tile buffer format that is appropriate for the copy operation.
 * Typically, this is the image render target type, however, if we are copying
 * depth/stencil to/from a buffer the hardware can't do raster loads/stores, so
 * we need to load and store to/from a tile color buffer using a compatible
 * color format.
 */
static uint32_t
choose_tlb_format(struct v3dv_meta_framebuffer *framebuffer,
                  VkImageAspectFlags aspect,
                  bool for_store,
                  bool is_copy_to_buffer,
                  bool is_copy_from_buffer)
{
   /* At this point the framebuffer was already lowered to single-plane */
   assert(framebuffer->format->plane_count == 1);

   if (is_copy_to_buffer || is_copy_from_buffer) {
      switch (framebuffer->vk_format) {
      case VK_FORMAT_D16_UNORM:
         return V3D_OUTPUT_IMAGE_FORMAT_R16UI;
      case VK_FORMAT_D32_SFLOAT:
         return V3D_OUTPUT_IMAGE_FORMAT_R32F;
      case VK_FORMAT_X8_D24_UNORM_PACK32:
         return V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI;
      case VK_FORMAT_D24_UNORM_S8_UINT:
         /* When storing the stencil aspect of a combined depth/stencil image
          * to a buffer, the Vulkan spec states that the output buffer must
          * have packed stencil values, so we choose an R8UI format for our
          * store outputs. For the load input we still want RGBA8UI since the
          * source image contains 4 channels (including the 3 channels
          * containing the 24-bit depth value).
          *
          * When loading the stencil aspect of a combined depth/stencil image
          * from a buffer, we read packed 8-bit stencil values from the buffer
          * that we need to put into the LSB of the 32-bit format (the R
          * channel), so we use R8UI. For the store, if we used R8UI then we
          * would write 8-bit stencil values consecutively over depth channels,
          * so we need to use RGBA8UI. This will write each stencil value in
          * its correct position, but will overwrite depth values (channels G
          * B,A) with undefined values. To fix this,  we will have to restore
          * the depth aspect from the Z tile buffer, which we should pre-load
          * from the image before the store).
          */
         if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
            return V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI;
         } else {
            assert(aspect & VK_IMAGE_ASPECT_STENCIL_BIT);
            if (is_copy_to_buffer) {
               return for_store ? V3D_OUTPUT_IMAGE_FORMAT_R8UI :
                                  V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI;
            } else {
               assert(is_copy_from_buffer);
               return for_store ? V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI :
                                  V3D_OUTPUT_IMAGE_FORMAT_R8UI;
            }
         }
      default: /* Color formats */
         return framebuffer->format->planes[0].rt_type;
         break;
      }
   } else {
      return framebuffer->format->planes[0].rt_type;
   }
}

static inline bool
format_needs_rb_swap(struct v3dv_device *device,
                     VkFormat format)
{
   /* We are calling these methods for framebuffer formats, that at this point
    * should be single-plane
    */
   assert(vk_format_get_plane_count(format) == 1);
   const uint8_t *swizzle = v3dv_get_format_swizzle(device, format, 0);
   return v3dv_format_swizzle_needs_rb_swap(swizzle);
}

static inline bool
format_needs_reverse(struct v3dv_device *device,
                     VkFormat format)
{
   /* We are calling these methods for framebuffer formats, that at this point
    * should be single-plane
    */
   assert(vk_format_get_plane_count(format) == 1);
   const uint8_t *swizzle = v3dv_get_format_swizzle(device, format, 0);
   return v3dv_format_swizzle_needs_reverse(swizzle);
}

static void
emit_image_load(struct v3dv_device *device,
                struct v3dv_cl *cl,
                struct v3dv_meta_framebuffer *framebuffer,
                struct v3dv_image *image,
                VkImageAspectFlags aspect,
                uint32_t layer,
                uint32_t mip_level,
                bool is_copy_to_buffer,
                bool is_copy_from_buffer)
{
   uint8_t plane = v3dv_plane_from_aspect(aspect);
   uint32_t layer_offset = v3dv_layer_offset(image, mip_level, layer, plane);

   /* For multi-plane formats we are copying plane by plane to the color
    * tlb. Framebuffer format was already selected to be a tlb single-plane
    * compatible format. We still need to use the real plane to get the
    * address etc from the source image.
    */
   assert(framebuffer->format->plane_count == 1);
   /* For image to/from buffer copies we always load to and store from RT0,
    * even for depth/stencil aspects, because the hardware can't do raster
    * stores or loads from/to the depth/stencil tile buffers.
    */
   bool load_to_color_tlb = is_copy_to_buffer || is_copy_from_buffer ||
                            image->format->plane_count > 1 ||
                            aspect == VK_IMAGE_ASPECT_COLOR_BIT;

   const struct v3d_resource_slice *slice = &image->planes[plane].slices[mip_level];
   cl_emit(cl, LOAD_TILE_BUFFER_GENERAL, load) {
      load.buffer_to_load = load_to_color_tlb ?
         RENDER_TARGET_0 : v3dX(zs_buffer_from_aspect_bits)(aspect);

      load.address = v3dv_cl_address(image->planes[plane].mem->bo, layer_offset);
      load.input_image_format = choose_tlb_format(framebuffer, aspect, false,
                                                  is_copy_to_buffer,
                                                  is_copy_from_buffer);
      load.memory_format = slice->tiling;

      /* When copying depth/stencil images to a buffer, for D24 formats Vulkan
       * expects the depth value in the LSB bits of each 32-bit pixel.
       * Unfortunately, the hardware seems to put the S8/X8 bits there and the
       * depth bits on the MSB. To work around that we can reverse the channel
       * order and then swap the R/B channels to get what we want.
       *
       * NOTE: reversing and swapping only gets us the behavior we want if the
       * operations happen in that exact order, which seems to be the case when
       * done on the tile buffer load operations. On the store, it seems the
       * order is not the same. The order on the store is probably reversed so
       * that reversing and swapping on both the load and the store preserves
       * the original order of the channels in memory.
       *
       * Notice that we only need to do this when copying to a buffer, where
       * depth and stencil aspects are copied as separate regions and
       * the spec expects them to be tightly packed.
       */
      bool needs_rb_swap = false;
      bool needs_chan_reverse = false;
      if (is_copy_to_buffer &&
         (framebuffer->vk_format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
          (framebuffer->vk_format == VK_FORMAT_D24_UNORM_S8_UINT &&
           (aspect & VK_IMAGE_ASPECT_DEPTH_BIT)))) {
         needs_rb_swap = true;
         needs_chan_reverse = true;
      } else if (!is_copy_from_buffer && !is_copy_to_buffer &&
                 (aspect & VK_IMAGE_ASPECT_COLOR_BIT)) {
         /* This is not a raw data copy (i.e. we are clearing the image),
          * so we need to make sure we respect the format swizzle.
          */
         needs_rb_swap = format_needs_rb_swap(device, framebuffer->vk_format);
         needs_chan_reverse = format_needs_reverse(device, framebuffer->vk_format);
      }

      load.r_b_swap = needs_rb_swap;
      load.channel_reverse = needs_chan_reverse;

      if (slice->tiling == V3D_TILING_UIF_NO_XOR ||
          slice->tiling == V3D_TILING_UIF_XOR) {
         load.height_in_ub_or_stride =
            slice->padded_height_of_output_image_in_uif_blocks;
      } else if (slice->tiling == V3D_TILING_RASTER) {
         load.height_in_ub_or_stride = slice->stride;
      }

      if (image->vk.samples > VK_SAMPLE_COUNT_1_BIT)
         load.decimate_mode = V3D_DECIMATE_MODE_ALL_SAMPLES;
      else
         load.decimate_mode = V3D_DECIMATE_MODE_SAMPLE_0;
   }
}

static void
emit_image_store(struct v3dv_device *device,
                 struct v3dv_cl *cl,
                 struct v3dv_meta_framebuffer *framebuffer,
                 struct v3dv_image *image,
                 VkImageAspectFlags aspect,
                 uint32_t layer,
                 uint32_t mip_level,
                 bool is_copy_to_buffer,
                 bool is_copy_from_buffer,
                 bool is_multisample_resolve)
{
   uint8_t plane = v3dv_plane_from_aspect(aspect);
   uint32_t layer_offset = v3dv_layer_offset(image, mip_level, layer, plane);

   /*
    * For multi-plane formats we are copying plane by plane to the color
    * tlb. Framebuffer format was already selected to be a tlb single-plane
    * compatible format. We still need to use the real plane to get the
    * address etc.
    */
   assert(framebuffer->format->plane_count == 1);

   bool store_from_color_tlb = is_copy_to_buffer || is_copy_from_buffer ||
                               image->format->plane_count > 1 ||
                               aspect == VK_IMAGE_ASPECT_COLOR_BIT;

   const struct v3d_resource_slice *slice = &image->planes[plane].slices[mip_level];
   cl_emit(cl, STORE_TILE_BUFFER_GENERAL, store) {
      store.buffer_to_store = store_from_color_tlb ?
         RENDER_TARGET_0 : v3dX(zs_buffer_from_aspect_bits)(aspect);

      store.address = v3dv_cl_address(image->planes[plane].mem->bo, layer_offset);

      store.clear_buffer_being_stored = false;

      /* See rationale in emit_image_load() */
      bool needs_rb_swap = false;
      bool needs_chan_reverse = false;
      if (is_copy_from_buffer &&
         (framebuffer->vk_format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
          (framebuffer->vk_format == VK_FORMAT_D24_UNORM_S8_UINT &&
           (aspect & VK_IMAGE_ASPECT_DEPTH_BIT)))) {
         needs_rb_swap = true;
         needs_chan_reverse = true;
      } else if (!is_copy_from_buffer && !is_copy_to_buffer &&
                 (aspect & VK_IMAGE_ASPECT_COLOR_BIT)) {
         needs_rb_swap = format_needs_rb_swap(device, framebuffer->vk_format);
         needs_chan_reverse = format_needs_reverse(device, framebuffer->vk_format);
      }

      store.r_b_swap = needs_rb_swap;
      store.channel_reverse = needs_chan_reverse;

      store.output_image_format = choose_tlb_format(framebuffer, aspect, true,
                                                    is_copy_to_buffer,
                                                    is_copy_from_buffer);
      store.memory_format = slice->tiling;
      if (slice->tiling == V3D_TILING_UIF_NO_XOR ||
          slice->tiling == V3D_TILING_UIF_XOR) {
         store.height_in_ub_or_stride =
            slice->padded_height_of_output_image_in_uif_blocks;
      } else if (slice->tiling == V3D_TILING_RASTER) {
         store.height_in_ub_or_stride = slice->stride;
      }

      if (image->vk.samples > VK_SAMPLE_COUNT_1_BIT)
         store.decimate_mode = V3D_DECIMATE_MODE_ALL_SAMPLES;
      else if (is_multisample_resolve)
         store.decimate_mode = V3D_DECIMATE_MODE_4X;
      else
         store.decimate_mode = V3D_DECIMATE_MODE_SAMPLE_0;
   }
}

static void
emit_copy_layer_to_buffer_per_tile_list(struct v3dv_job *job,
                                        struct v3dv_meta_framebuffer *framebuffer,
                                        struct v3dv_buffer *buffer,
                                        struct v3dv_image *image,
                                        uint32_t layer_offset,
                                        const VkBufferImageCopy2 *region)
{
   struct v3dv_cl *cl = &job->indirect;
   v3dv_cl_ensure_space(cl, 200, 1);
   v3dv_return_if_oom(NULL, job);

   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(cl);

   cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

   /* Load image to TLB */
   assert((image->vk.image_type != VK_IMAGE_TYPE_3D &&
           layer_offset < region->imageSubresource.layerCount) ||
          layer_offset < image->vk.extent.depth);

   const uint32_t image_layer = image->vk.image_type != VK_IMAGE_TYPE_3D ?
      region->imageSubresource.baseArrayLayer + layer_offset :
      region->imageOffset.z + layer_offset;

   emit_image_load(job->device, cl, framebuffer, image,
                   region->imageSubresource.aspectMask,
                   image_layer,
                   region->imageSubresource.mipLevel,
                   true, false);

   cl_emit(cl, END_OF_LOADS, end);

   cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   /* Store TLB to buffer */
   uint32_t width, height;
   if (region->bufferRowLength == 0)
      width = region->imageExtent.width;
   else
      width = region->bufferRowLength;

   if (region->bufferImageHeight == 0)
      height = region->imageExtent.height;
   else
      height = region->bufferImageHeight;

   /* Handle copy from compressed format */
   width = DIV_ROUND_UP(width, vk_format_get_blockwidth(image->vk.format));
   height = DIV_ROUND_UP(height, vk_format_get_blockheight(image->vk.format));

   /* If we are storing stencil from a combined depth/stencil format the
    * Vulkan spec states that the output buffer must have packed stencil
    * values, where each stencil value is 1 byte.
    */
   uint8_t plane = v3dv_plane_from_aspect(region->imageSubresource.aspectMask);
   uint32_t cpp =
      region->imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT ?
      1 : image->planes[plane].cpp;
   uint32_t buffer_stride = width * cpp;
   uint32_t buffer_offset = buffer->mem_offset + region->bufferOffset +
                            height * buffer_stride * layer_offset;

   uint32_t format = choose_tlb_format(framebuffer,
                                       region->imageSubresource.aspectMask,
                                       true, true, false);
   bool msaa = image->vk.samples > VK_SAMPLE_COUNT_1_BIT;

   emit_linear_store(cl, RENDER_TARGET_0, buffer->mem->bo,
                     buffer_offset, buffer_stride, msaa, format);

   cl_emit(cl, END_OF_TILE_MARKER, end);

   cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(cl);
   }
}

static void
emit_copy_layer_to_buffer(struct v3dv_job *job,
                          struct v3dv_buffer *buffer,
                          struct v3dv_image *image,
                          struct v3dv_meta_framebuffer *framebuffer,
                          uint32_t layer,
                          const VkBufferImageCopy2 *region)
{
   emit_copy_layer_to_buffer_per_tile_list(job, framebuffer, buffer,
                                           image, layer, region);
   emit_supertile_coordinates(job, framebuffer);
}

void
v3dX(meta_emit_copy_image_to_buffer_rcl)(struct v3dv_job *job,
                                         struct v3dv_buffer *buffer,
                                         struct v3dv_image *image,
                                         struct v3dv_meta_framebuffer *framebuffer,
                                         const VkBufferImageCopy2 *region)
{
   struct v3dv_cl *rcl = emit_rcl_prologue(job, framebuffer, NULL);
   v3dv_return_if_oom(NULL, job);

   emit_frame_setup(job, 0, NULL);
   for (int layer = 0; layer < job->frame_tiling.layers; layer++)
      emit_copy_layer_to_buffer(job, buffer, image, framebuffer, layer, region);
   cl_emit(rcl, END_OF_RENDERING, end);
}

static void
emit_resolve_image_layer_per_tile_list(struct v3dv_job *job,
                                       struct v3dv_meta_framebuffer *framebuffer,
                                       struct v3dv_image *dst,
                                       struct v3dv_image *src,
                                       uint32_t layer_offset,
                                       const VkImageResolve2 *region)
{
   struct v3dv_cl *cl = &job->indirect;
   v3dv_cl_ensure_space(cl, 200, 1);
   v3dv_return_if_oom(NULL, job);

   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(cl);

   cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

   assert((src->vk.image_type != VK_IMAGE_TYPE_3D &&
           layer_offset < region->srcSubresource.layerCount) ||
          layer_offset < src->vk.extent.depth);

   const uint32_t src_layer = src->vk.image_type != VK_IMAGE_TYPE_3D ?
      region->srcSubresource.baseArrayLayer + layer_offset :
      region->srcOffset.z + layer_offset;

   emit_image_load(job->device, cl, framebuffer, src,
                   region->srcSubresource.aspectMask,
                   src_layer,
                   region->srcSubresource.mipLevel,
                   false, false);

   cl_emit(cl, END_OF_LOADS, end);

   cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   assert((dst->vk.image_type != VK_IMAGE_TYPE_3D &&
           layer_offset < region->dstSubresource.layerCount) ||
          layer_offset < dst->vk.extent.depth);

   const uint32_t dst_layer = dst->vk.image_type != VK_IMAGE_TYPE_3D ?
      region->dstSubresource.baseArrayLayer + layer_offset :
      region->dstOffset.z + layer_offset;

   bool is_depth_or_stencil =
      region->dstSubresource.aspectMask &
      (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
   emit_image_store(job->device, cl, framebuffer, dst,
                    region->dstSubresource.aspectMask,
                    dst_layer,
                    region->dstSubresource.mipLevel,
                    false, false, !is_depth_or_stencil);

   cl_emit(cl, END_OF_TILE_MARKER, end);

   cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(cl);
   }
}

static void
emit_resolve_image_layer(struct v3dv_job *job,
                         struct v3dv_image *dst,
                         struct v3dv_image *src,
                         struct v3dv_meta_framebuffer *framebuffer,
                         uint32_t layer,
                         const VkImageResolve2 *region)
{
   emit_resolve_image_layer_per_tile_list(job, framebuffer,
                                          dst, src, layer, region);
   emit_supertile_coordinates(job, framebuffer);
}

void
v3dX(meta_emit_resolve_image_rcl)(struct v3dv_job *job,
                                  struct v3dv_image *dst,
                                  struct v3dv_image *src,
                                  struct v3dv_meta_framebuffer *framebuffer,
                                  const VkImageResolve2 *region)
{
   struct v3dv_cl *rcl = emit_rcl_prologue(job, framebuffer, NULL);
   v3dv_return_if_oom(NULL, job);

   emit_frame_setup(job, 0, NULL);
   for (int layer = 0; layer < job->frame_tiling.layers; layer++)
      emit_resolve_image_layer(job, dst, src, framebuffer, layer, region);
   cl_emit(rcl, END_OF_RENDERING, end);
}

static void
emit_copy_buffer_per_tile_list(struct v3dv_job *job,
                               struct v3dv_bo *dst,
                               struct v3dv_bo *src,
                               uint32_t dst_offset,
                               uint32_t src_offset,
                               uint32_t stride,
                               uint32_t format)
{
   struct v3dv_cl *cl = &job->indirect;
   v3dv_cl_ensure_space(cl, 200, 1);
   v3dv_return_if_oom(NULL, job);

   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(cl);

   cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

   emit_linear_load(cl, RENDER_TARGET_0, src, src_offset, stride, format);

   cl_emit(cl, END_OF_LOADS, end);

   cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   emit_linear_store(cl, RENDER_TARGET_0,
                     dst, dst_offset, stride, false, format);

   cl_emit(cl, END_OF_TILE_MARKER, end);

   cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(cl);
   }
}

void
v3dX(meta_emit_copy_buffer)(struct v3dv_job *job,
                            struct v3dv_bo *dst,
                            struct v3dv_bo *src,
                            uint32_t dst_offset,
                            uint32_t src_offset,
                            struct v3dv_meta_framebuffer *framebuffer,
                            uint32_t format,
                            uint32_t item_size)
{
   const uint32_t stride = job->frame_tiling.width * item_size;
   emit_copy_buffer_per_tile_list(job, dst, src,
                                  dst_offset, src_offset,
                                  stride, format);
   emit_supertile_coordinates(job, framebuffer);
}

void
v3dX(meta_emit_copy_buffer_rcl)(struct v3dv_job *job,
                                struct v3dv_bo *dst,
                                struct v3dv_bo *src,
                                uint32_t dst_offset,
                                uint32_t src_offset,
                                struct v3dv_meta_framebuffer *framebuffer,
                                uint32_t format,
                                uint32_t item_size)
{
   struct v3dv_cl *rcl = emit_rcl_prologue(job, framebuffer, NULL);
   v3dv_return_if_oom(NULL, job);

   emit_frame_setup(job, 0, NULL);

   v3dX(meta_emit_copy_buffer)(job, dst, src, dst_offset, src_offset,
                               framebuffer, format, item_size);

   cl_emit(rcl, END_OF_RENDERING, end);
}

static void
emit_copy_image_layer_per_tile_list(struct v3dv_job *job,
                                    struct v3dv_meta_framebuffer *framebuffer,
                                    struct v3dv_image *dst,
                                    struct v3dv_image *src,
                                    uint32_t layer_offset,
                                    const VkImageCopy2 *region)
{
   struct v3dv_cl *cl = &job->indirect;
   v3dv_cl_ensure_space(cl, 200, 1);
   v3dv_return_if_oom(NULL, job);

   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(cl);

   cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

   assert((src->vk.image_type != VK_IMAGE_TYPE_3D &&
           layer_offset < region->srcSubresource.layerCount) ||
          layer_offset < src->vk.extent.depth);

   const uint32_t src_layer = src->vk.image_type != VK_IMAGE_TYPE_3D ?
      region->srcSubresource.baseArrayLayer + layer_offset :
      region->srcOffset.z + layer_offset;

   emit_image_load(job->device, cl, framebuffer, src,
                   region->srcSubresource.aspectMask,
                   src_layer,
                   region->srcSubresource.mipLevel,
                   false, false);

   cl_emit(cl, END_OF_LOADS, end);

   cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   assert((dst->vk.image_type != VK_IMAGE_TYPE_3D &&
           layer_offset < region->dstSubresource.layerCount) ||
          layer_offset < dst->vk.extent.depth);

   const uint32_t dst_layer = dst->vk.image_type != VK_IMAGE_TYPE_3D ?
      region->dstSubresource.baseArrayLayer + layer_offset :
      region->dstOffset.z + layer_offset;

   emit_image_store(job->device, cl, framebuffer, dst,
                    region->dstSubresource.aspectMask,
                    dst_layer,
                    region->dstSubresource.mipLevel,
                    false, false, false);

   cl_emit(cl, END_OF_TILE_MARKER, end);

   cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(cl);
   }
}

static void
emit_copy_image_layer(struct v3dv_job *job,
                      struct v3dv_image *dst,
                      struct v3dv_image *src,
                      struct v3dv_meta_framebuffer *framebuffer,
                      uint32_t layer,
                      const VkImageCopy2 *region)
{
   emit_copy_image_layer_per_tile_list(job, framebuffer, dst, src, layer, region);
   emit_supertile_coordinates(job, framebuffer);
}

void
v3dX(meta_emit_copy_image_rcl)(struct v3dv_job *job,
                               struct v3dv_image *dst,
                               struct v3dv_image *src,
                               struct v3dv_meta_framebuffer *framebuffer,
                               const VkImageCopy2 *region)
{
   struct v3dv_cl *rcl = emit_rcl_prologue(job, framebuffer, NULL);
   v3dv_return_if_oom(NULL, job);

   emit_frame_setup(job, 0, NULL);
   for (int layer = 0; layer < job->frame_tiling.layers; layer++)
      emit_copy_image_layer(job, dst, src, framebuffer, layer, region);
   cl_emit(rcl, END_OF_RENDERING, end);
}

void
v3dX(meta_emit_tfu_job)(struct v3dv_cmd_buffer *cmd_buffer,
                        uint32_t dst_bo_handle,
                        uint32_t dst_offset,
                        enum v3d_tiling_mode dst_tiling,
                        uint32_t dst_padded_height_or_stride,
                        uint32_t dst_cpp,
                        uint32_t src_bo_handle,
                        uint32_t src_offset,
                        enum v3d_tiling_mode src_tiling,
                        uint32_t src_padded_height_or_stride,
                        uint32_t src_cpp,
                        uint32_t width,
                        uint32_t height,
                        const struct v3dv_format_plane *format_plane)
{
   struct drm_v3d_submit_tfu tfu = {
      .ios = (height << 16) | width,
      .bo_handles = {
         dst_bo_handle,
         src_bo_handle != dst_bo_handle ? src_bo_handle : 0
      },
   };

   tfu.iia |= src_offset;

#if V3D_VERSION <= 42
   if (src_tiling == V3D_TILING_RASTER) {
      tfu.icfg = V3D33_TFU_ICFG_FORMAT_RASTER << V3D33_TFU_ICFG_FORMAT_SHIFT;
   } else {
      tfu.icfg = (V3D33_TFU_ICFG_FORMAT_LINEARTILE +
                  (src_tiling - V3D_TILING_LINEARTILE)) <<
                   V3D33_TFU_ICFG_FORMAT_SHIFT;
   }
   tfu.icfg |= format_plane->tex_type << V3D33_TFU_ICFG_TTYPE_SHIFT;
#endif
#if V3D_VERSION >= 71
   if (src_tiling == V3D_TILING_RASTER) {
      tfu.icfg = V3D71_TFU_ICFG_FORMAT_RASTER << V3D71_TFU_ICFG_IFORMAT_SHIFT;
   } else {
      tfu.icfg = (V3D71_TFU_ICFG_FORMAT_LINEARTILE +
                  (src_tiling - V3D_TILING_LINEARTILE)) <<
                   V3D71_TFU_ICFG_IFORMAT_SHIFT;
   }
   tfu.icfg |= format_plane->tex_type << V3D71_TFU_ICFG_OTYPE_SHIFT;
#endif

   tfu.ioa = dst_offset;

#if V3D_VERSION <= 42
   tfu.ioa |= (V3D33_TFU_IOA_FORMAT_LINEARTILE +
               (dst_tiling - V3D_TILING_LINEARTILE)) <<
                V3D33_TFU_IOA_FORMAT_SHIFT;
#endif

#if V3D_VERSION >= 71
   tfu.v71.ioc = (V3D71_TFU_IOC_FORMAT_LINEARTILE +
                  (dst_tiling - V3D_TILING_LINEARTILE)) <<
                   V3D71_TFU_IOC_FORMAT_SHIFT;

   switch (dst_tiling) {
   case V3D_TILING_UIF_NO_XOR:
   case V3D_TILING_UIF_XOR:
      tfu.v71.ioc |=
         (dst_padded_height_or_stride / (2 * v3d_utile_height(dst_cpp))) <<
         V3D71_TFU_IOC_STRIDE_SHIFT;
      break;
   case V3D_TILING_RASTER:
      tfu.v71.ioc |= (dst_padded_height_or_stride / dst_cpp) <<
                      V3D71_TFU_IOC_STRIDE_SHIFT;
      break;
   default:
      break;
   }
#endif

   switch (src_tiling) {
   case V3D_TILING_UIF_NO_XOR:
   case V3D_TILING_UIF_XOR:
      tfu.iis |= src_padded_height_or_stride / (2 * v3d_utile_height(src_cpp));
      break;
   case V3D_TILING_RASTER:
      tfu.iis |= src_padded_height_or_stride / src_cpp;
      break;
   default:
      break;
   }

   /* The TFU can handle raster sources but always produces UIF results */
   assert(dst_tiling != V3D_TILING_RASTER);

#if V3D_VERSION <= 42
   /* If we're writing level 0 (!IOA_DIMTW), then we need to supply the
    * OPAD field for the destination (how many extra UIF blocks beyond
    * those necessary to cover the height).
    */
   if (dst_tiling == V3D_TILING_UIF_NO_XOR || dst_tiling == V3D_TILING_UIF_XOR) {
      uint32_t uif_block_h = 2 * v3d_utile_height(dst_cpp);
      uint32_t implicit_padded_height = align(height, uif_block_h);
      uint32_t icfg = (dst_padded_height_or_stride - implicit_padded_height) /
                      uif_block_h;
      tfu.icfg |= icfg << V3D33_TFU_ICFG_OPAD_SHIFT;
   }
#endif

   v3dv_cmd_buffer_add_tfu_job(cmd_buffer, &tfu);
}

static void
emit_clear_image_layer_per_tile_list(struct v3dv_job *job,
                                     struct v3dv_meta_framebuffer *framebuffer,
                                     struct v3dv_image *image,
                                     VkImageAspectFlags aspects,
                                     uint32_t layer,
                                     uint32_t level)
{
   struct v3dv_cl *cl = &job->indirect;
   v3dv_cl_ensure_space(cl, 200, 1);
   v3dv_return_if_oom(NULL, job);

   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(cl);

   cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

   cl_emit(cl, END_OF_LOADS, end);

   cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   emit_image_store(job->device, cl, framebuffer, image, aspects,
                    layer, level, false, false, false);

   cl_emit(cl, END_OF_TILE_MARKER, end);

   cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(cl);
   }
}

static void
emit_clear_image_layers(struct v3dv_job *job,
                 struct v3dv_image *image,
                 struct v3dv_meta_framebuffer *framebuffer,
                 VkImageAspectFlags aspects,
                 uint32_t min_layer,
                 uint32_t max_layer,
                 uint32_t level)
{
   for (uint32_t layer = min_layer; layer < max_layer; layer++) {
      emit_clear_image_layer_per_tile_list(job, framebuffer, image, aspects,
                                           layer, level);
      emit_supertile_coordinates(job, framebuffer);
   }
}

void
v3dX(meta_emit_clear_image_rcl)(struct v3dv_job *job,
                                struct v3dv_image *image,
                                struct v3dv_meta_framebuffer *framebuffer,
                                const union v3dv_clear_value *clear_value,
                                VkImageAspectFlags aspects,
                                uint32_t min_layer,
                                uint32_t max_layer,
                                uint32_t level)
{
   const struct rcl_clear_info clear_info = {
      .clear_value = clear_value,
      .image = image,
      .aspects = aspects,
      .level = level,
   };

   struct v3dv_cl *rcl = emit_rcl_prologue(job, framebuffer, &clear_info);
   v3dv_return_if_oom(NULL, job);

   emit_frame_setup(job, 0, clear_value);
   emit_clear_image_layers(job, image, framebuffer, aspects,
                           min_layer, max_layer, level);
   cl_emit(rcl, END_OF_RENDERING, end);
}

static void
emit_fill_buffer_per_tile_list(struct v3dv_job *job,
                               struct v3dv_bo *bo,
                               uint32_t offset,
                               uint32_t stride)
{
   struct v3dv_cl *cl = &job->indirect;
   v3dv_cl_ensure_space(cl, 200, 1);
   v3dv_return_if_oom(NULL, job);

   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(cl);

   cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

   cl_emit(cl, END_OF_LOADS, end);

   cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   emit_linear_store(cl, RENDER_TARGET_0, bo, offset, stride, false,
                     V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI);

   cl_emit(cl, END_OF_TILE_MARKER, end);

   cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(cl);
   }
}

static void
emit_fill_buffer(struct v3dv_job *job,
                 struct v3dv_bo *bo,
                 uint32_t offset,
                 struct v3dv_meta_framebuffer *framebuffer)
{
   const uint32_t stride = job->frame_tiling.width * 4;
   emit_fill_buffer_per_tile_list(job, bo, offset, stride);
   emit_supertile_coordinates(job, framebuffer);
}

void
v3dX(meta_emit_fill_buffer_rcl)(struct v3dv_job *job,
                                struct v3dv_bo *bo,
                                uint32_t offset,
                                struct v3dv_meta_framebuffer *framebuffer,
                                uint32_t data)
{
   const union v3dv_clear_value clear_value = {
       .color = { data, 0, 0, 0 },
   };

   const struct rcl_clear_info clear_info = {
      .clear_value = &clear_value,
      .image = NULL,
      .aspects = VK_IMAGE_ASPECT_COLOR_BIT,
      .level = 0,
   };

   struct v3dv_cl *rcl = emit_rcl_prologue(job, framebuffer, &clear_info);
   v3dv_return_if_oom(NULL, job);

   emit_frame_setup(job, 0, &clear_value);
   emit_fill_buffer(job, bo, offset, framebuffer);
   cl_emit(rcl, END_OF_RENDERING, end);
}


static void
emit_copy_buffer_to_layer_per_tile_list(struct v3dv_job *job,
                                        struct v3dv_meta_framebuffer *framebuffer,
                                        struct v3dv_image *image,
                                        struct v3dv_buffer *buffer,
                                        uint32_t layer,
                                        const VkBufferImageCopy2 *region)
{
   struct v3dv_cl *cl = &job->indirect;
   v3dv_cl_ensure_space(cl, 200, 1);
   v3dv_return_if_oom(NULL, job);

   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(cl);

   cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

   const VkImageSubresourceLayers *imgrsc = &region->imageSubresource;
   assert((image->vk.image_type != VK_IMAGE_TYPE_3D && layer < imgrsc->layerCount) ||
          layer < image->vk.extent.depth);

   /* Load TLB from buffer */
   uint32_t width, height;
   if (region->bufferRowLength == 0)
      width = region->imageExtent.width;
   else
      width = region->bufferRowLength;

   if (region->bufferImageHeight == 0)
      height = region->imageExtent.height;
   else
      height = region->bufferImageHeight;

   /* Handle copy to compressed format using a compatible format */
   width = DIV_ROUND_UP(width, vk_format_get_blockwidth(image->vk.format));
   height = DIV_ROUND_UP(height, vk_format_get_blockheight(image->vk.format));

   uint8_t plane = v3dv_plane_from_aspect(imgrsc->aspectMask);
   uint32_t cpp = imgrsc->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT ?
                  1 : image->planes[plane].cpp;
   uint32_t buffer_stride = width * cpp;
   uint32_t buffer_offset =
      buffer->mem_offset + region->bufferOffset + height * buffer_stride * layer;

   uint32_t format = choose_tlb_format(framebuffer, imgrsc->aspectMask,
                                       false, false, true);

   uint32_t image_layer = layer + (image->vk.image_type != VK_IMAGE_TYPE_3D ?
      imgrsc->baseArrayLayer : region->imageOffset.z);

   emit_linear_load(cl, RENDER_TARGET_0, buffer->mem->bo,
                    buffer_offset, buffer_stride, format);

   /* Because we can't do raster loads/stores of Z/S formats we need to
    * use a color tile buffer with a compatible RGBA color format instead.
    * However, when we are uploading a single aspect to a combined
    * depth/stencil image we have the problem that our tile buffer stores don't
    * allow us to mask out the other aspect, so we always write all four RGBA
    * channels to the image and we end up overwriting that other aspect with
    * undefined values. To work around that, we first load the aspect we are
    * not copying from the image memory into a proper Z/S tile buffer. Then we
    * do our store from the color buffer for the aspect we are copying, and
    * after that, we do another store from the Z/S tile buffer to restore the
    * other aspect to its original value.
    */
   if (framebuffer->vk_format == VK_FORMAT_D24_UNORM_S8_UINT) {
      if (imgrsc->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
         emit_image_load(job->device, cl, framebuffer, image,
                         VK_IMAGE_ASPECT_STENCIL_BIT,
                         image_layer, imgrsc->mipLevel,
                         false, false);
      } else {
         assert(imgrsc->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT);
         emit_image_load(job->device, cl, framebuffer, image,
                         VK_IMAGE_ASPECT_DEPTH_BIT,
                         image_layer, imgrsc->mipLevel,
                         false, false);
      }
   }

   cl_emit(cl, END_OF_LOADS, end);

   cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   /* Store TLB to image */
   emit_image_store(job->device, cl, framebuffer, image, imgrsc->aspectMask,
                    image_layer, imgrsc->mipLevel,
                    false, true, false);

   if (framebuffer->vk_format == VK_FORMAT_D24_UNORM_S8_UINT) {
      if (imgrsc->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
         emit_image_store(job->device, cl, framebuffer, image,
                          VK_IMAGE_ASPECT_STENCIL_BIT,
                          image_layer, imgrsc->mipLevel,
                          false, false, false);
      } else {
         assert(imgrsc->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT);
         emit_image_store(job->device, cl, framebuffer, image,
                          VK_IMAGE_ASPECT_DEPTH_BIT,
                          image_layer, imgrsc->mipLevel,
                          false, false, false);
      }
   }

   cl_emit(cl, END_OF_TILE_MARKER, end);

   cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(cl);
   }
}

static void
emit_copy_buffer_to_layer(struct v3dv_job *job,
                          struct v3dv_image *image,
                          struct v3dv_buffer *buffer,
                          struct v3dv_meta_framebuffer *framebuffer,
                          uint32_t layer,
                          const VkBufferImageCopy2 *region)
{
   emit_copy_buffer_to_layer_per_tile_list(job, framebuffer, image, buffer,
                                           layer, region);
   emit_supertile_coordinates(job, framebuffer);
}

void
v3dX(meta_emit_copy_buffer_to_image_rcl)(struct v3dv_job *job,
                                         struct v3dv_image *image,
                                         struct v3dv_buffer *buffer,
                                         struct v3dv_meta_framebuffer *framebuffer,
                                         const VkBufferImageCopy2 *region)
{
   struct v3dv_cl *rcl = emit_rcl_prologue(job, framebuffer, NULL);
   v3dv_return_if_oom(NULL, job);

   emit_frame_setup(job, 0, NULL);
   for (int layer = 0; layer < job->frame_tiling.layers; layer++)
      emit_copy_buffer_to_layer(job, image, buffer, framebuffer, layer, region);
   cl_emit(rcl, END_OF_RENDERING, end);
}

/* Figure out a TLB size configuration for a number of pixels to process.
 * Beware that we can't "render" more than MAX_DIMxMAX_DIM pixels in a single
 * job, if the pixel count is larger than this, the caller might need to split
 * the job and call this function multiple times.
 */
static void
framebuffer_size_for_pixel_count(uint32_t num_pixels,
                                 uint32_t *width,
                                 uint32_t *height)
{
   assert(num_pixels > 0);

   const uint32_t max_dim_pixels = V3D_MAX_IMAGE_DIMENSION;
   const uint32_t max_pixels = max_dim_pixels * max_dim_pixels;

   uint32_t w, h;
   if (num_pixels > max_pixels) {
      w = max_dim_pixels;
      h = max_dim_pixels;
   } else {
      w = num_pixels;
      h = 1;
      while (w > max_dim_pixels || ((w % 2) == 0 && w > 2 * h)) {
         w >>= 1;
         h <<= 1;
      }
   }
   assert(w <= max_dim_pixels && h <= max_dim_pixels);
   assert(w * h <= num_pixels);
   assert(w > 0 && h > 0);

   *width = w;
   *height = h;
}

struct v3dv_job *
v3dX(meta_copy_buffer)(struct v3dv_cmd_buffer *cmd_buffer,
                       struct v3dv_bo *dst,
                       uint32_t dst_offset,
                       struct v3dv_bo *src,
                       uint32_t src_offset,
                       const VkBufferCopy2 *region)
{
   const uint32_t internal_bpp = V3D_INTERNAL_BPP_32;
   const uint32_t internal_type = V3D_INTERNAL_TYPE_8UI;

   /* Select appropriate pixel format for the copy operation based on the
    * size to copy and the alignment of the source and destination offsets.
    */
   src_offset += region->srcOffset;
   dst_offset += region->dstOffset;
   uint32_t item_size = 4;
   while (item_size > 1 &&
          (src_offset % item_size != 0 || dst_offset % item_size != 0)) {
      item_size /= 2;
   }

   while (item_size > 1 && region->size % item_size != 0)
      item_size /= 2;

   assert(region->size % item_size == 0);
   uint32_t num_items = region->size / item_size;
   assert(num_items > 0);

   uint32_t format;
   VkFormat vk_format;
   switch (item_size) {
   case 4:
      format = V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI;
      vk_format = VK_FORMAT_R8G8B8A8_UINT;
      break;
   case 2:
      format = V3D_OUTPUT_IMAGE_FORMAT_RG8UI;
      vk_format = VK_FORMAT_R8G8_UINT;
      break;
   default:
      format = V3D_OUTPUT_IMAGE_FORMAT_R8UI;
      vk_format = VK_FORMAT_R8_UINT;
      break;
   }

   struct v3dv_job *job = NULL;
   while (num_items > 0) {
      job = v3dv_cmd_buffer_start_job(cmd_buffer, -1, V3DV_JOB_TYPE_GPU_CL);
      if (!job)
         return NULL;

      uint32_t width, height;
      framebuffer_size_for_pixel_count(num_items, &width, &height);

      v3dv_job_start_frame(job, width, height, 1, true, true, 1,
                           internal_bpp, 4 * v3d_internal_bpp_words(internal_bpp),
                           false);

      struct v3dv_meta_framebuffer framebuffer;
      v3dX(meta_framebuffer_init)(&framebuffer, vk_format, internal_type,
                                  &job->frame_tiling);

      v3dX(job_emit_binning_flush)(job);

      v3dX(meta_emit_copy_buffer_rcl)(job, dst, src, dst_offset, src_offset,
                                      &framebuffer, format, item_size);

      v3dv_cmd_buffer_finish_job(cmd_buffer);

      const uint32_t items_copied = width * height;
      const uint32_t bytes_copied = items_copied * item_size;
      num_items -= items_copied;
      src_offset += bytes_copied;
      dst_offset += bytes_copied;
   }

   return job;
}

void
v3dX(meta_fill_buffer)(struct v3dv_cmd_buffer *cmd_buffer,
                       struct v3dv_bo *bo,
                       uint32_t offset,
                       uint32_t size,
                       uint32_t data)
{
   assert(size > 0 && size % 4 == 0);
   assert(offset + size <= bo->size);

   const uint32_t internal_bpp = V3D_INTERNAL_BPP_32;
   const uint32_t internal_type = V3D_INTERNAL_TYPE_8UI;
   uint32_t num_items = size / 4;

   while (num_items > 0) {
      struct v3dv_job *job =
         v3dv_cmd_buffer_start_job(cmd_buffer, -1, V3DV_JOB_TYPE_GPU_CL);
      if (!job)
         return;

      uint32_t width, height;
      framebuffer_size_for_pixel_count(num_items, &width, &height);

      v3dv_job_start_frame(job, width, height, 1, true, true, 1,
                           internal_bpp, 4 * v3d_internal_bpp_words(internal_bpp),
                           false);

      struct v3dv_meta_framebuffer framebuffer;
      v3dX(meta_framebuffer_init)(&framebuffer, VK_FORMAT_R8G8B8A8_UINT,
                                  internal_type, &job->frame_tiling);

      v3dX(job_emit_binning_flush)(job);

      v3dX(meta_emit_fill_buffer_rcl)(job, bo, offset, &framebuffer, data);

      v3dv_cmd_buffer_finish_job(cmd_buffer);

      const uint32_t items_copied = width * height;
      const uint32_t bytes_copied = items_copied * 4;
      num_items -= items_copied;
      offset += bytes_copied;
   }
}

void
v3dX(meta_framebuffer_init)(struct v3dv_meta_framebuffer *fb,
                            VkFormat vk_format,
                            uint32_t internal_type,
                            const struct v3dv_frame_tiling *tiling)
{
   fb->internal_type = internal_type;

   /* Supertile coverage always starts at 0,0  */
   uint32_t supertile_w_in_pixels =
      tiling->tile_width * tiling->supertile_width;
   uint32_t supertile_h_in_pixels =
      tiling->tile_height * tiling->supertile_height;

   fb->min_x_supertile = 0;
   fb->min_y_supertile = 0;
   fb->max_x_supertile = (tiling->width - 1) / supertile_w_in_pixels;
   fb->max_y_supertile = (tiling->height - 1) / supertile_h_in_pixels;

   fb->vk_format = vk_format;
   fb->format = v3dX(get_format)(vk_format);

   fb->internal_depth_type = V3D_INTERNAL_TYPE_DEPTH_32F;
   if (vk_format_is_depth_or_stencil(vk_format))
      fb->internal_depth_type = v3dX(get_internal_depth_type)(vk_format);
}
