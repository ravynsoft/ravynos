/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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
 */

#include "pipe/p_defines.h"
#include "util/format/u_format.h"

#include "fd3_format.h"

/* Specifies the table of all the formats and their features. Also supplies
 * the helpers that look up various data in those tables.
 */

struct fd3_format {
   enum a3xx_vtx_fmt vtx;
   enum a3xx_tex_fmt tex;
   enum a3xx_color_fmt rb;
   enum a3xx_color_swap swap;
   bool present;
};

/* vertex + texture */
#define VT(pipe, fmt, rbfmt, swapfmt)                                          \
   [PIPE_FORMAT_##pipe] = {.present = 1,                                       \
                           .vtx = VFMT_##fmt,                                  \
                           .tex = TFMT_##fmt,                                  \
                           .rb = RB_##rbfmt,                                   \
                           .swap = swapfmt}

/* texture-only */
#define _T(pipe, fmt, rbfmt, swapfmt)                                          \
   [PIPE_FORMAT_##pipe] = {.present = 1,                                       \
                           .vtx = VFMT_NONE,                                   \
                           .tex = TFMT_##fmt,                                  \
                           .rb = RB_##rbfmt,                                   \
                           .swap = swapfmt}

/* vertex-only */
#define V_(pipe, fmt, rbfmt, swapfmt)                                          \
   [PIPE_FORMAT_##pipe] = {.present = 1,                                       \
                           .vtx = VFMT_##fmt,                                  \
                           .tex = TFMT_NONE,                                   \
                           .rb = RB_##rbfmt,                                   \
                           .swap = swapfmt}

/* clang-format off */
static struct fd3_format formats[PIPE_FORMAT_COUNT] = {
   /* 8-bit */
   VT(R8_UNORM,   8_UNORM, R8_UNORM, WZYX),
   _T(R8_SRGB,    8_UNORM, R8_UNORM, WZYX),
   VT(R8_SNORM,   8_SNORM, NONE,     WZYX),
   VT(R8_UINT,    8_UINT,  NONE,     WZYX),
   VT(R8_SINT,    8_SINT,  NONE,     WZYX),
   V_(R8_USCALED, 8_UINT,  NONE,     WZYX),
   V_(R8_SSCALED, 8_SINT,  NONE,     WZYX),

   _T(A8_UNORM,   8_UNORM, A8_UNORM, WZYX),
   _T(L8_UNORM,   8_UNORM, R8_UNORM, WZYX),
   _T(I8_UNORM,   8_UNORM, NONE,     WZYX),

   _T(A8_UINT,    8_UINT,  NONE,     WZYX),
   _T(A8_SINT,    8_SINT,  NONE,     WZYX),
   _T(L8_UINT,    8_UINT,  NONE,     WZYX),
   _T(L8_SINT,    8_SINT,  NONE,     WZYX),
   _T(I8_UINT,    8_UINT,  NONE,     WZYX),
   _T(I8_SINT,    8_SINT,  NONE,     WZYX),

   _T(S8_UINT,    8_UINT,  R8_UNORM, WZYX),

   /* 16-bit */
   VT(R16_UNORM,   16_UNORM, NONE,     WZYX),
   VT(R16_SNORM,   16_SNORM, NONE,     WZYX),
   VT(R16_UINT,    16_UINT,  R16_UINT, WZYX),
   VT(R16_SINT,    16_SINT,  R16_SINT, WZYX),
   V_(R16_USCALED, 16_UINT,  NONE,     WZYX),
   V_(R16_SSCALED, 16_SINT,  NONE,     WZYX),
   VT(R16_FLOAT,   16_FLOAT, R16_FLOAT,WZYX),

   _T(A16_UNORM,   16_UNORM, NONE,     WZYX),
   _T(A16_SNORM,   16_SNORM, NONE,     WZYX),
   _T(A16_UINT,    16_UINT,  NONE,     WZYX),
   _T(A16_SINT,    16_SINT,  NONE,     WZYX),
   _T(A16_FLOAT,   16_FLOAT, NONE,     WZYX),
   _T(L16_UNORM,   16_UNORM, NONE,     WZYX),
   _T(L16_SNORM,   16_SNORM, NONE,     WZYX),
   _T(L16_UINT,    16_UINT,  NONE,     WZYX),
   _T(L16_SINT,    16_SINT,  NONE,     WZYX),
   _T(L16_FLOAT,   16_FLOAT, NONE,     WZYX),
   _T(I16_UNORM,   16_UNORM, NONE,     WZYX),
   _T(I16_SNORM,   16_SNORM, NONE,     WZYX),
   _T(I16_UINT,    16_UINT,  NONE,     WZYX),
   _T(I16_SINT,    16_SINT,  NONE,     WZYX),
   _T(I16_FLOAT,   16_FLOAT, NONE,     WZYX),

   VT(R8G8_UNORM,   8_8_UNORM, R8G8_UNORM, WZYX),
   _T(R8G8_SRGB,    8_8_UNORM, R8G8_UNORM, WZYX),
   VT(R8G8_SNORM,   8_8_SNORM, R8G8_SNORM, WZYX),
   VT(R8G8_UINT,    8_8_UINT,  R8G8_UINT,  WZYX),
   VT(R8G8_SINT,    8_8_SINT,  R8G8_SINT,  WZYX),
   V_(R8G8_USCALED, 8_8_UINT,  NONE,       WZYX),
   V_(R8G8_SSCALED, 8_8_SINT,  NONE,       WZYX),

   _T(L8A8_UNORM,   8_8_UNORM, NONE,       WZYX),
   _T(L8A8_UINT,    8_8_UINT,  NONE,       WZYX),
   _T(L8A8_SINT,    8_8_SINT,  NONE,       WZYX),

   _T(Z16_UNORM,      Z16_UNORM,     R8G8_UNORM,     WZYX),
   _T(B5G6R5_UNORM,   5_6_5_UNORM,   R5G6B5_UNORM,   WXYZ),
   _T(B5G5R5A1_UNORM, 5_5_5_1_UNORM, R5G5B5A1_UNORM, WXYZ),
   _T(B5G5R5X1_UNORM, 5_5_5_1_UNORM, R5G5B5A1_UNORM, WXYZ),
   _T(B4G4R4A4_UNORM, 4_4_4_4_UNORM, R4G4B4A4_UNORM, WXYZ),

   /* 24-bit */
   V_(R8G8B8_UNORM,   8_8_8_UNORM, NONE, WZYX),
   V_(R8G8B8_SNORM,   8_8_8_SNORM, NONE, WZYX),
   V_(R8G8B8_UINT,    8_8_8_UINT,  NONE, WZYX),
   V_(R8G8B8_SINT,    8_8_8_SINT,  NONE, WZYX),
   V_(R8G8B8_USCALED, 8_8_8_UINT,  NONE, WZYX),
   V_(R8G8B8_SSCALED, 8_8_8_SINT,  NONE, WZYX),

   /* 32-bit */
   VT(R32_UINT,    32_UINT,  R32_UINT, WZYX),
   VT(R32_SINT,    32_SINT,  R32_SINT, WZYX),
   V_(R32_USCALED, 32_UINT,  NONE,     WZYX),
   V_(R32_SSCALED, 32_SINT,  NONE,     WZYX),
   VT(R32_FLOAT,   32_FLOAT, R32_FLOAT,WZYX),
   V_(R32_FIXED,   32_FIXED, NONE,     WZYX),

   _T(A32_UINT,    32_UINT,  NONE,     WZYX),
   _T(A32_SINT,    32_SINT,  NONE,     WZYX),
   _T(A32_FLOAT,   32_FLOAT, NONE,     WZYX),
   _T(L32_UINT,    32_UINT,  NONE,     WZYX),
   _T(L32_SINT,    32_SINT,  NONE,     WZYX),
   _T(L32_FLOAT,   32_FLOAT, NONE,     WZYX),
   _T(I32_UINT,    32_UINT,  NONE,     WZYX),
   _T(I32_SINT,    32_SINT,  NONE,     WZYX),
   _T(I32_FLOAT,   32_FLOAT, NONE,     WZYX),

   VT(R16G16_UNORM,   16_16_UNORM, NONE,        WZYX),
   VT(R16G16_SNORM,   16_16_SNORM, NONE,        WZYX),
   VT(R16G16_UINT,    16_16_UINT,  R16G16_UINT, WZYX),
   VT(R16G16_SINT,    16_16_SINT,  R16G16_SINT, WZYX),
   V_(R16G16_USCALED, 16_16_UINT,  NONE,        WZYX),
   V_(R16G16_SSCALED, 16_16_SINT,  NONE,        WZYX),
   VT(R16G16_FLOAT,   16_16_FLOAT, R16G16_FLOAT,WZYX),

   _T(L16A16_UNORM,   16_16_UNORM, NONE,        WZYX),
   _T(L16A16_SNORM,   16_16_SNORM, NONE,        WZYX),
   _T(L16A16_UINT,    16_16_UINT,  NONE,        WZYX),
   _T(L16A16_SINT,    16_16_SINT,  NONE,        WZYX),
   _T(L16A16_FLOAT,   16_16_FLOAT, NONE,        WZYX),

   VT(R8G8B8A8_UNORM,   8_8_8_8_UNORM, R8G8B8A8_UNORM, WZYX),
   _T(R8G8B8X8_UNORM,   8_8_8_8_UNORM, R8G8B8A8_UNORM, WZYX),
   _T(R8G8B8A8_SRGB,    8_8_8_8_UNORM, R8G8B8A8_UNORM, WZYX),
   _T(R8G8B8X8_SRGB,    8_8_8_8_UNORM, R8G8B8A8_UNORM, WZYX),
   VT(R8G8B8A8_SNORM,   8_8_8_8_SNORM, R8G8B8A8_SNORM, WZYX),
   VT(R8G8B8A8_UINT,    8_8_8_8_UINT,  R8G8B8A8_UINT,  WZYX),
   VT(R8G8B8A8_SINT,    8_8_8_8_SINT,  R8G8B8A8_SINT,  WZYX),
   V_(R8G8B8A8_USCALED, 8_8_8_8_UINT,  NONE,           WZYX),
   V_(R8G8B8A8_SSCALED, 8_8_8_8_SINT,  NONE,           WZYX),

   VT(B8G8R8A8_UNORM,   8_8_8_8_UNORM, R8G8B8A8_UNORM, WXYZ),
   _T(B8G8R8X8_UNORM,   8_8_8_8_UNORM, R8G8B8A8_UNORM, WXYZ),
   VT(B8G8R8A8_SRGB,    8_8_8_8_UNORM, R8G8B8A8_UNORM, WXYZ),
   _T(B8G8R8X8_SRGB,    8_8_8_8_UNORM, R8G8B8A8_UNORM, WXYZ),

   VT(A8B8G8R8_UNORM,   8_8_8_8_UNORM, R8G8B8A8_UNORM, XYZW),
   _T(X8B8G8R8_UNORM,   8_8_8_8_UNORM, R8G8B8A8_UNORM, XYZW),
   _T(A8B8G8R8_SRGB,    8_8_8_8_UNORM, R8G8B8A8_UNORM, XYZW),
   _T(X8B8G8R8_SRGB,    8_8_8_8_UNORM, R8G8B8A8_UNORM, XYZW),

   VT(A8R8G8B8_UNORM,   8_8_8_8_UNORM, R8G8B8A8_UNORM, ZYXW),
   _T(X8R8G8B8_UNORM,   8_8_8_8_UNORM, R8G8B8A8_UNORM, ZYXW),
   _T(A8R8G8B8_SRGB,    8_8_8_8_UNORM, R8G8B8A8_UNORM, ZYXW),
   _T(X8R8G8B8_SRGB,    8_8_8_8_UNORM, R8G8B8A8_UNORM, ZYXW),

   VT(R10G10B10A2_UNORM,   10_10_10_2_UNORM, R10G10B10A2_UNORM, WZYX),
   VT(B10G10R10A2_UNORM,   10_10_10_2_UNORM, R10G10B10A2_UNORM, WXYZ),
   _T(B10G10R10X2_UNORM,   10_10_10_2_UNORM, R10G10B10A2_UNORM, WXYZ),
   V_(R10G10B10A2_SNORM,   10_10_10_2_SNORM, NONE,              WZYX),
   V_(B10G10R10A2_SNORM,   10_10_10_2_SNORM, NONE,              WXYZ),
   VT(R10G10B10A2_UINT,    10_10_10_2_UINT,  NONE,              WZYX),
   V_(B10G10R10A2_UINT,    10_10_10_2_UINT,  NONE,              WXYZ),
   V_(R10G10B10A2_USCALED, 10_10_10_2_UINT,  NONE,              WZYX),
   V_(B10G10R10A2_USCALED, 10_10_10_2_UINT,  NONE,              WXYZ),
   V_(R10G10B10A2_SSCALED, 10_10_10_2_SINT,  NONE,              WZYX),
   V_(B10G10R10A2_SSCALED, 10_10_10_2_SINT,  NONE,              WXYZ),

   _T(R11G11B10_FLOAT, 11_11_10_FLOAT, R11G11B10_FLOAT, WZYX),
   _T(R9G9B9E5_FLOAT,  9_9_9_E5_FLOAT, NONE,            WZYX),

   _T(Z24X8_UNORM,       X8Z24_UNORM, R8G8B8A8_UNORM, WZYX),
   _T(Z24_UNORM_S8_UINT, X8Z24_UNORM, R8G8B8A8_UNORM, WZYX),
   _T(Z32_FLOAT,         Z32_FLOAT,   R8G8B8A8_UNORM, WZYX),
   _T(Z32_FLOAT_S8X24_UINT, Z32_FLOAT,R8G8B8A8_UNORM, WZYX),

   /* 48-bit */
   V_(R16G16B16_UNORM,   16_16_16_UNORM, NONE, WZYX),
   V_(R16G16B16_SNORM,   16_16_16_SNORM, NONE, WZYX),
   V_(R16G16B16_UINT,    16_16_16_UINT,  NONE, WZYX),
   V_(R16G16B16_SINT,    16_16_16_SINT,  NONE, WZYX),
   V_(R16G16B16_USCALED, 16_16_16_UINT,  NONE, WZYX),
   V_(R16G16B16_SSCALED, 16_16_16_SINT,  NONE, WZYX),
   V_(R16G16B16_FLOAT,   16_16_16_FLOAT, NONE, WZYX),

   /* 64-bit */
   VT(R16G16B16A16_UNORM,   16_16_16_16_UNORM, NONE,               WZYX),
   VT(R16G16B16A16_SNORM,   16_16_16_16_SNORM, NONE,               WZYX),
   VT(R16G16B16A16_UINT,    16_16_16_16_UINT,  R16G16B16A16_UINT,  WZYX),
   _T(R16G16B16X16_UINT,    16_16_16_16_UINT,  R16G16B16A16_UINT,  WZYX),
   VT(R16G16B16A16_SINT,    16_16_16_16_SINT,  R16G16B16A16_SINT,  WZYX),
   _T(R16G16B16X16_SINT,    16_16_16_16_SINT,  R16G16B16A16_SINT,  WZYX),
   V_(R16G16B16A16_USCALED, 16_16_16_16_UINT,  NONE,               WZYX),
   V_(R16G16B16A16_SSCALED, 16_16_16_16_SINT,  NONE,               WZYX),
   VT(R16G16B16A16_FLOAT,   16_16_16_16_FLOAT, R16G16B16A16_FLOAT, WZYX),
   _T(R16G16B16X16_FLOAT,   16_16_16_16_FLOAT, R16G16B16A16_FLOAT, WZYX),

   VT(R32G32_UINT,    32_32_UINT,  R32G32_UINT, WZYX),
   VT(R32G32_SINT,    32_32_SINT,  R32G32_SINT, WZYX),
   V_(R32G32_USCALED, 32_32_UINT,  NONE,        WZYX),
   V_(R32G32_SSCALED, 32_32_SINT,  NONE,        WZYX),
   VT(R32G32_FLOAT,   32_32_FLOAT, R32G32_FLOAT,WZYX),
   V_(R32G32_FIXED,   32_32_FIXED, NONE,        WZYX),

   _T(L32A32_UINT,    32_32_UINT,  NONE,        WZYX),
   _T(L32A32_SINT,    32_32_SINT,  NONE,        WZYX),
   _T(L32A32_FLOAT,   32_32_FLOAT, NONE,        WZYX),

   /* 96-bit */
   V_(R32G32B32_UINT,    32_32_32_UINT,  NONE, WZYX),
   V_(R32G32B32_SINT,    32_32_32_SINT,  NONE, WZYX),
   V_(R32G32B32_USCALED, 32_32_32_UINT,  NONE, WZYX),
   V_(R32G32B32_SSCALED, 32_32_32_SINT,  NONE, WZYX),
   V_(R32G32B32_FLOAT,   32_32_32_FLOAT, NONE, WZYX),
   V_(R32G32B32_FIXED,   32_32_32_FIXED, NONE, WZYX),

   /* 128-bit */
   VT(R32G32B32A32_UINT,    32_32_32_32_UINT,  R32G32B32A32_UINT,  WZYX),
   _T(R32G32B32X32_UINT,    32_32_32_32_UINT,  R32G32B32A32_UINT,  WZYX),
   VT(R32G32B32A32_SINT,    32_32_32_32_SINT,  R32G32B32A32_SINT,  WZYX),
   _T(R32G32B32X32_SINT,    32_32_32_32_SINT,  R32G32B32A32_SINT,  WZYX),
   V_(R32G32B32A32_USCALED, 32_32_32_32_UINT,  NONE,               WZYX),
   V_(R32G32B32A32_SSCALED, 32_32_32_32_SINT,  NONE,               WZYX),
   VT(R32G32B32A32_FLOAT,   32_32_32_32_FLOAT, R32G32B32A32_FLOAT, WZYX),
   _T(R32G32B32X32_FLOAT,   32_32_32_32_FLOAT, R32G32B32A32_FLOAT, WZYX),
   V_(R32G32B32A32_FIXED,   32_32_32_32_FIXED, NONE,               WZYX),

   /* compressed */
   _T(ETC1_RGB8, ETC1, NONE, WZYX),
   _T(ETC2_RGB8, ETC2_RGB8, NONE, WZYX),
   _T(ETC2_SRGB8, ETC2_RGB8, NONE, WZYX),
   _T(ETC2_RGB8A1, ETC2_RGB8A1, NONE, WZYX),
   _T(ETC2_SRGB8A1, ETC2_RGB8A1, NONE, WZYX),
   _T(ETC2_RGBA8, ETC2_RGBA8, NONE, WZYX),
   _T(ETC2_SRGBA8, ETC2_RGBA8, NONE, WZYX),
   _T(ETC2_R11_UNORM, ETC2_R11_UNORM, NONE, WZYX),
   _T(ETC2_R11_SNORM, ETC2_R11_SNORM, NONE, WZYX),
   _T(ETC2_RG11_UNORM, ETC2_RG11_UNORM, NONE, WZYX),
   _T(ETC2_RG11_SNORM, ETC2_RG11_SNORM, NONE, WZYX),

   _T(DXT1_RGB,   DXT1, NONE, WZYX),
   _T(DXT1_SRGB,  DXT1, NONE, WZYX),
   _T(DXT1_RGBA,  DXT1, NONE, WZYX),
   _T(DXT1_SRGBA, DXT1, NONE, WZYX),
   _T(DXT3_RGBA,  DXT3, NONE, WZYX),
   _T(DXT3_SRGBA, DXT3, NONE, WZYX),
   _T(DXT5_RGBA,  DXT5, NONE, WZYX),
   _T(DXT5_SRGBA, DXT5, NONE, WZYX),

   _T(ATC_RGB,               ATC_RGB,               NONE, WZYX),
   _T(ATC_RGBA_EXPLICIT,     ATC_RGBA_EXPLICIT,     NONE, WZYX),
   _T(ATC_RGBA_INTERPOLATED, ATC_RGBA_INTERPOLATED, NONE, WZYX),
};
/* clang-format on */

enum a3xx_vtx_fmt
fd3_pipe2vtx(enum pipe_format format)
{
   if (!formats[format].present)
      return VFMT_NONE;
   return formats[format].vtx;
}

enum a3xx_tex_fmt
fd3_pipe2tex(enum pipe_format format)
{
   if (!formats[format].present)
      return TFMT_NONE;
   return formats[format].tex;
}

enum a3xx_color_fmt
fd3_pipe2color(enum pipe_format format)
{
   if (!formats[format].present)
      return RB_NONE;
   return formats[format].rb;
}

enum a3xx_color_swap
fd3_pipe2swap(enum pipe_format format)
{
   if (!formats[format].present)
      return WZYX;
   return formats[format].swap;
}

enum a3xx_color_fmt
fd3_fs_output_format(enum pipe_format format)
{
   if (util_format_is_srgb(format))
      return RB_R16G16B16A16_FLOAT;
   switch (format) {
   case PIPE_FORMAT_R16_FLOAT:
   case PIPE_FORMAT_R16G16_FLOAT:
   case PIPE_FORMAT_R11G11B10_FLOAT:
      return RB_R16G16B16A16_FLOAT;
   case PIPE_FORMAT_L8_UNORM:
      return RB_R8G8B8A8_UNORM;
   default:
      return fd3_pipe2color(format);
   }
}

static inline enum a3xx_tex_swiz
tex_swiz(unsigned swiz)
{
   switch (swiz) {
   default:
   case PIPE_SWIZZLE_X:
      return A3XX_TEX_X;
   case PIPE_SWIZZLE_Y:
      return A3XX_TEX_Y;
   case PIPE_SWIZZLE_Z:
      return A3XX_TEX_Z;
   case PIPE_SWIZZLE_W:
      return A3XX_TEX_W;
   case PIPE_SWIZZLE_0:
      return A3XX_TEX_ZERO;
   case PIPE_SWIZZLE_1:
      return A3XX_TEX_ONE;
   }
}

uint32_t
fd3_tex_swiz(enum pipe_format format, unsigned swizzle_r, unsigned swizzle_g,
             unsigned swizzle_b, unsigned swizzle_a)
{
   const struct util_format_description *desc = util_format_description(format);
   unsigned char swiz[4] = {
      swizzle_r,
      swizzle_g,
      swizzle_b,
      swizzle_a,
   }, rswiz[4];

   util_format_compose_swizzles(desc->swizzle, swiz, rswiz);

   return A3XX_TEX_CONST_0_SWIZ_X(tex_swiz(rswiz[0])) |
          A3XX_TEX_CONST_0_SWIZ_Y(tex_swiz(rswiz[1])) |
          A3XX_TEX_CONST_0_SWIZ_Z(tex_swiz(rswiz[2])) |
          A3XX_TEX_CONST_0_SWIZ_W(tex_swiz(rswiz[3]));
}
