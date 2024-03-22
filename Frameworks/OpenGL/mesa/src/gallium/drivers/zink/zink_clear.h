/*
 * Copyright © 2020 Mike Blumenkrantz
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 * 
 * Authors:
 *    Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 */

#include "util/u_rect.h"
#include "zink_types.h"
#include "zink_screen.h"

void
zink_clear(struct pipe_context *pctx,
           unsigned buffers,
           const struct pipe_scissor_state *scissor_state,
           const union pipe_color_union *pcolor,
           double depth, unsigned stencil);
void
zink_clear_texture(struct pipe_context *ctx,
                   struct pipe_resource *p_res,
                   unsigned level,
                   const struct pipe_box *box,
                   const void *data);
void
zink_clear_texture_dynamic(struct pipe_context *ctx,
                           struct pipe_resource *p_res,
                           unsigned level,
                           const struct pipe_box *box,
                           const void *data);
void
zink_clear_buffer(struct pipe_context *pctx,
                  struct pipe_resource *pres,
                  unsigned offset,
                  unsigned size,
                  const void *clear_value,
                  int clear_value_size);

void
zink_clear_render_target(struct pipe_context *ctx, struct pipe_surface *dst,
                         const union pipe_color_union *color, unsigned dstx,
                         unsigned dsty, unsigned width, unsigned height,
                         bool render_condition_enabled);

void
zink_clear_depth_stencil(struct pipe_context *ctx, struct pipe_surface *dst,
                         unsigned clear_flags, double depth, unsigned stencil,
                         unsigned dstx, unsigned dsty, unsigned width, unsigned height,
                         bool render_condition_enabled);

bool
zink_fb_clear_needs_explicit(struct zink_framebuffer_clear *fb_clear);

bool
zink_fb_clear_first_needs_explicit(struct zink_framebuffer_clear *fb_clear);

void
zink_clear_framebuffer(struct zink_context *ctx, unsigned clear_buffers);

static inline struct zink_framebuffer_clear_data *
zink_fb_clear_element(struct zink_framebuffer_clear *fb_clear, int idx)
{
   return util_dynarray_element(&fb_clear->clears, struct zink_framebuffer_clear_data, idx);
}

static inline unsigned
zink_fb_clear_count(struct zink_framebuffer_clear *fb_clear)
{
   return fb_clear ? util_dynarray_num_elements(&fb_clear->clears, struct zink_framebuffer_clear_data) : 0;
}

void
zink_fb_clear_reset(struct zink_context *ctx, unsigned idx);

static inline bool
zink_fb_clear_element_needs_explicit(struct zink_framebuffer_clear_data *clear)
{
   return clear->has_scissor || clear->conditional;
}

static inline bool
zink_fb_clear_full_exists(struct zink_context *ctx, unsigned clear_buffer)
{
   struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[clear_buffer];
   return zink_fb_clear_count(fb_clear) && !zink_fb_clear_first_needs_explicit(fb_clear);
}

void
zink_clear_apply_conditionals(struct zink_context *ctx);

void
zink_fb_clears_apply(struct zink_context *ctx, struct pipe_resource *pres);

void
zink_fb_clears_discard(struct zink_context *ctx, struct pipe_resource *pres);

void
zink_fb_clears_apply_or_discard(struct zink_context *ctx, struct pipe_resource *pres, struct u_rect region, bool discard_only);

void
zink_fb_clears_apply_region(struct zink_context *ctx, struct pipe_resource *pres, struct u_rect region);

void
zink_fb_clear_rewrite(struct zink_context *ctx, unsigned idx, enum pipe_format before, enum pipe_format after);
