/*
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 * Authors: Marek Olšák <maraeo@gmail.com>
 *
 */

#include "r600_cs.h"
#include "evergreend.h"

/* 2xMSAA
 * There are two locations (4, 4), (-4, -4). */
const uint32_t eg_sample_locs_2x[4] = {
	FILL_SREG(4, 4, -4, -4, 4, 4, -4, -4),
	FILL_SREG(4, 4, -4, -4, 4, 4, -4, -4),
	FILL_SREG(4, 4, -4, -4, 4, 4, -4, -4),
	FILL_SREG(4, 4, -4, -4, 4, 4, -4, -4),
};
const unsigned eg_max_dist_2x = 4;
/* 4xMSAA
 * There are 4 locations: (-2, 6), (6, -2), (-6, 2), (2, 6). */
const uint32_t eg_sample_locs_4x[4] = {
	FILL_SREG(-2, -6, 6, -2, -6, 2, 2, 6),
	FILL_SREG(-2, -6, 6, -2, -6, 2, 2, 6),
	FILL_SREG(-2, -6, 6, -2, -6, 2, 2, 6),
	FILL_SREG(-2, -6, 6, -2, -6, 2, 2, 6),
};
const unsigned eg_max_dist_4x = 6;

/* Cayman 8xMSAA */
static const uint32_t cm_sample_locs_8x[] = {
	FILL_SREG( 1, -3, -1,  3, 5,  1, -3, -5),
	FILL_SREG( 1, -3, -1,  3, 5,  1, -3, -5),
	FILL_SREG( 1, -3, -1,  3, 5,  1, -3, -5),
	FILL_SREG( 1, -3, -1,  3, 5,  1, -3, -5),
	FILL_SREG(-5,  5, -7, -1, 3,  7,  7, -7),
	FILL_SREG(-5,  5, -7, -1, 3,  7,  7, -7),
	FILL_SREG(-5,  5, -7, -1, 3,  7,  7, -7),
	FILL_SREG(-5,  5, -7, -1, 3,  7,  7, -7),
};
static const unsigned cm_max_dist_8x = 8;
/* Cayman 16xMSAA */
static const uint32_t cm_sample_locs_16x[] = {
	FILL_SREG( 1,  1, -1, -3, -3,  2,  4, -1),
	FILL_SREG( 1,  1, -1, -3, -3,  2,  4, -1),
	FILL_SREG( 1,  1, -1, -3, -3,  2,  4, -1),
	FILL_SREG( 1,  1, -1, -3, -3,  2,  4, -1),
	FILL_SREG(-5, -2,  2,  5,  5,  3,  3, -5),
	FILL_SREG(-5, -2,  2,  5,  5,  3,  3, -5),
	FILL_SREG(-5, -2,  2,  5,  5,  3,  3, -5),
	FILL_SREG(-5, -2,  2,  5,  5,  3,  3, -5),
	FILL_SREG(-2,  6,  0, -7, -4, -6, -6,  4),
	FILL_SREG(-2,  6,  0, -7, -4, -6, -6,  4),
	FILL_SREG(-2,  6,  0, -7, -4, -6, -6,  4),
	FILL_SREG(-2,  6,  0, -7, -4, -6, -6,  4),
	FILL_SREG(-8,  0,  7, -4,  6,  7, -7, -8),
	FILL_SREG(-8,  0,  7, -4,  6,  7, -7, -8),
	FILL_SREG(-8,  0,  7, -4,  6,  7, -7, -8),
	FILL_SREG(-8,  0,  7, -4,  6,  7, -7, -8),
};
static const unsigned cm_max_dist_16x = 8;

void cayman_get_sample_position(struct pipe_context *ctx, unsigned sample_count,
				unsigned sample_index, float *out_value)
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
		index = (sample_index / 4) * 4;
		val.idx = (cm_sample_locs_8x[index] >> offset) & 0xf;
		out_value[0] = (float)(val.idx + 8) / 16.0f;
		val.idx = (cm_sample_locs_8x[index] >> (offset + 4)) & 0xf;
		out_value[1] = (float)(val.idx + 8) / 16.0f;
		break;
	case 16:
		offset = 4 * (sample_index % 4 * 2);
		index = (sample_index / 4) * 4;
		val.idx = (cm_sample_locs_16x[index] >> offset) & 0xf;
		out_value[0] = (float)(val.idx + 8) / 16.0f;
		val.idx = (cm_sample_locs_16x[index] >> (offset + 4)) & 0xf;
		out_value[1] = (float)(val.idx + 8) / 16.0f;
		break;
	}
}

void cayman_init_msaa(struct pipe_context *ctx)
{
	struct r600_common_context *rctx = (struct r600_common_context*)ctx;
	int i;

	cayman_get_sample_position(ctx, 1, 0, rctx->sample_locations_1x[0]);

	for (i = 0; i < 2; i++)
		cayman_get_sample_position(ctx, 2, i, rctx->sample_locations_2x[i]);
	for (i = 0; i < 4; i++)
		cayman_get_sample_position(ctx, 4, i, rctx->sample_locations_4x[i]);
	for (i = 0; i < 8; i++)
		cayman_get_sample_position(ctx, 8, i, rctx->sample_locations_8x[i]);
	for (i = 0; i < 16; i++)
		cayman_get_sample_position(ctx, 16, i, rctx->sample_locations_16x[i]);
}

static void cayman_emit_msaa_sample_locs(struct radeon_cmdbuf *cs, int nr_samples)
{
	switch (nr_samples) {
	default:
	case 1:
		radeon_set_context_reg(cs, CM_R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, 0);
		radeon_set_context_reg(cs, CM_R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0, 0);
		radeon_set_context_reg(cs, CM_R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0, 0);
		radeon_set_context_reg(cs, CM_R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0, 0);
		break;
	case 2:
		radeon_set_context_reg(cs, CM_R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, eg_sample_locs_2x[0]);
		radeon_set_context_reg(cs, CM_R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0, eg_sample_locs_2x[1]);
		radeon_set_context_reg(cs, CM_R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0, eg_sample_locs_2x[2]);
		radeon_set_context_reg(cs, CM_R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0, eg_sample_locs_2x[3]);
		break;
	case 4:
		radeon_set_context_reg(cs, CM_R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, eg_sample_locs_4x[0]);
		radeon_set_context_reg(cs, CM_R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0, eg_sample_locs_4x[1]);
		radeon_set_context_reg(cs, CM_R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0, eg_sample_locs_4x[2]);
		radeon_set_context_reg(cs, CM_R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0, eg_sample_locs_4x[3]);
		break;
	case 8:
		radeon_set_context_reg_seq(cs, CM_R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, 14);
		radeon_emit(cs, cm_sample_locs_8x[0]);
		radeon_emit(cs, cm_sample_locs_8x[4]);
		radeon_emit(cs, 0);
		radeon_emit(cs, 0);
		radeon_emit(cs, cm_sample_locs_8x[1]);
		radeon_emit(cs, cm_sample_locs_8x[5]);
		radeon_emit(cs, 0);
		radeon_emit(cs, 0);
		radeon_emit(cs, cm_sample_locs_8x[2]);
		radeon_emit(cs, cm_sample_locs_8x[6]);
		radeon_emit(cs, 0);
		radeon_emit(cs, 0);
		radeon_emit(cs, cm_sample_locs_8x[3]);
		radeon_emit(cs, cm_sample_locs_8x[7]);
		break;
	case 16:
		radeon_set_context_reg_seq(cs, CM_R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, 16);
		radeon_emit(cs, cm_sample_locs_16x[0]);
		radeon_emit(cs, cm_sample_locs_16x[4]);
		radeon_emit(cs, cm_sample_locs_16x[8]);
		radeon_emit(cs, cm_sample_locs_16x[12]);
		radeon_emit(cs, cm_sample_locs_16x[1]);
		radeon_emit(cs, cm_sample_locs_16x[5]);
		radeon_emit(cs, cm_sample_locs_16x[9]);
		radeon_emit(cs, cm_sample_locs_16x[13]);
		radeon_emit(cs, cm_sample_locs_16x[2]);
		radeon_emit(cs, cm_sample_locs_16x[6]);
		radeon_emit(cs, cm_sample_locs_16x[10]);
		radeon_emit(cs, cm_sample_locs_16x[14]);
		radeon_emit(cs, cm_sample_locs_16x[3]);
		radeon_emit(cs, cm_sample_locs_16x[7]);
		radeon_emit(cs, cm_sample_locs_16x[11]);
		radeon_emit(cs, cm_sample_locs_16x[15]);
		break;
	}
}

void cayman_emit_msaa_state(struct radeon_cmdbuf *cs, int nr_samples,
			    int ps_iter_samples, int overrast_samples)
{
	int setup_samples = nr_samples > 1 ? nr_samples :
			    overrast_samples > 1 ? overrast_samples : 0;
	/* Required by OpenGL line rasterization.
	 *
	 * TODO: We should also enable perpendicular endcaps for AA lines,
	 *       but that requires implementing line stippling in the pixel
	 *       shader. SC can only do line stippling with axis-aligned
	 *       endcaps.
	 */
	unsigned sc_line_cntl = S_028BDC_DX10_DIAMOND_TEST_ENA(1);
	unsigned sc_mode_cntl_1 =
		EG_S_028A4C_FORCE_EOV_CNTDWN_ENABLE(1) |
		EG_S_028A4C_FORCE_EOV_REZ_ENABLE(1);

	if (nr_samples > 1) {
		cayman_emit_msaa_sample_locs(cs, nr_samples);
	}

	if (setup_samples > 1) {
		/* indexed by log2(nr_samples) */
		const unsigned max_dist[] = {
			0,
			eg_max_dist_2x,
			eg_max_dist_4x,
			cm_max_dist_8x,
			cm_max_dist_16x
		};
		unsigned log_samples = util_logbase2(setup_samples);
		unsigned log_ps_iter_samples =
			util_logbase2(util_next_power_of_two(ps_iter_samples));

		radeon_set_context_reg_seq(cs, CM_R_028BDC_PA_SC_LINE_CNTL, 2);
		radeon_emit(cs, sc_line_cntl |
			    S_028BDC_EXPAND_LINE_WIDTH(1)); /* CM_R_028BDC_PA_SC_LINE_CNTL */
		radeon_emit(cs, S_028BE0_MSAA_NUM_SAMPLES(log_samples) |
			    S_028BE0_MAX_SAMPLE_DIST(max_dist[log_samples]) |
			    S_028BE0_MSAA_EXPOSED_SAMPLES(log_samples)); /* CM_R_028BE0_PA_SC_AA_CONFIG */

		if (nr_samples > 1) {
			radeon_set_context_reg(cs, CM_R_028804_DB_EQAA,
					       S_028804_MAX_ANCHOR_SAMPLES(log_samples) |
					       S_028804_PS_ITER_SAMPLES(log_ps_iter_samples) |
					       S_028804_MASK_EXPORT_NUM_SAMPLES(log_samples) |
					       S_028804_ALPHA_TO_MASK_NUM_SAMPLES(log_samples) |
					       S_028804_HIGH_QUALITY_INTERSECTIONS(1) |
					       S_028804_STATIC_ANCHOR_ASSOCIATIONS(1));
			radeon_set_context_reg(cs, EG_R_028A4C_PA_SC_MODE_CNTL_1,
					       EG_S_028A4C_PS_ITER_SAMPLE(ps_iter_samples > 1) |
					       sc_mode_cntl_1);
		} else if (overrast_samples > 1) {
			radeon_set_context_reg(cs, CM_R_028804_DB_EQAA,
					       S_028804_HIGH_QUALITY_INTERSECTIONS(1) |
					       S_028804_STATIC_ANCHOR_ASSOCIATIONS(1) |
					       S_028804_OVERRASTERIZATION_AMOUNT(log_samples));
			radeon_set_context_reg(cs, EG_R_028A4C_PA_SC_MODE_CNTL_1,
					       sc_mode_cntl_1);
		}
	} else {
		radeon_set_context_reg_seq(cs, CM_R_028BDC_PA_SC_LINE_CNTL, 2);
		radeon_emit(cs, sc_line_cntl); /* CM_R_028BDC_PA_SC_LINE_CNTL */
		radeon_emit(cs, 0); /* CM_R_028BE0_PA_SC_AA_CONFIG */

		radeon_set_context_reg(cs, CM_R_028804_DB_EQAA,
				       S_028804_HIGH_QUALITY_INTERSECTIONS(1) |
				       S_028804_STATIC_ANCHOR_ASSOCIATIONS(1));
		radeon_set_context_reg(cs, EG_R_028A4C_PA_SC_MODE_CNTL_1,
				       sc_mode_cntl_1);
	}
}
