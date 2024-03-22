/*
 * Copyright (c) 2017-2019 Lima Project
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
 */

/**
 * Stub support for occlusion queries.
 *
 * Since we expose support for GL 2.0, we have to expose occlusion queries,
 * but the spec allows you to expose 0 query counter bits, so we just return 0
 * as the result of all our queries.
 */

#include "util/u_debug.h"

#include "lima_context.h"

struct lima_query
{
   uint8_t pad;
};

static struct pipe_query *
lima_create_query(struct pipe_context *ctx, unsigned query_type, unsigned index)
{
   struct lima_query *query = calloc(1, sizeof(*query));

   /* Note that struct pipe_query isn't actually defined anywhere. */
   return (struct pipe_query *)query;
}

static void
lima_destroy_query(struct pipe_context *ctx, struct pipe_query *query)
{
   free(query);
}

static bool
lima_begin_query(struct pipe_context *ctx, struct pipe_query *query)
{
   return true;
}

static bool
lima_end_query(struct pipe_context *ctx, struct pipe_query *query)
{
   return true;
}

static bool
lima_get_query_result(struct pipe_context *ctx, struct pipe_query *query,
                     bool wait, union pipe_query_result *vresult)
{
   uint64_t *result = &vresult->u64;

   *result = 0;

   return true;
}

static void
lima_set_active_query_state(struct pipe_context *pipe, bool enable)
{

}

void
lima_query_init(struct lima_context *pctx)
{
   pctx->base.create_query = lima_create_query;
   pctx->base.destroy_query = lima_destroy_query;
   pctx->base.begin_query = lima_begin_query;
   pctx->base.end_query = lima_end_query;
   pctx->base.get_query_result = lima_get_query_result;
   pctx->base.set_active_query_state = lima_set_active_query_state;
}

