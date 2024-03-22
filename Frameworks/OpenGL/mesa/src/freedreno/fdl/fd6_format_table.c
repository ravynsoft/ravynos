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

#include "util/format/u_format.h"

#include "fd6_format_table.h"

/* Specifies the table of all the formats and their features. Also supplies
 * the helpers that look up various data in those tables.
 */

struct fd6_format {
   enum a6xx_format vtx;
   enum a6xx_format tex;
   enum a6xx_format rb;
   enum a3xx_color_swap swap;
   bool present;
};

#define FMT(pipe, vtxfmt, texfmt, rbfmt, swapfmt)                              \
   [PIPE_FORMAT_##pipe] = {.present = 1,                                       \
                           .vtx = FMT6_##vtxfmt,                               \
                           .tex = FMT6_##texfmt,                               \
                           .rb = FMT6_##rbfmt,                                 \
                           .swap = swapfmt}

/* vertex + texture + color */
#define VTC(pipe, fmt, swapfmt) FMT(pipe, fmt, fmt, fmt, swapfmt)

#define _TC(pipe, fmt, swapfmt) FMT(pipe, NONE, fmt, fmt, swapfmt)
#define _T_(pipe, fmt, swapfmt) FMT(pipe, NONE, fmt, NONE, swapfmt)
#define VT_(pipe, fmt, swapfmt) FMT(pipe, fmt, fmt, NONE, swapfmt)
#define V__(pipe, fmt, swapfmt) FMT(pipe, fmt, NONE, NONE, swapfmt)

/* clang-format off */
static const struct fd6_format formats[PIPE_FORMAT_COUNT] = {
   /* 8-bit */
   VTC(R8_UNORM,   8_UNORM,                     WZYX),
   VTC(R8_SNORM,   8_SNORM,                     WZYX),
   VTC(R8_UINT,    8_UINT,                      WZYX),
   VTC(R8_SINT,    8_SINT,                      WZYX),
   V__(R8_USCALED, 8_UINT,                      WZYX),
   V__(R8_SSCALED, 8_SINT,                      WZYX),
   _TC(R8_SRGB,    8_UNORM,                     WZYX),
   _TC(Y8_UNORM,   NV12_Y,                      WZYX),

   _TC(A8_UNORM,   A8_UNORM,                    WZYX),
   _TC(L8_UNORM,   8_UNORM,                     WZYX),
   _TC(L8_SRGB,    8_UNORM,                     WZYX),
   _TC(L8_SNORM,   8_SNORM,                     WZYX),
   _T_(I8_UNORM,   8_UNORM,                     WZYX),
   _T_(I8_SNORM,   8_SNORM,                     WZYX),

   _T_(A8_UINT,    8_UINT,                      WZYX),
   _T_(A8_SINT,    8_SINT,                      WZYX),
   _T_(L8_UINT,    8_UINT,                      WZYX),
   _T_(L8_SINT,    8_SINT,                      WZYX),
   _T_(I8_UINT,    8_UINT,                      WZYX),
   _T_(I8_SINT,    8_SINT,                      WZYX),

   _TC(S8_UINT,    8_UINT,                      WZYX),

   /* 16-bit */
   VTC(R16_UNORM,   16_UNORM,                   WZYX),
   VTC(R16_SNORM,   16_SNORM,                   WZYX),
   VTC(R16_UINT,    16_UINT,                    WZYX),
   VTC(R16_SINT,    16_SINT,                    WZYX),
   V__(R16_USCALED, 16_UINT,                    WZYX),
   V__(R16_SSCALED, 16_SINT,                    WZYX),
   VTC(R16_FLOAT,   16_FLOAT,                   WZYX),
   _TC(Z16_UNORM,   16_UNORM,                   WZYX),

   _T_(A16_UNORM,   16_UNORM,                   WZYX),
   _T_(A16_SNORM,   16_SNORM,                   WZYX),
   _T_(A16_UINT,    16_UINT,                    WZYX),
   _T_(A16_SINT,    16_SINT,                    WZYX),
   _T_(A16_FLOAT,   16_FLOAT,                   WZYX),
   _T_(L16_UNORM,   16_UNORM,                   WZYX),
   _T_(L16_SNORM,   16_SNORM,                   WZYX),
   _T_(L16_UINT,    16_UINT,                    WZYX),
   _T_(L16_SINT,    16_SINT,                    WZYX),
   _T_(L16_FLOAT,   16_FLOAT,                   WZYX),
   _T_(I16_UNORM,   16_UNORM,                   WZYX),
   _T_(I16_SNORM,   16_SNORM,                   WZYX),
   _T_(I16_UINT,    16_UINT,                    WZYX),
   _T_(I16_SINT,    16_SINT,                    WZYX),
   _T_(I16_FLOAT,   16_FLOAT,                   WZYX),

   VTC(R8G8_UNORM,   8_8_UNORM,                 WZYX),
   VTC(R8G8_SNORM,   8_8_SNORM,                 WZYX),
   VTC(R8G8_UINT,    8_8_UINT,                  WZYX),
   VTC(R8G8_SINT,    8_8_SINT,                  WZYX),
   V__(R8G8_USCALED, 8_8_UINT,                  WZYX),
   V__(R8G8_SSCALED, 8_8_SINT,                  WZYX),
   _TC(R8G8_SRGB,    8_8_UNORM,                 WZYX),

   _T_(L8A8_UNORM,   8_8_UNORM,                 WZYX),
   _T_(L8A8_UINT,    8_8_UINT,                  WZYX),
   _T_(L8A8_SINT,    8_8_SINT,                  WZYX),

   _TC(R5G6B5_UNORM,   5_6_5_UNORM,             WZYX),
   _TC(B5G6R5_UNORM,   5_6_5_UNORM,             WXYZ),

   _TC(R5G5B5A1_UNORM, 5_5_5_1_UNORM,           WZYX),
   _TC(B5G5R5A1_UNORM, 5_5_5_1_UNORM,           WXYZ),
   _TC(B5G5R5X1_UNORM, 5_5_5_1_UNORM,           WXYZ),
   _TC(A1R5G5B5_UNORM, 5_5_5_1_UNORM,           ZYXW),
   _TC(A1B5G5R5_UNORM, 5_5_5_1_UNORM,           XYZW),

   _TC(R4G4B4A4_UNORM, 4_4_4_4_UNORM,           WZYX),
   _TC(B4G4R4A4_UNORM, 4_4_4_4_UNORM,           WXYZ),
   _TC(A4R4G4B4_UNORM, 4_4_4_4_UNORM,           ZYXW),
   _TC(A4B4G4R4_UNORM, 4_4_4_4_UNORM,           XYZW),

   /* 24-bit */
   VT_(R8G8B8_UNORM,   8_8_8_UNORM,             WZYX),
   VT_(R8G8B8_SNORM,   8_8_8_SNORM,             WZYX),
   VT_(R8G8B8_UINT,    8_8_8_UINT,              WZYX),
   VT_(R8G8B8_SINT,    8_8_8_SINT,              WZYX),
   V__(R8G8B8_USCALED, 8_8_8_UINT,              WZYX),
   V__(R8G8B8_SSCALED, 8_8_8_SINT,              WZYX),

   /* 32-bit */
   V__(R32_UNORM,   32_UNORM,                   WZYX),
   V__(R32_SNORM,   32_SNORM,                   WZYX),
   VTC(R32_UINT,    32_UINT,                    WZYX),
   VTC(R32_SINT,    32_SINT,                    WZYX),
   V__(R32_USCALED, 32_UINT,                    WZYX),
   V__(R32_SSCALED, 32_SINT,                    WZYX),
   VTC(R32_FLOAT,   32_FLOAT,                   WZYX),
   V__(R32_FIXED,   32_FIXED,                   WZYX),

   _T_(A32_UINT,    32_UINT,                    WZYX),
   _T_(A32_SINT,    32_SINT,                    WZYX),
   _T_(A32_FLOAT,   32_FLOAT,                   WZYX),
   _T_(L32_UINT,    32_UINT,                    WZYX),
   _T_(L32_SINT,    32_SINT,                    WZYX),
   _T_(L32_FLOAT,   32_FLOAT,                   WZYX),
   _T_(I32_UINT,    32_UINT,                    WZYX),
   _T_(I32_SINT,    32_SINT,                    WZYX),
   _T_(I32_FLOAT,   32_FLOAT,                   WZYX),

   VTC(R16G16_UNORM,   16_16_UNORM,             WZYX),
   VTC(R16G16_SNORM,   16_16_SNORM,             WZYX),
   VTC(R16G16_UINT,    16_16_UINT,              WZYX),
   VTC(R16G16_SINT,    16_16_SINT,              WZYX),
   V__(R16G16_USCALED, 16_16_UINT,              WZYX),
   V__(R16G16_SSCALED, 16_16_SINT,              WZYX),
   VTC(R16G16_FLOAT,   16_16_FLOAT,             WZYX),

   _T_(L16A16_UNORM,   16_16_UNORM,             WZYX),
   _T_(L16A16_SNORM,   16_16_SNORM,             WZYX),
   _T_(L16A16_UINT,    16_16_UINT,              WZYX),
   _T_(L16A16_SINT,    16_16_SINT,              WZYX),
   _T_(L16A16_FLOAT,   16_16_FLOAT,             WZYX),

   VTC(R8G8B8A8_UNORM,   8_8_8_8_UNORM,         WZYX),
   _TC(R8G8B8X8_UNORM,   8_8_8_8_UNORM,         WZYX),
   _TC(R8G8B8A8_SRGB,    8_8_8_8_UNORM,         WZYX),
   _TC(R8G8B8X8_SRGB,    8_8_8_8_UNORM,         WZYX),
   VTC(R8G8B8A8_SNORM,   8_8_8_8_SNORM,         WZYX),
   VTC(R8G8B8A8_UINT,    8_8_8_8_UINT,          WZYX),
   VTC(R8G8B8A8_SINT,    8_8_8_8_SINT,          WZYX),
   V__(R8G8B8A8_USCALED, 8_8_8_8_UINT,          WZYX),
   V__(R8G8B8A8_SSCALED, 8_8_8_8_SINT,          WZYX),

   VTC(B8G8R8A8_UNORM,   8_8_8_8_UNORM,         WXYZ),
   _TC(B8G8R8X8_UNORM,   8_8_8_8_UNORM,         WXYZ),
   _TC(B8G8R8A8_SRGB,    8_8_8_8_UNORM,         WXYZ),
   _TC(B8G8R8X8_SRGB,    8_8_8_8_UNORM,         WXYZ),
   VTC(B8G8R8A8_SNORM,   8_8_8_8_SNORM,         WXYZ),
   VTC(B8G8R8A8_UINT,    8_8_8_8_UINT,          WXYZ),
   VTC(B8G8R8A8_SINT,    8_8_8_8_SINT,          WXYZ),
   V__(B8G8R8A8_USCALED, 8_8_8_8_UINT,          WXYZ),
   V__(B8G8R8A8_SSCALED, 8_8_8_8_SINT,          WXYZ),

   VTC(A8B8G8R8_UNORM,   8_8_8_8_UNORM,         XYZW),
   _TC(X8B8G8R8_UNORM,   8_8_8_8_UNORM,         XYZW),
   _TC(A8B8G8R8_SRGB,    8_8_8_8_UNORM,         XYZW),
   _TC(X8B8G8R8_SRGB,    8_8_8_8_UNORM,         XYZW),

   VTC(A8R8G8B8_UNORM,   8_8_8_8_UNORM,         ZYXW),
   _TC(X8R8G8B8_UNORM,   8_8_8_8_UNORM,         ZYXW),
   _TC(A8R8G8B8_SRGB,    8_8_8_8_UNORM,         ZYXW),
   _TC(X8R8G8B8_SRGB,    8_8_8_8_UNORM,         ZYXW),

   FMT(R10G10B10A2_UNORM, 10_10_10_2_UNORM, 10_10_10_2_UNORM, 10_10_10_2_UNORM_DEST, WZYX),
   FMT(B10G10R10A2_UNORM, 10_10_10_2_UNORM, 10_10_10_2_UNORM, 10_10_10_2_UNORM_DEST, WXYZ),
   FMT(B10G10R10X2_UNORM, NONE,             10_10_10_2_UNORM, 10_10_10_2_UNORM_DEST, WXYZ),
   V__(R10G10B10A2_SNORM,   10_10_10_2_SNORM,   WZYX),
   V__(B10G10R10A2_SNORM,   10_10_10_2_SNORM,   WXYZ),
   VTC(R10G10B10A2_UINT,    10_10_10_2_UINT,    WZYX),
   V__(R10G10B10A2_SINT,    10_10_10_2_SINT,    WZYX),
   VTC(B10G10R10A2_UINT,    10_10_10_2_UINT,    WXYZ),
   V__(B10G10R10A2_SINT,    10_10_10_2_SINT,    WXYZ),
   V__(R10G10B10A2_USCALED, 10_10_10_2_UINT,    WZYX),
   V__(B10G10R10A2_USCALED, 10_10_10_2_UINT,    WXYZ),
   V__(R10G10B10A2_SSCALED, 10_10_10_2_SINT,    WZYX),
   V__(B10G10R10A2_SSCALED, 10_10_10_2_SINT,    WXYZ),

   VTC(R11G11B10_FLOAT, 11_11_10_FLOAT,         WZYX),
   _T_(R9G9B9E5_FLOAT,  9_9_9_E5_FLOAT,         WZYX),

   _TC(Z24X8_UNORM,          Z24_UNORM_S8_UINT, WZYX),
   _TC(X24S8_UINT,           8_8_8_8_UINT,      WZYX),
   _TC(Z24_UNORM_S8_UINT,    Z24_UNORM_S8_UINT, WZYX),
   _TC(Z32_FLOAT,            32_FLOAT,          WZYX),
   _TC(Z32_FLOAT_S8X24_UINT, 32_FLOAT,          WZYX),
   _TC(X32_S8X24_UINT,       8_UINT,            WZYX),

   /* special format for blits: */
   _TC(Z24_UNORM_S8_UINT_AS_R8G8B8A8, Z24_UNORM_S8_UINT_AS_R8G8B8A8, WZYX),

   /* 48-bit */
   VT_(R16G16B16_UNORM,   16_16_16_UNORM,       WZYX),
   VT_(R16G16B16_SNORM,   16_16_16_SNORM,       WZYX),
   VT_(R16G16B16_UINT,    16_16_16_UINT,        WZYX),
   VT_(R16G16B16_SINT,    16_16_16_SINT,        WZYX),
   V__(R16G16B16_USCALED, 16_16_16_UINT,        WZYX),
   V__(R16G16B16_SSCALED, 16_16_16_SINT,        WZYX),
   VT_(R16G16B16_FLOAT,   16_16_16_FLOAT,       WZYX),

   /* 64-bit */
   VTC(R16G16B16A16_UNORM,   16_16_16_16_UNORM, WZYX),
   VTC(R16G16B16X16_UNORM,   16_16_16_16_UNORM, WZYX),
   VTC(R16G16B16A16_SNORM,   16_16_16_16_SNORM, WZYX),
   VTC(R16G16B16X16_SNORM,   16_16_16_16_SNORM, WZYX),
   VTC(R16G16B16A16_UINT,    16_16_16_16_UINT,  WZYX),
   VTC(R16G16B16X16_UINT,    16_16_16_16_UINT,  WZYX),
   VTC(R16G16B16A16_SINT,    16_16_16_16_SINT,  WZYX),
   VTC(R16G16B16X16_SINT,    16_16_16_16_SINT,  WZYX),
   V__(R16G16B16A16_USCALED, 16_16_16_16_UINT,  WZYX),
   V__(R16G16B16A16_SSCALED, 16_16_16_16_SINT,  WZYX),
   VTC(R16G16B16A16_FLOAT,   16_16_16_16_FLOAT, WZYX),
   VTC(R16G16B16X16_FLOAT,   16_16_16_16_FLOAT, WZYX),

   V__(R32G32_UNORM,   32_32_UNORM,             WZYX),
   V__(R32G32_SNORM,   32_32_SNORM,             WZYX),
   VTC(R32G32_UINT,    32_32_UINT,              WZYX),
   VTC(R32G32_SINT,    32_32_SINT,              WZYX),
   V__(R32G32_USCALED, 32_32_UINT,              WZYX),
   V__(R32G32_SSCALED, 32_32_SINT,              WZYX),
   VTC(R32G32_FLOAT,   32_32_FLOAT,             WZYX),
   V__(R32G32_FIXED,   32_32_FIXED,             WZYX),

   _T_(L32A32_UINT,    32_32_UINT,              WZYX),
   _T_(L32A32_SINT,    32_32_SINT,              WZYX),
   _T_(L32A32_FLOAT,   32_32_FLOAT,             WZYX),

   /* 96-bit */
   V__(R32G32B32_UNORM,   32_32_32_UNORM,       WZYX),
   V__(R32G32B32_SNORM,   32_32_32_SNORM,       WZYX),
   VT_(R32G32B32_UINT,    32_32_32_UINT,        WZYX),
   VT_(R32G32B32_SINT,    32_32_32_SINT,        WZYX),
   V__(R32G32B32_USCALED, 32_32_32_UINT,        WZYX),
   V__(R32G32B32_SSCALED, 32_32_32_SINT,        WZYX),
   VT_(R32G32B32_FLOAT,   32_32_32_FLOAT,       WZYX),
   V__(R32G32B32_FIXED,   32_32_32_FIXED,       WZYX),

   /* 128-bit */
   V__(R32G32B32A32_UNORM,   32_32_32_32_UNORM, WZYX),
   V__(R32G32B32A32_SNORM,   32_32_32_32_SNORM, WZYX),
   VTC(R32G32B32A32_UINT,    32_32_32_32_UINT,  WZYX),
   _TC(R32G32B32X32_UINT,    32_32_32_32_UINT,  WZYX),
   VTC(R32G32B32A32_SINT,    32_32_32_32_SINT,  WZYX),
   _TC(R32G32B32X32_SINT,    32_32_32_32_SINT,  WZYX),
   V__(R32G32B32A32_USCALED, 32_32_32_32_UINT,  WZYX),
   V__(R32G32B32A32_SSCALED, 32_32_32_32_SINT,  WZYX),
   VTC(R32G32B32A32_FLOAT,   32_32_32_32_FLOAT, WZYX),
   _TC(R32G32B32X32_FLOAT,   32_32_32_32_FLOAT, WZYX),
   V__(R32G32B32A32_FIXED,   32_32_32_32_FIXED, WZYX),

   /* compressed */
   _T_(ETC1_RGB8, ETC1,                         WZYX),
   _T_(ETC2_RGB8, ETC2_RGB8,                    WZYX),
   _T_(ETC2_SRGB8, ETC2_RGB8,                   WZYX),
   _T_(ETC2_RGB8A1, ETC2_RGB8A1,                WZYX),
   _T_(ETC2_SRGB8A1, ETC2_RGB8A1,               WZYX),
   _T_(ETC2_RGBA8, ETC2_RGBA8,                  WZYX),
   _T_(ETC2_SRGBA8, ETC2_RGBA8,                 WZYX),
   _T_(ETC2_R11_UNORM, ETC2_R11_UNORM,          WZYX),
   _T_(ETC2_R11_SNORM, ETC2_R11_SNORM,          WZYX),
   _T_(ETC2_RG11_UNORM, ETC2_RG11_UNORM,        WZYX),
   _T_(ETC2_RG11_SNORM, ETC2_RG11_SNORM,        WZYX),

   _T_(DXT1_RGB,   DXT1,                        WZYX),
   _T_(DXT1_SRGB,  DXT1,                        WZYX),
   _T_(DXT1_RGBA,  DXT1,                        WZYX),
   _T_(DXT1_SRGBA, DXT1,                        WZYX),
   _T_(DXT3_RGBA,  DXT3,                        WZYX),
   _T_(DXT3_SRGBA, DXT3,                        WZYX),
   _T_(DXT5_RGBA,  DXT5,                        WZYX),
   _T_(DXT5_SRGBA, DXT5,                        WZYX),

   _T_(BPTC_RGBA_UNORM, BPTC,                   WZYX),
   _T_(BPTC_SRGBA,      BPTC,                   WZYX),
   _T_(BPTC_RGB_FLOAT,  BPTC_FLOAT,             WZYX),
   _T_(BPTC_RGB_UFLOAT, BPTC_UFLOAT,            WZYX),

   _T_(RGTC1_UNORM, RGTC1_UNORM,                WZYX),
   _T_(RGTC1_SNORM, RGTC1_SNORM,                WZYX),
   _T_(RGTC2_UNORM, RGTC2_UNORM,                WZYX),
   _T_(RGTC2_SNORM, RGTC2_SNORM,                WZYX),
   _T_(LATC1_UNORM, RGTC1_UNORM,                WZYX),
   _T_(LATC1_SNORM, RGTC1_SNORM,                WZYX),
   _T_(LATC2_UNORM, RGTC2_UNORM,                WZYX),
   _T_(LATC2_SNORM, RGTC2_SNORM,                WZYX),

   _T_(ASTC_4x4,   ASTC_4x4,                    WZYX),
   _T_(ASTC_5x4,   ASTC_5x4,                    WZYX),
   _T_(ASTC_5x5,   ASTC_5x5,                    WZYX),
   _T_(ASTC_6x5,   ASTC_6x5,                    WZYX),
   _T_(ASTC_6x6,   ASTC_6x6,                    WZYX),
   _T_(ASTC_8x5,   ASTC_8x5,                    WZYX),
   _T_(ASTC_8x6,   ASTC_8x6,                    WZYX),
   _T_(ASTC_8x8,   ASTC_8x8,                    WZYX),
   _T_(ASTC_10x5,  ASTC_10x5,                   WZYX),
   _T_(ASTC_10x6,  ASTC_10x6,                   WZYX),
   _T_(ASTC_10x8,  ASTC_10x8,                   WZYX),
   _T_(ASTC_10x10, ASTC_10x10,                  WZYX),
   _T_(ASTC_12x10, ASTC_12x10,                  WZYX),
   _T_(ASTC_12x12, ASTC_12x12,                  WZYX),

   _T_(ASTC_4x4_SRGB,   ASTC_4x4,               WZYX),
   _T_(ASTC_5x4_SRGB,   ASTC_5x4,               WZYX),
   _T_(ASTC_5x5_SRGB,   ASTC_5x5,               WZYX),
   _T_(ASTC_6x5_SRGB,   ASTC_6x5,               WZYX),
   _T_(ASTC_6x6_SRGB,   ASTC_6x6,               WZYX),
   _T_(ASTC_8x5_SRGB,   ASTC_8x5,               WZYX),
   _T_(ASTC_8x6_SRGB,   ASTC_8x6,               WZYX),
   _T_(ASTC_8x8_SRGB,   ASTC_8x8,               WZYX),
   _T_(ASTC_10x5_SRGB,  ASTC_10x5,              WZYX),
   _T_(ASTC_10x6_SRGB,  ASTC_10x6,              WZYX),
   _T_(ASTC_10x8_SRGB,  ASTC_10x8,              WZYX),
   _T_(ASTC_10x10_SRGB, ASTC_10x10,             WZYX),
   _T_(ASTC_12x10_SRGB, ASTC_12x10,             WZYX),
   _T_(ASTC_12x12_SRGB, ASTC_12x12,             WZYX),

   _T_(R8G8_R8B8_UNORM, R8G8R8B8_422_UNORM, WZYX), /* YUYV */
   _T_(G8R8_B8R8_UNORM, G8R8B8R8_422_UNORM, WZYX), /* UYVY */

   _T_(R8_G8B8_420_UNORM, R8_G8B8_2PLANE_420_UNORM, WZYX), /* Gallium NV12 */
   _T_(G8_B8R8_420_UNORM, R8_G8B8_2PLANE_420_UNORM, WZYX), /* Vulkan NV12 */
   _T_(G8_B8_R8_420_UNORM, R8_G8_B8_3PLANE_420_UNORM, WZYX),
};
/* clang-format on */

static enum a3xx_color_swap
fd6_pipe2swap(enum pipe_format format, enum a6xx_tile_mode tile_mode)
{
   if (!formats[format].present)
      return WZYX;

   /* It seems CCU ignores swap and always uses WZYX when tiled.  TP, on the
    * other hand, always respects swap.  We should return WZYX such that CCU
    * and TP agree each other.
    */
   if (tile_mode)
      return WZYX;

   return formats[format].swap;
}

/* convert pipe format to vertex buffer format: */
enum a6xx_format
fd6_vertex_format(enum pipe_format format)
{
   if (!formats[format].present)
      return FMT6_NONE;
   return formats[format].vtx;
}

enum a3xx_color_swap
fd6_vertex_swap(enum pipe_format format)
{
   return fd6_pipe2swap(format, TILE6_LINEAR);
}

/* convert pipe format to texture sampler format: */
enum a6xx_format
fd6_texture_format(enum pipe_format format, enum a6xx_tile_mode tile_mode)
{
   if (!formats[format].present)
      return FMT6_NONE;

   if (!tile_mode) {
      switch (format) {
      /* Linear ARGB/ABGR1555 has a special format for sampling (tiled
       * 1555/5551 formats always have the same swizzle and layout).
       */
      case PIPE_FORMAT_A1R5G5B5_UNORM:
      case PIPE_FORMAT_A1B5G5R5_UNORM:
         return FMT6_1_5_5_5_UNORM;
      /* note: this may be more about UBWC than tiling, but we don't support
       * tiled non-UBWC NV12
       */
      case PIPE_FORMAT_Y8_UNORM:
         return FMT6_8_UNORM;
      default:
         break;
      }
   }

   return formats[format].tex;
}

enum a3xx_color_swap
fd6_texture_swap(enum pipe_format format, enum a6xx_tile_mode tile_mode)
{
   if (!tile_mode) {
      switch (format) {
      case PIPE_FORMAT_A1R5G5B5_UNORM:
         return WZYX;
      case PIPE_FORMAT_A1B5G5R5_UNORM:
         return WXYZ;
      default:
         break;
      }
   }

   /* format is PIPE_FORMAT_X24S8_UINT when texturing the stencil aspect of
    * PIPE_FORMAT_Z24_UNORM_S8_UINT.  Because we map the format to
    * FMT6_8_8_8_8_UINT, return XYZW such that the stencil value is in X
    * component.
    *
    * We used to return WZYX and apply swizzles.  That required us to
    * un-swizzle the user-specified border color, which could not be done for
    * turnip.
    */
   if (format == PIPE_FORMAT_X24S8_UINT)
      return XYZW;

   return fd6_pipe2swap(format, tile_mode);
}

/* convert pipe format to MRT / copydest format used for render-target: */
enum a6xx_format
fd6_color_format(enum pipe_format format, enum a6xx_tile_mode tile_mode)
{
   if (!formats[format].present)
      return FMT6_NONE;

   if (!tile_mode && format == PIPE_FORMAT_Y8_UNORM)
      return FMT6_8_UNORM;

   return formats[format].rb;
}

enum a3xx_color_swap
fd6_color_swap(enum pipe_format format, enum a6xx_tile_mode tile_mode)
{
   return fd6_pipe2swap(format, tile_mode);
}

enum a6xx_depth_format
fd6_pipe2depth(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      return DEPTH6_16;
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_X8Z24_UNORM:
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      return DEPTH6_24_8;
   case PIPE_FORMAT_Z32_FLOAT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return DEPTH6_32;
   default:
      return ~0;
   }
}
