/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_builder.h"
#include "agx_compiler.h"

/*
 * Lower 64-bit moves to 32-bit moves. Although there are not 64-bit moves in
 * the ISA, it is convenient to pretend there are for instruction selection.
 * They are lowered trivially after register allocation.
 *
 * General 64-bit lowering happens in nir_lower_int64.
 */
static bool
lower(agx_builder *b, agx_instr *I)
{
   if (I->op != AGX_OPCODE_MOV && I->op != AGX_OPCODE_MOV_IMM)
      return false;

   if (I->dest[0].size != AGX_SIZE_64)
      return false;

   agx_index dest = I->dest[0];
   dest.size = AGX_SIZE_32;

   if (I->op == AGX_OPCODE_MOV) {
      assert(I->src[0].type == AGX_INDEX_REGISTER ||
             I->src[0].type == AGX_INDEX_UNIFORM);
      assert(I->src[0].size == AGX_SIZE_64);
      agx_index src = I->src[0];
      src.size = AGX_SIZE_32;

      /* Low 32-bit */
      agx_mov_to(b, dest, src);

      /* High 32-bits */
      dest.value += 2;
      src.value += 2;
      agx_mov_to(b, dest, src);
   } else {
      /* Low 32-bit */
      agx_mov_imm_to(b, dest, I->imm & BITFIELD_MASK(32));

      /* High 32-bits */
      dest.value += 2;
      agx_mov_imm_to(b, dest, I->imm >> 32);
   }

   return true;
}

void
agx_lower_64bit_postra(agx_context *ctx)
{
   agx_foreach_instr_global_safe(ctx, I) {
      agx_builder b = agx_init_builder(ctx, agx_before_instr(I));

      if (lower(&b, I))
         agx_remove_instruction(I);
   }
}
