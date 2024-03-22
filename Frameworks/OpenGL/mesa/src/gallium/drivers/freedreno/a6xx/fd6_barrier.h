/*
 * Copyright © 2023 Google, Inc.
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
 */

#ifndef FD6_BARRIER_H_
#define FD6_BARRIER_H_

#include "freedreno_context.h"

/**
 * Various flush operations that could be needed
 */
enum fd6_flush {
   FD6_FLUSH_CCU_COLOR      = BIT(0),
   FD6_FLUSH_CCU_DEPTH      = BIT(1),
   FD6_INVALIDATE_CCU_COLOR = BIT(2),
   FD6_INVALIDATE_CCU_DEPTH = BIT(3),
   FD6_FLUSH_CACHE          = BIT(4),
   FD6_INVALIDATE_CACHE     = BIT(5),
   FD6_WAIT_MEM_WRITES      = BIT(6),
   FD6_WAIT_FOR_IDLE        = BIT(7),
   FD6_WAIT_FOR_ME          = BIT(8),
};

void fd6_emit_flushes(struct fd_context *ctx, struct fd_ringbuffer *ring,
                      unsigned flushes);

void fd6_barrier_flush(struct fd_batch *batch) assert_dt;

void fd6_barrier_init(struct pipe_context *pctx);

#endif /* FD6_BARRIER_H_ */
