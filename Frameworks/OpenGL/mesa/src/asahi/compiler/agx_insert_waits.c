/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_builder.h"
#include "agx_compiler.h"
#include "agx_debug.h"

#define AGX_MAX_PENDING (8)

/*
 * Returns whether an instruction is asynchronous and needs a scoreboard slot
 */
static bool
instr_is_async(agx_instr *I)
{
   return agx_opcodes_info[I->op].immediates & AGX_IMMEDIATE_SCOREBOARD;
}

struct slot {
   /* Set of registers this slot is currently writing */
   BITSET_DECLARE(writes, AGX_NUM_REGS);

   /* Number of pending messages on this slot. Must not exceed
    * AGX_MAX_PENDING for correct results.
    */
   uint8_t nr_pending;
};

/*
 * Insert waits within a block to stall after every async instruction. Useful
 * for debugging.
 */
static void
agx_insert_waits_trivial(agx_context *ctx, agx_block *block)
{
   agx_foreach_instr_in_block_safe(block, I) {
      if (instr_is_async(I)) {
         agx_builder b = agx_init_builder(ctx, agx_after_instr(I));
         agx_wait(&b, I->scoreboard);
      }
   }
}

/*
 * Insert waits within a block, assuming scoreboard slots have already been
 * assigned. This waits for everything at the end of the block, rather than
 * doing something more intelligent/global. This should be optimized.
 *
 * XXX: Do any instructions read their sources asynchronously?
 */
static void
agx_insert_waits_local(agx_context *ctx, agx_block *block)
{
   struct slot slots[2] = {0};

   agx_foreach_instr_in_block_safe(block, I) {
      uint8_t wait_mask = 0;

      /* Check for read-after-write */
      agx_foreach_src(I, s) {
         if (I->src[s].type != AGX_INDEX_REGISTER)
            continue;

         unsigned nr_read = agx_index_size_16(I->src[s]);
         for (unsigned slot = 0; slot < ARRAY_SIZE(slots); ++slot) {
            if (BITSET_TEST_RANGE(slots[slot].writes, I->src[s].value,
                                  I->src[s].value + nr_read - 1))
               wait_mask |= BITSET_BIT(slot);
         }
      }

      /* Check for write-after-write */
      agx_foreach_dest(I, d) {
         if (I->dest[d].type != AGX_INDEX_REGISTER)
            continue;

         unsigned nr_writes = agx_index_size_16(I->dest[d]);
         for (unsigned slot = 0; slot < ARRAY_SIZE(slots); ++slot) {
            if (BITSET_TEST_RANGE(slots[slot].writes, I->dest[d].value,
                                  I->dest[d].value + nr_writes - 1))
               wait_mask |= BITSET_BIT(slot);
         }
      }

      /* Check for barriers */
      if (I->op == AGX_OPCODE_THREADGROUP_BARRIER ||
          I->op == AGX_OPCODE_MEMORY_BARRIER) {

         for (unsigned slot = 0; slot < ARRAY_SIZE(slots); ++slot) {
            if (slots[slot].nr_pending)
               wait_mask |= BITSET_BIT(slot);
         }
      }

      /* Try to assign a free slot */
      if (instr_is_async(I)) {
         for (unsigned slot = 0; slot < ARRAY_SIZE(slots); ++slot) {
            if (slots[slot].nr_pending == 0) {
               I->scoreboard = slot;
               break;
            }
         }
      }

      /* Check for slot overflow */
      if (instr_is_async(I) &&
          slots[I->scoreboard].nr_pending >= AGX_MAX_PENDING)
         wait_mask |= BITSET_BIT(I->scoreboard);

      /* Insert the appropriate waits, clearing the slots */
      u_foreach_bit(slot, wait_mask) {
         agx_builder b = agx_init_builder(ctx, agx_before_instr(I));
         agx_wait(&b, slot);

         BITSET_ZERO(slots[slot].writes);
         slots[slot].nr_pending = 0;
      }

      /* Record access */
      if (instr_is_async(I)) {
         agx_foreach_dest(I, d) {
            if (agx_is_null(I->dest[d]))
               continue;

            assert(I->dest[d].type == AGX_INDEX_REGISTER);
            BITSET_SET_RANGE(
               slots[I->scoreboard].writes, I->dest[d].value,
               I->dest[d].value + agx_index_size_16(I->dest[d]) - 1);
         }

         slots[I->scoreboard].nr_pending++;
      }
   }

   /* If there are outstanding messages, wait for them. We don't do this for the
    * exit block, though, since nothing else will execute in the shader so
    * waiting is pointless.
    */
   if (block != agx_exit_block(ctx)) {
      agx_builder b = agx_init_builder(ctx, agx_after_block_logical(block));

      for (unsigned slot = 0; slot < ARRAY_SIZE(slots); ++slot) {
         if (slots[slot].nr_pending)
            agx_wait(&b, slot);
      }
   }
}

/*
 * Assign scoreboard slots to asynchronous instructions and insert waits for the
 * appropriate hazard tracking.
 */
void
agx_insert_waits(agx_context *ctx)
{
   agx_foreach_block(ctx, block) {
      if (agx_compiler_debug & AGX_DBG_WAIT)
         agx_insert_waits_trivial(ctx, block);
      else
         agx_insert_waits_local(ctx, block);
   }
}
