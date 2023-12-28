/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* CPU data for lm32.

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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "ansidecl.h"
#include "bfd.h"
#include "symcat.h"
#include "lm32-desc.h"
#include "lm32-opc.h"
#include "opintl.h"
#include "libiberty.h"
#include "xregex.h"

/* Attributes.  */

static const CGEN_ATTR_ENTRY bool_attr[] =
{
  { "#f", 0 },
  { "#t", 1 },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY MACH_attr[] ATTRIBUTE_UNUSED =
{
  { "base", MACH_BASE },
  { "lm32", MACH_LM32 },
  { "max", MACH_MAX },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY ISA_attr[] ATTRIBUTE_UNUSED =
{
  { "lm32", ISA_LM32 },
  { "max", ISA_MAX },
  { 0, 0 }
};

const CGEN_ATTR_TABLE lm32_cgen_ifield_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "PCREL-ADDR", &bool_attr[0], &bool_attr[0] },
  { "ABS-ADDR", &bool_attr[0], &bool_attr[0] },
  { "RESERVED", &bool_attr[0], &bool_attr[0] },
  { "SIGN-OPT", &bool_attr[0], &bool_attr[0] },
  { "SIGNED", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE lm32_cgen_hardware_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "CACHE-ADDR", &bool_attr[0], &bool_attr[0] },
  { "PC", &bool_attr[0], &bool_attr[0] },
  { "PROFILE", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE lm32_cgen_operand_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "PCREL-ADDR", &bool_attr[0], &bool_attr[0] },
  { "ABS-ADDR", &bool_attr[0], &bool_attr[0] },
  { "SIGN-OPT", &bool_attr[0], &bool_attr[0] },
  { "SIGNED", &bool_attr[0], &bool_attr[0] },
  { "NEGATIVE", &bool_attr[0], &bool_attr[0] },
  { "RELAX", &bool_attr[0], &bool_attr[0] },
  { "SEM-ONLY", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE lm32_cgen_insn_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "ALIAS", &bool_attr[0], &bool_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "UNCOND-CTI", &bool_attr[0], &bool_attr[0] },
  { "COND-CTI", &bool_attr[0], &bool_attr[0] },
  { "SKIP-CTI", &bool_attr[0], &bool_attr[0] },
  { "DELAY-SLOT", &bool_attr[0], &bool_attr[0] },
  { "RELAXABLE", &bool_attr[0], &bool_attr[0] },
  { "RELAXED", &bool_attr[0], &bool_attr[0] },
  { "NO-DIS", &bool_attr[0], &bool_attr[0] },
  { "PBB", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

/* Instruction set variants.  */

static const CGEN_ISA lm32_cgen_isa_table[] = {
  { "lm32", 32, 32, 32, 32 },
  { 0, 0, 0, 0, 0 }
};

/* Machine variants.  */

static const CGEN_MACH lm32_cgen_mach_table[] = {
  { "lm32", "lm32", MACH_LM32, 0 },
  { 0, 0, 0, 0 }
};

static CGEN_KEYWORD_ENTRY lm32_cgen_opval_h_gr_entries[] =
{
  { "gp", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "fp", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "sp", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "ra", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "ea", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "ba", 31, {0, {{{0, 0}}}}, 0, 0 },
  { "r0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "r1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "r2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "r3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "r4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "r5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "r6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "r7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "r8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "r9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "r10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "r11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "r12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "r13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "r14", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "r15", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "r16", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "r17", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "r18", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "r19", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "r20", 20, {0, {{{0, 0}}}}, 0, 0 },
  { "r21", 21, {0, {{{0, 0}}}}, 0, 0 },
  { "r22", 22, {0, {{{0, 0}}}}, 0, 0 },
  { "r23", 23, {0, {{{0, 0}}}}, 0, 0 },
  { "r24", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "r25", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "r26", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "r27", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "r28", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "r29", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "r30", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "r31", 31, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD lm32_cgen_opval_h_gr =
{
  & lm32_cgen_opval_h_gr_entries[0],
  38,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY lm32_cgen_opval_h_csr_entries[] =
{
  { "IE", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "IM", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "IP", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "ICC", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "DCC", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "CC", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "CFG", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "EBA", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "DC", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "DEBA", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "CFG2", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "JTX", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "JRX", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "BP0", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "BP1", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "BP2", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "BP3", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "WP0", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "WP1", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "WP2", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "WP3", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "PSW", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "TLBVADDR", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "TLBPADDR", 31, {0, {{{0, 0}}}}, 0, 0 },
  { "TLBBADVADDR", 31, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD lm32_cgen_opval_h_csr =
{
  & lm32_cgen_opval_h_csr_entries[0],
  25,
  0, 0, 0, 0, ""
};


/* The hardware table.  */

#define A(a) (1 << CGEN_HW_##a)

const CGEN_HW_ENTRY lm32_cgen_hw_table[] =
{
  { "h-memory", HW_H_MEMORY, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-sint", HW_H_SINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-uint", HW_H_UINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-addr", HW_H_ADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-iaddr", HW_H_IADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-pc", HW_H_PC, CGEN_ASM_NONE, 0, { 0|A(PC), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-gr", HW_H_GR, CGEN_ASM_KEYWORD, & lm32_cgen_opval_h_gr, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-csr", HW_H_CSR, CGEN_ASM_KEYWORD, & lm32_cgen_opval_h_csr, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { 0, 0, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A


/* The instruction field table.  */

#define A(a) (1 << CGEN_IFLD_##a)

const CGEN_IFLD lm32_cgen_ifld_table[] =
{
  { LM32_F_NIL, "f-nil", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_ANYOF, "f-anyof", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_OPCODE, "f-opcode", 0, 32, 31, 6, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_R0, "f-r0", 0, 32, 25, 5, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_R1, "f-r1", 0, 32, 20, 5, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_R2, "f-r2", 0, 32, 15, 5, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_RESV0, "f-resv0", 0, 32, 10, 11, { 0|A(RESERVED), { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_SHIFT, "f-shift", 0, 32, 4, 5, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_IMM, "f-imm", 0, 32, 15, 16, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_UIMM, "f-uimm", 0, 32, 15, 16, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_CSR, "f-csr", 0, 32, 25, 5, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_USER, "f-user", 0, 32, 10, 11, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_EXCEPTION, "f-exception", 0, 32, 25, 26, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_BRANCH, "f-branch", 0, 32, 15, 16, { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } } } }  },
  { LM32_F_CALL, "f-call", 0, 32, 25, 26, { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } } } }  },
  { 0, 0, 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A



/* multi ifield declarations */



/* multi ifield definitions */


/* The operand table.  */

#define A(a) (1 << CGEN_OPERAND_##a)
#define OPERAND(op) LM32_OPERAND_##op

const CGEN_OPERAND lm32_cgen_operand_table[] =
{
/* pc: program counter */
  { "pc", LM32_OPERAND_PC, HW_H_PC, 0, 0,
    { 0, { &lm32_cgen_ifld_table[LM32_F_NIL] } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* r0: register 0 */
  { "r0", LM32_OPERAND_R0, HW_H_GR, 25, 5,
    { 0, { &lm32_cgen_ifld_table[LM32_F_R0] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* r1: register 1 */
  { "r1", LM32_OPERAND_R1, HW_H_GR, 20, 5,
    { 0, { &lm32_cgen_ifld_table[LM32_F_R1] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* r2: register 2 */
  { "r2", LM32_OPERAND_R2, HW_H_GR, 15, 5,
    { 0, { &lm32_cgen_ifld_table[LM32_F_R2] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* shift: shift amout */
  { "shift", LM32_OPERAND_SHIFT, HW_H_UINT, 4, 5,
    { 0, { &lm32_cgen_ifld_table[LM32_F_SHIFT] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* imm: signed immediate */
  { "imm", LM32_OPERAND_IMM, HW_H_SINT, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_IMM] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* uimm: unsigned immediate */
  { "uimm", LM32_OPERAND_UIMM, HW_H_UINT, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_UIMM] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* branch: branch offset */
  { "branch", LM32_OPERAND_BRANCH, HW_H_IADDR, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_BRANCH] } },
    { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } } } }  },
/* call: call offset */
  { "call", LM32_OPERAND_CALL, HW_H_IADDR, 25, 26,
    { 0, { &lm32_cgen_ifld_table[LM32_F_CALL] } },
    { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } } } }  },
/* csr: csr */
  { "csr", LM32_OPERAND_CSR, HW_H_CSR, 25, 5,
    { 0, { &lm32_cgen_ifld_table[LM32_F_CSR] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* user: user */
  { "user", LM32_OPERAND_USER, HW_H_UINT, 10, 11,
    { 0, { &lm32_cgen_ifld_table[LM32_F_USER] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* exception: exception */
  { "exception", LM32_OPERAND_EXCEPTION, HW_H_UINT, 25, 26,
    { 0, { &lm32_cgen_ifld_table[LM32_F_EXCEPTION] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* hi16: high 16-bit immediate */
  { "hi16", LM32_OPERAND_HI16, HW_H_UINT, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_UIMM] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* lo16: low 16-bit immediate */
  { "lo16", LM32_OPERAND_LO16, HW_H_UINT, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_UIMM] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* gp16: gp relative 16-bit immediate */
  { "gp16", LM32_OPERAND_GP16, HW_H_SINT, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_IMM] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* got16: got 16-bit immediate */
  { "got16", LM32_OPERAND_GOT16, HW_H_SINT, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_IMM] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* gotoffhi16: got offset high 16-bit immediate */
  { "gotoffhi16", LM32_OPERAND_GOTOFFHI16, HW_H_SINT, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_IMM] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* gotofflo16: got offset low 16-bit immediate */
  { "gotofflo16", LM32_OPERAND_GOTOFFLO16, HW_H_SINT, 15, 16,
    { 0, { &lm32_cgen_ifld_table[LM32_F_IMM] } },
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* sentinel */
  { 0, 0, 0, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A


/* The instruction table.  */

#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))
#define A(a) (1 << CGEN_INSN_##a)

static const CGEN_IBASE lm32_cgen_insn_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
/* add $r2,$r0,$r1 */
  {
    LM32_INSN_ADD, "add", "add", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addi $r1,$r0,$imm */
  {
    LM32_INSN_ADDI, "addi", "addi", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and $r2,$r0,$r1 */
  {
    LM32_INSN_AND, "and", "and", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* andi $r1,$r0,$uimm */
  {
    LM32_INSN_ANDI, "andi", "andi", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* andhi $r1,$r0,$hi16 */
  {
    LM32_INSN_ANDHII, "andhii", "andhi", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* b $r0 */
  {
    LM32_INSN_B, "b", "b", 32,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bi $call */
  {
    LM32_INSN_BI, "bi", "bi", 32,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* be $r0,$r1,$branch */
  {
    LM32_INSN_BE, "be", "be", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bg $r0,$r1,$branch */
  {
    LM32_INSN_BG, "bg", "bg", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bge $r0,$r1,$branch */
  {
    LM32_INSN_BGE, "bge", "bge", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgeu $r0,$r1,$branch */
  {
    LM32_INSN_BGEU, "bgeu", "bgeu", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bgu $r0,$r1,$branch */
  {
    LM32_INSN_BGU, "bgu", "bgu", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bne $r0,$r1,$branch */
  {
    LM32_INSN_BNE, "bne", "bne", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* call $r0 */
  {
    LM32_INSN_CALL, "call", "call", 32,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* calli $call */
  {
    LM32_INSN_CALLI, "calli", "calli", 32,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpe $r2,$r0,$r1 */
  {
    LM32_INSN_CMPE, "cmpe", "cmpe", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpei $r1,$r0,$imm */
  {
    LM32_INSN_CMPEI, "cmpei", "cmpei", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpg $r2,$r0,$r1 */
  {
    LM32_INSN_CMPG, "cmpg", "cmpg", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpgi $r1,$r0,$imm */
  {
    LM32_INSN_CMPGI, "cmpgi", "cmpgi", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpge $r2,$r0,$r1 */
  {
    LM32_INSN_CMPGE, "cmpge", "cmpge", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpgei $r1,$r0,$imm */
  {
    LM32_INSN_CMPGEI, "cmpgei", "cmpgei", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpgeu $r2,$r0,$r1 */
  {
    LM32_INSN_CMPGEU, "cmpgeu", "cmpgeu", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpgeui $r1,$r0,$uimm */
  {
    LM32_INSN_CMPGEUI, "cmpgeui", "cmpgeui", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpgu $r2,$r0,$r1 */
  {
    LM32_INSN_CMPGU, "cmpgu", "cmpgu", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpgui $r1,$r0,$uimm */
  {
    LM32_INSN_CMPGUI, "cmpgui", "cmpgui", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpne $r2,$r0,$r1 */
  {
    LM32_INSN_CMPNE, "cmpne", "cmpne", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpnei $r1,$r0,$imm */
  {
    LM32_INSN_CMPNEI, "cmpnei", "cmpnei", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* divu $r2,$r0,$r1 */
  {
    LM32_INSN_DIVU, "divu", "divu", 32,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lb $r1,($r0+$imm) */
  {
    LM32_INSN_LB, "lb", "lb", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lbu $r1,($r0+$imm) */
  {
    LM32_INSN_LBU, "lbu", "lbu", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lh $r1,($r0+$imm) */
  {
    LM32_INSN_LH, "lh", "lh", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lhu $r1,($r0+$imm) */
  {
    LM32_INSN_LHU, "lhu", "lhu", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lw $r1,($r0+$imm) */
  {
    LM32_INSN_LW, "lw", "lw", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* modu $r2,$r0,$r1 */
  {
    LM32_INSN_MODU, "modu", "modu", 32,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mul $r2,$r0,$r1 */
  {
    LM32_INSN_MUL, "mul", "mul", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* muli $r1,$r0,$imm */
  {
    LM32_INSN_MULI, "muli", "muli", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* nor $r2,$r0,$r1 */
  {
    LM32_INSN_NOR, "nor", "nor", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* nori $r1,$r0,$uimm */
  {
    LM32_INSN_NORI, "nori", "nori", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* or $r2,$r0,$r1 */
  {
    LM32_INSN_OR, "or", "or", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* ori $r1,$r0,$lo16 */
  {
    LM32_INSN_ORI, "ori", "ori", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* orhi $r1,$r0,$hi16 */
  {
    LM32_INSN_ORHII, "orhii", "orhi", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* rcsr $r2,$csr */
  {
    LM32_INSN_RCSR, "rcsr", "rcsr", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sb ($r0+$imm),$r1 */
  {
    LM32_INSN_SB, "sb", "sb", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sextb $r2,$r0 */
  {
    LM32_INSN_SEXTB, "sextb", "sextb", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sexth $r2,$r0 */
  {
    LM32_INSN_SEXTH, "sexth", "sexth", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sh ($r0+$imm),$r1 */
  {
    LM32_INSN_SH, "sh", "sh", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sl $r2,$r0,$r1 */
  {
    LM32_INSN_SL, "sl", "sl", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sli $r1,$r0,$imm */
  {
    LM32_INSN_SLI, "sli", "sli", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sr $r2,$r0,$r1 */
  {
    LM32_INSN_SR, "sr", "sr", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sri $r1,$r0,$imm */
  {
    LM32_INSN_SRI, "sri", "sri", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sru $r2,$r0,$r1 */
  {
    LM32_INSN_SRU, "sru", "sru", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* srui $r1,$r0,$imm */
  {
    LM32_INSN_SRUI, "srui", "srui", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub $r2,$r0,$r1 */
  {
    LM32_INSN_SUB, "sub", "sub", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sw ($r0+$imm),$r1 */
  {
    LM32_INSN_SW, "sw", "sw", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* user $r2,$r0,$r1,$user */
  {
    LM32_INSN_USER, "user", "user", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* wcsr $csr,$r1 */
  {
    LM32_INSN_WCSR, "wcsr", "wcsr", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* xor $r2,$r0,$r1 */
  {
    LM32_INSN_XOR, "xor", "xor", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* xori $r1,$r0,$uimm */
  {
    LM32_INSN_XORI, "xori", "xori", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* xnor $r2,$r0,$r1 */
  {
    LM32_INSN_XNOR, "xnor", "xnor", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* xnori $r1,$r0,$uimm */
  {
    LM32_INSN_XNORI, "xnori", "xnori", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* break */
  {
    LM32_INSN_BREAK, "break", "break", 32,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* scall */
  {
    LM32_INSN_SCALL, "scall", "scall", 32,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bret */
  {
    -1, "bret", "bret", 32,
    { 0|A(ALIAS)|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* eret */
  {
    -1, "eret", "eret", 32,
    { 0|A(ALIAS)|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ret */
  {
    -1, "ret", "ret", 32,
    { 0|A(ALIAS)|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mv $r2,$r0 */
  {
    -1, "mv", "mv", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mvi $r1,$imm */
  {
    -1, "mvi", "mvi", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mvu $r1,$lo16 */
  {
    -1, "mvui", "mvu", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mvhi $r1,$hi16 */
  {
    -1, "mvhi", "mvhi", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* mva $r1,$gp16 */
  {
    -1, "mva", "mva", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* not $r2,$r0 */
  {
    -1, "not", "not", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* nop */
  {
    -1, "nop", "nop", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lb $r1,$gp16 */
  {
    -1, "lbgprel", "lb", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lbu $r1,$gp16 */
  {
    -1, "lbugprel", "lbu", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lh $r1,$gp16 */
  {
    -1, "lhgprel", "lh", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lhu $r1,$gp16 */
  {
    -1, "lhugprel", "lhu", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lw $r1,$gp16 */
  {
    -1, "lwgprel", "lw", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sb $gp16,$r1 */
  {
    -1, "sbgprel", "sb", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sh $gp16,$r1 */
  {
    -1, "shgprel", "sh", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sw $gp16,$r1 */
  {
    -1, "swgprel", "sw", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lw $r1,(gp+$got16) */
  {
    -1, "lwgotrel", "lw", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* orhi $r1,$r0,$gotoffhi16 */
  {
    -1, "orhigotoffi", "orhi", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* addi $r1,$r0,$gotofflo16 */
  {
    -1, "addgotoff", "addi", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sw ($r0+$gotofflo16),$r1 */
  {
    -1, "swgotoff", "sw", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lw $r1,($r0+$gotofflo16) */
  {
    -1, "lwgotoff", "lw", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sh ($r0+$gotofflo16),$r1 */
  {
    -1, "shgotoff", "sh", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lh $r1,($r0+$gotofflo16) */
  {
    -1, "lhgotoff", "lh", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lhu $r1,($r0+$gotofflo16) */
  {
    -1, "lhugotoff", "lhu", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* sb ($r0+$gotofflo16),$r1 */
  {
    -1, "sbgotoff", "sb", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lb $r1,($r0+$gotofflo16) */
  {
    -1, "lbgotoff", "lb", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
/* lbu $r1,($r0+$gotofflo16) */
  {
    -1, "lbugotoff", "lbu", 32,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } } } }
  },
};

#undef OP
#undef A

/* Initialize anything needed to be done once, before any cpu_open call.  */

static void
init_tables (void)
{
}

#ifndef opcodes_error_handler
#define opcodes_error_handler(...) \
  fprintf (stderr, __VA_ARGS__); fputc ('\n', stderr)
#endif

static const CGEN_MACH * lookup_mach_via_bfd_name (const CGEN_MACH *, const char *);
static void build_hw_table      (CGEN_CPU_TABLE *);
static void build_ifield_table  (CGEN_CPU_TABLE *);
static void build_operand_table (CGEN_CPU_TABLE *);
static void build_insn_table    (CGEN_CPU_TABLE *);
static void lm32_cgen_rebuild_tables (CGEN_CPU_TABLE *);

/* Subroutine of lm32_cgen_cpu_open to look up a mach via its bfd name.  */

static const CGEN_MACH *
lookup_mach_via_bfd_name (const CGEN_MACH *table, const char *name)
{
  while (table->name)
    {
      if (strcmp (name, table->bfd_name) == 0)
	return table;
      ++table;
    }
  return NULL;
}

/* Subroutine of lm32_cgen_cpu_open to build the hardware table.  */

static void
build_hw_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_HW_ENTRY *init = & lm32_cgen_hw_table[0];
  /* MAX_HW is only an upper bound on the number of selected entries.
     However each entry is indexed by it's enum so there can be holes in
     the table.  */
  const CGEN_HW_ENTRY **selected =
    (const CGEN_HW_ENTRY **) xmalloc (MAX_HW * sizeof (CGEN_HW_ENTRY *));

  cd->hw_table.init_entries = init;
  cd->hw_table.entry_size = sizeof (CGEN_HW_ENTRY);
  memset (selected, 0, MAX_HW * sizeof (CGEN_HW_ENTRY *));
  /* ??? For now we just use machs to determine which ones we want.  */
  for (i = 0; init[i].name != NULL; ++i)
    if (CGEN_HW_ATTR_VALUE (&init[i], CGEN_HW_MACH)
	& machs)
      selected[init[i].type] = &init[i];
  cd->hw_table.entries = selected;
  cd->hw_table.num_entries = MAX_HW;
}

/* Subroutine of lm32_cgen_cpu_open to build the hardware table.  */

static void
build_ifield_table (CGEN_CPU_TABLE *cd)
{
  cd->ifld_table = & lm32_cgen_ifld_table[0];
}

/* Subroutine of lm32_cgen_cpu_open to build the hardware table.  */

static void
build_operand_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_OPERAND *init = & lm32_cgen_operand_table[0];
  /* MAX_OPERANDS is only an upper bound on the number of selected entries.
     However each entry is indexed by it's enum so there can be holes in
     the table.  */
  const CGEN_OPERAND **selected = xmalloc (MAX_OPERANDS * sizeof (* selected));

  cd->operand_table.init_entries = init;
  cd->operand_table.entry_size = sizeof (CGEN_OPERAND);
  memset (selected, 0, MAX_OPERANDS * sizeof (CGEN_OPERAND *));
  /* ??? For now we just use mach to determine which ones we want.  */
  for (i = 0; init[i].name != NULL; ++i)
    if (CGEN_OPERAND_ATTR_VALUE (&init[i], CGEN_OPERAND_MACH)
	& machs)
      selected[init[i].type] = &init[i];
  cd->operand_table.entries = selected;
  cd->operand_table.num_entries = MAX_OPERANDS;
}

/* Subroutine of lm32_cgen_cpu_open to build the hardware table.
   ??? This could leave out insns not supported by the specified mach/isa,
   but that would cause errors like "foo only supported by bar" to become
   "unknown insn", so for now we include all insns and require the app to
   do the checking later.
   ??? On the other hand, parsing of such insns may require their hardware or
   operand elements to be in the table [which they mightn't be].  */

static void
build_insn_table (CGEN_CPU_TABLE *cd)
{
  int i;
  const CGEN_IBASE *ib = & lm32_cgen_insn_table[0];
  CGEN_INSN *insns = xmalloc (MAX_INSNS * sizeof (CGEN_INSN));

  memset (insns, 0, MAX_INSNS * sizeof (CGEN_INSN));
  for (i = 0; i < MAX_INSNS; ++i)
    insns[i].base = &ib[i];
  cd->insn_table.init_entries = insns;
  cd->insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->insn_table.num_init_entries = MAX_INSNS;
}

/* Subroutine of lm32_cgen_cpu_open to rebuild the tables.  */

static void
lm32_cgen_rebuild_tables (CGEN_CPU_TABLE *cd)
{
  int i;
  CGEN_BITSET *isas = cd->isas;
  unsigned int machs = cd->machs;

  cd->int_insn_p = CGEN_INT_INSN_P;

  /* Data derived from the isa spec.  */
#define UNSET (CGEN_SIZE_UNKNOWN + 1)
  cd->default_insn_bitsize = UNSET;
  cd->base_insn_bitsize = UNSET;
  cd->min_insn_bitsize = 65535; /* Some ridiculously big number.  */
  cd->max_insn_bitsize = 0;
  for (i = 0; i < MAX_ISAS; ++i)
    if (cgen_bitset_contains (isas, i))
      {
	const CGEN_ISA *isa = & lm32_cgen_isa_table[i];

	/* Default insn sizes of all selected isas must be
	   equal or we set the result to 0, meaning "unknown".  */
	if (cd->default_insn_bitsize == UNSET)
	  cd->default_insn_bitsize = isa->default_insn_bitsize;
	else if (isa->default_insn_bitsize == cd->default_insn_bitsize)
	  ; /* This is ok.  */
	else
	  cd->default_insn_bitsize = CGEN_SIZE_UNKNOWN;

	/* Base insn sizes of all selected isas must be equal
	   or we set the result to 0, meaning "unknown".  */
	if (cd->base_insn_bitsize == UNSET)
	  cd->base_insn_bitsize = isa->base_insn_bitsize;
	else if (isa->base_insn_bitsize == cd->base_insn_bitsize)
	  ; /* This is ok.  */
	else
	  cd->base_insn_bitsize = CGEN_SIZE_UNKNOWN;

	/* Set min,max insn sizes.  */
	if (isa->min_insn_bitsize < cd->min_insn_bitsize)
	  cd->min_insn_bitsize = isa->min_insn_bitsize;
	if (isa->max_insn_bitsize > cd->max_insn_bitsize)
	  cd->max_insn_bitsize = isa->max_insn_bitsize;
      }

  /* Data derived from the mach spec.  */
  for (i = 0; i < MAX_MACHS; ++i)
    if (((1 << i) & machs) != 0)
      {
	const CGEN_MACH *mach = & lm32_cgen_mach_table[i];

	if (mach->insn_chunk_bitsize != 0)
	{
	  if (cd->insn_chunk_bitsize != 0 && cd->insn_chunk_bitsize != mach->insn_chunk_bitsize)
	    {
	      opcodes_error_handler
		(/* xgettext:c-format */
		 _("internal error: lm32_cgen_rebuild_tables: "
		   "conflicting insn-chunk-bitsize values: `%d' vs. `%d'"),
		 cd->insn_chunk_bitsize, mach->insn_chunk_bitsize);
	      abort ();
	    }

 	  cd->insn_chunk_bitsize = mach->insn_chunk_bitsize;
	}
      }

  /* Determine which hw elements are used by MACH.  */
  build_hw_table (cd);

  /* Build the ifield table.  */
  build_ifield_table (cd);

  /* Determine which operands are used by MACH/ISA.  */
  build_operand_table (cd);

  /* Build the instruction table.  */
  build_insn_table (cd);
}

/* Initialize a cpu table and return a descriptor.
   It's much like opening a file, and must be the first function called.
   The arguments are a set of (type/value) pairs, terminated with
   CGEN_CPU_OPEN_END.

   Currently supported values:
   CGEN_CPU_OPEN_ISAS:    bitmap of values in enum isa_attr
   CGEN_CPU_OPEN_MACHS:   bitmap of values in enum mach_attr
   CGEN_CPU_OPEN_BFDMACH: specify 1 mach using bfd name
   CGEN_CPU_OPEN_ENDIAN:  specify endian choice
   CGEN_CPU_OPEN_INSN_ENDIAN: specify instruction endian choice
   CGEN_CPU_OPEN_END:     terminates arguments

   ??? Simultaneous multiple isas might not make sense, but it's not (yet)
   precluded.  */

CGEN_CPU_DESC
lm32_cgen_cpu_open (enum cgen_cpu_open_arg arg_type, ...)
{
  CGEN_CPU_TABLE *cd = (CGEN_CPU_TABLE *) xmalloc (sizeof (CGEN_CPU_TABLE));
  static int init_p;
  CGEN_BITSET *isas = 0;  /* 0 = "unspecified" */
  unsigned int machs = 0; /* 0 = "unspecified" */
  enum cgen_endian endian = CGEN_ENDIAN_UNKNOWN;
  enum cgen_endian insn_endian = CGEN_ENDIAN_UNKNOWN;
  va_list ap;

  if (! init_p)
    {
      init_tables ();
      init_p = 1;
    }

  memset (cd, 0, sizeof (*cd));

  va_start (ap, arg_type);
  while (arg_type != CGEN_CPU_OPEN_END)
    {
      switch (arg_type)
	{
	case CGEN_CPU_OPEN_ISAS :
	  isas = va_arg (ap, CGEN_BITSET *);
	  break;
	case CGEN_CPU_OPEN_MACHS :
	  machs = va_arg (ap, unsigned int);
	  break;
	case CGEN_CPU_OPEN_BFDMACH :
	  {
	    const char *name = va_arg (ap, const char *);
	    const CGEN_MACH *mach =
	      lookup_mach_via_bfd_name (lm32_cgen_mach_table, name);

	    if (mach != NULL)
	      machs |= 1 << mach->num;
	    break;
	  }
	case CGEN_CPU_OPEN_ENDIAN :
	  endian = va_arg (ap, enum cgen_endian);
	  break;
	case CGEN_CPU_OPEN_INSN_ENDIAN :
	  insn_endian = va_arg (ap, enum cgen_endian);
	  break;
	default :
	  opcodes_error_handler
	    (/* xgettext:c-format */
	     _("internal error: lm32_cgen_cpu_open: "
	       "unsupported argument `%d'"),
	     arg_type);
	  abort (); /* ??? return NULL? */
	}
      arg_type = va_arg (ap, enum cgen_cpu_open_arg);
    }
  va_end (ap);

  /* Mach unspecified means "all".  */
  if (machs == 0)
    machs = (1 << MAX_MACHS) - 1;
  /* Base mach is always selected.  */
  machs |= 1;
  if (endian == CGEN_ENDIAN_UNKNOWN)
    {
      /* ??? If target has only one, could have a default.  */
      opcodes_error_handler
	(/* xgettext:c-format */
	 _("internal error: lm32_cgen_cpu_open: no endianness specified"));
      abort ();
    }

  cd->isas = cgen_bitset_copy (isas);
  cd->machs = machs;
  cd->endian = endian;
  cd->insn_endian
    = (insn_endian == CGEN_ENDIAN_UNKNOWN ? endian : insn_endian);

  /* Table (re)builder.  */
  cd->rebuild_tables = lm32_cgen_rebuild_tables;
  lm32_cgen_rebuild_tables (cd);

  /* Default to not allowing signed overflow.  */
  cd->signed_overflow_ok_p = 0;

  return (CGEN_CPU_DESC) cd;
}

/* Cover fn to lm32_cgen_cpu_open to handle the simple case of 1 isa, 1 mach.
   MACH_NAME is the bfd name of the mach.  */

CGEN_CPU_DESC
lm32_cgen_cpu_open_1 (const char *mach_name, enum cgen_endian endian)
{
  return lm32_cgen_cpu_open (CGEN_CPU_OPEN_BFDMACH, mach_name,
			       CGEN_CPU_OPEN_ENDIAN, endian,
			       CGEN_CPU_OPEN_END);
}

/* Close a cpu table.
   ??? This can live in a machine independent file, but there's currently
   no place to put this file (there's no libcgen).  libopcodes is the wrong
   place as some simulator ports use this but they don't use libopcodes.  */

void
lm32_cgen_cpu_close (CGEN_CPU_DESC cd)
{
  unsigned int i;
  const CGEN_INSN *insns;

  if (cd->macro_insn_table.init_entries)
    {
      insns = cd->macro_insn_table.init_entries;
      for (i = 0; i < cd->macro_insn_table.num_init_entries; ++i, ++insns)
	if (CGEN_INSN_RX ((insns)))
	  regfree (CGEN_INSN_RX (insns));
    }

  if (cd->insn_table.init_entries)
    {
      insns = cd->insn_table.init_entries;
      for (i = 0; i < cd->insn_table.num_init_entries; ++i, ++insns)
	if (CGEN_INSN_RX (insns))
	  regfree (CGEN_INSN_RX (insns));
    }

  free ((CGEN_INSN *) cd->macro_insn_table.init_entries);
  free ((CGEN_INSN *) cd->insn_table.init_entries);
  free ((CGEN_HW_ENTRY *) cd->hw_table.entries);
  free ((CGEN_HW_ENTRY *) cd->operand_table.entries);
  free (cd);
}

