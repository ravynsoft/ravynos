/*
 * Copyright (C) 2017 Rob Clark <robclark@freedesktop.org>
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

#include "util/u_blitter.h"
#include "util/u_surface.h"

#include "freedreno_blitter.h"
#include "freedreno_context.h"
#include "freedreno_fence.h"
#include "freedreno_resource.h"

/* generic blit using u_blitter.. slightly modified version of util_blitter_blit
 * which also handles PIPE_BUFFER:
 */

static void
default_dst_texture(struct pipe_surface *dst_templ, struct pipe_resource *dst,
                    unsigned dstlevel, unsigned dstz)
{
   memset(dst_templ, 0, sizeof(*dst_templ));
   dst_templ->u.tex.level = dstlevel;
   dst_templ->u.tex.first_layer = dstz;
   dst_templ->u.tex.last_layer = dstz;
}

static void
default_src_texture(struct pipe_sampler_view *src_templ,
                    struct pipe_resource *src, unsigned srclevel)
{
   bool cube_as_2darray =
      src->screen->get_param(src->screen, PIPE_CAP_SAMPLER_VIEW_TARGET);

   memset(src_templ, 0, sizeof(*src_templ));

   if (cube_as_2darray && (src->target == PIPE_TEXTURE_CUBE ||
                           src->target == PIPE_TEXTURE_CUBE_ARRAY))
      src_templ->target = PIPE_TEXTURE_2D_ARRAY;
   else
      src_templ->target = src->target;

   if (src->target == PIPE_BUFFER) {
      src_templ->target = PIPE_TEXTURE_1D;
   }
   src_templ->u.tex.first_level = srclevel;
   src_templ->u.tex.last_level = srclevel;
   src_templ->u.tex.first_layer = 0;
   src_templ->u.tex.last_layer = src->target == PIPE_TEXTURE_3D
                                    ? u_minify(src->depth0, srclevel) - 1
                                    : (unsigned)(src->array_size - 1);
   src_templ->swizzle_r = PIPE_SWIZZLE_X;
   src_templ->swizzle_g = PIPE_SWIZZLE_Y;
   src_templ->swizzle_b = PIPE_SWIZZLE_Z;
   src_templ->swizzle_a = PIPE_SWIZZLE_W;
}

static void
fd_blitter_pipe_begin(struct fd_context *ctx, bool render_cond) assert_dt
{
   util_blitter_save_vertex_buffer_slot(ctx->blitter, ctx->vtx.vertexbuf.vb);
   util_blitter_save_vertex_elements(ctx->blitter, ctx->vtx.vtx);
   util_blitter_save_vertex_shader(ctx->blitter, ctx->prog.vs);
   util_blitter_save_tessctrl_shader(ctx->blitter, ctx->prog.hs);
   util_blitter_save_tesseval_shader(ctx->blitter, ctx->prog.ds);
   util_blitter_save_geometry_shader(ctx->blitter, ctx->prog.gs);
   util_blitter_save_so_targets(ctx->blitter, ctx->streamout.num_targets,
                                ctx->streamout.targets);
   util_blitter_save_rasterizer(ctx->blitter, ctx->rasterizer);
   util_blitter_save_viewport(ctx->blitter, &ctx->viewport[0]);
   util_blitter_save_scissor(ctx->blitter, &ctx->scissor[0]);
   util_blitter_save_fragment_shader(ctx->blitter, ctx->prog.fs);
   util_blitter_save_blend(ctx->blitter, ctx->blend);
   util_blitter_save_depth_stencil_alpha(ctx->blitter, ctx->zsa);
   util_blitter_save_stencil_ref(ctx->blitter, &ctx->stencil_ref);
   util_blitter_save_sample_mask(ctx->blitter, ctx->sample_mask, ctx->min_samples);
   util_blitter_save_framebuffer(ctx->blitter, &ctx->framebuffer);
   util_blitter_save_fragment_sampler_states(
      ctx->blitter, ctx->tex[PIPE_SHADER_FRAGMENT].num_samplers,
      (void **)ctx->tex[PIPE_SHADER_FRAGMENT].samplers);
   util_blitter_save_fragment_sampler_views(
      ctx->blitter, ctx->tex[PIPE_SHADER_FRAGMENT].num_textures,
      ctx->tex[PIPE_SHADER_FRAGMENT].textures);
   if (!render_cond)
      util_blitter_save_render_condition(ctx->blitter, ctx->cond_query,
                                         ctx->cond_cond, ctx->cond_mode);

   if (ctx->batch)
      fd_batch_update_queries(ctx->batch);
}

static void
fd_blitter_pipe_end(struct fd_context *ctx) assert_dt
{
}

bool
fd_blitter_blit(struct fd_context *ctx, const struct pipe_blit_info *info)
{
   struct pipe_context *pctx = &ctx->base;
   struct pipe_resource *dst = info->dst.resource;
   struct pipe_resource *src = info->src.resource;
   struct pipe_context *pipe = &ctx->base;
   struct pipe_surface *dst_view, dst_templ;
   struct pipe_sampler_view src_templ, *src_view;

   /* If the blit is updating the whole contents of the resource,
    * invalidate it so we don't trigger any unnecessary tile loads in the 3D
    * path.
    */
   if (util_blit_covers_whole_resource(info))
      pctx->invalidate_resource(pctx, info->dst.resource);

   /* The blit format may not match the resource format in this path, so
    * we need to validate that we can use the src/dst resource with the
    * requested format (and uncompress if necessary).  Normally this would
    * happen in ->set_sampler_view(), ->set_framebuffer_state(), etc.  But
    * that would cause recursion back into u_blitter, which ends in tears.
    *
    * To avoid recursion, this needs to be done before util_blitter_save_*()
    */
   if (ctx->validate_format) {
      ctx->validate_format(ctx, fd_resource(dst), info->dst.format);
      ctx->validate_format(ctx, fd_resource(src), info->src.format);
   }

   if (src == dst)
      pipe->flush(pipe, NULL, 0);

   DBG_BLIT(info, NULL);

   fd_blitter_pipe_begin(ctx, info->render_condition_enable);

   /* Initialize the surface. */
   default_dst_texture(&dst_templ, dst, info->dst.level, info->dst.box.z);
   dst_templ.format = info->dst.format;
   dst_view = pipe->create_surface(pipe, dst, &dst_templ);

   /* Initialize the sampler view. */
   default_src_texture(&src_templ, src, info->src.level);
   src_templ.format = info->src.format;
   src_view = pipe->create_sampler_view(pipe, src, &src_templ);

   /* Copy. */
   util_blitter_blit_generic(
      ctx->blitter, dst_view, &info->dst.box, src_view, &info->src.box,
      src->width0, src->height0, info->mask, info->filter,
      info->scissor_enable ? &info->scissor : NULL, info->alpha_blend, false, 0);

   pipe_surface_reference(&dst_view, NULL);
   pipe_sampler_view_reference(&src_view, NULL);

   fd_blitter_pipe_end(ctx);

   /* While this shouldn't technically be necessary, it is required for
    * dEQP-GLES31.functional.stencil_texturing.format.stencil_index8_cube and
    * 2d_array to pass.
    */
   fd_bc_flush_writer(ctx, fd_resource(info->dst.resource));

   /* The fallback blitter must never fail: */
   return true;
}

/* Generic clear implementation (partially) using u_blitter: */
void
fd_blitter_clear(struct pipe_context *pctx, unsigned buffers,
                 const union pipe_color_union *color, double depth,
                 unsigned stencil)
{
   struct fd_context *ctx = fd_context(pctx);
   struct pipe_framebuffer_state *pfb = &ctx->batch->framebuffer;
   struct blitter_context *blitter = ctx->blitter;

   /* Note: don't use discard=true, if there was something to
    * discard, that would have been already handled in fd_clear().
    */
   fd_blitter_pipe_begin(ctx, false);

   util_blitter_save_fragment_constant_buffer_slot(
      ctx->blitter, ctx->constbuf[PIPE_SHADER_FRAGMENT].cb);

   util_blitter_common_clear_setup(blitter, pfb->width, pfb->height, buffers,
                                   NULL, NULL);

   struct pipe_stencil_ref sr = {.ref_value = {stencil & 0xff}};
   pctx->set_stencil_ref(pctx, sr);

   struct pipe_constant_buffer cb = {
      .buffer_size = 16,
      .user_buffer = &color->ui,
   };
   pctx->set_constant_buffer(pctx, PIPE_SHADER_FRAGMENT, 0, false, &cb);

   unsigned rs_idx = pfb->samples > 1 ? 1 : 0;
   if (!ctx->clear_rs_state[rs_idx]) {
      const struct pipe_rasterizer_state tmpl = {
         .cull_face = PIPE_FACE_NONE,
         .half_pixel_center = 1,
         .bottom_edge_rule = 1,
         .flatshade = 1,
         .depth_clip_near = 1,
         .depth_clip_far = 1,
         .multisample = pfb->samples > 1,
      };
      ctx->clear_rs_state[rs_idx] = pctx->create_rasterizer_state(pctx, &tmpl);
   }
   pctx->bind_rasterizer_state(pctx, ctx->clear_rs_state[rs_idx]);

   struct pipe_viewport_state vp = {
      .scale = {0.5f * pfb->width, -0.5f * pfb->height, depth},
      .translate = {0.5f * pfb->width, 0.5f * pfb->height, 0.0f},
   };
   pctx->set_viewport_states(pctx, 0, 1, &vp);

   pctx->bind_vertex_elements_state(pctx, ctx->solid_vbuf_state.vtx);
   pctx->set_vertex_buffers(pctx, 1, 0, false,
                            &ctx->solid_vbuf_state.vertexbuf.vb[0]);
   pctx->set_stream_output_targets(pctx, 0, NULL, NULL);

   if (pfb->layers > 1)
      pctx->bind_vs_state(pctx, ctx->solid_layered_prog.vs);
   else
      pctx->bind_vs_state(pctx, ctx->solid_prog.vs);

   pctx->bind_fs_state(pctx, ctx->solid_prog.fs);

   /* Clear geom/tess shaders, lest the draw emit code think we are
    * trying to use use them:
    */
   pctx->bind_gs_state(pctx, NULL);
   pctx->bind_tcs_state(pctx, NULL);
   pctx->bind_tes_state(pctx, NULL);

   struct pipe_draw_info info = {
      .mode = MESA_PRIM_COUNT, /* maps to DI_PT_RECTLIST */
      .index_bounds_valid = true,
      .max_index = 1,
      .instance_count = MAX2(1, pfb->layers),
   };
   struct pipe_draw_start_count_bias draw = {
      .count = 2,
   };

   pctx->draw_vbo(pctx, &info, 0, NULL, &draw, 1);

   /* We expect that this should not have triggered a change in pfb: */
   assert(util_framebuffer_state_equal(pfb, &ctx->framebuffer));

   util_blitter_restore_constant_buffer_state(blitter);
   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_textures(blitter);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);

   fd_blitter_pipe_end(ctx);
}

/* Partially generic clear_render_target implementation using u_blitter */
void
fd_blitter_clear_render_target(struct pipe_context *pctx, struct pipe_surface *ps,
                               const union pipe_color_union *color, unsigned x,
                               unsigned y, unsigned w, unsigned h,
                               bool render_condition_enabled)
{
   struct fd_context *ctx = fd_context(pctx);

   fd_blitter_pipe_begin(ctx, render_condition_enabled);
   util_blitter_clear_render_target(ctx->blitter, ps, color, x, y, w, h);
   fd_blitter_pipe_end(ctx);
}

/* Partially generic clear_depth_stencil implementation using u_blitter */
void
fd_blitter_clear_depth_stencil(struct pipe_context *pctx, struct pipe_surface *ps,
                               unsigned buffers, double depth, unsigned stencil,
                               unsigned x, unsigned y, unsigned w, unsigned h,
                               bool render_condition_enabled)
{
   struct fd_context *ctx = fd_context(pctx);

   fd_blitter_pipe_begin(ctx, render_condition_enabled);
   util_blitter_clear_depth_stencil(ctx->blitter, ps, buffers, depth,
                                    stencil, x, y, w, h);
   fd_blitter_pipe_end(ctx);
}

/**
 * Optimal hardware path for blitting pixels.
 * Scaling, format conversion, up- and downsampling (resolve) are allowed.
 */
bool
fd_blit(struct pipe_context *pctx, const struct pipe_blit_info *blit_info)
{
   struct fd_context *ctx = fd_context(pctx);
   struct pipe_blit_info info = *blit_info;

   if (info.render_condition_enable && !fd_render_condition_check(pctx))
      return true;

   if (ctx->blit && ctx->blit(ctx, &info))
      return true;

   if (info.mask & PIPE_MASK_S) {
      DBG("cannot blit stencil, skipping");
      info.mask &= ~PIPE_MASK_S;
   }

   if (!util_blitter_is_blit_supported(ctx->blitter, &info)) {
      DBG("blit unsupported %s -> %s",
          util_format_short_name(info.src.resource->format),
          util_format_short_name(info.dst.resource->format));
      return false;
   }

   return fd_blitter_blit(ctx, &info);
}

/**
 * _copy_region using pipe (3d engine)
 */
static bool
fd_blitter_pipe_copy_region(struct fd_context *ctx, struct pipe_resource *dst,
                            unsigned dst_level, unsigned dstx, unsigned dsty,
                            unsigned dstz, struct pipe_resource *src,
                            unsigned src_level,
                            const struct pipe_box *src_box) assert_dt
{
   /* not until we allow rendertargets to be buffers */
   if (dst->target == PIPE_BUFFER || src->target == PIPE_BUFFER)
      return false;

   if (!util_blitter_is_copy_supported(ctx->blitter, dst, src))
      return false;

   if (src == dst) {
      struct pipe_context *pctx = &ctx->base;
      pctx->flush(pctx, NULL, 0);
   }

   /* TODO we could invalidate if dst box covers dst level fully. */
   fd_blitter_pipe_begin(ctx, false);
   util_blitter_copy_texture(ctx->blitter, dst, dst_level, dstx, dsty, dstz,
                             src, src_level, src_box);
   fd_blitter_pipe_end(ctx);

   return true;
}

/**
 * Copy a block of pixels from one resource to another.
 * The resource must be of the same format.
 */
void
fd_resource_copy_region(struct pipe_context *pctx, struct pipe_resource *dst,
                        unsigned dst_level, unsigned dstx, unsigned dsty,
                        unsigned dstz, struct pipe_resource *src,
                        unsigned src_level, const struct pipe_box *src_box)
{
   struct fd_context *ctx = fd_context(pctx);

   /* The blitter path handles compressed formats only if src and dst format
    * match, in other cases just fall back to sw:
    */
   if ((src->format != dst->format) &&
       (util_format_is_compressed(src->format) ||
        util_format_is_compressed(dst->format))) {
      perf_debug_ctx(ctx, "copy_region falls back to sw for {%"PRSC_FMT"} to {%"PRSC_FMT"}",
                     PRSC_ARGS(src), PRSC_ARGS(dst));
      goto fallback;
   }

   if (ctx->blit) {
      struct pipe_blit_info info;

      memset(&info, 0, sizeof info);
      info.dst.resource = dst;
      info.dst.level = dst_level;
      info.dst.box.x = dstx;
      info.dst.box.y = dsty;
      info.dst.box.z = dstz;
      info.dst.box.width = src_box->width;
      info.dst.box.height = src_box->height;
      assert(info.dst.box.width >= 0);
      assert(info.dst.box.height >= 0);
      info.dst.box.depth = 1;
      info.dst.format = dst->format;
      info.src.resource = src;
      info.src.level = src_level;
      info.src.box = *src_box;
      info.src.format = src->format;
      info.mask = util_format_get_mask(src->format);
      info.filter = PIPE_TEX_FILTER_NEAREST;
      info.scissor_enable = 0;

      if (ctx->blit(ctx, &info))
         return;
   }

   /* try blit on 3d pipe: */
   if (fd_blitter_pipe_copy_region(ctx, dst, dst_level, dstx, dsty, dstz, src,
                                   src_level, src_box))
      return;

   /* else fallback to pure sw: */
fallback:
   util_resource_copy_region(pctx, dst, dst_level, dstx, dsty, dstz, src,
                             src_level, src_box);
}
