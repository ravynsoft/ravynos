/*
 * Copyright © 2015 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef BRW_GFX_VER_ENUM_H
#define BRW_GFX_VER_ENUM_H

#include "util/macros.h"
#include "dev/intel_device_info.h"

enum gfx_ver {
   GFX4    = (1 << 0),
   GFX45   = (1 << 1),
   GFX5    = (1 << 2),
   GFX6    = (1 << 3),
   GFX7    = (1 << 4),
   GFX75   = (1 << 5),
   GFX8    = (1 << 6),
   GFX9    = (1 << 7),
   GFX10   = (1 << 8),
   GFX11   = (1 << 9),
   GFX12   = (1 << 10),
   GFX125  = (1 << 11),
   XE2     = (1 << 12),
   GFX_ALL = ~0
};

#define GFX_LT(ver) ((ver) - 1)
#define GFX_GE(ver) (~GFX_LT(ver))
#define GFX_LE(ver) (GFX_LT(ver) | (ver))

static inline enum gfx_ver
gfx_ver_from_devinfo(const struct intel_device_info *devinfo)
{
   switch (devinfo->verx10) {
   case 40: return GFX4;
   case 45: return GFX45;
   case 50: return GFX5;
   case 60: return GFX6;
   case 70: return GFX7;
   case 75: return GFX75;
   case 80: return GFX8;
   case 90: return GFX9;
   case 110: return GFX11;
   case 120: return GFX12;
   case 125: return GFX125;
   case 200: return XE2;
   default:
      unreachable("not reached");
   }
}

#endif
