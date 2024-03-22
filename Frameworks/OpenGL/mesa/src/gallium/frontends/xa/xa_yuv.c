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
#include "util/u_inlines.h"
#include "util/u_sampler.h"
#include "util/u_surface.h"
#include "cso_cache/cso_context.h"

static void
xa_yuv_bind_blend_state(struct xa_context *r)
{
    struct pipe_blend_state blend;

    memset(&blend, 0, sizeof(struct pipe_blend_state));
    blend.rt[0].blend_enable = 0;
    blend.rt[0].colormask = PIPE_MASK_RGBA;

    /* porter&duff src */
    blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
    blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
    blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_ZERO;
    blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ZERO;

    cso_set_blend(r->cso, &blend);
}

static void
xa_yuv_bind_shaders(struct xa_context *r)
{
    unsigned vs_traits = 0, fs_traits = 0;
    struct xa_shader shader;

    vs_traits |= VS_YUV;
    fs_traits |= FS_YUV;

    shader = xa_shaders_get(r->shaders, vs_traits, fs_traits);
    cso_set_vertex_shader_handle(r->cso, shader.vs);
    cso_set_fragment_shader_handle(r->cso, shader.fs);
}

static void
xa_yuv_bind_samplers(struct xa_context *r, struct xa_surface *yuv[])
{
    struct pipe_sampler_state *samplers[3];
    struct pipe_sampler_state sampler;
    struct pipe_sampler_view view_templ;
    unsigned int i;

    memset(&sampler, 0, sizeof(struct pipe_sampler_state));

    sampler.wrap_s = PIPE_TEX_WRAP_CLAMP;
    sampler.wrap_t = PIPE_TEX_WRAP_CLAMP;
    sampler.min_img_filter = PIPE_TEX_FILTER_LINEAR;
    sampler.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
    sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NEAREST;

    for (i = 0; i < 3; ++i) {
	samplers[i] = &sampler;
	u_sampler_view_default_template(&view_templ, yuv[i]->tex,
					yuv[i]->tex->format);

	r->bound_sampler_views[i] =
	    r->pipe->create_sampler_view(r->pipe, yuv[i]->tex, &view_templ);
    }
    r->num_bound_samplers = 3;
    cso_set_samplers(r->cso, PIPE_SHADER_FRAGMENT, 3, (const struct pipe_sampler_state **)samplers);
    r->pipe->set_sampler_views(r->pipe, PIPE_SHADER_FRAGMENT, 0, 3, 0, false, r->bound_sampler_views);
}

static void
xa_yuv_fs_constants(struct xa_context *r, const float conversion_matrix[])
{
    const int param_bytes = 16 * sizeof(float);

    renderer_set_constants(r, PIPE_SHADER_FRAGMENT,
			   conversion_matrix, param_bytes);
}

XA_EXPORT int
xa_yuv_planar_blit(struct xa_context *r,
		   int src_x,
		   int src_y,
		   int src_w,
		   int src_h,
		   int dst_x,
		   int dst_y,
		   int dst_w,
		   int dst_h,
		   struct xa_box *box,
		   unsigned int num_boxes,
		   const float conversion_matrix[],
		   struct xa_surface *dst, struct xa_surface *yuv[])
{
    float scale_x;
    float scale_y;
    int ret;

    if (dst_w == 0 || dst_h == 0)
	return XA_ERR_NONE;

    ret = xa_ctx_srf_create(r, dst);
    if (ret != XA_ERR_NONE)
	return -XA_ERR_NORES;

    renderer_bind_destination(r, r->srf);
    xa_yuv_bind_blend_state(r);
    xa_yuv_bind_shaders(r);
    xa_yuv_bind_samplers(r, yuv);
    xa_yuv_fs_constants(r, conversion_matrix);

    scale_x = (float)src_w / (float)dst_w;
    scale_y = (float)src_h / (float)dst_h;

    while (num_boxes--) {
	int x = box->x1;
	int y = box->y1;
	int w = box->x2 - box->x1;
	int h = box->y2 - box->y1;

        xa_scissor_update(r, x, y, box->x2, box->y2);
	renderer_draw_yuv(r,
			  (float)src_x + scale_x * (x - dst_x),
			  (float)src_y + scale_y * (y - dst_y),
			  scale_x * w, scale_y * h, x, y, w, h, yuv);
	box++;
    }

    xa_context_flush(r);

    xa_ctx_sampler_views_destroy(r);
    xa_ctx_srf_destroy(r);

    return XA_ERR_NONE;
}
