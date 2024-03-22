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

#ifndef FD2_BLEND_H_
#define FD2_BLEND_H_

#include "pipe/p_context.h"
#include "pipe/p_state.h"

struct fd2_blend_stateobj {
   struct pipe_blend_state base;
   uint32_t rb_blendcontrol;
   uint32_t rb_colorcontrol; /* must be OR'd w/ zsa->rb_colorcontrol */
   uint32_t rb_colormask;
};

static inline struct fd2_blend_stateobj *
fd2_blend_stateobj(struct pipe_blend_state *blend)
{
   return (struct fd2_blend_stateobj *)blend;
}

void *fd2_blend_state_create(struct pipe_context *pctx,
                             const struct pipe_blend_state *cso);

#endif /* FD2_BLEND_H_ */
