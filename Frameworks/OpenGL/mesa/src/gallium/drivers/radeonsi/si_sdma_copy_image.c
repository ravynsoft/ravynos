/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 * Copyright 2015-2021 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_build_pm4.h"
#include "sid.h"
#include "util/u_memory.h"


static
bool si_prepare_for_sdma_copy(struct si_context *sctx, struct si_texture *dst,struct si_texture *src)
{
   if (dst->surface.bpe != src->surface.bpe)
      return false;

   /* MSAA: Blits don't exist in the real world. */
   if (src->buffer.b.b.nr_samples > 1 || dst->buffer.b.b.nr_samples > 1)
      return false;

   if (dst->buffer.b.b.last_level != 0 || src->buffer.b.b.last_level != 0)
      return false;

   return true;
}

static unsigned minify_as_blocks(unsigned width, unsigned level, unsigned blk_w)
{
   width = u_minify(width, level);
   return DIV_ROUND_UP(width, blk_w);
}

static unsigned encode_legacy_tile_info(struct si_context *sctx, struct si_texture *tex)
{
   struct radeon_info *info = &sctx->screen->info;
   unsigned tile_index = tex->surface.u.legacy.tiling_index[0];
   unsigned macro_tile_index = tex->surface.u.legacy.macro_tile_index;
   unsigned tile_mode = info->si_tile_mode_array[tile_index];
   unsigned macro_tile_mode = info->cik_macrotile_mode_array[macro_tile_index];

   return util_logbase2(tex->surface.bpe) |
          (G_009910_ARRAY_MODE(tile_mode) << 3) |
          (G_009910_MICRO_TILE_MODE_NEW(tile_mode) << 8) |
          /* Non-depth modes don't have TILE_SPLIT set. */
          ((util_logbase2(tex->surface.u.legacy.tile_split >> 6)) << 11) |
          (G_009990_BANK_WIDTH(macro_tile_mode) << 15) |
          (G_009990_BANK_HEIGHT(macro_tile_mode) << 18) |
          (G_009990_NUM_BANKS(macro_tile_mode) << 21) |
          (G_009990_MACRO_TILE_ASPECT(macro_tile_mode) << 24) |
          (G_009910_PIPE_CONFIG(tile_mode) << 26);
}

static bool si_sdma_v4_v5_copy_texture(struct si_context *sctx, struct si_texture *sdst,
                                       struct si_texture *ssrc)
{
   bool is_v5 = sctx->gfx_level >= GFX10;
   bool is_v5_2 = sctx->gfx_level >= GFX10_3;
   unsigned bpp = sdst->surface.bpe;
   uint64_t dst_address = sdst->buffer.gpu_address + sdst->surface.u.gfx9.surf_offset;
   uint64_t src_address = ssrc->buffer.gpu_address + ssrc->surface.u.gfx9.surf_offset;
   unsigned dst_pitch = sdst->surface.u.gfx9.surf_pitch;
   unsigned src_pitch = ssrc->surface.u.gfx9.surf_pitch;
   unsigned copy_width = DIV_ROUND_UP(ssrc->buffer.b.b.width0, ssrc->surface.blk_w);
   unsigned copy_height = DIV_ROUND_UP(ssrc->buffer.b.b.height0, ssrc->surface.blk_h);

   bool tmz = (ssrc->buffer.flags & RADEON_FLAG_ENCRYPTED);
   assert (!tmz || (sdst->buffer.flags & RADEON_FLAG_ENCRYPTED));

   /* Linear -> linear sub-window copy. */
   if (ssrc->surface.is_linear && sdst->surface.is_linear) {
      struct radeon_cmdbuf *cs = sctx->sdma_cs;

      uint64_t bytes = (uint64_t)src_pitch * copy_height * bpp;
      uint32_t chunk_size = 1u << (is_v5_2 ? 30 : 22);
      uint32_t chunk_count = DIV_ROUND_UP(bytes, chunk_size);

      src_address += ssrc->surface.u.gfx9.offset[0];
      dst_address += sdst->surface.u.gfx9.offset[0];

      radeon_begin(cs);
      for (int i = 0; i < chunk_count; i++) {
         uint32_t size = MIN2(chunk_size, bytes);
         radeon_emit(SDMA_PACKET(SDMA_OPCODE_COPY,
                                     SDMA_COPY_SUB_OPCODE_LINEAR,
                                     (tmz ? 4 : 0)));
         radeon_emit(size - 1);
         radeon_emit(0);
         radeon_emit(src_address);
         radeon_emit(src_address >> 32);
         radeon_emit(dst_address);
         radeon_emit(dst_address >> 32);

         src_address += size;
         dst_address += size;
         bytes -= size;
      }
      radeon_end();
      return true;
   }

   /* Linear <-> Tiled sub-window copy */
   if (ssrc->surface.is_linear != sdst->surface.is_linear) {
      struct si_texture *tiled = ssrc->surface.is_linear ? sdst : ssrc;
      struct si_texture *linear = tiled == ssrc ? sdst : ssrc;
      unsigned tiled_width = DIV_ROUND_UP(tiled->buffer.b.b.width0, tiled->surface.blk_w);
      unsigned tiled_height = DIV_ROUND_UP(tiled->buffer.b.b.height0, tiled->surface.blk_h);
      unsigned linear_pitch = linear == ssrc ? src_pitch : dst_pitch;
      uint64_t linear_slice_pitch = linear->surface.u.gfx9.surf_slice_size / bpp;
      uint64_t tiled_address = tiled == ssrc ? src_address : dst_address;
      uint64_t linear_address = linear == ssrc ? src_address : dst_address;
      struct radeon_cmdbuf *cs = sctx->sdma_cs;
      /* Only SDMA 5 supports DCC with SDMA */
      bool dcc = vi_dcc_enabled(tiled, 0) && is_v5;
      assert(tiled->buffer.b.b.depth0 == 1);

      linear_address += linear->surface.u.gfx9.offset[0];

      /* Check if everything fits into the bitfields */
      if (!(tiled_width <= (1 << 14) && tiled_height <= (1 << 14) &&
            linear_pitch <= (1 << 14) && linear_slice_pitch <= (1 << 28) &&
            copy_width <= (1 << 14) && copy_height <= (1 << 14)))
         return false;

      radeon_begin(cs);
      radeon_emit(
         SDMA_PACKET(SDMA_OPCODE_COPY,
                     SDMA_COPY_SUB_OPCODE_TILED_SUB_WINDOW,
                         (tmz ? 4 : 0)) |
         dcc << 19 |
         (is_v5 ? 0 : tiled->buffer.b.b.last_level) << 20 |
         (linear == sdst ? 1u : 0) << 31);
      radeon_emit((uint32_t)tiled_address | (tiled->surface.tile_swizzle << 8));
      radeon_emit((uint32_t)(tiled_address >> 32));
      radeon_emit(0);
      radeon_emit(((tiled_width - 1) << 16));
      radeon_emit((tiled_height - 1));
      radeon_emit(util_logbase2(bpp) |
                  tiled->surface.u.gfx9.swizzle_mode << 3 |
                  tiled->surface.u.gfx9.resource_type << 9 |
                  (is_v5 ? tiled->buffer.b.b.last_level : tiled->surface.u.gfx9.epitch) << 16);
      radeon_emit((uint32_t)linear_address);
      radeon_emit((uint32_t)(linear_address >> 32));
      radeon_emit(0);
      radeon_emit(((linear_pitch - 1) << 16));
      radeon_emit(linear_slice_pitch - 1);
      radeon_emit((copy_width - 1) | ((copy_height - 1) << 16));
      radeon_emit(0);

      if (dcc) {
         unsigned hw_fmt = ac_get_cb_format(sctx->gfx_level, tiled->buffer.b.b.format);
         unsigned hw_type = ac_get_cb_number_type(tiled->buffer.b.b.format);
         uint64_t md_address = tiled_address + tiled->surface.meta_offset;

         /* Add metadata */
         radeon_emit((uint32_t)md_address);
         radeon_emit((uint32_t)(md_address >> 32));
         radeon_emit(hw_fmt |
                     vi_alpha_is_on_msb(sctx->screen, tiled->buffer.b.b.format) << 8 |
                     hw_type << 9 |
                     tiled->surface.u.gfx9.color.dcc.max_compressed_block_size << 24 |
                     V_028C78_MAX_BLOCK_SIZE_256B << 26 |
                     tmz << 29 |
                     tiled->surface.u.gfx9.color.dcc.pipe_aligned << 31);
      }
      radeon_end();
      return true;
   }

   return false;
}

static
bool cik_sdma_copy_texture(struct si_context *sctx, struct si_texture *sdst, struct si_texture *ssrc)
{
   struct radeon_info *info = &sctx->screen->info;
   unsigned bpp = sdst->surface.bpe;
   uint64_t dst_address = sdst->buffer.gpu_address + sdst->surface.u.legacy.level[0].offset_256B * 256;
   uint64_t src_address = ssrc->buffer.gpu_address + ssrc->surface.u.legacy.level[0].offset_256B * 256;
   unsigned dst_mode = sdst->surface.u.legacy.level[0].mode;
   unsigned src_mode = ssrc->surface.u.legacy.level[0].mode;
   unsigned dst_tile_index = sdst->surface.u.legacy.tiling_index[0];
   unsigned src_tile_index = ssrc->surface.u.legacy.tiling_index[0];
   unsigned dst_tile_mode = info->si_tile_mode_array[dst_tile_index];
   unsigned src_tile_mode = info->si_tile_mode_array[src_tile_index];
   unsigned dst_micro_mode = G_009910_MICRO_TILE_MODE_NEW(dst_tile_mode);
   unsigned src_micro_mode = G_009910_MICRO_TILE_MODE_NEW(src_tile_mode);
   unsigned dst_tile_swizzle = dst_mode == RADEON_SURF_MODE_2D ? sdst->surface.tile_swizzle : 0;
   unsigned src_tile_swizzle = src_mode == RADEON_SURF_MODE_2D ? ssrc->surface.tile_swizzle : 0;
   unsigned dst_pitch = sdst->surface.u.legacy.level[0].nblk_x;
   unsigned src_pitch = ssrc->surface.u.legacy.level[0].nblk_x;
   uint64_t dst_slice_pitch =
      ((uint64_t)sdst->surface.u.legacy.level[0].slice_size_dw * 4) / bpp;
   uint64_t src_slice_pitch =
      ((uint64_t)ssrc->surface.u.legacy.level[0].slice_size_dw * 4) / bpp;
   unsigned dst_width = minify_as_blocks(sdst->buffer.b.b.width0, 0, sdst->surface.blk_w);
   unsigned src_width = minify_as_blocks(ssrc->buffer.b.b.width0, 0, ssrc->surface.blk_w);
   unsigned copy_width = DIV_ROUND_UP(ssrc->buffer.b.b.width0, ssrc->surface.blk_w);
   unsigned copy_height = DIV_ROUND_UP(ssrc->buffer.b.b.height0, ssrc->surface.blk_h);

   dst_address |= dst_tile_swizzle << 8;
   src_address |= src_tile_swizzle << 8;

   /* Linear -> linear sub-window copy. */
   if (dst_mode == RADEON_SURF_MODE_LINEAR_ALIGNED && src_mode == RADEON_SURF_MODE_LINEAR_ALIGNED &&
       /* check if everything fits into the bitfields */
       src_pitch <= (1 << 14) && dst_pitch <= (1 << 14) && src_slice_pitch <= (1 << 28) &&
       dst_slice_pitch <= (1 << 28) && copy_width <= (1 << 14) && copy_height <= (1 << 14) &&
       /* HW limitation - GFX7: */
       (sctx->gfx_level != GFX7 ||
        (copy_width < (1 << 14) && copy_height < (1 << 14))) &&
       /* HW limitation - some GFX7 parts: */
       ((sctx->family != CHIP_BONAIRE && sctx->family != CHIP_KAVERI) ||
        (copy_width != (1 << 14) && copy_height != (1 << 14)))) {
      struct radeon_cmdbuf *cs = sctx->sdma_cs;

      radeon_begin(cs);
      radeon_emit(SDMA_PACKET(SDMA_OPCODE_COPY, SDMA_COPY_SUB_OPCODE_LINEAR_SUB_WINDOW, 0) |
                  (util_logbase2(bpp) << 29));
      radeon_emit(src_address);
      radeon_emit(src_address >> 32);
      radeon_emit(0);
      radeon_emit((src_pitch - 1) << 16);
      radeon_emit(src_slice_pitch - 1);
      radeon_emit(dst_address);
      radeon_emit(dst_address >> 32);
      radeon_emit(0);
      radeon_emit((dst_pitch - 1) << 16);
      radeon_emit(dst_slice_pitch - 1);
      if (sctx->gfx_level == GFX7) {
         radeon_emit(copy_width | (copy_height << 16));
         radeon_emit(0);
      } else {
         radeon_emit((copy_width - 1) | ((copy_height - 1) << 16));
         radeon_emit(0);
      }
      radeon_end();
      return true;
   }

   /* Tiled <-> linear sub-window copy. */
   if ((src_mode >= RADEON_SURF_MODE_1D) != (dst_mode >= RADEON_SURF_MODE_1D)) {
      struct si_texture *tiled = src_mode >= RADEON_SURF_MODE_1D ? ssrc : sdst;
      struct si_texture *linear = tiled == ssrc ? sdst : ssrc;
      unsigned tiled_width = tiled == ssrc ? src_width : dst_width;
      unsigned linear_width = linear == ssrc ? src_width : dst_width;
      unsigned tiled_pitch = tiled == ssrc ? src_pitch : dst_pitch;
      unsigned linear_pitch = linear == ssrc ? src_pitch : dst_pitch;
      unsigned tiled_slice_pitch = tiled == ssrc ? src_slice_pitch : dst_slice_pitch;
      unsigned linear_slice_pitch = linear == ssrc ? src_slice_pitch : dst_slice_pitch;
      uint64_t tiled_address = tiled == ssrc ? src_address : dst_address;
      uint64_t linear_address = linear == ssrc ? src_address : dst_address;
      unsigned tiled_micro_mode = tiled == ssrc ? src_micro_mode : dst_micro_mode;

      assert(tiled_pitch % 8 == 0);
      assert(tiled_slice_pitch % 64 == 0);
      unsigned pitch_tile_max = tiled_pitch / 8 - 1;
      unsigned slice_tile_max = tiled_slice_pitch / 64 - 1;
      unsigned xalign = MAX2(1, 4 / bpp);
      unsigned copy_width_aligned = copy_width;

      /* If the region ends at the last pixel and is unaligned, we
       * can copy the remainder of the line that is not visible to
       * make it aligned.
       */
      if (copy_width % xalign != 0 && 0 + copy_width == linear_width &&
          copy_width == tiled_width &&
          align(copy_width, xalign) <= linear_pitch &&
          align(copy_width, xalign) <= tiled_pitch)
         copy_width_aligned = align(copy_width, xalign);

      /* HW limitations. */
      if ((sctx->family == CHIP_BONAIRE || sctx->family == CHIP_KAVERI) &&
          linear_pitch - 1 == 0x3fff && bpp == 16)
         return false;

      if ((sctx->family == CHIP_BONAIRE || sctx->family == CHIP_KAVERI ||
           sctx->family == CHIP_KABINI) &&
          (copy_width == (1 << 14) || copy_height == (1 << 14)))
         return false;

      /* The hw can read outside of the given linear buffer bounds,
       * or access those pages but not touch the memory in case
       * of writes. (it still causes a VM fault)
       *
       * Out-of-bounds memory access or page directory access must
       * be prevented.
       */
      int64_t start_linear_address, end_linear_address;
      unsigned granularity;

      /* Deduce the size of reads from the linear surface. */
      switch (tiled_micro_mode) {
      case V_009910_ADDR_SURF_DISPLAY_MICRO_TILING:
         granularity = bpp == 1 ? 64 / (8 * bpp) : 128 / (8 * bpp);
         break;
      case V_009910_ADDR_SURF_THIN_MICRO_TILING:
      case V_009910_ADDR_SURF_DEPTH_MICRO_TILING:
         if (0 /* TODO: THICK microtiling */)
            granularity =
               bpp == 1 ? 32 / (8 * bpp)
                        : bpp == 2 ? 64 / (8 * bpp) : bpp <= 8 ? 128 / (8 * bpp) : 256 / (8 * bpp);
         else
            granularity = bpp <= 2 ? 64 / (8 * bpp) : bpp <= 8 ? 128 / (8 * bpp) : 256 / (8 * bpp);
         break;
      default:
         return false;
      }

      /* The linear reads start at tiled_x & ~(granularity - 1).
       * If linear_x == 0 && tiled_x % granularity != 0, the hw
       * starts reading from an address preceding linear_address!!!
       */
      start_linear_address =
         (uint64_t)linear->surface.u.legacy.level[0].offset_256B * 256;

      end_linear_address =
         (uint64_t)linear->surface.u.legacy.level[0].offset_256B * 256 +
         bpp * ((copy_height - 1) * (uint64_t)linear_pitch + copy_width);

      if ((0 + copy_width) % granularity)
         end_linear_address += granularity - (0 + copy_width) % granularity;

      if (start_linear_address < 0 || end_linear_address > linear->surface.surf_size)
         return false;

      /* Check requirements. */
      if (tiled_address % 256 == 0 && linear_address % 4 == 0 && linear_pitch % xalign == 0 &&
          copy_width_aligned % xalign == 0 &&
          tiled_micro_mode != V_009910_ADDR_SURF_ROTATED_MICRO_TILING &&
          /* check if everything fits into the bitfields */
          tiled->surface.u.legacy.tile_split <= 4096 && pitch_tile_max < (1 << 11) &&
          slice_tile_max < (1 << 22) && linear_pitch <= (1 << 14) &&
          linear_slice_pitch <= (1 << 28) && copy_width_aligned <= (1 << 14) &&
          copy_height <= (1 << 14)) {
         struct radeon_cmdbuf *cs = sctx->sdma_cs;
         uint32_t direction = linear == sdst ? 1u << 31 : 0;

         radeon_begin(cs);
         radeon_emit(SDMA_PACKET(SDMA_OPCODE_COPY,
                                 SDMA_COPY_SUB_OPCODE_TILED_SUB_WINDOW, 0) |
                     direction);
         radeon_emit(tiled_address);
         radeon_emit(tiled_address >> 32);
         radeon_emit(0);
         radeon_emit(pitch_tile_max << 16);
         radeon_emit(slice_tile_max);
         radeon_emit(encode_legacy_tile_info(sctx, tiled));
         radeon_emit(linear_address);
         radeon_emit(linear_address >> 32);
         radeon_emit(0);
         radeon_emit(((linear_pitch - 1) << 16));
         radeon_emit(linear_slice_pitch - 1);
         if (sctx->gfx_level == GFX7) {
            radeon_emit(copy_width_aligned | (copy_height << 16));
            radeon_emit(1);
         } else {
            radeon_emit((copy_width_aligned - 1) | ((copy_height - 1) << 16));
            radeon_emit(0);
         }
         radeon_end();
         return true;
      }
   }

   return false;
}

bool si_sdma_copy_image(struct si_context *sctx, struct si_texture *dst, struct si_texture *src)
{
   struct radeon_winsys *ws = sctx->ws;

   if (!sctx->sdma_cs) {
      if (sctx->screen->debug_flags & DBG(NO_DMA) || sctx->gfx_level < GFX7)
         return false;

      sctx->sdma_cs = CALLOC_STRUCT(radeon_cmdbuf);
      if (ws->cs_create(sctx->sdma_cs, sctx->ctx, AMD_IP_SDMA, NULL, NULL))
         return false;
   }

   if (!si_prepare_for_sdma_copy(sctx, dst, src))
      return false;

   /* TODO: DCC compression is possible on GFX10+. See si_set_mutable_tex_desc_fields for
    * additional constraints.
    * For now, the only use-case of SDMA is DRI_PRIME tiled->linear copy, and linear dst
    * never has DCC.
    */
   if (vi_dcc_enabled(dst, 0))
      return false;

   /* Decompress DCC on older chips where SDMA can't read it. */
   if (vi_dcc_enabled(src, 0) && sctx->gfx_level < GFX10)
      si_decompress_dcc(sctx, src);

   /* Always flush the gfx queue to get the winsys to handle the dependencies for us. */
   si_flush_gfx_cs(sctx, 0, NULL);

   switch (sctx->gfx_level) {
      case GFX7:
      case GFX8:
         if (!cik_sdma_copy_texture(sctx, dst, src))
            return false;
         break;
      case GFX9:
      case GFX10:
      case GFX10_3:
      case GFX11:
      case GFX11_5:
         if (!si_sdma_v4_v5_copy_texture(sctx, dst, src))
            return false;
         break;
      default:
         return false;
   }

   radeon_add_to_buffer_list(sctx, sctx->sdma_cs, &src->buffer, RADEON_USAGE_READ |
                             RADEON_PRIO_SAMPLER_TEXTURE);
   radeon_add_to_buffer_list(sctx, sctx->sdma_cs, &dst->buffer, RADEON_USAGE_WRITE |
                             RADEON_PRIO_SAMPLER_TEXTURE);

   unsigned flags = RADEON_FLUSH_START_NEXT_GFX_IB_NOW;
   if (unlikely(radeon_uses_secure_bos(sctx->ws))) {
      if ((bool) (src->buffer.flags & RADEON_FLAG_ENCRYPTED) !=
          sctx->ws->cs_is_secure(sctx->sdma_cs)) {
         flags = RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION;
      }
   }

   return ws->cs_flush(sctx->sdma_cs, flags, NULL) == 0;
}
