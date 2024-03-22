/*
 * Copyright © 2019 Vasily Khoruzhick <anarsoul@gmail.com>
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

#include "nir.h"
#include "nir_builder.h"

#include "lima_ir.h"

static bool
lima_nir_split_load_input_instr(nir_builder *b,
                                nir_instr *instr,
                                UNUSED void *cb_data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);
   if (alu->op != nir_op_mov)
      return false;

   nir_def *ssa = alu->src[0].src.ssa;
   if (ssa->parent_instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(ssa->parent_instr);
   if (intrin->intrinsic != nir_intrinsic_load_input)
      return false;

   uint8_t swizzle = alu->src[0].swizzle[0];
   int i;

   for (i = 1; i < alu->def.num_components; i++)
      if (alu->src[0].swizzle[i] != (swizzle + i))
         break;

   if (i != alu->def.num_components)
      return false;

   /* mali4xx can't access unaligned vec3, don't split load input */
   if (alu->def.num_components == 3 && swizzle > 0)
      return false;

   /* mali4xx can't access unaligned vec2, don't split load input */
   if (alu->def.num_components == 2 &&
       swizzle != 0 && swizzle != 2)
      return false;

   b->cursor = nir_before_instr(&intrin->instr);
   nir_intrinsic_instr *new_intrin = nir_intrinsic_instr_create(
                                          b->shader,
                                          intrin->intrinsic);
   nir_def_init(&new_intrin->instr, &new_intrin->def,
                alu->def.num_components, ssa->bit_size);
   new_intrin->num_components = alu->def.num_components;
   nir_intrinsic_set_base(new_intrin, nir_intrinsic_base(intrin));
   nir_intrinsic_set_component(new_intrin, nir_intrinsic_component(intrin) + swizzle);
   nir_intrinsic_set_dest_type(new_intrin, nir_intrinsic_dest_type(intrin));

   /* offset */
   new_intrin->src[0] = nir_src_for_ssa(intrin->src[0].ssa);

   nir_builder_instr_insert(b, &new_intrin->instr);
   nir_def_rewrite_uses(&alu->def,
                            &new_intrin->def);
   nir_instr_remove(&alu->instr);
   return true;
}

/* Replaces a single load of several packed varyings and number of movs with
 * a number of loads of smaller size
 */
bool
lima_nir_split_load_input(nir_shader *shader)
{
   return nir_shader_instructions_pass(shader, lima_nir_split_load_input_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}
