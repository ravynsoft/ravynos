/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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

#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "freedreno_draw.h"
#include "freedreno_resource.h"
#include "freedreno_state.h"

#include "fd4_context.h"
#include "fd4_draw.h"
#include "fd4_emit.h"
#include "fd4_format.h"
#include "fd4_gmem.h"
#include "fd4_program.h"
#include "fd4_zsa.h"

static void
fd4_gmem_emit_set_prog(struct fd_context *ctx, struct fd4_emit *emit,
                       struct fd_program_stateobj *prog)
{
   emit->skip_consts = true;
   emit->key.vs = prog->vs;
   emit->key.fs = prog->fs;
   emit->prog = fd4_program_state(
      ir3_cache_lookup(ctx->shader_cache, &emit->key, &ctx->debug));
   /* reset the fd4_emit_get_*p cache */
   emit->vs = NULL;
   emit->fs = NULL;
}

static void
emit_mrt(struct fd_ringbuffer *ring, unsigned nr_bufs,
         struct pipe_surface **bufs, const uint32_t *bases, uint32_t bin_w,
         bool decode_srgb)
{
   enum a4xx_tile_mode tile_mode;
   unsigned i;

   if (bin_w) {
      tile_mode = 2;
   } else {
      tile_mode = TILE4_LINEAR;
   }

   for (i = 0; i < A4XX_MAX_RENDER_TARGETS; i++) {
      enum a4xx_color_fmt format = 0;
      enum a3xx_color_swap swap = WZYX;
      bool srgb = false;
      struct fd_resource *rsc = NULL;
      uint32_t stride = 0;
      uint32_t base = 0;
      uint32_t offset = 0;

      if ((i < nr_bufs) && bufs[i]) {
         struct pipe_surface *psurf = bufs[i];
         enum pipe_format pformat = psurf->format;

         rsc = fd_resource(psurf->texture);

         /* In case we're drawing to Z32F_S8, the "color" actually goes to
          * the stencil
          */
         if (rsc->stencil) {
            rsc = rsc->stencil;
            pformat = rsc->b.b.format;
            if (bases)
               bases++;
         }

         format = fd4_pipe2color(pformat);
         swap = fd4_pipe2swap(pformat);

         if (decode_srgb)
            srgb = util_format_is_srgb(pformat);
         else
            pformat = util_format_linear(pformat);

         assert(psurf->u.tex.first_layer == psurf->u.tex.last_layer);

         offset = fd_resource_offset(rsc, psurf->u.tex.level,
                                     psurf->u.tex.first_layer);

         if (bin_w) {
            stride = bin_w << fdl_cpp_shift(&rsc->layout);

            if (bases) {
               base = bases[i];
            }
         } else {
            stride = fd_resource_pitch(rsc, psurf->u.tex.level);
         }
      } else if ((i < nr_bufs) && bases) {
         base = bases[i];
      }

      OUT_PKT0(ring, REG_A4XX_RB_MRT_BUF_INFO(i), 3);
      OUT_RING(ring, A4XX_RB_MRT_BUF_INFO_COLOR_FORMAT(format) |
                        A4XX_RB_MRT_BUF_INFO_COLOR_TILE_MODE(tile_mode) |
                        A4XX_RB_MRT_BUF_INFO_COLOR_BUF_PITCH(stride) |
                        A4XX_RB_MRT_BUF_INFO_COLOR_SWAP(swap) |
                        COND(srgb, A4XX_RB_MRT_BUF_INFO_COLOR_SRGB));
      if (bin_w || (i >= nr_bufs) || !bufs[i]) {
         OUT_RING(ring, base);
         OUT_RING(ring, A4XX_RB_MRT_CONTROL3_STRIDE(stride));
      } else {
         OUT_RELOC(ring, rsc->bo, offset, 0, 0);
         /* RB_MRT[i].CONTROL3.STRIDE not emitted by c2d..
          * not sure if we need to skip it for bypass or
          * not.
          */
         OUT_RING(ring, A4XX_RB_MRT_CONTROL3_STRIDE(0));
      }
   }
}

static bool
use_hw_binning(struct fd_batch *batch)
{
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;

   /* workaround: Like on a3xx, hw binning and scissor optimization
    * don't play nice together.
    *
    * Disable binning if scissor optimization is used.
    */
   if (gmem->minx || gmem->miny)
      return false;

   if ((gmem->maxpw * gmem->maxph) > 32)
      return false;

   if ((gmem->maxpw > 15) || (gmem->maxph > 15))
      return false;

   return fd_binning_enabled && ((gmem->nbins_x * gmem->nbins_y) > 2);
}

/* transfer from gmem to system memory (ie. normal RAM) */

static void
emit_gmem2mem_surf(struct fd_batch *batch, bool stencil, uint32_t base,
                   struct pipe_surface *psurf)
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct fd_resource *rsc = fd_resource(psurf->texture);
   enum pipe_format pformat = psurf->format;
   uint32_t offset, pitch;

   if (!rsc->valid)
      return;

   if (stencil) {
      assert(rsc->stencil);
      rsc = rsc->stencil;
      pformat = rsc->b.b.format;
   }

   offset =
      fd_resource_offset(rsc, psurf->u.tex.level, psurf->u.tex.first_layer);
   pitch = fd_resource_pitch(rsc, psurf->u.tex.level);

   assert(psurf->u.tex.first_layer == psurf->u.tex.last_layer);

   OUT_PKT0(ring, REG_A4XX_RB_COPY_CONTROL, 4);
   OUT_RING(ring, A4XX_RB_COPY_CONTROL_MSAA_RESOLVE(MSAA_ONE) |
                     A4XX_RB_COPY_CONTROL_MODE(RB_COPY_RESOLVE) |
                     A4XX_RB_COPY_CONTROL_GMEM_BASE(base));
   OUT_RELOC(ring, rsc->bo, offset, 0, 0); /* RB_COPY_DEST_BASE */
   OUT_RING(ring, A4XX_RB_COPY_DEST_PITCH_PITCH(pitch));
   OUT_RING(ring, A4XX_RB_COPY_DEST_INFO_TILE(TILE4_LINEAR) |
                     A4XX_RB_COPY_DEST_INFO_FORMAT(fd4_pipe2color(pformat)) |
                     A4XX_RB_COPY_DEST_INFO_COMPONENT_ENABLE(0xf) |
                     A4XX_RB_COPY_DEST_INFO_ENDIAN(ENDIAN_NONE) |
                     A4XX_RB_COPY_DEST_INFO_SWAP(fd4_pipe2swap(pformat)));

   fd4_draw(batch, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
            DI_SRC_SEL_AUTO_INDEX, 2, 1, INDEX4_SIZE_8_BIT, 0, 0, NULL);
}

static void
fd4_emit_tile_gmem2mem(struct fd_batch *batch,
                       const struct fd_tile *tile) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd4_emit emit = {
      .debug = &ctx->debug,
      .vtx = &ctx->solid_vbuf_state,
   };
   fd4_gmem_emit_set_prog(ctx, &emit, &ctx->solid_prog);

   OUT_PKT0(ring, REG_A4XX_RB_DEPTH_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_NEVER));

   OUT_PKT0(ring, REG_A4XX_RB_STENCIL_CONTROL, 2);
   OUT_RING(ring, A4XX_RB_STENCIL_CONTROL_FUNC(FUNC_NEVER) |
                     A4XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_NEVER) |
                     A4XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));
   OUT_RING(ring, 0x00000000); /* RB_STENCIL_CONTROL2 */

   OUT_PKT0(ring, REG_A4XX_RB_STENCILREFMASK, 2);
   OUT_RING(ring, 0xff000000 | A4XX_RB_STENCILREFMASK_STENCILREF(0) |
                     A4XX_RB_STENCILREFMASK_STENCILMASK(0) |
                     A4XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));
   OUT_RING(ring, 0xff000000 | A4XX_RB_STENCILREFMASK_BF_STENCILREF(0) |
                     A4XX_RB_STENCILREFMASK_BF_STENCILMASK(0) |
                     A4XX_RB_STENCILREFMASK_BF_STENCILWRITEMASK(0xff));

   OUT_PKT0(ring, REG_A4XX_GRAS_SU_MODE_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SU_MODE_CONTROL_LINEHALFWIDTH(0));

   fd_wfi(batch, ring);

   OUT_PKT0(ring, REG_A4XX_GRAS_CL_CLIP_CNTL, 1);
   OUT_RING(ring, 0x80000); /* GRAS_CL_CLIP_CNTL */

   OUT_PKT0(ring, REG_A4XX_GRAS_CL_VPORT_XOFFSET_0, 6);
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_XOFFSET_0((float)pfb->width / 2.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_XSCALE_0((float)pfb->width / 2.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_YOFFSET_0((float)pfb->height / 2.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_YSCALE_0(-(float)pfb->height / 2.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZOFFSET_0(0.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZSCALE_0(1.0f));

   OUT_PKT0(ring, REG_A4XX_RB_RENDER_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_RENDER_CONTROL_DISABLE_COLOR_PIPE | 0xa); /* XXX */

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RESOLVE_PASS) |
                     A4XX_GRAS_SC_CONTROL_MSAA_DISABLE |
                     A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A4XX_GRAS_SC_CONTROL_RASTER_MODE(1));

   OUT_PKT0(ring, REG_A4XX_PC_PRIM_VTX_CNTL, 1);
   OUT_RING(ring, A4XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST);

   OUT_PKT0(ring, REG_A4XX_GRAS_ALPHA_CONTROL, 1);
   OUT_RING(ring, 0x00000002);

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_WINDOW_SCISSOR_BR, 2);
   OUT_RING(ring, A4XX_GRAS_SC_WINDOW_SCISSOR_BR_X(pfb->width - 1) |
                     A4XX_GRAS_SC_WINDOW_SCISSOR_BR_Y(pfb->height - 1));
   OUT_RING(ring, A4XX_GRAS_SC_WINDOW_SCISSOR_TL_X(0) |
                     A4XX_GRAS_SC_WINDOW_SCISSOR_TL_Y(0));

   OUT_PKT0(ring, REG_A4XX_VFD_INDEX_OFFSET, 2);
   OUT_RING(ring, 0); /* VFD_INDEX_OFFSET */
   OUT_RING(ring, 0); /* ??? UNKNOWN_2209 */

   fd4_program_emit(ring, &emit, 0, NULL);
   fd4_emit_vertex_bufs(ring, &emit);

   if (batch->resolve & (FD_BUFFER_DEPTH | FD_BUFFER_STENCIL)) {
      struct fd_resource *rsc = fd_resource(pfb->zsbuf->texture);
      if (!rsc->stencil || (batch->resolve & FD_BUFFER_DEPTH))
         emit_gmem2mem_surf(batch, false, gmem->zsbuf_base[0], pfb->zsbuf);
      if (rsc->stencil && (batch->resolve & FD_BUFFER_STENCIL))
         emit_gmem2mem_surf(batch, true, gmem->zsbuf_base[1], pfb->zsbuf);
   }

   if (batch->resolve & FD_BUFFER_COLOR) {
      unsigned i;
      for (i = 0; i < pfb->nr_cbufs; i++) {
         if (!pfb->cbufs[i])
            continue;
         if (!(batch->resolve & (PIPE_CLEAR_COLOR0 << i)))
            continue;
         emit_gmem2mem_surf(batch, false, gmem->cbuf_base[i], pfb->cbufs[i]);
      }
   }

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A4XX_GRAS_SC_CONTROL_MSAA_DISABLE |
                     A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A4XX_GRAS_SC_CONTROL_RASTER_MODE(0));
}

/* transfer from system memory to gmem */

static void
emit_mem2gmem_surf(struct fd_batch *batch, const uint32_t *bases,
                   struct pipe_surface **bufs, uint32_t nr_bufs, uint32_t bin_w)
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_surface *zsbufs[2];

   emit_mrt(ring, nr_bufs, bufs, bases, bin_w, false);

   if (bufs[0] && (bufs[0]->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)) {
      /* The gmem_restore_tex logic will put the first buffer's stencil
       * as color. Supply it with the proper information to make that
       * happen.
       */
      zsbufs[0] = zsbufs[1] = bufs[0];
      bufs = zsbufs;
      nr_bufs = 2;
   }

   fd4_emit_gmem_restore_tex(ring, nr_bufs, bufs);

   fd4_draw(batch, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
            DI_SRC_SEL_AUTO_INDEX, 2, 1, INDEX4_SIZE_8_BIT, 0, 0, NULL);
}

static void
fd4_emit_tile_mem2gmem(struct fd_batch *batch,
                       const struct fd_tile *tile) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd4_emit emit = {
      .debug = &ctx->debug,
      .vtx = &ctx->blit_vbuf_state,
      .sprite_coord_enable = 1,
      .no_decode_srgb = true,
   };
   /* NOTE: They all use the same VP, this is for vtx bufs. */
   fd4_gmem_emit_set_prog(ctx, &emit, &ctx->blit_prog[0]);

   unsigned char mrt_comp[A4XX_MAX_RENDER_TARGETS] = {0};
   float x0, y0, x1, y1;
   unsigned bin_w = tile->bin_w;
   unsigned bin_h = tile->bin_h;
   unsigned i;

   /* write texture coordinates to vertexbuf: */
   x0 = ((float)tile->xoff) / ((float)pfb->width);
   x1 = ((float)tile->xoff + bin_w) / ((float)pfb->width);
   y0 = ((float)tile->yoff) / ((float)pfb->height);
   y1 = ((float)tile->yoff + bin_h) / ((float)pfb->height);

   OUT_PKT3(ring, CP_MEM_WRITE, 5);
   OUT_RELOC(ring, fd_resource(ctx->blit_texcoord_vbuf)->bo, 0, 0, 0);
   OUT_RING(ring, fui(x0));
   OUT_RING(ring, fui(y0));
   OUT_RING(ring, fui(x1));
   OUT_RING(ring, fui(y1));

   for (i = 0; i < A4XX_MAX_RENDER_TARGETS; i++) {
      mrt_comp[i] = ((i < pfb->nr_cbufs) && pfb->cbufs[i]) ? 0xf : 0;

      OUT_PKT0(ring, REG_A4XX_RB_MRT_CONTROL(i), 1);
      OUT_RING(ring, A4XX_RB_MRT_CONTROL_ROP_CODE(ROP_COPY) |
                        A4XX_RB_MRT_CONTROL_COMPONENT_ENABLE(0xf));

      OUT_PKT0(ring, REG_A4XX_RB_MRT_BLEND_CONTROL(i), 1);
      OUT_RING(
         ring,
         A4XX_RB_MRT_BLEND_CONTROL_RGB_SRC_FACTOR(FACTOR_ONE) |
            A4XX_RB_MRT_BLEND_CONTROL_RGB_BLEND_OPCODE(BLEND_DST_PLUS_SRC) |
            A4XX_RB_MRT_BLEND_CONTROL_RGB_DEST_FACTOR(FACTOR_ZERO) |
            A4XX_RB_MRT_BLEND_CONTROL_ALPHA_SRC_FACTOR(FACTOR_ONE) |
            A4XX_RB_MRT_BLEND_CONTROL_ALPHA_BLEND_OPCODE(BLEND_DST_PLUS_SRC) |
            A4XX_RB_MRT_BLEND_CONTROL_ALPHA_DEST_FACTOR(FACTOR_ZERO));
   }

   OUT_PKT0(ring, REG_A4XX_RB_RENDER_COMPONENTS, 1);
   OUT_RING(ring, A4XX_RB_RENDER_COMPONENTS_RT0(mrt_comp[0]) |
                     A4XX_RB_RENDER_COMPONENTS_RT1(mrt_comp[1]) |
                     A4XX_RB_RENDER_COMPONENTS_RT2(mrt_comp[2]) |
                     A4XX_RB_RENDER_COMPONENTS_RT3(mrt_comp[3]) |
                     A4XX_RB_RENDER_COMPONENTS_RT4(mrt_comp[4]) |
                     A4XX_RB_RENDER_COMPONENTS_RT5(mrt_comp[5]) |
                     A4XX_RB_RENDER_COMPONENTS_RT6(mrt_comp[6]) |
                     A4XX_RB_RENDER_COMPONENTS_RT7(mrt_comp[7]));

   OUT_PKT0(ring, REG_A4XX_RB_RENDER_CONTROL, 1);
   OUT_RING(ring, 0x8); /* XXX RB_RENDER_CONTROL */

   OUT_PKT0(ring, REG_A4XX_RB_DEPTH_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_LESS));

   OUT_PKT0(ring, REG_A4XX_GRAS_CL_CLIP_CNTL, 1);
   OUT_RING(ring, 0x280000); /* XXX GRAS_CL_CLIP_CNTL */

   OUT_PKT0(ring, REG_A4XX_GRAS_SU_MODE_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SU_MODE_CONTROL_LINEHALFWIDTH(0) |
                     A4XX_GRAS_SU_MODE_CONTROL_RENDERING_PASS);

   OUT_PKT0(ring, REG_A4XX_GRAS_CL_VPORT_XOFFSET_0, 6);
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_XOFFSET_0((float)bin_w / 2.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_XSCALE_0((float)bin_w / 2.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_YOFFSET_0((float)bin_h / 2.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_YSCALE_0(-(float)bin_h / 2.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZOFFSET_0(0.0f));
   OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZSCALE_0(1.0f));

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_WINDOW_SCISSOR_BR, 2);
   OUT_RING(ring, A4XX_GRAS_SC_WINDOW_SCISSOR_BR_X(bin_w - 1) |
                     A4XX_GRAS_SC_WINDOW_SCISSOR_BR_Y(bin_h - 1));
   OUT_RING(ring, A4XX_GRAS_SC_WINDOW_SCISSOR_TL_X(0) |
                     A4XX_GRAS_SC_WINDOW_SCISSOR_TL_Y(0));

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A4XX_GRAS_SC_SCREEN_SCISSOR_TL_X(0) |
                     A4XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(0));
   OUT_RING(ring, A4XX_GRAS_SC_SCREEN_SCISSOR_BR_X(bin_w - 1) |
                     A4XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(bin_h - 1));

   OUT_PKT0(ring, REG_A4XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_MODE_CONTROL_WIDTH(gmem->bin_w) |
                     A4XX_RB_MODE_CONTROL_HEIGHT(gmem->bin_h));

   OUT_PKT0(ring, REG_A4XX_RB_STENCIL_CONTROL, 2);
   OUT_RING(ring, A4XX_RB_STENCIL_CONTROL_FUNC(FUNC_ALWAYS) |
                     A4XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_ALWAYS) |
                     A4XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
                     A4XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));
   OUT_RING(ring, 0x00000000); /* RB_STENCIL_CONTROL2 */

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A4XX_GRAS_SC_CONTROL_MSAA_DISABLE |
                     A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A4XX_GRAS_SC_CONTROL_RASTER_MODE(1));

   OUT_PKT0(ring, REG_A4XX_PC_PRIM_VTX_CNTL, 1);
   OUT_RING(ring, A4XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST |
                     A4XX_PC_PRIM_VTX_CNTL_VAROUT(1));

   OUT_PKT0(ring, REG_A4XX_VFD_INDEX_OFFSET, 2);
   OUT_RING(ring, 0); /* VFD_INDEX_OFFSET */
   OUT_RING(ring, 0); /* ??? UNKNOWN_2209 */

   fd4_emit_vertex_bufs(ring, &emit);

   /* for gmem pitch/base calculations, we need to use the non-
    * truncated tile sizes:
    */
   bin_w = gmem->bin_w;
   bin_h = gmem->bin_h;

   if (fd_gmem_needs_restore(batch, tile, FD_BUFFER_COLOR)) {
      fd4_gmem_emit_set_prog(ctx, &emit, &ctx->blit_prog[pfb->nr_cbufs - 1]);
      fd4_program_emit(ring, &emit, pfb->nr_cbufs, pfb->cbufs);
      emit_mem2gmem_surf(batch, gmem->cbuf_base, pfb->cbufs, pfb->nr_cbufs,
                         bin_w);
   }

   if (fd_gmem_needs_restore(batch, tile,
                             FD_BUFFER_DEPTH | FD_BUFFER_STENCIL)) {
      switch (pfb->zsbuf->format) {
      case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      case PIPE_FORMAT_Z32_FLOAT:
         if (pfb->zsbuf->format == PIPE_FORMAT_Z32_FLOAT)
            fd4_gmem_emit_set_prog(ctx, &emit, &ctx->blit_z);
         else
            fd4_gmem_emit_set_prog(ctx, &emit, &ctx->blit_zs);

         OUT_PKT0(ring, REG_A4XX_RB_DEPTH_CONTROL, 1);
         OUT_RING(ring, A4XX_RB_DEPTH_CONTROL_Z_TEST_ENABLE |
                           A4XX_RB_DEPTH_CONTROL_Z_WRITE_ENABLE |
                           A4XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_ALWAYS) |
                           A4XX_RB_DEPTH_CONTROL_EARLY_Z_DISABLE);

         OUT_PKT0(ring, REG_A4XX_GRAS_ALPHA_CONTROL, 1);
         OUT_RING(ring, A4XX_GRAS_ALPHA_CONTROL_ALPHA_TEST_ENABLE);

         OUT_PKT0(ring, REG_A4XX_GRAS_CL_CLIP_CNTL, 1);
         OUT_RING(ring, 0x80000); /* GRAS_CL_CLIP_CNTL */

         break;
      default:
         /* Non-float can use a regular color write. It's split over 8-bit
          * components, so half precision is always sufficient.
          */
         fd4_gmem_emit_set_prog(ctx, &emit, &ctx->blit_prog[0]);
         break;
      }
      fd4_program_emit(ring, &emit, 1, &pfb->zsbuf);
      emit_mem2gmem_surf(batch, gmem->zsbuf_base, &pfb->zsbuf, 1, bin_w);
   }

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A4XX_GRAS_SC_CONTROL_RASTER_MODE(0));

   OUT_PKT0(ring, REG_A4XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_MODE_CONTROL_WIDTH(gmem->bin_w) |
                     A4XX_RB_MODE_CONTROL_HEIGHT(gmem->bin_h) |
                     0x00010000); /* XXX */
}

static void
patch_draws(struct fd_batch *batch, enum pc_di_vis_cull_mode vismode)
{
   unsigned i;
   for (i = 0; i < fd_patch_num_elements(&batch->draw_patches); i++) {
      struct fd_cs_patch *patch = fd_patch_element(&batch->draw_patches, i);
      *patch->cs = patch->val | DRAW4(0, 0, 0, vismode);
   }
   util_dynarray_clear(&batch->draw_patches);
}

/* for rendering directly to system memory: */
static void
fd4_emit_sysmem_prep(struct fd_batch *batch) assert_dt
{
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd_ringbuffer *ring = batch->gmem;

   fd4_emit_restore(batch, ring);

   OUT_PKT0(ring, REG_A4XX_RB_FRAME_BUFFER_DIMENSION, 1);
   OUT_RING(ring, A4XX_RB_FRAME_BUFFER_DIMENSION_WIDTH(pfb->width) |
                     A4XX_RB_FRAME_BUFFER_DIMENSION_HEIGHT(pfb->height));

   emit_mrt(ring, pfb->nr_cbufs, pfb->cbufs, NULL, 0, true);

   /* setup scissor/offset for current tile: */
   OUT_PKT0(ring, REG_A4XX_RB_BIN_OFFSET, 1);
   OUT_RING(ring, A4XX_RB_BIN_OFFSET_X(0) | A4XX_RB_BIN_OFFSET_Y(0));

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A4XX_GRAS_SC_SCREEN_SCISSOR_TL_X(0) |
                     A4XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(0));
   OUT_RING(ring, A4XX_GRAS_SC_SCREEN_SCISSOR_BR_X(pfb->width - 1) |
                     A4XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(pfb->height - 1));

   OUT_PKT0(ring, REG_A4XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_MODE_CONTROL_WIDTH(0) |
                     A4XX_RB_MODE_CONTROL_HEIGHT(0) | 0x00c00000); /* XXX */

   OUT_PKT0(ring, REG_A4XX_RB_RENDER_CONTROL, 1);
   OUT_RING(ring, 0x8);

   patch_draws(batch, IGNORE_VISIBILITY);
}

static void
update_vsc_pipe(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct fd4_context *fd4_ctx = fd4_context(ctx);
   struct fd_ringbuffer *ring = batch->gmem;
   int i;

   OUT_PKT0(ring, REG_A4XX_VSC_SIZE_ADDRESS, 1);
   OUT_RELOC(ring, fd4_ctx->vsc_size_mem, 0, 0, 0); /* VSC_SIZE_ADDRESS */

   OUT_PKT0(ring, REG_A4XX_VSC_PIPE_CONFIG_REG(0), 8);
   for (i = 0; i < 8; i++) {
      const struct fd_vsc_pipe *pipe = &gmem->vsc_pipe[i];
      OUT_RING(ring, A4XX_VSC_PIPE_CONFIG_REG_X(pipe->x) |
                        A4XX_VSC_PIPE_CONFIG_REG_Y(pipe->y) |
                        A4XX_VSC_PIPE_CONFIG_REG_W(pipe->w) |
                        A4XX_VSC_PIPE_CONFIG_REG_H(pipe->h));
   }

   OUT_PKT0(ring, REG_A4XX_VSC_PIPE_DATA_ADDRESS_REG(0), 8);
   for (i = 0; i < 8; i++) {
      if (!ctx->vsc_pipe_bo[i]) {
         ctx->vsc_pipe_bo[i] = fd_bo_new(
            ctx->dev, 0x40000, 0, "vsc_pipe[%u]", i);
      }
      OUT_RELOC(ring, ctx->vsc_pipe_bo[i], 0, 0,
                0); /* VSC_PIPE_DATA_ADDRESS[i] */
   }

   OUT_PKT0(ring, REG_A4XX_VSC_PIPE_DATA_LENGTH_REG(0), 8);
   for (i = 0; i < 8; i++) {
      OUT_RING(ring, fd_bo_size(ctx->vsc_pipe_bo[i]) -
                        32); /* VSC_PIPE_DATA_LENGTH[i] */
   }
}

static void
emit_binning_pass(struct fd_batch *batch) assert_dt
{
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd_ringbuffer *ring = batch->gmem;
   int i;

   uint32_t x1 = gmem->minx;
   uint32_t y1 = gmem->miny;
   uint32_t x2 = gmem->minx + gmem->width - 1;
   uint32_t y2 = gmem->miny + gmem->height - 1;

   OUT_PKT0(ring, REG_A4XX_PC_BINNING_COMMAND, 1);
   OUT_RING(ring, A4XX_PC_BINNING_COMMAND_BINNING_ENABLE);

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_TILING_PASS) |
                     A4XX_GRAS_SC_CONTROL_MSAA_DISABLE |
                     A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A4XX_GRAS_SC_CONTROL_RASTER_MODE(0));

   OUT_PKT0(ring, REG_A4XX_RB_FRAME_BUFFER_DIMENSION, 1);
   OUT_RING(ring, A4XX_RB_FRAME_BUFFER_DIMENSION_WIDTH(pfb->width) |
                     A4XX_RB_FRAME_BUFFER_DIMENSION_HEIGHT(pfb->height));

   /* setup scissor/offset for whole screen: */
   OUT_PKT0(ring, REG_A4XX_RB_BIN_OFFSET, 1);
   OUT_RING(ring, A4XX_RB_BIN_OFFSET_X(x1) | A4XX_RB_BIN_OFFSET_Y(y1));

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A4XX_GRAS_SC_SCREEN_SCISSOR_TL_X(x1) |
                     A4XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(y1));
   OUT_RING(ring, A4XX_GRAS_SC_SCREEN_SCISSOR_BR_X(x2) |
                     A4XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(y2));

   for (i = 0; i < A4XX_MAX_RENDER_TARGETS; i++) {
      OUT_PKT0(ring, REG_A4XX_RB_MRT_CONTROL(i), 1);
      OUT_RING(ring, A4XX_RB_MRT_CONTROL_ROP_CODE(ROP_CLEAR) |
                        A4XX_RB_MRT_CONTROL_COMPONENT_ENABLE(0xf));
   }

   /* emit IB to binning drawcmds: */
   fd4_emit_ib(ring, batch->binning);

   fd_reset_wfi(batch);
   fd_wfi(batch, ring);

   /* and then put stuff back the way it was: */

   OUT_PKT0(ring, REG_A4XX_PC_BINNING_COMMAND, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A4XX_GRAS_SC_CONTROL_MSAA_DISABLE |
                     A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A4XX_GRAS_SC_CONTROL_RASTER_MODE(0));

   fd_event_write(batch, ring, CACHE_FLUSH);
   fd_wfi(batch, ring);
}

/* before first tile */
static void
fd4_emit_tile_init(struct fd_batch *batch) assert_dt
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;

   fd4_emit_restore(batch, ring);

   OUT_PKT0(ring, REG_A4XX_VSC_BIN_SIZE, 1);
   OUT_RING(ring, A4XX_VSC_BIN_SIZE_WIDTH(gmem->bin_w) |
                     A4XX_VSC_BIN_SIZE_HEIGHT(gmem->bin_h));

   update_vsc_pipe(batch);

   fd_wfi(batch, ring);
   OUT_PKT0(ring, REG_A4XX_RB_FRAME_BUFFER_DIMENSION, 1);
   OUT_RING(ring, A4XX_RB_FRAME_BUFFER_DIMENSION_WIDTH(pfb->width) |
                     A4XX_RB_FRAME_BUFFER_DIMENSION_HEIGHT(pfb->height));

   if (use_hw_binning(batch)) {
      OUT_PKT0(ring, REG_A4XX_RB_MODE_CONTROL, 1);
      OUT_RING(ring, A4XX_RB_MODE_CONTROL_WIDTH(gmem->bin_w) |
                        A4XX_RB_MODE_CONTROL_HEIGHT(gmem->bin_h));

      OUT_PKT0(ring, REG_A4XX_RB_RENDER_CONTROL, 1);
      OUT_RING(ring, A4XX_RB_RENDER_CONTROL_BINNING_PASS |
                        A4XX_RB_RENDER_CONTROL_DISABLE_COLOR_PIPE | 0x8);

      /* emit hw binning pass: */
      emit_binning_pass(batch);

      patch_draws(batch, USE_VISIBILITY);
   } else {
      patch_draws(batch, IGNORE_VISIBILITY);
   }

   OUT_PKT0(ring, REG_A4XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_MODE_CONTROL_WIDTH(gmem->bin_w) |
                     A4XX_RB_MODE_CONTROL_HEIGHT(gmem->bin_h) |
                     A4XX_RB_MODE_CONTROL_ENABLE_GMEM);
}

/* before mem2gmem */
static void
fd4_emit_tile_prep(struct fd_batch *batch, const struct fd_tile *tile)
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;

   if (pfb->zsbuf) {
      struct fd_resource *rsc = fd_resource(pfb->zsbuf->texture);
      uint32_t cpp = rsc->layout.cpp;

      OUT_PKT0(ring, REG_A4XX_RB_DEPTH_INFO, 3);
      OUT_RING(ring, A4XX_RB_DEPTH_INFO_DEPTH_BASE(gmem->zsbuf_base[0]) |
                        A4XX_RB_DEPTH_INFO_DEPTH_FORMAT(
                           fd4_pipe2depth(pfb->zsbuf->format)));
      OUT_RING(ring, A4XX_RB_DEPTH_PITCH(cpp * gmem->bin_w));
      OUT_RING(ring, A4XX_RB_DEPTH_PITCH2(cpp * gmem->bin_w));

      OUT_PKT0(ring, REG_A4XX_RB_STENCIL_INFO, 2);
      if (rsc->stencil) {
         OUT_RING(ring,
                  A4XX_RB_STENCIL_INFO_SEPARATE_STENCIL |
                     A4XX_RB_STENCIL_INFO_STENCIL_BASE(gmem->zsbuf_base[1]));
         OUT_RING(ring, A4XX_RB_STENCIL_PITCH(rsc->stencil->layout.cpp *
                                              gmem->bin_w));
      } else {
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
      }
   } else {
      OUT_PKT0(ring, REG_A4XX_RB_DEPTH_INFO, 3);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x00000000);

      OUT_PKT0(ring, REG_A4XX_RB_STENCIL_INFO, 2);
      OUT_RING(ring, 0); /* RB_STENCIL_INFO */
      OUT_RING(ring, 0); /* RB_STENCIL_PITCH */
   }

   OUT_PKT0(ring, REG_A4XX_GRAS_DEPTH_CONTROL, 1);
   if (pfb->zsbuf) {
      OUT_RING(ring, A4XX_GRAS_DEPTH_CONTROL_FORMAT(
                        fd4_pipe2depth(pfb->zsbuf->format)));
   } else {
      OUT_RING(ring, A4XX_GRAS_DEPTH_CONTROL_FORMAT(DEPTH4_NONE));
   }
}

/* before IB to rendering cmds: */
static void
fd4_emit_tile_renderprep(struct fd_batch *batch,
                         const struct fd_tile *tile) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   struct fd4_context *fd4_ctx = fd4_context(ctx);
   struct fd_ringbuffer *ring = batch->gmem;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;

   uint32_t x1 = tile->xoff;
   uint32_t y1 = tile->yoff;
   uint32_t x2 = tile->xoff + tile->bin_w - 1;
   uint32_t y2 = tile->yoff + tile->bin_h - 1;

   if (use_hw_binning(batch)) {
      const struct fd_vsc_pipe *pipe = &gmem->vsc_pipe[tile->p];
      struct fd_bo *pipe_bo = ctx->vsc_pipe_bo[tile->p];

      assert(pipe->w && pipe->h);

      fd_event_write(batch, ring, HLSQ_FLUSH);
      fd_wfi(batch, ring);

      OUT_PKT0(ring, REG_A4XX_PC_VSTREAM_CONTROL, 1);
      OUT_RING(ring, A4XX_PC_VSTREAM_CONTROL_SIZE(pipe->w * pipe->h) |
                        A4XX_PC_VSTREAM_CONTROL_N(tile->n));

      OUT_PKT3(ring, CP_SET_BIN_DATA, 2);
      OUT_RELOC(ring, pipe_bo, 0, 0,
                0); /* BIN_DATA_ADDR <- VSC_PIPE[p].DATA_ADDRESS */
      OUT_RELOC(ring, fd4_ctx->vsc_size_mem, /* BIN_SIZE_ADDR <-
                                                VSC_SIZE_ADDRESS + (p * 4) */
                (tile->p * 4), 0, 0);
   } else {
      OUT_PKT0(ring, REG_A4XX_PC_VSTREAM_CONTROL, 1);
      OUT_RING(ring, 0x00000000);
   }

   OUT_PKT3(ring, CP_SET_BIN, 3);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, CP_SET_BIN_1_X1(x1) | CP_SET_BIN_1_Y1(y1));
   OUT_RING(ring, CP_SET_BIN_2_X2(x2) | CP_SET_BIN_2_Y2(y2));

   emit_mrt(ring, pfb->nr_cbufs, pfb->cbufs, gmem->cbuf_base, gmem->bin_w,
            true);

   /* setup scissor/offset for current tile: */
   OUT_PKT0(ring, REG_A4XX_RB_BIN_OFFSET, 1);
   OUT_RING(ring, A4XX_RB_BIN_OFFSET_X(tile->xoff) |
                     A4XX_RB_BIN_OFFSET_Y(tile->yoff));

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A4XX_GRAS_SC_SCREEN_SCISSOR_TL_X(x1) |
                     A4XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(y1));
   OUT_RING(ring, A4XX_GRAS_SC_SCREEN_SCISSOR_BR_X(x2) |
                     A4XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(y2));

   OUT_PKT0(ring, REG_A4XX_RB_RENDER_CONTROL, 1);
   OUT_RING(ring, 0x8);
}

void
fd4_gmem_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);

   ctx->emit_sysmem_prep = fd4_emit_sysmem_prep;
   ctx->emit_tile_init = fd4_emit_tile_init;
   ctx->emit_tile_prep = fd4_emit_tile_prep;
   ctx->emit_tile_mem2gmem = fd4_emit_tile_mem2gmem;
   ctx->emit_tile_renderprep = fd4_emit_tile_renderprep;
   ctx->emit_tile_gmem2mem = fd4_emit_tile_gmem2mem;
}
