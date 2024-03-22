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

#include "vpe_assert.h"
#include "custom_fp16.h"
#include <math.h>
#include <stdbool.h>

static bool build_custom_float(double value, const struct vpe_custom_float_format2 *fmt,
    bool *pbNegative, unsigned int *pexponent, unsigned int *pmantissa)
{
    bool         bRet             = false;
    double       mantissaConstant = 1.0;
    double       base             = 2.0;
    int          expOffset        = (int)(pow(2.0, fmt->exponentaBits - 1) - 1);
    double       maxFraction      = 1.0 - pow(0.5, fmt->mantissaBits);
    unsigned int exponent;
    double       mantissa;
    unsigned int Mantissa;
    int          i;
    bool         bAlwaysFalse = false;

    // if value negative and we should consider this otherwize just ignore
    if (value < 0 && fmt->flags.bits.sign == 1)
        *pbNegative = true;
    else
        *pbNegative = false;

    do {

        if (value == 0.0) {
            *pexponent = 0;
            *pmantissa = 0;
            bRet       = true;
            break;
        }

        if (value < 0)
            value = (-1.0) * value;

        if (value < mantissaConstant) {
            /*if log works faster then remove the loops!*/
            for (i = 1;; i++) {
                value *= base;
                if (value >= mantissaConstant)
                    break;
            }
            if (expOffset <= i) {
                *pexponent = 0;
                *pmantissa = 0;
                bRet       = true;
                break;
            }
            exponent = (unsigned int)(expOffset - i);
        } else if (value >= mantissaConstant + maxFraction) {
            for (i = 1;; i++) {
                value /= base;
                if (value <= mantissaConstant + maxFraction)
                    break;
            }
            exponent = (unsigned int)(expOffset + i);
        } else
            exponent = (unsigned int)expOffset;

        mantissa = value - mantissaConstant;

        if (mantissa < 0.0 || mantissa > 1.0)
            mantissa = 0;
        else
            mantissa *= pow(2.0, fmt->mantissaBits);

        Mantissa = (unsigned int)mantissa;

        *pexponent = exponent;
        *pmantissa = Mantissa;

        bRet = true;

    } while (bAlwaysFalse);

    return bRet;
}

static bool setup_custom_float(const struct vpe_custom_float_format2 *fmt, bool bNegative,
    unsigned int exponent, unsigned int mantissa, uint16_t *pvalue)
{
    unsigned int value = 0;
    unsigned int mask;
    unsigned int i;
    unsigned int j;

    if (fmt->exponentaBits == 6 && fmt->mantissaBits == 12 && fmt->flags.bits.sign == 0) {
        if (exponent & ~(unsigned int)0x3F)
            exponent = 0x3F;
        if (mantissa & ~(unsigned int)0xFFF)
            mantissa = 0xFFF;
    } else if (fmt->exponentaBits == 6 && fmt->mantissaBits == 10 && fmt->flags.bits.sign == 0) {
        if (exponent & ~(unsigned int)0x3F)
            exponent = 0x3F;
        if (mantissa & ~(unsigned int)0x3FF)
            mantissa = 0x3FF;
    } else if (fmt->exponentaBits == 6 && fmt->mantissaBits == 12 && fmt->flags.bits.sign == 1) {
        if (exponent & ~(unsigned int)0x3F)
            exponent = 0x3F;
        if (mantissa & ~(unsigned int)0xFFF)
            mantissa = 0xFFF;
    } else if (fmt->exponentaBits == 5 && fmt->mantissaBits == 10 && fmt->flags.bits.sign == 1) {
        if (exponent & ~(unsigned int)0x1F)
            exponent = 0x1F;
        if (mantissa & ~(unsigned int)0x3FF)
            mantissa = 0x3FF;
    } else
        return false;

    for (i = 0; i < fmt->mantissaBits; i++) {
        mask = 1 << i;

        if (mantissa & mask)
            value |= mask;
    }
    for (j = 0; j < fmt->exponentaBits; j++) {
        mask = 1 << j;

        if (exponent & mask)
            value |= mask << i;
    }

    if (bNegative == true && fmt->flags.bits.sign == 1)
        value |= 1 << (i + j);

    *pvalue = (uint16_t)value;

    return true;
}

bool vpe_convert_from_float_to_custom_float(
    double value, const struct vpe_custom_float_format2 *fmt, uint16_t *pvalue)
{
    bool         isNegative;
    unsigned int exponent;
    unsigned int mantissa;
    bool         ret = false;

    VPE_ASSERT(
        (fmt->flags.bits.sign == 1) && (fmt->mantissaBits == 10) && (fmt->exponentaBits == 5));

    if (!build_custom_float(value, fmt, &isNegative, &exponent, &mantissa))
        goto release;
    if (!setup_custom_float(fmt, isNegative, exponent, mantissa, pvalue))
        goto release;
    ret = true;
release:
    return ret;
}

static bool setup_custom_float_generic(const struct vpe_custom_float_format2 *fmt, bool bNegative,
    unsigned int exponent, unsigned int mantissa, int *pvalue)
{
    unsigned int value = 0;

    int mask;

    if (fmt->exponentaBits == 6 && fmt->mantissaBits == 12 && fmt->flags.bits.sign == 0) {
        if (exponent & ~0x3F)
            exponent = 0x3F;
        if (mantissa & ~0xFFF)
            mantissa = 0xFFF;
    } else if (fmt->exponentaBits == 6 && fmt->mantissaBits == 10 && fmt->flags.bits.sign == 0) {
        if (exponent & ~0x3F)
            exponent = 0x3F;
        if (mantissa & ~0x3FF)
            mantissa = 0x3FF;

    } else if (fmt->exponentaBits == 6 && fmt->mantissaBits == 12 && fmt->flags.bits.sign == 1) {
        if (exponent & ~0x3F)
            exponent = 0x3F;
        if (mantissa & ~0xFFF)
            mantissa = 0xFFF;

    } else
        return false;

    unsigned int i;
    unsigned int j;

    for (i = 0; i < fmt->mantissaBits; i++) {
        mask = 1 << i;

        if (mantissa & mask)
            value |= mask;
    }
    for (j = 0; j < fmt->exponentaBits; j++) {
        mask = 1 << j;

        if (exponent & mask)
            value |= mask << i;
    }

    if (bNegative == true && fmt->flags.bits.sign == 1)
        value |= 1 << (i + j);

    *pvalue = value;

    return true;
}

bool vpe_convert_to_custom_float_generic(
    double value, const struct vpe_custom_float_format2 *fmt, int *pvalue)
{
    bool         isNegative;
    unsigned int exponent;
    unsigned int mantissa;

    bool ret = false;

    if (!build_custom_float(value, fmt, &isNegative, &exponent, &mantissa))
        goto release;
    if (!setup_custom_float_generic(fmt, isNegative, exponent, mantissa, pvalue))
        goto release;
    ret = true;
release:
    return ret;
}

bool vpe_convert_to_custom_float_ex_generic(double value,
    const struct vpe_custom_float_format2 *fmt, struct vpe_custom_float_value2 *pvalue)
{
    bool ret = false;

    if (!build_custom_float(value, fmt, &pvalue->isNegative, &pvalue->exponenta, &pvalue->mantissa))
        goto release;
    if (!setup_custom_float_generic(
            fmt, pvalue->isNegative, pvalue->exponenta, pvalue->mantissa, &pvalue->value))
        goto release;

    ret = true;
release:
    return ret;
}

bool vpe_from_1_6_12_to_double(
    bool bIsNegative, unsigned int E, unsigned int F, double *DoubleFloat)
{
    double ret = 0;

    double M, F1, A, B, C, D2, e12;

    A   = 2.0;
    B   = 31.0;
    C   = 1.0;
    D2  = -30.0;
    e12 = pow(2, 12);

    M = F / e12;

    if (bIsNegative == false)
        F1 = 1.0;
    else
        F1 = -1.0;

    if (E > 0 && E < 63)
        ret = F1 * (C + M) * pow(A, E - B);
    else if (E == 0 && F != 0)
        ret = F1 * M * pow(A, D2);
    else if (E == 0 && F == 0 && bIsNegative == true)
        ret = -0;
    else if (E == 0 && F == 0 && bIsNegative == false)
        ret = 0;
    else if (E == 63 && F != 0)
        return false; // -1; /* Not a number*/
    else if (E == 63 && F == 0 && bIsNegative == true)
        return false; //-2; /* -Infinity*/
    else if (E == 63 && F == 0 && bIsNegative == false)
        return false; // -3; /* Infinity	*/

    *DoubleFloat = ret;

    return true;
}

bool vpe_convert_from_float_to_fp16(double value, uint16_t *pvalue)
{
    struct vpe_custom_float_format2 fmt;

    fmt.flags.Uint      = 0;
    fmt.flags.bits.sign = 1;
    fmt.mantissaBits    = 10;
    fmt.exponentaBits   = 5;

    return vpe_convert_from_float_to_custom_float(value, &fmt, pvalue);
}
