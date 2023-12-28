/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Instruction opcode table for epiphany.

THIS FILE IS MACHINE GENERATED WITH CGEN.

Copyright (C) 1996-2023 Free Software Foundation, Inc.

This file is part of the GNU Binutils and/or GDB, the GNU debugger.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.

*/

#include "sysdep.h"
#include "ansidecl.h"
#include "bfd.h"
#include "symcat.h"
#include "epiphany-desc.h"
#include "epiphany-opc.h"
#include "libiberty.h"

/* -- opc.c */



/* -- asm.c */
/* The hash functions are recorded here to help keep assembler code out of
   the disassembler and vice versa.  */

static int asm_hash_insn_p        (const CGEN_INSN *);
static unsigned int asm_hash_insn (const char *);
static int dis_hash_insn_p        (const CGEN_INSN *);
static unsigned int dis_hash_insn (const char *, CGEN_INSN_INT);

/* Instruction formats.  */

#define F(f) & epiphany_cgen_ifld_table[EPIPHANY_##f]
static const CGEN_IFMT ifmt_empty ATTRIBUTE_UNUSED = {
  0, 0, 0x0, { { 0 } }
};

static const CGEN_IFMT ifmt_beq16 ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_SIMM8) }, { F (F_CONDCODE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_beq ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_SIMM24) }, { F (F_CONDCODE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_jr16 ATTRIBUTE_UNUSED = {
  16, 16, 0xe3ff, { { F (F_DC_15_3) }, { F (F_RN) }, { F (F_DC_9_1) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_rts ATTRIBUTE_UNUSED = {
  32, 32, 0xffffffff, { { F (F_DC_31_3) }, { F (F_RN_X) }, { F (F_DC_25_6) }, { F (F_OPC_19_4) }, { F (F_DC_15_3) }, { F (F_RN) }, { F (F_DC_9_1) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_jr ATTRIBUTE_UNUSED = {
  32, 32, 0xe3ffe3ff, { { F (F_DC_31_3) }, { F (F_DC_25_6) }, { F (F_OPC_19_4) }, { F (F_DC_15_3) }, { F (F_RN6) }, { F (F_DC_9_1) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbx16_s ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_RD) }, { F (F_RN) }, { F (F_RM) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbx_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_ADDSUBX) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbp_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_DC_22_2) }, { F (F_ADDSUBX) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbd16_s ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbd_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_PM) }, { F (F_SUBD) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_DISP11) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov16EQ ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_RD) }, { F (F_RN) }, { F (F_DC_9_1) }, { F (F_OPC_8_1) }, { F (F_CONDCODE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmovEQ ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_DC_25_6) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_DC_9_1) }, { F (F_OPC_8_1) }, { F (F_CONDCODE) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_movts16 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_RD) }, { F (F_SN) }, { F (F_DC_9_1) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_movts6 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_SN6) }, { F (F_DC_9_1) }, { F (F_OPC_8_1) }, { F (F_DC_7_4) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_movtsdma ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_SN6) }, { F (F_DC_9_1) }, { F (F_OPC_8_1) }, { F (F_DC_7_4) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_movtsmem ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_SN6) }, { F (F_DC_9_1) }, { F (F_OPC_8_1) }, { F (F_DC_7_4) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_movtsmesh ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_SN6) }, { F (F_DC_9_1) }, { F (F_OPC_8_1) }, { F (F_DC_7_4) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_nop ATTRIBUTE_UNUSED = {
  16, 16, 0xffff, { { F (F_DC_15_7) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_unimpl ATTRIBUTE_UNUSED = {
  32, 32, 0xffffffff, { { F (F_OPC_31_32) }, { 0 } }
};

static const CGEN_IFMT ifmt_gien ATTRIBUTE_UNUSED = {
  16, 16, 0xffff, { { F (F_DC_15_6) }, { F (F_GIEN_GIDIS_9_1) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_swi_num ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_TRAP_NUM) }, { F (F_TRAP_SWI_9_1) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_swi ATTRIBUTE_UNUSED = {
  16, 16, 0xffff, { { F (F_DC_15_6) }, { F (F_TRAP_SWI_9_1) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_trap16 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_TRAP_NUM) }, { F (F_TRAP_SWI_9_1) }, { F (F_OPC_8_5) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_add16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_RD) }, { F (F_RN) }, { F (F_RM) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_add ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_DC_22_3) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_addi16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_RD) }, { F (F_RN) }, { F (F_SDISP3) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_addi ATTRIBUTE_UNUSED = {
  32, 32, 0x300007f, { { F (F_DC_25_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SDISP11) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_lsri16 ATTRIBUTE_UNUSED = {
  16, 16, 0x1f, { { F (F_RD) }, { F (F_RN) }, { F (F_SHIFT) }, { F (F_OPC_4_1) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_lsri32 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff001f, { { F (F_DC_25_6) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SHIFT) }, { F (F_OPC_4_1) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_bitr16 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_RD) }, { F (F_RN) }, { F (F_SHIFT) }, { F (F_OPC_4_1) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_bitr ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_DC_25_6) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SHIFT) }, { F (F_OPC_4_1) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_fext ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_DC_22_2) }, { F (F_DC_20_1) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_mov8 ATTRIBUTE_UNUSED = {
  16, 16, 0x1f, { { F (F_RD) }, { F (F_IMM8) }, { F (F_OPC_4_1) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_mov16 ATTRIBUTE_UNUSED = {
  32, 32, 0x100f001f, { { F (F_DC_28_1) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_IMM16) }, { F (F_OPC_4_1) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_absf16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_RD) }, { F (F_RN) }, { F (F_RN) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_absf32 ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_DC_22_3) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RN6) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_loatf16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_RD) }, { F (F_RN) }, { F (F_RN) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_recipf32 ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_DC_22_2) }, { F (F_DC_20_1) }, { F (F_OPC_19_4) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RN6) }, { F (F_OPC_6_3) }, { F (F_OPC) }, { 0 } }
};

#undef F

#define A(a) (1 << CGEN_INSN_##a)
#define OPERAND(op) EPIPHANY_OPERAND_##op
#define MNEM CGEN_SYNTAX_MNEMONIC /* syntax value for mnemonic */
#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))

/* The instruction table.  */

static const CGEN_OPCODE epiphany_cgen_insn_opcode_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { { 0, 0, 0, 0 }, {{0}}, 0, {0}},
/* beq.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x0 }
  },
/* beq.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x8 }
  },
/* bne.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x10 }
  },
/* bne.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x18 }
  },
/* bgtu.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x20 }
  },
/* bgtu.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x28 }
  },
/* bgteu.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x30 }
  },
/* bgteu.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x38 }
  },
/* blteu.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x40 }
  },
/* blteu.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x48 }
  },
/* bltu.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x50 }
  },
/* bltu.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x58 }
  },
/* bgt.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x60 }
  },
/* bgt.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x68 }
  },
/* bgte.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x70 }
  },
/* bgte.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x78 }
  },
/* blt.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x80 }
  },
/* blt.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x88 }
  },
/* blte.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0x90 }
  },
/* blte.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0x98 }
  },
/* bbeq.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0xa0 }
  },
/* bbeq.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0xa8 }
  },
/* bbne.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0xb0 }
  },
/* bbne.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0xb8 }
  },
/* bblt.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0xc0 }
  },
/* bblt.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0xc8 }
  },
/* bblte.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0xd0 }
  },
/* bblte.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0xd8 }
  },
/* b.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0xe0 }
  },
/* b.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0xe8 }
  },
/* bl.s $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16, { 0xf0 }
  },
/* bl.l $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq, { 0xf8 }
  },
/* jr $rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), 0 } },
    & ifmt_jr16, { 0x142 }
  },
/* rts */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_rts, { 0x402194f }
  },
/* jr $rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN6), 0 } },
    & ifmt_jr, { 0x2014f }
  },
/* jalr $rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), 0 } },
    & ifmt_jr16, { 0x152 }
  },
/* jalr $rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN6), 0 } },
    & ifmt_jr, { 0x2015f }
  },
/* ldrb $rd,[$rn,$rm] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (RM), ']', 0 } },
    & ifmt_ldrbx16_s, { 0x1 }
  },
/* ldrb $rd,[$rn],$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', ',', OP (RM), 0 } },
    & ifmt_ldrbx16_s, { 0x5 }
  },
/* ldrb $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x9 }
  },
/* ldrb $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp_l, { 0xd }
  },
/* ldrb $rd,[$rn,$disp3] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (DISP3), ']', 0 } },
    & ifmt_ldrbd16_s, { 0x4 }
  },
/* ldrb $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd_l, { 0xc }
  },
/* ldrb $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbd_l, { 0x200000c }
  },
/* ldrh $rd,[$rn,$rm] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (RM), ']', 0 } },
    & ifmt_ldrbx16_s, { 0x21 }
  },
/* ldrh $rd,[$rn],$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', ',', OP (RM), 0 } },
    & ifmt_ldrbx16_s, { 0x25 }
  },
/* ldrh $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x29 }
  },
/* ldrh $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp_l, { 0x2d }
  },
/* ldrh $rd,[$rn,$disp3] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (DISP3), ']', 0 } },
    & ifmt_ldrbd16_s, { 0x24 }
  },
/* ldrh $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd_l, { 0x2c }
  },
/* ldrh $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbd_l, { 0x200002c }
  },
/* ldr $rd,[$rn,$rm] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (RM), ']', 0 } },
    & ifmt_ldrbx16_s, { 0x41 }
  },
/* ldr $rd,[$rn],$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', ',', OP (RM), 0 } },
    & ifmt_ldrbx16_s, { 0x45 }
  },
/* ldr $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x49 }
  },
/* ldr $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp_l, { 0x4d }
  },
/* ldr $rd,[$rn,$disp3] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (DISP3), ']', 0 } },
    & ifmt_ldrbd16_s, { 0x44 }
  },
/* ldr $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd_l, { 0x4c }
  },
/* ldr $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbd_l, { 0x200004c }
  },
/* ldrd $rd,[$rn,$rm] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (RM), ']', 0 } },
    & ifmt_ldrbx16_s, { 0x61 }
  },
/* ldrd $rd,[$rn],$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', ',', OP (RM), 0 } },
    & ifmt_ldrbx16_s, { 0x65 }
  },
/* ldrd $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x69 }
  },
/* ldrd $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp_l, { 0x6d }
  },
/* ldrd $rd,[$rn,$disp3] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (DISP3), ']', 0 } },
    & ifmt_ldrbd16_s, { 0x64 }
  },
/* ldrd $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd_l, { 0x6c }
  },
/* ldrd $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbd_l, { 0x200006c }
  },
/* testsetb $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x200009 }
  },
/* testseth $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x200029 }
  },
/* testset $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x200049 }
  },
/* strb $rd,[$rn,$rm] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (RM), ']', 0 } },
    & ifmt_ldrbx16_s, { 0x11 }
  },
/* strb $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x19 }
  },
/* strb $rd,[$rn],$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', ',', OP (RM), 0 } },
    & ifmt_ldrbx16_s, { 0x15 }
  },
/* strb $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp_l, { 0x1d }
  },
/* strb $rd,[$rn,$disp3] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (DISP3), ']', 0 } },
    & ifmt_ldrbd16_s, { 0x14 }
  },
/* strb $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd_l, { 0x1c }
  },
/* strb $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbd_l, { 0x200001c }
  },
/* strh $rd,[$rn,$rm] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (RM), ']', 0 } },
    & ifmt_ldrbx16_s, { 0x31 }
  },
/* strh $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x39 }
  },
/* strh $rd,[$rn],$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', ',', OP (RM), 0 } },
    & ifmt_ldrbx16_s, { 0x35 }
  },
/* strh $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp_l, { 0x3d }
  },
/* strh $rd,[$rn,$disp3] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (DISP3), ']', 0 } },
    & ifmt_ldrbd16_s, { 0x34 }
  },
/* strh $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd_l, { 0x3c }
  },
/* strh $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbd_l, { 0x200003c }
  },
/* str $rd,[$rn,$rm] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (RM), ']', 0 } },
    & ifmt_ldrbx16_s, { 0x51 }
  },
/* str $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x59 }
  },
/* str $rd,[$rn],$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', ',', OP (RM), 0 } },
    & ifmt_ldrbx16_s, { 0x55 }
  },
/* str $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp_l, { 0x5d }
  },
/* str $rd,[$rn,$disp3] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (DISP3), ']', 0 } },
    & ifmt_ldrbd16_s, { 0x54 }
  },
/* str $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd_l, { 0x5c }
  },
/* str $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbd_l, { 0x200005c }
  },
/* strd $rd,[$rn,$rm] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (RM), ']', 0 } },
    & ifmt_ldrbx16_s, { 0x71 }
  },
/* strd $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx_l, { 0x79 }
  },
/* strd $rd,[$rn],$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', ',', OP (RM), 0 } },
    & ifmt_ldrbx16_s, { 0x75 }
  },
/* strd $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp_l, { 0x7d }
  },
/* strd $rd,[$rn,$disp3] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ',', OP (DISP3), ']', 0 } },
    & ifmt_ldrbd16_s, { 0x74 }
  },
/* strd $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd_l, { 0x7c }
  },
/* strd $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbd_l, { 0x200007c }
  },
/* moveq $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x2 }
  },
/* moveq $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2000f }
  },
/* movne $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x12 }
  },
/* movne $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2001f }
  },
/* movgtu $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x22 }
  },
/* movgtu $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2002f }
  },
/* movgteu $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x32 }
  },
/* movgteu $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2003f }
  },
/* movlteu $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x42 }
  },
/* movlteu $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2004f }
  },
/* movltu $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x52 }
  },
/* movltu $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2005f }
  },
/* movgt $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x62 }
  },
/* movgt $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2006f }
  },
/* movgte $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x72 }
  },
/* movgte $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2007f }
  },
/* movlt $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x82 }
  },
/* movlt $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2008f }
  },
/* movlte $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0x92 }
  },
/* movlte $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x2009f }
  },
/* mov $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0xe2 }
  },
/* mov $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x200ef }
  },
/* movbeq $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0xa2 }
  },
/* movbeq $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x200af }
  },
/* movbne $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0xb2 }
  },
/* movbne $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x200bf }
  },
/* movblt $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0xc2 }
  },
/* movblt $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x200cf }
  },
/* movblte $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_cmov16EQ, { 0xd2 }
  },
/* movblte $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmovEQ, { 0x200df }
  },
/* movts $sn,$rd */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SN), ',', OP (RD), 0 } },
    & ifmt_movts16, { 0x102 }
  },
/* movts $sn6,$rd6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SN6), ',', OP (RD6), 0 } },
    & ifmt_movts6, { 0x2010f }
  },
/* movts $sndma,$rd6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SNDMA), ',', OP (RD6), 0 } },
    & ifmt_movtsdma, { 0x12010f }
  },
/* movts $snmem,$rd6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SNMEM), ',', OP (RD6), 0 } },
    & ifmt_movtsmem, { 0x22010f }
  },
/* movts $snmesh,$rd6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SNMESH), ',', OP (RD6), 0 } },
    & ifmt_movtsmesh, { 0x32010f }
  },
/* movfs $rd,$sn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (SN), 0 } },
    & ifmt_movts16, { 0x112 }
  },
/* movfs $rd6,$sn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (SN6), 0 } },
    & ifmt_movts6, { 0x2011f }
  },
/* movfs $rd6,$sndma */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (SNDMA), 0 } },
    & ifmt_movtsdma, { 0x12011f }
  },
/* movfs $rd6,$snmem */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (SNMEM), 0 } },
    & ifmt_movtsmem, { 0x22011f }
  },
/* movfs $rd6,$snmesh */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (SNMESH), 0 } },
    & ifmt_movtsmesh, { 0x32011f }
  },
/* nop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x1a2 }
  },
/* snop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x3a2 }
  },
/* unimpl */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_unimpl, { 0xf000f }
  },
/* idle */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x1b2 }
  },
/* bkpt */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x1c2 }
  },
/* mbkpt */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x3c2 }
  },
/* rti */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x1d2 }
  },
/* wand */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x182 }
  },
/* sync */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x1f2 }
  },
/* gie */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_gien, { 0x192 }
  },
/* gid */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_gien, { 0x392 }
  },
/* swi $swi_num */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SWI_NUM), 0 } },
    & ifmt_swi_num, { 0x1e2 }
  },
/* swi */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_swi, { 0x1e2 }
  },
/* trap $trapnum6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (TRAPNUM6), 0 } },
    & ifmt_trap16, { 0x3e2 }
  },
/* add $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x1a }
  },
/* add $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0xa001f }
  },
/* sub $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x3a }
  },
/* sub $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0xa003f }
  },
/* and $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x5a }
  },
/* and $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0xa005f }
  },
/* orr $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x7a }
  },
/* orr $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0xa007f }
  },
/* eor $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0xa }
  },
/* eor $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0xa000f }
  },
/* add.s $rd,$rn,$simm3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (SIMM3), 0 } },
    & ifmt_addi16, { 0x13 }
  },
/* add.l $rd6,$rn6,$simm11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SIMM11), 0 } },
    & ifmt_addi, { 0x1b }
  },
/* sub.s $rd,$rn,$simm3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (SIMM3), 0 } },
    & ifmt_addi16, { 0x33 }
  },
/* sub.l $rd6,$rn6,$simm11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SIMM11), 0 } },
    & ifmt_addi, { 0x3b }
  },
/* asr $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x6a }
  },
/* asr $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0xa006f }
  },
/* lsr $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x4a }
  },
/* lsr $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0xa004f }
  },
/* lsl $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x2a }
  },
/* lsl $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0xa002f }
  },
/* lsr $rd,$rn,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (SHIFT), 0 } },
    & ifmt_lsri16, { 0x6 }
  },
/* lsr $rd6,$rn6,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SHIFT), 0 } },
    & ifmt_lsri32, { 0x6000f }
  },
/* lsl $rd,$rn,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (SHIFT), 0 } },
    & ifmt_lsri16, { 0x16 }
  },
/* lsl $rd6,$rn6,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SHIFT), 0 } },
    & ifmt_lsri32, { 0x6001f }
  },
/* asr $rd,$rn,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (SHIFT), 0 } },
    & ifmt_lsri16, { 0xe }
  },
/* asr $rd6,$rn6,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SHIFT), 0 } },
    & ifmt_lsri32, { 0xe000f }
  },
/* bitr $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_bitr16, { 0x1e }
  },
/* bitr $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_bitr, { 0xe001f }
  },
/* fext $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_fext, { 0x1a000f }
  },
/* fdep $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_fext, { 0x1a001f }
  },
/* lfsr $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_fext, { 0x1a002f }
  },
/* mov.b $rd,$imm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (IMM8), 0 } },
    & ifmt_mov8, { 0x3 }
  },
/* mov.l $rd6,$imm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (IMM16), 0 } },
    & ifmt_mov16, { 0x2000b }
  },
/* movt $rd6,$imm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (IMM16), 0 } },
    & ifmt_mov16, { 0x1002000b }
  },
/* fadd $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x7 }
  },
/* fadd $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0x7000f }
  },
/* fsub $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x17 }
  },
/* fsub $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0x7001f }
  },
/* fmul $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x27 }
  },
/* fmul $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0x7002f }
  },
/* fmadd $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x37 }
  },
/* fmadd $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0x7003f }
  },
/* fmsub $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add16, { 0x47 }
  },
/* fmsub $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add, { 0x7004f }
  },
/* fabs rd,rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', 'r', 'd', ',', 'r', 'n', 0 } },
    & ifmt_f_absf16, { 0x77 }
  },
/* fabs $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_f_absf32, { 0x7007f }
  },
/* float $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_f_loatf16, { 0x57 }
  },
/* float $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_f_absf32, { 0x7005f }
  },
/* fix $rd,$rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), 0 } },
    & ifmt_f_absf16, { 0x67 }
  },
/* fix $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_f_absf32, { 0x7006f }
  },
/* frecip $frd6,$frn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (FRD6), ',', OP (FRN6), 0 } },
    & ifmt_f_recipf32, { 0x17000f }
  },
/* fsqrt $frd6,$frn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (FRD6), ',', OP (FRN6), 0 } },
    & ifmt_f_recipf32, { 0x17001f }
  },
};

#undef A
#undef OPERAND
#undef MNEM
#undef OP

/* Formats for ALIAS macro-insns.  */

#define F(f) & epiphany_cgen_ifld_table[EPIPHANY_##f]
static const CGEN_IFMT ifmt_beq16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_beq32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bne16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bne32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bgtu16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bgtu32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bgteu16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bgteu32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_blteu16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_blteu32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bltu16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bltu32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bgt16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bgt32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bgte16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bgte32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_blt16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_blt32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_blte16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_blte32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bbeq16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bbeq32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bbne16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bbne32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bblt16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bblt32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bblte16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_bblte32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_b16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_b32r ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_bl16r ATTRIBUTE_UNUSED = {
  16, 16, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_blr ATTRIBUTE_UNUSED = {
  32, 32, 0xff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_SIMM24) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbx ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbp ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbd ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbdpm ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbds0 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbdl0 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrbdl0_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrhx ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrhp ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrhd ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrhdpm ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrhds0 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrhdl0 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrhdl0_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrx ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrp ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrd ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrdpm ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrds0 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrdl0 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrdl0_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrdx ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrdp ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrdd ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrddpm ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrdds0 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrddl0 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_ldrddl0_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_testsetbt_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_testsetht_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_testsett_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strbx_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strbp_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strbd_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strbdpm_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strbds0 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_strbdl0 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strbdl0_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strhx_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strhp_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strhd_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strhdpm_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strhds0 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_strhdl0 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strhdl0_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strx_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strp_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strd_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strdpm_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strds0 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_strdl0 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strdl0_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strdx_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_1) }, { F (F_DC_21_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strdp_l ATTRIBUTE_UNUSED = {
  32, 32, 0x6f007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_ADDSUBX) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_strdd_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strddpm_l ATTRIBUTE_UNUSED = {
  32, 32, 0x200007f, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strdds0 ATTRIBUTE_UNUSED = {
  16, 16, 0x3ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_RD) }, { F (F_RN) }, { F (F_DISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_strddl0 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_strddl0_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_WORDSIZE) }, { F (F_STORE) }, { F (F_PM) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SUBD) }, { F (F_DISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lEQ ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lNE ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lGTU ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lGTEU ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lLTEU ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lLTU ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lGT ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lGTE ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lLT ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lLTE ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lB ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lBEQ ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lBNE ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lBLT ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_lBLTE ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_CONDCODE) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_movts_l6 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_DC_7_4) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_SN6) }, { F (F_RD6) }, { 0 } }
};

static const CGEN_IFMT ifmt_movts_ldma ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_DC_7_4) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_SN6) }, { F (F_RD6) }, { 0 } }
};

static const CGEN_IFMT ifmt_movts_lmem ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_DC_7_4) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_SN6) }, { F (F_RD6) }, { 0 } }
};

static const CGEN_IFMT ifmt_movts_lmesh ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_DC_7_4) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_SN6) }, { F (F_RD6) }, { 0 } }
};

static const CGEN_IFMT ifmt_movfs_l6 ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_DC_7_4) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_RD6) }, { F (F_SN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_movfs_ldma ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_DC_7_4) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_RD6) }, { F (F_SN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_movfs_lmem ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_DC_7_4) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_RD6) }, { F (F_SN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_movfs_lmesh ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_DC_7_4) }, { F (F_OPC_8_1) }, { F (F_DC_9_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_4) }, { F (F_DC_21_2) }, { F (F_RD6) }, { F (F_SN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_add_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_sub_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_and_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_orr_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_eor_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_addir ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_RD) }, { F (F_RN) }, { F (F_SDISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_addi32r ATTRIBUTE_UNUSED = {
  32, 32, 0x300007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_DC_25_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SDISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_addi32m ATTRIBUTE_UNUSED = {
  32, 32, 0x300007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_DC_25_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SDISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_subir ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_RD) }, { F (F_RN) }, { F (F_SDISP3) }, { 0 } }
};

static const CGEN_IFMT ifmt_subi32r ATTRIBUTE_UNUSED = {
  32, 32, 0x300007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_DC_25_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SDISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_subi32m ATTRIBUTE_UNUSED = {
  32, 32, 0x300007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_DC_25_2) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SDISP11) }, { 0 } }
};

static const CGEN_IFMT ifmt_asr_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_lsr_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_lsl_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_lsri32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff001f, { { F (F_OPC) }, { F (F_OPC_4_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SHIFT) }, { 0 } }
};

static const CGEN_IFMT ifmt_lsli32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff001f, { { F (F_OPC) }, { F (F_OPC_4_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SHIFT) }, { 0 } }
};

static const CGEN_IFMT ifmt_asri32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff001f, { { F (F_OPC) }, { F (F_OPC_4_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SHIFT) }, { 0 } }
};

static const CGEN_IFMT ifmt_bitrl ATTRIBUTE_UNUSED = {
  32, 32, 0x3ff03ff, { { F (F_OPC) }, { F (F_OPC_4_1) }, { F (F_OPC_19_4) }, { F (F_DC_25_6) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_SHIFT) }, { 0 } }
};

static const CGEN_IFMT ifmt_fext_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_DC_20_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_fdep_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_DC_20_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_lfsr_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_DC_20_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_mov8r ATTRIBUTE_UNUSED = {
  16, 16, 0x1f, { { F (F_OPC) }, { F (F_OPC_4_1) }, { F (F_RD) }, { F (F_IMM8) }, { 0 } }
};

static const CGEN_IFMT ifmt_mov16r ATTRIBUTE_UNUSED = {
  32, 32, 0x100f001f, { { F (F_OPC) }, { F (F_OPC_4_1) }, { F (F_OPC_19_4) }, { F (F_DC_28_1) }, { F (F_RD6) }, { F (F_IMM16) }, { 0 } }
};

static const CGEN_IFMT ifmt_movtl ATTRIBUTE_UNUSED = {
  32, 32, 0x100f001f, { { F (F_OPC) }, { F (F_OPC_4_1) }, { F (F_OPC_19_4) }, { F (F_DC_28_1) }, { F (F_RD6) }, { F (F_IMM16) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_addf16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_RD) }, { F (F_RN) }, { F (F_RM) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_addf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_addf32 ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_addf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_subf16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_RD) }, { F (F_RN) }, { F (F_RM) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_subf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_subf32 ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_subf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_mulf16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_RD) }, { F (F_RN) }, { F (F_RM) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_mulf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_mulf32 ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_mulf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_maddf16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_RD) }, { F (F_RN) }, { F (F_RM) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_maddf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_maddf32 ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_maddf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_msubf16 ATTRIBUTE_UNUSED = {
  16, 16, 0x7f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_RD) }, { F (F_RN) }, { F (F_RM) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_msubf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_msubf32 ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_i_msubf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RM6) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_absf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_loatf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_ixf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_3) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_recipf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_DC_20_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RN6) }, { 0 } }
};

static const CGEN_IFMT ifmt_f_sqrtf32_l ATTRIBUTE_UNUSED = {
  32, 32, 0x7f007f, { { F (F_OPC) }, { F (F_OPC_6_3) }, { F (F_OPC_19_4) }, { F (F_DC_22_2) }, { F (F_DC_20_1) }, { F (F_RD6) }, { F (F_RN6) }, { F (F_RN6) }, { 0 } }
};

#undef F

/* Each non-simple macro entry points to an array of expansion possibilities.  */

#define A(a) (1 << CGEN_INSN_##a)
#define OPERAND(op) EPIPHANY_OPERAND_##op
#define MNEM CGEN_SYNTAX_MNEMONIC /* syntax value for mnemonic */
#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))

/* The macro instruction table.  */

static const CGEN_IBASE epiphany_cgen_macro_insn_table[] =
{
/* beq $simm8 */
  {
    -1, "beq16r", "beq", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* beq $simm24 */
  {
    -1, "beq32r", "beq", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bne $simm8 */
  {
    -1, "bne16r", "bne", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bne $simm24 */
  {
    -1, "bne32r", "bne", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgtu $simm8 */
  {
    -1, "bgtu16r", "bgtu", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgtu $simm24 */
  {
    -1, "bgtu32r", "bgtu", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgteu $simm8 */
  {
    -1, "bgteu16r", "bgteu", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgteu $simm24 */
  {
    -1, "bgteu32r", "bgteu", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* blteu $simm8 */
  {
    -1, "blteu16r", "blteu", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* blteu $simm24 */
  {
    -1, "blteu32r", "blteu", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bltu $simm8 */
  {
    -1, "bltu16r", "bltu", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bltu $simm24 */
  {
    -1, "bltu32r", "bltu", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgt $simm8 */
  {
    -1, "bgt16r", "bgt", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgt $simm24 */
  {
    -1, "bgt32r", "bgt", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgte $simm8 */
  {
    -1, "bgte16r", "bgte", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgte $simm24 */
  {
    -1, "bgte32r", "bgte", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* blt $simm8 */
  {
    -1, "blt16r", "blt", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* blt $simm24 */
  {
    -1, "blt32r", "blt", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* blte $simm8 */
  {
    -1, "blte16r", "blte", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* blte $simm24 */
  {
    -1, "blte32r", "blte", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bbeq $simm8 */
  {
    -1, "bbeq16r", "bbeq", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bbeq $simm24 */
  {
    -1, "bbeq32r", "bbeq", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bbne $simm8 */
  {
    -1, "bbne16r", "bbne", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bbne $simm24 */
  {
    -1, "bbne32r", "bbne", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bblt $simm8 */
  {
    -1, "bblt16r", "bblt", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bblt $simm24 */
  {
    -1, "bblt32r", "bblt", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bblte $simm8 */
  {
    -1, "bblte16r", "bblte", 16,
    { 0|A(RELAXABLE)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bblte $simm24 */
  {
    -1, "bblte32r", "bblte", 32,
    { 0|A(RELAXED)|A(COND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* b $simm8 */
  {
    -1, "b16r", "b", 16,
    { 0|A(RELAXABLE)|A(UNCOND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* b $simm24 */
  {
    -1, "b32r", "b", 32,
    { 0|A(RELAXED)|A(UNCOND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bl $simm8 */
  {
    -1, "bl16r", "bl", 16,
    { 0|A(RELAXABLE)|A(UNCOND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bl $simm24 */
  {
    -1, "blr", "bl", 32,
    { 0|A(RELAXED)|A(UNCOND_CTI)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrb.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "ldrbx", "ldrb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrb.l $rd6,[$rn6],$direction$rm6 */
  {
    -1, "ldrbp", "ldrb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrb.l $rd6,[$rn6,$dpmi$disp11] */
  {
    -1, "ldrbd", "ldrb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrb.l $rd6,[$rn6],$dpmi$disp11 */
  {
    -1, "ldrbdpm", "ldrb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrb $rd,[$rn] */
  {
    -1, "ldrbds0", "ldrb", 16,
    { 0|A(IMM3)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrb $rd6,[$rn6] */
  {
    -1, "ldrbdl0", "ldrb", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrb.l $rd6,[$rn6] */
  {
    -1, "ldrbdl0.l", "ldrb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrh.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "ldrhx", "ldrh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrh.l $rd6,[$rn6],$direction$rm6 */
  {
    -1, "ldrhp", "ldrh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrh.l $rd6,[$rn6,$dpmi$disp11] */
  {
    -1, "ldrhd", "ldrh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrh.l $rd6,[$rn6],$dpmi$disp11 */
  {
    -1, "ldrhdpm", "ldrh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrh $rd,[$rn] */
  {
    -1, "ldrhds0", "ldrh", 16,
    { 0|A(IMM3)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrh $rd6,[$rn6] */
  {
    -1, "ldrhdl0", "ldrh", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrh.l $rd6,[$rn6] */
  {
    -1, "ldrhdl0.l", "ldrh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldr.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "ldrx", "ldr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldr.l $rd6,[$rn6],$direction$rm6 */
  {
    -1, "ldrp", "ldr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldr.l $rd6,[$rn6,$dpmi$disp11] */
  {
    -1, "ldrd", "ldr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldr.l $rd6,[$rn6],$dpmi$disp11 */
  {
    -1, "ldrdpm", "ldr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldr $rd,[$rn] */
  {
    -1, "ldrds0", "ldr", 16,
    { 0|A(IMM3)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldr $rd6,[$rn6] */
  {
    -1, "ldrdl0", "ldr", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldr.l $rd6,[$rn6] */
  {
    -1, "ldrdl0.l", "ldr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrd.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "ldrdx", "ldrd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrd.l $rd6,[$rn6],$direction$rm6 */
  {
    -1, "ldrdp", "ldrd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrd.l $rd6,[$rn6,$dpmi$disp11] */
  {
    -1, "ldrdd", "ldrd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrd.l $rd6,[$rn6],$dpmi$disp11 */
  {
    -1, "ldrddpm", "ldrd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrd $rd,[$rn] */
  {
    -1, "ldrdds0", "ldrd", 16,
    { 0|A(IMM3)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrd $rd6,[$rn6] */
  {
    -1, "ldrddl0", "ldrd", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ldrd.l $rd6,[$rn6] */
  {
    -1, "ldrddl0.l", "ldrd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* testsetb.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "testsetbt.l", "testsetb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* testseth.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "testsetht.l", "testseth.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* testset.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "testsett.l", "testset.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strb.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "strbx.l", "strb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strb.l $rd6,[$rn6],$direction$rm6 */
  {
    -1, "strbp.l", "strb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strb.l $rd6,[$rn6,$dpmi$disp11] */
  {
    -1, "strbd.l", "strb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strb.l $rd6,[$rn6],$dpmi$disp11 */
  {
    -1, "strbdpm.l", "strb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strb $rd,[$rn] */
  {
    -1, "strbds0", "strb", 16,
    { 0|A(IMM3)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strb $rd6,[$rn6] */
  {
    -1, "strbdl0", "strb", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strb.l $rd6,[$rn6] */
  {
    -1, "strbdl0.l", "strb.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strh.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "strhx.l", "strh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strh.l $rd6,[$rn6],$direction$rm6 */
  {
    -1, "strhp.l", "strh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strh.l $rd6,[$rn6,$dpmi$disp11] */
  {
    -1, "strhd.l", "strh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strh.l $rd6,[$rn6],$dpmi$disp11 */
  {
    -1, "strhdpm.l", "strh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strh $rd,[$rn] */
  {
    -1, "strhds0", "strh", 16,
    { 0|A(IMM3)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strh $rd6,[$rn6] */
  {
    -1, "strhdl0", "strh", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strh.l $rd6,[$rn6] */
  {
    -1, "strhdl0.l", "strh.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* str.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "strx.l", "str.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* str.l $rd6,[$rn6],$direction$rm6 */
  {
    -1, "strp.l", "str.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* str.l $rd6,[$rn6,$dpmi$disp11] */
  {
    -1, "strd.l", "str.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* str.l $rd6,[$rn6],$dpmi$disp11 */
  {
    -1, "strdpm.l", "str.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* str $rd,[$rn] */
  {
    -1, "strds0", "str", 16,
    { 0|A(IMM3)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* str $rd6,[$rn6] */
  {
    -1, "strdl0", "str", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* str.l $rd6,[$rn6] */
  {
    -1, "strdl0.l", "str.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strd.l $rd6,[$rn6,$direction$rm6] */
  {
    -1, "strdx.l", "strd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strd.l $rd6,[$rn6],$direction$rm6 */
  {
    -1, "strdp.l", "strd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strd.l $rd6,[$rn6,$dpmi$disp11] */
  {
    -1, "strdd.l", "strd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strd.l $rd6,[$rn6],$dpmi$disp11 */
  {
    -1, "strddpm.l", "strd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strd $rd,[$rn] */
  {
    -1, "strdds0", "strd", 16,
    { 0|A(IMM3)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strd $rd6,[$rn6] */
  {
    -1, "strddl0", "strd", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* strd.l $rd6,[$rn6] */
  {
    -1, "strddl0.l", "strd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* moveq.l $rd6,$rn6 */
  {
    -1, "cmov.lEQ", "moveq.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movne.l $rd6,$rn6 */
  {
    -1, "cmov.lNE", "movne.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movgtu.l $rd6,$rn6 */
  {
    -1, "cmov.lGTU", "movgtu.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movgteu.l $rd6,$rn6 */
  {
    -1, "cmov.lGTEU", "movgteu.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movlteu.l $rd6,$rn6 */
  {
    -1, "cmov.lLTEU", "movlteu.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movltu.l $rd6,$rn6 */
  {
    -1, "cmov.lLTU", "movltu.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movgt.l $rd6,$rn6 */
  {
    -1, "cmov.lGT", "movgt.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movgte.l $rd6,$rn6 */
  {
    -1, "cmov.lGTE", "movgte.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movlt.l $rd6,$rn6 */
  {
    -1, "cmov.lLT", "movlt.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movlte.l $rd6,$rn6 */
  {
    -1, "cmov.lLTE", "movlte.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mov.l $rd6,$rn6 */
  {
    -1, "cmov.lB", "mov.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movbeq.l $rd6,$rn6 */
  {
    -1, "cmov.lBEQ", "movbeq.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movbne.l $rd6,$rn6 */
  {
    -1, "cmov.lBNE", "movbne.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movblt.l $rd6,$rn6 */
  {
    -1, "cmov.lBLT", "movblt.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movblte.l $rd6,$rn6 */
  {
    -1, "cmov.lBLTE", "movblte.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movts.l $sn6,$rd6 */
  {
    -1, "movts.l6", "movts.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movts.l $sndma,$rd6 */
  {
    -1, "movts.ldma", "movts.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movts.l $snmem,$rd6 */
  {
    -1, "movts.lmem", "movts.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movts.l $snmesh,$rd6 */
  {
    -1, "movts.lmesh", "movts.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movfs.l $rd6,$sn6 */
  {
    -1, "movfs.l6", "movfs.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movfs.l $rd6,$sndma */
  {
    -1, "movfs.ldma", "movfs.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movfs.l $rd6,$snmem */
  {
    -1, "movfs.lmem", "movfs.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movfs.l $rd6,$snmesh */
  {
    -1, "movfs.lmesh", "movfs.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* add.l $rd6,$rn6,$rm6 */
  {
    -1, "add.l", "add.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub.l $rd6,$rn6,$rm6 */
  {
    -1, "sub.l", "sub.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* and.l $rd6,$rn6,$rm6 */
  {
    -1, "and.l", "and.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* orr.l $rd6,$rn6,$rm6 */
  {
    -1, "orr.l", "orr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* eor.l $rd6,$rn6,$rm6 */
  {
    -1, "eor.l", "eor.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* add $rd,$rn,$simm3 */
  {
    -1, "addir", "add", 16,
    { 0|A(IMM3)|A(RELAXABLE)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* add $rd6,$rn6,$simm11 */
  {
    -1, "addi32r", "add", 32,
    { 0|A(RELAXED)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* add $rd6,$rn6,$simm11 */
  {
    -1, "addi32m", "add", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub $rd,$rn,$simm3 */
  {
    -1, "subir", "sub", 16,
    { 0|A(IMM3)|A(RELAXABLE)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub $rd6,$rn6,$simm11 */
  {
    -1, "subi32r", "sub", 32,
    { 0|A(RELAXED)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub $rd6,$rn6,$simm11 */
  {
    -1, "subi32m", "sub", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* asr.l $rd6,$rn6,$rm6 */
  {
    -1, "asr.l", "asr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lsr.l $rd6,$rn6,$rm6 */
  {
    -1, "lsr.l", "lsr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lsl.l $rd6,$rn6,$rm6 */
  {
    -1, "lsl.l", "lsl.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lsr.l $rd6,$rn6,$shift */
  {
    -1, "lsri32.l", "lsr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lsl.l $rd6,$rn6,$shift */
  {
    -1, "lsli32.l", "lsl.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* asr.l $rd6,$rn6,$shift */
  {
    -1, "asri32.l", "asr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bitr.l $rd6,$rn6 */
  {
    -1, "bitrl", "bitr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fext.l $rd6,$rn6,$rm6 */
  {
    -1, "fext.l", "fext.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fdep.l $rd6,$rn6,$rm6 */
  {
    -1, "fdep.l", "fdep.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lfsr.l $rd6,$rn6,$rm6 */
  {
    -1, "lfsr.l", "lfsr.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mov $rd,$imm8 */
  {
    -1, "mov8r", "mov", 16,
    { 0|A(RELAXABLE)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mov $rd6,$imm16 */
  {
    -1, "mov16r", "mov", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* movt.l $rd6,$imm16 */
  {
    -1, "movtl", "movt.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* iadd $rd,$rn,$rm */
  {
    -1, "i_addf16", "iadd", 16,
    { 0|A(NO_DIS)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fadd.l $rd6,$rn6,$rm6 */
  {
    -1, "f_addf32.l", "fadd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* iadd $rd6,$rn6,$rm6 */
  {
    -1, "i_addf32", "iadd", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* iadd.l $rd6,$rn6,$rm6 */
  {
    -1, "i_addf32.l", "iadd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* isub $rd,$rn,$rm */
  {
    -1, "i_subf16", "isub", 16,
    { 0|A(NO_DIS)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fsub.l $rd6,$rn6,$rm6 */
  {
    -1, "f_subf32.l", "fsub.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* isub $rd6,$rn6,$rm6 */
  {
    -1, "i_subf32", "isub", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* isub.l $rd6,$rn6,$rm6 */
  {
    -1, "i_subf32.l", "isub.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imul $rd,$rn,$rm */
  {
    -1, "i_mulf16", "imul", 16,
    { 0|A(NO_DIS)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fmul.l $rd6,$rn6,$rm6 */
  {
    -1, "f_mulf32.l", "fmul.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imul $rd6,$rn6,$rm6 */
  {
    -1, "i_mulf32", "imul", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imul.l $rd6,$rn6,$rm6 */
  {
    -1, "i_mulf32.l", "imul.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imadd $rd,$rn,$rm */
  {
    -1, "i_maddf16", "imadd", 16,
    { 0|A(NO_DIS)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fmadd.l $rd6,$rn6,$rm6 */
  {
    -1, "f_maddf32.l", "fmadd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imadd $rd6,$rn6,$rm6 */
  {
    -1, "i_maddf32", "imadd", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imadd.l $rd6,$rn6,$rm6 */
  {
    -1, "i_maddf32.l", "imadd.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imsub $rd,$rn,$rm */
  {
    -1, "i_msubf16", "imsub", 16,
    { 0|A(NO_DIS)|A(SHORT_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fmsub.l $rd6,$rn6,$rm6 */
  {
    -1, "f_msubf32.l", "fmsub.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imsub $rd6,$rn6,$rm6 */
  {
    -1, "i_msubf32", "imsub", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* imsub.l $rd6,$rn6,$rm6 */
  {
    -1, "i_msubf32.l", "imsub.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fabs.l $rd6,$rn6 */
  {
    -1, "f_absf32.l", "fabs.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* float.l $rd6,$rn6 */
  {
    -1, "f_loatf32.l", "float.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fix.l $rd6,$rn6 */
  {
    -1, "f_ixf32.l", "fix.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* frecip.l $frd6,$frn6 */
  {
    -1, "f_recipf32.l", "frecip.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* fsqrt.l $frd6,$frn6 */
  {
    -1, "f_sqrtf32.l", "fsqrt.l", 32,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
};

/* The macro instruction opcode table.  */

static const CGEN_OPCODE epiphany_cgen_macro_insn_opcode_table[] =
{
/* beq $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_beq16r, { 0x0 }
  },
/* beq $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_beq32r, { 0x8 }
  },
/* bne $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bne16r, { 0x10 }
  },
/* bne $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bne32r, { 0x18 }
  },
/* bgtu $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bgtu16r, { 0x20 }
  },
/* bgtu $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bgtu32r, { 0x28 }
  },
/* bgteu $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bgteu16r, { 0x30 }
  },
/* bgteu $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bgteu32r, { 0x38 }
  },
/* blteu $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_blteu16r, { 0x40 }
  },
/* blteu $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_blteu32r, { 0x48 }
  },
/* bltu $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bltu16r, { 0x50 }
  },
/* bltu $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bltu32r, { 0x58 }
  },
/* bgt $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bgt16r, { 0x60 }
  },
/* bgt $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bgt32r, { 0x68 }
  },
/* bgte $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bgte16r, { 0x70 }
  },
/* bgte $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bgte32r, { 0x78 }
  },
/* blt $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_blt16r, { 0x80 }
  },
/* blt $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_blt32r, { 0x88 }
  },
/* blte $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_blte16r, { 0x90 }
  },
/* blte $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_blte32r, { 0x98 }
  },
/* bbeq $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bbeq16r, { 0xa0 }
  },
/* bbeq $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bbeq32r, { 0xa8 }
  },
/* bbne $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bbne16r, { 0xb0 }
  },
/* bbne $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bbne32r, { 0xb8 }
  },
/* bblt $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bblt16r, { 0xc0 }
  },
/* bblt $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bblt32r, { 0xc8 }
  },
/* bblte $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bblte16r, { 0xd0 }
  },
/* bblte $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_bblte32r, { 0xd8 }
  },
/* b $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_b16r, { 0xe0 }
  },
/* b $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_b32r, { 0xe8 }
  },
/* bl $simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM8), 0 } },
    & ifmt_bl16r, { 0xf0 }
  },
/* bl $simm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SIMM24), 0 } },
    & ifmt_blr, { 0xf8 }
  },
/* ldrb.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrbx, { 0x9 }
  },
/* ldrb.l $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrbp, { 0xd }
  },
/* ldrb.l $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrbd, { 0xc }
  },
/* ldrb.l $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrbdpm, { 0x200000c }
  },
/* ldrb $rd,[$rn] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', 0 } },
    & ifmt_ldrbds0, { 0x4 }
  },
/* ldrb $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_ldrbdl0, { 0xc }
  },
/* ldrb.l $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_ldrbdl0_l, { 0xc }
  },
/* ldrh.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrhx, { 0x29 }
  },
/* ldrh.l $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrhp, { 0x2d }
  },
/* ldrh.l $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrhd, { 0x2c }
  },
/* ldrh.l $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrhdpm, { 0x200002c }
  },
/* ldrh $rd,[$rn] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', 0 } },
    & ifmt_ldrhds0, { 0x24 }
  },
/* ldrh $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_ldrhdl0, { 0x2c }
  },
/* ldrh.l $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_ldrhdl0_l, { 0x2c }
  },
/* ldr.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrx, { 0x49 }
  },
/* ldr.l $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrp, { 0x4d }
  },
/* ldr.l $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrd, { 0x4c }
  },
/* ldr.l $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrdpm, { 0x200004c }
  },
/* ldr $rd,[$rn] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', 0 } },
    & ifmt_ldrds0, { 0x44 }
  },
/* ldr $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_ldrdl0, { 0x4c }
  },
/* ldr.l $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_ldrdl0_l, { 0x4c }
  },
/* ldrd.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_ldrdx, { 0x69 }
  },
/* ldrd.l $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_ldrdp, { 0x6d }
  },
/* ldrd.l $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_ldrdd, { 0x6c }
  },
/* ldrd.l $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_ldrddpm, { 0x200006c }
  },
/* ldrd $rd,[$rn] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', 0 } },
    & ifmt_ldrdds0, { 0x64 }
  },
/* ldrd $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_ldrddl0, { 0x6c }
  },
/* ldrd.l $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_ldrddl0_l, { 0x6c }
  },
/* testsetb.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_testsetbt_l, { 0x200009 }
  },
/* testseth.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_testsetht_l, { 0x200029 }
  },
/* testset.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_testsett_l, { 0x200049 }
  },
/* strb.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_strbx_l, { 0x19 }
  },
/* strb.l $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_strbp_l, { 0x1d }
  },
/* strb.l $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_strbd_l, { 0x1c }
  },
/* strb.l $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_strbdpm_l, { 0x200001c }
  },
/* strb $rd,[$rn] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', 0 } },
    & ifmt_strbds0, { 0x14 }
  },
/* strb $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_strbdl0, { 0x1c }
  },
/* strb.l $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_strbdl0_l, { 0x1c }
  },
/* strh.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_strhx_l, { 0x39 }
  },
/* strh.l $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_strhp_l, { 0x3d }
  },
/* strh.l $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_strhd_l, { 0x3c }
  },
/* strh.l $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_strhdpm_l, { 0x200003c }
  },
/* strh $rd,[$rn] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', 0 } },
    & ifmt_strhds0, { 0x34 }
  },
/* strh $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_strhdl0, { 0x3c }
  },
/* strh.l $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_strhdl0_l, { 0x3c }
  },
/* str.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_strx_l, { 0x59 }
  },
/* str.l $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_strp_l, { 0x5d }
  },
/* str.l $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_strd_l, { 0x5c }
  },
/* str.l $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_strdpm_l, { 0x200005c }
  },
/* str $rd,[$rn] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', 0 } },
    & ifmt_strds0, { 0x54 }
  },
/* str $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_strdl0, { 0x5c }
  },
/* str.l $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_strdl0_l, { 0x5c }
  },
/* strd.l $rd6,[$rn6,$direction$rm6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DIRECTION), OP (RM6), ']', 0 } },
    & ifmt_strdx_l, { 0x79 }
  },
/* strd.l $rd6,[$rn6],$direction$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DIRECTION), OP (RM6), 0 } },
    & ifmt_strdp_l, { 0x7d }
  },
/* strd.l $rd6,[$rn6,$dpmi$disp11] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ',', OP (DPMI), OP (DISP11), ']', 0 } },
    & ifmt_strdd_l, { 0x7c }
  },
/* strd.l $rd6,[$rn6],$dpmi$disp11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', ',', OP (DPMI), OP (DISP11), 0 } },
    & ifmt_strddpm_l, { 0x200007c }
  },
/* strd $rd,[$rn] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', '[', OP (RN), ']', 0 } },
    & ifmt_strdds0, { 0x74 }
  },
/* strd $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_strddl0, { 0x7c }
  },
/* strd.l $rd6,[$rn6] */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', '[', OP (RN6), ']', 0 } },
    & ifmt_strddl0_l, { 0x7c }
  },
/* moveq.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lEQ, { 0x2000f }
  },
/* movne.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lNE, { 0x2001f }
  },
/* movgtu.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lGTU, { 0x2002f }
  },
/* movgteu.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lGTEU, { 0x2003f }
  },
/* movlteu.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lLTEU, { 0x2004f }
  },
/* movltu.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lLTU, { 0x2005f }
  },
/* movgt.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lGT, { 0x2006f }
  },
/* movgte.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lGTE, { 0x2007f }
  },
/* movlt.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lLT, { 0x2008f }
  },
/* movlte.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lLTE, { 0x2009f }
  },
/* mov.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lB, { 0x200ef }
  },
/* movbeq.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lBEQ, { 0x200af }
  },
/* movbne.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lBNE, { 0x200bf }
  },
/* movblt.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lBLT, { 0x200cf }
  },
/* movblte.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_cmov_lBLTE, { 0x200df }
  },
/* movts.l $sn6,$rd6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SN6), ',', OP (RD6), 0 } },
    & ifmt_movts_l6, { 0x2010f }
  },
/* movts.l $sndma,$rd6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SNDMA), ',', OP (RD6), 0 } },
    & ifmt_movts_ldma, { 0x12010f }
  },
/* movts.l $snmem,$rd6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SNMEM), ',', OP (RD6), 0 } },
    & ifmt_movts_lmem, { 0x22010f }
  },
/* movts.l $snmesh,$rd6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (SNMESH), ',', OP (RD6), 0 } },
    & ifmt_movts_lmesh, { 0x32010f }
  },
/* movfs.l $rd6,$sn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (SN6), 0 } },
    & ifmt_movfs_l6, { 0x2011f }
  },
/* movfs.l $rd6,$sndma */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (SNDMA), 0 } },
    & ifmt_movfs_ldma, { 0x12011f }
  },
/* movfs.l $rd6,$snmem */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (SNMEM), 0 } },
    & ifmt_movfs_lmem, { 0x22011f }
  },
/* movfs.l $rd6,$snmesh */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (SNMESH), 0 } },
    & ifmt_movfs_lmesh, { 0x32011f }
  },
/* add.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_add_l, { 0xa001f }
  },
/* sub.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_sub_l, { 0xa003f }
  },
/* and.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_and_l, { 0xa005f }
  },
/* orr.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_orr_l, { 0xa007f }
  },
/* eor.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_eor_l, { 0xa000f }
  },
/* add $rd,$rn,$simm3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (SIMM3), 0 } },
    & ifmt_addir, { 0x13 }
  },
/* add $rd6,$rn6,$simm11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SIMM11), 0 } },
    & ifmt_addi32r, { 0x1b }
  },
/* add $rd6,$rn6,$simm11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SIMM11), 0 } },
    & ifmt_addi32m, { 0x1b }
  },
/* sub $rd,$rn,$simm3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (SIMM3), 0 } },
    & ifmt_subir, { 0x33 }
  },
/* sub $rd6,$rn6,$simm11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SIMM11), 0 } },
    & ifmt_subi32r, { 0x3b }
  },
/* sub $rd6,$rn6,$simm11 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SIMM11), 0 } },
    & ifmt_subi32m, { 0x3b }
  },
/* asr.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_asr_l, { 0xa006f }
  },
/* lsr.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_lsr_l, { 0xa004f }
  },
/* lsl.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_lsl_l, { 0xa002f }
  },
/* lsr.l $rd6,$rn6,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SHIFT), 0 } },
    & ifmt_lsri32_l, { 0x6000f }
  },
/* lsl.l $rd6,$rn6,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SHIFT), 0 } },
    & ifmt_lsli32_l, { 0x6001f }
  },
/* asr.l $rd6,$rn6,$shift */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (SHIFT), 0 } },
    & ifmt_asri32_l, { 0xe000f }
  },
/* bitr.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_bitrl, { 0xe001f }
  },
/* fext.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_fext_l, { 0x1a000f }
  },
/* fdep.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_fdep_l, { 0x1a001f }
  },
/* lfsr.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_lfsr_l, { 0x1a002f }
  },
/* mov $rd,$imm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (IMM8), 0 } },
    & ifmt_mov8r, { 0x3 }
  },
/* mov $rd6,$imm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (IMM16), 0 } },
    & ifmt_mov16r, { 0x2000b }
  },
/* movt.l $rd6,$imm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (IMM16), 0 } },
    & ifmt_movtl, { 0x1002000b }
  },
/* iadd $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_i_addf16, { 0x7 }
  },
/* fadd.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_f_addf32_l, { 0x7000f }
  },
/* iadd $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_addf32, { 0x7000f }
  },
/* iadd.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_addf32_l, { 0x7000f }
  },
/* isub $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_i_subf16, { 0x17 }
  },
/* fsub.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_f_subf32_l, { 0x7001f }
  },
/* isub $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_subf32, { 0x7001f }
  },
/* isub.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_subf32_l, { 0x7001f }
  },
/* imul $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_i_mulf16, { 0x27 }
  },
/* fmul.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_f_mulf32_l, { 0x7002f }
  },
/* imul $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_mulf32, { 0x7002f }
  },
/* imul.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_mulf32_l, { 0x7002f }
  },
/* imadd $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_i_maddf16, { 0x37 }
  },
/* fmadd.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_f_maddf32_l, { 0x7003f }
  },
/* imadd $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_maddf32, { 0x7003f }
  },
/* imadd.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_maddf32_l, { 0x7003f }
  },
/* imsub $rd,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_i_msubf16, { 0x47 }
  },
/* fmsub.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_f_msubf32_l, { 0x7004f }
  },
/* imsub $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_msubf32, { 0x7004f }
  },
/* imsub.l $rd6,$rn6,$rm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), ',', OP (RM6), 0 } },
    & ifmt_i_msubf32_l, { 0x7004f }
  },
/* fabs.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_f_absf32_l, { 0x7007f }
  },
/* float.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_f_loatf32_l, { 0x7005f }
  },
/* fix.l $rd6,$rn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RD6), ',', OP (RN6), 0 } },
    & ifmt_f_ixf32_l, { 0x7006f }
  },
/* frecip.l $frd6,$frn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (FRD6), ',', OP (FRN6), 0 } },
    & ifmt_f_recipf32_l, { 0x17000f }
  },
/* fsqrt.l $frd6,$frn6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (FRD6), ',', OP (FRN6), 0 } },
    & ifmt_f_sqrtf32_l, { 0x17001f }
  },
};

#undef A
#undef OPERAND
#undef MNEM
#undef OP

#ifndef CGEN_ASM_HASH_P
#define CGEN_ASM_HASH_P(insn) 1
#endif

#ifndef CGEN_DIS_HASH_P
#define CGEN_DIS_HASH_P(insn) 1
#endif

/* Return non-zero if INSN is to be added to the hash table.
   Targets are free to override CGEN_{ASM,DIS}_HASH_P in the .opc file.  */

static int
asm_hash_insn_p (const CGEN_INSN *insn ATTRIBUTE_UNUSED)
{
  return CGEN_ASM_HASH_P (insn);
}

static int
dis_hash_insn_p (const CGEN_INSN *insn)
{
  /* If building the hash table and the NO-DIS attribute is present,
     ignore.  */
  if (CGEN_INSN_ATTR_VALUE (insn, CGEN_INSN_NO_DIS))
    return 0;
  return CGEN_DIS_HASH_P (insn);
}

#ifndef CGEN_ASM_HASH
#define CGEN_ASM_HASH_SIZE 127
#ifdef CGEN_MNEMONIC_OPERANDS
#define CGEN_ASM_HASH(mnem) (*(unsigned char *) (mnem) % CGEN_ASM_HASH_SIZE)
#else
#define CGEN_ASM_HASH(mnem) (*(unsigned char *) (mnem) % CGEN_ASM_HASH_SIZE) /*FIXME*/
#endif
#endif

/* It doesn't make much sense to provide a default here,
   but while this is under development we do.
   BUFFER is a pointer to the bytes of the insn, target order.
   VALUE is the first base_insn_bitsize bits as an int in host order.  */

#ifndef CGEN_DIS_HASH
#define CGEN_DIS_HASH_SIZE 256
#define CGEN_DIS_HASH(buf, value) (*(unsigned char *) (buf))
#endif

/* The result is the hash value of the insn.
   Targets are free to override CGEN_{ASM,DIS}_HASH in the .opc file.  */

static unsigned int
asm_hash_insn (const char *mnem)
{
  return CGEN_ASM_HASH (mnem);
}

/* BUF is a pointer to the bytes of the insn, target order.
   VALUE is the first base_insn_bitsize bits as an int in host order.  */

static unsigned int
dis_hash_insn (const char *buf ATTRIBUTE_UNUSED,
		     CGEN_INSN_INT value ATTRIBUTE_UNUSED)
{
  return CGEN_DIS_HASH (buf, value);
}

/* Set the recorded length of the insn in the CGEN_FIELDS struct.  */

static void
set_fields_bitsize (CGEN_FIELDS *fields, int size)
{
  CGEN_FIELDS_BITSIZE (fields) = size;
}

/* Function to call before using the operand instance table.
   This plugs the opcode entries and macro instructions into the cpu table.  */

void
epiphany_cgen_init_opcode_table (CGEN_CPU_DESC cd)
{
  int i;
  int num_macros = (sizeof (epiphany_cgen_macro_insn_table) /
		    sizeof (epiphany_cgen_macro_insn_table[0]));
  const CGEN_IBASE *ib = & epiphany_cgen_macro_insn_table[0];
  const CGEN_OPCODE *oc = & epiphany_cgen_macro_insn_opcode_table[0];
  CGEN_INSN *insns = xmalloc (num_macros * sizeof (CGEN_INSN));

  /* This test has been added to avoid a warning generated
     if memset is called with a third argument of value zero.  */
  if (num_macros >= 1)
    memset (insns, 0, num_macros * sizeof (CGEN_INSN));
  for (i = 0; i < num_macros; ++i)
    {
      insns[i].base = &ib[i];
      insns[i].opcode = &oc[i];
      epiphany_cgen_build_insn_regex (& insns[i]);
    }
  cd->macro_insn_table.init_entries = insns;
  cd->macro_insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->macro_insn_table.num_init_entries = num_macros;

  oc = & epiphany_cgen_insn_opcode_table[0];
  insns = (CGEN_INSN *) cd->insn_table.init_entries;
  for (i = 0; i < MAX_INSNS; ++i)
    {
      insns[i].opcode = &oc[i];
      epiphany_cgen_build_insn_regex (& insns[i]);
    }

  cd->sizeof_fields = sizeof (CGEN_FIELDS);
  cd->set_fields_bitsize = set_fields_bitsize;

  cd->asm_hash_p = asm_hash_insn_p;
  cd->asm_hash = asm_hash_insn;
  cd->asm_hash_size = CGEN_ASM_HASH_SIZE;

  cd->dis_hash_p = dis_hash_insn_p;
  cd->dis_hash = dis_hash_insn;
  cd->dis_hash_size = CGEN_DIS_HASH_SIZE;
}
