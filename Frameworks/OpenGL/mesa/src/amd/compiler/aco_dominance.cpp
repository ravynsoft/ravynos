/*
 * Copyright © 2018 Valve Corporation
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

#ifndef ACO_DOMINANCE_CPP
#define ACO_DOMINANCE_CPP

#include "aco_ir.h"

/*
 * Implements the algorithms for computing the dominator tree from
 * "A Simple, Fast Dominance Algorithm" by Cooper, Harvey, and Kennedy.
 *
 * Different from the paper, our CFG allows to compute the dominator tree
 * in a single pass as it is guaranteed that the dominating predecessors
 * are processed before the current block.
 */

namespace aco {

void
dominator_tree(Program* program)
{
   for (unsigned i = 0; i < program->blocks.size(); i++) {
      Block& block = program->blocks[i];

      /* If this block has no predecessor, it dominates itself by definition */
      if (block.linear_preds.empty()) {
         block.linear_idom = block.index;
         block.logical_idom = block.index;
         continue;
      }

      int new_logical_idom = -1;
      int new_linear_idom = -1;
      for (unsigned pred_idx : block.logical_preds) {
         if ((int)program->blocks[pred_idx].logical_idom == -1)
            continue;

         if (new_logical_idom == -1) {
            new_logical_idom = pred_idx;
            continue;
         }

         while ((int)pred_idx != new_logical_idom) {
            if ((int)pred_idx > new_logical_idom)
               pred_idx = program->blocks[pred_idx].logical_idom;
            if ((int)pred_idx < new_logical_idom)
               new_logical_idom = program->blocks[new_logical_idom].logical_idom;
         }
      }

      for (unsigned pred_idx : block.linear_preds) {
         if ((int)program->blocks[pred_idx].linear_idom == -1)
            continue;

         if (new_linear_idom == -1) {
            new_linear_idom = pred_idx;
            continue;
         }

         while ((int)pred_idx != new_linear_idom) {
            if ((int)pred_idx > new_linear_idom)
               pred_idx = program->blocks[pred_idx].linear_idom;
            if ((int)pred_idx < new_linear_idom)
               new_linear_idom = program->blocks[new_linear_idom].linear_idom;
         }
      }

      block.logical_idom = new_logical_idom;
      block.linear_idom = new_linear_idom;
   }
}

} // namespace aco
#endif
