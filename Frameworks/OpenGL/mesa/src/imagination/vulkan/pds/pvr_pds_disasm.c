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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pvr_rogue_pds_defs.h"
#include "pvr_rogue_pds_encode.h"
#include "pvr_rogue_pds_disasm.h"
#include "util/macros.h"

static void pvr_error_check(PVR_ERR_CALLBACK err_callback,
                            struct pvr_dissassembler_error error)
{
   if (err_callback)
      err_callback(error);
   else
      fprintf(stderr, "ERROR: %s\n", error.text);
}

#define X(a) #a,
static const char *const instructions[] = { PVR_INSTRUCTIONS };
#undef X

static void error_reg_range(uint32_t raw,
                            void *context,
                            PVR_ERR_CALLBACK err_callback,
                            uint32_t parameter,
                            struct pvr_dissassembler_error error)
{
   char param[32];

   error.type = PVR_PDS_ERR_PARAM_RANGE;
   error.parameter = parameter;
   error.raw = raw;

   if (parameter == 0)
      snprintf(param, sizeof(param), "dst");
   else
      snprintf(param, sizeof(param), "src%u", parameter - 1);

   error.text = malloc(PVR_PDS_MAX_INST_STR_LEN);
   assert(error.text);

   snprintf(error.text,
            PVR_PDS_MAX_INST_STR_LEN,
            "Register out of range, instruction: %s, operand: %s, value: %u",
            instructions[error.instruction],
            param,
            raw);
   pvr_error_check(err_callback, error);
}

static struct pvr_operand *
pvr_pds_disassemble_regs32(void *context,
                           PVR_ERR_CALLBACK err_callback,
                           struct pvr_dissassembler_error error,
                           uint32_t instruction,
                           uint32_t parameter)
{
   struct pvr_operand *op = calloc(1, sizeof(*op));
   assert(op);

   op->type = UNRESOLVED;
   instruction &= PVR_ROGUE_PDSINST_REGS32_MASK;
   switch (pvr_pds_inst_decode_field_range_regs32(instruction)) {
   case PVR_ROGUE_PDSINST_REGS32_CONST32:
      op->type = CONST32;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS32_CONST32_LOWER;
      op->absolute_address = op->address;
      break;
   case PVR_ROGUE_PDSINST_REGS32_TEMP32:
      op->type = TEMP32;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS32_TEMP32_LOWER;
      op->absolute_address = op->address;
      break;
   case PVR_ROGUE_PDSINST_REGS32_PTEMP32:
      op->type = PTEMP32;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS32_PTEMP32_LOWER;
      op->absolute_address = op->address;
      break;
   default:
      error_reg_range(instruction, context, err_callback, parameter, error);
   }
   return op;
}
static struct pvr_operand *
pvr_pds_disassemble_regs32tp(void *context,
                             PVR_ERR_CALLBACK err_callback,
                             struct pvr_dissassembler_error error,
                             uint32_t instruction,
                             uint32_t parameter)
{
   struct pvr_operand *op = calloc(1, sizeof(*op));
   assert(op);

   op->type = UNRESOLVED;
   instruction &= PVR_ROGUE_PDSINST_REGS32TP_MASK;
   switch (pvr_pds_inst_decode_field_range_regs32tp(instruction)) {
   case PVR_ROGUE_PDSINST_REGS32TP_TEMP32:
      op->type = TEMP32;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS32TP_TEMP32_LOWER;
      op->absolute_address = op->address;
      break;
   case PVR_ROGUE_PDSINST_REGS32TP_PTEMP32:
      op->type = PTEMP32;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS32TP_PTEMP32_LOWER;
      op->absolute_address = op->address;
      break;
   default:
      error_reg_range(instruction, context, err_callback, parameter, error);
   }
   return op;
}
static struct pvr_operand *
pvr_pds_disassemble_regs32t(void *context,
                            PVR_ERR_CALLBACK err_callback,
                            struct pvr_dissassembler_error error,
                            uint32_t instruction,
                            uint32_t parameter)
{
   struct pvr_operand *op = calloc(1, sizeof(*op));
   assert(op);

   op->type = UNRESOLVED;
   instruction &= PVR_ROGUE_PDSINST_REGS32T_MASK;
   switch (pvr_pds_inst_decode_field_range_regs32t(instruction)) {
   case PVR_ROGUE_PDSINST_REGS32T_TEMP32:
      op->type = TEMP32;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS32T_TEMP32_LOWER;
      op->absolute_address = op->address;
      break;
   default:
      error_reg_range(instruction, context, err_callback, parameter, error);
   }
   return op;
}

static struct pvr_operand *
pvr_pds_disassemble_regs64(void *context,
                           PVR_ERR_CALLBACK err_callback,
                           struct pvr_dissassembler_error error,
                           uint32_t instruction,
                           uint32_t parameter)
{
   struct pvr_operand *op = calloc(1, sizeof(*op));
   assert(op);

   op->type = UNRESOLVED;
   instruction &= PVR_ROGUE_PDSINST_REGS64_MASK;
   switch (pvr_pds_inst_decode_field_range_regs64(instruction)) {
   case PVR_ROGUE_PDSINST_REGS64_CONST64:
      op->type = CONST64;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER;
      op->absolute_address = op->address * 2;
      break;
   case PVR_ROGUE_PDSINST_REGS64_TEMP64:
      op->type = TEMP64;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS64_TEMP64_LOWER;
      op->absolute_address = op->address * 2;
      break;
   case PVR_ROGUE_PDSINST_REGS64_PTEMP64:
      op->type = PTEMP64;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS64_PTEMP64_LOWER;
      op->absolute_address = op->address * 2;
      break;
   default:
      error_reg_range(instruction, context, err_callback, parameter, error);
   }

   return op;
}
static struct pvr_operand *
pvr_pds_disassemble_regs64t(void *context,
                            PVR_ERR_CALLBACK err_callback,
                            struct pvr_dissassembler_error error,
                            uint32_t instruction,
                            uint32_t parameter)
{
   struct pvr_operand *op = calloc(1, sizeof(*op));
   assert(op);

   op->type = UNRESOLVED;
   instruction &= PVR_ROGUE_PDSINST_REGS64T_MASK;
   switch (pvr_pds_inst_decode_field_range_regs64tp(instruction)) {
   case PVR_ROGUE_PDSINST_REGS64T_TEMP64:
      op->type = TEMP64;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS64T_TEMP64_LOWER;
      op->absolute_address = op->address * 2;
      break;
   default:
      error_reg_range(instruction, context, err_callback, parameter, error);
   }
   return op;
}

static struct pvr_operand *
pvr_pds_disassemble_regs64C(void *context,
                            PVR_ERR_CALLBACK err_callback,
                            struct pvr_dissassembler_error error,
                            uint32_t instruction,
                            uint32_t parameter)
{
   struct pvr_operand *op = calloc(1, sizeof(*op));
   assert(op);

   op->type = UNRESOLVED;
   instruction &= PVR_ROGUE_PDSINST_REGS64C_MASK;
   switch (pvr_rogue_pds_inst_decode_field_range_regs64c(instruction)) {
   case PVR_ROGUE_PDSINST_REGS64C_CONST64:
      op->type = CONST64;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS64C_CONST64_LOWER;
      op->absolute_address = op->address * 2;
      break;
   default:
      error_reg_range(instruction, context, err_callback, parameter, error);
   }
   return op;
}

static struct pvr_operand *
pvr_pds_disassemble_regs64tp(void *context,
                             PVR_ERR_CALLBACK err_callback,
                             struct pvr_dissassembler_error error,
                             uint32_t instruction,
                             uint32_t parameter)
{
   struct pvr_operand *op = calloc(1, sizeof(*op));
   assert(op);

   op->type = UNRESOLVED;
   instruction &= PVR_ROGUE_PDSINST_REGS64TP_MASK;
   switch (pvr_pds_inst_decode_field_range_regs64tp(instruction)) {
   case PVR_ROGUE_PDSINST_REGS64TP_TEMP64:
      op->type = TEMP64;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS64TP_TEMP64_LOWER;
      op->absolute_address = op->address * 2;
      break;
   case PVR_ROGUE_PDSINST_REGS64TP_PTEMP64:
      op->type = PTEMP64;
      op->address = instruction - PVR_ROGUE_PDSINST_REGS64TP_PTEMP64_LOWER;
      op->absolute_address = op->address * 2;
      break;
   default:
      error_reg_range(instruction, context, err_callback, parameter, error);
   }
   return op;
}

#define PVR_TYPE_OPCODE BITFIELD_BIT(31U)
#define PVR_TYPE_OPCODE_SP BITFIELD_BIT(27U)
#define PVR_TYPE_OPCODEB BITFIELD_BIT(30U)

#define PVR_TYPE_OPCODE_SHIFT 28U
#define PVR_TYPE_OPCODE_SP_SHIFT 23U
#define PVR_TYPE_OPCODEB_SHIFT 29U

static struct pvr_instruction *
pvr_pds_disassemble_instruction_add64(void *context,
                                      PVR_ERR_CALLBACK err_callback,
                                      struct pvr_dissassembler_error error,
                                      uint32_t instruction)
{
   struct pvr_add *add = malloc(sizeof(*add));
   assert(add);

   add->instruction.type = INS_ADD64;
   add->instruction.next = NULL;

   add->cc = instruction & PVR_ROGUE_PDSINST_ADD64_CC_ENABLE;
   add->alum = instruction & PVR_ROGUE_PDSINST_ADD64_ALUM_SIGNED;
   add->sna = instruction & PVR_ROGUE_PDSINST_ADD64_SNA_SUB;

   add->src0 = pvr_pds_disassemble_regs64(context,
                                          err_callback,
                                          error,
                                          instruction >>
                                             PVR_ROGUE_PDSINST_ADD64_SRC0_SHIFT,
                                          1);
   add->src0->instruction = &add->instruction;
   add->src1 = pvr_pds_disassemble_regs64(context,
                                          err_callback,
                                          error,
                                          instruction >>
                                             PVR_ROGUE_PDSINST_ADD64_SRC1_SHIFT,
                                          2);
   add->src1->instruction = &add->instruction;
   add->dst = pvr_pds_disassemble_regs64tp(context,
                                           err_callback,
                                           error,
                                           instruction >>
                                              PVR_ROGUE_PDSINST_ADD64_DST_SHIFT,
                                           0);
   add->dst->instruction = &add->instruction;

   return &add->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_add32(void *context,
                                      PVR_ERR_CALLBACK err_callback,
                                      struct pvr_dissassembler_error error,
                                      uint32_t instruction)
{
   struct pvr_add *add = malloc(sizeof(*add));
   assert(add);

   add->instruction.type = INS_ADD32;
   add->instruction.next = NULL;

   add->cc = instruction & PVR_ROGUE_PDSINST_ADD32_CC_ENABLE;
   add->alum = instruction & PVR_ROGUE_PDSINST_ADD32_ALUM_SIGNED;
   add->sna = instruction & PVR_ROGUE_PDSINST_ADD32_SNA_SUB;

   add->src0 = pvr_pds_disassemble_regs32(context,
                                          err_callback,
                                          error,
                                          instruction >>
                                             PVR_ROGUE_PDSINST_ADD32_SRC0_SHIFT,
                                          1);
   add->src0->instruction = &add->instruction;
   add->src1 = pvr_pds_disassemble_regs32(context,
                                          err_callback,
                                          error,
                                          instruction >>
                                             PVR_ROGUE_PDSINST_ADD32_SRC1_SHIFT,
                                          2);
   add->src1->instruction = &add->instruction;
   add->dst = pvr_pds_disassemble_regs32tp(context,
                                           err_callback,
                                           error,
                                           instruction >>
                                              PVR_ROGUE_PDSINST_ADD32_DST_SHIFT,
                                           0);
   add->dst->instruction = &add->instruction;

   return &add->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_stm(void *context,
                                    PVR_ERR_CALLBACK err_callback,
                                    struct pvr_dissassembler_error error,
                                    uint32_t instruction)
{
   struct pvr_stm *stm = malloc(sizeof(*stm));
   assert(stm);

   stm->instruction.next = NULL;
   stm->instruction.type = INS_STM;

   stm->cc = instruction & (1 << PVR_ROGUE_PDSINST_STM_CCS_CCS_CC_SHIFT);
   stm->ccs_global = instruction &
                     (1 << PVR_ROGUE_PDSINST_STM_CCS_CCS_GLOBAL_SHIFT);
   stm->ccs_so = instruction & (1 << PVR_ROGUE_PDSINST_STM_CCS_CCS_SO_SHIFT);
   stm->tst = instruction & (1 << PVR_ROGUE_PDSINST_STM_SO_TST_SHIFT);

   stm->stream_out = (instruction >> PVR_ROGUE_PDSINST_STM_SO_SHIFT) &
                     PVR_ROGUE_PDSINST_SO_MASK;

   stm->src0 = pvr_pds_disassemble_regs64tp(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_STM_SO_SRC0_SHIFT,
      1);
   stm->src0->instruction = &stm->instruction;

   stm->src1 = pvr_pds_disassemble_regs64tp(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_STM_SO_SRC1_SHIFT,
      2);
   stm->src1->instruction = &stm->instruction;

   stm->src2 = pvr_pds_disassemble_regs32(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_STM_SO_SRC2_SHIFT,
      3);
   stm->src2->instruction = &stm->instruction;

   stm->src3 = pvr_pds_disassemble_regs64tp(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_STM_SO_SRC3_SHIFT,
      4);
   stm->src3->instruction = &stm->instruction;

   return &stm->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_sftlp32(void *context,
                                        PVR_ERR_CALLBACK err_callback,
                                        struct pvr_dissassembler_error error,
                                        uint32_t instruction)
{
   struct pvr_sftlp *ins = malloc(sizeof(*ins));
   assert(ins);

   ins->instruction.next = NULL;
   ins->instruction.type = INS_SFTLP32;

   ins->cc = instruction & PVR_ROGUE_PDSINST_SFTLP32_CC_ENABLE;
   ins->IM = instruction & PVR_ROGUE_PDSINST_SFTLP32_IM_ENABLE;
   ins->lop = (instruction >> PVR_ROGUE_PDSINST_SFTLP32_LOP_SHIFT) &
              PVR_ROGUE_PDSINST_LOP_MASK;
   ins->src0 = pvr_pds_disassemble_regs32t(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_SFTLP32_SRC0_SHIFT,
      1);
   ins->src0->instruction = &ins->instruction;
   ins->src1 = pvr_pds_disassemble_regs32(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_SFTLP32_SRC1_SHIFT,
      2);
   ins->src1->instruction = &ins->instruction;
   ins->dst = pvr_pds_disassemble_regs32t(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_SFTLP32_DST_SHIFT,
      0);
   ins->dst->instruction = &ins->instruction;

   if (ins->IM) {
      signed char cImmediate =
         ((instruction >> PVR_ROGUE_PDSINST_SFTLP32_SRC2_SHIFT) &
          PVR_ROGUE_PDSINST_REGS32_MASK)
         << 2;
      ins->src2 = calloc(1, sizeof(*ins->src2));
      assert(ins->src2);

      ins->src2->literal = abs((cImmediate / 4));
      ins->src2->negate = cImmediate < 0;
      ins->src2->instruction = &ins->instruction;
   } else {
      ins->src2 = pvr_pds_disassemble_regs32tp(
         context,
         err_callback,
         error,
         (instruction >> PVR_ROGUE_PDSINST_SFTLP32_SRC2_SHIFT),
         3);
      ins->src2->instruction = &ins->instruction;
   }

   return &ins->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_sftlp64(void *context,
                                        PVR_ERR_CALLBACK err_callback,
                                        struct pvr_dissassembler_error error,
                                        uint32_t instruction)
{
   struct pvr_sftlp *ins = malloc(sizeof(*ins));
   assert(ins);

   ins->instruction.next = NULL;
   ins->instruction.type = INS_SFTLP64;

   ins->cc = instruction & PVR_ROGUE_PDSINST_SFTLP64_CC_ENABLE;
   ins->IM = instruction & PVR_ROGUE_PDSINST_SFTLP64_IM_ENABLE;
   ins->lop = (instruction >> PVR_ROGUE_PDSINST_SFTLP64_LOP_SHIFT) &
              PVR_ROGUE_PDSINST_LOP_MASK;
   ins->src0 = pvr_pds_disassemble_regs64tp(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_SFTLP64_SRC0_SHIFT,
      1);
   ins->src0->instruction = &ins->instruction;
   ins->src1 = pvr_pds_disassemble_regs64tp(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_SFTLP64_SRC1_SHIFT,
      2);
   ins->src1->instruction = &ins->instruction;
   ins->dst = pvr_pds_disassemble_regs64tp(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_SFTLP64_DST_SHIFT,
      0);
   ins->dst->instruction = &ins->instruction;

   if (ins->IM) {
      signed char cImmediate =
         (instruction >> PVR_ROGUE_PDSINST_SFTLP64_SRC2_SHIFT) &
         PVR_ROGUE_PDSINST_REGS32_MASK;
      ins->src2 = calloc(1, sizeof(*ins->src2));
      assert(ins->src2);

      ins->src2->literal = (abs(cImmediate) > 63) ? 63 : abs(cImmediate);
      ins->src2->negate = (cImmediate < 0);
      ins->src2->instruction = &ins->instruction;
   } else {
      ins->src2 = pvr_pds_disassemble_regs32(
         context,
         err_callback,
         error,
         (instruction >> PVR_ROGUE_PDSINST_SFTLP64_SRC2_SHIFT),
         3);
      ins->src2->instruction = &ins->instruction;
   }

   return &ins->instruction;
}
static struct pvr_instruction *
pvr_pds_disassemble_instruction_cmp(void *context,
                                    PVR_ERR_CALLBACK err_callback,
                                    struct pvr_dissassembler_error error,
                                    uint32_t instruction)
{
   struct pvr_cmp *cmp = malloc(sizeof(*cmp));
   assert(cmp);

   cmp->instruction.next = NULL;
   cmp->instruction.type = INS_CMP;
   cmp->cc = instruction & PVR_ROGUE_PDSINST_CMP_CC_ENABLE;
   cmp->IM = instruction & PVR_ROGUE_PDSINST_CMP_IM_ENABLE;
   cmp->cop = instruction >> PVR_ROGUE_PDSINST_CMP_COP_SHIFT &
              PVR_ROGUE_PDSINST_COP_MASK;
   cmp->src0 = pvr_pds_disassemble_regs64tp(context,
                                            err_callback,
                                            error,
                                            instruction >>
                                               PVR_ROGUE_PDSINST_CMP_SRC0_SHIFT,
                                            1);
   cmp->src0->instruction = &cmp->instruction;

   if (cmp->IM) {
      uint32_t immediate = (instruction >> PVR_ROGUE_PDSINST_CMP_SRC1_SHIFT) &
                           PVR_ROGUE_PDSINST_IMM16_MASK;
      cmp->src1 = calloc(1, sizeof(*cmp->src1));
      assert(cmp->src1);

      cmp->src1->type = LITERAL_NUM;
      cmp->src1->literal = immediate;
   } else {
      cmp->src1 = pvr_pds_disassemble_regs64(
         context,
         err_callback,
         error,
         instruction >> PVR_ROGUE_PDSINST_CMP_SRC1_SHIFT,
         2);
   }
   cmp->src1->instruction = &cmp->instruction;

   return &cmp->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_sp_ld_st(void *context,
                                         PVR_ERR_CALLBACK err_callback,
                                         struct pvr_dissassembler_error error,
                                         bool ld,
                                         uint32_t instruction,
                                         bool cc)
{
   struct pvr_ldst *ins = malloc(sizeof(*ins));
   assert(ins);

   ins->instruction.next = NULL;
   ins->instruction.type = ld ? INS_LD : INS_ST;

   ins->cc = cc;
   ins->src0 =
      pvr_pds_disassemble_regs64(context,
                                 err_callback,
                                 error,
                                 instruction >> PVR_ROGUE_PDSINST_LD_SRC0_SHIFT,
                                 1);
   ins->src0->instruction = &ins->instruction;
   ins->st = !ld;

   return &ins->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_sp_stmc(uint32_t instruction, bool cc)
{
   struct pvr_stmc *stmc = malloc(sizeof(*stmc));
   assert(stmc);

   stmc->instruction.next = NULL;
   stmc->instruction.type = INS_STMC;

   stmc->cc = cc;
   stmc->src0 = calloc(1, sizeof(*stmc->src0));
   assert(stmc->src0);

   stmc->src0->type = LITERAL_NUM;
   stmc->src0->literal = (instruction >> PVR_ROGUE_PDSINST_STMC_SOMASK_SHIFT) &
                         PVR_ROGUE_PDSINST_SOMASK_MASK;
   stmc->src0->instruction = &stmc->instruction;

   return &stmc->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_sp_limm(void *context,
                                        PVR_ERR_CALLBACK err_callback,
                                        struct pvr_dissassembler_error error,
                                        uint32_t instruction,
                                        bool cc)
{
   struct pvr_limm *limm = malloc(sizeof(*limm));
   assert(limm);
   limm->instruction.next = NULL;
   limm->instruction.type = INS_LIMM;

   limm->cc = cc;
   limm->GR = (instruction & PVR_ROGUE_PDSINST_LIMM_GR_ENABLE) != 0;
   limm->src0 = calloc(1, sizeof(*limm->src0));
   assert(limm->src0);

   limm->src0->type = LITERAL_NUM;
   limm->src0->literal = (instruction >> PVR_ROGUE_PDSINST_LIMM_SRC0_SHIFT) &
                         PVR_ROGUE_PDSINST_IMM16_MASK;
   limm->src0->instruction = &limm->instruction;
   limm->dst = pvr_pds_disassemble_regs32t(context,
                                           err_callback,
                                           error,
                                           instruction >>
                                              PVR_ROGUE_PDSINST_LIMM_SRC1_SHIFT,
                                           0);
   limm->dst->instruction = &limm->instruction;

   return &limm->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_simple(enum pvr_instruction_type type, bool cc)
{
   struct pvr_simple *ins = malloc(sizeof(*ins));
   assert(ins);

   ins->instruction.next = NULL;
   ins->instruction.type = type;
   ins->cc = cc;

   return &ins->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_bra(uint32_t instruction)
{
   uint32_t branch_addr;
   struct pvr_bra *bra = (struct pvr_bra *)malloc(sizeof(*bra));
   assert(bra);

   bra->instruction.type = INS_BRA;
   bra->instruction.next = NULL;

   branch_addr = (instruction >> PVR_ROGUE_PDSINST_BRA_ADDR_SHIFT) &
                 PVR_ROGUE_PDSINST_BRAADDR_MASK;
   bra->address = (branch_addr & 0x40000U) ? ((int)branch_addr) - 0x80000
                                           : (int)branch_addr;

   bra->srcc = malloc(sizeof(*bra->srcc));
   assert(bra->srcc);

   bra->srcc->predicate = (instruction >> PVR_ROGUE_PDSINST_BRA_SRCC_SHIFT) &
                          PVR_ROGUE_PDSINST_PREDICATE_MASK;
   bra->srcc->negate = instruction & PVR_ROGUE_PDSINST_BRA_NEG_ENABLE;

   bra->setc = malloc(sizeof(*bra->setc));
   assert(bra->setc);

   bra->setc->predicate = (instruction >> PVR_ROGUE_PDSINST_BRA_SETC_SHIFT) &
                          PVR_ROGUE_PDSINST_PREDICATE_MASK;

   bra->target = NULL;

   return &bra->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_sp(void *context,
                                   PVR_ERR_CALLBACK err_callback,
                                   struct pvr_dissassembler_error error,
                                   uint32_t instruction)
{
   uint32_t op = (instruction >> PVR_TYPE_OPCODE_SP_SHIFT) &
                 PVR_ROGUE_PDSINST_OPCODESP_MASK;
   bool cc = instruction & PVR_TYPE_OPCODE_SP;

   switch (op) {
   case PVR_ROGUE_PDSINST_OPCODESP_LD:
      error.instruction = INS_LD;
      return pvr_pds_disassemble_instruction_sp_ld_st(
         context,
         err_callback,
         error,
         true,
         instruction,
         instruction & (1 << PVR_ROGUE_PDSINST_LD_CC_SHIFT));
   case PVR_ROGUE_PDSINST_OPCODESP_ST:
      error.instruction = INS_ST;
      return pvr_pds_disassemble_instruction_sp_ld_st(
         context,
         err_callback,
         error,
         false,
         instruction,
         instruction & (1 << PVR_ROGUE_PDSINST_ST_CC_SHIFT));
   case PVR_ROGUE_PDSINST_OPCODESP_STMC:
      error.instruction = INS_STMC;
      return pvr_pds_disassemble_instruction_sp_stmc(instruction, cc);
   case PVR_ROGUE_PDSINST_OPCODESP_LIMM:
      error.instruction = INS_LIMM;
      return pvr_pds_disassemble_instruction_sp_limm(context,
                                                     err_callback,
                                                     error,
                                                     instruction,
                                                     cc);
   case PVR_ROGUE_PDSINST_OPCODESP_WDF:
      error.instruction = INS_WDF;
      return pvr_pds_disassemble_simple(INS_WDF, cc);
   case PVR_ROGUE_PDSINST_OPCODESP_LOCK:
      error.instruction = INS_LOCK;
      return pvr_pds_disassemble_simple(INS_LOCK, cc);
   case PVR_ROGUE_PDSINST_OPCODESP_RELEASE:
      error.instruction = INS_RELEASE;
      return pvr_pds_disassemble_simple(INS_RELEASE, cc);
   case PVR_ROGUE_PDSINST_OPCODESP_HALT:
      error.instruction = INS_HALT;
      return pvr_pds_disassemble_simple(INS_HALT, cc);
   case PVR_ROGUE_PDSINST_OPCODESP_NOP:
      error.instruction = INS_NOP;
      return pvr_pds_disassemble_simple(INS_NOP, cc);
   default:
      error.type = PVR_PDS_ERR_SP_UNKNOWN;
      error.text = "opcode unknown for special instruction";
      pvr_error_check(err_callback, error);
      return NULL;
   }
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_ddmad(void *context,
                                      PVR_ERR_CALLBACK err_callback,
                                      struct pvr_dissassembler_error error,
                                      uint32_t instruction)
{
   struct pvr_ddmad *ddmad = malloc(sizeof(*ddmad));
   assert(ddmad);

   ddmad->instruction.next = NULL;
   ddmad->instruction.type = INS_DDMAD;

   ddmad->cc = instruction & PVR_ROGUE_PDSINST_DDMAD_CC_ENABLE;
   ddmad->END = instruction & PVR_ROGUE_PDSINST_DDMAD_END_ENABLE;

   ddmad->src0 = pvr_pds_disassemble_regs32(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_DDMAD_SRC0_SHIFT,
      1);
   ddmad->src0->instruction = &ddmad->instruction;

   ddmad->src1 = pvr_pds_disassemble_regs32t(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_DDMAD_SRC1_SHIFT,
      2);
   ddmad->src1->instruction = &ddmad->instruction;

   ddmad->src2 = pvr_pds_disassemble_regs64(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_DDMAD_SRC2_SHIFT,
      3);
   ddmad->src2->instruction = &ddmad->instruction;

   ddmad->src3 = pvr_pds_disassemble_regs64C(
      context,
      err_callback,
      error,
      instruction >> PVR_ROGUE_PDSINST_DDMAD_SRC3_SHIFT,
      4);
   ddmad->src3->instruction = &ddmad->instruction;

   return &ddmad->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_mad(void *context,
                                    PVR_ERR_CALLBACK err_callback,
                                    struct pvr_dissassembler_error error,
                                    uint32_t instruction)
{
   struct pvr_mad *mad = malloc(sizeof(*mad));
   assert(mad);

   mad->instruction.next = NULL;
   mad->instruction.type = INS_MAD;

   mad->cc = instruction & PVR_ROGUE_PDSINST_MAD_CC_ENABLE;
   mad->sna = instruction & PVR_ROGUE_PDSINST_MAD_SNA_SUB;
   mad->alum = (instruction & PVR_ROGUE_PDSINST_MAD_ALUM_SIGNED);

   mad->src0 = pvr_pds_disassemble_regs32(context,
                                          err_callback,
                                          error,
                                          instruction >>
                                             PVR_ROGUE_PDSINST_MAD_SRC0_SHIFT,
                                          1);
   mad->src0->instruction = &mad->instruction;

   mad->src1 = pvr_pds_disassemble_regs32(context,
                                          err_callback,
                                          error,
                                          instruction >>
                                             PVR_ROGUE_PDSINST_MAD_SRC1_SHIFT,
                                          2);
   mad->src1->instruction = &mad->instruction;

   mad->src2 = pvr_pds_disassemble_regs64(context,
                                          err_callback,
                                          error,
                                          instruction >>
                                             PVR_ROGUE_PDSINST_MAD_SRC2_SHIFT,
                                          3);
   mad->src2->instruction = &mad->instruction;

   mad->dst = pvr_pds_disassemble_regs64t(context,
                                          err_callback,
                                          error,
                                          instruction >>
                                             PVR_ROGUE_PDSINST_MAD_DST_SHIFT,
                                          0);
   mad->dst->instruction = &mad->instruction;

   return &mad->instruction;
}

static struct pvr_instruction *
pvr_pds_disassemble_instruction_dout(void *context,
                                     PVR_ERR_CALLBACK err_callback,
                                     struct pvr_dissassembler_error error,
                                     uint32_t instruction)
{
   struct pvr_dout *dout = malloc(sizeof(*dout));
   assert(dout);

   dout->instruction.next = NULL;
   dout->instruction.type = INS_DOUT;

   dout->END = instruction & PVR_ROGUE_PDSINST_DOUT_END_ENABLE;
   dout->cc = instruction & PVR_ROGUE_PDSINST_DOUT_CC_ENABLE;
   dout->dst = (instruction >> PVR_ROGUE_PDSINST_DOUT_DST_SHIFT) &
               PVR_ROGUE_PDSINST_DSTDOUT_MASK;

   dout->src0 = pvr_pds_disassemble_regs64(context,
                                           err_callback,
                                           error,
                                           instruction >>
                                              PVR_ROGUE_PDSINST_DOUT_SRC0_SHIFT,
                                           1);
   dout->src0->instruction = &dout->instruction;

   dout->src1 = pvr_pds_disassemble_regs32(context,
                                           err_callback,
                                           error,
                                           instruction >>
                                              PVR_ROGUE_PDSINST_DOUT_SRC1_SHIFT,
                                           2);
   dout->src1->instruction = &dout->instruction;

   return &dout->instruction;
}

static void pvr_pds_free_instruction_limm(struct pvr_limm *inst)
{
   free(inst->dst);
   free(inst->src0);
   free(inst);
}

static void pvr_pds_free_instruction_add(struct pvr_add *inst)
{
   free(inst->dst);
   free(inst->src0);
   free(inst->src1);
   free(inst);
}

static void pvr_pds_free_instruction_cmp(struct pvr_cmp *inst)
{
   free(inst->src0);
   free(inst->src1);
   free(inst);
}

static void pvr_pds_free_instruction_mad(struct pvr_mad *inst)
{
   free(inst->dst);
   free(inst->src0);
   free(inst->src1);
   free(inst->src2);
   free(inst);
}

static void pvr_pds_free_instruction_bra(struct pvr_bra *inst)
{
   free(inst->setc);
   free(inst->srcc);
   free(inst);
}

static void pvr_pds_free_instruction_ddmad(struct pvr_ddmad *inst)
{
   free(inst->src0);
   free(inst->src1);
   free(inst->src2);
   free(inst->src3);
   free(inst);
}

static void pvr_pds_free_instruction_dout(struct pvr_dout *inst)
{
   free(inst->src0);
   free(inst->src1);
   free(inst);
}

static void pvr_pds_free_instruction_ldst(struct pvr_ldst *inst)
{
   free(inst->src0);
   free(inst);
}

static void pvr_pds_free_instruction_simple(struct pvr_simple *inst)
{
   free(inst);
}

static void pvr_pds_free_instruction_sfltp(struct pvr_sftlp *inst)
{
   free(inst->dst);
   free(inst->src0);
   free(inst->src1);
   free(inst->src2);
   free(inst);
}

static void pvr_pds_free_instruction_stm(struct pvr_stm *inst)
{
   free(inst->src0);
   free(inst->src1);
   free(inst->src2);
   free(inst->src3);
   free(inst);
}

static void pvr_pds_free_instruction_stmc(struct pvr_stmc *inst)
{
   free(inst->src0);
   free(inst);
}

void pvr_pds_free_instruction(struct pvr_instruction *instruction)
{
   if (!instruction)
      return;

   switch (instruction->type) {
   case INS_LIMM:
      pvr_pds_free_instruction_limm((struct pvr_limm *)instruction);
      break;
   case INS_ADD64:
   case INS_ADD32:
      pvr_pds_free_instruction_add((struct pvr_add *)instruction);
      break;
   case INS_CMP:
      pvr_pds_free_instruction_cmp((struct pvr_cmp *)instruction);
      break;
   case INS_MAD:
      pvr_pds_free_instruction_mad((struct pvr_mad *)instruction);
      break;
   case INS_BRA:
      pvr_pds_free_instruction_bra((struct pvr_bra *)instruction);
      break;
   case INS_DDMAD:
      pvr_pds_free_instruction_ddmad((struct pvr_ddmad *)instruction);
      break;
   case INS_DOUT:
      pvr_pds_free_instruction_dout((struct pvr_dout *)instruction);
      break;
   case INS_LD:
   case INS_ST:
      pvr_pds_free_instruction_ldst((struct pvr_ldst *)instruction);
      break;
   case INS_WDF:
   case INS_LOCK:
   case INS_RELEASE:
   case INS_HALT:
   case INS_NOP:
      pvr_pds_free_instruction_simple((struct pvr_simple *)instruction);
      break;
   case INS_SFTLP64:
   case INS_SFTLP32:
      pvr_pds_free_instruction_sfltp((struct pvr_sftlp *)instruction);
      break;
   case INS_STM:
      pvr_pds_free_instruction_stm((struct pvr_stm *)instruction);
      break;
   case INS_STMC:
      pvr_pds_free_instruction_stmc((struct pvr_stmc *)instruction);
      break;
   }
}

struct pvr_instruction *
pvr_pds_disassemble_instruction2(void *context,
                                 PVR_ERR_CALLBACK err_callback,
                                 uint32_t instruction)
{
   struct pvr_dissassembler_error error = { .context = context };

   /* First we need to find out what type of OPCODE we are dealing with. */
   if (instruction & PVR_TYPE_OPCODE) {
      uint32_t opcode_C = (instruction >> PVR_TYPE_OPCODE_SHIFT) &
                          PVR_ROGUE_PDSINST_OPCODEC_MASK;
      switch (opcode_C) {
      case PVR_ROGUE_PDSINST_OPCODEC_ADD64:
         error.instruction = INS_ADD64;
         return pvr_pds_disassemble_instruction_add64(context,
                                                      err_callback,
                                                      error,
                                                      instruction);
      case PVR_ROGUE_PDSINST_OPCODEC_ADD32:
         error.instruction = INS_ADD32;
         return pvr_pds_disassemble_instruction_add32(context,
                                                      err_callback,
                                                      error,
                                                      instruction);
      case PVR_ROGUE_PDSINST_OPCODEC_SFTLP64:
         error.instruction = INS_SFTLP64;
         return pvr_pds_disassemble_instruction_sftlp64(context,
                                                        err_callback,
                                                        error,
                                                        instruction);
      case PVR_ROGUE_PDSINST_OPCODEC_CMP:
         error.instruction = INS_CMP;
         return pvr_pds_disassemble_instruction_cmp(context,
                                                    err_callback,
                                                    error,
                                                    instruction);
      case PVR_ROGUE_PDSINST_OPCODEC_BRA:
         error.instruction = INS_BRA;
         return pvr_pds_disassemble_instruction_bra(instruction);
      case PVR_ROGUE_PDSINST_OPCODEC_SP:
         return pvr_pds_disassemble_instruction_sp(context,
                                                   err_callback,
                                                   error,
                                                   instruction);
      case PVR_ROGUE_PDSINST_OPCODEC_DDMAD:
         error.instruction = INS_DDMAD;
         return pvr_pds_disassemble_instruction_ddmad(context,
                                                      err_callback,
                                                      error,
                                                      instruction);
      case PVR_ROGUE_PDSINST_OPCODEC_DOUT:
         error.instruction = INS_DOUT;
         return pvr_pds_disassemble_instruction_dout(context,
                                                     err_callback,
                                                     error,
                                                     instruction);
      }
   } else if (instruction & PVR_TYPE_OPCODEB) {
      uint32_t opcode_B = (instruction >> PVR_TYPE_OPCODEB_SHIFT) &
                          PVR_ROGUE_PDSINST_OPCODEB_MASK;
      switch (opcode_B) {
      case PVR_ROGUE_PDSINST_OPCODEB_SFTLP32:
         error.instruction = INS_SFTLP32;
         return pvr_pds_disassemble_instruction_sftlp32(context,
                                                        err_callback,
                                                        error,
                                                        instruction);
      case PVR_ROGUE_PDSINST_OPCODEB_STM:
         error.instruction = INS_STM;
         return pvr_pds_disassemble_instruction_stm(context,
                                                    err_callback,
                                                    error,
                                                    instruction);
      }
   } else { /* Opcode A - MAD instruction. */
      error.instruction = INS_MAD;
      return pvr_pds_disassemble_instruction_mad(context,
                                                 err_callback,
                                                 error,
                                                 instruction);
   }
   return NULL;
}
