/*
 * Copyright (c) 2017 Lima Project
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


static bool create_new_instr(ppir_block *block, ppir_node *node)
{
   ppir_instr *instr = ppir_instr_create(block);
   if (unlikely(!instr))
      return false;

   if (!ppir_instr_insert_node(instr, node))
      return false;

   return true;
}

/*
 * If a node has a pipeline dest, schedule it in the same instruction as its
 * successor.
 * Since it has a pipeline dest, it must have only one successor and since we
 * schedule nodes backwards, its successor must have already been scheduled.
 * Load varyings can't output to a pipeline register but are also potentially
 * trivial to insert and save an instruction if they have a single successor.
 */
static bool ppir_do_node_to_instr_try_insert(ppir_block *block, ppir_node *node)
{
   ppir_dest *dest = ppir_node_get_dest(node);

   if (dest && dest->type == ppir_target_pipeline) {
      assert(ppir_node_has_single_src_succ(node));
      ppir_node *succ = ppir_node_first_succ(node);
      assert(succ);
      assert(succ->instr);

      return ppir_instr_insert_node(succ->instr, node);
   }

   if (ppir_node_has_single_succ(node) &&
      ppir_node_has_single_pred(ppir_node_first_succ(node)) &&
      (ppir_node_first_succ(node)->type == ppir_node_type_branch)) {

      assert(ppir_node_has_single_succ(node));
      ppir_node *succ = ppir_node_first_succ(node);
      assert(succ);
      assert(succ->instr);

      return ppir_instr_insert_node(succ->instr, node);
   }

   switch (node->type) {
      case ppir_node_type_load:
         break;
      default:
         return false;
   }

   if (!ppir_node_has_single_src_succ(node))
      return false;

   ppir_node *succ = ppir_node_first_succ(node);
   assert(succ);
   assert(succ->instr);

   return ppir_instr_insert_node(succ->instr, node);
}

static bool ppir_do_one_node_to_instr(ppir_block *block, ppir_node *node)
{
   switch (node->type) {
   case ppir_node_type_alu:
   {
      /* don't create an instr for undef node */
      if (node->op == ppir_op_undef)
         break;

      /* merge pred mul and succ add in the same instr can save a reg
       * by using pipeline reg ^vmul/^fmul */
      ppir_alu_node *alu = ppir_node_to_alu(node);
      if (alu->dest.type == ppir_target_ssa &&
          ppir_node_has_single_succ(node) &&
          ppir_node_has_single_src_succ(node)) {
         ppir_node *succ = ppir_node_first_succ(node);
         if (succ->instr_pos == PPIR_INSTR_SLOT_ALU_VEC_ADD) {
            node->instr_pos = PPIR_INSTR_SLOT_ALU_VEC_MUL;
            ppir_instr_insert_mul_node(succ, node);
         }
         else if (succ->instr_pos == PPIR_INSTR_SLOT_ALU_SCL_ADD &&
                  alu->dest.ssa.num_components == 1) {
            node->instr_pos = PPIR_INSTR_SLOT_ALU_SCL_MUL;
            ppir_instr_insert_mul_node(succ, node);
         }
      }

      /* can't inserted to any existing instr, create one */
      if (!node->instr && !create_new_instr(block, node))
         return false;

      break;
   }
   case ppir_node_type_load:
   case ppir_node_type_load_texture:
   {
      if (!create_new_instr(block, node))
         return false;

      /* load varying output can be a register, it doesn't need a mov */
      switch (node->op) {
      case ppir_op_load_varying:
      case ppir_op_load_coords:
      case ppir_op_load_coords_reg:
      case ppir_op_load_fragcoord:
      case ppir_op_load_pointcoord:
      case ppir_op_load_frontface:
         return true;
      default:
         break;
      }

      /* Load cannot be pipelined, likely slot is already taken. Create a mov */
      assert(ppir_node_has_single_src_succ(node));
      ppir_dest *dest = ppir_node_get_dest(node);
      assert(dest->type == ppir_target_pipeline);
      ppir_pipeline pipeline_reg = dest->pipeline;

      /* Turn dest back to SSA, so we can update predecessors */
      ppir_node *succ = ppir_node_first_succ(node);

      /* Single succ can still have multiple references to this node */
      for (int i = 0; i < ppir_node_get_src_num(succ); i++) {
         ppir_src *src = ppir_node_get_src(succ, i);
         if (src && src->node == node) {
            /* Can consume uniforms directly */
            dest->type = ppir_target_ssa;
            dest->ssa.index = -1;
            ppir_node_target_assign(src, node);
         }
      }

      ppir_node *move = ppir_node_insert_mov(node);
      if (unlikely(!move))
         return false;

      ppir_src *mov_src = ppir_node_get_src(move, 0);
      mov_src->type = dest->type = ppir_target_pipeline;
      mov_src->pipeline = dest->pipeline = pipeline_reg;

      ppir_debug("node_to_instr create move %d for load %d\n",
                 move->index, node->index);

      if (!ppir_instr_insert_node(node->instr, move))
         return false;

      break;
   }
   case ppir_node_type_const: {
      /* Const cannot be pipelined, too many consts in the instruction.
       * Create a mov. */

      ppir_node *move = ppir_node_insert_mov(node);
      if (!create_new_instr(block, move))
         return false;

      ppir_debug("node_to_instr create move %d for const %d\n",
                 move->index, node->index);

      ppir_dest *dest = ppir_node_get_dest(node);
      ppir_src *mov_src = ppir_node_get_src(move, 0);

      /* update succ from ^const to ssa mov output */
      ppir_dest *move_dest = ppir_node_get_dest(move);
      move_dest->type = ppir_target_ssa;
      ppir_node *succ = ppir_node_first_succ(move);
      ppir_node_replace_child(succ, node, move);

      mov_src->type = dest->type = ppir_target_pipeline;
      mov_src->pipeline = dest->pipeline = ppir_pipeline_reg_const0;

      if (!ppir_instr_insert_node(move->instr, node))
         return false;

      break;
   }
   case ppir_node_type_store:
   {
      if (node->op == ppir_op_store_temp) {
         if (!create_new_instr(block, node))
            return false;
         break;
      }
      break;
   }
   case ppir_node_type_discard:
      if (!create_new_instr(block, node))
         return false;
      block->stop = true;
      break;
   case ppir_node_type_branch:
      if (!create_new_instr(block, node))
         return false;
      break;
   default:
      return false;
   }

   return true;
}

static unsigned int ppir_node_score(ppir_node *node)
{
   /* preferentially expand nodes in later instruction slots first, so
    * nodes for earlier slots (which are more likely pipelineable) get
    * added to the ready list. */
   unsigned int late_slot = 0;
   int *slots = ppir_op_infos[node->op].slots;
   if (slots)
      for (int i = 0; slots[i] != PPIR_INSTR_SLOT_END; i++)
         late_slot = MAX2(late_slot, slots[i]);

   /* to untie, favour nodes with pipelines for earlier expansion.
    * increase that for nodes with chained pipelines */
   unsigned int pipeline = 0;
   ppir_node *n = node;
   ppir_dest *dest = ppir_node_get_dest(n);
   while (dest && dest->type == ppir_target_pipeline) {
      pipeline++;
      assert(ppir_node_has_single_src_succ(n));
      n = ppir_node_first_succ(n);
      dest = ppir_node_get_dest(n);
   }
   assert(pipeline < 4);

   return (late_slot << 2 | pipeline);
}

static ppir_node *ppir_ready_list_pick_best(ppir_block *block,
                                            struct list_head *ready_list)
{
   unsigned int best_score = 0;
   ppir_node *best = NULL;

   list_for_each_entry(ppir_node, node, ready_list, sched_list) {
      unsigned int score = ppir_node_score(node);
      if (!best || score > best_score) {
         best = node;
         best_score = score;
      }
   }

   assert(best);
   return best;
}

static bool ppir_do_node_to_instr(ppir_block *block, ppir_node *root)
{
   struct list_head ready_list;
   list_inithead(&ready_list);
   list_addtail(&root->sched_list, &ready_list);

   while (!list_is_empty(&ready_list)) {
      ppir_node *node = ppir_ready_list_pick_best(block, &ready_list);
      list_del(&node->sched_list);

      /* first try pipeline sched, if that didn't succeed try normal sched */
      if (!ppir_do_node_to_instr_try_insert(block, node))
         if (!ppir_do_one_node_to_instr(block, node))
            return false;

      /* The node writes output register. We can't stop at this exact
       * instruction because there may be another node that writes another
       * output, so set stop flag for the block. We will set stop flag on
       * the last instruction of the block during codegen
       */
      if (node->is_out)
         block->stop = true;

      ppir_node_foreach_pred(node, dep) {
         ppir_node *pred = dep->pred;
         bool ready = true;

         /* pred may already have been processed by a previous node */
         if (pred->instr)
            continue;

         /* insert pred only when all its successors have been inserted */
         ppir_node_foreach_succ(pred, dep) {
            ppir_node *succ = dep->succ;
            if (!succ->instr) {
               ready = false;
               break;
            }
         }

         if (ready)
            list_addtail(&pred->sched_list, &ready_list);
      }
   }

   return true;
}

static bool ppir_create_instr_from_node(ppir_compiler *comp)
{
   list_for_each_entry(ppir_block, block, &comp->block_list, list) {
      list_for_each_entry(ppir_node, node, &block->node_list, list) {
         if (ppir_node_is_root(node)) {
            if (!ppir_do_node_to_instr(block, node))
               return false;
         }
      }
   }

   return true;
}

static void ppir_build_instr_dependency(ppir_compiler *comp)
{
   list_for_each_entry(ppir_block, block, &comp->block_list, list) {
      list_for_each_entry(ppir_instr, instr, &block->instr_list, list) {
         for (int i = 0; i < PPIR_INSTR_SLOT_NUM; i++) {
            ppir_node *node = instr->slots[i];
            if (node) {
               ppir_node_foreach_pred(node, dep) {
                  ppir_node *pred = dep->pred;
                  if (pred->instr && pred->instr != instr)
                     ppir_instr_add_dep(instr, pred->instr);
               }
            }
         }
      }
   }
}

bool ppir_node_to_instr(ppir_compiler *comp)
{
   if (!ppir_create_instr_from_node(comp))
      return false;
   ppir_instr_print_list(comp);

   ppir_build_instr_dependency(comp);
   ppir_instr_print_dep(comp);

   return true;
}
