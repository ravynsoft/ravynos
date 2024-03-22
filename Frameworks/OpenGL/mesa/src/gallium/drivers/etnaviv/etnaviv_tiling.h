/*
 * Copyright (c) 2012-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#ifndef H_ETNAVIV_TILING
#define H_ETNAVIV_TILING

#include <stdint.h>

/* texture or surface layout */
enum etna_surface_layout {
   ETNA_LAYOUT_BIT_TILE = (1 << 0),
   ETNA_LAYOUT_BIT_SUPER = (1 << 1),
   ETNA_LAYOUT_BIT_MULTI = (1 << 2),
   ETNA_LAYOUT_LINEAR = 0,
   ETNA_LAYOUT_TILED = ETNA_LAYOUT_BIT_TILE,
   ETNA_LAYOUT_SUPER_TILED = ETNA_LAYOUT_BIT_TILE | ETNA_LAYOUT_BIT_SUPER,
   ETNA_LAYOUT_MULTI_TILED = ETNA_LAYOUT_BIT_TILE | ETNA_LAYOUT_BIT_MULTI,
   ETNA_LAYOUT_MULTI_SUPERTILED = ETNA_LAYOUT_BIT_TILE | ETNA_LAYOUT_BIT_SUPER | ETNA_LAYOUT_BIT_MULTI,
};

void
etna_texture_tile(void *dest, void *src, unsigned basex, unsigned basey,
                 unsigned dst_stride, unsigned width, unsigned height,
                 unsigned src_stride, unsigned elmtsize);
void
etna_texture_untile(void *dest, void *src, unsigned basex, unsigned basey,
                   unsigned src_stride, unsigned width, unsigned height,
                   unsigned dst_stride, unsigned elmtsize);

/* XXX from/to supertiling (can have different layouts, may be better
 * to leave to RS) */

#endif
