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

#ifndef FREEDRENO_RINGBUFFER_SP_H_
#define FREEDRENO_RINGBUFFER_SP_H_

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>

#include "util/hash_table.h"
#include "util/os_file.h"
#include "util/slab.h"

#include "freedreno_priv.h"
#include "freedreno_ringbuffer.h"

/* A "softpin" implementation of submit/ringbuffer, which lowers CPU overhead
 * by avoiding the additional tracking necessary to build cmds/relocs tables
 * (but still builds a bos table)
 */

typedef int (*flush_submit_list_fn)(struct list_head *submit_list);

struct fd_submit_sp {
   struct fd_submit base;

   DECLARE_ARRAY(struct fd_bo *, bos);

   /* Keep a separate table of sub-alloc BOs.. the backing objects are
    * tracked in the main bos table (because this is what the kernel
    * sees), but we need to attach userspace fences to the sub-alloc'd
    * BOs so the driver knows when they are idle
    */
   DECLARE_ARRAY(struct fd_bo *, suballoc_bos);

   /* maps fd_bo to idx in bos table: */
   struct hash_table *bo_table;

   /* maps fd_bo to idx in suballoc_bos table: */
   struct hash_table *suballoc_bo_table;

   struct slab_child_pool ring_pool;

   /* Allow for sub-allocation of stateobj ring buffers (ie. sharing
    * the same underlying bo)..
    *
    * We also rely on previous stateobj having been fully constructed
    * so we can reclaim extra space at it's end.
    */
   struct fd_ringbuffer *suballoc_ring;

   /* Flush args, potentially attached to the last submit in the list
    * of submits to merge:
    */
   int in_fence_fd;
   struct fd_fence *out_fence;

   /* State for enqueued submits:
    */
   struct list_head submit_list;   /* includes this submit as last element */

   /* Used by retire_queue, if used by backend: */
   struct util_queue_fence retire_fence;

   flush_submit_list_fn flush_submit_list;

   uint32_t seqno;
};
FD_DEFINE_CAST(fd_submit, fd_submit_sp);

/* for FD_RINGBUFFER_GROWABLE rb's, tracks the 'finalized' cmdstream buffers
 * and sizes.  Ie. a finalized buffer can have no more commands appended to
 * it.
 */
struct fd_cmd_sp {
   struct fd_bo *ring_bo;
   unsigned size;
};

struct fd_ringbuffer_sp {
   struct fd_ringbuffer base;

   /* for FD_RINGBUFFER_STREAMING rb's which are sub-allocated */
   unsigned offset;

   union {
      /* for _FD_RINGBUFFER_OBJECT case, the array of BOs referenced from
       * this one
       */
      struct {
         struct fd_pipe *pipe;
         DECLARE_ARRAY(struct fd_bo *, reloc_bos);
#ifndef NDEBUG
         /* BOs to assert are attached to submit: */
         DECLARE_ARRAY(struct fd_bo *, assert_bos);
#endif

         /**
          * The seqno of the last submit we were emitted to.  For stateobjs
          * it is common to be re-emitted multiple times to the same submit,
          * we can use this to detect the case.
          */
         uint32_t last_submit_seqno;
      };
      /* for other cases: */
      struct {
         struct fd_submit *submit;
         DECLARE_ARRAY(struct fd_cmd_sp, cmds);
      };
   } u;

   struct fd_bo *ring_bo;
};
FD_DEFINE_CAST(fd_ringbuffer, fd_ringbuffer_sp);

void fd_pipe_sp_flush(struct fd_pipe *pipe, uint32_t fence);
uint32_t fd_submit_append_bo(struct fd_submit_sp *submit, struct fd_bo *bo);
struct fd_submit *fd_submit_sp_new(struct fd_pipe *pipe,
                                   flush_submit_list_fn flush_submit_list);
void fd_pipe_sp_ringpool_init(struct fd_pipe *pipe);
void fd_pipe_sp_ringpool_fini(struct fd_pipe *pipe);
struct fd_ringbuffer *fd_ringbuffer_sp_new_object(struct fd_pipe *pipe, uint32_t size);

#endif /* FREEDRENO_RINGBUFFER_SP_H_ */
