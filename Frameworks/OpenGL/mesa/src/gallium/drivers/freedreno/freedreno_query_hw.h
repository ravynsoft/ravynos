/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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

#ifndef FREEDRENO_QUERY_HW_H_
#define FREEDRENO_QUERY_HW_H_

#include "util/list.h"

#include "freedreno_context.h"
#include "freedreno_query.h"

/*
 * HW Queries:
 *
 * See: https://gitlab.freedesktop.org/freedreno/freedreno/-/wikis/Queries#hardware-queries
 *
 * Hardware queries will be specific to gpu generation, but they need
 * some common infrastructure for triggering start/stop samples at
 * various points (for example, to exclude mem2gmem/gmem2mem or clear)
 * as well as per tile tracking.
 *
 * NOTE: in at least some cases hw writes sample values to memory addr
 * specified in some register.  So we don't really have the option to
 * just sample the same counter multiple times for multiple different
 * queries with the same query_type.  So we cache per sample provider
 * the most recent sample since the last draw.  This way multiple
 * sample periods for multiple queries can reference the same sample.
 *
 * fd_hw_sample_provider:
 *   - one per query type, registered/implemented by gpu generation
 *     specific code
 *   - can construct fd_hw_samples on demand
 *   - most recent sample (since last draw) cached so multiple
 *     different queries can ref the same sample
 *
 * fd_hw_sample:
 *   - abstracts one snapshot of counter value(s) across N tiles
 *   - backing object not allocated until submit time when number
 *     of samples and number of tiles is known
 *
 * fd_hw_sample_period:
 *   - consists of start and stop sample
 *   - a query accumulates a list of sample periods
 *   - the query result is the sum of the sample periods
 */

struct fd_hw_sample_provider {
   unsigned query_type;

   /* Set if the provider should still count while !ctx->active_queries */
   bool always;

   /* Optional hook for enabling a counter.  Guaranteed to happen
    * at least once before the first ->get_sample() in a batch.
    */
   void (*enable)(struct fd_context *ctx, struct fd_ringbuffer *ring) dt;

   /* when a new sample is required, emit appropriate cmdstream
    * and return a sample object:
    */
   struct fd_hw_sample *(*get_sample)(struct fd_batch *batch,
                                      struct fd_ringbuffer *ring)dt;

   /* accumulate the results from specified sample period: */
   void (*accumulate_result)(struct fd_context *ctx, const void *start,
                             const void *end, union pipe_query_result *result);
};

struct fd_hw_sample {
   struct pipe_reference reference; /* keep this first */

   /* offset and size of the sample are know at the time the
    * sample is constructed.
    */
   uint32_t size;
   uint32_t offset;

   /* backing object, offset/stride/etc are determined not when
    * the sample is constructed, but when the batch is submitted.
    * This way we can defer allocation until total # of requested
    * samples, and total # of tiles, is known.
    */
   struct pipe_resource *prsc;
   uint32_t num_tiles;
   uint32_t tile_stride;
};

struct fd_hw_sample_period;

struct fd_hw_query {
   struct fd_query base;

   const struct fd_hw_sample_provider *provider;

   /* list of fd_hw_sample_periods: */
   struct list_head periods;

   /* if active and not paused, the current sample period (not
    * yet added to current_periods):
    */
   struct fd_hw_sample_period *period;

   struct list_head list; /* list-node in batch->active_queries */
};

static inline struct fd_hw_query *
fd_hw_query(struct fd_query *q)
{
   return (struct fd_hw_query *)q;
}

struct fd_query *fd_hw_create_query(struct fd_context *ctx, unsigned query_type,
                                    unsigned index);
/* helper for sample providers: */
struct fd_hw_sample *fd_hw_sample_init(struct fd_batch *batch, uint32_t size);
/* don't call directly, use fd_hw_sample_reference() */
void __fd_hw_sample_destroy(struct fd_context *ctx, struct fd_hw_sample *samp);
void fd_hw_query_prepare(struct fd_batch *batch, uint32_t num_tiles) assert_dt;
void fd_hw_query_prepare_tile(struct fd_batch *batch, uint32_t n,
                              struct fd_ringbuffer *ring) assert_dt;
void fd_hw_query_update_batch(struct fd_batch *batch, bool end_batch) assert_dt;
void fd_hw_query_enable(struct fd_batch *batch,
                        struct fd_ringbuffer *ring) assert_dt;
void
fd_hw_query_register_provider(struct pipe_context *pctx,
                              const struct fd_hw_sample_provider *provider);
void fd_hw_query_init(struct pipe_context *pctx);
void fd_hw_query_fini(struct pipe_context *pctx);

static inline void
fd_hw_sample_reference(struct fd_context *ctx, struct fd_hw_sample **ptr,
                       struct fd_hw_sample *samp)
{
   struct fd_hw_sample *old_samp = *ptr;

   if (pipe_reference(&(*ptr)->reference, &samp->reference))
      __fd_hw_sample_destroy(ctx, old_samp);
   *ptr = samp;
}

#endif /* FREEDRENO_QUERY_HW_H_ */
