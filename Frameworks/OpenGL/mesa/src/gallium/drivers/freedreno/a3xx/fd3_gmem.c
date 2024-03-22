/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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

#include "fd3_context.h"
#include "fd3_emit.h"
#include "fd3_format.h"
#include "fd3_gmem.h"
#include "fd3_program.h"
#include "fd3_zsa.h"

static void
fd3_gmem_emit_set_prog(struct fd_context *ctx, struct fd3_emit *emit,
                       struct fd_program_stateobj *prog)
{
   emit->skip_consts = true;
   emit->key.vs = prog->vs;
   emit->key.fs = prog->fs;
   emit->prog = fd3_program_state(
      ir3_cache_lookup(ctx->shader_cache, &emit->key, &ctx->debug));
   /* reset the fd3_emit_get_*p cache */
   emit->vs = NULL;
   emit->fs = NULL;
}

static void
emit_mrt(struct fd_ringbuffer *ring, unsigned nr_bufs,
         struct pipe_surface **bufs, const uint32_t *bases, uint32_t bin_w,
         bool decode_srgb)
{
   enum a3xx_tile_mode tile_mode;
   unsigned i;

   for (i = 0; i < A3XX_MAX_RENDER_TARGETS; i++) {
      enum pipe_format pformat = 0;
      enum a3xx_color_fmt format = 0;
      enum a3xx_color_swap swap = WZYX;
      bool srgb = false;
      struct fd_resource *rsc = NULL;
      uint32_t stride = 0;
      uint32_t base = 0;
      uint32_t offset = 0;

      if (bin_w) {
         tile_mode = TILE_32X32;
      } else {
         tile_mode = LINEAR;
      }

      if ((i < nr_bufs) && bufs[i]) {
         struct pipe_surface *psurf = bufs[i];

         rsc = fd_resource(psurf->texture);
         pformat = psurf->format;
         /* In case we're drawing to Z32F_S8, the "color" actually goes to
          * the stencil
          */
         if (rsc->stencil) {
            rsc = rsc->stencil;
            pformat = rsc->b.b.format;
            if (bases)
               bases++;
         }
         format = fd3_pipe2color(pformat);
         if (decode_srgb)
            srgb = util_format_is_srgb(pformat);
         else
            pformat = util_format_linear(pformat);

         assert(psurf->u.tex.first_layer == psurf->u.tex.last_layer);

         offset = fd_resource_offset(rsc, psurf->u.tex.level,
                                     psurf->u.tex.first_layer);
         swap = rsc->layout.tile_mode ? WZYX : fd3_pipe2swap(pformat);

         if (bin_w) {
            stride = bin_w << fdl_cpp_shift(&rsc->layout);

            if (bases) {
               base = bases[i];
            }
         } else {
            stride = fd_resource_pitch(rsc, psurf->u.tex.level);
            tile_mode = rsc->layout.tile_mode;
         }
      } else if (i < nr_bufs && bases) {
         base = bases[i];
      }

      OUT_PKT0(ring, REG_A3XX_RB_MRT_BUF_INFO(i), 2);
      OUT_RING(ring, A3XX_RB_MRT_BUF_INFO_COLOR_FORMAT(format) |
                        A3XX_RB_MRT_BUF_INFO_COLOR_TILE_MODE(tile_mode) |
                        A3XX_RB_MRT_BUF_INFO_COLOR_BUF_PITCH(stride) |
                        A3XX_RB_MRT_BUF_INFO_COLOR_SWAP(swap) |
                        COND(srgb, A3XX_RB_MRT_BUF_INFO_COLOR_SRGB));
      if (bin_w || (i >= nr_bufs) || !bufs[i]) {
         OUT_RING(ring, A3XX_RB_MRT_BUF_BASE_COLOR_BUF_BASE(base));
      } else {
         OUT_RELOC(ring, rsc->bo, offset, 0, -1);
      }

      OUT_PKT0(ring, REG_A3XX_SP_FS_IMAGE_OUTPUT_REG(i), 1);
      OUT_RING(ring, COND((i < nr_bufs) && bufs[i],
                          A3XX_SP_FS_IMAGE_OUTPUT_REG_MRTFORMAT(
                             fd3_fs_output_format(pformat))));
   }
}

static bool
use_hw_binning(struct fd_batch *batch)
{
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;

   /* workaround: combining scissor optimization and hw binning
    * seems problematic.  Seems like we end up with a mismatch
    * between binning pass and rendering pass, wrt. where the hw
    * thinks the vertices belong.  And the blob driver doesn't
    * seem to implement anything like scissor optimization, so
    * not entirely sure what I might be missing.
    *
    * But scissor optimization is mainly for window managers,
    * which don't have many vertices (and therefore doesn't
    * benefit much from binning pass).
    *
    * So for now just disable binning if scissor optimization is
    * used.
    */
   if (gmem->minx || gmem->miny)
      return false;

   if ((gmem->maxpw * gmem->maxph) > 32)
      return false;

   if ((gmem->maxpw > 15) || (gmem->maxph > 15))
      return false;

   return fd_binning_enabled && ((gmem->nbins_x * gmem->nbins_y) > 2);
}

/* workaround for (hlsq?) lockup with hw binning on a3xx patchlevel 0 */
static void update_vsc_pipe(struct fd_batch *batch);
static void
emit_binning_workaround(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct fd_ringbuffer *ring = batch->gmem;
   struct fd3_emit emit = {
      .debug = &ctx->debug,
      .vtx = &ctx->solid_vbuf_state,
      .key =
         {
            .vs = ctx->solid_prog.vs,
            .fs = ctx->solid_prog.fs,
         },
   };

   fd3_gmem_emit_set_prog(ctx, &emit, &ctx->solid_prog);

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 2);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_RESOLVE_PASS) |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(0));
   OUT_RING(ring, A3XX_RB_RENDER_CONTROL_BIN_WIDTH(32) |
                     A3XX_RB_RENDER_CONTROL_DISABLE_COLOR_PIPE |
                     A3XX_RB_RENDER_CONTROL_ALPHA_TEST_FUNC(FUNC_NEVER));

   OUT_PKT0(ring, REG_A3XX_RB_COPY_CONTROL, 4);
   OUT_RING(ring, A3XX_RB_COPY_CONTROL_MSAA_RESOLVE(MSAA_ONE) |
                     A3XX_RB_COPY_CONTROL_MODE(0) |
                     A3XX_RB_COPY_CONTROL_GMEM_BASE(0));
   OUT_RELOC(ring, fd_resource(ctx->solid_vbuf)->bo, 0x20, 0,
             -1); /* RB_COPY_DEST_BASE */
   OUT_RING(ring, A3XX_RB_COPY_DEST_PITCH_PITCH(128));
   OUT_RING(ring, A3XX_RB_COPY_DEST_INFO_TILE(LINEAR) |
                     A3XX_RB_COPY_DEST_INFO_FORMAT(RB_R8G8B8A8_UNORM) |
                     A3XX_RB_COPY_DEST_INFO_SWAP(WZYX) |
                     A3XX_RB_COPY_DEST_INFO_COMPONENT_ENABLE(0xf) |
                     A3XX_RB_COPY_DEST_INFO_ENDIAN(ENDIAN_NONE));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RESOLVE_PASS) |
                     A3XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A3XX_GRAS_SC_CONTROL_RASTER_MODE(1));

   fd3_program_emit(ring, &emit, 0, NULL);
   fd3_emit_vertex_bufs(ring, &emit);

   OUT_PKT0(ring, REG_A3XX_HLSQ_CONTROL_0_REG, 4);
   OUT_RING(ring, A3XX_HLSQ_CONTROL_0_REG_FSTHREADSIZE(FOUR_QUADS) |
                     A3XX_HLSQ_CONTROL_0_REG_FSSUPERTHREADENABLE |
                     A3XX_HLSQ_CONTROL_0_REG_RESERVED2 |
                     A3XX_HLSQ_CONTROL_0_REG_SPCONSTFULLUPDATE);
   OUT_RING(ring, A3XX_HLSQ_CONTROL_1_REG_VSTHREADSIZE(TWO_QUADS) |
                     A3XX_HLSQ_CONTROL_1_REG_VSSUPERTHREADENABLE);
   OUT_RING(ring, A3XX_HLSQ_CONTROL_2_REG_PRIMALLOCTHRESHOLD(31));
   OUT_RING(ring, 0); /* HLSQ_CONTROL_3_REG */

   OUT_PKT0(ring, REG_A3XX_HLSQ_CONST_FSPRESV_RANGE_REG, 1);
   OUT_RING(ring, A3XX_HLSQ_CONST_FSPRESV_RANGE_REG_STARTENTRY(0x20) |
                     A3XX_HLSQ_CONST_FSPRESV_RANGE_REG_ENDENTRY(0x20));

   OUT_PKT0(ring, REG_A3XX_RB_MSAA_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_MSAA_CONTROL_DISABLE |
                     A3XX_RB_MSAA_CONTROL_SAMPLES(MSAA_ONE) |
                     A3XX_RB_MSAA_CONTROL_SAMPLE_MASK(0xffff));

   OUT_PKT0(ring, REG_A3XX_RB_DEPTH_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_NEVER));

   OUT_PKT0(ring, REG_A3XX_RB_STENCIL_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_STENCIL_CONTROL_FUNC(FUNC_NEVER) |
                     A3XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_NEVER) |
                     A3XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));

   OUT_PKT0(ring, REG_A3XX_GRAS_SU_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SU_MODE_CONTROL_LINEHALFWIDTH(0.0f));

   OUT_PKT0(ring, REG_A3XX_VFD_INDEX_MIN, 4);
   OUT_RING(ring, 0); /* VFD_INDEX_MIN */
   OUT_RING(ring, 2); /* VFD_INDEX_MAX */
   OUT_RING(ring, 0); /* VFD_INSTANCEID_OFFSET */
   OUT_RING(ring, 0); /* VFD_INDEX_OFFSET */

   OUT_PKT0(ring, REG_A3XX_PC_PRIM_VTX_CNTL, 1);
   OUT_RING(ring,
            A3XX_PC_PRIM_VTX_CNTL_STRIDE_IN_VPC(0) |
               A3XX_PC_PRIM_VTX_CNTL_POLYMODE_FRONT_PTYPE(PC_DRAW_TRIANGLES) |
               A3XX_PC_PRIM_VTX_CNTL_POLYMODE_BACK_PTYPE(PC_DRAW_TRIANGLES) |
               A3XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST);

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_WINDOW_SCISSOR_TL, 2);
   OUT_RING(ring, A3XX_GRAS_SC_WINDOW_SCISSOR_TL_X(0) |
                     A3XX_GRAS_SC_WINDOW_SCISSOR_TL_Y(1));
   OUT_RING(ring, A3XX_GRAS_SC_WINDOW_SCISSOR_BR_X(0) |
                     A3XX_GRAS_SC_WINDOW_SCISSOR_BR_Y(1));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_TL_X(0) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(0));
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_BR_X(31) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(0));

   fd_wfi(batch, ring);
   OUT_PKT0(ring, REG_A3XX_GRAS_CL_VPORT_XOFFSET, 6);
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_XOFFSET(0.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_XSCALE(1.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_YOFFSET(0.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_YSCALE(1.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_ZOFFSET(0.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_ZSCALE(1.0f));

   OUT_PKT0(ring, REG_A3XX_GRAS_CL_CLIP_CNTL, 1);
   OUT_RING(ring, A3XX_GRAS_CL_CLIP_CNTL_CLIP_DISABLE |
                     A3XX_GRAS_CL_CLIP_CNTL_ZFAR_CLIP_DISABLE |
                     A3XX_GRAS_CL_CLIP_CNTL_VP_CLIP_CODE_IGNORE |
                     A3XX_GRAS_CL_CLIP_CNTL_VP_XFORM_DISABLE |
                     A3XX_GRAS_CL_CLIP_CNTL_PERSP_DIVISION_DISABLE);

   OUT_PKT0(ring, REG_A3XX_GRAS_CL_GB_CLIP_ADJ, 1);
   OUT_RING(ring, A3XX_GRAS_CL_GB_CLIP_ADJ_HORZ(0) |
                     A3XX_GRAS_CL_GB_CLIP_ADJ_VERT(0));

   OUT_PKT3(ring, CP_DRAW_INDX_2, 5);
   OUT_RING(ring, 0x00000000); /* viz query info. */
   OUT_RING(ring, DRAW(DI_PT_RECTLIST, DI_SRC_SEL_IMMEDIATE, INDEX_SIZE_32_BIT,
                       IGNORE_VISIBILITY, 0));
   OUT_RING(ring, 2); /* NumIndices */
   OUT_RING(ring, 2);
   OUT_RING(ring, 1);
   fd_reset_wfi(batch);

   OUT_PKT0(ring, REG_A3XX_HLSQ_CONTROL_0_REG, 1);
   OUT_RING(ring, A3XX_HLSQ_CONTROL_0_REG_FSTHREADSIZE(TWO_QUADS));

   OUT_PKT0(ring, REG_A3XX_VFD_PERFCOUNTER0_SELECT, 1);
   OUT_RING(ring, 0x00000000);

   fd_wfi(batch, ring);
   OUT_PKT0(ring, REG_A3XX_VSC_BIN_SIZE, 1);
   OUT_RING(ring, A3XX_VSC_BIN_SIZE_WIDTH(gmem->bin_w) |
                     A3XX_VSC_BIN_SIZE_HEIGHT(gmem->bin_h));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A3XX_GRAS_SC_CONTROL_RASTER_MODE(0));

   OUT_PKT0(ring, REG_A3XX_GRAS_CL_CLIP_CNTL, 1);
   OUT_RING(ring, 0x00000000);
}

/* transfer from gmem to system memory (ie. normal RAM) */

static void
emit_gmem2mem_surf(struct fd_batch *batch,
                   enum adreno_rb_copy_control_mode mode, bool stencil,
                   uint32_t base, struct pipe_surface *psurf)
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct fd_resource *rsc = fd_resource(psurf->texture);
   enum pipe_format format = psurf->format;

   if (!rsc->valid)
      return;

   if (stencil) {
      rsc = rsc->stencil;
      format = rsc->b.b.format;
   }

   uint32_t offset =
      fd_resource_offset(rsc, psurf->u.tex.level, psurf->u.tex.first_layer);
   uint32_t pitch = fd_resource_pitch(rsc, psurf->u.tex.level);

   assert(psurf->u.tex.first_layer == psurf->u.tex.last_layer);

   OUT_PKT0(ring, REG_A3XX_RB_COPY_CONTROL, 4);
   OUT_RING(ring, A3XX_RB_COPY_CONTROL_MSAA_RESOLVE(MSAA_ONE) |
                     A3XX_RB_COPY_CONTROL_MODE(mode) |
                     A3XX_RB_COPY_CONTROL_GMEM_BASE(base) |
                     COND(format == PIPE_FORMAT_Z32_FLOAT ||
                             format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT,
                          A3XX_RB_COPY_CONTROL_DEPTH32_RESOLVE));

   OUT_RELOC(ring, rsc->bo, offset, 0, -1); /* RB_COPY_DEST_BASE */
   OUT_RING(ring, A3XX_RB_COPY_DEST_PITCH_PITCH(pitch));
   OUT_RING(ring, A3XX_RB_COPY_DEST_INFO_TILE(rsc->layout.tile_mode) |
                     A3XX_RB_COPY_DEST_INFO_FORMAT(fd3_pipe2color(format)) |
                     A3XX_RB_COPY_DEST_INFO_COMPONENT_ENABLE(0xf) |
                     A3XX_RB_COPY_DEST_INFO_ENDIAN(ENDIAN_NONE) |
                     A3XX_RB_COPY_DEST_INFO_SWAP(fd3_pipe2swap(format)));

   fd_draw(batch, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
           DI_SRC_SEL_AUTO_INDEX, 2, 0, INDEX_SIZE_IGN, 0, 0, NULL);
}

static void
fd3_emit_tile_gmem2mem(struct fd_batch *batch,
                       const struct fd_tile *tile) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   struct fd_ringbuffer *ring = batch->gmem;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd3_emit emit = {.debug = &ctx->debug,
                           .vtx = &ctx->solid_vbuf_state,
                           .key = {
                              .vs = ctx->solid_prog.vs,
                              .fs = ctx->solid_prog.fs,
                           }};
   int i;

   emit.prog = fd3_program_state(
      ir3_cache_lookup(ctx->shader_cache, &emit.key, &ctx->debug));

   OUT_PKT0(ring, REG_A3XX_RB_DEPTH_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_NEVER));

   OUT_PKT0(ring, REG_A3XX_RB_STENCIL_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_STENCIL_CONTROL_FUNC(FUNC_NEVER) |
                     A3XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_NEVER) |
                     A3XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));

   OUT_PKT0(ring, REG_A3XX_RB_STENCILREFMASK, 2);
   OUT_RING(ring, 0xff000000 | A3XX_RB_STENCILREFMASK_STENCILREF(0) |
                     A3XX_RB_STENCILREFMASK_STENCILMASK(0) |
                     A3XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));
   OUT_RING(ring, 0xff000000 | A3XX_RB_STENCILREFMASK_STENCILREF(0) |
                     A3XX_RB_STENCILREFMASK_STENCILMASK(0) |
                     A3XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));

   OUT_PKT0(ring, REG_A3XX_GRAS_SU_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SU_MODE_CONTROL_LINEHALFWIDTH(0));

   OUT_PKT0(ring, REG_A3XX_GRAS_CL_CLIP_CNTL, 1);
   OUT_RING(ring, 0x00000000); /* GRAS_CL_CLIP_CNTL */

   fd_wfi(batch, ring);
   OUT_PKT0(ring, REG_A3XX_GRAS_CL_VPORT_XOFFSET, 6);
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_XOFFSET((float)pfb->width / 2.0f - 0.5f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_XSCALE((float)pfb->width / 2.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_YOFFSET((float)pfb->height / 2.0f - 0.5f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_YSCALE(-(float)pfb->height / 2.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_ZOFFSET(0.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_ZSCALE(1.0f));

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_RESOLVE_PASS) |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(0));

   OUT_PKT0(ring, REG_A3XX_RB_RENDER_CONTROL, 1);
   OUT_RING(ring,
            A3XX_RB_RENDER_CONTROL_DISABLE_COLOR_PIPE |
               A3XX_RB_RENDER_CONTROL_ENABLE_GMEM |
               A3XX_RB_RENDER_CONTROL_ALPHA_TEST_FUNC(FUNC_NEVER) |
               A3XX_RB_RENDER_CONTROL_BIN_WIDTH(batch->gmem_state->bin_w));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RESOLVE_PASS) |
                     A3XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A3XX_GRAS_SC_CONTROL_RASTER_MODE(1));

   OUT_PKT0(ring, REG_A3XX_PC_PRIM_VTX_CNTL, 1);
   OUT_RING(ring,
            A3XX_PC_PRIM_VTX_CNTL_STRIDE_IN_VPC(0) |
               A3XX_PC_PRIM_VTX_CNTL_POLYMODE_FRONT_PTYPE(PC_DRAW_TRIANGLES) |
               A3XX_PC_PRIM_VTX_CNTL_POLYMODE_BACK_PTYPE(PC_DRAW_TRIANGLES) |
               A3XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST);

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_WINDOW_SCISSOR_TL, 2);
   OUT_RING(ring, A3XX_GRAS_SC_WINDOW_SCISSOR_TL_X(0) |
                     A3XX_GRAS_SC_WINDOW_SCISSOR_TL_Y(0));
   OUT_RING(ring, A3XX_GRAS_SC_WINDOW_SCISSOR_BR_X(pfb->width - 1) |
                     A3XX_GRAS_SC_WINDOW_SCISSOR_BR_Y(pfb->height - 1));

   OUT_PKT0(ring, REG_A3XX_VFD_INDEX_MIN, 4);
   OUT_RING(ring, 0); /* VFD_INDEX_MIN */
   OUT_RING(ring, 2); /* VFD_INDEX_MAX */
   OUT_RING(ring, 0); /* VFD_INSTANCEID_OFFSET */
   OUT_RING(ring, 0); /* VFD_INDEX_OFFSET */

   fd3_program_emit(ring, &emit, 0, NULL);
   fd3_emit_vertex_bufs(ring, &emit);

   if (batch->resolve & (FD_BUFFER_DEPTH | FD_BUFFER_STENCIL)) {
      struct fd_resource *rsc = fd_resource(pfb->zsbuf->texture);
      if (!rsc->stencil || batch->resolve & FD_BUFFER_DEPTH)
         emit_gmem2mem_surf(batch, RB_COPY_DEPTH_STENCIL, false,
                            gmem->zsbuf_base[0], pfb->zsbuf);
      if (rsc->stencil && batch->resolve & FD_BUFFER_STENCIL)
         emit_gmem2mem_surf(batch, RB_COPY_DEPTH_STENCIL, true,
                            gmem->zsbuf_base[1], pfb->zsbuf);
   }

   if (batch->resolve & FD_BUFFER_COLOR) {
      for (i = 0; i < pfb->nr_cbufs; i++) {
         if (!pfb->cbufs[i])
            continue;
         if (!(batch->resolve & (PIPE_CLEAR_COLOR0 << i)))
            continue;
         emit_gmem2mem_surf(batch, RB_COPY_RESOLVE, false, gmem->cbuf_base[i],
                            pfb->cbufs[i]);
      }
   }

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(MAX2(1, pfb->nr_cbufs) - 1));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A3XX_GRAS_SC_CONTROL_RASTER_MODE(0));
}

/* transfer from system memory to gmem */

static void
emit_mem2gmem_surf(struct fd_batch *batch, const uint32_t bases[],
                   struct pipe_surface **psurf, uint32_t bufs, uint32_t bin_w)
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_surface *zsbufs[2];

   assert(bufs > 0);

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(bufs - 1));

   emit_mrt(ring, bufs, psurf, bases, bin_w, false);

   if (psurf[0] && (psurf[0]->format == PIPE_FORMAT_Z32_FLOAT ||
                    psurf[0]->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)) {
      /* Depth is stored as unorm in gmem, so we have to write it in using a
       * special blit shader which writes depth.
       */
      OUT_PKT0(ring, REG_A3XX_RB_DEPTH_CONTROL, 1);
      OUT_RING(ring, (A3XX_RB_DEPTH_CONTROL_FRAG_WRITES_Z |
                      A3XX_RB_DEPTH_CONTROL_Z_WRITE_ENABLE |
                      A3XX_RB_DEPTH_CONTROL_Z_TEST_ENABLE |
                      A3XX_RB_DEPTH_CONTROL_EARLY_Z_DISABLE |
                      A3XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_ALWAYS)));

      OUT_PKT0(ring, REG_A3XX_RB_DEPTH_INFO, 2);
      OUT_RING(ring, A3XX_RB_DEPTH_INFO_DEPTH_BASE(bases[0]) |
                        A3XX_RB_DEPTH_INFO_DEPTH_FORMAT(DEPTHX_32));
      OUT_RING(ring, A3XX_RB_DEPTH_PITCH(4 * batch->gmem_state->bin_w));

      if (psurf[0]->format == PIPE_FORMAT_Z32_FLOAT) {
         OUT_PKT0(ring, REG_A3XX_RB_MRT_CONTROL(0), 1);
         OUT_RING(ring, 0);
      } else {
         /* The gmem_restore_tex logic will put the first buffer's stencil
          * as color. Supply it with the proper information to make that
          * happen.
          */
         zsbufs[0] = zsbufs[1] = psurf[0];
         psurf = zsbufs;
         bufs = 2;
      }
   } else {
      OUT_PKT0(ring, REG_A3XX_SP_FS_OUTPUT_REG, 1);
      OUT_RING(ring, A3XX_SP_FS_OUTPUT_REG_MRT(bufs - 1));
   }

   fd3_emit_gmem_restore_tex(ring, psurf, bufs);

   fd_draw(batch, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
           DI_SRC_SEL_AUTO_INDEX, 2, 0, INDEX_SIZE_IGN, 0, 0, NULL);
}

static void
fd3_emit_tile_mem2gmem(struct fd_batch *batch,
                       const struct fd_tile *tile) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd3_emit emit = {
      .debug = &ctx->debug,
      .vtx = &ctx->blit_vbuf_state,
      .sprite_coord_enable = 1,
   };
   /* NOTE: They all use the same VP, this is for vtx bufs. */
   fd3_gmem_emit_set_prog(ctx, &emit, &ctx->blit_prog[0]);

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

   fd3_emit_cache_flush(batch, ring);

   for (i = 0; i < 4; i++) {
      OUT_PKT0(ring, REG_A3XX_RB_MRT_CONTROL(i), 1);
      OUT_RING(ring, A3XX_RB_MRT_CONTROL_ROP_CODE(ROP_COPY) |
                        A3XX_RB_MRT_CONTROL_DITHER_MODE(DITHER_DISABLE) |
                        A3XX_RB_MRT_CONTROL_COMPONENT_ENABLE(0xf));

      OUT_PKT0(ring, REG_A3XX_RB_MRT_BLEND_CONTROL(i), 1);
      OUT_RING(
         ring,
         A3XX_RB_MRT_BLEND_CONTROL_RGB_SRC_FACTOR(FACTOR_ONE) |
            A3XX_RB_MRT_BLEND_CONTROL_RGB_BLEND_OPCODE(BLEND_DST_PLUS_SRC) |
            A3XX_RB_MRT_BLEND_CONTROL_RGB_DEST_FACTOR(FACTOR_ZERO) |
            A3XX_RB_MRT_BLEND_CONTROL_ALPHA_SRC_FACTOR(FACTOR_ONE) |
            A3XX_RB_MRT_BLEND_CONTROL_ALPHA_BLEND_OPCODE(BLEND_DST_PLUS_SRC) |
            A3XX_RB_MRT_BLEND_CONTROL_ALPHA_DEST_FACTOR(FACTOR_ZERO));
   }

   OUT_PKT0(ring, REG_A3XX_RB_RENDER_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_RENDER_CONTROL_ALPHA_TEST_FUNC(FUNC_ALWAYS) |
                     A3XX_RB_RENDER_CONTROL_BIN_WIDTH(gmem->bin_w));

   fd_wfi(batch, ring);
   OUT_PKT0(ring, REG_A3XX_RB_DEPTH_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_LESS));

   OUT_PKT0(ring, REG_A3XX_RB_DEPTH_INFO, 2);
   OUT_RING(ring, 0);
   OUT_RING(ring, 0);

   OUT_PKT0(ring, REG_A3XX_GRAS_CL_CLIP_CNTL, 1);
   OUT_RING(ring,
            A3XX_GRAS_CL_CLIP_CNTL_IJ_PERSP_CENTER); /* GRAS_CL_CLIP_CNTL */

   fd_wfi(batch, ring);
   OUT_PKT0(ring, REG_A3XX_GRAS_CL_VPORT_XOFFSET, 6);
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_XOFFSET((float)bin_w / 2.0f - 0.5f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_XSCALE((float)bin_w / 2.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_YOFFSET((float)bin_h / 2.0f - 0.5f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_YSCALE(-(float)bin_h / 2.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_ZOFFSET(0.0f));
   OUT_RING(ring, A3XX_GRAS_CL_VPORT_ZSCALE(1.0f));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_WINDOW_SCISSOR_TL, 2);
   OUT_RING(ring, A3XX_GRAS_SC_WINDOW_SCISSOR_TL_X(0) |
                     A3XX_GRAS_SC_WINDOW_SCISSOR_TL_Y(0));
   OUT_RING(ring, A3XX_GRAS_SC_WINDOW_SCISSOR_BR_X(bin_w - 1) |
                     A3XX_GRAS_SC_WINDOW_SCISSOR_BR_Y(bin_h - 1));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_TL_X(0) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(0));
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_BR_X(bin_w - 1) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(bin_h - 1));

   OUT_PKT0(ring, REG_A3XX_RB_STENCIL_CONTROL, 1);
   OUT_RING(ring, 0x2 | A3XX_RB_STENCIL_CONTROL_FUNC(FUNC_ALWAYS) |
                     A3XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_ALWAYS) |
                     A3XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
                     A3XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));

   OUT_PKT0(ring, REG_A3XX_RB_STENCIL_INFO, 2);
   OUT_RING(ring, 0); /* RB_STENCIL_INFO */
   OUT_RING(ring, 0); /* RB_STENCIL_PITCH */

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A3XX_GRAS_SC_CONTROL_RASTER_MODE(1));

   OUT_PKT0(ring, REG_A3XX_PC_PRIM_VTX_CNTL, 1);
   OUT_RING(ring,
            A3XX_PC_PRIM_VTX_CNTL_STRIDE_IN_VPC(2) |
               A3XX_PC_PRIM_VTX_CNTL_POLYMODE_FRONT_PTYPE(PC_DRAW_TRIANGLES) |
               A3XX_PC_PRIM_VTX_CNTL_POLYMODE_BACK_PTYPE(PC_DRAW_TRIANGLES) |
               A3XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST);

   OUT_PKT0(ring, REG_A3XX_VFD_INDEX_MIN, 4);
   OUT_RING(ring, 0); /* VFD_INDEX_MIN */
   OUT_RING(ring, 2); /* VFD_INDEX_MAX */
   OUT_RING(ring, 0); /* VFD_INSTANCEID_OFFSET */
   OUT_RING(ring, 0); /* VFD_INDEX_OFFSET */

   fd3_emit_vertex_bufs(ring, &emit);

   /* for gmem pitch/base calculations, we need to use the non-
    * truncated tile sizes:
    */
   bin_w = gmem->bin_w;
   bin_h = gmem->bin_h;

   if (fd_gmem_needs_restore(batch, tile, FD_BUFFER_COLOR)) {
      fd3_gmem_emit_set_prog(ctx, &emit, &ctx->blit_prog[pfb->nr_cbufs - 1]);
      fd3_program_emit(ring, &emit, pfb->nr_cbufs, pfb->cbufs);
      emit_mem2gmem_surf(batch, gmem->cbuf_base, pfb->cbufs, pfb->nr_cbufs,
                         bin_w);
   }

   if (fd_gmem_needs_restore(batch, tile,
                             FD_BUFFER_DEPTH | FD_BUFFER_STENCIL)) {
      if (pfb->zsbuf->format != PIPE_FORMAT_Z32_FLOAT_S8X24_UINT &&
          pfb->zsbuf->format != PIPE_FORMAT_Z32_FLOAT) {
         /* Non-float can use a regular color write. It's split over 8-bit
          * components, so half precision is always sufficient.
          */
         fd3_gmem_emit_set_prog(ctx, &emit, &ctx->blit_prog[0]);
      } else {
         /* Float depth needs special blit shader that writes depth */
         if (pfb->zsbuf->format == PIPE_FORMAT_Z32_FLOAT)
            fd3_gmem_emit_set_prog(ctx, &emit, &ctx->blit_z);
         else
            fd3_gmem_emit_set_prog(ctx, &emit, &ctx->blit_zs);
      }
      fd3_program_emit(ring, &emit, 1, &pfb->zsbuf);
      emit_mem2gmem_surf(batch, gmem->zsbuf_base, &pfb->zsbuf, 1, bin_w);
   }

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A3XX_GRAS_SC_CONTROL_RASTER_MODE(0));

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(MAX2(1, pfb->nr_cbufs) - 1));
}

static void
patch_draws(struct fd_batch *batch, enum pc_di_vis_cull_mode vismode)
{
   unsigned i;
   for (i = 0; i < fd_patch_num_elements(&batch->draw_patches); i++) {
      struct fd_cs_patch *patch = fd_patch_element(&batch->draw_patches, i);
      *patch->cs = patch->val | DRAW(0, 0, 0, vismode, 0);
   }
   util_dynarray_clear(&batch->draw_patches);
}

static void
patch_rbrc(struct fd_batch *batch, uint32_t val)
{
   unsigned i;
   for (i = 0; i < fd_patch_num_elements(&batch->rbrc_patches); i++) {
      struct fd_cs_patch *patch = fd_patch_element(&batch->rbrc_patches, i);
      *patch->cs = patch->val | val;
   }
   util_dynarray_clear(&batch->rbrc_patches);
}

/* for rendering directly to system memory: */
static void
fd3_emit_sysmem_prep(struct fd_batch *batch) assert_dt
{
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd_ringbuffer *ring = batch->gmem;
   uint32_t i, pitch = 0;

   for (i = 0; i < pfb->nr_cbufs; i++) {
      struct pipe_surface *psurf = pfb->cbufs[i];
      if (!psurf)
         continue;
      struct fd_resource *rsc = fd_resource(psurf->texture);
      pitch = fd_resource_pitch(rsc, psurf->u.tex.level) / rsc->layout.cpp;
   }

   fd3_emit_restore(batch, ring);

   OUT_PKT0(ring, REG_A3XX_RB_FRAME_BUFFER_DIMENSION, 1);
   OUT_RING(ring, A3XX_RB_FRAME_BUFFER_DIMENSION_WIDTH(pfb->width) |
                     A3XX_RB_FRAME_BUFFER_DIMENSION_HEIGHT(pfb->height));

   emit_mrt(ring, pfb->nr_cbufs, pfb->cbufs, NULL, 0, true);

   /* setup scissor/offset for current tile: */
   OUT_PKT0(ring, REG_A3XX_RB_WINDOW_OFFSET, 1);
   OUT_RING(ring, A3XX_RB_WINDOW_OFFSET_X(0) | A3XX_RB_WINDOW_OFFSET_Y(0));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_TL_X(0) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(0));
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_BR_X(pfb->width - 1) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(pfb->height - 1));

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_RB_MODE_CONTROL_GMEM_BYPASS |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(MAX2(1, pfb->nr_cbufs) - 1));

   patch_draws(batch, IGNORE_VISIBILITY);
   patch_rbrc(batch, A3XX_RB_RENDER_CONTROL_BIN_WIDTH(pitch));
}

static void
update_vsc_pipe(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct fd3_context *fd3_ctx = fd3_context(ctx);
   struct fd_ringbuffer *ring = batch->gmem;
   int i;

   OUT_PKT0(ring, REG_A3XX_VSC_SIZE_ADDRESS, 1);
   OUT_RELOC(ring, fd3_ctx->vsc_size_mem, 0, 0, 0); /* VSC_SIZE_ADDRESS */

   for (i = 0; i < 8; i++) {
      const struct fd_vsc_pipe *pipe = &gmem->vsc_pipe[i];

      if (!ctx->vsc_pipe_bo[i]) {
         ctx->vsc_pipe_bo[i] = fd_bo_new(
            ctx->dev, 0x40000, 0, "vsc_pipe[%u]", i);
      }

      OUT_PKT0(ring, REG_A3XX_VSC_PIPE(i), 3);
      OUT_RING(ring, A3XX_VSC_PIPE_CONFIG_X(pipe->x) |
                        A3XX_VSC_PIPE_CONFIG_Y(pipe->y) |
                        A3XX_VSC_PIPE_CONFIG_W(pipe->w) |
                        A3XX_VSC_PIPE_CONFIG_H(pipe->h));
      OUT_RELOC(ring, ctx->vsc_pipe_bo[i], 0, 0,
                0); /* VSC_PIPE[i].DATA_ADDRESS */
      OUT_RING(ring, fd_bo_size(ctx->vsc_pipe_bo[i]) -
                        32); /* VSC_PIPE[i].DATA_LENGTH */
   }
}

static void
emit_binning_pass(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   struct fd_ringbuffer *ring = batch->gmem;
   int i;

   uint32_t x1 = gmem->minx;
   uint32_t y1 = gmem->miny;
   uint32_t x2 = gmem->minx + gmem->width - 1;
   uint32_t y2 = gmem->miny + gmem->height - 1;

   if (ctx->screen->gpu_id == 320) {
      emit_binning_workaround(batch);
      fd_wfi(batch, ring);
      OUT_PKT3(ring, CP_INVALIDATE_STATE, 1);
      OUT_RING(ring, 0x00007fff);
   }

   OUT_PKT0(ring, REG_A3XX_VSC_BIN_CONTROL, 1);
   OUT_RING(ring, A3XX_VSC_BIN_CONTROL_BINNING_ENABLE);

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SC_CONTROL_RENDER_MODE(RB_TILING_PASS) |
                     A3XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A3XX_GRAS_SC_CONTROL_RASTER_MODE(0));

   OUT_PKT0(ring, REG_A3XX_RB_FRAME_BUFFER_DIMENSION, 1);
   OUT_RING(ring, A3XX_RB_FRAME_BUFFER_DIMENSION_WIDTH(pfb->width) |
                     A3XX_RB_FRAME_BUFFER_DIMENSION_HEIGHT(pfb->height));

   OUT_PKT0(ring, REG_A3XX_RB_RENDER_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_RENDER_CONTROL_ALPHA_TEST_FUNC(FUNC_NEVER) |
                     A3XX_RB_RENDER_CONTROL_DISABLE_COLOR_PIPE |
                     A3XX_RB_RENDER_CONTROL_BIN_WIDTH(gmem->bin_w));

   /* setup scissor/offset for whole screen: */
   OUT_PKT0(ring, REG_A3XX_RB_WINDOW_OFFSET, 1);
   OUT_RING(ring, A3XX_RB_WINDOW_OFFSET_X(x1) | A3XX_RB_WINDOW_OFFSET_Y(y1));

   OUT_PKT0(ring, REG_A3XX_RB_LRZ_VSC_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_LRZ_VSC_CONTROL_BINNING_ENABLE);

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_TL_X(x1) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(y1));
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_BR_X(x2) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(y2));

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_TILING_PASS) |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(0));

   for (i = 0; i < 4; i++) {
      OUT_PKT0(ring, REG_A3XX_RB_MRT_CONTROL(i), 1);
      OUT_RING(ring, A3XX_RB_MRT_CONTROL_ROP_CODE(ROP_CLEAR) |
                        A3XX_RB_MRT_CONTROL_DITHER_MODE(DITHER_DISABLE) |
                        A3XX_RB_MRT_CONTROL_COMPONENT_ENABLE(0));
   }

   OUT_PKT0(ring, REG_A3XX_PC_VSTREAM_CONTROL, 1);
   OUT_RING(ring,
            A3XX_PC_VSTREAM_CONTROL_SIZE(1) | A3XX_PC_VSTREAM_CONTROL_N(0));

   /* emit IB to binning drawcmds: */
   fd3_emit_ib(ring, batch->binning);
   fd_reset_wfi(batch);

   fd_wfi(batch, ring);

   /* and then put stuff back the way it was: */

   OUT_PKT0(ring, REG_A3XX_VSC_BIN_CONTROL, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A3XX_SP_SP_CTRL_REG, 1);
   OUT_RING(ring, A3XX_SP_SP_CTRL_REG_RESOLVE |
                     A3XX_SP_SP_CTRL_REG_CONSTMODE(1) |
                     A3XX_SP_SP_CTRL_REG_SLEEPMODE(1) |
                     A3XX_SP_SP_CTRL_REG_L0MODE(0));

   OUT_PKT0(ring, REG_A3XX_RB_LRZ_VSC_CONTROL, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A3XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A3XX_GRAS_SC_CONTROL_RASTER_MODE(0));

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 2);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(pfb->nr_cbufs - 1));
   OUT_RING(ring, A3XX_RB_RENDER_CONTROL_ENABLE_GMEM |
                     A3XX_RB_RENDER_CONTROL_ALPHA_TEST_FUNC(FUNC_NEVER) |
                     A3XX_RB_RENDER_CONTROL_BIN_WIDTH(gmem->bin_w));

   fd_event_write(batch, ring, CACHE_FLUSH);
   fd_wfi(batch, ring);

   if (ctx->screen->gpu_id == 320) {
      /* dummy-draw workaround: */
      OUT_PKT3(ring, CP_DRAW_INDX, 3);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, DRAW(1, DI_SRC_SEL_AUTO_INDEX, INDEX_SIZE_IGN,
                          IGNORE_VISIBILITY, 0));
      OUT_RING(ring, 0); /* NumIndices */
      fd_reset_wfi(batch);
   }

   OUT_PKT3(ring, CP_NOP, 4);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000000);

   fd_wfi(batch, ring);

   if (ctx->screen->gpu_id == 320) {
      emit_binning_workaround(batch);
   }
}

/* before first tile */
static void
fd3_emit_tile_init(struct fd_batch *batch) assert_dt
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   uint32_t rb_render_control;

   fd3_emit_restore(batch, ring);

   /* note: use gmem->bin_w/h, the bin_w/h parameters may be truncated
    * at the right and bottom edge tiles
    */
   OUT_PKT0(ring, REG_A3XX_VSC_BIN_SIZE, 1);
   OUT_RING(ring, A3XX_VSC_BIN_SIZE_WIDTH(gmem->bin_w) |
                     A3XX_VSC_BIN_SIZE_HEIGHT(gmem->bin_h));

   update_vsc_pipe(batch);

   fd_wfi(batch, ring);
   OUT_PKT0(ring, REG_A3XX_RB_FRAME_BUFFER_DIMENSION, 1);
   OUT_RING(ring, A3XX_RB_FRAME_BUFFER_DIMENSION_WIDTH(pfb->width) |
                     A3XX_RB_FRAME_BUFFER_DIMENSION_HEIGHT(pfb->height));

   if (use_hw_binning(batch)) {
      /* emit hw binning pass: */
      emit_binning_pass(batch);

      patch_draws(batch, USE_VISIBILITY);
   } else {
      patch_draws(batch, IGNORE_VISIBILITY);
   }

   rb_render_control = A3XX_RB_RENDER_CONTROL_ENABLE_GMEM |
                       A3XX_RB_RENDER_CONTROL_BIN_WIDTH(gmem->bin_w);

   patch_rbrc(batch, rb_render_control);
}

/* before mem2gmem */
static void
fd3_emit_tile_prep(struct fd_batch *batch, const struct fd_tile *tile)
{
   struct fd_ringbuffer *ring = batch->gmem;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;

   OUT_PKT0(ring, REG_A3XX_RB_MODE_CONTROL, 1);
   OUT_RING(ring, A3XX_RB_MODE_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A3XX_RB_MODE_CONTROL_MARB_CACHE_SPLIT_MODE |
                     A3XX_RB_MODE_CONTROL_MRT(MAX2(1, pfb->nr_cbufs) - 1));
}

/* before IB to rendering cmds: */
static void
fd3_emit_tile_renderprep(struct fd_batch *batch,
                         const struct fd_tile *tile) assert_dt
{
   struct fd_context *ctx = batch->ctx;
   struct fd3_context *fd3_ctx = fd3_context(ctx);
   struct fd_ringbuffer *ring = batch->gmem;
   const struct fd_gmem_stateobj *gmem = batch->gmem_state;
   struct pipe_framebuffer_state *pfb = &batch->framebuffer;

   uint32_t x1 = tile->xoff;
   uint32_t y1 = tile->yoff;
   uint32_t x2 = tile->xoff + tile->bin_w - 1;
   uint32_t y2 = tile->yoff + tile->bin_h - 1;

   uint32_t reg;

   OUT_PKT0(ring, REG_A3XX_RB_DEPTH_INFO, 2);
   reg = A3XX_RB_DEPTH_INFO_DEPTH_BASE(gmem->zsbuf_base[0]);
   if (pfb->zsbuf) {
      reg |= A3XX_RB_DEPTH_INFO_DEPTH_FORMAT(fd_pipe2depth(pfb->zsbuf->format));
   }
   OUT_RING(ring, reg);
   if (pfb->zsbuf) {
      struct fd_resource *rsc = fd_resource(pfb->zsbuf->texture);
      OUT_RING(ring,
               A3XX_RB_DEPTH_PITCH(gmem->bin_w << fdl_cpp_shift(&rsc->layout)));
      if (rsc->stencil) {
         OUT_PKT0(ring, REG_A3XX_RB_STENCIL_INFO, 2);
         OUT_RING(ring, A3XX_RB_STENCIL_INFO_STENCIL_BASE(gmem->zsbuf_base[1]));
         OUT_RING(ring, A3XX_RB_STENCIL_PITCH(gmem->bin_w << fdl_cpp_shift(
                                                 &rsc->stencil->layout)));
      }
   } else {
      OUT_RING(ring, 0x00000000);
   }

   if (use_hw_binning(batch)) {
      const struct fd_vsc_pipe *pipe = &gmem->vsc_pipe[tile->p];
      struct fd_bo *pipe_bo = ctx->vsc_pipe_bo[tile->p];

      assert(pipe->w && pipe->h);

      fd_event_write(batch, ring, HLSQ_FLUSH);
      fd_wfi(batch, ring);

      OUT_PKT0(ring, REG_A3XX_PC_VSTREAM_CONTROL, 1);
      OUT_RING(ring, A3XX_PC_VSTREAM_CONTROL_SIZE(pipe->w * pipe->h) |
                        A3XX_PC_VSTREAM_CONTROL_N(tile->n));

      OUT_PKT3(ring, CP_SET_BIN_DATA, 2);
      OUT_RELOC(ring, pipe_bo, 0, 0,
                0); /* BIN_DATA_ADDR <- VSC_PIPE[p].DATA_ADDRESS */
      OUT_RELOC(ring, fd3_ctx->vsc_size_mem, /* BIN_SIZE_ADDR <-
                                                VSC_SIZE_ADDRESS + (p * 4) */
                (tile->p * 4), 0, 0);
   } else {
      OUT_PKT0(ring, REG_A3XX_PC_VSTREAM_CONTROL, 1);
      OUT_RING(ring, 0x00000000);
   }

   OUT_PKT3(ring, CP_SET_BIN, 3);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, CP_SET_BIN_1_X1(x1) | CP_SET_BIN_1_Y1(y1));
   OUT_RING(ring, CP_SET_BIN_2_X2(x2) | CP_SET_BIN_2_Y2(y2));

   emit_mrt(ring, pfb->nr_cbufs, pfb->cbufs, gmem->cbuf_base, gmem->bin_w,
            true);

   /* setup scissor/offset for current tile: */
   OUT_PKT0(ring, REG_A3XX_RB_WINDOW_OFFSET, 1);
   OUT_RING(ring, A3XX_RB_WINDOW_OFFSET_X(tile->xoff) |
                     A3XX_RB_WINDOW_OFFSET_Y(tile->yoff));

   OUT_PKT0(ring, REG_A3XX_GRAS_SC_SCREEN_SCISSOR_TL, 2);
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_TL_X(x1) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(y1));
   OUT_RING(ring, A3XX_GRAS_SC_SCREEN_SCISSOR_BR_X(x2) |
                     A3XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(y2));
}

void
fd3_gmem_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);

   ctx->emit_sysmem_prep = fd3_emit_sysmem_prep;
   ctx->emit_tile_init = fd3_emit_tile_init;
   ctx->emit_tile_prep = fd3_emit_tile_prep;
   ctx->emit_tile_mem2gmem = fd3_emit_tile_mem2gmem;
   ctx->emit_tile_renderprep = fd3_emit_tile_renderprep;
   ctx->emit_tile_gmem2mem = fd3_emit_tile_gmem2mem;
}
