/*
 * Copyright © 2018 Intel Corporation
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

static void
split_conversion(nir_builder *b, nir_alu_instr *alu, nir_alu_type src_type,
                 nir_alu_type tmp_type, nir_alu_type dst_type)
{
   b->cursor = nir_before_instr(&alu->instr);
   nir_def *src = nir_ssa_for_alu_src(b, alu, 0);
   nir_def *tmp = nir_type_convert(b, src, src_type, tmp_type, nir_rounding_mode_undef);
   nir_def *res = nir_type_convert(b, tmp, tmp_type, dst_type, nir_rounding_mode_undef);
   nir_def_rewrite_uses(&alu->def, res);
   nir_instr_remove(&alu->instr);
}

static bool
lower_alu_instr(nir_builder *b, nir_alu_instr *alu)
{
   unsigned src_bit_size = nir_src_bit_size(alu->src[0].src);
   nir_alu_type src_type = nir_op_infos[alu->op].input_types[0];
   nir_alu_type src_full_type = (nir_alu_type) (src_type | src_bit_size);

   unsigned dst_bit_size = alu->def.bit_size;
   nir_alu_type dst_full_type = nir_op_infos[alu->op].output_type;
   nir_alu_type dst_type = nir_alu_type_get_base_type(dst_full_type);

   /* BDW PRM, vol02, Command Reference Instructions, mov - MOVE:
    *
    *   "There is no direct conversion from HF to DF or DF to HF.
    *    Use two instructions and F (Float) as an intermediate type.
    *
    *    There is no direct conversion from HF to Q/UQ or Q/UQ to HF.
    *    Use two instructions and F (Float) or a word integer type
    *    or a DWord integer type as an intermediate type."
    *
    * It is important that the intermediate conversion happens through a
    * 32-bit float type so we don't lose range when we convert from
    * a 64-bit integer.
    */
   unsigned int64_types = nir_type_int64 | nir_type_uint64;
   if ((src_full_type == nir_type_float16 && (dst_full_type & int64_types)) ||
       ((src_full_type & int64_types) && dst_full_type == nir_type_float16)) {
      split_conversion(b, alu, src_type, nir_type_float | 32,
                       dst_type | dst_bit_size);
      return true;
   }

   /* SKL PRM, vol 02a, Command Reference: Instructions, Move:
    *
    *   "There is no direct conversion from B/UB to DF or DF to B/UB. Use
    *    two instructions and a word or DWord intermediate type."
    *
    *   "There is no direct conversion from B/UB to Q/UQ or Q/UQ to B/UB.
    *    Use two instructions and a word or DWord intermediate integer
    *    type."
    *
    * It is important that we use a 32-bit integer matching the sign of the
    * destination as the intermediate type so we avoid any chance of rtne
    * rounding happening before the conversion to integer (which is expected
    * to round towards zero) in double to byte conversions.
    */
   if ((src_bit_size == 8 && dst_bit_size == 64) ||
       (src_bit_size == 64 && dst_bit_size == 8)) {
      split_conversion(b, alu, src_type, dst_type | 32, dst_type | dst_bit_size);
      return true;
   }

   return false;
}

static bool
lower_instr(nir_builder *b, nir_instr *instr, UNUSED void *cb_data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   if (!nir_op_infos[alu->op].is_conversion)
      return false;

   return lower_alu_instr(b, alu);
}

bool
brw_nir_lower_conversions(nir_shader *shader)
{
   return nir_shader_instructions_pass(shader, lower_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}
