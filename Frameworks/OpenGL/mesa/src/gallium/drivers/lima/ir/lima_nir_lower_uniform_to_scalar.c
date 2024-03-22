/*
 * Copyright (c) 2019 Qiang Yu <yuq825@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "nir.h"
#include "nir_builder.h"
#include "lima_ir.h"

static void
lower_load_uniform_to_scalar(nir_builder *b, nir_intrinsic_instr *intr)
{
   b->cursor = nir_before_instr(&intr->instr);

   nir_def *loads[4];
   for (unsigned i = 0; i < intr->num_components; i++) {
      nir_intrinsic_instr *chan_intr =
         nir_intrinsic_instr_create(b->shader, intr->intrinsic);
      nir_def_init(&chan_intr->instr, &chan_intr->def, 1,
                   intr->def.bit_size);
      chan_intr->num_components = 1;

      nir_intrinsic_set_base(chan_intr, nir_intrinsic_base(intr) * 4 + i);
      nir_intrinsic_set_range(chan_intr, nir_intrinsic_range(intr) * 4);
      nir_intrinsic_set_dest_type(chan_intr, nir_intrinsic_dest_type(intr));

      chan_intr->src[0] =
         nir_src_for_ssa(nir_imul_imm(b, intr->src[0].ssa, 4));

      nir_builder_instr_insert(b, &chan_intr->instr);

      loads[i] = &chan_intr->def;
   }

   nir_def_rewrite_uses(&intr->def,
                            nir_vec(b, loads, intr->num_components));
   nir_instr_remove(&intr->instr);
}

void
lima_nir_lower_uniform_to_scalar(nir_shader *shader)
{
   nir_foreach_function_impl(impl, shader) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

            if (intr->intrinsic != nir_intrinsic_load_uniform)
               continue;

            lower_load_uniform_to_scalar(&b, intr);
         }
      }
   }
}
