/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

/* Always include headers in the reverse order!! ~ M. */
#include "r300_texture.h"

#include "r300_context.h"
#include "r300_reg.h"
#include "r300_texture_desc.h"
#include "r300_transfer.h"
#include "r300_screen.h"

#include "util/format/u_format.h"
#include "util/format/u_format_s3tc.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "pipe/p_screen.h"
#include "frontend/winsys_handle.h"

/* These formats are supported by swapping their bytes.
 * The swizzles must be set exactly like their non-swapped counterparts,
 * because byte-swapping is what reverses the component order, not swizzling.
 *
 * This function returns the format that must be used to program CB and TX
 * swizzles.
 */
static enum pipe_format r300_unbyteswap_array_format(enum pipe_format format)
{
    /* FIXME: Disabled on little endian because of a reported regression:
     * https://bugs.freedesktop.org/show_bug.cgi?id=98869 */
    if (PIPE_ENDIAN_NATIVE != PIPE_ENDIAN_BIG)
        return format;

    /* Only BGRA 8888 array formats are supported for simplicity of
     * the implementation. */
    switch (format) {
    case PIPE_FORMAT_A8R8G8B8_UNORM:
        return PIPE_FORMAT_B8G8R8A8_UNORM;
    case PIPE_FORMAT_A8R8G8B8_SRGB:
        return PIPE_FORMAT_B8G8R8A8_SRGB;
    case PIPE_FORMAT_X8R8G8B8_UNORM:
        return PIPE_FORMAT_B8G8R8X8_UNORM;
    case PIPE_FORMAT_X8R8G8B8_SRGB:
        return PIPE_FORMAT_B8G8R8X8_SRGB;
    default:
        return format;
    }
}

static unsigned r300_get_endian_swap(enum pipe_format format)
{
    const struct util_format_description *desc;
    unsigned swap_size;

    if (r300_unbyteswap_array_format(format) != format)
        return R300_SURF_DWORD_SWAP;

    if (PIPE_ENDIAN_NATIVE != PIPE_ENDIAN_BIG)
        return R300_SURF_NO_SWAP;

    desc = util_format_description(format);

    /* Compressed formats should be in the little endian format. */
    if (desc->block.width != 1 || desc->block.height != 1)
        return R300_SURF_NO_SWAP;

    swap_size = desc->is_array ? desc->channel[0].size : desc->block.bits;

    switch (swap_size) {
    default: /* shouldn't happen? */
    case 8:
        return R300_SURF_NO_SWAP;
    case 16:
        return R300_SURF_WORD_SWAP;
    case 32:
        return R300_SURF_DWORD_SWAP;
    }
}

unsigned r300_get_swizzle_combined(const unsigned char *swizzle_format,
                                   const unsigned char *swizzle_view,
                                   bool dxtc_swizzle)
{
    unsigned i;
    unsigned char swizzle[4];
    unsigned result = 0;
    const uint32_t swizzle_shift[4] = {
        R300_TX_FORMAT_R_SHIFT,
        R300_TX_FORMAT_G_SHIFT,
        R300_TX_FORMAT_B_SHIFT,
        R300_TX_FORMAT_A_SHIFT
    };
    uint32_t swizzle_bit[4] = {
        dxtc_swizzle ? R300_TX_FORMAT_Z : R300_TX_FORMAT_X,
        R300_TX_FORMAT_Y,
        dxtc_swizzle ? R300_TX_FORMAT_X : R300_TX_FORMAT_Z,
        R300_TX_FORMAT_W
    };

    if (swizzle_view) {
        /* Combine two sets of swizzles. */
        util_format_compose_swizzles(swizzle_format, swizzle_view, swizzle);
    } else {
        memcpy(swizzle, swizzle_format, 4);
    }

    /* Get swizzle. */
    for (i = 0; i < 4; i++) {
        switch (swizzle[i]) {
            case PIPE_SWIZZLE_Y:
                result |= swizzle_bit[1] << swizzle_shift[i];
                break;
            case PIPE_SWIZZLE_Z:
                result |= swizzle_bit[2] << swizzle_shift[i];
                break;
            case PIPE_SWIZZLE_W:
                result |= swizzle_bit[3] << swizzle_shift[i];
                break;
            case PIPE_SWIZZLE_0:
                result |= R300_TX_FORMAT_ZERO << swizzle_shift[i];
                break;
            case PIPE_SWIZZLE_1:
                result |= R300_TX_FORMAT_ONE << swizzle_shift[i];
                break;
            default: /* PIPE_SWIZZLE_X */
                result |= swizzle_bit[0] << swizzle_shift[i];
        }
    }
    return result;
}

/* Translate a pipe_format into a useful texture format for sampling.
 *
 * Some special formats are translated directly using R300_EASY_TX_FORMAT,
 * but the majority of them is translated in a generic way, automatically
 * supporting all the formats hw can support.
 *
 * R300_EASY_TX_FORMAT swizzles the texture.
 * Note the signature of R300_EASY_TX_FORMAT:
 *   R300_EASY_TX_FORMAT(B, G, R, A, FORMAT);
 *
 * The FORMAT specifies how the texture sampler will treat the texture, and
 * makes available X, Y, Z, W, ZERO, and ONE for swizzling. */
uint32_t r300_translate_texformat(enum pipe_format format,
                                  const unsigned char *swizzle_view,
                                  bool is_r500,
                                  bool dxtc_swizzle)
{
    uint32_t result = 0;
    const struct util_format_description *desc;
    int i;
    bool uniform = true;
    const uint32_t sign_bit[4] = {
        R300_TX_FORMAT_SIGNED_W,
        R300_TX_FORMAT_SIGNED_Z,
        R300_TX_FORMAT_SIGNED_Y,
        R300_TX_FORMAT_SIGNED_X,
    };

    format = r300_unbyteswap_array_format(format);
    desc = util_format_description(format);

    /* Colorspace (return non-RGB formats directly). */
    switch (desc->colorspace) {
        /* Depth stencil formats.
         * Swizzles are added in r300_merge_textures_and_samplers. */
        case UTIL_FORMAT_COLORSPACE_ZS:
            switch (format) {
                case PIPE_FORMAT_Z16_UNORM:
                    return R300_TX_FORMAT_X16;
                case PIPE_FORMAT_X8Z24_UNORM:
                case PIPE_FORMAT_S8_UINT_Z24_UNORM:
                    if (is_r500)
                        return R500_TX_FORMAT_Y8X24;
                    else
                        return R300_TX_FORMAT_Y16X16;
                default:
                    return ~0; /* Unsupported. */
            }

        /* YUV formats. */
        case UTIL_FORMAT_COLORSPACE_YUV:
            result |= R300_TX_FORMAT_YUV_TO_RGB;

            switch (format) {
                case PIPE_FORMAT_UYVY:
                    return R300_EASY_TX_FORMAT(X, Y, Z, ONE, YVYU422) | result;
                case PIPE_FORMAT_YUYV:
                    return R300_EASY_TX_FORMAT(X, Y, Z, ONE, VYUY422) | result;
                default:
                    return ~0; /* Unsupported/unknown. */
            }

        /* Add gamma correction. */
        case UTIL_FORMAT_COLORSPACE_SRGB:
            result |= R300_TX_FORMAT_GAMMA;
            break;

        default:
            switch (format) {
                /* Same as YUV but without the YUR->RGB conversion. */
                case PIPE_FORMAT_R8G8_B8G8_UNORM:
                    return R300_EASY_TX_FORMAT(X, Y, Z, ONE, YVYU422) | result;
                case PIPE_FORMAT_G8R8_G8B8_UNORM:
                    return R300_EASY_TX_FORMAT(X, Y, Z, ONE, VYUY422) | result;
                default:;
            }
    }

    /* Add swizzling. */
    /* The RGTC1_SNORM and LATC1_SNORM swizzle is done in the shader. */
    if (util_format_is_compressed(format) &&
        dxtc_swizzle &&
        format != PIPE_FORMAT_RGTC2_UNORM &&
        format != PIPE_FORMAT_RGTC2_SNORM &&
        format != PIPE_FORMAT_LATC2_UNORM &&
        format != PIPE_FORMAT_LATC2_SNORM &&
        format != PIPE_FORMAT_RGTC1_UNORM &&
        format != PIPE_FORMAT_RGTC1_SNORM &&
        format != PIPE_FORMAT_LATC1_UNORM &&
        format != PIPE_FORMAT_LATC1_SNORM) {
        result |= r300_get_swizzle_combined(desc->swizzle, swizzle_view,
                                            true);
    } else {
        result |= r300_get_swizzle_combined(desc->swizzle, swizzle_view,
                                            false);
    }

    /* S3TC formats. */
    if (desc->layout == UTIL_FORMAT_LAYOUT_S3TC) {
        switch (format) {
            case PIPE_FORMAT_DXT1_RGB:
            case PIPE_FORMAT_DXT1_RGBA:
            case PIPE_FORMAT_DXT1_SRGB:
            case PIPE_FORMAT_DXT1_SRGBA:
                return R300_TX_FORMAT_DXT1 | result;
            case PIPE_FORMAT_DXT3_RGBA:
            case PIPE_FORMAT_DXT3_SRGBA:
                return R300_TX_FORMAT_DXT3 | result;
            case PIPE_FORMAT_DXT5_RGBA:
            case PIPE_FORMAT_DXT5_SRGBA:
                return R300_TX_FORMAT_DXT5 | result;
            default:
                return ~0; /* Unsupported/unknown. */
        }
    }

    /* RGTC formats. */
    if (desc->layout == UTIL_FORMAT_LAYOUT_RGTC) {
        switch (format) {
            case PIPE_FORMAT_RGTC1_SNORM:
            case PIPE_FORMAT_LATC1_SNORM:
                result |= sign_bit[0];
                FALLTHROUGH;
            case PIPE_FORMAT_LATC1_UNORM:
            case PIPE_FORMAT_RGTC1_UNORM:
                return R500_TX_FORMAT_ATI1N | result;

            case PIPE_FORMAT_RGTC2_SNORM:
            case PIPE_FORMAT_LATC2_SNORM:
                result |= sign_bit[1] | sign_bit[0];
                FALLTHROUGH;
            case PIPE_FORMAT_RGTC2_UNORM:
            case PIPE_FORMAT_LATC2_UNORM:
                return R400_TX_FORMAT_ATI2N | result;

            default:
                return ~0; /* Unsupported/unknown. */
        }
    }

    /* This is truly a special format.
     * It stores R8G8 and B is computed using sqrt(1 - R^2 - G^2)
     * in the sampler unit. Also known as D3DFMT_CxV8U8. */
    if (format == PIPE_FORMAT_R8G8Bx_SNORM) {
        return R300_TX_FORMAT_CxV8U8 | result;
    }

    /* Integer and fixed-point 16.16 textures are not supported. */
    for (i = 0; i < 4; i++) {
        if (desc->channel[i].type == UTIL_FORMAT_TYPE_FIXED ||
            ((desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED ||
              desc->channel[i].type == UTIL_FORMAT_TYPE_UNSIGNED) &&
             (!desc->channel[i].normalized ||
              desc->channel[i].pure_integer))) {
            return ~0; /* Unsupported/unknown. */
        }
    }

    /* Add sign. */
    for (i = 0; i < desc->nr_channels; i++) {
        if (desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED) {
            result |= sign_bit[i];
        }
    }

    /* See whether the components are of the same size. */
    for (i = 1; i < desc->nr_channels; i++) {
        uniform = uniform && desc->channel[0].size == desc->channel[i].size;
    }

    /* Non-uniform formats. */
    if (!uniform) {
        switch (desc->nr_channels) {
            case 3:
                if (desc->channel[0].size == 5 &&
                    desc->channel[1].size == 6 &&
                    desc->channel[2].size == 5) {
                    return R300_TX_FORMAT_Z5Y6X5 | result;
                }
                if (desc->channel[0].size == 5 &&
                    desc->channel[1].size == 5 &&
                    desc->channel[2].size == 6) {
                    return R300_TX_FORMAT_Z6Y5X5 | result;
                }
                if (desc->channel[0].size == 2 &&
                    desc->channel[1].size == 3 &&
                    desc->channel[2].size == 3) {
                    return R300_TX_FORMAT_Z3Y3X2 | result;
                }
                return ~0; /* Unsupported/unknown. */

            case 4:
                if (desc->channel[0].size == 5 &&
                    desc->channel[1].size == 5 &&
                    desc->channel[2].size == 5 &&
                    desc->channel[3].size == 1) {
                    return R300_TX_FORMAT_W1Z5Y5X5 | result;
                }
                if (desc->channel[0].size == 10 &&
                    desc->channel[1].size == 10 &&
                    desc->channel[2].size == 10 &&
                    desc->channel[3].size == 2) {
                    return R300_TX_FORMAT_W2Z10Y10X10 | result;
                }
        }
        return ~0; /* Unsupported/unknown. */
    }

    i = util_format_get_first_non_void_channel(format);
    if (i == -1)
        return ~0; /* Unsupported/unknown. */

    /* And finally, uniform formats. */
    switch (desc->channel[i].type) {
        case UTIL_FORMAT_TYPE_UNSIGNED:
        case UTIL_FORMAT_TYPE_SIGNED:
            if (!desc->channel[i].normalized &&
                desc->colorspace != UTIL_FORMAT_COLORSPACE_SRGB) {
                return ~0;
            }

            switch (desc->channel[i].size) {
                case 4:
                    switch (desc->nr_channels) {
                        case 2:
                            return R300_TX_FORMAT_Y4X4 | result;
                        case 4:
                            return R300_TX_FORMAT_W4Z4Y4X4 | result;
                    }
                    return ~0;

                case 8:
                    switch (desc->nr_channels) {
                        case 1:
                            return R300_TX_FORMAT_X8 | result;
                        case 2:
                            return R300_TX_FORMAT_Y8X8 | result;
                        case 4:
                            return R300_TX_FORMAT_W8Z8Y8X8 | result;
                    }
                    return ~0;

                case 16:
                    switch (desc->nr_channels) {
                        case 1:
                            return R300_TX_FORMAT_X16 | result;
                        case 2:
                            return R300_TX_FORMAT_Y16X16 | result;
                        case 4:
                            return R300_TX_FORMAT_W16Z16Y16X16 | result;
                    }
            }
            return ~0;

        case UTIL_FORMAT_TYPE_FLOAT:
            switch (desc->channel[i].size) {
                case 16:
                    switch (desc->nr_channels) {
                        case 1:
                            return R300_TX_FORMAT_16F | result;
                        case 2:
                            return R300_TX_FORMAT_16F_16F | result;
                        case 4:
                            return R300_TX_FORMAT_16F_16F_16F_16F | result;
                    }
                    return ~0;

                case 32:
                    switch (desc->nr_channels) {
                        case 1:
                            return R300_TX_FORMAT_32F | result;
                        case 2:
                            return R300_TX_FORMAT_32F_32F | result;
                        case 4:
                            return R300_TX_FORMAT_32F_32F_32F_32F | result;
                    }
            }
    }

    return ~0; /* Unsupported/unknown. */
}

uint32_t r500_tx_format_msb_bit(enum pipe_format format)
{
    switch (format) {
        case PIPE_FORMAT_RGTC1_UNORM:
        case PIPE_FORMAT_RGTC1_SNORM:
        case PIPE_FORMAT_LATC1_UNORM:
        case PIPE_FORMAT_LATC1_SNORM:
        case PIPE_FORMAT_X8Z24_UNORM:
        case PIPE_FORMAT_S8_UINT_Z24_UNORM:
            return R500_TXFORMAT_MSB;
        default:
            return 0;
    }
}

/* Buffer formats. */

/* Colorbuffer formats. This is the unswizzled format of the RB3D block's
 * output. For the swizzling of the targets, check the shader's format. */
static uint32_t r300_translate_colorformat(enum pipe_format format)
{
    format = r300_unbyteswap_array_format(format);

    switch (format) {
        /* 8-bit buffers. */
        case PIPE_FORMAT_A8_UNORM:
        case PIPE_FORMAT_A8_SNORM:
        case PIPE_FORMAT_I8_UNORM:
        case PIPE_FORMAT_I8_SNORM:
        case PIPE_FORMAT_L8_UNORM:
        case PIPE_FORMAT_L8_SNORM:
        case PIPE_FORMAT_R8_UNORM:
        case PIPE_FORMAT_R8_SNORM:
            return R300_COLOR_FORMAT_I8;

        /* 16-bit buffers. */
        case PIPE_FORMAT_L8A8_UNORM:
        case PIPE_FORMAT_L8A8_SNORM:
        case PIPE_FORMAT_R8G8_UNORM:
        case PIPE_FORMAT_R8G8_SNORM:
        case PIPE_FORMAT_R8A8_UNORM:
        case PIPE_FORMAT_R8A8_SNORM:
        /* These formats work fine with UV88 if US_OUT_FMT is set correctly. */
        case PIPE_FORMAT_A16_UNORM:
        case PIPE_FORMAT_A16_SNORM:
        case PIPE_FORMAT_A16_FLOAT:
        case PIPE_FORMAT_L16_UNORM:
        case PIPE_FORMAT_L16_SNORM:
        case PIPE_FORMAT_L16_FLOAT:
        case PIPE_FORMAT_I16_UNORM:
        case PIPE_FORMAT_I16_SNORM:
        case PIPE_FORMAT_I16_FLOAT:
        case PIPE_FORMAT_R16_UNORM:
        case PIPE_FORMAT_R16_SNORM:
        case PIPE_FORMAT_R16_FLOAT:
            return R300_COLOR_FORMAT_UV88;

        case PIPE_FORMAT_B5G6R5_UNORM:
            return R300_COLOR_FORMAT_RGB565;

        case PIPE_FORMAT_B5G5R5A1_UNORM:
        case PIPE_FORMAT_B5G5R5X1_UNORM:
            return R300_COLOR_FORMAT_ARGB1555;

        case PIPE_FORMAT_B4G4R4A4_UNORM:
        case PIPE_FORMAT_B4G4R4X4_UNORM:
            return R300_COLOR_FORMAT_ARGB4444;

        /* 32-bit buffers. */
        case PIPE_FORMAT_B8G8R8A8_UNORM:
        /*case PIPE_FORMAT_B8G8R8A8_SNORM:*/
        case PIPE_FORMAT_B8G8R8X8_UNORM:
        /*case PIPE_FORMAT_B8G8R8X8_SNORM:*/
        case PIPE_FORMAT_R8G8B8A8_UNORM:
        case PIPE_FORMAT_R8G8B8A8_SNORM:
        case PIPE_FORMAT_R8G8B8X8_UNORM:
        case PIPE_FORMAT_R8G8B8X8_SNORM:
        /* These formats work fine with ARGB8888 if US_OUT_FMT is set
         * correctly. */
        case PIPE_FORMAT_R16G16_UNORM:
        case PIPE_FORMAT_R16G16_SNORM:
        case PIPE_FORMAT_R16G16_FLOAT:
        case PIPE_FORMAT_L16A16_UNORM:
        case PIPE_FORMAT_L16A16_SNORM:
        case PIPE_FORMAT_L16A16_FLOAT:
        case PIPE_FORMAT_R16A16_UNORM:
        case PIPE_FORMAT_R16A16_SNORM:
        case PIPE_FORMAT_R16A16_FLOAT:
        case PIPE_FORMAT_A32_FLOAT:
        case PIPE_FORMAT_L32_FLOAT:
        case PIPE_FORMAT_I32_FLOAT:
        case PIPE_FORMAT_R32_FLOAT:
            return R300_COLOR_FORMAT_ARGB8888;

        case PIPE_FORMAT_R10G10B10A2_UNORM:
        case PIPE_FORMAT_R10G10B10X2_SNORM:
        case PIPE_FORMAT_B10G10R10A2_UNORM:
        case PIPE_FORMAT_B10G10R10X2_UNORM:
            return R500_COLOR_FORMAT_ARGB2101010;  /* R5xx-only? */

        /* 64-bit buffers. */
        case PIPE_FORMAT_R16G16B16A16_UNORM:
        case PIPE_FORMAT_R16G16B16A16_SNORM:
        case PIPE_FORMAT_R16G16B16A16_FLOAT:
        case PIPE_FORMAT_R16G16B16X16_UNORM:
        case PIPE_FORMAT_R16G16B16X16_SNORM:
        case PIPE_FORMAT_R16G16B16X16_FLOAT:
        /* These formats work fine with ARGB16161616 if US_OUT_FMT is set
         * correctly. */
        case PIPE_FORMAT_R32G32_FLOAT:
        case PIPE_FORMAT_L32A32_FLOAT:
        case PIPE_FORMAT_R32A32_FLOAT:
            return R300_COLOR_FORMAT_ARGB16161616;

        /* 128-bit buffers. */
        case PIPE_FORMAT_R32G32B32A32_FLOAT:
        case PIPE_FORMAT_R32G32B32X32_FLOAT:
            return R300_COLOR_FORMAT_ARGB32323232;

        /* YUV buffers. */
        case PIPE_FORMAT_UYVY:
            return R300_COLOR_FORMAT_YVYU;
        case PIPE_FORMAT_YUYV:
            return R300_COLOR_FORMAT_VYUY;
        default:
            return ~0; /* Unsupported. */
    }
}

/* Depthbuffer and stencilbuffer. Thankfully, we only support two flavors. */
static uint32_t r300_translate_zsformat(enum pipe_format format)
{
    switch (format) {
        /* 16-bit depth, no stencil */
        case PIPE_FORMAT_Z16_UNORM:
            return R300_DEPTHFORMAT_16BIT_INT_Z;
        /* 24-bit depth, ignored stencil */
        case PIPE_FORMAT_X8Z24_UNORM:
        /* 24-bit depth, 8-bit stencil */
        case PIPE_FORMAT_S8_UINT_Z24_UNORM:
            return R300_DEPTHFORMAT_24BIT_INT_Z_8BIT_STENCIL;
        default:
            return ~0; /* Unsupported. */
    }
}

/* Shader output formats. This is essentially the swizzle from the shader
 * to the RB3D block.
 *
 * Note that formats are stored from C3 to C0. */
static uint32_t r300_translate_out_fmt(enum pipe_format format)
{
    uint32_t modifier = 0;
    int i;
    const struct util_format_description *desc;
    bool uniform_sign;

    format = r300_unbyteswap_array_format(format);
    desc = util_format_description(format);

    i = util_format_get_first_non_void_channel(format);
    if (i == -1)
        return ~0; /* Unsupported/unknown. */

    /* Specifies how the shader output is written to the fog unit. */
    switch (desc->channel[i].type) {
    case UTIL_FORMAT_TYPE_FLOAT:
        switch (desc->channel[i].size) {
        case 32:
            switch (desc->nr_channels) {
            case 1:
                modifier |= R300_US_OUT_FMT_C_32_FP;
                break;
            case 2:
                modifier |= R300_US_OUT_FMT_C2_32_FP;
                break;
            case 4:
                modifier |= R300_US_OUT_FMT_C4_32_FP;
                break;
            }
            break;

        case 16:
            switch (desc->nr_channels) {
            case 1:
                modifier |= R300_US_OUT_FMT_C_16_FP;
                break;
            case 2:
                modifier |= R300_US_OUT_FMT_C2_16_FP;
                break;
            case 4:
                modifier |= R300_US_OUT_FMT_C4_16_FP;
                break;
            }
            break;
        }
        break;

    default:
        switch (desc->channel[i].size) {
        case 16:
            switch (desc->nr_channels) {
            case 1:
                modifier |= R300_US_OUT_FMT_C_16;
                break;
            case 2:
                modifier |= R300_US_OUT_FMT_C2_16;
                break;
            case 4:
                modifier |= R300_US_OUT_FMT_C4_16;
                break;
            }
            break;

        case 10:
            modifier |= R300_US_OUT_FMT_C4_10;
            break;

        default:
            /* C4_8 seems to be used for the formats whose pixel size
             * is <= 32 bits. */
            modifier |= R300_US_OUT_FMT_C4_8;
            break;
        }
    }

    /* Add sign. */
    uniform_sign = true;
    for (i = 0; i < desc->nr_channels; i++)
        if (desc->channel[i].type != UTIL_FORMAT_TYPE_SIGNED)
            uniform_sign = false;

    if (uniform_sign)
        modifier |= R300_OUT_SIGN(0xf);

    /* Add swizzles and return. */
    switch (format) {
        /*** Special cases (non-standard channel mapping) ***/

        /* X8
         * COLORFORMAT_I8 stores the Z component (C2). */
        case PIPE_FORMAT_A8_UNORM:
        case PIPE_FORMAT_A8_SNORM:
            return modifier | R300_C2_SEL_A;
        case PIPE_FORMAT_I8_UNORM:
        case PIPE_FORMAT_I8_SNORM:
        case PIPE_FORMAT_L8_UNORM:
        case PIPE_FORMAT_L8_SNORM:
        case PIPE_FORMAT_R8_UNORM:
        case PIPE_FORMAT_R8_SNORM:
            return modifier | R300_C2_SEL_R;

        /* X8Y8
         * COLORFORMAT_UV88 stores ZX (C2 and C0). */
        case PIPE_FORMAT_L8A8_SNORM:
        case PIPE_FORMAT_L8A8_UNORM:
        case PIPE_FORMAT_R8A8_SNORM:
        case PIPE_FORMAT_R8A8_UNORM:
            return modifier | R300_C0_SEL_A | R300_C2_SEL_R;
        case PIPE_FORMAT_R8G8_SNORM:
        case PIPE_FORMAT_R8G8_UNORM:
            return modifier | R300_C0_SEL_G | R300_C2_SEL_R;

        /* X32Y32
         * ARGB16161616 stores XZ for RG32F */
        case PIPE_FORMAT_R32G32_FLOAT:
            return modifier | R300_C0_SEL_R | R300_C2_SEL_G;

        /*** Generic cases (standard channel mapping) ***/

        /* BGRA outputs. */
        case PIPE_FORMAT_B5G6R5_UNORM:
        case PIPE_FORMAT_B5G5R5A1_UNORM:
        case PIPE_FORMAT_B5G5R5X1_UNORM:
        case PIPE_FORMAT_B4G4R4A4_UNORM:
        case PIPE_FORMAT_B4G4R4X4_UNORM:
        case PIPE_FORMAT_B8G8R8A8_UNORM:
        /*case PIPE_FORMAT_B8G8R8A8_SNORM:*/
        case PIPE_FORMAT_B8G8R8X8_UNORM:
        /*case PIPE_FORMAT_B8G8R8X8_SNORM:*/
        case PIPE_FORMAT_B10G10R10A2_UNORM:
        case PIPE_FORMAT_B10G10R10X2_UNORM:
            return modifier |
                R300_C0_SEL_B | R300_C1_SEL_G |
                R300_C2_SEL_R | R300_C3_SEL_A;

        /* ARGB outputs. */
        case PIPE_FORMAT_A16_UNORM:
        case PIPE_FORMAT_A16_SNORM:
        case PIPE_FORMAT_A16_FLOAT:
        case PIPE_FORMAT_A32_FLOAT:
            return modifier |
                R300_C0_SEL_A | R300_C1_SEL_R |
                R300_C2_SEL_G | R300_C3_SEL_B;

        /* RGBA outputs. */
        case PIPE_FORMAT_R8G8B8X8_UNORM:
        case PIPE_FORMAT_R8G8B8X8_SNORM:
        case PIPE_FORMAT_R8G8B8A8_UNORM:
        case PIPE_FORMAT_R8G8B8A8_SNORM:
        case PIPE_FORMAT_R10G10B10A2_UNORM:
        case PIPE_FORMAT_R10G10B10X2_SNORM:
        case PIPE_FORMAT_R16_UNORM:
        case PIPE_FORMAT_R16G16_UNORM:
        case PIPE_FORMAT_R16G16B16A16_UNORM:
        case PIPE_FORMAT_R16_SNORM:
        case PIPE_FORMAT_R16G16_SNORM:
        case PIPE_FORMAT_R16G16B16A16_SNORM:
        case PIPE_FORMAT_R16_FLOAT:
        case PIPE_FORMAT_R16G16_FLOAT:
        case PIPE_FORMAT_R16G16B16A16_FLOAT:
        case PIPE_FORMAT_R32_FLOAT:
        case PIPE_FORMAT_R32G32B32A32_FLOAT:
        case PIPE_FORMAT_R32G32B32X32_FLOAT:
        case PIPE_FORMAT_L16_UNORM:
        case PIPE_FORMAT_L16_SNORM:
        case PIPE_FORMAT_L16_FLOAT:
        case PIPE_FORMAT_L32_FLOAT:
        case PIPE_FORMAT_I16_UNORM:
        case PIPE_FORMAT_I16_SNORM:
        case PIPE_FORMAT_I16_FLOAT:
        case PIPE_FORMAT_I32_FLOAT:
        case PIPE_FORMAT_R16G16B16X16_UNORM:
        case PIPE_FORMAT_R16G16B16X16_SNORM:
        case PIPE_FORMAT_R16G16B16X16_FLOAT:
            return modifier |
                R300_C0_SEL_R | R300_C1_SEL_G |
                R300_C2_SEL_B | R300_C3_SEL_A;

        /* LA outputs. */
        case PIPE_FORMAT_L16A16_UNORM:
        case PIPE_FORMAT_L16A16_SNORM:
        case PIPE_FORMAT_L16A16_FLOAT:
        case PIPE_FORMAT_R16A16_UNORM:
        case PIPE_FORMAT_R16A16_SNORM:
        case PIPE_FORMAT_R16A16_FLOAT:
        case PIPE_FORMAT_L32A32_FLOAT:
        case PIPE_FORMAT_R32A32_FLOAT:
            return modifier |
                R300_C0_SEL_R | R300_C1_SEL_A;

        default:
            return ~0; /* Unsupported. */
    }
}

static uint32_t r300_translate_colormask_swizzle(enum pipe_format format)
{
    format = r300_unbyteswap_array_format(format);

    switch (format) {
    case PIPE_FORMAT_A8_UNORM:
    case PIPE_FORMAT_A8_SNORM:
    case PIPE_FORMAT_A16_UNORM:
    case PIPE_FORMAT_A16_SNORM:
    case PIPE_FORMAT_A16_FLOAT:
    case PIPE_FORMAT_A32_FLOAT:
        return COLORMASK_AAAA;

    case PIPE_FORMAT_I8_UNORM:
    case PIPE_FORMAT_I8_SNORM:
    case PIPE_FORMAT_L8_UNORM:
    case PIPE_FORMAT_L8_SNORM:
    case PIPE_FORMAT_R8_UNORM:
    case PIPE_FORMAT_R8_SNORM:
    case PIPE_FORMAT_R32_FLOAT:
    case PIPE_FORMAT_L32_FLOAT:
    case PIPE_FORMAT_I32_FLOAT:
        return COLORMASK_RRRR;

    case PIPE_FORMAT_L8A8_SNORM:
    case PIPE_FORMAT_L8A8_UNORM:
    case PIPE_FORMAT_R8A8_UNORM:
    case PIPE_FORMAT_R8A8_SNORM:
    case PIPE_FORMAT_L16A16_UNORM:
    case PIPE_FORMAT_L16A16_SNORM:
    case PIPE_FORMAT_L16A16_FLOAT:
    case PIPE_FORMAT_R16A16_UNORM:
    case PIPE_FORMAT_R16A16_SNORM:
    case PIPE_FORMAT_R16A16_FLOAT:
    case PIPE_FORMAT_L32A32_FLOAT:
    case PIPE_FORMAT_R32A32_FLOAT:
        return COLORMASK_ARRA;

    case PIPE_FORMAT_R8G8_SNORM:
    case PIPE_FORMAT_R8G8_UNORM:
    case PIPE_FORMAT_R16G16_UNORM:
    case PIPE_FORMAT_R16G16_SNORM:
    case PIPE_FORMAT_R16G16_FLOAT:
    case PIPE_FORMAT_R32G32_FLOAT:
        return COLORMASK_GRRG;

    case PIPE_FORMAT_B5G5R5X1_UNORM:
    case PIPE_FORMAT_B4G4R4X4_UNORM:
    case PIPE_FORMAT_B8G8R8X8_UNORM:
    /*case PIPE_FORMAT_B8G8R8X8_SNORM:*/
    case PIPE_FORMAT_B10G10R10X2_UNORM:
        return COLORMASK_BGRX;

    case PIPE_FORMAT_B5G6R5_UNORM:
    case PIPE_FORMAT_B5G5R5A1_UNORM:
    case PIPE_FORMAT_B4G4R4A4_UNORM:
    case PIPE_FORMAT_B8G8R8A8_UNORM:
    /*case PIPE_FORMAT_B8G8R8A8_SNORM:*/
    case PIPE_FORMAT_B10G10R10A2_UNORM:
        return COLORMASK_BGRA;

    case PIPE_FORMAT_R8G8B8X8_UNORM:
    /* RGBX_SNORM formats are broken for an unknown reason */
    /*case PIPE_FORMAT_R8G8B8X8_SNORM:*/
    /*case PIPE_FORMAT_R10G10B10X2_SNORM:*/
    case PIPE_FORMAT_R16G16B16X16_UNORM:
    /*case PIPE_FORMAT_R16G16B16X16_SNORM:*/
    case PIPE_FORMAT_R16G16B16X16_FLOAT:
    case PIPE_FORMAT_R32G32B32X32_FLOAT:
        return COLORMASK_RGBX;

    case PIPE_FORMAT_R8G8B8A8_UNORM:
    case PIPE_FORMAT_R8G8B8A8_SNORM:
    case PIPE_FORMAT_R10G10B10A2_UNORM:
    case PIPE_FORMAT_R16_UNORM:
    case PIPE_FORMAT_R16G16B16A16_UNORM:
    case PIPE_FORMAT_R16_SNORM:
    case PIPE_FORMAT_R16G16B16A16_SNORM:
    case PIPE_FORMAT_R16_FLOAT:
    case PIPE_FORMAT_R16G16B16A16_FLOAT:
    case PIPE_FORMAT_R32G32B32A32_FLOAT:
    case PIPE_FORMAT_L16_UNORM:
    case PIPE_FORMAT_L16_SNORM:
    case PIPE_FORMAT_L16_FLOAT:
    case PIPE_FORMAT_I16_UNORM:
    case PIPE_FORMAT_I16_SNORM:
    case PIPE_FORMAT_I16_FLOAT:
        return COLORMASK_RGBA;

    default:
        return ~0; /* Unsupported. */
    }
}

bool r300_is_colorbuffer_format_supported(enum pipe_format format)
{
    return r300_translate_colorformat(format) != ~0 &&
           r300_translate_out_fmt(format) != ~0 &&
           r300_translate_colormask_swizzle(format) != ~0;
}

bool r300_is_zs_format_supported(enum pipe_format format)
{
    return r300_translate_zsformat(format) != ~0;
}

bool r300_is_sampler_format_supported(enum pipe_format format)
{
    return r300_translate_texformat(format, NULL, true, false) != ~0;
}

void r300_texture_setup_format_state(struct r300_screen *screen,
                                     struct r300_resource *tex,
                                     enum pipe_format format,
                                     unsigned level,
                                     unsigned width0_override,
                                     unsigned height0_override,
                                     struct r300_texture_format_state *out)
{
    struct pipe_resource *pt = &tex->b;
    struct r300_texture_desc *desc = &tex->tex;
    bool is_r500 = screen->caps.is_r500;
    unsigned width, height, depth;
    unsigned txwidth, txheight, txdepth;

    width = u_minify(width0_override, level);
    height = u_minify(height0_override, level);
    depth = u_minify(desc->depth0, level);

    txwidth = (width - 1) & 0x7ff;
    txheight = (height - 1) & 0x7ff;
    txdepth = util_logbase2(depth) & 0xf;

    /* Mask out all the fields we change. */
    out->format0 = 0;
    out->format1 &= ~R300_TX_FORMAT_TEX_COORD_TYPE_MASK;
    out->format2 &= R500_TXFORMAT_MSB;
    out->tile_config = 0;

    /* Set sampler state. */
    out->format0 =
        R300_TX_WIDTH(txwidth) |
        R300_TX_HEIGHT(txheight) |
        R300_TX_DEPTH(txdepth);

    if (desc->uses_stride_addressing) {
        unsigned stride =
            r300_stride_to_width(format, desc->stride_in_bytes[level]);
        /* rectangles love this */
        out->format0 |= R300_TX_PITCH_EN;
        out->format2 = (stride - 1) & 0x1fff;
    }

    if (pt->target == PIPE_TEXTURE_CUBE) {
        out->format1 |= R300_TX_FORMAT_CUBIC_MAP;
    }
    if (pt->target == PIPE_TEXTURE_3D) {
        out->format1 |= R300_TX_FORMAT_3D;
    }

    /* large textures on r500 */
    if (is_r500)
    {
        unsigned us_width = txwidth;
        unsigned us_height = txheight;
        unsigned us_depth = txdepth;

        if (width > 2048) {
            out->format2 |= R500_TXWIDTH_BIT11;
        }
        if (height > 2048) {
            out->format2 |= R500_TXHEIGHT_BIT11;
        }

        /* The US_FORMAT register fixes an R500 TX addressing bug.
         * Don't ask why it must be set like this. I don't know it either. */
        if (width > 2048) {
            us_width = (0x000007FF + us_width) >> 1;
            us_depth |= 0x0000000D;
        }
        if (height > 2048) {
            us_height = (0x000007FF + us_height) >> 1;
            us_depth |= 0x0000000E;
        }

        out->us_format0 =
            R300_TX_WIDTH(us_width) |
            R300_TX_HEIGHT(us_height) |
            R300_TX_DEPTH(us_depth);
    }

    out->tile_config = R300_TXO_MACRO_TILE(desc->macrotile[level]) |
                       R300_TXO_MICRO_TILE(desc->microtile) |
                       R300_TXO_ENDIAN(r300_get_endian_swap(format));
}

static void r300_texture_setup_fb_state(struct r300_surface *surf)
{
    struct r300_resource *tex = r300_resource(surf->base.texture);
    unsigned level = surf->base.u.tex.level;
    unsigned stride =
      r300_stride_to_width(surf->base.format, tex->tex.stride_in_bytes[level]);

    /* Set framebuffer state. */
    if (util_format_is_depth_or_stencil(surf->base.format)) {
        surf->pitch =
                stride |
                R300_DEPTHMACROTILE(tex->tex.macrotile[level]) |
                R300_DEPTHMICROTILE(tex->tex.microtile) |
                R300_DEPTHENDIAN(r300_get_endian_swap(surf->base.format));
        surf->format = r300_translate_zsformat(surf->base.format);
        surf->pitch_zmask = tex->tex.zmask_stride_in_pixels[level];
        surf->pitch_hiz = tex->tex.hiz_stride_in_pixels[level];
    } else {
        enum pipe_format format = util_format_linear(surf->base.format);

        surf->pitch =
                stride |
                r300_translate_colorformat(format) |
                R300_COLOR_TILE(tex->tex.macrotile[level]) |
                R300_COLOR_MICROTILE(tex->tex.microtile) |
                R300_COLOR_ENDIAN(r300_get_endian_swap(format));
        surf->format = r300_translate_out_fmt(format);
        surf->colormask_swizzle =
            r300_translate_colormask_swizzle(format);
        surf->pitch_cmask = tex->tex.cmask_stride_in_pixels;
    }
}

bool r300_resource_get_handle(struct pipe_screen* screen,
                              struct pipe_context *ctx,
                              struct pipe_resource *texture,
                              struct winsys_handle *whandle,
                              unsigned usage)
{
    struct radeon_winsys *rws = r300_screen(screen)->rws;
    struct r300_resource* tex = (struct r300_resource*)texture;

    if (!tex) {
        return false;
    }

    whandle->stride = tex->tex.stride_in_bytes[0];
    whandle->offset = 0;

    return rws->buffer_get_handle(rws, tex->buf, whandle);
}

/* The common texture constructor. */
static struct r300_resource*
r300_texture_create_object(struct r300_screen *rscreen,
                           const struct pipe_resource *base,
                           enum radeon_bo_layout microtile,
                           enum radeon_bo_layout macrotile,
                           unsigned stride_in_bytes_override,
                           struct pb_buffer_lean *buffer)
{
    struct radeon_winsys *rws = rscreen->rws;
    struct r300_resource *tex = NULL;
    struct radeon_bo_metadata tiling = {};

    tex = CALLOC_STRUCT(r300_resource);
    if (!tex) {
        goto fail;
    }

    pipe_reference_init(&tex->b.reference, 1);
    tex->b.screen = &rscreen->screen;
    tex->b.usage = base->usage;
    tex->b.bind = base->bind;
    tex->b.flags = base->flags;
    tex->tex.microtile = microtile;
    tex->tex.macrotile[0] = macrotile;
    tex->tex.stride_in_bytes_override = stride_in_bytes_override;
    tex->domain = (base->flags & R300_RESOURCE_FLAG_TRANSFER ||
                   base->usage == PIPE_USAGE_STAGING) ? RADEON_DOMAIN_GTT :
                  base->nr_samples > 1 ? RADEON_DOMAIN_VRAM :
                                         RADEON_DOMAIN_VRAM | RADEON_DOMAIN_GTT;
    tex->buf = buffer;

    r300_texture_desc_init(rscreen, tex, base);

    /* Figure out the ideal placement for the texture.. */
    if (tex->domain & RADEON_DOMAIN_VRAM &&
        tex->tex.size_in_bytes >= (uint64_t)rscreen->info.vram_size_kb * 1024) {
        tex->domain &= ~RADEON_DOMAIN_VRAM;
        tex->domain |= RADEON_DOMAIN_GTT;
    }
    if (tex->domain & RADEON_DOMAIN_GTT &&
        tex->tex.size_in_bytes >= (uint64_t)rscreen->info.gart_size_kb * 1024) {
        tex->domain &= ~RADEON_DOMAIN_GTT;
    }
    /* Just fail if the texture is too large. */
    if (!tex->domain) {
        goto fail;
    }

    /* Create the backing buffer if needed. */
    if (!tex->buf) {
        /* Only use the first domain for allocation. Multiple domains are not allowed. */
        unsigned alloc_domain =
            tex->domain & RADEON_DOMAIN_VRAM ? RADEON_DOMAIN_VRAM :
                                               RADEON_DOMAIN_GTT;

        tex->buf = rws->buffer_create(rws, tex->tex.size_in_bytes, 2048,
                                      alloc_domain,
                                      RADEON_FLAG_NO_SUBALLOC |
                                      /* Use the reusable pool: */
                                      RADEON_FLAG_NO_INTERPROCESS_SHARING);

        if (!tex->buf) {
            goto fail;
        }
    }

    if (SCREEN_DBG_ON(rscreen, DBG_MSAA) && base->nr_samples > 1) {
        fprintf(stderr, "r300: %ix MSAA %s buffer created\n",
                base->nr_samples,
                util_format_is_depth_or_stencil(base->format) ? "depth" : "color");
    }

    tiling.u.legacy.microtile = tex->tex.microtile;
    tiling.u.legacy.macrotile = tex->tex.macrotile[0];
    tiling.u.legacy.stride = tex->tex.stride_in_bytes[0];
    rws->buffer_set_metadata(rws, tex->buf, &tiling, NULL);

    return tex;

fail:
    FREE(tex);
    if (buffer)
        radeon_bo_reference(rscreen->rws, &buffer, NULL);
    return NULL;
}

/* Create a new texture. */
struct pipe_resource *r300_texture_create(struct pipe_screen *screen,
                                          const struct pipe_resource *base)
{
    struct r300_screen *rscreen = r300_screen(screen);
    enum radeon_bo_layout microtile, macrotile;

    if ((base->flags & R300_RESOURCE_FLAG_TRANSFER) ||
        (base->bind & (PIPE_BIND_SCANOUT | PIPE_BIND_LINEAR))) {
        microtile = RADEON_LAYOUT_LINEAR;
        macrotile = RADEON_LAYOUT_LINEAR;
    } else {
        /* This will make the texture_create_function select the layout. */
        microtile = RADEON_LAYOUT_UNKNOWN;
        macrotile = RADEON_LAYOUT_UNKNOWN;
    }

    return (struct pipe_resource*)
           r300_texture_create_object(rscreen, base, microtile, macrotile,
                                      0, NULL);
}

struct pipe_resource *r300_texture_from_handle(struct pipe_screen *screen,
                                               const struct pipe_resource *base,
                                               struct winsys_handle *whandle,
                                               unsigned usage)
{
    struct r300_screen *rscreen = r300_screen(screen);
    struct radeon_winsys *rws = rscreen->rws;
    struct pb_buffer_lean *buffer;
    struct radeon_bo_metadata tiling = {};

    /* Support only 2D textures without mipmaps */
    if ((base->target != PIPE_TEXTURE_2D &&
          base->target != PIPE_TEXTURE_RECT) ||
        base->depth0 != 1 ||
        base->last_level != 0) {
        return NULL;
    }

    buffer = rws->buffer_from_handle(rws, whandle, 0, false);
    if (!buffer)
        return NULL;

    rws->buffer_get_metadata(rws, buffer, &tiling, NULL);

    /* Enforce a microtiled zbuffer. */
    if (util_format_is_depth_or_stencil(base->format) &&
        tiling.u.legacy.microtile == RADEON_LAYOUT_LINEAR) {
        switch (util_format_get_blocksize(base->format)) {
            case 4:
                tiling.u.legacy.microtile = RADEON_LAYOUT_TILED;
                break;

            case 2:
                tiling.u.legacy.microtile = RADEON_LAYOUT_SQUARETILED;
                break;
        }
    }

    return (struct pipe_resource*)
           r300_texture_create_object(rscreen, base, tiling.u.legacy.microtile, tiling.u.legacy.macrotile,
                                      whandle->stride, buffer);
}

struct pipe_surface* r300_create_surface_custom(struct pipe_context * ctx,
                                         struct pipe_resource* texture,
                                         const struct pipe_surface *surf_tmpl,
                                         unsigned width0_override,
					 unsigned height0_override)
{
    struct r300_resource* tex = r300_resource(texture);
    struct r300_surface* surface = CALLOC_STRUCT(r300_surface);
    unsigned level = surf_tmpl->u.tex.level;

    assert(surf_tmpl->u.tex.first_layer == surf_tmpl->u.tex.last_layer);

    if (surface) {
        uint32_t offset, tile_height;

        pipe_reference_init(&surface->base.reference, 1);
        pipe_resource_reference(&surface->base.texture, texture);
        surface->base.context = ctx;
        surface->base.format = surf_tmpl->format;
        surface->base.width = u_minify(width0_override, level);
        surface->base.height = u_minify(height0_override, level);
        surface->base.u.tex.level = level;
        surface->base.u.tex.first_layer = surf_tmpl->u.tex.first_layer;
        surface->base.u.tex.last_layer = surf_tmpl->u.tex.last_layer;

        surface->buf = tex->buf;

        /* Prefer VRAM if there are multiple domains to choose from. */
        surface->domain = tex->domain;
        if (surface->domain & RADEON_DOMAIN_VRAM)
            surface->domain &= ~RADEON_DOMAIN_GTT;

        surface->offset = r300_texture_get_offset(tex, level,
                                                  surf_tmpl->u.tex.first_layer);
        r300_texture_setup_fb_state(surface);

        /* Parameters for the CBZB clear. */
        surface->cbzb_allowed = tex->tex.cbzb_allowed[level];
        surface->cbzb_width = align(surface->base.width, 64);

        /* Height must be aligned to the size of a tile. */
        tile_height = r300_get_pixel_alignment(surface->base.format,
                                               tex->b.nr_samples,
                                               tex->tex.microtile,
                                               tex->tex.macrotile[level],
                                               DIM_HEIGHT, 0);

        surface->cbzb_height = align((surface->base.height + 1) / 2,
                                     tile_height);

        /* Offset must be aligned to 2K and must point at the beginning
         * of a scanline. */
        offset = surface->offset +
                 tex->tex.stride_in_bytes[level] * surface->cbzb_height;
        surface->cbzb_midpoint_offset = offset & ~2047;

        surface->cbzb_pitch = surface->pitch & 0x1ffffc;

        if (util_format_get_blocksizebits(surface->base.format) == 32)
            surface->cbzb_format = R300_DEPTHFORMAT_24BIT_INT_Z_8BIT_STENCIL;
        else
            surface->cbzb_format = R300_DEPTHFORMAT_16BIT_INT_Z;

        DBG(r300_context(ctx), DBG_CBZB,
            "CBZB Allowed: %s, Dim: %ix%i, Misalignment: %i, Micro: %s, Macro: %s\n",
            surface->cbzb_allowed ? "YES" : " NO",
            surface->cbzb_width, surface->cbzb_height,
            offset & 2047,
            tex->tex.microtile ? "YES" : " NO",
            tex->tex.macrotile[level] ? "YES" : " NO");
    }

    return &surface->base;
}

struct pipe_surface* r300_create_surface(struct pipe_context * ctx,
                                         struct pipe_resource* texture,
                                         const struct pipe_surface *surf_tmpl)
{
    return r300_create_surface_custom(ctx, texture, surf_tmpl,
                                      texture->width0,
                                      texture->height0);
}

void r300_surface_destroy(struct pipe_context *ctx, struct pipe_surface* s)
{
    pipe_resource_reference(&s->texture, NULL);
    FREE(s);
}
