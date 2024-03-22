/*
 * Copyright (c) 2019 Zodiac Inflight Innovations
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Jonathan Marek <jonathan@marek.ca>
 */

#include "etnaviv_compiler_nir.h"
#include "compiler/nir/nir_worklist.h"

static void
range_include(struct live_def *def, unsigned index)
{
   if (def->live_start > index)
      def->live_start = index;
   if (def->live_end < index)
      def->live_end = index;
}

struct live_defs_state {
   unsigned num_defs;
   unsigned bitset_words;

   nir_function_impl *impl;
   nir_block *block; /* current block pointer */
   unsigned index; /* current live index */

   struct live_def *defs;
   unsigned *live_map; /* to map ssa/reg index into defs array */

   nir_block_worklist worklist;
};

static bool
init_liveness_block(nir_block *block,
                    struct live_defs_state *state)
{
   block->live_in = reralloc(block, block->live_in, BITSET_WORD,
                             state->bitset_words);
   memset(block->live_in, 0, state->bitset_words * sizeof(BITSET_WORD));

   block->live_out = reralloc(block, block->live_out, BITSET_WORD,
                              state->bitset_words);
   memset(block->live_out, 0, state->bitset_words * sizeof(BITSET_WORD));

   nir_block_worklist_push_head(&state->worklist, block);

   return true;
}

static bool
set_src_live(nir_src *src, void *void_state)
{
   struct live_defs_state *state = void_state;

   nir_instr *instr = src->ssa->parent_instr;

   if (is_sysval(instr) || instr->type == nir_instr_type_deref)
      return true;

   switch (instr->type) {
   case nir_instr_type_load_const:
   case nir_instr_type_undef:
      return true;
   case nir_instr_type_alu: {
      /* alu op bypass */
      nir_alu_instr *alu = nir_instr_as_alu(instr);
      if (instr->pass_flags & BYPASS_SRC) {
         for (unsigned i = 0; i < nir_op_infos[alu->op].num_inputs; i++)
            set_src_live(&alu->src[i].src, state);
         return true;
      }
      break;
   }
   default:
      break;
   }

   unsigned i = state->live_map[src_index(state->impl, src)];
   assert(i != ~0u);

   BITSET_SET(state->block->live_in, i);
   range_include(&state->defs[i], state->index);

   return true;
}

static bool
propagate_across_edge(nir_block *pred, nir_block *succ,
                      struct live_defs_state *state)
{
   BITSET_WORD progress = 0;
   for (unsigned i = 0; i < state->bitset_words; ++i) {
      progress |= succ->live_in[i] & ~pred->live_out[i];
      pred->live_out[i] |= succ->live_in[i];
   }
   return progress != 0;
}

unsigned
etna_live_defs(nir_function_impl *impl, struct live_def *defs, unsigned *live_map)
{
   struct live_defs_state state;
   unsigned block_live_index[impl->num_blocks + 1];

   state.impl = impl;
   state.defs = defs;
   state.live_map = live_map;

   state.num_defs = 0;
   nir_foreach_block(block, impl) {
      block_live_index[block->index] = state.num_defs;
      nir_foreach_instr(instr, block) {
         nir_def *def = def_for_instr(instr);
         if (!def)
            continue;

         unsigned idx = def_index(impl, def);
         /* register is already in defs */
         if (live_map[idx] != ~0u)
            continue;

         defs[state.num_defs] = (struct live_def) {instr, def, state.num_defs, 0};

         /* input live from the start */
         if (instr->type == nir_instr_type_intrinsic) {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic == nir_intrinsic_load_input ||
                intr->intrinsic == nir_intrinsic_load_instance_id)
               defs[state.num_defs].live_start = 0;
         }

         live_map[idx] = state.num_defs;
         state.num_defs++;
      }
   }
   block_live_index[impl->num_blocks] = state.num_defs;

   nir_block_worklist_init(&state.worklist, impl->num_blocks, NULL);

   /* We now know how many unique ssa definitions we have and we can go
    * ahead and allocate live_in and live_out sets and add all of the
    * blocks to the worklist.
    */
   state.bitset_words = BITSET_WORDS(state.num_defs);
   nir_foreach_block(block, impl) {
      init_liveness_block(block, &state);
   }

   /* We're now ready to work through the worklist and update the liveness
    * sets of each of the blocks.  By the time we get to this point, every
    * block in the function implementation has been pushed onto the
    * worklist in reverse order.  As long as we keep the worklist
    * up-to-date as we go, everything will get covered.
    */
   while (!nir_block_worklist_is_empty(&state.worklist)) {
      /* We pop them off in the reverse order we pushed them on.  This way
       * the first walk of the instructions is backwards so we only walk
       * once in the case of no control flow.
       */
      nir_block *block = nir_block_worklist_pop_head(&state.worklist);
      state.block = block;

      memcpy(block->live_in, block->live_out,
             state.bitset_words * sizeof(BITSET_WORD));

      state.index = block_live_index[block->index + 1];

      nir_if *following_if = nir_block_get_following_if(block);
      if (following_if)
         set_src_live(&following_if->condition, &state);

      nir_foreach_instr_reverse(instr, block) {
         /* when we come across the next "live" instruction, decrement index */
         if (state.index && instr == defs[state.index - 1].instr) {
            state.index--;
            /* the only source of writes to registers is phis:
             * we don't expect any partial write_mask alus
             * so clearing live_in here is OK
             */
            BITSET_CLEAR(block->live_in, state.index);
         }

         /* don't set_src_live for not-emitted instructions */
         if (is_dead_instruction(instr))
            continue;

         unsigned index = state.index;

         /* output live till the end */
         if (instr->type == nir_instr_type_intrinsic) {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic == nir_intrinsic_store_deref)
               state.index = ~0u;
         }

         bool processed = false;

         if (instr->type == nir_instr_type_intrinsic) {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic == nir_intrinsic_decl_reg ||
               intr->intrinsic == nir_intrinsic_store_reg)
               processed = true;
         }

         if (!processed)
            nir_foreach_src(instr, set_src_live, &state);

         state.index = index;
      }
      assert(state.index == block_live_index[block->index]);

      /* Walk over all of the predecessors of the current block updating
       * their live in with the live out of this one.  If anything has
       * changed, add the predecessor to the work list so that we ensure
       * that the new information is used.
       */
      set_foreach(block->predecessors, entry) {
         nir_block *pred = (nir_block *)entry->key;
         if (propagate_across_edge(pred, block, &state))
            nir_block_worklist_push_tail(&state.worklist, pred);
      }
   }

   nir_block_worklist_fini(&state.worklist);

   /* apply live_in/live_out to ranges */

   nir_foreach_block(block, impl) {
      int i;

      BITSET_FOREACH_SET(i, block->live_in, state.num_defs)
         range_include(&state.defs[i], block_live_index[block->index]);

      BITSET_FOREACH_SET(i, block->live_out, state.num_defs)
         range_include(&state.defs[i], block_live_index[block->index + 1]);
   }

   return state.num_defs;
}
