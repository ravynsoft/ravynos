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
#include <string.h>
#include <vulkan/vulkan.h>

#include "pvr_csb.h"
#include "pvr_csb_enum_helpers.h"
#include "pvr_formats.h"
#include "pvr_job_common.h"
#include "pvr_job_context.h"
#include "pvr_job_transfer.h"
#include "pvr_private.h"
#include "pvr_tex_state.h"
#include "pvr_transfer_frag_store.h"
#include "pvr_types.h"
#include "pvr_uscgen.h"
#include "pvr_util.h"
#include "pvr_winsys.h"
#include "util/bitscan.h"
#include "util/list.h"
#include "util/macros.h"
#include "util/u_math.h"
#include "util/xxhash.h"
#include "vk_format.h"
#include "vk_log.h"
#include "vk_sync.h"

#define PVR_TRANSFER_MAX_PASSES 10U
#define PVR_TRANSFER_MAX_CLIP_RECTS 4U
#define PVR_TRANSFER_MAX_PREPARES_PER_SUBMIT 16U
#define PVR_TRANSFER_MAX_CUSTOM_RECTS 3U

/* Number of triangles sent to the TSP per raster. */
#define PVR_TRANSFER_NUM_LAYERS 1U

#define PVR_MAX_WIDTH 16384
#define PVR_MAX_HEIGHT 16384

#define PVR_MAX_CLIP_SIZE(dev_info) \
   (PVR_HAS_FEATURE(dev_info, screen_size8K) ? 8192U : 16384U)

enum pvr_paired_tiles {
   PVR_PAIRED_TILES_NONE,
   PVR_PAIRED_TILES_X,
   PVR_PAIRED_TILES_Y
};

struct pvr_transfer_wa_source {
   uint32_t src_offset;
   uint32_t mapping_count;
   struct pvr_rect_mapping mappings[PVR_TRANSFER_MAX_CUSTOM_MAPPINGS];
   bool extend_height;
};

struct pvr_transfer_pass {
   uint32_t dst_offset;

   uint32_t source_count;
   struct pvr_transfer_wa_source sources[PVR_TRANSFER_MAX_SOURCES];

   uint32_t clip_rects_count;
   VkRect2D clip_rects[PVR_TRANSFER_MAX_CLIP_RECTS];
};

/* Structure representing a layer iteration. */
struct pvr_transfer_custom_mapping {
   bool double_stride;
   uint32_t texel_unwind_src;
   uint32_t texel_unwind_dst;
   uint32_t texel_extend_src;
   uint32_t texel_extend_dst;
   uint32_t pass_count;
   struct pvr_transfer_pass passes[PVR_TRANSFER_MAX_PASSES];
   uint32_t max_clip_rects;
   int32_t max_clip_size;
};

struct pvr_transfer_3d_iteration {
   uint32_t texture_coords[12];
};

struct pvr_transfer_3d_state {
   struct pvr_winsys_transfer_regs regs;

   bool empty_dst;
   bool down_scale;
   /* Write all channels present in the dst from the USC even if those are
    * constants.
    */
   bool dont_force_pbe;

   /* The rate of the shader. */
   uint32_t msaa_multiplier;
   /* Top left corner of the render in ISP tiles. */
   uint32_t origin_x_in_tiles;
   /* Top left corner of the render in ISP tiles. */
   uint32_t origin_y_in_tiles;
   /* Width of the render in ISP tiles. */
   uint32_t width_in_tiles;
   /* Height of the render in ISP tiles. */
   uint32_t height_in_tiles;

   /* Width of a sample in registers (pixel partition width). */
   uint32_t usc_pixel_width;

   /* Properties of the USC shader. */
   struct pvr_tq_shader_properties shader_props;

   /* TODO: Use pvr_dev_addr_t of an offset type for these. */
   uint32_t pds_shader_task_offset;
   uint32_t tex_state_data_offset;
   uint32_t uni_tex_code_offset;

   uint32_t uniform_data_size;
   uint32_t tex_state_data_size;
   uint32_t usc_coeff_regs;

   /* Pointer into the common store. */
   uint32_t common_ptr;
   /* Pointer into the dynamic constant reg buffer. */
   uint32_t dynamic_const_reg_ptr;
   /* Pointer into the USC constant reg buffer. */
   uint32_t usc_const_reg_ptr;

   uint32_t pds_coeff_task_offset;
   uint32_t coeff_data_size;

   /* Number of temporary 32bit registers used by PDS. */
   uint32_t pds_temps;

   struct pvr_transfer_custom_mapping custom_mapping;
   uint32_t pass_idx;

   enum pvr_filter filter[PVR_TRANSFER_MAX_SOURCES];
   bool custom_filter;

   enum pvr_paired_tiles pair_tiles;
};

struct pvr_transfer_prep_data {
   struct pvr_winsys_transfer_cmd_flags flags;
   struct pvr_transfer_3d_state state;
};

struct pvr_transfer_submit {
   uint32_t prep_count;
   struct pvr_transfer_prep_data
      prep_array[PVR_TRANSFER_MAX_PREPARES_PER_SUBMIT];
};

static enum pvr_transfer_pbe_pixel_src pvr_pbe_src_format_raw(VkFormat format)
{
   uint32_t bpp = vk_format_get_blocksizebits(format);

   if (bpp <= 32U)
      return PVR_TRANSFER_PBE_PIXEL_SRC_RAW32;
   else if (bpp <= 64U)
      return PVR_TRANSFER_PBE_PIXEL_SRC_RAW64;

   return PVR_TRANSFER_PBE_PIXEL_SRC_RAW128;
}

static VkResult pvr_pbe_src_format_pick_depth(
   const VkFormat src_format,
   const VkFormat dst_format,
   enum pvr_transfer_pbe_pixel_src *const src_format_out)
{
   if (dst_format != VK_FORMAT_D24_UNORM_S8_UINT)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   switch (src_format) {
   case VK_FORMAT_D24_UNORM_S8_UINT:
   case VK_FORMAT_X8_D24_UNORM_PACK32:
      *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D24S8_D24S8;
      break;

   case VK_FORMAT_D32_SFLOAT:
      *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32_D24S8;
      break;

   default:
      return VK_ERROR_FORMAT_NOT_SUPPORTED;
   }

   return VK_SUCCESS;
}

static VkResult pvr_pbe_src_format_pick_stencil(
   const VkFormat src_format,
   const VkFormat dst_format,
   enum pvr_transfer_pbe_pixel_src *const src_format_out)
{
   if ((src_format != VK_FORMAT_D24_UNORM_S8_UINT &&
        src_format != VK_FORMAT_S8_UINT) ||
       dst_format != VK_FORMAT_D24_UNORM_S8_UINT) {
      return VK_ERROR_FORMAT_NOT_SUPPORTED;
   }

   if (src_format == VK_FORMAT_S8_UINT)
      *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D24S8;
   else
      *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D24S8;

   return VK_SUCCESS;
}

static VkResult
pvr_pbe_src_format_ds(const struct pvr_transfer_cmd_surface *src,
                      const enum pvr_filter filter,
                      const VkFormat dst_format,
                      const uint32_t flags,
                      const bool down_scale,
                      enum pvr_transfer_pbe_pixel_src *src_format_out)
{
   const VkFormat src_format = src->vk_format;

   const bool src_depth = vk_format_has_depth(src_format);
   const bool dst_depth = vk_format_has_depth(dst_format);
   const bool src_stencil = vk_format_has_stencil(src_format);
   const bool dst_stencil = vk_format_has_stencil(dst_format);

   if (flags & PVR_TRANSFER_CMD_FLAGS_DSMERGE) {
      /* Merging, so destination should always have both. */
      if (!dst_depth || !dst_stencil)
         return VK_ERROR_FORMAT_NOT_SUPPORTED;

      if (flags & PVR_TRANSFER_CMD_FLAGS_PICKD) {
         return pvr_pbe_src_format_pick_depth(src_format,
                                              dst_format,
                                              src_format_out);
      } else {
         return pvr_pbe_src_format_pick_stencil(src_format,
                                                dst_format,
                                                src_format_out);
      }
   }

   /* We can't invent channels out of nowhere. */
   if ((dst_depth && !src_depth) || (dst_stencil && !src_stencil))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   switch (dst_format) {
   case VK_FORMAT_D16_UNORM:
      if (src_format == VK_FORMAT_D24_UNORM_S8_UINT)
         return VK_ERROR_FORMAT_NOT_SUPPORTED;

      if (!down_scale)
         *src_format_out = pvr_pbe_src_format_raw(dst_format);
      else
         *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_U16NORM;

      break;
   case VK_FORMAT_D24_UNORM_S8_UINT:
      switch (src_format) {
      case VK_FORMAT_D24_UNORM_S8_UINT:
         if (filter == PVR_FILTER_LINEAR)
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_D24S8;
         else
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_RAW32;

         break;

      /* D16_UNORM results in a 0.0->1.0 float from the TPU, the same as D32 */
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D32_SFLOAT:
         *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D32_D24S8;
         break;

      default:
         if (filter == PVR_FILTER_LINEAR)
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_D32S8;
         else
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_RAW64;
      }

      break;

   case VK_FORMAT_D32_SFLOAT:
      if (src_format == VK_FORMAT_D24_UNORM_S8_UINT)
         *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D24_D32;
      else
         *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_F32;

      break;

   default:
      if (src_format == VK_FORMAT_D24_UNORM_S8_UINT)
         *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_SWAP_LMSB;
      else
         *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_RAW32;
   }

   return VK_SUCCESS;
}

/**
 * How the PBE expects the output buffer for an RGBA space conversion.
 */
static VkResult
pvr_pbe_src_format_normal(VkFormat src_format,
                          VkFormat dst_format,
                          bool down_scale,
                          bool dont_force_pbe,
                          enum pvr_transfer_pbe_pixel_src *src_format_out)
{
   bool dst_signed = vk_format_is_sint(dst_format) ||
                     vk_format_is_snorm(dst_format);

   if (vk_format_is_int(dst_format)) {
      uint32_t red_width;
      bool src_signed;
      uint32_t count;

      if (!vk_format_is_int(src_format))
         return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);

      src_signed = vk_format_is_sint(src_format);

      red_width = vk_format_get_component_bits(dst_format,
                                               UTIL_FORMAT_COLORSPACE_RGB,
                                               0);

      switch (red_width) {
      case 8:
         if (!src_signed && !dst_signed)
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_UU8888;
         else if (src_signed && !dst_signed)
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_SU8888;
         else if (!src_signed && dst_signed)
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_US8888;
         else
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_SS8888;

         break;

      case 10:
         switch (dst_format) {
         case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            *src_format_out = src_signed ? PVR_TRANSFER_PBE_PIXEL_SRC_SU1010102
                                         : PVR_TRANSFER_PBE_PIXEL_SRC_UU1010102;
            break;

         case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            *src_format_out = src_signed
                                 ? PVR_TRANSFER_PBE_PIXEL_SRC_RBSWAP_SU1010102
                                 : PVR_TRANSFER_PBE_PIXEL_SRC_RBSWAP_UU1010102;
            break;

         default:
            return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);
         }
         break;

      case 16:
         if (!src_signed && !dst_signed)
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_UU16U16;
         else if (src_signed && !dst_signed)
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_SU16U16;
         else if (!src_signed && dst_signed)
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_US16S16;
         else
            *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_SS16S16;

         break;

      case 32:
         if (dont_force_pbe) {
            count = vk_format_get_blocksizebits(dst_format) / 32U;
         } else {
            count =
               vk_format_get_common_color_channel_count(src_format, dst_format);
         }

         if (!src_signed && !dst_signed) {
            *src_format_out = (count > 2U) ? PVR_TRANSFER_PBE_PIXEL_SRC_RAW128
                                           : PVR_TRANSFER_PBE_PIXEL_SRC_RAW64;
         } else if (src_signed && !dst_signed) {
            *src_format_out = (count > 2U) ? PVR_TRANSFER_PBE_PIXEL_SRC_S4XU32
                                           : PVR_TRANSFER_PBE_PIXEL_SRC_SU32U32;
         } else if (!src_signed && dst_signed) {
            *src_format_out = (count > 2U) ? PVR_TRANSFER_PBE_PIXEL_SRC_U4XS32
                                           : PVR_TRANSFER_PBE_PIXEL_SRC_US32S32;
         } else {
            *src_format_out = (count > 2U) ? PVR_TRANSFER_PBE_PIXEL_SRC_RAW128
                                           : PVR_TRANSFER_PBE_PIXEL_SRC_RAW64;
         }
         break;

      default:
         return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);
      }

   } else if (vk_format_is_float(dst_format) ||
              vk_format_is_normalized(dst_format)) {
      bool is_float = true;

      if (!vk_format_is_float(src_format) &&
          !vk_format_is_normalized(src_format) &&
          !vk_format_is_block_compressed(src_format)) {
         return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);
      }

      if (vk_format_is_normalized(dst_format)) {
         uint32_t chan_width;

         is_float = false;

         /* Alpha only. */
         switch (dst_format) {
         case VK_FORMAT_D16_UNORM:
            chan_width = 16;
            break;

         default:
            chan_width =
               vk_format_get_component_bits(dst_format,
                                            UTIL_FORMAT_COLORSPACE_RGB,
                                            0U);
            break;
         }

         if (src_format == dst_format) {
            switch (chan_width) {
            case 16U:
               if (down_scale) {
                  *src_format_out = dst_signed
                                       ? PVR_TRANSFER_PBE_PIXEL_SRC_S16NORM
                                       : PVR_TRANSFER_PBE_PIXEL_SRC_U16NORM;
               } else {
                  *src_format_out = dst_signed
                                       ? PVR_TRANSFER_PBE_PIXEL_SRC_SS16S16
                                       : PVR_TRANSFER_PBE_PIXEL_SRC_UU16U16;
               }
               break;

            case 32U:
               *src_format_out = pvr_pbe_src_format_raw(dst_format);
               break;
            default:
               is_float = true;
               break;
            }
         } else {
            switch (chan_width) {
            case 16U:
               *src_format_out = dst_signed
                                    ? PVR_TRANSFER_PBE_PIXEL_SRC_S16NORM
                                    : PVR_TRANSFER_PBE_PIXEL_SRC_U16NORM;
               break;
            default:
               is_float = true;
               break;
            }
         }
      }

      if (is_float) {
         if (vk_format_has_32bit_component(dst_format)) {
            uint32_t count;

            if (dont_force_pbe) {
               count = vk_format_get_blocksizebits(dst_format) / 32U;
            } else {
               count = vk_format_get_common_color_channel_count(src_format,
                                                                dst_format);
            }

            switch (count) {
            case 1U:
               *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_F32;
               break;
            case 2U:
               *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_F32X2;
               break;
            default:
               *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_F32X4;
               break;
            }
         } else {
            if (dst_format == VK_FORMAT_B8G8R8A8_UNORM ||
                dst_format == VK_FORMAT_R8G8B8A8_UNORM ||
                dst_format == VK_FORMAT_A8B8G8R8_UNORM_PACK32) {
               *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_F16_U8;
            } else {
               *src_format_out = PVR_TRANSFER_PBE_PIXEL_SRC_F16F16;
            }
         }
      }
   } else {
      return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);
   }

   return VK_SUCCESS;
}

static inline uint32_t
pvr_get_blit_flags(const struct pvr_transfer_cmd *transfer_cmd)
{
   return transfer_cmd->flags & PVR_TRANSFER_CMD_FLAGS_FAST2D
             ? 0
             : transfer_cmd->flags;
}

static VkResult pvr_pbe_src_format(struct pvr_transfer_cmd *transfer_cmd,
                                   struct pvr_transfer_3d_state *state,
                                   struct pvr_tq_shader_properties *prop)
{
   struct pvr_tq_layer_properties *layer = &prop->layer_props;
   const enum pvr_filter filter = transfer_cmd->source_count
                                     ? transfer_cmd->sources[0].filter
                                     : PVR_FILTER_POINT;
   const uint32_t flags = transfer_cmd->flags;
   VkFormat dst_format = transfer_cmd->dst.vk_format;
   const struct pvr_transfer_cmd_surface *src;
   VkFormat src_format;
   bool down_scale;

   if (transfer_cmd->source_count > 0) {
      src = &transfer_cmd->sources[0].surface;
      down_scale = transfer_cmd->sources[0].resolve_op == PVR_RESOLVE_BLEND &&
                   transfer_cmd->sources[0].surface.sample_count > 1U &&
                   transfer_cmd->dst.sample_count <= 1U;
   } else {
      src = &transfer_cmd->dst;
      down_scale = false;
   }

   src_format = src->vk_format;

   /* This has to come before the rest as S8 for instance is integer and
    * signedness check fails on D24S8.
    */
   if (vk_format_is_depth_or_stencil(src_format) ||
       vk_format_is_depth_or_stencil(dst_format) ||
       flags & PVR_TRANSFER_CMD_FLAGS_DSMERGE) {
      return pvr_pbe_src_format_ds(src,
                                   filter,
                                   dst_format,
                                   flags,
                                   down_scale,
                                   &layer->pbe_format);
   }

   return pvr_pbe_src_format_normal(src_format,
                                    dst_format,
                                    down_scale,
                                    state->dont_force_pbe,
                                    &layer->pbe_format);
}

static inline void pvr_setup_hwbg_object(const struct pvr_device_info *dev_info,
                                         struct pvr_transfer_3d_state *state)
{
   struct pvr_winsys_transfer_regs *regs = &state->regs;

   pvr_csb_pack (&regs->pds_bgnd0_base, CR_PDS_BGRND0_BASE, reg) {
      reg.shader_addr = PVR_DEV_ADDR(state->pds_shader_task_offset);
      assert(pvr_dev_addr_is_aligned(
         reg.shader_addr,
         PVRX(CR_PDS_BGRND0_BASE_SHADER_ADDR_ALIGNMENT)));
      reg.texunicode_addr = PVR_DEV_ADDR(state->uni_tex_code_offset);
      assert(pvr_dev_addr_is_aligned(
         reg.texunicode_addr,
         PVRX(CR_PDS_BGRND0_BASE_TEXUNICODE_ADDR_ALIGNMENT)));
   }

   pvr_csb_pack (&regs->pds_bgnd1_base, CR_PDS_BGRND1_BASE, reg) {
      reg.texturedata_addr = PVR_DEV_ADDR(state->tex_state_data_offset);
      assert(pvr_dev_addr_is_aligned(
         reg.texturedata_addr,
         PVRX(CR_PDS_BGRND1_BASE_TEXTUREDATA_ADDR_ALIGNMENT)));
   }

   /* BGRND 2 not needed, background object PDS doesn't use uniform program. */

   pvr_csb_pack (&regs->pds_bgnd3_sizeinfo, CR_PDS_BGRND3_SIZEINFO, reg) {
      reg.usc_sharedsize =
         DIV_ROUND_UP(state->common_ptr,
                      PVRX(CR_PDS_BGRND3_SIZEINFO_USC_SHAREDSIZE_UNIT_SIZE));

      assert(!(state->uniform_data_size &
               (PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_UNIFORMSIZE_UNIT_SIZE) - 1)));
      reg.pds_uniformsize =
         state->uniform_data_size /
         PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_UNIFORMSIZE_UNIT_SIZE);

      assert(
         !(state->tex_state_data_size &
           (PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_TEXTURESTATESIZE_UNIT_SIZE) - 1)));
      reg.pds_texturestatesize =
         state->tex_state_data_size /
         PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_TEXTURESTATESIZE_UNIT_SIZE);

      reg.pds_tempsize =
         DIV_ROUND_UP(state->pds_temps,
                      PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_TEMPSIZE_UNIT_SIZE));
   }
}

static inline bool
pvr_is_surface_aligned(pvr_dev_addr_t dev_addr, bool is_input, uint32_t bpp)
{
   /* 96 bpp is 32 bit granular. */
   if (bpp == 64U || bpp == 128U) {
      uint64_t mask = (uint64_t)((bpp >> 3U) - 1U);

      if ((dev_addr.addr & mask) != 0ULL)
         return false;
   }

   if (is_input) {
      if ((dev_addr.addr &
           (PVRX(TEXSTATE_STRIDE_IMAGE_WORD1_TEXADDR_ALIGNMENT) - 1U)) !=
          0ULL) {
         return false;
      }
   } else {
      if ((dev_addr.addr &
           (PVRX(PBESTATE_STATE_WORD0_ADDRESS_LOW_ALIGNMENT) - 1U)) != 0ULL) {
         return false;
      }
   }

   return true;
}

static inline VkResult
pvr_mem_layout_spec(const struct pvr_transfer_cmd_surface *surface,
                    uint32_t load,
                    bool is_input,
                    uint32_t *width_out,
                    uint32_t *height_out,
                    uint32_t *stride_out,
                    enum pvr_memlayout *mem_layout_out,
                    pvr_dev_addr_t *dev_addr_out)
{
   const uint32_t bpp = vk_format_get_blocksizebits(surface->vk_format);
   uint32_t unsigned_stride;

   *mem_layout_out = surface->mem_layout;
   *height_out = surface->height;
   *width_out = surface->width;
   *stride_out = surface->stride;
   *dev_addr_out = surface->dev_addr;

   if (surface->mem_layout != PVR_MEMLAYOUT_LINEAR &&
       !pvr_is_surface_aligned(*dev_addr_out, is_input, bpp)) {
      return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);
   }

   switch (surface->mem_layout) {
   case PVR_MEMLAYOUT_LINEAR:
      if (surface->stride == 0U)
         return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);

      unsigned_stride = *stride_out;

      if (!pvr_is_surface_aligned(*dev_addr_out, is_input, bpp))
         return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);

      if (unsigned_stride < *width_out)
         return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);

      if (!is_input) {
         if (unsigned_stride == 1U) {
            /* Change the setup to twiddling as that doesn't hit the stride
             * limit and twiddled == strided when 1px stride.
             */
            *mem_layout_out = PVR_MEMLAYOUT_TWIDDLED;
         }
      }

      *stride_out = unsigned_stride;
      break;

   case PVR_MEMLAYOUT_TWIDDLED:
   case PVR_MEMLAYOUT_3DTWIDDLED:
      /* Ignoring stride value for twiddled/tiled surface. */
      *stride_out = *width_out;
      break;

   default:
      return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);
   }

   return VK_SUCCESS;
}

static VkResult
pvr_pbe_setup_codegen_defaults(const struct pvr_device_info *dev_info,
                               const struct pvr_transfer_cmd *transfer_cmd,
                               struct pvr_transfer_3d_state *state,
                               struct pvr_pbe_surf_params *surface_params,
                               struct pvr_pbe_render_params *render_params)
{
   const struct pvr_transfer_cmd_surface *dst = &transfer_cmd->dst;
   const uint8_t *swizzle;
   VkFormat format;
   VkResult result;

   switch (dst->vk_format) {
   case VK_FORMAT_D24_UNORM_S8_UINT:
   case VK_FORMAT_X8_D24_UNORM_PACK32:
      format = VK_FORMAT_R32_UINT;
      break;

   default:
      format = dst->vk_format;
      break;
   }

   swizzle = pvr_get_format_swizzle(format);
   memcpy(surface_params->swizzle, swizzle, sizeof(surface_params->swizzle));

   pvr_pbe_get_src_format_and_gamma(format,
                                    PVR_PBE_GAMMA_NONE,
                                    false,
                                    &surface_params->source_format,
                                    &surface_params->gamma);

   surface_params->is_normalized = vk_format_is_normalized(format);
   surface_params->pbe_packmode = pvr_get_pbe_packmode(format);
   surface_params->nr_components = vk_format_get_nr_components(format);

   result = pvr_mem_layout_spec(dst,
                                0U,
                                false,
                                &surface_params->width,
                                &surface_params->height,
                                &surface_params->stride,
                                &surface_params->mem_layout,
                                &surface_params->addr);
   if (result != VK_SUCCESS)
      return result;

   surface_params->z_only_render = false;
   surface_params->depth = dst->depth;
   surface_params->down_scale = state->down_scale;

   if (surface_params->mem_layout == PVR_MEMLAYOUT_3DTWIDDLED)
      render_params->slice = (uint32_t)MAX2(dst->z_position, 0.0f);
   else
      render_params->slice = 0U;

   uint32_t tile_size_x = PVR_GET_FEATURE_VALUE(dev_info, tile_size_x, 0U);
   uint32_t tile_size_y = PVR_GET_FEATURE_VALUE(dev_info, tile_size_y, 0U);

   /* If the rectangle happens to be empty / off-screen we clip away
    * everything.
    */
   if (state->empty_dst) {
      render_params->min_x_clip = 2U * tile_size_x;
      render_params->max_x_clip = 3U * tile_size_x;
      render_params->min_y_clip = 2U * tile_size_y;
      render_params->max_y_clip = 3U * tile_size_y;
      state->origin_x_in_tiles = 0U;
      state->origin_y_in_tiles = 0U;
      state->height_in_tiles = 1U;
      state->width_in_tiles = 1U;
   } else {
      const VkRect2D *scissor = &transfer_cmd->scissor;

      /* Clamp */
      render_params->min_x_clip =
         MAX2(MIN2(scissor->offset.x, (int32_t)surface_params->width), 0U);
      render_params->max_x_clip =
         MAX2(MIN2(scissor->offset.x + scissor->extent.width,
                   (int32_t)surface_params->width),
              0U) -
         1U;

      render_params->min_y_clip =
         MAX2(MIN2(scissor->offset.y, surface_params->height), 0U);
      render_params->max_y_clip =
         MAX2(MIN2(scissor->offset.y + scissor->extent.height,
                   surface_params->height),
              0U) -
         1U;

      if (state->custom_mapping.pass_count > 0U) {
         struct pvr_transfer_pass *pass =
            &state->custom_mapping.passes[state->pass_idx];

         render_params->min_x_clip = (uint32_t)pass->clip_rects[0U].offset.x;
         render_params->max_x_clip =
            (uint32_t)(pass->clip_rects[0U].offset.x +
                       pass->clip_rects[0U].extent.width) -
            1U;
         render_params->min_y_clip = (uint32_t)pass->clip_rects[0U].offset.y;
         render_params->max_y_clip =
            (uint32_t)(pass->clip_rects[0U].offset.y +
                       pass->clip_rects[0U].extent.height) -
            1U;
      }

      state->origin_x_in_tiles = render_params->min_x_clip / tile_size_x;
      state->origin_y_in_tiles = render_params->min_y_clip / tile_size_y;
      state->width_in_tiles =
         (render_params->max_x_clip + tile_size_x) / tile_size_x;
      state->height_in_tiles =
         (render_params->max_y_clip + tile_size_y) / tile_size_y;

      /* Be careful here as this isn't the same as ((max_x_clip -
       * min_x_clip) + tile_size_x) >> tile_size_x.
       */
      state->width_in_tiles -= state->origin_x_in_tiles;
      state->height_in_tiles -= state->origin_y_in_tiles;
   }

   render_params->source_start = PVR_PBE_STARTPOS_BIT0;
   render_params->mrt_index = 0U;

   return VK_SUCCESS;
}

static VkResult
pvr_pbe_setup_modify_defaults(const struct pvr_transfer_cmd_surface *dst,
                              struct pvr_transfer_3d_state *state,
                              uint32_t rt_idx,
                              struct pvr_pbe_surf_params *surf_params,
                              struct pvr_pbe_render_params *render_params)
{
   struct pvr_transfer_pass *pass;
   VkRect2D *clip_rect;

   render_params->mrt_index = rt_idx;

   assert(rt_idx > 0 && rt_idx <= PVR_TRANSFER_MAX_RENDER_TARGETS);

   if (state->custom_mapping.pass_count == 0)
      return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);

   pass = &state->custom_mapping.passes[state->pass_idx];

   assert(rt_idx < PVR_TRANSFER_MAX_CUSTOM_RECTS);

   clip_rect = &pass->clip_rects[rt_idx];

   render_params->min_x_clip = (uint32_t)clip_rect->offset.x;
   render_params->max_x_clip =
      (uint32_t)clip_rect->offset.x + clip_rect->extent.width - 1U;
   render_params->min_y_clip = (uint32_t)clip_rect->offset.y;
   render_params->max_y_clip =
      (uint32_t)clip_rect->offset.y + clip_rect->extent.height - 1U;

   return VK_SUCCESS;
}

static uint32_t
pvr_pbe_get_pixel_size(enum pvr_transfer_pbe_pixel_src pixel_format)
{
   switch (pixel_format) {
   case PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D24_D32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D32_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_CONV_S8D24_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D24S8_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F16_U8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RAW32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RBSWAP_SU1010102:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RBSWAP_UU1010102:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SS8888:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SU1010102:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SU8888:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SWAP_LMSB:
   case PVR_TRANSFER_PBE_PIXEL_SRC_US8888:
   case PVR_TRANSFER_PBE_PIXEL_SRC_UU1010102:
   case PVR_TRANSFER_PBE_PIXEL_SRC_UU8888:
      return 1U;

   case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32S8_D32S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F16F16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32X2:
   case PVR_TRANSFER_PBE_PIXEL_SRC_MOV_BY45:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RAW64:
   case PVR_TRANSFER_PBE_PIXEL_SRC_S16NORM:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D32S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D32S8_D32S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D32S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SS16S16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SU16U16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SU32U32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_U16NORM:
   case PVR_TRANSFER_PBE_PIXEL_SRC_US16S16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_US32S32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_UU16U16:
      return 2U;

   case PVR_TRANSFER_PBE_PIXEL_SRC_F32X4:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RAW128:
   case PVR_TRANSFER_PBE_PIXEL_SRC_S4XU32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_U4XS32:
      return 4U;

   case PVR_TRANSFER_PBE_PIXEL_SRC_NUM:
   default:
      break;
   }

   return 0U;
}

static void pvr_pbe_setup_swizzle(const struct pvr_transfer_cmd *transfer_cmd,
                                  struct pvr_transfer_3d_state *state,
                                  struct pvr_pbe_surf_params *surf_params)
{
   bool color_fill = !!(transfer_cmd->flags & PVR_TRANSFER_CMD_FLAGS_FILL);
   const struct pvr_transfer_cmd_surface *dst = &transfer_cmd->dst;

   const uint32_t pixel_size =
      pvr_pbe_get_pixel_size(state->shader_props.layer_props.pbe_format);

   state->usc_pixel_width = MAX2(pixel_size, 1U);

   switch (dst->vk_format) {
   case VK_FORMAT_X8_D24_UNORM_PACK32:
   case VK_FORMAT_D24_UNORM_S8_UINT:
   case VK_FORMAT_S8_UINT:
      surf_params->swizzle[0U] = PIPE_SWIZZLE_X;
      surf_params->swizzle[1U] = PIPE_SWIZZLE_0;
      surf_params->swizzle[2U] = PIPE_SWIZZLE_0;
      surf_params->swizzle[3U] = PIPE_SWIZZLE_0;
      break;

   default: {
      const uint32_t red_width =
         vk_format_get_component_bits(dst->vk_format,
                                      UTIL_FORMAT_COLORSPACE_RGB,
                                      0U);

      if (transfer_cmd->source_count > 0 &&
          vk_format_is_alpha(dst->vk_format)) {
         if (vk_format_has_alpha(transfer_cmd->sources[0].surface.vk_format)) {
            /* Modify the destination format swizzle to always source from
             * src0.
             */
            surf_params->swizzle[0U] = PIPE_SWIZZLE_X;
            surf_params->swizzle[1U] = PIPE_SWIZZLE_0;
            surf_params->swizzle[2U] = PIPE_SWIZZLE_0;
            surf_params->swizzle[3U] = PIPE_SWIZZLE_1;
            break;
         }

         /* Source format having no alpha channel still allocates 4 output
          * buffer registers.
          */
      }

      if (vk_format_is_normalized(dst->vk_format)) {
         if (color_fill &&
             (dst->vk_format == VK_FORMAT_B8G8R8A8_UNORM ||
              dst->vk_format == VK_FORMAT_R8G8B8A8_UNORM ||
              dst->vk_format == VK_FORMAT_A8B8G8R8_UNORM_PACK32)) {
            surf_params->source_format =
               PVRX(PBESTATE_SOURCE_FORMAT_8_PER_CHANNEL);
         } else if (state->shader_props.layer_props.pbe_format ==
                    PVR_TRANSFER_PBE_PIXEL_SRC_F16_U8) {
            surf_params->source_format =
               PVRX(PBESTATE_SOURCE_FORMAT_8_PER_CHANNEL);
         } else if (red_width <= 8U) {
            surf_params->source_format =
               PVRX(PBESTATE_SOURCE_FORMAT_F16_PER_CHANNEL);
         }
      } else if (red_width == 32U && !state->dont_force_pbe) {
         uint32_t count = 0U;

         for (uint32_t i = 0; i < transfer_cmd->source_count; i++) {
            VkFormat src_format = transfer_cmd->sources[i].surface.vk_format;
            uint32_t tmp;

            tmp = vk_format_get_common_color_channel_count(src_format,
                                                           dst->vk_format);

            count = MAX2(count, tmp);
         }

         switch (count) {
         case 1U:
            surf_params->swizzle[1U] = PIPE_SWIZZLE_0;
            FALLTHROUGH;
         case 2U:
            surf_params->swizzle[2U] = PIPE_SWIZZLE_0;
            FALLTHROUGH;
         case 3U:
            surf_params->swizzle[3U] = PIPE_SWIZZLE_1;
            break;

         case 4U:
         default:
            break;
         }
      }
      break;
   }
   }
}

/**
 * Calculates the required PBE byte mask based on the incoming transfer command.
 *
 * @param transfer_cmd  the transfer command
 * @return the bytemask (active high disable mask)
 */

static uint64_t pvr_pbe_byte_mask(const struct pvr_device_info *dev_info,
                                  const struct pvr_transfer_cmd *transfer_cmd)
{
   uint32_t flags = pvr_get_blit_flags(transfer_cmd);

   assert(PVR_HAS_ERN(dev_info, 42064));

   if (flags & PVR_TRANSFER_CMD_FLAGS_DSMERGE) {
      uint32_t mask = 0U;

      switch (transfer_cmd->dst.vk_format) {
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
         mask = 0xF0F0F0F0U;
         break;
      case VK_FORMAT_D24_UNORM_S8_UINT:
         mask = 0x88888888U;
         break;
      default:
         break;
      }

      if ((flags & PVR_TRANSFER_CMD_FLAGS_PICKD) == 0U)
         mask = ~mask;

      return mask;
   }

   /* The mask is as it was inactive on cores without the ERN. This keeps the
    * firmware agnostic to the feature.
    */
   return 0U;
}

static VkResult pvr_pbe_setup_emit(const struct pvr_transfer_cmd *transfer_cmd,
                                   struct pvr_transfer_ctx *ctx,
                                   struct pvr_transfer_3d_state *state,
                                   uint32_t rt_count,
                                   uint32_t *pbe_setup_words)
{
   struct pvr_device *const device = ctx->device;
   const struct pvr_device_info *const dev_info = &device->pdevice->dev_info;

   struct pvr_winsys_transfer_regs *regs = &state->regs;
   struct pvr_pds_event_program program = {
      .emit_words = pbe_setup_words,
      .num_emit_word_pairs = rt_count,
   };
   struct pvr_pds_upload pds_upload;
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   pvr_dev_addr_t addr;
   VkResult result;

   /* Precondition, make sure to use a valid index for ctx->usc_eot_bos. */
   assert(rt_count <= ARRAY_SIZE(ctx->usc_eot_bos));
   assert(rt_count > 0U);

   addr.addr = ctx->usc_eot_bos[rt_count - 1U]->dev_addr.addr -
               device->heaps.usc_heap->base_addr.addr;

   pvr_pds_setup_doutu(&program.task_control,
                       addr.addr,
                       0U,
                       PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);

   pvr_pds_set_sizes_pixel_event(&program, dev_info);

   staging_buffer_size = PVR_DW_TO_BYTES(program.code_size + program.data_size);

   staging_buffer = vk_alloc(&device->vk.alloc,
                             staging_buffer_size,
                             8U,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pvr_pds_generate_pixel_event_data_segment(&program,
                                             staging_buffer,
                                             dev_info);

   /* TODO: We can save some memory by generating a code segment for each
    * rt_count, which at the time of writing is a maximum of 3, in
    * pvr_setup_transfer_eot_shaders() when we setup the corresponding EOT
    * USC programs.
    */
   pvr_pds_generate_pixel_event_code_segment(&program,
                                             staging_buffer + program.data_size,
                                             dev_info);

   result =
      pvr_cmd_buffer_upload_pds(transfer_cmd->cmd_buffer,
                                staging_buffer,
                                program.data_size,
                                PVRX(CR_EVENT_PIXEL_PDS_DATA_ADDR_ALIGNMENT),
                                staging_buffer + program.data_size,
                                program.code_size,
                                PVRX(CR_EVENT_PIXEL_PDS_CODE_ADDR_ALIGNMENT),
                                PVRX(CR_EVENT_PIXEL_PDS_DATA_ADDR_ALIGNMENT),
                                &pds_upload);
   vk_free(&device->vk.alloc, staging_buffer);
   if (result != VK_SUCCESS)
      return result;

   pvr_csb_pack (&regs->event_pixel_pds_info, CR_EVENT_PIXEL_PDS_INFO, reg) {
      reg.temp_stride = 0U;
      reg.const_size =
         DIV_ROUND_UP(program.data_size,
                      PVRX(CR_EVENT_PIXEL_PDS_INFO_CONST_SIZE_UNIT_SIZE));
      reg.usc_sr_size =
         DIV_ROUND_UP(rt_count * PVR_STATE_PBE_DWORDS,
                      PVRX(CR_EVENT_PIXEL_PDS_INFO_USC_SR_SIZE_UNIT_SIZE));
   }

   pvr_csb_pack (&regs->event_pixel_pds_data, CR_EVENT_PIXEL_PDS_DATA, reg) {
      reg.addr = PVR_DEV_ADDR(pds_upload.data_offset);
   }

   pvr_csb_pack (&regs->event_pixel_pds_code, CR_EVENT_PIXEL_PDS_CODE, reg) {
      reg.addr = PVR_DEV_ADDR(pds_upload.code_offset);
   }

   return VK_SUCCESS;
}

static VkResult pvr_pbe_setup(const struct pvr_transfer_cmd *transfer_cmd,
                              struct pvr_transfer_ctx *ctx,
                              struct pvr_transfer_3d_state *state)
{
   struct pvr_device *const device = ctx->device;
   const struct pvr_device_info *const dev_info = &device->pdevice->dev_info;

   const struct pvr_transfer_cmd_surface *dst = &transfer_cmd->dst;
   uint32_t num_rts = vk_format_get_plane_count(dst->vk_format);
   uint32_t pbe_setup_words[PVR_TRANSFER_MAX_RENDER_TARGETS *
                            ROGUE_NUM_PBESTATE_STATE_WORDS];
   struct pvr_pbe_render_params render_params;
   struct pvr_pbe_surf_params surf_params;
   VkResult result;

   if (state->custom_mapping.pass_count > 0U)
      num_rts = state->custom_mapping.passes[state->pass_idx].clip_rects_count;

   if (PVR_HAS_FEATURE(dev_info, paired_tiles))
      state->pair_tiles = PVR_PAIRED_TILES_NONE;

   for (uint32_t i = 0U; i < num_rts; i++) {
      uint64_t *pbe_regs;
      uint32_t *pbe_words;

      /* Ensure the access into the pbe_wordx_mrty is made within its bounds. */
      assert(i * ROGUE_NUM_PBESTATE_REG_WORDS_FOR_TRANSFER <
             ARRAY_SIZE(state->regs.pbe_wordx_mrty));
      /* Ensure the access into pbe_setup_words is made within its bounds. */
      assert(i * ROGUE_NUM_PBESTATE_STATE_WORDS < ARRAY_SIZE(pbe_setup_words));

      pbe_regs =
         &state->regs
             .pbe_wordx_mrty[i * ROGUE_NUM_PBESTATE_REG_WORDS_FOR_TRANSFER];
      pbe_words = &pbe_setup_words[i * ROGUE_NUM_PBESTATE_STATE_WORDS];

      if (PVR_HAS_ERN(dev_info, 42064))
         pbe_regs[2U] = 0UL;

      if (i == 0U) {
         result = pvr_pbe_setup_codegen_defaults(dev_info,
                                                 transfer_cmd,
                                                 state,
                                                 &surf_params,
                                                 &render_params);
         if (result != VK_SUCCESS)
            return result;
      } else {
         result = pvr_pbe_setup_modify_defaults(dst,
                                                state,
                                                i,
                                                &surf_params,
                                                &render_params);
         if (result != VK_SUCCESS)
            return result;
      }

      pvr_pbe_setup_swizzle(transfer_cmd, state, &surf_params);

      pvr_pbe_pack_state(dev_info,
                         &surf_params,
                         &render_params,
                         pbe_words,
                         pbe_regs);

      if (PVR_HAS_ERN(dev_info, 42064)) {
         uint64_t temp_reg;

         pvr_csb_pack (&temp_reg, PBESTATE_REG_WORD2, reg) {
            reg.sw_bytemask = pvr_pbe_byte_mask(dev_info, transfer_cmd);
         }

         pbe_regs[2U] |= temp_reg;
      }

      if (PVR_HAS_FEATURE(dev_info, paired_tiles)) {
         if (pbe_regs[2U] &
             (1ULL << PVRX(PBESTATE_REG_WORD2_PAIR_TILES_SHIFT))) {
            if (transfer_cmd->dst.mem_layout == PVR_MEMLAYOUT_TWIDDLED)
               state->pair_tiles = PVR_PAIRED_TILES_Y;
            else
               state->pair_tiles = PVR_PAIRED_TILES_X;
         }
      }
   }

   result =
      pvr_pbe_setup_emit(transfer_cmd, ctx, state, num_rts, pbe_setup_words);
   if (result != VK_SUCCESS)
      return result;

   /* Adjust tile origin and width to include all emits. */
   if (state->custom_mapping.pass_count > 0U) {
      const uint32_t tile_size_x =
         PVR_GET_FEATURE_VALUE(dev_info, tile_size_x, 0U);
      const uint32_t tile_size_y =
         PVR_GET_FEATURE_VALUE(dev_info, tile_size_y, 0U);
      struct pvr_transfer_pass *pass =
         &state->custom_mapping.passes[state->pass_idx];
      VkOffset2D offset = { 0U, 0U };
      VkOffset2D end = { 0U, 0U };

      for (uint32_t i = 0U; i < pass->clip_rects_count; i++) {
         VkRect2D *rect = &pass->clip_rects[i];

         offset.x = MIN2(offset.x, rect->offset.x);
         offset.y = MIN2(offset.y, rect->offset.y);
         end.x = MAX2(end.x, rect->offset.x + rect->extent.width);
         end.y = MAX2(end.y, rect->offset.y + rect->extent.height);
      }

      state->origin_x_in_tiles = (uint32_t)offset.x / tile_size_x;
      state->origin_y_in_tiles = (uint32_t)offset.y / tile_size_y;
      state->width_in_tiles =
         DIV_ROUND_UP((uint32_t)end.x, tile_size_x) - state->origin_x_in_tiles;
      state->height_in_tiles =
         DIV_ROUND_UP((uint32_t)end.y, tile_size_y) - state->origin_y_in_tiles;
   }

   return VK_SUCCESS;
}

/**
 * Writes the ISP tile registers according to the MSAA state. Sets up the USC
 * pixel partition allocations and the number of tiles in flight.
 */
static VkResult pvr_isp_tiles(const struct pvr_device *device,
                              struct pvr_transfer_3d_state *state)
{
   const struct pvr_device_runtime_info *dev_runtime_info =
      &device->pdevice->dev_runtime_info;
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   const uint32_t isp_samples =
      PVR_GET_FEATURE_VALUE(dev_info, isp_samples_per_pixel, 1U);
   uint32_t origin_x = state->origin_x_in_tiles;
   uint32_t origin_y = state->origin_y_in_tiles;
   uint32_t width = state->width_in_tiles;
   uint32_t height = state->height_in_tiles;
   uint32_t isp_tiles_in_flight;

   /* msaa_multiplier is calculated by sample_count & ~1U. Given sample
    * count is always in powers of two, we can get the sample count from
    * msaa_multiplier using the following logic.
    */
   const uint32_t samples = MAX2(state->msaa_multiplier, 1U);

   /* isp_samples_per_pixel feature is also know as "2x/4x for free", when
    * this is present SAMPLES_PER_PIXEL is 2/4, otherwise 1. The following
    * logic should end up with these numbers:
    *
    * |---------------------------------|
    * | 4 SAMPLES / ISP PIXEL           |
    * |-----------------------+----+----|
    * |                  MSAA | X* | Y* |
    * |                    2X |  1 |  1 |
    * |                    4X |  1 |  1 |
    * |---------------------------------|
    * | 2 SAMPLES / ISP PIXEL           |
    * |-----------------------+----+----|
    * |                  MSAA | X* | Y* |
    * |                    2X |  1 |  1 |
    * |                    4X |  1 |  2 |
    * |                    8X |  2 |  2 |
    * |-----------------------+----+----|
    * |  1 SAMPLE / ISP PIXEL           |
    * |-----------------------+----+----|
    * |                  MSAA | X* | Y* |
    * |                    2X |  1 |  2 |
    * |                    4X |  2 |  2 |
    * |-----------------------+----+----|
    */

   origin_x <<= (state->msaa_multiplier >> (isp_samples + 1U)) & 1U;
   origin_y <<= ((state->msaa_multiplier >> (isp_samples + 1U)) |
                 (state->msaa_multiplier >> isp_samples)) &
                1U;
   width <<= (state->msaa_multiplier >> (isp_samples + 1U)) & 1U;
   height <<= ((state->msaa_multiplier >> (isp_samples + 1U)) |
               (state->msaa_multiplier >> isp_samples)) &
              1U;

   if (PVR_HAS_FEATURE(dev_info, paired_tiles) &&
       state->pair_tiles != PVR_PAIRED_TILES_NONE) {
      width = ALIGN_POT(width, 2U);
      height = ALIGN_POT(height, 2U);
   }

   pvr_csb_pack (&state->regs.isp_mtile_size, CR_ISP_MTILE_SIZE, reg) {
      reg.x = width;
      reg.y = height;
   }

   pvr_csb_pack (&state->regs.isp_render_origin, CR_ISP_RENDER_ORIGIN, reg) {
      reg.x = origin_x;
      reg.y = origin_y;
   }

   pvr_setup_tiles_in_flight(dev_info,
                             dev_runtime_info,
                             pvr_cr_isp_aa_mode_type(samples),
                             state->usc_pixel_width,
                             state->pair_tiles != PVR_PAIRED_TILES_NONE,
                             0,
                             &isp_tiles_in_flight,
                             &state->regs.usc_pixel_output_ctrl);

   pvr_csb_pack (&state->regs.isp_ctl, CR_ISP_CTL, reg) {
      reg.process_empty_tiles = true;

      if (PVR_HAS_FEATURE(dev_info, paired_tiles)) {
         if (state->pair_tiles == PVR_PAIRED_TILES_X) {
            reg.pair_tiles = true;
         } else if (state->pair_tiles == PVR_PAIRED_TILES_Y) {
            reg.pair_tiles = true;
            reg.pair_tiles_vert = true;
         }
      }
   }

   state->regs.isp_ctl |= isp_tiles_in_flight;

   return VK_SUCCESS;
}

static bool
pvr_int_pbe_pixel_changes_dst_rate(const struct pvr_device_info *dev_info,
                                   enum pvr_transfer_pbe_pixel_src pbe_format)
{
   /* We don't emulate rate change from the USC with the pbe_yuv feature. */
   if (!PVR_HAS_FEATURE(dev_info, pbe_yuv) &&
       (pbe_format == PVR_TRANSFER_PBE_PIXEL_SRC_Y_UV_INTERLEAVED ||
        pbe_format == PVR_TRANSFER_PBE_PIXEL_SRC_Y_U_V)) {
      return true;
   }

   return false;
}

/**
 * Number of DWORDs from the unified store that floating texture coefficients
 * take up.
 */
static void pvr_uv_space(const struct pvr_device_info *dev_info,
                         const struct pvr_transfer_cmd *transfer_cmd,
                         struct pvr_transfer_3d_state *state)
{
   const struct pvr_transfer_cmd_surface *dst = &transfer_cmd->dst;
   const VkRect2D *dst_rect = &transfer_cmd->scissor;

   /* This also avoids division by 0 in pvr_dma_texture_floats(). */
   if (state->custom_mapping.pass_count == 0U &&
       (dst_rect->extent.width == 0U || dst_rect->extent.height == 0U ||
        MAX2(dst_rect->offset.x, dst_rect->offset.x + dst_rect->extent.width) <
           0U ||
        MIN2(dst_rect->offset.x, dst_rect->offset.x + dst_rect->extent.width) >
           (int32_t)dst->width ||
        MAX2(dst_rect->offset.y, dst_rect->offset.y + dst_rect->extent.height) <
           0U ||
        MIN2(dst_rect->offset.y, dst_rect->offset.y + dst_rect->extent.height) >
           (int32_t)dst->height)) {
      state->empty_dst = true;
   } else {
      state->empty_dst = false;

      if (transfer_cmd->source_count > 0) {
         struct pvr_tq_layer_properties *layer =
            &state->shader_props.layer_props;

         const VkRect2D *src_rect =
            &transfer_cmd->sources[0U].mappings[0U].src_rect;
         const VkRect2D *dst_rect =
            &transfer_cmd->sources[0U].mappings[0U].dst_rect;
         int32_t dst_x1 = dst_rect->offset.x + dst_rect->extent.width;
         int32_t dst_y1 = dst_rect->offset.y + dst_rect->extent.height;
         int32_t src_x1 = src_rect->offset.x + src_rect->extent.width;
         int32_t src_y1 = src_rect->offset.y + src_rect->extent.height;

         assert(transfer_cmd->source_count == 1);

         if (state->filter[0U] > PVR_FILTER_POINT) {
            layer->layer_floats = PVR_INT_COORD_SET_FLOATS_4;
         } else if (src_rect->extent.width == 0U ||
                    src_rect->extent.height == 0U) {
            layer->layer_floats = PVR_INT_COORD_SET_FLOATS_0;
         } else if ((src_rect->offset.x * dst_x1 !=
                     src_x1 * dst_rect->offset.x) ||
                    (src_rect->offset.y * dst_y1 !=
                     src_y1 * dst_rect->offset.y) ||
                    (src_rect->extent.width != dst_rect->extent.width) ||
                    (src_rect->extent.height != dst_rect->extent.height) ||
                    transfer_cmd->sources[0U].mappings[0U].flip_x ||
                    transfer_cmd->sources[0U].mappings[0U].flip_y) {
            layer->layer_floats = PVR_INT_COORD_SET_FLOATS_4;
         } else {
            layer->layer_floats = PVR_INT_COORD_SET_FLOATS_0;
         }

         /* We have to adjust the rate. */
         if (layer->layer_floats != PVR_INT_COORD_SET_FLOATS_0 &&
             pvr_int_pbe_pixel_changes_dst_rate(dev_info, layer->pbe_format)) {
            layer->layer_floats = PVR_INT_COORD_SET_FLOATS_6;
         }
      }
   }
}

static uint32_t pvr_int_pbe_pixel_num_sampler_and_image_states(
   enum pvr_transfer_pbe_pixel_src pbe_format)
{
   switch (pbe_format) {
   case PVR_TRANSFER_PBE_PIXEL_SRC_Y_UV_INTERLEAVED:
   case PVR_TRANSFER_PBE_PIXEL_SRC_Y_U_V:
      return 1U;
   default:
      return pvr_pbe_pixel_num_loads(pbe_format);
   }
}

static VkResult pvr_sampler_state_for_surface(
   const struct pvr_device_info *dev_info,
   const struct pvr_transfer_cmd_surface *surface,
   enum pvr_filter filter,
   const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout,
   uint32_t sampler,
   uint32_t *mem_ptr)
{
   uint64_t sampler_state[2U] = { 0UL, 0UL };

   pvr_csb_pack (&sampler_state[0U], TEXSTATE_SAMPLER, reg) {
      reg.anisoctl = PVRX(TEXSTATE_ANISOCTL_DISABLED);
      reg.minlod = PVRX(TEXSTATE_CLAMP_MIN);
      reg.maxlod = PVRX(TEXSTATE_CLAMP_MIN);
      reg.dadjust = PVRX(TEXSTATE_DADJUST_MIN_UINT);

      if (filter == PVR_FILTER_DONTCARE || filter == PVR_FILTER_POINT) {
         reg.minfilter = PVRX(TEXSTATE_FILTER_POINT);
         reg.magfilter = PVRX(TEXSTATE_FILTER_POINT);
      } else if (filter == PVR_FILTER_LINEAR) {
         reg.minfilter = PVRX(TEXSTATE_FILTER_LINEAR);
         reg.magfilter = PVRX(TEXSTATE_FILTER_LINEAR);
      } else {
         assert(PVR_HAS_FEATURE(dev_info, tf_bicubic_filter));
         reg.minfilter = PVRX(TEXSTATE_FILTER_BICUBIC);
         reg.magfilter = PVRX(TEXSTATE_FILTER_BICUBIC);
      }

      reg.addrmode_u = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
      reg.addrmode_v = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);

      if (surface->mem_layout == PVR_MEMLAYOUT_3DTWIDDLED)
         reg.addrmode_w = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
   }

   assert(sampler < PVR_TRANSFER_MAX_IMAGES);

   assert(sampler <= sh_reg_layout->combined_image_samplers.count);
   mem_ptr += sh_reg_layout->combined_image_samplers.offsets[sampler].sampler;

   memcpy(mem_ptr, sampler_state, sizeof(sampler_state));

   return VK_SUCCESS;
}

static inline VkResult pvr_image_state_set_codegen_defaults(
   struct pvr_device *device,
   struct pvr_transfer_3d_state *state,
   const struct pvr_transfer_cmd_surface *surface,
   uint32_t load,
   uint64_t *mem_ptr)
{
   struct pvr_tq_layer_properties *layer = &state->shader_props.layer_props;
   struct pvr_texture_state_info info = { 0U };
   VkResult result;

   switch (surface->vk_format) {
   /* ERN 46863 */
   case VK_FORMAT_D32_SFLOAT_S8_UINT:
      switch (layer->pbe_format) {
      case PVR_TRANSFER_PBE_PIXEL_SRC_RAW32:
      case PVR_TRANSFER_PBE_PIXEL_SRC_RAW64:
      case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D24S8:
      case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D32S8_D32S8:
      case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32S8_D32S8:
      case PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D32_D24S8:
      case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32_D24S8:
         info.format = VK_FORMAT_R32G32_UINT;
         break;
      default:
         break;
      }
      break;

   case VK_FORMAT_D24_UNORM_S8_UINT:
   case VK_FORMAT_X8_D24_UNORM_PACK32:
      info.format = VK_FORMAT_R32_UINT;
      break;

   default:
      info.format = surface->vk_format;
      break;
   }

   info.flags = 0U;
   info.base_level = 0U;
   info.mip_levels = 1U;
   info.mipmaps_present = false;
   info.sample_count = MAX2(surface->sample_count, 1U);

   if (surface->mem_layout == PVR_MEMLAYOUT_3DTWIDDLED)
      info.extent.depth = surface->depth;
   else
      info.extent.depth = 0U;

   if (PVR_HAS_FEATURE(&device->pdevice->dev_info, tpu_array_textures))
      info.array_size = 0U;

   result = pvr_mem_layout_spec(surface,
                                load,
                                true,
                                &info.extent.width,
                                &info.extent.height,
                                &info.stride,
                                &info.mem_layout,
                                &info.addr);
   if (result != VK_SUCCESS)
      return result;

   if (state->custom_mapping.texel_extend_dst > 1U) {
      info.extent.width /= state->custom_mapping.texel_extend_dst;
      info.stride /= state->custom_mapping.texel_extend_dst;
   }

   info.tex_state_type = PVR_TEXTURE_STATE_SAMPLE;
   memcpy(info.swizzle,
          pvr_get_format_swizzle(info.format),
          sizeof(info.swizzle));

   if (surface->vk_format == VK_FORMAT_S8_UINT) {
      info.swizzle[0U] = PIPE_SWIZZLE_X;
      info.swizzle[1U] = PIPE_SWIZZLE_0;
      info.swizzle[2U] = PIPE_SWIZZLE_0;
      info.swizzle[3U] = PIPE_SWIZZLE_0;
   }

   if (info.extent.depth > 0U)
      info.type = VK_IMAGE_VIEW_TYPE_3D;
   else if (info.extent.height > 1U)
      info.type = VK_IMAGE_VIEW_TYPE_2D;
   else
      info.type = VK_IMAGE_VIEW_TYPE_1D;

   result = pvr_pack_tex_state(device, &info, mem_ptr);
   if (result != VK_SUCCESS)
      return result;

   return VK_SUCCESS;
}

static VkResult pvr_image_state_for_surface(
   const struct pvr_transfer_ctx *ctx,
   const struct pvr_transfer_cmd *transfer_cmd,
   const struct pvr_transfer_cmd_surface *surface,
   uint32_t load,
   uint32_t source,
   const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout,
   struct pvr_transfer_3d_state *state,
   uint32_t uf_image,
   uint32_t *mem_ptr)
{
   uint32_t tex_state[ROGUE_MAXIMUM_IMAGE_STATE_SIZE] = { 0U };
   VkResult result;
   uint8_t offset;

   result = pvr_image_state_set_codegen_defaults(ctx->device,
                                                 state,
                                                 surface,
                                                 load,
                                                 (uint64_t *)tex_state);
   if (result != VK_SUCCESS)
      return result;

   assert(uf_image < PVR_TRANSFER_MAX_IMAGES);

   /* Offset of the shared registers containing the hardware image state. */
   assert(uf_image < sh_reg_layout->combined_image_samplers.count);
   offset = sh_reg_layout->combined_image_samplers.offsets[uf_image].image;

   /* Copy the image state to the buffer which is loaded into the shared
    * registers.
    */
   memcpy(mem_ptr + offset, tex_state, sizeof(tex_state));

   return VK_SUCCESS;
}

/* Writes the texture state/sampler state into DMAed memory. */
static VkResult
pvr_sampler_image_state(struct pvr_transfer_ctx *ctx,
                        const struct pvr_transfer_cmd *transfer_cmd,
                        const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout,
                        struct pvr_transfer_3d_state *state,
                        uint32_t *mem_ptr)
{
   if (!state->empty_dst) {
      uint32_t uf_sampler = 0U;
      uint32_t uf_image = 0U;

      for (uint32_t source = 0; source < transfer_cmd->source_count; source++) {
         struct pvr_tq_layer_properties *layer =
            &state->shader_props.layer_props;
         uint32_t max_load = pvr_pbe_pixel_num_loads(layer->pbe_format);

         for (uint32_t load = 0U; load < max_load; load++) {
            const struct pvr_transfer_cmd_surface *surface;
            enum pvr_filter filter;
            VkResult result;

            switch (layer->pbe_format) {
            case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D32S8:
            case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D32S8:
            case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D32S8_D32S8:
            case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32S8_D32S8:
            case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D24S8:
            case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D24S8:
            case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D24S8_D24S8:
            case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32_D24S8:
            case PVR_TRANSFER_PBE_PIXEL_SRC_F16F16:
            case PVR_TRANSFER_PBE_PIXEL_SRC_F16_U8:
               if (load > 0U) {
                  surface = &transfer_cmd->dst;
                  filter = transfer_cmd->sources[source].filter;
               } else {
                  surface = &transfer_cmd->sources[source].surface;
                  filter = state->filter[source];
               }
               break;

            case PVR_TRANSFER_PBE_PIXEL_SRC_Y_UV_INTERLEAVED:
            case PVR_TRANSFER_PBE_PIXEL_SRC_Y_U_V:
               surface = &transfer_cmd->sources[source].surface;
               filter = state->filter[source];
               break;

            default:
               surface = &transfer_cmd->sources[source + load].surface;
               filter = state->filter[source + load];
               break;
            }

            if (load < pvr_int_pbe_pixel_num_sampler_and_image_states(
                          layer->pbe_format)) {
               const struct pvr_device_info *dev_info =
                  &transfer_cmd->cmd_buffer->device->pdevice->dev_info;

               result = pvr_sampler_state_for_surface(dev_info,
                                                      surface,
                                                      filter,
                                                      sh_reg_layout,
                                                      uf_sampler,
                                                      mem_ptr);
               if (result != VK_SUCCESS)
                  return result;

               uf_sampler++;

               result = pvr_image_state_for_surface(ctx,
                                                    transfer_cmd,
                                                    surface,
                                                    load,
                                                    source,
                                                    sh_reg_layout,
                                                    state,
                                                    uf_image,
                                                    mem_ptr);
               if (result != VK_SUCCESS)
                  return result;

               uf_image++;
            }
         }
      }
   }

   return VK_SUCCESS;
}

/* The returned offset is in dwords. */
static inline uint32_t pvr_dynamic_const_reg_advance(
   const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout,
   struct pvr_transfer_3d_state *state)
{
   const uint32_t offset = sh_reg_layout->dynamic_consts.offset;

   assert(state->dynamic_const_reg_ptr < sh_reg_layout->dynamic_consts.count);

   return offset + state->dynamic_const_reg_ptr++;
}

/** Scales coefficients for sampling. (non normalized). */
static inline void
pvr_dma_texture_floats(const struct pvr_transfer_cmd *transfer_cmd,
                       struct pvr_transfer_3d_state *state,
                       const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout,
                       uint32_t *mem_ptr)

{
   if (transfer_cmd->source_count > 0) {
      struct pvr_tq_layer_properties *layer = &state->shader_props.layer_props;
      const struct pvr_rect_mapping *mapping =
         &transfer_cmd->sources[0].mappings[0U];
      VkRect2D src_rect = mapping->src_rect;
      VkRect2D dst_rect = mapping->dst_rect;

      switch (layer->layer_floats) {
      case PVR_INT_COORD_SET_FLOATS_0:
         break;

      case PVR_INT_COORD_SET_FLOATS_6:
      case PVR_INT_COORD_SET_FLOATS_4: {
         int32_t consts[2U] = { 0U, 0U };
         int32_t denom[2U] = { 0U, 0U };
         int32_t nums[2U] = { 0U, 0U };
         int32_t src_x, dst_x;
         int32_t src_y, dst_y;
         float offset = 0.0f;
         float tmp;

         dst_x = mapping->flip_x ? -(int32_t)dst_rect.extent.width
                                 : dst_rect.extent.width;
         dst_y = mapping->flip_y ? -(int32_t)dst_rect.extent.height
                                 : dst_rect.extent.height;
         src_x = src_rect.extent.width;
         src_y = src_rect.extent.height;

         nums[0U] = src_x;
         denom[0U] = dst_x;
         consts[0U] =
            mapping->flip_x
               ? src_rect.offset.x * dst_x -
                    src_x * (dst_rect.offset.x + dst_rect.extent.width)
               : src_rect.offset.x * dst_x - src_x * dst_rect.offset.x;
         nums[1U] = src_y;
         denom[1U] = dst_y;
         consts[1U] =
            mapping->flip_y
               ? src_rect.offset.y * dst_y -
                    src_y * (dst_rect.offset.y + dst_rect.extent.height)
               : src_rect.offset.y * dst_y - src_y * dst_rect.offset.y;

         for (uint32_t i = 0U; i < 2U; i++) {
            tmp = (float)(nums[i]) / (float)(denom[i]);
            mem_ptr[pvr_dynamic_const_reg_advance(sh_reg_layout, state)] =
               fui(tmp);

            tmp = ((float)(consts[i]) + (i == 1U ? offset : 0.0f)) /
                  (float)(denom[i]);
            mem_ptr[pvr_dynamic_const_reg_advance(sh_reg_layout, state)] =
               fui(tmp);
         }

         if (layer->layer_floats == PVR_INT_COORD_SET_FLOATS_6) {
            tmp = (float)MIN2(dst_rect.offset.x, dst_rect.offset.x + dst_x);
            mem_ptr[pvr_dynamic_const_reg_advance(sh_reg_layout, state)] =
               fui(tmp);

            tmp = (float)MIN2(dst_rect.offset.y, dst_rect.offset.y + dst_y);
            mem_ptr[pvr_dynamic_const_reg_advance(sh_reg_layout, state)] =
               fui(tmp);
         }
         break;
      }

      default:
         unreachable("Unknown COORD_SET_FLOATS.");
         break;
      }
   }
}

static bool pvr_int_pbe_pixel_requires_usc_filter(
   const struct pvr_device_info *dev_info,
   enum pvr_transfer_pbe_pixel_src pixel_format)
{
   switch (pixel_format) {
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D24S8_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_U16NORM:
   case PVR_TRANSFER_PBE_PIXEL_SRC_S16NORM:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32X2:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32X4:
      return true;
   case PVR_TRANSFER_PBE_PIXEL_SRC_F16F16:
      return !PVR_HAS_FEATURE(dev_info, pbe_filterable_f16);
   default:
      return false;
   }
}

/**
 * Sets up the MSAA related bits in the operation
 *
 * TPU sample count is read directly from transfer_cmd in the TPU code. An MSAA
 * src can be read from sample rate or instance rate shaders as long as the
 * sample count is set on the TPU. If a layer is single sample we expect the
 * same sample replicated in full rate shaders. If the layer is multi sample,
 * instance rate shaders are used to emulate the filter or to select the
 * specified sample. The sample number is static in the programs.
 */
static VkResult pvr_msaa_state(const struct pvr_device_info *dev_info,
                               const struct pvr_transfer_cmd *transfer_cmd,
                               struct pvr_transfer_3d_state *state,
                               uint32_t source)
{
   struct pvr_tq_shader_properties *shader_props = &state->shader_props;
   struct pvr_tq_layer_properties *layer = &shader_props->layer_props;
   struct pvr_winsys_transfer_regs *const regs = &state->regs;
   uint32_t src_sample_count =
      transfer_cmd->sources[source].surface.sample_count & ~1U;
   uint32_t dst_sample_count = transfer_cmd->dst.sample_count & ~1U;
   uint32_t bsample_count = 0U;

   shader_props->full_rate = false;
   state->msaa_multiplier = 1U;
   state->down_scale = false;

   /* clang-format off */
   pvr_csb_pack (&regs->isp_aa, CR_ISP_AA, reg);
   /* clang-format on */

   layer->sample_count = 1U;
   layer->resolve_op = PVR_RESOLVE_BLEND;

   bsample_count |= src_sample_count | dst_sample_count;

   if (bsample_count > PVR_GET_FEATURE_VALUE(dev_info, max_multisample, 0U))
      return vk_error(transfer_cmd->cmd_buffer, VK_ERROR_FORMAT_NOT_SUPPORTED);

   /* Shouldn't get two distinct bits set (implies different sample counts).
    * The reason being the rate at which the shader runs has to match.
    */
   if ((bsample_count & (bsample_count - 1U)) != 0U)
      return vk_error(transfer_cmd->cmd_buffer, VK_ERROR_FORMAT_NOT_SUPPORTED);

   if (src_sample_count == 0U && dst_sample_count == 0U) {
      /* S -> S (no MSAA involved). */
      layer->msaa = false;
   } else if (src_sample_count != 0U && dst_sample_count == 0U) {
      /* M -> S (resolve). */
      layer->resolve_op = transfer_cmd->sources[source].resolve_op;

      if ((uint32_t)layer->resolve_op >=
          (src_sample_count + (uint32_t)PVR_RESOLVE_SAMPLE0)) {
         return vk_error(transfer_cmd->cmd_buffer,
                         VK_ERROR_FORMAT_NOT_SUPPORTED);
      }

      layer->msaa = true;

      switch (layer->resolve_op) {
      case PVR_RESOLVE_MIN:
      case PVR_RESOLVE_MAX:
         switch (transfer_cmd->sources[source].surface.vk_format) {
         case VK_FORMAT_D32_SFLOAT:
         case VK_FORMAT_D16_UNORM:
         case VK_FORMAT_S8_UINT:
         case VK_FORMAT_D24_UNORM_S8_UINT:
         case VK_FORMAT_X8_D24_UNORM_PACK32:
            if (transfer_cmd->sources[source].surface.vk_format !=
                transfer_cmd->dst.vk_format) {
               return vk_error(transfer_cmd->cmd_buffer,
                               VK_ERROR_FORMAT_NOT_SUPPORTED);
            }
            break;

         default:
            return vk_error(transfer_cmd->cmd_buffer,
                            VK_ERROR_FORMAT_NOT_SUPPORTED);
         }

         /* Instance rate. */
         layer->sample_count = src_sample_count;
         state->shader_props.full_rate = false;
         break;

      case PVR_RESOLVE_BLEND:
         if (pvr_int_pbe_pixel_requires_usc_filter(dev_info,
                                                   layer->pbe_format)) {
            /* Instance rate. */
            layer->sample_count = src_sample_count;
            state->shader_props.full_rate = false;
         } else {
            /* Sample rate. */
            state->shader_props.full_rate = true;
            state->msaa_multiplier = src_sample_count;
            state->down_scale = true;

            pvr_csb_pack (&regs->isp_aa, CR_ISP_AA, reg) {
               reg.mode = pvr_cr_isp_aa_mode_type(src_sample_count);
            }
         }
         break;

      default:
         /* Shader doesn't have to know the number of samples. It's enough
          * if the TPU knows, and the shader sets the right sno (given to the
          * shader in resolve_op).
          */
         state->shader_props.full_rate = false;
         break;
      }
   } else {
      state->msaa_multiplier = dst_sample_count;

      pvr_csb_pack (&regs->isp_aa, CR_ISP_AA, reg) {
         reg.mode = pvr_cr_isp_aa_mode_type(dst_sample_count);
      }

      if (src_sample_count == 0U && dst_sample_count != 0U) {
         /* S -> M (replicate samples) */
         layer->msaa = false;
         state->shader_props.full_rate = !state->shader_props.iterated;
      } else {
         /* M -> M (sample to sample) */
         layer->msaa = true;
         state->shader_props.full_rate = true;
      }
   }

   return VK_SUCCESS;
}

static bool pvr_requires_usc_linear_filter(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_R32_SFLOAT:
   case VK_FORMAT_R32G32_SFLOAT:
   case VK_FORMAT_R32G32B32_SFLOAT:
   case VK_FORMAT_R32G32B32A32_SFLOAT:
   case VK_FORMAT_D32_SFLOAT:
   case VK_FORMAT_D24_UNORM_S8_UINT:
   case VK_FORMAT_X8_D24_UNORM_PACK32:
      return true;
   default:
      return false;
   }
}

static inline bool
pvr_int_pbe_usc_linear_filter(enum pvr_transfer_pbe_pixel_src pbe_format,
                              bool sample,
                              bool msaa,
                              bool full_rate)
{
   if (sample || msaa || full_rate)
      return false;

   switch (pbe_format) {
   case PVR_TRANSFER_PBE_PIXEL_SRC_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_S8D24:
   case PVR_TRANSFER_PBE_PIXEL_SRC_D32S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32X2:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32X4:
      return true;
   default:
      return false;
   }
}

static inline bool pvr_pick_component_needed(
   const struct pvr_transfer_custom_mapping *custom_mapping)
{
   return custom_mapping->pass_count > 0U &&
          custom_mapping->texel_extend_dst > 1U &&
          custom_mapping->texel_extend_src <= 1U;
}

/** Writes the shader related constants into the DMA space. */
static void
pvr_write_usc_constants(const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout,
                        uint32_t *dma_space)
{
   const uint32_t reg = sh_reg_layout->driver_total;
   const uint32_t consts_count =
      sh_reg_layout->compiler_out.usc_constants.count;

   /* If not we likely need to write more consts. */
   assert(consts_count == sh_reg_layout->compiler_out_total);

   /* Append the usc consts after the driver allocated regs. */
   for (uint32_t i = 0U; i < consts_count; i++)
      dma_space[reg + i] = sh_reg_layout->compiler_out.usc_constants.values[i];
}

static inline void
pvr_dma_texel_unwind(struct pvr_transfer_3d_state *state,
                     const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout,
                     uint32_t *mem_ptr)

{
   const uint32_t coord_sample_mask =
      state->custom_mapping.texel_extend_dst - 1U;

   mem_ptr[pvr_dynamic_const_reg_advance(sh_reg_layout, state)] =
      coord_sample_mask;
   mem_ptr[pvr_dynamic_const_reg_advance(sh_reg_layout, state)] =
      state->custom_mapping.texel_unwind_dst;
}

/** Writes the Uniform/Texture state data segments + the UniTex code. */
static inline VkResult
pvr_pds_unitex(const struct pvr_device_info *dev_info,
               struct pvr_transfer_ctx *ctx,
               const struct pvr_transfer_cmd *transfer_cmd,
               struct pvr_pds_pixel_shader_sa_program *program,
               struct pvr_transfer_prep_data *prep_data)
{
   struct pvr_pds_upload *unitex_code =
      &ctx->pds_unitex_code[program->num_texture_dma_kicks]
                           [program->num_uniform_dma_kicks];
   struct pvr_transfer_3d_state *state = &prep_data->state;
   struct pvr_suballoc_bo *pvr_bo;
   VkResult result;
   void *map;

   /* Uniform program is not used. */
   assert(program->num_uniform_dma_kicks == 0U);

   if (program->num_texture_dma_kicks == 0U) {
      state->uniform_data_size = 0U;
      state->tex_state_data_size = 0U;
      state->tex_state_data_offset = 0U;
      state->uni_tex_code_offset = 0U;

      return VK_SUCCESS;
   }

   pvr_pds_set_sizes_pixel_shader_sa_uniform_data(program, dev_info);
   assert(program->data_size == 0U);
   state->uniform_data_size = 0U;

   pvr_pds_set_sizes_pixel_shader_sa_texture_data(program, dev_info);
   state->tex_state_data_size =
      ALIGN_POT(program->data_size,
                PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEXTURESTATESIZE_UNIT_SIZE));

   result =
      pvr_cmd_buffer_alloc_mem(transfer_cmd->cmd_buffer,
                               ctx->device->heaps.pds_heap,
                               PVR_DW_TO_BYTES(state->tex_state_data_size),
                               &pvr_bo);
   if (result != VK_SUCCESS)
      return result;

   state->tex_state_data_offset =
      pvr_bo->dev_addr.addr - ctx->device->heaps.pds_heap->base_addr.addr;

   map = pvr_bo_suballoc_get_map_addr(pvr_bo);
   pvr_pds_generate_pixel_shader_sa_texture_state_data(program, map, dev_info);

   /* Save the dev_addr and size in the 3D state. */
   state->uni_tex_code_offset = unitex_code->code_offset;
   state->pds_temps = program->temps_used;

   return VK_SUCCESS;
}

/** Converts a float in range 0 to 1 to an N-bit fixed-point integer. */
static uint32_t pvr_float_to_ufixed(float value, uint32_t bits)
{
   uint32_t max = (1U << bits) - 1U;

   /* NaN and Inf and overflow. */
   if (util_is_inf_or_nan(value) || value >= 1.0f)
      return max;
   else if (value < 0.0f)
      return 0U;

   /* Normalise. */
   value = value * (float)max;

   /* Cast to double so that we can accurately represent the sum for N > 23. */
   return (uint32_t)floor((double)value + 0.5f);
}

/** Converts a float in range -1 to 1 to a signed N-bit fixed-point integer. */
static uint32_t pvr_float_to_sfixed(float value, uint32_t N)
{
   int32_t max = (1 << (N - 1)) - 1;
   int32_t min = 0 - (1 << (N - 1));
   union fi x;

   /* NaN and Inf and overflow. */
   if (util_is_inf_or_nan(value) || value >= 1.0f)
      return (uint32_t)max;
   else if (value == 0.0f)
      return 0U;
   else if (value <= -1.0f)
      return (uint32_t)min;

   /* Normalise. */
   value *= (float)max;

   /* Cast to double so that we can accurately represent the sum for N > 23. */
   if (value > 0.0f)
      x.i = (int32_t)floor((double)value + 0.5f);
   else
      x.i = (int32_t)floor((double)value - 0.5f);

   return x.ui;
}

/** Convert a value in IEEE single precision format to 16-bit floating point
 * format.
 */
/* TODO: See if we can use _mesa_float_to_float16_rtz_slow() instead. */
static uint16_t pvr_float_to_f16(float value, bool round_to_even)
{
   uint32_t input_value;
   uint32_t exponent;
   uint32_t mantissa;
   uint16_t output;

   /* 0.0f can be exactly expressed in binary using IEEE float format. */
   if (value == 0.0f)
      return 0U;

   if (value < 0U) {
      output = 0x8000;
      value = -value;
   } else {
      output = 0U;
   }

   /* 2^16 * (2 - 1/1024) = highest f16 representable value. */
   value = MIN2(value, 131008);
   input_value = fui(value);

   /* Extract the exponent and mantissa. */
   exponent = util_get_float32_exponent(value) + 15;
   mantissa = input_value & ((1 << 23) - 1);

   /* If the exponent is outside the supported range then denormalise the
    * mantissa.
    */
   if ((int32_t)exponent <= 0) {
      uint32_t shift;

      mantissa |= (1 << 23);
      exponent = input_value >> 23;
      shift = -14 + 127 - exponent;

      if (shift < 24)
         mantissa >>= shift;
      else
         mantissa = 0;
   } else {
      output = (uint16_t)(output | ((exponent << 10) & 0x7C00));
   }

   output = (uint16_t)(output | (((mantissa >> 13) << 0) & 0x03FF));

   if (round_to_even) {
      /* Round to nearest even. */
      if ((((int)value) % 2 != 0) && (((1 << 13) - 1) & mantissa))
         output++;
   } else {
      /* Round to nearest. */
      if (mantissa & (1 << 12))
         output++;
   }

   return output;
}

static VkResult pvr_pack_clear_color(VkFormat format,
                                     const union fi color[static 4],
                                     uint32_t pkd_color[static 4])
{
   const uint32_t red_width =
      vk_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 0U);
   uint32_t pbe_pack_mode = pvr_get_pbe_packmode(format);
   const bool pbe_norm = vk_format_is_normalized(format);

   if (pbe_pack_mode == PVRX(PBESTATE_PACKMODE_INVALID))
      return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);

   /* Set packed color based on PBE pack mode and PBE norm. */
   switch (pbe_pack_mode) {
   case PVRX(PBESTATE_PACKMODE_U8U8U8U8):
   case PVRX(PBESTATE_PACKMODE_A8R3G3B2):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 8) & 0xFFU;
         pkd_color[0] |= (pvr_float_to_ufixed(color[1].f, 8) & 0xFFU) << 8;
         pkd_color[0] |= (pvr_float_to_ufixed(color[2].f, 8) & 0xFFU) << 16;
         pkd_color[0] |= (pvr_float_to_ufixed(color[3].f, 8) & 0xFFU) << 24;
      } else {
         pkd_color[0] = color[0].ui & 0xFFU;
         pkd_color[0] |= (color[1].ui & 0xFFU) << 8;
         pkd_color[0] |= (color[2].ui & 0xFFU) << 16;
         pkd_color[0] |= (color[3].ui & 0xFFU) << 24;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_S8S8S8S8):
   case PVRX(PBESTATE_PACKMODE_X8U8S8S8):
   case PVRX(PBESTATE_PACKMODE_X8S8S8U8):
      if (pbe_norm) {
         pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, false);
         pkd_color[0] |= (uint32_t)pvr_float_to_f16(color[1].f, false) << 16;
         pkd_color[1] = (uint32_t)pvr_float_to_f16(color[2].f, false);
         pkd_color[1] |= (uint32_t)pvr_float_to_f16(color[3].f, false) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFU;
         pkd_color[0] |= (color[1].ui & 0xFFU) << 8;
         pkd_color[0] |= (color[2].ui & 0xFFU) << 16;
         pkd_color[0] |= (color[3].ui & 0xFFU) << 24;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_U16U16U16U16):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 16) & 0xFFFFU;
         pkd_color[0] |= (pvr_float_to_ufixed(color[1].f, 16) & 0xFFFFU) << 16;
         pkd_color[1] = pvr_float_to_ufixed(color[2].f, 16) & 0xFFFFU;
         pkd_color[1] |= (pvr_float_to_ufixed(color[3].f, 16) & 0xFFFFU) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFFFU;
         pkd_color[0] |= (color[1].ui & 0xFFFFU) << 16;
         pkd_color[1] = color[2].ui & 0xFFFFU;
         pkd_color[1] |= (color[3].ui & 0xFFFFU) << 16;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_S16S16S16S16):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_sfixed(color[0].f, 16) & 0xFFFFU;
         pkd_color[0] |= (pvr_float_to_sfixed(color[1].f, 16) & 0xFFFFU) << 16;
         pkd_color[1] = (pvr_float_to_sfixed(color[2].f, 16) & 0xFFFFU);
         pkd_color[1] |= (pvr_float_to_sfixed(color[3].f, 16) & 0xFFFFU) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFFFU;
         pkd_color[0] |= (color[1].ui & 0xFFFFU) << 16;
         pkd_color[1] = color[2].ui & 0xFFFFU;
         pkd_color[1] |= (color[3].ui & 0xFFFFU) << 16;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_A2_XRBIAS_U10U10U10):
   case PVRX(PBESTATE_PACKMODE_ARGBV16_XR10):
   case PVRX(PBESTATE_PACKMODE_F16F16F16F16):
   case PVRX(PBESTATE_PACKMODE_A2R10B10G10):
   case PVRX(PBESTATE_PACKMODE_A4R4G4B4):
   case PVRX(PBESTATE_PACKMODE_A1R5G5B5):
   case PVRX(PBESTATE_PACKMODE_R5G5B5A1):
   case PVRX(PBESTATE_PACKMODE_R5G6B5):
      if (red_width > 0) {
         pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, false);
         pkd_color[0] |= (uint32_t)pvr_float_to_f16(color[1].f, false) << 16;
         pkd_color[1] = (uint32_t)pvr_float_to_f16(color[2].f, false);
         pkd_color[1] |= (uint32_t)pvr_float_to_f16(color[3].f, false) << 16;
      } else {
         /* Swizzle only uses first channel for alpha formats. */
         pkd_color[0] = (uint32_t)pvr_float_to_f16(color[3].f, false);
      }
      break;

   case PVRX(PBESTATE_PACKMODE_U32U32U32U32):
      pkd_color[0] = color[0].ui;
      pkd_color[1] = color[1].ui;
      pkd_color[2] = color[2].ui;
      pkd_color[3] = color[3].ui;
      break;

   case PVRX(PBESTATE_PACKMODE_S32S32S32S32):
      pkd_color[0] = (uint32_t)color[0].i;
      pkd_color[1] = (uint32_t)color[1].i;
      pkd_color[2] = (uint32_t)color[2].i;
      pkd_color[3] = (uint32_t)color[3].i;
      break;

   case PVRX(PBESTATE_PACKMODE_F32F32F32F32):
      memcpy(pkd_color, &color[0].f, 4U * sizeof(float));
      break;

   case PVRX(PBESTATE_PACKMODE_R10B10G10A2):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 10) & 0xFFU;
         pkd_color[0] |= (pvr_float_to_ufixed(color[1].f, 10) & 0xFFU) << 10;
         pkd_color[0] |= (pvr_float_to_ufixed(color[2].f, 10) & 0xFFU) << 20;
         pkd_color[0] |= (pvr_float_to_ufixed(color[3].f, 2) & 0xFFU) << 30;
      } else if (format == VK_FORMAT_A2R10G10B10_UINT_PACK32) {
         pkd_color[0] = color[2].ui & 0x3FFU;
         pkd_color[0] |= (color[1].ui & 0x3FFU) << 10;
         pkd_color[0] |= (color[0].ui & 0x3FFU) << 20;
         pkd_color[0] |= (color[3].ui & 0x3U) << 30;
      } else {
         pkd_color[0] = color[0].ui & 0x3FFU;
         pkd_color[0] |= (color[1].ui & 0x3FFU) << 10;
         pkd_color[0] |= (color[2].ui & 0x3FFU) << 20;
         pkd_color[0] |= (color[3].ui & 0x3U) << 30;
      }

      break;

   case PVRX(PBESTATE_PACKMODE_A2F10F10F10):
   case PVRX(PBESTATE_PACKMODE_F10F10F10A2):
      pkd_color[0] = pvr_float_to_sfixed(color[0].f, 10) & 0xFFU;
      pkd_color[0] |= (pvr_float_to_sfixed(color[1].f, 10) & 0xFFU) << 10;
      pkd_color[0] |= (pvr_float_to_sfixed(color[2].f, 10) & 0xFFU) << 20;
      pkd_color[0] |= (pvr_float_to_sfixed(color[3].f, 2) & 0xFFU) << 30;
      break;

   case PVRX(PBESTATE_PACKMODE_U8U8U8):
   case PVRX(PBESTATE_PACKMODE_R5SG5SB6):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 8) & 0xFFU;
         pkd_color[0] |= (pvr_float_to_ufixed(color[1].f, 8) & 0xFFU) << 8;
         pkd_color[0] |= (pvr_float_to_ufixed(color[2].f, 8) & 0xFFU) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFU;
         pkd_color[0] |= (color[1].ui & 0xFFU) << 8;
         pkd_color[0] |= (color[2].ui & 0xFFU) << 16;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_S8S8S8):
   case PVRX(PBESTATE_PACKMODE_B6G5SR5S):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_sfixed(color[0].f, 8) & 0xFFU;
         pkd_color[0] |= (pvr_float_to_sfixed(color[1].f, 8) & 0xFFU) << 8;
         pkd_color[0] |= (pvr_float_to_sfixed(color[2].f, 8) & 0xFFU) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFU;
         pkd_color[0] |= (color[1].ui & 0xFFU) << 8;
         pkd_color[0] |= (color[2].ui & 0xFFU) << 16;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_U16U16U16):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 16) & 0xFFFFU;
         pkd_color[0] |= (pvr_float_to_ufixed(color[1].f, 16) & 0xFFFFU) << 16;
         pkd_color[1] = (pvr_float_to_ufixed(color[2].f, 16) & 0xFFFFU);
      } else {
         pkd_color[0] = color[0].ui & 0xFFFFU;
         pkd_color[0] |= (color[1].ui & 0xFFFFU) << 16;
         pkd_color[1] = color[2].ui & 0xFFFFU;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_S16S16S16):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_sfixed(color[0].f, 16) & 0xFFFFU;
         pkd_color[0] |= (pvr_float_to_sfixed(color[1].f, 16) & 0xFFFFU) << 16;
         pkd_color[1] = pvr_float_to_sfixed(color[2].f, 16) & 0xFFFFU;
      } else {
         pkd_color[0] = color[0].ui & 0xFFFFU;
         pkd_color[0] |= (color[1].ui & 0xFFFFU) << 16;
         pkd_color[1] = color[2].ui & 0xFFFFU;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_F16F16F16):
   case PVRX(PBESTATE_PACKMODE_F11F11F10):
   case PVRX(PBESTATE_PACKMODE_F10F11F11):
   case PVRX(PBESTATE_PACKMODE_SE9995):
      pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, true);
      pkd_color[0] |= (uint32_t)pvr_float_to_f16(color[1].f, true) << 16;
      pkd_color[1] = (uint32_t)pvr_float_to_f16(color[2].f, true);
      break;

   case PVRX(PBESTATE_PACKMODE_U32U32U32):
      pkd_color[0] = color[0].ui;
      pkd_color[1] = color[1].ui;
      pkd_color[2] = color[2].ui;
      break;

   case PVRX(PBESTATE_PACKMODE_S32S32S32):
      pkd_color[0] = (uint32_t)color[0].i;
      pkd_color[1] = (uint32_t)color[1].i;
      pkd_color[2] = (uint32_t)color[2].i;
      break;

   case PVRX(PBESTATE_PACKMODE_X24G8X32):
   case PVRX(PBESTATE_PACKMODE_U8X24):
      pkd_color[1] = (color[1].ui & 0xFFU) << 24;
      break;

   case PVRX(PBESTATE_PACKMODE_F32F32F32):
      memcpy(pkd_color, &color[0].f, 3U * sizeof(float));
      break;

   case PVRX(PBESTATE_PACKMODE_U8U8):
      if (pbe_norm) {
         pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, false);
         pkd_color[0] |= (uint32_t)pvr_float_to_f16(color[1].f, false) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFU;
         pkd_color[0] |= (color[1].ui & 0xFFU) << 8;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_S8S8):
      if (pbe_norm) {
         pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, false);
         pkd_color[0] |= (uint32_t)pvr_float_to_f16(color[1].f, false) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFU;
         pkd_color[0] |= (color[1].ui & 0xFFU) << 8;
         pkd_color[0] |= (color[2].ui & 0xFFU) << 16;
         pkd_color[0] |= (color[3].ui & 0xFFU) << 24;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_U16U16):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 16) & 0xFFFFU;
         pkd_color[0] |= (pvr_float_to_ufixed(color[1].f, 16) & 0xFFFFU) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFFFU;
         pkd_color[0] |= (color[1].ui & 0xFFFFU) << 16;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_S16S16):
      if (pbe_norm) {
         pkd_color[0] = pvr_float_to_sfixed(color[0].f, 16) & 0xFFFFU;
         pkd_color[0] |= (pvr_float_to_sfixed(color[1].f, 16) & 0xFFFFU) << 16;
      } else {
         pkd_color[0] = color[0].ui & 0xFFFFU;
         pkd_color[0] |= (color[1].ui & 0xFFFFU) << 16;
      }
      break;

   case PVRX(PBESTATE_PACKMODE_F16F16):
      pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, true);
      pkd_color[0] |= (uint32_t)pvr_float_to_f16(color[1].f, true) << 16;
      break;

   case PVRX(PBESTATE_PACKMODE_U32U32):
      pkd_color[0] = color[0].ui;
      pkd_color[1] = color[1].ui;
      break;

   case PVRX(PBESTATE_PACKMODE_S32S32):
      pkd_color[0] = (uint32_t)color[0].i;
      pkd_color[1] = (uint32_t)color[1].i;
      break;

   case PVRX(PBESTATE_PACKMODE_X24U8F32):
   case PVRX(PBESTATE_PACKMODE_X24X8F32):
      memcpy(pkd_color, &color[0].f, 1U * sizeof(float));
      pkd_color[1] = color[1].ui & 0xFFU;
      break;

   case PVRX(PBESTATE_PACKMODE_F32F32):
      memcpy(pkd_color, &color[0].f, 2U * sizeof(float));
      break;

   case PVRX(PBESTATE_PACKMODE_ST8U24):
      pkd_color[0] = pvr_float_to_ufixed(color[0].f, 24) & 0xFFFFFFU;
      pkd_color[0] |= color[1].ui << 24;
      break;

   case PVRX(PBESTATE_PACKMODE_U8):
      if (format == VK_FORMAT_S8_UINT)
         pkd_color[0] = color[1].ui & 0xFFU;
      else if (pbe_norm)
         pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, false);
      else
         pkd_color[0] = color[0].ui & 0xFFU;

      break;

   case PVRX(PBESTATE_PACKMODE_S8):
      if (pbe_norm)
         pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, false);
      else
         pkd_color[0] = color[0].ui & 0xFFU;
      break;

   case PVRX(PBESTATE_PACKMODE_U16):
      if (pbe_norm)
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 16) & 0xFFFFU;
      else
         pkd_color[0] = color[0].ui & 0xFFFFU;
      break;

   case PVRX(PBESTATE_PACKMODE_S16):
      if (pbe_norm)
         pkd_color[0] = pvr_float_to_sfixed(color[0].f, 16) & 0xFFFFU;
      else
         pkd_color[0] = color[0].ui & 0xFFFFU;
      break;

   case PVRX(PBESTATE_PACKMODE_F16):
      pkd_color[0] = (uint32_t)pvr_float_to_f16(color[0].f, true);
      break;

   /* U32 */
   case PVRX(PBESTATE_PACKMODE_U32):
      if (format == VK_FORMAT_X8_D24_UNORM_PACK32) {
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 24) & 0xFFFFFFU;
      } else if (format == VK_FORMAT_D24_UNORM_S8_UINT) {
         pkd_color[0] = pvr_float_to_ufixed(color[0].f, 24) & 0xFFFFFFU;
         pkd_color[0] |= (color[1].ui & 0xFFU) << 24;
      } else if (format == VK_FORMAT_A2B10G10R10_UINT_PACK32) {
         pkd_color[0] = color[0].ui & 0x3FFU;
         pkd_color[0] |= (color[1].ui & 0x3FFU) << 10;
         pkd_color[0] |= (color[2].ui & 0x3FFU) << 20;
         pkd_color[0] |= (color[3].ui & 0x3U) << 30;
      } else {
         pkd_color[0] = color[0].ui;
      }
      break;

   /* U24ST8 */
   case PVRX(PBESTATE_PACKMODE_U24ST8):
      pkd_color[1] = (color[1].ui & 0xFFU) << 24;
      pkd_color[1] |= pvr_float_to_ufixed(color[0].f, 24) & 0xFFFFFFU;
      break;

   /* S32 */
   case PVRX(PBESTATE_PACKMODE_S32):
      pkd_color[0] = (uint32_t)color[0].i;
      break;

   /* F32 */
   case PVRX(PBESTATE_PACKMODE_F32):
      memcpy(pkd_color, &color[0].f, sizeof(float));
      break;

   /* X8U24 */
   case PVRX(PBESTATE_PACKMODE_X8U24):
      pkd_color[0] = pvr_float_to_ufixed(color[0].f, 24) & 0xFFFFFFU;
      break;

   default:
      break;
   }

   return VK_SUCCESS;
}

static VkResult
pvr_isp_scan_direction(struct pvr_transfer_cmd *transfer_cmd,
                       bool custom_mapping,
                       enum PVRX(CR_DIR_TYPE) *const dir_type_out)
{
   pvr_dev_addr_t dst_dev_addr = transfer_cmd->dst.dev_addr;
   bool backwards_in_x = false;
   bool backwards_in_y = false;
   bool done_dest_rect = false;
   VkRect2D dst_rect;
   int32_t dst_x1;
   int32_t dst_y1;

   for (uint32_t i = 0; i < transfer_cmd->source_count; i++) {
      struct pvr_transfer_cmd_source *src = &transfer_cmd->sources[i];
      pvr_dev_addr_t src_dev_addr = src->surface.dev_addr;

      if (src_dev_addr.addr == dst_dev_addr.addr && !custom_mapping) {
         VkRect2D *src_rect = &src->mappings[0].src_rect;
         int32_t src_x1 = src_rect->offset.x + src_rect->extent.width;
         int32_t src_y1 = src_rect->offset.y + src_rect->extent.height;

         if (!done_dest_rect) {
            dst_rect = src->mappings[0].dst_rect;

            dst_x1 = dst_rect.offset.x + dst_rect.extent.width;
            dst_y1 = dst_rect.offset.y + dst_rect.extent.height;

            done_dest_rect = true;
         }

         if ((dst_rect.offset.x < src_x1 && dst_x1 > src_rect->offset.x) &&
             (dst_rect.offset.y < src_y1 && dst_y1 > src_rect->offset.y)) {
            if (src_rect->extent.width != dst_rect.extent.width ||
                src_rect->extent.height != dst_rect.extent.height) {
               /* Scaling is not possible. */
               return vk_error(NULL, VK_ERROR_FORMAT_NOT_SUPPORTED);
            }

            /* Direction is to the right. */
            backwards_in_x = dst_rect.offset.x > src_rect->offset.x;

            /* Direction is to the bottom. */
            backwards_in_y = dst_rect.offset.y > src_rect->offset.y;
         }
      }
   }

   if (backwards_in_x) {
      if (backwards_in_y)
         *dir_type_out = PVRX(CR_DIR_TYPE_BR2TL);
      else
         *dir_type_out = PVRX(CR_DIR_TYPE_TR2BL);
   } else {
      if (backwards_in_y)
         *dir_type_out = PVRX(CR_DIR_TYPE_BL2TR);
      else
         *dir_type_out = PVRX(CR_DIR_TYPE_TL2BR);
   }

   return VK_SUCCESS;
}

static VkResult pvr_3d_copy_blit_core(struct pvr_transfer_ctx *ctx,
                                      struct pvr_transfer_cmd *transfer_cmd,
                                      struct pvr_transfer_prep_data *prep_data,
                                      uint32_t pass_idx,
                                      bool *finished_out)
{
   struct pvr_transfer_3d_state *const state = &prep_data->state;
   struct pvr_winsys_transfer_regs *const regs = &state->regs;
   struct pvr_device *const device = ctx->device;
   const struct pvr_device_info *const dev_info = &device->pdevice->dev_info;

   VkResult result;

   *finished_out = true;

   state->common_ptr = 0U;
   state->dynamic_const_reg_ptr = 0U;
   state->usc_const_reg_ptr = 0U;

   if ((transfer_cmd->flags & PVR_TRANSFER_CMD_FLAGS_FILL) != 0U) {
      uint32_t packed_color[4U] = { 0U };

      if (transfer_cmd->source_count != 0U)
         return vk_error(device, VK_ERROR_FORMAT_NOT_SUPPORTED);

      if (vk_format_is_compressed(transfer_cmd->dst.vk_format))
         return vk_error(device, VK_ERROR_FORMAT_NOT_SUPPORTED);

      /* No shader. */
      state->pds_temps = 0U;
      state->uniform_data_size = 0U;
      state->tex_state_data_size = 0U;

      /* No background enabled. */
      /* clang-format off */
      pvr_csb_pack (&regs->isp_bgobjvals, CR_ISP_BGOBJVALS, reg);
      /* clang-format on */
      pvr_csb_pack (&regs->isp_aa, CR_ISP_AA, reg) {
         reg.mode = pvr_cr_isp_aa_mode_type(transfer_cmd->dst.sample_count);
      }

      result = pvr_pack_clear_color(transfer_cmd->dst.vk_format,
                                    transfer_cmd->clear_color,
                                    packed_color);
      if (result != VK_SUCCESS)
         return result;

      pvr_csb_pack (&regs->usc_clear_register0, CR_USC_CLEAR_REGISTER, reg) {
         reg.val = packed_color[0U];
      }

      pvr_csb_pack (&regs->usc_clear_register1, CR_USC_CLEAR_REGISTER, reg) {
         reg.val = packed_color[1U];
      }

      pvr_csb_pack (&regs->usc_clear_register2, CR_USC_CLEAR_REGISTER, reg) {
         reg.val = packed_color[2U];
      }

      pvr_csb_pack (&regs->usc_clear_register3, CR_USC_CLEAR_REGISTER, reg) {
         reg.val = packed_color[3U];
      }

      state->msaa_multiplier = transfer_cmd->dst.sample_count & ~1U;
      state->pds_shader_task_offset = 0U;
      state->uni_tex_code_offset = 0U;
      state->tex_state_data_offset = 0U;
   } else if (transfer_cmd->source_count > 0U) {
      const struct pvr_tq_frag_sh_reg_layout nop_sh_reg_layout = {
         /* TODO: Setting this to 1 so that we don't try to pvr_bo_alloc() with
          * zero size. The device will ignore the PDS program if USC_SHAREDSIZE
          * is zero and in the case of the nop shader we're expecting it to be
          * zero. See if we can safely pass PVR_DEV_ADDR_INVALID for the unitex
          * program.
          */
         .driver_total = 1,
      };
      const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout;
      struct pvr_pds_pixel_shader_sa_program unitex_prog = { 0U };
      uint32_t tex_state_dma_size_dw;
      struct pvr_suballoc_bo *pvr_bo;
      uint32_t *dma_space;

      result = pvr_pbe_src_format(transfer_cmd, state, &state->shader_props);
      if (result != VK_SUCCESS)
         return result;

      pvr_uv_space(dev_info, transfer_cmd, state);

      state->shader_props.iterated = false;

      state->shader_props.layer_props.sample =
         transfer_cmd->sources[0].surface.mem_layout ==
         PVR_MEMLAYOUT_3DTWIDDLED;

      result = pvr_msaa_state(dev_info, transfer_cmd, state, 0);
      if (result != VK_SUCCESS)
         return result;

      state->shader_props.pick_component =
         pvr_pick_component_needed(&state->custom_mapping);

      if (state->filter[0] == PVR_FILTER_LINEAR &&
          pvr_requires_usc_linear_filter(
             transfer_cmd->sources[0].surface.vk_format)) {
         if (pvr_int_pbe_usc_linear_filter(
                state->shader_props.layer_props.pbe_format,
                state->shader_props.layer_props.sample,
                state->shader_props.layer_props.msaa,
                state->shader_props.full_rate)) {
            state->shader_props.layer_props.linear = true;
         } else {
            mesa_logw("Transfer: F32 linear filter not supported.");
         }
      }

      if (state->empty_dst) {
         sh_reg_layout = &nop_sh_reg_layout;
         state->pds_shader_task_offset = device->nop_program.pds.data_offset;
      } else {
         pvr_dev_addr_t kick_usc_pds_dev_addr;

         result =
            pvr_transfer_frag_store_get_shader_info(device,
                                                    &ctx->frag_store,
                                                    &state->shader_props,
                                                    &kick_usc_pds_dev_addr,
                                                    &sh_reg_layout);
         if (result != VK_SUCCESS)
            return result;

         assert(kick_usc_pds_dev_addr.addr <= UINT32_MAX);
         state->pds_shader_task_offset = (uint32_t)kick_usc_pds_dev_addr.addr;
      }

      unitex_prog.kick_usc = false;
      unitex_prog.clear = false;

      tex_state_dma_size_dw =
         sh_reg_layout->driver_total + sh_reg_layout->compiler_out_total;

      unitex_prog.num_texture_dma_kicks = 1U;
      unitex_prog.num_uniform_dma_kicks = 0U;

      result = pvr_cmd_buffer_alloc_mem(transfer_cmd->cmd_buffer,
                                        device->heaps.general_heap,
                                        PVR_DW_TO_BYTES(tex_state_dma_size_dw),
                                        &pvr_bo);
      if (result != VK_SUCCESS)
         return result;

      dma_space = (uint32_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);

      result = pvr_sampler_image_state(ctx,
                                       transfer_cmd,
                                       sh_reg_layout,
                                       state,
                                       dma_space);
      if (result != VK_SUCCESS)
         return result;

      pvr_dma_texture_floats(transfer_cmd, state, sh_reg_layout, dma_space);

      if (transfer_cmd->sources[0].surface.mem_layout ==
          PVR_MEMLAYOUT_3DTWIDDLED) {
         dma_space[pvr_dynamic_const_reg_advance(sh_reg_layout, state)] =
            fui(transfer_cmd->sources[0].surface.z_position);
      }

      pvr_write_usc_constants(sh_reg_layout, dma_space);

      if (pvr_pick_component_needed(&state->custom_mapping))
         pvr_dma_texel_unwind(state, sh_reg_layout, dma_space);

      pvr_pds_encode_dma_burst(unitex_prog.texture_dma_control,
                               unitex_prog.texture_dma_address,
                               state->common_ptr,
                               tex_state_dma_size_dw,
                               pvr_bo->dev_addr.addr,
                               true,
                               dev_info);

      state->common_ptr += tex_state_dma_size_dw;

      result =
         pvr_pds_unitex(dev_info, ctx, transfer_cmd, &unitex_prog, prep_data);
      if (result != VK_SUCCESS)
         return result;

      pvr_csb_pack (&regs->isp_bgobjvals, CR_ISP_BGOBJVALS, reg) {
         reg.enablebgtag = true;
      }

      /* clang-format off */
      pvr_csb_pack (&regs->isp_aa, CR_ISP_AA, reg);
      /* clang-format on */
   } else {
      /* No shader. */
      state->pds_temps = 0U;
      state->uniform_data_size = 0U;
      state->tex_state_data_size = 0U;

      /* No background enabled. */
      /* clang-format off */
      pvr_csb_pack (&regs->isp_bgobjvals, CR_ISP_BGOBJVALS, reg);
      /* clang-format on */
      pvr_csb_pack (&regs->isp_aa, CR_ISP_AA, reg) {
         reg.mode = pvr_cr_isp_aa_mode_type(transfer_cmd->dst.sample_count);
      }
      state->msaa_multiplier = transfer_cmd->dst.sample_count & ~1U;
      state->pds_shader_task_offset = 0U;
      state->uni_tex_code_offset = 0U;
      state->tex_state_data_offset = 0U;

      result = pvr_pbe_src_format(transfer_cmd, state, &state->shader_props);
      if (result != VK_SUCCESS)
         return result;
   }

   pvr_setup_hwbg_object(dev_info, state);

   pvr_csb_pack (&regs->isp_render, CR_ISP_RENDER, reg) {
      reg.mode_type = PVRX(CR_ISP_RENDER_MODE_TYPE_FAST_SCALE);

      result = pvr_isp_scan_direction(transfer_cmd,
                                      state->custom_mapping.pass_count,
                                      &reg.dir_type);
      if (result != VK_SUCCESS)
         return result;
   }

   /* Set up pixel event handling. */
   result = pvr_pbe_setup(transfer_cmd, ctx, state);
   if (result != VK_SUCCESS)
      return result;

   result = pvr_isp_tiles(device, state);
   if (result != VK_SUCCESS)
      return result;

   if (PVR_HAS_FEATURE(&device->pdevice->dev_info, gpu_multicore_support)) {
      pvr_csb_pack (&regs->frag_screen, CR_FRAG_SCREEN, reg) {
         reg.xmax = transfer_cmd->dst.width - 1;
         reg.ymax = transfer_cmd->dst.height - 1;
      }
   }

   if ((pass_idx + 1U) < state->custom_mapping.pass_count)
      *finished_out = false;

   return VK_SUCCESS;
}

static VkResult
pvr_pbe_src_format_f2d(uint32_t merge_flags,
                       struct pvr_transfer_cmd_source *src,
                       VkFormat dst_format,
                       bool down_scale,
                       bool dont_force_pbe,
                       enum pvr_transfer_pbe_pixel_src *pixel_format_out)
{
   VkFormat src_format = src->surface.vk_format;

   /* This has to come before the rest as S8 for instance is integer and
    * signedsess check fails on D24S8.
    */
   if (vk_format_is_depth_or_stencil(src_format) ||
       vk_format_is_depth_or_stencil(dst_format) ||
       merge_flags & PVR_TRANSFER_CMD_FLAGS_DSMERGE) {
      return pvr_pbe_src_format_ds(&src->surface,
                                   src->filter,
                                   dst_format,
                                   merge_flags,
                                   down_scale,
                                   pixel_format_out);
   }

   return pvr_pbe_src_format_normal(src_format,
                                    dst_format,
                                    down_scale,
                                    dont_force_pbe,
                                    pixel_format_out);
}

/** Writes the coefficient loading PDS task. */
static inline VkResult
pvr_pds_coeff_task(struct pvr_transfer_ctx *ctx,
                   const struct pvr_transfer_cmd *transfer_cmd,
                   const bool sample_3d,
                   struct pvr_transfer_prep_data *prep_data)
{
   struct pvr_transfer_3d_state *state = &prep_data->state;
   struct pvr_pds_coeff_loading_program program = { 0U };
   struct pvr_suballoc_bo *pvr_bo;
   VkResult result;

   program.num_fpu_iterators = 1U;

   pvr_csb_pack (&program.FPU_iterators[0U],
                 PDSINST_DOUT_FIELDS_DOUTI_SRC,
                 reg) {
      if (sample_3d)
         reg.size = PVRX(PDSINST_DOUTI_SIZE_3D);
      else
         reg.size = PVRX(PDSINST_DOUTI_SIZE_2D);

      reg.perspective = false;

      /* Varying wrap on the TSP means that the TSP chooses the shorter path
       * out of the normal and the wrapping path i.e. chooses between u0->u1
       * and u1->1.0 == 0.0 -> u0. We don't need this behavior.
       */
      /*
       * if RHW ever needed offset SRC_F32 to the first U in 16 bit units
       * l0 U    <= offs 0
       * l0 V
       * l1 U    <= offs 4
       * ...
       */
      reg.shademodel = PVRX(PDSINST_DOUTI_SHADEMODEL_GOURUAD);
      reg.f32_offset = 0U;
   }

   if (sample_3d)
      state->usc_coeff_regs = 12U;
   else
      state->usc_coeff_regs = 8U;

   pvr_pds_set_sizes_coeff_loading(&program);

   result = pvr_cmd_buffer_alloc_mem(
      transfer_cmd->cmd_buffer,
      ctx->device->heaps.pds_heap,
      PVR_DW_TO_BYTES(program.data_size + program.code_size),
      &pvr_bo);
   if (result != VK_SUCCESS)
      return result;

   state->pds_coeff_task_offset =
      pvr_bo->dev_addr.addr - ctx->device->heaps.pds_heap->base_addr.addr;

   pvr_pds_generate_coeff_loading_program(&program,
                                          pvr_bo_suballoc_get_map_addr(pvr_bo));

   state->coeff_data_size = program.data_size;
   state->pds_temps = program.temps_used;

   return VK_SUCCESS;
}

#define X 0U
#define Y 1U
#define Z 2U

static void pvr_tsp_floats(const struct pvr_device_info *dev_info,
                           VkRect2D *rect,
                           const float recips[3U],
                           bool custom_filter,
                           bool z_present,
                           float z_value,
                           struct pvr_transfer_3d_iteration *layer)
{
#define U0 0U
#define U1 1U
#define V0 2U
#define V1 3U

   const uint32_t indices[8U] = { U0, V0, U0, V1, U1, V1, U1, V0 };
   float delta[2U] = { 0.0f, 0.0f };
   int32_t non_normalized[4U];
   uint32_t src_flipped[2U];
   uint32_t normalized[4U];
   int32_t src_span[2U];

   non_normalized[U0] = rect->offset.x;
   non_normalized[U1] = rect->offset.x + rect->extent.width;
   non_normalized[V0] = rect->offset.y;
   non_normalized[V1] = rect->offset.y + rect->extent.height;

   /* Filter adjust. */
   src_span[X] = rect->extent.width;
   src_flipped[X] = src_span[X] > 0U ? 0U : 1U;
   src_span[Y] = rect->extent.height;
   src_flipped[Y] = src_span[Y] > 0U ? 0U : 1U;
   /*
    * | X  | Y  | srcFlipX | srcFlipY |
    * +----+----+----------+----------|
    * | X  | Y  | 0        | 0        |
    * | -X | Y  | 1        | 0        |
    * | X  | -Y | 0        | 1        |
    * | -X | -Y | 1        | 1        |
    */
   for (uint32_t i = X; i <= Y; i++) {
      if (custom_filter) {
         if (src_flipped[i] != 0U)
            delta[i] += 0.25;
         else
            delta[i] -= 0.25;
      }
   }

   /* Normalize. */
   for (uint32_t i = 0U; i < ARRAY_SIZE(normalized); i++) {
      uint32_t tmp;
      float ftmp;

      ftmp = (float)non_normalized[i] + delta[i >> 1U];
      ftmp *= recips[i >> 1U];

      tmp = fui(ftmp);
      if (!PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format))
         tmp = XXH_rotl32(tmp, 1U);

      normalized[i] = tmp;
   }

   /* Apply indices. */
   for (uint32_t i = 0U; i < 8U; i++)
      layer->texture_coords[i] = normalized[indices[i]];

   if (z_present) {
      uint32_t tmp = fui(z_value * recips[2U]);

      if (!PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format))
         tmp = XXH_rotl32(tmp, 1U);

      for (uint32_t i = 8U; i < 12U; i++)
         layer->texture_coords[i] = tmp;
   }

#undef U0
#undef U1
#undef V0
#undef V1
}

static void
pvr_isp_prim_block_tsp_vertex_block(const struct pvr_device_info *dev_info,
                                    const struct pvr_transfer_cmd_source *src,
                                    struct pvr_rect_mapping *mappings,
                                    bool custom_filter,
                                    uint32_t num_mappings,
                                    uint32_t mapping_offset,
                                    uint32_t tsp_comp_format_in_dw,
                                    uint32_t **const cs_ptr_out)
{
   struct pvr_transfer_3d_iteration layer;
   uint32_t *cs_ptr = *cs_ptr_out;

   /*  |<-32b->|
    *  +-------+-----
    *  |  RHW  |    | X num_isp_vertices
    *  +-------+--  |
    *  |  U    | |  |
    *  |  V    | | X PVR_TRANSFER_NUM_LAYERS
    *  +-------+-----
    *
    * RHW is not there any more in the Transfer. The comment still explains
    * where it should go if ever needed.
    */
   for (uint32_t i = mapping_offset; i < mapping_offset + num_mappings; i++) {
      bool z_present = src->surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED;
      const float recips[3U] = {
         [X] = 1.0f / (float)src->surface.width,
         [Y] = 1.0f / (float)src->surface.height,
         [Z] = z_present ? 1.0f / (float)src->surface.depth : 0.0f,
      };
      float z_pos = (src->filter < PVR_FILTER_LINEAR)
                       ? floor(src->surface.z_position + 0.5f)
                       : src->surface.z_position;

      pvr_tsp_floats(dev_info,
                     &mappings[i].src_rect,
                     recips,
                     custom_filter,
                     z_present,
                     z_pos,
                     &layer);

      /* We request UVs from TSP for ISP triangle:
       *  0 u 1
       *  +---,
       * v|  /|
       *  | / |
       * 2'/--'3
       */
      for (uint32_t j = 0U; j < PVR_TRANSFER_NUM_LAYERS; j++) {
         *cs_ptr++ = layer.texture_coords[0U];
         *cs_ptr++ = layer.texture_coords[1U];
      }

      if (z_present) {
         *cs_ptr++ = layer.texture_coords[8U];
         *cs_ptr++ = 0U;
      }

      for (uint32_t j = 0U; j < PVR_TRANSFER_NUM_LAYERS; j++) {
         *cs_ptr++ = layer.texture_coords[6U];
         *cs_ptr++ = layer.texture_coords[7U];
      }

      if (z_present) {
         *cs_ptr++ = layer.texture_coords[11U];
         *cs_ptr++ = 0U;
      }

      for (uint32_t j = 0U; j < PVR_TRANSFER_NUM_LAYERS; j++) {
         *cs_ptr++ = layer.texture_coords[2U];
         *cs_ptr++ = layer.texture_coords[3U];
      }

      if (z_present) {
         *cs_ptr++ = layer.texture_coords[9U];
         *cs_ptr++ = 0U;
      }

      for (uint32_t j = 0U; j < PVR_TRANSFER_NUM_LAYERS; j++) {
         *cs_ptr++ = layer.texture_coords[4U];
         *cs_ptr++ = layer.texture_coords[5U];
      }

      if (z_present) {
         *cs_ptr++ = layer.texture_coords[10U];
         *cs_ptr++ = 0U;
      }
   }

   if (!PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
      /* Skipped optional primitive id. */
      for (uint32_t i = 0U; i < tsp_comp_format_in_dw; i++)
         *cs_ptr++ = 0x88888888U;
   } else {
      /* Align back to 64 bits. */
      if (((uintptr_t)cs_ptr & 7U) != 0U)
         cs_ptr++;
   }

   *cs_ptr_out = cs_ptr;
}

#undef X
#undef Y
#undef Z

static void pvr_isp_prim_block_pds_state(const struct pvr_device_info *dev_info,
                                         struct pvr_transfer_ctx *ctx,
                                         struct pvr_transfer_3d_state *state,
                                         uint32_t **const cs_ptr_out)
{
   uint32_t *cs_ptr = *cs_ptr_out;

   pvr_csb_pack (cs_ptr, TA_STATE_PDS_SHADERBASE, shader_base) {
      shader_base.addr = PVR_DEV_ADDR(state->pds_shader_task_offset);
   }
   cs_ptr++;

   pvr_csb_pack (cs_ptr, TA_STATE_PDS_TEXUNICODEBASE, tex_base) {
      tex_base.addr = PVR_DEV_ADDR(state->uni_tex_code_offset);
   }
   cs_ptr++;

   pvr_csb_pack (cs_ptr, TA_STATE_PDS_SIZEINFO1, info1) {
      info1.pds_uniformsize =
         state->uniform_data_size /
         PVRX(TA_STATE_PDS_SIZEINFO1_PDS_UNIFORMSIZE_UNIT_SIZE);

      info1.pds_texturestatesize =
         state->tex_state_data_size /
         PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEXTURESTATESIZE_UNIT_SIZE);

      info1.pds_varyingsize =
         state->coeff_data_size /
         PVRX(TA_STATE_PDS_SIZEINFO1_PDS_VARYINGSIZE_UNIT_SIZE);

      info1.usc_varyingsize =
         ALIGN_POT(state->usc_coeff_regs,
                   PVRX(TA_STATE_PDS_SIZEINFO1_USC_VARYINGSIZE_UNIT_SIZE)) /
         PVRX(TA_STATE_PDS_SIZEINFO1_USC_VARYINGSIZE_UNIT_SIZE);

      info1.pds_tempsize =
         ALIGN_POT(state->pds_temps,
                   PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEMPSIZE_UNIT_SIZE)) /
         PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEMPSIZE_UNIT_SIZE);
   }
   cs_ptr++;

   pvr_csb_pack (cs_ptr, TA_STATE_PDS_VARYINGBASE, base) {
      base.addr = PVR_DEV_ADDR(state->pds_coeff_task_offset);
   }
   cs_ptr++;

   pvr_csb_pack (cs_ptr, TA_STATE_PDS_TEXTUREDATABASE, base) {
      base.addr = PVR_DEV_ADDR(state->tex_state_data_offset);
   }
   cs_ptr++;

   /* PDS uniform program not used. */
   pvr_csb_pack (cs_ptr, TA_STATE_PDS_UNIFORMDATABASE, base) {
      base.addr = PVR_DEV_ADDR(0U);
   }
   cs_ptr++;

   pvr_csb_pack (cs_ptr, TA_STATE_PDS_SIZEINFO2, info) {
      info.usc_sharedsize =
         ALIGN_POT(state->common_ptr,
                   PVRX(TA_STATE_PDS_SIZEINFO2_USC_SHAREDSIZE_UNIT_SIZE)) /
         PVRX(TA_STATE_PDS_SIZEINFO2_USC_SHAREDSIZE_UNIT_SIZE);
      info.pds_tri_merge_disable = !PVR_HAS_ERN(dev_info, 42307);
      info.pds_batchnum = 0U;
   }
   cs_ptr++;

   /* Get back to 64 bits boundary. */
   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format))
      cs_ptr++;

   *cs_ptr_out = cs_ptr;
}

static void pvr_isp_prim_block_isp_state(const struct pvr_device_info *dev_info,
                                         UNUSED uint32_t tsp_comp_format_in_dw,
                                         uint32_t tsp_data_size_in_bytes,
                                         uint32_t num_isp_vertices,
                                         bool read_bgnd,
                                         uint32_t **const cs_ptr_out)
{
   const bool has_simple_internal_parameter_format_v2 =
      PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format_v2);
   uint32_t *cs_ptr = *cs_ptr_out;

   if (has_simple_internal_parameter_format_v2) {
      const uint32_t tsp_data_per_vrx_in_bytes =
         tsp_data_size_in_bytes / num_isp_vertices;

      pvr_csb_pack ((uint64_t *)cs_ptr,
                    IPF_VERTEX_FORMAT_WORD_SIPF2,
                    vert_fmt) {
         vert_fmt.vf_isp_state_size =
            pvr_cmd_length(TA_STATE_ISPCTL) + pvr_cmd_length(TA_STATE_ISPA);

         vert_fmt.vf_tsp_vtx_raw = true;
         vert_fmt.vf_isp_vtx_raw = true;

         vert_fmt.vf_varying_vertex_bits = tsp_data_per_vrx_in_bytes * 8U;
         vert_fmt.vf_primitive_total = (num_isp_vertices / 2U) - 1U;
         vert_fmt.vf_vertex_total = num_isp_vertices - 1U;
      }
      cs_ptr += pvr_cmd_length(IPF_VERTEX_FORMAT_WORD_SIPF2);
   }

   /* ISP state words. */

   /* clang-format off */
   pvr_csb_pack (cs_ptr, TA_STATE_ISPCTL, ispctl);
   /* clang-format on */
   cs_ptr += pvr_cmd_length(TA_STATE_ISPCTL);

   pvr_csb_pack (cs_ptr, TA_STATE_ISPA, ispa) {
      ispa.objtype = PVRX(TA_OBJTYPE_TRIANGLE);
      ispa.passtype = read_bgnd ? PVRX(TA_PASSTYPE_TRANSLUCENT)
                                : PVRX(TA_PASSTYPE_OPAQUE);
      ispa.dcmpmode = PVRX(TA_CMPMODE_ALWAYS);
      ispa.dwritedisable = true;
   }
   cs_ptr += pvr_cmd_length(TA_STATE_ISPA);

   if (has_simple_internal_parameter_format_v2) {
      *cs_ptr_out = cs_ptr;
      return;
   }

   /* How many bytes the TSP compression format needs? */
   pvr_csb_pack (cs_ptr, IPF_COMPRESSION_SIZE_WORD, word) {
      word.cs_isp_comp_table_size = 0U;
      word.cs_tsp_comp_format_size = tsp_comp_format_in_dw;
      word.cs_tsp_comp_table_size = 0U;
      word.cs_tsp_comp_vertex_size = tsp_data_size_in_bytes / num_isp_vertices;
   }
   cs_ptr += pvr_cmd_length(IPF_COMPRESSION_SIZE_WORD);

   /* ISP vertex compression. */
   pvr_csb_pack (cs_ptr, IPF_ISP_COMPRESSION_WORD_0, word0) {
      word0.cf_isp_comp_fmt_x0 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
      word0.cf_isp_comp_fmt_x1 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
      word0.cf_isp_comp_fmt_x2 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
      word0.cf_isp_comp_fmt_y0 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
      word0.cf_isp_comp_fmt_y1 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
      word0.cf_isp_comp_fmt_y2 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
      word0.cf_isp_comp_fmt_z0 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
      word0.cf_isp_comp_fmt_z1 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
   }
   cs_ptr += pvr_cmd_length(IPF_ISP_COMPRESSION_WORD_0);

   pvr_csb_pack (cs_ptr, IPF_ISP_COMPRESSION_WORD_1, word1) {
      word1.vf_prim_msaa = 0U;
      word1.vf_prim_id_pres = 0U;
      word1.vf_vertex_clipped = 0U;
      word1.vf_vertex_total = num_isp_vertices - 1U;
      word1.cf_isp_comp_fmt_z3 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
      word1.cf_isp_comp_fmt_z2 = PVRX(IPF_COMPRESSION_FORMAT_RAW_BYTE);
   }
   cs_ptr += pvr_cmd_length(IPF_ISP_COMPRESSION_WORD_1);

   *cs_ptr_out = cs_ptr;
}

static void
pvr_isp_prim_block_index_block(const struct pvr_device_info *dev_info,
                               uint32_t num_mappings,
                               uint32_t **const cs_ptr_out)
{
   uint32_t *cs_ptr = *cs_ptr_out;

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
      for (uint32_t i = 0U; i < DIV_ROUND_UP(num_mappings, 2U); i++) {
         const uint32_t idx = i * 8U;

         pvr_csb_pack ((uint64_t *)cs_ptr,
                       IPF_INDEX_DATA_WORDS_SIPF,
                       idx_data_word) {
            idx_data_word.ix_triangle3_index_2 = idx + 5U;
            idx_data_word.ix_triangle3_index_1 = idx + 6U;
            idx_data_word.ix_triangle3_index_0 = idx + 7U;

            idx_data_word.ix_triangle2_index_2 = idx + 6U;
            idx_data_word.ix_triangle2_index_1 = idx + 5U;
            idx_data_word.ix_triangle2_index_0 = idx + 4U;

            idx_data_word.ix_triangle1_index_2 = idx + 1U;
            idx_data_word.ix_triangle1_index_1 = idx + 2U;
            idx_data_word.ix_triangle1_index_0 = idx + 3U;

            idx_data_word.ix_triangle0_index_2 = idx + 2U;
            idx_data_word.ix_triangle0_index_1 = idx + 1U;
            idx_data_word.ix_triangle0_index_0 = idx + 0U;
         }
         cs_ptr += pvr_cmd_length(IPF_INDEX_DATA_WORDS_SIPF);
      }

      *cs_ptr_out = cs_ptr;
      return;
   }

   for (uint32_t i = 0U, j = 0U; i < num_mappings; i++, j += 4U) {
      if ((i & 1U) == 0U) {
         pvr_csb_pack (cs_ptr, IPF_INDEX_DATA, word) {
            word.ix_index0_0 = j;
            word.ix_index0_1 = j + 1U;
            word.ix_index0_2 = j + 2U;
            word.ix_index1_0 = j + 3U;
         }
         cs_ptr += pvr_cmd_length(IPF_INDEX_DATA);

         /* Don't increment cs_ptr here. IPF_INDEX_DATA is patched in the
          * else part and then cs_ptr is incremented.
          */
         pvr_csb_pack (cs_ptr, IPF_INDEX_DATA, word) {
            word.ix_index0_0 = j + 2U;
            word.ix_index0_1 = j + 1U;
         }
      } else {
         uint32_t tmp;

         pvr_csb_pack (&tmp, IPF_INDEX_DATA, word) {
            word.ix_index0_2 = j;
            word.ix_index1_0 = j + 1U;
         }
         *cs_ptr |= tmp;
         cs_ptr += pvr_cmd_length(IPF_INDEX_DATA);

         pvr_csb_pack (cs_ptr, IPF_INDEX_DATA, word) {
            word.ix_index0_0 = j + 2U;
            word.ix_index0_1 = j + 3U;
            word.ix_index0_2 = j + 2U;
            word.ix_index1_0 = j + 1U;
         }
         cs_ptr += pvr_cmd_length(IPF_INDEX_DATA);
      }
   }

   /* The last pass didn't ++. */
   if ((num_mappings & 1U) != 0U)
      cs_ptr++;

   *cs_ptr_out = cs_ptr;
}

/* Calculates a 24 bit fixed point (biased) representation of a signed integer.
 */
static inline VkResult
pvr_int32_to_isp_xy_vtx(const struct pvr_device_info *dev_info,
                        int32_t val,
                        bool bias,
                        uint32_t *word_out)
{
   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
      const uint32_t max_fractional = PVRX(IPF_ISP_VERTEX_XY_SIPF_FRAC_MAX_VAL);
      const uint32_t max_integer = PVRX(IPF_ISP_VERTEX_XY_SIPF_INTEGER_MAX_VAL);

      uint32_t fractional;
      uint32_t integer;

      if (bias)
         val += PVRX(IPF_ISP_VERTEX_XY_BIAS_VALUE_SIPF);

      if (val < 0 || val > max_integer + 1) {
         mesa_loge("ISP vertex xy value out of range.");
         return vk_error(NULL, VK_ERROR_UNKNOWN);
      }

      if (val <= max_integer) {
         integer = val;
         fractional = 0;
      } else if (val == max_integer + 1) {
         /* The integer field is 13 bits long so the max value is
          * 2 ^ 13 - 1 = 8191. For 8k support we need to handle 8192 so we set
          * all fractional bits to get as close as possible. The best we can do
          * is: 0x1FFF.F = 8191.9375 ≈ 8192 .
          */
         integer = max_integer;
         fractional = max_fractional;
      }

      pvr_csb_pack (word_out, IPF_ISP_VERTEX_XY_SIPF, word) {
         word.integer = integer;
         word.frac = fractional;
      }

      return VK_SUCCESS;
   }

   val += PVRX(IPF_ISP_VERTEX_XY_BIAS_VALUE);

   if (((uint32_t)val & 0x7fff8000U) != 0U)
      return vk_error(NULL, VK_ERROR_UNKNOWN);

   pvr_csb_pack (word_out, IPF_ISP_VERTEX_XY, word) {
      word.sign = val < 0;
      word.integer = val;
   }

   return VK_SUCCESS;
}

static VkResult
pvr_isp_prim_block_isp_vertices(const struct pvr_device_info *dev_info,
                                struct pvr_transfer_3d_state *state,
                                struct pvr_rect_mapping *mappings,
                                uint32_t num_mappings,
                                uint32_t mapping_offset,
                                uint32_t **const cs_ptr_out)
{
   uint32_t *cs_ptr = *cs_ptr_out;
   bool bias = true;
   uint32_t i;

   if (PVR_HAS_FEATURE(dev_info, screen_size8K))
      bias = state->width_in_tiles <= 256U && state->height_in_tiles <= 256U;

   for (i = mapping_offset; i < mapping_offset + num_mappings; i++) {
      uint32_t bottom = 0U;
      uint32_t right = 0U;
      uint32_t left = 0U;
      uint32_t top = 0U;
      VkResult result;

      /* ISP vertex data (X, Y, Z). */
      result = pvr_int32_to_isp_xy_vtx(dev_info,
                                       mappings[i].dst_rect.offset.y,
                                       bias,
                                       &top);
      if (result != VK_SUCCESS)
         return result;

      result = pvr_int32_to_isp_xy_vtx(dev_info,
                                       mappings[i].dst_rect.offset.y +
                                          mappings[i].dst_rect.extent.height,
                                       bias,
                                       &bottom);
      if (result != VK_SUCCESS)
         return result;

      result = pvr_int32_to_isp_xy_vtx(dev_info,
                                       mappings[i].dst_rect.offset.x,
                                       bias,
                                       &left);
      if (result != VK_SUCCESS)
         return result;

      result = pvr_int32_to_isp_xy_vtx(dev_info,
                                       mappings[i].dst_rect.offset.x +
                                          mappings[i].dst_rect.extent.width,
                                       bias,
                                       &right);
      if (result != VK_SUCCESS)
         return result;

      if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
         pvr_csb_pack ((uint64_t *)cs_ptr, IPF_ISP_VERTEX_WORD_SIPF, word) {
            word.y = top;
            word.x = left;
         }
         cs_ptr += pvr_cmd_length(IPF_ISP_VERTEX_WORD_SIPF);

         pvr_csb_pack ((uint64_t *)cs_ptr, IPF_ISP_VERTEX_WORD_SIPF, word) {
            word.y = top;
            word.x = right;
         }
         cs_ptr += pvr_cmd_length(IPF_ISP_VERTEX_WORD_SIPF);

         pvr_csb_pack ((uint64_t *)cs_ptr, IPF_ISP_VERTEX_WORD_SIPF, word) {
            word.y = bottom;
            word.x = left;
         }
         cs_ptr += pvr_cmd_length(IPF_ISP_VERTEX_WORD_SIPF);

         pvr_csb_pack ((uint64_t *)cs_ptr, IPF_ISP_VERTEX_WORD_SIPF, word) {
            word.y = bottom;
            word.x = right;
         }
         cs_ptr += pvr_cmd_length(IPF_ISP_VERTEX_WORD_SIPF);

         continue;
      }

      /* ISP vertices 0 and 1. */
      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_0, word0) {
         word0.x0 = left;
         word0.y0 = top & 0xFF;
      }
      cs_ptr++;

      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_1, word1) {
         word1.y0 = top >> PVRX(IPF_ISP_VERTEX_WORD_1_Y0_SHIFT);
      }
      cs_ptr++;

      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_2, word2) {
         word2.x1 = right & 0xFFFF;
         word2.z0 = 0U;
      }
      cs_ptr++;

      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_3, word3) {
         word3.x1 = right >> PVRX(IPF_ISP_VERTEX_WORD_3_X1_SHIFT);
         word3.y1 = top;
      }
      cs_ptr++;

      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_4, word4) {
         word4.z1 = 0U;
      }
      cs_ptr++;

      /* ISP vertices 2 and 3. */
      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_0, word0) {
         word0.x0 = left;
         word0.y0 = bottom & 0xFF;
      }
      cs_ptr++;

      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_1, word1) {
         word1.y0 = bottom >> PVRX(IPF_ISP_VERTEX_WORD_1_Y0_SHIFT);
      }
      cs_ptr++;

      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_2, word2) {
         word2.x1 = right & 0xFFFF;
         word2.z0 = 0U;
      }
      cs_ptr++;

      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_3, word3) {
         word3.x1 = right >> PVRX(IPF_ISP_VERTEX_WORD_3_X1_SHIFT);
         word3.y1 = bottom;
      }
      cs_ptr++;

      pvr_csb_pack (cs_ptr, IPF_ISP_VERTEX_WORD_4, word4) {
         word4.z1 = 0U;
      }
      cs_ptr++;
   }
   *cs_ptr_out = cs_ptr;

   return VK_SUCCESS;
}

static uint32_t
pvr_isp_primitive_block_size(const struct pvr_device_info *dev_info,
                             const struct pvr_transfer_cmd_source *src,
                             uint32_t num_mappings)
{
   uint32_t num_isp_vertices = num_mappings * 4U;
   uint32_t num_tsp_vertices_per_isp_vertex;
   uint32_t isp_vertex_data_size_dw;
   bool color_fill = (src == NULL);
   uint32_t tsp_comp_format_dw;
   uint32_t isp_state_size_dw;
   uint32_t pds_state_size_dw;
   uint32_t idx_data_size_dw;
   uint32_t tsp_data_size;
   uint32_t stream_size;

   if (color_fill) {
      num_tsp_vertices_per_isp_vertex = 0U;
   } else {
      num_tsp_vertices_per_isp_vertex =
         src->surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED ? 4U : 2U;
   }

   tsp_data_size = PVR_DW_TO_BYTES(num_isp_vertices * PVR_TRANSFER_NUM_LAYERS *
                                   num_tsp_vertices_per_isp_vertex);

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
      /* An XYZ vertex is 16/16/32 bits => 8 bytes. */
      isp_vertex_data_size_dw = num_isp_vertices * 2U;

      /* Round to even for 64 bit boundary. */
      idx_data_size_dw = ALIGN_POT(num_mappings, 2U);
      tsp_comp_format_dw = 0U;
      isp_state_size_dw = 4U;
      pds_state_size_dw = 8U;
   } else {
      tsp_comp_format_dw = color_fill ? 0U : PVR_TRANSFER_NUM_LAYERS;

      if (!color_fill) {
         if (src->surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED)
            tsp_comp_format_dw *= 2U;
      }

      /* An XYZ vertex is 24/24/32 bits => 10 bytes with last padded to 4 byte
       * burst align.
       */
      isp_vertex_data_size_dw = DIV_ROUND_UP(num_isp_vertices * 10U, 4U);

      /* 4 triangles fit in 3 dw: t0t0t0t1_t1t1t2t2_t2t3t3t3. */
      idx_data_size_dw = num_mappings + DIV_ROUND_UP(num_mappings, 2U);
      isp_state_size_dw = 5U;
      pds_state_size_dw = 7U;
   }

   stream_size =
      tsp_data_size + PVR_DW_TO_BYTES(idx_data_size_dw + tsp_comp_format_dw +
                                      isp_vertex_data_size_dw +
                                      isp_state_size_dw + pds_state_size_dw);

   return stream_size;
}

static VkResult
pvr_isp_primitive_block(const struct pvr_device_info *dev_info,
                        struct pvr_transfer_ctx *ctx,
                        const struct pvr_transfer_cmd *transfer_cmd,
                        struct pvr_transfer_prep_data *prep_data,
                        const struct pvr_transfer_cmd_source *src,
                        bool custom_filter,
                        struct pvr_rect_mapping *mappings,
                        uint32_t num_mappings,
                        uint32_t mapping_offset,
                        bool read_bgnd,
                        uint32_t *cs_start_offset,
                        uint32_t **cs_ptr_out)
{
   struct pvr_transfer_3d_state *state = &prep_data->state;
   uint32_t num_isp_vertices = num_mappings * 4U;
   uint32_t num_tsp_vertices_per_isp_vert;
   uint32_t tsp_data_size_in_bytes;
   uint32_t tsp_comp_format_in_dw;
   bool color_fill = src == NULL;
   uint32_t stream_size_in_bytes;
   uint32_t *cs_ptr_start;
   VkResult result;

   if (color_fill) {
      num_tsp_vertices_per_isp_vert = 0U;
   } else {
      num_tsp_vertices_per_isp_vert =
         src->surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED ? 4U : 2U;
   }

   tsp_data_size_in_bytes =
      PVR_DW_TO_BYTES(num_isp_vertices * PVR_TRANSFER_NUM_LAYERS *
                      num_tsp_vertices_per_isp_vert);

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
      tsp_comp_format_in_dw = 0U;
   } else {
      tsp_comp_format_in_dw = color_fill ? 0U : PVR_TRANSFER_NUM_LAYERS;

      if (!color_fill && src->surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED)
         tsp_comp_format_in_dw *= 2U;
   }

   stream_size_in_bytes =
      pvr_isp_primitive_block_size(dev_info, src, num_mappings);

   cs_ptr_start = *cs_ptr_out;

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
      /* This includes:
       *    Vertex formats.
       *    ISP state words.
       */
      pvr_isp_prim_block_isp_state(dev_info,
                                   tsp_comp_format_in_dw,
                                   tsp_data_size_in_bytes,
                                   num_isp_vertices,
                                   read_bgnd,
                                   cs_ptr_out);

      /* This include:
       *    Index data / point pitch.
       */
      pvr_isp_prim_block_index_block(dev_info, num_mappings, cs_ptr_out);

      result = pvr_isp_prim_block_isp_vertices(dev_info,
                                               state,
                                               mappings,
                                               num_mappings,
                                               mapping_offset,
                                               cs_ptr_out);
      if (result != VK_SUCCESS)
         return result;

      pvr_isp_prim_block_pds_state(dev_info, ctx, state, cs_ptr_out);

      if (!color_fill) {
         /* This includes:
          *    TSP vertex formats.
          */
         pvr_isp_prim_block_tsp_vertex_block(dev_info,
                                             src,
                                             mappings,
                                             custom_filter,
                                             num_mappings,
                                             mapping_offset,
                                             tsp_comp_format_in_dw,
                                             cs_ptr_out);
      }

      *cs_start_offset = 0;
   } else {
      if (!color_fill) {
         /* This includes:
          *    Compressed TSP vertex data & tables.
          *    Primitive id.
          *    TSP compression formats.
          */
         pvr_isp_prim_block_tsp_vertex_block(dev_info,
                                             src,
                                             mappings,
                                             custom_filter,
                                             num_mappings,
                                             mapping_offset,
                                             tsp_comp_format_in_dw,
                                             cs_ptr_out);
      }

      pvr_isp_prim_block_pds_state(dev_info, ctx, state, cs_ptr_out);

      /* Point the CS_PRIM_BASE here. */
      *cs_start_offset = (*cs_ptr_out - cs_ptr_start) * sizeof(cs_ptr_start[0]);

      /* This includes:
       *    ISP state words.
       *    Compression size word.
       *    ISP compression and vertex formats.
       */
      pvr_isp_prim_block_isp_state(dev_info,
                                   tsp_comp_format_in_dw,
                                   tsp_data_size_in_bytes,
                                   num_isp_vertices,
                                   read_bgnd,
                                   cs_ptr_out);

      pvr_isp_prim_block_index_block(dev_info, num_mappings, cs_ptr_out);

      result = pvr_isp_prim_block_isp_vertices(dev_info,
                                               state,
                                               mappings,
                                               num_mappings,
                                               mapping_offset,
                                               cs_ptr_out);
      if (result != VK_SUCCESS)
         return result;
   }

   assert((*cs_ptr_out - cs_ptr_start) * sizeof(cs_ptr_start[0]) ==
          stream_size_in_bytes);

   return VK_SUCCESS;
}

static inline uint32_t
pvr_transfer_prim_blocks_per_alloc(const struct pvr_device_info *dev_info)
{
   uint32_t ret = PVR_DW_TO_BYTES(PVRX(IPF_CONTROL_STREAM_SIZE_DWORDS));

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format))
      return ret / sizeof(uint64_t) / 2U;

   return ret / sizeof(uint32_t) / 2U - 1U;
}

static inline uint32_t
pvr_transfer_max_quads_per_pb(const struct pvr_device_info *dev_info)
{
   return PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format) ? 4U
                                                                      : 16U;
}

static inline uint8_t *pvr_isp_ctrl_stream_sipf_write_aligned(uint8_t *stream,
                                                              uint32_t data,
                                                              uint32_t size)
{
   const uint32_t offset = (uintptr_t)stream & 0x3U;
   uint32_t *aligned_stream = (uint32_t *)(stream - offset);
   const uint32_t current_data = *aligned_stream & ((1U << (offset * 8U)) - 1U);

   assert(size > 0 && size <= 4U);

   *aligned_stream = current_data | data << (offset * 8U);

   if (offset + size > 4U) {
      aligned_stream++;
      *aligned_stream = data >> ((4U - offset) * 8);
   }

   return stream + size;
}

/**
 * Writes ISP ctrl stream.
 *
 * We change sampler/texture state when we process a new TQ source. The
 * primitive block contains the shader pointers, but we supply the primitive
 * blocks with shaders from here.
 */
static VkResult pvr_isp_ctrl_stream(const struct pvr_device_info *dev_info,
                                    struct pvr_transfer_ctx *ctx,
                                    struct pvr_transfer_cmd *transfer_cmd,
                                    struct pvr_transfer_prep_data *prep_data)
{
   const uint32_t max_mappings_per_pb = pvr_transfer_max_quads_per_pb(dev_info);
   bool fill_blit = (transfer_cmd->flags & PVR_TRANSFER_CMD_FLAGS_FILL) != 0U;
   uint32_t free_ctrl_stream_words = PVRX(IPF_CONTROL_STREAM_SIZE_DWORDS);
   struct pvr_transfer_3d_state *const state = &prep_data->state;
   struct pvr_winsys_transfer_regs *const regs = &state->regs;
   struct pvr_transfer_pass *pass = NULL;
   uint32_t flags = transfer_cmd->flags;
   struct pvr_suballoc_bo *pvr_cs_bo;
   pvr_dev_addr_t stream_base_vaddr;
   uint32_t num_prim_blks = 0U;
   uint32_t prim_blk_size = 0U;
   uint32_t region_arrays_size;
   uint32_t num_region_arrays;
   uint32_t total_stream_size;
   bool was_linked = false;
   uint32_t rem_mappings;
   uint32_t num_sources;
   uint32_t *blk_cs_ptr;
   uint32_t *cs_ptr;
   uint32_t source;
   VkResult result;

   if (state->custom_mapping.pass_count > 0U) {
      pass = &state->custom_mapping.passes[state->pass_idx];

      num_sources = pass->source_count;

      for (source = 0; source < num_sources; source++) {
         uint32_t num_mappings = pass->sources[source].mapping_count;

         while (num_mappings > 0U) {
            if (fill_blit) {
               prim_blk_size += pvr_isp_primitive_block_size(
                  dev_info,
                  NULL,
                  MIN2(max_mappings_per_pb, num_mappings));
            }

            if (transfer_cmd->source_count > 0) {
               prim_blk_size += pvr_isp_primitive_block_size(
                  dev_info,
                  &transfer_cmd->sources[source],
                  MIN2(max_mappings_per_pb, num_mappings));
            }

            num_mappings -= MIN2(max_mappings_per_pb, num_mappings);
            num_prim_blks++;
         }
      }
   } else {
      num_sources = fill_blit ? 1U : transfer_cmd->source_count;

      if (fill_blit) {
         num_prim_blks = 1U;
         prim_blk_size +=
            pvr_isp_primitive_block_size(dev_info,
                                         NULL,
                                         MIN2(max_mappings_per_pb, 1U));

         /* Fill blits can also have a source; fallthrough to handle. */
      }

      for (source = 0; source < transfer_cmd->source_count; source++) {
         uint32_t num_mappings = transfer_cmd->sources[source].mapping_count;

         while (num_mappings > 0U) {
            prim_blk_size += pvr_isp_primitive_block_size(
               dev_info,
               &transfer_cmd->sources[source],
               MIN2(max_mappings_per_pb, num_mappings));

            num_mappings -= MIN2(max_mappings_per_pb, num_mappings);
            num_prim_blks++;
         }
      }
   }

   num_region_arrays =
      (num_prim_blks + (pvr_transfer_prim_blocks_per_alloc(dev_info) - 1U)) /
      pvr_transfer_prim_blocks_per_alloc(dev_info);
   region_arrays_size = PVRX(IPF_CONTROL_STREAM_SIZE_DWORDS) *
                        sizeof(uint32_t) * num_region_arrays;
   total_stream_size = region_arrays_size + prim_blk_size;

   /* Allocate space for IPF control stream. */
   result = pvr_cmd_buffer_alloc_mem(transfer_cmd->cmd_buffer,
                                     ctx->device->heaps.transfer_frag_heap,
                                     total_stream_size,
                                     &pvr_cs_bo);
   if (result != VK_SUCCESS)
      return result;

   stream_base_vaddr =
      PVR_DEV_ADDR(pvr_cs_bo->dev_addr.addr -
                   ctx->device->heaps.transfer_frag_heap->base_addr.addr);

   cs_ptr = pvr_bo_suballoc_get_map_addr(pvr_cs_bo);
   blk_cs_ptr = cs_ptr + region_arrays_size / sizeof(uint32_t);

   source = 0;
   while (source < num_sources) {
      if (fill_blit)
         rem_mappings = pass ? pass->sources[source].mapping_count : 1U;
      else
         rem_mappings = transfer_cmd->sources[source].mapping_count;

      if ((transfer_cmd->source_count > 0 || fill_blit) && rem_mappings != 0U) {
         struct pvr_pds_pixel_shader_sa_program unitex_pds_prog = { 0U };
         struct pvr_transfer_cmd_source *src = &transfer_cmd->sources[source];
         struct pvr_rect_mapping fill_mapping;
         uint32_t mapping_offset = 0U;
         bool read_bgnd = false;

         if (fill_blit) {
            uint32_t packed_color[4U] = { 0U };

            if (vk_format_is_compressed(transfer_cmd->dst.vk_format)) {
               return vk_error(transfer_cmd->cmd_buffer,
                               VK_ERROR_FORMAT_NOT_SUPPORTED);
            }

            state->pds_shader_task_offset = 0U;
            state->uni_tex_code_offset = 0U;
            state->tex_state_data_offset = 0U;
            state->common_ptr = 0U;

            result = pvr_pack_clear_color(transfer_cmd->dst.vk_format,
                                          transfer_cmd->clear_color,
                                          packed_color);
            if (result != VK_SUCCESS)
               return result;

            fill_mapping.dst_rect = transfer_cmd->scissor;

            pvr_csb_pack (&regs->usc_clear_register0,
                          CR_USC_CLEAR_REGISTER,
                          reg) {
               reg.val = packed_color[0U];
            }

            pvr_csb_pack (&regs->usc_clear_register1,
                          CR_USC_CLEAR_REGISTER,
                          reg) {
               reg.val = packed_color[1U];
            }

            pvr_csb_pack (&regs->usc_clear_register2,
                          CR_USC_CLEAR_REGISTER,
                          reg) {
               reg.val = packed_color[2U];
            }

            pvr_csb_pack (&regs->usc_clear_register3,
                          CR_USC_CLEAR_REGISTER,
                          reg) {
               reg.val = packed_color[3U];
            }

            state->pds_shader_task_offset =
               transfer_cmd->cmd_buffer->device->nop_program.pds.data_offset;

            unitex_pds_prog.kick_usc = false;
            unitex_pds_prog.clear = false;
         } else {
            const bool down_scale = transfer_cmd->sources[source].resolve_op ==
                                       PVR_RESOLVE_BLEND &&
                                    src->surface.sample_count > 1U &&
                                    transfer_cmd->dst.sample_count <= 1U;
            struct pvr_tq_shader_properties *shader_props =
               &state->shader_props;
            struct pvr_tq_layer_properties *layer = &shader_props->layer_props;
            const struct pvr_tq_frag_sh_reg_layout *sh_reg_layout;
            enum pvr_transfer_pbe_pixel_src pbe_src_format;
            struct pvr_suballoc_bo *pvr_bo;
            uint32_t tex_state_dma_size;
            pvr_dev_addr_t dev_offset;

            /* Reset the shared register bank ptrs each src implies new texture
             * state (Note that we don't change texture state per prim block).
             */
            state->common_ptr = 0U;
            state->usc_const_reg_ptr = 0U;
            /* We don't use state->dynamic_const_reg_ptr here. */

            if (flags & PVR_TRANSFER_CMD_FLAGS_DSMERGE)
               read_bgnd = true;

            result = pvr_pbe_src_format_f2d(flags,
                                            src,
                                            transfer_cmd->dst.vk_format,
                                            down_scale,
                                            state->dont_force_pbe,
                                            &pbe_src_format);
            if (result != VK_SUCCESS)
               return result;

            memset(shader_props, 0U, sizeof(*shader_props));

            layer->pbe_format = pbe_src_format;
            layer->sample =
               (src->surface.mem_layout == PVR_MEMLAYOUT_3DTWIDDLED);
            shader_props->iterated = true;

            shader_props->pick_component =
               pvr_pick_component_needed(&state->custom_mapping);

            result = pvr_msaa_state(dev_info, transfer_cmd, state, source);
            if (result != VK_SUCCESS)
               return result;

            if (state->filter[source] == PVR_FILTER_LINEAR &&
                pvr_requires_usc_linear_filter(src->surface.vk_format)) {
               if (pvr_int_pbe_usc_linear_filter(layer->pbe_format,
                                                 layer->sample,
                                                 layer->msaa,
                                                 shader_props->full_rate)) {
                  layer->linear = true;
               } else {
                  mesa_logw("Transfer: F32 linear filter not supported.");
               }
            }

            result = pvr_transfer_frag_store_get_shader_info(
               transfer_cmd->cmd_buffer->device,
               &ctx->frag_store,
               shader_props,
               &dev_offset,
               &sh_reg_layout);
            if (result != VK_SUCCESS)
               return result;

            assert(dev_offset.addr <= UINT32_MAX);
            prep_data->state.pds_shader_task_offset = (uint32_t)dev_offset.addr;

            result =
               pvr_pds_coeff_task(ctx, transfer_cmd, layer->sample, prep_data);
            if (result != VK_SUCCESS)
               return result;

            unitex_pds_prog.kick_usc = false;
            unitex_pds_prog.clear = false;

            tex_state_dma_size =
               sh_reg_layout->driver_total + sh_reg_layout->compiler_out_total;

            unitex_pds_prog.num_texture_dma_kicks = 1U;
            unitex_pds_prog.num_uniform_dma_kicks = 0U;

            /* Allocate memory for DMA. */
            result = pvr_cmd_buffer_alloc_mem(transfer_cmd->cmd_buffer,
                                              ctx->device->heaps.general_heap,
                                              tex_state_dma_size << 2U,
                                              &pvr_bo);
            if (result != VK_SUCCESS)
               return result;

            result = pvr_sampler_state_for_surface(
               dev_info,
               &transfer_cmd->sources[source].surface,
               state->filter[source],
               sh_reg_layout,
               0U,
               pvr_bo_suballoc_get_map_addr(pvr_bo));
            if (result != VK_SUCCESS)
               return result;

            result = pvr_image_state_for_surface(
               ctx,
               transfer_cmd,
               &transfer_cmd->sources[source].surface,
               0U,
               source,
               sh_reg_layout,
               state,
               0U,
               pvr_bo_suballoc_get_map_addr(pvr_bo));
            if (result != VK_SUCCESS)
               return result;

            pvr_pds_encode_dma_burst(unitex_pds_prog.texture_dma_control,
                                     unitex_pds_prog.texture_dma_address,
                                     state->common_ptr,
                                     tex_state_dma_size,
                                     pvr_bo->dev_addr.addr,
                                     true,
                                     dev_info);

            state->common_ptr += tex_state_dma_size;

            pvr_write_usc_constants(sh_reg_layout,
                                    pvr_bo_suballoc_get_map_addr(pvr_bo));

            if (pvr_pick_component_needed(&state->custom_mapping)) {
               pvr_dma_texel_unwind(state,
                                    sh_reg_layout,
                                    pvr_bo_suballoc_get_map_addr(pvr_bo));
            }
         }

         result = pvr_pds_unitex(dev_info,
                                 ctx,
                                 transfer_cmd,
                                 &unitex_pds_prog,
                                 prep_data);
         if (result != VK_SUCCESS)
            return result;

         while (rem_mappings > 0U) {
            const uint32_t min_free_ctrl_stream_words =
               PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format) ? 2
                                                                           : 3;
            const uint32_t num_mappings =
               MIN2(max_mappings_per_pb, rem_mappings);
            struct pvr_rect_mapping *mappings = NULL;
            uint32_t stream_start_offset = 0U;
            pvr_dev_addr_t prim_blk_addr;

            if (free_ctrl_stream_words < min_free_ctrl_stream_words) {
               pvr_dev_addr_t next_region_array_vaddr = stream_base_vaddr;

               num_region_arrays++;
               next_region_array_vaddr.addr +=
                  num_region_arrays *
                  PVR_DW_TO_BYTES(PVRX(IPF_CONTROL_STREAM_SIZE_DWORDS));

               if (PVR_HAS_FEATURE(dev_info,
                                   simple_internal_parameter_format_v2)) {
                  uint32_t link_addr;

                  pvr_csb_pack (&link_addr,
                                IPF_CONTROL_STREAM_LINK_SIPF2,
                                control_stream) {
                     control_stream.cs_ctrl_type =
                        PVRX(IPF_CS_CTRL_TYPE_SIPF2_LINK);
                     control_stream.cs_link.addr = next_region_array_vaddr.addr;
                  }

                  pvr_isp_ctrl_stream_sipf_write_aligned(
                     (uint8_t *)cs_ptr,
                     link_addr,
                     PVR_DW_TO_BYTES(
                        pvr_cmd_length(IPF_CONTROL_STREAM_LINK_SIPF2)));
               } else {
                  pvr_csb_pack (cs_ptr, IPF_CONTROL_STREAM, control_stream) {
                     control_stream.cs_type = PVRX(IPF_CS_TYPE_LINK);
                     control_stream.cs_link.addr = next_region_array_vaddr.addr;
                  }
               }

               cs_ptr =
                  (uint32_t *)pvr_bo_suballoc_get_map_addr(pvr_cs_bo) +
                  num_region_arrays * PVRX(IPF_CONTROL_STREAM_SIZE_DWORDS);
               free_ctrl_stream_words = PVRX(IPF_CONTROL_STREAM_SIZE_DWORDS);

               was_linked = PVR_HAS_FEATURE(dev_info, ipf_creq_pf);
            }

            if (fill_blit)
               mappings = pass ? pass->sources[source].mappings : &fill_mapping;
            else
               mappings = transfer_cmd->sources[source].mappings;

            prim_blk_addr = stream_base_vaddr;
            prim_blk_addr.addr +=
               (uintptr_t)blk_cs_ptr -
               (uintptr_t)pvr_bo_suballoc_get_map_addr(pvr_cs_bo);

            result = pvr_isp_primitive_block(dev_info,
                                             ctx,
                                             transfer_cmd,
                                             prep_data,
                                             fill_blit ? NULL : src,
                                             state->custom_filter,
                                             mappings,
                                             num_mappings,
                                             mapping_offset,
                                             read_bgnd,
                                             &stream_start_offset,
                                             &blk_cs_ptr);
            if (result != VK_SUCCESS)
               return result;

            prim_blk_addr.addr += stream_start_offset;

            if (PVR_HAS_FEATURE(dev_info,
                                simple_internal_parameter_format_v2)) {
               uint8_t *cs_byte_ptr = (uint8_t *)cs_ptr;
               uint32_t tmp;

               /* This part of the control stream is byte granular. */

               pvr_csb_pack (&tmp, IPF_PRIMITIVE_HEADER_SIPF2, prim_header) {
                  prim_header.cs_prim_base_size = 1;
                  prim_header.cs_mask_num_bytes = 1;
                  prim_header.cs_valid_tile0 = true;
               }
               cs_byte_ptr =
                  pvr_isp_ctrl_stream_sipf_write_aligned(cs_byte_ptr, tmp, 1);

               pvr_csb_pack (&tmp, IPF_PRIMITIVE_BASE_SIPF2, word) {
                  word.cs_prim_base = prim_blk_addr;
               }
               cs_byte_ptr =
                  pvr_isp_ctrl_stream_sipf_write_aligned(cs_byte_ptr, tmp, 4);

               /* IPF_BYTE_BASED_MASK_ONE_BYTE_WORD_0_SIPF2 since
                * IPF_PRIMITIVE_HEADER_SIPF2.cs_mask_num_bytes == 1.
                */
               pvr_csb_pack (&tmp,
                             IPF_BYTE_BASED_MASK_ONE_BYTE_WORD_0_SIPF2,
                             mask) {
                  switch (num_mappings) {
                  case 4:
                     mask.cs_mask_one_byte_tile0_7 = true;
                     mask.cs_mask_one_byte_tile0_6 = true;
                     FALLTHROUGH;
                  case 3:
                     mask.cs_mask_one_byte_tile0_5 = true;
                     mask.cs_mask_one_byte_tile0_4 = true;
                     FALLTHROUGH;
                  case 2:
                     mask.cs_mask_one_byte_tile0_3 = true;
                     mask.cs_mask_one_byte_tile0_2 = true;
                     FALLTHROUGH;
                  case 1:
                     mask.cs_mask_one_byte_tile0_1 = true;
                     mask.cs_mask_one_byte_tile0_0 = true;
                     break;
                  default:
                     /* Unreachable since we clamped the value earlier so
                      * reaching this is an implementation error.
                      */
                     unreachable("num_mapping exceeded max_mappings_per_pb");
                     break;
                  }
               }
               /* Only 1 byte since there's only 1 valid tile within the single
                * IPF_BYTE_BASED_MASK_ONE_BYTE_WORD_0_SIPF2 mask.
                * ROGUE_IPF_PRIMITIVE_HEADER_SIPF2.cs_valid_tile0 == true.
                */
               cs_byte_ptr =
                  pvr_isp_ctrl_stream_sipf_write_aligned(cs_byte_ptr, tmp, 1);

               cs_ptr = (uint32_t *)cs_byte_ptr;

               free_ctrl_stream_words -= 2;
            } else {
               pvr_csb_pack (cs_ptr, IPF_PRIMITIVE_FORMAT, word) {
                  word.cs_type = PVRX(IPF_CS_TYPE_PRIM);
                  word.cs_isp_state_read = true;
                  word.cs_isp_state_size = 2U;
                  word.cs_prim_total = 2U * num_mappings - 1U;
                  word.cs_mask_fmt = PVRX(IPF_CS_MASK_FMT_FULL);
                  word.cs_prim_base_pres = true;
               }
               cs_ptr += pvr_cmd_length(IPF_PRIMITIVE_FORMAT);

               pvr_csb_pack (cs_ptr, IPF_PRIMITIVE_BASE, word) {
                  word.cs_prim_base = prim_blk_addr;
               }
               cs_ptr += pvr_cmd_length(IPF_PRIMITIVE_BASE);

               free_ctrl_stream_words -= 2;
            }

            rem_mappings -= num_mappings;
            mapping_offset += num_mappings;
         }
      }

      source++;

      /* A fill blit may also have sources for normal blits. */
      if (fill_blit && transfer_cmd->source_count > 0) {
         /* Fill blit count for custom mapping equals source blit count. While
          * normal blits use only one fill blit.
          */
         if (state->custom_mapping.pass_count == 0 && source > num_sources) {
            fill_blit = false;
            source = 0;
         }
      }
   }

   if (PVR_HAS_FEATURE(dev_info, ipf_creq_pf))
      assert((num_region_arrays > 1) == was_linked);

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format_v2)) {
      uint8_t *cs_byte_ptr = (uint8_t *)cs_ptr;
      uint32_t tmp;

      /* clang-format off */
      pvr_csb_pack (&tmp, IPF_CONTROL_STREAM_TERMINATE_SIPF2, term);
      /* clang-format on */

      cs_byte_ptr = pvr_isp_ctrl_stream_sipf_write_aligned(cs_byte_ptr, tmp, 1);

      cs_ptr = (uint32_t *)cs_byte_ptr;
   } else {
      pvr_csb_pack (cs_ptr, IPF_CONTROL_STREAM, word) {
         word.cs_type = PVRX(IPF_CS_TYPE_TERM);
      }
      cs_ptr += pvr_cmd_length(IPF_CONTROL_STREAM);
   }

   pvr_csb_pack (&regs->isp_mtile_base, CR_ISP_MTILE_BASE, reg) {
      reg.addr =
         PVR_DEV_ADDR(pvr_cs_bo->dev_addr.addr -
                      ctx->device->heaps.transfer_frag_heap->base_addr.addr);
   }

   pvr_csb_pack (&regs->isp_render, CR_ISP_RENDER, reg) {
      reg.mode_type = PVRX(CR_ISP_RENDER_MODE_TYPE_FAST_2D);
   }

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format_v2) &&
       PVR_HAS_FEATURE(dev_info, ipf_creq_pf)) {
      pvr_csb_pack (&regs->isp_rgn, CR_ISP_RGN_SIPF, isp_rgn) {
         /* Bit 0 in CR_ISP_RGN.cs_size_ipf_creq_pf is used to indicate the
          * presence of a link.
          */
         isp_rgn.cs_size_ipf_creq_pf = was_linked;
      }
   } else {
      /* clang-format off */
      pvr_csb_pack(&regs->isp_rgn, CR_ISP_RGN, isp_rgn);
      /* clang-format on */
   }

   return VK_SUCCESS;
}

static void pvr_transfer_set_filter(struct pvr_transfer_cmd *transfer_cmd,
                                    struct pvr_transfer_3d_state *state)
{
   for (uint32_t i = 0; i < transfer_cmd->source_count; i++) {
      VkRect2D *src = &transfer_cmd->sources[i].mappings[0U].src_rect;
      VkRect2D *dst = &transfer_cmd->sources[i].mappings[0U].dst_rect;

      /* If no scaling is applied to the copy region, we can use point
       * filtering.
       */
      if (!state->custom_filter && (src->extent.width == dst->extent.width) &&
          (src->extent.height == dst->extent.height))
         state->filter[i] = PVR_FILTER_POINT;
      else
         state->filter[i] = transfer_cmd->sources[i].filter;
   }
}

/** Generates hw resources to kick a 3D clip blit. */
static VkResult pvr_3d_clip_blit(struct pvr_transfer_ctx *ctx,
                                 struct pvr_transfer_cmd *transfer_cmd,
                                 struct pvr_transfer_prep_data *prep_data,
                                 uint32_t pass_idx,
                                 bool *finished_out)
{
   struct pvr_transfer_3d_state *state = &prep_data->state;
   uint32_t texel_unwind_src = state->custom_mapping.texel_unwind_src;
   struct pvr_transfer_cmd bg_cmd = { 0U };
   uint32_t control_reg;
   VkResult result;

   state->dont_force_pbe = false;
   bg_cmd.scissor = transfer_cmd->scissor;
   bg_cmd.cmd_buffer = transfer_cmd->cmd_buffer;
   bg_cmd.flags = transfer_cmd->flags;
   bg_cmd.flags &=
      ~(PVR_TRANSFER_CMD_FLAGS_FAST2D | PVR_TRANSFER_CMD_FLAGS_FILL |
        PVR_TRANSFER_CMD_FLAGS_DSMERGE | PVR_TRANSFER_CMD_FLAGS_PICKD);

   bg_cmd.source_count = state->custom_mapping.pass_count > 0U ? 0 : 1;
   if (bg_cmd.source_count > 0) {
      struct pvr_transfer_cmd_source *src = &bg_cmd.sources[0];

      src->mappings[0U].src_rect = transfer_cmd->scissor;
      src->mappings[0U].dst_rect = transfer_cmd->scissor;
      src->resolve_op = PVR_RESOLVE_BLEND;
      src->surface = transfer_cmd->dst;
   }

   state->filter[0] = PVR_FILTER_DONTCARE;
   bg_cmd.dst = transfer_cmd->dst;
   state->custom_mapping.texel_unwind_src =
      state->custom_mapping.texel_unwind_dst;

   result =
      pvr_3d_copy_blit_core(ctx, &bg_cmd, prep_data, pass_idx, finished_out);
   if (result != VK_SUCCESS)
      return result;

   /* If the destination has 4 channels and the source has at most 2, we still
    * need all 4 channels from the USC into the PBE.
    */
   state->dont_force_pbe = true;
   state->custom_mapping.texel_unwind_src = texel_unwind_src;

   /* We need the viewport mask, otherwise all pixels would be disabled. */
   pvr_csb_pack (&control_reg, CR_ISP_BGOBJVALS, reg) {
      reg.mask = true;
   }
   state->regs.isp_bgobjvals |= control_reg;

   pvr_transfer_set_filter(transfer_cmd, state);
   result = pvr_isp_ctrl_stream(&ctx->device->pdevice->dev_info,
                                ctx,
                                transfer_cmd,
                                prep_data);
   if (result != VK_SUCCESS)
      return result;

   /* In case of resolve M -> S, the accumulation is read from and written to a
    * single sampled surface. Make sure that we are resolving and we have the
    * right number of tiles.
    */
   if (state->down_scale) {
      uint64_t tmp;

      pvr_csb_pack (&tmp, CR_PBE_WORD0_MRT0, reg) {
         reg.downscale = true;
      }
      state->regs.pbe_wordx_mrty[0U] |= tmp;

      result = pvr_isp_tiles(ctx->device, state);
      if (result != VK_SUCCESS)
         return result;
   }

   return VK_SUCCESS;
}

static bool pvr_texel_unwind(uint32_t bpp,
                             pvr_dev_addr_t dev_addr,
                             bool is_input,
                             uint32_t texel_extend,
                             uint32_t *texel_unwind_out)
{
   uint32_t texel_unwind = 0U;

   for (uint32_t i = 0U; i < 16U; i++) {
      if (pvr_is_surface_aligned(dev_addr, is_input, bpp)) {
         break;
      } else {
         if (i == 15U) {
            return false;
         } else {
            dev_addr.addr -= (bpp / texel_extend) / 8U;
            texel_unwind++;
         }
      }
   }

   *texel_unwind_out = texel_unwind;

   return true;
}

static bool pvr_is_identity_mapping(const struct pvr_rect_mapping *mapping)
{
   return (mapping->src_rect.offset.x == mapping->dst_rect.offset.x &&
           mapping->src_rect.offset.y == mapping->dst_rect.offset.y &&
           mapping->src_rect.extent.width == mapping->dst_rect.extent.width &&
           mapping->src_rect.extent.height == mapping->dst_rect.extent.height);
}

static inline bool pvr_is_pbe_stride_aligned(const uint32_t stride)
{
   if (stride == 1U)
      return true;

   return ((stride & (PVRX(PBESTATE_REG_WORD0_LINESTRIDE_UNIT_SIZE) - 1U)) ==
           0x0U);
}

static struct pvr_transfer_pass *
pvr_create_pass(struct pvr_transfer_custom_mapping *custom_mapping,
                uint32_t dst_offset)
{
   struct pvr_transfer_pass *pass;

   assert(custom_mapping->pass_count < PVR_TRANSFER_MAX_PASSES);

   pass = &custom_mapping->passes[custom_mapping->pass_count];
   pass->clip_rects_count = 0U;
   pass->dst_offset = dst_offset;
   pass->source_count = 0U;

   custom_mapping->pass_count++;

   return pass;
}

/* Acquire pass with given offset. If one doesn't exist, create new. */
static struct pvr_transfer_pass *
pvr_acquire_pass(struct pvr_transfer_custom_mapping *custom_mapping,
                 uint32_t dst_offset)
{
   for (uint32_t i = 0U; i < custom_mapping->pass_count; i++) {
      if (custom_mapping->passes[i].dst_offset == dst_offset)
         return &custom_mapping->passes[i];
   }

   return pvr_create_pass(custom_mapping, dst_offset);
}

static struct pvr_transfer_wa_source *
pvr_create_source(struct pvr_transfer_pass *pass,
                  uint32_t src_offset,
                  bool extend_height)
{
   struct pvr_transfer_wa_source *src;

   assert(pass->source_count < ARRAY_SIZE(pass->sources));

   src = &pass->sources[pass->source_count];
   src->mapping_count = 0U;
   src->extend_height = extend_height;

   pass->source_count++;

   return src;
}

/* Acquire source with given offset. If one doesn't exist, create new. */
static struct pvr_transfer_wa_source *
pvr_acquire_source(struct pvr_transfer_pass *pass,
                   uint32_t src_offset,
                   bool extend_height)
{
   for (uint32_t i = 0U; i < pass->source_count; i++) {
      if (pass->sources[i].src_offset == src_offset &&
          pass->sources[i].extend_height == extend_height)
         return &pass->sources[i];
   }

   return pvr_create_source(pass, src_offset, extend_height);
}

static void pvr_remove_source(struct pvr_transfer_pass *pass, uint32_t idx)
{
   assert(idx < pass->source_count);

   for (uint32_t i = idx; i < (pass->source_count - 1U); i++)
      pass->sources[i] = pass->sources[i + 1U];

   pass->source_count--;
}

static void pvr_remove_mapping(struct pvr_transfer_wa_source *src, uint32_t idx)
{
   assert(idx < src->mapping_count);

   for (uint32_t i = idx; i < (src->mapping_count - 1U); i++)
      src->mappings[i] = src->mappings[i + 1U];

   src->mapping_count--;
}

static struct pvr_rect_mapping *
pvr_create_mapping(struct pvr_transfer_wa_source *src)
{
   assert(src->mapping_count < ARRAY_SIZE(src->mappings));

   return &src->mappings[src->mapping_count++];
}

/**
 * If PBE can't write to surfaces with odd stride, the stride of
 * destination surface is doubled to make it even. Height of the surface is
 * halved. The source surface is not resized. Each half of the modified
 * destination surface samples every second row from the source surface. This
 * only works with nearest filtering.
 */
static bool pvr_double_stride(struct pvr_transfer_pass *pass, uint32_t stride)
{
   struct pvr_rect_mapping *mappings = pass->sources[0].mappings;
   uint32_t new_mapping = 0;

   if (stride == 1U)
      return false;

   if (mappings[0U].dst_rect.extent.height == 1U &&
       pass->sources[0].mapping_count == 1U) {
      /* Only one mapping required if height is 1. */
      if ((mappings[0U].dst_rect.offset.y & 1U) != 0U) {
         mappings[0U].dst_rect.offset.x += (int32_t)stride;
         mappings[0U].dst_rect.offset.y /= 2U;
         mappings[0U].dst_rect.extent.height =
            (mappings[0U].dst_rect.extent.height + 1U) / 2U;
      } else {
         mappings[0U].dst_rect.extent.height =
            (mappings[0U].dst_rect.offset.y +
             mappings[0U].dst_rect.extent.height + 1U) /
               2U -
            mappings[0U].dst_rect.offset.y;
         mappings[0U].dst_rect.offset.y /= 2U;
      }

      return true;
   }

   for (uint32_t i = 0; i < pass->sources[0].mapping_count; i++) {
      struct pvr_rect_mapping *mapping_a = &mappings[i];
      struct pvr_rect_mapping *mapping_b =
         &mappings[pass->sources[0].mapping_count + new_mapping];
      int32_t mapping_a_src_rect_y1 =
         mapping_a->src_rect.offset.y + mapping_a->src_rect.extent.height;
      int32_t mapping_b_src_rect_y1 = mapping_a_src_rect_y1;
      const bool dst_starts_odd_row = !!(mapping_a->dst_rect.offset.y & 1);
      const bool dst_ends_odd_row =
         !!((mapping_a->dst_rect.offset.y + mapping_a->dst_rect.extent.height) &
            1);
      const bool src_starts_odd_row = !!(mapping_a->src_rect.offset.y & 1);
      const bool src_ends_odd_row =
         !!((mapping_a->src_rect.offset.y + mapping_a->src_rect.extent.height) &
            1);

      assert(pass->sources[0].mapping_count + new_mapping <
             ARRAY_SIZE(pass->sources[0].mappings));
      *mapping_b = *mapping_a;

      mapping_a->src_rect.offset.y = ALIGN_POT(mapping_a->src_rect.offset.y, 2);
      if (dst_starts_odd_row && !src_starts_odd_row)
         mapping_a->src_rect.offset.y++;
      else if (!dst_starts_odd_row && src_starts_odd_row)
         mapping_a->src_rect.offset.y--;

      mapping_a_src_rect_y1 = ALIGN_POT(mapping_a_src_rect_y1, 2);
      if (dst_ends_odd_row && !src_ends_odd_row)
         mapping_a_src_rect_y1++;
      else if (!dst_ends_odd_row && src_ends_odd_row)
         mapping_a_src_rect_y1--;

      mapping_a->src_rect.extent.height =
         mapping_a_src_rect_y1 - mapping_a->src_rect.offset.y;

      mapping_b->src_rect.offset.y = ALIGN_POT(mapping_b->src_rect.offset.y, 2);
      if (dst_starts_odd_row && src_starts_odd_row)
         mapping_b->src_rect.offset.y--;
      else if (!dst_starts_odd_row && !src_starts_odd_row)
         mapping_b->src_rect.offset.y++;

      mapping_b_src_rect_y1 = ALIGN_POT(mapping_b_src_rect_y1, 2);
      if (dst_ends_odd_row && src_ends_odd_row)
         mapping_b_src_rect_y1--;
      else if (!dst_ends_odd_row && !src_ends_odd_row)
         mapping_b_src_rect_y1++;

      mapping_b->src_rect.extent.height =
         mapping_b_src_rect_y1 - mapping_b->src_rect.offset.y;

      /* Destination rectangles. */
      mapping_a->dst_rect.offset.y = mapping_a->dst_rect.offset.y / 2;

      if (dst_starts_odd_row)
         mapping_a->dst_rect.offset.y++;

      mapping_b->dst_rect.offset.x += stride;
      mapping_b->dst_rect.offset.y /= 2;
      mapping_b->dst_rect.extent.height /= 2;
      mapping_a->dst_rect.extent.height -= mapping_b->dst_rect.extent.height;

      if (!mapping_a->src_rect.extent.width ||
          !mapping_a->src_rect.extent.height) {
         *mapping_a = *mapping_b;
      } else if (mapping_b->src_rect.extent.width &&
                 mapping_b->src_rect.extent.height) {
         new_mapping++;
      }
   }

   pass->sources[0].mapping_count++;

   return true;
}

static void pvr_split_rect(uint32_t stride,
                           uint32_t height,
                           uint32_t texel_unwind,
                           VkRect2D *rect_a,
                           VkRect2D *rect_b)
{
   rect_a->offset.x = 0;
   rect_a->extent.width = stride - texel_unwind;
   rect_a->offset.y = 0;
   rect_a->extent.height = height;

   rect_b->offset.x = (int32_t)stride - texel_unwind;
   rect_b->extent.width = texel_unwind;
   rect_b->offset.y = 0;
   rect_b->extent.height = height;
}

static bool pvr_rect_width_covered_by(const VkRect2D *rect_a,
                                      const VkRect2D *rect_b)
{
   return (rect_b->offset.x <= rect_a->offset.x &&
           (rect_b->offset.x + rect_b->extent.width) >=
              (rect_a->offset.x + rect_a->extent.width));
}

static void pvr_unwind_rects(uint32_t width,
                             uint32_t height,
                             uint32_t texel_unwind,
                             bool input,
                             struct pvr_transfer_pass *pass)
{
   struct pvr_transfer_wa_source *const source = &pass->sources[0];
   struct pvr_rect_mapping *const mappings = source->mappings;
   const uint32_t num_mappings = source->mapping_count;
   VkRect2D rect_a, rect_b;

   if (texel_unwind == 0)
      return;

   pvr_split_rect(width, height, texel_unwind, &rect_a, &rect_b);

   for (uint32_t i = 0; i < num_mappings; i++) {
      VkRect2D *const old_rect = input ? &mappings[i].src_rect
                                       : &mappings[i].dst_rect;

      if (height == 1) {
         old_rect->offset.x += texel_unwind;
      } else if (width == 1) {
         old_rect->offset.y += texel_unwind;
      } else if (pvr_rect_width_covered_by(old_rect, &rect_a)) {
         old_rect->offset.x += texel_unwind;
      } else if (pvr_rect_width_covered_by(old_rect, &rect_b)) {
         old_rect->offset.x = texel_unwind - width + old_rect->offset.x;
         old_rect->offset.y++;
      } else {
         /* Mapping requires split. */
         const uint32_t new_mapping = source->mapping_count++;

         VkRect2D *const new_rect = input ? &mappings[new_mapping].src_rect
                                          : &mappings[new_mapping].dst_rect;

         VkRect2D *const new_rect_opp = input ? &mappings[new_mapping].dst_rect
                                              : &mappings[new_mapping].src_rect;
         VkRect2D *const old_rect_opp = input ? &mappings[i].dst_rect
                                              : &mappings[i].src_rect;

         const uint32_t split_point = width - texel_unwind;
         const uint32_t split_width =
            old_rect->offset.x + old_rect->extent.width - split_point;

         assert(new_mapping < ARRAY_SIZE(source->mappings));
         mappings[new_mapping] = mappings[i];

         old_rect_opp->extent.width -= split_width;
         new_rect_opp->extent.width = split_width;
         new_rect_opp->offset.x =
            old_rect_opp->offset.x + old_rect_opp->extent.width;

         old_rect->offset.x += texel_unwind;
         old_rect->extent.width = width - old_rect->offset.x;

         new_rect->offset.x = 0;
         new_rect->offset.y++;
         new_rect->extent.width = split_width;
      }
   }
}

/**
 * Assign clip rects to rectangle mappings. TDM can only do two PBE clip
 * rects per screen.
 */
static void
pvr_map_clip_rects(struct pvr_transfer_custom_mapping *custom_mapping)
{
   for (uint32_t i = 0U; i < custom_mapping->pass_count; i++) {
      struct pvr_transfer_pass *pass = &custom_mapping->passes[i];

      pass->clip_rects_count = 0U;

      for (uint32_t s = 0U; s < pass->source_count; s++) {
         struct pvr_transfer_wa_source *src = &pass->sources[s];

         for (uint32_t j = 0U; j < src->mapping_count; j++) {
            struct pvr_rect_mapping *mappings = src->mappings;
            VkRect2D *clip_rects = pass->clip_rects;
            bool merged = false;

            /* Try merge adjacent clip rects. */
            for (uint32_t k = 0U; k < pass->clip_rects_count; k++) {
               if (clip_rects[k].offset.y == mappings[j].dst_rect.offset.y &&
                   clip_rects[k].extent.height ==
                      mappings[j].dst_rect.extent.height &&
                   clip_rects[k].offset.x + clip_rects[k].extent.width ==
                      mappings[j].dst_rect.offset.x) {
                  clip_rects[k].extent.width +=
                     mappings[j].dst_rect.extent.width;
                  merged = true;
                  break;
               }

               if (clip_rects[k].offset.y == mappings[j].dst_rect.offset.y &&
                   clip_rects[k].extent.height ==
                      mappings[j].dst_rect.extent.height &&
                   clip_rects[k].offset.x ==
                      mappings[j].dst_rect.offset.x +
                         mappings[j].dst_rect.extent.width) {
                  clip_rects[k].offset.x = mappings[j].dst_rect.offset.x;
                  clip_rects[k].extent.width +=
                     mappings[j].dst_rect.extent.width;
                  merged = true;
                  break;
               }

               if (clip_rects[k].offset.x == mappings[j].dst_rect.offset.x &&
                   clip_rects[k].extent.width ==
                      mappings[j].dst_rect.extent.width &&
                   clip_rects[k].offset.y + clip_rects[k].extent.height ==
                      mappings[j].dst_rect.offset.y) {
                  clip_rects[k].extent.height +=
                     mappings[j].dst_rect.extent.height;
                  merged = true;
                  break;
               }

               if (clip_rects[k].offset.x == mappings[j].dst_rect.offset.x &&
                   clip_rects[k].extent.width ==
                      mappings[j].dst_rect.extent.width &&
                   clip_rects[k].offset.y ==
                      mappings[j].dst_rect.offset.y +
                         mappings[j].dst_rect.extent.height) {
                  clip_rects[k].extent.height +=
                     mappings[j].dst_rect.extent.height;
                  clip_rects[k].offset.y = mappings[j].dst_rect.offset.y;
                  merged = true;
                  break;
               }
            }

            if (merged)
               continue;

            /* Create new pass if needed, TDM can only have 2 clip rects. */
            if (pass->clip_rects_count >= custom_mapping->max_clip_rects) {
               struct pvr_transfer_pass *new_pass =
                  pvr_create_pass(custom_mapping, pass->dst_offset);
               struct pvr_transfer_wa_source *new_source =
                  pvr_create_source(new_pass,
                                    src->src_offset,
                                    src->extend_height);
               struct pvr_rect_mapping *new_mapping =
                  pvr_create_mapping(new_source);

               new_pass->clip_rects_count = 1U;
               *new_mapping = src->mappings[j];

               pvr_remove_mapping(src, j);

               if (src->mapping_count == 0) {
                  pvr_remove_source(pass, s);
                  s--;
               } else {
                  /* Redo - mapping was replaced. */
                  j--;
               }
            } else {
               pass->clip_rects[pass->clip_rects_count] =
                  src->mappings[j].dst_rect;

               pass->clip_rects_count++;

               assert(pass->clip_rects_count <= ARRAY_SIZE(pass->clip_rects));
            }
         }
      }
   }
}

static bool pvr_extend_height(const VkRect2D *rect,
                              const uint32_t height,
                              const uint32_t unwind_src)
{
   if (rect->offset.x >= (int32_t)unwind_src)
      return false;

   return (rect->offset.y > (int32_t)height) ||
          ((rect->offset.y + rect->extent.height) > (int32_t)height);
}

static void
pvr_generate_custom_mapping(uint32_t src_stride,
                            uint32_t src_width,
                            uint32_t src_height,
                            uint32_t dst_stride,
                            uint32_t dst_width,
                            uint32_t dst_height,
                            enum pvr_memlayout dst_mem_layout,
                            struct pvr_transfer_custom_mapping *custom_mapping)
{
   src_stride *= custom_mapping->texel_extend_src;
   src_width *= custom_mapping->texel_extend_src;
   dst_stride *= custom_mapping->texel_extend_dst;
   dst_width *= custom_mapping->texel_extend_dst;

   if (custom_mapping->texel_unwind_src > 0U) {
      pvr_unwind_rects(src_stride,
                       src_height,
                       custom_mapping->texel_unwind_src,
                       true,
                       &custom_mapping->passes[0U]);
   }

   if (custom_mapping->double_stride) {
      custom_mapping->double_stride =
         pvr_double_stride(&custom_mapping->passes[0U], dst_stride);

      dst_stride *= 2U;
   }

   pvr_unwind_rects(dst_stride,
                    dst_height,
                    custom_mapping->texel_unwind_dst,
                    false,
                    &custom_mapping->passes[0U]);

   pvr_map_clip_rects(custom_mapping);

   /* If the last row of the source mapping is sampled, height of the surface
    * can only be increased if the new area contains a valid region. Some blits
    * are split to two sources.
    */
   if (custom_mapping->texel_unwind_src > 0U) {
      for (uint32_t i = 0; i < custom_mapping->pass_count; i++) {
         struct pvr_transfer_pass *pass = &custom_mapping->passes[i];

         for (uint32_t j = 0; j < pass->source_count; j++) {
            struct pvr_transfer_wa_source *src = &pass->sources[j];

            for (uint32_t k = 0; k < src->mapping_count; k++) {
               VkRect2D *src_rect = &src->mappings[k].src_rect;
               bool extend_height =
                  pvr_extend_height(src_rect,
                                    src_height,
                                    custom_mapping->texel_unwind_src);

               if (src->mapping_count == 1) {
                  src->extend_height = extend_height;
               } else if (!src->extend_height && extend_height) {
                  struct pvr_transfer_wa_source *new_src =
                     pvr_acquire_source(pass, src->src_offset, extend_height);

                  new_src->mappings[new_src->mapping_count] = src->mappings[k];
                  new_src->src_offset = src->src_offset;

                  for (uint32_t l = k + 1; l < src->mapping_count; l++)
                     src->mappings[l - 1] = src->mappings[l];

                  new_src->mapping_count++;
                  src->mapping_count--;
                  k--;
               }
            }
         }
      }
   }
}

static bool
pvr_get_custom_mapping(const struct pvr_device_info *dev_info,
                       const struct pvr_transfer_cmd *transfer_cmd,
                       uint32_t max_clip_rects,
                       struct pvr_transfer_custom_mapping *custom_mapping)
{
   const uint32_t dst_bpp =
      vk_format_get_blocksizebits(transfer_cmd->dst.vk_format);
   const struct pvr_transfer_cmd_source *src = NULL;
   struct pvr_transfer_pass *pass;
   bool ret;

   custom_mapping->max_clip_rects = max_clip_rects;
   custom_mapping->texel_unwind_src = 0U;
   custom_mapping->texel_unwind_dst = 0U;
   custom_mapping->texel_extend_src = 1U;
   custom_mapping->texel_extend_dst = 1U;
   custom_mapping->pass_count = 0U;

   if (transfer_cmd->source_count > 1)
      return false;

   custom_mapping->max_clip_size = PVR_MAX_CLIP_SIZE(dev_info);

   ret = pvr_texel_unwind(dst_bpp,
                          transfer_cmd->dst.dev_addr,
                          false,
                          1U,
                          &custom_mapping->texel_unwind_dst);
   if (!ret) {
      custom_mapping->texel_extend_dst = dst_bpp / 8U;
      if (transfer_cmd->source_count > 0) {
         if (transfer_cmd->sources[0].surface.mem_layout ==
             PVR_MEMLAYOUT_LINEAR) {
            custom_mapping->texel_extend_src = custom_mapping->texel_extend_dst;
         } else if (transfer_cmd->sources[0].surface.mem_layout ==
                       PVR_MEMLAYOUT_TWIDDLED &&
                    transfer_cmd->sources[0].surface.height == 1U) {
            custom_mapping->texel_extend_src = custom_mapping->texel_extend_dst;
         }
      }

      ret = pvr_texel_unwind(dst_bpp,
                             transfer_cmd->dst.dev_addr,
                             false,
                             custom_mapping->texel_extend_dst,
                             &custom_mapping->texel_unwind_dst);
      if (!ret)
         return false;
   }

   if (transfer_cmd->source_count > 0) {
      src = &transfer_cmd->sources[0];
      const uint32_t src_bpp =
         vk_format_get_blocksizebits(src->surface.vk_format);

      ret = pvr_is_surface_aligned(src->surface.dev_addr, true, src_bpp);

      if (!ret && (src->surface.mem_layout == PVR_MEMLAYOUT_LINEAR ||
                   src->surface.height == 1U)) {
         ret = pvr_texel_unwind(src_bpp,
                                src->surface.dev_addr,
                                true,
                                custom_mapping->texel_extend_src,
                                &custom_mapping->texel_unwind_src);
      }

      if (!ret) {
         custom_mapping->texel_extend_src = dst_bpp / 8U;
         custom_mapping->texel_extend_dst = custom_mapping->texel_extend_src;

         ret = pvr_texel_unwind(src_bpp,
                                src->surface.dev_addr,
                                true,
                                custom_mapping->texel_extend_src,
                                &custom_mapping->texel_unwind_src);
      }

      if (!ret)
         return false;
   }

   VkRect2D rect = transfer_cmd->scissor;
   assert(
      (rect.offset.x + rect.extent.width) <= custom_mapping->max_clip_size &&
      (rect.offset.y + rect.extent.height) <= custom_mapping->max_clip_size);

   /* Texel extend only works with strided memory layout, because pixel width is
    * changed. Texel unwind only works with strided memory layout. 1D blits are
    * allowed.
    */
   if (src && src->surface.height > 1U &&
       (custom_mapping->texel_extend_src > 1U ||
        custom_mapping->texel_unwind_src > 0U) &&
       src->surface.mem_layout != PVR_MEMLAYOUT_LINEAR) {
      return false;
   }

   /* Texel extend only works with strided memory layout, because pixel width is
    * changed. Texel unwind only works with strided memory layout. 1D blits are
    * allowed.
    */
   if ((custom_mapping->texel_extend_dst > 1U ||
        custom_mapping->texel_unwind_dst > 0U) &&
       transfer_cmd->dst.mem_layout != PVR_MEMLAYOUT_LINEAR &&
       transfer_cmd->dst.height > 1U) {
      return false;
   }

   if (transfer_cmd->dst.mem_layout == PVR_MEMLAYOUT_LINEAR) {
      custom_mapping->double_stride = !pvr_is_pbe_stride_aligned(
         transfer_cmd->dst.stride * custom_mapping->texel_extend_dst);
   }

   if (custom_mapping->texel_unwind_src > 0U ||
       custom_mapping->texel_unwind_dst > 0U || custom_mapping->double_stride) {
      struct pvr_transfer_wa_source *wa_src;
      struct pvr_rect_mapping *mapping;

      pass = pvr_acquire_pass(custom_mapping, 0U);
      wa_src = pvr_create_source(pass, 0U, false);
      mapping = pvr_create_mapping(wa_src);

      if (transfer_cmd->source_count > 0) {
         *mapping = src->mappings[0U];
      } else {
         mapping->src_rect = transfer_cmd->scissor;
         mapping->dst_rect = transfer_cmd->scissor;
      }
   } else {
      return false;
   }

   if (custom_mapping->texel_extend_src > 1U ||
       custom_mapping->texel_extend_dst > 1U) {
      pass->sources[0].mappings[0U].src_rect.offset.x *=
         (int32_t)custom_mapping->texel_extend_dst;
      pass->sources[0].mappings[0U].src_rect.extent.width *=
         (int32_t)custom_mapping->texel_extend_dst;
      pass->sources[0].mappings[0U].dst_rect.offset.x *=
         (int32_t)custom_mapping->texel_extend_dst;
      pass->sources[0].mappings[0U].dst_rect.extent.width *=
         (int32_t)custom_mapping->texel_extend_dst;
   }

   if (transfer_cmd->source_count > 0) {
      pvr_generate_custom_mapping(transfer_cmd->sources[0].surface.stride,
                                  transfer_cmd->sources[0].surface.width,
                                  transfer_cmd->sources[0].surface.height,
                                  transfer_cmd->dst.stride,
                                  transfer_cmd->dst.width,
                                  transfer_cmd->dst.height,
                                  transfer_cmd->dst.mem_layout,
                                  custom_mapping);
   } else {
      pvr_generate_custom_mapping(0U,
                                  0U,
                                  0U,
                                  transfer_cmd->dst.stride,
                                  transfer_cmd->dst.width,
                                  transfer_cmd->dst.height,
                                  transfer_cmd->dst.mem_layout,
                                  custom_mapping);
   }

   return true;
}

static void pvr_pbe_extend_rect(uint32_t texel_extend, VkRect2D *rect)
{
   rect->offset.x *= texel_extend;
   rect->extent.width *= texel_extend;
}

static void pvr_pbe_rect_intersect(VkRect2D *rect_a, VkRect2D *rect_b)
{
   rect_a->extent.width = MIN2(rect_a->offset.x + rect_a->extent.width,
                               rect_b->offset.x + rect_b->extent.width) -
                          MAX2(rect_a->offset.x, rect_b->offset.x);
   rect_a->offset.x = MAX2(rect_a->offset.x, rect_b->offset.x);
   rect_a->extent.height = MIN2(rect_a->offset.y + rect_a->extent.height,
                                rect_b->offset.y + rect_b->extent.height) -
                           MAX2(rect_a->offset.y, rect_b->offset.y);
   rect_a->offset.y = MAX2(rect_a->offset.y, rect_b->offset.y);
}

static VkFormat pvr_texel_extend_src_format(VkFormat vk_format)
{
   uint32_t bpp = vk_format_get_blocksizebits(vk_format);
   VkFormat ext_format;

   switch (bpp) {
   case 16:
      ext_format = VK_FORMAT_R8G8_UINT;
      break;
   case 32:
      ext_format = VK_FORMAT_R8G8B8A8_UINT;
      break;
   case 48:
      ext_format = VK_FORMAT_R16G16B16_UINT;
      break;
   default:
      ext_format = VK_FORMAT_R8_UINT;
      break;
   }

   return ext_format;
}

static void
pvr_modify_command(struct pvr_transfer_custom_mapping *custom_mapping,
                   uint32_t pass_idx,
                   struct pvr_transfer_cmd *transfer_cmd)
{
   struct pvr_transfer_pass *pass = &custom_mapping->passes[pass_idx];
   uint32_t bpp;

   if (custom_mapping->texel_extend_src > 1U) {
      struct pvr_rect_mapping *mapping = &transfer_cmd->sources[0].mappings[0];

      pvr_pbe_extend_rect(custom_mapping->texel_extend_src, &mapping->dst_rect);
      pvr_pbe_extend_rect(custom_mapping->texel_extend_src, &mapping->src_rect);

      transfer_cmd->dst.vk_format = VK_FORMAT_R8_UINT;
      transfer_cmd->dst.width *= custom_mapping->texel_extend_src;
      transfer_cmd->dst.stride *= custom_mapping->texel_extend_src;
      transfer_cmd->sources[0].surface.vk_format = VK_FORMAT_R8_UINT;
      transfer_cmd->sources[0].surface.width *=
         custom_mapping->texel_extend_src;
      transfer_cmd->sources[0].surface.stride *=
         custom_mapping->texel_extend_src;
   } else if (custom_mapping->texel_extend_dst > 1U) {
      VkRect2D max_clip = {
         .offset = { 0, 0 },
         .extent = { custom_mapping->max_clip_size,
                     custom_mapping->max_clip_size },
      };

      pvr_pbe_extend_rect(custom_mapping->texel_extend_dst,
                          &transfer_cmd->scissor);

      pvr_pbe_rect_intersect(&transfer_cmd->scissor, &max_clip);

      if (transfer_cmd->source_count > 0) {
         transfer_cmd->sources[0].surface.width *=
            custom_mapping->texel_extend_dst;
         transfer_cmd->sources[0].surface.stride *=
            custom_mapping->texel_extend_dst;

         transfer_cmd->sources[0].surface.vk_format =
            pvr_texel_extend_src_format(
               transfer_cmd->sources[0].surface.vk_format);
      }

      transfer_cmd->dst.vk_format = VK_FORMAT_R8_UINT;
      transfer_cmd->dst.width *= custom_mapping->texel_extend_dst;
      transfer_cmd->dst.stride *= custom_mapping->texel_extend_dst;
   }

   if (custom_mapping->double_stride) {
      transfer_cmd->dst.width *= 2U;
      transfer_cmd->dst.stride *= 2U;
   }

   if (custom_mapping->texel_unwind_src > 0U) {
      if (transfer_cmd->sources[0].surface.height == 1U) {
         transfer_cmd->sources[0].surface.width +=
            custom_mapping->texel_unwind_src;
         transfer_cmd->sources[0].surface.stride +=
            custom_mapping->texel_unwind_src;
      } else if (transfer_cmd->sources[0].surface.stride == 1U) {
         transfer_cmd->sources[0].surface.height +=
            custom_mapping->texel_unwind_src;
      } else {
         /* Increase source width by texel unwind. If texel unwind is less than
          * the distance between width and stride. The blit can be done with one
          * rectangle mapping, but the width of the surface needs be to
          * increased in case we sample from the area between width and stride.
          */
         transfer_cmd->sources[0].surface.width =
            MIN2(transfer_cmd->sources[0].surface.width +
                    custom_mapping->texel_unwind_src,
                 transfer_cmd->sources[0].surface.stride);
      }
   }

   for (uint32_t i = 0U; i < pass->source_count; i++) {
      struct pvr_transfer_wa_source *src = &pass->sources[i];

      if (i > 0)
         transfer_cmd->sources[i] = transfer_cmd->sources[0];

      transfer_cmd->sources[i].mapping_count = src->mapping_count;
      for (uint32_t j = 0U; j < transfer_cmd->sources[i].mapping_count; j++)
         transfer_cmd->sources[i].mappings[j] = src->mappings[j];

      if (src->extend_height)
         transfer_cmd->sources[i].surface.height += 1U;

      transfer_cmd->sources[i].surface.width =
         MIN2(PVR_MAX_WIDTH, transfer_cmd->sources[i].surface.width);
      transfer_cmd->sources[i].surface.height =
         MIN2(PVR_MAX_WIDTH, transfer_cmd->sources[i].surface.height);
      transfer_cmd->sources[i].surface.stride =
         MIN2(PVR_MAX_WIDTH, transfer_cmd->sources[i].surface.stride);
   }

   if (transfer_cmd->dst.height == 1U) {
      transfer_cmd->dst.width =
         transfer_cmd->dst.stride + custom_mapping->texel_unwind_dst;
      transfer_cmd->dst.mem_layout = PVR_MEMLAYOUT_TWIDDLED;
   }

   if (transfer_cmd->dst.mem_layout == PVR_MEMLAYOUT_TWIDDLED) {
      transfer_cmd->dst.width =
         MIN2((uint32_t)custom_mapping->max_clip_size, transfer_cmd->dst.width);
      transfer_cmd->dst.height = MIN2((uint32_t)custom_mapping->max_clip_size,
                                      transfer_cmd->dst.height);
   } else {
      transfer_cmd->dst.width = MIN2(PVR_MAX_WIDTH, transfer_cmd->dst.width);
   }

   if (transfer_cmd->source_count > 0) {
      for (uint32_t i = 0; i < pass->source_count; i++) {
         struct pvr_transfer_cmd_source *src = &transfer_cmd->sources[i];

         bpp = vk_format_get_blocksizebits(src->surface.vk_format);

         src->surface.dev_addr.addr -=
            custom_mapping->texel_unwind_src * bpp / 8U;
         src->surface.dev_addr.addr += MAX2(src->surface.sample_count, 1U) *
                                       pass->sources[i].src_offset * bpp / 8U;
      }
   }

   bpp = vk_format_get_blocksizebits(transfer_cmd->dst.vk_format);
   transfer_cmd->dst.dev_addr.addr -=
      custom_mapping->texel_unwind_dst * bpp / 8U;
   transfer_cmd->dst.dev_addr.addr +=
      MAX2(transfer_cmd->dst.sample_count, 1U) * pass->dst_offset * bpp / 8U;

   if (transfer_cmd->source_count > 0)
      transfer_cmd->source_count = pass->source_count;
}

/* Route a copy_blit (FastScale HW) to a clip_blit (Fast2D HW).
 * Destination rectangle can be specified in dst_rect, or NULL to use existing.
 */
static VkResult pvr_reroute_to_clip(struct pvr_transfer_ctx *ctx,
                                    const struct pvr_transfer_cmd *transfer_cmd,
                                    const struct VkRect2D *dst_rect,
                                    struct pvr_transfer_prep_data *prep_data,
                                    uint32_t pass_idx,
                                    bool *finished_out)
{
   struct pvr_transfer_cmd clip_transfer_cmd;

   clip_transfer_cmd = *transfer_cmd;
   clip_transfer_cmd.flags |= PVR_TRANSFER_CMD_FLAGS_FAST2D;

   if (transfer_cmd->source_count <= 1U) {
      if (dst_rect)
         clip_transfer_cmd.scissor = *dst_rect;

      return pvr_3d_clip_blit(ctx,
                              &clip_transfer_cmd,
                              prep_data,
                              pass_idx,
                              finished_out);
   }

   return vk_error(ctx->device, VK_ERROR_FORMAT_NOT_SUPPORTED);
}

static VkResult pvr_3d_copy_blit(struct pvr_transfer_ctx *ctx,
                                 struct pvr_transfer_cmd *transfer_cmd,
                                 struct pvr_transfer_prep_data *prep_data,
                                 uint32_t pass_idx,
                                 bool *finished_out)
{
   const struct pvr_device_info *const dev_info =
      &ctx->device->pdevice->dev_info;

   struct pvr_transfer_3d_state *state = &prep_data->state;
   struct pvr_transfer_cmd *active_cmd = transfer_cmd;
   struct pvr_transfer_cmd int_cmd;
   VkResult result;

   state->dont_force_pbe = false;
   state->pass_idx = pass_idx;

   pvr_transfer_set_filter(transfer_cmd, state);

   if (transfer_cmd->source_count == 1U) {
      struct pvr_transfer_cmd_source *src = &transfer_cmd->sources[0];

      /* Try to work out a condition to map pixel formats to RAW. That is only
       * possible if we don't perform any kind of 2D operation on the blit as we
       * don't know the actual pixel values - i.e. it has to be point sampled -
       * scaling doesn't matter as long as point sampled.
       */
      if (src->surface.vk_format == transfer_cmd->dst.vk_format &&
          state->filter[0] == PVR_FILTER_POINT &&
          src->surface.sample_count <= transfer_cmd->dst.sample_count &&
          (transfer_cmd->flags & PVR_TRANSFER_CMD_FLAGS_DSMERGE) == 0U) {
         uint32_t bpp;

         int_cmd = *transfer_cmd;
         active_cmd = &int_cmd;
         bpp = vk_format_get_blocksizebits(int_cmd.dst.vk_format);

         if (bpp > 0U) {
            switch (bpp) {
            case 8U:
               int_cmd.sources[0].surface.vk_format = VK_FORMAT_R8_UINT;
               break;
            case 16U:
               int_cmd.sources[0].surface.vk_format = VK_FORMAT_R8G8_UINT;
               break;
            case 24U:
               int_cmd.sources[0].surface.vk_format = VK_FORMAT_R8G8B8_UINT;
               break;
            case 32U:
               int_cmd.sources[0].surface.vk_format = VK_FORMAT_R32_UINT;
               break;
            case 48U:
               int_cmd.sources[0].surface.vk_format = VK_FORMAT_R16G16B16_UINT;
               break;
            case 64U:
               int_cmd.sources[0].surface.vk_format = VK_FORMAT_R32G32_UINT;
               break;
            case 96U:
               int_cmd.sources[0].surface.vk_format = VK_FORMAT_R32G32B32_UINT;
               break;
            case 128U:
               int_cmd.sources[0].surface.vk_format =
                  VK_FORMAT_R32G32B32A32_UINT;
               break;
            default:
               active_cmd = transfer_cmd;
               break;
            }
         }

         int_cmd.dst.vk_format = int_cmd.sources[0].surface.vk_format;
      }
   }

   if (pass_idx == 0U) {
      pvr_get_custom_mapping(dev_info, active_cmd, 3U, &state->custom_mapping);

      if (state->custom_mapping.texel_extend_src > 1U)
         state->custom_mapping.texel_extend_dst = 1U;
   }

   if (state->custom_mapping.pass_count > 0U) {
      struct pvr_transfer_pass *pass = &state->custom_mapping.passes[pass_idx];

      if (active_cmd != &int_cmd) {
         int_cmd = *active_cmd;
         active_cmd = &int_cmd;
      }

      state->custom_filter = true;

      pvr_modify_command(&state->custom_mapping, pass_idx, active_cmd);

      if (state->custom_mapping.double_stride ||
          pass->sources[0].mapping_count > 1U || pass->source_count > 1U) {
         result =
            pvr_3d_clip_blit(ctx, active_cmd, prep_data, pass_idx, finished_out);
      } else {
         struct pvr_rect_mapping *mappings = &pass->sources[0].mappings[0U];

         mappings[0U].src_rect.offset.x /=
            MAX2(1U, state->custom_mapping.texel_extend_dst);
         mappings[0U].src_rect.extent.width /=
            MAX2(1U, state->custom_mapping.texel_extend_dst);

         if (int_cmd.source_count > 0) {
            for (uint32_t i = 0U; i < pass->sources[0].mapping_count; i++)
               active_cmd->sources[0].mappings[i] = mappings[i];
         }

         active_cmd->scissor = mappings[0U].dst_rect;

         result = pvr_3d_copy_blit_core(ctx,
                                        active_cmd,
                                        prep_data,
                                        pass_idx,
                                        finished_out);
      }

      return result;
   }

   /* Route DS merge blits to Clip blit. Background object is used to preserve
    * the unmerged channel.
    */
   if ((transfer_cmd->flags & PVR_TRANSFER_CMD_FLAGS_DSMERGE) != 0U) {
      /* PBE byte mask could be used for DS merge with FastScale. Clearing the
       * other channel on a DS merge requires Clip blit.
       */
      if (!PVR_HAS_ERN(dev_info, 42064) ||
          ((transfer_cmd->flags & PVR_TRANSFER_CMD_FLAGS_FILL) != 0U)) {
         return pvr_reroute_to_clip(ctx,
                                    active_cmd,
                                    &active_cmd->scissor,
                                    prep_data,
                                    pass_idx,
                                    finished_out);
      }
   }

   return pvr_3d_copy_blit_core(ctx,
                                active_cmd,
                                prep_data,
                                pass_idx,
                                finished_out);
}

/* TODO: This should be generated in csbgen. */
#define TEXSTATE_STRIDE_IMAGE_WORD1_TEXADDR_MASK \
   BITFIELD64_RANGE(2, (53 - 16) + 1)

static bool pvr_validate_source_addr(pvr_dev_addr_t addr)
{
   if (!pvr_dev_addr_is_aligned(
          addr,
          PVRX(TEXSTATE_STRIDE_IMAGE_WORD1_TEXADDR_ALIGNMENT))) {
      return false;
   }

   if (addr.addr & ~TEXSTATE_STRIDE_IMAGE_WORD1_TEXADDR_MASK)
      return false;

   return true;
}

static bool pvr_supports_texel_unwind(struct pvr_transfer_cmd *transfer_cmd)
{
   struct pvr_transfer_cmd_surface *dst = &transfer_cmd->dst;

   if (transfer_cmd->source_count > 1)
      return false;

   if (transfer_cmd->source_count) {
      struct pvr_transfer_cmd_surface *src = &transfer_cmd->sources[0].surface;

      if (src->height == 1) {
         if (src->mem_layout != PVR_MEMLAYOUT_LINEAR &&
             src->mem_layout != PVR_MEMLAYOUT_TWIDDLED &&
             src->mem_layout != PVR_MEMLAYOUT_3DTWIDDLED) {
            return false;
         }
      } else if (src->mem_layout == PVR_MEMLAYOUT_TWIDDLED ||
                 src->mem_layout == PVR_MEMLAYOUT_3DTWIDDLED) {
         if (!pvr_validate_source_addr(src->dev_addr))
            return false;
      } else {
         if (src->mem_layout != PVR_MEMLAYOUT_LINEAR)
            return false;
      }
   }

   if (dst->mem_layout != PVR_MEMLAYOUT_LINEAR &&
       dst->mem_layout != PVR_MEMLAYOUT_TWIDDLED) {
      return false;
   }

   return true;
}

static bool pvr_3d_validate_addr(struct pvr_transfer_cmd *transfer_cmd)
{
   if (!pvr_supports_texel_unwind(transfer_cmd)) {
      return pvr_dev_addr_is_aligned(
         transfer_cmd->dst.dev_addr,
         PVRX(PBESTATE_STATE_WORD0_ADDRESS_LOW_ALIGNMENT));
   }

   return true;
}

static void
pvr_submit_info_stream_init(struct pvr_transfer_ctx *ctx,
                            struct pvr_transfer_prep_data *prep_data,
                            struct pvr_winsys_transfer_cmd *cmd)
{
   const struct pvr_winsys_transfer_regs *const regs = &prep_data->state.regs;
   const struct pvr_physical_device *const pdevice = ctx->device->pdevice;
   const struct pvr_device_info *const dev_info = &pdevice->dev_info;

   uint32_t *stream_ptr = (uint32_t *)cmd->fw_stream;
   uint32_t *stream_len_ptr = stream_ptr;

   /* Leave space for stream header. */
   stream_ptr += pvr_cmd_length(KMD_STREAM_HDR);

   *(uint64_t *)stream_ptr = regs->pds_bgnd0_base;
   stream_ptr += pvr_cmd_length(CR_PDS_BGRND0_BASE);

   *(uint64_t *)stream_ptr = regs->pds_bgnd1_base;
   stream_ptr += pvr_cmd_length(CR_PDS_BGRND1_BASE);

   *(uint64_t *)stream_ptr = regs->pds_bgnd3_sizeinfo;
   stream_ptr += pvr_cmd_length(CR_PDS_BGRND3_SIZEINFO);

   *(uint64_t *)stream_ptr = regs->isp_mtile_base;
   stream_ptr += pvr_cmd_length(CR_ISP_MTILE_BASE);

   STATIC_ASSERT(ARRAY_SIZE(regs->pbe_wordx_mrty) == 9U);
   STATIC_ASSERT(sizeof(regs->pbe_wordx_mrty[0]) == sizeof(uint64_t));
   memcpy(stream_ptr, regs->pbe_wordx_mrty, sizeof(regs->pbe_wordx_mrty));
   stream_ptr += 9U * 2U;

   *stream_ptr = regs->isp_bgobjvals;
   stream_ptr += pvr_cmd_length(CR_ISP_BGOBJVALS);

   *stream_ptr = regs->usc_pixel_output_ctrl;
   stream_ptr += pvr_cmd_length(CR_USC_PIXEL_OUTPUT_CTRL);

   *stream_ptr = regs->usc_clear_register0;
   stream_ptr += pvr_cmd_length(CR_USC_CLEAR_REGISTER);

   *stream_ptr = regs->usc_clear_register1;
   stream_ptr += pvr_cmd_length(CR_USC_CLEAR_REGISTER);

   *stream_ptr = regs->usc_clear_register2;
   stream_ptr += pvr_cmd_length(CR_USC_CLEAR_REGISTER);

   *stream_ptr = regs->usc_clear_register3;
   stream_ptr += pvr_cmd_length(CR_USC_CLEAR_REGISTER);

   *stream_ptr = regs->isp_mtile_size;
   stream_ptr += pvr_cmd_length(CR_ISP_MTILE_SIZE);

   *stream_ptr = regs->isp_render_origin;
   stream_ptr += pvr_cmd_length(CR_ISP_RENDER_ORIGIN);

   *stream_ptr = regs->isp_ctl;
   stream_ptr += pvr_cmd_length(CR_ISP_CTL);

   *stream_ptr = regs->isp_aa;
   stream_ptr += pvr_cmd_length(CR_ISP_AA);

   *stream_ptr = regs->event_pixel_pds_info;
   stream_ptr += pvr_cmd_length(CR_EVENT_PIXEL_PDS_INFO);

   *stream_ptr = regs->event_pixel_pds_code;
   stream_ptr += pvr_cmd_length(CR_EVENT_PIXEL_PDS_CODE);

   *stream_ptr = regs->event_pixel_pds_data;
   stream_ptr += pvr_cmd_length(CR_EVENT_PIXEL_PDS_DATA);

   *stream_ptr = regs->isp_render;
   stream_ptr += pvr_cmd_length(CR_ISP_RENDER);

   *stream_ptr = regs->isp_rgn;
   stream_ptr++;

   if (PVR_HAS_FEATURE(dev_info, gpu_multicore_support)) {
      *stream_ptr = regs->frag_screen;
      stream_ptr++;
   }

   cmd->fw_stream_len = (uint8_t *)stream_ptr - (uint8_t *)cmd->fw_stream;
   assert(cmd->fw_stream_len <= ARRAY_SIZE(cmd->fw_stream));

   pvr_csb_pack ((uint64_t *)stream_len_ptr, KMD_STREAM_HDR, value) {
      value.length = cmd->fw_stream_len;
   }
}

static void
pvr_submit_info_flags_init(const struct pvr_device_info *const dev_info,
                           const struct pvr_transfer_prep_data *const prep_data,
                           struct pvr_winsys_transfer_cmd_flags *flags)
{
   *flags = prep_data->flags;
   flags->use_single_core = PVR_HAS_FEATURE(dev_info, gpu_multicore_support);
}

static void pvr_transfer_job_ws_submit_info_init(
   struct pvr_transfer_ctx *ctx,
   struct pvr_transfer_submit *submit,
   struct vk_sync *wait,
   struct pvr_winsys_transfer_submit_info *submit_info)
{
   const struct pvr_device *const device = ctx->device;
   const struct pvr_device_info *const dev_info = &device->pdevice->dev_info;

   submit_info->frame_num = device->global_queue_present_count;
   submit_info->job_num = device->global_cmd_buffer_submit_count;
   submit_info->wait = wait;
   submit_info->cmd_count = submit->prep_count;

   for (uint32_t i = 0U; i < submit->prep_count; i++) {
      struct pvr_winsys_transfer_cmd *const cmd = &submit_info->cmds[i];
      struct pvr_transfer_prep_data *prep_data = &submit->prep_array[i];

      pvr_submit_info_stream_init(ctx, prep_data, cmd);
      pvr_submit_info_flags_init(dev_info, prep_data, &cmd->flags);
   }
}

static VkResult pvr_submit_transfer(struct pvr_transfer_ctx *ctx,
                                    struct pvr_transfer_submit *submit,
                                    struct vk_sync *wait,
                                    struct vk_sync *signal_sync)
{
   struct pvr_winsys_transfer_submit_info submit_info;

   pvr_transfer_job_ws_submit_info_init(ctx, submit, wait, &submit_info);

   return ctx->device->ws->ops->transfer_submit(ctx->ws_ctx,
                                                &submit_info,
                                                &ctx->device->pdevice->dev_info,
                                                signal_sync);
}

static VkResult pvr_queue_transfer(struct pvr_transfer_ctx *ctx,
                                   struct pvr_transfer_cmd *transfer_cmd,
                                   struct vk_sync *wait,
                                   struct vk_sync *signal_sync)
{
   struct pvr_transfer_prep_data *prep_data = NULL;
   struct pvr_transfer_prep_data *prev_prep_data;
   struct pvr_transfer_submit submit = { 0U };
   bool finished = false;
   uint32_t pass = 0U;
   VkResult result;

   /* Transfer queue might decide to do a blit in multiple passes. When the
    * prepare doesn't set the finished flag this code will keep calling the
    * prepare with increasing pass. If queued transfers are submitted from
    * here we submit them straight away. That's why we only need a single
    * prepare for the blit rather then one for each pass. Otherwise we insert
    * each prepare into the prepare array. When the client does blit batching
    * and we split the blit into multiple passes each pass in each queued
    * transfer adds one more prepare. Thus the prepare array after 2
    * pvr_queue_transfer calls might look like:
    *
    * +------+------++-------+-------+-------+
    * |B0/P0 |B0/P1 || B1/P0 | B1/P1 | B1/P2 |
    * +------+------++-------+-------+-------+
    * F           S/U F                    S/U
    *
    * Bn/Pm : nth blit (queue transfer call) / mth prepare
    * F     : fence point
    * S/U   : update / server sync update point
    */

   while (!finished) {
      prev_prep_data = prep_data;
      prep_data = &submit.prep_array[submit.prep_count++];

      /* Clear down the memory before we write to this prep. */
      memset(prep_data, 0U, sizeof(*prep_data));

      if (pass == 0U) {
         if (!pvr_3d_validate_addr(transfer_cmd))
            return vk_error(ctx->device, VK_ERROR_FEATURE_NOT_PRESENT);
      } else {
         /* Transfer queue workarounds could use more than one pass with 3D
          * path.
          */
         prep_data->state = prev_prep_data->state;
      }

      if (transfer_cmd->flags & PVR_TRANSFER_CMD_FLAGS_FAST2D) {
         result =
            pvr_3d_clip_blit(ctx, transfer_cmd, prep_data, pass, &finished);
      } else {
         result =
            pvr_3d_copy_blit(ctx, transfer_cmd, prep_data, pass, &finished);
      }
      if (result != VK_SUCCESS)
         return result;

      /* Submit if we have finished the blit or if we are out of prepares. */
      if (finished || submit.prep_count == ARRAY_SIZE(submit.prep_array)) {
         result = pvr_submit_transfer(ctx,
                                      &submit,
                                      wait,
                                      finished ? signal_sync : NULL);
         if (result != VK_SUCCESS)
            return result;

         /* Check if we need to reset prep_count. */
         if (submit.prep_count == ARRAY_SIZE(submit.prep_array))
            submit.prep_count = 0U;
      }

      pass++;
   }

   return VK_SUCCESS;
}

VkResult pvr_transfer_job_submit(struct pvr_transfer_ctx *ctx,
                                 struct pvr_sub_cmd_transfer *sub_cmd,
                                 struct vk_sync *wait_sync,
                                 struct vk_sync *signal_sync)
{
   list_for_each_entry_safe (struct pvr_transfer_cmd,
                             transfer_cmd,
                             sub_cmd->transfer_cmds,
                             link) {
      /* The fw guarantees that any kick on the same context will be
       * synchronized in submission order. This means only the first kick must
       * wait, and only the last kick need signal.
       */
      struct vk_sync *first_cmd_wait_sync = NULL;
      struct vk_sync *last_cmd_signal_sync = NULL;
      VkResult result;

      if (list_first_entry(sub_cmd->transfer_cmds,
                           struct pvr_transfer_cmd,
                           link) == transfer_cmd) {
         first_cmd_wait_sync = wait_sync;
      }

      if (list_last_entry(sub_cmd->transfer_cmds,
                          struct pvr_transfer_cmd,
                          link) == transfer_cmd) {
         last_cmd_signal_sync = signal_sync;
      }

      result = pvr_queue_transfer(ctx,
                                  transfer_cmd,
                                  first_cmd_wait_sync,
                                  last_cmd_signal_sync);
      if (result != VK_SUCCESS)
         return result;
   }

   return VK_SUCCESS;
}
