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

#ifndef FREEDRENO_FENCE_H_
#define FREEDRENO_FENCE_H_

#include "pipe/p_context.h"
#include "util/u_queue.h"

#include "common/freedreno_common.h"
#include "drm/freedreno_drmif.h"

BEGINC;

struct pipe_fence_handle {
   struct pipe_reference reference;

   /* When a pre-created unflushed fence has no actual rendering to flush, and
    * the last_fence optimization is used, this will be a reference to the
    * *actualy* fence which needs to be flushed before waiting.
    */
   struct pipe_fence_handle *last_fence;

   /* fence holds a reference to the batch until the batch is flushed, to
    * accommodate PIPE_FLUSH_DEFERRED.  When the batch is actually flushed, it
    * is cleared (before the batch reference is dropped).  If we need to wait
    * on a fence, and the batch is not NULL, we need to flush it.
    *
    * Note that with u_threaded_context async flushes, if a fence is requested
    * by the frontend, the fence is initially created without a reference
    * to the batch, which is filled in later when fd_context_flush() is called
    * from the driver thread.  In this case tc_token will be non-null, in
    * which case threaded_context_flush() should be called in fd_fence_finish()
    */
   struct fd_batch *batch;

   struct tc_unflushed_batch_token *tc_token;
   bool needs_signal;

   /* For threaded_context async flushes, we must wait on the fence, signaled
    * when fence->batch is cleared, to know that the rendering has been actually
    * flushed from the driver thread.
    *
    * The ready fence is created signaled for non-async-flush fences, and only
    * transitions once from unsignalled->signalled for async-flush fences
    */
   struct util_queue_fence ready;

   /* Note that a fence can outlive the ctx, so we can only assume this is a
    * valid ptr for unflushed fences.  However we hold a reference to the
    * fence->pipe so that is safe to use after flushing.
    */
   struct fd_context *ctx;
   struct fd_pipe *pipe;
   struct fd_screen *screen;
   struct fd_fence *fence;

   bool use_fence_fd;
   bool flushed;
   uint32_t syncobj;
};

void fd_pipe_fence_repopulate(struct pipe_fence_handle *fence,
                              struct pipe_fence_handle *last_fence);
void fd_pipe_fence_ref(struct pipe_fence_handle **ptr,
                       struct pipe_fence_handle *pfence);
bool fd_pipe_fence_finish(struct pipe_screen *pscreen, struct pipe_context *ctx,
                          struct pipe_fence_handle *pfence, uint64_t timeout);
void fd_create_pipe_fence_fd(struct pipe_context *pctx,
                             struct pipe_fence_handle **pfence, int fd,
                             enum pipe_fd_type type);
void fd_pipe_fence_server_sync(struct pipe_context *pctx,
                               struct pipe_fence_handle *fence);
void fd_pipe_fence_server_signal(struct pipe_context *ctx,
                                 struct pipe_fence_handle *fence);
int fd_pipe_fence_get_fd(struct pipe_screen *pscreen,
                         struct pipe_fence_handle *pfence);
bool fd_pipe_fence_is_fd(struct pipe_fence_handle *fence);

struct fd_batch;
struct pipe_fence_handle *fd_pipe_fence_create(struct fd_batch *batch);

void fd_pipe_fence_set_batch(struct pipe_fence_handle *fence,
                             struct fd_batch *batch);
void fd_pipe_fence_set_submit_fence(struct pipe_fence_handle *fence,
                        struct fd_fence *submit_fence);

struct tc_unflushed_batch_token;
struct pipe_fence_handle *
fd_pipe_fence_create_unflushed(struct pipe_context *pctx,
                               struct tc_unflushed_batch_token *tc_token);

ENDC;

#endif /* FREEDRENO_FENCE_H_ */
