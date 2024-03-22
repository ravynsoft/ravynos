/*
 * Copyright (C) 2020 Collabora Ltd.
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
 *
 * Authors (Collabora):
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "compiler.h"

/* Assign dependency slots to each clause and calculate dependencies, This pass
 * must be run after scheduling.
 *
 * 1. A clause that does not produce a message must use the sentinel slot #0
 * 2a. A clause that depends on the results of a previous message-passing
 * instruction must depend on that instruction's dependency slot, unless all
 * reaching code paths already depended on it.
 * 2b. More generally, any dependencies must be encoded. This includes
 * Write-After-Write and Write-After-Read hazards with LOAD/STORE to memory.
 * 3. The shader must wait on slot #6 before running BLEND, ATEST
 * 4. The shader must wait on slot #7 before running BLEND, ST_TILE
 * 5. ATEST, ZS_EMIT must be issued with slot #0
 * 6. BARRIER must be issued with slot #7 and wait on every active slot.
 * 7. Only slots #0 through #5 may be used for clauses not otherwise specified.
 * 8. If a clause writes to a read staging register of an unresolved
 * dependency, it must set a staging barrier.
 *
 * Note it _is_ legal to reuse slots for multiple message passing instructions
 * with overlapping liveness, albeit with a slight performance penalty. As such
 * the problem is significantly easier than register allocation, rather than
 * spilling we may simply reuse slots. (TODO: does this have an optimal
 * linear-time solution).
 *
 * Within these constraints we are free to assign slots as we like. This pass
 * attempts to minimize stalls (TODO).
 */

#define BI_NUM_GENERAL_SLOTS 6
#define BI_NUM_SLOTS         8
#define BI_NUM_REGISTERS     64
#define BI_SLOT_SERIAL       0 /* arbitrary */

/*
 * Due to the crude scoreboarding we do, we need to serialize varying loads and
 * memory access. Identify these instructions here.
 */
static bool
bi_should_serialize(bi_instr *I)
{
   /* For debug, serialize everything to disable scoreboard opts */
   if (bifrost_debug & BIFROST_DBG_NOSB)
      return true;

   /* Although nominally on the attribute unit, image loads have the same
    * coherency requirements as general memory loads. Serialize them for
    * now until we can do something more clever.
    */
   if (I->op == BI_OPCODE_LD_ATTR_TEX)
      return true;

   switch (bi_opcode_props[I->op].message) {
   case BIFROST_MESSAGE_VARYING:
   case BIFROST_MESSAGE_LOAD:
   case BIFROST_MESSAGE_STORE:
   case BIFROST_MESSAGE_ATOMIC:
      return true;
   default:
      return false;
   }
}

/* Given a scoreboard model, choose a slot for a clause wrapping a given
 * message passing instruction. No side effects. */

static unsigned
bi_choose_scoreboard_slot(bi_instr *message)
{
   /* ATEST, ZS_EMIT must be issued with slot #0 */
   if (message->op == BI_OPCODE_ATEST || message->op == BI_OPCODE_ZS_EMIT)
      return 0;

   /* BARRIER must be issued with slot #7 */
   if (message->op == BI_OPCODE_BARRIER)
      return 7;

   /* For now, make serialization is easy */
   if (bi_should_serialize(message))
      return BI_SLOT_SERIAL;

   return 0;
}

static uint64_t
bi_read_mask(bi_instr *I, bool staging_only)
{
   uint64_t mask = 0;

   if (staging_only && !bi_opcode_props[I->op].sr_read)
      return mask;

   bi_foreach_src(I, s) {
      if (I->src[s].type == BI_INDEX_REGISTER) {
         unsigned reg = I->src[s].value;
         unsigned count = bi_count_read_registers(I, s);

         mask |= (BITFIELD64_MASK(count) << reg);
      }

      if (staging_only)
         break;
   }

   return mask;
}

static uint64_t
bi_write_mask(bi_instr *I)
{
   uint64_t mask = 0;

   bi_foreach_dest(I, d) {
      if (bi_is_null(I->dest[d]))
         continue;

      assert(I->dest[d].type == BI_INDEX_REGISTER);

      unsigned reg = I->dest[d].value;
      unsigned count = bi_count_write_registers(I, d);

      mask |= (BITFIELD64_MASK(count) << reg);
   }

   /* Instructions like AXCHG.i32 unconditionally both read and write
    * staging registers. Even if we discard the result, the write still
    * happens logically and needs to be included in our calculations.
    * Obscurely, ATOM_CX is sr_write but can ignore the staging register in
    * certain circumstances; this does not require consideration.
    */
   if (bi_opcode_props[I->op].sr_write && I->nr_dests && I->nr_srcs &&
       bi_is_null(I->dest[0]) && !bi_is_null(I->src[0])) {

      unsigned reg = I->src[0].value;
      unsigned count = bi_count_write_registers(I, 0);

      mask |= (BITFIELD64_MASK(count) << reg);
   }

   return mask;
}

/* Update the scoreboard model to assign an instruction to a given slot */

static void
bi_push_clause(struct bi_scoreboard_state *st, bi_clause *clause)
{
   bi_instr *I = clause->message;
   unsigned slot = clause->scoreboard_id;

   if (!I)
      return;

   st->read[slot] |= bi_read_mask(I, true);

   if (bi_opcode_props[I->op].sr_write)
      st->write[slot] |= bi_write_mask(I);
}

/* Adds a dependency on each slot writing any specified register */

static void
bi_depend_on_writers(bi_clause *clause, struct bi_scoreboard_state *st,
                     uint64_t regmask)
{
   for (unsigned slot = 0; slot < ARRAY_SIZE(st->write); ++slot) {
      if (!(st->write[slot] & regmask))
         continue;

      st->write[slot] = 0;
      st->read[slot] = 0;

      clause->dependencies |= BITFIELD_BIT(slot);
   }
}

static void
bi_set_staging_barrier(bi_clause *clause, struct bi_scoreboard_state *st,
                       uint64_t regmask)
{
   for (unsigned slot = 0; slot < ARRAY_SIZE(st->read); ++slot) {
      if (!(st->read[slot] & regmask))
         continue;

      st->read[slot] = 0;
      clause->staging_barrier = true;
   }
}

/* Sets the dependencies for a given clause, updating the model */

static void
bi_set_dependencies(bi_block *block, bi_clause *clause,
                    struct bi_scoreboard_state *st)
{
   bi_foreach_instr_in_clause(block, clause, I) {
      uint64_t read = bi_read_mask(I, false);
      uint64_t written = bi_write_mask(I);

      /* Read-after-write; write-after-write */
      bi_depend_on_writers(clause, st, read | written);

      /* Write-after-read */
      bi_set_staging_barrier(clause, st, written);
   }

   /* LD_VAR instructions must be serialized per-quad. Just always depend
    * on any LD_VAR instructions. This isn't optimal, but doing better
    * requires divergence-aware data flow analysis.
    *
    * Similarly, memory loads/stores need to be synchronized. For now,
    * force them to be serialized. This is not optimal.
    */
   if (clause->message && bi_should_serialize(clause->message))
      clause->dependencies |= BITFIELD_BIT(BI_SLOT_SERIAL);

   /* Barriers must wait on all slots to flush existing work. It might be
    * possible to skip this with more information about the barrier. For
    * now, be conservative.
    */
   if (clause->message && clause->message->op == BI_OPCODE_BARRIER)
      clause->dependencies |= BITFIELD_MASK(BI_NUM_GENERAL_SLOTS);
}

static bool
scoreboard_block_update(bi_block *blk)
{
   bool progress = false;

   /* pending_in[s] = sum { p in pred[s] } ( pending_out[p] ) */
   bi_foreach_predecessor(blk, pred) {
      for (unsigned i = 0; i < BI_NUM_SLOTS; ++i) {
         blk->scoreboard_in.read[i] |= (*pred)->scoreboard_out.read[i];
         blk->scoreboard_in.write[i] |= (*pred)->scoreboard_out.write[i];
      }
   }

   struct bi_scoreboard_state state = blk->scoreboard_in;

   /* Assign locally */

   bi_foreach_clause_in_block(blk, clause) {
      bi_set_dependencies(blk, clause, &state);
      bi_push_clause(&state, clause);
   }

   /* To figure out progress, diff scoreboard_out */

   for (unsigned i = 0; i < BI_NUM_SLOTS; ++i)
      progress |= !!memcmp(&state, &blk->scoreboard_out, sizeof(state));

   blk->scoreboard_out = state;

   return progress;
}

void
bi_assign_scoreboard(bi_context *ctx)
{
   u_worklist worklist;
   bi_worklist_init(ctx, &worklist);

   /* First, assign slots. */
   bi_foreach_block(ctx, block) {
      bi_foreach_clause_in_block(block, clause) {
         if (clause->message) {
            unsigned slot = bi_choose_scoreboard_slot(clause->message);
            clause->scoreboard_id = slot;
         }
      }

      bi_worklist_push_tail(&worklist, block);
   }

   /* Next, perform forward data flow analysis to calculate dependencies */
   while (!u_worklist_is_empty(&worklist)) {
      /* Pop from the front for forward analysis */
      bi_block *blk = bi_worklist_pop_head(&worklist);

      if (scoreboard_block_update(blk)) {
         bi_foreach_successor(blk, succ)
            bi_worklist_push_tail(&worklist, succ);
      }
   }

   u_worklist_fini(&worklist);
}
