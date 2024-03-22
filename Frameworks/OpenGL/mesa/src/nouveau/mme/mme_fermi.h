/*
 * Copyright © 2022 Mary Guillemard
 * SPDX-License-Identifier: MIT
 */
#ifndef MME_FERMI_H
#define MME_FERMI_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MME_FERMI_DRAM_COUNT 0xc00
#define MME_FERMI_SCRATCH_COUNT 128

enum ENUM_PACKED mme_fermi_reg {
   MME_FERMI_REG_ZERO,
   MME_FERMI_REG_R1,
   MME_FERMI_REG_R2,
   MME_FERMI_REG_R3,
   MME_FERMI_REG_R4,
   MME_FERMI_REG_R5,
   MME_FERMI_REG_R6,
   MME_FERMI_REG_R7,
};

enum ENUM_PACKED mme_fermi_op {
   MME_FERMI_OP_ALU_REG,
   MME_FERMI_OP_ADD_IMM,
   MME_FERMI_OP_MERGE,
   MME_FERMI_OP_BFE_LSL_IMM,
   MME_FERMI_OP_BFE_LSL_REG,
   MME_FERMI_OP_STATE,
   MME_FERMI_OP_UNK6,
   MME_FERMI_OP_BRANCH,
};

const char *mme_fermi_op_to_str(enum mme_fermi_op op);

enum ENUM_PACKED mme_fermi_alu_op {
    MME_FERMI_ALU_OP_ADD,
    MME_FERMI_ALU_OP_ADDC,
    MME_FERMI_ALU_OP_SUB,
    MME_FERMI_ALU_OP_SUBB,
    MME_FERMI_ALU_OP_RESERVED4,
    MME_FERMI_ALU_OP_RESERVED5,
    MME_FERMI_ALU_OP_RESERVED6,
    MME_FERMI_ALU_OP_RESERVED7,
    MME_FERMI_ALU_OP_XOR,
    MME_FERMI_ALU_OP_OR,
    MME_FERMI_ALU_OP_AND,
    MME_FERMI_ALU_OP_AND_NOT,
    MME_FERMI_ALU_OP_NAND,
    MME_FERMI_ALU_OP_RESERVED13,
    MME_FERMI_ALU_OP_RESERVED14,
    MME_FERMI_ALU_OP_RESERVED15,
    MME_FERMI_ALU_OP_RESERVED16,
    MME_FERMI_ALU_OP_RESERVED17,
    MME_FERMI_ALU_OP_RESERVED18,
    MME_FERMI_ALU_OP_RESERVED19,
    MME_FERMI_ALU_OP_RESERVED20,
    MME_FERMI_ALU_OP_RESERVED21,
    MME_FERMI_ALU_OP_RESERVED22,
    MME_FERMI_ALU_OP_RESERVED23,
    MME_FERMI_ALU_OP_RESERVED24,
    MME_FERMI_ALU_OP_RESERVED25,
    MME_FERMI_ALU_OP_RESERVED26,
    MME_FERMI_ALU_OP_RESERVED27,
    MME_FERMI_ALU_OP_RESERVED28,
    MME_FERMI_ALU_OP_RESERVED29,
    MME_FERMI_ALU_OP_RESERVED30,
    MME_FERMI_ALU_OP_RESERVED31,
};

const char *mme_fermi_alu_op_to_str(enum mme_fermi_alu_op op);


enum ENUM_PACKED mme_fermi_assign_op {
    MME_FERMI_ASSIGN_OP_LOAD,
    MME_FERMI_ASSIGN_OP_MOVE,
    MME_FERMI_ASSIGN_OP_MOVE_SET_MADDR,
    MME_FERMI_ASSIGN_OP_LOAD_EMIT,
    MME_FERMI_ASSIGN_OP_MOVE_EMIT,
    MME_FERMI_ASSIGN_OP_LOAD_SET_MADDR,
    MME_FERMI_ASSIGN_OP_MOVE_SET_MADDR_LOAD_EMIT,
    MME_FERMI_ASSIGN_OP_MOVE_SET_MADDR_LOAD_EMIT_HIGH,
};

const char *mme_fermi_assign_op_to_str(enum mme_fermi_assign_op op);

struct mme_fermi_bitfield {
    uint8_t src_bit;
    uint8_t size;
    uint8_t dst_bit;
};

struct mme_fermi_branch {
    bool not_zero;
    bool no_delay;
};

struct mme_fermi_inst {
    bool end_next;
    enum mme_fermi_assign_op assign_op;
    enum mme_fermi_op op;
    enum mme_fermi_reg dst;
    enum mme_fermi_reg src[2];
    int32_t imm;
    union {
        enum mme_fermi_alu_op alu_op;
        struct mme_fermi_bitfield bitfield;
        struct mme_fermi_branch branch;
    };
};

#define MME_FERMI_INST_DEFAULTS              \
   .end_next = false,                        \
   .assign_op = MME_FERMI_ASSIGN_OP_MOVE,    \
   .op = MME_FERMI_OP_ALU_REG,               \
   .dst = MME_FERMI_REG_ZERO,                \
   .src = {                                  \
      MME_FERMI_REG_ZERO,                    \
      MME_FERMI_REG_ZERO                     \
   },                                        \
   .imm = 0,                                 \
   .alu_op = MME_FERMI_ALU_OP_ADD,           \

void mme_fermi_print_inst(FILE *fp, unsigned indent,
                          const struct mme_fermi_inst *inst);

void mme_fermi_print(FILE *fp, const struct mme_fermi_inst *insts,
                     uint32_t inst_count);

void mme_fermi_encode(uint32_t *out, uint32_t inst_count,
                      const struct mme_fermi_inst *insts);

void mme_fermi_decode(struct mme_fermi_inst *insts,
                      const uint32_t *in, uint32_t inst_count);

void mme_fermi_dump(FILE *fp, uint32_t *encoded, size_t encoded_size);

#ifdef __cplusplus
}
#endif

#endif /* MME_FERMI_H */
