/**************************************************************************
 *
 * Copyright 2016 Ilia Mirkin.
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

#ifndef U_VIEWPORT_H
#define U_VIEWPORT_H

#include "pipe/p_state.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void
util_viewport_zmin_zmax(const struct pipe_viewport_state *vp, bool halfz,
                        float *zmin, float *zmax)
{
   float a, b;
   if (halfz) {
      a = vp->translate[2];
      b = vp->translate[2] + vp->scale[2];
   } else {
      a = vp->translate[2] - vp->scale[2];
      b = vp->translate[2] + vp->scale[2];
   }

   *zmin = a < b ? a : b;
   *zmax = a < b ? b : a;
}

#ifdef __cplusplus
}
#endif

#endif
