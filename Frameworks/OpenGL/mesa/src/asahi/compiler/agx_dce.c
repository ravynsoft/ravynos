/*
 * Copyright 2021 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_compiler.h"

/* SSA-based scalar dead code elimination */

void
agx_dce(agx_context *ctx, bool partial)
{
   bool progress;
   do {
      progress = false;

      BITSET_WORD *seen = calloc(BITSET_WORDS(ctx->alloc), sizeof(BITSET_WORD));

      agx_foreach_instr_global(ctx, I) {
         agx_foreach_ssa_src(I, s) {
            BITSET_SET(seen, I->src[s].value);
         }
      }

      agx_foreach_instr_global_safe_rev(ctx, I) {
         bool needed = false;

         agx_foreach_ssa_dest(I, d) {
            /* Eliminate destinations that are never read, as RA needs to
             * handle them specially. Visible only for instructions that write
             * multiple destinations (splits) or that write a destination but
             * cannot be DCE'd (atomics).
             */
            if (BITSET_TEST(seen, I->dest[d].value)) {
               needed = true;
            } else if (partial) {
               I->dest[d] = agx_null();
               progress = true;
            }
         }

         if (!needed && agx_opcodes_info[I->op].can_eliminate) {
            agx_remove_instruction(I);
            progress = true;
         }
      }

      free(seen);
   } while (progress);
}
