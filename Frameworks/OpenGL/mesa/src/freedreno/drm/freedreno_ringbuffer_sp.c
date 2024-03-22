/*
 * Copyright (C) 2018 Rob Clark <robclark@freedesktop.org>
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

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>

#include "util/hash_table.h"
#include "util/libsync.h"
#include "util/os_file.h"
#include "util/slab.h"

#include "freedreno_ringbuffer_sp.h"

/* A "softpin" implementation of submit/ringbuffer, which lowers CPU overhead
 * by avoiding the additional tracking necessary to build cmds/relocs tables
 * (but still builds a bos table)
 */

/* In the pipe->flush() path, we don't have a util_queue_fence we can wait on,
 * instead use a condition-variable.  Note that pipe->flush() is not expected
 * to be a common/hot path.
 */
static pthread_cond_t  flush_cnd = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t flush_mtx = PTHREAD_MUTEX_INITIALIZER;

static void finalize_current_cmd(struct fd_ringbuffer *ring);
static struct fd_ringbuffer *
fd_ringbuffer_sp_init(struct fd_ringbuffer_sp *fd_ring, uint32_t size,
                      enum fd_ringbuffer_flags flags);


static inline bool
check_append_suballoc_bo(struct fd_submit_sp *submit, struct fd_bo *bo, bool check)
{
   uint32_t idx = READ_ONCE(bo->idx);

   if (unlikely((idx >= submit->nr_suballoc_bos) ||
       (submit->suballoc_bos[idx] != bo))) {
      uint32_t hash = _mesa_hash_pointer(bo);
      struct hash_entry *entry;

      entry = _mesa_hash_table_search_pre_hashed(
            submit->suballoc_bo_table, hash, bo);
      if (entry) {
         /* found */
         idx = (uint32_t)(uintptr_t)entry->data;
      } else if (unlikely(check)) {
         return false;
      } else {
         idx = APPEND(submit, suballoc_bos, fd_bo_ref(bo));

         _mesa_hash_table_insert_pre_hashed(
               submit->suballoc_bo_table, hash, bo, (void *)(uintptr_t)idx);
      }
      bo->idx = idx;
   }

   return true;
}

static inline uint32_t
check_append_bo(struct fd_submit_sp *submit, struct fd_bo *bo, bool check)
{
   if (suballoc_bo(bo)) {
      if (check) {
         if (!check_append_suballoc_bo(submit, bo, true)) {
            return ~0;
         }
         bo = fd_bo_heap_block(bo);
      } else {
         check_append_suballoc_bo(submit, bo, false);
         bo = fd_bo_heap_block(bo);
      }
   }

   /* NOTE: it is legal to use the same bo on different threads for
    * different submits.  But it is not legal to use the same submit
    * from different threads.
    */
   uint32_t idx = READ_ONCE(bo->idx);

   if (unlikely((idx >= submit->nr_bos) || (submit->bos[idx] != bo))) {
      uint32_t hash = _mesa_hash_pointer(bo);
      struct hash_entry *entry;

      entry = _mesa_hash_table_search_pre_hashed(submit->bo_table, hash, bo);
      if (entry) {
         /* found */
         idx = (uint32_t)(uintptr_t)entry->data;
      } else if (unlikely(check)) {
         return ~0;
      } else {
         idx = APPEND(submit, bos, fd_bo_ref(bo));

         _mesa_hash_table_insert_pre_hashed(submit->bo_table, hash, bo,
                                            (void *)(uintptr_t)idx);
      }
      bo->idx = idx;
   }

   return idx;
}

/* add (if needed) bo to submit and return index: */
uint32_t
fd_submit_append_bo(struct fd_submit_sp *submit, struct fd_bo *bo)
{
   return check_append_bo(submit, bo, false);
}

static void
fd_submit_suballoc_ring_bo(struct fd_submit *submit,
                           struct fd_ringbuffer_sp *fd_ring, uint32_t size)
{
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(submit);
   unsigned suballoc_offset = 0;
   struct fd_bo *suballoc_bo = NULL;

   if (fd_submit->suballoc_ring) {
      struct fd_ringbuffer_sp *suballoc_ring =
         to_fd_ringbuffer_sp(fd_submit->suballoc_ring);

      suballoc_bo = suballoc_ring->ring_bo;
      suballoc_offset =
         fd_ringbuffer_size(fd_submit->suballoc_ring) + suballoc_ring->offset;

      suballoc_offset = align(suballoc_offset, SUBALLOC_ALIGNMENT);

      if ((size + suballoc_offset) > suballoc_bo->size) {
         suballoc_bo = NULL;
      }
   }

   if (!suballoc_bo) {
      // TODO possibly larger size for streaming bo?
      fd_ring->ring_bo = fd_bo_new_ring(submit->pipe->dev, SUBALLOC_SIZE);
      fd_ring->offset = 0;
   } else {
      fd_ring->ring_bo = fd_bo_ref(suballoc_bo);
      fd_ring->offset = suballoc_offset;
   }

   struct fd_ringbuffer *old_suballoc_ring = fd_submit->suballoc_ring;

   fd_submit->suballoc_ring = fd_ringbuffer_ref(&fd_ring->base);

   if (old_suballoc_ring)
      fd_ringbuffer_del(old_suballoc_ring);
}

static struct fd_ringbuffer *
fd_submit_sp_new_ringbuffer(struct fd_submit *submit, uint32_t size,
                            enum fd_ringbuffer_flags flags)
{
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(submit);
   struct fd_ringbuffer_sp *fd_ring;

   fd_ring = slab_alloc(&fd_submit->ring_pool);

   fd_ring->u.submit = submit;

   /* NOTE: needs to be before _suballoc_ring_bo() since it could
    * increment the refcnt of the current ring
    */
   fd_ring->base.refcnt = 1;

   if (flags & FD_RINGBUFFER_STREAMING) {
      fd_submit_suballoc_ring_bo(submit, fd_ring, size);
   } else {
      if (flags & FD_RINGBUFFER_GROWABLE)
         size = SUBALLOC_SIZE;

      fd_ring->offset = 0;
      fd_ring->ring_bo = fd_bo_new_ring(submit->pipe->dev, size);
   }

   if (!fd_ringbuffer_sp_init(fd_ring, size, flags))
      return NULL;

   return &fd_ring->base;
}

/**
 * Prepare submit for flush, always done synchronously.
 *
 * 1) Finalize primary ringbuffer, at this point no more cmdstream may
 *    be written into it, since from the PoV of the upper level driver
 *    the submit is flushed, even if deferred
 * 2) Add cmdstream bos to bos table
 * 3) Update bo fences
 */
static bool
fd_submit_sp_flush_prep(struct fd_submit *submit, int in_fence_fd,
                        struct fd_fence *out_fence)
{
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(submit);
   bool has_shared = false;

   finalize_current_cmd(submit->primary);

   struct fd_ringbuffer_sp *primary =
      to_fd_ringbuffer_sp(submit->primary);

   for (unsigned i = 0; i < primary->u.nr_cmds; i++)
      fd_submit_append_bo(fd_submit, primary->u.cmds[i].ring_bo);

   out_fence->ufence = submit->fence;

   simple_mtx_lock(&fence_lock);
   for (unsigned i = 0; i < fd_submit->nr_bos; i++) {
      fd_bo_add_fence(fd_submit->bos[i], out_fence);
      has_shared |= fd_submit->bos[i]->alloc_flags & FD_BO_SHARED;
   }
   for (unsigned i = 0; i < fd_submit->nr_suballoc_bos; i++) {
      fd_bo_add_fence(fd_submit->suballoc_bos[i], out_fence);
   }
   simple_mtx_unlock(&fence_lock);

   fd_submit->out_fence   = fd_fence_ref(out_fence);
   fd_submit->in_fence_fd = (in_fence_fd == -1) ?
         -1 : os_dupfd_cloexec(in_fence_fd);

   return has_shared;
}

static void
fd_submit_sp_flush_execute(void *job, void *gdata, int thread_index)
{
   struct fd_submit *submit = job;
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(submit);
   struct fd_pipe *pipe = submit->pipe;

   fd_submit->flush_submit_list(&fd_submit->submit_list);

   pthread_mutex_lock(&flush_mtx);
   assert(fd_fence_before(pipe->last_submit_fence, fd_submit->base.fence));
   pipe->last_submit_fence = fd_submit->base.fence;
   pthread_cond_broadcast(&flush_cnd);
   pthread_mutex_unlock(&flush_mtx);

   DEBUG_MSG("finish: %u", submit->fence);
}

static void
fd_submit_sp_flush_cleanup(void *job, void *gdata, int thread_index)
{
   struct fd_submit *submit = job;
   fd_submit_del(submit);
}

static void
flush_deferred_submits(struct fd_device *dev)
{
   MESA_TRACE_FUNC();

   simple_mtx_assert_locked(&dev->submit_lock);

   if (list_is_empty(&dev->deferred_submits))
      return;

   struct fd_submit *submit = last_submit(&dev->deferred_submits);
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(submit);
   list_replace(&dev->deferred_submits, &fd_submit->submit_list);
   list_inithead(&dev->deferred_submits);
   dev->deferred_cmds = 0;

   /* If we have multiple submits with in-fence-fd's then merge them: */
   foreach_submit (submit, &fd_submit->submit_list) {
      struct fd_submit_sp *fd_deferred_submit = to_fd_submit_sp(submit);

      if (fd_deferred_submit == fd_submit)
         break;

      if (fd_deferred_submit->in_fence_fd != -1) {
         sync_accumulate("freedreno",
                         &fd_submit->in_fence_fd,
                         fd_deferred_submit->in_fence_fd);
         close(fd_deferred_submit->in_fence_fd);
         fd_deferred_submit->in_fence_fd = -1;
      }
   }

   fd_fence_del(dev->deferred_submits_fence);
   dev->deferred_submits_fence = NULL;

   struct util_queue_fence *fence = &fd_submit->out_fence->ready;

   DEBUG_MSG("enqueue: %u", submit->fence);

   if (fd_device_threaded_submit(submit->pipe->dev)) {
      util_queue_add_job(&submit->pipe->dev->submit_queue,
                         submit, fence,
                         fd_submit_sp_flush_execute,
                         fd_submit_sp_flush_cleanup,
                         0);
   } else {
      fd_submit_sp_flush_execute(submit, NULL, 0);
      fd_submit_sp_flush_cleanup(submit, NULL, 0);
   }
}

static bool
should_defer(struct fd_submit *submit)
{
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(submit);

   /* if too many bo's, it may not be worth the CPU cost of submit merging: */
   if (fd_submit->nr_bos > 30)
      return false;

   /* On the kernel side, with 32K ringbuffer, we have an upper limit of 2k
    * cmds before we exceed the size of the ringbuffer, which results in
    * deadlock writing into the RB (ie. kernel doesn't finish writing into
    * the RB so it doesn't kick the GPU to start consuming from the RB)
    */
   if (submit->pipe->dev->deferred_cmds > 128)
      return false;

   return true;
}

static struct fd_fence *
fd_submit_sp_flush(struct fd_submit *submit, int in_fence_fd, bool use_fence_fd)
{
   struct fd_device *dev = submit->pipe->dev;
   struct fd_pipe *pipe = submit->pipe;

   MESA_TRACE_FUNC();

   /* Acquire lock before flush_prep() because it is possible to race between
    * this and pipe->flush():
    */
   simple_mtx_lock(&dev->submit_lock);

   /* If there are deferred submits from another fd_pipe, flush them now,
    * since we can't merge submits from different submitqueue's (ie. they
    * could have different priority, etc)
    */
   if (!list_is_empty(&dev->deferred_submits) &&
       (last_submit(&dev->deferred_submits)->pipe != submit->pipe)) {
      flush_deferred_submits(dev);
   }

   list_addtail(&fd_submit_ref(submit)->node, &dev->deferred_submits);

   if (!dev->deferred_submits_fence)
      dev->deferred_submits_fence = fd_fence_new(submit->pipe, use_fence_fd);

   struct fd_fence *out_fence = fd_fence_ref(dev->deferred_submits_fence);

   /* upgrade the out_fence for the deferred submits, if needed: */
   if (use_fence_fd)
      out_fence->use_fence_fd = true;

   bool has_shared = fd_submit_sp_flush_prep(submit, in_fence_fd, out_fence);

   if ((in_fence_fd != -1) || out_fence->use_fence_fd)
      pipe->no_implicit_sync = true;

   /* The rule about skipping submit merging with shared buffers is only
    * needed for implicit-sync.
    */
   if (pipe->no_implicit_sync)
      has_shared = false;

   assert(fd_fence_before(pipe->last_enqueue_fence, submit->fence));
   pipe->last_enqueue_fence = submit->fence;

   /* If we don't need an out-fence, we can defer the submit.
    *
    * TODO we could defer submits with in-fence as well.. if we took our own
    * reference to the fd, and merged all the in-fence-fd's when we flush the
    * deferred submits
    */
   if (!use_fence_fd && !has_shared && should_defer(submit)) {
      DEBUG_MSG("defer: %u", submit->fence);
      dev->deferred_cmds += fd_ringbuffer_cmd_count(submit->primary);
      assert(dev->deferred_cmds == fd_dev_count_deferred_cmds(dev));
      simple_mtx_unlock(&dev->submit_lock);

      return out_fence;
   }

   flush_deferred_submits(dev);

   simple_mtx_unlock(&dev->submit_lock);

   return out_fence;
}

void
fd_pipe_sp_flush(struct fd_pipe *pipe, uint32_t fence)
{
   struct fd_device *dev = pipe->dev;

   if (!fd_fence_before(pipe->last_submit_fence, fence))
      return;

   MESA_TRACE_FUNC();

   simple_mtx_lock(&dev->submit_lock);

   assert(!fd_fence_after(fence, pipe->last_enqueue_fence));

   flush_deferred_submits(dev);

   simple_mtx_unlock(&dev->submit_lock);

   if (!fd_device_threaded_submit(pipe->dev))
      return;

   /* Once we are sure that we've enqueued at least up to the requested
    * submit, we need to be sure that submitq has caught up and flushed
    * them to the kernel
    */
   pthread_mutex_lock(&flush_mtx);
   while (fd_fence_before(pipe->last_submit_fence, fence)) {
      pthread_cond_wait(&flush_cnd, &flush_mtx);
   }
   pthread_mutex_unlock(&flush_mtx);
}

static void
fd_submit_sp_destroy(struct fd_submit *submit)
{
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(submit);

   if (fd_submit->suballoc_ring)
      fd_ringbuffer_del(fd_submit->suballoc_ring);

   _mesa_hash_table_destroy(fd_submit->bo_table, NULL);
   _mesa_hash_table_destroy(fd_submit->suballoc_bo_table, NULL);

   // TODO it would be nice to have a way to assert() if all
   // rb's haven't been free'd back to the slab, because that is
   // an indication that we are leaking bo's
   slab_destroy_child(&fd_submit->ring_pool);

   fd_bo_del_array(fd_submit->bos, fd_submit->nr_bos);
   free(fd_submit->bos);

   fd_bo_del_array(fd_submit->suballoc_bos, fd_submit->nr_suballoc_bos);
   free(fd_submit->suballoc_bos);

   if (fd_submit->out_fence)
      fd_fence_del(fd_submit->out_fence);

   free(fd_submit);
}

static const struct fd_submit_funcs submit_funcs = {
   .new_ringbuffer = fd_submit_sp_new_ringbuffer,
   .flush = fd_submit_sp_flush,
   .destroy = fd_submit_sp_destroy,
};

struct fd_submit *
fd_submit_sp_new(struct fd_pipe *pipe, flush_submit_list_fn flush_submit_list)
{
   struct fd_submit_sp *fd_submit = calloc(1, sizeof(*fd_submit));
   struct fd_submit *submit;

   fd_submit->bo_table = _mesa_pointer_hash_table_create(NULL);
   fd_submit->suballoc_bo_table = _mesa_pointer_hash_table_create(NULL);

   slab_create_child(&fd_submit->ring_pool, &pipe->ring_pool);

   fd_submit->flush_submit_list = flush_submit_list;
   fd_submit->seqno = seqno_next(&pipe->submit_seqno);

   submit = &fd_submit->base;
   submit->funcs = &submit_funcs;

   return submit;
}

void
fd_pipe_sp_ringpool_init(struct fd_pipe *pipe)
{
   // TODO tune size:
   slab_create_parent(&pipe->ring_pool, sizeof(struct fd_ringbuffer_sp), 16);
}

void
fd_pipe_sp_ringpool_fini(struct fd_pipe *pipe)
{
   if (pipe->ring_pool.num_elements)
      slab_destroy_parent(&pipe->ring_pool);
}

static void
finalize_current_cmd(struct fd_ringbuffer *ring)
{
   assert(!(ring->flags & _FD_RINGBUFFER_OBJECT));

   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);
   APPEND(&fd_ring->u, cmds,
          (struct fd_cmd_sp){
             .ring_bo = fd_bo_ref(fd_ring->ring_bo),
             .size = offset_bytes(ring->cur, ring->start),
          });
}

static void
fd_ringbuffer_sp_grow(struct fd_ringbuffer *ring, uint32_t size)
{
   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);
   struct fd_pipe *pipe = fd_ring->u.submit->pipe;

   assert(ring->flags & FD_RINGBUFFER_GROWABLE);

   finalize_current_cmd(ring);

   fd_bo_del(fd_ring->ring_bo);
   fd_ring->ring_bo = fd_bo_new_ring(pipe->dev, size);

   ring->start = fd_bo_map(fd_ring->ring_bo);
   ring->end = &(ring->start[size / 4]);
   ring->cur = ring->start;
   ring->size = size;
}

static inline bool
fd_ringbuffer_references_bo(struct fd_ringbuffer *ring, struct fd_bo *bo)
{
   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);

   for (int i = 0; i < fd_ring->u.nr_reloc_bos; i++) {
      if (fd_ring->u.reloc_bos[i] == bo)
         return true;
   }
   return false;
}

static void
fd_ringbuffer_sp_emit_bo_nonobj(struct fd_ringbuffer *ring, struct fd_bo *bo)
{
   assert(!(ring->flags & _FD_RINGBUFFER_OBJECT));

   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(fd_ring->u.submit);

   fd_submit_append_bo(fd_submit, bo);
}

static void
fd_ringbuffer_sp_assert_attached_nonobj(struct fd_ringbuffer *ring, struct fd_bo *bo)
{
#ifndef NDEBUG
   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(fd_ring->u.submit);
   assert(check_append_bo(fd_submit, bo, true) != ~0);
#endif
}

static void
fd_ringbuffer_sp_emit_bo_obj(struct fd_ringbuffer *ring, struct fd_bo *bo)
{
   assert(ring->flags & _FD_RINGBUFFER_OBJECT);

   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);

   /* Avoid emitting duplicate BO references into the list.  Ringbuffer
    * objects are long-lived, so this saves ongoing work at draw time in
    * exchange for a bit at context setup/first draw.  And the number of
    * relocs per ringbuffer object is fairly small, so the O(n^2) doesn't
    * hurt much.
    */
   if (!fd_ringbuffer_references_bo(ring, bo)) {
      APPEND(&fd_ring->u, reloc_bos, fd_bo_ref(bo));
   }
}

static void
fd_ringbuffer_sp_assert_attached_obj(struct fd_ringbuffer *ring, struct fd_bo *bo)
{
#ifndef NDEBUG
   /* If the stateobj already references the bo, nothing more to do: */
   if (fd_ringbuffer_references_bo(ring, bo))
      return;

   /* If not, we need to defer the assert.. because the batch resource
    * tracking may have attached the bo to the submit that the stateobj
    * will eventually be referenced by:
    */
   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);
   for (int i = 0; i < fd_ring->u.nr_assert_bos; i++)
      if (fd_ring->u.assert_bos[i] == bo)
         return;

   APPEND(&fd_ring->u, assert_bos, fd_bo_ref(bo));
#endif
}

#define PTRSZ 64
#include "freedreno_ringbuffer_sp_reloc.h"
#undef PTRSZ
#define PTRSZ 32
#include "freedreno_ringbuffer_sp_reloc.h"
#undef PTRSZ

static uint32_t
fd_ringbuffer_sp_cmd_count(struct fd_ringbuffer *ring)
{
   if (ring->flags & FD_RINGBUFFER_GROWABLE)
      return to_fd_ringbuffer_sp(ring)->u.nr_cmds + 1;
   return 1;
}

static bool
fd_ringbuffer_sp_check_size(struct fd_ringbuffer *ring)
{
   assert(!(ring->flags & _FD_RINGBUFFER_OBJECT));
   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);
   struct fd_submit *submit = fd_ring->u.submit;

   if (to_fd_submit_sp(submit)->nr_bos > MAX_ARRAY_SIZE/2) {
      return false;
   }

   if (to_fd_submit_sp(submit)->nr_suballoc_bos > MAX_ARRAY_SIZE/2) {
      return false;
   }

   return true;
}

static void
fd_ringbuffer_sp_destroy(struct fd_ringbuffer *ring)
{
   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);

   fd_bo_del(fd_ring->ring_bo);

   if (ring->flags & _FD_RINGBUFFER_OBJECT) {
      fd_bo_del_array(fd_ring->u.reloc_bos, fd_ring->u.nr_reloc_bos);
      free(fd_ring->u.reloc_bos);
#ifndef NDEBUG
      fd_bo_del_array(fd_ring->u.assert_bos, fd_ring->u.nr_assert_bos);
      free(fd_ring->u.assert_bos);
#endif
      free(fd_ring);
   } else {
      struct fd_submit *submit = fd_ring->u.submit;

      // TODO re-arrange the data structures so we can use fd_bo_del_array()
      for (unsigned i = 0; i < fd_ring->u.nr_cmds; i++) {
         fd_bo_del(fd_ring->u.cmds[i].ring_bo);
      }
      free(fd_ring->u.cmds);

      slab_free(&to_fd_submit_sp(submit)->ring_pool, fd_ring);
   }
}

static const struct fd_ringbuffer_funcs ring_funcs_nonobj_32 = {
   .grow = fd_ringbuffer_sp_grow,
   .emit_bo = fd_ringbuffer_sp_emit_bo_nonobj,
   .assert_attached = fd_ringbuffer_sp_assert_attached_nonobj,
   .emit_reloc = fd_ringbuffer_sp_emit_reloc_nonobj_32,
   .emit_reloc_ring = fd_ringbuffer_sp_emit_reloc_ring_32,
   .cmd_count = fd_ringbuffer_sp_cmd_count,
   .check_size = fd_ringbuffer_sp_check_size,
   .destroy = fd_ringbuffer_sp_destroy,
};

static const struct fd_ringbuffer_funcs ring_funcs_obj_32 = {
   .grow = fd_ringbuffer_sp_grow,
   .emit_bo = fd_ringbuffer_sp_emit_bo_obj,
   .assert_attached = fd_ringbuffer_sp_assert_attached_obj,
   .emit_reloc = fd_ringbuffer_sp_emit_reloc_obj_32,
   .emit_reloc_ring = fd_ringbuffer_sp_emit_reloc_ring_32,
   .cmd_count = fd_ringbuffer_sp_cmd_count,
   .destroy = fd_ringbuffer_sp_destroy,
};

static const struct fd_ringbuffer_funcs ring_funcs_nonobj_64 = {
   .grow = fd_ringbuffer_sp_grow,
   .emit_bo = fd_ringbuffer_sp_emit_bo_nonobj,
   .assert_attached = fd_ringbuffer_sp_assert_attached_nonobj,
   .emit_reloc = fd_ringbuffer_sp_emit_reloc_nonobj_64,
   .emit_reloc_ring = fd_ringbuffer_sp_emit_reloc_ring_64,
   .cmd_count = fd_ringbuffer_sp_cmd_count,
   .check_size = fd_ringbuffer_sp_check_size,
   .destroy = fd_ringbuffer_sp_destroy,
};

static const struct fd_ringbuffer_funcs ring_funcs_obj_64 = {
   .grow = fd_ringbuffer_sp_grow,
   .emit_bo = fd_ringbuffer_sp_emit_bo_obj,
   .assert_attached = fd_ringbuffer_sp_assert_attached_obj,
   .emit_reloc = fd_ringbuffer_sp_emit_reloc_obj_64,
   .emit_reloc_ring = fd_ringbuffer_sp_emit_reloc_ring_64,
   .cmd_count = fd_ringbuffer_sp_cmd_count,
   .destroy = fd_ringbuffer_sp_destroy,
};

static inline struct fd_ringbuffer *
fd_ringbuffer_sp_init(struct fd_ringbuffer_sp *fd_ring, uint32_t size,
                      enum fd_ringbuffer_flags flags)
{
   struct fd_ringbuffer *ring = &fd_ring->base;

   assert(fd_ring->ring_bo);

   uint8_t *base = fd_bo_map(fd_ring->ring_bo);
   ring->start = (void *)(base + fd_ring->offset);
   ring->end = &(ring->start[size / 4]);
   ring->cur = ring->start;

   ring->size = size;
   ring->flags = flags;

   if (flags & _FD_RINGBUFFER_OBJECT) {
      if (fd_ring->u.pipe->is_64bit) {
         ring->funcs = &ring_funcs_obj_64;
      } else {
         ring->funcs = &ring_funcs_obj_32;
      }
   } else {
      if (fd_ring->u.submit->pipe->is_64bit) {
         ring->funcs = &ring_funcs_nonobj_64;
      } else {
         ring->funcs = &ring_funcs_nonobj_32;
      }
   }

   // TODO initializing these could probably be conditional on flags
   // since unneed for FD_RINGBUFFER_STAGING case..
   fd_ring->u.cmds = NULL;
   fd_ring->u.nr_cmds = fd_ring->u.max_cmds = 0;

   fd_ring->u.reloc_bos = NULL;
   fd_ring->u.nr_reloc_bos = fd_ring->u.max_reloc_bos = 0;
#ifndef NDEBUG
   fd_ring->u.assert_bos = NULL;
   fd_ring->u.nr_assert_bos = fd_ring->u.max_assert_bos = 0;
#endif

   return ring;
}

struct fd_ringbuffer *
fd_ringbuffer_sp_new_object(struct fd_pipe *pipe, uint32_t size)
{
   struct fd_device *dev = pipe->dev;
   struct fd_ringbuffer_sp *fd_ring = malloc(sizeof(*fd_ring));

   /* Lock access to the fd_pipe->suballoc_* since ringbuffer object allocation
    * can happen both on the frontend (most CSOs) and the driver thread (a6xx
    * cached tex state, for example)
    */
   simple_mtx_lock(&dev->suballoc_lock);

   fd_ring->offset = align(dev->suballoc_offset, SUBALLOC_ALIGNMENT);
   if (!dev->suballoc_bo ||
       fd_ring->offset + size > fd_bo_size(dev->suballoc_bo)) {
      if (dev->suballoc_bo)
         fd_bo_del(dev->suballoc_bo);
      dev->suballoc_bo =
         fd_bo_new_ring(dev, MAX2(SUBALLOC_SIZE, align(size, 4096)));
      fd_ring->offset = 0;
   }

   fd_ring->u.pipe = pipe;
   fd_ring->ring_bo = fd_bo_ref(dev->suballoc_bo);
   fd_ring->base.refcnt = 1;
   fd_ring->u.last_submit_seqno = 0;

   dev->suballoc_offset = fd_ring->offset + size;

   simple_mtx_unlock(&dev->suballoc_lock);

   return fd_ringbuffer_sp_init(fd_ring, size, _FD_RINGBUFFER_OBJECT);
}
