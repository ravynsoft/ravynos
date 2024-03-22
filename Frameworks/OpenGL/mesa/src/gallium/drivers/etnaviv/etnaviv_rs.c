/*
 * Copyright (c) 2012-2017 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#include "etnaviv_rs.h"

#include "etnaviv_clear_blit.h"
#include "etnaviv_context.h"
#include "etnaviv_emit.h"
#include "etnaviv_format.h"
#include "etnaviv_resource.h"
#include "etnaviv_screen.h"
#include "etnaviv_surface.h"
#include "etnaviv_tiling.h"
#include "etnaviv_translate.h"
#include "etnaviv_util.h"

#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/compiler.h"
#include "util/u_blitter.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_surface.h"

#include "hw/common.xml.h"
#include "hw/state.xml.h"
#include "hw/state_3d.xml.h"

#include <assert.h>

/* return a RS "compatible" format for use when copying */
static uint32_t
etna_compatible_rs_format(enum pipe_format fmt)
{
   /* YUYV and UYVY are blocksize 4, but 2 bytes per pixel */
   if (fmt == PIPE_FORMAT_YUYV || fmt == PIPE_FORMAT_UYVY)
      return RS_FORMAT_A4R4G4B4;

   switch (util_format_get_blocksize(fmt)) {
   case 2: return RS_FORMAT_A4R4G4B4;
   case 4: return RS_FORMAT_A8R8G8B8;
   default: return ETNA_NO_MATCH;
   }
}

void
etna_compile_rs_state(struct etna_context *ctx, struct compiled_rs_state *cs,
                      const struct rs_state *rs)
{
   struct etna_screen *screen = ctx->screen;

   memset(cs, 0, sizeof(*cs));

   /* TILED and SUPERTILED layout have their strides multiplied with 4 in RS */
   unsigned source_stride_shift = COND(rs->source_tiling != ETNA_LAYOUT_LINEAR, 2);
   unsigned dest_stride_shift = COND(rs->dest_tiling != ETNA_LAYOUT_LINEAR, 2);

   bool src_tiled = rs->source_tiling & ETNA_LAYOUT_BIT_TILE;
   bool dst_tiled = rs->dest_tiling & ETNA_LAYOUT_BIT_TILE;
   bool src_super = rs->source_tiling & ETNA_LAYOUT_BIT_SUPER;
   bool dst_super = rs->dest_tiling & ETNA_LAYOUT_BIT_SUPER;
   bool src_multi = rs->source_tiling & ETNA_LAYOUT_BIT_MULTI;
   bool dst_multi = rs->dest_tiling & ETNA_LAYOUT_BIT_MULTI;

   /* Vivante RS needs widths to be a multiple of 16 or bad things
    * happen, such as scribbing over memory, or the GPU hanging,
    * even for non-tiled formats.  As this is serious, use abort().
    */
   if (rs->width & ETNA_RS_WIDTH_MASK)
      abort();

   /* TODO could just pre-generate command buffer, would simply submit to one memcpy */
   cs->RS_CONFIG = VIVS_RS_CONFIG_SOURCE_FORMAT(rs->source_format) |
                   COND(rs->downsample_x, VIVS_RS_CONFIG_DOWNSAMPLE_X) |
                   COND(rs->downsample_y, VIVS_RS_CONFIG_DOWNSAMPLE_Y) |
                   COND(src_tiled, VIVS_RS_CONFIG_SOURCE_TILED) |
                   VIVS_RS_CONFIG_DEST_FORMAT(rs->dest_format) |
                   COND(dst_tiled, VIVS_RS_CONFIG_DEST_TILED) |
                   COND(rs->swap_rb, VIVS_RS_CONFIG_SWAP_RB) |
                   COND(rs->flip, VIVS_RS_CONFIG_FLIP);

   cs->RS_SOURCE_STRIDE = (rs->source_stride << source_stride_shift) |
                          COND(src_super, VIVS_RS_SOURCE_STRIDE_TILING) |
                          COND(src_multi, VIVS_RS_SOURCE_STRIDE_MULTI);

   if (VIV_FEATURE(ctx->screen, chipMinorFeatures6, CACHE128B256BPERLINE))
      cs->RS_SOURCE_STRIDE |= VIVS_RS_SOURCE_STRIDE_TS_MODE(rs->source_ts_mode) |
                              COND(src_super, VIVS_RS_SOURCE_STRIDE_SUPER_TILED_NEW);
   else if ((rs->downsample_x || rs->downsample_y) && VIV_FEATURE(screen, chipMinorFeatures4, SMALL_MSAA))
      cs->RS_SOURCE_STRIDE |= VIVS_RS_SOURCE_STRIDE_TS_MODE(TS_MODE_256B);

   /* Initially all pipes are set to the base address of the source and
    * destination buffer respectively. This will be overridden below as
    * necessary for the multi-pipe, multi-tiled case.
    */
   for (unsigned pipe = 0; pipe < screen->specs.pixel_pipes; ++pipe) {
      cs->source[pipe].bo = rs->source;
      cs->source[pipe].offset = rs->source_offset;
      cs->source[pipe].flags = ETNA_RELOC_READ;

      cs->dest[pipe].bo = rs->dest;
      cs->dest[pipe].offset = rs->dest_offset;
      cs->dest[pipe].flags = ETNA_RELOC_WRITE;

      cs->RS_PIPE_OFFSET[pipe] = VIVS_RS_PIPE_OFFSET_X(0) | VIVS_RS_PIPE_OFFSET_Y(0);
   }

   cs->RS_DEST_STRIDE = (rs->dest_stride << dest_stride_shift) |
                        COND(dst_super, VIVS_RS_DEST_STRIDE_TILING) |
                        COND(dst_multi, VIVS_RS_DEST_STRIDE_MULTI);

   if (VIV_FEATURE(ctx->screen, chipMinorFeatures6, CACHE128B256BPERLINE))
      cs->RS_DEST_STRIDE |= COND(dst_super, VIVS_RS_DEST_STRIDE_SUPER_TILED_NEW);

   if (src_multi)
      cs->source[1].offset = rs->source_offset + rs->source_stride * rs->source_padded_height / 2;

   if (dst_multi)
      cs->dest[1].offset = rs->dest_offset + rs->dest_stride * rs->dest_padded_height / 2;

   cs->RS_WINDOW_SIZE = VIVS_RS_WINDOW_SIZE_WIDTH(rs->width) |
                        VIVS_RS_WINDOW_SIZE_HEIGHT(rs->height);

   /* use dual pipe mode when required */
   if (!screen->specs.single_buffer && screen->specs.pixel_pipes == 2 &&
       !(rs->height & (rs->downsample_y ? 0xf : 0x7))) {
      cs->RS_WINDOW_SIZE = VIVS_RS_WINDOW_SIZE_WIDTH(rs->width) |
                              VIVS_RS_WINDOW_SIZE_HEIGHT(rs->height / 2);
      cs->RS_PIPE_OFFSET[1] = VIVS_RS_PIPE_OFFSET_X(0) | VIVS_RS_PIPE_OFFSET_Y(rs->height / 2);
   }

   cs->RS_DITHER[0] = rs->dither[0];
   cs->RS_DITHER[1] = rs->dither[1];
   cs->RS_CLEAR_CONTROL = VIVS_RS_CLEAR_CONTROL_BITS(rs->clear_bits) | rs->clear_mode;
   cs->RS_FILL_VALUE[0] = rs->clear_value[0];
   cs->RS_FILL_VALUE[1] = rs->clear_value[1];
   cs->RS_FILL_VALUE[2] = rs->clear_value[2];
   cs->RS_FILL_VALUE[3] = rs->clear_value[3];
   cs->RS_EXTRA_CONFIG = VIVS_RS_EXTRA_CONFIG_AA(rs->aa) |
                         VIVS_RS_EXTRA_CONFIG_ENDIAN(rs->endian_mode);

   /* If source the same as destination, and the hardware supports this,
    * do an in-place resolve to fill in unrendered tiles.
    */
   if (screen->specs.single_buffer && rs->source == rs->dest &&
         rs->source_offset == rs->dest_offset &&
         rs->source_format == rs->dest_format &&
         rs->source_tiling == rs->dest_tiling &&
         src_super &&
         rs->source_stride == rs->dest_stride &&
         !rs->downsample_x && !rs->downsample_y &&
         !rs->swap_rb && !rs->flip &&
         !rs->clear_mode && rs->source_padded_width &&
         !rs->source_ts_compressed) {
      if (VIV_FEATURE(ctx->screen, chipMinorFeatures6, CACHE128B256BPERLINE))
         cs->RS_EXTRA_CONFIG |= VIVS_RS_EXTRA_CONFIG_TS_MODE(rs->source_ts_mode);
      /* Total number of tiles (same as for autodisable) */
      cs->RS_KICKER_INPLACE = rs->tile_count;
   }
   cs->source_ts_valid = rs->source_ts_valid;
   cs->valid = true;
}

/* modify the clear bits value in the compiled RS state */
static void
etna_modify_rs_clearbits(struct compiled_rs_state *cs, uint32_t clear_bits)
{
   cs->RS_CLEAR_CONTROL &= ~VIVS_RS_CLEAR_CONTROL_BITS__MASK;
   cs->RS_CLEAR_CONTROL |= VIVS_RS_CLEAR_CONTROL_BITS(clear_bits);
}

#define EMIT_STATE(state_name, src_value) \
   etna_coalsence_emit(stream, &coalesce, VIVS_##state_name, src_value)

#define EMIT_STATE_FIXP(state_name, src_value) \
   etna_coalsence_emit_fixp(stream, &coalesce, VIVS_##state_name, src_value)

#define EMIT_STATE_RELOC(state_name, src_value) \
   etna_coalsence_emit_reloc(stream, &coalesce, VIVS_##state_name, src_value)

/* submit RS state, without any processing and no dependence on context
 * except TS if this is a source-to-destination blit. */
static void
etna_submit_rs_state(struct etna_context *ctx,
                     const struct compiled_rs_state *cs)
{
   struct etna_screen *screen = etna_screen(ctx->base.screen);
   struct etna_cmd_stream *stream = ctx->stream;
   struct etna_coalesce coalesce;

   if (cs->RS_KICKER_INPLACE && !cs->source_ts_valid)
      /* Inplace resolve is no-op if TS is not configured */
      return;

   ctx->stats.rs_operations++;

   if (cs->RS_KICKER_INPLACE) {
      etna_cmd_stream_reserve(stream, 6);
      etna_coalesce_start(stream, &coalesce);
      /* 0/1 */ EMIT_STATE(RS_EXTRA_CONFIG, cs->RS_EXTRA_CONFIG);
      /* 2/3 */ EMIT_STATE(RS_SOURCE_STRIDE, cs->RS_SOURCE_STRIDE);
      /* 4/5 */ EMIT_STATE(RS_KICKER_INPLACE, cs->RS_KICKER_INPLACE);
      etna_coalesce_end(stream, &coalesce);
   } else if (screen->specs.pixel_pipes > 1 ||
              VIV_FEATURE(screen, chipMinorFeatures7, RS_NEW_BASEADDR)) {
      etna_cmd_stream_reserve(stream, 34); /* worst case - both pipes multi=1 */
      etna_coalesce_start(stream, &coalesce);
      /* 0/1 */ EMIT_STATE(RS_CONFIG, cs->RS_CONFIG);
      /* 2/3 */ EMIT_STATE(RS_SOURCE_STRIDE, cs->RS_SOURCE_STRIDE);
      /* 4/5 */ EMIT_STATE(RS_DEST_STRIDE, cs->RS_DEST_STRIDE);
      /* 6/7 */ EMIT_STATE_RELOC(RS_PIPE_SOURCE_ADDR(0), &cs->source[0]);
      if (cs->RS_SOURCE_STRIDE & VIVS_RS_SOURCE_STRIDE_MULTI) {
         /*8 */ EMIT_STATE_RELOC(RS_PIPE_SOURCE_ADDR(1), &cs->source[1]);
         /*9 - pad */
      }
      /*10/11*/ EMIT_STATE_RELOC(RS_PIPE_DEST_ADDR(0), &cs->dest[0]);
      if (cs->RS_DEST_STRIDE & VIVS_RS_DEST_STRIDE_MULTI) {
         /*12*/ EMIT_STATE_RELOC(RS_PIPE_DEST_ADDR(1), &cs->dest[1]);
         /*13 - pad */
      }
      /*14/15*/ EMIT_STATE(RS_PIPE_OFFSET(0), cs->RS_PIPE_OFFSET[0]);
      /*16   */ EMIT_STATE(RS_PIPE_OFFSET(1), cs->RS_PIPE_OFFSET[1]);
      /*17 - pad */
      /*18/19*/ EMIT_STATE(RS_WINDOW_SIZE, cs->RS_WINDOW_SIZE);
      /*20/21*/ EMIT_STATE(RS_DITHER(0), cs->RS_DITHER[0]);
      /*22   */ EMIT_STATE(RS_DITHER(1), cs->RS_DITHER[1]);
      /*23 - pad */
      /*24/25*/ EMIT_STATE(RS_CLEAR_CONTROL, cs->RS_CLEAR_CONTROL);
      /*26   */ EMIT_STATE(RS_FILL_VALUE(0), cs->RS_FILL_VALUE[0]);
      /*27   */ EMIT_STATE(RS_FILL_VALUE(1), cs->RS_FILL_VALUE[1]);
      /*28   */ EMIT_STATE(RS_FILL_VALUE(2), cs->RS_FILL_VALUE[2]);
      /*29   */ EMIT_STATE(RS_FILL_VALUE(3), cs->RS_FILL_VALUE[3]);
      /*30/31*/ EMIT_STATE(RS_EXTRA_CONFIG, cs->RS_EXTRA_CONFIG);
      /*32/33*/ EMIT_STATE(RS_KICKER, 0xbeebbeeb);
      etna_coalesce_end(stream, &coalesce);
   } else {
      etna_cmd_stream_reserve(stream, 22);
      etna_coalesce_start(stream, &coalesce);
      /* 0/1 */ EMIT_STATE(RS_CONFIG, cs->RS_CONFIG);
      /* 2   */ EMIT_STATE_RELOC(RS_SOURCE_ADDR, &cs->source[0]);
      /* 3   */ EMIT_STATE(RS_SOURCE_STRIDE, cs->RS_SOURCE_STRIDE);
      /* 4   */ EMIT_STATE_RELOC(RS_DEST_ADDR, &cs->dest[0]);
      /* 5   */ EMIT_STATE(RS_DEST_STRIDE, cs->RS_DEST_STRIDE);
      /* 6/7 */ EMIT_STATE(RS_WINDOW_SIZE, cs->RS_WINDOW_SIZE);
      /* 8/9 */ EMIT_STATE(RS_DITHER(0), cs->RS_DITHER[0]);
      /*10   */ EMIT_STATE(RS_DITHER(1), cs->RS_DITHER[1]);
      /*11 - pad */
      /*12/13*/ EMIT_STATE(RS_CLEAR_CONTROL, cs->RS_CLEAR_CONTROL);
      /*14   */ EMIT_STATE(RS_FILL_VALUE(0), cs->RS_FILL_VALUE[0]);
      /*15   */ EMIT_STATE(RS_FILL_VALUE(1), cs->RS_FILL_VALUE[1]);
      /*16   */ EMIT_STATE(RS_FILL_VALUE(2), cs->RS_FILL_VALUE[2]);
      /*17   */ EMIT_STATE(RS_FILL_VALUE(3), cs->RS_FILL_VALUE[3]);
      /*18/19*/ EMIT_STATE(RS_EXTRA_CONFIG, cs->RS_EXTRA_CONFIG);
      /*20/21*/ EMIT_STATE(RS_KICKER, 0xbeebbeeb);
      etna_coalesce_end(stream, &coalesce);
   }
}

/* Generate clear command for a surface (non-fast clear case) */
static void
etna_rs_gen_clear_surface(struct etna_context *ctx, struct etna_surface *surf,
                          uint64_t clear_value)
{
   ASSERTED struct etna_screen *screen = ctx->screen;
   struct etna_resource *dst = etna_resource(surf->base.texture);
   uint32_t format;

   switch (util_format_get_blocksizebits(surf->base.format)) {
   case 16:
      format = RS_FORMAT_A4R4G4B4;
      break;
   case 32:
      format = RS_FORMAT_A8R8G8B8;
      break;
   case 64:
      assert(screen->specs.halti >= 2);
      format = RS_FORMAT_64BPP_CLEAR;
      break;
   default:
      unreachable("bpp not supported for clear by RS");
      break;
   }

   /* use tiled clear if width is multiple of 16 */
   bool tiled_clear = (surf->level->padded_width & ETNA_RS_WIDTH_MASK) == 0 &&
                      (surf->level->padded_height & ETNA_RS_HEIGHT_MASK) == 0;

   etna_compile_rs_state( ctx, &surf->clear_command, &(struct rs_state) {
      .source_format = format,
      .dest_format = format,
      .dest = dst->bo,
      .dest_offset = surf->offset,
      .dest_stride = surf->level->stride,
      .dest_padded_height = surf->level->padded_height,
      .dest_tiling = tiled_clear ? dst->layout : ETNA_LAYOUT_LINEAR,
      .dither = {0xffffffff, 0xffffffff},
      .width = surf->level->padded_width, /* These must be padded to 16x4 if !LINEAR, otherwise RS will hang */
      .height = surf->level->padded_height,
      .clear_value = {clear_value, clear_value >> 32, clear_value, clear_value >> 32},
      .clear_mode = VIVS_RS_CLEAR_CONTROL_MODE_ENABLED1,
      .clear_bits = 0xffff
   });
}

static void
etna_blit_clear_color_rs(struct pipe_context *pctx, struct pipe_surface *dst,
                      const union pipe_color_union *color)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_surface *surf = etna_surface(dst);
   uint64_t new_clear_value = etna_clear_blit_pack_rgba(surf->base.format, color);

   if (!surf->clear_command.valid)
      etna_rs_gen_clear_surface(ctx, surf, surf->level->clear_value);

   if (surf->level->ts_size) { /* TS: use precompiled clear command */
      ctx->framebuffer.TS_COLOR_CLEAR_VALUE = new_clear_value;
      ctx->framebuffer.TS_COLOR_CLEAR_VALUE_EXT = new_clear_value >> 32;

      if (VIV_FEATURE(ctx->screen, chipMinorFeatures1, AUTO_DISABLE)) {
         /* Set number of color tiles to be filled */
         etna_set_state(ctx->stream, VIVS_TS_COLOR_AUTO_DISABLE_COUNT,
                        surf->level->padded_width * surf->level->padded_height / 16);
         ctx->framebuffer.TS_MEM_CONFIG |= VIVS_TS_MEM_CONFIG_COLOR_AUTO_DISABLE;
      }

      /* update clear color in SW meta area of the buffer if TS is exported */
      if (unlikely(new_clear_value != surf->level->clear_value &&
          etna_resource_ext_ts(etna_resource(dst->texture))))
         surf->level->ts_meta->v0.clear_value = new_clear_value;

      etna_resource_level_ts_mark_valid(surf->level);
      ctx->dirty |= ETNA_DIRTY_TS | ETNA_DIRTY_DERIVE_TS;
   } else if (unlikely(new_clear_value != surf->level->clear_value)) { /* Queue normal RS clear for non-TS surfaces */
      /* If clear color changed, re-generate stored command */
      etna_rs_gen_clear_surface(ctx, surf, new_clear_value);
   }

   etna_submit_rs_state(ctx, &surf->clear_command);

   surf->level->clear_value = new_clear_value;
   resource_written(ctx, surf->base.texture);
   etna_resource_level_mark_changed(surf->level);
}

static void
etna_blit_clear_zs_rs(struct pipe_context *pctx, struct pipe_surface *dst,
                   unsigned buffers, double depth, unsigned stencil)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_surface *surf = etna_surface(dst);
   uint32_t new_clear_value = translate_clear_depth_stencil(surf->base.format, depth, stencil);
   uint32_t new_clear_bits = 0, clear_bits_depth, clear_bits_stencil;

   if (!surf->clear_command.valid)
      etna_rs_gen_clear_surface(ctx, surf, surf->level->clear_value);

   /* Get the channels to clear */
   switch (surf->base.format) {
   case PIPE_FORMAT_Z16_UNORM:
      clear_bits_depth = 0xffff;
      clear_bits_stencil = 0;
      break;
   case PIPE_FORMAT_X8Z24_UNORM:
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      clear_bits_depth = 0xeeee;
      clear_bits_stencil = 0x1111;
      break;
   default:
      clear_bits_depth = clear_bits_stencil = 0xffff;
      break;
   }

   if (buffers & PIPE_CLEAR_DEPTH)
      new_clear_bits |= clear_bits_depth;
   if (buffers & PIPE_CLEAR_STENCIL)
      new_clear_bits |= clear_bits_stencil;
   /* FIXME: when tile status is enabled, this becomes more complex as
    * we may separately clear the depth from the stencil.  In this case,
    * we want to resolve the surface, and avoid using the tile status.
    * We may be better off recording the pending clear operation,
    * delaying the actual clear to the first use.  This way, we can merge
    * consecutive clears together. */
   if (surf->level->ts_size) { /* TS: use precompiled clear command */
      /* Set new clear depth value */
      ctx->framebuffer.TS_DEPTH_CLEAR_VALUE = new_clear_value;
      if (VIV_FEATURE(ctx->screen, chipMinorFeatures1, AUTO_DISABLE)) {
         /* Set number of depth tiles to be filled */
         etna_set_state(ctx->stream, VIVS_TS_DEPTH_AUTO_DISABLE_COUNT,
                        surf->level->padded_width * surf->level->padded_height / 16);
         ctx->framebuffer.TS_MEM_CONFIG |= VIVS_TS_MEM_CONFIG_DEPTH_AUTO_DISABLE;
      }

      etna_resource_level_ts_mark_valid(surf->level);
      ctx->dirty |= ETNA_DIRTY_TS | ETNA_DIRTY_DERIVE_TS;
   } else {
      if (unlikely(new_clear_value != surf->level->clear_value)) { /* Queue normal RS clear for non-TS surfaces */
         /* If clear depth value changed, re-generate stored command */
         etna_rs_gen_clear_surface(ctx, surf, new_clear_value);
      }
      /* Update the channels to be cleared */
      etna_modify_rs_clearbits(&surf->clear_command, new_clear_bits);
   }

   etna_submit_rs_state(ctx, &surf->clear_command);

   surf->level->clear_value = new_clear_value;
   resource_written(ctx, surf->base.texture);
   etna_resource_level_mark_changed(surf->level);
}

static void
etna_clear_rs(struct pipe_context *pctx, unsigned buffers, const struct pipe_scissor_state *scissor_state,
           const union pipe_color_union *color, double depth, unsigned stencil)
{
   struct etna_context *ctx = etna_context(pctx);

   if (!etna_render_condition_check(pctx))
      return;

   /* Flush color and depth cache before clearing anything.
    * This is especially important when coming from another surface, as
    * otherwise it may clear part of the old surface instead. */
   etna_set_state(ctx->stream, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);
   etna_stall(ctx->stream, SYNC_RECIPIENT_RA, SYNC_RECIPIENT_PE);

   /* Preparation: Flush the TS if needed. This must be done after flushing
    * color and depth, otherwise it can result in crashes */
   bool need_ts_flush = false;
   if ((buffers & PIPE_CLEAR_COLOR) && ctx->framebuffer_s.nr_cbufs) {
      struct etna_surface *surf = etna_surface(ctx->framebuffer_s.cbufs[0]);

      if (surf->level->ts_size)
         need_ts_flush = true;
   }
   if ((buffers & PIPE_CLEAR_DEPTHSTENCIL) && ctx->framebuffer_s.zsbuf != NULL) {
      struct etna_surface *surf = etna_surface(ctx->framebuffer_s.zsbuf);

      if (surf->level->ts_size)
         need_ts_flush = true;
   }

   if (need_ts_flush)
      etna_set_state(ctx->stream, VIVS_TS_FLUSH_CACHE, VIVS_TS_FLUSH_CACHE_FLUSH);

   /* No need to set up the TS here as RS clear operations (in contrast to
    * resolve and copy) do not require the TS state.
    */
   if (buffers & PIPE_CLEAR_COLOR) {
      for (int idx = 0; idx < ctx->framebuffer_s.nr_cbufs; ++idx) {
         struct etna_surface *surf = etna_surface(ctx->framebuffer_s.cbufs[idx]);

         etna_blit_clear_color_rs(pctx, ctx->framebuffer_s.cbufs[idx],
                               &color[idx]);

         if (!etna_resource(surf->prsc)->explicit_flush)
            etna_context_add_flush_resource(ctx, surf->prsc);
      }
   }

   /* Flush the color and depth caches before each RS clear operation
    * This fixes a hang on GC600. */
   if (buffers & PIPE_CLEAR_DEPTHSTENCIL && buffers & PIPE_CLEAR_COLOR)
      etna_set_state(ctx->stream, VIVS_GL_FLUSH_CACHE,
                     VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);

   if ((buffers & PIPE_CLEAR_DEPTHSTENCIL) && ctx->framebuffer_s.zsbuf != NULL)
      etna_blit_clear_zs_rs(pctx, ctx->framebuffer_s.zsbuf, buffers, depth, stencil);

   etna_stall(ctx->stream, SYNC_RECIPIENT_RA, SYNC_RECIPIENT_PE);
}

static bool
etna_manual_blit(struct etna_resource *dst, struct etna_resource_level *dst_lev,
                 unsigned int dst_offset, struct etna_resource *src,
                 struct etna_resource_level *src_lev, unsigned int src_offset,
                 const struct pipe_blit_info *blit_info)
{
   void *smap, *srow, *dmap, *drow;
   size_t tile_size;

   assert(src->layout == ETNA_LAYOUT_TILED);
   assert(dst->layout == ETNA_LAYOUT_TILED);
   assert(src->base.nr_samples == 0);
   assert(dst->base.nr_samples == 0);

   tile_size = util_format_get_blocksize(blit_info->src.format) * 4 * 4;

   smap = etna_bo_map(src->bo);
   if (!smap)
      return false;

   dmap = etna_bo_map(dst->bo);
   if (!dmap)
      return false;

   srow = smap + src_offset;
   drow = dmap + dst_offset;

   etna_bo_cpu_prep(src->bo, DRM_ETNA_PREP_READ);
   etna_bo_cpu_prep(dst->bo, DRM_ETNA_PREP_WRITE);

   for (int y = 0; y < blit_info->src.box.height; y += 4) {
      memcpy(drow, srow, tile_size * blit_info->src.box.width);
      srow += src_lev->stride * 4;
      drow += dst_lev->stride * 4;
   }

   etna_bo_cpu_fini(dst->bo);
   etna_bo_cpu_fini(src->bo);

   return true;
}

static inline size_t
etna_compute_tileoffset(const struct pipe_box *box, enum pipe_format format,
                        size_t stride, enum etna_surface_layout layout)
{
   size_t offset;
   unsigned int x = box->x, y = box->y;
   unsigned int blocksize = util_format_get_blocksize(format);

   switch (layout) {
   case ETNA_LAYOUT_LINEAR:
      offset = y * stride + x * blocksize;
      break;
   case ETNA_LAYOUT_MULTI_TILED:
      y >>= 1;
      FALLTHROUGH;
   case ETNA_LAYOUT_TILED:
      assert(!(x & 0x03) && !(y & 0x03));
      offset = (y & ~0x03) * stride + blocksize * ((x & ~0x03) << 2);
      break;
   case ETNA_LAYOUT_MULTI_SUPERTILED:
      y >>= 1;
      FALLTHROUGH;
   case ETNA_LAYOUT_SUPER_TILED:
      assert(!(x & 0x3f) && !(y & 0x3f));
      offset = (y & ~0x3f) * stride + blocksize * ((x & ~0x3f) << 6);
      break;
   default:
      unreachable("invalid resource layout");
   }

   return offset;
}

static inline void
etna_get_rs_alignment_mask(const struct etna_context *ctx,
                           const enum etna_surface_layout layout,
                           unsigned int *width_mask, unsigned int *height_mask)
{
   struct etna_screen *screen = ctx->screen;
   unsigned int h_align, w_align;

   if (layout & ETNA_LAYOUT_BIT_SUPER) {
      w_align = 64;
      h_align = 64 * screen->specs.pixel_pipes;
   } else {
      w_align = ETNA_RS_WIDTH_MASK + 1;
      h_align = ETNA_RS_HEIGHT_MASK + 1;
   }

   *width_mask = w_align - 1;
   *height_mask = h_align -1;
}

static bool
etna_try_rs_blit(struct pipe_context *pctx,
                 const struct pipe_blit_info *blit_info)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_resource *src = etna_resource(blit_info->src.resource);
   struct etna_resource *dst = etna_resource(blit_info->dst.resource);
   struct compiled_rs_state copy_to_screen;
   int src_xscale, src_yscale, dst_xscale, dst_yscale;
   bool downsample_x = false, downsample_y = false;

   /* Ensure that the level is valid */
   assert(blit_info->src.level <= src->base.last_level);
   assert(blit_info->dst.level <= dst->base.last_level);

   if (!translate_samples_to_xyscale(src->base.nr_samples, &src_xscale, &src_yscale))
      return false;
   if (!translate_samples_to_xyscale(dst->base.nr_samples, &dst_xscale, &dst_yscale))
      return false;

   /* RS does not support upscaling */
   if ((src_xscale < dst_xscale) || (src_yscale < dst_yscale))
      return false;

   if (src_xscale > dst_xscale)
      downsample_x = true;
   if (src_yscale > dst_yscale)
      downsample_y = true;

   /* The width/height are in pixels; they do not change as a result of
    * multi-sampling. So, when blitting from a 4x multisampled surface
    * to a non-multisampled surface, the width and height will be
    * identical. As we do not support scaling, reject different sizes. */
   if (blit_info->dst.box.width != blit_info->src.box.width ||
       blit_info->dst.box.height != blit_info->src.box.height) {
      DBG("scaling requested: source %dx%d destination %dx%d",
          blit_info->src.box.width, blit_info->src.box.height,
          blit_info->dst.box.width, blit_info->dst.box.height);
      return false;
   }

   /* No masks - RS can't copy specific channels */
   unsigned mask = util_format_get_mask(blit_info->dst.format);
   if ((blit_info->mask & mask) != mask) {
      DBG("sub-mask requested: 0x%02x vs format mask 0x%02x", blit_info->mask, mask);
      return false;
   }

   /* Only support same format (used tiling/detiling) blits for now.
    * TODO: figure out which different-format blits are possible and test them
    *  - fail if swizzle needed
    *  - avoid trying to convert between float/int formats?
    */
   if (blit_info->src.format != blit_info->dst.format)
      return false;

   /* try to find a exact format match first */
   uint32_t format = translate_rs_format(blit_info->dst.format);
   /* When not resolving MSAA, but only doing a layout conversion, we can get
    * away with a fallback format of matching size.
    */
   if (format == ETNA_NO_MATCH && !downsample_x && !downsample_y)
      format = etna_compatible_rs_format(blit_info->dst.format);
   if (format == ETNA_NO_MATCH)
      return false;

   if (blit_info->scissor_enable ||
       blit_info->dst.box.depth != blit_info->src.box.depth ||
       blit_info->dst.box.depth != 1) {
      return false;
   }

   unsigned w_mask, h_mask;

   etna_get_rs_alignment_mask(ctx, src->layout, &w_mask, &h_mask);
   if ((blit_info->src.box.x & w_mask) || (blit_info->src.box.y & h_mask))
      return false;

   etna_get_rs_alignment_mask(ctx, dst->layout, &w_mask, &h_mask);
   if ((blit_info->dst.box.x & w_mask) || (blit_info->dst.box.y & h_mask))
      return false;

   struct etna_resource_level *src_lev = &src->levels[blit_info->src.level];
   struct etna_resource_level *dst_lev = &dst->levels[blit_info->dst.level];

   /* we may be given coordinates up to the padded width to avoid
    * any alignment issues with different tiling formats */
   assert((blit_info->src.box.x + blit_info->src.box.width) * src_xscale <= src_lev->padded_width);
   assert((blit_info->src.box.y + blit_info->src.box.height) * src_yscale <= src_lev->padded_height);
   assert(blit_info->dst.box.x + blit_info->dst.box.width <= dst_lev->padded_width);
   assert(blit_info->dst.box.y + blit_info->dst.box.height <= dst_lev->padded_height);

   unsigned src_offset = src_lev->offset +
                         blit_info->src.box.z * src_lev->layer_stride +
                         etna_compute_tileoffset(&blit_info->src.box,
                                                 blit_info->src.format,
                                                 src_lev->stride,
                                                 src->layout);
   unsigned dst_offset = dst_lev->offset +
                         blit_info->dst.box.z * dst_lev->layer_stride +
                         etna_compute_tileoffset(&blit_info->dst.box,
                                                 blit_info->dst.format,
                                                 dst_lev->stride,
                                                 dst->layout);

   if (src_lev->padded_width <= ETNA_RS_WIDTH_MASK ||
       dst_lev->padded_width <= ETNA_RS_WIDTH_MASK ||
       src_lev->padded_height <= ETNA_RS_HEIGHT_MASK ||
       dst_lev->padded_height <= ETNA_RS_HEIGHT_MASK)
      goto manual;

   /* If the width is not aligned to the RS width, but is within our
    * padding, adjust the width to suite the RS width restriction.
    * Note: the RS width/height are converted to source samples here. */
   unsigned int width = blit_info->src.box.width * src_xscale;
   unsigned int height = blit_info->src.box.height * src_yscale;
   unsigned int w_align = (ETNA_RS_WIDTH_MASK + 1) * src_xscale;
   unsigned int h_align = (ETNA_RS_HEIGHT_MASK + 1) * src_yscale;

   if (width & (w_align - 1) && width >= src_lev->width * src_xscale && width >= dst_lev->width)
      width = align(width, w_align);

   if (height & (h_align - 1) && height >= src_lev->height * src_yscale && height >= dst_lev->height) {
      if (!ctx->screen->specs.single_buffer &&
          align(height, h_align * ctx->screen->specs.pixel_pipes) <=
          dst_lev->padded_height * src_yscale)
         height = align(height, h_align * ctx->screen->specs.pixel_pipes);
      else
         height = align(height, h_align);
   }

   /* The padded dimensions are in samples */
   if (width > src_lev->padded_width ||
       width > dst_lev->padded_width * src_xscale ||
       height > src_lev->padded_height ||
       height > dst_lev->padded_height * src_yscale ||
       width & (w_align - 1) || height & (h_align - 1))
      goto manual;

   /* Flush destination, as the blit will invalidate any pending TS changes. */
   if (dst != src && etna_resource_level_needs_flush(dst_lev))
      etna_copy_resource(pctx, &dst->base, &dst->base,
                         blit_info->dst.level, blit_info->dst.level);

   /* Always flush color and depth cache together before resolving. This makes
    * sure that all previous cache content written by the PE is flushed out
    * before RS uses the pixel pipes, which invalidates those caches. */
   etna_set_state(ctx->stream, VIVS_GL_FLUSH_CACHE,
                  VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);
   etna_stall(ctx->stream, SYNC_RECIPIENT_RA, SYNC_RECIPIENT_PE);

   /* Set up color TS to source surface before blit, if needed */
   bool source_ts_valid = false;
   if (etna_resource_level_ts_valid(src_lev)) {
      struct etna_reloc reloc;
      unsigned ts_offset =
         src_lev->ts_offset + blit_info->src.box.z * src_lev->ts_layer_stride;
      uint32_t ts_mem_config = 0;

      /* flush TS cache before changing to another TS configuration */
      etna_set_state(ctx->stream, VIVS_TS_FLUSH_CACHE, VIVS_TS_FLUSH_CACHE_FLUSH);

      if (src_lev->ts_compress_fmt >= 0) {
         ts_mem_config |= VIVS_TS_MEM_CONFIG_COLOR_COMPRESSION |
                          VIVS_TS_MEM_CONFIG_COLOR_COMPRESSION_FORMAT(src_lev->ts_compress_fmt);
      }

      etna_set_state(ctx->stream, VIVS_TS_MEM_CONFIG,
                     VIVS_TS_MEM_CONFIG_COLOR_FAST_CLEAR | ts_mem_config);

      memset(&reloc, 0, sizeof(struct etna_reloc));
      reloc.bo = src->ts_bo;
      reloc.offset = ts_offset;
      reloc.flags = ETNA_RELOC_READ;
      etna_set_state_reloc(ctx->stream, VIVS_TS_COLOR_STATUS_BASE, &reloc);

      memset(&reloc, 0, sizeof(struct etna_reloc));
      reloc.bo = src->bo;
      reloc.offset = src_lev->offset +
                     blit_info->src.box.z * src_lev->layer_stride;
      reloc.flags = ETNA_RELOC_READ;
      etna_set_state_reloc(ctx->stream, VIVS_TS_COLOR_SURFACE_BASE, &reloc);

      etna_set_state(ctx->stream, VIVS_TS_COLOR_CLEAR_VALUE, src_lev->clear_value);
      etna_set_state(ctx->stream, VIVS_TS_COLOR_CLEAR_VALUE_EXT, src_lev->clear_value >> 32);

      source_ts_valid = true;
   } else {
      etna_set_state(ctx->stream, VIVS_TS_MEM_CONFIG, 0);
   }
   ctx->dirty |= ETNA_DIRTY_TS;

   /* Kick off RS here */
   etna_compile_rs_state(ctx, &copy_to_screen, &(struct rs_state) {
      .source_format = format,
      .source_tiling = src->layout,
      .source = src->bo,
      .source_offset = src_offset,
      .source_stride = src_lev->stride,
      .source_padded_width = src_lev->padded_width,
      .source_padded_height = src_lev->padded_height,
      .source_ts_valid = source_ts_valid,
      .source_ts_mode = src_lev->ts_mode,
      .source_ts_compressed = src_lev->ts_compress_fmt >= 0,
      .dest_format = format,
      .dest_tiling = dst->layout,
      .dest = dst->bo,
      .dest_offset = dst_offset,
      .dest_stride = dst_lev->stride,
      .dest_padded_height = dst_lev->padded_height,
      .downsample_x = downsample_x,
      .downsample_y = downsample_y,
      .swap_rb = translate_rb_src_dst_swap(src->base.format, dst->base.format),
      .dither = {0xffffffff, 0xffffffff}, // XXX dither when going from 24 to 16 bit?
      .clear_mode = VIVS_RS_CLEAR_CONTROL_MODE_DISABLED,
      .width = width,
      .height = height,
      .tile_count = src_lev->layer_stride /
                    etna_screen_get_tile_size(ctx->screen, src_lev->ts_mode,
                                              src->base.nr_samples > 1),
   });

   etna_submit_rs_state(ctx, &copy_to_screen);
   resource_read(ctx, &src->base);
   resource_written(ctx, &dst->base);
   etna_resource_level_mark_changed(dst_lev);

   /* We don't need to mark the TS as invalid if this was just a flush without
    * compression, as in that case only clear tiles are filled and the tile
    * status still matches the blit target buffer. For compressed formats the
    * tiles are decompressed, so tile status doesn't match anymore.
    */
   if (src != dst || src_lev->ts_compress_fmt >= 0)
      etna_resource_level_ts_mark_invalid(dst_lev);

   ctx->dirty |= ETNA_DIRTY_DERIVE_TS;

   return true;

manual:
   if (src->layout == ETNA_LAYOUT_TILED && dst->layout == ETNA_LAYOUT_TILED) {
      if ((etna_resource_status(ctx, src) & ETNA_PENDING_WRITE) ||
          (etna_resource_status(ctx, dst) & ETNA_PENDING_WRITE))
         etna_flush(pctx, NULL, 0, true);

      perf_debug_ctx(ctx, "RS blit falls back to sw");

      return etna_manual_blit(dst, dst_lev, dst_offset, src, src_lev, src_offset, blit_info);
   }

   return false;
}

void
etna_clear_blit_rs_init(struct pipe_context *pctx)
{
   struct etna_context *ctx = etna_context(pctx);

   DBG("etnaviv: Using RS blit engine");
   pctx->clear = etna_clear_rs;
   ctx->blit = etna_try_rs_blit;
}
