/*
 * Copyright © 2013 Intel Corporation
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

#include "brw_shader.h"

using namespace brw;

/** @file brw_predicated_break.cpp
 *
 * Loops are often structured as
 *
 * loop:
 *    CMP.f0
 *    (+f0) IF
 *    BREAK
 *    ENDIF
 *    ...
 *    WHILE loop
 *
 * This peephole pass removes the IF and ENDIF instructions and predicates the
 * BREAK, dropping two instructions from the loop body.
 *
 * If the loop was a DO { ... } WHILE loop, it looks like
 *
 * loop:
 *    ...
 *    CMP.f0
 *    (+f0) IF
 *    BREAK
 *    ENDIF
 *    WHILE loop
 *
 * and we can remove the BREAK instruction and predicate the WHILE.
 */

#define MAX_NESTING 128

struct loop_continue_tracking {
   BITSET_WORD has_continue[BITSET_WORDS(MAX_NESTING)];
   unsigned depth;
};

static void
enter_loop(struct loop_continue_tracking *s)
{
   s->depth++;

   /* Any loops deeper than that maximum nesting will just re-use the last
    * flag.  This simplifies most of the code.  MAX_NESTING is chosen to be
    * large enough that it is unlikely to occur.  Even if it does, the
    * optimization that uses this tracking is unlikely to make much
    * difference.
    */
   if (s->depth < MAX_NESTING)
      BITSET_CLEAR(s->has_continue, s->depth);
}

static void
exit_loop(struct loop_continue_tracking *s)
{
   assert(s->depth > 0);
   s->depth--;
}

static void
set_continue(struct loop_continue_tracking *s)
{
   const unsigned i = MIN2(s->depth, MAX_NESTING - 1);

   BITSET_SET(s->has_continue, i);
}

static bool
has_continue(const struct loop_continue_tracking *s)
{
   const unsigned i = MIN2(s->depth, MAX_NESTING - 1);

   return BITSET_TEST(s->has_continue, i);
}

bool
opt_predicated_break(backend_shader *s)
{
   bool progress = false;
   struct loop_continue_tracking state = { {0, }, 0 };

   foreach_block (block, s->cfg) {
      /* DO instructions, by definition, can only be found at the beginning of
       * basic blocks.
       */
      backend_instruction *const do_inst = block->start();

      /* BREAK, CONTINUE, and WHILE instructions, by definition, can only be
       * found at the ends of basic blocks.
       */
      backend_instruction *jump_inst = block->end();

      if (do_inst->opcode == BRW_OPCODE_DO)
         enter_loop(&state);

      if (jump_inst->opcode == BRW_OPCODE_CONTINUE)
         set_continue(&state);
      else if (jump_inst->opcode == BRW_OPCODE_WHILE)
         exit_loop(&state);

      if (block->start_ip != block->end_ip)
         continue;

      if (jump_inst->opcode != BRW_OPCODE_BREAK &&
          jump_inst->opcode != BRW_OPCODE_CONTINUE)
         continue;

      backend_instruction *if_inst = block->prev()->end();
      if (if_inst->opcode != BRW_OPCODE_IF)
         continue;

      backend_instruction *endif_inst = block->next()->start();
      if (endif_inst->opcode != BRW_OPCODE_ENDIF)
         continue;

      bblock_t *jump_block = block;
      bblock_t *if_block = jump_block->prev();
      bblock_t *endif_block = jump_block->next();

      jump_inst->predicate = if_inst->predicate;
      jump_inst->predicate_inverse = if_inst->predicate_inverse;

      bblock_t *earlier_block = if_block;
      if (if_block->start_ip == if_block->end_ip) {
         earlier_block = if_block->prev();
      }

      if_inst->remove(if_block);

      bblock_t *later_block = endif_block;
      if (endif_block->start_ip == endif_block->end_ip) {
         later_block = endif_block->next();
      }
      endif_inst->remove(endif_block);

      if (!earlier_block->ends_with_control_flow()) {
         /* FIXME: There is a potential problem here. If earlier_block starts
          * with a DO instruction, this will delete the physical link to the
          * WHILE block. It is unclear whether ENDIF has the same potential
          * problem.
          */
         assert(earlier_block->start() == NULL ||
                earlier_block->start()->opcode != BRW_OPCODE_DO);

         earlier_block->unlink_children();
         earlier_block->add_successor(s->cfg->mem_ctx, jump_block,
                                      bblock_link_logical);
      }

      if (!later_block->starts_with_control_flow()) {
         later_block->unlink_parents();
      }

      /* If jump_block already has a link to later_block, don't create another
       * one. Instead, promote the link to logical.
       */
      bool need_to_link = true;
      foreach_list_typed(bblock_link, link, link, &jump_block->children) {
         if (link->block == later_block) {
            assert(later_block->starts_with_control_flow());

            /* Update the link from later_block back to jump_block. */
            foreach_list_typed(bblock_link, parent_link, link, &later_block->parents) {
               if (parent_link->block == jump_block) {
                  parent_link->kind = bblock_link_logical;
               }
            }

            /* Update the link from jump_block to later_block. */
            link->kind = bblock_link_logical;
            need_to_link = false;
         }
      }

      if (need_to_link) {
         jump_block->add_successor(s->cfg->mem_ctx, later_block,
                                   bblock_link_logical);
      }

      if (earlier_block->can_combine_with(jump_block)) {
         earlier_block->combine_with(jump_block);

         block = earlier_block;
      }

      /* Now look at the first instruction of the block following the BREAK. If
       * it's a WHILE, we can delete the break, predicate the WHILE, and join
       * the two basic blocks.
       *
       * This optimization can only be applied if the only instruction that
       * can transfer control to the WHILE is the BREAK.  If other paths can
       * lead to the while, the flags may be in an unknown state, and the loop
       * could terminate prematurely.  This can occur if the loop contains a
       * CONT instruction.
       */
      bblock_t *while_block = earlier_block->next();
      backend_instruction *while_inst = while_block->start();

      if (jump_inst->opcode == BRW_OPCODE_BREAK &&
          while_inst->opcode == BRW_OPCODE_WHILE &&
          while_inst->predicate == BRW_PREDICATE_NONE &&
          !has_continue(&state)) {
         jump_inst->remove(earlier_block);
         while_inst->predicate = jump_inst->predicate;
         while_inst->predicate_inverse = !jump_inst->predicate_inverse;

         assert(earlier_block->can_combine_with(while_block));
         earlier_block->combine_with(while_block);
      }

      progress = true;
   }

   if (progress)
      s->invalidate_analysis(DEPENDENCY_BLOCKS | DEPENDENCY_INSTRUCTIONS);

   return progress;
}
