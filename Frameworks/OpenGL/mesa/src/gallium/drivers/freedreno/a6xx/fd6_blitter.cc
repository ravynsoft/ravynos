/*
 * Copyright (C) 2017 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#define FD_BO_NO_HARDPIN 1

#include "util/format_srgb.h"
#include "util/half_float.h"
#include "util/u_dump.h"
#include "util/u_log.h"
#include "util/u_surface.h"

#include "freedreno_blitter.h"
#include "freedreno_fence.h"
#include "freedreno_resource.h"
#include "freedreno_tracepoints.h"

#include "fd6_barrier.h"
#include "fd6_blitter.h"
#include "fd6_emit.h"
#include "fd6_pack.h"
#include "fd6_resource.h"

static inline enum a6xx_2d_ifmt
fd6_ifmt(enum a6xx_format fmt)
{
   switch (fmt) {
   case FMT6_A8_UNORM:
   case FMT6_8_UNORM:
   case FMT6_8_SNORM:
   case FMT6_8_8_UNORM:
   case FMT6_8_8_SNORM:
   case FMT6_8_8_8_8_UNORM:
   case FMT6_8_8_8_X8_UNORM:
   case FMT6_8_8_8_8_SNORM:
   case FMT6_4_4_4_4_UNORM:
   case FMT6_5_5_5_1_UNORM:
   case FMT6_5_6_5_UNORM:
      return R2D_UNORM8;

   case FMT6_32_UINT:
   case FMT6_32_SINT:
   case FMT6_32_32_UINT:
   case FMT6_32_32_SINT:
   case FMT6_32_32_32_32_UINT:
   case FMT6_32_32_32_32_SINT:
      return R2D_INT32;

   case FMT6_16_UINT:
   case FMT6_16_SINT:
   case FMT6_16_16_UINT:
   case FMT6_16_16_SINT:
   case FMT6_16_16_16_16_UINT:
   case FMT6_16_16_16_16_SINT:
   case FMT6_10_10_10_2_UINT:
      return R2D_INT16;

   case FMT6_8_UINT:
   case FMT6_8_SINT:
   case FMT6_8_8_UINT:
   case FMT6_8_8_SINT:
   case FMT6_8_8_8_8_UINT:
   case FMT6_8_8_8_8_SINT:
   case FMT6_Z24_UNORM_S8_UINT:
   case FMT6_Z24_UNORM_S8_UINT_AS_R8G8B8A8:
      return R2D_INT8;

   case FMT6_16_UNORM:
   case FMT6_16_SNORM:
   case FMT6_16_16_UNORM:
   case FMT6_16_16_SNORM:
   case FMT6_16_16_16_16_UNORM:
   case FMT6_16_16_16_16_SNORM:
   case FMT6_32_FLOAT:
   case FMT6_32_32_FLOAT:
   case FMT6_32_32_32_32_FLOAT:
      return R2D_FLOAT32;

   case FMT6_16_FLOAT:
   case FMT6_16_16_FLOAT:
   case FMT6_16_16_16_16_FLOAT:
   case FMT6_11_11_10_FLOAT:
   case FMT6_10_10_10_2_UNORM_DEST:
      return R2D_FLOAT16;

   default:
      unreachable("bad format");
      return (enum a6xx_2d_ifmt)0;
   }
}

/* Make sure none of the requested dimensions extend beyond the size of the
 * resource.  Not entirely sure why this happens, but sometimes it does, and
 * w/ 2d blt doesn't have wrap modes like a sampler, so force those cases
 * back to u_blitter
 */
static bool
ok_dims(const struct pipe_resource *r, const struct pipe_box *b, int lvl)
{
   int last_layer =
      r->target == PIPE_TEXTURE_3D ? u_minify(r->depth0, lvl) : r->array_size;

   return (b->x >= 0) && (b->x + b->width <= u_minify(r->width0, lvl)) &&
          (b->y >= 0) && (b->y + b->height <= u_minify(r->height0, lvl)) &&
          (b->z >= 0) && (b->z + b->depth <= last_layer);
}

static bool
ok_format(enum pipe_format pfmt)
{
   enum a6xx_format fmt = fd6_color_format(pfmt, TILE6_LINEAR);

   if (util_format_is_compressed(pfmt))
      return true;

   switch (pfmt) {
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z16_UNORM:
   case PIPE_FORMAT_Z32_UNORM:
   case PIPE_FORMAT_Z32_FLOAT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
   case PIPE_FORMAT_S8_UINT:
      return true;
   default:
      break;
   }

   if (fmt == FMT6_NONE)
      return false;

   return true;
}

#define DEBUG_BLIT          0
#define DEBUG_BLIT_FALLBACK 0

#define fail_if(cond)                                                          \
   do {                                                                        \
      if (cond) {                                                              \
         if (DEBUG_BLIT_FALLBACK) {                                            \
            fprintf(stderr, "falling back: %s for blit:\n", #cond);            \
            dump_blit_info(info);                                              \
         }                                                                     \
         return false;                                                         \
      }                                                                        \
   } while (0)

static bool
is_ubwc(struct pipe_resource *prsc, unsigned level)
{
   return fd_resource_ubwc_enabled(fd_resource(prsc), level);
}

static void
dump_blit_info(const struct pipe_blit_info *info)
{
   util_dump_blit_info(stderr, info);
   fprintf(stderr, "\ndst resource: ");
   util_dump_resource(stderr, info->dst.resource);
   if (is_ubwc(info->dst.resource, info->dst.level))
      fprintf(stderr, " (ubwc)");
   fprintf(stderr, "\nsrc resource: ");
   util_dump_resource(stderr, info->src.resource);
   if (is_ubwc(info->src.resource, info->src.level))
      fprintf(stderr, " (ubwc)");
   fprintf(stderr, "\n");
}

static bool
can_do_blit(const struct pipe_blit_info *info)
{
   /* I think we can do scaling, but not in z dimension since that would
    * require blending..
    */
   fail_if(info->dst.box.depth != info->src.box.depth);

   /* Fail if unsupported format: */
   fail_if(!ok_format(info->src.format));
   fail_if(!ok_format(info->dst.format));

   assert(!util_format_is_compressed(info->src.format));
   assert(!util_format_is_compressed(info->dst.format));

   fail_if(!ok_dims(info->src.resource, &info->src.box, info->src.level));

   fail_if(!ok_dims(info->dst.resource, &info->dst.box, info->dst.level));

   assert(info->dst.box.width >= 0);
   assert(info->dst.box.height >= 0);
   assert(info->dst.box.depth >= 0);

   fail_if(info->dst.resource->nr_samples > 1);

   fail_if(info->window_rectangle_include);

   const struct util_format_description *src_desc =
      util_format_description(info->src.format);
   const struct util_format_description *dst_desc =
      util_format_description(info->dst.format);
   const int common_channels =
      MIN2(src_desc->nr_channels, dst_desc->nr_channels);

   if (info->mask & PIPE_MASK_RGBA) {
      for (int i = 0; i < common_channels; i++) {
         fail_if(memcmp(&src_desc->channel[i], &dst_desc->channel[i],
                        sizeof(src_desc->channel[0])));
      }
   }

   fail_if(info->alpha_blend);

   return true;
}

static bool
can_do_clear(const struct pipe_resource *prsc, unsigned level,
             const struct pipe_box *box)
{
   return ok_format(prsc->format) &&
          ok_dims(prsc, box, level) &&
          (fd_resource_nr_samples(prsc) == 1);

   return true;
}

static void
emit_setup(struct fd_batch *batch)
{
   struct fd_ringbuffer *ring = batch->draw;
   struct fd_screen *screen = batch->ctx->screen;

   fd6_emit_flushes(batch->ctx, ring,
                    FD6_FLUSH_CCU_COLOR |
                    FD6_INVALIDATE_CCU_COLOR |
                    FD6_FLUSH_CCU_DEPTH |
                    FD6_INVALIDATE_CCU_DEPTH);

   /* normal BLIT_OP_SCALE operation needs bypass RB_CCU_CNTL */
   OUT_WFI5(ring);
   fd6_emit_ccu_cntl(ring, screen, false);
}

template <chip CHIP>
static void
emit_blit_setup(struct fd_ringbuffer *ring, enum pipe_format pfmt,
                bool scissor_enable, union pipe_color_union *color,
                uint32_t unknown_8c01, enum a6xx_rotation rotate)
{
   enum a6xx_format fmt = fd6_color_format(pfmt, TILE6_LINEAR);
   bool is_srgb = util_format_is_srgb(pfmt);
   enum a6xx_2d_ifmt ifmt = fd6_ifmt(fmt);

   if (is_srgb) {
      assert(ifmt == R2D_UNORM8);
      ifmt = R2D_UNORM8_SRGB;
   }

   uint32_t blit_cntl = A6XX_RB_2D_BLIT_CNTL_MASK(0xf) |
                        A6XX_RB_2D_BLIT_CNTL_COLOR_FORMAT(fmt) |
                        A6XX_RB_2D_BLIT_CNTL_IFMT(ifmt) |
                        A6XX_RB_2D_BLIT_CNTL_ROTATE(rotate) |
                        COND(color, A6XX_RB_2D_BLIT_CNTL_SOLID_COLOR) |
                        COND(scissor_enable, A6XX_RB_2D_BLIT_CNTL_SCISSOR);

   OUT_PKT4(ring, REG_A6XX_RB_2D_BLIT_CNTL, 1);
   OUT_RING(ring, blit_cntl);

   OUT_PKT4(ring, REG_A6XX_GRAS_2D_BLIT_CNTL, 1);
   OUT_RING(ring, blit_cntl);

   if (fmt == FMT6_10_10_10_2_UNORM_DEST)
      fmt = FMT6_16_16_16_16_FLOAT;

   /* This register is probably badly named... it seems that it's
    * controlling the internal/accumulator format or something like
    * that. It's certainly not tied to only the src format.
    */
   OUT_REG(ring, SP_2D_DST_FORMAT(
         CHIP,
         .sint = util_format_is_pure_sint(pfmt),
         .uint = util_format_is_pure_uint(pfmt),
         .color_format = fmt,
         .srgb = is_srgb,
         .mask = 0xf,
   ));

   OUT_PKT4(ring, REG_A6XX_RB_2D_UNKNOWN_8C01, 1);
   OUT_RING(ring, unknown_8c01);
}

static void
emit_blit_buffer_dst(struct fd_ringbuffer *ring, struct fd_resource *dst,
                     unsigned off, unsigned size)
{
   OUT_REG(ring,
           A6XX_RB_2D_DST_INFO(
                 .color_format = FMT6_8_UNORM,
                 .tile_mode = TILE6_LINEAR,
                 .color_swap = WZYX,
           ),
           A6XX_RB_2D_DST(
                 .bo = dst->bo,
                 .bo_offset = off,
           ),
           A6XX_RB_2D_DST_PITCH(size),
   );
}

/* buffers need to be handled specially since x/width can exceed the bounds
 * supported by hw.. if necessary decompose into (potentially) two 2D blits
 */
template <chip CHIP>
static void
emit_blit_buffer(struct fd_context *ctx, struct fd_ringbuffer *ring,
                 const struct pipe_blit_info *info)
{
   const struct pipe_box *sbox = &info->src.box;
   const struct pipe_box *dbox = &info->dst.box;
   struct fd_resource *src, *dst;
   unsigned sshift, dshift;

   if (DEBUG_BLIT) {
      fprintf(stderr, "buffer blit: ");
      dump_blit_info(info);
   }

   src = fd_resource(info->src.resource);
   dst = fd_resource(info->dst.resource);

   assert(src->layout.cpp == 1);
   assert(dst->layout.cpp == 1);
   assert(info->src.resource->format == info->dst.resource->format);
   assert((sbox->y == 0) && (sbox->height == 1));
   assert((dbox->y == 0) && (dbox->height == 1));
   assert((sbox->z == 0) && (sbox->depth == 1));
   assert((dbox->z == 0) && (dbox->depth == 1));
   assert(sbox->width == dbox->width);
   assert(info->src.level == 0);
   assert(info->dst.level == 0);

   /*
    * Buffers can have dimensions bigger than max width, remap into
    * multiple 1d blits to fit within max dimension
    *
    * Note that blob uses .ARRAY_PITCH=128 for blitting buffers, which
    * seems to prevent overfetch related faults.  Not quite sure what
    * the deal is there.
    *
    * Low 6 bits of SRC/DST addresses need to be zero (ie. address
    * aligned to 64) so we need to shift src/dst x1/x2 to make up the
    * difference.  On top of already splitting up the blit so width
    * isn't > 16k.
    *
    * We perhaps could do a bit better, if src and dst are aligned but
    * in the worst case this means we have to split the copy up into
    * 16k (0x4000) minus 64 (0x40).
    */

   sshift = sbox->x & 0x3f;
   dshift = dbox->x & 0x3f;

   emit_blit_setup<CHIP>(ring, PIPE_FORMAT_R8_UNORM, false, NULL, 0, ROTATE_0);

   for (unsigned off = 0; off < sbox->width; off += (0x4000 - 0x40)) {
      unsigned soff, doff, w, p;

      soff = (sbox->x + off) & ~0x3f;
      doff = (dbox->x + off) & ~0x3f;

      w = MIN2(sbox->width - off, (0x4000 - 0x40));
      p = align(w, 64);

      assert((soff + w) <= fd_bo_size(src->bo));
      assert((doff + w) <= fd_bo_size(dst->bo));

      /*
       * Emit source:
       */
      OUT_REG(ring,
              SP_PS_2D_SRC_INFO(
                    CHIP,
                    .color_format = FMT6_8_UNORM,
                    .tile_mode = TILE6_LINEAR,
                    .color_swap = WZYX,
                    .unk20 = true,
                    .unk22 = true,
              ),
              SP_PS_2D_SRC_SIZE(
                    CHIP,
                    .width = sshift + w,
                    .height = 1,
              ),
              SP_PS_2D_SRC(
                    CHIP,
                    .bo = src->bo,
                    .bo_offset = soff,
              ),
              SP_PS_2D_SRC_PITCH(
                    CHIP,
                    .pitch = p,
              ),
      );

      /*
       * Emit destination:
       */
      emit_blit_buffer_dst(ring, dst, doff, p);

      /*
       * Blit command:
       */
      OUT_REG(ring,
              A6XX_GRAS_2D_SRC_TL_X(sshift),
              A6XX_GRAS_2D_SRC_BR_X(sshift + w - 1),
              A6XX_GRAS_2D_SRC_TL_Y(0),
              A6XX_GRAS_2D_SRC_BR_Y(0),
      );

      OUT_PKT4(ring, REG_A6XX_GRAS_2D_DST_TL, 2);
      OUT_RING(ring, A6XX_GRAS_2D_DST_TL_X(dshift) | A6XX_GRAS_2D_DST_TL_Y(0));
      OUT_RING(ring, A6XX_GRAS_2D_DST_BR_X(dshift + w - 1) |
                        A6XX_GRAS_2D_DST_BR_Y(0));

      OUT_PKT7(ring, CP_EVENT_WRITE, 1);
      OUT_RING(ring, LABEL);
      OUT_WFI5(ring);

      OUT_PKT4(ring, REG_A6XX_RB_DBG_ECO_CNTL, 1);
      OUT_RING(ring, ctx->screen->info->a6xx.magic.RB_DBG_ECO_CNTL_blit);

      OUT_PKT7(ring, CP_BLIT, 1);
      OUT_RING(ring, CP_BLIT_0_OP(BLIT_OP_SCALE));

      OUT_WFI5(ring);

      OUT_PKT4(ring, REG_A6XX_RB_DBG_ECO_CNTL, 1);
      OUT_RING(ring, 0); /* RB_DBG_ECO_CNTL */
   }
}

template <chip CHIP>
static void
fd6_clear_ubwc(struct fd_batch *batch, struct fd_resource *rsc) assert_dt
{
   struct fd_ringbuffer *ring = fd_batch_get_prologue(batch);
   union pipe_color_union color = {};

   emit_blit_setup<CHIP>(ring, PIPE_FORMAT_R8_UNORM, false, &color, 0, ROTATE_0);

   OUT_REG(ring,
           SP_PS_2D_SRC_INFO(CHIP),
           SP_PS_2D_SRC_SIZE(CHIP),
           SP_PS_2D_SRC(CHIP),
           SP_PS_2D_SRC_PITCH(CHIP),
   );

   OUT_PKT4(ring, REG_A6XX_RB_2D_SRC_SOLID_C0, 4);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000000);

   OUT_REG(ring,
           A6XX_GRAS_2D_SRC_TL_X(0),
           A6XX_GRAS_2D_SRC_BR_X(0),
           A6XX_GRAS_2D_SRC_TL_Y(0),
           A6XX_GRAS_2D_SRC_BR_Y(0),
   );

   unsigned size = rsc->layout.slices[0].offset;
   unsigned offset = 0;

   /* We could be more clever here and realize that we could use a
    * larger width if the size is aligned to something more than a
    * single page.. or even use a format larger than r8 in those
    * cases. But for normal sized textures and even up to 16k x 16k
    * at <= 4byte/pixel, we'll only go thru the loop once
    */
   const unsigned w = 0x1000;

   /* ubwc size should always be page aligned: */
   assert((size % w) == 0);

   while (size > 0) {
      const unsigned h = MIN2(0x4000, size / w);
      /* width is already aligned to a suitable pitch: */
      const unsigned p = w;

      /*
       * Emit destination:
       */
      emit_blit_buffer_dst(ring, rsc, offset, p);

      /*
       * Blit command:
       */

      OUT_PKT4(ring, REG_A6XX_GRAS_2D_DST_TL, 2);
      OUT_RING(ring, A6XX_GRAS_2D_DST_TL_X(0) | A6XX_GRAS_2D_DST_TL_Y(0));
      OUT_RING(ring,
               A6XX_GRAS_2D_DST_BR_X(w - 1) | A6XX_GRAS_2D_DST_BR_Y(h - 1));

      OUT_PKT7(ring, CP_EVENT_WRITE, 1);
      OUT_RING(ring, LABEL);
      OUT_WFI5(ring);

      OUT_PKT4(ring, REG_A6XX_RB_DBG_ECO_CNTL, 1);
      OUT_RING(ring, batch->ctx->screen->info->a6xx.magic.RB_DBG_ECO_CNTL_blit);

      OUT_PKT7(ring, CP_BLIT, 1);
      OUT_RING(ring, CP_BLIT_0_OP(BLIT_OP_SCALE));

      OUT_WFI5(ring);

      OUT_PKT4(ring, REG_A6XX_RB_DBG_ECO_CNTL, 1);
      OUT_RING(ring, 0); /* RB_DBG_ECO_CNTL */

      offset += w * h;
      size -= w * h;
   }

   fd6_emit_flushes(batch->ctx, ring,
                    FD6_FLUSH_CCU_COLOR |
                    FD6_FLUSH_CCU_DEPTH |
                    FD6_FLUSH_CACHE |
                    FD6_WAIT_FOR_IDLE);
}

static void
emit_blit_dst(struct fd_ringbuffer *ring, struct pipe_resource *prsc,
              enum pipe_format pfmt, unsigned level, unsigned layer)
{
   struct fd_resource *dst = fd_resource(prsc);
   enum a6xx_format fmt =
         fd6_color_format(pfmt, (enum a6xx_tile_mode)dst->layout.tile_mode);
   enum a6xx_tile_mode tile =
         (enum a6xx_tile_mode)fd_resource_tile_mode(prsc, level);
   enum a3xx_color_swap swap =
         fd6_color_swap(pfmt, (enum a6xx_tile_mode)dst->layout.tile_mode);
   uint32_t pitch = fd_resource_pitch(dst, level);
   bool ubwc_enabled = fd_resource_ubwc_enabled(dst, level);
   unsigned off = fd_resource_offset(dst, level, layer);

   if (fmt == FMT6_Z24_UNORM_S8_UINT)
      fmt = FMT6_Z24_UNORM_S8_UINT_AS_R8G8B8A8;

   OUT_REG(ring,
           A6XX_RB_2D_DST_INFO(
                 .color_format = fmt,
                 .tile_mode = tile,
                 .color_swap = swap,
                 .flags = ubwc_enabled,
                 .srgb = util_format_is_srgb(pfmt),
           ),
           A6XX_RB_2D_DST(
                 .bo = dst->bo,
                 .bo_offset = off,
           ),
           A6XX_RB_2D_DST_PITCH(pitch),
   );

   if (ubwc_enabled) {
      OUT_PKT4(ring, REG_A6XX_RB_2D_DST_FLAGS, 6);
      fd6_emit_flag_reference(ring, dst, level, layer);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x00000000);
   }
}

template <chip CHIP>
static void
emit_blit_src(struct fd_ringbuffer *ring, const struct pipe_blit_info *info,
              unsigned layer, unsigned nr_samples)
{
   struct fd_resource *src = fd_resource(info->src.resource);
   enum a6xx_format sfmt =
      fd6_texture_format(info->src.format, (enum a6xx_tile_mode)src->layout.tile_mode);
   enum a6xx_tile_mode stile =
      (enum a6xx_tile_mode)fd_resource_tile_mode(info->src.resource, info->src.level);
   enum a3xx_color_swap sswap =
      fd6_texture_swap(info->src.format, (enum a6xx_tile_mode)src->layout.tile_mode);
   uint32_t pitch = fd_resource_pitch(src, info->src.level);
   bool subwc_enabled = fd_resource_ubwc_enabled(src, info->src.level);
   unsigned soff = fd_resource_offset(src, info->src.level, layer);
   uint32_t width = u_minify(src->b.b.width0, info->src.level) * nr_samples;
   uint32_t height = u_minify(src->b.b.height0, info->src.level);
   enum a3xx_msaa_samples samples = fd_msaa_samples(src->b.b.nr_samples);

   if (info->src.format == PIPE_FORMAT_A8_UNORM)
      sfmt = FMT6_A8_UNORM;

   OUT_REG(ring,
           SP_PS_2D_SRC_INFO(
                 CHIP,
                 .color_format = sfmt,
                 .tile_mode = stile,
                 .color_swap = sswap,
                 .flags = subwc_enabled,
                 .srgb  = util_format_is_srgb(info->src.format),
                 .samples = samples,
                 .filter = (info->filter == PIPE_TEX_FILTER_LINEAR),
                 .samples_average = (samples > MSAA_ONE) && !info->sample0_only,
                 .unk20 = true,
                 .unk22 = true,
           ),
           SP_PS_2D_SRC_SIZE(
                 CHIP,
                 .width = width,
                 .height = height,
           ),
           SP_PS_2D_SRC(
                 CHIP,
                 .bo = src->bo,
                 .bo_offset = soff,
           ),
           SP_PS_2D_SRC_PITCH(
                 CHIP,
                 .pitch = pitch,
           ),
   );

   if (subwc_enabled && fd_resource_ubwc_enabled(src, info->src.level)) {
      OUT_REG(ring,
              SP_PS_2D_SRC_FLAGS(
                    CHIP,
                    .bo = src->bo,
                    .bo_offset = fd_resource_ubwc_offset(src, info->src.level, layer),
              ),
              SP_PS_2D_SRC_FLAGS_PITCH(
                    CHIP, fdl_ubwc_pitch(&src->layout, info->src.level)),
      );
   }
}

template <chip CHIP>
static void
emit_blit_texture(struct fd_context *ctx, struct fd_ringbuffer *ring,
                  const struct pipe_blit_info *info)
{
   const struct pipe_box *sbox = &info->src.box;
   const struct pipe_box *dbox = &info->dst.box;
   struct fd_resource *dst;
   int sx1, sy1, sx2, sy2;
   int dx1, dy1, dx2, dy2;

   if (DEBUG_BLIT) {
      fprintf(stderr, "texture blit: ");
      dump_blit_info(info);
   }

   dst = fd_resource(info->dst.resource);

   uint32_t nr_samples = fd_resource_nr_samples(&dst->b.b);

   sx1 = sbox->x * nr_samples;
   sy1 = sbox->y;
   sx2 = (sbox->x + sbox->width) * nr_samples;
   sy2 = sbox->y + sbox->height;

   dx1 = dbox->x * nr_samples;
   dy1 = dbox->y;
   dx2 = (dbox->x + dbox->width) * nr_samples;
   dy2 = dbox->y + dbox->height;

   static const enum a6xx_rotation rotates[2][2] = {
      {ROTATE_0, ROTATE_HFLIP},
      {ROTATE_VFLIP, ROTATE_180},
   };
   bool mirror_x = (sx2 < sx1) != (dx2 < dx1);
   bool mirror_y = (sy2 < sy1) != (dy2 < dy1);

   enum a6xx_rotation rotate = rotates[mirror_y][mirror_x];

   OUT_REG(ring,
           A6XX_GRAS_2D_SRC_TL_X(MIN2(sx1, sx2)),
           A6XX_GRAS_2D_SRC_BR_X(MAX2(sx1, sx2) - 1),
           A6XX_GRAS_2D_SRC_TL_Y(MIN2(sy1, sy2)),
           A6XX_GRAS_2D_SRC_BR_Y(MAX2(sy1, sy2) - 1),
   );

   OUT_REG(ring,
           A6XX_GRAS_2D_DST_TL(.x = MIN2(dx1, dx2),
                               .y = MIN2(dy1, dy2)),
           A6XX_GRAS_2D_DST_BR(.x = MAX2(dx1, dx2) - 1,
                               .y = MAX2(dy1, dy2) - 1),
   );

   if (info->scissor_enable) {
      OUT_PKT4(ring, REG_A6XX_GRAS_2D_RESOLVE_CNTL_1, 2);
      OUT_RING(ring, A6XX_GRAS_2D_RESOLVE_CNTL_1_X(info->scissor.minx) |
                        A6XX_GRAS_2D_RESOLVE_CNTL_1_Y(info->scissor.miny));
      OUT_RING(ring, A6XX_GRAS_2D_RESOLVE_CNTL_1_X(info->scissor.maxx - 1) |
                        A6XX_GRAS_2D_RESOLVE_CNTL_1_Y(info->scissor.maxy - 1));
   }

   emit_blit_setup<CHIP>(ring, info->dst.format, info->scissor_enable, NULL, 0, rotate);

   for (unsigned i = 0; i < info->dst.box.depth; i++) {

      emit_blit_src<CHIP>(ring, info, sbox->z + i, nr_samples);
      emit_blit_dst(ring, info->dst.resource, info->dst.format, info->dst.level,
                    dbox->z + i);

      /*
       * Blit command:
       */
      OUT_PKT7(ring, CP_EVENT_WRITE, 1);
      OUT_RING(ring, LABEL);
      OUT_WFI5(ring);

      OUT_PKT4(ring, REG_A6XX_RB_DBG_ECO_CNTL, 1);
      OUT_RING(ring, ctx->screen->info->a6xx.magic.RB_DBG_ECO_CNTL_blit);

      OUT_PKT7(ring, CP_BLIT, 1);
      OUT_RING(ring, CP_BLIT_0_OP(BLIT_OP_SCALE));

      OUT_WFI5(ring);

      OUT_PKT4(ring, REG_A6XX_RB_DBG_ECO_CNTL, 1);
      OUT_RING(ring, 0); /* RB_DBG_ECO_CNTL */
   }
}

static void
emit_clear_color(struct fd_ringbuffer *ring, enum pipe_format pfmt,
                 union pipe_color_union *color)
{
   switch (pfmt) {
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_X24S8_UINT: {
      uint32_t depth_unorm24 = color->f[0] * ((1u << 24) - 1);
      uint8_t stencil = color->ui[1];
      color->ui[0] = depth_unorm24 & 0xff;
      color->ui[1] = (depth_unorm24 >> 8) & 0xff;
      color->ui[2] = (depth_unorm24 >> 16) & 0xff;
      color->ui[3] = stencil;
      break;
   }
   default:
      break;
   }

   OUT_PKT4(ring, REG_A6XX_RB_2D_SRC_SOLID_C0, 4);
   switch (fd6_ifmt(fd6_color_format(pfmt, TILE6_LINEAR))) {
   case R2D_UNORM8:
   case R2D_UNORM8_SRGB:
      /* The r2d ifmt is badly named, it also covers the signed case: */
      if (util_format_is_snorm(pfmt)) {
         OUT_RING(ring, float_to_byte_tex(color->f[0]));
         OUT_RING(ring, float_to_byte_tex(color->f[1]));
         OUT_RING(ring, float_to_byte_tex(color->f[2]));
         OUT_RING(ring, float_to_byte_tex(color->f[3]));
      } else {
         OUT_RING(ring, float_to_ubyte(color->f[0]));
         OUT_RING(ring, float_to_ubyte(color->f[1]));
         OUT_RING(ring, float_to_ubyte(color->f[2]));
         OUT_RING(ring, float_to_ubyte(color->f[3]));
      }
      break;
   case R2D_FLOAT16:
      OUT_RING(ring, _mesa_float_to_half(color->f[0]));
      OUT_RING(ring, _mesa_float_to_half(color->f[1]));
      OUT_RING(ring, _mesa_float_to_half(color->f[2]));
      OUT_RING(ring, _mesa_float_to_half(color->f[3]));
      break;
   case R2D_FLOAT32:
   case R2D_INT32:
   case R2D_INT16:
   case R2D_INT8:
   default:
      OUT_RING(ring, color->ui[0]);
      OUT_RING(ring, color->ui[1]);
      OUT_RING(ring, color->ui[2]);
      OUT_RING(ring, color->ui[3]);
      break;
   }
}

template <chip CHIP>
void
fd6_clear_lrz(struct fd_batch *batch, struct fd_resource *zsbuf,
              struct fd_bo *lrz, double depth)
{
   struct fd_ringbuffer *ring = fd_batch_get_prologue(batch);

   if (DEBUG_BLIT) {
      fprintf(stderr, "lrz clear:\ndst resource: ");
      util_dump_resource(stderr, &zsbuf->b.b);
      fprintf(stderr, "\n");
   }

   OUT_PKT4(ring, REG_A6XX_GRAS_2D_DST_TL, 2);
   OUT_RING(ring, A6XX_GRAS_2D_DST_TL_X(0) | A6XX_GRAS_2D_DST_TL_Y(0));
   OUT_RING(ring, A6XX_GRAS_2D_DST_BR_X(zsbuf->lrz_width - 1) |
                     A6XX_GRAS_2D_DST_BR_Y(zsbuf->lrz_height - 1));

   union pipe_color_union clear_color = { .f = {depth} };

   emit_clear_color(ring, PIPE_FORMAT_Z16_UNORM, &clear_color);
   emit_blit_setup<CHIP>(ring, PIPE_FORMAT_Z16_UNORM, false, &clear_color, 0, ROTATE_0);

   OUT_REG(ring,
           A6XX_RB_2D_DST_INFO(
                 .color_format = FMT6_16_UNORM,
                 .tile_mode = TILE6_LINEAR,
                 .color_swap = WZYX,
           ),
           A6XX_RB_2D_DST(
                 .bo = lrz,
           ),
           A6XX_RB_2D_DST_PITCH(zsbuf->lrz_pitch * 2),
   );

   /*
    * Blit command:
    */

   OUT_PKT7(ring, CP_BLIT, 1);
   OUT_RING(ring, CP_BLIT_0_OP(BLIT_OP_SCALE));
}

template void fd6_clear_lrz<A6XX>(struct fd_batch *batch, struct fd_resource *zsbuf, struct fd_bo *lrz, double depth);
template void fd6_clear_lrz<A7XX>(struct fd_batch *batch, struct fd_resource *zsbuf, struct fd_bo *lrz, double depth);

/**
 * Handle conversion of clear color
 */
static union pipe_color_union
convert_color(enum pipe_format format, union pipe_color_union *pcolor)
{
   union pipe_color_union color = *pcolor;

   /* For solid-fill blits, the hw isn't going to convert from
    * linear to srgb for us:
    */
   if (util_format_is_srgb(format)) {
      for (int i = 0; i < 3; i++)
         color.f[i] = util_format_linear_to_srgb_float(color.f[i]);
   }

   if (util_format_is_snorm(format)) {
      for (int i = 0; i < 3; i++)
         color.f[i] = CLAMP(color.f[i], -1.0f, 1.0f);
   }

   /* Note that float_to_ubyte() already clamps, for the unorm case */

   return color;
}

template <chip CHIP>
void
fd6_clear_surface(struct fd_context *ctx, struct fd_ringbuffer *ring,
                  struct pipe_surface *psurf, const struct pipe_box *box2d,
                  union pipe_color_union *color, uint32_t unknown_8c01)
{
   if (DEBUG_BLIT) {
      fprintf(stderr, "surface clear:\ndst resource: ");
      util_dump_resource(stderr, psurf->texture);
      fprintf(stderr, "\n");
   }

   uint32_t nr_samples = fd_resource_nr_samples(psurf->texture);
   OUT_PKT4(ring, REG_A6XX_GRAS_2D_DST_TL, 2);
   OUT_RING(ring, A6XX_GRAS_2D_DST_TL_X(box2d->x * nr_samples) |
                     A6XX_GRAS_2D_DST_TL_Y(box2d->y));
   OUT_RING(ring, A6XX_GRAS_2D_DST_BR_X((box2d->x + box2d->width) * nr_samples - 1) |
                     A6XX_GRAS_2D_DST_BR_Y(box2d->y + box2d->height - 1));

   union pipe_color_union clear_color = convert_color(psurf->format, color);

   emit_clear_color(ring, psurf->format, &clear_color);
   emit_blit_setup<CHIP>(ring, psurf->format, false, &clear_color, unknown_8c01, ROTATE_0);

   for (unsigned i = psurf->u.tex.first_layer; i <= psurf->u.tex.last_layer;
        i++) {
      emit_blit_dst(ring, psurf->texture, psurf->format, psurf->u.tex.level, i);

      /*
       * Blit command:
       */
      OUT_PKT7(ring, CP_EVENT_WRITE, 1);
      OUT_RING(ring, LABEL);
      OUT_WFI5(ring);

      OUT_PKT4(ring, REG_A6XX_RB_DBG_ECO_CNTL, 1);
      OUT_RING(ring, ctx->screen->info->a6xx.magic.RB_DBG_ECO_CNTL_blit);

      OUT_PKT7(ring, CP_BLIT, 1);
      OUT_RING(ring, CP_BLIT_0_OP(BLIT_OP_SCALE));

      OUT_WFI5(ring);

      OUT_PKT4(ring, REG_A6XX_RB_DBG_ECO_CNTL, 1);
      OUT_RING(ring, 0); /* RB_DBG_ECO_CNTL */
   }
}

template void fd6_clear_surface<A6XX>(struct fd_context *ctx, struct fd_ringbuffer *ring,
                                      struct pipe_surface *psurf, const struct pipe_box *box2d,
                                      union pipe_color_union *color, uint32_t unknown_8c01);
template void fd6_clear_surface<A7XX>(struct fd_context *ctx, struct fd_ringbuffer *ring,
                                      struct pipe_surface *psurf, const struct pipe_box *box2d,
                                      union pipe_color_union *color, uint32_t unknown_8c01);

template <chip CHIP>
static void
fd6_clear_texture(struct pipe_context *pctx, struct pipe_resource *prsc,
                  unsigned level, const struct pipe_box *box, const void *data)
   assert_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_resource *rsc = fd_resource(prsc);

   if (DEBUG_BLIT) {
      fprintf(stderr, "surface texture:\ndst resource: ");
      util_dump_resource(stderr, prsc);
      fprintf(stderr, "\n");
   }

   if (!can_do_clear(prsc, level, box)) {
      u_default_clear_texture(pctx, prsc, level, box, data);
      return;
   }

   union pipe_color_union color;

   if (util_format_is_depth_or_stencil(prsc->format)) {
      const struct util_format_description *desc =
             util_format_description(prsc->format);
      float depth = 0.0f;
      uint8_t stencil = 0;

      if (util_format_has_depth(desc))
         util_format_unpack_z_float(prsc->format, &depth, data, 1);

      if (util_format_has_stencil(desc))
         util_format_unpack_s_8uint(prsc->format, &stencil, data, 1);

      if (rsc->stencil)
         fd6_clear_texture<CHIP>(pctx, &rsc->stencil->b.b, level, box, &stencil);

      color.f[0] = depth;
      color.ui[1] = stencil;
   } else {
      util_format_unpack_rgba(prsc->format, color.ui, data, 1);
   }

   struct fd_batch *batch = fd_bc_alloc_batch(ctx, true);

   fd_screen_lock(ctx->screen);
   fd_batch_resource_write(batch, rsc);
   fd_screen_unlock(ctx->screen);

   assert(!batch->flushed);

   /* Marking the batch as needing flush must come after the batch
    * dependency tracking (resource_read()/resource_write()), as that
    * can trigger a flush
    */
   fd_batch_needs_flush(batch);

   fd_batch_update_queries(batch);

   emit_setup(batch);

   struct pipe_surface surf = {
         .format = prsc->format,
         .texture = prsc,
         .u = {
               .tex = {
                     .level = level,
                     .first_layer = box->z,
                     .last_layer = box->depth + box->z - 1,
               },
         },
   };

   fd6_clear_surface<CHIP>(ctx, batch->draw, &surf, box, &color, 0);

   fd6_emit_flushes(batch->ctx, batch->draw,
                    FD6_FLUSH_CCU_COLOR |
                    FD6_FLUSH_CCU_DEPTH |
                    FD6_FLUSH_CACHE |
                    FD6_WAIT_FOR_IDLE);

   fd_batch_flush(batch);
   fd_batch_reference(&batch, NULL);

   /* Acc query state will have been dirtied by our fd_batch_update_queries, so
    * the ctx->batch may need to turn its queries back on.
    */
   fd_context_dirty(ctx, FD_DIRTY_QUERY);
}

template <chip CHIP>
void
fd6_resolve_tile(struct fd_batch *batch, struct fd_ringbuffer *ring,
                 uint32_t base, struct pipe_surface *psurf, uint32_t unknown_8c01)
{
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   uint64_t gmem_base = batch->ctx->screen->gmem_base + base;
   uint32_t gmem_pitch = gmem->bin_w * batch->framebuffer.samples *
                         util_format_get_blocksize(psurf->format);

   OUT_PKT4(ring, REG_A6XX_GRAS_2D_DST_TL, 2);
   OUT_RING(ring, A6XX_GRAS_2D_DST_TL_X(0) | A6XX_GRAS_2D_DST_TL_Y(0));
   OUT_RING(ring, A6XX_GRAS_2D_DST_BR_X(psurf->width - 1) |
                     A6XX_GRAS_2D_DST_BR_Y(psurf->height - 1));

   OUT_REG(ring,
           A6XX_GRAS_2D_SRC_TL_X(0),
           A6XX_GRAS_2D_SRC_BR_X(psurf->width - 1),
           A6XX_GRAS_2D_SRC_TL_Y(0),
           A6XX_GRAS_2D_SRC_BR_Y(psurf->height - 1),
   );

   /* Enable scissor bit, which will take into account the window scissor
    * which is set per-tile
    */
   emit_blit_setup<CHIP>(ring, psurf->format, true, NULL, unknown_8c01, ROTATE_0);

   /* We shouldn't be using GMEM in the layered rendering case: */
   assert(psurf->u.tex.first_layer == psurf->u.tex.last_layer);

   emit_blit_dst(ring, psurf->texture, psurf->format, psurf->u.tex.level,
                 psurf->u.tex.first_layer);

   enum a6xx_format sfmt = fd6_color_format(psurf->format, TILE6_LINEAR);
   enum a3xx_msaa_samples samples = fd_msaa_samples(batch->framebuffer.samples);

   OUT_REG(ring,
           SP_PS_2D_SRC_INFO(
                 CHIP,
                 .color_format = sfmt,
                 .tile_mode = TILE6_2,
                 .color_swap = WZYX,
                 .srgb = util_format_is_srgb(psurf->format),
                 .samples = samples,
                 .samples_average = samples > MSAA_ONE,
                 .unk20 = true,
                 .unk22 = true,
           ),
           SP_PS_2D_SRC_SIZE(
                 CHIP,
                 .width = psurf->width,
                 .height = psurf->height,
           ),
           SP_PS_2D_SRC(
                 CHIP,
                 .qword = gmem_base,
           ),
           SP_PS_2D_SRC_PITCH(
                 CHIP,
                 .pitch = gmem_pitch,
           ),
   );

   /* sync GMEM writes with CACHE. */
   fd6_cache_inv(batch, ring);

   /* Wait for CACHE_INVALIDATE to land */
   OUT_WFI5(ring);

   OUT_PKT7(ring, CP_BLIT, 1);
   OUT_RING(ring, CP_BLIT_0_OP(BLIT_OP_SCALE));

   OUT_WFI5(ring);

   /* CP_BLIT writes to the CCU, unlike CP_EVENT_WRITE::BLIT which writes to
    * sysmem, and we generally assume that GMEM renderpasses leave their
    * results in sysmem, so we need to flush manually here.
    */
   fd6_emit_flushes(batch->ctx, ring,
                    FD6_FLUSH_CCU_COLOR | FD6_WAIT_FOR_IDLE);
}

template void fd6_resolve_tile<A6XX>(struct fd_batch *batch, struct fd_ringbuffer *ring,
                                     uint32_t base, struct pipe_surface *psurf, uint32_t unknown_8c01);
template void fd6_resolve_tile<A7XX>(struct fd_batch *batch, struct fd_ringbuffer *ring,
                                     uint32_t base, struct pipe_surface *psurf, uint32_t unknown_8c01);

template <chip CHIP>
static bool
handle_rgba_blit(struct fd_context *ctx, const struct pipe_blit_info *info)
   assert_dt
{
   struct fd_batch *batch;

   assert(!(info->mask & PIPE_MASK_ZS));

   if (!can_do_blit(info))
      return false;

   struct fd_resource *src = fd_resource(info->src.resource);
   struct fd_resource *dst = fd_resource(info->dst.resource);

   fd6_validate_format(ctx, src, info->src.format);
   fd6_validate_format(ctx, dst, info->dst.format);

   batch = fd_bc_alloc_batch(ctx, true);

   fd_screen_lock(ctx->screen);

   fd_batch_resource_read(batch, src);
   fd_batch_resource_write(batch, dst);

   fd_screen_unlock(ctx->screen);

   assert(!batch->flushed);

   /* Marking the batch as needing flush must come after the batch
    * dependency tracking (resource_read()/resource_write()), as that
    * can trigger a flush
    */
   fd_batch_needs_flush(batch);

   fd_batch_update_queries(batch);

   emit_setup(batch);

   DBG_BLIT(info, batch);

   trace_start_blit(&batch->trace, batch->draw, info->src.resource->target,
                    info->dst.resource->target);

   if ((info->src.resource->target == PIPE_BUFFER) &&
       (info->dst.resource->target == PIPE_BUFFER)) {
      assert(src->layout.tile_mode == TILE6_LINEAR);
      assert(dst->layout.tile_mode == TILE6_LINEAR);
      emit_blit_buffer<CHIP>(ctx, batch->draw, info);
   } else {
      /* I don't *think* we need to handle blits between buffer <-> !buffer */
      assert(info->src.resource->target != PIPE_BUFFER);
      assert(info->dst.resource->target != PIPE_BUFFER);
      emit_blit_texture<CHIP>(ctx, batch->draw, info);
   }

   trace_end_blit(&batch->trace, batch->draw);

   fd6_emit_flushes(batch->ctx, batch->draw,
                    FD6_FLUSH_CCU_COLOR |
                    FD6_FLUSH_CCU_DEPTH |
                    FD6_FLUSH_CACHE |
                    FD6_WAIT_FOR_IDLE);

   fd_batch_flush(batch);
   fd_batch_reference(&batch, NULL);

   /* Acc query state will have been dirtied by our fd_batch_update_queries, so
    * the ctx->batch may need to turn its queries back on.
    */
   fd_context_dirty(ctx, FD_DIRTY_QUERY);

   return true;
}

/**
 * Re-written z/s blits can still fail for various reasons (for example MSAA).
 * But we want to do the fallback blit with the re-written pipe_blit_info,
 * in particular as u_blitter cannot blit stencil.  So handle the fallback
 * ourself and never "fail".
 */
template <chip CHIP>
static bool
do_rewritten_blit(struct fd_context *ctx, const struct pipe_blit_info *info)
   assert_dt
{
   bool success = handle_rgba_blit<CHIP>(ctx, info);
   if (!success) {
      success = fd_blitter_blit(ctx, info);
   }
   assert(success); /* fallback should never fail! */
   return success;
}

/**
 * Handle depth/stencil blits either via u_blitter and/or re-writing the
 * blit into an equivilant format that we can handle
 */
template <chip CHIP>
static bool
handle_zs_blit(struct fd_context *ctx,
               const struct pipe_blit_info *info) assert_dt
{
   struct pipe_blit_info blit = *info;

   if (DEBUG_BLIT) {
      fprintf(stderr, "---- handle_zs_blit: ");
      dump_blit_info(info);
   }

   if (info->src.format != info->dst.format)
      return false;

   struct fd_resource *src = fd_resource(info->src.resource);
   struct fd_resource *dst = fd_resource(info->dst.resource);

   switch (info->dst.format) {
   case PIPE_FORMAT_S8_UINT:
      assert(info->mask == PIPE_MASK_S);
      blit.mask = PIPE_MASK_R;
      blit.src.format = PIPE_FORMAT_R8_UINT;
      blit.dst.format = PIPE_FORMAT_R8_UINT;
      blit.sample0_only = true;
      return do_rewritten_blit<CHIP>(ctx, &blit);

   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      if (info->mask & PIPE_MASK_Z) {
         blit.mask = PIPE_MASK_R;
         blit.src.format = PIPE_FORMAT_R32_FLOAT;
         blit.dst.format = PIPE_FORMAT_R32_FLOAT;
         blit.sample0_only = true;
         do_rewritten_blit<CHIP>(ctx, &blit);
      }

      if (info->mask & PIPE_MASK_S) {
         blit.mask = PIPE_MASK_R;
         blit.src.format = PIPE_FORMAT_R8_UINT;
         blit.dst.format = PIPE_FORMAT_R8_UINT;
         blit.src.resource = &src->stencil->b.b;
         blit.dst.resource = &dst->stencil->b.b;
         blit.sample0_only = true;
         do_rewritten_blit<CHIP>(ctx, &blit);
      }

      return true;

   case PIPE_FORMAT_Z16_UNORM:
      blit.mask = PIPE_MASK_R;
      blit.src.format = PIPE_FORMAT_R16_UNORM;
      blit.dst.format = PIPE_FORMAT_R16_UNORM;
      blit.sample0_only = true;
      return do_rewritten_blit<CHIP>(ctx, &blit);

   case PIPE_FORMAT_Z32_UNORM:
   case PIPE_FORMAT_Z32_FLOAT:
      assert(info->mask == PIPE_MASK_Z);
      blit.mask = PIPE_MASK_R;
      blit.src.format = PIPE_FORMAT_R32_UINT;
      blit.dst.format = PIPE_FORMAT_R32_UINT;
      blit.sample0_only = true;
      return do_rewritten_blit<CHIP>(ctx, &blit);

   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      blit.mask = 0;
      if (info->mask & PIPE_MASK_Z)
         blit.mask |= PIPE_MASK_R | PIPE_MASK_G | PIPE_MASK_B;
      if (info->mask & PIPE_MASK_S)
         blit.mask |= PIPE_MASK_A;
      blit.src.format = PIPE_FORMAT_Z24_UNORM_S8_UINT_AS_R8G8B8A8;
      blit.dst.format = PIPE_FORMAT_Z24_UNORM_S8_UINT_AS_R8G8B8A8;
      /* non-UBWC Z24_UNORM_S8_UINT_AS_R8G8B8A8 is broken on a630, fall back to
       * 8888_unorm.
       */
      if (!ctx->screen->info->a6xx.has_z24uint_s8uint) {
         if (!src->layout.ubwc && !dst->layout.ubwc) {
            blit.src.format = PIPE_FORMAT_RGBA8888_UINT;
            blit.dst.format = PIPE_FORMAT_RGBA8888_UINT;
         } else {
            if (!src->layout.ubwc)
               blit.src.format = PIPE_FORMAT_RGBA8888_UNORM;
            if (!dst->layout.ubwc)
               blit.dst.format = PIPE_FORMAT_RGBA8888_UNORM;
         }
      }
      if (info->src.resource->nr_samples > 1 && blit.src.format != PIPE_FORMAT_RGBA8888_UINT)
         blit.sample0_only = true;
      return fd_blitter_blit(ctx, &blit);

   default:
      return false;
   }
}

template <chip CHIP>
static bool
handle_compressed_blit(struct fd_context *ctx,
                       const struct pipe_blit_info *info) assert_dt
{
   struct pipe_blit_info blit = *info;

   if (DEBUG_BLIT) {
      fprintf(stderr, "---- handle_compressed_blit: ");
      dump_blit_info(info);
   }

   if (info->src.format != info->dst.format)
      return fd_blitter_blit(ctx, info);

   if (util_format_get_blocksize(info->src.format) == 8) {
      blit.src.format = blit.dst.format = PIPE_FORMAT_R16G16B16A16_UINT;
   } else {
      assert(util_format_get_blocksize(info->src.format) == 16);
      blit.src.format = blit.dst.format = PIPE_FORMAT_R32G32B32A32_UINT;
   }

   int bw = util_format_get_blockwidth(info->src.format);
   int bh = util_format_get_blockheight(info->src.format);

   /* NOTE: x/y *must* be aligned to block boundary (ie. in
    * glCompressedTexSubImage2D()) but width/height may not
    * be:
    */

   assert((blit.src.box.x % bw) == 0);
   assert((blit.src.box.y % bh) == 0);

   blit.src.box.x /= bw;
   blit.src.box.y /= bh;
   blit.src.box.width = DIV_ROUND_UP(blit.src.box.width, bw);
   blit.src.box.height = DIV_ROUND_UP(blit.src.box.height, bh);

   assert((blit.dst.box.x % bw) == 0);
   assert((blit.dst.box.y % bh) == 0);

   blit.dst.box.x /= bw;
   blit.dst.box.y /= bh;
   blit.dst.box.width = DIV_ROUND_UP(blit.dst.box.width, bw);
   blit.dst.box.height = DIV_ROUND_UP(blit.dst.box.height, bh);

   return do_rewritten_blit<CHIP>(ctx, &blit);
}

/**
 * For SNORM formats, copy them as the equivalent UNORM format.  If we treat
 * them as snorm then the 0x80 (-1.0 snorm8) value will get clamped to 0x81
 * (also -1.0), when we're supposed to be memcpying the bits. See
 * https://gitlab.khronos.org/Tracker/vk-gl-cts/-/issues/2917 for discussion.
 */
template <chip CHIP>
static bool
handle_snorm_copy_blit(struct fd_context *ctx,
                       const struct pipe_blit_info *info)
   assert_dt
{
   /* If we're interpolating the pixels, we can't just treat the values as unorm. */
   if (info->filter == PIPE_TEX_FILTER_LINEAR)
      return false;

   struct pipe_blit_info blit = *info;

   blit.src.format = blit.dst.format = util_format_snorm_to_unorm(info->src.format);

   return do_rewritten_blit<CHIP>(ctx, &blit);
}

template <chip CHIP>
static bool
fd6_blit(struct fd_context *ctx, const struct pipe_blit_info *info) assert_dt
{
   if (info->mask & PIPE_MASK_ZS)
      return handle_zs_blit<CHIP>(ctx, info);

   if (util_format_is_compressed(info->src.format) ||
       util_format_is_compressed(info->dst.format))
      return handle_compressed_blit<CHIP>(ctx, info);

   if ((info->src.format == info->dst.format) &&
       util_format_is_snorm(info->src.format))
      return handle_snorm_copy_blit<CHIP>(ctx, info);

   return handle_rgba_blit<CHIP>(ctx, info);
}

template <chip CHIP>
void
fd6_blitter_init(struct pipe_context *pctx)
   disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);

   ctx->clear_ubwc = fd6_clear_ubwc<CHIP>;
   ctx->validate_format = fd6_validate_format;

   if (FD_DBG(NOBLIT))
      return;

   pctx->clear_texture = fd6_clear_texture<CHIP>;
   ctx->blit = fd6_blit<CHIP>;
}

/* Teach the compiler about needed variants: */
template void fd6_blitter_init<A6XX>(struct pipe_context *pctx);
template void fd6_blitter_init<A7XX>(struct pipe_context *pctx);

unsigned
fd6_tile_mode_for_format(enum pipe_format pfmt)
{
   /* basically just has to be a format we can blit, so uploads/downloads
    * via linear staging buffer works:
    */
   if (ok_format(pfmt))
      return TILE6_3;

   return TILE6_LINEAR;
}
unsigned
fd6_tile_mode(const struct pipe_resource *tmpl)
{
   /* if the mipmap level 0 is still too small to be tiled, then don't
    * bother pretending:
    */
   if ((tmpl->width0 < FDL_MIN_UBWC_WIDTH) &&
         !util_format_is_depth_or_stencil(tmpl->format))
      return TILE6_LINEAR;

   return fd6_tile_mode_for_format(tmpl->format);
}
