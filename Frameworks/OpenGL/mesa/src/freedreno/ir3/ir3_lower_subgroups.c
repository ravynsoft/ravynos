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

#include "ir3.h"

/* Lower several macro-instructions needed for shader subgroup support that
 * must be turned into if statements. We do this after RA and post-RA
 * scheduling to give the scheduler a chance to rearrange them, because RA
 * may need to insert OPC_META_READ_FIRST to handle splitting live ranges, and
 * also because some (e.g. BALLOT and READ_FIRST) must produce a shared
 * register that cannot be spilled to a normal register until after the if,
 * which makes implementing spilling more complicated if they are already
 * lowered.
 */

static void
replace_pred(struct ir3_block *block, struct ir3_block *old_pred,
             struct ir3_block *new_pred)
{
   for (unsigned i = 0; i < block->predecessors_count; i++) {
      if (block->predecessors[i] == old_pred) {
         block->predecessors[i] = new_pred;
         return;
      }
   }
}

static void
replace_physical_pred(struct ir3_block *block, struct ir3_block *old_pred,
                      struct ir3_block *new_pred)
{
   for (unsigned i = 0; i < block->physical_predecessors_count; i++) {
      if (block->physical_predecessors[i] == old_pred) {
         block->physical_predecessors[i] = new_pred;
         return;
      }
   }
}

static void
mov_immed(struct ir3_register *dst, struct ir3_block *block, unsigned immed)
{
   struct ir3_instruction *mov = ir3_instr_create(block, OPC_MOV, 1, 1);
   struct ir3_register *mov_dst = ir3_dst_create(mov, dst->num, dst->flags);
   mov_dst->wrmask = dst->wrmask;
   struct ir3_register *src = ir3_src_create(
      mov, INVALID_REG, (dst->flags & IR3_REG_HALF) | IR3_REG_IMMED);
   src->uim_val = immed;
   mov->cat1.dst_type = (dst->flags & IR3_REG_HALF) ? TYPE_U16 : TYPE_U32;
   mov->cat1.src_type = mov->cat1.dst_type;
   mov->repeat = util_last_bit(mov_dst->wrmask) - 1;
}

static void
mov_reg(struct ir3_block *block, struct ir3_register *dst,
        struct ir3_register *src)
{
   struct ir3_instruction *mov = ir3_instr_create(block, OPC_MOV, 1, 1);

   struct ir3_register *mov_dst =
      ir3_dst_create(mov, dst->num, dst->flags & (IR3_REG_HALF | IR3_REG_SHARED));
   struct ir3_register *mov_src =
      ir3_src_create(mov, src->num, src->flags & (IR3_REG_HALF | IR3_REG_SHARED));
   mov_dst->wrmask = dst->wrmask;
   mov_src->wrmask = src->wrmask;
   mov->repeat = util_last_bit(mov_dst->wrmask) - 1;

   mov->cat1.dst_type = (dst->flags & IR3_REG_HALF) ? TYPE_U16 : TYPE_U32;
   mov->cat1.src_type = (src->flags & IR3_REG_HALF) ? TYPE_U16 : TYPE_U32;
}

static void
binop(struct ir3_block *block, opc_t opc, struct ir3_register *dst,
      struct ir3_register *src0, struct ir3_register *src1)
{
   struct ir3_instruction *instr = ir3_instr_create(block, opc, 1, 2);
   
   unsigned flags = dst->flags & IR3_REG_HALF;
   struct ir3_register *instr_dst = ir3_dst_create(instr, dst->num, flags);
   struct ir3_register *instr_src0 = ir3_src_create(instr, src0->num, flags);
   struct ir3_register *instr_src1 = ir3_src_create(instr, src1->num, flags);

   instr_dst->wrmask = dst->wrmask;
   instr_src0->wrmask = src0->wrmask;
   instr_src1->wrmask = src1->wrmask;
   instr->repeat = util_last_bit(instr_dst->wrmask) - 1;
}

static void
triop(struct ir3_block *block, opc_t opc, struct ir3_register *dst,
      struct ir3_register *src0, struct ir3_register *src1,
      struct ir3_register *src2)
{
   struct ir3_instruction *instr = ir3_instr_create(block, opc, 1, 3);
   
   unsigned flags = dst->flags & IR3_REG_HALF;
   struct ir3_register *instr_dst = ir3_dst_create(instr, dst->num, flags);
   struct ir3_register *instr_src0 = ir3_src_create(instr, src0->num, flags);
   struct ir3_register *instr_src1 = ir3_src_create(instr, src1->num, flags);
   struct ir3_register *instr_src2 = ir3_src_create(instr, src2->num, flags);

   instr_dst->wrmask = dst->wrmask;
   instr_src0->wrmask = src0->wrmask;
   instr_src1->wrmask = src1->wrmask;
   instr_src2->wrmask = src2->wrmask;
   instr->repeat = util_last_bit(instr_dst->wrmask) - 1;
}

static void
do_reduce(struct ir3_block *block, reduce_op_t opc,
          struct ir3_register *dst, struct ir3_register *src0,
          struct ir3_register *src1)
{
   switch (opc) {
#define CASE(name)                                                             \
   case REDUCE_OP_##name:                                                      \
      binop(block, OPC_##name, dst, src0, src1);                               \
      break;

   CASE(ADD_U)
   CASE(ADD_F)
   CASE(MUL_F)
   CASE(MIN_U)
   CASE(MIN_S)
   CASE(MIN_F)
   CASE(MAX_U)
   CASE(MAX_S)
   CASE(MAX_F)
   CASE(AND_B)
   CASE(OR_B)
   CASE(XOR_B)

#undef CASE

   case REDUCE_OP_MUL_U:
      if (dst->flags & IR3_REG_HALF) {
         binop(block, OPC_MUL_S24, dst, src0, src1);
      } else {
         /* 32-bit multiplication macro - see ir3_nir_imul */
         binop(block, OPC_MULL_U, dst, src0, src1);
         triop(block, OPC_MADSH_M16, dst, src0, src1, dst);
         triop(block, OPC_MADSH_M16, dst, src1, src0, dst);
      }
      break;
   }
}

static struct ir3_block *
split_block(struct ir3 *ir, struct ir3_block *before_block,
            struct ir3_instruction *instr)
{
   struct ir3_block *after_block = ir3_block_create(ir);
   list_add(&after_block->node, &before_block->node);

   for (unsigned i = 0; i < ARRAY_SIZE(before_block->successors); i++) {
      after_block->successors[i] = before_block->successors[i];
      if (after_block->successors[i])
         replace_pred(after_block->successors[i], before_block, after_block);
   }

   for (unsigned i = 0; i < ARRAY_SIZE(before_block->physical_successors);
        i++) {
      after_block->physical_successors[i] =
         before_block->physical_successors[i];
      if (after_block->physical_successors[i]) {
         replace_physical_pred(after_block->physical_successors[i],
                               before_block, after_block);
      }
   }

   before_block->successors[0] = before_block->successors[1] = NULL;
   before_block->physical_successors[0] = before_block->physical_successors[1] = NULL;

   foreach_instr_from_safe (rem_instr, &instr->node,
                            &before_block->instr_list) {
      list_del(&rem_instr->node);
      list_addtail(&rem_instr->node, &after_block->instr_list);
      rem_instr->block = after_block;
   }

   after_block->brtype = before_block->brtype;
   after_block->condition = before_block->condition;

   return after_block;
}

static void
link_blocks_physical(struct ir3_block *pred, struct ir3_block *succ,
                     unsigned index)
{
   pred->physical_successors[index] = succ;
   ir3_block_add_physical_predecessor(succ, pred);
}

static void
link_blocks(struct ir3_block *pred, struct ir3_block *succ, unsigned index)
{
   pred->successors[index] = succ;
   ir3_block_add_predecessor(succ, pred);
   link_blocks_physical(pred, succ, index);
}

static struct ir3_block *
create_if(struct ir3 *ir, struct ir3_block *before_block,
          struct ir3_block *after_block)
{
   struct ir3_block *then_block = ir3_block_create(ir);
   list_add(&then_block->node, &before_block->node);

   link_blocks(before_block, then_block, 0);
   link_blocks(before_block, after_block, 1);
   link_blocks(then_block, after_block, 0);

   return then_block;
}

static bool
lower_instr(struct ir3 *ir, struct ir3_block **block, struct ir3_instruction *instr)
{
   switch (instr->opc) {
   case OPC_BALLOT_MACRO:
   case OPC_ANY_MACRO:
   case OPC_ALL_MACRO:
   case OPC_ELECT_MACRO:
   case OPC_READ_COND_MACRO:
   case OPC_READ_FIRST_MACRO:
   case OPC_SWZ_SHARED_MACRO:
   case OPC_SCAN_MACRO:
      break;
   default:
      return false;
   }

   struct ir3_block *before_block = *block;
   struct ir3_block *after_block = split_block(ir, before_block, instr);

   if (instr->opc == OPC_SCAN_MACRO) {
      /* The pseudo-code for the scan macro is:
       *
       * while (true) {
       *    header:
       *    if (elect()) {
       *       exit:
       *       exclusive = reduce;
       *       inclusive = src OP exclusive;
       *       reduce = inclusive;
       *    }
       *    footer:
       * }
       *
       * This is based on the blob's sequence, and carefully crafted to avoid
       * using the shared register "reduce" except in move instructions, since
       * using it in the actual OP isn't possible for half-registers.
       */
      struct ir3_block *header = ir3_block_create(ir);
      list_add(&header->node, &before_block->node);

      struct ir3_block *exit = ir3_block_create(ir);
      list_add(&exit->node, &header->node);

      struct ir3_block *footer = ir3_block_create(ir);
      list_add(&footer->node, &exit->node);

      link_blocks(before_block, header, 0);

      link_blocks(header, exit, 0);
      link_blocks(header, footer, 1);
      header->brtype = IR3_BRANCH_GETONE;

      link_blocks(exit, after_block, 0);
      link_blocks_physical(exit, footer, 1);

      link_blocks(footer, header, 0);

      struct ir3_register *exclusive = instr->dsts[0];
      struct ir3_register *inclusive = instr->dsts[1];
      struct ir3_register *reduce = instr->dsts[2];
      struct ir3_register *src = instr->srcs[0];

      mov_reg(exit, exclusive, reduce);
      do_reduce(exit, instr->cat1.reduce_op, inclusive, src, exclusive);
      mov_reg(exit, reduce, inclusive);
   } else {
      struct ir3_block *then_block = create_if(ir, before_block, after_block);

      /* For ballot, the destination must be initialized to 0 before we do
       * the movmsk because the condition may be 0 and then the movmsk will
       * be skipped. Because it's a shared register we have to wrap the
       * initialization in a getone block.
       */
      if (instr->opc == OPC_BALLOT_MACRO) {
         before_block->brtype = IR3_BRANCH_GETONE;
         before_block->condition = NULL;
         mov_immed(instr->dsts[0], then_block, 0);
         before_block = after_block;
         after_block = split_block(ir, before_block, instr);
         then_block = create_if(ir, before_block, after_block);
      }

      switch (instr->opc) {
      case OPC_BALLOT_MACRO:
      case OPC_READ_COND_MACRO:
      case OPC_ANY_MACRO:
      case OPC_ALL_MACRO:
         before_block->condition = instr->srcs[0]->def->instr;
         break;
      default:
         before_block->condition = NULL;
         break;
      }

      switch (instr->opc) {
      case OPC_BALLOT_MACRO:
      case OPC_READ_COND_MACRO:
         before_block->brtype = IR3_BRANCH_COND;
         break;
      case OPC_ANY_MACRO:
         before_block->brtype = IR3_BRANCH_ANY;
         break;
      case OPC_ALL_MACRO:
         before_block->brtype = IR3_BRANCH_ALL;
         break;
      case OPC_ELECT_MACRO:
      case OPC_READ_FIRST_MACRO:
      case OPC_SWZ_SHARED_MACRO:
         before_block->brtype = IR3_BRANCH_GETONE;
         break;
      default:
         unreachable("bad opcode");
      }

      switch (instr->opc) {
      case OPC_ALL_MACRO:
      case OPC_ANY_MACRO:
      case OPC_ELECT_MACRO:
         mov_immed(instr->dsts[0], then_block, 1);
         mov_immed(instr->dsts[0], before_block, 0);
         break;

      case OPC_BALLOT_MACRO: {
         unsigned comp_count = util_last_bit(instr->dsts[0]->wrmask);
         struct ir3_instruction *movmsk =
            ir3_instr_create(then_block, OPC_MOVMSK, 1, 0);
         ir3_dst_create(movmsk, instr->dsts[0]->num, instr->dsts[0]->flags);
         movmsk->repeat = comp_count - 1;
         break;
      }

      case OPC_READ_COND_MACRO:
      case OPC_READ_FIRST_MACRO: {
         struct ir3_instruction *mov =
            ir3_instr_create(then_block, OPC_MOV, 1, 1);
         unsigned src = instr->opc == OPC_READ_COND_MACRO ? 1 : 0;
         ir3_dst_create(mov, instr->dsts[0]->num, instr->dsts[0]->flags);
         struct ir3_register *new_src = ir3_src_create(mov, 0, 0);
         *new_src = *instr->srcs[src];
         mov->cat1.dst_type = TYPE_U32;
         mov->cat1.src_type =
            (new_src->flags & IR3_REG_HALF) ? TYPE_U16 : TYPE_U32;
         break;
      }

      case OPC_SWZ_SHARED_MACRO: {
         struct ir3_instruction *swz =
            ir3_instr_create(then_block, OPC_SWZ, 2, 2);
         ir3_dst_create(swz, instr->dsts[0]->num, instr->dsts[0]->flags);
         ir3_dst_create(swz, instr->dsts[1]->num, instr->dsts[1]->flags);
         ir3_src_create(swz, instr->srcs[0]->num, instr->srcs[0]->flags);
         ir3_src_create(swz, instr->srcs[1]->num, instr->srcs[1]->flags);
         swz->cat1.dst_type = swz->cat1.src_type = TYPE_U32;
         swz->repeat = 1;
         break;
      }

      default:
         unreachable("bad opcode");
      }
   }

   *block = after_block;
   list_delinit(&instr->node);
   return true;
}

static bool
lower_block(struct ir3 *ir, struct ir3_block **block)
{
   bool progress = true;

   bool inner_progress;
   do {
      inner_progress = false;
      foreach_instr (instr, &(*block)->instr_list) {
         if (lower_instr(ir, block, instr)) {
            /* restart the loop with the new block we created because the
             * iterator has been invalidated.
             */
            progress = inner_progress = true;
            break;
         }
      }
   } while (inner_progress);

   return progress;
}

bool
ir3_lower_subgroups(struct ir3 *ir)
{
   bool progress = false;

   foreach_block (block, &ir->block_list)
      progress |= lower_block(ir, &block);

   return progress;
}
