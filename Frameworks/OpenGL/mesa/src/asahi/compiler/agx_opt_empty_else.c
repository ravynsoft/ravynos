/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "util/list.h"
#include "agx_builder.h"
#include "agx_compiler.h"
#include "agx_opcodes.h"

/*
 * Detect blocks with the sole contents:
 *
 *    else n=1
 *    pop_exec n=1
 *
 * The else instruction is a no-op. To see that, consider the pseudocode for the
 * sequence of operations "else n=1; pop_exec n=1":
 *
 *   # else n=1
 *   if r0l == 0:
 *     r0l = 1
 *   elif r0l == 1:
 *     if [...]:
 *       r0l = 0
 *     else:
 *       r0l = 1
 *   exec_mask[thread] = (r0l == 0)
 *
 *   # pop_exec n=1
 *   if r0l > 0:
 *     r0l -= 1
 *   exec_mask[thread] = (r0l == 0)
 *
 * That logic code simplifies to:
 *
 *   if r0l > 0:
 *     r0l = r0l - 1
 *   exec_mask[thread] = (r0l == 0)
 *
 * which is just "pop_exec n=1".
 *
 * Therefore, this pass detects these blocks and deletes the else instruction.
 * This has the effect of removing empty else blocks. Logically, that creates
 * critical edges, so this pass can only run late (post-RA).
 *
 * The pass itself uses a simple state machine for pattern matching.
 */

enum block_state {
   STATE_ELSE = 0,
   STATE_POP_EXEC,
   STATE_DONE,

   /* Must be last */
   STATE_NONE,
};

static enum block_state
state_for_instr(const agx_instr *I)
{
   switch (I->op) {
   case AGX_OPCODE_ELSE_ICMP:
   case AGX_OPCODE_ELSE_FCMP:
      return (I->nest == 1) ? STATE_ELSE : STATE_NONE;

   case AGX_OPCODE_POP_EXEC:
      return (I->nest == 1) ? STATE_POP_EXEC : STATE_NONE;

   default:
      return STATE_NONE;
   }
}

static bool
match_block(agx_block *blk)
{
   enum block_state state = STATE_ELSE;

   agx_foreach_instr_in_block(blk, I) {
      if (state_for_instr(I) == state)
         state++;
      else
         return false;
   }

   return (state == STATE_DONE);
}

void
agx_opt_empty_else(agx_context *ctx)
{
   agx_foreach_block(ctx, blk) {
      if (match_block(blk)) {
         agx_instr *else_instr = agx_first_instr(blk);
         assert(state_for_instr(else_instr) == STATE_ELSE && "block matched");

         agx_remove_instruction(else_instr);
      }
   }
}
