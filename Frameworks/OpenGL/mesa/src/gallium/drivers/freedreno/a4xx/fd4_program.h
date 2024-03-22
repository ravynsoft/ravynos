/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD4_PROGRAM_H_
#define FD4_PROGRAM_H_

#include "pipe/p_context.h"
#include "freedreno_context.h"

#include "ir3/ir3_cache.h"
#include "ir3/ir3_shader.h"

struct fd4_emit;

struct fd4_program_state {
   struct ir3_program_state base;
   const struct ir3_shader_variant *bs; /* VS for when emit->binning */
   const struct ir3_shader_variant *vs;
   const struct ir3_shader_variant *fs; /* FS for when !emit->binning */
};

static inline struct fd4_program_state *
fd4_program_state(struct ir3_program_state *state)
{
   return (struct fd4_program_state *)state;
}

void fd4_emit_shader(struct fd_ringbuffer *ring,
                     const struct ir3_shader_variant *so);

void fd4_program_emit(struct fd_ringbuffer *ring, struct fd4_emit *emit, int nr,
                      struct pipe_surface **bufs);

void fd4_prog_init(struct pipe_context *pctx);

#endif /* FD4_PROGRAM_H_ */
