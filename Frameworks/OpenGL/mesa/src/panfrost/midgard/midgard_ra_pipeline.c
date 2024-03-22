/*
 * Copyright (C) 2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019 Collabora, Ltd.
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

/* Creates pipeline registers. This is a prepass run before the main register
 * allocator but after scheduling, once bundles are created. It works by
 * iterating the scheduled IR, checking if a value is ever used after the end
 * of the current bundle. If it is not, it is promoted to a bundle-specific
 * pipeline register.
 *
 * Pipeline registers are only written from the first two stages of the
 * pipeline (vmul/sadd) lasting the duration of the bundle only. There are two
 * 128-bit pipeline registers available (r24/r25). The upshot is that no actual
 * register allocation is needed; we can _always_ promote a value to a pipeline
 * register, liveness permitting. This greatly simplifies the logic of this
 * passing, negating the need for a proper RA like work registers.
 */

static bool
mir_pipeline_ins(compiler_context *ctx, midgard_block *block,
                 midgard_bundle *bundle, unsigned i, unsigned pipeline_count)
{
   midgard_instruction *ins = bundle->instructions[i];

   /* Our goal is to create a pipeline register. Pipeline registers are
    * created at the start of the bundle and are destroyed at the end. So
    * we conservatively require:
    *
    *  1. Each component read in the second stage is written in the first stage.
    *  2. The index is not live after the bundle.
    *  3. We're not a special index (writeout, conditionals, ..)
    *
    * Rationale: #1 ensures that there is no need to go before the
    * creation of the bundle, so the pipeline register can exist. #2 is
    * since the pipeline register will be destroyed at the end. This
    * ensures that nothing will try to read/write the pipeline register
    * once it is not live, and that there's no need to go earlier. */

   unsigned node = ins->dest;
   unsigned read_mask = 0;

   if (node >= SSA_FIXED_MINIMUM)
      return false;

   if (node == ctx->blend_src1)
      return false;

   /* Analyze the bundle for a per-byte read mask */

   for (unsigned j = 0; j < bundle->instruction_count; ++j) {
      midgard_instruction *q = bundle->instructions[j];

      /* The fragment colour can't be pipelined (well, it is
       * pipelined in r0, but this is a delicate dance with
       * scheduling and RA, not for us to worry about) */

      if (q->compact_branch && q->writeout && mir_has_arg(q, node))
         return false;

      if (q->unit < UNIT_VADD)
         continue;
      read_mask |= mir_bytemask_of_read_components(q, node);
   }

   /* Now check what's written in the beginning stage  */
   for (unsigned j = 0; j < bundle->instruction_count; ++j) {
      midgard_instruction *q = bundle->instructions[j];
      if (q->unit >= UNIT_VADD)
         break;
      if (q->dest != node)
         continue;

      /* Remove the written mask from the read requirements */
      read_mask &= ~mir_bytemask(q);
   }

   /* Check for leftovers */
   if (read_mask)
      return false;

   /* We want to know if we live after this bundle, so check if
    * we're live after the last instruction of the bundle */

   midgard_instruction *end =
      bundle->instructions[bundle->instruction_count - 1];

   if (mir_is_live_after(ctx, block, end, ins->dest))
      return false;

   /* We're only live in this bundle -- pipeline! */
   unsigned preg = SSA_FIXED_REGISTER(24 + pipeline_count);

   for (unsigned j = 0; j < bundle->instruction_count; ++j) {
      midgard_instruction *q = bundle->instructions[j];

      if (q->unit >= UNIT_VADD)
         mir_rewrite_index_src_single(q, node, preg);
      else
         mir_rewrite_index_dst_single(q, node, preg);
   }

   return true;
}

void
mir_create_pipeline_registers(compiler_context *ctx)
{
   mir_invalidate_liveness(ctx);

   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;

      mir_foreach_bundle_in_block(block, bundle) {
         if (!mir_is_alu_bundle(bundle))
            continue;
         if (bundle->instruction_count < 2)
            continue;

         /* Only first 2 instructions could pipeline */
         bool succ = mir_pipeline_ins(ctx, block, bundle, 0, 0);
         mir_pipeline_ins(ctx, block, bundle, 1, succ);
      }
   }
}
