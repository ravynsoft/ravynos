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
#include "vpe_assert.h"
#include "common.h"
#include "vpe_priv.h"
#include "vpe_command.h"
#include "vpe10_cmd_builder.h"
#include "plane_desc_writer.h"
#include "reg_helper.h"

/***** Internal helpers *****/
static void get_np(struct vpe_priv *vpe_priv, struct vpe_cmd_info *cmd_info, int32_t *nps0,
    int32_t *nps1, int32_t *npd0, int32_t *npd1);

static enum VPE_PLANE_CFG_ELEMENT_SIZE vpe_get_element_size(
    enum vpe_surface_pixel_format format, int plane_idx);

void vpe10_construct_cmd_builder(struct vpe_priv *vpe_priv, struct cmd_builder *builder)
{
    builder->build_noops            = vpe10_build_noops;
    builder->build_vpe_cmd          = vpe10_build_vpe_cmd;
    builder->build_plane_descriptor = vpe10_build_plane_descriptor;
}

enum vpe_status vpe10_build_noops(struct vpe_priv *vpe_priv, uint32_t **ppbuf, uint32_t num_dwords)
{
    uint32_t  i;
    uint32_t *buffer = *ppbuf;
    uint32_t  noop   = VPE_CMD_HEADER(VPE_CMD_OPCODE_NOP, 0);

    for (i = 0; i < num_dwords; i++)
        *buffer++ = noop;

    *ppbuf = buffer;

    return VPE_STATUS_OK;
}

enum vpe_status vpe10_build_vpe_cmd(
    struct vpe_priv *vpe_priv, struct vpe_build_bufs *cur_bufs, uint32_t cmd_idx)
{
    struct cmd_builder  *builder  = &vpe_priv->resource.cmd_builder;
    struct vpe_buf      *emb_buf  = &cur_bufs->emb_buf;
    struct vpe_cmd_info *cmd_info = &vpe_priv->vpe_cmd_info[cmd_idx];
    struct output_ctx   *output_ctx;
    struct pipe_ctx     *pipe_ctx = NULL;
    uint32_t             i, j;

    vpe_desc_writer_init(&vpe_priv->vpe_desc_writer, &cur_bufs->cmd_buf, cmd_info->cd);

    // plane descriptor
    builder->build_plane_descriptor(vpe_priv, emb_buf, cmd_idx);

    vpe_desc_writer_add_plane_desc(
        &vpe_priv->vpe_desc_writer, vpe_priv->plane_desc_writer.base_gpu_va, emb_buf->tmz);

    // reclaim any pipe if the owner no longer presents
    vpe_pipe_reclaim(vpe_priv, cmd_info);

    config_writer_init(&vpe_priv->config_writer, emb_buf);

    // frontend programming
    for (i = 0; i < cmd_info->num_inputs; i++) {
        bool               reuse;
        struct stream_ctx *stream_ctx;
        enum vpe_cmd_type  cmd_type = VPE_CMD_TYPE_COUNT;

        // keep using the same pipe whenever possible
        // this would allow reuse of the previous register configs
        pipe_ctx = vpe_pipe_find_owner(vpe_priv, cmd_info->inputs[i].stream_idx, &reuse);
        VPE_ASSERT(pipe_ctx);

        if (!reuse) {
            vpe_priv->resource.program_frontend(vpe_priv, pipe_ctx->pipe_idx, cmd_idx, i, false);
        } else {
            if (vpe_priv->init.debug.disable_reuse_bit)
                reuse = false;

            stream_ctx = &vpe_priv->stream_ctx[cmd_info->inputs[i].stream_idx];

            // frame specific for same type of command
            if (cmd_info->ops == VPE_CMD_OPS_BG)
                cmd_type = VPE_CMD_TYPE_BG;
            else if (cmd_info->ops == VPE_CMD_OPS_COMPOSITING)
                cmd_type = VPE_CMD_TYPE_COMPOSITING;
            else if (cmd_info->ops == VPE_CMD_OPS_BG_VSCF_INPUT)
                cmd_type = VPE_CMD_TYPE_BG_VSCF_INPUT;
            else if (cmd_info->ops == VPE_CMD_OPS_BG_VSCF_OUTPUT)
                cmd_type = VPE_CMD_TYPE_BG_VSCF_OUTPUT;
            else {
                VPE_ASSERT(0);
                return VPE_STATUS_ERROR;
            }

            // follow the same order of config generation in "non-reuse" case
            // stream sharing
            VPE_ASSERT(stream_ctx->num_configs);
            for (j = 0; j < stream_ctx->num_configs; j++) {
                vpe_desc_writer_add_config_desc(&vpe_priv->vpe_desc_writer,
                    stream_ctx->configs[j].config_base_addr, reuse, emb_buf->tmz);
            }

            // stream-op sharing
            for (j = 0; j < stream_ctx->num_stream_op_configs[cmd_type]; j++) {
                vpe_desc_writer_add_config_desc(&vpe_priv->vpe_desc_writer,
                    stream_ctx->stream_op_configs[cmd_type][j].config_base_addr, reuse,
                    emb_buf->tmz);
            }

            // command specific
            vpe_priv->resource.program_frontend(vpe_priv, pipe_ctx->pipe_idx, cmd_idx, i, true);
        }
    }

    VPE_ASSERT(pipe_ctx);

    // If config writer has been crashed due to buffer overflow
    if (vpe_priv->config_writer.status != VPE_STATUS_OK) {
        return vpe_priv->config_writer.status;
    }

    // backend programming
    output_ctx = &vpe_priv->output_ctx;
    if (!output_ctx->num_configs) {
        vpe_priv->resource.program_backend(vpe_priv, pipe_ctx->pipe_idx, cmd_idx, false);
    } else {
        bool reuse = !vpe_priv->init.debug.disable_reuse_bit;
        // re-use output register configs
        for (j = 0; j < output_ctx->num_configs; j++) {
            vpe_desc_writer_add_config_desc(&vpe_priv->vpe_desc_writer,
                output_ctx->configs[j].config_base_addr, reuse, emb_buf->tmz);
        }

        vpe_priv->resource.program_backend(vpe_priv, pipe_ctx->pipe_idx, cmd_idx, true);
    }

    /* If writer crashed due to buffer overflow */
    if (vpe_priv->vpe_desc_writer.status != VPE_STATUS_OK) {
        return vpe_priv->vpe_desc_writer.status;
    }
    vpe_desc_writer_complete(&vpe_priv->vpe_desc_writer);

    return VPE_STATUS_OK;
}

enum vpe_status vpe10_build_plane_descriptor(
    struct vpe_priv *vpe_priv, struct vpe_buf *buf, uint32_t cmd_idx)
{
    struct stream_ctx       *stream_ctx;
    struct vpe_surface_info *surface_info;
    int32_t                  nps0, nps1, npd0, npd1;
    int32_t                  stream_idx;
    struct vpe_cmd_info     *cmd_info;
    PHYSICAL_ADDRESS_LOC    *addrloc;
    struct plane_desc_src    src;
    struct plane_desc_dst    dst;

    cmd_info = &vpe_priv->vpe_cmd_info[cmd_idx];

    VPE_ASSERT(cmd_info->num_inputs == 1);

    // obtains number of planes for each source/destination stream
    get_np(vpe_priv, cmd_info, &nps0, &nps1, &npd0, &npd1);

    plane_desc_writer_init(
        &vpe_priv->plane_desc_writer, buf, nps0, npd0, nps1, npd1, VPE_PLANE_CFG_SUBOP_1_TO_1);

    stream_idx   = cmd_info->inputs[0].stream_idx;
    stream_ctx   = &vpe_priv->stream_ctx[stream_idx];
    surface_info = &stream_ctx->stream.surface_info;

    src.tmz      = surface_info->address.tmz_surface;
    src.swizzle  = surface_info->swizzle;
    src.rotation = stream_ctx->stream.rotation;

    if (surface_info->address.type == VPE_PLN_ADDR_TYPE_VIDEO_PROGRESSIVE) {
        addrloc = &surface_info->address.video_progressive.luma_addr;

        src.base_addr_lo = addrloc->u.low_part;
        src.base_addr_hi = (uint32_t)addrloc->u.high_part;
        src.pitch        = (uint16_t)surface_info->plane_size.surface_pitch;
        src.viewport_x   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport.x;
        src.viewport_y   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport.y;
        src.viewport_w   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport.width;
        src.viewport_h   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport.height;
        src.elem_size    = (uint8_t)(vpe_get_element_size(surface_info->format, 0));

        plane_desc_writer_add_source(&vpe_priv->plane_desc_writer, &src, true);

        if (vpe_is_dual_plane_format(surface_info->format)) {
            addrloc = &surface_info->address.video_progressive.chroma_addr;

            src.base_addr_lo = addrloc->u.low_part;
            src.base_addr_hi = (uint32_t)addrloc->u.high_part;
            src.pitch        = (uint16_t)surface_info->plane_size.chroma_pitch;
            src.viewport_x   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport_c.x;
            src.viewport_y   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport_c.y;
            src.viewport_w   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport_c.width;
            src.viewport_h   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport_c.height;
            src.elem_size    = (uint8_t)(vpe_get_element_size(surface_info->format, 1));

            plane_desc_writer_add_source(&vpe_priv->plane_desc_writer, &src, false);
        }
    } else {
        addrloc = &surface_info->address.grph.addr;

        src.base_addr_lo = addrloc->u.low_part;
        src.base_addr_hi = (uint32_t)addrloc->u.high_part;
        src.pitch        = (uint16_t)surface_info->plane_size.surface_pitch;
        src.viewport_x   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport.x;
        src.viewport_y   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport.y;
        src.viewport_w   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport.width;
        src.viewport_h   = (uint16_t)cmd_info->inputs[0].scaler_data.viewport.height;
        src.elem_size    = (uint8_t)(vpe_get_element_size(surface_info->format, 0));

        plane_desc_writer_add_source(&vpe_priv->plane_desc_writer, &src, true);
    }

    surface_info = &vpe_priv->output_ctx.surface;

    VPE_ASSERT(surface_info->address.type == VPE_PLN_ADDR_TYPE_GRAPHICS);

    addrloc = &surface_info->address.grph.addr;

    dst.tmz     = surface_info->address.tmz_surface;
    dst.swizzle = surface_info->swizzle;

    if (stream_ctx->flip_horizonal_output)
        dst.mirror = VPE_MIRROR_HORIZONTAL;
    else
        dst.mirror = VPE_MIRROR_NONE;

    dst.base_addr_lo = addrloc->u.low_part;
    dst.base_addr_hi = (uint32_t)addrloc->u.high_part;
    dst.pitch        = (uint16_t)surface_info->plane_size.surface_pitch;
    dst.viewport_x   = (uint16_t)cmd_info->dst_viewport.x;
    dst.viewport_y   = (uint16_t)cmd_info->dst_viewport.y;
    dst.viewport_w   = (uint16_t)cmd_info->dst_viewport.width;
    dst.viewport_h   = (uint16_t)cmd_info->dst_viewport.height;
    dst.elem_size    = (uint8_t)(vpe_get_element_size(surface_info->format, 0));

    plane_desc_writer_add_destination(&vpe_priv->plane_desc_writer, &dst, true);

    return vpe_priv->plane_desc_writer.status;
}

static void get_np(struct vpe_priv *vpe_priv, struct vpe_cmd_info *cmd_info, int32_t *nps0,
    int32_t *nps1, int32_t *npd0, int32_t *npd1)
{
    *npd1 = 0;

    if (cmd_info->num_inputs == 1) {
        *nps1 = 0;
        if (vpe_is_dual_plane_format(
                vpe_priv->stream_ctx[cmd_info->inputs[0].stream_idx].stream.surface_info.format))
            *nps0 = VPE_PLANE_CFG_TWO_PLANES;
        else
            *nps0 = VPE_PLANE_CFG_ONE_PLANE;
    } else if (cmd_info->num_inputs == 2) {
        if (vpe_is_dual_plane_format(
                vpe_priv->stream_ctx[cmd_info->inputs[0].stream_idx].stream.surface_info.format))
            *nps0 = VPE_PLANE_CFG_TWO_PLANES;
        else
            *nps0 = VPE_PLANE_CFG_ONE_PLANE;

        if (vpe_is_dual_plane_format(
                vpe_priv->stream_ctx[cmd_info->inputs[1].stream_idx].stream.surface_info.format))
            *nps1 = VPE_PLANE_CFG_TWO_PLANES;
        else
            *nps1 = VPE_PLANE_CFG_ONE_PLANE;
    } else {
        *nps0 = 0;
        *nps1 = 0;
        *npd0 = 0;
        return;
    }

    if (vpe_is_dual_plane_format(vpe_priv->output_ctx.surface.format))
        *npd0 = 1;
    else
        *npd0 = 0;
}

static enum VPE_PLANE_CFG_ELEMENT_SIZE vpe_get_element_size(
    enum vpe_surface_pixel_format format, int plane_idx)
{
    switch (format) {
        // nv12/21
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
        if (plane_idx == 0)
            return VPE_PLANE_CFG_ELEMENT_SIZE_8BPE;
        else
            return VPE_PLANE_CFG_ELEMENT_SIZE_16BPE;
        // P010
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
        if (plane_idx == 0)
            return VPE_PLANE_CFG_ELEMENT_SIZE_16BPE;
        else
            return VPE_PLANE_CFG_ELEMENT_SIZE_32BPE;
        // 64bpp
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        return VPE_PLANE_CFG_ELEMENT_SIZE_64BPE;
    default:
        break;
    }
    return VPE_PLANE_CFG_ELEMENT_SIZE_32BPE;
}
