/*
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_pipe.h"
#include "sid.h"
#include "util/format/u_format.h"
#include "util/u_pack_color.h"
#include "util/u_surface.h"

enum {
   SI_CLEAR = SI_SAVE_FRAGMENT_STATE | SI_SAVE_FRAGMENT_CONSTANT,
   SI_CLEAR_SURFACE = SI_SAVE_FRAMEBUFFER | SI_SAVE_FRAGMENT_STATE,
};

void si_init_buffer_clear(struct si_clear_info *info,
                          struct pipe_resource *resource, uint64_t offset,
                          uint32_t size, uint32_t clear_value)
{
   info->resource = resource;
   info->offset = offset;
   info->size = size;
   info->clear_value = clear_value;
   info->writemask = 0xffffffff;
   info->is_dcc_msaa = false;
}

static void si_init_buffer_clear_rmw(struct si_clear_info *info,
                                     struct pipe_resource *resource, uint64_t offset,
                                     uint32_t size, uint32_t clear_value, uint32_t writemask)
{
   si_init_buffer_clear(info, resource, offset, size, clear_value);
   info->writemask = writemask;
}

void si_execute_clears(struct si_context *sctx, struct si_clear_info *info,
                       unsigned num_clears, unsigned types)
{
   if (!num_clears)
      return;

   /* Flush caches and wait for idle. */
   if (types & (SI_CLEAR_TYPE_CMASK | SI_CLEAR_TYPE_DCC))
      sctx->flags |= si_get_flush_flags(sctx, SI_COHERENCY_CB_META, L2_LRU);

   if (types & SI_CLEAR_TYPE_HTILE)
      sctx->flags |= si_get_flush_flags(sctx, SI_COHERENCY_DB_META, L2_LRU);

   /* Flush caches in case we use compute. */
   sctx->flags |= SI_CONTEXT_INV_VCACHE;

   /* GFX6-8: CB and DB don't use L2. */
   if (sctx->gfx_level <= GFX8)
      sctx->flags |= SI_CONTEXT_INV_L2;

   si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);

   /* Execute clears. */
   for (unsigned i = 0; i < num_clears; i++) {
      if (info[i].is_dcc_msaa) {
         gfx9_clear_dcc_msaa(sctx, info[i].resource, info[i].clear_value,
                             SI_OP_SKIP_CACHE_INV_BEFORE, SI_COHERENCY_CP);
         continue;
      }

      assert(info[i].size > 0);

      if (info[i].writemask != 0xffffffff) {
         si_compute_clear_buffer_rmw(sctx, info[i].resource, info[i].offset, info[i].size,
                                     info[i].clear_value, info[i].writemask,
                                     SI_OP_SKIP_CACHE_INV_BEFORE, SI_COHERENCY_CP);
      } else {
         /* Compute shaders are much faster on both dGPUs and APUs. Don't use CP DMA. */
         si_clear_buffer(sctx, info[i].resource, info[i].offset, info[i].size,
                         &info[i].clear_value, 4, SI_OP_SKIP_CACHE_INV_BEFORE,
                         SI_COHERENCY_CP, SI_COMPUTE_CLEAR_METHOD);
      }
   }

   /* Wait for idle. */
   sctx->flags |= SI_CONTEXT_CS_PARTIAL_FLUSH;

   /* GFX6-8: CB and DB don't use L2. */
   if (sctx->gfx_level <= GFX8)
      sctx->flags |= SI_CONTEXT_WB_L2;

   si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
}

static bool si_alloc_separate_cmask(struct si_screen *sscreen, struct si_texture *tex)
{
   assert(sscreen->info.gfx_level < GFX11);

   /* CMASK for MSAA is allocated in advance or always disabled
    * by "nofmask" option.
    */
   if (tex->cmask_buffer)
      return true;

   if (!tex->surface.cmask_size)
      return false;

   tex->cmask_buffer =
      si_aligned_buffer_create(&sscreen->b, PIPE_RESOURCE_FLAG_UNMAPPABLE, PIPE_USAGE_DEFAULT,
                               tex->surface.cmask_size, 1 << tex->surface.cmask_alignment_log2);
   if (tex->cmask_buffer == NULL)
      return false;

   /* These 2 fields are part of the framebuffer state but dirtying the atom
    * will be done by the caller.
    */
   tex->cmask_base_address_reg = tex->cmask_buffer->gpu_address >> 8;
   tex->cb_color_info |= S_028C70_FAST_CLEAR(1);

   p_atomic_inc(&sscreen->compressed_colortex_counter);
   return true;
}

static bool si_set_clear_color(struct si_texture *tex, enum pipe_format surface_format,
                               const union pipe_color_union *color)
{
   union util_color uc;

   memset(&uc, 0, sizeof(uc));

   if (tex->surface.bpe == 16) {
      /* DCC fast clear only:
       *   CLEAR_WORD0 = R = G = B
       *   CLEAR_WORD1 = A
       */
      assert(color->ui[0] == color->ui[1] && color->ui[0] == color->ui[2]);
      uc.ui[0] = color->ui[0];
      uc.ui[1] = color->ui[3];
   } else {
      if (tex->swap_rgb_to_bgr)
         surface_format = util_format_rgb_to_bgr(surface_format);

      util_pack_color_union(surface_format, &uc, color);
   }

   if (memcmp(tex->color_clear_value, &uc, 2 * sizeof(uint32_t)) == 0)
      return false;

   memcpy(tex->color_clear_value, &uc, 2 * sizeof(uint32_t));
   return true;
}

/** Linearize and convert luminance/intensity to red. */
enum pipe_format si_simplify_cb_format(enum pipe_format format)
{
   format = util_format_linear(format);
   format = util_format_luminance_to_red(format);
   return util_format_intensity_to_red(format);
}

bool vi_alpha_is_on_msb(struct si_screen *sscreen, enum pipe_format format)
{
   if (sscreen->info.gfx_level >= GFX11)
      return false;

   format = si_simplify_cb_format(format);
   const struct util_format_description *desc = util_format_description(format);
   unsigned comp_swap = si_translate_colorswap(sscreen->info.gfx_level, format, false);

   /* The following code matches the hw behavior. */
   if (desc->nr_channels == 1) {
      return (comp_swap == V_028C70_SWAP_ALT_REV) != (sscreen->info.family == CHIP_RAVEN2 ||
                                                      sscreen->info.family == CHIP_RENOIR);
   }

   return comp_swap != V_028C70_SWAP_STD_REV && comp_swap != V_028C70_SWAP_ALT_REV;
}

static bool gfx8_get_dcc_clear_parameters(struct si_screen *sscreen, enum pipe_format base_format,
                                          enum pipe_format surface_format,
                                          const union pipe_color_union *color, uint32_t *clear_value,
                                          bool *eliminate_needed)
{
   /* If we want to clear without needing a fast clear eliminate step, we
    * can set color and alpha independently to 0 or 1 (or 0/max for integer
    * formats).
    */
   bool values[4] = {};      /* whether to clear to 0 or 1 */
   bool color_value = false; /* clear color to 0 or 1 */
   bool alpha_value = false; /* clear alpha to 0 or 1 */
   int alpha_channel;        /* index of the alpha component */
   bool has_color = false;
   bool has_alpha = false;

   const struct util_format_description *desc =
      util_format_description(si_simplify_cb_format(surface_format));

   /* 128-bit fast clear with different R,G,B values is unsupported. */
   if (desc->block.bits == 128 && (color->ui[0] != color->ui[1] || color->ui[0] != color->ui[2]))
      return false;

   *eliminate_needed = true;
   *clear_value = GFX8_DCC_CLEAR_REG;

   if (desc->layout != UTIL_FORMAT_LAYOUT_PLAIN)
      return true; /* need ELIMINATE_FAST_CLEAR */

   bool base_alpha_is_on_msb = vi_alpha_is_on_msb(sscreen, base_format);
   bool surf_alpha_is_on_msb = vi_alpha_is_on_msb(sscreen, surface_format);

   /* Formats with 3 channels can't have alpha. */
   if (desc->nr_channels == 3)
      alpha_channel = -1;
   else if (surf_alpha_is_on_msb)
      alpha_channel = desc->nr_channels - 1;
   else
      alpha_channel = 0;

   for (int i = 0; i < 4; ++i) {
      if (desc->swizzle[i] >= PIPE_SWIZZLE_0)
         continue;

      if (desc->channel[i].pure_integer && desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED) {
         /* Use the maximum value for clamping the clear color. */
         int max = u_bit_consecutive(0, desc->channel[i].size - 1);

         values[i] = color->i[i] != 0;
         if (color->i[i] != 0 && MIN2(color->i[i], max) != max)
            return true; /* need ELIMINATE_FAST_CLEAR */
      } else if (desc->channel[i].pure_integer &&
                 desc->channel[i].type == UTIL_FORMAT_TYPE_UNSIGNED) {
         /* Use the maximum value for clamping the clear color. */
         unsigned max = u_bit_consecutive(0, desc->channel[i].size);

         values[i] = color->ui[i] != 0U;
         if (color->ui[i] != 0U && MIN2(color->ui[i], max) != max)
            return true; /* need ELIMINATE_FAST_CLEAR */
      } else {
         values[i] = color->f[i] != 0.0F;
         if (color->f[i] != 0.0F && color->f[i] != 1.0F)
            return true; /* need ELIMINATE_FAST_CLEAR */
      }

      if (desc->swizzle[i] == alpha_channel) {
         alpha_value = values[i];
         has_alpha = true;
      } else {
         color_value = values[i];
         has_color = true;
      }
   }

   /* If alpha isn't present, make it the same as color, and vice versa. */
   if (!has_alpha)
      alpha_value = color_value;
   else if (!has_color)
      color_value = alpha_value;

   if (color_value != alpha_value && base_alpha_is_on_msb != surf_alpha_is_on_msb)
      return true; /* require ELIMINATE_FAST_CLEAR */

   /* Check if all color values are equal if they are present. */
   for (int i = 0; i < 4; ++i) {
      if (desc->swizzle[i] <= PIPE_SWIZZLE_W && desc->swizzle[i] != alpha_channel &&
          values[i] != color_value)
         return true; /* require ELIMINATE_FAST_CLEAR */
   }

   /* This doesn't need ELIMINATE_FAST_CLEAR.
    * On chips predating Raven2, the DCC clear codes and the CB clear
    * color registers must match.
    */
   *eliminate_needed = false;

   if (color_value) {
      if (alpha_value)
         *clear_value = GFX8_DCC_CLEAR_1111;
      else
         *clear_value = GFX8_DCC_CLEAR_1110;
   } else {
      if (alpha_value)
         *clear_value = GFX8_DCC_CLEAR_0001;
      else
         *clear_value = GFX8_DCC_CLEAR_0000;
   }
   return true;
}

static bool gfx11_get_dcc_clear_parameters(struct si_screen *sscreen, enum pipe_format surface_format,
                                           const union pipe_color_union *color, uint32_t *clear_value)
{
   const struct util_format_description *desc =
      util_format_description(si_simplify_cb_format(surface_format));
   unsigned start_bit = UINT_MAX;
   unsigned end_bit = 0;

   /* TODO: 8bpp and 16bpp fast DCC clears don't work. */
   if (desc->block.bits <= 16)
      return false;

   /* Find the used bit range. */
   for (unsigned i = 0; i < 4; i++) {
      unsigned swizzle = desc->swizzle[i];

      if (swizzle >= PIPE_SWIZZLE_0)
         continue;

      start_bit = MIN2(start_bit, desc->channel[swizzle].shift);
      end_bit = MAX2(end_bit, desc->channel[swizzle].shift + desc->channel[swizzle].size);
   }

   union {
      uint8_t ub[16];
      uint16_t us[8];
      uint32_t ui[4];
   } value = {};
   util_pack_color_union(surface_format, (union util_color*)&value, color);

   /* Check the cases where all components or bits are either all 0 or all 1. */
   bool all_bits_are_0 = true;
   bool all_bits_are_1 = true;
   bool all_words_are_fp16_1 = false;
   bool all_words_are_fp32_1 = false;

   for (unsigned i = start_bit; i < end_bit; i++) {
      bool bit = value.ub[i / 8] & BITFIELD_BIT(i % 8);

      all_bits_are_0 &= !bit;
      all_bits_are_1 &= bit;
   }

   if (start_bit % 16 == 0 && end_bit % 16 == 0) {
      all_words_are_fp16_1 = true;
      for (unsigned i = start_bit / 16; i < end_bit / 16; i++)
         all_words_are_fp16_1 &= value.us[i] == 0x3c00;
   }

   if (start_bit % 32 == 0 && end_bit % 32 == 0) {
      all_words_are_fp32_1 = true;
      for (unsigned i = start_bit / 32; i < end_bit / 32; i++)
         all_words_are_fp32_1 &= value.ui[i] == 0x3f800000;
   }

#if 0 /* debug code */
   int i = util_format_get_first_non_void_channel(surface_format);
   if (desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED && desc->channel[i].pure_integer) {
      printf("%i %i %i %i\n", color->i[0], color->i[1], color->i[2], color->i[3]);
   } else if (desc->channel[i].type == UTIL_FORMAT_TYPE_UNSIGNED && desc->channel[i].pure_integer) {
      printf("%u %u %u %u\n", color->ui[0], color->ui[1], color->ui[2], color->ui[3]);
   } else {
      printf("%f %f %f %f\n", color->f[0], color->f[1], color->f[2], color->f[3]);
   }
   for (unsigned i = 0; i < end_bit / 8; i++)
      printf("%02x", value.ub[i]);
   printf("\n");
   printf("bits=[%u..%u)%s%s%s%s\n", start_bit, end_bit,
          all_bits_are_0 ? ", all 0" : "",
          all_bits_are_1 ? ", all 1" : "",
          all_words_are_fp16_1 ? ", all fp16 1" : "",
          all_words_are_fp32_1 ? ", all fp32 1" : "");
#endif

   *clear_value = 0;

   if (all_bits_are_0 || all_bits_are_1 || all_words_are_fp16_1 || all_words_are_fp32_1) {
      if (all_bits_are_0)
         *clear_value = GFX11_DCC_CLEAR_0000;
      else if (all_bits_are_1)
         *clear_value = GFX11_DCC_CLEAR_1111_UNORM;
      else if (all_words_are_fp16_1)
         *clear_value = GFX11_DCC_CLEAR_1111_FP16;
      else if (all_words_are_fp32_1)
         *clear_value = GFX11_DCC_CLEAR_1111_FP32;

      return true;
   }

   /* Check 0001 and 1110 cases. */
   if (desc->nr_channels == 2 && desc->channel[0].size == 8) {
      if (value.ub[0] == 0x00 && value.ub[1] == 0xff) {
         *clear_value = GFX11_DCC_CLEAR_0001_UNORM;
         return true;
      } else if (value.ub[0] == 0xff && value.ub[1] == 0x00) {
         *clear_value = GFX11_DCC_CLEAR_1110_UNORM;
         return true;
      }
   } else if (desc->nr_channels == 4 && desc->channel[0].size == 8) {
      if (value.ub[0] == 0x00 && value.ub[1] == 0x00 &&
          value.ub[2] == 0x00 && value.ub[3] == 0xff) {
         *clear_value = GFX11_DCC_CLEAR_0001_UNORM;
         return true;
      } else if (value.ub[0] == 0xff && value.ub[1] == 0xff &&
                 value.ub[2] == 0xff && value.ub[3] == 0x00) {
         *clear_value = GFX11_DCC_CLEAR_1110_UNORM;
         return true;
      }
   } else if (desc->nr_channels == 4 && desc->channel[0].size == 16) {
      if (value.us[0] == 0x0000 && value.us[1] == 0x0000 &&
          value.us[2] == 0x0000 && value.us[3] == 0xffff) {
         *clear_value = GFX11_DCC_CLEAR_0001_UNORM;
         return true;
      } else if (value.us[0] == 0xffff && value.us[1] == 0xffff &&
                 value.us[2] == 0xffff && value.us[3] == 0x0000) {
         *clear_value = GFX11_DCC_CLEAR_1110_UNORM;
         return true;
      }
   }

   return false;
}

bool vi_dcc_get_clear_info(struct si_context *sctx, struct si_texture *tex, unsigned level,
                           unsigned clear_value, struct si_clear_info *out)
{
   struct pipe_resource *dcc_buffer = &tex->buffer.b.b;
   uint64_t dcc_offset = tex->surface.meta_offset;
   uint32_t clear_size;

   assert(vi_dcc_enabled(tex, level));

   if (sctx->gfx_level >= GFX10) {
      /* 4x and 8x MSAA needs a sophisticated compute shader for
       * the clear. GFX11 doesn't need that.
       */
      if (sctx->gfx_level < GFX11 && tex->buffer.b.b.nr_storage_samples >= 4)
         return false;

      unsigned num_layers = util_num_layers(&tex->buffer.b.b, level);

      if (num_layers == 1) {
         /* Clear a specific level. */
         dcc_offset += tex->surface.u.gfx9.meta_levels[level].offset;
         clear_size = tex->surface.u.gfx9.meta_levels[level].size;
      } else if (tex->buffer.b.b.last_level == 0) {
         /* Clear all layers having only 1 level. */
         clear_size = tex->surface.meta_size;
      } else {
         /* Clearing DCC with both multiple levels and multiple layers is not
          * implemented.
          */
         return false;
      }
   } else if (sctx->gfx_level == GFX9) {
      /* TODO: Implement DCC fast clear for level 0 of mipmapped textures. Mipmapped
       * DCC has to clear a rectangular area of DCC for level 0 (because the whole miptree
       * is organized in a 2D plane).
       */
      if (tex->buffer.b.b.last_level > 0)
         return false;

      /* 4x and 8x MSAA need to clear only sample 0 and 1 in a compute shader and leave other
       * samples untouched. (only the first 2 samples are compressed) */
      if (tex->buffer.b.b.nr_storage_samples >= 4) {
         si_init_buffer_clear(out, dcc_buffer, 0, 0, clear_value);
         out->is_dcc_msaa = true;
         return true;
      }

      clear_size = tex->surface.meta_size;
   } else {
      unsigned num_layers = util_num_layers(&tex->buffer.b.b, level);

      /* If this is 0, fast clear isn't possible. (can occur with MSAA) */
      if (!tex->surface.u.legacy.color.dcc_level[level].dcc_fast_clear_size)
         return false;

      /* Layered 4x and 8x MSAA DCC fast clears need to clear
       * dcc_fast_clear_size bytes for each layer. A compute shader
       * would be more efficient than separate per-layer clear operations.
       */
      if (tex->buffer.b.b.nr_storage_samples >= 4 && num_layers > 1)
         return false;

      dcc_offset += tex->surface.u.legacy.color.dcc_level[level].dcc_offset;
      clear_size = tex->surface.u.legacy.color.dcc_level[level].dcc_fast_clear_size;
   }

   si_init_buffer_clear(out, dcc_buffer, dcc_offset, clear_size, clear_value);
   return true;
}

/* Set the same micro tile mode as the destination of the last MSAA resolve.
 * This allows hitting the MSAA resolve fast path, which requires that both
 * src and dst micro tile modes match.
 */
static void si_set_optimal_micro_tile_mode(struct si_screen *sscreen, struct si_texture *tex)
{
   if (sscreen->info.gfx_level >= GFX10 || tex->buffer.b.is_shared ||
       tex->buffer.b.b.nr_samples <= 1 ||
       tex->surface.micro_tile_mode == tex->last_msaa_resolve_target_micro_mode)
      return;

   assert(sscreen->info.gfx_level >= GFX9 ||
          tex->surface.u.legacy.level[0].mode == RADEON_SURF_MODE_2D);
   assert(tex->buffer.b.b.last_level == 0);

   if (sscreen->info.gfx_level >= GFX9) {
      /* 4K or larger tiles only. 0 is linear. 1-3 are 256B tiles. */
      assert(tex->surface.u.gfx9.swizzle_mode >= 4);

      /* If you do swizzle_mode % 4, you'll get:
       *   0 = Depth
       *   1 = Standard,
       *   2 = Displayable
       *   3 = Rotated
       *
       * Depth-sample order isn't allowed:
       */
      assert(tex->surface.u.gfx9.swizzle_mode % 4 != 0);

      switch (tex->last_msaa_resolve_target_micro_mode) {
      case RADEON_MICRO_MODE_DISPLAY:
         tex->surface.u.gfx9.swizzle_mode &= ~0x3;
         tex->surface.u.gfx9.swizzle_mode += 2; /* D */
         break;
      case RADEON_MICRO_MODE_STANDARD:
         tex->surface.u.gfx9.swizzle_mode &= ~0x3;
         tex->surface.u.gfx9.swizzle_mode += 1; /* S */
         break;
      case RADEON_MICRO_MODE_RENDER:
         tex->surface.u.gfx9.swizzle_mode &= ~0x3;
         tex->surface.u.gfx9.swizzle_mode += 3; /* R */
         break;
      default: /* depth */
         assert(!"unexpected micro mode");
         return;
      }
   } else if (sscreen->info.gfx_level >= GFX7) {
      /* These magic numbers were copied from addrlib. It doesn't use
       * any definitions for them either. They are all 2D_TILED_THIN1
       * modes with different bpp and micro tile mode.
       */
      switch (tex->last_msaa_resolve_target_micro_mode) {
      case RADEON_MICRO_MODE_DISPLAY:
         tex->surface.u.legacy.tiling_index[0] = 10;
         break;
      case RADEON_MICRO_MODE_STANDARD:
         tex->surface.u.legacy.tiling_index[0] = 14;
         break;
      case RADEON_MICRO_MODE_RENDER:
         tex->surface.u.legacy.tiling_index[0] = 28;
         break;
      default: /* depth, thick */
         assert(!"unexpected micro mode");
         return;
      }
   } else { /* GFX6 */
      switch (tex->last_msaa_resolve_target_micro_mode) {
      case RADEON_MICRO_MODE_DISPLAY:
         switch (tex->surface.bpe) {
         case 1:
            tex->surface.u.legacy.tiling_index[0] = 10;
            break;
         case 2:
            tex->surface.u.legacy.tiling_index[0] = 11;
            break;
         default: /* 4, 8 */
            tex->surface.u.legacy.tiling_index[0] = 12;
            break;
         }
         break;
      case RADEON_MICRO_MODE_STANDARD:
         switch (tex->surface.bpe) {
         case 1:
            tex->surface.u.legacy.tiling_index[0] = 14;
            break;
         case 2:
            tex->surface.u.legacy.tiling_index[0] = 15;
            break;
         case 4:
            tex->surface.u.legacy.tiling_index[0] = 16;
            break;
         default: /* 8, 16 */
            tex->surface.u.legacy.tiling_index[0] = 17;
            break;
         }
         break;
      default: /* depth, thick */
         assert(!"unexpected micro mode");
         return;
      }
   }

   tex->surface.micro_tile_mode = tex->last_msaa_resolve_target_micro_mode;

   p_atomic_inc(&sscreen->dirty_tex_counter);
}

static uint32_t si_get_htile_clear_value(struct si_texture *tex, float depth)
{
   /* Maximum 14-bit UINT value. */
   const uint32_t max_z_value = 0x3FFF;

   /* For clears, Zmask and Smem will always be set to zero. */
   const uint32_t zmask = 0;
   const uint32_t smem  = 0;

   /* Convert depthValue to 14-bit zmin/zmax uint values. */
   const uint32_t zmin = lroundf(depth * max_z_value);
   const uint32_t zmax = zmin;

   if (tex->htile_stencil_disabled) {
      /* Z-only HTILE is laid out as follows:
       * |31     18|17      4|3     0|
       * +---------+---------+-------+
       * |  Max Z  |  Min Z  | ZMask |
       */
      return ((zmax & 0x3FFF) << 18) |
             ((zmin & 0x3FFF) << 4) |
             ((zmask & 0xF) << 0);
   } else {
      /* Z+S HTILE is laid out as-follows:
       * |31       12|11 10|9    8|7   6|5   4|3     0|
       * +-----------+-----+------+-----+-----+-------+
       * |  Z Range  |     | SMem | SR1 | SR0 | ZMask |
       *
       * The base value for zRange is either zMax or zMin, depending on ZRANGE_PRECISION.
       * For a fast clear, zMin == zMax == clearValue. This means that the base will
       * always be the clear value (converted to 14-bit UINT).
       *
       * When abs(zMax-zMin) < 16, the delta is equal to the difference. In the case of
       * fast clears, where zMax == zMin, the delta is always zero.
       */
      const uint32_t delta = 0;
      const uint32_t zrange = (zmax << 6) | delta;

      /* SResults 0 & 1 are set based on the stencil compare state.
       * For fast-clear, the default value of sr0 and sr1 are both 0x3.
       */
      const uint32_t sresults = 0xf;

      return ((zrange & 0xFFFFF) << 12) |
             ((smem & 0x3) <<  8) |
             ((sresults & 0xF) <<  4) |
             ((zmask & 0xF) <<  0);
   }
}

static bool si_can_fast_clear_depth(struct si_texture *zstex, unsigned level, float depth,
                                    unsigned buffers)
{
   /* TC-compatible HTILE only supports depth clears to 0 or 1. */
   return buffers & PIPE_CLEAR_DEPTH &&
          si_htile_enabled(zstex, level, PIPE_MASK_Z) &&
          (!zstex->tc_compatible_htile || depth == 0 || depth == 1);
}

static bool si_can_fast_clear_stencil(struct si_texture *zstex, unsigned level, uint8_t stencil,
                                      unsigned buffers)
{
   /* TC-compatible HTILE only supports stencil clears to 0. */
   return buffers & PIPE_CLEAR_STENCIL &&
          si_htile_enabled(zstex, level, PIPE_MASK_S) &&
          (!zstex->tc_compatible_htile || stencil == 0);
}

static void si_fast_clear(struct si_context *sctx, unsigned *buffers,
                          const union pipe_color_union *color, float depth, uint8_t stencil)
{
   struct pipe_framebuffer_state *fb = &sctx->framebuffer.state;
   struct si_clear_info info[8 * 2 + 1]; /* MRTs * (CMASK + DCC) + ZS */
   unsigned num_clears = 0;
   unsigned clear_types = 0;
   unsigned num_pixels = fb->width * fb->height;

   /* This function is broken in BE, so just disable this path for now */
#if UTIL_ARCH_BIG_ENDIAN
   return;
#endif

   if (sctx->render_cond)
      return;

   /* Gather information about what to clear. */
   unsigned color_buffer_mask = (*buffers & PIPE_CLEAR_COLOR) >> util_logbase2(PIPE_CLEAR_COLOR0);
   while (color_buffer_mask) {
      unsigned i = u_bit_scan(&color_buffer_mask);

      struct si_texture *tex = (struct si_texture *)fb->cbufs[i]->texture;
      unsigned level = fb->cbufs[i]->u.tex.level;
      unsigned num_layers = util_num_layers(&tex->buffer.b.b, level);

      /* the clear is allowed if all layers are bound */
      if (fb->cbufs[i]->u.tex.first_layer != 0 ||
          fb->cbufs[i]->u.tex.last_layer != num_layers - 1) {
         continue;
      }

      /* We can change the micro tile mode before a full clear. */
      /* This is only used for MSAA textures when clearing all layers. */
      si_set_optimal_micro_tile_mode(sctx->screen, tex);

      if (tex->swap_rgb_to_bgr_on_next_clear) {
         assert(!tex->swap_rgb_to_bgr);
         assert(tex->buffer.b.b.nr_samples >= 2);
         tex->swap_rgb_to_bgr = true;
         tex->swap_rgb_to_bgr_on_next_clear = false;

         /* Update all sampler views and images. */
         p_atomic_inc(&sctx->screen->dirty_tex_counter);
      }

      /* only supported on tiled surfaces */
      if (tex->surface.is_linear) {
         continue;
      }

      /* Use a slow clear for small surfaces where the cost of
       * the eliminate pass can be higher than the benefit of fast
       * clear. The closed driver does this, but the numbers may differ.
       *
       * This helps on both dGPUs and APUs, even small APUs like Mullins.
       */
      bool fb_too_small = (uint64_t)num_pixels * num_layers <= 512 * 512;
      bool too_small = tex->buffer.b.b.nr_samples <= 1 && fb_too_small;
      bool eliminate_needed = false;
      bool fmask_decompress_needed = false;
      bool need_dirtying_fb = false;

      /* Try to clear DCC first, otherwise try CMASK. */
      if (vi_dcc_enabled(tex, level)) {
         uint32_t reset_value;

         if (sctx->screen->debug_flags & DBG(NO_DCC_CLEAR))
            continue;

         if (sctx->gfx_level >= GFX11) {
            if (!gfx11_get_dcc_clear_parameters(sctx->screen, fb->cbufs[i]->format, color,
                                                &reset_value))
               continue;
         } else {
            if (!gfx8_get_dcc_clear_parameters(sctx->screen, tex->buffer.b.b.format,
                                               fb->cbufs[i]->format, color, &reset_value,
                                               &eliminate_needed))
               continue;
         }

         /* Shared textures can't use fast clear without an explicit flush
          * because the clear color is not exported.
          *
          * Chips without DCC constant encoding must set the clear color registers
          * correctly even if the fast clear eliminate pass is not needed.
          */
         if ((eliminate_needed || !sctx->screen->info.has_dcc_constant_encode) &&
             tex->buffer.b.is_shared &&
             !(tex->buffer.external_usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH))
            continue;

         if (eliminate_needed && too_small)
            continue;

         /* We can clear any level, but we only set up the clear value registers for the first
          * level. Therefore, all other levels can be cleared only if the clear value registers
          * are not used, which is only the case with DCC constant encoding and 0/1 clear values.
          */
         if (level > 0 && (eliminate_needed || !sctx->screen->info.has_dcc_constant_encode))
            continue;

         if (tex->buffer.b.b.nr_samples >= 2 && eliminate_needed &&
             !sctx->screen->allow_dcc_msaa_clear_to_reg_for_bpp[util_logbase2(tex->surface.bpe)])
            continue;

         assert(num_clears < ARRAY_SIZE(info));

         if (!vi_dcc_get_clear_info(sctx, tex, level, reset_value, &info[num_clears]))
            continue;

         num_clears++;
         clear_types |= SI_CLEAR_TYPE_DCC;

         si_mark_display_dcc_dirty(sctx, tex);

         /* DCC fast clear with MSAA should clear CMASK to 0xC. */
         if (tex->buffer.b.b.nr_samples >= 2 && tex->cmask_buffer) {
            assert(sctx->gfx_level < GFX11); /* no FMASK/CMASK on GFX11 */
            assert(num_clears < ARRAY_SIZE(info));
            si_init_buffer_clear(&info[num_clears++], &tex->cmask_buffer->b.b,
                                 tex->surface.cmask_offset, tex->surface.cmask_size, 0xCCCCCCCC);
            clear_types |= SI_CLEAR_TYPE_CMASK;
            fmask_decompress_needed = true;
         }
      } else {
         /* No CMASK on GFX11. */
         if (sctx->gfx_level >= GFX11)
            continue;

         if (level > 0)
            continue;

         /* Shared textures can't use fast clear without an explicit flush
          * because the clear color is not exported.
          */
         if (tex->buffer.b.is_shared &&
             !(tex->buffer.external_usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH))
            continue;

         if (too_small)
            continue;

         /* 128-bit formats are unsupported */
         if (tex->surface.bpe > 8) {
            continue;
         }

         /* RB+ doesn't work with CMASK fast clear on Stoney. */
         if (sctx->family == CHIP_STONEY)
            continue;

         /* Disable fast clear if tex is encrypted */
         if (tex->buffer.flags & RADEON_FLAG_ENCRYPTED)
            continue;

         uint64_t cmask_offset = 0;
         unsigned clear_size = 0;
         bool had_cmask_buffer = tex->cmask_buffer != NULL;

         if (sctx->gfx_level >= GFX10) {
            assert(level == 0);

            /* Clearing CMASK with both multiple levels and multiple layers is not
             * implemented.
             */
            if (num_layers > 1 && tex->buffer.b.b.last_level > 0)
               continue;

            if (!si_alloc_separate_cmask(sctx->screen, tex))
               continue;

            if (num_layers == 1) {
               /* Clear level 0. */
               cmask_offset = tex->surface.cmask_offset + tex->surface.u.gfx9.color.cmask_level0.offset;
               clear_size = tex->surface.u.gfx9.color.cmask_level0.size;
            } else if (tex->buffer.b.b.last_level == 0) {
               /* Clear all layers having only 1 level. */
               cmask_offset = tex->surface.cmask_offset;
               clear_size = tex->surface.cmask_size;
            } else {
               assert(0); /* this is prevented above */
            }
         } else if (sctx->gfx_level == GFX9) {
            /* TODO: Implement CMASK fast clear for level 0 of mipmapped textures. Mipmapped
             * CMASK has to clear a rectangular area of CMASK for level 0 (because the whole
             * miptree is organized in a 2D plane).
             */
            if (tex->buffer.b.b.last_level > 0)
               continue;

            if (!si_alloc_separate_cmask(sctx->screen, tex))
               continue;

            cmask_offset = tex->surface.cmask_offset;
            clear_size = tex->surface.cmask_size;
         } else {
            if (!si_alloc_separate_cmask(sctx->screen, tex))
               continue;

            /* GFX6-8: This only covers mipmap level 0. */
            cmask_offset = tex->surface.cmask_offset;
            clear_size = tex->surface.cmask_size;
         }

         /* Do the fast clear. */
         assert(num_clears < ARRAY_SIZE(info));
         si_init_buffer_clear(&info[num_clears++], &tex->cmask_buffer->b.b,
                              cmask_offset, clear_size, 0);
         clear_types |= SI_CLEAR_TYPE_CMASK;
         eliminate_needed = true;
         /* If we allocated a cmask buffer for this tex we need to re-emit
          * the fb state.
          */
         need_dirtying_fb = !had_cmask_buffer;
      }

      if ((eliminate_needed || fmask_decompress_needed) &&
          !(tex->dirty_level_mask & (1 << level))) {
         assert(sctx->gfx_level < GFX11); /* no decompression needed on GFX11 */
         tex->dirty_level_mask |= 1 << level;
         p_atomic_inc(&sctx->screen->compressed_colortex_counter);
      }

      *buffers &= ~(PIPE_CLEAR_COLOR0 << i);

      /* Chips with DCC constant encoding don't need to set the clear
       * color registers for DCC clear values 0 and 1.
       */
      if (sctx->screen->info.has_dcc_constant_encode && !eliminate_needed)
         continue;

      /* There are no clear color registers on GFX11. */
      assert(sctx->gfx_level < GFX11);

      if (si_set_clear_color(tex, fb->cbufs[i]->format, color) || need_dirtying_fb) {
         sctx->framebuffer.dirty_cbufs |= 1 << i;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);
      }
   }

   /* Depth/stencil clears. */
   struct pipe_surface *zsbuf = fb->zsbuf;
   struct si_texture *zstex = zsbuf ? (struct si_texture *)zsbuf->texture : NULL;
   unsigned zs_num_layers = zstex ? util_num_layers(&zstex->buffer.b.b, zsbuf->u.tex.level) : 0;

   if (zstex && zsbuf->u.tex.first_layer == 0 &&
       zsbuf->u.tex.last_layer == zs_num_layers - 1 &&
       si_htile_enabled(zstex, zsbuf->u.tex.level, PIPE_MASK_ZS)) {
      unsigned level = zsbuf->u.tex.level;
      bool update_db_depth_clear = false;
      bool update_db_stencil_clear = false;
      bool fb_too_small = num_pixels * zs_num_layers <= 512 * 512;

      /* Transition from TC-incompatible to TC-compatible HTILE if requested. */
      if (zstex->enable_tc_compatible_htile_next_clear) {
          /* If both depth and stencil are present, they must be cleared together. */
         if ((*buffers & PIPE_CLEAR_DEPTHSTENCIL) == PIPE_CLEAR_DEPTHSTENCIL ||
             (*buffers & PIPE_CLEAR_DEPTH && (!zstex->surface.has_stencil ||
                                              zstex->htile_stencil_disabled))) {
            /* The conversion from TC-incompatible to TC-compatible can only be done in one clear. */
            assert(zstex->buffer.b.b.last_level == 0);
            assert(!zstex->tc_compatible_htile);

            /* Enable TC-compatible HTILE. */
            zstex->enable_tc_compatible_htile_next_clear = false;
            zstex->tc_compatible_htile = true;

            /* Update the framebuffer state to reflect the change. */
            sctx->framebuffer.DB_has_shader_readable_metadata = true;
            sctx->framebuffer.dirty_zsbuf = true;
            si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);

            /* Update all sampler views and shader images in all contexts. */
            p_atomic_inc(&sctx->screen->dirty_tex_counter);

            /* Perform the clear here if possible, else clear to uncompressed. */
            uint32_t clear_value;

            if (zstex->htile_stencil_disabled || !zstex->surface.has_stencil) {
               if (si_can_fast_clear_depth(zstex, level, depth, *buffers)) {
                  /* Z-only clear. */
                  clear_value = si_get_htile_clear_value(zstex, depth);
                  *buffers &= ~PIPE_CLEAR_DEPTH;
                  zstex->depth_cleared_level_mask_once |= BITFIELD_BIT(level);
                  zstex->depth_cleared_level_mask |= BITFIELD_BIT(level);
                  update_db_depth_clear = true;
               }
            } else if ((*buffers & PIPE_CLEAR_DEPTHSTENCIL) == PIPE_CLEAR_DEPTHSTENCIL) {
               if (si_can_fast_clear_depth(zstex, level, depth, *buffers) &&
                   si_can_fast_clear_stencil(zstex, level, stencil, *buffers)) {
                  /* Combined Z+S clear. */
                  clear_value = si_get_htile_clear_value(zstex, depth);
                  *buffers &= ~PIPE_CLEAR_DEPTHSTENCIL;
                  zstex->depth_cleared_level_mask_once |= BITFIELD_BIT(level);
                  zstex->depth_cleared_level_mask |= BITFIELD_BIT(level);
                  zstex->stencil_cleared_level_mask_once |= BITFIELD_BIT(level);
                  update_db_depth_clear = true;
                  update_db_stencil_clear = true;
               }
            }

            if (!update_db_depth_clear) {
               /* Clear to uncompressed, so that it doesn't contain values incompatible
                * with the new TC-compatible HTILE setting.
                *
                * 0xfffff30f = uncompressed Z + S
                * 0xfffc000f = uncompressed Z only
                */
               clear_value = !zstex->htile_stencil_disabled ? 0xfffff30f : 0xfffc000f;
            }

            zstex->need_flush_after_depth_decompression = sctx->gfx_level == GFX10_3;

            assert(num_clears < ARRAY_SIZE(info));
            si_init_buffer_clear(&info[num_clears++], &zstex->buffer.b.b,
                                 zstex->surface.meta_offset, zstex->surface.meta_size, clear_value);
            clear_types |= SI_CLEAR_TYPE_HTILE;
         }
      } else if (num_clears || !fb_too_small) {
         /* This is where the HTILE buffer clear is done.
          *
          * If there is no clear scheduled and the framebuffer size is too small, we should use
          * the draw-based clear that is without waits. If there is some other clear scheduled,
          * we will have to wait anyway, so add the HTILE buffer clear to the batch here.
          * If the framebuffer size is large enough, use this codepath too.
          */
         uint64_t htile_offset = zstex->surface.meta_offset;
         unsigned htile_size = 0;

         /* Determine the HTILE subset to clear. */
         if (sctx->gfx_level >= GFX10) {
            /* This can only clear a layered texture with 1 level or a mipmap texture
             * with 1 layer. Other cases are unimplemented.
             */
            if (zs_num_layers == 1) {
               /* Clear a specific level. */
               htile_offset += zstex->surface.u.gfx9.meta_levels[level].offset;
               htile_size = zstex->surface.u.gfx9.meta_levels[level].size;
            } else if (zstex->buffer.b.b.last_level == 0) {
               /* Clear all layers having only 1 level. */
               htile_size = zstex->surface.meta_size;
            }
         } else {
            /* This can only clear a layered texture with 1 level. Other cases are
             * unimplemented.
             */
            if (zstex->buffer.b.b.last_level == 0)
               htile_size = zstex->surface.meta_size;
         }

         /* Perform the clear if it's possible. */
         if (zstex->htile_stencil_disabled || !zstex->surface.has_stencil) {
            if (htile_size &&
                si_can_fast_clear_depth(zstex, level, depth, *buffers)) {
               /* Z-only clear. */
               assert(num_clears < ARRAY_SIZE(info));
               si_init_buffer_clear(&info[num_clears++], &zstex->buffer.b.b, htile_offset,
                                    htile_size, si_get_htile_clear_value(zstex, depth));
               clear_types |= SI_CLEAR_TYPE_HTILE;
               *buffers &= ~PIPE_CLEAR_DEPTH;
               zstex->depth_cleared_level_mask_once |= BITFIELD_BIT(level);
               zstex->depth_cleared_level_mask |= BITFIELD_BIT(level);
               update_db_depth_clear = true;
            }
         } else if ((*buffers & PIPE_CLEAR_DEPTHSTENCIL) == PIPE_CLEAR_DEPTHSTENCIL) {
            if (htile_size &&
                si_can_fast_clear_depth(zstex, level, depth, *buffers) &&
                si_can_fast_clear_stencil(zstex, level, stencil, *buffers)) {
               /* Combined Z+S clear. */
               assert(num_clears < ARRAY_SIZE(info));
               si_init_buffer_clear(&info[num_clears++], &zstex->buffer.b.b, htile_offset,
                                    htile_size, si_get_htile_clear_value(zstex, depth));
               clear_types |= SI_CLEAR_TYPE_HTILE;
               *buffers &= ~PIPE_CLEAR_DEPTHSTENCIL;
               zstex->depth_cleared_level_mask_once |= BITFIELD_BIT(level);
               zstex->depth_cleared_level_mask |= BITFIELD_BIT(level);
               zstex->stencil_cleared_level_mask_once |= BITFIELD_BIT(level);
               update_db_depth_clear = true;
               update_db_stencil_clear = true;
            }
         } else {
            /* Z-only or S-only clear when both Z/S are present using a read-modify-write
             * compute shader.
             *
             * If we get both clears but only one of them can be fast-cleared, we use
             * the draw-based fast clear to do both at the same time.
             */
            const uint32_t htile_depth_writemask = 0xfffffc0f;
            const uint32_t htile_stencil_writemask = 0x000003f0;

            if (htile_size &&
                !(*buffers & PIPE_CLEAR_STENCIL) &&
                si_can_fast_clear_depth(zstex, level, depth, *buffers)) {
               /* Z-only clear with stencil left intact. */
               assert(num_clears < ARRAY_SIZE(info));
               si_init_buffer_clear_rmw(&info[num_clears++], &zstex->buffer.b.b, htile_offset,
                                        htile_size, si_get_htile_clear_value(zstex, depth),
                                        htile_depth_writemask);
               clear_types |= SI_CLEAR_TYPE_HTILE;
               *buffers &= ~PIPE_CLEAR_DEPTH;
               zstex->depth_cleared_level_mask_once |= BITFIELD_BIT(level);
               zstex->depth_cleared_level_mask |= BITFIELD_BIT(level);
               update_db_depth_clear = true;
            } else if (htile_size &&
                       !(*buffers & PIPE_CLEAR_DEPTH) &&
                       si_can_fast_clear_stencil(zstex, level, stencil, *buffers)) {
               /* Stencil-only clear with depth left intact. */
               assert(num_clears < ARRAY_SIZE(info));
               si_init_buffer_clear_rmw(&info[num_clears++], &zstex->buffer.b.b, htile_offset,
                                        htile_size, si_get_htile_clear_value(zstex, depth),
                                        htile_stencil_writemask);
               clear_types |= SI_CLEAR_TYPE_HTILE;
               *buffers &= ~PIPE_CLEAR_STENCIL;
               zstex->stencil_cleared_level_mask_once |= BITFIELD_BIT(level);
               update_db_stencil_clear = true;
            }
         }

         zstex->need_flush_after_depth_decompression = update_db_depth_clear && sctx->gfx_level == GFX10_3;
      }

      /* Update DB_DEPTH_CLEAR. */
      if (update_db_depth_clear &&
          zstex->depth_clear_value[level] != (float)depth) {
         zstex->depth_clear_value[level] = depth;
         sctx->framebuffer.dirty_zsbuf = true;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);
      }

      /* Update DB_STENCIL_CLEAR. */
      if (update_db_stencil_clear &&
          zstex->stencil_clear_value[level] != stencil) {
         zstex->stencil_clear_value[level] = stencil;
         sctx->framebuffer.dirty_zsbuf = true;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);
      }
   }

   si_execute_clears(sctx, info, num_clears, clear_types);
}

static void si_clear(struct pipe_context *ctx, unsigned buffers,
                     const struct pipe_scissor_state *scissor_state,
                     const union pipe_color_union *color, double depth, unsigned stencil)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct pipe_framebuffer_state *fb = &sctx->framebuffer.state;
   struct pipe_surface *zsbuf = fb->zsbuf;
   struct si_texture *zstex = zsbuf ? (struct si_texture *)zsbuf->texture : NULL;
   bool needs_db_flush = false;

   /* Unset clear flags for non-existent buffers. */
   for (unsigned i = 0; i < 8; i++) {
      if (i >= fb->nr_cbufs || !fb->cbufs[i])
         buffers &= ~(PIPE_CLEAR_COLOR0 << i);
   }
   if (!zsbuf)
      buffers &= ~PIPE_CLEAR_DEPTHSTENCIL;
   else if (!util_format_has_stencil(util_format_description(zsbuf->format)))
      buffers &= ~PIPE_CLEAR_STENCIL;

   si_fast_clear(sctx, &buffers, color, depth, stencil);
   if (!buffers)
      return; /* all buffers have been cleared */

   if (buffers & PIPE_CLEAR_COLOR) {
      /* These buffers cannot use fast clear, make sure to disable expansion. */
      unsigned color_buffer_mask = (buffers & PIPE_CLEAR_COLOR) >> util_logbase2(PIPE_CLEAR_COLOR0);
      while (color_buffer_mask) {
         unsigned i = u_bit_scan(&color_buffer_mask);
         struct si_texture *tex = (struct si_texture *)fb->cbufs[i]->texture;
         if (tex->surface.fmask_size == 0)
            tex->dirty_level_mask &= ~(1 << fb->cbufs[i]->u.tex.level);
      }
   }

   if (zstex && zsbuf->u.tex.first_layer == 0 &&
       zsbuf->u.tex.last_layer == util_max_layer(&zstex->buffer.b.b, 0)) {
      unsigned level = zsbuf->u.tex.level;

      if (si_can_fast_clear_depth(zstex, level, depth, buffers)) {
         /* Need to disable EXPCLEAR temporarily if clearing
          * to a new value. */
         if (!(zstex->depth_cleared_level_mask_once & BITFIELD_BIT(level)) ||
             zstex->depth_clear_value[level] != depth) {
            sctx->db_depth_disable_expclear = true;
         }

         if (zstex->depth_clear_value[level] != (float)depth) {
            if ((zstex->depth_clear_value[level] != 0) != (depth != 0)) {
               /* ZRANGE_PRECISION register of a bound surface will change so we
                * must flush the DB caches. */
               needs_db_flush = true;
            }
            /* Update DB_DEPTH_CLEAR. */
            zstex->depth_clear_value[level] = depth;
            sctx->framebuffer.dirty_zsbuf = true;
            si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);
         }
         sctx->db_depth_clear = true;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);
      }

      if (si_can_fast_clear_stencil(zstex, level, stencil, buffers)) {
         stencil &= 0xff;

         /* Need to disable EXPCLEAR temporarily if clearing
          * to a new value. */
         if (!(zstex->stencil_cleared_level_mask_once & BITFIELD_BIT(level)) ||
             zstex->stencil_clear_value[level] != stencil) {
            sctx->db_stencil_disable_expclear = true;
         }

         if (zstex->stencil_clear_value[level] != (uint8_t)stencil) {
            /* Update DB_STENCIL_CLEAR. */
            zstex->stencil_clear_value[level] = stencil;
            sctx->framebuffer.dirty_zsbuf = true;
            si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);
         }
         sctx->db_stencil_clear = true;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);
      }

      if (needs_db_flush) {
         sctx->flags |= SI_CONTEXT_FLUSH_AND_INV_DB;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
      }
   }

   if (unlikely(sctx->sqtt_enabled)) {
      if (buffers & PIPE_CLEAR_COLOR)
         sctx->sqtt_next_event = EventCmdClearColorImage;
      else if (buffers & PIPE_CLEAR_DEPTHSTENCIL)
         sctx->sqtt_next_event = EventCmdClearDepthStencilImage;
   }

   si_blitter_begin(sctx, SI_CLEAR);
   util_blitter_clear(sctx->blitter, fb->width, fb->height, util_framebuffer_get_num_layers(fb),
                      buffers, color, depth, stencil, sctx->framebuffer.nr_samples > 1);
   si_blitter_end(sctx);

   if (sctx->db_depth_clear) {
      sctx->db_depth_clear = false;
      sctx->db_depth_disable_expclear = false;
      zstex->depth_cleared_level_mask_once |= BITFIELD_BIT(zsbuf->u.tex.level);
      zstex->depth_cleared_level_mask |= BITFIELD_BIT(zsbuf->u.tex.level);
      si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);
   }

   if (sctx->db_stencil_clear) {
      sctx->db_stencil_clear = false;
      sctx->db_stencil_disable_expclear = false;
      zstex->stencil_cleared_level_mask_once |= BITFIELD_BIT(zsbuf->u.tex.level);
      si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);
   }
}

static bool si_try_normal_clear(struct si_context *sctx, struct pipe_surface *dst,
                                unsigned dstx, unsigned dsty, unsigned width, unsigned height,
                                bool render_condition_enabled, unsigned buffers,
                                const union pipe_color_union *color,
                                float depth, unsigned stencil)
{
   /* This is worth it only if it's a whole image clear, so that we just clear DCC/HTILE. */
   if (dstx == 0 && dsty == 0 &&
       width == dst->width &&
       height == dst->height &&
       dst->u.tex.first_layer == 0 &&
       dst->u.tex.last_layer == util_max_layer(dst->texture, dst->u.tex.level) &&
       /* pipe->clear honors render_condition, so only use it if it's unset or if it's set and enabled. */
       (!sctx->render_cond || render_condition_enabled) &&
       sctx->has_graphics) {
      struct pipe_context *ctx = &sctx->b;
      struct pipe_framebuffer_state saved_fb = {}, fb = {};

      util_copy_framebuffer_state(&saved_fb, &sctx->framebuffer.state);

      if (buffers & PIPE_CLEAR_COLOR) {
         fb.cbufs[0] = dst;
         fb.nr_cbufs = 1;
      } else {
         fb.zsbuf = dst;
      }

      fb.width = dst->width;
      fb.height = dst->height;

      ctx->set_framebuffer_state(ctx, &fb);
      ctx->clear(ctx, buffers, NULL, color, depth, stencil);
      ctx->set_framebuffer_state(ctx, &saved_fb);

      util_copy_framebuffer_state(&saved_fb, NULL);

      return true;
   }

   return false;
}

static void si_clear_render_target(struct pipe_context *ctx, struct pipe_surface *dst,
                                   const union pipe_color_union *color, unsigned dstx,
                                   unsigned dsty, unsigned width, unsigned height,
                                   bool render_condition_enabled)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_texture *sdst = (struct si_texture *)dst->texture;

   /* Fast path that just clears DCC. */
   if (si_try_normal_clear(sctx, dst, dstx, dsty, width, height, render_condition_enabled,
                           PIPE_CLEAR_COLOR0, color, 0, 0))
      return;

   if (dst->texture->nr_samples <= 1 &&
       (sctx->gfx_level >= GFX10 || !vi_dcc_enabled(sdst, dst->u.tex.level))) {
      si_compute_clear_render_target(ctx, dst, color, dstx, dsty, width, height,
                                     render_condition_enabled);
      return;
   }

   si_blitter_begin(sctx,
                    SI_CLEAR_SURFACE | (render_condition_enabled ? 0 : SI_DISABLE_RENDER_COND));
   util_blitter_clear_render_target(sctx->blitter, dst, color, dstx, dsty, width, height);
   si_blitter_end(sctx);
}

static void si_clear_depth_stencil(struct pipe_context *ctx, struct pipe_surface *dst,
                                   unsigned clear_flags, double depth, unsigned stencil,
                                   unsigned dstx, unsigned dsty, unsigned width, unsigned height,
                                   bool render_condition_enabled)
{
   struct si_context *sctx = (struct si_context *)ctx;
   union pipe_color_union unused = {};

   /* Fast path that just clears HTILE. */
   if (si_try_normal_clear(sctx, dst, dstx, dsty, width, height, render_condition_enabled,
                           clear_flags, &unused, depth, stencil))
      return;

   si_blitter_begin(sctx,
                    SI_CLEAR_SURFACE | (render_condition_enabled ? 0 : SI_DISABLE_RENDER_COND));
   util_blitter_clear_depth_stencil(sctx->blitter, dst, clear_flags, depth, stencil, dstx, dsty,
                                    width, height);
   si_blitter_end(sctx);
}

void si_init_clear_functions(struct si_context *sctx)
{
   sctx->b.clear_render_target = si_clear_render_target;
   sctx->b.clear_texture = u_default_clear_texture;

   if (sctx->has_graphics) {
      sctx->b.clear = si_clear;
      sctx->b.clear_depth_stencil = si_clear_depth_stencil;
   }
}
