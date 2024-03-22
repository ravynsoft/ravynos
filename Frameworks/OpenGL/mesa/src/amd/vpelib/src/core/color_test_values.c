
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

#include "color_test_values.h"

// facilitate fpga testing with test 3d lut values

static const struct fixed31_32 hdr_mult_sdr = {281474976710656}; // 2^16

bool build_test_shaper_sdr(struct transfer_func *shaper)
{
    if (shaper == NULL)
        return false;

    memset(shaper, 0, sizeof(struct transfer_func));

    shaper->type = TF_TYPE_HWPWL;
    shaper->tf =
        TRANSFER_FUNC_SRGB; // it's 2.2, but no such type - we use PWL values, so irrelevant
    shaper->sdr_ref_white_level = 80;
    shaper->pwl.hw_points_num   = 256;

    shaper->pwl.arr_curve_points[0].offset        = 0;
    shaper->pwl.arr_curve_points[0].segments_num  = 0;
    shaper->pwl.arr_curve_points[1].offset        = 1;
    shaper->pwl.arr_curve_points[1].segments_num  = 0;
    shaper->pwl.arr_curve_points[2].offset        = 2;
    shaper->pwl.arr_curve_points[2].segments_num  = 0;
    shaper->pwl.arr_curve_points[3].offset        = 3;
    shaper->pwl.arr_curve_points[3].segments_num  = 0;
    shaper->pwl.arr_curve_points[4].offset        = 4;
    shaper->pwl.arr_curve_points[4].segments_num  = 0;
    shaper->pwl.arr_curve_points[5].offset        = 5;
    shaper->pwl.arr_curve_points[5].segments_num  = 0;
    shaper->pwl.arr_curve_points[6].offset        = 6;
    shaper->pwl.arr_curve_points[6].segments_num  = 0;
    shaper->pwl.arr_curve_points[7].offset        = 7;
    shaper->pwl.arr_curve_points[7].segments_num  = 0;
    shaper->pwl.arr_curve_points[8].offset        = 8;
    shaper->pwl.arr_curve_points[8].segments_num  = 0;
    shaper->pwl.arr_curve_points[9].offset        = 9;
    shaper->pwl.arr_curve_points[9].segments_num  = 0;
    shaper->pwl.arr_curve_points[10].offset       = 10;
    shaper->pwl.arr_curve_points[10].segments_num = 0;
    shaper->pwl.arr_curve_points[11].offset       = 11;
    shaper->pwl.arr_curve_points[11].segments_num = 0;
    shaper->pwl.arr_curve_points[12].offset       = 12;
    shaper->pwl.arr_curve_points[12].segments_num = 0;
    shaper->pwl.arr_curve_points[13].offset       = 13;
    shaper->pwl.arr_curve_points[13].segments_num = 0;
    shaper->pwl.arr_curve_points[14].offset       = 14;
    shaper->pwl.arr_curve_points[14].segments_num = 0;
    shaper->pwl.arr_curve_points[15].offset       = 15;
    shaper->pwl.arr_curve_points[15].segments_num = 0;
    shaper->pwl.arr_curve_points[16].offset       = 16;
    shaper->pwl.arr_curve_points[16].segments_num = 0;
    shaper->pwl.arr_curve_points[17].offset       = 17;
    shaper->pwl.arr_curve_points[17].segments_num = 0;
    shaper->pwl.arr_curve_points[18].offset       = 18;
    shaper->pwl.arr_curve_points[18].segments_num = 1;
    shaper->pwl.arr_curve_points[19].offset       = 20;
    shaper->pwl.arr_curve_points[19].segments_num = 1;
    shaper->pwl.arr_curve_points[20].offset       = 22;
    shaper->pwl.arr_curve_points[20].segments_num = 1;
    shaper->pwl.arr_curve_points[21].offset       = 24;
    shaper->pwl.arr_curve_points[21].segments_num = 2;
    shaper->pwl.arr_curve_points[22].offset       = 28;
    shaper->pwl.arr_curve_points[22].segments_num = 2;
    shaper->pwl.arr_curve_points[23].offset       = 32;
    shaper->pwl.arr_curve_points[23].segments_num = 3;
    shaper->pwl.arr_curve_points[24].offset       = 40;
    shaper->pwl.arr_curve_points[24].segments_num = 3;
    shaper->pwl.arr_curve_points[25].offset       = 48;
    shaper->pwl.arr_curve_points[25].segments_num = 4;
    shaper->pwl.arr_curve_points[26].offset       = 64;
    shaper->pwl.arr_curve_points[26].segments_num = 4;
    shaper->pwl.arr_curve_points[27].offset       = 80;
    shaper->pwl.arr_curve_points[27].segments_num = 4;
    shaper->pwl.arr_curve_points[28].offset       = 96;
    shaper->pwl.arr_curve_points[28].segments_num = 4;
    shaper->pwl.arr_curve_points[29].offset       = 112;
    shaper->pwl.arr_curve_points[29].segments_num = 4;
    shaper->pwl.arr_curve_points[30].offset       = 128;
    shaper->pwl.arr_curve_points[30].segments_num = 5;
    shaper->pwl.arr_curve_points[31].offset       = 160;
    shaper->pwl.arr_curve_points[31].segments_num = 5;
    shaper->pwl.arr_curve_points[32].offset       = 192;
    shaper->pwl.arr_curve_points[32].segments_num = 5;
    shaper->pwl.arr_curve_points[33].offset       = 224;
    shaper->pwl.arr_curve_points[33].segments_num = 5;

    shaper->pwl.arr_points[0].x.value             = 0;
    shaper->pwl.arr_points[0].y.value             = 0;
    shaper->pwl.arr_points[0].offset.value        = 0;
    shaper->pwl.arr_points[0].slope.value         = 0;
    shaper->pwl.arr_points[0].custom_float_x      = 57344;
    shaper->pwl.arr_points[0].custom_float_y      = 0;
    shaper->pwl.arr_points[0].custom_float_offset = 0;
    shaper->pwl.arr_points[0].custom_float_slope  = 0;
    shaper->pwl.arr_points[1].x.value             = 0;
    shaper->pwl.arr_points[1].y.value             = 0;
    shaper->pwl.arr_points[1].offset.value        = 0;
    shaper->pwl.arr_points[1].slope.value         = 0;
    shaper->pwl.arr_points[1].custom_float_x      = 57344;
    shaper->pwl.arr_points[1].custom_float_y      = 0;
    shaper->pwl.arr_points[1].custom_float_offset = 0;
    shaper->pwl.arr_points[1].custom_float_slope  = 0;

    shaper->pwl.corner_points[0].red.custom_float_x   = 57344;
    shaper->pwl.corner_points[0].green.custom_float_x = 57344;
    shaper->pwl.corner_points[0].blue.custom_float_x  = 57344;
    shaper->pwl.corner_points[0].red.custom_float_y   = 0;
    shaper->pwl.corner_points[0].green.custom_float_y = 0;
    shaper->pwl.corner_points[0].blue.custom_float_y  = 0;
    shaper->pwl.corner_points[1].red.custom_float_x   = 48128;
    shaper->pwl.corner_points[1].green.custom_float_x = 48128;
    shaper->pwl.corner_points[1].blue.custom_float_x  = 48128;
    shaper->pwl.corner_points[1].red.custom_float_y   = 16383;
    shaper->pwl.corner_points[1].green.custom_float_y = 16383;
    shaper->pwl.corner_points[1].blue.custom_float_y  = 16383;

    shaper->pwl.rgb_resulted[0].red_reg           = 0;
    shaper->pwl.rgb_resulted[0].green_reg         = 0;
    shaper->pwl.rgb_resulted[0].blue_reg          = 0;
    shaper->pwl.rgb_resulted[0].delta_red_reg     = 1;
    shaper->pwl.rgb_resulted[0].delta_green_reg   = 1;
    shaper->pwl.rgb_resulted[0].delta_blue_reg    = 1;
    shaper->pwl.rgb_resulted[1].red_reg           = 1;
    shaper->pwl.rgb_resulted[1].green_reg         = 1;
    shaper->pwl.rgb_resulted[1].blue_reg          = 1;
    shaper->pwl.rgb_resulted[1].delta_red_reg     = 0;
    shaper->pwl.rgb_resulted[1].delta_green_reg   = 0;
    shaper->pwl.rgb_resulted[1].delta_blue_reg    = 0;
    shaper->pwl.rgb_resulted[2].red_reg           = 1;
    shaper->pwl.rgb_resulted[2].green_reg         = 1;
    shaper->pwl.rgb_resulted[2].blue_reg          = 1;
    shaper->pwl.rgb_resulted[2].delta_red_reg     = 0;
    shaper->pwl.rgb_resulted[2].delta_green_reg   = 0;
    shaper->pwl.rgb_resulted[2].delta_blue_reg    = 0;
    shaper->pwl.rgb_resulted[3].red_reg           = 1;
    shaper->pwl.rgb_resulted[3].green_reg         = 1;
    shaper->pwl.rgb_resulted[3].blue_reg          = 1;
    shaper->pwl.rgb_resulted[3].delta_red_reg     = 1;
    shaper->pwl.rgb_resulted[3].delta_green_reg   = 1;
    shaper->pwl.rgb_resulted[3].delta_blue_reg    = 1;
    shaper->pwl.rgb_resulted[4].red_reg           = 2;
    shaper->pwl.rgb_resulted[4].green_reg         = 2;
    shaper->pwl.rgb_resulted[4].blue_reg          = 2;
    shaper->pwl.rgb_resulted[4].delta_red_reg     = 0;
    shaper->pwl.rgb_resulted[4].delta_green_reg   = 0;
    shaper->pwl.rgb_resulted[4].delta_blue_reg    = 0;
    shaper->pwl.rgb_resulted[5].red_reg           = 2;
    shaper->pwl.rgb_resulted[5].green_reg         = 2;
    shaper->pwl.rgb_resulted[5].blue_reg          = 2;
    shaper->pwl.rgb_resulted[5].delta_red_reg     = 1;
    shaper->pwl.rgb_resulted[5].delta_green_reg   = 1;
    shaper->pwl.rgb_resulted[5].delta_blue_reg    = 1;
    shaper->pwl.rgb_resulted[6].red_reg           = 3;
    shaper->pwl.rgb_resulted[6].green_reg         = 3;
    shaper->pwl.rgb_resulted[6].blue_reg          = 3;
    shaper->pwl.rgb_resulted[6].delta_red_reg     = 2;
    shaper->pwl.rgb_resulted[6].delta_green_reg   = 2;
    shaper->pwl.rgb_resulted[6].delta_blue_reg    = 2;
    shaper->pwl.rgb_resulted[7].red_reg           = 5;
    shaper->pwl.rgb_resulted[7].green_reg         = 5;
    shaper->pwl.rgb_resulted[7].blue_reg          = 5;
    shaper->pwl.rgb_resulted[7].delta_red_reg     = 1;
    shaper->pwl.rgb_resulted[7].delta_green_reg   = 1;
    shaper->pwl.rgb_resulted[7].delta_blue_reg    = 1;
    shaper->pwl.rgb_resulted[8].red_reg           = 6;
    shaper->pwl.rgb_resulted[8].green_reg         = 6;
    shaper->pwl.rgb_resulted[8].blue_reg          = 6;
    shaper->pwl.rgb_resulted[8].delta_red_reg     = 3;
    shaper->pwl.rgb_resulted[8].delta_green_reg   = 3;
    shaper->pwl.rgb_resulted[8].delta_blue_reg    = 3;
    shaper->pwl.rgb_resulted[9].red_reg           = 9;
    shaper->pwl.rgb_resulted[9].green_reg         = 9;
    shaper->pwl.rgb_resulted[9].blue_reg          = 9;
    shaper->pwl.rgb_resulted[9].delta_red_reg     = 3;
    shaper->pwl.rgb_resulted[9].delta_green_reg   = 3;
    shaper->pwl.rgb_resulted[9].delta_blue_reg    = 3;
    shaper->pwl.rgb_resulted[10].red_reg          = 12;
    shaper->pwl.rgb_resulted[10].green_reg        = 12;
    shaper->pwl.rgb_resulted[10].blue_reg         = 12;
    shaper->pwl.rgb_resulted[10].delta_red_reg    = 4;
    shaper->pwl.rgb_resulted[10].delta_green_reg  = 4;
    shaper->pwl.rgb_resulted[10].delta_blue_reg   = 4;
    shaper->pwl.rgb_resulted[11].red_reg          = 16;
    shaper->pwl.rgb_resulted[11].green_reg        = 16;
    shaper->pwl.rgb_resulted[11].blue_reg         = 16;
    shaper->pwl.rgb_resulted[11].delta_red_reg    = 6;
    shaper->pwl.rgb_resulted[11].delta_green_reg  = 6;
    shaper->pwl.rgb_resulted[11].delta_blue_reg   = 6;
    shaper->pwl.rgb_resulted[12].red_reg          = 22;
    shaper->pwl.rgb_resulted[12].green_reg        = 22;
    shaper->pwl.rgb_resulted[12].blue_reg         = 22;
    shaper->pwl.rgb_resulted[12].delta_red_reg    = 8;
    shaper->pwl.rgb_resulted[12].delta_green_reg  = 8;
    shaper->pwl.rgb_resulted[12].delta_blue_reg   = 8;
    shaper->pwl.rgb_resulted[13].red_reg          = 30;
    shaper->pwl.rgb_resulted[13].green_reg        = 30;
    shaper->pwl.rgb_resulted[13].blue_reg         = 30;
    shaper->pwl.rgb_resulted[13].delta_red_reg    = 11;
    shaper->pwl.rgb_resulted[13].delta_green_reg  = 11;
    shaper->pwl.rgb_resulted[13].delta_blue_reg   = 11;
    shaper->pwl.rgb_resulted[14].red_reg          = 41;
    shaper->pwl.rgb_resulted[14].green_reg        = 41;
    shaper->pwl.rgb_resulted[14].blue_reg         = 41;
    shaper->pwl.rgb_resulted[14].delta_red_reg    = 15;
    shaper->pwl.rgb_resulted[14].delta_green_reg  = 15;
    shaper->pwl.rgb_resulted[14].delta_blue_reg   = 15;
    shaper->pwl.rgb_resulted[15].red_reg          = 56;
    shaper->pwl.rgb_resulted[15].green_reg        = 56;
    shaper->pwl.rgb_resulted[15].blue_reg         = 56;
    shaper->pwl.rgb_resulted[15].delta_red_reg    = 21;
    shaper->pwl.rgb_resulted[15].delta_green_reg  = 21;
    shaper->pwl.rgb_resulted[15].delta_blue_reg   = 21;
    shaper->pwl.rgb_resulted[16].red_reg          = 77;
    shaper->pwl.rgb_resulted[16].green_reg        = 77;
    shaper->pwl.rgb_resulted[16].blue_reg         = 77;
    shaper->pwl.rgb_resulted[16].delta_red_reg    = 29;
    shaper->pwl.rgb_resulted[16].delta_green_reg  = 29;
    shaper->pwl.rgb_resulted[16].delta_blue_reg   = 29;
    shaper->pwl.rgb_resulted[17].red_reg          = 106;
    shaper->pwl.rgb_resulted[17].green_reg        = 106;
    shaper->pwl.rgb_resulted[17].blue_reg         = 106;
    shaper->pwl.rgb_resulted[17].delta_red_reg    = 39;
    shaper->pwl.rgb_resulted[17].delta_green_reg  = 39;
    shaper->pwl.rgb_resulted[17].delta_blue_reg   = 39;
    shaper->pwl.rgb_resulted[18].red_reg          = 145;
    shaper->pwl.rgb_resulted[18].green_reg        = 145;
    shaper->pwl.rgb_resulted[18].blue_reg         = 145;
    shaper->pwl.rgb_resulted[18].delta_red_reg    = 30;
    shaper->pwl.rgb_resulted[18].delta_green_reg  = 30;
    shaper->pwl.rgb_resulted[18].delta_blue_reg   = 30;
    shaper->pwl.rgb_resulted[19].red_reg          = 175;
    shaper->pwl.rgb_resulted[19].green_reg        = 175;
    shaper->pwl.rgb_resulted[19].blue_reg         = 175;
    shaper->pwl.rgb_resulted[19].delta_red_reg    = 24;
    shaper->pwl.rgb_resulted[19].delta_green_reg  = 24;
    shaper->pwl.rgb_resulted[19].delta_blue_reg   = 24;
    shaper->pwl.rgb_resulted[20].red_reg          = 199;
    shaper->pwl.rgb_resulted[20].green_reg        = 199;
    shaper->pwl.rgb_resulted[20].blue_reg         = 199;
    shaper->pwl.rgb_resulted[20].delta_red_reg    = 40;
    shaper->pwl.rgb_resulted[20].delta_green_reg  = 40;
    shaper->pwl.rgb_resulted[20].delta_blue_reg   = 40;
    shaper->pwl.rgb_resulted[21].red_reg          = 239;
    shaper->pwl.rgb_resulted[21].green_reg        = 239;
    shaper->pwl.rgb_resulted[21].blue_reg         = 239;
    shaper->pwl.rgb_resulted[21].delta_red_reg    = 34;
    shaper->pwl.rgb_resulted[21].delta_green_reg  = 34;
    shaper->pwl.rgb_resulted[21].delta_blue_reg   = 34;
    shaper->pwl.rgb_resulted[22].red_reg          = 273;
    shaper->pwl.rgb_resulted[22].green_reg        = 273;
    shaper->pwl.rgb_resulted[22].blue_reg         = 273;
    shaper->pwl.rgb_resulted[22].delta_red_reg    = 55;
    shaper->pwl.rgb_resulted[22].delta_green_reg  = 55;
    shaper->pwl.rgb_resulted[22].delta_blue_reg   = 55;
    shaper->pwl.rgb_resulted[23].red_reg          = 328;
    shaper->pwl.rgb_resulted[23].green_reg        = 328;
    shaper->pwl.rgb_resulted[23].blue_reg         = 328;
    shaper->pwl.rgb_resulted[23].delta_red_reg    = 46;
    shaper->pwl.rgb_resulted[23].delta_green_reg  = 46;
    shaper->pwl.rgb_resulted[23].delta_blue_reg   = 46;
    shaper->pwl.rgb_resulted[24].red_reg          = 374;
    shaper->pwl.rgb_resulted[24].green_reg        = 374;
    shaper->pwl.rgb_resulted[24].blue_reg         = 374;
    shaper->pwl.rgb_resulted[24].delta_red_reg    = 39;
    shaper->pwl.rgb_resulted[24].delta_green_reg  = 39;
    shaper->pwl.rgb_resulted[24].delta_blue_reg   = 39;
    shaper->pwl.rgb_resulted[25].red_reg          = 413;
    shaper->pwl.rgb_resulted[25].green_reg        = 413;
    shaper->pwl.rgb_resulted[25].blue_reg         = 413;
    shaper->pwl.rgb_resulted[25].delta_red_reg    = 36;
    shaper->pwl.rgb_resulted[25].delta_green_reg  = 36;
    shaper->pwl.rgb_resulted[25].delta_blue_reg   = 36;
    shaper->pwl.rgb_resulted[26].red_reg          = 449;
    shaper->pwl.rgb_resulted[26].green_reg        = 449;
    shaper->pwl.rgb_resulted[26].blue_reg         = 449;
    shaper->pwl.rgb_resulted[26].delta_red_reg    = 33;
    shaper->pwl.rgb_resulted[26].delta_green_reg  = 33;
    shaper->pwl.rgb_resulted[26].delta_blue_reg   = 33;
    shaper->pwl.rgb_resulted[27].red_reg          = 482;
    shaper->pwl.rgb_resulted[27].green_reg        = 482;
    shaper->pwl.rgb_resulted[27].blue_reg         = 482;
    shaper->pwl.rgb_resulted[27].delta_red_reg    = 30;
    shaper->pwl.rgb_resulted[27].delta_green_reg  = 30;
    shaper->pwl.rgb_resulted[27].delta_blue_reg   = 30;
    shaper->pwl.rgb_resulted[28].red_reg          = 512;
    shaper->pwl.rgb_resulted[28].green_reg        = 512;
    shaper->pwl.rgb_resulted[28].blue_reg         = 512;
    shaper->pwl.rgb_resulted[28].delta_red_reg    = 55;
    shaper->pwl.rgb_resulted[28].delta_green_reg  = 55;
    shaper->pwl.rgb_resulted[28].delta_blue_reg   = 55;
    shaper->pwl.rgb_resulted[29].red_reg          = 567;
    shaper->pwl.rgb_resulted[29].green_reg        = 567;
    shaper->pwl.rgb_resulted[29].blue_reg         = 567;
    shaper->pwl.rgb_resulted[29].delta_red_reg    = 49;
    shaper->pwl.rgb_resulted[29].delta_green_reg  = 49;
    shaper->pwl.rgb_resulted[29].delta_blue_reg   = 49;
    shaper->pwl.rgb_resulted[30].red_reg          = 616;
    shaper->pwl.rgb_resulted[30].green_reg        = 616;
    shaper->pwl.rgb_resulted[30].blue_reg         = 616;
    shaper->pwl.rgb_resulted[30].delta_red_reg    = 44;
    shaper->pwl.rgb_resulted[30].delta_green_reg  = 44;
    shaper->pwl.rgb_resulted[30].delta_blue_reg   = 44;
    shaper->pwl.rgb_resulted[31].red_reg          = 660;
    shaper->pwl.rgb_resulted[31].green_reg        = 660;
    shaper->pwl.rgb_resulted[31].blue_reg         = 660;
    shaper->pwl.rgb_resulted[31].delta_red_reg    = 42;
    shaper->pwl.rgb_resulted[31].delta_green_reg  = 42;
    shaper->pwl.rgb_resulted[31].delta_blue_reg   = 42;
    shaper->pwl.rgb_resulted[32].red_reg          = 702;
    shaper->pwl.rgb_resulted[32].green_reg        = 702;
    shaper->pwl.rgb_resulted[32].blue_reg         = 702;
    shaper->pwl.rgb_resulted[32].delta_red_reg    = 38;
    shaper->pwl.rgb_resulted[32].delta_green_reg  = 38;
    shaper->pwl.rgb_resulted[32].delta_blue_reg   = 38;
    shaper->pwl.rgb_resulted[33].red_reg          = 740;
    shaper->pwl.rgb_resulted[33].green_reg        = 740;
    shaper->pwl.rgb_resulted[33].blue_reg         = 740;
    shaper->pwl.rgb_resulted[33].delta_red_reg    = 36;
    shaper->pwl.rgb_resulted[33].delta_green_reg  = 36;
    shaper->pwl.rgb_resulted[33].delta_blue_reg   = 36;
    shaper->pwl.rgb_resulted[34].red_reg          = 776;
    shaper->pwl.rgb_resulted[34].green_reg        = 776;
    shaper->pwl.rgb_resulted[34].blue_reg         = 776;
    shaper->pwl.rgb_resulted[34].delta_red_reg    = 35;
    shaper->pwl.rgb_resulted[34].delta_green_reg  = 35;
    shaper->pwl.rgb_resulted[34].delta_blue_reg   = 35;
    shaper->pwl.rgb_resulted[35].red_reg          = 811;
    shaper->pwl.rgb_resulted[35].green_reg        = 811;
    shaper->pwl.rgb_resulted[35].blue_reg         = 811;
    shaper->pwl.rgb_resulted[35].delta_red_reg    = 33;
    shaper->pwl.rgb_resulted[35].delta_green_reg  = 33;
    shaper->pwl.rgb_resulted[35].delta_blue_reg   = 33;
    shaper->pwl.rgb_resulted[36].red_reg          = 844;
    shaper->pwl.rgb_resulted[36].green_reg        = 844;
    shaper->pwl.rgb_resulted[36].blue_reg         = 844;
    shaper->pwl.rgb_resulted[36].delta_red_reg    = 31;
    shaper->pwl.rgb_resulted[36].delta_green_reg  = 31;
    shaper->pwl.rgb_resulted[36].delta_blue_reg   = 31;
    shaper->pwl.rgb_resulted[37].red_reg          = 875;
    shaper->pwl.rgb_resulted[37].green_reg        = 875;
    shaper->pwl.rgb_resulted[37].blue_reg         = 875;
    shaper->pwl.rgb_resulted[37].delta_red_reg    = 30;
    shaper->pwl.rgb_resulted[37].delta_green_reg  = 30;
    shaper->pwl.rgb_resulted[37].delta_blue_reg   = 30;
    shaper->pwl.rgb_resulted[38].red_reg          = 905;
    shaper->pwl.rgb_resulted[38].green_reg        = 905;
    shaper->pwl.rgb_resulted[38].blue_reg         = 905;
    shaper->pwl.rgb_resulted[38].delta_red_reg    = 29;
    shaper->pwl.rgb_resulted[38].delta_green_reg  = 29;
    shaper->pwl.rgb_resulted[38].delta_blue_reg   = 29;
    shaper->pwl.rgb_resulted[39].red_reg          = 934;
    shaper->pwl.rgb_resulted[39].green_reg        = 934;
    shaper->pwl.rgb_resulted[39].blue_reg         = 934;
    shaper->pwl.rgb_resulted[39].delta_red_reg    = 27;
    shaper->pwl.rgb_resulted[39].delta_green_reg  = 27;
    shaper->pwl.rgb_resulted[39].delta_blue_reg   = 27;
    shaper->pwl.rgb_resulted[40].red_reg          = 961;
    shaper->pwl.rgb_resulted[40].green_reg        = 961;
    shaper->pwl.rgb_resulted[40].blue_reg         = 961;
    shaper->pwl.rgb_resulted[40].delta_red_reg    = 53;
    shaper->pwl.rgb_resulted[40].delta_green_reg  = 53;
    shaper->pwl.rgb_resulted[40].delta_blue_reg   = 53;
    shaper->pwl.rgb_resulted[41].red_reg          = 1014;
    shaper->pwl.rgb_resulted[41].green_reg        = 1014;
    shaper->pwl.rgb_resulted[41].blue_reg         = 1014;
    shaper->pwl.rgb_resulted[41].delta_red_reg    = 50;
    shaper->pwl.rgb_resulted[41].delta_green_reg  = 50;
    shaper->pwl.rgb_resulted[41].delta_blue_reg   = 50;
    shaper->pwl.rgb_resulted[42].red_reg          = 1064;
    shaper->pwl.rgb_resulted[42].green_reg        = 1064;
    shaper->pwl.rgb_resulted[42].blue_reg         = 1064;
    shaper->pwl.rgb_resulted[42].delta_red_reg    = 47;
    shaper->pwl.rgb_resulted[42].delta_green_reg  = 47;
    shaper->pwl.rgb_resulted[42].delta_blue_reg   = 47;
    shaper->pwl.rgb_resulted[43].red_reg          = 1111;
    shaper->pwl.rgb_resulted[43].green_reg        = 1111;
    shaper->pwl.rgb_resulted[43].blue_reg         = 1111;
    shaper->pwl.rgb_resulted[43].delta_red_reg    = 45;
    shaper->pwl.rgb_resulted[43].delta_green_reg  = 45;
    shaper->pwl.rgb_resulted[43].delta_blue_reg   = 45;
    shaper->pwl.rgb_resulted[44].red_reg          = 1156;
    shaper->pwl.rgb_resulted[44].green_reg        = 1156;
    shaper->pwl.rgb_resulted[44].blue_reg         = 1156;
    shaper->pwl.rgb_resulted[44].delta_red_reg    = 43;
    shaper->pwl.rgb_resulted[44].delta_green_reg  = 43;
    shaper->pwl.rgb_resulted[44].delta_blue_reg   = 43;
    shaper->pwl.rgb_resulted[45].red_reg          = 1199;
    shaper->pwl.rgb_resulted[45].green_reg        = 1199;
    shaper->pwl.rgb_resulted[45].blue_reg         = 1199;
    shaper->pwl.rgb_resulted[45].delta_red_reg    = 41;
    shaper->pwl.rgb_resulted[45].delta_green_reg  = 41;
    shaper->pwl.rgb_resulted[45].delta_blue_reg   = 41;
    shaper->pwl.rgb_resulted[46].red_reg          = 1240;
    shaper->pwl.rgb_resulted[46].green_reg        = 1240;
    shaper->pwl.rgb_resulted[46].blue_reg         = 1240;
    shaper->pwl.rgb_resulted[46].delta_red_reg    = 39;
    shaper->pwl.rgb_resulted[46].delta_green_reg  = 39;
    shaper->pwl.rgb_resulted[46].delta_blue_reg   = 39;
    shaper->pwl.rgb_resulted[47].red_reg          = 1279;
    shaper->pwl.rgb_resulted[47].green_reg        = 1279;
    shaper->pwl.rgb_resulted[47].blue_reg         = 1279;
    shaper->pwl.rgb_resulted[47].delta_red_reg    = 38;
    shaper->pwl.rgb_resulted[47].delta_green_reg  = 38;
    shaper->pwl.rgb_resulted[47].delta_blue_reg   = 38;
    shaper->pwl.rgb_resulted[48].red_reg          = 1317;
    shaper->pwl.rgb_resulted[48].green_reg        = 1317;
    shaper->pwl.rgb_resulted[48].blue_reg         = 1317;
    shaper->pwl.rgb_resulted[48].delta_red_reg    = 37;
    shaper->pwl.rgb_resulted[48].delta_green_reg  = 37;
    shaper->pwl.rgb_resulted[48].delta_blue_reg   = 37;
    shaper->pwl.rgb_resulted[49].red_reg          = 1354;
    shaper->pwl.rgb_resulted[49].green_reg        = 1354;
    shaper->pwl.rgb_resulted[49].blue_reg         = 1354;
    shaper->pwl.rgb_resulted[49].delta_red_reg    = 36;
    shaper->pwl.rgb_resulted[49].delta_green_reg  = 36;
    shaper->pwl.rgb_resulted[49].delta_blue_reg   = 36;
    shaper->pwl.rgb_resulted[50].red_reg          = 1390;
    shaper->pwl.rgb_resulted[50].green_reg        = 1390;
    shaper->pwl.rgb_resulted[50].blue_reg         = 1390;
    shaper->pwl.rgb_resulted[50].delta_red_reg    = 35;
    shaper->pwl.rgb_resulted[50].delta_green_reg  = 35;
    shaper->pwl.rgb_resulted[50].delta_blue_reg   = 35;
    shaper->pwl.rgb_resulted[51].red_reg          = 1425;
    shaper->pwl.rgb_resulted[51].green_reg        = 1425;
    shaper->pwl.rgb_resulted[51].blue_reg         = 1425;
    shaper->pwl.rgb_resulted[51].delta_red_reg    = 33;
    shaper->pwl.rgb_resulted[51].delta_green_reg  = 33;
    shaper->pwl.rgb_resulted[51].delta_blue_reg   = 33;
    shaper->pwl.rgb_resulted[52].red_reg          = 1458;
    shaper->pwl.rgb_resulted[52].green_reg        = 1458;
    shaper->pwl.rgb_resulted[52].blue_reg         = 1458;
    shaper->pwl.rgb_resulted[52].delta_red_reg    = 33;
    shaper->pwl.rgb_resulted[52].delta_green_reg  = 33;
    shaper->pwl.rgb_resulted[52].delta_blue_reg   = 33;
    shaper->pwl.rgb_resulted[53].red_reg          = 1491;
    shaper->pwl.rgb_resulted[53].green_reg        = 1491;
    shaper->pwl.rgb_resulted[53].blue_reg         = 1491;
    shaper->pwl.rgb_resulted[53].delta_red_reg    = 32;
    shaper->pwl.rgb_resulted[53].delta_green_reg  = 32;
    shaper->pwl.rgb_resulted[53].delta_blue_reg   = 32;
    shaper->pwl.rgb_resulted[54].red_reg          = 1523;
    shaper->pwl.rgb_resulted[54].green_reg        = 1523;
    shaper->pwl.rgb_resulted[54].blue_reg         = 1523;
    shaper->pwl.rgb_resulted[54].delta_red_reg    = 31;
    shaper->pwl.rgb_resulted[54].delta_green_reg  = 31;
    shaper->pwl.rgb_resulted[54].delta_blue_reg   = 31;
    shaper->pwl.rgb_resulted[55].red_reg          = 1554;
    shaper->pwl.rgb_resulted[55].green_reg        = 1554;
    shaper->pwl.rgb_resulted[55].blue_reg         = 1554;
    shaper->pwl.rgb_resulted[55].delta_red_reg    = 30;
    shaper->pwl.rgb_resulted[55].delta_green_reg  = 30;
    shaper->pwl.rgb_resulted[55].delta_blue_reg   = 30;
    shaper->pwl.rgb_resulted[56].red_reg          = 1584;
    shaper->pwl.rgb_resulted[56].green_reg        = 1584;
    shaper->pwl.rgb_resulted[56].blue_reg         = 1584;
    shaper->pwl.rgb_resulted[56].delta_red_reg    = 30;
    shaper->pwl.rgb_resulted[56].delta_green_reg  = 30;
    shaper->pwl.rgb_resulted[56].delta_blue_reg   = 30;
    shaper->pwl.rgb_resulted[57].red_reg          = 1614;
    shaper->pwl.rgb_resulted[57].green_reg        = 1614;
    shaper->pwl.rgb_resulted[57].blue_reg         = 1614;
    shaper->pwl.rgb_resulted[57].delta_red_reg    = 29;
    shaper->pwl.rgb_resulted[57].delta_green_reg  = 29;
    shaper->pwl.rgb_resulted[57].delta_blue_reg   = 29;
    shaper->pwl.rgb_resulted[58].red_reg          = 1643;
    shaper->pwl.rgb_resulted[58].green_reg        = 1643;
    shaper->pwl.rgb_resulted[58].blue_reg         = 1643;
    shaper->pwl.rgb_resulted[58].delta_red_reg    = 28;
    shaper->pwl.rgb_resulted[58].delta_green_reg  = 28;
    shaper->pwl.rgb_resulted[58].delta_blue_reg   = 28;
    shaper->pwl.rgb_resulted[59].red_reg          = 1671;
    shaper->pwl.rgb_resulted[59].green_reg        = 1671;
    shaper->pwl.rgb_resulted[59].blue_reg         = 1671;
    shaper->pwl.rgb_resulted[59].delta_red_reg    = 28;
    shaper->pwl.rgb_resulted[59].delta_green_reg  = 28;
    shaper->pwl.rgb_resulted[59].delta_blue_reg   = 28;
    shaper->pwl.rgb_resulted[60].red_reg          = 1699;
    shaper->pwl.rgb_resulted[60].green_reg        = 1699;
    shaper->pwl.rgb_resulted[60].blue_reg         = 1699;
    shaper->pwl.rgb_resulted[60].delta_red_reg    = 27;
    shaper->pwl.rgb_resulted[60].delta_green_reg  = 27;
    shaper->pwl.rgb_resulted[60].delta_blue_reg   = 27;
    shaper->pwl.rgb_resulted[61].red_reg          = 1726;
    shaper->pwl.rgb_resulted[61].green_reg        = 1726;
    shaper->pwl.rgb_resulted[61].blue_reg         = 1726;
    shaper->pwl.rgb_resulted[61].delta_red_reg    = 27;
    shaper->pwl.rgb_resulted[61].delta_green_reg  = 27;
    shaper->pwl.rgb_resulted[61].delta_blue_reg   = 27;
    shaper->pwl.rgb_resulted[62].red_reg          = 1753;
    shaper->pwl.rgb_resulted[62].green_reg        = 1753;
    shaper->pwl.rgb_resulted[62].blue_reg         = 1753;
    shaper->pwl.rgb_resulted[62].delta_red_reg    = 27;
    shaper->pwl.rgb_resulted[62].delta_green_reg  = 27;
    shaper->pwl.rgb_resulted[62].delta_blue_reg   = 27;
    shaper->pwl.rgb_resulted[63].red_reg          = 1780;
    shaper->pwl.rgb_resulted[63].green_reg        = 1780;
    shaper->pwl.rgb_resulted[63].blue_reg         = 1780;
    shaper->pwl.rgb_resulted[63].delta_red_reg    = 25;
    shaper->pwl.rgb_resulted[63].delta_green_reg  = 25;
    shaper->pwl.rgb_resulted[63].delta_blue_reg   = 25;
    shaper->pwl.rgb_resulted[64].red_reg          = 1805;
    shaper->pwl.rgb_resulted[64].green_reg        = 1805;
    shaper->pwl.rgb_resulted[64].blue_reg         = 1805;
    shaper->pwl.rgb_resulted[64].delta_red_reg    = 51;
    shaper->pwl.rgb_resulted[64].delta_green_reg  = 51;
    shaper->pwl.rgb_resulted[64].delta_blue_reg   = 51;
    shaper->pwl.rgb_resulted[65].red_reg          = 1856;
    shaper->pwl.rgb_resulted[65].green_reg        = 1856;
    shaper->pwl.rgb_resulted[65].blue_reg         = 1856;
    shaper->pwl.rgb_resulted[65].delta_red_reg    = 49;
    shaper->pwl.rgb_resulted[65].delta_green_reg  = 49;
    shaper->pwl.rgb_resulted[65].delta_blue_reg   = 49;
    shaper->pwl.rgb_resulted[66].red_reg          = 1905;
    shaper->pwl.rgb_resulted[66].green_reg        = 1905;
    shaper->pwl.rgb_resulted[66].blue_reg         = 1905;
    shaper->pwl.rgb_resulted[66].delta_red_reg    = 47;
    shaper->pwl.rgb_resulted[66].delta_green_reg  = 47;
    shaper->pwl.rgb_resulted[66].delta_blue_reg   = 47;
    shaper->pwl.rgb_resulted[67].red_reg          = 1952;
    shaper->pwl.rgb_resulted[67].green_reg        = 1952;
    shaper->pwl.rgb_resulted[67].blue_reg         = 1952;
    shaper->pwl.rgb_resulted[67].delta_red_reg    = 46;
    shaper->pwl.rgb_resulted[67].delta_green_reg  = 46;
    shaper->pwl.rgb_resulted[67].delta_blue_reg   = 46;
    shaper->pwl.rgb_resulted[68].red_reg          = 1998;
    shaper->pwl.rgb_resulted[68].green_reg        = 1998;
    shaper->pwl.rgb_resulted[68].blue_reg         = 1998;
    shaper->pwl.rgb_resulted[68].delta_red_reg    = 45;
    shaper->pwl.rgb_resulted[68].delta_green_reg  = 45;
    shaper->pwl.rgb_resulted[68].delta_blue_reg   = 45;
    shaper->pwl.rgb_resulted[69].red_reg          = 2043;
    shaper->pwl.rgb_resulted[69].green_reg        = 2043;
    shaper->pwl.rgb_resulted[69].blue_reg         = 2043;
    shaper->pwl.rgb_resulted[69].delta_red_reg    = 44;
    shaper->pwl.rgb_resulted[69].delta_green_reg  = 44;
    shaper->pwl.rgb_resulted[69].delta_blue_reg   = 44;
    shaper->pwl.rgb_resulted[70].red_reg          = 2087;
    shaper->pwl.rgb_resulted[70].green_reg        = 2087;
    shaper->pwl.rgb_resulted[70].blue_reg         = 2087;
    shaper->pwl.rgb_resulted[70].delta_red_reg    = 42;
    shaper->pwl.rgb_resulted[70].delta_green_reg  = 42;
    shaper->pwl.rgb_resulted[70].delta_blue_reg   = 42;
    shaper->pwl.rgb_resulted[71].red_reg          = 2129;
    shaper->pwl.rgb_resulted[71].green_reg        = 2129;
    shaper->pwl.rgb_resulted[71].blue_reg         = 2129;
    shaper->pwl.rgb_resulted[71].delta_red_reg    = 42;
    shaper->pwl.rgb_resulted[71].delta_green_reg  = 42;
    shaper->pwl.rgb_resulted[71].delta_blue_reg   = 42;
    shaper->pwl.rgb_resulted[72].red_reg          = 2171;
    shaper->pwl.rgb_resulted[72].green_reg        = 2171;
    shaper->pwl.rgb_resulted[72].blue_reg         = 2171;
    shaper->pwl.rgb_resulted[72].delta_red_reg    = 40;
    shaper->pwl.rgb_resulted[72].delta_green_reg  = 40;
    shaper->pwl.rgb_resulted[72].delta_blue_reg   = 40;
    shaper->pwl.rgb_resulted[73].red_reg          = 2211;
    shaper->pwl.rgb_resulted[73].green_reg        = 2211;
    shaper->pwl.rgb_resulted[73].blue_reg         = 2211;
    shaper->pwl.rgb_resulted[73].delta_red_reg    = 40;
    shaper->pwl.rgb_resulted[73].delta_green_reg  = 40;
    shaper->pwl.rgb_resulted[73].delta_blue_reg   = 40;
    shaper->pwl.rgb_resulted[74].red_reg          = 2251;
    shaper->pwl.rgb_resulted[74].green_reg        = 2251;
    shaper->pwl.rgb_resulted[74].blue_reg         = 2251;
    shaper->pwl.rgb_resulted[74].delta_red_reg    = 39;
    shaper->pwl.rgb_resulted[74].delta_green_reg  = 39;
    shaper->pwl.rgb_resulted[74].delta_blue_reg   = 39;
    shaper->pwl.rgb_resulted[75].red_reg          = 2290;
    shaper->pwl.rgb_resulted[75].green_reg        = 2290;
    shaper->pwl.rgb_resulted[75].blue_reg         = 2290;
    shaper->pwl.rgb_resulted[75].delta_red_reg    = 38;
    shaper->pwl.rgb_resulted[75].delta_green_reg  = 38;
    shaper->pwl.rgb_resulted[75].delta_blue_reg   = 38;
    shaper->pwl.rgb_resulted[76].red_reg          = 2328;
    shaper->pwl.rgb_resulted[76].green_reg        = 2328;
    shaper->pwl.rgb_resulted[76].blue_reg         = 2328;
    shaper->pwl.rgb_resulted[76].delta_red_reg    = 38;
    shaper->pwl.rgb_resulted[76].delta_green_reg  = 38;
    shaper->pwl.rgb_resulted[76].delta_blue_reg   = 38;
    shaper->pwl.rgb_resulted[77].red_reg          = 2366;
    shaper->pwl.rgb_resulted[77].green_reg        = 2366;
    shaper->pwl.rgb_resulted[77].blue_reg         = 2366;
    shaper->pwl.rgb_resulted[77].delta_red_reg    = 36;
    shaper->pwl.rgb_resulted[77].delta_green_reg  = 36;
    shaper->pwl.rgb_resulted[77].delta_blue_reg   = 36;
    shaper->pwl.rgb_resulted[78].red_reg          = 2402;
    shaper->pwl.rgb_resulted[78].green_reg        = 2402;
    shaper->pwl.rgb_resulted[78].blue_reg         = 2402;
    shaper->pwl.rgb_resulted[78].delta_red_reg    = 37;
    shaper->pwl.rgb_resulted[78].delta_green_reg  = 37;
    shaper->pwl.rgb_resulted[78].delta_blue_reg   = 37;
    shaper->pwl.rgb_resulted[79].red_reg          = 2439;
    shaper->pwl.rgb_resulted[79].green_reg        = 2439;
    shaper->pwl.rgb_resulted[79].blue_reg         = 2439;
    shaper->pwl.rgb_resulted[79].delta_red_reg    = 35;
    shaper->pwl.rgb_resulted[79].delta_green_reg  = 35;
    shaper->pwl.rgb_resulted[79].delta_blue_reg   = 35;
    shaper->pwl.rgb_resulted[80].red_reg          = 2474;
    shaper->pwl.rgb_resulted[80].green_reg        = 2474;
    shaper->pwl.rgb_resulted[80].blue_reg         = 2474;
    shaper->pwl.rgb_resulted[80].delta_red_reg    = 69;
    shaper->pwl.rgb_resulted[80].delta_green_reg  = 69;
    shaper->pwl.rgb_resulted[80].delta_blue_reg   = 69;
    shaper->pwl.rgb_resulted[81].red_reg          = 2543;
    shaper->pwl.rgb_resulted[81].green_reg        = 2543;
    shaper->pwl.rgb_resulted[81].blue_reg         = 2543;
    shaper->pwl.rgb_resulted[81].delta_red_reg    = 67;
    shaper->pwl.rgb_resulted[81].delta_green_reg  = 67;
    shaper->pwl.rgb_resulted[81].delta_blue_reg   = 67;
    shaper->pwl.rgb_resulted[82].red_reg          = 2610;
    shaper->pwl.rgb_resulted[82].green_reg        = 2610;
    shaper->pwl.rgb_resulted[82].blue_reg         = 2610;
    shaper->pwl.rgb_resulted[82].delta_red_reg    = 65;
    shaper->pwl.rgb_resulted[82].delta_green_reg  = 65;
    shaper->pwl.rgb_resulted[82].delta_blue_reg   = 65;
    shaper->pwl.rgb_resulted[83].red_reg          = 2675;
    shaper->pwl.rgb_resulted[83].green_reg        = 2675;
    shaper->pwl.rgb_resulted[83].blue_reg         = 2675;
    shaper->pwl.rgb_resulted[83].delta_red_reg    = 63;
    shaper->pwl.rgb_resulted[83].delta_green_reg  = 63;
    shaper->pwl.rgb_resulted[83].delta_blue_reg   = 63;
    shaper->pwl.rgb_resulted[84].red_reg          = 2738;
    shaper->pwl.rgb_resulted[84].green_reg        = 2738;
    shaper->pwl.rgb_resulted[84].blue_reg         = 2738;
    shaper->pwl.rgb_resulted[84].delta_red_reg    = 62;
    shaper->pwl.rgb_resulted[84].delta_green_reg  = 62;
    shaper->pwl.rgb_resulted[84].delta_blue_reg   = 62;
    shaper->pwl.rgb_resulted[85].red_reg          = 2800;
    shaper->pwl.rgb_resulted[85].green_reg        = 2800;
    shaper->pwl.rgb_resulted[85].blue_reg         = 2800;
    shaper->pwl.rgb_resulted[85].delta_red_reg    = 59;
    shaper->pwl.rgb_resulted[85].delta_green_reg  = 59;
    shaper->pwl.rgb_resulted[85].delta_blue_reg   = 59;
    shaper->pwl.rgb_resulted[86].red_reg          = 2859;
    shaper->pwl.rgb_resulted[86].green_reg        = 2859;
    shaper->pwl.rgb_resulted[86].blue_reg         = 2859;
    shaper->pwl.rgb_resulted[86].delta_red_reg    = 59;
    shaper->pwl.rgb_resulted[86].delta_green_reg  = 59;
    shaper->pwl.rgb_resulted[86].delta_blue_reg   = 59;
    shaper->pwl.rgb_resulted[87].red_reg          = 2918;
    shaper->pwl.rgb_resulted[87].green_reg        = 2918;
    shaper->pwl.rgb_resulted[87].blue_reg         = 2918;
    shaper->pwl.rgb_resulted[87].delta_red_reg    = 57;
    shaper->pwl.rgb_resulted[87].delta_green_reg  = 57;
    shaper->pwl.rgb_resulted[87].delta_blue_reg   = 57;
    shaper->pwl.rgb_resulted[88].red_reg          = 2975;
    shaper->pwl.rgb_resulted[88].green_reg        = 2975;
    shaper->pwl.rgb_resulted[88].blue_reg         = 2975;
    shaper->pwl.rgb_resulted[88].delta_red_reg    = 55;
    shaper->pwl.rgb_resulted[88].delta_green_reg  = 55;
    shaper->pwl.rgb_resulted[88].delta_blue_reg   = 55;
    shaper->pwl.rgb_resulted[89].red_reg          = 3030;
    shaper->pwl.rgb_resulted[89].green_reg        = 3030;
    shaper->pwl.rgb_resulted[89].blue_reg         = 3030;
    shaper->pwl.rgb_resulted[89].delta_red_reg    = 55;
    shaper->pwl.rgb_resulted[89].delta_green_reg  = 55;
    shaper->pwl.rgb_resulted[89].delta_blue_reg   = 55;
    shaper->pwl.rgb_resulted[90].red_reg          = 3085;
    shaper->pwl.rgb_resulted[90].green_reg        = 3085;
    shaper->pwl.rgb_resulted[90].blue_reg         = 3085;
    shaper->pwl.rgb_resulted[90].delta_red_reg    = 53;
    shaper->pwl.rgb_resulted[90].delta_green_reg  = 53;
    shaper->pwl.rgb_resulted[90].delta_blue_reg   = 53;
    shaper->pwl.rgb_resulted[91].red_reg          = 3138;
    shaper->pwl.rgb_resulted[91].green_reg        = 3138;
    shaper->pwl.rgb_resulted[91].blue_reg         = 3138;
    shaper->pwl.rgb_resulted[91].delta_red_reg    = 53;
    shaper->pwl.rgb_resulted[91].delta_green_reg  = 53;
    shaper->pwl.rgb_resulted[91].delta_blue_reg   = 53;
    shaper->pwl.rgb_resulted[92].red_reg          = 3191;
    shaper->pwl.rgb_resulted[92].green_reg        = 3191;
    shaper->pwl.rgb_resulted[92].blue_reg         = 3191;
    shaper->pwl.rgb_resulted[92].delta_red_reg    = 51;
    shaper->pwl.rgb_resulted[92].delta_green_reg  = 51;
    shaper->pwl.rgb_resulted[92].delta_blue_reg   = 51;
    shaper->pwl.rgb_resulted[93].red_reg          = 3242;
    shaper->pwl.rgb_resulted[93].green_reg        = 3242;
    shaper->pwl.rgb_resulted[93].blue_reg         = 3242;
    shaper->pwl.rgb_resulted[93].delta_red_reg    = 50;
    shaper->pwl.rgb_resulted[93].delta_green_reg  = 50;
    shaper->pwl.rgb_resulted[93].delta_blue_reg   = 50;
    shaper->pwl.rgb_resulted[94].red_reg          = 3292;
    shaper->pwl.rgb_resulted[94].green_reg        = 3292;
    shaper->pwl.rgb_resulted[94].blue_reg         = 3292;
    shaper->pwl.rgb_resulted[94].delta_red_reg    = 50;
    shaper->pwl.rgb_resulted[94].delta_green_reg  = 50;
    shaper->pwl.rgb_resulted[94].delta_blue_reg   = 50;
    shaper->pwl.rgb_resulted[95].red_reg          = 3342;
    shaper->pwl.rgb_resulted[95].green_reg        = 3342;
    shaper->pwl.rgb_resulted[95].blue_reg         = 3342;
    shaper->pwl.rgb_resulted[95].delta_red_reg    = 48;
    shaper->pwl.rgb_resulted[95].delta_green_reg  = 48;
    shaper->pwl.rgb_resulted[95].delta_blue_reg   = 48;
    shaper->pwl.rgb_resulted[96].red_reg          = 3390;
    shaper->pwl.rgb_resulted[96].green_reg        = 3390;
    shaper->pwl.rgb_resulted[96].blue_reg         = 3390;
    shaper->pwl.rgb_resulted[96].delta_red_reg    = 95;
    shaper->pwl.rgb_resulted[96].delta_green_reg  = 95;
    shaper->pwl.rgb_resulted[96].delta_blue_reg   = 95;
    shaper->pwl.rgb_resulted[97].red_reg          = 3485;
    shaper->pwl.rgb_resulted[97].green_reg        = 3485;
    shaper->pwl.rgb_resulted[97].blue_reg         = 3485;
    shaper->pwl.rgb_resulted[97].delta_red_reg    = 92;
    shaper->pwl.rgb_resulted[97].delta_green_reg  = 92;
    shaper->pwl.rgb_resulted[97].delta_blue_reg   = 92;
    shaper->pwl.rgb_resulted[98].red_reg          = 3577;
    shaper->pwl.rgb_resulted[98].green_reg        = 3577;
    shaper->pwl.rgb_resulted[98].blue_reg         = 3577;
    shaper->pwl.rgb_resulted[98].delta_red_reg    = 89;
    shaper->pwl.rgb_resulted[98].delta_green_reg  = 89;
    shaper->pwl.rgb_resulted[98].delta_blue_reg   = 89;
    shaper->pwl.rgb_resulted[99].red_reg          = 3666;
    shaper->pwl.rgb_resulted[99].green_reg        = 3666;
    shaper->pwl.rgb_resulted[99].blue_reg         = 3666;
    shaper->pwl.rgb_resulted[99].delta_red_reg    = 86;
    shaper->pwl.rgb_resulted[99].delta_green_reg  = 86;
    shaper->pwl.rgb_resulted[99].delta_blue_reg   = 86;
    shaper->pwl.rgb_resulted[100].red_reg         = 3752;
    shaper->pwl.rgb_resulted[100].green_reg       = 3752;
    shaper->pwl.rgb_resulted[100].blue_reg        = 3752;
    shaper->pwl.rgb_resulted[100].delta_red_reg   = 84;
    shaper->pwl.rgb_resulted[100].delta_green_reg = 84;
    shaper->pwl.rgb_resulted[100].delta_blue_reg  = 84;
    shaper->pwl.rgb_resulted[101].red_reg         = 3836;
    shaper->pwl.rgb_resulted[101].green_reg       = 3836;
    shaper->pwl.rgb_resulted[101].blue_reg        = 3836;
    shaper->pwl.rgb_resulted[101].delta_red_reg   = 82;
    shaper->pwl.rgb_resulted[101].delta_green_reg = 82;
    shaper->pwl.rgb_resulted[101].delta_blue_reg  = 82;
    shaper->pwl.rgb_resulted[102].red_reg         = 3918;
    shaper->pwl.rgb_resulted[102].green_reg       = 3918;
    shaper->pwl.rgb_resulted[102].blue_reg        = 3918;
    shaper->pwl.rgb_resulted[102].delta_red_reg   = 80;
    shaper->pwl.rgb_resulted[102].delta_green_reg = 80;
    shaper->pwl.rgb_resulted[102].delta_blue_reg  = 80;
    shaper->pwl.rgb_resulted[103].red_reg         = 3998;
    shaper->pwl.rgb_resulted[103].green_reg       = 3998;
    shaper->pwl.rgb_resulted[103].blue_reg        = 3998;
    shaper->pwl.rgb_resulted[103].delta_red_reg   = 78;
    shaper->pwl.rgb_resulted[103].delta_green_reg = 78;
    shaper->pwl.rgb_resulted[103].delta_blue_reg  = 78;
    shaper->pwl.rgb_resulted[104].red_reg         = 4076;
    shaper->pwl.rgb_resulted[104].green_reg       = 4076;
    shaper->pwl.rgb_resulted[104].blue_reg        = 4076;
    shaper->pwl.rgb_resulted[104].delta_red_reg   = 77;
    shaper->pwl.rgb_resulted[104].delta_green_reg = 77;
    shaper->pwl.rgb_resulted[104].delta_blue_reg  = 77;
    shaper->pwl.rgb_resulted[105].red_reg         = 4153;
    shaper->pwl.rgb_resulted[105].green_reg       = 4153;
    shaper->pwl.rgb_resulted[105].blue_reg        = 4153;
    shaper->pwl.rgb_resulted[105].delta_red_reg   = 74;
    shaper->pwl.rgb_resulted[105].delta_green_reg = 74;
    shaper->pwl.rgb_resulted[105].delta_blue_reg  = 74;
    shaper->pwl.rgb_resulted[106].red_reg         = 4227;
    shaper->pwl.rgb_resulted[106].green_reg       = 4227;
    shaper->pwl.rgb_resulted[106].blue_reg        = 4227;
    shaper->pwl.rgb_resulted[106].delta_red_reg   = 74;
    shaper->pwl.rgb_resulted[106].delta_green_reg = 74;
    shaper->pwl.rgb_resulted[106].delta_blue_reg  = 74;
    shaper->pwl.rgb_resulted[107].red_reg         = 4301;
    shaper->pwl.rgb_resulted[107].green_reg       = 4301;
    shaper->pwl.rgb_resulted[107].blue_reg        = 4301;
    shaper->pwl.rgb_resulted[107].delta_red_reg   = 71;
    shaper->pwl.rgb_resulted[107].delta_green_reg = 71;
    shaper->pwl.rgb_resulted[107].delta_blue_reg  = 71;
    shaper->pwl.rgb_resulted[108].red_reg         = 4372;
    shaper->pwl.rgb_resulted[108].green_reg       = 4372;
    shaper->pwl.rgb_resulted[108].blue_reg        = 4372;
    shaper->pwl.rgb_resulted[108].delta_red_reg   = 71;
    shaper->pwl.rgb_resulted[108].delta_green_reg = 71;
    shaper->pwl.rgb_resulted[108].delta_blue_reg  = 71;
    shaper->pwl.rgb_resulted[109].red_reg         = 4443;
    shaper->pwl.rgb_resulted[109].green_reg       = 4443;
    shaper->pwl.rgb_resulted[109].blue_reg        = 4443;
    shaper->pwl.rgb_resulted[109].delta_red_reg   = 69;
    shaper->pwl.rgb_resulted[109].delta_green_reg = 69;
    shaper->pwl.rgb_resulted[109].delta_blue_reg  = 69;
    shaper->pwl.rgb_resulted[110].red_reg         = 4512;
    shaper->pwl.rgb_resulted[110].green_reg       = 4512;
    shaper->pwl.rgb_resulted[110].blue_reg        = 4512;
    shaper->pwl.rgb_resulted[110].delta_red_reg   = 67;
    shaper->pwl.rgb_resulted[110].delta_green_reg = 67;
    shaper->pwl.rgb_resulted[110].delta_blue_reg  = 67;
    shaper->pwl.rgb_resulted[111].red_reg         = 4579;
    shaper->pwl.rgb_resulted[111].green_reg       = 4579;
    shaper->pwl.rgb_resulted[111].blue_reg        = 4579;
    shaper->pwl.rgb_resulted[111].delta_red_reg   = 67;
    shaper->pwl.rgb_resulted[111].delta_green_reg = 67;
    shaper->pwl.rgb_resulted[111].delta_blue_reg  = 67;
    shaper->pwl.rgb_resulted[112].red_reg         = 4646;
    shaper->pwl.rgb_resulted[112].green_reg       = 4646;
    shaper->pwl.rgb_resulted[112].blue_reg        = 4646;
    shaper->pwl.rgb_resulted[112].delta_red_reg   = 130;
    shaper->pwl.rgb_resulted[112].delta_green_reg = 130;
    shaper->pwl.rgb_resulted[112].delta_blue_reg  = 130;
    shaper->pwl.rgb_resulted[113].red_reg         = 4776;
    shaper->pwl.rgb_resulted[113].green_reg       = 4776;
    shaper->pwl.rgb_resulted[113].blue_reg        = 4776;
    shaper->pwl.rgb_resulted[113].delta_red_reg   = 125;
    shaper->pwl.rgb_resulted[113].delta_green_reg = 125;
    shaper->pwl.rgb_resulted[113].delta_blue_reg  = 125;
    shaper->pwl.rgb_resulted[114].red_reg         = 4901;
    shaper->pwl.rgb_resulted[114].green_reg       = 4901;
    shaper->pwl.rgb_resulted[114].blue_reg        = 4901;
    shaper->pwl.rgb_resulted[114].delta_red_reg   = 122;
    shaper->pwl.rgb_resulted[114].delta_green_reg = 122;
    shaper->pwl.rgb_resulted[114].delta_blue_reg  = 122;
    shaper->pwl.rgb_resulted[115].red_reg         = 5023;
    shaper->pwl.rgb_resulted[115].green_reg       = 5023;
    shaper->pwl.rgb_resulted[115].blue_reg        = 5023;
    shaper->pwl.rgb_resulted[115].delta_red_reg   = 119;
    shaper->pwl.rgb_resulted[115].delta_green_reg = 119;
    shaper->pwl.rgb_resulted[115].delta_blue_reg  = 119;
    shaper->pwl.rgb_resulted[116].red_reg         = 5142;
    shaper->pwl.rgb_resulted[116].green_reg       = 5142;
    shaper->pwl.rgb_resulted[116].blue_reg        = 5142;
    shaper->pwl.rgb_resulted[116].delta_red_reg   = 115;
    shaper->pwl.rgb_resulted[116].delta_green_reg = 115;
    shaper->pwl.rgb_resulted[116].delta_blue_reg  = 115;
    shaper->pwl.rgb_resulted[117].red_reg         = 5257;
    shaper->pwl.rgb_resulted[117].green_reg       = 5257;
    shaper->pwl.rgb_resulted[117].blue_reg        = 5257;
    shaper->pwl.rgb_resulted[117].delta_red_reg   = 112;
    shaper->pwl.rgb_resulted[117].delta_green_reg = 112;
    shaper->pwl.rgb_resulted[117].delta_blue_reg  = 112;
    shaper->pwl.rgb_resulted[118].red_reg         = 5369;
    shaper->pwl.rgb_resulted[118].green_reg       = 5369;
    shaper->pwl.rgb_resulted[118].blue_reg        = 5369;
    shaper->pwl.rgb_resulted[118].delta_red_reg   = 110;
    shaper->pwl.rgb_resulted[118].delta_green_reg = 110;
    shaper->pwl.rgb_resulted[118].delta_blue_reg  = 110;
    shaper->pwl.rgb_resulted[119].red_reg         = 5479;
    shaper->pwl.rgb_resulted[119].green_reg       = 5479;
    shaper->pwl.rgb_resulted[119].blue_reg        = 5479;
    shaper->pwl.rgb_resulted[119].delta_red_reg   = 107;
    shaper->pwl.rgb_resulted[119].delta_green_reg = 107;
    shaper->pwl.rgb_resulted[119].delta_blue_reg  = 107;
    shaper->pwl.rgb_resulted[120].red_reg         = 5586;
    shaper->pwl.rgb_resulted[120].green_reg       = 5586;
    shaper->pwl.rgb_resulted[120].blue_reg        = 5586;
    shaper->pwl.rgb_resulted[120].delta_red_reg   = 105;
    shaper->pwl.rgb_resulted[120].delta_green_reg = 105;
    shaper->pwl.rgb_resulted[120].delta_blue_reg  = 105;
    shaper->pwl.rgb_resulted[121].red_reg         = 5691;
    shaper->pwl.rgb_resulted[121].green_reg       = 5691;
    shaper->pwl.rgb_resulted[121].blue_reg        = 5691;
    shaper->pwl.rgb_resulted[121].delta_red_reg   = 102;
    shaper->pwl.rgb_resulted[121].delta_green_reg = 102;
    shaper->pwl.rgb_resulted[121].delta_blue_reg  = 102;
    shaper->pwl.rgb_resulted[122].red_reg         = 5793;
    shaper->pwl.rgb_resulted[122].green_reg       = 5793;
    shaper->pwl.rgb_resulted[122].blue_reg        = 5793;
    shaper->pwl.rgb_resulted[122].delta_red_reg   = 100;
    shaper->pwl.rgb_resulted[122].delta_green_reg = 100;
    shaper->pwl.rgb_resulted[122].delta_blue_reg  = 100;
    shaper->pwl.rgb_resulted[123].red_reg         = 5893;
    shaper->pwl.rgb_resulted[123].green_reg       = 5893;
    shaper->pwl.rgb_resulted[123].blue_reg        = 5893;
    shaper->pwl.rgb_resulted[123].delta_red_reg   = 99;
    shaper->pwl.rgb_resulted[123].delta_green_reg = 99;
    shaper->pwl.rgb_resulted[123].delta_blue_reg  = 99;
    shaper->pwl.rgb_resulted[124].red_reg         = 5992;
    shaper->pwl.rgb_resulted[124].green_reg       = 5992;
    shaper->pwl.rgb_resulted[124].blue_reg        = 5992;
    shaper->pwl.rgb_resulted[124].delta_red_reg   = 96;
    shaper->pwl.rgb_resulted[124].delta_green_reg = 96;
    shaper->pwl.rgb_resulted[124].delta_blue_reg  = 96;
    shaper->pwl.rgb_resulted[125].red_reg         = 6088;
    shaper->pwl.rgb_resulted[125].green_reg       = 6088;
    shaper->pwl.rgb_resulted[125].blue_reg        = 6088;
    shaper->pwl.rgb_resulted[125].delta_red_reg   = 94;
    shaper->pwl.rgb_resulted[125].delta_green_reg = 94;
    shaper->pwl.rgb_resulted[125].delta_blue_reg  = 94;
    shaper->pwl.rgb_resulted[126].red_reg         = 6182;
    shaper->pwl.rgb_resulted[126].green_reg       = 6182;
    shaper->pwl.rgb_resulted[126].blue_reg        = 6182;
    shaper->pwl.rgb_resulted[126].delta_red_reg   = 93;
    shaper->pwl.rgb_resulted[126].delta_green_reg = 93;
    shaper->pwl.rgb_resulted[126].delta_blue_reg  = 93;
    shaper->pwl.rgb_resulted[127].red_reg         = 6275;
    shaper->pwl.rgb_resulted[127].green_reg       = 6275;
    shaper->pwl.rgb_resulted[127].blue_reg        = 6275;
    shaper->pwl.rgb_resulted[127].delta_red_reg   = 91;
    shaper->pwl.rgb_resulted[127].delta_green_reg = 91;
    shaper->pwl.rgb_resulted[127].delta_blue_reg  = 91;
    shaper->pwl.rgb_resulted[128].red_reg         = 6366;
    shaper->pwl.rgb_resulted[128].green_reg       = 6366;
    shaper->pwl.rgb_resulted[128].blue_reg        = 6366;
    shaper->pwl.rgb_resulted[128].delta_red_reg   = 90;
    shaper->pwl.rgb_resulted[128].delta_green_reg = 90;
    shaper->pwl.rgb_resulted[128].delta_blue_reg  = 90;
    shaper->pwl.rgb_resulted[129].red_reg         = 6456;
    shaper->pwl.rgb_resulted[129].green_reg       = 6456;
    shaper->pwl.rgb_resulted[129].blue_reg        = 6456;
    shaper->pwl.rgb_resulted[129].delta_red_reg   = 88;
    shaper->pwl.rgb_resulted[129].delta_green_reg = 88;
    shaper->pwl.rgb_resulted[129].delta_blue_reg  = 88;
    shaper->pwl.rgb_resulted[130].red_reg         = 6544;
    shaper->pwl.rgb_resulted[130].green_reg       = 6544;
    shaper->pwl.rgb_resulted[130].blue_reg        = 6544;
    shaper->pwl.rgb_resulted[130].delta_red_reg   = 87;
    shaper->pwl.rgb_resulted[130].delta_green_reg = 87;
    shaper->pwl.rgb_resulted[130].delta_blue_reg  = 87;
    shaper->pwl.rgb_resulted[131].red_reg         = 6631;
    shaper->pwl.rgb_resulted[131].green_reg       = 6631;
    shaper->pwl.rgb_resulted[131].blue_reg        = 6631;
    shaper->pwl.rgb_resulted[131].delta_red_reg   = 86;
    shaper->pwl.rgb_resulted[131].delta_green_reg = 86;
    shaper->pwl.rgb_resulted[131].delta_blue_reg  = 86;
    shaper->pwl.rgb_resulted[132].red_reg         = 6717;
    shaper->pwl.rgb_resulted[132].green_reg       = 6717;
    shaper->pwl.rgb_resulted[132].blue_reg        = 6717;
    shaper->pwl.rgb_resulted[132].delta_red_reg   = 84;
    shaper->pwl.rgb_resulted[132].delta_green_reg = 84;
    shaper->pwl.rgb_resulted[132].delta_blue_reg  = 84;
    shaper->pwl.rgb_resulted[133].red_reg         = 6801;
    shaper->pwl.rgb_resulted[133].green_reg       = 6801;
    shaper->pwl.rgb_resulted[133].blue_reg        = 6801;
    shaper->pwl.rgb_resulted[133].delta_red_reg   = 83;
    shaper->pwl.rgb_resulted[133].delta_green_reg = 83;
    shaper->pwl.rgb_resulted[133].delta_blue_reg  = 83;
    shaper->pwl.rgb_resulted[134].red_reg         = 6884;
    shaper->pwl.rgb_resulted[134].green_reg       = 6884;
    shaper->pwl.rgb_resulted[134].blue_reg        = 6884;
    shaper->pwl.rgb_resulted[134].delta_red_reg   = 81;
    shaper->pwl.rgb_resulted[134].delta_green_reg = 81;
    shaper->pwl.rgb_resulted[134].delta_blue_reg  = 81;
    shaper->pwl.rgb_resulted[135].red_reg         = 6965;
    shaper->pwl.rgb_resulted[135].green_reg       = 6965;
    shaper->pwl.rgb_resulted[135].blue_reg        = 6965;
    shaper->pwl.rgb_resulted[135].delta_red_reg   = 81;
    shaper->pwl.rgb_resulted[135].delta_green_reg = 81;
    shaper->pwl.rgb_resulted[135].delta_blue_reg  = 81;
    shaper->pwl.rgb_resulted[136].red_reg         = 7046;
    shaper->pwl.rgb_resulted[136].green_reg       = 7046;
    shaper->pwl.rgb_resulted[136].blue_reg        = 7046;
    shaper->pwl.rgb_resulted[136].delta_red_reg   = 80;
    shaper->pwl.rgb_resulted[136].delta_green_reg = 80;
    shaper->pwl.rgb_resulted[136].delta_blue_reg  = 80;
    shaper->pwl.rgb_resulted[137].red_reg         = 7126;
    shaper->pwl.rgb_resulted[137].green_reg       = 7126;
    shaper->pwl.rgb_resulted[137].blue_reg        = 7126;
    shaper->pwl.rgb_resulted[137].delta_red_reg   = 78;
    shaper->pwl.rgb_resulted[137].delta_green_reg = 78;
    shaper->pwl.rgb_resulted[137].delta_blue_reg  = 78;
    shaper->pwl.rgb_resulted[138].red_reg         = 7204;
    shaper->pwl.rgb_resulted[138].green_reg       = 7204;
    shaper->pwl.rgb_resulted[138].blue_reg        = 7204;
    shaper->pwl.rgb_resulted[138].delta_red_reg   = 78;
    shaper->pwl.rgb_resulted[138].delta_green_reg = 78;
    shaper->pwl.rgb_resulted[138].delta_blue_reg  = 78;
    shaper->pwl.rgb_resulted[139].red_reg         = 7282;
    shaper->pwl.rgb_resulted[139].green_reg       = 7282;
    shaper->pwl.rgb_resulted[139].blue_reg        = 7282;
    shaper->pwl.rgb_resulted[139].delta_red_reg   = 76;
    shaper->pwl.rgb_resulted[139].delta_green_reg = 76;
    shaper->pwl.rgb_resulted[139].delta_blue_reg  = 76;
    shaper->pwl.rgb_resulted[140].red_reg         = 7358;
    shaper->pwl.rgb_resulted[140].green_reg       = 7358;
    shaper->pwl.rgb_resulted[140].blue_reg        = 7358;
    shaper->pwl.rgb_resulted[140].delta_red_reg   = 76;
    shaper->pwl.rgb_resulted[140].delta_green_reg = 76;
    shaper->pwl.rgb_resulted[140].delta_blue_reg  = 76;
    shaper->pwl.rgb_resulted[141].red_reg         = 7434;
    shaper->pwl.rgb_resulted[141].green_reg       = 7434;
    shaper->pwl.rgb_resulted[141].blue_reg        = 7434;
    shaper->pwl.rgb_resulted[141].delta_red_reg   = 74;
    shaper->pwl.rgb_resulted[141].delta_green_reg = 74;
    shaper->pwl.rgb_resulted[141].delta_blue_reg  = 74;
    shaper->pwl.rgb_resulted[142].red_reg         = 7508;
    shaper->pwl.rgb_resulted[142].green_reg       = 7508;
    shaper->pwl.rgb_resulted[142].blue_reg        = 7508;
    shaper->pwl.rgb_resulted[142].delta_red_reg   = 74;
    shaper->pwl.rgb_resulted[142].delta_green_reg = 74;
    shaper->pwl.rgb_resulted[142].delta_blue_reg  = 74;
    shaper->pwl.rgb_resulted[143].red_reg         = 7582;
    shaper->pwl.rgb_resulted[143].green_reg       = 7582;
    shaper->pwl.rgb_resulted[143].blue_reg        = 7582;
    shaper->pwl.rgb_resulted[143].delta_red_reg   = 73;
    shaper->pwl.rgb_resulted[143].delta_green_reg = 73;
    shaper->pwl.rgb_resulted[143].delta_blue_reg  = 73;
    shaper->pwl.rgb_resulted[144].red_reg         = 7655;
    shaper->pwl.rgb_resulted[144].green_reg       = 7655;
    shaper->pwl.rgb_resulted[144].blue_reg        = 7655;
    shaper->pwl.rgb_resulted[144].delta_red_reg   = 72;
    shaper->pwl.rgb_resulted[144].delta_green_reg = 72;
    shaper->pwl.rgb_resulted[144].delta_blue_reg  = 72;
    shaper->pwl.rgb_resulted[145].red_reg         = 7727;
    shaper->pwl.rgb_resulted[145].green_reg       = 7727;
    shaper->pwl.rgb_resulted[145].blue_reg        = 7727;
    shaper->pwl.rgb_resulted[145].delta_red_reg   = 71;
    shaper->pwl.rgb_resulted[145].delta_green_reg = 71;
    shaper->pwl.rgb_resulted[145].delta_blue_reg  = 71;
    shaper->pwl.rgb_resulted[146].red_reg         = 7798;
    shaper->pwl.rgb_resulted[146].green_reg       = 7798;
    shaper->pwl.rgb_resulted[146].blue_reg        = 7798;
    shaper->pwl.rgb_resulted[146].delta_red_reg   = 71;
    shaper->pwl.rgb_resulted[146].delta_green_reg = 71;
    shaper->pwl.rgb_resulted[146].delta_blue_reg  = 71;
    shaper->pwl.rgb_resulted[147].red_reg         = 7869;
    shaper->pwl.rgb_resulted[147].green_reg       = 7869;
    shaper->pwl.rgb_resulted[147].blue_reg        = 7869;
    shaper->pwl.rgb_resulted[147].delta_red_reg   = 70;
    shaper->pwl.rgb_resulted[147].delta_green_reg = 70;
    shaper->pwl.rgb_resulted[147].delta_blue_reg  = 70;
    shaper->pwl.rgb_resulted[148].red_reg         = 7939;
    shaper->pwl.rgb_resulted[148].green_reg       = 7939;
    shaper->pwl.rgb_resulted[148].blue_reg        = 7939;
    shaper->pwl.rgb_resulted[148].delta_red_reg   = 69;
    shaper->pwl.rgb_resulted[148].delta_green_reg = 69;
    shaper->pwl.rgb_resulted[148].delta_blue_reg  = 69;
    shaper->pwl.rgb_resulted[149].red_reg         = 8008;
    shaper->pwl.rgb_resulted[149].green_reg       = 8008;
    shaper->pwl.rgb_resulted[149].blue_reg        = 8008;
    shaper->pwl.rgb_resulted[149].delta_red_reg   = 68;
    shaper->pwl.rgb_resulted[149].delta_green_reg = 68;
    shaper->pwl.rgb_resulted[149].delta_blue_reg  = 68;
    shaper->pwl.rgb_resulted[150].red_reg         = 8076;
    shaper->pwl.rgb_resulted[150].green_reg       = 8076;
    shaper->pwl.rgb_resulted[150].blue_reg        = 8076;
    shaper->pwl.rgb_resulted[150].delta_red_reg   = 68;
    shaper->pwl.rgb_resulted[150].delta_green_reg = 68;
    shaper->pwl.rgb_resulted[150].delta_blue_reg  = 68;
    shaper->pwl.rgb_resulted[151].red_reg         = 8144;
    shaper->pwl.rgb_resulted[151].green_reg       = 8144;
    shaper->pwl.rgb_resulted[151].blue_reg        = 8144;
    shaper->pwl.rgb_resulted[151].delta_red_reg   = 67;
    shaper->pwl.rgb_resulted[151].delta_green_reg = 67;
    shaper->pwl.rgb_resulted[151].delta_blue_reg  = 67;
    shaper->pwl.rgb_resulted[152].red_reg         = 8211;
    shaper->pwl.rgb_resulted[152].green_reg       = 8211;
    shaper->pwl.rgb_resulted[152].blue_reg        = 8211;
    shaper->pwl.rgb_resulted[152].delta_red_reg   = 66;
    shaper->pwl.rgb_resulted[152].delta_green_reg = 66;
    shaper->pwl.rgb_resulted[152].delta_blue_reg  = 66;
    shaper->pwl.rgb_resulted[153].red_reg         = 8277;
    shaper->pwl.rgb_resulted[153].green_reg       = 8277;
    shaper->pwl.rgb_resulted[153].blue_reg        = 8277;
    shaper->pwl.rgb_resulted[153].delta_red_reg   = 66;
    shaper->pwl.rgb_resulted[153].delta_green_reg = 66;
    shaper->pwl.rgb_resulted[153].delta_blue_reg  = 66;
    shaper->pwl.rgb_resulted[154].red_reg         = 8343;
    shaper->pwl.rgb_resulted[154].green_reg       = 8343;
    shaper->pwl.rgb_resulted[154].blue_reg        = 8343;
    shaper->pwl.rgb_resulted[154].delta_red_reg   = 65;
    shaper->pwl.rgb_resulted[154].delta_green_reg = 65;
    shaper->pwl.rgb_resulted[154].delta_blue_reg  = 65;
    shaper->pwl.rgb_resulted[155].red_reg         = 8408;
    shaper->pwl.rgb_resulted[155].green_reg       = 8408;
    shaper->pwl.rgb_resulted[155].blue_reg        = 8408;
    shaper->pwl.rgb_resulted[155].delta_red_reg   = 64;
    shaper->pwl.rgb_resulted[155].delta_green_reg = 64;
    shaper->pwl.rgb_resulted[155].delta_blue_reg  = 64;
    shaper->pwl.rgb_resulted[156].red_reg         = 8472;
    shaper->pwl.rgb_resulted[156].green_reg       = 8472;
    shaper->pwl.rgb_resulted[156].blue_reg        = 8472;
    shaper->pwl.rgb_resulted[156].delta_red_reg   = 64;
    shaper->pwl.rgb_resulted[156].delta_green_reg = 64;
    shaper->pwl.rgb_resulted[156].delta_blue_reg  = 64;
    shaper->pwl.rgb_resulted[157].red_reg         = 8536;
    shaper->pwl.rgb_resulted[157].green_reg       = 8536;
    shaper->pwl.rgb_resulted[157].blue_reg        = 8536;
    shaper->pwl.rgb_resulted[157].delta_red_reg   = 63;
    shaper->pwl.rgb_resulted[157].delta_green_reg = 63;
    shaper->pwl.rgb_resulted[157].delta_blue_reg  = 63;
    shaper->pwl.rgb_resulted[158].red_reg         = 8599;
    shaper->pwl.rgb_resulted[158].green_reg       = 8599;
    shaper->pwl.rgb_resulted[158].blue_reg        = 8599;
    shaper->pwl.rgb_resulted[158].delta_red_reg   = 63;
    shaper->pwl.rgb_resulted[158].delta_green_reg = 63;
    shaper->pwl.rgb_resulted[158].delta_blue_reg  = 63;
    shaper->pwl.rgb_resulted[159].red_reg         = 8662;
    shaper->pwl.rgb_resulted[159].green_reg       = 8662;
    shaper->pwl.rgb_resulted[159].blue_reg        = 8662;
    shaper->pwl.rgb_resulted[159].delta_red_reg   = 62;
    shaper->pwl.rgb_resulted[159].delta_green_reg = 62;
    shaper->pwl.rgb_resulted[159].delta_blue_reg  = 62;
    shaper->pwl.rgb_resulted[160].red_reg         = 8724;
    shaper->pwl.rgb_resulted[160].green_reg       = 8724;
    shaper->pwl.rgb_resulted[160].blue_reg        = 8724;
    shaper->pwl.rgb_resulted[160].delta_red_reg   = 123;
    shaper->pwl.rgb_resulted[160].delta_green_reg = 123;
    shaper->pwl.rgb_resulted[160].delta_blue_reg  = 123;
    shaper->pwl.rgb_resulted[161].red_reg         = 8847;
    shaper->pwl.rgb_resulted[161].green_reg       = 8847;
    shaper->pwl.rgb_resulted[161].blue_reg        = 8847;
    shaper->pwl.rgb_resulted[161].delta_red_reg   = 121;
    shaper->pwl.rgb_resulted[161].delta_green_reg = 121;
    shaper->pwl.rgb_resulted[161].delta_blue_reg  = 121;
    shaper->pwl.rgb_resulted[162].red_reg         = 8968;
    shaper->pwl.rgb_resulted[162].green_reg       = 8968;
    shaper->pwl.rgb_resulted[162].blue_reg        = 8968;
    shaper->pwl.rgb_resulted[162].delta_red_reg   = 119;
    shaper->pwl.rgb_resulted[162].delta_green_reg = 119;
    shaper->pwl.rgb_resulted[162].delta_blue_reg  = 119;
    shaper->pwl.rgb_resulted[163].red_reg         = 9087;
    shaper->pwl.rgb_resulted[163].green_reg       = 9087;
    shaper->pwl.rgb_resulted[163].blue_reg        = 9087;
    shaper->pwl.rgb_resulted[163].delta_red_reg   = 117;
    shaper->pwl.rgb_resulted[163].delta_green_reg = 117;
    shaper->pwl.rgb_resulted[163].delta_blue_reg  = 117;
    shaper->pwl.rgb_resulted[164].red_reg         = 9204;
    shaper->pwl.rgb_resulted[164].green_reg       = 9204;
    shaper->pwl.rgb_resulted[164].blue_reg        = 9204;
    shaper->pwl.rgb_resulted[164].delta_red_reg   = 115;
    shaper->pwl.rgb_resulted[164].delta_green_reg = 115;
    shaper->pwl.rgb_resulted[164].delta_blue_reg  = 115;
    shaper->pwl.rgb_resulted[165].red_reg         = 9319;
    shaper->pwl.rgb_resulted[165].green_reg       = 9319;
    shaper->pwl.rgb_resulted[165].blue_reg        = 9319;
    shaper->pwl.rgb_resulted[165].delta_red_reg   = 114;
    shaper->pwl.rgb_resulted[165].delta_green_reg = 114;
    shaper->pwl.rgb_resulted[165].delta_blue_reg  = 114;
    shaper->pwl.rgb_resulted[166].red_reg         = 9433;
    shaper->pwl.rgb_resulted[166].green_reg       = 9433;
    shaper->pwl.rgb_resulted[166].blue_reg        = 9433;
    shaper->pwl.rgb_resulted[166].delta_red_reg   = 112;
    shaper->pwl.rgb_resulted[166].delta_green_reg = 112;
    shaper->pwl.rgb_resulted[166].delta_blue_reg  = 112;
    shaper->pwl.rgb_resulted[167].red_reg         = 9545;
    shaper->pwl.rgb_resulted[167].green_reg       = 9545;
    shaper->pwl.rgb_resulted[167].blue_reg        = 9545;
    shaper->pwl.rgb_resulted[167].delta_red_reg   = 111;
    shaper->pwl.rgb_resulted[167].delta_green_reg = 111;
    shaper->pwl.rgb_resulted[167].delta_blue_reg  = 111;
    shaper->pwl.rgb_resulted[168].red_reg         = 9656;
    shaper->pwl.rgb_resulted[168].green_reg       = 9656;
    shaper->pwl.rgb_resulted[168].blue_reg        = 9656;
    shaper->pwl.rgb_resulted[168].delta_red_reg   = 109;
    shaper->pwl.rgb_resulted[168].delta_green_reg = 109;
    shaper->pwl.rgb_resulted[168].delta_blue_reg  = 109;
    shaper->pwl.rgb_resulted[169].red_reg         = 9765;
    shaper->pwl.rgb_resulted[169].green_reg       = 9765;
    shaper->pwl.rgb_resulted[169].blue_reg        = 9765;
    shaper->pwl.rgb_resulted[169].delta_red_reg   = 107;
    shaper->pwl.rgb_resulted[169].delta_green_reg = 107;
    shaper->pwl.rgb_resulted[169].delta_blue_reg  = 107;
    shaper->pwl.rgb_resulted[170].red_reg         = 9872;
    shaper->pwl.rgb_resulted[170].green_reg       = 9872;
    shaper->pwl.rgb_resulted[170].blue_reg        = 9872;
    shaper->pwl.rgb_resulted[170].delta_red_reg   = 106;
    shaper->pwl.rgb_resulted[170].delta_green_reg = 106;
    shaper->pwl.rgb_resulted[170].delta_blue_reg  = 106;
    shaper->pwl.rgb_resulted[171].red_reg         = 9978;
    shaper->pwl.rgb_resulted[171].green_reg       = 9978;
    shaper->pwl.rgb_resulted[171].blue_reg        = 9978;
    shaper->pwl.rgb_resulted[171].delta_red_reg   = 105;
    shaper->pwl.rgb_resulted[171].delta_green_reg = 105;
    shaper->pwl.rgb_resulted[171].delta_blue_reg  = 105;
    shaper->pwl.rgb_resulted[172].red_reg         = 10083;
    shaper->pwl.rgb_resulted[172].green_reg       = 10083;
    shaper->pwl.rgb_resulted[172].blue_reg        = 10083;
    shaper->pwl.rgb_resulted[172].delta_red_reg   = 104;
    shaper->pwl.rgb_resulted[172].delta_green_reg = 104;
    shaper->pwl.rgb_resulted[172].delta_blue_reg  = 104;
    shaper->pwl.rgb_resulted[173].red_reg         = 10187;
    shaper->pwl.rgb_resulted[173].green_reg       = 10187;
    shaper->pwl.rgb_resulted[173].blue_reg        = 10187;
    shaper->pwl.rgb_resulted[173].delta_red_reg   = 102;
    shaper->pwl.rgb_resulted[173].delta_green_reg = 102;
    shaper->pwl.rgb_resulted[173].delta_blue_reg  = 102;
    shaper->pwl.rgb_resulted[174].red_reg         = 10289;
    shaper->pwl.rgb_resulted[174].green_reg       = 10289;
    shaper->pwl.rgb_resulted[174].blue_reg        = 10289;
    shaper->pwl.rgb_resulted[174].delta_red_reg   = 101;
    shaper->pwl.rgb_resulted[174].delta_green_reg = 101;
    shaper->pwl.rgb_resulted[174].delta_blue_reg  = 101;
    shaper->pwl.rgb_resulted[175].red_reg         = 10390;
    shaper->pwl.rgb_resulted[175].green_reg       = 10390;
    shaper->pwl.rgb_resulted[175].blue_reg        = 10390;
    shaper->pwl.rgb_resulted[175].delta_red_reg   = 100;
    shaper->pwl.rgb_resulted[175].delta_green_reg = 100;
    shaper->pwl.rgb_resulted[175].delta_blue_reg  = 100;
    shaper->pwl.rgb_resulted[176].red_reg         = 10490;
    shaper->pwl.rgb_resulted[176].green_reg       = 10490;
    shaper->pwl.rgb_resulted[176].blue_reg        = 10490;
    shaper->pwl.rgb_resulted[176].delta_red_reg   = 99;
    shaper->pwl.rgb_resulted[176].delta_green_reg = 99;
    shaper->pwl.rgb_resulted[176].delta_blue_reg  = 99;
    shaper->pwl.rgb_resulted[177].red_reg         = 10589;
    shaper->pwl.rgb_resulted[177].green_reg       = 10589;
    shaper->pwl.rgb_resulted[177].blue_reg        = 10589;
    shaper->pwl.rgb_resulted[177].delta_red_reg   = 97;
    shaper->pwl.rgb_resulted[177].delta_green_reg = 97;
    shaper->pwl.rgb_resulted[177].delta_blue_reg  = 97;
    shaper->pwl.rgb_resulted[178].red_reg         = 10686;
    shaper->pwl.rgb_resulted[178].green_reg       = 10686;
    shaper->pwl.rgb_resulted[178].blue_reg        = 10686;
    shaper->pwl.rgb_resulted[178].delta_red_reg   = 97;
    shaper->pwl.rgb_resulted[178].delta_green_reg = 97;
    shaper->pwl.rgb_resulted[178].delta_blue_reg  = 97;
    shaper->pwl.rgb_resulted[179].red_reg         = 10783;
    shaper->pwl.rgb_resulted[179].green_reg       = 10783;
    shaper->pwl.rgb_resulted[179].blue_reg        = 10783;
    shaper->pwl.rgb_resulted[179].delta_red_reg   = 96;
    shaper->pwl.rgb_resulted[179].delta_green_reg = 96;
    shaper->pwl.rgb_resulted[179].delta_blue_reg  = 96;
    shaper->pwl.rgb_resulted[180].red_reg         = 10879;
    shaper->pwl.rgb_resulted[180].green_reg       = 10879;
    shaper->pwl.rgb_resulted[180].blue_reg        = 10879;
    shaper->pwl.rgb_resulted[180].delta_red_reg   = 94;
    shaper->pwl.rgb_resulted[180].delta_green_reg = 94;
    shaper->pwl.rgb_resulted[180].delta_blue_reg  = 94;
    shaper->pwl.rgb_resulted[181].red_reg         = 10973;
    shaper->pwl.rgb_resulted[181].green_reg       = 10973;
    shaper->pwl.rgb_resulted[181].blue_reg        = 10973;
    shaper->pwl.rgb_resulted[181].delta_red_reg   = 94;
    shaper->pwl.rgb_resulted[181].delta_green_reg = 94;
    shaper->pwl.rgb_resulted[181].delta_blue_reg  = 94;
    shaper->pwl.rgb_resulted[182].red_reg         = 11067;
    shaper->pwl.rgb_resulted[182].green_reg       = 11067;
    shaper->pwl.rgb_resulted[182].blue_reg        = 11067;
    shaper->pwl.rgb_resulted[182].delta_red_reg   = 92;
    shaper->pwl.rgb_resulted[182].delta_green_reg = 92;
    shaper->pwl.rgb_resulted[182].delta_blue_reg  = 92;
    shaper->pwl.rgb_resulted[183].red_reg         = 11159;
    shaper->pwl.rgb_resulted[183].green_reg       = 11159;
    shaper->pwl.rgb_resulted[183].blue_reg        = 11159;
    shaper->pwl.rgb_resulted[183].delta_red_reg   = 92;
    shaper->pwl.rgb_resulted[183].delta_green_reg = 92;
    shaper->pwl.rgb_resulted[183].delta_blue_reg  = 92;
    shaper->pwl.rgb_resulted[184].red_reg         = 11251;
    shaper->pwl.rgb_resulted[184].green_reg       = 11251;
    shaper->pwl.rgb_resulted[184].blue_reg        = 11251;
    shaper->pwl.rgb_resulted[184].delta_red_reg   = 91;
    shaper->pwl.rgb_resulted[184].delta_green_reg = 91;
    shaper->pwl.rgb_resulted[184].delta_blue_reg  = 91;
    shaper->pwl.rgb_resulted[185].red_reg         = 11342;
    shaper->pwl.rgb_resulted[185].green_reg       = 11342;
    shaper->pwl.rgb_resulted[185].blue_reg        = 11342;
    shaper->pwl.rgb_resulted[185].delta_red_reg   = 90;
    shaper->pwl.rgb_resulted[185].delta_green_reg = 90;
    shaper->pwl.rgb_resulted[185].delta_blue_reg  = 90;
    shaper->pwl.rgb_resulted[186].red_reg         = 11432;
    shaper->pwl.rgb_resulted[186].green_reg       = 11432;
    shaper->pwl.rgb_resulted[186].blue_reg        = 11432;
    shaper->pwl.rgb_resulted[186].delta_red_reg   = 89;
    shaper->pwl.rgb_resulted[186].delta_green_reg = 89;
    shaper->pwl.rgb_resulted[186].delta_blue_reg  = 89;
    shaper->pwl.rgb_resulted[187].red_reg         = 11521;
    shaper->pwl.rgb_resulted[187].green_reg       = 11521;
    shaper->pwl.rgb_resulted[187].blue_reg        = 11521;
    shaper->pwl.rgb_resulted[187].delta_red_reg   = 89;
    shaper->pwl.rgb_resulted[187].delta_green_reg = 89;
    shaper->pwl.rgb_resulted[187].delta_blue_reg  = 89;
    shaper->pwl.rgb_resulted[188].red_reg         = 11610;
    shaper->pwl.rgb_resulted[188].green_reg       = 11610;
    shaper->pwl.rgb_resulted[188].blue_reg        = 11610;
    shaper->pwl.rgb_resulted[188].delta_red_reg   = 87;
    shaper->pwl.rgb_resulted[188].delta_green_reg = 87;
    shaper->pwl.rgb_resulted[188].delta_blue_reg  = 87;
    shaper->pwl.rgb_resulted[189].red_reg         = 11697;
    shaper->pwl.rgb_resulted[189].green_reg       = 11697;
    shaper->pwl.rgb_resulted[189].blue_reg        = 11697;
    shaper->pwl.rgb_resulted[189].delta_red_reg   = 87;
    shaper->pwl.rgb_resulted[189].delta_green_reg = 87;
    shaper->pwl.rgb_resulted[189].delta_blue_reg  = 87;
    shaper->pwl.rgb_resulted[190].red_reg         = 11784;
    shaper->pwl.rgb_resulted[190].green_reg       = 11784;
    shaper->pwl.rgb_resulted[190].blue_reg        = 11784;
    shaper->pwl.rgb_resulted[190].delta_red_reg   = 86;
    shaper->pwl.rgb_resulted[190].delta_green_reg = 86;
    shaper->pwl.rgb_resulted[190].delta_blue_reg  = 86;
    shaper->pwl.rgb_resulted[191].red_reg         = 11870;
    shaper->pwl.rgb_resulted[191].green_reg       = 11870;
    shaper->pwl.rgb_resulted[191].blue_reg        = 11870;
    shaper->pwl.rgb_resulted[191].delta_red_reg   = 85;
    shaper->pwl.rgb_resulted[191].delta_green_reg = 85;
    shaper->pwl.rgb_resulted[191].delta_blue_reg  = 85;
    shaper->pwl.rgb_resulted[192].red_reg         = 11955;
    shaper->pwl.rgb_resulted[192].green_reg       = 11955;
    shaper->pwl.rgb_resulted[192].blue_reg        = 11955;
    shaper->pwl.rgb_resulted[192].delta_red_reg   = 169;
    shaper->pwl.rgb_resulted[192].delta_green_reg = 169;
    shaper->pwl.rgb_resulted[192].delta_blue_reg  = 169;
    shaper->pwl.rgb_resulted[193].red_reg         = 12124;
    shaper->pwl.rgb_resulted[193].green_reg       = 12124;
    shaper->pwl.rgb_resulted[193].blue_reg        = 12124;
    shaper->pwl.rgb_resulted[193].delta_red_reg   = 165;
    shaper->pwl.rgb_resulted[193].delta_green_reg = 165;
    shaper->pwl.rgb_resulted[193].delta_blue_reg  = 165;
    shaper->pwl.rgb_resulted[194].red_reg         = 12289;
    shaper->pwl.rgb_resulted[194].green_reg       = 12289;
    shaper->pwl.rgb_resulted[194].blue_reg        = 12289;
    shaper->pwl.rgb_resulted[194].delta_red_reg   = 163;
    shaper->pwl.rgb_resulted[194].delta_green_reg = 163;
    shaper->pwl.rgb_resulted[194].delta_blue_reg  = 163;
    shaper->pwl.rgb_resulted[195].red_reg         = 12452;
    shaper->pwl.rgb_resulted[195].green_reg       = 12452;
    shaper->pwl.rgb_resulted[195].blue_reg        = 12452;
    shaper->pwl.rgb_resulted[195].delta_red_reg   = 161;
    shaper->pwl.rgb_resulted[195].delta_green_reg = 161;
    shaper->pwl.rgb_resulted[195].delta_blue_reg  = 161;
    shaper->pwl.rgb_resulted[196].red_reg         = 12613;
    shaper->pwl.rgb_resulted[196].green_reg       = 12613;
    shaper->pwl.rgb_resulted[196].blue_reg        = 12613;
    shaper->pwl.rgb_resulted[196].delta_red_reg   = 158;
    shaper->pwl.rgb_resulted[196].delta_green_reg = 158;
    shaper->pwl.rgb_resulted[196].delta_blue_reg  = 158;
    shaper->pwl.rgb_resulted[197].red_reg         = 12771;
    shaper->pwl.rgb_resulted[197].green_reg       = 12771;
    shaper->pwl.rgb_resulted[197].blue_reg        = 12771;
    shaper->pwl.rgb_resulted[197].delta_red_reg   = 156;
    shaper->pwl.rgb_resulted[197].delta_green_reg = 156;
    shaper->pwl.rgb_resulted[197].delta_blue_reg  = 156;
    shaper->pwl.rgb_resulted[198].red_reg         = 12927;
    shaper->pwl.rgb_resulted[198].green_reg       = 12927;
    shaper->pwl.rgb_resulted[198].blue_reg        = 12927;
    shaper->pwl.rgb_resulted[198].delta_red_reg   = 153;
    shaper->pwl.rgb_resulted[198].delta_green_reg = 153;
    shaper->pwl.rgb_resulted[198].delta_blue_reg  = 153;
    shaper->pwl.rgb_resulted[199].red_reg         = 13080;
    shaper->pwl.rgb_resulted[199].green_reg       = 13080;
    shaper->pwl.rgb_resulted[199].blue_reg        = 13080;
    shaper->pwl.rgb_resulted[199].delta_red_reg   = 152;
    shaper->pwl.rgb_resulted[199].delta_green_reg = 152;
    shaper->pwl.rgb_resulted[199].delta_blue_reg  = 152;
    shaper->pwl.rgb_resulted[200].red_reg         = 13232;
    shaper->pwl.rgb_resulted[200].green_reg       = 13232;
    shaper->pwl.rgb_resulted[200].blue_reg        = 13232;
    shaper->pwl.rgb_resulted[200].delta_red_reg   = 149;
    shaper->pwl.rgb_resulted[200].delta_green_reg = 149;
    shaper->pwl.rgb_resulted[200].delta_blue_reg  = 149;
    shaper->pwl.rgb_resulted[201].red_reg         = 13381;
    shaper->pwl.rgb_resulted[201].green_reg       = 13381;
    shaper->pwl.rgb_resulted[201].blue_reg        = 13381;
    shaper->pwl.rgb_resulted[201].delta_red_reg   = 147;
    shaper->pwl.rgb_resulted[201].delta_green_reg = 147;
    shaper->pwl.rgb_resulted[201].delta_blue_reg  = 147;
    shaper->pwl.rgb_resulted[202].red_reg         = 13528;
    shaper->pwl.rgb_resulted[202].green_reg       = 13528;
    shaper->pwl.rgb_resulted[202].blue_reg        = 13528;
    shaper->pwl.rgb_resulted[202].delta_red_reg   = 146;
    shaper->pwl.rgb_resulted[202].delta_green_reg = 146;
    shaper->pwl.rgb_resulted[202].delta_blue_reg  = 146;
    shaper->pwl.rgb_resulted[203].red_reg         = 13674;
    shaper->pwl.rgb_resulted[203].green_reg       = 13674;
    shaper->pwl.rgb_resulted[203].blue_reg        = 13674;
    shaper->pwl.rgb_resulted[203].delta_red_reg   = 143;
    shaper->pwl.rgb_resulted[203].delta_green_reg = 143;
    shaper->pwl.rgb_resulted[203].delta_blue_reg  = 143;
    shaper->pwl.rgb_resulted[204].red_reg         = 13817;
    shaper->pwl.rgb_resulted[204].green_reg       = 13817;
    shaper->pwl.rgb_resulted[204].blue_reg        = 13817;
    shaper->pwl.rgb_resulted[204].delta_red_reg   = 142;
    shaper->pwl.rgb_resulted[204].delta_green_reg = 142;
    shaper->pwl.rgb_resulted[204].delta_blue_reg  = 142;
    shaper->pwl.rgb_resulted[205].red_reg         = 13959;
    shaper->pwl.rgb_resulted[205].green_reg       = 13959;
    shaper->pwl.rgb_resulted[205].blue_reg        = 13959;
    shaper->pwl.rgb_resulted[205].delta_red_reg   = 140;
    shaper->pwl.rgb_resulted[205].delta_green_reg = 140;
    shaper->pwl.rgb_resulted[205].delta_blue_reg  = 140;
    shaper->pwl.rgb_resulted[206].red_reg         = 14099;
    shaper->pwl.rgb_resulted[206].green_reg       = 14099;
    shaper->pwl.rgb_resulted[206].blue_reg        = 14099;
    shaper->pwl.rgb_resulted[206].delta_red_reg   = 139;
    shaper->pwl.rgb_resulted[206].delta_green_reg = 139;
    shaper->pwl.rgb_resulted[206].delta_blue_reg  = 139;
    shaper->pwl.rgb_resulted[207].red_reg         = 14238;
    shaper->pwl.rgb_resulted[207].green_reg       = 14238;
    shaper->pwl.rgb_resulted[207].blue_reg        = 14238;
    shaper->pwl.rgb_resulted[207].delta_red_reg   = 137;
    shaper->pwl.rgb_resulted[207].delta_green_reg = 137;
    shaper->pwl.rgb_resulted[207].delta_blue_reg  = 137;
    shaper->pwl.rgb_resulted[208].red_reg         = 14375;
    shaper->pwl.rgb_resulted[208].green_reg       = 14375;
    shaper->pwl.rgb_resulted[208].blue_reg        = 14375;
    shaper->pwl.rgb_resulted[208].delta_red_reg   = 135;
    shaper->pwl.rgb_resulted[208].delta_green_reg = 135;
    shaper->pwl.rgb_resulted[208].delta_blue_reg  = 135;
    shaper->pwl.rgb_resulted[209].red_reg         = 14510;
    shaper->pwl.rgb_resulted[209].green_reg       = 14510;
    shaper->pwl.rgb_resulted[209].blue_reg        = 14510;
    shaper->pwl.rgb_resulted[209].delta_red_reg   = 134;
    shaper->pwl.rgb_resulted[209].delta_green_reg = 134;
    shaper->pwl.rgb_resulted[209].delta_blue_reg  = 134;
    shaper->pwl.rgb_resulted[210].red_reg         = 14644;
    shaper->pwl.rgb_resulted[210].green_reg       = 14644;
    shaper->pwl.rgb_resulted[210].blue_reg        = 14644;
    shaper->pwl.rgb_resulted[210].delta_red_reg   = 132;
    shaper->pwl.rgb_resulted[210].delta_green_reg = 132;
    shaper->pwl.rgb_resulted[210].delta_blue_reg  = 132;
    shaper->pwl.rgb_resulted[211].red_reg         = 14776;
    shaper->pwl.rgb_resulted[211].green_reg       = 14776;
    shaper->pwl.rgb_resulted[211].blue_reg        = 14776;
    shaper->pwl.rgb_resulted[211].delta_red_reg   = 131;
    shaper->pwl.rgb_resulted[211].delta_green_reg = 131;
    shaper->pwl.rgb_resulted[211].delta_blue_reg  = 131;
    shaper->pwl.rgb_resulted[212].red_reg         = 14907;
    shaper->pwl.rgb_resulted[212].green_reg       = 14907;
    shaper->pwl.rgb_resulted[212].blue_reg        = 14907;
    shaper->pwl.rgb_resulted[212].delta_red_reg   = 130;
    shaper->pwl.rgb_resulted[212].delta_green_reg = 130;
    shaper->pwl.rgb_resulted[212].delta_blue_reg  = 130;
    shaper->pwl.rgb_resulted[213].red_reg         = 15037;
    shaper->pwl.rgb_resulted[213].green_reg       = 15037;
    shaper->pwl.rgb_resulted[213].blue_reg        = 15037;
    shaper->pwl.rgb_resulted[213].delta_red_reg   = 128;
    shaper->pwl.rgb_resulted[213].delta_green_reg = 128;
    shaper->pwl.rgb_resulted[213].delta_blue_reg  = 128;
    shaper->pwl.rgb_resulted[214].red_reg         = 15165;
    shaper->pwl.rgb_resulted[214].green_reg       = 15165;
    shaper->pwl.rgb_resulted[214].blue_reg        = 15165;
    shaper->pwl.rgb_resulted[214].delta_red_reg   = 127;
    shaper->pwl.rgb_resulted[214].delta_green_reg = 127;
    shaper->pwl.rgb_resulted[214].delta_blue_reg  = 127;
    shaper->pwl.rgb_resulted[215].red_reg         = 15292;
    shaper->pwl.rgb_resulted[215].green_reg       = 15292;
    shaper->pwl.rgb_resulted[215].blue_reg        = 15292;
    shaper->pwl.rgb_resulted[215].delta_red_reg   = 126;
    shaper->pwl.rgb_resulted[215].delta_green_reg = 126;
    shaper->pwl.rgb_resulted[215].delta_blue_reg  = 126;
    shaper->pwl.rgb_resulted[216].red_reg         = 15418;
    shaper->pwl.rgb_resulted[216].green_reg       = 15418;
    shaper->pwl.rgb_resulted[216].blue_reg        = 15418;
    shaper->pwl.rgb_resulted[216].delta_red_reg   = 125;
    shaper->pwl.rgb_resulted[216].delta_green_reg = 125;
    shaper->pwl.rgb_resulted[216].delta_blue_reg  = 125;
    shaper->pwl.rgb_resulted[217].red_reg         = 15543;
    shaper->pwl.rgb_resulted[217].green_reg       = 15543;
    shaper->pwl.rgb_resulted[217].blue_reg        = 15543;
    shaper->pwl.rgb_resulted[217].delta_red_reg   = 123;
    shaper->pwl.rgb_resulted[217].delta_green_reg = 123;
    shaper->pwl.rgb_resulted[217].delta_blue_reg  = 123;
    shaper->pwl.rgb_resulted[218].red_reg         = 15666;
    shaper->pwl.rgb_resulted[218].green_reg       = 15666;
    shaper->pwl.rgb_resulted[218].blue_reg        = 15666;
    shaper->pwl.rgb_resulted[218].delta_red_reg   = 122;
    shaper->pwl.rgb_resulted[218].delta_green_reg = 122;
    shaper->pwl.rgb_resulted[218].delta_blue_reg  = 122;
    shaper->pwl.rgb_resulted[219].red_reg         = 15788;
    shaper->pwl.rgb_resulted[219].green_reg       = 15788;
    shaper->pwl.rgb_resulted[219].blue_reg        = 15788;
    shaper->pwl.rgb_resulted[219].delta_red_reg   = 121;
    shaper->pwl.rgb_resulted[219].delta_green_reg = 121;
    shaper->pwl.rgb_resulted[219].delta_blue_reg  = 121;
    shaper->pwl.rgb_resulted[220].red_reg         = 15909;
    shaper->pwl.rgb_resulted[220].green_reg       = 15909;
    shaper->pwl.rgb_resulted[220].blue_reg        = 15909;
    shaper->pwl.rgb_resulted[220].delta_red_reg   = 120;
    shaper->pwl.rgb_resulted[220].delta_green_reg = 120;
    shaper->pwl.rgb_resulted[220].delta_blue_reg  = 120;
    shaper->pwl.rgb_resulted[221].red_reg         = 16029;
    shaper->pwl.rgb_resulted[221].green_reg       = 16029;
    shaper->pwl.rgb_resulted[221].blue_reg        = 16029;
    shaper->pwl.rgb_resulted[221].delta_red_reg   = 119;
    shaper->pwl.rgb_resulted[221].delta_green_reg = 119;
    shaper->pwl.rgb_resulted[221].delta_blue_reg  = 119;
    shaper->pwl.rgb_resulted[222].red_reg         = 16148;
    shaper->pwl.rgb_resulted[222].green_reg       = 16148;
    shaper->pwl.rgb_resulted[222].blue_reg        = 16148;
    shaper->pwl.rgb_resulted[222].delta_red_reg   = 118;
    shaper->pwl.rgb_resulted[222].delta_green_reg = 118;
    shaper->pwl.rgb_resulted[222].delta_blue_reg  = 118;
    shaper->pwl.rgb_resulted[223].red_reg         = 16266;
    shaper->pwl.rgb_resulted[223].green_reg       = 16266;
    shaper->pwl.rgb_resulted[223].blue_reg        = 16266;
    shaper->pwl.rgb_resulted[223].delta_red_reg   = 117;
    shaper->pwl.rgb_resulted[223].delta_green_reg = 117;
    shaper->pwl.rgb_resulted[223].delta_blue_reg  = 117;
    shaper->pwl.rgb_resulted[224].red_reg         = 16383;
    shaper->pwl.rgb_resulted[224].green_reg       = 16383;
    shaper->pwl.rgb_resulted[224].blue_reg        = 16383;
    shaper->pwl.rgb_resulted[224].delta_red_reg   = 4294951143;
    shaper->pwl.rgb_resulted[224].delta_green_reg = 4294951143;
    shaper->pwl.rgb_resulted[224].delta_blue_reg  = 4294951143;
    shaper->pwl.rgb_resulted[225].red_reg         = 230;
    shaper->pwl.rgb_resulted[225].green_reg       = 230;
    shaper->pwl.rgb_resulted[225].blue_reg        = 230;
    shaper->pwl.rgb_resulted[225].delta_red_reg   = 227;
    shaper->pwl.rgb_resulted[225].delta_green_reg = 227;
    shaper->pwl.rgb_resulted[225].delta_blue_reg  = 227;
    shaper->pwl.rgb_resulted[226].red_reg         = 457;
    shaper->pwl.rgb_resulted[226].green_reg       = 457;
    shaper->pwl.rgb_resulted[226].blue_reg        = 457;
    shaper->pwl.rgb_resulted[226].delta_red_reg   = 223;
    shaper->pwl.rgb_resulted[226].delta_green_reg = 223;
    shaper->pwl.rgb_resulted[226].delta_blue_reg  = 223;
    shaper->pwl.rgb_resulted[227].red_reg         = 680;
    shaper->pwl.rgb_resulted[227].green_reg       = 680;
    shaper->pwl.rgb_resulted[227].blue_reg        = 680;
    shaper->pwl.rgb_resulted[227].delta_red_reg   = 220;
    shaper->pwl.rgb_resulted[227].delta_green_reg = 220;
    shaper->pwl.rgb_resulted[227].delta_blue_reg  = 220;
    shaper->pwl.rgb_resulted[228].red_reg         = 900;
    shaper->pwl.rgb_resulted[228].green_reg       = 900;
    shaper->pwl.rgb_resulted[228].blue_reg        = 900;
    shaper->pwl.rgb_resulted[228].delta_red_reg   = 217;
    shaper->pwl.rgb_resulted[228].delta_green_reg = 217;
    shaper->pwl.rgb_resulted[228].delta_blue_reg  = 217;
    shaper->pwl.rgb_resulted[229].red_reg         = 1117;
    shaper->pwl.rgb_resulted[229].green_reg       = 1117;
    shaper->pwl.rgb_resulted[229].blue_reg        = 1117;
    shaper->pwl.rgb_resulted[229].delta_red_reg   = 213;
    shaper->pwl.rgb_resulted[229].delta_green_reg = 213;
    shaper->pwl.rgb_resulted[229].delta_blue_reg  = 213;
    shaper->pwl.rgb_resulted[230].red_reg         = 1330;
    shaper->pwl.rgb_resulted[230].green_reg       = 1330;
    shaper->pwl.rgb_resulted[230].blue_reg        = 1330;
    shaper->pwl.rgb_resulted[230].delta_red_reg   = 210;
    shaper->pwl.rgb_resulted[230].delta_green_reg = 210;
    shaper->pwl.rgb_resulted[230].delta_blue_reg  = 210;
    shaper->pwl.rgb_resulted[231].red_reg         = 1540;
    shaper->pwl.rgb_resulted[231].green_reg       = 1540;
    shaper->pwl.rgb_resulted[231].blue_reg        = 1540;
    shaper->pwl.rgb_resulted[231].delta_red_reg   = 208;
    shaper->pwl.rgb_resulted[231].delta_green_reg = 208;
    shaper->pwl.rgb_resulted[231].delta_blue_reg  = 208;
    shaper->pwl.rgb_resulted[232].red_reg         = 1748;
    shaper->pwl.rgb_resulted[232].green_reg       = 1748;
    shaper->pwl.rgb_resulted[232].blue_reg        = 1748;
    shaper->pwl.rgb_resulted[232].delta_red_reg   = 205;
    shaper->pwl.rgb_resulted[232].delta_green_reg = 205;
    shaper->pwl.rgb_resulted[232].delta_blue_reg  = 205;
    shaper->pwl.rgb_resulted[233].red_reg         = 1953;
    shaper->pwl.rgb_resulted[233].green_reg       = 1953;
    shaper->pwl.rgb_resulted[233].blue_reg        = 1953;
    shaper->pwl.rgb_resulted[233].delta_red_reg   = 202;
    shaper->pwl.rgb_resulted[233].delta_green_reg = 202;
    shaper->pwl.rgb_resulted[233].delta_blue_reg  = 202;
    shaper->pwl.rgb_resulted[234].red_reg         = 2155;
    shaper->pwl.rgb_resulted[234].green_reg       = 2155;
    shaper->pwl.rgb_resulted[234].blue_reg        = 2155;
    shaper->pwl.rgb_resulted[234].delta_red_reg   = 199;
    shaper->pwl.rgb_resulted[234].delta_green_reg = 199;
    shaper->pwl.rgb_resulted[234].delta_blue_reg  = 199;
    shaper->pwl.rgb_resulted[235].red_reg         = 2354;
    shaper->pwl.rgb_resulted[235].green_reg       = 2354;
    shaper->pwl.rgb_resulted[235].blue_reg        = 2354;
    shaper->pwl.rgb_resulted[235].delta_red_reg   = 197;
    shaper->pwl.rgb_resulted[235].delta_green_reg = 197;
    shaper->pwl.rgb_resulted[235].delta_blue_reg  = 197;
    shaper->pwl.rgb_resulted[236].red_reg         = 2551;
    shaper->pwl.rgb_resulted[236].green_reg       = 2551;
    shaper->pwl.rgb_resulted[236].blue_reg        = 2551;
    shaper->pwl.rgb_resulted[236].delta_red_reg   = 194;
    shaper->pwl.rgb_resulted[236].delta_green_reg = 194;
    shaper->pwl.rgb_resulted[236].delta_blue_reg  = 194;
    shaper->pwl.rgb_resulted[237].red_reg         = 2745;
    shaper->pwl.rgb_resulted[237].green_reg       = 2745;
    shaper->pwl.rgb_resulted[237].blue_reg        = 2745;
    shaper->pwl.rgb_resulted[237].delta_red_reg   = 192;
    shaper->pwl.rgb_resulted[237].delta_green_reg = 192;
    shaper->pwl.rgb_resulted[237].delta_blue_reg  = 192;
    shaper->pwl.rgb_resulted[238].red_reg         = 2937;
    shaper->pwl.rgb_resulted[238].green_reg       = 2937;
    shaper->pwl.rgb_resulted[238].blue_reg        = 2937;
    shaper->pwl.rgb_resulted[238].delta_red_reg   = 190;
    shaper->pwl.rgb_resulted[238].delta_green_reg = 190;
    shaper->pwl.rgb_resulted[238].delta_blue_reg  = 190;
    shaper->pwl.rgb_resulted[239].red_reg         = 3127;
    shaper->pwl.rgb_resulted[239].green_reg       = 3127;
    shaper->pwl.rgb_resulted[239].blue_reg        = 3127;
    shaper->pwl.rgb_resulted[239].delta_red_reg   = 188;
    shaper->pwl.rgb_resulted[239].delta_green_reg = 188;
    shaper->pwl.rgb_resulted[239].delta_blue_reg  = 188;
    shaper->pwl.rgb_resulted[240].red_reg         = 3315;
    shaper->pwl.rgb_resulted[240].green_reg       = 3315;
    shaper->pwl.rgb_resulted[240].blue_reg        = 3315;
    shaper->pwl.rgb_resulted[240].delta_red_reg   = 185;
    shaper->pwl.rgb_resulted[240].delta_green_reg = 185;
    shaper->pwl.rgb_resulted[240].delta_blue_reg  = 185;
    shaper->pwl.rgb_resulted[241].red_reg         = 3500;
    shaper->pwl.rgb_resulted[241].green_reg       = 3500;
    shaper->pwl.rgb_resulted[241].blue_reg        = 3500;
    shaper->pwl.rgb_resulted[241].delta_red_reg   = 184;
    shaper->pwl.rgb_resulted[241].delta_green_reg = 184;
    shaper->pwl.rgb_resulted[241].delta_blue_reg  = 184;
    shaper->pwl.rgb_resulted[242].red_reg         = 3684;
    shaper->pwl.rgb_resulted[242].green_reg       = 3684;
    shaper->pwl.rgb_resulted[242].blue_reg        = 3684;
    shaper->pwl.rgb_resulted[242].delta_red_reg   = 181;
    shaper->pwl.rgb_resulted[242].delta_green_reg = 181;
    shaper->pwl.rgb_resulted[242].delta_blue_reg  = 181;
    shaper->pwl.rgb_resulted[243].red_reg         = 3865;
    shaper->pwl.rgb_resulted[243].green_reg       = 3865;
    shaper->pwl.rgb_resulted[243].blue_reg        = 3865;
    shaper->pwl.rgb_resulted[243].delta_red_reg   = 179;
    shaper->pwl.rgb_resulted[243].delta_green_reg = 179;
    shaper->pwl.rgb_resulted[243].delta_blue_reg  = 179;
    shaper->pwl.rgb_resulted[244].red_reg         = 4044;
    shaper->pwl.rgb_resulted[244].green_reg       = 4044;
    shaper->pwl.rgb_resulted[244].blue_reg        = 4044;
    shaper->pwl.rgb_resulted[244].delta_red_reg   = 178;
    shaper->pwl.rgb_resulted[244].delta_green_reg = 178;
    shaper->pwl.rgb_resulted[244].delta_blue_reg  = 178;
    shaper->pwl.rgb_resulted[245].red_reg         = 4222;
    shaper->pwl.rgb_resulted[245].green_reg       = 4222;
    shaper->pwl.rgb_resulted[245].blue_reg        = 4222;
    shaper->pwl.rgb_resulted[245].delta_red_reg   = 176;
    shaper->pwl.rgb_resulted[245].delta_green_reg = 176;
    shaper->pwl.rgb_resulted[245].delta_blue_reg  = 176;
    shaper->pwl.rgb_resulted[246].red_reg         = 4398;
    shaper->pwl.rgb_resulted[246].green_reg       = 4398;
    shaper->pwl.rgb_resulted[246].blue_reg        = 4398;
    shaper->pwl.rgb_resulted[246].delta_red_reg   = 174;
    shaper->pwl.rgb_resulted[246].delta_green_reg = 174;
    shaper->pwl.rgb_resulted[246].delta_blue_reg  = 174;
    shaper->pwl.rgb_resulted[247].red_reg         = 4572;
    shaper->pwl.rgb_resulted[247].green_reg       = 4572;
    shaper->pwl.rgb_resulted[247].blue_reg        = 4572;
    shaper->pwl.rgb_resulted[247].delta_red_reg   = 172;
    shaper->pwl.rgb_resulted[247].delta_green_reg = 172;
    shaper->pwl.rgb_resulted[247].delta_blue_reg  = 172;
    shaper->pwl.rgb_resulted[248].red_reg         = 4744;
    shaper->pwl.rgb_resulted[248].green_reg       = 4744;
    shaper->pwl.rgb_resulted[248].blue_reg        = 4744;
    shaper->pwl.rgb_resulted[248].delta_red_reg   = 171;
    shaper->pwl.rgb_resulted[248].delta_green_reg = 171;
    shaper->pwl.rgb_resulted[248].delta_blue_reg  = 171;
    shaper->pwl.rgb_resulted[249].red_reg         = 4915;
    shaper->pwl.rgb_resulted[249].green_reg       = 4915;
    shaper->pwl.rgb_resulted[249].blue_reg        = 4915;
    shaper->pwl.rgb_resulted[249].delta_red_reg   = 169;
    shaper->pwl.rgb_resulted[249].delta_green_reg = 169;
    shaper->pwl.rgb_resulted[249].delta_blue_reg  = 169;
    shaper->pwl.rgb_resulted[250].red_reg         = 5084;
    shaper->pwl.rgb_resulted[250].green_reg       = 5084;
    shaper->pwl.rgb_resulted[250].blue_reg        = 5084;
    shaper->pwl.rgb_resulted[250].delta_red_reg   = 168;
    shaper->pwl.rgb_resulted[250].delta_green_reg = 168;
    shaper->pwl.rgb_resulted[250].delta_blue_reg  = 168;
    shaper->pwl.rgb_resulted[251].red_reg         = 5252;
    shaper->pwl.rgb_resulted[251].green_reg       = 5252;
    shaper->pwl.rgb_resulted[251].blue_reg        = 5252;
    shaper->pwl.rgb_resulted[251].delta_red_reg   = 165;
    shaper->pwl.rgb_resulted[251].delta_green_reg = 165;
    shaper->pwl.rgb_resulted[251].delta_blue_reg  = 165;
    shaper->pwl.rgb_resulted[252].red_reg         = 5417;
    shaper->pwl.rgb_resulted[252].green_reg       = 5417;
    shaper->pwl.rgb_resulted[252].blue_reg        = 5417;
    shaper->pwl.rgb_resulted[252].delta_red_reg   = 165;
    shaper->pwl.rgb_resulted[252].delta_green_reg = 165;
    shaper->pwl.rgb_resulted[252].delta_blue_reg  = 165;
    shaper->pwl.rgb_resulted[253].red_reg         = 5582;
    shaper->pwl.rgb_resulted[253].green_reg       = 5582;
    shaper->pwl.rgb_resulted[253].blue_reg        = 5582;
    shaper->pwl.rgb_resulted[253].delta_red_reg   = 163;
    shaper->pwl.rgb_resulted[253].delta_green_reg = 163;
    shaper->pwl.rgb_resulted[253].delta_blue_reg  = 163;
    shaper->pwl.rgb_resulted[254].red_reg         = 5745;
    shaper->pwl.rgb_resulted[254].green_reg       = 5745;
    shaper->pwl.rgb_resulted[254].blue_reg        = 5745;
    shaper->pwl.rgb_resulted[254].delta_red_reg   = 161;
    shaper->pwl.rgb_resulted[254].delta_green_reg = 161;
    shaper->pwl.rgb_resulted[254].delta_blue_reg  = 161;
    shaper->pwl.rgb_resulted[255].red_reg         = 5906;
    shaper->pwl.rgb_resulted[255].green_reg       = 5906;
    shaper->pwl.rgb_resulted[255].blue_reg        = 5906;
    shaper->pwl.rgb_resulted[255].delta_red_reg   = 0;
    shaper->pwl.rgb_resulted[255].delta_green_reg = 0;
    shaper->pwl.rgb_resulted[255].delta_blue_reg  = 0;
    shaper->pwl.rgb_resulted[256].red_reg         = 0;
    shaper->pwl.rgb_resulted[256].green_reg       = 0;
    shaper->pwl.rgb_resulted[256].blue_reg        = 0;
    shaper->pwl.rgb_resulted[256].delta_red_reg   = 0;
    shaper->pwl.rgb_resulted[256].delta_green_reg = 0;
    shaper->pwl.rgb_resulted[256].delta_blue_reg  = 0;
    shaper->pwl.rgb_resulted[257].red_reg         = 0;
    shaper->pwl.rgb_resulted[257].green_reg       = 0;
    shaper->pwl.rgb_resulted[257].blue_reg        = 0;
    shaper->pwl.rgb_resulted[257].delta_red_reg   = 0;
    shaper->pwl.rgb_resulted[257].delta_green_reg = 0;
    shaper->pwl.rgb_resulted[257].delta_blue_reg  = 0;
    shaper->pwl.rgb_resulted[258].red_reg         = 0;
    shaper->pwl.rgb_resulted[258].green_reg       = 0;
    shaper->pwl.rgb_resulted[258].blue_reg        = 0;
    shaper->pwl.rgb_resulted[258].delta_red_reg   = 0;
    shaper->pwl.rgb_resulted[258].delta_green_reg = 0;
    shaper->pwl.rgb_resulted[258].delta_blue_reg  = 0;

    return true;
}

bool build_test_post1dlut_sdr(struct transfer_func *post1D)
{
    if (post1D == NULL)
        return false;

    memset(post1D, 0, sizeof(struct transfer_func));

    post1D->type = TF_TYPE_DISTRIBUTED_POINTS;
    post1D->tf   = TRANSFER_FUNC_SRGB; // see comment for shaper - actually 2.2, but irrelevant when
                                       // distributed points used
    post1D->sdr_ref_white_level = 80;

    post1D->tf_pts.red[0].value      = 0;
    post1D->tf_pts.green[0].value    = 0;
    post1D->tf_pts.blue[0].value     = 0;
    post1D->tf_pts.red[1].value      = 0;
    post1D->tf_pts.green[1].value    = 0;
    post1D->tf_pts.blue[1].value     = 0;
    post1D->tf_pts.red[2].value      = 0;
    post1D->tf_pts.green[2].value    = 0;
    post1D->tf_pts.blue[2].value     = 0;
    post1D->tf_pts.red[3].value      = 0;
    post1D->tf_pts.green[3].value    = 0;
    post1D->tf_pts.blue[3].value     = 0;
    post1D->tf_pts.red[4].value      = 0;
    post1D->tf_pts.green[4].value    = 0;
    post1D->tf_pts.blue[4].value     = 0;
    post1D->tf_pts.red[5].value      = 0;
    post1D->tf_pts.green[5].value    = 0;
    post1D->tf_pts.blue[5].value     = 0;
    post1D->tf_pts.red[6].value      = 0;
    post1D->tf_pts.green[6].value    = 0;
    post1D->tf_pts.blue[6].value     = 0;
    post1D->tf_pts.red[7].value      = 0;
    post1D->tf_pts.green[7].value    = 0;
    post1D->tf_pts.blue[7].value     = 0;
    post1D->tf_pts.red[8].value      = 0;
    post1D->tf_pts.green[8].value    = 0;
    post1D->tf_pts.blue[8].value     = 0;
    post1D->tf_pts.red[9].value      = 0;
    post1D->tf_pts.green[9].value    = 0;
    post1D->tf_pts.blue[9].value     = 0;
    post1D->tf_pts.red[10].value     = 0;
    post1D->tf_pts.green[10].value   = 0;
    post1D->tf_pts.blue[10].value    = 0;
    post1D->tf_pts.red[11].value     = 0;
    post1D->tf_pts.green[11].value   = 0;
    post1D->tf_pts.blue[11].value    = 0;
    post1D->tf_pts.red[12].value     = 0;
    post1D->tf_pts.green[12].value   = 0;
    post1D->tf_pts.blue[12].value    = 0;
    post1D->tf_pts.red[13].value     = 0;
    post1D->tf_pts.green[13].value   = 0;
    post1D->tf_pts.blue[13].value    = 0;
    post1D->tf_pts.red[14].value     = 0;
    post1D->tf_pts.green[14].value   = 0;
    post1D->tf_pts.blue[14].value    = 0;
    post1D->tf_pts.red[15].value     = 0;
    post1D->tf_pts.green[15].value   = 0;
    post1D->tf_pts.blue[15].value    = 0;
    post1D->tf_pts.red[16].value     = 0;
    post1D->tf_pts.green[16].value   = 0;
    post1D->tf_pts.blue[16].value    = 0;
    post1D->tf_pts.red[17].value     = 0;
    post1D->tf_pts.green[17].value   = 0;
    post1D->tf_pts.blue[17].value    = 0;
    post1D->tf_pts.red[18].value     = 0;
    post1D->tf_pts.green[18].value   = 0;
    post1D->tf_pts.blue[18].value    = 0;
    post1D->tf_pts.red[19].value     = 0;
    post1D->tf_pts.green[19].value   = 0;
    post1D->tf_pts.blue[19].value    = 0;
    post1D->tf_pts.red[20].value     = 0;
    post1D->tf_pts.green[20].value   = 0;
    post1D->tf_pts.blue[20].value    = 0;
    post1D->tf_pts.red[21].value     = 0;
    post1D->tf_pts.green[21].value   = 0;
    post1D->tf_pts.blue[21].value    = 0;
    post1D->tf_pts.red[22].value     = 0;
    post1D->tf_pts.green[22].value   = 0;
    post1D->tf_pts.blue[22].value    = 0;
    post1D->tf_pts.red[23].value     = 0;
    post1D->tf_pts.green[23].value   = 0;
    post1D->tf_pts.blue[23].value    = 0;
    post1D->tf_pts.red[24].value     = 0;
    post1D->tf_pts.green[24].value   = 0;
    post1D->tf_pts.blue[24].value    = 0;
    post1D->tf_pts.red[25].value     = 0;
    post1D->tf_pts.green[25].value   = 0;
    post1D->tf_pts.blue[25].value    = 0;
    post1D->tf_pts.red[26].value     = 0;
    post1D->tf_pts.green[26].value   = 0;
    post1D->tf_pts.blue[26].value    = 0;
    post1D->tf_pts.red[27].value     = 0;
    post1D->tf_pts.green[27].value   = 0;
    post1D->tf_pts.blue[27].value    = 0;
    post1D->tf_pts.red[28].value     = 0;
    post1D->tf_pts.green[28].value   = 0;
    post1D->tf_pts.blue[28].value    = 0;
    post1D->tf_pts.red[29].value     = 0;
    post1D->tf_pts.green[29].value   = 0;
    post1D->tf_pts.blue[29].value    = 0;
    post1D->tf_pts.red[30].value     = 0;
    post1D->tf_pts.green[30].value   = 0;
    post1D->tf_pts.blue[30].value    = 0;
    post1D->tf_pts.red[31].value     = 0;
    post1D->tf_pts.green[31].value   = 0;
    post1D->tf_pts.blue[31].value    = 0;
    post1D->tf_pts.red[32].value     = 0;
    post1D->tf_pts.green[32].value   = 0;
    post1D->tf_pts.blue[32].value    = 0;
    post1D->tf_pts.red[33].value     = 0;
    post1D->tf_pts.green[33].value   = 0;
    post1D->tf_pts.blue[33].value    = 0;
    post1D->tf_pts.red[34].value     = 0;
    post1D->tf_pts.green[34].value   = 0;
    post1D->tf_pts.blue[34].value    = 0;
    post1D->tf_pts.red[35].value     = 0;
    post1D->tf_pts.green[35].value   = 0;
    post1D->tf_pts.blue[35].value    = 0;
    post1D->tf_pts.red[36].value     = 0;
    post1D->tf_pts.green[36].value   = 0;
    post1D->tf_pts.blue[36].value    = 0;
    post1D->tf_pts.red[37].value     = 0;
    post1D->tf_pts.green[37].value   = 0;
    post1D->tf_pts.blue[37].value    = 0;
    post1D->tf_pts.red[38].value     = 0;
    post1D->tf_pts.green[38].value   = 0;
    post1D->tf_pts.blue[38].value    = 0;
    post1D->tf_pts.red[39].value     = 0;
    post1D->tf_pts.green[39].value   = 0;
    post1D->tf_pts.blue[39].value    = 0;
    post1D->tf_pts.red[40].value     = 0;
    post1D->tf_pts.green[40].value   = 0;
    post1D->tf_pts.blue[40].value    = 0;
    post1D->tf_pts.red[41].value     = 0;
    post1D->tf_pts.green[41].value   = 0;
    post1D->tf_pts.blue[41].value    = 0;
    post1D->tf_pts.red[42].value     = 0;
    post1D->tf_pts.green[42].value   = 0;
    post1D->tf_pts.blue[42].value    = 0;
    post1D->tf_pts.red[43].value     = 0;
    post1D->tf_pts.green[43].value   = 0;
    post1D->tf_pts.blue[43].value    = 0;
    post1D->tf_pts.red[44].value     = 0;
    post1D->tf_pts.green[44].value   = 0;
    post1D->tf_pts.blue[44].value    = 0;
    post1D->tf_pts.red[45].value     = 0;
    post1D->tf_pts.green[45].value   = 0;
    post1D->tf_pts.blue[45].value    = 0;
    post1D->tf_pts.red[46].value     = 0;
    post1D->tf_pts.green[46].value   = 0;
    post1D->tf_pts.blue[46].value    = 0;
    post1D->tf_pts.red[47].value     = 0;
    post1D->tf_pts.green[47].value   = 0;
    post1D->tf_pts.blue[47].value    = 0;
    post1D->tf_pts.red[48].value     = 0;
    post1D->tf_pts.green[48].value   = 0;
    post1D->tf_pts.blue[48].value    = 0;
    post1D->tf_pts.red[49].value     = 0;
    post1D->tf_pts.green[49].value   = 0;
    post1D->tf_pts.blue[49].value    = 0;
    post1D->tf_pts.red[50].value     = 0;
    post1D->tf_pts.green[50].value   = 0;
    post1D->tf_pts.blue[50].value    = 0;
    post1D->tf_pts.red[51].value     = 0;
    post1D->tf_pts.green[51].value   = 0;
    post1D->tf_pts.blue[51].value    = 0;
    post1D->tf_pts.red[52].value     = 0;
    post1D->tf_pts.green[52].value   = 0;
    post1D->tf_pts.blue[52].value    = 0;
    post1D->tf_pts.red[53].value     = 0;
    post1D->tf_pts.green[53].value   = 0;
    post1D->tf_pts.blue[53].value    = 0;
    post1D->tf_pts.red[54].value     = 0;
    post1D->tf_pts.green[54].value   = 0;
    post1D->tf_pts.blue[54].value    = 0;
    post1D->tf_pts.red[55].value     = 0;
    post1D->tf_pts.green[55].value   = 0;
    post1D->tf_pts.blue[55].value    = 0;
    post1D->tf_pts.red[56].value     = 0;
    post1D->tf_pts.green[56].value   = 0;
    post1D->tf_pts.blue[56].value    = 0;
    post1D->tf_pts.red[57].value     = 0;
    post1D->tf_pts.green[57].value   = 0;
    post1D->tf_pts.blue[57].value    = 0;
    post1D->tf_pts.red[58].value     = 0;
    post1D->tf_pts.green[58].value   = 0;
    post1D->tf_pts.blue[58].value    = 0;
    post1D->tf_pts.red[59].value     = 0;
    post1D->tf_pts.green[59].value   = 0;
    post1D->tf_pts.blue[59].value    = 0;
    post1D->tf_pts.red[60].value     = 0;
    post1D->tf_pts.green[60].value   = 0;
    post1D->tf_pts.blue[60].value    = 0;
    post1D->tf_pts.red[61].value     = 0;
    post1D->tf_pts.green[61].value   = 0;
    post1D->tf_pts.blue[61].value    = 0;
    post1D->tf_pts.red[62].value     = 0;
    post1D->tf_pts.green[62].value   = 0;
    post1D->tf_pts.blue[62].value    = 0;
    post1D->tf_pts.red[63].value     = 0;
    post1D->tf_pts.green[63].value   = 0;
    post1D->tf_pts.blue[63].value    = 0;
    post1D->tf_pts.red[64].value     = 0;
    post1D->tf_pts.green[64].value   = 0;
    post1D->tf_pts.blue[64].value    = 0;
    post1D->tf_pts.red[65].value     = 0;
    post1D->tf_pts.green[65].value   = 0;
    post1D->tf_pts.blue[65].value    = 0;
    post1D->tf_pts.red[66].value     = 0;
    post1D->tf_pts.green[66].value   = 0;
    post1D->tf_pts.blue[66].value    = 0;
    post1D->tf_pts.red[67].value     = 0;
    post1D->tf_pts.green[67].value   = 0;
    post1D->tf_pts.blue[67].value    = 0;
    post1D->tf_pts.red[68].value     = 0;
    post1D->tf_pts.green[68].value   = 0;
    post1D->tf_pts.blue[68].value    = 0;
    post1D->tf_pts.red[69].value     = 0;
    post1D->tf_pts.green[69].value   = 0;
    post1D->tf_pts.blue[69].value    = 0;
    post1D->tf_pts.red[70].value     = 0;
    post1D->tf_pts.green[70].value   = 0;
    post1D->tf_pts.blue[70].value    = 0;
    post1D->tf_pts.red[71].value     = 0;
    post1D->tf_pts.green[71].value   = 0;
    post1D->tf_pts.blue[71].value    = 0;
    post1D->tf_pts.red[72].value     = 0;
    post1D->tf_pts.green[72].value   = 0;
    post1D->tf_pts.blue[72].value    = 0;
    post1D->tf_pts.red[73].value     = 0;
    post1D->tf_pts.green[73].value   = 0;
    post1D->tf_pts.blue[73].value    = 0;
    post1D->tf_pts.red[74].value     = 0;
    post1D->tf_pts.green[74].value   = 0;
    post1D->tf_pts.blue[74].value    = 0;
    post1D->tf_pts.red[75].value     = 0;
    post1D->tf_pts.green[75].value   = 0;
    post1D->tf_pts.blue[75].value    = 0;
    post1D->tf_pts.red[76].value     = 0;
    post1D->tf_pts.green[76].value   = 0;
    post1D->tf_pts.blue[76].value    = 0;
    post1D->tf_pts.red[77].value     = 0;
    post1D->tf_pts.green[77].value   = 0;
    post1D->tf_pts.blue[77].value    = 0;
    post1D->tf_pts.red[78].value     = 0;
    post1D->tf_pts.green[78].value   = 0;
    post1D->tf_pts.blue[78].value    = 0;
    post1D->tf_pts.red[79].value     = 0;
    post1D->tf_pts.green[79].value   = 0;
    post1D->tf_pts.blue[79].value    = 0;
    post1D->tf_pts.red[80].value     = 0;
    post1D->tf_pts.green[80].value   = 0;
    post1D->tf_pts.blue[80].value    = 0;
    post1D->tf_pts.red[81].value     = 0;
    post1D->tf_pts.green[81].value   = 0;
    post1D->tf_pts.blue[81].value    = 0;
    post1D->tf_pts.red[82].value     = 0;
    post1D->tf_pts.green[82].value   = 0;
    post1D->tf_pts.blue[82].value    = 0;
    post1D->tf_pts.red[83].value     = 0;
    post1D->tf_pts.green[83].value   = 0;
    post1D->tf_pts.blue[83].value    = 0;
    post1D->tf_pts.red[84].value     = 0;
    post1D->tf_pts.green[84].value   = 0;
    post1D->tf_pts.blue[84].value    = 0;
    post1D->tf_pts.red[85].value     = 0;
    post1D->tf_pts.green[85].value   = 0;
    post1D->tf_pts.blue[85].value    = 0;
    post1D->tf_pts.red[86].value     = 0;
    post1D->tf_pts.green[86].value   = 0;
    post1D->tf_pts.blue[86].value    = 0;
    post1D->tf_pts.red[87].value     = 0;
    post1D->tf_pts.green[87].value   = 0;
    post1D->tf_pts.blue[87].value    = 0;
    post1D->tf_pts.red[88].value     = 0;
    post1D->tf_pts.green[88].value   = 0;
    post1D->tf_pts.blue[88].value    = 0;
    post1D->tf_pts.red[89].value     = 0;
    post1D->tf_pts.green[89].value   = 0;
    post1D->tf_pts.blue[89].value    = 0;
    post1D->tf_pts.red[90].value     = 0;
    post1D->tf_pts.green[90].value   = 0;
    post1D->tf_pts.blue[90].value    = 0;
    post1D->tf_pts.red[91].value     = 0;
    post1D->tf_pts.green[91].value   = 0;
    post1D->tf_pts.blue[91].value    = 0;
    post1D->tf_pts.red[92].value     = 0;
    post1D->tf_pts.green[92].value   = 0;
    post1D->tf_pts.blue[92].value    = 0;
    post1D->tf_pts.red[93].value     = 0;
    post1D->tf_pts.green[93].value   = 0;
    post1D->tf_pts.blue[93].value    = 0;
    post1D->tf_pts.red[94].value     = 0;
    post1D->tf_pts.green[94].value   = 0;
    post1D->tf_pts.blue[94].value    = 0;
    post1D->tf_pts.red[95].value     = 0;
    post1D->tf_pts.green[95].value   = 0;
    post1D->tf_pts.blue[95].value    = 0;
    post1D->tf_pts.red[96].value     = 0;
    post1D->tf_pts.green[96].value   = 0;
    post1D->tf_pts.blue[96].value    = 0;
    post1D->tf_pts.red[97].value     = 0;
    post1D->tf_pts.green[97].value   = 0;
    post1D->tf_pts.blue[97].value    = 0;
    post1D->tf_pts.red[98].value     = 0;
    post1D->tf_pts.green[98].value   = 0;
    post1D->tf_pts.blue[98].value    = 0;
    post1D->tf_pts.red[99].value     = 0;
    post1D->tf_pts.green[99].value   = 0;
    post1D->tf_pts.blue[99].value    = 0;
    post1D->tf_pts.red[100].value    = 0;
    post1D->tf_pts.green[100].value  = 0;
    post1D->tf_pts.blue[100].value   = 0;
    post1D->tf_pts.red[101].value    = 0;
    post1D->tf_pts.green[101].value  = 0;
    post1D->tf_pts.blue[101].value   = 0;
    post1D->tf_pts.red[102].value    = 0;
    post1D->tf_pts.green[102].value  = 0;
    post1D->tf_pts.blue[102].value   = 0;
    post1D->tf_pts.red[103].value    = 0;
    post1D->tf_pts.green[103].value  = 0;
    post1D->tf_pts.blue[103].value   = 0;
    post1D->tf_pts.red[104].value    = 0;
    post1D->tf_pts.green[104].value  = 0;
    post1D->tf_pts.blue[104].value   = 0;
    post1D->tf_pts.red[105].value    = 0;
    post1D->tf_pts.green[105].value  = 0;
    post1D->tf_pts.blue[105].value   = 0;
    post1D->tf_pts.red[106].value    = 0;
    post1D->tf_pts.green[106].value  = 0;
    post1D->tf_pts.blue[106].value   = 0;
    post1D->tf_pts.red[107].value    = 0;
    post1D->tf_pts.green[107].value  = 0;
    post1D->tf_pts.blue[107].value   = 0;
    post1D->tf_pts.red[108].value    = 0;
    post1D->tf_pts.green[108].value  = 0;
    post1D->tf_pts.blue[108].value   = 0;
    post1D->tf_pts.red[109].value    = 0;
    post1D->tf_pts.green[109].value  = 0;
    post1D->tf_pts.blue[109].value   = 0;
    post1D->tf_pts.red[110].value    = 0;
    post1D->tf_pts.green[110].value  = 0;
    post1D->tf_pts.blue[110].value   = 0;
    post1D->tf_pts.red[111].value    = 0;
    post1D->tf_pts.green[111].value  = 0;
    post1D->tf_pts.blue[111].value   = 0;
    post1D->tf_pts.red[112].value    = 0;
    post1D->tf_pts.green[112].value  = 0;
    post1D->tf_pts.blue[112].value   = 0;
    post1D->tf_pts.red[113].value    = 0;
    post1D->tf_pts.green[113].value  = 0;
    post1D->tf_pts.blue[113].value   = 0;
    post1D->tf_pts.red[114].value    = 0;
    post1D->tf_pts.green[114].value  = 0;
    post1D->tf_pts.blue[114].value   = 0;
    post1D->tf_pts.red[115].value    = 0;
    post1D->tf_pts.green[115].value  = 0;
    post1D->tf_pts.blue[115].value   = 0;
    post1D->tf_pts.red[116].value    = 0;
    post1D->tf_pts.green[116].value  = 0;
    post1D->tf_pts.blue[116].value   = 0;
    post1D->tf_pts.red[117].value    = 0;
    post1D->tf_pts.green[117].value  = 0;
    post1D->tf_pts.blue[117].value   = 0;
    post1D->tf_pts.red[118].value    = 0;
    post1D->tf_pts.green[118].value  = 0;
    post1D->tf_pts.blue[118].value   = 0;
    post1D->tf_pts.red[119].value    = 0;
    post1D->tf_pts.green[119].value  = 0;
    post1D->tf_pts.blue[119].value   = 0;
    post1D->tf_pts.red[120].value    = 0;
    post1D->tf_pts.green[120].value  = 0;
    post1D->tf_pts.blue[120].value   = 0;
    post1D->tf_pts.red[121].value    = 0;
    post1D->tf_pts.green[121].value  = 0;
    post1D->tf_pts.blue[121].value   = 0;
    post1D->tf_pts.red[122].value    = 0;
    post1D->tf_pts.green[122].value  = 0;
    post1D->tf_pts.blue[122].value   = 0;
    post1D->tf_pts.red[123].value    = 0;
    post1D->tf_pts.green[123].value  = 0;
    post1D->tf_pts.blue[123].value   = 0;
    post1D->tf_pts.red[124].value    = 0;
    post1D->tf_pts.green[124].value  = 0;
    post1D->tf_pts.blue[124].value   = 0;
    post1D->tf_pts.red[125].value    = 0;
    post1D->tf_pts.green[125].value  = 0;
    post1D->tf_pts.blue[125].value   = 0;
    post1D->tf_pts.red[126].value    = 0;
    post1D->tf_pts.green[126].value  = 0;
    post1D->tf_pts.blue[126].value   = 0;
    post1D->tf_pts.red[127].value    = 0;
    post1D->tf_pts.green[127].value  = 0;
    post1D->tf_pts.blue[127].value   = 0;
    post1D->tf_pts.red[128].value    = 0;
    post1D->tf_pts.green[128].value  = 0;
    post1D->tf_pts.blue[128].value   = 0;
    post1D->tf_pts.red[129].value    = 0;
    post1D->tf_pts.green[129].value  = 0;
    post1D->tf_pts.blue[129].value   = 0;
    post1D->tf_pts.red[130].value    = 0;
    post1D->tf_pts.green[130].value  = 0;
    post1D->tf_pts.blue[130].value   = 0;
    post1D->tf_pts.red[131].value    = 0;
    post1D->tf_pts.green[131].value  = 0;
    post1D->tf_pts.blue[131].value   = 0;
    post1D->tf_pts.red[132].value    = 0;
    post1D->tf_pts.green[132].value  = 0;
    post1D->tf_pts.blue[132].value   = 0;
    post1D->tf_pts.red[133].value    = 0;
    post1D->tf_pts.green[133].value  = 0;
    post1D->tf_pts.blue[133].value   = 0;
    post1D->tf_pts.red[134].value    = 0;
    post1D->tf_pts.green[134].value  = 0;
    post1D->tf_pts.blue[134].value   = 0;
    post1D->tf_pts.red[135].value    = 0;
    post1D->tf_pts.green[135].value  = 0;
    post1D->tf_pts.blue[135].value   = 0;
    post1D->tf_pts.red[136].value    = 0;
    post1D->tf_pts.green[136].value  = 0;
    post1D->tf_pts.blue[136].value   = 0;
    post1D->tf_pts.red[137].value    = 0;
    post1D->tf_pts.green[137].value  = 0;
    post1D->tf_pts.blue[137].value   = 0;
    post1D->tf_pts.red[138].value    = 0;
    post1D->tf_pts.green[138].value  = 0;
    post1D->tf_pts.blue[138].value   = 0;
    post1D->tf_pts.red[139].value    = 0;
    post1D->tf_pts.green[139].value  = 0;
    post1D->tf_pts.blue[139].value   = 0;
    post1D->tf_pts.red[140].value    = 0;
    post1D->tf_pts.green[140].value  = 0;
    post1D->tf_pts.blue[140].value   = 0;
    post1D->tf_pts.red[141].value    = 0;
    post1D->tf_pts.green[141].value  = 0;
    post1D->tf_pts.blue[141].value   = 0;
    post1D->tf_pts.red[142].value    = 0;
    post1D->tf_pts.green[142].value  = 0;
    post1D->tf_pts.blue[142].value   = 0;
    post1D->tf_pts.red[143].value    = 0;
    post1D->tf_pts.green[143].value  = 0;
    post1D->tf_pts.blue[143].value   = 0;
    post1D->tf_pts.red[144].value    = 0;
    post1D->tf_pts.green[144].value  = 0;
    post1D->tf_pts.blue[144].value   = 0;
    post1D->tf_pts.red[145].value    = 0;
    post1D->tf_pts.green[145].value  = 0;
    post1D->tf_pts.blue[145].value   = 0;
    post1D->tf_pts.red[146].value    = 0;
    post1D->tf_pts.green[146].value  = 0;
    post1D->tf_pts.blue[146].value   = 0;
    post1D->tf_pts.red[147].value    = 0;
    post1D->tf_pts.green[147].value  = 0;
    post1D->tf_pts.blue[147].value   = 0;
    post1D->tf_pts.red[148].value    = 0;
    post1D->tf_pts.green[148].value  = 0;
    post1D->tf_pts.blue[148].value   = 0;
    post1D->tf_pts.red[149].value    = 0;
    post1D->tf_pts.green[149].value  = 0;
    post1D->tf_pts.blue[149].value   = 0;
    post1D->tf_pts.red[150].value    = 0;
    post1D->tf_pts.green[150].value  = 0;
    post1D->tf_pts.blue[150].value   = 0;
    post1D->tf_pts.red[151].value    = 0;
    post1D->tf_pts.green[151].value  = 0;
    post1D->tf_pts.blue[151].value   = 0;
    post1D->tf_pts.red[152].value    = 0;
    post1D->tf_pts.green[152].value  = 0;
    post1D->tf_pts.blue[152].value   = 0;
    post1D->tf_pts.red[153].value    = 0;
    post1D->tf_pts.green[153].value  = 0;
    post1D->tf_pts.blue[153].value   = 0;
    post1D->tf_pts.red[154].value    = 0;
    post1D->tf_pts.green[154].value  = 0;
    post1D->tf_pts.blue[154].value   = 0;
    post1D->tf_pts.red[155].value    = 0;
    post1D->tf_pts.green[155].value  = 0;
    post1D->tf_pts.blue[155].value   = 0;
    post1D->tf_pts.red[156].value    = 0;
    post1D->tf_pts.green[156].value  = 0;
    post1D->tf_pts.blue[156].value   = 0;
    post1D->tf_pts.red[157].value    = 0;
    post1D->tf_pts.green[157].value  = 0;
    post1D->tf_pts.blue[157].value   = 0;
    post1D->tf_pts.red[158].value    = 0;
    post1D->tf_pts.green[158].value  = 0;
    post1D->tf_pts.blue[158].value   = 0;
    post1D->tf_pts.red[159].value    = 0;
    post1D->tf_pts.green[159].value  = 0;
    post1D->tf_pts.blue[159].value   = 0;
    post1D->tf_pts.red[160].value    = 0;
    post1D->tf_pts.green[160].value  = 0;
    post1D->tf_pts.blue[160].value   = 0;
    post1D->tf_pts.red[161].value    = 0;
    post1D->tf_pts.green[161].value  = 0;
    post1D->tf_pts.blue[161].value   = 0;
    post1D->tf_pts.red[162].value    = 0;
    post1D->tf_pts.green[162].value  = 0;
    post1D->tf_pts.blue[162].value   = 0;
    post1D->tf_pts.red[163].value    = 0;
    post1D->tf_pts.green[163].value  = 0;
    post1D->tf_pts.blue[163].value   = 0;
    post1D->tf_pts.red[164].value    = 0;
    post1D->tf_pts.green[164].value  = 0;
    post1D->tf_pts.blue[164].value   = 0;
    post1D->tf_pts.red[165].value    = 0;
    post1D->tf_pts.green[165].value  = 0;
    post1D->tf_pts.blue[165].value   = 0;
    post1D->tf_pts.red[166].value    = 0;
    post1D->tf_pts.green[166].value  = 0;
    post1D->tf_pts.blue[166].value   = 0;
    post1D->tf_pts.red[167].value    = 0;
    post1D->tf_pts.green[167].value  = 0;
    post1D->tf_pts.blue[167].value   = 0;
    post1D->tf_pts.red[168].value    = 0;
    post1D->tf_pts.green[168].value  = 0;
    post1D->tf_pts.blue[168].value   = 0;
    post1D->tf_pts.red[169].value    = 0;
    post1D->tf_pts.green[169].value  = 0;
    post1D->tf_pts.blue[169].value   = 0;
    post1D->tf_pts.red[170].value    = 0;
    post1D->tf_pts.green[170].value  = 0;
    post1D->tf_pts.blue[170].value   = 0;
    post1D->tf_pts.red[171].value    = 0;
    post1D->tf_pts.green[171].value  = 0;
    post1D->tf_pts.blue[171].value   = 0;
    post1D->tf_pts.red[172].value    = 0;
    post1D->tf_pts.green[172].value  = 0;
    post1D->tf_pts.blue[172].value   = 0;
    post1D->tf_pts.red[173].value    = 0;
    post1D->tf_pts.green[173].value  = 0;
    post1D->tf_pts.blue[173].value   = 0;
    post1D->tf_pts.red[174].value    = 0;
    post1D->tf_pts.green[174].value  = 0;
    post1D->tf_pts.blue[174].value   = 0;
    post1D->tf_pts.red[175].value    = 0;
    post1D->tf_pts.green[175].value  = 0;
    post1D->tf_pts.blue[175].value   = 0;
    post1D->tf_pts.red[176].value    = 0;
    post1D->tf_pts.green[176].value  = 0;
    post1D->tf_pts.blue[176].value   = 0;
    post1D->tf_pts.red[177].value    = 0;
    post1D->tf_pts.green[177].value  = 0;
    post1D->tf_pts.blue[177].value   = 0;
    post1D->tf_pts.red[178].value    = 0;
    post1D->tf_pts.green[178].value  = 0;
    post1D->tf_pts.blue[178].value   = 0;
    post1D->tf_pts.red[179].value    = 0;
    post1D->tf_pts.green[179].value  = 0;
    post1D->tf_pts.blue[179].value   = 0;
    post1D->tf_pts.red[180].value    = 0;
    post1D->tf_pts.green[180].value  = 0;
    post1D->tf_pts.blue[180].value   = 0;
    post1D->tf_pts.red[181].value    = 0;
    post1D->tf_pts.green[181].value  = 0;
    post1D->tf_pts.blue[181].value   = 0;
    post1D->tf_pts.red[182].value    = 0;
    post1D->tf_pts.green[182].value  = 0;
    post1D->tf_pts.blue[182].value   = 0;
    post1D->tf_pts.red[183].value    = 0;
    post1D->tf_pts.green[183].value  = 0;
    post1D->tf_pts.blue[183].value   = 0;
    post1D->tf_pts.red[184].value    = 0;
    post1D->tf_pts.green[184].value  = 0;
    post1D->tf_pts.blue[184].value   = 0;
    post1D->tf_pts.red[185].value    = 0;
    post1D->tf_pts.green[185].value  = 0;
    post1D->tf_pts.blue[185].value   = 0;
    post1D->tf_pts.red[186].value    = 0;
    post1D->tf_pts.green[186].value  = 0;
    post1D->tf_pts.blue[186].value   = 0;
    post1D->tf_pts.red[187].value    = 0;
    post1D->tf_pts.green[187].value  = 0;
    post1D->tf_pts.blue[187].value   = 0;
    post1D->tf_pts.red[188].value    = 0;
    post1D->tf_pts.green[188].value  = 0;
    post1D->tf_pts.blue[188].value   = 0;
    post1D->tf_pts.red[189].value    = 0;
    post1D->tf_pts.green[189].value  = 0;
    post1D->tf_pts.blue[189].value   = 0;
    post1D->tf_pts.red[190].value    = 0;
    post1D->tf_pts.green[190].value  = 0;
    post1D->tf_pts.blue[190].value   = 0;
    post1D->tf_pts.red[191].value    = 0;
    post1D->tf_pts.green[191].value  = 0;
    post1D->tf_pts.blue[191].value   = 0;
    post1D->tf_pts.red[192].value    = 0;
    post1D->tf_pts.green[192].value  = 0;
    post1D->tf_pts.blue[192].value   = 0;
    post1D->tf_pts.red[193].value    = 0;
    post1D->tf_pts.green[193].value  = 0;
    post1D->tf_pts.blue[193].value   = 0;
    post1D->tf_pts.red[194].value    = 0;
    post1D->tf_pts.green[194].value  = 0;
    post1D->tf_pts.blue[194].value   = 0;
    post1D->tf_pts.red[195].value    = 0;
    post1D->tf_pts.green[195].value  = 0;
    post1D->tf_pts.blue[195].value   = 0;
    post1D->tf_pts.red[196].value    = 0;
    post1D->tf_pts.green[196].value  = 0;
    post1D->tf_pts.blue[196].value   = 0;
    post1D->tf_pts.red[197].value    = 0;
    post1D->tf_pts.green[197].value  = 0;
    post1D->tf_pts.blue[197].value   = 0;
    post1D->tf_pts.red[198].value    = 0;
    post1D->tf_pts.green[198].value  = 0;
    post1D->tf_pts.blue[198].value   = 0;
    post1D->tf_pts.red[199].value    = 0;
    post1D->tf_pts.green[199].value  = 0;
    post1D->tf_pts.blue[199].value   = 0;
    post1D->tf_pts.red[200].value    = 0;
    post1D->tf_pts.green[200].value  = 0;
    post1D->tf_pts.blue[200].value   = 0;
    post1D->tf_pts.red[201].value    = 0;
    post1D->tf_pts.green[201].value  = 0;
    post1D->tf_pts.blue[201].value   = 0;
    post1D->tf_pts.red[202].value    = 0;
    post1D->tf_pts.green[202].value  = 0;
    post1D->tf_pts.blue[202].value   = 0;
    post1D->tf_pts.red[203].value    = 0;
    post1D->tf_pts.green[203].value  = 0;
    post1D->tf_pts.blue[203].value   = 0;
    post1D->tf_pts.red[204].value    = 0;
    post1D->tf_pts.green[204].value  = 0;
    post1D->tf_pts.blue[204].value   = 0;
    post1D->tf_pts.red[205].value    = 0;
    post1D->tf_pts.green[205].value  = 0;
    post1D->tf_pts.blue[205].value   = 0;
    post1D->tf_pts.red[206].value    = 0;
    post1D->tf_pts.green[206].value  = 0;
    post1D->tf_pts.blue[206].value   = 0;
    post1D->tf_pts.red[207].value    = 0;
    post1D->tf_pts.green[207].value  = 0;
    post1D->tf_pts.blue[207].value   = 0;
    post1D->tf_pts.red[208].value    = 49;
    post1D->tf_pts.green[208].value  = 49;
    post1D->tf_pts.blue[208].value   = 49;
    post1D->tf_pts.red[209].value    = 55;
    post1D->tf_pts.green[209].value  = 55;
    post1D->tf_pts.blue[209].value   = 55;
    post1D->tf_pts.red[210].value    = 63;
    post1D->tf_pts.green[210].value  = 63;
    post1D->tf_pts.blue[210].value   = 63;
    post1D->tf_pts.red[211].value    = 71;
    post1D->tf_pts.green[211].value  = 71;
    post1D->tf_pts.blue[211].value   = 71;
    post1D->tf_pts.red[212].value    = 79;
    post1D->tf_pts.green[212].value  = 79;
    post1D->tf_pts.blue[212].value   = 79;
    post1D->tf_pts.red[213].value    = 88;
    post1D->tf_pts.green[213].value  = 88;
    post1D->tf_pts.blue[213].value   = 88;
    post1D->tf_pts.red[214].value    = 98;
    post1D->tf_pts.green[214].value  = 98;
    post1D->tf_pts.blue[214].value   = 98;
    post1D->tf_pts.red[215].value    = 108;
    post1D->tf_pts.green[215].value  = 108;
    post1D->tf_pts.blue[215].value   = 108;
    post1D->tf_pts.red[216].value    = 118;
    post1D->tf_pts.green[216].value  = 118;
    post1D->tf_pts.blue[216].value   = 118;
    post1D->tf_pts.red[217].value    = 129;
    post1D->tf_pts.green[217].value  = 129;
    post1D->tf_pts.blue[217].value   = 129;
    post1D->tf_pts.red[218].value    = 141;
    post1D->tf_pts.green[218].value  = 141;
    post1D->tf_pts.blue[218].value   = 141;
    post1D->tf_pts.red[219].value    = 153;
    post1D->tf_pts.green[219].value  = 153;
    post1D->tf_pts.blue[219].value   = 153;
    post1D->tf_pts.red[220].value    = 166;
    post1D->tf_pts.green[220].value  = 166;
    post1D->tf_pts.blue[220].value   = 166;
    post1D->tf_pts.red[221].value    = 179;
    post1D->tf_pts.green[221].value  = 179;
    post1D->tf_pts.blue[221].value   = 179;
    post1D->tf_pts.red[222].value    = 193;
    post1D->tf_pts.green[222].value  = 193;
    post1D->tf_pts.blue[222].value   = 193;
    post1D->tf_pts.red[223].value    = 208;
    post1D->tf_pts.green[223].value  = 208;
    post1D->tf_pts.blue[223].value   = 208;
    post1D->tf_pts.red[224].value    = 223;
    post1D->tf_pts.green[224].value  = 223;
    post1D->tf_pts.blue[224].value   = 223;
    post1D->tf_pts.red[225].value    = 255;
    post1D->tf_pts.green[225].value  = 255;
    post1D->tf_pts.blue[225].value   = 255;
    post1D->tf_pts.red[226].value    = 289;
    post1D->tf_pts.green[226].value  = 289;
    post1D->tf_pts.blue[226].value   = 289;
    post1D->tf_pts.red[227].value    = 325;
    post1D->tf_pts.green[227].value  = 325;
    post1D->tf_pts.blue[227].value   = 325;
    post1D->tf_pts.red[228].value    = 364;
    post1D->tf_pts.green[228].value  = 364;
    post1D->tf_pts.blue[228].value   = 364;
    post1D->tf_pts.red[229].value    = 405;
    post1D->tf_pts.green[229].value  = 405;
    post1D->tf_pts.blue[229].value   = 405;
    post1D->tf_pts.red[230].value    = 449;
    post1D->tf_pts.green[230].value  = 449;
    post1D->tf_pts.blue[230].value   = 449;
    post1D->tf_pts.red[231].value    = 495;
    post1D->tf_pts.green[231].value  = 495;
    post1D->tf_pts.blue[231].value   = 495;
    post1D->tf_pts.red[232].value    = 544;
    post1D->tf_pts.green[232].value  = 544;
    post1D->tf_pts.blue[232].value   = 544;
    post1D->tf_pts.red[233].value    = 595;
    post1D->tf_pts.green[233].value  = 595;
    post1D->tf_pts.blue[233].value   = 595;
    post1D->tf_pts.red[234].value    = 649;
    post1D->tf_pts.green[234].value  = 649;
    post1D->tf_pts.blue[234].value   = 649;
    post1D->tf_pts.red[235].value    = 705;
    post1D->tf_pts.green[235].value  = 705;
    post1D->tf_pts.blue[235].value   = 705;
    post1D->tf_pts.red[236].value    = 763;
    post1D->tf_pts.green[236].value  = 763;
    post1D->tf_pts.blue[236].value   = 763;
    post1D->tf_pts.red[237].value    = 825;
    post1D->tf_pts.green[237].value  = 825;
    post1D->tf_pts.blue[237].value   = 825;
    post1D->tf_pts.red[238].value    = 888;
    post1D->tf_pts.green[238].value  = 888;
    post1D->tf_pts.blue[238].value   = 888;
    post1D->tf_pts.red[239].value    = 955;
    post1D->tf_pts.green[239].value  = 955;
    post1D->tf_pts.blue[239].value   = 955;
    post1D->tf_pts.red[240].value    = 1024;
    post1D->tf_pts.green[240].value  = 1024;
    post1D->tf_pts.blue[240].value   = 1024;
    post1D->tf_pts.red[241].value    = 1170;
    post1D->tf_pts.green[241].value  = 1170;
    post1D->tf_pts.blue[241].value   = 1170;
    post1D->tf_pts.red[242].value    = 1327;
    post1D->tf_pts.green[242].value  = 1327;
    post1D->tf_pts.blue[242].value   = 1327;
    post1D->tf_pts.red[243].value    = 1494;
    post1D->tf_pts.green[243].value  = 1494;
    post1D->tf_pts.blue[243].value   = 1494;
    post1D->tf_pts.red[244].value    = 1673;
    post1D->tf_pts.green[244].value  = 1673;
    post1D->tf_pts.blue[244].value   = 1673;
    post1D->tf_pts.red[245].value    = 1863;
    post1D->tf_pts.green[245].value  = 1863;
    post1D->tf_pts.blue[245].value   = 1863;
    post1D->tf_pts.red[246].value    = 2063;
    post1D->tf_pts.green[246].value  = 2063;
    post1D->tf_pts.blue[246].value   = 2063;
    post1D->tf_pts.red[247].value    = 2275;
    post1D->tf_pts.green[247].value  = 2275;
    post1D->tf_pts.blue[247].value   = 2275;
    post1D->tf_pts.red[248].value    = 2499;
    post1D->tf_pts.green[248].value  = 2499;
    post1D->tf_pts.blue[248].value   = 2499;
    post1D->tf_pts.red[249].value    = 2733;
    post1D->tf_pts.green[249].value  = 2733;
    post1D->tf_pts.blue[249].value   = 2733;
    post1D->tf_pts.red[250].value    = 2980;
    post1D->tf_pts.green[250].value  = 2980;
    post1D->tf_pts.blue[250].value   = 2980;
    post1D->tf_pts.red[251].value    = 3238;
    post1D->tf_pts.green[251].value  = 3238;
    post1D->tf_pts.blue[251].value   = 3238;
    post1D->tf_pts.red[252].value    = 3507;
    post1D->tf_pts.green[252].value  = 3507;
    post1D->tf_pts.blue[252].value   = 3507;
    post1D->tf_pts.red[253].value    = 3789;
    post1D->tf_pts.green[253].value  = 3789;
    post1D->tf_pts.blue[253].value   = 3789;
    post1D->tf_pts.red[254].value    = 4082;
    post1D->tf_pts.green[254].value  = 4082;
    post1D->tf_pts.blue[254].value   = 4082;
    post1D->tf_pts.red[255].value    = 4388;
    post1D->tf_pts.green[255].value  = 4388;
    post1D->tf_pts.blue[255].value   = 4388;
    post1D->tf_pts.red[256].value    = 4705;
    post1D->tf_pts.green[256].value  = 4705;
    post1D->tf_pts.blue[256].value   = 4705;
    post1D->tf_pts.red[257].value    = 5376;
    post1D->tf_pts.green[257].value  = 5376;
    post1D->tf_pts.blue[257].value   = 5376;
    post1D->tf_pts.red[258].value    = 6097;
    post1D->tf_pts.green[258].value  = 6097;
    post1D->tf_pts.blue[258].value   = 6097;
    post1D->tf_pts.red[259].value    = 6867;
    post1D->tf_pts.green[259].value  = 6867;
    post1D->tf_pts.blue[259].value   = 6867;
    post1D->tf_pts.red[260].value    = 7687;
    post1D->tf_pts.green[260].value  = 7687;
    post1D->tf_pts.blue[260].value   = 7687;
    post1D->tf_pts.red[261].value    = 8558;
    post1D->tf_pts.green[261].value  = 8558;
    post1D->tf_pts.blue[261].value   = 8558;
    post1D->tf_pts.red[262].value    = 9481;
    post1D->tf_pts.green[262].value  = 9481;
    post1D->tf_pts.blue[262].value   = 9481;
    post1D->tf_pts.red[263].value    = 10454;
    post1D->tf_pts.green[263].value  = 10454;
    post1D->tf_pts.blue[263].value   = 10454;
    post1D->tf_pts.red[264].value    = 11481;
    post1D->tf_pts.green[264].value  = 11481;
    post1D->tf_pts.blue[264].value   = 11481;
    post1D->tf_pts.red[265].value    = 12559;
    post1D->tf_pts.green[265].value  = 12559;
    post1D->tf_pts.blue[265].value   = 12559;
    post1D->tf_pts.red[266].value    = 13691;
    post1D->tf_pts.green[266].value  = 13691;
    post1D->tf_pts.blue[266].value   = 13691;
    post1D->tf_pts.red[267].value    = 14877;
    post1D->tf_pts.green[267].value  = 14877;
    post1D->tf_pts.blue[267].value   = 14877;
    post1D->tf_pts.red[268].value    = 16116;
    post1D->tf_pts.green[268].value  = 16116;
    post1D->tf_pts.blue[268].value   = 16116;
    post1D->tf_pts.red[269].value    = 17409;
    post1D->tf_pts.green[269].value  = 17409;
    post1D->tf_pts.blue[269].value   = 17409;
    post1D->tf_pts.red[270].value    = 18757;
    post1D->tf_pts.green[270].value  = 18757;
    post1D->tf_pts.blue[270].value   = 18757;
    post1D->tf_pts.red[271].value    = 20160;
    post1D->tf_pts.green[271].value  = 20160;
    post1D->tf_pts.blue[271].value   = 20160;
    post1D->tf_pts.red[272].value    = 21619;
    post1D->tf_pts.green[272].value  = 21619;
    post1D->tf_pts.blue[272].value   = 21619;
    post1D->tf_pts.red[273].value    = 24703;
    post1D->tf_pts.green[273].value  = 24703;
    post1D->tf_pts.blue[273].value   = 24703;
    post1D->tf_pts.red[274].value    = 28014;
    post1D->tf_pts.green[274].value  = 28014;
    post1D->tf_pts.blue[274].value   = 28014;
    post1D->tf_pts.red[275].value    = 31552;
    post1D->tf_pts.green[275].value  = 31552;
    post1D->tf_pts.blue[275].value   = 31552;
    post1D->tf_pts.red[276].value    = 35321;
    post1D->tf_pts.green[276].value  = 35321;
    post1D->tf_pts.blue[276].value   = 35321;
    post1D->tf_pts.red[277].value    = 39323;
    post1D->tf_pts.green[277].value  = 39323;
    post1D->tf_pts.blue[277].value   = 39323;
    post1D->tf_pts.red[278].value    = 43561;
    post1D->tf_pts.green[278].value  = 43561;
    post1D->tf_pts.blue[278].value   = 43561;
    post1D->tf_pts.red[279].value    = 48036;
    post1D->tf_pts.green[279].value  = 48036;
    post1D->tf_pts.blue[279].value   = 48036;
    post1D->tf_pts.red[280].value    = 52751;
    post1D->tf_pts.green[280].value  = 52751;
    post1D->tf_pts.blue[280].value   = 52751;
    post1D->tf_pts.red[281].value    = 57708;
    post1D->tf_pts.green[281].value  = 57708;
    post1D->tf_pts.blue[281].value   = 57708;
    post1D->tf_pts.red[282].value    = 62909;
    post1D->tf_pts.green[282].value  = 62909;
    post1D->tf_pts.blue[282].value   = 62909;
    post1D->tf_pts.red[283].value    = 68355;
    post1D->tf_pts.green[283].value  = 68355;
    post1D->tf_pts.blue[283].value   = 68355;
    post1D->tf_pts.red[284].value    = 74048;
    post1D->tf_pts.green[284].value  = 74048;
    post1D->tf_pts.blue[284].value   = 74048;
    post1D->tf_pts.red[285].value    = 79991;
    post1D->tf_pts.green[285].value  = 79991;
    post1D->tf_pts.blue[285].value   = 79991;
    post1D->tf_pts.red[286].value    = 86186;
    post1D->tf_pts.green[286].value  = 86186;
    post1D->tf_pts.blue[286].value   = 86186;
    post1D->tf_pts.red[287].value    = 92633;
    post1D->tf_pts.green[287].value  = 92633;
    post1D->tf_pts.blue[287].value   = 92633;
    post1D->tf_pts.red[288].value    = 99334;
    post1D->tf_pts.green[288].value  = 99334;
    post1D->tf_pts.blue[288].value   = 99334;
    post1D->tf_pts.red[289].value    = 113507;
    post1D->tf_pts.green[289].value  = 113507;
    post1D->tf_pts.blue[289].value   = 113507;
    post1D->tf_pts.red[290].value    = 128716;
    post1D->tf_pts.green[290].value  = 128716;
    post1D->tf_pts.blue[290].value   = 128716;
    post1D->tf_pts.red[291].value    = 144975;
    post1D->tf_pts.green[291].value  = 144975;
    post1D->tf_pts.blue[291].value   = 144975;
    post1D->tf_pts.red[292].value    = 162293;
    post1D->tf_pts.green[292].value  = 162293;
    post1D->tf_pts.blue[292].value   = 162293;
    post1D->tf_pts.red[293].value    = 180683;
    post1D->tf_pts.green[293].value  = 180683;
    post1D->tf_pts.blue[293].value   = 180683;
    post1D->tf_pts.red[294].value    = 200154;
    post1D->tf_pts.green[294].value  = 200154;
    post1D->tf_pts.blue[294].value   = 200154;
    post1D->tf_pts.red[295].value    = 220717;
    post1D->tf_pts.green[295].value  = 220717;
    post1D->tf_pts.blue[295].value   = 220717;
    post1D->tf_pts.red[296].value    = 242381;
    post1D->tf_pts.green[296].value  = 242381;
    post1D->tf_pts.blue[296].value   = 242381;
    post1D->tf_pts.red[297].value    = 265156;
    post1D->tf_pts.green[297].value  = 265156;
    post1D->tf_pts.blue[297].value   = 265156;
    post1D->tf_pts.red[298].value    = 289052;
    post1D->tf_pts.green[298].value  = 289052;
    post1D->tf_pts.blue[298].value   = 289052;
    post1D->tf_pts.red[299].value    = 314076;
    post1D->tf_pts.green[299].value  = 314076;
    post1D->tf_pts.blue[299].value   = 314076;
    post1D->tf_pts.red[300].value    = 340237;
    post1D->tf_pts.green[300].value  = 340237;
    post1D->tf_pts.blue[300].value   = 340237;
    post1D->tf_pts.red[301].value    = 367544;
    post1D->tf_pts.green[301].value  = 367544;
    post1D->tf_pts.blue[301].value   = 367544;
    post1D->tf_pts.red[302].value    = 396005;
    post1D->tf_pts.green[302].value  = 396005;
    post1D->tf_pts.blue[302].value   = 396005;
    post1D->tf_pts.red[303].value    = 425628;
    post1D->tf_pts.green[303].value  = 425628;
    post1D->tf_pts.blue[303].value   = 425628;
    post1D->tf_pts.red[304].value    = 456419;
    post1D->tf_pts.green[304].value  = 456419;
    post1D->tf_pts.blue[304].value   = 456419;
    post1D->tf_pts.red[305].value    = 521540;
    post1D->tf_pts.green[305].value  = 521540;
    post1D->tf_pts.blue[305].value   = 521540;
    post1D->tf_pts.red[306].value    = 591425;
    post1D->tf_pts.green[306].value  = 591425;
    post1D->tf_pts.blue[306].value   = 591425;
    post1D->tf_pts.red[307].value    = 666128;
    post1D->tf_pts.green[307].value  = 666128;
    post1D->tf_pts.blue[307].value   = 666128;
    post1D->tf_pts.red[308].value    = 745703;
    post1D->tf_pts.green[308].value  = 745703;
    post1D->tf_pts.blue[308].value   = 745703;
    post1D->tf_pts.red[309].value    = 830199;
    post1D->tf_pts.green[309].value  = 830199;
    post1D->tf_pts.blue[309].value   = 830199;
    post1D->tf_pts.red[310].value    = 919665;
    post1D->tf_pts.green[310].value  = 919665;
    post1D->tf_pts.blue[310].value   = 919665;
    post1D->tf_pts.red[311].value    = 1014148;
    post1D->tf_pts.green[311].value  = 1014148;
    post1D->tf_pts.blue[311].value   = 1014148;
    post1D->tf_pts.red[312].value    = 1113691;
    post1D->tf_pts.green[312].value  = 1113691;
    post1D->tf_pts.blue[312].value   = 1113691;
    post1D->tf_pts.red[313].value    = 1218339;
    post1D->tf_pts.green[313].value  = 1218339;
    post1D->tf_pts.blue[313].value   = 1218339;
    post1D->tf_pts.red[314].value    = 1328132;
    post1D->tf_pts.green[314].value  = 1328132;
    post1D->tf_pts.blue[314].value   = 1328132;
    post1D->tf_pts.red[315].value    = 1443113;
    post1D->tf_pts.green[315].value  = 1443113;
    post1D->tf_pts.blue[315].value   = 1443113;
    post1D->tf_pts.red[316].value    = 1563319;
    post1D->tf_pts.green[316].value  = 1563319;
    post1D->tf_pts.blue[316].value   = 1563319;
    post1D->tf_pts.red[317].value    = 1688790;
    post1D->tf_pts.green[317].value  = 1688790;
    post1D->tf_pts.blue[317].value   = 1688790;
    post1D->tf_pts.red[318].value    = 1819561;
    post1D->tf_pts.green[318].value  = 1819561;
    post1D->tf_pts.blue[318].value   = 1819561;
    post1D->tf_pts.red[319].value    = 1955670;
    post1D->tf_pts.green[319].value  = 1955670;
    post1D->tf_pts.blue[319].value   = 1955670;
    post1D->tf_pts.red[320].value    = 2097152;
    post1D->tf_pts.green[320].value  = 2097152;
    post1D->tf_pts.blue[320].value   = 2097152;
    post1D->tf_pts.red[321].value    = 2396368;
    post1D->tf_pts.green[321].value  = 2396368;
    post1D->tf_pts.blue[321].value   = 2396368;
    post1D->tf_pts.red[322].value    = 2717474;
    post1D->tf_pts.green[322].value  = 2717474;
    post1D->tf_pts.blue[322].value   = 2717474;
    post1D->tf_pts.red[323].value    = 3060722;
    post1D->tf_pts.green[323].value  = 3060722;
    post1D->tf_pts.blue[323].value   = 3060722;
    post1D->tf_pts.red[324].value    = 3426352;
    post1D->tf_pts.green[324].value  = 3426352;
    post1D->tf_pts.blue[324].value   = 3426352;
    post1D->tf_pts.red[325].value    = 3814595;
    post1D->tf_pts.green[325].value  = 3814595;
    post1D->tf_pts.blue[325].value   = 3814595;
    post1D->tf_pts.red[326].value    = 4225673;
    post1D->tf_pts.green[326].value  = 4225673;
    post1D->tf_pts.blue[326].value   = 4225673;
    post1D->tf_pts.red[327].value    = 4659799;
    post1D->tf_pts.green[327].value  = 4659799;
    post1D->tf_pts.blue[327].value   = 4659799;
    post1D->tf_pts.red[328].value    = 5117180;
    post1D->tf_pts.green[328].value  = 5117180;
    post1D->tf_pts.blue[328].value   = 5117180;
    post1D->tf_pts.red[329].value    = 5598014;
    post1D->tf_pts.green[329].value  = 5598014;
    post1D->tf_pts.blue[329].value   = 5598014;
    post1D->tf_pts.red[330].value    = 6102493;
    post1D->tf_pts.green[330].value  = 6102493;
    post1D->tf_pts.blue[330].value   = 6102493;
    post1D->tf_pts.red[331].value    = 6630805;
    post1D->tf_pts.green[331].value  = 6630805;
    post1D->tf_pts.blue[331].value   = 6630805;
    post1D->tf_pts.red[332].value    = 7183128;
    post1D->tf_pts.green[332].value  = 7183128;
    post1D->tf_pts.blue[332].value   = 7183128;
    post1D->tf_pts.red[333].value    = 7759639;
    post1D->tf_pts.green[333].value  = 7759639;
    post1D->tf_pts.blue[333].value   = 7759639;
    post1D->tf_pts.red[334].value    = 8360509;
    post1D->tf_pts.green[334].value  = 8360509;
    post1D->tf_pts.blue[334].value   = 8360509;
    post1D->tf_pts.red[335].value    = 8985902;
    post1D->tf_pts.green[335].value  = 8985902;
    post1D->tf_pts.blue[335].value   = 8985902;
    post1D->tf_pts.red[336].value    = 9635980;
    post1D->tf_pts.green[336].value  = 9635980;
    post1D->tf_pts.blue[336].value   = 9635980;
    post1D->tf_pts.red[337].value    = 11010818;
    post1D->tf_pts.green[337].value  = 11010818;
    post1D->tf_pts.blue[337].value   = 11010818;
    post1D->tf_pts.red[338].value    = 12486233;
    post1D->tf_pts.green[338].value  = 12486233;
    post1D->tf_pts.blue[338].value   = 12486233;
    post1D->tf_pts.red[339].value    = 14063385;
    post1D->tf_pts.green[339].value  = 14063385;
    post1D->tf_pts.blue[339].value   = 14063385;
    post1D->tf_pts.red[340].value    = 15743378;
    post1D->tf_pts.green[340].value  = 15743378;
    post1D->tf_pts.blue[340].value   = 15743378;
    post1D->tf_pts.red[341].value    = 17527274;
    post1D->tf_pts.green[341].value  = 17527274;
    post1D->tf_pts.blue[341].value   = 17527274;
    post1D->tf_pts.red[342].value    = 19416093;
    post1D->tf_pts.green[342].value  = 19416093;
    post1D->tf_pts.blue[342].value   = 19416093;
    post1D->tf_pts.red[343].value    = 21410814;
    post1D->tf_pts.green[343].value  = 21410814;
    post1D->tf_pts.blue[343].value   = 21410814;
    post1D->tf_pts.red[344].value    = 23512384;
    post1D->tf_pts.green[344].value  = 23512384;
    post1D->tf_pts.blue[344].value   = 23512384;
    post1D->tf_pts.red[345].value    = 25721717;
    post1D->tf_pts.green[345].value  = 25721717;
    post1D->tf_pts.blue[345].value   = 25721717;
    post1D->tf_pts.red[346].value    = 28039696;
    post1D->tf_pts.green[346].value  = 28039696;
    post1D->tf_pts.blue[346].value   = 28039696;
    post1D->tf_pts.red[347].value    = 30467177;
    post1D->tf_pts.green[347].value  = 30467177;
    post1D->tf_pts.blue[347].value   = 30467177;
    post1D->tf_pts.red[348].value    = 33004990;
    post1D->tf_pts.green[348].value  = 33004990;
    post1D->tf_pts.blue[348].value   = 33004990;
    post1D->tf_pts.red[349].value    = 35653940;
    post1D->tf_pts.green[349].value  = 35653940;
    post1D->tf_pts.blue[349].value   = 35653940;
    post1D->tf_pts.red[350].value    = 38414811;
    post1D->tf_pts.green[350].value  = 38414811;
    post1D->tf_pts.blue[350].value   = 38414811;
    post1D->tf_pts.red[351].value    = 41288363;
    post1D->tf_pts.green[351].value  = 41288363;
    post1D->tf_pts.blue[351].value   = 41288363;
    post1D->tf_pts.red[352].value    = 44275338;
    post1D->tf_pts.green[352].value  = 44275338;
    post1D->tf_pts.blue[352].value   = 44275338;
    post1D->tf_pts.red[353].value    = 50592432;
    post1D->tf_pts.green[353].value  = 50592432;
    post1D->tf_pts.blue[353].value   = 50592432;
    post1D->tf_pts.red[354].value    = 57371663;
    post1D->tf_pts.green[354].value  = 57371663;
    post1D->tf_pts.blue[354].value   = 57371663;
    post1D->tf_pts.red[355].value    = 64618348;
    post1D->tf_pts.green[355].value  = 64618348;
    post1D->tf_pts.blue[355].value   = 64618348;
    post1D->tf_pts.red[356].value    = 72337570;
    post1D->tf_pts.green[356].value  = 72337570;
    post1D->tf_pts.blue[356].value   = 72337570;
    post1D->tf_pts.red[357].value    = 80534205;
    post1D->tf_pts.green[357].value  = 80534205;
    post1D->tf_pts.blue[357].value   = 80534205;
    post1D->tf_pts.red[358].value    = 89212935;
    post1D->tf_pts.green[358].value  = 89212935;
    post1D->tf_pts.blue[358].value   = 89212935;
    post1D->tf_pts.red[359].value    = 98378267;
    post1D->tf_pts.green[359].value  = 98378267;
    post1D->tf_pts.blue[359].value   = 98378267;
    post1D->tf_pts.red[360].value    = 108034548;
    post1D->tf_pts.green[360].value  = 108034548;
    post1D->tf_pts.blue[360].value   = 108034548;
    post1D->tf_pts.red[361].value    = 118185976;
    post1D->tf_pts.green[361].value  = 118185976;
    post1D->tf_pts.blue[361].value   = 118185976;
    post1D->tf_pts.red[362].value    = 128836611;
    post1D->tf_pts.green[362].value  = 128836611;
    post1D->tf_pts.blue[362].value   = 128836611;
    post1D->tf_pts.red[363].value    = 139990385;
    post1D->tf_pts.green[363].value  = 139990385;
    post1D->tf_pts.blue[363].value   = 139990385;
    post1D->tf_pts.red[364].value    = 151651111;
    post1D->tf_pts.green[364].value  = 151651111;
    post1D->tf_pts.blue[364].value   = 151651111;
    post1D->tf_pts.red[365].value    = 163822490;
    post1D->tf_pts.green[365].value  = 163822490;
    post1D->tf_pts.blue[365].value   = 163822490;
    post1D->tf_pts.red[366].value    = 176508120;
    post1D->tf_pts.green[366].value  = 176508120;
    post1D->tf_pts.blue[366].value   = 176508120;
    post1D->tf_pts.red[367].value    = 189711499;
    post1D->tf_pts.green[367].value  = 189711499;
    post1D->tf_pts.blue[367].value   = 189711499;
    post1D->tf_pts.red[368].value    = 203436034;
    post1D->tf_pts.green[368].value  = 203436034;
    post1D->tf_pts.blue[368].value   = 203436034;
    post1D->tf_pts.red[369].value    = 232461773;
    post1D->tf_pts.green[369].value  = 232461773;
    post1D->tf_pts.blue[369].value   = 232461773;
    post1D->tf_pts.red[370].value    = 263610940;
    post1D->tf_pts.green[370].value  = 263610940;
    post1D->tf_pts.blue[370].value   = 263610940;
    post1D->tf_pts.red[371].value    = 296907960;
    post1D->tf_pts.green[371].value  = 296907960;
    post1D->tf_pts.blue[371].value   = 296907960;
    post1D->tf_pts.red[372].value    = 332376193;
    post1D->tf_pts.green[372].value  = 332376193;
    post1D->tf_pts.blue[372].value   = 332376193;
    post1D->tf_pts.red[373].value    = 370038035;
    post1D->tf_pts.green[373].value  = 370038035;
    post1D->tf_pts.blue[373].value   = 370038035;
    post1D->tf_pts.red[374].value    = 409915005;
    post1D->tf_pts.green[374].value  = 409915005;
    post1D->tf_pts.blue[374].value   = 409915005;
    post1D->tf_pts.red[375].value    = 452027813;
    post1D->tf_pts.green[375].value  = 452027813;
    post1D->tf_pts.blue[375].value   = 452027813;
    post1D->tf_pts.red[376].value    = 496396431;
    post1D->tf_pts.green[376].value  = 496396431;
    post1D->tf_pts.blue[376].value   = 496396431;
    post1D->tf_pts.red[377].value    = 543040146;
    post1D->tf_pts.green[377].value  = 543040146;
    post1D->tf_pts.blue[377].value   = 543040146;
    post1D->tf_pts.red[378].value    = 591977614;
    post1D->tf_pts.green[378].value  = 591977614;
    post1D->tf_pts.blue[378].value   = 591977614;
    post1D->tf_pts.red[379].value    = 643226902;
    post1D->tf_pts.green[379].value  = 643226902;
    post1D->tf_pts.blue[379].value   = 643226902;
    post1D->tf_pts.red[380].value    = 696805528;
    post1D->tf_pts.green[380].value  = 696805528;
    post1D->tf_pts.blue[380].value   = 696805528;
    post1D->tf_pts.red[381].value    = 752730501;
    post1D->tf_pts.green[381].value  = 752730501;
    post1D->tf_pts.blue[381].value   = 752730501;
    post1D->tf_pts.red[382].value    = 811018347;
    post1D->tf_pts.green[382].value  = 811018347;
    post1D->tf_pts.blue[382].value   = 811018347;
    post1D->tf_pts.red[383].value    = 871685145;
    post1D->tf_pts.green[383].value  = 871685145;
    post1D->tf_pts.blue[383].value   = 871685145;
    post1D->tf_pts.red[384].value    = 934746550;
    post1D->tf_pts.green[384].value  = 934746550;
    post1D->tf_pts.blue[384].value   = 934746550;
    post1D->tf_pts.red[385].value    = 1068113823;
    post1D->tf_pts.green[385].value  = 1068113823;
    post1D->tf_pts.blue[385].value   = 1068113823;
    post1D->tf_pts.red[386].value    = 1211237812;
    post1D->tf_pts.green[386].value  = 1211237812;
    post1D->tf_pts.blue[386].value   = 1211237812;
    post1D->tf_pts.red[387].value    = 1364230740;
    post1D->tf_pts.green[387].value  = 1364230740;
    post1D->tf_pts.blue[387].value   = 1364230740;
    post1D->tf_pts.red[388].value    = 1527199943;
    post1D->tf_pts.green[388].value  = 1527199943;
    post1D->tf_pts.blue[388].value   = 1527199943;
    post1D->tf_pts.red[389].value    = 1700248331;
    post1D->tf_pts.green[389].value  = 1700248331;
    post1D->tf_pts.blue[389].value   = 1700248331;
    post1D->tf_pts.red[390].value    = 1883474769;
    post1D->tf_pts.green[390].value  = 1883474769;
    post1D->tf_pts.blue[390].value   = 1883474769;
    post1D->tf_pts.red[391].value    = 2076974422;
    post1D->tf_pts.green[391].value  = 2076974422;
    post1D->tf_pts.blue[391].value   = 2076974422;
    post1D->tf_pts.red[392].value    = 2280839055;
    post1D->tf_pts.green[392].value  = 2280839055;
    post1D->tf_pts.blue[392].value   = 2280839055;
    post1D->tf_pts.red[393].value    = 2495157291;
    post1D->tf_pts.green[393].value  = 2495157291;
    post1D->tf_pts.blue[393].value   = 2495157291;
    post1D->tf_pts.red[394].value    = 2720014848;
    post1D->tf_pts.green[394].value  = 2720014848;
    post1D->tf_pts.blue[394].value   = 2720014848;
    post1D->tf_pts.red[395].value    = 2955494736;
    post1D->tf_pts.green[395].value  = 2955494736;
    post1D->tf_pts.blue[395].value   = 2955494736;
    post1D->tf_pts.red[396].value    = 3201677455;
    post1D->tf_pts.green[396].value  = 3201677455;
    post1D->tf_pts.blue[396].value   = 3201677455;
    post1D->tf_pts.red[397].value    = 3458641150;
    post1D->tf_pts.green[397].value  = 3458641150;
    post1D->tf_pts.blue[397].value   = 3458641150;
    post1D->tf_pts.red[398].value    = 3726461762;
    post1D->tf_pts.green[398].value  = 3726461762;
    post1D->tf_pts.blue[398].value   = 3726461762;
    post1D->tf_pts.red[399].value    = 4005213167;
    post1D->tf_pts.green[399].value  = 4005213167;
    post1D->tf_pts.blue[399].value   = 4005213167;
    post1D->tf_pts.red[400].value    = 4294967296;
    post1D->tf_pts.green[400].value  = 4294967296;
    post1D->tf_pts.blue[400].value   = 4294967296;
    post1D->tf_pts.red[401].value    = 4294967296;
    post1D->tf_pts.green[401].value  = 4294967296;
    post1D->tf_pts.blue[401].value   = 4294967296;
    post1D->tf_pts.red[402].value    = 4294967296;
    post1D->tf_pts.green[402].value  = 4294967296;
    post1D->tf_pts.blue[402].value   = 4294967296;
    post1D->tf_pts.red[403].value    = 4294967296;
    post1D->tf_pts.green[403].value  = 4294967296;
    post1D->tf_pts.blue[403].value   = 4294967296;
    post1D->tf_pts.red[404].value    = 4294967296;
    post1D->tf_pts.green[404].value  = 4294967296;
    post1D->tf_pts.blue[404].value   = 4294967296;
    post1D->tf_pts.red[405].value    = 4294967296;
    post1D->tf_pts.green[405].value  = 4294967296;
    post1D->tf_pts.blue[405].value   = 4294967296;
    post1D->tf_pts.red[406].value    = 4294967296;
    post1D->tf_pts.green[406].value  = 4294967296;
    post1D->tf_pts.blue[406].value   = 4294967296;
    post1D->tf_pts.red[407].value    = 4294967296;
    post1D->tf_pts.green[407].value  = 4294967296;
    post1D->tf_pts.blue[407].value   = 4294967296;
    post1D->tf_pts.red[408].value    = 4294967296;
    post1D->tf_pts.green[408].value  = 4294967296;
    post1D->tf_pts.blue[408].value   = 4294967296;
    post1D->tf_pts.red[409].value    = 4294967296;
    post1D->tf_pts.green[409].value  = 4294967296;
    post1D->tf_pts.blue[409].value   = 4294967296;
    post1D->tf_pts.red[410].value    = 4294967296;
    post1D->tf_pts.green[410].value  = 4294967296;
    post1D->tf_pts.blue[410].value   = 4294967296;
    post1D->tf_pts.red[411].value    = 4294967296;
    post1D->tf_pts.green[411].value  = 4294967296;
    post1D->tf_pts.blue[411].value   = 4294967296;
    post1D->tf_pts.red[412].value    = 4294967296;
    post1D->tf_pts.green[412].value  = 4294967296;
    post1D->tf_pts.blue[412].value   = 4294967296;
    post1D->tf_pts.red[413].value    = 4294967296;
    post1D->tf_pts.green[413].value  = 4294967296;
    post1D->tf_pts.blue[413].value   = 4294967296;
    post1D->tf_pts.red[414].value    = 4294967296;
    post1D->tf_pts.green[414].value  = 4294967296;
    post1D->tf_pts.blue[414].value   = 4294967296;
    post1D->tf_pts.red[415].value    = 4294967296;
    post1D->tf_pts.green[415].value  = 4294967296;
    post1D->tf_pts.blue[415].value   = 4294967296;
    post1D->tf_pts.red[416].value    = 4294967296;
    post1D->tf_pts.green[416].value  = 4294967296;
    post1D->tf_pts.blue[416].value   = 4294967296;
    post1D->tf_pts.red[417].value    = 4294967296;
    post1D->tf_pts.green[417].value  = 4294967296;
    post1D->tf_pts.blue[417].value   = 4294967296;
    post1D->tf_pts.red[418].value    = 4294967296;
    post1D->tf_pts.green[418].value  = 4294967296;
    post1D->tf_pts.blue[418].value   = 4294967296;
    post1D->tf_pts.red[419].value    = 4294967296;
    post1D->tf_pts.green[419].value  = 4294967296;
    post1D->tf_pts.blue[419].value   = 4294967296;
    post1D->tf_pts.red[420].value    = 4294967296;
    post1D->tf_pts.green[420].value  = 4294967296;
    post1D->tf_pts.blue[420].value   = 4294967296;
    post1D->tf_pts.red[421].value    = 4294967296;
    post1D->tf_pts.green[421].value  = 4294967296;
    post1D->tf_pts.blue[421].value   = 4294967296;
    post1D->tf_pts.red[422].value    = 4294967296;
    post1D->tf_pts.green[422].value  = 4294967296;
    post1D->tf_pts.blue[422].value   = 4294967296;
    post1D->tf_pts.red[423].value    = 4294967296;
    post1D->tf_pts.green[423].value  = 4294967296;
    post1D->tf_pts.blue[423].value   = 4294967296;
    post1D->tf_pts.red[424].value    = 4294967296;
    post1D->tf_pts.green[424].value  = 4294967296;
    post1D->tf_pts.blue[424].value   = 4294967296;
    post1D->tf_pts.red[425].value    = 4294967296;
    post1D->tf_pts.green[425].value  = 4294967296;
    post1D->tf_pts.blue[425].value   = 4294967296;
    post1D->tf_pts.red[426].value    = 4294967296;
    post1D->tf_pts.green[426].value  = 4294967296;
    post1D->tf_pts.blue[426].value   = 4294967296;
    post1D->tf_pts.red[427].value    = 4294967296;
    post1D->tf_pts.green[427].value  = 4294967296;
    post1D->tf_pts.blue[427].value   = 4294967296;
    post1D->tf_pts.red[428].value    = 4294967296;
    post1D->tf_pts.green[428].value  = 4294967296;
    post1D->tf_pts.blue[428].value   = 4294967296;
    post1D->tf_pts.red[429].value    = 4294967296;
    post1D->tf_pts.green[429].value  = 4294967296;
    post1D->tf_pts.blue[429].value   = 4294967296;
    post1D->tf_pts.red[430].value    = 4294967296;
    post1D->tf_pts.green[430].value  = 4294967296;
    post1D->tf_pts.blue[430].value   = 4294967296;
    post1D->tf_pts.red[431].value    = 4294967296;
    post1D->tf_pts.green[431].value  = 4294967296;
    post1D->tf_pts.blue[431].value   = 4294967296;
    post1D->tf_pts.red[432].value    = 4294967296;
    post1D->tf_pts.green[432].value  = 4294967296;
    post1D->tf_pts.blue[432].value   = 4294967296;
    post1D->tf_pts.red[433].value    = 4294967296;
    post1D->tf_pts.green[433].value  = 4294967296;
    post1D->tf_pts.blue[433].value   = 4294967296;
    post1D->tf_pts.red[434].value    = 4294967296;
    post1D->tf_pts.green[434].value  = 4294967296;
    post1D->tf_pts.blue[434].value   = 4294967296;
    post1D->tf_pts.red[435].value    = 4294967296;
    post1D->tf_pts.green[435].value  = 4294967296;
    post1D->tf_pts.blue[435].value   = 4294967296;
    post1D->tf_pts.red[436].value    = 4294967296;
    post1D->tf_pts.green[436].value  = 4294967296;
    post1D->tf_pts.blue[436].value   = 4294967296;
    post1D->tf_pts.red[437].value    = 4294967296;
    post1D->tf_pts.green[437].value  = 4294967296;
    post1D->tf_pts.blue[437].value   = 4294967296;
    post1D->tf_pts.red[438].value    = 4294967296;
    post1D->tf_pts.green[438].value  = 4294967296;
    post1D->tf_pts.blue[438].value   = 4294967296;
    post1D->tf_pts.red[439].value    = 4294967296;
    post1D->tf_pts.green[439].value  = 4294967296;
    post1D->tf_pts.blue[439].value   = 4294967296;
    post1D->tf_pts.red[440].value    = 4294967296;
    post1D->tf_pts.green[440].value  = 4294967296;
    post1D->tf_pts.blue[440].value   = 4294967296;
    post1D->tf_pts.red[441].value    = 4294967296;
    post1D->tf_pts.green[441].value  = 4294967296;
    post1D->tf_pts.blue[441].value   = 4294967296;
    post1D->tf_pts.red[442].value    = 4294967296;
    post1D->tf_pts.green[442].value  = 4294967296;
    post1D->tf_pts.blue[442].value   = 4294967296;
    post1D->tf_pts.red[443].value    = 4294967296;
    post1D->tf_pts.green[443].value  = 4294967296;
    post1D->tf_pts.blue[443].value   = 4294967296;
    post1D->tf_pts.red[444].value    = 4294967296;
    post1D->tf_pts.green[444].value  = 4294967296;
    post1D->tf_pts.blue[444].value   = 4294967296;
    post1D->tf_pts.red[445].value    = 4294967296;
    post1D->tf_pts.green[445].value  = 4294967296;
    post1D->tf_pts.blue[445].value   = 4294967296;
    post1D->tf_pts.red[446].value    = 4294967296;
    post1D->tf_pts.green[446].value  = 4294967296;
    post1D->tf_pts.blue[446].value   = 4294967296;
    post1D->tf_pts.red[447].value    = 4294967296;
    post1D->tf_pts.green[447].value  = 4294967296;
    post1D->tf_pts.blue[447].value   = 4294967296;
    post1D->tf_pts.red[448].value    = 4294967296;
    post1D->tf_pts.green[448].value  = 4294967296;
    post1D->tf_pts.blue[448].value   = 4294967296;
    post1D->tf_pts.red[449].value    = 4294967296;
    post1D->tf_pts.green[449].value  = 4294967296;
    post1D->tf_pts.blue[449].value   = 4294967296;
    post1D->tf_pts.red[450].value    = 4294967296;
    post1D->tf_pts.green[450].value  = 4294967296;
    post1D->tf_pts.blue[450].value   = 4294967296;
    post1D->tf_pts.red[451].value    = 4294967296;
    post1D->tf_pts.green[451].value  = 4294967296;
    post1D->tf_pts.blue[451].value   = 4294967296;
    post1D->tf_pts.red[452].value    = 4294967296;
    post1D->tf_pts.green[452].value  = 4294967296;
    post1D->tf_pts.blue[452].value   = 4294967296;
    post1D->tf_pts.red[453].value    = 4294967296;
    post1D->tf_pts.green[453].value  = 4294967296;
    post1D->tf_pts.blue[453].value   = 4294967296;
    post1D->tf_pts.red[454].value    = 4294967296;
    post1D->tf_pts.green[454].value  = 4294967296;
    post1D->tf_pts.blue[454].value   = 4294967296;
    post1D->tf_pts.red[455].value    = 4294967296;
    post1D->tf_pts.green[455].value  = 4294967296;
    post1D->tf_pts.blue[455].value   = 4294967296;
    post1D->tf_pts.red[456].value    = 4294967296;
    post1D->tf_pts.green[456].value  = 4294967296;
    post1D->tf_pts.blue[456].value   = 4294967296;
    post1D->tf_pts.red[457].value    = 4294967296;
    post1D->tf_pts.green[457].value  = 4294967296;
    post1D->tf_pts.blue[457].value   = 4294967296;
    post1D->tf_pts.red[458].value    = 4294967296;
    post1D->tf_pts.green[458].value  = 4294967296;
    post1D->tf_pts.blue[458].value   = 4294967296;
    post1D->tf_pts.red[459].value    = 4294967296;
    post1D->tf_pts.green[459].value  = 4294967296;
    post1D->tf_pts.blue[459].value   = 4294967296;
    post1D->tf_pts.red[460].value    = 4294967296;
    post1D->tf_pts.green[460].value  = 4294967296;
    post1D->tf_pts.blue[460].value   = 4294967296;
    post1D->tf_pts.red[461].value    = 4294967296;
    post1D->tf_pts.green[461].value  = 4294967296;
    post1D->tf_pts.blue[461].value   = 4294967296;
    post1D->tf_pts.red[462].value    = 4294967296;
    post1D->tf_pts.green[462].value  = 4294967296;
    post1D->tf_pts.blue[462].value   = 4294967296;
    post1D->tf_pts.red[463].value    = 4294967296;
    post1D->tf_pts.green[463].value  = 4294967296;
    post1D->tf_pts.blue[463].value   = 4294967296;
    post1D->tf_pts.red[464].value    = 4294967296;
    post1D->tf_pts.green[464].value  = 4294967296;
    post1D->tf_pts.blue[464].value   = 4294967296;
    post1D->tf_pts.red[465].value    = 4294967296;
    post1D->tf_pts.green[465].value  = 4294967296;
    post1D->tf_pts.blue[465].value   = 4294967296;
    post1D->tf_pts.red[466].value    = 4294967296;
    post1D->tf_pts.green[466].value  = 4294967296;
    post1D->tf_pts.blue[466].value   = 4294967296;
    post1D->tf_pts.red[467].value    = 4294967296;
    post1D->tf_pts.green[467].value  = 4294967296;
    post1D->tf_pts.blue[467].value   = 4294967296;
    post1D->tf_pts.red[468].value    = 4294967296;
    post1D->tf_pts.green[468].value  = 4294967296;
    post1D->tf_pts.blue[468].value   = 4294967296;
    post1D->tf_pts.red[469].value    = 4294967296;
    post1D->tf_pts.green[469].value  = 4294967296;
    post1D->tf_pts.blue[469].value   = 4294967296;
    post1D->tf_pts.red[470].value    = 4294967296;
    post1D->tf_pts.green[470].value  = 4294967296;
    post1D->tf_pts.blue[470].value   = 4294967296;
    post1D->tf_pts.red[471].value    = 4294967296;
    post1D->tf_pts.green[471].value  = 4294967296;
    post1D->tf_pts.blue[471].value   = 4294967296;
    post1D->tf_pts.red[472].value    = 4294967296;
    post1D->tf_pts.green[472].value  = 4294967296;
    post1D->tf_pts.blue[472].value   = 4294967296;
    post1D->tf_pts.red[473].value    = 4294967296;
    post1D->tf_pts.green[473].value  = 4294967296;
    post1D->tf_pts.blue[473].value   = 4294967296;
    post1D->tf_pts.red[474].value    = 4294967296;
    post1D->tf_pts.green[474].value  = 4294967296;
    post1D->tf_pts.blue[474].value   = 4294967296;
    post1D->tf_pts.red[475].value    = 4294967296;
    post1D->tf_pts.green[475].value  = 4294967296;
    post1D->tf_pts.blue[475].value   = 4294967296;
    post1D->tf_pts.red[476].value    = 4294967296;
    post1D->tf_pts.green[476].value  = 4294967296;
    post1D->tf_pts.blue[476].value   = 4294967296;
    post1D->tf_pts.red[477].value    = 4294967296;
    post1D->tf_pts.green[477].value  = 4294967296;
    post1D->tf_pts.blue[477].value   = 4294967296;
    post1D->tf_pts.red[478].value    = 4294967296;
    post1D->tf_pts.green[478].value  = 4294967296;
    post1D->tf_pts.blue[478].value   = 4294967296;
    post1D->tf_pts.red[479].value    = 4294967296;
    post1D->tf_pts.green[479].value  = 4294967296;
    post1D->tf_pts.blue[479].value   = 4294967296;
    post1D->tf_pts.red[480].value    = 4294967296;
    post1D->tf_pts.green[480].value  = 4294967296;
    post1D->tf_pts.blue[480].value   = 4294967296;
    post1D->tf_pts.red[481].value    = 4294967296;
    post1D->tf_pts.green[481].value  = 4294967296;
    post1D->tf_pts.blue[481].value   = 4294967296;
    post1D->tf_pts.red[482].value    = 4294967296;
    post1D->tf_pts.green[482].value  = 4294967296;
    post1D->tf_pts.blue[482].value   = 4294967296;
    post1D->tf_pts.red[483].value    = 4294967296;
    post1D->tf_pts.green[483].value  = 4294967296;
    post1D->tf_pts.blue[483].value   = 4294967296;
    post1D->tf_pts.red[484].value    = 4294967296;
    post1D->tf_pts.green[484].value  = 4294967296;
    post1D->tf_pts.blue[484].value   = 4294967296;
    post1D->tf_pts.red[485].value    = 4294967296;
    post1D->tf_pts.green[485].value  = 4294967296;
    post1D->tf_pts.blue[485].value   = 4294967296;
    post1D->tf_pts.red[486].value    = 4294967296;
    post1D->tf_pts.green[486].value  = 4294967296;
    post1D->tf_pts.blue[486].value   = 4294967296;
    post1D->tf_pts.red[487].value    = 4294967296;
    post1D->tf_pts.green[487].value  = 4294967296;
    post1D->tf_pts.blue[487].value   = 4294967296;
    post1D->tf_pts.red[488].value    = 4294967296;
    post1D->tf_pts.green[488].value  = 4294967296;
    post1D->tf_pts.blue[488].value   = 4294967296;
    post1D->tf_pts.red[489].value    = 4294967296;
    post1D->tf_pts.green[489].value  = 4294967296;
    post1D->tf_pts.blue[489].value   = 4294967296;
    post1D->tf_pts.red[490].value    = 4294967296;
    post1D->tf_pts.green[490].value  = 4294967296;
    post1D->tf_pts.blue[490].value   = 4294967296;
    post1D->tf_pts.red[491].value    = 4294967296;
    post1D->tf_pts.green[491].value  = 4294967296;
    post1D->tf_pts.blue[491].value   = 4294967296;
    post1D->tf_pts.red[492].value    = 4294967296;
    post1D->tf_pts.green[492].value  = 4294967296;
    post1D->tf_pts.blue[492].value   = 4294967296;
    post1D->tf_pts.red[493].value    = 4294967296;
    post1D->tf_pts.green[493].value  = 4294967296;
    post1D->tf_pts.blue[493].value   = 4294967296;
    post1D->tf_pts.red[494].value    = 4294967296;
    post1D->tf_pts.green[494].value  = 4294967296;
    post1D->tf_pts.blue[494].value   = 4294967296;
    post1D->tf_pts.red[495].value    = 4294967296;
    post1D->tf_pts.green[495].value  = 4294967296;
    post1D->tf_pts.blue[495].value   = 4294967296;
    post1D->tf_pts.red[496].value    = 4294967296;
    post1D->tf_pts.green[496].value  = 4294967296;
    post1D->tf_pts.blue[496].value   = 4294967296;
    post1D->tf_pts.red[497].value    = 4294967296;
    post1D->tf_pts.green[497].value  = 4294967296;
    post1D->tf_pts.blue[497].value   = 4294967296;
    post1D->tf_pts.red[498].value    = 4294967296;
    post1D->tf_pts.green[498].value  = 4294967296;
    post1D->tf_pts.blue[498].value   = 4294967296;
    post1D->tf_pts.red[499].value    = 4294967296;
    post1D->tf_pts.green[499].value  = 4294967296;
    post1D->tf_pts.blue[499].value   = 4294967296;
    post1D->tf_pts.red[500].value    = 4294967296;
    post1D->tf_pts.green[500].value  = 4294967296;
    post1D->tf_pts.blue[500].value   = 4294967296;
    post1D->tf_pts.red[501].value    = 4294967296;
    post1D->tf_pts.green[501].value  = 4294967296;
    post1D->tf_pts.blue[501].value   = 4294967296;
    post1D->tf_pts.red[502].value    = 4294967296;
    post1D->tf_pts.green[502].value  = 4294967296;
    post1D->tf_pts.blue[502].value   = 4294967296;
    post1D->tf_pts.red[503].value    = 4294967296;
    post1D->tf_pts.green[503].value  = 4294967296;
    post1D->tf_pts.blue[503].value   = 4294967296;
    post1D->tf_pts.red[504].value    = 4294967296;
    post1D->tf_pts.green[504].value  = 4294967296;
    post1D->tf_pts.blue[504].value   = 4294967296;
    post1D->tf_pts.red[505].value    = 4294967296;
    post1D->tf_pts.green[505].value  = 4294967296;
    post1D->tf_pts.blue[505].value   = 4294967296;
    post1D->tf_pts.red[506].value    = 4294967296;
    post1D->tf_pts.green[506].value  = 4294967296;
    post1D->tf_pts.blue[506].value   = 4294967296;
    post1D->tf_pts.red[507].value    = 4294967296;
    post1D->tf_pts.green[507].value  = 4294967296;
    post1D->tf_pts.blue[507].value   = 4294967296;
    post1D->tf_pts.red[508].value    = 4294967296;
    post1D->tf_pts.green[508].value  = 4294967296;
    post1D->tf_pts.blue[508].value   = 4294967296;
    post1D->tf_pts.red[509].value    = 4294967296;
    post1D->tf_pts.green[509].value  = 4294967296;
    post1D->tf_pts.blue[509].value   = 4294967296;
    post1D->tf_pts.red[510].value    = 4294967296;
    post1D->tf_pts.green[510].value  = 4294967296;
    post1D->tf_pts.blue[510].value   = 4294967296;
    post1D->tf_pts.red[511].value    = 4294967296;
    post1D->tf_pts.green[511].value  = 4294967296;
    post1D->tf_pts.blue[511].value   = 4294967296;
    post1D->tf_pts.red[512].value    = 4294967296;
    post1D->tf_pts.green[512].value  = 4294967296;
    post1D->tf_pts.blue[512].value   = 4294967296;
    post1D->tf_pts.red[513].value    = 0;
    post1D->tf_pts.green[513].value  = 0;
    post1D->tf_pts.blue[513].value   = 0;
    post1D->tf_pts.red[514].value    = 0;
    post1D->tf_pts.green[514].value  = 0;
    post1D->tf_pts.blue[514].value   = 0;
    post1D->tf_pts.red[515].value    = 0;
    post1D->tf_pts.green[515].value  = 0;
    post1D->tf_pts.blue[515].value   = 0;
    post1D->tf_pts.red[516].value    = 0;
    post1D->tf_pts.green[516].value  = 0;
    post1D->tf_pts.blue[516].value   = 0;
    post1D->tf_pts.red[517].value    = 0;
    post1D->tf_pts.green[517].value  = 0;
    post1D->tf_pts.blue[517].value   = 0;
    post1D->tf_pts.red[518].value    = 0;
    post1D->tf_pts.green[518].value  = 0;
    post1D->tf_pts.blue[518].value   = 0;
    post1D->tf_pts.red[519].value    = 0;
    post1D->tf_pts.green[519].value  = 0;
    post1D->tf_pts.blue[519].value   = 0;
    post1D->tf_pts.red[520].value    = 0;
    post1D->tf_pts.green[520].value  = 0;
    post1D->tf_pts.blue[520].value   = 0;
    post1D->tf_pts.red[521].value    = 0;
    post1D->tf_pts.green[521].value  = 0;
    post1D->tf_pts.blue[521].value   = 0;
    post1D->tf_pts.red[522].value    = 0;
    post1D->tf_pts.green[522].value  = 0;
    post1D->tf_pts.blue[522].value   = 0;
    post1D->tf_pts.red[523].value    = 0;
    post1D->tf_pts.green[523].value  = 0;
    post1D->tf_pts.blue[523].value   = 0;
    post1D->tf_pts.red[524].value    = 0;
    post1D->tf_pts.green[524].value  = 0;
    post1D->tf_pts.blue[524].value   = 0;
    post1D->tf_pts.red[525].value    = 0;
    post1D->tf_pts.green[525].value  = 0;
    post1D->tf_pts.blue[525].value   = 0;
    post1D->tf_pts.red[526].value    = 0;
    post1D->tf_pts.green[526].value  = 0;
    post1D->tf_pts.blue[526].value   = 0;
    post1D->tf_pts.red[527].value    = 0;
    post1D->tf_pts.green[527].value  = 0;
    post1D->tf_pts.blue[527].value   = 0;
    post1D->tf_pts.red[528].value    = 0;
    post1D->tf_pts.green[528].value  = 0;
    post1D->tf_pts.blue[528].value   = 0;
    post1D->tf_pts.red[529].value    = 0;
    post1D->tf_pts.green[529].value  = 0;
    post1D->tf_pts.blue[529].value   = 0;
    post1D->tf_pts.red[530].value    = 0;
    post1D->tf_pts.green[530].value  = 0;
    post1D->tf_pts.blue[530].value   = 0;
    post1D->tf_pts.red[531].value    = 0;
    post1D->tf_pts.green[531].value  = 0;
    post1D->tf_pts.blue[531].value   = 0;
    post1D->tf_pts.red[532].value    = 0;
    post1D->tf_pts.green[532].value  = 0;
    post1D->tf_pts.blue[532].value   = 0;
    post1D->tf_pts.red[533].value    = 0;
    post1D->tf_pts.green[533].value  = 0;
    post1D->tf_pts.blue[533].value   = 0;
    post1D->tf_pts.red[534].value    = 0;
    post1D->tf_pts.green[534].value  = 0;
    post1D->tf_pts.blue[534].value   = 0;
    post1D->tf_pts.red[535].value    = 0;
    post1D->tf_pts.green[535].value  = 0;
    post1D->tf_pts.blue[535].value   = 0;
    post1D->tf_pts.red[536].value    = 0;
    post1D->tf_pts.green[536].value  = 0;
    post1D->tf_pts.blue[536].value   = 0;
    post1D->tf_pts.red[537].value    = 0;
    post1D->tf_pts.green[537].value  = 0;
    post1D->tf_pts.blue[537].value   = 0;
    post1D->tf_pts.red[538].value    = 0;
    post1D->tf_pts.green[538].value  = 0;
    post1D->tf_pts.blue[538].value   = 0;
    post1D->tf_pts.red[539].value    = 0;
    post1D->tf_pts.green[539].value  = 0;
    post1D->tf_pts.blue[539].value   = 0;
    post1D->tf_pts.red[540].value    = 0;
    post1D->tf_pts.green[540].value  = 0;
    post1D->tf_pts.blue[540].value   = 0;
    post1D->tf_pts.red[541].value    = 0;
    post1D->tf_pts.green[541].value  = 0;
    post1D->tf_pts.blue[541].value   = 0;
    post1D->tf_pts.red[542].value    = 0;
    post1D->tf_pts.green[542].value  = 0;
    post1D->tf_pts.blue[542].value   = 0;
    post1D->tf_pts.red[543].value    = 0;
    post1D->tf_pts.green[543].value  = 0;
    post1D->tf_pts.blue[543].value   = 0;
    post1D->tf_pts.red[544].value    = 0;
    post1D->tf_pts.green[544].value  = 0;
    post1D->tf_pts.blue[544].value   = 0;
    post1D->tf_pts.red[545].value    = 0;
    post1D->tf_pts.green[545].value  = 0;
    post1D->tf_pts.blue[545].value   = 0;
    post1D->tf_pts.red[546].value    = 0;
    post1D->tf_pts.green[546].value  = 0;
    post1D->tf_pts.blue[546].value   = 0;
    post1D->tf_pts.red[547].value    = 0;
    post1D->tf_pts.green[547].value  = 0;
    post1D->tf_pts.blue[547].value   = 0;
    post1D->tf_pts.red[548].value    = 0;
    post1D->tf_pts.green[548].value  = 0;
    post1D->tf_pts.blue[548].value   = 0;
    post1D->tf_pts.red[549].value    = 0;
    post1D->tf_pts.green[549].value  = 0;
    post1D->tf_pts.blue[549].value   = 0;
    post1D->tf_pts.red[550].value    = 0;
    post1D->tf_pts.green[550].value  = 0;
    post1D->tf_pts.blue[550].value   = 0;
    post1D->tf_pts.red[551].value    = 0;
    post1D->tf_pts.green[551].value  = 0;
    post1D->tf_pts.blue[551].value   = 0;
    post1D->tf_pts.red[552].value    = 0;
    post1D->tf_pts.green[552].value  = 0;
    post1D->tf_pts.blue[552].value   = 0;
    post1D->tf_pts.red[553].value    = 0;
    post1D->tf_pts.green[553].value  = 0;
    post1D->tf_pts.blue[553].value   = 0;
    post1D->tf_pts.red[554].value    = 0;
    post1D->tf_pts.green[554].value  = 0;
    post1D->tf_pts.blue[554].value   = 0;
    post1D->tf_pts.red[555].value    = 0;
    post1D->tf_pts.green[555].value  = 0;
    post1D->tf_pts.blue[555].value   = 0;
    post1D->tf_pts.red[556].value    = 0;
    post1D->tf_pts.green[556].value  = 0;
    post1D->tf_pts.blue[556].value   = 0;
    post1D->tf_pts.red[557].value    = 0;
    post1D->tf_pts.green[557].value  = 0;
    post1D->tf_pts.blue[557].value   = 0;
    post1D->tf_pts.red[558].value    = 0;
    post1D->tf_pts.green[558].value  = 0;
    post1D->tf_pts.blue[558].value   = 0;
    post1D->tf_pts.red[559].value    = 0;
    post1D->tf_pts.green[559].value  = 0;
    post1D->tf_pts.blue[559].value   = 0;
    post1D->tf_pts.red[560].value    = 0;
    post1D->tf_pts.green[560].value  = 0;
    post1D->tf_pts.blue[560].value   = 0;
    post1D->tf_pts.red[561].value    = 0;
    post1D->tf_pts.green[561].value  = 0;
    post1D->tf_pts.blue[561].value   = 0;
    post1D->tf_pts.red[562].value    = 0;
    post1D->tf_pts.green[562].value  = 0;
    post1D->tf_pts.blue[562].value   = 0;
    post1D->tf_pts.red[563].value    = 0;
    post1D->tf_pts.green[563].value  = 0;
    post1D->tf_pts.blue[563].value   = 0;
    post1D->tf_pts.red[564].value    = 0;
    post1D->tf_pts.green[564].value  = 0;
    post1D->tf_pts.blue[564].value   = 0;
    post1D->tf_pts.red[565].value    = 0;
    post1D->tf_pts.green[565].value  = 0;
    post1D->tf_pts.blue[565].value   = 0;
    post1D->tf_pts.red[566].value    = 0;
    post1D->tf_pts.green[566].value  = 0;
    post1D->tf_pts.blue[566].value   = 0;
    post1D->tf_pts.red[567].value    = 0;
    post1D->tf_pts.green[567].value  = 0;
    post1D->tf_pts.blue[567].value   = 0;
    post1D->tf_pts.red[568].value    = 0;
    post1D->tf_pts.green[568].value  = 0;
    post1D->tf_pts.blue[568].value   = 0;
    post1D->tf_pts.red[569].value    = 0;
    post1D->tf_pts.green[569].value  = 0;
    post1D->tf_pts.blue[569].value   = 0;
    post1D->tf_pts.red[570].value    = 0;
    post1D->tf_pts.green[570].value  = 0;
    post1D->tf_pts.blue[570].value   = 0;
    post1D->tf_pts.red[571].value    = 0;
    post1D->tf_pts.green[571].value  = 0;
    post1D->tf_pts.blue[571].value   = 0;
    post1D->tf_pts.red[572].value    = 0;
    post1D->tf_pts.green[572].value  = 0;
    post1D->tf_pts.blue[572].value   = 0;
    post1D->tf_pts.red[573].value    = 0;
    post1D->tf_pts.green[573].value  = 0;
    post1D->tf_pts.blue[573].value   = 0;
    post1D->tf_pts.red[574].value    = 0;
    post1D->tf_pts.green[574].value  = 0;
    post1D->tf_pts.blue[574].value   = 0;
    post1D->tf_pts.red[575].value    = 0;
    post1D->tf_pts.green[575].value  = 0;
    post1D->tf_pts.blue[575].value   = 0;
    post1D->tf_pts.red[576].value    = 0;
    post1D->tf_pts.green[576].value  = 0;
    post1D->tf_pts.blue[576].value   = 0;
    post1D->tf_pts.red[577].value    = 0;
    post1D->tf_pts.green[577].value  = 0;
    post1D->tf_pts.blue[577].value   = 0;
    post1D->tf_pts.red[578].value    = 0;
    post1D->tf_pts.green[578].value  = 0;
    post1D->tf_pts.blue[578].value   = 0;
    post1D->tf_pts.red[579].value    = 0;
    post1D->tf_pts.green[579].value  = 0;
    post1D->tf_pts.blue[579].value   = 0;
    post1D->tf_pts.red[580].value    = 0;
    post1D->tf_pts.green[580].value  = 0;
    post1D->tf_pts.blue[580].value   = 0;
    post1D->tf_pts.red[581].value    = 0;
    post1D->tf_pts.green[581].value  = 0;
    post1D->tf_pts.blue[581].value   = 0;
    post1D->tf_pts.red[582].value    = 0;
    post1D->tf_pts.green[582].value  = 0;
    post1D->tf_pts.blue[582].value   = 0;
    post1D->tf_pts.red[583].value    = 0;
    post1D->tf_pts.green[583].value  = 0;
    post1D->tf_pts.blue[583].value   = 0;
    post1D->tf_pts.red[584].value    = 0;
    post1D->tf_pts.green[584].value  = 0;
    post1D->tf_pts.blue[584].value   = 0;
    post1D->tf_pts.red[585].value    = 0;
    post1D->tf_pts.green[585].value  = 0;
    post1D->tf_pts.blue[585].value   = 0;
    post1D->tf_pts.red[586].value    = 0;
    post1D->tf_pts.green[586].value  = 0;
    post1D->tf_pts.blue[586].value   = 0;
    post1D->tf_pts.red[587].value    = 0;
    post1D->tf_pts.green[587].value  = 0;
    post1D->tf_pts.blue[587].value   = 0;
    post1D->tf_pts.red[588].value    = 0;
    post1D->tf_pts.green[588].value  = 0;
    post1D->tf_pts.blue[588].value   = 0;
    post1D->tf_pts.red[589].value    = 0;
    post1D->tf_pts.green[589].value  = 0;
    post1D->tf_pts.blue[589].value   = 0;
    post1D->tf_pts.red[590].value    = 0;
    post1D->tf_pts.green[590].value  = 0;
    post1D->tf_pts.blue[590].value   = 0;
    post1D->tf_pts.red[591].value    = 0;
    post1D->tf_pts.green[591].value  = 0;
    post1D->tf_pts.blue[591].value   = 0;
    post1D->tf_pts.red[592].value    = 0;
    post1D->tf_pts.green[592].value  = 0;
    post1D->tf_pts.blue[592].value   = 0;
    post1D->tf_pts.red[593].value    = 0;
    post1D->tf_pts.green[593].value  = 0;
    post1D->tf_pts.blue[593].value   = 0;
    post1D->tf_pts.red[594].value    = 0;
    post1D->tf_pts.green[594].value  = 0;
    post1D->tf_pts.blue[594].value   = 0;
    post1D->tf_pts.red[595].value    = 0;
    post1D->tf_pts.green[595].value  = 0;
    post1D->tf_pts.blue[595].value   = 0;
    post1D->tf_pts.red[596].value    = 0;
    post1D->tf_pts.green[596].value  = 0;
    post1D->tf_pts.blue[596].value   = 0;
    post1D->tf_pts.red[597].value    = 0;
    post1D->tf_pts.green[597].value  = 0;
    post1D->tf_pts.blue[597].value   = 0;
    post1D->tf_pts.red[598].value    = 0;
    post1D->tf_pts.green[598].value  = 0;
    post1D->tf_pts.blue[598].value   = 0;
    post1D->tf_pts.red[599].value    = 0;
    post1D->tf_pts.green[599].value  = 0;
    post1D->tf_pts.blue[599].value   = 0;
    post1D->tf_pts.red[600].value    = 0;
    post1D->tf_pts.green[600].value  = 0;
    post1D->tf_pts.blue[600].value   = 0;
    post1D->tf_pts.red[601].value    = 0;
    post1D->tf_pts.green[601].value  = 0;
    post1D->tf_pts.blue[601].value   = 0;
    post1D->tf_pts.red[602].value    = 0;
    post1D->tf_pts.green[602].value  = 0;
    post1D->tf_pts.blue[602].value   = 0;
    post1D->tf_pts.red[603].value    = 0;
    post1D->tf_pts.green[603].value  = 0;
    post1D->tf_pts.blue[603].value   = 0;
    post1D->tf_pts.red[604].value    = 0;
    post1D->tf_pts.green[604].value  = 0;
    post1D->tf_pts.blue[604].value   = 0;
    post1D->tf_pts.red[605].value    = 0;
    post1D->tf_pts.green[605].value  = 0;
    post1D->tf_pts.blue[605].value   = 0;
    post1D->tf_pts.red[606].value    = 0;
    post1D->tf_pts.green[606].value  = 0;
    post1D->tf_pts.blue[606].value   = 0;
    post1D->tf_pts.red[607].value    = 0;
    post1D->tf_pts.green[607].value  = 0;
    post1D->tf_pts.blue[607].value   = 0;
    post1D->tf_pts.red[608].value    = 0;
    post1D->tf_pts.green[608].value  = 0;
    post1D->tf_pts.blue[608].value   = 0;
    post1D->tf_pts.red[609].value    = 0;
    post1D->tf_pts.green[609].value  = 0;
    post1D->tf_pts.blue[609].value   = 0;
    post1D->tf_pts.red[610].value    = 0;
    post1D->tf_pts.green[610].value  = 0;
    post1D->tf_pts.blue[610].value   = 0;
    post1D->tf_pts.red[611].value    = 0;
    post1D->tf_pts.green[611].value  = 0;
    post1D->tf_pts.blue[611].value   = 0;
    post1D->tf_pts.red[612].value    = 0;
    post1D->tf_pts.green[612].value  = 0;
    post1D->tf_pts.blue[612].value   = 0;
    post1D->tf_pts.red[613].value    = 0;
    post1D->tf_pts.green[613].value  = 0;
    post1D->tf_pts.blue[613].value   = 0;
    post1D->tf_pts.red[614].value    = 0;
    post1D->tf_pts.green[614].value  = 0;
    post1D->tf_pts.blue[614].value   = 0;
    post1D->tf_pts.red[615].value    = 0;
    post1D->tf_pts.green[615].value  = 0;
    post1D->tf_pts.blue[615].value   = 0;
    post1D->tf_pts.red[616].value    = 0;
    post1D->tf_pts.green[616].value  = 0;
    post1D->tf_pts.blue[616].value   = 0;
    post1D->tf_pts.red[617].value    = 0;
    post1D->tf_pts.green[617].value  = 0;
    post1D->tf_pts.blue[617].value   = 0;
    post1D->tf_pts.red[618].value    = 0;
    post1D->tf_pts.green[618].value  = 0;
    post1D->tf_pts.blue[618].value   = 0;
    post1D->tf_pts.red[619].value    = 0;
    post1D->tf_pts.green[619].value  = 0;
    post1D->tf_pts.blue[619].value   = 0;
    post1D->tf_pts.red[620].value    = 0;
    post1D->tf_pts.green[620].value  = 0;
    post1D->tf_pts.blue[620].value   = 0;
    post1D->tf_pts.red[621].value    = 0;
    post1D->tf_pts.green[621].value  = 0;
    post1D->tf_pts.blue[621].value   = 0;
    post1D->tf_pts.red[622].value    = 0;
    post1D->tf_pts.green[622].value  = 0;
    post1D->tf_pts.blue[622].value   = 0;
    post1D->tf_pts.red[623].value    = 0;
    post1D->tf_pts.green[623].value  = 0;
    post1D->tf_pts.blue[623].value   = 0;
    post1D->tf_pts.red[624].value    = 0;
    post1D->tf_pts.green[624].value  = 0;
    post1D->tf_pts.blue[624].value   = 0;
    post1D->tf_pts.red[625].value    = 0;
    post1D->tf_pts.green[625].value  = 0;
    post1D->tf_pts.blue[625].value   = 0;
    post1D->tf_pts.red[626].value    = 0;
    post1D->tf_pts.green[626].value  = 0;
    post1D->tf_pts.blue[626].value   = 0;
    post1D->tf_pts.red[627].value    = 0;
    post1D->tf_pts.green[627].value  = 0;
    post1D->tf_pts.blue[627].value   = 0;
    post1D->tf_pts.red[628].value    = 0;
    post1D->tf_pts.green[628].value  = 0;
    post1D->tf_pts.blue[628].value   = 0;
    post1D->tf_pts.red[629].value    = 0;
    post1D->tf_pts.green[629].value  = 0;
    post1D->tf_pts.blue[629].value   = 0;
    post1D->tf_pts.red[630].value    = 0;
    post1D->tf_pts.green[630].value  = 0;
    post1D->tf_pts.blue[630].value   = 0;
    post1D->tf_pts.red[631].value    = 0;
    post1D->tf_pts.green[631].value  = 0;
    post1D->tf_pts.blue[631].value   = 0;
    post1D->tf_pts.red[632].value    = 0;
    post1D->tf_pts.green[632].value  = 0;
    post1D->tf_pts.blue[632].value   = 0;
    post1D->tf_pts.red[633].value    = 0;
    post1D->tf_pts.green[633].value  = 0;
    post1D->tf_pts.blue[633].value   = 0;
    post1D->tf_pts.red[634].value    = 0;
    post1D->tf_pts.green[634].value  = 0;
    post1D->tf_pts.blue[634].value   = 0;
    post1D->tf_pts.red[635].value    = 0;
    post1D->tf_pts.green[635].value  = 0;
    post1D->tf_pts.blue[635].value   = 0;
    post1D->tf_pts.red[636].value    = 0;
    post1D->tf_pts.green[636].value  = 0;
    post1D->tf_pts.blue[636].value   = 0;
    post1D->tf_pts.red[637].value    = 0;
    post1D->tf_pts.green[637].value  = 0;
    post1D->tf_pts.blue[637].value   = 0;
    post1D->tf_pts.red[638].value    = 0;
    post1D->tf_pts.green[638].value  = 0;
    post1D->tf_pts.blue[638].value   = 0;
    post1D->tf_pts.red[639].value    = 0;
    post1D->tf_pts.green[639].value  = 0;
    post1D->tf_pts.blue[639].value   = 0;
    post1D->tf_pts.red[640].value    = 0;
    post1D->tf_pts.green[640].value  = 0;
    post1D->tf_pts.blue[640].value   = 0;
    post1D->tf_pts.red[641].value    = 0;
    post1D->tf_pts.green[641].value  = 0;
    post1D->tf_pts.blue[641].value   = 0;
    post1D->tf_pts.red[642].value    = 0;
    post1D->tf_pts.green[642].value  = 0;
    post1D->tf_pts.blue[642].value   = 0;
    post1D->tf_pts.red[643].value    = 0;
    post1D->tf_pts.green[643].value  = 0;
    post1D->tf_pts.blue[643].value   = 0;
    post1D->tf_pts.red[644].value    = 0;
    post1D->tf_pts.green[644].value  = 0;
    post1D->tf_pts.blue[644].value   = 0;
    post1D->tf_pts.red[645].value    = 0;
    post1D->tf_pts.green[645].value  = 0;
    post1D->tf_pts.blue[645].value   = 0;
    post1D->tf_pts.red[646].value    = 0;
    post1D->tf_pts.green[646].value  = 0;
    post1D->tf_pts.blue[646].value   = 0;
    post1D->tf_pts.red[647].value    = 0;
    post1D->tf_pts.green[647].value  = 0;
    post1D->tf_pts.blue[647].value   = 0;
    post1D->tf_pts.red[648].value    = 0;
    post1D->tf_pts.green[648].value  = 0;
    post1D->tf_pts.blue[648].value   = 0;
    post1D->tf_pts.red[649].value    = 0;
    post1D->tf_pts.green[649].value  = 0;
    post1D->tf_pts.blue[649].value   = 0;
    post1D->tf_pts.red[650].value    = 0;
    post1D->tf_pts.green[650].value  = 0;
    post1D->tf_pts.blue[650].value   = 0;
    post1D->tf_pts.red[651].value    = 0;
    post1D->tf_pts.green[651].value  = 0;
    post1D->tf_pts.blue[651].value   = 0;
    post1D->tf_pts.red[652].value    = 0;
    post1D->tf_pts.green[652].value  = 0;
    post1D->tf_pts.blue[652].value   = 0;
    post1D->tf_pts.red[653].value    = 0;
    post1D->tf_pts.green[653].value  = 0;
    post1D->tf_pts.blue[653].value   = 0;
    post1D->tf_pts.red[654].value    = 0;
    post1D->tf_pts.green[654].value  = 0;
    post1D->tf_pts.blue[654].value   = 0;
    post1D->tf_pts.red[655].value    = 0;
    post1D->tf_pts.green[655].value  = 0;
    post1D->tf_pts.blue[655].value   = 0;
    post1D->tf_pts.red[656].value    = 0;
    post1D->tf_pts.green[656].value  = 0;
    post1D->tf_pts.blue[656].value   = 0;
    post1D->tf_pts.red[657].value    = 0;
    post1D->tf_pts.green[657].value  = 0;
    post1D->tf_pts.blue[657].value   = 0;
    post1D->tf_pts.red[658].value    = 0;
    post1D->tf_pts.green[658].value  = 0;
    post1D->tf_pts.blue[658].value   = 0;
    post1D->tf_pts.red[659].value    = 0;
    post1D->tf_pts.green[659].value  = 0;
    post1D->tf_pts.blue[659].value   = 0;
    post1D->tf_pts.red[660].value    = 0;
    post1D->tf_pts.green[660].value  = 0;
    post1D->tf_pts.blue[660].value   = 0;
    post1D->tf_pts.red[661].value    = 0;
    post1D->tf_pts.green[661].value  = 0;
    post1D->tf_pts.blue[661].value   = 0;
    post1D->tf_pts.red[662].value    = 0;
    post1D->tf_pts.green[662].value  = 0;
    post1D->tf_pts.blue[662].value   = 0;
    post1D->tf_pts.red[663].value    = 0;
    post1D->tf_pts.green[663].value  = 0;
    post1D->tf_pts.blue[663].value   = 0;
    post1D->tf_pts.red[664].value    = 0;
    post1D->tf_pts.green[664].value  = 0;
    post1D->tf_pts.blue[664].value   = 0;
    post1D->tf_pts.red[665].value    = 0;
    post1D->tf_pts.green[665].value  = 0;
    post1D->tf_pts.blue[665].value   = 0;
    post1D->tf_pts.red[666].value    = 0;
    post1D->tf_pts.green[666].value  = 0;
    post1D->tf_pts.blue[666].value   = 0;
    post1D->tf_pts.red[667].value    = 0;
    post1D->tf_pts.green[667].value  = 0;
    post1D->tf_pts.blue[667].value   = 0;
    post1D->tf_pts.red[668].value    = 0;
    post1D->tf_pts.green[668].value  = 0;
    post1D->tf_pts.blue[668].value   = 0;
    post1D->tf_pts.red[669].value    = 0;
    post1D->tf_pts.green[669].value  = 0;
    post1D->tf_pts.blue[669].value   = 0;
    post1D->tf_pts.red[670].value    = 0;
    post1D->tf_pts.green[670].value  = 0;
    post1D->tf_pts.blue[670].value   = 0;
    post1D->tf_pts.red[671].value    = 0;
    post1D->tf_pts.green[671].value  = 0;
    post1D->tf_pts.blue[671].value   = 0;
    post1D->tf_pts.red[672].value    = 0;
    post1D->tf_pts.green[672].value  = 0;
    post1D->tf_pts.blue[672].value   = 0;
    post1D->tf_pts.red[673].value    = 0;
    post1D->tf_pts.green[673].value  = 0;
    post1D->tf_pts.blue[673].value   = 0;
    post1D->tf_pts.red[674].value    = 0;
    post1D->tf_pts.green[674].value  = 0;
    post1D->tf_pts.blue[674].value   = 0;
    post1D->tf_pts.red[675].value    = 0;
    post1D->tf_pts.green[675].value  = 0;
    post1D->tf_pts.blue[675].value   = 0;
    post1D->tf_pts.red[676].value    = 0;
    post1D->tf_pts.green[676].value  = 0;
    post1D->tf_pts.blue[676].value   = 0;
    post1D->tf_pts.red[677].value    = 0;
    post1D->tf_pts.green[677].value  = 0;
    post1D->tf_pts.blue[677].value   = 0;
    post1D->tf_pts.red[678].value    = 0;
    post1D->tf_pts.green[678].value  = 0;
    post1D->tf_pts.blue[678].value   = 0;
    post1D->tf_pts.red[679].value    = 0;
    post1D->tf_pts.green[679].value  = 0;
    post1D->tf_pts.blue[679].value   = 0;
    post1D->tf_pts.red[680].value    = 0;
    post1D->tf_pts.green[680].value  = 0;
    post1D->tf_pts.blue[680].value   = 0;
    post1D->tf_pts.red[681].value    = 0;
    post1D->tf_pts.green[681].value  = 0;
    post1D->tf_pts.blue[681].value   = 0;
    post1D->tf_pts.red[682].value    = 0;
    post1D->tf_pts.green[682].value  = 0;
    post1D->tf_pts.blue[682].value   = 0;
    post1D->tf_pts.red[683].value    = 0;
    post1D->tf_pts.green[683].value  = 0;
    post1D->tf_pts.blue[683].value   = 0;
    post1D->tf_pts.red[684].value    = 0;
    post1D->tf_pts.green[684].value  = 0;
    post1D->tf_pts.blue[684].value   = 0;
    post1D->tf_pts.red[685].value    = 0;
    post1D->tf_pts.green[685].value  = 0;
    post1D->tf_pts.blue[685].value   = 0;
    post1D->tf_pts.red[686].value    = 0;
    post1D->tf_pts.green[686].value  = 0;
    post1D->tf_pts.blue[686].value   = 0;
    post1D->tf_pts.red[687].value    = 0;
    post1D->tf_pts.green[687].value  = 0;
    post1D->tf_pts.blue[687].value   = 0;
    post1D->tf_pts.red[688].value    = 0;
    post1D->tf_pts.green[688].value  = 0;
    post1D->tf_pts.blue[688].value   = 0;
    post1D->tf_pts.red[689].value    = 0;
    post1D->tf_pts.green[689].value  = 0;
    post1D->tf_pts.blue[689].value   = 0;
    post1D->tf_pts.red[690].value    = 0;
    post1D->tf_pts.green[690].value  = 0;
    post1D->tf_pts.blue[690].value   = 0;
    post1D->tf_pts.red[691].value    = 0;
    post1D->tf_pts.green[691].value  = 0;
    post1D->tf_pts.blue[691].value   = 0;
    post1D->tf_pts.red[692].value    = 0;
    post1D->tf_pts.green[692].value  = 0;
    post1D->tf_pts.blue[692].value   = 0;
    post1D->tf_pts.red[693].value    = 0;
    post1D->tf_pts.green[693].value  = 0;
    post1D->tf_pts.blue[693].value   = 0;
    post1D->tf_pts.red[694].value    = 0;
    post1D->tf_pts.green[694].value  = 0;
    post1D->tf_pts.blue[694].value   = 0;
    post1D->tf_pts.red[695].value    = 0;
    post1D->tf_pts.green[695].value  = 0;
    post1D->tf_pts.blue[695].value   = 0;
    post1D->tf_pts.red[696].value    = 0;
    post1D->tf_pts.green[696].value  = 0;
    post1D->tf_pts.blue[696].value   = 0;
    post1D->tf_pts.red[697].value    = 0;
    post1D->tf_pts.green[697].value  = 0;
    post1D->tf_pts.blue[697].value   = 0;
    post1D->tf_pts.red[698].value    = 0;
    post1D->tf_pts.green[698].value  = 0;
    post1D->tf_pts.blue[698].value   = 0;
    post1D->tf_pts.red[699].value    = 0;
    post1D->tf_pts.green[699].value  = 0;
    post1D->tf_pts.blue[699].value   = 0;
    post1D->tf_pts.red[700].value    = 0;
    post1D->tf_pts.green[700].value  = 0;
    post1D->tf_pts.blue[700].value   = 0;
    post1D->tf_pts.red[701].value    = 0;
    post1D->tf_pts.green[701].value  = 0;
    post1D->tf_pts.blue[701].value   = 0;
    post1D->tf_pts.red[702].value    = 0;
    post1D->tf_pts.green[702].value  = 0;
    post1D->tf_pts.blue[702].value   = 0;
    post1D->tf_pts.red[703].value    = 0;
    post1D->tf_pts.green[703].value  = 0;
    post1D->tf_pts.blue[703].value   = 0;
    post1D->tf_pts.red[704].value    = 0;
    post1D->tf_pts.green[704].value  = 0;
    post1D->tf_pts.blue[704].value   = 0;
    post1D->tf_pts.red[705].value    = 0;
    post1D->tf_pts.green[705].value  = 0;
    post1D->tf_pts.blue[705].value   = 0;
    post1D->tf_pts.red[706].value    = 0;
    post1D->tf_pts.green[706].value  = 0;
    post1D->tf_pts.blue[706].value   = 0;
    post1D->tf_pts.red[707].value    = 0;
    post1D->tf_pts.green[707].value  = 0;
    post1D->tf_pts.blue[707].value   = 0;
    post1D->tf_pts.red[708].value    = 0;
    post1D->tf_pts.green[708].value  = 0;
    post1D->tf_pts.blue[708].value   = 0;
    post1D->tf_pts.red[709].value    = 0;
    post1D->tf_pts.green[709].value  = 0;
    post1D->tf_pts.blue[709].value   = 0;
    post1D->tf_pts.red[710].value    = 0;
    post1D->tf_pts.green[710].value  = 0;
    post1D->tf_pts.blue[710].value   = 0;
    post1D->tf_pts.red[711].value    = 0;
    post1D->tf_pts.green[711].value  = 0;
    post1D->tf_pts.blue[711].value   = 0;
    post1D->tf_pts.red[712].value    = 0;
    post1D->tf_pts.green[712].value  = 0;
    post1D->tf_pts.blue[712].value   = 0;
    post1D->tf_pts.red[713].value    = 0;
    post1D->tf_pts.green[713].value  = 0;
    post1D->tf_pts.blue[713].value   = 0;
    post1D->tf_pts.red[714].value    = 0;
    post1D->tf_pts.green[714].value  = 0;
    post1D->tf_pts.blue[714].value   = 0;
    post1D->tf_pts.red[715].value    = 0;
    post1D->tf_pts.green[715].value  = 0;
    post1D->tf_pts.blue[715].value   = 0;
    post1D->tf_pts.red[716].value    = 0;
    post1D->tf_pts.green[716].value  = 0;
    post1D->tf_pts.blue[716].value   = 0;
    post1D->tf_pts.red[717].value    = 0;
    post1D->tf_pts.green[717].value  = 0;
    post1D->tf_pts.blue[717].value   = 0;
    post1D->tf_pts.red[718].value    = 0;
    post1D->tf_pts.green[718].value  = 0;
    post1D->tf_pts.blue[718].value   = 0;
    post1D->tf_pts.red[719].value    = 0;
    post1D->tf_pts.green[719].value  = 0;
    post1D->tf_pts.blue[719].value   = 0;
    post1D->tf_pts.red[720].value    = 0;
    post1D->tf_pts.green[720].value  = 0;
    post1D->tf_pts.blue[720].value   = 0;
    post1D->tf_pts.red[721].value    = 0;
    post1D->tf_pts.green[721].value  = 0;
    post1D->tf_pts.blue[721].value   = 0;
    post1D->tf_pts.red[722].value    = 0;
    post1D->tf_pts.green[722].value  = 0;
    post1D->tf_pts.blue[722].value   = 0;
    post1D->tf_pts.red[723].value    = 0;
    post1D->tf_pts.green[723].value  = 0;
    post1D->tf_pts.blue[723].value   = 0;
    post1D->tf_pts.red[724].value    = 0;
    post1D->tf_pts.green[724].value  = 0;
    post1D->tf_pts.blue[724].value   = 0;
    post1D->tf_pts.red[725].value    = 0;
    post1D->tf_pts.green[725].value  = 0;
    post1D->tf_pts.blue[725].value   = 0;
    post1D->tf_pts.red[726].value    = 0;
    post1D->tf_pts.green[726].value  = 0;
    post1D->tf_pts.blue[726].value   = 0;
    post1D->tf_pts.red[727].value    = 0;
    post1D->tf_pts.green[727].value  = 0;
    post1D->tf_pts.blue[727].value   = 0;
    post1D->tf_pts.red[728].value    = 0;
    post1D->tf_pts.green[728].value  = 0;
    post1D->tf_pts.blue[728].value   = 0;
    post1D->tf_pts.red[729].value    = 0;
    post1D->tf_pts.green[729].value  = 0;
    post1D->tf_pts.blue[729].value   = 0;
    post1D->tf_pts.red[730].value    = 0;
    post1D->tf_pts.green[730].value  = 0;
    post1D->tf_pts.blue[730].value   = 0;
    post1D->tf_pts.red[731].value    = 0;
    post1D->tf_pts.green[731].value  = 0;
    post1D->tf_pts.blue[731].value   = 0;
    post1D->tf_pts.red[732].value    = 0;
    post1D->tf_pts.green[732].value  = 0;
    post1D->tf_pts.blue[732].value   = 0;
    post1D->tf_pts.red[733].value    = 0;
    post1D->tf_pts.green[733].value  = 0;
    post1D->tf_pts.blue[733].value   = 0;
    post1D->tf_pts.red[734].value    = 0;
    post1D->tf_pts.green[734].value  = 0;
    post1D->tf_pts.blue[734].value   = 0;
    post1D->tf_pts.red[735].value    = 0;
    post1D->tf_pts.green[735].value  = 0;
    post1D->tf_pts.blue[735].value   = 0;
    post1D->tf_pts.red[736].value    = 0;
    post1D->tf_pts.green[736].value  = 0;
    post1D->tf_pts.blue[736].value   = 0;
    post1D->tf_pts.red[737].value    = 0;
    post1D->tf_pts.green[737].value  = 0;
    post1D->tf_pts.blue[737].value   = 0;
    post1D->tf_pts.red[738].value    = 0;
    post1D->tf_pts.green[738].value  = 0;
    post1D->tf_pts.blue[738].value   = 0;
    post1D->tf_pts.red[739].value    = 0;
    post1D->tf_pts.green[739].value  = 0;
    post1D->tf_pts.blue[739].value   = 0;
    post1D->tf_pts.red[740].value    = 0;
    post1D->tf_pts.green[740].value  = 0;
    post1D->tf_pts.blue[740].value   = 0;
    post1D->tf_pts.red[741].value    = 0;
    post1D->tf_pts.green[741].value  = 0;
    post1D->tf_pts.blue[741].value   = 0;
    post1D->tf_pts.red[742].value    = 0;
    post1D->tf_pts.green[742].value  = 0;
    post1D->tf_pts.blue[742].value   = 0;
    post1D->tf_pts.red[743].value    = 0;
    post1D->tf_pts.green[743].value  = 0;
    post1D->tf_pts.blue[743].value   = 0;
    post1D->tf_pts.red[744].value    = 0;
    post1D->tf_pts.green[744].value  = 0;
    post1D->tf_pts.blue[744].value   = 0;
    post1D->tf_pts.red[745].value    = 0;
    post1D->tf_pts.green[745].value  = 0;
    post1D->tf_pts.blue[745].value   = 0;
    post1D->tf_pts.red[746].value    = 0;
    post1D->tf_pts.green[746].value  = 0;
    post1D->tf_pts.blue[746].value   = 0;
    post1D->tf_pts.red[747].value    = 0;
    post1D->tf_pts.green[747].value  = 0;
    post1D->tf_pts.blue[747].value   = 0;
    post1D->tf_pts.red[748].value    = 0;
    post1D->tf_pts.green[748].value  = 0;
    post1D->tf_pts.blue[748].value   = 0;
    post1D->tf_pts.red[749].value    = 0;
    post1D->tf_pts.green[749].value  = 0;
    post1D->tf_pts.blue[749].value   = 0;
    post1D->tf_pts.red[750].value    = 0;
    post1D->tf_pts.green[750].value  = 0;
    post1D->tf_pts.blue[750].value   = 0;
    post1D->tf_pts.red[751].value    = 0;
    post1D->tf_pts.green[751].value  = 0;
    post1D->tf_pts.blue[751].value   = 0;
    post1D->tf_pts.red[752].value    = 0;
    post1D->tf_pts.green[752].value  = 0;
    post1D->tf_pts.blue[752].value   = 0;
    post1D->tf_pts.red[753].value    = 0;
    post1D->tf_pts.green[753].value  = 0;
    post1D->tf_pts.blue[753].value   = 0;
    post1D->tf_pts.red[754].value    = 0;
    post1D->tf_pts.green[754].value  = 0;
    post1D->tf_pts.blue[754].value   = 0;
    post1D->tf_pts.red[755].value    = 0;
    post1D->tf_pts.green[755].value  = 0;
    post1D->tf_pts.blue[755].value   = 0;
    post1D->tf_pts.red[756].value    = 0;
    post1D->tf_pts.green[756].value  = 0;
    post1D->tf_pts.blue[756].value   = 0;
    post1D->tf_pts.red[757].value    = 0;
    post1D->tf_pts.green[757].value  = 0;
    post1D->tf_pts.blue[757].value   = 0;
    post1D->tf_pts.red[758].value    = 0;
    post1D->tf_pts.green[758].value  = 0;
    post1D->tf_pts.blue[758].value   = 0;
    post1D->tf_pts.red[759].value    = 0;
    post1D->tf_pts.green[759].value  = 0;
    post1D->tf_pts.blue[759].value   = 0;
    post1D->tf_pts.red[760].value    = 0;
    post1D->tf_pts.green[760].value  = 0;
    post1D->tf_pts.blue[760].value   = 0;
    post1D->tf_pts.red[761].value    = 0;
    post1D->tf_pts.green[761].value  = 0;
    post1D->tf_pts.blue[761].value   = 0;
    post1D->tf_pts.red[762].value    = 0;
    post1D->tf_pts.green[762].value  = 0;
    post1D->tf_pts.blue[762].value   = 0;
    post1D->tf_pts.red[763].value    = 0;
    post1D->tf_pts.green[763].value  = 0;
    post1D->tf_pts.blue[763].value   = 0;
    post1D->tf_pts.red[764].value    = 0;
    post1D->tf_pts.green[764].value  = 0;
    post1D->tf_pts.blue[764].value   = 0;
    post1D->tf_pts.red[765].value    = 0;
    post1D->tf_pts.green[765].value  = 0;
    post1D->tf_pts.blue[765].value   = 0;
    post1D->tf_pts.red[766].value    = 0;
    post1D->tf_pts.green[766].value  = 0;
    post1D->tf_pts.blue[766].value   = 0;
    post1D->tf_pts.red[767].value    = 0;
    post1D->tf_pts.green[767].value  = 0;
    post1D->tf_pts.blue[767].value   = 0;
    post1D->tf_pts.red[768].value    = 0;
    post1D->tf_pts.green[768].value  = 0;
    post1D->tf_pts.blue[768].value   = 0;
    post1D->tf_pts.red[769].value    = 0;
    post1D->tf_pts.green[769].value  = 0;
    post1D->tf_pts.blue[769].value   = 0;
    post1D->tf_pts.red[770].value    = 0;
    post1D->tf_pts.green[770].value  = 0;
    post1D->tf_pts.blue[770].value   = 0;
    post1D->tf_pts.red[771].value    = 0;
    post1D->tf_pts.green[771].value  = 0;
    post1D->tf_pts.blue[771].value   = 0;
    post1D->tf_pts.red[772].value    = 0;
    post1D->tf_pts.green[772].value  = 0;
    post1D->tf_pts.blue[772].value   = 0;
    post1D->tf_pts.red[773].value    = 0;
    post1D->tf_pts.green[773].value  = 0;
    post1D->tf_pts.blue[773].value   = 0;
    post1D->tf_pts.red[774].value    = 0;
    post1D->tf_pts.green[774].value  = 0;
    post1D->tf_pts.blue[774].value   = 0;
    post1D->tf_pts.red[775].value    = 0;
    post1D->tf_pts.green[775].value  = 0;
    post1D->tf_pts.blue[775].value   = 0;
    post1D->tf_pts.red[776].value    = 0;
    post1D->tf_pts.green[776].value  = 0;
    post1D->tf_pts.blue[776].value   = 0;
    post1D->tf_pts.red[777].value    = 0;
    post1D->tf_pts.green[777].value  = 0;
    post1D->tf_pts.blue[777].value   = 0;
    post1D->tf_pts.red[778].value    = 0;
    post1D->tf_pts.green[778].value  = 0;
    post1D->tf_pts.blue[778].value   = 0;
    post1D->tf_pts.red[779].value    = 0;
    post1D->tf_pts.green[779].value  = 0;
    post1D->tf_pts.blue[779].value   = 0;
    post1D->tf_pts.red[780].value    = 0;
    post1D->tf_pts.green[780].value  = 0;
    post1D->tf_pts.blue[780].value   = 0;
    post1D->tf_pts.red[781].value    = 0;
    post1D->tf_pts.green[781].value  = 0;
    post1D->tf_pts.blue[781].value   = 0;
    post1D->tf_pts.red[782].value    = 0;
    post1D->tf_pts.green[782].value  = 0;
    post1D->tf_pts.blue[782].value   = 0;
    post1D->tf_pts.red[783].value    = 0;
    post1D->tf_pts.green[783].value  = 0;
    post1D->tf_pts.blue[783].value   = 0;
    post1D->tf_pts.red[784].value    = 0;
    post1D->tf_pts.green[784].value  = 0;
    post1D->tf_pts.blue[784].value   = 0;
    post1D->tf_pts.red[785].value    = 0;
    post1D->tf_pts.green[785].value  = 0;
    post1D->tf_pts.blue[785].value   = 0;
    post1D->tf_pts.red[786].value    = 0;
    post1D->tf_pts.green[786].value  = 0;
    post1D->tf_pts.blue[786].value   = 0;
    post1D->tf_pts.red[787].value    = 0;
    post1D->tf_pts.green[787].value  = 0;
    post1D->tf_pts.blue[787].value   = 0;
    post1D->tf_pts.red[788].value    = 0;
    post1D->tf_pts.green[788].value  = 0;
    post1D->tf_pts.blue[788].value   = 0;
    post1D->tf_pts.red[789].value    = 0;
    post1D->tf_pts.green[789].value  = 0;
    post1D->tf_pts.blue[789].value   = 0;
    post1D->tf_pts.red[790].value    = 0;
    post1D->tf_pts.green[790].value  = 0;
    post1D->tf_pts.blue[790].value   = 0;
    post1D->tf_pts.red[791].value    = 0;
    post1D->tf_pts.green[791].value  = 0;
    post1D->tf_pts.blue[791].value   = 0;
    post1D->tf_pts.red[792].value    = 0;
    post1D->tf_pts.green[792].value  = 0;
    post1D->tf_pts.blue[792].value   = 0;
    post1D->tf_pts.red[793].value    = 0;
    post1D->tf_pts.green[793].value  = 0;
    post1D->tf_pts.blue[793].value   = 0;
    post1D->tf_pts.red[794].value    = 0;
    post1D->tf_pts.green[794].value  = 0;
    post1D->tf_pts.blue[794].value   = 0;
    post1D->tf_pts.red[795].value    = 0;
    post1D->tf_pts.green[795].value  = 0;
    post1D->tf_pts.blue[795].value   = 0;
    post1D->tf_pts.red[796].value    = 0;
    post1D->tf_pts.green[796].value  = 0;
    post1D->tf_pts.blue[796].value   = 0;
    post1D->tf_pts.red[797].value    = 0;
    post1D->tf_pts.green[797].value  = 0;
    post1D->tf_pts.blue[797].value   = 0;
    post1D->tf_pts.red[798].value    = 0;
    post1D->tf_pts.green[798].value  = 0;
    post1D->tf_pts.blue[798].value   = 0;
    post1D->tf_pts.red[799].value    = 0;
    post1D->tf_pts.green[799].value  = 0;
    post1D->tf_pts.blue[799].value   = 0;
    post1D->tf_pts.red[800].value    = 0;
    post1D->tf_pts.green[800].value  = 0;
    post1D->tf_pts.blue[800].value   = 0;
    post1D->tf_pts.red[801].value    = 0;
    post1D->tf_pts.green[801].value  = 0;
    post1D->tf_pts.blue[801].value   = 0;
    post1D->tf_pts.red[802].value    = 0;
    post1D->tf_pts.green[802].value  = 0;
    post1D->tf_pts.blue[802].value   = 0;
    post1D->tf_pts.red[803].value    = 0;
    post1D->tf_pts.green[803].value  = 0;
    post1D->tf_pts.blue[803].value   = 0;
    post1D->tf_pts.red[804].value    = 0;
    post1D->tf_pts.green[804].value  = 0;
    post1D->tf_pts.blue[804].value   = 0;
    post1D->tf_pts.red[805].value    = 0;
    post1D->tf_pts.green[805].value  = 0;
    post1D->tf_pts.blue[805].value   = 0;
    post1D->tf_pts.red[806].value    = 0;
    post1D->tf_pts.green[806].value  = 0;
    post1D->tf_pts.blue[806].value   = 0;
    post1D->tf_pts.red[807].value    = 0;
    post1D->tf_pts.green[807].value  = 0;
    post1D->tf_pts.blue[807].value   = 0;
    post1D->tf_pts.red[808].value    = 0;
    post1D->tf_pts.green[808].value  = 0;
    post1D->tf_pts.blue[808].value   = 0;
    post1D->tf_pts.red[809].value    = 0;
    post1D->tf_pts.green[809].value  = 0;
    post1D->tf_pts.blue[809].value   = 0;
    post1D->tf_pts.red[810].value    = 0;
    post1D->tf_pts.green[810].value  = 0;
    post1D->tf_pts.blue[810].value   = 0;
    post1D->tf_pts.red[811].value    = 0;
    post1D->tf_pts.green[811].value  = 0;
    post1D->tf_pts.blue[811].value   = 0;
    post1D->tf_pts.red[812].value    = 0;
    post1D->tf_pts.green[812].value  = 0;
    post1D->tf_pts.blue[812].value   = 0;
    post1D->tf_pts.red[813].value    = 0;
    post1D->tf_pts.green[813].value  = 0;
    post1D->tf_pts.blue[813].value   = 0;
    post1D->tf_pts.red[814].value    = 0;
    post1D->tf_pts.green[814].value  = 0;
    post1D->tf_pts.blue[814].value   = 0;
    post1D->tf_pts.red[815].value    = 0;
    post1D->tf_pts.green[815].value  = 0;
    post1D->tf_pts.blue[815].value   = 0;
    post1D->tf_pts.red[816].value    = 0;
    post1D->tf_pts.green[816].value  = 0;
    post1D->tf_pts.blue[816].value   = 0;
    post1D->tf_pts.red[817].value    = 0;
    post1D->tf_pts.green[817].value  = 0;
    post1D->tf_pts.blue[817].value   = 0;
    post1D->tf_pts.red[818].value    = 0;
    post1D->tf_pts.green[818].value  = 0;
    post1D->tf_pts.blue[818].value   = 0;
    post1D->tf_pts.red[819].value    = 0;
    post1D->tf_pts.green[819].value  = 0;
    post1D->tf_pts.blue[819].value   = 0;
    post1D->tf_pts.red[820].value    = 0;
    post1D->tf_pts.green[820].value  = 0;
    post1D->tf_pts.blue[820].value   = 0;
    post1D->tf_pts.red[821].value    = 0;
    post1D->tf_pts.green[821].value  = 0;
    post1D->tf_pts.blue[821].value   = 0;
    post1D->tf_pts.red[822].value    = 0;
    post1D->tf_pts.green[822].value  = 0;
    post1D->tf_pts.blue[822].value   = 0;
    post1D->tf_pts.red[823].value    = 0;
    post1D->tf_pts.green[823].value  = 0;
    post1D->tf_pts.blue[823].value   = 0;
    post1D->tf_pts.red[824].value    = 0;
    post1D->tf_pts.green[824].value  = 0;
    post1D->tf_pts.blue[824].value   = 0;
    post1D->tf_pts.red[825].value    = 0;
    post1D->tf_pts.green[825].value  = 0;
    post1D->tf_pts.blue[825].value   = 0;
    post1D->tf_pts.red[826].value    = 0;
    post1D->tf_pts.green[826].value  = 0;
    post1D->tf_pts.blue[826].value   = 0;
    post1D->tf_pts.red[827].value    = 0;
    post1D->tf_pts.green[827].value  = 0;
    post1D->tf_pts.blue[827].value   = 0;
    post1D->tf_pts.red[828].value    = 0;
    post1D->tf_pts.green[828].value  = 0;
    post1D->tf_pts.blue[828].value   = 0;
    post1D->tf_pts.red[829].value    = 0;
    post1D->tf_pts.green[829].value  = 0;
    post1D->tf_pts.blue[829].value   = 0;
    post1D->tf_pts.red[830].value    = 0;
    post1D->tf_pts.green[830].value  = 0;
    post1D->tf_pts.blue[830].value   = 0;
    post1D->tf_pts.red[831].value    = 0;
    post1D->tf_pts.green[831].value  = 0;
    post1D->tf_pts.blue[831].value   = 0;
    post1D->tf_pts.red[832].value    = 0;
    post1D->tf_pts.green[832].value  = 0;
    post1D->tf_pts.blue[832].value   = 0;
    post1D->tf_pts.red[833].value    = 0;
    post1D->tf_pts.green[833].value  = 0;
    post1D->tf_pts.blue[833].value   = 0;
    post1D->tf_pts.red[834].value    = 0;
    post1D->tf_pts.green[834].value  = 0;
    post1D->tf_pts.blue[834].value   = 0;
    post1D->tf_pts.red[835].value    = 0;
    post1D->tf_pts.green[835].value  = 0;
    post1D->tf_pts.blue[835].value   = 0;
    post1D->tf_pts.red[836].value    = 0;
    post1D->tf_pts.green[836].value  = 0;
    post1D->tf_pts.blue[836].value   = 0;
    post1D->tf_pts.red[837].value    = 0;
    post1D->tf_pts.green[837].value  = 0;
    post1D->tf_pts.blue[837].value   = 0;
    post1D->tf_pts.red[838].value    = 0;
    post1D->tf_pts.green[838].value  = 0;
    post1D->tf_pts.blue[838].value   = 0;
    post1D->tf_pts.red[839].value    = 0;
    post1D->tf_pts.green[839].value  = 0;
    post1D->tf_pts.blue[839].value   = 0;
    post1D->tf_pts.red[840].value    = 0;
    post1D->tf_pts.green[840].value  = 0;
    post1D->tf_pts.blue[840].value   = 0;
    post1D->tf_pts.red[841].value    = 0;
    post1D->tf_pts.green[841].value  = 0;
    post1D->tf_pts.blue[841].value   = 0;
    post1D->tf_pts.red[842].value    = 0;
    post1D->tf_pts.green[842].value  = 0;
    post1D->tf_pts.blue[842].value   = 0;
    post1D->tf_pts.red[843].value    = 0;
    post1D->tf_pts.green[843].value  = 0;
    post1D->tf_pts.blue[843].value   = 0;
    post1D->tf_pts.red[844].value    = 0;
    post1D->tf_pts.green[844].value  = 0;
    post1D->tf_pts.blue[844].value   = 0;
    post1D->tf_pts.red[845].value    = 0;
    post1D->tf_pts.green[845].value  = 0;
    post1D->tf_pts.blue[845].value   = 0;
    post1D->tf_pts.red[846].value    = 0;
    post1D->tf_pts.green[846].value  = 0;
    post1D->tf_pts.blue[846].value   = 0;
    post1D->tf_pts.red[847].value    = 0;
    post1D->tf_pts.green[847].value  = 0;
    post1D->tf_pts.blue[847].value   = 0;
    post1D->tf_pts.red[848].value    = 0;
    post1D->tf_pts.green[848].value  = 0;
    post1D->tf_pts.blue[848].value   = 0;
    post1D->tf_pts.red[849].value    = 0;
    post1D->tf_pts.green[849].value  = 0;
    post1D->tf_pts.blue[849].value   = 0;
    post1D->tf_pts.red[850].value    = 0;
    post1D->tf_pts.green[850].value  = 0;
    post1D->tf_pts.blue[850].value   = 0;
    post1D->tf_pts.red[851].value    = 0;
    post1D->tf_pts.green[851].value  = 0;
    post1D->tf_pts.blue[851].value   = 0;
    post1D->tf_pts.red[852].value    = 0;
    post1D->tf_pts.green[852].value  = 0;
    post1D->tf_pts.blue[852].value   = 0;
    post1D->tf_pts.red[853].value    = 0;
    post1D->tf_pts.green[853].value  = 0;
    post1D->tf_pts.blue[853].value   = 0;
    post1D->tf_pts.red[854].value    = 0;
    post1D->tf_pts.green[854].value  = 0;
    post1D->tf_pts.blue[854].value   = 0;
    post1D->tf_pts.red[855].value    = 0;
    post1D->tf_pts.green[855].value  = 0;
    post1D->tf_pts.blue[855].value   = 0;
    post1D->tf_pts.red[856].value    = 0;
    post1D->tf_pts.green[856].value  = 0;
    post1D->tf_pts.blue[856].value   = 0;
    post1D->tf_pts.red[857].value    = 0;
    post1D->tf_pts.green[857].value  = 0;
    post1D->tf_pts.blue[857].value   = 0;
    post1D->tf_pts.red[858].value    = 0;
    post1D->tf_pts.green[858].value  = 0;
    post1D->tf_pts.blue[858].value   = 0;
    post1D->tf_pts.red[859].value    = 0;
    post1D->tf_pts.green[859].value  = 0;
    post1D->tf_pts.blue[859].value   = 0;
    post1D->tf_pts.red[860].value    = 0;
    post1D->tf_pts.green[860].value  = 0;
    post1D->tf_pts.blue[860].value   = 0;
    post1D->tf_pts.red[861].value    = 0;
    post1D->tf_pts.green[861].value  = 0;
    post1D->tf_pts.blue[861].value   = 0;
    post1D->tf_pts.red[862].value    = 0;
    post1D->tf_pts.green[862].value  = 0;
    post1D->tf_pts.blue[862].value   = 0;
    post1D->tf_pts.red[863].value    = 0;
    post1D->tf_pts.green[863].value  = 0;
    post1D->tf_pts.blue[863].value   = 0;
    post1D->tf_pts.red[864].value    = 0;
    post1D->tf_pts.green[864].value  = 0;
    post1D->tf_pts.blue[864].value   = 0;
    post1D->tf_pts.red[865].value    = 0;
    post1D->tf_pts.green[865].value  = 0;
    post1D->tf_pts.blue[865].value   = 0;
    post1D->tf_pts.red[866].value    = 0;
    post1D->tf_pts.green[866].value  = 0;
    post1D->tf_pts.blue[866].value   = 0;
    post1D->tf_pts.red[867].value    = 0;
    post1D->tf_pts.green[867].value  = 0;
    post1D->tf_pts.blue[867].value   = 0;
    post1D->tf_pts.red[868].value    = 0;
    post1D->tf_pts.green[868].value  = 0;
    post1D->tf_pts.blue[868].value   = 0;
    post1D->tf_pts.red[869].value    = 0;
    post1D->tf_pts.green[869].value  = 0;
    post1D->tf_pts.blue[869].value   = 0;
    post1D->tf_pts.red[870].value    = 0;
    post1D->tf_pts.green[870].value  = 0;
    post1D->tf_pts.blue[870].value   = 0;
    post1D->tf_pts.red[871].value    = 0;
    post1D->tf_pts.green[871].value  = 0;
    post1D->tf_pts.blue[871].value   = 0;
    post1D->tf_pts.red[872].value    = 0;
    post1D->tf_pts.green[872].value  = 0;
    post1D->tf_pts.blue[872].value   = 0;
    post1D->tf_pts.red[873].value    = 0;
    post1D->tf_pts.green[873].value  = 0;
    post1D->tf_pts.blue[873].value   = 0;
    post1D->tf_pts.red[874].value    = 0;
    post1D->tf_pts.green[874].value  = 0;
    post1D->tf_pts.blue[874].value   = 0;
    post1D->tf_pts.red[875].value    = 0;
    post1D->tf_pts.green[875].value  = 0;
    post1D->tf_pts.blue[875].value   = 0;
    post1D->tf_pts.red[876].value    = 0;
    post1D->tf_pts.green[876].value  = 0;
    post1D->tf_pts.blue[876].value   = 0;
    post1D->tf_pts.red[877].value    = 0;
    post1D->tf_pts.green[877].value  = 0;
    post1D->tf_pts.blue[877].value   = 0;
    post1D->tf_pts.red[878].value    = 0;
    post1D->tf_pts.green[878].value  = 0;
    post1D->tf_pts.blue[878].value   = 0;
    post1D->tf_pts.red[879].value    = 0;
    post1D->tf_pts.green[879].value  = 0;
    post1D->tf_pts.blue[879].value   = 0;
    post1D->tf_pts.red[880].value    = 0;
    post1D->tf_pts.green[880].value  = 0;
    post1D->tf_pts.blue[880].value   = 0;
    post1D->tf_pts.red[881].value    = 0;
    post1D->tf_pts.green[881].value  = 0;
    post1D->tf_pts.blue[881].value   = 0;
    post1D->tf_pts.red[882].value    = 0;
    post1D->tf_pts.green[882].value  = 0;
    post1D->tf_pts.blue[882].value   = 0;
    post1D->tf_pts.red[883].value    = 0;
    post1D->tf_pts.green[883].value  = 0;
    post1D->tf_pts.blue[883].value   = 0;
    post1D->tf_pts.red[884].value    = 0;
    post1D->tf_pts.green[884].value  = 0;
    post1D->tf_pts.blue[884].value   = 0;
    post1D->tf_pts.red[885].value    = 0;
    post1D->tf_pts.green[885].value  = 0;
    post1D->tf_pts.blue[885].value   = 0;
    post1D->tf_pts.red[886].value    = 0;
    post1D->tf_pts.green[886].value  = 0;
    post1D->tf_pts.blue[886].value   = 0;
    post1D->tf_pts.red[887].value    = 0;
    post1D->tf_pts.green[887].value  = 0;
    post1D->tf_pts.blue[887].value   = 0;
    post1D->tf_pts.red[888].value    = 0;
    post1D->tf_pts.green[888].value  = 0;
    post1D->tf_pts.blue[888].value   = 0;
    post1D->tf_pts.red[889].value    = 0;
    post1D->tf_pts.green[889].value  = 0;
    post1D->tf_pts.blue[889].value   = 0;
    post1D->tf_pts.red[890].value    = 0;
    post1D->tf_pts.green[890].value  = 0;
    post1D->tf_pts.blue[890].value   = 0;
    post1D->tf_pts.red[891].value    = 0;
    post1D->tf_pts.green[891].value  = 0;
    post1D->tf_pts.blue[891].value   = 0;
    post1D->tf_pts.red[892].value    = 0;
    post1D->tf_pts.green[892].value  = 0;
    post1D->tf_pts.blue[892].value   = 0;
    post1D->tf_pts.red[893].value    = 0;
    post1D->tf_pts.green[893].value  = 0;
    post1D->tf_pts.blue[893].value   = 0;
    post1D->tf_pts.red[894].value    = 0;
    post1D->tf_pts.green[894].value  = 0;
    post1D->tf_pts.blue[894].value   = 0;
    post1D->tf_pts.red[895].value    = 0;
    post1D->tf_pts.green[895].value  = 0;
    post1D->tf_pts.blue[895].value   = 0;
    post1D->tf_pts.red[896].value    = 0;
    post1D->tf_pts.green[896].value  = 0;
    post1D->tf_pts.blue[896].value   = 0;
    post1D->tf_pts.red[897].value    = 0;
    post1D->tf_pts.green[897].value  = 0;
    post1D->tf_pts.blue[897].value   = 0;
    post1D->tf_pts.red[898].value    = 0;
    post1D->tf_pts.green[898].value  = 0;
    post1D->tf_pts.blue[898].value   = 0;
    post1D->tf_pts.red[899].value    = 0;
    post1D->tf_pts.green[899].value  = 0;
    post1D->tf_pts.blue[899].value   = 0;
    post1D->tf_pts.red[900].value    = 0;
    post1D->tf_pts.green[900].value  = 0;
    post1D->tf_pts.blue[900].value   = 0;
    post1D->tf_pts.red[901].value    = 0;
    post1D->tf_pts.green[901].value  = 0;
    post1D->tf_pts.blue[901].value   = 0;
    post1D->tf_pts.red[902].value    = 0;
    post1D->tf_pts.green[902].value  = 0;
    post1D->tf_pts.blue[902].value   = 0;
    post1D->tf_pts.red[903].value    = 0;
    post1D->tf_pts.green[903].value  = 0;
    post1D->tf_pts.blue[903].value   = 0;
    post1D->tf_pts.red[904].value    = 0;
    post1D->tf_pts.green[904].value  = 0;
    post1D->tf_pts.blue[904].value   = 0;
    post1D->tf_pts.red[905].value    = 0;
    post1D->tf_pts.green[905].value  = 0;
    post1D->tf_pts.blue[905].value   = 0;
    post1D->tf_pts.red[906].value    = 0;
    post1D->tf_pts.green[906].value  = 0;
    post1D->tf_pts.blue[906].value   = 0;
    post1D->tf_pts.red[907].value    = 0;
    post1D->tf_pts.green[907].value  = 0;
    post1D->tf_pts.blue[907].value   = 0;
    post1D->tf_pts.red[908].value    = 0;
    post1D->tf_pts.green[908].value  = 0;
    post1D->tf_pts.blue[908].value   = 0;
    post1D->tf_pts.red[909].value    = 0;
    post1D->tf_pts.green[909].value  = 0;
    post1D->tf_pts.blue[909].value   = 0;
    post1D->tf_pts.red[910].value    = 0;
    post1D->tf_pts.green[910].value  = 0;
    post1D->tf_pts.blue[910].value   = 0;
    post1D->tf_pts.red[911].value    = 0;
    post1D->tf_pts.green[911].value  = 0;
    post1D->tf_pts.blue[911].value   = 0;
    post1D->tf_pts.red[912].value    = 0;
    post1D->tf_pts.green[912].value  = 0;
    post1D->tf_pts.blue[912].value   = 0;
    post1D->tf_pts.red[913].value    = 0;
    post1D->tf_pts.green[913].value  = 0;
    post1D->tf_pts.blue[913].value   = 0;
    post1D->tf_pts.red[914].value    = 0;
    post1D->tf_pts.green[914].value  = 0;
    post1D->tf_pts.blue[914].value   = 0;
    post1D->tf_pts.red[915].value    = 0;
    post1D->tf_pts.green[915].value  = 0;
    post1D->tf_pts.blue[915].value   = 0;
    post1D->tf_pts.red[916].value    = 0;
    post1D->tf_pts.green[916].value  = 0;
    post1D->tf_pts.blue[916].value   = 0;
    post1D->tf_pts.red[917].value    = 0;
    post1D->tf_pts.green[917].value  = 0;
    post1D->tf_pts.blue[917].value   = 0;
    post1D->tf_pts.red[918].value    = 0;
    post1D->tf_pts.green[918].value  = 0;
    post1D->tf_pts.blue[918].value   = 0;
    post1D->tf_pts.red[919].value    = 0;
    post1D->tf_pts.green[919].value  = 0;
    post1D->tf_pts.blue[919].value   = 0;
    post1D->tf_pts.red[920].value    = 0;
    post1D->tf_pts.green[920].value  = 0;
    post1D->tf_pts.blue[920].value   = 0;
    post1D->tf_pts.red[921].value    = 0;
    post1D->tf_pts.green[921].value  = 0;
    post1D->tf_pts.blue[921].value   = 0;
    post1D->tf_pts.red[922].value    = 0;
    post1D->tf_pts.green[922].value  = 0;
    post1D->tf_pts.blue[922].value   = 0;
    post1D->tf_pts.red[923].value    = 0;
    post1D->tf_pts.green[923].value  = 0;
    post1D->tf_pts.blue[923].value   = 0;
    post1D->tf_pts.red[924].value    = 0;
    post1D->tf_pts.green[924].value  = 0;
    post1D->tf_pts.blue[924].value   = 0;
    post1D->tf_pts.red[925].value    = 0;
    post1D->tf_pts.green[925].value  = 0;
    post1D->tf_pts.blue[925].value   = 0;
    post1D->tf_pts.red[926].value    = 0;
    post1D->tf_pts.green[926].value  = 0;
    post1D->tf_pts.blue[926].value   = 0;
    post1D->tf_pts.red[927].value    = 0;
    post1D->tf_pts.green[927].value  = 0;
    post1D->tf_pts.blue[927].value   = 0;
    post1D->tf_pts.red[928].value    = 0;
    post1D->tf_pts.green[928].value  = 0;
    post1D->tf_pts.blue[928].value   = 0;
    post1D->tf_pts.red[929].value    = 0;
    post1D->tf_pts.green[929].value  = 0;
    post1D->tf_pts.blue[929].value   = 0;
    post1D->tf_pts.red[930].value    = 0;
    post1D->tf_pts.green[930].value  = 0;
    post1D->tf_pts.blue[930].value   = 0;
    post1D->tf_pts.red[931].value    = 0;
    post1D->tf_pts.green[931].value  = 0;
    post1D->tf_pts.blue[931].value   = 0;
    post1D->tf_pts.red[932].value    = 0;
    post1D->tf_pts.green[932].value  = 0;
    post1D->tf_pts.blue[932].value   = 0;
    post1D->tf_pts.red[933].value    = 0;
    post1D->tf_pts.green[933].value  = 0;
    post1D->tf_pts.blue[933].value   = 0;
    post1D->tf_pts.red[934].value    = 0;
    post1D->tf_pts.green[934].value  = 0;
    post1D->tf_pts.blue[934].value   = 0;
    post1D->tf_pts.red[935].value    = 0;
    post1D->tf_pts.green[935].value  = 0;
    post1D->tf_pts.blue[935].value   = 0;
    post1D->tf_pts.red[936].value    = 0;
    post1D->tf_pts.green[936].value  = 0;
    post1D->tf_pts.blue[936].value   = 0;
    post1D->tf_pts.red[937].value    = 0;
    post1D->tf_pts.green[937].value  = 0;
    post1D->tf_pts.blue[937].value   = 0;
    post1D->tf_pts.red[938].value    = 0;
    post1D->tf_pts.green[938].value  = 0;
    post1D->tf_pts.blue[938].value   = 0;
    post1D->tf_pts.red[939].value    = 0;
    post1D->tf_pts.green[939].value  = 0;
    post1D->tf_pts.blue[939].value   = 0;
    post1D->tf_pts.red[940].value    = 0;
    post1D->tf_pts.green[940].value  = 0;
    post1D->tf_pts.blue[940].value   = 0;
    post1D->tf_pts.red[941].value    = 0;
    post1D->tf_pts.green[941].value  = 0;
    post1D->tf_pts.blue[941].value   = 0;
    post1D->tf_pts.red[942].value    = 0;
    post1D->tf_pts.green[942].value  = 0;
    post1D->tf_pts.blue[942].value   = 0;
    post1D->tf_pts.red[943].value    = 0;
    post1D->tf_pts.green[943].value  = 0;
    post1D->tf_pts.blue[943].value   = 0;
    post1D->tf_pts.red[944].value    = 0;
    post1D->tf_pts.green[944].value  = 0;
    post1D->tf_pts.blue[944].value   = 0;
    post1D->tf_pts.red[945].value    = 0;
    post1D->tf_pts.green[945].value  = 0;
    post1D->tf_pts.blue[945].value   = 0;
    post1D->tf_pts.red[946].value    = 0;
    post1D->tf_pts.green[946].value  = 0;
    post1D->tf_pts.blue[946].value   = 0;
    post1D->tf_pts.red[947].value    = 0;
    post1D->tf_pts.green[947].value  = 0;
    post1D->tf_pts.blue[947].value   = 0;
    post1D->tf_pts.red[948].value    = 0;
    post1D->tf_pts.green[948].value  = 0;
    post1D->tf_pts.blue[948].value   = 0;
    post1D->tf_pts.red[949].value    = 0;
    post1D->tf_pts.green[949].value  = 0;
    post1D->tf_pts.blue[949].value   = 0;
    post1D->tf_pts.red[950].value    = 0;
    post1D->tf_pts.green[950].value  = 0;
    post1D->tf_pts.blue[950].value   = 0;
    post1D->tf_pts.red[951].value    = 0;
    post1D->tf_pts.green[951].value  = 0;
    post1D->tf_pts.blue[951].value   = 0;
    post1D->tf_pts.red[952].value    = 0;
    post1D->tf_pts.green[952].value  = 0;
    post1D->tf_pts.blue[952].value   = 0;
    post1D->tf_pts.red[953].value    = 0;
    post1D->tf_pts.green[953].value  = 0;
    post1D->tf_pts.blue[953].value   = 0;
    post1D->tf_pts.red[954].value    = 0;
    post1D->tf_pts.green[954].value  = 0;
    post1D->tf_pts.blue[954].value   = 0;
    post1D->tf_pts.red[955].value    = 0;
    post1D->tf_pts.green[955].value  = 0;
    post1D->tf_pts.blue[955].value   = 0;
    post1D->tf_pts.red[956].value    = 0;
    post1D->tf_pts.green[956].value  = 0;
    post1D->tf_pts.blue[956].value   = 0;
    post1D->tf_pts.red[957].value    = 0;
    post1D->tf_pts.green[957].value  = 0;
    post1D->tf_pts.blue[957].value   = 0;
    post1D->tf_pts.red[958].value    = 0;
    post1D->tf_pts.green[958].value  = 0;
    post1D->tf_pts.blue[958].value   = 0;
    post1D->tf_pts.red[959].value    = 0;
    post1D->tf_pts.green[959].value  = 0;
    post1D->tf_pts.blue[959].value   = 0;
    post1D->tf_pts.red[960].value    = 0;
    post1D->tf_pts.green[960].value  = 0;
    post1D->tf_pts.blue[960].value   = 0;
    post1D->tf_pts.red[961].value    = 0;
    post1D->tf_pts.green[961].value  = 0;
    post1D->tf_pts.blue[961].value   = 0;
    post1D->tf_pts.red[962].value    = 0;
    post1D->tf_pts.green[962].value  = 0;
    post1D->tf_pts.blue[962].value   = 0;
    post1D->tf_pts.red[963].value    = 0;
    post1D->tf_pts.green[963].value  = 0;
    post1D->tf_pts.blue[963].value   = 0;
    post1D->tf_pts.red[964].value    = 0;
    post1D->tf_pts.green[964].value  = 0;
    post1D->tf_pts.blue[964].value   = 0;
    post1D->tf_pts.red[965].value    = 0;
    post1D->tf_pts.green[965].value  = 0;
    post1D->tf_pts.blue[965].value   = 0;
    post1D->tf_pts.red[966].value    = 0;
    post1D->tf_pts.green[966].value  = 0;
    post1D->tf_pts.blue[966].value   = 0;
    post1D->tf_pts.red[967].value    = 0;
    post1D->tf_pts.green[967].value  = 0;
    post1D->tf_pts.blue[967].value   = 0;
    post1D->tf_pts.red[968].value    = 0;
    post1D->tf_pts.green[968].value  = 0;
    post1D->tf_pts.blue[968].value   = 0;
    post1D->tf_pts.red[969].value    = 0;
    post1D->tf_pts.green[969].value  = 0;
    post1D->tf_pts.blue[969].value   = 0;
    post1D->tf_pts.red[970].value    = 0;
    post1D->tf_pts.green[970].value  = 0;
    post1D->tf_pts.blue[970].value   = 0;
    post1D->tf_pts.red[971].value    = 0;
    post1D->tf_pts.green[971].value  = 0;
    post1D->tf_pts.blue[971].value   = 0;
    post1D->tf_pts.red[972].value    = 0;
    post1D->tf_pts.green[972].value  = 0;
    post1D->tf_pts.blue[972].value   = 0;
    post1D->tf_pts.red[973].value    = 0;
    post1D->tf_pts.green[973].value  = 0;
    post1D->tf_pts.blue[973].value   = 0;
    post1D->tf_pts.red[974].value    = 0;
    post1D->tf_pts.green[974].value  = 0;
    post1D->tf_pts.blue[974].value   = 0;
    post1D->tf_pts.red[975].value    = 0;
    post1D->tf_pts.green[975].value  = 0;
    post1D->tf_pts.blue[975].value   = 0;
    post1D->tf_pts.red[976].value    = 0;
    post1D->tf_pts.green[976].value  = 0;
    post1D->tf_pts.blue[976].value   = 0;
    post1D->tf_pts.red[977].value    = 0;
    post1D->tf_pts.green[977].value  = 0;
    post1D->tf_pts.blue[977].value   = 0;
    post1D->tf_pts.red[978].value    = 0;
    post1D->tf_pts.green[978].value  = 0;
    post1D->tf_pts.blue[978].value   = 0;
    post1D->tf_pts.red[979].value    = 0;
    post1D->tf_pts.green[979].value  = 0;
    post1D->tf_pts.blue[979].value   = 0;
    post1D->tf_pts.red[980].value    = 0;
    post1D->tf_pts.green[980].value  = 0;
    post1D->tf_pts.blue[980].value   = 0;
    post1D->tf_pts.red[981].value    = 0;
    post1D->tf_pts.green[981].value  = 0;
    post1D->tf_pts.blue[981].value   = 0;
    post1D->tf_pts.red[982].value    = 0;
    post1D->tf_pts.green[982].value  = 0;
    post1D->tf_pts.blue[982].value   = 0;
    post1D->tf_pts.red[983].value    = 0;
    post1D->tf_pts.green[983].value  = 0;
    post1D->tf_pts.blue[983].value   = 0;
    post1D->tf_pts.red[984].value    = 0;
    post1D->tf_pts.green[984].value  = 0;
    post1D->tf_pts.blue[984].value   = 0;
    post1D->tf_pts.red[985].value    = 0;
    post1D->tf_pts.green[985].value  = 0;
    post1D->tf_pts.blue[985].value   = 0;
    post1D->tf_pts.red[986].value    = 0;
    post1D->tf_pts.green[986].value  = 0;
    post1D->tf_pts.blue[986].value   = 0;
    post1D->tf_pts.red[987].value    = 0;
    post1D->tf_pts.green[987].value  = 0;
    post1D->tf_pts.blue[987].value   = 0;
    post1D->tf_pts.red[988].value    = 0;
    post1D->tf_pts.green[988].value  = 0;
    post1D->tf_pts.blue[988].value   = 0;
    post1D->tf_pts.red[989].value    = 0;
    post1D->tf_pts.green[989].value  = 0;
    post1D->tf_pts.blue[989].value   = 0;
    post1D->tf_pts.red[990].value    = 0;
    post1D->tf_pts.green[990].value  = 0;
    post1D->tf_pts.blue[990].value   = 0;
    post1D->tf_pts.red[991].value    = 0;
    post1D->tf_pts.green[991].value  = 0;
    post1D->tf_pts.blue[991].value   = 0;
    post1D->tf_pts.red[992].value    = 0;
    post1D->tf_pts.green[992].value  = 0;
    post1D->tf_pts.blue[992].value   = 0;
    post1D->tf_pts.red[993].value    = 0;
    post1D->tf_pts.green[993].value  = 0;
    post1D->tf_pts.blue[993].value   = 0;
    post1D->tf_pts.red[994].value    = 0;
    post1D->tf_pts.green[994].value  = 0;
    post1D->tf_pts.blue[994].value   = 0;
    post1D->tf_pts.red[995].value    = 0;
    post1D->tf_pts.green[995].value  = 0;
    post1D->tf_pts.blue[995].value   = 0;
    post1D->tf_pts.red[996].value    = 0;
    post1D->tf_pts.green[996].value  = 0;
    post1D->tf_pts.blue[996].value   = 0;
    post1D->tf_pts.red[997].value    = 0;
    post1D->tf_pts.green[997].value  = 0;
    post1D->tf_pts.blue[997].value   = 0;
    post1D->tf_pts.red[998].value    = 0;
    post1D->tf_pts.green[998].value  = 0;
    post1D->tf_pts.blue[998].value   = 0;
    post1D->tf_pts.red[999].value    = 0;
    post1D->tf_pts.green[999].value  = 0;
    post1D->tf_pts.blue[999].value   = 0;
    post1D->tf_pts.red[1000].value   = 0;
    post1D->tf_pts.green[1000].value = 0;
    post1D->tf_pts.blue[1000].value  = 0;
    post1D->tf_pts.red[1001].value   = 0;
    post1D->tf_pts.green[1001].value = 0;
    post1D->tf_pts.blue[1001].value  = 0;
    post1D->tf_pts.red[1002].value   = 0;
    post1D->tf_pts.green[1002].value = 0;
    post1D->tf_pts.blue[1002].value  = 0;
    post1D->tf_pts.red[1003].value   = 0;
    post1D->tf_pts.green[1003].value = 0;
    post1D->tf_pts.blue[1003].value  = 0;
    post1D->tf_pts.red[1004].value   = 0;
    post1D->tf_pts.green[1004].value = 0;
    post1D->tf_pts.blue[1004].value  = 0;
    post1D->tf_pts.red[1005].value   = 0;
    post1D->tf_pts.green[1005].value = 0;
    post1D->tf_pts.blue[1005].value  = 0;
    post1D->tf_pts.red[1006].value   = 0;
    post1D->tf_pts.green[1006].value = 0;
    post1D->tf_pts.blue[1006].value  = 0;
    post1D->tf_pts.red[1007].value   = 0;
    post1D->tf_pts.green[1007].value = 0;
    post1D->tf_pts.blue[1007].value  = 0;
    post1D->tf_pts.red[1008].value   = 0;
    post1D->tf_pts.green[1008].value = 0;
    post1D->tf_pts.blue[1008].value  = 0;
    post1D->tf_pts.red[1009].value   = 0;
    post1D->tf_pts.green[1009].value = 0;
    post1D->tf_pts.blue[1009].value  = 0;
    post1D->tf_pts.red[1010].value   = 0;
    post1D->tf_pts.green[1010].value = 0;
    post1D->tf_pts.blue[1010].value  = 0;
    post1D->tf_pts.red[1011].value   = 0;
    post1D->tf_pts.green[1011].value = 0;
    post1D->tf_pts.blue[1011].value  = 0;
    post1D->tf_pts.red[1012].value   = 0;
    post1D->tf_pts.green[1012].value = 0;
    post1D->tf_pts.blue[1012].value  = 0;
    post1D->tf_pts.red[1013].value   = 0;
    post1D->tf_pts.green[1013].value = 0;
    post1D->tf_pts.blue[1013].value  = 0;
    post1D->tf_pts.red[1014].value   = 0;
    post1D->tf_pts.green[1014].value = 0;
    post1D->tf_pts.blue[1014].value  = 0;
    post1D->tf_pts.red[1015].value   = 0;
    post1D->tf_pts.green[1015].value = 0;
    post1D->tf_pts.blue[1015].value  = 0;
    post1D->tf_pts.red[1016].value   = 0;
    post1D->tf_pts.green[1016].value = 0;
    post1D->tf_pts.blue[1016].value  = 0;
    post1D->tf_pts.red[1017].value   = 0;
    post1D->tf_pts.green[1017].value = 0;
    post1D->tf_pts.blue[1017].value  = 0;
    post1D->tf_pts.red[1018].value   = 0;
    post1D->tf_pts.green[1018].value = 0;
    post1D->tf_pts.blue[1018].value  = 0;
    post1D->tf_pts.red[1019].value   = 0;
    post1D->tf_pts.green[1019].value = 0;
    post1D->tf_pts.blue[1019].value  = 0;
    post1D->tf_pts.red[1020].value   = 0;
    post1D->tf_pts.green[1020].value = 0;
    post1D->tf_pts.blue[1020].value  = 0;
    post1D->tf_pts.red[1021].value   = 0;
    post1D->tf_pts.green[1021].value = 0;
    post1D->tf_pts.blue[1021].value  = 0;
    post1D->tf_pts.red[1022].value   = 0;
    post1D->tf_pts.green[1022].value = 0;
    post1D->tf_pts.blue[1022].value  = 0;
    post1D->tf_pts.red[1023].value   = 0;
    post1D->tf_pts.green[1023].value = 0;
    post1D->tf_pts.blue[1023].value  = 0;
    post1D->tf_pts.red[1024].value   = 0;
    post1D->tf_pts.green[1024].value = 0;
    post1D->tf_pts.blue[1024].value  = 0;

    post1D->tf_pts.end_exponent        = 0;
    post1D->tf_pts.x_point_at_y1_red   = 1;
    post1D->tf_pts.x_point_at_y1_green = 1;
    post1D->tf_pts.x_point_at_y1_blue  = 1;

    return true;
}

static const struct tetrahedral_17x17x17 tetra_ident_3dlut = {
    {{0, 0, 0}, {1023, 0, 0}, {2047, 0, 0}, {3071, 0, 0}, {4095, 0, 0}, {767, 255, 0},
        {1791, 255, 0}, {2815, 255, 0}, {3839, 255, 0}, {511, 511, 0}, {1535, 511, 0},
        {2559, 511, 0}, {3583, 511, 0}, {255, 767, 0}, {1279, 767, 0}, {2303, 767, 0},
        {3327, 767, 0}, {0, 1023, 0}, {1023, 1023, 0}, {2047, 1023, 0}, {3071, 1023, 0},
        {4095, 1023, 0}, {767, 1279, 0}, {1791, 1279, 0}, {2815, 1279, 0}, {3839, 1279, 0},
        {511, 1535, 0}, {1535, 1535, 0}, {2559, 1535, 0}, {3583, 1535, 0}, {255, 1791, 0},
        {1279, 1791, 0}, {2303, 1791, 0}, {3327, 1791, 0}, {0, 2047, 0}, {1023, 2047, 0},
        {2047, 2047, 0}, {3071, 2047, 0}, {4095, 2047, 0}, {767, 2303, 0}, {1791, 2303, 0},
        {2815, 2303, 0}, {3839, 2303, 0}, {511, 2559, 0}, {1535, 2559, 0}, {2559, 2559, 0},
        {3583, 2559, 0}, {255, 2815, 0}, {1279, 2815, 0}, {2303, 2815, 0}, {3327, 2815, 0},
        {0, 3071, 0}, {1023, 3071, 0}, {2047, 3071, 0}, {3071, 3071, 0}, {4095, 3071, 0},
        {767, 3327, 0}, {1791, 3327, 0}, {2815, 3327, 0}, {3839, 3327, 0}, {511, 3583, 0},
        {1535, 3583, 0}, {2559, 3583, 0}, {3583, 3583, 0}, {255, 3839, 0}, {1279, 3839, 0},
        {2303, 3839, 0}, {3327, 3839, 0}, {0, 4095, 0}, {1023, 4095, 0}, {2047, 4095, 0},
        {3071, 4095, 0}, {4095, 4095, 0}, {767, 0, 255}, {1791, 0, 255}, {2815, 0, 255},
        {3839, 0, 255}, {511, 255, 255}, {1535, 255, 255}, {2559, 255, 255}, {3583, 255, 255},
        {255, 511, 255}, {1279, 511, 255}, {2303, 511, 255}, {3327, 511, 255}, {0, 767, 255},
        {1023, 767, 255}, {2047, 767, 255}, {3071, 767, 255}, {4095, 767, 255}, {767, 1023, 255},
        {1791, 1023, 255}, {2815, 1023, 255}, {3839, 1023, 255}, {511, 1279, 255},
        {1535, 1279, 255}, {2559, 1279, 255}, {3583, 1279, 255}, {255, 1535, 255},
        {1279, 1535, 255}, {2303, 1535, 255}, {3327, 1535, 255}, {0, 1791, 255}, {1023, 1791, 255},
        {2047, 1791, 255}, {3071, 1791, 255}, {4095, 1791, 255}, {767, 2047, 255},
        {1791, 2047, 255}, {2815, 2047, 255}, {3839, 2047, 255}, {511, 2303, 255},
        {1535, 2303, 255}, {2559, 2303, 255}, {3583, 2303, 255}, {255, 2559, 255},
        {1279, 2559, 255}, {2303, 2559, 255}, {3327, 2559, 255}, {0, 2815, 255}, {1023, 2815, 255},
        {2047, 2815, 255}, {3071, 2815, 255}, {4095, 2815, 255}, {767, 3071, 255},
        {1791, 3071, 255}, {2815, 3071, 255}, {3839, 3071, 255}, {511, 3327, 255},
        {1535, 3327, 255}, {2559, 3327, 255}, {3583, 3327, 255}, {255, 3583, 255},
        {1279, 3583, 255}, {2303, 3583, 255}, {3327, 3583, 255}, {0, 3839, 255}, {1023, 3839, 255},
        {2047, 3839, 255}, {3071, 3839, 255}, {4095, 3839, 255}, {767, 4095, 255},
        {1791, 4095, 255}, {2815, 4095, 255}, {3839, 4095, 255}, {511, 0, 511}, {1535, 0, 511},
        {2559, 0, 511}, {3583, 0, 511}, {255, 255, 511}, {1279, 255, 511}, {2303, 255, 511},
        {3327, 255, 511}, {0, 511, 511}, {1023, 511, 511}, {2047, 511, 511}, {3071, 511, 511},
        {4095, 511, 511}, {767, 767, 511}, {1791, 767, 511}, {2815, 767, 511}, {3839, 767, 511},
        {511, 1023, 511}, {1535, 1023, 511}, {2559, 1023, 511}, {3583, 1023, 511}, {255, 1279, 511},
        {1279, 1279, 511}, {2303, 1279, 511}, {3327, 1279, 511}, {0, 1535, 511}, {1023, 1535, 511},
        {2047, 1535, 511}, {3071, 1535, 511}, {4095, 1535, 511}, {767, 1791, 511},
        {1791, 1791, 511}, {2815, 1791, 511}, {3839, 1791, 511}, {511, 2047, 511},
        {1535, 2047, 511}, {2559, 2047, 511}, {3583, 2047, 511}, {255, 2303, 511},
        {1279, 2303, 511}, {2303, 2303, 511}, {3327, 2303, 511}, {0, 2559, 511}, {1023, 2559, 511},
        {2047, 2559, 511}, {3071, 2559, 511}, {4095, 2559, 511}, {767, 2815, 511},
        {1791, 2815, 511}, {2815, 2815, 511}, {3839, 2815, 511}, {511, 3071, 511},
        {1535, 3071, 511}, {2559, 3071, 511}, {3583, 3071, 511}, {255, 3327, 511},
        {1279, 3327, 511}, {2303, 3327, 511}, {3327, 3327, 511}, {0, 3583, 511}, {1023, 3583, 511},
        {2047, 3583, 511}, {3071, 3583, 511}, {4095, 3583, 511}, {767, 3839, 511},
        {1791, 3839, 511}, {2815, 3839, 511}, {3839, 3839, 511}, {511, 4095, 511},
        {1535, 4095, 511}, {2559, 4095, 511}, {3583, 4095, 511}, {255, 0, 767}, {1279, 0, 767},
        {2303, 0, 767}, {3327, 0, 767}, {0, 255, 767}, {1023, 255, 767}, {2047, 255, 767},
        {3071, 255, 767}, {4095, 255, 767}, {767, 511, 767}, {1791, 511, 767}, {2815, 511, 767},
        {3839, 511, 767}, {511, 767, 767}, {1535, 767, 767}, {2559, 767, 767}, {3583, 767, 767},
        {255, 1023, 767}, {1279, 1023, 767}, {2303, 1023, 767}, {3327, 1023, 767}, {0, 1279, 767},
        {1023, 1279, 767}, {2047, 1279, 767}, {3071, 1279, 767}, {4095, 1279, 767},
        {767, 1535, 767}, {1791, 1535, 767}, {2815, 1535, 767}, {3839, 1535, 767}, {511, 1791, 767},
        {1535, 1791, 767}, {2559, 1791, 767}, {3583, 1791, 767}, {255, 2047, 767},
        {1279, 2047, 767}, {2303, 2047, 767}, {3327, 2047, 767}, {0, 2303, 767}, {1023, 2303, 767},
        {2047, 2303, 767}, {3071, 2303, 767}, {4095, 2303, 767}, {767, 2559, 767},
        {1791, 2559, 767}, {2815, 2559, 767}, {3839, 2559, 767}, {511, 2815, 767},
        {1535, 2815, 767}, {2559, 2815, 767}, {3583, 2815, 767}, {255, 3071, 767},
        {1279, 3071, 767}, {2303, 3071, 767}, {3327, 3071, 767}, {0, 3327, 767}, {1023, 3327, 767},
        {2047, 3327, 767}, {3071, 3327, 767}, {4095, 3327, 767}, {767, 3583, 767},
        {1791, 3583, 767}, {2815, 3583, 767}, {3839, 3583, 767}, {511, 3839, 767},
        {1535, 3839, 767}, {2559, 3839, 767}, {3583, 3839, 767}, {255, 4095, 767},
        {1279, 4095, 767}, {2303, 4095, 767}, {3327, 4095, 767}, {0, 0, 1023}, {1023, 0, 1023},
        {2047, 0, 1023}, {3071, 0, 1023}, {4095, 0, 1023}, {767, 255, 1023}, {1791, 255, 1023},
        {2815, 255, 1023}, {3839, 255, 1023}, {511, 511, 1023}, {1535, 511, 1023},
        {2559, 511, 1023}, {3583, 511, 1023}, {255, 767, 1023}, {1279, 767, 1023},
        {2303, 767, 1023}, {3327, 767, 1023}, {0, 1023, 1023}, {1023, 1023, 1023},
        {2047, 1023, 1023}, {3071, 1023, 1023}, {4095, 1023, 1023}, {767, 1279, 1023},
        {1791, 1279, 1023}, {2815, 1279, 1023}, {3839, 1279, 1023}, {511, 1535, 1023},
        {1535, 1535, 1023}, {2559, 1535, 1023}, {3583, 1535, 1023}, {255, 1791, 1023},
        {1279, 1791, 1023}, {2303, 1791, 1023}, {3327, 1791, 1023}, {0, 2047, 1023},
        {1023, 2047, 1023}, {2047, 2047, 1023}, {3071, 2047, 1023}, {4095, 2047, 1023},
        {767, 2303, 1023}, {1791, 2303, 1023}, {2815, 2303, 1023}, {3839, 2303, 1023},
        {511, 2559, 1023}, {1535, 2559, 1023}, {2559, 2559, 1023}, {3583, 2559, 1023},
        {255, 2815, 1023}, {1279, 2815, 1023}, {2303, 2815, 1023}, {3327, 2815, 1023},
        {0, 3071, 1023}, {1023, 3071, 1023}, {2047, 3071, 1023}, {3071, 3071, 1023},
        {4095, 3071, 1023}, {767, 3327, 1023}, {1791, 3327, 1023}, {2815, 3327, 1023},
        {3839, 3327, 1023}, {511, 3583, 1023}, {1535, 3583, 1023}, {2559, 3583, 1023},
        {3583, 3583, 1023}, {255, 3839, 1023}, {1279, 3839, 1023}, {2303, 3839, 1023},
        {3327, 3839, 1023}, {0, 4095, 1023}, {1023, 4095, 1023}, {2047, 4095, 1023},
        {3071, 4095, 1023}, {4095, 4095, 1023}, {767, 0, 1279}, {1791, 0, 1279}, {2815, 0, 1279},
        {3839, 0, 1279}, {511, 255, 1279}, {1535, 255, 1279}, {2559, 255, 1279}, {3583, 255, 1279},
        {255, 511, 1279}, {1279, 511, 1279}, {2303, 511, 1279}, {3327, 511, 1279}, {0, 767, 1279},
        {1023, 767, 1279}, {2047, 767, 1279}, {3071, 767, 1279}, {4095, 767, 1279},
        {767, 1023, 1279}, {1791, 1023, 1279}, {2815, 1023, 1279}, {3839, 1023, 1279},
        {511, 1279, 1279}, {1535, 1279, 1279}, {2559, 1279, 1279}, {3583, 1279, 1279},
        {255, 1535, 1279}, {1279, 1535, 1279}, {2303, 1535, 1279}, {3327, 1535, 1279},
        {0, 1791, 1279}, {1023, 1791, 1279}, {2047, 1791, 1279}, {3071, 1791, 1279},
        {4095, 1791, 1279}, {767, 2047, 1279}, {1791, 2047, 1279}, {2815, 2047, 1279},
        {3839, 2047, 1279}, {511, 2303, 1279}, {1535, 2303, 1279}, {2559, 2303, 1279},
        {3583, 2303, 1279}, {255, 2559, 1279}, {1279, 2559, 1279}, {2303, 2559, 1279},
        {3327, 2559, 1279}, {0, 2815, 1279}, {1023, 2815, 1279}, {2047, 2815, 1279},
        {3071, 2815, 1279}, {4095, 2815, 1279}, {767, 3071, 1279}, {1791, 3071, 1279},
        {2815, 3071, 1279}, {3839, 3071, 1279}, {511, 3327, 1279}, {1535, 3327, 1279},
        {2559, 3327, 1279}, {3583, 3327, 1279}, {255, 3583, 1279}, {1279, 3583, 1279},
        {2303, 3583, 1279}, {3327, 3583, 1279}, {0, 3839, 1279}, {1023, 3839, 1279},
        {2047, 3839, 1279}, {3071, 3839, 1279}, {4095, 3839, 1279}, {767, 4095, 1279},
        {1791, 4095, 1279}, {2815, 4095, 1279}, {3839, 4095, 1279}, {511, 0, 1535}, {1535, 0, 1535},
        {2559, 0, 1535}, {3583, 0, 1535}, {255, 255, 1535}, {1279, 255, 1535}, {2303, 255, 1535},
        {3327, 255, 1535}, {0, 511, 1535}, {1023, 511, 1535}, {2047, 511, 1535}, {3071, 511, 1535},
        {4095, 511, 1535}, {767, 767, 1535}, {1791, 767, 1535}, {2815, 767, 1535},
        {3839, 767, 1535}, {511, 1023, 1535}, {1535, 1023, 1535}, {2559, 1023, 1535},
        {3583, 1023, 1535}, {255, 1279, 1535}, {1279, 1279, 1535}, {2303, 1279, 1535},
        {3327, 1279, 1535}, {0, 1535, 1535}, {1023, 1535, 1535}, {2047, 1535, 1535},
        {3071, 1535, 1535}, {4095, 1535, 1535}, {767, 1791, 1535}, {1791, 1791, 1535},
        {2815, 1791, 1535}, {3839, 1791, 1535}, {511, 2047, 1535}, {1535, 2047, 1535},
        {2559, 2047, 1535}, {3583, 2047, 1535}, {255, 2303, 1535}, {1279, 2303, 1535},
        {2303, 2303, 1535}, {3327, 2303, 1535}, {0, 2559, 1535}, {1023, 2559, 1535},
        {2047, 2559, 1535}, {3071, 2559, 1535}, {4095, 2559, 1535}, {767, 2815, 1535},
        {1791, 2815, 1535}, {2815, 2815, 1535}, {3839, 2815, 1535}, {511, 3071, 1535},
        {1535, 3071, 1535}, {2559, 3071, 1535}, {3583, 3071, 1535}, {255, 3327, 1535},
        {1279, 3327, 1535}, {2303, 3327, 1535}, {3327, 3327, 1535}, {0, 3583, 1535},
        {1023, 3583, 1535}, {2047, 3583, 1535}, {3071, 3583, 1535}, {4095, 3583, 1535},
        {767, 3839, 1535}, {1791, 3839, 1535}, {2815, 3839, 1535}, {3839, 3839, 1535},
        {511, 4095, 1535}, {1535, 4095, 1535}, {2559, 4095, 1535}, {3583, 4095, 1535},
        {255, 0, 1791}, {1279, 0, 1791}, {2303, 0, 1791}, {3327, 0, 1791}, {0, 255, 1791},
        {1023, 255, 1791}, {2047, 255, 1791}, {3071, 255, 1791}, {4095, 255, 1791},
        {767, 511, 1791}, {1791, 511, 1791}, {2815, 511, 1791}, {3839, 511, 1791}, {511, 767, 1791},
        {1535, 767, 1791}, {2559, 767, 1791}, {3583, 767, 1791}, {255, 1023, 1791},
        {1279, 1023, 1791}, {2303, 1023, 1791}, {3327, 1023, 1791}, {0, 1279, 1791},
        {1023, 1279, 1791}, {2047, 1279, 1791}, {3071, 1279, 1791}, {4095, 1279, 1791},
        {767, 1535, 1791}, {1791, 1535, 1791}, {2815, 1535, 1791}, {3839, 1535, 1791},
        {511, 1791, 1791}, {1535, 1791, 1791}, {2559, 1791, 1791}, {3583, 1791, 1791},
        {255, 2047, 1791}, {1279, 2047, 1791}, {2303, 2047, 1791}, {3327, 2047, 1791},
        {0, 2303, 1791}, {1023, 2303, 1791}, {2047, 2303, 1791}, {3071, 2303, 1791},
        {4095, 2303, 1791}, {767, 2559, 1791}, {1791, 2559, 1791}, {2815, 2559, 1791},
        {3839, 2559, 1791}, {511, 2815, 1791}, {1535, 2815, 1791}, {2559, 2815, 1791},
        {3583, 2815, 1791}, {255, 3071, 1791}, {1279, 3071, 1791}, {2303, 3071, 1791},
        {3327, 3071, 1791}, {0, 3327, 1791}, {1023, 3327, 1791}, {2047, 3327, 1791},
        {3071, 3327, 1791}, {4095, 3327, 1791}, {767, 3583, 1791}, {1791, 3583, 1791},
        {2815, 3583, 1791}, {3839, 3583, 1791}, {511, 3839, 1791}, {1535, 3839, 1791},
        {2559, 3839, 1791}, {3583, 3839, 1791}, {255, 4095, 1791}, {1279, 4095, 1791},
        {2303, 4095, 1791}, {3327, 4095, 1791}, {0, 0, 2047}, {1023, 0, 2047}, {2047, 0, 2047},
        {3071, 0, 2047}, {4095, 0, 2047}, {767, 255, 2047}, {1791, 255, 2047}, {2815, 255, 2047},
        {3839, 255, 2047}, {511, 511, 2047}, {1535, 511, 2047}, {2559, 511, 2047},
        {3583, 511, 2047}, {255, 767, 2047}, {1279, 767, 2047}, {2303, 767, 2047},
        {3327, 767, 2047}, {0, 1023, 2047}, {1023, 1023, 2047}, {2047, 1023, 2047},
        {3071, 1023, 2047}, {4095, 1023, 2047}, {767, 1279, 2047}, {1791, 1279, 2047},
        {2815, 1279, 2047}, {3839, 1279, 2047}, {511, 1535, 2047}, {1535, 1535, 2047},
        {2559, 1535, 2047}, {3583, 1535, 2047}, {255, 1791, 2047}, {1279, 1791, 2047},
        {2303, 1791, 2047}, {3327, 1791, 2047}, {0, 2047, 2047}, {1023, 2047, 2047},
        {2047, 2047, 2047}, {3071, 2047, 2047}, {4095, 2047, 2047}, {767, 2303, 2047},
        {1791, 2303, 2047}, {2815, 2303, 2047}, {3839, 2303, 2047}, {511, 2559, 2047},
        {1535, 2559, 2047}, {2559, 2559, 2047}, {3583, 2559, 2047}, {255, 2815, 2047},
        {1279, 2815, 2047}, {2303, 2815, 2047}, {3327, 2815, 2047}, {0, 3071, 2047},
        {1023, 3071, 2047}, {2047, 3071, 2047}, {3071, 3071, 2047}, {4095, 3071, 2047},
        {767, 3327, 2047}, {1791, 3327, 2047}, {2815, 3327, 2047}, {3839, 3327, 2047},
        {511, 3583, 2047}, {1535, 3583, 2047}, {2559, 3583, 2047}, {3583, 3583, 2047},
        {255, 3839, 2047}, {1279, 3839, 2047}, {2303, 3839, 2047}, {3327, 3839, 2047},
        {0, 4095, 2047}, {1023, 4095, 2047}, {2047, 4095, 2047}, {3071, 4095, 2047},
        {4095, 4095, 2047}, {767, 0, 2303}, {1791, 0, 2303}, {2815, 0, 2303}, {3839, 0, 2303},
        {511, 255, 2303}, {1535, 255, 2303}, {2559, 255, 2303}, {3583, 255, 2303}, {255, 511, 2303},
        {1279, 511, 2303}, {2303, 511, 2303}, {3327, 511, 2303}, {0, 767, 2303}, {1023, 767, 2303},
        {2047, 767, 2303}, {3071, 767, 2303}, {4095, 767, 2303}, {767, 1023, 2303},
        {1791, 1023, 2303}, {2815, 1023, 2303}, {3839, 1023, 2303}, {511, 1279, 2303},
        {1535, 1279, 2303}, {2559, 1279, 2303}, {3583, 1279, 2303}, {255, 1535, 2303},
        {1279, 1535, 2303}, {2303, 1535, 2303}, {3327, 1535, 2303}, {0, 1791, 2303},
        {1023, 1791, 2303}, {2047, 1791, 2303}, {3071, 1791, 2303}, {4095, 1791, 2303},
        {767, 2047, 2303}, {1791, 2047, 2303}, {2815, 2047, 2303}, {3839, 2047, 2303},
        {511, 2303, 2303}, {1535, 2303, 2303}, {2559, 2303, 2303}, {3583, 2303, 2303},
        {255, 2559, 2303}, {1279, 2559, 2303}, {2303, 2559, 2303}, {3327, 2559, 2303},
        {0, 2815, 2303}, {1023, 2815, 2303}, {2047, 2815, 2303}, {3071, 2815, 2303},
        {4095, 2815, 2303}, {767, 3071, 2303}, {1791, 3071, 2303}, {2815, 3071, 2303},
        {3839, 3071, 2303}, {511, 3327, 2303}, {1535, 3327, 2303}, {2559, 3327, 2303},
        {3583, 3327, 2303}, {255, 3583, 2303}, {1279, 3583, 2303}, {2303, 3583, 2303},
        {3327, 3583, 2303}, {0, 3839, 2303}, {1023, 3839, 2303}, {2047, 3839, 2303},
        {3071, 3839, 2303}, {4095, 3839, 2303}, {767, 4095, 2303}, {1791, 4095, 2303},
        {2815, 4095, 2303}, {3839, 4095, 2303}, {511, 0, 2559}, {1535, 0, 2559}, {2559, 0, 2559},
        {3583, 0, 2559}, {255, 255, 2559}, {1279, 255, 2559}, {2303, 255, 2559}, {3327, 255, 2559},
        {0, 511, 2559}, {1023, 511, 2559}, {2047, 511, 2559}, {3071, 511, 2559}, {4095, 511, 2559},
        {767, 767, 2559}, {1791, 767, 2559}, {2815, 767, 2559}, {3839, 767, 2559},
        {511, 1023, 2559}, {1535, 1023, 2559}, {2559, 1023, 2559}, {3583, 1023, 2559},
        {255, 1279, 2559}, {1279, 1279, 2559}, {2303, 1279, 2559}, {3327, 1279, 2559},
        {0, 1535, 2559}, {1023, 1535, 2559}, {2047, 1535, 2559}, {3071, 1535, 2559},
        {4095, 1535, 2559}, {767, 1791, 2559}, {1791, 1791, 2559}, {2815, 1791, 2559},
        {3839, 1791, 2559}, {511, 2047, 2559}, {1535, 2047, 2559}, {2559, 2047, 2559},
        {3583, 2047, 2559}, {255, 2303, 2559}, {1279, 2303, 2559}, {2303, 2303, 2559},
        {3327, 2303, 2559}, {0, 2559, 2559}, {1023, 2559, 2559}, {2047, 2559, 2559},
        {3071, 2559, 2559}, {4095, 2559, 2559}, {767, 2815, 2559}, {1791, 2815, 2559},
        {2815, 2815, 2559}, {3839, 2815, 2559}, {511, 3071, 2559}, {1535, 3071, 2559},
        {2559, 3071, 2559}, {3583, 3071, 2559}, {255, 3327, 2559}, {1279, 3327, 2559},
        {2303, 3327, 2559}, {3327, 3327, 2559}, {0, 3583, 2559}, {1023, 3583, 2559},
        {2047, 3583, 2559}, {3071, 3583, 2559}, {4095, 3583, 2559}, {767, 3839, 2559},
        {1791, 3839, 2559}, {2815, 3839, 2559}, {3839, 3839, 2559}, {511, 4095, 2559},
        {1535, 4095, 2559}, {2559, 4095, 2559}, {3583, 4095, 2559}, {255, 0, 2815}, {1279, 0, 2815},
        {2303, 0, 2815}, {3327, 0, 2815}, {0, 255, 2815}, {1023, 255, 2815}, {2047, 255, 2815},
        {3071, 255, 2815}, {4095, 255, 2815}, {767, 511, 2815}, {1791, 511, 2815},
        {2815, 511, 2815}, {3839, 511, 2815}, {511, 767, 2815}, {1535, 767, 2815},
        {2559, 767, 2815}, {3583, 767, 2815}, {255, 1023, 2815}, {1279, 1023, 2815},
        {2303, 1023, 2815}, {3327, 1023, 2815}, {0, 1279, 2815}, {1023, 1279, 2815},
        {2047, 1279, 2815}, {3071, 1279, 2815}, {4095, 1279, 2815}, {767, 1535, 2815},
        {1791, 1535, 2815}, {2815, 1535, 2815}, {3839, 1535, 2815}, {511, 1791, 2815},
        {1535, 1791, 2815}, {2559, 1791, 2815}, {3583, 1791, 2815}, {255, 2047, 2815},
        {1279, 2047, 2815}, {2303, 2047, 2815}, {3327, 2047, 2815}, {0, 2303, 2815},
        {1023, 2303, 2815}, {2047, 2303, 2815}, {3071, 2303, 2815}, {4095, 2303, 2815},
        {767, 2559, 2815}, {1791, 2559, 2815}, {2815, 2559, 2815}, {3839, 2559, 2815},
        {511, 2815, 2815}, {1535, 2815, 2815}, {2559, 2815, 2815}, {3583, 2815, 2815},
        {255, 3071, 2815}, {1279, 3071, 2815}, {2303, 3071, 2815}, {3327, 3071, 2815},
        {0, 3327, 2815}, {1023, 3327, 2815}, {2047, 3327, 2815}, {3071, 3327, 2815},
        {4095, 3327, 2815}, {767, 3583, 2815}, {1791, 3583, 2815}, {2815, 3583, 2815},
        {3839, 3583, 2815}, {511, 3839, 2815}, {1535, 3839, 2815}, {2559, 3839, 2815},
        {3583, 3839, 2815}, {255, 4095, 2815}, {1279, 4095, 2815}, {2303, 4095, 2815},
        {3327, 4095, 2815}, {0, 0, 3071}, {1023, 0, 3071}, {2047, 0, 3071}, {3071, 0, 3071},
        {4095, 0, 3071}, {767, 255, 3071}, {1791, 255, 3071}, {2815, 255, 3071}, {3839, 255, 3071},
        {511, 511, 3071}, {1535, 511, 3071}, {2559, 511, 3071}, {3583, 511, 3071}, {255, 767, 3071},
        {1279, 767, 3071}, {2303, 767, 3071}, {3327, 767, 3071}, {0, 1023, 3071},
        {1023, 1023, 3071}, {2047, 1023, 3071}, {3071, 1023, 3071}, {4095, 1023, 3071},
        {767, 1279, 3071}, {1791, 1279, 3071}, {2815, 1279, 3071}, {3839, 1279, 3071},
        {511, 1535, 3071}, {1535, 1535, 3071}, {2559, 1535, 3071}, {3583, 1535, 3071},
        {255, 1791, 3071}, {1279, 1791, 3071}, {2303, 1791, 3071}, {3327, 1791, 3071},
        {0, 2047, 3071}, {1023, 2047, 3071}, {2047, 2047, 3071}, {3071, 2047, 3071},
        {4095, 2047, 3071}, {767, 2303, 3071}, {1791, 2303, 3071}, {2815, 2303, 3071},
        {3839, 2303, 3071}, {511, 2559, 3071}, {1535, 2559, 3071}, {2559, 2559, 3071},
        {3583, 2559, 3071}, {255, 2815, 3071}, {1279, 2815, 3071}, {2303, 2815, 3071},
        {3327, 2815, 3071}, {0, 3071, 3071}, {1023, 3071, 3071}, {2047, 3071, 3071},
        {3071, 3071, 3071}, {4095, 3071, 3071}, {767, 3327, 3071}, {1791, 3327, 3071},
        {2815, 3327, 3071}, {3839, 3327, 3071}, {511, 3583, 3071}, {1535, 3583, 3071},
        {2559, 3583, 3071}, {3583, 3583, 3071}, {255, 3839, 3071}, {1279, 3839, 3071},
        {2303, 3839, 3071}, {3327, 3839, 3071}, {0, 4095, 3071}, {1023, 4095, 3071},
        {2047, 4095, 3071}, {3071, 4095, 3071}, {4095, 4095, 3071}, {767, 0, 3327}, {1791, 0, 3327},
        {2815, 0, 3327}, {3839, 0, 3327}, {511, 255, 3327}, {1535, 255, 3327}, {2559, 255, 3327},
        {3583, 255, 3327}, {255, 511, 3327}, {1279, 511, 3327}, {2303, 511, 3327},
        {3327, 511, 3327}, {0, 767, 3327}, {1023, 767, 3327}, {2047, 767, 3327}, {3071, 767, 3327},
        {4095, 767, 3327}, {767, 1023, 3327}, {1791, 1023, 3327}, {2815, 1023, 3327},
        {3839, 1023, 3327}, {511, 1279, 3327}, {1535, 1279, 3327}, {2559, 1279, 3327},
        {3583, 1279, 3327}, {255, 1535, 3327}, {1279, 1535, 3327}, {2303, 1535, 3327},
        {3327, 1535, 3327}, {0, 1791, 3327}, {1023, 1791, 3327}, {2047, 1791, 3327},
        {3071, 1791, 3327}, {4095, 1791, 3327}, {767, 2047, 3327}, {1791, 2047, 3327},
        {2815, 2047, 3327}, {3839, 2047, 3327}, {511, 2303, 3327}, {1535, 2303, 3327},
        {2559, 2303, 3327}, {3583, 2303, 3327}, {255, 2559, 3327}, {1279, 2559, 3327},
        {2303, 2559, 3327}, {3327, 2559, 3327}, {0, 2815, 3327}, {1023, 2815, 3327},
        {2047, 2815, 3327}, {3071, 2815, 3327}, {4095, 2815, 3327}, {767, 3071, 3327},
        {1791, 3071, 3327}, {2815, 3071, 3327}, {3839, 3071, 3327}, {511, 3327, 3327},
        {1535, 3327, 3327}, {2559, 3327, 3327}, {3583, 3327, 3327}, {255, 3583, 3327},
        {1279, 3583, 3327}, {2303, 3583, 3327}, {3327, 3583, 3327}, {0, 3839, 3327},
        {1023, 3839, 3327}, {2047, 3839, 3327}, {3071, 3839, 3327}, {4095, 3839, 3327},
        {767, 4095, 3327}, {1791, 4095, 3327}, {2815, 4095, 3327}, {3839, 4095, 3327},
        {511, 0, 3583}, {1535, 0, 3583}, {2559, 0, 3583}, {3583, 0, 3583}, {255, 255, 3583},
        {1279, 255, 3583}, {2303, 255, 3583}, {3327, 255, 3583}, {0, 511, 3583}, {1023, 511, 3583},
        {2047, 511, 3583}, {3071, 511, 3583}, {4095, 511, 3583}, {767, 767, 3583},
        {1791, 767, 3583}, {2815, 767, 3583}, {3839, 767, 3583}, {511, 1023, 3583},
        {1535, 1023, 3583}, {2559, 1023, 3583}, {3583, 1023, 3583}, {255, 1279, 3583},
        {1279, 1279, 3583}, {2303, 1279, 3583}, {3327, 1279, 3583}, {0, 1535, 3583},
        {1023, 1535, 3583}, {2047, 1535, 3583}, {3071, 1535, 3583}, {4095, 1535, 3583},
        {767, 1791, 3583}, {1791, 1791, 3583}, {2815, 1791, 3583}, {3839, 1791, 3583},
        {511, 2047, 3583}, {1535, 2047, 3583}, {2559, 2047, 3583}, {3583, 2047, 3583},
        {255, 2303, 3583}, {1279, 2303, 3583}, {2303, 2303, 3583}, {3327, 2303, 3583},
        {0, 2559, 3583}, {1023, 2559, 3583}, {2047, 2559, 3583}, {3071, 2559, 3583},
        {4095, 2559, 3583}, {767, 2815, 3583}, {1791, 2815, 3583}, {2815, 2815, 3583},
        {3839, 2815, 3583}, {511, 3071, 3583}, {1535, 3071, 3583}, {2559, 3071, 3583},
        {3583, 3071, 3583}, {255, 3327, 3583}, {1279, 3327, 3583}, {2303, 3327, 3583},
        {3327, 3327, 3583}, {0, 3583, 3583}, {1023, 3583, 3583}, {2047, 3583, 3583},
        {3071, 3583, 3583}, {4095, 3583, 3583}, {767, 3839, 3583}, {1791, 3839, 3583},
        {2815, 3839, 3583}, {3839, 3839, 3583}, {511, 4095, 3583}, {1535, 4095, 3583},
        {2559, 4095, 3583}, {3583, 4095, 3583}, {255, 0, 3839}, {1279, 0, 3839}, {2303, 0, 3839},
        {3327, 0, 3839}, {0, 255, 3839}, {1023, 255, 3839}, {2047, 255, 3839}, {3071, 255, 3839},
        {4095, 255, 3839}, {767, 511, 3839}, {1791, 511, 3839}, {2815, 511, 3839},
        {3839, 511, 3839}, {511, 767, 3839}, {1535, 767, 3839}, {2559, 767, 3839},
        {3583, 767, 3839}, {255, 1023, 3839}, {1279, 1023, 3839}, {2303, 1023, 3839},
        {3327, 1023, 3839}, {0, 1279, 3839}, {1023, 1279, 3839}, {2047, 1279, 3839},
        {3071, 1279, 3839}, {4095, 1279, 3839}, {767, 1535, 3839}, {1791, 1535, 3839},
        {2815, 1535, 3839}, {3839, 1535, 3839}, {511, 1791, 3839}, {1535, 1791, 3839},
        {2559, 1791, 3839}, {3583, 1791, 3839}, {255, 2047, 3839}, {1279, 2047, 3839},
        {2303, 2047, 3839}, {3327, 2047, 3839}, {0, 2303, 3839}, {1023, 2303, 3839},
        {2047, 2303, 3839}, {3071, 2303, 3839}, {4095, 2303, 3839}, {767, 2559, 3839},
        {1791, 2559, 3839}, {2815, 2559, 3839}, {3839, 2559, 3839}, {511, 2815, 3839},
        {1535, 2815, 3839}, {2559, 2815, 3839}, {3583, 2815, 3839}, {255, 3071, 3839},
        {1279, 3071, 3839}, {2303, 3071, 3839}, {3327, 3071, 3839}, {0, 3327, 3839},
        {1023, 3327, 3839}, {2047, 3327, 3839}, {3071, 3327, 3839}, {4095, 3327, 3839},
        {767, 3583, 3839}, {1791, 3583, 3839}, {2815, 3583, 3839}, {3839, 3583, 3839},
        {511, 3839, 3839}, {1535, 3839, 3839}, {2559, 3839, 3839}, {3583, 3839, 3839},
        {255, 4095, 3839}, {1279, 4095, 3839}, {2303, 4095, 3839}, {3327, 4095, 3839}, {0, 0, 4095},
        {1023, 0, 4095}, {2047, 0, 4095}, {3071, 0, 4095}, {4095, 0, 4095}, {767, 255, 4095},
        {1791, 255, 4095}, {2815, 255, 4095}, {3839, 255, 4095}, {511, 511, 4095},
        {1535, 511, 4095}, {2559, 511, 4095}, {3583, 511, 4095}, {255, 767, 4095},
        {1279, 767, 4095}, {2303, 767, 4095}, {3327, 767, 4095}, {0, 1023, 4095},
        {1023, 1023, 4095}, {2047, 1023, 4095}, {3071, 1023, 4095}, {4095, 1023, 4095},
        {767, 1279, 4095}, {1791, 1279, 4095}, {2815, 1279, 4095}, {3839, 1279, 4095},
        {511, 1535, 4095}, {1535, 1535, 4095}, {2559, 1535, 4095}, {3583, 1535, 4095},
        {255, 1791, 4095}, {1279, 1791, 4095}, {2303, 1791, 4095}, {3327, 1791, 4095},
        {0, 2047, 4095}, {1023, 2047, 4095}, {2047, 2047, 4095}, {3071, 2047, 4095},
        {4095, 2047, 4095}, {767, 2303, 4095}, {1791, 2303, 4095}, {2815, 2303, 4095},
        {3839, 2303, 4095}, {511, 2559, 4095}, {1535, 2559, 4095}, {2559, 2559, 4095},
        {3583, 2559, 4095}, {255, 2815, 4095}, {1279, 2815, 4095}, {2303, 2815, 4095},
        {3327, 2815, 4095}, {0, 3071, 4095}, {1023, 3071, 4095}, {2047, 3071, 4095},
        {3071, 3071, 4095}, {4095, 3071, 4095}, {767, 3327, 4095}, {1791, 3327, 4095},
        {2815, 3327, 4095}, {3839, 3327, 4095}, {511, 3583, 4095}, {1535, 3583, 4095},
        {2559, 3583, 4095}, {3583, 3583, 4095}, {255, 3839, 4095}, {1279, 3839, 4095},
        {2303, 3839, 4095}, {3327, 3839, 4095}, {0, 4095, 4095}, {1023, 4095, 4095},
        {2047, 4095, 4095}, {3071, 4095, 4095}, {4095, 4095, 4095}},
    // lut 1
    {{255, 0, 0}, {1279, 0, 0}, {2303, 0, 0}, {3327, 0, 0}, {0, 255, 0}, {1023, 255, 0},
        {2047, 255, 0}, {3071, 255, 0}, {4095, 255, 0}, {767, 511, 0}, {1791, 511, 0},
        {2815, 511, 0}, {3839, 511, 0}, {511, 767, 0}, {1535, 767, 0}, {2559, 767, 0},
        {3583, 767, 0}, {255, 1023, 0}, {1279, 1023, 0}, {2303, 1023, 0}, {3327, 1023, 0},
        {0, 1279, 0}, {1023, 1279, 0}, {2047, 1279, 0}, {3071, 1279, 0}, {4095, 1279, 0},
        {767, 1535, 0}, {1791, 1535, 0}, {2815, 1535, 0}, {3839, 1535, 0}, {511, 1791, 0},
        {1535, 1791, 0}, {2559, 1791, 0}, {3583, 1791, 0}, {255, 2047, 0}, {1279, 2047, 0},
        {2303, 2047, 0}, {3327, 2047, 0}, {0, 2303, 0}, {1023, 2303, 0}, {2047, 2303, 0},
        {3071, 2303, 0}, {4095, 2303, 0}, {767, 2559, 0}, {1791, 2559, 0}, {2815, 2559, 0},
        {3839, 2559, 0}, {511, 2815, 0}, {1535, 2815, 0}, {2559, 2815, 0}, {3583, 2815, 0},
        {255, 3071, 0}, {1279, 3071, 0}, {2303, 3071, 0}, {3327, 3071, 0}, {0, 3327, 0},
        {1023, 3327, 0}, {2047, 3327, 0}, {3071, 3327, 0}, {4095, 3327, 0}, {767, 3583, 0},
        {1791, 3583, 0}, {2815, 3583, 0}, {3839, 3583, 0}, {511, 3839, 0}, {1535, 3839, 0},
        {2559, 3839, 0}, {3583, 3839, 0}, {255, 4095, 0}, {1279, 4095, 0}, {2303, 4095, 0},
        {3327, 4095, 0}, {0, 0, 255}, {1023, 0, 255}, {2047, 0, 255}, {3071, 0, 255},
        {4095, 0, 255}, {767, 255, 255}, {1791, 255, 255}, {2815, 255, 255}, {3839, 255, 255},
        {511, 511, 255}, {1535, 511, 255}, {2559, 511, 255}, {3583, 511, 255}, {255, 767, 255},
        {1279, 767, 255}, {2303, 767, 255}, {3327, 767, 255}, {0, 1023, 255}, {1023, 1023, 255},
        {2047, 1023, 255}, {3071, 1023, 255}, {4095, 1023, 255}, {767, 1279, 255},
        {1791, 1279, 255}, {2815, 1279, 255}, {3839, 1279, 255}, {511, 1535, 255},
        {1535, 1535, 255}, {2559, 1535, 255}, {3583, 1535, 255}, {255, 1791, 255},
        {1279, 1791, 255}, {2303, 1791, 255}, {3327, 1791, 255}, {0, 2047, 255}, {1023, 2047, 255},
        {2047, 2047, 255}, {3071, 2047, 255}, {4095, 2047, 255}, {767, 2303, 255},
        {1791, 2303, 255}, {2815, 2303, 255}, {3839, 2303, 255}, {511, 2559, 255},
        {1535, 2559, 255}, {2559, 2559, 255}, {3583, 2559, 255}, {255, 2815, 255},
        {1279, 2815, 255}, {2303, 2815, 255}, {3327, 2815, 255}, {0, 3071, 255}, {1023, 3071, 255},
        {2047, 3071, 255}, {3071, 3071, 255}, {4095, 3071, 255}, {767, 3327, 255},
        {1791, 3327, 255}, {2815, 3327, 255}, {3839, 3327, 255}, {511, 3583, 255},
        {1535, 3583, 255}, {2559, 3583, 255}, {3583, 3583, 255}, {255, 3839, 255},
        {1279, 3839, 255}, {2303, 3839, 255}, {3327, 3839, 255}, {0, 4095, 255}, {1023, 4095, 255},
        {2047, 4095, 255}, {3071, 4095, 255}, {4095, 4095, 255}, {767, 0, 511}, {1791, 0, 511},
        {2815, 0, 511}, {3839, 0, 511}, {511, 255, 511}, {1535, 255, 511}, {2559, 255, 511},
        {3583, 255, 511}, {255, 511, 511}, {1279, 511, 511}, {2303, 511, 511}, {3327, 511, 511},
        {0, 767, 511}, {1023, 767, 511}, {2047, 767, 511}, {3071, 767, 511}, {4095, 767, 511},
        {767, 1023, 511}, {1791, 1023, 511}, {2815, 1023, 511}, {3839, 1023, 511}, {511, 1279, 511},
        {1535, 1279, 511}, {2559, 1279, 511}, {3583, 1279, 511}, {255, 1535, 511},
        {1279, 1535, 511}, {2303, 1535, 511}, {3327, 1535, 511}, {0, 1791, 511}, {1023, 1791, 511},
        {2047, 1791, 511}, {3071, 1791, 511}, {4095, 1791, 511}, {767, 2047, 511},
        {1791, 2047, 511}, {2815, 2047, 511}, {3839, 2047, 511}, {511, 2303, 511},
        {1535, 2303, 511}, {2559, 2303, 511}, {3583, 2303, 511}, {255, 2559, 511},
        {1279, 2559, 511}, {2303, 2559, 511}, {3327, 2559, 511}, {0, 2815, 511}, {1023, 2815, 511},
        {2047, 2815, 511}, {3071, 2815, 511}, {4095, 2815, 511}, {767, 3071, 511},
        {1791, 3071, 511}, {2815, 3071, 511}, {3839, 3071, 511}, {511, 3327, 511},
        {1535, 3327, 511}, {2559, 3327, 511}, {3583, 3327, 511}, {255, 3583, 511},
        {1279, 3583, 511}, {2303, 3583, 511}, {3327, 3583, 511}, {0, 3839, 511}, {1023, 3839, 511},
        {2047, 3839, 511}, {3071, 3839, 511}, {4095, 3839, 511}, {767, 4095, 511},
        {1791, 4095, 511}, {2815, 4095, 511}, {3839, 4095, 511}, {511, 0, 767}, {1535, 0, 767},
        {2559, 0, 767}, {3583, 0, 767}, {255, 255, 767}, {1279, 255, 767}, {2303, 255, 767},
        {3327, 255, 767}, {0, 511, 767}, {1023, 511, 767}, {2047, 511, 767}, {3071, 511, 767},
        {4095, 511, 767}, {767, 767, 767}, {1791, 767, 767}, {2815, 767, 767}, {3839, 767, 767},
        {511, 1023, 767}, {1535, 1023, 767}, {2559, 1023, 767}, {3583, 1023, 767}, {255, 1279, 767},
        {1279, 1279, 767}, {2303, 1279, 767}, {3327, 1279, 767}, {0, 1535, 767}, {1023, 1535, 767},
        {2047, 1535, 767}, {3071, 1535, 767}, {4095, 1535, 767}, {767, 1791, 767},
        {1791, 1791, 767}, {2815, 1791, 767}, {3839, 1791, 767}, {511, 2047, 767},
        {1535, 2047, 767}, {2559, 2047, 767}, {3583, 2047, 767}, {255, 2303, 767},
        {1279, 2303, 767}, {2303, 2303, 767}, {3327, 2303, 767}, {0, 2559, 767}, {1023, 2559, 767},
        {2047, 2559, 767}, {3071, 2559, 767}, {4095, 2559, 767}, {767, 2815, 767},
        {1791, 2815, 767}, {2815, 2815, 767}, {3839, 2815, 767}, {511, 3071, 767},
        {1535, 3071, 767}, {2559, 3071, 767}, {3583, 3071, 767}, {255, 3327, 767},
        {1279, 3327, 767}, {2303, 3327, 767}, {3327, 3327, 767}, {0, 3583, 767}, {1023, 3583, 767},
        {2047, 3583, 767}, {3071, 3583, 767}, {4095, 3583, 767}, {767, 3839, 767},
        {1791, 3839, 767}, {2815, 3839, 767}, {3839, 3839, 767}, {511, 4095, 767},
        {1535, 4095, 767}, {2559, 4095, 767}, {3583, 4095, 767}, {255, 0, 1023}, {1279, 0, 1023},
        {2303, 0, 1023}, {3327, 0, 1023}, {0, 255, 1023}, {1023, 255, 1023}, {2047, 255, 1023},
        {3071, 255, 1023}, {4095, 255, 1023}, {767, 511, 1023}, {1791, 511, 1023},
        {2815, 511, 1023}, {3839, 511, 1023}, {511, 767, 1023}, {1535, 767, 1023},
        {2559, 767, 1023}, {3583, 767, 1023}, {255, 1023, 1023}, {1279, 1023, 1023},
        {2303, 1023, 1023}, {3327, 1023, 1023}, {0, 1279, 1023}, {1023, 1279, 1023},
        {2047, 1279, 1023}, {3071, 1279, 1023}, {4095, 1279, 1023}, {767, 1535, 1023},
        {1791, 1535, 1023}, {2815, 1535, 1023}, {3839, 1535, 1023}, {511, 1791, 1023},
        {1535, 1791, 1023}, {2559, 1791, 1023}, {3583, 1791, 1023}, {255, 2047, 1023},
        {1279, 2047, 1023}, {2303, 2047, 1023}, {3327, 2047, 1023}, {0, 2303, 1023},
        {1023, 2303, 1023}, {2047, 2303, 1023}, {3071, 2303, 1023}, {4095, 2303, 1023},
        {767, 2559, 1023}, {1791, 2559, 1023}, {2815, 2559, 1023}, {3839, 2559, 1023},
        {511, 2815, 1023}, {1535, 2815, 1023}, {2559, 2815, 1023}, {3583, 2815, 1023},
        {255, 3071, 1023}, {1279, 3071, 1023}, {2303, 3071, 1023}, {3327, 3071, 1023},
        {0, 3327, 1023}, {1023, 3327, 1023}, {2047, 3327, 1023}, {3071, 3327, 1023},
        {4095, 3327, 1023}, {767, 3583, 1023}, {1791, 3583, 1023}, {2815, 3583, 1023},
        {3839, 3583, 1023}, {511, 3839, 1023}, {1535, 3839, 1023}, {2559, 3839, 1023},
        {3583, 3839, 1023}, {255, 4095, 1023}, {1279, 4095, 1023}, {2303, 4095, 1023},
        {3327, 4095, 1023}, {0, 0, 1279}, {1023, 0, 1279}, {2047, 0, 1279}, {3071, 0, 1279},
        {4095, 0, 1279}, {767, 255, 1279}, {1791, 255, 1279}, {2815, 255, 1279}, {3839, 255, 1279},
        {511, 511, 1279}, {1535, 511, 1279}, {2559, 511, 1279}, {3583, 511, 1279}, {255, 767, 1279},
        {1279, 767, 1279}, {2303, 767, 1279}, {3327, 767, 1279}, {0, 1023, 1279},
        {1023, 1023, 1279}, {2047, 1023, 1279}, {3071, 1023, 1279}, {4095, 1023, 1279},
        {767, 1279, 1279}, {1791, 1279, 1279}, {2815, 1279, 1279}, {3839, 1279, 1279},
        {511, 1535, 1279}, {1535, 1535, 1279}, {2559, 1535, 1279}, {3583, 1535, 1279},
        {255, 1791, 1279}, {1279, 1791, 1279}, {2303, 1791, 1279}, {3327, 1791, 1279},
        {0, 2047, 1279}, {1023, 2047, 1279}, {2047, 2047, 1279}, {3071, 2047, 1279},
        {4095, 2047, 1279}, {767, 2303, 1279}, {1791, 2303, 1279}, {2815, 2303, 1279},
        {3839, 2303, 1279}, {511, 2559, 1279}, {1535, 2559, 1279}, {2559, 2559, 1279},
        {3583, 2559, 1279}, {255, 2815, 1279}, {1279, 2815, 1279}, {2303, 2815, 1279},
        {3327, 2815, 1279}, {0, 3071, 1279}, {1023, 3071, 1279}, {2047, 3071, 1279},
        {3071, 3071, 1279}, {4095, 3071, 1279}, {767, 3327, 1279}, {1791, 3327, 1279},
        {2815, 3327, 1279}, {3839, 3327, 1279}, {511, 3583, 1279}, {1535, 3583, 1279},
        {2559, 3583, 1279}, {3583, 3583, 1279}, {255, 3839, 1279}, {1279, 3839, 1279},
        {2303, 3839, 1279}, {3327, 3839, 1279}, {0, 4095, 1279}, {1023, 4095, 1279},
        {2047, 4095, 1279}, {3071, 4095, 1279}, {4095, 4095, 1279}, {767, 0, 1535}, {1791, 0, 1535},
        {2815, 0, 1535}, {3839, 0, 1535}, {511, 255, 1535}, {1535, 255, 1535}, {2559, 255, 1535},
        {3583, 255, 1535}, {255, 511, 1535}, {1279, 511, 1535}, {2303, 511, 1535},
        {3327, 511, 1535}, {0, 767, 1535}, {1023, 767, 1535}, {2047, 767, 1535}, {3071, 767, 1535},
        {4095, 767, 1535}, {767, 1023, 1535}, {1791, 1023, 1535}, {2815, 1023, 1535},
        {3839, 1023, 1535}, {511, 1279, 1535}, {1535, 1279, 1535}, {2559, 1279, 1535},
        {3583, 1279, 1535}, {255, 1535, 1535}, {1279, 1535, 1535}, {2303, 1535, 1535},
        {3327, 1535, 1535}, {0, 1791, 1535}, {1023, 1791, 1535}, {2047, 1791, 1535},
        {3071, 1791, 1535}, {4095, 1791, 1535}, {767, 2047, 1535}, {1791, 2047, 1535},
        {2815, 2047, 1535}, {3839, 2047, 1535}, {511, 2303, 1535}, {1535, 2303, 1535},
        {2559, 2303, 1535}, {3583, 2303, 1535}, {255, 2559, 1535}, {1279, 2559, 1535},
        {2303, 2559, 1535}, {3327, 2559, 1535}, {0, 2815, 1535}, {1023, 2815, 1535},
        {2047, 2815, 1535}, {3071, 2815, 1535}, {4095, 2815, 1535}, {767, 3071, 1535},
        {1791, 3071, 1535}, {2815, 3071, 1535}, {3839, 3071, 1535}, {511, 3327, 1535},
        {1535, 3327, 1535}, {2559, 3327, 1535}, {3583, 3327, 1535}, {255, 3583, 1535},
        {1279, 3583, 1535}, {2303, 3583, 1535}, {3327, 3583, 1535}, {0, 3839, 1535},
        {1023, 3839, 1535}, {2047, 3839, 1535}, {3071, 3839, 1535}, {4095, 3839, 1535},
        {767, 4095, 1535}, {1791, 4095, 1535}, {2815, 4095, 1535}, {3839, 4095, 1535},
        {511, 0, 1791}, {1535, 0, 1791}, {2559, 0, 1791}, {3583, 0, 1791}, {255, 255, 1791},
        {1279, 255, 1791}, {2303, 255, 1791}, {3327, 255, 1791}, {0, 511, 1791}, {1023, 511, 1791},
        {2047, 511, 1791}, {3071, 511, 1791}, {4095, 511, 1791}, {767, 767, 1791},
        {1791, 767, 1791}, {2815, 767, 1791}, {3839, 767, 1791}, {511, 1023, 1791},
        {1535, 1023, 1791}, {2559, 1023, 1791}, {3583, 1023, 1791}, {255, 1279, 1791},
        {1279, 1279, 1791}, {2303, 1279, 1791}, {3327, 1279, 1791}, {0, 1535, 1791},
        {1023, 1535, 1791}, {2047, 1535, 1791}, {3071, 1535, 1791}, {4095, 1535, 1791},
        {767, 1791, 1791}, {1791, 1791, 1791}, {2815, 1791, 1791}, {3839, 1791, 1791},
        {511, 2047, 1791}, {1535, 2047, 1791}, {2559, 2047, 1791}, {3583, 2047, 1791},
        {255, 2303, 1791}, {1279, 2303, 1791}, {2303, 2303, 1791}, {3327, 2303, 1791},
        {0, 2559, 1791}, {1023, 2559, 1791}, {2047, 2559, 1791}, {3071, 2559, 1791},
        {4095, 2559, 1791}, {767, 2815, 1791}, {1791, 2815, 1791}, {2815, 2815, 1791},
        {3839, 2815, 1791}, {511, 3071, 1791}, {1535, 3071, 1791}, {2559, 3071, 1791},
        {3583, 3071, 1791}, {255, 3327, 1791}, {1279, 3327, 1791}, {2303, 3327, 1791},
        {3327, 3327, 1791}, {0, 3583, 1791}, {1023, 3583, 1791}, {2047, 3583, 1791},
        {3071, 3583, 1791}, {4095, 3583, 1791}, {767, 3839, 1791}, {1791, 3839, 1791},
        {2815, 3839, 1791}, {3839, 3839, 1791}, {511, 4095, 1791}, {1535, 4095, 1791},
        {2559, 4095, 1791}, {3583, 4095, 1791}, {255, 0, 2047}, {1279, 0, 2047}, {2303, 0, 2047},
        {3327, 0, 2047}, {0, 255, 2047}, {1023, 255, 2047}, {2047, 255, 2047}, {3071, 255, 2047},
        {4095, 255, 2047}, {767, 511, 2047}, {1791, 511, 2047}, {2815, 511, 2047},
        {3839, 511, 2047}, {511, 767, 2047}, {1535, 767, 2047}, {2559, 767, 2047},
        {3583, 767, 2047}, {255, 1023, 2047}, {1279, 1023, 2047}, {2303, 1023, 2047},
        {3327, 1023, 2047}, {0, 1279, 2047}, {1023, 1279, 2047}, {2047, 1279, 2047},
        {3071, 1279, 2047}, {4095, 1279, 2047}, {767, 1535, 2047}, {1791, 1535, 2047},
        {2815, 1535, 2047}, {3839, 1535, 2047}, {511, 1791, 2047}, {1535, 1791, 2047},
        {2559, 1791, 2047}, {3583, 1791, 2047}, {255, 2047, 2047}, {1279, 2047, 2047},
        {2303, 2047, 2047}, {3327, 2047, 2047}, {0, 2303, 2047}, {1023, 2303, 2047},
        {2047, 2303, 2047}, {3071, 2303, 2047}, {4095, 2303, 2047}, {767, 2559, 2047},
        {1791, 2559, 2047}, {2815, 2559, 2047}, {3839, 2559, 2047}, {511, 2815, 2047},
        {1535, 2815, 2047}, {2559, 2815, 2047}, {3583, 2815, 2047}, {255, 3071, 2047},
        {1279, 3071, 2047}, {2303, 3071, 2047}, {3327, 3071, 2047}, {0, 3327, 2047},
        {1023, 3327, 2047}, {2047, 3327, 2047}, {3071, 3327, 2047}, {4095, 3327, 2047},
        {767, 3583, 2047}, {1791, 3583, 2047}, {2815, 3583, 2047}, {3839, 3583, 2047},
        {511, 3839, 2047}, {1535, 3839, 2047}, {2559, 3839, 2047}, {3583, 3839, 2047},
        {255, 4095, 2047}, {1279, 4095, 2047}, {2303, 4095, 2047}, {3327, 4095, 2047}, {0, 0, 2303},
        {1023, 0, 2303}, {2047, 0, 2303}, {3071, 0, 2303}, {4095, 0, 2303}, {767, 255, 2303},
        {1791, 255, 2303}, {2815, 255, 2303}, {3839, 255, 2303}, {511, 511, 2303},
        {1535, 511, 2303}, {2559, 511, 2303}, {3583, 511, 2303}, {255, 767, 2303},
        {1279, 767, 2303}, {2303, 767, 2303}, {3327, 767, 2303}, {0, 1023, 2303},
        {1023, 1023, 2303}, {2047, 1023, 2303}, {3071, 1023, 2303}, {4095, 1023, 2303},
        {767, 1279, 2303}, {1791, 1279, 2303}, {2815, 1279, 2303}, {3839, 1279, 2303},
        {511, 1535, 2303}, {1535, 1535, 2303}, {2559, 1535, 2303}, {3583, 1535, 2303},
        {255, 1791, 2303}, {1279, 1791, 2303}, {2303, 1791, 2303}, {3327, 1791, 2303},
        {0, 2047, 2303}, {1023, 2047, 2303}, {2047, 2047, 2303}, {3071, 2047, 2303},
        {4095, 2047, 2303}, {767, 2303, 2303}, {1791, 2303, 2303}, {2815, 2303, 2303},
        {3839, 2303, 2303}, {511, 2559, 2303}, {1535, 2559, 2303}, {2559, 2559, 2303},
        {3583, 2559, 2303}, {255, 2815, 2303}, {1279, 2815, 2303}, {2303, 2815, 2303},
        {3327, 2815, 2303}, {0, 3071, 2303}, {1023, 3071, 2303}, {2047, 3071, 2303},
        {3071, 3071, 2303}, {4095, 3071, 2303}, {767, 3327, 2303}, {1791, 3327, 2303},
        {2815, 3327, 2303}, {3839, 3327, 2303}, {511, 3583, 2303}, {1535, 3583, 2303},
        {2559, 3583, 2303}, {3583, 3583, 2303}, {255, 3839, 2303}, {1279, 3839, 2303},
        {2303, 3839, 2303}, {3327, 3839, 2303}, {0, 4095, 2303}, {1023, 4095, 2303},
        {2047, 4095, 2303}, {3071, 4095, 2303}, {4095, 4095, 2303}, {767, 0, 2559}, {1791, 0, 2559},
        {2815, 0, 2559}, {3839, 0, 2559}, {511, 255, 2559}, {1535, 255, 2559}, {2559, 255, 2559},
        {3583, 255, 2559}, {255, 511, 2559}, {1279, 511, 2559}, {2303, 511, 2559},
        {3327, 511, 2559}, {0, 767, 2559}, {1023, 767, 2559}, {2047, 767, 2559}, {3071, 767, 2559},
        {4095, 767, 2559}, {767, 1023, 2559}, {1791, 1023, 2559}, {2815, 1023, 2559},
        {3839, 1023, 2559}, {511, 1279, 2559}, {1535, 1279, 2559}, {2559, 1279, 2559},
        {3583, 1279, 2559}, {255, 1535, 2559}, {1279, 1535, 2559}, {2303, 1535, 2559},
        {3327, 1535, 2559}, {0, 1791, 2559}, {1023, 1791, 2559}, {2047, 1791, 2559},
        {3071, 1791, 2559}, {4095, 1791, 2559}, {767, 2047, 2559}, {1791, 2047, 2559},
        {2815, 2047, 2559}, {3839, 2047, 2559}, {511, 2303, 2559}, {1535, 2303, 2559},
        {2559, 2303, 2559}, {3583, 2303, 2559}, {255, 2559, 2559}, {1279, 2559, 2559},
        {2303, 2559, 2559}, {3327, 2559, 2559}, {0, 2815, 2559}, {1023, 2815, 2559},
        {2047, 2815, 2559}, {3071, 2815, 2559}, {4095, 2815, 2559}, {767, 3071, 2559},
        {1791, 3071, 2559}, {2815, 3071, 2559}, {3839, 3071, 2559}, {511, 3327, 2559},
        {1535, 3327, 2559}, {2559, 3327, 2559}, {3583, 3327, 2559}, {255, 3583, 2559},
        {1279, 3583, 2559}, {2303, 3583, 2559}, {3327, 3583, 2559}, {0, 3839, 2559},
        {1023, 3839, 2559}, {2047, 3839, 2559}, {3071, 3839, 2559}, {4095, 3839, 2559},
        {767, 4095, 2559}, {1791, 4095, 2559}, {2815, 4095, 2559}, {3839, 4095, 2559},
        {511, 0, 2815}, {1535, 0, 2815}, {2559, 0, 2815}, {3583, 0, 2815}, {255, 255, 2815},
        {1279, 255, 2815}, {2303, 255, 2815}, {3327, 255, 2815}, {0, 511, 2815}, {1023, 511, 2815},
        {2047, 511, 2815}, {3071, 511, 2815}, {4095, 511, 2815}, {767, 767, 2815},
        {1791, 767, 2815}, {2815, 767, 2815}, {3839, 767, 2815}, {511, 1023, 2815},
        {1535, 1023, 2815}, {2559, 1023, 2815}, {3583, 1023, 2815}, {255, 1279, 2815},
        {1279, 1279, 2815}, {2303, 1279, 2815}, {3327, 1279, 2815}, {0, 1535, 2815},
        {1023, 1535, 2815}, {2047, 1535, 2815}, {3071, 1535, 2815}, {4095, 1535, 2815},
        {767, 1791, 2815}, {1791, 1791, 2815}, {2815, 1791, 2815}, {3839, 1791, 2815},
        {511, 2047, 2815}, {1535, 2047, 2815}, {2559, 2047, 2815}, {3583, 2047, 2815},
        {255, 2303, 2815}, {1279, 2303, 2815}, {2303, 2303, 2815}, {3327, 2303, 2815},
        {0, 2559, 2815}, {1023, 2559, 2815}, {2047, 2559, 2815}, {3071, 2559, 2815},
        {4095, 2559, 2815}, {767, 2815, 2815}, {1791, 2815, 2815}, {2815, 2815, 2815},
        {3839, 2815, 2815}, {511, 3071, 2815}, {1535, 3071, 2815}, {2559, 3071, 2815},
        {3583, 3071, 2815}, {255, 3327, 2815}, {1279, 3327, 2815}, {2303, 3327, 2815},
        {3327, 3327, 2815}, {0, 3583, 2815}, {1023, 3583, 2815}, {2047, 3583, 2815},
        {3071, 3583, 2815}, {4095, 3583, 2815}, {767, 3839, 2815}, {1791, 3839, 2815},
        {2815, 3839, 2815}, {3839, 3839, 2815}, {511, 4095, 2815}, {1535, 4095, 2815},
        {2559, 4095, 2815}, {3583, 4095, 2815}, {255, 0, 3071}, {1279, 0, 3071}, {2303, 0, 3071},
        {3327, 0, 3071}, {0, 255, 3071}, {1023, 255, 3071}, {2047, 255, 3071}, {3071, 255, 3071},
        {4095, 255, 3071}, {767, 511, 3071}, {1791, 511, 3071}, {2815, 511, 3071},
        {3839, 511, 3071}, {511, 767, 3071}, {1535, 767, 3071}, {2559, 767, 3071},
        {3583, 767, 3071}, {255, 1023, 3071}, {1279, 1023, 3071}, {2303, 1023, 3071},
        {3327, 1023, 3071}, {0, 1279, 3071}, {1023, 1279, 3071}, {2047, 1279, 3071},
        {3071, 1279, 3071}, {4095, 1279, 3071}, {767, 1535, 3071}, {1791, 1535, 3071},
        {2815, 1535, 3071}, {3839, 1535, 3071}, {511, 1791, 3071}, {1535, 1791, 3071},
        {2559, 1791, 3071}, {3583, 1791, 3071}, {255, 2047, 3071}, {1279, 2047, 3071},
        {2303, 2047, 3071}, {3327, 2047, 3071}, {0, 2303, 3071}, {1023, 2303, 3071},
        {2047, 2303, 3071}, {3071, 2303, 3071}, {4095, 2303, 3071}, {767, 2559, 3071},
        {1791, 2559, 3071}, {2815, 2559, 3071}, {3839, 2559, 3071}, {511, 2815, 3071},
        {1535, 2815, 3071}, {2559, 2815, 3071}, {3583, 2815, 3071}, {255, 3071, 3071},
        {1279, 3071, 3071}, {2303, 3071, 3071}, {3327, 3071, 3071}, {0, 3327, 3071},
        {1023, 3327, 3071}, {2047, 3327, 3071}, {3071, 3327, 3071}, {4095, 3327, 3071},
        {767, 3583, 3071}, {1791, 3583, 3071}, {2815, 3583, 3071}, {3839, 3583, 3071},
        {511, 3839, 3071}, {1535, 3839, 3071}, {2559, 3839, 3071}, {3583, 3839, 3071},
        {255, 4095, 3071}, {1279, 4095, 3071}, {2303, 4095, 3071}, {3327, 4095, 3071}, {0, 0, 3327},
        {1023, 0, 3327}, {2047, 0, 3327}, {3071, 0, 3327}, {4095, 0, 3327}, {767, 255, 3327},
        {1791, 255, 3327}, {2815, 255, 3327}, {3839, 255, 3327}, {511, 511, 3327},
        {1535, 511, 3327}, {2559, 511, 3327}, {3583, 511, 3327}, {255, 767, 3327},
        {1279, 767, 3327}, {2303, 767, 3327}, {3327, 767, 3327}, {0, 1023, 3327},
        {1023, 1023, 3327}, {2047, 1023, 3327}, {3071, 1023, 3327}, {4095, 1023, 3327},
        {767, 1279, 3327}, {1791, 1279, 3327}, {2815, 1279, 3327}, {3839, 1279, 3327},
        {511, 1535, 3327}, {1535, 1535, 3327}, {2559, 1535, 3327}, {3583, 1535, 3327},
        {255, 1791, 3327}, {1279, 1791, 3327}, {2303, 1791, 3327}, {3327, 1791, 3327},
        {0, 2047, 3327}, {1023, 2047, 3327}, {2047, 2047, 3327}, {3071, 2047, 3327},
        {4095, 2047, 3327}, {767, 2303, 3327}, {1791, 2303, 3327}, {2815, 2303, 3327},
        {3839, 2303, 3327}, {511, 2559, 3327}, {1535, 2559, 3327}, {2559, 2559, 3327},
        {3583, 2559, 3327}, {255, 2815, 3327}, {1279, 2815, 3327}, {2303, 2815, 3327},
        {3327, 2815, 3327}, {0, 3071, 3327}, {1023, 3071, 3327}, {2047, 3071, 3327},
        {3071, 3071, 3327}, {4095, 3071, 3327}, {767, 3327, 3327}, {1791, 3327, 3327},
        {2815, 3327, 3327}, {3839, 3327, 3327}, {511, 3583, 3327}, {1535, 3583, 3327},
        {2559, 3583, 3327}, {3583, 3583, 3327}, {255, 3839, 3327}, {1279, 3839, 3327},
        {2303, 3839, 3327}, {3327, 3839, 3327}, {0, 4095, 3327}, {1023, 4095, 3327},
        {2047, 4095, 3327}, {3071, 4095, 3327}, {4095, 4095, 3327}, {767, 0, 3583}, {1791, 0, 3583},
        {2815, 0, 3583}, {3839, 0, 3583}, {511, 255, 3583}, {1535, 255, 3583}, {2559, 255, 3583},
        {3583, 255, 3583}, {255, 511, 3583}, {1279, 511, 3583}, {2303, 511, 3583},
        {3327, 511, 3583}, {0, 767, 3583}, {1023, 767, 3583}, {2047, 767, 3583}, {3071, 767, 3583},
        {4095, 767, 3583}, {767, 1023, 3583}, {1791, 1023, 3583}, {2815, 1023, 3583},
        {3839, 1023, 3583}, {511, 1279, 3583}, {1535, 1279, 3583}, {2559, 1279, 3583},
        {3583, 1279, 3583}, {255, 1535, 3583}, {1279, 1535, 3583}, {2303, 1535, 3583},
        {3327, 1535, 3583}, {0, 1791, 3583}, {1023, 1791, 3583}, {2047, 1791, 3583},
        {3071, 1791, 3583}, {4095, 1791, 3583}, {767, 2047, 3583}, {1791, 2047, 3583},
        {2815, 2047, 3583}, {3839, 2047, 3583}, {511, 2303, 3583}, {1535, 2303, 3583},
        {2559, 2303, 3583}, {3583, 2303, 3583}, {255, 2559, 3583}, {1279, 2559, 3583},
        {2303, 2559, 3583}, {3327, 2559, 3583}, {0, 2815, 3583}, {1023, 2815, 3583},
        {2047, 2815, 3583}, {3071, 2815, 3583}, {4095, 2815, 3583}, {767, 3071, 3583},
        {1791, 3071, 3583}, {2815, 3071, 3583}, {3839, 3071, 3583}, {511, 3327, 3583},
        {1535, 3327, 3583}, {2559, 3327, 3583}, {3583, 3327, 3583}, {255, 3583, 3583},
        {1279, 3583, 3583}, {2303, 3583, 3583}, {3327, 3583, 3583}, {0, 3839, 3583},
        {1023, 3839, 3583}, {2047, 3839, 3583}, {3071, 3839, 3583}, {4095, 3839, 3583},
        {767, 4095, 3583}, {1791, 4095, 3583}, {2815, 4095, 3583}, {3839, 4095, 3583},
        {511, 0, 3839}, {1535, 0, 3839}, {2559, 0, 3839}, {3583, 0, 3839}, {255, 255, 3839},
        {1279, 255, 3839}, {2303, 255, 3839}, {3327, 255, 3839}, {0, 511, 3839}, {1023, 511, 3839},
        {2047, 511, 3839}, {3071, 511, 3839}, {4095, 511, 3839}, {767, 767, 3839},
        {1791, 767, 3839}, {2815, 767, 3839}, {3839, 767, 3839}, {511, 1023, 3839},
        {1535, 1023, 3839}, {2559, 1023, 3839}, {3583, 1023, 3839}, {255, 1279, 3839},
        {1279, 1279, 3839}, {2303, 1279, 3839}, {3327, 1279, 3839}, {0, 1535, 3839},
        {1023, 1535, 3839}, {2047, 1535, 3839}, {3071, 1535, 3839}, {4095, 1535, 3839},
        {767, 1791, 3839}, {1791, 1791, 3839}, {2815, 1791, 3839}, {3839, 1791, 3839},
        {511, 2047, 3839}, {1535, 2047, 3839}, {2559, 2047, 3839}, {3583, 2047, 3839},
        {255, 2303, 3839}, {1279, 2303, 3839}, {2303, 2303, 3839}, {3327, 2303, 3839},
        {0, 2559, 3839}, {1023, 2559, 3839}, {2047, 2559, 3839}, {3071, 2559, 3839},
        {4095, 2559, 3839}, {767, 2815, 3839}, {1791, 2815, 3839}, {2815, 2815, 3839},
        {3839, 2815, 3839}, {511, 3071, 3839}, {1535, 3071, 3839}, {2559, 3071, 3839},
        {3583, 3071, 3839}, {255, 3327, 3839}, {1279, 3327, 3839}, {2303, 3327, 3839},
        {3327, 3327, 3839}, {0, 3583, 3839}, {1023, 3583, 3839}, {2047, 3583, 3839},
        {3071, 3583, 3839}, {4095, 3583, 3839}, {767, 3839, 3839}, {1791, 3839, 3839},
        {2815, 3839, 3839}, {3839, 3839, 3839}, {511, 4095, 3839}, {1535, 4095, 3839},
        {2559, 4095, 3839}, {3583, 4095, 3839}, {255, 0, 4095}, {1279, 0, 4095}, {2303, 0, 4095},
        {3327, 0, 4095}, {0, 255, 4095}, {1023, 255, 4095}, {2047, 255, 4095}, {3071, 255, 4095},
        {4095, 255, 4095}, {767, 511, 4095}, {1791, 511, 4095}, {2815, 511, 4095},
        {3839, 511, 4095}, {511, 767, 4095}, {1535, 767, 4095}, {2559, 767, 4095},
        {3583, 767, 4095}, {255, 1023, 4095}, {1279, 1023, 4095}, {2303, 1023, 4095},
        {3327, 1023, 4095}, {0, 1279, 4095}, {1023, 1279, 4095}, {2047, 1279, 4095},
        {3071, 1279, 4095}, {4095, 1279, 4095}, {767, 1535, 4095}, {1791, 1535, 4095},
        {2815, 1535, 4095}, {3839, 1535, 4095}, {511, 1791, 4095}, {1535, 1791, 4095},
        {2559, 1791, 4095}, {3583, 1791, 4095}, {255, 2047, 4095}, {1279, 2047, 4095},
        {2303, 2047, 4095}, {3327, 2047, 4095}, {0, 2303, 4095}, {1023, 2303, 4095},
        {2047, 2303, 4095}, {3071, 2303, 4095}, {4095, 2303, 4095}, {767, 2559, 4095},
        {1791, 2559, 4095}, {2815, 2559, 4095}, {3839, 2559, 4095}, {511, 2815, 4095},
        {1535, 2815, 4095}, {2559, 2815, 4095}, {3583, 2815, 4095}, {255, 3071, 4095},
        {1279, 3071, 4095}, {2303, 3071, 4095}, {3327, 3071, 4095}, {0, 3327, 4095},
        {1023, 3327, 4095}, {2047, 3327, 4095}, {3071, 3327, 4095}, {4095, 3327, 4095},
        {767, 3583, 4095}, {1791, 3583, 4095}, {2815, 3583, 4095}, {3839, 3583, 4095},
        {511, 3839, 4095}, {1535, 3839, 4095}, {2559, 3839, 4095}, {3583, 3839, 4095},
        {255, 4095, 4095}, {1279, 4095, 4095}, {2303, 4095, 4095}, {3327, 4095, 4095}},
    // lut 2
    {{511, 0, 0}, {1535, 0, 0}, {2559, 0, 0}, {3583, 0, 0}, {255, 255, 0}, {1279, 255, 0},
        {2303, 255, 0}, {3327, 255, 0}, {0, 511, 0}, {1023, 511, 0}, {2047, 511, 0}, {3071, 511, 0},
        {4095, 511, 0}, {767, 767, 0}, {1791, 767, 0}, {2815, 767, 0}, {3839, 767, 0},
        {511, 1023, 0}, {1535, 1023, 0}, {2559, 1023, 0}, {3583, 1023, 0}, {255, 1279, 0},
        {1279, 1279, 0}, {2303, 1279, 0}, {3327, 1279, 0}, {0, 1535, 0}, {1023, 1535, 0},
        {2047, 1535, 0}, {3071, 1535, 0}, {4095, 1535, 0}, {767, 1791, 0}, {1791, 1791, 0},
        {2815, 1791, 0}, {3839, 1791, 0}, {511, 2047, 0}, {1535, 2047, 0}, {2559, 2047, 0},
        {3583, 2047, 0}, {255, 2303, 0}, {1279, 2303, 0}, {2303, 2303, 0}, {3327, 2303, 0},
        {0, 2559, 0}, {1023, 2559, 0}, {2047, 2559, 0}, {3071, 2559, 0}, {4095, 2559, 0},
        {767, 2815, 0}, {1791, 2815, 0}, {2815, 2815, 0}, {3839, 2815, 0}, {511, 3071, 0},
        {1535, 3071, 0}, {2559, 3071, 0}, {3583, 3071, 0}, {255, 3327, 0}, {1279, 3327, 0},
        {2303, 3327, 0}, {3327, 3327, 0}, {0, 3583, 0}, {1023, 3583, 0}, {2047, 3583, 0},
        {3071, 3583, 0}, {4095, 3583, 0}, {767, 3839, 0}, {1791, 3839, 0}, {2815, 3839, 0},
        {3839, 3839, 0}, {511, 4095, 0}, {1535, 4095, 0}, {2559, 4095, 0}, {3583, 4095, 0},
        {255, 0, 255}, {1279, 0, 255}, {2303, 0, 255}, {3327, 0, 255}, {0, 255, 255},
        {1023, 255, 255}, {2047, 255, 255}, {3071, 255, 255}, {4095, 255, 255}, {767, 511, 255},
        {1791, 511, 255}, {2815, 511, 255}, {3839, 511, 255}, {511, 767, 255}, {1535, 767, 255},
        {2559, 767, 255}, {3583, 767, 255}, {255, 1023, 255}, {1279, 1023, 255}, {2303, 1023, 255},
        {3327, 1023, 255}, {0, 1279, 255}, {1023, 1279, 255}, {2047, 1279, 255}, {3071, 1279, 255},
        {4095, 1279, 255}, {767, 1535, 255}, {1791, 1535, 255}, {2815, 1535, 255},
        {3839, 1535, 255}, {511, 1791, 255}, {1535, 1791, 255}, {2559, 1791, 255},
        {3583, 1791, 255}, {255, 2047, 255}, {1279, 2047, 255}, {2303, 2047, 255},
        {3327, 2047, 255}, {0, 2303, 255}, {1023, 2303, 255}, {2047, 2303, 255}, {3071, 2303, 255},
        {4095, 2303, 255}, {767, 2559, 255}, {1791, 2559, 255}, {2815, 2559, 255},
        {3839, 2559, 255}, {511, 2815, 255}, {1535, 2815, 255}, {2559, 2815, 255},
        {3583, 2815, 255}, {255, 3071, 255}, {1279, 3071, 255}, {2303, 3071, 255},
        {3327, 3071, 255}, {0, 3327, 255}, {1023, 3327, 255}, {2047, 3327, 255}, {3071, 3327, 255},
        {4095, 3327, 255}, {767, 3583, 255}, {1791, 3583, 255}, {2815, 3583, 255},
        {3839, 3583, 255}, {511, 3839, 255}, {1535, 3839, 255}, {2559, 3839, 255},
        {3583, 3839, 255}, {255, 4095, 255}, {1279, 4095, 255}, {2303, 4095, 255},
        {3327, 4095, 255}, {0, 0, 511}, {1023, 0, 511}, {2047, 0, 511}, {3071, 0, 511},
        {4095, 0, 511}, {767, 255, 511}, {1791, 255, 511}, {2815, 255, 511}, {3839, 255, 511},
        {511, 511, 511}, {1535, 511, 511}, {2559, 511, 511}, {3583, 511, 511}, {255, 767, 511},
        {1279, 767, 511}, {2303, 767, 511}, {3327, 767, 511}, {0, 1023, 511}, {1023, 1023, 511},
        {2047, 1023, 511}, {3071, 1023, 511}, {4095, 1023, 511}, {767, 1279, 511},
        {1791, 1279, 511}, {2815, 1279, 511}, {3839, 1279, 511}, {511, 1535, 511},
        {1535, 1535, 511}, {2559, 1535, 511}, {3583, 1535, 511}, {255, 1791, 511},
        {1279, 1791, 511}, {2303, 1791, 511}, {3327, 1791, 511}, {0, 2047, 511}, {1023, 2047, 511},
        {2047, 2047, 511}, {3071, 2047, 511}, {4095, 2047, 511}, {767, 2303, 511},
        {1791, 2303, 511}, {2815, 2303, 511}, {3839, 2303, 511}, {511, 2559, 511},
        {1535, 2559, 511}, {2559, 2559, 511}, {3583, 2559, 511}, {255, 2815, 511},
        {1279, 2815, 511}, {2303, 2815, 511}, {3327, 2815, 511}, {0, 3071, 511}, {1023, 3071, 511},
        {2047, 3071, 511}, {3071, 3071, 511}, {4095, 3071, 511}, {767, 3327, 511},
        {1791, 3327, 511}, {2815, 3327, 511}, {3839, 3327, 511}, {511, 3583, 511},
        {1535, 3583, 511}, {2559, 3583, 511}, {3583, 3583, 511}, {255, 3839, 511},
        {1279, 3839, 511}, {2303, 3839, 511}, {3327, 3839, 511}, {0, 4095, 511}, {1023, 4095, 511},
        {2047, 4095, 511}, {3071, 4095, 511}, {4095, 4095, 511}, {767, 0, 767}, {1791, 0, 767},
        {2815, 0, 767}, {3839, 0, 767}, {511, 255, 767}, {1535, 255, 767}, {2559, 255, 767},
        {3583, 255, 767}, {255, 511, 767}, {1279, 511, 767}, {2303, 511, 767}, {3327, 511, 767},
        {0, 767, 767}, {1023, 767, 767}, {2047, 767, 767}, {3071, 767, 767}, {4095, 767, 767},
        {767, 1023, 767}, {1791, 1023, 767}, {2815, 1023, 767}, {3839, 1023, 767}, {511, 1279, 767},
        {1535, 1279, 767}, {2559, 1279, 767}, {3583, 1279, 767}, {255, 1535, 767},
        {1279, 1535, 767}, {2303, 1535, 767}, {3327, 1535, 767}, {0, 1791, 767}, {1023, 1791, 767},
        {2047, 1791, 767}, {3071, 1791, 767}, {4095, 1791, 767}, {767, 2047, 767},
        {1791, 2047, 767}, {2815, 2047, 767}, {3839, 2047, 767}, {511, 2303, 767},
        {1535, 2303, 767}, {2559, 2303, 767}, {3583, 2303, 767}, {255, 2559, 767},
        {1279, 2559, 767}, {2303, 2559, 767}, {3327, 2559, 767}, {0, 2815, 767}, {1023, 2815, 767},
        {2047, 2815, 767}, {3071, 2815, 767}, {4095, 2815, 767}, {767, 3071, 767},
        {1791, 3071, 767}, {2815, 3071, 767}, {3839, 3071, 767}, {511, 3327, 767},
        {1535, 3327, 767}, {2559, 3327, 767}, {3583, 3327, 767}, {255, 3583, 767},
        {1279, 3583, 767}, {2303, 3583, 767}, {3327, 3583, 767}, {0, 3839, 767}, {1023, 3839, 767},
        {2047, 3839, 767}, {3071, 3839, 767}, {4095, 3839, 767}, {767, 4095, 767},
        {1791, 4095, 767}, {2815, 4095, 767}, {3839, 4095, 767}, {511, 0, 1023}, {1535, 0, 1023},
        {2559, 0, 1023}, {3583, 0, 1023}, {255, 255, 1023}, {1279, 255, 1023}, {2303, 255, 1023},
        {3327, 255, 1023}, {0, 511, 1023}, {1023, 511, 1023}, {2047, 511, 1023}, {3071, 511, 1023},
        {4095, 511, 1023}, {767, 767, 1023}, {1791, 767, 1023}, {2815, 767, 1023},
        {3839, 767, 1023}, {511, 1023, 1023}, {1535, 1023, 1023}, {2559, 1023, 1023},
        {3583, 1023, 1023}, {255, 1279, 1023}, {1279, 1279, 1023}, {2303, 1279, 1023},
        {3327, 1279, 1023}, {0, 1535, 1023}, {1023, 1535, 1023}, {2047, 1535, 1023},
        {3071, 1535, 1023}, {4095, 1535, 1023}, {767, 1791, 1023}, {1791, 1791, 1023},
        {2815, 1791, 1023}, {3839, 1791, 1023}, {511, 2047, 1023}, {1535, 2047, 1023},
        {2559, 2047, 1023}, {3583, 2047, 1023}, {255, 2303, 1023}, {1279, 2303, 1023},
        {2303, 2303, 1023}, {3327, 2303, 1023}, {0, 2559, 1023}, {1023, 2559, 1023},
        {2047, 2559, 1023}, {3071, 2559, 1023}, {4095, 2559, 1023}, {767, 2815, 1023},
        {1791, 2815, 1023}, {2815, 2815, 1023}, {3839, 2815, 1023}, {511, 3071, 1023},
        {1535, 3071, 1023}, {2559, 3071, 1023}, {3583, 3071, 1023}, {255, 3327, 1023},
        {1279, 3327, 1023}, {2303, 3327, 1023}, {3327, 3327, 1023}, {0, 3583, 1023},
        {1023, 3583, 1023}, {2047, 3583, 1023}, {3071, 3583, 1023}, {4095, 3583, 1023},
        {767, 3839, 1023}, {1791, 3839, 1023}, {2815, 3839, 1023}, {3839, 3839, 1023},
        {511, 4095, 1023}, {1535, 4095, 1023}, {2559, 4095, 1023}, {3583, 4095, 1023},
        {255, 0, 1279}, {1279, 0, 1279}, {2303, 0, 1279}, {3327, 0, 1279}, {0, 255, 1279},
        {1023, 255, 1279}, {2047, 255, 1279}, {3071, 255, 1279}, {4095, 255, 1279},
        {767, 511, 1279}, {1791, 511, 1279}, {2815, 511, 1279}, {3839, 511, 1279}, {511, 767, 1279},
        {1535, 767, 1279}, {2559, 767, 1279}, {3583, 767, 1279}, {255, 1023, 1279},
        {1279, 1023, 1279}, {2303, 1023, 1279}, {3327, 1023, 1279}, {0, 1279, 1279},
        {1023, 1279, 1279}, {2047, 1279, 1279}, {3071, 1279, 1279}, {4095, 1279, 1279},
        {767, 1535, 1279}, {1791, 1535, 1279}, {2815, 1535, 1279}, {3839, 1535, 1279},
        {511, 1791, 1279}, {1535, 1791, 1279}, {2559, 1791, 1279}, {3583, 1791, 1279},
        {255, 2047, 1279}, {1279, 2047, 1279}, {2303, 2047, 1279}, {3327, 2047, 1279},
        {0, 2303, 1279}, {1023, 2303, 1279}, {2047, 2303, 1279}, {3071, 2303, 1279},
        {4095, 2303, 1279}, {767, 2559, 1279}, {1791, 2559, 1279}, {2815, 2559, 1279},
        {3839, 2559, 1279}, {511, 2815, 1279}, {1535, 2815, 1279}, {2559, 2815, 1279},
        {3583, 2815, 1279}, {255, 3071, 1279}, {1279, 3071, 1279}, {2303, 3071, 1279},
        {3327, 3071, 1279}, {0, 3327, 1279}, {1023, 3327, 1279}, {2047, 3327, 1279},
        {3071, 3327, 1279}, {4095, 3327, 1279}, {767, 3583, 1279}, {1791, 3583, 1279},
        {2815, 3583, 1279}, {3839, 3583, 1279}, {511, 3839, 1279}, {1535, 3839, 1279},
        {2559, 3839, 1279}, {3583, 3839, 1279}, {255, 4095, 1279}, {1279, 4095, 1279},
        {2303, 4095, 1279}, {3327, 4095, 1279}, {0, 0, 1535}, {1023, 0, 1535}, {2047, 0, 1535},
        {3071, 0, 1535}, {4095, 0, 1535}, {767, 255, 1535}, {1791, 255, 1535}, {2815, 255, 1535},
        {3839, 255, 1535}, {511, 511, 1535}, {1535, 511, 1535}, {2559, 511, 1535},
        {3583, 511, 1535}, {255, 767, 1535}, {1279, 767, 1535}, {2303, 767, 1535},
        {3327, 767, 1535}, {0, 1023, 1535}, {1023, 1023, 1535}, {2047, 1023, 1535},
        {3071, 1023, 1535}, {4095, 1023, 1535}, {767, 1279, 1535}, {1791, 1279, 1535},
        {2815, 1279, 1535}, {3839, 1279, 1535}, {511, 1535, 1535}, {1535, 1535, 1535},
        {2559, 1535, 1535}, {3583, 1535, 1535}, {255, 1791, 1535}, {1279, 1791, 1535},
        {2303, 1791, 1535}, {3327, 1791, 1535}, {0, 2047, 1535}, {1023, 2047, 1535},
        {2047, 2047, 1535}, {3071, 2047, 1535}, {4095, 2047, 1535}, {767, 2303, 1535},
        {1791, 2303, 1535}, {2815, 2303, 1535}, {3839, 2303, 1535}, {511, 2559, 1535},
        {1535, 2559, 1535}, {2559, 2559, 1535}, {3583, 2559, 1535}, {255, 2815, 1535},
        {1279, 2815, 1535}, {2303, 2815, 1535}, {3327, 2815, 1535}, {0, 3071, 1535},
        {1023, 3071, 1535}, {2047, 3071, 1535}, {3071, 3071, 1535}, {4095, 3071, 1535},
        {767, 3327, 1535}, {1791, 3327, 1535}, {2815, 3327, 1535}, {3839, 3327, 1535},
        {511, 3583, 1535}, {1535, 3583, 1535}, {2559, 3583, 1535}, {3583, 3583, 1535},
        {255, 3839, 1535}, {1279, 3839, 1535}, {2303, 3839, 1535}, {3327, 3839, 1535},
        {0, 4095, 1535}, {1023, 4095, 1535}, {2047, 4095, 1535}, {3071, 4095, 1535},
        {4095, 4095, 1535}, {767, 0, 1791}, {1791, 0, 1791}, {2815, 0, 1791}, {3839, 0, 1791},
        {511, 255, 1791}, {1535, 255, 1791}, {2559, 255, 1791}, {3583, 255, 1791}, {255, 511, 1791},
        {1279, 511, 1791}, {2303, 511, 1791}, {3327, 511, 1791}, {0, 767, 1791}, {1023, 767, 1791},
        {2047, 767, 1791}, {3071, 767, 1791}, {4095, 767, 1791}, {767, 1023, 1791},
        {1791, 1023, 1791}, {2815, 1023, 1791}, {3839, 1023, 1791}, {511, 1279, 1791},
        {1535, 1279, 1791}, {2559, 1279, 1791}, {3583, 1279, 1791}, {255, 1535, 1791},
        {1279, 1535, 1791}, {2303, 1535, 1791}, {3327, 1535, 1791}, {0, 1791, 1791},
        {1023, 1791, 1791}, {2047, 1791, 1791}, {3071, 1791, 1791}, {4095, 1791, 1791},
        {767, 2047, 1791}, {1791, 2047, 1791}, {2815, 2047, 1791}, {3839, 2047, 1791},
        {511, 2303, 1791}, {1535, 2303, 1791}, {2559, 2303, 1791}, {3583, 2303, 1791},
        {255, 2559, 1791}, {1279, 2559, 1791}, {2303, 2559, 1791}, {3327, 2559, 1791},
        {0, 2815, 1791}, {1023, 2815, 1791}, {2047, 2815, 1791}, {3071, 2815, 1791},
        {4095, 2815, 1791}, {767, 3071, 1791}, {1791, 3071, 1791}, {2815, 3071, 1791},
        {3839, 3071, 1791}, {511, 3327, 1791}, {1535, 3327, 1791}, {2559, 3327, 1791},
        {3583, 3327, 1791}, {255, 3583, 1791}, {1279, 3583, 1791}, {2303, 3583, 1791},
        {3327, 3583, 1791}, {0, 3839, 1791}, {1023, 3839, 1791}, {2047, 3839, 1791},
        {3071, 3839, 1791}, {4095, 3839, 1791}, {767, 4095, 1791}, {1791, 4095, 1791},
        {2815, 4095, 1791}, {3839, 4095, 1791}, {511, 0, 2047}, {1535, 0, 2047}, {2559, 0, 2047},
        {3583, 0, 2047}, {255, 255, 2047}, {1279, 255, 2047}, {2303, 255, 2047}, {3327, 255, 2047},
        {0, 511, 2047}, {1023, 511, 2047}, {2047, 511, 2047}, {3071, 511, 2047}, {4095, 511, 2047},
        {767, 767, 2047}, {1791, 767, 2047}, {2815, 767, 2047}, {3839, 767, 2047},
        {511, 1023, 2047}, {1535, 1023, 2047}, {2559, 1023, 2047}, {3583, 1023, 2047},
        {255, 1279, 2047}, {1279, 1279, 2047}, {2303, 1279, 2047}, {3327, 1279, 2047},
        {0, 1535, 2047}, {1023, 1535, 2047}, {2047, 1535, 2047}, {3071, 1535, 2047},
        {4095, 1535, 2047}, {767, 1791, 2047}, {1791, 1791, 2047}, {2815, 1791, 2047},
        {3839, 1791, 2047}, {511, 2047, 2047}, {1535, 2047, 2047}, {2559, 2047, 2047},
        {3583, 2047, 2047}, {255, 2303, 2047}, {1279, 2303, 2047}, {2303, 2303, 2047},
        {3327, 2303, 2047}, {0, 2559, 2047}, {1023, 2559, 2047}, {2047, 2559, 2047},
        {3071, 2559, 2047}, {4095, 2559, 2047}, {767, 2815, 2047}, {1791, 2815, 2047},
        {2815, 2815, 2047}, {3839, 2815, 2047}, {511, 3071, 2047}, {1535, 3071, 2047},
        {2559, 3071, 2047}, {3583, 3071, 2047}, {255, 3327, 2047}, {1279, 3327, 2047},
        {2303, 3327, 2047}, {3327, 3327, 2047}, {0, 3583, 2047}, {1023, 3583, 2047},
        {2047, 3583, 2047}, {3071, 3583, 2047}, {4095, 3583, 2047}, {767, 3839, 2047},
        {1791, 3839, 2047}, {2815, 3839, 2047}, {3839, 3839, 2047}, {511, 4095, 2047},
        {1535, 4095, 2047}, {2559, 4095, 2047}, {3583, 4095, 2047}, {255, 0, 2303}, {1279, 0, 2303},
        {2303, 0, 2303}, {3327, 0, 2303}, {0, 255, 2303}, {1023, 255, 2303}, {2047, 255, 2303},
        {3071, 255, 2303}, {4095, 255, 2303}, {767, 511, 2303}, {1791, 511, 2303},
        {2815, 511, 2303}, {3839, 511, 2303}, {511, 767, 2303}, {1535, 767, 2303},
        {2559, 767, 2303}, {3583, 767, 2303}, {255, 1023, 2303}, {1279, 1023, 2303},
        {2303, 1023, 2303}, {3327, 1023, 2303}, {0, 1279, 2303}, {1023, 1279, 2303},
        {2047, 1279, 2303}, {3071, 1279, 2303}, {4095, 1279, 2303}, {767, 1535, 2303},
        {1791, 1535, 2303}, {2815, 1535, 2303}, {3839, 1535, 2303}, {511, 1791, 2303},
        {1535, 1791, 2303}, {2559, 1791, 2303}, {3583, 1791, 2303}, {255, 2047, 2303},
        {1279, 2047, 2303}, {2303, 2047, 2303}, {3327, 2047, 2303}, {0, 2303, 2303},
        {1023, 2303, 2303}, {2047, 2303, 2303}, {3071, 2303, 2303}, {4095, 2303, 2303},
        {767, 2559, 2303}, {1791, 2559, 2303}, {2815, 2559, 2303}, {3839, 2559, 2303},
        {511, 2815, 2303}, {1535, 2815, 2303}, {2559, 2815, 2303}, {3583, 2815, 2303},
        {255, 3071, 2303}, {1279, 3071, 2303}, {2303, 3071, 2303}, {3327, 3071, 2303},
        {0, 3327, 2303}, {1023, 3327, 2303}, {2047, 3327, 2303}, {3071, 3327, 2303},
        {4095, 3327, 2303}, {767, 3583, 2303}, {1791, 3583, 2303}, {2815, 3583, 2303},
        {3839, 3583, 2303}, {511, 3839, 2303}, {1535, 3839, 2303}, {2559, 3839, 2303},
        {3583, 3839, 2303}, {255, 4095, 2303}, {1279, 4095, 2303}, {2303, 4095, 2303},
        {3327, 4095, 2303}, {0, 0, 2559}, {1023, 0, 2559}, {2047, 0, 2559}, {3071, 0, 2559},
        {4095, 0, 2559}, {767, 255, 2559}, {1791, 255, 2559}, {2815, 255, 2559}, {3839, 255, 2559},
        {511, 511, 2559}, {1535, 511, 2559}, {2559, 511, 2559}, {3583, 511, 2559}, {255, 767, 2559},
        {1279, 767, 2559}, {2303, 767, 2559}, {3327, 767, 2559}, {0, 1023, 2559},
        {1023, 1023, 2559}, {2047, 1023, 2559}, {3071, 1023, 2559}, {4095, 1023, 2559},
        {767, 1279, 2559}, {1791, 1279, 2559}, {2815, 1279, 2559}, {3839, 1279, 2559},
        {511, 1535, 2559}, {1535, 1535, 2559}, {2559, 1535, 2559}, {3583, 1535, 2559},
        {255, 1791, 2559}, {1279, 1791, 2559}, {2303, 1791, 2559}, {3327, 1791, 2559},
        {0, 2047, 2559}, {1023, 2047, 2559}, {2047, 2047, 2559}, {3071, 2047, 2559},
        {4095, 2047, 2559}, {767, 2303, 2559}, {1791, 2303, 2559}, {2815, 2303, 2559},
        {3839, 2303, 2559}, {511, 2559, 2559}, {1535, 2559, 2559}, {2559, 2559, 2559},
        {3583, 2559, 2559}, {255, 2815, 2559}, {1279, 2815, 2559}, {2303, 2815, 2559},
        {3327, 2815, 2559}, {0, 3071, 2559}, {1023, 3071, 2559}, {2047, 3071, 2559},
        {3071, 3071, 2559}, {4095, 3071, 2559}, {767, 3327, 2559}, {1791, 3327, 2559},
        {2815, 3327, 2559}, {3839, 3327, 2559}, {511, 3583, 2559}, {1535, 3583, 2559},
        {2559, 3583, 2559}, {3583, 3583, 2559}, {255, 3839, 2559}, {1279, 3839, 2559},
        {2303, 3839, 2559}, {3327, 3839, 2559}, {0, 4095, 2559}, {1023, 4095, 2559},
        {2047, 4095, 2559}, {3071, 4095, 2559}, {4095, 4095, 2559}, {767, 0, 2815}, {1791, 0, 2815},
        {2815, 0, 2815}, {3839, 0, 2815}, {511, 255, 2815}, {1535, 255, 2815}, {2559, 255, 2815},
        {3583, 255, 2815}, {255, 511, 2815}, {1279, 511, 2815}, {2303, 511, 2815},
        {3327, 511, 2815}, {0, 767, 2815}, {1023, 767, 2815}, {2047, 767, 2815}, {3071, 767, 2815},
        {4095, 767, 2815}, {767, 1023, 2815}, {1791, 1023, 2815}, {2815, 1023, 2815},
        {3839, 1023, 2815}, {511, 1279, 2815}, {1535, 1279, 2815}, {2559, 1279, 2815},
        {3583, 1279, 2815}, {255, 1535, 2815}, {1279, 1535, 2815}, {2303, 1535, 2815},
        {3327, 1535, 2815}, {0, 1791, 2815}, {1023, 1791, 2815}, {2047, 1791, 2815},
        {3071, 1791, 2815}, {4095, 1791, 2815}, {767, 2047, 2815}, {1791, 2047, 2815},
        {2815, 2047, 2815}, {3839, 2047, 2815}, {511, 2303, 2815}, {1535, 2303, 2815},
        {2559, 2303, 2815}, {3583, 2303, 2815}, {255, 2559, 2815}, {1279, 2559, 2815},
        {2303, 2559, 2815}, {3327, 2559, 2815}, {0, 2815, 2815}, {1023, 2815, 2815},
        {2047, 2815, 2815}, {3071, 2815, 2815}, {4095, 2815, 2815}, {767, 3071, 2815},
        {1791, 3071, 2815}, {2815, 3071, 2815}, {3839, 3071, 2815}, {511, 3327, 2815},
        {1535, 3327, 2815}, {2559, 3327, 2815}, {3583, 3327, 2815}, {255, 3583, 2815},
        {1279, 3583, 2815}, {2303, 3583, 2815}, {3327, 3583, 2815}, {0, 3839, 2815},
        {1023, 3839, 2815}, {2047, 3839, 2815}, {3071, 3839, 2815}, {4095, 3839, 2815},
        {767, 4095, 2815}, {1791, 4095, 2815}, {2815, 4095, 2815}, {3839, 4095, 2815},
        {511, 0, 3071}, {1535, 0, 3071}, {2559, 0, 3071}, {3583, 0, 3071}, {255, 255, 3071},
        {1279, 255, 3071}, {2303, 255, 3071}, {3327, 255, 3071}, {0, 511, 3071}, {1023, 511, 3071},
        {2047, 511, 3071}, {3071, 511, 3071}, {4095, 511, 3071}, {767, 767, 3071},
        {1791, 767, 3071}, {2815, 767, 3071}, {3839, 767, 3071}, {511, 1023, 3071},
        {1535, 1023, 3071}, {2559, 1023, 3071}, {3583, 1023, 3071}, {255, 1279, 3071},
        {1279, 1279, 3071}, {2303, 1279, 3071}, {3327, 1279, 3071}, {0, 1535, 3071},
        {1023, 1535, 3071}, {2047, 1535, 3071}, {3071, 1535, 3071}, {4095, 1535, 3071},
        {767, 1791, 3071}, {1791, 1791, 3071}, {2815, 1791, 3071}, {3839, 1791, 3071},
        {511, 2047, 3071}, {1535, 2047, 3071}, {2559, 2047, 3071}, {3583, 2047, 3071},
        {255, 2303, 3071}, {1279, 2303, 3071}, {2303, 2303, 3071}, {3327, 2303, 3071},
        {0, 2559, 3071}, {1023, 2559, 3071}, {2047, 2559, 3071}, {3071, 2559, 3071},
        {4095, 2559, 3071}, {767, 2815, 3071}, {1791, 2815, 3071}, {2815, 2815, 3071},
        {3839, 2815, 3071}, {511, 3071, 3071}, {1535, 3071, 3071}, {2559, 3071, 3071},
        {3583, 3071, 3071}, {255, 3327, 3071}, {1279, 3327, 3071}, {2303, 3327, 3071},
        {3327, 3327, 3071}, {0, 3583, 3071}, {1023, 3583, 3071}, {2047, 3583, 3071},
        {3071, 3583, 3071}, {4095, 3583, 3071}, {767, 3839, 3071}, {1791, 3839, 3071},
        {2815, 3839, 3071}, {3839, 3839, 3071}, {511, 4095, 3071}, {1535, 4095, 3071},
        {2559, 4095, 3071}, {3583, 4095, 3071}, {255, 0, 3327}, {1279, 0, 3327}, {2303, 0, 3327},
        {3327, 0, 3327}, {0, 255, 3327}, {1023, 255, 3327}, {2047, 255, 3327}, {3071, 255, 3327},
        {4095, 255, 3327}, {767, 511, 3327}, {1791, 511, 3327}, {2815, 511, 3327},
        {3839, 511, 3327}, {511, 767, 3327}, {1535, 767, 3327}, {2559, 767, 3327},
        {3583, 767, 3327}, {255, 1023, 3327}, {1279, 1023, 3327}, {2303, 1023, 3327},
        {3327, 1023, 3327}, {0, 1279, 3327}, {1023, 1279, 3327}, {2047, 1279, 3327},
        {3071, 1279, 3327}, {4095, 1279, 3327}, {767, 1535, 3327}, {1791, 1535, 3327},
        {2815, 1535, 3327}, {3839, 1535, 3327}, {511, 1791, 3327}, {1535, 1791, 3327},
        {2559, 1791, 3327}, {3583, 1791, 3327}, {255, 2047, 3327}, {1279, 2047, 3327},
        {2303, 2047, 3327}, {3327, 2047, 3327}, {0, 2303, 3327}, {1023, 2303, 3327},
        {2047, 2303, 3327}, {3071, 2303, 3327}, {4095, 2303, 3327}, {767, 2559, 3327},
        {1791, 2559, 3327}, {2815, 2559, 3327}, {3839, 2559, 3327}, {511, 2815, 3327},
        {1535, 2815, 3327}, {2559, 2815, 3327}, {3583, 2815, 3327}, {255, 3071, 3327},
        {1279, 3071, 3327}, {2303, 3071, 3327}, {3327, 3071, 3327}, {0, 3327, 3327},
        {1023, 3327, 3327}, {2047, 3327, 3327}, {3071, 3327, 3327}, {4095, 3327, 3327},
        {767, 3583, 3327}, {1791, 3583, 3327}, {2815, 3583, 3327}, {3839, 3583, 3327},
        {511, 3839, 3327}, {1535, 3839, 3327}, {2559, 3839, 3327}, {3583, 3839, 3327},
        {255, 4095, 3327}, {1279, 4095, 3327}, {2303, 4095, 3327}, {3327, 4095, 3327}, {0, 0, 3583},
        {1023, 0, 3583}, {2047, 0, 3583}, {3071, 0, 3583}, {4095, 0, 3583}, {767, 255, 3583},
        {1791, 255, 3583}, {2815, 255, 3583}, {3839, 255, 3583}, {511, 511, 3583},
        {1535, 511, 3583}, {2559, 511, 3583}, {3583, 511, 3583}, {255, 767, 3583},
        {1279, 767, 3583}, {2303, 767, 3583}, {3327, 767, 3583}, {0, 1023, 3583},
        {1023, 1023, 3583}, {2047, 1023, 3583}, {3071, 1023, 3583}, {4095, 1023, 3583},
        {767, 1279, 3583}, {1791, 1279, 3583}, {2815, 1279, 3583}, {3839, 1279, 3583},
        {511, 1535, 3583}, {1535, 1535, 3583}, {2559, 1535, 3583}, {3583, 1535, 3583},
        {255, 1791, 3583}, {1279, 1791, 3583}, {2303, 1791, 3583}, {3327, 1791, 3583},
        {0, 2047, 3583}, {1023, 2047, 3583}, {2047, 2047, 3583}, {3071, 2047, 3583},
        {4095, 2047, 3583}, {767, 2303, 3583}, {1791, 2303, 3583}, {2815, 2303, 3583},
        {3839, 2303, 3583}, {511, 2559, 3583}, {1535, 2559, 3583}, {2559, 2559, 3583},
        {3583, 2559, 3583}, {255, 2815, 3583}, {1279, 2815, 3583}, {2303, 2815, 3583},
        {3327, 2815, 3583}, {0, 3071, 3583}, {1023, 3071, 3583}, {2047, 3071, 3583},
        {3071, 3071, 3583}, {4095, 3071, 3583}, {767, 3327, 3583}, {1791, 3327, 3583},
        {2815, 3327, 3583}, {3839, 3327, 3583}, {511, 3583, 3583}, {1535, 3583, 3583},
        {2559, 3583, 3583}, {3583, 3583, 3583}, {255, 3839, 3583}, {1279, 3839, 3583},
        {2303, 3839, 3583}, {3327, 3839, 3583}, {0, 4095, 3583}, {1023, 4095, 3583},
        {2047, 4095, 3583}, {3071, 4095, 3583}, {4095, 4095, 3583}, {767, 0, 3839}, {1791, 0, 3839},
        {2815, 0, 3839}, {3839, 0, 3839}, {511, 255, 3839}, {1535, 255, 3839}, {2559, 255, 3839},
        {3583, 255, 3839}, {255, 511, 3839}, {1279, 511, 3839}, {2303, 511, 3839},
        {3327, 511, 3839}, {0, 767, 3839}, {1023, 767, 3839}, {2047, 767, 3839}, {3071, 767, 3839},
        {4095, 767, 3839}, {767, 1023, 3839}, {1791, 1023, 3839}, {2815, 1023, 3839},
        {3839, 1023, 3839}, {511, 1279, 3839}, {1535, 1279, 3839}, {2559, 1279, 3839},
        {3583, 1279, 3839}, {255, 1535, 3839}, {1279, 1535, 3839}, {2303, 1535, 3839},
        {3327, 1535, 3839}, {0, 1791, 3839}, {1023, 1791, 3839}, {2047, 1791, 3839},
        {3071, 1791, 3839}, {4095, 1791, 3839}, {767, 2047, 3839}, {1791, 2047, 3839},
        {2815, 2047, 3839}, {3839, 2047, 3839}, {511, 2303, 3839}, {1535, 2303, 3839},
        {2559, 2303, 3839}, {3583, 2303, 3839}, {255, 2559, 3839}, {1279, 2559, 3839},
        {2303, 2559, 3839}, {3327, 2559, 3839}, {0, 2815, 3839}, {1023, 2815, 3839},
        {2047, 2815, 3839}, {3071, 2815, 3839}, {4095, 2815, 3839}, {767, 3071, 3839},
        {1791, 3071, 3839}, {2815, 3071, 3839}, {3839, 3071, 3839}, {511, 3327, 3839},
        {1535, 3327, 3839}, {2559, 3327, 3839}, {3583, 3327, 3839}, {255, 3583, 3839},
        {1279, 3583, 3839}, {2303, 3583, 3839}, {3327, 3583, 3839}, {0, 3839, 3839},
        {1023, 3839, 3839}, {2047, 3839, 3839}, {3071, 3839, 3839}, {4095, 3839, 3839},
        {767, 4095, 3839}, {1791, 4095, 3839}, {2815, 4095, 3839}, {3839, 4095, 3839},
        {511, 0, 4095}, {1535, 0, 4095}, {2559, 0, 4095}, {3583, 0, 4095}, {255, 255, 4095},
        {1279, 255, 4095}, {2303, 255, 4095}, {3327, 255, 4095}, {0, 511, 4095}, {1023, 511, 4095},
        {2047, 511, 4095}, {3071, 511, 4095}, {4095, 511, 4095}, {767, 767, 4095},
        {1791, 767, 4095}, {2815, 767, 4095}, {3839, 767, 4095}, {511, 1023, 4095},
        {1535, 1023, 4095}, {2559, 1023, 4095}, {3583, 1023, 4095}, {255, 1279, 4095},
        {1279, 1279, 4095}, {2303, 1279, 4095}, {3327, 1279, 4095}, {0, 1535, 4095},
        {1023, 1535, 4095}, {2047, 1535, 4095}, {3071, 1535, 4095}, {4095, 1535, 4095},
        {767, 1791, 4095}, {1791, 1791, 4095}, {2815, 1791, 4095}, {3839, 1791, 4095},
        {511, 2047, 4095}, {1535, 2047, 4095}, {2559, 2047, 4095}, {3583, 2047, 4095},
        {255, 2303, 4095}, {1279, 2303, 4095}, {2303, 2303, 4095}, {3327, 2303, 4095},
        {0, 2559, 4095}, {1023, 2559, 4095}, {2047, 2559, 4095}, {3071, 2559, 4095},
        {4095, 2559, 4095}, {767, 2815, 4095}, {1791, 2815, 4095}, {2815, 2815, 4095},
        {3839, 2815, 4095}, {511, 3071, 4095}, {1535, 3071, 4095}, {2559, 3071, 4095},
        {3583, 3071, 4095}, {255, 3327, 4095}, {1279, 3327, 4095}, {2303, 3327, 4095},
        {3327, 3327, 4095}, {0, 3583, 4095}, {1023, 3583, 4095}, {2047, 3583, 4095},
        {3071, 3583, 4095}, {4095, 3583, 4095}, {767, 3839, 4095}, {1791, 3839, 4095},
        {2815, 3839, 4095}, {3839, 3839, 4095}, {511, 4095, 4095}, {1535, 4095, 4095},
        {2559, 4095, 4095}, {3583, 4095, 4095}},
    // lut 3
    {{767, 0, 0}, {1791, 0, 0}, {2815, 0, 0}, {3839, 0, 0}, {511, 255, 0}, {1535, 255, 0},
        {2559, 255, 0}, {3583, 255, 0}, {255, 511, 0}, {1279, 511, 0}, {2303, 511, 0},
        {3327, 511, 0}, {0, 767, 0}, {1023, 767, 0}, {2047, 767, 0}, {3071, 767, 0}, {4095, 767, 0},
        {767, 1023, 0}, {1791, 1023, 0}, {2815, 1023, 0}, {3839, 1023, 0}, {511, 1279, 0},
        {1535, 1279, 0}, {2559, 1279, 0}, {3583, 1279, 0}, {255, 1535, 0}, {1279, 1535, 0},
        {2303, 1535, 0}, {3327, 1535, 0}, {0, 1791, 0}, {1023, 1791, 0}, {2047, 1791, 0},
        {3071, 1791, 0}, {4095, 1791, 0}, {767, 2047, 0}, {1791, 2047, 0}, {2815, 2047, 0},
        {3839, 2047, 0}, {511, 2303, 0}, {1535, 2303, 0}, {2559, 2303, 0}, {3583, 2303, 0},
        {255, 2559, 0}, {1279, 2559, 0}, {2303, 2559, 0}, {3327, 2559, 0}, {0, 2815, 0},
        {1023, 2815, 0}, {2047, 2815, 0}, {3071, 2815, 0}, {4095, 2815, 0}, {767, 3071, 0},
        {1791, 3071, 0}, {2815, 3071, 0}, {3839, 3071, 0}, {511, 3327, 0}, {1535, 3327, 0},
        {2559, 3327, 0}, {3583, 3327, 0}, {255, 3583, 0}, {1279, 3583, 0}, {2303, 3583, 0},
        {3327, 3583, 0}, {0, 3839, 0}, {1023, 3839, 0}, {2047, 3839, 0}, {3071, 3839, 0},
        {4095, 3839, 0}, {767, 4095, 0}, {1791, 4095, 0}, {2815, 4095, 0}, {3839, 4095, 0},
        {511, 0, 255}, {1535, 0, 255}, {2559, 0, 255}, {3583, 0, 255}, {255, 255, 255},
        {1279, 255, 255}, {2303, 255, 255}, {3327, 255, 255}, {0, 511, 255}, {1023, 511, 255},
        {2047, 511, 255}, {3071, 511, 255}, {4095, 511, 255}, {767, 767, 255}, {1791, 767, 255},
        {2815, 767, 255}, {3839, 767, 255}, {511, 1023, 255}, {1535, 1023, 255}, {2559, 1023, 255},
        {3583, 1023, 255}, {255, 1279, 255}, {1279, 1279, 255}, {2303, 1279, 255},
        {3327, 1279, 255}, {0, 1535, 255}, {1023, 1535, 255}, {2047, 1535, 255}, {3071, 1535, 255},
        {4095, 1535, 255}, {767, 1791, 255}, {1791, 1791, 255}, {2815, 1791, 255},
        {3839, 1791, 255}, {511, 2047, 255}, {1535, 2047, 255}, {2559, 2047, 255},
        {3583, 2047, 255}, {255, 2303, 255}, {1279, 2303, 255}, {2303, 2303, 255},
        {3327, 2303, 255}, {0, 2559, 255}, {1023, 2559, 255}, {2047, 2559, 255}, {3071, 2559, 255},
        {4095, 2559, 255}, {767, 2815, 255}, {1791, 2815, 255}, {2815, 2815, 255},
        {3839, 2815, 255}, {511, 3071, 255}, {1535, 3071, 255}, {2559, 3071, 255},
        {3583, 3071, 255}, {255, 3327, 255}, {1279, 3327, 255}, {2303, 3327, 255},
        {3327, 3327, 255}, {0, 3583, 255}, {1023, 3583, 255}, {2047, 3583, 255}, {3071, 3583, 255},
        {4095, 3583, 255}, {767, 3839, 255}, {1791, 3839, 255}, {2815, 3839, 255},
        {3839, 3839, 255}, {511, 4095, 255}, {1535, 4095, 255}, {2559, 4095, 255},
        {3583, 4095, 255}, {255, 0, 511}, {1279, 0, 511}, {2303, 0, 511}, {3327, 0, 511},
        {0, 255, 511}, {1023, 255, 511}, {2047, 255, 511}, {3071, 255, 511}, {4095, 255, 511},
        {767, 511, 511}, {1791, 511, 511}, {2815, 511, 511}, {3839, 511, 511}, {511, 767, 511},
        {1535, 767, 511}, {2559, 767, 511}, {3583, 767, 511}, {255, 1023, 511}, {1279, 1023, 511},
        {2303, 1023, 511}, {3327, 1023, 511}, {0, 1279, 511}, {1023, 1279, 511}, {2047, 1279, 511},
        {3071, 1279, 511}, {4095, 1279, 511}, {767, 1535, 511}, {1791, 1535, 511},
        {2815, 1535, 511}, {3839, 1535, 511}, {511, 1791, 511}, {1535, 1791, 511},
        {2559, 1791, 511}, {3583, 1791, 511}, {255, 2047, 511}, {1279, 2047, 511},
        {2303, 2047, 511}, {3327, 2047, 511}, {0, 2303, 511}, {1023, 2303, 511}, {2047, 2303, 511},
        {3071, 2303, 511}, {4095, 2303, 511}, {767, 2559, 511}, {1791, 2559, 511},
        {2815, 2559, 511}, {3839, 2559, 511}, {511, 2815, 511}, {1535, 2815, 511},
        {2559, 2815, 511}, {3583, 2815, 511}, {255, 3071, 511}, {1279, 3071, 511},
        {2303, 3071, 511}, {3327, 3071, 511}, {0, 3327, 511}, {1023, 3327, 511}, {2047, 3327, 511},
        {3071, 3327, 511}, {4095, 3327, 511}, {767, 3583, 511}, {1791, 3583, 511},
        {2815, 3583, 511}, {3839, 3583, 511}, {511, 3839, 511}, {1535, 3839, 511},
        {2559, 3839, 511}, {3583, 3839, 511}, {255, 4095, 511}, {1279, 4095, 511},
        {2303, 4095, 511}, {3327, 4095, 511}, {0, 0, 767}, {1023, 0, 767}, {2047, 0, 767},
        {3071, 0, 767}, {4095, 0, 767}, {767, 255, 767}, {1791, 255, 767}, {2815, 255, 767},
        {3839, 255, 767}, {511, 511, 767}, {1535, 511, 767}, {2559, 511, 767}, {3583, 511, 767},
        {255, 767, 767}, {1279, 767, 767}, {2303, 767, 767}, {3327, 767, 767}, {0, 1023, 767},
        {1023, 1023, 767}, {2047, 1023, 767}, {3071, 1023, 767}, {4095, 1023, 767},
        {767, 1279, 767}, {1791, 1279, 767}, {2815, 1279, 767}, {3839, 1279, 767}, {511, 1535, 767},
        {1535, 1535, 767}, {2559, 1535, 767}, {3583, 1535, 767}, {255, 1791, 767},
        {1279, 1791, 767}, {2303, 1791, 767}, {3327, 1791, 767}, {0, 2047, 767}, {1023, 2047, 767},
        {2047, 2047, 767}, {3071, 2047, 767}, {4095, 2047, 767}, {767, 2303, 767},
        {1791, 2303, 767}, {2815, 2303, 767}, {3839, 2303, 767}, {511, 2559, 767},
        {1535, 2559, 767}, {2559, 2559, 767}, {3583, 2559, 767}, {255, 2815, 767},
        {1279, 2815, 767}, {2303, 2815, 767}, {3327, 2815, 767}, {0, 3071, 767}, {1023, 3071, 767},
        {2047, 3071, 767}, {3071, 3071, 767}, {4095, 3071, 767}, {767, 3327, 767},
        {1791, 3327, 767}, {2815, 3327, 767}, {3839, 3327, 767}, {511, 3583, 767},
        {1535, 3583, 767}, {2559, 3583, 767}, {3583, 3583, 767}, {255, 3839, 767},
        {1279, 3839, 767}, {2303, 3839, 767}, {3327, 3839, 767}, {0, 4095, 767}, {1023, 4095, 767},
        {2047, 4095, 767}, {3071, 4095, 767}, {4095, 4095, 767}, {767, 0, 1023}, {1791, 0, 1023},
        {2815, 0, 1023}, {3839, 0, 1023}, {511, 255, 1023}, {1535, 255, 1023}, {2559, 255, 1023},
        {3583, 255, 1023}, {255, 511, 1023}, {1279, 511, 1023}, {2303, 511, 1023},
        {3327, 511, 1023}, {0, 767, 1023}, {1023, 767, 1023}, {2047, 767, 1023}, {3071, 767, 1023},
        {4095, 767, 1023}, {767, 1023, 1023}, {1791, 1023, 1023}, {2815, 1023, 1023},
        {3839, 1023, 1023}, {511, 1279, 1023}, {1535, 1279, 1023}, {2559, 1279, 1023},
        {3583, 1279, 1023}, {255, 1535, 1023}, {1279, 1535, 1023}, {2303, 1535, 1023},
        {3327, 1535, 1023}, {0, 1791, 1023}, {1023, 1791, 1023}, {2047, 1791, 1023},
        {3071, 1791, 1023}, {4095, 1791, 1023}, {767, 2047, 1023}, {1791, 2047, 1023},
        {2815, 2047, 1023}, {3839, 2047, 1023}, {511, 2303, 1023}, {1535, 2303, 1023},
        {2559, 2303, 1023}, {3583, 2303, 1023}, {255, 2559, 1023}, {1279, 2559, 1023},
        {2303, 2559, 1023}, {3327, 2559, 1023}, {0, 2815, 1023}, {1023, 2815, 1023},
        {2047, 2815, 1023}, {3071, 2815, 1023}, {4095, 2815, 1023}, {767, 3071, 1023},
        {1791, 3071, 1023}, {2815, 3071, 1023}, {3839, 3071, 1023}, {511, 3327, 1023},
        {1535, 3327, 1023}, {2559, 3327, 1023}, {3583, 3327, 1023}, {255, 3583, 1023},
        {1279, 3583, 1023}, {2303, 3583, 1023}, {3327, 3583, 1023}, {0, 3839, 1023},
        {1023, 3839, 1023}, {2047, 3839, 1023}, {3071, 3839, 1023}, {4095, 3839, 1023},
        {767, 4095, 1023}, {1791, 4095, 1023}, {2815, 4095, 1023}, {3839, 4095, 1023},
        {511, 0, 1279}, {1535, 0, 1279}, {2559, 0, 1279}, {3583, 0, 1279}, {255, 255, 1279},
        {1279, 255, 1279}, {2303, 255, 1279}, {3327, 255, 1279}, {0, 511, 1279}, {1023, 511, 1279},
        {2047, 511, 1279}, {3071, 511, 1279}, {4095, 511, 1279}, {767, 767, 1279},
        {1791, 767, 1279}, {2815, 767, 1279}, {3839, 767, 1279}, {511, 1023, 1279},
        {1535, 1023, 1279}, {2559, 1023, 1279}, {3583, 1023, 1279}, {255, 1279, 1279},
        {1279, 1279, 1279}, {2303, 1279, 1279}, {3327, 1279, 1279}, {0, 1535, 1279},
        {1023, 1535, 1279}, {2047, 1535, 1279}, {3071, 1535, 1279}, {4095, 1535, 1279},
        {767, 1791, 1279}, {1791, 1791, 1279}, {2815, 1791, 1279}, {3839, 1791, 1279},
        {511, 2047, 1279}, {1535, 2047, 1279}, {2559, 2047, 1279}, {3583, 2047, 1279},
        {255, 2303, 1279}, {1279, 2303, 1279}, {2303, 2303, 1279}, {3327, 2303, 1279},
        {0, 2559, 1279}, {1023, 2559, 1279}, {2047, 2559, 1279}, {3071, 2559, 1279},
        {4095, 2559, 1279}, {767, 2815, 1279}, {1791, 2815, 1279}, {2815, 2815, 1279},
        {3839, 2815, 1279}, {511, 3071, 1279}, {1535, 3071, 1279}, {2559, 3071, 1279},
        {3583, 3071, 1279}, {255, 3327, 1279}, {1279, 3327, 1279}, {2303, 3327, 1279},
        {3327, 3327, 1279}, {0, 3583, 1279}, {1023, 3583, 1279}, {2047, 3583, 1279},
        {3071, 3583, 1279}, {4095, 3583, 1279}, {767, 3839, 1279}, {1791, 3839, 1279},
        {2815, 3839, 1279}, {3839, 3839, 1279}, {511, 4095, 1279}, {1535, 4095, 1279},
        {2559, 4095, 1279}, {3583, 4095, 1279}, {255, 0, 1535}, {1279, 0, 1535}, {2303, 0, 1535},
        {3327, 0, 1535}, {0, 255, 1535}, {1023, 255, 1535}, {2047, 255, 1535}, {3071, 255, 1535},
        {4095, 255, 1535}, {767, 511, 1535}, {1791, 511, 1535}, {2815, 511, 1535},
        {3839, 511, 1535}, {511, 767, 1535}, {1535, 767, 1535}, {2559, 767, 1535},
        {3583, 767, 1535}, {255, 1023, 1535}, {1279, 1023, 1535}, {2303, 1023, 1535},
        {3327, 1023, 1535}, {0, 1279, 1535}, {1023, 1279, 1535}, {2047, 1279, 1535},
        {3071, 1279, 1535}, {4095, 1279, 1535}, {767, 1535, 1535}, {1791, 1535, 1535},
        {2815, 1535, 1535}, {3839, 1535, 1535}, {511, 1791, 1535}, {1535, 1791, 1535},
        {2559, 1791, 1535}, {3583, 1791, 1535}, {255, 2047, 1535}, {1279, 2047, 1535},
        {2303, 2047, 1535}, {3327, 2047, 1535}, {0, 2303, 1535}, {1023, 2303, 1535},
        {2047, 2303, 1535}, {3071, 2303, 1535}, {4095, 2303, 1535}, {767, 2559, 1535},
        {1791, 2559, 1535}, {2815, 2559, 1535}, {3839, 2559, 1535}, {511, 2815, 1535},
        {1535, 2815, 1535}, {2559, 2815, 1535}, {3583, 2815, 1535}, {255, 3071, 1535},
        {1279, 3071, 1535}, {2303, 3071, 1535}, {3327, 3071, 1535}, {0, 3327, 1535},
        {1023, 3327, 1535}, {2047, 3327, 1535}, {3071, 3327, 1535}, {4095, 3327, 1535},
        {767, 3583, 1535}, {1791, 3583, 1535}, {2815, 3583, 1535}, {3839, 3583, 1535},
        {511, 3839, 1535}, {1535, 3839, 1535}, {2559, 3839, 1535}, {3583, 3839, 1535},
        {255, 4095, 1535}, {1279, 4095, 1535}, {2303, 4095, 1535}, {3327, 4095, 1535}, {0, 0, 1791},
        {1023, 0, 1791}, {2047, 0, 1791}, {3071, 0, 1791}, {4095, 0, 1791}, {767, 255, 1791},
        {1791, 255, 1791}, {2815, 255, 1791}, {3839, 255, 1791}, {511, 511, 1791},
        {1535, 511, 1791}, {2559, 511, 1791}, {3583, 511, 1791}, {255, 767, 1791},
        {1279, 767, 1791}, {2303, 767, 1791}, {3327, 767, 1791}, {0, 1023, 1791},
        {1023, 1023, 1791}, {2047, 1023, 1791}, {3071, 1023, 1791}, {4095, 1023, 1791},
        {767, 1279, 1791}, {1791, 1279, 1791}, {2815, 1279, 1791}, {3839, 1279, 1791},
        {511, 1535, 1791}, {1535, 1535, 1791}, {2559, 1535, 1791}, {3583, 1535, 1791},
        {255, 1791, 1791}, {1279, 1791, 1791}, {2303, 1791, 1791}, {3327, 1791, 1791},
        {0, 2047, 1791}, {1023, 2047, 1791}, {2047, 2047, 1791}, {3071, 2047, 1791},
        {4095, 2047, 1791}, {767, 2303, 1791}, {1791, 2303, 1791}, {2815, 2303, 1791},
        {3839, 2303, 1791}, {511, 2559, 1791}, {1535, 2559, 1791}, {2559, 2559, 1791},
        {3583, 2559, 1791}, {255, 2815, 1791}, {1279, 2815, 1791}, {2303, 2815, 1791},
        {3327, 2815, 1791}, {0, 3071, 1791}, {1023, 3071, 1791}, {2047, 3071, 1791},
        {3071, 3071, 1791}, {4095, 3071, 1791}, {767, 3327, 1791}, {1791, 3327, 1791},
        {2815, 3327, 1791}, {3839, 3327, 1791}, {511, 3583, 1791}, {1535, 3583, 1791},
        {2559, 3583, 1791}, {3583, 3583, 1791}, {255, 3839, 1791}, {1279, 3839, 1791},
        {2303, 3839, 1791}, {3327, 3839, 1791}, {0, 4095, 1791}, {1023, 4095, 1791},
        {2047, 4095, 1791}, {3071, 4095, 1791}, {4095, 4095, 1791}, {767, 0, 2047}, {1791, 0, 2047},
        {2815, 0, 2047}, {3839, 0, 2047}, {511, 255, 2047}, {1535, 255, 2047}, {2559, 255, 2047},
        {3583, 255, 2047}, {255, 511, 2047}, {1279, 511, 2047}, {2303, 511, 2047},
        {3327, 511, 2047}, {0, 767, 2047}, {1023, 767, 2047}, {2047, 767, 2047}, {3071, 767, 2047},
        {4095, 767, 2047}, {767, 1023, 2047}, {1791, 1023, 2047}, {2815, 1023, 2047},
        {3839, 1023, 2047}, {511, 1279, 2047}, {1535, 1279, 2047}, {2559, 1279, 2047},
        {3583, 1279, 2047}, {255, 1535, 2047}, {1279, 1535, 2047}, {2303, 1535, 2047},
        {3327, 1535, 2047}, {0, 1791, 2047}, {1023, 1791, 2047}, {2047, 1791, 2047},
        {3071, 1791, 2047}, {4095, 1791, 2047}, {767, 2047, 2047}, {1791, 2047, 2047},
        {2815, 2047, 2047}, {3839, 2047, 2047}, {511, 2303, 2047}, {1535, 2303, 2047},
        {2559, 2303, 2047}, {3583, 2303, 2047}, {255, 2559, 2047}, {1279, 2559, 2047},
        {2303, 2559, 2047}, {3327, 2559, 2047}, {0, 2815, 2047}, {1023, 2815, 2047},
        {2047, 2815, 2047}, {3071, 2815, 2047}, {4095, 2815, 2047}, {767, 3071, 2047},
        {1791, 3071, 2047}, {2815, 3071, 2047}, {3839, 3071, 2047}, {511, 3327, 2047},
        {1535, 3327, 2047}, {2559, 3327, 2047}, {3583, 3327, 2047}, {255, 3583, 2047},
        {1279, 3583, 2047}, {2303, 3583, 2047}, {3327, 3583, 2047}, {0, 3839, 2047},
        {1023, 3839, 2047}, {2047, 3839, 2047}, {3071, 3839, 2047}, {4095, 3839, 2047},
        {767, 4095, 2047}, {1791, 4095, 2047}, {2815, 4095, 2047}, {3839, 4095, 2047},
        {511, 0, 2303}, {1535, 0, 2303}, {2559, 0, 2303}, {3583, 0, 2303}, {255, 255, 2303},
        {1279, 255, 2303}, {2303, 255, 2303}, {3327, 255, 2303}, {0, 511, 2303}, {1023, 511, 2303},
        {2047, 511, 2303}, {3071, 511, 2303}, {4095, 511, 2303}, {767, 767, 2303},
        {1791, 767, 2303}, {2815, 767, 2303}, {3839, 767, 2303}, {511, 1023, 2303},
        {1535, 1023, 2303}, {2559, 1023, 2303}, {3583, 1023, 2303}, {255, 1279, 2303},
        {1279, 1279, 2303}, {2303, 1279, 2303}, {3327, 1279, 2303}, {0, 1535, 2303},
        {1023, 1535, 2303}, {2047, 1535, 2303}, {3071, 1535, 2303}, {4095, 1535, 2303},
        {767, 1791, 2303}, {1791, 1791, 2303}, {2815, 1791, 2303}, {3839, 1791, 2303},
        {511, 2047, 2303}, {1535, 2047, 2303}, {2559, 2047, 2303}, {3583, 2047, 2303},
        {255, 2303, 2303}, {1279, 2303, 2303}, {2303, 2303, 2303}, {3327, 2303, 2303},
        {0, 2559, 2303}, {1023, 2559, 2303}, {2047, 2559, 2303}, {3071, 2559, 2303},
        {4095, 2559, 2303}, {767, 2815, 2303}, {1791, 2815, 2303}, {2815, 2815, 2303},
        {3839, 2815, 2303}, {511, 3071, 2303}, {1535, 3071, 2303}, {2559, 3071, 2303},
        {3583, 3071, 2303}, {255, 3327, 2303}, {1279, 3327, 2303}, {2303, 3327, 2303},
        {3327, 3327, 2303}, {0, 3583, 2303}, {1023, 3583, 2303}, {2047, 3583, 2303},
        {3071, 3583, 2303}, {4095, 3583, 2303}, {767, 3839, 2303}, {1791, 3839, 2303},
        {2815, 3839, 2303}, {3839, 3839, 2303}, {511, 4095, 2303}, {1535, 4095, 2303},
        {2559, 4095, 2303}, {3583, 4095, 2303}, {255, 0, 2559}, {1279, 0, 2559}, {2303, 0, 2559},
        {3327, 0, 2559}, {0, 255, 2559}, {1023, 255, 2559}, {2047, 255, 2559}, {3071, 255, 2559},
        {4095, 255, 2559}, {767, 511, 2559}, {1791, 511, 2559}, {2815, 511, 2559},
        {3839, 511, 2559}, {511, 767, 2559}, {1535, 767, 2559}, {2559, 767, 2559},
        {3583, 767, 2559}, {255, 1023, 2559}, {1279, 1023, 2559}, {2303, 1023, 2559},
        {3327, 1023, 2559}, {0, 1279, 2559}, {1023, 1279, 2559}, {2047, 1279, 2559},
        {3071, 1279, 2559}, {4095, 1279, 2559}, {767, 1535, 2559}, {1791, 1535, 2559},
        {2815, 1535, 2559}, {3839, 1535, 2559}, {511, 1791, 2559}, {1535, 1791, 2559},
        {2559, 1791, 2559}, {3583, 1791, 2559}, {255, 2047, 2559}, {1279, 2047, 2559},
        {2303, 2047, 2559}, {3327, 2047, 2559}, {0, 2303, 2559}, {1023, 2303, 2559},
        {2047, 2303, 2559}, {3071, 2303, 2559}, {4095, 2303, 2559}, {767, 2559, 2559},
        {1791, 2559, 2559}, {2815, 2559, 2559}, {3839, 2559, 2559}, {511, 2815, 2559},
        {1535, 2815, 2559}, {2559, 2815, 2559}, {3583, 2815, 2559}, {255, 3071, 2559},
        {1279, 3071, 2559}, {2303, 3071, 2559}, {3327, 3071, 2559}, {0, 3327, 2559},
        {1023, 3327, 2559}, {2047, 3327, 2559}, {3071, 3327, 2559}, {4095, 3327, 2559},
        {767, 3583, 2559}, {1791, 3583, 2559}, {2815, 3583, 2559}, {3839, 3583, 2559},
        {511, 3839, 2559}, {1535, 3839, 2559}, {2559, 3839, 2559}, {3583, 3839, 2559},
        {255, 4095, 2559}, {1279, 4095, 2559}, {2303, 4095, 2559}, {3327, 4095, 2559}, {0, 0, 2815},
        {1023, 0, 2815}, {2047, 0, 2815}, {3071, 0, 2815}, {4095, 0, 2815}, {767, 255, 2815},
        {1791, 255, 2815}, {2815, 255, 2815}, {3839, 255, 2815}, {511, 511, 2815},
        {1535, 511, 2815}, {2559, 511, 2815}, {3583, 511, 2815}, {255, 767, 2815},
        {1279, 767, 2815}, {2303, 767, 2815}, {3327, 767, 2815}, {0, 1023, 2815},
        {1023, 1023, 2815}, {2047, 1023, 2815}, {3071, 1023, 2815}, {4095, 1023, 2815},
        {767, 1279, 2815}, {1791, 1279, 2815}, {2815, 1279, 2815}, {3839, 1279, 2815},
        {511, 1535, 2815}, {1535, 1535, 2815}, {2559, 1535, 2815}, {3583, 1535, 2815},
        {255, 1791, 2815}, {1279, 1791, 2815}, {2303, 1791, 2815}, {3327, 1791, 2815},
        {0, 2047, 2815}, {1023, 2047, 2815}, {2047, 2047, 2815}, {3071, 2047, 2815},
        {4095, 2047, 2815}, {767, 2303, 2815}, {1791, 2303, 2815}, {2815, 2303, 2815},
        {3839, 2303, 2815}, {511, 2559, 2815}, {1535, 2559, 2815}, {2559, 2559, 2815},
        {3583, 2559, 2815}, {255, 2815, 2815}, {1279, 2815, 2815}, {2303, 2815, 2815},
        {3327, 2815, 2815}, {0, 3071, 2815}, {1023, 3071, 2815}, {2047, 3071, 2815},
        {3071, 3071, 2815}, {4095, 3071, 2815}, {767, 3327, 2815}, {1791, 3327, 2815},
        {2815, 3327, 2815}, {3839, 3327, 2815}, {511, 3583, 2815}, {1535, 3583, 2815},
        {2559, 3583, 2815}, {3583, 3583, 2815}, {255, 3839, 2815}, {1279, 3839, 2815},
        {2303, 3839, 2815}, {3327, 3839, 2815}, {0, 4095, 2815}, {1023, 4095, 2815},
        {2047, 4095, 2815}, {3071, 4095, 2815}, {4095, 4095, 2815}, {767, 0, 3071}, {1791, 0, 3071},
        {2815, 0, 3071}, {3839, 0, 3071}, {511, 255, 3071}, {1535, 255, 3071}, {2559, 255, 3071},
        {3583, 255, 3071}, {255, 511, 3071}, {1279, 511, 3071}, {2303, 511, 3071},
        {3327, 511, 3071}, {0, 767, 3071}, {1023, 767, 3071}, {2047, 767, 3071}, {3071, 767, 3071},
        {4095, 767, 3071}, {767, 1023, 3071}, {1791, 1023, 3071}, {2815, 1023, 3071},
        {3839, 1023, 3071}, {511, 1279, 3071}, {1535, 1279, 3071}, {2559, 1279, 3071},
        {3583, 1279, 3071}, {255, 1535, 3071}, {1279, 1535, 3071}, {2303, 1535, 3071},
        {3327, 1535, 3071}, {0, 1791, 3071}, {1023, 1791, 3071}, {2047, 1791, 3071},
        {3071, 1791, 3071}, {4095, 1791, 3071}, {767, 2047, 3071}, {1791, 2047, 3071},
        {2815, 2047, 3071}, {3839, 2047, 3071}, {511, 2303, 3071}, {1535, 2303, 3071},
        {2559, 2303, 3071}, {3583, 2303, 3071}, {255, 2559, 3071}, {1279, 2559, 3071},
        {2303, 2559, 3071}, {3327, 2559, 3071}, {0, 2815, 3071}, {1023, 2815, 3071},
        {2047, 2815, 3071}, {3071, 2815, 3071}, {4095, 2815, 3071}, {767, 3071, 3071},
        {1791, 3071, 3071}, {2815, 3071, 3071}, {3839, 3071, 3071}, {511, 3327, 3071},
        {1535, 3327, 3071}, {2559, 3327, 3071}, {3583, 3327, 3071}, {255, 3583, 3071},
        {1279, 3583, 3071}, {2303, 3583, 3071}, {3327, 3583, 3071}, {0, 3839, 3071},
        {1023, 3839, 3071}, {2047, 3839, 3071}, {3071, 3839, 3071}, {4095, 3839, 3071},
        {767, 4095, 3071}, {1791, 4095, 3071}, {2815, 4095, 3071}, {3839, 4095, 3071},
        {511, 0, 3327}, {1535, 0, 3327}, {2559, 0, 3327}, {3583, 0, 3327}, {255, 255, 3327},
        {1279, 255, 3327}, {2303, 255, 3327}, {3327, 255, 3327}, {0, 511, 3327}, {1023, 511, 3327},
        {2047, 511, 3327}, {3071, 511, 3327}, {4095, 511, 3327}, {767, 767, 3327},
        {1791, 767, 3327}, {2815, 767, 3327}, {3839, 767, 3327}, {511, 1023, 3327},
        {1535, 1023, 3327}, {2559, 1023, 3327}, {3583, 1023, 3327}, {255, 1279, 3327},
        {1279, 1279, 3327}, {2303, 1279, 3327}, {3327, 1279, 3327}, {0, 1535, 3327},
        {1023, 1535, 3327}, {2047, 1535, 3327}, {3071, 1535, 3327}, {4095, 1535, 3327},
        {767, 1791, 3327}, {1791, 1791, 3327}, {2815, 1791, 3327}, {3839, 1791, 3327},
        {511, 2047, 3327}, {1535, 2047, 3327}, {2559, 2047, 3327}, {3583, 2047, 3327},
        {255, 2303, 3327}, {1279, 2303, 3327}, {2303, 2303, 3327}, {3327, 2303, 3327},
        {0, 2559, 3327}, {1023, 2559, 3327}, {2047, 2559, 3327}, {3071, 2559, 3327},
        {4095, 2559, 3327}, {767, 2815, 3327}, {1791, 2815, 3327}, {2815, 2815, 3327},
        {3839, 2815, 3327}, {511, 3071, 3327}, {1535, 3071, 3327}, {2559, 3071, 3327},
        {3583, 3071, 3327}, {255, 3327, 3327}, {1279, 3327, 3327}, {2303, 3327, 3327},
        {3327, 3327, 3327}, {0, 3583, 3327}, {1023, 3583, 3327}, {2047, 3583, 3327},
        {3071, 3583, 3327}, {4095, 3583, 3327}, {767, 3839, 3327}, {1791, 3839, 3327},
        {2815, 3839, 3327}, {3839, 3839, 3327}, {511, 4095, 3327}, {1535, 4095, 3327},
        {2559, 4095, 3327}, {3583, 4095, 3327}, {255, 0, 3583}, {1279, 0, 3583}, {2303, 0, 3583},
        {3327, 0, 3583}, {0, 255, 3583}, {1023, 255, 3583}, {2047, 255, 3583}, {3071, 255, 3583},
        {4095, 255, 3583}, {767, 511, 3583}, {1791, 511, 3583}, {2815, 511, 3583},
        {3839, 511, 3583}, {511, 767, 3583}, {1535, 767, 3583}, {2559, 767, 3583},
        {3583, 767, 3583}, {255, 1023, 3583}, {1279, 1023, 3583}, {2303, 1023, 3583},
        {3327, 1023, 3583}, {0, 1279, 3583}, {1023, 1279, 3583}, {2047, 1279, 3583},
        {3071, 1279, 3583}, {4095, 1279, 3583}, {767, 1535, 3583}, {1791, 1535, 3583},
        {2815, 1535, 3583}, {3839, 1535, 3583}, {511, 1791, 3583}, {1535, 1791, 3583},
        {2559, 1791, 3583}, {3583, 1791, 3583}, {255, 2047, 3583}, {1279, 2047, 3583},
        {2303, 2047, 3583}, {3327, 2047, 3583}, {0, 2303, 3583}, {1023, 2303, 3583},
        {2047, 2303, 3583}, {3071, 2303, 3583}, {4095, 2303, 3583}, {767, 2559, 3583},
        {1791, 2559, 3583}, {2815, 2559, 3583}, {3839, 2559, 3583}, {511, 2815, 3583},
        {1535, 2815, 3583}, {2559, 2815, 3583}, {3583, 2815, 3583}, {255, 3071, 3583},
        {1279, 3071, 3583}, {2303, 3071, 3583}, {3327, 3071, 3583}, {0, 3327, 3583},
        {1023, 3327, 3583}, {2047, 3327, 3583}, {3071, 3327, 3583}, {4095, 3327, 3583},
        {767, 3583, 3583}, {1791, 3583, 3583}, {2815, 3583, 3583}, {3839, 3583, 3583},
        {511, 3839, 3583}, {1535, 3839, 3583}, {2559, 3839, 3583}, {3583, 3839, 3583},
        {255, 4095, 3583}, {1279, 4095, 3583}, {2303, 4095, 3583}, {3327, 4095, 3583}, {0, 0, 3839},
        {1023, 0, 3839}, {2047, 0, 3839}, {3071, 0, 3839}, {4095, 0, 3839}, {767, 255, 3839},
        {1791, 255, 3839}, {2815, 255, 3839}, {3839, 255, 3839}, {511, 511, 3839},
        {1535, 511, 3839}, {2559, 511, 3839}, {3583, 511, 3839}, {255, 767, 3839},
        {1279, 767, 3839}, {2303, 767, 3839}, {3327, 767, 3839}, {0, 1023, 3839},
        {1023, 1023, 3839}, {2047, 1023, 3839}, {3071, 1023, 3839}, {4095, 1023, 3839},
        {767, 1279, 3839}, {1791, 1279, 3839}, {2815, 1279, 3839}, {3839, 1279, 3839},
        {511, 1535, 3839}, {1535, 1535, 3839}, {2559, 1535, 3839}, {3583, 1535, 3839},
        {255, 1791, 3839}, {1279, 1791, 3839}, {2303, 1791, 3839}, {3327, 1791, 3839},
        {0, 2047, 3839}, {1023, 2047, 3839}, {2047, 2047, 3839}, {3071, 2047, 3839},
        {4095, 2047, 3839}, {767, 2303, 3839}, {1791, 2303, 3839}, {2815, 2303, 3839},
        {3839, 2303, 3839}, {511, 2559, 3839}, {1535, 2559, 3839}, {2559, 2559, 3839},
        {3583, 2559, 3839}, {255, 2815, 3839}, {1279, 2815, 3839}, {2303, 2815, 3839},
        {3327, 2815, 3839}, {0, 3071, 3839}, {1023, 3071, 3839}, {2047, 3071, 3839},
        {3071, 3071, 3839}, {4095, 3071, 3839}, {767, 3327, 3839}, {1791, 3327, 3839},
        {2815, 3327, 3839}, {3839, 3327, 3839}, {511, 3583, 3839}, {1535, 3583, 3839},
        {2559, 3583, 3839}, {3583, 3583, 3839}, {255, 3839, 3839}, {1279, 3839, 3839},
        {2303, 3839, 3839}, {3327, 3839, 3839}, {0, 4095, 3839}, {1023, 4095, 3839},
        {2047, 4095, 3839}, {3071, 4095, 3839}, {4095, 4095, 3839}, {767, 0, 4095}, {1791, 0, 4095},
        {2815, 0, 4095}, {3839, 0, 4095}, {511, 255, 4095}, {1535, 255, 4095}, {2559, 255, 4095},
        {3583, 255, 4095}, {255, 511, 4095}, {1279, 511, 4095}, {2303, 511, 4095},
        {3327, 511, 4095}, {0, 767, 4095}, {1023, 767, 4095}, {2047, 767, 4095}, {3071, 767, 4095},
        {4095, 767, 4095}, {767, 1023, 4095}, {1791, 1023, 4095}, {2815, 1023, 4095},
        {3839, 1023, 4095}, {511, 1279, 4095}, {1535, 1279, 4095}, {2559, 1279, 4095},
        {3583, 1279, 4095}, {255, 1535, 4095}, {1279, 1535, 4095}, {2303, 1535, 4095},
        {3327, 1535, 4095}, {0, 1791, 4095}, {1023, 1791, 4095}, {2047, 1791, 4095},
        {3071, 1791, 4095}, {4095, 1791, 4095}, {767, 2047, 4095}, {1791, 2047, 4095},
        {2815, 2047, 4095}, {3839, 2047, 4095}, {511, 2303, 4095}, {1535, 2303, 4095},
        {2559, 2303, 4095}, {3583, 2303, 4095}, {255, 2559, 4095}, {1279, 2559, 4095},
        {2303, 2559, 4095}, {3327, 2559, 4095}, {0, 2815, 4095}, {1023, 2815, 4095},
        {2047, 2815, 4095}, {3071, 2815, 4095}, {4095, 2815, 4095}, {767, 3071, 4095},
        {1791, 3071, 4095}, {2815, 3071, 4095}, {3839, 3071, 4095}, {511, 3327, 4095},
        {1535, 3327, 4095}, {2559, 3327, 4095}, {3583, 3327, 4095}, {255, 3583, 4095},
        {1279, 3583, 4095}, {2303, 3583, 4095}, {3327, 3583, 4095}, {0, 3839, 4095},
        {1023, 3839, 4095}, {2047, 3839, 4095}, {3071, 3839, 4095}, {4095, 3839, 4095},
        {767, 4095, 4095}, {1791, 4095, 4095}, {2815, 4095, 4095}, {3839, 4095, 4095}}};

static const struct tetrahedral_17x17x17 tetra_sce_3dlut = {
    // lut 0
    {{0, 0, 0}, {1135, 0, 37}, {2163, 0, 70}, {3422, 0, 0}, {4095, 0, 0}, {900, 244, 0},
        {1848, 0, 0}, {3108, 0, 0}, {4095, 0, 0}, {552, 543, 0}, {1571, 331, 0}, {2804, 0, 0},
        {4065, 0, 0}, {0, 748, 0}, {1309, 684, 0}, {2515, 581, 0}, {3765, 0, 0}, {0, 1005, 0},
        {1018, 996, 0}, {2238, 951, 0}, {3472, 822, 0}, {4095, 461, 0}, {562, 1319, 0},
        {1954, 1279, 0}, {3185, 1206, 0}, {4095, 1044, 0}, {0, 1670, 0}, {1636, 1603, 0},
        {2894, 1547, 0}, {4095, 1441, 0}, {0, 2018, 0}, {1242, 1958, 0}, {2585, 1876, 0},
        {3819, 1792, 0}, {0, 2357, 0}, {443, 2319, 0}, {2245, 2200, 0}, {3512, 2127, 0},
        {4095, 2013, 0}, {0, 2670, 0}, {1867, 2561, 0}, {3184, 2452, 0}, {4095, 2352, 0},
        {0, 3006, 0}, {1302, 2920, 0}, {2828, 2771, 0}, {4084, 2678, 0}, {0, 3322, 0}, {0, 3263, 0},
        {2452, 3124, 0}, {3742, 2995, 0}, {0, 3622, 0}, {0, 3587, 0}, {1944, 3468, 0},
        {3374, 3303, 0}, {4095, 3188, 0}, {0, 3890, 0}, {1175, 3794, 0}, {2992, 3636, 0},
        {4095, 3491, 0}, {0, 4095, 0}, {0, 4095, 0}, {2508, 3955, 0}, {3866, 3783, 0}, {0, 4095, 0},
        {0, 4095, 0}, {1870, 4095, 0}, {3472, 4084, 0}, {0, 4095, 0}, {0, 4095, 0}, {761, 4095, 0},
        {2994, 4095, 0}, {4095, 4095, 0}, {949, 0, 321}, {1851, 0, 60}, {3115, 0, 101},
        {4095, 0, 0}, {682, 342, 351}, {1562, 0, 51}, {2799, 0, 91}, {4052, 0, 0}, {113, 558, 167},
        {1293, 392, 0}, {2485, 0, 0}, {3748, 0, 0}, {153, 753, 58}, {1018, 700, 0}, {2202, 626, 0},
        {3450, 350, 0}, {4095, 0, 0}, {670, 999, 0}, {1931, 969, 0}, {3160, 871, 0}, {4095, 605, 0},
        {0, 1330, 0}, {1642, 1291, 0}, {2875, 1233, 0}, {4095, 1100, 0}, {0, 1675, 0},
        {1304, 1625, 0}, {2583, 1566, 0}, {3811, 1476, 0}, {0, 2018, 0}, {755, 1984, 0},
        {2267, 1892, 0}, {3514, 1819, 0}, {4095, 1696, 0}, {0, 2341, 0}, {1922, 2233, 0},
        {3202, 2149, 0}, {4095, 2047, 0}, {0, 2687, 0}, {1455, 2597, 0}, {2865, 2472, 0},
        {4095, 2381, 0}, {0, 3010, 0}, {564, 2952, 0}, {2512, 2810, 0}, {3779, 2705, 0},
        {0, 3322, 0}, {0, 3289, 0}, {2072, 3165, 0}, {3427, 3020, 0}, {4095, 2908, 0}, {0, 3607, 0},
        {1440, 3506, 0}, {3063, 3345, 0}, {4095, 3220, 0}, {0, 3903, 0}, {0, 3827, 0},
        {2633, 3679, 0}, {3939, 3522, 0}, {0, 4095, 0}, {0, 4095, 0}, {2072, 3997, 0},
        {3564, 3826, 0}, {0, 4095, 0}, {0, 4095, 0}, {1239, 4095, 0}, {3131, 4095, 0},
        {4095, 3959, 0}, {0, 4095, 0}, {0, 4095, 0}, {2600, 4095, 0}, {3988, 4095, 0},
        {735, 245, 732}, {1560, 0, 462}, {2773, 0, 402}, {4049, 0, 241}, {403, 400, 741},
        {1320, 0, 491}, {2462, 0, 416}, {3735, 0, 296}, {225, 614, 629}, {1051, 463, 483},
        {2169, 231, 416}, {3432, 0, 318}, {4095, 0, 133}, {731, 727, 406}, {1885, 668, 385},
        {3125, 498, 295}, {4095, 0, 0}, {204, 1003, 256}, {1622, 986, 328}, {2842, 920, 250},
        {4079, 723, 0}, {269, 1326, 102}, {1322, 1303, 195}, {2562, 1259, 61}, {3789, 1154, 0},
        {338, 1664, 127}, {894, 1653, 0}, {2269, 1584, 0}, {3500, 1510, 0}, {4095, 1370, 0},
        {0, 2014, 0}, {1939, 1907, 0}, {3203, 1844, 0}, {4095, 1739, 0}, {0, 2354, 0},
        {1553, 2271, 0}, {2886, 2170, 0}, {4095, 2080, 0}, {542, 2667, 204}, {902, 2635, 0},
        {2538, 2491, 0}, {3801, 2409, 0}, {0, 3007, 0}, {0, 2985, 0}, {2161, 2852, 0},
        {3466, 2730, 0}, {4095, 2621, 0}, {0, 3316, 0}, {1624, 3207, 0}, {3104, 3044, 0},
        {4095, 2941, 0}, {0, 3617, 0}, {622, 3544, 0}, {2726, 3390, 0}, {3998, 3251, 0},
        {0, 3903, 0}, {0, 3859, 0}, {2228, 3724, 0}, {3625, 3552, 0}, {0, 4095, 0}, {0, 4095, 0},
        {1533, 4038, 0}, {3238, 3871, 0}, {4095, 3716, 0}, {0, 4095, 0}, {0, 4095, 0},
        {2757, 4095, 0}, {4082, 3996, 0}, {0, 4095, 0}, {0, 4095, 0}, {2147, 4095, 0},
        {3678, 4095, 0}, {390, 236, 1080}, {1322, 0, 791}, {2433, 0, 737}, {3693, 0, 696},
        {160, 371, 1036}, {1096, 261, 830}, {2132, 0, 741}, {3379, 0, 709}, {4095, 0, 622},
        {826, 545, 832}, {1850, 379, 741}, {3079, 0, 714}, {4095, 0, 642}, {542, 778, 798},
        {1586, 701, 732}, {2798, 587, 712}, {4049, 0, 649}, {203, 1013, 711}, {1314, 998, 686},
        {2519, 962, 683}, {3774, 801, 643}, {267, 1323, 643}, {964, 1326, 590}, {2246, 1281, 643},
        {3469, 1206, 598}, {4095, 1029, 476}, {338, 1665, 500}, {1950, 1600, 573},
        {3185, 1542, 542}, {4095, 1429, 429}, {405, 1998, 359}, {1608, 1943, 394},
        {2888, 1867, 441}, {4095, 1779, 326}, {473, 2331, 179}, {1080, 2317, 0}, {2564, 2190, 245},
        {3806, 2112, 0}, {540, 2660, 204}, {0, 2679, 0}, {2214, 2533, 0}, {3489, 2436, 0},
        {4095, 2329, 0}, {0, 3001, 0}, {1754, 2900, 0}, {3146, 2755, 0}, {4095, 2655, 0},
        {669, 3292, 251}, {1004, 3253, 0}, {2787, 3087, 0}, {4042, 2972, 0}, {729, 3590, 274},
        {0, 3584, 0}, {2349, 3437, 0}, {3685, 3280, 0}, {787, 3874, 296}, {0, 3892, 0},
        {1745, 3770, 0}, {3315, 3596, 0}, {4095, 3460, 0}, {0, 4095, 0}, {614, 4079, 0},
        {2883, 3918, 0}, {4095, 3752, 0}, {0, 4095, 0}, {0, 4095, 0}, {2332, 4095, 0},
        {3782, 4041, 0}, {0, 4095, 0}, {0, 4095, 0}, {1576, 4095, 0}, {3343, 4095, 0},
        {126, 164, 1338}, {1092, 128, 1102}, {2111, 0, 1032}, {3333, 0, 1020}, {4095, 0, 973},
        {848, 288, 1143}, {1827, 0, 1033}, {3024, 0, 1025}, {4095, 0, 988}, {543, 524, 1125},
        {1556, 457, 1035}, {2729, 175, 1027}, {3978, 0, 997}, {214, 759, 1063}, {1294, 744, 1037},
        {2451, 664, 1027}, {3688, 405, 1001}, {199, 1016, 1042}, {1032, 1005, 1031},
        {2195, 981, 1023}, {3416, 882, 1000}, {4095, 611, 939}, {615, 1315, 1001},
        {1930, 1298, 987}, {3140, 1249, 971}, {4095, 1093, 934}, {331, 1642, 961},
        {1623, 1613, 938}, {2866, 1570, 937}, {4079, 1482, 892}, {400, 1977, 895},
        {1189, 2000, 808}, {2570, 1888, 879}, {3792, 1817, 848}, {468, 2314, 793}, {473, 2328, 730},
        {2233, 2208, 802}, {3493, 2140, 778}, {4095, 2037, 704}, {538, 2651, 598},
        {1845, 2587, 574}, {3172, 2460, 679}, {4095, 2366, 611}, {602, 2970, 338}, {1219, 2957, 0},
        {2817, 2777, 537}, {4070, 2687, 462}, {666, 3281, 251}, {0, 3303, 0}, {2438, 3139, 0},
        {3731, 3001, 37}, {727, 3580, 274}, {727, 3578, 273}, {1905, 3491, 0}, {3363, 3308, 0},
        {4095, 3194, 0}, {785, 3863, 295}, {1056, 3820, 0}, {2979, 3645, 0}, {4095, 3495, 0},
        {827, 4069, 311}, {0, 4095, 0}, {2483, 3968, 0}, {3856, 3786, 0}, {827, 4069, 311},
        {0, 4095, 0}, {1817, 4095, 0}, {3460, 4088, 0}, {827, 4069, 311}, {0, 4095, 0},
        {517, 4095, 0}, {2975, 4095, 0}, {4095, 4095, 0}, {830, 0, 1412}, {1817, 0, 1307},
        {2984, 0, 1315}, {4095, 0, 1297}, {564, 232, 1453}, {1547, 189, 1304}, {2684, 0, 1314},
        {3915, 0, 1305}, {241, 496, 1412}, {1288, 500, 1303}, {2396, 368, 1313}, {3615, 0, 1309},
        {128, 742, 1336}, {1030, 746, 1312}, {2120, 714, 1313}, {3326, 563, 1311}, {4095, 0, 1277},
        {755, 1007, 1322}, {1855, 1002, 1314}, {3055, 939, 1313}, {4095, 756, 1281},
        {558, 1270, 1304}, {1617, 1290, 1323}, {2805, 1260, 1313}, {4021, 1159, 1281},
        {317, 1592, 1316}, {1304, 1648, 1299}, {2545, 1592, 1282}, {3774, 1508, 1274},
        {389, 1937, 1284}, {789, 1966, 1261}, {2248, 1904, 1234}, {3473, 1849, 1221},
        {4095, 1748, 1168}, {464, 2294, 1206}, {1904, 2262, 1148}, {3177, 2165, 1169},
        {4095, 2079, 1129}, {530, 2622, 1126}, {1363, 2659, 1010}, {2848, 2482, 1100},
        {4078, 2399, 1070}, {597, 2947, 1015}, {601, 2961, 937}, {2495, 2826, 974},
        {3759, 2716, 990}, {661, 3262, 860}, {663, 3267, 802}, {2029, 3201, 739}, {3411, 3028, 889},
        {4095, 2920, 831}, {723, 3565, 589}, {1309, 3551, 331}, {3046, 3354, 684},
        {4095, 3228, 698}, {781, 3852, 295}, {0, 3867, 0}, {2604, 3700, 0}, {3924, 3528, 503},
        {826, 4069, 311}, {827, 4069, 311}, {2005, 4023, 0}, {3546, 3831, 0}, {826, 4069, 311},
        {827, 4069, 311}, {1055, 4095, 0}, {3107, 4095, 0}, {4095, 3961, 0}, {827, 4069, 311},
        {0, 4095, 0}, {2556, 4095, 0}, {3970, 4095, 0}, {550, 0, 1718}, {1543, 0, 1563},
        {2654, 0, 1589}, {3863, 0, 1595}, {272, 55, 1734}, {1287, 192, 1572}, {2366, 0, 1583},
        {3558, 0, 1596}, {44, 453, 1675}, {1027, 465, 1585}, {2087, 449, 1577}, {3261, 0, 1596},
        {4095, 0, 1583}, {759, 728, 1602}, {1815, 747, 1570}, {2975, 652, 1596}, {4095, 331, 1588},
        {468, 1003, 1607}, {1554, 1013, 1568}, {2700, 976, 1596}, {3919, 855, 1592},
        {238, 1276, 1588}, {1309, 1279, 1600}, {2438, 1270, 1600}, {3652, 1209, 1594},
        {298, 1525, 1564}, {1058, 1525, 1565}, {2212, 1574, 1615}, {3406, 1532, 1597},
        {4095, 1428, 1554}, {519, 1881, 1606}, {1918, 1916, 1573}, {3151, 1876, 1568},
        {4095, 1771, 1548}, {447, 2231, 1587}, {1548, 2300, 1554}, {2858, 2184, 1522},
        {4062, 2116, 1495}, {518, 2575, 1535}, {949, 2610, 1498}, {2516, 2501, 1462},
        {3765, 2428, 1446}, {587, 2910, 1456}, {592, 2926, 1427}, {2125, 2900, 1338},
        {3442, 2741, 1380}, {4095, 2643, 1340}, {655, 3238, 1333}, {1486, 3277, 1186},
        {3082, 3052, 1304}, {4095, 2953, 1269}, {718, 3542, 1211}, {720, 3548, 1117},
        {2698, 3415, 1121}, {3976, 3259, 1181}, {777, 3833, 1053}, {778, 3832, 976},
        {2158, 3763, 881}, {3604, 3558, 1077}, {825, 4069, 835}, {825, 4069, 774},
        {1349, 4082, 521}, {3212, 3884, 797}, {4095, 3721, 888}, {825, 4069, 410}, {0, 4095, 0},
        {2708, 4095, 180}, {4061, 3999, 719}, {826, 4069, 311}, {827, 4069, 311}, {2049, 4095, 0},
        {3652, 4095, 0}, {275, 0, 1992}, {1289, 0, 1848}, {2347, 0, 1844}, {3515, 0, 1873},
        {0, 0, 1982}, {1028, 0, 1864}, {2070, 151, 1833}, {3217, 0, 1868}, {4095, 0, 1872},
        {764, 418, 1882}, {1803, 495, 1825}, {2926, 331, 1863}, {4095, 0, 1875}, {479, 708, 1899},
        {1555, 745, 1842}, {2643, 706, 1857}, {3840, 526, 1876}, {170, 997, 1895},
        {1302, 1004, 1867}, {2364, 999, 1850}, {3557, 921, 1877}, {231, 1277, 1881},
        {1031, 1280, 1904}, {2088, 1269, 1839}, {3284, 1239, 1879}, {4095, 1130, 1866},
        {772, 1544, 1866}, {1823, 1527, 1834}, {3027, 1539, 1885}, {4095, 1472, 1869},
        {602, 1779, 1825}, {1570, 1780, 1828}, {2804, 1851, 1902}, {3994, 1796, 1872},
        {424, 2137, 1885}, {1212, 2181, 1901}, {2535, 2199, 1866}, {3735, 2144, 1840},
        {500, 2502, 1883}, {507, 2529, 1880}, {2200, 2588, 1811}, {3448, 2452, 1799},
        {4095, 2368, 1756}, {576, 2865, 1831}, {1748, 2922, 1775}, {3120, 2763, 1738},
        {4095, 2676, 1709}, {644, 3192, 1759}, {1077, 3221, 1705}, {2763, 3110, 1646},
        {4010, 2983, 1645}, {708, 3506, 1663}, {712, 3518, 1619}, {2283, 3492, 1499},
        {3659, 3287, 1571}, {770, 3805, 1543}, {771, 3807, 1512}, {1571, 3829, 1343},
        {3288, 3607, 1450}, {4095, 3467, 1444}, {824, 4069, 1379}, {826, 4069, 1268},
        {2835, 3945, 1242}, {4095, 3756, 1349}, {824, 4069, 1209}, {826, 4069, 1115},
        {2229, 4095, 993}, {3753, 4045, 1187}, {825, 4069, 988}, {825, 4069, 908},
        {1333, 4095, 646}, {3301, 4095, 881}, {93, 0, 2240}, {1031, 0, 2150}, {2058, 0, 2084},
        {3184, 0, 2133}, {4095, 0, 2151}, {770, 0, 2169}, {1808, 156, 2099}, {2895, 0, 2123},
        {4079, 0, 2150}, {498, 354, 2186}, {1555, 453, 2118}, {2611, 430, 2111}, {3783, 0, 2147},
        {128, 686, 2196}, {1299, 721, 2143}, {2332, 742, 2096}, {3493, 627, 2144}, {161, 987, 2190},
        {1034, 993, 2173}, {2068, 1009, 2090}, {3208, 963, 2139}, {4095, 825, 2152},
        {745, 1275, 2193}, {1829, 1269, 2118}, {2928, 1256, 2134}, {4095, 1183, 2154},
        {460, 1548, 2162}, {1585, 1547, 2164}, {2653, 1532, 2127}, {3866, 1500, 2158},
        {340, 1801, 2125}, {1311, 1817, 2153}, {2380, 1799, 2121}, {3612, 1805, 2164},
        {398, 2034, 2086}, {1084, 2034, 2087}, {2174, 2118, 2172}, {3388, 2119, 2179},
        {4095, 2051, 2137}, {478, 2409, 2163}, {1849, 2505, 2206}, {3128, 2471, 2146},
        {4095, 2375, 2135}, {554, 2775, 2170}, {1361, 2831, 2168}, {2788, 2781, 2086},
        {4016, 2705, 2062}, {627, 3123, 2131}, {635, 3153, 2114}, {2391, 3213, 2002},
        {3696, 3008, 2002}, {695, 3454, 2059}, {699, 3466, 2042}, {1902, 3499, 1960},
        {3332, 3312, 1932}, {4095, 3205, 1894}, {760, 3768, 1950}, {1170, 3780, 1877},
        {2940, 3676, 1792}, {4095, 3500, 1819}, {819, 4053, 1838}, {821, 4053, 1777},
        {2380, 4017, 1626}, {3824, 3788, 1736}, {823, 4069, 1701}, {824, 4069, 1654},
        {1610, 4095, 1466}, {3420, 4095, 1552}, {824, 4069, 1538}, {824, 4069, 1506},
        {826, 4069, 1379}, {2901, 4095, 1329}, {4095, 4095, 1482}, {777, 0, 2462}, {1814, 0, 2379},
        {2871, 0, 2376}, {4037, 0, 2418}, {516, 0, 2478}, {1558, 0, 2400}, {2590, 65, 2360},
        {3741, 0, 2411}, {219, 265, 2488}, {1299, 389, 2425}, {2318, 488, 2346}, {3449, 268, 2402},
        {58, 660, 2492}, {1037, 690, 2453}, {2074, 738, 2367}, {3163, 689, 2391}, {4095, 475, 2424},
        {757, 977, 2481}, {1829, 998, 2396}, {2879, 990, 2378}, {4060, 896, 2422},
        {435, 1268, 2484}, {1578, 1267, 2433}, {2596, 1263, 2359}, {3777, 1220, 2419},
        {275, 1548, 2460}, {1311, 1551, 2479}, {2336, 1523, 2357}, {3499, 1515, 2416},
        {334, 1813, 2429}, {1028, 1821, 2451}, {2109, 1798, 2401}, {3227, 1797, 2415},
        {4095, 1757, 2429}, {792, 2064, 2396}, {1865, 2100, 2457}, {2968, 2079, 2420},
        {4095, 2063, 2435}, {659, 2288, 2347}, {1589, 2289, 2349}, {2785, 2419, 2479},
        {3959, 2375, 2446}, {526, 2660, 2429}, {1156, 2686, 2443}, {2536, 2861, 2523},
        {3698, 2729, 2410}, {604, 3031, 2448}, {609, 3050, 2449}, {2048, 3135, 2446},
        {3376, 3029, 2350}, {4095, 2940, 2309}, {680, 3389, 2410}, {1485, 3434, 2381},
        {3020, 3383, 2271}, {4095, 3234, 2248}, {746, 3710, 2341}, {753, 3730, 2305},
        {2511, 3770, 2147}, {3891, 3531, 2178}, {808, 4009, 2246}, {811, 4015, 2213},
        {2011, 4015, 2106}, {3511, 3841, 2070}, {821, 4069, 2126}, {822, 4069, 2103},
        {1195, 4069, 2009}, {3039, 4095, 1897}, {4095, 3959, 1961}, {822, 4069, 1972},
        {824, 4069, 1892}, {2413, 4095, 1711}, {3928, 4095, 1828}, {527, 0, 2775}, {1564, 0, 2689},
        {2573, 0, 2605}, {3707, 0, 2669}, {272, 0, 2785}, {1304, 0, 2714}, {2327, 89, 2624},
        {3417, 0, 2656}, {0, 96, 2787}, {1041, 284, 2741}, {2079, 435, 2647}, {3132, 402, 2640},
        {4095, 0, 2688}, {770, 650, 2767}, {1829, 709, 2676}, {2849, 733, 2621}, {4007, 591, 2681},
        {462, 958, 2789}, {1576, 981, 2711}, {2582, 1005, 2611}, {3720, 945, 2672},
        {205, 1258, 2780}, {1312, 1260, 2750}, {2345, 1262, 2639}, {3434, 1243, 2660},
        {268, 1544, 2761}, {1023, 1547, 2788}, {2107, 1530, 2678}, {3150, 1519, 2645},
        {4095, 1476, 2694}, {729, 1821, 2747}, {1860, 1814, 2731}, {2864, 1780, 2623},
        {4069, 1772, 2694}, {458, 2078, 2702}, {1582, 2099, 2748}, {2606, 2038, 2623},
        {3801, 2059, 2696}, {441, 2318, 2654}, {1318, 2329, 2671}, {2403, 2350, 2708},
        {3549, 2349, 2705}, {497, 2542, 2608}, {1117, 2543, 2609}, {2100, 2544, 2612},
        {3354, 2678, 2746}, {4095, 2620, 2699}, {578, 2922, 2698}, {1760, 2969, 2725},
        {3047, 3045, 2697}, {4095, 2969, 2656}, {655, 3288, 2720}, {1195, 3324, 2723},
        {2714, 3429, 2705}, {3930, 3259, 2597}, {727, 3631, 2688}, {732, 3651, 2677},
        {2195, 3700, 2626}, {3565, 3554, 2525}, {793, 3949, 2617}, {795, 3956, 2604},
        {1573, 3974, 2544}, {3165, 3925, 2403}, {4095, 3735, 2411}, {819, 4069, 2507},
        {822, 4069, 2449}, {2575, 4095, 2255}, {4027, 4002, 2322}, {820, 4069, 2388},
        {822, 4069, 2337}, {2041, 4068, 2204}, {3595, 4095, 2145}, {295, 0, 3084}, {1311, 0, 3011},
        {2337, 0, 2907}, {3391, 0, 2904}, {133, 0, 3087}, {1049, 0, 3037}, {2085, 0, 2932},
        {3108, 0, 2885}, {4095, 0, 2946}, {782, 0, 3061}, {1832, 353, 2961}, {2833, 478, 2867},
        {3968, 150, 2935}, {496, 602, 3080}, {1576, 669, 2996}, {2591, 730, 2890},
        {3680, 666, 2921}, {114, 937, 3091}, {1314, 958, 3032}, {2349, 989, 2920},
        {3394, 979, 2903}, {195, 1245, 3080}, {1036, 1247, 3067}, {2105, 1255, 2958},
        {3109, 1258, 2880}, {4095, 1200, 2949}, {726, 1538, 3078}, {1854, 1530, 3002},
        {2849, 1519, 2878}, {3993, 1496, 2941}, {375, 1818, 3044}, {1587, 1818, 3052},
        {2619, 1784, 2913}, {3710, 1776, 2930}, {378, 2085, 3007}, {1296, 2096, 3040},
        {2387, 2065, 2965}, {3426, 2043, 2915}, {435, 2338, 2964}, {1027, 2345, 2979},
        {2142, 2374, 3043}, {3140, 2298, 2894}, {4095, 2315, 2966}, {819, 2577, 2920},
        {1849, 2597, 2951}, {2874, 2552, 2888}, {4095, 2607, 2974}, {726, 2796, 2869},
        {1612, 2797, 2871}, {2614, 2799, 2874}, {3906, 2924, 3000}, {626, 3169, 2958},
        {1112, 3184, 2967}, {2347, 3260, 3008}, {3614, 3278, 2945}, {703, 3533, 2984},
        {706, 3544, 2985}, {1879, 3594, 2985}, {3264, 3648, 2878}, {4095, 3495, 2847},
        {775, 3876, 2950}, {1212, 3901, 2927}, {2845, 3944, 2846}, {4095, 3782, 2773},
        {816, 4068, 2877}, {818, 4068, 2848}, {2289, 4068, 2760}, {3729, 4069, 2653},
        {818, 4069, 2772}, {819, 4069, 2744}, {1583, 4068, 2653}, {3209, 4095, 2470},
        {184, 0, 3389}, {1058, 0, 3338}, {2093, 0, 3223}, {3088, 0, 3126}, {4095, 0, 3199},
        {797, 0, 3361}, {1838, 0, 3253}, {2844, 0, 3147}, {3936, 0, 3185}, {525, 0, 3379},
        {1579, 186, 3287}, {2599, 411, 3172}, {3650, 365, 3167}, {191, 545, 3390},
        {1318, 612, 3322}, {2353, 693, 3203}, {3366, 721, 3144}, {86, 912, 3391}, {1046, 928, 3354},
        {2105, 967, 3241}, {3096, 1000, 3132}, {4095, 923, 3200}, {748, 1230, 3382},
        {1852, 1243, 3283}, {2859, 1255, 3160}, {3944, 1229, 3185}, {387, 1527, 3373},
        {1588, 1525, 3326}, {2623, 1518, 3196}, {3657, 1509, 3165}, {312, 1812, 3345},
        {1302, 1815, 3369}, {2383, 1791, 3241}, {3370, 1773, 3140}, {371, 2086, 3313},
        {1001, 2091, 3334}, {2132, 2077, 3296}, {3118, 2033, 3145}, {4095, 2031, 3209},
        {710, 2352, 3283}, {1855, 2371, 3338}, {2894, 2307, 3189}, {3990, 2304, 3200},
        {485, 2598, 3230}, {1570, 2613, 3260}, {2670, 2610, 3263}, {3713, 2571, 3191},
        {541, 2830, 3177}, {1331, 2837, 3188}, {2387, 2870, 3240}, {3439, 2832, 3181},
        {597, 3051, 3129}, {1158, 3051, 3130}, {2118, 3052, 3133}, {3255, 3172, 3252},
        {4095, 3170, 3253}, {675, 3423, 3220}, {1712, 3446, 3232}, {2940, 3555, 3286},
        {4095, 3526, 3203}, {751, 3777, 3242}, {1034, 3796, 3242}, {2504, 3865, 3241},
        {3826, 3821, 3133}, {812, 4068, 3211}, {813, 4068, 3205}, {1963, 4068, 3172},
        {3364, 4095, 2984}, {815, 4068, 3129}, {816, 4068, 3118}, {1158, 4068, 3071},
        {2895, 4067, 2926}, {4095, 4095, 2852}, {807, 0, 3666}, {1845, 0, 3551}, {2856, 0, 3431},
        {3909, 0, 3430}, {551, 0, 3683}, {1586, 0, 3584}, {2608, 0, 3459}, {3625, 0, 3408},
        {279, 0, 3693}, {1324, 0, 3618}, {2359, 302, 3491}, {3347, 465, 3387}, {0, 478, 3696},
        {1056, 536, 3650}, {2108, 642, 3528}, {3107, 719, 3412}, {4095, 638, 3447},
        {772, 891, 3675}, {1853, 936, 3569}, {2867, 978, 3442}, {3909, 966, 3427},
        {434, 1209, 3692}, {1590, 1224, 3611}, {2625, 1242, 3479}, {3624, 1251, 3403},
        {238, 1514, 3671}, {1313, 1514, 3649}, {2381, 1513, 3523}, {3362, 1514, 3399},
        {304, 1804, 3648}, {1007, 1806, 3672}, {2129, 1792, 3571}, {3130, 1774, 3431},
        {4095, 1761, 3448}, {682, 2086, 3630}, {1860, 2080, 3621}, {2897, 2044, 3473},
        {3922, 2030, 3428}, {422, 2354, 3586}, {1566, 2365, 3627}, {2660, 2327, 3527},
        {3634, 2286, 3402}, {479, 2610, 3539}, {1280, 2619, 3562}, {2406, 2628, 3595},
        {3385, 2546, 3410}, {536, 2853, 3488}, {1030, 2858, 3498}, {2112, 2882, 3546},
        {3170, 2834, 3467}, {4095, 2831, 3470}, {855, 3085, 3439}, {1850, 3097, 3457},
        {2937, 3149, 3540}, {4031, 3118, 3484}, {801, 3305, 3390}, {1642, 3306, 3392},
        {2629, 3307, 3395}, {3882, 3487, 3573}, {723, 3667, 3475}, {1074, 3674, 3478},
        {2286, 3723, 3511}, {3581, 3869, 3564}, {798, 4022, 3504}, {801, 4033, 3507},
        {1768, 4068, 3517}, {3102, 4067, 3474}, {4095, 3981, 3320}, {812, 4068, 3462},
        {858, 4068, 3447}, {2575, 4067, 3368}, {3880, 4095, 3167}, {566, 0, 3989}, {1594, 0, 3887},
        {2619, 0, 3750}, {3602, 0, 3646}, {333, 0, 3999}, {1333, 0, 3920}, {2366, 0, 3784},
        {3360, 0, 3669}, {154, 0, 4001}, {1068, 0, 3951}, {2112, 0, 3821}, {3117, 380, 3695},
        {4095, 315, 3691}, {792, 433, 3975}, {1856, 569, 3861}, {2874, 673, 3727},
        {3882, 707, 3668}, {487, 848, 3992}, {1594, 895, 3902}, {2629, 949, 3765},
        {3609, 994, 3652}, {141, 1186, 3990}, {1322, 1199, 3939}, {2381, 1224, 3807},
        {3373, 1247, 3680}, {226, 1498, 3974}, {1028, 1498, 3970}, {2127, 1503, 3853},
        {3137, 1507, 3715}, {4095, 1499, 3686}, {697, 1794, 3965}, {1862, 1787, 3899},
        {2899, 1774, 3756}, {3882, 1768, 3660}, {355, 2078, 3929}, {1577, 2078, 3943},
        {2656, 2050, 3805}, {3629, 2027, 3665}, {415, 2352, 3890}, {1272, 2358, 3919},
        {2402, 2337, 3858}, {3400, 2293, 3701}, {473, 2615, 3847}, {973, 2620, 3863},
        {2124, 2634, 3915}, {3171, 2570, 3749}, {4095, 2547, 3691}, {694, 2870, 3806},
        {1829, 2885, 3844}, {2933, 2865, 3814}, {3903, 2799, 3666}, {584, 3109, 3749},
        {1565, 3119, 3769}, {2656, 3149, 3832}, {3651, 3058, 3672}, {641, 3337, 3696},
        {1351, 3341, 3702}, {2371, 3357, 3727}, {3496, 3418, 3811}, {696, 3559, 3651},
        {1206, 3559, 3652}, {2155, 3580, 3674}, {3247, 3676, 3775}, {4095, 3730, 3823},
        {776, 3937, 3751}, {1705, 3971, 3779}, {2880, 4040, 3824}, {4025, 4024, 3682},
        {807, 4068, 3764}, {843, 4068, 3770}, {2363, 4067, 3754}, {3598, 4066, 3588},
        {363, 0, 4095}, {1342, 0, 4095}, {2375, 0, 4082}, {3374, 0, 3954}, {253, 0, 4095},
        {1081, 0, 4095}, {2120, 0, 4095}, {3128, 0, 3983}, {4095, 0, 3931}, {815, 0, 4095},
        {1861, 0, 4095}, {2881, 225, 4016}, {3861, 450, 3908}, {530, 279, 4095}, {1600, 461, 4095},
        {2633, 609, 4054}, {3622, 705, 3933}, {142, 800, 4095}, {1331, 843, 4095},
        {2383, 910, 4095}, {3382, 965, 3963}, {119, 1160, 4095}, {1047, 1168, 4095},
        {2128, 1199, 4095}, {3143, 1228, 3999}, {4095, 1244, 3924}, {730, 1478, 4095},
        {1865, 1487, 4095}, {2901, 1496, 4041}, {3875, 1509, 3918}, {311, 1780, 4095},
        {1587, 1777, 4095}, {2655, 1770, 4087}, {3641, 1766, 3949}, {348, 2069, 4095},
        {1284, 2070, 4095}, {2400, 2051, 4095}, {3408, 2030, 3987}, {408, 2347, 4095},
        {965, 2350, 4095}, {2130, 2339, 4095}, {3171, 2302, 4032}, {4095, 2281, 3918},
        {625, 2618, 4095}, {1834, 2627, 4095}, {2927, 2585, 4084}, {3896, 2540, 3930},
        {522, 2876, 4095}, {1538, 2885, 4095}, {2667, 2878, 4095}, {3669, 2810, 3969},
        {579, 3123, 4058}, {1265, 3130, 4074}, {2373, 3148, 4095}, {3439, 3095, 4023},
        {634, 3360, 4005}, {1039, 3364, 4011}, {2094, 3377, 4038}, {3215, 3425, 4095},
        {4095, 3409, 4044}, {899, 3590, 3955}, {1869, 3609, 3980}, {2956, 3689, 4079},
        {4075, 3716, 4092}, {899, 3843, 3942}, {1710, 3869, 3970}, {2737, 3933, 4036},
        {3816, 3980, 4087}, {801, 4068, 4019}, {1052, 4068, 4032}, {2270, 4067, 4056},
        {3368, 4066, 4013}, {301, 0, 4095}, {1093, 0, 4095}, {2129, 0, 4095}, {3140, 0, 4095},
        {4095, 0, 4095}, {836, 0, 4095}, {1870, 0, 4095}, {2891, 0, 4095}, {3876, 0, 4095},
        {575, 0, 4095}, {1609, 0, 4095}, {2640, 0, 4095}, {3634, 341, 4095}, {286, 0, 4095},
        {1342, 287, 4095}, {2388, 520, 4095}, {3391, 650, 4095}, {0, 752, 4095}, {1066, 782, 4095},
        {2132, 860, 4095}, {3149, 930, 4095}, {4095, 987, 4095}, {768, 1134, 4095},
        {1870, 1168, 4095}, {2904, 1205, 4095}, {3886, 1238, 4095}, {399, 1458, 4095},
        {1597, 1467, 4095}, {2656, 1482, 4095}, {3650, 1496, 4095}, {280, 1765, 4095},
        {1305, 1764, 4095}, {2400, 1762, 4095}, {3413, 1760, 4095}, {345, 2058, 4095},
        {984, 2058, 4095}, {2134, 2047, 4095}, {3172, 2030, 4095}, {4095, 2022, 4095},
        {625, 2342, 4095}, {1849, 2337, 4095}, {2926, 2308, 4095}, {3909, 2283, 4095},
        {462, 2614, 4095}, {1542, 2619, 4095}, {2667, 2592, 4095}, {3676, 2550, 4095},
        {518, 2878, 4095}, {1237, 2882, 4095}, {2387, 2884, 4095}, {3439, 2827, 4095},
        {574, 3131, 4095}, {944, 3135, 4095}, {2087, 3146, 4095}, {3190, 3114, 4095},
        {4095, 3083, 4095}, {685, 3378, 4095}, {1808, 3387, 4095}, {2922, 3415, 4095},
        {4016, 3396, 4095}, {683, 3612, 4095}, {1571, 3627, 4095}, {2659, 3681, 4095},
        {3798, 3713, 4095}, {745, 3866, 4095}, {1401, 3884, 4095}, {2432, 3935, 4095},
        {3514, 3981, 4095}, {820, 4068, 4095}, {1290, 4068, 4095}, {2227, 4067, 4095},
        {3247, 4066, 4095}, {4095, 4065, 4095}},
    // lut1
    {{576, 239, 213}, {1338, 0, 43}, {2465, 0, 0}, {3739, 0, 0}, {88, 433, 33}, {1104, 98, 0},
        {2155, 0, 0}, {3426, 0, 0}, {4095, 0, 0}, {818, 492, 0}, {1868, 262, 0}, {3120, 0, 0},
        {4095, 0, 0}, {393, 737, 0}, {1601, 668, 0}, {2827, 527, 0}, {4076, 0, 0}, {0, 1003, 0},
        {1325, 987, 0}, {2545, 930, 0}, {3782, 765, 0}, {0, 1331, 0}, {990, 1309, 0},
        {2262, 1266, 0}, {3492, 1176, 0}, {4095, 982, 0}, {267, 1659, 0}, {1957, 1592, 0},
        {3201, 1526, 0}, {4095, 1402, 0}, {0, 2013, 0}, {1617, 1931, 0}, {2898, 1859, 0},
        {4095, 1764, 0}, {0, 2357, 0}, {1125, 2294, 0}, {2571, 2185, 0}, {3817, 2103, 0},
        {0, 2688, 0}, {0, 2651, 0}, {2222, 2525, 0}, {3497, 2430, 0}, {4095, 2320, 0}, {0, 2993, 0},
        {1772, 2886, 0}, {3152, 2751, 0}, {4095, 2649, 0}, {0, 3318, 0}, {1070, 3234, 0},
        {2794, 3083, 0}, {4049, 2968, 0}, {0, 3621, 0}, {0, 3565, 0}, {2360, 3428, 0},
        {3691, 3278, 0}, {0, 3906, 0}, {0, 3873, 0}, {1773, 3759, 0}, {3322, 3593, 0},
        {4095, 3458, 0}, {0, 4095, 0}, {739, 4067, 0}, {2891, 3913, 0}, {4095, 3751, 0},
        {0, 4095, 0}, {0, 4095, 0}, {2350, 4095, 0}, {3788, 4040, 0}, {0, 4095, 0}, {0, 4095, 0},
        {1613, 4095, 0}, {3351, 4095, 0}, {247, 303, 606}, {1134, 0, 244}, {2158, 0, 70},
        {3435, 0, 111}, {4095, 0, 0}, {901, 261, 281}, {1856, 0, 60}, {3120, 0, 101}, {4095, 0, 0},
        {547, 542, 194}, {1566, 343, 0}, {2800, 0, 0}, {4062, 0, 0}, {152, 746, 57}, {1306, 687, 0},
        {2512, 588, 0}, {3762, 52, 0}, {203, 1001, 76}, {1017, 997, 0}, {2236, 954, 0},
        {3470, 827, 0}, {4095, 474, 0}, {553, 1322, 0}, {1952, 1281, 0}, {3183, 1209, 0},
        {4095, 1048, 0}, {0, 1673, 0}, {1635, 1604, 0}, {2892, 1548, 0}, {4095, 1443, 0},
        {0, 2018, 0}, {1240, 1960, 0}, {2584, 1877, 0}, {3818, 1794, 0}, {0, 2356, 0},
        {426, 2321, 0}, {2244, 2201, 0}, {3511, 2127, 0}, {4095, 2014, 0}, {0, 2673, 0},
        {1866, 2563, 0}, {3184, 2453, 0}, {4095, 2352, 0}, {0, 3009, 0}, {1298, 2921, 0},
        {2828, 2771, 0}, {4083, 2678, 0}, {0, 3322, 0}, {0, 3265, 0}, {2452, 3125, 0},
        {3741, 2995, 0}, {0, 3621, 0}, {0, 3589, 0}, {1942, 3469, 0}, {3373, 3304, 0},
        {4095, 3188, 0}, {0, 3891, 0}, {1170, 3795, 0}, {2992, 3637, 0}, {4095, 3491, 0},
        {0, 4095, 0}, {0, 4095, 0}, {2506, 3956, 0}, {3865, 3783, 0}, {0, 4095, 0}, {0, 4095, 0},
        {1868, 4095, 0}, {3472, 4084, 0}, {0, 4095, 0}, {0, 4095, 0}, {752, 4095, 0},
        {2994, 4095, 0}, {4095, 4095, 0}, {936, 148, 638}, {1844, 0, 447}, {3092, 0, 378},
        {4095, 0, 134}, {683, 380, 686}, {1556, 0, 455}, {2779, 0, 398}, {4052, 0, 232},
        {358, 606, 622}, {1294, 417, 452}, {2482, 0, 401}, {3749, 0, 268}, {154, 763, 442},
        {1012, 705, 399}, {2191, 645, 373}, {3438, 401, 245}, {4095, 0, 0}, {652, 1003, 254},
        {1924, 976, 321}, {3151, 887, 198}, {4095, 635, 0}, {269, 1326, 101}, {1639, 1295, 186},
        {2869, 1240, 0}, {4094, 1113, 0}, {338, 1664, 127}, {1300, 1629, 0}, {2579, 1569, 0},
        {3804, 1483, 0}, {407, 2002, 153}, {728, 1993, 0}, {2264, 1894, 0}, {3509, 1823, 0},
        {4095, 1703, 0}, {0, 2352, 0}, {1919, 2236, 0}, {3199, 2151, 0}, {4095, 2051, 0},
        {0, 2685, 0}, {1447, 2603, 0}, {2863, 2474, 0}, {4095, 2383, 0}, {0, 3007, 0},
        {512, 2959, 0}, {2509, 2812, 0}, {3776, 2706, 0}, {0, 3319, 0}, {0, 3297, 0},
        {2067, 3168, 0}, {3425, 3021, 0}, {4095, 2909, 0}, {0, 3614, 0}, {1428, 3510, 0},
        {3061, 3346, 0}, {4095, 3221, 0}, {0, 3902, 0}, {0, 3832, 0}, {2629, 3682, 0},
        {3938, 3523, 0}, {0, 4095, 0}, {0, 4095, 0}, {2065, 3999, 0}, {3562, 3827, 0}, {0, 4095, 0},
        {0, 4095, 0}, {1222, 4095, 0}, {3129, 4095, 0}, {4095, 3959, 0}, {0, 4095, 0}, {0, 4095, 0},
        {2596, 4095, 0}, {3986, 4095, 0}, {673, 211, 993}, {1552, 0, 755}, {2744, 0, 731},
        {4010, 0, 677}, {361, 362, 1012}, {1313, 163, 783}, {2437, 0, 736}, {3697, 0, 694},
        {102, 573, 890}, {1054, 501, 792}, {2148, 316, 736}, {3395, 0, 701}, {4095, 0, 612},
        {759, 739, 758}, {1880, 685, 729}, {3110, 526, 701}, {4095, 0, 622}, {230, 1010, 696},
        {1611, 994, 690}, {2825, 940, 673}, {4084, 734, 620}, {267, 1324, 640}, {1317, 1307, 639},
        {2551, 1269, 637}, {3775, 1173, 577}, {335, 1657, 550}, {863, 1668, 469}, {2263, 1588, 570},
        {3489, 1521, 524}, {4095, 1387, 378}, {407, 2002, 310}, {1935, 1910, 467},
        {3196, 1850, 425}, {4095, 1749, 264}, {473, 2333, 179}, {1543, 2281, 0}, {2882, 2173, 217},
        {4095, 2086, 0}, {540, 2660, 204}, {857, 2650, 0}, {2534, 2494, 0}, {3796, 2413, 0},
        {605, 2981, 228}, {0, 3001, 0}, {2156, 2858, 0}, {3463, 2733, 0}, {4095, 2624, 0},
        {0, 3312, 0}, {1607, 3216, 0}, {3101, 3046, 0}, {4095, 2943, 0}, {729, 3589, 274},
        {531, 3554, 0}, {2721, 3393, 0}, {3995, 3253, 0}, {787, 3873, 296}, {0, 3869, 0},
        {2219, 3729, 0}, {3622, 3553, 0}, {0, 4095, 0}, {0, 4095, 0}, {1510, 4044, 0},
        {3234, 3873, 0}, {4095, 3717, 0}, {0, 4095, 0}, {0, 4095, 0}, {2750, 4095, 0},
        {4079, 3997, 0}, {0, 4095, 0}, {0, 4095, 0}, {2134, 4095, 0}, {3674, 4095, 0},
        {340, 158, 1311}, {1310, 0, 1059}, {2408, 0, 1031}, {3647, 0, 1013}, {116, 315, 1280},
        {1079, 281, 1090}, {2114, 0, 1032}, {3337, 0, 1020}, {4095, 0, 972}, {803, 518, 1090},
        {1836, 423, 1033}, {3038, 0, 1023}, {4095, 0, 984}, {490, 748, 1055}, {1569, 728, 1033},
        {2757, 627, 1023}, {4001, 219, 989}, {314, 1016, 1042}, {1315, 1000, 1026},
        {2497, 966, 1020}, {3726, 835, 989}, {261, 1305, 1018}, {979, 1324, 977}, {2231, 1292, 987},
        {3469, 1208, 980}, {4095, 1039, 913}, {370, 1650, 947}, {1944, 1604, 940},
        {3170, 1554, 930}, {4095, 1449, 873}, {401, 1981, 886}, {1601, 1954, 843},
        {2880, 1873, 876}, {4092, 1792, 832}, {469, 2315, 790}, {1034, 2340, 676},
        {2559, 2193, 799}, {3796, 2119, 765}, {536, 2645, 640}, {539, 2655, 557}, {2209, 2539, 639},
        {3483, 2440, 667}, {4095, 2337, 583}, {603, 2972, 282}, {1737, 2914, 163},
        {3141, 2757, 524}, {4095, 2659, 430}, {666, 3281, 251}, {937, 3270, 0}, {2782, 3091, 0},
        {4037, 2975, 0}, {727, 3580, 274}, {0, 3599, 0}, {2339, 3445, 0}, {3681, 3282, 0},
        {785, 3866, 296}, {0, 3885, 0}, {1719, 3780, 0}, {3310, 3598, 0}, {4095, 3462, 0},
        {827, 4069, 311}, {466, 4090, 0}, {2875, 3923, 0}, {4095, 3754, 0}, {827, 4069, 311},
        {0, 4095, 0}, {2317, 4095, 0}, {3777, 4042, 0}, {827, 4069, 311}, {0, 4095, 0},
        {1542, 4095, 0}, {3336, 4095, 0}, {98, 0, 1555}, {1066, 0, 1353}, {2096, 0, 1309},
        {3290, 0, 1314}, {4095, 0, 1287}, {817, 232, 1395}, {1818, 45, 1307}, {2986, 0, 1315},
        {4095, 0, 1297}, {525, 493, 1386}, {1550, 483, 1305}, {2694, 289, 1315}, {3926, 0, 1303},
        {162, 739, 1334}, {1291, 757, 1304}, {2415, 691, 1315}, {3636, 486, 1306},
        {187, 1010, 1315}, {1032, 1008, 1319}, {2149, 993, 1316}, {3361, 908, 1308},
        {4095, 678, 1268}, {799, 1270, 1304}, {1910, 1287, 1322}, {3107, 1243, 1308},
        {4095, 1119, 1268}, {318, 1596, 1315}, {1618, 1615, 1276}, {2847, 1582, 1278},
        {4074, 1479, 1262}, {389, 1938, 1283}, {1228, 1985, 1241}, {2561, 1892, 1234},
        {3774, 1829, 1212}, {460, 2282, 1222}, {517, 2304, 1189}, {2226, 2211, 1175},
        {3482, 2147, 1163}, {4095, 2050, 1112}, {532, 2628, 1114}, {1832, 2605, 1036},
        {3165, 2464, 1095}, {4095, 2374, 1056}, {597, 2949, 1007}, {1154, 2983, 862},
        {2811, 2780, 1013}, {4061, 2691, 979}, {661, 3262, 858}, {664, 3269, 760},
        {2429, 3148, 792}, {3724, 3004, 878}, {723, 3565, 632}, {724, 3566, 546}, {1879, 3506, 425},
        {3357, 3310, 748}, {4095, 3197, 676}, {782, 3851, 295}, {966, 3837, 0}, {2972, 3650, 210},
        {4095, 3497, 473}, {826, 4069, 311}, {0, 4095, 0}, {2466, 3976, 0}, {3850, 3787, 0},
        {826, 4069, 311}, {0, 4095, 0}, {1781, 4095, 0}, {3453, 4091, 0}, {826, 4069, 311},
        {827, 4069, 311}, {198, 4095, 0}, {2962, 4095, 0}, {4095, 4095, 0}, {802, 0, 1661},
        {1808, 0, 1569}, {2950, 0, 1593}, {4095, 0, 1591}, {539, 92, 1697}, {1544, 233, 1563},
        {2656, 0, 1589}, {3866, 0, 1595}, {225, 454, 1668}, {1289, 482, 1574}, {2371, 409, 1584},
        {3567, 0, 1596}, {117, 721, 1611}, {1030, 737, 1590}, {2094, 731, 1580}, {3278, 608, 1597},
        {4095, 0, 1580}, {750, 1002, 1612}, {1822, 1009, 1573}, {3001, 957, 1599},
        {4095, 802, 1585}, {497, 1277, 1591}, {1560, 1270, 1570}, {2740, 1262, 1602},
        {3957, 1179, 1588}, {380, 1525, 1564}, {1309, 1525, 1566}, {2508, 1568, 1613},
        {3707, 1512, 1589}, {371, 1867, 1604}, {904, 1896, 1609}, {2239, 1905, 1575},
        {3447, 1859, 1558}, {4095, 1742, 1533}, {450, 2240, 1584}, {1901, 2286, 1515},
        {3164, 2170, 1519}, {4095, 2092, 1482}, {520, 2579, 1531}, {1435, 2626, 1475},
        {2841, 2484, 1460}, {4063, 2407, 1436}, {588, 2911, 1454}, {636, 2935, 1407},
        {2489, 2835, 1365}, {3749, 2719, 1373}, {654, 3233, 1351}, {657, 3242, 1317},
        {2005, 3221, 1208}, {3404, 3030, 1297}, {4095, 2924, 1256}, {718, 3543, 1199},
        {1238, 3570, 1032}, {3039, 3359, 1166}, {4095, 3230, 1169}, {777, 3833, 1045},
        {778, 3830, 933}, {2589, 3711, 927}, {3916, 3529, 1065}, {825, 4069, 832}, {826, 4069, 732},
        {1968, 4036, 586}, {3538, 3833, 870}, {825, 4069, 472}, {826, 4069, 343}, {937, 4095, 0},
        {3095, 4095, 393}, {4095, 3962, 695}, {826, 4069, 311}, {0, 4095, 0}, {2532, 4095, 0},
        {3962, 4095, 131}, {531, 0, 1959}, {1547, 0, 1835}, {2630, 0, 1854}, {3818, 0, 1875},
        {261, 0, 1973}, {1289, 95, 1849}, {2347, 0, 1844}, {3517, 0, 1873}, {0, 400, 1932},
        {1029, 439, 1867}, {2071, 475, 1833}, {3223, 222, 1869}, {4095, 0, 1872}, {760, 716, 1888},
        {1806, 754, 1827}, {2936, 678, 1866}, {4095, 432, 1875}, {459, 996, 1900},
        {1561, 1008, 1848}, {2657, 987, 1861}, {3861, 885, 1877}, {230, 1277, 1883},
        {1309, 1277, 1883}, {2382, 1268, 1857}, {3588, 1221, 1880}, {290, 1540, 1857},
        {1038, 1548, 1876}, {2111, 1537, 1851}, {3330, 1529, 1885}, {4095, 1444, 1859},
        {829, 1779, 1826}, {1892, 1843, 1890}, {3101, 1841, 1896}, {4095, 1774, 1861},
        {425, 2141, 1886}, {1553, 2217, 1913}, {2842, 2188, 1864}, {4062, 2102, 1859},
        {501, 2503, 1883}, {1033, 2547, 1877}, {2508, 2501, 1808}, {3748, 2435, 1792},
        {574, 2854, 1838}, {579, 2875, 1824}, {2108, 2931, 1720}, {3432, 2743, 1734},
        {4095, 2651, 1696}, {645, 3196, 1752}, {1601, 3230, 1676}, {3074, 3053, 1666},
        {4095, 2957, 1635}, {708, 3507, 1658}, {720, 3522, 1594}, {2687, 3428, 1524},
        {3967, 3260, 1562}, {770, 3805, 1541}, {771, 3808, 1491}, {2121, 3783, 1352},
        {3595, 3558, 1478}, {824, 4069, 1397}, {824, 4069, 1362}, {1280, 4092, 1175},
        {3202, 3889, 1283}, {4095, 3722, 1334}, {825, 4069, 1196}, {827, 4069, 1070},
        {2685, 4095, 1035}, {4052, 3999, 1225}, {825, 4069, 980}, {826, 4069, 864},
        {2001, 4095, 700}, {3640, 4095, 947}, {270, 0, 2232}, {1292, 0, 2131}, {2330, 0, 2097},
        {3479, 0, 2141}, {0, 0, 2227}, {1031, 0, 2151}, {2059, 217, 2084}, {3185, 0, 2133},
        {4095, 0, 2151}, {767, 378, 2172}, {1810, 473, 2101}, {2897, 379, 2124}, {4084, 0, 2151},
        {482, 691, 2189}, {1560, 732, 2124}, {2615, 722, 2112}, {3792, 574, 2149}, {161, 987, 2191},
        {1305, 997, 2154}, {2335, 1005, 2097}, {3507, 940, 2146}, {223, 1274, 2179},
        {1032, 1275, 2190}, {2074, 1267, 2093}, {3228, 1245, 2144}, {4095, 1153, 2151},
        {750, 1550, 2170}, {1840, 1536, 2130}, {2955, 1531, 2143}, {4095, 1481, 2156},
        {522, 1803, 2127}, {1589, 1829, 2177}, {2694, 1813, 2147}, {3914, 1792, 2161},
        {459, 2034, 2086}, {1329, 2034, 2088}, {2502, 2144, 2197}, {3683, 2106, 2171},
        {475, 2400, 2159}, {841, 2419, 2168}, {2243, 2575, 2232}, {3427, 2457, 2139},
        {4095, 2350, 2118}, {556, 2782, 2170}, {1756, 2860, 2164}, {3110, 2762, 2083},
        {4095, 2683, 2051}, {628, 3127, 2129}, {1149, 3168, 2102}, {2760, 3124, 2006},
        {3996, 2985, 1993}, {696, 3455, 2058}, {701, 3473, 2029}, {2249, 3522, 1887},
        {3650, 3287, 1924}, {760, 3764, 1962}, {761, 3770, 1939}, {1723, 3781, 1843},
        {3279, 3611, 1820}, {4095, 3468, 1807}, {819, 4054, 1830}, {821, 4051, 1748},
        {2815, 3957, 1650}, {4095, 3756, 1724}, {823, 4069, 1697}, {825, 4069, 1629},
        {2179, 4095, 1461}, {3742, 4045, 1593}, {824, 4069, 1537}, {825, 4069, 1484},
        {1272, 4095, 1283}, {3284, 4095, 1366}, {110, 0, 2493}, {1036, 0, 2441}, {2067, 0, 2361},
        {3156, 0, 2390}, {4095, 0, 2422}, {775, 0, 2462}, {1815, 0, 2380}, {2871, 0, 2376},
        {4038, 0, 2418}, {503, 288, 2479}, {1560, 419, 2404}, {2590, 461, 2359}, {3744, 0, 2412},
        {141, 662, 2491}, {1303, 703, 2433}, {2321, 749, 2348}, {3455, 655, 2404}, {150, 974, 2488},
        {1036, 982, 2465}, {2080, 1002, 2372}, {3169, 975, 2394}, {4095, 857, 2425},
        {742, 1268, 2493}, {1837, 1266, 2406}, {2886, 1258, 2380}, {4078, 1198, 2424},
        {429, 1548, 2464}, {1585, 1544, 2452}, {2603, 1523, 2361}, {3801, 1503, 2424},
        {333, 1814, 2430}, {1307, 1827, 2466}, {2343, 1782, 2360}, {3532, 1794, 2426},
        {391, 2060, 2390}, {1050, 2068, 2402}, {2133, 2085, 2434}, {3279, 2084, 2434},
        {4095, 2047, 2427}, {868, 2288, 2347}, {1839, 2289, 2350}, {3080, 2411, 2473},
        {4095, 2359, 2434}, {527, 2663, 2431}, {1476, 2705, 2453}, {2779, 2778, 2429},
        {3991, 2711, 2398}, {605, 3032, 2448}, {825, 3063, 2450}, {2443, 3171, 2436},
        {3682, 3008, 2343}, {678, 3381, 2414}, {682, 3397, 2407}, {1915, 3451, 2363},
        {3323, 3310, 2275}, {4095, 3207, 2236}, {747, 3713, 2336}, {1237, 3737, 2286},
        {2926, 3693, 2156}, {4095, 3504, 2169}, {808, 4010, 2243}, {812, 4017, 2196},
        {2336, 4034, 2018}, {3815, 3789, 2088}, {821, 4069, 2125}, {822, 4069, 2087},
        {1784, 4068, 1969}, {3406, 4095, 1924}, {822, 4069, 1986}, {823, 4069, 1959},
        {825, 4069, 1857}, {2869, 4095, 1733}, {4095, 4095, 1848}, {783, 0, 2758}, {1822, 0, 2665},
        {2849, 0, 2623}, {4000, 0, 2679}, {523, 0, 2775}, {1564, 0, 2690}, {2574, 192, 2605},
        {3707, 0, 2669}, {230, 118, 2785}, {1305, 332, 2719}, {2329, 460, 2626}, {3419, 334, 2656},
        {0, 630, 2790}, {1041, 664, 2748}, {2083, 722, 2652}, {3134, 708, 2640}, {4095, 529, 2688},
        {760, 963, 2776}, {1835, 988, 2685}, {2850, 999, 2620}, {4015, 916, 2682},
        {430, 1258, 2785}, {1581, 1260, 2724}, {2587, 1263, 2615}, {3730, 1228, 2675},
        {268, 1544, 2762}, {1311, 1544, 2768}, {2353, 1525, 2646}, {3446, 1514, 2664},
        {327, 1818, 2734}, {1019, 1824, 2758}, {2118, 1802, 2694}, {3163, 1785, 2651},
        {4095, 1759, 2698}, {749, 2081, 2707}, {1868, 2105, 2764}, {2879, 2042, 2632},
        {4095, 2052, 2701}, {559, 2319, 2655}, {1582, 2336, 2683}, {2612, 2297, 2625},
        {3854, 2347, 2710}, {546, 2542, 2608}, {1355, 2543, 2609}, {2353, 2544, 2612},
        {3645, 2667, 2737}, {576, 2916, 2695}, {784, 2927, 2702}, {2075, 3001, 2742},
        {3363, 3026, 2691}, {4095, 2917, 2680}, {656, 3293, 2720}, {1588, 3343, 2723},
        {3022, 3409, 2633}, {4095, 3234, 2586}, {728, 3633, 2687}, {791, 3661, 2670},
        {2592, 3712, 2597}, {3886, 3534, 2521}, {793, 3949, 2617}, {797, 3960, 2595},
        {2028, 3977, 2516}, {3512, 3855, 2423}, {819, 4069, 2517}, {820, 4069, 2499},
        {1266, 4068, 2423}, {3013, 4095, 2260}, {4095, 3963, 2305}, {821, 4069, 2381},
        {823, 4069, 2313}, {2369, 4095, 2108}, {3914, 4095, 2175}, {536, 0, 3074}, {1571, 0, 2984},
        {2585, 0, 2885}, {3677, 0, 2921}, {285, 0, 3084}, {1311, 0, 3012}, {2337, 0, 2907},
        {3391, 0, 2904}, {0, 0, 3087}, {1047, 148, 3040}, {2087, 392, 2935}, {3108, 442, 2884},
        {4095, 0, 2946}, {775, 615, 3066}, {1836, 686, 2968}, {2835, 743, 2869}, {3971, 623, 2935},
        {467, 938, 3087}, {1580, 967, 3005}, {2596, 995, 2895}, {3684, 959, 2921},
        {194, 1245, 3081}, {1314, 1249, 3044}, {2356, 1256, 2928}, {3397, 1249, 2904},
        {259, 1537, 3063}, {1024, 1538, 3081}, {2113, 1527, 2970}, {3112, 1518, 2880},
        {4095, 1482, 2952}, {715, 1819, 3052}, {1860, 1810, 3020}, {2855, 1777, 2881},
        {4009, 1770, 2945}, {392, 2086, 3010}, {1581, 2101, 3058}, {2629, 2049, 2923},
        {3729, 2046, 2937}, {435, 2338, 2964}, {1300, 2350, 2990}, {2403, 2348, 2993},
        {3452, 2314, 2928}, {492, 2574, 2916}, {1068, 2579, 2924}, {2121, 2611, 2972},
        {3178, 2577, 2919}, {4095, 2599, 2971}, {918, 2797, 2869}, {1859, 2798, 2871},
        {2994, 2918, 2991}, {4095, 2908, 2986}, {626, 3171, 2959}, {1430, 3193, 2972},
        {2684, 3310, 3033}, {3909, 3257, 2933}, {703, 3534, 2984}, {708, 3552, 2985},
        {2232, 3616, 2982}, {3559, 3553, 2865}, {774, 3871, 2953}, {776, 3880, 2947},
        {1669, 3910, 2915}, {3149, 3952, 2764}, {4095, 3748, 2757}, {817, 4068, 2873},
        {819, 4068, 2834}, {2688, 4067, 2719}, {4032, 4015, 2668}, {818, 4068, 2769},
        {820, 4068, 2728}, {2066, 4068, 2616}, {3581, 4095, 2488}, {310, 0, 3387}, {1318, 0, 3312},
        {2347, 0, 3195}, {3366, 0, 3147}, {163, 0, 3389}, {1057, 0, 3339}, {2094, 0, 3224},
        {3088, 156, 3126}, {4095, 0, 3199}, {789, 0, 3364}, {1839, 272, 3257}, {2846, 443, 3148},
        {3937, 267, 3184}, {503, 554, 3382}, {1582, 635, 3293}, {2603, 709, 3176},
        {3651, 690, 3166}, {84, 911, 3393}, {1318, 938, 3330}, {2359, 975, 3210}, {3366, 991, 3143},
        {181, 1229, 3382}, {1038, 1233, 3364}, {2112, 1246, 3251}, {3100, 1258, 3135},
        {4095, 1211, 3201}, {724, 1527, 3381}, {1857, 1523, 3296}, {2866, 1517, 3166},
        {3950, 1500, 3186}, {349, 1813, 3349}, {1586, 1810, 3342}, {2632, 1784, 3206},
        {3663, 1772, 3167}, {371, 2086, 3314}, {1291, 2095, 3347}, {2392, 2065, 3258},
        {3375, 2031, 3142}, {429, 2348, 3274}, {1004, 2355, 3291}, {2137, 2364, 3323},
        {3124, 2291, 3147}, {4095, 2303, 3215}, {754, 2600, 3234}, {1845, 2621, 3277},
        {2908, 2578, 3204}, {4021, 2578, 3212}, {608, 2830, 3178}, {1587, 2841, 3194},
        {2675, 2894, 3277}, {3763, 2857, 3217}, {637, 3051, 3129}, {1388, 3051, 3131},
        {2369, 3053, 3133}, {3586, 3200, 3280}, {675, 3419, 3218}, {731, 3426, 3221},
        {2007, 3459, 3240}, {3322, 3620, 3307}, {4095, 3505, 3188}, {751, 3780, 3242},
        {1448, 3806, 3241}, {2855, 3890, 3232}, {4095, 3791, 3120}, {813, 4068, 3210},
        {814, 4068, 3200}, {2344, 4067, 3152}, {3726, 4082, 2997}, {815, 4068, 3128},
        {816, 4068, 3111}, {1687, 4068, 3048}, {3170, 4095, 2814}, {210, 0, 3695}, {1066, 0, 3643},
        {2102, 0, 3518}, {3102, 0, 3407}, {4095, 0, 3448}, {805, 0, 3666}, {1846, 0, 3552},
        {2857, 0, 3432}, {3908, 0, 3429}, {535, 0, 3684}, {1586, 0, 3587}, {2610, 354, 3461},
        {3624, 417, 3407}, {210, 481, 3694}, {1323, 564, 3623}, {2362, 664, 3496},
        {3349, 736, 3389}, {0, 881, 3695}, {1050, 901, 3655}, {2112, 947, 3536}, {3111, 987, 3416},
        {4095, 941, 3447}, {751, 1211, 3681}, {1856, 1229, 3579}, {2873, 1246, 3449},
        {3911, 1238, 3426}, {383, 1514, 3676}, {1590, 1514, 3622}, {2633, 1512, 3488},
        {3624, 1512, 3401}, {304, 1804, 3649}, {1302, 1804, 3662}, {2388, 1787, 3535},
        {3368, 1772, 3402}, {364, 2084, 3619}, {993, 2087, 3640}, {2132, 2072, 3587},
        {3138, 2036, 3437}, {4095, 2027, 3450}, {679, 2355, 3592}, {1854, 2368, 3642},
        {2907, 2312, 3485}, {3932, 2292, 3432}, {479, 2611, 3541}, {1560, 2624, 3576},
        {2670, 2607, 3549}, {3644, 2545, 3406}, {535, 2853, 3489}, {1298, 2862, 3506},
        {2393, 2893, 3569}, {3391, 2804, 3411}, {591, 3084, 3437}, {1093, 3087, 3442},
        {2111, 3103, 3467}, {3196, 3125, 3506}, {4095, 3139, 3513}, {975, 3305, 3391},
        {1883, 3306, 3392}, {2881, 3307, 3395}, {4095, 3480, 3567}, {723, 3668, 3475},
        {1393, 3679, 3480}, {2588, 3754, 3532}, {3828, 3827, 3481}, {798, 4023, 3504},
        {803, 4040, 3509}, {2111, 4067, 3518}, {3441, 4066, 3428}, {811, 4068, 3464},
        {812, 4068, 3461}, {1412, 4068, 3437}, {2921, 4067, 3324}, {4095, 4095, 3172},
        {816, 0, 3972}, {1854, 0, 3852}, {2868, 0, 3721}, {3883, 0, 3670}, {562, 0, 3989},
        {1594, 0, 3888}, {2619, 0, 3751}, {3603, 88, 3647}, {298, 0, 3999}, {1331, 0, 3923},
        {2368, 170, 3787}, {3362, 421, 3670}, {0, 388, 4001}, {1062, 465, 3953}, {2115, 599, 3826},
        {3120, 693, 3699}, {4095, 668, 3690}, {777, 856, 3978}, {1858, 909, 3869},
        {2878, 960, 3733}, {3881, 982, 3666}, {440, 1185, 3995}, {1594, 1205, 3910},
        {2635, 1230, 3773}, {3613, 1253, 3655}, {225, 1498, 3975}, {1314, 1499, 3947},
        {2387, 1504, 3818}, {3379, 1509, 3685}, {294, 1794, 3952}, {1006, 1794, 3976},
        {2131, 1784, 3865}, {3144, 1772, 3722}, {4095, 1763, 3686}, {671, 2079, 3935},
        {1859, 2072, 3913}, {2907, 2043, 3766}, {3883, 2026, 3659}, {415, 2353, 3893},
        {1562, 2360, 3934}, {2663, 2325, 3819}, {3635, 2285, 3668}, {473, 2615, 3848},
        {1266, 2623, 3873}, {2402, 2619, 3878}, {3409, 2555, 3707}, {529, 2867, 3799},
        {990, 2872, 3811}, {2109, 2892, 3861}, {3181, 2842, 3764}, {4095, 2809, 3697},
        {767, 3110, 3752}, {1832, 3124, 3779}, {2940, 3155, 3847}, {3945, 3082, 3700},
        {666, 3337, 3697}, {1601, 3343, 3706}, {2636, 3365, 3741}, {3743, 3396, 3760},
        {730, 3559, 3651}, {1427, 3560, 3652}, {2416, 3598, 3693}, {3535, 3706, 3805},
        {775, 3931, 3746}, {777, 3942, 3755}, {2002, 3986, 3791}, {3172, 4055, 3825},
        {4095, 3977, 3651}, {807, 4068, 3765}, {1315, 4068, 3771}, {2677, 4067, 3735},
        {3852, 4095, 3485}, {579, 0, 4095}, {1602, 0, 4095}, {2629, 0, 4046}, {3618, 0, 3929},
        {355, 0, 4095}, {1341, 0, 4095}, {2376, 0, 4082}, {3374, 0, 3955}, {198, 0, 4095},
        {1076, 0, 4095}, {2120, 0, 4095}, {3130, 304, 3985}, {4095, 386, 3930}, {800, 319, 4095},
        {1862, 504, 4095}, {2885, 637, 4020}, {3863, 727, 3909}, {497, 806, 4095},
        {1599, 860, 4095}, {2638, 925, 4060}, {3625, 978, 3936}, {119, 1160, 4095},
        {1325, 1175, 4095}, {2388, 1207, 4095}, {3388, 1235, 3968}, {213, 1480, 4095},
        {1030, 1480, 4095}, {2131, 1490, 4095}, {3149, 1499, 4006}, {4095, 1506, 3922},
        {696, 1780, 4095}, {1863, 1776, 4095}, {2908, 1768, 4050}, {3880, 1767, 3922},
        {348, 2069, 4095}, {1576, 2066, 4095}, {2660, 2045, 4095}, {3648, 2027, 3954},
        {408, 2347, 4095}, {1267, 2351, 4095}, {2401, 2330, 4095}, {3415, 2294, 3994},
        {466, 2616, 4095}, {954, 2620, 4095}, {2121, 2623, 4095}, {3179, 2571, 4042},
        {4095, 2537, 3918}, {638, 2877, 4095}, {1822, 2889, 4095}, {2931, 2861, 4095},
        {3901, 2797, 3932}, {578, 3124, 4060}, {1542, 3133, 4083}, {2656, 3155, 4095},
        {3683, 3078, 3981}, {634, 3360, 4005}, {1301, 3366, 4015}, {2363, 3383, 4051},
        {3494, 3415, 4095}, {690, 3589, 3954}, {1125, 3591, 3957}, {2131, 3624, 3997},
        {3244, 3716, 4095}, {4095, 3707, 4084}, {1058, 3847, 3946}, {1956, 3882, 3983},
        {3008, 3951, 4055}, {4083, 3979, 4079}, {801, 4068, 4020}, {1381, 4068, 4038},
        {2553, 4067, 4057}, {3630, 4066, 3967}, {388, 0, 4095}, {1351, 0, 4095}, {2385, 0, 4095},
        {3387, 0, 4095}, {289, 0, 4095}, {1091, 0, 4095}, {2129, 0, 4095}, {3140, 0, 4095},
        {4095, 0, 4095}, {827, 0, 4095}, {1870, 0, 4095}, {2892, 0, 4095}, {3877, 395, 4095},
        {545, 0, 4095}, {1607, 360, 4095}, {2643, 559, 4095}, {3636, 675, 4095}, {193, 751, 4095},
        {1337, 800, 4095}, {2391, 879, 4095}, {3395, 944, 4095}, {108, 1131, 4095},
        {1053, 1141, 4095}, {2134, 1177, 4095}, {3154, 1213, 4095}, {4095, 1248, 4095},
        {736, 1457, 4095}, {1868, 1471, 4095}, {2909, 1485, 4095}, {3891, 1501, 4095},
        {316, 1765, 4095}, {1589, 1764, 4095}, {2660, 1761, 4095}, {3656, 1761, 4095},
        {345, 2058, 4095}, {1285, 2057, 4095}, {2402, 2043, 4095}, {3419, 2027, 4095},
        {405, 2341, 4095}, {960, 2342, 4095}, {2129, 2330, 4095}, {3179, 2300, 4095},
        {4095, 2280, 4095}, {599, 2615, 4095}, {1832, 2621, 4095}, {2929, 2581, 4095},
        {3915, 2543, 4095}, {518, 2878, 4095}, {1529, 2885, 4095}, {2663, 2870, 4095},
        {3683, 2814, 4095}, {574, 3132, 4095}, {1239, 3137, 4095}, {2369, 3150, 4095},
        {3444, 3095, 4095}, {628, 3376, 4095}, {979, 3379, 4095}, {2081, 3390, 4095},
        {3209, 3414, 4095}, {4095, 3391, 4095}, {789, 3612, 4095}, {1837, 3636, 4095},
        {2942, 3699, 4095}, {4061, 3699, 4095}, {751, 3868, 4095}, {1649, 3894, 4095},
        {2702, 3950, 4095}, {3779, 3976, 4095}, {850, 4068, 4095}, {1505, 4067, 4095},
        {2481, 4067, 4095}, {3495, 4066, 4095}},
    // lut2
    {{774, 154, 132}, {1568, 0, 51}, {2784, 0, 0}, {4055, 0, 0}, {412, 413, 98}, {1318, 0, 0},
        {2471, 0, 0}, {3743, 0, 0}, {0, 575, 0}, {1059, 440, 0}, {2175, 117, 0}, {3436, 0, 0},
        {4095, 0, 0}, {742, 725, 0}, {1900, 647, 0}, {3140, 451, 0}, {4095, 0, 0}, {0, 1001, 0},
        {1629, 978, 0}, {2853, 902, 0}, {4090, 692, 0}, {0, 1331, 0}, {1326, 1298, 0},
        {2570, 1250, 0}, {3799, 1140, 0}, {0, 1675, 0}, {913, 1643, 0}, {2273, 1579, 0},
        {3507, 1502, 0}, {4095, 1358, 0}, {0, 2000, 0}, {1943, 1904, 0}, {3208, 1840, 0},
        {4095, 1731, 0}, {0, 2352, 0}, {1559, 2264, 0}, {2889, 2167, 0}, {4095, 2076, 0},
        {0, 2688, 0}, {932, 2626, 0}, {2541, 2489, 0}, {3804, 2407, 0}, {0, 3011, 0}, {0, 2974, 0},
        {2165, 2848, 0}, {3469, 2729, 0}, {4095, 2618, 0}, {0, 3305, 0}, {1635, 3201, 0},
        {3107, 3043, 0}, {4095, 2939, 0}, {0, 3617, 0}, {676, 3537, 0}, {2729, 3387, 0},
        {4001, 3250, 0}, {0, 3906, 0}, {0, 3852, 0}, {2235, 3720, 0}, {3628, 3551, 0}, {0, 4095, 0},
        {0, 4095, 0}, {1549, 4033, 0}, {3241, 3870, 0}, {4095, 3716, 0}, {0, 4095, 0}, {0, 4095, 0},
        {2762, 4095, 0}, {4085, 3996, 0}, {0, 4095, 0}, {0, 4095, 0}, {2156, 4095, 0},
        {3681, 4095, 0}, {550, 283, 541}, {1337, 0, 178}, {2474, 0, 80}, {3755, 0, 121},
        {276, 465, 476}, {1108, 142, 218}, {2164, 0, 70}, {3440, 0, 111}, {4095, 0, 0},
        {811, 494, 160}, {1864, 280, 0}, {3117, 0, 0}, {4095, 0, 0}, {378, 736, 0}, {1598, 672, 0},
        {2824, 536, 0}, {4073, 0, 0}, {203, 1000, 76}, {1324, 989, 0}, {2542, 933, 0},
        {3779, 771, 0}, {269, 1324, 101}, {987, 1311, 0}, {2261, 1268, 0}, {3490, 1179, 0},
        {4095, 986, 0}, {235, 1662, 0}, {1957, 1593, 0}, {3200, 1528, 0}, {4095, 1405, 0},
        {0, 2016, 0}, {1616, 1932, 0}, {2897, 1860, 0}, {4095, 1765, 0}, {0, 2356, 0},
        {1121, 2296, 0}, {2570, 2185, 0}, {3816, 2103, 0}, {0, 2687, 0}, {0, 2653, 0},
        {2221, 2526, 0}, {3496, 2431, 0}, {4095, 2321, 0}, {0, 2996, 0}, {1771, 2887, 0},
        {3152, 2751, 0}, {4095, 2650, 0}, {0, 3320, 0}, {1064, 3236, 0}, {2793, 3083, 0},
        {4048, 2968, 0}, {0, 3621, 0}, {0, 3567, 0}, {2359, 3429, 0}, {3691, 3278, 0}, {0, 3906, 0},
        {0, 3875, 0}, {1771, 3760, 0}, {3321, 3593, 0}, {4095, 3458, 0}, {0, 4095, 0},
        {729, 4068, 0}, {2891, 3914, 0}, {4095, 3751, 0}, {0, 4095, 0}, {0, 4095, 0},
        {2348, 4095, 0}, {3788, 4040, 0}, {0, 4095, 0}, {0, 4095, 0}, {1610, 4095, 0},
        {3351, 4095, 0}, {205, 295, 882}, {1128, 0, 560}, {2145, 0, 435}, {3411, 0, 346},
        {4095, 0, 134}, {900, 307, 610}, {1848, 0, 443}, {3097, 0, 373}, {4095, 0, 134},
        {568, 554, 568}, {1570, 375, 438}, {2797, 0, 381}, {4065, 0, 192}, {153, 756, 430},
        {1297, 696, 398}, {2501, 611, 355}, {3751, 226, 165}, {204, 1008, 300}, {1013, 1001, 330},
        {2228, 963, 307}, {3461, 845, 95}, {4095, 514, 0}, {517, 1330, 0}, {1949, 1285, 169},
        {3176, 1217, 0}, {4095, 1063, 0}, {338, 1665, 127}, {1632, 1606, 0}, {2888, 1553, 0},
        {4095, 1451, 0}, {407, 2003, 153}, {1232, 1966, 0}, {2582, 1879, 0}, {3813, 1798, 0},
        {475, 2338, 179}, {351, 2331, 0}, {2242, 2202, 0}, {3508, 2130, 0}, {4095, 2019, 0},
        {0, 2683, 0}, {1862, 2567, 0}, {3181, 2454, 0}, {4095, 2355, 0}, {0, 3007, 0},
        {1285, 2928, 0}, {2826, 2773, 0}, {4081, 2680, 0}, {0, 3319, 0}, {0, 3272, 0},
        {2449, 3127, 0}, {3739, 2997, 0}, {0, 3618, 0}, {0, 3596, 0}, {1936, 3473, 0},
        {3371, 3305, 0}, {4095, 3189, 0}, {0, 3898, 0}, {1151, 3800, 0}, {2989, 3638, 0},
        {4095, 3492, 0}, {0, 4095, 0}, {0, 4095, 0}, {2502, 3958, 0}, {3864, 3784, 0}, {0, 4095, 0},
        {0, 4095, 0}, {1859, 4095, 0}, {3469, 4085, 0}, {0, 4095, 0}, {0, 4095, 0}, {717, 4095, 0},
        {2990, 4095, 0}, {4095, 4095, 0}, {908, 194, 912}, {1834, 0, 748}, {3058, 0, 722},
        {4095, 0, 654}, {638, 343, 951}, {1554, 0, 753}, {2748, 0, 729}, {4014, 0, 675},
        {289, 564, 882}, {1291, 455, 758}, {2454, 207, 731}, {3711, 0, 686}, {184, 788, 809},
        {1019, 719, 738}, {2182, 663, 725}, {3423, 439, 687}, {4095, 0, 590}, {657, 1001, 660},
        {1911, 988, 691}, {3133, 910, 661}, {4095, 681, 571}, {268, 1326, 626}, {1634, 1299, 642},
        {2857, 1253, 628}, {4081, 1134, 550}, {336, 1658, 546}, {1292, 1638, 520},
        {2572, 1575, 565}, {3793, 1495, 499}, {404, 1995, 389}, {670, 2010, 221}, {2260, 1897, 461},
        {3501, 1829, 401}, {4095, 1714, 159}, {474, 2335, 178}, {1915, 2242, 0}, {3194, 2155, 172},
        {4095, 2058, 0}, {541, 2661, 203}, {1431, 2614, 0}, {2860, 2476, 0}, {4095, 2387, 0},
        {606, 2981, 228}, {390, 2973, 0}, {2506, 2815, 0}, {3772, 2709, 0}, {669, 3292, 251},
        {0, 3311, 0}, {2059, 3175, 0}, {3422, 3023, 0}, {4095, 2912, 0}, {0, 3611, 0},
        {1404, 3519, 0}, {3057, 3348, 0}, {4095, 3223, 0}, {0, 3897, 0}, {0, 3841, 0},
        {2624, 3686, 0}, {3934, 3524, 0}, {0, 4095, 0}, {0, 4095, 0}, {2052, 4004, 0},
        {3558, 3828, 0}, {0, 4095, 0}, {0, 4095, 0}, {1188, 4095, 0}, {3124, 4095, 0},
        {4095, 3960, 0}, {0, 4095, 0}, {0, 4095, 0}, {2587, 4095, 0}, {3982, 4095, 0},
        {619, 143, 1241}, {1550, 0, 1036}, {2712, 0, 1029}, {3963, 0, 1003}, {322, 310, 1261},
        {1302, 215, 1053}, {2411, 0, 1031}, {3651, 0, 1012}, {88, 538, 1159}, {1049, 514, 1059},
        {2126, 377, 1031}, {3350, 0, 1017}, {4095, 0, 966}, {760, 742, 1043}, {1856, 712, 1031},
        {3065, 576, 1018}, {4095, 0, 974}, {541, 1016, 1042}, {1604, 998, 1026}, {2801, 946, 1015},
        {4035, 777, 975}, {261, 1306, 1017}, {1313, 1310, 978}, {2534, 1282, 984},
        {3774, 1176, 968}, {330, 1637, 969}, {898, 1660, 923}, {2255, 1594, 942}, {3474, 1535, 920},
        {4095, 1409, 851}, {403, 1987, 870}, {1930, 1914, 881}, {3186, 1857, 869},
        {4095, 1763, 812}, {469, 2317, 780}, {1528, 2297, 713}, {2876, 2177, 794},
        {4095, 2094, 749}, {536, 2646, 637}, {775, 2673, 466}, {2529, 2497, 695}, {3788, 2417, 653},
        {602, 2968, 371}, {604, 2974, 227}, {2148, 2868, 334}, {3457, 2735, 507}, {4095, 2630, 388},
        {666, 3282, 251}, {1581, 3230, 0}, {3096, 3048, 225}, {4095, 2946, 0}, {727, 3580, 274},
        {331, 3570, 0}, {2715, 3398, 0}, {3990, 3255, 0}, {785, 3865, 295}, {0, 3881, 0},
        {2205, 3737, 0}, {3617, 3555, 0}, {827, 4069, 311}, {0, 4095, 0}, {1474, 4053, 0},
        {3229, 3876, 0}, {4095, 3719, 0}, {827, 4069, 311}, {0, 4095, 0}, {2740, 4095, 0},
        {4074, 3998, 0}, {827, 4069, 311}, {0, 4095, 0}, {2114, 4095, 0}, {3669, 4095, 0},
        {307, 0, 1536}, {1295, 0, 1311}, {2385, 0, 1312}, {3600, 0, 1310}, {70, 235, 1514},
        {1057, 234, 1343}, {2098, 0, 1310}, {3293, 0, 1313}, {4095, 0, 1286}, {785, 490, 1351},
        {1822, 455, 1308}, {2997, 118, 1315}, {4095, 0, 1294}, {476, 731, 1329}, {1555, 748, 1306},
        {2715, 660, 1315}, {3947, 370, 1299}, {195, 1010, 1316}, {1297, 1014, 1305},
        {2448, 980, 1317}, {3670, 869, 1301}, {248, 1271, 1303}, {1048, 1270, 1304},
        {2206, 1281, 1320}, {3411, 1221, 1300}, {4095, 1072, 1253}, {569, 1605, 1313},
        {1936, 1606, 1281}, {3148, 1567, 1271}, {4095, 1448, 1247}, {391, 1943, 1280},
        {1594, 1974, 1212}, {2867, 1880, 1233}, {4073, 1806, 1200}, {460, 2283, 1221},
        {1106, 2317, 1165}, {2552, 2196, 1174}, {3783, 2127, 1154}, {529, 2619, 1135},
        {534, 2635, 1094}, {2204, 2548, 1071}, {3474, 2444, 1089}, {4095, 2346, 1040},
        {598, 2952, 993}, {1712, 2934, 892}, {3135, 2760, 1006}, {4095, 2664, 965},
        {662, 3263, 848}, {837, 3291, 660}, {2776, 3096, 850}, {4030, 2978, 865}, {723, 3565, 629},
        {724, 3565, 480}, {2325, 3457, 514}, {3675, 3284, 734}, {782, 3853, 295}, {781, 3850, 295},
        {1682, 3794, 0}, {3303, 3600, 418}, {4095, 3464, 438}, {826, 4069, 311}, {0, 4095, 0},
        {2865, 3928, 0}, {4095, 3755, 0}, {826, 4069, 311}, {0, 4095, 0}, {2295, 4095, 0},
        {3770, 4043, 0}, {826, 4069, 311}, {0, 4095, 0}, {1493, 4095, 0}, {3327, 4095, 0},
        {84, 0, 1775}, {1045, 0, 1608}, {2082, 0, 1576}, {3251, 0, 1596}, {4095, 0, 1585},
        {793, 127, 1647}, {1809, 173, 1569}, {2952, 0, 1593}, {4095, 0, 1591}, {510, 454, 1645},
        {1546, 498, 1564}, {2662, 353, 1590}, {3875, 0, 1594}, {131, 720, 1612}, {1293, 746, 1577},
        {2382, 712, 1587}, {3584, 547, 1596}, {179, 1005, 1602}, {1032, 1005, 1600},
        {2108, 1001, 1584}, {3306, 931, 1599}, {4095, 737, 1575}, {764, 1279, 1597},
        {1840, 1273, 1581}, {3043, 1249, 1602}, {4095, 1144, 1579}, {578, 1525, 1565},
        {1606, 1565, 1605}, {2806, 1559, 1608}, {4008, 1489, 1579}, {371, 1868, 1604},
        {1250, 1923, 1614}, {2546, 1897, 1576}, {3774, 1821, 1575}, {446, 2225, 1589},
        {658, 2254, 1579}, {2220, 2212, 1523}, {3465, 2154, 1513}, {4095, 2065, 1467},
        {521, 2586, 1525}, {1813, 2634, 1427}, {3156, 2467, 1457}, {4095, 2383, 1425},
        {588, 2914, 1450}, {1276, 2944, 1379}, {2804, 2782, 1390}, {4050, 2696, 1365},
        {654, 3233, 1349}, {658, 3247, 1293}, {2419, 3162, 1238}, {3716, 3006, 1289},
        {717, 3541, 1220}, {719, 3545, 1180}, {1842, 3526, 1048}, {3349, 3312, 1201},
        {4095, 3199, 1155}, {777, 3833, 1030}, {863, 3850, 829}, {2963, 3657, 979},
        {4095, 3499, 1051}, {825, 4069, 822}, {827, 4069, 674}, {2445, 3987, 654},
        {3842, 3788, 923}, {825, 4069, 467}, {826, 4069, 311}, {1731, 4095, 0}, {3443, 4093, 530},
        {825, 4069, 312}, {826, 4069, 311}, {0, 4095, 0}, {2946, 4095, 0}, {4095, 4095, 376},
        {783, 0, 1912}, {1801, 0, 1823}, {2920, 0, 1862}, {4095, 0, 1875}, {523, 0, 1944},
        {1548, 176, 1835}, {2631, 0, 1854}, {3820, 0, 1875}, {214, 401, 1926}, {1292, 459, 1852},
        {2350, 441, 1845}, {3523, 0, 1873}, {105, 706, 1902}, {1031, 725, 1873}, {2074, 745, 1834},
        {3234, 641, 1871}, {4095, 286, 1871}, {753, 997, 1899}, {1811, 1011, 1829},
        {2953, 970, 1869}, {4095, 840, 1874}, {463, 1278, 1887}, {1571, 1273, 1857},
        {2680, 1263, 1868}, {3893, 1196, 1878}, {290, 1540, 1858}, {1312, 1556, 1893},
        {2418, 1545, 1871}, {3634, 1515, 1882}, {348, 1779, 1825}, {1070, 1780, 1826},
        {2212, 1863, 1910}, {3399, 1829, 1890}, {4095, 1749, 1848}, {464, 2148, 1889},
        {1939, 2277, 1929}, {3144, 2176, 1859}, {4095, 2078, 1844}, {502, 2507, 1883},
        {1451, 2573, 1871}, {2833, 2485, 1807}, {4043, 2415, 1782}, {574, 2855, 1838},
        {751, 2888, 1813}, {2485, 2849, 1730}, {3737, 2722, 1728}, {643, 3188, 1763},
        {647, 3203, 1741}, {1970, 3252, 1610}, {3396, 3031, 1660}, {4095, 2929, 1623},
        {709, 3510, 1650}, {1406, 3526, 1562}, {3031, 3364, 1554}, {4095, 3232, 1551},
        {770, 3805, 1536}, {773, 3809, 1463}, {2570, 3724, 1379}, {3908, 3530, 1468},
        {824, 4069, 1395}, {825, 4069, 1339}, {1918, 4053, 1176}, {3529, 3835, 1329},
        {824, 4069, 1217}, {825, 4069, 1177}, {837, 4095, 964}, {3080, 4095, 1082},
        {4095, 3962, 1208}, {825, 4069, 965}, {827, 4069, 806}, {2502, 4095, 757},
        {3952, 4095, 1013}, {521, 0, 2208}, {1552, 0, 2114}, {2609, 0, 2111}, {3777, 0, 2147},
        {257, 0, 2220}, {1293, 0, 2132}, {2330, 120, 2097}, {3480, 0, 2141}, {0, 333, 2197},
        {1032, 404, 2154}, {2061, 492, 2085}, {3189, 304, 2134}, {4095, 0, 2151}, {762, 699, 2177},
        {1815, 742, 2105}, {2903, 698, 2125}, {4094, 501, 2152}, {456, 986, 2196},
        {1567, 1001, 2132}, {2621, 995, 2114}, {3808, 909, 2151}, {223, 1274, 2180},
        {1310, 1273, 2169}, {2341, 1266, 2099}, {3529, 1230, 2151}, {282, 1547, 2157},
        {1030, 1554, 2182}, {2080, 1525, 2096}, {3259, 1526, 2152}, {4095, 1457, 2150},
        {781, 1805, 2132}, {1860, 1818, 2158}, {3003, 1816, 2159}, {4095, 1775, 2155},
        {630, 2034, 2086}, {1579, 2035, 2089}, {2797, 2137, 2192}, {3978, 2090, 2161},
        {475, 2401, 2159}, {1182, 2435, 2176}, {2501, 2499, 2151}, {3724, 2442, 2130},
        {553, 2770, 2170}, {559, 2793, 2170}, {2157, 2895, 2153}, {3420, 2744, 2078},
        {4095, 2659, 2037}, {629, 3132, 2126}, {1617, 3185, 2086}, {3065, 3053, 2014},
        {4095, 2960, 1983}, {696, 3457, 2055}, {824, 3482, 2012}, {2674, 3446, 1898},
        {3957, 3261, 1916}, {760, 3764, 1961}, {763, 3774, 1924}, {2075, 3804, 1760},
        {3586, 3558, 1840}, {819, 4053, 1844}, {820, 4054, 1818}, {1494, 4046, 1711},
        {3189, 3896, 1678}, {4095, 3722, 1711}, {824, 4069, 1687}, {825, 4069, 1597},
        {2655, 4095, 1486}, {4042, 3998, 1619}, {824, 4069, 1531}, {825, 4069, 1457},
        {1937, 4095, 1268}, {3627, 4095, 1408}, {272, 0, 2489}, {1297, 0, 2420}, {2316, 0, 2344},
        {3446, 0, 2401}, {60, 0, 2491}, {1036, 0, 2442}, {2068, 130, 2361}, {3157, 0, 2390},
        {4095, 0, 2423}, {771, 320, 2465}, {1817, 445, 2383}, {2872, 417, 2376}, {4041, 0, 2419},
        {485, 668, 2483}, {1565, 716, 2411}, {2591, 738, 2359}, {3750, 610, 2413}, {149, 974, 2489},
        {1307, 988, 2444}, {2325, 1007, 2351}, {3464, 954, 2406}, {215, 1267, 2478},
        {1033, 1268, 2480}, {2087, 1265, 2379}, {3180, 1249, 2397}, {4095, 1172, 2426},
        {738, 1550, 2472}, {1847, 1537, 2420}, {2899, 1525, 2386}, {4095, 1487, 2428},
        {458, 1815, 2434}, {1590, 1832, 2481}, {2616, 1785, 2369}, {3836, 1786, 2431},
        {391, 2061, 2390}, {1313, 2074, 2412}, {2350, 2041, 2362}, {3585, 2082, 2439},
        {447, 2288, 2347}, {1100, 2288, 2348}, {2092, 2290, 2351}, {3374, 2401, 2465},
        {4095, 2340, 2421}, {528, 2667, 2433}, {1795, 2735, 2468}, {3098, 2760, 2425},
        {4095, 2664, 2421}, {605, 3035, 2448}, {1277, 3081, 2451}, {2762, 3151, 2372},
        {3979, 2986, 2334}, {678, 3382, 2414}, {684, 3407, 2401}, {2327, 3467, 2338},
        {3639, 3286, 2268}, {745, 3707, 2344}, {748, 3717, 2329}, {1742, 3744, 2261},
        {3271, 3616, 2173}, {4095, 3473, 2156}, {809, 4011, 2236}, {863, 4018, 2172},
        {2789, 3973, 2024}, {4095, 3756, 2075}, {822, 4069, 2121}, {823, 4069, 2067},
        {2133, 4095, 1878}, {3731, 4045, 1956}, {822, 4069, 1984}, {824, 4069, 1942},
        {1493, 4069, 1815}, {3262, 4095, 1760}, {134, 0, 2788}, {1043, 0, 2737}, {2076, 0, 2643},
        {3131, 0, 2640}, {4095, 0, 2687}, {781, 0, 2759}, {1822, 0, 2666}, {2849, 0, 2622},
        {4000, 0, 2679}, {509, 170, 2776}, {1566, 373, 2694}, {2575, 483, 2606}, {3709, 221, 2669},
        {155, 632, 2788}, {1308, 680, 2726}, {2333, 734, 2629}, {3422, 678, 2657}, {135, 958, 2788},
        {1039, 968, 2759}, {2089, 993, 2658}, {3136, 985, 2641}, {4095, 883, 2690},
        {743, 1258, 2789}, {1843, 1261, 2696}, {2852, 1261, 2620}, {4027, 1210, 2685},
        {408, 1545, 2767}, {1586, 1540, 2741}, {2593, 1521, 2618}, {3745, 1505, 2679},
        {327, 1818, 2735}, {1304, 1828, 2773}, {2364, 1790, 2656}, {3465, 1785, 2672},
        {384, 2077, 2698}, {1027, 2085, 2716}, {2133, 2084, 2720}, {3188, 2055, 2664},
        {4095, 2042, 2701}, {804, 2321, 2658}, {1853, 2348, 2702}, {2914, 2319, 2655},
        {4095, 2339, 2708}, {692, 2542, 2608}, {1600, 2543, 2610}, {2726, 2656, 2723},
        {3935, 2653, 2726}, {576, 2916, 2696}, {1133, 2936, 2706}, {2415, 3053, 2768},
        {3663, 3008, 2682}, {654, 3285, 2719}, {658, 3300, 2721}, {1957, 3367, 2722},
        {3313, 3307, 2615}, {4095, 3213, 2574}, {728, 3637, 2685}, {1351, 3672, 2660},
        {2908, 3719, 2521}, {4095, 3511, 2514}, {794, 3951, 2615}, {799, 3965, 2583},
        {2447, 3979, 2481}, {3819, 3801, 2438}, {819, 4069, 2516}, {820, 4069, 2486},
        {1808, 4068, 2389}, {3394, 4095, 2280}, {820, 4069, 2393}, {821, 4069, 2371},
        {823, 4069, 2284}, {2828, 4095, 2101}, {4095, 4095, 2191}, {790, 0, 3058}, {1829, 0, 2957},
        {2831, 0, 2865}, {3966, 0, 2935}, {531, 0, 3075}, {1571, 0, 2985}, {2586, 0, 2885},
        {3677, 0, 2921}, {243, 0, 3085}, {1311, 242, 3016}, {2339, 424, 2910}, {3391, 385, 2904},
        {0, 591, 3090}, {1045, 632, 3047}, {2092, 702, 2940}, {3108, 727, 2883}, {4095, 571, 2947},
        {763, 944, 3074}, {1841, 974, 2977}, {2839, 1003, 2872}, {3976, 934, 2936},
        {429, 1245, 3087}, {1584, 1251, 3017}, {2602, 1258, 2900}, {3689, 1236, 2923},
        {259, 1537, 3065}, {1311, 1536, 3059}, {2365, 1523, 2937}, {3403, 1514, 2905},
        {320, 1817, 3039}, {1013, 1821, 3063}, {2123, 1801, 2985}, {3116, 1776, 2881},
        {4095, 1760, 2955}, {719, 2088, 3016}, {1864, 2096, 3045}, {2862, 2035, 2884},
        {4031, 2043, 2952}, {463, 2339, 2967}, {1575, 2357, 3005}, {2643, 2320, 2938},
        {3760, 2321, 2950}, {492, 2575, 2916}, {1324, 2583, 2929}, {2409, 2633, 3008},
        {3503, 2601, 2955}, {547, 2796, 2869}, {1137, 2797, 2869}, {2109, 2798, 2872},
        {3329, 2949, 3022}, {4095, 2891, 2971}, {627, 3173, 2960}, {1733, 3207, 2980},
        {3078, 3385, 3063}, {4095, 3235, 2919}, {703, 3535, 2984}, {1115, 3563, 2986},
        {2591, 3642, 2974}, {3882, 3538, 2864}, {774, 3872, 2953}, {778, 3886, 2943},
        {2073, 3921, 2898}, {3512, 3872, 2773}, {816, 4068, 2879}, {817, 4068, 2867},
        {1384, 4068, 2816}, {2981, 4095, 2620}, {4095, 3974, 2648}, {818, 4069, 2764},
        {821, 4068, 2708}, {2492, 4068, 2571}, {3906, 4095, 2513}, {545, 0, 3377}, {1579, 0, 3282},
        {2597, 0, 3169}, {3650, 0, 3167}, {300, 0, 3387}, {1318, 0, 3313}, {2347, 0, 3196},
        {3366, 0, 3147}, {0, 0, 3390}, {1053, 0, 3342}, {2096, 330, 3227}, {3090, 472, 3127},
        {4095, 0, 3199}, {780, 570, 3368}, {1843, 656, 3263}, {2849, 724, 3151}, {3938, 652, 3184},
        {472, 914, 3387}, {1585, 948, 3302}, {2608, 983, 3182}, {3652, 973, 3165},
        {180, 1229, 3384}, {1316, 1236, 3340}, {2366, 1248, 3219}, {3366, 1255, 3141},
        {250, 1527, 3367}, {1025, 1527, 3376}, {2119, 1521, 3263}, {3106, 1516, 3139},
        {4095, 1488, 3202}, {706, 1814, 3356}, {1861, 1804, 3312}, {2874, 1779, 3172},
        {3958, 1768, 3188}, {371, 2087, 3317}, {1579, 2098, 3363}, {2642, 2053, 3218},
        {3672, 2035, 3171}, {429, 2349, 3274}, {1288, 2359, 3303}, {2403, 2346, 3280},
        {3384, 2290, 3146}, {486, 2596, 3227}, {1027, 2603, 3240}, {2126, 2632, 3300},
        {3130, 2549, 3149}, {4095, 2578, 3223}, {836, 2832, 3180}, {1848, 2847, 3204},
        {2936, 2870, 3244}, {4072, 2863, 3230}, {762, 3051, 3130}, {1626, 3052, 3131},
        {2621, 3053, 3134}, {3872, 3186, 3267}, {675, 3420, 3218}, {1093, 3430, 3224},
        {2304, 3479, 3250}, {3555, 3554, 3208}, {750, 3776, 3242}, {753, 3784, 3242},
        {1810, 3818, 3239}, {3221, 3909, 3207}, {4095, 3757, 3101}, {813, 4068, 3209},
        {1062, 4068, 3195}, {2712, 4067, 3121}, {4034, 4023, 3009}, {815, 4068, 3126},
        {817, 4068, 3102}, {2122, 4068, 3017}, {3562, 4095, 2825}, {325, 0, 3692}, {1326, 0, 3615},
        {2357, 0, 3487}, {3345, 0, 3386}, {193, 0, 3695}, {1065, 0, 3643}, {2103, 0, 3519},
        {3102, 0, 3408}, {4095, 0, 3448}, {797, 0, 3668}, {1846, 83, 3555}, {2859, 396, 3434},
        {3908, 341, 3429}, {511, 493, 3686}, {1588, 591, 3593}, {2614, 683, 3466},
        {3624, 714, 3406}, {0, 880, 3696}, {1322, 912, 3630}, {2368, 958, 3503}, {3353, 997, 3392},
        {163, 1210, 3685}, {1040, 1215, 3663}, {2118, 1233, 3546}, {3116, 1251, 3420},
        {4095, 1222, 3446}, {724, 1513, 3685}, {1860, 1514, 3591}, {2880, 1512, 3456},
        {3913, 1504, 3426}, {330, 1805, 3653}, {1586, 1801, 3635}, {2641, 1782, 3499},
        {3626, 1771, 3400}, {364, 2084, 3620}, {1287, 2089, 3653}, {2396, 2063, 3550},
        {3373, 2030, 3405}, {423, 2352, 3582}, {987, 2358, 3600}, {2134, 2356, 3607},
        {3147, 2299, 3444}, {4095, 2293, 3454}, {701, 2612, 3546}, {1841, 2630, 3592},
        {2919, 2585, 3501}, {3947, 2556, 3438}, {535, 2854, 3490}, {1567, 2867, 3515},
        {2679, 2900, 3585}, {3661, 2808, 3416}, {591, 3084, 3437}, {1340, 3089, 3445},
        {2376, 3112, 3481}, {3396, 3062, 3411}, {647, 3305, 3390}, {1182, 3305, 3391},
        {2129, 3306, 3393}, {3143, 3315, 3404}, {4095, 3464, 3552}, {723, 3669, 3476},
        {1693, 3684, 3483}, {2898, 3790, 3553}, {4095, 3798, 3466}, {799, 4025, 3504},
        {964, 4049, 3512}, {2443, 4067, 3513}, {3717, 4093, 3339}, {811, 4068, 3464},
        {812, 4068, 3458}, {1838, 4068, 3422}, {3263, 4066, 3265}, {236, 0, 4001}, {1074, 0, 3949},
        {2111, 0, 3817}, {3115, 0, 3693}, {4095, 0, 3691}, {814, 0, 3973}, {1854, 0, 3853},
        {2869, 0, 3721}, {3883, 0, 3670}, {546, 0, 3990}, {1593, 0, 3890}, {2621, 268, 3754},
        {3604, 458, 3648}, {231, 393, 3999}, {1329, 500, 3926}, {2371, 626, 3792},
        {3365, 712, 3672}, {0, 844, 3998}, {1054, 867, 3958}, {2119, 923, 3834}, {3125, 972, 3703},
        {4095, 958, 3689}, {755, 1189, 3982}, {1861, 1212, 3877}, {2884, 1235, 3739},
        {3881, 1248, 3664}, {383, 1498, 3980}, {1591, 1501, 3919}, {2641, 1505, 3782},
        {3618, 1512, 3658}, {293, 1794, 3954}, {1302, 1792, 3957}, {2393, 1781, 3829},
        {3386, 1770, 3690}, {356, 2078, 3924}, {988, 2080, 3945}, {2133, 2065, 3879},
        {3152, 2037, 3730}, {4095, 2025, 3686}, {655, 2354, 3899}, {1851, 2358, 3929},
        {2916, 2313, 3778}, {3887, 2283, 3659}, {472, 2616, 3851}, {1552, 2627, 3886},
        {2669, 2603, 3837}, {3641, 2543, 3670}, {529, 2868, 3800}, {1272, 2876, 3820},
        {2393, 2899, 3882}, {3418, 2819, 3715}, {585, 3108, 3747}, {1034, 3112, 3756},
        {2102, 3130, 3792}, {3194, 3122, 3789}, {4095, 3112, 3749}, {876, 3338, 3698},
        {1854, 3346, 3711}, {2908, 3378, 3762}, {4056, 3425, 3793}, {840, 3559, 3651},
        {1658, 3560, 3652}, {2686, 3621, 3716}, {3869, 3770, 3865}, {775, 3932, 3747},
        {1076, 3949, 3761}, {2296, 4003, 3803}, {3469, 4064, 3810}, {806, 4068, 3763},
        {807, 4068, 3767}, {1695, 4068, 3769}, {2984, 4067, 3704}, {4095, 4095, 3485},
        {826, 0, 4095}, {1862, 0, 4095}, {2880, 0, 4013}, {3860, 0, 3906}, {575, 0, 4095},
        {1602, 0, 4095}, {2629, 0, 4047}, {3618, 0, 3929}, {322, 0, 4095}, {1338, 0, 4095},
        {2377, 0, 4085}, {3376, 361, 3956}, {0, 245, 4095}, {1068, 366, 4095}, {2122, 544, 4095},
        {3133, 661, 3988}, {4095, 698, 3929}, {783, 815, 4095}, {1864, 877, 4095},
        {2889, 939, 4026}, {3866, 991, 3912}, {449, 1159, 4095}, {1597, 1183, 4095},
        {2643, 1214, 4068}, {3630, 1243, 3940}, {213, 1479, 4095}, {1316, 1482, 4095},
        {2392, 1492, 4095}, {3394, 1501, 3974}, {284, 1780, 4095}, {1007, 1779, 4095},
        {2133, 1774, 4095}, {3156, 1767, 4014}, {4095, 1766, 3920}, {663, 2070, 4095},
        {1859, 2062, 4095}, {2915, 2040, 4060}, {3885, 2025, 3925}, {407, 2348, 4095},
        {1560, 2353, 4095}, {2665, 2321, 4095}, {3655, 2287, 3959}, {466, 2616, 4095},
        {1255, 2622, 4095}, {2399, 2611, 4095}, {3423, 2559, 4001}, {523, 2875, 4095},
        {958, 2879, 4095}, {2105, 2893, 4095}, {3186, 2843, 4055}, {4095, 2794, 3921},
        {688, 3125, 4063}, {1818, 3137, 4094}, {2932, 3142, 4095}, {3936, 3080, 3963},
        {634, 3361, 4006}, {1564, 3369, 4021}, {2635, 3390, 4067}, {3760, 3407, 4064},
        {690, 3589, 3954}, {1363, 3592, 3959}, {2399, 3642, 4019}, {3536, 3740, 4095},
        {765, 3841, 3940}, {1255, 3852, 3952}, {2210, 3897, 3999}, {3280, 3966, 4072},
        {4095, 3938, 4035}, {801, 4068, 4023}, {1687, 4067, 4045}, {2830, 4067, 4053},
        {3903, 4065, 3888}, {596, 0, 4095}, {1611, 0, 4095}, {2639, 0, 4095}, {3632, 0, 4095},
        {380, 0, 4095}, {1350, 0, 4095}, {2385, 0, 4095}, {3387, 0, 4095}, {244, 0, 4095},
        {1086, 0, 4095}, {2129, 0, 4095}, {3142, 171, 4095}, {4095, 442, 4095}, {810, 0, 4095},
        {1869, 422, 4095}, {2895, 593, 4095}, {3879, 698, 4095}, {511, 756, 4095},
        {1605, 820, 4095}, {2646, 897, 4095}, {3640, 958, 4095}, {108, 1130, 4095},
        {1330, 1149, 4095}, {2394, 1187, 4095}, {3400, 1222, 4095}, {208, 1459, 4095},
        {1035, 1460, 4095}, {2135, 1474, 4095}, {3160, 1489, 4095}, {4095, 1507, 4095},
        {699, 1764, 4095}, {1865, 1763, 4095}, {2915, 1761, 4095}, {3897, 1762, 4095},
        {344, 2058, 4095}, {1577, 2054, 4095}, {2664, 2039, 4095}, {3663, 2024, 4095},
        {405, 2341, 4095}, {1264, 2343, 4095}, {2401, 2323, 4095}, {3426, 2293, 4095},
        {462, 2614, 4095}, {941, 2616, 4095}, {2119, 2613, 4095}, {3184, 2570, 4095},
        {4095, 2537, 4095}, {594, 2879, 4095}, {1816, 2887, 4095}, {2930, 2856, 4095},
        {3921, 2803, 4095}, {573, 3132, 4095}, {1525, 3140, 4095}, {2653, 3150, 4095},
        {3700, 3088, 4095}, {628, 3376, 4095}, {1260, 3381, 4095}, {2355, 3395, 4095},
        {3487, 3407, 4095}, {684, 3611, 4095}, {1047, 3615, 4095}, {2106, 3648, 4095},
        {3229, 3716, 4095}, {4095, 3685, 4095}, {942, 3872, 4095}, {1905, 3906, 4095},
        {2973, 3965, 4095}, {4041, 3952, 4095}, {946, 4068, 4095}, {1737, 4067, 4095},
        {2737, 4067, 4095}, {3733, 4066, 4095}},
    // lut3
    {{950, 0, 31}, {1854, 0, 60}, {3103, 0, 0}, {4095, 0, 0}, {685, 333, 0}, {1554, 0, 0},
        {2789, 0, 0}, {4059, 0, 0}, {105, 563, 0}, {1300, 384, 0}, {2488, 0, 0}, {3752, 0, 0},
        {0, 754, 0}, {1020, 698, 0}, {2206, 620, 0}, {3453, 333, 0}, {4095, 0, 0}, {675, 998, 0},
        {1933, 966, 0}, {3163, 867, 0}, {4095, 596, 0}, {0, 1327, 0}, {1643, 1290, 0},
        {2877, 1230, 0}, {4095, 1096, 0}, {0, 1675, 0}, {1306, 1623, 0}, {2584, 1564, 0},
        {3812, 1474, 0}, {0, 2018, 0}, {762, 1981, 0}, {2267, 1891, 0}, {3515, 1818, 0},
        {4095, 1695, 0}, {0, 2338, 0}, {1923, 2232, 0}, {3203, 2148, 0}, {4095, 2046, 0},
        {0, 2684, 0}, {1458, 2595, 0}, {2866, 2472, 0}, {4095, 2380, 0}, {0, 3011, 0},
        {577, 2950, 0}, {2512, 2809, 0}, {3779, 2704, 0}, {0, 3323, 0}, {0, 3287, 0},
        {2073, 3164, 0}, {3428, 3020, 0}, {4095, 2907, 0}, {0, 3605, 0}, {1444, 3504, 0},
        {3064, 3345, 0}, {4095, 3220, 0}, {0, 3901, 0}, {0, 3825, 0}, {2633, 3679, 0},
        {3940, 3522, 0}, {0, 4095, 0}, {0, 4095, 0}, {2073, 3996, 0}, {3564, 3826, 0}, {0, 4095, 0},
        {0, 4095, 0}, {1244, 4095, 0}, {3132, 4095, 0}, {4095, 3959, 0}, {0, 4095, 0}, {0, 4095, 0},
        {2602, 4095, 0}, {3988, 4095, 0}, {769, 199, 421}, {1566, 0, 94}, {2794, 0, 91},
        {4048, 0, 0}, {428, 417, 428}, {1324, 0, 153}, {2480, 0, 80}, {3736, 0, 0}, {117, 577, 201},
        {1052, 445, 105}, {2171, 160, 0}, {3433, 0, 0}, {4095, 0, 0}, {739, 726, 0}, {1896, 652, 0},
        {3136, 462, 0}, {4095, 0, 0}, {0, 1003, 0}, {1627, 980, 0}, {2851, 906, 0}, {4087, 699, 0},
        {0, 1332, 0}, {1325, 1299, 0}, {2568, 1252, 0}, {3796, 1143, 0}, {0, 1675, 0},
        {909, 1645, 0}, {2272, 1580, 0}, {3506, 1504, 0}, {4095, 1360, 0}, {0, 2003, 0},
        {1942, 1905, 0}, {3207, 1841, 0}, {4095, 1733, 0}, {0, 2355, 0}, {1558, 2266, 0},
        {2889, 2168, 0}, {4095, 2077, 0}, {0, 2688, 0}, {925, 2628, 0}, {2540, 2490, 0},
        {3804, 2407, 0}, {0, 3010, 0}, {0, 2977, 0}, {2164, 2849, 0}, {3468, 2729, 0},
        {4095, 2619, 0}, {0, 3308, 0}, {1632, 3202, 0}, {3106, 3043, 0}, {4095, 2939, 0},
        {0, 3619, 0}, {665, 3538, 0}, {2728, 3388, 0}, {4000, 3250, 0}, {0, 3905, 0}, {0, 3853, 0},
        {2233, 3721, 0}, {3627, 3551, 0}, {0, 4095, 0}, {0, 4095, 0}, {1545, 4034, 0},
        {3240, 3870, 0}, {4095, 3716, 0}, {0, 4095, 0}, {0, 4095, 0}, {2761, 4095, 0},
        {4084, 3996, 0}, {0, 4095, 0}, {0, 4095, 0}, {2154, 4095, 0}, {3680, 4095, 0},
        {461, 272, 828}, {1332, 0, 502}, {2457, 0, 420}, {3731, 0, 303}, {203, 414, 767},
        {1105, 218, 542}, {2150, 0, 431}, {3416, 0, 340}, {4095, 0, 134}, {810, 505, 519},
        {1865, 321, 428}, {3114, 0, 354}, {4095, 0, 133}, {370, 730, 385}, {1587, 685, 393},
        {2813, 564, 330}, {4063, 0, 0}, {204, 1007, 291}, {1320, 993, 330}, {2534, 945, 284},
        {3770, 791, 0}, {269, 1326, 102}, {980, 1317, 0}, {2256, 1273, 138}, {3482, 1189, 0},
        {4095, 1002, 0}, {0, 1672, 0}, {1954, 1596, 0}, {3194, 1533, 0}, {4095, 1414, 0},
        {0, 2017, 0}, {1613, 1936, 0}, {2894, 1863, 0}, {4095, 1771, 0}, {475, 2338, 179},
        {1108, 2303, 0}, {2568, 2187, 0}, {3813, 2107, 0}, {542, 2667, 204}, {0, 2662, 0},
        {2218, 2528, 0}, {3494, 2433, 0}, {4095, 2324, 0}, {0, 3005, 0}, {1765, 2892, 0},
        {3150, 2752, 0}, {4095, 2652, 0}, {0, 3318, 0}, {1044, 3242, 0}, {2791, 3085, 0},
        {4046, 2970, 0}, {0, 3618, 0}, {0, 3573, 0}, {2356, 3432, 0}, {3689, 3279, 0}, {0, 3903, 0},
        {0, 3881, 0}, {1762, 3763, 0}, {3319, 3594, 0}, {4095, 3459, 0}, {0, 4095, 0},
        {692, 4072, 0}, {2888, 3915, 0}, {4095, 3751, 0}, {0, 4095, 0}, {0, 4095, 0},
        {2343, 4095, 0}, {3786, 4040, 0}, {0, 4095, 0}, {0, 4095, 0}, {1598, 4095, 0},
        {3348, 4095, 0}, {162, 250, 1117}, {1113, 19, 845}, {2128, 0, 742}, {3375, 0, 710},
        {4095, 0, 625}, {882, 327, 889}, {1837, 0, 747}, {3063, 0, 720}, {4095, 0, 651},
        {562, 548, 858}, {1563, 421, 747}, {2765, 0, 724}, {4026, 0, 666}, {310, 786, 806},
        {1299, 711, 735}, {2488, 631, 720}, {3736, 296, 670}, {203, 1014, 715}, {1008, 1005, 681},
        {2213, 978, 688}, {3441, 871, 645}, {4095, 575, 536}, {548, 1328, 598}, {1942, 1291, 644},
        {3163, 1232, 615}, {4095, 1086, 517}, {337, 1661, 531}, {1628, 1610, 573},
        {2879, 1560, 556}, {4095, 1465, 468}, {405, 1996, 383}, {1216, 1979, 308},
        {2577, 1883, 453}, {3805, 1806, 369}, {473, 2330, 179}, {0, 2349, 0}, {2238, 2205, 266},
        {3502, 2134, 79}, {4095, 2026, 0}, {541, 2663, 203}, {1855, 2575, 0}, {3178, 2457, 0},
        {4095, 2360, 0}, {606, 2982, 228}, {1260, 2939, 0}, {2822, 2775, 0}, {4076, 2683, 0},
        {669, 3292, 251}, {0, 3285, 0}, {2444, 3132, 0}, {3736, 2998, 0}, {729, 3590, 274},
        {0, 3609, 0}, {1924, 3480, 0}, {3368, 3306, 0}, {4095, 3191, 0}, {0, 3895, 0},
        {1115, 3808, 0}, {2985, 3641, 0}, {4095, 3494, 0}, {0, 4095, 0}, {0, 4095, 0},
        {2494, 3962, 0}, {3860, 3785, 0}, {0, 4095, 0}, {0, 4095, 0}, {1842, 4095, 0},
        {3466, 4086, 0}, {0, 4095, 0}, {0, 4095, 0}, {647, 4095, 0}, {2984, 4095, 0},
        {4095, 4095, 0}, {866, 130, 1164}, {1825, 0, 1033}, {3020, 0, 1026}, {4095, 0, 990},
        {597, 299, 1206}, {1551, 106, 1036}, {2715, 0, 1029}, {3966, 0, 1002}, {260, 531, 1152},
        {1288, 485, 1038}, {2424, 307, 1030}, {3664, 0, 1008}, {139, 763, 1065}, {1034, 758, 1042},
        {2151, 691, 1029}, {3376, 507, 1011}, {4095, 0, 956}, {788, 1016, 1043}, {1897, 992, 1025},
        {3108, 918, 1008}, {4095, 705, 959}, {262, 1309, 1012}, {1627, 1303, 983},
        {2838, 1268, 979}, {4079, 1138, 953}, {330, 1638, 968}, {1281, 1653, 904},
        {2561, 1583, 941}, {3777, 1511, 907}, {399, 1976, 897}, {750, 1996, 844}, {2255, 1901, 880},
        {3490, 1838, 860}, {4095, 1729, 789}, {471, 2322, 761}, {1909, 2250, 759},
        {3187, 2159, 788}, {4095, 2067, 729}, {537, 2648, 624}, {1404, 2632, 516},
        {2855, 2479, 687}, {4089, 2393, 634}, {602, 2969, 364}, {0, 2995, 0}, {2500, 2820, 453},
        {3766, 2712, 487}, {665, 3281, 251}, {667, 3282, 251}, {2047, 3186, 0}, {3417, 3025, 170},
        {4095, 2916, 0}, {727, 3579, 273}, {1366, 3533, 0}, {3052, 3351, 0}, {4095, 3225, 0},
        {785, 3865, 295}, {0, 3855, 0}, {2615, 3692, 0}, {3930, 3526, 0}, {827, 4069, 311},
        {0, 4095, 0}, {2033, 4012, 0}, {3553, 3829, 0}, {827, 4069, 311}, {0, 4095, 0},
        {1135, 4095, 0}, {3117, 4095, 0}, {4095, 3961, 0}, {0, 4095, 0}, {0, 4095, 0},
        {2574, 4095, 0}, {3977, 4095, 0}, {578, 0, 1480}, {1547, 0, 1304}, {2681, 0, 1314},
        {3912, 0, 1305}, {293, 234, 1499}, {1290, 242, 1306}, {2387, 0, 1312}, {3603, 0, 1310},
        {71, 498, 1420}, {1035, 490, 1317}, {2105, 419, 1311}, {3304, 0, 1313}, {4095, 0, 1283},
        {759, 736, 1320}, {1833, 732, 1310}, {3019, 619, 1314}, {4095, 108, 1289},
        {488, 1009, 1318}, {1567, 1009, 1309}, {2750, 963, 1316}, {3978, 818, 1292},
        {345, 1271, 1303}, {1319, 1286, 1318}, {2504, 1272, 1317}, {3716, 1193, 1292},
        {317, 1591, 1316}, {939, 1621, 1309}, {2242, 1600, 1283}, {3475, 1532, 1285},
        {4095, 1411, 1230}, {393, 1952, 1273}, {1924, 1916, 1232}, {3171, 1865, 1228},
        {4095, 1779, 1185}, {461, 2287, 1216}, {1506, 2323, 1123}, {2868, 2181, 1172},
        {4081, 2104, 1143}, {530, 2620, 1133}, {916, 2643, 1066}, {2523, 2499, 1103},
        {3778, 2423, 1081}, {596, 2946, 1017}, {600, 2956, 970}, {2138, 2881, 929},
        {3451, 2738, 999}, {4095, 2636, 948}, {662, 3265, 830}, {1542, 3250, 692},
        {3089, 3050, 898}, {4095, 2950, 849}, {723, 3565, 615}, {0, 3586, 273}, {2707, 3406, 600},
        {3984, 3257, 717}, {782, 3853, 295}, {781, 3847, 294}, {2185, 3748, 0}, {3611, 3556, 527},
        {826, 4069, 311}, {827, 4069, 311}, {1422, 4066, 0}, {3221, 3880, 0}, {4095, 3720, 0},
        {827, 4069, 311}, {0, 4095, 0}, {2726, 4095, 0}, {4068, 3999, 0}, {827, 4069, 311},
        {0, 4095, 0}, {2086, 4095, 0}, {3661, 4095, 0}, {286, 0, 1761}, {1286, 0, 1571},
        {2365, 0, 1583}, {3556, 0, 1596}, {0, 36, 1746}, {1039, 156, 1600}, {2083, 0, 1576},
        {3253, 0, 1596}, {4095, 0, 1584}, {771, 457, 1613}, {1811, 480, 1569}, {2960, 262, 1594},
        {4095, 0, 1590}, {477, 721, 1611}, {1549, 755, 1565}, {2676, 686, 1593}, {3892, 462, 1593},
        {178, 1004, 1603}, {1300, 1009, 1584}, {2402, 991, 1592}, {3612, 898, 1596},
        {238, 1276, 1587}, {1033, 1282, 1608}, {2138, 1274, 1594}, {3347, 1232, 1599},
        {4095, 1103, 1567}, {812, 1525, 1565}, {1916, 1578, 1618}, {3105, 1547, 1603},
        {4095, 1461, 1567}, {372, 1872, 1605}, {1625, 1967, 1617}, {2849, 1887, 1573},
        {4068, 1797, 1563}, {446, 2226, 1588}, {1132, 2274, 1569}, {2545, 2198, 1523},
        {3765, 2136, 1505}, {518, 2574, 1536}, {524, 2597, 1514}, {2200, 2561, 1443},
        {3463, 2448, 1452}, {4095, 2356, 1411}, {590, 2919, 1441}, {1675, 2964, 1318},
        {3128, 2762, 1385}, {4095, 2670, 1354}, {654, 3235, 1344}, {1046, 3252, 1262},
        {2769, 3102, 1274}, {4021, 2980, 1280}, {717, 3541, 1218}, {719, 3547, 1153},
        {2307, 3471, 1081}, {3667, 3286, 1192}, {777, 3833, 1055}, {777, 3833, 1008},
        {1630, 3813, 840}, {3296, 3603, 1035}, {4095, 3466, 1035}, {825, 4069, 803}, {0, 4095, 516},
        {2852, 3936, 725}, {4095, 3756, 907}, {825, 4069, 448}, {827, 4069, 311}, {2267, 4095, 0},
        {3762, 4044, 644}, {825, 4069, 312}, {827, 4069, 311}, {1426, 4095, 0}, {3315, 4095, 0},
        {83, 0, 2002}, {1030, 0, 1867}, {2070, 0, 1833}, {3216, 0, 1868}, {4095, 0, 1872},
        {777, 0, 1903}, {1801, 226, 1824}, {2922, 0, 1862}, {4095, 0, 1875}, {500, 407, 1907},
        {1550, 478, 1837}, {2635, 396, 1855}, {3826, 0, 1875}, {123, 705, 1903}, {1296, 735, 1857},
        {2355, 728, 1847}, {3536, 592, 1875}, {170, 997, 1894}, {1033, 1000, 1885},
        {2078, 1008, 1835}, {3254, 949, 1875}, {4095, 786, 1869}, {751, 1279, 1895},
        {1817, 1269, 1832}, {2981, 1253, 1876}, {4095, 1166, 1873}, {508, 1541, 1861},
        {1585, 1549, 1880}, {2723, 1545, 1881}, {3937, 1496, 1877}, {419, 1779, 1825},
        {1318, 1780, 1827}, {2508, 1858, 1906}, {3696, 1814, 1882}, {423, 2136, 1884},
        {872, 2160, 1893}, {2213, 2211, 1865}, {3443, 2163, 1852}, {4095, 2052, 1828},
        {504, 2515, 1882}, {1858, 2604, 1858}, {3144, 2469, 1804}, {4095, 2393, 1770},
        {575, 2858, 1836}, {1298, 2904, 1798}, {2796, 2782, 1742}, {4035, 2700, 1719},
        {643, 3189, 1762}, {649, 3211, 1726}, {2407, 3181, 1625}, {3707, 3007, 1654},
        {708, 3506, 1664}, {711, 3514, 1637}, {1792, 3551, 1484}, {3341, 3312, 1579},
        {4095, 3202, 1539}, {770, 3806, 1526}, {1138, 3808, 1428}, {2952, 3665, 1412},
        {4095, 3500, 1456}, {824, 4069, 1390}, {825, 4069, 1308}, {2417, 4000, 1206},
        {3834, 3789, 1362}, {824, 4069, 1215}, {825, 4069, 1150}, {1664, 4095, 956},
        {3433, 4095, 1132}, {825, 4069, 990}, {825, 4069, 941}, {0, 4095, 668}, {2926, 4095, 818},
        {4095, 4095, 1058}, {772, 0, 2172}, {1807, 0, 2098}, {2894, 0, 2123}, {4078, 0, 2150},
        {514, 0, 2198}, {1552, 0, 2114}, {2609, 0, 2111}, {3778, 0, 2147}, {211, 338, 2194},
        {1295, 430, 2136}, {2331, 469, 2097}, {3484, 160, 2142}, {87, 686, 2196}, {1033, 710, 2161},
        {2064, 752, 2088}, {3196, 667, 2136}, {4095, 402, 2151}, {755, 989, 2189},
        {1821, 1005, 2110}, {2913, 981, 2128}, {4095, 871, 2153}, {445, 1274, 2185},
        {1575, 1271, 2144}, {2632, 1263, 2119}, {3832, 1209, 2154}, {282, 1547, 2158},
        {1311, 1557, 2194}, {2352, 1528, 2104}, {3562, 1515, 2157}, {341, 1801, 2125},
        {1044, 1809, 2140}, {2087, 1784, 2098}, {3309, 1813, 2164}, {4095, 1754, 2146},
        {847, 2034, 2087}, {1831, 2035, 2089}, {3093, 2129, 2186}, {4095, 2072, 2150},
        {476, 2404, 2161}, {1508, 2461, 2188}, {2822, 2484, 2149}, {4053, 2399, 2152},
        {553, 2771, 2170}, {932, 2809, 2170}, {2487, 2877, 2097}, {3721, 2725, 2072},
        {627, 3123, 2131}, {631, 3141, 2121}, {2045, 3203, 2063}, {3386, 3031, 2008},
        {4095, 2934, 1971}, {697, 3461, 2050}, {1429, 3491, 1990}, {3025, 3371, 1916},
        {4095, 3234, 1906}, {760, 3766, 1957}, {764, 3777, 1904}, {2545, 3743, 1771},
        {3898, 3530, 1830}, {819, 4053, 1843}, {821, 4054, 1800}, {1867, 4065, 1623},
        {3520, 3837, 1713}, {823, 4069, 1703}, {824, 4069, 1673}, {1157, 4069, 1557},
        {3062, 4095, 1517}, {4095, 3961, 1604}, {824, 4069, 1521}, {825, 4069, 1422},
        {2463, 4095, 1296}, {3940, 4095, 1454}, {520, 0, 2478}, {1557, 0, 2399}, {2590, 0, 2360},
        {3740, 0, 2411}, {261, 0, 2488}, {1298, 0, 2421}, {2316, 206, 2345}, {3447, 0, 2402},
        {0, 258, 2490}, {1036, 356, 2446}, {2070, 467, 2363}, {3159, 358, 2390}, {4095, 0, 2423},
        {766, 678, 2470}, {1822, 728, 2388}, {2874, 716, 2376}, {4048, 551, 2420}, {458, 974, 2493},
        {1571, 993, 2420}, {2593, 1003, 2358}, {3761, 929, 2415}, {215, 1267, 2479},
        {1311, 1268, 2458}, {2330, 1265, 2354}, {3478, 1237, 2410}, {275, 1547, 2459},
        {1025, 1552, 2485}, {2097, 1530, 2387}, {3198, 1522, 2404}, {4095, 1468, 2428},
        {749, 1817, 2440}, {1860, 1817, 2444}, {2921, 1795, 2396}, {4095, 1773, 2432},
        {539, 2062, 2392}, {1583, 2083, 2429}, {2648, 2060, 2389}, {3887, 2075, 2439},
        {502, 2288, 2347}, {1341, 2289, 2349}, {2452, 2390, 2450}, {3667, 2390, 2456},
        {526, 2660, 2429}, {812, 2675, 2437}, {2136, 2784, 2492}, {3402, 2745, 2419},
        {4095, 2638, 2402}, {607, 3040, 2449}, {1667, 3105, 2450}, {3056, 3050, 2356},
        {4095, 2964, 2323}, {678, 3385, 2413}, {984, 3419, 2393}, {2658, 3476, 2270},
        {3944, 3260, 2259}, {745, 3708, 2343}, {750, 3723, 2319}, {2184, 3749, 2228},
        {3576, 3557, 2186}, {808, 4009, 2247}, {810, 4013, 2227}, {1519, 4018, 2143},
        {3176, 3905, 2044}, {4095, 3721, 2061}, {822, 4069, 2114}, {824, 4069, 2041},
        {2617, 4095, 1876}, {4031, 3997, 1976}, {822, 4069, 1980}, {824, 4069, 1920},
        {1891, 4095, 1723}, {3612, 4095, 1792}, {283, 0, 2785}, {1304, 0, 2713}, {2326, 0, 2623},
        {3417, 0, 2656}, {101, 0, 2787}, {1042, 0, 2738}, {2076, 0, 2644}, {3131, 0, 2640},
        {4095, 0, 2687}, {776, 229, 2761}, {1825, 406, 2670}, {2849, 452, 2622}, {4002, 0, 2680},
        {490, 639, 2780}, {1570, 695, 2701}, {2578, 746, 2609}, {3713, 640, 2670}, {134, 958, 2789},
        {1310, 975, 2737}, {2338, 999, 2634}, {3427, 967, 2658}, {206, 1258, 2778},
        {1034, 1259, 2772}, {2097, 1261, 2667}, {3141, 1253, 2642}, {4095, 1187, 2692},
        {730, 1546, 2775}, {1851, 1535, 2710}, {2856, 1520, 2620}, {4044, 1492, 2689},
        {410, 1819, 2739}, {1588, 1825, 2764}, {2599, 1779, 2621}, {3767, 1780, 2686},
        {384, 2077, 2699}, {1303, 2091, 2729}, {2377, 2060, 2670}, {3496, 2060, 2684},
        {441, 2318, 2653}, {1058, 2324, 2663}, {2139, 2368, 2735}, {3237, 2342, 2689},
        {4095, 2328, 2703}, {892, 2542, 2608}, {1848, 2544, 2611}, {3062, 2688, 2755},
        {4095, 2637, 2713}, {577, 2918, 2696}, {1451, 2949, 2714}, {2815, 3132, 2801},
        {3955, 2989, 2671}, {654, 3286, 2719}, {707, 3310, 2722}, {2327, 3397, 2717},
        {3628, 3283, 2607}, {727, 3631, 2688}, {730, 3643, 2682}, {1790, 3686, 2646},
        {3265, 3626, 2521}, {4095, 3484, 2503}, {794, 3953, 2611}, {1013, 3970, 2566},
        {2755, 3999, 2392}, {4095, 3770, 2426}, {819, 4069, 2513}, {821, 4069, 2470},
        {2264, 4068, 2350}, {3726, 4053, 2306}, {820, 4069, 2392}, {822, 4069, 2356},
        {1524, 4068, 2248}, {3237, 4095, 2120}, {159, 0, 3087}, {1050, 0, 3036}, {2085, 0, 2931},
        {3108, 0, 2885}, {4095, 0, 2946}, {789, 0, 3058}, {1830, 0, 2958}, {2831, 176, 2866},
        {3967, 0, 2935}, {516, 0, 3076}, {1573, 305, 2989}, {2588, 452, 2887}, {3678, 304, 2921},
        {172, 594, 3088}, {1312, 650, 3023}, {2343, 716, 2914}, {3392, 700, 2903}, {115, 937, 3089},
        {1042, 951, 3056}, {2098, 982, 2948}, {3108, 996, 2881}, {4095, 904, 2948},
        {746, 1245, 3085}, {1848, 1253, 2988}, {2844, 1261, 2875}, {3983, 1220, 2938},
        {395, 1537, 3069}, {1587, 1533, 3033}, {2610, 1521, 2906}, {3697, 1507, 2925},
        {320, 1817, 3040}, {1302, 1824, 3078}, {2375, 1792, 2949}, {3412, 1778, 2908},
        {378, 2084, 3006}, {1011, 2091, 3026}, {2133, 2081, 3007}, {3124, 2035, 2885},
        {4095, 2036, 2960}, {751, 2342, 2972}, {1856, 2367, 3026}, {2868, 2294, 2886},
        {4065, 2320, 2962}, {582, 2575, 2917}, {1584, 2589, 2938}, {2671, 2612, 2978},
        {3814, 2609, 2970}, {591, 2796, 2869}, {1371, 2797, 2870}, {2361, 2798, 2873},
        {3619, 2937, 3012}, {626, 3169, 2958}, {757, 3178, 2963}, {2036, 3228, 2991},
        {3302, 3301, 2953}, {4095, 3220, 2909}, {704, 3539, 2984}, {1516, 3576, 2986},
        {2968, 3669, 2956}, {4095, 3519, 2858}, {774, 3873, 2952}, {781, 3893, 2936},
        {2460, 3935, 2877}, {3823, 3812, 2786}, {816, 4068, 2879}, {817, 4068, 2859},
        {1865, 4068, 2792}, {3384, 4095, 2632}, {818, 4069, 2773}, {819, 4068, 2756},
        {941, 4068, 2683}, {2787, 4095, 2468}, {4095, 4095, 2525}, {799, 0, 3361}, {1837, 0, 3252},
        {2844, 0, 3146}, {3936, 0, 3185}, {541, 0, 3377}, {1579, 0, 3283}, {2597, 0, 3170},
        {3650, 0, 3167}, {260, 0, 3388}, {1317, 0, 3316}, {2349, 374, 3198}, {3366, 430, 3146},
        {0, 542, 3393}, {1050, 590, 3347}, {2100, 675, 3232}, {3092, 739, 3129}, {4095, 606, 3199},
        {767, 920, 3374}, {1847, 957, 3272}, {2854, 992, 3155}, {3941, 950, 3184},
        {430, 1228, 3389}, {1587, 1239, 3313}, {2615, 1251, 3188}, {3654, 1243, 3165},
        {250, 1527, 3368}, {1312, 1526, 3353}, {2374, 1519, 3229}, {3368, 1515, 3140},
        {312, 1812, 3344}, {1009, 1815, 3368}, {2126, 1798, 3277}, {3111, 1774, 3142},
        {4095, 1760, 3205}, {698, 2089, 3324}, {1862, 2088, 3332}, {2883, 2042, 3179},
        {3971, 2035, 3193}, {429, 2350, 3277}, {1570, 2365, 3318}, {2654, 2326, 3234},
        {3687, 2300, 3178}, {485, 2597, 3228}, {1298, 2607, 3248}, {2413, 2640, 3317},
        {3401, 2554, 3156}, {541, 2830, 3177}, {1080, 2834, 3183}, {2113, 2856, 3218},
        {3135, 2807, 3150}, {4095, 2860, 3232}, {945, 3051, 3130}, {1870, 3052, 3132},
        {2875, 3053, 3135}, {4095, 3171, 3253}, {675, 3421, 3218}, {1411, 3436, 3227},
        {2610, 3509, 3265}, {3877, 3542, 3209}, {750, 3776, 3242}, {754, 3789, 3242},
        {2158, 3839, 3241}, {3512, 3895, 3129}, {812, 4068, 3211}, {813, 4068, 3207},
        {1552, 4068, 3186}, {3078, 4067, 3078}, {4095, 3980, 2986}, {815, 4068, 3123},
        {818, 4068, 3089}, {2519, 4067, 2977}, {3896, 4095, 2844}, {555, 0, 3683}, {1586, 0, 3584},
        {2608, 0, 3458}, {3625, 0, 3408}, {316, 0, 3692}, {1326, 0, 3616}, {2357, 0, 3488},
        {3345, 129, 3386}, {105, 0, 3695}, {1060, 0, 3646}, {2104, 229, 3522}, {3104, 432, 3409},
        {4095, 222, 3447}, {786, 512, 3671}, {1849, 618, 3561}, {2862, 701, 3438},
        {3909, 679, 3428}, {479, 884, 3689}, {1589, 924, 3601}, {2619, 968, 3472},
        {3624, 987, 3404}, {163, 1210, 3687}, {1319, 1219, 3639}, {2374, 1238, 3512},
        {3357, 1256, 3395}, {239, 1514, 3670}, {1026, 1513, 3672}, {2123, 1513, 3557},
        {3123, 1513, 3425}, {4095, 1493, 3447}, {700, 1805, 3661}, {1861, 1797, 3605},
        {2888, 1778, 3464}, {3916, 1767, 3426}, {363, 2085, 3623}, {1578, 2088, 3652},
        {2650, 2053, 3511}, {3629, 2028, 3400}, {422, 2353, 3583}, {1279, 2361, 3612},
        {2402, 2342, 3569}, {3379, 2288, 3408}, {479, 2609, 3538}, {996, 2615, 3553},
        {2126, 2638, 3613}, {3157, 2564, 3453}, {4095, 2560, 3460}, {760, 2856, 3494},
        {1837, 2873, 3528}, {2934, 2868, 3529}, {3972, 2825, 3450}, {636, 3084, 3438},
        {1593, 3092, 3450}, {2649, 3126, 3503}, {3697, 3085, 3440}, {683, 3305, 3390},
        {1406, 3305, 3391}, {2378, 3307, 3394}, {3543, 3453, 3540}, {723, 3667, 3475},
        {724, 3671, 3477}, {1989, 3699, 3494}, {3224, 3832, 3569}, {4095, 3764, 3442},
        {800, 4028, 3505}, {1398, 4061, 3515}, {2771, 4067, 3500}, {4031, 4026, 3347},
        {811, 4068, 3463}, {813, 4068, 3454}, {2218, 4067, 3400}, {3528, 4095, 3156},
        {343, 0, 3999}, {1334, 0, 3920}, {2366, 0, 3783}, {3360, 0, 3668}, {221, 0, 4001},
        {1073, 0, 3949}, {2111, 0, 3818}, {3116, 0, 3693}, {4095, 0, 3691}, {805, 0, 3973},
        {1854, 0, 3856}, {2870, 331, 3723}, {3882, 402, 3669}, {519, 408, 3990}, {1594, 535, 3895},
        {2624, 651, 3758}, {3606, 731, 3649}, {88, 843, 4000}, {1326, 881, 3932}, {2376, 937, 3799},
        {3368, 983, 3676}, {142, 1187, 3989}, {1043, 1193, 3963}, {2123, 1218, 3843},
        {3130, 1241, 3708}, {4095, 1232, 3688}, {725, 1497, 3988}, {1862, 1502, 3888},
        {2891, 1506, 3747}, {3881, 1509, 3661}, {317, 1794, 3958}, {1587, 1790, 3930},
        {2649, 1777, 3792}, {3624, 1769, 3662}, {356, 2078, 3925}, {1285, 2081, 3958},
        {2398, 2058, 3843}, {3393, 2031, 3696}, {416, 2352, 3889}, {974, 2356, 3907},
        {2131, 2348, 3896}, {3161, 2302, 3738}, {4095, 2285, 3688}, {659, 2618, 3855},
        {1838, 2631, 3902}, {2924, 2586, 3793}, {3893, 2540, 3661}, {529, 2869, 3802},
        {1551, 2880, 3830}, {2671, 2888, 3861}, {3646, 2801, 3671}, {585, 3108, 3748},
        {1299, 3115, 3761}, {2376, 3139, 3809}, {3429, 3088, 3728}, {641, 3337, 3696},
        {1108, 3339, 3700}, {2111, 3351, 3718}, {3209, 3416, 3815}, {4095, 3436, 3814},
        {1006, 3559, 3652}, {1902, 3566, 3659}, {2963, 3647, 3744}, {4095, 3760, 3853},
        {775, 3933, 3748}, {1400, 3959, 3769}, {2588, 4021, 3815}, {3782, 4059, 3765},
        {806, 4068, 3764}, {807, 4068, 3768}, {2038, 4067, 3765}, {3289, 4066, 3656},
        {266, 0, 4095}, {1083, 0, 4095}, {2120, 0, 4095}, {3128, 0, 3982}, {4095, 0, 3931},
        {824, 0, 4095}, {1862, 0, 4095}, {2880, 0, 4014}, {3860, 0, 3907}, {559, 0, 4095},
        {1601, 0, 4095}, {2630, 19, 4049}, {3619, 408, 3931}, {255, 253, 4095}, {1335, 414, 4095},
        {2379, 579, 4090}, {3378, 684, 3959}, {0, 801, 4095}, {1059, 828, 4095}, {2125, 894, 4095},
        {3137, 952, 3993}, {4095, 976, 3927}, {761, 1163, 4095}, {1865, 1191, 4095},
        {2895, 1221, 4033}, {3870, 1251, 3915}, {388, 1479, 4095}, {1594, 1485, 4095},
        {2649, 1494, 4077}, {3635, 1505, 3944}, {284, 1780, 4095}, {1303, 1778, 4095},
        {2397, 1772, 4095}, {3400, 1766, 3980}, {348, 2069, 4095}, {984, 2070, 4095},
        {2133, 2057, 4095}, {3164, 2035, 4022}, {4095, 2024, 3918}, {637, 2349, 4095},
        {1849, 2347, 4095}, {2921, 2311, 4071}, {3891, 2282, 3928}, {465, 2617, 4095},
        {1546, 2624, 4095}, {2668, 2598, 4095}, {3662, 2549, 3964}, {523, 2875, 4095},
        {1252, 2882, 4095}, {2391, 2895, 4095}, {3431, 2825, 4011}, {579, 3123, 4058},
        {984, 3127, 4068}, {2094, 3142, 4095}, {3192, 3118, 4073}, {4095, 3091, 3973},
        {777, 3362, 4008}, {1828, 3372, 4028}, {2918, 3405, 4095}, {4015, 3402, 4037},
        {698, 3589, 3954}, {1612, 3599, 3967}, {2675, 3664, 4047}, {3820, 3736, 4095},
        {796, 3841, 3940}, {1475, 3859, 3960}, {2471, 3914, 4017}, {3550, 3977, 4083},
        {801, 4068, 4018}, {801, 4068, 4027}, {1982, 4067, 4051}, {3102, 4066, 4040},
        {4095, 4095, 3793}, {839, 0, 4095}, {1870, 0, 4095}, {2891, 0, 4095}, {3875, 0, 4095},
        {591, 0, 4095}, {1611, 0, 4095}, {2639, 0, 4095}, {3632, 0, 4095}, {350, 0, 4095},
        {1347, 0, 4095}, {2386, 0, 4095}, {3389, 274, 4095}, {116, 0, 4095}, {1076, 195, 4095},
        {2130, 474, 4095}, {3144, 622, 4095}, {4095, 722, 4095}, {792, 767, 4095},
        {1870, 840, 4095}, {2899, 914, 4095}, {3882, 973, 4095}, {463, 1129, 4095},
        {1602, 1158, 4095}, {2651, 1196, 4095}, {3645, 1230, 4095}, {207, 1459, 4095},
        {1320, 1463, 4095}, {2398, 1478, 4095}, {3406, 1492, 4095}, {280, 1765, 4095},
        {1011, 1763, 4095}, {2136, 1763, 4095}, {3166, 1760, 4095}, {4095, 1765, 4095},
        {661, 2058, 4095}, {1859, 2051, 4095}, {2921, 2034, 4095}, {3903, 2023, 4095},
        {404, 2341, 4095}, {1559, 2343, 4095}, {2667, 2315, 4095}, {3670, 2287, 4095},
        {462, 2614, 4095}, {1248, 2618, 4095}, {2397, 2603, 4095}, {3433, 2560, 4095},
        {518, 2877, 4095}, {934, 2881, 4095}, {2102, 2890, 4095}, {3188, 2841, 4095},
        {4095, 2796, 4095}, {620, 3133, 4095}, {1807, 3143, 4095}, {2926, 3132, 4095},
        {3954, 3085, 4095}, {628, 3377, 4095}, {1535, 3384, 4095}, {2632, 3400, 4095},
        {3756, 3401, 4095}, {683, 3611, 4095}, {1308, 3620, 4095}, {2380, 3663, 4095},
        {3519, 3725, 4095}, {744, 3866, 4095}, {1163, 3877, 4095}, {2166, 3919, 4095},
        {3245, 3976, 4095}, {4095, 3921, 4095}, {1099, 4068, 4095}, {1978, 4067, 4095},
        {2994, 4066, 4095}, {3958, 4065, 4095}}};

bool build_test_3dlut(enum test3d_type type, struct vpe_3dlut *lut3d)
{
    if (!lut3d)
        return false;

    if (type == lut3d_identity) {
        memcpy(&(lut3d->lut_3d), &tetra_ident_3dlut, sizeof(struct tetrahedral_17x17x17));
    } else if (type == lut3d_sce) {
        memcpy(&(lut3d->lut_3d), &tetra_sce_3dlut, sizeof(struct tetrahedral_17x17x17));
    } else
        return false;

    lut3d->lut_3d.use_12bits = 1;
    lut3d->hdr_multiplier    = hdr_mult_sdr;

    lut3d->state.bits.initialized = 1;

    return true;
}
