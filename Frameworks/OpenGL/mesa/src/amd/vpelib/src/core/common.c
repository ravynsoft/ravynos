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

#include <string.h>
#include "vpe_types.h"
#include "vpe_priv.h"
#include "common.h"

bool vpe_find_color_space_from_table(
    const struct vpe_color_space *table, int table_size, const struct vpe_color_space *cs)
{
    int i;
    for (i = 0; i < table_size; i++) {
        if (!memcmp(table, cs, sizeof(struct vpe_color_space)))
            return true;
    }
    return false;
}

bool vpe_is_dual_plane_format(enum vpe_surface_pixel_format format)
{
    switch (format) {
        // nv12/21
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
        // p010
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
        return true;
    default:
        return false;
    }
}

bool vpe_is_32bit_packed_rgb(enum vpe_surface_pixel_format format)
{
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
        return true;
    default:
        return false;
    }
}

bool vpe_is_rgb8(enum vpe_surface_pixel_format format)
{
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
        return true;
    default:
        return false;
    }
}

bool vpe_is_rgb10(enum vpe_surface_pixel_format format)
{
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
        return true;
    default:
        return false;
    }
}

bool vpe_is_fp16(enum vpe_surface_pixel_format format)
{
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        return true;
    default:
        return false;
    }
}

bool vpe_is_yuv420_8(enum vpe_surface_pixel_format format)
{
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
        return true;
    default:
        return false;
    }
}

bool vpe_is_yuv420_10(enum vpe_surface_pixel_format format)
{
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
        return true;
    default:
        return false;
    }
}

bool vpe_is_yuv420(enum vpe_surface_pixel_format format)
{
    return (vpe_is_yuv420_8(format) || vpe_is_yuv420_10(format));
}

bool vpe_is_yuv444_8(enum vpe_surface_pixel_format format)
{
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCrCb8888:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCbCr8888:
        return true;
    default:
        return false;
    }
}

bool vpe_is_yuv444_10(enum vpe_surface_pixel_format format)
{
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_ACrYCb2101010:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_CrYCbA1010102:
        return true;
    default:
        return false;
    }
}

static uint8_t vpe_get_element_size_in_bytes(enum vpe_surface_pixel_format format, int plane_idx)
{
    switch (format) {
        // nv12/21
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
        if (plane_idx == 0)
            return 1;
        else
            return 2;
        // P010
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
        if (plane_idx == 0)
            return 2;
        else
            return 4;
        // 64bpp
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        return 8;
    default:
        break;
    }
    // default 32bpp packed format
    return 4;
}

enum color_depth vpe_get_color_depth(enum vpe_surface_pixel_format format)
{
    enum color_depth c_depth;
    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB565:
        c_depth = COLOR_DEPTH_666;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCrCb8888:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCbCr8888:
        c_depth = COLOR_DEPTH_888;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_ACrYCb2101010:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_CrYCbA1010102:
        c_depth = COLOR_DEPTH_101010;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        c_depth = COLOR_DEPTH_161616;
        break;
    default:
        c_depth = COLOR_DEPTH_888;
    }

    return c_depth;
}

bool vpe_has_per_pixel_alpha(enum vpe_surface_pixel_format format)
{
    bool alpha = true;

    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB1555:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBE_ALPHA:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_ACrYCb2101010:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_CrYCbA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCrCb8888:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCbCr8888:
        alpha = true;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB565:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB111110_FIX:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGR101111_FIX:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB111110_FLOAT:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGR101111_FLOAT:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBE:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
    default:
        alpha = false;
        break;
    }

    return alpha;
}

// Note there is another function vpe_is_hdr that performs the same function but with the translated
// internal VPE enums, not the api enums as done below. C does not support function overloading so
// another function is needed here.
static bool is_HDR(enum vpe_transfer_function tf)
{
    return (tf == VPE_TF_PQ || tf == VPE_TF_G10);
}

enum vpe_status vpe_check_output_support(struct vpe *vpe, const struct vpe_build_param *param)
{
    struct vpe_priv               *vpe_priv = container_of(vpe, struct vpe_priv, pub);
    struct vpec                   *vpec;
    struct dpp                    *dpp;
    struct cdc                    *cdc;
    const struct vpe_surface_info *surface_info = &param->dst_surface;
    struct vpe_dcc_surface_param   input;
    struct vpe_surface_dcc_cap     output;
    bool                           support;

    vpec = &vpe_priv->resource.vpec;
    dpp  = vpe_priv->resource.dpp[0];
    cdc  = vpe_priv->resource.cdc[0];

    // swizzle mode
    support = vpec->funcs->check_swmode_support(vpec, surface_info->swizzle);
    if (!support) {
        vpe_log("output swizzle mode not supported %d\n", surface_info->swizzle);
        return VPE_STATUS_SWIZZLE_NOT_SUPPORTED;
    }

    // pitch
    if ((surface_info->plane_size.surface_pitch *
            vpe_get_element_size_in_bytes(surface_info->format, 0) %
            vpe->caps->plane_caps.pitch_alignment) ||
        ((uint32_t)(surface_info->plane_size.surface_size.x +
                    (int32_t)surface_info->plane_size.surface_size.width) >
            surface_info->plane_size.surface_pitch)) {
        vpe_log("pitch alignment not supported %lu. %lu\n", surface_info->plane_size.surface_pitch,
            vpe->caps->plane_caps.pitch_alignment);
        return VPE_STATUS_PITCH_ALIGNMENT_NOT_SUPPORTED;
    }

    // target rect shouldn't exceed width/height
    if ((param->target_rect.x < surface_info->plane_size.surface_size.x ||
            param->target_rect.x + (int32_t)param->target_rect.width >
                surface_info->plane_size.surface_size.x +
                    (int32_t)surface_info->plane_size.surface_size.width)) {
        vpe_log("target rect exceed surface boundary, target x= %d, width = %u, surface x = %d, "
                "width = %u\n",
            param->target_rect.x, param->target_rect.width, surface_info->plane_size.surface_size.x,
            surface_info->plane_size.surface_size.width);
        return VPE_STATUS_PARAM_CHECK_ERROR;
    }

    if ((param->target_rect.y < surface_info->plane_size.surface_size.y ||
            param->target_rect.y + (int32_t)param->target_rect.height >
                surface_info->plane_size.surface_size.y +
                    (int32_t)surface_info->plane_size.surface_size.height)) {
        vpe_log(
            "target rect exceed surface boundary, y= %d, height = %u, surface x = %d, width = %u\n",
            param->target_rect.y, param->target_rect.height,
            surface_info->plane_size.surface_size.y, surface_info->plane_size.surface_size.height);
        return VPE_STATUS_PARAM_CHECK_ERROR;
    }

    if (surface_info->address.type == VPE_PLN_ADDR_TYPE_VIDEO_PROGRESSIVE) {
        if (((uint32_t)surface_info->plane_size.chroma_pitch *
                vpe_get_element_size_in_bytes(surface_info->format, 1) %
                vpe->caps->plane_caps.pitch_alignment) ||
            ((uint32_t)(surface_info->plane_size.chroma_size.x +
                        (int32_t)surface_info->plane_size.chroma_size.width) >
                surface_info->plane_size.chroma_pitch)) {
            vpe_log("chroma pitch alignment not supported %u. %u\n",
                surface_info->plane_size.chroma_pitch, vpe->caps->plane_caps.pitch_alignment);
            return VPE_STATUS_PITCH_ALIGNMENT_NOT_SUPPORTED;
        }
    }

    // dcc
    if (surface_info->dcc.enable) {
        input.surface_size.width  = surface_info->plane_size.surface_size.width;
        input.surface_size.height = surface_info->plane_size.surface_size.height;
        input.format              = surface_info->format;
        input.swizzle_mode        = surface_info->swizzle;
        input.scan                = VPE_SCAN_DIRECTION_HORIZONTAL;

        support = vpec->funcs->get_dcc_compression_cap(vpec, &input, &output);
        if (!support) {
            vpe_log("output dcc not supported\n");
            return VPE_STATUS_DCC_NOT_SUPPORTED;
        }
    }

    // pixel format
    support = cdc->funcs->check_output_format(cdc, surface_info->format);
    if (!support) {
        vpe_log("output pixel format not supported %d\n", (int)surface_info->format);
        return VPE_STATUS_PIXEL_FORMAT_NOT_SUPPORTED;
    }

    // color space value
    support = vpe_priv->resource.check_output_color_space(
        vpe_priv, surface_info->format, &surface_info->cs);
    if (!support) {
        vpe_log("output color space not supported fmt: %d, "
                "encoding: %d, cositing: %d, gamma: %d, range: %d, primaries: %d\n",
            (int)surface_info->format, (int)surface_info->cs.encoding,
            (int)surface_info->cs.cositing, (int)surface_info->cs.tf, (int)surface_info->cs.range,
            (int)surface_info->cs.primaries);
        return VPE_STATUS_COLOR_SPACE_VALUE_NOT_SUPPORTED;
    }

    return VPE_STATUS_OK;
}

enum vpe_status vpe_check_input_support(struct vpe *vpe, const struct vpe_stream *stream)
{
    struct vpe_priv               *vpe_priv = container_of(vpe, struct vpe_priv, pub);
    struct vpec                   *vpec;
    struct dpp                    *dpp;
    struct cdc                    *cdc;
    const struct vpe_surface_info *surface_info = &stream->surface_info;
    struct vpe_dcc_surface_param   input;
    struct vpe_surface_dcc_cap     output;
    bool                           support;
    const PHYSICAL_ADDRESS_LOC    *addrloc;
    bool                           use_adj = vpe_use_csc_adjust(&stream->color_adj);

    vpec = &vpe_priv->resource.vpec;
    dpp  = vpe_priv->resource.dpp[0];
    cdc  = vpe_priv->resource.cdc[0];

    // swizzle mode
    support = vpec->funcs->check_swmode_support(vpec, surface_info->swizzle);
    if (!support) {
        vpe_log("input swizzle mode not supported %d\n", surface_info->swizzle);
        return VPE_STATUS_SWIZZLE_NOT_SUPPORTED;
    }

    // pitch & address
    if ((surface_info->plane_size.surface_pitch *
            vpe_get_element_size_in_bytes(surface_info->format, 0) %
            vpe->caps->plane_caps.pitch_alignment) ||
        ((uint32_t)(surface_info->plane_size.surface_size.x +
                    (int32_t)surface_info->plane_size.surface_size.width) >
            surface_info->plane_size.surface_pitch)) {

        vpe_log("pitch alignment not supported %d. %d\n", surface_info->plane_size.surface_pitch,
            vpe->caps->plane_caps.pitch_alignment);
        return VPE_STATUS_PITCH_ALIGNMENT_NOT_SUPPORTED;
    }

    if (surface_info->address.type == VPE_PLN_ADDR_TYPE_VIDEO_PROGRESSIVE) {

        addrloc = &surface_info->address.video_progressive.luma_addr;
        if (addrloc->u.low_part % vpe->caps->plane_caps.addr_alignment) {
            vpe_log("failed. addr not aligned to 256 bytes\n");
            return VPE_STATUS_PLANE_ADDR_NOT_SUPPORTED;
        }

        if (vpe_is_dual_plane_format(surface_info->format)) {
            if ((surface_info->plane_size.chroma_pitch *
                    vpe_get_element_size_in_bytes(surface_info->format, 1) %
                    vpe->caps->plane_caps.pitch_alignment) ||
                ((uint32_t)(surface_info->plane_size.chroma_size.x +
                            (int32_t)surface_info->plane_size.chroma_size.width) >
                    surface_info->plane_size.chroma_pitch)) {
                vpe_log("chroma pitch alignment not supported %d. %d\n",
                    surface_info->plane_size.chroma_pitch, vpe->caps->plane_caps.pitch_alignment);
                return VPE_STATUS_PITCH_ALIGNMENT_NOT_SUPPORTED;
            }

            addrloc = &surface_info->address.video_progressive.chroma_addr;
            if (addrloc->u.low_part % vpe->caps->plane_caps.addr_alignment) {
                vpe_log("failed. addr not aligned to 256 bytes\n");
                return VPE_STATUS_PLANE_ADDR_NOT_SUPPORTED;
            }
        }
    } else {
        addrloc = &surface_info->address.grph.addr;
        if (addrloc->u.low_part % vpe->caps->plane_caps.addr_alignment) {
            vpe_log("failed. addr not aligned to 256 bytes\n");
            return VPE_STATUS_PLANE_ADDR_NOT_SUPPORTED;
        }
    }

    // dcc
    if (surface_info->dcc.enable) {

        input.surface_size.width  = surface_info->plane_size.surface_size.width;
        input.surface_size.height = surface_info->plane_size.surface_size.height;
        input.format              = surface_info->format;
        input.swizzle_mode        = surface_info->swizzle;

        if (stream->rotation == VPE_ROTATION_ANGLE_0 || stream->rotation == VPE_ROTATION_ANGLE_180)
            input.scan = VPE_SCAN_DIRECTION_HORIZONTAL;
        else if (stream->rotation == VPE_ROTATION_ANGLE_90 ||
                 stream->rotation == VPE_ROTATION_ANGLE_270)
            input.scan = VPE_SCAN_DIRECTION_VERTICAL;
        else
            input.scan = VPE_SCAN_DIRECTION_UNKNOWN;

        support = vpec->funcs->get_dcc_compression_cap(vpec, &input, &output);
        if (!support) {
            vpe_log("input dcc not supported\n");
            return VPE_STATUS_DCC_NOT_SUPPORTED;
        }
    }

    // pixel format
    support = cdc->funcs->check_input_format(cdc, surface_info->format);
    if (!support) {
        vpe_log("input pixel format not supported %d\n", (int)surface_info->format);
        return VPE_STATUS_PIXEL_FORMAT_NOT_SUPPORTED;
    }

    // color space value
    support = vpe_priv->resource.check_input_color_space(
        vpe_priv, surface_info->format, &surface_info->cs);
    if (!support) {
        vpe_log("input color space not supported fmt: %d, "
                "encoding: %d, cositing: %d, gamma: %d, range: %d, primaries: %d\n",
            (int)surface_info->format, (int)surface_info->cs.encoding,
            (int)surface_info->cs.cositing, (int)surface_info->cs.tf, (int)surface_info->cs.range,
            (int)surface_info->cs.primaries);
        return VPE_STATUS_COLOR_SPACE_VALUE_NOT_SUPPORTED;
    }

    // TODO: Add support
    // adjustments
    if (surface_info->cs.primaries == VPE_PRIMARIES_BT2020 &&
        surface_info->cs.encoding == VPE_PIXEL_ENCODING_RGB && use_adj) {
        // for BT2020 + RGB input with adjustments, it is expected not working.
        vpe_log("for BT2020 + RGB input with adjustments, it is expected not working\n");
        return VPE_STATUS_ADJUSTMENT_NOT_SUPPORTED;
    }

    // rotation
    if ((stream->rotation != VPE_ROTATION_ANGLE_0) && !vpe->caps->rotation_support) {
        vpe_log("output rotation not supported\n");
        return VPE_STATUS_ROTATION_NOT_SUPPORTED;
    }

    // luma keying
    if (stream->enable_luma_key && !vpe->caps->color_caps.dpp.luma_key) {
        vpe_log("luma keying not supported\n");
        return VPE_STATUS_LUMA_KEYING_NOT_SUPPORTED;
    }

    if (stream->horizontal_mirror && !vpe->caps->h_mirror_support) {
        vpe_log("output horizontal mirroring not supported h:%d\n", (int)stream->horizontal_mirror);
        return VPE_STATUS_MIRROR_NOT_SUPPORTED;
    }

    if (stream->vertical_mirror && !vpe->caps->v_mirror_support) {
        vpe_log("output vertical mirroring not supported v:%d\n", (int)stream->vertical_mirror);
        return VPE_STATUS_MIRROR_NOT_SUPPORTED;
    }

    return VPE_STATUS_OK;
}

enum vpe_status vpe_cache_tone_map_params(
    struct stream_ctx *stream_ctx, const struct vpe_build_param *param)
{

    stream_ctx->update_3dlut = stream_ctx->update_3dlut || param->streams->tm_params.update_3dlut;

    return VPE_STATUS_OK;
}

enum vpe_status vpe_check_tone_map_support(
    struct vpe *vpe, const struct vpe_stream *stream, const struct vpe_build_param *param)
{
    enum vpe_status status = VPE_STATUS_OK;

    // If tone map enabled but bad luminance reject.
    if (stream->tm_params.enable_3dlut &&
        stream->hdr_metadata.max_mastering <= param->hdr_metadata.max_mastering) {
        status = VPE_STATUS_BAD_TONE_MAP_PARAMS;
        goto exit;
    }

    // If tone map enabled but input is not HDR, reject.
    if (stream->tm_params.enable_3dlut && !is_HDR(stream->surface_info.cs.tf)) {
        status = VPE_STATUS_BAD_TONE_MAP_PARAMS;
        goto exit;
    }

    // If tone map case but enable tm flag is not set or 3dlut pointer is null reject.
    if (stream->hdr_metadata.max_mastering > param->hdr_metadata.max_mastering &&
        is_HDR(stream->surface_info.cs.tf) &&
        (!stream->tm_params.enable_3dlut || stream->tm_params.lut_data == NULL)) {
        status = VPE_STATUS_BAD_HDR_METADATA;
    }

exit:
    return status;
}
