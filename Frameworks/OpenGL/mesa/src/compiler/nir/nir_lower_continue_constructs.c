/*
 * Copyright © 2021 Valve Corporation
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
 *
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_control_flow.h"

static bool
lower_loop_continue_block(nir_builder *b, nir_loop *loop, bool *repair_ssa)
{
   if (!nir_loop_has_continue_construct(loop))
      return false;

   nir_block *header = nir_loop_first_block(loop);
   nir_block *cont = nir_loop_first_continue_block(loop);

   /* count continue statements excluding unreachable ones */
   unsigned num_continue = 0;
   nir_block *single_predecessor = NULL;
   set_foreach(cont->predecessors, entry) {
      nir_block *pred = (nir_block *)entry->key;
      /* If the continue block has no predecessors, it is unreachable. */
      if (pred->predecessors->entries == 0)
         continue;

      single_predecessor = pred;
      if (num_continue++)
         break;
   }

   nir_lower_phis_to_regs_block(header);

   if (num_continue == 0) {
      /* this loop doesn't continue at all. delete the continue construct */
      nir_cf_list extracted;
      nir_cf_list_extract(&extracted, &loop->continue_list);
      nir_cf_delete(&extracted);
   } else if (num_continue == 1) {
      /* inline the continue construct */
      assert(single_predecessor->successors[0] == cont);
      assert(single_predecessor->successors[1] == NULL);

      nir_cf_list extracted;
      nir_cf_list_extract(&extracted, &loop->continue_list);
      nir_cf_reinsert(&extracted,
                      nir_after_block_before_jump(single_predecessor));
   } else {
      nir_lower_phis_to_regs_block(cont);
      *repair_ssa = true;

      /* As control flow has to re-converge before executing the continue
       * construct, we insert it at the beginning of the loop with a flag
       * to ensure that it doesn't get executed in the first iteration:
       *
       *    loop {
       *       if (i != 0) {
       *          continue construct
       *       }
       *       loop body
       *    }
       */

      nir_variable *do_cont =
         nir_local_variable_create(b->impl, glsl_bool_type(), "cont");

      b->cursor = nir_before_cf_node(&loop->cf_node);
      nir_store_var(b, do_cont, nir_imm_false(b), 1);
      b->cursor = nir_before_block(header);
      nir_if *cont_if = nir_push_if(b, nir_load_var(b, do_cont));
      {
         nir_cf_list extracted;
         nir_cf_list_extract(&extracted, &loop->continue_list);
         nir_cf_reinsert(&extracted, nir_before_cf_list(&cont_if->then_list));
      }
      nir_pop_if(b, cont_if);
      nir_store_var(b, do_cont, nir_imm_true(b), 1);
   }

   nir_loop_remove_continue_construct(loop);
   return true;
}

static bool
visit_cf_list(nir_builder *b, struct exec_list *list, bool *repair_ssa)
{
   bool progress = false;

   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block:
         continue;
      case nir_cf_node_if: {
         nir_if *nif = nir_cf_node_as_if(node);
         progress |= visit_cf_list(b, &nif->then_list, repair_ssa);
         progress |= visit_cf_list(b, &nif->else_list, repair_ssa);
         break;
      }
      case nir_cf_node_loop: {
         nir_loop *loop = nir_cf_node_as_loop(node);
         progress |= visit_cf_list(b, &loop->body, repair_ssa);
         progress |= visit_cf_list(b, &loop->continue_list, repair_ssa);
         progress |= lower_loop_continue_block(b, loop, repair_ssa);
         break;
      }
      case nir_cf_node_function:
         unreachable("Unsupported cf_node type.");
      }
   }

   return progress;
}

static bool
lower_continue_constructs_impl(nir_function_impl *impl)
{
   nir_builder b = nir_builder_create(impl);
   bool repair_ssa = false;
   bool progress = visit_cf_list(&b, &impl->body, &repair_ssa);

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_none);

      /* Merge the Phis from Header and Continue Target */
      nir_lower_reg_intrinsics_to_ssa_impl(impl);

      /* Re-inserting the Continue Target at the beginning of the loop
       * violates the dominance property if instructions in the continue
       * use SSA defs from the loop body.
       */
      if (repair_ssa)
         nir_repair_ssa_impl(impl);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   return progress;
}

bool
nir_lower_continue_constructs(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      if (lower_continue_constructs_impl(impl))
         progress = true;
   }

   return progress;
}
