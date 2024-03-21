/* TI PRU opcode list for GAS, the GNU assembler.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   Contributed by Dimitar Dimitrov <dimitar@dinux.eu>

   This file is part of the GNU opcodes library.

   GAS/GDB is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS/GDB is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS or GDB; see the file COPYING3.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#ifndef _PRU_H_
#define _PRU_H_

#include "bfd.h"

/****************************************************************************
 * This file contains structures, bit masks and shift counts used
 * by the GNU toolchain to define the PRU instruction set and
 * access various opcode fields.
 ****************************************************************************/

/* Identify different overflow situations for error messages.  */
enum overflow_type
{
  call_target_overflow = 0,
  qbranch_target_overflow,
  address_offset_overflow,
  signed_immed16_overflow,
  unsigned_immed32_overflow,
  unsigned_immed16_overflow,
  unsigned_immed8_overflow,
  unsigned_immed5_overflow,
  no_overflow
};

enum opcode_format_type
{
  opcode_format1,
  opcode_format2ab,
  opcode_format2abl,
  opcode_format2c,
  opcode_format2de,
  opcode_format45,
  opcode_format6
};

/* Opcode ID listing. Used for indexing by the simulator.  */
enum pru_instr_type
{
  prui_add, prui_adc, prui_sub, prui_suc, prui_lsl, prui_lsr, prui_rsb,
  prui_rsc, prui_and, prui_or,  prui_xor, prui_min, prui_max, prui_clr,
  prui_set, prui_not, prui_jmp, prui_jal, prui_ldi, prui_lmbd,
  prui_halt, prui_slp, prui_xin, prui_xout, prui_xchg, prui_sxin,
  prui_sxout, prui_sxchg, prui_loop, prui_iloop, prui_qbgt, prui_qbge,
  prui_qblt, prui_qble, prui_qbeq, prui_qbne, prui_qba, prui_qbbs,
  prui_qbbc, prui_lbbo, prui_sbbo, prui_lbco, prui_sbco
};

/* This structure holds information for a particular instruction.

   The args field is a string describing the operands.  The following
   letters can appear in the args:
     b - a 5.3-bit right source register index OR 8-bit unsigned immediate
     B - same as 'b', but for LOOP instruction where IMM is decremented
     c - a 5 bit unsigned immediate for constant table offset
     d - a 5.3-bit destination register index
     D - a 5.2-bit destination register index
     E - for internal GAS self-tests only
     i - a 32-bit immediate or label
     j - a 5.3-bit right source register index OR 18-bit PC address
     l - burst length (unsigned 7-bit immediate or r0.b[0-3]) for xLBCO
     n - burst length (unsigned 7-bit immediate or r0.b[0-3]) for XFR
     o - a 10-bit signed PC-relative offset
     O - an 8-bit unsigned PC-relative offset for LOOP termination point
     R - a 5-bit destination register index
     s - a 5.3-bit left source register index
     S - a 5-bit left source register index
     w - a single bit for "WakeOnStatus"
     W - a 16-bit unsigned immediate with IO=0 field (LDI)
     x - an 8-bit XFR wide-bus address immediate
   Literal ',' character may also appear in the args as delimiter.

   Most of the macro names are from [1].

   The pinfo field is INSN_MACRO for a macro.  Otherwise, it is a collection
   of bits describing the instruction, notably any relevant hazard
   information.

   When assembling, the match field contains the opcode template, which
   is modified by the arguments to produce the actual opcode
   that is emitted.  If pinfo is INSN_MACRO, then this is 0.

   If pinfo is INSN_MACRO, the mask field stores the macro identifier.
   Otherwise this is a bit mask for the relevant portions of the opcode
   when disassembling.  If the actual opcode anded with the match field
   equals the opcode field, then we have found the correct instruction.

  [1] http://processors.wiki.ti.com/index.php/Programmable_Realtime_Unit  */

struct pru_opcode
{
  const char *name;		/* The name of the instruction.  */
  enum pru_instr_type type;	/* Instruction type. Used for fast indexing
				   by the simulator.  */
  const char *args;		/* A string describing the arguments for this
				   instruction.  */
  unsigned long match;		/* The basic opcode for the instruction.  */
  unsigned long mask;		/* Mask for the opcode field of the
				   instruction.  */
  unsigned long pinfo;		/* Is this a real instruction or instruction
				   macro?  */
  enum overflow_type overflow_msg;  /* Used to generate informative
				       message when fixup overflows.  */
};

/* This value is used in the pru_opcode.pinfo field to indicate that the
   instruction is a macro or pseudo-op.  This requires special treatment by
   the assembler, and is used by the disassembler to determine whether to
   check for a nop.  */
#define PRU_INSN_MACRO		0x80000000

/* This macro is specially handled throughout the code because it is
   the only insn to output 2 words (64 bits). */
#define PRU_INSN_LDI32		0x40000000

/* Associates a register name with a 5-bit index and 3-bit regsel.  */
struct pru_reg
{
  const char *name;		/* Name, e.g. "r10".  */
  const unsigned int index;	/* Index, e.g. 10.  */
  const unsigned int regsel;	/* Register field selector, .e.g RSEL_31_0.  */
};

/* Macros for getting and setting an instruction field.  */
#define GET_INSN_FIELD(X, i) \
  (((i) & OP_MASK_##X) >> OP_SH_##X)
#define SET_INSN_FIELD(X, i, v) \
  ((i) = (((i) & ~OP_MASK_##X) | (((v) << OP_SH_##X) & OP_MASK_##X)))

#define CHECK_INSN_FIELD(X, i) \
  (((i) & OP_MASK_##X) == OP_MATCH_##X)

/* Masks, values, shifts and macros for accessing the various opcode fields.  */

#define OP_SH_FMT1_OP			29
#define OP_MASK_FMT1_OP			(0x7u << 29)
#define OP_MATCH_FMT1_OP		(0x0u << 29)

#define OP_SH_FMT2_OP			29
#define OP_MASK_FMT2_OP			(0x7u << 29)
#define OP_MATCH_FMT2_OP		(0x1u << 29)

#define OP_SH_FMT4_OP			30
#define OP_MASK_FMT4_OP			(0x3u << 30)
#define OP_MATCH_FMT4_OP		(0x1u << 30)

#define OP_SH_FMT5_OP			29
#define OP_MASK_FMT5_OP			(0x7u << 29)
#define OP_MATCH_FMT5_OP		(0x6u << 29)

#define OP_SH_FMT6AB_OP			29
#define OP_MASK_FMT6AB_OP		(0x7u << 29)
#define OP_MATCH_FMT6AB_OP		(0x7u << 29)

#define OP_SH_FMT6CD_OP			29
#define OP_MASK_FMT6CD_OP		(0x7u << 29)
#define OP_MATCH_FMT6CD_OP		(0x4u << 29)

/* Generic fields.  */
#define OP_SH_SUBOP			25
#define OP_MASK_SUBOP			(0xfu << 25)

#define OP_SH_IO			24
#define OP_MASK_IO			(0x1u << 24)

#define OP_SH_RS2SEL			21
#define OP_MASK_RS2SEL			(0x7u << 21)
#define OP_SH_RS2			16
#define OP_MASK_RS2			(0x1fu << 16)
#define OP_SH_RS1SEL			13
#define OP_MASK_RS1SEL			(0x7u << 13)
#define OP_SH_RS1			8
#define OP_MASK_RS1			(0x1fu << 8)
#define OP_SH_RDSEL			5
#define OP_MASK_RDSEL			(0x7u << 5)
#define OP_SH_RD			0
#define OP_MASK_RD			(0x1fu << 0)
#define OP_SH_IMM8			16
#define OP_MASK_IMM8			(0xffu << 16)
#define OP_SH_IMM16			8
#define OP_MASK_IMM16			(0xffffu << 8)

#define RSEL_7_0			0u
#define RSEL_15_8			1u
#define RSEL_23_16			2u
#define RSEL_31_24			3u
#define RSEL_15_0			4u
#define RSEL_23_8			5u
#define RSEL_31_16			6u
#define RSEL_31_0			7u
#define RSEL_NUM_ITEMS			8u

/* Format 1 specific fields.  */
#define SUBOP_ADD			0u
#define SUBOP_ADC			1u
#define SUBOP_SUB			2u
#define SUBOP_SUC			3u
#define SUBOP_LSL			4u
#define SUBOP_LSR			5u
#define SUBOP_RSB			6u
#define SUBOP_RSC			7u
#define SUBOP_AND			8u
#define SUBOP_OR			9u
#define SUBOP_XOR			10u
#define SUBOP_NOT			11u
#define SUBOP_MIN			12u
#define SUBOP_MAX			13u
#define SUBOP_CLR			14u
#define SUBOP_SET			15u

/* Format 2 specific fields.  */
#define SUBOP_JMP			0u
#define SUBOP_JAL			1u
#define SUBOP_LDI			2u
#define SUBOP_LMBD			3u
#define SUBOP_SCAN			4u
#define SUBOP_HALT			5u
#define SUBOP_RSVD_FOR_MVIx		6u
#define SUBOP_XFR			7u
#define SUBOP_LOOP			8u
#define SUBOP_RSVD_FOR_RFI		14u
#define SUBOP_SLP			15u

#define OP_SH_WAKEONSTATUS		23
#define OP_MASK_WAKEONSTATUS		(0x1u << 23)

/* Format 2 XFR specific fields.  */
#define OP_SH_SUBOP_XFR			23
#define OP_MASK_SUBOP_XFR		(3u << 23)
#define OP_SH_XFR_WBA			15
#define OP_MASK_XFR_WBA			(0xffu << 15)
#define OP_SH_XFR_S			14
#define OP_MASK_XFR_S			(1u << 14)
#define OP_SH_XFR_LENGTH		7
#define OP_MASK_XFR_LENGTH		(0x7fu << 7)

#define SUBOP_XFR_XIN			1u
#define SUBOP_XFR_XOUT			2u
#define SUBOP_XFR_XCHG			3u

/* Format 2 LOOP specific fields.  */
#define OP_SH_LOOP_INTERRUPTIBLE	15
#define OP_MASK_LOOP_INTERRUPTIBLE	(1u << 15)
#define OP_SH_LOOP_JMPOFFS		0
#define OP_MASK_LOOP_JMPOFFS		(0xffu << 0)

/* Format 4 specific fields.  */
#define OP_SH_BROFF98			25
#define OP_MASK_BROFF98			(0x3u << 25)
#define OP_SH_BROFF70			0
#define OP_MASK_BROFF70			(0xffu << 0)
#define OP_SH_GT			29
#define OP_MASK_GT			(0x1u << 29)
#define OP_SH_EQ			28
#define OP_MASK_EQ			(0x1u << 28)
#define OP_SH_LT			27
#define OP_MASK_LT			(0x1u << 27)
#define OP_MASK_CMP			(OP_MASK_GT | OP_MASK_EQ | OP_MASK_LT)


/* Format 5 specific fields.  */
#define OP_SH_BS			28
#define OP_MASK_BS			(0x1u << 28)
#define OP_SH_BC			27
#define OP_MASK_BC			(0x1u << 27)
#define OP_MASK_BCMP			(OP_MASK_BS | OP_MASK_BC)

/* Format 6 specific fields.  */
#define OP_SH_LOADSTORE			28
#define OP_MASK_LOADSTORE		(0x1u << 28)
#define OP_SH_BURSTLEN64		25
#define OP_MASK_BURSTLEN64		(0x7u << 25)
#define OP_SH_BURSTLEN31		13
#define OP_MASK_BURSTLEN31		(0x7u << 13)
#define OP_SH_CB			8
#define OP_MASK_CB			(0x1fu << 8)
#define OP_SH_BURSTLEN0			7
#define OP_MASK_BURSTLEN0		(0x1u << 7)
#define OP_SH_RDB			5
#define OP_MASK_RDB			(0x3u << 5)

#define LSSBBO_BYTECOUNT_R0_BITS7_0	124u
#define LSBBO_BYTECOUNT_R0_BITS15_8	125u
#define LSBBO_BYTECOUNT_R0_BITS23_16	126u
#define LSBBO_BYTECOUNT_R0_BITS31_24	127u

/* The following macros define the opcode matches for each
   instruction code & OP_MASK_INST == OP_MATCH_INST.  */
#define OP_MATCH_ADD	(OP_MATCH_FMT1_OP | (SUBOP_ADD << OP_SH_SUBOP))
#define OP_MATCH_ADC	(OP_MATCH_FMT1_OP | (SUBOP_ADC << OP_SH_SUBOP))
#define OP_MATCH_SUB	(OP_MATCH_FMT1_OP | (SUBOP_SUB << OP_SH_SUBOP))
#define OP_MATCH_SUC	(OP_MATCH_FMT1_OP | (SUBOP_SUC << OP_SH_SUBOP))
#define OP_MATCH_LSL	(OP_MATCH_FMT1_OP | (SUBOP_LSL << OP_SH_SUBOP))
#define OP_MATCH_LSR	(OP_MATCH_FMT1_OP | (SUBOP_LSR << OP_SH_SUBOP))
#define OP_MATCH_RSB	(OP_MATCH_FMT1_OP | (SUBOP_RSB << OP_SH_SUBOP))
#define OP_MATCH_RSC	(OP_MATCH_FMT1_OP | (SUBOP_RSC << OP_SH_SUBOP))
#define OP_MATCH_AND	(OP_MATCH_FMT1_OP | (SUBOP_AND << OP_SH_SUBOP))
#define OP_MATCH_OR	(OP_MATCH_FMT1_OP | (SUBOP_OR << OP_SH_SUBOP))
#define OP_MATCH_XOR	(OP_MATCH_FMT1_OP | (SUBOP_XOR << OP_SH_SUBOP))
#define OP_MATCH_NOT	(OP_MATCH_FMT1_OP | (SUBOP_NOT << OP_SH_SUBOP))
#define OP_MATCH_MIN	(OP_MATCH_FMT1_OP | (SUBOP_MIN << OP_SH_SUBOP))
#define OP_MATCH_MAX	(OP_MATCH_FMT1_OP | (SUBOP_MAX << OP_SH_SUBOP))
#define OP_MATCH_CLR	(OP_MATCH_FMT1_OP | (SUBOP_CLR << OP_SH_SUBOP))
#define OP_MATCH_SET	(OP_MATCH_FMT1_OP | (SUBOP_SET << OP_SH_SUBOP))

#define OP_MATCH_JMP	(OP_MATCH_FMT2_OP | (SUBOP_JMP << OP_SH_SUBOP))
#define OP_MATCH_JAL	(OP_MATCH_FMT2_OP | (SUBOP_JAL << OP_SH_SUBOP))
#define OP_MATCH_LDI	(OP_MATCH_FMT2_OP | (SUBOP_LDI << OP_SH_SUBOP))
#define OP_MATCH_LMBD	(OP_MATCH_FMT2_OP | (SUBOP_LMBD << OP_SH_SUBOP))
#define OP_MATCH_SCAN	(OP_MATCH_FMT2_OP | (SUBOP_SCAN << OP_SH_SUBOP))
#define OP_MATCH_HALT	(OP_MATCH_FMT2_OP | (SUBOP_HALT << OP_SH_SUBOP))
#define OP_MATCH_SLP	(OP_MATCH_FMT2_OP | (SUBOP_SLP << OP_SH_SUBOP))
#define OP_MATCH_XFR	(OP_MATCH_FMT2_OP | (SUBOP_XFR << OP_SH_SUBOP))
#define OP_MATCH_SXFR	(OP_MATCH_XFR | OP_MASK_XFR_S)
#define OP_MATCH_XIN	(OP_MATCH_XFR | (SUBOP_XFR_XIN << OP_SH_SUBOP_XFR))
#define OP_MATCH_XOUT	(OP_MATCH_XFR | (SUBOP_XFR_XOUT << OP_SH_SUBOP_XFR))
#define OP_MATCH_XCHG	(OP_MATCH_XFR | (SUBOP_XFR_XCHG << OP_SH_SUBOP_XFR))
#define OP_MATCH_SXIN	(OP_MATCH_SXFR | (SUBOP_XFR_XIN << OP_SH_SUBOP_XFR))
#define OP_MATCH_SXOUT	(OP_MATCH_SXFR | (SUBOP_XFR_XOUT << OP_SH_SUBOP_XFR))
#define OP_MATCH_SXCHG	(OP_MATCH_SXFR | (SUBOP_XFR_XCHG << OP_SH_SUBOP_XFR))
#define OP_MATCH_LOOP	(OP_MATCH_FMT2_OP | (SUBOP_LOOP << OP_SH_SUBOP))
#define OP_MATCH_ILOOP	(OP_MATCH_FMT2_OP | (SUBOP_LOOP << OP_SH_SUBOP) \
			 | OP_MASK_LOOP_INTERRUPTIBLE)

#define OP_MATCH_QBGT	(OP_MATCH_FMT4_OP | OP_MASK_GT)
#define OP_MATCH_QBGE	(OP_MATCH_FMT4_OP | OP_MASK_GT | OP_MASK_EQ)
#define OP_MATCH_QBLT	(OP_MATCH_FMT4_OP | OP_MASK_LT)
#define OP_MATCH_QBLE	(OP_MATCH_FMT4_OP | OP_MASK_LT | OP_MASK_EQ)
#define OP_MATCH_QBEQ	(OP_MATCH_FMT4_OP | OP_MASK_EQ)
#define OP_MATCH_QBNE	(OP_MATCH_FMT4_OP | OP_MASK_GT | OP_MASK_LT)
#define OP_MATCH_QBA	(OP_MATCH_FMT4_OP | OP_MASK_GT | OP_MASK_LT \
			 | OP_MASK_EQ)

#define OP_MATCH_QBBS	(OP_MATCH_FMT5_OP | OP_MASK_BS)
#define OP_MATCH_QBBC	(OP_MATCH_FMT5_OP | OP_MASK_BC)

#define OP_MATCH_LBBO	(OP_MATCH_FMT6AB_OP | OP_MASK_LOADSTORE)
#define OP_MATCH_SBBO	(OP_MATCH_FMT6AB_OP)
#define OP_MATCH_LBCO	(OP_MATCH_FMT6CD_OP | OP_MASK_LOADSTORE)
#define OP_MATCH_SBCO	(OP_MATCH_FMT6CD_OP)

/* Some special extractions.  */
#define OP_MASK_BROFF		  (OP_MASK_BROFF98 | OP_MASK_BROFF70)

#define GET_BROFF_URAW(i)	  \
  ((GET_INSN_FIELD (BROFF98, i) << 8) | (GET_INSN_FIELD (BROFF70, i) << 0))

#define GET_BROFF_SIGNED(i)	  \
  ((long)(GET_BROFF_URAW (i) - (!!(GET_BROFF_URAW (i) & (1 << 9)) << 10)))

#define SET_BROFF_URAW(i, v)		      \
  do {					      \
      SET_INSN_FIELD (BROFF98, (i), (v) >> 8);    \
      SET_INSN_FIELD (BROFF70, (i), (v) & 0xff);  \
  } while (0)

#define GET_BURSTLEN(i)	  \
  ( (GET_INSN_FIELD (BURSTLEN64, (i)) << 4) |   \
    (GET_INSN_FIELD (BURSTLEN31, (i)) << 1) |   \
    (GET_INSN_FIELD (BURSTLEN0, (i)) << 0))

#define SET_BURSTLEN(i, v)		      \
  do {					      \
      SET_INSN_FIELD (BURSTLEN64, (i), (v) >> 4); \
      SET_INSN_FIELD (BURSTLEN31, (i), (v) >> 1); \
      SET_INSN_FIELD (BURSTLEN0, (i), (v) >> 0);  \
  } while (0)

/* Miscellaneous helpers.  */
#define OP_MASK_XFR_OP		(OP_MASK_FMT2_OP | OP_MASK_SUBOP \
				 | OP_MASK_SUBOP_XFR | OP_MASK_XFR_S)

#define OP_MASK_LOOP_OP		(OP_MASK_FMT2_OP | OP_MASK_SUBOP \
				 | OP_MASK_LOOP_INTERRUPTIBLE)

/* These are the data structures we use to hold the instruction information.  */
extern const struct pru_opcode pru_opcodes[];
extern const int bfd_pru_num_opcodes;

/* These are the data structures used to hold the register information.  */
extern const struct pru_reg pru_regs[];
extern const int pru_num_regs;

/* Machine-independent macro for number of opcodes.  */
#define NUMOPCODES bfd_pru_num_opcodes
#define NUMREGISTERS pru_num_regs;

/* This is made extern so that the assembler can use it to find out
   what instruction caused an error.  */
extern const struct pru_opcode *pru_find_opcode (unsigned long);

#endif /* _PRU_H */
