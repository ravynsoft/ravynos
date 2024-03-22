/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "rogue.h"
#include "util/macros.h"

#include <stdbool.h>

/**
 * \file rogue_trim.c
 *
 * \brief Contains the rogue_trim pass.
 */

static bool rogue_trim_instrs(rogue_shader *shader)
{
   bool progress = false;

   shader->next_block = 0;
   shader->next_instr = 0;

   rogue_foreach_block (block, shader) {
      progress |= (block->index != shader->next_block);
      block->index = shader->next_block++;
      rogue_foreach_instr_in_block (instr, block) {
         progress |= (instr->index != shader->next_instr);
         instr->index = shader->next_instr++;
      }
   }

   return progress;
}

static bool rogue_trim_regs(rogue_shader *shader)
{
   bool progress = false;

   rogue_reset_reg_usage(shader, ROGUE_REG_CLASS_SSA);
   rogue_reset_reg_usage(shader, ROGUE_REG_CLASS_TEMP);

   unsigned index[ROGUE_REG_CLASS_COUNT] = { 0 };

   rogue_foreach_regarray (regarray, shader) {
      enum rogue_reg_class class = regarray->regs[0]->class;
      if (class != ROGUE_REG_CLASS_SSA && class != ROGUE_REG_CLASS_TEMP)
         continue;

      if (regarray->parent)
         continue;

      rogue_regarray_set(shader, regarray, class, index[class], true);

      rogue_foreach_subarray (subarray, regarray) {
         unsigned idx_offset =
            subarray->regs[0]->index - regarray->regs[0]->index;
         progress &= rogue_regarray_set(shader,
                                        subarray,
                                        class,
                                        index[class] + idx_offset,
                                        false);
      }

      index[class] += regarray->size;
   }

   rogue_foreach_reg (reg, shader, ROGUE_REG_CLASS_SSA) {
      if (reg->dirty)
         continue;

      progress |= rogue_reg_set(shader, reg, reg->class, index[reg->class]++);
   }

   rogue_foreach_reg (reg, shader, ROGUE_REG_CLASS_TEMP) {
      if (reg->dirty)
         continue;

      progress |= rogue_reg_set(shader, reg, reg->class, index[reg->class]++);
   }

   return progress;
}

/* Renumbers instructions, blocks, and temp/ssa registers. */
PUBLIC
bool rogue_trim(rogue_shader *shader)
{
   if (shader->is_grouped)
      return false;

   bool progress = false;

   progress |= rogue_trim_instrs(shader);
   progress |= rogue_trim_regs(shader);

   return progress;
}
