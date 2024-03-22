/*
 * Copyright (C) 2019 Google.
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

#include "util/ralloc.h"

#include "ir3.h"

static bool
is_safe_conv(struct ir3_instruction *instr, type_t src_type, opc_t *src_opc)
{
   if (instr->opc != OPC_MOV)
      return false;

   /* Only allow half->full or full->half without any type conversion (like
    * int to float).
    */
   if (type_size(instr->cat1.src_type) == type_size(instr->cat1.dst_type) ||
       full_type(instr->cat1.src_type) != full_type(instr->cat1.dst_type))
      return false;

   /* mul.s24/u24 always return 32b result regardless of its sources size,
    * hence we cannot guarantee the high 16b of dst being zero or sign extended.
    */
   if ((*src_opc == OPC_MUL_S24 || *src_opc == OPC_MUL_U24) &&
       type_size(instr->cat1.src_type) == 16)
      return false;

   struct ir3_register *dst = instr->dsts[0];
   struct ir3_register *src = instr->srcs[0];

   /* disallow conversions that cannot be folded into
    * alu instructions:
    */
   if (instr->cat1.round != ROUND_ZERO)
      return false;

   if (dst->flags & (IR3_REG_RELATIV | IR3_REG_ARRAY))
      return false;
   if (src->flags & (IR3_REG_RELATIV | IR3_REG_ARRAY))
      return false;

   /* Check that the source of the conv matches the type of the src
    * instruction.
    */
   if (src_type == instr->cat1.src_type)
      return true;

   /* We can handle mismatches with integer types by converting the opcode
    * but not when an integer is reinterpreted as a float or vice-versa.
    */
   if (type_float(src_type) != type_float(instr->cat1.src_type))
      return false;

   /* We have types with mismatched signedness. Mismatches on the signedness
    * don't matter when narrowing:
    */
   if (type_size(instr->cat1.dst_type) < type_size(instr->cat1.src_type))
      return true;

   /* Try swapping the opcode: */
   bool can_swap = true;
   *src_opc = ir3_try_swap_signedness(*src_opc, &can_swap);
   return can_swap;
}

static bool
all_uses_safe_conv(struct ir3_instruction *conv_src, type_t src_type)
{
   opc_t opc = conv_src->opc;
   bool first = true;
   foreach_ssa_use (use, conv_src) {
      opc_t new_opc = opc;
      if (!is_safe_conv(use, src_type, &new_opc))
         return false;
      /* Check if multiple uses have conflicting requirements on the opcode.
       */
      if (!first && opc != new_opc)
         return false;
      first = false;
      opc = new_opc;
   }
   conv_src->opc = opc;
   return true;
}

/* For an instruction which has a conversion folded in, re-write the
 * uses of *all* conv's that used that src to be a simple mov that
 * cp can eliminate.  This avoids invalidating the SSA uses, it just
 * shifts the use to a simple mov.
 */
static void
rewrite_src_uses(struct ir3_instruction *src)
{
   foreach_ssa_use (use, src) {
      assert(use->opc == OPC_MOV);

      if (is_half(src)) {
         use->srcs[0]->flags |= IR3_REG_HALF;
      } else {
         use->srcs[0]->flags &= ~IR3_REG_HALF;
      }

      use->cat1.src_type = use->cat1.dst_type;
   }
}

static bool
try_conversion_folding(struct ir3_instruction *conv)
{
   struct ir3_instruction *src;

   if (conv->opc != OPC_MOV)
      return false;

   /* NOTE: we can have non-ssa srcs after copy propagation: */
   src = ssa(conv->srcs[0]);
   if (!src)
      return false;

   if (!is_alu(src))
      return false;

   bool can_fold;
   type_t base_type = ir3_output_conv_type(src, &can_fold);
   if (!can_fold)
      return false;

   type_t src_type = ir3_output_conv_src_type(src, base_type);
   type_t dst_type = ir3_output_conv_dst_type(src, base_type);

   /* Avoid cases where we've already folded in a conversion. We assume that
    * if there is a chain of conversions that's foldable then it's been
    * folded in NIR already.
    */
   if (src_type != dst_type)
      return false;

   if (!all_uses_safe_conv(src, src_type))
      return false;

   ir3_set_dst_type(src, is_half(conv));
   rewrite_src_uses(src);

   return true;
}

bool
ir3_cf(struct ir3 *ir)
{
   void *mem_ctx = ralloc_context(NULL);
   bool progress = false;

   ir3_find_ssa_uses(ir, mem_ctx, false);

   foreach_block (block, &ir->block_list) {
      foreach_instr (instr, &block->instr_list) {
         progress |= try_conversion_folding(instr);
      }
   }

   ralloc_free(mem_ctx);

   return progress;
}
