/*
 * Copyright (c) 2019 Lima Project
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

#include "ppir.h"

/* Propagates liveness from a liveness set to another by performing the
 * union between sets. */
static void
ppir_liveness_propagate(ppir_compiler *comp,
                        BITSET_WORD *dest_set, BITSET_WORD *src_set,
                        uint8_t *dest_mask, uint8_t *src_mask)
{
   for (int i = 0; i < BITSET_WORDS(comp->reg_num); i++)
      dest_set[i] |= src_set[i];

   for (int i = 0; i < reg_mask_size(comp->reg_num); i++)
      dest_mask[i] |= src_mask[i];
}

/* Check whether two liveness sets are equal. */
static bool
ppir_liveness_set_equal(ppir_compiler *comp,
                        BITSET_WORD *set1, BITSET_WORD *set2,
                        uint8_t *mask1, uint8_t *mask2)
{
   for (int i = 0; i < BITSET_WORDS(comp->reg_num); i++)
      if (set1[i] != set2[i])
         return false;

   for (int i = 0; i < reg_mask_size(comp->reg_num); i++)
      if (mask1[i] != mask2[i])
         return false;

   return true;
}

/* Update the liveness information of the instruction by adding its srcs
 * as live registers to the live_in set. */
static void
ppir_liveness_instr_srcs(ppir_compiler *comp, ppir_instr *instr)
{
   for (int i = PPIR_INSTR_SLOT_NUM-1; i >= 0; i--) {
      ppir_node *node = instr->slots[i];
      if (!node)
         continue;

      switch(node->op) {
         case ppir_op_const:
         case ppir_op_undef:
            continue;
         default:
            break;
      }

      for (int i = 0; i < ppir_node_get_src_num(node); i++) {
         ppir_src *src = ppir_node_get_src(node, i);
         if (!src || src->type == ppir_target_pipeline)
            continue;

         ppir_reg *reg = ppir_src_get_reg(src);
         if (!reg || reg->undef)
            continue;

         unsigned int index = reg->regalloc_index;

         /* if some other op on this same instruction is writing,
          * we just need to reserve a register for this particular
          * instruction. */
         if (src->node && src->node->instr == instr) {
            BITSET_SET(instr->live_internal, index);
            continue;
         }

         bool live = BITSET_TEST(instr->live_set, index);
         if (src->type == ppir_target_ssa) {
            /* reg is read, needs to be live before instr */
            if (live)
               continue;

            BITSET_SET(instr->live_set, index);
         }
         else {
            unsigned int mask = ppir_src_get_mask(src);
            uint8_t live_mask = get_reg_mask(instr->live_mask, index);

            /* read reg is type register, need to check if this sets
             * any additional bits in the current mask */
            if (live && (live_mask == (live_mask | mask)))
               continue;

            /* some new components */
            set_reg_mask(instr->live_mask, index, (live_mask | mask));
            BITSET_SET(instr->live_set, index);
         }
      }
   }
}


/* Update the liveness information of the instruction by removing its
 * dests from the live_in set. */
static void
ppir_liveness_instr_dest(ppir_compiler *comp, ppir_instr *instr, ppir_instr *last)
{
   for (int i = PPIR_INSTR_SLOT_NUM-1; i >= 0; i--) {
      ppir_node *node = instr->slots[i];
      if (!node)
         continue;

      switch(node->op) {
         case ppir_op_const:
         case ppir_op_undef:
            continue;
         default:
            break;
      }

      ppir_dest *dest = ppir_node_get_dest(node);
      if (!dest || dest->type == ppir_target_pipeline)
         continue;
      ppir_reg *reg = ppir_dest_get_reg(dest);
      if (!reg || reg->undef)
         continue;

      unsigned int index = reg->regalloc_index;
      bool live = BITSET_TEST(instr->live_set, index);

      /* If it's an out reg, it's alive till the end of the block, so add it
       * to live_set of the last instruction */
      if (!live && reg->out_reg && (instr != last)) {
         BITSET_SET(last->live_set, index);
         BITSET_CLEAR(instr->live_set, index);
         continue;
      }

      /* If a register is written but wasn't read in a later instruction, it is
       * either an output register in last instruction, dead code or a bug.
       * For now, assign an interference to it to ensure it doesn't get assigned
       * a live register and overwrites it. */
      if (!live) {
         BITSET_SET(instr->live_internal, index);
         continue;
      }

      if (dest->type == ppir_target_ssa) {
         /* reg is written and ssa, is not live before instr */
         BITSET_CLEAR(instr->live_set, index);
      }
      else {
         unsigned int mask = dest->write_mask;
         uint8_t live_mask = get_reg_mask(instr->live_mask, index);
         /* written reg is type register, need to check if this clears
          * the remaining mask to remove it from the live set */
         if (live_mask == (live_mask & ~mask))
            continue;

         set_reg_mask(instr->live_mask, index, (live_mask & ~mask));
         /* unset reg if all remaining bits were cleared */
         if ((live_mask & ~mask) == 0) {
            BITSET_CLEAR(instr->live_set, index);
         }
      }
   }
}

/* Main loop, iterate blocks/instructions/ops backwards, propagate
 * livenss and update liveness of each instruction. */
static bool
ppir_liveness_compute_live_sets(ppir_compiler *comp)
{
   uint8_t temp_live_mask[reg_mask_size(comp->reg_num)];
   BITSET_DECLARE(temp_live_set, comp->reg_num);
   bool cont = false;
   list_for_each_entry_rev(ppir_block, block, &comp->block_list, list) {
      if (list_is_empty(&block->instr_list))
         continue;

      ppir_instr *last = list_last_entry(&block->instr_list, ppir_instr, list);
      assert(last);

      list_for_each_entry_rev(ppir_instr, instr, &block->instr_list, list) {
         /* initial copy to check for changes */
         memset(temp_live_mask, 0, sizeof(temp_live_mask));
         memset(temp_live_set, 0, sizeof(temp_live_set));

         ppir_liveness_propagate(comp,
                                 temp_live_set, instr->live_set,
                                 temp_live_mask, instr->live_mask);

         /* inherit (or-) live variables from next instr or block */
         if (instr == last) {
            ppir_instr *next_instr;
            /* inherit liveness from the first instruction in the next blocks */
            for (int i = 0; i < 2; i++) {
               ppir_block *succ = block->successors[i];
               if (!succ)
                  continue;

               /* if the block is empty, go for the next-next until a non-empty
                * one is found */
               while (list_is_empty(&succ->instr_list)) {
                  assert(succ->successors[0] && !succ->successors[1]);
                  succ = succ->successors[0];
               }

               next_instr = list_first_entry(&succ->instr_list, ppir_instr, list);
               assert(next_instr);

               ppir_liveness_propagate(comp,
                                       instr->live_set, next_instr->live_set,
                                       instr->live_mask, next_instr->live_mask);
            }
         }
         else {
            ppir_instr *next_instr = list_entry(instr->list.next, ppir_instr, list);
            ppir_liveness_propagate(comp,
                                    instr->live_set, next_instr->live_set,
                                    instr->live_mask, next_instr->live_mask);
         }

         ppir_liveness_instr_dest(comp, instr, last);
         ppir_liveness_instr_srcs(comp, instr);

         cont |= !ppir_liveness_set_equal(comp,
                                          temp_live_set, instr->live_set,
                                          temp_live_mask, instr->live_mask);
      }
   }

   return cont;
}

/*
 * Liveness analysis is based on https://en.wikipedia.org/wiki/Live_variable_analysis
 * This implementation calculates liveness for each instruction.
 * The liveness set in this implementation is defined as the set of
 * registers live before the instruction executes.
 * Blocks/instructions/ops are iterated backwards so register reads are
 * propagated up to the instruction that writes it.
 *
 * 1) Before computing liveness for an instruction, propagate liveness
 *    from the next instruction. If it is the last instruction in a
 *    block, propagate liveness from all possible next instructions in
 *    the successor blocks.
 * 2) Calculate the live set for the instruction. The initial live set
 *    is a propagated set of the live set from the next instructions.
 *    - Registers which aren't touched by this instruction are kept
 *    intact.
 *    - If a register is written by this instruction, it no longer needs
 *    to be live before the instruction, so it is removed from the live
 *    set of that instruction.
 *    - If a register is read by this instruction, it needs to be live
 *    before its execution, so add it to its live set.
 *    - Non-ssa registers are a special case. For this, the algorithm
 *    keeps and updates the mask of live components following the same
 *    logic as above. The register is only removed from the live set of
 *    the instruction when no live components are left.
 *    - If a non-ssa register is written and read in the same
 *    instruction, it stays in the live set.
 *    - Another special case is when a register is only written and read
 *    within a single instruciton. In this case a register needs to be
 *    reserved but not propagated. The algorithm adds it to the
 *    live_internal set so that the register allocator properly assigns
 *    an interference for it.
 * 3) The algorithm must run over the entire program until it converges,
 *    i.e. a full run happens without changes. This is because blocks
 *    are updated sequentially and updates in a block may need to be
 *    propagated to parent blocks that were already calculated in the
 *    current run.
 */
void
ppir_liveness_analysis(ppir_compiler *comp)
{
   while (ppir_liveness_compute_live_sets(comp))
      ;
}
