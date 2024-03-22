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
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_string.h"

#include "freedreno_resource.h"
#include "freedreno_state.h"

#include "fd4_context.h"
#include "fd4_draw.h"
#include "fd4_emit.h"
#include "fd4_format.h"
#include "fd4_program.h"
#include "fd4_zsa.h"

static void
draw_impl(struct fd_context *ctx, struct fd_ringbuffer *ring,
          struct fd4_emit *emit, unsigned index_offset) assert_dt
{
   const struct pipe_draw_info *info = emit->info;
   enum pc_di_primtype primtype = ctx->screen->primtypes[info->mode];

   fd4_emit_state(ctx, ring, emit);

   if (emit->dirty & (FD_DIRTY_VTXBUF | FD_DIRTY_VTXSTATE))
      fd4_emit_vertex_bufs(ring, emit);

   OUT_PKT0(ring, REG_A4XX_VFD_INDEX_OFFSET, 2);
   OUT_RING(ring, info->index_size ? emit->draw->index_bias
                                   : emit->draw->start); /* VFD_INDEX_OFFSET */
   OUT_RING(ring, info->start_instance);                 /* ??? UNKNOWN_2209 */

   OUT_PKT0(ring, REG_A4XX_PC_RESTART_INDEX, 1);
   OUT_RING(ring, info->primitive_restart ? /* PC_RESTART_INDEX */
                     info->restart_index
                                          : 0xffffffff);

   /* points + psize -> spritelist: */
   if (ctx->rasterizer->point_size_per_vertex &&
       fd4_emit_get_vp(emit)->writes_psize && (info->mode == MESA_PRIM_POINTS))
      primtype = DI_PT_POINTLIST_PSIZE;

   fd4_draw_emit(ctx->batch, ring, primtype,
                 emit->binning_pass ? IGNORE_VISIBILITY : USE_VISIBILITY, info,
                 emit->indirect, emit->draw, index_offset);
}

static bool
fd4_draw_vbo(struct fd_context *ctx, const struct pipe_draw_info *info,
             unsigned drawid_offset,
             const struct pipe_draw_indirect_info *indirect,
             const struct pipe_draw_start_count_bias *draw,
             unsigned index_offset) in_dt
{
   struct fd4_context *fd4_ctx = fd4_context(ctx);
   struct fd4_emit emit = {
      .debug = &ctx->debug,
      .vtx = &ctx->vtx,
      .info = info,
      .drawid_offset = drawid_offset,
      .indirect = indirect,
      .draw = draw,
      .key = {
         .vs = ctx->prog.vs,
         .fs = ctx->prog.fs,
         .key = {
            .rasterflat = ctx->rasterizer->flatshade,
            .ucp_enables = ctx->rasterizer->clip_plane_enable,
            .has_per_samp = fd4_ctx->fastc_srgb || fd4_ctx->vastc_srgb,
            .vastc_srgb = fd4_ctx->vastc_srgb,
            .fastc_srgb = fd4_ctx->fastc_srgb,
         },
      },
      .rasterflat = ctx->rasterizer->flatshade,
      .sprite_coord_enable = ctx->rasterizer->sprite_coord_enable,
      .sprite_coord_mode = ctx->rasterizer->sprite_coord_mode,
   };

   /* Check if we actually need the tg4 workarounds */
   if (ir3_get_shader_info(emit.key.vs)->uses_texture_gather) {
      emit.key.key.has_per_samp = true;
      memcpy(emit.key.key.vsampler_swizzles, fd4_ctx->vsampler_swizzles,
            sizeof(emit.key.key.vsampler_swizzles));
   }
   if (ir3_get_shader_info(emit.key.fs)->uses_texture_gather) {
      emit.key.key.has_per_samp = true;
      memcpy(emit.key.key.fsampler_swizzles, fd4_ctx->fsampler_swizzles,
            sizeof(emit.key.key.fsampler_swizzles));
   }

   if (info->mode != MESA_PRIM_COUNT && !indirect && !info->primitive_restart &&
       !u_trim_pipe_prim(info->mode, (unsigned *)&draw->count))
      return false;

   ir3_fixup_shader_state(&ctx->base, &emit.key.key);

   enum fd_dirty_3d_state dirty = ctx->dirty;

   emit.prog = fd4_program_state(
      ir3_cache_lookup(ctx->shader_cache, &emit.key, &ctx->debug));

   /* bail if compile failed: */
   if (!emit.prog)
      return false;

   fd_blend_tracking(ctx);

   const struct ir3_shader_variant *vp = fd4_emit_get_vp(&emit);
   const struct ir3_shader_variant *fp = fd4_emit_get_fp(&emit);

   ir3_update_max_tf_vtx(ctx, vp);

   /* do regular pass first: */

   if (unlikely(ctx->stats_users > 0)) {
      ctx->stats.vs_regs += ir3_shader_halfregs(vp);
      ctx->stats.fs_regs += ir3_shader_halfregs(fp);
   }

   emit.binning_pass = false;
   emit.dirty = dirty;

   struct fd_ringbuffer *ring = ctx->batch->draw;

   if (ctx->rasterizer->rasterizer_discard) {
      fd_wfi(ctx->batch, ring);
      OUT_PKT3(ring, CP_REG_RMW, 3);
      OUT_RING(ring, REG_A4XX_RB_RENDER_CONTROL);
      OUT_RING(ring, ~A4XX_RB_RENDER_CONTROL_DISABLE_COLOR_PIPE);
      OUT_RING(ring, A4XX_RB_RENDER_CONTROL_DISABLE_COLOR_PIPE);
   }

   draw_impl(ctx, ctx->batch->draw, &emit, index_offset);

   if (ctx->rasterizer->rasterizer_discard) {
      fd_wfi(ctx->batch, ring);
      OUT_PKT3(ring, CP_REG_RMW, 3);
      OUT_RING(ring, REG_A4XX_RB_RENDER_CONTROL);
      OUT_RING(ring, ~A4XX_RB_RENDER_CONTROL_DISABLE_COLOR_PIPE);
      OUT_RING(ring, 0);
   }

   /* and now binning pass: */
   emit.binning_pass = true;
   emit.dirty = dirty & ~(FD_DIRTY_BLEND);
   emit.vs = NULL; /* we changed key so need to refetch vs */
   emit.fs = NULL;
   draw_impl(ctx, ctx->batch->binning, &emit, index_offset);

   fd_context_all_clean(ctx);

   return true;
}

static void
fd4_draw_vbos(struct fd_context *ctx, const struct pipe_draw_info *info,
              unsigned drawid_offset,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draws,
              unsigned num_draws,
              unsigned index_offset)
   assert_dt
{
   for (unsigned i = 0; i < num_draws; i++)
      fd4_draw_vbo(ctx, info, drawid_offset, indirect, &draws[i], index_offset);
}

void
fd4_draw_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->draw_vbos = fd4_draw_vbos;
}
