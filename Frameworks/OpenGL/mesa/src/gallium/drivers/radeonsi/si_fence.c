/*
 * Copyright 2013-2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_build_pm4.h"
#include "util/os_time.h"
#include "util/u_memory.h"
#include "util/u_queue.h"
#include "util/u_upload_mgr.h"

#include <libsync.h>

struct si_fine_fence {
   struct si_resource *buf;
   unsigned offset;
};

struct si_fence {
   struct pipe_reference reference;
   struct pipe_fence_handle *gfx;
   struct tc_unflushed_batch_token *tc_token;
   struct util_queue_fence ready;

   /* If the context wasn't flushed at fence creation, this is non-NULL. */
   struct {
      struct si_context *ctx;
      unsigned ib_index;
   } gfx_unflushed;

   struct si_fine_fence fine;
};

/**
 * Write an EOP event.
 *
 * \param event        EVENT_TYPE_*
 * \param event_flags  Optional cache flush flags (TC)
 * \param dst_sel      MEM or TC_L2
 * \param int_sel      NONE or SEND_DATA_AFTER_WR_CONFIRM
 * \param data_sel     DISCARD, VALUE_32BIT, TIMESTAMP, or GDS
 * \param buf          Buffer
 * \param va           GPU address
 * \param old_value    Previous fence value (for a bug workaround)
 * \param new_value    Fence value to write for this event.
 */
void si_cp_release_mem(struct si_context *ctx, struct radeon_cmdbuf *cs, unsigned event,
                       unsigned event_flags, unsigned dst_sel, unsigned int_sel, unsigned data_sel,
                       struct si_resource *buf, uint64_t va, uint32_t new_fence,
                       unsigned query_type)
{
   unsigned op = EVENT_TYPE(event) |
                 EVENT_INDEX(event == V_028A90_CS_DONE || event == V_028A90_PS_DONE ? 6 : 5) |
                 event_flags;
   unsigned sel = EOP_DST_SEL(dst_sel) | EOP_INT_SEL(int_sel) | EOP_DATA_SEL(data_sel);
   bool compute_ib = !ctx->has_graphics;

   radeon_begin(cs);

   if (ctx->gfx_level >= GFX9 || (compute_ib && ctx->gfx_level >= GFX7)) {
      /* A ZPASS_DONE or PIXEL_STAT_DUMP_EVENT (of the DB occlusion
       * counters) must immediately precede every timestamp event to
       * prevent a GPU hang on GFX9.
       *
       * Occlusion queries don't need to do it here, because they
       * always do ZPASS_DONE before the timestamp.
       */
      if (ctx->gfx_level == GFX9 && !compute_ib && query_type != PIPE_QUERY_OCCLUSION_COUNTER &&
          query_type != PIPE_QUERY_OCCLUSION_PREDICATE &&
          query_type != PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE) {
         struct si_screen *sscreen = ctx->screen;
         struct si_resource *scratch;

         if (!ctx->ws->cs_is_secure(&ctx->gfx_cs)) {
            scratch = ctx->eop_bug_scratch;
         } else {
            assert(ctx->screen->info.has_tmz_support);
            if (!ctx->eop_bug_scratch_tmz)
               ctx->eop_bug_scratch_tmz =
                  si_aligned_buffer_create(&sscreen->b,
                                           PIPE_RESOURCE_FLAG_ENCRYPTED |
                                           PIPE_RESOURCE_FLAG_UNMAPPABLE |
                                           SI_RESOURCE_FLAG_DRIVER_INTERNAL,
                                           PIPE_USAGE_DEFAULT,
                                           16 * sscreen->info.max_render_backends, 256);

            scratch = ctx->eop_bug_scratch_tmz;
         }

         assert(16 * ctx->screen->info.max_render_backends <= scratch->b.b.width0);
         radeon_emit(PKT3(PKT3_EVENT_WRITE, 2, 0));
         radeon_emit(EVENT_TYPE(V_028A90_ZPASS_DONE) | EVENT_INDEX(1));
         radeon_emit(scratch->gpu_address);
         radeon_emit(scratch->gpu_address >> 32);

         radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, scratch,
                                   RADEON_USAGE_WRITE | RADEON_PRIO_QUERY);
      }

      radeon_emit(PKT3(PKT3_RELEASE_MEM, ctx->gfx_level >= GFX9 ? 6 : 5, 0));
      radeon_emit(op);
      radeon_emit(sel);
      radeon_emit(va);        /* address lo */
      radeon_emit(va >> 32);  /* address hi */
      radeon_emit(new_fence); /* immediate data lo */
      radeon_emit(0);         /* immediate data hi */
      if (ctx->gfx_level >= GFX9)
         radeon_emit(0); /* unused */
   } else {
      if (ctx->gfx_level == GFX7 || ctx->gfx_level == GFX8) {
         struct si_resource *scratch = ctx->eop_bug_scratch;
         uint64_t va = scratch->gpu_address;

         /* Two EOP events are required to make all engines go idle
          * (and optional cache flushes executed) before the timestamp
          * is written.
          */
         radeon_emit(PKT3(PKT3_EVENT_WRITE_EOP, 4, 0));
         radeon_emit(op);
         radeon_emit(va);
         radeon_emit(((va >> 32) & 0xffff) | sel);
         radeon_emit(0); /* immediate data */
         radeon_emit(0); /* unused */

         radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, scratch,
                                   RADEON_USAGE_WRITE | RADEON_PRIO_QUERY);
      }

      radeon_emit(PKT3(PKT3_EVENT_WRITE_EOP, 4, 0));
      radeon_emit(op);
      radeon_emit(va);
      radeon_emit(((va >> 32) & 0xffff) | sel);
      radeon_emit(new_fence); /* immediate data */
      radeon_emit(0);         /* unused */
   }

   radeon_end();

   if (buf) {
      radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, buf, RADEON_USAGE_WRITE | RADEON_PRIO_QUERY);
   }
}

unsigned si_cp_write_fence_dwords(struct si_screen *screen)
{
   unsigned dwords = 6;

   if (screen->info.gfx_level == GFX7 || screen->info.gfx_level == GFX8)
      dwords *= 2;

   return dwords;
}

void si_cp_wait_mem(struct si_context *ctx, struct radeon_cmdbuf *cs, uint64_t va, uint32_t ref,
                    uint32_t mask, unsigned flags)
{
   radeon_begin(cs);
   radeon_emit(PKT3(PKT3_WAIT_REG_MEM, 5, 0));
   radeon_emit(WAIT_REG_MEM_MEM_SPACE(1) | flags);
   radeon_emit(va);
   radeon_emit(va >> 32);
   radeon_emit(ref);  /* reference value */
   radeon_emit(mask); /* mask */
   radeon_emit(4);    /* poll interval */
   radeon_end();
}

static void si_add_fence_dependency(struct si_context *sctx, struct pipe_fence_handle *fence)
{
   struct radeon_winsys *ws = sctx->ws;

   ws->cs_add_fence_dependency(&sctx->gfx_cs, fence);
}

static void si_add_syncobj_signal(struct si_context *sctx, struct pipe_fence_handle *fence)
{
   sctx->ws->cs_add_syncobj_signal(&sctx->gfx_cs, fence);
}

static void si_fence_reference(struct pipe_screen *screen, struct pipe_fence_handle **dst,
                               struct pipe_fence_handle *src)
{
   struct radeon_winsys *ws = ((struct si_screen *)screen)->ws;
   struct si_fence **sdst = (struct si_fence **)dst;
   struct si_fence *ssrc = (struct si_fence *)src;

   if (pipe_reference(&(*sdst)->reference, &ssrc->reference)) {
      ws->fence_reference(ws, &(*sdst)->gfx, NULL);
      tc_unflushed_batch_token_reference(&(*sdst)->tc_token, NULL);
      si_resource_reference(&(*sdst)->fine.buf, NULL);
      FREE(*sdst);
   }
   *sdst = ssrc;
}

static struct si_fence *si_alloc_fence()
{
   struct si_fence *fence = CALLOC_STRUCT(si_fence);
   if (!fence)
      return NULL;

   pipe_reference_init(&fence->reference, 1);
   util_queue_fence_init(&fence->ready);

   return fence;
}

struct pipe_fence_handle *si_create_fence(struct pipe_context *ctx,
                                          struct tc_unflushed_batch_token *tc_token)
{
   struct si_fence *fence = si_alloc_fence();
   if (!fence)
      return NULL;

   util_queue_fence_reset(&fence->ready);
   tc_unflushed_batch_token_reference(&fence->tc_token, tc_token);

   return (struct pipe_fence_handle *)fence;
}

static bool si_fine_fence_signaled(struct radeon_winsys *rws, const struct si_fine_fence *fine)
{
   char *map =
      rws->buffer_map(rws, fine->buf->buf, NULL, PIPE_MAP_READ | PIPE_MAP_UNSYNCHRONIZED);
   if (!map)
      return false;

   uint32_t *fence = (uint32_t *)(map + fine->offset);
   return *fence != 0;
}

static void si_fine_fence_set(struct si_context *ctx, struct si_fine_fence *fine, unsigned flags)
{
   uint32_t *fence_ptr;

   assert(util_bitcount(flags & (PIPE_FLUSH_TOP_OF_PIPE | PIPE_FLUSH_BOTTOM_OF_PIPE)) == 1);

   /* Use cached system memory for the fence. */
   u_upload_alloc(ctx->cached_gtt_allocator, 0, 4, 4, &fine->offset,
                  (struct pipe_resource **)&fine->buf, (void **)&fence_ptr);
   if (!fine->buf)
      return;

   *fence_ptr = 0;

   if (flags & PIPE_FLUSH_TOP_OF_PIPE) {
      uint32_t value = 0x80000000;

      si_cp_write_data(ctx, fine->buf, fine->offset, 4, V_370_MEM, V_370_PFP, &value);
   } else if (flags & PIPE_FLUSH_BOTTOM_OF_PIPE) {
      uint64_t fence_va = fine->buf->gpu_address + fine->offset;

      radeon_add_to_buffer_list(ctx, &ctx->gfx_cs, fine->buf, RADEON_USAGE_WRITE | RADEON_PRIO_QUERY);
      si_cp_release_mem(ctx, &ctx->gfx_cs, V_028A90_BOTTOM_OF_PIPE_TS, 0, EOP_DST_SEL_MEM,
                        EOP_INT_SEL_NONE, EOP_DATA_SEL_VALUE_32BIT, NULL, fence_va, 0x80000000,
                        PIPE_QUERY_GPU_FINISHED);
   } else {
      assert(false);
   }
}

static bool si_fence_finish(struct pipe_screen *screen, struct pipe_context *ctx,
                            struct pipe_fence_handle *fence, uint64_t timeout)
{
   struct radeon_winsys *rws = ((struct si_screen *)screen)->ws;
   struct si_fence *sfence = (struct si_fence *)fence;
   struct si_context *sctx;
   int64_t abs_timeout = os_time_get_absolute_timeout(timeout);

   ctx = threaded_context_unwrap_sync(ctx);
   sctx = (struct si_context *)(ctx ? ctx : NULL);

   if (!util_queue_fence_is_signalled(&sfence->ready)) {
      if (sfence->tc_token) {
         /* Ensure that si_flush_from_st will be called for
          * this fence, but only if we're in the API thread
          * where the context is current.
          *
          * Note that the batch containing the flush may already
          * be in flight in the driver thread, so the fence
          * may not be ready yet when this call returns.
          */
         threaded_context_flush(ctx, sfence->tc_token, timeout == 0);
      }

      if (!timeout)
         return false;

      if (timeout == OS_TIMEOUT_INFINITE) {
         util_queue_fence_wait(&sfence->ready);
      } else {
         if (!util_queue_fence_wait_timeout(&sfence->ready, abs_timeout))
            return false;
      }

      if (timeout && timeout != OS_TIMEOUT_INFINITE) {
         int64_t time = os_time_get_nano();
         timeout = abs_timeout > time ? abs_timeout - time : 0;
      }
   }

   if (!sfence->gfx)
      return true;

   if (sfence->fine.buf && si_fine_fence_signaled(rws, &sfence->fine)) {
      rws->fence_reference(rws, &sfence->gfx, NULL);
      si_resource_reference(&sfence->fine.buf, NULL);
      return true;
   }

   /* Flush the gfx IB if it hasn't been flushed yet. */
   if (sctx && sfence->gfx_unflushed.ctx == sctx &&
       sfence->gfx_unflushed.ib_index == sctx->num_gfx_cs_flushes) {
      /* Section 4.1.2 (Signaling) of the OpenGL 4.6 (Core profile)
       * spec says:
       *
       *    "If the sync object being blocked upon will not be
       *     signaled in finite time (for example, by an associated
       *     fence command issued previously, but not yet flushed to
       *     the graphics pipeline), then ClientWaitSync may hang
       *     forever. To help prevent this behavior, if
       *     ClientWaitSync is called and all of the following are
       *     true:
       *
       *     * the SYNC_FLUSH_COMMANDS_BIT bit is set in flags,
       *     * sync is unsignaled when ClientWaitSync is called,
       *     * and the calls to ClientWaitSync and FenceSync were
       *       issued from the same context,
       *
       *     then the GL will behave as if the equivalent of Flush
       *     were inserted immediately after the creation of sync."
       *
       * This means we need to flush for such fences even when we're
       * not going to wait.
       */
      si_flush_gfx_cs(sctx, (timeout ? 0 : PIPE_FLUSH_ASYNC) | RADEON_FLUSH_START_NEXT_GFX_IB_NOW,
                      NULL);
      sfence->gfx_unflushed.ctx = NULL;

      if (!timeout)
         return false;

      /* Recompute the timeout after all that. */
      if (timeout && timeout != OS_TIMEOUT_INFINITE) {
         int64_t time = os_time_get_nano();
         timeout = abs_timeout > time ? abs_timeout - time : 0;
      }
   }

   if (rws->fence_wait(rws, sfence->gfx, timeout))
      return true;

   /* Re-check in case the GPU is slow or hangs, but the commands before
    * the fine-grained fence have completed. */
   if (sfence->fine.buf && si_fine_fence_signaled(rws, &sfence->fine))
      return true;

   return false;
}

static void si_create_fence_fd(struct pipe_context *ctx, struct pipe_fence_handle **pfence, int fd,
                               enum pipe_fd_type type)
{
   struct si_screen *sscreen = (struct si_screen *)ctx->screen;
   struct radeon_winsys *ws = sscreen->ws;
   struct si_fence *sfence;

   *pfence = NULL;

   sfence = si_alloc_fence();
   if (!sfence)
      return;

   switch (type) {
   case PIPE_FD_TYPE_NATIVE_SYNC:
      if (!sscreen->info.has_fence_to_handle)
         goto finish;

      sfence->gfx = ws->fence_import_sync_file(ws, fd);
      break;

   case PIPE_FD_TYPE_SYNCOBJ:
      if (!sscreen->info.has_syncobj)
         goto finish;

      sfence->gfx = ws->fence_import_syncobj(ws, fd);
      break;

   default:
      unreachable("bad fence fd type when importing");
   }

finish:
   if (!sfence->gfx) {
      FREE(sfence);
      return;
   }

   *pfence = (struct pipe_fence_handle *)sfence;
}

static int si_fence_get_fd(struct pipe_screen *screen, struct pipe_fence_handle *fence)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct radeon_winsys *ws = sscreen->ws;
   struct si_fence *sfence = (struct si_fence *)fence;
   int gfx_fd = -1;

   if (!sscreen->info.has_fence_to_handle)
      return -1;

   util_queue_fence_wait(&sfence->ready);

   /* Deferred fences aren't supported. */
   assert(!sfence->gfx_unflushed.ctx);
   if (sfence->gfx_unflushed.ctx)
      return -1;

   if (sfence->gfx) {
      gfx_fd = ws->fence_export_sync_file(ws, sfence->gfx);
      if (gfx_fd == -1) {
         return -1;
      }
   }

   /* If we don't have FDs at this point, it means we don't have fences
    * either. */
   if (gfx_fd == -1)
      return ws->export_signalled_sync_file(ws);

   return gfx_fd;
}

static void si_flush_all_queues(struct pipe_context *ctx,
                                struct pipe_fence_handle **fence,
                                unsigned flags, bool force_flush)
{
   struct pipe_screen *screen = ctx->screen;
   struct si_context *sctx = (struct si_context *)ctx;
   struct radeon_winsys *ws = sctx->ws;
   struct pipe_fence_handle *gfx_fence = NULL;
   bool deferred_fence = false;
   struct si_fine_fence fine = {};
   unsigned rflags = PIPE_FLUSH_ASYNC;

   if (!(flags & PIPE_FLUSH_DEFERRED)) {
      si_flush_implicit_resources(sctx);
   }

   if (flags & PIPE_FLUSH_END_OF_FRAME)
      rflags |= PIPE_FLUSH_END_OF_FRAME;

   if (flags & (PIPE_FLUSH_TOP_OF_PIPE | PIPE_FLUSH_BOTTOM_OF_PIPE)) {
      assert(flags & PIPE_FLUSH_DEFERRED);
      assert(fence);

      si_fine_fence_set(sctx, &fine, flags);
   }

   if (force_flush) {
      sctx->initial_gfx_cs_size = 0;
   }

   if (!radeon_emitted(&sctx->gfx_cs, sctx->initial_gfx_cs_size)) {
      if (fence)
         ws->fence_reference(ws, &gfx_fence, sctx->last_gfx_fence);
      if (!(flags & PIPE_FLUSH_DEFERRED))
         ws->cs_sync_flush(&sctx->gfx_cs);

      tc_driver_internal_flush_notify(sctx->tc);

      if (unlikely(sctx->sqtt && (flags & PIPE_FLUSH_END_OF_FRAME))) {
         si_handle_sqtt(sctx, &sctx->gfx_cs);
      }
      
      if (u_trace_perfetto_active(&sctx->ds.trace_context)) {
         u_trace_context_process(&sctx->ds.trace_context, flags & PIPE_FLUSH_END_OF_FRAME);
      }
   } else {
      /* Instead of flushing, create a deferred fence. Constraints:
       * - the gallium frontend must allow a deferred flush.
       * - the gallium frontend must request a fence.
       * - fence_get_fd is not allowed.
       * Thread safety in fence_finish must be ensured by the gallium frontend.
       */
      if (flags & PIPE_FLUSH_DEFERRED && !(flags & PIPE_FLUSH_FENCE_FD) && fence) {
         gfx_fence = sctx->ws->cs_get_next_fence(&sctx->gfx_cs);
         deferred_fence = true;
      } else {
         si_flush_gfx_cs(sctx, rflags, fence ? &gfx_fence : NULL);
      }
   }

   /* Both engines can signal out of order, so we need to keep both fences. */
   if (fence) {
      struct si_fence *new_fence;

      if (flags & TC_FLUSH_ASYNC) {
         new_fence = (struct si_fence *)*fence;
         assert(new_fence);
      } else {
         new_fence = si_alloc_fence();
         if (!new_fence) {
            ws->fence_reference(ws, &gfx_fence, NULL);
            goto finish;
         }

         screen->fence_reference(screen, fence, NULL);
         *fence = (struct pipe_fence_handle *)new_fence;
      }

      /* If both fences are NULL, fence_finish will always return true. */
      new_fence->gfx = gfx_fence;

      if (deferred_fence) {
         new_fence->gfx_unflushed.ctx = sctx;
         new_fence->gfx_unflushed.ib_index = sctx->num_gfx_cs_flushes;
      }

      new_fence->fine = fine;
      fine.buf = NULL;

      if (flags & TC_FLUSH_ASYNC) {
         util_queue_fence_signal(&new_fence->ready);
         tc_unflushed_batch_token_reference(&new_fence->tc_token, NULL);
      }
   }
   assert(!fine.buf);
finish:
   if (!(flags & (PIPE_FLUSH_DEFERRED | PIPE_FLUSH_ASYNC))) {
      ws->cs_sync_flush(&sctx->gfx_cs);
   }
}

static void si_flush_from_st(struct pipe_context *ctx, struct pipe_fence_handle **fence,
                             unsigned flags)
{
   return si_flush_all_queues(ctx, fence, flags, false);
}

static void si_fence_server_signal(struct pipe_context *ctx, struct pipe_fence_handle *fence)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_fence *sfence = (struct si_fence *)fence;

   assert(sfence->gfx);

   if (sfence->gfx)
      si_add_syncobj_signal(sctx, sfence->gfx);

   /**
    * The spec requires a flush here. We insert a flush
    * because syncobj based signals are not directly placed into
    * the command stream. Instead the signal happens when the
    * submission associated with the syncobj finishes execution.
    *
    * Therefore, we must make sure that we flush the pipe to avoid
    * new work being emitted and getting executed before the signal
    * operation.
    *
    * Forces a flush even if the GFX CS is empty.
    *
    * The flush must not be asynchronous because the kernel must receive
    * the scheduled "signal" operation before any wait.
    */
   si_flush_all_queues(ctx, NULL, 0, true);
}

static void si_fence_server_sync(struct pipe_context *ctx, struct pipe_fence_handle *fence)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_fence *sfence = (struct si_fence *)fence;

   util_queue_fence_wait(&sfence->ready);

   /* Unflushed fences from the same context are no-ops. */
   if (sfence->gfx_unflushed.ctx && sfence->gfx_unflushed.ctx == sctx)
      return;

   /* All unflushed commands will not start execution before this fence
    * dependency is signalled. That's fine. Flushing is very expensive
    * if we get fence_server_sync after every draw call. (which happens
    * with Android/SurfaceFlinger)
    *
    * In a nutshell, when CPU overhead is greater than GPU overhead,
    * or when the time it takes to execute an IB on the GPU is less than
    * the time it takes to create and submit that IB, flushing decreases
    * performance. Therefore, DO NOT FLUSH.
    */
   if (sfence->gfx)
      si_add_fence_dependency(sctx, sfence->gfx);
}

void si_init_fence_functions(struct si_context *ctx)
{
   ctx->b.flush = si_flush_from_st;
   ctx->b.create_fence_fd = si_create_fence_fd;
   ctx->b.fence_server_sync = si_fence_server_sync;
   ctx->b.fence_server_signal = si_fence_server_signal;
}

void si_init_screen_fence_functions(struct si_screen *screen)
{
   screen->b.fence_finish = si_fence_finish;
   screen->b.fence_reference = si_fence_reference;
   screen->b.fence_get_fd = si_fence_get_fd;
}
