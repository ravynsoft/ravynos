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

#include "va_compiler.h"

/* Valhall specific instruction selection optimizations */

static enum bi_opcode
va_op_add_imm(enum bi_opcode op)
{
   switch (op) {
   case BI_OPCODE_FADD_F32:
      return BI_OPCODE_FADD_IMM_F32;
   case BI_OPCODE_FADD_V2F16:
      return BI_OPCODE_FADD_IMM_V2F16;
   case BI_OPCODE_IADD_S32:
   case BI_OPCODE_IADD_U32:
      return BI_OPCODE_IADD_IMM_I32;
   case BI_OPCODE_IADD_V2S16:
   case BI_OPCODE_IADD_V2U16:
      return BI_OPCODE_IADD_IMM_V2I16;
   case BI_OPCODE_IADD_V4S8:
   case BI_OPCODE_IADD_V4U8:
      return BI_OPCODE_IADD_IMM_V4I8;
   default:
      return 0;
   }
}

static bool
va_is_add_imm(bi_instr *I, unsigned s)
{
   assert(s < I->nr_srcs);

   return I->src[s].swizzle == BI_SWIZZLE_H01 && !I->src[s].abs &&
          !I->src[s].neg && !I->clamp && !I->round;
}

static unsigned
va_choose_imm(bi_instr *I)
{
   for (unsigned i = 0; i < 2; ++i) {
      if (I->src[i].type == BI_INDEX_CONSTANT)
         return i;
   }

   return ~0;
}

/* Lower MOV.i32 #constant --> IADD_IMM.i32 0x0, #constant */
static void
va_lower_mov_imm(bi_instr *I)
{
   assert(I->nr_srcs == 1);

   if (I->src[0].type == BI_INDEX_CONSTANT) {
      I->op = BI_OPCODE_IADD_IMM_I32;
      I->index = I->src[0].value;
      I->src[0] = bi_zero();
   }
}

void
va_fuse_add_imm(bi_instr *I)
{
   if (I->op == BI_OPCODE_MOV_I32) {
      va_lower_mov_imm(I);
      return;
   }

   enum bi_opcode op = va_op_add_imm(I->op);
   if (!op)
      return;

   unsigned s = va_choose_imm(I);
   if (s > 1)
      return;
   if (!va_is_add_imm(I, 1 - s))
      return;

   I->op = op;
   I->index = bi_apply_swizzle(I->src[s].value, I->src[s].swizzle);

   assert(!I->src[s].abs && "redundant .abs set");

   /* If the constant is negated, flip the sign bit */
   if (I->src[s].neg) {
      if (I->op == BI_OPCODE_FADD_IMM_F32)
         I->index ^= (1u << 31);
      else if (I->op == BI_OPCODE_FADD_IMM_V2F16)
         I->index ^= (1u << 31) | (1u << 15);
      else
         unreachable("unexpected .neg");
   }

   I->src[0] = I->src[1 - s];
   bi_drop_srcs(I, 1);
}

void
va_optimize(bi_context *ctx)
{
   bi_foreach_instr_global(ctx, I) {
      va_fuse_add_imm(I);
   }
}
