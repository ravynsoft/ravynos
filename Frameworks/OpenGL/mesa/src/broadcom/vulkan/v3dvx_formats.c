/*
 * Copyright © 2021 Raspberry Pi Ltd
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "v3dv_private.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/cle/v3dx_pack.h"

#include "util/format/u_format.h"
#include "vulkan/util/vk_util.h"
#include "vk_enum_to_str.h"
#include "vk_enum_defines.h"

#define SWIZ(x,y,z,w) {   \
   PIPE_SWIZZLE_##x,      \
   PIPE_SWIZZLE_##y,      \
   PIPE_SWIZZLE_##z,      \
   PIPE_SWIZZLE_##w       \
}

#define FORMAT(vk, rt, tex, swiz, return_size, supports_filtering)  \
   [VK_ENUM_OFFSET(VK_FORMAT_##vk)] = {                             \
      1,                                                            \
      {{                                                            \
         V3D_OUTPUT_IMAGE_FORMAT_##rt,                              \
         TEXTURE_DATA_FORMAT_##tex,                                 \
         swiz,                                                      \
         return_size,                                               \
      }},                                                           \
      supports_filtering,                                           \
   }

#define PLANE(rt, tex, swiz, return_size)  \
   {                                       \
      V3D_OUTPUT_IMAGE_FORMAT_##rt,        \
      TEXTURE_DATA_FORMAT_##tex,           \
      swiz,                                \
      return_size                          \
   }

#define YCBCR_FORMAT(vk, supports_filtering, plane_count, ...)  \
   [VK_ENUM_OFFSET(VK_FORMAT_##vk)] = {                         \
      plane_count,                                              \
      {                                                         \
         __VA_ARGS__,                                           \
      },                                                        \
      supports_filtering,                                       \
   }

#define SWIZ_X001 SWIZ(X, 0, 0, 1)
#define SWIZ_XY01 SWIZ(X, Y, 0, 1)
#define SWIZ_XYZ1 SWIZ(X, Y, Z, 1)
#define SWIZ_XYZW SWIZ(X, Y, Z, W)
#define SWIZ_YZWX SWIZ(Y, Z, W, X)
#define SWIZ_YZW1 SWIZ(Y, Z, W, 1)
#define SWIZ_ZYXW SWIZ(Z, Y, X, W)
#define SWIZ_ZYX1 SWIZ(Z, Y, X, 1)
#define SWIZ_XXXY SWIZ(X, X, X, Y)
#define SWIZ_XXX1 SWIZ(X, X, X, 1)
#define SWIZ_XXXX SWIZ(X, X, X, X)
#define SWIZ_000X SWIZ(0, 0, 0, X)
#define SWIZ_WXYZ SWIZ(W, X, Y, Z)
#define SWIZ_WZYX SWIZ(W, Z, Y, X)

/* FIXME: expand format table to describe whether the format is supported
 * for buffer surfaces (texel buffers, vertex buffers, etc).
 */
static const struct v3dv_format format_table[] = {
   /* Color, 4 channels */
   FORMAT(B8G8R8A8_SRGB,           SRGB8_ALPHA8, RGBA8,         SWIZ_ZYXW, 16, true),
   FORMAT(B8G8R8A8_UNORM,          RGBA8,        RGBA8,         SWIZ_ZYXW, 16, true),

   FORMAT(R8G8B8A8_SRGB,           SRGB8_ALPHA8, RGBA8,         SWIZ_XYZW, 16, true),
   FORMAT(R8G8B8A8_UNORM,          RGBA8,        RGBA8,         SWIZ_XYZW, 16, true),
   FORMAT(R8G8B8A8_SNORM,          NO,           RGBA8_SNORM,   SWIZ_XYZW, 16, true),
   FORMAT(R8G8B8A8_SINT,           RGBA8I,       RGBA8I,        SWIZ_XYZW, 16, false),
   FORMAT(R8G8B8A8_UINT,           RGBA8UI,      RGBA8UI,       SWIZ_XYZW, 16, false),

   FORMAT(R16G16B16A16_SFLOAT,     RGBA16F,      RGBA16F,       SWIZ_XYZW, 16, true),
   FORMAT(R16G16B16A16_UNORM,      NO,           RGBA16,        SWIZ_XYZW, 32, true),
   FORMAT(R16G16B16A16_SNORM,      NO,           RGBA16_SNORM,  SWIZ_XYZW, 32, true),
   FORMAT(R16G16B16A16_SINT,       RGBA16I,      RGBA16I,       SWIZ_XYZW, 16, false),
   FORMAT(R16G16B16A16_UINT,       RGBA16UI,     RGBA16UI,      SWIZ_XYZW, 16, false),

   FORMAT(R32G32B32A32_SFLOAT,     RGBA32F,      RGBA32F,       SWIZ_XYZW, 32, false),
   FORMAT(R32G32B32A32_SINT,       RGBA32I,      RGBA32I,       SWIZ_XYZW, 32, false),
   FORMAT(R32G32B32A32_UINT,       RGBA32UI,     RGBA32UI,      SWIZ_XYZW, 32, false),

   /* Color, 3 channels */
   FORMAT(R32G32B32_SFLOAT,        NO,           NO,            SWIZ_XYZ1,  0, false),
   FORMAT(R32G32B32_UINT,          NO,           NO,            SWIZ_XYZ1,  0, false),
   FORMAT(R32G32B32_SINT,          NO,           NO,            SWIZ_XYZ1,  0, false),

   /* Color, 2 channels */
   FORMAT(R8G8_UNORM,              RG8,          RG8,           SWIZ_XY01, 16, true),
   FORMAT(R8G8_SNORM,              NO,           RG8_SNORM,     SWIZ_XY01, 16, true),
   FORMAT(R8G8_SINT,               RG8I,         RG8I,          SWIZ_XY01, 16, false),
   FORMAT(R8G8_UINT,               RG8UI,        RG8UI,         SWIZ_XY01, 16, false),

   FORMAT(R16G16_UNORM,            NO,           RG16,          SWIZ_XY01, 32, true),
   FORMAT(R16G16_SNORM,            NO,           RG16_SNORM,    SWIZ_XY01, 32, true),
   FORMAT(R16G16_SFLOAT,           RG16F,        RG16F,         SWIZ_XY01, 16, true),
   FORMAT(R16G16_SINT,             RG16I,        RG16I,         SWIZ_XY01, 16, false),
   FORMAT(R16G16_UINT,             RG16UI,       RG16UI,        SWIZ_XY01, 16, false),

   FORMAT(R32G32_SFLOAT,           RG32F,        RG32F,         SWIZ_XY01, 32, false),
   FORMAT(R32G32_SINT,             RG32I,        RG32I,         SWIZ_XY01, 32, false),
   FORMAT(R32G32_UINT,             RG32UI,       RG32UI,        SWIZ_XY01, 32, false),

   /* Color, 1 channel */
   FORMAT(R8_UNORM,                R8,           R8,            SWIZ_X001, 16, true),
   FORMAT(R8_SNORM,                NO,           R8_SNORM,      SWIZ_X001, 16, true),
   FORMAT(R8_SINT,                 R8I,          R8I,           SWIZ_X001, 16, false),
   FORMAT(R8_UINT,                 R8UI,         R8UI,          SWIZ_X001, 16, false),

   FORMAT(R16_UNORM,               NO,           R16,           SWIZ_X001, 32, true),
   FORMAT(R16_SNORM,               NO,           R16_SNORM,     SWIZ_X001, 32, true),
   FORMAT(R16_SFLOAT,              R16F,         R16F,          SWIZ_X001, 16, true),
   FORMAT(R16_SINT,                R16I,         R16I,          SWIZ_X001, 16, false),
   FORMAT(R16_UINT,                R16UI,        R16UI,         SWIZ_X001, 16, false),

   FORMAT(R32_SFLOAT,              R32F,         R32F,          SWIZ_X001, 32, false),
   FORMAT(R32_SINT,                R32I,         R32I,          SWIZ_X001, 32, false),
   FORMAT(R32_UINT,                R32UI,        R32UI,         SWIZ_X001, 32, false),

   /* Color, packed */
   FORMAT(R4G4B4A4_UNORM_PACK16,   ABGR4444,     RGBA4,         SWIZ_XYZW, 16, true),
   FORMAT(B4G4R4A4_UNORM_PACK16,   ABGR4444,     RGBA4,         SWIZ_ZYXW, 16, true), /* Swap RB */
   FORMAT(R5G6B5_UNORM_PACK16,     BGR565,       RGB565,        SWIZ_XYZ1, 16, true),
   FORMAT(R5G5B5A1_UNORM_PACK16,   ABGR1555,     RGB5_A1,       SWIZ_XYZW, 16, true),
   FORMAT(A1R5G5B5_UNORM_PACK16,   RGBA5551,     A1_RGB5,       SWIZ_ZYXW, 16, true), /* Swap RB */
   FORMAT(A8B8G8R8_UNORM_PACK32,   RGBA8,        RGBA8,         SWIZ_XYZW, 16, true), /* RGBA8 UNORM */
   FORMAT(A8B8G8R8_SNORM_PACK32,   NO,           RGBA8_SNORM,   SWIZ_XYZW, 16, true), /* RGBA8 SNORM */
   FORMAT(A8B8G8R8_UINT_PACK32,    RGBA8UI,      RGBA8UI,       SWIZ_XYZW, 16, false), /* RGBA8 UINT */
   FORMAT(A8B8G8R8_SINT_PACK32,    RGBA8I,       RGBA8I,        SWIZ_XYZW, 16, false), /* RGBA8 SINT */
   FORMAT(A8B8G8R8_SRGB_PACK32,    SRGB8_ALPHA8, RGBA8,         SWIZ_XYZW, 16, true), /* RGBA8 sRGB */
   FORMAT(A2B10G10R10_UNORM_PACK32,RGB10_A2,     RGB10_A2,      SWIZ_XYZW, 16, true),
   FORMAT(A2B10G10R10_UINT_PACK32, RGB10_A2UI,   RGB10_A2UI,    SWIZ_XYZW, 16, false),
   FORMAT(A2R10G10B10_UNORM_PACK32,RGB10_A2,     RGB10_A2,      SWIZ_ZYXW, 16, true),
   FORMAT(E5B9G9R9_UFLOAT_PACK32,  NO,           RGB9_E5,       SWIZ_XYZ1, 16, true),
   FORMAT(B10G11R11_UFLOAT_PACK32, R11F_G11F_B10F,R11F_G11F_B10F, SWIZ_XYZ1, 16, true),

   /* Depth */
   FORMAT(D16_UNORM,               D16,          DEPTH_COMP16,  SWIZ_X001, 32, false),
   FORMAT(D32_SFLOAT,              D32F,         DEPTH_COMP32F, SWIZ_X001, 32, false),
   FORMAT(X8_D24_UNORM_PACK32,     D24S8,        DEPTH24_X8,    SWIZ_X001, 32, false),

   /* Depth + Stencil */
   FORMAT(D24_UNORM_S8_UINT,       D24S8,        DEPTH24_X8,    SWIZ_X001, 32, false),

   /* Compressed: ETC2 / EAC */
   FORMAT(ETC2_R8G8B8_UNORM_BLOCK,    NO,  RGB8_ETC2,                SWIZ_XYZ1, 16, true),
   FORMAT(ETC2_R8G8B8_SRGB_BLOCK,     NO,  RGB8_ETC2,                SWIZ_XYZ1, 16, true),
   FORMAT(ETC2_R8G8B8A1_UNORM_BLOCK,  NO,  RGB8_PUNCHTHROUGH_ALPHA1, SWIZ_XYZW, 16, true),
   FORMAT(ETC2_R8G8B8A1_SRGB_BLOCK,   NO,  RGB8_PUNCHTHROUGH_ALPHA1, SWIZ_XYZW, 16, true),
   FORMAT(ETC2_R8G8B8A8_UNORM_BLOCK,  NO,  RGBA8_ETC2_EAC,           SWIZ_XYZW, 16, true),
   FORMAT(ETC2_R8G8B8A8_SRGB_BLOCK,   NO,  RGBA8_ETC2_EAC,           SWIZ_XYZW, 16, true),
   FORMAT(EAC_R11_UNORM_BLOCK,        NO,  R11_EAC,                  SWIZ_X001, 16, true),
   FORMAT(EAC_R11_SNORM_BLOCK,        NO,  SIGNED_R11_EAC,           SWIZ_X001, 16, true),
   FORMAT(EAC_R11G11_UNORM_BLOCK,     NO,  RG11_EAC,                 SWIZ_XY01, 16, true),
   FORMAT(EAC_R11G11_SNORM_BLOCK,     NO,  SIGNED_RG11_EAC,          SWIZ_XY01, 16, true),

   /* Compressed: BC1-3 */
   FORMAT(BC1_RGB_UNORM_BLOCK,        NO,  BC1,                      SWIZ_XYZ1, 16, true),
   FORMAT(BC1_RGB_SRGB_BLOCK,         NO,  BC1,                      SWIZ_XYZ1, 16, true),
   FORMAT(BC1_RGBA_UNORM_BLOCK,       NO,  BC1,                      SWIZ_XYZW, 16, true),
   FORMAT(BC1_RGBA_SRGB_BLOCK,        NO,  BC1,                      SWIZ_XYZW, 16, true),
   FORMAT(BC2_UNORM_BLOCK,            NO,  BC2,                      SWIZ_XYZW, 16, true),
   FORMAT(BC2_SRGB_BLOCK,             NO,  BC2,                      SWIZ_XYZW, 16, true),
   FORMAT(BC3_UNORM_BLOCK,            NO,  BC3,                      SWIZ_XYZW, 16, true),
   FORMAT(BC3_SRGB_BLOCK,             NO,  BC3,                      SWIZ_XYZW, 16, true),

   /* Compressed: ASTC */
   FORMAT(ASTC_4x4_UNORM_BLOCK,       NO,  ASTC_4X4,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_4x4_SRGB_BLOCK,        NO,  ASTC_4X4,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_5x4_UNORM_BLOCK,       NO,  ASTC_5X4,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_5x4_SRGB_BLOCK,        NO,  ASTC_5X4,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_5x5_UNORM_BLOCK,       NO,  ASTC_5X5,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_5x5_SRGB_BLOCK,        NO,  ASTC_5X5,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_6x5_UNORM_BLOCK,       NO,  ASTC_6X5,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_6x5_SRGB_BLOCK,        NO,  ASTC_6X5,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_6x6_UNORM_BLOCK,       NO,  ASTC_6X6,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_6x6_SRGB_BLOCK,        NO,  ASTC_6X6,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_8x5_UNORM_BLOCK,       NO,  ASTC_8X5,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_8x5_SRGB_BLOCK,        NO,  ASTC_8X5,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_8x6_UNORM_BLOCK,       NO,  ASTC_8X6,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_8x6_SRGB_BLOCK,        NO,  ASTC_8X6,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_8x8_UNORM_BLOCK,       NO,  ASTC_8X8,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_8x8_SRGB_BLOCK,        NO,  ASTC_8X8,                 SWIZ_XYZW, 16, true),
   FORMAT(ASTC_10x5_UNORM_BLOCK,      NO,  ASTC_10X5,                SWIZ_XYZW, 16, true),
   FORMAT(ASTC_10x5_SRGB_BLOCK,       NO,  ASTC_10X5,                SWIZ_XYZW, 16, true),
   FORMAT(ASTC_10x6_UNORM_BLOCK,      NO,  ASTC_10X6,                SWIZ_XYZW, 16, true),
   FORMAT(ASTC_10x6_SRGB_BLOCK,       NO,  ASTC_10X6,                SWIZ_XYZW, 16, true),
   FORMAT(ASTC_10x8_UNORM_BLOCK,      NO,  ASTC_10X8,                SWIZ_XYZW, 16, true),
   FORMAT(ASTC_10x8_SRGB_BLOCK,       NO,  ASTC_10X8,                SWIZ_XYZW, 16, true),
   FORMAT(ASTC_10x10_UNORM_BLOCK,     NO,  ASTC_10X10,               SWIZ_XYZW, 16, true),
   FORMAT(ASTC_10x10_SRGB_BLOCK,      NO,  ASTC_10X10,               SWIZ_XYZW, 16, true),
   FORMAT(ASTC_12x10_UNORM_BLOCK,     NO,  ASTC_12X10,               SWIZ_XYZW, 16, true),
   FORMAT(ASTC_12x10_SRGB_BLOCK,      NO,  ASTC_12X10,               SWIZ_XYZW, 16, true),
   FORMAT(ASTC_12x12_UNORM_BLOCK,     NO,  ASTC_12X12,               SWIZ_XYZW, 16, true),
   FORMAT(ASTC_12x12_SRGB_BLOCK,      NO,  ASTC_12X12,               SWIZ_XYZW, 16, true),
};

/**
 * Vulkan layout for 4444 formats is defined like this:
 *
 * Vulkan ABGR4: (LSB) R | G | B | A (MSB)
 * Vulkan ARGB4: (LSB) B | G | R | A (MSB)
 *
 * We map this to the V3D RGB4 texture format, which really, is ABGR4 with
 * R in the MSB, so:
 *
 * V3D ABGR4   : (LSB) A | B | G | R (MSB)
 *
 * Which is reversed from Vulkan's ABGR4 layout. So in order to match Vulkan
 * semantics we need to apply the following swizzles:
 *
 * ABGR4: WZYX (reverse)
 * ARGB4: YZWX (reverse + swap R/B)
 */
static const struct v3dv_format format_table_4444[] = {
   FORMAT(A4B4G4R4_UNORM_PACK16, ABGR4444, RGBA4, SWIZ_WZYX, 16, true), /* Reverse */
   FORMAT(A4R4G4B4_UNORM_PACK16, ABGR4444, RGBA4, SWIZ_YZWX, 16, true), /* Reverse + RB swap */
};

static const struct v3dv_format format_table_ycbcr[] = {
   YCBCR_FORMAT(G8_B8R8_2PLANE_420_UNORM, false, 2,
       PLANE(R8, R8, SWIZ(X, 0, 0, 1), 16),
       PLANE(RG8, RG8, SWIZ(X, Y, 0, 1), 16)
   ),
   YCBCR_FORMAT(G8_B8_R8_3PLANE_420_UNORM, false, 3,
       PLANE(R8, R8, SWIZ(X, 0, 0, 1), 16),
       PLANE(R8, R8, SWIZ(X, 0, 0, 1), 16),
       PLANE(R8, R8, SWIZ(X, 0, 0, 1), 16)
   ),
};

const struct v3dv_format *
v3dX(get_format)(VkFormat format)
{
   /* Core formats */
   if (format < ARRAY_SIZE(format_table) && format_table[format].plane_count)
      return &format_table[format];

   uint32_t ext_number = VK_ENUM_EXTENSION(format);
   uint32_t enum_offset = VK_ENUM_OFFSET(format);

   switch (ext_number) {
   case _VK_EXT_4444_formats_number:
      return &format_table_4444[enum_offset];
   case _VK_KHR_sampler_ycbcr_conversion_number:
      if (enum_offset < ARRAY_SIZE(format_table_ycbcr))
         return &format_table_ycbcr[enum_offset];
      else
         return NULL;
   default:
      return NULL;
   }
}

void
v3dX(get_internal_type_bpp_for_output_format)(uint32_t format,
                                              uint32_t *type,
                                              uint32_t *bpp)
{
   switch (format) {
   case V3D_OUTPUT_IMAGE_FORMAT_RGBA8:
   case V3D_OUTPUT_IMAGE_FORMAT_RGB8:
   case V3D_OUTPUT_IMAGE_FORMAT_RG8:
   case V3D_OUTPUT_IMAGE_FORMAT_R8:
   case V3D_OUTPUT_IMAGE_FORMAT_ABGR4444:
   case V3D_OUTPUT_IMAGE_FORMAT_BGR565:
   case V3D_OUTPUT_IMAGE_FORMAT_ABGR1555:
      *type = V3D_INTERNAL_TYPE_8;
      *bpp = V3D_INTERNAL_BPP_32;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RGBA8I:
   case V3D_OUTPUT_IMAGE_FORMAT_RG8I:
   case V3D_OUTPUT_IMAGE_FORMAT_R8I:
      *type = V3D_INTERNAL_TYPE_8I;
      *bpp = V3D_INTERNAL_BPP_32;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RGBA8UI:
   case V3D_OUTPUT_IMAGE_FORMAT_RG8UI:
   case V3D_OUTPUT_IMAGE_FORMAT_R8UI:
      *type = V3D_INTERNAL_TYPE_8UI;
      *bpp = V3D_INTERNAL_BPP_32;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_SRGB8_ALPHA8:
   case V3D_OUTPUT_IMAGE_FORMAT_SRGB:
   case V3D_OUTPUT_IMAGE_FORMAT_RGB10_A2:
   case V3D_OUTPUT_IMAGE_FORMAT_R11F_G11F_B10F:
   case V3D_OUTPUT_IMAGE_FORMAT_RGBA16F:
      /* Note that sRGB RTs are stored in the tile buffer at 16F,
       * and the conversion to sRGB happens at tilebuffer load/store.
       */
      *type = V3D_INTERNAL_TYPE_16F;
      *bpp = V3D_INTERNAL_BPP_64;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RG16F:
   case V3D_OUTPUT_IMAGE_FORMAT_R16F:
      *type = V3D_INTERNAL_TYPE_16F;
      /* Use 64bpp to make sure the TLB doesn't throw away the alpha
       * channel before alpha test happens.
       */
      *bpp = V3D_INTERNAL_BPP_64;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RGBA16I:
      *type = V3D_INTERNAL_TYPE_16I;
      *bpp = V3D_INTERNAL_BPP_64;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RG16I:
   case V3D_OUTPUT_IMAGE_FORMAT_R16I:
      *type = V3D_INTERNAL_TYPE_16I;
      *bpp = V3D_INTERNAL_BPP_32;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RGB10_A2UI:
   case V3D_OUTPUT_IMAGE_FORMAT_RGBA16UI:
      *type = V3D_INTERNAL_TYPE_16UI;
      *bpp = V3D_INTERNAL_BPP_64;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RG16UI:
   case V3D_OUTPUT_IMAGE_FORMAT_R16UI:
      *type = V3D_INTERNAL_TYPE_16UI;
      *bpp = V3D_INTERNAL_BPP_32;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RGBA32I:
      *type = V3D_INTERNAL_TYPE_32I;
      *bpp = V3D_INTERNAL_BPP_128;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RG32I:
      *type = V3D_INTERNAL_TYPE_32I;
      *bpp = V3D_INTERNAL_BPP_64;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_R32I:
      *type = V3D_INTERNAL_TYPE_32I;
      *bpp = V3D_INTERNAL_BPP_32;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RGBA32UI:
      *type = V3D_INTERNAL_TYPE_32UI;
      *bpp = V3D_INTERNAL_BPP_128;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RG32UI:
      *type = V3D_INTERNAL_TYPE_32UI;
      *bpp = V3D_INTERNAL_BPP_64;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_R32UI:
      *type = V3D_INTERNAL_TYPE_32UI;
      *bpp = V3D_INTERNAL_BPP_32;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RGBA32F:
      *type = V3D_INTERNAL_TYPE_32F;
      *bpp = V3D_INTERNAL_BPP_128;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_RG32F:
      *type = V3D_INTERNAL_TYPE_32F;
      *bpp = V3D_INTERNAL_BPP_64;
      break;

   case V3D_OUTPUT_IMAGE_FORMAT_R32F:
      *type = V3D_INTERNAL_TYPE_32F;
      *bpp = V3D_INTERNAL_BPP_32;
      break;

   default:
      /* Provide some default values, as we'll be called at RB
       * creation time, even if an RB with this format isn't supported.
       */
      *type = V3D_INTERNAL_TYPE_8;
      *bpp = V3D_INTERNAL_BPP_32;
      break;
   }
}

bool
v3dX(format_supports_tlb_resolve)(const struct v3dv_format *format)
{
   uint32_t type, bpp;

   /* Multiplanar images cannot be multisampled:
    *
    *   "sampleCounts will be set to VK_SAMPLE_COUNT_1_BIT if at least one of
    *    the following conditions is true: (...) format is one of the formats
    *    that require a sampler Y′CBCR conversion (...)"
    */
   if (!format->plane_count || format->plane_count > 1)
      return false;

   v3dX(get_internal_type_bpp_for_output_format)(format->planes[0].rt_type, &type, &bpp);
   return type == V3D_INTERNAL_TYPE_8 || type == V3D_INTERNAL_TYPE_16F;
}

bool
v3dX(format_supports_blending)(const struct v3dv_format *format)
{
   /* ycbcr formats don't support blending */
   if (!format->plane_count || format->plane_count > 1)
      return false;

   /* Hardware blending is only supported on render targets that are configured
    * 4x8-bit unorm, 2x16-bit float or 4x16-bit float.
    */
   uint32_t type, bpp;
   v3dX(get_internal_type_bpp_for_output_format)(format->planes[0].rt_type, &type, &bpp);
   switch (type) {
   case V3D_INTERNAL_TYPE_8:
      return bpp == V3D_INTERNAL_BPP_32;
   case V3D_INTERNAL_TYPE_16F:
      return bpp == V3D_INTERNAL_BPP_32 || V3D_INTERNAL_BPP_64;
   default:
      return false;
   }
}

bool
v3dX(tfu_supports_tex_format)(uint32_t tex_format)
{
   switch (tex_format) {
   case TEXTURE_DATA_FORMAT_R8:
   case TEXTURE_DATA_FORMAT_R8_SNORM:
   case TEXTURE_DATA_FORMAT_RG8:
   case TEXTURE_DATA_FORMAT_RG8_SNORM:
   case TEXTURE_DATA_FORMAT_RGBA8:
   case TEXTURE_DATA_FORMAT_RGBA8_SNORM:
   case TEXTURE_DATA_FORMAT_RGB565:
   case TEXTURE_DATA_FORMAT_RGBA4:
   case TEXTURE_DATA_FORMAT_RGB5_A1:
   case TEXTURE_DATA_FORMAT_RGB10_A2:
   case TEXTURE_DATA_FORMAT_R16:
   case TEXTURE_DATA_FORMAT_R16_SNORM:
   case TEXTURE_DATA_FORMAT_RG16:
   case TEXTURE_DATA_FORMAT_RG16_SNORM:
   case TEXTURE_DATA_FORMAT_RGBA16:
   case TEXTURE_DATA_FORMAT_RGBA16_SNORM:
   case TEXTURE_DATA_FORMAT_R16F:
   case TEXTURE_DATA_FORMAT_RG16F:
   case TEXTURE_DATA_FORMAT_RGBA16F:
   case TEXTURE_DATA_FORMAT_R11F_G11F_B10F:
   case TEXTURE_DATA_FORMAT_R4:
   case TEXTURE_DATA_FORMAT_RGB9_E5:
   case TEXTURE_DATA_FORMAT_R32F:
   case TEXTURE_DATA_FORMAT_RG32F:
   case TEXTURE_DATA_FORMAT_RGBA32F:
   case TEXTURE_DATA_FORMAT_RGB8_ETC2:
   case TEXTURE_DATA_FORMAT_RGB8_PUNCHTHROUGH_ALPHA1:
   case TEXTURE_DATA_FORMAT_RGBA8_ETC2_EAC:
   case TEXTURE_DATA_FORMAT_R11_EAC:
   case TEXTURE_DATA_FORMAT_SIGNED_R11_EAC:
   case TEXTURE_DATA_FORMAT_RG11_EAC:
   case TEXTURE_DATA_FORMAT_SIGNED_RG11_EAC:
      return true;
   default:
      return false;
   }
}

uint8_t
v3dX(get_internal_depth_type)(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_D16_UNORM:
      return V3D_INTERNAL_TYPE_DEPTH_16;
   case VK_FORMAT_D32_SFLOAT:
      return V3D_INTERNAL_TYPE_DEPTH_32F;
   case VK_FORMAT_X8_D24_UNORM_PACK32:
   case VK_FORMAT_D24_UNORM_S8_UINT:
      return V3D_INTERNAL_TYPE_DEPTH_24;
   default:
      unreachable("Invalid depth format");
      break;
   }
}

void
v3dX(get_internal_type_bpp_for_image_aspects)(VkFormat vk_format,
                                              VkImageAspectFlags aspect_mask,
                                              uint32_t *internal_type,
                                              uint32_t *internal_bpp)
{
   /* We can't store depth/stencil pixel formats to a raster format, so
    * instead we load our depth/stencil aspects to a compatible color format.
    */
   if (aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
      *internal_bpp = V3D_INTERNAL_BPP_32;
      switch (vk_format) {
      case VK_FORMAT_D16_UNORM:
         *internal_type = V3D_INTERNAL_TYPE_16UI;
         break;
      case VK_FORMAT_D32_SFLOAT:
         *internal_type = V3D_INTERNAL_TYPE_32F;
         break;
      case VK_FORMAT_X8_D24_UNORM_PACK32:
      case VK_FORMAT_D24_UNORM_S8_UINT:
         /* Use RGBA8 format so we can relocate the X/S bits in the appropriate
          * place to match Vulkan expectations. See the comment on the tile
          * load command for more details.
          */
         *internal_type = V3D_INTERNAL_TYPE_8UI;
         break;
      default:
         assert(!"unsupported format");
         break;
      }
   } else {
      const struct v3dv_format *format = v3dX(get_format)(vk_format);
      /* We only expect this to be called for single-plane formats */
      assert(format->plane_count == 1);
      v3dX(get_internal_type_bpp_for_output_format)(format->planes[0].rt_type,
                                                    internal_type, internal_bpp);
   }
}
