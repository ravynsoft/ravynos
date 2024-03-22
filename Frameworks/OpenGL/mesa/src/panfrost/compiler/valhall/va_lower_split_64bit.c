/*
 * Copyright (C) 2021 Collabora Ltd.
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

#include "bi_builder.h"
#include "va_compiler.h"

/*
 * Bifrost uses split 64-bit addresses, specified as two consecutive sources.
 * Valhall uses contiguous 64-bit addresses, specified as a single source with
 * an aligned register pair. This simple pass inserts moves to lower split
 * 64-bit sources into a collect, ensuring the register constraints will be
 * respected by RA. This could be optimized, but this is deferred for now.
 */
static void
lower_split_src(bi_context *ctx, bi_instr *I, unsigned s)
{
   /* Skip sources that are already split properly */
   bi_index offset_fau = I->src[s];
   offset_fau.offset++;

   if (I->src[s].type == BI_INDEX_FAU && I->src[s].offset == 0 &&
       bi_is_value_equiv(offset_fau, I->src[s + 1])) {
      return;
   }

   /* Allocate temporary before the instruction */
   bi_builder b = bi_init_builder(ctx, bi_before_instr(I));
   bi_index vec = bi_temp(ctx);
   bi_instr *collect = bi_collect_i32_to(&b, vec, 2);
   bi_instr *split = bi_split_i32_to(&b, 2, vec);

   /* Emit collect */
   for (unsigned w = 0; w < 2; ++w) {
      collect->src[w] = I->src[s + w];

      split->dest[w] = bi_temp(ctx);
      I->src[s + w] = split->dest[w];
   }
}

void
va_lower_split_64bit(bi_context *ctx)
{
   bi_foreach_instr_global(ctx, I) {
      bi_foreach_src(I, s) {
         if (bi_is_null(I->src[s]) || s >= 4)
            continue;

         struct va_src_info info = va_src_info(I->op, s);

         if (info.size == VA_SIZE_64)
            lower_split_src(ctx, I, s);
      }
   }
}
