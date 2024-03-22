/*
 * Copyright 2021 Advanced Micro Devices, Inc.
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

/* This deduplicates pipe_vertex_state CSOs to enable draw merging in
 * u_threaded_context because the draw merging is possible only if different
 * display lists use the same pipe_vertex_state CSO.
 */

#ifndef U_VERTEX_STATE_CACHE_H
#define U_VERTEX_STATE_CACHE_H

#include "util/simple_mtx.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"

struct util_vertex_state_cache {
   simple_mtx_t lock;
   struct set *set;

   pipe_create_vertex_state_func create;
   pipe_vertex_state_destroy_func destroy;
};

void
util_vertex_state_cache_init(struct util_vertex_state_cache *cache,
                             pipe_create_vertex_state_func create,
                             pipe_vertex_state_destroy_func destroy);

void
util_vertex_state_cache_deinit(struct util_vertex_state_cache *cache);

struct pipe_vertex_state *
util_vertex_state_cache_get(struct pipe_screen *screen,
                            struct pipe_vertex_buffer *buffer,
                            const struct pipe_vertex_element *elements,
                            unsigned num_elements,
                            struct pipe_resource *indexbuf,
                            uint32_t full_velem_mask,
                            struct util_vertex_state_cache *cache);

void
util_vertex_state_destroy(struct pipe_screen *screen,
                          struct util_vertex_state_cache *cache,
                          struct pipe_vertex_state *state);

#endif
