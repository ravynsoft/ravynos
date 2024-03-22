/*
 * Copyright 2009 Joakim Sindholt <opensource@zhasha.com>
 *                Corbin Simpson <MostAwesomeDude@gmail.com>
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

#ifndef R300_STATE_INLINES_H
#define R300_STATE_INLINES_H

#include "draw/draw_vertex.h"
#include "util/format/u_formats.h"
#include "util/format/u_format.h"
#include "r300_reg.h"
#include <stdio.h>

/* Some maths. These should probably find their way to u_math, if needed. */

static inline int pack_float_16_6x(float f) {
    return ((int)(f * 6.0) & 0xffff);
}

/* Blend state. */

static inline uint32_t r300_translate_blend_function(int blend_func,
                                                     bool clamp)
{
    switch (blend_func) {
    case PIPE_BLEND_ADD:
        return clamp ? R300_COMB_FCN_ADD_CLAMP : R300_COMB_FCN_ADD_NOCLAMP;
    case PIPE_BLEND_SUBTRACT:
        return clamp ? R300_COMB_FCN_SUB_CLAMP : R300_COMB_FCN_SUB_NOCLAMP;
    case PIPE_BLEND_REVERSE_SUBTRACT:
        return clamp ? R300_COMB_FCN_RSUB_CLAMP : R300_COMB_FCN_RSUB_NOCLAMP;
    case PIPE_BLEND_MIN:
        return R300_COMB_FCN_MIN;
    case PIPE_BLEND_MAX:
        return R300_COMB_FCN_MAX;
    default:
        fprintf(stderr, "r300: Unknown blend function %d\n", blend_func);
        assert(0);
        break;
    }
    return 0;
}

static inline uint32_t r300_translate_blend_factor(int blend_fact)
{
    switch (blend_fact) {
        case PIPE_BLENDFACTOR_ONE:
            return R300_BLEND_GL_ONE;
        case PIPE_BLENDFACTOR_SRC_COLOR:
            return R300_BLEND_GL_SRC_COLOR;
        case PIPE_BLENDFACTOR_SRC_ALPHA:
            return R300_BLEND_GL_SRC_ALPHA;
        case PIPE_BLENDFACTOR_DST_ALPHA:
            return R300_BLEND_GL_DST_ALPHA;
        case PIPE_BLENDFACTOR_DST_COLOR:
            return R300_BLEND_GL_DST_COLOR;
        case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
            return R300_BLEND_GL_SRC_ALPHA_SATURATE;
        case PIPE_BLENDFACTOR_CONST_COLOR:
            return R300_BLEND_GL_CONST_COLOR;
        case PIPE_BLENDFACTOR_CONST_ALPHA:
            return R300_BLEND_GL_CONST_ALPHA;
        case PIPE_BLENDFACTOR_ZERO:
            return R300_BLEND_GL_ZERO;
        case PIPE_BLENDFACTOR_INV_SRC_COLOR:
            return R300_BLEND_GL_ONE_MINUS_SRC_COLOR;
        case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
            return R300_BLEND_GL_ONE_MINUS_SRC_ALPHA;
        case PIPE_BLENDFACTOR_INV_DST_ALPHA:
            return R300_BLEND_GL_ONE_MINUS_DST_ALPHA;
        case PIPE_BLENDFACTOR_INV_DST_COLOR:
            return R300_BLEND_GL_ONE_MINUS_DST_COLOR;
        case PIPE_BLENDFACTOR_INV_CONST_COLOR:
            return R300_BLEND_GL_ONE_MINUS_CONST_COLOR;
        case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
            return R300_BLEND_GL_ONE_MINUS_CONST_ALPHA;

        case PIPE_BLENDFACTOR_SRC1_COLOR:
        case PIPE_BLENDFACTOR_SRC1_ALPHA:
        case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
        case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
            fprintf(stderr, "r300: Implementation error: "
                "Bad blend factor %d not supported!\n", blend_fact);
            assert(0);
            break;

        default:
            fprintf(stderr, "r300: Unknown blend factor %d\n", blend_fact);
            assert(0);
            break;
    }
    return 0;
}

/* DSA state. */

static inline uint32_t r300_translate_depth_stencil_function(int zs_func)
{
    switch (zs_func) {
        case PIPE_FUNC_NEVER:
            return R300_ZS_NEVER;
        case PIPE_FUNC_LESS:
            return R300_ZS_LESS;
        case PIPE_FUNC_EQUAL:
            return R300_ZS_EQUAL;
        case PIPE_FUNC_LEQUAL:
            return R300_ZS_LEQUAL;
        case PIPE_FUNC_GREATER:
            return R300_ZS_GREATER;
        case PIPE_FUNC_NOTEQUAL:
            return R300_ZS_NOTEQUAL;
        case PIPE_FUNC_GEQUAL:
            return R300_ZS_GEQUAL;
        case PIPE_FUNC_ALWAYS:
            return R300_ZS_ALWAYS;
        default:
            fprintf(stderr, "r300: Unknown depth/stencil function %d\n",
                zs_func);
            assert(0);
            break;
    }
    return 0;
}

static inline uint32_t r300_translate_stencil_op(int s_op)
{
    switch (s_op) {
        case PIPE_STENCIL_OP_KEEP:
            return R300_ZS_KEEP;
        case PIPE_STENCIL_OP_ZERO:
            return R300_ZS_ZERO;
        case PIPE_STENCIL_OP_REPLACE:
            return R300_ZS_REPLACE;
        case PIPE_STENCIL_OP_INCR:
            return R300_ZS_INCR;
        case PIPE_STENCIL_OP_DECR:
            return R300_ZS_DECR;
        case PIPE_STENCIL_OP_INCR_WRAP:
            return R300_ZS_INCR_WRAP;
        case PIPE_STENCIL_OP_DECR_WRAP:
            return R300_ZS_DECR_WRAP;
        case PIPE_STENCIL_OP_INVERT:
            return R300_ZS_INVERT;
        default:
            fprintf(stderr, "r300: Unknown stencil op %d", s_op);
            assert(0);
            break;
    }
    return 0;
}

static inline uint32_t r300_translate_alpha_function(int alpha_func)
{
    switch (alpha_func) {
        case PIPE_FUNC_NEVER:
            return R300_FG_ALPHA_FUNC_NEVER;
        case PIPE_FUNC_LESS:
            return R300_FG_ALPHA_FUNC_LESS;
        case PIPE_FUNC_EQUAL:
            return R300_FG_ALPHA_FUNC_EQUAL;
        case PIPE_FUNC_LEQUAL:
            return R300_FG_ALPHA_FUNC_LE;
        case PIPE_FUNC_GREATER:
            return R300_FG_ALPHA_FUNC_GREATER;
        case PIPE_FUNC_NOTEQUAL:
            return R300_FG_ALPHA_FUNC_NOTEQUAL;
        case PIPE_FUNC_GEQUAL:
            return R300_FG_ALPHA_FUNC_GE;
        case PIPE_FUNC_ALWAYS:
            return R300_FG_ALPHA_FUNC_ALWAYS;
        default:
            fprintf(stderr, "r300: Unknown alpha function %d", alpha_func);
            assert(0);
            break;
    }
    return 0;
}

static inline uint32_t
r300_translate_polygon_mode_front(unsigned mode) {
    switch (mode)
    {
        case PIPE_POLYGON_MODE_FILL:
            return R300_GA_POLY_MODE_FRONT_PTYPE_TRI;
        case PIPE_POLYGON_MODE_LINE:
            return R300_GA_POLY_MODE_FRONT_PTYPE_LINE;
        case PIPE_POLYGON_MODE_POINT:
            return R300_GA_POLY_MODE_FRONT_PTYPE_POINT;

        default:
            fprintf(stderr, "r300: Bad polygon mode %i in %s\n", mode,
                __func__);
            return R300_GA_POLY_MODE_FRONT_PTYPE_TRI;
    }
}

static inline uint32_t
r300_translate_polygon_mode_back(unsigned mode) {
    switch (mode)
    {
        case PIPE_POLYGON_MODE_FILL:
            return R300_GA_POLY_MODE_BACK_PTYPE_TRI;
        case PIPE_POLYGON_MODE_LINE:
            return R300_GA_POLY_MODE_BACK_PTYPE_LINE;
        case PIPE_POLYGON_MODE_POINT:
            return R300_GA_POLY_MODE_BACK_PTYPE_POINT;

        default:
            fprintf(stderr, "r300: Bad polygon mode %i in %s\n", mode,
                __func__);
            return R300_GA_POLY_MODE_BACK_PTYPE_TRI;
    }
}

/* Texture sampler state. */

static inline uint32_t r300_translate_wrap(int wrap)
{
    switch (wrap) {
        case PIPE_TEX_WRAP_REPEAT:
            return R300_TX_REPEAT;
        case PIPE_TEX_WRAP_CLAMP:
            return R300_TX_CLAMP;
        case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
            return R300_TX_CLAMP_TO_EDGE;
        case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
            return R300_TX_CLAMP_TO_BORDER;
        case PIPE_TEX_WRAP_MIRROR_REPEAT:
            return R300_TX_REPEAT | R300_TX_MIRRORED;
        case PIPE_TEX_WRAP_MIRROR_CLAMP:
            return R300_TX_CLAMP | R300_TX_MIRRORED;
        case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
            return R300_TX_CLAMP_TO_EDGE | R300_TX_MIRRORED;
        case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
            return R300_TX_CLAMP_TO_BORDER | R300_TX_MIRRORED;
        default:
            fprintf(stderr, "r300: Unknown texture wrap %d", wrap);
            assert(0);
            return 0;
    }
}

static inline uint32_t r300_translate_tex_filters(int min, int mag, int mip,
                                                  bool is_anisotropic)
{
    uint32_t retval = 0;

    switch (min) {
    case PIPE_TEX_FILTER_NEAREST:
        retval |= R300_TX_MIN_FILTER_NEAREST;
        break;
    case PIPE_TEX_FILTER_LINEAR:
        retval |= is_anisotropic ? R300_TX_MIN_FILTER_ANISO :
                                   R300_TX_MIN_FILTER_LINEAR;
        break;
    default:
        fprintf(stderr, "r300: Unknown texture filter %d\n", min);
        assert(0);
    }

    switch (mag) {
    case PIPE_TEX_FILTER_NEAREST:
        retval |= R300_TX_MAG_FILTER_NEAREST;
        break;
    case PIPE_TEX_FILTER_LINEAR:
        retval |= is_anisotropic ? R300_TX_MAG_FILTER_ANISO :
                                   R300_TX_MAG_FILTER_LINEAR;
        break;
    default:
        fprintf(stderr, "r300: Unknown texture filter %d\n", mag);
        assert(0);
    }

    switch (mip) {
    case PIPE_TEX_MIPFILTER_NONE:
        retval |= R300_TX_MIN_FILTER_MIP_NONE;
        break;
    case PIPE_TEX_MIPFILTER_NEAREST:
        retval |= R300_TX_MIN_FILTER_MIP_NEAREST;
        break;
    case PIPE_TEX_MIPFILTER_LINEAR:
        retval |= R300_TX_MIN_FILTER_MIP_LINEAR;
        break;
    default:
        fprintf(stderr, "r300: Unknown texture filter %d\n", mip);
        assert(0);
    }

    return retval;
}

static inline uint32_t r300_anisotropy(unsigned max_aniso)
{
    if (max_aniso >= 16) {
        return R300_TX_MAX_ANISO_16_TO_1;
    } else if (max_aniso >= 8) {
        return R300_TX_MAX_ANISO_8_TO_1;
    } else if (max_aniso >= 4) {
        return R300_TX_MAX_ANISO_4_TO_1;
    } else if (max_aniso >= 2) {
        return R300_TX_MAX_ANISO_2_TO_1;
    } else {
        return R300_TX_MAX_ANISO_1_TO_1;
    }
}

static inline uint32_t r500_anisotropy(unsigned max_aniso)
{
    if (!max_aniso) {
        return 0;
    }
    max_aniso -= 1;

    // Map the range [0, 15] to [0, 63].
    return R500_TX_MAX_ANISO(MIN2((unsigned)(max_aniso*4.2001), 63)) |
           R500_TX_ANISO_HIGH_QUALITY;
}

/* Translate pipe_formats into PSC vertex types. */
static inline uint16_t
r300_translate_vertex_data_type(enum pipe_format format) {
    uint32_t result = 0;
    const struct util_format_description *desc;
    int i = util_format_get_first_non_void_channel(format);

    if (!format)
        format = PIPE_FORMAT_R32_FLOAT;

    desc = util_format_description(format);

    if (desc->layout != UTIL_FORMAT_LAYOUT_PLAIN) {
        return R300_INVALID_FORMAT;
    }

    switch (desc->channel[i].type) {
        /* Half-floats, floats, doubles */
        case UTIL_FORMAT_TYPE_FLOAT:
            switch (desc->channel[i].size) {
                case 16:
                    /* Supported only on RV350 and later. */
                    if (desc->nr_channels > 2) {
                        result = R300_DATA_TYPE_FLT16_4;
                    } else {
                        result = R300_DATA_TYPE_FLT16_2;
                    }
                    break;
                case 32:
                    result = R300_DATA_TYPE_FLOAT_1 + (desc->nr_channels - 1);
                    break;
                default:
                    return R300_INVALID_FORMAT;
            }
            break;
        /* Unsigned ints */
        case UTIL_FORMAT_TYPE_UNSIGNED:
        /* Signed ints */
        case UTIL_FORMAT_TYPE_SIGNED:
            switch (desc->channel[i].size) {
                case 8:
                    result = R300_DATA_TYPE_BYTE;
                    break;
                case 16:
                    if (desc->nr_channels > 2) {
                        result = R300_DATA_TYPE_SHORT_4;
                    } else {
                        result = R300_DATA_TYPE_SHORT_2;
                    }
                    break;
                default:
                    return R300_INVALID_FORMAT;
            }
            break;
        default:
            return R300_INVALID_FORMAT;
    }

    if (desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED) {
        result |= R300_SIGNED;
    }
    if (desc->channel[i].normalized) {
        result |= R300_NORMALIZE;
    }

    return result;
}

static inline uint16_t
r300_translate_vertex_data_swizzle(enum pipe_format format) {
    const struct util_format_description *desc;
    unsigned i, swizzle = 0;

    if (!format)
        return (R300_SWIZZLE_SELECT_FP_ZERO << R300_SWIZZLE_SELECT_X_SHIFT) |
               (R300_SWIZZLE_SELECT_FP_ZERO << R300_SWIZZLE_SELECT_Y_SHIFT) |
               (R300_SWIZZLE_SELECT_FP_ZERO << R300_SWIZZLE_SELECT_Z_SHIFT) |
               (R300_SWIZZLE_SELECT_FP_ONE << R300_SWIZZLE_SELECT_W_SHIFT);

    desc = util_format_description(format);

    if (desc->layout != UTIL_FORMAT_LAYOUT_PLAIN) {
        fprintf(stderr, "r300: Bad format %s in %s:%d\n",
            util_format_short_name(format), __func__, __LINE__);
        return 0;
    }

    for (i = 0; i < desc->nr_channels; i++) {
        swizzle |=
            MIN2(desc->swizzle[i], R300_SWIZZLE_SELECT_FP_ONE) << (3*i);
    }
    /* Set (0,0,0,1) in unused components. */
    for (; i < 3; i++) {
        swizzle |= R300_SWIZZLE_SELECT_FP_ZERO << (3*i);
    }
    for (; i < 4; i++) {
        swizzle |= R300_SWIZZLE_SELECT_FP_ONE << (3*i);
    }

    return swizzle | (0xf << R300_WRITE_ENA_SHIFT);
}

#endif /* R300_STATE_INLINES_H */
