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
 */
#include "xa_priv.h"

#include "util/format/u_formats.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "pipe/p_shader_tokens.h"

#include "util/u_memory.h"

#include "tgsi/tgsi_ureg.h"

#include "cso_cache/cso_context.h"
#include "cso_cache/cso_hash.h"

/* Vertex shader:
 * IN[0]    = vertex pos
 * IN[1]    = src tex coord | solid fill color
 * IN[2]    = mask tex coord
 * IN[3]    = dst tex coord
 * CONST[0] = (2/dst_width, 2/dst_height, 1, 1)
 * CONST[1] = (-1, -1, 0, 0)
 *
 * OUT[0]   = vertex pos
 * OUT[1]   = src tex coord
 * OUT[2]   = mask tex coord
 * OUT[3]   = dst tex coord
 */

/* Fragment shader. Samplers are allocated when needed.
 * SAMP[0]  = sampler for first texture (src or mask if src is solid)
 * SAMP[1]  = sampler for second texture (mask or none)
 * IN[0]    = first texture coordinates if present
 * IN[1]    = second texture coordinates if present
 * CONST[0] = Solid color (src if src solid or mask if mask solid
 *            or src in mask if both solid).
 *
 * OUT[0] = color
 */

static void
print_fs_traits(int fs_traits)
{
    const char *strings[] = {
	"FS_COMPOSITE",		/* = 1 << 0, */
	"FS_MASK",		/* = 1 << 1, */
	"FS_SRC_SRC",	        /* = 1 << 2, */
	"FS_MASK_SRC",	        /* = 1 << 3, */
	"FS_YUV",	        /* = 1 << 4, */
	"FS_SRC_REPEAT_NONE",	/* = 1 << 5, */
	"FS_MASK_REPEAT_NONE",	/* = 1 << 6, */
	"FS_SRC_SWIZZLE_RGB",	/* = 1 << 7, */
	"FS_MASK_SWIZZLE_RGB",	/* = 1 << 8, */
	"FS_SRC_SET_ALPHA",	/* = 1 << 9, */
	"FS_MASK_SET_ALPHA",	/* = 1 << 10, */
	"FS_SRC_LUMINANCE",	/* = 1 << 11, */
	"FS_MASK_LUMINANCE",	/* = 1 << 12, */
	"FS_DST_LUMINANCE",     /* = 1 << 13, */
        "FS_CA",                /* = 1 << 14, */
    };
    int i, k;

    debug_printf("%s: ", __func__);

    for (i = 0, k = 1; k < (1 << 16); i++, k <<= 1) {
	if (fs_traits & k)
	    debug_printf("%s, ", strings[i]);
    }

    debug_printf("\n");
}

struct xa_shaders {
    struct xa_context *r;

    struct cso_hash vs_hash;
    struct cso_hash fs_hash;
};

static inline void
src_in_mask(struct ureg_program *ureg,
	    struct ureg_dst dst,
	    struct ureg_src src,
	    struct ureg_src mask,
	    unsigned mask_luminance, bool component_alpha)
{
    if (mask_luminance)
        if (component_alpha) {
            ureg_MOV(ureg, dst, src);
            ureg_MUL(ureg, ureg_writemask(dst, TGSI_WRITEMASK_W),
                     src, ureg_scalar(mask, TGSI_SWIZZLE_X));
        } else {
            ureg_MUL(ureg, dst, src, ureg_scalar(mask, TGSI_SWIZZLE_X));
        }
    else if (!component_alpha)
        ureg_MUL(ureg, dst, src, ureg_scalar(mask, TGSI_SWIZZLE_W));
    else
        ureg_MUL(ureg, dst, src, mask);
}

static struct ureg_src
vs_normalize_coords(struct ureg_program *ureg,
		    struct ureg_src coords,
		    struct ureg_src const0, struct ureg_src const1)
{
    struct ureg_dst tmp = ureg_DECL_temporary(ureg);
    struct ureg_src ret;

    ureg_MAD(ureg, tmp, coords, const0, const1);
    ret = ureg_src(tmp);
    ureg_release_temporary(ureg, tmp);
    return ret;
}

static void *
create_vs(struct pipe_context *pipe, unsigned vs_traits)
{
    struct ureg_program *ureg;
    struct ureg_src src;
    struct ureg_dst dst;
    struct ureg_src const0, const1;
    bool is_composite = (vs_traits & VS_COMPOSITE) != 0;
    bool has_mask = (vs_traits & VS_MASK) != 0;
    bool is_yuv = (vs_traits & VS_YUV) != 0;
    bool is_src_src = (vs_traits & VS_SRC_SRC) != 0;
    bool is_mask_src = (vs_traits & VS_MASK_SRC) != 0;
    unsigned input_slot = 0;

    ureg = ureg_create(PIPE_SHADER_VERTEX);
    if (ureg == NULL)
	return NULL;

    const0 = ureg_DECL_constant(ureg, 0);
    const1 = ureg_DECL_constant(ureg, 1);

    /* it has to be either a fill or a composite op */
    src = ureg_DECL_vs_input(ureg, input_slot++);
    dst = ureg_DECL_output(ureg, TGSI_SEMANTIC_POSITION, 0);
    src = vs_normalize_coords(ureg, src, const0, const1);
    ureg_MOV(ureg, dst, src);

    if (is_yuv) {
	src = ureg_DECL_vs_input(ureg, input_slot++);
	dst = ureg_DECL_output(ureg, TGSI_SEMANTIC_GENERIC, 0);
	ureg_MOV(ureg, dst, src);
    }

    if (is_composite) {
        if (!is_src_src || (has_mask && !is_mask_src)) {
            src = ureg_DECL_vs_input(ureg, input_slot++);
            dst = ureg_DECL_output(ureg, TGSI_SEMANTIC_GENERIC, 0);
            ureg_MOV(ureg, dst, src);
        }

        if (!is_src_src && (has_mask && !is_mask_src)) {
            src = ureg_DECL_vs_input(ureg, input_slot++);
            dst = ureg_DECL_output(ureg, TGSI_SEMANTIC_GENERIC, 1);
            ureg_MOV(ureg, dst, src);
        }
    }

    ureg_END(ureg);

    return ureg_create_shader_and_destroy(ureg, pipe);
}

static void *
create_yuv_shader(struct pipe_context *pipe, struct ureg_program *ureg)
{
    struct ureg_src y_sampler, u_sampler, v_sampler;
    struct ureg_src pos;
    struct ureg_src matrow0, matrow1, matrow2, matrow3;
    struct ureg_dst y, u, v, rgb;
    struct ureg_dst out = ureg_DECL_output(ureg,
					   TGSI_SEMANTIC_COLOR,
					   0);

    pos = ureg_DECL_fs_input(ureg,
			     TGSI_SEMANTIC_GENERIC, 0,
			     TGSI_INTERPOLATE_PERSPECTIVE);

    rgb = ureg_DECL_temporary(ureg);
    y = ureg_DECL_temporary(ureg);
    u = ureg_DECL_temporary(ureg);
    v = ureg_DECL_temporary(ureg);

    y_sampler = ureg_DECL_sampler(ureg, 0);
    u_sampler = ureg_DECL_sampler(ureg, 1);
    v_sampler = ureg_DECL_sampler(ureg, 2);

    ureg_DECL_sampler_view(ureg, 0, TGSI_TEXTURE_2D,
                           TGSI_RETURN_TYPE_FLOAT, TGSI_RETURN_TYPE_FLOAT,
                           TGSI_RETURN_TYPE_FLOAT, TGSI_RETURN_TYPE_FLOAT);
    ureg_DECL_sampler_view(ureg, 1, TGSI_TEXTURE_2D,
                           TGSI_RETURN_TYPE_FLOAT, TGSI_RETURN_TYPE_FLOAT,
                           TGSI_RETURN_TYPE_FLOAT, TGSI_RETURN_TYPE_FLOAT);
    ureg_DECL_sampler_view(ureg, 2, TGSI_TEXTURE_2D,
                           TGSI_RETURN_TYPE_FLOAT, TGSI_RETURN_TYPE_FLOAT,
                           TGSI_RETURN_TYPE_FLOAT, TGSI_RETURN_TYPE_FLOAT);

    matrow0 = ureg_DECL_constant(ureg, 0);
    matrow1 = ureg_DECL_constant(ureg, 1);
    matrow2 = ureg_DECL_constant(ureg, 2);
    matrow3 = ureg_DECL_constant(ureg, 3);

    ureg_TEX(ureg, y, TGSI_TEXTURE_2D, pos, y_sampler);
    ureg_TEX(ureg, u, TGSI_TEXTURE_2D, pos, u_sampler);
    ureg_TEX(ureg, v, TGSI_TEXTURE_2D, pos, v_sampler);

    ureg_MOV(ureg, rgb, matrow3);
    ureg_MAD(ureg, rgb,
	     ureg_scalar(ureg_src(y), TGSI_SWIZZLE_X), matrow0, ureg_src(rgb));
    ureg_MAD(ureg, rgb,
	     ureg_scalar(ureg_src(u), TGSI_SWIZZLE_X), matrow1, ureg_src(rgb));
    ureg_MAD(ureg, rgb,
	     ureg_scalar(ureg_src(v), TGSI_SWIZZLE_X), matrow2, ureg_src(rgb));

    ureg_MOV(ureg, out, ureg_src(rgb));

    ureg_release_temporary(ureg, rgb);
    ureg_release_temporary(ureg, y);
    ureg_release_temporary(ureg, u);
    ureg_release_temporary(ureg, v);

    ureg_END(ureg);

    return ureg_create_shader_and_destroy(ureg, pipe);
}

static inline void
xrender_tex(struct ureg_program *ureg,
	    struct ureg_dst dst,
	    struct ureg_src coords,
	    struct ureg_src sampler,
	    const struct ureg_src *imm0,
	    bool repeat_none, bool swizzle, bool set_alpha)
{
    if (repeat_none) {
	struct ureg_dst tmp0 = ureg_DECL_temporary(ureg);
	struct ureg_dst tmp1 = ureg_DECL_temporary(ureg);

	ureg_SGT(ureg, tmp1, ureg_swizzle(coords,
					  TGSI_SWIZZLE_X,
					  TGSI_SWIZZLE_Y,
					  TGSI_SWIZZLE_X,
					  TGSI_SWIZZLE_Y), ureg_scalar(*imm0,
								       TGSI_SWIZZLE_X));
	ureg_SLT(ureg, tmp0,
		 ureg_swizzle(coords, TGSI_SWIZZLE_X, TGSI_SWIZZLE_Y,
			      TGSI_SWIZZLE_X, TGSI_SWIZZLE_Y), ureg_scalar(*imm0,
									   TGSI_SWIZZLE_W));
	ureg_MIN(ureg, tmp0, ureg_src(tmp0), ureg_src(tmp1));
	ureg_MIN(ureg, tmp0, ureg_scalar(ureg_src(tmp0), TGSI_SWIZZLE_X),
		 ureg_scalar(ureg_src(tmp0), TGSI_SWIZZLE_Y));
	ureg_TEX(ureg, tmp1, TGSI_TEXTURE_2D, coords, sampler);
	if (swizzle)
	    ureg_MOV(ureg, tmp1, ureg_swizzle(ureg_src(tmp1),
					      TGSI_SWIZZLE_Z,
					      TGSI_SWIZZLE_Y, TGSI_SWIZZLE_X,
					      TGSI_SWIZZLE_W));
	if (set_alpha)
	    ureg_MOV(ureg,
		     ureg_writemask(tmp1, TGSI_WRITEMASK_W),
		     ureg_scalar(*imm0, TGSI_SWIZZLE_W));
	ureg_MUL(ureg, dst, ureg_src(tmp1), ureg_src(tmp0));
	ureg_release_temporary(ureg, tmp0);
	ureg_release_temporary(ureg, tmp1);
    } else {
	if (swizzle) {
	    struct ureg_dst tmp = ureg_DECL_temporary(ureg);

	    ureg_TEX(ureg, tmp, TGSI_TEXTURE_2D, coords, sampler);
	    ureg_MOV(ureg, dst, ureg_swizzle(ureg_src(tmp),
					     TGSI_SWIZZLE_Z,
					     TGSI_SWIZZLE_Y, TGSI_SWIZZLE_X,
					     TGSI_SWIZZLE_W));
	    ureg_release_temporary(ureg, tmp);
	} else {
	    ureg_TEX(ureg, dst, TGSI_TEXTURE_2D, coords, sampler);
	}
	if (set_alpha)
	    ureg_MOV(ureg,
		     ureg_writemask(dst, TGSI_WRITEMASK_W),
		     ureg_scalar(*imm0, TGSI_SWIZZLE_W));
    }
}

static void
read_input(struct ureg_program *ureg,
           struct ureg_dst dst,
           const struct ureg_src *imm0,
           bool repeat_none, bool swizzle, bool set_alpha,
           bool is_src, unsigned *cur_constant, unsigned *cur_sampler)
{
    struct ureg_src input, sampler;

    if (is_src) {
        input = ureg_DECL_constant(ureg, (*cur_constant)++);
        ureg_MOV(ureg, dst, input);
    } else {
        sampler = ureg_DECL_sampler(ureg, *cur_sampler);
        ureg_DECL_sampler_view(ureg, *cur_sampler, TGSI_TEXTURE_2D,
                               TGSI_RETURN_TYPE_FLOAT, TGSI_RETURN_TYPE_FLOAT,
                               TGSI_RETURN_TYPE_FLOAT, TGSI_RETURN_TYPE_FLOAT);
        input = ureg_DECL_fs_input(ureg,
                                   TGSI_SEMANTIC_GENERIC, (*cur_sampler)++,
                                   TGSI_INTERPOLATE_PERSPECTIVE);
        xrender_tex(ureg, dst, input, sampler, imm0,
                    repeat_none, swizzle, set_alpha);
    }
}

static void *
create_fs(struct pipe_context *pipe, unsigned fs_traits)
{
    struct ureg_program *ureg;
    struct ureg_dst src, mask;
    struct ureg_dst out;
    struct ureg_src imm0 = { 0 };
    unsigned has_mask = (fs_traits & FS_MASK) != 0;
    unsigned is_yuv = (fs_traits & FS_YUV) != 0;
    unsigned src_repeat_none = (fs_traits & FS_SRC_REPEAT_NONE) != 0;
    unsigned mask_repeat_none = (fs_traits & FS_MASK_REPEAT_NONE) != 0;
    unsigned src_swizzle = (fs_traits & FS_SRC_SWIZZLE_RGB) != 0;
    unsigned mask_swizzle = (fs_traits & FS_MASK_SWIZZLE_RGB) != 0;
    unsigned src_set_alpha = (fs_traits & FS_SRC_SET_ALPHA) != 0;
    unsigned mask_set_alpha = (fs_traits & FS_MASK_SET_ALPHA) != 0;
    unsigned src_luminance = (fs_traits & FS_SRC_LUMINANCE) != 0;
    unsigned mask_luminance = (fs_traits & FS_MASK_LUMINANCE) != 0;
    unsigned dst_luminance = (fs_traits & FS_DST_LUMINANCE) != 0;
    unsigned is_src_src = (fs_traits & FS_SRC_SRC) != 0;
    unsigned is_mask_src = (fs_traits & FS_MASK_SRC) != 0;
    bool component_alpha = (fs_traits & FS_CA) != 0;
    unsigned cur_sampler = 0;
    unsigned cur_constant = 0;

#if 0
    print_fs_traits(fs_traits);
#else
    (void)print_fs_traits;
#endif

    ureg = ureg_create(PIPE_SHADER_FRAGMENT);
    if (ureg == NULL)
	return NULL;

    if (is_yuv)
       return create_yuv_shader(pipe, ureg);

    out = ureg_DECL_output(ureg, TGSI_SEMANTIC_COLOR, 0);

    if (src_repeat_none || mask_repeat_none ||
	src_set_alpha || mask_set_alpha || src_luminance) {
	imm0 = ureg_imm4f(ureg, 0, 0, 0, 1);
    }

    src = (has_mask || src_luminance || dst_luminance) ?
        ureg_DECL_temporary(ureg) : out;

    read_input(ureg, src, &imm0, src_repeat_none, src_swizzle,
               src_set_alpha, is_src_src, &cur_constant, &cur_sampler);

    if (src_luminance) {
	ureg_MOV(ureg, src, ureg_scalar(ureg_src(src), TGSI_SWIZZLE_X));
	ureg_MOV(ureg, ureg_writemask(src, TGSI_WRITEMASK_XYZ),
		 ureg_scalar(imm0, TGSI_SWIZZLE_X));
	if (!has_mask && !dst_luminance)
	    ureg_MOV(ureg, out, ureg_src(src));
    }

    if (has_mask) {
	mask = ureg_DECL_temporary(ureg);
        read_input(ureg, mask, &imm0, mask_repeat_none,
                   mask_swizzle, mask_set_alpha, is_mask_src, &cur_constant,
                   &cur_sampler);

	src_in_mask(ureg, (dst_luminance) ? src : out, ureg_src(src),
		    ureg_src(mask), mask_luminance, component_alpha);

	ureg_release_temporary(ureg, mask);
    }

    if (dst_luminance) {
	/*
	 * Make sure the alpha channel goes into the output L8 surface.
	 */
	ureg_MOV(ureg, out, ureg_scalar(ureg_src(src), TGSI_SWIZZLE_W));
    }

    ureg_END(ureg);

    return ureg_create_shader_and_destroy(ureg, pipe);
}

struct xa_shaders *
xa_shaders_create(struct xa_context *r)
{
    struct xa_shaders *sc = CALLOC_STRUCT(xa_shaders);

    sc->r = r;
    cso_hash_init(&sc->vs_hash);
    cso_hash_init(&sc->fs_hash);

    return sc;
}

static void
cache_destroy(struct pipe_context *pipe,
	      struct cso_hash *hash, unsigned processor)
{
    struct cso_hash_iter iter = cso_hash_first_node(hash);

    while (!cso_hash_iter_is_null(iter)) {
	void *shader = (void *)cso_hash_iter_data(iter);

	if (processor == PIPE_SHADER_FRAGMENT) {
	    pipe->delete_fs_state(pipe, shader);
	} else if (processor == PIPE_SHADER_VERTEX) {
	    pipe->delete_vs_state(pipe, shader);
	}
	iter = cso_hash_erase(hash, iter);
    }
    cso_hash_deinit(hash);
}

void
xa_shaders_destroy(struct xa_shaders *sc)
{
    cache_destroy(sc->r->pipe, &sc->vs_hash, PIPE_SHADER_VERTEX);
    cache_destroy(sc->r->pipe, &sc->fs_hash, PIPE_SHADER_FRAGMENT);

    FREE(sc);
}

static inline void *
shader_from_cache(struct pipe_context *pipe,
		  unsigned type, struct cso_hash *hash, unsigned key)
{
    void *shader = NULL;

    struct cso_hash_iter iter = cso_hash_find(hash, key);

    if (cso_hash_iter_is_null(iter)) {
	if (type == PIPE_SHADER_VERTEX)
	    shader = create_vs(pipe, key);
	else
	    shader = create_fs(pipe, key);
	cso_hash_insert(hash, key, shader);
    } else
	shader = (void *)cso_hash_iter_data(iter);

    return shader;
}

struct xa_shader
xa_shaders_get(struct xa_shaders *sc, unsigned vs_traits, unsigned fs_traits)
{
    struct xa_shader shader = { NULL, NULL };
    void *vs, *fs;

    vs = shader_from_cache(sc->r->pipe, PIPE_SHADER_VERTEX,
			   &sc->vs_hash, vs_traits);
    fs = shader_from_cache(sc->r->pipe, PIPE_SHADER_FRAGMENT,
			   &sc->fs_hash, fs_traits);

    assert(vs && fs);
    if (!vs || !fs)
	return shader;

    shader.vs = vs;
    shader.fs = fs;

    return shader;
}
