/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_UTIL_H
#define PVR_UTIL_H

#include <assert.h>
#include <stdint.h>

#include "pvr_types.h"

#include "util/bitscan.h"
#include "util/macros.h"

static inline bool pvr_dev_addr_is_aligned(pvr_dev_addr_t addr,
                                           const uint32_t alignment)
{
   assert(util_is_power_of_two_nonzero(alignment));
   return ((uintptr_t)(addr.addr) & (alignment - 1)) == 0;
}

static inline bool ptr_is_aligned(const void *const ptr,
                                  const uint32_t alignment)
{
   assert(util_is_power_of_two_nonzero(alignment));
   return ((uintptr_t)(ptr) & (alignment - 1)) == 0;
}

/*****************************************************************************
  Math functions
*****************************************************************************/

static inline uint32_t u32_bin_digits(uint32_t x)
{
   return x == 0 ? 1 : util_last_bit(x);
}

static inline uint32_t u64_bin_digits(uint64_t x)
{
   return x == 0 ? 1 : util_last_bit64(x);
}

extern const uint8_t est_log10_from_log2[64 + 1];
extern const uint32_t u32_powers_of_ten[10];
extern const uint64_t u64_powers_of_ten[20];

/*
 * This function includes parts of a public-domain log10 algorithm from
 * "Bit Twiddling Hacks", an aggregate collection which is:
 *
 * © 1997-2005 Sean Eron Anderson.
 *
 * https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
 */
static inline uint32_t u32_dec_digits(uint32_t x)
{
   /* The value of nr_digits is an estimation of ceil(log10(x)) with variance
    * +0/-1 (as per the comment on the estimation lut).
    */
   const uint32_t nr_digits = est_log10_from_log2[util_last_bit(x)];

   /* We then convert this approximation to a real value by adding 1 iff
    * x >= 10**nr_digits using another lut to quickly compute the
    * exponentiation. See the comment on u32_powers_of_ten for details of this.
    */
   return nr_digits + (x >= u32_powers_of_ten[nr_digits]);
}

/* This function is an extended form of u32_dec_digits(); see the comments on
 * that function for details.
 */
static inline uint32_t u64_dec_digits(uint64_t x)
{
   const uint32_t nr_digits = est_log10_from_log2[util_last_bit64(x)];
   return nr_digits + (x >= u64_powers_of_ten[nr_digits]);
}

static inline uint32_t u32_hex_digits(uint32_t x)
{
   const uint32_t binary_digits = u32_bin_digits(x);
   return (binary_digits >> 2) + !!(binary_digits & 3);
}

static inline uint32_t u64_hex_digits(uint64_t x)
{
   const uint32_t binary_digits = u64_bin_digits(x);
   return (binary_digits >> 2) + !!(binary_digits & 3);
}

#endif /* PVR_UTIL_H */
