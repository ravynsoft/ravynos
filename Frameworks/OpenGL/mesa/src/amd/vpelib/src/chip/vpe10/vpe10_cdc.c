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

#include "common.h"
#include "vpe_priv.h"
#include "vpe10_cdc.h"
#include "reg_helper.h"

#define CTX_BASE cdc
#define CTX      vpe10_cdc

enum mux_sel {
    MUX_SEL_ALPHA = 0,
    MUX_SEL_Y_G   = 1,
    MUX_SEL_CB_B  = 2,
    MUX_SEL_CR_R  = 3
};

static struct cdc_funcs cdc_func = {
    .check_input_format  = vpe10_cdc_check_input_format,
    .check_output_format = vpe10_cdc_check_output_format,

    .program_surface_config  = vpe10_cdc_program_surface_config,
    .program_crossbar_config = vpe10_cdc_program_crossbar_config,
    .program_global_sync     = vpe10_cdc_program_global_sync,
    .program_p2b_config      = vpe10_cdc_program_p2b_config,
    .program_viewport        = vpe10_cdc_program_viewport,
};

void vpe10_construct_cdc(struct vpe_priv *vpe_priv, struct cdc *cdc)
{
    cdc->vpe_priv = vpe_priv;
    cdc->funcs    = &cdc_func;
}

bool vpe10_cdc_check_input_format(struct cdc *cdc, enum vpe_surface_pixel_format format)
{
    if (vpe_is_32bit_packed_rgb(format))
        return true;

    if (format == VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr ||
        format == VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb)
        return true;

    if (format == VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr ||
        format == VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb)
        return true;

    return false;
}

bool vpe10_cdc_check_output_format(struct cdc *cdc, enum vpe_surface_pixel_format format)
{
    if (vpe_is_32bit_packed_rgb(format))
        return true;
    if (vpe_is_fp16(format))
        return true;

    return false;
}

void vpe10_cdc_program_surface_config(struct cdc *cdc, enum vpe_surface_pixel_format format,
    enum vpe_rotation_angle rotation, bool horizontal_mirror, enum vpe_swizzle_mode_values swizzle)
{
    uint32_t rotation_angle = 0, surface_linear;
    uint32_t surf_format    = 8;

    PROGRAM_ENTRY();

    /* Program rotation angle and horz mirror - no mirror */
    if (rotation == VPE_ROTATION_ANGLE_0)
        rotation_angle = 0;
    else if (rotation == VPE_ROTATION_ANGLE_90)
        rotation_angle = 1;
    else if (rotation == VPE_ROTATION_ANGLE_180)
        rotation_angle = 2;
    else if (rotation == VPE_ROTATION_ANGLE_270)
        rotation_angle = 3;

    if (swizzle == VPE_SW_LINEAR)
        surface_linear = 1;
    else
        surface_linear = 0;

    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB1555:
        surf_format = 1;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB565:
        surf_format = 3;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
        surf_format = 8;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
        surf_format = 9;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
        surf_format = 10;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
        surf_format = 11;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616:
        surf_format = 22;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F: /* use crossbar */
        surf_format = 24;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        surf_format = 25;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
        surf_format = 65;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
        surf_format = 64;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
        surf_format = 67;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
        surf_format = 66;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCrCb8888:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCbCr8888: // use crossbar
        surf_format = 12;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB111110_FIX:
        surf_format = 112;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGR101111_FIX:
        surf_format = 113;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_ACrYCb2101010:
        surf_format = 114;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB111110_FLOAT:
        surf_format = 118;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGR101111_FLOAT:
        surf_format = 119;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBE:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBE_ALPHA:
    default:
        vpe_log("cdc: invalid pixel format %d\n", (int)format);
        break;
    }

    REG_SET_4(VPCDC_FE0_SURFACE_CONFIG, 0, SURFACE_PIXEL_FORMAT_FE0, surf_format,
        ROTATION_ANGLE_FE0, rotation_angle, H_MIRROR_EN_FE0, (unsigned)horizontal_mirror,
        PIX_SURFACE_LINEAR_FE0, surface_linear);
}

void vpe10_cdc_program_crossbar_config(struct cdc *cdc, enum vpe_surface_pixel_format format)
{
    uint32_t alpha_bar = (uint32_t)MUX_SEL_ALPHA;
    uint32_t green_bar = (uint32_t)MUX_SEL_Y_G;
    uint32_t red_bar   = (uint32_t)MUX_SEL_CR_R;
    uint32_t blue_bar  = (uint32_t)MUX_SEL_CB_B;

    PROGRAM_ENTRY();

    if (format == VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888 ||
        format == VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888 ||
        format == VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010 ||
        format == VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F ||
        format == VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888 ||
        format == VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888 ||
        format == VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102 ||
        format == VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F ||
        format == VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCbCr8888) {
        red_bar  = MUX_SEL_CB_B;
        blue_bar = MUX_SEL_CR_R;
    }

    REG_SET_4(VPCDC_FE0_CROSSBAR_CONFIG, 0, CROSSBAR_SRC_ALPHA_FE0, alpha_bar,
        CROSSBAR_SRC_CR_R_FE0, red_bar, CROSSBAR_SRC_Y_G_FE0, green_bar, CROSSBAR_SRC_CB_B_FE0,
        blue_bar);
}

void vpe10_cdc_program_global_sync(
    struct cdc *cdc, uint32_t vupdate_offset, uint32_t vupdate_width, uint32_t vready_offset)
{
    PROGRAM_ENTRY();

    REG_SET_3(VPCDC_BE0_GLOBAL_SYNC_CONFIG, 0, BE0_VUPDATE_OFFSET, vupdate_offset,
        BE0_VUPDATE_WIDTH, vupdate_width, BE0_VREADY_OFFSET, vready_offset);
}

void vpe10_cdc_program_p2b_config(struct cdc *cdc, enum vpe_surface_pixel_format format)
{
    uint32_t bar_sel0       = (uint32_t)MUX_SEL_CB_B;
    uint32_t bar_sel1       = (uint32_t)MUX_SEL_Y_G;
    uint32_t bar_sel2       = (uint32_t)MUX_SEL_CR_R;
    uint32_t bar_sel3       = (uint32_t)MUX_SEL_ALPHA;
    uint32_t p2b_format_sel = 0;

    PROGRAM_ENTRY();

    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
        p2b_format_sel = 0;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
        p2b_format_sel = 1;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        p2b_format_sel = 2;
        break;
    default:
        VPE_ASSERT(0);
        break;
    }

    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
        bar_sel3 = (uint32_t)MUX_SEL_CR_R;
        bar_sel2 = (uint32_t)MUX_SEL_Y_G;
        bar_sel1 = (uint32_t)MUX_SEL_CB_B;
        bar_sel0 = (uint32_t)MUX_SEL_ALPHA;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
        bar_sel3 = (uint32_t)MUX_SEL_ALPHA;
        bar_sel2 = (uint32_t)MUX_SEL_CB_B;
        bar_sel1 = (uint32_t)MUX_SEL_Y_G;
        bar_sel0 = (uint32_t)MUX_SEL_CR_R;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        bar_sel3 = (uint32_t)MUX_SEL_CB_B;
        bar_sel2 = (uint32_t)MUX_SEL_Y_G;
        bar_sel1 = (uint32_t)MUX_SEL_CR_R;
        bar_sel0 = (uint32_t)MUX_SEL_ALPHA;
        break;
    default:
        break;
    }

    REG_SET_5(VPCDC_BE0_P2B_CONFIG, 0, VPCDC_BE0_P2B_XBAR_SEL0, bar_sel0, VPCDC_BE0_P2B_XBAR_SEL1,
        bar_sel1, VPCDC_BE0_P2B_XBAR_SEL2, bar_sel2, VPCDC_BE0_P2B_XBAR_SEL3, bar_sel3,
        VPCDC_BE0_P2B_FORMAT_SEL, p2b_format_sel);
}

/** segment specific */
void vpe10_cdc_program_viewport(
    struct cdc *cdc, const struct vpe_rect *viewport, const struct vpe_rect *viewport_c)
{

    PROGRAM_ENTRY();

    REG_SET_2(VPCDC_FE0_VIEWPORT_START_CONFIG, 0, VIEWPORT_X_START_FE0, viewport->x,
        VIEWPORT_Y_START_FE0, viewport->y);

    REG_SET_2(VPCDC_FE0_VIEWPORT_DIMENSION_CONFIG, 0, VIEWPORT_WIDTH_FE0, viewport->width,
        VIEWPORT_HEIGHT_FE0, viewport->height);

    REG_SET_2(VPCDC_FE0_VIEWPORT_START_C_CONFIG, 0, VIEWPORT_X_START_C_FE0, viewport_c->x,
        VIEWPORT_Y_START_C_FE0, viewport_c->y);

    REG_SET_2(VPCDC_FE0_VIEWPORT_DIMENSION_C_CONFIG, 0, VIEWPORT_WIDTH_C_FE0, viewport_c->width,
        VIEWPORT_HEIGHT_C_FE0, viewport_c->height);
}

