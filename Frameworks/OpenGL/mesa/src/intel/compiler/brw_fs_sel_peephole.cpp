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

#include "brw_fs.h"
#include "brw_fs_builder.h"
#include "brw_cfg.h"

/** @file brw_fs_sel_peephole.cpp
 *
 * This file contains the opt_peephole_sel() optimization pass that replaces
 * MOV instructions to the same destination in the "then" and "else" bodies of
 * an if statement with SEL instructions.
 */

/* Four MOVs seems to be pretty typical, so I picked the next power of two in
 * the hopes that it would handle almost anything possible in a single
 * pass.
 */
#define MAX_MOVS 8 /**< The maximum number of MOVs to attempt to match. */

using namespace brw;

/**
 * Scans forwards from an IF counting consecutive MOV instructions in the
 * "then" and "else" blocks of the if statement.
 *
 * A pointer to the bblock_t following the IF is passed as the <then_block>
 * argument. The function stores pointers to the MOV instructions in the
 * <then_mov> and <else_mov> arrays.
 *
 * \return the minimum number of MOVs found in the two branches or zero if
 *         an error occurred.
 *
 * E.g.:
 *                  IF ...
 *    then_mov[0] = MOV g4, ...
 *    then_mov[1] = MOV g5, ...
 *    then_mov[2] = MOV g6, ...
 *                  ELSE ...
 *    else_mov[0] = MOV g4, ...
 *    else_mov[1] = MOV g5, ...
 *    else_mov[2] = MOV g7, ...
 *                  ENDIF
 *    returns 3.
 */
static int
count_movs_from_if(const intel_device_info *devinfo,
                   fs_inst *then_mov[MAX_MOVS], fs_inst *else_mov[MAX_MOVS],
                   bblock_t *then_block, bblock_t *else_block)
{
   int then_movs = 0;
   foreach_inst_in_block(fs_inst, inst, then_block) {
      if (then_movs == MAX_MOVS || inst->opcode != BRW_OPCODE_MOV ||
          inst->flags_written(devinfo))
         break;

      then_mov[then_movs] = inst;
      then_movs++;
   }

   int else_movs = 0;
   foreach_inst_in_block(fs_inst, inst, else_block) {
      if (else_movs == MAX_MOVS || inst->opcode != BRW_OPCODE_MOV ||
          inst->flags_written(devinfo))
         break;

      else_mov[else_movs] = inst;
      else_movs++;
   }

   return MIN2(then_movs, else_movs);
}

/**
 * Try to replace IF/MOV+/ELSE/MOV+/ENDIF with SEL.
 *
 * Many GLSL shaders contain the following pattern:
 *
 *    x = condition ? foo : bar
 *
 * or
 *
 *    if (...) a.xyzw = foo.xyzw;
 *    else     a.xyzw = bar.xyzw;
 *
 * The compiler emits an ir_if tree for this, since each subexpression might be
 * a complex tree that could have side-effects or short-circuit logic.
 *
 * However, the common case is to simply select one of two constants or
 * variable values---which is exactly what SEL is for.  In this case, the
 * assembly looks like:
 *
 *    (+f0) IF
 *    MOV dst src0
 *    ...
 *    ELSE
 *    MOV dst src1
 *    ...
 *    ENDIF
 *
 * where each pair of MOVs to a common destination and can be easily translated
 * into
 *
 *    (+f0) SEL dst src0 src1
 *
 * If src0 is an immediate value, we promote it to a temporary GRF.
 */
bool
fs_visitor::opt_peephole_sel()
{
   bool progress = false;

   foreach_block (block, cfg) {
      /* IF instructions, by definition, can only be found at the ends of
       * basic blocks.
       */
      fs_inst *if_inst = (fs_inst *)block->end();
      if (if_inst->opcode != BRW_OPCODE_IF)
         continue;

      fs_inst *else_mov[MAX_MOVS] = { NULL };
      fs_inst *then_mov[MAX_MOVS] = { NULL };

      bblock_t *then_block = block->next();
      bblock_t *else_block = NULL;
      foreach_list_typed(bblock_link, child, link, &block->children) {
         if (child->block != then_block) {
            if (child->block->prev()->end()->opcode == BRW_OPCODE_ELSE) {
               else_block = child->block;
            }
            break;
         }
      }
      if (else_block == NULL)
         continue;

      int movs = count_movs_from_if(devinfo, then_mov, else_mov, then_block, else_block);

      if (movs == 0)
         continue;

      /* Generate SEL instructions for pairs of MOVs to a common destination. */
      for (int i = 0; i < movs; i++) {
         if (!then_mov[i] || !else_mov[i])
            break;

         /* Check that the MOVs are the right form. */
         if (!then_mov[i]->dst.equals(else_mov[i]->dst) ||
             then_mov[i]->exec_size != else_mov[i]->exec_size ||
             then_mov[i]->group != else_mov[i]->group ||
             then_mov[i]->force_writemask_all != else_mov[i]->force_writemask_all ||
             then_mov[i]->is_partial_write() ||
             else_mov[i]->is_partial_write() ||
             then_mov[i]->conditional_mod != BRW_CONDITIONAL_NONE ||
             else_mov[i]->conditional_mod != BRW_CONDITIONAL_NONE) {
            movs = i;
            break;
         }

         /* Check that source types for mov operations match. */
         if (then_mov[i]->src[0].type != else_mov[i]->src[0].type) {
            movs = i;
            break;
         }
      }

      if (movs == 0)
         continue;

      for (int i = 0; i < movs; i++) {
         const fs_builder ibld = fs_builder(this, then_block, then_mov[i])
                                 .at(block, if_inst);

         if (then_mov[i]->src[0].equals(else_mov[i]->src[0])) {
            ibld.MOV(then_mov[i]->dst, then_mov[i]->src[0]);
         } else {
            /* Only the last source register can be a constant, so if the MOV
             * in the "then" clause uses a constant, we need to put it in a
             * temporary.
             */
            fs_reg src0(then_mov[i]->src[0]);
            if (src0.file == IMM) {
               src0 = ibld.vgrf(then_mov[i]->src[0].type);
               ibld.MOV(src0, then_mov[i]->src[0]);
            }

            /* 64-bit immediates can't be placed in src1. */
            fs_reg src1(else_mov[i]->src[0]);
            if (src1.file == IMM && type_sz(src1.type) == 8) {
               src1 = ibld.vgrf(else_mov[i]->src[0].type);
               ibld.MOV(src1, else_mov[i]->src[0]);
            }

            set_predicate_inv(if_inst->predicate, if_inst->predicate_inverse,
                              ibld.SEL(then_mov[i]->dst, src0, src1));
         }

         then_mov[i]->remove(then_block);
         else_mov[i]->remove(else_block);
      }

      progress = true;
   }

   if (progress)
      invalidate_analysis(DEPENDENCY_INSTRUCTIONS | DEPENDENCY_VARIABLES);

   return progress;
}
