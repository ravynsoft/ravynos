/*
 * Copyright (C) 2021 Valve Corporation
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

#include "ir3_ra.h"
#include "ir3_shader.h"
#include "util/ralloc.h"

/* A note on how phi node uses are handled:
 *
 * - Phi node sources are considered to happen after the end of the
 *   predecessor block, so the live_out for that block contains phi sources.
 * - On the other hand, phi destinations are considered to happen at the start
 *   of the block, so that live_in does *not* contain phi destinations. This
 *   is mainly because phi destinations and live-through values have to be
 *   treated very differently by RA at the beginning of a block.
 */

static bool
compute_block_liveness(struct ir3_liveness *live, struct ir3_block *block,
                       BITSET_WORD *tmp_live, unsigned bitset_words)
{
   memcpy(tmp_live, live->live_out[block->index],
          bitset_words * sizeof(BITSET_WORD));

   /* Process instructions */
   foreach_instr_rev (instr, &block->instr_list) {
      ra_foreach_dst (dst, instr) {
         if (BITSET_TEST(tmp_live, dst->name))
            dst->flags &= ~IR3_REG_UNUSED;
         else
            dst->flags |= IR3_REG_UNUSED;
         BITSET_CLEAR(tmp_live, dst->name);
      }

      /* Phi node uses occur after the predecessor block */
      if (instr->opc != OPC_META_PHI) {
         ra_foreach_src (src, instr) {
            if (BITSET_TEST(tmp_live, src->def->name))
               src->flags &= ~IR3_REG_KILL;
            else
               src->flags |= IR3_REG_KILL;
         }

         ra_foreach_src (src, instr) {
            if (BITSET_TEST(tmp_live, src->def->name))
               src->flags &= ~IR3_REG_FIRST_KILL;
            else
               src->flags |= IR3_REG_FIRST_KILL;
            BITSET_SET(tmp_live, src->def->name);
         }
      }
   }

   memcpy(live->live_in[block->index], tmp_live,
          bitset_words * sizeof(BITSET_WORD));

   bool progress = false;
   for (unsigned i = 0; i < block->predecessors_count; i++) {
      const struct ir3_block *pred = block->predecessors[i];
      for (unsigned j = 0; j < bitset_words; j++) {
         if (tmp_live[j] & ~live->live_out[pred->index][j])
            progress = true;
         live->live_out[pred->index][j] |= tmp_live[j];
      }

      /* Process phi sources. */
      foreach_instr (phi, &block->instr_list) {
         if (phi->opc != OPC_META_PHI)
            break;
         if (!phi->srcs[i]->def)
            continue;
         unsigned name = phi->srcs[i]->def->name;
         if (!BITSET_TEST(live->live_out[pred->index], name)) {
            progress = true;
            BITSET_SET(live->live_out[pred->index], name);
         }
      }
   }

   for (unsigned i = 0; i < block->physical_predecessors_count; i++) {
      const struct ir3_block *pred = block->physical_predecessors[i];
      unsigned name;
      BITSET_FOREACH_SET (name, tmp_live, live->definitions_count) {
         struct ir3_register *reg = live->definitions[name];
         if (!(reg->flags & IR3_REG_SHARED))
            continue;
         if (!BITSET_TEST(live->live_out[pred->index], name)) {
            progress = true;
            BITSET_SET(live->live_out[pred->index], name);
         }
      }
   }

   return progress;
}

struct ir3_liveness *
ir3_calc_liveness(void *mem_ctx, struct ir3 *ir)
{
   struct ir3_liveness *live = rzalloc(mem_ctx, struct ir3_liveness);

   /* Reserve name 0 to mean "doesn't have a name yet" to make the debug
    * output nicer.
    */
   array_insert(live, live->definitions, NULL);

   /* Build definition <-> name mapping */
   unsigned block_count = 0;
   foreach_block (block, &ir->block_list) {
      block->index = block_count++;
      foreach_instr (instr, &block->instr_list) {
         ra_foreach_dst (dst, instr) {
            dst->name = live->definitions_count;
            array_insert(live, live->definitions, dst);
         }
      }
   }

   live->block_count = block_count;

   unsigned bitset_words = BITSET_WORDS(live->definitions_count);
   BITSET_WORD *tmp_live = ralloc_array(live, BITSET_WORD, bitset_words);
   live->live_in = ralloc_array(live, BITSET_WORD *, block_count);
   live->live_out = ralloc_array(live, BITSET_WORD *, block_count);
   unsigned i = 0;
   foreach_block (block, &ir->block_list) {
      block->index = i++;
      live->live_in[block->index] =
         rzalloc_array(live, BITSET_WORD, bitset_words);
      live->live_out[block->index] =
         rzalloc_array(live, BITSET_WORD, bitset_words);
   }

   bool progress = true;
   while (progress) {
      progress = false;
      foreach_block_rev (block, &ir->block_list) {
         progress |=
            compute_block_liveness(live, block, tmp_live, bitset_words);
      }
   }

   return live;
}

/* Return true if "def" is live after "instr". It's assumed that "def"
 * dominates "instr".
 */
bool
ir3_def_live_after(struct ir3_liveness *live, struct ir3_register *def,
                   struct ir3_instruction *instr)
{
   /* If it's live out then it's definitely live at the instruction. */
   if (BITSET_TEST(live->live_out[instr->block->index], def->name))
      return true;

   /* If it's not live in and not defined in the same block then the live
    * range can't extend to the instruction.
    */
   if (def->instr->block != instr->block &&
       !BITSET_TEST(live->live_in[instr->block->index], def->name))
      return false;

   /* Ok, now comes the tricky case, where "def" is killed somewhere in
    * "instr"'s block and we have to check if it's before or after.
    */
   foreach_instr_rev (test_instr, &instr->block->instr_list) {
      if (test_instr == instr)
         break;

      for (unsigned i = 0; i < test_instr->srcs_count; i++) {
         if (test_instr->srcs[i]->def == def)
            return true;
      }
   }

   return false;
}
