/*
 * Mesa 3-D graphics library
 *
 * Copyright 2012 Intel Corporation
 * Copyright 2013 Google
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chad Versace <chad.versace@linux.intel.com>
 *    Frank Henigman <fjhenigman@google.com>
 */


#include "isl_tiled_memcpy.c"

void
_isl_memcpy_linear_to_tiled(uint32_t xt1, uint32_t xt2,
                            uint32_t yt1, uint32_t yt2,
                            char *dst, const char *src,
                            uint32_t dst_pitch, int32_t src_pitch,
                            bool has_swizzling,
                            enum isl_tiling tiling,
                            isl_memcpy_type copy_type)
{
   linear_to_tiled(xt1, xt2, yt1, yt2, dst, src, dst_pitch, src_pitch,
                   has_swizzling, tiling, copy_type);
}

void
_isl_memcpy_tiled_to_linear(uint32_t xt1, uint32_t xt2,
                            uint32_t yt1, uint32_t yt2,
                            char *dst, const char *src,
                            int32_t dst_pitch, uint32_t src_pitch,
                            bool has_swizzling,
                            enum isl_tiling tiling,
                            isl_memcpy_type copy_type)
{
   tiled_to_linear(xt1, xt2, yt1, yt2, dst, src, dst_pitch, src_pitch,
                   has_swizzling, tiling, copy_type);
}
