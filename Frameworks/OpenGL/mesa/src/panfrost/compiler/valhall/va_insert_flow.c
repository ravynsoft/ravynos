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

#include "bi_builder.h"
#include "va_compiler.h"
#include "valhall_enums.h"

/*
 * Insert flow control into a scheduled and register allocated shader.  This
 * pass runs after scheduling and register allocation. This pass only
 * inserts NOPs with the appropriate flow control modifiers. It should be
 * followed by a cleanup pass to merge flow control modifiers on adjacent
 * instructions, eliminating the NOPs. This decouples optimization from
 * correctness, simplifying both passes.
 *
 * This pass is responsible for calculating dependencies, according to the
 * rules:
 *
 * 1. An instruction that depends on the results of a previous asyncronous
 *    must first wait for that instruction's slot, unless all
 *    reaching code paths already depended on it.
 * 2. More generally, any dependencies must be encoded. This includes
 *    Write-After-Write and Write-After-Read hazards with LOAD/STORE to memory.
 * 3. The shader must wait on slot #6 before running BLEND, ATEST
 * 4. The shader must wait on slot #7 before running BLEND, ST_TILE
 * 6. BARRIER must wait on every active slot.
 *
 * Unlike Bifrost, it is not necessary to worry about outbound staging
 * registers, as the hardware stalls reading staging registers when issuing
 * asynchronous instructions. So we don't track reads in our model of the
 * hardware scoreboard. This makes things a bit simpler.
 *
 * We may reuse slots for multiple asynchronous instructions, though there may
 * be a performance penalty.
 */

#define BI_NUM_REGISTERS 64

/*
 * Insert a NOP instruction with given flow control.
 */
static void
bi_flow(bi_context *ctx, bi_cursor cursor, enum va_flow flow)
{
   bi_builder b = bi_init_builder(ctx, cursor);

   bi_nop(&b)->flow = flow;
}

static uint64_t
bi_read_mask(bi_instr *I)
{
   uint64_t mask = 0;

   bi_foreach_src(I, s) {
      if (I->src[s].type == BI_INDEX_REGISTER) {
         unsigned reg = I->src[s].value;
         unsigned count = bi_count_read_registers(I, s);

         mask |= (BITFIELD64_MASK(count) << reg);
      }
   }

   return mask;
}

static uint64_t
bi_write_mask(bi_instr *I)
{
   uint64_t mask = 0;

   bi_foreach_dest(I, d) {
      assert(I->dest[d].type == BI_INDEX_REGISTER);

      unsigned reg = I->dest[d].value;
      unsigned count = bi_count_write_registers(I, d);

      mask |= (BITFIELD64_MASK(count) << reg);
   }

   return mask;
}

static bool
bi_ld_vary_writes_hidden_register(const bi_instr *I)
{
   /* Only varying loads can write the hidden register */
   if (bi_opcode_props[I->op].message != BIFROST_MESSAGE_VARYING)
      return false;

   /* They only write in some update modes */
   return (I->update == BI_UPDATE_STORE) || (I->update == BI_UPDATE_CLOBBER);
}

static bool
bi_is_memory_access(const bi_instr *I)
{
   /* On the attribute unit but functionally a general memory load */
   if (I->op == BI_OPCODE_LD_ATTR_TEX)
      return true;

   /* UBOs are read-only so there are no ordering constriants */
   if (I->seg == BI_SEG_UBO)
      return false;

   switch (bi_opcode_props[I->op].message) {
   case BIFROST_MESSAGE_LOAD:
   case BIFROST_MESSAGE_STORE:
   case BIFROST_MESSAGE_ATOMIC:
      return true;
   default:
      return false;
   }
}

/* Update the scoreboard model to assign an instruction to a given slot */

static void
bi_push_instr(struct bi_scoreboard_state *st, bi_instr *I)
{
   if (bi_opcode_props[I->op].sr_write)
      st->write[I->slot] |= bi_write_mask(I);

   if (bi_is_memory_access(I))
      st->memory |= BITFIELD_BIT(I->slot);

   if (bi_opcode_props[I->op].message == BIFROST_MESSAGE_VARYING)
      st->varying |= BITFIELD_BIT(I->slot);
}

static uint8_t MUST_CHECK
bi_pop_slot(struct bi_scoreboard_state *st, unsigned slot)
{
   st->write[slot] = 0;
   st->varying &= ~BITFIELD_BIT(slot);
   st->memory &= ~BITFIELD_BIT(slot);

   return BITFIELD_BIT(slot);
}

/* Adds a dependency on each slot writing any specified register */

static uint8_t MUST_CHECK
bi_depend_on_writers(struct bi_scoreboard_state *st, uint64_t regmask)
{
   uint8_t slots = 0;

   for (unsigned slot = 0; slot < ARRAY_SIZE(st->write); ++slot) {
      if (st->write[slot] & regmask)
         slots |= bi_pop_slot(st, slot);
   }

   return slots;
}

/* Sets the dependencies for a given clause, updating the model */

static void
bi_set_dependencies(bi_block *block, bi_instr *I,
                    struct bi_scoreboard_state *st)
{
   /* Depend on writers to handle read-after-write and write-after-write
    * dependencies. Write-after-read dependencies are handled in the hardware
    * where necessary, so we don't worry about them.
    */
   I->flow |= bi_depend_on_writers(st, bi_read_mask(I) | bi_write_mask(I));

   /* Handle write-after-write and write-after-read dependencies for the varying
    * hidden registers. Read-after-write dependencies handled in hardware.
    */
   if (bi_ld_vary_writes_hidden_register(I)) {
      u_foreach_bit(slot, st->varying)
         I->flow |= bi_pop_slot(st, slot);
   }

   /* For now, serialize all memory access */
   if (bi_is_memory_access(I)) {
      u_foreach_bit(slot, st->memory)
         I->flow |= bi_pop_slot(st, slot);
   }

   /* We need to wait for all general slots before a barrier. The reason is
    * unknown. In theory, this is redundant, since the BARRIER instruction will
    * be followed immediately by .wait which waits for all slots. However, that
    * doesn't seem to work properly in practice.
    *
    * The DDK is observed to use the same workaround, going so far as
    * introducing a NOP before a BARRIER at the beginning of a basic block when
    * there are outstanding stores.
    *
    *     NOP.wait12
    *     BARRIER.slot7.wait
    *
    * Luckily, this situation is pretty rare. The wait introduced here can
    * usually be merged into the preceding instruction.
    *
    * We also use the same workaround to serialize all async instructions when
    * debugging this pass with the BIFROST_MESA_DEBUG=nosb option.
    */
   if (I->op == BI_OPCODE_BARRIER || (bifrost_debug & BIFROST_DBG_NOSB)) {
      for (unsigned i = 0; i < VA_NUM_GENERAL_SLOTS; ++i) {
         if (st->write[i] || ((st->varying | st->memory) & BITFIELD_BIT(i)))
            I->flow |= bi_pop_slot(st, i);
      }
   }
}

static bool
scoreboard_block_update(bi_context *ctx, bi_block *blk)
{
   bool progress = false;

   /* pending_in[s] = sum { p in pred[s] } ( pending_out[p] ) */
   bi_foreach_predecessor(blk, pred) {
      for (unsigned i = 0; i < BI_NUM_SLOTS; ++i) {
         blk->scoreboard_in.read[i] |= (*pred)->scoreboard_out.read[i];
         blk->scoreboard_in.write[i] |= (*pred)->scoreboard_out.write[i];
         blk->scoreboard_in.varying |= (*pred)->scoreboard_out.varying;
         blk->scoreboard_in.memory |= (*pred)->scoreboard_out.memory;
      }
   }

   struct bi_scoreboard_state state = blk->scoreboard_in;

   /* Assign locally */

   bi_foreach_instr_in_block(blk, I) {
      bi_set_dependencies(blk, I, &state);
      bi_push_instr(&state, I);
   }

   /* Insert a wait for varyings at the end of the block.
    *
    * A varying load with .store has to wait for all other varying loads
    * in the quad to complete. The bad case looks like:
    *
    *    if (dynamic) {
    *        x = ld_var()
    *    } else {
    *       x = ld_var()
    *    }
    *
    * Logically, a given thread executes only a single ld_var instruction. But
    * if the quad diverges, the second ld_var has to wait for the first ld_var.
    * For correct handling, we need to maintain a physical control flow graph
    * and do the dataflow analysis on that instead of the logical control flow
    * graph. However, this probably doesn't matter much in practice. This seems
    * like a decent compromise for now.
    *
    * TODO: Consider optimizing this case.
    */
   if (state.varying) {
      uint8_t flow = 0;

      u_foreach_bit(slot, state.varying)
         flow |= bi_pop_slot(&state, slot);

      bi_flow(ctx, bi_after_block(blk), flow);
   }

   /* To figure out progress, diff scoreboard_out */
   progress = !!memcmp(&state, &blk->scoreboard_out, sizeof(state));

   blk->scoreboard_out = state;

   return progress;
}

static void
va_assign_scoreboard(bi_context *ctx)
{
   u_worklist worklist;
   bi_worklist_init(ctx, &worklist);

   bi_foreach_block(ctx, block) {
      bi_worklist_push_tail(&worklist, block);
   }

   /* Perform forward data flow analysis to calculate dependencies */
   while (!u_worklist_is_empty(&worklist)) {
      /* Pop from the front for forward analysis */
      bi_block *blk = bi_worklist_pop_head(&worklist);

      if (scoreboard_block_update(ctx, blk)) {
         bi_foreach_successor(blk, succ)
            bi_worklist_push_tail(&worklist, succ);
      }
   }

   u_worklist_fini(&worklist);
}

/*
 * Determine if execution should terminate after a given block. Execution cannot
 * terminate within a basic block.
 */
static bool
va_should_end(bi_block *block)
{
   /* Don't return if we're succeeded by instructions */
   for (unsigned i = 0; i < ARRAY_SIZE(block->successors); ++i) {
      bi_block *succ = block->successors[i];

      if (succ)
         return false;
   }

   return true;
}

/*
 * We should discard helper invocations as soon as helper invocations die after
 * their last use. Either they die after an instruction using helper
 * invocations, or they die along a control flow edge. The former is handled by
 * discarding appropriately after instructions. The latter is handled by
 * inserting a discard at the _start_ of some blocks:
 *
 * Lemma: If a non-critical edge discards helpers, it is the only edge that
 * enters its destination.
 *
 * Proof: An edge discards helpers if helpers are live at the end of the source
 * block and dead at the start of the destination block. By definition, helpers
 * are live at the end of a block iff they are live at the start of some
 * successor of a block. The source block therefore has a successor with helpers
 * live at the start and a successor with helpers dead at the start. As the
 * source block has at least two successors, the edge is NOT the only edge
 * exiting its source. Hence it is the only edge entering the destination,
 * otherwise the edge would be critical.
 *
 * By corrollary, we may handle discards on control flow edges by discarding at
 * the start of blocks with a single predecessor.
 *
 * This routine tests if a block should discard helper invocations at its start.
 */
static bool
va_discard_before_block(bi_block *block)
{
   /* Do not discard if the block requires helpers at the start */
   if (block->pass_flags)
      return false;

   /* By the lemma, if we need to discard, there is a unique predecessor */
   if (bi_num_predecessors(block) != 1)
      return false;

   bi_block *pred = *util_dynarray_element(&block->predecessors, bi_block *, 0);

   /* Discard if helpers are live at the end of the predecessor, due to helpers
    * live at the start of some (other) successor.
    */
   bi_foreach_successor(pred, succ) {
      if (succ->pass_flags)
         return true;
   }

   return false;
}

/*
 * Test if a program is empty, in the sense of having zero instructions. Empty
 * shaders get special handling.
 */
static bool
bi_is_empty(bi_context *ctx)
{
   bi_foreach_instr_global(ctx, _)
      return false;

   return true;
}

/*
 * Given a program with no flow control modifiers, insert NOPs signaling the
 * required flow control. Not much optimization happens here.
 */
void
va_insert_flow_control_nops(bi_context *ctx)
{
   /* Special case: if a program is empty, leave it empty. In particular, do not
    * insert NOP.end. There is special handling in the driver for skipping empty
    * shaders for shader stage. The .end is not necessary and disrupts
    * optimizations.
    */
   if (bi_is_empty(ctx))
      return;

   /* First do dataflow analysis for the scoreboard. This populates I->flow with
    * a bitmap of slots to wait on.
    *
    * Also do dataflow analysis for helper invocations in fragment shaders. This
    * populates block->pass_flags with helper invocation information.
    */
   va_assign_scoreboard(ctx);
   bi_analyze_helper_terminate(ctx);

   bi_foreach_block(ctx, block) {
      /* Handle discards along control flow edges */
      if (va_discard_before_block(block))
         bi_flow(ctx, bi_before_block(block), VA_FLOW_DISCARD);

      bi_foreach_instr_in_block_safe(block, I) {
         switch (I->op) {
         /* Signal barriers immediately */
         case BI_OPCODE_BARRIER:
            bi_flow(ctx, bi_after_instr(I), VA_FLOW_WAIT);
            break;

         /* Insert waits for tilebuffer and depth/stencil instructions. These
          * only happen in regular fragment shaders, as the required waits are
          * assumed to already have happened in blend shaders.
          *
          * For discarded thread handling, ATEST must be serialized against all
          * other asynchronous instructions and should be serialized against all
          * instructions. Wait for slot 0 immediately after the ATEST.
          */
         case BI_OPCODE_BLEND:
         case BI_OPCODE_LD_TILE:
         case BI_OPCODE_ST_TILE:
            if (!ctx->inputs->is_blend)
               bi_flow(ctx, bi_before_instr(I), VA_FLOW_WAIT);
            break;
         case BI_OPCODE_ATEST:
            bi_flow(ctx, bi_before_instr(I), VA_FLOW_WAIT0126);
            bi_flow(ctx, bi_after_instr(I), VA_FLOW_WAIT0);
            break;
         case BI_OPCODE_ZS_EMIT:
            if (!ctx->inputs->is_blend)
               bi_flow(ctx, bi_before_instr(I), VA_FLOW_WAIT0126);
            break;

         default:
            break;
         }

         if (I->flow && I->op != BI_OPCODE_NOP) {
            /* Wait on the results of asynchronous instructions
             *
             * Bitmap of general slots lines up with the encoding of va_flow for
             * waits on general slots. The dataflow analysis should be ignoring
             * the special slots #6 and #7, which are handled separately.
             */
            assert((I->flow & ~BITFIELD_MASK(VA_NUM_GENERAL_SLOTS)) == 0);

            bi_flow(ctx, bi_before_instr(I), I->flow);
            I->flow = 0;
         }
      }

      /* Terminate helpers after the last use */
      if (ctx->stage == MESA_SHADER_FRAGMENT && !ctx->inputs->is_blend &&
          block->pass_flags && bi_block_terminates_helpers(block)) {

         bi_foreach_instr_in_block_safe_rev(block, I) {
            if (bi_instr_uses_helpers(I)) {
               bi_flow(ctx, bi_after_instr(I), VA_FLOW_DISCARD);
               break;
            }
         }
      }

      /* End exeuction at the end of the block if needed, or reconverge if we
       * continue but we don't need to end execution.
       */
      if (va_should_end(block) || block->needs_nop) {
         /* Don't bother adding a NOP into an unreachable block */
         if (block == bi_start_block(&ctx->blocks) ||
             bi_num_predecessors(block))
            bi_flow(ctx, bi_after_block(block), VA_FLOW_END);
      } else if (bi_reconverge_branches(block)) {
         /* TODO: Do we have ever need to reconverge from an empty block? */
         if (!list_is_empty(&block->instructions))
            bi_flow(ctx, bi_after_block(block), VA_FLOW_RECONVERGE);
      }
   }

   /* If helpers are not used anywhere, they are not used at the start, so we
    * terminate at the start. Else, helpers are used somewhere in the shader and
    * are terminated after last use.
    */
   bi_block *start = bi_start_block(&ctx->blocks);
   bool frag = (ctx->stage == MESA_SHADER_FRAGMENT && !ctx->inputs->is_blend);

   if (frag && !start->pass_flags)
      bi_flow(ctx, bi_before_block(start), VA_FLOW_DISCARD);
}

/*
 * Assign slots to all asynchronous instructions. A few special instructions
 * require specific slots. For the rest, we assign slots in a round-robin
 * fashion to reduce false dependencies when encoding waits.
 *
 * This should be called before va_insert_flow_control_nops.
 */
void
va_assign_slots(bi_context *ctx)
{
   unsigned counter = 0;

   bi_foreach_instr_global(ctx, I) {
      if (I->op == BI_OPCODE_BARRIER) {
         I->slot = 7;
      } else if (I->op == BI_OPCODE_ZS_EMIT || I->op == BI_OPCODE_ATEST) {
         I->slot = 0;
      } else if (bi_opcode_props[I->op].message) {
         I->slot = counter++;

         if (counter == 3)
            counter = 0;
      }
   }
}
