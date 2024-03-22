/*
 * Copyright © 2020 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef CROCUS_FINE_FENCE_DOT_H
#define CROCUS_FINE_FENCE_DOT_H

#include <stdbool.h>
#include <stdint.h>

#include "crocus_screen.h"
#include "crocus_resource.h"

/**
 * A lightweight sequence number fence.
 *
 * We emit PIPE_CONTROLs inside a batch (possibly in the middle)
 * which update a monotonically increasing, 32-bit counter.  We
 * can then check if that moment has passed by either:
 *
 * 1. Checking on the CPU by snooping on the DWord via a coherent map
 *
 * 2. Blocking on the GPU with MI_SEMAPHORE_WAIT from a second batch
 *    (relying on mid-batch preemption to switch GPU execution to the
 *    batch that writes it).
 */
struct crocus_fine_fence {
   struct pipe_reference reference;

   /** Buffer where the seqno lives */
   struct crocus_state_ref ref;

   /** Coherent CPU map of the buffer containing the seqno DWord. */
   const uint32_t *map;

   /**
    * A drm_syncobj pointing which will be signaled at the end of the
    * batch which writes this seqno.  This can be used to block until
    * the seqno has definitely passed (but may wait longer than necessary).
    */
   struct crocus_syncobj *syncobj;

#define CROCUS_FENCE_BOTTOM_OF_PIPE 0x0 /**< Written by bottom-of-pipe flush */
#define CROCUS_FENCE_TOP_OF_PIPE    0x1 /**< Written by top-of-pipe flush */
#define CROCUS_FENCE_END            0x2 /**< Written at the end of a batch */

   /** Information about the type of flush involved (see CROCUS_FENCE_*) */
   uint32_t flags;

   /**
    * Sequence number expected to be written by the flush we inserted
    * when creating this fence.  The crocus_fine_fence is 'signaled' when *@map
    * (written by the flush on the GPU) is greater-than-or-equal to @seqno.
    */
   uint32_t seqno;
};

void crocus_fine_fence_init(struct crocus_batch *batch);

struct crocus_fine_fence *crocus_fine_fence_new(struct crocus_batch *batch,
                                                unsigned flags);

void crocus_fine_fence_destroy(struct crocus_screen *screen,
                               struct crocus_fine_fence *sq);

static inline void
crocus_fine_fence_reference(struct crocus_screen *screen,
                            struct crocus_fine_fence **dst,
                            struct crocus_fine_fence *src)
{
   if (pipe_reference(&(*dst)->reference, &src->reference))
      crocus_fine_fence_destroy(screen, *dst);

   *dst = src;
}

/**
 * Return true if this seqno has passed.
 *
 * NULL is considered signaled.
 */
static inline bool
crocus_fine_fence_signaled(const struct crocus_fine_fence *sq)
{
   if (sq && !sq->map)
      return false;
   return !sq || (READ_ONCE(*sq->map) >= sq->seqno);
}

#endif
