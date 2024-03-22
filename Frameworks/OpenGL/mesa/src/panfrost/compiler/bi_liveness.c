/*
 * Copyright (C) 2020 Collabora, Ltd.
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "util/u_memory.h"
#include "compiler.h"

void
bi_liveness_ins_update_ssa(BITSET_WORD *live, const bi_instr *I)
{
   bi_foreach_dest(I, d)
      BITSET_CLEAR(live, I->dest[d].value);

   bi_foreach_ssa_src(I, s)
      BITSET_SET(live, I->src[s].value);
}

void
bi_compute_liveness_ssa(bi_context *ctx)
{
   u_worklist worklist;
   u_worklist_init(&worklist, ctx->num_blocks, NULL);

   /* Free any previous liveness, and allocate */
   unsigned words = BITSET_WORDS(ctx->ssa_alloc);

   bi_foreach_block(ctx, block) {
      if (block->ssa_live_in)
         ralloc_free(block->ssa_live_in);

      if (block->ssa_live_out)
         ralloc_free(block->ssa_live_out);

      block->ssa_live_in = rzalloc_array(block, BITSET_WORD, words);
      block->ssa_live_out = rzalloc_array(block, BITSET_WORD, words);

      bi_worklist_push_head(&worklist, block);
   }

   /* Iterate the work list */
   while (!u_worklist_is_empty(&worklist)) {
      /* Pop in reverse order since liveness is a backwards pass */
      bi_block *blk = bi_worklist_pop_head(&worklist);

      /* Update its liveness information */
      memcpy(blk->ssa_live_in, blk->ssa_live_out, words * sizeof(BITSET_WORD));

      bi_foreach_instr_in_block_rev(blk, I) {
         /* Phi nodes are handled separately, so we skip them. As phi nodes are
          * at the beginning and we're iterating backwards, we stop as soon as
          * we hit a phi node.
          */
         if (I->op == BI_OPCODE_PHI)
            break;

         bi_liveness_ins_update_ssa(blk->ssa_live_in, I);
      }

      /* Propagate the live in of the successor (blk) to the live out of
       * predecessors.
       *
       * Phi nodes are logically on the control flow edge and act in parallel.
       * To handle when propagating, we kill writes from phis and make live the
       * corresponding sources.
       */
      bi_foreach_predecessor(blk, pred) {
         BITSET_WORD *live = ralloc_array(blk, BITSET_WORD, words);
         memcpy(live, blk->ssa_live_in, words * sizeof(BITSET_WORD));

         /* Kill write */
         bi_foreach_instr_in_block(blk, I) {
            if (I->op != BI_OPCODE_PHI)
               break;

            BITSET_CLEAR(live, I->dest[0].value);
         }

         /* Make live the corresponding source */
         bi_foreach_instr_in_block(blk, I) {
            if (I->op != BI_OPCODE_PHI)
               break;

            bi_index operand = I->src[bi_predecessor_index(blk, *pred)];
            if (bi_is_ssa(operand))
               BITSET_SET(live, operand.value);
         }

         BITSET_WORD progress = 0;

         for (unsigned i = 0; i < words; ++i) {
            progress |= live[i] & ~((*pred)->ssa_live_out[i]);
            (*pred)->ssa_live_out[i] |= live[i];
         }

         if (progress != 0)
            bi_worklist_push_tail(&worklist, *pred);
      }
   }

   u_worklist_fini(&worklist);
}
