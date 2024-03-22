/*
 * Copyright (c) 2016 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "util/os_time.h"
#include "pipe/p_state.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "etnaviv_context.h"
#include "etnaviv_query_sw.h"

static void
etna_sw_destroy_query(struct etna_context *ctx, struct etna_query *q)
{
   struct etna_sw_query *sq = etna_sw_query(q);

   FREE(sq);
}

static uint64_t
read_counter(struct etna_context *ctx, unsigned type)
{
   switch (type) {
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      return ctx->stats.prims_generated;
   case ETNA_QUERY_DRAW_CALLS:
      return ctx->stats.draw_calls;
   case ETNA_QUERY_RS_OPERATIONS:
      return ctx->stats.rs_operations;
   }

   return 0;
}

static void
etna_sw_begin_query(struct etna_context *ctx, struct etna_query *q)
{
   struct etna_sw_query *sq = etna_sw_query(q);

   sq->begin_value = read_counter(ctx, q->type);
}

static void
etna_sw_end_query(struct etna_context *ctx, struct etna_query *q)
{
   struct etna_sw_query *sq = etna_sw_query(q);

   sq->end_value = read_counter(ctx, q->type);
}

static bool
etna_sw_get_query_result(struct etna_context *ctx, struct etna_query *q,
                         bool wait, union pipe_query_result *result)
{
   struct etna_sw_query *sq = etna_sw_query(q);

   result->u64 = sq->end_value - sq->begin_value;

   return true;
}

static const struct etna_query_funcs sw_query_funcs = {
   .destroy_query = etna_sw_destroy_query,
   .begin_query = etna_sw_begin_query,
   .end_query = etna_sw_end_query,
   .get_query_result = etna_sw_get_query_result,
};

struct etna_query *
etna_sw_create_query(struct etna_context *ctx, unsigned query_type)
{
   struct etna_sw_query *sq;
   struct etna_query *q;

   switch (query_type) {
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case ETNA_QUERY_DRAW_CALLS:
   case ETNA_QUERY_RS_OPERATIONS:
      break;
   default:
      return NULL;
   }

   sq = CALLOC_STRUCT(etna_sw_query);
   if (!sq)
      return NULL;

   q = &sq->base;
   q->funcs = &sw_query_funcs;
   q->type = query_type;

   return q;
}

static const struct pipe_driver_query_info list[] = {
   {"prims-generated", PIPE_QUERY_PRIMITIVES_GENERATED, { 0 }},
   {"draw-calls", ETNA_QUERY_DRAW_CALLS, { 0 }},
   {"rs-operations", ETNA_QUERY_RS_OPERATIONS, { 0 }},
};

int
etna_sw_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                              struct pipe_driver_query_info *info)
{
   if (!info)
      return ARRAY_SIZE(list);

   if (index >= ARRAY_SIZE(list))
      return 0;

   *info = list[index];

   return 1;
}

int
etna_sw_get_driver_query_group_info(struct pipe_screen *pscreen,
                                    unsigned index,
                                    struct pipe_driver_query_group_info *info)
{
   if (!info)
      return 1;

   if (index != 0)
      return 0;

   info->name = "driver";
   info->max_active_queries = ARRAY_SIZE(list);
   info->num_queries = ARRAY_SIZE(list);

   return 1;
}
