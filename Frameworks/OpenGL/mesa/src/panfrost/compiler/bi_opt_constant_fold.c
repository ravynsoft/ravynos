/*
 * Copyright (C) 2020 Collabora Ltd.
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
#include "compiler.h"

/* Dead simple constant folding to cleanup compiler frontend patterns. Before
 * adding a new pattern here, check why you need it and whether we can avoid
 * generating the constant BIR at all. */

static inline uint32_t
bi_source_value(const bi_instr *I, unsigned s)
{
   if (s < I->nr_srcs)
      return bi_apply_swizzle(I->src[s].value, I->src[s].swizzle);
   else
      return 0;
}

uint32_t
bi_fold_constant(bi_instr *I, bool *unsupported)
{
   /* We can only fold instructions where all sources are constant */
   bi_foreach_src(I, s) {
      if (I->src[s].type != BI_INDEX_CONSTANT) {
         *unsupported = true;
         return 0;
      }
   }

   /* Grab the sources */
   uint32_t a = bi_source_value(I, 0);
   uint32_t b = bi_source_value(I, 1);
   uint32_t c = bi_source_value(I, 2);
   uint32_t d = bi_source_value(I, 3);

   /* Evaluate the instruction */
   switch (I->op) {
   case BI_OPCODE_SWZ_V2I16:
      return a;

   case BI_OPCODE_MKVEC_V2I16:
      return (b << 16) | (a & 0xFFFF);

   case BI_OPCODE_MKVEC_V4I8:
      return (d << 24) | ((c & 0xFF) << 16) | ((b & 0xFF) << 8) | (a & 0xFF);

   case BI_OPCODE_MKVEC_V2I8:
      return (c << 16) | ((b & 0xFF) << 8) | (a & 0xFF);

   case BI_OPCODE_LSHIFT_OR_I32:
      if (I->not_result || I->src[0].neg || I->src[1].neg)
         break;

      return (a << c) | b;

   case BI_OPCODE_F32_TO_U32:
      if (I->round == BI_ROUND_NONE) {
         /* Explicitly clamp to prevent undefined behaviour and
          * match hardware rules */
         float f = uif(a);
         return (f >= 0.0) ? (uint32_t)f : 0;
      } else
         break;

   default:
      break;
   }

   *unsupported = true;
   return 0;
}

bool
bi_opt_constant_fold(bi_context *ctx)
{
   bool progress = false;

   bi_foreach_instr_global_safe(ctx, ins) {
      bool unsupported = false;
      uint32_t replace = bi_fold_constant(ins, &unsupported);
      if (unsupported)
         continue;

      /* Replace with constant move, to be copypropped */
      assert(ins->nr_dests == 1);
      bi_builder b = bi_init_builder(ctx, bi_after_instr(ins));
      bi_mov_i32_to(&b, ins->dest[0], bi_imm_u32(replace));
      bi_remove_instruction(ins);
      progress = true;
   }

   return progress;
}
