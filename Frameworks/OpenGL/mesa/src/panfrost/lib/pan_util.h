/**************************************************************************
 *
 * Copyright 2019 Collabora, Ltd.
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
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef PAN_UTIL_H
#define PAN_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include "util/format/u_format.h"

#define PAN_DBG_PERF  0x0001
#define PAN_DBG_TRACE 0x0002
/* 0x4 unused */
#define PAN_DBG_DIRTY 0x0008
#define PAN_DBG_SYNC  0x0010
/* 0x20 unused */
#define PAN_DBG_NOFP16  0x0040
#define PAN_DBG_CRC     0x0080
#define PAN_DBG_GL3     0x0100
#define PAN_DBG_NO_AFBC 0x0200
#define PAN_DBG_MSAA16  0x0400
/* 0x800 unused */
#define PAN_DBG_LINEAR   0x1000
#define PAN_DBG_NO_CACHE 0x2000
#define PAN_DBG_DUMP     0x4000

#ifndef NDEBUG
#define PAN_DBG_OVERFLOW 0x8000
#endif

#define PAN_DBG_YUV        0x20000
#define PAN_DBG_FORCE_PACK 0x40000

struct panfrost_device;
struct pan_blendable_format;

unsigned panfrost_translate_swizzle_4(const unsigned char swizzle[4]);

void panfrost_invert_swizzle(const unsigned char *in, unsigned char *out);

unsigned panfrost_format_to_bifrost_blend(const struct panfrost_device *dev,
                                          enum pipe_format format,
                                          bool dithered);

void pan_pack_color(const struct pan_blendable_format *blendable_formats,
                    uint32_t *packed, const union pipe_color_union *color,
                    enum pipe_format format, bool dithered);

/* Get the last blend shader, for an erratum workaround on v5 */

static inline uint64_t
panfrost_last_nonnull(uint64_t *ptrs, unsigned count)
{
   for (signed i = ((signed)count - 1); i >= 0; --i) {
      if (ptrs[i])
         return ptrs[i];
   }

   return 0;
}

#endif /* PAN_UTIL_H */
