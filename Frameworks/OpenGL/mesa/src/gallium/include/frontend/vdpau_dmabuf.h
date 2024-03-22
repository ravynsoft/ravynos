/**************************************************************************
 *
 * Copyright 2016 Advanced Micro Devices, Inc.
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

#ifndef _VDPAU_DMABUF_H_
#define _VDPAU_DMABUF_H_

#include <vdpau/vdpau.h>

/* driver specific functions for NV_vdpau_interop */
#ifndef VDP_FUNC_ID_BASE_DRIVER
#define VDP_FUNC_ID_BASE_DRIVER 0x2000
#endif

/* New DMA-buf based implementation */
#define VDP_FUNC_ID_VIDEO_SURFACE_DMA_BUF (VDP_FUNC_ID_BASE_DRIVER + 2)
#define VDP_FUNC_ID_OUTPUT_SURFACE_DMA_BUF (VDP_FUNC_ID_BASE_DRIVER + 3)

/* Define some more internal RGBA formats for more
 * robust handling of Video Surfaces
 */
#define VDP_RGBA_FORMAT_R8          (-1)
#define VDP_RGBA_FORMAT_R8G8        (-2)

struct VdpSurfaceDMABufDesc {
   /* DMA-buf file descriptor */
   uint32_t handle;
   /* Width in pixel */
   uint32_t width;
   /* Height in pixel */
   uint32_t height;
   /* Offset in bytes */
   uint32_t offset;
   /* Stride in bytes */
   uint32_t stride;
   /* VDP_RGBA_FORMAT_* as defined in the VDPAU spec and above. */
   uint32_t format;
};

/**
 * \brief Video surface planes
 */
typedef uint32_t VdpVideoSurfacePlane;

/** \hideinitializer \brief Luma top field */
#define VDP_VIDEO_SURFACE_PLANE_LUMA_TOP      ((VdpVideoSurfacePlane)0)
/** \hideinitializer \brief Luma bottom field */
#define VDP_VIDEO_SURFACE_PLANE_LUMA_BOTTOM   ((VdpVideoSurfacePlane)1)
/** \hideinitializer \brief Chroma top field */
#define VDP_VIDEO_SURFACE_PLANE_CHROMA_TOP    ((VdpVideoSurfacePlane)2)
/** \hideinitializer \brief Chroma bottom field */
#define VDP_VIDEO_SURFACE_PLANE_CHROMA_BOTTOM ((VdpVideoSurfacePlane)3)

typedef VdpStatus VdpVideoSurfaceDMABuf(
   VdpVideoSurface               surface,
   VdpVideoSurfacePlane          plane,
   struct VdpSurfaceDMABufDesc * result
);

typedef VdpStatus VdpOutputSurfaceDMABuf(
   VdpOutputSurface              surface,
   struct VdpSurfaceDMABufDesc * result
);

#endif /* _VDPAU_DMABUF_H_ */
