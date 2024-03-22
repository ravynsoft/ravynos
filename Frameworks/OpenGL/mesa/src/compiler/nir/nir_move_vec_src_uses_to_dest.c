/*
 * Copyright © 2014 Intel Corporation
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

/*
 * Implements a pass that tries to move uses vecN sources to their
 * destinations.  This is kind of like an inverse copy-propagation pass.
 * For instance, if you have
 *
 * ssa_1 = vec4(a, b, c, d)
 * ssa_2 = fadd(a, b)
 *
 * This will be turned into
 *
 * ssa_1 = vec4(a, b, c, d)
 * ssa_2 = fadd(ssa_1.x, ssa_1.y)
 *
 * While this is "worse" because it adds a bunch of unneeded dependencies, it
 * actually makes it much easier for vec4-based backends to coalesce the MOVs
 * that result from the vec4 operation because it doesn't have to worry about
 * quite as many reads.
 */

/* Returns true if the given SSA def dominates the instruction.  An SSA def is
 * considered to *not* dominate the instruction that defines it.
 */
static bool
ssa_def_dominates_instr(nir_def *def, nir_instr *instr)
{
   if (instr->index <= def->parent_instr->index) {
      return false;
   } else if (def->parent_instr->block == instr->block) {
      return def->parent_instr->index < instr->index;
   } else {
      return nir_block_dominates(def->parent_instr->block, instr->block);
   }
}

static bool
move_vec_src_uses_to_dest_block(nir_block *block, bool skip_const_srcs)
{
   bool progress = false;

   nir_foreach_instr(instr, block) {
      if (instr->type != nir_instr_type_alu)
         continue;

      nir_alu_instr *vec = nir_instr_as_alu(instr);

      switch (vec->op) {
      case nir_op_vec2:
      case nir_op_vec3:
      case nir_op_vec4:
         break;
      default:
         continue; /* The loop */
      }

      /* If the vec is used only in single store output than by reusing it
       * we lose the ability to write it to the output directly.
       */
      if (list_is_singular(&vec->def.uses)) {
         nir_src *src = list_first_entry(&vec->def.uses, nir_src, use_link);
         nir_instr *use_instr = nir_src_parent_instr(src);
         if (use_instr->type == nir_instr_type_intrinsic) {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(use_instr);
            if (intr->intrinsic == nir_intrinsic_store_output)
               return false;
         }
      }

      /* First, mark all of the sources we are going to consider for rewriting
       * to the destination
       */
      int srcs_remaining = 0;
      for (unsigned i = 0; i < nir_op_infos[vec->op].num_inputs; i++) {
         if (skip_const_srcs && nir_src_is_const(vec->src[i].src))
            continue;

         srcs_remaining |= 1 << i;
      }

      /* We can't actually do anything with this instruction */
      if (srcs_remaining == 0)
         continue;

      for (unsigned i; i = ffs(srcs_remaining) - 1, srcs_remaining;) {
         int8_t swizzle[NIR_MAX_VEC_COMPONENTS];
         memset(swizzle, -1, sizeof(swizzle));

         for (unsigned j = i; j < nir_op_infos[vec->op].num_inputs; j++) {
            if (vec->src[j].src.ssa != vec->src[i].src.ssa)
               continue;

            /* Mark the given channel as having been handled */
            srcs_remaining &= ~(1 << j);

            /* Mark the appropriate channel as coming from src j */
            swizzle[vec->src[j].swizzle[0]] = j;
         }

         nir_foreach_use_safe(use, vec->src[i].src.ssa) {
            if (nir_src_parent_instr(use) == &vec->instr)
               continue;

            /* We need to dominate the use if we are going to rewrite it */
            if (!ssa_def_dominates_instr(&vec->def, nir_src_parent_instr(use)))
               continue;

            /* For now, we'll just rewrite ALU instructions */
            if (nir_src_parent_instr(use)->type != nir_instr_type_alu)
               continue;

            nir_alu_instr *use_alu = nir_instr_as_alu(nir_src_parent_instr(use));

            /* Figure out which source we're actually looking at */
            nir_alu_src *use_alu_src = exec_node_data(nir_alu_src, use, src);
            unsigned src_idx = use_alu_src - use_alu->src;
            assert(src_idx < nir_op_infos[use_alu->op].num_inputs);

            bool can_reswizzle = true;
            for (unsigned j = 0; j < 4; j++) {
               if (!nir_alu_instr_channel_used(use_alu, src_idx, j))
                  continue;

               if (swizzle[use_alu_src->swizzle[j]] == -1) {
                  can_reswizzle = false;
                  break;
               }
            }

            if (!can_reswizzle)
               continue;

            /* At this point, we have determined that the given use can be
             * reswizzled to actually use the destination of the vecN operation.
             * Go ahead and rewrite it as needed.
             */
            nir_src_rewrite(use, &vec->def);
            for (unsigned j = 0; j < 4; j++) {
               if (!nir_alu_instr_channel_used(use_alu, src_idx, j))
                  continue;

               use_alu_src->swizzle[j] = swizzle[use_alu_src->swizzle[j]];
               progress = true;
            }
         }
      }
   }

   return progress;
}

static bool
nir_move_vec_src_uses_to_dest_impl(nir_shader *shader, nir_function_impl *impl,
                                   bool skip_const_srcs)
{
   bool progress = false;

   nir_metadata_require(impl, nir_metadata_dominance);

   nir_index_instrs(impl);

   nir_foreach_block(block, impl) {
      progress |= move_vec_src_uses_to_dest_block(block, skip_const_srcs);
   }

   nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);

   return progress;
}

bool
nir_move_vec_src_uses_to_dest(nir_shader *shader, bool skip_const_srcs)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      progress |= nir_move_vec_src_uses_to_dest_impl(shader, impl, skip_const_srcs);
   }

   return progress;
}
