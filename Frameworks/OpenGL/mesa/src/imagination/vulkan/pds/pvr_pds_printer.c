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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pvr_rogue_pds_defs.h"
#include "pvr_rogue_pds_disasm.h"
#include "pvr_rogue_pds_encode.h"
#include "util/log.h"

#define X(lop, str) #str,
static const char *const LOP[] = { PVR_PDS_LOP };
#undef X

static void pvr_pds_disassemble_operand(struct pvr_operand *op,
                                        char *instr_str,
                                        size_t instr_len)
{
#define X(enum, str, size) { #str, #size },
   static const char *const regs[][2] = { PVR_PDS_OPERAND_TYPES };
#undef X

   if (op->type == LITERAL_NUM) {
      snprintf(instr_str,
               instr_len,
               "%s (%llu)",
               regs[op->type][0],
               (unsigned long long)op->literal);
   } else if (op->type == UNRESOLVED) {
      snprintf(instr_str, instr_len, "UNRESOLVED");
   } else {
      snprintf(instr_str,
               instr_len,
               "%s[%u].%s",
               regs[op->type][0],
               op->absolute_address,
               regs[op->type][1]);
   }
}

static void pvr_pds_disassemble_instruction_add64(struct pvr_add *add,
                                                  char *instr_str,
                                                  size_t instr_len)
{
   char dst[32];
   char src0[32];
   char src1[32];

   pvr_pds_disassemble_operand(add->src0, src0, sizeof(src0));
   pvr_pds_disassemble_operand(add->src1, src1, sizeof(src1));
   pvr_pds_disassemble_operand(add->dst, dst, sizeof(dst));

   snprintf(instr_str,
            instr_len,
            "%-16s%s%s = %s %s %s %s",
            "ADD64",
            add->cc ? "? " : "",
            dst,
            src0,
            add->sna ? "-" : "+",
            src1,
            add->alum ? "[signed]" : "");
}

static void pvr_pds_disassemble_instruction_add32(struct pvr_add *add,
                                                  char *instr_str,
                                                  size_t instr_len)
{
   char dst[32];
   char src0[32];
   char src1[32];

   pvr_pds_disassemble_operand(add->src0, src0, sizeof(src0));
   pvr_pds_disassemble_operand(add->src1, src1, sizeof(src1));
   pvr_pds_disassemble_operand(add->dst, dst, sizeof(dst));

   snprintf(instr_str,
            instr_len,
            "%-16s%s%s = %s %s %s %s",
            "ADD32",
            add->cc ? "? " : "",
            dst,
            src0,
            add->sna ? "-" : "+",
            src1,
            add->alum ? "[signed]" : "");
}

static void
pvr_pds_disassemble_instruction_sftlp32(struct pvr_sftlp *instruction,
                                        char *instr_str,
                                        size_t instr_len)
{
   char dst[32];
   char src0[32];
   char src1[32];
   char src2[32];

   pvr_pds_disassemble_operand(instruction->src0, src0, sizeof(src0));
   pvr_pds_disassemble_operand(instruction->src1, src1, sizeof(src1));
   pvr_pds_disassemble_operand(instruction->dst, dst, sizeof(dst));

   if (instruction->IM)
      snprintf(src2, sizeof(src2), "%u", (uint32_t)instruction->src2->literal);
   else
      pvr_pds_disassemble_operand(instruction->src2, src2, sizeof(src2));

   if (instruction->lop == LOP_NONE) {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = %s %s %s",
               "SFTLP32",
               instruction->cc ? "? " : "",
               dst,
               src0,
               instruction->IM ? instruction->src2->negate ? ">>" : "<<" : "<<",
               src2);
   } else if (instruction->lop == LOP_NOT) {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = (~%s) %s %s",
               "SFTLP32",
               instruction->cc ? "? " : "",
               dst,
               src0,
               instruction->IM ? instruction->src2->negate ? ">>" : "<<" : "<<",
               src2);
   } else {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = (%s %s %s) %s %s",
               "SFTLP32",
               instruction->cc ? "? " : "",
               dst,
               src0,
               LOP[instruction->lop],
               src1,
               instruction->IM ? instruction->src2->negate ? ">>" : "<<" : "<<",
               src2);
   }
}

static void pvr_pds_disassemble_instruction_stm(struct pvr_stm *instruction,
                                                char *instr_str,
                                                size_t instr_len)
{
   char src0[32];
   char src1[32];
   char src2[32];
   char src3[32];

   char stm_pred[64];

   pvr_pds_disassemble_operand(instruction->src0, src0, sizeof(src0));
   pvr_pds_disassemble_operand(instruction->src1, src1, sizeof(src1));
   pvr_pds_disassemble_operand(instruction->src2, src2, sizeof(src2));
   pvr_pds_disassemble_operand(instruction->src3, src3, sizeof(src3));

   if (instruction->ccs_global)
      snprintf(stm_pred, sizeof(stm_pred), "overflow_any");
   else if (instruction->ccs_so)
      snprintf(stm_pred, sizeof(stm_pred), "overflow_current");
   else
      stm_pred[0] = 0;

   snprintf(instr_str,
            instr_len,
            "%-16s%s%s%s stm%u = %s, %s, %s, %s",
            "STM",
            instruction->cc ? "? " : "",
            stm_pred,
            instruction->tst ? " (TST only)" : "",
            instruction->stream_out,
            src0,
            src1,
            src2,
            src3);
}

static void pds_disassemble_instruction_stmc(struct pvr_stmc *instruction,
                                             char *instr_str,
                                             size_t instr_len)
{
   char src0[32];

   pvr_pds_disassemble_operand(instruction->src0, src0, sizeof(src0));

   snprintf(instr_str,
            instr_len,
            "%-16s%s %s",
            "STMC",
            instruction->cc ? "? " : "",
            src0);
}

static void
pvr_pds_disassemble_instruction_sftlp64(struct pvr_sftlp *instruction,
                                        char *instr_str,
                                        size_t instr_len)
{
   char dst[32];
   char src0[32];
   char src1[32];
   char src2[32];

   pvr_pds_disassemble_operand(instruction->src0, src0, sizeof(src0));
   pvr_pds_disassemble_operand(instruction->src1, src1, sizeof(src1));
   pvr_pds_disassemble_operand(instruction->dst, dst, sizeof(dst));

   if (instruction->IM)
      snprintf(src2, sizeof(src2), "%u", (uint32_t)instruction->src2->literal);
   else
      pvr_pds_disassemble_operand(instruction->src2, src2, sizeof(src2));

   if (instruction->lop == LOP_NONE) {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = %s %s %s",
               "SFTLP64",
               instruction->cc ? "? " : "",
               dst,
               src0,
               instruction->IM ? instruction->src2->negate ? ">>" : "<<" : "<<",
               src2);
   } else if (instruction->lop == LOP_NOT) {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = (~%s) %s %s",
               "SFTLP64",
               instruction->cc ? "? " : "",
               dst,
               src0,
               instruction->IM ? instruction->src2->negate ? ">>" : "<<" : "<<",
               src2);
   } else {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = (%s %s %s) %s %s",
               "SFTLP64",
               instruction->cc ? "? " : "",
               dst,
               src0,
               LOP[instruction->lop],
               src1,
               instruction->IM ? instruction->src2->negate ? ">>" : "<<" : "<<",
               src2);
   }
}

static void pvr_pds_disassemble_instruction_cmp(struct pvr_cmp *cmp,
                                                char *instr_str,
                                                size_t instr_len)
{
   char src0[32];
   char src1[32];
   static const char *const COP[] = { "=", ">", "<", "!=" };

   pvr_pds_disassemble_operand(cmp->src0, src0, sizeof(src0));

   if (cmp->IM) {
      snprintf(src1,
               sizeof(src1),
               "%#04llx",
               (unsigned long long)cmp->src1->literal);
   } else {
      pvr_pds_disassemble_operand(cmp->src1, src1, sizeof(src1));
   }

   snprintf(instr_str,
            instr_len,
            "%-16s%sP0 = (%s %s %s)",
            "CMP",
            cmp->cc ? "? " : "",
            src0,
            COP[cmp->cop],
            src1);
}

static void pvr_pds_disassemble_instruction_ldst(struct pvr_ldst *ins,
                                                 char *instr_str,
                                                 size_t instr_len)
{
   char src0[PVR_PDS_MAX_INST_STR_LEN];

   pvr_pds_disassemble_operand(ins->src0, src0, sizeof(src0));

   if (ins->st) {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s: mem(%s) <= src(%s)",
               "ST",
               ins->cc ? "? " : "",
               src0,
               "?",
               "?");
   } else {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s: dst(%s) <= mem(%s)",
               "ld",
               ins->cc ? "? " : "",
               src0,
               "?",
               "?");
   }
}

static void pvr_pds_disassemble_simple(struct pvr_simple *simple,
                                       const char *type,
                                       char *instr_str,
                                       size_t instr_len)
{
   snprintf(instr_str, instr_len, "%-16s%s", type, simple->cc ? "? " : "");
}

static void pvr_pds_disassemble_instruction_limm(struct pvr_limm *limm,
                                                 char *instr_str,
                                                 size_t instr_len)
{
   int32_t imm = (uint32_t)limm->src0->literal;
   char dst[PVR_PDS_MAX_INST_STR_LEN];

   pvr_pds_disassemble_operand(limm->dst, dst, sizeof(dst));

   if (limm->GR) {
      char *pchGReg;

      switch (imm) {
      case 0:
         pchGReg = "cluster";
         break;
      case 1:
         pchGReg = "instance";
         break;
      default:
         pchGReg = "unknown";
      }

      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = G%d (%s)",
               "LIMM",
               limm->cc ? "? " : "",
               dst,
               imm,
               pchGReg);
   } else {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = %#04x",
               "LIMM",
               limm->cc ? "? " : "",
               dst,
               imm);
   }
}

static void pvr_pds_disassemble_instruction_ddmad(struct pvr_ddmad *ddmad,
                                                  char *instr_str,
                                                  size_t instr_len)
{
   char src0[PVR_PDS_MAX_INST_STR_LEN];
   char src1[PVR_PDS_MAX_INST_STR_LEN];
   char src2[PVR_PDS_MAX_INST_STR_LEN];
   char src3[PVR_PDS_MAX_INST_STR_LEN];

   pvr_pds_disassemble_operand(ddmad->src0, src0, sizeof(src0));
   pvr_pds_disassemble_operand(ddmad->src1, src1, sizeof(src1));
   pvr_pds_disassemble_operand(ddmad->src2, src2, sizeof(src2));
   pvr_pds_disassemble_operand(ddmad->src3, src3, sizeof(src3));

   snprintf(instr_str,
            instr_len,
            "%-16s%sdoutd = (%s * %s) + %s, %s%s",
            "DDMAD",
            ddmad->cc ? "? " : "",
            src0,
            src1,
            src2,
            src3,
            ddmad->END ? "; HALT" : "");
}

static void pvr_pds_disassemble_predicate(uint32_t predicate,
                                          char *buffer,
                                          size_t buffer_length)
{
   switch (predicate) {
   case PVR_ROGUE_PDSINST_PREDICATE_P0:
      snprintf(buffer, buffer_length, "%s", "p0");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_IF0:
      snprintf(buffer, buffer_length, "%s", "if0");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_IF1:
      snprintf(buffer, buffer_length, "%s", "if1");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_SO_OVERFLOW_PREDICATE_0:
      snprintf(buffer, buffer_length, "%s", "so_overflow_0");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_SO_OVERFLOW_PREDICATE_1:
      snprintf(buffer, buffer_length, "%s", "so_overflow_1");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_SO_OVERFLOW_PREDICATE_2:
      snprintf(buffer, buffer_length, "%s", "so_overflow_2");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_SO_OVERFLOW_PREDICATE_3:
      snprintf(buffer, buffer_length, "%s", "so_overflow_3");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_SO_OVERFLOW_PREDICATE_GLOBAL:
      snprintf(buffer, buffer_length, "%s", "so_overflow_any");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_KEEP:
      snprintf(buffer, buffer_length, "%s", "keep");
      break;
   case PVR_ROGUE_PDSINST_PREDICATE_OOB:
      snprintf(buffer, buffer_length, "%s", "oob");
      break;
   default:
      snprintf(buffer, buffer_length, "%s", "<ERROR>");
      break;
   }
}

static void pvr_pds_disassemble_instruction_bra(struct pvr_bra *bra,
                                                char *instr_str,
                                                size_t instr_len)
{
   char setc_pred[32];
   char srcc_pred[32];

   pvr_pds_disassemble_predicate(bra->srcc->predicate,
                                 srcc_pred,
                                 sizeof(srcc_pred));
   pvr_pds_disassemble_predicate(bra->setc->predicate,
                                 setc_pred,
                                 sizeof(setc_pred));

   if (bra->setc->predicate != PVR_ROGUE_PDSINST_PREDICATE_KEEP) {
      snprintf(instr_str,
               instr_len,
               "%-16sif %s%s %d ( setc = %s )",
               "BRA",
               bra->srcc->negate ? "! " : "",
               srcc_pred,
               bra->address,
               setc_pred);
   } else {
      snprintf(instr_str,
               instr_len,
               "%-16sif %s%s %d",
               "BRA",
               bra->srcc->negate ? "! " : "",
               srcc_pred,
               bra->address);
   }
}

static void pvr_pds_disassemble_instruction_mad(struct pvr_mad *mad,
                                                char *instr_str,
                                                size_t instr_len)
{
   char src0[PVR_PDS_MAX_INST_STR_LEN];
   char src1[PVR_PDS_MAX_INST_STR_LEN];
   char src2[PVR_PDS_MAX_INST_STR_LEN];
   char dst[PVR_PDS_MAX_INST_STR_LEN];

   pvr_pds_disassemble_operand(mad->src0, src0, sizeof(src0));
   pvr_pds_disassemble_operand(mad->src1, src1, sizeof(src1));
   pvr_pds_disassemble_operand(mad->src2, src2, sizeof(src2));
   pvr_pds_disassemble_operand(mad->dst, dst, sizeof(dst));

   snprintf(instr_str,
            instr_len,
            "%-16s%s%s = (%s * %s) %s %s%s",
            "MAD",
            mad->cc ? "? " : "",
            dst,
            src0,
            src1,
            mad->sna ? "-" : "+",
            src2,
            mad->alum ? " [signed]" : "");
}

static void pvr_pds_disassemble_instruction_dout(struct pvr_dout *dout,
                                                 char *instr_str,
                                                 size_t instr_len)
{
   char src0[PVR_PDS_MAX_INST_STR_LEN];
   char src1[PVR_PDS_MAX_INST_STR_LEN];

#define X(dout_dst, str) #str,
   static const char *const dst[] = { PVR_PDS_DOUT_DSTS };
#undef X

   pvr_pds_disassemble_operand(dout->src0, src0, sizeof(src0));
   pvr_pds_disassemble_operand(dout->src1, src1, sizeof(src1));

   {
      snprintf(instr_str,
               instr_len,
               "%-16s%s%s = %s, %s%s",
               "DOUT",
               dout->cc ? "? " : "",
               dst[dout->dst],
               src0,
               src1,
               dout->END ? "; HALT" : "");
   }
}

void pvr_pds_disassemble_instruction(char *instr_str,
                                     size_t instr_len,
                                     struct pvr_instruction *instruction)
{
   if (!instruction) {
      snprintf(instr_str,
               instr_len,
               "Instruction was not disassembled properly\n");
      return;
   }

   switch (instruction->type) {
   case INS_LIMM:
      pvr_pds_disassemble_instruction_limm((struct pvr_limm *)instruction,
                                           instr_str,
                                           instr_len);
      break;
   case INS_ADD64:
      pvr_pds_disassemble_instruction_add64((struct pvr_add *)instruction,
                                            instr_str,
                                            instr_len);
      break;
   case INS_ADD32:
      pvr_pds_disassemble_instruction_add32((struct pvr_add *)instruction,
                                            instr_str,
                                            instr_len);
      break;
   case INS_CMP:
      pvr_pds_disassemble_instruction_cmp((struct pvr_cmp *)instruction,
                                          instr_str,
                                          instr_len);
      break;
   case INS_MAD:
      pvr_pds_disassemble_instruction_mad((struct pvr_mad *)instruction,
                                          instr_str,
                                          instr_len);
      break;
   case INS_BRA:
      pvr_pds_disassemble_instruction_bra((struct pvr_bra *)instruction,
                                          instr_str,
                                          instr_len);
      break;
   case INS_DDMAD:
      pvr_pds_disassemble_instruction_ddmad((struct pvr_ddmad *)instruction,
                                            instr_str,
                                            instr_len);
      break;
   case INS_DOUT:
      pvr_pds_disassemble_instruction_dout((struct pvr_dout *)instruction,
                                           instr_str,
                                           instr_len);
      break;
   case INS_LD:
   case INS_ST:
      pvr_pds_disassemble_instruction_ldst((struct pvr_ldst *)instruction,
                                           instr_str,
                                           instr_len);
      break;
   case INS_WDF:
      pvr_pds_disassemble_simple((struct pvr_simple *)instruction,
                                 "WDF",
                                 instr_str,
                                 instr_len);
      break;
   case INS_LOCK:
      pvr_pds_disassemble_simple((struct pvr_simple *)instruction,
                                 "LOCK",
                                 instr_str,
                                 instr_len);
      break;
   case INS_RELEASE:
      pvr_pds_disassemble_simple((struct pvr_simple *)instruction,
                                 "RELEASE",
                                 instr_str,
                                 instr_len);
      break;
   case INS_HALT:
      pvr_pds_disassemble_simple((struct pvr_simple *)instruction,
                                 "HALT",
                                 instr_str,
                                 instr_len);
      break;
   case INS_NOP:
      pvr_pds_disassemble_simple((struct pvr_simple *)instruction,
                                 "NOP",
                                 instr_str,
                                 instr_len);
      break;
   case INS_SFTLP32:
      pvr_pds_disassemble_instruction_sftlp32((struct pvr_sftlp *)instruction,
                                              instr_str,
                                              instr_len);
      break;
   case INS_SFTLP64:
      pvr_pds_disassemble_instruction_sftlp64((struct pvr_sftlp *)instruction,
                                              instr_str,
                                              instr_len);
      break;
   case INS_STM:
      pvr_pds_disassemble_instruction_stm((struct pvr_stm *)instruction,
                                          instr_str,
                                          instr_len);
      break;
   case INS_STMC:
      pds_disassemble_instruction_stmc((struct pvr_stmc *)instruction,
                                       instr_str,
                                       instr_len);
      break;
   default:
      snprintf(instr_str, instr_len, "Printing not implemented\n");
      break;
   }
}

#if defined(DUMP_PDS)
void pvr_pds_print_instruction(uint32_t instr)
{
   char instruction_str[1024];
   struct pvr_instruction *decoded =
      pvr_pds_disassemble_instruction2(0, 0, instr);

   if (!decoded) {
      mesa_logd("%X\n", instr);
   } else {
      pvr_pds_disassemble_instruction(instruction_str,
                                      sizeof(instruction_str),
                                      decoded);
      mesa_logd("\t0x%08x, /* %s */\n", instr, instruction_str);
   }
}
#endif
