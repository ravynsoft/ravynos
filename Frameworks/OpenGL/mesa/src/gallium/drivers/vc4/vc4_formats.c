/*
 * Copyright © 2014 Broadcom
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

/**
 * @file vc4_formats.c
 *
 * Contains the table and accessors for VC4 texture and render target format
 * support.
 *
 * The hardware has limited support for texture formats, and extremely limited
 * support for render target formats.  As a result, we emulate other formats
 * in our shader code, and this stores the table for doing so.
 */

#include "util/format/u_format.h"
#include "util/macros.h"

#include "vc4_context.h"

#define RT_NO        0
#define RT_RGBA8888  1
#define RT_RGB565    2

struct vc4_format {
        /** Set if the pipe format is defined in the table. */
        bool present;

        /** Set to 0 if unsupported, 1 if RGBA8888, 2 if rgb565. */
        uint8_t rt_type;

        /** One of VC4_TEXTURE_TYPE_*. */
        uint8_t tex_type;

        /**
         * Swizzle to apply to the RGBA shader output for storing to the tile
         * buffer, to the RGBA tile buffer to produce shader input (for
         * blending), and for turning the rgba8888 texture sampler return
         * value into shader rgba values.
         */
        uint8_t swizzle[4];
};

#define SWIZ(x,y,z,w) {          \
        PIPE_SWIZZLE_##x, \
        PIPE_SWIZZLE_##y, \
        PIPE_SWIZZLE_##z, \
        PIPE_SWIZZLE_##w  \
}

#define FORMAT(pipe, rt, tex, swiz)                                     \
        [PIPE_FORMAT_##pipe] = { true, RT_##rt, VC4_TEXTURE_TYPE_##tex, swiz }

static const struct vc4_format vc4_format_table[] = {
        FORMAT(R8G8B8A8_UNORM, RGBA8888, RGBA8888, SWIZ(X, Y, Z, W)),
        FORMAT(R8G8B8X8_UNORM, RGBA8888, RGBA8888, SWIZ(X, Y, Z, 1)),
        FORMAT(R8G8B8A8_SRGB,  RGBA8888, RGBA8888, SWIZ(X, Y, Z, W)),
        FORMAT(R8G8B8X8_SRGB,  RGBA8888, RGBA8888, SWIZ(X, Y, Z, 1)),

        FORMAT(B8G8R8A8_UNORM, RGBA8888, RGBA8888, SWIZ(Z, Y, X, W)),
        FORMAT(B8G8R8X8_UNORM, RGBA8888, RGBA8888, SWIZ(Z, Y, X, 1)),
        FORMAT(B8G8R8A8_SRGB, RGBA8888, RGBA8888, SWIZ(Z, Y, X, W)),
        FORMAT(B8G8R8X8_SRGB, RGBA8888, RGBA8888, SWIZ(Z, Y, X, 1)),

        FORMAT(B5G6R5_UNORM, RGB565, RGB565, SWIZ(X, Y, Z, 1)),

        FORMAT(ETC1_RGB8, NO, ETC1, SWIZ(X, Y, Z, 1)),

        /* Depth sampling will be handled by doing nearest filtering and not
         * unpacking the RGBA value.
         */
        FORMAT(S8_UINT_Z24_UNORM, NO, RGBA8888, SWIZ(X, Y, Z, W)),
        FORMAT(X8Z24_UNORM,       NO, RGBA8888, SWIZ(X, Y, Z, W)),

        FORMAT(B4G4R4A4_UNORM, NO, RGBA4444, SWIZ(Y, Z, W, X)),
        FORMAT(B4G4R4X4_UNORM, NO, RGBA4444, SWIZ(Y, Z, W, 1)),

        FORMAT(A1B5G5R5_UNORM, NO, RGBA5551, SWIZ(X, Y, Z, W)),
        FORMAT(X1B5G5R5_UNORM, NO, RGBA5551, SWIZ(X, Y, Z, 1)),

        FORMAT(A8_UNORM, NO, ALPHA, SWIZ(0, 0, 0, W)),
        FORMAT(L8_UNORM, NO, ALPHA, SWIZ(W, W, W, 1)),
        FORMAT(I8_UNORM, NO, ALPHA, SWIZ(W, W, W, W)),
        FORMAT(R8_UNORM, NO, ALPHA, SWIZ(W, 0, 0, 1)),

        FORMAT(L8A8_UNORM, NO, LUMALPHA, SWIZ(X, X, X, W)),
        FORMAT(R8G8_UNORM, NO, LUMALPHA, SWIZ(X, W, 0, 1)),
};

static const struct vc4_format *
get_format(enum pipe_format f)
{
        if (f >= ARRAY_SIZE(vc4_format_table) ||
            !vc4_format_table[f].present)
                return NULL;
        else
                return &vc4_format_table[f];
}

bool
vc4_rt_format_supported(enum pipe_format f)
{
        const struct vc4_format *vf = get_format(f);

        if (!vf)
                return false;

        return vf->rt_type != RT_NO;
}

bool
vc4_rt_format_is_565(enum pipe_format f)
{
        const struct vc4_format *vf = get_format(f);

        if (!vf)
                return false;

        return vf->rt_type == RT_RGB565;
}

bool
vc4_tex_format_supported(enum pipe_format f)
{
        const struct vc4_format *vf = get_format(f);

        return vf != NULL;
}

uint8_t
vc4_get_tex_format(enum pipe_format f)
{
        const struct vc4_format *vf = get_format(f);

        if (!vf)
                return 0;

        return vf->tex_type;
}

const uint8_t *
vc4_get_format_swizzle(enum pipe_format f)
{
        const struct vc4_format *vf = get_format(f);
        static const uint8_t fallback[] = {0, 1, 2, 3};

        if (!vf)
                return fallback;

        return vf->swizzle;
}
