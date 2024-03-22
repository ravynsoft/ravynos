/*
 * Copyright 2021 Alyssa Rosenzweig
 * Copyright 2019-2020 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "util/list.h"
#include "util/set.h"
#include "util/u_memory.h"
#include "agx_compiler.h"

/* Liveness analysis is a backwards-may dataflow analysis pass. Within a block,
 * we compute live_out from live_in. The intrablock pass is linear-time. It
 * returns whether progress was made. */

/* live_in[s] = GEN[s] + (live_out[s] - KILL[s]) */

void
agx_liveness_ins_update(BITSET_WORD *live, agx_instr *I)
{
   agx_foreach_ssa_dest(I, d)
      BITSET_CLEAR(live, I->dest[d].value);

   agx_foreach_ssa_src(I, s) {
      /* If the source is not live after this instruction, but becomes live
       * at this instruction, this is the use that kills the source
       */
      I->src[s].kill = !BITSET_TEST(live, I->src[s].value);
      BITSET_SET(live, I->src[s].value);
   }
}

/* Globally, liveness analysis uses a fixed-point algorithm based on a
 * worklist. We initialize a work list with the exit block. We iterate the work
 * list to compute live_in from live_out for each block on the work list,
 * adding the predecessors of the block to the work list if we made progress.
 */

void
agx_compute_liveness(agx_context *ctx)
{
   u_worklist worklist;
   u_worklist_init(&worklist, ctx->num_blocks, NULL);

   /* Free any previous liveness, and allocate */
   unsigned words = BITSET_WORDS(ctx->alloc);

   agx_foreach_block(ctx, block) {
      if (block->live_in)
         ralloc_free(block->live_in);

      if (block->live_out)
         ralloc_free(block->live_out);

      block->live_in = rzalloc_array(block, BITSET_WORD, words);
      block->live_out = rzalloc_array(block, BITSET_WORD, words);

      agx_worklist_push_head(&worklist, block);
   }

   /* Iterate the work list */
   while (!u_worklist_is_empty(&worklist)) {
      /* Pop in reverse order since liveness is a backwards pass */
      agx_block *blk = agx_worklist_pop_head(&worklist);

      /* Update its liveness information */
      memcpy(blk->live_in, blk->live_out, words * sizeof(BITSET_WORD));

      agx_foreach_instr_in_block_rev(blk, I) {
         if (I->op != AGX_OPCODE_PHI)
            agx_liveness_ins_update(blk->live_in, I);
      }

      /* Propagate the live in of the successor (blk) to the live out of
       * predecessors.
       *
       * Phi nodes are logically on the control flow edge and act in parallel.
       * To handle when propagating, we kill writes from phis and make live the
       * corresponding sources.
       */
      agx_foreach_predecessor(blk, pred) {
         BITSET_WORD *live = ralloc_array(blk, BITSET_WORD, words);
         memcpy(live, blk->live_in, words * sizeof(BITSET_WORD));

         /* Kill write */
         agx_foreach_phi_in_block(blk, phi) {
            assert(phi->dest[0].type == AGX_INDEX_NORMAL);
            BITSET_CLEAR(live, phi->dest[0].value);
         }

         /* Make live the corresponding source */
         agx_foreach_phi_in_block(blk, phi) {
            agx_index operand = phi->src[agx_predecessor_index(blk, *pred)];
            if (operand.type == AGX_INDEX_NORMAL)
               BITSET_SET(live, operand.value);
         }

         bool progress = false;

         for (unsigned i = 0; i < words; ++i) {
            progress |= live[i] & ~((*pred)->live_out[i]);
            (*pred)->live_out[i] |= live[i];
         }

         if (progress)
            agx_worklist_push_tail(&worklist, *pred);
      }
   }

   u_worklist_fini(&worklist);
}
