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

#include "cmd_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

void vpe10_construct_cmd_builder(struct vpe_priv *vpe_priv, struct cmd_builder *cmd_builder);

enum vpe_status vpe10_build_noops(struct vpe_priv *vpe_priv, uint32_t **ppbuf, uint32_t num_dwords);

enum vpe_status vpe10_build_vpe_cmd(
    struct vpe_priv *vpe_priv, struct vpe_build_bufs *cur_bufs, uint32_t cmd_idx);

enum vpe_status vpe10_build_plane_descriptor(
    struct vpe_priv *vpe_priv, struct vpe_buf *buf, uint32_t cmd_idx);

#ifdef __cplusplus
}
#endif
