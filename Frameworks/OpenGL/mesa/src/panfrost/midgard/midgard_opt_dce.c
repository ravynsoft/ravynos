/*
 * Copyright (C) 2018 Alyssa Rosenzweig
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

#include "util/u_memory.h"
#include "compiler.h"
#include "midgard_ops.h"

/* SIMD-aware dead code elimination. Perform liveness analysis step-by-step,
 * removing dead components. If an instruction ends up with a zero mask, the
 * instruction in total is dead and should be removed. */

static bool
can_cull_mask(compiler_context *ctx, midgard_instruction *ins)
{
   if (ins->dest >= ctx->temp_count)
      return false;

   if (ins->dest == ctx->blend_src1)
      return false;

   if (ins->type == TAG_LOAD_STORE_4)
      if (load_store_opcode_props[ins->op].props & LDST_SPECIAL_MASK)
         return false;

   return true;
}

static bool
can_dce(midgard_instruction *ins)
{
   if (ins->mask)
      return false;

   if (ins->compact_branch)
      return false;

   if (ins->type == TAG_LOAD_STORE_4)
      if (load_store_opcode_props[ins->op].props & LDST_SIDE_FX)
         return false;

   if (ins->type == TAG_TEXTURE_4)
      if (ins->op == midgard_tex_op_barrier)
         return false;

   return true;
}

static bool
midgard_opt_dead_code_eliminate_block(compiler_context *ctx,
                                      midgard_block *block)
{
   bool progress = false;

   uint16_t *live =
      mem_dup(block->base.live_out, ctx->temp_count * sizeof(uint16_t));

   mir_foreach_instr_in_block_rev(block, ins) {
      if (can_cull_mask(ctx, ins)) {
         unsigned type_size = nir_alu_type_get_type_size(ins->dest_type);
         unsigned round_size = type_size;
         unsigned oldmask = ins->mask;

         /* Make sure we're packable */
         if (type_size < 32 && ins->type == TAG_LOAD_STORE_4)
            round_size = 32;

         unsigned rounded = mir_round_bytemask_up(live[ins->dest], round_size);
         unsigned cmask = mir_from_bytemask(rounded, type_size);

         ins->mask &= cmask;
         progress |= (ins->mask != oldmask);
      }

      mir_liveness_ins_update(live, ins, ctx->temp_count);
   }

   mir_foreach_instr_in_block_safe(block, ins) {
      if (can_dce(ins)) {
         mir_remove_instruction(ins);
         progress = true;
      }
   }

   free(live);

   return progress;
}

bool
midgard_opt_dead_code_eliminate(compiler_context *ctx)
{
   /* We track liveness. In fact, it's ok if we assume more things are
    * live than they actually are, that just reduces the effectiveness of
    * this iterations lightly. And DCE has the effect of strictly reducing
    * liveness, so we can run DCE across all blocks while only computing
    * liveness at the beginning. */

   mir_invalidate_liveness(ctx);
   mir_compute_liveness(ctx);

   bool progress = false;

   mir_foreach_block(ctx, block) {
      progress |=
         midgard_opt_dead_code_eliminate_block(ctx, (midgard_block *)block);
   }

   return progress;
}

/* Removes dead moves, that is, moves with a destination overwritten before
 * being read. Normally handled implicitly as part of DCE, but this has to run
 * after the out-of-SSA pass */

bool
midgard_opt_dead_move_eliminate(compiler_context *ctx, midgard_block *block)
{
   bool progress = false;

   mir_foreach_instr_in_block_safe(block, ins) {
      if (ins->type != TAG_ALU_4)
         continue;
      if (ins->compact_branch)
         continue;
      if (!OP_IS_MOVE(ins->op))
         continue;

      /* Check if it's overwritten in this block before being read */
      bool overwritten = false;

      mir_foreach_instr_in_block_from(block, q, mir_next_op(ins)) {
         /* Check if used */
         if (mir_has_arg(q, ins->dest))
            break;

         /* Check if overwritten */
         if (q->dest == ins->dest) {
            /* Special case to vec4; component tracking is
             * harder */

            overwritten = (q->mask == 0xF);
            break;
         }
      }

      if (overwritten) {
         mir_remove_instruction(ins);
         progress = true;
      }
   }

   return progress;
}
