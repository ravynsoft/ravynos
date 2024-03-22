/*
 * Copyright (c) 2020 Lima Project
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
lima_nir_duplicate_intrinsic(nir_builder *b, nir_intrinsic_instr *itr,
                             nir_intrinsic_op op)
{
   nir_intrinsic_instr *last_dupl = NULL;
   nir_instr *last_parent_instr = NULL;

   nir_foreach_use_safe(use_src, &itr->def) {
      nir_intrinsic_instr *dupl;

      if (last_parent_instr != nir_src_parent_instr(use_src)) {
         /* if ssa use, clone for the target block */
         b->cursor = nir_before_instr(nir_src_parent_instr(use_src));
         dupl = nir_intrinsic_instr_create(b->shader, op);
         dupl->num_components = itr->num_components;
         memcpy(dupl->const_index, itr->const_index, sizeof(itr->const_index));
         dupl->src[0].ssa = itr->src[0].ssa;

         nir_def_init(&dupl->instr, &dupl->def, dupl->num_components,
                      itr->def.bit_size);

         dupl->instr.pass_flags = 1;
         nir_builder_instr_insert(b, &dupl->instr);
      }
      else {
         dupl = last_dupl;
      }

      nir_src_rewrite(use_src, &dupl->def);
      last_parent_instr = nir_src_parent_instr(use_src);
      last_dupl = dupl;
   }

   last_dupl = NULL;
   nir_if *last_parent_if = NULL;

   nir_foreach_if_use_safe(use_src, &itr->def) {
      nir_intrinsic_instr *dupl;
      nir_if *nif = nir_src_parent_if(use_src);

      if (last_parent_if != nif) {
         /* if 'if use', clone where it is */
         b->cursor = nir_before_instr(&itr->instr);
         dupl = nir_intrinsic_instr_create(b->shader, op);
         dupl->num_components = itr->num_components;
         memcpy(dupl->const_index, itr->const_index, sizeof(itr->const_index));
         dupl->src[0].ssa = itr->src[0].ssa;

         nir_def_init(&dupl->instr, &dupl->def, dupl->num_components,
                      itr->def.bit_size);

         dupl->instr.pass_flags = 1;
         nir_builder_instr_insert(b, &dupl->instr);
      }
      else {
         dupl = last_dupl;
      }

      nir_src_rewrite(&nir_src_parent_if(use_src)->condition, &dupl->def);
      last_parent_if = nif;
      last_dupl = dupl;
   }

   nir_instr_remove(&itr->instr);
   return true;
}

static void
lima_nir_duplicate_intrinsic_impl(nir_shader *shader, nir_function_impl *impl,
                                  nir_intrinsic_op op)
{
   nir_builder builder = nir_builder_create(impl);

   nir_foreach_block(block, impl) {
      nir_foreach_instr(instr, block) {
         instr->pass_flags = 0;
      }

      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *itr = nir_instr_as_intrinsic(instr);

         if (itr->intrinsic != op)
            continue;

         if (itr->instr.pass_flags)
            continue;

         lima_nir_duplicate_intrinsic(&builder, itr, op);
      }
   }

   nir_metadata_preserve(impl, nir_metadata_block_index |
                               nir_metadata_dominance);
}

/* Duplicate load uniforms for every user.
 * Helps by utilizing the load uniform instruction slots that would
 * otherwise stay empty, and reduces register pressure. */
void
lima_nir_duplicate_load_uniforms(nir_shader *shader)
{
   nir_foreach_function_impl(impl, shader) {
      lima_nir_duplicate_intrinsic_impl(shader, impl, nir_intrinsic_load_uniform);
   }
}

/* Duplicate load inputs for every user.
 * Helps by utilizing the load input instruction slots that would
 * otherwise stay empty, and reduces register pressure. */
void
lima_nir_duplicate_load_inputs(nir_shader *shader)
{
   nir_foreach_function_impl(impl, shader) {
      lima_nir_duplicate_intrinsic_impl(shader, impl, nir_intrinsic_load_input);
   }
}
