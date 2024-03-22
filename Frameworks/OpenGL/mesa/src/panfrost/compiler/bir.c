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
 *
 * Authors (Collabora):
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "bi_builder.h"
#include "compiler.h"

bool
bi_has_arg(const bi_instr *ins, bi_index arg)
{
   if (!ins)
      return false;

   bi_foreach_src(ins, s) {
      if (bi_is_equiv(ins->src[s], arg))
         return true;
   }

   return false;
}

/* Precondition: valid 16-bit or 32-bit register format. Returns whether it is
 * 32-bit. Note auto reads to 32-bit registers even if the memory format is
 * 16-bit, so is considered as such here */

bool
bi_is_regfmt_16(enum bi_register_format fmt)
{
   switch (fmt) {
   case BI_REGISTER_FORMAT_F16:
   case BI_REGISTER_FORMAT_S16:
   case BI_REGISTER_FORMAT_U16:
      return true;
   case BI_REGISTER_FORMAT_F32:
   case BI_REGISTER_FORMAT_S32:
   case BI_REGISTER_FORMAT_U32:
   case BI_REGISTER_FORMAT_AUTO:
      return false;
   default:
      unreachable("Invalid register format");
   }
}

static unsigned
bi_count_staging_registers(const bi_instr *ins)
{
   enum bi_sr_count count = bi_opcode_props[ins->op].sr_count;
   unsigned vecsize = ins->vecsize + 1; /* XXX: off-by-one */

   switch (count) {
   case BI_SR_COUNT_0 ... BI_SR_COUNT_4:
      return count;
   case BI_SR_COUNT_FORMAT:
      return bi_is_regfmt_16(ins->register_format) ? DIV_ROUND_UP(vecsize, 2)
                                                   : vecsize;
   case BI_SR_COUNT_VECSIZE:
      return vecsize;
   case BI_SR_COUNT_SR_COUNT:
      return ins->sr_count;
   }

   unreachable("Invalid sr_count");
}

unsigned
bi_count_read_registers(const bi_instr *ins, unsigned s)
{
   /* ATOM reads 1 but writes 2. Exception for ACMPXCHG */
   if (s == 0 && ins->op == BI_OPCODE_ATOM_RETURN_I32)
      return (ins->atom_opc == BI_ATOM_OPC_ACMPXCHG) ? 2 : 1;
   else if (s == 0 && bi_opcode_props[ins->op].sr_read)
      return bi_count_staging_registers(ins);
   else if (s == 4 && ins->op == BI_OPCODE_BLEND)
      return ins->sr_count_2; /* Dual source blending */
   else if (s == 0 && ins->op == BI_OPCODE_SPLIT_I32)
      return ins->nr_dests;
   else
      return 1;
}

unsigned
bi_count_write_registers(const bi_instr *ins, unsigned d)
{
   if (d == 0 && bi_opcode_props[ins->op].sr_write) {
      switch (ins->op) {
      case BI_OPCODE_TEXC:
      case BI_OPCODE_TEXC_DUAL:
         if (ins->sr_count_2)
            return ins->sr_count;
         else
            return bi_is_regfmt_16(ins->register_format) ? 2 : 4;

      case BI_OPCODE_TEX_SINGLE:
      case BI_OPCODE_TEX_FETCH:
      case BI_OPCODE_TEX_GATHER: {
         unsigned chans = util_bitcount(ins->write_mask);

         return bi_is_regfmt_16(ins->register_format) ? DIV_ROUND_UP(chans, 2)
                                                      : chans;
      }

      case BI_OPCODE_ACMPXCHG_I32:
         /* Reads 2 but writes 1 */
         return 1;

      case BI_OPCODE_ATOM1_RETURN_I32:
         /* Allow omitting the destination for plain ATOM1 */
         return bi_is_null(ins->dest[0]) ? 0 : ins->sr_count;
      default:
         return bi_count_staging_registers(ins);
      }
   } else if (ins->op == BI_OPCODE_SEG_ADD_I64) {
      return 2;
   } else if (ins->op == BI_OPCODE_TEXC_DUAL && d == 1) {
      return ins->sr_count_2;
   } else if (ins->op == BI_OPCODE_COLLECT_I32 && d == 0) {
      return ins->nr_srcs;
   }

   return 1;
}

unsigned
bi_writemask(const bi_instr *ins, unsigned d)
{
   unsigned mask = BITFIELD_MASK(bi_count_write_registers(ins, d));
   unsigned shift = ins->dest[d].offset;
   return (mask << shift);
}

bi_clause *
bi_next_clause(bi_context *ctx, bi_block *block, bi_clause *clause)
{
   if (!block && !clause)
      return NULL;

   /* Try the first clause in this block if we're starting from scratch */
   if (!clause && !list_is_empty(&block->clauses))
      return list_first_entry(&block->clauses, bi_clause, link);

   /* Try the next clause in this block */
   if (clause && clause->link.next != &block->clauses)
      return list_first_entry(&(clause->link), bi_clause, link);

   /* Try the next block, or the one after that if it's empty, etc .*/
   bi_block *next_block = bi_next_block(block);

   bi_foreach_block_from(ctx, next_block, block) {
      if (!list_is_empty(&block->clauses))
         return list_first_entry(&block->clauses, bi_clause, link);
   }

   return NULL;
}

/* Does an instruction have a side effect not captured by its register
 * destination? Applies to certain message-passing instructions, +DISCARD, and
 * branching only, used in dead code elimation. Branches are characterized by
 * `last` which applies to them and some atomics, +BARRIER, +BLEND which
 * implies no loss of generality */

bool
bi_side_effects(const bi_instr *I)
{
   if (bi_opcode_props[I->op].last)
      return true;

   switch (I->op) {
   case BI_OPCODE_DISCARD_F32:
   case BI_OPCODE_DISCARD_B32:
      return true;
   default:
      break;
   }

   switch (bi_opcode_props[I->op].message) {
   case BIFROST_MESSAGE_NONE:
   case BIFROST_MESSAGE_VARYING:
   case BIFROST_MESSAGE_ATTRIBUTE:
   case BIFROST_MESSAGE_TEX:
   case BIFROST_MESSAGE_VARTEX:
   case BIFROST_MESSAGE_LOAD:
   case BIFROST_MESSAGE_64BIT:
      return false;

   case BIFROST_MESSAGE_STORE:
   case BIFROST_MESSAGE_ATOMIC:
   case BIFROST_MESSAGE_BARRIER:
   case BIFROST_MESSAGE_BLEND:
   case BIFROST_MESSAGE_Z_STENCIL:
   case BIFROST_MESSAGE_ATEST:
   case BIFROST_MESSAGE_JOB:
      return true;

   case BIFROST_MESSAGE_TILE:
      return (I->op != BI_OPCODE_LD_TILE);
   }

   unreachable("Invalid message type");
}

/* Branch reconvergence is required when the execution mask may change
 * between adjacent instructions (clauses). This occurs for conditional
 * branches and for the last instruction (clause) in a block whose
 * fallthrough successor has multiple predecessors.
 */

bool
bi_reconverge_branches(bi_block *block)
{
   if (bi_num_successors(block) == 1)
      return bi_num_predecessors(block->successors[0]) > 1;
   else
      return true;
}

/*
 * When MUX.i32 or MUX.v2i16 is used to multiplex entire sources, they can be
 * replaced by CSEL as follows:
 *
 *      MUX.neg(x, y, b) -> CSEL.s.lt(b, 0, x, y)
 *      MUX.int_zero(x, y, b) -> CSEL.i.eq(b, 0, x, y)
 *      MUX.fp_zero(x, y, b) -> CSEL.f.eq(b, 0, x, y)
 *
 * MUX.bit cannot be transformed like this.
 *
 * Note that MUX.v2i16 has partial support for swizzles, which CSEL.v2i16 lacks.
 * So we must check the swizzles too.
 */
bool
bi_can_replace_with_csel(bi_instr *I)
{
   return ((I->op == BI_OPCODE_MUX_I32) || (I->op == BI_OPCODE_MUX_V2I16)) &&
          (I->mux != BI_MUX_BIT) && (I->src[0].swizzle == BI_SWIZZLE_H01) &&
          (I->src[1].swizzle == BI_SWIZZLE_H01) &&
          (I->src[2].swizzle == BI_SWIZZLE_H01);
}

static enum bi_opcode
bi_csel_for_mux(bool must_sign, bool b32, enum bi_mux mux)
{
   switch (mux) {
   case BI_MUX_INT_ZERO:
      if (must_sign)
         return b32 ? BI_OPCODE_CSEL_U32 : BI_OPCODE_CSEL_V2U16;
      else
         return b32 ? BI_OPCODE_CSEL_I32 : BI_OPCODE_CSEL_V2I16;
   case BI_MUX_NEG:
      return b32 ? BI_OPCODE_CSEL_S32 : BI_OPCODE_CSEL_V2S16;
   case BI_MUX_FP_ZERO:
      return b32 ? BI_OPCODE_CSEL_F32 : BI_OPCODE_CSEL_V2F16;
   default:
      unreachable("No CSEL for MUX.bit");
   }
}

bi_instr *
bi_csel_from_mux(bi_builder *b, const bi_instr *I, bool must_sign)
{
   assert(I->op == BI_OPCODE_MUX_I32 || I->op == BI_OPCODE_MUX_V2I16);

   /* Build a new CSEL */
   enum bi_cmpf cmpf = (I->mux == BI_MUX_NEG) ? BI_CMPF_LT : BI_CMPF_EQ;
   bi_instr *csel = bi_csel_u32_to(b, I->dest[0], I->src[2], bi_zero(),
                                   I->src[0], I->src[1], cmpf);

   /* Fixup the opcode and use it */
   csel->op = bi_csel_for_mux(must_sign, I->op == BI_OPCODE_MUX_I32, I->mux);
   return csel;
}
