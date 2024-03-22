/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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

#ifndef FREEDRENO_QUERY_H_
#define FREEDRENO_QUERY_H_

#include "pipe/p_context.h"

#include "util/u_threaded_context.h"

#include "freedreno_util.h"

struct fd_context;
struct fd_query;
struct fd_resource;

struct fd_query_funcs {
   void (*destroy_query)(struct fd_context *ctx, struct fd_query *q) dt;
   void (*begin_query)(struct fd_context *ctx, struct fd_query *q) dt;
   void (*end_query)(struct fd_context *ctx, struct fd_query *q) dt;
   bool (*get_query_result)(struct fd_context *ctx, struct fd_query *q,
                            bool wait, union pipe_query_result *result);
   void (*get_query_result_resource)(struct fd_context *ctx, struct fd_query *q,
                                     enum pipe_query_flags flags,
                                     enum pipe_query_value_type result_type,
                                     int index, struct fd_resource *dst,
                                     unsigned offset) dt;
};

struct fd_query {
   struct threaded_query base;

   const struct fd_query_funcs *funcs;
   int type;
   unsigned index;
};

static inline struct fd_query *
fd_query(struct pipe_query *pq)
{
   return (struct fd_query *)pq;
}

#define FD_QUERY_DRAW_CALLS (PIPE_QUERY_DRIVER_SPECIFIC + 0)
#define FD_QUERY_BATCH_TOTAL                                                   \
   (PIPE_QUERY_DRIVER_SPECIFIC + 1) /* total # of batches (submits) */
#define FD_QUERY_BATCH_SYSMEM                                                  \
   (PIPE_QUERY_DRIVER_SPECIFIC +                                               \
    2) /* batches using system memory (GMEM bypass) */
#define FD_QUERY_BATCH_GMEM                                                    \
   (PIPE_QUERY_DRIVER_SPECIFIC + 3) /* batches using GMEM */
#define FD_QUERY_BATCH_NONDRAW                                                 \
   (PIPE_QUERY_DRIVER_SPECIFIC + 4) /* compute/blit batches */
#define FD_QUERY_BATCH_RESTORE                                                 \
   (PIPE_QUERY_DRIVER_SPECIFIC + 5) /* batches requiring GMEM restore */
#define FD_QUERY_STAGING_UPLOADS                                               \
   (PIPE_QUERY_DRIVER_SPECIFIC +                                               \
    6) /* texture/buffer uploads using staging blit */
#define FD_QUERY_SHADOW_UPLOADS                                                \
   (PIPE_QUERY_DRIVER_SPECIFIC +                                               \
    7) /* texture/buffer uploads that shadowed rsc */
#define FD_QUERY_VS_REGS                                                       \
   (PIPE_QUERY_DRIVER_SPECIFIC +                                               \
    8) /* avg # of VS registers (scaled up by 100x) */
#define FD_QUERY_FS_REGS                                                       \
   (PIPE_QUERY_DRIVER_SPECIFIC +                                               \
    9) /* avg # of VS registers (scaled up by 100x) */
/* insert any new non-perfcntr queries here, the first perfcntr index
 * needs to come last!
 */
#define FD_QUERY_FIRST_PERFCNTR (PIPE_QUERY_DRIVER_SPECIFIC + 10)

void fd_query_screen_init(struct pipe_screen *pscreen);
void fd_query_context_init(struct pipe_context *pctx);

static inline bool
skip_begin_query(int type)
{
   switch (type) {
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_GPU_FINISHED:
      return true;
   default:
      return false;
   }
}

/* maps query_type to sample provider idx: */
static inline int
pidx(unsigned query_type)
{
   switch (query_type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
      return 0;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
      return 1;
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      return 2;
   /* TODO currently queries only emitted in main pass (not in binning pass)..
    * which is fine for occlusion query, but pretty much not anything else.
    */
   case PIPE_QUERY_TIME_ELAPSED:
      return 3;
   case PIPE_QUERY_TIMESTAMP:
      return 4;

   case PIPE_QUERY_PRIMITIVES_GENERATED:
      return 5;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      return 6;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      return 7;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      return 8;
   case PIPE_QUERY_PIPELINE_STATISTICS_SINGLE:
      return 9;

   default:
      return -1;
   }
}

/** Returns true if get_query_result is being called from the driver thread. */
static inline bool
fd_get_query_result_in_driver_thread(struct fd_query *q)
{
   return !q->base.flushed;
}

#endif /* FREEDRENO_QUERY_H_ */
