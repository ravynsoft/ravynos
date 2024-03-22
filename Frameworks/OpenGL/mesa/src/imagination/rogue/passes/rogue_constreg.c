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
 * \file rogue_constreg.c
 *
 * \brief Contains the rogue_constreg pass.
 */

/* Converts immediate values to constant register values. */
PUBLIC
bool rogue_constreg(rogue_shader *shader)
{
   if (shader->is_grouped)
      return false;

   bool progress = false;

   rogue_foreach_imm_use_safe (imm_use, shader) {
      unsigned index = rogue_constreg_lookup(*imm_use->imm);
      /* Skip values that aren't in the special constant registers; they'll be
       * replaced with immediate movs. */
      if (index == ROGUE_NO_CONST_REG)
         continue;

      rogue_reg *reg = rogue_const_reg(shader, index);

      struct list_head imm_use_link = imm_use->link;
      if (rogue_src_imm_replace(imm_use, reg)) {
         list_del(&imm_use_link);
         progress |= true;
      }
   }

   return progress;
}
