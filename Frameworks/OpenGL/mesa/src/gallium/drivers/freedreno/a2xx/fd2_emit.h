/*
 * Copyright (C) 2012-2013 Rob Clark <robclark@freedesktop.org>
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

#ifndef FD2_EMIT_H
#define FD2_EMIT_H

#include "pipe/p_context.h"

#include "freedreno_context.h"

struct fd_ringbuffer;

struct fd2_vertex_buf {
   unsigned offset, size;
   struct pipe_resource *prsc;
};

void fd2_emit_vertex_bufs(struct fd_ringbuffer *ring, uint32_t val,
                          struct fd2_vertex_buf *vbufs, uint32_t n);
void fd2_emit_state_binning(struct fd_context *ctx,
                            const enum fd_dirty_3d_state dirty) assert_dt;
void fd2_emit_state(struct fd_context *ctx,
                    const enum fd_dirty_3d_state dirty) assert_dt;
void fd2_emit_restore(struct fd_context *ctx, struct fd_ringbuffer *ring);

void fd2_emit_init_screen(struct pipe_screen *pscreen);
void fd2_emit_init(struct pipe_context *pctx);

static inline void
fd2_emit_ib(struct fd_ringbuffer *ring, struct fd_ringbuffer *target)
{
   __OUT_IB(ring, false, target);
}

#endif /* FD2_EMIT_H */
