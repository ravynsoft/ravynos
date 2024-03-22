/*
 * Copyright (C) 2020 Collabora, Ltd.
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

#include "compiler.h"

/* The scheduler packs multiple instructions into a clause (grouped as tuple),
 * and the packing code takes in a clause and emits it to the wire. During
 * scheduling, we need to lay out the instructions (tuples) and constants
 * within the clause so constraints can be resolved during scheduling instead
 * of failing packing. These routines will help building clauses from
 * instructions so the scheduler can focus on the high-level algorithm, and
 * manipulating clause layouts.
 */

/* Is embedded constant 0 packed for free in a clause with this many tuples? */

bool
bi_ec0_packed(unsigned tuple_count)
{
   return (tuple_count == 3) || (tuple_count == 5) || (tuple_count == 6) ||
          (tuple_count == 8);
}

/* Helper to calculate the number of quadwords in a clause. This is a function
 * of the number of instructions and constants; it doesn't require actually
 * packing, which is useful for branch offsets.
 *
 * Table of instruction count to instruction quadwords, per the packing
 * algorithm, where * indicates a constant is packed for free:
 *
 *   X | Y
 *  ---|---
 *   1 | 1
 *   2 | 2
 *   3 | 3*
 *   4 | 3
 *   5 | 4*
 *   6 | 5*
 *   7 | 5
 *   8 | 6*
 *
 * Y = { X      if X <= 3
 *     { X - 1  if 4 <= X <= 6
 *     { X - 2  if 7 <= X <= 8
 *
 * and there is a constant for free if X is in {3, 5, 6, 8}. The remaining
 * constants are packed two-by-two as constant quadwords.
 */

static unsigned
bi_clause_quadwords(bi_clause *clause)
{
   unsigned X = clause->tuple_count;
   unsigned Y = X - ((X >= 7) ? 2 : (X >= 4) ? 1 : 0);

   unsigned constants = clause->constant_count;

   if ((X != 4) && (X != 7) && (X >= 3) && constants)
      constants--;

   return Y + DIV_ROUND_UP(constants, 2);
}

/* Measures the number of quadwords a branch jumps. Bifrost relative offsets
 * are from the beginning of a clause so to jump forward we count the current
 * clause length, but to jump backwards we do not. */

signed
bi_block_offset(bi_context *ctx, bi_clause *start, bi_block *target)
{
   /* Signed since we might jump backwards */
   signed ret = 0;

   /* Determine if the block we're branching to is strictly greater in
    * source order */
   bool forwards = target->index > start->block->index;

   if (forwards) {
      /* We have to jump through this block from the start of this
       * clause to the end */
      bi_foreach_clause_in_block_from(start->block, clause, start) {
         ret += bi_clause_quadwords(clause);
      }

      /* We then need to jump through every clause of every following
       * block until the target */
      bi_foreach_block_from(ctx, start->block, blk) {
         /* Don't double-count the first block */
         if (blk == start->block)
            continue;

         /* End just before the target */
         if (blk == target)
            break;

         /* Count every clause in the block */
         bi_foreach_clause_in_block(blk, clause) {
            ret += bi_clause_quadwords(clause);
         }
      }
   } else {
      /* We start at the beginning of the clause but have to jump
       * through the clauses before us in the block */
      bi_foreach_clause_in_block_from_rev(start->block, clause, start) {
         if (clause == start)
            continue;

         ret -= bi_clause_quadwords(clause);
      }

      /* And jump back every clause of preceding blocks up through
       * and including the target to get to the beginning of the
       * target */
      bi_foreach_block_from_rev(ctx, start->block, blk) {
         if (blk == start->block)
            continue;

         bi_foreach_clause_in_block(blk, clause) {
            ret -= bi_clause_quadwords(clause);
         }

         /* End just after the target */
         if (blk == target)
            break;
      }
   }

   return ret;
}
