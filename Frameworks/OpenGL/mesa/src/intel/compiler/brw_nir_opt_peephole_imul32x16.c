/*
 * Copyright © 2022 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "brw_nir.h"
#include "compiler/nir/nir_builder.h"

/**
 * Implement a peephole pass to convert integer multiplications to imul32x16.
 */

struct pass_data {
   struct hash_table *range_ht;
};

static void
replace_imul_instr(nir_builder *b, nir_alu_instr *imul, unsigned small_val,
                   nir_op new_opcode)
{
   assert(small_val == 0 || small_val == 1);

   b->cursor = nir_before_instr(&imul->instr);

   nir_alu_instr *imul_32x16 = nir_alu_instr_create(b->shader, new_opcode);

   nir_alu_src_copy(&imul_32x16->src[0], &imul->src[1 - small_val]);
   nir_alu_src_copy(&imul_32x16->src[1], &imul->src[small_val]);

   nir_def_init(&imul_32x16->instr, &imul_32x16->def,
                imul->def.num_components, 32);

   nir_def_rewrite_uses(&imul->def,
                            &imul_32x16->def);

   nir_builder_instr_insert(b, &imul_32x16->instr);

   nir_instr_remove(&imul->instr);
   nir_instr_free(&imul->instr);
}

enum root_operation {
   non_unary = 0,
   integer_neg = 1 << 0,
   integer_abs = 1 << 1,
   integer_neg_abs = integer_neg | integer_abs,
   invalid_root = 255
};

static enum root_operation
signed_integer_range_analysis(nir_shader *shader, struct hash_table *range_ht,
                              nir_scalar scalar, int *lo, int *hi)
{
   if (nir_scalar_is_const(scalar)) {
      *lo = nir_scalar_as_int(scalar);
      *hi = *lo;
      return non_unary;
   }

   if (nir_scalar_is_alu(scalar)) {
      switch (nir_scalar_alu_op(scalar)) {
      case nir_op_iabs:
         signed_integer_range_analysis(shader, range_ht,
                                       nir_scalar_chase_alu_src(scalar, 0),
                                       lo, hi);

         if (*lo == INT32_MIN) {
            *hi = INT32_MAX;
         } else {
            const int32_t a = abs(*lo);
            const int32_t b = abs(*hi);

            *lo = MIN2(a, b);
            *hi = MAX2(a, b);
         }

         /* Absolute value wipes out any inner negations, and it is redundant
          * with any inner absolute values.
          */
         return integer_abs;

      case nir_op_ineg: {
         const enum root_operation root =
            signed_integer_range_analysis(shader, range_ht,
                                          nir_scalar_chase_alu_src(scalar, 0),
                                          lo, hi);

         if (*lo == INT32_MIN) {
            *hi = INT32_MAX;
         } else {
            const int32_t a = -(*lo);
            const int32_t b = -(*hi);

            *lo = MIN2(a, b);
            *hi = MAX2(a, b);
         }

         /* Negation of a negation cancels out, but negation of absolute value
          * must preserve the integer_abs bit.
          */
         return root ^ integer_neg;
      }

      case nir_op_imax: {
         int src0_lo, src0_hi;
         int src1_lo, src1_hi;

         signed_integer_range_analysis(shader, range_ht,
                                       nir_scalar_chase_alu_src(scalar, 0),
                                       &src0_lo, &src0_hi);
         signed_integer_range_analysis(shader, range_ht,
                                       nir_scalar_chase_alu_src(scalar, 1),
                                       &src1_lo, &src1_hi);

         *lo = MAX2(src0_lo, src1_lo);
         *hi = MAX2(src0_hi, src1_hi);

         return non_unary;
      }

      case nir_op_imin: {
         int src0_lo, src0_hi;
         int src1_lo, src1_hi;

         signed_integer_range_analysis(shader, range_ht,
                                       nir_scalar_chase_alu_src(scalar, 0),
                                       &src0_lo, &src0_hi);
         signed_integer_range_analysis(shader, range_ht,
                                       nir_scalar_chase_alu_src(scalar, 1),
                                       &src1_lo, &src1_hi);

         *lo = MIN2(src0_lo, src1_lo);
         *hi = MIN2(src0_hi, src1_hi);

         return non_unary;
      }

      default:
         break;
      }
   }

   /* Any value with the sign-bit set is problematic. Consider the case when
    * bound is 0x80000000. As an unsigned value, this means the value must be
    * in the range [0, 0x80000000]. As a signed value, it means the value must
    * be in the range [0, INT_MAX] or it must be INT_MIN.
    *
    * If bound is -2, it means the value is either in the range [INT_MIN, -2]
    * or it is in the range [0, INT_MAX].
    *
    * This function only returns a single, contiguous range. The union of the
    * two ranges for any value of bound with the sign-bit set is [INT_MIN,
    * INT_MAX].
    */
   const int32_t bound = nir_unsigned_upper_bound(shader, range_ht,
                                                     scalar, NULL);
   if (bound < 0) {
      *lo = INT32_MIN;
      *hi = INT32_MAX;
   } else {
      *lo = 0;
      *hi = bound;
   }

   return non_unary;
}

static bool
brw_nir_opt_peephole_imul32x16_instr(nir_builder *b,
                                     nir_instr *instr,
                                     void *cb_data)
{
   struct pass_data *d = (struct pass_data *) cb_data;
   struct hash_table *range_ht = d->range_ht;

   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *imul = nir_instr_as_alu(instr);
   if (imul->op != nir_op_imul)
      return false;

   if (imul->def.bit_size != 32)
      return false;

   nir_op new_opcode = nir_num_opcodes;

   unsigned i;
   for (i = 0; i < 2; i++) {
      if (!nir_src_is_const(imul->src[i].src))
         continue;

      int64_t lo = INT64_MAX;
      int64_t hi = INT64_MIN;

      for (unsigned comp = 0; comp < imul->def.num_components; comp++) {
         int64_t v = nir_src_comp_as_int(imul->src[i].src, comp);

         if (v < lo)
            lo = v;

         if (v > hi)
            hi = v;
      }

      if (lo >= INT16_MIN && hi <= INT16_MAX) {
         new_opcode = nir_op_imul_32x16;
         break;
      } else if (lo >= 0 && hi <= UINT16_MAX) {
         new_opcode = nir_op_umul_32x16;
         break;
      }
   }

   if (new_opcode != nir_num_opcodes) {
      replace_imul_instr(b, imul, i, new_opcode);
      return true;
   }

   if (imul->def.num_components > 1)
      return false;

   const nir_scalar imul_scalar = { &imul->def, 0 };
   int idx = -1;
   enum root_operation prev_root = invalid_root;

   for (i = 0; i < 2; i++) {
      /* All constants were previously processed.  There is nothing more to
       * learn from a constant here.
       */
      if (imul->src[i].src.ssa->parent_instr->type == nir_instr_type_load_const)
         continue;

      nir_scalar scalar = nir_scalar_chase_alu_src(imul_scalar, i);
      int lo = INT32_MIN;
      int hi = INT32_MAX;

      const enum root_operation root =
         signed_integer_range_analysis(b->shader, range_ht, scalar, &lo, &hi);

      /* Copy propagation (in the backend) has trouble handling cases like
       *
       *    mov(8)          g60<1>D         -g59<8,8,1>D
       *    mul(8)          g61<1>D         g63<8,8,1>D     g60<16,8,2>W
       *
       * If g59 had absolute value instead of negation, even improved copy
       * propagation would not be able to make progress.
       *
       * In cases where both sources to the integer multiplication can fit in
       * 16-bits, choose the source that does not have a source modifier.
       */
      if (root < prev_root) {
         if (lo >= INT16_MIN && hi <= INT16_MAX) {
            new_opcode = nir_op_imul_32x16;
            idx = i;
            prev_root = root;

            if (root == non_unary)
               break;
         } else if (lo >= 0 && hi <= UINT16_MAX) {
            new_opcode = nir_op_umul_32x16;
            idx = i;
            prev_root = root;

            if (root == non_unary)
               break;
         }
      }
   }

   if (new_opcode == nir_num_opcodes) {
      assert(idx == -1);
      assert(prev_root == invalid_root);
      return false;
   }

   assert(idx != -1);
   assert(prev_root != invalid_root);

   replace_imul_instr(b, imul, idx, new_opcode);
   return true;
}

bool
brw_nir_opt_peephole_imul32x16(nir_shader *shader)
{
   struct pass_data cb_data;

   cb_data.range_ht = _mesa_pointer_hash_table_create(NULL);

   bool progress = nir_shader_instructions_pass(shader,
                                                brw_nir_opt_peephole_imul32x16_instr,
                                                nir_metadata_block_index |
                                                nir_metadata_dominance,
                                                &cb_data);

   _mesa_hash_table_destroy(cb_data.range_ht, NULL);

   return progress;
}

