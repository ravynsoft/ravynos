
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
#include "valhall.h"

void
va_count_instr_stats(bi_instr *I, struct va_stats *stats)
{
   /* Adjusted for 64-bit arithmetic */
   unsigned words = bi_count_write_registers(I, 0);

   switch (valhall_opcodes[I->op].unit) {
   /* Arithmetic is 2x slower for 64-bit than 32-bit */
   case VA_UNIT_FMA:
      stats->fma += words;
      return;

   case VA_UNIT_CVT:
      stats->cvt += words;
      return;

   case VA_UNIT_SFU:
      stats->sfu += words;
      return;

   /* Varying is scaled by 16-bit components interpolated */
   case VA_UNIT_V:
      stats->v +=
         (I->vecsize + 1) * (bi_is_regfmt_16(I->register_format) ? 1 : 2);
      return;

   /* We just count load/store and texturing for now */
   case VA_UNIT_LS:
      stats->ls++;
      return;

   case VA_UNIT_T:
      stats->t++;
      return;

   /* Fused varying+texture loads 2 FP32 components of varying for texture
    * coordinates and then textures */
   case VA_UNIT_VT:
      stats->ls += (2 * 2);
      stats->t++;
      return;

   /* Nothing to do here */
   case VA_UNIT_NONE:
      return;
   }

   unreachable("Invalid unit");
}
