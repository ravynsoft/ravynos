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

#include <stdbool.h>
#include <stdint.h>

union custom_float_format_flags2 {
    unsigned int Uint;
    struct {
        unsigned int sign       : 1;
        unsigned int reserved31 : 31;
    } bits;
};
struct vpe_custom_float_format2 {
    unsigned int                     mantissaBits;
    unsigned int                     exponentaBits;
    union custom_float_format_flags2 flags;
};

struct vpe_custom_float_value2 {
    unsigned int mantissa;
    unsigned int exponenta;
    int          value;
    bool         isNegative;
};

bool vpe_convert_from_float_to_custom_float(
    double value, const struct vpe_custom_float_format2 *fmt, uint16_t *pvalue);
bool vpe_convert_from_float_to_fp16(double value, uint16_t *pvalue);

bool vpe_convert_to_custom_float_generic(
    double value, const struct vpe_custom_float_format2 *fmt, int *pvalue);

bool vpe_convert_to_custom_float_ex_generic(double value,
    const struct vpe_custom_float_format2 *fmt, struct vpe_custom_float_value2 *pvalue);

bool vpe_from_1_6_12_to_double(
    bool bIsNegative, unsigned int E, unsigned int F, double *DoubleFloat);
