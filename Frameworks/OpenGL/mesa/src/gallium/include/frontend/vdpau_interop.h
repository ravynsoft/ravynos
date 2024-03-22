/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
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

/*
 * Authors:
 *      Christian König <christian.koenig@amd.com>
 *
 */

#ifndef _VDPAU_INTEROP_H_
#define _VDPAU_INTEROP_H_

/* driver specific functions for NV_vdpau_interop */
#ifndef VDP_FUNC_ID_BASE_DRIVER
#define VDP_FUNC_ID_BASE_DRIVER 0x2000
#endif

/* Older implementation relying on passing pipe_video_buffer and
 * pipe_resources around. Deprecated and shouldn't be used for new things.
 */
#define VDP_FUNC_ID_VIDEO_SURFACE_GALLIUM (VDP_FUNC_ID_BASE_DRIVER + 0)
#define VDP_FUNC_ID_OUTPUT_SURFACE_GALLIUM (VDP_FUNC_ID_BASE_DRIVER + 1)

struct pipe_resource;
struct pipe_video_buffer;

typedef struct pipe_video_buffer *VdpVideoSurfaceGallium(uint32_t surface);
typedef struct pipe_resource *VdpOutputSurfaceGallium(uint32_t surface);

#endif /* _VDPAU_INTEROP_H_ */
