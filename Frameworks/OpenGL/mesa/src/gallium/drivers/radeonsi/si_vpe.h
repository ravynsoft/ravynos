/**************************************************************************
 *
 * Copyright 2022 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef SI_VPE_H
#define SI_VPE_H

#include "pipe/p_screen.h"
#include "pipe/p_video_codec.h"
#include "vpelib/inc/vpelib.h"
#include "radeon_video.h"

/* The buffer size of cmd_buf and emb_buf in bytes
 *
 * TODO: vpe-utils also use this value. Need to be reviewed further.
 */
#define VPE_BUILD_BUFS_SIZE	 1000000
#define VPE_FENCE_TIMEOUT_NS 1000000000

/* For Hooking VPE as a decoder instance */
struct vpe_video_processor {
    struct pipe_video_codec base;

    struct pipe_screen *screen;
    struct radeon_winsys *ws;
    struct radeon_cmdbuf cs;

    struct rvid_buffer cmd_buffer;
    struct rvid_buffer emb_buffer;

    struct pipe_fence_handle *process_fence;

    /* VPE HW version */
    uint8_t ver_major;
    uint8_t ver_minor;

    struct vpe *vpe_handle;
    struct vpe_init_data vpe_data;
    struct vpe_build_bufs *vpe_build_bufs;
    struct vpe_build_param *vpe_build_param;

    uint8_t log_level;

    struct pipe_surface **src_surfaces;
    struct pipe_surface **dst_surfaces;
};

struct pipe_video_codec*
si_vpe_create_processor(struct pipe_context *context,
                        const struct pipe_video_codec *templ);

#endif
