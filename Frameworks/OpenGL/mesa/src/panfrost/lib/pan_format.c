/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "pan_format.h"
#include "genxml/gen_macros.h"
#include "util/format/u_format.h"

/* Convenience */

#define MALI_BLEND_AU_R8G8B8A8    (MALI_RGBA8_TB << 12)
#define MALI_BLEND_PU_R8G8B8A8    (MALI_RGBA8_TB << 12)
#define MALI_BLEND_AU_R10G10B10A2 (MALI_RGB10_A2_TB << 12)
#define MALI_BLEND_PU_R10G10B10A2 (MALI_RGB10_A2_TB << 12)
#define MALI_BLEND_AU_R8G8B8A2    (MALI_RGB8_A2_AU << 12)
#define MALI_BLEND_PU_R8G8B8A2    (MALI_RGB8_A2_PU << 12)
#define MALI_BLEND_AU_R4G4B4A4    (MALI_RGBA4_AU << 12)
#define MALI_BLEND_PU_R4G4B4A4    (MALI_RGBA4_PU << 12)
#define MALI_BLEND_AU_R5G6B5A0    (MALI_R5G6B5_AU << 12)
#define MALI_BLEND_PU_R5G6B5A0    (MALI_R5G6B5_PU << 12)
#define MALI_BLEND_AU_R5G5B5A1    (MALI_RGB5_A1_AU << 12)
#define MALI_BLEND_PU_R5G5B5A1    (MALI_RGB5_A1_PU << 12)

#if PAN_ARCH <= 6
#define BFMT2(pipe, internal, writeback, srgb)                                 \
   [PIPE_FORMAT_##pipe] = {                                                    \
      MALI_COLOR_BUFFER_INTERNAL_FORMAT_##internal,                            \
      MALI_COLOR_FORMAT_##writeback,                                           \
      {                                                                        \
         MALI_BLEND_PU_##internal | (srgb ? (1 << 20) : 0) |                   \
            PAN_V6_SWIZZLE(R, G, B, A),                                        \
         MALI_BLEND_AU_##internal | (srgb ? (1 << 20) : 0) |                   \
            PAN_V6_SWIZZLE(R, G, B, A),                                        \
      },                                                                       \
   }
#else
#define BFMT2(pipe, internal, writeback, srgb)                                 \
   [PIPE_FORMAT_##pipe] = {                                                    \
      MALI_COLOR_BUFFER_INTERNAL_FORMAT_##internal,                            \
      MALI_COLOR_FORMAT_##writeback,                                           \
      {                                                                        \
         MALI_BLEND_PU_##internal | (srgb ? (1 << 20) : 0),                    \
         MALI_BLEND_AU_##internal | (srgb ? (1 << 20) : 0),                    \
      },                                                                       \
   }
#endif

#define BFMT(pipe, internal_and_writeback)                                     \
   BFMT2(pipe, internal_and_writeback, internal_and_writeback, 0)

#define BFMT_SRGB(pipe, writeback)                                             \
   BFMT2(pipe##_UNORM, R8G8B8A8, writeback, 0),                                \
      BFMT2(pipe##_SRGB, R8G8B8A8, writeback, 1)

const struct pan_blendable_format
   GENX(panfrost_blendable_formats)[PIPE_FORMAT_COUNT] = {
      BFMT_SRGB(L8, R8),
      BFMT_SRGB(L8A8, R8G8),
      BFMT_SRGB(R8, R8),
      BFMT_SRGB(R8G8, R8G8),
      BFMT_SRGB(R8G8B8, R8G8B8),

      BFMT_SRGB(B8G8R8A8, R8G8B8A8),
      BFMT_SRGB(B8G8R8X8, R8G8B8A8),
      BFMT_SRGB(A8R8G8B8, R8G8B8A8),
      BFMT_SRGB(X8R8G8B8, R8G8B8A8),
      BFMT_SRGB(A8B8G8R8, R8G8B8A8),
      BFMT_SRGB(X8B8G8R8, R8G8B8A8),
      BFMT_SRGB(R8G8B8X8, R8G8B8A8),
      BFMT_SRGB(R8G8B8A8, R8G8B8A8),

      BFMT2(A8_UNORM, R8G8B8A8, R8, 0),
      BFMT2(I8_UNORM, R8G8B8A8, R8, 0),
      BFMT2(R5G6B5_UNORM, R5G6B5A0, R5G6B5, 0),
      BFMT2(B5G6R5_UNORM, R5G6B5A0, R5G6B5, 0),

      BFMT(A4B4G4R4_UNORM, R4G4B4A4),
      BFMT(B4G4R4A4_UNORM, R4G4B4A4),
      BFMT(R4G4B4A4_UNORM, R4G4B4A4),

      BFMT(R10G10B10A2_UNORM, R10G10B10A2),
      BFMT(B10G10R10A2_UNORM, R10G10B10A2),
      BFMT(R10G10B10X2_UNORM, R10G10B10A2),
      BFMT(B10G10R10X2_UNORM, R10G10B10A2),

      BFMT(B5G5R5A1_UNORM, R5G5B5A1),
      BFMT(R5G5B5A1_UNORM, R5G5B5A1),
      BFMT(B5G5R5X1_UNORM, R5G5B5A1),
};

/* Convenience */

#define _V PAN_BIND_VERTEX_BUFFER
#define _T PAN_BIND_SAMPLER_VIEW
#define _R PAN_BIND_RENDER_TARGET
#define _Z PAN_BIND_DEPTH_STENCIL

#define FLAGS_V___ (_V)
#define FLAGS__T__ (_T)
#define FLAGS_VTR_ (_V | _T | _R)
#define FLAGS_VT__ (_V | _T)
#define FLAGS__T_Z (_T | _Z)

#define FMT(pipe, mali, swizzle, srgb, flags)                                  \
   [PIPE_FORMAT_##pipe] = {                                                    \
      .hw = MALI_PACK_FMT(mali, swizzle, srgb),                                \
      .bind = FLAGS_##flags,                                                   \
   }

#if PAN_ARCH >= 7
#define YUV_NO_SWAP (0)
#define YUV_SWAP    (1)

#define FMT_YUV(pipe, mali, swizzle, swap, siting, flags)                      \
   [PIPE_FORMAT_##pipe] = {                                                    \
      .hw = (MALI_YUV_SWIZZLE_##swizzle) | ((YUV_##swap) << 3) |               \
            ((MALI_YUV_CR_SITING_##siting) << 9) | ((MALI_##mali) << 12),      \
      .bind = FLAGS_##flags,                                                   \
   }
#endif

/* clang-format off */
const struct panfrost_format GENX(panfrost_pipe_format)[PIPE_FORMAT_COUNT] = {
   FMT(NONE,                    CONSTANT,        0000, L, VTR_),

#if PAN_ARCH >= 7
   /* Multiplane formats */
   FMT_YUV(R8G8_R8B8_UNORM, YUYV8, UVYA, NO_SWAP, CENTER_Y, _T__),
   FMT_YUV(G8R8_B8R8_UNORM, VYUY8, UYVA, SWAP,    CENTER_Y, _T__),
   FMT_YUV(R8B8_R8G8_UNORM, YUYV8, VYUA, NO_SWAP, CENTER_Y, _T__),
   FMT_YUV(B8R8_G8R8_UNORM, VYUY8, VUYA, SWAP,    CENTER_Y, _T__),
   FMT_YUV(R8_G8B8_420_UNORM, Y8_UV8_420, YUVA, NO_SWAP, CENTER, _T__),
   FMT_YUV(R8_B8G8_420_UNORM, Y8_UV8_420, YVUA, NO_SWAP, CENTER, _T__),
   FMT_YUV(R8_G8_B8_420_UNORM, Y8_U8_V8_420, YUVA, NO_SWAP, CENTER, _T__),
   FMT_YUV(R8_B8_G8_420_UNORM, Y8_U8_V8_420, YVUA, NO_SWAP, CENTER, _T__),
#endif

#if PAN_ARCH <= 7
   FMT(ETC1_RGB8,               ETC2_RGB8,       RGB1, L, _T__),
   FMT(ETC2_RGB8,               ETC2_RGB8,       RGB1, L, _T__),
   FMT(ETC2_SRGB8,              ETC2_RGB8,       RGB1, S, _T__),
   FMT(ETC2_R11_UNORM,          ETC2_R11_UNORM,  R001, L, _T__),
   FMT(ETC2_RGBA8,              ETC2_RGBA8,      RGBA, L, _T__),
   FMT(ETC2_SRGBA8,             ETC2_RGBA8,      RGBA, S, _T__),
   FMT(ETC2_RG11_UNORM,         ETC2_RG11_UNORM, RG01, L, _T__),
   FMT(ETC2_R11_SNORM,          ETC2_R11_SNORM,  R001, L, _T__),
   FMT(ETC2_RG11_SNORM,         ETC2_RG11_SNORM, RG01, L, _T__),
   FMT(ETC2_RGB8A1,             ETC2_RGB8A1,     RGBA, L, _T__),
   FMT(ETC2_SRGB8A1,            ETC2_RGB8A1,     RGBA, S, _T__),
   FMT(DXT1_RGB,                BC1_UNORM,       RGB1, L, _T__),
   FMT(DXT1_RGBA,               BC1_UNORM,       RGBA, L, _T__),
   FMT(DXT1_SRGB,               BC1_UNORM,       RGB1, S, _T__),
   FMT(DXT1_SRGBA,              BC1_UNORM,       RGBA, S, _T__),
   FMT(DXT3_RGBA,               BC2_UNORM,       RGBA, L, _T__),
   FMT(DXT3_SRGBA,              BC2_UNORM,       RGBA, S, _T__),
   FMT(DXT5_RGBA,               BC3_UNORM,       RGBA, L, _T__),
   FMT(DXT5_SRGBA,              BC3_UNORM,       RGBA, S, _T__),
   FMT(RGTC1_UNORM,             BC4_UNORM,       R001, L, _T__),
   FMT(RGTC1_SNORM,             BC4_SNORM,       R001, L, _T__),
   FMT(RGTC2_UNORM,             BC5_UNORM,       RG01, L, _T__),
   FMT(RGTC2_SNORM,             BC5_SNORM,       RG01, L, _T__),
   FMT(BPTC_RGB_FLOAT,          BC6H_SF16,       RGB1, L, _T__),
   FMT(BPTC_RGB_UFLOAT,         BC6H_UF16,       RGB1, L, _T__),
   FMT(BPTC_RGBA_UNORM,         BC7_UNORM,       RGBA, L, _T__),
   FMT(BPTC_SRGBA,              BC7_UNORM,       RGBA, S, _T__),
   FMT(ASTC_4x4,                ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_5x4,                ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_5x5,                ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_6x5,                ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_6x6,                ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_8x5,                ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_8x6,                ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_8x8,                ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_10x5,               ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_10x6,               ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_10x8,               ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_10x10,              ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_12x10,              ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_12x12,              ASTC_2D_HDR,     RGBA, L, _T__),
   FMT(ASTC_4x4_SRGB,           ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_5x4_SRGB,           ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_5x5_SRGB,           ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_6x5_SRGB,           ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_6x6_SRGB,           ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_8x5_SRGB,           ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_8x6_SRGB,           ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_8x8_SRGB,           ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_10x5_SRGB,          ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_10x6_SRGB,          ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_10x8_SRGB,          ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_10x10_SRGB,         ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_12x10_SRGB,         ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_12x12_SRGB,         ASTC_2D_LDR,     RGBA, S, _T__),
   FMT(ASTC_3x3x3,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_4x3x3,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_4x4x3,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_4x4x4,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_5x4x4,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_5x5x4,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_5x5x5,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_6x5x5,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_6x6x5,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_6x6x6,              ASTC_3D_HDR,     RGBA, L, _T__),
   FMT(ASTC_3x3x3_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_4x3x3_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_4x4x3_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_4x4x4_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_5x4x4_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_5x5x4_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_5x5x5_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_6x5x5_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_6x6x5_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
   FMT(ASTC_6x6x6_SRGB,         ASTC_3D_LDR,     RGBA, S, _T__),
#else
   /* Map to interchange format, as compression is specified in the plane
    * descriptor on Valhall.
    */
   FMT(ETC1_RGB8,               RGBA8_UNORM,     RGB1, L, _T__),
   FMT(ETC2_RGB8,               RGBA8_UNORM,     RGB1, L, _T__),
   FMT(ETC2_SRGB8,              RGBA8_UNORM,     RGB1, S, _T__),
   FMT(ETC2_R11_UNORM,          R16_UNORM,       R001, L, _T__),
   FMT(ETC2_RGBA8,              RGBA8_UNORM,     RGBA, L, _T__),
   FMT(ETC2_SRGBA8,             RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ETC2_RG11_UNORM,         RG16_UNORM,      RG01, L, _T__),
   FMT(ETC2_R11_SNORM,          R16_SNORM,       R001, L, _T__),
   FMT(ETC2_RG11_SNORM,         RG16_SNORM,      RG01, L, _T__),
   FMT(ETC2_RGB8A1,             RGBA8_UNORM,     RGBA, L, _T__),
   FMT(ETC2_SRGB8A1,            RGBA8_UNORM,     RGBA, S, _T__),
   FMT(DXT1_RGB,                RGBA8_UNORM,     RGB1, L, _T__),
   FMT(DXT1_RGBA,               RGBA8_UNORM,     RGBA, L, _T__),
   FMT(DXT1_SRGB,               RGBA8_UNORM,     RGB1, S, _T__),
   FMT(DXT1_SRGBA,              RGBA8_UNORM,     RGBA, S, _T__),
   FMT(DXT3_RGBA,               RGBA8_UNORM,     RGBA, L, _T__),
   FMT(DXT3_SRGBA,              RGBA8_UNORM,     RGBA, S, _T__),
   FMT(DXT5_RGBA,               RGBA8_UNORM,     RGBA, L, _T__),
   FMT(DXT5_SRGBA,              RGBA8_UNORM,     RGBA, S, _T__),
   FMT(RGTC1_UNORM,             R16_UNORM,       R001, L, _T__),
   FMT(RGTC1_SNORM,             R16_SNORM,       R001, L, _T__),
   FMT(RGTC2_UNORM,             RG16_UNORM,      RG01, L, _T__),
   FMT(RGTC2_SNORM,             RG16_SNORM,      RG01, L, _T__),
   FMT(BPTC_RGB_FLOAT,          RGBA16F,         RGB1, L, _T__),
   FMT(BPTC_RGB_UFLOAT,         RGBA16F,         RGB1, L, _T__),
   FMT(BPTC_RGBA_UNORM,         RGBA8_UNORM,     RGBA, L, _T__),
   FMT(BPTC_SRGBA,              RGBA8_UNORM,     RGBA, S, _T__),

   /* Mesa does not yet support astc_decode_mode extensions, so non-sRGB
    * formats must be assumed to be wide.
    */
   FMT(ASTC_4x4,                RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_5x4,                RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_5x5,                RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_6x5,                RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_6x6,                RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_8x5,                RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_8x6,                RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_8x8,                RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_10x5,               RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_10x6,               RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_10x8,               RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_10x10,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_12x10,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_12x12,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_3x3x3,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_4x3x3,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_4x4x3,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_4x4x4,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_5x4x4,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_5x5x4,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_5x5x5,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_6x5x5,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_6x6x5,              RGBA16F,         RGBA, L, _T__),
   FMT(ASTC_6x6x6,              RGBA16F,         RGBA, L, _T__),

   /* By definition, sRGB formats are narrow */
   FMT(ASTC_4x4_SRGB,           RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_5x4_SRGB,           RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_5x5_SRGB,           RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_6x5_SRGB,           RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_6x6_SRGB,           RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_8x5_SRGB,           RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_8x6_SRGB,           RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_8x8_SRGB,           RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_10x5_SRGB,          RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_10x6_SRGB,          RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_10x8_SRGB,          RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_10x10_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_12x10_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_12x12_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_3x3x3_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_4x3x3_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_4x4x3_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_4x4x4_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_5x4x4_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_5x5x4_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_5x5x5_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_6x5x5_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
   FMT(ASTC_6x6x5_SRGB,         RGBA8_UNORM,     RGBA, S, _T__),
#endif

   FMT(R5G6B5_UNORM,            RGB565,          RGB1, L, VTR_),
   FMT(B5G6R5_UNORM,            RGB565,          BGR1, L, VTR_),
   FMT(R5G5B5X1_UNORM,          RGB5_A1_UNORM,   RGB1, L, VT__),
   FMT(B5G5R5X1_UNORM,          RGB5_A1_UNORM,   BGR1, L, VT__),
   FMT(R5G5B5A1_UNORM,          RGB5_A1_UNORM,   RGBA, L, VTR_),
   FMT(B5G5R5A1_UNORM,          RGB5_A1_UNORM,   BGRA, L, VTR_),
   FMT(R10G10B10X2_UNORM,       RGB10_A2_UNORM,  RGB1, L, VTR_),
   FMT(B10G10R10X2_UNORM,       RGB10_A2_UNORM,  BGR1, L, VTR_),
   FMT(R10G10B10A2_UNORM,       RGB10_A2_UNORM,  RGBA, L, VTR_),
   FMT(B10G10R10A2_UNORM,       RGB10_A2_UNORM,  BGRA, L, VTR_),
#if PAN_ARCH <= 5
   FMT(R10G10B10X2_SNORM,       RGB10_A2_SNORM,  RGB1, L, VT__),
   FMT(R10G10B10A2_SNORM,       RGB10_A2_SNORM,  RGBA, L, VT__),
   FMT(B10G10R10A2_SNORM,       RGB10_A2_SNORM,  BGRA, L, VT__),
   FMT(R3G3B2_UNORM,            RGB332_UNORM,    RGB1, L, VT__),
#else
   FMT(R10G10B10X2_SNORM,       RGB10_A2_SNORM,  RGB1, L, V___),
   FMT(R10G10B10A2_SNORM,       RGB10_A2_SNORM,  RGBA, L, V___),
   FMT(B10G10R10A2_SNORM,       RGB10_A2_SNORM,  BGRA, L, V___),
#endif
   FMT(R10G10B10A2_UINT,        RGB10_A2UI,      RGBA, L, VTR_),
   FMT(B10G10R10A2_UINT,        RGB10_A2UI,      BGRA, L, VTR_),
   FMT(R10G10B10A2_USCALED,     RGB10_A2UI,      RGBA, L, V___),
   FMT(B10G10R10A2_USCALED,     RGB10_A2UI,      BGRA, L, V___),
   FMT(R10G10B10A2_SINT,        RGB10_A2I,       RGBA, L, VTR_),
   FMT(B10G10R10A2_SINT,        RGB10_A2I,       BGRA, L, VTR_),
   FMT(R10G10B10A2_SSCALED,     RGB10_A2I,       RGBA, L, V___),
   FMT(B10G10R10A2_SSCALED,     RGB10_A2I,       BGRA, L, V___),
   FMT(R8_SSCALED,              R8I,             R001, L, V___),
   FMT(R8G8_SSCALED,            RG8I,            RG01, L, V___),
   FMT(R8G8B8_SSCALED,          RGB8I,           RGB1, L, V___),
   FMT(B8G8R8_SSCALED,          RGB8I,           BGR1, L, V___),
   FMT(R8G8B8A8_SSCALED,        RGBA8I,          RGBA, L, V___),
   FMT(B8G8R8A8_SSCALED,        RGBA8I,          BGRA, L, V___),
   FMT(A8B8G8R8_SSCALED,        RGBA8I,          ABGR, L, V___),
   FMT(R8_USCALED,              R8UI,            R001, L, V___),
   FMT(R8G8_USCALED,            RG8UI,           RG01, L, V___),
   FMT(R8G8B8_USCALED,          RGB8UI,          RGB1, L, V___),
   FMT(B8G8R8_USCALED,          RGB8UI,          BGR1, L, V___),
   FMT(R8G8B8A8_USCALED,        RGBA8UI,         RGBA, L, V___),
   FMT(B8G8R8A8_USCALED,        RGBA8UI,         BGRA, L, V___),
   FMT(A8B8G8R8_USCALED,        RGBA8UI,         ABGR, L, V___),
   FMT(R16_USCALED,             R16UI,           R001, L, V___),
   FMT(R16G16_USCALED,          RG16UI,          RG01, L, V___),
   FMT(R16G16B16A16_USCALED,    RGBA16UI,        RGBA, L, V___),
   FMT(R16_SSCALED,             R16I,            R001, L, V___),
   FMT(R16G16_SSCALED,          RG16I,           RG01, L, V___),
   FMT(R16G16B16A16_SSCALED,    RGBA16I,         RGBA, L, V___),
   FMT(R32_USCALED,             R32UI,           R001, L, V___),
   FMT(R32G32_USCALED,          RG32UI,          RG01, L, V___),
   FMT(R32G32B32_USCALED,       RGB32UI,         RGB1, L, V___),
   FMT(R32G32B32A32_USCALED,    RGBA32UI,        RGBA, L, V___),
   FMT(R32_SSCALED,             R32I,            R001, L, V___),
   FMT(R32G32_SSCALED,          RG32I,           RG01, L, V___),
   FMT(R32G32B32_SSCALED,       RGB32I,          RGB1, L, V___),
   FMT(R32G32B32A32_SSCALED,    RGBA32I,         RGBA, L, V___),
   FMT(R32_FIXED,               R32_FIXED,       R001, L, V___),
   FMT(R32G32_FIXED,            RG32_FIXED,      RG01, L, V___),
   FMT(R32G32B32_FIXED,         RGB32_FIXED,     RGB1, L, V___),
   FMT(R32G32B32A32_FIXED,      RGBA32_FIXED,    RGBA, L, V___),
   FMT(R11G11B10_FLOAT,         R11F_G11F_B10F,  RGB1, L, VTR_),
   FMT(R9G9B9E5_FLOAT,          R9F_G9F_B9F_E5F, RGB1, L, VT__),
#if PAN_ARCH >= 6
   /* SNORM is renderable on Bifrost (with blend shaders) */
   FMT(R8_SNORM,                R8_SNORM,        R001, L, VTR_),
   FMT(R16_SNORM,               R16_SNORM,       R001, L, VTR_),
   FMT(R8G8_SNORM,              RG8_SNORM,       RG01, L, VTR_),
   FMT(R16G16_SNORM,            RG16_SNORM,      RG01, L, VTR_),
   FMT(R8G8B8_SNORM,            RGB8_SNORM,      RGB1, L, VTR_),
   FMT(R8G8B8A8_SNORM,          RGBA8_SNORM,     RGBA, L, VTR_),
   FMT(R16G16B16A16_SNORM,      RGBA16_SNORM,    RGBA, L, VTR_),
#else
   /* So far we haven't needed SNORM rendering on Midgard */
   FMT(R8_SNORM,                R8_SNORM,        R001, L, VT__),
   FMT(R16_SNORM,               R16_SNORM,       R001, L, VT__),
   FMT(R8G8_SNORM,              RG8_SNORM,       RG01, L, VT__),
   FMT(R16G16_SNORM,            RG16_SNORM,      RG01, L, VT__),
   FMT(R8G8B8_SNORM,            RGB8_SNORM,      RGB1, L, VT__),
   FMT(R8G8B8A8_SNORM,          RGBA8_SNORM,     RGBA, L, VT__),
   FMT(R16G16B16A16_SNORM,      RGBA16_SNORM,    RGBA, L, VT__),
#endif
   FMT(I8_SINT,                 R8I,             RRRR, L, VTR_),
   FMT(L8_SINT,                 R8I,             RRR1, L, VTR_),
   FMT(I8_UINT,                 R8UI,            RRRR, L, VTR_),
   FMT(L8_UINT,                 R8UI,            RRR1, L, VTR_),
   FMT(I16_SINT,                R16I,            RRRR, L, VTR_),
   FMT(L16_SINT,                R16I,            RRR1, L, VTR_),
   FMT(I16_UINT,                R16UI,           RRRR, L, VTR_),
   FMT(L16_UINT,                R16UI,           RRR1, L, VTR_),
   FMT(I32_SINT,                R32I,            RRRR, L, VTR_),
   FMT(L32_SINT,                R32I,            RRR1, L, VTR_),
   FMT(I32_UINT,                R32UI,           RRRR, L, VTR_),
   FMT(L32_UINT,                R32UI,           RRR1, L, VTR_),
   FMT(B8G8R8_UINT,             RGB8UI,          BGR1, L, V___),
   FMT(B8G8R8_SINT,             RGB8I,           BGR1, L, V___),
   FMT(B8G8R8A8_UINT,           RGBA8UI,         BGRA, L, VTR_),
   FMT(B8G8R8A8_SINT,           RGBA8I,          BGRA, L, VTR_),
   FMT(A8R8G8B8_UINT,           RGBA8UI,         GBAR, L, VTR_),
   FMT(A8B8G8R8_UINT,           RGBA8UI,         ABGR, L, VTR_),
   FMT(R8_UINT,                 R8UI,            R001, L, VTR_),
   FMT(R16_UINT,                R16UI,           R001, L, VTR_),
   FMT(R32_UINT,                R32UI,           R001, L, VTR_),
   FMT(R8G8_UINT,               RG8UI,           RG01, L, VTR_),
   FMT(R16G16_UINT,             RG16UI,          RG01, L, VTR_),
   FMT(R32G32_UINT,             RG32UI,          RG01, L, VTR_),
   FMT(R8G8B8_UINT,             RGB8UI,          RGB1, L, V___),
   FMT(R32G32B32_UINT,          RGB32UI,         RGB1, L, VTR_),
   FMT(R8G8B8A8_UINT,           RGBA8UI,         RGBA, L, VTR_),
   FMT(R16G16B16A16_UINT,       RGBA16UI,        RGBA, L, VTR_),
   FMT(R32G32B32A32_UINT,       RGBA32UI,        RGBA, L, VTR_),
   FMT(R32_FLOAT,               R32F,            R001, L, VTR_),
   FMT(R32G32_FLOAT,            RG32F,           RG01, L, VTR_),
   FMT(R32G32B32_FLOAT,         RGB32F,          RGB1, L, VTR_),
   FMT(R32G32B32A32_FLOAT,      RGBA32F,         RGBA, L, VTR_),
   FMT(R8_UNORM,                R8_UNORM,        R001, L, VTR_),
   FMT(R16_UNORM,               R16_UNORM,       R001, L, VTR_),
   FMT(R8G8_UNORM,              RG8_UNORM,       RG01, L, VTR_),
   FMT(R16G16_UNORM,            RG16_UNORM,      RG01, L, VTR_),
   FMT(R8G8B8_UNORM,            RGB8_UNORM,      RGB1, L, V___),

   /* 32-bit NORM is not texturable in v7 onwards. It's renderable
    * everywhere, but rendering without texturing is not useful.
    */
#if PAN_ARCH <= 6
   FMT(R32_UNORM,               R32_UNORM,       R001, L, VTR_),
   FMT(R32G32_UNORM,            RG32_UNORM,      RG01, L, VTR_),
   FMT(R32G32B32_UNORM,         RGB32_UNORM,     RGB1, L, VT__),
   FMT(R32G32B32A32_UNORM,      RGBA32_UNORM,    RGBA, L, VTR_),
   FMT(R32_SNORM,               R32_SNORM,       R001, L, VT__),
   FMT(R32G32_SNORM,            RG32_SNORM,      RG01, L, VT__),
   FMT(R32G32B32_SNORM,         RGB32_SNORM,     RGB1, L, VT__),
   FMT(R32G32B32A32_SNORM,      RGBA32_SNORM,    RGBA, L, VT__),
#else
   FMT(R32_UNORM,               R32_UNORM,       R001, L, V___),
   FMT(R32G32_UNORM,            RG32_UNORM,      RG01, L, V___),
   FMT(R32G32B32_UNORM,         RGB32_UNORM,     RGB1, L, V___),
   FMT(R32G32B32A32_UNORM,      RGBA32_UNORM,    RGBA, L, V___),
   FMT(R32_SNORM,               R32_SNORM,       R001, L, V___),
   FMT(R32G32_SNORM,            RG32_SNORM,      RG01, L, V___),
   FMT(R32G32B32_SNORM,         RGB32_SNORM,     RGB1, L, V___),
   FMT(R32G32B32A32_SNORM,      RGBA32_SNORM,    RGBA, L, V___),
#endif

   /* Don't allow render/texture for 48-bit  */
   FMT(R16G16B16_UNORM,         RGB16_UNORM,     RGB1, L, V___),
   FMT(R16G16B16_SINT,          RGB16I,          RGB1, L, V___),
   FMT(R16G16B16_FLOAT,         RGB16F,          RGB1, L, V___),
   FMT(R16G16B16_USCALED,       RGB16UI,         RGB1, L, V___),
   FMT(R16G16B16_SSCALED,       RGB16I,          RGB1, L, V___),
   FMT(R16G16B16_SNORM,         RGB16_SNORM,     RGB1, L, V___),
   FMT(R16G16B16_UINT,          RGB16UI,         RGB1, L, V___),
   FMT(R4G4B4A4_UNORM,          RGBA4_UNORM,     RGBA, L, VTR_),
   FMT(B4G4R4A4_UNORM,          RGBA4_UNORM,     BGRA, L, VTR_),
   FMT(R16G16B16A16_UNORM,      RGBA16_UNORM,    RGBA, L, VTR_),
   FMT(B8G8R8A8_UNORM,          RGBA8_UNORM,     BGRA, L, VTR_),
   FMT(B8G8R8X8_UNORM,          RGBA8_UNORM,     BGR1, L, VTR_),
   FMT(A8R8G8B8_UNORM,          RGBA8_UNORM,     GBAR, L, VTR_),
   FMT(X8R8G8B8_UNORM,          RGBA8_UNORM,     GBA1, L, VTR_),
   FMT(A8B8G8R8_UNORM,          RGBA8_UNORM,     ABGR, L, VTR_),
   FMT(X8B8G8R8_UNORM,          RGBA8_UNORM,     ABG1, L, VTR_),
   FMT(R8G8B8X8_UNORM,          RGBA8_UNORM,     RGB1, L, VTR_),
   FMT(R8G8B8A8_UNORM,          RGBA8_UNORM,     RGBA, L, VTR_),
   FMT(R8G8B8X8_SNORM,          RGBA8_SNORM,     RGB1, L, VT__),
   FMT(R8G8B8X8_SRGB,           RGBA8_UNORM,     RGB1, S, VTR_),
   FMT(R8G8B8X8_UINT,           RGBA8UI,         RGB1, L, VTR_),
   FMT(R8G8B8X8_SINT,           RGBA8I,          RGB1, L, VTR_),
   FMT(L8_UNORM,                R8_UNORM,        RRR1, L, VTR_),
   FMT(I8_UNORM,                R8_UNORM,        RRRR, L, VTR_),
   FMT(L16_UNORM,               R16_UNORM,       RRR1, L, VT__),
   FMT(I16_UNORM,               R16_UNORM,       RRRR, L, VT__),
   FMT(L8_SNORM,                R8_SNORM,        RRR1, L, VT__),
   FMT(I8_SNORM,                R8_SNORM,        RRRR, L, VT__),
   FMT(L16_SNORM,               R16_SNORM,       RRR1, L, VT__),
   FMT(I16_SNORM,               R16_SNORM,       RRRR, L, VT__),
   FMT(L16_FLOAT,               R16F,            RRR1, L, VTR_),
   FMT(I16_FLOAT,               RG16F,           RRRR, L, VTR_),
   FMT(L8_SRGB,                 R8_UNORM,        RRR1, S, VTR_),
   FMT(R8_SRGB,                 R8_UNORM,        R001, S, VTR_),
   FMT(R8G8_SRGB,               RG8_UNORM,       RG01, S, VTR_),
   FMT(R8G8B8_SRGB,             RGB8_UNORM,      RGB1, S, V___),
   FMT(B8G8R8_SRGB,             RGB8_UNORM,      BGR1, S, V___),
   FMT(R8G8B8A8_SRGB,           RGBA8_UNORM,     RGBA, S, VTR_),
   FMT(A8B8G8R8_SRGB,           RGBA8_UNORM,     ABGR, S, VTR_),
   FMT(X8B8G8R8_SRGB,           RGBA8_UNORM,     ABG1, S, VTR_),
   FMT(B8G8R8A8_SRGB,           RGBA8_UNORM,     BGRA, S, VTR_),
   FMT(B8G8R8X8_SRGB,           RGBA8_UNORM,     BGR1, S, VTR_),
   FMT(A8R8G8B8_SRGB,           RGBA8_UNORM,     GBAR, S, VTR_),
   FMT(X8R8G8B8_SRGB,           RGBA8_UNORM,     GBA1, S, VTR_),
   FMT(R8_SINT,                 R8I,             R001, L, VTR_),
   FMT(R16_SINT,                R16I,            R001, L, VTR_),
   FMT(R32_SINT,                R32I,            R001, L, VTR_),
   FMT(R16_FLOAT,               R16F,            R001, L, VTR_),
   FMT(R8G8_SINT,               RG8I,            RG01, L, VTR_),
   FMT(R16G16_SINT,             RG16I,           RG01, L, VTR_),
   FMT(R32G32_SINT,             RG32I,           RG01, L, VTR_),
   FMT(R16G16_FLOAT,            RG16F,           RG01, L, VTR_),
   FMT(R8G8B8_SINT,             RGB8I,           RGB1, L, V___),
   FMT(R32G32B32_SINT,          RGB32I,          RGB1, L, VTR_),
   FMT(R8G8B8A8_SINT,           RGBA8I,          RGBA, L, VTR_),
   FMT(R16G16B16A16_SINT,       RGBA16I,         RGBA, L, VTR_),
   FMT(R32G32B32A32_SINT,       RGBA32I,         RGBA, L, VTR_),
   FMT(R16G16B16A16_FLOAT,      RGBA16F,         RGBA, L, VTR_),
   FMT(R16G16B16X16_UNORM,      RGBA16_UNORM,    RGB1, L, VTR_),
   FMT(R16G16B16X16_SNORM,      RGBA16_SNORM,    RGB1, L, VT__),
   FMT(R16G16B16X16_FLOAT,      RGBA16F,         RGB1, L, VTR_),
   FMT(R16G16B16X16_UINT,       RGBA16UI,        RGB1, L, VTR_),
   FMT(R16G16B16X16_SINT,       RGBA16I,         RGB1, L, VTR_),
   FMT(R32G32B32X32_FLOAT,      RGBA32F,         RGB1, L, VTR_),
   FMT(R32G32B32X32_UINT,       RGBA32UI,        RGB1, L, VTR_),
   FMT(R32G32B32X32_SINT,       RGBA32I,         RGB1, L, VTR_),

#if PAN_ARCH <= 6
   FMT(Z16_UNORM,               R16_UNORM,       RRRR, L, _T_Z),
   FMT(Z24_UNORM_S8_UINT,       Z24X8_UNORM,     RRRR, L, _T_Z),
   FMT(Z24X8_UNORM,             Z24X8_UNORM,     RRRR, L, _T_Z),
   FMT(Z32_FLOAT,               R32F,            RRRR, L, _T_Z),
   FMT(Z32_FLOAT_S8X24_UINT,    RG32F,           RRRR, L, _T_Z),
   FMT(X32_S8X24_UINT,          X32_S8X24,       GGGG, L, _T_Z),
   FMT(X24S8_UINT,              RGBA8UI,         AAAA, L, _T_Z),
   FMT(S8_UINT,                 R8UI,            RRRR, L, _T__),

   FMT(A8_UNORM,                R8_UNORM,        000R, L, VTR_),
   FMT(L8A8_UNORM,              RG8_UNORM,       RRRG, L, VTR_),
   FMT(L8A8_SRGB,               RG8_UNORM,       RRRG, S, VTR_),

   /* These formats were removed in v7 */
   FMT(A8_SNORM,                R8_SNORM,        000R, L, VT__),
   FMT(A8_SINT,                 R8I,             000R, L, VTR_),
   FMT(A8_UINT,                 R8UI,            000R, L, VTR_),
   FMT(A16_SINT,                R16I,            000R, L, VTR_),
   FMT(A16_UINT,                R16UI,           000R, L, VTR_),
   FMT(A32_SINT,                R32I,            000R, L, VTR_),
   FMT(A32_UINT,                R32UI,           000R, L, VTR_),
   FMT(A16_UNORM,               R16_UNORM,       000R, L, VT__),
   FMT(A16_SNORM,               R16_SNORM,       000R, L, VT__),
   FMT(A16_FLOAT,               R16F,            000R, L, VTR_),

#else
   FMT(Z16_UNORM,               Z16_UNORM,       RGBA, L, _T_Z),
   FMT(Z24_UNORM_S8_UINT,       Z24X8_UNORM,     RGBA, L, _T_Z),
   FMT(Z24X8_UNORM,             Z24X8_UNORM,     RGBA, L, _T_Z),
   FMT(Z32_FLOAT,               R32F,            RGBA, L, _T_Z),

#if PAN_ARCH >= 9
   /* Specify interchange formats, the actual format for depth/stencil is
    * determined by the plane descriptor on Valhall.
    *
    * On Valhall, S8 logically acts like "X8S8", so "S8 RGBA" is logically
    * "0s00" and "S8 GRBA" is logically "s000". For Bifrost compatibility
    * we want stencil in the red channel, so we use the GRBA swizzles.
    */
   FMT(Z32_FLOAT_S8X24_UINT,    R32F,            GRBA, L, _T_Z),
   FMT(X32_S8X24_UINT,          S8,              GRBA, L, _T__),
   FMT(X24S8_UINT,              S8,              GRBA, L, _T_Z),
   FMT(S8_UINT,                 S8,              GRBA, L, _T__),

#else
   /* Specify real formats on Bifrost */
   FMT(Z32_FLOAT_S8X24_UINT,    Z32_X32,         RGBA, L, _T_Z),
   FMT(X32_S8X24_UINT,          X32_S8X24,       GRBA, L, _T__),
   FMT(X24S8_UINT,              X24S8,           GRBA, L, _T_Z),
   FMT(S8_UINT,                 S8,              GRBA, L, _T__),

   /* Obsolete formats removed in Valhall */
   FMT(A8_UNORM,                A8_UNORM,        000A, L, VTR_),
   FMT(L8A8_UNORM,              R8A8_UNORM,      RRRA, L, VTR_),
   FMT(L8A8_SRGB,               R8A8_UNORM,      RRRA, S, VTR_),
#endif

#endif
};
/* clang-format on */

#if PAN_ARCH == 7
/*
 * Decompose a component ordering swizzle into a component ordering (applied
 * first) and a swizzle (applied second). The output ordering "pre" is allowed
 * with compression and the swizzle "post" is a bijection.
 *
 * These properties allow any component ordering to be used with compression, by
 * using the output "pre" ordering, composing the API swizzle with the "post"
 * ordering, and applying the inverse of the "post" ordering to the border
 * colour to undo what we compose into the API swizzle.
 *
 * Note that "post" is a swizzle, not a component ordering, which means it is
 * inverted from the ordering. E.g. ARGB ordering uses a GBAR (YZWX) swizzle.
 */
struct pan_decomposed_swizzle
GENX(pan_decompose_swizzle)(enum mali_rgb_component_order order)
{
#define CASE(case_, pre_, R_, G_, B_, A_)                                      \
   case MALI_RGB_COMPONENT_ORDER_##case_:                                      \
      return (struct pan_decomposed_swizzle){                                  \
         MALI_RGB_COMPONENT_ORDER_##pre_,                                      \
         {                                                                     \
            PIPE_SWIZZLE_##R_,                                                 \
            PIPE_SWIZZLE_##G_,                                                 \
            PIPE_SWIZZLE_##B_,                                                 \
            PIPE_SWIZZLE_##A_,                                                 \
         },                                                                    \
      };

   switch (order) {
      CASE(RGBA, RGBA, X, Y, Z, W);
      CASE(GRBA, RGBA, Y, X, Z, W);
      CASE(BGRA, RGBA, Z, Y, X, W);
      CASE(ARGB, RGBA, Y, Z, W, X);
      CASE(AGRB, RGBA, Z, Y, W, X);
      CASE(ABGR, RGBA, W, Z, Y, X);
      CASE(RGB1, RGB1, X, Y, Z, W);
      CASE(GRB1, RGB1, Y, X, Z, W);
      CASE(BGR1, RGB1, Z, Y, X, W);
      CASE(1RGB, RGB1, Y, Z, W, X);
      CASE(1GRB, RGB1, Z, Y, W, X);
      CASE(1BGR, RGB1, W, Z, Y, X);
      CASE(RRRR, RRRR, X, Y, Z, W);
      CASE(RRR1, RRR1, X, Y, Z, W);
      CASE(RRRA, RRRA, X, Y, Z, W);
      CASE(000A, 000A, X, Y, Z, W);
      CASE(0001, 0001, X, Y, Z, W);
      CASE(0000, 0000, X, Y, Z, W);
   default:
      unreachable("Invalid case for texturing");
   }

#undef CASE
}
#endif
