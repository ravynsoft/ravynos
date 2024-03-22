/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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

#ifndef FD6_FORMAT_TABLE_H
#define FD6_FORMAT_TABLE_H

#include "util/format/u_format.h"
#include "util/u_math.h"

#include "common/freedreno_common.h"

#include "adreno_pm4.xml.h"
#include "adreno_common.xml.h"
#include "a6xx.xml.h"

BEGINC;

static inline enum a6xx_tex_swiz
fdl6_swiz(unsigned char swiz)
{
   STATIC_ASSERT((unsigned) A6XX_TEX_X == (unsigned) PIPE_SWIZZLE_X);
   STATIC_ASSERT((unsigned) A6XX_TEX_Y == (unsigned) PIPE_SWIZZLE_Y);
   STATIC_ASSERT((unsigned) A6XX_TEX_Z == (unsigned) PIPE_SWIZZLE_Z);
   STATIC_ASSERT((unsigned) A6XX_TEX_W == (unsigned) PIPE_SWIZZLE_W);
   STATIC_ASSERT((unsigned) A6XX_TEX_ZERO == (unsigned) PIPE_SWIZZLE_0);
   STATIC_ASSERT((unsigned) A6XX_TEX_ONE == (unsigned) PIPE_SWIZZLE_1);
   return (enum a6xx_tex_swiz) swiz;
}

enum a6xx_depth_format fd6_pipe2depth(enum pipe_format format);

enum a6xx_format fd6_vertex_format(enum pipe_format format) ATTRIBUTE_CONST;
enum a3xx_color_swap fd6_vertex_swap(enum pipe_format format) ATTRIBUTE_CONST;
enum a6xx_format fd6_texture_format(enum pipe_format format,
                                    enum a6xx_tile_mode tile_mode) ATTRIBUTE_CONST;
enum a3xx_color_swap fd6_texture_swap(enum pipe_format format,
                                      enum a6xx_tile_mode tile_mode) ATTRIBUTE_CONST;
enum a6xx_format fd6_color_format(enum pipe_format format,
                                  enum a6xx_tile_mode tile_mode) ATTRIBUTE_CONST;
enum a3xx_color_swap fd6_color_swap(enum pipe_format format,
                                    enum a6xx_tile_mode tile_mode) ATTRIBUTE_CONST;

ENDC;

#endif /* FD6_FORMAT_TABLE_H */
