/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iris_fence.c
 *
 * Fences for driver and IPC serialisation, scheduling and synchronisation.
 */

#include "drm-uapi/sync_file.h"
#include "util/u_debug.h"
#include "util/u_inlines.h"
#include "intel/common/intel_gem.h"

#include "iris_batch.h"
#include "iris_bufmgr.h"
#include "iris_context.h"
#include "iris_fence.h"
#include "iris_screen.h"

static uint32_t
gem_syncobj_create(int fd, uint32_t flags)
{
   struct drm_syncobj_create args = {
      .flags = flags,
   };

   intel_ioctl(fd, DRM_IOCTL_SYNCOBJ_CREATE, &args);

   return args.handle;
}

static void
gem_syncobj_destroy(int fd, uint32_t handle)
{
   struct drm_syncobj_destroy args = {
      .handle = handle,
   };

   intel_ioctl(fd, DRM_IOCTL_SYNCOBJ_DESTROY, &args);
}

/**
 * Make a new sync-point.
 */
struct iris_syncobj *
iris_create_syncobj(struct iris_bufmgr *bufmgr)
{
   int fd = iris_bufmgr_get_fd(bufmgr);
   struct iris_syncobj *syncobj = malloc(sizeof(*syncobj));

   if (!syncobj)
      return NULL;

   syncobj->handle = gem_syncobj_create(fd, 0);
   assert(syncobj->handle);

   pipe_reference_init(&syncobj->ref, 1);

   return syncobj;
}

void
iris_syncobj_destroy(struct iris_bufmgr *bufmgr, struct iris_syncobj *syncobj)
{
   int fd = iris_bufmgr_get_fd(bufmgr);
   gem_syncobj_destroy(fd, syncobj->handle);
   free(syncobj);
}

void
iris_syncobj_signal(struct iris_bufmgr *bufmgr, struct iris_syncobj *syncobj)
{
   int fd = iris_bufmgr_get_fd(bufmgr);
   struct drm_syncobj_array args = {
      .handles = (uintptr_t)&syncobj->handle,
      .count_handles = 1,
   };

   if (intel_ioctl(fd, DRM_IOCTL_SYNCOBJ_SIGNAL, &args)) {
      fprintf(stderr, "failed to signal syncobj %"PRIu32"\n",
              syncobj->handle);
   }
}

/**
 * Add a sync-point to the batch, with the given flags.
 *
 * \p flags   One of IRIS_BATCH_FENCE_WAIT or IRIS_BATCH_FENCE_SIGNAL.
 */
void
iris_batch_add_syncobj(struct iris_batch *batch,
                       struct iris_syncobj *syncobj,
                       uint32_t flags)
{
   struct iris_batch_fence *fence =
      util_dynarray_grow(&batch->exec_fences, struct iris_batch_fence, 1);

   *fence = (struct iris_batch_fence) {
      .handle = syncobj->handle,
      .flags = flags,
   };

   struct iris_syncobj **store =
      util_dynarray_grow(&batch->syncobjs, struct iris_syncobj *, 1);

   *store = NULL;
   iris_syncobj_reference(batch->screen->bufmgr, store, syncobj);
}

/**
 * Walk through a batch's dependencies (any IRIS_BATCH_FENCE_WAIT syncobjs)
 * and unreference any which have already passed.
 *
 * Sometimes the compute batch is seldom used, and accumulates references
 * to stale render batches that are no longer of interest, so we can free
 * those up.
 */
static void
clear_stale_syncobjs(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;

   int n = util_dynarray_num_elements(&batch->syncobjs, struct iris_syncobj *);

   assert(n == util_dynarray_num_elements(&batch->exec_fences,
                                          struct iris_batch_fence));

   /* Skip the first syncobj, as it's the signalling one. */
   for (int i = n - 1; i > 0; i--) {
      struct iris_syncobj **syncobj =
         util_dynarray_element(&batch->syncobjs, struct iris_syncobj *, i);
      struct iris_batch_fence *fence =
         util_dynarray_element(&batch->exec_fences,
                               struct iris_batch_fence, i);
      assert(fence->flags & IRIS_BATCH_FENCE_WAIT);

      if (iris_wait_syncobj(bufmgr, *syncobj, 0) == false)
         continue;

      /* This sync object has already passed, there's no need to continue
       * marking it as a dependency; we can stop holding on to the reference.
       */
      iris_syncobj_reference(bufmgr, syncobj, NULL);

      /* Remove it from the lists; move the last element here. */
      struct iris_syncobj **nth_syncobj =
         util_dynarray_pop_ptr(&batch->syncobjs, struct iris_syncobj *);
      struct iris_batch_fence *nth_fence =
         util_dynarray_pop_ptr(&batch->exec_fences, struct iris_batch_fence);

      if (syncobj != nth_syncobj) {
         *syncobj = *nth_syncobj;
         memcpy(fence, nth_fence, sizeof(*fence));
      }
   }
}

/* ------------------------------------------------------------------- */

struct pipe_fence_handle {
   struct pipe_reference ref;

   struct pipe_context *unflushed_ctx;

   struct iris_fine_fence *fine[IRIS_BATCH_COUNT];
};

static void
iris_fence_destroy(struct pipe_screen *p_screen,
                   struct pipe_fence_handle *fence)
{
   struct iris_screen *screen = (struct iris_screen *)p_screen;

   for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++)
      iris_fine_fence_reference(screen, &fence->fine[i], NULL);

   free(fence);
}

static void
iris_fence_reference(struct pipe_screen *p_screen,
                     struct pipe_fence_handle **dst,
                     struct pipe_fence_handle *src)
{
   if (pipe_reference(*dst ? &(*dst)->ref : NULL,
                      src ? &src->ref : NULL))
      iris_fence_destroy(p_screen, *dst);

   *dst = src;
}

bool
iris_wait_syncobj(struct iris_bufmgr *bufmgr,
                  struct iris_syncobj *syncobj,
                  int64_t timeout_nsec)
{
   if (!syncobj)
      return false;

   int fd = iris_bufmgr_get_fd(bufmgr);

   struct drm_syncobj_wait args = {
      .handles = (uintptr_t)&syncobj->handle,
      .count_handles = 1,
      .timeout_nsec = timeout_nsec,
   };
   return intel_ioctl(fd, DRM_IOCTL_SYNCOBJ_WAIT, &args) == 0;
}

#define CSI "\e["
#define BLUE_HEADER  CSI "0;97;44m"
#define NORMAL       CSI "0m"

static void
iris_fence_flush(struct pipe_context *ctx,
                 struct pipe_fence_handle **out_fence,
                 unsigned flags)
{
   struct iris_screen *screen = (void *) ctx->screen;
   struct iris_context *ice = (struct iris_context *)ctx;

   /* We require DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT (kernel 5.2+) for
    * deferred flushes.  Just ignore the request to defer on older kernels.
    */
   if (!(screen->kernel_features & KERNEL_HAS_WAIT_FOR_SUBMIT))
      flags &= ~PIPE_FLUSH_DEFERRED;

   const bool deferred = flags & PIPE_FLUSH_DEFERRED;

   if (flags & PIPE_FLUSH_END_OF_FRAME) {
      ice->frame++;

      if (INTEL_DEBUG(DEBUG_SUBMIT)) {
         fprintf(stderr, "%s ::: FRAME %-10u (ctx %p)%-35c%s\n",
                 INTEL_DEBUG(DEBUG_COLOR) ? BLUE_HEADER : "",
                 ice->frame, ctx, ' ',
                 INTEL_DEBUG(DEBUG_COLOR) ? NORMAL : "");
      }
   }

   iris_flush_dirty_dmabufs(ice);

   if (!deferred) {
      iris_foreach_batch(ice, batch)
         iris_batch_flush(batch);
   }

   if (flags & PIPE_FLUSH_END_OF_FRAME) {
      iris_measure_frame_end(ice);
   }

   intel_ds_device_process(&ice->ds, flags & PIPE_FLUSH_END_OF_FRAME);

   if (!out_fence)
      return;

   struct pipe_fence_handle *fence = calloc(1, sizeof(*fence));
   if (!fence)
      return;

   pipe_reference_init(&fence->ref, 1);

   if (deferred)
      fence->unflushed_ctx = ctx;

   iris_foreach_batch(ice, batch) {
      unsigned b = batch->name;

      if (deferred && iris_batch_bytes_used(batch) > 0) {
         struct iris_fine_fence *fine = iris_fine_fence_new(batch);
         iris_fine_fence_reference(screen, &fence->fine[b], fine);
         iris_fine_fence_reference(screen, &fine, NULL);
      } else {
         /* This batch has no commands queued up (perhaps we just flushed,
          * or all the commands are on the other batch).  Wait for the last
          * syncobj on this engine - unless it's already finished by now.
          */
         if (iris_fine_fence_signaled(batch->last_fence))
            continue;

         iris_fine_fence_reference(screen, &fence->fine[b], batch->last_fence);
      }
   }

   iris_fence_reference(ctx->screen, out_fence, NULL);
   *out_fence = fence;
}

static void
iris_fence_await(struct pipe_context *ctx,
                 struct pipe_fence_handle *fence)
{
   struct iris_context *ice = (struct iris_context *)ctx;

   /* Unflushed fences from the same context are no-ops. */
   if (ctx && ctx == fence->unflushed_ctx)
      return;

   /* XXX: We can't safely flush the other context, because it might be
    *      bound to another thread, and poking at its internals wouldn't
    *      be safe.  In the future we should use MI_SEMAPHORE_WAIT and
    *      block until the other job has been submitted, relying on
    *      kernel timeslicing to preempt us until the other job is
    *      actually flushed and the seqno finally passes.
    */
   if (fence->unflushed_ctx) {
      util_debug_message(&ice->dbg, CONFORMANCE, "%s",
                         "glWaitSync on unflushed fence from another context "
                         "is unlikely to work without kernel 5.8+\n");
   }

   for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++) {
      struct iris_fine_fence *fine = fence->fine[i];

      if (iris_fine_fence_signaled(fine))
         continue;

      iris_foreach_batch(ice, batch) {
         /* We're going to make any future work in this batch wait for our
          * fence to have gone by.  But any currently queued work doesn't
          * need to wait.  Flush the batch now, so it can happen sooner.
          */
         iris_batch_flush(batch);

         /* Before adding a new reference, clean out any stale ones. */
         clear_stale_syncobjs(batch);

         iris_batch_add_syncobj(batch, fine->syncobj, IRIS_BATCH_FENCE_WAIT);
      }
   }
}

#define NSEC_PER_SEC (1000 * USEC_PER_SEC)
#define USEC_PER_SEC (1000 * MSEC_PER_SEC)
#define MSEC_PER_SEC (1000)

static uint64_t
gettime_ns(void)
{
   struct timespec current;
   clock_gettime(CLOCK_MONOTONIC, &current);
   return (uint64_t)current.tv_sec * NSEC_PER_SEC + current.tv_nsec;
}

static uint64_t
rel2abs(uint64_t timeout)
{
   if (timeout == 0)
      return 0;

   uint64_t current_time = gettime_ns();
   uint64_t max_timeout = (uint64_t) INT64_MAX - current_time;

   timeout = MIN2(max_timeout, timeout);

   return current_time + timeout;
}

static bool
iris_fence_finish(struct pipe_screen *p_screen,
                  struct pipe_context *ctx,
                  struct pipe_fence_handle *fence,
                  uint64_t timeout)
{
   ctx = threaded_context_unwrap_sync(ctx);

   struct iris_context *ice = (struct iris_context *)ctx;
   struct iris_screen *screen = (struct iris_screen *)p_screen;

   /* If we created the fence with PIPE_FLUSH_DEFERRED, we may not have
    * flushed yet.  Check if our syncobj is the current batch's signalling
    * syncobj - if so, we haven't flushed and need to now.
    *
    * The Gallium docs mention that a flush will occur if \p ctx matches
    * the context the fence was created with.  It may be NULL, so we check
    * that it matches first.
    */
   if (ctx && ctx == fence->unflushed_ctx) {
      iris_foreach_batch(ice, batch) {
         struct iris_fine_fence *fine = fence->fine[batch->name];

         if (iris_fine_fence_signaled(fine))
            continue;

         if (fine->syncobj == iris_batch_get_signal_syncobj(batch))
            iris_batch_flush(batch);
      }

      /* The fence is no longer deferred. */
      fence->unflushed_ctx = NULL;
   }

   unsigned int handle_count = 0;
   uint32_t handles[ARRAY_SIZE(fence->fine)];
   for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++) {
      struct iris_fine_fence *fine = fence->fine[i];

      if (iris_fine_fence_signaled(fine))
         continue;

      handles[handle_count++] = fine->syncobj->handle;
   }

   if (handle_count == 0)
      return true;

   struct drm_syncobj_wait args = {
      .handles = (uintptr_t)handles,
      .count_handles = handle_count,
      .timeout_nsec = rel2abs(timeout),
      .flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL
   };

   if (fence->unflushed_ctx) {
      /* This fence had a deferred flush from another context.  We can't
       * safely flush it here, because the context might be bound to a
       * different thread, and poking at its internals wouldn't be safe.
       *
       * Instead, use the WAIT_FOR_SUBMIT flag to block and hope that
       * another thread submits the work.
       */
      args.flags |= DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT;
   }

   return intel_ioctl(screen->fd, DRM_IOCTL_SYNCOBJ_WAIT, &args) == 0;
}

static int
sync_merge_fd(int sync_fd, int new_fd)
{
   if (sync_fd == -1)
      return new_fd;

   if (new_fd == -1)
      return sync_fd;

   struct sync_merge_data args = {
      .name = "iris fence",
      .fd2 = new_fd,
      .fence = -1,
   };

   intel_ioctl(sync_fd, SYNC_IOC_MERGE, &args);
   close(new_fd);
   close(sync_fd);

   return args.fence;
}

static int
iris_fence_get_fd(struct pipe_screen *p_screen,
                  struct pipe_fence_handle *fence)
{
   struct iris_screen *screen = (struct iris_screen *)p_screen;
   int fd = -1;

   /* Deferred fences aren't supported. */
   if (fence->unflushed_ctx)
      return -1;

   for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++) {
      struct iris_fine_fence *fine = fence->fine[i];

      if (iris_fine_fence_signaled(fine))
         continue;

      struct drm_syncobj_handle args = {
         .handle = fine->syncobj->handle,
         .flags = DRM_SYNCOBJ_HANDLE_TO_FD_FLAGS_EXPORT_SYNC_FILE,
         .fd = -1,
      };

      intel_ioctl(screen->fd, DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD, &args);
      fd = sync_merge_fd(fd, args.fd);
   }

   if (fd == -1) {
      /* Our fence has no syncobj's recorded.  This means that all of the
       * batches had already completed, their syncobj's had been signalled,
       * and so we didn't bother to record them.  But we're being asked to
       * export such a fence.  So export a dummy already-signalled syncobj.
       */
      struct drm_syncobj_handle args = {
         .flags = DRM_SYNCOBJ_HANDLE_TO_FD_FLAGS_EXPORT_SYNC_FILE, .fd = -1,
      };

      args.handle = gem_syncobj_create(screen->fd, DRM_SYNCOBJ_CREATE_SIGNALED);
      intel_ioctl(screen->fd, DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD, &args);
      gem_syncobj_destroy(screen->fd, args.handle);
      return args.fd;
   }

   return fd;
}

static void
iris_fence_create_fd(struct pipe_context *ctx,
                     struct pipe_fence_handle **out,
                     int fd,
                     enum pipe_fd_type type)
{
   assert(type == PIPE_FD_TYPE_NATIVE_SYNC || type == PIPE_FD_TYPE_SYNCOBJ);

   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   struct drm_syncobj_handle args = {
      .fd = fd,
   };

   if (type == PIPE_FD_TYPE_NATIVE_SYNC) {
      args.flags = DRM_SYNCOBJ_FD_TO_HANDLE_FLAGS_IMPORT_SYNC_FILE;
      args.handle = gem_syncobj_create(screen->fd, DRM_SYNCOBJ_CREATE_SIGNALED);
   }

   if (intel_ioctl(screen->fd, DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE, &args) == -1) {
      fprintf(stderr, "DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE failed: %s\n",
              strerror(errno));
      if (type == PIPE_FD_TYPE_NATIVE_SYNC)
         gem_syncobj_destroy(screen->fd, args.handle);
      *out = NULL;
      return;
   }

   struct iris_syncobj *syncobj = malloc(sizeof(*syncobj));
   if (!syncobj) {
      *out = NULL;
      return;
   }
   syncobj->handle = args.handle;
   pipe_reference_init(&syncobj->ref, 1);

   struct iris_fine_fence *fine = calloc(1, sizeof(*fine));
   if (!fine) {
      free(syncobj);
      *out = NULL;
      return;
   }

   static const uint32_t zero = 0;

   /* Fences work in terms of iris_fine_fence, but we don't actually have a
    * seqno for an imported fence.  So, create a fake one which always
    * returns as 'not signaled' so we fall back to using the sync object.
    */
   fine->seqno = UINT32_MAX;
   fine->map = &zero;
   fine->syncobj = syncobj;
   pipe_reference_init(&fine->reference, 1);

   struct pipe_fence_handle *fence = calloc(1, sizeof(*fence));
   if (!fence) {
      free(fine);
      free(syncobj);
      *out = NULL;
      return;
   }
   pipe_reference_init(&fence->ref, 1);
   fence->fine[0] = fine;

   *out = fence;
}

static void
iris_fence_signal(struct pipe_context *ctx,
                  struct pipe_fence_handle *fence)
{
   struct iris_context *ice = (struct iris_context *)ctx;

   if (ctx == fence->unflushed_ctx)
      return;

   iris_foreach_batch(ice, batch) {
      for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++) {
         struct iris_fine_fence *fine = fence->fine[i];

         /* already signaled fence skipped */
         if (iris_fine_fence_signaled(fine))
            continue;

         batch->contains_fence_signal = true;
         iris_batch_add_syncobj(batch, fine->syncobj, IRIS_BATCH_FENCE_SIGNAL);
      }
      if (batch->contains_fence_signal)
         iris_batch_flush(batch);
   }
}

void
iris_init_screen_fence_functions(struct pipe_screen *screen)
{
   screen->fence_reference = iris_fence_reference;
   screen->fence_finish = iris_fence_finish;
   screen->fence_get_fd = iris_fence_get_fd;
}

void
iris_init_context_fence_functions(struct pipe_context *ctx)
{
   ctx->flush = iris_fence_flush;
   ctx->create_fence_fd = iris_fence_create_fd;
   ctx->fence_server_sync = iris_fence_await;
   ctx->fence_server_signal = iris_fence_signal;
}
