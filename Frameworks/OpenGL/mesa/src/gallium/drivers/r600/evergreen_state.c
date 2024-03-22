/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "r600_formats.h"
#include "r600_shader.h"
#include "r600_query.h"
#include "r600d_common.h"
#include "evergreend.h"

#include "pipe/p_shader_tokens.h"
#include "util/u_endian.h"
#include "util/u_pack_color.h"
#include "util/u_memory.h"
#include "util/u_framebuffer.h"
#include "util/u_dual_blend.h"
#include "evergreen_compute.h"
#include "util/u_math.h"

#include <assert.h>

static inline unsigned evergreen_array_mode(unsigned mode)
{
	switch (mode) {
	default:
	case RADEON_SURF_MODE_LINEAR_ALIGNED:	return V_028C70_ARRAY_LINEAR_ALIGNED;
		break;
	case RADEON_SURF_MODE_1D:		return V_028C70_ARRAY_1D_TILED_THIN1;
		break;
	case RADEON_SURF_MODE_2D:		return V_028C70_ARRAY_2D_TILED_THIN1;
	}
}

static uint32_t eg_num_banks(uint32_t nbanks)
{
	switch (nbanks) {
	case 2:
		return 0;
	case 4:
		return 1;
	case 8:
	default:
		return 2;
	case 16:
		return 3;
	}
}


static unsigned eg_tile_split(unsigned tile_split)
{
	switch (tile_split) {
	case 64:	tile_split = 0;	break;
	case 128:	tile_split = 1;	break;
	case 256:	tile_split = 2;	break;
	case 512:	tile_split = 3;	break;
	default:
	case 1024:	tile_split = 4;	break;
	case 2048:	tile_split = 5;	break;
	case 4096:	tile_split = 6;	break;
	}
	return tile_split;
}

static unsigned eg_macro_tile_aspect(unsigned macro_tile_aspect)
{
	switch (macro_tile_aspect) {
	default:
	case 1:	macro_tile_aspect = 0;	break;
	case 2:	macro_tile_aspect = 1;	break;
	case 4:	macro_tile_aspect = 2;	break;
	case 8:	macro_tile_aspect = 3;	break;
	}
	return macro_tile_aspect;
}

static unsigned eg_bank_wh(unsigned bankwh)
{
	switch (bankwh) {
	default:
	case 1:	bankwh = 0;	break;
	case 2:	bankwh = 1;	break;
	case 4:	bankwh = 2;	break;
	case 8:	bankwh = 3;	break;
	}
	return bankwh;
}

static uint32_t r600_translate_blend_function(int blend_func)
{
	switch (blend_func) {
	case PIPE_BLEND_ADD:
		return V_028780_COMB_DST_PLUS_SRC;
	case PIPE_BLEND_SUBTRACT:
		return V_028780_COMB_SRC_MINUS_DST;
	case PIPE_BLEND_REVERSE_SUBTRACT:
		return V_028780_COMB_DST_MINUS_SRC;
	case PIPE_BLEND_MIN:
		return V_028780_COMB_MIN_DST_SRC;
	case PIPE_BLEND_MAX:
		return V_028780_COMB_MAX_DST_SRC;
	default:
		R600_ERR("Unknown blend function %d\n", blend_func);
		assert(0);
		break;
	}
	return 0;
}

static uint32_t r600_translate_blend_factor(int blend_fact)
{
	switch (blend_fact) {
	case PIPE_BLENDFACTOR_ONE:
		return V_028780_BLEND_ONE;
	case PIPE_BLENDFACTOR_SRC_COLOR:
		return V_028780_BLEND_SRC_COLOR;
	case PIPE_BLENDFACTOR_SRC_ALPHA:
		return V_028780_BLEND_SRC_ALPHA;
	case PIPE_BLENDFACTOR_DST_ALPHA:
		return V_028780_BLEND_DST_ALPHA;
	case PIPE_BLENDFACTOR_DST_COLOR:
		return V_028780_BLEND_DST_COLOR;
	case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
		return V_028780_BLEND_SRC_ALPHA_SATURATE;
	case PIPE_BLENDFACTOR_CONST_COLOR:
		return V_028780_BLEND_CONST_COLOR;
	case PIPE_BLENDFACTOR_CONST_ALPHA:
		return V_028780_BLEND_CONST_ALPHA;
	case PIPE_BLENDFACTOR_ZERO:
		return V_028780_BLEND_ZERO;
	case PIPE_BLENDFACTOR_INV_SRC_COLOR:
		return V_028780_BLEND_ONE_MINUS_SRC_COLOR;
	case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
		return V_028780_BLEND_ONE_MINUS_SRC_ALPHA;
	case PIPE_BLENDFACTOR_INV_DST_ALPHA:
		return V_028780_BLEND_ONE_MINUS_DST_ALPHA;
	case PIPE_BLENDFACTOR_INV_DST_COLOR:
		return V_028780_BLEND_ONE_MINUS_DST_COLOR;
	case PIPE_BLENDFACTOR_INV_CONST_COLOR:
		return V_028780_BLEND_ONE_MINUS_CONST_COLOR;
	case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
		return V_028780_BLEND_ONE_MINUS_CONST_ALPHA;
	case PIPE_BLENDFACTOR_SRC1_COLOR:
		return V_028780_BLEND_SRC1_COLOR;
	case PIPE_BLENDFACTOR_SRC1_ALPHA:
		return V_028780_BLEND_SRC1_ALPHA;
	case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
		return V_028780_BLEND_INV_SRC1_COLOR;
	case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
		return V_028780_BLEND_INV_SRC1_ALPHA;
	default:
		R600_ERR("Bad blend factor %d not supported!\n", blend_fact);
		assert(0);
		break;
	}
	return 0;
}

static unsigned r600_tex_dim(struct r600_texture *rtex,
			     unsigned view_target, unsigned nr_samples)
{
	unsigned res_target = rtex->resource.b.b.target;

	if (view_target == PIPE_TEXTURE_CUBE ||
	    view_target == PIPE_TEXTURE_CUBE_ARRAY)
		res_target = view_target;
		/* If interpreting cubemaps as something else, set 2D_ARRAY. */
	else if (res_target == PIPE_TEXTURE_CUBE ||
		 res_target == PIPE_TEXTURE_CUBE_ARRAY)
		res_target = PIPE_TEXTURE_2D_ARRAY;

	switch (res_target) {
	default:
	case PIPE_TEXTURE_1D:
		return V_030000_SQ_TEX_DIM_1D;
	case PIPE_TEXTURE_1D_ARRAY:
		return V_030000_SQ_TEX_DIM_1D_ARRAY;
	case PIPE_TEXTURE_2D:
	case PIPE_TEXTURE_RECT:
		return nr_samples > 1 ? V_030000_SQ_TEX_DIM_2D_MSAA :
					V_030000_SQ_TEX_DIM_2D;
	case PIPE_TEXTURE_2D_ARRAY:
		return nr_samples > 1 ? V_030000_SQ_TEX_DIM_2D_ARRAY_MSAA :
					V_030000_SQ_TEX_DIM_2D_ARRAY;
	case PIPE_TEXTURE_3D:
		return V_030000_SQ_TEX_DIM_3D;
	case PIPE_TEXTURE_CUBE:
	case PIPE_TEXTURE_CUBE_ARRAY:
		return V_030000_SQ_TEX_DIM_CUBEMAP;
	}
}

static uint32_t r600_translate_dbformat(enum pipe_format format)
{
	switch (format) {
	case PIPE_FORMAT_Z16_UNORM:
		return V_028040_Z_16;
	case PIPE_FORMAT_Z24X8_UNORM:
	case PIPE_FORMAT_Z24_UNORM_S8_UINT:
	case PIPE_FORMAT_X8Z24_UNORM:
	case PIPE_FORMAT_S8_UINT_Z24_UNORM:
		return V_028040_Z_24;
	case PIPE_FORMAT_Z32_FLOAT:
	case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
		return V_028040_Z_32_FLOAT;
	default:
		return ~0U;
	}
}

static bool r600_is_sampler_format_supported(struct pipe_screen *screen, enum pipe_format format)
{
	return r600_translate_texformat(screen, format, NULL, NULL, NULL,
                                   false) != ~0U;
}

static bool r600_is_colorbuffer_format_supported(enum amd_gfx_level chip, enum pipe_format format)
{
	return r600_translate_colorformat(chip, format, false) != ~0U &&
		r600_translate_colorswap(format, false) != ~0U;
}

static bool r600_is_zs_format_supported(enum pipe_format format)
{
	return r600_translate_dbformat(format) != ~0U;
}

bool evergreen_is_format_supported(struct pipe_screen *screen,
				   enum pipe_format format,
				   enum pipe_texture_target target,
				   unsigned sample_count,
				   unsigned storage_sample_count,
				   unsigned usage)
{
	struct r600_screen *rscreen = (struct r600_screen*)screen;
	unsigned retval = 0;

	if (target >= PIPE_MAX_TEXTURE_TYPES) {
		R600_ERR("r600: unsupported texture type %d\n", target);
		return false;
	}

	if (util_format_get_num_planes(format) > 1)
		return false;

	if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
		return false;

	if (sample_count > 1) {
		if (!rscreen->has_msaa)
			return false;

		switch (sample_count) {
		case 2:
		case 4:
		case 8:
			break;
		default:
			return false;
		}
	}

	if (usage & PIPE_BIND_SAMPLER_VIEW) {
		if (target == PIPE_BUFFER) {
			if (r600_is_buffer_format_supported(format, false))
				retval |= PIPE_BIND_SAMPLER_VIEW;
		} else {
			if (r600_is_sampler_format_supported(screen, format))
				retval |= PIPE_BIND_SAMPLER_VIEW;
		}
	}

	if ((usage & (PIPE_BIND_RENDER_TARGET |
		      PIPE_BIND_DISPLAY_TARGET |
		      PIPE_BIND_SCANOUT |
		      PIPE_BIND_SHARED |
		      PIPE_BIND_BLENDABLE)) &&
	    r600_is_colorbuffer_format_supported(rscreen->b.gfx_level, format)) {
		retval |= usage &
			  (PIPE_BIND_RENDER_TARGET |
			   PIPE_BIND_DISPLAY_TARGET |
			   PIPE_BIND_SCANOUT |
			   PIPE_BIND_SHARED);
		if (!util_format_is_pure_integer(format) &&
		    !util_format_is_depth_or_stencil(format))
			retval |= usage & PIPE_BIND_BLENDABLE;
	}

	if ((usage & PIPE_BIND_DEPTH_STENCIL) &&
	    r600_is_zs_format_supported(format)) {
		retval |= PIPE_BIND_DEPTH_STENCIL;
	}

	if ((usage & PIPE_BIND_VERTEX_BUFFER) &&
	    r600_is_buffer_format_supported(format, true)) {
		retval |= PIPE_BIND_VERTEX_BUFFER;
	}

	if (usage & PIPE_BIND_INDEX_BUFFER &&
	    r600_is_index_format_supported(format)) {
		retval |= PIPE_BIND_INDEX_BUFFER;
	}

	if ((usage & PIPE_BIND_LINEAR) &&
	    !util_format_is_compressed(format) &&
	    !(usage & PIPE_BIND_DEPTH_STENCIL))
		retval |= PIPE_BIND_LINEAR;

	return retval == usage;
}

static void *evergreen_create_blend_state_mode(struct pipe_context *ctx,
					       const struct pipe_blend_state *state, int mode)
{
	uint32_t color_control = 0, target_mask = 0;
	uint32_t alpha_to_mask = 0;
	struct r600_blend_state *blend = CALLOC_STRUCT(r600_blend_state);

	if (!blend) {
		return NULL;
	}

	r600_init_command_buffer(&blend->buffer, 20);
	r600_init_command_buffer(&blend->buffer_no_blend, 20);

	if (state->logicop_enable) {
		color_control |= (state->logicop_func << 16) | (state->logicop_func << 20);
	} else {
		color_control |= (0xcc << 16);
	}
	/* we pretend 8 buffer are used, CB_SHADER_MASK will disable unused one */
	if (state->independent_blend_enable) {
		for (int i = 0; i < 8; i++) {
			target_mask |= (state->rt[i].colormask << (4 * i));
		}
	} else {
		for (int i = 0; i < 8; i++) {
			target_mask |= (state->rt[0].colormask << (4 * i));
		}
	}

	/* only have dual source on MRT0 */
	blend->dual_src_blend = util_blend_state_is_dual(state, 0);
	blend->cb_target_mask = target_mask;
	blend->alpha_to_one = state->alpha_to_one;

	if (target_mask)
		color_control |= S_028808_MODE(mode);
	else
		color_control |= S_028808_MODE(V_028808_CB_DISABLE);

	r600_store_context_reg(&blend->buffer, R_028808_CB_COLOR_CONTROL, color_control);

	if (state->alpha_to_coverage) {
		if (state->alpha_to_coverage_dither) {
			alpha_to_mask = S_028B70_ALPHA_TO_MASK_ENABLE(1) |
			                S_028B70_ALPHA_TO_MASK_OFFSET0(3) |
			                S_028B70_ALPHA_TO_MASK_OFFSET1(1) |
			                S_028B70_ALPHA_TO_MASK_OFFSET2(0) |
			                S_028B70_ALPHA_TO_MASK_OFFSET3(2) |
			                S_028B70_OFFSET_ROUND(1);
		} else {
			alpha_to_mask = S_028B70_ALPHA_TO_MASK_ENABLE(1) |
			                S_028B70_ALPHA_TO_MASK_OFFSET0(2) |
			                S_028B70_ALPHA_TO_MASK_OFFSET1(2) |
			                S_028B70_ALPHA_TO_MASK_OFFSET2(2) |
			                S_028B70_ALPHA_TO_MASK_OFFSET3(2) |
			                S_028B70_OFFSET_ROUND(0);
		}
	}
	r600_store_context_reg(&blend->buffer, R_028B70_DB_ALPHA_TO_MASK, alpha_to_mask);

	r600_store_context_reg_seq(&blend->buffer, R_028780_CB_BLEND0_CONTROL, 8);

	/* Copy over the dwords set so far into buffer_no_blend.
	 * Only the CB_BLENDi_CONTROL registers must be set after this. */
	memcpy(blend->buffer_no_blend.buf, blend->buffer.buf, blend->buffer.num_dw * 4);
	blend->buffer_no_blend.num_dw = blend->buffer.num_dw;

	for (int i = 0; i < 8; i++) {
		/* state->rt entries > 0 only written if independent blending */
		const int j = state->independent_blend_enable ? i : 0;

		unsigned eqRGB = state->rt[j].rgb_func;
		unsigned srcRGB = state->rt[j].rgb_src_factor;
		unsigned dstRGB = state->rt[j].rgb_dst_factor;
		unsigned eqA = state->rt[j].alpha_func;
		unsigned srcA = state->rt[j].alpha_src_factor;
		unsigned dstA = state->rt[j].alpha_dst_factor;
		uint32_t bc = 0;

		r600_store_value(&blend->buffer_no_blend, 0);

		if (!state->rt[j].blend_enable) {
			r600_store_value(&blend->buffer, 0);
			continue;
		}

		bc |= S_028780_BLEND_CONTROL_ENABLE(1);
		bc |= S_028780_COLOR_COMB_FCN(r600_translate_blend_function(eqRGB));
		bc |= S_028780_COLOR_SRCBLEND(r600_translate_blend_factor(srcRGB));
		bc |= S_028780_COLOR_DESTBLEND(r600_translate_blend_factor(dstRGB));

		if (srcA != srcRGB || dstA != dstRGB || eqA != eqRGB) {
			bc |= S_028780_SEPARATE_ALPHA_BLEND(1);
			bc |= S_028780_ALPHA_COMB_FCN(r600_translate_blend_function(eqA));
			bc |= S_028780_ALPHA_SRCBLEND(r600_translate_blend_factor(srcA));
			bc |= S_028780_ALPHA_DESTBLEND(r600_translate_blend_factor(dstA));
		}
		r600_store_value(&blend->buffer, bc);
	}
	return blend;
}

static void *evergreen_create_blend_state(struct pipe_context *ctx,
					const struct pipe_blend_state *state)
{

	return evergreen_create_blend_state_mode(ctx, state, V_028808_CB_NORMAL);
}

static void *evergreen_create_dsa_state(struct pipe_context *ctx,
				   const struct pipe_depth_stencil_alpha_state *state)
{
	unsigned db_depth_control, alpha_test_control, alpha_ref;
	struct r600_dsa_state *dsa = CALLOC_STRUCT(r600_dsa_state);

	if (!dsa) {
		return NULL;
	}

	r600_init_command_buffer(&dsa->buffer, 3);

	dsa->valuemask[0] = state->stencil[0].valuemask;
	dsa->valuemask[1] = state->stencil[1].valuemask;
	dsa->writemask[0] = state->stencil[0].writemask;
	dsa->writemask[1] = state->stencil[1].writemask;
	dsa->zwritemask = state->depth_writemask;

	db_depth_control = S_028800_Z_ENABLE(state->depth_enabled) |
		S_028800_Z_WRITE_ENABLE(state->depth_writemask) |
		S_028800_ZFUNC(state->depth_func);

	/* stencil */
	if (state->stencil[0].enabled) {
		db_depth_control |= S_028800_STENCIL_ENABLE(1);
		db_depth_control |= S_028800_STENCILFUNC(state->stencil[0].func); /* translates straight */
		db_depth_control |= S_028800_STENCILFAIL(r600_translate_stencil_op(state->stencil[0].fail_op));
		db_depth_control |= S_028800_STENCILZPASS(r600_translate_stencil_op(state->stencil[0].zpass_op));
		db_depth_control |= S_028800_STENCILZFAIL(r600_translate_stencil_op(state->stencil[0].zfail_op));

		if (state->stencil[1].enabled) {
			db_depth_control |= S_028800_BACKFACE_ENABLE(1);
			db_depth_control |= S_028800_STENCILFUNC_BF(state->stencil[1].func); /* translates straight */
			db_depth_control |= S_028800_STENCILFAIL_BF(r600_translate_stencil_op(state->stencil[1].fail_op));
			db_depth_control |= S_028800_STENCILZPASS_BF(r600_translate_stencil_op(state->stencil[1].zpass_op));
			db_depth_control |= S_028800_STENCILZFAIL_BF(r600_translate_stencil_op(state->stencil[1].zfail_op));
		}
	}

	/* alpha */
	alpha_test_control = 0;
	alpha_ref = 0;
	if (state->alpha_enabled) {
		alpha_test_control = S_028410_ALPHA_FUNC(state->alpha_func);
		alpha_test_control |= S_028410_ALPHA_TEST_ENABLE(1);
		alpha_ref = fui(state->alpha_ref_value);
	}
	dsa->sx_alpha_test_control = alpha_test_control & 0xff;
	dsa->alpha_ref = alpha_ref;

	/* misc */
	r600_store_context_reg(&dsa->buffer, R_028800_DB_DEPTH_CONTROL, db_depth_control);
	return dsa;
}

static void *evergreen_create_rs_state(struct pipe_context *ctx,
					const struct pipe_rasterizer_state *state)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	unsigned tmp, spi_interp;
	float psize_min, psize_max;
	struct r600_rasterizer_state *rs = CALLOC_STRUCT(r600_rasterizer_state);

	if (!rs) {
		return NULL;
	}

	r600_init_command_buffer(&rs->buffer, 30);

	rs->scissor_enable = state->scissor;
	rs->clip_halfz = state->clip_halfz;
	rs->flatshade = state->flatshade;
	rs->sprite_coord_enable = state->sprite_coord_enable;
	rs->rasterizer_discard = state->rasterizer_discard;
	rs->two_side = state->light_twoside;
	rs->clip_plane_enable = state->clip_plane_enable;
	rs->pa_sc_line_stipple = state->line_stipple_enable ?
				S_028A0C_LINE_PATTERN(state->line_stipple_pattern) |
				S_028A0C_REPEAT_COUNT(state->line_stipple_factor) : 0;
	rs->pa_cl_clip_cntl =
		S_028810_DX_CLIP_SPACE_DEF(state->clip_halfz) |
		S_028810_ZCLIP_NEAR_DISABLE(!state->depth_clip_near) |
		S_028810_ZCLIP_FAR_DISABLE(!state->depth_clip_far) |
		S_028810_DX_LINEAR_ATTR_CLIP_ENA(1) |
		S_028810_DX_RASTERIZATION_KILL(state->rasterizer_discard);
	rs->multisample_enable = state->multisample;

	/* offset */
	rs->offset_units = state->offset_units;
	rs->offset_scale = state->offset_scale * 16.0f;
	rs->offset_enable = state->offset_point || state->offset_line || state->offset_tri;
	rs->offset_units_unscaled = state->offset_units_unscaled;

	if (state->point_size_per_vertex) {
		psize_min = util_get_min_point_size(state);
		psize_max = 8192;
	} else {
		/* Force the point size to be as if the vertex output was disabled. */
		psize_min = state->point_size;
		psize_max = state->point_size;
	}

	spi_interp = S_0286D4_FLAT_SHADE_ENA(1);
	spi_interp |= S_0286D4_PNT_SPRITE_ENA(1) |
		S_0286D4_PNT_SPRITE_OVRD_X(2) |
		S_0286D4_PNT_SPRITE_OVRD_Y(3) |
		S_0286D4_PNT_SPRITE_OVRD_Z(0) |
		S_0286D4_PNT_SPRITE_OVRD_W(1);
	if (state->sprite_coord_mode != PIPE_SPRITE_COORD_UPPER_LEFT) {
		spi_interp |= S_0286D4_PNT_SPRITE_TOP_1(1);
	}

	r600_store_context_reg_seq(&rs->buffer, R_028A00_PA_SU_POINT_SIZE, 3);
	/* point size 12.4 fixed point (divide by two, because 0.5 = 1 pixel) */
	tmp = r600_pack_float_12p4(state->point_size/2);
	r600_store_value(&rs->buffer, /* R_028A00_PA_SU_POINT_SIZE */
			 S_028A00_HEIGHT(tmp) | S_028A00_WIDTH(tmp));
	r600_store_value(&rs->buffer, /* R_028A04_PA_SU_POINT_MINMAX */
			 S_028A04_MIN_SIZE(r600_pack_float_12p4(psize_min/2)) |
			 S_028A04_MAX_SIZE(r600_pack_float_12p4(psize_max/2)));
	r600_store_value(&rs->buffer, /* R_028A08_PA_SU_LINE_CNTL */
			 S_028A08_WIDTH((unsigned)(state->line_width * 8)));

	r600_store_context_reg(&rs->buffer, R_0286D4_SPI_INTERP_CONTROL_0, spi_interp);
	r600_store_context_reg(&rs->buffer, R_028A48_PA_SC_MODE_CNTL_0,
			       S_028A48_MSAA_ENABLE(state->multisample) |
			       S_028A48_VPORT_SCISSOR_ENABLE(1) |
			       S_028A48_LINE_STIPPLE_ENABLE(state->line_stipple_enable));

	if (rctx->b.gfx_level == CAYMAN) {
		r600_store_context_reg(&rs->buffer, CM_R_028BE4_PA_SU_VTX_CNTL,
				       S_028C08_PIX_CENTER_HALF(state->half_pixel_center) |
				       S_028C08_QUANT_MODE(V_028C08_X_1_256TH));
	} else {
		r600_store_context_reg(&rs->buffer, R_028C08_PA_SU_VTX_CNTL,
				       S_028C08_PIX_CENTER_HALF(state->half_pixel_center) |
				       S_028C08_QUANT_MODE(V_028C08_X_1_256TH));
	}

	r600_store_context_reg(&rs->buffer, R_028B7C_PA_SU_POLY_OFFSET_CLAMP, fui(state->offset_clamp));
	r600_store_context_reg(&rs->buffer, R_028814_PA_SU_SC_MODE_CNTL,
			       S_028814_PROVOKING_VTX_LAST(!state->flatshade_first) |
			       S_028814_CULL_FRONT((state->cull_face & PIPE_FACE_FRONT) ? 1 : 0) |
			       S_028814_CULL_BACK((state->cull_face & PIPE_FACE_BACK) ? 1 : 0) |
			       S_028814_FACE(!state->front_ccw) |
			       S_028814_POLY_OFFSET_FRONT_ENABLE(util_get_offset(state, state->fill_front)) |
			       S_028814_POLY_OFFSET_BACK_ENABLE(util_get_offset(state, state->fill_back)) |
			       S_028814_POLY_OFFSET_PARA_ENABLE(state->offset_point || state->offset_line) |
			       S_028814_POLY_MODE(state->fill_front != PIPE_POLYGON_MODE_FILL ||
						  state->fill_back != PIPE_POLYGON_MODE_FILL) |
			       S_028814_POLYMODE_FRONT_PTYPE(r600_translate_fill(state->fill_front)) |
			       S_028814_POLYMODE_BACK_PTYPE(r600_translate_fill(state->fill_back)));
	return rs;
}

static void *evergreen_create_sampler_state(struct pipe_context *ctx,
					const struct pipe_sampler_state *state)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)ctx->screen;
	struct r600_pipe_sampler_state *ss = CALLOC_STRUCT(r600_pipe_sampler_state);
	unsigned max_aniso = rscreen->force_aniso >= 0 ? rscreen->force_aniso
						       : state->max_anisotropy;
	unsigned max_aniso_ratio = r600_tex_aniso_filter(max_aniso);
	bool trunc_coord = state->min_img_filter == PIPE_TEX_FILTER_NEAREST &&
			   state->mag_img_filter == PIPE_TEX_FILTER_NEAREST;
	float max_lod = state->max_lod;

	if (!ss) {
		return NULL;
	}

	/* If the min_mip_filter is NONE, then the texture has no mipmapping and
	 * MIP_FILTER will also be set to NONE. However, if more then one LOD is
	 * configured, then the texture lookup seems to fail for some specific texture
	 * formats. Forcing the number of LODs to one in this case fixes it. */
	if (state->min_mip_filter == PIPE_TEX_MIPFILTER_NONE)
		max_lod = state->min_lod;

	ss->border_color_use = sampler_state_needs_border_color(state);

	/* R_03C000_SQ_TEX_SAMPLER_WORD0_0 */
	ss->tex_sampler_words[0] =
		S_03C000_CLAMP_X(r600_tex_wrap(state->wrap_s)) |
		S_03C000_CLAMP_Y(r600_tex_wrap(state->wrap_t)) |
		S_03C000_CLAMP_Z(r600_tex_wrap(state->wrap_r)) |
		S_03C000_XY_MAG_FILTER(eg_tex_filter(state->mag_img_filter, max_aniso)) |
		S_03C000_XY_MIN_FILTER(eg_tex_filter(state->min_img_filter, max_aniso)) |
		S_03C000_MIP_FILTER(r600_tex_mipfilter(state->min_mip_filter)) |
		S_03C000_MAX_ANISO_RATIO(max_aniso_ratio) |
		S_03C000_DEPTH_COMPARE_FUNCTION(r600_tex_compare(state->compare_func)) |
		S_03C000_BORDER_COLOR_TYPE(ss->border_color_use ? V_03C000_SQ_TEX_BORDER_COLOR_REGISTER : 0);
	/* R_03C004_SQ_TEX_SAMPLER_WORD1_0 */
	ss->tex_sampler_words[1] =
		S_03C004_MIN_LOD(S_FIXED(CLAMP(state->min_lod, 0, 15), 8)) |
		S_03C004_MAX_LOD(S_FIXED(CLAMP(max_lod, 0, 15), 8));
	/* R_03C008_SQ_TEX_SAMPLER_WORD2_0 */
	ss->tex_sampler_words[2] =
		S_03C008_LOD_BIAS(S_FIXED(CLAMP(state->lod_bias, -16, 16), 8)) |
		(state->seamless_cube_map ? 0 : S_03C008_DISABLE_CUBE_WRAP(1)) |
		S_03C008_TRUNCATE_COORD(trunc_coord) |
		S_03C008_TYPE(1);

	if (ss->border_color_use) {
		memcpy(&ss->border_color, &state->border_color, sizeof(state->border_color));
	}
	return ss;
}

struct eg_buf_res_params {
	enum pipe_format pipe_format;
	unsigned offset;
	unsigned size;
	unsigned char swizzle[4];
	bool uncached;
	bool force_swizzle;
	bool size_in_bytes;
};

static void evergreen_fill_buffer_resource_words(struct r600_context *rctx,
						 struct pipe_resource *buffer,
						 struct eg_buf_res_params *params,
						 bool *skip_mip_address_reloc,
						 unsigned tex_resource_words[8])
{
	struct r600_texture *tmp = (struct r600_texture*)buffer;
	uint64_t va;
	int stride = util_format_get_blocksize(params->pipe_format);
	unsigned format, num_format, format_comp, endian;
	unsigned swizzle_res;
	const struct util_format_description *desc;

	r600_vertex_data_type(params->pipe_format,
			      &format, &num_format, &format_comp,
			      &endian);

	desc = util_format_description(params->pipe_format);

	if (params->force_swizzle)
		swizzle_res = r600_get_swizzle_combined(params->swizzle, NULL, true);
	else
		swizzle_res = r600_get_swizzle_combined(desc->swizzle, params->swizzle, true);

	va = tmp->resource.gpu_address + params->offset;
	*skip_mip_address_reloc = true;
	tex_resource_words[0] = va;
	tex_resource_words[1] = params->size - 1;
	tex_resource_words[2] = S_030008_BASE_ADDRESS_HI(va >> 32UL) |
		S_030008_STRIDE(stride) |
		S_030008_DATA_FORMAT(format) |
		S_030008_NUM_FORMAT_ALL(num_format) |
		S_030008_FORMAT_COMP_ALL(format_comp) |
		S_030008_ENDIAN_SWAP(endian);
	tex_resource_words[3] = swizzle_res | S_03000C_UNCACHED(params->uncached);
	/*
	 * dword 4 is for number of elements, for use with resinfo,
	 * albeit the amd gpu shader analyser
	 * uses a const buffer to store the element sizes for buffer txq
	 */
	tex_resource_words[4] = params->size_in_bytes ? params->size : (params->size / stride);

	tex_resource_words[5] = tex_resource_words[6] = 0;
	tex_resource_words[7] = S_03001C_TYPE(V_03001C_SQ_TEX_VTX_VALID_BUFFER);
}

static struct pipe_sampler_view *
texture_buffer_sampler_view(struct r600_context *rctx,
			    struct r600_pipe_sampler_view *view,
			    unsigned width0, unsigned height0)
{
	struct r600_texture *tmp = (struct r600_texture*)view->base.texture;
	struct eg_buf_res_params params;

	memset(&params, 0, sizeof(params));

	params.pipe_format = view->base.format;
	params.offset = view->base.u.buf.offset;
	params.size = view->base.u.buf.size;
	params.swizzle[0] = view->base.swizzle_r;
	params.swizzle[1] = view->base.swizzle_g;
	params.swizzle[2] = view->base.swizzle_b;
	params.swizzle[3] = view->base.swizzle_a;

	evergreen_fill_buffer_resource_words(rctx, view->base.texture,
					     &params, &view->skip_mip_address_reloc,
					     view->tex_resource_words);
	view->tex_resource = &tmp->resource;

	if (tmp->resource.gpu_address)
		list_addtail(&view->list, &rctx->texture_buffers);
	return &view->base;
}

struct eg_tex_res_params {
	enum pipe_format pipe_format;
	int force_level;
	unsigned width0;
	unsigned height0;
	unsigned first_level;
	unsigned last_level;
	unsigned first_layer;
	unsigned last_layer;
	unsigned target;
	unsigned char swizzle[4];
};

static int evergreen_fill_tex_resource_words(struct r600_context *rctx,
					     struct pipe_resource *texture,
					     struct eg_tex_res_params *params,
					     bool *skip_mip_address_reloc,
					     unsigned tex_resource_words[8])
{
	struct r600_screen *rscreen = (struct r600_screen*)rctx->b.b.screen;
	struct r600_texture *tmp = (struct r600_texture*)texture;
	unsigned format, endian;
	uint32_t word4 = 0, yuv_format = 0, pitch = 0;
	unsigned char array_mode = 0, non_disp_tiling = 0;
	unsigned height, depth, width;
	unsigned macro_aspect, tile_split, bankh, bankw, nbanks, fmask_bankh;
	struct legacy_surf_level *surflevel;
	unsigned base_level, first_level, last_level;
	unsigned dim, last_layer;
	uint64_t va;
	bool do_endian_swap = false;

	tile_split = tmp->surface.u.legacy.tile_split;
	surflevel = tmp->surface.u.legacy.level;

	/* Texturing with separate depth and stencil. */
	if (tmp->db_compatible) {
		switch (params->pipe_format) {
		case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
			params->pipe_format = PIPE_FORMAT_Z32_FLOAT;
			break;
		case PIPE_FORMAT_X8Z24_UNORM:
		case PIPE_FORMAT_S8_UINT_Z24_UNORM:
			/* Z24 is always stored like this for DB
			 * compatibility.
			 */
			params->pipe_format = PIPE_FORMAT_Z24X8_UNORM;
			break;
		case PIPE_FORMAT_X24S8_UINT:
		case PIPE_FORMAT_S8X24_UINT:
		case PIPE_FORMAT_X32_S8X24_UINT:
			params->pipe_format = PIPE_FORMAT_S8_UINT;
			tile_split = tmp->surface.u.legacy.stencil_tile_split;
			surflevel = tmp->surface.u.legacy.zs.stencil_level;
			break;
		default:;
		}
	}

	if (UTIL_ARCH_BIG_ENDIAN)
		do_endian_swap = !tmp->db_compatible;

	format = r600_translate_texformat(rctx->b.b.screen, params->pipe_format,
					  params->swizzle,
					  &word4, &yuv_format, do_endian_swap);
	assert(format != ~0);
	if (format == ~0) {
		return -1;
	}

	endian = r600_colorformat_endian_swap(format, do_endian_swap);

	base_level = 0;
	first_level = params->first_level;
	last_level = params->last_level;
	width = params->width0;
	height = params->height0;
	depth = texture->depth0;

	if (params->force_level) {
		base_level = params->force_level;
		first_level = 0;
		last_level = 0;
		width = u_minify(width, params->force_level);
		height = u_minify(height, params->force_level);
		depth = u_minify(depth, params->force_level);
	}

	pitch = surflevel[base_level].nblk_x * util_format_get_blockwidth(params->pipe_format);
	non_disp_tiling = tmp->non_disp_tiling;

	switch (surflevel[base_level].mode) {
	default:
	case RADEON_SURF_MODE_LINEAR_ALIGNED:
		array_mode = V_028C70_ARRAY_LINEAR_ALIGNED;
		break;
	case RADEON_SURF_MODE_2D:
		array_mode = V_028C70_ARRAY_2D_TILED_THIN1;
		break;
	case RADEON_SURF_MODE_1D:
		array_mode = V_028C70_ARRAY_1D_TILED_THIN1;
		break;
	}
	macro_aspect = tmp->surface.u.legacy.mtilea;
	bankw = tmp->surface.u.legacy.bankw;
	bankh = tmp->surface.u.legacy.bankh;
	tile_split = eg_tile_split(tile_split);
	macro_aspect = eg_macro_tile_aspect(macro_aspect);
	bankw = eg_bank_wh(bankw);
	bankh = eg_bank_wh(bankh);
	fmask_bankh = eg_bank_wh(tmp->fmask.bank_height);

	/* 128 bit formats require tile type = 1 */
	if (rscreen->b.gfx_level == CAYMAN) {
		if (util_format_get_blocksize(params->pipe_format) >= 16)
			non_disp_tiling = 1;
	}
	nbanks = eg_num_banks(rscreen->b.info.r600_num_banks);


	va = tmp->resource.gpu_address;

	/* array type views and views into array types need to use layer offset */
	dim = r600_tex_dim(tmp, params->target, texture->nr_samples);

	if (dim == V_030000_SQ_TEX_DIM_1D_ARRAY) {
	        height = 1;
		depth = texture->array_size;
	} else if (dim == V_030000_SQ_TEX_DIM_2D_ARRAY ||
		   dim == V_030000_SQ_TEX_DIM_2D_ARRAY_MSAA) {
		depth = texture->array_size;
	} else if (dim == V_030000_SQ_TEX_DIM_CUBEMAP)
		depth = texture->array_size / 6;

	tex_resource_words[0] = (S_030000_DIM(dim) |
				 S_030000_PITCH((pitch / 8) - 1) |
				 S_030000_TEX_WIDTH(width - 1));
	if (rscreen->b.gfx_level == CAYMAN)
		tex_resource_words[0] |= CM_S_030000_NON_DISP_TILING_ORDER(non_disp_tiling);
	else
		tex_resource_words[0] |= S_030000_NON_DISP_TILING_ORDER(non_disp_tiling);
	tex_resource_words[1] = (S_030004_TEX_HEIGHT(height - 1) |
				       S_030004_TEX_DEPTH(depth - 1) |
				       S_030004_ARRAY_MODE(array_mode));
	tex_resource_words[2] = ((uint64_t)surflevel[base_level].offset_256B * 256 + va) >> 8;

	*skip_mip_address_reloc = false;
	/* TEX_RESOURCE_WORD3.MIP_ADDRESS */
	if (texture->nr_samples > 1 && rscreen->has_compressed_msaa_texturing) {
		if (tmp->is_depth) {
			/* disable FMASK (0 = disabled) */
			tex_resource_words[3] = 0;
			*skip_mip_address_reloc = true;
		} else {
			/* FMASK should be in MIP_ADDRESS for multisample textures */
			tex_resource_words[3] = (tmp->fmask.offset + va) >> 8;
		}
	} else if (last_level && texture->nr_samples <= 1) {
		tex_resource_words[3] = ((uint64_t)surflevel[1].offset_256B * 256 + va) >> 8;
	} else {
		tex_resource_words[3] = ((uint64_t)surflevel[base_level].offset_256B * 256 + va) >> 8;
	}

	last_layer = params->last_layer;
	if (params->target != texture->target && depth == 1) {
		last_layer = params->first_layer;
	}
	tex_resource_words[4] = (word4 |
				 S_030010_ENDIAN_SWAP(endian));
	tex_resource_words[5] = S_030014_BASE_ARRAY(params->first_layer) |
		                S_030014_LAST_ARRAY(last_layer);
	tex_resource_words[6] = S_030018_TILE_SPLIT(tile_split);

	if (texture->nr_samples > 1) {
		unsigned log_samples = util_logbase2(texture->nr_samples);
		if (rscreen->b.gfx_level == CAYMAN) {
			tex_resource_words[4] |= S_030010_LOG2_NUM_FRAGMENTS(log_samples);
		}
		/* LAST_LEVEL holds log2(nr_samples) for multisample textures */
		tex_resource_words[5] |= S_030014_LAST_LEVEL(log_samples);
		tex_resource_words[6] |= S_030018_FMASK_BANK_HEIGHT(fmask_bankh);
	} else {
		bool no_mip = first_level == last_level;

		tex_resource_words[4] |= S_030010_BASE_LEVEL(first_level);
		tex_resource_words[5] |= S_030014_LAST_LEVEL(last_level);
		/* aniso max 16 samples */
		tex_resource_words[6] |= S_030018_MAX_ANISO_RATIO(no_mip ? 0 : 4);
	}

	tex_resource_words[7] = S_03001C_DATA_FORMAT(format) |
				      S_03001C_TYPE(V_03001C_SQ_TEX_VTX_VALID_TEXTURE) |
				      S_03001C_BANK_WIDTH(bankw) |
				      S_03001C_BANK_HEIGHT(bankh) |
				      S_03001C_MACRO_TILE_ASPECT(macro_aspect) |
				      S_03001C_NUM_BANKS(nbanks) |
				      S_03001C_DEPTH_SAMPLE_ORDER(tmp->db_compatible);
	return 0;
}

struct pipe_sampler_view *
evergreen_create_sampler_view_custom(struct pipe_context *ctx,
				     struct pipe_resource *texture,
				     const struct pipe_sampler_view *state,
				     unsigned width0, unsigned height0,
				     unsigned force_level)
{
	struct r600_context *rctx = (struct r600_context*)ctx;
	struct r600_pipe_sampler_view *view = CALLOC_STRUCT(r600_pipe_sampler_view);
	struct r600_texture *tmp = (struct r600_texture*)texture;
	struct eg_tex_res_params params;
	int ret;

	if (!view)
		return NULL;

	/* initialize base object */
	view->base = *state;
	view->base.texture = NULL;
	pipe_reference(NULL, &texture->reference);
	view->base.texture = texture;
	view->base.reference.count = 1;
	view->base.context = ctx;

	if (state->target == PIPE_BUFFER)
		return texture_buffer_sampler_view(rctx, view, width0, height0);

	memset(&params, 0, sizeof(params));
	params.pipe_format = state->format;
	params.force_level = force_level;
	params.width0 = width0;
	params.height0 = height0;
	params.first_level = state->u.tex.first_level;
	params.last_level = state->u.tex.last_level;
	params.first_layer = state->u.tex.first_layer;
	params.last_layer = state->u.tex.last_layer;
	params.target = state->target;
	params.swizzle[0] = state->swizzle_r;
	params.swizzle[1] = state->swizzle_g;
	params.swizzle[2] = state->swizzle_b;
	params.swizzle[3] = state->swizzle_a;

	ret = evergreen_fill_tex_resource_words(rctx, texture, &params,
						&view->skip_mip_address_reloc,
						view->tex_resource_words);
	if (ret != 0) {
		FREE(view);
		return NULL;
	}

	if (state->format == PIPE_FORMAT_X24S8_UINT ||
	    state->format == PIPE_FORMAT_S8X24_UINT ||
	    state->format == PIPE_FORMAT_X32_S8X24_UINT ||
	    state->format == PIPE_FORMAT_S8_UINT)
		view->is_stencil_sampler = true;

	view->tex_resource = &tmp->resource;

	return &view->base;
}

static struct pipe_sampler_view *
evergreen_create_sampler_view(struct pipe_context *ctx,
			      struct pipe_resource *tex,
			      const struct pipe_sampler_view *state)
{
	return evergreen_create_sampler_view_custom(ctx, tex, state,
						    tex->width0, tex->height0, 0);
}

static void evergreen_emit_config_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_config_state *a = (struct r600_config_state*)atom;

	radeon_set_config_reg_seq(cs, R_008C04_SQ_GPR_RESOURCE_MGMT_1, 3);
	if (a->dyn_gpr_enabled) {
		radeon_emit(cs, S_008C04_NUM_CLAUSE_TEMP_GPRS(rctx->r6xx_num_clause_temp_gprs));
		radeon_emit(cs, 0);
		radeon_emit(cs, 0);
	} else {
		radeon_emit(cs, a->sq_gpr_resource_mgmt_1);
		radeon_emit(cs, a->sq_gpr_resource_mgmt_2);
		radeon_emit(cs, a->sq_gpr_resource_mgmt_3);
	}
	radeon_set_config_reg(cs, R_008D8C_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ, (a->dyn_gpr_enabled << 8));
	if (a->dyn_gpr_enabled) {
		radeon_set_context_reg(cs, R_028838_SQ_DYN_GPR_RESOURCE_LIMIT_1,
				       S_028838_PS_GPRS(0x1e) |
				       S_028838_VS_GPRS(0x1e) |
				       S_028838_GS_GPRS(0x1e) |
				       S_028838_ES_GPRS(0x1e) |
				       S_028838_HS_GPRS(0x1e) |
				       S_028838_LS_GPRS(0x1e)); /* workaround for hw issues with dyn gpr - must set all limits to 240 instead of 0, 0x1e == 240 / 8*/
	}
}

static void evergreen_emit_clip_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct pipe_clip_state *state = &rctx->clip_state.state;

	radeon_set_context_reg_seq(cs, R_0285BC_PA_CL_UCP0_X, 6*4);
	radeon_emit_array(cs, (unsigned*)state, 6*4);
}

static void evergreen_set_polygon_stipple(struct pipe_context *ctx,
					 const struct pipe_poly_stipple *state)
{
}

static void evergreen_get_scissor_rect(struct r600_context *rctx,
				       unsigned tl_x, unsigned tl_y, unsigned br_x, unsigned br_y,
				       uint32_t *tl, uint32_t *br)
{
	struct pipe_scissor_state scissor = {tl_x, tl_y, br_x, br_y};

	evergreen_apply_scissor_bug_workaround(&rctx->b, &scissor);

	*tl = S_028240_TL_X(scissor.minx) | S_028240_TL_Y(scissor.miny);
	*br = S_028244_BR_X(scissor.maxx) | S_028244_BR_Y(scissor.maxy);
}

struct r600_tex_color_info {
	unsigned info;
	unsigned view;
	unsigned dim;
	unsigned pitch;
	unsigned slice;
	unsigned attrib;
	unsigned ntype;
	unsigned fmask;
	unsigned fmask_slice;
	uint64_t offset;
	bool export_16bpc;
};

static void evergreen_set_color_surface_buffer(struct r600_context *rctx,
					       struct r600_resource *res,
					       enum pipe_format pformat,
					       unsigned first_element,
					       unsigned last_element,
					       struct r600_tex_color_info *color)
{
	unsigned format, swap, ntype, endian;
	const struct util_format_description *desc;
	unsigned block_size = util_format_get_blocksize(res->b.b.format);
	unsigned pitch_alignment =
		MAX2(64, rctx->screen->b.info.pipe_interleave_bytes / block_size);
	unsigned pitch = align(res->b.b.width0, pitch_alignment);
	int i;
	unsigned width_elements;

	width_elements = last_element - first_element + 1;

	format = r600_translate_colorformat(rctx->b.gfx_level, pformat, false);
	swap = r600_translate_colorswap(pformat, false);

	endian = r600_colorformat_endian_swap(format, false);

	desc = util_format_description(pformat);
	i = util_format_get_first_non_void_channel(pformat);
	ntype = V_028C70_NUMBER_UNORM;
	if (desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB)
		ntype = V_028C70_NUMBER_SRGB;
	else if (desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED) {
		if (desc->channel[i].normalized)
			ntype = V_028C70_NUMBER_SNORM;
		else if (desc->channel[i].pure_integer)
			ntype = V_028C70_NUMBER_SINT;
	} else if (desc->channel[i].type == UTIL_FORMAT_TYPE_UNSIGNED) {
		if (desc->channel[i].normalized)
			ntype = V_028C70_NUMBER_UNORM;
		else if (desc->channel[i].pure_integer)
			ntype = V_028C70_NUMBER_UINT;
	} else if (desc->channel[i].type == UTIL_FORMAT_TYPE_FLOAT) {
		ntype = V_028C70_NUMBER_FLOAT;
	}

	pitch = (pitch / 8) - 1;
	color->pitch = S_028C64_PITCH_TILE_MAX(pitch);

	color->info = S_028C70_ARRAY_MODE(V_028C70_ARRAY_LINEAR_ALIGNED);
	color->info |= S_028C70_FORMAT(format) |
		       S_028C70_COMP_SWAP(swap) |
		       S_028C70_BLEND_CLAMP(0) |
		       S_028C70_BLEND_BYPASS(1) |
		       S_028C70_NUMBER_TYPE(ntype) |
		       S_028C70_ENDIAN(endian);
	color->attrib = S_028C74_NON_DISP_TILING_ORDER(1);
	color->ntype = ntype;
	color->export_16bpc = false;
	color->dim = width_elements - 1;
	color->slice = 0; /* (width_elements / 64) - 1;*/
	color->view = 0;
	color->offset = (res->gpu_address + first_element) >> 8;

	color->fmask = color->offset;
	color->fmask_slice = 0;
}

static void evergreen_set_color_surface_common(struct r600_context *rctx,
					       struct r600_texture *rtex,
					       unsigned level,
					       unsigned first_layer,
					       unsigned last_layer,
					       enum pipe_format pformat,
					       struct r600_tex_color_info *color)
{
	struct r600_screen *rscreen = rctx->screen;
	unsigned pitch, slice;
	unsigned non_disp_tiling, macro_aspect, tile_split, bankh, bankw, fmask_bankh, nbanks;
	unsigned format, swap, ntype, endian;
	const struct util_format_description *desc;
	bool blend_clamp = 0, blend_bypass = 0, do_endian_swap = false;
	int i;

	color->offset = (uint64_t)rtex->surface.u.legacy.level[level].offset_256B * 256;
	color->view = S_028C6C_SLICE_START(first_layer) |
			S_028C6C_SLICE_MAX(last_layer);

	color->offset += rtex->resource.gpu_address;
	color->offset >>= 8;

	color->dim = 0;
	pitch = (rtex->surface.u.legacy.level[level].nblk_x) / 8 - 1;
	slice = (rtex->surface.u.legacy.level[level].nblk_x * rtex->surface.u.legacy.level[level].nblk_y) / 64;
	if (slice) {
		slice = slice - 1;
	}

	color->info = 0;
	switch (rtex->surface.u.legacy.level[level].mode) {
	default:
	case RADEON_SURF_MODE_LINEAR_ALIGNED:
		color->info = S_028C70_ARRAY_MODE(V_028C70_ARRAY_LINEAR_ALIGNED);
		non_disp_tiling = 1;
		break;
	case RADEON_SURF_MODE_1D:
		color->info = S_028C70_ARRAY_MODE(V_028C70_ARRAY_1D_TILED_THIN1);
		non_disp_tiling = rtex->non_disp_tiling;
		break;
	case RADEON_SURF_MODE_2D:
		color->info = S_028C70_ARRAY_MODE(V_028C70_ARRAY_2D_TILED_THIN1);
		non_disp_tiling = rtex->non_disp_tiling;
		break;
	}
	tile_split = rtex->surface.u.legacy.tile_split;
	macro_aspect = rtex->surface.u.legacy.mtilea;
	bankw = rtex->surface.u.legacy.bankw;
	bankh = rtex->surface.u.legacy.bankh;
	if (rtex->fmask.size)
		fmask_bankh = rtex->fmask.bank_height;
	else
		fmask_bankh = rtex->surface.u.legacy.bankh;
	tile_split = eg_tile_split(tile_split);
	macro_aspect = eg_macro_tile_aspect(macro_aspect);
	bankw = eg_bank_wh(bankw);
	bankh = eg_bank_wh(bankh);
	fmask_bankh = eg_bank_wh(fmask_bankh);

	if (rscreen->b.gfx_level == CAYMAN) {
		if (util_format_get_blocksize(pformat) >= 16)
			non_disp_tiling = 1;
	}
	nbanks = eg_num_banks(rscreen->b.info.r600_num_banks);
	desc = util_format_description(pformat);
	i = util_format_get_first_non_void_channel(pformat);
	color->attrib = S_028C74_TILE_SPLIT(tile_split)|
		S_028C74_NUM_BANKS(nbanks) |
		S_028C74_BANK_WIDTH(bankw) |
		S_028C74_BANK_HEIGHT(bankh) |
		S_028C74_MACRO_TILE_ASPECT(macro_aspect) |
		S_028C74_NON_DISP_TILING_ORDER(non_disp_tiling) |
		S_028C74_FMASK_BANK_HEIGHT(fmask_bankh);

	if (rctx->b.gfx_level == CAYMAN) {
		color->attrib |= S_028C74_FORCE_DST_ALPHA_1(desc->swizzle[3] ==
							   PIPE_SWIZZLE_1);

		if (rtex->resource.b.b.nr_samples > 1) {
			unsigned log_samples = util_logbase2(rtex->resource.b.b.nr_samples);
			color->attrib |= S_028C74_NUM_SAMPLES(log_samples) |
					S_028C74_NUM_FRAGMENTS(log_samples);
		}
	}

	ntype = V_028C70_NUMBER_UNORM;
	if (desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB)
		ntype = V_028C70_NUMBER_SRGB;
	else if (desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED) {
		if (desc->channel[i].normalized)
			ntype = V_028C70_NUMBER_SNORM;
		else if (desc->channel[i].pure_integer)
			ntype = V_028C70_NUMBER_SINT;
	} else if (desc->channel[i].type == UTIL_FORMAT_TYPE_UNSIGNED) {
		if (desc->channel[i].normalized)
			ntype = V_028C70_NUMBER_UNORM;
		else if (desc->channel[i].pure_integer)
			ntype = V_028C70_NUMBER_UINT;
	} else if (desc->channel[i].type == UTIL_FORMAT_TYPE_FLOAT) {
		ntype = V_028C70_NUMBER_FLOAT;
	}

	if (UTIL_ARCH_BIG_ENDIAN)
		do_endian_swap = !rtex->db_compatible;

	format = r600_translate_colorformat(rctx->b.gfx_level, pformat, do_endian_swap);
	assert(format != ~0);
	swap = r600_translate_colorswap(pformat, do_endian_swap);
	assert(swap != ~0);

	endian = r600_colorformat_endian_swap(format, do_endian_swap);

	/* blend clamp should be set for all NORM/SRGB types */
	if (ntype == V_028C70_NUMBER_UNORM || ntype == V_028C70_NUMBER_SNORM ||
	    ntype == V_028C70_NUMBER_SRGB)
		blend_clamp = 1;

	/* set blend bypass according to docs if SINT/UINT or
	   8/24 COLOR variants */
	if (ntype == V_028C70_NUMBER_UINT || ntype == V_028C70_NUMBER_SINT ||
	    format == V_028C70_COLOR_8_24 || format == V_028C70_COLOR_24_8 ||
	    format == V_028C70_COLOR_X24_8_32_FLOAT) {
		blend_clamp = 0;
		blend_bypass = 1;
	}

	color->ntype = ntype;
	color->info |= S_028C70_FORMAT(format) |
		S_028C70_COMP_SWAP(swap) |
		S_028C70_BLEND_CLAMP(blend_clamp) |
		S_028C70_BLEND_BYPASS(blend_bypass) |
		S_028C70_SIMPLE_FLOAT(1) |
		S_028C70_NUMBER_TYPE(ntype) |
		S_028C70_ENDIAN(endian);

	if (rtex->fmask.size) {
		color->info |= S_028C70_COMPRESSION(1);
	}

	/* EXPORT_NORM is an optimization that can be enabled for better
	 * performance in certain cases.
	 * EXPORT_NORM can be enabled if:
	 * - 11-bit or smaller UNORM/SNORM/SRGB
	 * - 16-bit or smaller FLOAT
	 */
	color->export_16bpc = false;
	if (desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS &&
	    ((desc->channel[i].size < 12 &&
	      desc->channel[i].type != UTIL_FORMAT_TYPE_FLOAT &&
	      ntype != V_028C70_NUMBER_UINT && ntype != V_028C70_NUMBER_SINT) ||
	     (desc->channel[i].size < 17 &&
	      desc->channel[i].type == UTIL_FORMAT_TYPE_FLOAT))) {
		color->info |= S_028C70_SOURCE_FORMAT(V_028C70_EXPORT_4C_16BPC);
		color->export_16bpc = true;
	}

	color->pitch = S_028C64_PITCH_TILE_MAX(pitch);
	color->slice = S_028C68_SLICE_TILE_MAX(slice);

	if (rtex->fmask.size) {
		color->fmask = (rtex->resource.gpu_address + rtex->fmask.offset) >> 8;
		color->fmask_slice = S_028C88_TILE_MAX(rtex->fmask.slice_tile_max);
	} else {
		color->fmask = color->offset;
		color->fmask_slice = S_028C88_TILE_MAX(slice);
	}
}

/**
 * This function initializes the CB* register values for RATs.  It is meant
 * to be used for 1D aligned buffers that do not have an associated
 * radeon_surf.
 */
void evergreen_init_color_surface_rat(struct r600_context *rctx,
					struct r600_surface *surf)
{
	struct pipe_resource *pipe_buffer = surf->base.texture;
	struct r600_tex_color_info color;

	evergreen_set_color_surface_buffer(rctx, (struct r600_resource *)surf->base.texture,
					   surf->base.format, 0, pipe_buffer->width0,
					   &color);

	surf->cb_color_base = color.offset;
	surf->cb_color_dim = color.dim;
	surf->cb_color_info = color.info | S_028C70_RAT(1);
	surf->cb_color_pitch = color.pitch;
	surf->cb_color_slice = color.slice;
	surf->cb_color_view = color.view;
	surf->cb_color_attrib = color.attrib;
	surf->cb_color_fmask = color.fmask;
	surf->cb_color_fmask_slice = color.fmask_slice;

	surf->cb_color_view = 0;

	/* Set the buffer range the GPU will have access to: */
	util_range_add(pipe_buffer, &r600_resource(pipe_buffer)->valid_buffer_range,
		       0, pipe_buffer->width0);
}


void evergreen_init_color_surface(struct r600_context *rctx,
				  struct r600_surface *surf)
{
	struct r600_texture *rtex = (struct r600_texture*)surf->base.texture;
	unsigned level = surf->base.u.tex.level;
	struct r600_tex_color_info color;

	evergreen_set_color_surface_common(rctx, rtex, level,
					   surf->base.u.tex.first_layer,
					   surf->base.u.tex.last_layer,
					   surf->base.format,
					   &color);

	surf->alphatest_bypass = color.ntype == V_028C70_NUMBER_UINT ||
		color.ntype == V_028C70_NUMBER_SINT;
	surf->export_16bpc = color.export_16bpc;

	/* XXX handle enabling of CB beyond BASE8 which has different offset */
	surf->cb_color_base = color.offset;
	surf->cb_color_dim = color.dim;
	surf->cb_color_info = color.info;
	surf->cb_color_pitch = color.pitch;
	surf->cb_color_slice = color.slice;
	surf->cb_color_view = color.view;
	surf->cb_color_attrib = color.attrib;
	surf->cb_color_fmask = color.fmask;
	surf->cb_color_fmask_slice = color.fmask_slice;

	surf->color_initialized = true;
}

static void evergreen_init_depth_surface(struct r600_context *rctx,
					 struct r600_surface *surf)
{
	struct r600_screen *rscreen = rctx->screen;
	struct r600_texture *rtex = (struct r600_texture*)surf->base.texture;
	unsigned level = surf->base.u.tex.level;
	struct legacy_surf_level *levelinfo = &rtex->surface.u.legacy.level[level];
	uint64_t offset;
	unsigned format, array_mode;
	unsigned macro_aspect, tile_split, bankh, bankw, nbanks;


	format = r600_translate_dbformat(surf->base.format);
	assert(format != ~0);

	offset = rtex->resource.gpu_address;
	offset += (uint64_t)rtex->surface.u.legacy.level[level].offset_256B * 256;

	switch (rtex->surface.u.legacy.level[level].mode) {
	case RADEON_SURF_MODE_2D:
		array_mode = V_028C70_ARRAY_2D_TILED_THIN1;
		break;
	case RADEON_SURF_MODE_1D:
	case RADEON_SURF_MODE_LINEAR_ALIGNED:
	default:
		array_mode = V_028C70_ARRAY_1D_TILED_THIN1;
		break;
	}
	tile_split = rtex->surface.u.legacy.tile_split;
	macro_aspect = rtex->surface.u.legacy.mtilea;
	bankw = rtex->surface.u.legacy.bankw;
	bankh = rtex->surface.u.legacy.bankh;
	tile_split = eg_tile_split(tile_split);
	macro_aspect = eg_macro_tile_aspect(macro_aspect);
	bankw = eg_bank_wh(bankw);
	bankh = eg_bank_wh(bankh);
	nbanks = eg_num_banks(rscreen->b.info.r600_num_banks);
	offset >>= 8;

	surf->db_z_info = S_028040_ARRAY_MODE(array_mode) |
			  S_028040_FORMAT(format) |
			  S_028040_TILE_SPLIT(tile_split)|
			  S_028040_NUM_BANKS(nbanks) |
			  S_028040_BANK_WIDTH(bankw) |
			  S_028040_BANK_HEIGHT(bankh) |
			  S_028040_MACRO_TILE_ASPECT(macro_aspect);
	if (rscreen->b.gfx_level == CAYMAN && rtex->resource.b.b.nr_samples > 1) {
		surf->db_z_info |= S_028040_NUM_SAMPLES(util_logbase2(rtex->resource.b.b.nr_samples));
	}

	assert(levelinfo->nblk_x % 8 == 0 && levelinfo->nblk_y % 8 == 0);

	surf->db_depth_base = offset;
	surf->db_depth_view = S_028008_SLICE_START(surf->base.u.tex.first_layer) |
			      S_028008_SLICE_MAX(surf->base.u.tex.last_layer);
	surf->db_depth_size = S_028058_PITCH_TILE_MAX(levelinfo->nblk_x / 8 - 1) |
			      S_028058_HEIGHT_TILE_MAX(levelinfo->nblk_y / 8 - 1);
	surf->db_depth_slice = S_02805C_SLICE_TILE_MAX(levelinfo->nblk_x *
						       levelinfo->nblk_y / 64 - 1);

	if (rtex->surface.has_stencil) {
		uint64_t stencil_offset;
		unsigned stile_split = rtex->surface.u.legacy.stencil_tile_split;

		stile_split = eg_tile_split(stile_split);

		stencil_offset = (uint64_t)rtex->surface.u.legacy.zs.stencil_level[level].offset_256B * 256;
		stencil_offset += rtex->resource.gpu_address;

		surf->db_stencil_base = stencil_offset >> 8;
		surf->db_stencil_info = S_028044_FORMAT(V_028044_STENCIL_8) |
					S_028044_TILE_SPLIT(stile_split);
	} else {
		surf->db_stencil_base = offset;
		surf->db_stencil_info = S_028044_FORMAT(V_028044_STENCIL_INVALID);
	}

	if (r600_htile_enabled(rtex, level)) {
		uint64_t va = rtex->resource.gpu_address + rtex->htile_offset;
		surf->db_htile_data_base = va >> 8;
		surf->db_htile_surface = S_028ABC_HTILE_WIDTH(1) |
					 S_028ABC_HTILE_HEIGHT(1) |
					 S_028ABC_FULL_CACHE(1);
		surf->db_z_info |= S_028040_TILE_SURFACE_ENABLE(1);
		surf->db_preload_control = 0;
	}

	surf->depth_initialized = true;
}

static void evergreen_set_framebuffer_state(struct pipe_context *ctx,
					    const struct pipe_framebuffer_state *state)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_surface *surf;
	struct r600_texture *rtex;
	uint32_t i, log_samples;
	uint32_t target_mask = 0;
	/* Flush TC when changing the framebuffer state, because the only
	 * client not using TC that can change textures is the framebuffer.
	 * Other places don't typically have to flush TC.
	 */
	rctx->b.flags |= R600_CONTEXT_WAIT_3D_IDLE |
			 R600_CONTEXT_FLUSH_AND_INV |
			 R600_CONTEXT_FLUSH_AND_INV_CB |
			 R600_CONTEXT_FLUSH_AND_INV_CB_META |
			 R600_CONTEXT_FLUSH_AND_INV_DB |
			 R600_CONTEXT_FLUSH_AND_INV_DB_META |
			 R600_CONTEXT_INV_TEX_CACHE;

	util_copy_framebuffer_state(&rctx->framebuffer.state, state);

	/* Colorbuffers. */
	rctx->framebuffer.export_16bpc = state->nr_cbufs != 0;
	rctx->framebuffer.cb0_is_integer = state->nr_cbufs && state->cbufs[0] &&
					   util_format_is_pure_integer(state->cbufs[0]->format);
	rctx->framebuffer.compressed_cb_mask = 0;
	rctx->framebuffer.nr_samples = util_framebuffer_get_num_samples(state);

	for (i = 0; i < state->nr_cbufs; i++) {
		surf = (struct r600_surface*)state->cbufs[i];
		if (!surf)
			continue;

		target_mask |= (0xf << (i * 4));

		rtex = (struct r600_texture*)surf->base.texture;

		r600_context_add_resource_size(ctx, state->cbufs[i]->texture);

		if (!surf->color_initialized) {
			evergreen_init_color_surface(rctx, surf);
		}

		if (!surf->export_16bpc) {
			rctx->framebuffer.export_16bpc = false;
		}

		if (rtex->fmask.size) {
			rctx->framebuffer.compressed_cb_mask |= 1 << i;
		}
	}

	/* Update alpha-test state dependencies.
	 * Alpha-test is done on the first colorbuffer only. */
	if (state->nr_cbufs) {
		bool alphatest_bypass = false;
		bool export_16bpc = true;

		surf = (struct r600_surface*)state->cbufs[0];
		if (surf) {
			alphatest_bypass = surf->alphatest_bypass;
			export_16bpc = surf->export_16bpc;
		}

		if (rctx->alphatest_state.bypass != alphatest_bypass) {
			rctx->alphatest_state.bypass = alphatest_bypass;
			r600_mark_atom_dirty(rctx, &rctx->alphatest_state.atom);
		}
		if (rctx->alphatest_state.cb0_export_16bpc != export_16bpc) {
			rctx->alphatest_state.cb0_export_16bpc = export_16bpc;
			r600_mark_atom_dirty(rctx, &rctx->alphatest_state.atom);
		}
	}

	/* ZS buffer. */
	if (state->zsbuf) {
		surf = (struct r600_surface*)state->zsbuf;

		r600_context_add_resource_size(ctx, state->zsbuf->texture);

		if (!surf->depth_initialized) {
			evergreen_init_depth_surface(rctx, surf);
		}

		if (state->zsbuf->format != rctx->poly_offset_state.zs_format) {
			rctx->poly_offset_state.zs_format = state->zsbuf->format;
			r600_mark_atom_dirty(rctx, &rctx->poly_offset_state.atom);
		}

		if (rctx->db_state.rsurf != surf) {
			rctx->db_state.rsurf = surf;
			r600_mark_atom_dirty(rctx, &rctx->db_state.atom);
			r600_mark_atom_dirty(rctx, &rctx->db_misc_state.atom);
		}
	} else if (rctx->db_state.rsurf) {
		rctx->db_state.rsurf = NULL;
		r600_mark_atom_dirty(rctx, &rctx->db_state.atom);
		r600_mark_atom_dirty(rctx, &rctx->db_misc_state.atom);
	}

	if (rctx->cb_misc_state.nr_cbufs != state->nr_cbufs ||
	    rctx->cb_misc_state.bound_cbufs_target_mask != target_mask) {
		rctx->cb_misc_state.bound_cbufs_target_mask = target_mask;
		rctx->cb_misc_state.nr_cbufs = state->nr_cbufs;
		r600_mark_atom_dirty(rctx, &rctx->cb_misc_state.atom);
	}

	if (state->nr_cbufs == 0 && rctx->alphatest_state.bypass) {
		rctx->alphatest_state.bypass = false;
		r600_mark_atom_dirty(rctx, &rctx->alphatest_state.atom);
	}

	log_samples = util_logbase2(rctx->framebuffer.nr_samples);
	/* This is for Cayman to program SAMPLE_RATE, and for RV770 to fix a hw bug. */
	if ((rctx->b.gfx_level == CAYMAN ||
	     rctx->b.family == CHIP_RV770) &&
	    rctx->db_misc_state.log_samples != log_samples) {
		rctx->db_misc_state.log_samples = log_samples;
		r600_mark_atom_dirty(rctx, &rctx->db_misc_state.atom);
	}


	/* Calculate the CS size. */
	rctx->framebuffer.atom.num_dw = 4; /* SCISSOR */

	/* MSAA. */
	if (rctx->b.gfx_level == EVERGREEN)
		rctx->framebuffer.atom.num_dw += 17; /* Evergreen */
	else
		rctx->framebuffer.atom.num_dw += 28; /* Cayman */

	/* Colorbuffers. */
	rctx->framebuffer.atom.num_dw += state->nr_cbufs * 23;
	rctx->framebuffer.atom.num_dw += state->nr_cbufs * 2;
	rctx->framebuffer.atom.num_dw += (12 - state->nr_cbufs) * 3;

	/* ZS buffer. */
	if (state->zsbuf) {
		rctx->framebuffer.atom.num_dw += 24;
		rctx->framebuffer.atom.num_dw += 2;
	} else {
		rctx->framebuffer.atom.num_dw += 4;
	}

	r600_mark_atom_dirty(rctx, &rctx->framebuffer.atom);

	r600_set_sample_locations_constant_buffer(rctx);
	rctx->framebuffer.do_update_surf_dirtiness = true;
}

static void evergreen_set_min_samples(struct pipe_context *ctx, unsigned min_samples)
{
	struct r600_context *rctx = (struct r600_context *)ctx;

	if (rctx->ps_iter_samples == min_samples)
		return;

	rctx->ps_iter_samples = min_samples;
	if (rctx->framebuffer.nr_samples > 1) {
		r600_mark_atom_dirty(rctx, &rctx->framebuffer.atom);
	}
}

/* 8xMSAA */
static const uint32_t sample_locs_8x[] = {
	FILL_SREG(-1,  1,  1,  5,  3, -5,  5,  3),
	FILL_SREG(-7, -1, -3, -7,  7, -3, -5,  7),
	FILL_SREG(-1,  1,  1,  5,  3, -5,  5,  3),
	FILL_SREG(-7, -1, -3, -7,  7, -3, -5,  7),
	FILL_SREG(-1,  1,  1,  5,  3, -5,  5,  3),
	FILL_SREG(-7, -1, -3, -7,  7, -3, -5,  7),
	FILL_SREG(-1,  1,  1,  5,  3, -5,  5,  3),
	FILL_SREG(-7, -1, -3, -7,  7, -3, -5,  7),
};
static unsigned max_dist_8x = 7;

static void evergreen_get_sample_position(struct pipe_context *ctx,
				     unsigned sample_count,
				     unsigned sample_index,
				     float *out_value)
{
	int offset, index;
	struct {
		int idx:4;
	} val;
	switch (sample_count) {
	case 1:
	default:
		out_value[0] = out_value[1] = 0.5;
		break;
	case 2:
		offset = 4 * (sample_index * 2);
		val.idx = (eg_sample_locs_2x[0] >> offset) & 0xf;
		out_value[0] = (float)(val.idx + 8) / 16.0f;
		val.idx = (eg_sample_locs_2x[0] >> (offset + 4)) & 0xf;
		out_value[1] = (float)(val.idx + 8) / 16.0f;
		break;
	case 4:
		offset = 4 * (sample_index * 2);
		val.idx = (eg_sample_locs_4x[0] >> offset) & 0xf;
		out_value[0] = (float)(val.idx + 8) / 16.0f;
		val.idx = (eg_sample_locs_4x[0] >> (offset + 4)) & 0xf;
		out_value[1] = (float)(val.idx + 8) / 16.0f;
		break;
	case 8:
		offset = 4 * (sample_index % 4 * 2);
		index = (sample_index / 4);
		val.idx = (sample_locs_8x[index] >> offset) & 0xf;
		out_value[0] = (float)(val.idx + 8) / 16.0f;
		val.idx = (sample_locs_8x[index] >> (offset + 4)) & 0xf;
		out_value[1] = (float)(val.idx + 8) / 16.0f;
		break;
	}
}

static void evergreen_emit_msaa_state(struct r600_context *rctx, int nr_samples, int ps_iter_samples)
{

	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	unsigned max_dist = 0;

	switch (nr_samples) {
	default:
		nr_samples = 0;
		break;
	case 2:
		radeon_set_context_reg_seq(cs, R_028C1C_PA_SC_AA_SAMPLE_LOCS_0, ARRAY_SIZE(eg_sample_locs_2x));
		radeon_emit_array(cs, eg_sample_locs_2x, ARRAY_SIZE(eg_sample_locs_2x));
		max_dist = eg_max_dist_2x;
		break;
	case 4:
		radeon_set_context_reg_seq(cs, R_028C1C_PA_SC_AA_SAMPLE_LOCS_0, ARRAY_SIZE(eg_sample_locs_4x));
		radeon_emit_array(cs, eg_sample_locs_4x, ARRAY_SIZE(eg_sample_locs_4x));
		max_dist = eg_max_dist_4x;
		break;
	case 8:
		radeon_set_context_reg_seq(cs, R_028C1C_PA_SC_AA_SAMPLE_LOCS_0, ARRAY_SIZE(sample_locs_8x));
		radeon_emit_array(cs, sample_locs_8x, ARRAY_SIZE(sample_locs_8x));
		max_dist = max_dist_8x;
		break;
	}

	if (nr_samples > 1) {
		radeon_set_context_reg_seq(cs, R_028C00_PA_SC_LINE_CNTL, 2);
		radeon_emit(cs, S_028C00_LAST_PIXEL(1) |
				     S_028C00_EXPAND_LINE_WIDTH(1)); /* R_028C00_PA_SC_LINE_CNTL */
		radeon_emit(cs, S_028C04_MSAA_NUM_SAMPLES(util_logbase2(nr_samples)) |
				     S_028C04_MAX_SAMPLE_DIST(max_dist)); /* R_028C04_PA_SC_AA_CONFIG */
		radeon_set_context_reg(cs, R_028A4C_PA_SC_MODE_CNTL_1,
				       EG_S_028A4C_PS_ITER_SAMPLE(ps_iter_samples > 1) |
				       EG_S_028A4C_FORCE_EOV_CNTDWN_ENABLE(1) |
				       EG_S_028A4C_FORCE_EOV_REZ_ENABLE(1));
	} else {
		radeon_set_context_reg_seq(cs, R_028C00_PA_SC_LINE_CNTL, 2);
		radeon_emit(cs, S_028C00_LAST_PIXEL(1)); /* R_028C00_PA_SC_LINE_CNTL */
		radeon_emit(cs, 0); /* R_028C04_PA_SC_AA_CONFIG */
		radeon_set_context_reg(cs, R_028A4C_PA_SC_MODE_CNTL_1,
				       EG_S_028A4C_FORCE_EOV_CNTDWN_ENABLE(1) |
				       EG_S_028A4C_FORCE_EOV_REZ_ENABLE(1));
	}
}

static void evergreen_emit_image_state(struct r600_context *rctx, struct r600_atom *atom,
				       int immed_id_base, int res_id_base, int offset, uint32_t pkt_flags)
{
	struct r600_image_state *state = (struct r600_image_state *)atom;
	struct pipe_framebuffer_state *fb_state = &rctx->framebuffer.state;
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_texture *rtex;
	struct r600_resource *resource;
	int i;

	for (i = 0; i < R600_MAX_IMAGES; i++) {
		struct r600_image_view *image = &state->views[i];
		unsigned reloc, immed_reloc;
		int idx = i + offset;

		if (!pkt_flags)
			idx += fb_state->nr_cbufs + (rctx->dual_src_blend ? 1 : 0);
		if (!image->base.resource)
			continue;

		resource = (struct r600_resource *)image->base.resource;
		if (resource->b.b.target != PIPE_BUFFER)
			rtex = (struct r600_texture *)image->base.resource;
		else
			rtex = NULL;

		reloc = radeon_add_to_buffer_list(&rctx->b,
						  &rctx->b.gfx,
						  resource,
						  RADEON_USAGE_READWRITE |
						  RADEON_PRIO_SHADER_RW_BUFFER);

		immed_reloc = radeon_add_to_buffer_list(&rctx->b,
							&rctx->b.gfx,
							resource->immed_buffer,
							RADEON_USAGE_READWRITE |
							RADEON_PRIO_SHADER_RW_BUFFER);

		if (pkt_flags)
			radeon_compute_set_context_reg_seq(cs, R_028C60_CB_COLOR0_BASE + idx * 0x3C, 13);
		else
			radeon_set_context_reg_seq(cs, R_028C60_CB_COLOR0_BASE + idx * 0x3C, 13);

		radeon_emit(cs, image->cb_color_base);	/* R_028C60_CB_COLOR0_BASE */
		radeon_emit(cs, image->cb_color_pitch);	/* R_028C64_CB_COLOR0_PITCH */
		radeon_emit(cs, image->cb_color_slice);	/* R_028C68_CB_COLOR0_SLICE */
		radeon_emit(cs, image->cb_color_view);	/* R_028C6C_CB_COLOR0_VIEW */
		radeon_emit(cs, image->cb_color_info); /* R_028C70_CB_COLOR0_INFO */
		radeon_emit(cs, image->cb_color_attrib);	/* R_028C74_CB_COLOR0_ATTRIB */
		radeon_emit(cs, image->cb_color_dim);		/* R_028C78_CB_COLOR0_DIM */
		radeon_emit(cs, rtex ? rtex->cmask.base_address_reg : image->cb_color_base);	/* R_028C7C_CB_COLOR0_CMASK */
		radeon_emit(cs, rtex ? rtex->cmask.slice_tile_max : 0);	/* R_028C80_CB_COLOR0_CMASK_SLICE */
		radeon_emit(cs, image->cb_color_fmask);	/* R_028C84_CB_COLOR0_FMASK */
		radeon_emit(cs, image->cb_color_fmask_slice); /* R_028C88_CB_COLOR0_FMASK_SLICE */
		radeon_emit(cs, rtex ? rtex->color_clear_value[0] : 0); /* R_028C8C_CB_COLOR0_CLEAR_WORD0 */
		radeon_emit(cs, rtex ? rtex->color_clear_value[1] : 0); /* R_028C90_CB_COLOR0_CLEAR_WORD1 */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C60_CB_COLOR0_BASE */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C74_CB_COLOR0_ATTRIB */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C7C_CB_COLOR0_CMASK */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C84_CB_COLOR0_FMASK */
		radeon_emit(cs, reloc);

		if (pkt_flags)
			radeon_compute_set_context_reg(cs, R_028B9C_CB_IMMED0_BASE + (idx * 4), resource->immed_buffer->gpu_address >> 8);
		else
			radeon_set_context_reg(cs, R_028B9C_CB_IMMED0_BASE + (idx * 4), resource->immed_buffer->gpu_address >> 8);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /**/
		radeon_emit(cs, immed_reloc);

		radeon_emit(cs, PKT3(PKT3_SET_RESOURCE, 8, 0) | pkt_flags);
		radeon_emit(cs, (immed_id_base + i + offset) * 8);
		radeon_emit_array(cs, image->immed_resource_words, 8);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0) | pkt_flags);
		radeon_emit(cs, immed_reloc);

		radeon_emit(cs, PKT3(PKT3_SET_RESOURCE, 8, 0) | pkt_flags);
		radeon_emit(cs, (res_id_base + i + offset) * 8);
		radeon_emit_array(cs, image->resource_words, 8);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0) | pkt_flags);
		radeon_emit(cs, reloc);

		if (!image->skip_mip_address_reloc) {
			radeon_emit(cs, PKT3(PKT3_NOP, 0, 0) | pkt_flags);
			radeon_emit(cs, reloc);
		}
	}
}

static void evergreen_emit_fragment_image_state(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_image_state(rctx, atom,
				   R600_IMAGE_IMMED_RESOURCE_OFFSET,
				   R600_IMAGE_REAL_RESOURCE_OFFSET, 0, 0);
}

static void evergreen_emit_compute_image_state(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_image_state(rctx, atom,
				   EG_FETCH_CONSTANTS_OFFSET_CS + R600_IMAGE_IMMED_RESOURCE_OFFSET,
				   EG_FETCH_CONSTANTS_OFFSET_CS + R600_IMAGE_REAL_RESOURCE_OFFSET,
				   0, RADEON_CP_PACKET3_COMPUTE_MODE);
}

static void evergreen_emit_fragment_buffer_state(struct r600_context *rctx, struct r600_atom *atom)
{
	int offset = util_bitcount(rctx->fragment_images.enabled_mask);
	evergreen_emit_image_state(rctx, atom,
				   R600_IMAGE_IMMED_RESOURCE_OFFSET,
				   R600_IMAGE_REAL_RESOURCE_OFFSET, offset, 0);
}

static void evergreen_emit_compute_buffer_state(struct r600_context *rctx, struct r600_atom *atom)
{
	int offset = util_bitcount(rctx->compute_images.enabled_mask);
	evergreen_emit_image_state(rctx, atom,
				   EG_FETCH_CONSTANTS_OFFSET_CS + R600_IMAGE_IMMED_RESOURCE_OFFSET,
				   EG_FETCH_CONSTANTS_OFFSET_CS + R600_IMAGE_REAL_RESOURCE_OFFSET,
				   offset, RADEON_CP_PACKET3_COMPUTE_MODE);
}

static void evergreen_emit_framebuffer_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct pipe_framebuffer_state *state = &rctx->framebuffer.state;
	unsigned nr_cbufs = state->nr_cbufs;
	unsigned i, tl, br;
	struct r600_texture *tex = NULL;
	struct r600_surface *cb = NULL;

	/* XXX support more colorbuffers once we need them */
	assert(nr_cbufs <= 8);
	if (nr_cbufs > 8)
		nr_cbufs = 8;

	/* Colorbuffers. */
	for (i = 0; i < nr_cbufs; i++) {
		unsigned reloc, cmask_reloc;

		cb = (struct r600_surface*)state->cbufs[i];
		if (!cb) {
			radeon_set_context_reg(cs, R_028C70_CB_COLOR0_INFO + i * 0x3C,
					       S_028C70_FORMAT(V_028C70_COLOR_INVALID));
			continue;
		}

		tex = (struct r600_texture *)cb->base.texture;
		reloc = radeon_add_to_buffer_list(&rctx->b,
					      &rctx->b.gfx,
					      (struct r600_resource*)cb->base.texture,
					      RADEON_USAGE_READWRITE |
					      (tex->resource.b.b.nr_samples > 1 ?
						      RADEON_PRIO_COLOR_BUFFER_MSAA :
						      RADEON_PRIO_COLOR_BUFFER));

		if (tex->cmask_buffer && tex->cmask_buffer != &tex->resource) {
			cmask_reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
				tex->cmask_buffer, RADEON_USAGE_READWRITE | RADEON_PRIO_SEPARATE_META);
		} else {
			cmask_reloc = reloc;
		}

		radeon_set_context_reg_seq(cs, R_028C60_CB_COLOR0_BASE + i * 0x3C, 13);
		radeon_emit(cs, cb->cb_color_base);	/* R_028C60_CB_COLOR0_BASE */
		radeon_emit(cs, cb->cb_color_pitch);	/* R_028C64_CB_COLOR0_PITCH */
		radeon_emit(cs, cb->cb_color_slice);	/* R_028C68_CB_COLOR0_SLICE */
		radeon_emit(cs, cb->cb_color_view);	/* R_028C6C_CB_COLOR0_VIEW */
		radeon_emit(cs, cb->cb_color_info | tex->cb_color_info); /* R_028C70_CB_COLOR0_INFO */
		radeon_emit(cs, cb->cb_color_attrib);	/* R_028C74_CB_COLOR0_ATTRIB */
		radeon_emit(cs, cb->cb_color_dim);		/* R_028C78_CB_COLOR0_DIM */
		radeon_emit(cs, tex->cmask.base_address_reg);	/* R_028C7C_CB_COLOR0_CMASK */
		radeon_emit(cs, tex->cmask.slice_tile_max);	/* R_028C80_CB_COLOR0_CMASK_SLICE */
		radeon_emit(cs, cb->cb_color_fmask);	/* R_028C84_CB_COLOR0_FMASK */
		radeon_emit(cs, cb->cb_color_fmask_slice); /* R_028C88_CB_COLOR0_FMASK_SLICE */
		radeon_emit(cs, tex->color_clear_value[0]); /* R_028C8C_CB_COLOR0_CLEAR_WORD0 */
		radeon_emit(cs, tex->color_clear_value[1]); /* R_028C90_CB_COLOR0_CLEAR_WORD1 */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C60_CB_COLOR0_BASE */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C74_CB_COLOR0_ATTRIB */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C7C_CB_COLOR0_CMASK */
		radeon_emit(cs, cmask_reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C84_CB_COLOR0_FMASK */
		radeon_emit(cs, reloc);
	}
	/* set CB_COLOR1_INFO for possible dual-src blending */
	if (rctx->framebuffer.dual_src_blend && i == 1 && state->cbufs[0]) {
		radeon_set_context_reg(cs, R_028C70_CB_COLOR0_INFO + 1 * 0x3C,
				       cb->cb_color_info | tex->cb_color_info);
		i++;
	}
	i += util_bitcount(rctx->fragment_images.enabled_mask);
	i += util_bitcount(rctx->fragment_buffers.enabled_mask);
	for (; i < 8 ; i++)
		radeon_set_context_reg(cs, R_028C70_CB_COLOR0_INFO + i * 0x3C, 0);
	for (; i < 12; i++)
		radeon_set_context_reg(cs, R_028E50_CB_COLOR8_INFO + (i - 8) * 0x1C, 0);

	/* ZS buffer. */
	if (state->zsbuf) {
		struct r600_surface *zb = (struct r600_surface*)state->zsbuf;
		unsigned reloc = radeon_add_to_buffer_list(&rctx->b,
						       &rctx->b.gfx,
						       (struct r600_resource*)state->zsbuf->texture,
						       RADEON_USAGE_READWRITE |
						       (zb->base.texture->nr_samples > 1 ?
							       RADEON_PRIO_DEPTH_BUFFER_MSAA :
							       RADEON_PRIO_DEPTH_BUFFER));

		radeon_set_context_reg(cs, R_028008_DB_DEPTH_VIEW, zb->db_depth_view);

		radeon_set_context_reg_seq(cs, R_028040_DB_Z_INFO, 8);
		radeon_emit(cs, zb->db_z_info);		/* R_028040_DB_Z_INFO */
		radeon_emit(cs, zb->db_stencil_info);	/* R_028044_DB_STENCIL_INFO */
		radeon_emit(cs, zb->db_depth_base);	/* R_028048_DB_Z_READ_BASE */
		radeon_emit(cs, zb->db_stencil_base);	/* R_02804C_DB_STENCIL_READ_BASE */
		radeon_emit(cs, zb->db_depth_base);	/* R_028050_DB_Z_WRITE_BASE */
		radeon_emit(cs, zb->db_stencil_base);	/* R_028054_DB_STENCIL_WRITE_BASE */
		radeon_emit(cs, zb->db_depth_size);	/* R_028058_DB_DEPTH_SIZE */
		radeon_emit(cs, zb->db_depth_slice);	/* R_02805C_DB_DEPTH_SLICE */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028048_DB_Z_READ_BASE */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_02804C_DB_STENCIL_READ_BASE */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028050_DB_Z_WRITE_BASE */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028054_DB_STENCIL_WRITE_BASE */
		radeon_emit(cs, reloc);
	} else {
		radeon_set_context_reg_seq(cs, R_028040_DB_Z_INFO, 2);
		radeon_emit(cs, S_028040_FORMAT(V_028040_Z_INVALID)); /* R_028040_DB_Z_INFO */
		radeon_emit(cs, S_028044_FORMAT(V_028044_STENCIL_INVALID)); /* R_028044_DB_STENCIL_INFO */
	}

	/* Framebuffer dimensions. */
	evergreen_get_scissor_rect(rctx, 0, 0, state->width, state->height, &tl, &br);

	radeon_set_context_reg_seq(cs, R_028204_PA_SC_WINDOW_SCISSOR_TL, 2);
	radeon_emit(cs, tl); /* R_028204_PA_SC_WINDOW_SCISSOR_TL */
	radeon_emit(cs, br); /* R_028208_PA_SC_WINDOW_SCISSOR_BR */

	if (rctx->b.gfx_level == EVERGREEN) {
		evergreen_emit_msaa_state(rctx, rctx->framebuffer.nr_samples, rctx->ps_iter_samples);
	} else {
		cayman_emit_msaa_state(cs, rctx->framebuffer.nr_samples,
				       rctx->ps_iter_samples, 0);
	}
}

static void evergreen_emit_polygon_offset(struct r600_context *rctx, struct r600_atom *a)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_poly_offset_state *state = (struct r600_poly_offset_state*)a;
	float offset_units = state->offset_units;
	float offset_scale = state->offset_scale;
	uint32_t pa_su_poly_offset_db_fmt_cntl = 0;

	if (!state->offset_units_unscaled) {
		switch (state->zs_format) {
		case PIPE_FORMAT_Z24X8_UNORM:
		case PIPE_FORMAT_Z24_UNORM_S8_UINT:
		case PIPE_FORMAT_X8Z24_UNORM:
		case PIPE_FORMAT_S8_UINT_Z24_UNORM:
			offset_units *= 2.0f;
			pa_su_poly_offset_db_fmt_cntl =
				S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS((char)-24);
			break;
		case PIPE_FORMAT_Z16_UNORM:
			offset_units *= 4.0f;
			pa_su_poly_offset_db_fmt_cntl =
				S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS((char)-16);
			break;
		default:
			pa_su_poly_offset_db_fmt_cntl =
				S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS((char)-23) |
				S_028B78_POLY_OFFSET_DB_IS_FLOAT_FMT(1);
		}
	}

	radeon_set_context_reg_seq(cs, R_028B80_PA_SU_POLY_OFFSET_FRONT_SCALE, 4);
	radeon_emit(cs, fui(offset_scale));
	radeon_emit(cs, fui(offset_units));
	radeon_emit(cs, fui(offset_scale));
	radeon_emit(cs, fui(offset_units));

	radeon_set_context_reg(cs, R_028B78_PA_SU_POLY_OFFSET_DB_FMT_CNTL,
			       pa_su_poly_offset_db_fmt_cntl);
}

uint32_t evergreen_construct_rat_mask(struct r600_context *rctx, struct r600_cb_misc_state *a,
				      unsigned nr_cbufs)
{
	unsigned base_mask = 0;
	unsigned dirty_mask = a->image_rat_enabled_mask;
	while (dirty_mask) {
		unsigned idx = u_bit_scan(&dirty_mask);
		base_mask |= (0xf << (idx * 4));
	}
	unsigned offset = util_last_bit(a->image_rat_enabled_mask);
	dirty_mask = a->buffer_rat_enabled_mask;
	while (dirty_mask) {
		unsigned idx = u_bit_scan(&dirty_mask);
		base_mask |= (0xf << (idx + offset) * 4);
	}
	return base_mask << (nr_cbufs * 4);
}

static void evergreen_emit_cb_misc_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_cb_misc_state *a = (struct r600_cb_misc_state*)atom;
	unsigned fb_colormask = a->bound_cbufs_target_mask;
	unsigned ps_colormask = a->ps_color_export_mask;
	unsigned rat_colormask = evergreen_construct_rat_mask(rctx, a, a->nr_cbufs);
	radeon_set_context_reg_seq(cs, R_028238_CB_TARGET_MASK, 2);
	radeon_emit(cs, (a->blend_colormask & fb_colormask) | rat_colormask); /* R_028238_CB_TARGET_MASK */
	/* This must match the used export instructions exactly.
	 * Other values may lead to undefined behavior and hangs.
	 */
	radeon_emit(cs, ps_colormask); /* R_02823C_CB_SHADER_MASK */
}

static void evergreen_emit_db_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_db_state *a = (struct r600_db_state*)atom;

	if (a->rsurf && a->rsurf->db_htile_surface) {
		struct r600_texture *rtex = (struct r600_texture *)a->rsurf->base.texture;
		unsigned reloc_idx;

		radeon_set_context_reg(cs, R_02802C_DB_DEPTH_CLEAR, fui(rtex->depth_clear_value));
		radeon_set_context_reg(cs, R_028ABC_DB_HTILE_SURFACE, a->rsurf->db_htile_surface);
		radeon_set_context_reg(cs, R_028AC8_DB_PRELOAD_CONTROL, a->rsurf->db_preload_control);
		radeon_set_context_reg(cs, R_028014_DB_HTILE_DATA_BASE, a->rsurf->db_htile_data_base);
		reloc_idx = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, &rtex->resource,
						  RADEON_USAGE_READWRITE | RADEON_PRIO_SEPARATE_META);
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc_idx);
	} else {
		radeon_set_context_reg(cs, R_028ABC_DB_HTILE_SURFACE, 0);
		radeon_set_context_reg(cs, R_028AC8_DB_PRELOAD_CONTROL, 0);
	}
}

static void evergreen_emit_db_misc_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_db_misc_state *a = (struct r600_db_misc_state*)atom;
	unsigned db_render_control = 0;
	unsigned db_count_control = 0;
	unsigned db_render_override =
		S_02800C_FORCE_HIS_ENABLE0(V_02800C_FORCE_DISABLE) |
		S_02800C_FORCE_HIS_ENABLE1(V_02800C_FORCE_DISABLE);

	if (rctx->b.num_occlusion_queries > 0 &&
	    !a->occlusion_queries_disabled) {
		db_count_control |= S_028004_PERFECT_ZPASS_COUNTS(1);
		if (rctx->b.gfx_level == CAYMAN) {
			db_count_control |= S_028004_SAMPLE_RATE(a->log_samples);
		}
		db_render_override |= S_02800C_NOOP_CULL_DISABLE(1);
	} else {
		db_count_control |= S_028004_ZPASS_INCREMENT_DISABLE(1);
	}

	/* This is to fix a lockup when hyperz and alpha test are enabled at
	 * the same time somehow GPU get confuse on which order to pick for
	 * z test
	 */
	if (rctx->alphatest_state.sx_alpha_test_control)
		db_render_override |= S_02800C_FORCE_SHADER_Z_ORDER(1);

	if (a->flush_depthstencil_through_cb) {
		assert(a->copy_depth || a->copy_stencil);

		db_render_control |= S_028000_DEPTH_COPY_ENABLE(a->copy_depth) |
				     S_028000_STENCIL_COPY_ENABLE(a->copy_stencil) |
				     S_028000_COPY_CENTROID(1) |
				     S_028000_COPY_SAMPLE(a->copy_sample);
	} else if (a->flush_depth_inplace || a->flush_stencil_inplace) {
		db_render_control |= S_028000_DEPTH_COMPRESS_DISABLE(a->flush_depth_inplace) |
				     S_028000_STENCIL_COMPRESS_DISABLE(a->flush_stencil_inplace);
		db_render_override |= S_02800C_DISABLE_PIXEL_RATE_TILES(1);
	}
	if (a->htile_clear) {
		/* FIXME we might want to disable cliprect here */
		db_render_control |= S_028000_DEPTH_CLEAR_ENABLE(1);
	}

	radeon_set_context_reg_seq(cs, R_028000_DB_RENDER_CONTROL, 2);
	radeon_emit(cs, db_render_control); /* R_028000_DB_RENDER_CONTROL */
	radeon_emit(cs, db_count_control); /* R_028004_DB_COUNT_CONTROL */
	radeon_set_context_reg(cs, R_02800C_DB_RENDER_OVERRIDE, db_render_override);
	radeon_set_context_reg(cs, R_02880C_DB_SHADER_CONTROL, a->db_shader_control);
}

static void evergreen_emit_vertex_buffers(struct r600_context *rctx,
					  struct r600_vertexbuf_state *state,
					  unsigned resource_offset,
					  unsigned pkt_flags)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_fetch_shader *shader = (struct r600_fetch_shader*)rctx->vertex_fetch_shader.cso;
	uint32_t dirty_mask = state->dirty_mask & shader->buffer_mask;

	while (dirty_mask) {
		struct pipe_vertex_buffer *vb;
		struct r600_resource *rbuffer;
		uint64_t va;
		unsigned buffer_index = u_bit_scan(&dirty_mask);
		unsigned stride = pkt_flags == RADEON_CP_PACKET3_COMPUTE_MODE ?
				  1 : shader->strides[buffer_index];

		vb = &state->vb[buffer_index];
		rbuffer = (struct r600_resource*)vb->buffer.resource;
		assert(rbuffer);

		va = rbuffer->gpu_address + vb->buffer_offset;

		/* fetch resources start at index 992 */
		radeon_emit(cs, PKT3(PKT3_SET_RESOURCE, 8, 0) | pkt_flags);
		radeon_emit(cs, (resource_offset + buffer_index) * 8);
		radeon_emit(cs, va); /* RESOURCEi_WORD0 */
		radeon_emit(cs, rbuffer->b.b.width0 - vb->buffer_offset - 1); /* RESOURCEi_WORD1 */
		radeon_emit(cs, /* RESOURCEi_WORD2 */
				 S_030008_ENDIAN_SWAP(r600_endian_swap(32)) |
				 S_030008_STRIDE(stride) |
				 S_030008_BASE_ADDRESS_HI(va >> 32UL));
		radeon_emit(cs, /* RESOURCEi_WORD3 */
				 S_03000C_DST_SEL_X(V_03000C_SQ_SEL_X) |
				 S_03000C_DST_SEL_Y(V_03000C_SQ_SEL_Y) |
				 S_03000C_DST_SEL_Z(V_03000C_SQ_SEL_Z) |
				 S_03000C_DST_SEL_W(V_03000C_SQ_SEL_W));
		radeon_emit(cs, 0); /* RESOURCEi_WORD4 */
		radeon_emit(cs, 0); /* RESOURCEi_WORD5 */
		radeon_emit(cs, 0); /* RESOURCEi_WORD6 */
		radeon_emit(cs, 0xc0000000); /* RESOURCEi_WORD7 */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0) | pkt_flags);
		radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
						      RADEON_USAGE_READ | RADEON_PRIO_VERTEX_BUFFER));
	}
	state->dirty_mask &= ~shader->buffer_mask;
}

static void evergreen_fs_emit_vertex_buffers(struct r600_context *rctx, struct r600_atom * atom)
{
	evergreen_emit_vertex_buffers(rctx, &rctx->vertex_buffer_state, EG_FETCH_CONSTANTS_OFFSET_FS, 0);
}

static void evergreen_cs_emit_vertex_buffers(struct r600_context *rctx, struct r600_atom * atom)
{
	evergreen_emit_vertex_buffers(rctx, &rctx->cs_vertex_buffer_state, EG_FETCH_CONSTANTS_OFFSET_CS,
				      RADEON_CP_PACKET3_COMPUTE_MODE);
}

static void evergreen_emit_constant_buffers(struct r600_context *rctx,
					    struct r600_constbuf_state *state,
					    unsigned buffer_id_base,
					    unsigned reg_alu_constbuf_size,
					    unsigned reg_alu_const_cache,
					    unsigned pkt_flags)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint32_t dirty_mask = state->dirty_mask;

	while (dirty_mask) {
		struct pipe_constant_buffer *cb;
		struct r600_resource *rbuffer;
		uint64_t va;
		unsigned buffer_index = ffs(dirty_mask) - 1;
		unsigned gs_ring_buffer = (buffer_index == R600_GS_RING_CONST_BUFFER);

		cb = &state->cb[buffer_index];
		rbuffer = (struct r600_resource*)cb->buffer;
		assert(rbuffer);

		va = rbuffer->gpu_address + cb->buffer_offset;

		if (buffer_index < R600_MAX_ALU_CONST_BUFFERS) {
			radeon_set_context_reg_flag(cs, reg_alu_constbuf_size + buffer_index * 4,
						    DIV_ROUND_UP(cb->buffer_size, 256), pkt_flags);
			radeon_set_context_reg_flag(cs, reg_alu_const_cache + buffer_index * 4, va >> 8,
						    pkt_flags);
			radeon_emit(cs, PKT3(PKT3_NOP, 0, 0) | pkt_flags);
			radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
								  RADEON_USAGE_READ | RADEON_PRIO_CONST_BUFFER));
		}

		radeon_emit(cs, PKT3(PKT3_SET_RESOURCE, 8, 0) | pkt_flags);
		radeon_emit(cs, (buffer_id_base + buffer_index) * 8);
		radeon_emit(cs, va); /* RESOURCEi_WORD0 */
		radeon_emit(cs, cb->buffer_size -1); /* RESOURCEi_WORD1 */
		radeon_emit(cs, /* RESOURCEi_WORD2 */
			    S_030008_ENDIAN_SWAP(gs_ring_buffer ? ENDIAN_NONE : r600_endian_swap(32)) |
			    S_030008_STRIDE(gs_ring_buffer ? 4 : 16) |
			    S_030008_BASE_ADDRESS_HI(va >> 32UL) |
			    S_030008_DATA_FORMAT(FMT_32_32_32_32_FLOAT));
		radeon_emit(cs, /* RESOURCEi_WORD3 */
			         S_03000C_UNCACHED(gs_ring_buffer ? 1 : 0) |
				 S_03000C_DST_SEL_X(V_03000C_SQ_SEL_X) |
				 S_03000C_DST_SEL_Y(V_03000C_SQ_SEL_Y) |
				 S_03000C_DST_SEL_Z(V_03000C_SQ_SEL_Z) |
				 S_03000C_DST_SEL_W(V_03000C_SQ_SEL_W));
		radeon_emit(cs, 0); /* RESOURCEi_WORD4 */
		radeon_emit(cs, 0); /* RESOURCEi_WORD5 */
		radeon_emit(cs, 0); /* RESOURCEi_WORD6 */
		radeon_emit(cs, /* RESOURCEi_WORD7 */
			    S_03001C_TYPE(V_03001C_SQ_TEX_VTX_VALID_BUFFER));

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0) | pkt_flags);
		radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
						      RADEON_USAGE_READ | RADEON_PRIO_CONST_BUFFER));

		dirty_mask &= ~(1 << buffer_index);
	}
	state->dirty_mask = 0;
}

/* VS constants can be in VS/ES (same space) or LS if tess is enabled */
static void evergreen_emit_vs_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	if (rctx->vs_shader->current->shader.vs_as_ls) {
		evergreen_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_VERTEX],
						EG_FETCH_CONSTANTS_OFFSET_LS,
						R_028FC0_ALU_CONST_BUFFER_SIZE_LS_0,
						R_028F40_ALU_CONST_CACHE_LS_0,
						0 /* PKT3 flags */);
	} else {
		evergreen_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_VERTEX],
						EG_FETCH_CONSTANTS_OFFSET_VS,
						R_028180_ALU_CONST_BUFFER_SIZE_VS_0,
						R_028980_ALU_CONST_CACHE_VS_0,
						0 /* PKT3 flags */);
	}
}

static void evergreen_emit_gs_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_GEOMETRY],
					EG_FETCH_CONSTANTS_OFFSET_GS,
					R_0281C0_ALU_CONST_BUFFER_SIZE_GS_0,
					R_0289C0_ALU_CONST_CACHE_GS_0,
					0 /* PKT3 flags */);
}

static void evergreen_emit_ps_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_FRAGMENT],
					EG_FETCH_CONSTANTS_OFFSET_PS,
					R_028140_ALU_CONST_BUFFER_SIZE_PS_0,
					R_028940_ALU_CONST_CACHE_PS_0,
					0 /* PKT3 flags */);
}

static void evergreen_emit_cs_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_COMPUTE],
					EG_FETCH_CONSTANTS_OFFSET_CS,
					R_028FC0_ALU_CONST_BUFFER_SIZE_LS_0,
					R_028F40_ALU_CONST_CACHE_LS_0,
					RADEON_CP_PACKET3_COMPUTE_MODE);
}

/* tes constants can be emitted to VS or ES - which are common */
static void evergreen_emit_tes_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	if (!rctx->tes_shader)
		return;
	evergreen_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_TESS_EVAL],
					EG_FETCH_CONSTANTS_OFFSET_VS,
					R_028180_ALU_CONST_BUFFER_SIZE_VS_0,
					R_028980_ALU_CONST_CACHE_VS_0,
					0);
}

static void evergreen_emit_tcs_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	if (!rctx->tes_shader)
		return;
	evergreen_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_TESS_CTRL],
					EG_FETCH_CONSTANTS_OFFSET_HS,
					R_028F80_ALU_CONST_BUFFER_SIZE_HS_0,
					R_028F00_ALU_CONST_CACHE_HS_0,
					0);
}

void evergreen_setup_scratch_buffers(struct r600_context *rctx) {
	static const struct {
		unsigned ring_base;
		unsigned item_size;
		unsigned ring_size;
	} regs[EG_NUM_HW_STAGES] = {
		[R600_HW_STAGE_PS] = { R_008C68_SQ_PSTMP_RING_BASE, R_028914_SQ_PSTMP_RING_ITEMSIZE, R_008C6C_SQ_PSTMP_RING_SIZE },
		[R600_HW_STAGE_VS] = { R_008C60_SQ_VSTMP_RING_BASE, R_028910_SQ_VSTMP_RING_ITEMSIZE, R_008C64_SQ_VSTMP_RING_SIZE },
		[R600_HW_STAGE_GS] = { R_008C58_SQ_GSTMP_RING_BASE, R_02890C_SQ_GSTMP_RING_ITEMSIZE, R_008C5C_SQ_GSTMP_RING_SIZE },
		[R600_HW_STAGE_ES] = { R_008C50_SQ_ESTMP_RING_BASE, R_028908_SQ_ESTMP_RING_ITEMSIZE, R_008C54_SQ_ESTMP_RING_SIZE },
		[EG_HW_STAGE_LS] = { R_008E10_SQ_LSTMP_RING_BASE, R_028830_SQ_LSTMP_RING_ITEMSIZE, R_008E14_SQ_LSTMP_RING_SIZE },
		[EG_HW_STAGE_HS] = { R_008E18_SQ_HSTMP_RING_BASE, R_028834_SQ_HSTMP_RING_ITEMSIZE, R_008E1C_SQ_HSTMP_RING_SIZE }
	};

	for (unsigned i = 0; i < EG_NUM_HW_STAGES; i++) {
		struct r600_pipe_shader *stage = rctx->hw_shader_stages[i].shader;

		if (stage && unlikely(stage->scratch_space_needed)) {
			r600_setup_scratch_area_for_shader(rctx, stage,
				&rctx->scratch_buffers[i], regs[i].ring_base, regs[i].item_size, regs[i].ring_size);
		}
	}
}

static void evergreen_emit_sampler_views(struct r600_context *rctx,
					 struct r600_samplerview_state *state,
					 unsigned resource_id_base, unsigned pkt_flags)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint32_t dirty_mask = state->dirty_mask;

	while (dirty_mask) {
		struct r600_pipe_sampler_view *rview;
		unsigned resource_index = u_bit_scan(&dirty_mask);
		unsigned reloc;

		rview = state->views[resource_index];
		assert(rview);

		radeon_emit(cs, PKT3(PKT3_SET_RESOURCE, 8, 0) | pkt_flags);
		radeon_emit(cs, (resource_id_base + resource_index) * 8);
		radeon_emit_array(cs, rview->tex_resource_words, 8);

		reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rview->tex_resource,
					      RADEON_USAGE_READ |
					      r600_get_sampler_view_priority(rview->tex_resource));
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0) | pkt_flags);
		radeon_emit(cs, reloc);

		if (!rview->skip_mip_address_reloc) {
			radeon_emit(cs, PKT3(PKT3_NOP, 0, 0) | pkt_flags);
			radeon_emit(cs, reloc);
		}
	}
	state->dirty_mask = 0;
}

static void evergreen_emit_vs_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	if (rctx->vs_shader->current->shader.vs_as_ls) {
		evergreen_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_VERTEX].views,
					     EG_FETCH_CONSTANTS_OFFSET_LS + R600_MAX_CONST_BUFFERS, 0);
	} else {
		evergreen_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_VERTEX].views,
					     EG_FETCH_CONSTANTS_OFFSET_VS + R600_MAX_CONST_BUFFERS, 0);
	}
}

static void evergreen_emit_gs_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_GEOMETRY].views,
	                             EG_FETCH_CONSTANTS_OFFSET_GS + R600_MAX_CONST_BUFFERS, 0);
}

static void evergreen_emit_tcs_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_TESS_CTRL].views,
	                             EG_FETCH_CONSTANTS_OFFSET_HS + R600_MAX_CONST_BUFFERS, 0);
}

static void evergreen_emit_tes_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	if (!rctx->tes_shader)
		return;
	evergreen_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_TESS_EVAL].views,
	                             EG_FETCH_CONSTANTS_OFFSET_VS + R600_MAX_CONST_BUFFERS, 0);
}

static void evergreen_emit_ps_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_FRAGMENT].views,
	                             EG_FETCH_CONSTANTS_OFFSET_PS + R600_MAX_CONST_BUFFERS, 0);
}

static void evergreen_emit_cs_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_COMPUTE].views,
	                             EG_FETCH_CONSTANTS_OFFSET_CS + R600_MAX_CONST_BUFFERS, RADEON_CP_PACKET3_COMPUTE_MODE);
}

static void cayman_convert_border_color(union pipe_color_union *in,
                                        union pipe_color_union *out,
                                        struct pipe_sampler_view *view)
{
   enum  pipe_format format = view->format;
   const struct util_format_description *d = util_format_description(format);

   if ((!util_format_is_alpha(format) &&
        !util_format_is_luminance(format) &&
        !util_format_is_luminance_alpha(format) &&
        !util_format_is_intensity(format) &&
        //!util_format_is_depth_or_stencil(format) &&
        (format != PIPE_FORMAT_RGTC1_SNORM) &&
        (format != PIPE_FORMAT_RGTC1_UNORM) &&
        (format != PIPE_FORMAT_RGTC2_SNORM) &&
        (format != PIPE_FORMAT_RGTC2_UNORM) &&
        !(d->channel[0].size < 8) &&
        (d->nr_channels > 2)) ||
       (util_format_is_srgb(format) ||
        util_format_is_s3tc(format))
       ) {
                const float values[PIPE_SWIZZLE_MAX] = {
                   in->f[0], in->f[1], in->f[2], in->f[3], 0.0f, 1.0f, 0.0f /* none */
                };

                STATIC_ASSERT(PIPE_SWIZZLE_0 == 4);
                STATIC_ASSERT(PIPE_SWIZZLE_1 == 5);
                STATIC_ASSERT(PIPE_SWIZZLE_NONE == 6);
                STATIC_ASSERT(PIPE_SWIZZLE_MAX == 7);

                out->f[0] = values[view->swizzle_r];
                out->f[1] = values[view->swizzle_g];
                out->f[2] = values[view->swizzle_b];
                out->f[3] = values[view->swizzle_a];
   } else {
      memcpy(out->f, in->f, 4 * sizeof(float));
   }
}

static void evergreen_convert_border_color(union pipe_color_union *in,
                                           union pipe_color_union *out,
                                           struct pipe_sampler_view *view)
{
   enum  pipe_format format = view->format;
   const struct util_format_description *d = util_format_description(format);

   int swizzle[4] = { view->swizzle_r, view->swizzle_g, view->swizzle_b,
                      view->swizzle_a };

   bool is_lai = util_format_is_alpha(format) ||
                 util_format_is_luminance(format) ||
                 util_format_is_luminance_alpha(format) ||
                 util_format_is_intensity(format) ||
                 d->channel[0].size < 8;

   if (is_lai) {
         for (int i = 0; i < 4; ++i) {
            swizzle[i] = i;
         }
   }

   if (!util_format_is_depth_or_stencil(format)) {

      for (int i = 0; i < 4; ++i) {

         if (swizzle[i] == 4) {
            out->f[i] = 0.0f;
            continue;
         }

         if (swizzle[i] == 5) {
            out->f[i] = 1.0f;
            continue;
         }

         if (util_format_is_pure_integer(format)) {
            int cs = d->channel[d->swizzle[i]].size;
            if (d->channel[d->swizzle[i]].type == UTIL_FORMAT_TYPE_SIGNED)
               out->f[i] = ((double)(in->i[swizzle[i]])) / ((1ul << (cs - 1)) - 1 );
            else if (d->channel[d->swizzle[i]].type == UTIL_FORMAT_TYPE_UNSIGNED)
               out->f[i] = ((double)(in->ui[swizzle[i]])) / ((1ul << cs) - 1 );
            else
               out->f[i] = 0;
         } else {
            out->f[i] = in->f[swizzle[i]];
         }
      }

   } else {
		switch (format) {
		case PIPE_FORMAT_X24S8_UINT:
		case PIPE_FORMAT_X32_S8X24_UINT:
			out->f[0] = (double)(in->ui[0]) / 255.0;
			out->f[1] = out->f[2] = out->f[3] = 0.0f;
			break;
		default:
			memcpy(out->f, in->f, 4 * sizeof(float));
		}
	}
}

static void evergreen_emit_sampler_states(struct r600_context *rctx,
				struct r600_textures_info *texinfo,
				unsigned resource_id_base,
				unsigned border_index_reg,
				unsigned pkt_flags)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint32_t dirty_mask = texinfo->states.dirty_mask;
	union pipe_color_union border_color = {{0,0,0,1}};
	union pipe_color_union *border_color_ptr = &border_color;

	while (dirty_mask) {
		struct r600_pipe_sampler_state *rstate;
		unsigned i = u_bit_scan(&dirty_mask);

		rstate = texinfo->states.states[i];
		assert(rstate);

		if (rstate->border_color_use) {
			struct r600_pipe_sampler_view	*rview = texinfo->views.views[i];
         if (rview) {
            if (rctx->b.gfx_level < CAYMAN) {
               evergreen_convert_border_color(&rstate->border_color,
                                              &border_color, &rview->base);
            } else {
               cayman_convert_border_color(&rstate->border_color,
                                           &border_color, &rview->base);
            }
         } else {
            border_color_ptr = &rstate->border_color;
			}
		}

		radeon_emit(cs, PKT3(PKT3_SET_SAMPLER, 3, 0) | pkt_flags);
		radeon_emit(cs, (resource_id_base + i) * 3);
		radeon_emit_array(cs, rstate->tex_sampler_words, 3);

		if (rstate->border_color_use) {
			radeon_set_config_reg_seq(cs, border_index_reg, 5);
			radeon_emit(cs, i);
			radeon_emit_array(cs, border_color_ptr->ui, 4);
		}
	}
	texinfo->states.dirty_mask = 0;
}

static void evergreen_emit_vs_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	if (rctx->vs_shader->current->shader.vs_as_ls) {
		evergreen_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_VERTEX], 72,
					      R_00A450_TD_LS_SAMPLER0_BORDER_COLOR_INDEX, 0);
	} else {
		evergreen_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_VERTEX], 18,
					      R_00A414_TD_VS_SAMPLER0_BORDER_INDEX, 0);
	}
}

static void evergreen_emit_gs_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_GEOMETRY], 36,
	                              R_00A428_TD_GS_SAMPLER0_BORDER_INDEX, 0);
}

static void evergreen_emit_tcs_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_TESS_CTRL], 54,
	                              R_00A43C_TD_HS_SAMPLER0_BORDER_COLOR_INDEX, 0);
}

static void evergreen_emit_tes_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	if (!rctx->tes_shader)
		return;
	evergreen_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_TESS_EVAL], 18,
				      R_00A414_TD_VS_SAMPLER0_BORDER_INDEX, 0);
}

static void evergreen_emit_ps_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_FRAGMENT], 0,
	                              R_00A400_TD_PS_SAMPLER0_BORDER_INDEX, 0);
}

static void evergreen_emit_cs_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	evergreen_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_COMPUTE], 90,
	                              R_00A464_TD_CS_SAMPLER0_BORDER_INDEX,
	                              RADEON_CP_PACKET3_COMPUTE_MODE);
}

static void evergreen_emit_sample_mask(struct r600_context *rctx, struct r600_atom *a)
{
	struct r600_sample_mask *s = (struct r600_sample_mask*)a;
	uint8_t mask = s->sample_mask;

	radeon_set_context_reg(&rctx->b.gfx.cs, R_028C3C_PA_SC_AA_MASK,
			       mask | (mask << 8) | (mask << 16) | (mask << 24));
}

static void cayman_emit_sample_mask(struct r600_context *rctx, struct r600_atom *a)
{
	struct r600_sample_mask *s = (struct r600_sample_mask*)a;
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint16_t mask = s->sample_mask;

	radeon_set_context_reg_seq(cs, CM_R_028C38_PA_SC_AA_MASK_X0Y0_X1Y0, 2);
	radeon_emit(cs, mask | (mask << 16)); /* X0Y0_X1Y0 */
	radeon_emit(cs, mask | (mask << 16)); /* X0Y1_X1Y1 */
}

static void evergreen_emit_vertex_fetch_shader(struct r600_context *rctx, struct r600_atom *a)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_cso_state *state = (struct r600_cso_state*)a;
	struct r600_fetch_shader *shader = (struct r600_fetch_shader*)state->cso;

	if (!shader)
		return;

	radeon_set_context_reg(cs, R_0288A4_SQ_PGM_START_FS,
			       (shader->buffer->gpu_address + shader->offset) >> 8);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, shader->buffer,
                                                  RADEON_USAGE_READ |
                                                  RADEON_PRIO_SHADER_BINARY));
}

static void evergreen_emit_shader_stages(struct r600_context *rctx, struct r600_atom *a)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_shader_stages_state *state = (struct r600_shader_stages_state*)a;

	uint32_t v = 0, v2 = 0, primid = 0, tf_param = 0;

	if (rctx->vs_shader->current->shader.vs_as_gs_a) {
		v2 = S_028A40_MODE(V_028A40_GS_SCENARIO_A);
		primid = 1;
	}

	if (state->geom_enable) {
		uint32_t cut_val;

		if (rctx->gs_shader->gs_max_out_vertices <= 128)
			cut_val = V_028A40_GS_CUT_128;
		else if (rctx->gs_shader->gs_max_out_vertices <= 256)
			cut_val = V_028A40_GS_CUT_256;
		else if (rctx->gs_shader->gs_max_out_vertices <= 512)
			cut_val = V_028A40_GS_CUT_512;
		else
			cut_val = V_028A40_GS_CUT_1024;

		v = S_028B54_GS_EN(1) |
		    S_028B54_VS_EN(V_028B54_VS_STAGE_COPY_SHADER);
		if (!rctx->tes_shader)
			v |= S_028B54_ES_EN(V_028B54_ES_STAGE_REAL);

		v2 = S_028A40_MODE(V_028A40_GS_SCENARIO_G) |
			S_028A40_CUT_MODE(cut_val);

		if (rctx->gs_shader->current->shader.gs_prim_id_input)
			primid = 1;
	}

	if (rctx->tes_shader) {
		uint32_t type, partitioning, topology;
		struct tgsi_shader_info *info = &rctx->tes_shader->current->selector->info;
		unsigned tes_prim_mode = info->properties[TGSI_PROPERTY_TES_PRIM_MODE];
		unsigned tes_spacing = info->properties[TGSI_PROPERTY_TES_SPACING];
		bool tes_vertex_order_cw = info->properties[TGSI_PROPERTY_TES_VERTEX_ORDER_CW];
		bool tes_point_mode = info->properties[TGSI_PROPERTY_TES_POINT_MODE];
		switch (tes_prim_mode) {
		case MESA_PRIM_LINES:
			type = V_028B6C_TESS_ISOLINE;
			break;
		case MESA_PRIM_TRIANGLES:
			type = V_028B6C_TESS_TRIANGLE;
			break;
		case MESA_PRIM_QUADS:
			type = V_028B6C_TESS_QUAD;
			break;
		default:
			assert(0);
			return;
		}

		switch (tes_spacing) {
		case PIPE_TESS_SPACING_FRACTIONAL_ODD:
			partitioning = V_028B6C_PART_FRAC_ODD;
			break;
		case PIPE_TESS_SPACING_FRACTIONAL_EVEN:
			partitioning = V_028B6C_PART_FRAC_EVEN;
			break;
		case PIPE_TESS_SPACING_EQUAL:
			partitioning = V_028B6C_PART_INTEGER;
			break;
		default:
			assert(0);
			return;
		}

		if (tes_point_mode)
			topology = V_028B6C_OUTPUT_POINT;
		else if (tes_prim_mode == MESA_PRIM_LINES)
			topology = V_028B6C_OUTPUT_LINE;
		else if (tes_vertex_order_cw)
			/* XXX follow radeonsi and invert */
			topology = V_028B6C_OUTPUT_TRIANGLE_CCW;
		else
			topology = V_028B6C_OUTPUT_TRIANGLE_CW;

		tf_param = S_028B6C_TYPE(type) |
			S_028B6C_PARTITIONING(partitioning) |
			S_028B6C_TOPOLOGY(topology);
	}

	if (rctx->tes_shader) {
		v |= S_028B54_LS_EN(V_028B54_LS_STAGE_ON) |
		     S_028B54_HS_EN(1);
		if (!state->geom_enable)
			v |= S_028B54_VS_EN(V_028B54_VS_STAGE_DS);
		else
			v |= S_028B54_ES_EN(V_028B54_ES_STAGE_DS);
	}

	radeon_set_context_reg(cs, R_028AB8_VGT_VTX_CNT_EN, v ? 1 : 0 );
	radeon_set_context_reg(cs, R_028B54_VGT_SHADER_STAGES_EN, v);
	radeon_set_context_reg(cs, R_028A40_VGT_GS_MODE, v2);
	radeon_set_context_reg(cs, R_028A84_VGT_PRIMITIVEID_EN, primid);
	radeon_set_context_reg(cs, R_028B6C_VGT_TF_PARAM, tf_param);
}

static void evergreen_emit_gs_rings(struct r600_context *rctx, struct r600_atom *a)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_gs_rings_state *state = (struct r600_gs_rings_state*)a;
	struct r600_resource *rbuffer;

	radeon_set_config_reg(cs, R_008040_WAIT_UNTIL, S_008040_WAIT_3D_IDLE(1));
	radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
	radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_VGT_FLUSH));

	if (state->enable) {
		rbuffer =(struct r600_resource*)state->esgs_ring.buffer;
		radeon_set_config_reg(cs, R_008C40_SQ_ESGS_RING_BASE,
				rbuffer->gpu_address >> 8);
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
						      RADEON_USAGE_READWRITE |
						      RADEON_PRIO_SHADER_RINGS));
		radeon_set_config_reg(cs, R_008C44_SQ_ESGS_RING_SIZE,
				state->esgs_ring.buffer_size >> 8);

		rbuffer =(struct r600_resource*)state->gsvs_ring.buffer;
		radeon_set_config_reg(cs, R_008C48_SQ_GSVS_RING_BASE,
				rbuffer->gpu_address >> 8);
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
						      RADEON_USAGE_READWRITE |
						      RADEON_PRIO_SHADER_RINGS));
		radeon_set_config_reg(cs, R_008C4C_SQ_GSVS_RING_SIZE,
				state->gsvs_ring.buffer_size >> 8);
	} else {
		radeon_set_config_reg(cs, R_008C44_SQ_ESGS_RING_SIZE, 0);
		radeon_set_config_reg(cs, R_008C4C_SQ_GSVS_RING_SIZE, 0);
	}

	radeon_set_config_reg(cs, R_008040_WAIT_UNTIL, S_008040_WAIT_3D_IDLE(1));
	radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
	radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_VGT_FLUSH));
}

void cayman_init_common_regs(struct r600_command_buffer *cb,
			     enum amd_gfx_level gfx_level,
			     enum radeon_family ctx_family,
			     int ctx_drm_minor)
{
	r600_store_config_reg_seq(cb, R_008C00_SQ_CONFIG, 2);
	r600_store_value(cb, S_008C00_EXPORT_SRC_C(1)); /* R_008C00_SQ_CONFIG */
	/* always set the temp clauses */
	r600_store_value(cb, S_008C04_NUM_CLAUSE_TEMP_GPRS(4)); /* R_008C04_SQ_GPR_RESOURCE_MGMT_1 */

	r600_store_config_reg_seq(cb, R_008C10_SQ_GLOBAL_GPR_RESOURCE_MGMT_1, 2);
	r600_store_value(cb, 0); /* R_008C10_SQ_GLOBAL_GPR_RESOURCE_MGMT_1 */
	r600_store_value(cb, 0); /* R_008C14_SQ_GLOBAL_GPR_RESOURCE_MGMT_2 */

	r600_store_config_reg(cb, R_008D8C_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ, (1 << 8));

	r600_store_context_reg_seq(cb, R_028350_SX_MISC, 2);
	r600_store_value(cb, 0);
	r600_store_value(cb, S_028354_SURFACE_SYNC_MASK(0xf));

	r600_store_context_reg(cb, R_028800_DB_DEPTH_CONTROL, 0);
}

static void cayman_init_atom_start_cs(struct r600_context *rctx)
{
	struct r600_command_buffer *cb = &rctx->start_cs_cmd;
	int i;

	r600_init_command_buffer(cb, 338);

	/* This must be first. */
	r600_store_value(cb, PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
	r600_store_value(cb, 0x80000000);
	r600_store_value(cb, 0x80000000);

	/* We're setting config registers here. */
	r600_store_value(cb, PKT3(PKT3_EVENT_WRITE, 0, 0));
	r600_store_value(cb, EVENT_TYPE(EVENT_TYPE_PS_PARTIAL_FLUSH) | EVENT_INDEX(4));

	/* This enables pipeline stat & streamout queries.
	 * They are only disabled by blits.
	 */
	r600_store_value(cb, PKT3(PKT3_EVENT_WRITE, 0, 0));
	r600_store_value(cb, EVENT_TYPE(EVENT_TYPE_PIPELINESTAT_START) | EVENT_INDEX(0));

	cayman_init_common_regs(cb, rctx->b.gfx_level,
				rctx->b.family, rctx->screen->b.info.drm_minor);

	r600_store_config_reg(cb, R_009100_SPI_CONFIG_CNTL, 0);
	r600_store_config_reg(cb, R_00913C_SPI_CONFIG_CNTL_1, S_00913C_VTX_DONE_DELAY(4));

	/* remove LS/HS from one SIMD for hw workaround */
	r600_store_config_reg_seq(cb, R_008E20_SQ_STATIC_THREAD_MGMT1, 3);
	r600_store_value(cb, 0xffffffff);
	r600_store_value(cb, 0xffffffff);
	r600_store_value(cb, 0xfffffffe);

	r600_store_context_reg_seq(cb, R_028900_SQ_ESGS_RING_ITEMSIZE, 6);
	r600_store_value(cb, 0); /* R_028900_SQ_ESGS_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028904_SQ_GSVS_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028908_SQ_ESTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_02890C_SQ_GSTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028910_SQ_VSTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028914_SQ_PSTMP_RING_ITEMSIZE */

	r600_store_context_reg_seq(cb, R_02891C_SQ_GS_VERT_ITEMSIZE, 4);
	r600_store_value(cb, 0); /* R_02891C_SQ_GS_VERT_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028920_SQ_GS_VERT_ITEMSIZE_1 */
	r600_store_value(cb, 0); /* R_028924_SQ_GS_VERT_ITEMSIZE_2 */
	r600_store_value(cb, 0); /* R_028928_SQ_GS_VERT_ITEMSIZE_3 */

	r600_store_context_reg_seq(cb, R_028A10_VGT_OUTPUT_PATH_CNTL, 13);
	r600_store_value(cb, 0); /* R_028A10_VGT_OUTPUT_PATH_CNTL */
	r600_store_value(cb, 0); /* R_028A14_VGT_HOS_CNTL */
	r600_store_value(cb, fui(64)); /* R_028A18_VGT_HOS_MAX_TESS_LEVEL */
	r600_store_value(cb, fui(0)); /* R_028A1C_VGT_HOS_MIN_TESS_LEVEL */
	r600_store_value(cb, 16); /* R_028A20_VGT_HOS_REUSE_DEPTH */
	r600_store_value(cb, 0); /* R_028A24_VGT_GROUP_PRIM_TYPE */
	r600_store_value(cb, 0); /* R_028A28_VGT_GROUP_FIRST_DECR */
	r600_store_value(cb, 0); /* R_028A2C_VGT_GROUP_DECR */
	r600_store_value(cb, 0); /* R_028A30_VGT_GROUP_VECT_0_CNTL */
	r600_store_value(cb, 0); /* R_028A34_VGT_GROUP_VECT_1_CNTL */
	r600_store_value(cb, 0); /* R_028A38_VGT_GROUP_VECT_0_FMT_CNTL */
	r600_store_value(cb, 0); /* R_028A3C_VGT_GROUP_VECT_1_FMT_CNTL */
	r600_store_value(cb, 0); /* R_028A40_VGT_GS_MODE */

	r600_store_context_reg(cb, R_028B98_VGT_STRMOUT_BUFFER_CONFIG, 0);

	r600_store_config_reg(cb, R_008A14_PA_CL_ENHANCE, (3 << 1) | 1);

	r600_store_context_reg_seq(cb, CM_R_028BD4_PA_SC_CENTROID_PRIORITY_0, 2);
	r600_store_value(cb, 0x76543210); /* CM_R_028BD4_PA_SC_CENTROID_PRIORITY_0 */
	r600_store_value(cb, 0xfedcba98); /* CM_R_028BD8_PA_SC_CENTROID_PRIORITY_1 */

	r600_store_context_reg(cb, R_028724_GDS_ADDR_SIZE, 0x3fff);
	r600_store_context_reg_seq(cb, R_0288E8_SQ_LDS_ALLOC, 2);
	r600_store_value(cb, 0); /* R_0288E8_SQ_LDS_ALLOC */
	r600_store_value(cb, 0); /* R_0288EC_SQ_LDS_ALLOC_PS */

        r600_store_context_reg(cb, R_0288F0_SQ_VTX_SEMANTIC_CLEAR, ~0);

        r600_store_context_reg_seq(cb, R_028400_VGT_MAX_VTX_INDX, 2);
	r600_store_value(cb, ~0); /* R_028400_VGT_MAX_VTX_INDX */
	r600_store_value(cb, 0); /* R_028404_VGT_MIN_VTX_INDX */

	r600_store_ctl_const(cb, R_03CFF0_SQ_VTX_BASE_VTX_LOC, 0);

	r600_store_context_reg(cb, R_028028_DB_STENCIL_CLEAR, 0);

	r600_store_context_reg(cb, R_0286DC_SPI_FOG_CNTL, 0);

	r600_store_context_reg_seq(cb, R_028AC0_DB_SRESULTS_COMPARE_STATE0, 3);
	r600_store_value(cb, 0); /* R_028AC0_DB_SRESULTS_COMPARE_STATE0 */
	r600_store_value(cb, 0); /* R_028AC4_DB_SRESULTS_COMPARE_STATE1 */
	r600_store_value(cb, 0); /* R_028AC8_DB_PRELOAD_CONTROL */

	r600_store_context_reg(cb, R_028200_PA_SC_WINDOW_OFFSET, 0);
	r600_store_context_reg(cb, R_02820C_PA_SC_CLIPRECT_RULE, 0xFFFF);

	r600_store_context_reg(cb, R_028230_PA_SC_EDGERULE, 0xAAAAAAAA);
	r600_store_context_reg(cb, R_028820_PA_CL_NANINF_CNTL, 0);

	r600_store_context_reg_seq(cb, R_028240_PA_SC_GENERIC_SCISSOR_TL, 2);
	r600_store_value(cb, 0); /* R_028240_PA_SC_GENERIC_SCISSOR_TL */
	r600_store_value(cb, S_028244_BR_X(16384) | S_028244_BR_Y(16384)); /* R_028244_PA_SC_GENERIC_SCISSOR_BR */

	r600_store_context_reg_seq(cb, R_028030_PA_SC_SCREEN_SCISSOR_TL, 2);
	r600_store_value(cb, 0); /* R_028030_PA_SC_SCREEN_SCISSOR_TL */
	r600_store_value(cb, S_028034_BR_X(16384) | S_028034_BR_Y(16384)); /* R_028034_PA_SC_SCREEN_SCISSOR_BR */

	r600_store_context_reg(cb, R_028848_SQ_PGM_RESOURCES_2_PS, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_028864_SQ_PGM_RESOURCES_2_VS, S_028864_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_02887C_SQ_PGM_RESOURCES_2_GS, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_028894_SQ_PGM_RESOURCES_2_ES, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_0288C0_SQ_PGM_RESOURCES_2_HS, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_0288D8_SQ_PGM_RESOURCES_2_LS, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));

	r600_store_context_reg(cb, R_0288A8_SQ_PGM_RESOURCES_FS, 0);

	/* to avoid GPU doing any preloading of constant from random address */
	r600_store_context_reg_seq(cb, R_028140_ALU_CONST_BUFFER_SIZE_PS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg_seq(cb, R_028180_ALU_CONST_BUFFER_SIZE_VS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg_seq(cb, R_0281C0_ALU_CONST_BUFFER_SIZE_GS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg_seq(cb, R_028FC0_ALU_CONST_BUFFER_SIZE_LS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg_seq(cb, R_028F80_ALU_CONST_BUFFER_SIZE_HS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	if (rctx->screen->b.has_streamout) {
		r600_store_context_reg(cb, R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET, 0);
	}

	r600_store_context_reg(cb, R_028010_DB_RENDER_OVERRIDE2, 0);
	r600_store_context_reg(cb, R_028234_PA_SU_HARDWARE_SCREEN_OFFSET, 0);
	r600_store_context_reg(cb, R_0286C8_SPI_THREAD_GROUPING, 0);
	r600_store_context_reg_seq(cb, R_0286E4_SPI_PS_IN_CONTROL_2, 2);
	r600_store_value(cb, 0); /* R_0286E4_SPI_PS_IN_CONTROL_2 */
	r600_store_value(cb, 0); /* R_0286E8_SPI_COMPUTE_INPUT_CNTL */

	r600_store_context_reg_seq(cb, R_028B54_VGT_SHADER_STAGES_EN, 2);
	r600_store_value(cb, 0); /* R028B54_VGT_SHADER_STAGES_EN */
	r600_store_value(cb, 0); /* R028B58_VGT_LS_HS_CONFIG */
	r600_store_context_reg(cb, R_028B6C_VGT_TF_PARAM, 0);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0, 0x01000FFF);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (32 * 4), 0x01000FFF);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (64 * 4), 0x01000FFF);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (96 * 4), 0x01000FFF);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (128 * 4), 0x01000FFF);
}

void evergreen_init_common_regs(struct r600_context *rctx, struct r600_command_buffer *cb,
				enum amd_gfx_level gfx_level,
				enum radeon_family ctx_family,
				int ctx_drm_minor)
{
	int ps_prio;
	int vs_prio;
	int gs_prio;
	int es_prio;

	int hs_prio;
	int cs_prio;
	int ls_prio;

	unsigned tmp;

	ps_prio = 0;
	vs_prio = 1;
	gs_prio = 2;
	es_prio = 3;
	hs_prio = 3;
	ls_prio = 3;
	cs_prio = 0;

	rctx->default_gprs[R600_HW_STAGE_PS] = 93;
	rctx->default_gprs[R600_HW_STAGE_VS] = 46;
	rctx->r6xx_num_clause_temp_gprs = 4;
	rctx->default_gprs[R600_HW_STAGE_GS] = 31;
	rctx->default_gprs[R600_HW_STAGE_ES] = 31;
	rctx->default_gprs[EG_HW_STAGE_HS] = 23;
	rctx->default_gprs[EG_HW_STAGE_LS] = 23;

	tmp = 0;
	switch (ctx_family) {
	case CHIP_CEDAR:
	case CHIP_PALM:
	case CHIP_SUMO:
	case CHIP_SUMO2:
	case CHIP_CAICOS:
		break;
	default:
		tmp |= S_008C00_VC_ENABLE(1);
		break;
	}
	tmp |= S_008C00_EXPORT_SRC_C(1);
	tmp |= S_008C00_CS_PRIO(cs_prio);
	tmp |= S_008C00_LS_PRIO(ls_prio);
	tmp |= S_008C00_HS_PRIO(hs_prio);
	tmp |= S_008C00_PS_PRIO(ps_prio);
	tmp |= S_008C00_VS_PRIO(vs_prio);
	tmp |= S_008C00_GS_PRIO(gs_prio);
	tmp |= S_008C00_ES_PRIO(es_prio);

	r600_store_config_reg_seq(cb, R_008C00_SQ_CONFIG, 1);
	r600_store_value(cb, tmp); /* R_008C00_SQ_CONFIG */

	r600_store_config_reg_seq(cb, R_008C10_SQ_GLOBAL_GPR_RESOURCE_MGMT_1, 2);
	r600_store_value(cb, 0); /* R_008C10_SQ_GLOBAL_GPR_RESOURCE_MGMT_1 */
	r600_store_value(cb, 0); /* R_008C14_SQ_GLOBAL_GPR_RESOURCE_MGMT_2 */

	/* The cs checker requires this register to be set. */
	r600_store_context_reg(cb, R_028800_DB_DEPTH_CONTROL, 0);

	r600_store_context_reg_seq(cb, R_028350_SX_MISC, 2);
	r600_store_value(cb, 0);
	r600_store_value(cb, S_028354_SURFACE_SYNC_MASK(0xf));

	return;
}

void evergreen_init_atom_start_cs(struct r600_context *rctx)
{
	struct r600_command_buffer *cb = &rctx->start_cs_cmd;
	int num_ps_threads;
	int num_vs_threads;
	int num_gs_threads;
	int num_es_threads;
	int num_hs_threads;
	int num_ls_threads;

	int num_ps_stack_entries;
	int num_vs_stack_entries;
	int num_gs_stack_entries;
	int num_es_stack_entries;
	int num_hs_stack_entries;
	int num_ls_stack_entries;
	enum radeon_family family;
	unsigned tmp, i;

	if (rctx->b.gfx_level == CAYMAN) {
		cayman_init_atom_start_cs(rctx);
		return;
	}

	r600_init_command_buffer(cb, 338);

	/* This must be first. */
	r600_store_value(cb, PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
	r600_store_value(cb, 0x80000000);
	r600_store_value(cb, 0x80000000);

	/* We're setting config registers here. */
	r600_store_value(cb, PKT3(PKT3_EVENT_WRITE, 0, 0));
	r600_store_value(cb, EVENT_TYPE(EVENT_TYPE_PS_PARTIAL_FLUSH) | EVENT_INDEX(4));

	/* This enables pipeline stat & streamout queries.
	 * They are only disabled by blits.
	 */
	r600_store_value(cb, PKT3(PKT3_EVENT_WRITE, 0, 0));
	r600_store_value(cb, EVENT_TYPE(EVENT_TYPE_PIPELINESTAT_START) | EVENT_INDEX(0));

	evergreen_init_common_regs(rctx, cb, rctx->b.gfx_level,
				   rctx->b.family, rctx->screen->b.info.drm_minor);

	family = rctx->b.family;
	switch (family) {
	case CHIP_CEDAR:
	default:
		num_ps_threads = 96;
		num_vs_threads = 16;
		num_gs_threads = 16;
		num_es_threads = 16;
		num_hs_threads = 16;
		num_ls_threads = 16;
		num_ps_stack_entries = 42;
		num_vs_stack_entries = 42;
		num_gs_stack_entries = 42;
		num_es_stack_entries = 42;
		num_hs_stack_entries = 42;
		num_ls_stack_entries = 42;
		break;
	case CHIP_REDWOOD:
		num_ps_threads = 128;
		num_vs_threads = 20;
		num_gs_threads = 20;
		num_es_threads = 20;
		num_hs_threads = 20;
		num_ls_threads = 20;
		num_ps_stack_entries = 42;
		num_vs_stack_entries = 42;
		num_gs_stack_entries = 42;
		num_es_stack_entries = 42;
		num_hs_stack_entries = 42;
		num_ls_stack_entries = 42;
		break;
	case CHIP_JUNIPER:
		num_ps_threads = 128;
		num_vs_threads = 20;
		num_gs_threads = 20;
		num_es_threads = 20;
		num_hs_threads = 20;
		num_ls_threads = 20;
		num_ps_stack_entries = 85;
		num_vs_stack_entries = 85;
		num_gs_stack_entries = 85;
		num_es_stack_entries = 85;
		num_hs_stack_entries = 85;
		num_ls_stack_entries = 85;
		break;
	case CHIP_CYPRESS:
	case CHIP_HEMLOCK:
		num_ps_threads = 128;
		num_vs_threads = 20;
		num_gs_threads = 20;
		num_es_threads = 20;
		num_hs_threads = 20;
		num_ls_threads = 20;
		num_ps_stack_entries = 85;
		num_vs_stack_entries = 85;
		num_gs_stack_entries = 85;
		num_es_stack_entries = 85;
		num_hs_stack_entries = 85;
		num_ls_stack_entries = 85;
		break;
	case CHIP_PALM:
		num_ps_threads = 96;
		num_vs_threads = 16;
		num_gs_threads = 16;
		num_es_threads = 16;
		num_hs_threads = 16;
		num_ls_threads = 16;
		num_ps_stack_entries = 42;
		num_vs_stack_entries = 42;
		num_gs_stack_entries = 42;
		num_es_stack_entries = 42;
		num_hs_stack_entries = 42;
		num_ls_stack_entries = 42;
		break;
	case CHIP_SUMO:
		num_ps_threads = 96;
		num_vs_threads = 25;
		num_gs_threads = 25;
		num_es_threads = 25;
		num_hs_threads = 16;
		num_ls_threads = 16;
		num_ps_stack_entries = 42;
		num_vs_stack_entries = 42;
		num_gs_stack_entries = 42;
		num_es_stack_entries = 42;
		num_hs_stack_entries = 42;
		num_ls_stack_entries = 42;
		break;
	case CHIP_SUMO2:
		num_ps_threads = 96;
		num_vs_threads = 25;
		num_gs_threads = 25;
		num_es_threads = 25;
		num_hs_threads = 16;
		num_ls_threads = 16;
		num_ps_stack_entries = 85;
		num_vs_stack_entries = 85;
		num_gs_stack_entries = 85;
		num_es_stack_entries = 85;
		num_hs_stack_entries = 85;
		num_ls_stack_entries = 85;
		break;
	case CHIP_BARTS:
		num_ps_threads = 128;
		num_vs_threads = 20;
		num_gs_threads = 20;
		num_es_threads = 20;
		num_hs_threads = 20;
		num_ls_threads = 20;
		num_ps_stack_entries = 85;
		num_vs_stack_entries = 85;
		num_gs_stack_entries = 85;
		num_es_stack_entries = 85;
		num_hs_stack_entries = 85;
		num_ls_stack_entries = 85;
		break;
	case CHIP_TURKS:
		num_ps_threads = 128;
		num_vs_threads = 20;
		num_gs_threads = 20;
		num_es_threads = 20;
		num_hs_threads = 20;
		num_ls_threads = 20;
		num_ps_stack_entries = 42;
		num_vs_stack_entries = 42;
		num_gs_stack_entries = 42;
		num_es_stack_entries = 42;
		num_hs_stack_entries = 42;
		num_ls_stack_entries = 42;
		break;
	case CHIP_CAICOS:
		num_ps_threads = 96;
		num_vs_threads = 10;
		num_gs_threads = 10;
		num_es_threads = 10;
		num_hs_threads = 10;
		num_ls_threads = 10;
		num_ps_stack_entries = 42;
		num_vs_stack_entries = 42;
		num_gs_stack_entries = 42;
		num_es_stack_entries = 42;
		num_hs_stack_entries = 42;
		num_ls_stack_entries = 42;
		break;
	}

	tmp = S_008C18_NUM_PS_THREADS(num_ps_threads);
	tmp |= S_008C18_NUM_VS_THREADS(num_vs_threads);
	tmp |= S_008C18_NUM_GS_THREADS(num_gs_threads);
	tmp |= S_008C18_NUM_ES_THREADS(num_es_threads);

	r600_store_config_reg_seq(cb, R_008C18_SQ_THREAD_RESOURCE_MGMT_1, 5);
	r600_store_value(cb, tmp); /* R_008C18_SQ_THREAD_RESOURCE_MGMT_1 */

	tmp = S_008C1C_NUM_HS_THREADS(num_hs_threads);
	tmp |= S_008C1C_NUM_LS_THREADS(num_ls_threads);
	r600_store_value(cb, tmp); /* R_008C1C_SQ_THREAD_RESOURCE_MGMT_2 */

	tmp = S_008C20_NUM_PS_STACK_ENTRIES(num_ps_stack_entries);
	tmp |= S_008C20_NUM_VS_STACK_ENTRIES(num_vs_stack_entries);
	r600_store_value(cb, tmp); /* R_008C20_SQ_STACK_RESOURCE_MGMT_1 */

	tmp = S_008C24_NUM_GS_STACK_ENTRIES(num_gs_stack_entries);
	tmp |= S_008C24_NUM_ES_STACK_ENTRIES(num_es_stack_entries);
	r600_store_value(cb, tmp); /* R_008C24_SQ_STACK_RESOURCE_MGMT_2 */

	tmp = S_008C28_NUM_HS_STACK_ENTRIES(num_hs_stack_entries);
	tmp |= S_008C28_NUM_LS_STACK_ENTRIES(num_ls_stack_entries);
	r600_store_value(cb, tmp); /* R_008C28_SQ_STACK_RESOURCE_MGMT_3 */

	r600_store_config_reg(cb, R_008E2C_SQ_LDS_RESOURCE_MGMT,
			      S_008E2C_NUM_PS_LDS(0x1000) | S_008E2C_NUM_LS_LDS(0x1000));

	/* remove LS/HS from one SIMD for hw workaround */
	r600_store_config_reg_seq(cb, R_008E20_SQ_STATIC_THREAD_MGMT1, 3);
	r600_store_value(cb, 0xffffffff);
	r600_store_value(cb, 0xffffffff);
	r600_store_value(cb, 0xfffffffe);

	r600_store_config_reg(cb, R_009100_SPI_CONFIG_CNTL, 0);
	r600_store_config_reg(cb, R_00913C_SPI_CONFIG_CNTL_1, S_00913C_VTX_DONE_DELAY(4));

	r600_store_context_reg_seq(cb, R_028900_SQ_ESGS_RING_ITEMSIZE, 6);
	r600_store_value(cb, 0); /* R_028900_SQ_ESGS_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028904_SQ_GSVS_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028908_SQ_ESTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_02890C_SQ_GSTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028910_SQ_VSTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028914_SQ_PSTMP_RING_ITEMSIZE */

	r600_store_context_reg_seq(cb, R_02891C_SQ_GS_VERT_ITEMSIZE, 4);
	r600_store_value(cb, 0); /* R_02891C_SQ_GS_VERT_ITEMSIZE */
	r600_store_value(cb, 0); /* R_028920_SQ_GS_VERT_ITEMSIZE_1 */
	r600_store_value(cb, 0); /* R_028924_SQ_GS_VERT_ITEMSIZE_2 */
	r600_store_value(cb, 0); /* R_028928_SQ_GS_VERT_ITEMSIZE_3 */

	r600_store_context_reg_seq(cb, R_028A10_VGT_OUTPUT_PATH_CNTL, 13);
	r600_store_value(cb, 0); /* R_028A10_VGT_OUTPUT_PATH_CNTL */
	r600_store_value(cb, 0); /* R_028A14_VGT_HOS_CNTL */
	r600_store_value(cb, fui(64)); /* R_028A18_VGT_HOS_MAX_TESS_LEVEL */
	r600_store_value(cb, fui(1.0)); /* R_028A1C_VGT_HOS_MIN_TESS_LEVEL */
	r600_store_value(cb, 16); /* R_028A20_VGT_HOS_REUSE_DEPTH */
	r600_store_value(cb, 0); /* R_028A24_VGT_GROUP_PRIM_TYPE */
	r600_store_value(cb, 0); /* R_028A28_VGT_GROUP_FIRST_DECR */
	r600_store_value(cb, 0); /* R_028A2C_VGT_GROUP_DECR */
	r600_store_value(cb, 0); /* R_028A30_VGT_GROUP_VECT_0_CNTL */
	r600_store_value(cb, 0); /* R_028A34_VGT_GROUP_VECT_1_CNTL */
	r600_store_value(cb, 0); /* R_028A38_VGT_GROUP_VECT_0_FMT_CNTL */
	r600_store_value(cb, 0); /* R_028A3C_VGT_GROUP_VECT_1_FMT_CNTL */
	r600_store_value(cb, 0); /* R_028A40_VGT_GS_MODE */

	r600_store_config_reg(cb, R_008A14_PA_CL_ENHANCE, (3 << 1) | 1);

        r600_store_context_reg(cb, R_0288F0_SQ_VTX_SEMANTIC_CLEAR, ~0);

        r600_store_context_reg_seq(cb, R_028400_VGT_MAX_VTX_INDX, 2);
	r600_store_value(cb, ~0); /* R_028400_VGT_MAX_VTX_INDX */
	r600_store_value(cb, 0); /* R_028404_VGT_MIN_VTX_INDX */

	r600_store_ctl_const(cb, R_03CFF0_SQ_VTX_BASE_VTX_LOC, 0);

	r600_store_context_reg(cb, R_028028_DB_STENCIL_CLEAR, 0);

	r600_store_context_reg(cb, R_028200_PA_SC_WINDOW_OFFSET, 0);
	r600_store_context_reg(cb, R_02820C_PA_SC_CLIPRECT_RULE, 0xFFFF);
	r600_store_context_reg(cb, R_028230_PA_SC_EDGERULE, 0xAAAAAAAA);

	r600_store_context_reg(cb, R_0286DC_SPI_FOG_CNTL, 0);
	r600_store_context_reg(cb, R_028820_PA_CL_NANINF_CNTL, 0);

	r600_store_context_reg_seq(cb, R_028AC0_DB_SRESULTS_COMPARE_STATE0, 3);
	r600_store_value(cb, 0); /* R_028AC0_DB_SRESULTS_COMPARE_STATE0 */
	r600_store_value(cb, 0); /* R_028AC4_DB_SRESULTS_COMPARE_STATE1 */
	r600_store_value(cb, 0); /* R_028AC8_DB_PRELOAD_CONTROL */

	r600_store_context_reg_seq(cb, R_028240_PA_SC_GENERIC_SCISSOR_TL, 2);
	r600_store_value(cb, 0); /* R_028240_PA_SC_GENERIC_SCISSOR_TL */
	r600_store_value(cb, S_028244_BR_X(16384) | S_028244_BR_Y(16384)); /* R_028244_PA_SC_GENERIC_SCISSOR_BR */

	r600_store_context_reg_seq(cb, R_028030_PA_SC_SCREEN_SCISSOR_TL, 2);
	r600_store_value(cb, 0); /* R_028030_PA_SC_SCREEN_SCISSOR_TL */
	r600_store_value(cb, S_028034_BR_X(16384) | S_028034_BR_Y(16384)); /* R_028034_PA_SC_SCREEN_SCISSOR_BR */

	r600_store_context_reg(cb, R_028848_SQ_PGM_RESOURCES_2_PS, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_028864_SQ_PGM_RESOURCES_2_VS, S_028864_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_02887C_SQ_PGM_RESOURCES_2_GS, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_028894_SQ_PGM_RESOURCES_2_ES, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_0288A8_SQ_PGM_RESOURCES_FS, 0);
	r600_store_context_reg(cb, R_0288C0_SQ_PGM_RESOURCES_2_HS, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));
	r600_store_context_reg(cb, R_0288D8_SQ_PGM_RESOURCES_2_LS, S_028848_SINGLE_ROUND(V_SQ_ROUND_NEAREST_EVEN));

	/* to avoid GPU doing any preloading of constant from random address */
	r600_store_context_reg_seq(cb, R_028140_ALU_CONST_BUFFER_SIZE_PS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg_seq(cb, R_028180_ALU_CONST_BUFFER_SIZE_VS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg_seq(cb, R_0281C0_ALU_CONST_BUFFER_SIZE_GS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg_seq(cb, R_028FC0_ALU_CONST_BUFFER_SIZE_LS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg_seq(cb, R_028F80_ALU_CONST_BUFFER_SIZE_HS_0, 16);
	for (i = 0; i < 16; i++)
		r600_store_value(cb, 0);

	r600_store_context_reg(cb, R_028B98_VGT_STRMOUT_BUFFER_CONFIG, 0);

	if (rctx->screen->b.has_streamout) {
		r600_store_context_reg(cb, R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET, 0);
	}

	r600_store_context_reg(cb, R_028010_DB_RENDER_OVERRIDE2, 0);
	r600_store_context_reg(cb, R_028234_PA_SU_HARDWARE_SCREEN_OFFSET, 0);
	r600_store_context_reg(cb, R_0286C8_SPI_THREAD_GROUPING, 0);
	r600_store_context_reg_seq(cb, R_0286E4_SPI_PS_IN_CONTROL_2, 2);
	r600_store_value(cb, 0); /* R_0286E4_SPI_PS_IN_CONTROL_2 */
	r600_store_value(cb, 0); /* R_0286E8_SPI_COMPUTE_INPUT_CNTL */

	r600_store_context_reg_seq(cb, R_0288E8_SQ_LDS_ALLOC, 2);
	r600_store_value(cb, 0); /* R_0288E8_SQ_LDS_ALLOC */
	r600_store_value(cb, 0); /* R_0288EC_SQ_LDS_ALLOC_PS */

	if (rctx->b.family == CHIP_CAICOS) {
		r600_store_context_reg_seq(cb, R_028B54_VGT_SHADER_STAGES_EN, 2);
		r600_store_value(cb, 0); /* R028B54_VGT_SHADER_STAGES_EN */
		r600_store_value(cb, 0); /* R028B58_VGT_LS_HS_CONFIG */
		r600_store_context_reg(cb, R_028B6C_VGT_TF_PARAM, 0);
	} else {
		r600_store_context_reg_seq(cb, R_028B54_VGT_SHADER_STAGES_EN, 7);
		r600_store_value(cb, 0); /* R028B54_VGT_SHADER_STAGES_EN */
		r600_store_value(cb, 0); /* R028B58_VGT_LS_HS_CONFIG */
		r600_store_value(cb, 0); /* R028B5C_VGT_LS_SIZE */
		r600_store_value(cb, 0); /* R028B60_VGT_HS_SIZE */
		r600_store_value(cb, 0); /* R028B64_VGT_LS_HS_ALLOC */
		r600_store_value(cb, 0); /* R028B68_VGT_HS_PATCH_CONST */
		r600_store_value(cb, 0); /* R028B68_VGT_TF_PARAM */
	}

	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0, 0x01000FFF);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (32 * 4), 0x01000FFF);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (64 * 4), 0x01000FFF);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (96 * 4), 0x01000FFF);
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (128 * 4), 0x01000FFF);
}

void evergreen_update_ps_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;
	unsigned i, exports_ps, num_cout, spi_ps_in_control_0, spi_input_z, spi_ps_in_control_1, db_shader_control = 0;
	int pos_index = -1, face_index = -1, fixed_pt_position_index = -1;
	int ninterp = 0;
	bool have_perspective = false, have_linear = false;
	static const unsigned spi_baryc_enable_bit[6] = {
		S_0286E0_PERSP_SAMPLE_ENA(1),
		S_0286E0_PERSP_CENTER_ENA(1),
		S_0286E0_PERSP_CENTROID_ENA(1),
		S_0286E0_LINEAR_SAMPLE_ENA(1),
		S_0286E0_LINEAR_CENTER_ENA(1),
		S_0286E0_LINEAR_CENTROID_ENA(1)
	};
	unsigned spi_baryc_cntl = 0, sid, tmp, num = 0;
	unsigned z_export = 0, stencil_export = 0, mask_export = 0;
	uint32_t spi_ps_input_cntl[32];

	/* Pull any state we use out of rctx.  Make sure that any additional
	 * state added to this list is also checked in the caller in
	 * r600_update_derived_state().
	 */
	bool sprite_coord_enable = rctx->rasterizer ? rctx->rasterizer->sprite_coord_enable : 0;
	bool flatshade = rctx->rasterizer ? rctx->rasterizer->flatshade : 0;
	bool msaa = rctx->framebuffer.nr_samples > 1 && rctx->ps_iter_samples > 0;

	if (!cb->buf) {
		r600_init_command_buffer(cb, 64);
	} else {
		cb->num_dw = 0;
	}

	for (i = 0; i < rshader->ninput; i++) {
		const gl_varying_slot varying_slot = rshader->input[i].varying_slot;

		/* evergreen NUM_INTERP only contains values interpolated into the LDS,
		   POSITION goes via GPRs from the SC so isn't counted */
		if (varying_slot == VARYING_SLOT_POS)
			pos_index = i;
		else if (varying_slot == VARYING_SLOT_FACE) {
			if (face_index == -1)
				face_index = i;
		}
		else if (rshader->input[i].system_value == SYSTEM_VALUE_SAMPLE_MASK_IN) {
			if (face_index == -1)
				face_index = i; /* lives in same register, same enable bit */
		}
		else if (rshader->input[i].system_value == SYSTEM_VALUE_SAMPLE_ID) {
			fixed_pt_position_index = i;
		}
		else {
			ninterp++;
			int k = eg_get_interpolator_index(
				rshader->input[i].interpolate,
				rshader->input[i].interpolate_location);
			if (k >= 0) {
				spi_baryc_cntl |= spi_baryc_enable_bit[k];
				have_perspective |= k < 3;
				have_linear |= !(k < 3);
				if (rshader->input[i].uses_interpolate_at_centroid) {
					k = eg_get_interpolator_index(
						rshader->input[i].interpolate,
						TGSI_INTERPOLATE_LOC_CENTROID);
					spi_baryc_cntl |= spi_baryc_enable_bit[k];
				}
			}
		}

		sid = rshader->input[i].spi_sid;

		if (sid) {
			tmp = S_028644_SEMANTIC(sid);

			/* D3D 9 behaviour. GL is undefined */
			if (varying_slot == VARYING_SLOT_COL0)
				tmp |= S_028644_DEFAULT_VAL(3);

			if (varying_slot == VARYING_SLOT_POS ||
				rshader->input[i].interpolate == TGSI_INTERPOLATE_CONSTANT ||
				(rshader->input[i].interpolate == TGSI_INTERPOLATE_COLOR && flatshade)) {
				tmp |= S_028644_FLAT_SHADE(1);
			}

			if (varying_slot == VARYING_SLOT_PNTC ||
			    (varying_slot >= VARYING_SLOT_TEX0 && varying_slot <= VARYING_SLOT_TEX7 &&
			     (sprite_coord_enable & (1 << ((int)varying_slot - (int)VARYING_SLOT_TEX0))))) {
				tmp |= S_028644_PT_SPRITE_TEX(1);
			}

			spi_ps_input_cntl[num++] = tmp;
		}
	}

	r600_store_context_reg_seq(cb, R_028644_SPI_PS_INPUT_CNTL_0, num);
	r600_store_array(cb, num, spi_ps_input_cntl);

	exports_ps = 0;
	for (i = 0; i < rshader->noutput; i++) {
		switch (rshader->output[i].frag_result) {
		case FRAG_RESULT_DEPTH:
			z_export = 1;
			exports_ps |= 1;
			break;
		case FRAG_RESULT_STENCIL:
			stencil_export = 1;
			exports_ps |= 1;
			break;
		case FRAG_RESULT_SAMPLE_MASK:
			if (msaa)
				mask_export = 1;
			exports_ps |= 1;
			break;
		default:
			break;
		}
	}
	if (rshader->uses_kill)
		db_shader_control |= S_02880C_KILL_ENABLE(1);

	db_shader_control |= S_02880C_Z_EXPORT_ENABLE(z_export);
	db_shader_control |= S_02880C_STENCIL_EXPORT_ENABLE(stencil_export);
	db_shader_control |= S_02880C_MASK_EXPORT_ENABLE(mask_export);

	if (shader->selector->info.properties[TGSI_PROPERTY_FS_EARLY_DEPTH_STENCIL]) {
		db_shader_control |= S_02880C_DEPTH_BEFORE_SHADER(1) |
			S_02880C_EXEC_ON_NOOP(shader->selector->info.writes_memory);
	} else if (shader->selector->info.writes_memory) {
		db_shader_control |= S_02880C_EXEC_ON_HIER_FAIL(1);
	}

	switch (rshader->ps_conservative_z) {
	default: /* fall through */
	case FRAG_DEPTH_LAYOUT_ANY:
		db_shader_control |= S_02880C_CONSERVATIVE_Z_EXPORT(V_02880C_EXPORT_ANY_Z);
		break;
	case FRAG_DEPTH_LAYOUT_GREATER:
		db_shader_control |= S_02880C_CONSERVATIVE_Z_EXPORT(V_02880C_EXPORT_GREATER_THAN_Z);
		break;
	case FRAG_DEPTH_LAYOUT_LESS:
		db_shader_control |= S_02880C_CONSERVATIVE_Z_EXPORT(V_02880C_EXPORT_LESS_THAN_Z);
		break;
	}

	num_cout = rshader->ps_export_highest + 1;

	exports_ps |= S_02884C_EXPORT_COLORS(num_cout);
	if (!exports_ps) {
		/* always at least export 1 component per pixel */
		exports_ps = 2;
	}
	shader->nr_ps_color_outputs = num_cout;
	shader->ps_color_export_mask = rshader->ps_color_export_mask;
	if (ninterp == 0) {
		ninterp = 1;
		have_perspective = true;
	}
	if (!spi_baryc_cntl)
		spi_baryc_cntl |= spi_baryc_enable_bit[0];

	if (!have_perspective && !have_linear)
		have_perspective = true;

	spi_ps_in_control_0 = S_0286CC_NUM_INTERP(ninterp) |
		              S_0286CC_PERSP_GRADIENT_ENA(have_perspective) |
		              S_0286CC_LINEAR_GRADIENT_ENA(have_linear);
	spi_input_z = 0;
	if (pos_index != -1) {
		spi_ps_in_control_0 |=  S_0286CC_POSITION_ENA(1) |
			S_0286CC_POSITION_CENTROID(rshader->input[pos_index].interpolate_location == TGSI_INTERPOLATE_LOC_CENTROID) |
			S_0286CC_POSITION_ADDR(rshader->input[pos_index].gpr);
		spi_input_z |= S_0286D8_PROVIDE_Z_TO_SPI(1);
	}

	spi_ps_in_control_1 = 0;
	if (face_index != -1) {
		spi_ps_in_control_1 |= S_0286D0_FRONT_FACE_ENA(1) |
			S_0286D0_FRONT_FACE_ADDR(rshader->input[face_index].gpr);
	}
	if (fixed_pt_position_index != -1) {
		spi_ps_in_control_1 |= S_0286D0_FIXED_PT_POSITION_ENA(1) |
			S_0286D0_FIXED_PT_POSITION_ADDR(rshader->input[fixed_pt_position_index].gpr);
	}

	r600_store_context_reg_seq(cb, R_0286CC_SPI_PS_IN_CONTROL_0, 2);
	r600_store_value(cb, spi_ps_in_control_0); /* R_0286CC_SPI_PS_IN_CONTROL_0 */
	r600_store_value(cb, spi_ps_in_control_1); /* R_0286D0_SPI_PS_IN_CONTROL_1 */

	r600_store_context_reg(cb, R_0286E0_SPI_BARYC_CNTL, spi_baryc_cntl);
	r600_store_context_reg(cb, R_0286D8_SPI_INPUT_Z, spi_input_z);
	r600_store_context_reg(cb, R_02884C_SQ_PGM_EXPORTS_PS, exports_ps);

	r600_store_context_reg_seq(cb, R_028840_SQ_PGM_START_PS, 2);
	r600_store_value(cb, shader->bo->gpu_address >> 8);
	r600_store_value(cb, /* R_028844_SQ_PGM_RESOURCES_PS */
			 S_028844_NUM_GPRS(rshader->bc.ngpr) |
			 S_028844_PRIME_CACHE_ON_DRAW(1) |
			 S_028844_DX10_CLAMP(1) |
			 S_028844_STACK_SIZE(rshader->bc.nstack));
	/* After that, the NOP relocation packet must be emitted (shader->bo, RADEON_USAGE_READ). */

	shader->db_shader_control = db_shader_control;
	shader->ps_depth_export = z_export | stencil_export | mask_export;

	shader->sprite_coord_enable = sprite_coord_enable;
	shader->flatshade = flatshade;
	shader->msaa = msaa;
}

void evergreen_update_es_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;

	r600_init_command_buffer(cb, 32);

	r600_store_context_reg(cb, R_028890_SQ_PGM_RESOURCES_ES,
			       S_028890_NUM_GPRS(rshader->bc.ngpr) |
			       S_028890_DX10_CLAMP(1) |
			       S_028890_STACK_SIZE(rshader->bc.nstack));
	r600_store_context_reg(cb, R_02888C_SQ_PGM_START_ES,
			       shader->bo->gpu_address >> 8);
	/* After that, the NOP relocation packet must be emitted (shader->bo, RADEON_USAGE_READ). */
}

void evergreen_update_gs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;
	struct r600_shader *cp_shader = &shader->gs_copy_shader->shader;
	unsigned gsvs_itemsizes[4] = {
			(cp_shader->ring_item_sizes[0] * shader->selector->gs_max_out_vertices) >> 2,
			(cp_shader->ring_item_sizes[1] * shader->selector->gs_max_out_vertices) >> 2,
			(cp_shader->ring_item_sizes[2] * shader->selector->gs_max_out_vertices) >> 2,
			(cp_shader->ring_item_sizes[3] * shader->selector->gs_max_out_vertices) >> 2
	};

	r600_init_command_buffer(cb, 64);

	/* VGT_GS_MODE is written by evergreen_emit_shader_stages */


	r600_store_context_reg(cb, R_028B38_VGT_GS_MAX_VERT_OUT,
			       S_028B38_MAX_VERT_OUT(shader->selector->gs_max_out_vertices));
	r600_store_context_reg(cb, R_028A6C_VGT_GS_OUT_PRIM_TYPE,
			       r600_conv_prim_to_gs_out(shader->selector->gs_output_prim));

	r600_store_context_reg(cb, R_028B90_VGT_GS_INSTANCE_CNT,
				S_028B90_CNT(MIN2(shader->selector->gs_num_invocations, 127)) |
				S_028B90_ENABLE(shader->selector->gs_num_invocations > 0));
	r600_store_context_reg_seq(cb, R_02891C_SQ_GS_VERT_ITEMSIZE, 4);
	r600_store_value(cb, cp_shader->ring_item_sizes[0] >> 2);
	r600_store_value(cb, cp_shader->ring_item_sizes[1] >> 2);
	r600_store_value(cb, cp_shader->ring_item_sizes[2] >> 2);
	r600_store_value(cb, cp_shader->ring_item_sizes[3] >> 2);

	r600_store_context_reg(cb, R_028900_SQ_ESGS_RING_ITEMSIZE,
			       (rshader->ring_item_sizes[0]) >> 2);

	r600_store_context_reg(cb, R_028904_SQ_GSVS_RING_ITEMSIZE,
			       gsvs_itemsizes[0] +
			       gsvs_itemsizes[1] +
			       gsvs_itemsizes[2] +
			       gsvs_itemsizes[3]);

	r600_store_context_reg_seq(cb, R_02892C_SQ_GSVS_RING_OFFSET_1, 3);
	r600_store_value(cb, gsvs_itemsizes[0]);
	r600_store_value(cb, gsvs_itemsizes[0] + gsvs_itemsizes[1]);
	r600_store_value(cb, gsvs_itemsizes[0] + gsvs_itemsizes[1] + gsvs_itemsizes[2]);

	/* FIXME calculate these values somehow ??? */
	r600_store_context_reg_seq(cb, R_028A54_GS_PER_ES, 3);
	r600_store_value(cb, 0x80); /* GS_PER_ES */
	r600_store_value(cb, 0x100); /* ES_PER_GS */
	r600_store_value(cb, 0x2); /* GS_PER_VS */

	r600_store_context_reg(cb, R_028878_SQ_PGM_RESOURCES_GS,
			       S_028878_NUM_GPRS(rshader->bc.ngpr) |
			       S_028878_DX10_CLAMP(1) |
			       S_028878_STACK_SIZE(rshader->bc.nstack));
	r600_store_context_reg(cb, R_028874_SQ_PGM_START_GS,
			       shader->bo->gpu_address >> 8);
	/* After that, the NOP relocation packet must be emitted (shader->bo, RADEON_USAGE_READ). */
}


void evergreen_update_vs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;
	unsigned spi_vs_out_id[10] = {};
	unsigned i;

	for (i = 0; i < rshader->noutput; i++) {
		const int param = rshader->output[i].export_param;
		if (param < 0)
			continue;
		unsigned *const param_spi_vs_out_id = &spi_vs_out_id[param / 4];
		const unsigned param_shift = (param & 3) * 8;
		assert(!(*param_spi_vs_out_id & (0xFFu << param_shift)));
		*param_spi_vs_out_id |= (unsigned)rshader->output[i].spi_sid << param_shift;
	}

	r600_init_command_buffer(cb, 32);

	r600_store_context_reg_seq(cb, R_02861C_SPI_VS_OUT_ID_0, 10);
	for (i = 0; i < 10; i++) {
		r600_store_value(cb, spi_vs_out_id[i]);
	}

	r600_store_context_reg(cb, R_0286C4_SPI_VS_OUT_CONFIG,
			       S_0286C4_VS_EXPORT_COUNT(rshader->highest_export_param));
	r600_store_context_reg(cb, R_028860_SQ_PGM_RESOURCES_VS,
			       S_028860_NUM_GPRS(rshader->bc.ngpr) |
			       S_028860_DX10_CLAMP(1) |
			       S_028860_STACK_SIZE(rshader->bc.nstack));
	if (rshader->vs_position_window_space) {
		r600_store_context_reg(cb, R_028818_PA_CL_VTE_CNTL,
			S_028818_VTX_XY_FMT(1) | S_028818_VTX_Z_FMT(1));
	} else {
		r600_store_context_reg(cb, R_028818_PA_CL_VTE_CNTL,
			S_028818_VTX_W0_FMT(1) |
			S_028818_VPORT_X_SCALE_ENA(1) | S_028818_VPORT_X_OFFSET_ENA(1) |
			S_028818_VPORT_Y_SCALE_ENA(1) | S_028818_VPORT_Y_OFFSET_ENA(1) |
			S_028818_VPORT_Z_SCALE_ENA(1) | S_028818_VPORT_Z_OFFSET_ENA(1));

	}
	r600_store_context_reg(cb, R_02885C_SQ_PGM_START_VS,
			       shader->bo->gpu_address >> 8);
	/* After that, the NOP relocation packet must be emitted (shader->bo, RADEON_USAGE_READ). */

	shader->pa_cl_vs_out_cntl =
		S_02881C_VS_OUT_CCDIST0_VEC_ENA((rshader->cc_dist_mask & 0x0F) != 0) |
		S_02881C_VS_OUT_CCDIST1_VEC_ENA((rshader->cc_dist_mask & 0xF0) != 0) |
		S_02881C_VS_OUT_MISC_VEC_ENA(rshader->vs_out_misc_write) |
		S_02881C_USE_VTX_POINT_SIZE(rshader->vs_out_point_size) |
		S_02881C_USE_VTX_EDGE_FLAG(rshader->vs_out_edgeflag) |
		S_02881C_USE_VTX_VIEWPORT_INDX(rshader->vs_out_viewport) |
		S_02881C_USE_VTX_RENDER_TARGET_INDX(rshader->vs_out_layer);
}

void evergreen_update_hs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;

	r600_init_command_buffer(cb, 32);
	r600_store_context_reg(cb, R_0288BC_SQ_PGM_RESOURCES_HS,
			       S_0288BC_NUM_GPRS(rshader->bc.ngpr) |
			       S_0288BC_DX10_CLAMP(1) |
			       S_0288BC_STACK_SIZE(rshader->bc.nstack));
	r600_store_context_reg(cb, R_0288B8_SQ_PGM_START_HS,
			       shader->bo->gpu_address >> 8);
}

void evergreen_update_ls_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;

	r600_init_command_buffer(cb, 32);
	r600_store_context_reg(cb, R_0288D4_SQ_PGM_RESOURCES_LS,
			       S_0288D4_NUM_GPRS(rshader->bc.ngpr) |
			       S_0288D4_DX10_CLAMP(1) |
			       S_0288D4_STACK_SIZE(rshader->bc.nstack));
	r600_store_context_reg(cb, R_0288D0_SQ_PGM_START_LS,
			       shader->bo->gpu_address >> 8);
}
void *evergreen_create_resolve_blend(struct r600_context *rctx)
{
	struct pipe_blend_state blend;

	memset(&blend, 0, sizeof(blend));
	blend.independent_blend_enable = true;
	blend.rt[0].colormask = 0xf;
	return evergreen_create_blend_state_mode(&rctx->b.b, &blend, V_028808_CB_RESOLVE);
}

void *evergreen_create_decompress_blend(struct r600_context *rctx)
{
	struct pipe_blend_state blend;
	unsigned mode = rctx->screen->has_compressed_msaa_texturing ?
			V_028808_CB_FMASK_DECOMPRESS : V_028808_CB_DECOMPRESS;

	memset(&blend, 0, sizeof(blend));
	blend.independent_blend_enable = true;
	blend.rt[0].colormask = 0xf;
	return evergreen_create_blend_state_mode(&rctx->b.b, &blend, mode);
}

void *evergreen_create_fastclear_blend(struct r600_context *rctx)
{
	struct pipe_blend_state blend;
	unsigned mode = V_028808_CB_ELIMINATE_FAST_CLEAR;

	memset(&blend, 0, sizeof(blend));
	blend.independent_blend_enable = true;
	blend.rt[0].colormask = 0xf;
	return evergreen_create_blend_state_mode(&rctx->b.b, &blend, mode);
}

void *evergreen_create_db_flush_dsa(struct r600_context *rctx)
{
	struct pipe_depth_stencil_alpha_state dsa = {{{0}}};

	return rctx->b.b.create_depth_stencil_alpha_state(&rctx->b.b, &dsa);
}

void evergreen_update_db_shader_control(struct r600_context * rctx)
{
	bool dual_export;
	unsigned db_shader_control;

	if (!rctx->ps_shader) {
		return;
	}

	dual_export = rctx->framebuffer.export_16bpc &&
		      !rctx->ps_shader->current->ps_depth_export;

	db_shader_control = rctx->ps_shader->current->db_shader_control |
			    S_02880C_DUAL_EXPORT_ENABLE(dual_export) |
			    S_02880C_DB_SOURCE_FORMAT(dual_export ? V_02880C_EXPORT_DB_TWO :
								    V_02880C_EXPORT_DB_FULL) |
			    S_02880C_ALPHA_TO_MASK_DISABLE(rctx->framebuffer.cb0_is_integer);

	/* When alpha test is enabled we can't trust the hw to make the proper
	 * decision on the order in which ztest should be run related to fragment
	 * shader execution.
	 *
	 * If alpha test is enabled perform early z rejection (RE_Z) but don't early
	 * write to the zbuffer. Write to zbuffer is delayed after fragment shader
	 * execution and thus after alpha test so if discarded by the alpha test
	 * the z value is not written.
	 * If ReZ is enabled, and the zfunc/zenable/zwrite values change you can
	 * get a hang unless you flush the DB in between.  For now just use
	 * LATE_Z.
	 */
	if (rctx->alphatest_state.sx_alpha_test_control || rctx->ps_shader->info.writes_memory) {
		db_shader_control |= S_02880C_Z_ORDER(V_02880C_LATE_Z);
	} else {
		db_shader_control |= S_02880C_Z_ORDER(V_02880C_EARLY_Z_THEN_LATE_Z);
	}

	if (db_shader_control != rctx->db_misc_state.db_shader_control) {
		rctx->db_misc_state.db_shader_control = db_shader_control;
		r600_mark_atom_dirty(rctx, &rctx->db_misc_state.atom);
	}
}

static void evergreen_dma_copy_tile(struct r600_context *rctx,
				struct pipe_resource *dst,
				unsigned dst_level,
				unsigned dst_x,
				unsigned dst_y,
				unsigned dst_z,
				struct pipe_resource *src,
				unsigned src_level,
				unsigned src_x,
				unsigned src_y,
				unsigned src_z,
				unsigned copy_height,
				unsigned pitch,
				unsigned bpp)
{
	struct radeon_cmdbuf *cs = &rctx->b.dma.cs;
	struct r600_texture *rsrc = (struct r600_texture*)src;
	struct r600_texture *rdst = (struct r600_texture*)dst;
	unsigned array_mode, lbpp, pitch_tile_max, slice_tile_max, size;
	unsigned ncopy, height, cheight, detile, i, x, y, z, src_mode, dst_mode;
	unsigned sub_cmd, bank_h, bank_w, mt_aspect, nbanks, tile_split, non_disp_tiling = 0;
	uint64_t base, addr;

	dst_mode = rdst->surface.u.legacy.level[dst_level].mode;
	src_mode = rsrc->surface.u.legacy.level[src_level].mode;
	assert(dst_mode != src_mode);

	/* non_disp_tiling bit needs to be set for depth, stencil, and fmask surfaces */
	if (util_format_has_depth(util_format_description(src->format)))
		non_disp_tiling = 1;

	y = 0;
	sub_cmd = EG_DMA_COPY_TILED;
	lbpp = util_logbase2(bpp);
	pitch_tile_max = ((pitch / bpp) / 8) - 1;
	nbanks = eg_num_banks(rctx->screen->b.info.r600_num_banks);

	if (dst_mode == RADEON_SURF_MODE_LINEAR_ALIGNED) {
		/* T2L */
		array_mode = evergreen_array_mode(src_mode);
		slice_tile_max = (rsrc->surface.u.legacy.level[src_level].nblk_x * rsrc->surface.u.legacy.level[src_level].nblk_y) / (8*8);
		slice_tile_max = slice_tile_max ? slice_tile_max - 1 : 0;
		/* linear height must be the same as the slice tile max height, it's ok even
		 * if the linear destination/source have smaller height as the size of the
		 * dma packet will be using the copy_height which is always smaller or equal
		 * to the linear height
		 */
		height = u_minify(rsrc->resource.b.b.height0, src_level);
		detile = 1;
		x = src_x;
		y = src_y;
		z = src_z;
		base = (uint64_t)rsrc->surface.u.legacy.level[src_level].offset_256B * 256;
		addr = (uint64_t)rdst->surface.u.legacy.level[dst_level].offset_256B * 256;
		addr += (uint64_t)rdst->surface.u.legacy.level[dst_level].slice_size_dw * 4 * dst_z;
		addr += dst_y * pitch + dst_x * bpp;
		bank_h = eg_bank_wh(rsrc->surface.u.legacy.bankh);
		bank_w = eg_bank_wh(rsrc->surface.u.legacy.bankw);
		mt_aspect = eg_macro_tile_aspect(rsrc->surface.u.legacy.mtilea);
		tile_split = eg_tile_split(rsrc->surface.u.legacy.tile_split);
		base += rsrc->resource.gpu_address;
		addr += rdst->resource.gpu_address;
	} else {
		/* L2T */
		array_mode = evergreen_array_mode(dst_mode);
		slice_tile_max = (rdst->surface.u.legacy.level[dst_level].nblk_x * rdst->surface.u.legacy.level[dst_level].nblk_y) / (8*8);
		slice_tile_max = slice_tile_max ? slice_tile_max - 1 : 0;
		/* linear height must be the same as the slice tile max height, it's ok even
		 * if the linear destination/source have smaller height as the size of the
		 * dma packet will be using the copy_height which is always smaller or equal
		 * to the linear height
		 */
		height = u_minify(rdst->resource.b.b.height0, dst_level);
		detile = 0;
		x = dst_x;
		y = dst_y;
		z = dst_z;
		base = (uint64_t)rdst->surface.u.legacy.level[dst_level].offset_256B * 256;
		addr = (uint64_t)rsrc->surface.u.legacy.level[src_level].offset_256B * 256;
		addr += (uint64_t)rsrc->surface.u.legacy.level[src_level].slice_size_dw * 4 * src_z;
		addr += src_y * pitch + src_x * bpp;
		bank_h = eg_bank_wh(rdst->surface.u.legacy.bankh);
		bank_w = eg_bank_wh(rdst->surface.u.legacy.bankw);
		mt_aspect = eg_macro_tile_aspect(rdst->surface.u.legacy.mtilea);
		tile_split = eg_tile_split(rdst->surface.u.legacy.tile_split);
		base += rdst->resource.gpu_address;
		addr += rsrc->resource.gpu_address;
	}

	size = (copy_height * pitch) / 4;
	ncopy = (size / EG_DMA_COPY_MAX_SIZE) + !!(size % EG_DMA_COPY_MAX_SIZE);
	r600_need_dma_space(&rctx->b, ncopy * 9, &rdst->resource, &rsrc->resource);

	for (i = 0; i < ncopy; i++) {
		cheight = copy_height;
		if (((cheight * pitch) / 4) > EG_DMA_COPY_MAX_SIZE) {
			cheight = (EG_DMA_COPY_MAX_SIZE * 4) / pitch;
		}
		size = (cheight * pitch) / 4;
		/* emit reloc before writing cs so that cs is always in consistent state */
		radeon_add_to_buffer_list(&rctx->b, &rctx->b.dma, &rsrc->resource,
				      RADEON_USAGE_READ);
		radeon_add_to_buffer_list(&rctx->b, &rctx->b.dma, &rdst->resource,
				      RADEON_USAGE_WRITE);
		radeon_emit(cs, DMA_PACKET(DMA_PACKET_COPY, sub_cmd, size));
		radeon_emit(cs, base >> 8);
		radeon_emit(cs, (detile << 31) | (array_mode << 27) |
				(lbpp << 24) | (bank_h << 21) |
				(bank_w << 18) | (mt_aspect << 16));
		radeon_emit(cs, (pitch_tile_max << 0) | ((height - 1) << 16));
		radeon_emit(cs, (slice_tile_max << 0));
		radeon_emit(cs, (x << 0) | (z << 18));
		radeon_emit(cs, (y << 0) | (tile_split << 21) | (nbanks << 25) | (non_disp_tiling << 28));
		radeon_emit(cs, addr & 0xfffffffc);
		radeon_emit(cs, (addr >> 32UL) & 0xff);
		copy_height -= cheight;
		addr += cheight * pitch;
		y += cheight;
	}
}

static void evergreen_dma_copy(struct pipe_context *ctx,
			       struct pipe_resource *dst,
			       unsigned dst_level,
			       unsigned dstx, unsigned dsty, unsigned dstz,
			       struct pipe_resource *src,
			       unsigned src_level,
			       const struct pipe_box *src_box)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_texture *rsrc = (struct r600_texture*)src;
	struct r600_texture *rdst = (struct r600_texture*)dst;
	unsigned dst_pitch, src_pitch, bpp, dst_mode, src_mode, copy_height;
	unsigned src_w, dst_w;
	unsigned src_x, src_y;
	unsigned dst_x = dstx, dst_y = dsty, dst_z = dstz;

	if (rctx->b.dma.cs.priv == NULL) {
		goto fallback;
	}

	if (rctx->cmd_buf_is_compute) {
		rctx->b.gfx.flush(rctx, PIPE_FLUSH_ASYNC, NULL);
		rctx->cmd_buf_is_compute = false;
	}

	if (dst->target == PIPE_BUFFER && src->target == PIPE_BUFFER) {
		evergreen_dma_copy_buffer(rctx, dst, src, dst_x, src_box->x, src_box->width);
		return;
	}

	if (src_box->depth > 1 ||
	    !r600_prepare_for_dma_blit(&rctx->b, rdst, dst_level, dstx, dsty,
					dstz, rsrc, src_level, src_box))
		goto fallback;

	src_x = util_format_get_nblocksx(src->format, src_box->x);
	dst_x = util_format_get_nblocksx(src->format, dst_x);
	src_y = util_format_get_nblocksy(src->format, src_box->y);
	dst_y = util_format_get_nblocksy(src->format, dst_y);

	bpp = rdst->surface.bpe;
	dst_pitch = rdst->surface.u.legacy.level[dst_level].nblk_x * rdst->surface.bpe;
	src_pitch = rsrc->surface.u.legacy.level[src_level].nblk_x * rsrc->surface.bpe;
	src_w = u_minify(rsrc->resource.b.b.width0, src_level);
	dst_w = u_minify(rdst->resource.b.b.width0, dst_level);
	copy_height = src_box->height / rsrc->surface.blk_h;

	dst_mode = rdst->surface.u.legacy.level[dst_level].mode;
	src_mode = rsrc->surface.u.legacy.level[src_level].mode;

	if (src_pitch != dst_pitch || src_box->x || dst_x || src_w != dst_w) {
		/* FIXME evergreen can do partial blit */
		goto fallback;
	}
	/* the x test here are currently useless (because we don't support partial blit)
	 * but keep them around so we don't forget about those
	 */
	if (src_pitch % 8 || src_box->x % 8 || dst_x % 8 || src_box->y % 8 || dst_y % 8) {
		goto fallback;
	}

	/* 128 bpp surfaces require non_disp_tiling for both
	 * tiled and linear buffers on cayman.  However, async
	 * DMA only supports it on the tiled side.  As such
	 * the tile order is backwards after a L2T/T2L packet.
	 */
	if ((rctx->b.gfx_level == CAYMAN) &&
	    (src_mode != dst_mode) &&
	    (util_format_get_blocksize(src->format) >= 16)) {
		goto fallback;
	}

	if (src_mode == dst_mode) {
		uint64_t dst_offset, src_offset;
		/* simple dma blit would do NOTE code here assume :
		 *   src_box.x/y == 0
		 *   dst_x/y == 0
		 *   dst_pitch == src_pitch
		 */
		src_offset= (uint64_t)rsrc->surface.u.legacy.level[src_level].offset_256B * 256;
		src_offset += (uint64_t)rsrc->surface.u.legacy.level[src_level].slice_size_dw * 4 * src_box->z;
		src_offset += src_y * src_pitch + src_x * bpp;
		dst_offset = (uint64_t)rdst->surface.u.legacy.level[dst_level].offset_256B * 256;
		dst_offset += (uint64_t)rdst->surface.u.legacy.level[dst_level].slice_size_dw * 4 * dst_z;
		dst_offset += dst_y * dst_pitch + dst_x * bpp;
		evergreen_dma_copy_buffer(rctx, dst, src, dst_offset, src_offset,
					src_box->height * src_pitch);
	} else {
		evergreen_dma_copy_tile(rctx, dst, dst_level, dst_x, dst_y, dst_z,
					src, src_level, src_x, src_y, src_box->z,
					copy_height, dst_pitch, bpp);
	}
	return;

fallback:
	r600_resource_copy_region(ctx, dst, dst_level, dstx, dsty, dstz,
				  src, src_level, src_box);
}

static void evergreen_set_tess_state(struct pipe_context *ctx,
				     const float default_outer_level[4],
				     const float default_inner_level[2])
{
	struct r600_context *rctx = (struct r600_context *)ctx;

	memcpy(rctx->tess_state, default_outer_level, sizeof(float) * 4);
	memcpy(rctx->tess_state+4, default_inner_level, sizeof(float) * 2);
	rctx->driver_consts[PIPE_SHADER_TESS_CTRL].tcs_default_levels_dirty = true;
}

static void evergreen_set_patch_vertices(struct pipe_context *ctx, uint8_t patch_vertices)
{
	struct r600_context *rctx = (struct r600_context *)ctx;

	rctx->patch_vertices = patch_vertices;
}

static void evergreen_setup_immed_buffer(struct r600_context *rctx,
					 struct r600_image_view *rview,
					 enum pipe_format pformat)
{
	struct r600_screen *rscreen = (struct r600_screen *)rctx->b.b.screen;
	uint32_t immed_size = rscreen->b.info.max_se * 256 * 64 * util_format_get_blocksize(pformat);
	struct eg_buf_res_params buf_params;
	bool skip_reloc = false;
	struct r600_resource *resource = (struct r600_resource *)rview->base.resource;
	if (!resource->immed_buffer) {
		eg_resource_alloc_immed(&rscreen->b, resource, immed_size);
	}

	memset(&buf_params, 0, sizeof(buf_params));
	buf_params.pipe_format = pformat;
	buf_params.size = resource->immed_buffer->b.b.width0;
	buf_params.swizzle[0] = PIPE_SWIZZLE_X;
	buf_params.swizzle[1] = PIPE_SWIZZLE_Y;
	buf_params.swizzle[2] = PIPE_SWIZZLE_Z;
	buf_params.swizzle[3] = PIPE_SWIZZLE_W;
	buf_params.uncached = 1;
	evergreen_fill_buffer_resource_words(rctx, &resource->immed_buffer->b.b,
					     &buf_params, &skip_reloc,
					     rview->immed_resource_words);
}

static void evergreen_set_hw_atomic_buffers(struct pipe_context *ctx,
					    unsigned start_slot,
					    unsigned count,
					    const struct pipe_shader_buffer *buffers)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_atomic_buffer_state *astate;
	unsigned i, idx;

	astate = &rctx->atomic_buffer_state;

	/* we'd probably like to expand this to 8 later so put the logic in */
	for (i = start_slot, idx = 0; i < start_slot + count; i++, idx++) {
		const struct pipe_shader_buffer *buf;
		struct pipe_shader_buffer *abuf;

		abuf = &astate->buffer[i];

		if (!buffers || !buffers[idx].buffer) {
			pipe_resource_reference(&abuf->buffer, NULL);
			continue;
		}
		buf = &buffers[idx];

		pipe_resource_reference(&abuf->buffer, buf->buffer);
		abuf->buffer_offset = buf->buffer_offset;
		abuf->buffer_size = buf->buffer_size;
	}
}

static void evergreen_set_shader_buffers(struct pipe_context *ctx,
					 enum pipe_shader_type shader, unsigned start_slot,
					 unsigned count,
					 const struct pipe_shader_buffer *buffers,
					 unsigned writable_bitmask)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_image_state *istate = NULL;
	struct r600_image_view *rview;
	struct r600_tex_color_info color;
	struct eg_buf_res_params buf_params;
	struct r600_resource *resource;
	unsigned i, idx;
	unsigned old_mask;

	if ((shader != PIPE_SHADER_FRAGMENT &&
        shader != PIPE_SHADER_COMPUTE) || count == 0)
		return;

	if (shader == PIPE_SHADER_FRAGMENT)
		istate = &rctx->fragment_buffers;
	else if (shader == PIPE_SHADER_COMPUTE)
		istate = &rctx->compute_buffers;

	old_mask = istate->enabled_mask;
	for (i = start_slot, idx = 0; i < start_slot + count; i++, idx++) {
		const struct pipe_shader_buffer *buf;
		unsigned res_type;

		rview = &istate->views[i];

		if (!buffers || !buffers[idx].buffer) {
			pipe_resource_reference((struct pipe_resource **)&rview->base.resource, NULL);
			istate->enabled_mask &= ~(1 << i);
			continue;
		}

		buf = &buffers[idx];
		pipe_resource_reference((struct pipe_resource **)&rview->base.resource, buf->buffer);

		resource = (struct r600_resource *)rview->base.resource;

		evergreen_setup_immed_buffer(rctx, rview, PIPE_FORMAT_R32_UINT);

		color.offset = 0;
		color.view = 0;
		evergreen_set_color_surface_buffer(rctx, resource,
						   PIPE_FORMAT_R32_UINT,
						   buf->buffer_offset,
						   buf->buffer_offset + buf->buffer_size,
						   &color);

		res_type = V_028C70_BUFFER;

		rview->cb_color_base = color.offset;
		rview->cb_color_dim = color.dim;
		rview->cb_color_info = color.info |
			S_028C70_RAT(1) |
			S_028C70_RESOURCE_TYPE(res_type);
		rview->cb_color_pitch = color.pitch;
		rview->cb_color_slice = color.slice;
		rview->cb_color_view = color.view;
		rview->cb_color_attrib = color.attrib;
		rview->cb_color_fmask = color.fmask;
		rview->cb_color_fmask_slice = color.fmask_slice;

		memset(&buf_params, 0, sizeof(buf_params));
		buf_params.pipe_format = PIPE_FORMAT_R32_UINT;
		buf_params.offset = buf->buffer_offset;
		buf_params.size = buf->buffer_size;
		buf_params.swizzle[0] = PIPE_SWIZZLE_X;
		buf_params.swizzle[1] = PIPE_SWIZZLE_Y;
		buf_params.swizzle[2] = PIPE_SWIZZLE_Z;
		buf_params.swizzle[3] = PIPE_SWIZZLE_W;
		buf_params.force_swizzle = true;
		buf_params.uncached = 1;
		buf_params.size_in_bytes = true;
		evergreen_fill_buffer_resource_words(rctx, &resource->b.b,
						     &buf_params,
						     &rview->skip_mip_address_reloc,
						     rview->resource_words);

		istate->enabled_mask |= (1 << i);
	}

	istate->atom.num_dw = util_bitcount(istate->enabled_mask) * 46;

	if (old_mask != istate->enabled_mask)
		r600_mark_atom_dirty(rctx, &rctx->framebuffer.atom);

	/* construct the target mask */
	if (rctx->cb_misc_state.buffer_rat_enabled_mask != istate->enabled_mask) {
		rctx->cb_misc_state.buffer_rat_enabled_mask = istate->enabled_mask;
		r600_mark_atom_dirty(rctx, &rctx->cb_misc_state.atom);
	}

	if (shader == PIPE_SHADER_FRAGMENT)
		r600_mark_atom_dirty(rctx, &istate->atom);
}

static void evergreen_set_shader_images(struct pipe_context *ctx,
					enum pipe_shader_type shader, unsigned start_slot,
					unsigned count, unsigned unbind_num_trailing_slots,
					const struct pipe_image_view *images)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	unsigned i;
	struct r600_image_view *rview;
	struct pipe_resource *image;
	struct r600_resource *resource;
	struct r600_tex_color_info color;
	struct eg_buf_res_params buf_params;
	struct eg_tex_res_params tex_params;
	unsigned old_mask;
	struct r600_image_state *istate = NULL;
	int idx;
	if (shader != PIPE_SHADER_FRAGMENT && shader != PIPE_SHADER_COMPUTE)
		return;
	if (!count && !unbind_num_trailing_slots)
		return;

	if (shader == PIPE_SHADER_FRAGMENT)
		istate = &rctx->fragment_images;
	else if (shader == PIPE_SHADER_COMPUTE)
		istate = &rctx->compute_images;

	assert (shader == PIPE_SHADER_FRAGMENT || shader == PIPE_SHADER_COMPUTE);

	old_mask = istate->enabled_mask;
	for (i = start_slot, idx = 0; i < start_slot + count; i++, idx++) {
		unsigned res_type;
		const struct pipe_image_view *iview;
		rview = &istate->views[i];

		if (!images || !images[idx].resource) {
			pipe_resource_reference((struct pipe_resource **)&rview->base.resource, NULL);
			istate->enabled_mask &= ~(1 << i);
			istate->compressed_colortex_mask &= ~(1 << i);
			istate->compressed_depthtex_mask &= ~(1 << i);
			continue;
		}

		iview = &images[idx];
		image = iview->resource;
		resource = (struct r600_resource *)image;

		r600_context_add_resource_size(ctx, image);

		struct pipe_resource *const pipe_saved = rview->base.resource;
		rview->base = *iview;
		rview->base.resource = pipe_saved;
		pipe_resource_reference((struct pipe_resource **)&rview->base.resource, image);

		evergreen_setup_immed_buffer(rctx, rview, iview->format);

		bool is_buffer = image->target == PIPE_BUFFER;
		struct r600_texture *rtex = (struct r600_texture *)image;
		if (!is_buffer && rtex->db_compatible)
			istate->compressed_depthtex_mask |= 1 << i;
		else
			istate->compressed_depthtex_mask &= ~(1 << i);

		if (!is_buffer && rtex->cmask.size)
			istate->compressed_colortex_mask |= 1 << i;
		else
			istate->compressed_colortex_mask &= ~(1 << i);
		if (!is_buffer) {

			evergreen_set_color_surface_common(rctx, rtex,
							   iview->u.tex.level,
							   iview->u.tex.first_layer,
							   iview->u.tex.last_layer,
							   iview->format,
							   &color);
			color.dim = S_028C78_WIDTH_MAX(u_minify(image->width0, iview->u.tex.level) - 1) |
			  S_028C78_HEIGHT_MAX(u_minify(image->height0, iview->u.tex.level) - 1);
		} else {
			color.offset = 0;
			color.view = 0;
			evergreen_set_color_surface_buffer(rctx, resource,
							   iview->format,
							   iview->u.buf.offset,
							   iview->u.buf.size,
							   &color);
		}

		switch (image->target) {
		case PIPE_BUFFER:
			res_type = V_028C70_BUFFER;
			break;
		case PIPE_TEXTURE_1D:
			res_type = V_028C70_TEXTURE1D;
			break;
		case PIPE_TEXTURE_1D_ARRAY:
			res_type = V_028C70_TEXTURE1DARRAY;
			break;
		case PIPE_TEXTURE_2D:
		case PIPE_TEXTURE_RECT:
			res_type = V_028C70_TEXTURE2D;
			break;
		case PIPE_TEXTURE_3D:
			res_type = V_028C70_TEXTURE3D;
			break;
		case PIPE_TEXTURE_2D_ARRAY:
		case PIPE_TEXTURE_CUBE:
		case PIPE_TEXTURE_CUBE_ARRAY:
			res_type = V_028C70_TEXTURE2DARRAY;
			break;
		default:
			assert(0);
			res_type = 0;
			break;
		}

		rview->cb_color_base = color.offset;
		rview->cb_color_dim = color.dim;
		rview->cb_color_info = color.info |
			S_028C70_RAT(1) |
			S_028C70_RESOURCE_TYPE(res_type);
		rview->cb_color_pitch = color.pitch;
		rview->cb_color_slice = color.slice;
		rview->cb_color_view = color.view;
		rview->cb_color_attrib = color.attrib;
		rview->cb_color_fmask = color.fmask;
		rview->cb_color_fmask_slice = color.fmask_slice;

		if (image->target != PIPE_BUFFER) {
			memset(&tex_params, 0, sizeof(tex_params));
			tex_params.pipe_format = iview->format;
			tex_params.force_level = 0;
			tex_params.width0 = image->width0;
			tex_params.height0 = image->height0;
			tex_params.first_level = iview->u.tex.level;
			tex_params.last_level = iview->u.tex.level;
			tex_params.first_layer = iview->u.tex.first_layer;
			tex_params.last_layer = iview->u.tex.last_layer;
			tex_params.target = image->target;
			tex_params.swizzle[0] = PIPE_SWIZZLE_X;
			tex_params.swizzle[1] = PIPE_SWIZZLE_Y;
			tex_params.swizzle[2] = PIPE_SWIZZLE_Z;
			tex_params.swizzle[3] = PIPE_SWIZZLE_W;
			evergreen_fill_tex_resource_words(rctx, &resource->b.b, &tex_params,
							  &rview->skip_mip_address_reloc,
							  rview->resource_words);

		} else {
			memset(&buf_params, 0, sizeof(buf_params));
			buf_params.pipe_format = iview->format;
			buf_params.size = iview->u.buf.size;
			buf_params.offset = iview->u.buf.offset;
			buf_params.swizzle[0] = PIPE_SWIZZLE_X;
			buf_params.swizzle[1] = PIPE_SWIZZLE_Y;
			buf_params.swizzle[2] = PIPE_SWIZZLE_Z;
			buf_params.swizzle[3] = PIPE_SWIZZLE_W;
			evergreen_fill_buffer_resource_words(rctx, &resource->b.b,
							     &buf_params,
							     &rview->skip_mip_address_reloc,
							     rview->resource_words);
		}
		istate->enabled_mask |= (1 << i);
	}

	for (i = start_slot + count, idx = 0;
	     i < start_slot + count + unbind_num_trailing_slots; i++, idx++) {
		rview = &istate->views[i];

		pipe_resource_reference((struct pipe_resource **)&rview->base.resource, NULL);
		istate->enabled_mask &= ~(1 << i);
		istate->compressed_colortex_mask &= ~(1 << i);
		istate->compressed_depthtex_mask &= ~(1 << i);
	}

	istate->atom.num_dw = util_bitcount(istate->enabled_mask) * 46;
	istate->dirty_buffer_constants = true;
	rctx->b.flags |= R600_CONTEXT_WAIT_3D_IDLE | R600_CONTEXT_FLUSH_AND_INV;
	rctx->b.flags |= R600_CONTEXT_FLUSH_AND_INV_CB |
		R600_CONTEXT_FLUSH_AND_INV_CB_META;

	if (old_mask != istate->enabled_mask)
		r600_mark_atom_dirty(rctx, &rctx->framebuffer.atom);

	if (rctx->cb_misc_state.image_rat_enabled_mask != istate->enabled_mask) {
		rctx->cb_misc_state.image_rat_enabled_mask = istate->enabled_mask;
		r600_mark_atom_dirty(rctx, &rctx->cb_misc_state.atom);
	}

	if (shader == PIPE_SHADER_FRAGMENT)
		r600_mark_atom_dirty(rctx, &istate->atom);
}

static void evergreen_get_pipe_constant_buffer(struct r600_context *rctx,
					       enum pipe_shader_type shader, uint slot,
					       struct pipe_constant_buffer *cbuf)
{
	struct r600_constbuf_state *state = &rctx->constbuf_state[shader];
	struct pipe_constant_buffer *cb;
	cbuf->user_buffer = NULL;

	cb = &state->cb[slot];

	cbuf->buffer_size = cb->buffer_size;
	pipe_resource_reference(&cbuf->buffer, cb->buffer);
}

static void evergreen_get_shader_buffers(struct r600_context *rctx,
					 enum pipe_shader_type shader,
					 uint start_slot, uint count,
					 struct pipe_shader_buffer *sbuf)
{
	assert(shader == PIPE_SHADER_COMPUTE);
	int idx, i;
	struct r600_image_state *istate = &rctx->compute_buffers;
	struct r600_image_view *rview;

	for (i = start_slot, idx = 0; i < start_slot + count; i++, idx++) {

		rview = &istate->views[i];

		pipe_resource_reference(&sbuf[idx].buffer, rview->base.resource);
		if (rview->base.resource) {
			uint64_t rview_va = ((struct r600_resource *)rview->base.resource)->gpu_address;

			uint64_t prog_va = rview->resource_words[0];

			prog_va += ((uint64_t)G_030008_BASE_ADDRESS_HI(rview->resource_words[2])) << 32;
			prog_va -= rview_va;

			sbuf[idx].buffer_offset = prog_va & 0xffffffff;
			sbuf[idx].buffer_size = rview->resource_words[1] + 1;;
		} else {
			sbuf[idx].buffer_offset = 0;
			sbuf[idx].buffer_size = 0;
		}
	}
}

static void evergreen_save_qbo_state(struct pipe_context *ctx, struct r600_qbo_state *st)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	st->saved_compute = rctx->cs_shader_state.shader;

	/* save constant buffer 0 */
	evergreen_get_pipe_constant_buffer(rctx, PIPE_SHADER_COMPUTE, 0, &st->saved_const0);
	/* save ssbo 0 */
	evergreen_get_shader_buffers(rctx, PIPE_SHADER_COMPUTE, 0, 3, st->saved_ssbo);
}


void evergreen_init_state_functions(struct r600_context *rctx)
{
	unsigned id = 1;
	unsigned i;
	/* !!!
	 *  To avoid GPU lockup registers must be emitted in a specific order
	 * (no kidding ...). The order below is important and have been
	 * partially inferred from analyzing fglrx command stream.
	 *
	 * Don't reorder atom without carefully checking the effect (GPU lockup
	 * or piglit regression).
	 * !!!
	 */
	if (rctx->b.gfx_level == EVERGREEN) {
		r600_init_atom(rctx, &rctx->config_state.atom, id++, evergreen_emit_config_state, 11);
		rctx->config_state.dyn_gpr_enabled = true;
	}
	r600_init_atom(rctx, &rctx->framebuffer.atom, id++, evergreen_emit_framebuffer_state, 0);
	r600_init_atom(rctx, &rctx->fragment_images.atom, id++, evergreen_emit_fragment_image_state, 0);
	r600_init_atom(rctx, &rctx->compute_images.atom, id++, evergreen_emit_compute_image_state, 0);
	r600_init_atom(rctx, &rctx->fragment_buffers.atom, id++, evergreen_emit_fragment_buffer_state, 0);
	r600_init_atom(rctx, &rctx->compute_buffers.atom, id++, evergreen_emit_compute_buffer_state, 0);
	/* shader const */
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_VERTEX].atom, id++, evergreen_emit_vs_constant_buffers, 0);
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_GEOMETRY].atom, id++, evergreen_emit_gs_constant_buffers, 0);
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_FRAGMENT].atom, id++, evergreen_emit_ps_constant_buffers, 0);
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_TESS_CTRL].atom, id++, evergreen_emit_tcs_constant_buffers, 0);
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_TESS_EVAL].atom, id++, evergreen_emit_tes_constant_buffers, 0);
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_COMPUTE].atom, id++, evergreen_emit_cs_constant_buffers, 0);
	/* shader program */
	r600_init_atom(rctx, &rctx->cs_shader_state.atom, id++, evergreen_emit_cs_shader, 0);
	/* sampler */
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_VERTEX].states.atom, id++, evergreen_emit_vs_sampler_states, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_GEOMETRY].states.atom, id++, evergreen_emit_gs_sampler_states, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_TESS_CTRL].states.atom, id++, evergreen_emit_tcs_sampler_states, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_TESS_EVAL].states.atom, id++, evergreen_emit_tes_sampler_states, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_FRAGMENT].states.atom, id++, evergreen_emit_ps_sampler_states, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_COMPUTE].states.atom, id++, evergreen_emit_cs_sampler_states, 0);
	/* resources */
	r600_init_atom(rctx, &rctx->vertex_buffer_state.atom, id++, evergreen_fs_emit_vertex_buffers, 0);
	r600_init_atom(rctx, &rctx->cs_vertex_buffer_state.atom, id++, evergreen_cs_emit_vertex_buffers, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_VERTEX].views.atom, id++, evergreen_emit_vs_sampler_views, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_GEOMETRY].views.atom, id++, evergreen_emit_gs_sampler_views, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_TESS_CTRL].views.atom, id++, evergreen_emit_tcs_sampler_views, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_TESS_EVAL].views.atom, id++, evergreen_emit_tes_sampler_views, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_FRAGMENT].views.atom, id++, evergreen_emit_ps_sampler_views, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_COMPUTE].views.atom, id++, evergreen_emit_cs_sampler_views, 0);

	r600_init_atom(rctx, &rctx->vgt_state.atom, id++, r600_emit_vgt_state, 10);

	if (rctx->b.gfx_level == EVERGREEN) {
		r600_init_atom(rctx, &rctx->sample_mask.atom, id++, evergreen_emit_sample_mask, 3);
	} else {
		r600_init_atom(rctx, &rctx->sample_mask.atom, id++, cayman_emit_sample_mask, 4);
	}
	rctx->sample_mask.sample_mask = ~0;

	r600_init_atom(rctx, &rctx->alphatest_state.atom, id++, r600_emit_alphatest_state, 6);
	r600_init_atom(rctx, &rctx->blend_color.atom, id++, r600_emit_blend_color, 6);
	r600_init_atom(rctx, &rctx->blend_state.atom, id++, r600_emit_cso_state, 0);
	r600_init_atom(rctx, &rctx->cb_misc_state.atom, id++, evergreen_emit_cb_misc_state, 4);
	r600_init_atom(rctx, &rctx->clip_misc_state.atom, id++, r600_emit_clip_misc_state, 9);
	r600_init_atom(rctx, &rctx->clip_state.atom, id++, evergreen_emit_clip_state, 26);
	r600_init_atom(rctx, &rctx->db_misc_state.atom, id++, evergreen_emit_db_misc_state, 10);
	r600_init_atom(rctx, &rctx->db_state.atom, id++, evergreen_emit_db_state, 14);
	r600_init_atom(rctx, &rctx->dsa_state.atom, id++, r600_emit_cso_state, 0);
	r600_init_atom(rctx, &rctx->poly_offset_state.atom, id++, evergreen_emit_polygon_offset, 9);
	r600_init_atom(rctx, &rctx->rasterizer_state.atom, id++, r600_emit_cso_state, 0);
	r600_add_atom(rctx, &rctx->b.scissors.atom, id++);
	r600_add_atom(rctx, &rctx->b.viewports.atom, id++);
	r600_init_atom(rctx, &rctx->stencil_ref.atom, id++, r600_emit_stencil_ref, 4);
	r600_init_atom(rctx, &rctx->vertex_fetch_shader.atom, id++, evergreen_emit_vertex_fetch_shader, 5);
	r600_add_atom(rctx, &rctx->b.render_cond_atom, id++);
	r600_add_atom(rctx, &rctx->b.streamout.begin_atom, id++);
	r600_add_atom(rctx, &rctx->b.streamout.enable_atom, id++);
	for (i = 0; i < EG_NUM_HW_STAGES; i++)
		r600_init_atom(rctx, &rctx->hw_shader_stages[i].atom, id++, r600_emit_shader, 0);
	r600_init_atom(rctx, &rctx->shader_stages.atom, id++, evergreen_emit_shader_stages, 15);
	r600_init_atom(rctx, &rctx->gs_rings.atom, id++, evergreen_emit_gs_rings, 26);

	rctx->b.b.create_blend_state = evergreen_create_blend_state;
	rctx->b.b.create_depth_stencil_alpha_state = evergreen_create_dsa_state;
	rctx->b.b.create_rasterizer_state = evergreen_create_rs_state;
	rctx->b.b.create_sampler_state = evergreen_create_sampler_state;
	rctx->b.b.create_sampler_view = evergreen_create_sampler_view;
	rctx->b.b.set_framebuffer_state = evergreen_set_framebuffer_state;
	rctx->b.b.set_polygon_stipple = evergreen_set_polygon_stipple;
	rctx->b.b.set_min_samples = evergreen_set_min_samples;
	rctx->b.b.set_tess_state = evergreen_set_tess_state;
	rctx->b.b.set_patch_vertices = evergreen_set_patch_vertices;
	rctx->b.b.set_hw_atomic_buffers = evergreen_set_hw_atomic_buffers;
	rctx->b.b.set_shader_images = evergreen_set_shader_images;
	rctx->b.b.set_shader_buffers = evergreen_set_shader_buffers;
	if (rctx->b.gfx_level == EVERGREEN)
                rctx->b.b.get_sample_position = evergreen_get_sample_position;
        else
                rctx->b.b.get_sample_position = cayman_get_sample_position;
	rctx->b.dma_copy = evergreen_dma_copy;
	rctx->b.save_qbo_state = evergreen_save_qbo_state;

	evergreen_init_compute_state_functions(rctx);
}

/**
 * This calculates the LDS size for tessellation shaders (VS, TCS, TES).
 *
 * The information about LDS and other non-compile-time parameters is then
 * written to the const buffer.

 * const buffer contains -
 * uint32_t input_patch_size
 * uint32_t input_vertex_size
 * uint32_t num_tcs_input_cp
 * uint32_t num_tcs_output_cp;
 * uint32_t output_patch_size
 * uint32_t output_vertex_size
 * uint32_t output_patch0_offset
 * uint32_t perpatch_output_offset
 * and the same constbuf is bound to LS/HS/VS(ES).
 */
void evergreen_setup_tess_constants(struct r600_context *rctx, const struct pipe_draw_info *info, unsigned *num_patches)
{
	struct pipe_constant_buffer constbuf = {0};
	struct r600_pipe_shader_selector *tcs = rctx->tcs_shader ? rctx->tcs_shader : rctx->tes_shader;
	struct r600_pipe_shader_selector *ls = rctx->vs_shader;
	unsigned num_tcs_input_cp = rctx->patch_vertices;
	unsigned num_tcs_outputs;
	unsigned num_tcs_output_cp;
	unsigned num_tcs_patch_outputs;
	unsigned num_tcs_inputs;
	unsigned input_vertex_size, output_vertex_size;
	unsigned input_patch_size, pervertex_output_patch_size, output_patch_size;
	unsigned output_patch0_offset, perpatch_output_offset, lds_size;
	uint32_t values[8];
	unsigned num_waves;
	unsigned num_pipes = rctx->screen->b.info.r600_max_quad_pipes;
	unsigned wave_divisor = (16 * num_pipes);

	*num_patches = 1;

	if (!rctx->tes_shader) {
		rctx->lds_alloc = 0;
		rctx->b.b.set_constant_buffer(&rctx->b.b, PIPE_SHADER_VERTEX,
					      R600_LDS_INFO_CONST_BUFFER, false, NULL);
		rctx->b.b.set_constant_buffer(&rctx->b.b, PIPE_SHADER_TESS_CTRL,
					      R600_LDS_INFO_CONST_BUFFER, false, NULL);
		rctx->b.b.set_constant_buffer(&rctx->b.b, PIPE_SHADER_TESS_EVAL,
					      R600_LDS_INFO_CONST_BUFFER, false, NULL);
		return;
	}

	if (rctx->lds_alloc != 0 &&
	    rctx->last_ls == ls &&
	    rctx->last_num_tcs_input_cp == num_tcs_input_cp &&
	    rctx->last_tcs == tcs)
		return;

	num_tcs_inputs = util_last_bit64(ls->lds_outputs_written_mask);

	if (rctx->tcs_shader) {
		num_tcs_outputs = util_last_bit64(tcs->lds_outputs_written_mask);
		num_tcs_output_cp = tcs->info.properties[TGSI_PROPERTY_TCS_VERTICES_OUT];
		num_tcs_patch_outputs = util_last_bit64(tcs->lds_patch_outputs_written_mask);
	} else {
		num_tcs_outputs = num_tcs_inputs;
		num_tcs_output_cp = num_tcs_input_cp;
		num_tcs_patch_outputs = 2; /* TESSINNER + TESSOUTER */
	}

	/* size in bytes */
	input_vertex_size = num_tcs_inputs * 16;
	output_vertex_size = num_tcs_outputs * 16;

	input_patch_size = num_tcs_input_cp * input_vertex_size;

	pervertex_output_patch_size = num_tcs_output_cp * output_vertex_size;
	output_patch_size = pervertex_output_patch_size + num_tcs_patch_outputs * 16;

	output_patch0_offset = rctx->tcs_shader ? input_patch_size * *num_patches : 0;
	perpatch_output_offset = output_patch0_offset + pervertex_output_patch_size;

	lds_size = output_patch0_offset + output_patch_size * *num_patches;

	values[0] = input_patch_size;
	values[1] = input_vertex_size;
	values[2] = num_tcs_input_cp;
	values[3] = num_tcs_output_cp;

	values[4] = output_patch_size;
	values[5] = output_vertex_size;
	values[6] = output_patch0_offset;
	values[7] = perpatch_output_offset;

	/* docs say HS_NUM_WAVES - CEIL((LS_HS_CONFIG.NUM_PATCHES *
	   LS_HS_CONFIG.HS_NUM_OUTPUT_CP) / (NUM_GOOD_PIPES * 16)) */
	num_waves = ceilf((float)(*num_patches * num_tcs_output_cp) / (float)wave_divisor);

	rctx->lds_alloc = (lds_size | (num_waves << 14));

	rctx->last_ls = ls;
	rctx->last_tcs = tcs;
	rctx->last_num_tcs_input_cp = num_tcs_input_cp;

	constbuf.user_buffer = values;
	constbuf.buffer_size = 8 * 4;

	rctx->b.b.set_constant_buffer(&rctx->b.b, PIPE_SHADER_VERTEX,
				      R600_LDS_INFO_CONST_BUFFER, false, &constbuf);
	rctx->b.b.set_constant_buffer(&rctx->b.b, PIPE_SHADER_TESS_CTRL,
				      R600_LDS_INFO_CONST_BUFFER, false, &constbuf);
	rctx->b.b.set_constant_buffer(&rctx->b.b, PIPE_SHADER_TESS_EVAL,
				      R600_LDS_INFO_CONST_BUFFER, true, &constbuf);
}

uint32_t evergreen_get_ls_hs_config(struct r600_context *rctx,
				    const struct pipe_draw_info *info,
				    unsigned num_patches)
{
	unsigned num_output_cp;

	if (!rctx->tes_shader)
		return 0;

	num_output_cp = rctx->tcs_shader ?
		rctx->tcs_shader->info.properties[TGSI_PROPERTY_TCS_VERTICES_OUT] :
		rctx->patch_vertices;

	return S_028B58_NUM_PATCHES(num_patches) |
		S_028B58_HS_NUM_INPUT_CP(rctx->patch_vertices) |
		S_028B58_HS_NUM_OUTPUT_CP(num_output_cp);
}

void evergreen_set_ls_hs_config(struct r600_context *rctx,
				struct radeon_cmdbuf *cs,
				uint32_t ls_hs_config)
{
	radeon_set_context_reg(cs, R_028B58_VGT_LS_HS_CONFIG, ls_hs_config);
}

void evergreen_set_lds_alloc(struct r600_context *rctx,
			     struct radeon_cmdbuf *cs,
			     uint32_t lds_alloc)
{
	radeon_set_context_reg(cs, R_0288E8_SQ_LDS_ALLOC, lds_alloc);
}

/* on evergreen if you are running tessellation you need to disable dynamic
   GPRs to workaround a hardware bug.*/
bool evergreen_adjust_gprs(struct r600_context *rctx)
{
	unsigned num_gprs[EG_NUM_HW_STAGES];
	unsigned def_gprs[EG_NUM_HW_STAGES];
	unsigned cur_gprs[EG_NUM_HW_STAGES];
	unsigned new_gprs[EG_NUM_HW_STAGES];
	unsigned def_num_clause_temp_gprs = rctx->r6xx_num_clause_temp_gprs;
	unsigned max_gprs;
	unsigned i;
	unsigned total_gprs;
	unsigned tmp[3];
	bool rework = false, set_default = false, set_dirty = false;
	max_gprs = 0;
	for (i = 0; i < EG_NUM_HW_STAGES; i++) {
		def_gprs[i] = rctx->default_gprs[i];
		max_gprs += def_gprs[i];
	}
	max_gprs += def_num_clause_temp_gprs * 2;

	/* if we have no TESS and dyn gpr is enabled then do nothing. */
	if (!rctx->hw_shader_stages[EG_HW_STAGE_HS].shader) {
		if (rctx->config_state.dyn_gpr_enabled)
			return true;

		/* transition back to dyn gpr enabled state */
		rctx->config_state.dyn_gpr_enabled = true;
		r600_mark_atom_dirty(rctx, &rctx->config_state.atom);
		rctx->b.flags |= R600_CONTEXT_WAIT_3D_IDLE;
		return true;
	}


	/* gather required shader gprs */
	for (i = 0; i < EG_NUM_HW_STAGES; i++) {
		if (rctx->hw_shader_stages[i].shader)
			num_gprs[i] = rctx->hw_shader_stages[i].shader->shader.bc.ngpr;
		else
			num_gprs[i] = 0;
	}

	cur_gprs[R600_HW_STAGE_PS] = G_008C04_NUM_PS_GPRS(rctx->config_state.sq_gpr_resource_mgmt_1);
	cur_gprs[R600_HW_STAGE_VS] = G_008C04_NUM_VS_GPRS(rctx->config_state.sq_gpr_resource_mgmt_1);
	cur_gprs[R600_HW_STAGE_GS] = G_008C08_NUM_GS_GPRS(rctx->config_state.sq_gpr_resource_mgmt_2);
	cur_gprs[R600_HW_STAGE_ES] = G_008C08_NUM_ES_GPRS(rctx->config_state.sq_gpr_resource_mgmt_2);
	cur_gprs[EG_HW_STAGE_LS] = G_008C0C_NUM_LS_GPRS(rctx->config_state.sq_gpr_resource_mgmt_3);
	cur_gprs[EG_HW_STAGE_HS] = G_008C0C_NUM_HS_GPRS(rctx->config_state.sq_gpr_resource_mgmt_3);

	total_gprs = 0;
	for (i = 0; i < EG_NUM_HW_STAGES; i++)	{
		new_gprs[i] = num_gprs[i];
		total_gprs += num_gprs[i];
	}

	if (total_gprs > (max_gprs - (2 * def_num_clause_temp_gprs)))
		return false;

	for (i = 0; i < EG_NUM_HW_STAGES; i++) {
		if (new_gprs[i] > cur_gprs[i]) {
			rework = true;
			break;
		}
	}

	if (rctx->config_state.dyn_gpr_enabled) {
		set_dirty = true;
		rctx->config_state.dyn_gpr_enabled = false;
	}

	if (rework) {
		set_default = true;
		for (i = 0; i < EG_NUM_HW_STAGES; i++) {
			if (new_gprs[i] > def_gprs[i])
				set_default = false;
		}

		if (set_default) {
			for (i = 0; i < EG_NUM_HW_STAGES; i++) {
				new_gprs[i] = def_gprs[i];
			}
		} else {
			unsigned ps_value = max_gprs;

			ps_value -= (def_num_clause_temp_gprs * 2);
			for (i = R600_HW_STAGE_VS; i < EG_NUM_HW_STAGES; i++)
				ps_value -= new_gprs[i];

			new_gprs[R600_HW_STAGE_PS] = ps_value;
		}

		tmp[0] = S_008C04_NUM_PS_GPRS(new_gprs[R600_HW_STAGE_PS]) |
			S_008C04_NUM_VS_GPRS(new_gprs[R600_HW_STAGE_VS]) |
			S_008C04_NUM_CLAUSE_TEMP_GPRS(def_num_clause_temp_gprs);

		tmp[1] = S_008C08_NUM_ES_GPRS(new_gprs[R600_HW_STAGE_ES]) |
			S_008C08_NUM_GS_GPRS(new_gprs[R600_HW_STAGE_GS]);

		tmp[2] = S_008C0C_NUM_HS_GPRS(new_gprs[EG_HW_STAGE_HS]) |
			S_008C0C_NUM_LS_GPRS(new_gprs[EG_HW_STAGE_LS]);

		if (rctx->config_state.sq_gpr_resource_mgmt_1 != tmp[0] ||
		    rctx->config_state.sq_gpr_resource_mgmt_2 != tmp[1] ||
		    rctx->config_state.sq_gpr_resource_mgmt_3 != tmp[2]) {
			rctx->config_state.sq_gpr_resource_mgmt_1 = tmp[0];
			rctx->config_state.sq_gpr_resource_mgmt_2 = tmp[1];
			rctx->config_state.sq_gpr_resource_mgmt_3 = tmp[2];
			set_dirty = true;
		}
	}


	if (set_dirty) {
		r600_mark_atom_dirty(rctx, &rctx->config_state.atom);
		rctx->b.flags |= R600_CONTEXT_WAIT_3D_IDLE;
	}
	return true;
}

#define AC_ENCODE_TRACE_POINT(id)       (0xcafe0000 | ((id) & 0xffff))

void eg_trace_emit(struct r600_context *rctx)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	unsigned reloc;

	if (rctx->b.gfx_level < EVERGREEN)
		return;

	/* This must be done after r600_need_cs_space. */
	reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
					  (struct r600_resource*)rctx->trace_buf, RADEON_USAGE_WRITE |
					  RADEON_PRIO_CP_DMA);

	rctx->trace_id++;
	radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rctx->trace_buf,
			      RADEON_USAGE_READWRITE | RADEON_PRIO_FENCE_TRACE);
	radeon_emit(cs, PKT3(PKT3_MEM_WRITE, 3, 0));
	radeon_emit(cs, rctx->trace_buf->gpu_address);
	radeon_emit(cs, rctx->trace_buf->gpu_address >> 32 | MEM_WRITE_32_BITS | MEM_WRITE_CONFIRM);
	radeon_emit(cs, rctx->trace_id);
	radeon_emit(cs, 0);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, reloc);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, AC_ENCODE_TRACE_POINT(rctx->trace_id));
}

static void evergreen_emit_set_append_cnt(struct r600_context *rctx,
					  struct r600_shader_atomic *atomic,
					  struct r600_resource *resource,
					  uint32_t pkt_flags)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	unsigned reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
						   resource,
						   RADEON_USAGE_READ |
						   RADEON_PRIO_SHADER_RW_BUFFER);
	uint64_t dst_offset = resource->gpu_address + (atomic->start * 4);
	uint32_t base_reg_0 = R_02872C_GDS_APPEND_COUNT_0;

	uint32_t reg_val = (base_reg_0 + atomic->hw_idx * 4 - EVERGREEN_CONTEXT_REG_OFFSET) >> 2;

	radeon_emit(cs, PKT3(PKT3_SET_APPEND_CNT, 2, 0) | pkt_flags);
	radeon_emit(cs, (reg_val << 16) | 0x3);
	radeon_emit(cs, dst_offset & 0xfffffffc);
	radeon_emit(cs, (dst_offset >> 32) & 0xff);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, reloc);
}

static void evergreen_emit_event_write_eos(struct r600_context *rctx,
					   struct r600_shader_atomic *atomic,
					   struct r600_resource *resource,
					   uint32_t pkt_flags)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint32_t event = EVENT_TYPE_PS_DONE;
	uint32_t base_reg_0 = R_02872C_GDS_APPEND_COUNT_0;
	uint32_t reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
						   resource,
						   RADEON_USAGE_WRITE |
						   RADEON_PRIO_SHADER_RW_BUFFER);
	uint64_t dst_offset = resource->gpu_address + (atomic->start * 4);
	uint32_t reg_val = (base_reg_0 + atomic->hw_idx * 4) >> 2;

	if (pkt_flags == RADEON_CP_PACKET3_COMPUTE_MODE)
		event = EVENT_TYPE_CS_DONE;

	radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_EOS, 3, 0) | pkt_flags);
	radeon_emit(cs, EVENT_TYPE(event) | EVENT_INDEX(6));
	radeon_emit(cs, (dst_offset) & 0xffffffff);
	radeon_emit(cs, (0 << 29) | ((dst_offset >> 32) & 0xff));
	radeon_emit(cs, reg_val);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, reloc);
}

static void cayman_emit_event_write_eos(struct r600_context *rctx,
					struct r600_shader_atomic *atomic,
					struct r600_resource *resource,
					uint32_t pkt_flags)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint32_t event = EVENT_TYPE_PS_DONE;
	uint32_t reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
						   resource,
						   RADEON_USAGE_WRITE |
						   RADEON_PRIO_SHADER_RW_BUFFER);
	uint64_t dst_offset = resource->gpu_address + (atomic->start * 4);

	if (pkt_flags == RADEON_CP_PACKET3_COMPUTE_MODE)
		event = EVENT_TYPE_CS_DONE;

	radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_EOS, 3, 0) | pkt_flags);
	radeon_emit(cs, EVENT_TYPE(event) | EVENT_INDEX(6));
	radeon_emit(cs, (dst_offset) & 0xffffffff);
	radeon_emit(cs, (1 << 29) | ((dst_offset >> 32) & 0xff));
	radeon_emit(cs, (atomic->hw_idx) | (1 << 16));
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, reloc);
}

/* writes count from a buffer into GDS */
static void cayman_write_count_to_gds(struct r600_context *rctx,
				      struct r600_shader_atomic *atomic,
				      struct r600_resource *resource,
				      uint32_t pkt_flags)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	unsigned reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
						   resource,
						   RADEON_USAGE_READ |
						   RADEON_PRIO_SHADER_RW_BUFFER);
	uint64_t dst_offset = resource->gpu_address + (atomic->start * 4);

	radeon_emit(cs, PKT3(PKT3_CP_DMA, 4, 0) | pkt_flags);
	radeon_emit(cs, dst_offset & 0xffffffff);
	radeon_emit(cs, PKT3_CP_DMA_CP_SYNC | PKT3_CP_DMA_DST_SEL(1) | ((dst_offset >> 32) & 0xff));// GDS
	radeon_emit(cs, atomic->hw_idx * 4);
	radeon_emit(cs, 0);
	radeon_emit(cs, PKT3_CP_DMA_CMD_DAS | 4);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, reloc);
}

void evergreen_emit_atomic_buffer_setup_count(struct r600_context *rctx,
					      struct r600_pipe_shader *cs_shader,
					      struct r600_shader_atomic *combined_atomics,
					      uint8_t *atomic_used_mask_p)
{
	uint8_t atomic_used_mask = 0;
	int i, j, k;
	bool is_compute = cs_shader ? true : false;

	for (i = 0; i < (is_compute ? 1 : EG_NUM_HW_STAGES); i++) {
		uint8_t num_atomic_stage;
		struct r600_pipe_shader *pshader;

		if (is_compute)
			pshader = cs_shader;
		else
			pshader = rctx->hw_shader_stages[i].shader;
		if (!pshader)
			continue;

		num_atomic_stage = pshader->shader.nhwatomic_ranges;
		if (!num_atomic_stage)
			continue;

		for (j = 0; j < num_atomic_stage; j++) {
			struct r600_shader_atomic *atomic = &pshader->shader.atomics[j];
			int natomics = atomic->end - atomic->start + 1;

			for (k = 0; k < natomics; k++) {
				/* seen this in a previous stage */
				if (atomic_used_mask & (1u << (atomic->hw_idx + k)))
					continue;

				combined_atomics[atomic->hw_idx + k].hw_idx = atomic->hw_idx + k;
				combined_atomics[atomic->hw_idx + k].buffer_id = atomic->buffer_id;
				combined_atomics[atomic->hw_idx + k].start = atomic->start + k;
				combined_atomics[atomic->hw_idx + k].end = combined_atomics[atomic->hw_idx + k].start + 1;
				atomic_used_mask |= (1u << (atomic->hw_idx + k));
			}
		}
	}
	*atomic_used_mask_p = atomic_used_mask;
}

void evergreen_emit_atomic_buffer_setup(struct r600_context *rctx,
					bool is_compute,
					struct r600_shader_atomic *combined_atomics,
					uint8_t atomic_used_mask)
{
	struct r600_atomic_buffer_state *astate = &rctx->atomic_buffer_state;
	unsigned pkt_flags = 0;
	uint32_t mask;

	if (is_compute)
		pkt_flags = RADEON_CP_PACKET3_COMPUTE_MODE;

	mask = atomic_used_mask;
	if (!mask)
		return;

	while (mask) {
		unsigned atomic_index = u_bit_scan(&mask);
		struct r600_shader_atomic *atomic = &combined_atomics[atomic_index];
		struct r600_resource *resource = r600_resource(astate->buffer[atomic->buffer_id].buffer);
		assert(resource);

		if (rctx->b.gfx_level == CAYMAN)
			cayman_write_count_to_gds(rctx, atomic, resource, pkt_flags);
		else
			evergreen_emit_set_append_cnt(rctx, atomic, resource, pkt_flags);
	}
}

void evergreen_emit_atomic_buffer_save(struct r600_context *rctx,
				       bool is_compute,
				       struct r600_shader_atomic *combined_atomics,
				       uint8_t *atomic_used_mask_p)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_atomic_buffer_state *astate = &rctx->atomic_buffer_state;
	uint32_t pkt_flags = 0;
	uint32_t event = EVENT_TYPE_PS_DONE;
	uint32_t mask;
	uint64_t dst_offset;
	unsigned reloc;

	if (is_compute)
		pkt_flags = RADEON_CP_PACKET3_COMPUTE_MODE;

	mask = *atomic_used_mask_p;
	if (!mask)
		return;

	while (mask) {
		unsigned atomic_index = u_bit_scan(&mask);
		struct r600_shader_atomic *atomic = &combined_atomics[atomic_index];
		struct r600_resource *resource = r600_resource(astate->buffer[atomic->buffer_id].buffer);
		assert(resource);

		if (rctx->b.gfx_level == CAYMAN)
			cayman_emit_event_write_eos(rctx, atomic, resource, pkt_flags);
		else
			evergreen_emit_event_write_eos(rctx, atomic, resource, pkt_flags);
	}

	if (pkt_flags == RADEON_CP_PACKET3_COMPUTE_MODE)
		event = EVENT_TYPE_CS_DONE;

	++rctx->append_fence_id;
	reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
					  r600_resource(rctx->append_fence),
					  RADEON_USAGE_READWRITE |
					  RADEON_PRIO_SHADER_RW_BUFFER);
	dst_offset = r600_resource(rctx->append_fence)->gpu_address;
	radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_EOS, 3, 0) | pkt_flags);
	radeon_emit(cs, EVENT_TYPE(event) | EVENT_INDEX(6));
	radeon_emit(cs, dst_offset & 0xffffffff);
	radeon_emit(cs, (2 << 29) | ((dst_offset >> 32) & 0xff));
	radeon_emit(cs, rctx->append_fence_id);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, reloc);

	radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0) | pkt_flags);
	radeon_emit(cs, WAIT_REG_MEM_GEQUAL | WAIT_REG_MEM_MEMORY | (1 << 8));
	radeon_emit(cs, dst_offset & 0xffffffff);
	radeon_emit(cs, ((dst_offset >> 32) & 0xff));
	radeon_emit(cs, rctx->append_fence_id);
	radeon_emit(cs, 0xffffffff);
	radeon_emit(cs, 0xa);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, reloc);
}
