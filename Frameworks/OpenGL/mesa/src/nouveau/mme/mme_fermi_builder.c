/*
 * Copyright © 2022 Mary Guillemard
 * SPDX-License-Identifier: MIT
 */
#include "mme_builder.h"

#include <stdio.h>
#include <stdlib.h>

#include "util/u_math.h"

void
mme_fermi_builder_init(struct mme_builder *b)
{
   /* R0 is reserved for the zero register */
   mme_reg_alloc_init(&b->reg_alloc, 0xfe);

   /* Pre-allocate R1 for the first parameter value */
   ASSERTED struct mme_value r1 = mme_reg_alloc_alloc(&b->reg_alloc);
   assert(r1.reg == 1);
}

static inline bool
mme_fermi_is_zero_or_reg(struct mme_value x)
{
   switch (x.type) {
   case MME_VALUE_TYPE_ZERO:  return true;
   case MME_VALUE_TYPE_IMM:   return x.imm == 0;
   case MME_VALUE_TYPE_REG:   return true;
   default: unreachable("Invalid MME value type");
   }
}

static inline bool
mme_fermi_is_zero_or_imm(struct mme_value x)
{
   switch (x.type) {
   case MME_VALUE_TYPE_ZERO:  return true;
   case MME_VALUE_TYPE_IMM:   return true;
   case MME_VALUE_TYPE_REG:   return false;
   default: unreachable("Invalid MME value type");
   }
}

static inline enum mme_fermi_reg
mme_value_alu_reg(struct mme_value val)
{
   assert(mme_fermi_is_zero_or_reg(val));

   switch (val.type) {
   case MME_VALUE_TYPE_ZERO:
      return MME_FERMI_REG_ZERO;
   case MME_VALUE_TYPE_REG:
      assert(val.reg > 0 && val.reg <= 7);
      return MME_FERMI_REG_ZERO + val.reg;
   case MME_VALUE_TYPE_IMM:
      return MME_FERMI_REG_ZERO;
   }
   unreachable("Invalid value type");
}

static inline uint32_t
mme_value_alu_imm(struct mme_value val)
{
   assert(mme_fermi_is_zero_or_imm(val));

   switch (val.type) {
   case MME_VALUE_TYPE_ZERO:
      return 0;
   case MME_VALUE_TYPE_IMM:
      return val.imm;
   case MME_VALUE_TYPE_REG:
      return 0;
   }
   unreachable("Invalid value type");
}

static inline void
mme_free_reg_if_tmp(struct mme_builder *b,
                    struct mme_value data,
                    struct mme_value maybe_tmp)
{
   if (!mme_is_zero(data) &&
       !mme_is_zero(maybe_tmp) &&
       data.type != maybe_tmp.type)
      mme_free_reg(b, maybe_tmp);
}

static void
mme_fermi_new_inst(struct mme_fermi_builder *b)
{
   struct mme_fermi_inst noop = { MME_FERMI_INST_DEFAULTS };
   assert(b->inst_count < ARRAY_SIZE(b->insts));
   b->insts[b->inst_count] = noop;
   b->inst_count++;
   b->inst_parts = 0;
}

static struct mme_fermi_inst *
mme_fermi_cur_inst(struct mme_fermi_builder *b)
{
   assert(b->inst_count > 0 && b->inst_count < ARRAY_SIZE(b->insts));
   return &b->insts[b->inst_count - 1];
}

void
mme_fermi_add_inst(struct mme_builder *b,
                   const struct mme_fermi_inst *inst)
{
   struct mme_fermi_builder *fb = &b->fermi;

   if (fb->inst_parts || fb->inst_count == 0)
      mme_fermi_new_inst(fb);

   *mme_fermi_cur_inst(fb) = *inst;
   mme_fermi_new_inst(fb);
}

static inline void
mme_fermi_set_inst_parts(struct mme_fermi_builder *b,
                         enum mme_fermi_instr_parts parts)
{
   assert(!(b->inst_parts & parts));
   b->inst_parts |= parts;
}

static inline bool
mme_fermi_next_inst_can_fit_a_full_inst(struct mme_fermi_builder *b)
{
   return !mme_fermi_is_empty(b) && b->inst_parts == 0;
}

void
mme_fermi_mthd_arr(struct mme_builder *b,
                   uint16_t mthd, struct mme_value index)
{
   struct mme_fermi_builder *fb = &b->fermi;
   struct mme_value src_reg = mme_zero();

   if (!mme_fermi_next_inst_can_fit_a_full_inst(fb))
      mme_fermi_new_inst(fb);

   struct mme_fermi_inst *inst = mme_fermi_cur_inst(fb);

   uint32_t mthd_imm = (1 << 12) | (mthd >> 2);

   if (index.type == MME_VALUE_TYPE_REG) {
      src_reg = index;
   } else if (index.type == MME_VALUE_TYPE_IMM) {
      mthd_imm += index.imm;
   }

   inst->op = MME_FERMI_OP_ADD_IMM;
   inst->src[0] = mme_value_alu_reg(src_reg);
   inst->imm = mthd_imm;
   inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE_SET_MADDR;
   inst->dst = MME_FERMI_REG_ZERO;

   mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_OP |
                                MME_FERMI_INSTR_PART_ASSIGN);
}

static inline bool
mme_fermi_prev_inst_can_emit(struct mme_fermi_builder *b, struct mme_value data) {
   if (mme_fermi_is_empty(b)) {
      return false;
   }

   if ((b->inst_parts & MME_FERMI_INSTR_PART_ASSIGN) == MME_FERMI_INSTR_PART_ASSIGN) {
      struct mme_fermi_inst *inst = mme_fermi_cur_inst(b);

      if (inst->assign_op == MME_FERMI_ASSIGN_OP_MOVE && data.type == MME_VALUE_TYPE_REG &&
          mme_value_alu_reg(data) == inst->dst) {
         return true;
      }
   }

   return false;
}

static inline bool
mme_fermi_next_inst_can_emit(struct mme_fermi_builder *fb,
                             struct mme_value data)
{
   if (mme_fermi_is_empty(fb))
      return false;

   if (fb->inst_parts == 0)
      return true;

   return mme_fermi_prev_inst_can_emit(fb, data);
}

static inline struct mme_value
mme_fermi_reg(uint32_t reg)
{
   struct mme_value val = {
      .type = MME_VALUE_TYPE_REG,
      .reg = reg,
   };
   return val;
}

static bool
is_int18(uint32_t i)
{
   return i == (uint32_t)util_mask_sign_extend(i, 18);
}

static inline void
mme_fermi_add_imm18(struct mme_fermi_builder *fb,
                    struct mme_value dst,
                    struct mme_value src,
                    uint32_t imm)
{
   assert(dst.type == MME_VALUE_TYPE_REG &&
          mme_fermi_is_zero_or_reg(src) && is_int18(imm));

   if (!mme_fermi_next_inst_can_fit_a_full_inst(fb)) {
      mme_fermi_new_inst(fb);
   }

   struct mme_fermi_inst *inst = mme_fermi_cur_inst(fb);

   inst->op = MME_FERMI_OP_ADD_IMM;
   inst->src[0] = mme_value_alu_reg(src);
   inst->imm = imm & BITFIELD_MASK(18);
   inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE;
   inst->dst = mme_value_alu_reg(dst);

   mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_OP |
                                MME_FERMI_INSTR_PART_ASSIGN);
}

static bool
mme_fermi_bfe_lsl_can_use_imm(struct mme_fermi_builder *b,
                              struct mme_value src_bits,
                              struct mme_value dst_bits)
{
   return (mme_fermi_is_zero_or_reg(src_bits) &&
           mme_fermi_is_zero_or_imm(dst_bits) &&
           mme_value_alu_imm(dst_bits) <= 31);
}

static bool
mme_fermi_bfe_lsl_can_use_reg(struct mme_fermi_builder *b,
                              struct mme_value src_bits,
                              struct mme_value dst_bits)
{
   return (mme_fermi_is_zero_or_imm(src_bits) &&
           mme_fermi_is_zero_or_reg(dst_bits) &&
           mme_value_alu_imm(src_bits) <= 31);
}

static void
mme_fermi_bfe(struct mme_fermi_builder *fb,
              struct mme_value dst_reg,
              struct mme_value src_bits,
              struct mme_value src_reg,
              struct mme_value dst_bits,
              uint32_t size)
{
   assert(dst_reg.type == MME_VALUE_TYPE_REG &&
          mme_fermi_is_zero_or_reg(src_reg) &&
          (mme_fermi_bfe_lsl_can_use_imm(fb, src_bits, dst_bits) ||
           mme_fermi_bfe_lsl_can_use_reg(fb, src_bits, dst_bits)));

   if (!mme_fermi_next_inst_can_fit_a_full_inst(fb))
      mme_fermi_new_inst(fb);

   struct mme_fermi_inst *inst = mme_fermi_cur_inst(fb);

   if (mme_fermi_bfe_lsl_can_use_imm(fb, src_bits, dst_bits)) {
      inst->op = MME_FERMI_OP_BFE_LSL_IMM;
      inst->src[0] = mme_value_alu_reg(src_bits);
      inst->src[1] = mme_value_alu_reg(src_reg);
      inst->bitfield.dst_bit = mme_value_alu_imm(dst_bits);
      inst->bitfield.size = size;
   } else if (mme_fermi_bfe_lsl_can_use_reg(fb, src_bits, dst_bits)) {
      inst->op = MME_FERMI_OP_BFE_LSL_REG;
      inst->src[0] = mme_value_alu_reg(dst_bits);
      inst->src[1] = mme_value_alu_reg(src_reg);
      inst->bitfield.src_bit = mme_value_alu_imm(src_bits);
      inst->bitfield.size = size;
   }

   inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE;
   inst->dst = mme_value_alu_reg(dst_reg);

   mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_OP |
                                MME_FERMI_INSTR_PART_ASSIGN);
}

static void
mme_fermi_sll_to(struct mme_fermi_builder *b,
                 struct mme_value dst,
                 struct mme_value x,
                 struct mme_value y)
{
   assert(mme_fermi_is_zero_or_reg(dst));

   mme_fermi_bfe(b, dst, mme_zero(), x, y, 31);
}

static void
mme_fermi_srl_to(struct mme_fermi_builder *b,
                 struct mme_value dst,
                 struct mme_value x,
                 struct mme_value y)
{
   assert(mme_fermi_is_zero_or_reg(dst));

   mme_fermi_bfe(b, dst, y, x, mme_zero(), 31);
}

void
mme_fermi_bfe_to(struct mme_builder *b, struct mme_value dst,
                 struct mme_value x, struct mme_value pos, uint8_t bits)
{
   struct mme_fermi_builder *fb = &b->fermi;
   assert(mme_fermi_is_zero_or_reg(dst));

   mme_fermi_bfe(fb, dst, pos, x, mme_zero(), bits);
}

static struct mme_value
mme_fermi_load_imm_to_reg(struct mme_builder *b, struct mme_value data)
{
   struct mme_fermi_builder *fb = &b->fermi;

   assert(data.type == MME_VALUE_TYPE_IMM ||
          data.type == MME_VALUE_TYPE_ZERO);

   /* If the immediate is zero, we can simplify this */
   if (mme_is_zero(data)) {
      return mme_zero();
   } else {
      uint32_t imm = data.imm;

      struct mme_value dst = mme_alloc_reg(b);

      if (is_int18(imm)) {
         mme_fermi_add_imm18(fb, dst, mme_zero(), imm);
      } else {
         /* TODO: a possible optimisation involve searching for the first bit
          * offset and see if it can fit in 16 bits.
          */
         uint32_t high_bits = imm >> 16;
         uint32_t low_bits = imm & UINT16_MAX;

         mme_fermi_add_imm18(fb, dst, mme_zero(), high_bits);
         mme_fermi_sll_to(fb, dst, dst, mme_imm(16));
         mme_fermi_add_imm18(fb, dst, dst, low_bits);
      }

      return dst;
   }
}

static inline struct mme_value
mme_fermi_value_as_reg(struct mme_builder *b,
                       struct mme_value data)
{
   if (data.type == MME_VALUE_TYPE_REG || mme_is_zero(data)) {
      return data;
   }

   return mme_fermi_load_imm_to_reg(b, data);
}

void mme_fermi_emit(struct mme_builder *b,
                    struct mme_value data)
{
   struct mme_fermi_builder *fb = &b->fermi;
   struct mme_fermi_inst *inst;

   /* Check if previous assign was to the same dst register and modify assign
    * mode if needed
    */
   if (mme_fermi_prev_inst_can_emit(fb, data)) {
      inst = mme_fermi_cur_inst(fb);
      inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE_EMIT;
   } else {
      struct mme_value data_reg = mme_fermi_value_as_reg(b, data);

      /* Because of mme_fermi_value_as_reg, it is possible that a new load
       * that can be simplify
       */
      if (mme_fermi_prev_inst_can_emit(fb, data_reg)) {
         inst = mme_fermi_cur_inst(fb);
         inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE_EMIT;
      } else {
         if (!mme_fermi_next_inst_can_emit(fb, data))
            mme_fermi_new_inst(fb);

         inst = mme_fermi_cur_inst(fb);
         inst->op = MME_FERMI_OP_ALU_REG;
         inst->alu_op = MME_FERMI_ALU_OP_ADD;
         inst->src[0] = mme_value_alu_reg(data_reg);
         inst->src[1] = MME_FERMI_REG_ZERO;
         inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE_EMIT;
         inst->dst = MME_FERMI_REG_ZERO;

         mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_OP |
                                      MME_FERMI_INSTR_PART_ASSIGN);
      }

      mme_free_reg_if_tmp(b, data, data_reg);
   }
}

static void
mme_fermi_branch(struct mme_fermi_builder *fb,
                 enum mme_fermi_reg src, int32_t offset, bool if_zero)
{
   if (fb->inst_parts || mme_fermi_is_empty(fb))
      mme_fermi_new_inst(fb);

   struct mme_fermi_inst *inst = mme_fermi_cur_inst(fb);

   inst->op = MME_FERMI_OP_BRANCH;
   inst->src[0] = src;
   inst->imm = offset;
   inst->branch.no_delay = true;
   inst->branch.not_zero = if_zero;

   mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_OP |
                                MME_FERMI_INSTR_PART_ASSIGN);
}

static void
mme_fermi_start_cf(struct mme_builder *b,
                   enum mme_cf_type type,
                   struct mme_value cond,
                   bool is_zero)
{
   struct mme_fermi_builder *fb = &b->fermi;

   /* The condition here is inverted because we want to branch and skip the
    * block when the condition fails.
    */
   assert(mme_fermi_is_zero_or_reg(cond));
   mme_fermi_branch(fb, mme_value_alu_reg(cond), 0, is_zero);

   uint16_t ip = fb->inst_count - 1;
   assert(fb->insts[ip].op == MME_FERMI_OP_BRANCH);

   assert(fb->cf_depth < ARRAY_SIZE(fb->cf_stack));
   fb->cf_stack[fb->cf_depth++] = (struct mme_cf) {
      .type = type,
      .start_ip = ip,
   };

   /* The inside of control-flow needs to start with a new instruction */
   mme_fermi_new_inst(fb);
}

static struct mme_cf
mme_fermi_end_cf(struct mme_builder *b, enum mme_cf_type type)
{
   struct mme_fermi_builder *fb = &b->fermi;

   if (fb->inst_parts)
      mme_fermi_new_inst(fb);

   assert(fb->cf_depth > 0);
   struct mme_cf cf = fb->cf_stack[--fb->cf_depth];
   assert(cf.type == type);

   assert(fb->insts[cf.start_ip].op == MME_FERMI_OP_BRANCH);
   fb->insts[cf.start_ip].imm = fb->inst_count - cf.start_ip - 1;

   return cf;
}

static struct mme_value
mme_fermi_neq(struct mme_builder *b, struct mme_value x, struct mme_value y)
{
   struct mme_fermi_builder *fb = &b->fermi;

   /* Generate some value that's non-zero if x != y */
   struct mme_value res = mme_alloc_reg(b);
   if (x.type == MME_VALUE_TYPE_IMM && is_int18(-x.imm)) {
      mme_fermi_add_imm18(fb, res, y, -x.imm);
   } else if (y.type == MME_VALUE_TYPE_IMM && is_int18(-y.imm)) {
      mme_fermi_add_imm18(fb, res, x, -y.imm);
   } else {
      mme_xor_to(b, res, x, y);
   }
   return res;
}

void
mme_fermi_start_if(struct mme_builder *b,
                   enum mme_cmp_op op,
                   bool if_true,
                   struct mme_value x,
                   struct mme_value y)
{
   assert(op == MME_CMP_OP_EQ);

   if (mme_is_zero(x)) {
      mme_fermi_start_cf(b, MME_CF_TYPE_IF, y, if_true);
   } else if (mme_is_zero(y)) {
      mme_fermi_start_cf(b, MME_CF_TYPE_IF, x, if_true);
   } else {
      struct mme_value tmp = mme_fermi_neq(b, x, y);
      mme_fermi_start_cf(b, MME_CF_TYPE_IF, tmp, if_true);
      mme_free_reg(b, tmp);
   }
}

void
mme_fermi_end_if(struct mme_builder *b)
{
   mme_fermi_end_cf(b, MME_CF_TYPE_IF);
}

void
mme_fermi_start_while(struct mme_builder *b)
{
   mme_fermi_start_cf(b, MME_CF_TYPE_WHILE, mme_zero(), false);
}

static void
mme_fermi_end_while_zero(struct mme_builder *b,
                         struct mme_cf cf,
                         struct mme_value cond,
                         bool is_zero)
{
   struct mme_fermi_builder *fb = &b->fermi;

   if (fb->inst_parts)
      mme_fermi_new_inst(fb);

   int delta = fb->inst_count - cf.start_ip - 2;
   mme_fermi_branch(fb, mme_value_alu_reg(cond), -delta, !is_zero);
}

void
mme_fermi_end_while(struct mme_builder *b,
                    enum mme_cmp_op op,
                    bool if_true,
                    struct mme_value x,
                    struct mme_value y)
{
   assert(op == MME_CMP_OP_EQ);

   struct mme_cf cf = mme_fermi_end_cf(b, MME_CF_TYPE_WHILE);

   if (mme_is_zero(x)) {
      mme_fermi_end_while_zero(b, cf, y, if_true);
   } else if (mme_is_zero(y)) {
      mme_fermi_end_while_zero(b, cf, x, if_true);
   } else {
      struct mme_value tmp = mme_fermi_neq(b, x, y);
      mme_fermi_end_while_zero(b, cf, tmp, if_true);
      mme_free_reg(b, tmp);
   }
}

void
mme_fermi_start_loop(struct mme_builder *b,
                     struct mme_value count)
{
   struct mme_fermi_builder *fb = &b->fermi;

   assert(mme_is_zero(fb->loop_counter));
   fb->loop_counter = mme_mov(b, count);

   mme_start_while(b);
}

void
mme_fermi_end_loop(struct mme_builder *b)
{
   struct mme_fermi_builder *fb = &b->fermi;

   mme_sub_to(b, fb->loop_counter, fb->loop_counter, mme_imm(1));
   mme_fermi_end_while(b, MME_CMP_OP_EQ, false, fb->loop_counter, mme_zero());

   mme_free_reg(b, fb->loop_counter);
   fb->loop_counter = mme_zero();
}

static inline bool
mme_fermi_next_inst_can_load_to(struct mme_fermi_builder *b)
{
   return !mme_fermi_is_empty(b) && !(b->inst_parts & MME_FERMI_INSTR_PART_ASSIGN);
}

void mme_fermi_load_to(struct mme_builder *b,
                       struct mme_value dst)
{
   struct mme_fermi_builder *fb = &b->fermi;

   assert(dst.type == MME_VALUE_TYPE_REG ||
          dst.type == MME_VALUE_TYPE_ZERO);

   if (!fb->first_loaded) {
      struct mme_value r1 = {
         .type = MME_VALUE_TYPE_REG,
         .reg = 1,
      };
      mme_mov_to(b, dst, r1);
      mme_free_reg(b, r1);
      fb->first_loaded = true;
      return;
   }

   if (!mme_fermi_next_inst_can_load_to(fb))
      mme_fermi_new_inst(fb);

   struct mme_fermi_inst *inst = mme_fermi_cur_inst(fb);

   inst->assign_op = MME_FERMI_ASSIGN_OP_LOAD;
   inst->dst = mme_value_alu_reg(dst);

   mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_ASSIGN);
}


struct mme_value
mme_fermi_load(struct mme_builder *b)
{
   struct mme_fermi_builder *fb = &b->fermi;

   if (!fb->first_loaded) {
      struct mme_value r1 = {
         .type = MME_VALUE_TYPE_REG,
         .reg = 1,
      };
      fb->first_loaded = true;
      return r1;
   }

   struct mme_value dst = mme_alloc_reg(b);
   mme_fermi_load_to(b, dst);

   return dst;
}

static enum mme_fermi_alu_op
mme_to_fermi_alu_op(enum mme_alu_op op)
{
   switch (op) {
#define ALU_CASE(op) case MME_ALU_OP_##op: return MME_FERMI_ALU_OP_##op;
   ALU_CASE(ADD)
   ALU_CASE(ADDC)
   ALU_CASE(SUB)
   ALU_CASE(SUBB)
   ALU_CASE(AND)
   ALU_CASE(NAND)
   ALU_CASE(OR)
   ALU_CASE(XOR)
#undef ALU_CASE
   default:
      unreachable("Unsupported MME ALU op");
   }
}

void
mme_fermi_alu_to(struct mme_builder *b,
                 struct mme_value dst,
                 enum mme_alu_op op,
                 struct mme_value x,
                 struct mme_value y)
{
   struct mme_fermi_builder *fb = &b->fermi;

   switch (op) {
   case MME_ALU_OP_ADD:
      if (x.type == MME_VALUE_TYPE_IMM && x.imm != 0 && is_int18(x.imm)) {
         mme_fermi_add_imm18(fb, dst, y, x.imm);
         return;
      }
      if (y.type == MME_VALUE_TYPE_IMM && y.imm != 0 && is_int18(y.imm)) {
         mme_fermi_add_imm18(fb, dst, x, y.imm);
         return;
      }
      break;
   case MME_ALU_OP_SUB:
      if (y.type == MME_VALUE_TYPE_IMM && is_int18(-y.imm)) {
         mme_fermi_add_imm18(fb, dst, x, -y.imm);
         return;
      }
      break;
   case MME_ALU_OP_SLL:
      mme_fermi_sll_to(fb, dst, x, y);
      return;
   case MME_ALU_OP_SRL:
      mme_fermi_srl_to(fb, dst, x, y);
      return;
   default:
      break;
   }

   assert(mme_fermi_is_zero_or_reg(dst));

   struct mme_value x_reg = mme_fermi_value_as_reg(b, x);
   struct mme_value y_reg = mme_fermi_value_as_reg(b, y);

   if (!mme_fermi_next_inst_can_fit_a_full_inst(fb))
      mme_fermi_new_inst(fb);

   struct mme_fermi_inst *inst = mme_fermi_cur_inst(fb);
   inst->op = MME_FERMI_OP_ALU_REG;
   inst->alu_op = mme_to_fermi_alu_op(op);
   inst->src[0] = mme_value_alu_reg(x_reg);
   inst->src[1] = mme_value_alu_reg(y_reg);
   inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE;
   inst->dst = mme_value_alu_reg(dst);

   mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_OP |
                                MME_FERMI_INSTR_PART_ASSIGN);

   mme_free_reg_if_tmp(b, x, x_reg);
   mme_free_reg_if_tmp(b, y, y_reg);
}


void mme_fermi_state_arr_to(struct mme_builder *b,
                            struct mme_value dst,
                            uint16_t state,
                            struct mme_value index)
{
   struct mme_fermi_builder *fb = &b->fermi;

   assert(mme_fermi_is_zero_or_reg(dst));
   assert(state % 4 == 0);

   struct mme_value index_reg = mme_fermi_value_as_reg(b, index);

   if (!mme_fermi_next_inst_can_fit_a_full_inst(fb))
      mme_fermi_new_inst(fb);

   struct mme_fermi_inst *inst = mme_fermi_cur_inst(fb);
   inst->op = MME_FERMI_OP_STATE;
   inst->src[0] = mme_value_alu_reg(index_reg);
   inst->src[1] = MME_FERMI_REG_ZERO;
   inst->imm = state >> 2;
   inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE;
   inst->dst = mme_value_alu_reg(dst);

   mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_OP |
                                MME_FERMI_INSTR_PART_ASSIGN);

   mme_free_reg_if_tmp(b, index, index_reg);
}

void
mme_fermi_merge_to(struct mme_builder *b, struct mme_value dst,
                   struct mme_value x, struct mme_value y,
                   uint16_t dst_pos, uint16_t bits, uint16_t src_pos)
{
   struct mme_fermi_builder *fb = &b->fermi;

   assert(mme_fermi_is_zero_or_reg(dst));
   assert(dst_pos < 32);
   assert(bits < 32);
   assert(src_pos < 32);

   struct mme_value x_reg = mme_fermi_value_as_reg(b, x);
   struct mme_value y_reg = mme_fermi_value_as_reg(b, y);

   if (!mme_fermi_next_inst_can_fit_a_full_inst(fb))
      mme_fermi_new_inst(fb);

   struct mme_fermi_inst *inst = mme_fermi_cur_inst(fb);

   inst->op = MME_FERMI_OP_MERGE;
   inst->src[0] = mme_value_alu_reg(x_reg);
   inst->src[1] = mme_value_alu_reg(y_reg);
   inst->bitfield.dst_bit = dst_pos;
   inst->bitfield.src_bit = src_pos;
   inst->bitfield.size = bits;

   inst->assign_op = MME_FERMI_ASSIGN_OP_MOVE;
   inst->dst = mme_value_alu_reg(dst);

   mme_fermi_set_inst_parts(fb, MME_FERMI_INSTR_PART_OP |
                                MME_FERMI_INSTR_PART_ASSIGN);

   mme_free_reg_if_tmp(b, x, x_reg);
   mme_free_reg_if_tmp(b, y, y_reg);
}

uint32_t *
mme_fermi_builder_finish(struct mme_fermi_builder *b, size_t *size_out)
{
   assert(b->cf_depth == 0);

   /* TODO: If there are at least two instructions and we can guarantee the
    * last two instructions get exeucted (not in control-flow), we don't need
    * to add a pair of NOPs.
    */
   mme_fermi_new_inst(b);
   mme_fermi_new_inst(b);

   b->insts[b->inst_count - 2].end_next = true;

   size_t enc_size = b->inst_count * sizeof(uint32_t);
   uint32_t *enc = malloc(enc_size);
   if (enc != NULL) {
      mme_fermi_encode(enc, b->inst_count, b->insts);
      *size_out = enc_size;
   }
   return enc;
}

void
mme_fermi_builder_dump(struct mme_builder *b, FILE *fp)
{
   struct mme_fermi_builder *fb = &b->fermi;

   mme_fermi_print(fp, fb->insts, fb->inst_count);
}
