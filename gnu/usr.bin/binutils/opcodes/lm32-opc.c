/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Instruction opcode table for lm32.

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
#include "lm32-desc.h"
#include "lm32-opc.h"
#include "libiberty.h"

/* The hash functions are recorded here to help keep assembler code out of
   the disassembler and vice versa.  */

static int asm_hash_insn_p        (const CGEN_INSN *);
static unsigned int asm_hash_insn (const char *);
static int dis_hash_insn_p        (const CGEN_INSN *);
static unsigned int dis_hash_insn (const char *, CGEN_INSN_INT);

/* Instruction formats.  */

#define F(f) & lm32_cgen_ifld_table[LM32_##f]
static const CGEN_IFMT ifmt_empty ATTRIBUTE_UNUSED = {
  0, 0, 0x0, { { 0 } }
};

static const CGEN_IFMT ifmt_add ATTRIBUTE_UNUSED = {
  32, 32, 0xfc0007ff, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_R2) }, { F (F_RESV0) }, { 0 } }
};

static const CGEN_IFMT ifmt_addi ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_IMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_andi ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_UIMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_andhii ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_UIMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_b ATTRIBUTE_UNUSED = {
  32, 32, 0xfc1fffff, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_R2) }, { F (F_RESV0) }, { 0 } }
};

static const CGEN_IFMT ifmt_bi ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_CALL) }, { 0 } }
};

static const CGEN_IFMT ifmt_be ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_BRANCH) }, { 0 } }
};

static const CGEN_IFMT ifmt_ori ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_UIMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_rcsr ATTRIBUTE_UNUSED = {
  32, 32, 0xfc1f07ff, { { F (F_OPCODE) }, { F (F_CSR) }, { F (F_R1) }, { F (F_R2) }, { F (F_RESV0) }, { 0 } }
};

static const CGEN_IFMT ifmt_sextb ATTRIBUTE_UNUSED = {
  32, 32, 0xfc1f07ff, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_R2) }, { F (F_RESV0) }, { 0 } }
};

static const CGEN_IFMT ifmt_user ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_R2) }, { F (F_USER) }, { 0 } }
};

static const CGEN_IFMT ifmt_wcsr ATTRIBUTE_UNUSED = {
  32, 32, 0xfc00ffff, { { F (F_OPCODE) }, { F (F_CSR) }, { F (F_R1) }, { F (F_R2) }, { F (F_RESV0) }, { 0 } }
};

static const CGEN_IFMT ifmt_break ATTRIBUTE_UNUSED = {
  32, 32, 0xffffffff, { { F (F_OPCODE) }, { F (F_EXCEPTION) }, { 0 } }
};

static const CGEN_IFMT ifmt_bret ATTRIBUTE_UNUSED = {
  32, 32, 0xffffffff, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_R2) }, { F (F_RESV0) }, { 0 } }
};

static const CGEN_IFMT ifmt_mvi ATTRIBUTE_UNUSED = {
  32, 32, 0xffe00000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_IMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_mvui ATTRIBUTE_UNUSED = {
  32, 32, 0xffe00000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_UIMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_mvhi ATTRIBUTE_UNUSED = {
  32, 32, 0xffe00000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_UIMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_mva ATTRIBUTE_UNUSED = {
  32, 32, 0xffe00000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_IMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_nop ATTRIBUTE_UNUSED = {
  32, 32, 0xffffffff, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_IMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_lwgotrel ATTRIBUTE_UNUSED = {
  32, 32, 0xffe00000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_IMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_orhigotoffi ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_IMM) }, { 0 } }
};

static const CGEN_IFMT ifmt_addgotoff ATTRIBUTE_UNUSED = {
  32, 32, 0xfc000000, { { F (F_OPCODE) }, { F (F_R0) }, { F (F_R1) }, { F (F_IMM) }, { 0 } }
};

#undef F

#define A(a) (1 << CGEN_INSN_##a)
#define OPERAND(op) LM32_OPERAND_##op
#define MNEM CGEN_SYNTAX_MNEMONIC /* syntax value for mnemonic */
#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))

/* The instruction table.  */

static const CGEN_OPCODE lm32_cgen_insn_opcode_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { { 0, 0, 0, 0 }, {{0}}, 0, {0}},
/* add $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xb4000000 }
  },
/* addi $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x34000000 }
  },
/* and $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xa0000000 }
  },
/* andi $r1,$r0,$uimm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (UIMM), 0 } },
    & ifmt_andi, { 0x20000000 }
  },
/* andhi $r1,$r0,$hi16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (HI16), 0 } },
    & ifmt_andhii, { 0x60000000 }
  },
/* b $r0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R0), 0 } },
    & ifmt_b, { 0xc0000000 }
  },
/* bi $call */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CALL), 0 } },
    & ifmt_bi, { 0xe0000000 }
  },
/* be $r0,$r1,$branch */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R0), ',', OP (R1), ',', OP (BRANCH), 0 } },
    & ifmt_be, { 0x44000000 }
  },
/* bg $r0,$r1,$branch */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R0), ',', OP (R1), ',', OP (BRANCH), 0 } },
    & ifmt_be, { 0x48000000 }
  },
/* bge $r0,$r1,$branch */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R0), ',', OP (R1), ',', OP (BRANCH), 0 } },
    & ifmt_be, { 0x4c000000 }
  },
/* bgeu $r0,$r1,$branch */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R0), ',', OP (R1), ',', OP (BRANCH), 0 } },
    & ifmt_be, { 0x50000000 }
  },
/* bgu $r0,$r1,$branch */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R0), ',', OP (R1), ',', OP (BRANCH), 0 } },
    & ifmt_be, { 0x54000000 }
  },
/* bne $r0,$r1,$branch */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R0), ',', OP (R1), ',', OP (BRANCH), 0 } },
    & ifmt_be, { 0x5c000000 }
  },
/* call $r0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R0), 0 } },
    & ifmt_b, { 0xd8000000 }
  },
/* calli $call */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CALL), 0 } },
    & ifmt_bi, { 0xf8000000 }
  },
/* cmpe $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xe4000000 }
  },
/* cmpei $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x64000000 }
  },
/* cmpg $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xe8000000 }
  },
/* cmpgi $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x68000000 }
  },
/* cmpge $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xec000000 }
  },
/* cmpgei $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x6c000000 }
  },
/* cmpgeu $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xf0000000 }
  },
/* cmpgeui $r1,$r0,$uimm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (UIMM), 0 } },
    & ifmt_andi, { 0x70000000 }
  },
/* cmpgu $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xf4000000 }
  },
/* cmpgui $r1,$r0,$uimm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (UIMM), 0 } },
    & ifmt_andi, { 0x74000000 }
  },
/* cmpne $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xfc000000 }
  },
/* cmpnei $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x7c000000 }
  },
/* divu $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0x8c000000 }
  },
/* lb $r1,($r0+$imm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (IMM), ')', 0 } },
    & ifmt_addi, { 0x10000000 }
  },
/* lbu $r1,($r0+$imm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (IMM), ')', 0 } },
    & ifmt_addi, { 0x40000000 }
  },
/* lh $r1,($r0+$imm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (IMM), ')', 0 } },
    & ifmt_addi, { 0x1c000000 }
  },
/* lhu $r1,($r0+$imm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (IMM), ')', 0 } },
    & ifmt_addi, { 0x2c000000 }
  },
/* lw $r1,($r0+$imm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (IMM), ')', 0 } },
    & ifmt_addi, { 0x28000000 }
  },
/* modu $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xc4000000 }
  },
/* mul $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0x88000000 }
  },
/* muli $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x8000000 }
  },
/* nor $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0x84000000 }
  },
/* nori $r1,$r0,$uimm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (UIMM), 0 } },
    & ifmt_andi, { 0x4000000 }
  },
/* or $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xb8000000 }
  },
/* ori $r1,$r0,$lo16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (LO16), 0 } },
    & ifmt_ori, { 0x38000000 }
  },
/* orhi $r1,$r0,$hi16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (HI16), 0 } },
    & ifmt_andhii, { 0x78000000 }
  },
/* rcsr $r2,$csr */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (CSR), 0 } },
    & ifmt_rcsr, { 0x90000000 }
  },
/* sb ($r0+$imm),$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (R0), '+', OP (IMM), ')', ',', OP (R1), 0 } },
    & ifmt_addi, { 0x30000000 }
  },
/* sextb $r2,$r0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), 0 } },
    & ifmt_sextb, { 0xb0000000 }
  },
/* sexth $r2,$r0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), 0 } },
    & ifmt_sextb, { 0xdc000000 }
  },
/* sh ($r0+$imm),$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (R0), '+', OP (IMM), ')', ',', OP (R1), 0 } },
    & ifmt_addi, { 0xc000000 }
  },
/* sl $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xbc000000 }
  },
/* sli $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x3c000000 }
  },
/* sr $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0x94000000 }
  },
/* sri $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x14000000 }
  },
/* sru $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0x80000000 }
  },
/* srui $r1,$r0,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (IMM), 0 } },
    & ifmt_addi, { 0x0 }
  },
/* sub $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xc8000000 }
  },
/* sw ($r0+$imm),$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (R0), '+', OP (IMM), ')', ',', OP (R1), 0 } },
    & ifmt_addi, { 0x58000000 }
  },
/* user $r2,$r0,$r1,$user */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), ',', OP (USER), 0 } },
    & ifmt_user, { 0xcc000000 }
  },
/* wcsr $csr,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CSR), ',', OP (R1), 0 } },
    & ifmt_wcsr, { 0xd0000000 }
  },
/* xor $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0x98000000 }
  },
/* xori $r1,$r0,$uimm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (UIMM), 0 } },
    & ifmt_andi, { 0x18000000 }
  },
/* xnor $r2,$r0,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), ',', OP (R1), 0 } },
    & ifmt_add, { 0xa4000000 }
  },
/* xnori $r1,$r0,$uimm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (UIMM), 0 } },
    & ifmt_andi, { 0x24000000 }
  },
/* break */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_break, { 0xac000002 }
  },
/* scall */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_break, { 0xac000007 }
  },
/* bret */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_bret, { 0xc3e00000 }
  },
/* eret */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_bret, { 0xc3c00000 }
  },
/* ret */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_bret, { 0xc3a00000 }
  },
/* mv $r2,$r0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), 0 } },
    & ifmt_sextb, { 0xb8000000 }
  },
/* mvi $r1,$imm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (IMM), 0 } },
    & ifmt_mvi, { 0x34000000 }
  },
/* mvu $r1,$lo16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (LO16), 0 } },
    & ifmt_mvui, { 0x38000000 }
  },
/* mvhi $r1,$hi16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (HI16), 0 } },
    & ifmt_mvhi, { 0x78000000 }
  },
/* mva $r1,$gp16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (GP16), 0 } },
    & ifmt_mva, { 0x37400000 }
  },
/* not $r2,$r0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R2), ',', OP (R0), 0 } },
    & ifmt_sextb, { 0xa4000000 }
  },
/* nop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x34000000 }
  },
/* lb $r1,$gp16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (GP16), 0 } },
    & ifmt_mva, { 0x13400000 }
  },
/* lbu $r1,$gp16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (GP16), 0 } },
    & ifmt_mva, { 0x43400000 }
  },
/* lh $r1,$gp16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (GP16), 0 } },
    & ifmt_mva, { 0x1f400000 }
  },
/* lhu $r1,$gp16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (GP16), 0 } },
    & ifmt_mva, { 0x2f400000 }
  },
/* lw $r1,$gp16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (GP16), 0 } },
    & ifmt_mva, { 0x2b400000 }
  },
/* sb $gp16,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (GP16), ',', OP (R1), 0 } },
    & ifmt_mva, { 0x33400000 }
  },
/* sh $gp16,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (GP16), ',', OP (R1), 0 } },
    & ifmt_mva, { 0xf400000 }
  },
/* sw $gp16,$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (GP16), ',', OP (R1), 0 } },
    & ifmt_mva, { 0x5b400000 }
  },
/* lw $r1,(gp+$got16) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', 'g', 'p', '+', OP (GOT16), ')', 0 } },
    & ifmt_lwgotrel, { 0x2b400000 }
  },
/* orhi $r1,$r0,$gotoffhi16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (GOTOFFHI16), 0 } },
    & ifmt_orhigotoffi, { 0x78000000 }
  },
/* addi $r1,$r0,$gotofflo16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', OP (R0), ',', OP (GOTOFFLO16), 0 } },
    & ifmt_addgotoff, { 0x34000000 }
  },
/* sw ($r0+$gotofflo16),$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (R0), '+', OP (GOTOFFLO16), ')', ',', OP (R1), 0 } },
    & ifmt_addgotoff, { 0x58000000 }
  },
/* lw $r1,($r0+$gotofflo16) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (GOTOFFLO16), ')', 0 } },
    & ifmt_addgotoff, { 0x28000000 }
  },
/* sh ($r0+$gotofflo16),$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (R0), '+', OP (GOTOFFLO16), ')', ',', OP (R1), 0 } },
    & ifmt_addgotoff, { 0xc000000 }
  },
/* lh $r1,($r0+$gotofflo16) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (GOTOFFLO16), ')', 0 } },
    & ifmt_addgotoff, { 0x1c000000 }
  },
/* lhu $r1,($r0+$gotofflo16) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (GOTOFFLO16), ')', 0 } },
    & ifmt_addgotoff, { 0x2c000000 }
  },
/* sb ($r0+$gotofflo16),$r1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (R0), '+', OP (GOTOFFLO16), ')', ',', OP (R1), 0 } },
    & ifmt_addgotoff, { 0x30000000 }
  },
/* lb $r1,($r0+$gotofflo16) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (GOTOFFLO16), ')', 0 } },
    & ifmt_addgotoff, { 0x10000000 }
  },
/* lbu $r1,($r0+$gotofflo16) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (R1), ',', '(', OP (R0), '+', OP (GOTOFFLO16), ')', 0 } },
    & ifmt_addgotoff, { 0x40000000 }
  },
};

#undef A
#undef OPERAND
#undef MNEM
#undef OP

/* Formats for ALIAS macro-insns.  */

#define F(f) & lm32_cgen_ifld_table[LM32_##f]
#undef F

/* Each non-simple macro entry points to an array of expansion possibilities.  */

#define A(a) (1 << CGEN_INSN_##a)
#define OPERAND(op) LM32_OPERAND_##op
#define MNEM CGEN_SYNTAX_MNEMONIC /* syntax value for mnemonic */
#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))

/* The macro instruction table.  */

static const CGEN_IBASE lm32_cgen_macro_insn_table[] =
{
};

/* The macro instruction opcode table.  */

static const CGEN_OPCODE lm32_cgen_macro_insn_opcode_table[] =
{
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
lm32_cgen_init_opcode_table (CGEN_CPU_DESC cd)
{
  int i;
  int num_macros = (sizeof (lm32_cgen_macro_insn_table) /
		    sizeof (lm32_cgen_macro_insn_table[0]));
  const CGEN_IBASE *ib = & lm32_cgen_macro_insn_table[0];
  const CGEN_OPCODE *oc = & lm32_cgen_macro_insn_opcode_table[0];
  CGEN_INSN *insns = xmalloc (num_macros * sizeof (CGEN_INSN));

  /* This test has been added to avoid a warning generated
     if memset is called with a third argument of value zero.  */
  if (num_macros >= 1)
    memset (insns, 0, num_macros * sizeof (CGEN_INSN));
  for (i = 0; i < num_macros; ++i)
    {
      insns[i].base = &ib[i];
      insns[i].opcode = &oc[i];
      lm32_cgen_build_insn_regex (& insns[i]);
    }
  cd->macro_insn_table.init_entries = insns;
  cd->macro_insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->macro_insn_table.num_init_entries = num_macros;

  oc = & lm32_cgen_insn_opcode_table[0];
  insns = (CGEN_INSN *) cd->insn_table.init_entries;
  for (i = 0; i < MAX_INSNS; ++i)
    {
      insns[i].opcode = &oc[i];
      lm32_cgen_build_insn_regex (& insns[i]);
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
