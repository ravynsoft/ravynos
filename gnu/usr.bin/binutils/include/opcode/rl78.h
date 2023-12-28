/* Opcode decoder for the Renesas RL78
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Written by DJ Delorie <dj@redhat.com>

   This file is part of GDB, the GNU Debugger and GAS, the GNU Assembler.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* The RL78 decoder in libopcodes is used by the simulator, gdb's
   analyzer, and the disassembler.  Given an opcode data source, it
   decodes the next opcode into the following structures.  */

#ifndef RL78_OPCODES_H_INCLUDED
#define RL78_OPCODES_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  RL78_ISA_DEFAULT,
  RL78_ISA_G10,
  RL78_ISA_G13,
  RL78_ISA_G14,
} RL78_Dis_Isa;

/* For the purposes of these structures, the RL78 registers are as
   follows, despite most of these being memory-mapped and
   bank-switched:  */
typedef enum {
  RL78_Reg_None,
  /* The order of these matches the encodings.  */
  RL78_Reg_X,
  RL78_Reg_A,
  RL78_Reg_C,
  RL78_Reg_B,
  RL78_Reg_E,
  RL78_Reg_D,
  RL78_Reg_L,
  RL78_Reg_H,
  /* The order of these matches the encodings.  */
  RL78_Reg_AX,
  RL78_Reg_BC,
  RL78_Reg_DE,
  RL78_Reg_HL,
  /* Unordered.  */
  RL78_Reg_SP,
  RL78_Reg_PSW,
  RL78_Reg_CS,
  RL78_Reg_ES,
  RL78_Reg_PMC,
  RL78_Reg_MEM
} RL78_Register;

typedef enum
{
  RL78_Byte = 0,
  RL78_Word
} RL78_Size;

typedef enum {
  RL78_Condition_T,
  RL78_Condition_F,
  RL78_Condition_C,
  RL78_Condition_NC,
  RL78_Condition_H,
  RL78_Condition_NH,
  RL78_Condition_Z,
  RL78_Condition_NZ
} RL78_Condition;

typedef enum {
  RL78_Operand_None = 0,
  RL78_Operand_Immediate,	/* #addend */
  RL78_Operand_Register,	/* reg */
  RL78_Operand_Indirect,	/* [reg + reg2 + addend] */
  RL78_Operand_Bit,		/* reg.bit */
  RL78_Operand_BitIndirect,	/* [reg+reg2+addend].bit */
  RL78_Operand_PreDec,		/* [--reg] = push */
  RL78_Operand_PostInc		/* [reg++] = pop */
} RL78_Operand_Type;

typedef enum
{
  RLO_unknown,
  RLO_add,			/* d += s */
  RLO_addc,			/* d += s + CY */
  RLO_and,			/* d &= s (byte, word, bit) */
  RLO_branch,			/* pc = d */
  RLO_branch_cond,		/* pc = d if cond(src) */
  RLO_branch_cond_clear,	/* pc = d if cond(src), and clear(src) */
  RLO_break,			/* BRK */
  RLO_call,			/* call */
  RLO_cmp,			/* cmp d, s */
  RLO_divhu,			/* DIVHU */
  RLO_divwu,			/* DIVWU */
  RLO_halt,			/* HALT */
  RLO_mov,			/* d = s */
  RLO_mach,			/* MACH */
  RLO_machu,			/* MACHU */
  RLO_mulu,			/* MULU */
  RLO_mulh,			/* MULH */
  RLO_mulhu,			/* MULHU */
  RLO_nop,			/* NOP */
  RLO_or,			/* d |= s */
  RLO_ret,			/* RET */
  RLO_reti,			/* RETI */
  RLO_rol,			/* d <<= s, MSB to LSB and CY */
  RLO_rolc,			/* d <<= s, MSB to CY, CY, to LSB */
  RLO_ror,			/* d >>= s, LSB to MSB and CY */
  RLO_rorc,			/* d >>= s, LSB to CY, CY, to MSB */
  RLO_sar,			/* d >>= s, signed */
  RLO_sel,			/* rb = s */
  RLO_shr,			/* d >>= s, unsigned */
  RLO_shl,			/* d <<= s */
  RLO_skip,			/* skip next insn is cond(s) */
  RLO_stop,			/* STOP */
  RLO_sub,			/* d -= s */
  RLO_subc,			/* d -= s - CY */
  RLO_xch,			/* swap d, s  */
  RLO_xor,			/* d ^= s */
} RL78_Opcode_ID;

typedef struct {
  RL78_Operand_Type  type;
  int              addend;
  RL78_Register	   reg : 8;
  RL78_Register	   reg2 : 8;
  unsigned char	   bit_number : 4;
  unsigned char	   condition : 3;
  unsigned char	   use_es : 1;
} RL78_Opcode_Operand;

/* PSW flag bits */
#define RL78_PSW_IE	0x80
#define RL78_PSW_Z	0x40
#define RL78_PSW_RBS1	0x20
#define RL78_PSW_AC	0x10
#define	RL78_PSW_RBS0	0x08
#define	RL78_PSW_ISP1	0x04
#define	RL78_PSW_ISP0	0x02
#define RL78_PSW_CY	0x01

#define	RL78_SFR_SP	0xffff8
#define	RL78_SFR_PSW	0xffffa
#define	RL78_SFR_CS	0xffffc
#define	RL78_SFR_ES	0xffffd
#define	RL78_SFR_PMC	0xffffe
#define	RL78_SFR_MEM	0xfffff

typedef struct
{
  int lineno;
  RL78_Opcode_ID	id:24;
  unsigned		flags:8; /* PSW mask, for side effects only */
  int			n_bytes;
  char *		syntax;
  RL78_Size		size;
  /* By convention, these are destination, source.  */
  RL78_Opcode_Operand	op[2];
} RL78_Opcode_Decoded;

int rl78_decode_opcode (unsigned long, RL78_Opcode_Decoded *, int (*)(void *), void *, RL78_Dis_Isa);

#ifdef __cplusplus
}
#endif

#endif
