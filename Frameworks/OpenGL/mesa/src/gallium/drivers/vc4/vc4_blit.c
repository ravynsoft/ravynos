/*
 * Copyright © 2015 Broadcom
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
 */

#include "nir/pipe_nir.h"
#include "util/format/u_format.h"
#include "util/u_surface.h"
#include "util/u_blitter.h"
#include "compiler/nir/nir_builder.h"
#include "vc4_context.h"

static struct pipe_surface *
vc4_get_blit_surface(struct pipe_context *pctx,
                     struct pipe_resource *prsc, unsigned level,
                     unsigned layer)
{
        struct pipe_surface tmpl;

        memset(&tmpl, 0, sizeof(tmpl));
        tmpl.format = prsc->format;
        tmpl.u.tex.level = level;
        tmpl.u.tex.first_layer = tmpl.u.tex.last_layer = layer;

        return pctx->create_surface(pctx, prsc, &tmpl);
}

static bool
is_tile_unaligned(unsigned size, unsigned tile_size)
{
        return size & (tile_size - 1);
}

static void
vc4_tile_blit(struct pipe_context *pctx, struct pipe_blit_info *info)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        bool msaa = (info->src.resource->nr_samples > 1 ||
                     info->dst.resource->nr_samples > 1);
        int tile_width = msaa ? 32 : 64;
        int tile_height = msaa ? 32 : 64;

        if (!info->mask)
                return;

        bool is_color_blit = info->mask & PIPE_MASK_RGBA;
        bool is_depth_blit = info->mask & PIPE_MASK_Z;
        bool is_stencil_blit = info->mask & PIPE_MASK_S;

        /* Either we receive a depth/stencil blit, or color blit, but not both.
         */
        assert ((is_color_blit && !(is_depth_blit || is_stencil_blit)) ||
                (!is_color_blit && (is_depth_blit || is_stencil_blit)));

        if (info->scissor_enable)
                return;

        if (info->dst.box.x != info->src.box.x ||
            info->dst.box.y != info->src.box.y ||
            info->dst.box.width != info->src.box.width ||
            info->dst.box.height != info->src.box.height ||
            info->dst.box.depth != info->src.box.depth ||
            info->dst.box.depth != 1) {
                return;
        }

        if (is_color_blit &&
            util_format_is_depth_or_stencil(info->dst.format))
                return;

        if ((is_depth_blit || is_stencil_blit) &&
            !util_format_is_depth_or_stencil(info->dst.format))
                return;

        int dst_surface_width = u_minify(info->dst.resource->width0,
                                         info->dst.level);
        int dst_surface_height = u_minify(info->dst.resource->height0,
                                         info->dst.level);
        if (is_tile_unaligned(info->dst.box.x, tile_width) ||
            is_tile_unaligned(info->dst.box.y, tile_height) ||
            (is_tile_unaligned(info->dst.box.width, tile_width) &&
             info->dst.box.x + info->dst.box.width != dst_surface_width) ||
            (is_tile_unaligned(info->dst.box.height, tile_height) &&
             info->dst.box.y + info->dst.box.height != dst_surface_height)) {
                return;
        }

        /* VC4_PACKET_LOAD_TILE_BUFFER_GENERAL uses the
         * VC4_PACKET_TILE_RENDERING_MODE_CONFIG's width (determined by our
         * destination surface) to determine the stride.  This may be wrong
         * when reading from texture miplevels > 0, which are stored in
         * POT-sized areas.  For MSAA, the tile addresses are computed
         * explicitly by the RCL, but still use the destination width to
         * determine the stride (which could be fixed by explicitly supplying
         * it in the ABI).
         */
        struct vc4_resource *rsc = vc4_resource(info->src.resource);

        uint32_t stride;

        if (info->src.resource->nr_samples > 1)
                stride = align(dst_surface_width, 32) * 4 * rsc->cpp;
        else if (rsc->slices[info->src.level].tiling == VC4_TILING_FORMAT_T)
                stride = align(dst_surface_width * rsc->cpp, 128);
        else
                stride = align(dst_surface_width * rsc->cpp, 16);

        if (stride != rsc->slices[info->src.level].stride)
                return;

        if (info->dst.resource->format != info->src.resource->format)
                return;

        if (false) {
                fprintf(stderr, "RCL blit from %d,%d to %d,%d (%d,%d)\n",
                        info->src.box.x,
                        info->src.box.y,
                        info->dst.box.x,
                        info->dst.box.y,
                        info->dst.box.width,
                        info->dst.box.height);
        }

        struct pipe_surface *dst_surf =
                vc4_get_blit_surface(pctx, info->dst.resource, info->dst.level,
                                           info->dst.box.z);
        struct pipe_surface *src_surf =
                vc4_get_blit_surface(pctx, info->src.resource, info->src.level,
                                           info->src.box.z);

        vc4_flush_jobs_reading_resource(vc4, info->src.resource);

        struct vc4_job *job;
        if (is_color_blit) {
                job = vc4_get_job(vc4, dst_surf, NULL);
                pipe_surface_reference(&job->color_read, src_surf);
        } else {
                job = vc4_get_job(vc4, NULL, dst_surf);
                pipe_surface_reference(&job->zs_read, src_surf);
        }

        job->draw_min_x = info->dst.box.x;
        job->draw_min_y = info->dst.box.y;
        job->draw_max_x = info->dst.box.x + info->dst.box.width;
        job->draw_max_y = info->dst.box.y + info->dst.box.height;
        job->draw_width = dst_surf->width;
        job->draw_height = dst_surf->height;

        job->tile_width = tile_width;
        job->tile_height = tile_height;
        job->msaa = msaa;
        job->needs_flush = true;

        if (is_color_blit) {
                job->resolve |= PIPE_CLEAR_COLOR;
                info->mask &= ~PIPE_MASK_RGBA;
        }

        if (is_depth_blit) {
                job->resolve |= PIPE_CLEAR_DEPTH;
                info->mask &= ~PIPE_MASK_Z;
        }

        if (is_stencil_blit) {
                job->resolve |= PIPE_CLEAR_STENCIL;
                info->mask &= ~PIPE_MASK_S;
        }

        vc4_job_submit(vc4, job);

        pipe_surface_reference(&dst_surf, NULL);
        pipe_surface_reference(&src_surf, NULL);
}

void
vc4_blitter_save(struct vc4_context *vc4)
{
        util_blitter_save_fragment_constant_buffer_slot(vc4->blitter,
                                                        vc4->constbuf[PIPE_SHADER_FRAGMENT].cb);
        util_blitter_save_vertex_buffer_slot(vc4->blitter, vc4->vertexbuf.vb);
        util_blitter_save_vertex_elements(vc4->blitter, vc4->vtx);
        util_blitter_save_vertex_shader(vc4->blitter, vc4->prog.bind_vs);
        util_blitter_save_rasterizer(vc4->blitter, vc4->rasterizer);
        util_blitter_save_viewport(vc4->blitter, &vc4->viewport);
        util_blitter_save_scissor(vc4->blitter, &vc4->scissor);
        util_blitter_save_fragment_shader(vc4->blitter, vc4->prog.bind_fs);
        util_blitter_save_blend(vc4->blitter, vc4->blend);
        util_blitter_save_depth_stencil_alpha(vc4->blitter, vc4->zsa);
        util_blitter_save_stencil_ref(vc4->blitter, &vc4->stencil_ref);
        util_blitter_save_sample_mask(vc4->blitter, vc4->sample_mask, 0);
        util_blitter_save_framebuffer(vc4->blitter, &vc4->framebuffer);
        util_blitter_save_fragment_sampler_states(vc4->blitter,
                        vc4->fragtex.num_samplers,
                        (void **)vc4->fragtex.samplers);
        util_blitter_save_fragment_sampler_views(vc4->blitter,
                        vc4->fragtex.num_textures, vc4->fragtex.textures);
}

static void *vc4_get_yuv_vs(struct pipe_context *pctx)
{
   struct vc4_context *vc4 = vc4_context(pctx);
   struct pipe_screen *pscreen = pctx->screen;

   if (vc4->yuv_linear_blit_vs)
           return vc4->yuv_linear_blit_vs;

   const struct nir_shader_compiler_options *options =
           pscreen->get_compiler_options(pscreen,
                                         PIPE_SHADER_IR_NIR,
                                         PIPE_SHADER_VERTEX);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_VERTEX, options,
                                                  "linear_blit_vs");

   const struct glsl_type *vec4 = glsl_vec4_type();
   nir_variable *pos_in = nir_variable_create(b.shader, nir_var_shader_in,
                                              vec4, "pos");

   nir_variable *pos_out = nir_variable_create(b.shader, nir_var_shader_out,
                                               vec4, "gl_Position");
   pos_out->data.location = VARYING_SLOT_POS;

   nir_store_var(&b, pos_out, nir_load_var(&b, pos_in), 0xf);

   vc4->yuv_linear_blit_vs = pipe_shader_from_nir(pctx, b.shader);

   return vc4->yuv_linear_blit_vs;
}

static void *vc4_get_yuv_fs(struct pipe_context *pctx, int cpp)
{
   struct vc4_context *vc4 = vc4_context(pctx);
   struct pipe_screen *pscreen = pctx->screen;
   struct pipe_shader_state **cached_shader;
   const char *name;

   if (cpp == 1) {
           cached_shader = &vc4->yuv_linear_blit_fs_8bit;
           name = "linear_blit_8bit_fs";
   } else {
           cached_shader = &vc4->yuv_linear_blit_fs_16bit;
           name = "linear_blit_16bit_fs";
   }

   if (*cached_shader)
           return *cached_shader;

   const struct nir_shader_compiler_options *options =
           pscreen->get_compiler_options(pscreen,
                                         PIPE_SHADER_IR_NIR,
                                         PIPE_SHADER_FRAGMENT);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,
                                                  options, "%s", name);

   const struct glsl_type *vec4 = glsl_vec4_type();
   const struct glsl_type *glsl_int = glsl_int_type();

   nir_variable *color_out = nir_variable_create(b.shader, nir_var_shader_out,
                                                 vec4, "f_color");
   color_out->data.location = FRAG_RESULT_COLOR;

   nir_variable *pos_in = nir_variable_create(b.shader, nir_var_shader_in,
                                              vec4, "pos");
   pos_in->data.location = VARYING_SLOT_POS;
   nir_def *pos = nir_load_var(&b, pos_in);

   nir_def *one = nir_imm_int(&b, 1);
   nir_def *two = nir_imm_int(&b, 2);

   nir_def *x = nir_f2i32(&b, nir_channel(&b, pos, 0));
   nir_def *y = nir_f2i32(&b, nir_channel(&b, pos, 1));

   nir_variable *stride_in = nir_variable_create(b.shader, nir_var_uniform,
                                                 glsl_int, "stride");
   nir_def *stride = nir_load_var(&b, stride_in);

   nir_def *x_offset;
   nir_def *y_offset;
   if (cpp == 1) {
           nir_def *intra_utile_x_offset =
                   nir_ishl(&b, nir_iand(&b, x, one), two);
           nir_def *inter_utile_x_offset =
                   nir_ishl(&b, nir_iand(&b, x, nir_imm_int(&b, ~3)), one);

           x_offset = nir_iadd(&b,
                               intra_utile_x_offset,
                               inter_utile_x_offset);
           y_offset = nir_imul(&b,
                               nir_iadd(&b,
                                        nir_ishl(&b, y, one),
                                        nir_ushr(&b, nir_iand(&b, x, two), one)),
                               stride);
   } else {
           x_offset = nir_ishl(&b, x, two);
           y_offset = nir_imul(&b, y, stride);
   }

   nir_def *load =
      nir_load_ubo(&b, 1, 32, one, nir_iadd(&b, x_offset, y_offset),
                   .align_mul = 4,
                   .align_offset = 0,
                   .range_base = 0,
                   .range = ~0);

   nir_store_var(&b, color_out,
                 nir_unpack_unorm_4x8(&b, load),
                 0xf);

   *cached_shader = pipe_shader_from_nir(pctx, b.shader);

   return *cached_shader;
}

static void
vc4_yuv_blit(struct pipe_context *pctx, struct pipe_blit_info *info)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_resource *src = vc4_resource(info->src.resource);
        struct vc4_resource *dst = vc4_resource(info->dst.resource);
        bool ok;

        if (!(info->mask & PIPE_MASK_RGBA))
                return;

        if (src->tiled)
                return;

        if (src->base.format != PIPE_FORMAT_R8_UNORM &&
            src->base.format != PIPE_FORMAT_R8G8_UNORM)
                return;

        /* YUV blits always turn raster-order to tiled */
        assert(dst->base.format == src->base.format);
        assert(dst->tiled);

        /* Always 1:1 and at the origin */
        assert(info->src.box.x == 0 && info->dst.box.x == 0);
        assert(info->src.box.y == 0 && info->dst.box.y == 0);
        assert(info->src.box.width == info->dst.box.width);
        assert(info->src.box.height == info->dst.box.height);

        if ((src->slices[info->src.level].offset & 3) ||
            (src->slices[info->src.level].stride & 3)) {
                perf_debug("YUV-blit src texture offset/stride misaligned: 0x%08x/%d\n",
                           src->slices[info->src.level].offset,
                           src->slices[info->src.level].stride);
                goto fallback;
        }

        vc4_blitter_save(vc4);

        /* Create a renderable surface mapping the T-tiled shadow buffer.
         */
        struct pipe_surface dst_tmpl;
        util_blitter_default_dst_texture(&dst_tmpl, info->dst.resource,
                                         info->dst.level, info->dst.box.z);
        dst_tmpl.format = PIPE_FORMAT_RGBA8888_UNORM;
        struct pipe_surface *dst_surf =
                pctx->create_surface(pctx, info->dst.resource, &dst_tmpl);
        if (!dst_surf) {
                fprintf(stderr, "Failed to create YUV dst surface\n");
                util_blitter_unset_running_flag(vc4->blitter);
                return;
        }
        dst_surf->width = align(dst_surf->width, 8) / 2;
        if (dst->cpp == 1)
                dst_surf->height /= 2;

        /* Set the constant buffer. */
        uint32_t stride = src->slices[info->src.level].stride;
        struct pipe_constant_buffer cb_uniforms = {
                .user_buffer = &stride,
                .buffer_size = sizeof(stride),
        };
        pctx->set_constant_buffer(pctx, PIPE_SHADER_FRAGMENT, 0, false, &cb_uniforms);
        struct pipe_constant_buffer cb_src = {
                .buffer = info->src.resource,
                .buffer_offset = src->slices[info->src.level].offset,
                .buffer_size = (src->bo->size -
                                src->slices[info->src.level].offset),
        };
        pctx->set_constant_buffer(pctx, PIPE_SHADER_FRAGMENT, 1, false, &cb_src);

        /* Unbind the textures, to make sure we don't try to recurse into the
         * shadow blit.
         */
        pctx->set_sampler_views(pctx, PIPE_SHADER_FRAGMENT, 0, 0, 0, false, NULL);
        pctx->bind_sampler_states(pctx, PIPE_SHADER_FRAGMENT, 0, 0, NULL);

        util_blitter_custom_shader(vc4->blitter, dst_surf,
                                   vc4_get_yuv_vs(pctx),
                                   vc4_get_yuv_fs(pctx, src->cpp));

        util_blitter_restore_textures(vc4->blitter);
        util_blitter_restore_constant_buffer_state(vc4->blitter);
        /* Restore cb1 (util_blitter doesn't handle this one). */
        struct pipe_constant_buffer cb_disabled = { 0 };
        pctx->set_constant_buffer(pctx, PIPE_SHADER_FRAGMENT, 1, false, &cb_disabled);

        pipe_surface_reference(&dst_surf, NULL);

        info->mask &= ~PIPE_MASK_RGBA;

        return;

fallback:
        /* Do an immediate SW fallback, since the render blit path
         * would just recurse.
         */
        ok = util_try_blit_via_copy_region(pctx, info, false);
        assert(ok); (void)ok;

        info->mask &= ~PIPE_MASK_RGBA;
}

static void
vc4_render_blit(struct pipe_context *ctx, struct pipe_blit_info *info)
{
        struct vc4_context *vc4 = vc4_context(ctx);

        if (!info->mask)
                return;

        if (!util_blitter_is_blit_supported(vc4->blitter, info)) {
                fprintf(stderr, "blit unsupported %s -> %s\n",
                    util_format_short_name(info->src.resource->format),
                    util_format_short_name(info->dst.resource->format));
                return;
        }

        /* Enable the scissor, so we get a minimal set of tiles rendered. */
        if (!info->scissor_enable) {
                info->scissor_enable = true;
                info->scissor.minx = info->dst.box.x;
                info->scissor.miny = info->dst.box.y;
                info->scissor.maxx = info->dst.box.x + info->dst.box.width;
                info->scissor.maxy = info->dst.box.y + info->dst.box.height;
        }

        vc4_blitter_save(vc4);
        util_blitter_blit(vc4->blitter, info);

        info->mask = 0;
}

/* Implement stencil and stencil/depth blit by reinterpreting stencil data as
 * an RGBA8888 texture.
 */
static void
vc4_stencil_blit(struct pipe_context *ctx, struct pipe_blit_info *info)
{
        struct vc4_context *vc4 = vc4_context(ctx);
        struct vc4_resource *src = vc4_resource(info->src.resource);
        struct vc4_resource *dst = vc4_resource(info->dst.resource);
        enum pipe_format src_format, dst_format;

        if ((info->mask & PIPE_MASK_S) == 0)
                return;

        src_format = (info->mask & PIPE_MASK_ZS) ?
                     PIPE_FORMAT_RGBA8888_UINT :
                     PIPE_FORMAT_R8_UINT;

        dst_format = (info->mask & PIPE_MASK_ZS) ?
                     PIPE_FORMAT_RGBA8888_UINT :
                     PIPE_FORMAT_R8_UINT;

        /* Initialize the surface */
        struct pipe_surface dst_tmpl = {
                .u.tex = {
                        .level = info->dst.level,
                        .first_layer = info->dst.box.z,
                        .last_layer = info->dst.box.z,
                },
                .format = dst_format,
        };
        struct pipe_surface *dst_surf =
                ctx->create_surface(ctx, &dst->base, &dst_tmpl);

        /* Initialize the sampler view */
        struct pipe_sampler_view src_tmpl = {
                .target = (src->base.target == PIPE_TEXTURE_CUBE_ARRAY) ?
                          PIPE_TEXTURE_2D_ARRAY :
                          src->base.target,
                .format = src_format,
                .u.tex =  {
                        .first_level = info->src.level,
                        .last_level = info->src.level,
                        .first_layer = 0,
                        .last_layer = (PIPE_TEXTURE_2D ?
                                       u_minify(src->base.depth0,
                                                info->src.level) - 1 :
                                       src->base.array_size - 1),
                },
                .swizzle_r = PIPE_SWIZZLE_X,
                .swizzle_g = PIPE_SWIZZLE_Y,
                .swizzle_b = PIPE_SWIZZLE_Z,
                .swizzle_a = PIPE_SWIZZLE_W,
        };
        struct pipe_sampler_view *src_view =
                ctx->create_sampler_view(ctx, &src->base, &src_tmpl);

        vc4_blitter_save(vc4);
        util_blitter_blit_generic(vc4->blitter, dst_surf, &info->dst.box,
                                  src_view, &info->src.box,
                                  src->base.width0, src->base.height0,
                                  (info->mask & PIPE_MASK_ZS) ?
                                  PIPE_MASK_RGBA : PIPE_MASK_R,
                                  PIPE_TEX_FILTER_NEAREST,
                                  info->scissor_enable ? &info->scissor :  NULL,
                                  info->alpha_blend, false, 0);

        pipe_surface_reference(&dst_surf, NULL);
        pipe_sampler_view_reference(&src_view, NULL);

        info->mask &= ~PIPE_MASK_ZS;
}

/* Optimal hardware path for blitting pixels.
 * Scaling, format conversion, up- and downsampling (resolve) are allowed.
 */
void
vc4_blit(struct pipe_context *pctx, const struct pipe_blit_info *blit_info)
{
        struct pipe_blit_info info = *blit_info;

        vc4_yuv_blit(pctx, &info);

        vc4_tile_blit(pctx, &info);

        if (info.mask &&
            util_try_blit_via_copy_region(pctx, &info, false))
                return;

        vc4_stencil_blit(pctx, &info);

        vc4_render_blit(pctx, &info);

        if (info.mask)
                fprintf(stderr, "Unsupported blit\n");
}
