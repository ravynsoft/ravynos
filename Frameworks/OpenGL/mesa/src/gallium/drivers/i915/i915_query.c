/**************************************************************************
 *
 * Copyright 2011 The Chromium OS authors.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL GOOGLE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/* Fake occlusion queries which return 0, it's better than crashing */

#include "util/compiler.h"

#include "util/u_memory.h"

#include "i915_context.h"
#include "i915_query.h"

struct i915_query {
   unsigned query;
};

static struct pipe_query *
i915_create_query(struct pipe_context *ctx, unsigned query_type, unsigned index)
{
   struct i915_query *query = CALLOC_STRUCT(i915_query);

   return (struct pipe_query *)query;
}

static void
i915_destroy_query(struct pipe_context *ctx, struct pipe_query *query)
{
   FREE(query);
}

static bool
i915_begin_query(struct pipe_context *ctx, struct pipe_query *query)
{
   return true;
}

static bool
i915_end_query(struct pipe_context *ctx, struct pipe_query *query)
{
   return true;
}

static bool
i915_get_query_result(struct pipe_context *ctx, struct pipe_query *query,
                      bool wait, union pipe_query_result *vresult)
{
   uint64_t *result = (uint64_t *)vresult;

   /* 2* viewport Max */
   *result = 512 * 1024 * 1024;
   return true;
}

static void
i915_set_active_query_state(struct pipe_context *pipe, bool enable)
{
}

void
i915_init_query_functions(struct i915_context *i915)
{
   i915->base.create_query = i915_create_query;
   i915->base.destroy_query = i915_destroy_query;
   i915->base.begin_query = i915_begin_query;
   i915->base.end_query = i915_end_query;
   i915->base.get_query_result = i915_get_query_result;
   i915->base.set_active_query_state = i915_set_active_query_state;
}
