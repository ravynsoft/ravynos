/*
 * Copyright (C) 2014 Broadcom
 * Copyright (C) 2019 Collabora, Ltd.
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
 * Authors (Collabora):
 *   Tomeu Vizoso <tomeu.vizoso@collabora.com>
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 *
 */

#include "util/format/u_format.h"
#include "pan_context.h"
#include "pan_resource.h"
#include "pan_util.h"

void
panfrost_blitter_save(struct panfrost_context *ctx,
                      const enum panfrost_blitter_op blitter_op)
{
   struct blitter_context *blitter = ctx->blitter;

   util_blitter_save_vertex_buffer_slot(blitter, ctx->vertex_buffers);
   util_blitter_save_vertex_elements(blitter, ctx->vertex);
   util_blitter_save_vertex_shader(blitter,
                                   ctx->uncompiled[PIPE_SHADER_VERTEX]);
   util_blitter_save_rasterizer(blitter, ctx->rasterizer);
   util_blitter_save_viewport(blitter, &ctx->pipe_viewport);
   util_blitter_save_so_targets(blitter, 0, NULL);

   if (blitter_op & PAN_SAVE_FRAGMENT_STATE) {
      if (blitter_op & PAN_SAVE_FRAGMENT_CONSTANT)
         util_blitter_save_fragment_constant_buffer_slot(
            blitter, ctx->constant_buffer[PIPE_SHADER_FRAGMENT].cb);

      util_blitter_save_blend(blitter, ctx->blend);
      util_blitter_save_depth_stencil_alpha(blitter, ctx->depth_stencil);
      util_blitter_save_stencil_ref(blitter, &ctx->stencil_ref);
      util_blitter_save_fragment_shader(blitter,
                                        ctx->uncompiled[PIPE_SHADER_FRAGMENT]);
      util_blitter_save_sample_mask(blitter, ctx->sample_mask,
                                    ctx->min_samples);
      util_blitter_save_scissor(blitter, &ctx->scissor);
   }

   if (blitter_op & PAN_SAVE_FRAMEBUFFER)
      util_blitter_save_framebuffer(blitter, &ctx->pipe_framebuffer);

   if (blitter_op & PAN_SAVE_TEXTURES) {
      util_blitter_save_fragment_sampler_states(
         blitter, ctx->sampler_count[PIPE_SHADER_FRAGMENT],
         (void **)(&ctx->samplers[PIPE_SHADER_FRAGMENT]));
      util_blitter_save_fragment_sampler_views(
         blitter, ctx->sampler_view_count[PIPE_SHADER_FRAGMENT],
         (struct pipe_sampler_view **)&ctx->sampler_views[PIPE_SHADER_FRAGMENT]);
   }

   if (!(blitter_op & PAN_DISABLE_RENDER_COND)) {
      util_blitter_save_render_condition(blitter,
                                         (struct pipe_query *)ctx->cond_query,
                                         ctx->cond_cond, ctx->cond_mode);
   }
}

void
panfrost_blit_no_afbc_legalization(struct pipe_context *pipe,
                                   const struct pipe_blit_info *info)
{
   struct panfrost_context *ctx = pan_context(pipe);

   panfrost_blitter_save(ctx, info->render_condition_enable
                                 ? PAN_RENDER_BLIT_COND
                                 : PAN_RENDER_BLIT);
   util_blitter_blit(ctx->blitter, info);
}

void
panfrost_blit(struct pipe_context *pipe, const struct pipe_blit_info *info)
{
   struct panfrost_context *ctx = pan_context(pipe);

   if (info->render_condition_enable && !panfrost_render_condition_check(ctx))
      return;

   if (!util_blitter_is_blit_supported(ctx->blitter, info))
      unreachable("Unsupported blit\n");

   /* Legalize here because it could trigger a recursive blit otherwise */
   struct panfrost_resource *src = pan_resource(info->src.resource);
   enum pipe_format src_view_format = util_format_linear(info->src.format);
   pan_legalize_afbc_format(ctx, src, src_view_format, false, false);

   struct panfrost_resource *dst = pan_resource(info->dst.resource);
   enum pipe_format dst_view_format = util_format_linear(info->dst.format);
   pan_legalize_afbc_format(ctx, dst, dst_view_format, true, false);

   panfrost_blit_no_afbc_legalization(pipe, info);
}
