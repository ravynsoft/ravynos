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

#include "pipe/p_state.h"
#include "util/u_memory.h"

#include "freedreno_context.h"
#include "freedreno_query.h"
#include "freedreno_query_hw.h"
#include "freedreno_query_sw.h"
#include "freedreno_resource.h"
#include "freedreno_util.h"

/*
 * Pipe Query interface:
 */

static struct pipe_query *
fd_create_query(struct pipe_context *pctx, unsigned query_type, unsigned index)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_query *q = NULL;

   if (ctx->create_query)
      q = ctx->create_query(ctx, query_type, index);
   if (!q)
      q = fd_sw_create_query(ctx, query_type, index);

   return (struct pipe_query *)q;
}

static void
fd_destroy_query(struct pipe_context *pctx, struct pipe_query *pq) in_dt
{
   struct fd_query *q = fd_query(pq);
   q->funcs->destroy_query(fd_context(pctx), q);
}

static bool
fd_begin_query(struct pipe_context *pctx, struct pipe_query *pq) in_dt
{
   struct fd_query *q = fd_query(pq);

   q->funcs->begin_query(fd_context(pctx), q);

   return true;
}

static bool
fd_end_query(struct pipe_context *pctx, struct pipe_query *pq) in_dt
{
   struct fd_query *q = fd_query(pq);

   /* there are a couple special cases, which don't have
    * a matching ->begin_query():
    */
   if (skip_begin_query(q->type))
      fd_begin_query(pctx, pq);

   q->funcs->end_query(fd_context(pctx), q);

   return true;
}

static bool
fd_get_query_result(struct pipe_context *pctx, struct pipe_query *pq, bool wait,
                    union pipe_query_result *result)
{
   struct fd_query *q = fd_query(pq);

   util_query_clear_result(result, q->type);

   return q->funcs->get_query_result(fd_context(pctx), q, wait, result);
}

static void
fd_get_query_result_resource(struct pipe_context *pctx, struct pipe_query *pq,
                             enum pipe_query_flags flags,
                             enum pipe_query_value_type result_type,
                             int index, struct pipe_resource *prsc,
                             unsigned offset)
   in_dt
{
   struct fd_query *q = fd_query(pq);

   q->funcs->get_query_result_resource(fd_context(pctx), q, flags, result_type,
                                       index, fd_resource(prsc), offset);
}

static void
fd_render_condition(struct pipe_context *pctx, struct pipe_query *pq,
                    bool condition, enum pipe_render_cond_flag mode) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->cond_query = pq;
   ctx->cond_cond = condition;
   ctx->cond_mode = mode;
}

#define _Q(_name, _query_type, _type, _result_type) {                          \
      .name = _name, .query_type = _query_type,                                \
      .type = PIPE_DRIVER_QUERY_TYPE_##_type,                                  \
      .result_type = PIPE_DRIVER_QUERY_RESULT_TYPE_##_result_type,             \
      .group_id = ~(unsigned)0,                                                \
   }

#define FQ(_name, _query_type, _type, _result_type)                            \
   _Q(_name, FD_QUERY_##_query_type, _type, _result_type)

#define PQ(_name, _query_type, _type, _result_type)                            \
   _Q(_name, PIPE_QUERY_##_query_type, _type, _result_type)

static const struct pipe_driver_query_info sw_query_list[] = {
   FQ("draw-calls", DRAW_CALLS, UINT64, AVERAGE),
   FQ("batches", BATCH_TOTAL, UINT64, AVERAGE),
   FQ("batches-sysmem", BATCH_SYSMEM, UINT64, AVERAGE),
   FQ("batches-gmem", BATCH_GMEM, UINT64, AVERAGE),
   FQ("batches-nondraw", BATCH_NONDRAW, UINT64, AVERAGE),
   FQ("restores", BATCH_RESTORE, UINT64, AVERAGE),
   PQ("prims-emitted", PRIMITIVES_EMITTED, UINT64, AVERAGE),
   FQ("staging", STAGING_UPLOADS, UINT64, AVERAGE),
   FQ("shadow", SHADOW_UPLOADS, UINT64, AVERAGE),
   FQ("vsregs", VS_REGS, FLOAT, AVERAGE),
   FQ("fsregs", FS_REGS, FLOAT, AVERAGE),
};

static int
fd_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                         struct pipe_driver_query_info *info)
{
   struct fd_screen *screen = fd_screen(pscreen);

   if (!info)
      return ARRAY_SIZE(sw_query_list) + screen->num_perfcntr_queries;

   if (index >= ARRAY_SIZE(sw_query_list)) {
      index -= ARRAY_SIZE(sw_query_list);
      if (index >= screen->num_perfcntr_queries)
         return 0;
      *info = screen->perfcntr_queries[index];
      return 1;
   }

   *info = sw_query_list[index];
   return 1;
}

static int
fd_get_driver_query_group_info(struct pipe_screen *pscreen, unsigned index,
                               struct pipe_driver_query_group_info *info)
{
   struct fd_screen *screen = fd_screen(pscreen);

   if (!info)
      return screen->num_perfcntr_groups;

   if (index >= screen->num_perfcntr_groups)
      return 0;

   const struct fd_perfcntr_group *g = &screen->perfcntr_groups[index];

   info->name = g->name;
   info->max_active_queries = g->num_counters;
   info->num_queries = g->num_countables;

   return 1;
}

static void
fd_set_active_query_state(struct pipe_context *pctx, bool enable) assert_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->active_queries = enable;
   fd_context_dirty(ctx, FD_DIRTY_QUERY);
}

static enum pipe_driver_query_type
query_type(enum fd_perfcntr_type type)
{
#define ENUM(t)                                                                \
   case FD_PERFCNTR_##t:                                                       \
      return PIPE_DRIVER_QUERY_##t
   switch (type) {
      ENUM(TYPE_UINT64);
      ENUM(TYPE_UINT);
      ENUM(TYPE_FLOAT);
      ENUM(TYPE_PERCENTAGE);
      ENUM(TYPE_BYTES);
      ENUM(TYPE_MICROSECONDS);
      ENUM(TYPE_HZ);
      ENUM(TYPE_DBM);
      ENUM(TYPE_TEMPERATURE);
      ENUM(TYPE_VOLTS);
      ENUM(TYPE_AMPS);
      ENUM(TYPE_WATTS);
   default:
      unreachable("bad type");
      return 0;
   }
}

static enum pipe_driver_query_result_type
query_result_type(enum fd_perfcntr_result_type type)
{
   switch (type) {
      ENUM(RESULT_TYPE_AVERAGE);
      ENUM(RESULT_TYPE_CUMULATIVE);
   default:
      unreachable("bad type");
      return 0;
   }
}

static void
setup_perfcntr_query_info(struct fd_screen *screen)
{
   unsigned num_queries = 0;

   for (unsigned i = 0; i < screen->num_perfcntr_groups; i++)
      num_queries += screen->perfcntr_groups[i].num_countables;

   screen->perfcntr_queries =
      calloc(num_queries, sizeof(screen->perfcntr_queries[0]));
   screen->num_perfcntr_queries = num_queries;

   unsigned idx = 0;
   for (unsigned i = 0; i < screen->num_perfcntr_groups; i++) {
      const struct fd_perfcntr_group *g = &screen->perfcntr_groups[i];
      for (unsigned j = 0; j < g->num_countables; j++) {
         struct pipe_driver_query_info *info = &screen->perfcntr_queries[idx];
         const struct fd_perfcntr_countable *c = &g->countables[j];

         info->name = c->name;
         info->query_type = FD_QUERY_FIRST_PERFCNTR + idx;
         info->type = query_type(c->query_type);
         info->result_type = query_result_type(c->result_type);
         info->group_id = i;
         info->flags = PIPE_DRIVER_QUERY_FLAG_BATCH;

         idx++;
      }
   }
}

void
fd_query_screen_init(struct pipe_screen *pscreen)
{
   pscreen->get_driver_query_info = fd_get_driver_query_info;
   pscreen->get_driver_query_group_info = fd_get_driver_query_group_info;
   setup_perfcntr_query_info(fd_screen(pscreen));
}

void
fd_query_context_init(struct pipe_context *pctx)
{
   pctx->create_query = fd_create_query;
   pctx->destroy_query = fd_destroy_query;
   pctx->begin_query = fd_begin_query;
   pctx->end_query = fd_end_query;
   pctx->get_query_result = fd_get_query_result;
   pctx->get_query_result_resource  = fd_get_query_result_resource;
   pctx->set_active_query_state = fd_set_active_query_state;
   pctx->render_condition = fd_render_condition;
}
