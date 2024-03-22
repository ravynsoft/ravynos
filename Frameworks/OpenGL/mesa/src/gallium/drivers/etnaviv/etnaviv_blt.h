/*
 * Copyright (c) 2017 Etnaviv Project
 * Copyright (C) 2017 Zodiac Inflight Innovations
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
#ifndef H_ETNAVIV_BLT
#define H_ETNAVIV_BLT

#include "etnaviv_tiling.h"

#include <stdbool.h>
#include "drm/etnaviv_drmif.h"

struct pipe_context;

/* src/dest info for image operations */
struct blt_imginfo
{
   unsigned downsample_x : 1; /* Downsample in x direction */
   unsigned downsample_y : 1; /* Downsample in y direction */
   unsigned use_ts:1;
   struct etna_reloc addr;
   struct etna_reloc ts_addr;
   uint32_t format; /* BLT_FORMAT_* */
   uint32_t stride;
   enum etna_surface_layout tiling; /* ETNA_LAYOUT_* */
   uint32_t ts_clear_value[2];
   uint8_t swizzle[4]; /* TEXTURE_SWIZZLE_* */
   uint8_t ts_mode; /* TS_MODE_* */
   int8_t ts_compress_fmt; /* COLOR_COMPRESSION_FORMAT_* */
   uint8_t endian_mode; /* ENDIAN_MODE_* */
   uint8_t bpp; /* # bytes per pixel 1/2/4/8 - only used for CLEAR_IMAGE */
};

/** (Partial) image clear operation.
 */
struct blt_clear_op
{
   struct blt_imginfo dest;
   uint32_t clear_value[2];
   uint32_t clear_bits[2]; /* bit mask of bits to clear */
   uint16_t rect_x;
   uint16_t rect_y;
   uint16_t rect_w;
   uint16_t rect_h;
};

/** Copy image operation.
 */
struct blt_imgcopy_op
{
   unsigned flip_y:1;
   struct blt_imginfo src;
   struct blt_imginfo dest;
   uint16_t src_x;
   uint16_t src_y;
   uint16_t dest_x;
   uint16_t dest_y;
   uint16_t rect_w;
   uint16_t rect_h;
};

/** Resolve-in-place operation.
 * Fills unfilled tiles.
 */
struct blt_inplace_op
{
   struct etna_reloc addr;
   struct etna_reloc ts_addr;
   uint32_t ts_clear_value[2];
   uint32_t num_tiles;
   uint8_t ts_mode; /* TS_MODE_* */
   uint8_t bpp;
};

/* Context initialization for BLT clear_blit functions. */
void
etna_clear_blit_blt_init(struct pipe_context *pctx);

#endif
