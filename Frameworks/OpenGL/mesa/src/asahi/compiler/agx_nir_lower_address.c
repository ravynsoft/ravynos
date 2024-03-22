/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir_builder.h"
#include "agx_compiler.h"

/* Results of pattern matching */
struct match {
   nir_scalar base, offset;
   bool sign_extend;

   /* Signed shift. A negative shift indicates that the offset needs ushr
    * applied. It's cheaper to fold iadd and materialize an extra ushr, than
    * to leave the iadd untouched, so this is good.
    */
   int8_t shift;
};

/*
 * Try to match a multiplication with an immediate value. This generalizes to
 * both imul and ishl. If successful, returns true and sets the output
 * variables. Otherwise, returns false.
 */
static bool
match_imul_imm(nir_scalar scalar, nir_scalar *variable, uint32_t *imm)
{
   if (!nir_scalar_is_alu(scalar))
      return false;

   nir_op op = nir_scalar_alu_op(scalar);
   if (op != nir_op_imul && op != nir_op_ishl)
      return false;

   nir_scalar inputs[] = {
      nir_scalar_chase_alu_src(scalar, 0),
      nir_scalar_chase_alu_src(scalar, 1),
   };

   /* For imul check both operands for an immediate, since imul is commutative.
    * For ishl, only check the operand on the right.
    */
   bool commutes = (op == nir_op_imul);

   for (unsigned i = commutes ? 0 : 1; i < ARRAY_SIZE(inputs); ++i) {
      if (!nir_scalar_is_const(inputs[i]))
         continue;

      *variable = inputs[1 - i];

      uint32_t value = nir_scalar_as_uint(inputs[i]);

      if (op == nir_op_imul)
         *imm = value;
      else
         *imm = (1 << value);

      return true;
   }

   return false;
}

/*
 * Try to rewrite (a << (#b + #c)) + #d as ((a << #b) + #d') << #c,
 * assuming that #d is a multiple of 1 << #c. This takes advantage of
 * the hardware's implicit << #c and avoids a right-shift.
 *
 * Similarly, try to rewrite (a * (#b << #c)) + #d as ((a * #b) + #d') << #c.
 *
 * This pattern occurs with a struct-of-array layout.
 */
static bool
match_soa(nir_builder *b, struct match *match, unsigned format_shift)
{
   if (!nir_scalar_is_alu(match->offset) ||
       nir_scalar_alu_op(match->offset) != nir_op_iadd)
      return false;

   nir_scalar summands[] = {
      nir_scalar_chase_alu_src(match->offset, 0),
      nir_scalar_chase_alu_src(match->offset, 1),
   };

   for (unsigned i = 0; i < ARRAY_SIZE(summands); ++i) {
      if (!nir_scalar_is_const(summands[i]))
         continue;

      /* Note: This is treated as signed regardless of the sign of the match.
       * The final addition into the base can be signed or unsigned, but when
       * we shift right by the format shift below we need to always sign extend
       * to ensure that any negative offset remains negative when added into
       * the index. That is, in:
       *
       * addr = base + (u64)((index + offset) << shift)
       *
       * `index` and `offset` are always 32 bits, and a negative `offset` needs
       * to subtract from the index, so it needs to be sign extended when we
       * apply the format shift regardless of the fact that the later conversion
       * to 64 bits does not sign extend.
       *
       * TODO: We need to confirm how the hardware handles 32-bit overflow when
       * applying the format shift, which might need rework here again.
       */
      int offset = nir_scalar_as_int(summands[i]);
      nir_scalar variable;
      uint32_t multiplier;

      /* The other operand must multiply */
      if (!match_imul_imm(summands[1 - i], &variable, &multiplier))
         return false;

      int offset_shifted = offset >> format_shift;
      uint32_t multiplier_shifted = multiplier >> format_shift;

      /* If the multiplier or the offset are not aligned, we can't rewrite */
      if (multiplier != (multiplier_shifted << format_shift))
         return false;

      if (offset != (offset_shifted << format_shift))
         return false;

      /* Otherwise, rewrite! */
      nir_def *unmultiplied = nir_vec_scalars(b, &variable, 1);

      nir_def *rewrite = nir_iadd_imm(
         b, nir_imul_imm(b, unmultiplied, multiplier_shifted), offset_shifted);

      match->offset = nir_get_scalar(rewrite, 0);
      match->shift = 0;
      return true;
   }

   return false;
}

/* Try to pattern match address calculation */
static struct match
match_address(nir_builder *b, nir_scalar base, int8_t format_shift)
{
   struct match match = {.base = base};

   /* All address calculations are iadd at the root */
   if (!nir_scalar_is_alu(base) || nir_scalar_alu_op(base) != nir_op_iadd)
      return match;

   /* Only 64+32 addition is supported, look for an extension */
   nir_scalar summands[] = {
      nir_scalar_chase_alu_src(base, 0),
      nir_scalar_chase_alu_src(base, 1),
   };

   for (unsigned i = 0; i < ARRAY_SIZE(summands); ++i) {
      /* We can add a small constant to the 64-bit base for free */
      if (nir_scalar_is_const(summands[i]) &&
          nir_scalar_as_uint(summands[i]) < (1ull << 32)) {

         uint32_t value = nir_scalar_as_uint(summands[i]);

         return (struct match){
            .base = summands[1 - i],
            .offset = nir_get_scalar(nir_imm_int(b, value), 0),
            .shift = -format_shift,
            .sign_extend = false,
         };
      }

      /* Otherwise, we can only add an offset extended from 32-bits */
      if (!nir_scalar_is_alu(summands[i]))
         continue;

      nir_op op = nir_scalar_alu_op(summands[i]);

      if (op != nir_op_u2u64 && op != nir_op_i2i64)
         continue;

      /* We've found a summand, commit to it */
      match.base = summands[1 - i];
      match.offset = nir_scalar_chase_alu_src(summands[i], 0);
      match.sign_extend = (op == nir_op_i2i64);

      /* Undo the implicit shift from using as offset */
      match.shift = -format_shift;
      break;
   }

   /* If we didn't find something to fold in, there's nothing else we can do */
   if (!match.offset.def)
      return match;

   /* But if we did, we can try to fold in in a multiply */
   nir_scalar multiplied;
   uint32_t multiplier;

   if (match_imul_imm(match.offset, &multiplied, &multiplier)) {
      int8_t new_shift = match.shift;

      /* Try to fold in either a full power-of-two, or just the power-of-two
       * part of a non-power-of-two stride.
       */
      if (util_is_power_of_two_nonzero(multiplier)) {
         new_shift += util_logbase2(multiplier);
         multiplier = 1;
      } else if (((multiplier >> format_shift) << format_shift) == multiplier) {
         new_shift += format_shift;
         multiplier >>= format_shift;
      } else {
         return match;
      }

      nir_def *multiplied_ssa = nir_vec_scalars(b, &multiplied, 1);

      /* Only fold in if we wouldn't overflow the lsl field */
      if (new_shift <= 2) {
         match.offset =
            nir_get_scalar(nir_imul_imm(b, multiplied_ssa, multiplier), 0);
         match.shift = new_shift;
      } else if (new_shift > 0) {
         /* For large shifts, we do need a multiply, but we can
          * shrink the shift to avoid generating an ishr.
          */
         assert(new_shift >= 3);

         nir_def *rewrite =
            nir_imul_imm(b, multiplied_ssa, multiplier << new_shift);

         match.offset = nir_get_scalar(rewrite, 0);
         match.shift = 0;
      }
   } else {
      /* Try to match struct-of-arrays pattern, updating match if possible */
      match_soa(b, &match, format_shift);
   }

   return match;
}

static enum pipe_format
format_for_bitsize(unsigned bitsize)
{
   switch (bitsize) {
   case 8:
      return PIPE_FORMAT_R8_UINT;
   case 16:
      return PIPE_FORMAT_R16_UINT;
   case 32:
      return PIPE_FORMAT_R32_UINT;
   default:
      unreachable("should have been lowered");
   }
}

static bool
pass(struct nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_global &&
       intr->intrinsic != nir_intrinsic_load_global_constant &&
       intr->intrinsic != nir_intrinsic_global_atomic &&
       intr->intrinsic != nir_intrinsic_global_atomic_swap &&
       intr->intrinsic != nir_intrinsic_store_global)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   unsigned bitsize = intr->intrinsic == nir_intrinsic_store_global
                         ? nir_src_bit_size(intr->src[0])
                         : intr->def.bit_size;
   enum pipe_format format = format_for_bitsize(bitsize);
   unsigned format_shift = util_logbase2(util_format_get_blocksize(format));

   nir_src *orig_offset = nir_get_io_offset_src(intr);
   nir_scalar base = nir_scalar_resolved(orig_offset->ssa, 0);
   struct match match = match_address(b, base, format_shift);

   nir_def *offset = match.offset.def != NULL
                        ? nir_channel(b, match.offset.def, match.offset.comp)
                        : nir_imm_int(b, 0);

   /* If we were unable to fold in the shift, insert a right-shift now to undo
    * the implicit left shift of the instruction.
    */
   if (match.shift < 0) {
      if (match.sign_extend)
         offset = nir_ishr_imm(b, offset, -match.shift);
      else
         offset = nir_ushr_imm(b, offset, -match.shift);

      match.shift = 0;
   }

   /* Hardware offsets must be 32-bits. Upconvert if the source code used
    * smaller integers.
    */
   if (offset->bit_size != 32) {
      assert(offset->bit_size < 32);

      if (match.sign_extend)
         offset = nir_i2i32(b, offset);
      else
         offset = nir_u2u32(b, offset);
   }

   assert(match.shift >= 0);
   nir_def *new_base = nir_channel(b, match.base.def, match.base.comp);

   nir_def *repl = NULL;
   bool has_dest = (intr->intrinsic != nir_intrinsic_store_global);
   unsigned num_components = has_dest ? intr->def.num_components : 0;
   unsigned bit_size = has_dest ? intr->def.bit_size : 0;

   if (intr->intrinsic == nir_intrinsic_load_global) {
      repl =
         nir_load_agx(b, num_components, bit_size, new_base, offset,
                      .access = nir_intrinsic_access(intr), .base = match.shift,
                      .format = format, .sign_extend = match.sign_extend);

   } else if (intr->intrinsic == nir_intrinsic_load_global_constant) {
      repl = nir_load_constant_agx(b, num_components, bit_size, new_base,
                                   offset, .access = nir_intrinsic_access(intr),
                                   .base = match.shift, .format = format,
                                   .sign_extend = match.sign_extend);
   } else if (intr->intrinsic == nir_intrinsic_global_atomic) {
      offset = nir_ishl_imm(b, offset, match.shift);
      repl =
         nir_global_atomic_agx(b, bit_size, new_base, offset, intr->src[1].ssa,
                               .atomic_op = nir_intrinsic_atomic_op(intr),
                               .sign_extend = match.sign_extend);
   } else if (intr->intrinsic == nir_intrinsic_global_atomic_swap) {
      offset = nir_ishl_imm(b, offset, match.shift);
      repl = nir_global_atomic_swap_agx(
         b, bit_size, new_base, offset, intr->src[1].ssa, intr->src[2].ssa,
         .atomic_op = nir_intrinsic_atomic_op(intr),
         .sign_extend = match.sign_extend);
   } else {
      nir_store_agx(b, intr->src[0].ssa, new_base, offset,
                    .access = nir_intrinsic_access(intr), .base = match.shift,
                    .format = format, .sign_extend = match.sign_extend);
   }

   if (repl)
      nir_def_rewrite_uses(&intr->def, repl);

   nir_instr_remove(&intr->instr);
   return true;
}

bool
agx_nir_lower_address(nir_shader *shader)
{
   return nir_shader_intrinsics_pass(
      shader, pass, nir_metadata_block_index | nir_metadata_dominance, NULL);
}
