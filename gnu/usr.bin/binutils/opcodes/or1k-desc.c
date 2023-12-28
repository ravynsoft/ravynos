/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* CPU data for or1k.

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
#include "or1k-desc.h"
#include "or1k-opc.h"
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
  { "or32", MACH_OR32 },
  { "or32nd", MACH_OR32ND },
  { "max", MACH_MAX },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY ISA_attr[] ATTRIBUTE_UNUSED =
{
  { "openrisc", ISA_OPENRISC },
  { "max", ISA_MAX },
  { 0, 0 }
};

const CGEN_ATTR_TABLE or1k_cgen_ifield_attr_table[] =
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

const CGEN_ATTR_TABLE or1k_cgen_hardware_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "CACHE-ADDR", &bool_attr[0], &bool_attr[0] },
  { "PC", &bool_attr[0], &bool_attr[0] },
  { "PROFILE", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE or1k_cgen_operand_attr_table[] =
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

const CGEN_ATTR_TABLE or1k_cgen_insn_attr_table[] =
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
  { "DELAYED-CTI", &bool_attr[0], &bool_attr[0] },
  { "NOT-IN-DELAY-SLOT", &bool_attr[0], &bool_attr[0] },
  { "FORCED-CTI", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

/* Instruction set variants.  */

static const CGEN_ISA or1k_cgen_isa_table[] = {
  { "openrisc", 32, 32, 32, 32 },
  { 0, 0, 0, 0, 0 }
};

/* Machine variants.  */

static const CGEN_MACH or1k_cgen_mach_table[] = {
  { "or32", "or1k", MACH_OR32, 0 },
  { "or32nd", "or1knd", MACH_OR32ND, 0 },
  { 0, 0, 0, 0 }
};

static CGEN_KEYWORD_ENTRY or1k_cgen_opval_h_gpr_entries[] =
{
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
  { "r31", 31, {0, {{{0, 0}}}}, 0, 0 },
  { "lr", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "sp", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "fp", 2, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD or1k_cgen_opval_h_gpr =
{
  & or1k_cgen_opval_h_gpr_entries[0],
  35,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY or1k_cgen_opval_h_fsr_entries[] =
{
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
  { "r31", 31, {0, {{{0, 0}}}}, 0, 0 },
  { "lr", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "sp", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "fp", 2, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD or1k_cgen_opval_h_fsr =
{
  & or1k_cgen_opval_h_fsr_entries[0],
  35,
  0, 0, 0, 0, ""
};


/* The hardware table.  */

#define A(a) (1 << CGEN_HW_##a)

const CGEN_HW_ENTRY or1k_cgen_hw_table[] =
{
  { "h-memory", HW_H_MEMORY, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-sint", HW_H_SINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-uint", HW_H_UINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-addr", HW_H_ADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-iaddr", HW_H_IADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-pc", HW_H_PC, CGEN_ASM_NONE, 0, { 0|A(PC), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-spr", HW_H_SPR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-gpr", HW_H_GPR, CGEN_ASM_KEYWORD, & or1k_cgen_opval_h_gpr, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-fsr", HW_H_FSR, CGEN_ASM_KEYWORD, & or1k_cgen_opval_h_fsr, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-fd32r", HW_H_FD32R, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-i64r", HW_H_I64R, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-vr", HW_H_SYS_VR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr", HW_H_SYS_UPR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr", HW_H_SYS_CPUCFGR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-dmmucfgr", HW_H_SYS_DMMUCFGR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-immucfgr", HW_H_SYS_IMMUCFGR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-dccfgr", HW_H_SYS_DCCFGR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-iccfgr", HW_H_SYS_ICCFGR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-dcfgr", HW_H_SYS_DCFGR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-pccfgr", HW_H_SYS_PCCFGR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-npc", HW_H_SYS_NPC, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr", HW_H_SYS_SR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-ppc", HW_H_SYS_PPC, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr", HW_H_SYS_FPCSR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr0", HW_H_SYS_EPCR0, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr1", HW_H_SYS_EPCR1, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr2", HW_H_SYS_EPCR2, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr3", HW_H_SYS_EPCR3, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr4", HW_H_SYS_EPCR4, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr5", HW_H_SYS_EPCR5, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr6", HW_H_SYS_EPCR6, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr7", HW_H_SYS_EPCR7, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr8", HW_H_SYS_EPCR8, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr9", HW_H_SYS_EPCR9, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr10", HW_H_SYS_EPCR10, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr11", HW_H_SYS_EPCR11, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr12", HW_H_SYS_EPCR12, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr13", HW_H_SYS_EPCR13, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr14", HW_H_SYS_EPCR14, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-epcr15", HW_H_SYS_EPCR15, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear0", HW_H_SYS_EEAR0, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear1", HW_H_SYS_EEAR1, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear2", HW_H_SYS_EEAR2, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear3", HW_H_SYS_EEAR3, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear4", HW_H_SYS_EEAR4, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear5", HW_H_SYS_EEAR5, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear6", HW_H_SYS_EEAR6, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear7", HW_H_SYS_EEAR7, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear8", HW_H_SYS_EEAR8, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear9", HW_H_SYS_EEAR9, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear10", HW_H_SYS_EEAR10, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear11", HW_H_SYS_EEAR11, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear12", HW_H_SYS_EEAR12, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear13", HW_H_SYS_EEAR13, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear14", HW_H_SYS_EEAR14, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-eear15", HW_H_SYS_EEAR15, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr0", HW_H_SYS_ESR0, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr1", HW_H_SYS_ESR1, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr2", HW_H_SYS_ESR2, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr3", HW_H_SYS_ESR3, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr4", HW_H_SYS_ESR4, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr5", HW_H_SYS_ESR5, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr6", HW_H_SYS_ESR6, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr7", HW_H_SYS_ESR7, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr8", HW_H_SYS_ESR8, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr9", HW_H_SYS_ESR9, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr10", HW_H_SYS_ESR10, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr11", HW_H_SYS_ESR11, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr12", HW_H_SYS_ESR12, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr13", HW_H_SYS_ESR13, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr14", HW_H_SYS_ESR14, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-esr15", HW_H_SYS_ESR15, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr0", HW_H_SYS_GPR0, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr1", HW_H_SYS_GPR1, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr2", HW_H_SYS_GPR2, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr3", HW_H_SYS_GPR3, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr4", HW_H_SYS_GPR4, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr5", HW_H_SYS_GPR5, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr6", HW_H_SYS_GPR6, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr7", HW_H_SYS_GPR7, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr8", HW_H_SYS_GPR8, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr9", HW_H_SYS_GPR9, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr10", HW_H_SYS_GPR10, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr11", HW_H_SYS_GPR11, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr12", HW_H_SYS_GPR12, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr13", HW_H_SYS_GPR13, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr14", HW_H_SYS_GPR14, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr15", HW_H_SYS_GPR15, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr16", HW_H_SYS_GPR16, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr17", HW_H_SYS_GPR17, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr18", HW_H_SYS_GPR18, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr19", HW_H_SYS_GPR19, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr20", HW_H_SYS_GPR20, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr21", HW_H_SYS_GPR21, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr22", HW_H_SYS_GPR22, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr23", HW_H_SYS_GPR23, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr24", HW_H_SYS_GPR24, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr25", HW_H_SYS_GPR25, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr26", HW_H_SYS_GPR26, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr27", HW_H_SYS_GPR27, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr28", HW_H_SYS_GPR28, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr29", HW_H_SYS_GPR29, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr30", HW_H_SYS_GPR30, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr31", HW_H_SYS_GPR31, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr32", HW_H_SYS_GPR32, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr33", HW_H_SYS_GPR33, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr34", HW_H_SYS_GPR34, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr35", HW_H_SYS_GPR35, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr36", HW_H_SYS_GPR36, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr37", HW_H_SYS_GPR37, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr38", HW_H_SYS_GPR38, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr39", HW_H_SYS_GPR39, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr40", HW_H_SYS_GPR40, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr41", HW_H_SYS_GPR41, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr42", HW_H_SYS_GPR42, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr43", HW_H_SYS_GPR43, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr44", HW_H_SYS_GPR44, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr45", HW_H_SYS_GPR45, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr46", HW_H_SYS_GPR46, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr47", HW_H_SYS_GPR47, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr48", HW_H_SYS_GPR48, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr49", HW_H_SYS_GPR49, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr50", HW_H_SYS_GPR50, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr51", HW_H_SYS_GPR51, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr52", HW_H_SYS_GPR52, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr53", HW_H_SYS_GPR53, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr54", HW_H_SYS_GPR54, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr55", HW_H_SYS_GPR55, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr56", HW_H_SYS_GPR56, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr57", HW_H_SYS_GPR57, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr58", HW_H_SYS_GPR58, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr59", HW_H_SYS_GPR59, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr60", HW_H_SYS_GPR60, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr61", HW_H_SYS_GPR61, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr62", HW_H_SYS_GPR62, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr63", HW_H_SYS_GPR63, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr64", HW_H_SYS_GPR64, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr65", HW_H_SYS_GPR65, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr66", HW_H_SYS_GPR66, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr67", HW_H_SYS_GPR67, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr68", HW_H_SYS_GPR68, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr69", HW_H_SYS_GPR69, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr70", HW_H_SYS_GPR70, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr71", HW_H_SYS_GPR71, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr72", HW_H_SYS_GPR72, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr73", HW_H_SYS_GPR73, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr74", HW_H_SYS_GPR74, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr75", HW_H_SYS_GPR75, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr76", HW_H_SYS_GPR76, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr77", HW_H_SYS_GPR77, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr78", HW_H_SYS_GPR78, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr79", HW_H_SYS_GPR79, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr80", HW_H_SYS_GPR80, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr81", HW_H_SYS_GPR81, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr82", HW_H_SYS_GPR82, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr83", HW_H_SYS_GPR83, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr84", HW_H_SYS_GPR84, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr85", HW_H_SYS_GPR85, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr86", HW_H_SYS_GPR86, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr87", HW_H_SYS_GPR87, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr88", HW_H_SYS_GPR88, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr89", HW_H_SYS_GPR89, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr90", HW_H_SYS_GPR90, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr91", HW_H_SYS_GPR91, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr92", HW_H_SYS_GPR92, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr93", HW_H_SYS_GPR93, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr94", HW_H_SYS_GPR94, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr95", HW_H_SYS_GPR95, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr96", HW_H_SYS_GPR96, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr97", HW_H_SYS_GPR97, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr98", HW_H_SYS_GPR98, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr99", HW_H_SYS_GPR99, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr100", HW_H_SYS_GPR100, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr101", HW_H_SYS_GPR101, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr102", HW_H_SYS_GPR102, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr103", HW_H_SYS_GPR103, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr104", HW_H_SYS_GPR104, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr105", HW_H_SYS_GPR105, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr106", HW_H_SYS_GPR106, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr107", HW_H_SYS_GPR107, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr108", HW_H_SYS_GPR108, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr109", HW_H_SYS_GPR109, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr110", HW_H_SYS_GPR110, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr111", HW_H_SYS_GPR111, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr112", HW_H_SYS_GPR112, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr113", HW_H_SYS_GPR113, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr114", HW_H_SYS_GPR114, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr115", HW_H_SYS_GPR115, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr116", HW_H_SYS_GPR116, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr117", HW_H_SYS_GPR117, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr118", HW_H_SYS_GPR118, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr119", HW_H_SYS_GPR119, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr120", HW_H_SYS_GPR120, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr121", HW_H_SYS_GPR121, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr122", HW_H_SYS_GPR122, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr123", HW_H_SYS_GPR123, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr124", HW_H_SYS_GPR124, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr125", HW_H_SYS_GPR125, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr126", HW_H_SYS_GPR126, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr127", HW_H_SYS_GPR127, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr128", HW_H_SYS_GPR128, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr129", HW_H_SYS_GPR129, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr130", HW_H_SYS_GPR130, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr131", HW_H_SYS_GPR131, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr132", HW_H_SYS_GPR132, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr133", HW_H_SYS_GPR133, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr134", HW_H_SYS_GPR134, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr135", HW_H_SYS_GPR135, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr136", HW_H_SYS_GPR136, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr137", HW_H_SYS_GPR137, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr138", HW_H_SYS_GPR138, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr139", HW_H_SYS_GPR139, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr140", HW_H_SYS_GPR140, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr141", HW_H_SYS_GPR141, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr142", HW_H_SYS_GPR142, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr143", HW_H_SYS_GPR143, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr144", HW_H_SYS_GPR144, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr145", HW_H_SYS_GPR145, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr146", HW_H_SYS_GPR146, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr147", HW_H_SYS_GPR147, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr148", HW_H_SYS_GPR148, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr149", HW_H_SYS_GPR149, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr150", HW_H_SYS_GPR150, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr151", HW_H_SYS_GPR151, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr152", HW_H_SYS_GPR152, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr153", HW_H_SYS_GPR153, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr154", HW_H_SYS_GPR154, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr155", HW_H_SYS_GPR155, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr156", HW_H_SYS_GPR156, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr157", HW_H_SYS_GPR157, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr158", HW_H_SYS_GPR158, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr159", HW_H_SYS_GPR159, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr160", HW_H_SYS_GPR160, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr161", HW_H_SYS_GPR161, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr162", HW_H_SYS_GPR162, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr163", HW_H_SYS_GPR163, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr164", HW_H_SYS_GPR164, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr165", HW_H_SYS_GPR165, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr166", HW_H_SYS_GPR166, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr167", HW_H_SYS_GPR167, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr168", HW_H_SYS_GPR168, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr169", HW_H_SYS_GPR169, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr170", HW_H_SYS_GPR170, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr171", HW_H_SYS_GPR171, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr172", HW_H_SYS_GPR172, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr173", HW_H_SYS_GPR173, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr174", HW_H_SYS_GPR174, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr175", HW_H_SYS_GPR175, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr176", HW_H_SYS_GPR176, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr177", HW_H_SYS_GPR177, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr178", HW_H_SYS_GPR178, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr179", HW_H_SYS_GPR179, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr180", HW_H_SYS_GPR180, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr181", HW_H_SYS_GPR181, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr182", HW_H_SYS_GPR182, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr183", HW_H_SYS_GPR183, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr184", HW_H_SYS_GPR184, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr185", HW_H_SYS_GPR185, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr186", HW_H_SYS_GPR186, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr187", HW_H_SYS_GPR187, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr188", HW_H_SYS_GPR188, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr189", HW_H_SYS_GPR189, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr190", HW_H_SYS_GPR190, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr191", HW_H_SYS_GPR191, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr192", HW_H_SYS_GPR192, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr193", HW_H_SYS_GPR193, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr194", HW_H_SYS_GPR194, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr195", HW_H_SYS_GPR195, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr196", HW_H_SYS_GPR196, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr197", HW_H_SYS_GPR197, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr198", HW_H_SYS_GPR198, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr199", HW_H_SYS_GPR199, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr200", HW_H_SYS_GPR200, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr201", HW_H_SYS_GPR201, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr202", HW_H_SYS_GPR202, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr203", HW_H_SYS_GPR203, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr204", HW_H_SYS_GPR204, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr205", HW_H_SYS_GPR205, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr206", HW_H_SYS_GPR206, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr207", HW_H_SYS_GPR207, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr208", HW_H_SYS_GPR208, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr209", HW_H_SYS_GPR209, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr210", HW_H_SYS_GPR210, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr211", HW_H_SYS_GPR211, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr212", HW_H_SYS_GPR212, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr213", HW_H_SYS_GPR213, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr214", HW_H_SYS_GPR214, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr215", HW_H_SYS_GPR215, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr216", HW_H_SYS_GPR216, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr217", HW_H_SYS_GPR217, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr218", HW_H_SYS_GPR218, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr219", HW_H_SYS_GPR219, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr220", HW_H_SYS_GPR220, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr221", HW_H_SYS_GPR221, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr222", HW_H_SYS_GPR222, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr223", HW_H_SYS_GPR223, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr224", HW_H_SYS_GPR224, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr225", HW_H_SYS_GPR225, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr226", HW_H_SYS_GPR226, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr227", HW_H_SYS_GPR227, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr228", HW_H_SYS_GPR228, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr229", HW_H_SYS_GPR229, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr230", HW_H_SYS_GPR230, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr231", HW_H_SYS_GPR231, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr232", HW_H_SYS_GPR232, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr233", HW_H_SYS_GPR233, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr234", HW_H_SYS_GPR234, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr235", HW_H_SYS_GPR235, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr236", HW_H_SYS_GPR236, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr237", HW_H_SYS_GPR237, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr238", HW_H_SYS_GPR238, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr239", HW_H_SYS_GPR239, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr240", HW_H_SYS_GPR240, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr241", HW_H_SYS_GPR241, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr242", HW_H_SYS_GPR242, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr243", HW_H_SYS_GPR243, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr244", HW_H_SYS_GPR244, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr245", HW_H_SYS_GPR245, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr246", HW_H_SYS_GPR246, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr247", HW_H_SYS_GPR247, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr248", HW_H_SYS_GPR248, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr249", HW_H_SYS_GPR249, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr250", HW_H_SYS_GPR250, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr251", HW_H_SYS_GPR251, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr252", HW_H_SYS_GPR252, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr253", HW_H_SYS_GPR253, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr254", HW_H_SYS_GPR254, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr255", HW_H_SYS_GPR255, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr256", HW_H_SYS_GPR256, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr257", HW_H_SYS_GPR257, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr258", HW_H_SYS_GPR258, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr259", HW_H_SYS_GPR259, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr260", HW_H_SYS_GPR260, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr261", HW_H_SYS_GPR261, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr262", HW_H_SYS_GPR262, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr263", HW_H_SYS_GPR263, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr264", HW_H_SYS_GPR264, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr265", HW_H_SYS_GPR265, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr266", HW_H_SYS_GPR266, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr267", HW_H_SYS_GPR267, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr268", HW_H_SYS_GPR268, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr269", HW_H_SYS_GPR269, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr270", HW_H_SYS_GPR270, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr271", HW_H_SYS_GPR271, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr272", HW_H_SYS_GPR272, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr273", HW_H_SYS_GPR273, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr274", HW_H_SYS_GPR274, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr275", HW_H_SYS_GPR275, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr276", HW_H_SYS_GPR276, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr277", HW_H_SYS_GPR277, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr278", HW_H_SYS_GPR278, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr279", HW_H_SYS_GPR279, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr280", HW_H_SYS_GPR280, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr281", HW_H_SYS_GPR281, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr282", HW_H_SYS_GPR282, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr283", HW_H_SYS_GPR283, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr284", HW_H_SYS_GPR284, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr285", HW_H_SYS_GPR285, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr286", HW_H_SYS_GPR286, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr287", HW_H_SYS_GPR287, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr288", HW_H_SYS_GPR288, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr289", HW_H_SYS_GPR289, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr290", HW_H_SYS_GPR290, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr291", HW_H_SYS_GPR291, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr292", HW_H_SYS_GPR292, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr293", HW_H_SYS_GPR293, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr294", HW_H_SYS_GPR294, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr295", HW_H_SYS_GPR295, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr296", HW_H_SYS_GPR296, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr297", HW_H_SYS_GPR297, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr298", HW_H_SYS_GPR298, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr299", HW_H_SYS_GPR299, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr300", HW_H_SYS_GPR300, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr301", HW_H_SYS_GPR301, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr302", HW_H_SYS_GPR302, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr303", HW_H_SYS_GPR303, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr304", HW_H_SYS_GPR304, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr305", HW_H_SYS_GPR305, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr306", HW_H_SYS_GPR306, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr307", HW_H_SYS_GPR307, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr308", HW_H_SYS_GPR308, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr309", HW_H_SYS_GPR309, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr310", HW_H_SYS_GPR310, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr311", HW_H_SYS_GPR311, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr312", HW_H_SYS_GPR312, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr313", HW_H_SYS_GPR313, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr314", HW_H_SYS_GPR314, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr315", HW_H_SYS_GPR315, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr316", HW_H_SYS_GPR316, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr317", HW_H_SYS_GPR317, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr318", HW_H_SYS_GPR318, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr319", HW_H_SYS_GPR319, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr320", HW_H_SYS_GPR320, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr321", HW_H_SYS_GPR321, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr322", HW_H_SYS_GPR322, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr323", HW_H_SYS_GPR323, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr324", HW_H_SYS_GPR324, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr325", HW_H_SYS_GPR325, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr326", HW_H_SYS_GPR326, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr327", HW_H_SYS_GPR327, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr328", HW_H_SYS_GPR328, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr329", HW_H_SYS_GPR329, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr330", HW_H_SYS_GPR330, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr331", HW_H_SYS_GPR331, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr332", HW_H_SYS_GPR332, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr333", HW_H_SYS_GPR333, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr334", HW_H_SYS_GPR334, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr335", HW_H_SYS_GPR335, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr336", HW_H_SYS_GPR336, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr337", HW_H_SYS_GPR337, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr338", HW_H_SYS_GPR338, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr339", HW_H_SYS_GPR339, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr340", HW_H_SYS_GPR340, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr341", HW_H_SYS_GPR341, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr342", HW_H_SYS_GPR342, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr343", HW_H_SYS_GPR343, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr344", HW_H_SYS_GPR344, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr345", HW_H_SYS_GPR345, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr346", HW_H_SYS_GPR346, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr347", HW_H_SYS_GPR347, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr348", HW_H_SYS_GPR348, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr349", HW_H_SYS_GPR349, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr350", HW_H_SYS_GPR350, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr351", HW_H_SYS_GPR351, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr352", HW_H_SYS_GPR352, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr353", HW_H_SYS_GPR353, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr354", HW_H_SYS_GPR354, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr355", HW_H_SYS_GPR355, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr356", HW_H_SYS_GPR356, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr357", HW_H_SYS_GPR357, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr358", HW_H_SYS_GPR358, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr359", HW_H_SYS_GPR359, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr360", HW_H_SYS_GPR360, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr361", HW_H_SYS_GPR361, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr362", HW_H_SYS_GPR362, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr363", HW_H_SYS_GPR363, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr364", HW_H_SYS_GPR364, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr365", HW_H_SYS_GPR365, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr366", HW_H_SYS_GPR366, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr367", HW_H_SYS_GPR367, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr368", HW_H_SYS_GPR368, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr369", HW_H_SYS_GPR369, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr370", HW_H_SYS_GPR370, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr371", HW_H_SYS_GPR371, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr372", HW_H_SYS_GPR372, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr373", HW_H_SYS_GPR373, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr374", HW_H_SYS_GPR374, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr375", HW_H_SYS_GPR375, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr376", HW_H_SYS_GPR376, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr377", HW_H_SYS_GPR377, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr378", HW_H_SYS_GPR378, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr379", HW_H_SYS_GPR379, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr380", HW_H_SYS_GPR380, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr381", HW_H_SYS_GPR381, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr382", HW_H_SYS_GPR382, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr383", HW_H_SYS_GPR383, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr384", HW_H_SYS_GPR384, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr385", HW_H_SYS_GPR385, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr386", HW_H_SYS_GPR386, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr387", HW_H_SYS_GPR387, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr388", HW_H_SYS_GPR388, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr389", HW_H_SYS_GPR389, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr390", HW_H_SYS_GPR390, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr391", HW_H_SYS_GPR391, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr392", HW_H_SYS_GPR392, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr393", HW_H_SYS_GPR393, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr394", HW_H_SYS_GPR394, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr395", HW_H_SYS_GPR395, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr396", HW_H_SYS_GPR396, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr397", HW_H_SYS_GPR397, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr398", HW_H_SYS_GPR398, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr399", HW_H_SYS_GPR399, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr400", HW_H_SYS_GPR400, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr401", HW_H_SYS_GPR401, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr402", HW_H_SYS_GPR402, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr403", HW_H_SYS_GPR403, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr404", HW_H_SYS_GPR404, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr405", HW_H_SYS_GPR405, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr406", HW_H_SYS_GPR406, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr407", HW_H_SYS_GPR407, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr408", HW_H_SYS_GPR408, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr409", HW_H_SYS_GPR409, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr410", HW_H_SYS_GPR410, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr411", HW_H_SYS_GPR411, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr412", HW_H_SYS_GPR412, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr413", HW_H_SYS_GPR413, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr414", HW_H_SYS_GPR414, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr415", HW_H_SYS_GPR415, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr416", HW_H_SYS_GPR416, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr417", HW_H_SYS_GPR417, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr418", HW_H_SYS_GPR418, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr419", HW_H_SYS_GPR419, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr420", HW_H_SYS_GPR420, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr421", HW_H_SYS_GPR421, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr422", HW_H_SYS_GPR422, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr423", HW_H_SYS_GPR423, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr424", HW_H_SYS_GPR424, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr425", HW_H_SYS_GPR425, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr426", HW_H_SYS_GPR426, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr427", HW_H_SYS_GPR427, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr428", HW_H_SYS_GPR428, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr429", HW_H_SYS_GPR429, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr430", HW_H_SYS_GPR430, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr431", HW_H_SYS_GPR431, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr432", HW_H_SYS_GPR432, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr433", HW_H_SYS_GPR433, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr434", HW_H_SYS_GPR434, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr435", HW_H_SYS_GPR435, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr436", HW_H_SYS_GPR436, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr437", HW_H_SYS_GPR437, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr438", HW_H_SYS_GPR438, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr439", HW_H_SYS_GPR439, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr440", HW_H_SYS_GPR440, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr441", HW_H_SYS_GPR441, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr442", HW_H_SYS_GPR442, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr443", HW_H_SYS_GPR443, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr444", HW_H_SYS_GPR444, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr445", HW_H_SYS_GPR445, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr446", HW_H_SYS_GPR446, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr447", HW_H_SYS_GPR447, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr448", HW_H_SYS_GPR448, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr449", HW_H_SYS_GPR449, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr450", HW_H_SYS_GPR450, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr451", HW_H_SYS_GPR451, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr452", HW_H_SYS_GPR452, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr453", HW_H_SYS_GPR453, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr454", HW_H_SYS_GPR454, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr455", HW_H_SYS_GPR455, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr456", HW_H_SYS_GPR456, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr457", HW_H_SYS_GPR457, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr458", HW_H_SYS_GPR458, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr459", HW_H_SYS_GPR459, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr460", HW_H_SYS_GPR460, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr461", HW_H_SYS_GPR461, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr462", HW_H_SYS_GPR462, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr463", HW_H_SYS_GPR463, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr464", HW_H_SYS_GPR464, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr465", HW_H_SYS_GPR465, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr466", HW_H_SYS_GPR466, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr467", HW_H_SYS_GPR467, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr468", HW_H_SYS_GPR468, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr469", HW_H_SYS_GPR469, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr470", HW_H_SYS_GPR470, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr471", HW_H_SYS_GPR471, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr472", HW_H_SYS_GPR472, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr473", HW_H_SYS_GPR473, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr474", HW_H_SYS_GPR474, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr475", HW_H_SYS_GPR475, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr476", HW_H_SYS_GPR476, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr477", HW_H_SYS_GPR477, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr478", HW_H_SYS_GPR478, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr479", HW_H_SYS_GPR479, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr480", HW_H_SYS_GPR480, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr481", HW_H_SYS_GPR481, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr482", HW_H_SYS_GPR482, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr483", HW_H_SYS_GPR483, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr484", HW_H_SYS_GPR484, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr485", HW_H_SYS_GPR485, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr486", HW_H_SYS_GPR486, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr487", HW_H_SYS_GPR487, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr488", HW_H_SYS_GPR488, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr489", HW_H_SYS_GPR489, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr490", HW_H_SYS_GPR490, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr491", HW_H_SYS_GPR491, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr492", HW_H_SYS_GPR492, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr493", HW_H_SYS_GPR493, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr494", HW_H_SYS_GPR494, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr495", HW_H_SYS_GPR495, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr496", HW_H_SYS_GPR496, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr497", HW_H_SYS_GPR497, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr498", HW_H_SYS_GPR498, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr499", HW_H_SYS_GPR499, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr500", HW_H_SYS_GPR500, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr501", HW_H_SYS_GPR501, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr502", HW_H_SYS_GPR502, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr503", HW_H_SYS_GPR503, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr504", HW_H_SYS_GPR504, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr505", HW_H_SYS_GPR505, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr506", HW_H_SYS_GPR506, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr507", HW_H_SYS_GPR507, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr508", HW_H_SYS_GPR508, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr509", HW_H_SYS_GPR509, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr510", HW_H_SYS_GPR510, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-gpr511", HW_H_SYS_GPR511, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-mac-maclo", HW_H_MAC_MACLO, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-mac-machi", HW_H_MAC_MACHI, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-tick-ttmr", HW_H_TICK_TTMR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-vr-rev", HW_H_SYS_VR_REV, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-vr-cfg", HW_H_SYS_VR_CFG, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-vr-ver", HW_H_SYS_VR_VER, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-up", HW_H_SYS_UPR_UP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-dcp", HW_H_SYS_UPR_DCP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-icp", HW_H_SYS_UPR_ICP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-dmp", HW_H_SYS_UPR_DMP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-mp", HW_H_SYS_UPR_MP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-imp", HW_H_SYS_UPR_IMP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-dup", HW_H_SYS_UPR_DUP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-pcup", HW_H_SYS_UPR_PCUP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-picp", HW_H_SYS_UPR_PICP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-pmp", HW_H_SYS_UPR_PMP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-ttp", HW_H_SYS_UPR_TTP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-upr-cup", HW_H_SYS_UPR_CUP, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr-nsgr", HW_H_SYS_CPUCFGR_NSGR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr-cgf", HW_H_SYS_CPUCFGR_CGF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr-ob32s", HW_H_SYS_CPUCFGR_OB32S, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr-ob64s", HW_H_SYS_CPUCFGR_OB64S, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr-of32s", HW_H_SYS_CPUCFGR_OF32S, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr-of64s", HW_H_SYS_CPUCFGR_OF64S, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr-ov64s", HW_H_SYS_CPUCFGR_OV64S, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-cpucfgr-nd", HW_H_SYS_CPUCFGR_ND, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-sm", HW_H_SYS_SR_SM, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-tee", HW_H_SYS_SR_TEE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-iee", HW_H_SYS_SR_IEE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-dce", HW_H_SYS_SR_DCE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-ice", HW_H_SYS_SR_ICE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-dme", HW_H_SYS_SR_DME, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-ime", HW_H_SYS_SR_IME, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-lee", HW_H_SYS_SR_LEE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-ce", HW_H_SYS_SR_CE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-f", HW_H_SYS_SR_F, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-cy", HW_H_SYS_SR_CY, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-ov", HW_H_SYS_SR_OV, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-ove", HW_H_SYS_SR_OVE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-dsx", HW_H_SYS_SR_DSX, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-eph", HW_H_SYS_SR_EPH, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-fo", HW_H_SYS_SR_FO, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-sumra", HW_H_SYS_SR_SUMRA, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-sr-cid", HW_H_SYS_SR_CID, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-fpee", HW_H_SYS_FPCSR_FPEE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-rm", HW_H_SYS_FPCSR_RM, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-ovf", HW_H_SYS_FPCSR_OVF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-unf", HW_H_SYS_FPCSR_UNF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-snf", HW_H_SYS_FPCSR_SNF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-qnf", HW_H_SYS_FPCSR_QNF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-zf", HW_H_SYS_FPCSR_ZF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-ixf", HW_H_SYS_FPCSR_IXF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-ivf", HW_H_SYS_FPCSR_IVF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-inf", HW_H_SYS_FPCSR_INF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-sys-fpcsr-dzf", HW_H_SYS_FPCSR_DZF, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-simm16", HW_H_SIMM16, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } } },
  { "h-uimm16", HW_H_UIMM16, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-uimm6", HW_H_UIMM6, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-atomic-reserve", HW_H_ATOMIC_RESERVE, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-atomic-address", HW_H_ATOMIC_ADDRESS, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-roff1", HW_H_ROFF1, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { 0, 0, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A


/* The instruction field table.  */

#define A(a) (1 << CGEN_IFLD_##a)

const CGEN_IFLD or1k_cgen_ifld_table[] =
{
  { OR1K_F_NIL, "f-nil", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { OR1K_F_ANYOF, "f-anyof", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { OR1K_F_OPCODE, "f-opcode", 0, 32, 31, 6, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_R1, "f-r1", 0, 32, 25, 5, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_R2, "f-r2", 0, 32, 20, 5, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_R3, "f-r3", 0, 32, 15, 5, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_25_2, "f-op-25-2", 0, 32, 25, 2, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_25_5, "f-op-25-5", 0, 32, 25, 5, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_16_1, "f-op-16-1", 0, 32, 16, 1, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_7_4, "f-op-7-4", 0, 32, 7, 4, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_3_4, "f-op-3-4", 0, 32, 3, 4, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_9_2, "f-op-9-2", 0, 32, 9, 2, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_9_4, "f-op-9-4", 0, 32, 9, 4, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_7_8, "f-op-7-8", 0, 32, 7, 8, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_OP_7_2, "f-op-7-2", 0, 32, 7, 2, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_25_26, "f-resv-25-26", 0, 32, 25, 26, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_25_10, "f-resv-25-10", 0, 32, 25, 10, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_25_5, "f-resv-25-5", 0, 32, 25, 5, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_23_8, "f-resv-23-8", 0, 32, 23, 8, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_20_21, "f-resv-20-21", 0, 32, 20, 21, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_20_5, "f-resv-20-5", 0, 32, 20, 5, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_20_4, "f-resv-20-4", 0, 32, 20, 4, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_15_8, "f-resv-15-8", 0, 32, 15, 8, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_15_6, "f-resv-15-6", 0, 32, 15, 6, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_10_11, "f-resv-10-11", 0, 32, 10, 11, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_10_7, "f-resv-10-7", 0, 32, 10, 7, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_10_3, "f-resv-10-3", 0, 32, 10, 3, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_10_1, "f-resv-10-1", 0, 32, 10, 1, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_8_1, "f-resv-8-1", 0, 32, 8, 1, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_7_4, "f-resv-7-4", 0, 32, 7, 4, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RESV_5_2, "f-resv-5-2", 0, 32, 5, 2, { 0|A(RESERVED), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_IMM16_25_5, "f-imm16-25-5", 0, 32, 25, 5, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_IMM16_10_11, "f-imm16-10-11", 0, 32, 10, 11, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_DISP26, "f-disp26", 0, 32, 25, 26, { 0|A(PCREL_ADDR), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_DISP21, "f-disp21", 0, 32, 20, 21, { 0|A(ABS_ADDR), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_UIMM16, "f-uimm16", 0, 32, 15, 16, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_SIMM16, "f-simm16", 0, 32, 15, 16, { 0|A(SIGN_OPT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_UIMM6, "f-uimm6", 0, 32, 5, 6, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_UIMM16_SPLIT, "f-uimm16-split", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_SIMM16_SPLIT, "f-simm16-split", 0, 0, 0, 0,{ 0|A(SIGN_OPT)|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RDOFF_10_1, "f-rdoff-10-1", 0, 32, 10, 1, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RAOFF_9_1, "f-raoff-9-1", 0, 32, 9, 1, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RBOFF_8_1, "f-rboff-8-1", 0, 32, 8, 1, { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RDD32, "f-rdd32", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RAD32, "f-rad32", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { OR1K_F_RBD32, "f-rbd32", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
  { 0, 0, 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A



/* multi ifield declarations */

const CGEN_MAYBE_MULTI_IFLD OR1K_F_UIMM16_SPLIT_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD OR1K_F_SIMM16_SPLIT_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD OR1K_F_RDD32_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD OR1K_F_RAD32_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD OR1K_F_RBD32_MULTI_IFIELD [];


/* multi ifield definitions */

const CGEN_MAYBE_MULTI_IFLD OR1K_F_UIMM16_SPLIT_MULTI_IFIELD [] =
{
    { 0, { &or1k_cgen_ifld_table[OR1K_F_IMM16_25_5] } },
    { 0, { &or1k_cgen_ifld_table[OR1K_F_IMM16_10_11] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD OR1K_F_SIMM16_SPLIT_MULTI_IFIELD [] =
{
    { 0, { &or1k_cgen_ifld_table[OR1K_F_IMM16_25_5] } },
    { 0, { &or1k_cgen_ifld_table[OR1K_F_IMM16_10_11] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD OR1K_F_RDD32_MULTI_IFIELD [] =
{
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R1] } },
    { 0, { &or1k_cgen_ifld_table[OR1K_F_RDOFF_10_1] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD OR1K_F_RAD32_MULTI_IFIELD [] =
{
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R2] } },
    { 0, { &or1k_cgen_ifld_table[OR1K_F_RAOFF_9_1] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD OR1K_F_RBD32_MULTI_IFIELD [] =
{
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R3] } },
    { 0, { &or1k_cgen_ifld_table[OR1K_F_RBOFF_8_1] } },
    { 0, { 0 } }
};

/* The operand table.  */

#define A(a) (1 << CGEN_OPERAND_##a)
#define OPERAND(op) OR1K_OPERAND_##op

const CGEN_OPERAND or1k_cgen_operand_table[] =
{
/* pc: program counter */
  { "pc", OR1K_OPERAND_PC, HW_H_PC, 0, 0,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_NIL] } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* sys-sr: supervision register */
  { "sys-sr", OR1K_OPERAND_SYS_SR, HW_H_SYS_SR, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-esr0: exception supervision register 0 */
  { "sys-esr0", OR1K_OPERAND_SYS_ESR0, HW_H_SYS_ESR0, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-epcr0: exception PC register 0 */
  { "sys-epcr0", OR1K_OPERAND_SYS_EPCR0, HW_H_SYS_EPCR0, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-sr-lee: SR little endian enable bit */
  { "sys-sr-lee", OR1K_OPERAND_SYS_SR_LEE, HW_H_SYS_SR_LEE, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-sr-f: SR flag bit */
  { "sys-sr-f", OR1K_OPERAND_SYS_SR_F, HW_H_SYS_SR_F, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-sr-cy: SR carry bit */
  { "sys-sr-cy", OR1K_OPERAND_SYS_SR_CY, HW_H_SYS_SR_CY, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-sr-ov: SR overflow bit */
  { "sys-sr-ov", OR1K_OPERAND_SYS_SR_OV, HW_H_SYS_SR_OV, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-sr-ove: SR overflow exception enable bit */
  { "sys-sr-ove", OR1K_OPERAND_SYS_SR_OVE, HW_H_SYS_SR_OVE, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-cpucfgr-ob64s: CPUCFGR ORBIS64 supported bit */
  { "sys-cpucfgr-ob64s", OR1K_OPERAND_SYS_CPUCFGR_OB64S, HW_H_SYS_CPUCFGR_OB64S, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-cpucfgr-nd: CPUCFGR no delay bit */
  { "sys-cpucfgr-nd", OR1K_OPERAND_SYS_CPUCFGR_ND, HW_H_SYS_CPUCFGR_ND, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sys-fpcsr-rm: floating point round mode */
  { "sys-fpcsr-rm", OR1K_OPERAND_SYS_FPCSR_RM, HW_H_SYS_FPCSR_RM, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* mac-machi: MAC HI result register */
  { "mac-machi", OR1K_OPERAND_MAC_MACHI, HW_H_MAC_MACHI, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* mac-maclo: MAC LO result register */
  { "mac-maclo", OR1K_OPERAND_MAC_MACLO, HW_H_MAC_MACLO, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* atomic-reserve: atomic reserve flag */
  { "atomic-reserve", OR1K_OPERAND_ATOMIC_RESERVE, HW_H_ATOMIC_RESERVE, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* atomic-address: atomic address */
  { "atomic-address", OR1K_OPERAND_ATOMIC_ADDRESS, HW_H_ATOMIC_ADDRESS, 0, 0,
    { 0, { 0 } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* uimm6: uimm6 */
  { "uimm6", OR1K_OPERAND_UIMM6, HW_H_UIMM6, 5, 6,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_UIMM6] } },
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rD: destination register */
  { "rD", OR1K_OPERAND_RD, HW_H_GPR, 25, 5,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R1] } },
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rA: source register A */
  { "rA", OR1K_OPERAND_RA, HW_H_GPR, 20, 5,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R2] } },
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rB: source register B */
  { "rB", OR1K_OPERAND_RB, HW_H_GPR, 15, 5,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R3] } },
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* disp26: pc-rel 26 bit */
  { "disp26", OR1K_OPERAND_DISP26, HW_H_IADDR, 25, 26,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_DISP26] } },
    { 0|A(PCREL_ADDR), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* disp21: pc-rel 21 bit */
  { "disp21", OR1K_OPERAND_DISP21, HW_H_IADDR, 20, 21,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_DISP21] } },
    { 0|A(ABS_ADDR), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* simm16: 16-bit signed immediate */
  { "simm16", OR1K_OPERAND_SIMM16, HW_H_SIMM16, 15, 16,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_SIMM16] } },
    { 0|A(SIGN_OPT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* uimm16: 16-bit unsigned immediate */
  { "uimm16", OR1K_OPERAND_UIMM16, HW_H_UIMM16, 15, 16,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_UIMM16] } },
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* simm16-split: split 16-bit signed immediate */
  { "simm16-split", OR1K_OPERAND_SIMM16_SPLIT, HW_H_SIMM16, 10, 16,
    { 2, { &OR1K_F_SIMM16_SPLIT_MULTI_IFIELD[0] } },
    { 0|A(SIGN_OPT)|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* uimm16-split: split 16-bit unsigned immediate */
  { "uimm16-split", OR1K_OPERAND_UIMM16_SPLIT, HW_H_UIMM16, 10, 16,
    { 2, { &OR1K_F_UIMM16_SPLIT_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rDSF: destination register (single floating point mode) */
  { "rDSF", OR1K_OPERAND_RDSF, HW_H_FSR, 25, 5,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R1] } },
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rASF: source register A (single floating point mode) */
  { "rASF", OR1K_OPERAND_RASF, HW_H_FSR, 20, 5,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R2] } },
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rBSF: source register B (single floating point mode) */
  { "rBSF", OR1K_OPERAND_RBSF, HW_H_FSR, 15, 5,
    { 0, { &or1k_cgen_ifld_table[OR1K_F_R3] } },
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rDD32F: destination register (double floating point pair) */
  { "rDD32F", OR1K_OPERAND_RDD32F, HW_H_FD32R, 10, 6,
    { 2, { &OR1K_F_RDD32_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rDDI: destination register (double integer pair) */
  { "rDDI", OR1K_OPERAND_RDDI, HW_H_I64R, 10, 6,
    { 2, { &OR1K_F_RDD32_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rAD32F: source register A (double floating point pair) */
  { "rAD32F", OR1K_OPERAND_RAD32F, HW_H_FD32R, 9, 6,
    { 2, { &OR1K_F_RAD32_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rADI: source register A (double integer pair) */
  { "rADI", OR1K_OPERAND_RADI, HW_H_I64R, 9, 6,
    { 2, { &OR1K_F_RAD32_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rBD32F: source register B (double floating point pair) */
  { "rBD32F", OR1K_OPERAND_RBD32F, HW_H_FD32R, 8, 6,
    { 2, { &OR1K_F_RBD32_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* rBDI: source register B (double integer pair) */
  { "rBDI", OR1K_OPERAND_RBDI, HW_H_I64R, 8, 6,
    { 2, { &OR1K_F_RBD32_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }  },
/* sentinel */
  { 0, 0, 0, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A


/* The instruction table.  */

#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))
#define A(a) (1 << CGEN_INSN_##a)

static const CGEN_IBASE or1k_cgen_insn_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
/* l.j ${disp26} */
  {
    OR1K_INSN_L_J, "l-j", "l.j", 32,
    { 0|A(UNCOND_CTI)|A(NOT_IN_DELAY_SLOT)|A(DELAYED_CTI)|A(SKIP_CTI)|A(DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.adrp $rD,${disp21} */
  {
    OR1K_INSN_L_ADRP, "l-adrp", "l.adrp", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.jal ${disp26} */
  {
    OR1K_INSN_L_JAL, "l-jal", "l.jal", 32,
    { 0|A(UNCOND_CTI)|A(NOT_IN_DELAY_SLOT)|A(DELAYED_CTI)|A(SKIP_CTI)|A(DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.jr $rB */
  {
    OR1K_INSN_L_JR, "l-jr", "l.jr", 32,
    { 0|A(UNCOND_CTI)|A(NOT_IN_DELAY_SLOT)|A(DELAYED_CTI)|A(SKIP_CTI)|A(DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.jalr $rB */
  {
    OR1K_INSN_L_JALR, "l-jalr", "l.jalr", 32,
    { 0|A(UNCOND_CTI)|A(NOT_IN_DELAY_SLOT)|A(DELAYED_CTI)|A(SKIP_CTI)|A(DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.bnf ${disp26} */
  {
    OR1K_INSN_L_BNF, "l-bnf", "l.bnf", 32,
    { 0|A(COND_CTI)|A(NOT_IN_DELAY_SLOT)|A(DELAYED_CTI)|A(SKIP_CTI)|A(DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.bf ${disp26} */
  {
    OR1K_INSN_L_BF, "l-bf", "l.bf", 32,
    { 0|A(COND_CTI)|A(NOT_IN_DELAY_SLOT)|A(DELAYED_CTI)|A(SKIP_CTI)|A(DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.trap ${uimm16} */
  {
    OR1K_INSN_L_TRAP, "l-trap", "l.trap", 32,
    { 0|A(NOT_IN_DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sys ${uimm16} */
  {
    OR1K_INSN_L_SYS, "l-sys", "l.sys", 32,
    { 0|A(NOT_IN_DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.msync */
  {
    OR1K_INSN_L_MSYNC, "l-msync", "l.msync", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.psync */
  {
    OR1K_INSN_L_PSYNC, "l-psync", "l.psync", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.csync */
  {
    OR1K_INSN_L_CSYNC, "l-csync", "l.csync", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.rfe */
  {
    OR1K_INSN_L_RFE, "l-rfe", "l.rfe", 32,
    { 0|A(FORCED_CTI)|A(NOT_IN_DELAY_SLOT), { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.nop ${uimm16} */
  {
    OR1K_INSN_L_NOP_IMM, "l-nop-imm", "l.nop", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.nop */
  {
    OR1K_INSN_L_NOP, "l-nop", "l.nop", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.movhi $rD,$uimm16 */
  {
    OR1K_INSN_L_MOVHI, "l-movhi", "l.movhi", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.macrc $rD */
  {
    OR1K_INSN_L_MACRC, "l-macrc", "l.macrc", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.mfspr $rD,$rA,${uimm16} */
  {
    OR1K_INSN_L_MFSPR, "l-mfspr", "l.mfspr", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.mtspr $rA,$rB,${uimm16-split} */
  {
    OR1K_INSN_L_MTSPR, "l-mtspr", "l.mtspr", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.lwz $rD,${simm16}($rA) */
  {
    OR1K_INSN_L_LWZ, "l-lwz", "l.lwz", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.lws $rD,${simm16}($rA) */
  {
    OR1K_INSN_L_LWS, "l-lws", "l.lws", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.lwa $rD,${simm16}($rA) */
  {
    OR1K_INSN_L_LWA, "l-lwa", "l.lwa", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.lbz $rD,${simm16}($rA) */
  {
    OR1K_INSN_L_LBZ, "l-lbz", "l.lbz", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.lbs $rD,${simm16}($rA) */
  {
    OR1K_INSN_L_LBS, "l-lbs", "l.lbs", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.lhz $rD,${simm16}($rA) */
  {
    OR1K_INSN_L_LHZ, "l-lhz", "l.lhz", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.lhs $rD,${simm16}($rA) */
  {
    OR1K_INSN_L_LHS, "l-lhs", "l.lhs", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sw ${simm16-split}($rA),$rB */
  {
    OR1K_INSN_L_SW, "l-sw", "l.sw", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sb ${simm16-split}($rA),$rB */
  {
    OR1K_INSN_L_SB, "l-sb", "l.sb", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sh ${simm16-split}($rA),$rB */
  {
    OR1K_INSN_L_SH, "l-sh", "l.sh", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.swa ${simm16-split}($rA),$rB */
  {
    OR1K_INSN_L_SWA, "l-swa", "l.swa", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sll $rD,$rA,$rB */
  {
    OR1K_INSN_L_SLL, "l-sll", "l.sll", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.slli $rD,$rA,${uimm6} */
  {
    OR1K_INSN_L_SLLI, "l-slli", "l.slli", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.srl $rD,$rA,$rB */
  {
    OR1K_INSN_L_SRL, "l-srl", "l.srl", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.srli $rD,$rA,${uimm6} */
  {
    OR1K_INSN_L_SRLI, "l-srli", "l.srli", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sra $rD,$rA,$rB */
  {
    OR1K_INSN_L_SRA, "l-sra", "l.sra", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.srai $rD,$rA,${uimm6} */
  {
    OR1K_INSN_L_SRAI, "l-srai", "l.srai", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.ror $rD,$rA,$rB */
  {
    OR1K_INSN_L_ROR, "l-ror", "l.ror", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.rori $rD,$rA,${uimm6} */
  {
    OR1K_INSN_L_RORI, "l-rori", "l.rori", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.and $rD,$rA,$rB */
  {
    OR1K_INSN_L_AND, "l-and", "l.and", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.or $rD,$rA,$rB */
  {
    OR1K_INSN_L_OR, "l-or", "l.or", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.xor $rD,$rA,$rB */
  {
    OR1K_INSN_L_XOR, "l-xor", "l.xor", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.add $rD,$rA,$rB */
  {
    OR1K_INSN_L_ADD, "l-add", "l.add", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sub $rD,$rA,$rB */
  {
    OR1K_INSN_L_SUB, "l-sub", "l.sub", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.addc $rD,$rA,$rB */
  {
    OR1K_INSN_L_ADDC, "l-addc", "l.addc", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.mul $rD,$rA,$rB */
  {
    OR1K_INSN_L_MUL, "l-mul", "l.mul", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.muld $rA,$rB */
  {
    OR1K_INSN_L_MULD, "l-muld", "l.muld", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.mulu $rD,$rA,$rB */
  {
    OR1K_INSN_L_MULU, "l-mulu", "l.mulu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.muldu $rA,$rB */
  {
    OR1K_INSN_L_MULDU, "l-muldu", "l.muldu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.div $rD,$rA,$rB */
  {
    OR1K_INSN_L_DIV, "l-div", "l.div", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.divu $rD,$rA,$rB */
  {
    OR1K_INSN_L_DIVU, "l-divu", "l.divu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.ff1 $rD,$rA */
  {
    OR1K_INSN_L_FF1, "l-ff1", "l.ff1", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.fl1 $rD,$rA */
  {
    OR1K_INSN_L_FL1, "l-fl1", "l.fl1", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.andi $rD,$rA,$uimm16 */
  {
    OR1K_INSN_L_ANDI, "l-andi", "l.andi", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.ori $rD,$rA,$uimm16 */
  {
    OR1K_INSN_L_ORI, "l-ori", "l.ori", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.xori $rD,$rA,$simm16 */
  {
    OR1K_INSN_L_XORI, "l-xori", "l.xori", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.addi $rD,$rA,$simm16 */
  {
    OR1K_INSN_L_ADDI, "l-addi", "l.addi", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.addic $rD,$rA,$simm16 */
  {
    OR1K_INSN_L_ADDIC, "l-addic", "l.addic", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.muli $rD,$rA,$simm16 */
  {
    OR1K_INSN_L_MULI, "l-muli", "l.muli", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.exths $rD,$rA */
  {
    OR1K_INSN_L_EXTHS, "l-exths", "l.exths", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.extbs $rD,$rA */
  {
    OR1K_INSN_L_EXTBS, "l-extbs", "l.extbs", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.exthz $rD,$rA */
  {
    OR1K_INSN_L_EXTHZ, "l-exthz", "l.exthz", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.extbz $rD,$rA */
  {
    OR1K_INSN_L_EXTBZ, "l-extbz", "l.extbz", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.extws $rD,$rA */
  {
    OR1K_INSN_L_EXTWS, "l-extws", "l.extws", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.extwz $rD,$rA */
  {
    OR1K_INSN_L_EXTWZ, "l-extwz", "l.extwz", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cmov $rD,$rA,$rB */
  {
    OR1K_INSN_L_CMOV, "l-cmov", "l.cmov", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfgts $rA,$rB */
  {
    OR1K_INSN_L_SFGTS, "l-sfgts", "l.sfgts", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfgtsi $rA,$simm16 */
  {
    OR1K_INSN_L_SFGTSI, "l-sfgtsi", "l.sfgtsi", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfgtu $rA,$rB */
  {
    OR1K_INSN_L_SFGTU, "l-sfgtu", "l.sfgtu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfgtui $rA,$simm16 */
  {
    OR1K_INSN_L_SFGTUI, "l-sfgtui", "l.sfgtui", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfges $rA,$rB */
  {
    OR1K_INSN_L_SFGES, "l-sfges", "l.sfges", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfgesi $rA,$simm16 */
  {
    OR1K_INSN_L_SFGESI, "l-sfgesi", "l.sfgesi", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfgeu $rA,$rB */
  {
    OR1K_INSN_L_SFGEU, "l-sfgeu", "l.sfgeu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfgeui $rA,$simm16 */
  {
    OR1K_INSN_L_SFGEUI, "l-sfgeui", "l.sfgeui", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sflts $rA,$rB */
  {
    OR1K_INSN_L_SFLTS, "l-sflts", "l.sflts", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfltsi $rA,$simm16 */
  {
    OR1K_INSN_L_SFLTSI, "l-sfltsi", "l.sfltsi", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfltu $rA,$rB */
  {
    OR1K_INSN_L_SFLTU, "l-sfltu", "l.sfltu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfltui $rA,$simm16 */
  {
    OR1K_INSN_L_SFLTUI, "l-sfltui", "l.sfltui", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfles $rA,$rB */
  {
    OR1K_INSN_L_SFLES, "l-sfles", "l.sfles", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sflesi $rA,$simm16 */
  {
    OR1K_INSN_L_SFLESI, "l-sflesi", "l.sflesi", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfleu $rA,$rB */
  {
    OR1K_INSN_L_SFLEU, "l-sfleu", "l.sfleu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfleui $rA,$simm16 */
  {
    OR1K_INSN_L_SFLEUI, "l-sfleui", "l.sfleui", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfeq $rA,$rB */
  {
    OR1K_INSN_L_SFEQ, "l-sfeq", "l.sfeq", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfeqi $rA,$simm16 */
  {
    OR1K_INSN_L_SFEQI, "l-sfeqi", "l.sfeqi", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfne $rA,$rB */
  {
    OR1K_INSN_L_SFNE, "l-sfne", "l.sfne", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.sfnei $rA,$simm16 */
  {
    OR1K_INSN_L_SFNEI, "l-sfnei", "l.sfnei", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.mac $rA,$rB */
  {
    OR1K_INSN_L_MAC, "l-mac", "l.mac", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.maci $rA,${simm16} */
  {
    OR1K_INSN_L_MACI, "l-maci", "l.maci", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.macu $rA,$rB */
  {
    OR1K_INSN_L_MACU, "l-macu", "l.macu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.msb $rA,$rB */
  {
    OR1K_INSN_L_MSB, "l-msb", "l.msb", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.msbu $rA,$rB */
  {
    OR1K_INSN_L_MSBU, "l-msbu", "l.msbu", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cust1 */
  {
    OR1K_INSN_L_CUST1, "l-cust1", "l.cust1", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cust2 */
  {
    OR1K_INSN_L_CUST2, "l-cust2", "l.cust2", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cust3 */
  {
    OR1K_INSN_L_CUST3, "l-cust3", "l.cust3", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cust4 */
  {
    OR1K_INSN_L_CUST4, "l-cust4", "l.cust4", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cust5 */
  {
    OR1K_INSN_L_CUST5, "l-cust5", "l.cust5", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cust6 */
  {
    OR1K_INSN_L_CUST6, "l-cust6", "l.cust6", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cust7 */
  {
    OR1K_INSN_L_CUST7, "l-cust7", "l.cust7", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* l.cust8 */
  {
    OR1K_INSN_L_CUST8, "l-cust8", "l.cust8", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.add.s $rDSF,$rASF,$rBSF */
  {
    OR1K_INSN_LF_ADD_S, "lf-add-s", "lf.add.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.add.d $rDD32F,$rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_ADD_D32, "lf-add-d32", "lf.add.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sub.s $rDSF,$rASF,$rBSF */
  {
    OR1K_INSN_LF_SUB_S, "lf-sub-s", "lf.sub.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sub.d $rDD32F,$rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SUB_D32, "lf-sub-d32", "lf.sub.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.mul.s $rDSF,$rASF,$rBSF */
  {
    OR1K_INSN_LF_MUL_S, "lf-mul-s", "lf.mul.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.mul.d $rDD32F,$rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_MUL_D32, "lf-mul-d32", "lf.mul.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.div.s $rDSF,$rASF,$rBSF */
  {
    OR1K_INSN_LF_DIV_S, "lf-div-s", "lf.div.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.div.d $rDD32F,$rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_DIV_D32, "lf-div-d32", "lf.div.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.rem.s $rDSF,$rASF,$rBSF */
  {
    OR1K_INSN_LF_REM_S, "lf-rem-s", "lf.rem.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.rem.d $rDD32F,$rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_REM_D32, "lf-rem-d32", "lf.rem.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.itof.s $rDSF,$rA */
  {
    OR1K_INSN_LF_ITOF_S, "lf-itof-s", "lf.itof.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.itof.d $rDD32F,$rADI */
  {
    OR1K_INSN_LF_ITOF_D32, "lf-itof-d32", "lf.itof.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.ftoi.s $rD,$rASF */
  {
    OR1K_INSN_LF_FTOI_S, "lf-ftoi-s", "lf.ftoi.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.ftoi.d $rDDI,$rAD32F */
  {
    OR1K_INSN_LF_FTOI_D32, "lf-ftoi-d32", "lf.ftoi.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfeq.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFEQ_S, "lf-sfeq-s", "lf.sfeq.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfeq.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFEQ_D32, "lf-sfeq-d32", "lf.sfeq.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfne.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFNE_S, "lf-sfne-s", "lf.sfne.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfne.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFNE_D32, "lf-sfne-d32", "lf.sfne.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfge.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFGE_S, "lf-sfge-s", "lf.sfge.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfge.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFGE_D32, "lf-sfge-d32", "lf.sfge.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfgt.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFGT_S, "lf-sfgt-s", "lf.sfgt.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfgt.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFGT_D32, "lf-sfgt-d32", "lf.sfgt.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sflt.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFLT_S, "lf-sflt-s", "lf.sflt.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sflt.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFLT_D32, "lf-sflt-d32", "lf.sflt.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfle.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFLE_S, "lf-sfle-s", "lf.sfle.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfle.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFLE_D32, "lf-sfle-d32", "lf.sfle.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfueq.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFUEQ_S, "lf-sfueq-s", "lf.sfueq.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfueq.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFUEQ_D32, "lf-sfueq-d32", "lf.sfueq.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfune.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFUNE_S, "lf-sfune-s", "lf.sfune.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfune.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFUNE_D32, "lf-sfune-d32", "lf.sfune.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfugt.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFUGT_S, "lf-sfugt-s", "lf.sfugt.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfugt.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFUGT_D32, "lf-sfugt-d32", "lf.sfugt.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfuge.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFUGE_S, "lf-sfuge-s", "lf.sfuge.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfuge.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFUGE_D32, "lf-sfuge-d32", "lf.sfuge.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfult.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFULT_S, "lf-sfult-s", "lf.sfult.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfult.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFULT_D32, "lf-sfult-d32", "lf.sfult.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfule.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFULE_S, "lf-sfule-s", "lf.sfule.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfule.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFULE_D32, "lf-sfule-d32", "lf.sfule.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfun.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_SFUN_S, "lf-sfun-s", "lf.sfun.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.sfun.d $rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_SFUN_D32, "lf-sfun-d32", "lf.sfun.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.madd.s $rDSF,$rASF,$rBSF */
  {
    OR1K_INSN_LF_MADD_S, "lf-madd-s", "lf.madd.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.madd.d $rDD32F,$rAD32F,$rBD32F */
  {
    OR1K_INSN_LF_MADD_D32, "lf-madd-d32", "lf.madd.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.cust1.s $rASF,$rBSF */
  {
    OR1K_INSN_LF_CUST1_S, "lf-cust1-s", "lf.cust1.s", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
  },
/* lf.cust1.d */
  {
    OR1K_INSN_LF_CUST1_D32, "lf-cust1-d32", "lf.cust1.d", 32,
    { 0, { { { (1<<MACH_OR32)|(1<<MACH_OR32ND), 0 } } } }
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
static void or1k_cgen_rebuild_tables (CGEN_CPU_TABLE *);

/* Subroutine of or1k_cgen_cpu_open to look up a mach via its bfd name.  */

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

/* Subroutine of or1k_cgen_cpu_open to build the hardware table.  */

static void
build_hw_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_HW_ENTRY *init = & or1k_cgen_hw_table[0];
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

/* Subroutine of or1k_cgen_cpu_open to build the hardware table.  */

static void
build_ifield_table (CGEN_CPU_TABLE *cd)
{
  cd->ifld_table = & or1k_cgen_ifld_table[0];
}

/* Subroutine of or1k_cgen_cpu_open to build the hardware table.  */

static void
build_operand_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_OPERAND *init = & or1k_cgen_operand_table[0];
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

/* Subroutine of or1k_cgen_cpu_open to build the hardware table.
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
  const CGEN_IBASE *ib = & or1k_cgen_insn_table[0];
  CGEN_INSN *insns = xmalloc (MAX_INSNS * sizeof (CGEN_INSN));

  memset (insns, 0, MAX_INSNS * sizeof (CGEN_INSN));
  for (i = 0; i < MAX_INSNS; ++i)
    insns[i].base = &ib[i];
  cd->insn_table.init_entries = insns;
  cd->insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->insn_table.num_init_entries = MAX_INSNS;
}

/* Subroutine of or1k_cgen_cpu_open to rebuild the tables.  */

static void
or1k_cgen_rebuild_tables (CGEN_CPU_TABLE *cd)
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
	const CGEN_ISA *isa = & or1k_cgen_isa_table[i];

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
	const CGEN_MACH *mach = & or1k_cgen_mach_table[i];

	if (mach->insn_chunk_bitsize != 0)
	{
	  if (cd->insn_chunk_bitsize != 0 && cd->insn_chunk_bitsize != mach->insn_chunk_bitsize)
	    {
	      opcodes_error_handler
		(/* xgettext:c-format */
		 _("internal error: or1k_cgen_rebuild_tables: "
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
or1k_cgen_cpu_open (enum cgen_cpu_open_arg arg_type, ...)
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
	      lookup_mach_via_bfd_name (or1k_cgen_mach_table, name);

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
	     _("internal error: or1k_cgen_cpu_open: "
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
	 _("internal error: or1k_cgen_cpu_open: no endianness specified"));
      abort ();
    }

  cd->isas = cgen_bitset_copy (isas);
  cd->machs = machs;
  cd->endian = endian;
  cd->insn_endian
    = (insn_endian == CGEN_ENDIAN_UNKNOWN ? endian : insn_endian);

  /* Table (re)builder.  */
  cd->rebuild_tables = or1k_cgen_rebuild_tables;
  or1k_cgen_rebuild_tables (cd);

  /* Default to not allowing signed overflow.  */
  cd->signed_overflow_ok_p = 0;

  return (CGEN_CPU_DESC) cd;
}

/* Cover fn to or1k_cgen_cpu_open to handle the simple case of 1 isa, 1 mach.
   MACH_NAME is the bfd name of the mach.  */

CGEN_CPU_DESC
or1k_cgen_cpu_open_1 (const char *mach_name, enum cgen_endian endian)
{
  return or1k_cgen_cpu_open (CGEN_CPU_OPEN_BFDMACH, mach_name,
			       CGEN_CPU_OPEN_ENDIAN, endian,
			       CGEN_CPU_OPEN_END);
}

/* Close a cpu table.
   ??? This can live in a machine independent file, but there's currently
   no place to put this file (there's no libcgen).  libopcodes is the wrong
   place as some simulator ports use this but they don't use libopcodes.  */

void
or1k_cgen_cpu_close (CGEN_CPU_DESC cd)
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

