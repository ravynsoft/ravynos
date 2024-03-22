/**************************************************************************
 *
 * Copyright © 2010 Jakob Bornecrantz
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#define USE_TRACE 0
#define WIDTH 300
#define HEIGHT 300
#define NEAR 0
#define FAR 1
#define FLIP 0

/* pipe_*_state structs */
#include "pipe/p_state.h"
/* pipe_context */
#include "pipe/p_context.h"
/* pipe_screen */
#include "pipe/p_screen.h"
/* PIPE_* */
#include "pipe/p_defines.h"
/* TGSI_SEMANTIC_{POSITION|GENERIC} */
#include "pipe/p_shader_tokens.h"
/* pipe_buffer_* helpers */
#include "util/u_inlines.h"

/* constant state object helper */
#include "cso_cache/cso_context.h"

/* debug_dump_surface_bmp */
#include "util/u_debug_image.h"
/* util_draw_vertex_buffer helper */
#include "util/u_draw_quad.h"
/* FREE & CALLOC_STRUCT */
#include "util/u_memory.h"
/* util_make_[fragment|vertex]_passthrough_shader */
#include "util/u_simple_shaders.h"
/* to get a hardware pipe driver */
#include "pipe-loader/pipe_loader.h"

struct program
{
	struct pipe_loader_device *dev;
	struct pipe_screen *screen;
	struct pipe_context *pipe;
	struct cso_context *cso;

	struct pipe_blend_state blend;
	struct pipe_depth_stencil_alpha_state depthstencil;
	struct pipe_rasterizer_state rasterizer;
	struct pipe_viewport_state viewport;
	struct pipe_framebuffer_state framebuffer;
	struct cso_velems_state velem;

	void *vs;
	void *fs;

	union pipe_color_union clear_color;

	struct pipe_resource *vbuf;
	struct pipe_resource *target;
};

static void init_prog(struct program *p)
{
	struct pipe_surface surf_tmpl;
	ASSERTED int ret;

	/* find a hardware device */
	ret = pipe_loader_probe(&p->dev, 1, false);
	assert(ret);

	/* init a pipe screen */
	p->screen = pipe_loader_create_screen(p->dev);
	assert(p->screen);

	/* create the pipe driver context and cso context */
	p->pipe = p->screen->context_create(p->screen, NULL, 0);
	p->cso = cso_create_context(p->pipe, 0);

	/* set clear color */
	p->clear_color.f[0] = 0.3;
	p->clear_color.f[1] = 0.1;
	p->clear_color.f[2] = 0.3;
	p->clear_color.f[3] = 1.0;

	/* vertex buffer */
	{
		float vertices[4][2][4] = {
			{
				{ 0.0f, -0.9f, 0.0f, 1.0f },
				{ 1.0f, 0.0f, 0.0f, 1.0f }
			},
			{
				{ -0.9f, 0.9f, 0.0f, 1.0f },
				{ 0.0f, 1.0f, 0.0f, 1.0f }
			},
			{
				{ 0.9f, 0.9f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 1.0f, 1.0f }
			}
		};

		p->vbuf = pipe_buffer_create(p->screen, PIPE_BIND_VERTEX_BUFFER,
					     PIPE_USAGE_DEFAULT, sizeof(vertices));
		pipe_buffer_write(p->pipe, p->vbuf, 0, sizeof(vertices), vertices);
	}

	/* render target texture */
	{
		struct pipe_resource tmplt;
		memset(&tmplt, 0, sizeof(tmplt));
		tmplt.target = PIPE_TEXTURE_2D;
		tmplt.format = PIPE_FORMAT_B8G8R8A8_UNORM; /* All drivers support this */
		tmplt.width0 = WIDTH;
		tmplt.height0 = HEIGHT;
		tmplt.depth0 = 1;
		tmplt.array_size = 1;
		tmplt.last_level = 0;
		tmplt.bind = PIPE_BIND_RENDER_TARGET;

		p->target = p->screen->resource_create(p->screen, &tmplt);
	}

	/* disabled blending/masking */
	memset(&p->blend, 0, sizeof(p->blend));
	p->blend.rt[0].colormask = PIPE_MASK_RGBA;

	/* no-op depth/stencil/alpha */
	memset(&p->depthstencil, 0, sizeof(p->depthstencil));

	/* rasterizer */
	memset(&p->rasterizer, 0, sizeof(p->rasterizer));
	p->rasterizer.cull_face = PIPE_FACE_NONE;
	p->rasterizer.half_pixel_center = 1;
	p->rasterizer.bottom_edge_rule = 1;
	p->rasterizer.depth_clip_near = 1;
	p->rasterizer.depth_clip_far = 1;

	surf_tmpl.format = PIPE_FORMAT_B8G8R8A8_UNORM;
	surf_tmpl.u.tex.level = 0;
	surf_tmpl.u.tex.first_layer = 0;
	surf_tmpl.u.tex.last_layer = 0;
	/* drawing destination */
	memset(&p->framebuffer, 0, sizeof(p->framebuffer));
	p->framebuffer.width = WIDTH;
	p->framebuffer.height = HEIGHT;
	p->framebuffer.nr_cbufs = 1;
	p->framebuffer.cbufs[0] = p->pipe->create_surface(p->pipe, p->target, &surf_tmpl);

	/* viewport, depth isn't really needed */
	{
		float x = 0;
		float y = 0;
		float z = NEAR;
		float half_width = (float)WIDTH / 2.0f;
		float half_height = (float)HEIGHT / 2.0f;
		float half_depth = ((float)FAR - (float)NEAR) / 2.0f;
		float scale, bias;

		if (FLIP) {
			scale = -1.0f;
			bias = (float)HEIGHT;
		} else {
			scale = 1.0f;
			bias = 0.0f;
		}

		p->viewport.scale[0] = half_width;
		p->viewport.scale[1] = half_height * scale;
		p->viewport.scale[2] = half_depth;

		p->viewport.translate[0] = half_width + x;
		p->viewport.translate[1] = (half_height + y) * scale + bias;
		p->viewport.translate[2] = half_depth + z;

		p->viewport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
		p->viewport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
		p->viewport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
		p->viewport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;
	}

	/* vertex elements state */
	memset(&p->velem, 0, sizeof(p->velem));
        p->velem.count = 2;

	p->velem.velems[0].src_offset = 0 * 4 * sizeof(float); /* offset 0, first element */
	p->velem.velems[0].instance_divisor = 0;
	p->velem.velems[0].vertex_buffer_index = 0;
	p->velem.velems[0].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
   p->velem.velems[0].src_stride = 2 * 4 * sizeof(float);

	p->velem.velems[1].src_offset = 1 * 4 * sizeof(float); /* offset 16, second element */
	p->velem.velems[1].instance_divisor = 0;
	p->velem.velems[1].vertex_buffer_index = 0;
	p->velem.velems[1].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
   p->velem.velems[1].src_stride = 2 * 4 * sizeof(float);

	/* vertex shader */
	{
		const enum tgsi_semantic semantic_names[] =
			{ TGSI_SEMANTIC_POSITION, TGSI_SEMANTIC_COLOR };
		const uint semantic_indexes[] = { 0, 0 };
		p->vs = util_make_vertex_passthrough_shader(p->pipe, 2, semantic_names, semantic_indexes, false);
	}

	/* fragment shader */
	p->fs = util_make_fragment_passthrough_shader(p->pipe,
                    TGSI_SEMANTIC_COLOR, TGSI_INTERPOLATE_PERSPECTIVE, true);
}

static void close_prog(struct program *p)
{
	cso_destroy_context(p->cso);

	p->pipe->delete_vs_state(p->pipe, p->vs);
	p->pipe->delete_fs_state(p->pipe, p->fs);

	pipe_surface_reference(&p->framebuffer.cbufs[0], NULL);
	pipe_resource_reference(&p->target, NULL);
	pipe_resource_reference(&p->vbuf, NULL);

	p->pipe->destroy(p->pipe);
	p->screen->destroy(p->screen);
	pipe_loader_release(&p->dev, 1);

	FREE(p);
}

static void draw(struct program *p)
{
	/* set the render target */
	cso_set_framebuffer(p->cso, &p->framebuffer);

	/* clear the render target */
	p->pipe->clear(p->pipe, PIPE_CLEAR_COLOR, NULL, &p->clear_color, 0, 0);

	/* set misc state we care about */
	cso_set_blend(p->cso, &p->blend);
	cso_set_depth_stencil_alpha(p->cso, &p->depthstencil);
	cso_set_rasterizer(p->cso, &p->rasterizer);
	cso_set_viewport(p->cso, &p->viewport);

	/* shaders */
	cso_set_fragment_shader_handle(p->cso, p->fs);
	cso_set_vertex_shader_handle(p->cso, p->vs);

	/* vertex element data */
	cso_set_vertex_elements(p->cso, &p->velem);

	util_draw_vertex_buffer(p->pipe, p->cso,
	                        p->vbuf, 0,
	                        MESA_PRIM_TRIANGLES,
	                        3,  /* verts */
	                        2); /* attribs/vert */

        p->pipe->flush(p->pipe, NULL, 0);

	debug_dump_surface_bmp(p->pipe, "result.bmp", p->framebuffer.cbufs[0]);
}

int main(int argc, char** argv)
{
	struct program *p = CALLOC_STRUCT(program);

	init_prog(p);
	draw(p);
	close_prog(p);

	return 0;
}
