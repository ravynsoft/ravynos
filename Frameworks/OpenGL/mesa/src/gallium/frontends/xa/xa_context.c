/**********************************************************
 * Copyright 2009-2011 VMware, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *********************************************************
 * Authors:
 * Zack Rusin <zackr-at-vmware-dot-com>
 * Thomas Hellstrom <thellstrom-at-vmware-dot-com>
 */
#include "xa_context.h"
#include "xa_priv.h"
#include "cso_cache/cso_context.h"
#include "util/u_inlines.h"
#include "util/u_rect.h"
#include "util/u_surface.h"
#include "pipe/p_context.h"

XA_EXPORT void
xa_context_flush(struct xa_context *ctx)
{
    if (ctx->last_fence) {
        struct pipe_screen *screen = ctx->xa->screen;
        screen->fence_reference(screen, &ctx->last_fence, NULL);
    }
    ctx->pipe->flush(ctx->pipe, &ctx->last_fence, 0);
}

XA_EXPORT struct xa_context *
xa_context_default(struct xa_tracker *xa)
{
    return xa->default_ctx;
}

XA_EXPORT struct xa_context *
xa_context_create(struct xa_tracker *xa)
{
    struct xa_context *ctx = calloc(1, sizeof(*ctx));

    ctx->xa = xa;
    ctx->pipe = xa->screen->context_create(xa->screen, NULL, 0);
    ctx->cso = cso_create_context(ctx->pipe, 0);
    ctx->shaders = xa_shaders_create(ctx);
    renderer_init_state(ctx);

    return ctx;
}

XA_EXPORT void
xa_context_destroy(struct xa_context *r)
{
    struct pipe_resource **vsbuf = &r->vs_const_buffer;
    struct pipe_resource **fsbuf = &r->fs_const_buffer;

    if (*vsbuf)
	pipe_resource_reference(vsbuf, NULL);

    if (*fsbuf)
	pipe_resource_reference(fsbuf, NULL);

    if (r->shaders) {
	xa_shaders_destroy(r->shaders);
	r->shaders = NULL;
    }

    xa_ctx_sampler_views_destroy(r);
    if (r->srf)
        pipe_surface_reference(&r->srf, NULL);

    if (r->cso) {
	cso_destroy_context(r->cso);
	r->cso = NULL;
    }

    r->pipe->destroy(r->pipe);
    free(r);
}

XA_EXPORT int
xa_surface_dma(struct xa_context *ctx,
	       struct xa_surface *srf,
	       void *data,
	       unsigned int pitch,
	       int to_surface, struct xa_box *boxes, unsigned int num_boxes)
{
    struct pipe_transfer *transfer;
    void *map;
    int w, h, i;
    enum pipe_map_flags transfer_direction;
    struct pipe_context *pipe = ctx->pipe;

    transfer_direction = (to_surface ? PIPE_MAP_WRITE :
			  PIPE_MAP_READ);

    for (i = 0; i < num_boxes; ++i, ++boxes) {
	w = boxes->x2 - boxes->x1;
	h = boxes->y2 - boxes->y1;

	map = pipe_texture_map(pipe, srf->tex, 0, 0,
                                transfer_direction, boxes->x1, boxes->y1,
                                w, h, &transfer);
	if (!map)
	    return -XA_ERR_NORES;

	if (to_surface) {
	    util_copy_rect(map, srf->tex->format, transfer->stride,
			   0, 0, w, h, data, pitch, boxes->x1, boxes->y1);
	} else {
	    util_copy_rect(data, srf->tex->format, pitch,
			   boxes->x1, boxes->y1, w, h, map, transfer->stride, 0,
			   0);
	}
	pipe->texture_unmap(pipe, transfer);
    }
    return XA_ERR_NONE;
}

XA_EXPORT void *
xa_surface_map(struct xa_context *ctx,
	       struct xa_surface *srf, unsigned int usage)
{
    void *map;
    unsigned int gallium_usage = 0;
    struct pipe_context *pipe = ctx->pipe;

    /*
     * A surface may only have a single map.
     */
    if (srf->transfer)
	return NULL;

    if (usage & XA_MAP_READ)
	gallium_usage |= PIPE_MAP_READ;
    if (usage & XA_MAP_WRITE)
	gallium_usage |= PIPE_MAP_WRITE;
    if (usage & XA_MAP_MAP_DIRECTLY)
	gallium_usage |= PIPE_MAP_DIRECTLY;
    if (usage & XA_MAP_UNSYNCHRONIZED)
	gallium_usage |= PIPE_MAP_UNSYNCHRONIZED;
    if (usage & XA_MAP_DONTBLOCK)
	gallium_usage |= PIPE_MAP_DONTBLOCK;
    if (usage & XA_MAP_DISCARD_WHOLE_RESOURCE)
	gallium_usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;

    if (!(gallium_usage & (PIPE_MAP_READ_WRITE)))
	return NULL;

    map = pipe_texture_map(pipe, srf->tex, 0, 0,
                            gallium_usage, 0, 0,
                            srf->tex->width0, srf->tex->height0,
                            &srf->transfer);
    if (!map)
	return NULL;

    srf->mapping_pipe = pipe;
    return map;
}

XA_EXPORT void
xa_surface_unmap(struct xa_surface *srf)
{
    if (srf->transfer) {
	struct pipe_context *pipe = srf->mapping_pipe;

	pipe->texture_unmap(pipe, srf->transfer);
	srf->transfer = NULL;
    }
}

int
xa_ctx_srf_create(struct xa_context *ctx, struct xa_surface *dst)
{
    struct pipe_screen *screen = ctx->pipe->screen;
    struct pipe_surface srf_templ;

    /*
     * Cache surfaces unless we change render target
     */
    if (ctx->srf) {
        if (ctx->srf->texture == dst->tex)
            return XA_ERR_NONE;

        pipe_surface_reference(&ctx->srf, NULL);
    }

    if (!screen->is_format_supported(screen,  dst->tex->format,
				     PIPE_TEXTURE_2D, 0, 0,
				     PIPE_BIND_RENDER_TARGET))
	return -XA_ERR_INVAL;

    u_surface_default_template(&srf_templ, dst->tex);
    ctx->srf = ctx->pipe->create_surface(ctx->pipe, dst->tex, &srf_templ);
    if (!ctx->srf)
	return -XA_ERR_NORES;

    return XA_ERR_NONE;
}

void
xa_ctx_srf_destroy(struct xa_context *ctx)
{
    /*
     * Cache surfaces unless we change render target.
     * Final destruction on context destroy.
     */
}

XA_EXPORT int
xa_copy_prepare(struct xa_context *ctx,
		struct xa_surface *dst, struct xa_surface *src)
{
    if (src == dst)
	return -XA_ERR_INVAL;

    if (src->tex->format != dst->tex->format) {
	int ret = xa_ctx_srf_create(ctx, dst);
	if (ret != XA_ERR_NONE)
	    return ret;
	renderer_copy_prepare(ctx, ctx->srf, src->tex,
			      src->fdesc.xa_format,
			      dst->fdesc.xa_format);
	ctx->simple_copy = 0;
    } else
	ctx->simple_copy = 1;

    ctx->src = src;
    ctx->dst = dst;
    xa_ctx_srf_destroy(ctx);

    return 0;
}

XA_EXPORT void
xa_copy(struct xa_context *ctx,
	int dx, int dy, int sx, int sy, int width, int height)
{
    struct pipe_box src_box;

    xa_scissor_update(ctx, dx, dy, dx + width, dy + height);

    if (ctx->simple_copy) {
	u_box_2d(sx, sy, width, height, &src_box);
	ctx->pipe->resource_copy_region(ctx->pipe,
					ctx->dst->tex, 0, dx, dy, 0,
					ctx->src->tex,
					0, &src_box);
    } else
	renderer_copy(ctx, dx, dy, sx, sy, width, height,
		      (float) ctx->src->tex->width0,
		      (float) ctx->src->tex->height0);
}

XA_EXPORT void
xa_copy_done(struct xa_context *ctx)
{
    if (!ctx->simple_copy) {
	renderer_draw_flush(ctx);
    }
}

static void
bind_solid_blend_state(struct xa_context *ctx)
{
    struct pipe_blend_state blend;

    memset(&blend, 0, sizeof(struct pipe_blend_state));
    blend.rt[0].blend_enable = 0;
    blend.rt[0].colormask = PIPE_MASK_RGBA;

    blend.rt[0].rgb_src_factor   = PIPE_BLENDFACTOR_ONE;
    blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
    blend.rt[0].rgb_dst_factor   = PIPE_BLENDFACTOR_ZERO;
    blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ZERO;

    cso_set_blend(ctx->cso, &blend);
}

XA_EXPORT int
xa_solid_prepare(struct xa_context *ctx, struct xa_surface *dst,
		 uint32_t fg)
{
    unsigned vs_traits, fs_traits;
    struct xa_shader shader;
    int ret;

    ret = xa_ctx_srf_create(ctx, dst);
    if (ret != XA_ERR_NONE)
	return ret;

    if (ctx->srf->format == PIPE_FORMAT_L8_UNORM)
	xa_pixel_to_float4_a8(fg, ctx->solid_color);
    else
	xa_pixel_to_float4(fg, ctx->solid_color);
    ctx->has_solid_src = 1;

    ctx->dst = dst;

#if 0
    debug_printf("Color Pixel=(%d, %d, %d, %d), RGBA=(%f, %f, %f, %f)\n",
		 (fg >> 24) & 0xff, (fg >> 16) & 0xff,
		 (fg >> 8) & 0xff,  (fg >> 0) & 0xff,
		 exa->solid_color[0], exa->solid_color[1],
		 exa->solid_color[2], exa->solid_color[3]);
#endif

    vs_traits = VS_SRC_SRC | VS_COMPOSITE;
    fs_traits = FS_SRC_SRC | VS_COMPOSITE;

    renderer_bind_destination(ctx, ctx->srf);
    bind_solid_blend_state(ctx);
    cso_set_samplers(ctx->cso, PIPE_SHADER_FRAGMENT, 0, NULL);
    ctx->pipe->set_sampler_views(ctx->pipe, PIPE_SHADER_FRAGMENT, 0, 0,
                                 XA_MAX_SAMPLERS, false, NULL);

    shader = xa_shaders_get(ctx->shaders, vs_traits, fs_traits);
    cso_set_vertex_shader_handle(ctx->cso, shader.vs);
    cso_set_fragment_shader_handle(ctx->cso, shader.fs);

    renderer_begin_solid(ctx);

    xa_ctx_srf_destroy(ctx);
    return XA_ERR_NONE;
}

XA_EXPORT void
xa_solid(struct xa_context *ctx, int x, int y, int width, int height)
{
    xa_scissor_update(ctx, x, y, x + width, y + height);
    renderer_solid(ctx, x, y, x + width, y + height);
}

XA_EXPORT void
xa_solid_done(struct xa_context *ctx)
{
    renderer_draw_flush(ctx);
    ctx->comp = NULL;
    ctx->has_solid_src = false;
    ctx->num_bound_samplers = 0;
}

XA_EXPORT struct xa_fence *
xa_fence_get(struct xa_context *ctx)
{
    struct xa_fence *fence = calloc(1, sizeof(*fence));
    struct pipe_screen *screen = ctx->xa->screen;

    if (!fence)
	return NULL;

    fence->xa = ctx->xa;

    if (ctx->last_fence == NULL)
	fence->pipe_fence = NULL;
    else
	screen->fence_reference(screen, &fence->pipe_fence, ctx->last_fence);

    return fence;
}

XA_EXPORT int
xa_fence_wait(struct xa_fence *fence, uint64_t timeout)
{
    if (!fence)
	return XA_ERR_NONE;

    if (fence->pipe_fence) {
	struct pipe_screen *screen = fence->xa->screen;
	bool timed_out;

	timed_out = !screen->fence_finish(screen, NULL, fence->pipe_fence, timeout);
	if (timed_out)
	    return -XA_ERR_BUSY;

	screen->fence_reference(screen, &fence->pipe_fence, NULL);
    }
    return XA_ERR_NONE;
}

XA_EXPORT void
xa_fence_destroy(struct xa_fence *fence)
{
    if (!fence)
	return;

    if (fence->pipe_fence) {
	struct pipe_screen *screen = fence->xa->screen;

	screen->fence_reference(screen, &fence->pipe_fence, NULL);
    }

    free(fence);
}

void
xa_ctx_sampler_views_destroy(struct xa_context *ctx)
{
    int i;

    for (i = 0; i < ctx->num_bound_samplers; ++i)
	pipe_sampler_view_reference(&ctx->bound_sampler_views[i], NULL);
    ctx->num_bound_samplers = 0;
}
