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
 * @file crocus_fence.c
 *
 * Fences for driver and IPC serialisation, scheduling and synchronisation.
 */

#include "util/u_inlines.h"
#include "intel/common/intel_gem.h"

#include "crocus_batch.h"
#include "crocus_bufmgr.h"
#include "crocus_context.h"
#include "crocus_fence.h"
#include "crocus_screen.h"

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
struct crocus_syncobj *
crocus_create_syncobj(struct crocus_screen *screen)
{
   struct crocus_syncobj *syncobj = malloc(sizeof(*syncobj));

   if (!syncobj)
      return NULL;

   syncobj->handle = gem_syncobj_create(screen->fd, 0);
   assert(syncobj->handle);

   pipe_reference_init(&syncobj->ref, 1);

   return syncobj;
}

void
crocus_syncobj_destroy(struct crocus_screen *screen,
                       struct crocus_syncobj *syncobj)
{
   gem_syncobj_destroy(screen->fd, syncobj->handle);
   free(syncobj);
}

/**
 * Add a sync-point to the batch, with the given flags.
 *
 * \p flags   One of I915_EXEC_FENCE_WAIT or I915_EXEC_FENCE_SIGNAL.
 */
void
crocus_batch_add_syncobj(struct crocus_batch *batch,
                         struct crocus_syncobj *syncobj, unsigned flags)
{
   struct drm_i915_gem_exec_fence *fence =
      util_dynarray_grow(&batch->exec_fences, struct drm_i915_gem_exec_fence, 1);

   *fence = (struct drm_i915_gem_exec_fence){
      .handle = syncobj->handle,
      .flags = flags,
   };

   struct crocus_syncobj **store =
      util_dynarray_grow(&batch->syncobjs, struct crocus_syncobj *, 1);

   *store = NULL;
   crocus_syncobj_reference(batch->screen, store, syncobj);
}

/**
 * Walk through a batch's dependencies (any I915_EXEC_FENCE_WAIT syncobjs)
 * and unreference any which have already passed.
 *
 * Sometimes the compute batch is seldom used, and accumulates references
 * to stale render batches that are no longer of interest, so we can free
 * those up.
 */
static void
clear_stale_syncobjs(struct crocus_batch *batch)
{
   struct crocus_screen *screen = batch->screen;

   int n = util_dynarray_num_elements(&batch->syncobjs, struct crocus_syncobj *);

   assert(n == util_dynarray_num_elements(&batch->exec_fences,
                                          struct drm_i915_gem_exec_fence));

   /* Skip the first syncobj, as it's the signalling one. */
   for (int i = n - 1; i > 0; i--) {
      struct crocus_syncobj **syncobj =
         util_dynarray_element(&batch->syncobjs, struct crocus_syncobj *, i);
      struct drm_i915_gem_exec_fence *fence =
         util_dynarray_element(&batch->exec_fences,
                               struct drm_i915_gem_exec_fence, i);
      assert(fence->flags & I915_EXEC_FENCE_WAIT);

      if (crocus_wait_syncobj(&screen->base, *syncobj, 0))
         continue;

      /* This sync object has already passed, there's no need to continue
       * marking it as a dependency; we can stop holding on to the reference.
       */
      crocus_syncobj_reference(screen, syncobj, NULL);

      /* Remove it from the lists; move the last element here. */
      struct crocus_syncobj **nth_syncobj =
         util_dynarray_pop_ptr(&batch->syncobjs, struct crocus_syncobj *);
      struct drm_i915_gem_exec_fence *nth_fence =
         util_dynarray_pop_ptr(&batch->exec_fences,
                               struct drm_i915_gem_exec_fence);

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

   struct crocus_fine_fence *fine[CROCUS_BATCH_COUNT];
};

static void
crocus_fence_destroy(struct pipe_screen *p_screen,
                     struct pipe_fence_handle *fence)
{
   struct crocus_screen *screen = (struct crocus_screen *)p_screen;

   for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++)
      crocus_fine_fence_reference(screen, &fence->fine[i], NULL);

   free(fence);
}

static void
crocus_fence_reference(struct pipe_screen *p_screen,
                       struct pipe_fence_handle **dst,
                       struct pipe_fence_handle *src)
{
   if (pipe_reference(&(*dst)->ref, &src->ref))
      crocus_fence_destroy(p_screen, *dst);

   *dst = src;
}

bool
crocus_wait_syncobj(struct pipe_screen *p_screen,
                    struct crocus_syncobj *syncobj, int64_t timeout_nsec)
{
   if (!syncobj)
      return false;

   struct crocus_screen *screen = (struct crocus_screen *)p_screen;
   struct drm_syncobj_wait args = {
      .handles = (uintptr_t)&syncobj->handle,
      .count_handles = 1,
      .timeout_nsec = timeout_nsec,
   };
   return intel_ioctl(screen->fd, DRM_IOCTL_SYNCOBJ_WAIT, &args);
}

static void
crocus_fence_flush(struct pipe_context *ctx,
                   struct pipe_fence_handle **out_fence, unsigned flags)
{
   struct crocus_screen *screen = (void *)ctx->screen;
   struct crocus_context *ice = (struct crocus_context *)ctx;

   const bool deferred = flags & PIPE_FLUSH_DEFERRED;

   if (!deferred) {
      for (unsigned i = 0; i < ice->batch_count; i++)
         crocus_batch_flush(&ice->batches[i]);
   }

   if (!out_fence)
      return;

   struct pipe_fence_handle *fence = calloc(1, sizeof(*fence));
   if (!fence)
      return;

   pipe_reference_init(&fence->ref, 1);

   if (deferred)
      fence->unflushed_ctx = ctx;

   for (unsigned b = 0; b < ice->batch_count; b++) {
      struct crocus_batch *batch = &ice->batches[b];

      if (deferred && crocus_batch_bytes_used(batch) > 0) {
         struct crocus_fine_fence *fine =
            crocus_fine_fence_new(batch, CROCUS_FENCE_BOTTOM_OF_PIPE);
         crocus_fine_fence_reference(screen, &fence->fine[b], fine);
         crocus_fine_fence_reference(screen, &fine, NULL);
      } else {
         /* This batch has no commands queued up (perhaps we just flushed,
          * or all the commands are on the other batch).  Wait for the last
          * syncobj on this engine - unless it's already finished by now.
          */
         if (crocus_fine_fence_signaled(batch->last_fence))
            continue;

         crocus_fine_fence_reference(screen, &fence->fine[b],
                                     batch->last_fence);
      }
   }

   crocus_fence_reference(ctx->screen, out_fence, NULL);
   *out_fence = fence;
}

static void
crocus_fence_await(struct pipe_context *ctx, struct pipe_fence_handle *fence)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;

   /* Unflushed fences from the same context are no-ops. */
   if (ctx && ctx == fence->unflushed_ctx)
      return;

   for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++) {
      struct crocus_fine_fence *fine = fence->fine[i];

      if (crocus_fine_fence_signaled(fine))
         continue;

      for (unsigned b = 0; b < ice->batch_count; b++) {
         struct crocus_batch *batch = &ice->batches[b];

         /* We're going to make any future work in this batch wait for our
          * fence to have gone by.  But any currently queued work doesn't
          * need to wait.  Flush the batch now, so it can happen sooner.
          */
         crocus_batch_flush(batch);

         /* Before adding a new reference, clean out any stale ones. */
         clear_stale_syncobjs(batch);

         crocus_batch_add_syncobj(batch, fine->syncobj, I915_EXEC_FENCE_WAIT);
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
   uint64_t max_timeout = (uint64_t)INT64_MAX - current_time;

   timeout = MIN2(max_timeout, timeout);

   return current_time + timeout;
}

static bool
crocus_fence_finish(struct pipe_screen *p_screen, struct pipe_context *ctx,
                    struct pipe_fence_handle *fence, uint64_t timeout)
{
   ctx = threaded_context_unwrap_sync(ctx);
   struct crocus_context *ice = (struct crocus_context *)ctx;
   struct crocus_screen *screen = (struct crocus_screen *)p_screen;

   /* If we created the fence with PIPE_FLUSH_DEFERRED, we may not have
    * flushed yet.  Check if our syncobj is the current batch's signalling
    * syncobj - if so, we haven't flushed and need to now.
    *
    * The Gallium docs mention that a flush will occur if \p ctx matches
    * the context the fence was created with.  It may be NULL, so we check
    * that it matches first.
    */
   if (ctx && ctx == fence->unflushed_ctx) {
      for (unsigned i = 0; i < ice->batch_count; i++) {
         struct crocus_fine_fence *fine = fence->fine[i];

         if (crocus_fine_fence_signaled(fine))
            continue;

         if (fine->syncobj == crocus_batch_get_signal_syncobj(&ice->batches[i]))
            crocus_batch_flush(&ice->batches[i]);
      }

      /* The fence is no longer deferred. */
      fence->unflushed_ctx = NULL;
   }

   unsigned int handle_count = 0;
   uint32_t handles[ARRAY_SIZE(fence->fine)];
   for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++) {
      struct crocus_fine_fence *fine = fence->fine[i];

      if (crocus_fine_fence_signaled(fine))
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

#ifndef SYNC_IOC_MAGIC
/* duplicated from linux/sync_file.h to avoid build-time dependency
 * on new (v4.7) kernel headers.  Once distro's are mostly using
 * something newer than v4.7 drop this and #include <linux/sync_file.h>
 * instead.
 */
struct sync_merge_data {
   char name[32];
   __s32 fd2;
   __s32 fence;
   __u32 flags;
   __u32 pad;
};

#define SYNC_IOC_MAGIC '>'
#define SYNC_IOC_MERGE _IOWR(SYNC_IOC_MAGIC, 3, struct sync_merge_data)
#endif

static int
sync_merge_fd(int sync_fd, int new_fd)
{
   if (sync_fd == -1)
      return new_fd;

   if (new_fd == -1)
      return sync_fd;

   struct sync_merge_data args = {
      .name = "crocus fence",
      .fd2 = new_fd,
      .fence = -1,
   };

   intel_ioctl(sync_fd, SYNC_IOC_MERGE, &args);
   close(new_fd);
   close(sync_fd);

   return args.fence;
}

static int
crocus_fence_get_fd(struct pipe_screen *p_screen,
                    struct pipe_fence_handle *fence)
{
   struct crocus_screen *screen = (struct crocus_screen *)p_screen;
   int fd = -1;

   /* Deferred fences aren't supported. */
   if (fence->unflushed_ctx)
      return -1;

   for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++) {
      struct crocus_fine_fence *fine = fence->fine[i];

      if (crocus_fine_fence_signaled(fine))
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
         .flags = DRM_SYNCOBJ_HANDLE_TO_FD_FLAGS_EXPORT_SYNC_FILE,
         .fd = -1,
      };

      args.handle = gem_syncobj_create(screen->fd, DRM_SYNCOBJ_CREATE_SIGNALED);
      intel_ioctl(screen->fd, DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD, &args);
      gem_syncobj_destroy(screen->fd, args.handle);
      return args.fd;
   }

   return fd;
}

static void
crocus_fence_create_fd(struct pipe_context *ctx, struct pipe_fence_handle **out,
                       int fd, enum pipe_fd_type type)
{
   assert(type == PIPE_FD_TYPE_NATIVE_SYNC || type == PIPE_FD_TYPE_SYNCOBJ);

   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
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

   struct crocus_syncobj *syncobj = malloc(sizeof(*syncobj));
   if (!syncobj) {
      *out = NULL;
      return;
   }
   syncobj->handle = args.handle;
   pipe_reference_init(&syncobj->ref, 1);

   struct crocus_fine_fence *fine = calloc(1, sizeof(*fine));
   if (!fine) {
      free(syncobj);
      *out = NULL;
      return;
   }

   static const uint32_t zero = 0;

   /* Fences work in terms of crocus_fine_fence, but we don't actually have a
    * seqno for an imported fence.  So, create a fake one which always
    * returns as 'not signaled' so we fall back to using the sync object.
    */
   fine->seqno = UINT32_MAX;
   fine->map = &zero;
   fine->syncobj = syncobj;
   fine->flags = CROCUS_FENCE_END;
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
crocus_fence_signal(struct pipe_context *ctx, struct pipe_fence_handle *fence)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;

   if (ctx == fence->unflushed_ctx)
      return;

   for (unsigned b = 0; b < ice->batch_count; b++) {
      for (unsigned i = 0; i < ARRAY_SIZE(fence->fine); i++) {
         struct crocus_fine_fence *fine = fence->fine[i];

         /* already signaled fence skipped */
         if (crocus_fine_fence_signaled(fine))
            continue;

         ice->batches[b].contains_fence_signal = true;
         crocus_batch_add_syncobj(&ice->batches[b], fine->syncobj,
                                  I915_EXEC_FENCE_SIGNAL);
      }
      if (ice->batches[b].contains_fence_signal)
         crocus_batch_flush(&ice->batches[b]);
   }
}

void
crocus_init_screen_fence_functions(struct pipe_screen *screen)
{
   screen->fence_reference = crocus_fence_reference;
   screen->fence_finish = crocus_fence_finish;
   screen->fence_get_fd = crocus_fence_get_fd;
}

void
crocus_init_context_fence_functions(struct pipe_context *ctx)
{
   ctx->flush = crocus_fence_flush;
   ctx->create_fence_fd = crocus_fence_create_fd;
   ctx->fence_server_sync = crocus_fence_await;
   ctx->fence_server_signal = crocus_fence_signal;
}
