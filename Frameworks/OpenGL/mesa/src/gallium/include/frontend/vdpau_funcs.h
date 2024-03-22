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

#ifndef _VDPAU_FUNCS_H_
#define _VDPAU_FUNCS_H_

#include "vdpau_dmabuf.h"

/* Used for implementing NV_vdpau_interop */
static inline enum pipe_format
VdpFormatRGBAToPipe(uint32_t vdpau_format)
{
   switch (vdpau_format) {
   case VDP_RGBA_FORMAT_R8:
      return PIPE_FORMAT_R8_UNORM;
   case VDP_RGBA_FORMAT_R8G8:
      return PIPE_FORMAT_R8G8_UNORM;
   case VDP_RGBA_FORMAT_A8:
      return PIPE_FORMAT_A8_UNORM;
   case VDP_RGBA_FORMAT_B10G10R10A2:
      return PIPE_FORMAT_B10G10R10A2_UNORM;
   case VDP_RGBA_FORMAT_B8G8R8A8:
      return PIPE_FORMAT_B8G8R8A8_UNORM;
   case VDP_RGBA_FORMAT_R10G10B10A2:
      return PIPE_FORMAT_R10G10B10A2_UNORM;
   case VDP_RGBA_FORMAT_R8G8B8A8:
      return PIPE_FORMAT_R8G8B8A8_UNORM;
   default:
      assert(0);
   }

   return PIPE_FORMAT_NONE;
}

#endif /* _VDPAU_FUNCS_H_ */
