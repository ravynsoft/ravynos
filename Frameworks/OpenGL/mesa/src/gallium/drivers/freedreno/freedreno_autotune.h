/*
 * Copyright © 2021 Google, Inc.
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
 */

#ifndef FREEDRENO_AUTOTUNE_H
#define FREEDRENO_AUTOTUNE_H

#include "util/hash_table.h"
#include "util/list.h"

#include "freedreno_util.h"

struct fd_autotune_results;

/**
 * "autotune" our decisions about bypass vs GMEM rendering, based on historical
 * data about a given render target.
 *
 * In deciding which path to take there are tradeoffs, including some that
 * are not reasonably estimateable without having some additional information:
 *
 *  (1) If you know you are touching every pixel (ie. there is a glClear()),
 *      then the GMEM path will at least not cost more memory bandwidth than
 *      sysmem[1]
 *
 *  (2) If there is no clear, GMEM could potentially cost *more* bandwidth
 *      due to sysmem->GMEM restore pass.
 *
 *  (3) If you see a high draw count, that is an indication that there will be
 *      enough pixels accessed multiple times to benefit from the reduced
 *      memory bandwidth that GMEM brings
 *
 *  (4) But high draw count where there is not much overdraw can actually be
 *      faster in bypass mode if it is pushing a lot of state change, due to
 *      not having to go thru the state changes per-tile[2]
 *
 * The approach taken is to measure the samples-passed for the batch to estimate
 * the amount of overdraw to detect cases where the number of pixels touched is
 * low.
 *
 * Note however, that (at least since a5xx) we have PERF_RB_{Z,C}_{READ,WRITE}
 * performance countables, which give a more direct measurement of what we want
 * to know (ie. is framebuffer memory access high enough to prefer GMEM), but
 * with the downside of consuming half of the available RB counters.  With the
 * additional complication that external perfcntr collection (fdperf, perfetto)
 * and the drive could be stomping on each other's feet.  (Also reading the
 * perfcntrs accurately requires a WFI.)
 *
 * [1] ignoring UBWC
 * [2] ignoring early-tile-exit optimizations, but any draw that touches all/
 *     most of the tiles late in the tile-pass can defeat that
 */
struct fd_autotune {

   /**
    * Cache to map batch->key (also used for batch-cache) to historical
    * information about rendering to that particular render target.
    */
   struct hash_table *ht;

   /**
    * List of recently used historical results (to age out old results)
    */
   struct list_head lru;

   /**
    * GPU buffer used to communicate back results to the CPU
    */
   struct fd_bo *results_mem;
   struct fd_autotune_results *results;

   /**
    * List of per-batch results that we are waiting for the GPU to finish
    * with before reading back the results.
    */
   struct list_head pending_results;

   uint32_t fence_counter;
   uint32_t idx_counter;
};

/**
 * The layout of the memory used to read back per-batch results from the
 * GPU
 *
 * Note this struct is intentionally aligned to 4k.  And hw requires the
 * sample start/stop locations to be 128b aligned.
 */
struct fd_autotune_results {

   /**
    * The GPU writes back a "fence" seqno value from the cmdstream after
    * it finishes writing it's result slot, so that the CPU knows when
    * results are valid
    */
   uint32_t fence;

   uint32_t __pad0;
   uint64_t __pad1;

   /**
    * From the cmdstream, the captured samples-passed values are recorded
    * at the start and end of the batch.
    *
    * Note that we do the math on the CPU to avoid a WFI.  But pre-emption
    * may force us to revisit that.
    */
   struct {
      uint64_t samples_start;
      uint64_t __pad0;
      uint64_t samples_end;
      uint64_t __pad1;
   } result[127];
};

#define __offset(base, ptr) ((uint8_t *)(ptr) - (uint8_t *)(base))
#define results_ptr(at, member)                                                \
   (at)->results_mem, __offset((at)->results, &(at)->results->member), 0, 0

struct fd_batch_history;

/**
 * Tracks the results from an individual batch.  Initially created per batch,
 * and appended to the tail of at->pending_results.  At a later time, when
 * the GPU has finished writing the results,
 *
 * ralloc parent is the associated fd_batch_history
 */
struct fd_batch_result {

   /**
    * The index/slot in fd_autotune_results::result[] to write start/end
    * counter to
    */
   unsigned idx;

   /**
    * Fence value to write back to fd_autotune_results::fence after both
    * start/end values written
    */
   uint32_t fence;

   /*
    * Below here, only used internally within autotune
    */
   struct fd_batch_history *history;
   struct list_head node;
   uint32_t cost;
   uint64_t samples_passed;
};

void fd_autotune_init(struct fd_autotune *at, struct fd_device *dev);
void fd_autotune_fini(struct fd_autotune *at);

struct fd_batch;
bool fd_autotune_use_bypass(struct fd_autotune *at,
                            struct fd_batch *batch) assert_dt;

#endif /* FREEDRENO_AUTOTUNE_H */
