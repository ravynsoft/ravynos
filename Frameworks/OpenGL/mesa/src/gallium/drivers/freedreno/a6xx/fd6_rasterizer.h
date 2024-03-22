/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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

#ifndef FD6_RASTERIZER_H_
#define FD6_RASTERIZER_H_

#include "pipe/p_context.h"
#include "pipe/p_state.h"

#include "freedreno_context.h"

struct fd6_rasterizer_stateobj {
   struct pipe_rasterizer_state base;

   struct fd_ringbuffer *stateobjs[2];
};

static inline struct fd6_rasterizer_stateobj *
fd6_rasterizer_stateobj(struct pipe_rasterizer_state *rast)
{
   return (struct fd6_rasterizer_stateobj *)rast;
}

void *fd6_rasterizer_state_create(struct pipe_context *pctx,
                                  const struct pipe_rasterizer_state *cso);
void fd6_rasterizer_state_delete(struct pipe_context *, void *hwcso);

template <chip CHIP>
struct fd_ringbuffer *
__fd6_setup_rasterizer_stateobj(struct fd_context *ctx,
                                const struct pipe_rasterizer_state *cso,
                                bool primitive_restart);

template <chip CHIP>
static inline struct fd_ringbuffer *
fd6_rasterizer_state(struct fd_context *ctx, bool primitive_restart) assert_dt
{
   struct fd6_rasterizer_stateobj *rasterizer =
      fd6_rasterizer_stateobj(ctx->rasterizer);
   unsigned variant = primitive_restart;

   if (unlikely(!rasterizer->stateobjs[variant])) {
      rasterizer->stateobjs[variant] = __fd6_setup_rasterizer_stateobj<CHIP>(
         ctx, ctx->rasterizer, primitive_restart);
   }

   return rasterizer->stateobjs[variant];
}

#endif /* FD6_RASTERIZER_H_ */
