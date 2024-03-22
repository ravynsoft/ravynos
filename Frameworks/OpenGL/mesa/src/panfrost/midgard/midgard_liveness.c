/*
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019-2020 Collabora, Ltd.
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

void
mir_liveness_ins_update(uint16_t *live, midgard_instruction *ins, unsigned max)
{
   /* live_in[s] = GEN[s] + (live_out[s] - KILL[s]) */

   pan_liveness_kill(live, ins->dest, max, mir_bytemask(ins));

   mir_foreach_src(ins, src) {
      unsigned node = ins->src[src];
      unsigned bytemask = mir_bytemask_of_read_components(ins, node);

      pan_liveness_gen(live, node, max, bytemask);
   }
}

static void
mir_liveness_ins_update_wrap(uint16_t *live, void *ins, unsigned max)
{
   mir_liveness_ins_update(live, (midgard_instruction *)ins, max);
}

void
mir_compute_liveness(compiler_context *ctx)
{
   /* If we already have fresh liveness, nothing to do */
   if (ctx->metadata & MIDGARD_METADATA_LIVENESS)
      return;

   mir_compute_temp_count(ctx);
   pan_compute_liveness(&ctx->blocks, ctx->temp_count,
                        mir_liveness_ins_update_wrap);

   /* Liveness is now valid */
   ctx->metadata |= MIDGARD_METADATA_LIVENESS;
}

/* Once liveness data is no longer valid, call this */

void
mir_invalidate_liveness(compiler_context *ctx)
{
   /* If we didn't already compute liveness, there's nothing to do */
   if (!(ctx->metadata & MIDGARD_METADATA_LIVENESS))
      return;

   pan_free_liveness(&ctx->blocks);

   /* It's now invalid regardless */
   ctx->metadata &= ~MIDGARD_METADATA_LIVENESS;
}

bool
mir_is_live_after(compiler_context *ctx, midgard_block *block,
                  midgard_instruction *start, int src)
{
   mir_compute_liveness(ctx);

   /* Check whether we're live in the successors */

   if (pan_liveness_get(block->base.live_out, src, ctx->temp_count))
      return true;

   /* Check the rest of the block for liveness */

   mir_foreach_instr_in_block_from(block, ins, mir_next_op(start)) {
      if (mir_has_arg(ins, src))
         return true;
   }

   return false;
}
