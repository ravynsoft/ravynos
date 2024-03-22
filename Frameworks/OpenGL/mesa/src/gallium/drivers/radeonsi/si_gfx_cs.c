/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_build_pm4.h"
#include "si_pipe.h"
#include "sid.h"
#include "util/os_time.h"
#include "util/u_log.h"
#include "util/u_upload_mgr.h"
#include "ac_debug.h"
#include "si_utrace.h"

void si_flush_gfx_cs(struct si_context *ctx, unsigned flags, struct pipe_fence_handle **fence)
{
   struct radeon_cmdbuf *cs = &ctx->gfx_cs;
   struct radeon_winsys *ws = ctx->ws;
   struct si_screen *sscreen = ctx->screen;
   const unsigned wait_ps_cs = SI_CONTEXT_PS_PARTIAL_FLUSH | SI_CONTEXT_CS_PARTIAL_FLUSH;
   unsigned wait_flags = 0;

   if (ctx->gfx_flush_in_progress)
      return;

   /* The amdgpu kernel driver synchronizes execution for shared DMABUFs between
    * processes on DRM >= 3.39.0, so we don't have to wait at the end of IBs to
    * make sure everything is idle.
    *
    * The amdgpu winsys synchronizes execution for buffers shared by different
    * contexts within the same process.
    *
    * Interop with AMDVLK, RADV, or OpenCL within the same process requires
    * explicit fences or glFinish.
    */
   if (sscreen->info.is_amdgpu && sscreen->info.drm_minor >= 39)
      flags |= RADEON_FLUSH_START_NEXT_GFX_IB_NOW;

   if (ctx->gfx_level == GFX6) {
      /* The kernel flushes L2 before shaders are finished. */
      wait_flags |= wait_ps_cs;
   } else if (!(flags & RADEON_FLUSH_START_NEXT_GFX_IB_NOW) ||
              ((flags & RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION) &&
                !ws->cs_is_secure(cs))) {
      /* TODO: this workaround fixes subtitles rendering with mpv -vo=vaapi and
       * tmz but shouldn't be necessary.
       */
      wait_flags |= wait_ps_cs;
   }

   /* Drop this flush if it's a no-op. */
   if (!radeon_emitted(cs, ctx->initial_gfx_cs_size) &&
       (!wait_flags || !ctx->gfx_last_ib_is_busy) &&
       !(flags & RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION)) {
      tc_driver_internal_flush_notify(ctx->tc);
      return;
   }

   /* Non-aux contexts must set up no-op API dispatch on GPU resets. This is
    * similar to si_get_reset_status but here we can ignore soft-recoveries,
    * while si_get_reset_status can't. */
   if (!(ctx->context_flags & SI_CONTEXT_FLAG_AUX) &&
       ctx->device_reset_callback.reset) {
      enum pipe_reset_status status = ctx->ws->ctx_query_reset_status(ctx->ctx, true, NULL, NULL);
      if (status != PIPE_NO_RESET)
         ctx->device_reset_callback.reset(ctx->device_reset_callback.data, status);
   }

   if (sscreen->debug_flags & DBG(CHECK_VM))
      flags &= ~PIPE_FLUSH_ASYNC;

   ctx->gfx_flush_in_progress = true;

   if (ctx->has_graphics) {
      if (!list_is_empty(&ctx->active_queries))
         si_suspend_queries(ctx);

      ctx->streamout.suspended = false;
      if (ctx->streamout.begin_emitted) {
         si_emit_streamout_end(ctx);
         ctx->streamout.suspended = true;
      }
   }

   /* Make sure CP DMA is idle at the end of IBs after L2 prefetches
    * because the kernel doesn't wait for it. */
   if (ctx->gfx_level >= GFX7)
      si_cp_dma_wait_for_idle(ctx, &ctx->gfx_cs);

   /* If we use s_sendmsg to set tess factors to all 0 or all 1 instead of writing to the tess
    * factor buffer, we need this at the end of command buffers:
    */
   if ((ctx->gfx_level == GFX11 || ctx->gfx_level == GFX11_5) && ctx->tess_rings) {
      radeon_begin(cs);
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_SQ_NON_EVENT) | EVENT_INDEX(0));
      radeon_end();
   }

   /* Wait for draw calls to finish if needed. */
   if (wait_flags) {
      ctx->flags |= wait_flags;
      si_emit_cache_flush_direct(ctx);
   }
   ctx->gfx_last_ib_is_busy = (wait_flags & wait_ps_cs) != wait_ps_cs;

   if (ctx->current_saved_cs) {
      si_trace_emit(ctx);

      /* Save the IB for debug contexts. */
      si_save_cs(ws, cs, &ctx->current_saved_cs->gfx, true);
      ctx->current_saved_cs->flushed = true;
      ctx->current_saved_cs->time_flush = os_time_get_nano();

      si_log_hw_flush(ctx);
   }

   if (sscreen->debug_flags & DBG(IB))
      si_print_current_ib(ctx, stderr);

   if (sscreen->context_roll_log_filename)
      si_gather_context_rolls(ctx);

   if (ctx->is_noop)
      flags |= RADEON_FLUSH_NOOP;

   uint64_t start_ts = 0, submission_id = 0;
   if (u_trace_perfetto_active(&ctx->ds.trace_context)) {
      start_ts = si_ds_begin_submit(&ctx->ds_queue);
      submission_id = ctx->ds_queue.submission_id;
   }

   /* Flush the CS. */
   ws->cs_flush(cs, flags, &ctx->last_gfx_fence);

   if (u_trace_perfetto_active(&ctx->ds.trace_context) && start_ts > 0) {
      si_ds_end_submit(&ctx->ds_queue, start_ts);
   }

   tc_driver_internal_flush_notify(ctx->tc);
   if (fence)
      ws->fence_reference(ws, fence, ctx->last_gfx_fence);

   ctx->num_gfx_cs_flushes++;

   /* Check VM faults if needed. */
   if (sscreen->debug_flags & DBG(CHECK_VM)) {
      /* Use conservative timeout 800ms, after which we won't wait any
       * longer and assume the GPU is hung.
       */
      ctx->ws->fence_wait(ctx->ws, ctx->last_gfx_fence, 800 * 1000 * 1000);

      si_check_vm_faults(ctx, &ctx->current_saved_cs->gfx, AMD_IP_GFX);
   }

   if (unlikely(ctx->sqtt && (flags & PIPE_FLUSH_END_OF_FRAME))) {
      si_handle_sqtt(ctx, &ctx->gfx_cs);
   }

   if (ctx->current_saved_cs)
      si_saved_cs_reference(&ctx->current_saved_cs, NULL);

   if (u_trace_perfetto_active(&ctx->ds.trace_context))
      si_utrace_flush(ctx, submission_id);

   si_begin_new_gfx_cs(ctx, false);
   ctx->gfx_flush_in_progress = false;
}

static void si_begin_gfx_cs_debug(struct si_context *ctx)
{
   static const uint32_t zeros[1];
   assert(!ctx->current_saved_cs);

   ctx->current_saved_cs = calloc(1, sizeof(*ctx->current_saved_cs));
   if (!ctx->current_saved_cs)
      return;

   pipe_reference_init(&ctx->current_saved_cs->reference, 1);

   ctx->current_saved_cs->trace_buf =
      si_resource(pipe_buffer_create(ctx->b.screen, 0, PIPE_USAGE_STAGING, 4));
   if (!ctx->current_saved_cs->trace_buf) {
      free(ctx->current_saved_cs);
      ctx->current_saved_cs = NULL;
      return;
   }

   pipe_buffer_write_nooverlap(&ctx->b, &ctx->current_saved_cs->trace_buf->b.b, 0, sizeof(zeros),
                               zeros);
   ctx->current_saved_cs->trace_id = 0;

   si_trace_emit(ctx);

   radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, ctx->current_saved_cs->trace_buf,
                             RADEON_USAGE_READWRITE | RADEON_PRIO_FENCE_TRACE);
}

static void si_add_gds_to_buffer_list(struct si_context *sctx)
{
}

void si_set_tracked_regs_to_clear_state(struct si_context *ctx)
{
   STATIC_ASSERT(SI_NUM_ALL_TRACKED_REGS <= sizeof(ctx->tracked_regs.reg_saved_mask) * 8);

   ctx->tracked_regs.reg_value[SI_TRACKED_DB_RENDER_CONTROL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_DB_COUNT_CONTROL] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_DB_DEPTH_CONTROL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_DB_STENCIL_CONTROL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_DB_DEPTH_BOUNDS_MIN] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_DB_DEPTH_BOUNDS_MAX] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_INTERP_CONTROL_0] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_POINT_SIZE] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_POINT_MINMAX] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_LINE_CNTL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SC_MODE_CNTL_0] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_SC_MODE_CNTL] = 0x4;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SC_EDGERULE] = 0xaa99aaaa;

   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_POLY_OFFSET_DB_FMT_CNTL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_POLY_OFFSET_CLAMP] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_POLY_OFFSET_FRONT_SCALE] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_POLY_OFFSET_FRONT_OFFSET] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_POLY_OFFSET_BACK_SCALE] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_POLY_OFFSET_BACK_OFFSET] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SC_LINE_CNTL] = 0x1000;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SC_AA_CONFIG] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_VTX_CNTL] = 0x5;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_CL_GB_VERT_CLIP_ADJ] = 0x3f800000;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_CL_GB_VERT_DISC_ADJ] = 0x3f800000;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_CL_GB_HORZ_CLIP_ADJ] = 0x3f800000;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_CL_GB_HORZ_DISC_ADJ] = 0x3f800000;

   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_SHADER_POS_FORMAT] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_SHADER_Z_FORMAT] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_SHADER_COL_FORMAT] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_BARYC_CNTL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_PS_INPUT_ENA] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_PS_INPUT_ADDR] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_DB_EQAA] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_DB_SHADER_CONTROL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_CB_SHADER_MASK] = 0xffffffff;
   ctx->tracked_regs.reg_value[SI_TRACKED_CB_TARGET_MASK] = 0xffffffff;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_CL_CLIP_CNTL] = 0x90000;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_CL_VS_OUT_CNTL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_CL_VTE_CNTL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SC_CLIPRECT_RULE] = 0xffff;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SC_LINE_STIPPLE] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SC_MODE_CNTL_1] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_HARDWARE_SCREEN_OFFSET] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_PS_IN_CONTROL] = 0x2;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_INSTANCE_CNT] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_MAX_VERT_OUT] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_SHADER_STAGES_EN] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_LS_HS_CONFIG] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_TF_PARAM] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SU_SMALL_PRIM_FILTER_CNTL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_SC_BINNER_CNTL_0] = 0x3;
   ctx->tracked_regs.reg_value[SI_TRACKED_GE_MAX_OUTPUT_PER_SUBGROUP] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_GE_NGG_SUBGRP_CNTL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_PA_CL_NGG_CNTL] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_DB_PA_SC_VRS_OVERRIDE_CNTL] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_SX_PS_DOWNCONVERT] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_SX_BLEND_OPT_EPSILON] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_SX_BLEND_OPT_CONTROL] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_ESGS_RING_ITEMSIZE] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_REUSE_OFF] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_IA_MULTI_VGT_PARAM] = 0xff;

   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_MAX_PRIMS_PER_SUBGROUP] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_ONCHIP_CNTL] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GSVS_RING_ITEMSIZE] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_MODE] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_VERTEX_REUSE_BLOCK_CNTL] = 0x1e;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_OUT_PRIM_TYPE] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GSVS_RING_OFFSET_1] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GSVS_RING_OFFSET_2] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GSVS_RING_OFFSET_3] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_VERT_ITEMSIZE] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_VERT_ITEMSIZE_1] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_VERT_ITEMSIZE_2] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_GS_VERT_ITEMSIZE_3] = 0;

   ctx->tracked_regs.reg_value[SI_TRACKED_DB_RENDER_OVERRIDE2] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_SPI_VS_OUT_CONFIG] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_VGT_PRIMITIVEID_EN] = 0;
   ctx->tracked_regs.reg_value[SI_TRACKED_CB_DCC_CONTROL] = 0;

   /* Set all cleared context registers to saved. */
   BITSET_SET_RANGE(ctx->tracked_regs.reg_saved_mask, 0, SI_NUM_TRACKED_CONTEXT_REGS - 1);
}

void si_install_draw_wrapper(struct si_context *sctx, pipe_draw_func wrapper,
                             pipe_draw_vertex_state_func vstate_wrapper)
{
   if (wrapper) {
      if (wrapper != sctx->b.draw_vbo) {
         assert(!sctx->real_draw_vbo);
         assert(!sctx->real_draw_vertex_state);
         sctx->real_draw_vbo = sctx->b.draw_vbo;
         sctx->real_draw_vertex_state = sctx->b.draw_vertex_state;
         sctx->b.draw_vbo = wrapper;
         sctx->b.draw_vertex_state = vstate_wrapper;
      }
   } else if (sctx->real_draw_vbo) {
      sctx->real_draw_vbo = NULL;
      sctx->real_draw_vertex_state = NULL;
      si_select_draw_vbo(sctx);
   }
}

static void si_tmz_preamble(struct si_context *sctx)
{
   bool secure = si_gfx_resources_check_encrypted(sctx);
   if (secure != sctx->ws->cs_is_secure(&sctx->gfx_cs)) {
      si_flush_gfx_cs(sctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW |
                            RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION, NULL);
   }
}

static void si_draw_vbo_tmz_preamble(struct pipe_context *ctx,
                                     const struct pipe_draw_info *info,
                                     unsigned drawid_offset,
                                     const struct pipe_draw_indirect_info *indirect,
                                     const struct pipe_draw_start_count_bias *draws,
                                     unsigned num_draws) {
   struct si_context *sctx = (struct si_context *)ctx;

   si_tmz_preamble(sctx);
   sctx->real_draw_vbo(ctx, info, drawid_offset, indirect, draws, num_draws);
}

static void si_draw_vstate_tmz_preamble(struct pipe_context *ctx,
                                        struct pipe_vertex_state *state,
                                        uint32_t partial_velem_mask,
                                        struct pipe_draw_vertex_state_info info,
                                        const struct pipe_draw_start_count_bias *draws,
                                        unsigned num_draws) {
   struct si_context *sctx = (struct si_context *)ctx;

   si_tmz_preamble(sctx);
   sctx->real_draw_vertex_state(ctx, state, partial_velem_mask, info, draws, num_draws);
}

void si_begin_new_gfx_cs(struct si_context *ctx, bool first_cs)
{
   bool is_secure = false;

   if (!first_cs)
      u_trace_fini(&ctx->trace);

   u_trace_init(&ctx->trace, &ctx->ds.trace_context);

   if (unlikely(radeon_uses_secure_bos(ctx->ws))) {
      is_secure = ctx->ws->cs_is_secure(&ctx->gfx_cs);

      si_install_draw_wrapper(ctx, si_draw_vbo_tmz_preamble,
                              si_draw_vstate_tmz_preamble);
   }

   if (ctx->is_debug)
      si_begin_gfx_cs_debug(ctx);

   if (ctx->screen->gds_oa)
      ctx->ws->cs_add_buffer(&ctx->gfx_cs, ctx->screen->gds_oa, RADEON_USAGE_READWRITE, 0);

   /* Always invalidate caches at the beginning of IBs, because external
    * users (e.g. BO evictions and SDMA/UVD/VCE IBs) can modify our
    * buffers.
    *
    * Gfx10+ automatically invalidates I$, SMEM$, VMEM$, and GL1$ at the beginning of IBs,
    * so we only need to flush the GL2 cache.
    *
    * Note that the cache flush done by the kernel at the end of GFX IBs
    * isn't useful here, because that flush can finish after the following
    * IB starts drawing.
    *
    * TODO: Do we also need to invalidate CB & DB caches?
    */
   ctx->flags |= SI_CONTEXT_INV_L2;
   if (ctx->gfx_level < GFX10)
      ctx->flags |= SI_CONTEXT_INV_ICACHE | SI_CONTEXT_INV_SCACHE | SI_CONTEXT_INV_VCACHE;

   /* Disable pipeline stats if there are no active queries. */
   ctx->flags &= ~SI_CONTEXT_START_PIPELINE_STATS & ~SI_CONTEXT_STOP_PIPELINE_STATS;
   if (ctx->num_hw_pipestat_streamout_queries)
      ctx->flags |= SI_CONTEXT_START_PIPELINE_STATS;
   else
      ctx->flags |= SI_CONTEXT_STOP_PIPELINE_STATS;

   ctx->pipeline_stats_enabled = -1; /* indicate that the current hw state is unknown */

   /* We don't know if the last draw used NGG because it can be a different process.
    * When switching NGG->legacy, we need to flush VGT for certain hw generations.
    */
   if (ctx->screen->info.has_vgt_flush_ngg_legacy_bug && !ctx->ngg)
      ctx->flags |= SI_CONTEXT_VGT_FLUSH;

   si_mark_atom_dirty(ctx, &ctx->atoms.s.cache_flush);

   if (ctx->screen->attribute_ring) {
      radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, ctx->screen->attribute_ring,
                                RADEON_USAGE_READWRITE | RADEON_PRIO_SHADER_RINGS);
   }
   if (ctx->border_color_buffer) {
      radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, ctx->border_color_buffer,
                                RADEON_USAGE_READ | RADEON_PRIO_BORDER_COLORS);
   }
   if (ctx->shadowing.registers) {
      radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, ctx->shadowing.registers,
                                RADEON_USAGE_READWRITE | RADEON_PRIO_DESCRIPTORS);

      if (ctx->shadowing.csa)
         radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, ctx->shadowing.csa,
                                   RADEON_USAGE_READWRITE | RADEON_PRIO_DESCRIPTORS);
   }

   si_add_all_descriptors_to_bo_list(ctx);
   si_shader_pointers_mark_dirty(ctx);
   ctx->cs_shader_state.emitted_program = NULL;

   /* The CS initialization should be emitted before everything else. */
   if (ctx->cs_preamble_state) {
      struct si_pm4_state *preamble = is_secure ? ctx->cs_preamble_state_tmz :
                                                  ctx->cs_preamble_state;
      radeon_begin(&ctx->gfx_cs);
      radeon_emit_array(preamble->pm4, preamble->ndw);
      radeon_end();
   }

   if (!ctx->has_graphics) {
      ctx->initial_gfx_cs_size = ctx->gfx_cs.current.cdw;
      return;
   }

   if (ctx->tess_rings) {
      radeon_add_to_buffer_list(ctx, &ctx->gfx_cs,
                                unlikely(is_secure) ? si_resource(ctx->tess_rings_tmz) : si_resource(ctx->tess_rings),
                                RADEON_USAGE_READWRITE | RADEON_PRIO_SHADER_RINGS);
   }

   /* set all valid group as dirty so they get reemited on
    * next draw command
    */
   si_pm4_reset_emitted(ctx);

   if (ctx->queued.named.ls)
      ctx->prefetch_L2_mask |= SI_PREFETCH_LS;
   if (ctx->queued.named.hs)
      ctx->prefetch_L2_mask |= SI_PREFETCH_HS;
   if (ctx->queued.named.es)
      ctx->prefetch_L2_mask |= SI_PREFETCH_ES;
   if (ctx->queued.named.gs)
      ctx->prefetch_L2_mask |= SI_PREFETCH_GS;
   if (ctx->queued.named.vs)
      ctx->prefetch_L2_mask |= SI_PREFETCH_VS;
   if (ctx->queued.named.ps)
      ctx->prefetch_L2_mask |= SI_PREFETCH_PS;

   /* CLEAR_STATE disables all colorbuffers, so only enable bound ones. */
   bool has_clear_state = ctx->screen->info.has_clear_state;
   if (has_clear_state) {
      ctx->framebuffer.dirty_cbufs =
            u_bit_consecutive(0, ctx->framebuffer.state.nr_cbufs);
      /* CLEAR_STATE disables the zbuffer, so only enable it if it's bound. */
      ctx->framebuffer.dirty_zsbuf = ctx->framebuffer.state.zsbuf != NULL;
   } else {
      ctx->framebuffer.dirty_cbufs = u_bit_consecutive(0, 8);
      ctx->framebuffer.dirty_zsbuf = true;
   }

   /* RB+ depth-only rendering needs to set CB_COLOR0_INFO differently from CLEAR_STATE. */
   if (ctx->screen->info.rbplus_allowed)
      ctx->framebuffer.dirty_cbufs |= 0x1;

   /* GFX11+ needs to set NUM_SAMPLES differently from CLEAR_STATE. */
   if (ctx->gfx_level >= GFX11)
      ctx->framebuffer.dirty_zsbuf = true;

   /* Even with shadowed registers, we have to add buffers to the buffer list.
    * These atoms are the only ones that add buffers.
    *
    * The framebuffer state also needs to set PA_SC_WINDOW_SCISSOR_BR differently from CLEAR_STATE.
    */
   si_mark_atom_dirty(ctx, &ctx->atoms.s.framebuffer);
   si_mark_atom_dirty(ctx, &ctx->atoms.s.render_cond);
   if (ctx->screen->use_ngg_culling)
      si_mark_atom_dirty(ctx, &ctx->atoms.s.ngg_cull_state);

   if (first_cs || !ctx->shadowing.registers) {
      /* These don't add any buffers, so skip them with shadowing. */
      si_mark_atom_dirty(ctx, &ctx->atoms.s.clip_regs);
      /* CLEAR_STATE sets zeros. */
      if (!has_clear_state || ctx->clip_state_any_nonzeros)
         si_mark_atom_dirty(ctx, &ctx->atoms.s.clip_state);
      ctx->sample_locs_num_samples = 0;
      si_mark_atom_dirty(ctx, &ctx->atoms.s.sample_locations);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.msaa_config);
      /* CLEAR_STATE sets 0xffff. */
      if (!has_clear_state || ctx->sample_mask != 0xffff)
         si_mark_atom_dirty(ctx, &ctx->atoms.s.sample_mask);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.cb_render_state);
      /* CLEAR_STATE sets zeros. */
      if (!has_clear_state || ctx->blend_color_any_nonzeros)
         si_mark_atom_dirty(ctx, &ctx->atoms.s.blend_color);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.db_render_state);
      if (ctx->gfx_level >= GFX9)
         si_mark_atom_dirty(ctx, &ctx->atoms.s.dpbb_state);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.stencil_ref);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.spi_map);
      if (ctx->gfx_level < GFX11)
         si_mark_atom_dirty(ctx, &ctx->atoms.s.streamout_enable);
      /* CLEAR_STATE disables all window rectangles. */
      if (!has_clear_state || ctx->num_window_rectangles > 0)
         si_mark_atom_dirty(ctx, &ctx->atoms.s.window_rectangles);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.guardband);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.scissors);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.viewports);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.vgt_pipeline_state);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.tess_io_layout);

      /* Set all register values to unknown. */
      BITSET_ZERO(ctx->tracked_regs.reg_saved_mask);

      if (has_clear_state)
         si_set_tracked_regs_to_clear_state(ctx);

      /* 0xffffffff is an impossible value for SPI_PS_INPUT_CNTL_n registers */
      memset(ctx->tracked_regs.spi_ps_input_cntl, 0xff, sizeof(uint32_t) * 32);
   }

   /* Invalidate various draw states so that they are emitted before
    * the first draw call. */
   ctx->last_instance_count = SI_INSTANCE_COUNT_UNKNOWN;
   ctx->last_index_size = -1;
   /* Primitive restart is set to false by the gfx preamble on GFX11+. */
   ctx->last_primitive_restart_en = ctx->gfx_level >= GFX11 ? false : -1;
   ctx->last_restart_index = SI_RESTART_INDEX_UNKNOWN;
   ctx->last_prim = -1;
   ctx->last_vs_state = ~0;
   ctx->last_gs_state = ~0;
   ctx->last_ls = NULL;
   ctx->last_tcs = NULL;
   ctx->last_tes_sh_base = -1;
   ctx->last_num_tcs_input_cp = -1;

   assert(ctx->num_buffered_gfx_sh_regs == 0);
   assert(ctx->num_buffered_compute_sh_regs == 0);
   ctx->num_buffered_gfx_sh_regs = 0;
   ctx->num_buffered_compute_sh_regs = 0;

   if (ctx->scratch_buffer)
      si_mark_atom_dirty(ctx, &ctx->atoms.s.scratch_state);

   if (ctx->streamout.suspended) {
      ctx->streamout.append_bitmask = ctx->streamout.enabled_mask;
      si_streamout_buffers_dirty(ctx);
   }

   if (!list_is_empty(&ctx->active_queries))
      si_resume_queries(ctx);

   assert(!ctx->gfx_cs.prev_dw);
   ctx->initial_gfx_cs_size = ctx->gfx_cs.current.cdw;

   /* All buffer references are removed on a flush, so si_check_needs_implicit_sync
    * cannot determine if si_make_CB_shader_coherent() needs to be called.
    * ctx->force_cb_shader_coherent will be cleared by the first call to
    * si_make_CB_shader_coherent.
    */
   ctx->force_cb_shader_coherent = true;
}

void si_trace_emit(struct si_context *sctx)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   uint32_t trace_id = ++sctx->current_saved_cs->trace_id;

   si_cp_write_data(sctx, sctx->current_saved_cs->trace_buf, 0, 4, V_370_MEM, V_370_ME, &trace_id);

   radeon_begin(cs);
   radeon_emit(PKT3(PKT3_NOP, 0, 0));
   radeon_emit(AC_ENCODE_TRACE_POINT(trace_id));
   radeon_end();

   if (sctx->log)
      u_log_flush(sctx->log);
}

/* timestamp logging for u_trace: */
void si_emit_ts(struct si_context *sctx, struct si_resource* buffer, unsigned int offset)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   uint64_t va = buffer->gpu_address + offset;
   si_cp_release_mem(sctx, cs, V_028A90_BOTTOM_OF_PIPE_TS, 0, EOP_DST_SEL_MEM, EOP_INT_SEL_NONE,
                        EOP_DATA_SEL_TIMESTAMP, buffer, va, 0, PIPE_QUERY_TIMESTAMP);
}

void si_emit_surface_sync(struct si_context *sctx, struct radeon_cmdbuf *cs, unsigned cp_coher_cntl)
{
   bool compute_ib = !sctx->has_graphics;

   assert(sctx->gfx_level <= GFX9);

   /* This seems problematic with GFX7 (see #4764) */
   if (sctx->gfx_level != GFX7)
      cp_coher_cntl |= 1u << 31; /* don't sync PFP, i.e. execute the sync in ME */

   radeon_begin(cs);

   if (sctx->gfx_level == GFX9 || compute_ib) {
      /* Flush caches and wait for the caches to assert idle. */
      radeon_emit(PKT3(PKT3_ACQUIRE_MEM, 5, 0));
      radeon_emit(cp_coher_cntl); /* CP_COHER_CNTL */
      radeon_emit(0xffffffff);    /* CP_COHER_SIZE */
      radeon_emit(0xffffff);      /* CP_COHER_SIZE_HI */
      radeon_emit(0);             /* CP_COHER_BASE */
      radeon_emit(0);             /* CP_COHER_BASE_HI */
      radeon_emit(0x0000000A);    /* POLL_INTERVAL */
   } else {
      /* ACQUIRE_MEM is only required on a compute ring. */
      radeon_emit(PKT3(PKT3_SURFACE_SYNC, 3, 0));
      radeon_emit(cp_coher_cntl); /* CP_COHER_CNTL */
      radeon_emit(0xffffffff);    /* CP_COHER_SIZE */
      radeon_emit(0);             /* CP_COHER_BASE */
      radeon_emit(0x0000000A);    /* POLL_INTERVAL */
   }
   radeon_end();

   /* ACQUIRE_MEM has an implicit context roll if the current context
    * is busy. */
   if (!compute_ib)
      sctx->context_roll = true;
}

static struct si_resource *si_get_wait_mem_scratch_bo(struct si_context *ctx,
                                                      struct radeon_cmdbuf *cs, bool is_secure)
{
   struct si_screen *sscreen = ctx->screen;

   assert(ctx->gfx_level < GFX11);

   if (likely(!is_secure)) {
      return ctx->wait_mem_scratch;
   } else {
      assert(sscreen->info.has_tmz_support);
      if (!ctx->wait_mem_scratch_tmz) {
         ctx->wait_mem_scratch_tmz =
            si_aligned_buffer_create(&sscreen->b,
                                     PIPE_RESOURCE_FLAG_UNMAPPABLE |
                                     SI_RESOURCE_FLAG_DRIVER_INTERNAL |
                                     PIPE_RESOURCE_FLAG_ENCRYPTED,
                                     PIPE_USAGE_DEFAULT, 4,
                                     sscreen->info.tcc_cache_line_size);
         si_cp_write_data(ctx, ctx->wait_mem_scratch_tmz, 0, 4, V_370_MEM, V_370_ME,
                          &ctx->wait_mem_number);
      }

      return ctx->wait_mem_scratch_tmz;
   }
}

void gfx10_emit_cache_flush(struct si_context *ctx, struct radeon_cmdbuf *cs)
{
   uint32_t gcr_cntl = 0;
   unsigned cb_db_event = 0;
   unsigned flags = ctx->flags;

   if (!flags)
      return;

   if (!ctx->has_graphics) {
      /* Only process compute flags. */
      flags &= SI_CONTEXT_INV_ICACHE | SI_CONTEXT_INV_SCACHE | SI_CONTEXT_INV_VCACHE |
               SI_CONTEXT_INV_L2 | SI_CONTEXT_WB_L2 | SI_CONTEXT_INV_L2_METADATA |
               SI_CONTEXT_CS_PARTIAL_FLUSH;
   }

   /* We don't need these. */
   assert(!(flags & (SI_CONTEXT_VGT_STREAMOUT_SYNC | SI_CONTEXT_FLUSH_AND_INV_DB_META)));

   radeon_begin(cs);

   if (flags & SI_CONTEXT_VGT_FLUSH) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_VGT_FLUSH) | EVENT_INDEX(0));
   }

   if (flags & SI_CONTEXT_FLUSH_AND_INV_CB)
      ctx->num_cb_cache_flushes++;
   if (flags & SI_CONTEXT_FLUSH_AND_INV_DB)
      ctx->num_db_cache_flushes++;

   if (flags & SI_CONTEXT_INV_ICACHE)
      gcr_cntl |= S_586_GLI_INV(V_586_GLI_ALL);
   if (flags & SI_CONTEXT_INV_SCACHE) {
      /* TODO: When writing to the SMEM L1 cache, we need to set SEQ
       * to FORWARD when both L1 and L2 are written out (WB or INV).
       */
      gcr_cntl |= S_586_GL1_INV(1) | S_586_GLK_INV(1);
   }
   if (flags & SI_CONTEXT_INV_VCACHE)
      gcr_cntl |= S_586_GL1_INV(1) | S_586_GLV_INV(1);

   /* The L2 cache ops are:
    * - INV: - invalidate lines that reflect memory (were loaded from memory)
    *        - don't touch lines that were overwritten (were stored by gfx clients)
    * - WB: - don't touch lines that reflect memory
    *       - write back lines that were overwritten
    * - WB | INV: - invalidate lines that reflect memory
    *             - write back lines that were overwritten
    *
    * GLM doesn't support WB alone. If WB is set, INV must be set too.
    */
   if (flags & SI_CONTEXT_INV_L2) {
      /* Writeback and invalidate everything in L2. */
      gcr_cntl |= S_586_GL2_INV(1) | S_586_GL2_WB(1) | S_586_GLM_INV(1) | S_586_GLM_WB(1);
      ctx->num_L2_invalidates++;
   } else if (flags & SI_CONTEXT_WB_L2) {
      gcr_cntl |= S_586_GL2_WB(1) | S_586_GLM_WB(1) | S_586_GLM_INV(1);
   } else if (flags & SI_CONTEXT_INV_L2_METADATA) {
      gcr_cntl |= S_586_GLM_INV(1) | S_586_GLM_WB(1);
   }

   if (flags & (SI_CONTEXT_FLUSH_AND_INV_CB | SI_CONTEXT_FLUSH_AND_INV_DB)) {
      if (flags & SI_CONTEXT_FLUSH_AND_INV_CB) {
         /* Flush CMASK/FMASK/DCC. Will wait for idle later. */
         radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(EVENT_TYPE(V_028A90_FLUSH_AND_INV_CB_META) | EVENT_INDEX(0));
      }

      /* Gfx11 can't flush DB_META and should use a TS event instead. */
      if (ctx->gfx_level != GFX11 && flags & SI_CONTEXT_FLUSH_AND_INV_DB) {
         /* Flush HTILE. Will wait for idle later. */
         radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(EVENT_TYPE(V_028A90_FLUSH_AND_INV_DB_META) | EVENT_INDEX(0));
      }

      /* First flush CB/DB, then L1/L2. */
      gcr_cntl |= S_586_SEQ(V_586_SEQ_FORWARD);

      if ((flags & (SI_CONTEXT_FLUSH_AND_INV_CB | SI_CONTEXT_FLUSH_AND_INV_DB)) ==
          (SI_CONTEXT_FLUSH_AND_INV_CB | SI_CONTEXT_FLUSH_AND_INV_DB)) {
         cb_db_event = V_028A90_CACHE_FLUSH_AND_INV_TS_EVENT;
      } else if (flags & SI_CONTEXT_FLUSH_AND_INV_CB) {
         cb_db_event = V_028A90_FLUSH_AND_INV_CB_DATA_TS;
      } else if (flags & SI_CONTEXT_FLUSH_AND_INV_DB) {
         if (ctx->gfx_level == GFX11)
            cb_db_event = V_028A90_CACHE_FLUSH_AND_INV_TS_EVENT;
         else
            cb_db_event = V_028A90_FLUSH_AND_INV_DB_DATA_TS;
      } else {
         assert(0);
      }
   } else {
      /* Wait for graphics shaders to go idle if requested. */
      if (flags & SI_CONTEXT_PS_PARTIAL_FLUSH) {
         radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(EVENT_TYPE(V_028A90_PS_PARTIAL_FLUSH) | EVENT_INDEX(4));
         /* Only count explicit shader flushes, not implicit ones. */
         ctx->num_vs_flushes++;
         ctx->num_ps_flushes++;
      } else if (flags & SI_CONTEXT_VS_PARTIAL_FLUSH) {
         radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(EVENT_TYPE(V_028A90_VS_PARTIAL_FLUSH) | EVENT_INDEX(4));
         ctx->num_vs_flushes++;
      }
   }

   if (flags & SI_CONTEXT_CS_PARTIAL_FLUSH && ctx->compute_is_busy) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_CS_PARTIAL_FLUSH | EVENT_INDEX(4)));
      ctx->num_cs_flushes++;
      ctx->compute_is_busy = false;
   }

   if (cb_db_event) {
      if (ctx->gfx_level >= GFX11) {
         /* Get GCR_CNTL fields, because the encoding is different in RELEASE_MEM. */
         unsigned glm_wb = G_586_GLM_WB(gcr_cntl);
         unsigned glm_inv = G_586_GLM_INV(gcr_cntl);
         unsigned glk_wb = G_586_GLK_WB(gcr_cntl);
         unsigned glk_inv = G_586_GLK_INV(gcr_cntl);
         unsigned glv_inv = G_586_GLV_INV(gcr_cntl);
         unsigned gl1_inv = G_586_GL1_INV(gcr_cntl);
         assert(G_586_GL2_US(gcr_cntl) == 0);
         assert(G_586_GL2_RANGE(gcr_cntl) == 0);
         assert(G_586_GL2_DISCARD(gcr_cntl) == 0);
         unsigned gl2_inv = G_586_GL2_INV(gcr_cntl);
         unsigned gl2_wb = G_586_GL2_WB(gcr_cntl);
         unsigned gcr_seq = G_586_SEQ(gcr_cntl);

         gcr_cntl &= C_586_GLM_WB & C_586_GLM_INV & C_586_GLK_WB & C_586_GLK_INV &
                     C_586_GLV_INV & C_586_GL1_INV & C_586_GL2_INV & C_586_GL2_WB; /* keep SEQ */

         /* Send an event that flushes caches. */
         radeon_emit(PKT3(PKT3_RELEASE_MEM, 6, 0));
         radeon_emit(S_490_EVENT_TYPE(cb_db_event) |
                     S_490_EVENT_INDEX(5) |
                     S_490_GLM_WB(glm_wb) | S_490_GLM_INV(glm_inv) | S_490_GLV_INV(glv_inv) |
                     S_490_GL1_INV(gl1_inv) | S_490_GL2_INV(gl2_inv) | S_490_GL2_WB(gl2_wb) |
                     S_490_SEQ(gcr_seq) | S_490_GLK_WB(glk_wb) | S_490_GLK_INV(glk_inv) |
                     S_490_PWS_ENABLE(1));
         radeon_emit(0); /* DST_SEL, INT_SEL, DATA_SEL */
         radeon_emit(0); /* ADDRESS_LO */
         radeon_emit(0); /* ADDRESS_HI */
         radeon_emit(0); /* DATA_LO */
         radeon_emit(0); /* DATA_HI */
         radeon_emit(0); /* INT_CTXID */

         if (unlikely(ctx->sqtt_enabled)) {
            radeon_end();
            si_sqtt_describe_barrier_start(ctx, &ctx->gfx_cs);
            radeon_begin_again(cs);
         }

         /* Wait for the event and invalidate remaining caches if needed. */
         radeon_emit(PKT3(PKT3_ACQUIRE_MEM, 6, 0));
         radeon_emit(S_580_PWS_STAGE_SEL(flags & SI_CONTEXT_PFP_SYNC_ME ? V_580_CP_PFP :
                                                                          V_580_CP_ME) |
                     S_580_PWS_COUNTER_SEL(V_580_TS_SELECT) |
                     S_580_PWS_ENA2(1) |
                     S_580_PWS_COUNT(0));
         radeon_emit(0xffffffff); /* GCR_SIZE */
         radeon_emit(0x01ffffff); /* GCR_SIZE_HI */
         radeon_emit(0); /* GCR_BASE_LO */
         radeon_emit(0); /* GCR_BASE_HI */
         radeon_emit(S_585_PWS_ENA(1));
         radeon_emit(gcr_cntl); /* GCR_CNTL */

         if (unlikely(ctx->sqtt_enabled)) {
            radeon_end();
            si_sqtt_describe_barrier_end(ctx, &ctx->gfx_cs, flags);
            radeon_begin_again(cs);
         }

         gcr_cntl = 0; /* all done */
         flags &= ~SI_CONTEXT_PFP_SYNC_ME;
      } else {
         /* GFX10 */
         radeon_end();

         struct si_resource *wait_mem_scratch =
           si_get_wait_mem_scratch_bo(ctx, cs, ctx->ws->cs_is_secure(cs));

         /* CB/DB flush and invalidate via RELEASE_MEM.
          * Combine this with other cache flushes when possible.
          */
         uint64_t va = wait_mem_scratch->gpu_address;
         ctx->wait_mem_number++;

         /* Get GCR_CNTL fields, because the encoding is different in RELEASE_MEM. */
         unsigned glm_wb = G_586_GLM_WB(gcr_cntl);
         unsigned glm_inv = G_586_GLM_INV(gcr_cntl);
         unsigned glv_inv = G_586_GLV_INV(gcr_cntl);
         unsigned gl1_inv = G_586_GL1_INV(gcr_cntl);
         assert(G_586_GL2_US(gcr_cntl) == 0);
         assert(G_586_GL2_RANGE(gcr_cntl) == 0);
         assert(G_586_GL2_DISCARD(gcr_cntl) == 0);
         unsigned gl2_inv = G_586_GL2_INV(gcr_cntl);
         unsigned gl2_wb = G_586_GL2_WB(gcr_cntl);
         unsigned gcr_seq = G_586_SEQ(gcr_cntl);

         gcr_cntl &= C_586_GLM_WB & C_586_GLM_INV & C_586_GLV_INV & C_586_GL1_INV & C_586_GL2_INV &
                     C_586_GL2_WB; /* keep SEQ */

         si_cp_release_mem(ctx, cs, cb_db_event,
                           S_490_GLM_WB(glm_wb) | S_490_GLM_INV(glm_inv) | S_490_GLV_INV(glv_inv) |
                           S_490_GL1_INV(gl1_inv) | S_490_GL2_INV(gl2_inv) | S_490_GL2_WB(gl2_wb) |
                           S_490_SEQ(gcr_seq),
                           EOP_DST_SEL_MEM, EOP_INT_SEL_SEND_DATA_AFTER_WR_CONFIRM,
                           EOP_DATA_SEL_VALUE_32BIT, wait_mem_scratch, va, ctx->wait_mem_number,
                           SI_NOT_QUERY);

         if (unlikely(ctx->sqtt_enabled)) {
            si_sqtt_describe_barrier_start(ctx, &ctx->gfx_cs);
         }

         si_cp_wait_mem(ctx, cs, va, ctx->wait_mem_number, 0xffffffff, WAIT_REG_MEM_EQUAL);

         if (unlikely(ctx->sqtt_enabled)) {
            si_sqtt_describe_barrier_end(ctx, &ctx->gfx_cs, flags);
         }

         radeon_begin_again(cs);
      }
   }

   /* Ignore fields that only modify the behavior of other fields. */
   if (gcr_cntl & C_586_GL1_RANGE & C_586_GL2_RANGE & C_586_SEQ) {
      unsigned dont_sync_pfp = (!(flags & SI_CONTEXT_PFP_SYNC_ME)) << 31;

      /* Flush caches and wait for the caches to assert idle.
       * The cache flush is executed in the ME, but the PFP waits
       * for completion.
       */
      radeon_emit(PKT3(PKT3_ACQUIRE_MEM, 6, 0));
      radeon_emit(dont_sync_pfp); /* CP_COHER_CNTL */
      radeon_emit(0xffffffff); /* CP_COHER_SIZE */
      radeon_emit(0xffffff);   /* CP_COHER_SIZE_HI */
      radeon_emit(0);          /* CP_COHER_BASE */
      radeon_emit(0);          /* CP_COHER_BASE_HI */
      radeon_emit(0x0000000A); /* POLL_INTERVAL */
      radeon_emit(gcr_cntl);   /* GCR_CNTL */
   } else if (flags & SI_CONTEXT_PFP_SYNC_ME) {
      /* Synchronize PFP with ME. (this stalls PFP) */
      radeon_emit(PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      radeon_emit(0);
   }

   if (flags & SI_CONTEXT_START_PIPELINE_STATS && ctx->pipeline_stats_enabled != 1) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_PIPELINESTAT_START) | EVENT_INDEX(0));
      ctx->pipeline_stats_enabled = 1;
   } else if (flags & SI_CONTEXT_STOP_PIPELINE_STATS && ctx->pipeline_stats_enabled != 0) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_PIPELINESTAT_STOP) | EVENT_INDEX(0));
      ctx->pipeline_stats_enabled = 0;
   }
   radeon_end();

   ctx->flags = 0;
}

void gfx6_emit_cache_flush(struct si_context *sctx, struct radeon_cmdbuf *cs)
{
   uint32_t flags = sctx->flags;

   if (!flags)
      return;

   if (!sctx->has_graphics) {
      /* Only process compute flags. */
      flags &= SI_CONTEXT_INV_ICACHE | SI_CONTEXT_INV_SCACHE | SI_CONTEXT_INV_VCACHE |
               SI_CONTEXT_INV_L2 | SI_CONTEXT_WB_L2 | SI_CONTEXT_INV_L2_METADATA |
               SI_CONTEXT_CS_PARTIAL_FLUSH;
   }

   uint32_t cp_coher_cntl = 0;
   const uint32_t flush_cb_db = flags & (SI_CONTEXT_FLUSH_AND_INV_CB | SI_CONTEXT_FLUSH_AND_INV_DB);

   assert(sctx->gfx_level <= GFX9);

   if (flags & SI_CONTEXT_FLUSH_AND_INV_CB)
      sctx->num_cb_cache_flushes++;
   if (flags & SI_CONTEXT_FLUSH_AND_INV_DB)
      sctx->num_db_cache_flushes++;

   /* GFX6 has a bug that it always flushes ICACHE and KCACHE if either
    * bit is set. An alternative way is to write SQC_CACHES, but that
    * doesn't seem to work reliably. Since the bug doesn't affect
    * correctness (it only does more work than necessary) and
    * the performance impact is likely negligible, there is no plan
    * to add a workaround for it.
    */

   if (flags & SI_CONTEXT_INV_ICACHE)
      cp_coher_cntl |= S_0085F0_SH_ICACHE_ACTION_ENA(1);
   if (flags & SI_CONTEXT_INV_SCACHE)
      cp_coher_cntl |= S_0085F0_SH_KCACHE_ACTION_ENA(1);

   if (sctx->gfx_level <= GFX8) {
      if (flags & SI_CONTEXT_FLUSH_AND_INV_CB) {
         cp_coher_cntl |= S_0085F0_CB_ACTION_ENA(1) | S_0085F0_CB0_DEST_BASE_ENA(1) |
                          S_0085F0_CB1_DEST_BASE_ENA(1) | S_0085F0_CB2_DEST_BASE_ENA(1) |
                          S_0085F0_CB3_DEST_BASE_ENA(1) | S_0085F0_CB4_DEST_BASE_ENA(1) |
                          S_0085F0_CB5_DEST_BASE_ENA(1) | S_0085F0_CB6_DEST_BASE_ENA(1) |
                          S_0085F0_CB7_DEST_BASE_ENA(1);

         /* Necessary for DCC */
         if (sctx->gfx_level == GFX8)
            si_cp_release_mem(sctx, cs, V_028A90_FLUSH_AND_INV_CB_DATA_TS, 0, EOP_DST_SEL_MEM,
                              EOP_INT_SEL_NONE, EOP_DATA_SEL_DISCARD, NULL, 0, 0, SI_NOT_QUERY);
      }
      if (flags & SI_CONTEXT_FLUSH_AND_INV_DB)
         cp_coher_cntl |= S_0085F0_DB_ACTION_ENA(1) | S_0085F0_DB_DEST_BASE_ENA(1);
   }

   radeon_begin(cs);

   if (flags & SI_CONTEXT_FLUSH_AND_INV_CB) {
      /* Flush CMASK/FMASK/DCC. SURFACE_SYNC will wait for idle. */
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_FLUSH_AND_INV_CB_META) | EVENT_INDEX(0));
   }
   if (flags & (SI_CONTEXT_FLUSH_AND_INV_DB | SI_CONTEXT_FLUSH_AND_INV_DB_META)) {
      /* Flush HTILE. SURFACE_SYNC will wait for idle. */
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_FLUSH_AND_INV_DB_META) | EVENT_INDEX(0));
   }

   /* Wait for shader engines to go idle.
    * VS and PS waits are unnecessary if SURFACE_SYNC is going to wait
    * for everything including CB/DB cache flushes.
    */
   if (!flush_cb_db) {
      if (flags & SI_CONTEXT_PS_PARTIAL_FLUSH) {
         radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(EVENT_TYPE(V_028A90_PS_PARTIAL_FLUSH) | EVENT_INDEX(4));
         /* Only count explicit shader flushes, not implicit ones
          * done by SURFACE_SYNC.
          */
         sctx->num_vs_flushes++;
         sctx->num_ps_flushes++;
      } else if (flags & SI_CONTEXT_VS_PARTIAL_FLUSH) {
         radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(EVENT_TYPE(V_028A90_VS_PARTIAL_FLUSH) | EVENT_INDEX(4));
         sctx->num_vs_flushes++;
      }
   }

   if (flags & SI_CONTEXT_CS_PARTIAL_FLUSH && sctx->compute_is_busy) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_CS_PARTIAL_FLUSH) | EVENT_INDEX(4));
      sctx->num_cs_flushes++;
      sctx->compute_is_busy = false;
   }

   /* VGT state synchronization. */
   if (flags & SI_CONTEXT_VGT_FLUSH) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_VGT_FLUSH) | EVENT_INDEX(0));
   }
   if (flags & SI_CONTEXT_VGT_STREAMOUT_SYNC) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_VGT_STREAMOUT_SYNC) | EVENT_INDEX(0));
   }

   radeon_end();

   /* GFX9: Wait for idle if we're flushing CB or DB. ACQUIRE_MEM doesn't
    * wait for idle on GFX9. We have to use a TS event.
    */
   if (sctx->gfx_level == GFX9 && flush_cb_db) {
      uint64_t va;
      unsigned tc_flags, cb_db_event;

      /* Set the CB/DB flush event. */
      switch (flush_cb_db) {
      case SI_CONTEXT_FLUSH_AND_INV_CB:
         cb_db_event = V_028A90_FLUSH_AND_INV_CB_DATA_TS;
         break;
      case SI_CONTEXT_FLUSH_AND_INV_DB:
         cb_db_event = V_028A90_FLUSH_AND_INV_DB_DATA_TS;
         break;
      default:
         /* both CB & DB */
         cb_db_event = V_028A90_CACHE_FLUSH_AND_INV_TS_EVENT;
      }

      /* These are the only allowed combinations. If you need to
       * do multiple operations at once, do them separately.
       * All operations that invalidate L2 also seem to invalidate
       * metadata. Volatile (VOL) and WC flushes are not listed here.
       *
       * TC    | TC_WB         = writeback & invalidate L2
       * TC    | TC_WB | TC_NC = writeback & invalidate L2 for MTYPE == NC
       *         TC_WB | TC_NC = writeback L2 for MTYPE == NC
       * TC            | TC_NC = invalidate L2 for MTYPE == NC
       * TC    | TC_MD         = writeback & invalidate L2 metadata (DCC, etc.)
       * TCL1                  = invalidate L1
       */
      tc_flags = 0;

      if (flags & SI_CONTEXT_INV_L2_METADATA) {
         tc_flags = EVENT_TC_ACTION_ENA | EVENT_TC_MD_ACTION_ENA;
      }

      /* Ideally flush TC together with CB/DB. */
      if (flags & SI_CONTEXT_INV_L2) {
         /* Writeback and invalidate everything in L2 & L1. */
         tc_flags = EVENT_TC_ACTION_ENA | EVENT_TC_WB_ACTION_ENA;

         /* Clear the flags. */
         flags &= ~(SI_CONTEXT_INV_L2 | SI_CONTEXT_WB_L2);
         sctx->num_L2_invalidates++;
      }

      /* Do the flush (enqueue the event and wait for it). */
      struct si_resource* wait_mem_scratch =
        si_get_wait_mem_scratch_bo(sctx, cs, sctx->ws->cs_is_secure(cs));

      va = wait_mem_scratch->gpu_address;
      sctx->wait_mem_number++;

      si_cp_release_mem(sctx, cs, cb_db_event, tc_flags, EOP_DST_SEL_MEM,
                        EOP_INT_SEL_SEND_DATA_AFTER_WR_CONFIRM, EOP_DATA_SEL_VALUE_32BIT,
                        wait_mem_scratch, va, sctx->wait_mem_number, SI_NOT_QUERY);

      if (unlikely(sctx->sqtt_enabled)) {
         si_sqtt_describe_barrier_start(sctx, &sctx->gfx_cs);
      }

      si_cp_wait_mem(sctx, cs, va, sctx->wait_mem_number, 0xffffffff, WAIT_REG_MEM_EQUAL);

      if (unlikely(sctx->sqtt_enabled)) {
         si_sqtt_describe_barrier_end(sctx, &sctx->gfx_cs, sctx->flags);
      }
   }

   /* GFX6-GFX8 only:
    *   When one of the CP_COHER_CNTL.DEST_BASE flags is set, SURFACE_SYNC
    *   waits for idle, so it should be last. SURFACE_SYNC is done in PFP.
    *
    * cp_coher_cntl should contain all necessary flags except TC and PFP flags
    * at this point.
    *
    * GFX6-GFX7 don't support L2 write-back.
    */
   if (flags & SI_CONTEXT_INV_L2 || (sctx->gfx_level <= GFX7 && (flags & SI_CONTEXT_WB_L2))) {
      /* Invalidate L1 & L2. (L1 is always invalidated on GFX6)
       * WB must be set on GFX8+ when TC_ACTION is set.
       */
      si_emit_surface_sync(sctx, cs,
                           cp_coher_cntl | S_0085F0_TC_ACTION_ENA(1) | S_0085F0_TCL1_ACTION_ENA(1) |
                              S_0301F0_TC_WB_ACTION_ENA(sctx->gfx_level >= GFX8));
      cp_coher_cntl = 0;
      sctx->num_L2_invalidates++;
   } else {
      /* L1 invalidation and L2 writeback must be done separately,
       * because both operations can't be done together.
       */
      if (flags & SI_CONTEXT_WB_L2) {
         /* WB = write-back
          * NC = apply to non-coherent MTYPEs
          *      (i.e. MTYPE <= 1, which is what we use everywhere)
          *
          * WB doesn't work without NC.
          */
         si_emit_surface_sync(
            sctx, cs,
            cp_coher_cntl | S_0301F0_TC_WB_ACTION_ENA(1) | S_0301F0_TC_NC_ACTION_ENA(1));
         cp_coher_cntl = 0;
         sctx->num_L2_writebacks++;
      }
      if (flags & SI_CONTEXT_INV_VCACHE) {
         /* Invalidate per-CU VMEM L1. */
         si_emit_surface_sync(sctx, cs, cp_coher_cntl | S_0085F0_TCL1_ACTION_ENA(1));
         cp_coher_cntl = 0;
      }
   }

   /* If TC flushes haven't cleared this... */
   if (cp_coher_cntl)
      si_emit_surface_sync(sctx, cs, cp_coher_cntl);

   if (flags & SI_CONTEXT_PFP_SYNC_ME) {
      radeon_begin(cs);
      radeon_emit(PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      radeon_emit(0);
      radeon_end();
   }

   if (flags & SI_CONTEXT_START_PIPELINE_STATS && sctx->pipeline_stats_enabled != 1) {
      radeon_begin(cs);
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_PIPELINESTAT_START) | EVENT_INDEX(0));
      radeon_end();
      sctx->pipeline_stats_enabled = 1;
   } else if (flags & SI_CONTEXT_STOP_PIPELINE_STATS && sctx->pipeline_stats_enabled != 0) {
      radeon_begin(cs);
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_PIPELINESTAT_STOP) | EVENT_INDEX(0));
      radeon_end();
      sctx->pipeline_stats_enabled = 0;
   }

   sctx->flags = 0;
}
