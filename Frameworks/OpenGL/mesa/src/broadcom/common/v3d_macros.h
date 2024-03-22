/*
 * Copyright © 2015 Intel Corporation
 * Copyright © 2015 Broadcom
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

#ifndef V3DX_MACROS_H
#define V3DX_MACROS_H

#ifndef V3D_VERSION
#  error "The V3D_VERSION macro must be defined"
#endif

#if (V3D_VERSION == 21)
#  define V3DX(x) V3D21_##x
#  define v3dX(x) v3d21_##x
#elif (V3D_VERSION == 42)
#  define V3DX(x) V3D42_##x
#  define v3dX(x) v3d42_##x
#elif (V3D_VERSION == 71)
#  define V3DX(x) V3D71_##x
#  define v3dX(x) v3d71_##x
#else
#  error "Need to add prefixing macros for this v3d version"
#endif

#endif /* V3DX_MACROS_H */
