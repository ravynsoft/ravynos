/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "freedreno_draw.h"
#include "freedreno_resource.h"
#include "freedreno_state.h"

#include "ir2/instr-a2xx.h"
#include "fd2_context.h"
#include "fd2_draw.h"
#include "fd2_emit.h"
#include "fd2_gmem.h"
#include "fd2_program.h"
#include "fd2_util.h"
#include "fd2_zsa.h"

static uint32_t
fmt2swap(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_B8G8R8X8_UNORM:
   case PIPE_FORMAT_B5G6R5_UNORM:
   case PIPE_FORMAT_B5G5R5A1_UNORM:
   case PIPE_FORMAT_B5G5R5X1_UNORM:
   case PIPE_FORMAT_B4G4R4A4_UNORM:
   case PIPE_FORMAT_B4G4R4X4_UNORM:
   case PIPE_FORMAT_B2G3R3_UNORM:
      return 1;
   default:
      return 0;
   }
}

static bool
use_hw_binning(struct fd_batch *batch)
{
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;

   /* we hardcoded a limit of 8 "pipes", we can increase this limit
    * at the cost of a slightly larger command stream
    * however very few cases will need more than 8
    * gmem->num_vsc_pipes == 0 means empty batch (TODO: does it still happen?)
    */
   if (gmem->num_vsc_pipes > 8 || !gmem->num_vsc_pipes)
      return false;

   /* only a20x hw binning is implement
    * a22x is more like a3xx, but perhaps the a20x works? (TODO)
    */
   if (!is_a20x(batch->ctx->screen))
      return false;

   return fd_binning_enabled && ((gmem->nbins_x * gmem->nbins_y) > 2);
}

/* transfer from gmem to system memory (ie. normal RAM) */

static void
emit_gmem2mem_surf(struct fd_batch *batch, uint32_t base,
                   struct pipe_surface *psurf)
{
   struct fd_ringbuffer *ring = batch->tile_store;
   struct fd_resource *rsc = fd_resource(psurf->texture);
   uint32_t offset =
      fd_resource_offset(rsc, psurf->u.tex.level, psurf->u.tex.first_layer);
   enum pipe_format format = fd_gmem_restore_format(psurf->format);
   uint32_t pitch = fdl2_pitch_pixels(&rsc->layout, psurf->u.tex.level);

   assert((pitch & 31) == 0);
   assert((offset & 0xfff) == 0);

   if (!rsc->valid)
      return;

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COLOR_INFO));
   OUT_RING(ring, A2XX_RB_COLOR_INFO_BASE(base) |
                     A2XX_RB_COLOR_INFO_FORMAT(fd2_pipe2color(format)));

   OUT_PKT3(ring, CP_SET_CONSTANT, 5);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COPY_CONTROL));
   OUT_RING(ring, 0x00000000);             /* RB_COPY_CONTROL */
   OUT_RELOC(ring, rsc->bo, offset, 0, 0); /* RB_COPY_DEST_BASE */
   OUT_RING(ring, pitch >> 5);             /* RB_COPY_DEST_PITCH */
   OUT_RING(ring,                          /* RB_COPY_DEST_INFO */
            A2XX_RB_COPY_DEST_INFO_FORMAT(fd2_pipe2color(format)) |
               COND(!rsc->layout.tile_mode, A2XX_RB_COPY_DEST_INFO_LINEAR) |
               A2XX_RB_COPY_DEST_INFO_WRITE_RED |
               A2XX_RB_COPY_DEST_INFO_WRITE_GREEN |
               A2XX_RB_COPY_DEST_INFO_WRITE_BLUE |
               A2XX_RB_COPY_DEST_INFO_WRITE_ALPHA);

   if (!is_a20x(batch->ctx->screen)) {
      OUT_WFI(ring);

      OUT_PKT3(ring, CP_SET_CONSTANT, 3);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_MAX_VTX_INDX));
      OUT_RING(ring, 3); /* VGT_MAX_VTX_INDX */
      OUT_RING(ring, 0); /* VGT_MIN_VTX_INDX */
   }

   fd_draw(batch, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
           DI_SRC_SEL_AUTO_INDEX, 3, 0, INDEX_SIZE_IGN, 0, 0, NULL);
}

static void
prepare_tile_fini_ib(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   struct fd2_context *fd2_ctx = fd2_context(ctx);
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd_ringbuffer *ring;

   batch->tile_store =
      fd_submit_new_ringbuffer(batch->submit, 0x1000, FD_RINGBUFFER_STREAMING);
   ring = batch->tile_store;

   fd2_emit_vertex_bufs(ring, 0x9c,
                        (struct fd2_vertex_buf[]){
                           {.prsc = fd2_ctx->solid_vertexbuf, .size = 36},
                        },
                        1);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_WINDOW_OFFSET));
   OUT_RING(ring, 0x00000000); /* PA_SC_WINDOW_OFFSET */

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_INDX_OFFSET));
   OUT_RING(ring, 0);

   if (!is_a20x(ctx->screen)) {
      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_VERTEX_REUSE_BLOCK_CNTL));
      OUT_RING(ring, 0x0000028f);
   }

   fd2_program_emit(ctx, ring, &ctx->solid_prog);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_AA_MASK));
   OUT_RING(ring, 0x0000ffff);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_DEPTHCONTROL));
   OUT_RING(ring, A2XX_RB_DEPTHCONTROL_EARLY_Z_ENABLE);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SU_SC_MODE_CNTL));
   OUT_RING(
      ring,
      A2XX_PA_SU_SC_MODE_CNTL_PROVOKING_VTX_LAST | /* PA_SU_SC_MODE_CNTL */
         A2XX_PA_SU_SC_MODE_CNTL_FRONT_PTYPE(PC_DRAW_TRIANGLES) |
         A2XX_PA_SU_SC_MODE_CNTL_BACK_PTYPE(PC_DRAW_TRIANGLES));

   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_WINDOW_SCISSOR_TL));
   OUT_RING(ring, xy2d(0, 0));                    /* PA_SC_WINDOW_SCISSOR_TL */
   OUT_RING(ring, xy2d(pfb->width, pfb->height)); /* PA_SC_WINDOW_SCISSOR_BR */

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_CLIP_CNTL));
   OUT_RING(ring, 0x00000000);

   OUT_PKT3(ring, CP_SET_CONSTANT, 5);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_VPORT_XSCALE));
   OUT_RING(ring, fui((float)gmem->bin_w / 2.0f)); /* XSCALE */
   OUT_RING(ring, fui((float)gmem->bin_w / 2.0f)); /* XOFFSET */
   OUT_RING(ring, fui((float)gmem->bin_h / 2.0f)); /* YSCALE */
   OUT_RING(ring, fui((float)gmem->bin_h / 2.0f)); /* YOFFSET */

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_MODECONTROL));
   OUT_RING(ring, A2XX_RB_MODECONTROL_EDRAM_MODE(EDRAM_COPY));

   if (batch->resolve & (FD_BUFFER_DEPTH | FD_BUFFER_STENCIL))
      emit_gmem2mem_surf(batch, gmem->zsbuf_base[0], pfb->zsbuf);

   if (batch->resolve & FD_BUFFER_COLOR)
      emit_gmem2mem_surf(batch, gmem->cbuf_base[0], pfb->cbufs[0]);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_MODECONTROL));
   OUT_RING(ring, A2XX_RB_MODECONTROL_EDRAM_MODE(COLOR_DEPTH));

   if (!is_a20x(ctx->screen)) {
      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_VERTEX_REUSE_BLOCK_CNTL));
      OUT_RING(ring, 0x0000003b);
   }
}

static void
fd2_emit_tile_gmem2mem(struct fd_batch *batch, const struct fd_tile *tile)
{
   fd2_emit_ib(batch->gmem, batch->tile_store);
}

/* transfer from system memory to gmem */

static void
emit_mem2gmem_surf(struct fd_batch *batch, uint32_t base,
                   struct pipe_surface *psurf)
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct fd_resource *rsc = fd_resource(psurf->texture);
   uint32_t offset =
      fd_resource_offset(rsc, psurf->u.tex.level, psurf->u.tex.first_layer);
   enum pipe_format format = fd_gmem_restore_format(psurf->format);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COLOR_INFO));
   OUT_RING(ring, A2XX_RB_COLOR_INFO_BASE(base) |
                     A2XX_RB_COLOR_INFO_FORMAT(fd2_pipe2color(format)));

   /* emit fb as a texture: */
   OUT_PKT3(ring, CP_SET_CONSTANT, 7);
   OUT_RING(ring, 0x00010000);
   OUT_RING(ring, A2XX_SQ_TEX_0_CLAMP_X(SQ_TEX_WRAP) |
                     A2XX_SQ_TEX_0_CLAMP_Y(SQ_TEX_WRAP) |
                     A2XX_SQ_TEX_0_CLAMP_Z(SQ_TEX_WRAP) |
                     A2XX_SQ_TEX_0_PITCH(
                        fdl2_pitch_pixels(&rsc->layout, psurf->u.tex.level)));
   OUT_RELOC(ring, rsc->bo, offset,
             A2XX_SQ_TEX_1_FORMAT(fd2_pipe2surface(format).format) |
                A2XX_SQ_TEX_1_CLAMP_POLICY(SQ_TEX_CLAMP_POLICY_OGL),
             0);
   OUT_RING(ring, A2XX_SQ_TEX_2_WIDTH(psurf->width - 1) |
                     A2XX_SQ_TEX_2_HEIGHT(psurf->height - 1));
   OUT_RING(ring, A2XX_SQ_TEX_3_MIP_FILTER(SQ_TEX_FILTER_BASEMAP) |
                     A2XX_SQ_TEX_3_SWIZ_X(0) | A2XX_SQ_TEX_3_SWIZ_Y(1) |
                     A2XX_SQ_TEX_3_SWIZ_Z(2) | A2XX_SQ_TEX_3_SWIZ_W(3) |
                     A2XX_SQ_TEX_3_XY_MAG_FILTER(SQ_TEX_FILTER_POINT) |
                     A2XX_SQ_TEX_3_XY_MIN_FILTER(SQ_TEX_FILTER_POINT));
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, A2XX_SQ_TEX_5_DIMENSION(SQ_TEX_DIMENSION_2D));

   if (!is_a20x(batch->ctx->screen)) {
      OUT_PKT3(ring, CP_SET_CONSTANT, 3);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_MAX_VTX_INDX));
      OUT_RING(ring, 3); /* VGT_MAX_VTX_INDX */
      OUT_RING(ring, 0); /* VGT_MIN_VTX_INDX */
   }

   fd_draw(batch, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
           DI_SRC_SEL_AUTO_INDEX, 3, 0, INDEX_SIZE_IGN, 0, 0, NULL);
}

static void
fd2_emit_tile_mem2gmem(struct fd_batch *batch,
                       const struct fd_tile *tile) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   struct fd2_context *fd2_ctx = fd2_context(ctx);
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   unsigned bin_w = tile->bin_w;
   unsigned bin_h = tile->bin_h;
   float x0, y0, x1, y1;

   fd2_emit_vertex_bufs(
      ring, 0x9c,
      (struct fd2_vertex_buf[]){
         {.prsc = fd2_ctx->solid_vertexbuf, .size = 36},
         {.prsc = fd2_ctx->solid_vertexbuf, .size = 24, .offset = 36},
      },
      2);

   /* write texture coordinates to vertexbuf: */
   x0 = ((float)tile->xoff) / ((float)pfb->width);
   x1 = ((float)tile->xoff + bin_w) / ((float)pfb->width);
   y0 = ((float)tile->yoff) / ((float)pfb->height);
   y1 = ((float)tile->yoff + bin_h) / ((float)pfb->height);
   OUT_PKT3(ring, CP_MEM_WRITE, 7);
   OUT_RELOC(ring, fd_resource(fd2_ctx->solid_vertexbuf)->bo, 36, 0, 0);
   OUT_RING(ring, fui(x0));
   OUT_RING(ring, fui(y0));
   OUT_RING(ring, fui(x1));
   OUT_RING(ring, fui(y0));
   OUT_RING(ring, fui(x0));
   OUT_RING(ring, fui(y1));

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_INDX_OFFSET));
   OUT_RING(ring, 0);

   fd2_program_emit(ctx, ring, &ctx->blit_prog[0]);

   OUT_PKT0(ring, REG_A2XX_TC_CNTL_STATUS, 1);
   OUT_RING(ring, A2XX_TC_CNTL_STATUS_L2_INVALIDATE);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_DEPTHCONTROL));
   OUT_RING(ring, A2XX_RB_DEPTHCONTROL_EARLY_Z_ENABLE);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SU_SC_MODE_CNTL));
   OUT_RING(ring, A2XX_PA_SU_SC_MODE_CNTL_PROVOKING_VTX_LAST |
                     A2XX_PA_SU_SC_MODE_CNTL_FRONT_PTYPE(PC_DRAW_TRIANGLES) |
                     A2XX_PA_SU_SC_MODE_CNTL_BACK_PTYPE(PC_DRAW_TRIANGLES));

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_AA_MASK));
   OUT_RING(ring, 0x0000ffff);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COLORCONTROL));
   OUT_RING(ring, A2XX_RB_COLORCONTROL_ALPHA_FUNC(FUNC_ALWAYS) |
                     A2XX_RB_COLORCONTROL_BLEND_DISABLE |
                     A2XX_RB_COLORCONTROL_ROP_CODE(12) |
                     A2XX_RB_COLORCONTROL_DITHER_MODE(DITHER_DISABLE) |
                     A2XX_RB_COLORCONTROL_DITHER_TYPE(DITHER_PIXEL));

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_BLEND_CONTROL));
   OUT_RING(ring, A2XX_RB_BLEND_CONTROL_COLOR_SRCBLEND(FACTOR_ONE) |
                     A2XX_RB_BLEND_CONTROL_COLOR_COMB_FCN(BLEND2_DST_PLUS_SRC) |
                     A2XX_RB_BLEND_CONTROL_COLOR_DESTBLEND(FACTOR_ZERO) |
                     A2XX_RB_BLEND_CONTROL_ALPHA_SRCBLEND(FACTOR_ONE) |
                     A2XX_RB_BLEND_CONTROL_ALPHA_COMB_FCN(BLEND2_DST_PLUS_SRC) |
                     A2XX_RB_BLEND_CONTROL_ALPHA_DESTBLEND(FACTOR_ZERO));

   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_WINDOW_SCISSOR_TL));
   OUT_RING(ring, A2XX_PA_SC_WINDOW_OFFSET_DISABLE |
                     xy2d(0, 0));      /* PA_SC_WINDOW_SCISSOR_TL */
   OUT_RING(ring, xy2d(bin_w, bin_h)); /* PA_SC_WINDOW_SCISSOR_BR */

   OUT_PKT3(ring, CP_SET_CONSTANT, 5);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_VPORT_XSCALE));
   OUT_RING(ring, fui((float)bin_w / 2.0f));  /* PA_CL_VPORT_XSCALE */
   OUT_RING(ring, fui((float)bin_w / 2.0f));  /* PA_CL_VPORT_XOFFSET */
   OUT_RING(ring, fui(-(float)bin_h / 2.0f)); /* PA_CL_VPORT_YSCALE */
   OUT_RING(ring, fui((float)bin_h / 2.0f));  /* PA_CL_VPORT_YOFFSET */

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_VTE_CNTL));
   OUT_RING(ring, A2XX_PA_CL_VTE_CNTL_VTX_XY_FMT |
                     A2XX_PA_CL_VTE_CNTL_VTX_Z_FMT | // XXX check this???
                     A2XX_PA_CL_VTE_CNTL_VPORT_X_SCALE_ENA |
                     A2XX_PA_CL_VTE_CNTL_VPORT_X_OFFSET_ENA |
                     A2XX_PA_CL_VTE_CNTL_VPORT_Y_SCALE_ENA |
                     A2XX_PA_CL_VTE_CNTL_VPORT_Y_OFFSET_ENA);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_CLIP_CNTL));
   OUT_RING(ring, 0x00000000);

   if (fd_gmem_needs_restore(batch, tile, FD_BUFFER_DEPTH | FD_BUFFER_STENCIL))
      emit_mem2gmem_surf(batch, gmem->zsbuf_base[0], pfb->zsbuf);

   if (fd_gmem_needs_restore(batch, tile, FD_BUFFER_COLOR))
      emit_mem2gmem_surf(batch, gmem->cbuf_base[0], pfb->cbufs[0]);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_CL_VTE_CNTL));
   OUT_RING(ring, A2XX_PA_CL_VTE_CNTL_VTX_W0_FMT |
                     A2XX_PA_CL_VTE_CNTL_VPORT_X_SCALE_ENA |
                     A2XX_PA_CL_VTE_CNTL_VPORT_X_OFFSET_ENA |
                     A2XX_PA_CL_VTE_CNTL_VPORT_Y_SCALE_ENA |
                     A2XX_PA_CL_VTE_CNTL_VPORT_Y_OFFSET_ENA |
                     A2XX_PA_CL_VTE_CNTL_VPORT_Z_SCALE_ENA |
                     A2XX_PA_CL_VTE_CNTL_VPORT_Z_OFFSET_ENA);

   /* TODO blob driver seems to toss in a CACHE_FLUSH after each DRAW_INDX.. */
}

static void
patch_draws(struct fd_batch *batch, enum pc_di_vis_cull_mode vismode)
{
   unsigned i;

   if (!is_a20x(batch->ctx->screen)) {
      /* identical to a3xx */
      for (i = 0; i < fd_patch_num_elements(&batch->draw_patches); i++) {
         struct fd_cs_patch *patch = fd_patch_element(&batch->draw_patches, i);
         *patch->cs = patch->val | DRAW(0, 0, 0, vismode, 0);
      }
      util_dynarray_clear(&batch->draw_patches);
      return;
   }

   if (vismode == USE_VISIBILITY)
      return;

   for (i = 0; i < batch->draw_patches.size / sizeof(uint32_t *); i++) {
      uint32_t *ptr =
         *util_dynarray_element(&batch->draw_patches, uint32_t *, i);
      unsigned cnt = ptr[0] >> 16 & 0xfff; /* 5 with idx buffer, 3 without */

      /* convert CP_DRAW_INDX_BIN to a CP_DRAW_INDX
       * replace first two DWORDS with NOP and move the rest down
       * (we don't want to have to move the idx buffer reloc)
       */
      ptr[0] = CP_TYPE3_PKT | (CP_NOP << 8);
      ptr[1] = 0x00000000;

      ptr[4] = ptr[2] & ~(1 << 14 | 1 << 15); /* remove cull_enable bits */
      ptr[2] = CP_TYPE3_PKT | ((cnt - 2) << 16) | (CP_DRAW_INDX << 8);
      ptr[3] = 0x00000000;
   }
}

static void
fd2_emit_sysmem_prep(struct fd_batch *batch)
{
   struct fd_context *ctx = batch->ctx;
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct pipe_surface *psurf = pfb->cbufs[0];

   if (!psurf)
      return;

   struct fd_resource *rsc = fd_resource(psurf->texture);
   uint32_t offset =
      fd_resource_offset(rsc, psurf->u.tex.level, psurf->u.tex.first_layer);
   uint32_t pitch = fdl2_pitch_pixels(&rsc->layout, psurf->u.tex.level);

   assert((pitch & 31) == 0);
   assert((offset & 0xfff) == 0);

   fd2_emit_restore(ctx, ring);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_SURFACE_INFO));
   OUT_RING(ring, A2XX_RB_SURFACE_INFO_SURFACE_PITCH(pitch));

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COLOR_INFO));
   OUT_RELOC(ring, rsc->bo, offset,
             COND(!rsc->layout.tile_mode, A2XX_RB_COLOR_INFO_LINEAR) |
                A2XX_RB_COLOR_INFO_SWAP(fmt2swap(psurf->format)) |
                A2XX_RB_COLOR_INFO_FORMAT(fd2_pipe2color(psurf->format)),
             0);

   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_SCREEN_SCISSOR_TL));
   OUT_RING(ring, A2XX_PA_SC_SCREEN_SCISSOR_TL_WINDOW_OFFSET_DISABLE);
   OUT_RING(ring, A2XX_PA_SC_SCREEN_SCISSOR_BR_X(pfb->width) |
                     A2XX_PA_SC_SCREEN_SCISSOR_BR_Y(pfb->height));

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_WINDOW_OFFSET));
   OUT_RING(ring,
            A2XX_PA_SC_WINDOW_OFFSET_X(0) | A2XX_PA_SC_WINDOW_OFFSET_Y(0));

   patch_draws(batch, IGNORE_VISIBILITY);
   util_dynarray_clear(&batch->draw_patches);
   util_dynarray_clear(&batch->shader_patches);
}

/* before first tile */
static void
fd2_emit_tile_init(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   enum pipe_format format = pipe_surface_format(pfb->cbufs[0]);
   uint32_t reg;

   fd2_emit_restore(ctx, ring);

   prepare_tile_fini_ib(batch);

   OUT_PKT3(ring, CP_SET_CONSTANT, 4);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_SURFACE_INFO));
   OUT_RING(ring, gmem->bin_w); /* RB_SURFACE_INFO */
   OUT_RING(ring, A2XX_RB_COLOR_INFO_SWAP(fmt2swap(format)) |
                     A2XX_RB_COLOR_INFO_FORMAT(fd2_pipe2color(format)));
   reg = A2XX_RB_DEPTH_INFO_DEPTH_BASE(gmem->zsbuf_base[0]);
   if (pfb->zsbuf)
      reg |= A2XX_RB_DEPTH_INFO_DEPTH_FORMAT(fd_pipe2depth(pfb->zsbuf->format));
   OUT_RING(ring, reg); /* RB_DEPTH_INFO */

   /* fast clear patches */
   int depth_size = -1;
   int color_size = -1;

   if (pfb->cbufs[0])
      color_size = util_format_get_blocksizebits(format) == 32 ? 4 : 2;

   if (pfb->zsbuf)
      depth_size = fd_pipe2depth(pfb->zsbuf->format) == 1 ? 4 : 2;

   for (int i = 0; i < fd_patch_num_elements(&batch->gmem_patches); i++) {
      struct fd_cs_patch *patch = fd_patch_element(&batch->gmem_patches, i);
      uint32_t color_base = 0, depth_base = gmem->zsbuf_base[0];
      uint32_t size, lines;

      /* note: 1 "line" is 512 bytes in both color/depth areas (1K total) */
      switch (patch->val) {
      case GMEM_PATCH_FASTCLEAR_COLOR:
         size = align(gmem->bin_w * gmem->bin_h * color_size, 0x8000);
         lines = size / 1024;
         depth_base = size / 2;
         break;
      case GMEM_PATCH_FASTCLEAR_DEPTH:
         size = align(gmem->bin_w * gmem->bin_h * depth_size, 0x8000);
         lines = size / 1024;
         color_base = depth_base;
         depth_base = depth_base + size / 2;
         break;
      case GMEM_PATCH_FASTCLEAR_COLOR_DEPTH:
         lines =
            align(gmem->bin_w * gmem->bin_h * color_size * 2, 0x8000) / 1024;
         break;
      case GMEM_PATCH_RESTORE_INFO:
         patch->cs[0] = gmem->bin_w;
         patch->cs[1] = A2XX_RB_COLOR_INFO_SWAP(fmt2swap(format)) |
                        A2XX_RB_COLOR_INFO_FORMAT(fd2_pipe2color(format));
         patch->cs[2] = A2XX_RB_DEPTH_INFO_DEPTH_BASE(gmem->zsbuf_base[0]);
         if (pfb->zsbuf)
            patch->cs[2] |= A2XX_RB_DEPTH_INFO_DEPTH_FORMAT(
               fd_pipe2depth(pfb->zsbuf->format));
         continue;
      default:
         continue;
      }

      patch->cs[0] = A2XX_PA_SC_SCREEN_SCISSOR_BR_X(32) |
                     A2XX_PA_SC_SCREEN_SCISSOR_BR_Y(lines);
      patch->cs[4] = A2XX_RB_COLOR_INFO_BASE(color_base) |
                     A2XX_RB_COLOR_INFO_FORMAT(COLORX_8_8_8_8);
      patch->cs[5] = A2XX_RB_DEPTH_INFO_DEPTH_BASE(depth_base) |
                     A2XX_RB_DEPTH_INFO_DEPTH_FORMAT(1);
   }
   util_dynarray_clear(&batch->gmem_patches);

   /* set to zero, for some reason hardware doesn't like certain values */
   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_CURRENT_BIN_ID_MIN));
   OUT_RING(ring, 0);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_VGT_CURRENT_BIN_ID_MAX));
   OUT_RING(ring, 0);

   if (use_hw_binning(batch)) {
      /* patch out unneeded memory exports by changing EXEC CF to EXEC_END
       *
       * in the shader compiler, we guarantee that the shader ends with
       * a specific pattern of ALLOC/EXEC CF pairs for the hw binning exports
       *
       * the since patches point only to dwords and CFs are 1.5 dwords
       * the patch is aligned and might point to a ALLOC CF
       */
      for (int i = 0; i < batch->shader_patches.size / sizeof(void *); i++) {
         instr_cf_t *cf =
            *util_dynarray_element(&batch->shader_patches, instr_cf_t *, i);
         if (cf->opc == ALLOC)
            cf++;
         assert(cf->opc == EXEC);
         assert(cf[ctx->screen->info->num_vsc_pipes * 2 - 2].opc == EXEC_END);
         cf[2 * (gmem->num_vsc_pipes - 1)].opc = EXEC_END;
      }

      patch_draws(batch, USE_VISIBILITY);

      /* initialize shader constants for the binning memexport */
      OUT_PKT3(ring, CP_SET_CONSTANT, 1 + gmem->num_vsc_pipes * 4);
      OUT_RING(ring, 0x0000000C);

      for (int i = 0; i < gmem->num_vsc_pipes; i++) {
         /* allocate in 64k increments to avoid reallocs */
         uint32_t bo_size = align(batch->num_vertices, 0x10000);
         if (!ctx->vsc_pipe_bo[i] ||
             fd_bo_size(ctx->vsc_pipe_bo[i]) < bo_size) {
            if (ctx->vsc_pipe_bo[i])
               fd_bo_del(ctx->vsc_pipe_bo[i]);
            ctx->vsc_pipe_bo[i] =
               fd_bo_new(ctx->dev, bo_size, 0, "vsc_pipe[%u]", i);
            assert(ctx->vsc_pipe_bo[i]);
         }

         /* memory export address (export32):
          * .x: (base_address >> 2) | 0x40000000 (?)
          * .y: index (float) - set by shader
          * .z: 0x4B00D000 (?)
          * .w: 0x4B000000 (?) | max_index (?)
          */
         OUT_RELOC(ring, ctx->vsc_pipe_bo[i], 0, 0x40000000, -2);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x4B00D000);
         OUT_RING(ring, 0x4B000000 | bo_size);
      }

      OUT_PKT3(ring, CP_SET_CONSTANT, 1 + gmem->num_vsc_pipes * 8);
      OUT_RING(ring, 0x0000018C);

      for (int i = 0; i < gmem->num_vsc_pipes; i++) {
         const struct fd_vsc_pipe *pipe = &gmem->vsc_pipe[i];
         float off_x, off_y, mul_x, mul_y;

         /* const to tranform from [-1,1] to bin coordinates for this pipe
          * for x/y, [0,256/2040] = 0, [256/2040,512/2040] = 1, etc
          * 8 possible values on x/y axis,
          * to clip at binning stage: only use center 6x6
          * TODO: set the z parameters too so that hw binning
          * can clip primitives in Z too
          */

         mul_x = 1.0f / (float)(gmem->bin_w * 8);
         mul_y = 1.0f / (float)(gmem->bin_h * 8);
         off_x = -pipe->x * (1.0f / 8.0f) + 0.125f - mul_x * gmem->minx;
         off_y = -pipe->y * (1.0f / 8.0f) + 0.125f - mul_y * gmem->miny;

         OUT_RING(ring, fui(off_x * (256.0f / 255.0f)));
         OUT_RING(ring, fui(off_y * (256.0f / 255.0f)));
         OUT_RING(ring, 0x3f000000);
         OUT_RING(ring, fui(0.0f));

         OUT_RING(ring, fui(mul_x * (256.0f / 255.0f)));
         OUT_RING(ring, fui(mul_y * (256.0f / 255.0f)));
         OUT_RING(ring, fui(0.0f));
         OUT_RING(ring, fui(0.0f));
      }

      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_VERTEX_REUSE_BLOCK_CNTL));
      OUT_RING(ring, 0);

      fd2_emit_ib(ring, batch->binning);

      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_VERTEX_REUSE_BLOCK_CNTL));
      OUT_RING(ring, 0x00000002);
   } else {
      patch_draws(batch, IGNORE_VISIBILITY);
   }

   util_dynarray_clear(&batch->draw_patches);
   util_dynarray_clear(&batch->shader_patches);
}

/* before mem2gmem */
static void
fd2_emit_tile_prep(struct fd_batch *batch, const struct fd_tile *tile)
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   enum pipe_format format = pipe_surface_format(pfb->cbufs[0]);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COLOR_INFO));
   OUT_RING(ring, A2XX_RB_COLOR_INFO_SWAP(1) | /* RB_COLOR_INFO */
                     A2XX_RB_COLOR_INFO_FORMAT(fd2_pipe2color(format)));

   /* setup screen scissor for current tile (same for mem2gmem): */
   OUT_PKT3(ring, CP_SET_CONSTANT, 3);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_SCREEN_SCISSOR_TL));
   OUT_RING(ring, A2XX_PA_SC_SCREEN_SCISSOR_TL_X(0) |
                     A2XX_PA_SC_SCREEN_SCISSOR_TL_Y(0));
   OUT_RING(ring, A2XX_PA_SC_SCREEN_SCISSOR_BR_X(tile->bin_w) |
                     A2XX_PA_SC_SCREEN_SCISSOR_BR_Y(tile->bin_h));
}

/* before IB to rendering cmds: */
static void
fd2_emit_tile_renderprep(struct fd_batch *batch,
                         const struct fd_tile *tile) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   struct fd2_context *fd2_ctx = fd2_context(ctx);
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   enum pipe_format format = pipe_surface_format(pfb->cbufs[0]);

   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COLOR_INFO));
   OUT_RING(ring, A2XX_RB_COLOR_INFO_SWAP(fmt2swap(format)) |
                     A2XX_RB_COLOR_INFO_FORMAT(fd2_pipe2color(format)));

   /* setup window scissor and offset for current tile (different
    * from mem2gmem):
    */
   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_PA_SC_WINDOW_OFFSET));
   OUT_RING(ring, A2XX_PA_SC_WINDOW_OFFSET_X(-tile->xoff) |
                     A2XX_PA_SC_WINDOW_OFFSET_Y(-tile->yoff));

   /* write SCISSOR_BR to memory so fast clear path can restore from it */
   OUT_PKT3(ring, CP_MEM_WRITE, 2);
   OUT_RELOC(ring, fd_resource(fd2_ctx->solid_vertexbuf)->bo, 60, 0, 0);
   OUT_RING(ring, A2XX_PA_SC_SCREEN_SCISSOR_BR_X(tile->bin_w) |
                     A2XX_PA_SC_SCREEN_SCISSOR_BR_Y(tile->bin_h));

   /* set the copy offset for gmem2mem */
   OUT_PKT3(ring, CP_SET_CONSTANT, 2);
   OUT_RING(ring, CP_REG(REG_A2XX_RB_COPY_DEST_OFFSET));
   OUT_RING(ring, A2XX_RB_COPY_DEST_OFFSET_X(tile->xoff) |
                     A2XX_RB_COPY_DEST_OFFSET_Y(tile->yoff));

   /* tile offset for gl_FragCoord on a20x (C64 in fragment shader) */
   if (is_a20x(ctx->screen)) {
      OUT_PKT3(ring, CP_SET_CONSTANT, 5);
      OUT_RING(ring, 0x00000580);
      OUT_RING(ring, fui(tile->xoff));
      OUT_RING(ring, fui(tile->yoff));
      OUT_RING(ring, fui(0.0f));
      OUT_RING(ring, fui(0.0f));
   }

   if (use_hw_binning(batch)) {
      struct fd_bo *pipe_bo = ctx->vsc_pipe_bo[tile->p];

      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_CURRENT_BIN_ID_MIN));
      OUT_RING(ring, tile->n);

      OUT_PKT3(ring, CP_SET_CONSTANT, 2);
      OUT_RING(ring, CP_REG(REG_A2XX_VGT_CURRENT_BIN_ID_MAX));
      OUT_RING(ring, tile->n);

      /* TODO only emit this when tile->p changes */
      OUT_PKT3(ring, CP_SET_DRAW_INIT_FLAGS, 1);
      OUT_RELOC(ring, pipe_bo, 0, 0, 0);
   }
}

void
fd2_gmem_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);

   ctx->emit_sysmem_prep = fd2_emit_sysmem_prep;
   ctx->emit_tile_init = fd2_emit_tile_init;
   ctx->emit_tile_prep = fd2_emit_tile_prep;
   ctx->emit_tile_mem2gmem = fd2_emit_tile_mem2gmem;
   ctx->emit_tile_renderprep = fd2_emit_tile_renderprep;
   ctx->emit_tile_gmem2mem = fd2_emit_tile_gmem2mem;
}
