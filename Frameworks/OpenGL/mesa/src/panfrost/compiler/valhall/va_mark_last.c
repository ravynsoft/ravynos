/*
 * Copyright (C) 2022 Collabora Ltd.
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

#include "va_compiler.h"
#include "valhall_enums.h"

/*
 * Valhall sources may marked as the last use of a register, according
 * to the following rules:
 *
 * 1. The last use of a register should be marked allowing the hardware
 *    to elide register writes.
 * 2. Staging sources may be read at any time before the asynchronous
 *    instruction completes. If a register is used as both a staging source and
 *    a regular source, the regular source cannot be marked until the program
 *    waits for the asynchronous instruction.
 * 3. Marking a register pair marks both registers in the pair.
 *
 * Last use information follows immediately from (post-RA) liveness analysis:
 * a register is dead immediately after its last use.
 *
 * Staging information follows from scoreboard analysis: do not mark registers
 * that are read by a pending asynchronous instruction. Note that the Valhall
 * scoreboard analysis does not track reads, so we handle that with our own
 * (simplified) scoreboard analysis.
 *
 * Register pairs are marked conservatively: if either register in a pair cannot
 * be marked, do not mark either register.
 */

static uint64_t
bi_staging_read_mask(const bi_instr *I)
{
   uint64_t mask = 0;

   bi_foreach_src(I, s) {
      if (bi_is_staging_src(I, s) && !bi_is_null(I->src[s])) {
         assert(I->src[s].type == BI_INDEX_REGISTER);
         unsigned reg = I->src[s].value;
         unsigned count = bi_count_read_registers(I, s);

         mask |= (BITFIELD64_MASK(count) << reg);
      }
   }

   return mask;
}

static bool
bi_writes_reg(const bi_instr *I, unsigned reg)
{
   bi_foreach_dest(I, d) {
      assert(I->dest[d].type == BI_INDEX_REGISTER);

      unsigned count = bi_count_write_registers(I, d);

      if (reg >= I->dest[d].value && (reg - I->dest[d].value) < count)
         return true;
   }

   return false;
}

static unsigned
waits_on_slot(enum va_flow flow, unsigned slot)
{
   return (flow == VA_FLOW_WAIT) || (flow == VA_FLOW_WAIT0126) ||
          (va_flow_is_wait_or_none(flow) && (flow & BITFIELD_BIT(slot)));
}

static void
scoreboard_update(struct bi_scoreboard_state *st, const bi_instr *I)
{
   /* Mark read staging registers */
   st->read[I->slot] |= bi_staging_read_mask(I);

   /* Unmark registers after they are waited on */
   for (unsigned i = 0; i < VA_NUM_GENERAL_SLOTS; ++i) {
      if (waits_on_slot(I->flow, i))
         st->read[i] = 0;
   }
}

static void
va_analyze_scoreboard_reads(bi_context *ctx)
{
   u_worklist worklist;
   bi_worklist_init(ctx, &worklist);

   bi_foreach_block(ctx, block) {
      bi_worklist_push_tail(&worklist, block);

      /* Reset analysis from previous pass */
      block->scoreboard_in = (struct bi_scoreboard_state){0};
      block->scoreboard_out = (struct bi_scoreboard_state){0};
   }

   /* Perform forward data flow analysis to calculate dependencies */
   while (!u_worklist_is_empty(&worklist)) {
      /* Pop from the front for forward analysis */
      bi_block *blk = bi_worklist_pop_head(&worklist);

      bi_foreach_predecessor(blk, pred) {
         for (unsigned i = 0; i < VA_NUM_GENERAL_SLOTS; ++i)
            blk->scoreboard_in.read[i] |= (*pred)->scoreboard_out.read[i];
      }

      struct bi_scoreboard_state state = blk->scoreboard_in;

      bi_foreach_instr_in_block(blk, I)
         scoreboard_update(&state, I);

      /* If there was progress, reprocess successors */
      if (memcmp(&state, &blk->scoreboard_out, sizeof(state)) != 0) {
         bi_foreach_successor(blk, succ)
            bi_worklist_push_tail(&worklist, succ);
      }

      blk->scoreboard_out = state;
   }

   u_worklist_fini(&worklist);
}

void
va_mark_last(bi_context *ctx)
{
   /* Analyze the shader globally */
   bi_postra_liveness(ctx);
   va_analyze_scoreboard_reads(ctx);

   bi_foreach_block(ctx, block) {
      uint64_t live = block->reg_live_out;

      /* Mark all last uses */
      bi_foreach_instr_in_block_rev(block, I) {
         bi_foreach_src(I, s) {
            if (I->src[s].type != BI_INDEX_REGISTER)
               continue;

            unsigned nr = bi_count_read_registers(I, s);
            uint64_t mask = BITFIELD64_MASK(nr) << I->src[s].value;

            /* If the register dead after this instruction, it's the last use */
            I->src[s].discard = (live & mask) == 0;

            /* If the register is overwritten this cycle, it is implicitly
             * discarded, but that won't show up in the liveness analysis.
             */
            I->src[s].discard |= bi_writes_reg(I, I->src[s].value);
         }

         live = bi_postra_liveness_ins(live, I);
      }

      struct bi_scoreboard_state st = block->scoreboard_in;

      bi_foreach_instr_in_block(block, I) {
         /* Unmark registers read by a pending async instruction */
         bi_foreach_src(I, s) {
            if (!I->src[s].discard)
               continue;

            assert(I->src[s].type == BI_INDEX_REGISTER);

            uint64_t pending_regs = st.read[0] | st.read[1] | st.read[2];
            bool pending = (pending_regs & BITFIELD64_BIT(I->src[s].value));

            if (bi_is_staging_src(I, s) || pending)
               I->src[s].discard = false;
         }

         /* Unmark register pairs where one half must be preserved */
         bi_foreach_src(I, s) {
            /* Only look for "real" architectural registers */
            if (s >= 3)
               break;

            if (va_src_info(I->op, s).size == VA_SIZE_64) {
               bool both_discard = I->src[s].discard && I->src[s + 1].discard;

               I->src[s + 0].discard = both_discard;
               I->src[s + 1].discard = both_discard;
            }
         }

         scoreboard_update(&st, I);
      }
   }
}
