/*
 * Copyright (C) 2012-2013 Rob Clark <robclark@freedesktop.org>
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
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_string.h"

#include "freedreno_resource.h"
#include "freedreno_state.h"

#include "fd2_context.h"
#include "fd2_draw.h"
#include "fd2_emit.h"
#include "fd2_program.h"
#include "fd2_util.h"
#include "fd2_zsa.h"

static inline uint32_t
pack_rgba(enum pipe_format format, const float *rgba)
{
   union util_color uc;
   util_pack_color(rgba, format, &uc);
   return uc.ui[0];
}

static void
emit_cacheflush(struct fd_ringbuffer *ring)
{
   unsigned i;

   for (i = 0; i < 12; i++) {
      OUT_PKT3(ring, CP_EVENT_WRITE, 1);
      OUT_RING(ring, CACHE_FLUSH);
   }
}

static void
emit_vertexbufs(struct fd_context *ctx) assert_dt
{
   struct fd_vertex_stateobj *vtx = ctx->vtx.vtx;
   struct fd_vertexbuf_stateobj *vertexbuf = &ctx->vtx.vertexbuf;
   struct fd2_vertex_buf bufs[PIPE_MAX_ATTRIBS];
   unsigned i;

   if (!vtx->num_elements)
      return;

   for (i = 0; i < vtx->num_elements; i++) {
      struct pipe_vertex_element *elem = &vtx->pipe[i];
      struct pipe_vertex_buffer *vb = &vertexbuf->vb[elem->vertex_buffer_index];
      bufs[i].offset = vb->buffer_offset;
      bufs[i].size = fd_bo_size(fd_resource(vb->buffer.resource)->bo);
      bufs[i].prsc = vb->buffer.resource;
   }

   // NOTE I believe the 0x78 (or 0x9c in solid_vp) relates to the
   // CONST(20,0) (or CONST(26,0) in soliv_vp)

   fd2_emit_vertex_bufs(ctx->batch->draw, 0x78, bufs, vtx->num_elements);
   fd2_emit_vertex_bufs(ctx->batch->binning, 0x78, bufs, vtx->num_elements);
}

static void
draw_impl(struct fd_context *ctx, const struct pipe_draw_info *info,
          const struct pipe_draw_start_count_bias *draw, struct fd_ringbuffer *ring,
          unsigned index_offset, bool binning) assert_dt
{
   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_INDX_OFFSET));
   OUT_RING(ring, info->index_size ? 0 : draw->start);

   OUT_PKT0(ring, REG_A2XX_TC_CNTL_STATUS, 1);
   OUT_RING(ring, A2XX_TC_CNTL_STATUS_L2_INVALIDATE);

   if (is_a20x(ctx->screen)) {
      /* wait for DMA to finish and
       * dummy draw one triangle with indexes 0,0,0.
       * with PRE_FETCH_CULL_ENABLE | GRP_CULL_ENABLE.
       *
       * this workaround is for a HW bug related to DMA alignment:
       * it is necessary for indexed draws and possibly also
       * draws that read binning data
       */
      OUT_PKT3(ring, CP_WAIT_REG_EQ, 4);
      OUT_RING(ring, 0x000005d0); /* RBBM_STATUS */
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x00001000); /* bit: 12: VGT_BUSY_NO_DMA */
      OUT_RING(ring, 0x00000001);

      OUT_PKT3(ring, CP_DRAW_INDX_BIN, 6);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x0003c004);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x00000003);
      OUT_RELOC(ring, fd_resource(fd2_context(ctx)->solid_vertexbuf)->bo, 64, 0,
                0);
      OUT_RING(ring, 0x00000006);
   } else {
      OUT_WFI(ring);

      OUT_PKT3(ring, CP_SET_CONSTANT, 3);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_MAX_VTX_INDX));
      OUT_RING(ring, info->index_bounds_valid ? info->max_index
                                              : ~0); /* VGT_MAX_VTX_INDX */
      OUT_RING(ring, info->index_bounds_valid ? info->min_index
                                              : 0); /* VGT_MIN_VTX_INDX */
   }

   /* binning shader will take offset from C64 */
   if (binning && is_a20x(ctx->screen)) {
      OUT_PKT3(ring, CP_SET_CONSTANT, 5);
      OUT_RING(ring, 0x00000180);
      OUT_RING(ring, fui(ctx->batch->num_vertices));
      OUT_RING(ring, fui(0.0f));
      OUT_RING(ring, fui(0.0f));
      OUT_RING(ring, fui(0.0f));
   }

   enum pc_di_vis_cull_mode vismode = USE_VISIBILITY;
   if (binning || info->mode == MESA_PRIM_POINTS)
      vismode = IGNORE_VISIBILITY;

   fd_draw_emit(ctx->batch, ring, ctx->screen->primtypes[info->mode],
                vismode, info, draw, index_offset);

   if (is_a20x(ctx->screen)) {
      /* not sure why this is required, but it fixes some hangs */
      OUT_WFI(ring);
   } else {
      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_UNKNOWN_2010));
      OUT_RING(ring, 0x00000000);
   }

   emit_cacheflush(ring);
}

static bool
fd2_draw_vbo(struct fd_context *ctx, const struct pipe_draw_info *pinfo,
             unsigned drawid_offset,
             const struct pipe_draw_indirect_info *indirect,
             const struct pipe_draw_start_count_bias *pdraw,
             unsigned index_offset) assert_dt
{
   if (!ctx->prog.fs || !ctx->prog.vs)
      return false;

   if (pinfo->mode != MESA_PRIM_COUNT && !indirect && !pinfo->primitive_restart &&
       !u_trim_pipe_prim(pinfo->mode, (unsigned *)&pdraw->count))
      return false;

   if (ctx->dirty & FD_DIRTY_VTXBUF)
      emit_vertexbufs(ctx);

   fd_blend_tracking(ctx);

   if (fd_binning_enabled)
      fd2_emit_state_binning(ctx, ctx->dirty);

   fd2_emit_state(ctx, ctx->dirty);

   /* a2xx can draw only 65535 vertices at once
    * on a22x the field in the draw command is 32bits but seems limited too
    * using a limit of 32k because it fixes an unexplained hang
    * 32766 works for all primitives (multiple of 2 and 3)
    */
   if (pdraw->count > 32766) {
      /* clang-format off */
      static const uint16_t step_tbl[MESA_PRIM_COUNT] = {
         [0 ... MESA_PRIM_COUNT - 1]  = 32766,
         [MESA_PRIM_LINE_STRIP]     = 32765,
         [MESA_PRIM_TRIANGLE_STRIP] = 32764,

         /* needs more work */
         [MESA_PRIM_TRIANGLE_FAN]   = 0,
         [MESA_PRIM_LINE_LOOP]      = 0,
      };
      /* clang-format on */

      struct pipe_draw_start_count_bias draw = *pdraw;
      unsigned count = draw.count;
      unsigned step = step_tbl[pinfo->mode];
      unsigned num_vertices = ctx->batch->num_vertices;

      if (!step)
         return false;

      for (; count + step > 32766; count -= step) {
         draw.count = MIN2(count, 32766);
         draw_impl(ctx, pinfo, &draw, ctx->batch->draw, index_offset, false);
         draw_impl(ctx, pinfo, &draw, ctx->batch->binning, index_offset, true);
         draw.start += step;
         ctx->batch->num_vertices += step;
      }
      /* changing this value is a hack, restore it */
      ctx->batch->num_vertices = num_vertices;
   } else {
      draw_impl(ctx, pinfo, pdraw, ctx->batch->draw, index_offset, false);
      draw_impl(ctx, pinfo, pdraw, ctx->batch->binning, index_offset, true);
   }

   fd_context_all_clean(ctx);

   ctx->batch->num_vertices += pdraw->count * pinfo->instance_count;

   return true;
}

static void
fd2_draw_vbos(struct fd_context *ctx, const struct pipe_draw_info *info,
              unsigned drawid_offset,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draws,
              unsigned num_draws,
              unsigned index_offset)
   assert_dt
{
   for (unsigned i = 0; i < num_draws; i++)
      fd2_draw_vbo(ctx, info, drawid_offset, indirect, &draws[i], index_offset);
}

static void
clear_state(struct fd_batch *batch, struct fd_ringbuffer *ring,
            unsigned buffers, bool fast_clear) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   struct fd2_context *fd2_ctx = fd2_context(ctx);
   uint32_t reg;

   fd2_emit_vertex_bufs(ring, 0x9c,
                        (struct fd2_vertex_buf[]){
                           {.prsc = fd2_ctx->solid_vertexbuf, .size = 36},
                        },
                        1);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_INDX_OFFSET));
   OUT_RING(ring, 0);

   fd2_program_emit(ctx, ring, &ctx->solid_prog);

   OUT_PKT0(ring, REG_A2XX_TC_CNTL_STATUS, 1);
   OUT_RING(ring, A2XX_TC_CNTL_STATUS_L2_INVALIDATE);

   if (buffers & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) {
      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_RB_DEPTHCONTROL));
      reg = 0;
      if (buffers & PIPE_CLEAR_DEPTH) {
         reg |= A2XX_RB_DEPTHCONTROL_ZFUNC(FUNC_ALWAYS) |
                A2XX_RB_DEPTHCONTROL_Z_ENABLE |
                A2XX_RB_DEPTHCONTROL_Z_WRITE_ENABLE |
                A2XX_RB_DEPTHCONTROL_EARLY_Z_ENABLE;
      }
      if (buffers & PIPE_CLEAR_STENCIL) {
         reg |= A2XX_RB_DEPTHCONTROL_STENCILFUNC(FUNC_ALWAYS) |
                A2XX_RB_DEPTHCONTROL_STENCIL_ENABLE |
                A2XX_RB_DEPTHCONTROL_STENCILZPASS(STENCIL_REPLACE);
      }
      OUT_RING(ring, reg);
   }

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COLORCONTROL));
   OUT_RING(ring, A2XX_RB_COLORCONTROL_ALPHA_FUNC(FUNC_ALWAYS) |
                     A2XX_RB_COLORCONTROL_BLEND_DISABLE |
                     A2XX_RB_COLORCONTROL_ROP_CODE(12) |
                     A2XX_RB_COLORCONTROL_DITHER_MODE(DITHER_DISABLE) |
                     A2XX_RB_COLORCONTROL_DITHER_TYPE(DITHER_PIXEL));

   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_CLIP_CNTL));
   OUT_RING(ring, 0x00000000); /* PA_CL_CLIP_CNTL */
   OUT_RING(
      ring,
      A2XX_PA_SU_SC_MODE_CNTL_PROVOKING_VTX_LAST | /* PA_SU_SC_MODE_CNTL */
         A2XX_PA_SU_SC_MODE_CNTL_FRONT_PTYPE(PC_DRAW_TRIANGLES) |
         A2XX_PA_SU_SC_MODE_CNTL_BACK_PTYPE(PC_DRAW_TRIANGLES) |
         (fast_clear ? A2XX_PA_SU_SC_MODE_CNTL_MSAA_ENABLE : 0));

   if (fast_clear) {
      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_AA_CONFIG));
      OUT_RING(ring, A2XX_PA_SC_AA_CONFIG_MSAA_NUM_SAMPLES(3));
   }

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_AA_MASK));
   OUT_RING(ring, 0x0000ffff);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COLOR_MASK));
   if (buffers & PIPE_CLEAR_COLOR) {
      OUT_RING(ring, A2XX_RB_COLOR_MASK_WRITE_RED |
                        A2XX_RB_COLOR_MASK_WRITE_GREEN |
                        A2XX_RB_COLOR_MASK_WRITE_BLUE |
                        A2XX_RB_COLOR_MASK_WRITE_ALPHA);
   } else {
      OUT_RING(ring, 0x0);
   }

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_BLEND_CONTROL));
   OUT_RING(ring, 0);

   if (is_a20x(batch->ctx->screen))
      return;

   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_MAX_VTX_INDX));
   OUT_RING(ring, 3); /* VGT_MAX_VTX_INDX */
   OUT_RING(ring, 0); /* VGT_MIN_VTX_INDX */

   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_STENCILREFMASK_BF));
   OUT_RING(ring,
            0xff000000 | A2XX_RB_STENCILREFMASK_BF_STENCILWRITEMASK(0xff));
   OUT_RING(ring, 0xff000000 | A2XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_A220_RB_LRZ_VSC_CONTROL));
   OUT_RING(ring, 0x00000084);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_VERTEX_REUSE_BLOCK_CNTL));
   OUT_RING(ring, 0x0000028f);
}

static void
clear_state_restore(struct fd_context *ctx, struct fd_ringbuffer *ring)
{
   if (is_a20x(ctx->screen))
      return;

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COPY_CONTROL));
   OUT_RING(ring, 0x00000000);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_A220_RB_LRZ_VSC_CONTROL));
   OUT_RING(ring, 0x00000000);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_VERTEX_REUSE_BLOCK_CNTL));
   OUT_RING(ring, 0x0000003b);
}

static void
clear_fast(struct fd_batch *batch, struct fd_ringbuffer *ring,
           uint32_t color_clear, uint32_t depth_clear, unsigned patch_type)
{
   BEGIN_RING(ring, 8); /* preallocate next 2 packets (for patching) */

   /* zero values are patched in */
   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_SCREEN_SCISSOR_BR));
   OUT_RINGP(ring, patch_type, &batch->gmem_patches);

   OUT_PKT3(ring, CP_SET_CONSTANT, 4);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_SURFACE_INFO));
   OUT_RING(ring, 0x8000 | 32);
   OUT_RING(ring, 0);
   OUT_RING(ring, 0);

   /* set fill values */
   if (!is_a20x(batch->ctx->screen)) {
      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_CLEAR_COLOR));
      OUT_RING(ring, color_clear);

      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_RB_COPY_CONTROL));
      OUT_RING(ring, A2XX_RB_COPY_CONTROL_DEPTH_CLEAR_ENABLE |
                        A2XX_RB_COPY_CONTROL_CLEAR_MASK(0xf));

      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_RB_DEPTH_CLEAR));
      OUT_RING(ring, depth_clear);
   } else {
      const float sc = 1.0f / 255.0f;

      OUT_PKT3(ring, CP_SET_CONSTANT, 5);
      OUT_RING(ring, 0x00000480);
      OUT_RING(ring, fui((float)(color_clear >> 0 & 0xff) * sc));
      OUT_RING(ring, fui((float)(color_clear >> 8 & 0xff) * sc));
      OUT_RING(ring, fui((float)(color_clear >> 16 & 0xff) * sc));
      OUT_RING(ring, fui((float)(color_clear >> 24 & 0xff) * sc));

      // XXX if using float the rounding error breaks it..
      float depth = ((double)(depth_clear >> 8)) * (1.0 / (double)0xffffff);
      assert((unsigned)(((double)depth * (double)0xffffff)) ==
             (depth_clear >> 8));

      OUT_PKT3(ring, CP_SET_CONSTANT, 3);
      OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_VPORT_ZSCALE));
      OUT_RING(ring, fui(0.0f));
      OUT_RING(ring, fui(depth));

      OUT_PKT3(ring, CP_SET_CONSTANT, 3);
      OUT_RING(ring, CP_REG(REG_A2XX_RB_STENCILREFMASK_BF));
      OUT_RING(ring,
               0xff000000 |
                  A2XX_RB_STENCILREFMASK_BF_STENCILREF(depth_clear & 0xff) |
                  A2XX_RB_STENCILREFMASK_BF_STENCILWRITEMASK(0xff));
      OUT_RING(ring, 0xff000000 |
                        A2XX_RB_STENCILREFMASK_STENCILREF(depth_clear & 0xff) |
                        A2XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));
   }

   fd_draw(batch, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
           DI_SRC_SEL_AUTO_INDEX, 3, 0, INDEX_SIZE_IGN, 0, 0, NULL);
}

static bool
fd2_clear_fast(struct fd_context *ctx, unsigned buffers,
               const union pipe_color_union *color, double depth,
               unsigned stencil) assert_dt
{
   /* using 4x MSAA allows clearing ~2x faster
    * then we can use higher bpp clearing to clear lower bpp
    * 1 "pixel" can clear 64 bits (rgba8+depth24+stencil8)
    * note: its possible to clear with 32_32_32_32 format but its not faster
    * note: fast clear doesn't work with sysmem rendering
    * (sysmem rendering is disabled when clear is used)
    *
    * we only have 16-bit / 32-bit color formats
    * and 16-bit / 32-bit depth formats
    * so there are only a few possible combinations
    *
    * if the bpp of the color/depth doesn't match
    * we clear with depth/color individually
    */
   struct fd2_context *fd2_ctx = fd2_context(ctx);
   struct fd_batch *batch = ctx->batch;
   struct fd_ringbuffer *ring = batch->draw;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   uint32_t color_clear = 0, depth_clear = 0;
   enum pipe_format format = pipe_surface_format(pfb->cbufs[0]);
   int depth_size = -1; /* -1: no clear, 0: clear 16-bit, 1: clear 32-bit */
   int color_size = -1;

   /* TODO: need to test performance on a22x */
   if (!is_a20x(ctx->screen))
      return false;

   if (buffers & PIPE_CLEAR_COLOR)
      color_size = util_format_get_blocksizebits(format) == 32;

   if (buffers & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) {
      /* no fast clear when clearing only one component of depth+stencil buffer */
      if (!(buffers & PIPE_CLEAR_DEPTH))
         return false;

      if ((pfb->zsbuf->format == PIPE_FORMAT_Z24_UNORM_S8_UINT ||
           pfb->zsbuf->format == PIPE_FORMAT_S8_UINT_Z24_UNORM) &&
          !(buffers & PIPE_CLEAR_STENCIL))
         return false;

      depth_size = fd_pipe2depth(pfb->zsbuf->format) == DEPTHX_24_8;
   }

   assert(color_size >= 0 || depth_size >= 0);

   if (color_size == 0) {
      color_clear = pack_rgba(format, color->f);
      color_clear = (color_clear << 16) | (color_clear & 0xffff);
   } else if (color_size == 1) {
      color_clear = pack_rgba(format, color->f);
   }

   if (depth_size == 0) {
      depth_clear = (uint32_t)(0xffff * depth);
      depth_clear |= depth_clear << 16;
   } else if (depth_size == 1) {
      depth_clear = (((uint32_t)(0xffffff * depth)) << 8);
      depth_clear |= (stencil & 0xff);
   }

   /* disable "window" scissor.. */
   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_WINDOW_SCISSOR_TL));
   OUT_RING(ring, xy2d(0, 0));
   OUT_RING(ring, xy2d(0x7fff, 0x7fff));

   /* make sure we fill all "pixels" (in SCREEN_SCISSOR) */
   OUT_PKT3(ring, CP_SET_CONSTANT, 5);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_VPORT_XSCALE));
   OUT_RING(ring, fui(4096.0f));
   OUT_RING(ring, fui(4096.0f));
   OUT_RING(ring, fui(4096.0f));
   OUT_RING(ring, fui(4096.0f));

   clear_state(batch, ring, ~0u, true);

   if (color_size >= 0 && depth_size != color_size)
      clear_fast(batch, ring, color_clear, color_clear,
                 GMEM_PATCH_FASTCLEAR_COLOR);

   if (depth_size >= 0 && depth_size != color_size)
      clear_fast(batch, ring, depth_clear, depth_clear,
                 GMEM_PATCH_FASTCLEAR_DEPTH);

   if (depth_size == color_size)
      clear_fast(batch, ring, color_clear, depth_clear,
                 GMEM_PATCH_FASTCLEAR_COLOR_DEPTH);

   clear_state_restore(ctx, ring);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_AA_CONFIG));
   OUT_RING(ring, 0);

   /* can't patch in SCREEN_SCISSOR_BR as it can be different for each tile.
    * MEM_WRITE the value in tile_renderprep, and use CP_LOAD_CONSTANT_CONTEXT
    * the value is read from byte offset 60 in the given bo
    */
   OUT_PKT3(ring, CP_LOAD_CONSTANT_CONTEXT, 3);
   OUT_RELOC(ring, fd_resource(fd2_ctx->solid_vertexbuf)->bo, 0, 0, 0);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_SCREEN_SCISSOR_BR));
   OUT_RING(ring, 1);

   OUT_PKT3(ring, CP_SET_CONSTANT, 4);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_SURFACE_INFO));
   OUT_RINGP(ring, GMEM_PATCH_RESTORE_INFO, &batch->gmem_patches);
   OUT_RING(ring, 0);
   OUT_RING(ring, 0);
   return true;
}

static bool
fd2_clear(struct fd_context *ctx, enum fd_buffer_mask buffers,
          const union pipe_color_union *color, double depth,
          unsigned stencil) assert_dt
{
   struct fd_ringbuffer *ring = ctx->batch->draw;
   struct pipe_framebuffer_state *fb = &ctx->batch->framebuffer;

   if (fd2_clear_fast(ctx, buffers, color, depth, stencil))
      goto dirty;

   /* set clear value */
   if (is_a20x(ctx->screen)) {
      if (buffers & FD_BUFFER_COLOR) {
         /* C0 used by fragment shader */
         OUT_PKT3(ring, CP_SET_CONSTANT, 5);
         OUT_RING(ring, 0x00000480);
         OUT_RING(ring, color->ui[0]);
         OUT_RING(ring, color->ui[1]);
         OUT_RING(ring, color->ui[2]);
         OUT_RING(ring, color->ui[3]);
      }

      if (buffers & FD_BUFFER_DEPTH) {
         /* use viewport to set depth value */
         OUT_PKT3(ring, CP_SET_CONSTANT, 3);
         OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_VPORT_ZSCALE));
         OUT_RING(ring, fui(0.0f));
         OUT_RING(ring, fui(depth));
      }

      if (buffers & FD_BUFFER_STENCIL) {
         OUT_PKT3(ring, CP_SET_CONSTANT, 3);
         OUT_RING(ring, CP_REG(REG_A2XX_RB_STENCILREFMASK_BF));
         OUT_RING(ring, 0xff000000 |
                           A2XX_RB_STENCILREFMASK_BF_STENCILREF(stencil) |
                           A2XX_RB_STENCILREFMASK_BF_STENCILWRITEMASK(0xff));
         OUT_RING(ring, 0xff000000 |
                           A2XX_RB_STENCILREFMASK_STENCILREF(stencil) |
                           A2XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));
      }
   } else {
      if (buffers & FD_BUFFER_COLOR) {
         OUT_PKT3(ring, CP_SET_CONSTANT, 2);
         OUT_RING(ring, CP_REG(REG_A2XX_CLEAR_COLOR));
         OUT_RING(ring, pack_rgba(PIPE_FORMAT_R8G8B8A8_UNORM, color->f));
      }

      if (buffers & (FD_BUFFER_DEPTH | FD_BUFFER_STENCIL)) {
         uint32_t clear_mask, depth_clear;
         switch (fd_pipe2depth(fb->zsbuf->format)) {
         case DEPTHX_24_8:
            clear_mask = ((buffers & FD_BUFFER_DEPTH) ? 0xe : 0) |
                         ((buffers & FD_BUFFER_STENCIL) ? 0x1 : 0);
            depth_clear =
               (((uint32_t)(0xffffff * depth)) << 8) | (stencil & 0xff);
            break;
         case DEPTHX_16:
            clear_mask = 0xf;
            depth_clear = (uint32_t)(0xffffffff * depth);
            break;
         default:
            unreachable("invalid depth");
            break;
         }

         OUT_PKT3(ring, CP_SET_CONSTANT, 2);
         OUT_RING(ring, CP_REG(REG_A2XX_RB_COPY_CONTROL));
         OUT_RING(ring, A2XX_RB_COPY_CONTROL_DEPTH_CLEAR_ENABLE |
                           A2XX_RB_COPY_CONTROL_CLEAR_MASK(clear_mask));

         OUT_PKT3(ring, CP_SET_CONSTANT, 2);
         OUT_RING(ring, CP_REG(REG_A2XX_RB_DEPTH_CLEAR));
         OUT_RING(ring, depth_clear);
      }
   }

   /* scissor state */
   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_WINDOW_SCISSOR_TL));
   OUT_RING(ring, xy2d(0, 0));
   OUT_RING(ring, xy2d(fb->width, fb->height));

   /* viewport state */
   OUT_PKT3(ring, CP_SET_CONSTANT, 5);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_VPORT_XSCALE));
   OUT_RING(ring, fui((float)fb->width / 2.0f));
   OUT_RING(ring, fui((float)fb->width / 2.0f));
   OUT_RING(ring, fui((float)fb->height / 2.0f));
   OUT_RING(ring, fui((float)fb->height / 2.0f));

   /* common state */
   clear_state(ctx->batch, ring, buffers, false);

   fd_draw(ctx->batch, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
           DI_SRC_SEL_AUTO_INDEX, 3, 0, INDEX_SIZE_IGN, 0, 0, NULL);

   clear_state_restore(ctx, ring);

dirty:
   ctx->dirty |= FD_DIRTY_ZSA | FD_DIRTY_VIEWPORT | FD_DIRTY_RASTERIZER |
                 FD_DIRTY_SAMPLE_MASK | FD_DIRTY_PROG | FD_DIRTY_CONST |
                 FD_DIRTY_BLEND | FD_DIRTY_FRAMEBUFFER | FD_DIRTY_SCISSOR;

   ctx->dirty_shader[PIPE_SHADER_VERTEX] |= FD_DIRTY_SHADER_PROG;
   ctx->dirty_shader[PIPE_SHADER_FRAGMENT] |=
      FD_DIRTY_SHADER_PROG | FD_DIRTY_SHADER_CONST;

   return true;
}

void
fd2_draw_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->draw_vbos = fd2_draw_vbos;
   ctx->clear = fd2_clear;
}
