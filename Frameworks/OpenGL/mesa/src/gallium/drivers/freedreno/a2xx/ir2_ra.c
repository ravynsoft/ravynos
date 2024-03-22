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

/* if an instruction has side effects, we should never kill it */
static bool
has_side_effects(struct ir2_instr *instr)
{
   if (instr->type == IR2_CF)
      return true;
   else if (instr->type == IR2_FETCH)
      return false;

   switch (instr->alu.scalar_opc) {
   case PRED_SETEs ... KILLONEs:
      return true;
   default:
      break;
   }

   switch (instr->alu.vector_opc) {
   case PRED_SETE_PUSHv ... KILLNEv:
      return true;
   default:
      break;
   }

   return instr->alu.export >= 0;
}

/* mark an instruction as required, and all its sources recursively */
static void
set_need_emit(struct ir2_context *ctx, struct ir2_instr *instr)
{
   struct ir2_reg *reg;

   /* don't repeat work already done */
   if (instr->need_emit)
      return;

   instr->need_emit = true;

   ir2_foreach_src (src, instr) {
      switch (src->type) {
      case IR2_SRC_SSA:
         set_need_emit(ctx, &ctx->instr[src->num]);
         break;
      case IR2_SRC_REG:
         /* slow ..  */
         reg = get_reg_src(ctx, src);
         ir2_foreach_instr (instr, ctx) {
            if (!instr->is_ssa && instr->reg == reg)
               set_need_emit(ctx, instr);
         }
         break;
      default:
         break;
      }
   }
}

/* get current bit mask of allocated components for a register */
static unsigned
reg_mask(struct ir2_context *ctx, unsigned idx)
{
   return ctx->reg_state[idx / 8] >> idx % 8 * 4 & 0xf;
}

static void
reg_setmask(struct ir2_context *ctx, unsigned idx, unsigned c)
{
   idx = idx * 4 + c;
   ctx->reg_state[idx / 32] |= 1 << idx % 32;
}

static void
reg_freemask(struct ir2_context *ctx, unsigned idx, unsigned c)
{
   idx = idx * 4 + c;
   ctx->reg_state[idx / 32] &= ~(1 << idx % 32);
}

void
ra_count_refs(struct ir2_context *ctx)
{
   struct ir2_reg *reg;

   /* mark instructions as needed
    * need to do this because "substitutions" pass makes many movs not needed
    */
   ir2_foreach_instr (instr, ctx) {
      if (has_side_effects(instr))
         set_need_emit(ctx, instr);
   }

   /* compute ref_counts */
   ir2_foreach_instr (instr, ctx) {
      /* kill non-needed so they can be skipped */
      if (!instr->need_emit) {
         instr->type = IR2_NONE;
         continue;
      }

      ir2_foreach_src (src, instr) {
         if (src->type == IR2_SRC_CONST)
            continue;

         reg = get_reg_src(ctx, src);
         for (int i = 0; i < src_ncomp(instr); i++)
            reg->comp[swiz_get(src->swizzle, i)].ref_count++;
      }
   }
}

void
ra_reg(struct ir2_context *ctx, struct ir2_reg *reg, int force_idx, bool export,
       uint8_t export_writemask)
{
   /* for export, don't allocate anything but set component layout */
   if (export) {
      for (int i = 0; i < 4; i++)
         reg->comp[i].c = i;
      return;
   }

   unsigned idx = force_idx;

   /* TODO: allocate into the same register if theres room
    * note: the blob doesn't do it, so verify that it is indeed better
    * also, doing it would conflict with scalar mov insertion
    */

   /* check if already allocated */
   for (int i = 0; i < reg->ncomp; i++) {
      if (reg->comp[i].alloc)
         return;
   }

   if (force_idx < 0) {
      for (idx = 0; idx < 64; idx++) {
         if (reg_mask(ctx, idx) == 0)
            break;
      }
   }
   assert(idx != 64); /* TODO ran out of register space.. */

   /* update max_reg value */
   ctx->info->max_reg = MAX2(ctx->info->max_reg, (int)idx);

   unsigned mask = reg_mask(ctx, idx);

   for (int i = 0; i < reg->ncomp; i++) {
      /* don't allocate never used values */
      if (reg->comp[i].ref_count == 0) {
         reg->comp[i].c = 7;
         continue;
      }

      /* TODO */
      unsigned c = 1 ? i : (ffs(~mask) - 1);
      mask |= 1 << c;
      reg->comp[i].c = c;
      reg_setmask(ctx, idx, c);
      reg->comp[i].alloc = true;
   }

   reg->idx = idx;
   ctx->live_regs[reg->idx] = reg;
}

/* reduce srcs ref_count and free if needed */
void
ra_src_free(struct ir2_context *ctx, struct ir2_instr *instr)
{
   struct ir2_reg *reg;
   struct ir2_reg_component *comp;

   ir2_foreach_src (src, instr) {
      if (src->type == IR2_SRC_CONST)
         continue;

      reg = get_reg_src(ctx, src);
      /* XXX use before write case */

      for (int i = 0; i < src_ncomp(instr); i++) {
         comp = &reg->comp[swiz_get(src->swizzle, i)];
         if (!--comp->ref_count && reg->block_idx_free < 0) {
            reg_freemask(ctx, reg->idx, comp->c);
            comp->alloc = false;
         }
      }
   }
}

/* free any regs left for a block */
void
ra_block_free(struct ir2_context *ctx, unsigned block)
{
   ir2_foreach_live_reg (reg, ctx) {
      if (reg->block_idx_free != block)
         continue;

      for (int i = 0; i < reg->ncomp; i++) {
         if (!reg->comp[i].alloc) /* XXX should never be true? */
            continue;

         reg_freemask(ctx, reg->idx, reg->comp[i].c);
         reg->comp[i].alloc = false;
      }
      ctx->live_regs[reg->idx] = NULL;
   }
}
