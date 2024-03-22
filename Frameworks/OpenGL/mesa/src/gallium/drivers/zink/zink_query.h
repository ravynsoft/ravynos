/*
 * Copyright 2019 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ZINK_QUERY_H
#define ZINK_QUERY_H

#include <stdbool.h>
#include <inttypes.h>
#include "zink_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void
zink_suspend_queries(struct zink_context *ctx, struct zink_batch *batch);

void
zink_resume_queries(struct zink_context *ctx, struct zink_batch *batch);

void
zink_query_renderpass_suspend(struct zink_context *ctx);

void
zink_resume_cs_query(struct zink_context *ctx);

void
zink_prune_query(struct zink_batch_state *bs, struct zink_query *query);
void
zink_query_sync(struct zink_context *ctx, struct zink_query *query);

void
zink_query_update_gs_states(struct zink_context *ctx);

void
zink_start_conditional_render(struct zink_context *ctx);

void
zink_stop_conditional_render(struct zink_context *ctx);

void
zink_context_destroy_query_pools(struct zink_context *ctx);
uint64_t
zink_get_timestamp(struct pipe_screen *pscreen);
#ifdef __cplusplus
}
#endif

#endif
