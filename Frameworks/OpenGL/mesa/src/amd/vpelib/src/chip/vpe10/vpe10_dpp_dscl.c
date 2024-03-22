/* Copyright 2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#include "vpe_priv.h"
#include "vpe10_dpp.h"

#define CTX      vpe10_dpp
#define CTX_BASE dpp

#define NUM_PHASES    64
#define HORZ_MAX_TAPS 8
#define VERT_MAX_TAPS 8

#define LB_MAX_PARTITION 12

enum vpe10_coef_filter_type_sel {
    SCL_COEF_LUMA_VERT_FILTER   = 0,
    SCL_COEF_LUMA_HORZ_FILTER   = 1,
    SCL_COEF_CHROMA_VERT_FILTER = 2,
    SCL_COEF_CHROMA_HORZ_FILTER = 3,
    SCL_COEF_ALPHA_VERT_FILTER  = 4,
    SCL_COEF_ALPHA_HORZ_FILTER  = 5
};

enum dscl_autocal_mode {
    AUTOCAL_MODE_OFF = 0,

    /* Autocal calculate the scaling ratio and initial phase and the
     * DSCL_MODE_SEL must be set to 1
     */
    AUTOCAL_MODE_AUTOSCALE = 1,
    /* Autocal perform auto centering without replication and the
     * DSCL_MODE_SEL must be set to 0
     */
    AUTOCAL_MODE_AUTOCENTER = 2,
    /* Autocal perform auto centering and auto replication and the
     * DSCL_MODE_SEL must be set to 0
     */
    AUTOCAL_MODE_AUTOREPLICATE = 3
};

enum dscl_mode_sel {
    DSCL_MODE_SCALING_444_BYPASS        = 0,
    DSCL_MODE_SCALING_444_RGB_ENABLE    = 1,
    DSCL_MODE_SCALING_444_YCBCR_ENABLE  = 2,
    DSCL_MODE_SCALING_420_YCBCR_ENABLE  = 3,
    DSCL_MODE_SCALING_420_LUMA_BYPASS   = 4,
    DSCL_MODE_SCALING_420_CHROMA_BYPASS = 5,
    DSCL_MODE_DSCL_BYPASS               = 6
};

static bool dpp1_dscl_is_ycbcr(const enum vpe_surface_pixel_format format)
{
    return format >= VPE_SURFACE_PIXEL_FORMAT_VIDEO_BEGIN &&
           format <= VPE_SURFACE_PIXEL_FORMAT_VIDEO_END;
}

static bool dpp1_dscl_is_video_subsampled(const enum vpe_surface_pixel_format format)
{
    return (format >= VPE_SURFACE_PIXEL_FORMAT_VIDEO_BEGIN &&
            format <= VPE_SURFACE_PIXEL_FORMAT_SUBSAMPLE_END);
}

static enum dscl_mode_sel dpp1_dscl_get_dscl_mode(const struct scaler_data *data)
{

    // TODO Check if bypass bit enabled
    const long long one = vpe_fixpt_one.value;

    if (data->ratios.horz.value == one && data->ratios.vert.value == one &&
        data->ratios.horz_c.value == one && data->ratios.vert_c.value == one)
        return DSCL_MODE_DSCL_BYPASS;

    if (!dpp1_dscl_is_ycbcr(data->format))
        return DSCL_MODE_SCALING_444_RGB_ENABLE;

    if (!dpp1_dscl_is_video_subsampled(data->format))
        return DSCL_MODE_SCALING_444_YCBCR_ENABLE;

    if (data->ratios.horz.value == one && data->ratios.vert.value == one)
        return DSCL_MODE_SCALING_420_LUMA_BYPASS;

    return DSCL_MODE_SCALING_420_YCBCR_ENABLE;
}

static void dpp1_dscl_set_dscl_mode(struct dpp *dpp, enum dscl_mode_sel dscl_mode)
{

    PROGRAM_ENTRY();

    REG_SET(VPDSCL_MODE, 0, VPDSCL_MODE, dscl_mode);
}

static void dpp1_dscl_set_recout(struct dpp *dpp, const struct vpe_rect *recout)
{

    PROGRAM_ENTRY();

    REG_SET_2(VPDSCL_RECOUT_START, 0, RECOUT_START_X, recout->x, RECOUT_START_Y, recout->y);

    REG_SET_2(VPDSCL_RECOUT_SIZE, 0, RECOUT_WIDTH, recout->width, RECOUT_HEIGHT, recout->height);
}

static void dpp1_dscl_set_mpc_size(struct dpp *dpp, const struct scaler_data *scl_data)
{

    PROGRAM_ENTRY();

    REG_SET_2(VPMPC_SIZE, 0, VPMPC_WIDTH, scl_data->h_active, VPMPC_HEIGHT, scl_data->v_active);
}

static void dpp1_dscl_set_h_blank(struct dpp *dpp, uint16_t start, uint16_t end)
{

    PROGRAM_ENTRY();
    REG_SET_2(VPOTG_H_BLANK, 0, OTG_H_BLANK_END, end, OTG_H_BLANK_START, start);
}

static void dpp1_dscl_set_v_blank(struct dpp *dpp, uint16_t start, uint16_t end)
{

    PROGRAM_ENTRY();
    REG_SET_2(VPOTG_V_BLANK, 0, OTG_V_BLANK_END, end, OTG_V_BLANK_START, start);
}

static void dpp1_dscl_set_taps(struct dpp *dpp, const struct scaler_data *scl_data)
{

    PROGRAM_ENTRY();

    REG_SET_4(VPDSCL_TAP_CONTROL, 0, SCL_V_NUM_TAPS, scl_data->taps.v_taps - 1, SCL_H_NUM_TAPS,
        scl_data->taps.h_taps - 1, SCL_V_NUM_TAPS_C, scl_data->taps.v_taps_c - 1, SCL_H_NUM_TAPS_C,
        scl_data->taps.h_taps_c - 1);
}

static const uint16_t *dpp1_dscl_get_filter_coeffs_64p(int taps, struct fixed31_32 ratio)
{
    if (taps == 8)
        return vpe_get_filter_8tap_64p(ratio);
    else if (taps == 6)
        return vpe_get_filter_6tap_64p(ratio);
    else if (taps == 4)
        return vpe_get_filter_4tap_64p(ratio);
    else if (taps == 2)
        return vpe_get_2tap_bilinear_64p();
    else if (taps == 1)
        return NULL;
    else {
        /* should never happen, bug */
        return NULL;
    }
}

static void dpp1_dscl_set_scaler_filter(struct dpp *dpp, uint32_t taps,
    enum vpe10_coef_filter_type_sel filter_type, const uint16_t *filter)
{
    const int tap_pairs = (taps + 1) / 2;
    int       phase;
    int       pair;
    uint16_t  odd_coef, even_coef;

    PROGRAM_ENTRY();

    REG_SET_3(VPDSCL_COEF_RAM_TAP_SELECT, 0, SCL_COEF_RAM_TAP_PAIR_IDX, 0, SCL_COEF_RAM_PHASE, 0,
        SCL_COEF_RAM_FILTER_TYPE, filter_type);

    for (phase = 0; phase < (NUM_PHASES / 2 + 1); phase++) {
        for (pair = 0; pair < tap_pairs; pair++) {
            even_coef = filter[phase * (int)taps + 2 * pair];
            if ((pair * 2 + 1) < (int)taps)
                odd_coef = filter[phase * (int)taps + 2 * pair + 1];
            else
                odd_coef = 0;

            REG_SET_4(VPDSCL_COEF_RAM_TAP_DATA, 0,
                /* Even tap coefficient (bits 1:0 fixed to 0) */
                SCL_COEF_RAM_EVEN_TAP_COEF, even_coef,
                /* Write/read control for even coefficient */
                SCL_COEF_RAM_EVEN_TAP_COEF_EN, 1,
                /* Odd tap coefficient (bits 1:0 fixed to 0) */
                SCL_COEF_RAM_ODD_TAP_COEF, odd_coef,
                /* Write/read control for odd coefficient */
                SCL_COEF_RAM_ODD_TAP_COEF_EN, 1);
        }
    }
}

static void dpp1_dscl_set_scl_filter(struct dpp *dpp, const struct scaler_data *scl_data,
    enum dscl_mode_sel scl_mode, bool chroma_coef_mode)
{

    const uint16_t *filter_h   = NULL;
    const uint16_t *filter_v   = NULL;
    const uint16_t *filter_h_c = NULL;
    const uint16_t *filter_v_c = NULL;

    PROGRAM_ENTRY();

    if (scl_data->polyphase_filter_coeffs == 0) /*no externally provided set of coeffs and taps*/
    {
        filter_h = (uint16_t *)dpp1_dscl_get_filter_coeffs_64p(
            (int)scl_data->taps.h_taps, scl_data->ratios.horz);
        filter_v =
            dpp1_dscl_get_filter_coeffs_64p((int)scl_data->taps.v_taps, scl_data->ratios.vert);
    } else {
        filter_h = (const uint16_t *)&scl_data->polyphase_filter_coeffs->horiz_polyphase_coeffs;
        filter_v = (const uint16_t *)&scl_data->polyphase_filter_coeffs->vert_polyphase_coeffs;
    }
    if (filter_h != NULL)
        dpp1_dscl_set_scaler_filter(
            dpp, scl_data->taps.h_taps, SCL_COEF_LUMA_HORZ_FILTER, filter_h);

    if (filter_v != NULL)
        dpp1_dscl_set_scaler_filter(
            dpp, scl_data->taps.v_taps, SCL_COEF_LUMA_VERT_FILTER, filter_v);

    if (chroma_coef_mode) {

        filter_h_c =
            dpp1_dscl_get_filter_coeffs_64p((int)scl_data->taps.h_taps_c, scl_data->ratios.horz_c);
        filter_v_c =
            dpp1_dscl_get_filter_coeffs_64p((int)scl_data->taps.v_taps_c, scl_data->ratios.vert_c);

        if (filter_h_c != NULL)
            dpp1_dscl_set_scaler_filter(
                dpp, scl_data->taps.h_taps_c, SCL_COEF_CHROMA_HORZ_FILTER, filter_h_c);

        if (filter_v_c != NULL)
            dpp1_dscl_set_scaler_filter(
                dpp, scl_data->taps.v_taps_c, SCL_COEF_CHROMA_VERT_FILTER, filter_v_c);
    }

    REG_UPDATE(VPDSCL_MODE, SCL_CHROMA_COEF_MODE, chroma_coef_mode);
}

static void dpp1_dscl_set_lb(struct dpp *dpp, const struct line_buffer_params *lb_params,
    enum lb_memory_config mem_size_config)
{

    PROGRAM_ENTRY();

    REG_SET(VPLB_DATA_FORMAT, 0, ALPHA_EN, lb_params->alpha_en); /* Alpha enable */

    REG_SET_2(
        VPLB_MEMORY_CTRL, 0, MEMORY_CONFIG, mem_size_config, LB_MAX_PARTITIONS, LB_MAX_PARTITION);
}

static void dpp1_dscl_set_scale_ratio(struct dpp *dpp, const struct scaler_data *data)
{

    PROGRAM_ENTRY();

    REG_SET(VPDSCL_HORZ_FILTER_SCALE_RATIO, 0, SCL_H_SCALE_RATIO,
        vpe_fixpt_u3d19(data->ratios.horz) << 5);

    REG_SET(VPDSCL_VERT_FILTER_SCALE_RATIO, 0, SCL_V_SCALE_RATIO,
        vpe_fixpt_u3d19(data->ratios.vert) << 5);

    REG_SET(VPDSCL_HORZ_FILTER_SCALE_RATIO_C, 0, SCL_H_SCALE_RATIO_C,
        vpe_fixpt_u3d19(data->ratios.horz_c) << 5);

    REG_SET(VPDSCL_VERT_FILTER_SCALE_RATIO_C, 0, SCL_V_SCALE_RATIO_C,
        vpe_fixpt_u3d19(data->ratios.vert_c) << 5);
}

static void dpp1_dscl_set_scaler_position(struct dpp *dpp, const struct scaler_data *data)
{
    uint32_t init_frac = 0;
    uint32_t init_int  = 0;

    PROGRAM_ENTRY();

    /*
     * 0.24 format for fraction, first five bits zeroed
     */
    init_frac = vpe_fixpt_u0d19(data->inits.h) << 5;
    init_int  = (uint32_t)vpe_fixpt_floor(data->inits.h);
    REG_SET_2(VPDSCL_HORZ_FILTER_INIT, 0, SCL_H_INIT_FRAC, init_frac, SCL_H_INIT_INT, init_int);

    init_frac = vpe_fixpt_u0d19(data->inits.h_c) << 5;
    init_int  = (uint32_t)vpe_fixpt_floor(data->inits.h_c);
    REG_SET_2(
        VPDSCL_HORZ_FILTER_INIT_C, 0, SCL_H_INIT_FRAC_C, init_frac, SCL_H_INIT_INT_C, init_int);

    init_frac = vpe_fixpt_u0d19(data->inits.v) << 5;
    init_int  = (uint32_t)vpe_fixpt_floor(data->inits.v);
    REG_SET_2(VPDSCL_VERT_FILTER_INIT, 0, SCL_V_INIT_FRAC, init_frac, SCL_V_INIT_INT, init_int);

    init_frac = vpe_fixpt_u0d19(data->inits.v_c) << 5;
    init_int  = (uint32_t)vpe_fixpt_floor(data->inits.v_c);
    REG_SET_2(
        VPDSCL_VERT_FILTER_INIT_C, 0, SCL_V_INIT_FRAC_C, init_frac, SCL_V_INIT_INT_C, init_int);
}

static void dpp1_power_on_dscl(struct dpp *dpp, bool power_on)
{
    PROGRAM_ENTRY();

    if (dpp->vpe_priv->init.debug.enable_mem_low_power.bits.dscl) {
        if (power_on) {
            REG_SET_2(VPDSCL_MEM_PWR_CTRL, REG_DEFAULT(VPDSCL_MEM_PWR_CTRL), LUT_MEM_PWR_DIS, 0,
                LUT_MEM_PWR_FORCE, 0);

            // introduce a delay by dummy set
            REG_SET_2(VPDSCL_MEM_PWR_CTRL, REG_DEFAULT(VPDSCL_MEM_PWR_CTRL), LUT_MEM_PWR_DIS, 0,
                LUT_MEM_PWR_FORCE, 0);

            REG_SET_2(VPDSCL_MEM_PWR_CTRL, REG_DEFAULT(VPDSCL_MEM_PWR_CTRL), LUT_MEM_PWR_DIS, 0,
                LUT_MEM_PWR_FORCE, 0);
        } else {
            REG_SET_2(VPDSCL_MEM_PWR_CTRL, REG_DEFAULT(VPDSCL_MEM_PWR_CTRL), LUT_MEM_PWR_DIS, 0,
                LUT_MEM_PWR_FORCE, 3);
        }
    } else {
        if (power_on) {
            REG_SET_2(VPDSCL_MEM_PWR_CTRL, REG_DEFAULT(VPDSCL_MEM_PWR_CTRL), LUT_MEM_PWR_DIS, 1,
                LUT_MEM_PWR_FORCE, 0);
        } else {
            REG_SET_2(VPDSCL_MEM_PWR_CTRL, REG_DEFAULT(VPDSCL_MEM_PWR_CTRL), LUT_MEM_PWR_DIS, 0,
                LUT_MEM_PWR_FORCE, 0);
        }
    }
}

void vpe10_dpp_set_segment_scaler(struct dpp *dpp, const struct scaler_data *scl_data)
{

    enum dscl_mode_sel dscl_mode = dpp1_dscl_get_dscl_mode(scl_data);

    dpp1_dscl_set_recout(dpp, &scl_data->recout);
    dpp1_dscl_set_mpc_size(dpp, scl_data);

    if (dscl_mode == DSCL_MODE_DSCL_BYPASS)
        return;

    dpp1_dscl_set_scaler_position(dpp, scl_data);
}

void vpe10_dpp_set_frame_scaler(struct dpp *dpp, const struct scaler_data *scl_data)
{

    enum dscl_mode_sel dscl_mode = dpp1_dscl_get_dscl_mode(scl_data);
    bool               ycbcr     = dpp1_dscl_is_ycbcr(scl_data->format);

    dpp1_dscl_set_h_blank(dpp, 1, 0);
    dpp1_dscl_set_v_blank(dpp, 1, 0);

    if (dscl_mode != DSCL_MODE_DSCL_BYPASS)
        dpp1_power_on_dscl(dpp, true);

    dpp1_dscl_set_dscl_mode(dpp, dscl_mode);

    if (dscl_mode == DSCL_MODE_DSCL_BYPASS) {
        dpp1_power_on_dscl(dpp, false);
        return;
    }

    dpp1_dscl_set_lb(dpp, &scl_data->lb_params, LB_MEMORY_CONFIG_0);
    dpp1_dscl_set_scale_ratio(dpp, scl_data);
    dpp1_dscl_set_taps(dpp, scl_data);
    dpp1_dscl_set_scl_filter(dpp, scl_data, dscl_mode, ycbcr);
}
