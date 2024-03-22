/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 *    Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "compiler.h"

/* Midgard's generic load/store instructions, particularly to implement SSBOs
 * and globals, have support for address arithmetic natively. In particularly,
 * they take two indirect arguments A, B and two immediates #s, #c, calculating
 * the address:
 *
 *      A + (zext?(B) << #s) + #c
 *
 * This allows for fast indexing into arrays. This file tries to pattern match
 * the offset in NIR with this form to reduce pressure on the ALU pipe.
 */

struct mir_address {
   nir_scalar A;
   nir_scalar B;

   midgard_index_address_format type;
   unsigned shift;
   unsigned bias;
};

static bool
mir_args_ssa(nir_scalar s, unsigned count)
{
   nir_alu_instr *alu = nir_instr_as_alu(s.def->parent_instr);

   if (count > nir_op_infos[alu->op].num_inputs)
      return false;

   return true;
}

/* Matches a constant in either slot and moves it to the bias */

static void
mir_match_constant(struct mir_address *address)
{
   if (address->A.def && nir_scalar_is_const(address->A)) {
      address->bias += nir_scalar_as_uint(address->A);
      address->A.def = NULL;
   }

   if (address->B.def && nir_scalar_is_const(address->B)) {
      address->bias += nir_scalar_as_uint(address->B);
      address->B.def = NULL;
   }
}

/* Matches an iadd when there is a free slot or constant */

/* The offset field is a 18-bit signed integer */
#define MAX_POSITIVE_OFFSET ((1 << 17) - 1)

static void
mir_match_iadd(struct mir_address *address, bool first_free)
{
   if (!address->B.def || !nir_scalar_is_alu(address->B))
      return;

   if (!mir_args_ssa(address->B, 2))
      return;

   nir_op op = nir_scalar_alu_op(address->B);

   if (op != nir_op_iadd)
      return;

   nir_scalar op1 = nir_scalar_chase_alu_src(address->B, 0);
   nir_scalar op2 = nir_scalar_chase_alu_src(address->B, 1);

   if (nir_scalar_is_const(op1) &&
       nir_scalar_as_uint(op1) <= MAX_POSITIVE_OFFSET) {
      address->bias += nir_scalar_as_uint(op1);
      address->B = op2;
   } else if (nir_scalar_is_const(op2) &&
              nir_scalar_as_uint(op2) <= MAX_POSITIVE_OFFSET) {
      address->bias += nir_scalar_as_uint(op2);
      address->B = op1;
   } else if (!nir_scalar_is_const(op1) && !nir_scalar_is_const(op2) &&
              first_free && !address->A.def) {
      address->A = op1;
      address->B = op2;
   }
}

/* Matches u2u64 and sets type */

static void
mir_match_u2u64(struct mir_address *address)
{
   if (!address->B.def || !nir_scalar_is_alu(address->B))
      return;

   if (!mir_args_ssa(address->B, 1))
      return;

   nir_op op = nir_scalar_alu_op(address->B);
   if (op != nir_op_u2u64)
      return;
   nir_scalar arg = nir_scalar_chase_alu_src(address->B, 0);

   address->B = arg;
   address->type = midgard_index_address_u32;
}

/* Matches i2i64 and sets type */

static void
mir_match_i2i64(struct mir_address *address)
{
   if (!address->B.def || !nir_scalar_is_alu(address->B))
      return;

   if (!mir_args_ssa(address->B, 1))
      return;

   nir_op op = nir_scalar_alu_op(address->B);
   if (op != nir_op_i2i64)
      return;
   nir_scalar arg = nir_scalar_chase_alu_src(address->B, 0);

   address->B = arg;
   address->type = midgard_index_address_s32;
}

/* Matches ishl to shift */

static void
mir_match_ishl(struct mir_address *address)
{
   if (!address->B.def || !nir_scalar_is_alu(address->B))
      return;

   if (!mir_args_ssa(address->B, 2))
      return;

   nir_op op = nir_scalar_alu_op(address->B);
   if (op != nir_op_ishl)
      return;
   nir_scalar op1 = nir_scalar_chase_alu_src(address->B, 0);
   nir_scalar op2 = nir_scalar_chase_alu_src(address->B, 1);

   if (!nir_scalar_is_const(op2))
      return;

   unsigned shift = nir_scalar_as_uint(op2);
   if (shift > 0x7)
      return;

   address->B = op1;
   address->shift = shift;
}

/* Strings through mov which can happen from NIR vectorization */

static void
mir_match_mov(struct mir_address *address)
{
   if (address->A.def && nir_scalar_is_alu(address->A)) {
      nir_op op = nir_scalar_alu_op(address->A);

      if (op == nir_op_mov && mir_args_ssa(address->A, 1))
         address->A = nir_scalar_chase_alu_src(address->A, 0);
   }

   if (address->B.def && nir_scalar_is_alu(address->B)) {
      nir_op op = nir_scalar_alu_op(address->B);

      if (op == nir_op_mov && mir_args_ssa(address->B, 1))
         address->B = nir_scalar_chase_alu_src(address->B, 0);
   }
}

/* Tries to pattern match into mir_address */

static struct mir_address
mir_match_offset(nir_def *offset, bool first_free, bool extend)
{
   struct mir_address address = {
      .B = {.def = offset},
      .type = extend ? midgard_index_address_u64 : midgard_index_address_u32,
   };

   mir_match_mov(&address);
   mir_match_constant(&address);
   mir_match_mov(&address);
   mir_match_iadd(&address, first_free);
   mir_match_mov(&address);

   if (extend) {
      mir_match_u2u64(&address);
      mir_match_i2i64(&address);
      mir_match_mov(&address);
   }

   mir_match_ishl(&address);

   return address;
}

void
mir_set_offset(compiler_context *ctx, midgard_instruction *ins, nir_src *offset,
               unsigned seg)
{
   for (unsigned i = 0; i < 16; ++i) {
      ins->swizzle[1][i] = 0;
      ins->swizzle[2][i] = 0;
   }

   /* Sign extend instead of zero extend in case the address is something
    * like `base + offset + 20`, where offset could be negative. */
   bool force_sext = (nir_src_bit_size(*offset) < 64);
   bool first_free = (seg == LDST_GLOBAL);

   struct mir_address match = mir_match_offset(offset->ssa, first_free, true);

   if (match.A.def) {
      unsigned bitsize = match.A.def->bit_size;
      assert(bitsize == 32 || bitsize == 64);

      ins->src[1] = nir_ssa_index(match.A.def);
      ins->swizzle[1][0] = match.A.comp;
      ins->src_types[1] = nir_type_uint | bitsize;
      ins->load_store.bitsize_toggle = (bitsize == 64);
   } else {
      ins->load_store.bitsize_toggle = true;
      ins->load_store.arg_comp = seg & 0x3;
      ins->load_store.arg_reg = (seg >> 2) & 0x7;
   }

   if (match.B.def) {
      ins->src[2] = nir_ssa_index(match.B.def);
      ins->swizzle[2][0] = match.B.comp;
      ins->src_types[2] = nir_type_uint | match.B.def->bit_size;
   } else
      ins->load_store.index_reg = REGISTER_LDST_ZERO;

   if (force_sext)
      match.type = midgard_index_address_s32;

   ins->load_store.index_format = match.type;

   assert(match.shift <= 7);
   ins->load_store.index_shift = match.shift;

   ins->constants.u32[0] = match.bias;
}

void
mir_set_ubo_offset(midgard_instruction *ins, nir_src *src, unsigned bias)
{
   struct mir_address match = mir_match_offset(src->ssa, false, false);

   if (match.B.def) {
      ins->src[2] = nir_ssa_index(match.B.def);

      for (unsigned i = 0; i < ARRAY_SIZE(ins->swizzle[2]); ++i)
         ins->swizzle[2][i] = match.B.comp;
   }

   ins->load_store.index_shift = match.shift;
   ins->constants.u32[0] = match.bias + bias;
}
