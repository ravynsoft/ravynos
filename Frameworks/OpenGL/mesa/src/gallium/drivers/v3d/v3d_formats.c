/*
 * Copyright © 2014-2017 Broadcom
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
 * @file v3d_formats.c
 *
 * Contains the table and accessors for V3D texture and render target format
 * support.
 *
 * The hardware has limited support for texture formats, and extremely limited
 * support for render target formats.  As a result, we emulate other formats
 * in our shader code, and this stores the table for doing so.
 */

#include "util/macros.h"

#include "v3d_context.h"
#include "v3d_format_table.h"

/* The format internal types are the same across V3D versions */
#define V3D_VERSION 42
#include "broadcom/cle/v3dx_pack.h"

bool
v3d_rt_format_supported(const struct v3d_device_info *devinfo,
                        enum pipe_format f)
{
        const struct v3d_format *vf = v3d_X(devinfo, get_format_desc)(f);

        if (!vf)
                return false;

        return vf->rt_type != V3D_OUTPUT_IMAGE_FORMAT_NO;
}

uint8_t
v3d_get_rt_format(const struct v3d_device_info *devinfo, enum pipe_format f)
{
        const struct v3d_format *vf = v3d_X(devinfo, get_format_desc)(f);

        if (!vf)
                return 0;

        return vf->rt_type;
}

bool
v3d_tex_format_supported(const struct v3d_device_info *devinfo,
                         enum pipe_format f)
{
        const struct v3d_format *vf = v3d_X(devinfo, get_format_desc)(f);

        return vf != NULL;
}

uint8_t
v3d_get_tex_format(const struct v3d_device_info *devinfo, enum pipe_format f)
{
        const struct v3d_format *vf = v3d_X(devinfo, get_format_desc)(f);

        if (!vf)
                return 0;

        return vf->tex_type;
}

uint8_t
v3d_get_tex_return_size(const struct v3d_device_info *devinfo,
                        enum pipe_format f)
{
        const struct v3d_format *vf = v3d_X(devinfo, get_format_desc)(f);

        if (!vf)
                return 0;

        if (V3D_DBG(TMU_16BIT))
                return 16;

        if (V3D_DBG(TMU_32BIT))
                return 32;

        return vf->return_size;
}

uint8_t
v3d_get_tex_return_channels(const struct v3d_device_info *devinfo,
                            enum pipe_format f)
{
        const struct v3d_format *vf = v3d_X(devinfo, get_format_desc)(f);

        if (!vf)
                return 0;

        return vf->return_channels;
}

const uint8_t *
v3d_get_format_swizzle(const struct v3d_device_info *devinfo, enum pipe_format f)
{
        const struct v3d_format *vf = v3d_X(devinfo, get_format_desc)(f);
        static const uint8_t fallback[] = {0, 1, 2, 3};

        if (!vf)
                return fallback;

        return vf->swizzle;
}

bool
v3d_format_supports_tlb_msaa_resolve(const struct v3d_device_info *devinfo,
                                     enum pipe_format f)
{
        uint32_t internal_type;
        uint32_t internal_bpp;

        const struct v3d_format *vf = v3d_X(devinfo, get_format_desc)(f);

        if (!vf)
                return false;

        v3d_X(devinfo, get_internal_type_bpp_for_output_format)
           (vf->rt_type, &internal_type, &internal_bpp);

        return internal_type == V3D_INTERNAL_TYPE_8 ||
               internal_type == V3D_INTERNAL_TYPE_16F;
}
