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

#ifndef H_ETNAVIV_QUERY
#define H_ETNAVIV_QUERY

#include "pipe/p_context.h"

struct etna_context;
struct etna_query;

struct etna_query_funcs {
   void (*destroy_query)(struct etna_context *ctx, struct etna_query *q);
   void (*begin_query)(struct etna_context *ctx, struct etna_query *q);
   void (*end_query)(struct etna_context *ctx, struct etna_query *q);
   bool (*get_query_result)(struct etna_context *ctx, struct etna_query *q,
                            bool wait, union pipe_query_result *result);
};

struct etna_query {
   const struct etna_query_funcs *funcs;
   unsigned type;
};

static inline struct etna_query *
etna_query(struct pipe_query *pq)
{
   return (struct etna_query *)pq;
}

#define ETNA_SW_QUERY_BASE       (PIPE_QUERY_DRIVER_SPECIFIC + 0)
#define ETNA_PM_QUERY_BASE       (PIPE_QUERY_DRIVER_SPECIFIC + 32)

void
etna_query_screen_init(struct pipe_screen *pscreen);

void
etna_query_context_init(struct pipe_context *pctx);

#endif
