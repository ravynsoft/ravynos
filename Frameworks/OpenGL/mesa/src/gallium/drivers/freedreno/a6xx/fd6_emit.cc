/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
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

#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_helpers.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include "util/u_viewport.h"

#include "freedreno_query_hw.h"
#include "freedreno_resource.h"
#include "freedreno_state.h"
#include "freedreno_tracepoints.h"

#include "fd6_blend.h"
#include "fd6_const.h"
#include "fd6_context.h"
#include "fd6_compute.h"
#include "fd6_emit.h"
#include "fd6_image.h"
#include "fd6_pack.h"
#include "fd6_program.h"
#include "fd6_rasterizer.h"
#include "fd6_texture.h"
#include "fd6_zsa.h"

/* Helper to get tex stateobj.
 */
static struct fd_ringbuffer *
tex_state(struct fd_context *ctx, enum pipe_shader_type type)
   assert_dt
{
   if (ctx->tex[type].num_textures == 0)
      return NULL;

   return fd_ringbuffer_ref(fd6_texture_state(ctx, type)->stateobj);
}

static struct fd_ringbuffer *
build_vbo_state(struct fd6_emit *emit) assert_dt
{
   const struct fd_vertex_state *vtx = &emit->ctx->vtx;

   const unsigned cnt = vtx->vertexbuf.count;
   const unsigned dwords = cnt * 4;  /* per vbo: reg64 + one reg32 + pkt hdr */

   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      emit->ctx->batch->submit, 4 * dwords, FD_RINGBUFFER_STREAMING);

   for (int32_t j = 0; j < cnt; j++) {
      OUT_PKT4(ring, REG_A6XX_VFD_FETCH(j), 3);
      const struct pipe_vertex_buffer *vb = &vtx->vertexbuf.vb[j];
      struct fd_resource *rsc = fd_resource(vb->buffer.resource);
      if (rsc == NULL) {
         OUT_RING(ring, 0);
         OUT_RING(ring, 0);
         OUT_RING(ring, 0);
      } else {
         uint32_t off = vb->buffer_offset;
         uint32_t size = vb->buffer.resource->width0 - off;

         OUT_RELOC(ring, rsc->bo, off, 0, 0);
         OUT_RING(ring, size);       /* VFD_FETCH[j].SIZE */
      }
   }

   return ring;
}

static enum a6xx_ztest_mode
compute_ztest_mode(struct fd6_emit *emit, bool lrz_valid) assert_dt
{
   if (emit->prog->lrz_mask.z_mode != A6XX_INVALID_ZTEST)
      return emit->prog->lrz_mask.z_mode;

   struct fd_context *ctx = emit->ctx;
   struct pipe_framebuffer_state *pfb = &ctx->batch->framebuffer;
   struct fd6_zsa_stateobj *zsa = fd6_zsa_stateobj(ctx->zsa);
   const struct ir3_shader_variant *fs = emit->fs;

   if (!zsa->base.depth_enabled) {
      return A6XX_LATE_Z;
   } else if ((fs->has_kill || zsa->alpha_test) &&
              (zsa->writes_zs || !pfb->zsbuf)) {
      /* Slightly odd, but seems like the hw wants us to select
       * LATE_Z mode if there is no depth buffer + discard.  Either
       * that, or when occlusion query is enabled.  See:
       *
       * dEQP-GLES31.functional.fbo.no_attachments.*
       */
      return lrz_valid ? A6XX_EARLY_LRZ_LATE_Z : A6XX_LATE_Z;
   } else {
      return A6XX_EARLY_Z;
   }
}

/**
 * Calculate normalized LRZ state based on zsa/prog/blend state, updating
 * the zsbuf's lrz state as necessary to detect the cases where we need
 * to invalidate lrz.
 */
static struct fd6_lrz_state
compute_lrz_state(struct fd6_emit *emit) assert_dt
{
   struct fd_context *ctx = emit->ctx;
   struct pipe_framebuffer_state *pfb = &ctx->batch->framebuffer;
   struct fd6_lrz_state lrz;

   if (!pfb->zsbuf) {
      memset(&lrz, 0, sizeof(lrz));
      lrz.z_mode = compute_ztest_mode(emit, false);
      return lrz;
   }

   struct fd6_blend_stateobj *blend = fd6_blend_stateobj(ctx->blend);
   struct fd6_zsa_stateobj *zsa = fd6_zsa_stateobj(ctx->zsa);
   struct fd_resource *rsc = fd_resource(pfb->zsbuf->texture);
   bool reads_dest = blend->reads_dest;

   lrz = zsa->lrz;

   lrz.val &= emit->prog->lrz_mask.val;

   /* normalize lrz state: */
   if (reads_dest || blend->base.alpha_to_coverage) {
      lrz.write = false;
   }

   /* Unwritten channels *that actually exist* are a form of blending
    * reading the dest from the PoV of LRZ, but the valid dst channels
    * isn't known when blend CSO is constructed so we need to handle
    * that here.
    */
   if (ctx->all_mrt_channel_mask & ~blend->all_mrt_write_mask) {
      lrz.write = false;
      reads_dest = true;
   }

   /* Writing depth with blend enabled means we need to invalidate LRZ,
    * because the written depth value could mean that a later draw with
    * depth enabled (where we would otherwise write LRZ) could have
    * fragments which don't pass the depth test due to this draw.  For
    * example, consider this sequence of draws, with depth mode GREATER:
    *
    *   draw A:
    *     z=0.1, fragments pass
    *   draw B:
    *     z=0.4, fragments pass
    *     blend enabled (LRZ write disabled)
    *     depth write enabled
    *   draw C:
    *     z=0.2, fragments don't pass
    *     blend disabled
    *     depth write enabled
    *
    * Normally looking at the state in draw C, we'd assume we could
    * enable LRZ write.  But this would cause early-z/lrz to discard
    * fragments from draw A which should be visible due to draw B.
    */
   if (reads_dest && zsa->writes_z && ctx->screen->driconf.conservative_lrz) {
      if (!zsa->perf_warn_blend && rsc->lrz_valid) {
         perf_debug_ctx(ctx, "Invalidating LRZ due to blend+depthwrite");
         zsa->perf_warn_blend = true;
      }
      rsc->lrz_valid = false;
   }

   /* if we change depthfunc direction, bail out on using LRZ.  The
    * LRZ buffer encodes a min/max depth value per block, but if
    * we switch from GT/GE <-> LT/LE, those values cannot be
    * interpreted properly.
    */
   if (zsa->base.depth_enabled && (rsc->lrz_direction != FD_LRZ_UNKNOWN) &&
       (rsc->lrz_direction != lrz.direction)) {
      if (!zsa->perf_warn_zdir && rsc->lrz_valid) {
         perf_debug_ctx(ctx, "Invalidating LRZ due to depth test direction change");
         zsa->perf_warn_zdir = true;
      }
      rsc->lrz_valid = false;
   }

   if (zsa->invalidate_lrz || !rsc->lrz_valid) {
      rsc->lrz_valid = false;
      memset(&lrz, 0, sizeof(lrz));
   }

   lrz.z_mode = compute_ztest_mode(emit, rsc->lrz_valid);

   /* Once we start writing to the real depth buffer, we lock in the
    * direction for LRZ.. if we have to skip a LRZ write for any
    * reason, it is still safe to have LRZ until there is a direction
    * reversal.  Prior to the reversal, since we disabled LRZ writes
    * in the "unsafe" cases, this just means that the LRZ test may
    * not early-discard some things that end up not passing a later
    * test (ie. be overly concervative).  But once you have a reversal
    * of direction, it is possible to increase/decrease the z value
    * to the point where the overly-conservative test is incorrect.
    */
   if (zsa->base.depth_writemask) {
      rsc->lrz_direction = lrz.direction;
   }

   return lrz;
}

static struct fd_ringbuffer *
build_lrz(struct fd6_emit *emit) assert_dt
{
   struct fd_context *ctx = emit->ctx;
   struct fd6_context *fd6_ctx = fd6_context(ctx);
   struct fd6_lrz_state lrz = compute_lrz_state(emit);

   /* If the LRZ state has not changed, we can skip the emit: */
   if (!ctx->last.dirty && (fd6_ctx->last.lrz.val == lrz.val))
      return NULL;

   fd6_ctx->last.lrz = lrz;

   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      ctx->batch->submit, 8 * 4, FD_RINGBUFFER_STREAMING);

   OUT_REG(ring,
           A6XX_GRAS_LRZ_CNTL(.enable = lrz.enable, .lrz_write = lrz.write,
                              .greater = lrz.direction == FD_LRZ_GREATER,
                              .z_test_enable = lrz.test,
                              .z_bounds_enable = lrz.z_bounds_enable, ));
   OUT_REG(ring, A6XX_RB_LRZ_CNTL(.enable = lrz.enable, ));

   OUT_REG(ring, A6XX_RB_DEPTH_PLANE_CNTL(.z_mode = lrz.z_mode, ));

   OUT_REG(ring, A6XX_GRAS_SU_DEPTH_PLANE_CNTL(.z_mode = lrz.z_mode, ));

   return ring;
}

static struct fd_ringbuffer *
build_scissor(struct fd6_emit *emit) assert_dt
{
   struct fd_context *ctx = emit->ctx;
   struct pipe_scissor_state *scissors = fd_context_get_scissor(ctx);
   unsigned num_viewports = emit->prog->num_viewports;

   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      emit->ctx->batch->submit, (1 + (2 * num_viewports)) * 4, FD_RINGBUFFER_STREAMING);

   OUT_PKT4(ring, REG_A6XX_GRAS_SC_SCREEN_SCISSOR_TL(0), 2 * num_viewports);
   for (unsigned i = 0; i < num_viewports; i++) {
      OUT_RING(ring, A6XX_GRAS_SC_SCREEN_SCISSOR_TL_X(scissors[i].minx) |
               A6XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(scissors[i].miny));
      OUT_RING(ring, A6XX_GRAS_SC_SCREEN_SCISSOR_BR_X(scissors[i].maxx) |
               A6XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(scissors[i].maxy));
   }

   return ring;
}

/* Combination of FD_DIRTY_FRAMEBUFFER | FD_DIRTY_RASTERIZER_DISCARD |
 * FD_DIRTY_PROG | FD_DIRTY_DUAL_BLEND
 */
static struct fd_ringbuffer *
build_prog_fb_rast(struct fd6_emit *emit) assert_dt
{
   struct fd_context *ctx = emit->ctx;
   struct pipe_framebuffer_state *pfb = &ctx->batch->framebuffer;
   const struct fd6_program_state *prog = fd6_emit_get_prog(emit);
   const struct ir3_shader_variant *fs = emit->fs;

   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      ctx->batch->submit, 9 * 4, FD_RINGBUFFER_STREAMING);

   unsigned nr = pfb->nr_cbufs;

   if (ctx->rasterizer->rasterizer_discard)
      nr = 0;

   struct fd6_blend_stateobj *blend = fd6_blend_stateobj(ctx->blend);

   if (blend->use_dual_src_blend)
      nr++;

   OUT_PKT4(ring, REG_A6XX_RB_FS_OUTPUT_CNTL0, 2);
   OUT_RING(ring, COND(fs->writes_pos, A6XX_RB_FS_OUTPUT_CNTL0_FRAG_WRITES_Z) |
                     COND(fs->writes_smask && pfb->samples > 1,
                          A6XX_RB_FS_OUTPUT_CNTL0_FRAG_WRITES_SAMPMASK) |
                     COND(fs->writes_stencilref,
                          A6XX_RB_FS_OUTPUT_CNTL0_FRAG_WRITES_STENCILREF) |
                     COND(blend->use_dual_src_blend,
                          A6XX_RB_FS_OUTPUT_CNTL0_DUAL_COLOR_IN_ENABLE));
   OUT_RING(ring, A6XX_RB_FS_OUTPUT_CNTL1_MRT(nr));

   OUT_PKT4(ring, REG_A6XX_SP_FS_OUTPUT_CNTL1, 1);
   OUT_RING(ring, A6XX_SP_FS_OUTPUT_CNTL1_MRT(nr));

   unsigned mrt_components = 0;
   for (unsigned i = 0; i < pfb->nr_cbufs; i++) {
      if (!pfb->cbufs[i])
         continue;
      mrt_components |= 0xf << (i * 4);
   }

   /* dual source blending has an extra fs output in the 2nd slot */
   if (blend->use_dual_src_blend)
      mrt_components |= 0xf << 4;

   mrt_components &= prog->mrt_components;

   OUT_REG(ring, A6XX_SP_FS_RENDER_COMPONENTS(.dword = mrt_components));
   OUT_REG(ring, A6XX_RB_RENDER_COMPONENTS(.dword = mrt_components));

   return ring;
}

static struct fd_ringbuffer *
build_blend_color(struct fd6_emit *emit) assert_dt
{
   struct fd_context *ctx = emit->ctx;
   struct pipe_blend_color *bcolor = &ctx->blend_color;
   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      ctx->batch->submit, 5 * 4, FD_RINGBUFFER_STREAMING);

   OUT_REG(ring, A6XX_RB_BLEND_RED_F32(bcolor->color[0]),
           A6XX_RB_BLEND_GREEN_F32(bcolor->color[1]),
           A6XX_RB_BLEND_BLUE_F32(bcolor->color[2]),
           A6XX_RB_BLEND_ALPHA_F32(bcolor->color[3]));

   return ring;
}

static struct fd_ringbuffer *
build_sample_locations(struct fd6_emit *emit)
   assert_dt
{
   struct fd_context *ctx = emit->ctx;

   if (!ctx->sample_locations_enabled) {
      struct fd6_context *fd6_ctx = fd6_context(ctx);
      return fd_ringbuffer_ref(fd6_ctx->sample_locations_disable_stateobj);
   }

   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      ctx->batch->submit, 9 * 4, FD_RINGBUFFER_STREAMING);

   uint32_t sample_locations = 0;
   for (int i = 0; i < 4; i++) {
      float x = (ctx->sample_locations[i] & 0xf) / 16.0f;
      float y = (16 - (ctx->sample_locations[i] >> 4)) / 16.0f;

      x = CLAMP(x, 0.0f, 0.9375f);
      y = CLAMP(y, 0.0f, 0.9375f);

      sample_locations |=
         (A6XX_RB_SAMPLE_LOCATION_0_SAMPLE_0_X(x) |
          A6XX_RB_SAMPLE_LOCATION_0_SAMPLE_0_Y(y)) << i*8;
   }

   OUT_REG(ring, A6XX_GRAS_SAMPLE_CONFIG(.location_enable = true),
                 A6XX_GRAS_SAMPLE_LOCATION_0(.dword = sample_locations));

   OUT_REG(ring, A6XX_RB_SAMPLE_CONFIG(.location_enable = true),
                 A6XX_RB_SAMPLE_LOCATION_0(.dword = sample_locations));

   OUT_REG(ring, A6XX_SP_TP_SAMPLE_CONFIG(.location_enable = true),
                 A6XX_SP_TP_SAMPLE_LOCATION_0(.dword = sample_locations));

   return ring;
}

static void
fd6_emit_streamout(struct fd_ringbuffer *ring, struct fd6_emit *emit) assert_dt
{
   struct fd_context *ctx = emit->ctx;
   const struct fd6_program_state *prog = fd6_emit_get_prog(emit);
   const struct ir3_stream_output_info *info = prog->stream_output;
   struct fd_streamout_stateobj *so = &ctx->streamout;
   unsigned streamout_mask = 0;

   if (!info)
      return;

   for (unsigned i = 0; i < so->num_targets; i++) {
      struct fd_stream_output_target *target =
         fd_stream_output_target(so->targets[i]);

      if (!target)
         continue;

      target->stride = info->stride[i];

      OUT_PKT4(ring, REG_A6XX_VPC_SO_BUFFER_BASE(i), 3);
      /* VPC_SO[i].BUFFER_BASE_LO: */
      OUT_RELOC(ring, fd_resource(target->base.buffer)->bo, 0, 0, 0);
      OUT_RING(ring, target->base.buffer_size + target->base.buffer_offset);

      struct fd_bo *offset_bo = fd_resource(target->offset_buf)->bo;

      if (so->reset & (1 << i)) {
         assert(so->offsets[i] == 0);

         OUT_PKT7(ring, CP_MEM_WRITE, 3);
         OUT_RELOC(ring, offset_bo, 0, 0, 0);
         OUT_RING(ring, target->base.buffer_offset);

         OUT_PKT4(ring, REG_A6XX_VPC_SO_BUFFER_OFFSET(i), 1);
         OUT_RING(ring, target->base.buffer_offset);
      } else {
         OUT_PKT7(ring, CP_MEM_TO_REG, 3);
         OUT_RING(ring, CP_MEM_TO_REG_0_REG(REG_A6XX_VPC_SO_BUFFER_OFFSET(i)) |
                           CP_MEM_TO_REG_0_SHIFT_BY_2 | CP_MEM_TO_REG_0_UNK31 |
                           CP_MEM_TO_REG_0_CNT(0));
         OUT_RELOC(ring, offset_bo, 0, 0, 0);
      }

      // After a draw HW would write the new offset to offset_bo
      OUT_PKT4(ring, REG_A6XX_VPC_SO_FLUSH_BASE(i), 2);
      OUT_RELOC(ring, offset_bo, 0, 0, 0);

      so->reset &= ~(1 << i);

      streamout_mask |= (1 << i);
   }

   if (streamout_mask) {
      fd6_state_add_group(&emit->state, prog->streamout_stateobj, FD6_GROUP_SO);
   } else if (ctx->last.streamout_mask != 0) {
      /* If we transition from a draw with streamout to one without, turn
       * off streamout.
       */
      fd6_state_add_group(&emit->state, fd6_context(ctx)->streamout_disable_stateobj,
                         FD6_GROUP_SO);
   }

   /* Make sure that any use of our TFB outputs (indirect draw source or shader
    * UBO reads) comes after the TFB output is written.  From the GL 4.6 core
    * spec:
    *
    *     "Buffers should not be bound or in use for both transform feedback and
    *      other purposes in the GL.  Specifically, if a buffer object is
    *      simultaneously bound to a transform feedback buffer binding point
    *      and elsewhere in the GL, any writes to or reads from the buffer
    *      generate undefined values."
    *
    * So we idle whenever SO buffers change.  Note that this function is called
    * on every draw with TFB enabled, so check the dirty flag for the buffers
    * themselves.
    */
   if (ctx->dirty & FD_DIRTY_STREAMOUT)
      OUT_WFI5(ring);

   ctx->last.streamout_mask = streamout_mask;
   emit->streamout_mask = streamout_mask;
}

/**
 * Stuff that less frequently changes and isn't (yet) moved into stategroups
 */
static void
fd6_emit_non_ring(struct fd_ringbuffer *ring, struct fd6_emit *emit) assert_dt
{
   struct fd_context *ctx = emit->ctx;
   const enum fd_dirty_3d_state dirty = ctx->dirty;
   unsigned num_viewports = emit->prog->num_viewports;

   if (dirty & FD_DIRTY_STENCIL_REF) {
      struct pipe_stencil_ref *sr = &ctx->stencil_ref;

      OUT_PKT4(ring, REG_A6XX_RB_STENCILREF, 1);
      OUT_RING(ring, A6XX_RB_STENCILREF_REF(sr->ref_value[0]) |
                        A6XX_RB_STENCILREF_BFREF(sr->ref_value[1]));
   }

   if (dirty & (FD_DIRTY_VIEWPORT | FD_DIRTY_PROG)) {
      for (unsigned i = 0; i < num_viewports; i++) {
         struct pipe_scissor_state *scissor = &ctx->viewport_scissor[i];
         struct pipe_viewport_state *vp = & ctx->viewport[i];

         OUT_REG(ring, A6XX_GRAS_CL_VPORT_XOFFSET(i, vp->translate[0]),
                 A6XX_GRAS_CL_VPORT_XSCALE(i, vp->scale[0]),
                 A6XX_GRAS_CL_VPORT_YOFFSET(i, vp->translate[1]),
                 A6XX_GRAS_CL_VPORT_YSCALE(i, vp->scale[1]),
                 A6XX_GRAS_CL_VPORT_ZOFFSET(i, vp->translate[2]),
                 A6XX_GRAS_CL_VPORT_ZSCALE(i, vp->scale[2]));

         OUT_REG(
               ring,
               A6XX_GRAS_SC_VIEWPORT_SCISSOR_TL(i,
                                                .x = scissor->minx,
                                                .y = scissor->miny),
               A6XX_GRAS_SC_VIEWPORT_SCISSOR_BR(i,
                                                .x = scissor->maxx,
                                                .y = scissor->maxy));
      }

      OUT_REG(ring, A6XX_GRAS_CL_GUARDBAND_CLIP_ADJ(.horz = ctx->guardband.x,
                                                    .vert = ctx->guardband.y));
   }

   /* The clamp ranges are only used when the rasterizer wants depth
    * clamping.
    */
   if ((dirty & (FD_DIRTY_VIEWPORT | FD_DIRTY_RASTERIZER | FD_DIRTY_PROG)) &&
       fd_depth_clamp_enabled(ctx)) {
      for (unsigned i = 0; i < num_viewports; i++) {
         struct pipe_viewport_state *vp = & ctx->viewport[i];
         float zmin, zmax;

         util_viewport_zmin_zmax(vp, ctx->rasterizer->clip_halfz,
                                 &zmin, &zmax);

         OUT_REG(ring, A6XX_GRAS_CL_Z_CLAMP_MIN(i, zmin),
                 A6XX_GRAS_CL_Z_CLAMP_MAX(i, zmax));

         /* TODO: what to do about this and multi viewport ? */
         if (i == 0)
            OUT_REG(ring, A6XX_RB_Z_CLAMP_MIN(zmin), A6XX_RB_Z_CLAMP_MAX(zmax));
      }
   }
}

static struct fd_ringbuffer*
build_prim_mode(struct fd6_emit *emit, struct fd_context *ctx, bool gmem)
   assert_dt
{
   struct fd_ringbuffer *ring =
      fd_submit_new_ringbuffer(emit->ctx->batch->submit, 2 * 4, FD_RINGBUFFER_STREAMING);
   uint32_t prim_mode = NO_FLUSH;
   if (emit->fs->fs.uses_fbfetch_output) {
      if (gmem) {
         prim_mode = (ctx->blend->blend_coherent || emit->fs->fs.fbfetch_coherent)
            ? FLUSH_PER_OVERLAP : NO_FLUSH;
      } else {
         prim_mode = FLUSH_PER_OVERLAP_AND_OVERWRITE;
      }
   } else {
      prim_mode = NO_FLUSH;
   }
   OUT_REG(ring, A6XX_GRAS_SC_CNTL(.ccusinglecachelinesize = 2,
                                   .single_prim_mode = (enum a6xx_single_prim_mode)prim_mode));
   return ring;
}

template <chip CHIP, fd6_pipeline_type PIPELINE>
void
fd6_emit_3d_state(struct fd_ringbuffer *ring, struct fd6_emit *emit)
{
   struct fd_context *ctx = emit->ctx;
   struct pipe_framebuffer_state *pfb = &ctx->batch->framebuffer;
   const struct fd6_program_state *prog = fd6_emit_get_prog(emit);
   const struct ir3_shader_variant *fs = emit->fs;

   emit_marker6(ring, 5);

   /* Special case, we need to re-emit bindless FS state w/ the
    * fb-read state appended:
    */
   if ((emit->dirty_groups & BIT(FD6_GROUP_PROG)) && fs->fb_read) {
      ctx->batch->gmem_reason |= FD_GMEM_FB_READ;
      emit->dirty_groups |= BIT(FD6_GROUP_FS_BINDLESS);
   }

   u_foreach_bit (b, emit->dirty_groups) {
      enum fd6_state_id group = (enum fd6_state_id)b;
      struct fd_ringbuffer *state = NULL;

      switch (group) {
      case FD6_GROUP_VTXSTATE:
         state = fd6_vertex_stateobj(ctx->vtx.vtx)->stateobj;
         fd6_state_add_group(&emit->state, state, FD6_GROUP_VTXSTATE);
         break;
      case FD6_GROUP_VBO:
         state = build_vbo_state(emit);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_VBO);
         break;
      case FD6_GROUP_ZSA:
         state = fd6_zsa_state(
            ctx,
            util_format_is_pure_integer(pipe_surface_format(pfb->cbufs[0])),
            fd_depth_clamp_enabled(ctx));
         fd6_state_add_group(&emit->state, state, FD6_GROUP_ZSA);
         break;
      case FD6_GROUP_LRZ:
         state = build_lrz(emit);
         if (state)
            fd6_state_take_group(&emit->state, state, FD6_GROUP_LRZ);
         break;
      case FD6_GROUP_SCISSOR:
         state = build_scissor(emit);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_SCISSOR);
         break;
      case FD6_GROUP_PROG:
         fd6_state_add_group(&emit->state, prog->config_stateobj,
                             FD6_GROUP_PROG_CONFIG);
         fd6_state_add_group(&emit->state, prog->stateobj, FD6_GROUP_PROG);
         fd6_state_add_group(&emit->state, prog->binning_stateobj,
                             FD6_GROUP_PROG_BINNING);

         /* emit remaining streaming program state, ie. what depends on
          * other emit state, so cannot be pre-baked.
          */
         fd6_state_take_group(&emit->state, fd6_program_interp_state(emit),
                              FD6_GROUP_PROG_INTERP);
         break;
      case FD6_GROUP_RASTERIZER:
         state = fd6_rasterizer_state<CHIP>(ctx, emit->primitive_restart);
         fd6_state_add_group(&emit->state, state, FD6_GROUP_RASTERIZER);
         break;
      case FD6_GROUP_PROG_FB_RAST:
         state = build_prog_fb_rast(emit);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_PROG_FB_RAST);
         break;
      case FD6_GROUP_BLEND:
         state = fd6_blend_variant(ctx->blend, pfb->samples, ctx->sample_mask)
                    ->stateobj;
         fd6_state_add_group(&emit->state, state, FD6_GROUP_BLEND);
         break;
      case FD6_GROUP_BLEND_COLOR:
         state = build_blend_color(emit);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_BLEND_COLOR);
         break;
      case FD6_GROUP_SAMPLE_LOCATIONS:
         state = build_sample_locations(emit);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_SAMPLE_LOCATIONS);
         break;
      case FD6_GROUP_VS_BINDLESS:
         state = fd6_build_bindless_state<CHIP>(ctx, PIPE_SHADER_VERTEX, false);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_VS_BINDLESS);
         break;
      case FD6_GROUP_HS_BINDLESS:
         state = fd6_build_bindless_state<CHIP>(ctx, PIPE_SHADER_TESS_CTRL, false);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_HS_BINDLESS);
         break;
      case FD6_GROUP_DS_BINDLESS:
         state = fd6_build_bindless_state<CHIP>(ctx, PIPE_SHADER_TESS_EVAL, false);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_DS_BINDLESS);
         break;
      case FD6_GROUP_GS_BINDLESS:
         state = fd6_build_bindless_state<CHIP>(ctx, PIPE_SHADER_GEOMETRY, false);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_GS_BINDLESS);
         break;
      case FD6_GROUP_FS_BINDLESS:
         state = fd6_build_bindless_state<CHIP>(ctx, PIPE_SHADER_FRAGMENT, fs->fb_read);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_FS_BINDLESS);
         break;
      case FD6_GROUP_CONST:
         state = fd6_build_user_consts<PIPELINE>(emit);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_CONST);
         break;
      case FD6_GROUP_DRIVER_PARAMS:
         state = fd6_build_driver_params<PIPELINE>(emit);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_DRIVER_PARAMS);
         break;
      case FD6_GROUP_PRIMITIVE_PARAMS:
         if (PIPELINE == HAS_TESS_GS) {
            state = fd6_build_tess_consts(emit);
            fd6_state_take_group(&emit->state, state, FD6_GROUP_PRIMITIVE_PARAMS);
         }
         break;
      case FD6_GROUP_VS_TEX:
         state = tex_state(ctx, PIPE_SHADER_VERTEX);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_VS_TEX);
         break;
      case FD6_GROUP_HS_TEX:
         state = tex_state(ctx, PIPE_SHADER_TESS_CTRL);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_HS_TEX);
         break;
      case FD6_GROUP_DS_TEX:
         state = tex_state(ctx, PIPE_SHADER_TESS_EVAL);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_DS_TEX);
         break;
      case FD6_GROUP_GS_TEX:
         state = tex_state(ctx, PIPE_SHADER_GEOMETRY);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_GS_TEX);
         break;
      case FD6_GROUP_FS_TEX:
         state = tex_state(ctx, PIPE_SHADER_FRAGMENT);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_FS_TEX);
         break;
      case FD6_GROUP_SO:
         fd6_emit_streamout(ring, emit);
         break;
      case FD6_GROUP_PRIM_MODE_SYSMEM:
         state = build_prim_mode(emit, ctx, false);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_PRIM_MODE_SYSMEM);
         break;
      case FD6_GROUP_PRIM_MODE_GMEM:
         state = build_prim_mode(emit, ctx, true);
         fd6_state_take_group(&emit->state, state, FD6_GROUP_PRIM_MODE_GMEM);
         break;
      case FD6_GROUP_NON_GROUP:
         fd6_emit_non_ring(ring, emit);
         break;
      default:
         break;
      }
   }

   fd6_state_emit(&emit->state, ring);
}

template void fd6_emit_3d_state<A6XX, NO_TESS_GS>(struct fd_ringbuffer *ring, struct fd6_emit *emit);
template void fd6_emit_3d_state<A7XX, NO_TESS_GS>(struct fd_ringbuffer *ring, struct fd6_emit *emit);
template void fd6_emit_3d_state<A6XX, HAS_TESS_GS>(struct fd_ringbuffer *ring, struct fd6_emit *emit);
template void fd6_emit_3d_state<A7XX, HAS_TESS_GS>(struct fd_ringbuffer *ring, struct fd6_emit *emit);

template <chip CHIP>
void
fd6_emit_cs_state(struct fd_context *ctx, struct fd_ringbuffer *ring,
                  struct fd6_compute_state *cs)
{
   struct fd6_state state = {};

   /* We want CP_SET_DRAW_STATE to execute immediately, otherwise we need to
    * emit consts as draw state groups (which otherwise has no benefit outside
    * of GMEM 3d using viz stream from binning pass).
    *
    * In particular, the PROG state group sets up the configuration for the
    * const state, so it must execute before we start loading consts, rather
    * than be deferred until CP_EXEC_CS.
    */
   OUT_PKT7(ring, CP_SET_MODE, 1);
   OUT_RING(ring, 1);

   uint32_t gen_dirty = ctx->gen_dirty &
         (BIT(FD6_GROUP_PROG) | BIT(FD6_GROUP_CS_TEX) | BIT(FD6_GROUP_CS_BINDLESS));

   u_foreach_bit (b, gen_dirty) {
      enum fd6_state_id group = (enum fd6_state_id)b;

      switch (group) {
      case FD6_GROUP_PROG:
         fd6_state_add_group(&state, cs->stateobj, FD6_GROUP_PROG);
         break;
      case FD6_GROUP_CS_TEX:
         fd6_state_take_group(
               &state,
               tex_state(ctx, PIPE_SHADER_COMPUTE),
               FD6_GROUP_CS_TEX);
         break;
      case FD6_GROUP_CS_BINDLESS:
         fd6_state_take_group(
               &state,
               fd6_build_bindless_state<CHIP>(ctx, PIPE_SHADER_COMPUTE, false),
               FD6_GROUP_CS_BINDLESS);
         break;
      default:
         /* State-group unused for compute shaders */
         break;
      }
   }

   fd6_state_emit(&state, ring);
}

void
fd6_emit_ccu_cntl(struct fd_ringbuffer *ring, struct fd_screen *screen, bool gmem)
{
   enum a6xx_ccu_color_cache_size cache_size = (a6xx_ccu_color_cache_size)(screen->info->a6xx.gmem_ccu_color_cache_fraction);
   uint32_t offset = gmem ? screen->ccu_offset_gmem : screen->ccu_offset_bypass;
   uint32_t offset_hi = offset >> 21;
   offset &= 0x1fffff;

   OUT_REG(ring,
           A6XX_RB_CCU_CNTL(.gmem_fast_clear_disable =
                               !screen->info->a6xx.has_gmem_fast_clear,
                            .concurrent_resolve =
                               screen->info->a6xx.concurrent_resolve,
                            .depth_offset_hi = 0,
                            .color_offset_hi = offset_hi,
                            .depth_cache_size = 0,
                            .depth_offset = 0,
                            .color_cache_size = cache_size,
                            .color_offset = offset,
                            ));
}

template void fd6_emit_cs_state<A6XX>(struct fd_context *ctx, struct fd_ringbuffer *ring, struct fd6_compute_state *cs);
template void fd6_emit_cs_state<A7XX>(struct fd_context *ctx, struct fd_ringbuffer *ring, struct fd6_compute_state *cs);

/* emit setup at begin of new cmdstream buffer (don't rely on previous
 * state, there could have been a context switch between ioctls):
 */
template <chip CHIP>
void
fd6_emit_restore(struct fd_batch *batch, struct fd_ringbuffer *ring)
{
   struct fd_screen *screen = batch->ctx->screen;

   if (!batch->nondraw) {
      trace_start_state_restore(&batch->trace, ring);
   }

   OUT_PKT7(ring, CP_SET_MODE, 1);
   OUT_RING(ring, 0);

   fd6_cache_inv(batch, ring);

   OUT_REG(ring,
           HLSQ_INVALIDATE_CMD(CHIP, .vs_state = true, .hs_state = true,
                                    .ds_state = true, .gs_state = true,
                                    .fs_state = true, .cs_state = true,
                                    .cs_ibo = true, .gfx_ibo = true,
                                    .cs_shared_const = true,
                                    .gfx_shared_const = true,
                                    .cs_bindless = 0x1f, .gfx_bindless = 0x1f));

   OUT_WFI5(ring);

   WRITE(REG_A6XX_RB_DBG_ECO_CNTL, screen->info->a6xx.magic.RB_DBG_ECO_CNTL);
   WRITE(REG_A6XX_SP_FLOAT_CNTL, A6XX_SP_FLOAT_CNTL_F16_NO_INF);
   WRITE(REG_A6XX_SP_DBG_ECO_CNTL, screen->info->a6xx.magic.SP_DBG_ECO_CNTL);
   WRITE(REG_A6XX_SP_PERFCTR_ENABLE, 0x3f);
   WRITE(REG_A6XX_TPL1_UNKNOWN_B605, 0x44);
   WRITE(REG_A6XX_TPL1_DBG_ECO_CNTL, screen->info->a6xx.magic.TPL1_DBG_ECO_CNTL);
   WRITE(REG_A6XX_HLSQ_UNKNOWN_BE00, 0x80);
   WRITE(REG_A6XX_HLSQ_UNKNOWN_BE01, 0);

   WRITE(REG_A6XX_VPC_DBG_ECO_CNTL, screen->info->a6xx.magic.VPC_DBG_ECO_CNTL);
   WRITE(REG_A6XX_GRAS_DBG_ECO_CNTL, screen->info->a6xx.magic.GRAS_DBG_ECO_CNTL);
   WRITE(REG_A6XX_HLSQ_DBG_ECO_CNTL, screen->info->a6xx.magic.HLSQ_DBG_ECO_CNTL);
   WRITE(REG_A6XX_SP_CHICKEN_BITS, screen->info->a6xx.magic.SP_CHICKEN_BITS);
   WRITE(REG_A6XX_SP_IBO_COUNT, 0);
   WRITE(REG_A6XX_SP_UNKNOWN_B182, 0);
   WRITE(REG_A6XX_HLSQ_SHARED_CONSTS, 0);
   WRITE(REG_A6XX_UCHE_UNKNOWN_0E12, screen->info->a6xx.magic.UCHE_UNKNOWN_0E12);
   WRITE(REG_A6XX_UCHE_CLIENT_PF, screen->info->a6xx.magic.UCHE_CLIENT_PF);
   WRITE(REG_A6XX_RB_UNKNOWN_8E01, screen->info->a6xx.magic.RB_UNKNOWN_8E01);
   WRITE(REG_A6XX_SP_UNKNOWN_A9A8, 0);
   WRITE(REG_A6XX_SP_MODE_CONTROL,
         A6XX_SP_MODE_CONTROL_CONSTANT_DEMOTION_ENABLE | 4);
   WRITE(REG_A6XX_VFD_ADD_OFFSET, A6XX_VFD_ADD_OFFSET_VERTEX);
   WRITE(REG_A6XX_VPC_UNKNOWN_9107, 0);
   WRITE(REG_A6XX_RB_UNKNOWN_8811, 0x00000010);
   WRITE(REG_A6XX_PC_MODE_CNTL, screen->info->a6xx.magic.PC_MODE_CNTL);

   WRITE(REG_A6XX_GRAS_LRZ_PS_INPUT_CNTL, 0);
   WRITE(REG_A6XX_GRAS_SAMPLE_CNTL, 0);
   WRITE(REG_A6XX_GRAS_UNKNOWN_8110, 0x2);

   WRITE(REG_A6XX_RB_UNKNOWN_8818, 0);
   WRITE(REG_A6XX_RB_UNKNOWN_8819, 0);
   WRITE(REG_A6XX_RB_UNKNOWN_881A, 0);
   WRITE(REG_A6XX_RB_UNKNOWN_881B, 0);
   WRITE(REG_A6XX_RB_UNKNOWN_881C, 0);
   WRITE(REG_A6XX_RB_UNKNOWN_881D, 0);
   WRITE(REG_A6XX_RB_UNKNOWN_881E, 0);
   WRITE(REG_A6XX_RB_UNKNOWN_88F0, 0);

   WRITE(REG_A6XX_VPC_POINT_COORD_INVERT, A6XX_VPC_POINT_COORD_INVERT(0).value);
   WRITE(REG_A6XX_VPC_UNKNOWN_9300, 0);

   WRITE(REG_A6XX_VPC_SO_DISABLE, A6XX_VPC_SO_DISABLE(true).value);

   OUT_REG(ring, PC_RASTER_CNTL(CHIP));

   WRITE(REG_A6XX_PC_MULTIVIEW_CNTL, 0);

   WRITE(REG_A6XX_SP_UNKNOWN_B183, 0);

   WRITE(REG_A6XX_GRAS_SU_CONSERVATIVE_RAS_CNTL, 0);
   WRITE(REG_A6XX_GRAS_VS_LAYER_CNTL, 0);
   WRITE(REG_A6XX_GRAS_SC_CNTL, A6XX_GRAS_SC_CNTL_CCUSINGLECACHELINESIZE(2));
   WRITE(REG_A6XX_GRAS_UNKNOWN_80AF, 0);
   WRITE(REG_A6XX_VPC_UNKNOWN_9210, 0);
   WRITE(REG_A6XX_VPC_UNKNOWN_9211, 0);
   WRITE(REG_A6XX_VPC_UNKNOWN_9602, 0);
   WRITE(REG_A6XX_PC_UNKNOWN_9E72, 0);
   /* NOTE blob seems to (mostly?) use 0xb2 for SP_TP_MODE_CNTL
    * but this seems to kill texture gather offsets.
    */
   WRITE(REG_A6XX_SP_TP_MODE_CNTL, 0xa0 |
         A6XX_SP_TP_MODE_CNTL_ISAMMODE(ISAMMODE_GL));

   OUT_REG(ring, HLSQ_CONTROL_5_REG(
         CHIP,
         .linelengthregid = INVALID_REG,
         .foveationqualityregid = INVALID_REG,
   ));

   emit_marker6(ring, 7);

   OUT_PKT4(ring, REG_A6XX_VFD_MODE_CNTL, 1);
   OUT_RING(ring, 0x00000000); /* VFD_MODE_CNTL */

   WRITE(REG_A6XX_VFD_MULTIVIEW_CNTL, 0);

   OUT_PKT4(ring, REG_A6XX_PC_MODE_CNTL, 1);
   OUT_RING(ring, 0x0000001f); /* PC_MODE_CNTL */

   /* Clear any potential pending state groups to be safe: */
   OUT_PKT7(ring, CP_SET_DRAW_STATE, 3);
   OUT_RING(ring, CP_SET_DRAW_STATE__0_COUNT(0) |
                     CP_SET_DRAW_STATE__0_DISABLE_ALL_GROUPS |
                     CP_SET_DRAW_STATE__0_GROUP_ID(0));
   OUT_RING(ring, CP_SET_DRAW_STATE__1_ADDR_LO(0));
   OUT_RING(ring, CP_SET_DRAW_STATE__2_ADDR_HI(0));

   OUT_PKT4(ring, REG_A6XX_VPC_SO_STREAM_CNTL, 1);
   OUT_RING(ring, 0x00000000); /* VPC_SO_STREAM_CNTL */

   OUT_PKT4(ring, REG_A6XX_GRAS_LRZ_CNTL, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT4(ring, REG_A6XX_RB_LRZ_CNTL, 1);
   OUT_RING(ring, 0x00000000);

   /* Initialize VFD_FETCH[n].SIZE to zero to avoid iova faults trying
    * to fetch from a VFD_FETCH[n].BASE which we've potentially inherited
    * from another process:
    */
   for (int32_t i = 0; i < 32; i++) {
      OUT_PKT4(ring, REG_A6XX_VFD_FETCH_SIZE(i), 1);
      OUT_RING(ring, 0);
   }

   /* This happens after all drawing has been emitted to the draw CS, so we know
    * whether we need the tess BO pointers.
    */
   if (batch->tessellation) {
      assert(screen->tess_bo);
      fd_ringbuffer_attach_bo(ring, screen->tess_bo);
      OUT_PKT4(ring, REG_A6XX_PC_TESSFACTOR_ADDR, 2);
      OUT_RELOC(ring, screen->tess_bo, 0, 0, 0);
      /* Updating PC_TESSFACTOR_ADDR could race with the next draw which uses it. */
      OUT_WFI5(ring);
   }

   struct fd6_context *fd6_ctx = fd6_context(batch->ctx);
   struct fd_bo *bcolor_mem = fd6_ctx->bcolor_mem;

   OUT_PKT4(ring, REG_A6XX_SP_TP_BORDER_COLOR_BASE_ADDR, 2);
   OUT_RELOC(ring, bcolor_mem, 0, 0, 0);

   OUT_PKT4(ring, REG_A6XX_SP_PS_TP_BORDER_COLOR_BASE_ADDR, 2);
   OUT_RELOC(ring, bcolor_mem, 0, 0, 0);

   if (!batch->nondraw) {
      trace_end_state_restore(&batch->trace, ring);
   }
}

template void fd6_emit_restore<A6XX>(struct fd_batch *batch, struct fd_ringbuffer *ring);
template void fd6_emit_restore<A7XX>(struct fd_batch *batch, struct fd_ringbuffer *ring);

static void
fd6_mem_to_mem(struct fd_ringbuffer *ring, struct pipe_resource *dst,
               unsigned dst_off, struct pipe_resource *src, unsigned src_off,
               unsigned sizedwords)
{
   struct fd_bo *src_bo = fd_resource(src)->bo;
   struct fd_bo *dst_bo = fd_resource(dst)->bo;
   unsigned i;

   fd_ringbuffer_attach_bo(ring, dst_bo);
   fd_ringbuffer_attach_bo(ring, src_bo);

   for (i = 0; i < sizedwords; i++) {
      OUT_PKT7(ring, CP_MEM_TO_MEM, 5);
      OUT_RING(ring, 0x00000000);
      OUT_RELOC(ring, dst_bo, dst_off, 0, 0);
      OUT_RELOC(ring, src_bo, src_off, 0, 0);

      dst_off += 4;
      src_off += 4;
   }
}

void
fd6_emit_init_screen(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);
   screen->mem_to_mem = fd6_mem_to_mem;
}
