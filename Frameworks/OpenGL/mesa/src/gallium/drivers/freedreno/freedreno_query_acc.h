/*
 * Copyright (C) 2017 Rob Clark <robclark@freedesktop.org>
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

#ifndef FREEDRENO_QUERY_ACC_H_
#define FREEDRENO_QUERY_ACC_H_

#include "util/list.h"

#include "freedreno_context.h"
#include "freedreno_query.h"
#include "freedreno_resource.h"

BEGINC;

/*
 * Accumulated HW Queries:
 *
 * Unlike the original HW Queries in earlier adreno generations (see
 * freedreno_query_hw.[ch], later generations can accumulate the per-
 * tile results of some (a4xx) or all (a5xx+?) queries in the cmdstream.
 * But we still need to handle pausing/resuming the query across stage
 * changes (in particular when switching between batches).
 *
 * fd_acc_sample_provider:
 *   - one per accumulated query type, registered/implemented by gpu
 *     generation specific code
 *   - knows how to emit cmdstream to pause/resume a query instance
 *
 * fd_acc_query:
 *   - one instance per query object
 *   - each query object has it's own result buffer, which may
 *     span multiple batches, etc.
 */

struct fd_acc_query;

/**
 * Base class for all query samples, on the GPU 'avail' is written to
 * one when the query result is available.
 */
struct PACKED fd_acc_query_sample {
   uint64_t avail;
};


/**
 * Helper to assert sample struct field has required alignment (ie. to
 * catch issues at compile time if struct fd_acc_query_sample header
 * ever changed, and to make the hw requirements more obvious)
 */
#define ASSERT_ALIGNED(type, field, nbytes) \
   STATIC_ASSERT((offsetof(type, field) % nbytes) == 0)

struct fd_acc_sample_provider {
   unsigned query_type;

   /* Set if the provider should still count while !ctx->active_queries */
   bool always;

   unsigned size;

   void (*resume)(struct fd_acc_query *aq, struct fd_batch *batch) dt;
   void (*pause)(struct fd_acc_query *aq, struct fd_batch *batch) dt;

   void (*result)(struct fd_acc_query *aq, struct fd_acc_query_sample *s,
                  union pipe_query_result *result);
   void (*result_resource)(struct fd_acc_query *aq, struct fd_ringbuffer *ring,
                           enum pipe_query_value_type result_type, int index,
                           struct fd_resource *dst, unsigned offset);
};

struct fd_acc_query {
   struct fd_query base;

   const struct fd_acc_sample_provider *provider;

   struct pipe_resource *prsc;

   /* Pointer to the batch that our query has had resume() called on (if
    * any).
    */
   struct fd_batch *batch;

   /* usually the same as provider->size but for batch queries we
    * need to calculate the size dynamically when the query is
    * allocated:
    */
   unsigned size;

   struct list_head node; /* list-node in ctx->active_acc_queries */

   void *query_data; /* query specific data */
};

static inline struct fd_acc_query *
fd_acc_query(struct fd_query *q)
{
   return (struct fd_acc_query *)q;
}

struct fd_query *fd_acc_create_query(struct fd_context *ctx,
                                     unsigned query_type, unsigned index);
struct fd_query *
fd_acc_create_query2(struct fd_context *ctx, unsigned query_type,
                     unsigned index,
                     const struct fd_acc_sample_provider *provider);
void fd_acc_query_update_batch(struct fd_batch *batch,
                               bool disable_all) assert_dt;
void
fd_acc_query_register_provider(struct pipe_context *pctx,
                               const struct fd_acc_sample_provider *provider);

static inline void
copy_result(struct fd_ringbuffer *ring, enum pipe_query_value_type result_type,
            struct fd_resource *dst, unsigned dst_offset,
            struct fd_resource *src, unsigned src_offset)
{
   fd_ringbuffer_attach_bo(ring, dst->bo);
   fd_ringbuffer_attach_bo(ring, src->bo);

   OUT_PKT7(ring, CP_MEM_TO_MEM, 5);
   OUT_RING(ring, COND(result_type >= PIPE_QUERY_TYPE_I64, CP_MEM_TO_MEM_0_DOUBLE));
   OUT_RELOC(ring, dst->bo, dst_offset, 0, 0);
   OUT_RELOC(ring, src->bo, src_offset, 0, 0);
}

ENDC;

#endif /* FREEDRENO_QUERY_ACC_H_ */
