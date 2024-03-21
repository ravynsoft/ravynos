/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* CPU data for bpf.

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
#include "bpf-desc.h"
#include "bpf-opc.h"
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
  { "bpf", MACH_BPF },
  { "xbpf", MACH_XBPF },
  { "max", MACH_MAX },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY ISA_attr[] ATTRIBUTE_UNUSED =
{
  { "ebpfle", ISA_EBPFLE },
  { "ebpfbe", ISA_EBPFBE },
  { "xbpfle", ISA_XBPFLE },
  { "xbpfbe", ISA_XBPFBE },
  { "max", ISA_MAX },
  { 0, 0 }
};

const CGEN_ATTR_TABLE bpf_cgen_ifield_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "ISA", & ISA_attr[0], & ISA_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "PCREL-ADDR", &bool_attr[0], &bool_attr[0] },
  { "ABS-ADDR", &bool_attr[0], &bool_attr[0] },
  { "RESERVED", &bool_attr[0], &bool_attr[0] },
  { "SIGN-OPT", &bool_attr[0], &bool_attr[0] },
  { "SIGNED", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE bpf_cgen_hardware_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "ISA", & ISA_attr[0], & ISA_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "CACHE-ADDR", &bool_attr[0], &bool_attr[0] },
  { "PC", &bool_attr[0], &bool_attr[0] },
  { "PROFILE", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE bpf_cgen_operand_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "ISA", & ISA_attr[0], & ISA_attr[0] },
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

const CGEN_ATTR_TABLE bpf_cgen_insn_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "ISA", & ISA_attr[0], & ISA_attr[0] },
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

static const CGEN_ISA bpf_cgen_isa_table[] = {
  { "ebpfle", 64, 64, 64, 128 },
  { "ebpfbe", 64, 64, 64, 128 },
  { "xbpfle", 64, 64, 64, 128 },
  { "xbpfbe", 64, 64, 64, 128 },
  { 0, 0, 0, 0, 0 }
};

/* Machine variants.  */

static const CGEN_MACH bpf_cgen_mach_table[] = {
  { "bpf", "bpf", MACH_BPF, 0 },
  { "xbpf", "xbpf", MACH_XBPF, 0 },
  { 0, 0, 0, 0 }
};

static CGEN_KEYWORD_ENTRY bpf_cgen_opval_h_gpr_entries[] =
{
  { "%r0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "%r1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "%r2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "%r3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "%r4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "%r5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "%r6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "%r7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "%r8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "%r9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "%fp", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "%r0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "%r6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "%r10", 10, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD bpf_cgen_opval_h_gpr =
{
  & bpf_cgen_opval_h_gpr_entries[0],
  14,
  0, 0, 0, 0, ""
};


/* The hardware table.  */

#define A(a) (1 << CGEN_HW_##a)

const CGEN_HW_ENTRY bpf_cgen_hw_table[] =
{
  { "h-memory", HW_H_MEMORY, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } } },
  { "h-sint", HW_H_SINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } } },
  { "h-uint", HW_H_UINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } } },
  { "h-addr", HW_H_ADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } } },
  { "h-iaddr", HW_H_IADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } } },
  { "h-gpr", HW_H_GPR, CGEN_ASM_KEYWORD, & bpf_cgen_opval_h_gpr, { 0, { { { (1<<MACH_BPF)|(1<<MACH_XBPF), 0 } }, { { 1, "\xf0" } } } } },
  { "h-pc", HW_H_PC, CGEN_ASM_NONE, 0, { 0|A(PROFILE)|A(PC), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } } },
  { "h-sint64", HW_H_SINT64, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } } },
  { 0, 0, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } } } } }
};

#undef A


/* The instruction field table.  */

#define A(a) (1 << CGEN_IFLD_##a)

const CGEN_IFLD bpf_cgen_ifld_table[] =
{
  { BPF_F_NIL, "f-nil", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_ANYOF, "f-anyof", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_OP_CODE, "f-op-code", 0, 8, 7, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_OP_SRC, "f-op-src", 0, 8, 3, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_OP_CLASS, "f-op-class", 0, 8, 2, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_OP_MODE, "f-op-mode", 0, 8, 7, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_OP_SIZE, "f-op-size", 0, 8, 4, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_DSTLE, "f-dstle", 8, 8, 3, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }  },
  { BPF_F_SRCLE, "f-srcle", 8, 8, 7, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }  },
  { BPF_F_DSTBE, "f-dstbe", 8, 8, 7, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }  },
  { BPF_F_SRCBE, "f-srcbe", 8, 8, 3, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }  },
  { BPF_F_REGS, "f-regs", 8, 8, 7, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_OFFSET16, "f-offset16", 16, 16, 15, 16, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_IMM32, "f-imm32", 32, 32, 31, 32, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_IMM64_A, "f-imm64-a", 32, 32, 31, 32, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_IMM64_B, "f-imm64-b", 64, 32, 31, 32, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_IMM64_C, "f-imm64-c", 96, 32, 31, 32, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { BPF_F_IMM64, "f-imm64", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
  { 0, 0, 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } } } } }
};

#undef A



/* multi ifield declarations */

const CGEN_MAYBE_MULTI_IFLD BPF_F_IMM64_MULTI_IFIELD [];


/* multi ifield definitions */

const CGEN_MAYBE_MULTI_IFLD BPF_F_IMM64_MULTI_IFIELD [] =
{
    { 0, { &bpf_cgen_ifld_table[BPF_F_IMM64_A] } },
    { 0, { &bpf_cgen_ifld_table[BPF_F_IMM64_B] } },
    { 0, { &bpf_cgen_ifld_table[BPF_F_IMM64_C] } },
    { 0, { 0 } }
};

/* The operand table.  */

#define A(a) (1 << CGEN_OPERAND_##a)
#define OPERAND(op) BPF_OPERAND_##op

const CGEN_OPERAND bpf_cgen_operand_table[] =
{
/* pc: program counter */
  { "pc", BPF_OPERAND_PC, HW_H_PC, 0, 0,
    { 0, { &bpf_cgen_ifld_table[BPF_F_NIL] } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
/* dstle: destination register */
  { "dstle", BPF_OPERAND_DSTLE, HW_H_GPR, 3, 4,
    { 0, { &bpf_cgen_ifld_table[BPF_F_DSTLE] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }  },
/* srcle: source register */
  { "srcle", BPF_OPERAND_SRCLE, HW_H_GPR, 7, 4,
    { 0, { &bpf_cgen_ifld_table[BPF_F_SRCLE] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }  },
/* dstbe: destination register */
  { "dstbe", BPF_OPERAND_DSTBE, HW_H_GPR, 7, 4,
    { 0, { &bpf_cgen_ifld_table[BPF_F_DSTBE] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }  },
/* srcbe: source register */
  { "srcbe", BPF_OPERAND_SRCBE, HW_H_GPR, 3, 4,
    { 0, { &bpf_cgen_ifld_table[BPF_F_SRCBE] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }  },
/* disp16: 16-bit PC-relative address */
  { "disp16", BPF_OPERAND_DISP16, HW_H_SINT, 15, 16,
    { 0, { &bpf_cgen_ifld_table[BPF_F_OFFSET16] } },
    { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
/* disp32: 32-bit PC-relative address */
  { "disp32", BPF_OPERAND_DISP32, HW_H_SINT, 31, 32,
    { 0, { &bpf_cgen_ifld_table[BPF_F_IMM32] } },
    { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
/* imm32: 32-bit immediate */
  { "imm32", BPF_OPERAND_IMM32, HW_H_SINT, 31, 32,
    { 0, { &bpf_cgen_ifld_table[BPF_F_IMM32] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
/* offset16: 16-bit offset */
  { "offset16", BPF_OPERAND_OFFSET16, HW_H_SINT, 15, 16,
    { 0, { &bpf_cgen_ifld_table[BPF_F_OFFSET16] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
/* imm64: 64-bit immediate */
  { "imm64", BPF_OPERAND_IMM64, HW_H_SINT64, 31, 96,
    { 3, { &BPF_F_IMM64_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
/* endsize: endianness size immediate: 16, 32 or 64 */
  { "endsize", BPF_OPERAND_ENDSIZE, HW_H_UINT, 31, 32,
    { 0, { &bpf_cgen_ifld_table[BPF_F_IMM32] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }  },
/* sentinel */
  { 0, 0, 0, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } } } } }
};

#undef A


/* The instruction table.  */

#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))
#define A(a) (1 << CGEN_INSN_##a)

static const CGEN_IBASE bpf_cgen_insn_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } } } } },
/* add $dstle,$imm32 */
  {
    BPF_INSN_ADDILE, "addile", "add", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* add $dstle,$srcle */
  {
    BPF_INSN_ADDRLE, "addrle", "add", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* add32 $dstle,$imm32 */
  {
    BPF_INSN_ADD32ILE, "add32ile", "add32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* add32 $dstle,$srcle */
  {
    BPF_INSN_ADD32RLE, "add32rle", "add32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* sub $dstle,$imm32 */
  {
    BPF_INSN_SUBILE, "subile", "sub", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* sub $dstle,$srcle */
  {
    BPF_INSN_SUBRLE, "subrle", "sub", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* sub32 $dstle,$imm32 */
  {
    BPF_INSN_SUB32ILE, "sub32ile", "sub32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* sub32 $dstle,$srcle */
  {
    BPF_INSN_SUB32RLE, "sub32rle", "sub32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mul $dstle,$imm32 */
  {
    BPF_INSN_MULILE, "mulile", "mul", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mul $dstle,$srcle */
  {
    BPF_INSN_MULRLE, "mulrle", "mul", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mul32 $dstle,$imm32 */
  {
    BPF_INSN_MUL32ILE, "mul32ile", "mul32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mul32 $dstle,$srcle */
  {
    BPF_INSN_MUL32RLE, "mul32rle", "mul32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* div $dstle,$imm32 */
  {
    BPF_INSN_DIVILE, "divile", "div", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* div $dstle,$srcle */
  {
    BPF_INSN_DIVRLE, "divrle", "div", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* div32 $dstle,$imm32 */
  {
    BPF_INSN_DIV32ILE, "div32ile", "div32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* div32 $dstle,$srcle */
  {
    BPF_INSN_DIV32RLE, "div32rle", "div32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* or $dstle,$imm32 */
  {
    BPF_INSN_ORILE, "orile", "or", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* or $dstle,$srcle */
  {
    BPF_INSN_ORRLE, "orrle", "or", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* or32 $dstle,$imm32 */
  {
    BPF_INSN_OR32ILE, "or32ile", "or32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* or32 $dstle,$srcle */
  {
    BPF_INSN_OR32RLE, "or32rle", "or32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* and $dstle,$imm32 */
  {
    BPF_INSN_ANDILE, "andile", "and", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* and $dstle,$srcle */
  {
    BPF_INSN_ANDRLE, "andrle", "and", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* and32 $dstle,$imm32 */
  {
    BPF_INSN_AND32ILE, "and32ile", "and32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* and32 $dstle,$srcle */
  {
    BPF_INSN_AND32RLE, "and32rle", "and32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* lsh $dstle,$imm32 */
  {
    BPF_INSN_LSHILE, "lshile", "lsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* lsh $dstle,$srcle */
  {
    BPF_INSN_LSHRLE, "lshrle", "lsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* lsh32 $dstle,$imm32 */
  {
    BPF_INSN_LSH32ILE, "lsh32ile", "lsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* lsh32 $dstle,$srcle */
  {
    BPF_INSN_LSH32RLE, "lsh32rle", "lsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* rsh $dstle,$imm32 */
  {
    BPF_INSN_RSHILE, "rshile", "rsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* rsh $dstle,$srcle */
  {
    BPF_INSN_RSHRLE, "rshrle", "rsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* rsh32 $dstle,$imm32 */
  {
    BPF_INSN_RSH32ILE, "rsh32ile", "rsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* rsh32 $dstle,$srcle */
  {
    BPF_INSN_RSH32RLE, "rsh32rle", "rsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mod $dstle,$imm32 */
  {
    BPF_INSN_MODILE, "modile", "mod", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mod $dstle,$srcle */
  {
    BPF_INSN_MODRLE, "modrle", "mod", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mod32 $dstle,$imm32 */
  {
    BPF_INSN_MOD32ILE, "mod32ile", "mod32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mod32 $dstle,$srcle */
  {
    BPF_INSN_MOD32RLE, "mod32rle", "mod32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* xor $dstle,$imm32 */
  {
    BPF_INSN_XORILE, "xorile", "xor", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* xor $dstle,$srcle */
  {
    BPF_INSN_XORRLE, "xorrle", "xor", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* xor32 $dstle,$imm32 */
  {
    BPF_INSN_XOR32ILE, "xor32ile", "xor32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* xor32 $dstle,$srcle */
  {
    BPF_INSN_XOR32RLE, "xor32rle", "xor32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* arsh $dstle,$imm32 */
  {
    BPF_INSN_ARSHILE, "arshile", "arsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* arsh $dstle,$srcle */
  {
    BPF_INSN_ARSHRLE, "arshrle", "arsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* arsh32 $dstle,$imm32 */
  {
    BPF_INSN_ARSH32ILE, "arsh32ile", "arsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* arsh32 $dstle,$srcle */
  {
    BPF_INSN_ARSH32RLE, "arsh32rle", "arsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* sdiv $dstle,$imm32 */
  {
    BPF_INSN_SDIVILE, "sdivile", "sdiv", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* sdiv $dstle,$srcle */
  {
    BPF_INSN_SDIVRLE, "sdivrle", "sdiv", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* sdiv32 $dstle,$imm32 */
  {
    BPF_INSN_SDIV32ILE, "sdiv32ile", "sdiv32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* sdiv32 $dstle,$srcle */
  {
    BPF_INSN_SDIV32RLE, "sdiv32rle", "sdiv32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* smod $dstle,$imm32 */
  {
    BPF_INSN_SMODILE, "smodile", "smod", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* smod $dstle,$srcle */
  {
    BPF_INSN_SMODRLE, "smodrle", "smod", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* smod32 $dstle,$imm32 */
  {
    BPF_INSN_SMOD32ILE, "smod32ile", "smod32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* smod32 $dstle,$srcle */
  {
    BPF_INSN_SMOD32RLE, "smod32rle", "smod32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* neg $dstle */
  {
    BPF_INSN_NEGLE, "negle", "neg", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* neg32 $dstle */
  {
    BPF_INSN_NEG32LE, "neg32le", "neg32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mov $dstle,$imm32 */
  {
    BPF_INSN_MOVILE, "movile", "mov", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mov $dstle,$srcle */
  {
    BPF_INSN_MOVRLE, "movrle", "mov", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mov32 $dstle,$imm32 */
  {
    BPF_INSN_MOV32ILE, "mov32ile", "mov32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* mov32 $dstle,$srcle */
  {
    BPF_INSN_MOV32RLE, "mov32rle", "mov32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* add $dstbe,$imm32 */
  {
    BPF_INSN_ADDIBE, "addibe", "add", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* add $dstbe,$srcbe */
  {
    BPF_INSN_ADDRBE, "addrbe", "add", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* add32 $dstbe,$imm32 */
  {
    BPF_INSN_ADD32IBE, "add32ibe", "add32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* add32 $dstbe,$srcbe */
  {
    BPF_INSN_ADD32RBE, "add32rbe", "add32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* sub $dstbe,$imm32 */
  {
    BPF_INSN_SUBIBE, "subibe", "sub", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* sub $dstbe,$srcbe */
  {
    BPF_INSN_SUBRBE, "subrbe", "sub", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* sub32 $dstbe,$imm32 */
  {
    BPF_INSN_SUB32IBE, "sub32ibe", "sub32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* sub32 $dstbe,$srcbe */
  {
    BPF_INSN_SUB32RBE, "sub32rbe", "sub32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mul $dstbe,$imm32 */
  {
    BPF_INSN_MULIBE, "mulibe", "mul", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mul $dstbe,$srcbe */
  {
    BPF_INSN_MULRBE, "mulrbe", "mul", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mul32 $dstbe,$imm32 */
  {
    BPF_INSN_MUL32IBE, "mul32ibe", "mul32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mul32 $dstbe,$srcbe */
  {
    BPF_INSN_MUL32RBE, "mul32rbe", "mul32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* div $dstbe,$imm32 */
  {
    BPF_INSN_DIVIBE, "divibe", "div", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* div $dstbe,$srcbe */
  {
    BPF_INSN_DIVRBE, "divrbe", "div", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* div32 $dstbe,$imm32 */
  {
    BPF_INSN_DIV32IBE, "div32ibe", "div32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* div32 $dstbe,$srcbe */
  {
    BPF_INSN_DIV32RBE, "div32rbe", "div32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* or $dstbe,$imm32 */
  {
    BPF_INSN_ORIBE, "oribe", "or", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* or $dstbe,$srcbe */
  {
    BPF_INSN_ORRBE, "orrbe", "or", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* or32 $dstbe,$imm32 */
  {
    BPF_INSN_OR32IBE, "or32ibe", "or32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* or32 $dstbe,$srcbe */
  {
    BPF_INSN_OR32RBE, "or32rbe", "or32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* and $dstbe,$imm32 */
  {
    BPF_INSN_ANDIBE, "andibe", "and", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* and $dstbe,$srcbe */
  {
    BPF_INSN_ANDRBE, "andrbe", "and", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* and32 $dstbe,$imm32 */
  {
    BPF_INSN_AND32IBE, "and32ibe", "and32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* and32 $dstbe,$srcbe */
  {
    BPF_INSN_AND32RBE, "and32rbe", "and32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* lsh $dstbe,$imm32 */
  {
    BPF_INSN_LSHIBE, "lshibe", "lsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* lsh $dstbe,$srcbe */
  {
    BPF_INSN_LSHRBE, "lshrbe", "lsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* lsh32 $dstbe,$imm32 */
  {
    BPF_INSN_LSH32IBE, "lsh32ibe", "lsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* lsh32 $dstbe,$srcbe */
  {
    BPF_INSN_LSH32RBE, "lsh32rbe", "lsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* rsh $dstbe,$imm32 */
  {
    BPF_INSN_RSHIBE, "rshibe", "rsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* rsh $dstbe,$srcbe */
  {
    BPF_INSN_RSHRBE, "rshrbe", "rsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* rsh32 $dstbe,$imm32 */
  {
    BPF_INSN_RSH32IBE, "rsh32ibe", "rsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* rsh32 $dstbe,$srcbe */
  {
    BPF_INSN_RSH32RBE, "rsh32rbe", "rsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mod $dstbe,$imm32 */
  {
    BPF_INSN_MODIBE, "modibe", "mod", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mod $dstbe,$srcbe */
  {
    BPF_INSN_MODRBE, "modrbe", "mod", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mod32 $dstbe,$imm32 */
  {
    BPF_INSN_MOD32IBE, "mod32ibe", "mod32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mod32 $dstbe,$srcbe */
  {
    BPF_INSN_MOD32RBE, "mod32rbe", "mod32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* xor $dstbe,$imm32 */
  {
    BPF_INSN_XORIBE, "xoribe", "xor", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* xor $dstbe,$srcbe */
  {
    BPF_INSN_XORRBE, "xorrbe", "xor", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* xor32 $dstbe,$imm32 */
  {
    BPF_INSN_XOR32IBE, "xor32ibe", "xor32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* xor32 $dstbe,$srcbe */
  {
    BPF_INSN_XOR32RBE, "xor32rbe", "xor32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* arsh $dstbe,$imm32 */
  {
    BPF_INSN_ARSHIBE, "arshibe", "arsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* arsh $dstbe,$srcbe */
  {
    BPF_INSN_ARSHRBE, "arshrbe", "arsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* arsh32 $dstbe,$imm32 */
  {
    BPF_INSN_ARSH32IBE, "arsh32ibe", "arsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* arsh32 $dstbe,$srcbe */
  {
    BPF_INSN_ARSH32RBE, "arsh32rbe", "arsh32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* sdiv $dstbe,$imm32 */
  {
    BPF_INSN_SDIVIBE, "sdivibe", "sdiv", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* sdiv $dstbe,$srcbe */
  {
    BPF_INSN_SDIVRBE, "sdivrbe", "sdiv", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* sdiv32 $dstbe,$imm32 */
  {
    BPF_INSN_SDIV32IBE, "sdiv32ibe", "sdiv32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* sdiv32 $dstbe,$srcbe */
  {
    BPF_INSN_SDIV32RBE, "sdiv32rbe", "sdiv32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* smod $dstbe,$imm32 */
  {
    BPF_INSN_SMODIBE, "smodibe", "smod", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* smod $dstbe,$srcbe */
  {
    BPF_INSN_SMODRBE, "smodrbe", "smod", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* smod32 $dstbe,$imm32 */
  {
    BPF_INSN_SMOD32IBE, "smod32ibe", "smod32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* smod32 $dstbe,$srcbe */
  {
    BPF_INSN_SMOD32RBE, "smod32rbe", "smod32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* neg $dstbe */
  {
    BPF_INSN_NEGBE, "negbe", "neg", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* neg32 $dstbe */
  {
    BPF_INSN_NEG32BE, "neg32be", "neg32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mov $dstbe,$imm32 */
  {
    BPF_INSN_MOVIBE, "movibe", "mov", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mov $dstbe,$srcbe */
  {
    BPF_INSN_MOVRBE, "movrbe", "mov", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mov32 $dstbe,$imm32 */
  {
    BPF_INSN_MOV32IBE, "mov32ibe", "mov32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* mov32 $dstbe,$srcbe */
  {
    BPF_INSN_MOV32RBE, "mov32rbe", "mov32", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* endle $dstle,$endsize */
  {
    BPF_INSN_ENDLELE, "endlele", "endle", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* endbe $dstle,$endsize */
  {
    BPF_INSN_ENDBELE, "endbele", "endbe", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* endle $dstbe,$endsize */
  {
    BPF_INSN_ENDLEBE, "endlebe", "endle", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* endbe $dstbe,$endsize */
  {
    BPF_INSN_ENDBEBE, "endbebe", "endbe", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* lddw $dstle,$imm64 */
  {
    BPF_INSN_LDDWLE, "lddwle", "lddw", 128,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* lddw $dstbe,$imm64 */
  {
    BPF_INSN_LDDWBE, "lddwbe", "lddw", 128,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* ldabsw $imm32 */
  {
    BPF_INSN_LDABSW, "ldabsw", "ldabsw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }
  },
/* ldabsh $imm32 */
  {
    BPF_INSN_LDABSH, "ldabsh", "ldabsh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }
  },
/* ldabsb $imm32 */
  {
    BPF_INSN_LDABSB, "ldabsb", "ldabsb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }
  },
/* ldabsdw $imm32 */
  {
    BPF_INSN_LDABSDW, "ldabsdw", "ldabsdw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }
  },
/* ldindw $srcle,$imm32 */
  {
    BPF_INSN_LDINDWLE, "ldindwle", "ldindw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* ldindh $srcle,$imm32 */
  {
    BPF_INSN_LDINDHLE, "ldindhle", "ldindh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* ldindb $srcle,$imm32 */
  {
    BPF_INSN_LDINDBLE, "ldindble", "ldindb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* ldinddw $srcle,$imm32 */
  {
    BPF_INSN_LDINDDWLE, "ldinddwle", "ldinddw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* ldindw $srcbe,$imm32 */
  {
    BPF_INSN_LDINDWBE, "ldindwbe", "ldindw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* ldindh $srcbe,$imm32 */
  {
    BPF_INSN_LDINDHBE, "ldindhbe", "ldindh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* ldindb $srcbe,$imm32 */
  {
    BPF_INSN_LDINDBBE, "ldindbbe", "ldindb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* ldinddw $srcbe,$imm32 */
  {
    BPF_INSN_LDINDDWBE, "ldinddwbe", "ldinddw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* ldxw $dstle,[$srcle+$offset16] */
  {
    BPF_INSN_LDXWLE, "ldxwle", "ldxw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* ldxh $dstle,[$srcle+$offset16] */
  {
    BPF_INSN_LDXHLE, "ldxhle", "ldxh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* ldxb $dstle,[$srcle+$offset16] */
  {
    BPF_INSN_LDXBLE, "ldxble", "ldxb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* ldxdw $dstle,[$srcle+$offset16] */
  {
    BPF_INSN_LDXDWLE, "ldxdwle", "ldxdw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* stxw [$dstle+$offset16],$srcle */
  {
    BPF_INSN_STXWLE, "stxwle", "stxw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* stxh [$dstle+$offset16],$srcle */
  {
    BPF_INSN_STXHLE, "stxhle", "stxh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* stxb [$dstle+$offset16],$srcle */
  {
    BPF_INSN_STXBLE, "stxble", "stxb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* stxdw [$dstle+$offset16],$srcle */
  {
    BPF_INSN_STXDWLE, "stxdwle", "stxdw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* ldxw $dstbe,[$srcbe+$offset16] */
  {
    BPF_INSN_LDXWBE, "ldxwbe", "ldxw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* ldxh $dstbe,[$srcbe+$offset16] */
  {
    BPF_INSN_LDXHBE, "ldxhbe", "ldxh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* ldxb $dstbe,[$srcbe+$offset16] */
  {
    BPF_INSN_LDXBBE, "ldxbbe", "ldxb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* ldxdw $dstbe,[$srcbe+$offset16] */
  {
    BPF_INSN_LDXDWBE, "ldxdwbe", "ldxdw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* stxw [$dstbe+$offset16],$srcbe */
  {
    BPF_INSN_STXWBE, "stxwbe", "stxw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* stxh [$dstbe+$offset16],$srcbe */
  {
    BPF_INSN_STXHBE, "stxhbe", "stxh", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* stxb [$dstbe+$offset16],$srcbe */
  {
    BPF_INSN_STXBBE, "stxbbe", "stxb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* stxdw [$dstbe+$offset16],$srcbe */
  {
    BPF_INSN_STXDWBE, "stxdwbe", "stxdw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* stb [$dstle+$offset16],$imm32 */
  {
    BPF_INSN_STBLE, "stble", "stb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* sth [$dstle+$offset16],$imm32 */
  {
    BPF_INSN_STHLE, "sthle", "sth", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* stw [$dstle+$offset16],$imm32 */
  {
    BPF_INSN_STWLE, "stwle", "stw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* stdw [$dstle+$offset16],$imm32 */
  {
    BPF_INSN_STDWLE, "stdwle", "stdw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* stb [$dstbe+$offset16],$imm32 */
  {
    BPF_INSN_STBBE, "stbbe", "stb", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* sth [$dstbe+$offset16],$imm32 */
  {
    BPF_INSN_STHBE, "sthbe", "sth", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* stw [$dstbe+$offset16],$imm32 */
  {
    BPF_INSN_STWBE, "stwbe", "stw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* stdw [$dstbe+$offset16],$imm32 */
  {
    BPF_INSN_STDWBE, "stdwbe", "stdw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jeq $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JEQILE, "jeqile", "jeq", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jeq $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JEQRLE, "jeqrle", "jeq", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jeq32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JEQ32ILE, "jeq32ile", "jeq32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jeq32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JEQ32RLE, "jeq32rle", "jeq32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jgt $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JGTILE, "jgtile", "jgt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jgt $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JGTRLE, "jgtrle", "jgt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jgt32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JGT32ILE, "jgt32ile", "jgt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jgt32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JGT32RLE, "jgt32rle", "jgt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jge $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JGEILE, "jgeile", "jge", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jge $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JGERLE, "jgerle", "jge", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jge32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JGE32ILE, "jge32ile", "jge32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jge32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JGE32RLE, "jge32rle", "jge32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jlt $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JLTILE, "jltile", "jlt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jlt $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JLTRLE, "jltrle", "jlt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jlt32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JLT32ILE, "jlt32ile", "jlt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jlt32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JLT32RLE, "jlt32rle", "jlt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jle $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JLEILE, "jleile", "jle", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jle $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JLERLE, "jlerle", "jle", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jle32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JLE32ILE, "jle32ile", "jle32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jle32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JLE32RLE, "jle32rle", "jle32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jset $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSETILE, "jsetile", "jset", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jset $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSETRLE, "jsetrle", "jset", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jset32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSET32ILE, "jset32ile", "jset32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jset32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSET32RLE, "jset32rle", "jset32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jne $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JNEILE, "jneile", "jne", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jne $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JNERLE, "jnerle", "jne", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jne32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JNE32ILE, "jne32ile", "jne32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jne32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JNE32RLE, "jne32rle", "jne32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsgt $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSGTILE, "jsgtile", "jsgt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsgt $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSGTRLE, "jsgtrle", "jsgt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsgt32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSGT32ILE, "jsgt32ile", "jsgt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsgt32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSGT32RLE, "jsgt32rle", "jsgt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsge $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSGEILE, "jsgeile", "jsge", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsge $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSGERLE, "jsgerle", "jsge", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsge32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSGE32ILE, "jsge32ile", "jsge32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsge32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSGE32RLE, "jsge32rle", "jsge32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jslt $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSLTILE, "jsltile", "jslt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jslt $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSLTRLE, "jsltrle", "jslt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jslt32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSLT32ILE, "jslt32ile", "jslt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jslt32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSLT32RLE, "jslt32rle", "jslt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsle $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSLEILE, "jsleile", "jsle", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsle $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSLERLE, "jslerle", "jsle", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsle32 $dstle,$imm32,$disp16 */
  {
    BPF_INSN_JSLE32ILE, "jsle32ile", "jsle32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jsle32 $dstle,$srcle,$disp16 */
  {
    BPF_INSN_JSLE32RLE, "jsle32rle", "jsle32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* jeq $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JEQIBE, "jeqibe", "jeq", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jeq $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JEQRBE, "jeqrbe", "jeq", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jeq32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JEQ32IBE, "jeq32ibe", "jeq32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jeq32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JEQ32RBE, "jeq32rbe", "jeq32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jgt $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JGTIBE, "jgtibe", "jgt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jgt $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JGTRBE, "jgtrbe", "jgt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jgt32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JGT32IBE, "jgt32ibe", "jgt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jgt32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JGT32RBE, "jgt32rbe", "jgt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jge $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JGEIBE, "jgeibe", "jge", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jge $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JGERBE, "jgerbe", "jge", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jge32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JGE32IBE, "jge32ibe", "jge32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jge32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JGE32RBE, "jge32rbe", "jge32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jlt $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JLTIBE, "jltibe", "jlt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jlt $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JLTRBE, "jltrbe", "jlt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jlt32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JLT32IBE, "jlt32ibe", "jlt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jlt32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JLT32RBE, "jlt32rbe", "jlt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jle $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JLEIBE, "jleibe", "jle", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jle $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JLERBE, "jlerbe", "jle", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jle32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JLE32IBE, "jle32ibe", "jle32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jle32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JLE32RBE, "jle32rbe", "jle32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jset $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSETIBE, "jsetibe", "jset", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jset $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSETRBE, "jsetrbe", "jset", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jset32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSET32IBE, "jset32ibe", "jset32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jset32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSET32RBE, "jset32rbe", "jset32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jne $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JNEIBE, "jneibe", "jne", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jne $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JNERBE, "jnerbe", "jne", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jne32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JNE32IBE, "jne32ibe", "jne32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jne32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JNE32RBE, "jne32rbe", "jne32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsgt $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSGTIBE, "jsgtibe", "jsgt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsgt $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSGTRBE, "jsgtrbe", "jsgt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsgt32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSGT32IBE, "jsgt32ibe", "jsgt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsgt32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSGT32RBE, "jsgt32rbe", "jsgt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsge $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSGEIBE, "jsgeibe", "jsge", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsge $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSGERBE, "jsgerbe", "jsge", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsge32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSGE32IBE, "jsge32ibe", "jsge32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsge32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSGE32RBE, "jsge32rbe", "jsge32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jslt $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSLTIBE, "jsltibe", "jslt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jslt $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSLTRBE, "jsltrbe", "jslt", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jslt32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSLT32IBE, "jslt32ibe", "jslt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jslt32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSLT32RBE, "jslt32rbe", "jslt32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsle $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSLEIBE, "jsleibe", "jsle", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsle $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSLERBE, "jslerbe", "jsle", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsle32 $dstbe,$imm32,$disp16 */
  {
    BPF_INSN_JSLE32IBE, "jsle32ibe", "jsle32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* jsle32 $dstbe,$srcbe,$disp16 */
  {
    BPF_INSN_JSLE32RBE, "jsle32rbe", "jsle32", 64,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* call $disp32 */
  {
    BPF_INSN_CALLLE, "callle", "call", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* call $disp32 */
  {
    BPF_INSN_CALLBE, "callbe", "call", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* call $dstle */
  {
    BPF_INSN_CALLRLE, "callrle", "call", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } } } }
  },
/* call $dstbe */
  {
    BPF_INSN_CALLRBE, "callrbe", "call", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } } } }
  },
/* ja $disp16 */
  {
    BPF_INSN_JA, "ja", "ja", 64,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }
  },
/* exit */
  {
    BPF_INSN_EXIT, "exit", "exit", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }
  },
/* xadddw [$dstle+$offset16],$srcle */
  {
    BPF_INSN_XADDDWLE, "xadddwle", "xadddw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* xaddw [$dstle+$offset16],$srcle */
  {
    BPF_INSN_XADDWLE, "xaddwle", "xaddw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xa0" } } } }
  },
/* xadddw [$dstbe+$offset16],$srcbe */
  {
    BPF_INSN_XADDDWBE, "xadddwbe", "xadddw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* xaddw [$dstbe+$offset16],$srcbe */
  {
    BPF_INSN_XADDWBE, "xaddwbe", "xaddw", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x50" } } } }
  },
/* brkpt */
  {
    BPF_INSN_BRKPT, "brkpt", "brkpt", 64,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xf0" } } } }
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
static void bpf_cgen_rebuild_tables (CGEN_CPU_TABLE *);

/* Subroutine of bpf_cgen_cpu_open to look up a mach via its bfd name.  */

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

/* Subroutine of bpf_cgen_cpu_open to build the hardware table.  */

static void
build_hw_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_HW_ENTRY *init = & bpf_cgen_hw_table[0];
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

/* Subroutine of bpf_cgen_cpu_open to build the hardware table.  */

static void
build_ifield_table (CGEN_CPU_TABLE *cd)
{
  cd->ifld_table = & bpf_cgen_ifld_table[0];
}

/* Subroutine of bpf_cgen_cpu_open to build the hardware table.  */

static void
build_operand_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_OPERAND *init = & bpf_cgen_operand_table[0];
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

/* Subroutine of bpf_cgen_cpu_open to build the hardware table.
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
  const CGEN_IBASE *ib = & bpf_cgen_insn_table[0];
  CGEN_INSN *insns = xmalloc (MAX_INSNS * sizeof (CGEN_INSN));

  memset (insns, 0, MAX_INSNS * sizeof (CGEN_INSN));
  for (i = 0; i < MAX_INSNS; ++i)
    insns[i].base = &ib[i];
  cd->insn_table.init_entries = insns;
  cd->insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->insn_table.num_init_entries = MAX_INSNS;
}

/* Subroutine of bpf_cgen_cpu_open to rebuild the tables.  */

static void
bpf_cgen_rebuild_tables (CGEN_CPU_TABLE *cd)
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
	const CGEN_ISA *isa = & bpf_cgen_isa_table[i];

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
	const CGEN_MACH *mach = & bpf_cgen_mach_table[i];

	if (mach->insn_chunk_bitsize != 0)
	{
	  if (cd->insn_chunk_bitsize != 0 && cd->insn_chunk_bitsize != mach->insn_chunk_bitsize)
	    {
	      opcodes_error_handler
		(/* xgettext:c-format */
		 _("internal error: bpf_cgen_rebuild_tables: "
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
bpf_cgen_cpu_open (enum cgen_cpu_open_arg arg_type, ...)
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
	      lookup_mach_via_bfd_name (bpf_cgen_mach_table, name);

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
	     _("internal error: bpf_cgen_cpu_open: "
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
	 _("internal error: bpf_cgen_cpu_open: no endianness specified"));
      abort ();
    }

  cd->isas = cgen_bitset_copy (isas);
  cd->machs = machs;
  cd->endian = endian;
  cd->insn_endian
    = (insn_endian == CGEN_ENDIAN_UNKNOWN ? endian : insn_endian);

  /* Table (re)builder.  */
  cd->rebuild_tables = bpf_cgen_rebuild_tables;
  bpf_cgen_rebuild_tables (cd);

  /* Default to not allowing signed overflow.  */
  cd->signed_overflow_ok_p = 0;

  return (CGEN_CPU_DESC) cd;
}

/* Cover fn to bpf_cgen_cpu_open to handle the simple case of 1 isa, 1 mach.
   MACH_NAME is the bfd name of the mach.  */

CGEN_CPU_DESC
bpf_cgen_cpu_open_1 (const char *mach_name, enum cgen_endian endian)
{
  return bpf_cgen_cpu_open (CGEN_CPU_OPEN_BFDMACH, mach_name,
			       CGEN_CPU_OPEN_ENDIAN, endian,
			       CGEN_CPU_OPEN_END);
}

/* Close a cpu table.
   ??? This can live in a machine independent file, but there's currently
   no place to put this file (there's no libcgen).  libopcodes is the wrong
   place as some simulator ports use this but they don't use libopcodes.  */

void
bpf_cgen_cpu_close (CGEN_CPU_DESC cd)
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

