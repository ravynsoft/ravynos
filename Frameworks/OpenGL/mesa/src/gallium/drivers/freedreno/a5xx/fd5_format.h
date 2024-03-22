/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
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

#ifndef FD5_UTIL_H_
#define FD5_UTIL_H_

#include "freedreno_util.h"

#include "a5xx.xml.h"

enum a5xx_vtx_fmt fd5_pipe2vtx(enum pipe_format format);
enum a5xx_tex_fmt fd5_pipe2tex(enum pipe_format format);
enum a5xx_color_fmt fd5_pipe2color(enum pipe_format format);
enum a3xx_color_swap fd5_pipe2swap(enum pipe_format format);
enum a5xx_depth_format fd5_pipe2depth(enum pipe_format format);

uint32_t fd5_tex_swiz(enum pipe_format format, unsigned swizzle_r,
                      unsigned swizzle_g, unsigned swizzle_b,
                      unsigned swizzle_a);

#endif /* FD5_UTIL_H_ */
