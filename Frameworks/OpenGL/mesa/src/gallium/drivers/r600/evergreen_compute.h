/*
 * Copyright 2011 Adam Rak <adam.rak@streamnovation.com>
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
 *
 * Authors:
 *      Adam Rak <adam.rak@streamnovation.com>
 */

#ifndef EVERGREEN_COMPUTE_H
#define EVERGREEN_COMPUTE_H

#include "r600_pipe.h"

struct r600_atom;
struct evergreen_compute_resource;
struct compute_memory_item;

struct r600_resource_global {
	struct r600_resource base;
	struct compute_memory_item *chunk;
};

void evergreen_init_atom_start_compute_cs(struct r600_context *rctx);
void evergreen_init_compute_state_functions(struct r600_context *rctx);
void evergreen_emit_cs_shader(struct r600_context *rctx, struct r600_atom * atom);

struct r600_resource* r600_compute_buffer_alloc_vram(struct r600_screen *screen, unsigned size);
struct pipe_resource *r600_compute_global_buffer_create(struct pipe_screen *screen, const struct pipe_resource *templ);
void r600_compute_global_buffer_destroy(struct pipe_screen *screen,
					struct pipe_resource *res);
void *r600_compute_global_transfer_map(struct pipe_context *ctx,
				      struct pipe_resource *resource,
				      unsigned level,
				      unsigned usage,
				      const struct pipe_box *box,
				      struct pipe_transfer **ptransfer);
void r600_compute_global_transfer_unmap(struct pipe_context *ctx,
					struct pipe_transfer *transfer);

#endif
