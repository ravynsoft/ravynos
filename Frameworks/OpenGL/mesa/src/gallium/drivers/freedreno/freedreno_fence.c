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

#include "util/os_file.h"
#include "util/u_inlines.h"

#include "freedreno_batch.h"
#include "freedreno_context.h"
#include "freedreno_fence.h"
#include "freedreno_util.h"
/* TODO: Use the interface drm/freedreno_drmif.h instead of calling directly */
#include <xf86drm.h>

static bool
fence_flush(struct pipe_context *pctx, struct pipe_fence_handle *fence,
            uint64_t timeout)
   /* NOTE: in the !fence_is_signalled() case we may be called from non-driver
    * thread, but we don't call fd_batch_flush() in that case
    */
   in_dt
{
   if (fence->flushed)
      return true;

   MESA_TRACE_FUNC();

   if (!util_queue_fence_is_signalled(&fence->ready)) {
      if (fence->tc_token) {
         threaded_context_flush(pctx, fence->tc_token, timeout == 0);
      }

      if (!timeout)
         return false;

      if (timeout == OS_TIMEOUT_INFINITE) {
         util_queue_fence_wait(&fence->ready);
      } else {
         int64_t abs_timeout = os_time_get_absolute_timeout(timeout);
         if (!util_queue_fence_wait_timeout(&fence->ready, abs_timeout)) {
            return false;
         }
      }

      goto out;
   }

   if (fence->batch)
      fd_batch_flush(fence->batch);

out:
   if (fence->fence)
      fd_fence_flush(fence->fence);

   assert(!fence->batch);
   fence->flushed = true;
   return true;
}

void
fd_pipe_fence_repopulate(struct pipe_fence_handle *fence,
                         struct pipe_fence_handle *last_fence)
{
   if (last_fence->last_fence)
      fd_pipe_fence_repopulate(fence, last_fence->last_fence);

   /* The fence we are re-populating must not be an fd-fence (but last_fince
    * might have been)
    */
   assert(!fence->use_fence_fd);
   assert(!last_fence->batch);

   fd_pipe_fence_ref(&fence->last_fence, last_fence);

   /* We have nothing to flush, so nothing will clear the batch reference
    * (which is normally done when the batch is flushed), so do it now:
    */
   fd_pipe_fence_set_batch(fence, NULL);
}

static void
fd_fence_destroy(struct pipe_fence_handle *fence)
{
   fd_pipe_fence_ref(&fence->last_fence, NULL);

   tc_unflushed_batch_token_reference(&fence->tc_token, NULL);

   if (fence->syncobj)
      drmSyncobjDestroy(fd_device_fd(fence->screen->dev), fence->syncobj);
   fd_pipe_del(fence->pipe);
   if (fence->fence)
      fd_fence_del(fence->fence);

   FREE(fence);
}

void
fd_pipe_fence_ref(struct pipe_fence_handle **ptr,
                  struct pipe_fence_handle *pfence)
{
   if (pipe_reference(&(*ptr)->reference, &pfence->reference))
      fd_fence_destroy(*ptr);

   *ptr = pfence;
}

bool
fd_pipe_fence_finish(struct pipe_screen *pscreen, struct pipe_context *pctx,
                     struct pipe_fence_handle *fence, uint64_t timeout)
{
   MESA_TRACE_SCOPE(timeout ? "fd_pipe_fence_finish(wait)" : "fd_pipe_fence_finish(nowait)");

   /* Note: for TC deferred fence, pctx->flush() may not have been called
    * yet, so always do fence_flush() *first* before delegating to
    * fence->last_fence
    */
   if (!fence_flush(pctx, fence, timeout))
      return false;

   if (fence->last_fence)
      return fd_pipe_fence_finish(pscreen, pctx, fence->last_fence, timeout);

   if (fence->last_fence)
      fence = fence->last_fence;

   if (fence->use_fence_fd) {
      assert(fence->fence);
      int ret = sync_wait(fence->fence->fence_fd, timeout / 1000000);
      return ret == 0;
   }

   if (fd_pipe_wait_timeout(fence->pipe, fence->fence, timeout))
      return false;

   return true;
}

static struct pipe_fence_handle *
fence_create(struct fd_context *ctx, struct fd_batch *batch, int fence_fd,
             int syncobj)
{
   struct pipe_fence_handle *fence;

   fence = CALLOC_STRUCT(pipe_fence_handle);
   if (!fence)
      return NULL;

   pipe_reference_init(&fence->reference, 1);
   util_queue_fence_init(&fence->ready);

   fence->ctx = ctx;
   fd_pipe_fence_set_batch(fence, batch);
   fence->pipe = fd_pipe_ref(ctx->pipe);
   fence->screen = ctx->screen;
   fence->use_fence_fd = (fence_fd != -1);
   fence->syncobj = syncobj;

   if (fence_fd != -1) {
      fence->fence = fd_fence_new(fence->pipe, fence->use_fence_fd);
      fence->fence->fence_fd = fence_fd;
   }

   return fence;
}

void
fd_create_pipe_fence_fd(struct pipe_context *pctx, struct pipe_fence_handle **pfence,
                        int fd, enum pipe_fd_type type)
{
   struct fd_context *ctx = fd_context(pctx);

   switch (type) {
   case PIPE_FD_TYPE_NATIVE_SYNC:
      *pfence =
         fence_create(fd_context(pctx), NULL, os_dupfd_cloexec(fd), 0);
      break;
   case PIPE_FD_TYPE_SYNCOBJ: {
      int ret;
      uint32_t syncobj;

      assert(ctx->screen->has_syncobj);
      ret = drmSyncobjFDToHandle(fd_device_fd(ctx->screen->dev), fd, &syncobj);
      if (!ret)
         close(fd);

      *pfence = fence_create(fd_context(pctx), NULL, -1, syncobj);
      break;
   }
   default:
      unreachable("Unhandled fence type");
   }
}

void
fd_pipe_fence_server_sync(struct pipe_context *pctx, struct pipe_fence_handle *fence)
{
   struct fd_context *ctx = fd_context(pctx);

   MESA_TRACE_FUNC();

   /* NOTE: we don't expect the combination of fence-fd + async-flush-fence,
    * so timeout==0 is ok here:
    */
   fence_flush(pctx, fence, 0);

   if (fence->last_fence) {
      fd_pipe_fence_server_sync(pctx, fence->last_fence);
      return;
   }

   /* if not an external fence, then nothing more to do without preemption: */
   if (!fence->use_fence_fd)
      return;

   ctx->no_implicit_sync = true;

   assert(fence->fence);
   if (sync_accumulate("freedreno", &ctx->in_fence_fd, fence->fence->fence_fd)) {
      /* error */
   }
}

void
fd_pipe_fence_server_signal(struct pipe_context *pctx,
                            struct pipe_fence_handle *fence)
{
   struct fd_context *ctx = fd_context(pctx);

   if (fence->syncobj) {
      drmSyncobjSignal(fd_device_fd(ctx->screen->dev), &fence->syncobj, 1);
   }
}

int
fd_pipe_fence_get_fd(struct pipe_screen *pscreen, struct pipe_fence_handle *fence)
{
   MESA_TRACE_FUNC();

   /* We don't expect deferred flush to be combined with fence-fd: */
   assert(!fence->last_fence);

   assert(fence->use_fence_fd);

   /* NOTE: in the deferred fence case, the pctx we want is the threaded-ctx
    * but if TC is not used, this will be null.  Which is fine, we won't call
    * threaded_context_flush() in that case
    */
   fence_flush(&fence->ctx->tc->base, fence, OS_TIMEOUT_INFINITE);
   assert(fence->fence);
   return os_dupfd_cloexec(fence->fence->fence_fd);
}

bool
fd_pipe_fence_is_fd(struct pipe_fence_handle *fence)
{
   return fence->use_fence_fd;
}

struct pipe_fence_handle *
fd_pipe_fence_create(struct fd_batch *batch)
{
   return fence_create(batch->ctx, batch, -1, 0);
}

void
fd_pipe_fence_set_batch(struct pipe_fence_handle *fence, struct fd_batch *batch)
{
   if (batch) {
      assert(!fence->batch);
      fd_batch_reference(&fence->batch, batch);
      fd_batch_needs_flush(batch);
   } else {
      fd_batch_reference(&fence->batch, NULL);

      /* When the batch is dis-associated with the fence, we can signal TC
       * that the fence is flushed
       */
      if (fence->needs_signal) {
         util_queue_fence_signal(&fence->ready);
         fence->needs_signal = false;
      }
   }
}

void
fd_pipe_fence_set_submit_fence(struct pipe_fence_handle *fence,
                               struct fd_fence *submit_fence)
{
   /* Take ownership of the drm fence after batch/submit is flushed: */
   assert(!fence->fence);
   fence->fence = submit_fence;
   fd_pipe_fence_set_batch(fence, NULL);
}

struct pipe_fence_handle *
fd_pipe_fence_create_unflushed(struct pipe_context *pctx,
                               struct tc_unflushed_batch_token *tc_token)
{
   struct pipe_fence_handle *fence =
      fence_create(fd_context(pctx), NULL, -1, 0);
   fence->needs_signal = true;
   util_queue_fence_reset(&fence->ready);
   tc_unflushed_batch_token_reference(&fence->tc_token, tc_token);
   return fence;
}
