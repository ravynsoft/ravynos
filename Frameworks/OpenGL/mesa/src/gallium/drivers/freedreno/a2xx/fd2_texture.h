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

#ifndef FD2_TEXTURE_H_
#define FD2_TEXTURE_H_

#include "pipe/p_context.h"

#include "freedreno_resource.h"
#include "freedreno_texture.h"

#include "fd2_context.h"
#include "fd2_util.h"

struct fd2_sampler_stateobj {
   struct pipe_sampler_state base;
   uint32_t tex0, tex3, tex4;
};

static inline struct fd2_sampler_stateobj *
fd2_sampler_stateobj(struct pipe_sampler_state *samp)
{
   return (struct fd2_sampler_stateobj *)samp;
}

struct fd2_pipe_sampler_view {
   struct pipe_sampler_view base;
   uint32_t tex0, tex1, tex2, tex3, tex4, tex5;
};

static inline struct fd2_pipe_sampler_view *
fd2_pipe_sampler_view(struct pipe_sampler_view *pview)
{
   return (struct fd2_pipe_sampler_view *)pview;
}

unsigned fd2_get_const_idx(struct fd_context *ctx,
                           struct fd_texture_stateobj *tex, unsigned samp_id);

void fd2_texture_init(struct pipe_context *pctx);

#endif /* FD2_TEXTURE_H_ */
