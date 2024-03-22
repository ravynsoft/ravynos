/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _DOUBLE_H_
#define _DOUBLE_H_

#include "half_float.h"
#include "u_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This API is no more than a wrapper to the counterpart softfloat.h
 * calls. Still, softfloat.h conversion API is meant to be kept private. In
 * other words, only use the API published here, instead of calling directly
 * the softfloat.h one.
 */

float _mesa_double_to_float(double val);
float _mesa_double_to_float_rtz(double val);

static inline float
_mesa_double_to_float_rtne(double val)
{
   return _mesa_double_to_float(val);
}

/*
 * We round down from double to half float by going through float in between,
 * but this can give us inaccurate results in some cases.
 * One such case is 0x40ee6a0000000001, which should round to 0x7b9b, but
 * going through float first turns into 0x7b9a instead. This is because the
 * first non-fitting bit is set, so we get a tie, but with the least
 * significant bit of the original number set, the tie should break rounding
 * up.
 * The cast to float, however, turns into 0x47735000, which when going to half
 * still ties, but now we lost the tie-up bit, and instead we round to the
 * nearest even, which in this case is down.
 *
 * To fix this, we check if the original would have tied, and if the tie would
 * have rounded up, and if both are true, set the least significant bit of the
 * intermediate float to 1, so that a tie on the next cast rounds up as well.
 * If the rounding already got rid of the tie, that set bit will just be
 * truncated anyway and the end result doesn't change.
 *
 * Another failing case is 0x40effdffffffffff. This one doesn't have the tie
 * from double to half, so it just rounds down to 0x7bff (65504.0), but going
 * through float first, it turns into 0x477ff000, which does have the tie bit
 * for half set, and when that one gets rounded it turns into 0x7c00
 * (Infinity).
 * The fix for that one is to make sure the intermediate float does not have
 * the tie bit set if the original didn't have it.
 */
static inline uint16_t
_mesa_double_to_float16_rtne(double val)
{
   int significand_bits16 = 10;
   int significand_bits32 = 23;
   int significand_bits64 = 52;
   int f64_to_16_tie_bit = significand_bits64 - significand_bits16 - 1;
   int f32_to_16_tie_bit = significand_bits32 - significand_bits16 - 1;
   uint64_t f64_rounds_up_mask = ((1ULL << f64_to_16_tie_bit) - 1);

   union di src;
   union fi dst;

   src.d = val;
   dst.f = val;

   bool f64_has_tie = (src.ui & (1ULL << f64_to_16_tie_bit)) != 0;
   bool f64_rounds_up = (src.ui & f64_rounds_up_mask) != 0;

   dst.ui |= (f64_has_tie && f64_rounds_up);
   if (!f64_has_tie)
      dst.ui &= ~(1U << f32_to_16_tie_bit);

   return _mesa_float_to_float16_rtne(dst.f);
}

/*
 * double -> float -> half with RTZ doesn't have as many complications as
 * RTNE, but we do need to ensure that the double -> float cast also uses RTZ.
 */
static inline uint16_t
_mesa_double_to_float16_rtz(double val)
{
   return _mesa_float_to_float16_rtz(_mesa_double_to_float_rtz(val));
}

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _DOUBLE_H_ */
