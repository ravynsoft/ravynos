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

#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

enum vpe_status vpe10_construct_resource(struct vpe_priv *vpe_priv, struct resource *res);

void vpe10_destroy_resource(struct vpe_priv *vpe_priv, struct resource *res);

enum vpe_status vpe10_set_num_segments(struct vpe_priv *vpe_priv, struct stream_ctx *stream_ctx,
    struct scaler_data *scl_data, struct vpe_rect *src_rect, struct vpe_rect *dst_rect,
    uint32_t *max_seg_width);

bool vpe10_get_dcc_compression_cap(const struct vpe *vpe, const struct vpe_dcc_surface_param *input,
    struct vpe_surface_dcc_cap *output);

bool vpe10_check_input_color_space(struct vpe_priv *vpe_priv, enum vpe_surface_pixel_format format,
    const struct vpe_color_space *vcs);

bool vpe10_check_output_color_space(struct vpe_priv *vpe_priv, enum vpe_surface_pixel_format format,
    const struct vpe_color_space *vcs);

bool vpe10_check_h_mirror_support(bool *input_mirror, bool *output_mirror);

enum vpe_status vpe10_calculate_segments(
    struct vpe_priv *vpe_priv, const struct vpe_build_param *params);

int32_t vpe10_program_frontend(struct vpe_priv *vpe_priv, uint32_t pipe_idx, uint32_t cmd_idx,
    uint32_t cmd_input_idx, bool seg_only);

int32_t vpe10_program_backend(
    struct vpe_priv *vpe_priv, uint32_t pipe_idx, uint32_t cmd_idx, bool seg_only);

enum vpe_status vpe10_populate_cmd_info(struct vpe_priv *vpe_priv);

void vpe10_calculate_dst_viewport_and_active(
    struct segment_ctx *segment_ctx, uint32_t max_seg_width);

void vpe10_create_stream_ops_config(struct vpe_priv *vpe_priv, uint32_t pipe_idx,
    struct stream_ctx *stream_ctx, struct vpe_cmd_input *cmd_input, enum vpe_cmd_ops ops);

void vpe10_get_bufs_req(struct vpe_priv *vpe_priv, struct vpe_bufs_req *req);

struct opp *vpe10_opp_create(struct vpe_priv *vpe_priv, int inst);

struct mpc *vpe10_mpc_create(struct vpe_priv *vpe_priv, int inst);

struct dpp *vpe10_dpp_create(struct vpe_priv *vpe_priv, int inst);

struct cdc *vpe10_cdc_create(struct vpe_priv *vpe_priv, int inst);

#ifdef __cplusplus
}
#endif
