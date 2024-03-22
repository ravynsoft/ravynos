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

#include "pipe/p_state.h"
#include "util/os_time.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "freedreno_context.h"
#include "freedreno_query_sw.h"
#include "freedreno_util.h"

/*
 * SW Queries:
 *
 * In the core, we have some support for basic sw counters
 */

static void
fd_sw_destroy_query(struct fd_context *ctx, struct fd_query *q)
{
   struct fd_sw_query *sq = fd_sw_query(q);
   free(sq);
}

static uint64_t
read_counter(struct fd_context *ctx, int type) assert_dt
{
   switch (type) {
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      return ctx->stats.prims_generated;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      return ctx->stats.prims_emitted;
   case FD_QUERY_DRAW_CALLS:
      return ctx->stats.draw_calls;
   case FD_QUERY_BATCH_TOTAL:
      return ctx->stats.batch_total;
   case FD_QUERY_BATCH_SYSMEM:
      return ctx->stats.batch_sysmem;
   case FD_QUERY_BATCH_GMEM:
      return ctx->stats.batch_gmem;
   case FD_QUERY_BATCH_NONDRAW:
      return ctx->stats.batch_nondraw;
   case FD_QUERY_BATCH_RESTORE:
      return ctx->stats.batch_restore;
   case FD_QUERY_STAGING_UPLOADS:
      return ctx->stats.staging_uploads;
   case FD_QUERY_SHADOW_UPLOADS:
      return ctx->stats.shadow_uploads;
   case FD_QUERY_VS_REGS:
      return ctx->stats.vs_regs;
   case FD_QUERY_FS_REGS:
      return ctx->stats.fs_regs;
   }
   return 0;
}

static bool
is_time_rate_query(struct fd_query *q)
{
   switch (q->type) {
   case FD_QUERY_BATCH_TOTAL:
   case FD_QUERY_BATCH_SYSMEM:
   case FD_QUERY_BATCH_GMEM:
   case FD_QUERY_BATCH_NONDRAW:
   case FD_QUERY_BATCH_RESTORE:
   case FD_QUERY_STAGING_UPLOADS:
   case FD_QUERY_SHADOW_UPLOADS:
      return true;
   default:
      return false;
   }
}

static bool
is_draw_rate_query(struct fd_query *q)
{
   switch (q->type) {
   case FD_QUERY_VS_REGS:
   case FD_QUERY_FS_REGS:
      return true;
   default:
      return false;
   }
}

static void
fd_sw_begin_query(struct fd_context *ctx, struct fd_query *q) assert_dt
{
   struct fd_sw_query *sq = fd_sw_query(q);

   ctx->stats_users++;

   sq->begin_value = read_counter(ctx, q->type);
   if (is_time_rate_query(q)) {
      sq->begin_time = os_time_get();
   } else if (is_draw_rate_query(q)) {
      sq->begin_time = ctx->stats.draw_calls;
   }
}

static void
fd_sw_end_query(struct fd_context *ctx, struct fd_query *q) assert_dt
{
   struct fd_sw_query *sq = fd_sw_query(q);

   assert(ctx->stats_users > 0);
   ctx->stats_users--;

   sq->end_value = read_counter(ctx, q->type);
   if (is_time_rate_query(q)) {
      sq->end_time = os_time_get();
   } else if (is_draw_rate_query(q)) {
      sq->end_time = ctx->stats.draw_calls;
   }
}

static bool
fd_sw_get_query_result(struct fd_context *ctx, struct fd_query *q, bool wait,
                       union pipe_query_result *result)
{
   struct fd_sw_query *sq = fd_sw_query(q);

   result->u64 = sq->end_value - sq->begin_value;

   if (is_time_rate_query(q)) {
      double fps =
         (result->u64 * 1000000) / (double)(sq->end_time - sq->begin_time);
      result->u64 = (uint64_t)fps;
   } else if (is_draw_rate_query(q)) {
      double avg =
         ((double)result->u64) / (double)(sq->end_time - sq->begin_time);
      result->f = avg;
   }

   return true;
}

static const struct fd_query_funcs sw_query_funcs = {
   .destroy_query = fd_sw_destroy_query,
   .begin_query = fd_sw_begin_query,
   .end_query = fd_sw_end_query,
   .get_query_result = fd_sw_get_query_result,
};

struct fd_query *
fd_sw_create_query(struct fd_context *ctx, unsigned query_type, unsigned index)
{
   struct fd_sw_query *sq;
   struct fd_query *q;

   switch (query_type) {
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case FD_QUERY_DRAW_CALLS:
   case FD_QUERY_BATCH_TOTAL:
   case FD_QUERY_BATCH_SYSMEM:
   case FD_QUERY_BATCH_GMEM:
   case FD_QUERY_BATCH_NONDRAW:
   case FD_QUERY_BATCH_RESTORE:
   case FD_QUERY_STAGING_UPLOADS:
   case FD_QUERY_SHADOW_UPLOADS:
   case FD_QUERY_VS_REGS:
   case FD_QUERY_FS_REGS:
      break;
   default:
      return NULL;
   }

   sq = CALLOC_STRUCT(fd_sw_query);
   if (!sq)
      return NULL;

   q = &sq->base;
   q->funcs = &sw_query_funcs;
   q->type = query_type;

   return q;
}
