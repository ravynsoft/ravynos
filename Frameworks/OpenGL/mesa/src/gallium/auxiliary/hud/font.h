/**************************************************************************
 *
 * Copyright 2013 Marek Olšák <maraeo@gmail.com>
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef FONT_H
#define FONT_H

#include "util/compiler.h"

struct pipe_resource;
struct pipe_context;

enum util_font_name {
   UTIL_FONT_FIXED_8X13
};

/* The font is stored in a 2D texture. There are 256 glyphs
 * drawn in a 16x16 matrix. The texture coordinates of a glyph
 * within the matrix should be calculated as follows:
 *
 *    x1 = (glyph % 16) * glyph_width;
 *    y1 = (glyph / 16) * glyph_height;
 *    x2 = x1 + glyph_width;
 *    y2 = y1 + glyph_height;
 */
struct util_font {
   struct pipe_resource *texture;
   unsigned glyph_width;
   unsigned glyph_height;
};

bool
util_font_create(struct pipe_context *pipe, enum util_font_name name,
                         struct util_font *out_font);

#endif
