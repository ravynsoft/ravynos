/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_builder.h"
#include "agx_compiler.h"
#include "agx_opcodes.h"

/*
 * AGX control flow instructions predicate out threads. No forward branches are
 * inserted during instruction selection, only backwards branches at the end of
 * loops exist before this pass. This means, prior to this pass, we would always
 * execute both sides of an if.
 *
 * To improve performance, this pass inserts conservative forward branches after
 * if_*cmp, else_*cmp, and break_if_*cmp instructions, jumping the
 * subgroup to their logical destination if all threads in the subgroup are
 * inactive. This has the effect of skipping over the unexecuted half of an if.
 * That means this pass is critical for control flow performance.
 */

/* Estimated cost of inserting a jmp_exec_none. This value is tuned to Dolphin
 * ubershaders. It needs to be retuned in lockstep with changes to the cost
 * estimation heuristic.
 */
#define COST_JMP (19)

static uint32_t
cost_instr(agx_instr *I)
{
   /* TODO: Better heuristic */
   switch (I->op) {
   case AGX_OPCODE_DEVICE_LOAD:
   case AGX_OPCODE_TEXTURE_LOAD:
   case AGX_OPCODE_TEXTURE_SAMPLE:
      return 10;
   default:
      return 1;
   }
}

/*
 * Estimate the cost between the instruction and the branch target. This is an
 * input for our heuristic. The branch target is guaranteed to be a forward
 * branch.
 */
static uint32_t
cost_between(agx_context *ctx, agx_block *from, agx_instr *from_I,
             agx_block *target, bool skip_to_end_of_target)
{
   uint32_t cost = 0;

   /* Consider the cost in the rest of this block */
   if (from_I != agx_last_instr(from)) {
      agx_foreach_instr_in_block_from(from, J, from_I) {
         /* If we reach the end, we're done */
         if (from == target && skip_to_end_of_target &&
             J == agx_last_instr(target))
            break;

         cost += cost_instr(J);
      }
   }

   if (from == target)
      return cost;

   /* Consider the cost in the subsequent blocks */
   agx_foreach_block_from(ctx, from, block) {
      if (block == from)
         continue;

      if (block == target && !skip_to_end_of_target)
         break;

      agx_foreach_instr_in_block(block, I) {
         if (block == target && I == agx_last_instr(target))
            break;

         cost += cost_instr(I);
      }

      if (block == target) {
         assert(skip_to_end_of_target);
         break;
      }
   }

   return cost;
}

static void
try_insert_jmp(agx_context *ctx, agx_block *from, agx_instr *from_I,
               agx_block *target, bool skip_to_end_of_target,
               unsigned inverse_probability)
{
   agx_builder b = agx_init_builder(ctx, agx_after_instr(from_I));

   /* If the control flow instruction was only inserted for its side effects,
    * there is nowhere to jump. Bail.
    */
   if (!target)
      return;

   /* If we do not insert a jump, we execute the predicated instructions
    * unconditionally, with an expected cost C.
    *
    * If we do insert a jump, then we pay the cost J of the jump, AND if we do
    * not take the jump, also the cost of the instructions C. The expected cost
    * if we insert a jump is therefore J + P(not all threads inactive) C.
    *
    * Therefore, we should insert a jump if:
    *
    *    J + P(not all threads inactive) C < C
    *
    * To model the implicit (i-cache, etc) costs of inserting a jump
    * instruction, we tie break conservatively, comparing with < instead of <=.
    *
    * Rearranging terms, we should NOT insert a jump if:
    *
    *    C < J / P(all threads inactive).
    */
   uint32_t cost_instructions =
      cost_between(ctx, from, from_I, target, skip_to_end_of_target);

   if (cost_instructions < COST_JMP * inverse_probability)
      return;

   /* It looks like inserting a jump will be a win. Do so. */
   if (skip_to_end_of_target)
      agx_jmp_exec_none_after(&b, target);
   else
      agx_jmp_exec_none(&b, target);
}

void
agx_opt_jmp_none(agx_context *ctx)
{
   agx_foreach_block(ctx, blk) {
      /* Handle the beginning of blocks */
      agx_instr *first_ = agx_first_instr(blk);
      if (first_ && (first_->op == AGX_OPCODE_ELSE_ICMP ||
                     first_->op == AGX_OPCODE_ELSE_FCMP)) {

         /* The target of the else is the last block of the else, so we skip
          * to the end of the block (to start execution with the pop_exec).
          */
         try_insert_jmp(ctx, blk, first_, first_->target, true, 2);
      } else if (first_ &&
                 (first_->op == AGX_OPCODE_BREAK_IF_ICMP ||
                  first_->op == AGX_OPCODE_BREAK_IF_FCMP) &&
                 first_->nest == 1) {
         /* The target of the break is the block immediately after the end of
          * the loop, so jump to the end of the previous block to get the
          * appropriate pop_exec.
          *
          * Also, note we only do this for nest=1 to ensure we don't insert
          * jumps inside if-statements inside breaks. We can't insert a
          * jmp_exec_none inside the if because it would break out of the loop
          * for threads that are still running the loop but merely predicated
          * out due to the if-condition. This is similarly why we don't bother
          * handling unconditional break.
          *
          * TODO: This is not optimal, but fixing this would require
          * considerably more CFG gymnastics.
          */
         agx_block *target = agx_prev_block(first_->target);
         try_insert_jmp(ctx, blk, first_, target, true, 10);
      }

      /* Handle end of block instructions */
      agx_foreach_instr_in_block_rev(blk, I) {
         if (!instr_after_logical_end(I))
            break;

         if (I->op == AGX_OPCODE_IF_ICMP || I->op == AGX_OPCODE_IF_FCMP) {
            try_insert_jmp(ctx, blk, I, I->target, false, 2);
            break;
         }
      }
   }
}
