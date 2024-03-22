/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "nir.h"
#include "nir_control_flow.h"

static bool
is_block_empty(nir_block *block)
{
   return nir_cf_node_is_last(&block->cf_node) &&
          exec_list_is_empty(&block->instr_list);
}

static bool
is_block_singular(nir_block *block)
{
   return nir_cf_node_is_last(&block->cf_node) &&
          (exec_list_is_empty(&block->instr_list) ||
           (exec_list_is_singular(&block->instr_list) && nir_block_ends_in_jump(block)));
}

static bool
nir_block_ends_in_continue(nir_block *block)
{
   if (exec_list_is_empty(&block->instr_list))
      return false;

   nir_instr *instr = nir_block_last_instr(block);
   return instr->type == nir_instr_type_jump &&
          nir_instr_as_jump(instr)->type == nir_jump_continue;
}

/**
 * This optimization tries to merge two equal jump instructions (break or
 * continue) into a single one.
 *
 * This optimization turns
 *
 *     loop {
 *        ...
 *        if (cond) {
 *           do_work_1();
 *           break;
 *        } else {
 *           do_work_2();
 *           break;
 *        }
 *     }
 *
 * into:
 *
 *     loop {
 *        ...
 *        if (cond) {
 *           do_work_1();
 *        } else {
 *           do_work_2();
 *        }
 *        break;
 *     }
 *
 * It does the same with continue statements, respectively.
 *
 */
static bool
opt_loop_merge_break_continue(nir_if *nif)
{
   nir_block *after_if = nir_cf_node_cf_tree_next(&nif->cf_node);

   /* The block after the IF must have no predecessors and be empty. */
   if (after_if->predecessors->entries > 0 || !is_block_empty(after_if))
      return false;

   nir_block *last_then = nir_if_last_then_block(nif);
   nir_block *last_else = nir_if_last_else_block(nif);
   const bool then_break = nir_block_ends_in_break(last_then);
   const bool else_break = nir_block_ends_in_break(last_else);
   const bool then_cont = nir_block_ends_in_continue(last_then);
   const bool else_cont = nir_block_ends_in_continue(last_else);

   /* If both branch legs end with the same jump instruction,
    * merge the statement after the branch
    */
   if ((then_break && else_break) || (then_cont && else_cont)) {
      nir_lower_phis_to_regs_block(last_then->successors[0]);
      nir_instr_remove_v(nir_block_last_instr(last_then));
      nir_instr *jump = nir_block_last_instr(last_else);
      nir_instr_remove_v(jump);
      nir_instr_insert(nir_after_block(after_if), jump);
      return true;
   }

   return false;
}

/**
 * This optimization simplifies potential loop terminators which then allows
 * other passes such as opt_if_simplification() and loop unrolling to progress
 * further:
 *
 *     if (cond) {
 *        ... then block instructions ...
 *     } else {
 *         ...
 *        break;
 *     }
 *
 * into:
 *
 *     if (cond) {
 *     } else {
 *         ...
 *        break;
 *     }
 *     ... then block instructions ...
 */
static bool
opt_loop_terminator(nir_if *nif)
{
   nir_block *break_blk = NULL;
   nir_block *continue_from_blk = NULL;
   nir_block *first_continue_from_blk = NULL;

   nir_block *last_then = nir_if_last_then_block(nif);
   nir_block *last_else = nir_if_last_else_block(nif);

   if (nir_block_ends_in_break(last_then)) {
      break_blk = last_then;
      continue_from_blk = last_else;
      first_continue_from_blk = nir_if_first_else_block(nif);
   } else if (nir_block_ends_in_break(last_else)) {
      break_blk = last_else;
      continue_from_blk = last_then;
      first_continue_from_blk = nir_if_first_then_block(nif);
   }

   /* Continue if the if-statement contained no jumps at all */
   if (!break_blk)
      return false;

   /* If the continue from block is empty then return as there is nothing to
    * move.
    */
   if (is_block_empty(first_continue_from_blk))
      return false;

   if (nir_block_ends_in_jump(continue_from_blk)) {
      /* Let nir_opt_dead_cf() clean up any dead code. */
      if (!is_block_empty(nir_cf_node_cf_tree_next(&nif->cf_node)))
         return false;

      /* We are about to move the predecessor. */
      nir_lower_phis_to_regs_block(continue_from_blk->successors[0]);
   }

   /* Even though this if statement has a jump on one side, we may still have
    * phis afterwards.  Single-source phis can be produced by loop unrolling
    * or dead control-flow passes and are perfectly legal.  Run a quick phi
    * removal on the block after the if to clean up any such phis.
    */
   nir_opt_remove_phis_block(nir_cf_node_as_block(nir_cf_node_next(&nif->cf_node)));

   /* Finally, move the continue from branch after the if-statement. */
   nir_cf_list tmp;
   nir_cf_extract(&tmp, nir_before_block(first_continue_from_blk),
                  nir_after_block(continue_from_blk));
   nir_cf_reinsert(&tmp, nir_after_cf_node(&nif->cf_node));

   return true;
}

/**
 * This optimization tries to merge the jump instruction (break or continue)
 * of a block with an equal one from a previous IF.
 *
 * This optimization turns:
 *
 *     loop {
 *        ...
 *        if (cond) {
 *           do_work_1();
 *           break;
 *        } else {
 *        }
 *        do_work_2();
 *        break;
 *     }
 *
 * into:
 *
 *     loop {
 *        ...
 *        if (cond) {
 *           do_work_1();
 *        } else {
 *           do_work_2();
 *        }
 *        break;
 *     }
 *
 * It does the same with continue statements, respectively.
 *
 */
static bool
opt_loop_last_block(nir_block *block, bool is_trivial_continue, bool is_trivial_break)
{
   /* If this block has no predecessors, let nir_opt_dead_cf() do the cleanup */
   if (block->predecessors->entries == 0)
      return false;

   bool progress = false;
   bool has_break = nir_block_ends_in_break(block);
   bool has_continue = nir_block_ends_in_continue(block);

   /* Remove any "trivial" break and continue, i.e. those that are at the tail
    * of a CF-list where we can just delete the instruction and
    * control-flow will naturally take us to the same target block.
    */
   if ((has_break && is_trivial_break) || (has_continue && is_trivial_continue)) {
      nir_lower_phis_to_regs_block(block->successors[0]);
      nir_instr_remove_v(nir_block_last_instr(block));
      return true;
   }

   if (!nir_block_ends_in_jump(block)) {
      has_break = is_trivial_break;
      has_continue = is_trivial_continue;
   } else if (is_trivial_continue || is_trivial_break) {
      /* This block ends in a jump that cannot be removed because the implicit
       * fallthrough leads to a different target block.
       *
       * We already optimized this block's jump with the predecessors' when visiting
       * this block with opt_loop_last_block(block, is_trivial_* = false, false).
       */
      return false;
   }

   /* Nothing to do. */
   if (!has_continue && !has_break)
      return false;

   /* Walk backwards and check for previous IF statements whether one of the
    * branch legs ends with an equal jump instruction as this block.
    */
   for (nir_cf_node *prev = nir_cf_node_prev(&block->cf_node); prev != NULL; prev = nir_cf_node_prev(prev)) {
      /* Skip blocks and nested loops */
      if (prev->type != nir_cf_node_if)
         continue;

      nir_if *nif = nir_cf_node_as_if(prev);
      nir_block *then_block = nir_if_last_then_block(nif);
      nir_block *else_block = nir_if_last_else_block(nif);
      if (!nir_block_ends_in_jump(then_block) && !nir_block_ends_in_jump(else_block))
         continue;

      bool merge_into_then = (has_continue && nir_block_ends_in_continue(else_block)) ||
                             (has_break && nir_block_ends_in_break(else_block));
      bool merge_into_else = (has_continue && nir_block_ends_in_continue(then_block)) ||
                             (has_break && nir_block_ends_in_break(then_block));

      if (!merge_into_then && !merge_into_else)
         continue;

      /* If there are single-source phis after the IF, get rid of them first */
      nir_opt_remove_phis_block(nir_cf_node_cf_tree_next(prev));

      /* We are about to remove one predecessor. */
      nir_lower_phis_to_regs_block(block->successors[0]);

      nir_cf_list tmp;
      nir_cf_extract(&tmp, nir_after_cf_node(prev), nir_after_block_before_jump(block));

      if (merge_into_then) {
         nir_cf_reinsert(&tmp, nir_after_block(then_block));
      } else {
         nir_cf_reinsert(&tmp, nir_after_block(else_block));
      }

      /* Because we split the current block, the pointer is not valid anymore. */
      block = nir_cf_node_cf_tree_next(prev);
      progress = true;
   }

   /* Revisit the predecessor blocks in order to remove implicit jump instructions. */
   if (is_block_singular(block)) {
      nir_cf_node *prev = nir_cf_node_prev(&block->cf_node);
      if (prev && prev->type == nir_cf_node_if) {
         nir_if *nif = nir_cf_node_as_if(prev);
         progress |= opt_loop_last_block(nir_if_last_then_block(nif), has_continue, has_break);
         progress |= opt_loop_last_block(nir_if_last_else_block(nif), has_continue, has_break);
      }
   }

   return progress;
}

static bool
opt_loop_cf_list(struct exec_list *cf_list)
{
   bool progress = false;
   foreach_list_typed_safe(nir_cf_node, cf_node, node, cf_list) {
      switch (cf_node->type) {
      case nir_cf_node_block: {
         nir_block *block = nir_cf_node_as_block(cf_node);
         progress |= opt_loop_last_block(block, false, false);
         break;
      }

      case nir_cf_node_if: {
         nir_if *nif = nir_cf_node_as_if(cf_node);
         progress |= opt_loop_cf_list(&nif->then_list);
         progress |= opt_loop_cf_list(&nif->else_list);
         progress |= opt_loop_merge_break_continue(nif);
         progress |= opt_loop_terminator(nif);
         break;
      }

      case nir_cf_node_loop: {
         nir_loop *loop = nir_cf_node_as_loop(cf_node);
         assert(!nir_loop_has_continue_construct(loop));
         progress |= opt_loop_cf_list(&loop->body);
         progress |= opt_loop_last_block(nir_loop_last_block(loop), true, false);
         break;
      }

      case nir_cf_node_function:
         unreachable("Invalid cf type");
      }
   }

   return progress;
}

/**
 * This pass aims to simplify loop control-flow by reducing the number
 * of break and continue statements.
 */
bool
nir_opt_loop(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      /* First we run the simple pass to get rid of pesky continues */
      if (opt_loop_cf_list(&impl->body)) {
         nir_metadata_preserve(impl, nir_metadata_none);

         /* If that made progress, we're no longer really in SSA form. */
         nir_lower_reg_intrinsics_to_ssa_impl(impl);
         progress = true;
      } else {
         nir_metadata_preserve(impl, nir_metadata_all);
      }
   }

   return progress;
}
