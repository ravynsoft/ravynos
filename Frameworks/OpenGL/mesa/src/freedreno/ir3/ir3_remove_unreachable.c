/*
 * Copyright (C) 2021 Valve Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ir3.h"

/* Sometimes we can get unreachable blocks from NIR. In particular this happens
 * for blocks after an if where both sides end in a break/continue. These blocks
 * are then reachable only via the physical CFG. This pass deletes these blocks
 * and reroutes the physical edge past it.
 */

static void
delete_block(struct ir3 *ir, struct ir3_block *block)
{
   struct ir3_instruction *end = NULL;
   foreach_instr (instr, &block->instr_list) {
      if (instr->opc == OPC_END) {
         end = instr;
         break;
      }
   }

   /* The end block can be legitimately unreachable if the shader only exits via
    * discarding. ir3_legalize will then insert a branch to the end. Keep the
    * block around but delete all the other instructions and make the end not
    * take any sources, so that we don't have any dangling references to other
    * unreachable blocks.
    */
   if (end) {
      foreach_instr_safe (instr, &block->instr_list) {
         if (instr != end)
            list_delinit(&instr->node);
      }
      end->srcs_count = 0;
      return;
   }

   for (unsigned i = 0; i < 2; i++) {
      struct ir3_block *succ = block->successors[i];
      if (!succ)
         continue;

      unsigned pred_idx = ir3_block_get_pred_index(succ, block);

      /* If this isn't the last predecessor, we swap it with the last before
       * removing it.
       */
      bool swap_pred = pred_idx != succ->predecessors_count - 1;

      foreach_instr (phi, &succ->instr_list) {
         if (phi->opc != OPC_META_PHI)
            break;

         if (swap_pred)
            phi->srcs[pred_idx] = phi->srcs[phi->srcs_count - 1];
         phi->srcs_count--;
      }
      if (swap_pred) {
         succ->predecessors[pred_idx] =
            succ->predecessors[succ->predecessors_count - 1];
      }
      succ->predecessors_count--;
   }

   for (unsigned i = 0; i < 2; i++) {
      struct ir3_block *succ = block->physical_successors[i];
      if (!succ)
         continue;

      ir3_block_remove_physical_predecessor(succ, block);
   }

   if (block->physical_predecessors_count != 0) {
      /* There should be only one physical predecessor, for the fallthrough
       * edge.
       */
      assert(block->physical_predecessors_count == 1);
      struct ir3_block *pred = block->physical_predecessors[0];
      assert(block->node.next != &ir->block_list);
      struct ir3_block *next = list_entry(block->node.next, struct ir3_block, node);
      if (pred->physical_successors[1] == block)
         pred->physical_successors[1] = next;
      else
         pred->physical_successors[0] = next;
      ir3_block_add_physical_predecessor(next, pred);
   }
}

bool
ir3_remove_unreachable(struct ir3 *ir)
{
   bool progress = false;
   foreach_block_safe (block, &ir->block_list) {
      if (block != ir3_start_block(ir) && block->predecessors_count == 0) {
         delete_block(ir, block);
         list_del(&block->node);
         progress = true;
      }
   }

   return progress;
}
