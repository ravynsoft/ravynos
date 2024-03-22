/*
 * Copyright © 2014 Intel Corporation
 * Copyright © 2023 Igalia S.L.
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

#include "etnaviv_nir.h"
#include "etnaviv_compiler_nir.h"

/*
 * This pass lowers the neg and abs operations to pass_flags on
 * ALU operations.
 */

static void
alu_src_consume_abs(nir_instr *instr, unsigned idx)
{
   set_src_mod_abs(instr, idx);
}

static void
alu_src_consume_negate(nir_instr *instr, unsigned idx)
{
   /* If abs is set on the source, the negate goes away */
   if (!is_src_mod_abs(instr, idx))
      toggle_src_mod_neg(instr, idx);
}

static bool
nir_lower_to_source_mods_instr(nir_builder *b, nir_instr *instr,
                               void *data)
{
   bool progress = false;

   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   for (unsigned i = 0; i < nir_op_infos[alu->op].num_inputs; i++) {
      if (alu->src[i].src.ssa->parent_instr->type != nir_instr_type_alu)
         continue;

      nir_alu_instr *parent = nir_instr_as_alu(alu->src[i].src.ssa->parent_instr);

      if (nir_alu_type_get_base_type(nir_op_infos[alu->op].input_types[i]) != nir_type_float)
         continue;

      if (!(parent->op == nir_op_fabs) &&
          !(parent->op == nir_op_fneg)) {
         continue;
      }

      if (nir_src_bit_size(alu->src[i].src) == 64)
         continue;

      /* We can only store up to 3 source modifiers. */
      if (i >= 3)
         continue;

      nir_src_rewrite(&alu->src[i].src, parent->src[0].src.ssa);

      /* Apply any modifiers that come from the parent opcode */
      if (parent->op == nir_op_fneg)
         alu_src_consume_negate(instr, i);
      if (parent->op == nir_op_fabs)
         alu_src_consume_abs(instr, i);

      /* Apply modifiers from the parent source */
      if (is_src_mod_neg(&parent->instr, 0))
         alu_src_consume_negate(instr, i);
      if (is_src_mod_abs(&parent->instr, 0))
         alu_src_consume_abs(instr, i);

      for (int j = 0; j < 4; ++j) {
         if (!nir_alu_instr_channel_used(alu, i, j))
            continue;
         alu->src[i].swizzle[j] = parent->src[0].swizzle[alu->src[i].swizzle[j]];
      }

      if (nir_def_is_unused(&parent->def))
         nir_instr_remove(&parent->instr);

      progress = true;
   }

   return progress;
}

bool
etna_nir_lower_to_source_mods(nir_shader *shader)
{
   nir_shader_clear_pass_flags(shader);

   return nir_shader_instructions_pass(shader,
                                       nir_lower_to_source_mods_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}
