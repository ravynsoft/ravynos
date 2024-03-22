/*
 * Copyright © 2021 Intel Corporation
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
#ifndef INTEL_PIXEL_HASH_H
#define INTEL_PIXEL_HASH_H

/**
 * Compute an \p n x \p m pixel hashing table usable as slice, subslice or
 * pixel pipe hashing table.  The resulting table is the cyclic repetition of
 * a fixed pattern with periodicity equal to \p period.
 *
 * If \p index is specified to be equal to \p period, a 2-way hashing table
 * will be generated such that indices 0 and 1 are returned for the following
 * fractions of entries respectively:
 *
 *   p_0 = ceil(period / 2) / period
 *   p_1 = floor(period / 2) / period
 *
 * If \p index is even and less than \p period, a 3-way hashing table will be
 * generated such that indices 0, 1 and 2 are returned for the following
 * fractions of entries:
 *
 *   p_0 = (ceil(period / 2) - 1) / period
 *   p_1 = floor(period / 2) / period
 *   p_2 = 1 / period
 *
 * The equations above apply if \p flip is equal to 0, if it is equal to 1 p_0
 * and p_1 will be swapped for the result.  Note that in the context of pixel
 * pipe hashing this can be always 0 on Gfx12 platforms, since the hardware
 * transparently remaps logical indices found on the table to physical pixel
 * pipe indices from the highest to lowest EU count.
 */
UNUSED static void
intel_compute_pixel_hash_table_3way(unsigned n, unsigned m,
                                    unsigned period, unsigned index, bool flip,
                                    uint32_t *p)
{
   for (unsigned i = 0; i < n; i++) {
      for (unsigned j = 0; j < m; j++) {
         const unsigned k = (i + j) % period;
         p[j + m * i] = (k == index ? 2 : (k & 1) ^ flip);
      }
   }
}

/**
 * Compute an \p n x \p m pixel hashing table usable as slice,
 * subslice or pixel pipe hashing table.  This generalizes the
 * previous 3-way hash table function to an arbitrary number of ways
 * given by the number of bits set in the expression "mask1 | mask2".
 * If a way is only set in one of the two mask arguments it will
 * appear on the table with half the frequency as a way set on both
 * masks.
 */
UNUSED static void
intel_compute_pixel_hash_table_nway(unsigned n, unsigned m,
                                    uint32_t mask1, uint32_t mask2,
                                    uint32_t *p)
{
   /* If both masks are equal all ways are expected to show up with
    * the same frequency on the final table, so we can zero out one of
    * the masks in order to halve the number of IDs we need to handle.
    */
   if (mask1 == mask2)
      mask2 = 0;

   /* Construct a table mapping consecutive indices to the physical
    * indices given by the bits set on the mask arguments.  Ways
    * enabled on both masks will appear twice on the mapping, so
    * they'll show up with twice the frequency on the final table.
    */
   unsigned phys_ids[(sizeof(mask1) + sizeof(mask2)) * CHAR_BIT];
   unsigned num_ids = 0;

   for (unsigned i = 0; i < sizeof(mask1) * CHAR_BIT; i++) {
      if (mask1 & (1u << i))
         phys_ids[num_ids++] = i;
      if (mask2 & (1u << i))
         phys_ids[num_ids++] = i;
   }

   assert(num_ids > 0);

   /* Compute a permutation of the above indices that assigns indices
    * as far as possible to adjacent entries.  This permutation is
    * designed to be equivalent to the bit reversal of each index in
    * cases where num_ids is a power of two, but doesn't actually
    * require it to be a power of two in order to satisfy the required
    * properties (which is necessary to handle configurations with
    * arbitrary non-power of two fusing).  By construction, flipping
    * bit l of its input will lead to a change in its result of the
    * order of num_ids/2^(l+1) (see variable t below).  The
    * bijectivity of this permutation can be verified easily by
    * induction.  This permutation is applied cyclically to the
    * vertical indices of the hashing table constructed below.
    */
   const unsigned bits = util_logbase2_ceil(num_ids);
   unsigned swzy[ARRAY_SIZE(phys_ids)];

   for (unsigned k = 0; k < num_ids; k++) {
      unsigned t = num_ids;
      unsigned s = 0;

      for (unsigned l = 0; l < bits; l++) {
         if (k & (1u << l)) {
            s += (t + 1) >> 1;
            t >>= 1;
         } else {
            t = (t + 1) >> 1;
         }
      }

      swzy[k] = s;
   }

   /* Compute a second permutation applied cyclically to the
    * horizontal indices of the hashing table.  In cases where a
    * single mask is present (which means that all ways are expected
    * to have the same frequency) this permutation will be the
    * identity and will have no effect.
    *
    * In cases where some ways have twice the frequency of the others,
    * use a similar iterative halving of the range of the permutation
    * as in the the swzy[] permutation defined above, but instead of
    * scanning the bits of its argument (the "k" variable above) in
    * the opposite order (from LSB to MSB), proceed by halving the
    * domain of the permutation in the same order as its range, which
    * would lead to an identity permutation if it wasn't because the
    * LSB of its range is adjusted as early as possible instead of at
    * the last iteration.
    *
    * The reason for the special casing of the LSB is that we want to
    * avoid assigning adjacent IDs to adjacent elements of the table,
    * since ways that appear duplicated in the phys_ids mapping above
    * would then appear duplicated in adjacent positions of the final
    * table, which would lead to poor utilization for small primitives
    * that only cover a small contiguous portion of the hashing table
    * and would have twice as much work as necessary submitted to the
    * same way instead of spreading its processing over a larger
    * number of ways.
    */
   unsigned swzx[ARRAY_SIZE(phys_ids)];

   if (mask1 && mask2) {
      for (unsigned k = 0; k < num_ids; k++) {
         unsigned l = k;
         unsigned t = num_ids;
         unsigned s = 0;
         bool in_range = false;

         while (t > 1) {
            const bool first_in_range = t <= m && !in_range;
            in_range |= first_in_range;

            if (l >= (t + 1) >> 1) {
               /* Apply the s++ increment (which could only occur in
                * the last t == 2 iteration if we were constructing an
                * identity permutation) as soon as the domain of the
                * permutation has been decomposed into a chunk smaller
                * than the width of the hashing table \p m (which
                * causes in_range to be first set to true), since
                * doing it earlier would prevent any alternation
                * between even and odd indices in the first \p m
                * elements of swzx[], which are the only ones actually
                * used.
                *
                * Subsequent (in_range == true) increments of s need
                * to be doubled since they are selecting between
                * indices of the same parity.
                */
               if (!in_range)
                  s += (t + 1) >> 1;
               else if (first_in_range)
                  s++;
               else
                  s += (t + 1) >> 1 << 1;

               l -= (t + 1) >> 1;
               t >>= 1;
            } else {
               t = (t + 1) >> 1;
            }
         }

         swzx[k] = s;
      }
   } else {
      for (unsigned k = 0; k < num_ids; k++)
         swzx[k] = k;
   }

   /* Initialize the table with the cyclic repetition of a
    * num_ids-periodic pattern.
    *
    * Note that the horizontal and vertical permutations (swzx and
    * swzy respectively) are different, and the former is either an
    * identity permutation or close to the identity.  This asymmetry
    * is intentional in order to minimize the size of the contiguous
    * area that needs to be rendered in parallel in order to utilize
    * the whole GPU: In cases where swzx is the identity a rendering
    * rectangle of width W will need to be at least H blocks high,
    * where H is bounded by 2^ceil(log2(num_ids/W)) thanks to the
    * above definition of the swzy permutation.
    */
   for (unsigned i = 0; i < n; i++) {
      const unsigned k = i % num_ids;
      for (unsigned j = 0; j < m; j++) {
         const unsigned l = j % num_ids;
         p[j + m * i] = phys_ids[(swzx[l] + swzy[k]) % num_ids];
      }
   }
}

#endif
