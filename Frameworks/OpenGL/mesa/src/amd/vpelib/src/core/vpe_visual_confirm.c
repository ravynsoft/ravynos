/* Copyright 2023 Advanced Micro Devices, Inc.
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

#include "vpe_visual_confirm.h"
#include "common.h"
#include "vpe_priv.h"
#include "color_bg.h"
#include "background.h"
#include "resource.h"

static uint16_t get_visual_confirm_segs_count(uint32_t max_seg_width, uint32_t target_rect_width)
{
    // Unlike max_gaps logic in vpe10_calculate_segments, we are pure BG seg, no need to worry
    // stream splitted among one of the segment. so no need to "+1", just round up the calculated
    // number of segments.
    uint16_t seg_cnt = (uint16_t)(max((target_rect_width + max_seg_width - 1) / max_seg_width, 1));

    return seg_cnt;
}

static uint16_t vpe_get_visual_confirm_total_seg_count(
    struct vpe_priv *vpe_priv, uint32_t max_seg_width, const struct vpe_build_param *params)
{
    uint16_t           segs_num                  = 0;
    uint16_t           total_visual_confirm_segs = 0;
    uint16_t           stream_idx;
    struct stream_ctx *stream_ctx;

    if (vpe_priv->init.debug.visual_confirm_params.input_format) {
        for (stream_idx = 0; stream_idx < params->num_streams; stream_idx++) {
            stream_ctx = &vpe_priv->stream_ctx[stream_idx];
            total_visual_confirm_segs += get_visual_confirm_segs_count(
                max_seg_width, stream_ctx->stream.scaling_info.dst_rect.width);
        }
    }

    if (vpe_priv->init.debug.visual_confirm_params.output_format) {
        total_visual_confirm_segs +=
            get_visual_confirm_segs_count(max_seg_width, params->target_rect.width);
    }

    return total_visual_confirm_segs;
}

struct vpe_color vpe_get_visual_confirm_color(enum vpe_surface_pixel_format format,
    struct vpe_color_space cs, enum color_space output_cs, struct transfer_func *output_tf,
    bool enable_3dlut)
{
    struct vpe_color visual_confirm_color;
    visual_confirm_color.is_ycbcr = false;
    visual_confirm_color.rgba.a   = 0.0;
    visual_confirm_color.rgba.r   = 0.0;
    visual_confirm_color.rgba.g   = 0.0;
    visual_confirm_color.rgba.b   = 0.0;

    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
        // YUV420 8bit: Green
        visual_confirm_color.rgba.r = 0.0;
        visual_confirm_color.rgba.g = 1.0;
        visual_confirm_color.rgba.b = 0.0;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
        // YUV420 10bit: yellow (SDR)
        switch (cs.tf) {
        case VPE_TF_G22:
        case VPE_TF_G24:
            visual_confirm_color.rgba.r = 1.0;
            visual_confirm_color.rgba.g = 1.0;
            visual_confirm_color.rgba.b = 0.0;
            break;
            // YUV420 10bit: White (HDR)
        case VPE_TF_PQ:
        case VPE_TF_HLG:
            if (enable_3dlut) {
                visual_confirm_color.rgba.r = 1.0;
                visual_confirm_color.rgba.g = 1.0;
                visual_confirm_color.rgba.b = 1.0;
            } else {
                visual_confirm_color.rgba.r = 1.0;
                visual_confirm_color.rgba.g = 0.0;
                visual_confirm_color.rgba.b = 0.0;
            }
            break;
        default:
            break;
        }
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
        // RGBA and RGBX 8 bit and variants : Pink
        visual_confirm_color.rgba.r = 1.0;
        visual_confirm_color.rgba.g = 0.5;
        visual_confirm_color.rgba.b = 1.0;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
        // RGBA 10 bit and variants : Cyan
        visual_confirm_color.rgba.r = 0.0;
        visual_confirm_color.rgba.g = 1.0;
        visual_confirm_color.rgba.b = 1.0;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        // FP16 and variants: orange
        visual_confirm_color.rgba.r = 1.0;
        visual_confirm_color.rgba.g = 0.21972f;
        visual_confirm_color.rgba.b = 0.0;
        break;
    default:
        break;
    }

    // Due to there will be regamma (ogam), need convert the bg color for visual confirm
    vpe_bg_color_convert(output_cs, output_tf, &visual_confirm_color);

    // Experimental: To make FP16 Linear color looks more visually ok
    if (output_tf->tf == TRANSFER_FUNC_LINEAR_0_125) {
        visual_confirm_color.rgba.r /= 125;
        visual_confirm_color.rgba.g /= 125;
        visual_confirm_color.rgba.b /= 125;
    }

    return visual_confirm_color;
}

enum vpe_status vpe_create_visual_confirm_segs(
    struct vpe_priv *vpe_priv, const struct vpe_build_param *params, uint32_t max_seg_width)
{
    uint16_t           stream_idx;
    struct stream_ctx *stream_ctx;
    struct vpe_rect    visual_confirm_rect;
    struct vpe_rect   *visual_confirm_gaps;
    struct vpe_rect   *current_gap;

    uint16_t total_seg_cnt =
        vpe_get_visual_confirm_total_seg_count(vpe_priv, max_seg_width, params);
    uint16_t seg_cnt = 0;

    if (!total_seg_cnt)
        return VPE_STATUS_OK;

    visual_confirm_gaps = vpe_zalloc(sizeof(struct vpe_rect) * total_seg_cnt);
    if (!visual_confirm_gaps)
        return VPE_STATUS_NO_MEMORY;

    current_gap = visual_confirm_gaps;

    // Do visual confirm bg generation for intput format
    if (vpe_priv->init.debug.visual_confirm_params.input_format &&
        params->target_rect.height > 2 * VISUAL_CONFIRM_HEIGHT) {
        for (stream_idx = 0; stream_idx < params->num_streams; stream_idx++) {
            stream_ctx          = &vpe_priv->stream_ctx[stream_idx];
            visual_confirm_rect = stream_ctx->stream.scaling_info.dst_rect;
            visual_confirm_rect.y += 0;
            visual_confirm_rect.height = VISUAL_CONFIRM_HEIGHT;
            seg_cnt                    = get_visual_confirm_segs_count(
                max_seg_width, stream_ctx->stream.scaling_info.dst_rect.width);
            vpe_full_bg_gaps(current_gap, &visual_confirm_rect, seg_cnt);
            vpe_priv->resource.create_bg_segments(
                vpe_priv, current_gap, seg_cnt, VPE_CMD_OPS_BG_VSCF_INPUT);
            current_gap += seg_cnt;
        }
    }
    // Do visual confirm bg generation for output format
    if (vpe_priv->init.debug.visual_confirm_params.output_format &&
        params->target_rect.height > VISUAL_CONFIRM_HEIGHT) {
        visual_confirm_rect = params->target_rect;
        visual_confirm_rect.y += VISUAL_CONFIRM_HEIGHT;
        visual_confirm_rect.height = VISUAL_CONFIRM_HEIGHT;
        seg_cnt = get_visual_confirm_segs_count(max_seg_width, params->target_rect.width);
        vpe_full_bg_gaps(current_gap, &visual_confirm_rect, seg_cnt);
        vpe_priv->resource.create_bg_segments(
            vpe_priv, current_gap, seg_cnt, VPE_CMD_OPS_BG_VSCF_OUTPUT);
    }

    if (visual_confirm_gaps != NULL) {
        vpe_free(visual_confirm_gaps);
        visual_confirm_gaps = NULL;
        current_gap         = NULL;
    }

    return VPE_STATUS_OK;
}
