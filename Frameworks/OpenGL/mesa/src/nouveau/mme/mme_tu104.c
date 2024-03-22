/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include "mme_tu104.h"
#include "mme_tu104_encode.h"

#include "util/u_math.h"

#include <stdlib.h>

#include "nvk_clc597.h"

#define PRED_TO_STR(OP) [MME_TU104_PRED_##OP] = #OP
const char *pred_to_str[] = {
   PRED_TO_STR(UUUU),
   PRED_TO_STR(TTTT),
   PRED_TO_STR(FFFF),
   PRED_TO_STR(TTUU),
   PRED_TO_STR(FFUU),
   PRED_TO_STR(TFUU),
   PRED_TO_STR(TUUU),
   PRED_TO_STR(FUUU),
   PRED_TO_STR(UUTT),
   PRED_TO_STR(UUTF),
   PRED_TO_STR(UUTU),
   PRED_TO_STR(UUFT),
   PRED_TO_STR(UUFF),
   PRED_TO_STR(UUFU),
   PRED_TO_STR(UUUT),
   PRED_TO_STR(UUUF),
};
#undef PRED_TO_STR

const char *
mme_tu104_pred_to_str(enum mme_tu104_pred pred)
{
   assert(pred < ARRAY_SIZE(pred_to_str));
   return pred_to_str[pred];
}

#define OP_TO_STR(OP) [MME_TU104_ALU_OP_##OP] = #OP
static const char *alu_op_to_str[] = {
   OP_TO_STR(ADD),
   OP_TO_STR(ADDC),
   OP_TO_STR(SUB),
   OP_TO_STR(SUBB),
   OP_TO_STR(MUL),
   OP_TO_STR(MULH),
   OP_TO_STR(MULU),
   OP_TO_STR(EXTENDED),
   OP_TO_STR(CLZ),
   OP_TO_STR(SLL),
   OP_TO_STR(SRL),
   OP_TO_STR(SRA),
   OP_TO_STR(AND),
   OP_TO_STR(NAND),
   OP_TO_STR(OR),
   OP_TO_STR(XOR),
   OP_TO_STR(MERGE),
   OP_TO_STR(SLT),
   OP_TO_STR(SLTU),
   OP_TO_STR(SLE),
   OP_TO_STR(SLEU),
   OP_TO_STR(SEQ),
   OP_TO_STR(STATE),
   OP_TO_STR(LOOP),
   OP_TO_STR(JAL),
   OP_TO_STR(BLT),
   OP_TO_STR(BLTU),
   OP_TO_STR(BLE),
   OP_TO_STR(BLEU),
   OP_TO_STR(BEQ),
   OP_TO_STR(DREAD),
   OP_TO_STR(DWRITE),
};
#undef OP_TO_STR

const char *
mme_tu104_alu_op_to_str(enum mme_tu104_alu_op op)
{
   assert(op < ARRAY_SIZE(alu_op_to_str));
   return alu_op_to_str[op];
}

void
mme_tu104_encode(uint32_t *out, uint32_t inst_count,
                 const struct mme_tu104_inst *insts)
{
   for (uint32_t i = 0; i < inst_count; i++) {
      bitmask_t enc = encode__instruction(NULL, NULL, insts[i]);

      /* Annoyingly, the words are reversed in the actual encoding */
      out[i * 3 + 0] = enc.bitset[2];
      out[i * 3 + 1] = enc.bitset[1];
      out[i * 3 + 2] = enc.bitset[0];
   }
}

static uint64_t
unpack_field(bitmask_t bitmask, unsigned low, unsigned high, bool is_signed)
{
   bitmask_t field, mask;

   assert(high >= low);

   BITSET_ZERO(mask.bitset);
   BITSET_SET_RANGE(mask.bitset, 0, high - low);

   BITSET_COPY(field.bitset, bitmask.bitset);
   BITSET_SHR(field.bitset, low);
   BITSET_AND(field.bitset, field.bitset, mask.bitset);

   uint64_t data = bitmask_to_uint64_t(field);
   if (is_signed)
      data = util_sign_extend(data, high - low + 1);

   return data;
}

void
mme_tu104_decode(struct mme_tu104_inst *insts,
                 const uint32_t *in, uint32_t inst_count)
{
   for (uint32_t i = 0; i < inst_count; i++) {
      /* Annoyingly, the words are reversed in the actual encoding */
      bitmask_t enc;
      enc.bitset[0] = in[i * 3 + 2];
      enc.bitset[1] = in[i * 3 + 1];
      enc.bitset[2] = in[i * 3 + 0];

      insts[i].end_next       = unpack_field(enc,  0,  0, false);
      insts[i].pred_mode      = unpack_field(enc,  1,  4, false);
      insts[i].pred           = unpack_field(enc,  5,  9, false);

      insts[i].alu[0].op      = unpack_field(enc, 10, 14, false);
      insts[i].alu[0].dst     = unpack_field(enc, 15, 19, false);
      insts[i].alu[0].src[0]  = unpack_field(enc, 20, 24, false);
      insts[i].alu[0].src[1]  = unpack_field(enc, 25, 29, false);
      insts[i].imm[0]         = unpack_field(enc, 30, 45, false);

      insts[i].alu[1].op      = unpack_field(enc, 46, 50, false);
      insts[i].alu[1].dst     = unpack_field(enc, 51, 55, false);
      insts[i].alu[1].src[0]  = unpack_field(enc, 56, 60, false);
      insts[i].alu[1].src[1]  = unpack_field(enc, 61, 65, false);
      insts[i].imm[1]         = unpack_field(enc, 66, 81, false);

      insts[i].out[0].mthd    = unpack_field(enc, 82, 84, false);
      insts[i].out[0].emit    = unpack_field(enc, 85, 88, false);

      insts[i].out[1].mthd    = unpack_field(enc, 89, 91, false);
      insts[i].out[1].emit    = unpack_field(enc, 92, 95, false);
   }
}

static void
print_indent(FILE *fp, unsigned depth)
{
   for (unsigned i = 0; i < depth; i++)
      fprintf(fp, "    ");
}

static bool
mme_tu104_alu_src_is_imm(const struct mme_tu104_inst *inst,
                         unsigned alu_idx, unsigned src_idx,
                         uint32_t *imm)
{
   const enum mme_tu104_reg reg = inst->alu[alu_idx].src[src_idx];

   switch (reg) {
   case MME_TU104_REG_ZERO:
      *imm = 0;
      return true;
   case MME_TU104_REG_IMM:
      *imm = (int32_t)(int16_t)inst->imm[alu_idx];
      return true;
   case MME_TU104_REG_IMMPAIR:
      *imm = (int32_t)(int16_t)inst->imm[1 - alu_idx];
      return true;
   case MME_TU104_REG_IMM32:
      *imm = ((uint32_t)inst->imm[0] << 16) | inst->imm[1];
      return true;
   default:
      return false;
   }
}

static void
mme_tu104_print_alu_src(FILE *fp, const struct mme_tu104_inst *inst,
                        unsigned alu_idx, unsigned src_idx)
{
   const enum mme_tu104_reg reg = inst->alu[alu_idx].src[src_idx];
   if (reg <= MME_TU104_REG_R23) {
      fprintf(fp, " $r%u", (unsigned)reg);
   } else {
      switch (reg) {
      case MME_TU104_REG_ZERO:
         fprintf(fp, " $zero");
         break;
      case MME_TU104_REG_IMM:
         fprintf(fp, " %d /* 0x%04x */", (int)(int16_t)inst->imm[alu_idx],
                 (unsigned)inst->imm[alu_idx]);
         break;
      case MME_TU104_REG_IMMPAIR:
         fprintf(fp, " %d /* 0x%04x */", (int)(int16_t)inst->imm[1 - alu_idx],
                 (unsigned)inst->imm[1 - alu_idx]);
         break;
      case MME_TU104_REG_IMM32:
         fprintf(fp, " 0x%x", ((uint32_t)inst->imm[0] << 16) | inst->imm[1]);
         break;
      case MME_TU104_REG_LOAD0:
         fprintf(fp, " $load0");
         break;
      case MME_TU104_REG_LOAD1:
         fprintf(fp, " $load1");
         break;
      default:
         unreachable("Invalid ALU source register");
      }
   }
}

bool
mme_tu104_alu_op_has_implicit_imm(enum mme_tu104_alu_op op)
{
   switch (op) {
   case MME_TU104_ALU_OP_MERGE:
   case MME_TU104_ALU_OP_LOOP:
   case MME_TU104_ALU_OP_JAL:
   case MME_TU104_ALU_OP_BLT:
   case MME_TU104_ALU_OP_BLTU:
   case MME_TU104_ALU_OP_BLE:
   case MME_TU104_ALU_OP_BLEU:
   case MME_TU104_ALU_OP_BEQ:
      return true;
   default:
      return false;
   }
}

bool
mme_tu104_alu_op_has_side_effects(enum mme_tu104_alu_op op)
{
   return mme_tu104_alu_op_is_control_flow(op) ||
          op == MME_TU104_ALU_OP_EXTENDED ||
          op == MME_TU104_ALU_OP_DWRITE;
}

bool
mme_tu104_alu_op_is_control_flow(enum mme_tu104_alu_op op)
{
   switch (op) {
   case MME_TU104_ALU_OP_LOOP:
   case MME_TU104_ALU_OP_JAL:
   case MME_TU104_ALU_OP_BLT:
   case MME_TU104_ALU_OP_BLTU:
   case MME_TU104_ALU_OP_BLE:
   case MME_TU104_ALU_OP_BLEU:
   case MME_TU104_ALU_OP_BEQ:
      return true;
   default:
      return false;
   }
}

bool
mme_tu104_alu_op_may_depend_on_mthd(enum mme_tu104_alu_op op)
{
   switch (op) {
   case MME_TU104_ALU_OP_EXTENDED:
   case MME_TU104_ALU_OP_STATE:
      return true;
   default:
      return false;
   }
}

bool
mme_tu104_alus_have_dependency(const struct mme_tu104_alu *first,
                               const struct mme_tu104_alu *second)
{
   if (first->dst != MME_TU104_REG_ZERO &&
       (first->dst == second->dst ||
        first->dst == second->src[0] ||
        first->dst == second->src[1]))
      return true;

   /* TODO: This could be more detailed */
   if (first->op == MME_TU104_ALU_OP_DWRITE &&
       (second->op == MME_TU104_ALU_OP_DREAD ||
        second->op == MME_TU104_ALU_OP_DWRITE))
      return true;

   /* TODO: This could be more detailed */
   if (second->op == MME_TU104_ALU_OP_DWRITE &&
       (first->op == MME_TU104_ALU_OP_DREAD ||
        first->op == MME_TU104_ALU_OP_DWRITE))
      return true;

   /* EXTENDED acts like a barrier between MME_DMA_READ_FIFOED and LOAD0/1 */
   if (first->op == MME_TU104_ALU_OP_EXTENDED &&
       (second->src[0] == MME_TU104_REG_LOAD0 ||
        second->src[0] == MME_TU104_REG_LOAD1 ||
        second->src[1] == MME_TU104_REG_LOAD0 ||
        second->src[1] == MME_TU104_REG_LOAD1))
      return true;

   return false;
}

static bool
mme_tu104_alu_is_branch(const struct mme_tu104_inst *inst, unsigned alu_idx,
                        int *then_offset, unsigned *else_offset)
{
   switch (inst->alu[alu_idx].op) {
   case MME_TU104_ALU_OP_BLT:
   case MME_TU104_ALU_OP_BLTU:
   case MME_TU104_ALU_OP_BLE:
   case MME_TU104_ALU_OP_BLEU:
   case MME_TU104_ALU_OP_BEQ:
      *then_offset = util_mask_sign_extend(inst->imm[alu_idx], 14);
      *else_offset = (inst->imm[alu_idx] >> 14) & 0x3;
      return true;
   default:
      return false;
   }
}

static void
mme_tu104_print_alu(FILE *fp, unsigned indent,
                    const struct mme_tu104_inst *inst,
                    unsigned alu_idx)
{
   const struct mme_tu104_alu *alu = &inst->alu[alu_idx];

   const bool used_by_out =
      inst->out[0].mthd == MME_TU104_OUT_OP_ALU0 + alu_idx ||
      inst->out[0].emit == MME_TU104_OUT_OP_ALU0 + alu_idx ||
      inst->out[1].mthd == MME_TU104_OUT_OP_ALU0 + alu_idx ||
      inst->out[1].emit == MME_TU104_OUT_OP_ALU0 + alu_idx;

   if (!used_by_out && alu->dst == MME_TU104_REG_ZERO &&
       !mme_tu104_alu_op_has_side_effects(alu->op))
      return;

   print_indent(fp, indent);

   if (used_by_out || alu->dst != MME_TU104_REG_ZERO) {
      if (used_by_out)
         fprintf(fp, "$alu%u", alu_idx);
      if (alu->dst <= MME_TU104_REG_R23) {
         fprintf(fp, "%s$r%u", used_by_out ? ", " : "", (unsigned)alu->dst);
      } else {
         assert(alu->dst == MME_TU104_REG_ZERO);
      }
      fprintf(fp, " = ");
   }

   switch (alu->op) {
   case MME_TU104_ALU_OP_ADDC:
      assert(alu_idx == 1);
      fprintf(fp, "ADDC");
      mme_tu104_print_alu_src(fp, inst, alu_idx, 0);
      mme_tu104_print_alu_src(fp, inst, alu_idx, 1);
      fprintf(fp, " $carry");
      break;
   case MME_TU104_ALU_OP_SUBB:
      assert(alu_idx == 1);
      fprintf(fp, "SUBB");
      mme_tu104_print_alu_src(fp, inst, alu_idx, 0);
      mme_tu104_print_alu_src(fp, inst, alu_idx, 1);
      fprintf(fp, " $borrow");
      break;
   case MME_TU104_ALU_OP_MULH:
      assert(alu_idx == 1);
      assert(alu->src[0] == MME_TU104_REG_ZERO);
      assert(alu->src[1] == MME_TU104_REG_ZERO);
      fprintf(fp, "MULH $alu0");
      break;
   case MME_TU104_ALU_OP_MERGE: {
      uint16_t immed = inst->imm[alu_idx];
      uint32_t src_pos  = (immed >> 0)  & 0x1f;
      uint32_t bits     = (immed >> 5)  & 0x1f;
      uint32_t dst_pos  = (immed >> 10) & 0x3f;
      fprintf(fp, "MERGE");
      mme_tu104_print_alu_src(fp, inst, alu_idx, 0);
      mme_tu104_print_alu_src(fp, inst, alu_idx, 1);
      fprintf(fp, " (%u, %u, %u)", src_pos, bits, dst_pos);
      break;
   }
   case MME_TU104_ALU_OP_STATE: {
      fprintf(fp, "STATE");
      mme_tu104_print_alu_src(fp, inst, alu_idx, 0);
      if (alu->src[1] != MME_TU104_REG_ZERO) {
         fprintf(fp, " +");
         mme_tu104_print_alu_src(fp, inst, alu_idx, 1);
      }
      uint32_t imm;
      if (mme_tu104_alu_src_is_imm(inst, alu_idx, 0, &imm)) {
         uint32_t mthd = imm << 2;
         fprintf(fp, " /* %s", P_PARSE_NVC597_MTHD(mthd));
         if (alu->src[1] != MME_TU104_REG_ZERO) {
            fprintf(fp, " +");
            mme_tu104_print_alu_src(fp, inst, alu_idx, 1);
         }
         fprintf(fp, " */");
      }
      break;
   }
   case MME_TU104_ALU_OP_JAL:
      assert(alu->src[0] == MME_TU104_REG_ZERO);
      assert(alu->src[1] == MME_TU104_REG_ZERO);
      fprintf(fp, "JAL (0x%04x)", (unsigned)inst->imm[alu_idx]);
      break;
   case MME_TU104_ALU_OP_LOOP:
   case MME_TU104_ALU_OP_DREAD:
      fprintf(fp, "%s", mme_tu104_alu_op_to_str(alu->op));
      mme_tu104_print_alu_src(fp, inst, alu_idx, 0);
      assert(alu->src[1] == MME_TU104_REG_ZERO);
      break;
   default:
      fprintf(fp, "%s", mme_tu104_alu_op_to_str(alu->op));
      mme_tu104_print_alu_src(fp, inst, alu_idx, 0);
      mme_tu104_print_alu_src(fp, inst, alu_idx, 1);
      break;
   }

   int then_offset;
   unsigned else_offset;
   if (mme_tu104_alu_is_branch(inst, alu_idx, &then_offset, &else_offset))
      fprintf(fp, " (%d, %u)", then_offset, else_offset);

   if (alu->op == MME_TU104_ALU_OP_LOOP)
      fprintf(fp, " (%d)", (unsigned)inst->imm[alu_idx]);

   fprintf(fp, "\n");
}

static bool
mme_tu104_out_is_imm(const struct mme_tu104_inst *inst,
                     enum mme_tu104_out_op op, uint32_t *imm)
{
   switch (op) {
   case MME_TU104_OUT_OP_IMM0:
   case MME_TU104_OUT_OP_IMM1:
      *imm = inst->imm[op - MME_TU104_OUT_OP_IMM0];
      return true;
   case MME_TU104_OUT_OP_IMMHIGH0:
   case MME_TU104_OUT_OP_IMMHIGH1:
      *imm = inst->imm[op - MME_TU104_OUT_OP_IMMHIGH0] >> 12;
      return true;
   case MME_TU104_OUT_OP_IMM32:
      *imm = ((uint32_t)inst->imm[0] << 16) | inst->imm[1];
      return true;
   default:
      return false;
   }
}

static void
mme_tu104_print_out_src(FILE *fp, const struct mme_tu104_inst *inst,
                        enum mme_tu104_out_op op)
{
   switch (op) {
   case MME_TU104_OUT_OP_ALU0:
   case MME_TU104_OUT_OP_ALU1:
      fprintf(fp, "$alu%u", (int)op - MME_TU104_OUT_OP_ALU0);
      break;
   case MME_TU104_OUT_OP_LOAD0:
   case MME_TU104_OUT_OP_LOAD1:
      fprintf(fp, "$load%u", (int)op - MME_TU104_OUT_OP_LOAD0);
      break;
   case MME_TU104_OUT_OP_IMM0:
   case MME_TU104_OUT_OP_IMM1:
      fprintf(fp, "0x%x", (unsigned)inst->imm[op - MME_TU104_OUT_OP_IMM0]);
      break;
   case MME_TU104_OUT_OP_RESERVED:
      fprintf(fp, "RESERVED");
      break;
   case MME_TU104_OUT_OP_IMMHIGH0:
   case MME_TU104_OUT_OP_IMMHIGH1:
      fprintf(fp, "%u",
              (unsigned)(inst->imm[op - MME_TU104_OUT_OP_IMMHIGH0] >> 12));
      break;
   case MME_TU104_OUT_OP_IMM32:
      fprintf(fp, "0x%x", ((uint32_t)inst->imm[0] << 16) | inst->imm[1]);
      break;
   default:
      unreachable("Invalid output source");
   }
};

static void
mme_tu104_print_out(FILE *fp, unsigned indent,
                    const struct mme_tu104_inst *inst,
                    unsigned out_idx)
{
   const struct mme_tu104_out *out = &inst->out[out_idx];

   if (out->mthd != MME_TU104_OUT_OP_NONE) {
      print_indent(fp, indent);
      fprintf(fp, "mthd(");
      uint32_t imm;
      if (mme_tu104_out_is_imm(inst, out->mthd, &imm)) {
         uint32_t mthd = (imm & 0xfff) << 2;
         uint32_t incr = imm >> 12;
         fprintf(fp, "0x%04x, %u)", mthd, incr);
         fprintf(fp, " /* %s */", P_PARSE_NVC597_MTHD(mthd));
      } else {
         mme_tu104_print_out_src(fp, inst, out->mthd);
         fprintf(fp, ")");
      }
      fprintf(fp, "\n");
   }
   if (out->emit != MME_TU104_OUT_OP_NONE) {
      print_indent(fp, indent);
      fprintf(fp, "emit(");
      mme_tu104_print_out_src(fp, inst, out->emit);
      fprintf(fp, ")\n");
   }
}

void
mme_tu104_print_inst(FILE *fp, unsigned indent,
                     const struct mme_tu104_inst *inst)
{
   if (inst->pred_mode != MME_TU104_PRED_UUUU) {
      print_indent(fp, indent);
      fprintf(fp, "pred %s", mme_tu104_pred_to_str(inst->pred_mode));
      fprintf(fp, " $r%u {\n", (unsigned)inst->pred);
      indent++;
   }

   mme_tu104_print_alu(fp, indent, inst, 0);
   mme_tu104_print_alu(fp, indent, inst, 1);
   mme_tu104_print_out(fp, indent, inst, 0);
   mme_tu104_print_out(fp, indent, inst, 1);

   if (inst->pred_mode != MME_TU104_PRED_UUUU) {
      indent--;
      print_indent(fp, indent);
      fprintf(fp, "}\n");
   }
}

void
mme_tu104_print(FILE *fp, const struct mme_tu104_inst *insts,
                uint32_t inst_count)
{
   for (uint32_t i = 0; i < inst_count; i++) {
      fprintf(fp, "%u:\n", i);
      mme_tu104_print_inst(fp, 1, &insts[i]);
   }
}
