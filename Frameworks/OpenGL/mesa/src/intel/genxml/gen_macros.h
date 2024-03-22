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

#ifndef GEN_MACROS_H
#define GEN_MACROS_H

/* Macros for handling per-gen compilation.
 *
 * The prefixing macros GENX() and genX() automatically prefix whatever you
 * give them by GENX_ or genX_  where X is the gen number.
 *
 * You can do pseudo-runtime checks in your function such as
 *
 * if (GFX_VERx10 == 75) {
 *    // Do something
 * }
 *
 * The contents of the if statement must be valid regardless of gen, but
 * the if will get compiled away on everything except haswell.
 *
 * For places where you really do have a compile-time conflict, you can
 * use preprocessor logic:
 *
 * #if (GFX_VERx10 == 75)
 *    // Do something
 * #endif
 *
 * However, it is strongly recommended that the former be used whenever
 * possible.
 */

/* Base macro defined on the command line.  If we don't have this, we can't
 * do anything.
 */
#ifndef GFX_VERx10
#  error "The GFX_VERx10 macro must be defined"
#endif

#define GFX_VER ((GFX_VERx10) / 10)

/* Prefixing macros */
#if (GFX_VERx10 == 40)
#  define GENX(X) GFX4_##X
#  define genX(x) gfx4_##x
#elif (GFX_VERx10 == 45)
#  define GENX(X) GFX45_##X
#  define genX(x) gfx45_##x
#elif (GFX_VERx10 == 50)
#  define GENX(X) GFX5_##X
#  define genX(x) gfx5_##x
#elif (GFX_VERx10 == 60)
#  define GENX(X) GFX6_##X
#  define genX(x) gfx6_##x
#elif (GFX_VERx10 == 70)
#  define GENX(X) GFX7_##X
#  define genX(x) gfx7_##x
#elif (GFX_VERx10 == 75)
#  define GENX(X) GFX75_##X
#  define genX(x) gfx75_##x
#elif (GFX_VERx10 == 80)
#  define GENX(X) GFX8_##X
#  define genX(x) gfx8_##x
#elif (GFX_VERx10 == 90)
#  define GENX(X) GFX9_##X
#  define genX(x) gfx9_##x
#elif (GFX_VERx10 == 110)
#  define GENX(X) GFX11_##X
#  define genX(x) gfx11_##x
#elif (GFX_VERx10 == 120)
#  define GENX(X) GFX12_##X
#  define genX(x) gfx12_##x
#elif (GFX_VERx10 == 125)
#  define GENX(X) GFX125_##X
#  define genX(x) gfx125_##x
#elif (GFX_VERx10 == 200)
#  define GENX(X) GFX20_##X
#  define genX(x) gfx20_##x
#else
#  error "Need to add prefixing macros for this gen"
#endif

#endif /* GEN_MACROS_H */
