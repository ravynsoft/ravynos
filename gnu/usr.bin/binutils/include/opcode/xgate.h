/* xgate.h -- Freescale XGATE opcode list
   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Written by Sean Keys (skeys@ipdatasys.com)

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
   MA 02110-1301, USA. */

#ifndef _OPCODE_XGATE_H
#define _OPCODE_XGATE_H

/* XGATE CCR flag definitions.  */
#define XGATE_N_BIT   0x08	/* XGN - Sign Flag */
#define XGATE_Z_BIT   0x04	/* XGZ - Zero Flag */
#define XGATE_V_BIT   0x02	/* XGV - Overflow Flag */
#define XGATE_C_BIT   0x01	/* XGC - Carry Flag */

/* Access Detail Notation
   V - Vector fetch: always an aligned word read, lasts for at least one RISC core cycle
   P - Program word fetch: always an aligned word read, lasts for at least one RISC core cycle
   r - 8-bit data read: lasts for at least one RISC core cycle
   R - 16-bit data read: lasts for at least one RISC core cycle
   w - 8-bit data write: lasts for at least one RISC core cycle
   W - 16-bit data write: lasts for at least one RISC core cycle
   A - Alignment cycle: no read or write, lasts for zero or one RISC core cycles
   f - Free cycle: no read or write, lasts for one RISC core cycles.  */
#define XGATE_CYCLE_V	0x01
#define XGATE_CYCLE_P	0x02
#define XGATE_CYCLE_r	0x04
#define XGATE_CYCLE_R	0x08
#define XGATE_CYCLE_w	0x10
#define XGATE_CYCLE_W	0x20
#define XGATE_CYCLE_A	0x40
#define XGATE_CYCLE_f	0x80

/* XGATE operand formats as stored in the XGATE_opcode table.
   They are only used by GAS to recognize operands.  */
#define XGATE_OP_INH                  ""  /* Inherent.  */
#define XGATE_OP_TRI		 "r,r,r"  /* Register followed by two registers.                    */
#define XGATE_OP_DYA		   "r,r"  /* Register followed by a register.                       */
#define XGATE_OP_IMM16            "r,if"  /* Register followed by 16-bit value.                    */
#define XGATE_OP_IMM8	          "r,i8"  /* Register followed by 8-bit value.                      */
#define XGATE_OP_IMM4             "r,i4"  /* Register followed by 4-bit value.                     */
#define XGATE_OP_IMM3	            "i3"  /* Register followed by 3-bit value.                      */
#define XGATE_OP_MON		     "r"  /* Single register.                                       */
#define XGATE_OP_MON_R_C	   "r,c"  /* General register followed by ccr register.             */
#define XGATE_OP_MON_C_R	   "c,r"  /* CCR register followed by a general register.           */
#define XGATE_OP_MON_R_P	   "r,p"  /* General register followed by pc register.              */
#define XGATE_OP_IDR		 "r,r,+"  /* Three registers with the third having a -/+ directive. */
#define XGATE_OP_IDO5	        "r,r,i5"  /* Two general registers followed by an immediate value.  */
#define XGATE_OP_REL9	            "b9"  /* 9-bit value that is relative to the current pc.        */
#define XGATE_OP_REL10	            "ba"  /* 10-bit value that is relative to the current pc.       */
#define XGATE_OP_DYA_MON	    "=r"
/* Macro definitions.  */
#define XGATE_OP_IMM16mADD    "r,if; addl addh"
#define XGATE_OP_IMM16mAND    "r,if; andl andh"
#define XGATE_OP_IMM16mCPC    "r,if; cmpl cpch"
#define XGATE_OP_IMM16mSUB    "r,if; subl subh"
#define XGATE_OP_IMM16mLDW    "r,if; ldl ldh"

/* CPU variant identification.  */
#define XGATE_V1 0x1
#define XGATE_V2 0x2
#define XGATE_V3 0x4

/* The opcode table definitions.  */
struct xgate_opcode
{
  char * name;                  /* Op-code name.  */
  char * constraints;           /* Constraint chars.  */
  char * format;                /* Bit definitions.  */
  unsigned int size;            /* Opcode size in bytes.  */
  unsigned int bin_opcode;      /* Binary opcode with operands masked off.  */
  unsigned char cycles_min;     /* Minimum cpu cycles needed.  */
  unsigned char cycles_max;     /* Maximum cpu cycles needed.  */
  unsigned char set_flags_mask; /* CCR flags set.  */
  unsigned char clr_flags_mask; /* CCR flags cleared.  */
  unsigned char chg_flags_mask; /* CCR flags changed.  */
  unsigned char arch;           /* CPU variant.  */
};

/* The opcode table.  The table contains all the opcodes (all pages).
   You can't rely on the order.  */
extern const struct xgate_opcode xgate_opcodes[];
extern const int xgate_num_opcodes;

#endif /* _OPCODE_XGATE_H */
