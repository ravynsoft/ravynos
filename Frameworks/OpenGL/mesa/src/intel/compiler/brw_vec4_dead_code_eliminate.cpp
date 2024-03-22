/*
 * Copyright © 2014 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "brw_vec4.h"
#include "brw_vec4_live_variables.h"
#include "brw_cfg.h"

/** @file brw_vec4_dead_code_eliminate.cpp
 *
 * Dataflow-aware dead code elimination.
 *
 * Walks the instruction list from the bottom, removing instructions that
 * have results that both aren't used in later blocks and haven't been read
 * yet in the tail end of this block.
 */

using namespace brw;

bool
vec4_visitor::dead_code_eliminate()
{
   bool progress = false;

   const vec4_live_variables &live_vars = live_analysis.require();
   int num_vars = live_vars.num_vars;
   BITSET_WORD *live = rzalloc_array(NULL, BITSET_WORD, BITSET_WORDS(num_vars));
   BITSET_WORD *flag_live = rzalloc_array(NULL, BITSET_WORD, 1);

   foreach_block_reverse_safe(block, cfg) {
      memcpy(live, live_vars.block_data[block->num].liveout,
             sizeof(BITSET_WORD) * BITSET_WORDS(num_vars));
      memcpy(flag_live, live_vars.block_data[block->num].flag_liveout,
             sizeof(BITSET_WORD));

      foreach_inst_in_block_reverse_safe(vec4_instruction, inst, block) {
         if ((inst->dst.file == VGRF && !inst->has_side_effects()) ||
             (inst->dst.is_null() && inst->writes_flag(devinfo))){
            bool result_live[4] = { false };
            if (inst->dst.file == VGRF) {
               for (unsigned i = 0; i < DIV_ROUND_UP(inst->size_written, 16); i++) {
                  for (int c = 0; c < 4; c++) {
                     const unsigned v = var_from_reg(alloc, inst->dst, c, i);
                     result_live[c] |= BITSET_TEST(live, v);
                  }
               }
            } else {
               for (unsigned c = 0; c < 4; c++)
                  result_live[c] = BITSET_TEST(flag_live, c);
            }

            /* If the instruction can't do writemasking, then it's all or
             * nothing.
             */
            if (!inst->can_do_writemask(devinfo)) {
               bool result = result_live[0] | result_live[1] |
                             result_live[2] | result_live[3];
               result_live[0] = result;
               result_live[1] = result;
               result_live[2] = result;
               result_live[3] = result;
            }

            if (inst->writes_flag(devinfo)) {
               /* Independently calculate the usage of the flag components and
                * the destination value components.
                */
               uint8_t flag_mask = inst->dst.writemask;
               uint8_t dest_mask = inst->dst.writemask;

               for (int c = 0; c < 4; c++) {
                  if (!result_live[c] && dest_mask & (1 << c))
                     dest_mask &= ~(1 << c);

                  if (!BITSET_TEST(flag_live, c))
                     flag_mask &= ~(1 << c);
               }

               if (inst->dst.writemask != (flag_mask | dest_mask)) {
                  progress = true;
                  inst->dst.writemask = flag_mask | dest_mask;
               }

               /* If none of the destination components are read, replace the
                * destination register with the NULL register.
                */
               if (dest_mask == 0) {
                  progress = true;
                  inst->dst = dst_reg(retype(brw_null_reg(), inst->dst.type));
               }
            } else {
               for (int c = 0; c < 4; c++) {
                  if (!result_live[c] && inst->dst.writemask & (1 << c)) {
                     inst->dst.writemask &= ~(1 << c);
                     progress = true;

                     if (inst->dst.writemask == 0) {
                        if (inst->writes_accumulator) {
                           inst->dst = dst_reg(retype(brw_null_reg(), inst->dst.type));
                        } else {
                           inst->opcode = BRW_OPCODE_NOP;
                           break;
                        }
                     }
                  }
               }
            }
         }

         if (inst->dst.is_null() && inst->writes_flag(devinfo)) {
            bool combined_live = false;
            for (unsigned c = 0; c < 4; c++)
               combined_live |= BITSET_TEST(flag_live, c);

            if (!combined_live) {
               inst->opcode = BRW_OPCODE_NOP;
               progress = true;
            }
         }

         if (inst->dst.file == VGRF && !inst->predicate &&
             !inst->is_align1_partial_write()) {
            for (unsigned i = 0; i < DIV_ROUND_UP(inst->size_written, 16); i++) {
               for (int c = 0; c < 4; c++) {
                  if (inst->dst.writemask & (1 << c)) {
                     const unsigned v = var_from_reg(alloc, inst->dst, c, i);
                     BITSET_CLEAR(live, v);
                  }
               }
            }
         }

         if (inst->writes_flag(devinfo) && !inst->predicate && inst->exec_size == 8) {
            for (unsigned c = 0; c < 4; c++)
               BITSET_CLEAR(flag_live, c);
         }

         if (inst->opcode == BRW_OPCODE_NOP) {
            inst->remove(block);
            continue;
         }

         for (int i = 0; i < 3; i++) {
            if (inst->src[i].file == VGRF) {
               for (unsigned j = 0; j < DIV_ROUND_UP(inst->size_read(i), 16); j++) {
                  for (int c = 0; c < 4; c++) {
                     const unsigned v = var_from_reg(alloc, inst->src[i], c, j);
                     BITSET_SET(live, v);
                  }
               }
            }
         }

         for (unsigned c = 0; c < 4; c++) {
            if (inst->reads_flag(c)) {
               BITSET_SET(flag_live, c);
            }
         }
      }
   }

   ralloc_free(live);
   ralloc_free(flag_live);

   if (progress)
      invalidate_analysis(DEPENDENCY_INSTRUCTIONS);

   return progress;
}
