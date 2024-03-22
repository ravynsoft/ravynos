/*
 * Copyright © 2020 Valve Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __FREEDRENO_GUARDBAND_H__
#define __FREEDRENO_GUARDBAND_H__

#include <assert.h>
#include <math.h>
#include <stdbool.h>

/* All 1's but don't overflow the GUARDBAND_CLIP_ADJ bitfields: */
#define MAX_GB 0x1ff

static inline unsigned
fd_calc_guardband(float offset, float scale, bool is_a3xx)
{
   /* On a3xx, the viewport max is 4k and the docs say the max guardband
    * width is 8k. That is, GRAS cannot handle triangle coordinates more than
    * 8k, positive or negative. On a4xx+ the viewport width was bumped to
    * 16k, and so the guardband width was necessarily also bumped. Note that
    * the numbers here should correspond to
    * VkPhysicalDeviceLimits::viewportBoundsRange in Vulkan.
    */
   const float gb_min = is_a3xx ? -8192. : -32768.;
   const float gb_max = is_a3xx ? 8191. : 32767.;

   /* Clipping happens in normalized device coordinates, so we have to
    * transform gb_min and gb_max to ndc using the inverse of the viewport
    * transform. Avoid flipping min and max by using the absolute value of
    * the scale.
    */
   const float gb_min_ndc = (gb_min - offset) / fabsf(scale);
   const float gb_max_ndc = (gb_max - offset) / fabsf(scale);

   /* There's only one GB_ADJ field, so presumably the guardband is
    * [-GB_ADJ, GB_ADJ] like on Radeon. It's always safe to make the
    * guardband smaller, so we have to take the min to get the largest range
    * contained in [gb_min_ndc, gb_max_ndc].
    */
   const float gb_adj = fminf(-gb_min_ndc, gb_max_ndc);

   /* The viewport should always be contained in the guardband. */
   if (gb_adj < 1.0)
      return MAX_GB;

   /* frexp returns an unspecified value if given an infinite value, which
    * can happen if scale == 0.
    */
   if (isinf(gb_adj))
      return MAX_GB;

   /* Convert gb_adj to 3.6 floating point, rounding down since it's always
    * safe to make the guard band smaller (but not the other way around!).
    *
    * Note: After converting back to a float, the value the blob returns here
    * is sometimes a little smaller than the value we return. This seems to
    * happen around the boundary between two different rounded values. For
    * example, using the a6xx blob:
    *
    * min  | width  | unrounded gb_adj | blob result | mesa result
    * ------------------------------------------------------------
    * 0    | 510    |          127.498 |        127. |        127.
    * 0    | 511    |          127.247 |        126. |        127.
    * 0    | 512    |          126.996 |        126. |        126.
    *
    * The guardband must be 32767 wide, since that's what the blob reports
    * for viewportBoundsRange, so I'm guessing that they're rounding slightly
    * more conservatively somehow.
    */
   int gb_adj_exp;
   float gb_adj_mantissa = frexpf(gb_adj, &gb_adj_exp);
   if (gb_adj_exp <= 0)
      return MAX_GB;

   /* Round non-representable numbers down to the largest possible number. */
   if (gb_adj_exp > 8)
      return MAX_GB;

   return ((gb_adj_exp - 1) << 6) |
          ((unsigned)truncf(gb_adj_mantissa * (1 << 7)) - (1 << 6));
}

#endif /* __FREEDRENO_GUARDBAND_H__ */
