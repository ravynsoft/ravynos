/*
 * Copyright (C) 2012-2018 Rob Clark <robclark@freedesktop.org>
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

#include "util/hash_table.h"
#include "util/set.h"
#include "util/slab.h"

#include "drm/freedreno_ringbuffer.h"
#include "msm_priv.h"

/* The legacy implementation of submit/ringbuffer, which still does the
 * traditional reloc and cmd tracking
 */

#define INIT_SIZE 0x1000

struct msm_submit {
   struct fd_submit base;

   DECLARE_ARRAY(struct drm_msm_gem_submit_bo, submit_bos);
   DECLARE_ARRAY(struct fd_bo *, bos);

   /* maps fd_bo to idx in bos table: */
   struct hash_table *bo_table;

   struct slab_mempool ring_pool;

   /* hash-set of associated rings: */
   struct set *ring_set;

   /* Allow for sub-allocation of stateobj ring buffers (ie. sharing
    * the same underlying bo)..
    *
    * We also rely on previous stateobj having been fully constructed
    * so we can reclaim extra space at it's end.
    */
   struct fd_ringbuffer *suballoc_ring;
};
FD_DEFINE_CAST(fd_submit, msm_submit);

/* for FD_RINGBUFFER_GROWABLE rb's, tracks the 'finalized' cmdstream buffers
 * and sizes.  Ie. a finalized buffer can have no more commands appended to
 * it.
 */
struct msm_cmd {
   struct fd_bo *ring_bo;
   unsigned size;
   DECLARE_ARRAY(struct drm_msm_gem_submit_reloc, relocs);
};

static struct msm_cmd *
cmd_new(struct fd_bo *ring_bo)
{
   struct msm_cmd *cmd = malloc(sizeof(*cmd));
   cmd->ring_bo = fd_bo_ref(ring_bo);
   cmd->size = 0;
   cmd->nr_relocs = cmd->max_relocs = 0;
   cmd->relocs = NULL;
   return cmd;
}

static void
cmd_free(struct msm_cmd *cmd)
{
   fd_bo_del(cmd->ring_bo);
   free(cmd->relocs);
   free(cmd);
}

struct msm_ringbuffer {
   struct fd_ringbuffer base;

   /* for FD_RINGBUFFER_STREAMING rb's which are sub-allocated */
   unsigned offset;

   union {
      /* for _FD_RINGBUFFER_OBJECT case: */
      struct {
         struct fd_pipe *pipe;
         DECLARE_ARRAY(struct fd_bo *, reloc_bos);
         struct set *ring_set;
      };
      /* for other cases: */
      struct {
         struct fd_submit *submit;
         DECLARE_ARRAY(struct msm_cmd *, cmds);
      };
   } u;

   struct msm_cmd *cmd; /* current cmd */
   struct fd_bo *ring_bo;
};
FD_DEFINE_CAST(fd_ringbuffer, msm_ringbuffer);

static void finalize_current_cmd(struct fd_ringbuffer *ring);
static struct fd_ringbuffer *
msm_ringbuffer_init(struct msm_ringbuffer *msm_ring, uint32_t size,
                    enum fd_ringbuffer_flags flags);

/* add (if needed) bo to submit and return index: */
static uint32_t
append_bo(struct msm_submit *submit, struct fd_bo *bo)
{
   uint32_t idx;

   /* NOTE: it is legal to use the same bo on different threads for
    * different submits.  But it is not legal to use the same submit
    * from given threads.
    */
   idx = READ_ONCE(bo->idx);

   if (unlikely((idx >= submit->nr_submit_bos) ||
                (submit->submit_bos[idx].handle != bo->handle))) {
      uint32_t hash = _mesa_hash_pointer(bo);
      struct hash_entry *entry;

      entry = _mesa_hash_table_search_pre_hashed(submit->bo_table, hash, bo);
      if (entry) {
         /* found */
         idx = (uint32_t)(uintptr_t)entry->data;
      } else {
         idx = APPEND(
            submit, submit_bos,
            (struct drm_msm_gem_submit_bo){
               .flags = bo->reloc_flags & (MSM_SUBMIT_BO_READ | MSM_SUBMIT_BO_WRITE),
               .handle = bo->handle,
               .presumed = 0,
            });
         APPEND(submit, bos, fd_bo_ref(bo));

         _mesa_hash_table_insert_pre_hashed(submit->bo_table, hash, bo,
                                            (void *)(uintptr_t)idx);
      }
      bo->idx = idx;
   }

   return idx;
}

static void
append_ring(struct set *set, struct fd_ringbuffer *ring)
{
   uint32_t hash = _mesa_hash_pointer(ring);

   if (!_mesa_set_search_pre_hashed(set, hash, ring)) {
      fd_ringbuffer_ref(ring);
      _mesa_set_add_pre_hashed(set, hash, ring);
   }
}

static void
msm_submit_suballoc_ring_bo(struct fd_submit *submit,
                            struct msm_ringbuffer *msm_ring, uint32_t size)
{
   struct msm_submit *msm_submit = to_msm_submit(submit);
   unsigned suballoc_offset = 0;
   struct fd_bo *suballoc_bo = NULL;

   if (msm_submit->suballoc_ring) {
      struct msm_ringbuffer *suballoc_ring =
         to_msm_ringbuffer(msm_submit->suballoc_ring);

      suballoc_bo = suballoc_ring->ring_bo;
      suballoc_offset =
         fd_ringbuffer_size(msm_submit->suballoc_ring) + suballoc_ring->offset;

      suballoc_offset = align(suballoc_offset, 0x10);

      if ((size + suballoc_offset) > suballoc_bo->size) {
         suballoc_bo = NULL;
      }
   }

   if (!suballoc_bo) {
      // TODO possibly larger size for streaming bo?
      msm_ring->ring_bo = fd_bo_new_ring(submit->pipe->dev, 0x8000);
      msm_ring->offset = 0;
   } else {
      msm_ring->ring_bo = fd_bo_ref(suballoc_bo);
      msm_ring->offset = suballoc_offset;
   }

   struct fd_ringbuffer *old_suballoc_ring = msm_submit->suballoc_ring;

   msm_submit->suballoc_ring = fd_ringbuffer_ref(&msm_ring->base);

   if (old_suballoc_ring)
      fd_ringbuffer_del(old_suballoc_ring);
}

static struct fd_ringbuffer *
msm_submit_new_ringbuffer(struct fd_submit *submit, uint32_t size,
                          enum fd_ringbuffer_flags flags)
{
   struct msm_submit *msm_submit = to_msm_submit(submit);
   struct msm_ringbuffer *msm_ring;

   msm_ring = slab_alloc_st(&msm_submit->ring_pool);

   msm_ring->u.submit = submit;

   /* NOTE: needs to be before _suballoc_ring_bo() since it could
    * increment the refcnt of the current ring
    */
   msm_ring->base.refcnt = 1;

   if (flags & FD_RINGBUFFER_STREAMING) {
      msm_submit_suballoc_ring_bo(submit, msm_ring, size);
   } else {
      if (flags & FD_RINGBUFFER_GROWABLE)
         size = INIT_SIZE;

      msm_ring->offset = 0;
      msm_ring->ring_bo = fd_bo_new_ring(submit->pipe->dev, size);
   }

   if (!msm_ringbuffer_init(msm_ring, size, flags))
      return NULL;

   return &msm_ring->base;
}

static struct drm_msm_gem_submit_reloc *
handle_stateobj_relocs(struct msm_submit *submit, struct msm_ringbuffer *ring)
{
   struct msm_cmd *cmd = ring->cmd;
   struct drm_msm_gem_submit_reloc *relocs;

   relocs = malloc(cmd->nr_relocs * sizeof(*relocs));

   for (unsigned i = 0; i < cmd->nr_relocs; i++) {
      unsigned idx = cmd->relocs[i].reloc_idx;
      struct fd_bo *bo = ring->u.reloc_bos[idx];

      relocs[i] = cmd->relocs[i];
      relocs[i].reloc_idx = append_bo(submit, bo);
   }

   return relocs;
}

static struct fd_fence *
msm_submit_flush(struct fd_submit *submit, int in_fence_fd, bool use_fence_fd)
{
   struct msm_submit *msm_submit = to_msm_submit(submit);
   struct msm_pipe *msm_pipe = to_msm_pipe(submit->pipe);
   struct drm_msm_gem_submit req = {
      .flags = msm_pipe->pipe,
      .queueid = msm_pipe->queue_id,
   };
   int ret;

   finalize_current_cmd(submit->primary);
   append_ring(msm_submit->ring_set, submit->primary);

   unsigned nr_cmds = 0;
   unsigned nr_objs = 0;

   set_foreach (msm_submit->ring_set, entry) {
      struct fd_ringbuffer *ring = (void *)entry->key;
      if (ring->flags & _FD_RINGBUFFER_OBJECT) {
         nr_cmds += 1;
         nr_objs += 1;
      } else {
         if (ring != submit->primary)
            finalize_current_cmd(ring);
         nr_cmds += to_msm_ringbuffer(ring)->u.nr_cmds;
      }
   }

   void *obj_relocs[nr_objs];
   struct drm_msm_gem_submit_cmd cmds[nr_cmds];
   unsigned i = 0, o = 0;

   set_foreach (msm_submit->ring_set, entry) {
      struct fd_ringbuffer *ring = (void *)entry->key;
      struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);

      assert(i < nr_cmds);

      // TODO handle relocs:
      if (ring->flags & _FD_RINGBUFFER_OBJECT) {

         assert(o < nr_objs);

         void *relocs = handle_stateobj_relocs(msm_submit, msm_ring);
         obj_relocs[o++] = relocs;

         cmds[i].type = MSM_SUBMIT_CMD_IB_TARGET_BUF;
         cmds[i].submit_idx = append_bo(msm_submit, msm_ring->ring_bo);
         cmds[i].submit_offset = submit_offset(msm_ring->ring_bo, msm_ring->offset);
         cmds[i].size = offset_bytes(ring->cur, ring->start);
         cmds[i].pad = 0;
         cmds[i].nr_relocs = msm_ring->cmd->nr_relocs;
         cmds[i].relocs = VOID2U64(relocs);

         i++;
      } else {
         for (unsigned j = 0; j < msm_ring->u.nr_cmds; j++) {
            if (ring->flags & FD_RINGBUFFER_PRIMARY) {
               cmds[i].type = MSM_SUBMIT_CMD_BUF;
            } else {
               cmds[i].type = MSM_SUBMIT_CMD_IB_TARGET_BUF;
            }
            struct fd_bo *ring_bo = msm_ring->u.cmds[j]->ring_bo;
            cmds[i].submit_idx = append_bo(msm_submit, ring_bo);
            cmds[i].submit_offset = submit_offset(ring_bo, msm_ring->offset);
            cmds[i].size = msm_ring->u.cmds[j]->size;
            cmds[i].pad = 0;
            cmds[i].nr_relocs = msm_ring->u.cmds[j]->nr_relocs;
            cmds[i].relocs = VOID2U64(msm_ring->u.cmds[j]->relocs);

            i++;
         }
      }
   }

   struct fd_fence *out_fence = fd_fence_new(submit->pipe, use_fence_fd);

   simple_mtx_lock(&fence_lock);
   for (unsigned j = 0; j < msm_submit->nr_bos; j++) {
      fd_bo_add_fence(msm_submit->bos[j], out_fence);
   }
   simple_mtx_unlock(&fence_lock);

   if (in_fence_fd != -1) {
      req.flags |= MSM_SUBMIT_FENCE_FD_IN | MSM_SUBMIT_NO_IMPLICIT;
      req.fence_fd = in_fence_fd;
   }

   if (out_fence->use_fence_fd) {
      req.flags |= MSM_SUBMIT_FENCE_FD_OUT;
   }

   /* needs to be after get_cmd() as that could create bos/cmds table: */
   req.bos = VOID2U64(msm_submit->submit_bos),
   req.nr_bos = msm_submit->nr_submit_bos;
   req.cmds = VOID2U64(cmds), req.nr_cmds = nr_cmds;

   DEBUG_MSG("nr_cmds=%u, nr_bos=%u", req.nr_cmds, req.nr_bos);

   ret = drmCommandWriteRead(submit->pipe->dev->fd, DRM_MSM_GEM_SUBMIT, &req,
                             sizeof(req));
   if (ret) {
      ERROR_MSG("submit failed: %d (%s)", ret, strerror(errno));
      fd_fence_del(out_fence);
      out_fence = NULL;
      msm_dump_submit(&req);
   } else if (!ret && out_fence) {
      out_fence->kfence = req.fence;
      out_fence->ufence = submit->fence;
      out_fence->fence_fd = req.fence_fd;
   }

   for (unsigned o = 0; o < nr_objs; o++)
      free(obj_relocs[o]);

   return out_fence;
}

static void
unref_rings(struct set_entry *entry)
{
   struct fd_ringbuffer *ring = (void *)entry->key;
   fd_ringbuffer_del(ring);
}

static void
msm_submit_destroy(struct fd_submit *submit)
{
   struct msm_submit *msm_submit = to_msm_submit(submit);

   if (msm_submit->suballoc_ring)
      fd_ringbuffer_del(msm_submit->suballoc_ring);

   _mesa_hash_table_destroy(msm_submit->bo_table, NULL);
   _mesa_set_destroy(msm_submit->ring_set, unref_rings);

   // TODO it would be nice to have a way to assert() if all
   // rb's haven't been free'd back to the slab, because that is
   // an indication that we are leaking bo's
   slab_destroy(&msm_submit->ring_pool);

   for (unsigned i = 0; i < msm_submit->nr_bos; i++)
      fd_bo_del(msm_submit->bos[i]);

   free(msm_submit->submit_bos);
   free(msm_submit->bos);
   free(msm_submit);
}

static const struct fd_submit_funcs submit_funcs = {
   .new_ringbuffer = msm_submit_new_ringbuffer,
   .flush = msm_submit_flush,
   .destroy = msm_submit_destroy,
};

struct fd_submit *
msm_submit_new(struct fd_pipe *pipe)
{
   struct msm_submit *msm_submit = calloc(1, sizeof(*msm_submit));
   struct fd_submit *submit;

   msm_submit->bo_table = _mesa_hash_table_create(NULL, _mesa_hash_pointer,
                                                  _mesa_key_pointer_equal);
   msm_submit->ring_set =
      _mesa_set_create(NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);
   // TODO tune size:
   slab_create(&msm_submit->ring_pool, sizeof(struct msm_ringbuffer), 16);

   submit = &msm_submit->base;
   submit->funcs = &submit_funcs;

   return submit;
}

static void
finalize_current_cmd(struct fd_ringbuffer *ring)
{
   struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);

   assert(!(ring->flags & _FD_RINGBUFFER_OBJECT));

   if (!msm_ring->cmd)
      return;

   assert(msm_ring->cmd->ring_bo == msm_ring->ring_bo);

   msm_ring->cmd->size = offset_bytes(ring->cur, ring->start);
   APPEND(&msm_ring->u, cmds, msm_ring->cmd);
   msm_ring->cmd = NULL;
}

static void
msm_ringbuffer_grow(struct fd_ringbuffer *ring, uint32_t size)
{
   struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
   struct fd_pipe *pipe = msm_ring->u.submit->pipe;

   assert(ring->flags & FD_RINGBUFFER_GROWABLE);

   finalize_current_cmd(ring);

   fd_bo_del(msm_ring->ring_bo);
   msm_ring->ring_bo = fd_bo_new_ring(pipe->dev, size);
   msm_ring->cmd = cmd_new(msm_ring->ring_bo);

   ring->start = fd_bo_map(msm_ring->ring_bo);
   ring->end = &(ring->start[size / 4]);
   ring->cur = ring->start;
   ring->size = size;
}

static void
msm_ringbuffer_emit_reloc(struct fd_ringbuffer *ring,
                          const struct fd_reloc *reloc)
{
   struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
   struct fd_pipe *pipe;
   unsigned reloc_idx;

   if (ring->flags & _FD_RINGBUFFER_OBJECT) {
      unsigned idx = APPEND(&msm_ring->u, reloc_bos, fd_bo_ref(reloc->bo));

      /* this gets fixed up at submit->flush() time, since this state-
       * object rb can be used with many different submits
       */
      reloc_idx = idx;

      pipe = msm_ring->u.pipe;
   } else {
      struct msm_submit *msm_submit = to_msm_submit(msm_ring->u.submit);

      reloc_idx = append_bo(msm_submit, reloc->bo);

      pipe = msm_ring->u.submit->pipe;
   }

   APPEND(msm_ring->cmd, relocs,
          (struct drm_msm_gem_submit_reloc){
             .reloc_idx = reloc_idx,
             .reloc_offset = reloc->offset,
             .or = reloc->orval,
             .shift = reloc->shift,
             .submit_offset =
                offset_bytes(ring->cur, ring->start) + msm_ring->offset,
          });

   ring->cur++;

   if (pipe->is_64bit) {
      APPEND(msm_ring->cmd, relocs,
             (struct drm_msm_gem_submit_reloc){
                .reloc_idx = reloc_idx,
                .reloc_offset = reloc->offset,
                .or = reloc->orval >> 32,
                .shift = reloc->shift - 32,
                .submit_offset =
                   offset_bytes(ring->cur, ring->start) + msm_ring->offset,
             });

      ring->cur++;
   }
}

static void
append_stateobj_rings(struct msm_submit *submit, struct fd_ringbuffer *target)
{
   struct msm_ringbuffer *msm_target = to_msm_ringbuffer(target);

   assert(target->flags & _FD_RINGBUFFER_OBJECT);

   set_foreach (msm_target->u.ring_set, entry) {
      struct fd_ringbuffer *ring = (void *)entry->key;

      append_ring(submit->ring_set, ring);

      if (ring->flags & _FD_RINGBUFFER_OBJECT) {
         append_stateobj_rings(submit, ring);
      }
   }
}

static uint32_t
msm_ringbuffer_emit_reloc_ring(struct fd_ringbuffer *ring,
                               struct fd_ringbuffer *target, uint32_t cmd_idx)
{
   struct msm_ringbuffer *msm_target = to_msm_ringbuffer(target);
   struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
   struct fd_bo *bo;
   uint32_t size;

   if ((target->flags & FD_RINGBUFFER_GROWABLE) &&
       (cmd_idx < msm_target->u.nr_cmds)) {
      bo = msm_target->u.cmds[cmd_idx]->ring_bo;
      size = msm_target->u.cmds[cmd_idx]->size;
   } else {
      bo = msm_target->ring_bo;
      size = offset_bytes(target->cur, target->start);
   }

   msm_ringbuffer_emit_reloc(ring, &(struct fd_reloc){
                                      .bo = bo,
                                      .iova = bo->iova + msm_target->offset,
                                      .offset = msm_target->offset,
                                   });

   if (!size)
      return 0;

   if ((target->flags & _FD_RINGBUFFER_OBJECT) &&
       !(ring->flags & _FD_RINGBUFFER_OBJECT)) {
      struct msm_submit *msm_submit = to_msm_submit(msm_ring->u.submit);

      append_stateobj_rings(msm_submit, target);
   }

   if (ring->flags & _FD_RINGBUFFER_OBJECT) {
      append_ring(msm_ring->u.ring_set, target);
   } else {
      struct msm_submit *msm_submit = to_msm_submit(msm_ring->u.submit);
      append_ring(msm_submit->ring_set, target);
   }

   return size;
}

static uint32_t
msm_ringbuffer_cmd_count(struct fd_ringbuffer *ring)
{
   if (ring->flags & FD_RINGBUFFER_GROWABLE)
      return to_msm_ringbuffer(ring)->u.nr_cmds + 1;
   return 1;
}

static bool
msm_ringbuffer_check_size(struct fd_ringbuffer *ring)
{
   assert(!(ring->flags & _FD_RINGBUFFER_OBJECT));
   struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
   struct fd_submit *submit = msm_ring->u.submit;
   struct fd_pipe *pipe = submit->pipe;

   if ((fd_device_version(pipe->dev) < FD_VERSION_UNLIMITED_CMDS) &&
       ((ring->cur - ring->start) > (ring->size / 4 - 0x1000))) {
      return false;
   }

   if (to_msm_submit(submit)->nr_bos > MAX_ARRAY_SIZE/2) {
      return false;
   }

   return true;
}

static void
msm_ringbuffer_destroy(struct fd_ringbuffer *ring)
{
   struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);

   fd_bo_del(msm_ring->ring_bo);
   if (msm_ring->cmd)
      cmd_free(msm_ring->cmd);

   if (ring->flags & _FD_RINGBUFFER_OBJECT) {
      for (unsigned i = 0; i < msm_ring->u.nr_reloc_bos; i++) {
         fd_bo_del(msm_ring->u.reloc_bos[i]);
      }

      _mesa_set_destroy(msm_ring->u.ring_set, unref_rings);

      free(msm_ring->u.reloc_bos);
      free(msm_ring);
   } else {
      struct fd_submit *submit = msm_ring->u.submit;

      for (unsigned i = 0; i < msm_ring->u.nr_cmds; i++) {
         cmd_free(msm_ring->u.cmds[i]);
      }

      free(msm_ring->u.cmds);
      slab_free_st(&to_msm_submit(submit)->ring_pool, msm_ring);
   }
}

static const struct fd_ringbuffer_funcs ring_funcs = {
   .grow = msm_ringbuffer_grow,
   .emit_reloc = msm_ringbuffer_emit_reloc,
   .emit_reloc_ring = msm_ringbuffer_emit_reloc_ring,
   .cmd_count = msm_ringbuffer_cmd_count,
   .check_size = msm_ringbuffer_check_size,
   .destroy = msm_ringbuffer_destroy,
};

static inline struct fd_ringbuffer *
msm_ringbuffer_init(struct msm_ringbuffer *msm_ring, uint32_t size,
                    enum fd_ringbuffer_flags flags)
{
   struct fd_ringbuffer *ring = &msm_ring->base;

   assert(msm_ring->ring_bo);

   uint8_t *base = fd_bo_map(msm_ring->ring_bo);
   ring->start = (void *)(base + msm_ring->offset);
   ring->end = &(ring->start[size / 4]);
   ring->cur = ring->start;

   ring->size = size;
   ring->flags = flags;

   ring->funcs = &ring_funcs;

   msm_ring->u.cmds = NULL;
   msm_ring->u.nr_cmds = msm_ring->u.max_cmds = 0;

   msm_ring->cmd = cmd_new(msm_ring->ring_bo);

   return ring;
}

struct fd_ringbuffer *
msm_ringbuffer_new_object(struct fd_pipe *pipe, uint32_t size)
{
   struct msm_ringbuffer *msm_ring = malloc(sizeof(*msm_ring));

   msm_ring->u.pipe = pipe;
   msm_ring->offset = 0;
   msm_ring->ring_bo = fd_bo_new_ring(pipe->dev, size);
   msm_ring->base.refcnt = 1;

   msm_ring->u.reloc_bos = NULL;
   msm_ring->u.nr_reloc_bos = msm_ring->u.max_reloc_bos = 0;

   msm_ring->u.ring_set =
      _mesa_set_create(NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);

   return msm_ringbuffer_init(msm_ring, size, _FD_RINGBUFFER_OBJECT);
}
