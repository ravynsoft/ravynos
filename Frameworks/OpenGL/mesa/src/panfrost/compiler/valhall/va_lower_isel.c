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

static bi_instr *
lower(bi_builder *b, bi_instr *I)
{
   switch (I->op) {

   /* Integer addition has swizzles and addition with 0 is canonical swizzle */
   case BI_OPCODE_SWZ_V2I16:
      return bi_iadd_v2u16_to(b, I->dest[0], I->src[0], bi_zero(), false);

   case BI_OPCODE_SWZ_V4I8:
      return bi_iadd_v4u8_to(b, I->dest[0], I->src[0], bi_zero(), false);

   case BI_OPCODE_ICMP_I32:
      return bi_icmp_or_u32_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                               I->cmpf, I->result_type);

   case BI_OPCODE_ICMP_V2I16:
      return bi_icmp_or_v2u16_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                                 I->cmpf, I->result_type);

   case BI_OPCODE_ICMP_V4I8:
      return bi_icmp_or_v4u8_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                                I->cmpf, I->result_type);

   case BI_OPCODE_ICMP_U32:
      return bi_icmp_or_u32_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                               I->cmpf, I->result_type);

   case BI_OPCODE_ICMP_V2U16:
      return bi_icmp_or_v2u16_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                                 I->cmpf, I->result_type);

   case BI_OPCODE_ICMP_V4U8:
      return bi_icmp_or_v4u8_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                                I->cmpf, I->result_type);

   case BI_OPCODE_ICMP_S32:
      return bi_icmp_or_s32_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                               I->cmpf, I->result_type);

   case BI_OPCODE_ICMP_V2S16:
      return bi_icmp_or_v2s16_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                                 I->cmpf, I->result_type);

   case BI_OPCODE_ICMP_V4S8:
      return bi_icmp_or_v4s8_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                                I->cmpf, I->result_type);

   case BI_OPCODE_FCMP_F32:
      return bi_fcmp_or_f32_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                               I->cmpf, I->result_type);

   case BI_OPCODE_FCMP_V2F16:
      return bi_fcmp_or_v2f16_to(b, I->dest[0], I->src[0], I->src[1], bi_zero(),
                                 I->cmpf, I->result_type);

   /* Integer CSEL must have a signedness */
   case BI_OPCODE_CSEL_I32:
   case BI_OPCODE_CSEL_V2I16:
      assert(I->cmpf == BI_CMPF_EQ || I->cmpf == BI_CMPF_NE);

      I->op = (I->op == BI_OPCODE_CSEL_I32) ? BI_OPCODE_CSEL_U32
                                            : BI_OPCODE_CSEL_V2U16;
      return NULL;

   /* Jump -> conditional branch with condition tied to true. */
   case BI_OPCODE_JUMP:
      if (I->branch_target) {
         bi_instr *new_I = bi_branchz_i16(b, bi_zero(), I->src[0], BI_CMPF_EQ);
         new_I->branch_target = I->branch_target;
         return I;
      } else {
         return bi_branchzi(b, bi_zero(), I->src[0], BI_CMPF_EQ);
      }

   case BI_OPCODE_AXCHG_I32:
      I->op = BI_OPCODE_ATOM_RETURN_I32;
      I->atom_opc = BI_ATOM_OPC_AXCHG;
      I->sr_count = 1;
      return NULL;

   case BI_OPCODE_ACMPXCHG_I32:
      I->op = BI_OPCODE_ATOM_RETURN_I32;
      I->atom_opc = BI_ATOM_OPC_ACMPXCHG;
      /* Reads 2, this is special cased in bir.c */
      I->sr_count = 1;
      return NULL;

   case BI_OPCODE_ATOM_RETURN_I32:
      if (bi_is_null(I->dest[0]))
         I->op = BI_OPCODE_ATOM_I32;

      return NULL;

   case BI_OPCODE_MUX_I32:
   case BI_OPCODE_MUX_V2I16:
      if (bi_can_replace_with_csel(I))
         return bi_csel_from_mux(b, I, true);

      return NULL;

   case BI_OPCODE_FADD_RSCALE_F32:
      return bi_fma_rscale_f32_to(b, I->dest[0], I->src[0], bi_imm_f32(1.0),
                                  I->src[1], I->src[2], I->special);

   default:
      return NULL;
   }
}

void
va_lower_isel(bi_context *ctx)
{
   bi_foreach_instr_global_safe(ctx, I) {
      bi_builder b = bi_init_builder(ctx, bi_before_instr(I));

      if (lower(&b, I))
         bi_remove_instruction(I);
   }
}
