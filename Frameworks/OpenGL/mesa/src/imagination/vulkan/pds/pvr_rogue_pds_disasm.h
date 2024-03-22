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

#ifndef PVR_ROGUE_PDS_DISASM_H
#define PVR_ROGUE_PDS_DISASM_H

#include <stdbool.h>
#include <stdint.h>

#include "util/log.h"

/* Type of operand for an instruction. */
#define PVR_PDS_OPERAND_TYPES   \
   X(TEMP32, temp, 32)          \
   X(PTEMP32, ptemp, 32)        \
   X(CONST32, const, 32)        \
   X(TEMP64, temp, 64)          \
   X(PTEMP64, ptemp, 64)        \
   X(CONST64, const, 64)        \
   X(UNRESOLVED, UNRESOLVED, 0) \
   X(LITERAL_NUM, literal, 0)

#define X(enum, str, size) enum,
enum pvr_operand_type { PVR_PDS_OPERAND_TYPES };
#undef X

#if defined(DUMP_PDS)
#   define PVR_PDS_PRINT_INST(X) pvr_pds_print_instruction(X)
#   define PVR_PDS_PRINT_DATA(X, Y, Z) \
      mesa_logd("\t%s   : DATA = 0x%lX ADDRESS = 0x%X\n", X, (uint64_t)(Y), Z)
#else
#   define PVR_PDS_PRINT_INST(X)
#   define PVR_PDS_PRINT_DATA(X, Y, Z)
#endif

#define PVR_INSTRUCTION_STMP
#define PVR_INSTRUCTION_IDIV
#define PVR_INSTRUCTION_AA
#define PVR_INSTRUCTION_POL
#define PVR_INSTRUCTION_IDF

#define PVR_INSTRUCTIONS \
   X(STM)                \
   PVR_INSTRUCTION_STMP  \
   PVR_INSTRUCTION_IDIV  \
   PVR_INSTRUCTION_AA    \
   PVR_INSTRUCTION_IDF   \
   PVR_INSTRUCTION_POL   \
   X(STMC)               \
   X(LD)                 \
   X(ST)                 \
   X(ADD32)              \
   X(ADD64)              \
   X(MAD)                \
   X(DDMAD)              \
   X(DOUT)               \
   X(CMP)                \
   X(BRA)                \
   X(LIMM)               \
   X(SFTLP32)            \
   X(SFTLP64)            \
   X(WDF)                \
   X(LOCK)               \
   X(RELEASE)            \
   X(HALT)               \
   X(NOP)

#define X(a) INS_##a,
enum pvr_instruction_type { PVR_INSTRUCTIONS };
#undef X

struct pvr_predicate {
   uint32_t predicate;
   bool negate;
};

struct pvr_instruction;

/* Operands are either sources or dst of an instruction. */
struct pvr_operand {
   enum pvr_operand_type type;

   struct pvr_instruction *instruction;
   uint64_t literal; /* Literal value if type == LITERAL_NUM */
   int address; /* Address in word-sizes. */
   unsigned absolute_address; /* Address in segment, */
   unsigned index; /* Index within instruction, 0 = dst, 1 = src0 .. */
   bool negate; /* True if the literal is negative. */
};

#define PVR_PDS_LOP  \
   X(LOP_NONE, none) \
   X(LOP_NOT, ~)     \
   X(LOP_AND, &)     \
   X(LOP_OR, |)      \
   X(LOP_XOR, xor)   \
   X(LOP_XNOR, xnor) \
   X(LOP_NAND, nand) \
   X(LOP_NOR, nor)

#define X(lop, str) lop,
enum pvr_pds_lop { PVR_PDS_LOP };
#undef X

#define PVR_PDS_DOUT_DSTS \
   X(DOUT_D, doutd)       \
   X(DOUT_W, doutw)       \
   X(DOUT_U, doutu)       \
   X(DOUT_V, doutv)       \
   X(DOUT_I, douti)       \
   X(DOUT_C, doutc)       \
   X(DOUT_R, doutr)       \
   X(DOUT_INVALID0, invalid)

#define X(dout_dst, str) dout_dst,
enum pvr_dout_type { PVR_PDS_DOUT_DSTS };
#undef X

#define PVR_PDS_MAX_INST_STR_LEN 256

enum pvr_cop { COP_EQ, COP_GT, COP_LT, COP_NE };

struct pvr_instruction {
   enum pvr_instruction_type type;
   struct pvr_instruction *next;
};

struct pvr_add {
   struct pvr_instruction instruction;
   struct pvr_operand *dst;
   struct pvr_operand *src1;
   struct pvr_operand *src0;
   bool cc;
   bool sna;
   bool alum;
};

struct pvr_simple {
   struct pvr_instruction instruction;
   bool cc;
};

struct pvr_ldst {
   struct pvr_instruction instruction;
   bool cc;
   struct pvr_operand *src0;
   bool st;
};

struct pvr_mad {
   struct pvr_instruction instruction;
   struct pvr_operand *dst;
   struct pvr_operand *src0;
   struct pvr_operand *src1;
   struct pvr_operand *src2;
   bool cc;
   bool sna;
   bool alum;
};

struct pvr_stm {
   struct pvr_instruction instruction;
   struct pvr_operand *src0;
   struct pvr_operand *src1;
   struct pvr_operand *src2;
   struct pvr_operand *src3;
   unsigned stream_out;
   bool tst;
   bool cc;
   bool ccs_global;
   bool ccs_so;
};

struct pvr_stmc {
   struct pvr_instruction instruction;
   struct pvr_operand *src0;
   bool cc;
};

struct pvr_bra {
   struct pvr_instruction instruction;
   struct pvr_predicate *srcc;
   struct pvr_predicate *setc; /* negate ignored */
   char *target;
   signed address; /* signed relative address */
};

struct pvr_dout {
   struct pvr_instruction instruction;
   struct pvr_operand *src0;
   struct pvr_operand *src1;
   enum pvr_dout_type dst;
   bool cc;
   bool END;
};

struct pvr_ddmad {
   struct pvr_instruction instruction;
   struct pvr_operand *src0;
   struct pvr_operand *src1;
   struct pvr_operand *src2;
   struct pvr_operand *src3;
   bool cc;
   bool END;
};

struct pvr_sftlp {
   struct pvr_instruction instruction;
   enum pvr_pds_lop lop;
   struct pvr_operand *dst;
   struct pvr_operand *src0;
   struct pvr_operand *src1;
   struct pvr_operand *src2;
   bool cc;
   bool IM;
};

struct pvr_limm {
   struct pvr_instruction instruction;
   bool cc;
   bool GR;
   struct pvr_operand *dst;
   struct pvr_operand *src0;
};

struct pvr_cmp {
   struct pvr_instruction instruction;
   enum pvr_cop cop;
   bool IM;
   bool cc;
   struct pvr_operand *src0;
   struct pvr_operand *src1;
};

#define PVR_PDS_ERR_PARAM_RANGE 0 /* Error when register is out of range. */
#define PVR_PDS_ERR_SP_UNKNOWN \
   1 /* Error when opcode for sp instruction is unknown. */

struct pvr_dissassembler_error {
   uint32_t type; /* One of PDS_ERR_* */
   enum pvr_instruction_type instruction; /* The type of instruction where
                                             the error occurred. */
   char *text; /* A string representation of the error. */
   uint32_t parameter; /* The parameter of the instruction, 0 = dst,
                          1 = src0.. */
   uint32_t raw; /* The raw value that caused the error. */

   void *context; /* The passed in context. */
};

/* Callback when an error happens. */
typedef void (*PVR_ERR_CALLBACK)(struct pvr_dissassembler_error);

void pvr_pds_free_instruction(struct pvr_instruction *inst);
struct pvr_instruction *
pvr_pds_disassemble_instruction2(void *context,
                                 PVR_ERR_CALLBACK error_call_back,
                                 uint32_t instruction);
void pvr_pds_disassemble_instruction(char *buffer,
                                     size_t instr_len,
                                     struct pvr_instruction *instruction);

#if defined(DUMP_PDS)
void pvr_pds_print_instruction(uint32_t instr);
#endif

#endif /* PVR_ROGUE_PDS_DISASM_H */
