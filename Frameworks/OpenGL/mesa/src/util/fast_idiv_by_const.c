/*
 * Copyright © 2018 Advanced Micro Devices, Inc.
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

/* Imported from:
 *   https://raw.githubusercontent.com/ridiculousfish/libdivide/master/divide_by_constants_codegen_reference.c
 * Paper:
 *   http://ridiculousfish.com/files/faster_unsigned_division_by_constants.pdf
 *
 * The author, ridiculous_fish, wrote:
 *
 *  ''Reference implementations of computing and using the "magic number"
 *    approach to dividing by constants, including codegen instructions.
 *    The unsigned division incorporates the "round down" optimization per
 *    ridiculous_fish.
 *
 *    This is free and unencumbered software. Any copyright is dedicated
 *    to the Public Domain.''
 */

#include "fast_idiv_by_const.h"
#include "u_math.h"
#include "util/macros.h"
#include <limits.h>
#include <assert.h>

struct util_fast_udiv_info
util_compute_fast_udiv_info(uint64_t D, unsigned num_bits, unsigned UINT_BITS)
{
   /* The numerator must fit in a uint64_t */
   assert(num_bits > 0 && num_bits <= UINT_BITS);
   assert(D != 0);

   /* The eventual result */
   struct util_fast_udiv_info result;

   if (util_is_power_of_two_or_zero64(D)) {
      unsigned div_shift = util_logbase2_64(D);

      if (div_shift) {
         /* Dividing by a power of two. */
         result.multiplier = 1ull << (UINT_BITS - div_shift);
         result.pre_shift = 0;
         result.post_shift = 0;
         result.increment = 0;
         return result;
      } else {
         /* Dividing by 1. */
         /* Assuming: floor((num + 1) * (2^32 - 1) / 2^32) = num */
         result.multiplier = u_uintN_max(UINT_BITS);
         result.pre_shift = 0;
         result.post_shift = 0;
         result.increment = 1;
         return result;
      }
   }

   /* The extra shift implicit in the difference between UINT_BITS and num_bits
    */
   const unsigned extra_shift = UINT_BITS - num_bits;

   /* The initial power of 2 is one less than the first one that can possibly
    * work.
    */
   const uint64_t initial_power_of_2 = (uint64_t)1 << (UINT_BITS-1);

   /* The remainder and quotient of our power of 2 divided by d */
   uint64_t quotient = initial_power_of_2 / D;
   uint64_t remainder = initial_power_of_2 % D;

   /* ceil(log_2 D) */
   unsigned ceil_log_2_D;

   /* The magic info for the variant "round down" algorithm */
   uint64_t down_multiplier = 0;
   unsigned down_exponent = 0;
   int has_magic_down = 0;

   /* Compute ceil(log_2 D) */
   ceil_log_2_D = 0;
   uint64_t tmp;
   for (tmp = D; tmp > 0; tmp >>= 1)
      ceil_log_2_D += 1;


   /* Begin a loop that increments the exponent, until we find a power of 2
    * that works.
    */
   unsigned exponent;
   for (exponent = 0; ; exponent++) {
      /* Quotient and remainder is from previous exponent; compute it for this
       * exponent.
       */
      if (remainder >= D - remainder) {
         /* Doubling remainder will wrap around D */
         quotient = quotient * 2 + 1;
         remainder = remainder * 2 - D;
      } else {
         /* Remainder will not wrap */
         quotient = quotient * 2;
         remainder = remainder * 2;
      }

      /* We're done if this exponent works for the round_up algorithm.
       * Note that exponent may be larger than the maximum shift supported,
       * so the check for >= ceil_log_2_D is critical.
       */
      if ((exponent + extra_shift >= ceil_log_2_D) ||
          (D - remainder) <= ((uint64_t)1 << (exponent + extra_shift)))
         break;

      /* Set magic_down if we have not set it yet and this exponent works for
       * the round_down algorithm
       */
      if (!has_magic_down &&
          remainder <= ((uint64_t)1 << (exponent + extra_shift))) {
         has_magic_down = 1;
         down_multiplier = quotient;
         down_exponent = exponent;
      }
   }

   if (exponent < ceil_log_2_D) {
      /* magic_up is efficient */
      result.multiplier = quotient + 1;
      result.pre_shift = 0;
      result.post_shift = exponent;
      result.increment = 0;
   } else if (D & 1) {
      /* Odd divisor, so use magic_down, which must have been set */
      assert(has_magic_down);
      result.multiplier = down_multiplier;
      result.pre_shift = 0;
      result.post_shift = down_exponent;
      result.increment = 1;
   } else {
      /* Even divisor, so use a prefix-shifted dividend */
      unsigned pre_shift = 0;
      uint64_t shifted_D = D;
      while ((shifted_D & 1) == 0) {
         shifted_D >>= 1;
         pre_shift += 1;
      }
      result = util_compute_fast_udiv_info(shifted_D, num_bits - pre_shift,
                                           UINT_BITS);
      /* expect no increment or pre_shift in this path */
      assert(result.increment == 0 && result.pre_shift == 0);
      result.pre_shift = pre_shift;
   }
   return result;
}

struct util_fast_sdiv_info
util_compute_fast_sdiv_info(int64_t D, unsigned SINT_BITS)
{
   /* D must not be zero. */
   assert(D != 0);
   /* The result is not correct for these divisors. */
   assert(D != 1 && D != -1);

   /* Our result */
   struct util_fast_sdiv_info result;

   /* Absolute value of D (we know D is not the most negative value since
    * that's a power of 2)
    */
   const uint64_t abs_d = (D < 0 ? -D : D);

   /* The initial power of 2 is one less than the first one that can possibly
    * work */
   /* "two31" in Warren */
   unsigned exponent = SINT_BITS - 1;
   const uint64_t initial_power_of_2 = (uint64_t)1 << exponent;

   /* Compute the absolute value of our "test numerator,"
    * which is the largest dividend whose remainder with d is d-1.
    * This is called anc in Warren.
    */
   const uint64_t tmp = initial_power_of_2 + (D < 0);
   const uint64_t abs_test_numer = tmp - 1 - tmp % abs_d;

   /* Initialize our quotients and remainders (q1, r1, q2, r2 in Warren) */
   uint64_t quotient1 = initial_power_of_2 / abs_test_numer;
   uint64_t remainder1 = initial_power_of_2 % abs_test_numer;
   uint64_t quotient2 = initial_power_of_2 / abs_d;
   uint64_t remainder2 = initial_power_of_2 % abs_d;
   uint64_t delta;

   /* Begin our loop */
   do {
      /* Update the exponent */
      exponent++;

      /* Update quotient1 and remainder1 */
      quotient1 *= 2;
      remainder1 *= 2;
      if (remainder1 >= abs_test_numer) {
         quotient1 += 1;
         remainder1 -= abs_test_numer;
      }

      /* Update quotient2 and remainder2 */
      quotient2 *= 2;
      remainder2 *= 2;
      if (remainder2 >= abs_d) {
         quotient2 += 1;
         remainder2 -= abs_d;
      }

      /* Keep going as long as (2**exponent) / abs_d <= delta */
      delta = abs_d - remainder2;
   } while (quotient1 < delta || (quotient1 == delta && remainder1 == 0));

   result.multiplier = util_sign_extend(quotient2 + 1, SINT_BITS);
   if (D < 0) result.multiplier = -result.multiplier;
   result.shift = exponent - SINT_BITS;
   return result;
}
