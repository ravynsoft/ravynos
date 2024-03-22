/*
 * Copyright © 2023 Intel Corporation
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

#include <string.h>
#include "util/libdrm.h"

#include "intel_kmd.h"

enum intel_kmd_type
intel_get_kmd_type(int fd)
{
   enum intel_kmd_type type = INTEL_KMD_TYPE_INVALID;
   drmVersionPtr version = drmGetVersion(fd);

   if (!version)
      return type;

   if (strcmp(version->name, "i915") == 0)
      type = INTEL_KMD_TYPE_I915;
#ifdef INTEL_XE_KMD_SUPPORTED
   else if (strcmp(version->name, "xe") == 0)
      type = INTEL_KMD_TYPE_XE;
#endif

   drmFreeVersion(version);
   return type;
}
