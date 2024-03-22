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

#pragma once

#include "vpe_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mpc;
struct vpe_priv;
struct output_ctx;

enum mpc_mpccid {
    MPC_MPCCID_0 = 0,
    MPC_MPCCID_COUNT,
};

enum mpc_mux_topsel {
    MPC_MUX_TOPSEL_DPP0    = 0,
    MPC_MUX_TOPSEL_DISABLE = 0x0f,
};

enum mpc_mux_botsel {
    MPC_MUX_BOTSEL_MPCC0   = 0,
    MPC_MUX_BOTSEL_DISABLE = 0x0f,
};

enum mpc_mux_outmux {
    MPC_MUX_OUTMUX_MPCC0   = 0,
    MPC_MUX_OUTMUX_DISABLE = 0x0f,
};

enum mpc_mux_oppid {
    MPC_MUX_OPPID_OPP0    = 0,
    MPC_MUX_OPPID_DISABLE = 0x0f,
};

enum mpcc_blend_mode {
    MPCC_BLEND_MODE_BYPASS,                // Direct digital bypass
    MPCC_BLEND_MODE_TOP_LAYER_PASSTHROUGH, // Top layer pass-through
    MPCC_BLEND_MODE_TOP_LAYER_ONLY,        // Top layer bleneded with background color
    MPCC_BLEND_MODE_TOP_BOT_BLENDING       // Top and bottom blending
};

enum mpcc_alpha_blend_mode {
    MPCC_ALPHA_BLEND_MODE_PER_PIXEL_ALPHA,
    MPCC_ALPHA_BLEND_MODE_PER_PIXEL_ALPHA_COMBINED_GLOBAL_GAIN,
    MPCC_ALPHA_BLEND_MODE_GLOBAL_ALPHA
};

/*
 * MPCC blending configuration
 */
struct mpcc_blnd_cfg {
    struct vpe_color           bg_color;             /* background color */
    enum mpcc_alpha_blend_mode alpha_mode;           /* alpha blend mode */
    bool                       pre_multiplied_alpha; /* alpha pre-multiplied mode flag */
    uint8_t                    global_gain;
    uint8_t                    global_alpha;
    bool                       overlap_only;

    /* MPCC top/bottom gain settings */
    int bottom_gain_mode;
    int background_color_bpc;
    int top_gain;
    int bottom_inside_gain;
    int bottom_outside_gain;
};

enum mpc_output_csc_mode {
    MPC_OUTPUT_CSC_DISABLE = 0,
    MPC_OUTPUT_CSC_COEF_A,
};

struct mpc_denorm_clamp {
    int clamp_max_r_cr;
    int clamp_min_r_cr;
    int clamp_max_g_y;
    int clamp_min_g_y;
    int clamp_max_b_cb;
    int clamp_min_b_cb;
};

struct mpc_funcs {
    // TODO finalize it
    void (*program_mpcc_mux)(struct mpc *mpc, enum mpc_mpccid mpcc_idx, enum mpc_mux_topsel topsel,
        enum mpc_mux_botsel botsel, enum mpc_mux_outmux outmux, enum mpc_mux_oppid oppid);

    void (*program_mpcc_blending)(
        struct mpc *mpc, enum mpc_mpccid mpcc_idx, struct mpcc_blnd_cfg *blnd_cfg);

    void (*program_mpc_bypass_bg_color)(struct mpc *mpc, struct mpcc_blnd_cfg *blnd_cfg);

    void (*power_on_ogam_lut)(struct mpc *mpc, bool power_on);

    void (*set_output_csc)(
        struct mpc *mpc, const uint16_t *regval, enum mpc_output_csc_mode ocsc_mode);

    void (*set_ocsc_default)(struct mpc *mpc, enum vpe_surface_pixel_format pixel_format,
        enum color_space color_space, enum mpc_output_csc_mode ocsc_mode);

    void (*program_output_csc)(struct mpc *mpc, enum vpe_surface_pixel_format pixel_format,
        enum color_space colorspace, uint16_t *matrix);

    void (*set_output_gamma)(struct mpc *mpc, const struct pwl_params *params);

    void (*set_gamut_remap)(struct mpc *mpc, struct colorspace_transform *gamut_remap);

    void (*power_on_1dlut_shaper_3dlut)(struct mpc *mpc, bool power_on);

    bool (*program_shaper)(struct mpc *mpc, const struct pwl_params *params);

    // using direct config to program the 3dlut specified in params
    void (*program_3dlut)(struct mpc *mpc, const struct tetrahedral_params *params);

    // using indirect config to configure the 3DLut
    bool (*program_3dlut_indirect)(struct mpc *mpc,
        struct vpe_buf *lut0_3_buf, // 3d lut buf which contains the data for lut0-lut3
        bool use_tetrahedral_9, bool use_12bits);

    // Blend-gamma control.
    void (*program_1dlut)(struct mpc *mpc, const struct pwl_params *params);

    void (*program_cm_location)(struct mpc *mpc, uint8_t location);

    void (*set_denorm)(struct mpc *mpc, int opp_id, enum color_depth output_depth,
        struct mpc_denorm_clamp *denorm_clamp);

    void (*set_out_float_en)(struct mpc *mpc, bool float_enable);

    void (*program_mpc_out)(struct mpc *mpc, enum vpe_surface_pixel_format format);

    void (*set_output_transfer_func)(struct mpc *mpc, struct output_ctx *output_ctx);

    void (*set_mpc_shaper_3dlut)(struct mpc *mpc, const struct transfer_func *func_shaper,
        const struct vpe_3dlut *lut3d_func);

    void (*set_blend_lut)(struct mpc *mpc, const struct transfer_func *blend_tf);

    bool (*program_movable_cm)(struct mpc *mpc, const struct transfer_func *func_shaper,
        const struct vpe_3dlut *lut3d_func, const struct transfer_func *blend_tf, bool afterblend);
    void (*program_crc)(struct mpc *mpc, bool enable);

};

struct mpc {
    struct vpe_priv  *vpe_priv;
    struct mpc_funcs *funcs;
    struct pwl_params regamma_params;
    struct pwl_params blender_params;
    struct pwl_params shaper_params;
};

const uint16_t *vpe_find_color_matrix(
    enum color_space color_space, enum vpe_surface_pixel_format pixel_format, uint32_t *array_size);

#ifdef __cplusplus
}
#endif
