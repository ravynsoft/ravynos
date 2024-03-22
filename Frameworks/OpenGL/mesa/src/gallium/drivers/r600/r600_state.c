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
#include "r600d.h"
#include "r600d_common.h"

#include "pipe/p_shader_tokens.h"
#include "util/u_endian.h"
#include "util/u_pack_color.h"
#include "util/u_memory.h"
#include "util/u_framebuffer.h"
#include "util/u_dual_blend.h"

#include <assert.h>

static uint32_t r600_translate_blend_function(int blend_func)
{
	switch (blend_func) {
	case PIPE_BLEND_ADD:
		return V_028804_COMB_DST_PLUS_SRC;
	case PIPE_BLEND_SUBTRACT:
		return V_028804_COMB_SRC_MINUS_DST;
	case PIPE_BLEND_REVERSE_SUBTRACT:
		return V_028804_COMB_DST_MINUS_SRC;
	case PIPE_BLEND_MIN:
		return V_028804_COMB_MIN_DST_SRC;
	case PIPE_BLEND_MAX:
		return V_028804_COMB_MAX_DST_SRC;
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
		return V_028804_BLEND_ONE;
	case PIPE_BLENDFACTOR_SRC_COLOR:
		return V_028804_BLEND_SRC_COLOR;
	case PIPE_BLENDFACTOR_SRC_ALPHA:
		return V_028804_BLEND_SRC_ALPHA;
	case PIPE_BLENDFACTOR_DST_ALPHA:
		return V_028804_BLEND_DST_ALPHA;
	case PIPE_BLENDFACTOR_DST_COLOR:
		return V_028804_BLEND_DST_COLOR;
	case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
		return V_028804_BLEND_SRC_ALPHA_SATURATE;
	case PIPE_BLENDFACTOR_CONST_COLOR:
		return V_028804_BLEND_CONST_COLOR;
	case PIPE_BLENDFACTOR_CONST_ALPHA:
		return V_028804_BLEND_CONST_ALPHA;
	case PIPE_BLENDFACTOR_ZERO:
		return V_028804_BLEND_ZERO;
	case PIPE_BLENDFACTOR_INV_SRC_COLOR:
		return V_028804_BLEND_ONE_MINUS_SRC_COLOR;
	case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
		return V_028804_BLEND_ONE_MINUS_SRC_ALPHA;
	case PIPE_BLENDFACTOR_INV_DST_ALPHA:
		return V_028804_BLEND_ONE_MINUS_DST_ALPHA;
	case PIPE_BLENDFACTOR_INV_DST_COLOR:
		return V_028804_BLEND_ONE_MINUS_DST_COLOR;
	case PIPE_BLENDFACTOR_INV_CONST_COLOR:
		return V_028804_BLEND_ONE_MINUS_CONST_COLOR;
	case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
		return V_028804_BLEND_ONE_MINUS_CONST_ALPHA;
	case PIPE_BLENDFACTOR_SRC1_COLOR:
		return V_028804_BLEND_SRC1_COLOR;
	case PIPE_BLENDFACTOR_SRC1_ALPHA:
		return V_028804_BLEND_SRC1_ALPHA;
	case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
		return V_028804_BLEND_INV_SRC1_COLOR;
	case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
		return V_028804_BLEND_INV_SRC1_ALPHA;
	default:
		R600_ERR("Bad blend factor %d not supported!\n", blend_fact);
		assert(0);
		break;
	}
	return 0;
}

static unsigned r600_tex_dim(unsigned dim, unsigned nr_samples)
{
	switch (dim) {
	default:
	case PIPE_TEXTURE_1D:
		return V_038000_SQ_TEX_DIM_1D;
	case PIPE_TEXTURE_1D_ARRAY:
		return V_038000_SQ_TEX_DIM_1D_ARRAY;
	case PIPE_TEXTURE_2D:
	case PIPE_TEXTURE_RECT:
		return nr_samples > 1 ? V_038000_SQ_TEX_DIM_2D_MSAA :
					V_038000_SQ_TEX_DIM_2D;
	case PIPE_TEXTURE_2D_ARRAY:
		return nr_samples > 1 ? V_038000_SQ_TEX_DIM_2D_ARRAY_MSAA :
					V_038000_SQ_TEX_DIM_2D_ARRAY;
	case PIPE_TEXTURE_3D:
		return V_038000_SQ_TEX_DIM_3D;
	case PIPE_TEXTURE_CUBE:
	case PIPE_TEXTURE_CUBE_ARRAY:
		return V_038000_SQ_TEX_DIM_CUBEMAP;
	}
}

static uint32_t r600_translate_dbformat(enum pipe_format format)
{
	switch (format) {
	case PIPE_FORMAT_Z16_UNORM:
		return V_028010_DEPTH_16;
	case PIPE_FORMAT_Z24X8_UNORM:
		return V_028010_DEPTH_X8_24;
	case PIPE_FORMAT_Z24_UNORM_S8_UINT:
		return V_028010_DEPTH_8_24;
	case PIPE_FORMAT_Z32_FLOAT:
		return V_028010_DEPTH_32_FLOAT;
	case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
		return V_028010_DEPTH_X24_8_32_FLOAT;
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

bool r600_is_format_supported(struct pipe_screen *screen,
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

		/* R11G11B10 is broken on R6xx. */
		if (rscreen->b.gfx_level == R600 &&
		    format == PIPE_FORMAT_R11G11B10_FLOAT)
			return false;

		/* MSAA integer colorbuffers hang. */
		if (util_format_is_pure_integer(format) &&
		    !util_format_is_depth_or_stencil(format))
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

static void r600_emit_polygon_offset(struct r600_context *rctx, struct r600_atom *a)
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
			offset_units *= 2.0f;
			pa_su_poly_offset_db_fmt_cntl =
				S_028DF8_POLY_OFFSET_NEG_NUM_DB_BITS((char)-24);
			break;
		case PIPE_FORMAT_Z16_UNORM:
			offset_units *= 4.0f;
			pa_su_poly_offset_db_fmt_cntl =
				S_028DF8_POLY_OFFSET_NEG_NUM_DB_BITS((char)-16);
			break;
		default:
			pa_su_poly_offset_db_fmt_cntl =
				S_028DF8_POLY_OFFSET_NEG_NUM_DB_BITS((char)-23) |
				S_028DF8_POLY_OFFSET_DB_IS_FLOAT_FMT(1);
		}
	}

	radeon_set_context_reg_seq(cs, R_028E00_PA_SU_POLY_OFFSET_FRONT_SCALE, 4);
	radeon_emit(cs, fui(offset_scale));
	radeon_emit(cs, fui(offset_units));
	radeon_emit(cs, fui(offset_scale));
	radeon_emit(cs, fui(offset_units));

	radeon_set_context_reg(cs, R_028DF8_PA_SU_POLY_OFFSET_DB_FMT_CNTL,
			       pa_su_poly_offset_db_fmt_cntl);
}

static uint32_t r600_get_blend_control(const struct pipe_blend_state *state, unsigned i)
{
	int j = state->independent_blend_enable ? i : 0;

	unsigned eqRGB = state->rt[j].rgb_func;
	unsigned srcRGB = state->rt[j].rgb_src_factor;
	unsigned dstRGB = state->rt[j].rgb_dst_factor;

	unsigned eqA = state->rt[j].alpha_func;
	unsigned srcA = state->rt[j].alpha_src_factor;
	unsigned dstA = state->rt[j].alpha_dst_factor;
	uint32_t bc = 0;

	if (!state->rt[j].blend_enable)
		return 0;

	bc |= S_028804_COLOR_COMB_FCN(r600_translate_blend_function(eqRGB));
	bc |= S_028804_COLOR_SRCBLEND(r600_translate_blend_factor(srcRGB));
	bc |= S_028804_COLOR_DESTBLEND(r600_translate_blend_factor(dstRGB));

	if (srcA != srcRGB || dstA != dstRGB || eqA != eqRGB) {
		bc |= S_028804_SEPARATE_ALPHA_BLEND(1);
		bc |= S_028804_ALPHA_COMB_FCN(r600_translate_blend_function(eqA));
		bc |= S_028804_ALPHA_SRCBLEND(r600_translate_blend_factor(srcA));
		bc |= S_028804_ALPHA_DESTBLEND(r600_translate_blend_factor(dstA));
	}
	return bc;
}

static void *r600_create_blend_state_mode(struct pipe_context *ctx,
					  const struct pipe_blend_state *state,
					  int mode)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	uint32_t color_control = 0, target_mask = 0;
	struct r600_blend_state *blend = CALLOC_STRUCT(r600_blend_state);

	if (!blend) {
		return NULL;
	}

	r600_init_command_buffer(&blend->buffer, 20);
	r600_init_command_buffer(&blend->buffer_no_blend, 20);

	/* R600 does not support per-MRT blends */
	if (rctx->b.family > CHIP_R600)
		color_control |= S_028808_PER_MRT_BLEND(1);

	if (state->logicop_enable) {
		color_control |= (state->logicop_func << 16) | (state->logicop_func << 20);
	} else {
		color_control |= (0xcc << 16);
	}
	/* we pretend 8 buffer are used, CB_SHADER_MASK will disable unused one */
	if (state->independent_blend_enable) {
		for (int i = 0; i < 8; i++) {
			if (state->rt[i].blend_enable) {
				color_control |= S_028808_TARGET_BLEND_ENABLE(1 << i);
			}
			target_mask |= (state->rt[i].colormask << (4 * i));
		}
	} else {
		for (int i = 0; i < 8; i++) {
			if (state->rt[0].blend_enable) {
				color_control |= S_028808_TARGET_BLEND_ENABLE(1 << i);
			}
			target_mask |= (state->rt[0].colormask << (4 * i));
		}
	}

	if (target_mask)
		color_control |= S_028808_SPECIAL_OP(mode);
	else
		color_control |= S_028808_SPECIAL_OP(V_028808_DISABLE);

	/* only MRT0 has dual src blend */
	blend->dual_src_blend = util_blend_state_is_dual(state, 0);
	blend->cb_target_mask = target_mask;
	blend->cb_color_control = color_control;
	blend->cb_color_control_no_blend = color_control & C_028808_TARGET_BLEND_ENABLE;
	blend->alpha_to_one = state->alpha_to_one;

	r600_store_context_reg(&blend->buffer, R_028D44_DB_ALPHA_TO_MASK,
			       S_028D44_ALPHA_TO_MASK_ENABLE(state->alpha_to_coverage) |
			       S_028D44_ALPHA_TO_MASK_OFFSET0(2) |
			       S_028D44_ALPHA_TO_MASK_OFFSET1(2) |
			       S_028D44_ALPHA_TO_MASK_OFFSET2(2) |
			       S_028D44_ALPHA_TO_MASK_OFFSET3(2));

	/* Copy over the registers set so far into buffer_no_blend. */
	memcpy(blend->buffer_no_blend.buf, blend->buffer.buf, blend->buffer.num_dw * 4);
	blend->buffer_no_blend.num_dw = blend->buffer.num_dw;

	/* Only add blend registers if blending is enabled. */
	if (!G_028808_TARGET_BLEND_ENABLE(color_control)) {
		return blend;
	}

	/* The first R600 does not support per-MRT blends */
	r600_store_context_reg(&blend->buffer, R_028804_CB_BLEND_CONTROL,
			       r600_get_blend_control(state, 0));

	if (rctx->b.family > CHIP_R600) {
		r600_store_context_reg_seq(&blend->buffer, R_028780_CB_BLEND0_CONTROL, 8);
		for (int i = 0; i < 8; i++) {
			r600_store_value(&blend->buffer, r600_get_blend_control(state, i));
		}
	}
	return blend;
}

static void *r600_create_blend_state(struct pipe_context *ctx,
				     const struct pipe_blend_state *state)
{
	return r600_create_blend_state_mode(ctx, state, V_028808_SPECIAL_NORMAL);
}

static void *r600_create_dsa_state(struct pipe_context *ctx,
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

	r600_store_context_reg(&dsa->buffer, R_028800_DB_DEPTH_CONTROL, db_depth_control);
	return dsa;
}

static void *r600_create_rs_state(struct pipe_context *ctx,
				  const struct pipe_rasterizer_state *state)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	unsigned tmp, sc_mode_cntl, spi_interp;
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
		S_028810_DX_LINEAR_ATTR_CLIP_ENA(1);
	if (rctx->b.gfx_level == R700) {
		rs->pa_cl_clip_cntl |=
			S_028810_DX_RASTERIZATION_KILL(state->rasterizer_discard);
	}
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

	sc_mode_cntl = S_028A4C_MSAA_ENABLE(state->multisample) |
		       S_028A4C_LINE_STIPPLE_ENABLE(state->line_stipple_enable) |
		       S_028A4C_FORCE_EOV_CNTDWN_ENABLE(1) |
		       S_028A4C_PS_ITER_SAMPLE(state->multisample && rctx->ps_iter_samples > 1);
	if (rctx->b.family == CHIP_RV770) {
		/* workaround possible rendering corruption on RV770 with hyperz together with sample shading */
		sc_mode_cntl |= S_028A4C_TILE_COVER_DISABLE(state->multisample && rctx->ps_iter_samples > 1);
	}
	if (rctx->b.gfx_level >= R700) {
		sc_mode_cntl |= S_028A4C_FORCE_EOV_REZ_ENABLE(1) |
				S_028A4C_R700_ZMM_LINE_OFFSET(1) |
				S_028A4C_R700_VPORT_SCISSOR_ENABLE(1);
	} else {
		sc_mode_cntl |= S_028A4C_WALK_ALIGN8_PRIM_FITS_ST(1);
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
	/* point size 12.4 fixed point (divide by two, because 0.5 = 1 pixel. */
	tmp = r600_pack_float_12p4(state->point_size/2);
	r600_store_value(&rs->buffer, /* R_028A00_PA_SU_POINT_SIZE */
			 S_028A00_HEIGHT(tmp) | S_028A00_WIDTH(tmp));
	r600_store_value(&rs->buffer, /* R_028A04_PA_SU_POINT_MINMAX */
			 S_028A04_MIN_SIZE(r600_pack_float_12p4(psize_min/2)) |
			 S_028A04_MAX_SIZE(r600_pack_float_12p4(psize_max/2)));
	r600_store_value(&rs->buffer, /* R_028A08_PA_SU_LINE_CNTL */
			 S_028A08_WIDTH(r600_pack_float_12p4(state->line_width/2)));

	r600_store_context_reg(&rs->buffer, R_0286D4_SPI_INTERP_CONTROL_0, spi_interp);
	r600_store_context_reg(&rs->buffer, R_028A4C_PA_SC_MODE_CNTL, sc_mode_cntl);
	r600_store_context_reg(&rs->buffer, R_028C08_PA_SU_VTX_CNTL,
			       S_028C08_PIX_CENTER_HALF(state->half_pixel_center) |
			       S_028C08_QUANT_MODE(V_028C08_X_1_256TH));
	r600_store_context_reg(&rs->buffer, R_028DFC_PA_SU_POLY_OFFSET_CLAMP, fui(state->offset_clamp));

	rs->pa_su_sc_mode_cntl = S_028814_PROVOKING_VTX_LAST(!state->flatshade_first) |
				 S_028814_CULL_FRONT(state->cull_face & PIPE_FACE_FRONT ? 1 : 0) |
				 S_028814_CULL_BACK(state->cull_face & PIPE_FACE_BACK ? 1 : 0) |
				 S_028814_FACE(!state->front_ccw) |
				 S_028814_POLY_OFFSET_FRONT_ENABLE(util_get_offset(state, state->fill_front)) |
				 S_028814_POLY_OFFSET_BACK_ENABLE(util_get_offset(state, state->fill_back)) |
				 S_028814_POLY_OFFSET_PARA_ENABLE(state->offset_point || state->offset_line) |
				 S_028814_POLY_MODE(state->fill_front != PIPE_POLYGON_MODE_FILL ||
									 state->fill_back != PIPE_POLYGON_MODE_FILL) |
				 S_028814_POLYMODE_FRONT_PTYPE(r600_translate_fill(state->fill_front)) |
				 S_028814_POLYMODE_BACK_PTYPE(r600_translate_fill(state->fill_back));
	if (rctx->b.gfx_level == R700) {
		r600_store_context_reg(&rs->buffer, R_028814_PA_SU_SC_MODE_CNTL, rs->pa_su_sc_mode_cntl);
	}
	if (rctx->b.gfx_level == R600) {
		r600_store_context_reg(&rs->buffer, R_028350_SX_MISC,
				       S_028350_MULTIPASS(state->rasterizer_discard));
	}
	return rs;
}

static unsigned r600_tex_filter(unsigned filter, unsigned max_aniso)
{
	if (filter == PIPE_TEX_FILTER_LINEAR)
		return max_aniso > 1 ? V_03C000_SQ_TEX_XY_FILTER_ANISO_BILINEAR
				     : V_03C000_SQ_TEX_XY_FILTER_BILINEAR;
	else
		return max_aniso > 1 ? V_03C000_SQ_TEX_XY_FILTER_ANISO_POINT
				     : V_03C000_SQ_TEX_XY_FILTER_POINT;
}

static void *r600_create_sampler_state(struct pipe_context *ctx,
					const struct pipe_sampler_state *state)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)ctx->screen;
	struct r600_pipe_sampler_state *ss = CALLOC_STRUCT(r600_pipe_sampler_state);
	unsigned max_aniso = rscreen->force_aniso >= 0 ? rscreen->force_aniso
						       : state->max_anisotropy;
	unsigned max_aniso_ratio = r600_tex_aniso_filter(max_aniso);

	if (!ss) {
		return NULL;
	}

	ss->seamless_cube_map = state->seamless_cube_map;
	ss->border_color_use = sampler_state_needs_border_color(state);

	/* R_03C000_SQ_TEX_SAMPLER_WORD0_0 */
	ss->tex_sampler_words[0] =
		S_03C000_CLAMP_X(r600_tex_wrap(state->wrap_s)) |
		S_03C000_CLAMP_Y(r600_tex_wrap(state->wrap_t)) |
		S_03C000_CLAMP_Z(r600_tex_wrap(state->wrap_r)) |
		S_03C000_XY_MAG_FILTER(r600_tex_filter(state->mag_img_filter, max_aniso)) |
		S_03C000_XY_MIN_FILTER(r600_tex_filter(state->min_img_filter, max_aniso)) |
		S_03C000_MIP_FILTER(r600_tex_mipfilter(state->min_mip_filter)) |
		S_03C000_MAX_ANISO_RATIO(max_aniso_ratio) |
		S_03C000_DEPTH_COMPARE_FUNCTION(r600_tex_compare(state->compare_func)) |
		S_03C000_BORDER_COLOR_TYPE(ss->border_color_use ? V_03C000_SQ_TEX_BORDER_COLOR_REGISTER : 0);
	/* R_03C004_SQ_TEX_SAMPLER_WORD1_0 */
	ss->tex_sampler_words[1] =
		S_03C004_MIN_LOD(S_FIXED(CLAMP(state->min_lod, 0, 15), 6)) |
		S_03C004_MAX_LOD(S_FIXED(CLAMP(state->max_lod, 0, 15), 6)) |
		S_03C004_LOD_BIAS(S_FIXED(CLAMP(state->lod_bias, -16, 16), 6));
	/* R_03C008_SQ_TEX_SAMPLER_WORD2_0 */
	ss->tex_sampler_words[2] = S_03C008_TYPE(1);

	if (ss->border_color_use) {
		memcpy(&ss->border_color, &state->border_color, sizeof(state->border_color));
	}
	return ss;
}

static struct pipe_sampler_view *
texture_buffer_sampler_view(struct r600_pipe_sampler_view *view,
			    unsigned width0, unsigned height0)

{
	struct r600_texture *tmp = (struct r600_texture*)view->base.texture;
	int stride = util_format_get_blocksize(view->base.format);
	unsigned format, num_format, format_comp, endian;
	uint64_t offset = view->base.u.buf.offset;
	unsigned size = view->base.u.buf.size;

	r600_vertex_data_type(view->base.format,
			      &format, &num_format, &format_comp,
			      &endian);

	view->tex_resource = &tmp->resource;
	view->skip_mip_address_reloc = true;

	view->tex_resource_words[0] = offset;
	view->tex_resource_words[1] = size - 1;
	view->tex_resource_words[2] = S_038008_BASE_ADDRESS_HI(offset >> 32UL) |
		S_038008_STRIDE(stride) |
		S_038008_DATA_FORMAT(format) |
		S_038008_NUM_FORMAT_ALL(num_format) |
		S_038008_FORMAT_COMP_ALL(format_comp) |
		S_038008_ENDIAN_SWAP(endian);
	view->tex_resource_words[3] = 0;
	/*
	 * in theory dword 4 is for number of elements, for use with resinfo,
	 * but it seems to utterly fail to work, the amd gpu shader analyser
	 * uses a const buffer to store the element sizes for buffer txq
	 */
	view->tex_resource_words[4] = 0;
	view->tex_resource_words[5] = 0;
	view->tex_resource_words[6] = S_038018_TYPE(V_038010_SQ_TEX_VTX_VALID_BUFFER);
	return &view->base;
}

struct pipe_sampler_view *
r600_create_sampler_view_custom(struct pipe_context *ctx,
				struct pipe_resource *texture,
				const struct pipe_sampler_view *state,
				unsigned width_first_level, unsigned height_first_level)
{
	struct r600_pipe_sampler_view *view = CALLOC_STRUCT(r600_pipe_sampler_view);
	struct r600_texture *tmp = (struct r600_texture*)texture;
	unsigned format, endian;
	uint32_t word4 = 0, yuv_format = 0, pitch = 0;
	unsigned char swizzle[4], array_mode = 0;
	unsigned width, height, depth, offset_level, last_level;
	bool do_endian_swap = false;

	if (!view)
		return NULL;

	/* initialize base object */
	view->base = *state;
	view->base.texture = NULL;
	pipe_reference(NULL, &texture->reference);
	view->base.texture = texture;
	view->base.reference.count = 1;
	view->base.context = ctx;

	if (texture->target == PIPE_BUFFER)
		return texture_buffer_sampler_view(view, texture->width0, 1);

	swizzle[0] = state->swizzle_r;
	swizzle[1] = state->swizzle_g;
	swizzle[2] = state->swizzle_b;
	swizzle[3] = state->swizzle_a;

	if (UTIL_ARCH_BIG_ENDIAN)
		do_endian_swap = !tmp->db_compatible;

	format = r600_translate_texformat(ctx->screen, state->format,
					  swizzle,
					  &word4, &yuv_format, do_endian_swap);
	assert(format != ~0);
	if (format == ~0) {
		FREE(view);
		return NULL;
	}

	if (state->format == PIPE_FORMAT_X24S8_UINT ||
	    state->format == PIPE_FORMAT_S8X24_UINT ||
	    state->format == PIPE_FORMAT_X32_S8X24_UINT ||
	    state->format == PIPE_FORMAT_S8_UINT)
		view->is_stencil_sampler = true;

	if (tmp->is_depth && !r600_can_sample_zs(tmp, view->is_stencil_sampler)) {
		if (!r600_init_flushed_depth_texture(ctx, texture, NULL)) {
			FREE(view);
			return NULL;
		}
		tmp = tmp->flushed_depth_texture;
	}

	endian = r600_colorformat_endian_swap(format, do_endian_swap);

	offset_level = state->u.tex.first_level;
	last_level = state->u.tex.last_level - offset_level;
	width = width_first_level;
	height = height_first_level;
        depth = u_minify(texture->depth0, offset_level);
	pitch = tmp->surface.u.legacy.level[offset_level].nblk_x * util_format_get_blockwidth(state->format);

	if (texture->target == PIPE_TEXTURE_1D_ARRAY) {
		height = 1;
		depth = texture->array_size;
	} else if (texture->target == PIPE_TEXTURE_2D_ARRAY) {
		depth = texture->array_size;
	} else if (texture->target == PIPE_TEXTURE_CUBE_ARRAY)
		depth = texture->array_size / 6;

	switch (tmp->surface.u.legacy.level[offset_level].mode) {
	default:
	case RADEON_SURF_MODE_LINEAR_ALIGNED:
		array_mode = V_038000_ARRAY_LINEAR_ALIGNED;
		break;
	case RADEON_SURF_MODE_1D:
		array_mode = V_038000_ARRAY_1D_TILED_THIN1;
		break;
	case RADEON_SURF_MODE_2D:
		array_mode = V_038000_ARRAY_2D_TILED_THIN1;
		break;
	}

	view->tex_resource = &tmp->resource;
	view->tex_resource_words[0] = (S_038000_DIM(r600_tex_dim(texture->target, texture->nr_samples)) |
				       S_038000_TILE_MODE(array_mode) |
				       S_038000_TILE_TYPE(tmp->non_disp_tiling) |
				       S_038000_PITCH((pitch / 8) - 1) |
				       S_038000_TEX_WIDTH(width - 1));
	view->tex_resource_words[1] = (S_038004_TEX_HEIGHT(height - 1) |
				       S_038004_TEX_DEPTH(depth - 1) |
				       S_038004_DATA_FORMAT(format));
	view->tex_resource_words[2] = tmp->surface.u.legacy.level[offset_level].offset_256B;
	if (offset_level >= tmp->resource.b.b.last_level) {
		view->tex_resource_words[3] = tmp->surface.u.legacy.level[offset_level].offset_256B;
	} else {
		view->tex_resource_words[3] = tmp->surface.u.legacy.level[offset_level + 1].offset_256B;
	}
	view->tex_resource_words[4] = (word4 |
				       S_038010_REQUEST_SIZE(1) |
				       S_038010_ENDIAN_SWAP(endian) |
				       S_038010_BASE_LEVEL(0));
	view->tex_resource_words[5] = (S_038014_BASE_ARRAY(state->u.tex.first_layer) |
				       S_038014_LAST_ARRAY(state->u.tex.last_layer));
	if (texture->nr_samples > 1) {
		/* LAST_LEVEL holds log2(nr_samples) for multisample textures */
		view->tex_resource_words[5] |= S_038014_LAST_LEVEL(util_logbase2(texture->nr_samples));
	} else {
		view->tex_resource_words[5] |= S_038014_LAST_LEVEL(last_level);
	}
	view->tex_resource_words[6] = (S_038018_TYPE(V_038010_SQ_TEX_VTX_VALID_TEXTURE) |
				       S_038018_MAX_ANISO(4 /* max 16 samples */));
	return &view->base;
}

static struct pipe_sampler_view *
r600_create_sampler_view(struct pipe_context *ctx,
			 struct pipe_resource *tex,
			 const struct pipe_sampler_view *state)
{
	return r600_create_sampler_view_custom(ctx, tex, state,
                                               u_minify(tex->width0, state->u.tex.first_level),
                                               u_minify(tex->height0, state->u.tex.first_level));
}

static void r600_emit_clip_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct pipe_clip_state *state = &rctx->clip_state.state;

	radeon_set_context_reg_seq(cs, R_028E20_PA_CL_UCP0_X, 6*4);
	radeon_emit_array(cs, (unsigned*)state, 6*4);
}

static void r600_set_polygon_stipple(struct pipe_context *ctx,
					 const struct pipe_poly_stipple *state)
{
}

static void r600_init_color_surface(struct r600_context *rctx,
				    struct r600_surface *surf,
				    bool force_cmask_fmask)
{
	struct r600_screen *rscreen = rctx->screen;
	struct r600_texture *rtex = (struct r600_texture*)surf->base.texture;
	unsigned level = surf->base.u.tex.level;
	unsigned pitch, slice;
	unsigned color_info;
	unsigned color_view;
	unsigned format, swap, ntype, endian;
	unsigned offset;
	const struct util_format_description *desc;
	int i;
	bool blend_bypass = 0, blend_clamp = 0, do_endian_swap = false;

	if (rtex->db_compatible && !r600_can_sample_zs(rtex, false)) {
		r600_init_flushed_depth_texture(&rctx->b.b, surf->base.texture, NULL);
		rtex = rtex->flushed_depth_texture;
		assert(rtex);
	}

	offset = (uint64_t)rtex->surface.u.legacy.level[level].offset_256B * 256;
	color_view = S_028080_SLICE_START(surf->base.u.tex.first_layer) |
		     S_028080_SLICE_MAX(surf->base.u.tex.last_layer);

	pitch = rtex->surface.u.legacy.level[level].nblk_x / 8 - 1;
	slice = (rtex->surface.u.legacy.level[level].nblk_x * rtex->surface.u.legacy.level[level].nblk_y) / 64;
	if (slice) {
		slice = slice - 1;
	}
	color_info = 0;
	switch (rtex->surface.u.legacy.level[level].mode) {
	default:
	case RADEON_SURF_MODE_LINEAR_ALIGNED:
		color_info = S_0280A0_ARRAY_MODE(V_038000_ARRAY_LINEAR_ALIGNED);
		break;
	case RADEON_SURF_MODE_1D:
		color_info = S_0280A0_ARRAY_MODE(V_038000_ARRAY_1D_TILED_THIN1);
		break;
	case RADEON_SURF_MODE_2D:
		color_info = S_0280A0_ARRAY_MODE(V_038000_ARRAY_2D_TILED_THIN1);
		break;
	}

	desc = util_format_description(surf->base.format);

	i = util_format_get_first_non_void_channel(surf->base.format);

	ntype = V_0280A0_NUMBER_UNORM;
	if (desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB)
		ntype = V_0280A0_NUMBER_SRGB;
	else if (desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED) {
		if (desc->channel[i].normalized)
			ntype = V_0280A0_NUMBER_SNORM;
		else if (desc->channel[i].pure_integer)
			ntype = V_0280A0_NUMBER_SINT;
	} else if (desc->channel[i].type == UTIL_FORMAT_TYPE_UNSIGNED) {
		if (desc->channel[i].normalized)
			ntype = V_0280A0_NUMBER_UNORM;
		else if (desc->channel[i].pure_integer)
			ntype = V_0280A0_NUMBER_UINT;
	} else if (desc->channel[i].type == UTIL_FORMAT_TYPE_FLOAT) {
		ntype = V_0280A0_NUMBER_FLOAT;
	}

	if (UTIL_ARCH_BIG_ENDIAN)
		do_endian_swap = !rtex->db_compatible;

	format = r600_translate_colorformat(rctx->b.gfx_level, surf->base.format,
			                              do_endian_swap);
	assert(format != ~0);

	swap = r600_translate_colorswap(surf->base.format, do_endian_swap);
	assert(swap != ~0);

	endian = r600_colorformat_endian_swap(format, do_endian_swap);

	/* blend clamp should be set for all NORM/SRGB types */
	if (ntype == V_0280A0_NUMBER_UNORM || ntype == V_0280A0_NUMBER_SNORM ||
	    ntype == V_0280A0_NUMBER_SRGB)
		blend_clamp = 1;

	/* set blend bypass according to docs if SINT/UINT or
	   8/24 COLOR variants */
	if (ntype == V_0280A0_NUMBER_UINT || ntype == V_0280A0_NUMBER_SINT ||
	    format == V_0280A0_COLOR_8_24 || format == V_0280A0_COLOR_24_8 ||
	    format == V_0280A0_COLOR_X24_8_32_FLOAT) {
		blend_clamp = 0;
		blend_bypass = 1;
	}

	surf->alphatest_bypass = ntype == V_0280A0_NUMBER_UINT || ntype == V_0280A0_NUMBER_SINT;

	color_info |= S_0280A0_FORMAT(format) |
		S_0280A0_COMP_SWAP(swap) |
		S_0280A0_BLEND_BYPASS(blend_bypass) |
		S_0280A0_BLEND_CLAMP(blend_clamp) |
		S_0280A0_SIMPLE_FLOAT(1) |
		S_0280A0_NUMBER_TYPE(ntype) |
		S_0280A0_ENDIAN(endian);

	/* EXPORT_NORM is an optimization that can be enabled for better
	 * performance in certain cases
	 */
	if (rctx->b.gfx_level == R600) {
		/* EXPORT_NORM can be enabled if:
		 * - 11-bit or smaller UNORM/SNORM/SRGB
		 * - BLEND_CLAMP is enabled
		 * - BLEND_FLOAT32 is disabled
		 */
		if (desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS &&
		    (desc->channel[i].size < 12 &&
		     desc->channel[i].type != UTIL_FORMAT_TYPE_FLOAT &&
		     ntype != V_0280A0_NUMBER_UINT &&
		     ntype != V_0280A0_NUMBER_SINT) &&
		    G_0280A0_BLEND_CLAMP(color_info) &&
		    /* XXX this condition is always true since BLEND_FLOAT32 is never set (bug?). */
		    !G_0280A0_BLEND_FLOAT32(color_info)) {
			color_info |= S_0280A0_SOURCE_FORMAT(V_0280A0_EXPORT_NORM);
			surf->export_16bpc = true;
		}
	} else {
		/* EXPORT_NORM can be enabled if:
		 * - 11-bit or smaller UNORM/SNORM/SRGB
		 * - 16-bit or smaller FLOAT
		 */
		if (desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS &&
		    ((desc->channel[i].size < 12 &&
		      desc->channel[i].type != UTIL_FORMAT_TYPE_FLOAT &&
		      ntype != V_0280A0_NUMBER_UINT && ntype != V_0280A0_NUMBER_SINT) ||
		    (desc->channel[i].size < 17 &&
		     desc->channel[i].type == UTIL_FORMAT_TYPE_FLOAT))) {
			color_info |= S_0280A0_SOURCE_FORMAT(V_0280A0_EXPORT_NORM);
			surf->export_16bpc = true;
		}
	}

	/* These might not always be initialized to zero. */
	surf->cb_color_base = offset >> 8;
	surf->cb_color_size = S_028060_PITCH_TILE_MAX(pitch) |
			      S_028060_SLICE_TILE_MAX(slice);
	surf->cb_color_fmask = surf->cb_color_base;
	surf->cb_color_cmask = surf->cb_color_base;
	surf->cb_color_mask = 0;

	r600_resource_reference(&surf->cb_buffer_cmask, &rtex->resource);
	r600_resource_reference(&surf->cb_buffer_fmask, &rtex->resource);

	if (rtex->cmask.size) {
		surf->cb_color_cmask = rtex->cmask.offset >> 8;
		surf->cb_color_mask |= S_028100_CMASK_BLOCK_MAX(rtex->cmask.slice_tile_max);

		if (rtex->fmask.size) {
			color_info |= S_0280A0_TILE_MODE(V_0280A0_FRAG_ENABLE);
			surf->cb_color_fmask = rtex->fmask.offset >> 8;
			surf->cb_color_mask |= S_028100_FMASK_TILE_MAX(rtex->fmask.slice_tile_max);
		} else { /* cmask only */
			color_info |= S_0280A0_TILE_MODE(V_0280A0_CLEAR_ENABLE);
		}
	} else if (force_cmask_fmask) {
		/* Allocate dummy FMASK and CMASK if they aren't allocated already.
		 *
		 * R6xx needs FMASK and CMASK for the destination buffer of color resolve,
		 * otherwise it hangs. We don't have FMASK and CMASK pre-allocated,
		 * because it's not an MSAA buffer.
		 */
		struct r600_cmask_info cmask;
		struct r600_fmask_info fmask;

		r600_texture_get_cmask_info(&rscreen->b, rtex, &cmask);
		r600_texture_get_fmask_info(&rscreen->b, rtex, 8, &fmask);

		/* CMASK. */
		if (!rctx->dummy_cmask ||
		    rctx->dummy_cmask->b.b.width0 < cmask.size ||
		    (1 << rctx->dummy_cmask->buf->alignment_log2) % cmask.alignment != 0) {
			struct pipe_transfer *transfer;
			void *ptr;

			r600_resource_reference(&rctx->dummy_cmask, NULL);
			rctx->dummy_cmask = (struct r600_resource*)
				r600_aligned_buffer_create(&rscreen->b.b, 0,
							   PIPE_USAGE_DEFAULT,
							   cmask.size, cmask.alignment);

			if (unlikely(!rctx->dummy_cmask)) {
				surf->color_initialized = false;
				return;
			}

			/* Set the contents to 0xCC. */
			ptr = pipe_buffer_map(&rctx->b.b, &rctx->dummy_cmask->b.b, PIPE_MAP_WRITE, &transfer);
			memset(ptr, 0xCC, cmask.size);
			pipe_buffer_unmap(&rctx->b.b, transfer);
		}
		r600_resource_reference(&surf->cb_buffer_cmask, rctx->dummy_cmask);

		/* FMASK. */
		if (!rctx->dummy_fmask ||
		    rctx->dummy_fmask->b.b.width0 < fmask.size ||
		    (1 << rctx->dummy_fmask->buf->alignment_log2) % fmask.alignment != 0) {
			r600_resource_reference(&rctx->dummy_fmask, NULL);
			rctx->dummy_fmask = (struct r600_resource*)
				r600_aligned_buffer_create(&rscreen->b.b, 0,
							   PIPE_USAGE_DEFAULT,
							   fmask.size, fmask.alignment);

			if (unlikely(!rctx->dummy_fmask)) {
				surf->color_initialized = false;
				return;
			}
		}
		r600_resource_reference(&surf->cb_buffer_fmask, rctx->dummy_fmask);

		/* Init the registers. */
		color_info |= S_0280A0_TILE_MODE(V_0280A0_FRAG_ENABLE);
		surf->cb_color_cmask = 0;
		surf->cb_color_fmask = 0;
		surf->cb_color_mask = S_028100_CMASK_BLOCK_MAX(cmask.slice_tile_max) |
				      S_028100_FMASK_TILE_MAX(fmask.slice_tile_max);
	}

	surf->cb_color_info = color_info;
	surf->cb_color_view = color_view;
	surf->color_initialized = true;
}

static void r600_init_depth_surface(struct r600_context *rctx,
				    struct r600_surface *surf)
{
	struct r600_texture *rtex = (struct r600_texture*)surf->base.texture;
	unsigned level, pitch, slice, format, offset, array_mode;

	level = surf->base.u.tex.level;
	offset = (uint64_t)rtex->surface.u.legacy.level[level].offset_256B * 256;
	pitch = rtex->surface.u.legacy.level[level].nblk_x / 8 - 1;
	slice = (rtex->surface.u.legacy.level[level].nblk_x * rtex->surface.u.legacy.level[level].nblk_y) / 64;
	if (slice) {
		slice = slice - 1;
	}
	switch (rtex->surface.u.legacy.level[level].mode) {
	case RADEON_SURF_MODE_2D:
		array_mode = V_0280A0_ARRAY_2D_TILED_THIN1;
		break;
	case RADEON_SURF_MODE_1D:
	case RADEON_SURF_MODE_LINEAR_ALIGNED:
	default:
		array_mode = V_0280A0_ARRAY_1D_TILED_THIN1;
		break;
	}

	format = r600_translate_dbformat(surf->base.format);
	assert(format != ~0);

	surf->db_depth_info = S_028010_ARRAY_MODE(array_mode) | S_028010_FORMAT(format);
	surf->db_depth_base = offset >> 8;
	surf->db_depth_view = S_028004_SLICE_START(surf->base.u.tex.first_layer) |
			      S_028004_SLICE_MAX(surf->base.u.tex.last_layer);
	surf->db_depth_size = S_028000_PITCH_TILE_MAX(pitch) | S_028000_SLICE_TILE_MAX(slice);
	surf->db_prefetch_limit = (rtex->surface.u.legacy.level[level].nblk_y / 8) - 1;

	if (r600_htile_enabled(rtex, level)) {
		surf->db_htile_data_base = rtex->htile_offset >> 8;
		surf->db_htile_surface = S_028D24_HTILE_WIDTH(1) |
					 S_028D24_HTILE_HEIGHT(1) |
					 S_028D24_FULL_CACHE(1);
		/* preload is not working properly on r6xx/r7xx */
		surf->db_depth_info |= S_028010_TILE_SURFACE_ENABLE(1);
	}

	surf->depth_initialized = true;
}

static void r600_set_framebuffer_state(struct pipe_context *ctx,
					const struct pipe_framebuffer_state *state)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_surface *surf;
	struct r600_texture *rtex;
	unsigned i;
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

	/* Set the new state. */
	util_copy_framebuffer_state(&rctx->framebuffer.state, state);

	rctx->framebuffer.export_16bpc = state->nr_cbufs != 0;
	rctx->framebuffer.cb0_is_integer = state->nr_cbufs && state->cbufs[0] &&
			       util_format_is_pure_integer(state->cbufs[0]->format);
	rctx->framebuffer.compressed_cb_mask = 0;
	rctx->framebuffer.is_msaa_resolve = state->nr_cbufs == 2 &&
					    state->cbufs[0] && state->cbufs[1] &&
					    state->cbufs[0]->texture->nr_samples > 1 &&
				            state->cbufs[1]->texture->nr_samples <= 1;
	rctx->framebuffer.nr_samples = util_framebuffer_get_num_samples(state);

	/* Colorbuffers. */
	for (i = 0; i < state->nr_cbufs; i++) {
		/* The resolve buffer must have CMASK and FMASK to prevent hardlocks on R6xx. */
		bool force_cmask_fmask = rctx->b.gfx_level == R600 &&
					 rctx->framebuffer.is_msaa_resolve &&
					 i == 1;

		surf = (struct r600_surface*)state->cbufs[i];
		if (!surf)
			continue;

		rtex = (struct r600_texture*)surf->base.texture;
		r600_context_add_resource_size(ctx, state->cbufs[i]->texture);

		target_mask |= (0xf << (i * 4));

		if (!surf->color_initialized || force_cmask_fmask) {
			r600_init_color_surface(rctx, surf, force_cmask_fmask);
			if (force_cmask_fmask) {
				/* re-initialize later without compression */
				surf->color_initialized = false;
			}
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

		surf = (struct r600_surface*)state->cbufs[0];
		if (surf) {
			alphatest_bypass = surf->alphatest_bypass;
		}

		if (rctx->alphatest_state.bypass != alphatest_bypass) {
			rctx->alphatest_state.bypass = alphatest_bypass;
			r600_mark_atom_dirty(rctx, &rctx->alphatest_state.atom);
		}
	}

	/* ZS buffer. */
	if (state->zsbuf) {
		surf = (struct r600_surface*)state->zsbuf;

		r600_context_add_resource_size(ctx, state->zsbuf->texture);

		if (!surf->depth_initialized) {
			r600_init_depth_surface(rctx, surf);
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

	/* Calculate the CS size. */
	rctx->framebuffer.atom.num_dw =
		10 /*COLOR_INFO*/ + 4 /*SCISSOR*/ + 3 /*SHADER_CONTROL*/ + 8 /*MSAA*/;

	if (rctx->framebuffer.state.nr_cbufs) {
		rctx->framebuffer.atom.num_dw += 15 * rctx->framebuffer.state.nr_cbufs;
		rctx->framebuffer.atom.num_dw += 3 * (2 + rctx->framebuffer.state.nr_cbufs);
	}
	if (rctx->framebuffer.state.zsbuf) {
		rctx->framebuffer.atom.num_dw += 16;
	} else {
		rctx->framebuffer.atom.num_dw += 3;
	}
	if (rctx->b.family > CHIP_R600 && rctx->b.family < CHIP_RV770) {
		rctx->framebuffer.atom.num_dw += 2;
	}

	r600_mark_atom_dirty(rctx, &rctx->framebuffer.atom);

	r600_set_sample_locations_constant_buffer(rctx);
	rctx->framebuffer.do_update_surf_dirtiness = true;
}

static const uint32_t sample_locs_2x[] = {
	FILL_SREG(-4, 4, 4, -4, -4, 4, 4, -4),
	FILL_SREG(-4, 4, 4, -4, -4, 4, 4, -4),
};
static const unsigned max_dist_2x = 4;

static const uint32_t sample_locs_4x[] = {
	FILL_SREG(-2, -2, 2, 2, -6, 6, 6, -6),
	FILL_SREG(-2, -2, 2, 2, -6, 6, 6, -6),
};
static const unsigned max_dist_4x = 6;
static const uint32_t sample_locs_8x[] = {
	FILL_SREG(-1,  1,  1,  5,  3, -5,  5,  3),
	FILL_SREG(-7, -1, -3, -7,  7, -3, -5,  7),
};
static const unsigned max_dist_8x = 7;

static void r600_get_sample_position(struct pipe_context *ctx,
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
		val.idx = (sample_locs_2x[0] >> offset) & 0xf;
		out_value[0] = (float)(val.idx + 8) / 16.0f;
		val.idx = (sample_locs_2x[0] >> (offset + 4)) & 0xf;
		out_value[1] = (float)(val.idx + 8) / 16.0f;
		break;
	case 4:
		offset = 4 * (sample_index * 2);
		val.idx = (sample_locs_4x[0] >> offset) & 0xf;
		out_value[0] = (float)(val.idx + 8) / 16.0f;
		val.idx = (sample_locs_4x[0] >> (offset + 4)) & 0xf;
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

static void r600_emit_msaa_state(struct r600_context *rctx, int nr_samples)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	unsigned max_dist = 0;

	if (rctx->b.family == CHIP_R600) {
		switch (nr_samples) {
		default:
			nr_samples = 0;
			break;
		case 2:
			radeon_set_config_reg(cs, R_008B40_PA_SC_AA_SAMPLE_LOCS_2S, sample_locs_2x[0]);
			max_dist = max_dist_2x;
			break;
		case 4:
			radeon_set_config_reg(cs, R_008B44_PA_SC_AA_SAMPLE_LOCS_4S, sample_locs_4x[0]);
			max_dist = max_dist_4x;
			break;
		case 8:
			radeon_set_config_reg_seq(cs, R_008B48_PA_SC_AA_SAMPLE_LOCS_8S_WD0, 2);
			radeon_emit(cs, sample_locs_8x[0]); /* R_008B48_PA_SC_AA_SAMPLE_LOCS_8S_WD0 */
			radeon_emit(cs, sample_locs_8x[1]); /* R_008B4C_PA_SC_AA_SAMPLE_LOCS_8S_WD1 */
			max_dist = max_dist_8x;
			break;
		}
	} else {
		switch (nr_samples) {
		default:
			radeon_set_context_reg_seq(cs, R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX, 2);
			radeon_emit(cs, 0); /* R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX */
			radeon_emit(cs, 0); /* R_028C20_PA_SC_AA_SAMPLE_LOCS_8D_WD1_MCTX */
			nr_samples = 0;
			break;
		case 2:
			radeon_set_context_reg_seq(cs, R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX, 2);
			radeon_emit(cs, sample_locs_2x[0]); /* R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX */
			radeon_emit(cs, sample_locs_2x[1]); /* R_028C20_PA_SC_AA_SAMPLE_LOCS_8D_WD1_MCTX */
			max_dist = max_dist_2x;
			break;
		case 4:
			radeon_set_context_reg_seq(cs, R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX, 2);
			radeon_emit(cs, sample_locs_4x[0]); /* R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX */
			radeon_emit(cs, sample_locs_4x[1]); /* R_028C20_PA_SC_AA_SAMPLE_LOCS_8D_WD1_MCTX */
			max_dist = max_dist_4x;
			break;
		case 8:
			radeon_set_context_reg_seq(cs, R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX, 2);
			radeon_emit(cs, sample_locs_8x[0]); /* R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX */
			radeon_emit(cs, sample_locs_8x[1]); /* R_028C20_PA_SC_AA_SAMPLE_LOCS_8D_WD1_MCTX */
			max_dist = max_dist_8x;
			break;
		}
	}

	if (nr_samples > 1) {
		radeon_set_context_reg_seq(cs, R_028C00_PA_SC_LINE_CNTL, 2);
		radeon_emit(cs, S_028C00_LAST_PIXEL(1) |
				     S_028C00_EXPAND_LINE_WIDTH(1)); /* R_028C00_PA_SC_LINE_CNTL */
		radeon_emit(cs, S_028C04_MSAA_NUM_SAMPLES(util_logbase2(nr_samples)) |
				     S_028C04_MAX_SAMPLE_DIST(max_dist)); /* R_028C04_PA_SC_AA_CONFIG */
	} else {
		radeon_set_context_reg_seq(cs, R_028C00_PA_SC_LINE_CNTL, 2);
		radeon_emit(cs, S_028C00_LAST_PIXEL(1)); /* R_028C00_PA_SC_LINE_CNTL */
		radeon_emit(cs, 0); /* R_028C04_PA_SC_AA_CONFIG */
	}
}

static void r600_emit_framebuffer_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct pipe_framebuffer_state *state = &rctx->framebuffer.state;
	unsigned nr_cbufs = state->nr_cbufs;
	struct r600_surface **cb = (struct r600_surface**)&state->cbufs[0];
	unsigned i, sbu = 0;

	/* Colorbuffers. */
	radeon_set_context_reg_seq(cs, R_0280A0_CB_COLOR0_INFO, 8);
	for (i = 0; i < nr_cbufs; i++) {
		radeon_emit(cs, cb[i] ? cb[i]->cb_color_info : 0);
	}
	/* set CB_COLOR1_INFO for possible dual-src blending */
	if (rctx->framebuffer.dual_src_blend && i == 1 && cb[0]) {
		radeon_emit(cs, cb[0]->cb_color_info);
		i++;
	}
	for (; i < 8; i++) {
		radeon_emit(cs, 0);
	}

	if (nr_cbufs) {
		for (i = 0; i < nr_cbufs; i++) {
			unsigned reloc;

			if (!cb[i])
				continue;

			/* COLOR_BASE */
			radeon_set_context_reg(cs, R_028040_CB_COLOR0_BASE + i*4, cb[i]->cb_color_base);

			reloc = radeon_add_to_buffer_list(&rctx->b,
						      &rctx->b.gfx,
						      (struct r600_resource*)cb[i]->base.texture,
						      RADEON_USAGE_READWRITE |
						      (cb[i]->base.texture->nr_samples > 1 ?
							      RADEON_PRIO_COLOR_BUFFER_MSAA :
							      RADEON_PRIO_COLOR_BUFFER));
			radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
			radeon_emit(cs, reloc);

			/* FMASK */
			radeon_set_context_reg(cs, R_0280E0_CB_COLOR0_FRAG + i*4, cb[i]->cb_color_fmask);

			reloc = radeon_add_to_buffer_list(&rctx->b,
						      &rctx->b.gfx,
						      cb[i]->cb_buffer_fmask,
						      RADEON_USAGE_READWRITE |
						      (cb[i]->base.texture->nr_samples > 1 ?
							      RADEON_PRIO_COLOR_BUFFER_MSAA :
							      RADEON_PRIO_COLOR_BUFFER));
			radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
			radeon_emit(cs, reloc);

			/* CMASK */
			radeon_set_context_reg(cs, R_0280C0_CB_COLOR0_TILE + i*4, cb[i]->cb_color_cmask);

			reloc = radeon_add_to_buffer_list(&rctx->b,
						      &rctx->b.gfx,
						      cb[i]->cb_buffer_cmask,
						      RADEON_USAGE_READWRITE |
						      (cb[i]->base.texture->nr_samples > 1 ?
							      RADEON_PRIO_COLOR_BUFFER_MSAA :
							      RADEON_PRIO_COLOR_BUFFER));
			radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
			radeon_emit(cs, reloc);
		}

		radeon_set_context_reg_seq(cs, R_028060_CB_COLOR0_SIZE, nr_cbufs);
		for (i = 0; i < nr_cbufs; i++) {
			radeon_emit(cs, cb[i] ? cb[i]->cb_color_size : 0);
		}

		radeon_set_context_reg_seq(cs, R_028080_CB_COLOR0_VIEW, nr_cbufs);
		for (i = 0; i < nr_cbufs; i++) {
			radeon_emit(cs, cb[i] ? cb[i]->cb_color_view : 0);
		}

		radeon_set_context_reg_seq(cs, R_028100_CB_COLOR0_MASK, nr_cbufs);
		for (i = 0; i < nr_cbufs; i++) {
			radeon_emit(cs, cb[i] ? cb[i]->cb_color_mask : 0);
		}

		sbu |= SURFACE_BASE_UPDATE_COLOR_NUM(nr_cbufs);
	}

	/* SURFACE_BASE_UPDATE */
	if (rctx->b.family > CHIP_R600 && rctx->b.family < CHIP_RV770 && sbu) {
		radeon_emit(cs, PKT3(PKT3_SURFACE_BASE_UPDATE, 0, 0));
		radeon_emit(cs, sbu);
		sbu = 0;
	}

	/* Zbuffer. */
	if (state->zsbuf) {
		struct r600_surface *surf = (struct r600_surface*)state->zsbuf;
		unsigned reloc = radeon_add_to_buffer_list(&rctx->b,
						       &rctx->b.gfx,
						       (struct r600_resource*)state->zsbuf->texture,
						       RADEON_USAGE_READWRITE |
						       (surf->base.texture->nr_samples > 1 ?
							       RADEON_PRIO_DEPTH_BUFFER_MSAA :
							       RADEON_PRIO_DEPTH_BUFFER));

		radeon_set_context_reg_seq(cs, R_028000_DB_DEPTH_SIZE, 2);
		radeon_emit(cs, surf->db_depth_size); /* R_028000_DB_DEPTH_SIZE */
		radeon_emit(cs, surf->db_depth_view); /* R_028004_DB_DEPTH_VIEW */
		radeon_set_context_reg_seq(cs, R_02800C_DB_DEPTH_BASE, 2);
		radeon_emit(cs, surf->db_depth_base); /* R_02800C_DB_DEPTH_BASE */
		radeon_emit(cs, surf->db_depth_info); /* R_028010_DB_DEPTH_INFO */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc);

		radeon_set_context_reg(cs, R_028D34_DB_PREFETCH_LIMIT, surf->db_prefetch_limit);

		sbu |= SURFACE_BASE_UPDATE_DEPTH;
	} else {
		radeon_set_context_reg(cs, R_028010_DB_DEPTH_INFO, S_028010_FORMAT(V_028010_DEPTH_INVALID));
	}

	/* SURFACE_BASE_UPDATE */
	if (rctx->b.family > CHIP_R600 && rctx->b.family < CHIP_RV770 && sbu) {
		radeon_emit(cs, PKT3(PKT3_SURFACE_BASE_UPDATE, 0, 0));
		radeon_emit(cs, sbu);
		sbu = 0;
	}

	/* Framebuffer dimensions. */
	radeon_set_context_reg_seq(cs, R_028204_PA_SC_WINDOW_SCISSOR_TL, 2);
	radeon_emit(cs, S_028240_TL_X(0) | S_028240_TL_Y(0) |
			     S_028240_WINDOW_OFFSET_DISABLE(1)); /* R_028204_PA_SC_WINDOW_SCISSOR_TL */
	radeon_emit(cs, S_028244_BR_X(state->width) |
			     S_028244_BR_Y(state->height)); /* R_028208_PA_SC_WINDOW_SCISSOR_BR */

	if (rctx->framebuffer.is_msaa_resolve) {
		radeon_set_context_reg(cs, R_0287A0_CB_SHADER_CONTROL, 1);
	} else {
		/* Always enable the first colorbuffer in CB_SHADER_CONTROL. This
		 * will assure that the alpha-test will work even if there is
		 * no colorbuffer bound. */
		radeon_set_context_reg(cs, R_0287A0_CB_SHADER_CONTROL,
				       (1ull << MAX2(nr_cbufs, 1)) - 1);
	}

	r600_emit_msaa_state(rctx, rctx->framebuffer.nr_samples);
}

static void r600_set_min_samples(struct pipe_context *ctx, unsigned min_samples)
{
	struct r600_context *rctx = (struct r600_context *)ctx;

	if (rctx->ps_iter_samples == min_samples)
		return;

	rctx->ps_iter_samples = min_samples;
	if (rctx->framebuffer.nr_samples > 1) {
		r600_mark_atom_dirty(rctx, &rctx->rasterizer_state.atom);
		if (rctx->b.gfx_level == R600)
			r600_mark_atom_dirty(rctx, &rctx->db_misc_state.atom);
	}
}

static void r600_emit_cb_misc_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_cb_misc_state *a = (struct r600_cb_misc_state*)atom;

	if (G_028808_SPECIAL_OP(a->cb_color_control) == V_028808_SPECIAL_RESOLVE_BOX) {
		radeon_set_context_reg_seq(cs, R_028238_CB_TARGET_MASK, 2);
		if (rctx->b.gfx_level == R600) {
			radeon_emit(cs, 0xff); /* R_028238_CB_TARGET_MASK */
			radeon_emit(cs, 0xff); /* R_02823C_CB_SHADER_MASK */
		} else {
			radeon_emit(cs, 0xf); /* R_028238_CB_TARGET_MASK */
			radeon_emit(cs, 0xf); /* R_02823C_CB_SHADER_MASK */
		}
		radeon_set_context_reg(cs, R_028808_CB_COLOR_CONTROL, a->cb_color_control);
	} else {
		unsigned fb_colormask = a->bound_cbufs_target_mask;
		unsigned ps_colormask = a->ps_color_export_mask;
		unsigned multiwrite = a->multiwrite && a->nr_cbufs > 1;

		radeon_set_context_reg_seq(cs, R_028238_CB_TARGET_MASK, 2);
		radeon_emit(cs, a->blend_colormask & fb_colormask); /* R_028238_CB_TARGET_MASK */
		/* Always enable the first color output to make sure alpha-test works even without one. */
		radeon_emit(cs, 0xf | (multiwrite ? fb_colormask : ps_colormask)); /* R_02823C_CB_SHADER_MASK */
		radeon_set_context_reg(cs, R_028808_CB_COLOR_CONTROL,
				       a->cb_color_control |
				       S_028808_MULTIWRITE_ENABLE(multiwrite));
	}
}

static void r600_emit_db_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_db_state *a = (struct r600_db_state*)atom;

	if (a->rsurf && a->rsurf->db_htile_surface) {
		struct r600_texture *rtex = (struct r600_texture *)a->rsurf->base.texture;
		unsigned reloc_idx;

		radeon_set_context_reg(cs, R_02802C_DB_DEPTH_CLEAR, fui(rtex->depth_clear_value));
		radeon_set_context_reg(cs, R_028D24_DB_HTILE_SURFACE, a->rsurf->db_htile_surface);
		radeon_set_context_reg(cs, R_028014_DB_HTILE_DATA_BASE, a->rsurf->db_htile_data_base);
		reloc_idx = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, &rtex->resource,
						  RADEON_USAGE_READWRITE | RADEON_PRIO_SEPARATE_META);
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc_idx);
	} else {
		radeon_set_context_reg(cs, R_028D24_DB_HTILE_SURFACE, 0);
	}
}

static void r600_emit_db_misc_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_db_misc_state *a = (struct r600_db_misc_state*)atom;
	unsigned db_render_control = 0;
	unsigned db_render_override =
		S_028D10_FORCE_HIS_ENABLE0(V_028D10_FORCE_DISABLE) |
		S_028D10_FORCE_HIS_ENABLE1(V_028D10_FORCE_DISABLE);

	if (rctx->b.gfx_level >= R700) {
		switch (a->ps_conservative_z) {
		default: /* fall through */
		case FRAG_DEPTH_LAYOUT_ANY:
			db_render_control |= S_028D0C_CONSERVATIVE_Z_EXPORT(V_028D0C_EXPORT_ANY_Z);
			break;
		case FRAG_DEPTH_LAYOUT_GREATER:
			db_render_control |= S_028D0C_CONSERVATIVE_Z_EXPORT(V_028D0C_EXPORT_GREATER_THAN_Z);
			break;
		case FRAG_DEPTH_LAYOUT_LESS:
			db_render_control |= S_028D0C_CONSERVATIVE_Z_EXPORT(V_028D0C_EXPORT_LESS_THAN_Z);
			break;
		}
	}

	if (rctx->b.num_occlusion_queries > 0 &&
	    !a->occlusion_queries_disabled) {
		if (rctx->b.gfx_level >= R700) {
			db_render_control |= S_028D0C_R700_PERFECT_ZPASS_COUNTS(1);
		}
		db_render_override |= S_028D10_NOOP_CULL_DISABLE(1);
	} else {
		db_render_control |= S_028D0C_ZPASS_INCREMENT_DISABLE(1);
	}

	if (rctx->db_state.rsurf && rctx->db_state.rsurf->db_htile_surface) {
		/* FORCE_OFF means HiZ/HiS are determined by DB_SHADER_CONTROL */
		db_render_override |= S_028D10_FORCE_HIZ_ENABLE(V_028D10_FORCE_OFF);
		/* This is to fix a lockup when hyperz and alpha test are enabled at
		 * the same time somehow GPU get confuse on which order to pick for
		 * z test
		 */
		if (rctx->alphatest_state.sx_alpha_test_control) {
			db_render_override |= S_028D10_FORCE_SHADER_Z_ORDER(1);
		}
	} else {
		db_render_override |= S_028D10_FORCE_HIZ_ENABLE(V_028D10_FORCE_DISABLE);
	}
	if (rctx->b.gfx_level == R600 && rctx->framebuffer.nr_samples > 1 && rctx->ps_iter_samples > 0) {
		/* sample shading and hyperz causes lockups on R6xx chips */
		db_render_override |= S_028D10_FORCE_HIZ_ENABLE(V_028D10_FORCE_DISABLE);
	}
	if (a->flush_depthstencil_through_cb) {
		assert(a->copy_depth || a->copy_stencil);

		db_render_control |= S_028D0C_DEPTH_COPY_ENABLE(a->copy_depth) |
				     S_028D0C_STENCIL_COPY_ENABLE(a->copy_stencil) |
				     S_028D0C_COPY_CENTROID(1) |
				     S_028D0C_COPY_SAMPLE(a->copy_sample);

		if (rctx->b.gfx_level == R600)
			db_render_override |= S_028D10_NOOP_CULL_DISABLE(1);

		if (rctx->b.family == CHIP_RV610 || rctx->b.family == CHIP_RV630 ||
		    rctx->b.family == CHIP_RV620 || rctx->b.family == CHIP_RV635)
			db_render_override |= S_028D10_FORCE_HIZ_ENABLE(V_028D10_FORCE_DISABLE);
	} else if (a->flush_depth_inplace || a->flush_stencil_inplace) {
		db_render_control |= S_028D0C_DEPTH_COMPRESS_DISABLE(a->flush_depth_inplace) |
				     S_028D0C_STENCIL_COMPRESS_DISABLE(a->flush_stencil_inplace);
		db_render_override |= S_028D10_NOOP_CULL_DISABLE(1);
	}
	if (a->htile_clear) {
		db_render_control |= S_028D0C_DEPTH_CLEAR_ENABLE(1);
	}

	/* RV770 workaround for a hang with 8x MSAA. */
	if (rctx->b.family == CHIP_RV770 && a->log_samples == 3) {
		db_render_override |= S_028D10_MAX_TILES_IN_DTT(6);
	}

	radeon_set_context_reg_seq(cs, R_028D0C_DB_RENDER_CONTROL, 2);
	radeon_emit(cs, db_render_control); /* R_028D0C_DB_RENDER_CONTROL */
	radeon_emit(cs, db_render_override); /* R_028D10_DB_RENDER_OVERRIDE */
	radeon_set_context_reg(cs, R_02880C_DB_SHADER_CONTROL, a->db_shader_control);
}

static void r600_emit_config_state(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_config_state *a = (struct r600_config_state*)atom;

	radeon_set_config_reg(cs, R_008C04_SQ_GPR_RESOURCE_MGMT_1, a->sq_gpr_resource_mgmt_1);
	radeon_set_config_reg(cs, R_008C08_SQ_GPR_RESOURCE_MGMT_2, a->sq_gpr_resource_mgmt_2);
}

static void r600_emit_vertex_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_fetch_shader *shader = (struct r600_fetch_shader*)rctx->vertex_fetch_shader.cso;
	uint32_t dirty_mask = rctx->vertex_buffer_state.dirty_mask & shader->buffer_mask;

	while (dirty_mask) {
		struct pipe_vertex_buffer *vb;
		struct r600_resource *rbuffer;
		unsigned offset;
		unsigned buffer_index = u_bit_scan(&dirty_mask);
		unsigned stride = shader->strides[buffer_index];

		vb = &rctx->vertex_buffer_state.vb[buffer_index];
		rbuffer = (struct r600_resource*)vb->buffer.resource;
		assert(rbuffer);

		offset = vb->buffer_offset;

		/* fetch resources start at index 320 (OFFSET_FS) */
		radeon_emit(cs, PKT3(PKT3_SET_RESOURCE, 7, 0));
		radeon_emit(cs, (R600_FETCH_CONSTANTS_OFFSET_FS + buffer_index) * 7);
		radeon_emit(cs, offset); /* RESOURCEi_WORD0 */
		radeon_emit(cs, rbuffer->b.b.width0 - offset - 1); /* RESOURCEi_WORD1 */
		radeon_emit(cs, /* RESOURCEi_WORD2 */
				 S_038008_ENDIAN_SWAP(r600_endian_swap(32)) |
				 S_038008_STRIDE(stride));
		radeon_emit(cs, 0); /* RESOURCEi_WORD3 */
		radeon_emit(cs, 0); /* RESOURCEi_WORD4 */
		radeon_emit(cs, 0); /* RESOURCEi_WORD5 */
		radeon_emit(cs, 0xc0000000); /* RESOURCEi_WORD6 */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
						      RADEON_USAGE_READ | RADEON_PRIO_VERTEX_BUFFER));
	}
}

static void r600_emit_constant_buffers(struct r600_context *rctx,
				       struct r600_constbuf_state *state,
				       unsigned buffer_id_base,
				       unsigned reg_alu_constbuf_size,
				       unsigned reg_alu_const_cache)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint32_t dirty_mask = state->dirty_mask;

	while (dirty_mask) {
		struct pipe_constant_buffer *cb;
		struct r600_resource *rbuffer;
		unsigned offset;
		unsigned buffer_index = ffs(dirty_mask) - 1;
		unsigned gs_ring_buffer = (buffer_index == R600_GS_RING_CONST_BUFFER);
		cb = &state->cb[buffer_index];
		rbuffer = (struct r600_resource*)cb->buffer;
		assert(rbuffer);

		offset = cb->buffer_offset;

		if (!gs_ring_buffer) {
			assert(buffer_index < R600_MAX_ALU_CONST_BUFFERS);
			radeon_set_context_reg(cs, reg_alu_constbuf_size + buffer_index * 4,
					       DIV_ROUND_UP(cb->buffer_size, 256));
			radeon_set_context_reg(cs, reg_alu_const_cache + buffer_index * 4, offset >> 8);
			radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
			radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
								  RADEON_USAGE_READ | RADEON_PRIO_CONST_BUFFER));
		}

		radeon_emit(cs, PKT3(PKT3_SET_RESOURCE, 7, 0));
		radeon_emit(cs, (buffer_id_base + buffer_index) * 7);
		radeon_emit(cs, offset); /* RESOURCEi_WORD0 */
		radeon_emit(cs, cb->buffer_size - 1); /* RESOURCEi_WORD1 */
		radeon_emit(cs, /* RESOURCEi_WORD2 */
			    S_038008_ENDIAN_SWAP(gs_ring_buffer ? ENDIAN_NONE : r600_endian_swap(32)) |
			    S_038008_STRIDE(gs_ring_buffer ? 4 : 16));
		radeon_emit(cs, 0); /* RESOURCEi_WORD3 */
		radeon_emit(cs, 0); /* RESOURCEi_WORD4 */
		radeon_emit(cs, 0); /* RESOURCEi_WORD5 */
		radeon_emit(cs, 0xc0000000); /* RESOURCEi_WORD6 */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
						      RADEON_USAGE_READ | RADEON_PRIO_CONST_BUFFER));

		dirty_mask &= ~(1 << buffer_index);
	}
	state->dirty_mask = 0;
}

static void r600_emit_vs_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_VERTEX],
				   R600_FETCH_CONSTANTS_OFFSET_VS,
				   R_028180_ALU_CONST_BUFFER_SIZE_VS_0,
				   R_028980_ALU_CONST_CACHE_VS_0);
}

static void r600_emit_gs_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_GEOMETRY],
				   R600_FETCH_CONSTANTS_OFFSET_GS,
				   R_0281C0_ALU_CONST_BUFFER_SIZE_GS_0,
				   R_0289C0_ALU_CONST_CACHE_GS_0);
}

static void r600_emit_ps_constant_buffers(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_constant_buffers(rctx, &rctx->constbuf_state[PIPE_SHADER_FRAGMENT],
				   R600_FETCH_CONSTANTS_OFFSET_PS,
				   R_028140_ALU_CONST_BUFFER_SIZE_PS_0,
				   R_028940_ALU_CONST_CACHE_PS_0);
}

static void r600_emit_sampler_views(struct r600_context *rctx,
				    struct r600_samplerview_state *state,
				    unsigned resource_id_base)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint32_t dirty_mask = state->dirty_mask;

	while (dirty_mask) {
		struct r600_pipe_sampler_view *rview;
		unsigned resource_index = u_bit_scan(&dirty_mask);
		unsigned reloc;

		rview = state->views[resource_index];
		assert(rview);

		radeon_emit(cs, PKT3(PKT3_SET_RESOURCE, 7, 0));
		radeon_emit(cs, (resource_id_base + resource_index) * 7);
		radeon_emit_array(cs, rview->tex_resource_words, 7);

		reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rview->tex_resource,
					      RADEON_USAGE_READ |
					      r600_get_sampler_view_priority(rview->tex_resource));
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc);
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc);
	}
	state->dirty_mask = 0;
}


static void r600_emit_vs_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_VERTEX].views, R600_FETCH_CONSTANTS_OFFSET_VS + R600_MAX_CONST_BUFFERS);
}

static void r600_emit_gs_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_GEOMETRY].views, R600_FETCH_CONSTANTS_OFFSET_GS + R600_MAX_CONST_BUFFERS);
}

static void r600_emit_ps_sampler_views(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_sampler_views(rctx, &rctx->samplers[PIPE_SHADER_FRAGMENT].views, R600_FETCH_CONSTANTS_OFFSET_PS + R600_MAX_CONST_BUFFERS);
}

static void r600_emit_sampler_states(struct r600_context *rctx,
				struct r600_textures_info *texinfo,
				unsigned resource_id_base,
				unsigned border_color_reg)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint32_t dirty_mask = texinfo->states.dirty_mask;

	while (dirty_mask) {
		struct r600_pipe_sampler_state *rstate;
		struct r600_pipe_sampler_view *rview;
		unsigned i = u_bit_scan(&dirty_mask);

		rstate = texinfo->states.states[i];
		assert(rstate);
		rview = texinfo->views.views[i];

		/* TEX_ARRAY_OVERRIDE must be set for array textures to disable
		 * filtering between layers.
		 */
		enum pipe_texture_target target = PIPE_BUFFER;
		if (rview)
			target = rview->base.texture->target;

                /* If seamless cube map is set, set the CAMP_(X|Y|Z) to
                 * SQ_TEX_WRAP which seems to trigger properly ignoring the
                 * texture wrap mode */
                if (target == PIPE_TEXTURE_CUBE ||
		    target == PIPE_TEXTURE_CUBE_ARRAY) {
                   if (rstate->seamless_cube_map){
                      uint32_t mask = ~(S_03C000_CLAMP_X(7) |
                                        S_03C000_CLAMP_Y(7) |
                                        S_03C000_CLAMP_Z(7));
                      rstate->tex_sampler_words[0] &= mask;
                   }
                }

		if (target == PIPE_TEXTURE_1D_ARRAY ||
		    target == PIPE_TEXTURE_2D_ARRAY) {
			rstate->tex_sampler_words[0] |= S_03C000_TEX_ARRAY_OVERRIDE(1);
			texinfo->is_array_sampler[i] = true;
		} else {
			rstate->tex_sampler_words[0] &= C_03C000_TEX_ARRAY_OVERRIDE;
			texinfo->is_array_sampler[i] = false;
		}

		radeon_emit(cs, PKT3(PKT3_SET_SAMPLER, 3, 0));
		radeon_emit(cs, (resource_id_base + i) * 3);
		radeon_emit_array(cs, rstate->tex_sampler_words, 3);

		if (rstate->border_color_use) {
			unsigned offset;

			offset = border_color_reg;
			offset += i * 16;
			radeon_set_config_reg_seq(cs, offset, 4);
			radeon_emit_array(cs, rstate->border_color.ui, 4);
		}
	}
	texinfo->states.dirty_mask = 0;
}

static void r600_emit_vs_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_VERTEX], 18, R_00A600_TD_VS_SAMPLER0_BORDER_RED);
}

static void r600_emit_gs_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_GEOMETRY], 36, R_00A800_TD_GS_SAMPLER0_BORDER_RED);
}

static void r600_emit_ps_sampler_states(struct r600_context *rctx, struct r600_atom *atom)
{
	r600_emit_sampler_states(rctx, &rctx->samplers[PIPE_SHADER_FRAGMENT], 0, R_00A400_TD_PS_SAMPLER0_BORDER_RED);
}

static void r600_emit_seamless_cube_map(struct r600_context *rctx, struct r600_atom *atom)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	unsigned tmp;

	tmp = S_009508_DISABLE_CUBE_ANISO(1) |
		S_009508_SYNC_GRADIENT(1) |
		S_009508_SYNC_WALKER(1) |
		S_009508_SYNC_ALIGNER(1);
	if (!rctx->seamless_cube_map.enabled) {
		tmp |= S_009508_DISABLE_CUBE_WRAP(1);
	}
	radeon_set_config_reg(cs, R_009508_TA_CNTL_AUX, tmp);
}

static void r600_emit_sample_mask(struct r600_context *rctx, struct r600_atom *a)
{
	struct r600_sample_mask *s = (struct r600_sample_mask*)a;
	uint8_t mask = s->sample_mask;

	radeon_set_context_reg(&rctx->b.gfx.cs, R_028C48_PA_SC_AA_MASK,
			       mask | (mask << 8) | (mask << 16) | (mask << 24));
}

static void r600_emit_vertex_fetch_shader(struct r600_context *rctx, struct r600_atom *a)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_cso_state *state = (struct r600_cso_state*)a;
	struct r600_fetch_shader *shader = (struct r600_fetch_shader*)state->cso;

	if (!shader)
		return;

	radeon_set_context_reg(cs, R_028894_SQ_PGM_START_FS, shader->offset >> 8);
	radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
	radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, shader->buffer,
                                                  RADEON_USAGE_READ |
                                                  RADEON_PRIO_SHADER_BINARY));
}

static void r600_emit_shader_stages(struct r600_context *rctx, struct r600_atom *a)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_shader_stages_state *state = (struct r600_shader_stages_state*)a;

	uint32_t v2 = 0, primid = 0;

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

		v2 = S_028A40_MODE(V_028A40_GS_SCENARIO_G) |
			S_028A40_CUT_MODE(cut_val);

		if (rctx->gs_shader->current->shader.gs_prim_id_input)
			primid = 1;
	}

	radeon_set_context_reg(cs, R_028A40_VGT_GS_MODE, v2);
	radeon_set_context_reg(cs, R_028A84_VGT_PRIMITIVEID_EN, primid);
}

static void r600_emit_gs_rings(struct r600_context *rctx, struct r600_atom *a)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_gs_rings_state *state = (struct r600_gs_rings_state*)a;
	struct r600_resource *rbuffer;

	radeon_set_config_reg(cs, R_008040_WAIT_UNTIL, S_008040_WAIT_3D_IDLE(1));
	radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
	radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_VGT_FLUSH));

	if (state->enable) {
		rbuffer =(struct r600_resource*)state->esgs_ring.buffer;
		radeon_set_config_reg(cs, R_008C40_SQ_ESGS_RING_BASE, 0);
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, rbuffer,
						      RADEON_USAGE_READWRITE |
						      RADEON_PRIO_SHADER_RINGS));
		radeon_set_config_reg(cs, R_008C44_SQ_ESGS_RING_SIZE,
				state->esgs_ring.buffer_size >> 8);

		rbuffer =(struct r600_resource*)state->gsvs_ring.buffer;
		radeon_set_config_reg(cs, R_008C48_SQ_GSVS_RING_BASE, 0);
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

/* Adjust GPR allocation on R6xx/R7xx */
bool r600_adjust_gprs(struct r600_context *rctx)
{
	unsigned num_gprs[R600_NUM_HW_STAGES];
	unsigned new_gprs[R600_NUM_HW_STAGES];
	unsigned cur_gprs[R600_NUM_HW_STAGES];
	unsigned def_gprs[R600_NUM_HW_STAGES];
	unsigned def_num_clause_temp_gprs = rctx->r6xx_num_clause_temp_gprs;
	unsigned max_gprs;
	unsigned tmp, tmp2;
	unsigned i;
	bool need_recalc = false, use_default = true;

	/* hardware will reserve twice num_clause_temp_gprs */
	max_gprs = def_num_clause_temp_gprs * 2;
	for (i = 0; i < R600_NUM_HW_STAGES; i++) {
		def_gprs[i] = rctx->default_gprs[i];
		max_gprs += def_gprs[i];
	}

	cur_gprs[R600_HW_STAGE_PS] = G_008C04_NUM_PS_GPRS(rctx->config_state.sq_gpr_resource_mgmt_1);
	cur_gprs[R600_HW_STAGE_VS] = G_008C04_NUM_VS_GPRS(rctx->config_state.sq_gpr_resource_mgmt_1);
	cur_gprs[R600_HW_STAGE_GS] = G_008C08_NUM_GS_GPRS(rctx->config_state.sq_gpr_resource_mgmt_2);
	cur_gprs[R600_HW_STAGE_ES] = G_008C08_NUM_ES_GPRS(rctx->config_state.sq_gpr_resource_mgmt_2);

	num_gprs[R600_HW_STAGE_PS] = rctx->ps_shader->current->shader.bc.ngpr;
	if (rctx->gs_shader) {
		num_gprs[R600_HW_STAGE_ES] = rctx->vs_shader->current->shader.bc.ngpr;
		num_gprs[R600_HW_STAGE_GS] = rctx->gs_shader->current->shader.bc.ngpr;
		num_gprs[R600_HW_STAGE_VS] = rctx->gs_shader->current->gs_copy_shader->shader.bc.ngpr;
	} else {
		num_gprs[R600_HW_STAGE_ES] = 0;
		num_gprs[R600_HW_STAGE_GS] = 0;
		num_gprs[R600_HW_STAGE_VS] = rctx->vs_shader->current->shader.bc.ngpr;
	}

	for (i = 0; i < R600_NUM_HW_STAGES; i++) {
		new_gprs[i] = num_gprs[i];
		if (new_gprs[i] > cur_gprs[i])
			need_recalc = true;
		if (new_gprs[i] > def_gprs[i])
			use_default = false;
	}

	/* the sum of all SQ_GPR_RESOURCE_MGMT*.NUM_*_GPRS must <= to max_gprs */
	if (!need_recalc)
		return true;

	/* try to use switch back to default */
	if (!use_default) {
		/* always privilege vs stage so that at worst we have the
		 * pixel stage producing wrong output (not the vertex
		 * stage) */
		new_gprs[R600_HW_STAGE_PS] = max_gprs - def_num_clause_temp_gprs * 2;
		for (i = R600_HW_STAGE_VS; i < R600_NUM_HW_STAGES; i++)
			new_gprs[R600_HW_STAGE_PS] -= new_gprs[i];
	} else {
		for (i = 0; i < R600_NUM_HW_STAGES; i++)
			new_gprs[i] = def_gprs[i];
	}

	/* SQ_PGM_RESOURCES_*.NUM_GPRS must always be program to a value <=
	 * SQ_GPR_RESOURCE_MGMT*.NUM_*_GPRS otherwise the GPU will lockup
	 * Also if a shader use more gpr than SQ_GPR_RESOURCE_MGMT*.NUM_*_GPRS
	 * it will lockup. So in this case just discard the draw command
	 * and don't change the current gprs repartitions.
	 */
	for (i = 0; i < R600_NUM_HW_STAGES; i++) {
		if (num_gprs[i] > new_gprs[i]) {
			R600_ERR("shaders require too many register (%d + %d + %d + %d) "
				 "for a combined maximum of %d\n",
				 num_gprs[R600_HW_STAGE_PS], num_gprs[R600_HW_STAGE_VS], num_gprs[R600_HW_STAGE_ES], num_gprs[R600_HW_STAGE_GS], max_gprs);
			return false;
		}
	}

	/* in some case we endup recomputing the current value */
	tmp = S_008C04_NUM_PS_GPRS(new_gprs[R600_HW_STAGE_PS]) |
		S_008C04_NUM_VS_GPRS(new_gprs[R600_HW_STAGE_VS]) |
		S_008C04_NUM_CLAUSE_TEMP_GPRS(def_num_clause_temp_gprs);

	tmp2 = S_008C08_NUM_ES_GPRS(new_gprs[R600_HW_STAGE_ES]) |
		S_008C08_NUM_GS_GPRS(new_gprs[R600_HW_STAGE_GS]);
	if (rctx->config_state.sq_gpr_resource_mgmt_1 != tmp || rctx->config_state.sq_gpr_resource_mgmt_2 != tmp2) {
		rctx->config_state.sq_gpr_resource_mgmt_1 = tmp;
		rctx->config_state.sq_gpr_resource_mgmt_2 = tmp2;
		r600_mark_atom_dirty(rctx, &rctx->config_state.atom);
		rctx->b.flags |= R600_CONTEXT_WAIT_3D_IDLE;
	}
	return true;
}

void r600_init_atom_start_cs(struct r600_context *rctx)
{
	int ps_prio;
	int vs_prio;
	int gs_prio;
	int es_prio;
	int num_ps_gprs;
	int num_vs_gprs;
	int num_gs_gprs;
	int num_es_gprs;
	int num_temp_gprs;
	int num_ps_threads;
	int num_vs_threads;
	int num_gs_threads;
	int num_es_threads;
	int num_ps_stack_entries;
	int num_vs_stack_entries;
	int num_gs_stack_entries;
	int num_es_stack_entries;
	enum radeon_family family;
	struct r600_command_buffer *cb = &rctx->start_cs_cmd;
	uint32_t tmp, i;

	r600_init_command_buffer(cb, 256);

	/* R6xx requires this packet at the start of each command buffer */
	if (rctx->b.gfx_level == R600) {
		r600_store_value(cb, PKT3(PKT3_START_3D_CMDBUF, 0, 0));
		r600_store_value(cb, 0);
	}
	/* All asics require this one */
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

	family = rctx->b.family;
	ps_prio = 0;
	vs_prio = 1;
	gs_prio = 2;
	es_prio = 3;
	switch (family) {
	case CHIP_R600:
		num_ps_gprs = 192;
		num_vs_gprs = 56;
		num_temp_gprs = 4;
		num_gs_gprs = 0;
		num_es_gprs = 0;
		num_ps_threads = 136;
		num_vs_threads = 48;
		num_gs_threads = 4;
		num_es_threads = 4;
		num_ps_stack_entries = 128;
		num_vs_stack_entries = 128;
		num_gs_stack_entries = 0;
		num_es_stack_entries = 0;
		break;
	case CHIP_RV630:
	case CHIP_RV635:
		num_ps_gprs = 84;
		num_vs_gprs = 36;
		num_temp_gprs = 4;
		num_gs_gprs = 0;
		num_es_gprs = 0;
		num_ps_threads = 144;
		num_vs_threads = 40;
		num_gs_threads = 4;
		num_es_threads = 4;
		num_ps_stack_entries = 40;
		num_vs_stack_entries = 40;
		num_gs_stack_entries = 32;
		num_es_stack_entries = 16;
		break;
	case CHIP_RV610:
	case CHIP_RV620:
	case CHIP_RS780:
	case CHIP_RS880:
	default:
		num_ps_gprs = 84;
		num_vs_gprs = 36;
		num_temp_gprs = 4;
		num_gs_gprs = 0;
		num_es_gprs = 0;
		/* use limits 40 VS and at least 16 ES/GS */
		num_ps_threads = 120;
		num_vs_threads = 40;
		num_gs_threads = 16;
		num_es_threads = 16;
		num_ps_stack_entries = 40;
		num_vs_stack_entries = 40;
		num_gs_stack_entries = 32;
		num_es_stack_entries = 16;
		break;
	case CHIP_RV670:
		num_ps_gprs = 144;
		num_vs_gprs = 40;
		num_temp_gprs = 4;
		num_gs_gprs = 0;
		num_es_gprs = 0;
		num_ps_threads = 136;
		num_vs_threads = 48;
		num_gs_threads = 4;
		num_es_threads = 4;
		num_ps_stack_entries = 40;
		num_vs_stack_entries = 40;
		num_gs_stack_entries = 32;
		num_es_stack_entries = 16;
		break;
	case CHIP_RV770:
		num_ps_gprs = 130;
		num_vs_gprs = 56;
		num_temp_gprs = 4;
		num_gs_gprs = 31;
		num_es_gprs = 31;
		num_ps_threads = 180;
		num_vs_threads = 60;
		num_gs_threads = 4;
		num_es_threads = 4;
		num_ps_stack_entries = 128;
		num_vs_stack_entries = 128;
		num_gs_stack_entries = 128;
		num_es_stack_entries = 128;
		break;
	case CHIP_RV730:
	case CHIP_RV740:
		num_ps_gprs = 84;
		num_vs_gprs = 36;
		num_temp_gprs = 4;
		num_gs_gprs = 0;
		num_es_gprs = 0;
		num_ps_threads = 180;
		num_vs_threads = 60;
		num_gs_threads = 4;
		num_es_threads = 4;
		num_ps_stack_entries = 128;
		num_vs_stack_entries = 128;
		num_gs_stack_entries = 0;
		num_es_stack_entries = 0;
		break;
	case CHIP_RV710:
		num_ps_gprs = 192;
		num_vs_gprs = 56;
		num_temp_gprs = 4;
		num_gs_gprs = 0;
		num_es_gprs = 0;
		num_ps_threads = 136;
		num_vs_threads = 48;
		num_gs_threads = 4;
		num_es_threads = 4;
		num_ps_stack_entries = 128;
		num_vs_stack_entries = 128;
		num_gs_stack_entries = 0;
		num_es_stack_entries = 0;
		break;
	}

	rctx->default_gprs[R600_HW_STAGE_PS] = num_ps_gprs;
	rctx->default_gprs[R600_HW_STAGE_VS] = num_vs_gprs;
	rctx->default_gprs[R600_HW_STAGE_GS] = 0;
	rctx->default_gprs[R600_HW_STAGE_ES] = 0;

	rctx->r6xx_num_clause_temp_gprs = num_temp_gprs;

	/* SQ_CONFIG */
	tmp = 0;
	switch (family) {
	case CHIP_RV610:
	case CHIP_RV620:
	case CHIP_RS780:
	case CHIP_RS880:
	case CHIP_RV710:
		break;
	default:
		tmp |= S_008C00_VC_ENABLE(1);
		break;
	}
	tmp |= S_008C00_DX9_CONSTS(0);
	tmp |= S_008C00_ALU_INST_PREFER_VECTOR(1);
	tmp |= S_008C00_PS_PRIO(ps_prio);
	tmp |= S_008C00_VS_PRIO(vs_prio);
	tmp |= S_008C00_GS_PRIO(gs_prio);
	tmp |= S_008C00_ES_PRIO(es_prio);
	r600_store_config_reg(cb, R_008C00_SQ_CONFIG, tmp);

	/* SQ_GPR_RESOURCE_MGMT_2 */
	tmp = S_008C08_NUM_GS_GPRS(num_gs_gprs);
	tmp |= S_008C08_NUM_ES_GPRS(num_es_gprs);
	r600_store_config_reg_seq(cb, R_008C08_SQ_GPR_RESOURCE_MGMT_2, 4);
	r600_store_value(cb, tmp);

	/* SQ_THREAD_RESOURCE_MGMT */
	tmp = S_008C0C_NUM_PS_THREADS(num_ps_threads);
	tmp |= S_008C0C_NUM_VS_THREADS(num_vs_threads);
	tmp |= S_008C0C_NUM_GS_THREADS(num_gs_threads);
	tmp |= S_008C0C_NUM_ES_THREADS(num_es_threads);
	r600_store_value(cb, tmp); /* R_008C0C_SQ_THREAD_RESOURCE_MGMT */

	/* SQ_STACK_RESOURCE_MGMT_1 */
	tmp = S_008C10_NUM_PS_STACK_ENTRIES(num_ps_stack_entries);
	tmp |= S_008C10_NUM_VS_STACK_ENTRIES(num_vs_stack_entries);
	r600_store_value(cb, tmp); /* R_008C10_SQ_STACK_RESOURCE_MGMT_1 */

	/* SQ_STACK_RESOURCE_MGMT_2 */
	tmp = S_008C14_NUM_GS_STACK_ENTRIES(num_gs_stack_entries);
	tmp |= S_008C14_NUM_ES_STACK_ENTRIES(num_es_stack_entries);
	r600_store_value(cb, tmp); /* R_008C14_SQ_STACK_RESOURCE_MGMT_2 */

	r600_store_config_reg(cb, R_009714_VC_ENHANCE, 0);

	if (rctx->b.gfx_level >= R700) {
		r600_store_context_reg(cb, R_028A50_VGT_ENHANCE, 4);
		r600_store_config_reg(cb, R_008D8C_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ, 0x00004000);
		r600_store_config_reg(cb, R_009830_DB_DEBUG, 0);
		r600_store_config_reg(cb, R_009838_DB_WATERMARKS, 0x00420204);
		r600_store_context_reg(cb, R_0286C8_SPI_THREAD_GROUPING, 0);
	} else {
		r600_store_config_reg(cb, R_008D8C_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ, 0);
		r600_store_config_reg(cb, R_009830_DB_DEBUG, 0x82000000);
		r600_store_config_reg(cb, R_009838_DB_WATERMARKS, 0x01020204);
		r600_store_context_reg(cb, R_0286C8_SPI_THREAD_GROUPING, 1);
	}
	r600_store_context_reg_seq(cb, R_0288A8_SQ_ESGS_RING_ITEMSIZE, 9);
	r600_store_value(cb, 0); /* R_0288A8_SQ_ESGS_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_0288AC_SQ_GSVS_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_0288B0_SQ_ESTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_0288B4_SQ_GSTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_0288B8_SQ_VSTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_0288BC_SQ_PSTMP_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_0288C0_SQ_FBUF_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_0288C4_SQ_REDUC_RING_ITEMSIZE */
	r600_store_value(cb, 0); /* R_0288C8_SQ_GS_VERT_ITEMSIZE */

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

	r600_store_context_reg_seq(cb, R_028A10_VGT_OUTPUT_PATH_CNTL, 13);
	r600_store_value(cb, 0); /* R_028A10_VGT_OUTPUT_PATH_CNTL */
	r600_store_value(cb, 0); /* R_028A14_VGT_HOS_CNTL */
	r600_store_value(cb, 0); /* R_028A18_VGT_HOS_MAX_TESS_LEVEL */
	r600_store_value(cb, 0); /* R_028A1C_VGT_HOS_MIN_TESS_LEVEL */
	r600_store_value(cb, 0); /* R_028A20_VGT_HOS_REUSE_DEPTH */
	r600_store_value(cb, 0); /* R_028A24_VGT_GROUP_PRIM_TYPE */
	r600_store_value(cb, 0); /* R_028A28_VGT_GROUP_FIRST_DECR */
	r600_store_value(cb, 0); /* R_028A2C_VGT_GROUP_DECR */
	r600_store_value(cb, 0); /* R_028A30_VGT_GROUP_VECT_0_CNTL */
	r600_store_value(cb, 0); /* R_028A34_VGT_GROUP_VECT_1_CNTL */
	r600_store_value(cb, 0); /* R_028A38_VGT_GROUP_VECT_0_FMT_CNTL */
	r600_store_value(cb, 0); /* R_028A3C_VGT_GROUP_VECT_1_FMT_CNTL */
	r600_store_value(cb, 0); /* R_028A40_VGT_GS_MODE, 0); */

	r600_store_context_reg(cb, R_028A84_VGT_PRIMITIVEID_EN, 0);
	r600_store_context_reg(cb, R_028AA0_VGT_INSTANCE_STEP_RATE_0, 0);
	r600_store_context_reg(cb, R_028AA4_VGT_INSTANCE_STEP_RATE_1, 0);

	r600_store_context_reg_seq(cb, R_028AB4_VGT_REUSE_OFF, 2);
	r600_store_value(cb, 1); /* R_028AB4_VGT_REUSE_OFF */
	r600_store_value(cb, 0); /* R_028AB8_VGT_VTX_CNT_EN */

	r600_store_context_reg(cb, R_028B20_VGT_STRMOUT_BUFFER_EN, 0);

	r600_store_ctl_const(cb, R_03CFF0_SQ_VTX_BASE_VTX_LOC, 0);

	r600_store_context_reg(cb, R_028028_DB_STENCIL_CLEAR, 0);

	r600_store_context_reg_seq(cb, R_0286DC_SPI_FOG_CNTL, 3);
	r600_store_value(cb, 0); /* R_0286DC_SPI_FOG_CNTL */
	r600_store_value(cb, 0); /* R_0286E0_SPI_FOG_FUNC_SCALE */
	r600_store_value(cb, 0); /* R_0286E4_SPI_FOG_FUNC_BIAS */

	r600_store_context_reg_seq(cb, R_028D28_DB_SRESULTS_COMPARE_STATE0, 3);
	r600_store_value(cb, 0); /* R_028D28_DB_SRESULTS_COMPARE_STATE0 */
	r600_store_value(cb, 0); /* R_028D2C_DB_SRESULTS_COMPARE_STATE1 */
	r600_store_value(cb, 0); /* R_028D30_DB_PRELOAD_CONTROL */

	r600_store_context_reg(cb, R_028820_PA_CL_NANINF_CNTL, 0);
	r600_store_context_reg(cb, R_028A48_PA_SC_MPASS_PS_CNTL, 0);

	r600_store_context_reg(cb, R_028200_PA_SC_WINDOW_OFFSET, 0);
	r600_store_context_reg(cb, R_02820C_PA_SC_CLIPRECT_RULE, 0xFFFF);

	if (rctx->b.gfx_level >= R700) {
		r600_store_context_reg(cb, R_028230_PA_SC_EDGERULE, 0xAAAAAAAA);
	}

	r600_store_context_reg_seq(cb, R_028C30_CB_CLRCMP_CONTROL, 4);
	r600_store_value(cb, 0x1000000);  /* R_028C30_CB_CLRCMP_CONTROL */
	r600_store_value(cb, 0);          /* R_028C34_CB_CLRCMP_SRC */
	r600_store_value(cb, 0xFF);       /* R_028C38_CB_CLRCMP_DST */
	r600_store_value(cb, 0xFFFFFFFF); /* R_028C3C_CB_CLRCMP_MSK */

	r600_store_context_reg_seq(cb, R_028030_PA_SC_SCREEN_SCISSOR_TL, 2);
	r600_store_value(cb, 0); /* R_028030_PA_SC_SCREEN_SCISSOR_TL */
	r600_store_value(cb, S_028034_BR_X(8192) | S_028034_BR_Y(8192)); /* R_028034_PA_SC_SCREEN_SCISSOR_BR */

	r600_store_context_reg_seq(cb, R_028240_PA_SC_GENERIC_SCISSOR_TL, 2);
	r600_store_value(cb, 0); /* R_028240_PA_SC_GENERIC_SCISSOR_TL */
	r600_store_value(cb, S_028244_BR_X(8192) | S_028244_BR_Y(8192)); /* R_028244_PA_SC_GENERIC_SCISSOR_BR */

	r600_store_context_reg_seq(cb, R_0288CC_SQ_PGM_CF_OFFSET_PS, 5);
	r600_store_value(cb, 0); /* R_0288CC_SQ_PGM_CF_OFFSET_PS */
	r600_store_value(cb, 0); /* R_0288D0_SQ_PGM_CF_OFFSET_VS */
	r600_store_value(cb, 0); /* R_0288D4_SQ_PGM_CF_OFFSET_GS */
	r600_store_value(cb, 0); /* R_0288D8_SQ_PGM_CF_OFFSET_ES */
	r600_store_value(cb, 0); /* R_0288DC_SQ_PGM_CF_OFFSET_FS */

        r600_store_context_reg(cb, R_0288E0_SQ_VTX_SEMANTIC_CLEAR, ~0);

        r600_store_context_reg_seq(cb, R_028400_VGT_MAX_VTX_INDX, 2);
	r600_store_value(cb, ~0); /* R_028400_VGT_MAX_VTX_INDX */
	r600_store_value(cb, 0); /* R_028404_VGT_MIN_VTX_INDX */

	r600_store_context_reg(cb, R_0288A4_SQ_PGM_RESOURCES_FS, 0);

	if (rctx->b.gfx_level == R700)
		r600_store_context_reg(cb, R_028350_SX_MISC, 0);
	if (rctx->b.gfx_level == R700 && rctx->screen->b.has_streamout)
		r600_store_context_reg(cb, R_028354_SX_SURFACE_SYNC, S_028354_SURFACE_SYNC_MASK(0xf));

	r600_store_context_reg(cb, R_028800_DB_DEPTH_CONTROL, 0);
	if (rctx->screen->b.has_streamout) {
		r600_store_context_reg(cb, R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET, 0);
	}

	r600_store_loop_const(cb, R_03E200_SQ_LOOP_CONST_0, 0x1000FFF);
	r600_store_loop_const(cb, R_03E200_SQ_LOOP_CONST_0 + (32 * 4), 0x1000FFF);
	r600_store_loop_const(cb, R_03E200_SQ_LOOP_CONST_0 + (64 * 4), 0x1000FFF);
}

void r600_update_ps_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;
	unsigned i, exports_ps, num_cout, spi_ps_in_control_0, spi_input_z, spi_ps_in_control_1, db_shader_control;
	int pos_index = -1, face_index = -1, fixed_pt_position_index = -1;
	unsigned tmp, sid, ufi = 0;
	int need_linear = 0;
	unsigned z_export = 0, stencil_export = 0, mask_export = 0;

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

	r600_store_context_reg_seq(cb, R_028644_SPI_PS_INPUT_CNTL_0, rshader->ninput);
	for (i = 0; i < rshader->ninput; i++) {
		const gl_varying_slot varying_slot = rshader->input[i].varying_slot;

		if (varying_slot == VARYING_SLOT_POS)
			pos_index = i;
		else if (varying_slot == VARYING_SLOT_FACE) {
			if (face_index == -1)
				face_index = i;
		}
		else if (rshader->input[i].system_value == SYSTEM_VALUE_SAMPLE_ID)
			fixed_pt_position_index = i;

		sid = rshader->input[i].spi_sid;

		tmp = S_028644_SEMANTIC(sid);

		/* D3D 9 behaviour. GL is undefined */
		if (varying_slot == VARYING_SLOT_COL0)
			tmp |= S_028644_DEFAULT_VAL(3);

		if (varying_slot == VARYING_SLOT_POS ||
			rshader->input[i].interpolate == TGSI_INTERPOLATE_CONSTANT ||
			(rshader->input[i].interpolate == TGSI_INTERPOLATE_COLOR && flatshade))
			tmp |= S_028644_FLAT_SHADE(1);

		if (varying_slot == VARYING_SLOT_PNTC ||
		    (varying_slot >= VARYING_SLOT_TEX0 && varying_slot <= VARYING_SLOT_TEX7 &&
		     (sprite_coord_enable & (1 << ((int)varying_slot - (int)VARYING_SLOT_TEX0))))) {
			tmp |= S_028644_PT_SPRITE_TEX(1);
		}

		if (rshader->input[i].interpolate_location == TGSI_INTERPOLATE_LOC_CENTROID)
			tmp |= S_028644_SEL_CENTROID(1);

		if (rshader->input[i].interpolate_location == TGSI_INTERPOLATE_LOC_SAMPLE)
			tmp |= S_028644_SEL_SAMPLE(1);

		if (rshader->input[i].interpolate == TGSI_INTERPOLATE_LINEAR) {
			need_linear = 1;
			tmp |= S_028644_SEL_LINEAR(1);
		}

		r600_store_value(cb, tmp);
	}

	db_shader_control = 0;
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
	db_shader_control |= S_02880C_Z_EXPORT_ENABLE(z_export);
	db_shader_control |= S_02880C_STENCIL_REF_EXPORT_ENABLE(stencil_export);
	db_shader_control |= S_02880C_MASK_EXPORT_ENABLE(mask_export);
	if (rshader->uses_kill)
		db_shader_control |= S_02880C_KILL_ENABLE(1);

	num_cout = rshader->nr_ps_color_exports;
	exports_ps |= S_028854_EXPORT_COLORS(num_cout);
	if (!exports_ps) {
		/* always at least export 1 component per pixel */
		exports_ps = 2;
	}

	shader->nr_ps_color_outputs = num_cout;
	shader->ps_color_export_mask = rshader->ps_color_export_mask;

	spi_ps_in_control_0 = S_0286CC_NUM_INTERP(rshader->ninput) |
				S_0286CC_PERSP_GRADIENT_ENA(1)|
				S_0286CC_LINEAR_GRADIENT_ENA(need_linear);
	spi_input_z = 0;
	if (pos_index != -1) {
		spi_ps_in_control_0 |= (S_0286CC_POSITION_ENA(1) |
					S_0286CC_POSITION_CENTROID(rshader->input[pos_index].interpolate_location == TGSI_INTERPOLATE_LOC_CENTROID) |
					S_0286CC_POSITION_ADDR(rshader->input[pos_index].gpr) |
					S_0286CC_BARYC_SAMPLE_CNTL(1)) |
					S_0286CC_POSITION_SAMPLE(rshader->input[pos_index].interpolate_location == TGSI_INTERPOLATE_LOC_SAMPLE);
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

	/* HW bug in original R600 */
	if (rctx->b.family == CHIP_R600)
		ufi = 1;

	r600_store_context_reg_seq(cb, R_0286CC_SPI_PS_IN_CONTROL_0, 2);
	r600_store_value(cb, spi_ps_in_control_0); /* R_0286CC_SPI_PS_IN_CONTROL_0 */
	r600_store_value(cb, spi_ps_in_control_1); /* R_0286D0_SPI_PS_IN_CONTROL_1 */

	r600_store_context_reg(cb, R_0286D8_SPI_INPUT_Z, spi_input_z);

	r600_store_context_reg_seq(cb, R_028850_SQ_PGM_RESOURCES_PS, 2);
	r600_store_value(cb, /* R_028850_SQ_PGM_RESOURCES_PS*/
			 S_028850_NUM_GPRS(rshader->bc.ngpr) |
	/*
	 * docs are misleading about the dx10_clamp bit. This only affects
	 * instructions using CLAMP dst modifier, in which case they will
	 * return 0 with this set for a NaN (otherwise NaN).
	 */
			 S_028850_DX10_CLAMP(1) |
			 S_028850_STACK_SIZE(rshader->bc.nstack) |
			 S_028850_UNCACHED_FIRST_INST(ufi));
	r600_store_value(cb, exports_ps); /* R_028854_SQ_PGM_EXPORTS_PS */

	r600_store_context_reg(cb, R_028840_SQ_PGM_START_PS, 0);
	/* After that, the NOP relocation packet must be emitted (shader->bo, RADEON_USAGE_READ). */

	/* only set some bits here, the other bits are set in the dsa state */
	shader->db_shader_control = db_shader_control;
	shader->ps_depth_export = z_export | stencil_export | mask_export;

	shader->sprite_coord_enable = sprite_coord_enable;
	shader->flatshade = flatshade;
	shader->msaa = msaa;
}

void r600_update_vs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
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

	r600_store_context_reg_seq(cb, R_028614_SPI_VS_OUT_ID_0, 10);
	for (i = 0; i < 10; i++) {
		r600_store_value(cb, spi_vs_out_id[i]);
	}

	r600_store_context_reg(cb, R_0286C4_SPI_VS_OUT_CONFIG,
			       S_0286C4_VS_EXPORT_COUNT(rshader->highest_export_param));
	r600_store_context_reg(cb, R_028868_SQ_PGM_RESOURCES_VS,
			       S_028868_NUM_GPRS(rshader->bc.ngpr) |
			       S_028868_DX10_CLAMP(1) |
			       S_028868_STACK_SIZE(rshader->bc.nstack));
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
	r600_store_context_reg(cb, R_028858_SQ_PGM_START_VS, 0);
	/* After that, the NOP relocation packet must be emitted (shader->bo, RADEON_USAGE_READ). */

	shader->pa_cl_vs_out_cntl =
		S_02881C_VS_OUT_CCDIST0_VEC_ENA((rshader->clip_dist_write & 0x0F) != 0) |
		S_02881C_VS_OUT_CCDIST1_VEC_ENA((rshader->clip_dist_write & 0xF0) != 0) |
		S_02881C_VS_OUT_MISC_VEC_ENA(rshader->vs_out_misc_write) |
		S_02881C_USE_VTX_POINT_SIZE(rshader->vs_out_point_size) |
		S_02881C_USE_VTX_EDGE_FLAG(rshader->vs_out_edgeflag) |
		S_02881C_USE_VTX_RENDER_TARGET_INDX(rshader->vs_out_layer) |
		S_02881C_USE_VTX_VIEWPORT_INDX(rshader->vs_out_viewport);
}

#define RV610_GSVS_ALIGN 32
#define R600_GSVS_ALIGN 16

void r600_update_gs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;
	struct r600_shader *cp_shader = &shader->gs_copy_shader->shader;
	unsigned gsvs_itemsize =
			(cp_shader->ring_item_sizes[0] * shader->selector->gs_max_out_vertices) >> 2;

	/* some r600s needs gsvs itemsize aligned to cacheline size
	   this was fixed in rs780 and above. */
	switch (rctx->b.family) {
	case CHIP_RV610:
		gsvs_itemsize = align(gsvs_itemsize, RV610_GSVS_ALIGN);
		break;
	case CHIP_R600:
	case CHIP_RV630:
	case CHIP_RV670:
	case CHIP_RV620:
	case CHIP_RV635:
		gsvs_itemsize = align(gsvs_itemsize, R600_GSVS_ALIGN);
		break;
	default:
		break;
	}

	r600_init_command_buffer(cb, 64);

	/* VGT_GS_MODE is written by r600_emit_shader_stages */
	r600_store_context_reg(cb, R_028AB8_VGT_VTX_CNT_EN, 1);

	if (rctx->b.gfx_level >= R700) {
		r600_store_context_reg(cb, R_028B38_VGT_GS_MAX_VERT_OUT,
				       S_028B38_MAX_VERT_OUT(shader->selector->gs_max_out_vertices));
	}
	r600_store_context_reg(cb, R_028A6C_VGT_GS_OUT_PRIM_TYPE,
			       r600_conv_prim_to_gs_out(shader->selector->gs_output_prim));

	r600_store_context_reg(cb, R_0288C8_SQ_GS_VERT_ITEMSIZE,
	                       cp_shader->ring_item_sizes[0] >> 2);

	r600_store_context_reg(cb, R_0288A8_SQ_ESGS_RING_ITEMSIZE,
			       (rshader->ring_item_sizes[0]) >> 2);

	r600_store_context_reg(cb, R_0288AC_SQ_GSVS_RING_ITEMSIZE,
			       gsvs_itemsize);

	/* FIXME calculate these values somehow ??? */
	r600_store_config_reg_seq(cb, R_0088C8_VGT_GS_PER_ES, 2);
	r600_store_value(cb, 0x80); /* GS_PER_ES */
	r600_store_value(cb, 0x100); /* ES_PER_GS */
	r600_store_config_reg_seq(cb, R_0088E8_VGT_GS_PER_VS, 1);
	r600_store_value(cb, 0x2); /* GS_PER_VS */

	r600_store_context_reg(cb, R_02887C_SQ_PGM_RESOURCES_GS,
			       S_02887C_NUM_GPRS(rshader->bc.ngpr) |
			       S_02887C_DX10_CLAMP(1) |
			       S_02887C_STACK_SIZE(rshader->bc.nstack));
	r600_store_context_reg(cb, R_02886C_SQ_PGM_START_GS, 0);
	/* After that, the NOP relocation packet must be emitted (shader->bo, RADEON_USAGE_READ). */
}

void r600_update_es_state(struct pipe_context *ctx, struct r600_pipe_shader *shader)
{
	struct r600_command_buffer *cb = &shader->command_buffer;
	struct r600_shader *rshader = &shader->shader;

	r600_init_command_buffer(cb, 32);

	r600_store_context_reg(cb, R_028890_SQ_PGM_RESOURCES_ES,
			       S_028890_NUM_GPRS(rshader->bc.ngpr) |
			       S_028890_DX10_CLAMP(1) |
			       S_028890_STACK_SIZE(rshader->bc.nstack));
	r600_store_context_reg(cb, R_028880_SQ_PGM_START_ES, 0);
	/* After that, the NOP relocation packet must be emitted (shader->bo, RADEON_USAGE_READ). */
}


void *r600_create_resolve_blend(struct r600_context *rctx)
{
	struct pipe_blend_state blend;
	unsigned i;

	memset(&blend, 0, sizeof(blend));
	blend.independent_blend_enable = true;
	for (i = 0; i < 2; i++) {
		blend.rt[i].colormask = 0xf;
		blend.rt[i].blend_enable = 1;
		blend.rt[i].rgb_func = PIPE_BLEND_ADD;
		blend.rt[i].alpha_func = PIPE_BLEND_ADD;
		blend.rt[i].rgb_src_factor = PIPE_BLENDFACTOR_ZERO;
		blend.rt[i].rgb_dst_factor = PIPE_BLENDFACTOR_ZERO;
		blend.rt[i].alpha_src_factor = PIPE_BLENDFACTOR_ZERO;
		blend.rt[i].alpha_dst_factor = PIPE_BLENDFACTOR_ZERO;
	}
	return r600_create_blend_state_mode(&rctx->b.b, &blend, V_028808_SPECIAL_RESOLVE_BOX);
}

void *r700_create_resolve_blend(struct r600_context *rctx)
{
	struct pipe_blend_state blend;

	memset(&blend, 0, sizeof(blend));
	blend.independent_blend_enable = true;
	blend.rt[0].colormask = 0xf;
	return r600_create_blend_state_mode(&rctx->b.b, &blend, V_028808_SPECIAL_RESOLVE_BOX);
}

void *r600_create_decompress_blend(struct r600_context *rctx)
{
	struct pipe_blend_state blend;

	memset(&blend, 0, sizeof(blend));
	blend.independent_blend_enable = true;
	blend.rt[0].colormask = 0xf;
	return r600_create_blend_state_mode(&rctx->b.b, &blend, V_028808_SPECIAL_EXPAND_SAMPLES);
}

void *r600_create_db_flush_dsa(struct r600_context *rctx)
{
	struct pipe_depth_stencil_alpha_state dsa;
	bool quirk = false;

	if (rctx->b.family == CHIP_RV610 || rctx->b.family == CHIP_RV630 ||
		rctx->b.family == CHIP_RV620 || rctx->b.family == CHIP_RV635)
		quirk = true;

	memset(&dsa, 0, sizeof(dsa));

	if (quirk) {
		dsa.depth_enabled = 1;
		dsa.depth_func = PIPE_FUNC_LEQUAL;
		dsa.stencil[0].enabled = 1;
		dsa.stencil[0].func = PIPE_FUNC_ALWAYS;
		dsa.stencil[0].zpass_op = PIPE_STENCIL_OP_KEEP;
		dsa.stencil[0].zfail_op = PIPE_STENCIL_OP_INCR;
		dsa.stencil[0].writemask = 0xff;
	}

	return rctx->b.b.create_depth_stencil_alpha_state(&rctx->b.b, &dsa);
}

void r600_update_db_shader_control(struct r600_context * rctx)
{
	bool dual_export;
	unsigned db_shader_control;
	uint8_t ps_conservative_z;

	if (!rctx->ps_shader) {
		return;
	}

	dual_export = rctx->framebuffer.export_16bpc &&
		      !rctx->ps_shader->current->ps_depth_export;

	db_shader_control = rctx->ps_shader->current->db_shader_control |
			    S_02880C_DUAL_EXPORT_ENABLE(dual_export);

	ps_conservative_z = rctx->ps_shader->current->shader.ps_conservative_z;

	/* When alpha test is enabled we can't trust the hw to make the proper
	 * decision on the order in which ztest should be run related to fragment
	 * shader execution.
	 *
	 * If alpha test is enabled perform z test after fragment. RE_Z (early
	 * z test but no write to the zbuffer) seems to cause lockup on r6xx/r7xx
	 */
	if (rctx->alphatest_state.sx_alpha_test_control) {
		db_shader_control |= S_02880C_Z_ORDER(V_02880C_LATE_Z);
	} else {
		db_shader_control |= S_02880C_Z_ORDER(V_02880C_EARLY_Z_THEN_LATE_Z);
	}

	if (db_shader_control != rctx->db_misc_state.db_shader_control ||
		ps_conservative_z != rctx->db_misc_state.ps_conservative_z) {
		rctx->db_misc_state.db_shader_control = db_shader_control;
		rctx->db_misc_state.ps_conservative_z = ps_conservative_z;
		r600_mark_atom_dirty(rctx, &rctx->db_misc_state.atom);
	}
}

static inline unsigned r600_array_mode(unsigned mode)
{
	switch (mode) {
	default:
	case RADEON_SURF_MODE_LINEAR_ALIGNED:	return V_0280A0_ARRAY_LINEAR_ALIGNED;
		break;
	case RADEON_SURF_MODE_1D:		return V_0280A0_ARRAY_1D_TILED_THIN1;
		break;
	case RADEON_SURF_MODE_2D:		return V_0280A0_ARRAY_2D_TILED_THIN1;
	}
}

static bool r600_dma_copy_tile(struct r600_context *rctx,
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
	uint64_t base, addr;

	dst_mode = rdst->surface.u.legacy.level[dst_level].mode;
	src_mode = rsrc->surface.u.legacy.level[src_level].mode;
	assert(dst_mode != src_mode);

	y = 0;
	lbpp = util_logbase2(bpp);
	pitch_tile_max = ((pitch / bpp) / 8) - 1;

	if (dst_mode == RADEON_SURF_MODE_LINEAR_ALIGNED) {
		/* T2L */
		array_mode = r600_array_mode(src_mode);
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
	} else {
		/* L2T */
		array_mode = r600_array_mode(dst_mode);
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
	}
	/* check that we are in dw/base alignment constraint */
	if (addr % 4 || base % 256) {
		return false;
	}

	/* It's a r6xx/r7xx limitation, the blit must be on 8 boundary for number
	 * line in the blit. Compute max 8 line we can copy in the size limit
	 */
	cheight = ((R600_DMA_COPY_MAX_SIZE_DW * 4) / pitch) & 0xfffffff8;
	ncopy = (copy_height / cheight) + !!(copy_height % cheight);
	r600_need_dma_space(&rctx->b, ncopy * 7, &rdst->resource, &rsrc->resource);

	for (i = 0; i < ncopy; i++) {
		cheight = cheight > copy_height ? copy_height : cheight;
		size = (cheight * pitch) / 4;
		/* emit reloc before writing cs so that cs is always in consistent state */
		radeon_add_to_buffer_list(&rctx->b, &rctx->b.dma, &rsrc->resource, RADEON_USAGE_READ);
		radeon_add_to_buffer_list(&rctx->b, &rctx->b.dma, &rdst->resource, RADEON_USAGE_WRITE);
		radeon_emit(cs, DMA_PACKET(DMA_PACKET_COPY, 1, 0, size));
		radeon_emit(cs, base >> 8);
		radeon_emit(cs, (detile << 31) | (array_mode << 27) |
				(lbpp << 24) | ((height - 1) << 10) |
				pitch_tile_max);
		radeon_emit(cs, (slice_tile_max << 12) | (z << 0));
		radeon_emit(cs, (x << 3) | (y << 17));
		radeon_emit(cs, addr & 0xfffffffc);
		radeon_emit(cs, (addr >> 32UL) & 0xff);
		copy_height -= cheight;
		addr += cheight * pitch;
		y += cheight;
	}
	return true;
}

static void r600_dma_copy(struct pipe_context *ctx,
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

	if (dst->target == PIPE_BUFFER && src->target == PIPE_BUFFER) {
		if (dst_x % 4 || src_box->x % 4 || src_box->width % 4)
			goto fallback;

		r600_dma_copy_buffer(rctx, dst, src, dst_x, src_box->x, src_box->width);
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
		/* strict requirement on r6xx/r7xx */
		goto fallback;
	}
	/* lot of constraint on alignment this should capture them all */
	if (src_pitch % 8 || src_box->y % 8 || dst_y % 8) {
		goto fallback;
	}

	if (src_mode == dst_mode) {
		uint64_t dst_offset, src_offset, size;

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
		size = src_box->height * src_pitch;
		/* must be dw aligned */
		if (dst_offset % 4 || src_offset % 4 || size % 4) {
			goto fallback;
		}
		r600_dma_copy_buffer(rctx, dst, src, dst_offset, src_offset, size);
	} else {
		if (!r600_dma_copy_tile(rctx, dst, dst_level, dst_x, dst_y, dst_z,
					src, src_level, src_x, src_y, src_box->z,
					copy_height, dst_pitch, bpp)) {
			goto fallback;
		}
	}
	return;

fallback:
	r600_resource_copy_region(ctx, dst, dst_level, dstx, dsty, dstz,
				  src, src_level, src_box);
}

void r600_init_state_functions(struct r600_context *rctx)
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

	r600_init_atom(rctx, &rctx->framebuffer.atom, id++, r600_emit_framebuffer_state, 0);

	/* shader const */
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_VERTEX].atom, id++, r600_emit_vs_constant_buffers, 0);
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_GEOMETRY].atom, id++, r600_emit_gs_constant_buffers, 0);
	r600_init_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_FRAGMENT].atom, id++, r600_emit_ps_constant_buffers, 0);

	/* sampler must be emitted before TA_CNTL_AUX otherwise DISABLE_CUBE_WRAP change
	 * does not take effect (TA_CNTL_AUX emitted by r600_emit_seamless_cube_map)
	 */
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_VERTEX].states.atom, id++, r600_emit_vs_sampler_states, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_GEOMETRY].states.atom, id++, r600_emit_gs_sampler_states, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_FRAGMENT].states.atom, id++, r600_emit_ps_sampler_states, 0);
	/* resource */
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_VERTEX].views.atom, id++, r600_emit_vs_sampler_views, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_GEOMETRY].views.atom, id++, r600_emit_gs_sampler_views, 0);
	r600_init_atom(rctx, &rctx->samplers[PIPE_SHADER_FRAGMENT].views.atom, id++, r600_emit_ps_sampler_views, 0);
	r600_init_atom(rctx, &rctx->vertex_buffer_state.atom, id++, r600_emit_vertex_buffers, 0);

	r600_init_atom(rctx, &rctx->vgt_state.atom, id++, r600_emit_vgt_state, 10);

	r600_init_atom(rctx, &rctx->seamless_cube_map.atom, id++, r600_emit_seamless_cube_map, 3);
	r600_init_atom(rctx, &rctx->sample_mask.atom, id++, r600_emit_sample_mask, 3);
	rctx->sample_mask.sample_mask = ~0;

	r600_init_atom(rctx, &rctx->alphatest_state.atom, id++, r600_emit_alphatest_state, 6);
	r600_init_atom(rctx, &rctx->blend_color.atom, id++, r600_emit_blend_color, 6);
	r600_init_atom(rctx, &rctx->blend_state.atom, id++, r600_emit_cso_state, 0);
	r600_init_atom(rctx, &rctx->cb_misc_state.atom, id++, r600_emit_cb_misc_state, 7);
	r600_init_atom(rctx, &rctx->clip_misc_state.atom, id++, r600_emit_clip_misc_state, 6);
	r600_init_atom(rctx, &rctx->clip_state.atom, id++, r600_emit_clip_state, 26);
	r600_init_atom(rctx, &rctx->db_misc_state.atom, id++, r600_emit_db_misc_state, 7);
	r600_init_atom(rctx, &rctx->db_state.atom, id++, r600_emit_db_state, 11);
	r600_init_atom(rctx, &rctx->dsa_state.atom, id++, r600_emit_cso_state, 0);
	r600_init_atom(rctx, &rctx->poly_offset_state.atom, id++, r600_emit_polygon_offset, 9);
	r600_init_atom(rctx, &rctx->rasterizer_state.atom, id++, r600_emit_cso_state, 0);
	r600_add_atom(rctx, &rctx->b.scissors.atom, id++);
	r600_add_atom(rctx, &rctx->b.viewports.atom, id++);
	r600_init_atom(rctx, &rctx->config_state.atom, id++, r600_emit_config_state, 3);
	r600_init_atom(rctx, &rctx->stencil_ref.atom, id++, r600_emit_stencil_ref, 4);
	r600_init_atom(rctx, &rctx->vertex_fetch_shader.atom, id++, r600_emit_vertex_fetch_shader, 5);
	r600_add_atom(rctx, &rctx->b.render_cond_atom, id++);
	r600_add_atom(rctx, &rctx->b.streamout.begin_atom, id++);
	r600_add_atom(rctx, &rctx->b.streamout.enable_atom, id++);
	for (i = 0; i < R600_NUM_HW_STAGES; i++)
		r600_init_atom(rctx, &rctx->hw_shader_stages[i].atom, id++, r600_emit_shader, 0);
	r600_init_atom(rctx, &rctx->shader_stages.atom, id++, r600_emit_shader_stages, 0);
	r600_init_atom(rctx, &rctx->gs_rings.atom, id++, r600_emit_gs_rings, 0);

	rctx->b.b.create_blend_state = r600_create_blend_state;
	rctx->b.b.create_depth_stencil_alpha_state = r600_create_dsa_state;
	rctx->b.b.create_rasterizer_state = r600_create_rs_state;
	rctx->b.b.create_sampler_state = r600_create_sampler_state;
	rctx->b.b.create_sampler_view = r600_create_sampler_view;
	rctx->b.b.set_framebuffer_state = r600_set_framebuffer_state;
	rctx->b.b.set_polygon_stipple = r600_set_polygon_stipple;
	rctx->b.b.set_min_samples = r600_set_min_samples;
	rctx->b.b.get_sample_position = r600_get_sample_position;
	rctx->b.dma_copy = r600_dma_copy;
}
/* this function must be last */
