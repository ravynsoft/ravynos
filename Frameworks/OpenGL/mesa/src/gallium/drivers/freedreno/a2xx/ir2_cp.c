/*
 * Copyright (C) 2018 Jonathan Marek <jonathan@marek.ca>
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
 * Authors:
 *    Jonathan Marek <jonathan@marek.ca>
 */

#include "ir2_private.h"

static bool
is_mov(struct ir2_instr *instr)
{
   return instr->type == IR2_ALU && instr->alu.vector_opc == MAXv &&
          instr->src_count == 1;
}

static void
src_combine(struct ir2_src *src, struct ir2_src b)
{
   src->num = b.num;
   src->type = b.type;
   src->swizzle = swiz_merge(b.swizzle, src->swizzle);
   if (!src->abs) /* if we have abs we don't care about previous negate */
      src->negate ^= b.negate;
   src->abs |= b.abs;
}

/* cp_src: replace src regs when they refer to a mov instruction
 * example:
 *	ALU:      MAXv    R7 = C7, C7
 *	ALU:      MULADDv R7 = R7, R10, R0.xxxx
 * becomes:
 *	ALU:      MULADDv R7 = C7, R10, R0.xxxx
 */
void
cp_src(struct ir2_context *ctx)
{
   struct ir2_instr *p;

   ir2_foreach_instr (instr, ctx) {
      ir2_foreach_src (src, instr) {
         /* loop to replace recursively */
         do {
            if (src->type != IR2_SRC_SSA)
               break;

            p = &ctx->instr[src->num];
            /* don't work across blocks to avoid possible issues */
            if (p->block_idx != instr->block_idx)
               break;

            if (!is_mov(p))
               break;

            if (p->alu.saturate)
               break;

            /* cant apply abs to const src, const src only for alu */
            if (p->src[0].type == IR2_SRC_CONST &&
                (src->abs || instr->type != IR2_ALU))
               break;

            src_combine(src, p->src[0]);
         } while (1);
      }
   }
}

/* cp_export: replace mov to export when possible
 * in the cp_src pass we bypass any mov instructions related
 * to the src registers, but for exports for need something different
 * example:
 *	ALU:      MAXv    R3.x___ = C9.x???, C9.x???
 *	ALU:      MAXv    R3._y__ = R0.?x??, C8.?x??
 *	ALU:      MAXv    export0 = R3.yyyx, R3.yyyx
 * becomes:
 *	ALU:      MAXv    export0.___w = C9.???x, C9.???x
 *	ALU:      MAXv    export0.xyz_ = R0.xxx?, C8.xxx?
 *
 */
void
cp_export(struct ir2_context *ctx)
{
   struct ir2_instr *c[4], *ins[4];
   struct ir2_src *src;
   struct ir2_reg *reg;
   unsigned ncomp;

   ir2_foreach_instr (instr, ctx) {
      if (!is_export(instr)) /* TODO */
         continue;

      if (!is_mov(instr))
         continue;

      src = &instr->src[0];

      if (src->negate || src->abs) /* TODO handle these cases */
         continue;

      if (src->type == IR2_SRC_INPUT || src->type == IR2_SRC_CONST)
         continue;

      reg = get_reg_src(ctx, src);
      ncomp = dst_ncomp(instr);

      unsigned reswiz[4] = {};
      unsigned num_instr = 0;

      /* fill array c with pointers to instrs that write each component */
      if (src->type == IR2_SRC_SSA) {
         struct ir2_instr *instr = &ctx->instr[src->num];

         if (instr->type != IR2_ALU)
            continue;

         for (int i = 0; i < ncomp; i++)
            c[i] = instr;

         ins[num_instr++] = instr;
         reswiz[0] = src->swizzle;
      } else {
         bool ok = true;
         unsigned write_mask = 0;

         ir2_foreach_instr (instr, ctx) {
            if (instr->is_ssa || instr->reg != reg)
               continue;

            /* set by non-ALU */
            if (instr->type != IR2_ALU) {
               ok = false;
               break;
            }

            /* component written more than once */
            if (write_mask & instr->alu.write_mask) {
               ok = false;
               break;
            }

            write_mask |= instr->alu.write_mask;

            /* src pointers for components */
            for (int i = 0, j = 0; i < 4; i++) {
               unsigned k = swiz_get(src->swizzle, i);
               if (instr->alu.write_mask & 1 << k) {
                  c[i] = instr;

                  /* reswiz = compressed src->swizzle */
                  unsigned x = 0;
                  for (int i = 0; i < k; i++)
                     x += !!(instr->alu.write_mask & 1 << i);

                  assert(src->swizzle || x == j);
                  reswiz[num_instr] |= swiz_set(x, j++);
               }
            }
            ins[num_instr++] = instr;
         }
         if (!ok)
            continue;
      }

      bool redirect = true;

      /* must all be in same block */
      for (int i = 0; i < ncomp; i++)
         redirect &= (c[i]->block_idx == instr->block_idx);

      /* no other instr using the value */
      ir2_foreach_instr (p, ctx) {
         if (p == instr)
            continue;
         ir2_foreach_src (src, p)
            redirect &= reg != get_reg_src(ctx, src);
      }

      if (!redirect)
         continue;

      /* redirect the instructions writing to the register */
      for (int i = 0; i < num_instr; i++) {
         struct ir2_instr *p = ins[i];

         p->alu.export = instr->alu.export;
         p->alu.write_mask = 0;
         p->is_ssa = true;
         p->ssa.ncomp = 0;
         memset(p->ssa.comp, 0, sizeof(p->ssa.comp));
         p->alu.saturate |= instr->alu.saturate;

         switch (p->alu.vector_opc) {
         case PRED_SETE_PUSHv ... PRED_SETGTE_PUSHv:
         case DOT2ADDv:
         case DOT3v:
         case DOT4v:
         case CUBEv:
            continue;
         default:
            break;
         }
         ir2_foreach_src (s, p)
            swiz_merge_p(&s->swizzle, reswiz[i]);
      }

      for (int i = 0; i < ncomp; i++) {
         c[i]->alu.write_mask |= (1 << i);
         c[i]->ssa.ncomp++;
      }
      instr->type = IR2_NONE;
      instr->need_emit = false;
   }
}
