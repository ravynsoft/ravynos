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

#ifndef ZINK_STATE_H
#define ZINK_STATE_H

#include "zink_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void
zink_context_state_init(struct pipe_context *pctx);


struct pipe_vertex_state *
zink_create_vertex_state(struct pipe_screen *pscreen,
                          struct pipe_vertex_buffer *buffer,
                          const struct pipe_vertex_element *elements,
                          unsigned num_elements,
                          struct pipe_resource *indexbuf,
                          uint32_t full_velem_mask);
void
zink_vertex_state_destroy(struct pipe_screen *pscreen, struct pipe_vertex_state *vstate);
struct pipe_vertex_state *
zink_cache_create_vertex_state(struct pipe_screen *pscreen,
                               struct pipe_vertex_buffer *buffer,
                               const struct pipe_vertex_element *elements,
                               unsigned num_elements,
                               struct pipe_resource *indexbuf,
                               uint32_t full_velem_mask);
void
zink_cache_vertex_state_destroy(struct pipe_screen *pscreen, struct pipe_vertex_state *vstate);


#ifdef __cplusplus
}
#endif

#endif
