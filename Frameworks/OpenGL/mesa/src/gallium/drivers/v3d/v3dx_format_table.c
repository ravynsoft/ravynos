/*
 * Copyright © 2014-2018 Broadcom
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

#include "util/format/u_format.h"

#include "v3d_context.h"
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/common/v3d_macros.h"
#include "v3d_format_table.h"

#define SWIZ(x,y,z,w) {          \
        PIPE_SWIZZLE_##x, \
        PIPE_SWIZZLE_##y, \
        PIPE_SWIZZLE_##z, \
        PIPE_SWIZZLE_##w  \
}

#define FORMAT(pipe, rt, tex, swiz, return_size, return_channels)       \
        [PIPE_FORMAT_##pipe] = {                                        \
                true,                                                   \
                V3D_OUTPUT_IMAGE_FORMAT_##rt,                           \
                TEXTURE_DATA_FORMAT_##tex,                              \
                swiz,                                                   \
                return_size,                                            \
                return_channels,                                        \
        }

#define SWIZ_X001	SWIZ(X, 0, 0, 1)
#define SWIZ_XY01	SWIZ(X, Y, 0, 1)
#define SWIZ_XYZ1	SWIZ(X, Y, Z, 1)
#define SWIZ_XYZW	SWIZ(X, Y, Z, W)
#define SWIZ_YZWX	SWIZ(Y, Z, W, X)
#define SWIZ_YZW1	SWIZ(Y, Z, W, 1)
#define SWIZ_ZYXW	SWIZ(Z, Y, X, W)
#define SWIZ_ZYX1	SWIZ(Z, Y, X, 1)
#define SWIZ_XXXY	SWIZ(X, X, X, Y)
#define SWIZ_XXX1	SWIZ(X, X, X, 1)
#define SWIZ_XXXX	SWIZ(X, X, X, X)
#define SWIZ_000X	SWIZ(0, 0, 0, X)

static const struct v3d_format format_table[] = {
        FORMAT(B8G8R8A8_UNORM,    RGBA8,        RGBA8,       SWIZ_ZYXW, 16, 0),
        FORMAT(B8G8R8X8_UNORM,    RGBA8,        RGBA8,       SWIZ_ZYX1, 16, 0),
        FORMAT(B8G8R8A8_SRGB,     SRGB8_ALPHA8, RGBA8,       SWIZ_ZYXW, 16, 0),
        FORMAT(B8G8R8X8_SRGB,     SRGB8_ALPHA8, RGBA8,       SWIZ_ZYX1, 16, 0),
        FORMAT(R8G8B8A8_UNORM,    RGBA8,        RGBA8,       SWIZ_XYZW, 16, 0),
        FORMAT(R8G8B8X8_UNORM,    RGBA8,        RGBA8,       SWIZ_XYZ1, 16, 0),
        FORMAT(R8G8B8A8_SRGB,     SRGB8_ALPHA8, RGBA8,       SWIZ_XYZW, 16, 0),
        FORMAT(R8G8B8X8_SRGB,     SRGB8_ALPHA8, RGBA8,       SWIZ_XYZ1, 16, 0),
        FORMAT(R8G8B8A8_SNORM,    NO,           RGBA8_SNORM, SWIZ_XYZW, 16, 0),
        FORMAT(R8G8B8X8_SNORM,    NO,           RGBA8_SNORM, SWIZ_XYZ1, 16, 0),
        FORMAT(R10G10B10A2_UNORM, RGB10_A2,     RGB10_A2,    SWIZ_XYZW, 16, 0),
        FORMAT(B10G10R10A2_UNORM, RGB10_A2,     RGB10_A2,    SWIZ_ZYXW, 16, 0),
        FORMAT(R10G10B10X2_UNORM, RGB10_A2,     RGB10_A2,    SWIZ_XYZ1, 16, 0),
        FORMAT(B10G10R10X2_UNORM, RGB10_A2,     RGB10_A2,    SWIZ_ZYX1, 16, 0),
        FORMAT(R10G10B10A2_UINT,  RGB10_A2UI,   RGB10_A2UI,  SWIZ_XYZW, 16, 0),

        FORMAT(A4B4G4R4_UNORM,    ABGR4444,     RGBA4,       SWIZ_XYZW, 16, 0),

        FORMAT(A1B5G5R5_UNORM,    ABGR1555,     RGB5_A1,     SWIZ_XYZW, 16, 0),
        FORMAT(X1B5G5R5_UNORM,    ABGR1555,     RGB5_A1,     SWIZ_XYZ1, 16, 0),
        FORMAT(B5G6R5_UNORM,      BGR565,       RGB565,      SWIZ_XYZ1, 16, 0),

        FORMAT(R8_UNORM,          R8,           R8,          SWIZ_X001, 16, 0),
        FORMAT(R8_SNORM,          NO,           R8_SNORM,    SWIZ_X001, 16, 0),
        FORMAT(R8G8_UNORM,        RG8,          RG8,         SWIZ_XY01, 16, 0),
        FORMAT(R8G8_SNORM,        NO,           RG8_SNORM,   SWIZ_XY01, 16, 0),

        FORMAT(R16_UNORM,         NO,           R16,         SWIZ_X001, 32, 1),
        FORMAT(R16_SNORM,         NO,           R16_SNORM,   SWIZ_X001, 32, 1),
        FORMAT(R16_FLOAT,         R16F,         R16F,        SWIZ_X001, 16, 0),
        FORMAT(R32_FLOAT,         R32F,         R32F,        SWIZ_X001, 32, 1),

        FORMAT(R16G16_UNORM,      NO,           RG16,        SWIZ_XY01, 32, 2),
        FORMAT(R16G16_SNORM,      NO,           RG16_SNORM,  SWIZ_XY01, 32, 2),
        FORMAT(R16G16_FLOAT,      RG16F,        RG16F,       SWIZ_XY01, 16, 0),
        FORMAT(R32G32_FLOAT,      RG32F,        RG32F,       SWIZ_XY01, 32, 2),

        FORMAT(R16G16B16A16_UNORM, NO,          RGBA16,      SWIZ_XYZW, 32, 4),
        FORMAT(R16G16B16A16_SNORM, NO,          RGBA16_SNORM, SWIZ_XYZW, 32, 4),
        FORMAT(R16G16B16A16_FLOAT, RGBA16F,     RGBA16F,     SWIZ_XYZW, 16, 0),
        FORMAT(R32G32B32A32_FLOAT, RGBA32F,     RGBA32F,     SWIZ_XYZW, 32, 4),

        /* If we don't have L/A/LA16, mesa/st will fall back to RGBA16. */
        FORMAT(L16_UNORM,         NO,           R16,         SWIZ_XXX1, 32, 1),
        FORMAT(L16_SNORM,         NO,           R16_SNORM,   SWIZ_XXX1, 32, 1),
        FORMAT(I16_UNORM,         NO,           R16,         SWIZ_XXXX, 32, 1),
        FORMAT(I16_SNORM,         NO,           R16_SNORM,   SWIZ_XXXX, 32, 1),
        FORMAT(A16_UNORM,         NO,           R16,         SWIZ_000X, 32, 1),
        FORMAT(A16_SNORM,         NO,           R16_SNORM,   SWIZ_000X, 32, 1),
        FORMAT(L16A16_UNORM,      NO,           RG16,        SWIZ_XXXY, 32, 2),
        FORMAT(L16A16_SNORM,      NO,           RG16_SNORM,  SWIZ_XXXY, 32, 2),

        FORMAT(A8_UNORM,          NO,           R8,          SWIZ_000X, 16, 0),
        FORMAT(L8_UNORM,          NO,           R8,          SWIZ_XXX1, 16, 0),
        FORMAT(I8_UNORM,          NO,           R8,          SWIZ_XXXX, 16, 0),
        FORMAT(L8A8_UNORM,        NO,           RG8,         SWIZ_XXXY, 16, 0),

        FORMAT(R8_SINT,           R8I,          R8I,         SWIZ_X001, 16, 0),
        FORMAT(R8_UINT,           R8UI,         R8UI,        SWIZ_X001, 16, 0),
        FORMAT(R8G8_SINT,         RG8I,         RG8I,        SWIZ_XY01, 16, 0),
        FORMAT(R8G8_UINT,         RG8UI,        RG8UI,       SWIZ_XY01, 16, 0),
        FORMAT(R8G8B8A8_SINT,     RGBA8I,       RGBA8I,      SWIZ_XYZW, 16, 0),
        FORMAT(R8G8B8A8_UINT,     RGBA8UI,      RGBA8UI,     SWIZ_XYZW, 16, 0),

        FORMAT(R16_SINT,          R16I,         R16I,        SWIZ_X001, 16, 0),
        FORMAT(R16_UINT,          R16UI,        R16UI,       SWIZ_X001, 16, 0),
        FORMAT(R16G16_SINT,       RG16I,        RG16I,       SWIZ_XY01, 16, 0),
        FORMAT(R16G16_UINT,       RG16UI,       RG16UI,      SWIZ_XY01, 16, 0),
        FORMAT(R16G16B16A16_SINT, RGBA16I,      RGBA16I,     SWIZ_XYZW, 16, 0),
        FORMAT(R16G16B16A16_UINT, RGBA16UI,     RGBA16UI,    SWIZ_XYZW, 16, 0),

        FORMAT(R32_SINT,          R32I,         R32I,        SWIZ_X001, 32, 1),
        FORMAT(R32_UINT,          R32UI,        R32UI,       SWIZ_X001, 32, 1),
        FORMAT(R32G32_SINT,       RG32I,        RG32I,       SWIZ_XY01, 32, 2),
        FORMAT(R32G32_UINT,       RG32UI,       RG32UI,      SWIZ_XY01, 32, 2),
        FORMAT(R32G32B32A32_SINT, RGBA32I,      RGBA32I,     SWIZ_XYZW, 32, 4),
        FORMAT(R32G32B32A32_UINT, RGBA32UI,     RGBA32UI,    SWIZ_XYZW, 32, 4),

        FORMAT(A8_SINT,           R8I,          R8I,         SWIZ_000X, 16, 0),
        FORMAT(A8_UINT,           R8UI,         R8UI,        SWIZ_000X, 16, 0),
        FORMAT(A16_SINT,          R16I,         R16I,        SWIZ_000X, 16, 0),
        FORMAT(A16_UINT,          R16UI,        R16UI,       SWIZ_000X, 16, 0),
        FORMAT(A32_SINT,          R32I,         R32I,        SWIZ_000X, 32, 1),
        FORMAT(A32_UINT,          R32UI,        R32UI,       SWIZ_000X, 32, 1),

        FORMAT(R11G11B10_FLOAT,   R11F_G11F_B10F, R11F_G11F_B10F, SWIZ_XYZ1, 16, 0),
        FORMAT(R9G9B9E5_FLOAT,    NO,           RGB9_E5,     SWIZ_XYZ1, 16, 0),

        FORMAT(S8_UINT_Z24_UNORM, D24S8,        DEPTH24_X8,  SWIZ_XXXX, 32, 1),
        FORMAT(X8Z24_UNORM,       D24S8,        DEPTH24_X8,  SWIZ_XXXX, 32, 1),
        FORMAT(S8X24_UINT,        S8,           RGBA8UI, SWIZ_XXXX, 16, 1),
        FORMAT(Z32_FLOAT,         D32F,         DEPTH_COMP32F, SWIZ_XXXX, 32, 1),
        FORMAT(Z16_UNORM,         D16,          DEPTH_COMP16,SWIZ_XXXX, 32, 1),

        /* Pretend we support this, but it'll be separate Z32F depth and S8. */
        FORMAT(Z32_FLOAT_S8X24_UINT, D32F,      DEPTH_COMP32F, SWIZ_XXXX, 32, 1),
        FORMAT(X32_S8X24_UINT,    S8,           R8UI,          SWIZ_XXXX, 16, 1),

        FORMAT(ETC2_RGB8,         NO,           RGB8_ETC2,   SWIZ_XYZ1, 16, 0),
        FORMAT(ETC2_SRGB8,        NO,           RGB8_ETC2,   SWIZ_XYZ1, 16, 0),
        FORMAT(ETC2_RGB8A1,       NO,           RGB8_PUNCHTHROUGH_ALPHA1, SWIZ_XYZW, 16, 0),
        FORMAT(ETC2_SRGB8A1,      NO,           RGB8_PUNCHTHROUGH_ALPHA1, SWIZ_XYZW, 16, 0),
        FORMAT(ETC2_RGBA8,        NO,           RGBA8_ETC2_EAC, SWIZ_XYZW, 16, 0),
        FORMAT(ETC2_SRGBA8,       NO,           RGBA8_ETC2_EAC, SWIZ_XYZW, 16, 0),
        FORMAT(ETC2_R11_UNORM,    NO,           R11_EAC,     SWIZ_X001, 16, 0),
        FORMAT(ETC2_R11_SNORM,    NO,           SIGNED_R11_EAC, SWIZ_X001, 16, 0),
        FORMAT(ETC2_RG11_UNORM,   NO,           RG11_EAC,    SWIZ_XY01, 16, 0),
        FORMAT(ETC2_RG11_SNORM,   NO,           SIGNED_RG11_EAC, SWIZ_XY01, 16, 0),

        FORMAT(DXT1_RGB,          NO,           BC1,         SWIZ_XYZ1, 16, 0),
        FORMAT(DXT1_SRGB,         NO,           BC1,         SWIZ_XYZ1, 16, 0),
        FORMAT(DXT1_RGBA,         NO,           BC1,         SWIZ_XYZW, 16, 0),
        FORMAT(DXT1_SRGBA,        NO,           BC1,         SWIZ_XYZW, 16, 0),
        FORMAT(DXT3_RGBA,         NO,           BC2,         SWIZ_XYZW, 16, 0),
        FORMAT(DXT3_SRGBA,        NO,           BC2,         SWIZ_XYZW, 16, 0),
        FORMAT(DXT5_RGBA,         NO,           BC3,         SWIZ_XYZW, 16, 0),
        FORMAT(DXT5_SRGBA,        NO,           BC3,         SWIZ_XYZW, 16, 0),

        /* Compressed: ASTC */
        FORMAT(ASTC_4x4,          NO,           ASTC_4X4,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_4x4_SRGB,     NO,           ASTC_4X4,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_5x4,          NO,           ASTC_5X4,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_5x4_SRGB,     NO,           ASTC_5X4,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_5x5,          NO,           ASTC_5X5,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_5x5_SRGB,     NO,           ASTC_5X5,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_6x5,          NO,           ASTC_6X5,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_6x5_SRGB,     NO,           ASTC_6X5,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_6x6,          NO,           ASTC_6X6,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_6x6_SRGB,     NO,           ASTC_6X6,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_8x5,          NO,           ASTC_8X5,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_8x5_SRGB,     NO,           ASTC_8X5,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_8x6,          NO,           ASTC_8X6,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_8x6_SRGB,     NO,           ASTC_8X6,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_8x8,          NO,           ASTC_8X8,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_8x8_SRGB,     NO,           ASTC_8X8,    SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_10x5,         NO,           ASTC_10X5,   SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_10x5_SRGB,    NO,           ASTC_10X5,   SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_10x6,         NO,           ASTC_10X6,   SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_10x6_SRGB,    NO,           ASTC_10X6,   SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_10x8,         NO,           ASTC_10X8,   SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_10x8_SRGB,    NO,           ASTC_10X8,   SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_10x10,        NO,           ASTC_10X10,  SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_10x10_SRGB,   NO,           ASTC_10X10,  SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_12x10,        NO,           ASTC_12X10,  SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_12x10_SRGB,   NO,           ASTC_12X10,  SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_12x12,        NO,           ASTC_12X12,  SWIZ_XYZW, 16, 1),
        FORMAT(ASTC_12x12_SRGB,   NO,           ASTC_12X12,  SWIZ_XYZW, 16, 1),
};

const struct v3d_format *
v3dX(get_format_desc)(enum pipe_format f)
{
        if (f < ARRAY_SIZE(format_table) && format_table[f].present)
                return &format_table[f];
        else
                return NULL;
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
                 * and the conversion to sRGB happens at tilebuffer
                 * load/store.
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
                 * creation time, even if an RB with this format isn't
                 * supported.
                 */
                *type = V3D_INTERNAL_TYPE_8;
                *bpp = V3D_INTERNAL_BPP_32;
                break;
        }
}

bool
v3dX(tfu_supports_tex_format)(uint32_t tex_format,
                              bool for_mipmap)
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
                return true;
        case TEXTURE_DATA_FORMAT_RGB9_E5:
        case TEXTURE_DATA_FORMAT_R32F:
        case TEXTURE_DATA_FORMAT_RG32F:
        case TEXTURE_DATA_FORMAT_RGBA32F:
                return !for_mipmap;
        default:
                return false;
        }
}
