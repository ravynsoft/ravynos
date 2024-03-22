/*
 * Copyright (c) 2019 Connor Abbott <cwabbott0@gmail.com>
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

/* This pass clones certain input intrinsics, creating a copy for each user.
 * Inputs are relatively cheap, since in both PP and GP one input can be
 * loaded "for free" in each instruction bundle. In GP especially, if there is
 * a load instruction with multiple uses in different basic blocks, we need to
 * split it in NIR so that we don't generate a register write and reads for
 * it, which is almost certainly more expensive than splitting. Hence this
 * pass is more aggressive than nir_opt_move, which just moves the intrinsic
 * down but won't split it.
 */

static nir_def *
clone_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin)
{
   nir_intrinsic_instr *new_intrin =
      nir_instr_as_intrinsic(nir_instr_clone(b->shader, &intrin->instr));

   nir_builder_instr_insert(b, &new_intrin->instr);

   return &new_intrin->def;
}

static bool
replace_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin)
{
   if (intrin->intrinsic != nir_intrinsic_load_input &&
       intrin->intrinsic != nir_intrinsic_load_uniform)
      return false;

   if (intrin->src[0].ssa->parent_instr->type == nir_instr_type_load_const)
      return false;

   struct hash_table *visited_instrs = _mesa_pointer_hash_table_create(NULL);

   nir_foreach_use_safe(src, &intrin->def) {
      struct hash_entry *entry =
         _mesa_hash_table_search(visited_instrs, nir_src_parent_instr(src));
      if (entry && (nir_src_parent_instr(src)->type != nir_instr_type_phi)) {
         nir_def *def = entry->data;
         nir_src_rewrite(src, def);
         continue;
      }
      b->cursor = nir_before_src(src);
      nir_def *new = clone_intrinsic(b, intrin);
      nir_src_rewrite(src, new);
      _mesa_hash_table_insert(visited_instrs, nir_src_parent_instr(src), new);
   }
   nir_foreach_if_use_safe(src, &intrin->def) {
      b->cursor = nir_before_src(src);
      nir_src_rewrite(&nir_src_parent_if(src)->condition, clone_intrinsic(b, intrin));
   }

   nir_instr_remove(&intrin->instr);
   _mesa_hash_table_destroy(visited_instrs, NULL);
   return true;
}

static void
replace_load_const(nir_builder *b, nir_load_const_instr *load_const)
{
   struct hash_table *visited_instrs = _mesa_pointer_hash_table_create(NULL);

   nir_foreach_use_safe(src, &load_const->def) {
      struct hash_entry *entry =
         _mesa_hash_table_search(visited_instrs, nir_src_parent_instr(src));
      if (entry && (nir_src_parent_instr(src)->type != nir_instr_type_phi)) {
         nir_def *def = entry->data;
         nir_src_rewrite(src, def);
         continue;
      }
      b->cursor = nir_before_src(src);
      nir_def *new = nir_build_imm(b, load_const->def.num_components,
                                       load_const->def.bit_size,
                                       load_const->value);
      nir_src_rewrite(src, new);
      _mesa_hash_table_insert(visited_instrs, nir_src_parent_instr(src), new);
   }

   nir_instr_remove(&load_const->instr);
   _mesa_hash_table_destroy(visited_instrs, NULL);
}

bool
lima_nir_split_loads(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block_reverse(block, impl) {
         nir_foreach_instr_reverse_safe(instr, block) {
            if (instr->type == nir_instr_type_load_const) {
               replace_load_const(&b, nir_instr_as_load_const(instr));
               progress = true;
            } else if (instr->type == nir_instr_type_intrinsic) {
               progress |= replace_intrinsic(&b, nir_instr_as_intrinsic(instr));
            }
         }
      }
   }

   return progress;
}

