/**************************************************************************
 *
 * Copyright 2011 Christian König
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "vl_zscan_data.h"

const int vl_zscan_normal_16[] =
{
   /* Zig-Zag scan pattern */
    0, 1, 4, 8, 5, 2, 3, 6,
    9,12,13,10, 7,11,14,15
};

const int vl_zscan_linear[] =
{
   /* Linear scan pattern */
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9,10,11,12,13,14,15,
   16,17,18,19,20,21,22,23,
   24,25,26,27,28,29,30,31,
   32,33,34,35,36,37,38,39,
   40,41,42,43,44,45,46,47,
   48,49,50,51,52,53,54,55,
   56,57,58,59,60,61,62,63
};

const int vl_zscan_normal[] =
{
   /* Zig-Zag scan pattern */
    0, 1, 8,16, 9, 2, 3,10,
   17,24,32,25,18,11, 4, 5,
   12,19,26,33,40,48,41,34,
   27,20,13, 6, 7,14,21,28,
   35,42,49,56,57,50,43,36,
   29,22,15,23,30,37,44,51,
   58,59,52,45,38,31,39,46,
   53,60,61,54,47,55,62,63
};

const int vl_zscan_alternate[] =
{
   /* Alternate scan pattern */
    0, 8,16,24, 1, 9, 2,10,
   17,25,32,40,48,56,57,49,
   41,33,26,18, 3,11, 4,12,
   19,27,34,42,50,58,35,43,
   51,59,20,28, 5,13, 6,14,
   21,29,36,44,52,60,37,45,
   53,61,22,30, 7,15,23,31,
   38,46,54,62,39,47,55,63
};

const int vl_zscan_h265_up_right_diagonal_16[] =
{
   /* Up-right diagonal scan order for 4x4 blocks - see H.265 section 6.5.3. */
    0,  4,  1,  8,  5,  2, 12,  9,
    6,  3, 13, 10,  7, 14, 11, 15,
};

const int vl_zscan_h265_up_right_diagonal[] =
{
   /* Up-right diagonal scan order for 8x8 blocks - see H.265 section 6.5.3. */
    0,  8,  1, 16,  9,  2, 24, 17,
   10,  3, 32, 25, 18, 11,  4, 40,
   33, 26, 19, 12,  5, 48, 41, 34,
   27, 20, 13,  6, 56, 49, 42, 35,
   28, 21, 14,  7, 57, 50, 43, 36,
   29, 22, 15, 58, 51, 44, 37, 30,
   23, 59, 52, 45, 38, 31, 60, 53,
   46, 39, 61, 54, 47, 62, 55, 63,
};
