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

#pragma once

#include "fixed31_32.h"
#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_PTS_IN_REGION     16
#define NUM_REGIONS           32
#define MAX_HW_POINTS         (NUM_PTS_IN_REGION * NUM_REGIONS)
#define MAX_HW_POINTS_DEGAMMA 257

enum table_type {
    type_pq_table,
    type_de_pq_table
};

bool vpe_color_is_table_init(enum table_type type);

struct fixed31_32 *vpe_color_get_table(enum table_type type);

void vpe_color_set_table_init_state(enum table_type type, bool state);

struct vpe_csc_matrix {
    enum color_space cs;
    uint16_t         regval[12];
};

static const struct vpe_csc_matrix vpe_input_csc_matrix_fixed[] = {
    {COLOR_SPACE_SRGB, {0x2000, 0, 0, 0, 0, 0x2000, 0, 0, 0, 0, 0x2000, 0}},
    {COLOR_SPACE_YCBCR601,
        {0x2cdd, 0x2000, 0, 0xe991, 0xe926, 0x2000, 0xf4fd, 0x10ef, 0, 0x2000, 0x38b4, 0xe3a6}},
    {COLOR_SPACE_YCBCR709,
        {0x3265, 0x2000, 0, 0xe6ce, 0xf105, 0x2000, 0xfa01, 0xa7d, 0, 0x2000, 0x3b61, 0xe24f}},
    {COLOR_SPACE_2020_YCBCR,
        {0x2f2f, 0x2000, 0, 0xe869, 0xedb8, 0x2000, 0xfabc, 0xbc6, 0x0, 0x2000, 0x3c34, 0xe1e6}}};

#ifdef __cplusplus
}
#endif
