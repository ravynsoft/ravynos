/*
 * Copyright 2018 Collabora Ltd.
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

#ifndef ZINK_BATCH_H
#define ZINK_BATCH_H

#include <vulkan/vulkan_core.h>
#include "zink_types.h"

#include "util/list.h"
#include "util/set.h"
#include "util/u_dynarray.h"

#include "zink_fence.h"

#ifdef __cplusplus
extern "C" {
#endif


void
zink_reset_batch_state(struct zink_context *ctx, struct zink_batch_state *bs);

void
zink_clear_batch_state(struct zink_context *ctx, struct zink_batch_state *bs);

void
zink_batch_reset_all(struct zink_context *ctx);

void
zink_batch_state_destroy(struct zink_screen *screen, struct zink_batch_state *bs);

void
zink_batch_state_clear_resources(struct zink_screen *screen, struct zink_batch_state *bs);

void
zink_reset_batch(struct zink_context *ctx, struct zink_batch *batch);
void
zink_start_batch(struct zink_context *ctx, struct zink_batch *batch);

void
zink_end_batch(struct zink_context *ctx, struct zink_batch *batch);

void
zink_batch_add_wait_semaphore(struct zink_batch *batch, VkSemaphore sem);

void
zink_batch_reference_resource_rw(struct zink_batch *batch,
                                 struct zink_resource *res,
                                 bool write);
void
zink_batch_reference_resource(struct zink_batch *batch, struct zink_resource *res);

bool
zink_batch_reference_resource_move(struct zink_batch *batch, struct zink_resource *res);

void
zink_batch_reference_program(struct zink_batch *batch,
                             struct zink_program *pg);

void
zink_batch_bind_db(struct zink_context *ctx);
void
debug_describe_zink_batch_state(char *buf, const struct zink_batch_state *ptr);

static ALWAYS_INLINE bool
zink_batch_usage_is_unflushed(const struct zink_batch_usage *u)
{
   return u && u->unflushed;
}

static ALWAYS_INLINE void
zink_batch_usage_unset(struct zink_batch_usage **u, struct zink_batch_state *bs)
{
   (void)p_atomic_cmpxchg((uintptr_t *)u, (uintptr_t)&bs->usage, (uintptr_t)NULL);
}

static ALWAYS_INLINE void
zink_batch_usage_set(struct zink_batch_usage **u, struct zink_batch_state *bs)
{
   *u = &bs->usage;
}

static ALWAYS_INLINE bool
zink_batch_usage_matches(const struct zink_batch_usage *u, const struct zink_batch_state *bs)
{
   return u == &bs->usage;
}

static ALWAYS_INLINE bool
zink_batch_usage_exists(const struct zink_batch_usage *u)
{
   return u && (u->usage || u->unflushed);
}

bool
zink_screen_usage_check_completion(struct zink_screen *screen, const struct zink_batch_usage *u);
bool
zink_screen_usage_check_completion_fast(struct zink_screen *screen, const struct zink_batch_usage *u);

bool
zink_batch_usage_check_completion(struct zink_context *ctx, const struct zink_batch_usage *u);

void
zink_batch_usage_wait(struct zink_context *ctx, struct zink_batch_usage *u);

void
zink_batch_usage_try_wait(struct zink_context *ctx, struct zink_batch_usage *u);

#ifdef __cplusplus
}
#endif

#endif
