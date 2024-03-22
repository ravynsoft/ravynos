/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include "mme_builder.h"

#include <stdio.h>
#include <stdlib.h>

#define MME_TU104_MAX_REGS 23

void
mme_tu104_builder_init(struct mme_builder *b)
{
   mme_reg_alloc_init(&b->reg_alloc, BITFIELD_MASK(MME_TU104_MAX_REGS));
}

static void
mme_tu104_new_inst(struct mme_tu104_builder *tb)
{
   struct mme_tu104_inst noop = { MME_TU104_INST_DEFAULTS };
   assert(tb->inst_count < ARRAY_SIZE(tb->insts));
   tb->insts[tb->inst_count] = noop;
   tb->inst_count++;
   tb->inst_parts = 0;
}

static struct mme_tu104_inst *
mme_tu104_cur_inst(struct mme_tu104_builder *tb)
{
   assert(tb->inst_count > 0 && tb->inst_count < ARRAY_SIZE(tb->insts));
   return &tb->insts[tb->inst_count - 1];
}

static inline void
mme_tu104_set_inst_parts(struct mme_tu104_builder *tb,
                         enum mme_tu104_instr_parts parts)
{
   assert(!(tb->inst_parts & parts));
   tb->inst_parts |= parts;
}

void
mme_tu104_add_inst(struct mme_builder *b,
                   const struct mme_tu104_inst *inst)
{
   struct mme_tu104_builder *tb = &b->tu104;

   if (tb->inst_parts || tb->inst_count == 0)
      mme_tu104_new_inst(&b->tu104);
   *mme_tu104_cur_inst(tb) = *inst;
   mme_tu104_new_inst(tb);
}

static unsigned
mme_tu104_reg_num_imms(enum mme_tu104_reg reg)
{
   switch (reg) {
   case MME_TU104_REG_IMM:
   case MME_TU104_REG_IMMPAIR:
      return 1;
   case MME_TU104_REG_IMM32:
      return 2;
   default:
      return 0;
   }
}

static bool
mme_tu104_next_inst_can_add_alu(struct mme_tu104_builder *tb,
                                const struct mme_tu104_alu *alu,
                                bool must_be_alu0)
{
   if (tb->inst_count == 0)
      return false;

   /* Most ALU can be re-ordered with respect to outputs but a couple can't.
    * In the case where it may depend on an output, flush if we have one.
    */
   if (mme_tu104_alu_op_may_depend_on_mthd(alu->op) &&
       tb->inst_parts & (MME_TU104_INSTR_PART_MTHD0 |
                         MME_TU104_INSTR_PART_EMIT0 |
                         MME_TU104_INSTR_PART_MTHD1 |
                         MME_TU104_INSTR_PART_EMIT1))
      return false;

   if (must_be_alu0 && (tb->inst_parts & MME_TU104_INSTR_PART_ALU0))
      return false;

   if (tb->inst_parts & MME_TU104_INSTR_PART_ALU1) {
      assert(tb->inst_parts & MME_TU104_INSTR_PART_ALU0);
      return false;
   }

   assert(alu->src[0] != MME_TU104_REG_LOAD1 &&
          alu->src[1] != MME_TU104_REG_LOAD0 &&
          alu->src[1] != MME_TU104_REG_LOAD1);
   if (alu->src[0] == MME_TU104_REG_LOAD0 &&
       (tb->inst_parts & MME_TU104_INSTR_PART_LOAD1))
      return false;

   const unsigned used_imms =
      util_bitcount(tb->inst_parts & (MME_TU104_INSTR_PART_IMM0 |
                                      MME_TU104_INSTR_PART_IMM1));

   const unsigned num_imms = mme_tu104_alu_op_has_implicit_imm(alu->op) +
                             mme_tu104_reg_num_imms(alu->src[0]) +
                             mme_tu104_reg_num_imms(alu->src[1]);
   assert(num_imms <= 2);
   if (num_imms + used_imms > 2)
      return false;

   if (mme_tu104_alu_op_has_implicit_imm(alu->op) &&
       (tb->inst_parts & MME_TU104_INSTR_PART_ALU0) &&
       (tb->inst_parts & MME_TU104_INSTR_PART_IMM1))
      return false;

   struct mme_tu104_inst *cur = mme_tu104_cur_inst(tb);

   if ((tb->inst_parts & MME_TU104_INSTR_PART_ALU0) &&
       mme_tu104_alus_have_dependency(&cur->alu[0], alu))
      return false;

   /* No idea why the HW has this rule but it does */
   if (alu->op == MME_TU104_ALU_OP_STATE &&
       (tb->inst_parts & MME_TU104_INSTR_PART_ALU0) &&
       cur->alu[0].op == MME_TU104_ALU_OP_STATE)
      return false;

   return true;
}

static unsigned
mme_tu104_push_alu(struct mme_tu104_builder *tb,
                   const struct mme_tu104_alu *alu,
                   uint16_t imm0, uint16_t imm1,
                   uint16_t implicit_imm,
                   bool must_be_alu0)
{
   if (!mme_tu104_next_inst_can_add_alu(tb, alu, must_be_alu0))
      mme_tu104_new_inst(tb);

   if (mme_tu104_alu_op_has_implicit_imm(alu->op) &&
       (tb->inst_parts & MME_TU104_INSTR_PART_IMM0))
      tb->inst_parts |= MME_TU104_INSTR_PART_ALU0;

   assert(mme_tu104_next_inst_can_add_alu(tb, alu, must_be_alu0));

   struct mme_tu104_inst *inst = mme_tu104_cur_inst(tb);
   unsigned alu_idx = (tb->inst_parts & MME_TU104_INSTR_PART_ALU0) != 0;
   assert(alu_idx == 0 || !must_be_alu0);

   switch (alu->op) {
   case MME_TU104_ALU_OP_ADDC:
      assert(inst->alu[0].op == MME_TU104_ALU_OP_ADD);
      assert(alu_idx == 1);
      break;
   case MME_TU104_ALU_OP_SUBB:
      assert(inst->alu[0].op == MME_TU104_ALU_OP_SUB);
      assert(alu_idx == 1);
      break;
   case MME_TU104_ALU_OP_MULH:
      assert(inst->alu[0].op == MME_TU104_ALU_OP_MUL ||
             inst->alu[0].op == MME_TU104_ALU_OP_MULU);
      assert(alu_idx == 1);
      break;
   default:
      break;
   }

   mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_ALU0 << alu_idx);
   inst->alu[alu_idx] = *alu;

   if (alu->src[0] == MME_TU104_REG_LOAD0) {
      unsigned next_load = (tb->inst_parts & MME_TU104_INSTR_PART_LOAD0) != 0;
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_LOAD0 << next_load);
      inst->alu[alu_idx].src[0] = MME_TU104_REG_LOAD0 + next_load;
   }

   unsigned next_imm = (tb->inst_parts & MME_TU104_INSTR_PART_IMM0) != 0;
   const unsigned num_imms = mme_tu104_reg_num_imms(alu->src[0]) +
                             mme_tu104_reg_num_imms(alu->src[1]);

   if (mme_tu104_alu_op_has_implicit_imm(alu->op)) {
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_IMM0 << alu_idx);
      inst->imm[alu_idx] = implicit_imm;
      assert(num_imms <= 1);
      next_imm = 1 - alu_idx;
   }

   if (num_imms == 1) {
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_IMM0 << next_imm);
      inst->imm[next_imm] = imm0;
      assert(alu->src[0] != MME_TU104_REG_IMM32 &&
             alu->src[0] != MME_TU104_REG_IMMPAIR &&
             alu->src[1] != MME_TU104_REG_IMM32 &&
             alu->src[1] != MME_TU104_REG_IMMPAIR);
      if (alu->src[0] == MME_TU104_REG_IMM && alu_idx != next_imm)
         inst->alu[alu_idx].src[0] = MME_TU104_REG_IMMPAIR;
      if (alu->src[1] == MME_TU104_REG_IMM && alu_idx != next_imm)
         inst->alu[alu_idx].src[1] = MME_TU104_REG_IMMPAIR;
   } else if (num_imms == 2) {
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_IMM0 |
                                   MME_TU104_INSTR_PART_IMM1);
      inst->imm[0] = imm0;
      inst->imm[1] = imm1;
   }

   return alu_idx;
}

static inline enum mme_tu104_reg
mme_value_alu_reg(struct mme_value val)
{
   switch (val.type) {
   case MME_VALUE_TYPE_ZERO:
      return MME_TU104_REG_ZERO;
   case MME_VALUE_TYPE_IMM:
      if (val.imm == 0)
         return MME_TU104_REG_ZERO;
      else if (val.imm == (uint32_t)(int16_t)val.imm)
         return MME_TU104_REG_IMM;
      else
         return MME_TU104_REG_IMM32;
   case MME_VALUE_TYPE_REG:
      assert(val.reg <= 23);
      return MME_TU104_REG_R0 + val.reg;
   }
   unreachable("Invalid value type");
}

static void
build_alu_to(struct mme_builder *b,
             struct mme_value dst,
             enum mme_tu104_alu_op op,
             struct mme_value x,
             struct mme_value y,
             uint16_t implicit_imm,
             bool must_be_alu0)
{
   assert(dst.type == MME_VALUE_TYPE_ZERO ||
          dst.type == MME_VALUE_TYPE_REG);

   enum mme_tu104_reg x_reg = mme_value_alu_reg(x);
   enum mme_tu104_reg y_reg = mme_value_alu_reg(y);

   if (x_reg == MME_TU104_REG_IMM32 && y_reg == MME_TU104_REG_IMM32) {
      y = mme_mov(b, y);
      y_reg = mme_value_alu_reg(y);
   }

   if (mme_tu104_alu_op_has_implicit_imm(op) &&
       (x_reg == MME_TU104_REG_IMM32 ||
        (x_reg == MME_TU104_REG_IMM && y_reg == MME_TU104_REG_IMM))) {
      x = mme_mov(b, x);
      x_reg = mme_value_alu_reg(x);
   }

   uint16_t imm0 = 0, imm1 = 0;
   if (x_reg == MME_TU104_REG_IMM32) {
      assert(mme_tu104_reg_num_imms(y_reg) == 0);
      imm0 = x.imm >> 16;
      imm1 = x.imm;
   } else if (y_reg == MME_TU104_REG_IMM32) {
      assert(mme_tu104_reg_num_imms(x_reg) == 0);
      imm0 = y.imm >> 16;
      imm1 = y.imm;
   } else if (x_reg == MME_TU104_REG_IMM) {
      assert(mme_tu104_reg_num_imms(y_reg) <= 1);
      imm0 = x.imm;
      if (y_reg == MME_TU104_REG_IMM) {
         imm1 = y.imm;
         y_reg = MME_TU104_REG_IMMPAIR;
      }
   } else if (y_reg == MME_TU104_REG_IMM) {
      imm0 = y.imm;
   } else {
      assert(mme_tu104_reg_num_imms(x_reg) == 0);
      assert(mme_tu104_reg_num_imms(y_reg) == 0);
   }

   struct mme_tu104_alu alu = {
      .dst = mme_value_alu_reg(dst),
      .op = op,
      .src = { x_reg, y_reg },
   };
   mme_tu104_push_alu(&b->tu104, &alu, imm0, imm1, implicit_imm, must_be_alu0);
}

static enum mme_tu104_alu_op
mme_to_tu104_alu_op(enum mme_alu_op op)
{
   switch (op) {
#define ALU_CASE(op) case MME_ALU_OP_##op: return MME_TU104_ALU_OP_##op;
   ALU_CASE(ADD)
   ALU_CASE(ADDC)
   ALU_CASE(SUB)
   ALU_CASE(SUBB)
   ALU_CASE(MUL)
   ALU_CASE(MULH)
   ALU_CASE(MULU)
   ALU_CASE(CLZ)
   ALU_CASE(SLL)
   ALU_CASE(SRL)
   ALU_CASE(SRA)
   ALU_CASE(AND)
   ALU_CASE(NAND)
   ALU_CASE(OR)
   ALU_CASE(XOR)
   ALU_CASE(SLT)
   ALU_CASE(SLTU)
   ALU_CASE(SLE)
   ALU_CASE(SLEU)
   ALU_CASE(SEQ)
   ALU_CASE(DREAD)
   ALU_CASE(DWRITE)
#undef ALU_CASE
   default:
      unreachable("Unsupported MME ALU op");
   }
}

void
mme_tu104_alu_to(struct mme_builder *b,
                 struct mme_value dst,
                 enum mme_alu_op op,
                 struct mme_value x,
                 struct mme_value y)
{
   build_alu_to(b, dst, mme_to_tu104_alu_op(op), x, y, 0, false);
}

void
mme_tu104_alu64_to(struct mme_builder *b,
                   struct mme_value64 dst,
                   enum mme_alu_op op_lo,
                   enum mme_alu_op op_hi,
                   struct mme_value64 x,
                   struct mme_value64 y)
{
   assert(dst.lo.type == MME_VALUE_TYPE_REG);
   assert(dst.hi.type == MME_VALUE_TYPE_REG);

   /* We can't have any non-zero immediates in the high part or else we might
    * get half-way through emitting and realize we've run out.
    */
   if (x.hi.type == MME_VALUE_TYPE_IMM && x.hi.imm != 0)
      x.hi = mme_mov(b, x.hi);
   if (y.hi.type == MME_VALUE_TYPE_IMM && y.hi.imm != 0)
      y.hi = mme_mov(b, y.hi);

   build_alu_to(b, dst.lo, mme_to_tu104_alu_op(op_lo), x.lo, y.lo, 0, true);
   build_alu_to(b, dst.hi, mme_to_tu104_alu_op(op_hi), x.hi, y.hi, 0, false);
}

void
mme_tu104_merge_to(struct mme_builder *b, struct mme_value dst,
                   struct mme_value x, struct mme_value y,
                   uint16_t dst_pos, uint16_t bits, uint16_t src_pos)
{
   assert(dst_pos < 32);
   assert(bits < 32);
   assert(src_pos < 32);
   uint32_t ctrl = (dst_pos << 10) | (bits << 5) | src_pos;
   build_alu_to(b, dst, MME_TU104_ALU_OP_MERGE, x, y, ctrl, false);
}

void
mme_tu104_state_arr_to(struct mme_builder *b, struct mme_value dst,
                       uint16_t state, struct mme_value index)
{
   assert(state % 4 == 0);
   build_alu_to(b, dst, MME_TU104_ALU_OP_STATE,
                mme_imm(state >> 2), index, 0, false);
}

void
mme_tu104_load_barrier(struct mme_builder *b)
{
   build_alu_to(b, mme_zero(), MME_TU104_ALU_OP_EXTENDED,
                mme_imm(0x1000), mme_imm(1), 0, false);
}

void
mme_tu104_load_to(struct mme_builder *b, struct mme_value dst)
{
   assert(dst.type == MME_VALUE_TYPE_REG ||
          dst.type == MME_VALUE_TYPE_ZERO);

   struct mme_tu104_alu alu = {
      .dst = mme_value_alu_reg(dst),
      .op = MME_TU104_ALU_OP_ADD,
      .src = {
         MME_TU104_REG_LOAD0,
         MME_TU104_REG_ZERO,
      },
   };
   mme_tu104_push_alu(&b->tu104, &alu, 0, 0, 0, 0);
}

static bool
mme_tu104_next_inst_can_add_mthd(struct mme_tu104_builder *tb,
                                 enum mme_tu104_out_op out)
{
   if (tb->inst_count == 0)
      return false;

   if (tb->inst_parts & MME_TU104_INSTR_PART_MTHD1) {
      assert(tb->inst_parts & MME_TU104_INSTR_PART_MTHD0);
      return false;
   }

   if (out == MME_TU104_OUT_OP_IMM0 &&
       (tb->inst_parts & MME_TU104_INSTR_PART_IMM0) &&
       (tb->inst_parts & MME_TU104_INSTR_PART_IMM1))
      return false;

   return true;
}

static void
mme_tu104_push_mthd(struct mme_tu104_builder *tb,
                    enum mme_tu104_out_op out,
                    uint16_t imm)
{
   struct mme_tu104_inst *inst = mme_tu104_cur_inst(tb);
   if (out == MME_TU104_OUT_OP_IMM0) {
      unsigned imm_idx = (tb->inst_parts & MME_TU104_INSTR_PART_IMM0) != 0;
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_IMM0 << imm_idx);
      out = MME_TU104_OUT_OP_IMM0 + imm_idx;
      inst->imm[imm_idx] = imm;
   }
   unsigned mthd_idx = (tb->inst_parts & MME_TU104_INSTR_PART_MTHD0) != 0;
   /* If we're pushing mthd1, the next emit MUST be emit1 */
   if (mthd_idx > 0 && !(tb->inst_parts & MME_TU104_INSTR_PART_EMIT0))
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_EMIT0);
   mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_MTHD0 << mthd_idx);
   inst->out[mthd_idx].mthd = out;
}

void
mme_tu104_mthd(struct mme_builder *b, uint16_t mthd, struct mme_value index)
{
   struct mme_tu104_builder *tb = &b->tu104;

   assert(mthd % 4 == 0);
   uint32_t mthd_imm = (1 << 12) | (mthd >> 2);

   if (index.type == MME_VALUE_TYPE_REG) {
      if (!mme_tu104_next_inst_can_add_mthd(tb, MME_TU104_OUT_OP_ALU0))
         mme_tu104_new_inst(tb);

      const struct mme_tu104_alu alu = {
         .dst = MME_TU104_REG_ZERO,
         .op = MME_TU104_ALU_OP_ADD,
         .src = {
            MME_TU104_REG_IMM,
            mme_value_alu_reg(index),
         },
      };
      unsigned alu_idx = mme_tu104_push_alu(tb, &alu, mthd_imm, 0, 0, false);
      mme_tu104_push_mthd(tb, MME_TU104_OUT_OP_ALU0 + alu_idx, 0);
   } else {
      if (!mme_tu104_next_inst_can_add_mthd(tb, MME_TU104_OUT_OP_IMM0))
         mme_tu104_new_inst(tb);

      if (index.type == MME_VALUE_TYPE_IMM)
         mthd_imm += index.imm;

      mme_tu104_push_mthd(tb, MME_TU104_OUT_OP_IMM0, mthd_imm);
   }
}

static bool
mme_tu104_next_inst_can_add_emit(struct mme_tu104_builder *tb,
                                 enum mme_tu104_out_op out,
                                 uint32_t imm)
{
   assert(tb->inst_count > 0);

   if (tb->inst_parts & MME_TU104_INSTR_PART_EMIT1) {
      assert(tb->inst_parts & MME_TU104_INSTR_PART_EMIT0);
      return false;
   }

   const unsigned used_imms =
      util_bitcount(tb->inst_parts & (MME_TU104_INSTR_PART_IMM0 |
                                      MME_TU104_INSTR_PART_IMM1));
   if (out == MME_TU104_OUT_OP_IMM0 && used_imms > 1)
      return false;
   if (out == MME_TU104_OUT_OP_IMM32 && used_imms > 0)
      return false;

   return true;
}

static void
mme_tu104_push_emit(struct mme_tu104_builder *tb,
                    enum mme_tu104_out_op out,
                    uint32_t imm)
{
   struct mme_tu104_inst *inst = mme_tu104_cur_inst(tb);
   if (out == MME_TU104_OUT_OP_IMM0) {
      unsigned imm_idx = (tb->inst_parts & MME_TU104_INSTR_PART_IMM0) != 0;
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_IMM0 << imm_idx);
      out = MME_TU104_OUT_OP_IMM0 + imm_idx;
      inst->imm[imm_idx] = imm;
   } else if (out == MME_TU104_OUT_OP_IMM32) {
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_IMM0 |
                                   MME_TU104_INSTR_PART_IMM1);
      inst->imm[0] = imm >> 16;
      inst->imm[1] = imm;
   }
   unsigned emit_idx = (tb->inst_parts & MME_TU104_INSTR_PART_EMIT0) != 0;
   mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_EMIT0 << emit_idx);
   /* If we're pushing emitN, the next mthd MUST be mthdN+1 */
   if (!(tb->inst_parts & (MME_TU104_INSTR_PART_MTHD0 << emit_idx)))
      mme_tu104_set_inst_parts(tb, MME_TU104_INSTR_PART_MTHD0 << emit_idx);
   inst->out[emit_idx].emit = out;
}

static int
find_alu_idx_for_dst(const struct mme_tu104_inst *inst,
                     struct mme_value dst)
{
   assert(dst.type == MME_VALUE_TYPE_REG);
   for (int i = 0; i < 2; i++) {
      if (inst->alu[i].dst == mme_value_alu_reg(dst))
         return i;
   }
   return -1;
}

void
mme_tu104_emit(struct mme_builder *b, struct mme_value data)
{
   struct mme_tu104_builder *tb = &b->tu104;

   if (data.type == MME_VALUE_TYPE_REG) {
      if (!mme_tu104_next_inst_can_add_emit(tb, MME_TU104_OUT_OP_ALU0, 0))
         mme_tu104_new_inst(tb);

      struct mme_tu104_inst *inst = mme_tu104_cur_inst(tb);
      int alu_idx = find_alu_idx_for_dst(inst, data);
      if (alu_idx < 0) {
         const struct mme_tu104_alu alu = {
            .dst = MME_TU104_REG_ZERO,
            .op = MME_TU104_ALU_OP_ADD,
            .src = {
               mme_value_alu_reg(data),
               MME_TU104_REG_ZERO,
            },
         };
         alu_idx = mme_tu104_push_alu(tb, &alu, 0, 0, 0, false);
      }
      mme_tu104_push_emit(tb, MME_TU104_OUT_OP_ALU0 + alu_idx, 0);
   } else {
      enum mme_tu104_out_op out;
      uint32_t imm;
      if (data.type == MME_VALUE_TYPE_ZERO) {
         out = MME_TU104_OUT_OP_IMM0;
         imm = 0;
      } else {
         assert(data.type == MME_VALUE_TYPE_IMM);
         imm = data.imm;
         out = data.imm == (uint16_t)data.imm ? MME_TU104_OUT_OP_IMM0 :
                                                MME_TU104_OUT_OP_IMM32;
      }
      if (!mme_tu104_next_inst_can_add_emit(tb, out, 0))
         mme_tu104_new_inst(tb);

      mme_tu104_push_emit(tb, out, imm);
   }
}

static enum mme_tu104_alu_op
mme_cmp_to_tu104_branch_op(enum mme_cmp_op op)
{
   switch (op) {
#define CMP_CASE(op) case MME_CMP_OP_##op: return MME_TU104_ALU_OP_B##op;
   CMP_CASE(LT)
   CMP_CASE(LTU)
   CMP_CASE(LE)
   CMP_CASE(LEU)
   CMP_CASE(EQ)
#undef CMP_CASE
   default:
      unreachable("Unsupported MME CMP op");
   }
}

static void
mme_tu104_start_cf(struct mme_builder *b,
                   enum mme_cf_type type,
                   enum mme_tu104_alu_op op,
                   struct mme_value x,
                   struct mme_value y,
                   uint16_t control)
{
   struct mme_tu104_builder *tb = &b->tu104;

   /* The HW seems to want at least LOOP to always be in alu0 */
   build_alu_to(b, mme_zero(), op, x, y, control, true);

   uint16_t ip = tb->inst_count - 1;
   assert(tb->insts[ip].alu[0].op == op);

   tb->cf_stack[tb->cf_depth++] = (struct mme_cf) {
      .type = type,
      .start_ip = ip,
   };

   /* The inside of control-flow needs to start with a new instruction */
   mme_tu104_new_inst(tb);
}

static struct mme_cf
mme_tu104_end_cf(struct mme_builder *b, enum mme_cf_type type)
{
   struct mme_tu104_builder *tb = &b->tu104;

   if (tb->inst_parts)
      mme_tu104_new_inst(tb);

   assert(tb->cf_depth > 0);
   struct mme_cf cf = tb->cf_stack[--tb->cf_depth];
   assert(cf.type == type);

   int delta = tb->inst_count - cf.start_ip - 1;
   assert(delta > 0 && delta < (1 << 13));
   tb->insts[cf.start_ip].imm[0] |= delta;

   return cf;
}

void
mme_tu104_start_loop(struct mme_builder *b, struct mme_value count)
{
   mme_tu104_start_cf(b, MME_CF_TYPE_LOOP, MME_TU104_ALU_OP_LOOP,
                      count, mme_zero(), 0);
}

void
mme_tu104_end_loop(struct mme_builder *b)
{
   mme_tu104_end_cf(b, MME_CF_TYPE_LOOP);
}

void
mme_tu104_start_if(struct mme_builder *b,
                   enum mme_cmp_op op, bool if_true,
                   struct mme_value x, struct mme_value y)
{
   uint16_t control = if_true ? 0 : BITFIELD_BIT(15);
   mme_tu104_start_cf(b, MME_CF_TYPE_IF, mme_cmp_to_tu104_branch_op(op),
                      x, y, control);
}

void
mme_tu104_end_if(struct mme_builder *b)
{
   mme_tu104_end_cf(b, MME_CF_TYPE_IF);
}

void
mme_tu104_start_while(struct mme_builder *b)
{
   mme_tu104_start_cf(b, MME_CF_TYPE_WHILE, MME_TU104_ALU_OP_JAL,
                      mme_zero(), mme_zero(), BITFIELD_BIT(15));
}

void
mme_tu104_end_while(struct mme_builder *b,
                    enum mme_cmp_op cmp,
                    bool if_true,
                    struct mme_value x,
                    struct mme_value y)
{
   struct mme_tu104_builder *tb = &b->tu104;

   struct mme_cf cf = mme_tu104_end_cf(b, MME_CF_TYPE_WHILE);

   int delta = tb->inst_count - cf.start_ip - 2;
   uint16_t control = (-delta & BITFIELD_MASK(13)) |
                      (if_true ? BITFIELD_BIT(15) : 0);
   build_alu_to(b, mme_zero(), mme_cmp_to_tu104_branch_op(cmp),
                x, y, control, true);

   /* Start a new instruction so next thing to come along doesn't end up being
    * the 2nd half of of our back-edge while.
    */
   mme_tu104_new_inst(tb);
}

void mme_tu104_exit_if(struct mme_builder *b,
                       enum mme_cmp_op op,
                       bool if_true,
                       struct mme_value x,
                       struct mme_value y)
{
   struct mme_tu104_builder *tb = &b->tu104;

   /* we reverse it as we want to take the branch if the condition is true */
   uint16_t control = if_true ? BITFIELD_BIT(15) : 0;
   /* magic offset to exit the macro */
   control |= 0x1000;
   build_alu_to(b, mme_zero(), mme_cmp_to_tu104_branch_op(op), x, y, control,
                true);

   mme_tu104_new_inst(tb);
}

uint32_t *
mme_tu104_builder_finish(struct mme_tu104_builder *tb, size_t *size_out)
{
   assert(tb->cf_depth == 0);

   /* TODO: If there are at least two instructions and we can guarantee the
    * last two instructions get exeucted (not in control-flow), we don't need
    * to add a pair of NOPs.
    */
   mme_tu104_new_inst(tb);
   mme_tu104_new_inst(tb);
   tb->insts[tb->inst_count - 2].end_next = true;

   if (0)
      mme_tu104_print(stderr, tb->insts, tb->inst_count);

   size_t enc_size = tb->inst_count * 3 * sizeof(uint32_t);
   uint32_t *enc = malloc(enc_size);
   if (enc != NULL) {
      mme_tu104_encode(enc, tb->inst_count, tb->insts);
      *size_out = enc_size;
   }
   return enc;
}

void
mme_tu104_builder_dump(struct mme_builder *b, FILE *fp)
{
   struct mme_tu104_builder *tb = &b->tu104;

   mme_tu104_print(stderr, tb->insts, tb->inst_count);
}
