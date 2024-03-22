/* Copyright 2023 Advanced Micro Devices, Inc.
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

#include "vpe10_background.h"
#include "common.h"
#include "vpe_priv.h"

bool vpe10_split_bg_gap(struct vpe_rect *gaps, const struct vpe_rect *target_rect,
    uint32_t max_width, uint16_t max_gaps, uint16_t *num_gaps, uint16_t num_multiple)
{
    uint16_t gap_cnt, gap_idx, num_gaps_t;
    uint16_t prev_idx = *num_gaps - 1;
    uint32_t gap_width, gap_height;
    int32_t  gap_x, gap_y;

    // -1 is for removing the previous "going-to-be" splitted segment
    num_gaps_t = *num_gaps - 1;
    gap_x      = gaps[prev_idx].x;
    gap_y      = gaps[prev_idx].y;
    gap_width  = gaps[prev_idx].width;
    gap_height = gaps[prev_idx].height;

    gap_cnt = (uint16_t)((gap_width + max_width - 1) / max_width);

    if (gap_cnt % num_multiple != 0) {
        gap_cnt += (num_multiple - (gap_cnt % num_multiple));
        max_width = (uint16_t)((gap_width + gap_cnt - 1) / gap_cnt);
    }

    if (num_gaps_t + gap_cnt > max_gaps)
        return false;

    for (gap_idx = prev_idx; gap_idx < num_gaps_t + gap_cnt; gap_idx++) {
        gaps[gap_idx].y      = gap_y;
        gaps[gap_idx].height = gap_height;
        gaps[gap_idx].x      = gap_x;
        gaps[gap_idx].width  = gap_width < max_width ? gap_width : max_width;

        gap_x     = gap_x + (int32_t)gaps[gap_idx].width;
        gap_width = gap_width - gaps[gap_idx].width;
    }

    *num_gaps = num_gaps_t + gap_cnt;
    return true;
}
