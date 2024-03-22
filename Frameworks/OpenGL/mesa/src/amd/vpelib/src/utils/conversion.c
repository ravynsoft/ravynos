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
#include "conversion.h"

#define DIVIDER 10000

/* S2D13 value in [-3.99...3.9999] */
#define S2D13_MIN ((long long)(-3.999 * DIVIDER))
#define S2D13_MAX ((long long)(3.999 * DIVIDER))

#define FRACTIONAL_PART_MASK ((1ULL << FIXED31_32_BITS_PER_FRACTIONAL_PART) - 1)

#define GET_INTEGER_PART(x) ((x) >> FIXED31_32_BITS_PER_FRACTIONAL_PART)

#define GET_FRACTIONAL_PART(x) (FRACTIONAL_PART_MASK & (x))

uint16_t conv_fixed_point_to_int_frac(
    struct fixed31_32 arg, uint8_t integer_bits, uint8_t fractional_bits)
{
    int32_t numerator;
    int32_t divisor = 1 << fractional_bits;

    uint16_t result;

    uint16_t d = (uint16_t)vpe_fixpt_floor(vpe_fixpt_abs(arg));

    if (d <= (uint16_t)(1 << integer_bits) - (1 / (uint16_t)divisor))
        numerator = (uint16_t)vpe_fixpt_round(vpe_fixpt_mul_int(arg, divisor));
    else {
        numerator = vpe_fixpt_floor(vpe_fixpt_sub(
            vpe_fixpt_from_int(1LL << integer_bits), vpe_fixpt_recip(vpe_fixpt_from_int(divisor))));
    }

    if (numerator >= 0)
        result = (uint16_t)numerator;
    else
        result = (uint16_t)((1 << (integer_bits + fractional_bits + 1)) + numerator);

    if ((result != 0) && vpe_fixpt_lt(arg, vpe_fixpt_zero))
        result |= 1 << (integer_bits + fractional_bits);

    return result;
}

void conv_convert_float_matrix(uint16_t *matrix, const struct fixed31_32 *flt, uint32_t buffer_size)
{
    const struct fixed31_32 min_2_13 = vpe_fixpt_from_fraction(S2D13_MIN, DIVIDER);
    const struct fixed31_32 max_2_13 = vpe_fixpt_from_fraction(S2D13_MAX, DIVIDER);
    uint32_t                i;

    for (i = 0; i < buffer_size; ++i) {
        uint32_t reg_value =
            conv_fixed_point_to_int_frac(vpe_fixpt_clamp(flt[i], min_2_13, max_2_13), 2, 13);

        matrix[i] = (uint16_t)reg_value;
    }
}

struct fixed31_32 vpe_convfix31_32(int16_t inval)
{
    const int         integerBits    = 2;
    const int         fractionalBits = 13;
    struct fixed31_32 result;
    long long         outintegerPart;
    long long         outfractionalPart;
    int               sign = 1;
    if (inval & (1 << (integerBits + fractionalBits + 1))) {
        sign  = -1;
        inval = -inval;
    }
    outintegerPart    = ((long long)inval >> fractionalBits) << 32;
    outfractionalPart = ((long long)inval & (((long long)1 << fractionalBits) - 1));
    ;
    outfractionalPart <<= (32 - fractionalBits);
    result.value = outintegerPart | outfractionalPart;
    if (sign < 0)
        result.value = -result.value;
    return result;
}
