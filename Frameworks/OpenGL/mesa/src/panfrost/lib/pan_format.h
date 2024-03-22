/*
 * Copyright (C) 2008 VMware, Inc.
 * Copyright (C) 2014 Broadcom
 * Copyright (C) 2018-2019 Alyssa Rosenzweig
 * Copyright (C) 2019-2020 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef __PAN_FORMAT_H
#define __PAN_FORMAT_H

#include "genxml/gen_macros.h"

#include "util/format/u_format.h"

/* Formats */

typedef uint32_t mali_pixel_format;

/* pan bind flags */
#define PAN_BIND_DEPTH_STENCIL (1 << 0)
#define PAN_BIND_RENDER_TARGET (1 << 1)
#define PAN_BIND_SAMPLER_VIEW  (1 << 3)
#define PAN_BIND_VERTEX_BUFFER (1 << 4)

struct panfrost_format {
   mali_pixel_format hw;
   unsigned bind;
};

struct pan_blendable_format {
   /* enum mali_color_buffer_internal_format */ uint16_t internal;
   /* enum mali_mfbd_color_format */ uint16_t writeback;

   /* Indexed by the dithered? flag. So _PU first, then _AU */
   mali_pixel_format bifrost[2];
};

extern const struct pan_blendable_format
   panfrost_blendable_formats_v6[PIPE_FORMAT_COUNT];
extern const struct pan_blendable_format
   panfrost_blendable_formats_v7[PIPE_FORMAT_COUNT];
extern const struct pan_blendable_format
   panfrost_blendable_formats_v9[PIPE_FORMAT_COUNT];
extern const struct panfrost_format panfrost_pipe_format_v6[PIPE_FORMAT_COUNT];
extern const struct panfrost_format panfrost_pipe_format_v7[PIPE_FORMAT_COUNT];
extern const struct panfrost_format panfrost_pipe_format_v9[PIPE_FORMAT_COUNT];

/* Helpers to construct swizzles */

#define PAN_V6_SWIZZLE(R, G, B, A)                                             \
   (((MALI_CHANNEL_##R) << 0) | ((MALI_CHANNEL_##G) << 3) |                    \
    ((MALI_CHANNEL_##B) << 6) | ((MALI_CHANNEL_##A) << 9))

static inline unsigned
panfrost_get_default_swizzle(unsigned components)
{
   switch (components) {
   case 1:
      return PAN_V6_SWIZZLE(R, 0, 0, 1);
   case 2:
      return PAN_V6_SWIZZLE(R, G, 0, 1);
   case 3:
      return PAN_V6_SWIZZLE(R, G, B, 1);
   case 4:
      return PAN_V6_SWIZZLE(R, G, B, A);
   default:
      unreachable("Invalid number of components");
   }
}

#if PAN_ARCH == 7
struct pan_decomposed_swizzle {
   /* Component ordering to apply first */
   enum mali_rgb_component_order pre;

   /* Bijective swizzle applied after */
   unsigned char post[4];
};

struct pan_decomposed_swizzle
   GENX(pan_decompose_swizzle)(enum mali_rgb_component_order order);
#endif

#define MALI_SRGB_L (0)
#define MALI_SRGB_S (1)

#if PAN_ARCH <= 6

#define MALI_V6_0000 PAN_V6_SWIZZLE(0, 0, 0, 0)
#define MALI_V6_000R PAN_V6_SWIZZLE(0, 0, 0, R)
#define MALI_V6_0R00 PAN_V6_SWIZZLE(0, R, 0, 0)
#define MALI_V6_0A00 PAN_V6_SWIZZLE(0, A, 0, 0)
#define MALI_V6_AAAA PAN_V6_SWIZZLE(A, A, A, A)
#define MALI_V6_A001 PAN_V6_SWIZZLE(A, 0, 0, 1)
#define MALI_V6_ABG1 PAN_V6_SWIZZLE(A, B, G, 1)
#define MALI_V6_ABGR PAN_V6_SWIZZLE(A, B, G, R)
#define MALI_V6_BGR1 PAN_V6_SWIZZLE(B, G, R, 1)
#define MALI_V6_BGRA PAN_V6_SWIZZLE(B, G, R, A)
#define MALI_V6_GBA1 PAN_V6_SWIZZLE(G, B, A, 1)
#define MALI_V6_GBAR PAN_V6_SWIZZLE(G, B, A, R)
#define MALI_V6_R000 PAN_V6_SWIZZLE(R, 0, 0, 0)
#define MALI_V6_R001 PAN_V6_SWIZZLE(R, 0, 0, 1)
#define MALI_V6_RG01 PAN_V6_SWIZZLE(R, G, 0, 1)
#define MALI_V6_RGB1 PAN_V6_SWIZZLE(R, G, B, 1)
#define MALI_V6_RGBA PAN_V6_SWIZZLE(R, G, B, A)
#define MALI_V6_RRR1 PAN_V6_SWIZZLE(R, R, R, 1)
#define MALI_V6_RRRG PAN_V6_SWIZZLE(R, R, R, G)
#define MALI_V6_RRRR PAN_V6_SWIZZLE(R, R, R, R)
#define MALI_V6_GGGG PAN_V6_SWIZZLE(G, G, G, G)

#define MALI_PACK_FMT(mali, swizzle, srgb)                                     \
   (MALI_V6_##swizzle) | ((MALI_##mali) << 12) | (((MALI_SRGB_##srgb)) << 20)

#else

#define MALI_RGB_COMPONENT_ORDER_R001 MALI_RGB_COMPONENT_ORDER_RGB1
#define MALI_RGB_COMPONENT_ORDER_RG01 MALI_RGB_COMPONENT_ORDER_RGB1
#define MALI_RGB_COMPONENT_ORDER_GBAR MALI_RGB_COMPONENT_ORDER_ARGB
#define MALI_RGB_COMPONENT_ORDER_GBA1 MALI_RGB_COMPONENT_ORDER_1RGB
#define MALI_RGB_COMPONENT_ORDER_ABG1 MALI_RGB_COMPONENT_ORDER_1BGR

#define MALI_PACK_FMT(mali, swizzle, srgb)                                     \
   (MALI_RGB_COMPONENT_ORDER_##swizzle) | ((MALI_##mali) << 12) |              \
      (((MALI_SRGB_##srgb)) << 20)

#endif

#endif
