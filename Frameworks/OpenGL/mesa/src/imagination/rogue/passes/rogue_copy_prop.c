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
#include "rogue_builder.h"
#include "util/macros.h"

#include <stdbool.h>

/**
 * \file rogue_copy_prop.c
 *
 * \brief Contains the rogue_copy_prop pass.
 */

static bool can_back_prop(rogue_alu_instr *mov)
{
   /* TODO: Check for src/dst modifiers when support is added for them. */

   if (!rogue_ref_is_reg(&mov->src[0].ref) ||
       !rogue_ref_is_reg(&mov->dst[0].ref))
      return false;

   if (mov->src[0].ref.reg->regarray)
      return false;

   /* Vertex outputs require uvsw.write; only back-propagate if the parent
    * instruction is also a mov. */
   if (mov->dst[0].ref.reg->class == ROGUE_REG_CLASS_VTXOUT) {
      rogue_reg_write *write =
         list_first_entry(&mov->src[0].ref.reg->writes, rogue_reg_write, link);

      if (write->instr->type != ROGUE_INSTR_TYPE_ALU)
         return false;

      rogue_alu_instr *alu = rogue_instr_as_alu(write->instr);
      if (alu->op != ROGUE_ALU_OP_MOV)
         return false;
   }

   /* Is the source register only written to once? */
   if (!list_is_singular(&mov->src[0].ref.reg->writes))
      return false;

   /* Is this the only instruction that writes to this register? */
   if (!list_is_singular(&mov->dst[0].ref.reg->writes))
      return false;

   return true;
}

static bool can_forward_prop(rogue_alu_instr *mov)
{
   /* TODO: Check for src/dst modifiers when support is added for them. */

   if (!rogue_ref_is_reg(&mov->src[0].ref) ||
       !rogue_ref_is_reg(&mov->dst[0].ref))
      return false;

   if (mov->dst[0].ref.reg->regarray)
      return false;

   if (mov->dst[0].ref.reg->class != ROGUE_REG_CLASS_SSA)
      return false;

   /* Is the source register written to more than once (driver-supplied regs can
    * have zero writes)? */
   if (list_length(&mov->src[0].ref.reg->writes) > 1)
      return false;

   /* Is this the only instruction that writes to this register? */
   if (!list_is_singular(&mov->dst[0].ref.reg->writes))
      return false;

   return true;
}

static bool rogue_back_prop(rogue_alu_instr *mov)
{
   rogue_reg *mov_src = mov->src[0].ref.reg;
   rogue_reg *mov_dst = mov->dst[0].ref.reg;

   rogue_reg_write *write =
      list_first_entry(&mov_src->writes, rogue_reg_write, link);

   if (!rogue_dst_reg_replace(write, mov_dst))
      return false;

   rogue_instr_delete(&mov->instr);

   return true;
}

static bool rogue_forward_prop(rogue_alu_instr *mov)
{
   bool success = true;

   rogue_reg *mov_src = mov->src[0].ref.reg;
   rogue_reg *mov_dst = mov->dst[0].ref.reg;

   rogue_foreach_reg_use_safe (use, mov_dst)
      if (rogue_can_replace_reg_use(use, mov_src))
         success &= rogue_src_reg_replace(use, mov_src);
      else
         success = false;

   if (!success)
      return false;

   rogue_instr_delete(&mov->instr);

   return true;
}

/* Copy propagation pass. */
/* Forward and back, so that regarrays can be handled to a degree (making sure
 * to propagate the regarray register rather than replace it and mess up the
 * contiguous numbering). */
PUBLIC
bool rogue_copy_prop(rogue_shader *shader)
{
   if (shader->is_grouped)
      return false;

   bool progress = false;

   rogue_foreach_instr_in_shader_safe (instr, shader) {
      if (instr->type != ROGUE_INSTR_TYPE_ALU)
         continue;

      rogue_alu_instr *mov = rogue_instr_as_alu(instr);
      if (mov->op != ROGUE_ALU_OP_MOV)
         continue;

      if (can_forward_prop(mov))
         progress |= rogue_forward_prop(mov);
      else if (can_back_prop(mov))
         progress |= rogue_back_prop(mov);
   }

   return progress;
}
