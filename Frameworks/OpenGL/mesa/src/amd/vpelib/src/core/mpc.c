/* Copyright 2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#include <string.h>
#include "vpe_priv.h"
#include "common.h"
#include "mpc.h"

enum mpc_color_space_type {
    COLOR_SPACE_RGB_TYPE,
    COLOR_SPACE_RGB_LIMITED_8PBC_TYPE,
    COLOR_SPACE_RGB_LIMITED_10PBC_TYPE,
    COLOR_SPACE_YCBCR601_TYPE,
    COLOR_SPACE_YCBCR709_TYPE,
    COLOR_SPACE_YCBCR2020_TYPE,
    COLOR_SPACE_YCBCR601_LIMITED_TYPE,
    COLOR_SPACE_YCBCR709_LIMITED_TYPE,
    // COLOR_SPACE_YCBCR709_BLACK_TYPE,
};
struct out_csc_color_matrix_type {
    enum mpc_color_space_type color_space_type;
    uint16_t                  regval[12];
};

static const struct out_csc_color_matrix_type output_csc_matrix[] = {
    {COLOR_SPACE_RGB_TYPE, {0x2000, 0, 0, 0, 0, 0x2000, 0, 0, 0, 0, 0x2000, 0}},
    {COLOR_SPACE_RGB_LIMITED_8PBC_TYPE,
        {0x1B7B, 0, 0, 0x202, 0, 0x1B7B, 0, 0x202, 0, 0, 0x1B7B, 0x202}},
    {COLOR_SPACE_RGB_LIMITED_10PBC_TYPE,
        {0x1B66, 0, 0, 0x200, 0, 0x1B66, 0, 0x200, 0, 0, 0x1B66, 0x200}},
    {COLOR_SPACE_YCBCR601_TYPE, {0xE04, 0xF444, 0xFDB9, 0x1004, 0x831, 0x1016, 0x320, 0x201, 0xFB45,
                                    0xF6B7, 0xE04, 0x1004}},
    {COLOR_SPACE_YCBCR709_TYPE, {0xE04, 0xF345, 0xFEB7, 0x1004, 0x5D3, 0x1399, 0x1FA, 0x201, 0xFCCA,
                                    0xF533, 0xE04, 0x1004}},
    /* TODO: correct values below */
    {COLOR_SPACE_YCBCR601_LIMITED_TYPE, {0xE00, 0xF447, 0xFDB9, 0x1000, 0x991, 0x12C9, 0x3A6, 0x200,
                                            0xFB47, 0xF6B9, 0xE00, 0x1000}},
    {COLOR_SPACE_YCBCR709_LIMITED_TYPE, {0xE00, 0xF349, 0xFEB7, 0x1000, 0x6CE, 0x16E3, 0x24F, 0x200,
                                            0xFCCB, 0xF535, 0xE00, 0x1000}},
    {COLOR_SPACE_YCBCR2020_TYPE, {0x1000, 0xF149, 0xFEB7, 0x0000, 0x0868, 0x15B2, 0x01E6, 0x0000,
                                     0xFB88, 0xF478, 0x1000, 0x0000}},
};

static bool is_rgb_full_type(enum color_space color_space)
{
    bool ret = false;

    if (color_space == COLOR_SPACE_SRGB || color_space == COLOR_SPACE_MSREF_SCRGB ||
        color_space == COLOR_SPACE_2020_RGB_FULLRANGE)
        ret = true;

    return ret;
}

static bool is_rgb_limited_type(enum color_space color_space)
{
    bool ret = false;

    if (color_space == COLOR_SPACE_SRGB_LIMITED || color_space == COLOR_SPACE_2020_RGB_LIMITEDRANGE)
        ret = true;

    return ret;
}

static bool is_ycbcr601_full_type(enum color_space color_space)
{
    bool ret = false;

    if (color_space == COLOR_SPACE_YCBCR601)
        ret = true;

    return ret;
}

static bool is_ycbcr601_limited_type(enum color_space color_space)
{
    bool ret = false;

    if (color_space == COLOR_SPACE_YCBCR601_LIMITED)
        ret = true;

    return ret;
}

static bool is_ycbcr709_full_type(enum color_space color_space)
{
    bool ret = false;

    if (color_space == COLOR_SPACE_YCBCR709)
        ret = true;

    return ret;
}

static bool is_ycbcr709_limited_type(enum color_space color_space)
{
    bool ret = false;

    if (color_space == COLOR_SPACE_YCBCR709_LIMITED)
        ret = true;

    return ret;
}

static bool is_ycbcr2020_type(enum color_space color_space)
{
    bool ret = false;

    if (color_space == COLOR_SPACE_2020_YCBCR)
        ret = true;

    return ret;
}

static enum mpc_color_space_type get_color_space_type(
    enum color_space color_space, enum vpe_surface_pixel_format pixel_format)
{
    enum mpc_color_space_type type = COLOR_SPACE_RGB_TYPE;

    if (is_rgb_full_type(color_space))
        type = COLOR_SPACE_RGB_TYPE;
    else if (is_rgb_limited_type(color_space))
        type = vpe_is_rgb8(pixel_format) ? COLOR_SPACE_RGB_LIMITED_8PBC_TYPE
                                         : COLOR_SPACE_RGB_LIMITED_10PBC_TYPE;
    else if (is_ycbcr601_full_type(color_space))
        type = COLOR_SPACE_YCBCR601_TYPE;
    else if (is_ycbcr709_full_type(color_space))
        type = COLOR_SPACE_YCBCR709_TYPE;
    else if (is_ycbcr601_limited_type(color_space))
        type = COLOR_SPACE_YCBCR601_LIMITED_TYPE;
    else if (is_ycbcr709_limited_type(color_space))
        type = COLOR_SPACE_YCBCR709_LIMITED_TYPE;
    else if (is_ycbcr2020_type(color_space))
        type = COLOR_SPACE_YCBCR2020_TYPE;

    return type;
}

#define NUM_ELEMENTS(a) (sizeof(a) / sizeof((a)[0]))

const uint16_t *vpe_find_color_matrix(
    enum color_space color_space, enum vpe_surface_pixel_format pixel_format, uint32_t *array_size)
{
    int                       i;
    enum mpc_color_space_type type;
    const uint16_t           *val      = NULL;
    int                       arr_size = NUM_ELEMENTS(output_csc_matrix);

    type = get_color_space_type(color_space, pixel_format);
    for (i = 0; i < arr_size; i++)
        if (output_csc_matrix[i].color_space_type == type) {
            val         = output_csc_matrix[i].regval;
            *array_size = 12;
            break;
        }

    return val;
}
