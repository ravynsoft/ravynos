/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#ifndef FREEDRENO_STATE_H_
#define FREEDRENO_STATE_H_

#include "pipe/p_context.h"
#include "freedreno_context.h"

BEGINC;

static inline bool
fd_depth_enabled(struct fd_context *ctx) assert_dt
{
   return ctx->zsa && ctx->zsa->depth_enabled;
}

static inline bool
fd_depth_write_enabled(struct fd_context *ctx) assert_dt
{
   return ctx->zsa && ctx->zsa->depth_writemask;
}

static inline bool
fd_stencil_enabled(struct fd_context *ctx) assert_dt
{
   return ctx->zsa && ctx->zsa->stencil[0].enabled;
}

static inline bool
fd_blend_enabled(struct fd_context *ctx, unsigned n) assert_dt
{
   return ctx->blend && ctx->blend->rt[n].blend_enable;
}

static inline bool
fd_depth_clamp_enabled(struct fd_context *ctx) assert_dt
{
   return !(ctx->rasterizer->depth_clip_near &&
            ctx->rasterizer->depth_clip_far);
}

void fd_set_shader_buffers(struct pipe_context *pctx,
                           enum pipe_shader_type shader,
                           unsigned start, unsigned count,
                           const struct pipe_shader_buffer *buffers,
                           unsigned writable_bitmask) in_dt;

void fd_set_shader_images(struct pipe_context *pctx,
                          enum pipe_shader_type shader, unsigned start,
                          unsigned count, unsigned unbind_num_trailing_slots,
                          const struct pipe_image_view *images) in_dt;

void fd_set_framebuffer_state(struct pipe_context *pctx,
                         const struct pipe_framebuffer_state *framebuffer) in_dt;

void fd_state_init(struct pipe_context *pctx);

ENDC;

#endif /* FREEDRENO_STATE_H_ */
