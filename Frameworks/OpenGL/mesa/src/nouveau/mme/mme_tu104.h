/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#ifndef MME_TU104_H
#define MME_TU104_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MME_TU104_DRAM_COUNT 0xc00
#define MME_TU104_SCRATCH_COUNT 256

enum ENUM_PACKED mme_tu104_pred {
   MME_TU104_PRED_UUUU,
   MME_TU104_PRED_TTTT,
   MME_TU104_PRED_FFFF,
   MME_TU104_PRED_TTUU,
   MME_TU104_PRED_FFUU,
   MME_TU104_PRED_TFUU,
   MME_TU104_PRED_TUUU,
   MME_TU104_PRED_FUUU,
   MME_TU104_PRED_UUTT,
   MME_TU104_PRED_UUTF,
   MME_TU104_PRED_UUTU,
   MME_TU104_PRED_UUFT,
   MME_TU104_PRED_UUFF,
   MME_TU104_PRED_UUFU,
   MME_TU104_PRED_UUUT,
   MME_TU104_PRED_UUUF,
};

const char *mme_tu104_pred_to_str(enum mme_tu104_pred pred);

enum ENUM_PACKED mme_tu104_reg {
   MME_TU104_REG_R0,
   MME_TU104_REG_R1,
   MME_TU104_REG_R2,
   MME_TU104_REG_R3,
   MME_TU104_REG_R4,
   MME_TU104_REG_R5,
   MME_TU104_REG_R6,
   MME_TU104_REG_R7,
   MME_TU104_REG_R8,
   MME_TU104_REG_R9,
   MME_TU104_REG_R10,
   MME_TU104_REG_R11,
   MME_TU104_REG_R12,
   MME_TU104_REG_R13,
   MME_TU104_REG_R14,
   MME_TU104_REG_R15,
   MME_TU104_REG_R16,
   MME_TU104_REG_R17,
   MME_TU104_REG_R18,
   MME_TU104_REG_R19,
   MME_TU104_REG_R20,
   MME_TU104_REG_R21,
   MME_TU104_REG_R22,
   MME_TU104_REG_R23,
   MME_TU104_REG_ZERO,
   MME_TU104_REG_IMM,
   MME_TU104_REG_IMMPAIR,
   MME_TU104_REG_IMM32,
   MME_TU104_REG_LOAD0,
   MME_TU104_REG_LOAD1,
   MME_TU104_REG_VIRTUAL0 = 32,
};

enum ENUM_PACKED mme_tu104_alu_op {
   MME_TU104_ALU_OP_ADD,
   MME_TU104_ALU_OP_ADDC,
   MME_TU104_ALU_OP_SUB,
   MME_TU104_ALU_OP_SUBB,
   MME_TU104_ALU_OP_MUL,
   MME_TU104_ALU_OP_MULH,
   MME_TU104_ALU_OP_MULU,
   MME_TU104_ALU_OP_EXTENDED,
   MME_TU104_ALU_OP_CLZ,
   MME_TU104_ALU_OP_SLL,
   MME_TU104_ALU_OP_SRL,
   MME_TU104_ALU_OP_SRA,
   MME_TU104_ALU_OP_AND,
   MME_TU104_ALU_OP_NAND,
   MME_TU104_ALU_OP_OR,
   MME_TU104_ALU_OP_XOR,
   MME_TU104_ALU_OP_MERGE,
   MME_TU104_ALU_OP_SLT,
   MME_TU104_ALU_OP_SLTU,
   MME_TU104_ALU_OP_SLE,
   MME_TU104_ALU_OP_SLEU,
   MME_TU104_ALU_OP_SEQ,
   MME_TU104_ALU_OP_STATE,
   MME_TU104_ALU_OP_LOOP,
   MME_TU104_ALU_OP_JAL,
   MME_TU104_ALU_OP_BLT,
   MME_TU104_ALU_OP_BLTU,
   MME_TU104_ALU_OP_BLE,
   MME_TU104_ALU_OP_BLEU,
   MME_TU104_ALU_OP_BEQ,
   MME_TU104_ALU_OP_DREAD,
   MME_TU104_ALU_OP_DWRITE,
};

const char *mme_tu104_alu_op_to_str(enum mme_tu104_alu_op op);

bool mme_tu104_alu_op_has_implicit_imm(enum mme_tu104_alu_op op);
bool mme_tu104_alu_op_has_side_effects(enum mme_tu104_alu_op op);
bool mme_tu104_alu_op_is_control_flow(enum mme_tu104_alu_op op);
bool mme_tu104_alu_op_may_depend_on_mthd(enum mme_tu104_alu_op op);

enum ENUM_PACKED mme_tu104_out_op {
   MME_TU104_OUT_OP_NONE,
   MME_TU104_OUT_OP_ALU0,
   MME_TU104_OUT_OP_ALU1,
   MME_TU104_OUT_OP_LOAD0,
   MME_TU104_OUT_OP_LOAD1,
   MME_TU104_OUT_OP_IMM0,
   MME_TU104_OUT_OP_IMM1,
   MME_TU104_OUT_OP_RESERVED,
   MME_TU104_OUT_OP_IMMHIGH0,
   MME_TU104_OUT_OP_IMMHIGH1,
   MME_TU104_OUT_OP_IMM32,
};

struct mme_tu104_alu {
   enum mme_tu104_reg dst;
   enum mme_tu104_alu_op op;
   enum mme_tu104_reg src[2];
};

bool mme_tu104_alus_have_dependency(const struct mme_tu104_alu *first,
                                    const struct mme_tu104_alu *second);

#define MME_TU104_ALU_DEFAULTS      \
   .dst = MME_TU104_REG_ZERO,       \
   .op = MME_TU104_ALU_OP_ADD,      \
   .src = {                         \
      MME_TU104_REG_ZERO,           \
      MME_TU104_REG_ZERO,           \
   },

struct mme_tu104_out {
   enum mme_tu104_out_op mthd;
   enum mme_tu104_out_op emit;
};

#define MME_TU104_OUT_DEFAULTS      \
   .mthd = MME_TU104_OUT_OP_NONE,   \
   .emit = MME_TU104_OUT_OP_NONE,

struct mme_tu104_inst {
   bool end_next;
   enum mme_tu104_pred pred_mode;
   enum mme_tu104_reg pred;
   uint16_t imm[2];
   struct mme_tu104_alu alu[2];
   struct mme_tu104_out out[2];
};

#define MME_TU104_INST_DEFAULTS        \
   .end_next = false,                  \
   .pred_mode = MME_TU104_PRED_UUUU,   \
   .pred = MME_TU104_REG_ZERO,         \
   .imm = { 0, 0 },                    \
   .alu = {                            \
      { MME_TU104_ALU_DEFAULTS },      \
      { MME_TU104_ALU_DEFAULTS }       \
   },                                  \
   .out = {                            \
      { MME_TU104_OUT_DEFAULTS },      \
      { MME_TU104_OUT_DEFAULTS }       \
   },

void mme_tu104_print_inst(FILE *fp, unsigned indent,
                          const struct mme_tu104_inst *inst);

void mme_tu104_print(FILE *fp, const struct mme_tu104_inst *insts,
                     uint32_t inst_count);

void mme_tu104_encode(uint32_t *out, uint32_t inst_count,
                      const struct mme_tu104_inst *insts);

void mme_tu104_decode(struct mme_tu104_inst *insts,
                      const uint32_t *in, uint32_t inst_count);

void mme_tu104_dump(FILE *fp, uint32_t *encoded, size_t encoded_size);

#ifdef __cplusplus
}
#endif

#endif /* MME_TU104_H */
