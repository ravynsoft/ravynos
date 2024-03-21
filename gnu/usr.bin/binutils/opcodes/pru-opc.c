/* TI PRU opcode list.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   Contributed by Dimitar Dimitrov <dimitar@dinux.eu>

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Source:
   http://processors.wiki.ti.com/index.php/Programmable_Realtime_Unit  */

#include "sysdep.h"
#include <stdio.h>
#include "opcode/pru.h"

/* Register string table.  */

#define DECLARE_REG(name, index)		\
  { #name ".b0", (index), RSEL_7_0 },		\
  { #name ".b1", (index), RSEL_15_8 },		\
  { #name ".b2", (index), RSEL_23_16 },		\
  { #name ".b3", (index), RSEL_31_24 },		\
  { #name ".w0", (index), RSEL_15_0 },		\
  { #name ".w1", (index), RSEL_23_8 },		\
  { #name ".w2", (index), RSEL_31_16 },		\
  { #name , (index), RSEL_31_0 }

const struct pru_reg pru_regs[] = {
  /* Standard register names.  */
  DECLARE_REG (r0, 0),
  DECLARE_REG (r1, 1),
  DECLARE_REG (sp, 2),		/* Stack pointer.  */
  DECLARE_REG (ra, 3),		/* Return address.  */
  DECLARE_REG (fp, 4),		/* Frame pointer.  */
  DECLARE_REG (r5, 5),
  DECLARE_REG (r6, 6),
  DECLARE_REG (r7, 7),
  DECLARE_REG (r8, 8),
  DECLARE_REG (r9, 9),
  DECLARE_REG (r10, 10),
  DECLARE_REG (r11, 11),
  DECLARE_REG (r12, 12),
  DECLARE_REG (r13, 13),
  DECLARE_REG (r14, 14),
  DECLARE_REG (r15, 15),
  DECLARE_REG (r16, 16),
  DECLARE_REG (r17, 17),
  DECLARE_REG (r18, 18),
  DECLARE_REG (r19, 19),
  DECLARE_REG (r20, 20),
  DECLARE_REG (r21, 21),
  DECLARE_REG (r22, 22),
  DECLARE_REG (r23, 23),
  DECLARE_REG (r24, 24),
  DECLARE_REG (r25, 25),
  DECLARE_REG (r26, 26),
  DECLARE_REG (r27, 27),
  DECLARE_REG (r28, 28),
  DECLARE_REG (r29, 29),
  DECLARE_REG (r30, 30),
  DECLARE_REG (r31, 31),

  /* Alternative names for special registers.  */
  DECLARE_REG (r2, 2),
  DECLARE_REG (r3, 3),
  DECLARE_REG (r4, 4)
};

#define PRU_NUM_REGS \
       ((sizeof pru_regs) / (sizeof (pru_regs[0])))
const int pru_num_regs = PRU_NUM_REGS;

#undef PRU_NUM_REGS

/* This is the opcode table used by the PRU GNU as and disassembler.  */
const struct pru_opcode pru_opcodes[] =
{
  /* { name, args,
       match, mask, pinfo, overflow_msg } */
#define DECLARE_FORMAT1_OPCODE(str, subop) \
  { #str, prui_ ## str, "d,s,b", \
    OP_MATCH_ ## subop, OP_MASK_FMT1_OP | OP_MASK_SUBOP, 0, \
    unsigned_immed8_overflow }

  DECLARE_FORMAT1_OPCODE (add, ADD),
  DECLARE_FORMAT1_OPCODE (adc, ADC),
  DECLARE_FORMAT1_OPCODE (sub, SUB),
  DECLARE_FORMAT1_OPCODE (suc, SUC),
  DECLARE_FORMAT1_OPCODE (lsl, LSL),
  DECLARE_FORMAT1_OPCODE (lsr, LSR),
  DECLARE_FORMAT1_OPCODE (rsb, RSB),
  DECLARE_FORMAT1_OPCODE (rsc, RSC),
  DECLARE_FORMAT1_OPCODE (and, AND),
  DECLARE_FORMAT1_OPCODE (or, OR),
  DECLARE_FORMAT1_OPCODE (xor, XOR),
  DECLARE_FORMAT1_OPCODE (min, MIN),
  DECLARE_FORMAT1_OPCODE (max, MAX),
  DECLARE_FORMAT1_OPCODE (clr, CLR),
  DECLARE_FORMAT1_OPCODE (set, SET),

  { "not", prui_not, "d,s",
   OP_MATCH_NOT | OP_MASK_IO,
   OP_MASK_FMT1_OP | OP_MASK_SUBOP | OP_MASK_IO, 0, no_overflow},

  { "jmp", prui_jmp, "j",
   OP_MATCH_JMP, OP_MASK_FMT2_OP | OP_MASK_SUBOP, 0, unsigned_immed16_overflow},
  { "jal", prui_jal, "d,j",
   OP_MATCH_JAL, OP_MASK_FMT2_OP | OP_MASK_SUBOP, 0, unsigned_immed16_overflow},
  { "ldi", prui_ldi, "d,W",
   OP_MATCH_LDI, OP_MASK_FMT2_OP | OP_MASK_SUBOP, 0, unsigned_immed16_overflow},
  { "lmbd", prui_lmbd, "d,s,b",
   OP_MATCH_LMBD, OP_MASK_FMT2_OP | OP_MASK_SUBOP, 0, unsigned_immed8_overflow},
  { "halt", prui_halt, "",
   OP_MATCH_HALT, OP_MASK_FMT2_OP | OP_MASK_SUBOP, 0, no_overflow},
  { "slp", prui_slp, "w",
   OP_MATCH_SLP, OP_MASK_FMT2_OP | OP_MASK_SUBOP, 0, no_overflow},

  { "xin", prui_xin, "x,D,n",
   OP_MATCH_XIN, OP_MASK_XFR_OP, 0, unsigned_immed8_overflow},
  { "xout", prui_xout, "x,D,n",
   OP_MATCH_XOUT, OP_MASK_XFR_OP, 0, unsigned_immed8_overflow},
  { "xchg", prui_xchg, "x,D,n",
   OP_MATCH_XCHG, OP_MASK_XFR_OP, 0, unsigned_immed8_overflow},
  { "sxin", prui_sxin, "x,D,n",
   OP_MATCH_SXIN, OP_MASK_XFR_OP, 0, unsigned_immed8_overflow},
  { "sxout", prui_sxout, "x,D,n",
   OP_MATCH_SXOUT, OP_MASK_XFR_OP, 0, unsigned_immed8_overflow},
  { "sxchg", prui_sxchg, "x,D,n",
   OP_MATCH_SXCHG, OP_MASK_XFR_OP, 0, unsigned_immed8_overflow},

  { "loop", prui_loop, "O,B",
   OP_MATCH_LOOP, OP_MASK_LOOP_OP, 0, unsigned_immed8_overflow},
  { "iloop", prui_loop, "O,B",
   OP_MATCH_ILOOP, OP_MASK_LOOP_OP, 0, unsigned_immed8_overflow},

  { "qbgt", prui_qbgt, "o,s,b",
   OP_MATCH_QBGT, OP_MASK_FMT4_OP | OP_MASK_CMP, 0, qbranch_target_overflow},
  { "qbge", prui_qbge, "o,s,b",
   OP_MATCH_QBGE, OP_MASK_FMT4_OP | OP_MASK_CMP, 0, qbranch_target_overflow},
  { "qblt", prui_qblt, "o,s,b",
   OP_MATCH_QBLT, OP_MASK_FMT4_OP | OP_MASK_CMP, 0, qbranch_target_overflow},
  { "qble", prui_qble, "o,s,b",
   OP_MATCH_QBLE, OP_MASK_FMT4_OP | OP_MASK_CMP, 0, qbranch_target_overflow},
  { "qbeq", prui_qbeq, "o,s,b",
   OP_MATCH_QBEQ, OP_MASK_FMT4_OP | OP_MASK_CMP, 0, qbranch_target_overflow},
  { "qbne", prui_qbne, "o,s,b",
   OP_MATCH_QBNE, OP_MASK_FMT4_OP | OP_MASK_CMP, 0, qbranch_target_overflow},
  { "qba", prui_qba, "o",
   OP_MATCH_QBA, OP_MASK_FMT4_OP | OP_MASK_CMP, 0, qbranch_target_overflow},

  { "qbbs", prui_qbbs, "o,s,b",
   OP_MATCH_QBBS, OP_MASK_FMT5_OP | OP_MASK_BCMP, 0, qbranch_target_overflow},
  { "qbbc", prui_qbbc, "o,s,b",
   OP_MATCH_QBBC, OP_MASK_FMT5_OP | OP_MASK_BCMP, 0, qbranch_target_overflow},

  { "lbbo", prui_lbbo, "D,S,b,l",
   OP_MATCH_LBBO, OP_MASK_FMT6AB_OP | OP_MASK_LOADSTORE, 0,
   unsigned_immed8_overflow},
  { "sbbo", prui_sbbo, "D,S,b,l",
   OP_MATCH_SBBO, OP_MASK_FMT6AB_OP | OP_MASK_LOADSTORE, 0,
   unsigned_immed8_overflow},
  { "lbco", prui_lbco, "D,c,b,l",
   OP_MATCH_LBCO, OP_MASK_FMT6CD_OP | OP_MASK_LOADSTORE, 0,
   unsigned_immed8_overflow},
  { "sbco", prui_sbco, "D,c,b,l",
   OP_MATCH_SBCO, OP_MASK_FMT6CD_OP | OP_MASK_LOADSTORE, 0,
   unsigned_immed8_overflow},

  /* Fill in the default values for the real-instruction arguments.
     The assembler will not do it!  */
  { "nop", prui_or, "",
   OP_MATCH_OR
     | (RSEL_31_0 << OP_SH_RS2SEL) | (0 << OP_SH_RS2)
     | (RSEL_31_0 << OP_SH_RS1SEL) | (0 << OP_SH_RS1)
     | (RSEL_31_0 << OP_SH_RDSEL) | (0 << OP_SH_RD),
   OP_MASK_FMT1_OP | OP_MASK_SUBOP
     | OP_MASK_RS2SEL | OP_MASK_RS2 | OP_MASK_RS1SEL | OP_MASK_RS1
     | OP_MASK_RDSEL | OP_MASK_RD | OP_MASK_IO,
   PRU_INSN_MACRO, no_overflow},
  { "mov", prui_or, "d,s",
   OP_MATCH_OR | (0 << OP_SH_IMM8) | OP_MASK_IO,
   OP_MASK_FMT1_OP | OP_MASK_SUBOP | OP_MASK_IMM8 | OP_MASK_IO,
   PRU_INSN_MACRO, no_overflow},
  { "ret", prui_jmp, "",
   OP_MATCH_JMP
     | (RSEL_31_16 << OP_SH_RS2SEL) | (3 << OP_SH_RS2),
   OP_MASK_FMT2_OP | OP_MASK_SUBOP
     | OP_MASK_RS2SEL | OP_MASK_RS2 | OP_MASK_IO,
   PRU_INSN_MACRO, unsigned_immed16_overflow},
  { "call", prui_jal, "j",
   OP_MATCH_JAL
     | (RSEL_31_16 << OP_SH_RDSEL) | (3 << OP_SH_RD),
   OP_MASK_FMT2_OP | OP_MASK_SUBOP
     | OP_MASK_RDSEL | OP_MASK_RD,
   PRU_INSN_MACRO, unsigned_immed16_overflow},

  { "wbc", prui_qbbs, "s,b",
   OP_MATCH_QBBS | (0 << OP_SH_BROFF98) | (0 << OP_SH_BROFF70),
   OP_MASK_FMT5_OP | OP_MASK_BCMP | OP_MASK_BROFF,
   PRU_INSN_MACRO, qbranch_target_overflow},
  { "wbs", prui_qbbc, "s,b",
   OP_MATCH_QBBC | (0 << OP_SH_BROFF98) | (0 << OP_SH_BROFF70),
   OP_MASK_FMT5_OP | OP_MASK_BCMP | OP_MASK_BROFF,
   PRU_INSN_MACRO, qbranch_target_overflow},

  { "fill", prui_xin, "D,n",
   OP_MATCH_XIN | (254 << OP_SH_XFR_WBA),
   OP_MASK_XFR_OP | OP_MASK_XFR_WBA,
   PRU_INSN_MACRO, unsigned_immed8_overflow},
  { "zero", prui_xin, "D,n",
   OP_MATCH_XIN | (255 << OP_SH_XFR_WBA),
   OP_MASK_XFR_OP | OP_MASK_XFR_WBA,
   PRU_INSN_MACRO, unsigned_immed8_overflow},

  { "ldi32", prui_ldi, "R,i",
   OP_MATCH_LDI, OP_MASK_FMT2_OP | OP_MASK_SUBOP,
   PRU_INSN_LDI32, unsigned_immed32_overflow},
};

#define PRU_NUM_OPCODES \
       ((sizeof pru_opcodes) / (sizeof (pru_opcodes[0])))
const int bfd_pru_num_opcodes = PRU_NUM_OPCODES;

#undef PRU_NUM_OPCODES
