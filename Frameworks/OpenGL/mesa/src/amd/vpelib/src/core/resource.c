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
#include <math.h>
#include "vpe_types.h"
#include "vpe_priv.h"
#include "vpe_version.h"
#include "common.h"

#ifdef VPE_BUILD_1_0
#include "vpe10_resource.h"
#endif

static const struct vpe_debug_options debug_defaults = {
    .flags                   = {0},
    .cm_in_bypass            = 0,
    .vpcnvc_bypass           = 0,
    .mpc_bypass              = 0,
    .identity_3dlut          = 0,
    .sce_3dlut               = 0,
    .disable_reuse_bit       = 0,
    .bg_bit_depth            = 0,
    .bypass_gamcor           = 0,
    .bypass_ogam             = 0,
    .bypass_dpp_gamut_remap  = 0,
    .bypass_post_csc         = 0,
    .bg_color_fill_only      = 0,
    .assert_when_not_support = 0,
    .enable_mem_low_power =
        {
            .bits =
                {
                    .cm   = false,
                    .dscl = false,
                    .mpc  = false,
                },
        },
    .force_tf_calculation                    = 1,
    .expansion_mode                          = 1,
    .clamping_setting                        = 1,
    .clamping_params =
    	{
            .r_clamp_component_lower         = 0x1000,
            .g_clamp_component_lower         = 0x1000,
            .b_clamp_component_lower         = 0x1000,
            .r_clamp_component_upper         = 0xEB00,
            .g_clamp_component_upper         = 0xEB00,
            .b_clamp_component_upper         = 0xEB00,
            .clamping_range                  = 4,
	},
    .bypass_per_pixel_alpha                  = 0,
    .opp_pipe_crc_ctrl                       = 0,
    .dpp_crc_ctrl                            = 0,
    .mpc_crc_ctrl                            = 0,
    .visual_confirm_params                   = {{{0}}},
};

enum vpe_ip_level vpe_resource_parse_ip_version(
    uint8_t mj, uint8_t mn, uint8_t rv)
{
    enum vpe_ip_level ip_level = VPE_IP_LEVEL_UNKNOWN;
    switch (VPE_VERSION(mj, mn, rv)) {
#if VPE_BUILD_1_X
#if VPE_BUILD_1_0
    case VPE_VERSION(6, 1, 0):
        ip_level = VPE_IP_LEVEL_1_0;
#endif
        break;
#endif
    default:
        ip_level = VPE_IP_LEVEL_UNKNOWN;
        break;
    }
    return ip_level;
}

enum vpe_status vpe_construct_resource(
    struct vpe_priv *vpe_priv, enum vpe_ip_level level, struct resource *res)
{
    enum vpe_status status = VPE_STATUS_OK;
    switch (level) {
#ifdef VPE_BUILD_1_0
    case VPE_IP_LEVEL_1_0:
        status = vpe10_construct_resource(vpe_priv, res);
        break;
#endif
    default:
        status = VPE_STATUS_NOT_SUPPORTED;
        vpe_log("invalid ip level: %d", (int)level);
        break;
    }

    vpe_priv->init.debug     = debug_defaults;
    vpe_priv->expansion_mode = vpe_priv->init.debug.expansion_mode;
    if (res)
        res->vpe_priv = vpe_priv;

    return status;
}

void vpe_destroy_resource(struct vpe_priv *vpe_priv, struct resource *res)
{
    switch (vpe_priv->pub.level) {
#ifdef VPE_BUILD_1_0
    case VPE_IP_LEVEL_1_0:
        vpe10_destroy_resource(vpe_priv, res);
        break;
#endif
    default:
        break;
    }
}

struct segment_ctx *vpe_alloc_segment_ctx(struct vpe_priv *vpe_priv, uint16_t num_segments)
{
    struct segment_ctx *segment_ctx_base;

    segment_ctx_base = (struct segment_ctx *)vpe_zalloc(sizeof(struct segment_ctx) * num_segments);

    if (!segment_ctx_base)
        return NULL;

    return segment_ctx_base;
}

struct stream_ctx *vpe_alloc_stream_ctx(struct vpe_priv *vpe_priv, uint32_t num_streams)
{
    struct stream_ctx *ctx_base, *ctx;
    uint32_t           i;

    ctx_base = (struct stream_ctx *)vpe_zalloc(sizeof(struct stream_ctx) * num_streams);
    if (!ctx_base)
        return NULL;

    for (i = 0; i < num_streams; i++) {
        ctx           = &ctx_base[i];
        ctx->cs       = COLOR_SPACE_UNKNOWN;
        ctx->tf       = TRANSFER_FUNC_UNKNOWN;
        ctx->vpe_priv = vpe_priv;
        vpe_color_set_adjustments_to_default(&ctx->color_adjustments);
        ctx->tf_scaling_factor = vpe_fixpt_one;
    }

    return ctx_base;
}

void vpe_free_stream_ctx(struct vpe_priv *vpe_priv)
{
    uint16_t           i;
    struct stream_ctx *ctx;

    if (!vpe_priv->stream_ctx || !vpe_priv->num_streams)
        return;

    for (i = 0; i < vpe_priv->num_streams; i++) {
        ctx = &vpe_priv->stream_ctx[i];
        if (ctx->input_tf) {
            vpe_free(ctx->input_tf);
            ctx->input_tf = NULL;
        }

        if (ctx->bias_scale) {
            vpe_free(ctx->bias_scale);
            ctx->bias_scale = NULL;
        }

        if (ctx->input_cs) {
            vpe_free(ctx->input_cs);
            ctx->input_cs = NULL;
        }

        if (ctx->gamut_remap) {
            vpe_free(ctx->gamut_remap);
            ctx->gamut_remap = NULL;
        }

        if (ctx->in_shaper_func) {
            vpe_free(ctx->in_shaper_func);
            ctx->in_shaper_func = NULL;
        }

        if (ctx->blend_tf) {
            vpe_free(ctx->blend_tf);
            ctx->blend_tf = NULL;
        }

        if (ctx->lut3d_func) {
            vpe_free(ctx->lut3d_func);
            ctx->lut3d_func = NULL;
        }

        if (ctx->segment_ctx) {
            vpe_free(ctx->segment_ctx);
            ctx->segment_ctx = NULL;
        }
    }
    vpe_free(vpe_priv->stream_ctx);
    vpe_priv->stream_ctx  = NULL;
    vpe_priv->num_streams = 0;
}

void vpe_free_output_ctx(struct vpe_priv *vpe_priv)
{
    if (vpe_priv->output_ctx.gamut_remap)
        vpe_free(vpe_priv->output_ctx.gamut_remap);

    if (vpe_priv->output_ctx.output_tf)
        vpe_free(vpe_priv->output_ctx.output_tf);
}

void vpe_pipe_reset(struct vpe_priv *vpe_priv)
{
    int              i;
    struct pipe_ctx *pipe_ctx;

    for (i = 0; i < vpe_priv->num_pipe; i++) {
        pipe_ctx               = &vpe_priv->pipe_ctx[i];
        pipe_ctx->is_top_pipe  = true;
        pipe_ctx->owner        = PIPE_CTX_NO_OWNER;
        pipe_ctx->top_pipe_idx = 0xff;
    }
}

void vpe_pipe_reclaim(struct vpe_priv *vpe_priv, struct vpe_cmd_info *cmd_info)
{
    int              i, j;
    struct pipe_ctx *pipe_ctx;

    for (i = 0; i < vpe_priv->num_pipe; i++) {
        pipe_ctx = &vpe_priv->pipe_ctx[i];
        if (pipe_ctx->owner != PIPE_CTX_NO_OWNER) {
            for (j = 0; j < cmd_info->num_inputs; j++)
                if (pipe_ctx->owner == cmd_info->inputs[j].stream_idx)
                    break;

            if (j == cmd_info->num_inputs) {
                // that stream no longer exists
                pipe_ctx->is_top_pipe  = true;
                pipe_ctx->owner        = PIPE_CTX_NO_OWNER;
                pipe_ctx->top_pipe_idx = 0xff;
            }
        }
    }
}

struct pipe_ctx *vpe_pipe_find_owner(struct vpe_priv *vpe_priv, uint32_t stream_idx, bool *reuse)
{
    int              i;
    struct pipe_ctx *pipe_ctx;
    struct pipe_ctx *free_pipe = NULL;

    for (i = 0; i < vpe_priv->num_pipe; i++) {
        pipe_ctx = &vpe_priv->pipe_ctx[i];

        if (!free_pipe && (pipe_ctx->owner == PIPE_CTX_NO_OWNER))
            free_pipe = pipe_ctx;
        // re-use the same pipe
        else if (pipe_ctx->owner == stream_idx) {
            *reuse = true;
            return pipe_ctx;
        }
    }

    if (free_pipe) {
        free_pipe->owner = stream_idx;
    }
    *reuse = false;
    return free_pipe;
}

static void calculate_recout(struct segment_ctx *segment)
{
    struct stream_ctx  *stream_ctx = segment->stream_ctx;
    struct scaler_data *data       = &segment->scaler_data;
    struct vpe_rect    *dst_rect;
    int32_t             split_count, split_idx;

    dst_rect = &stream_ctx->stream.scaling_info.dst_rect;

    split_count = stream_ctx->num_segments - 1;
    split_idx   = segment->segment_idx;

    // src & dst rect has been clipped earlier
    data->recout.x      = 0;
    data->recout.y      = 0;
    data->recout.width  = dst_rect->width;
    data->recout.height = dst_rect->height;

    if (split_count) {
        /* extra pixels in the division remainder need to go to pipes after
         * the extra pixel index minus one(epimo) defined here as:
         */
        int32_t epimo = split_count - (int32_t)data->recout.width % (split_count + 1);

        data->recout.x += ((int32_t)data->recout.width / (split_count + 1)) * split_idx;
        if (split_idx > epimo)
            data->recout.x += split_idx - epimo - 1;

        data->recout.width =
            data->recout.width / (uint32_t)(split_count + 1) + (split_idx > epimo ? 1 : 0);
    }
}

void calculate_scaling_ratios(struct scaler_data *scl_data, struct vpe_rect *src_rect,
    struct vpe_rect *dst_rect, enum vpe_surface_pixel_format format)
{
    // no rotation support

    scl_data->ratios.horz   = vpe_fixpt_from_fraction(src_rect->width, dst_rect->width);
    scl_data->ratios.vert   = vpe_fixpt_from_fraction(src_rect->height, dst_rect->height);
    scl_data->ratios.horz_c = scl_data->ratios.horz;
    scl_data->ratios.vert_c = scl_data->ratios.vert;

    if (vpe_is_yuv420(format)) {
        scl_data->ratios.horz_c.value /= 2;
        scl_data->ratios.vert_c.value /= 2;
    }

    scl_data->ratios.horz   = vpe_fixpt_truncate(scl_data->ratios.horz, 19);
    scl_data->ratios.vert   = vpe_fixpt_truncate(scl_data->ratios.vert, 19);
    scl_data->ratios.horz_c = vpe_fixpt_truncate(scl_data->ratios.horz_c, 19);
    scl_data->ratios.vert_c = vpe_fixpt_truncate(scl_data->ratios.vert_c, 19);
}

/*
 * This is a preliminary vp size calculation to allow us to check taps support.
 * The result is completely overridden afterwards.
 */
static void calculate_viewport_size(struct segment_ctx *segment_ctx)
{
    struct scaler_data *data = &segment_ctx->scaler_data;

    data->viewport.width =
        (uint32_t)vpe_fixpt_ceil(vpe_fixpt_mul_int(data->ratios.horz, (int)data->recout.width));
    data->viewport.height =
        (uint32_t)vpe_fixpt_ceil(vpe_fixpt_mul_int(data->ratios.vert, (int)data->recout.height));
    data->viewport_c.width =
        (uint32_t)vpe_fixpt_ceil(vpe_fixpt_mul_int(data->ratios.horz_c, (int)data->recout.width));
    data->viewport_c.height =
        (uint32_t)vpe_fixpt_ceil(vpe_fixpt_mul_int(data->ratios.vert_c, (int)data->recout.height));
}

/*
 * We completely calculate vp offset, size and inits here based entirely on scaling
 * ratios and recout for pixel perfect pipe combine.
 */
static void calculate_init_and_vp(bool flip_scan_dir, int32_t recout_offset, uint32_t recout_size,
    uint32_t src_size, uint32_t taps, struct fixed31_32 ratio, struct fixed31_32 init_adj,
    struct fixed31_32 *init, int32_t *vp_offset, uint32_t *vp_size)
{

    struct fixed31_32 src_offset, temp;
    int32_t           int_part;

    /*
     * First of the taps starts sampling pixel number <init_int_part> corresponding to recout
     * pixel 1. Next recout pixel samples int part of <init + scaling ratio> and so on.
     * All following calculations are based on this logic.
     */
    src_offset = vpe_fixpt_mul_int(ratio, recout_offset);
    *vp_offset = vpe_fixpt_floor(src_offset);

    // calculate the phase
    init->value = src_offset.value & 0xffffffff; // for phase accumulation
    *init       = vpe_fixpt_add(*init, init_adj);
    int_part    = vpe_fixpt_floor(vpe_fixpt_from_fraction(taps, 2)) +
               1; // middle point of the sampling window
    *init = vpe_fixpt_add_int(*init, int_part);
    *init = vpe_fixpt_truncate(*init, 19);
    /*
     * If there are more pixels on the left hand side (top for vertical scaling) of the
     * sampling point which can be covered by the taps, init value needs go get increased
     * to be able to buffer the pixels as much as taps.
     */
    if (int_part < (int32_t)taps) {
        int32_t left = (int32_t)taps - int_part;
        if (left > *vp_offset)
            left = *vp_offset;
        *vp_offset -= left;
        *init = vpe_fixpt_add_int(*init, left);
    }
    /*
     * If taps are sampling outside of viewport at end of recout and there are more pixels
     * available in the surface we should increase the viewport size, regardless set vp to
     * only what is used.
     */
    temp     = vpe_fixpt_add(*init, vpe_fixpt_mul_int(ratio, (int)(recout_size - 1)));
    *vp_size = (uint32_t)vpe_fixpt_floor(temp);
    if ((uint32_t)((int32_t)*vp_size + *vp_offset) > src_size)
        *vp_size = (uint32_t)((int32_t)src_size - *vp_offset);
    /* We did all the math assuming we are scanning same direction as display does,
     * however mirror/rotation changes how vp scans vs how it is offset. If scan direction
     * is flipped we simply need to calculate offset from the other side of plane.
     * Note that outside of viewport all scaling hardware works in recout space.
     */
    if (flip_scan_dir)
        *vp_offset = (int32_t)src_size - *vp_offset - (int32_t)*vp_size;
}

static inline void get_vp_scan_direction(enum vpe_rotation_angle rotation, bool horizontal_mirror,
    bool *orthogonal_rotation, bool *flip_vert_scan_dir, bool *flip_horz_scan_dir)
{
    *orthogonal_rotation = false;
    *flip_vert_scan_dir  = false;
    *flip_horz_scan_dir  = false;
    if (rotation == VPE_ROTATION_ANGLE_180) {
        *flip_vert_scan_dir = true;
        *flip_horz_scan_dir = true;
    } else if (rotation == VPE_ROTATION_ANGLE_90) {
        *orthogonal_rotation = true;
        *flip_horz_scan_dir  = true;
    } else if (rotation == VPE_ROTATION_ANGLE_270) {
        *orthogonal_rotation = true;
        *flip_vert_scan_dir  = true;
    }

    if (horizontal_mirror)
        *flip_horz_scan_dir = !*flip_horz_scan_dir;
}

static enum vpe_status calculate_inits_and_viewports(struct segment_ctx *segment_ctx)
{
    struct stream_ctx       *stream_ctx   = segment_ctx->stream_ctx;
    struct vpe_surface_info *surface_info = &stream_ctx->stream.surface_info;
    struct vpe_rect          src_rect     = stream_ctx->stream.scaling_info.src_rect;
    struct vpe_rect         *dst_rect     = &stream_ctx->stream.scaling_info.dst_rect;
    struct scaler_data      *data         = &segment_ctx->scaler_data;
    uint32_t                 vpc_div      = vpe_is_yuv420(data->format) ? 2 : 1;
    bool                     orthogonal_rotation, flip_vert_scan_dir, flip_horz_scan_dir;
    struct fixed31_32        init_adj_h = vpe_fixpt_zero;
    struct fixed31_32        init_adj_v = vpe_fixpt_zero;

    get_vp_scan_direction(stream_ctx->stream.rotation, stream_ctx->stream.horizontal_mirror,
        &orthogonal_rotation, &flip_vert_scan_dir, &flip_horz_scan_dir);

    if (orthogonal_rotation) {
        swap(src_rect.width, src_rect.height);
        swap(flip_vert_scan_dir, flip_horz_scan_dir);
    }

    if (flip_horz_scan_dir) {
        if (stream_ctx->flip_horizonal_output)
            // flip at the output instead
            flip_horz_scan_dir = false;
    }

    if (vpe_is_yuv420(data->format)) {
        int sign = -1; // this gives the direction of the cositing (negative will move left, right
                       // otherwise)
        switch (surface_info->cs.cositing) {

        case VPE_CHROMA_COSITING_LEFT:
            init_adj_h = vpe_fixpt_zero;
            init_adj_v = vpe_fixpt_from_fraction(sign, 4);
            break;
        case VPE_CHROMA_COSITING_NONE:
            init_adj_h = vpe_fixpt_from_fraction(sign, 4);
            init_adj_v = vpe_fixpt_from_fraction(sign, 4);
            break;
        case VPE_CHROMA_COSITING_TOPLEFT:
        default:
            init_adj_h = vpe_fixpt_zero;
            init_adj_v = vpe_fixpt_zero;
            break;
        }
    }

    calculate_init_and_vp(flip_horz_scan_dir, data->recout.x, data->recout.width, src_rect.width,
        data->taps.h_taps, data->ratios.horz, vpe_fixpt_zero, &data->inits.h, &data->viewport.x,
        &data->viewport.width);
    calculate_init_and_vp(flip_horz_scan_dir, data->recout.x, data->recout.width,
        src_rect.width / vpc_div, data->taps.h_taps_c, data->ratios.horz_c, init_adj_h,
        &data->inits.h_c, &data->viewport_c.x, &data->viewport_c.width);
    calculate_init_and_vp(flip_vert_scan_dir, data->recout.y, data->recout.height, src_rect.height,
        data->taps.v_taps, data->ratios.vert, vpe_fixpt_zero, &data->inits.v, &data->viewport.y,
        &data->viewport.height);
    calculate_init_and_vp(flip_vert_scan_dir, data->recout.y, data->recout.height,
        src_rect.height / vpc_div, data->taps.v_taps_c, data->ratios.vert_c, init_adj_v,
        &data->inits.v_c, &data->viewport_c.y, &data->viewport_c.height);

    // convert to absolute address
    data->viewport.x += src_rect.x;
    data->viewport.y += src_rect.y;
    data->viewport_c.x += src_rect.x / (int32_t)vpc_div;
    data->viewport_c.y += src_rect.y / (int32_t)vpc_div;

    return VPE_STATUS_OK;
}

uint16_t vpe_get_num_segments(struct vpe_priv *vpe_priv, const struct vpe_rect *src,
    const struct vpe_rect *dst, const uint32_t max_seg_width)
{
    int num_seg_src = (int)(ceil((double)src->width / max_seg_width));
    int num_seg_dst = (int)(ceil((double)dst->width / max_seg_width));
    return (uint16_t)(max(max(num_seg_src, num_seg_dst), 1));
}

void vpe_clip_stream(
    struct vpe_rect *src_rect, struct vpe_rect *dst_rect, const struct vpe_rect *target_rect)
{
    struct fixed31_32 scaling_ratio_h;
    struct fixed31_32 scaling_ratio_v;

    struct vpe_rect clipped_dst_rect, clipped_src_rect;
    uint32_t        clipped_pixels;

    clipped_dst_rect = *dst_rect;
    clipped_src_rect = *src_rect;

    scaling_ratio_h = vpe_fixpt_from_fraction(src_rect->width, dst_rect->width);
    scaling_ratio_v = vpe_fixpt_from_fraction(src_rect->height, dst_rect->height);

    if (dst_rect->x < target_rect->x) {
        clipped_pixels     = (uint32_t)(target_rect->x - dst_rect->x);
        clipped_dst_rect.x = target_rect->x;
        clipped_dst_rect.width -= clipped_pixels;
        clipped_pixels = (uint32_t)vpe_fixpt_round(
            vpe_fixpt_mul_int(scaling_ratio_h, (int)(target_rect->x - dst_rect->x)));
        clipped_src_rect.x += (int32_t)clipped_pixels;
        clipped_src_rect.width -= clipped_pixels;
    }
    if (dst_rect->y < target_rect->y) {
        clipped_pixels     = (uint32_t)(target_rect->y - dst_rect->y);
        clipped_dst_rect.y = target_rect->y;
        clipped_dst_rect.height -= clipped_pixels;
        clipped_pixels = (uint32_t)vpe_fixpt_round(
            vpe_fixpt_mul_int(scaling_ratio_v, (int)(target_rect->y - dst_rect->y)));
        clipped_src_rect.y += (int32_t)clipped_pixels;
        clipped_src_rect.height -= clipped_pixels;
    }
    if (dst_rect->x + (int32_t)dst_rect->width > target_rect->x + (int32_t)target_rect->width) {
        clipped_dst_rect.width =
            (uint32_t)(target_rect->x + (int32_t)target_rect->width - clipped_dst_rect.x);
        clipped_src_rect.width = (uint32_t)vpe_fixpt_round(
            vpe_fixpt_mul_int(scaling_ratio_h, (int)clipped_dst_rect.width));
    }
    if (dst_rect->y + (int32_t)dst_rect->height > target_rect->y + (int32_t)target_rect->height) {
        clipped_dst_rect.height =
            (uint32_t)(target_rect->y + (int32_t)target_rect->height - clipped_dst_rect.y);
        clipped_src_rect.height = (uint32_t)vpe_fixpt_round(
            vpe_fixpt_mul_int(scaling_ratio_v, (int)clipped_dst_rect.height));
    }

    *src_rect = clipped_src_rect;
    *dst_rect = clipped_dst_rect;
}

enum vpe_status vpe_resource_build_scaling_params(struct segment_ctx *segment_ctx)
{
    struct stream_ctx  *stream_ctx = segment_ctx->stream_ctx;
    struct scaler_data *scl_data   = &segment_ctx->scaler_data;
    struct dpp         *dpp        = stream_ctx->vpe_priv->resource.dpp[0];

    scl_data->format             = stream_ctx->stream.surface_info.format;
    scl_data->lb_params.alpha_en = stream_ctx->per_pixel_alpha;

    // h/v active will be set later

    /* recout.x is temporary for viewport calculation,
     * will be finalized in calculate_dst_viewport_and_active()
     */

    calculate_recout(segment_ctx);
    calculate_viewport_size(segment_ctx);

    if (scl_data->viewport.height < 1 || scl_data->viewport.width < 1)
        return VPE_STATUS_VIEWPORT_SIZE_NOT_SUPPORTED;

    if (!dpp->funcs->validate_number_of_taps(dpp, scl_data)) {
        return VPE_STATUS_SCALING_RATIO_NOT_SUPPORTED;
    }

    calculate_inits_and_viewports(segment_ctx);

    if (scl_data->viewport.height < VPE_MIN_VIEWPORT_SIZE ||
        scl_data->viewport.width < VPE_MIN_VIEWPORT_SIZE)
        return VPE_STATUS_VIEWPORT_SIZE_NOT_SUPPORTED;

    return VPE_STATUS_OK;
}

void vpe_handle_output_h_mirror(struct vpe_priv *vpe_priv)
{
    uint16_t           stream_idx;
    int                seg_idx;
    struct stream_ctx *stream_ctx;

    // swap the stream output location
    for (stream_idx = 0; stream_idx < vpe_priv->num_streams; stream_idx++) {
        stream_ctx = &vpe_priv->stream_ctx[stream_idx];
        if (stream_ctx->flip_horizonal_output) {
            struct segment_ctx *first_seg, *last_seg;

            // swap the segment output order, init the last segment first
            first_seg = &stream_ctx->segment_ctx[0];
            last_seg  = &stream_ctx->segment_ctx[stream_ctx->num_segments - 1];

            // last segment becomes first
            last_seg->scaler_data.dst_viewport.x = first_seg->scaler_data.dst_viewport.x;

            for (seg_idx = (int)(stream_ctx->num_segments - 2); seg_idx >= 0; seg_idx--) {
                struct segment_ctx *prev_seg, *curr_seg;

                // set the x in reverse order
                prev_seg = &stream_ctx->segment_ctx[seg_idx + 1];
                curr_seg = &stream_ctx->segment_ctx[seg_idx];

                curr_seg->scaler_data.dst_viewport.x =
                    prev_seg->scaler_data.dst_viewport.x +
                    (int32_t)prev_seg->scaler_data.dst_viewport.width;

                curr_seg->scaler_data.dst_viewport_c.x =
                    prev_seg->scaler_data.dst_viewport_c.x +
                    (int32_t)prev_seg->scaler_data.dst_viewport_c.width;
            }
        }
    }
}

void vpe_resource_build_bit_depth_reduction_params(
    struct opp *opp, struct bit_depth_reduction_params *fmt_bit_depth)
{
    struct vpe_priv         *vpe_priv    = opp->vpe_priv;
    struct vpe_surface_info *dst_surface = &vpe_priv->output_ctx.surface;
    enum color_depth         display_color_depth;
    memset(fmt_bit_depth, 0, sizeof(*fmt_bit_depth));

    display_color_depth = vpe_get_color_depth(dst_surface->format);

    switch (display_color_depth) {
    case COLOR_DEPTH_888:
    case COLOR_DEPTH_101010:
        fmt_bit_depth->flags.TRUNCATE_ENABLED = 1;
        fmt_bit_depth->flags.TRUNCATE_DEPTH   = (display_color_depth == COLOR_DEPTH_888) ? 1 : 2;
        fmt_bit_depth->flags.TRUNCATE_MODE    = 1;
        break;
    default:
        break;
    }
}
