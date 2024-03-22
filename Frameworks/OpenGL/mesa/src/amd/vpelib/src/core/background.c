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

#include "background.h"
#include "common.h"
#include "vpe_priv.h"
#include "color_bg.h"

void vpe_create_bg_segments(
    struct vpe_priv *vpe_priv, struct vpe_rect *gaps, uint16_t gaps_cnt, enum vpe_cmd_ops ops)
{
    uint16_t            gap_index;
    struct scaler_data *scaler_data;
    struct stream_ctx  *stream_ctx = &(vpe_priv->stream_ctx[0]);
    int32_t             vp_x       = stream_ctx->stream.scaling_info.src_rect.x;
    int32_t             vp_y       = stream_ctx->stream.scaling_info.src_rect.y;
    uint16_t            src_div    = vpe_is_yuv420(stream_ctx->stream.surface_info.format) ? 2 : 1;
    uint16_t            dst_div    = vpe_is_yuv420(vpe_priv->output_ctx.surface.format) ? 2 : 1;

    for (gap_index = 0; gap_index < gaps_cnt; gap_index++) {

        scaler_data = &(vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].inputs[0].scaler_data);

        /* format */
        scaler_data->format             = stream_ctx->stream.surface_info.format;
        scaler_data->lb_params.alpha_en = stream_ctx->per_pixel_alpha;

        /* recout */

        scaler_data->recout.x      = 0;
        scaler_data->recout.y      = 0;
        scaler_data->recout.height = VPE_MIN_VIEWPORT_SIZE;
        scaler_data->recout.width  = VPE_MIN_VIEWPORT_SIZE;

        /* ratios */
        scaler_data->ratios.horz = vpe_fixpt_one;
        scaler_data->ratios.vert = vpe_fixpt_one;

        if (vpe_is_yuv420(scaler_data->format)) {
            scaler_data->ratios.horz_c = vpe_fixpt_from_fraction(1, 2);
            scaler_data->ratios.vert_c = vpe_fixpt_from_fraction(1, 2);
        } else {
            scaler_data->ratios.horz_c = vpe_fixpt_one;
            scaler_data->ratios.vert_c = vpe_fixpt_one;
        }

        /* Active region */
        scaler_data->h_active = gaps[gap_index].width;
        scaler_data->v_active = gaps[gap_index].height;

        /* viewport */

        scaler_data->viewport.x      = vp_x;
        scaler_data->viewport.y      = vp_y;
        scaler_data->viewport.width  = VPE_MIN_VIEWPORT_SIZE;
        scaler_data->viewport.height = VPE_MIN_VIEWPORT_SIZE;

        scaler_data->viewport_c.x      = scaler_data->viewport.x / src_div;
        scaler_data->viewport_c.y      = scaler_data->viewport.y / src_div;
        scaler_data->viewport_c.width  = scaler_data->viewport.width / src_div;
        scaler_data->viewport_c.height = scaler_data->viewport.height / src_div;

        /* destination viewport */
        scaler_data->dst_viewport = gaps[gap_index];

        scaler_data->dst_viewport_c.x      = scaler_data->dst_viewport.x / dst_div;
        scaler_data->dst_viewport_c.y      = scaler_data->dst_viewport.y / dst_div;
        scaler_data->dst_viewport_c.width  = scaler_data->dst_viewport.width / dst_div;
        scaler_data->dst_viewport_c.height = scaler_data->dst_viewport.height / dst_div;

        /* taps and inits */
        scaler_data->taps.h_taps = scaler_data->taps.v_taps = 4;
        scaler_data->taps.h_taps_c = scaler_data->taps.v_taps_c = 2;

        scaler_data->inits.h = vpe_fixpt_div_int(
            vpe_fixpt_add_int(scaler_data->ratios.horz, (int)(scaler_data->taps.h_taps + 1)), 2);
        scaler_data->inits.v = vpe_fixpt_div_int(
            vpe_fixpt_add_int(scaler_data->ratios.vert, (int)(scaler_data->taps.v_taps + 1)), 2);
        scaler_data->inits.h_c = vpe_fixpt_div_int(
            vpe_fixpt_add_int(scaler_data->ratios.horz_c, (int)(scaler_data->taps.h_taps_c + 1)),
            2);
        scaler_data->inits.v_c = vpe_fixpt_div_int(
            vpe_fixpt_add_int(scaler_data->ratios.vert_c, (int)(scaler_data->taps.v_taps_c + 1)),
            2);

        VPE_ASSERT(gaps_cnt - gap_index - 1 <= (uint16_t)0xF);

        // background takes stream_idx 0 as its input
        vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].inputs[0].stream_idx = 0;
        vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].dst_viewport   = scaler_data->dst_viewport;
        vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].dst_viewport_c = scaler_data->dst_viewport_c;
        vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].num_inputs     = 1;
        vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].ops            = ops;
        vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].cd = (uint8_t)(gaps_cnt - gap_index - 1);
        vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].tm_enabled =
            false; // currently only support frontend tm

        if (vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].cd == (gaps_cnt - 1)) {
            vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].is_begin = true;
        }

        if (vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].cd == 0) {
            vpe_priv->vpe_cmd_info[vpe_priv->num_vpe_cmds].is_end = true;
        }

        vpe_priv->num_vpe_cmds++;
    }
}

void vpe_full_bg_gaps(struct vpe_rect *gaps, const struct vpe_rect *target_rect, uint16_t max_gaps)
{
    uint16_t gap_index;
    int32_t  last_covered;
    uint32_t gap_width, gap_remainder;

    last_covered  = target_rect->x;
    gap_width     = target_rect->width / max_gaps;
    gap_remainder = target_rect->width % max_gaps;

    for (gap_index = 0; gap_index < max_gaps; gap_index++) {
        gaps[gap_index].x     = last_covered;
        gaps[gap_index].y     = target_rect->y;
        gaps[gap_index].width = gap_width;
        if (gap_index >= max_gaps - gap_remainder) {
            gaps[gap_index].width += 1;
        }
        gaps[gap_index].height = target_rect->height;
        last_covered           = last_covered + (int32_t)gaps[gap_index].width;
    }
}

/* calculates the gaps in target_rect which are not covered by the first stream
   and returns the number of gaps */
uint16_t vpe_find_bg_gaps(struct vpe_priv *vpe_priv, const struct vpe_rect *target_rect,
    struct vpe_rect *gaps, uint16_t max_gaps)
{
    uint16_t         num_gaps = 0;
    uint16_t         num_segs;
    struct vpe_rect *dst_viewport_rect;
    bool             full_bg       = false;
    const uint32_t   max_seg_width = vpe_priv->pub.caps->plane_caps.max_viewport_width;
    const uint16_t num_multiple = 1;

    num_segs          = vpe_priv->stream_ctx[0].num_segments;
    dst_viewport_rect = &(vpe_priv->stream_ctx[0].segment_ctx[0].scaler_data.dst_viewport);

    if (target_rect->x < dst_viewport_rect->x) {

        if (target_rect->width <= max_seg_width) {
            goto full_bg;
        }
        gaps[0].x      = target_rect->x;
        gaps[0].y      = target_rect->y;
        gaps[0].width  = (uint32_t)(dst_viewport_rect->x - target_rect->x);
        gaps[0].height = target_rect->height;
        num_gaps++;
        if (gaps[0].width > max_seg_width) {
            if (!vpe_priv->resource.split_bg_gap(
                    gaps, target_rect, max_seg_width, max_gaps, &num_gaps, num_multiple)) {
                goto full_bg;
            }
        }
    }
    dst_viewport_rect =
        &(vpe_priv->stream_ctx[0].segment_ctx[num_segs - 1].scaler_data.dst_viewport);

    if (target_rect->x + (int32_t)target_rect->width >
        dst_viewport_rect->x + (int32_t)dst_viewport_rect->width) {

        if (num_gaps == max_gaps) {
            goto full_bg;
        }

        gaps[num_gaps].x = dst_viewport_rect->x + (int32_t)dst_viewport_rect->width;
        gaps[num_gaps].y = target_rect->y;
        gaps[num_gaps].width =
            (uint32_t)(target_rect->x + (int32_t)target_rect->width -
                       (dst_viewport_rect->x + (int32_t)dst_viewport_rect->width));
        gaps[num_gaps].height = target_rect->height;
        num_gaps++;
        if (gaps[num_gaps - 1].width > max_seg_width) {
            if (!vpe_priv->resource.split_bg_gap(
                    gaps, target_rect, max_seg_width, max_gaps, &num_gaps, num_multiple)) {
                goto full_bg;
            }
        }
    }
    return num_gaps;

full_bg:
    vpe_full_bg_gaps(gaps, target_rect, max_gaps);
    return max_gaps;
}
