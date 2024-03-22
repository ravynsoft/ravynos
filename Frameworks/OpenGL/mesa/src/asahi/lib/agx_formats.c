/*
 * Copyright 2021 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_formats.h"
#include "agx_internal_formats.h"
#include "agx_pack.h"

#define T                     true
#define F                     false
#define AGX_INTERNAL_FORMAT__ PIPE_FORMAT_NONE

#define AGX_FMT(pipe, channels_, type_, is_renderable, internal_fmt)           \
   [PIPE_FORMAT_##pipe] = {                                                    \
      .channels = AGX_CHANNELS_##channels_,                                    \
      .type = AGX_TEXTURE_TYPE_##type_,                                        \
      .renderable = is_renderable,                                             \
      .texturable = true,                                                      \
      .internal = (enum pipe_format)AGX_INTERNAL_FORMAT_##internal_fmt,        \
   }

/* clang-format off */
const struct agx_pixel_format_entry agx_pixel_format[PIPE_FORMAT_COUNT] = {
   AGX_FMT(R5G6B5_UNORM,            R5G6B5,        UNORM,  T, F16),
   AGX_FMT(B5G6R5_UNORM,            R5G6B5,        UNORM,  T, F16),

   AGX_FMT(R5G5B5A1_UNORM,          R5G5B5A1,      UNORM,  T, F16),
   AGX_FMT(B5G5R5A1_UNORM,          R5G5B5A1,      UNORM,  T, F16),
   AGX_FMT(R5G5B5X1_UNORM,          R5G5B5A1,      UNORM,  T, F16),
   AGX_FMT(B5G5R5X1_UNORM,          R5G5B5A1,      UNORM,  T, F16),

   AGX_FMT(R4G4B4A4_UNORM,          R4G4B4A4,      UNORM,  T, F16),
   AGX_FMT(B4G4R4A4_UNORM,          R4G4B4A4,      UNORM,  T, F16),
   AGX_FMT(A4B4G4R4_UNORM,          R4G4B4A4,      UNORM,  T, F16),

   AGX_FMT(R8_UNORM,                R8,            UNORM,  T, U8NORM),
   AGX_FMT(R8G8_UNORM,              R8G8,          UNORM,  T, U8NORM),
   AGX_FMT(R8G8B8A8_UNORM,          R8G8B8A8,      UNORM,  T, U8NORM),
   AGX_FMT(A8R8G8B8_UNORM,          R8G8B8A8,      UNORM,  T, U8NORM),
   AGX_FMT(A8B8G8R8_UNORM,          R8G8B8A8,      UNORM,  T, U8NORM),
   AGX_FMT(B8G8R8A8_UNORM,          R8G8B8A8,      UNORM,  T, U8NORM),
   AGX_FMT(R8G8B8X8_UNORM,          R8G8B8A8,      UNORM,  T, U8NORM),
   AGX_FMT(X8R8G8B8_UNORM,          R8G8B8A8,      UNORM,  T, U8NORM),
   AGX_FMT(X8B8G8R8_UNORM,          R8G8B8A8,      UNORM,  T, U8NORM),
   AGX_FMT(B8G8R8X8_UNORM,          R8G8B8A8,      UNORM,  T, U8NORM),

   AGX_FMT(R16_UNORM,               R16,           UNORM,  T, U16NORM),
   AGX_FMT(R16G16_UNORM,            R16G16,        UNORM,  T, U16NORM),
   AGX_FMT(R16G16B16A16_UNORM,      R16G16B16A16,  UNORM,  T, U16NORM),
   AGX_FMT(R16_SNORM,               R16,           SNORM,  T, S16NORM),
   AGX_FMT(R16G16_SNORM,            R16G16,        SNORM,  T, S16NORM),
   AGX_FMT(R16G16B16A16_SNORM,      R16G16B16A16,  SNORM,  T, S16NORM),

   AGX_FMT(R8_SRGB,                 R8,            UNORM,  T, SRGBA8),
   AGX_FMT(R8G8_SRGB,               R8G8,          UNORM,  T, SRGBA8),
   AGX_FMT(R8G8B8A8_SRGB,           R8G8B8A8,      UNORM,  T, SRGBA8),
   AGX_FMT(A8R8G8B8_SRGB,           R8G8B8A8,      UNORM,  T, SRGBA8),
   AGX_FMT(A8B8G8R8_SRGB,           R8G8B8A8,      UNORM,  T, SRGBA8),
   AGX_FMT(B8G8R8A8_SRGB,           R8G8B8A8,      UNORM,  T, SRGBA8),
   AGX_FMT(R8G8B8X8_SRGB,           R8G8B8A8,      UNORM,  T, SRGBA8),
   AGX_FMT(X8R8G8B8_SRGB,           R8G8B8A8,      UNORM,  T, SRGBA8),
   AGX_FMT(X8B8G8R8_SRGB,           R8G8B8A8,      UNORM,  T, SRGBA8),
   AGX_FMT(B8G8R8X8_SRGB,           R8G8B8A8,      UNORM,  T, SRGBA8),

   AGX_FMT(R8_SNORM,                R8,            SNORM,  T, S8NORM),
   AGX_FMT(R8G8_SNORM,              R8G8,          SNORM,  T, S8NORM),
   AGX_FMT(R8G8B8A8_SNORM,          R8G8B8A8,      SNORM,  T, S8NORM),
   AGX_FMT(A8R8G8B8_SNORM,          R8G8B8A8,      SNORM,  T, S8NORM),
   AGX_FMT(A8B8G8R8_SNORM,          R8G8B8A8,      SNORM,  T, S8NORM),
   AGX_FMT(B8G8R8A8_SNORM,          R8G8B8A8,      SNORM,  T, S8NORM),
   AGX_FMT(R8G8B8X8_SNORM,          R8G8B8A8,      SNORM,  T, S8NORM),
   AGX_FMT(X8R8G8B8_SNORM,          R8G8B8A8,      SNORM,  T, S8NORM),
   AGX_FMT(X8B8G8R8_SNORM,          R8G8B8A8,      SNORM,  T, S8NORM),
   AGX_FMT(B8G8R8X8_SNORM,          R8G8B8A8,      SNORM,  T, S8NORM),

   AGX_FMT(R16_FLOAT,               R16,           FLOAT,  T, F16),
   AGX_FMT(R16G16_FLOAT,            R16G16,        FLOAT,  T, F16),
   AGX_FMT(R16G16B16X16_FLOAT,      R16G16B16A16,  FLOAT,  T, F16),
   AGX_FMT(R16G16B16A16_FLOAT,      R16G16B16A16,  FLOAT,  T, F16),

   AGX_FMT(R32_FLOAT,               R32,           FLOAT,  T, I32),
   AGX_FMT(R32G32_FLOAT,            R32G32,        FLOAT,  T, I32),
   AGX_FMT(R32G32B32X32_FLOAT,      R32G32B32A32,  FLOAT,  T, I32),
   AGX_FMT(R32G32B32A32_FLOAT,      R32G32B32A32,  FLOAT,  T, I32),

   AGX_FMT(R8_UINT,                 R8,            UINT,   T, I8),
   AGX_FMT(R8G8_UINT,               R8G8,          UINT,   T, I8),
   AGX_FMT(R8G8B8X8_UINT,           R8G8B8A8,      UINT,   T, I8),
   AGX_FMT(R8G8B8A8_UINT,           R8G8B8A8,      UINT,   T, I8),
   AGX_FMT(B8G8R8X8_UINT,           R8G8B8A8,      UINT,   T, I8),
   AGX_FMT(B8G8R8A8_UINT,           R8G8B8A8,      UINT,   T, I8),

   AGX_FMT(R16_UINT,                R16,           UINT,   T, I16),
   AGX_FMT(R16G16_UINT,             R16G16,        UINT,   T, I16),
   AGX_FMT(R16G16B16X16_UINT,       R16G16B16A16,  UINT,   T, I16),
   AGX_FMT(R16G16B16A16_UINT,       R16G16B16A16,  UINT,   T, I16),

   AGX_FMT(R32_UINT,                R32,           UINT,   T, I32),
   AGX_FMT(R32G32_UINT,             R32G32,        UINT,   T, I32),
   AGX_FMT(R32G32B32X32_UINT,       R32G32B32A32,  UINT,   T, I32),
   AGX_FMT(R32G32B32A32_UINT,       R32G32B32A32,  UINT,   T, I32),

   AGX_FMT(R8_SINT,                 R8,            SINT,   T, I8),
   AGX_FMT(R8G8_SINT,               R8G8,          SINT,   T, I8),
   AGX_FMT(R8G8B8X8_SINT,           R8G8B8A8,      SINT,   T, I8),
   AGX_FMT(R8G8B8A8_SINT,           R8G8B8A8,      SINT,   T, I8),
   AGX_FMT(B8G8R8X8_SINT,           R8G8B8A8,      SINT,   T, I8),
   AGX_FMT(B8G8R8A8_SINT,           R8G8B8A8,      SINT,   T, I8),

   AGX_FMT(R16_SINT,                R16,           SINT,   T, I16),
   AGX_FMT(R16G16_SINT,             R16G16,        SINT,   T, I16),
   AGX_FMT(R16G16B16X16_SINT,       R16G16B16A16,  SINT,   T, I16),
   AGX_FMT(R16G16B16A16_SINT,       R16G16B16A16,  SINT,   T, I16),

   AGX_FMT(R32_SINT,                R32,           SINT,   T, I32),
   AGX_FMT(R32G32_SINT,             R32G32,        SINT,   T, I32),
   AGX_FMT(R32G32B32X32_SINT,       R32G32B32A32,  SINT,   T, I32),
   AGX_FMT(R32G32B32A32_SINT,       R32G32B32A32,  SINT,   T, I32),

   AGX_FMT(Z16_UNORM,               R16,           UNORM,  F, _),
   AGX_FMT(Z32_FLOAT,               R32,           FLOAT,  F, _),
   AGX_FMT(Z32_FLOAT_S8X24_UINT,    R32,           FLOAT,  F, _),
   AGX_FMT(S8_UINT,                 R8,            UINT,   F, _),

   /* The stencil part of Z32F + S8 is just S8 */
   AGX_FMT(X32_S8X24_UINT,          R8,            UINT,   F, _),

   /* These must be lowered by u_transfer_helper to Z32F + S8 */
   AGX_FMT(Z24X8_UNORM,             R32,           FLOAT,  F, _),
   AGX_FMT(Z24_UNORM_S8_UINT,       R32,           FLOAT,  F, _),

   AGX_FMT(R10G10B10A2_UNORM,       R10G10B10A2,   UNORM,  T, RGB10A2),
   AGX_FMT(R10G10B10X2_UNORM,       R10G10B10A2,   UNORM,  T, RGB10A2),
   AGX_FMT(B10G10R10A2_UNORM,       R10G10B10A2,   UNORM,  T, RGB10A2),
   AGX_FMT(B10G10R10X2_UNORM,       R10G10B10A2,   UNORM,  T, RGB10A2),

   AGX_FMT(R10G10B10A2_UINT,        R10G10B10A2,   UINT,   T, I16),
   AGX_FMT(B10G10R10A2_UINT,        R10G10B10A2,   UINT,   T, I16),

   AGX_FMT(R10G10B10A2_SINT,        R10G10B10A2,   SINT,   T, I16),
   AGX_FMT(B10G10R10A2_SINT,        R10G10B10A2,   SINT,   T, I16),

   AGX_FMT(R11G11B10_FLOAT,         R11G11B10,     FLOAT,  T, RG11B10F),
   AGX_FMT(R9G9B9E5_FLOAT,          R9G9B9E5,      FLOAT,  F, RGB9E5),

   /* These formats are emulated for texture buffers only */
   AGX_FMT(R32G32B32_FLOAT,         R32G32B32_EMULATED,    FLOAT,  F, _),
   AGX_FMT(R32G32B32_UINT,          R32G32B32_EMULATED,    UINT,   F, _),
   AGX_FMT(R32G32B32_SINT,          R32G32B32_EMULATED,    SINT,   F, _),

   /* Likewise, luminance/alpha/intensity formats are supported for texturing,
    * because they are required for texture buffers in the compat profile and
    * mesa/st is unable to emulate them for texture buffers. Our Gallium driver
    * handles the swizzles appropriately, so we just need to plumb through the
    * enums.
    *
    * If mesa/st grows emulation for these formats later, we can drop this.
    */
   AGX_FMT(A8_UNORM,                R8,                    UNORM,  F, _),
   AGX_FMT(A16_UNORM,               R16,                   UNORM,  F, _),
   AGX_FMT(A8_SINT,                 R8,                    SINT,   F, _),
   AGX_FMT(A16_SINT,                R16,                   SINT,   F, _),
   AGX_FMT(A32_SINT,                R32,                   SINT,   F, _),
   AGX_FMT(A8_UINT,                 R8,                    UINT,   F, _),
   AGX_FMT(A16_UINT,                R16,                   UINT,   F, _),
   AGX_FMT(A32_UINT,                R32,                   UINT,   F, _),
   AGX_FMT(A16_FLOAT,               R16,                   FLOAT,  F, _),
   AGX_FMT(A32_FLOAT,               R32,                   FLOAT,  F, _),

   AGX_FMT(L8_UNORM,                R8,                    UNORM,  F, _),
   AGX_FMT(L16_UNORM,               R16,                   UNORM,  F, _),
   AGX_FMT(L8_SINT,                 R8,                    SINT,   F, _),
   AGX_FMT(L16_SINT,                R16,                   SINT,   F, _),
   AGX_FMT(L32_SINT,                R32,                   SINT,   F, _),
   AGX_FMT(L8_UINT,                 R8,                    UINT,   F, _),
   AGX_FMT(L16_UINT,                R16,                   UINT,   F, _),
   AGX_FMT(L32_UINT,                R32,                   UINT,   F, _),
   AGX_FMT(L16_FLOAT,               R16,                   FLOAT,  F, _),
   AGX_FMT(L32_FLOAT,               R32,                   FLOAT,  F, _),

   AGX_FMT(L8A8_UNORM,              R8G8,                  UNORM,  F, _),
   AGX_FMT(L16A16_UNORM,            R16G16,                UNORM,  F, _),
   AGX_FMT(L8A8_SINT,               R8G8,                  SINT,   F, _),
   AGX_FMT(L16A16_SINT,             R16G16,                SINT,   F, _),
   AGX_FMT(L32A32_SINT,             R32G32,                SINT,   F, _),
   AGX_FMT(L8A8_UINT,               R8G8,                  UINT,   F, _),
   AGX_FMT(L16A16_UINT,             R16G16,                UINT,   F, _),
   AGX_FMT(L32A32_UINT,             R32G32,                UINT,   F, _),
   AGX_FMT(L16A16_FLOAT,            R16G16,                FLOAT,  F, _),
   AGX_FMT(L32A32_FLOAT,            R32G32,                FLOAT,  F, _),

   AGX_FMT(I8_UNORM,                R8,                    UNORM,  F, _),
   AGX_FMT(I16_UNORM,               R16,                   UNORM,  F, _),
   AGX_FMT(I8_SINT,                 R8,                    SINT,   F, _),
   AGX_FMT(I16_SINT,                R16,                   SINT,   F, _),
   AGX_FMT(I32_SINT,                R32,                   SINT,   F, _),
   AGX_FMT(I8_UINT,                 R8,                    UINT,   F, _),
   AGX_FMT(I16_UINT,                R16,                   UINT,   F, _),
   AGX_FMT(I32_UINT,                R32,                   UINT,   F, _),
   AGX_FMT(I16_FLOAT,               R16,                   FLOAT,  F, _),
   AGX_FMT(I32_FLOAT,               R32,                   FLOAT,  F, _),

   AGX_FMT(ETC1_RGB8,               ETC2_RGB8,     UNORM,  F,_),
   AGX_FMT(ETC2_RGB8,               ETC2_RGB8,     UNORM,  F,_),
   AGX_FMT(ETC2_SRGB8,              ETC2_RGB8,     UNORM,  F,_),
   AGX_FMT(ETC2_RGB8A1,             ETC2_RGB8A1,   UNORM,  F,_),
   AGX_FMT(ETC2_SRGB8A1,            ETC2_RGB8A1,   UNORM,  F,_),
   AGX_FMT(ETC2_RGBA8,              ETC2_RGBA8,    UNORM,  F,_),
   AGX_FMT(ETC2_SRGBA8,             ETC2_RGBA8,    UNORM,  F,_),
   AGX_FMT(ETC2_R11_UNORM,          EAC_R11,       UNORM,  F,_),
   AGX_FMT(ETC2_R11_SNORM,          EAC_R11,       SNORM,  F,_),
   AGX_FMT(ETC2_RG11_UNORM,         EAC_RG11,      UNORM,  F,_),
   AGX_FMT(ETC2_RG11_SNORM,         EAC_RG11,      SNORM,  F,_),

   AGX_FMT(ASTC_4x4,                ASTC_4X4,      UNORM,  F, _),
   AGX_FMT(ASTC_5x4,                ASTC_5X4,      UNORM,  F, _),
   AGX_FMT(ASTC_5x5,                ASTC_5X5,      UNORM,  F, _),
   AGX_FMT(ASTC_6x5,                ASTC_6X5,      UNORM,  F, _),
   AGX_FMT(ASTC_6x6,                ASTC_6X6,      UNORM,  F, _),
   AGX_FMT(ASTC_8x5,                ASTC_8X5,      UNORM,  F, _),
   AGX_FMT(ASTC_8x6,                ASTC_8X6,      UNORM,  F, _),
   AGX_FMT(ASTC_8x8,                ASTC_8X8,      UNORM,  F, _),
   AGX_FMT(ASTC_10x5,               ASTC_10X5,     UNORM,  F, _),
   AGX_FMT(ASTC_10x6,               ASTC_10X6,     UNORM,  F, _),
   AGX_FMT(ASTC_10x8,               ASTC_10X8,     UNORM,  F, _),
   AGX_FMT(ASTC_10x10,              ASTC_10X10,    UNORM,  F, _),
   AGX_FMT(ASTC_12x10,              ASTC_12X10,    UNORM,  F, _),
   AGX_FMT(ASTC_12x12,              ASTC_12X12,    UNORM,  F, _),

   AGX_FMT(ASTC_4x4_SRGB,           ASTC_4X4,      UNORM,  F, _),
   AGX_FMT(ASTC_5x4_SRGB,           ASTC_5X4,      UNORM,  F, _),
   AGX_FMT(ASTC_5x5_SRGB,           ASTC_5X5,      UNORM,  F, _),
   AGX_FMT(ASTC_6x5_SRGB,           ASTC_6X5,      UNORM,  F, _),
   AGX_FMT(ASTC_6x6_SRGB,           ASTC_6X6,      UNORM,  F, _),
   AGX_FMT(ASTC_8x5_SRGB,           ASTC_8X5,      UNORM,  F, _),
   AGX_FMT(ASTC_8x6_SRGB,           ASTC_8X6,      UNORM,  F, _),
   AGX_FMT(ASTC_8x8_SRGB,           ASTC_8X8,      UNORM,  F, _),
   AGX_FMT(ASTC_10x5_SRGB,          ASTC_10X5,     UNORM,  F, _),
   AGX_FMT(ASTC_10x6_SRGB,          ASTC_10X6,     UNORM,  F, _),
   AGX_FMT(ASTC_10x8_SRGB,          ASTC_10X8,     UNORM,  F, _),
   AGX_FMT(ASTC_10x10_SRGB,         ASTC_10X10,    UNORM,  F, _),
   AGX_FMT(ASTC_12x10_SRGB,         ASTC_12X10,    UNORM,  F, _),
   AGX_FMT(ASTC_12x12_SRGB,         ASTC_12X12,    UNORM,  F, _),

   AGX_FMT(DXT1_RGB,                BC1,           UNORM,  F, _),
   AGX_FMT(DXT1_RGBA,               BC1,           UNORM,  F, _),
   AGX_FMT(DXT1_SRGB,               BC1,           UNORM,  F, _),
   AGX_FMT(DXT1_SRGBA,              BC1,           UNORM,  F, _),
   AGX_FMT(DXT3_RGBA,               BC2,           UNORM,  F, _),
   AGX_FMT(DXT3_SRGBA,              BC2,           UNORM,  F, _),
   AGX_FMT(DXT5_RGBA,               BC3,           UNORM,  F, _),
   AGX_FMT(DXT5_SRGBA,              BC3,           UNORM,  F, _),
   AGX_FMT(RGTC1_UNORM,             BC4,           UNORM,  F, _),
   AGX_FMT(RGTC1_SNORM,             BC4,           SNORM,  F, _),
   AGX_FMT(RGTC2_UNORM,             BC5,           UNORM,  F, _),
   AGX_FMT(RGTC2_SNORM,             BC5,           SNORM,  F, _),
   AGX_FMT(BPTC_RGB_FLOAT,          BC6H,          FLOAT,  F, _),
   AGX_FMT(BPTC_RGB_UFLOAT,         BC6H_UFLOAT,   FLOAT,  F, _),
   AGX_FMT(BPTC_RGBA_UNORM,         BC7,           UNORM,  F, _),
   AGX_FMT(BPTC_SRGBA,              BC7,           UNORM,  F, _),
};
/* clang-format on */
