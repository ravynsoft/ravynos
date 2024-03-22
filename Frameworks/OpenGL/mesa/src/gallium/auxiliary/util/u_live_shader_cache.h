/*
 * Copyright 2019 Advanced Micro Devices, Inc.
 * All Rights Reserved.
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

/* This deduplicates live shader CSOs, meaning that creating 2 shaders with
 * the same IR will return the same CSO.
 *
 * How to use this:
 *
 * - create_xx_state should only call util_live_shader_cache_get.
 *
 * - delete_xx_state should only call util_shader_reference(&shader, NULL).
 *   This will decrease the reference count.
 *
 * - Driver shaders must inherit util_live_shader. They don't have to
 *   initialize it.
 *
 * - Declare struct util_live_shader_cache in your pipe_screen (no pointer) if
 *   you support shareable shaders. If not, declare it in your pipe_context.
 *
 * - Set your create_shader and destroy_shader driver callbacks with
 *   util_live_shader_cache_init. These are your driver versions of
 *   create_xx_state and delete_xx_state. There is no distinction between
 *   vs, tcs, tes, gs, fs. Instead, get the shader type from the IR.
 *
 * - Call util_live_shader_cache_deinit when you destroy your screen or context.
 */

#ifndef U_LIVE_SHADER_CACHE_H
#define U_LIVE_SHADER_CACHE_H

#include "util/simple_mtx.h"
#include "pipe/p_state.h"

#ifdef __cplusplus
extern "C" {
#endif

struct util_live_shader_cache {
   simple_mtx_t lock;
   struct hash_table *hashtable;

   void *(*create_shader)(struct pipe_context *,
                          const struct pipe_shader_state *state);
   void (*destroy_shader)(struct pipe_context *, void *);

   unsigned hits, misses;
};

struct util_live_shader {
   struct pipe_reference reference;
   unsigned char sha1[20];
};

void
util_live_shader_cache_init(struct util_live_shader_cache *cache,
                              void *(*create_shader)(struct pipe_context *,
                                                     const struct pipe_shader_state *state),
                              void (*destroy_shader)(struct pipe_context *, void *));

void
util_live_shader_cache_deinit(struct util_live_shader_cache *cache);

void *
util_live_shader_cache_get(struct pipe_context *ctx,
                           struct util_live_shader_cache *cache,
                           const struct pipe_shader_state *state,
                           bool* cache_hit);

void
util_shader_reference(struct pipe_context *ctx,
                      struct util_live_shader_cache *cache,
                      void **dst, void *src);

#ifdef __cplusplus
}
#endif

#endif
