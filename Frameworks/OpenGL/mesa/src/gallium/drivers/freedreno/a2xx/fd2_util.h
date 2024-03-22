/*
 * Copyright (C) 2012-2013 Rob Clark <robclark@freedesktop.org>
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD2_UTIL_H_
#define FD2_UTIL_H_

#include "freedreno_util.h"

#include "a2xx.xml.h"

struct surface_format {
/* If enum is a signed type, 0x7f is out of range. Cast it to avoid warnings. */
#define FMT_INVALID ((enum a2xx_sq_surfaceformat)0x7f)
   enum a2xx_sq_surfaceformat format : 7;
   enum sq_tex_sign sign : 2;
   enum sq_tex_num_format num_format : 1;
   int exp_adjust : 6;
};

struct surface_format fd2_pipe2surface(enum pipe_format format);
enum a2xx_colorformatx fd2_pipe2color(enum pipe_format format);
uint32_t fd2_tex_swiz(enum pipe_format format, unsigned swizzle_r,
                      unsigned swizzle_g, unsigned swizzle_b,
                      unsigned swizzle_a);
uint32_t fd2_vtx_swiz(enum pipe_format format, unsigned swizzle);

/* convert x,y to dword */
static inline uint32_t
xy2d(uint16_t x, uint16_t y)
{
   return ((y & 0x3fff) << 16) | (x & 0x3fff);
}

#endif /* FD2_UTIL_H_ */
