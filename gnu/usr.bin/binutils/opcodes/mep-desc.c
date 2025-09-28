/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* CPU data for mep.

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
#include "mep-desc.h"
#include "mep-opc.h"
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
  { "mep", MACH_MEP },
  { "h1", MACH_H1 },
  { "c5", MACH_C5 },
  { "max", MACH_MAX },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY ISA_attr[] ATTRIBUTE_UNUSED =
{
  { "mep", ISA_MEP },
  { "ext_core1", ISA_EXT_CORE1 },
  { "ext_cop1_16", ISA_EXT_COP1_16 },
  { "ext_cop1_32", ISA_EXT_COP1_32 },
  { "ext_cop1_48", ISA_EXT_COP1_48 },
  { "ext_cop1_64", ISA_EXT_COP1_64 },
  { "max", ISA_MAX },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY CDATA_attr[] ATTRIBUTE_UNUSED =
{
  { "LABEL", CDATA_LABEL },
  { "REGNUM", CDATA_REGNUM },
  { "FMAX_FLOAT", CDATA_FMAX_FLOAT },
  { "FMAX_INT", CDATA_FMAX_INT },
  { "POINTER", CDATA_POINTER },
  { "LONG", CDATA_LONG },
  { "ULONG", CDATA_ULONG },
  { "SHORT", CDATA_SHORT },
  { "USHORT", CDATA_USHORT },
  { "CHAR", CDATA_CHAR },
  { "UCHAR", CDATA_UCHAR },
  { "CP_DATA_BUS_INT", CDATA_CP_DATA_BUS_INT },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY CPTYPE_attr[] ATTRIBUTE_UNUSED =
{
  { "CP_DATA_BUS_INT", CPTYPE_CP_DATA_BUS_INT },
  { "VECT", CPTYPE_VECT },
  { "V2SI", CPTYPE_V2SI },
  { "V4HI", CPTYPE_V4HI },
  { "V8QI", CPTYPE_V8QI },
  { "V2USI", CPTYPE_V2USI },
  { "V4UHI", CPTYPE_V4UHI },
  { "V8UQI", CPTYPE_V8UQI },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY CRET_attr[] ATTRIBUTE_UNUSED =
{
  { "VOID", CRET_VOID },
  { "FIRST", CRET_FIRST },
  { "FIRSTCOPY", CRET_FIRSTCOPY },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY ALIGN_attr [] ATTRIBUTE_UNUSED =
{
  {"integer", 1},
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY LATENCY_attr [] ATTRIBUTE_UNUSED =
{
  {"integer", 0},
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY CONFIG_attr[] ATTRIBUTE_UNUSED =
{
  { "NONE", CONFIG_NONE },
  { "default", CONFIG_DEFAULT },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY SLOTS_attr[] ATTRIBUTE_UNUSED =
{
  { "CORE", SLOTS_CORE },
  { "C3", SLOTS_C3 },
  { "P0S", SLOTS_P0S },
  { "P0", SLOTS_P0 },
  { "P1", SLOTS_P1 },
  { 0, 0 }
};

const CGEN_ATTR_TABLE mep_cgen_ifield_attr_table[] =
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

const CGEN_ATTR_TABLE mep_cgen_hardware_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "ISA", & ISA_attr[0], & ISA_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "CACHE-ADDR", &bool_attr[0], &bool_attr[0] },
  { "PC", &bool_attr[0], &bool_attr[0] },
  { "PROFILE", &bool_attr[0], &bool_attr[0] },
  { "IS_FLOAT", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE mep_cgen_operand_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "ISA", & ISA_attr[0], & ISA_attr[0] },
  { "CDATA", & CDATA_attr[0], & CDATA_attr[0] },
  { "ALIGN", & ALIGN_attr[0], & ALIGN_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "PCREL-ADDR", &bool_attr[0], &bool_attr[0] },
  { "ABS-ADDR", &bool_attr[0], &bool_attr[0] },
  { "SIGN-OPT", &bool_attr[0], &bool_attr[0] },
  { "SIGNED", &bool_attr[0], &bool_attr[0] },
  { "NEGATIVE", &bool_attr[0], &bool_attr[0] },
  { "RELAX", &bool_attr[0], &bool_attr[0] },
  { "SEM-ONLY", &bool_attr[0], &bool_attr[0] },
  { "RELOC_IMPLIES_OVERFLOW", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE mep_cgen_insn_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "ISA", & ISA_attr[0], & ISA_attr[0] },
  { "CPTYPE", & CPTYPE_attr[0], & CPTYPE_attr[0] },
  { "CRET", & CRET_attr[0], & CRET_attr[0] },
  { "LATENCY", & LATENCY_attr[0], & LATENCY_attr[0] },
  { "CONFIG", & CONFIG_attr[0], & CONFIG_attr[0] },
  { "SLOTS", & SLOTS_attr[0], & SLOTS_attr[0] },
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
  { "OPTIONAL_BIT_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_MUL_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_DIV_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_DEBUG_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_LDZ_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_ABS_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_AVE_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_MINMAX_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_CLIP_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_SAT_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_UCI_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_DSP_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_CP_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_CP64_INSN", &bool_attr[0], &bool_attr[0] },
  { "OPTIONAL_VLIW64", &bool_attr[0], &bool_attr[0] },
  { "MAY_TRAP", &bool_attr[0], &bool_attr[0] },
  { "VLIW_ALONE", &bool_attr[0], &bool_attr[0] },
  { "VLIW_NO_CORE_NOP", &bool_attr[0], &bool_attr[0] },
  { "VLIW_NO_COP_NOP", &bool_attr[0], &bool_attr[0] },
  { "VLIW64_NO_MATCHING_NOP", &bool_attr[0], &bool_attr[0] },
  { "VLIW32_NO_MATCHING_NOP", &bool_attr[0], &bool_attr[0] },
  { "VOLATILE", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

/* Instruction set variants.  */

static const CGEN_ISA mep_cgen_isa_table[] = {
  { "mep", 32, 32, 16, 32 },
  { "ext_core1", 32, 32, 16, 32 },
  { "ext_cop1_16", 32, 32, 32, 32 },
  { "ext_cop1_32", 32, 32, 32, 32 },
  { "ext_cop1_48", 32, 32, 32, 32 },
  { "ext_cop1_64", 32, 32, 32, 32 },
  { 0, 0, 0, 0, 0 }
};

/* Machine variants.  */

static const CGEN_MACH mep_cgen_mach_table[] = {
  { "mep", "mep", MACH_MEP, 16 },
  { "h1", "h1", MACH_H1, 16 },
  { "c5", "c5", MACH_C5, 16 },
  { 0, 0, 0, 0 }
};

static CGEN_KEYWORD_ENTRY mep_cgen_opval_h_gpr_entries[] =
{
  { "$0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "$1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "$2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "$3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "$4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "$5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "$6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "$7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "$8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "$9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "$10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "$11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "$fp", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "$tp", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "$gp", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "$sp", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "$12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "$13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "$14", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "$15", 15, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD mep_cgen_opval_h_gpr =
{
  & mep_cgen_opval_h_gpr_entries[0],
  20,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY mep_cgen_opval_h_csr_entries[] =
{
  { "$pc", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "$lp", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "$sar", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "$rpb", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "$rpe", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "$rpc", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "$hi", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "$lo", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "$mb0", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "$me0", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "$mb1", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "$me1", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "$psw", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "$id", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "$tmp", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "$epc", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "$exc", 20, {0, {{{0, 0}}}}, 0, 0 },
  { "$cfg", 21, {0, {{{0, 0}}}}, 0, 0 },
  { "$npc", 23, {0, {{{0, 0}}}}, 0, 0 },
  { "$dbg", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "$depc", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "$opt", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "$rcfg", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccfg", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "$vid", 22, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD mep_cgen_opval_h_csr =
{
  & mep_cgen_opval_h_csr_entries[0],
  25,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY mep_cgen_opval_h_cr64_entries[] =
{
  { "$c0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "$c1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "$c2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "$c3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "$c4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "$c5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "$c6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "$c7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "$c8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "$c9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "$c10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "$c11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "$c12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "$c13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "$c14", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "$c15", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "$c16", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "$c17", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "$c18", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "$c19", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "$c20", 20, {0, {{{0, 0}}}}, 0, 0 },
  { "$c21", 21, {0, {{{0, 0}}}}, 0, 0 },
  { "$c22", 22, {0, {{{0, 0}}}}, 0, 0 },
  { "$c23", 23, {0, {{{0, 0}}}}, 0, 0 },
  { "$c24", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "$c25", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "$c26", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "$c27", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "$c28", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "$c29", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "$c30", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "$c31", 31, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD mep_cgen_opval_h_cr64 =
{
  & mep_cgen_opval_h_cr64_entries[0],
  32,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY mep_cgen_opval_h_cr_entries[] =
{
  { "$c0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "$c1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "$c2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "$c3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "$c4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "$c5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "$c6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "$c7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "$c8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "$c9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "$c10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "$c11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "$c12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "$c13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "$c14", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "$c15", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "$c16", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "$c17", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "$c18", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "$c19", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "$c20", 20, {0, {{{0, 0}}}}, 0, 0 },
  { "$c21", 21, {0, {{{0, 0}}}}, 0, 0 },
  { "$c22", 22, {0, {{{0, 0}}}}, 0, 0 },
  { "$c23", 23, {0, {{{0, 0}}}}, 0, 0 },
  { "$c24", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "$c25", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "$c26", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "$c27", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "$c28", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "$c29", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "$c30", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "$c31", 31, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD mep_cgen_opval_h_cr =
{
  & mep_cgen_opval_h_cr_entries[0],
  32,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY mep_cgen_opval_h_ccr_entries[] =
{
  { "$ccr0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr14", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr15", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr16", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr17", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr18", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr19", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr20", 20, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr21", 21, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr22", 22, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr23", 23, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr24", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr25", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr26", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr27", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr28", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr29", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr30", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr31", 31, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr32", 32, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr33", 33, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr34", 34, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr35", 35, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr36", 36, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr37", 37, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr38", 38, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr39", 39, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr40", 40, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr41", 41, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr42", 42, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr43", 43, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr44", 44, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr45", 45, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr46", 46, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr47", 47, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr48", 48, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr49", 49, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr50", 50, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr51", 51, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr52", 52, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr53", 53, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr54", 54, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr55", 55, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr56", 56, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr57", 57, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr58", 58, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr59", 59, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr60", 60, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr61", 61, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr62", 62, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr63", 63, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD mep_cgen_opval_h_ccr =
{
  & mep_cgen_opval_h_ccr_entries[0],
  64,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY mep_cgen_opval_h_cr_ivc2_entries[] =
{
  { "$c0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "$c1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "$c2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "$c3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "$c4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "$c5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "$c6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "$c7", 7, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD mep_cgen_opval_h_cr_ivc2 =
{
  & mep_cgen_opval_h_cr_ivc2_entries[0],
  8,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY mep_cgen_opval_h_ccr_ivc2_entries[] =
{
  { "$csar0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "$cc", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "$cofr0", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "$cofr1", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "$cofa0", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "$cofa1", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "$csar1", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc0_0", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc0_1", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc0_2", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc0_3", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc0_4", 20, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc0_5", 21, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc0_6", 22, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc0_7", 23, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc1_0", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc1_1", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc1_2", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc1_3", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc1_4", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc1_5", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc1_6", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "$acc1_7", 31, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr14", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr15", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr16", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr17", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr18", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr19", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr20", 20, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr21", 21, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr22", 22, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr23", 23, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr24", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr25", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr26", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr27", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr28", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr29", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr30", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "$ccr31", 31, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD mep_cgen_opval_h_ccr_ivc2 =
{
  & mep_cgen_opval_h_ccr_ivc2_entries[0],
  55,
  0, 0, 0, 0, ""
};


/* The hardware table.  */

#define A(a) (1 << CGEN_HW_##a)

const CGEN_HW_ENTRY mep_cgen_hw_table[] =
{
  { "h-memory", HW_H_MEMORY, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-sint", HW_H_SINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-uint", HW_H_UINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-addr", HW_H_ADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-iaddr", HW_H_IADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-pc", HW_H_PC, CGEN_ASM_NONE, 0, { 0|A(PROFILE)|A(PC), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-gpr", HW_H_GPR, CGEN_ASM_KEYWORD, & mep_cgen_opval_h_gpr, { 0|A(PROFILE)|A(CACHE_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-csr", HW_H_CSR, CGEN_ASM_KEYWORD, & mep_cgen_opval_h_csr, { 0|A(PROFILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-cr64", HW_H_CR64, CGEN_ASM_KEYWORD, & mep_cgen_opval_h_cr64, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-cr64-w", HW_H_CR64_W, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-cr", HW_H_CR, CGEN_ASM_KEYWORD, & mep_cgen_opval_h_cr, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-ccr", HW_H_CCR, CGEN_ASM_KEYWORD, & mep_cgen_opval_h_ccr, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-ccr-w", HW_H_CCR_W, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { "h-cr-ivc2", HW_H_CR_IVC2, CGEN_ASM_KEYWORD, & mep_cgen_opval_h_cr_ivc2, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } } },
  { "h-ccr-ivc2", HW_H_CCR_IVC2, CGEN_ASM_KEYWORD, & mep_cgen_opval_h_ccr_ivc2, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } } },
  { 0, 0, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } } } } }
};

#undef A


/* The instruction field table.  */

#define A(a) (1 << CGEN_IFLD_##a)

const CGEN_IFLD mep_cgen_ifld_table[] =
{
  { MEP_F_NIL, "f-nil", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_ANYOF, "f-anyof", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_MAJOR, "f-major", 0, 32, 0, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_RN, "f-rn", 0, 32, 4, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_RN3, "f-rn3", 0, 32, 5, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_RM, "f-rm", 0, 32, 8, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_RL, "f-rl", 0, 32, 12, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_SUB2, "f-sub2", 0, 32, 14, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_SUB3, "f-sub3", 0, 32, 13, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_SUB4, "f-sub4", 0, 32, 12, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_EXT, "f-ext", 0, 32, 16, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_EXT4, "f-ext4", 0, 32, 16, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_EXT62, "f-ext62", 0, 32, 20, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CRN, "f-crn", 0, 32, 4, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CSRN_HI, "f-csrn-hi", 0, 32, 15, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CSRN_LO, "f-csrn-lo", 0, 32, 8, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CSRN, "f-csrn", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CRNX_HI, "f-crnx-hi", 0, 32, 28, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CRNX_LO, "f-crnx-lo", 0, 32, 4, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CRNX, "f-crnx", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_0, "f-0", 0, 32, 0, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_1, "f-1", 0, 32, 1, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_2, "f-2", 0, 32, 2, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_3, "f-3", 0, 32, 3, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_4, "f-4", 0, 32, 4, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_5, "f-5", 0, 32, 5, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_6, "f-6", 0, 32, 6, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_7, "f-7", 0, 32, 7, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_8, "f-8", 0, 32, 8, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_9, "f-9", 0, 32, 9, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_10, "f-10", 0, 32, 10, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_11, "f-11", 0, 32, 11, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_12, "f-12", 0, 32, 12, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_13, "f-13", 0, 32, 13, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_14, "f-14", 0, 32, 14, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_15, "f-15", 0, 32, 15, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_16, "f-16", 0, 32, 16, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_17, "f-17", 0, 32, 17, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_18, "f-18", 0, 32, 18, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_19, "f-19", 0, 32, 19, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_20, "f-20", 0, 32, 20, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_21, "f-21", 0, 32, 21, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_22, "f-22", 0, 32, 22, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_23, "f-23", 0, 32, 23, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_24, "f-24", 0, 32, 24, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_25, "f-25", 0, 32, 25, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_26, "f-26", 0, 32, 26, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_27, "f-27", 0, 32, 27, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_28, "f-28", 0, 32, 28, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_29, "f-29", 0, 32, 29, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_30, "f-30", 0, 32, 30, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_31, "f-31", 0, 32, 31, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_8S8A2, "f-8s8a2", 0, 32, 8, 7, { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_12S4A2, "f-12s4a2", 0, 32, 4, 11, { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_17S16A2, "f-17s16a2", 0, 32, 16, 16, { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24S5A2N_HI, "f-24s5a2n-hi", 0, 32, 16, 16, { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24S5A2N_LO, "f-24s5a2n-lo", 0, 32, 5, 7, { 0|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24S5A2N, "f-24s5a2n", 0, 0, 0, 0,{ 0|A(PCREL_ADDR)|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U5A2N_HI, "f-24u5a2n-hi", 0, 32, 16, 16, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U5A2N_LO, "f-24u5a2n-lo", 0, 32, 5, 7, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U5A2N, "f-24u5a2n", 0, 0, 0, 0,{ 0|A(ABS_ADDR)|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_2U6, "f-2u6", 0, 32, 6, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_7U9, "f-7u9", 0, 32, 9, 7, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_7U9A2, "f-7u9a2", 0, 32, 9, 6, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_7U9A4, "f-7u9a4", 0, 32, 9, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_16S16, "f-16s16", 0, 32, 16, 16, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_2U10, "f-2u10", 0, 32, 10, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_3U5, "f-3u5", 0, 32, 5, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_4U8, "f-4u8", 0, 32, 8, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_5U8, "f-5u8", 0, 32, 8, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_5U24, "f-5u24", 0, 32, 24, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_6S8, "f-6s8", 0, 32, 8, 6, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_8S8, "f-8s8", 0, 32, 8, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_16U16, "f-16u16", 0, 32, 16, 16, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_12U16, "f-12u16", 0, 32, 16, 12, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_3U29, "f-3u29", 0, 32, 29, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CDISP10, "f-cdisp10", 0, 32, 22, 10, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U8A4N_HI, "f-24u8a4n-hi", 0, 32, 16, 16, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U8A4N_LO, "f-24u8a4n-lo", 0, 32, 8, 6, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U8A4N, "f-24u8a4n", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U8N_HI, "f-24u8n-hi", 0, 32, 16, 16, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U8N_LO, "f-24u8n-lo", 0, 32, 8, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U8N, "f-24u8n", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U4N_HI, "f-24u4n-hi", 0, 32, 4, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U4N_LO, "f-24u4n-lo", 0, 32, 16, 16, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_24U4N, "f-24u4n", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CALLNUM, "f-callnum", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CCRN_HI, "f-ccrn-hi", 0, 32, 28, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CCRN_LO, "f-ccrn-lo", 0, 32, 4, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_CCRN, "f-ccrn", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_C5N4, "f-c5n4", 0, 32, 16, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_C5N5, "f-c5n5", 0, 32, 20, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_C5N6, "f-c5n6", 0, 32, 24, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_C5N7, "f-c5n7", 0, 32, 28, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_RL5, "f-rl5", 0, 32, 20, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_12S20, "f-12s20", 0, 32, 20, 12, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } } } }  },
  { MEP_F_C5_RNM, "f-c5-rnm", 0, 32, 4, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_C5_RM, "f-c5-rm", 0, 32, 8, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_C5_16U16, "f-c5-16u16", 0, 32, 16, 16, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_C5_RMUIMM20, "f-c5-rmuimm20", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_C5_RNMUIMM24, "f-c5-rnmuimm24", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_2U4, "f-ivc2-2u4", 0, 32, 4, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_3U4, "f-ivc2-3u4", 0, 32, 4, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_8U4, "f-ivc2-8u4", 0, 32, 4, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_8S4, "f-ivc2-8s4", 0, 32, 4, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_1U6, "f-ivc2-1u6", 0, 32, 6, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_2U6, "f-ivc2-2u6", 0, 32, 6, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_3U6, "f-ivc2-3u6", 0, 32, 6, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_6U6, "f-ivc2-6u6", 0, 32, 6, 6, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U7, "f-ivc2-5u7", 0, 32, 7, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_4U8, "f-ivc2-4u8", 0, 32, 8, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_3U9, "f-ivc2-3u9", 0, 32, 9, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U16, "f-ivc2-5u16", 0, 32, 16, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U21, "f-ivc2-5u21", 0, 32, 21, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U26, "f-ivc2-5u26", 0, 32, 26, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_1U31, "f-ivc2-1u31", 0, 32, 31, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_4U16, "f-ivc2-4u16", 0, 32, 16, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_4U20, "f-ivc2-4u20", 0, 32, 20, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_4U24, "f-ivc2-4u24", 0, 32, 24, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_4U28, "f-ivc2-4u28", 0, 32, 28, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_2U0, "f-ivc2-2u0", 0, 32, 0, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_3U0, "f-ivc2-3u0", 0, 32, 0, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_4U0, "f-ivc2-4u0", 0, 32, 0, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U0, "f-ivc2-5u0", 0, 32, 0, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_8U0, "f-ivc2-8u0", 0, 32, 0, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_8S0, "f-ivc2-8s0", 0, 32, 0, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_6U2, "f-ivc2-6u2", 0, 32, 2, 6, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U3, "f-ivc2-5u3", 0, 32, 3, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_4U4, "f-ivc2-4u4", 0, 32, 4, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_3U5, "f-ivc2-3u5", 0, 32, 5, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U8, "f-ivc2-5u8", 0, 32, 8, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_4U10, "f-ivc2-4u10", 0, 32, 10, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_3U12, "f-ivc2-3u12", 0, 32, 12, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U13, "f-ivc2-5u13", 0, 32, 13, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_2U18, "f-ivc2-2u18", 0, 32, 18, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U18, "f-ivc2-5u18", 0, 32, 18, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_8U20, "f-ivc2-8u20", 0, 32, 20, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_8S20, "f-ivc2-8s20", 0, 32, 20, 8, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_5U23, "f-ivc2-5u23", 0, 32, 23, 5, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_2U23, "f-ivc2-2u23", 0, 32, 23, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_3U25, "f-ivc2-3u25", 0, 32, 25, 3, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_IMM16P0, "f-ivc2-imm16p0", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_SIMM16P0, "f-ivc2-simm16p0", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CCRN_C3HI, "f-ivc2-ccrn-c3hi", 0, 32, 28, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CCRN_C3LO, "f-ivc2-ccrn-c3lo", 0, 32, 4, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CRN, "f-ivc2-crn", 0, 32, 0, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CRM, "f-ivc2-crm", 0, 32, 4, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CCRN_H1, "f-ivc2-ccrn-h1", 0, 32, 20, 1, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CCRN_H2, "f-ivc2-ccrn-h2", 0, 32, 20, 2, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CCRN_LO, "f-ivc2-ccrn-lo", 0, 32, 0, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CMOV1, "f-ivc2-cmov1", 0, 32, 8, 12, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CMOV2, "f-ivc2-cmov2", 0, 32, 22, 6, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CMOV3, "f-ivc2-cmov3", 0, 32, 28, 4, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CCRN_C3, "f-ivc2-ccrn-c3", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CCRN, "f-ivc2-ccrn", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { MEP_F_IVC2_CRNX, "f-ivc2-crnx", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } } } }  },
  { 0, 0, 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } } } } }
};

#undef A



/* multi ifield declarations */

const CGEN_MAYBE_MULTI_IFLD MEP_F_CSRN_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_CRNX_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_24S5A2N_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_24U5A2N_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_24U8A4N_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_24U8N_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_24U4N_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_CALLNUM_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_CCRN_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_C5_RMUIMM20_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_C5_RNMUIMM24_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_IMM16P0_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_SIMM16P0_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_CCRN_C3_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_CCRN_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_CRNX_MULTI_IFIELD [];


/* multi ifield definitions */

const CGEN_MAYBE_MULTI_IFLD MEP_F_CSRN_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_CSRN_HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_CSRN_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_CRNX_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_CRNX_HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_CRNX_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_24S5A2N_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_24S5A2N_HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_24S5A2N_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_24U5A2N_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_24U5A2N_HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_24U5A2N_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_24U8A4N_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_24U8A4N_HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_24U8A4N_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_24U8N_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_24U8N_HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_24U8N_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_24U4N_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_24U4N_HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_24U4N_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_CALLNUM_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_5] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_6] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_7] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_11] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_CCRN_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_CCRN_HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_CCRN_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_C5_RMUIMM20_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_C5_RM] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_C5_16U16] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_C5_RNMUIMM24_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_C5_RNM] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_C5_16U16] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_IMM16P0_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8U0] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8U20] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_SIMM16P0_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8U0] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8U20] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_CCRN_C3_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_CCRN_C3HI] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_CCRN_C3LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_CCRN_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_CCRN_H2] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_CCRN_LO] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD MEP_F_IVC2_CRNX_MULTI_IFIELD [] =
{
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_CCRN_H1] } },
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_CCRN_LO] } },
    { 0, { 0 } }
};

/* The operand table.  */

#define A(a) (1 << CGEN_OPERAND_##a)
#define OPERAND(op) MEP_OPERAND_##op

const CGEN_OPERAND mep_cgen_operand_table[] =
{
/* pc: program counter */
  { "pc", MEP_OPERAND_PC, HW_H_PC, 0, 0,
    { 0, { &mep_cgen_ifld_table[MEP_F_NIL] } },
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* r0: register 0 */
  { "r0", MEP_OPERAND_R0, HW_H_GPR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rn: register Rn */
  { "rn", MEP_OPERAND_RN, HW_H_GPR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rm: register Rm */
  { "rm", MEP_OPERAND_RM, HW_H_GPR, 8, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RM] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rl: register Rl */
  { "rl", MEP_OPERAND_RL, HW_H_GPR, 12, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RL] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rn3: register 0-7 */
  { "rn3", MEP_OPERAND_RN3, HW_H_GPR, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN3] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rma: register Rm holding pointer */
  { "rma", MEP_OPERAND_RMA, HW_H_GPR, 8, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RM] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_POINTER, 0 } }, { { 1, 0 } } } }  },
/* rnc: register Rn holding char */
  { "rnc", MEP_OPERAND_RNC, HW_H_GPR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rnuc: register Rn holding unsigned char */
  { "rnuc", MEP_OPERAND_RNUC, HW_H_GPR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rns: register Rn holding short */
  { "rns", MEP_OPERAND_RNS, HW_H_GPR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rnus: register Rn holding unsigned short */
  { "rnus", MEP_OPERAND_RNUS, HW_H_GPR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rnl: register Rn holding long */
  { "rnl", MEP_OPERAND_RNL, HW_H_GPR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rnul: register Rn holding unsigned  long */
  { "rnul", MEP_OPERAND_RNUL, HW_H_GPR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_ULONG, 0 } }, { { 1, 0 } } } }  },
/* rn3c: register 0-7 holding unsigned char */
  { "rn3c", MEP_OPERAND_RN3C, HW_H_GPR, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN3] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rn3uc: register 0-7 holding byte */
  { "rn3uc", MEP_OPERAND_RN3UC, HW_H_GPR, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN3] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rn3s: register 0-7 holding unsigned short */
  { "rn3s", MEP_OPERAND_RN3S, HW_H_GPR, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN3] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rn3us: register 0-7 holding short */
  { "rn3us", MEP_OPERAND_RN3US, HW_H_GPR, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN3] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rn3l: register 0-7 holding unsigned long */
  { "rn3l", MEP_OPERAND_RN3L, HW_H_GPR, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN3] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rn3ul: register 0-7 holding long */
  { "rn3ul", MEP_OPERAND_RN3UL, HW_H_GPR, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN3] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_ULONG, 0 } }, { { 1, 0 } } } }  },
/* lp: link pointer */
  { "lp", MEP_OPERAND_LP, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* sar: shift amount register */
  { "sar", MEP_OPERAND_SAR, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* hi: high result */
  { "hi", MEP_OPERAND_HI, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* lo: low result */
  { "lo", MEP_OPERAND_LO, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* mb0: modulo begin register 0 */
  { "mb0", MEP_OPERAND_MB0, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* me0: modulo end register 0 */
  { "me0", MEP_OPERAND_ME0, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* mb1: modulo begin register 1 */
  { "mb1", MEP_OPERAND_MB1, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* me1: modulo end register 1 */
  { "me1", MEP_OPERAND_ME1, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* psw: program status word */
  { "psw", MEP_OPERAND_PSW, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* epc: exception prog counter */
  { "epc", MEP_OPERAND_EPC, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* exc: exception cause */
  { "exc", MEP_OPERAND_EXC, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* npc: nmi program counter */
  { "npc", MEP_OPERAND_NPC, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* dbg: debug register */
  { "dbg", MEP_OPERAND_DBG, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* depc: debug exception pc */
  { "depc", MEP_OPERAND_DEPC, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* opt: option register */
  { "opt", MEP_OPERAND_OPT, HW_H_CSR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* r1: register 1 */
  { "r1", MEP_OPERAND_R1, HW_H_GPR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* tp: tiny data area pointer */
  { "tp", MEP_OPERAND_TP, HW_H_GPR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* sp: stack pointer */
  { "sp", MEP_OPERAND_SP, HW_H_GPR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* tpr: comment */
  { "tpr", MEP_OPERAND_TPR, HW_H_GPR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* spr: comment */
  { "spr", MEP_OPERAND_SPR, HW_H_GPR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* csrn: control/special register */
  { "csrn", MEP_OPERAND_CSRN, HW_H_CSR, 8, 5,
    { 2, { &MEP_F_CSRN_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_REGNUM, 0 } }, { { 1, 0 } } } }  },
/* csrn-idx: control/special reg idx */
  { "csrn-idx", MEP_OPERAND_CSRN_IDX, HW_H_UINT, 8, 5,
    { 2, { &MEP_F_CSRN_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* crn64: copro Rn (64-bit) */
  { "crn64", MEP_OPERAND_CRN64, HW_H_CR64, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_CRN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* crn: copro Rn (32-bit) */
  { "crn", MEP_OPERAND_CRN, HW_H_CR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_CRN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* crnx64: copro Rn (0-31, 64-bit) */
  { "crnx64", MEP_OPERAND_CRNX64, HW_H_CR64, 4, 5,
    { 2, { &MEP_F_CRNX_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* crnx: copro Rn (0-31, 32-bit) */
  { "crnx", MEP_OPERAND_CRNX, HW_H_CR, 4, 5,
    { 2, { &MEP_F_CRNX_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* ccrn: copro control reg CCRn */
  { "ccrn", MEP_OPERAND_CCRN, HW_H_CCR, 4, 6,
    { 2, { &MEP_F_CCRN_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_REGNUM, 0 } }, { { 1, 0 } } } }  },
/* cccc: copro flags */
  { "cccc", MEP_OPERAND_CCCC, HW_H_UINT, 8, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RM] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* pcrel8a2: comment */
  { "pcrel8a2", MEP_OPERAND_PCREL8A2, HW_H_SINT, 8, 7,
    { 0, { &mep_cgen_ifld_table[MEP_F_8S8A2] } },
    { 0|A(RELAX)|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LABEL, 0 } }, { { 1, 0 } } } }  },
/* pcrel12a2: comment */
  { "pcrel12a2", MEP_OPERAND_PCREL12A2, HW_H_SINT, 4, 11,
    { 0, { &mep_cgen_ifld_table[MEP_F_12S4A2] } },
    { 0|A(RELAX)|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LABEL, 0 } }, { { 1, 0 } } } }  },
/* pcrel17a2: comment */
  { "pcrel17a2", MEP_OPERAND_PCREL17A2, HW_H_SINT, 16, 16,
    { 0, { &mep_cgen_ifld_table[MEP_F_17S16A2] } },
    { 0|A(RELAX)|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LABEL, 0 } }, { { 1, 0 } } } }  },
/* pcrel24a2: comment */
  { "pcrel24a2", MEP_OPERAND_PCREL24A2, HW_H_SINT, 5, 23,
    { 2, { &MEP_F_24S5A2N_MULTI_IFIELD[0] } },
    { 0|A(PCREL_ADDR)|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LABEL, 0 } }, { { 1, 0 } } } }  },
/* pcabs24a2: comment */
  { "pcabs24a2", MEP_OPERAND_PCABS24A2, HW_H_UINT, 5, 23,
    { 2, { &MEP_F_24U5A2N_MULTI_IFIELD[0] } },
    { 0|A(ABS_ADDR)|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LABEL, 0 } }, { { 1, 0 } } } }  },
/* sdisp16: comment */
  { "sdisp16", MEP_OPERAND_SDISP16, HW_H_SINT, 16, 16,
    { 0, { &mep_cgen_ifld_table[MEP_F_16S16] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* simm16: comment */
  { "simm16", MEP_OPERAND_SIMM16, HW_H_SINT, 16, 16,
    { 0, { &mep_cgen_ifld_table[MEP_F_16S16] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* uimm16: comment */
  { "uimm16", MEP_OPERAND_UIMM16, HW_H_UINT, 16, 16,
    { 0, { &mep_cgen_ifld_table[MEP_F_16U16] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* code16: uci/dsp code (16 bits) */
  { "code16", MEP_OPERAND_CODE16, HW_H_UINT, 16, 16,
    { 0, { &mep_cgen_ifld_table[MEP_F_16U16] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* udisp2: SSARB addend (2 bits) */
  { "udisp2", MEP_OPERAND_UDISP2, HW_H_SINT, 6, 2,
    { 0, { &mep_cgen_ifld_table[MEP_F_2U6] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* uimm2: interrupt (2 bits) */
  { "uimm2", MEP_OPERAND_UIMM2, HW_H_UINT, 10, 2,
    { 0, { &mep_cgen_ifld_table[MEP_F_2U10] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* simm6: add const (6 bits) */
  { "simm6", MEP_OPERAND_SIMM6, HW_H_SINT, 8, 6,
    { 0, { &mep_cgen_ifld_table[MEP_F_6S8] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* simm8: mov const (8 bits) */
  { "simm8", MEP_OPERAND_SIMM8, HW_H_SINT, 8, 8,
    { 0, { &mep_cgen_ifld_table[MEP_F_8S8] } },
    { 0|A(RELOC_IMPLIES_OVERFLOW), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* addr24a4: comment */
  { "addr24a4", MEP_OPERAND_ADDR24A4, HW_H_UINT, 8, 22,
    { 2, { &MEP_F_24U8A4N_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 4, 0 } } } }  },
/* code24: coprocessor code */
  { "code24", MEP_OPERAND_CODE24, HW_H_UINT, 4, 24,
    { 2, { &MEP_F_24U4N_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* callnum: system call number */
  { "callnum", MEP_OPERAND_CALLNUM, HW_H_UINT, 5, 4,
    { 4, { &MEP_F_CALLNUM_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* uimm3: bit immediate (3 bits) */
  { "uimm3", MEP_OPERAND_UIMM3, HW_H_UINT, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_3U5] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* uimm4: bCC const (4 bits) */
  { "uimm4", MEP_OPERAND_UIMM4, HW_H_UINT, 8, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_4U8] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* uimm5: bit/shift val (5 bits) */
  { "uimm5", MEP_OPERAND_UIMM5, HW_H_UINT, 8, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_5U8] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* udisp7: comment */
  { "udisp7", MEP_OPERAND_UDISP7, HW_H_UINT, 9, 7,
    { 0, { &mep_cgen_ifld_table[MEP_F_7U9] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* udisp7a2: comment */
  { "udisp7a2", MEP_OPERAND_UDISP7A2, HW_H_UINT, 9, 6,
    { 0, { &mep_cgen_ifld_table[MEP_F_7U9A2] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 2, 0 } } } }  },
/* udisp7a4: comment */
  { "udisp7a4", MEP_OPERAND_UDISP7A4, HW_H_UINT, 9, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_7U9A4] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 4, 0 } } } }  },
/* uimm7a4: comment */
  { "uimm7a4", MEP_OPERAND_UIMM7A4, HW_H_UINT, 9, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_7U9A4] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 4, 0 } } } }  },
/* uimm24: immediate (24 bits) */
  { "uimm24", MEP_OPERAND_UIMM24, HW_H_UINT, 8, 24,
    { 2, { &MEP_F_24U8N_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* cimm4: cache immed'te (4 bits) */
  { "cimm4", MEP_OPERAND_CIMM4, HW_H_UINT, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RN] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* cimm5: clip immediate (5 bits) */
  { "cimm5", MEP_OPERAND_CIMM5, HW_H_UINT, 24, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_5U24] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* cdisp10: comment */
  { "cdisp10", MEP_OPERAND_CDISP10, HW_H_SINT, 22, 10,
    { 0, { &mep_cgen_ifld_table[MEP_F_CDISP10] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* cdisp10a2: comment */
  { "cdisp10a2", MEP_OPERAND_CDISP10A2, HW_H_SINT, 22, 10,
    { 0, { &mep_cgen_ifld_table[MEP_F_CDISP10] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* cdisp10a4: comment */
  { "cdisp10a4", MEP_OPERAND_CDISP10A4, HW_H_SINT, 22, 10,
    { 0, { &mep_cgen_ifld_table[MEP_F_CDISP10] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* cdisp10a8: comment */
  { "cdisp10a8", MEP_OPERAND_CDISP10A8, HW_H_SINT, 22, 10,
    { 0, { &mep_cgen_ifld_table[MEP_F_CDISP10] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* zero: Zero operand */
  { "zero", MEP_OPERAND_ZERO, HW_H_SINT, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* rl5: register Rl c5 */
  { "rl5", MEP_OPERAND_RL5, HW_H_GPR, 20, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_RL5] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* cdisp12: copro addend (12 bits) */
  { "cdisp12", MEP_OPERAND_CDISP12, HW_H_SINT, 20, 12,
    { 0, { &mep_cgen_ifld_table[MEP_F_12S20] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* c5rmuimm20: 20-bit immediate in rm and imm16 */
  { "c5rmuimm20", MEP_OPERAND_C5RMUIMM20, HW_H_UINT, 8, 20,
    { 2, { &MEP_F_C5_RMUIMM20_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* c5rnmuimm24: 24-bit immediate in rn, rm, and imm16 */
  { "c5rnmuimm24", MEP_OPERAND_C5RNMUIMM24, HW_H_UINT, 4, 24,
    { 2, { &MEP_F_C5_RNMUIMM24_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xd0" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* cp_flag: branch condition register */
  { "cp_flag", MEP_OPERAND_CP_FLAG, HW_H_CCR, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_csar0: ivc2_csar0 */
  { "ivc2_csar0", MEP_OPERAND_IVC2_CSAR0, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_cc: ivc2_cc */
  { "ivc2_cc", MEP_OPERAND_IVC2_CC, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_cofr0: ivc2_cofr0 */
  { "ivc2_cofr0", MEP_OPERAND_IVC2_COFR0, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_cofr1: ivc2_cofr1 */
  { "ivc2_cofr1", MEP_OPERAND_IVC2_COFR1, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_cofa0: ivc2_cofa0 */
  { "ivc2_cofa0", MEP_OPERAND_IVC2_COFA0, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_cofa1: ivc2_cofa1 */
  { "ivc2_cofa1", MEP_OPERAND_IVC2_COFA1, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_csar1: ivc2_csar1 */
  { "ivc2_csar1", MEP_OPERAND_IVC2_CSAR1, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc0_0: acc0_0 */
  { "ivc2_acc0_0", MEP_OPERAND_IVC2_ACC0_0, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc0_1: acc0_1 */
  { "ivc2_acc0_1", MEP_OPERAND_IVC2_ACC0_1, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc0_2: acc0_2 */
  { "ivc2_acc0_2", MEP_OPERAND_IVC2_ACC0_2, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc0_3: acc0_3 */
  { "ivc2_acc0_3", MEP_OPERAND_IVC2_ACC0_3, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc0_4: acc0_4 */
  { "ivc2_acc0_4", MEP_OPERAND_IVC2_ACC0_4, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc0_5: acc0_5 */
  { "ivc2_acc0_5", MEP_OPERAND_IVC2_ACC0_5, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc0_6: acc0_6 */
  { "ivc2_acc0_6", MEP_OPERAND_IVC2_ACC0_6, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc0_7: acc0_7 */
  { "ivc2_acc0_7", MEP_OPERAND_IVC2_ACC0_7, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc1_0: acc1_0 */
  { "ivc2_acc1_0", MEP_OPERAND_IVC2_ACC1_0, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc1_1: acc1_1 */
  { "ivc2_acc1_1", MEP_OPERAND_IVC2_ACC1_1, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc1_2: acc1_2 */
  { "ivc2_acc1_2", MEP_OPERAND_IVC2_ACC1_2, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc1_3: acc1_3 */
  { "ivc2_acc1_3", MEP_OPERAND_IVC2_ACC1_3, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc1_4: acc1_4 */
  { "ivc2_acc1_4", MEP_OPERAND_IVC2_ACC1_4, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc1_5: acc1_5 */
  { "ivc2_acc1_5", MEP_OPERAND_IVC2_ACC1_5, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc1_6: acc1_6 */
  { "ivc2_acc1_6", MEP_OPERAND_IVC2_ACC1_6, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2_acc1_7: acc1_7 */
  { "ivc2_acc1_7", MEP_OPERAND_IVC2_ACC1_7, HW_H_CCR_IVC2, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x7c" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* croc: $CRo C3 */
  { "croc", MEP_OPERAND_CROC, HW_H_CR64, 7, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U7] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* crqc: $CRq C3 */
  { "crqc", MEP_OPERAND_CRQC, HW_H_CR64, 21, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U21] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* crpc: $CRp C3 */
  { "crpc", MEP_OPERAND_CRPC, HW_H_CR64, 26, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U26] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* ivc-x-6-1: filler */
  { "ivc-x-6-1", MEP_OPERAND_IVC_X_6_1, HW_H_UINT, 6, 1,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_1U6] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc-x-6-2: filler */
  { "ivc-x-6-2", MEP_OPERAND_IVC_X_6_2, HW_H_UINT, 6, 2,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_2U6] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc-x-6-3: filler */
  { "ivc-x-6-3", MEP_OPERAND_IVC_X_6_3, HW_H_UINT, 6, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_3U6] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm3p4: Imm3p4 */
  { "imm3p4", MEP_OPERAND_IMM3P4, HW_H_UINT, 4, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_3U4] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm3p9: Imm3p9 */
  { "imm3p9", MEP_OPERAND_IMM3P9, HW_H_UINT, 9, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_3U9] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm4p8: Imm4p8 */
  { "imm4p8", MEP_OPERAND_IMM4P8, HW_H_UINT, 8, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_4U8] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm5p7: Imm5p7 */
  { "imm5p7", MEP_OPERAND_IMM5P7, HW_H_UINT, 7, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U7] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm6p6: Imm6p6 */
  { "imm6p6", MEP_OPERAND_IMM6P6, HW_H_UINT, 6, 6,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_6U6] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm8p4: Imm8p4 */
  { "imm8p4", MEP_OPERAND_IMM8P4, HW_H_UINT, 4, 8,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8U4] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* simm8p4: sImm8p4 */
  { "simm8p4", MEP_OPERAND_SIMM8P4, HW_H_SINT, 4, 8,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8S4] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm3p5: Imm3p5 */
  { "imm3p5", MEP_OPERAND_IMM3P5, HW_H_UINT, 5, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_3U5] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm3p12: Imm3p12 */
  { "imm3p12", MEP_OPERAND_IMM3P12, HW_H_UINT, 12, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_3U12] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm4p4: Imm4p4 */
  { "imm4p4", MEP_OPERAND_IMM4P4, HW_H_UINT, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_4U4] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm4p10: Imm4p10 */
  { "imm4p10", MEP_OPERAND_IMM4P10, HW_H_UINT, 10, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_4U10] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm5p8: Imm5p8 */
  { "imm5p8", MEP_OPERAND_IMM5P8, HW_H_UINT, 8, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U8] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm5p3: Imm5p3 */
  { "imm5p3", MEP_OPERAND_IMM5P3, HW_H_UINT, 3, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U3] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm6p2: Imm6p2 */
  { "imm6p2", MEP_OPERAND_IMM6P2, HW_H_UINT, 2, 6,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_6U2] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm5p23: Imm5p23 */
  { "imm5p23", MEP_OPERAND_IMM5P23, HW_H_UINT, 23, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U23] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm3p25: Imm3p25 */
  { "imm3p25", MEP_OPERAND_IMM3P25, HW_H_UINT, 25, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_3U25] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm8p0: Imm8p0 */
  { "imm8p0", MEP_OPERAND_IMM8P0, HW_H_UINT, 0, 8,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8U0] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* simm8p0: sImm8p0 */
  { "simm8p0", MEP_OPERAND_SIMM8P0, HW_H_SINT, 0, 8,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8S0] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* simm8p20: sImm8p20 */
  { "simm8p20", MEP_OPERAND_SIMM8P20, HW_H_SINT, 20, 8,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8S20] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm8p20: Imm8p20 */
  { "imm8p20", MEP_OPERAND_IMM8P20, HW_H_UINT, 20, 8,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_8U20] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* crop: $CRo Pn */
  { "crop", MEP_OPERAND_CROP, HW_H_CR64, 23, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U23] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* crqp: $CRq Pn */
  { "crqp", MEP_OPERAND_CRQP, HW_H_CR64, 13, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U13] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* crpp: $CRp Pn */
  { "crpp", MEP_OPERAND_CRPP, HW_H_CR64, 18, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U18] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* ivc-x-0-2: filler */
  { "ivc-x-0-2", MEP_OPERAND_IVC_X_0_2, HW_H_UINT, 0, 2,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_2U0] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc-x-0-3: filler */
  { "ivc-x-0-3", MEP_OPERAND_IVC_X_0_3, HW_H_UINT, 0, 3,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_3U0] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc-x-0-4: filler */
  { "ivc-x-0-4", MEP_OPERAND_IVC_X_0_4, HW_H_UINT, 0, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_4U0] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc-x-0-5: filler */
  { "ivc-x-0-5", MEP_OPERAND_IVC_X_0_5, HW_H_UINT, 0, 5,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_5U0] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* imm16p0: comment */
  { "imm16p0", MEP_OPERAND_IMM16P0, HW_H_UINT, 0, 16,
    { 2, { &MEP_F_IVC2_IMM16P0_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* simm16p0: comment */
  { "simm16p0", MEP_OPERAND_SIMM16P0, HW_H_SINT, 0, 16,
    { 2, { &MEP_F_IVC2_SIMM16P0_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2rm: reg Rm */
  { "ivc2rm", MEP_OPERAND_IVC2RM, HW_H_GPR, 4, 4,
    { 0, { &mep_cgen_ifld_table[MEP_F_IVC2_CRM] } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } }  },
/* ivc2crn: copro Rn (0-31, 64-bit */
  { "ivc2crn", MEP_OPERAND_IVC2CRN, HW_H_CR64, 0, 5,
    { 2, { &MEP_F_IVC2_CRNX_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_CP_DATA_BUS_INT, 0 } }, { { 1, 0 } } } }  },
/* ivc2ccrn: copro control reg CCRn */
  { "ivc2ccrn", MEP_OPERAND_IVC2CCRN, HW_H_CCR_IVC2, 0, 6,
    { 2, { &MEP_F_IVC2_CCRN_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_REGNUM, 0 } }, { { 1, 0 } } } }  },
/* ivc2c3ccrn: copro control reg CCRn */
  { "ivc2c3ccrn", MEP_OPERAND_IVC2C3CCRN, HW_H_CCR_IVC2, 4, 6,
    { 2, { &MEP_F_IVC2_CCRN_C3_MULTI_IFIELD[0] } },
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xfc" } }, { { CDATA_REGNUM, 0 } }, { { 1, 0 } } } }  },
/* sentinel */
  { 0, 0, 0, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } }, { { CDATA_LONG, 0 } }, { { 1, 0 } } } } }
};

#undef A


/* The instruction table.  */

#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))
#define A(a) (1 << CGEN_INSN_##a)

static const CGEN_IBASE mep_cgen_insn_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } } },
/* stcb $rn,($rma) */
  {
    MEP_INSN_STCB_R, "stcb_r", "stcb", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ldcb $rn,($rma) */
  {
    MEP_INSN_LDCB_R, "ldcb_r", "ldcb", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 3, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* pref $cimm4,($rma) */
  {
    MEP_INSN_PREF, "pref", "pref", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* pref $cimm4,$sdisp16($rma) */
  {
    MEP_INSN_PREFD, "prefd", "pref", 32,
    { 0|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* casb3 $rl5,$rn,($rm) */
  {
    MEP_INSN_CASB3, "casb3", "casb3", 32,
    { 0|A(OPTIONAL_BIT_INSN)|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* cash3 $rl5,$rn,($rm) */
  {
    MEP_INSN_CASH3, "cash3", "cash3", 32,
    { 0|A(OPTIONAL_BIT_INSN)|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* casw3 $rl5,$rn,($rm) */
  {
    MEP_INSN_CASW3, "casw3", "casw3", 32,
    { 0|A(OPTIONAL_BIT_INSN)|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sbcp $crn,$cdisp12($rma) */
  {
    MEP_INSN_SBCP, "sbcp", "sbcp", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbcp $crn,$cdisp12($rma) */
  {
    MEP_INSN_LBCP, "lbcp", "lbcp", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbucp $crn,$cdisp12($rma) */
  {
    MEP_INSN_LBUCP, "lbucp", "lbucp", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* shcp $crn,$cdisp12($rma) */
  {
    MEP_INSN_SHCP, "shcp", "shcp", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhcp $crn,$cdisp12($rma) */
  {
    MEP_INSN_LHCP, "lhcp", "lhcp", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhucp $crn,$cdisp12($rma) */
  {
    MEP_INSN_LHUCP, "lhucp", "lhucp", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbucpa $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_LBUCPA, "lbucpa", "lbucpa", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhucpa $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_LHUCPA, "lhucpa", "lhucpa", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbucpm0 $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_LBUCPM0, "lbucpm0", "lbucpm0", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhucpm0 $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_LHUCPM0, "lhucpm0", "lhucpm0", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbucpm1 $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_LBUCPM1, "lbucpm1", "lbucpm1", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhucpm1 $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_LHUCPM1, "lhucpm1", "lhucpm1", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* uci $rn,$rm,$uimm16 */
  {
    MEP_INSN_UCI, "uci", "uci", 32,
    { 0|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* dsp $rn,$rm,$uimm16 */
  {
    MEP_INSN_DSP, "dsp", "dsp", 32,
    { 0|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* dsp0 $c5rnmuimm24 */
  {
    -1, "dsp0", "dsp0", 32,
    { 0|A(ALIAS)|A(NO_DIS)|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* dsp1 $rn,$c5rmuimm20 */
  {
    -1, "dsp1", "dsp1", 32,
    { 0|A(ALIAS)|A(NO_DIS)|A(VOLATILE), { { { (1<<MACH_C5), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sb $rnc,($rma) */
  {
    MEP_INSN_SB, "sb", "sb", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sh $rns,($rma) */
  {
    MEP_INSN_SH, "sh", "sh", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sw $rnl,($rma) */
  {
    MEP_INSN_SW, "sw", "sw", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lb $rnc,($rma) */
  {
    MEP_INSN_LB, "lb", "lb", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lh $rns,($rma) */
  {
    MEP_INSN_LH, "lh", "lh", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lw $rnl,($rma) */
  {
    MEP_INSN_LW, "lw", "lw", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbu $rnuc,($rma) */
  {
    MEP_INSN_LBU, "lbu", "lbu", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhu $rnus,($rma) */
  {
    MEP_INSN_LHU, "lhu", "lhu", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sw $rnl,$udisp7a4($spr) */
  {
    MEP_INSN_SW_SP, "sw-sp", "sw", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lw $rnl,$udisp7a4($spr) */
  {
    MEP_INSN_LW_SP, "lw-sp", "lw", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sb $rn3c,$udisp7($tpr) */
  {
    MEP_INSN_SB_TP, "sb-tp", "sb", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sh $rn3s,$udisp7a2($tpr) */
  {
    MEP_INSN_SH_TP, "sh-tp", "sh", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sw $rn3l,$udisp7a4($tpr) */
  {
    MEP_INSN_SW_TP, "sw-tp", "sw", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lb $rn3c,$udisp7($tpr) */
  {
    MEP_INSN_LB_TP, "lb-tp", "lb", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lh $rn3s,$udisp7a2($tpr) */
  {
    MEP_INSN_LH_TP, "lh-tp", "lh", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lw $rn3l,$udisp7a4($tpr) */
  {
    MEP_INSN_LW_TP, "lw-tp", "lw", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbu $rn3uc,$udisp7($tpr) */
  {
    MEP_INSN_LBU_TP, "lbu-tp", "lbu", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhu $rn3us,$udisp7a2($tpr) */
  {
    MEP_INSN_LHU_TP, "lhu-tp", "lhu", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sb $rnc,$sdisp16($rma) */
  {
    MEP_INSN_SB16, "sb16", "sb", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sh $rns,$sdisp16($rma) */
  {
    MEP_INSN_SH16, "sh16", "sh", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sw $rnl,$sdisp16($rma) */
  {
    MEP_INSN_SW16, "sw16", "sw", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lb $rnc,$sdisp16($rma) */
  {
    MEP_INSN_LB16, "lb16", "lb", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lh $rns,$sdisp16($rma) */
  {
    MEP_INSN_LH16, "lh16", "lh", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lw $rnl,$sdisp16($rma) */
  {
    MEP_INSN_LW16, "lw16", "lw", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbu $rnuc,$sdisp16($rma) */
  {
    MEP_INSN_LBU16, "lbu16", "lbu", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhu $rnus,$sdisp16($rma) */
  {
    MEP_INSN_LHU16, "lhu16", "lhu", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sw $rnl,($addr24a4) */
  {
    MEP_INSN_SW24, "sw24", "sw", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lw $rnl,($addr24a4) */
  {
    MEP_INSN_LW24, "lw24", "lw", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* extb $rn */
  {
    MEP_INSN_EXTB, "extb", "extb", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* exth $rn */
  {
    MEP_INSN_EXTH, "exth", "exth", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* extub $rn */
  {
    MEP_INSN_EXTUB, "extub", "extub", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* extuh $rn */
  {
    MEP_INSN_EXTUH, "extuh", "extuh", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ssarb $udisp2($rm) */
  {
    MEP_INSN_SSARB, "ssarb", "ssarb", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* mov $rn,$rm */
  {
    MEP_INSN_MOV, "mov", "mov", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* mov $rn,$simm8 */
  {
    MEP_INSN_MOVI8, "movi8", "mov", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* mov $rn,$simm16 */
  {
    MEP_INSN_MOVI16, "movi16", "mov", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* movu $rn3,$uimm24 */
  {
    MEP_INSN_MOVU24, "movu24", "movu", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* movu $rn,$uimm16 */
  {
    MEP_INSN_MOVU16, "movu16", "movu", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* movh $rn,$uimm16 */
  {
    MEP_INSN_MOVH, "movh", "movh", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* add3 $rl,$rn,$rm */
  {
    MEP_INSN_ADD3, "add3", "add3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* add $rn,$simm6 */
  {
    MEP_INSN_ADD, "add", "add", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* add3 $rn,$spr,$uimm7a4 */
  {
    MEP_INSN_ADD3I, "add3i", "add3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* advck3 \$0,$rn,$rm */
  {
    MEP_INSN_ADVCK3, "advck3", "advck3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sub $rn,$rm */
  {
    MEP_INSN_SUB, "sub", "sub", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sbvck3 \$0,$rn,$rm */
  {
    MEP_INSN_SBVCK3, "sbvck3", "sbvck3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* neg $rn,$rm */
  {
    MEP_INSN_NEG, "neg", "neg", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* slt3 \$0,$rn,$rm */
  {
    MEP_INSN_SLT3, "slt3", "slt3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sltu3 \$0,$rn,$rm */
  {
    MEP_INSN_SLTU3, "sltu3", "sltu3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* slt3 \$0,$rn,$uimm5 */
  {
    MEP_INSN_SLT3I, "slt3i", "slt3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sltu3 \$0,$rn,$uimm5 */
  {
    MEP_INSN_SLTU3I, "sltu3i", "sltu3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sl1ad3 \$0,$rn,$rm */
  {
    MEP_INSN_SL1AD3, "sl1ad3", "sl1ad3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sl2ad3 \$0,$rn,$rm */
  {
    MEP_INSN_SL2AD3, "sl2ad3", "sl2ad3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* add3 $rn,$rm,$simm16 */
  {
    MEP_INSN_ADD3X, "add3x", "add3", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* slt3 $rn,$rm,$simm16 */
  {
    MEP_INSN_SLT3X, "slt3x", "slt3", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sltu3 $rn,$rm,$uimm16 */
  {
    MEP_INSN_SLTU3X, "sltu3x", "sltu3", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* or $rn,$rm */
  {
    MEP_INSN_OR, "or", "or", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* and $rn,$rm */
  {
    MEP_INSN_AND, "and", "and", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* xor $rn,$rm */
  {
    MEP_INSN_XOR, "xor", "xor", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* nor $rn,$rm */
  {
    MEP_INSN_NOR, "nor", "nor", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* or3 $rn,$rm,$uimm16 */
  {
    MEP_INSN_OR3, "or3", "or3", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* and3 $rn,$rm,$uimm16 */
  {
    MEP_INSN_AND3, "and3", "and3", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* xor3 $rn,$rm,$uimm16 */
  {
    MEP_INSN_XOR3, "xor3", "xor3", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sra $rn,$rm */
  {
    MEP_INSN_SRA, "sra", "sra", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* srl $rn,$rm */
  {
    MEP_INSN_SRL, "srl", "srl", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sll $rn,$rm */
  {
    MEP_INSN_SLL, "sll", "sll", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sra $rn,$uimm5 */
  {
    MEP_INSN_SRAI, "srai", "sra", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* srl $rn,$uimm5 */
  {
    MEP_INSN_SRLI, "srli", "srl", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sll $rn,$uimm5 */
  {
    MEP_INSN_SLLI, "slli", "sll", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sll3 \$0,$rn,$uimm5 */
  {
    MEP_INSN_SLL3, "sll3", "sll3", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* fsft $rn,$rm */
  {
    MEP_INSN_FSFT, "fsft", "fsft", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bra $pcrel12a2 */
  {
    MEP_INSN_BRA, "bra", "bra", 16,
    { 0|A(RELAXABLE)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* beqz $rn,$pcrel8a2 */
  {
    MEP_INSN_BEQZ, "beqz", "beqz", 16,
    { 0|A(RELAXABLE)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bnez $rn,$pcrel8a2 */
  {
    MEP_INSN_BNEZ, "bnez", "bnez", 16,
    { 0|A(RELAXABLE)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* beqi $rn,$uimm4,$pcrel17a2 */
  {
    MEP_INSN_BEQI, "beqi", "beqi", 32,
    { 0|A(RELAXABLE)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bnei $rn,$uimm4,$pcrel17a2 */
  {
    MEP_INSN_BNEI, "bnei", "bnei", 32,
    { 0|A(RELAXABLE)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* blti $rn,$uimm4,$pcrel17a2 */
  {
    MEP_INSN_BLTI, "blti", "blti", 32,
    { 0|A(RELAXABLE)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bgei $rn,$uimm4,$pcrel17a2 */
  {
    MEP_INSN_BGEI, "bgei", "bgei", 32,
    { 0|A(RELAXABLE)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* beq $rn,$rm,$pcrel17a2 */
  {
    MEP_INSN_BEQ, "beq", "beq", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bne $rn,$rm,$pcrel17a2 */
  {
    MEP_INSN_BNE, "bne", "bne", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bsr $pcrel12a2 */
  {
    MEP_INSN_BSR12, "bsr12", "bsr", 16,
    { 0|A(RELAXABLE)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bsr $pcrel24a2 */
  {
    MEP_INSN_BSR24, "bsr24", "bsr", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* jmp $rm */
  {
    MEP_INSN_JMP, "jmp", "jmp", 16,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* jmp $pcabs24a2 */
  {
    MEP_INSN_JMP24, "jmp24", "jmp", 32,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* jsr $rm */
  {
    MEP_INSN_JSR, "jsr", "jsr", 16,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ret */
  {
    MEP_INSN_RET, "ret", "ret", 16,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* repeat $rn,$pcrel17a2 */
  {
    MEP_INSN_REPEAT, "repeat", "repeat", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* erepeat $pcrel17a2 */
  {
    MEP_INSN_EREPEAT, "erepeat", "erepeat", 32,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* stc $rn,\$lp */
  {
    MEP_INSN_STC_LP, "stc_lp", "stc", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* stc $rn,\$hi */
  {
    MEP_INSN_STC_HI, "stc_hi", "stc", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* stc $rn,\$lo */
  {
    MEP_INSN_STC_LO, "stc_lo", "stc", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* stc $rn,$csrn */
  {
    MEP_INSN_STC, "stc", "stc", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ldc $rn,\$lp */
  {
    MEP_INSN_LDC_LP, "ldc_lp", "ldc", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ldc $rn,\$hi */
  {
    MEP_INSN_LDC_HI, "ldc_hi", "ldc", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ldc $rn,\$lo */
  {
    MEP_INSN_LDC_LO, "ldc_lo", "ldc", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ldc $rn,$csrn */
  {
    MEP_INSN_LDC, "ldc", "ldc", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 2, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* di */
  {
    MEP_INSN_DI, "di", "di", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ei */
  {
    MEP_INSN_EI, "ei", "ei", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* reti */
  {
    MEP_INSN_RETI, "reti", "reti", 16,
    { 0|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* halt */
  {
    MEP_INSN_HALT, "halt", "halt", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sleep */
  {
    MEP_INSN_SLEEP, "sleep", "sleep", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* swi $uimm2 */
  {
    MEP_INSN_SWI, "swi", "swi", 16,
    { 0|A(VOLATILE)|A(MAY_TRAP), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* break */
  {
    MEP_INSN_BREAK, "break", "break", 16,
    { 0|A(VOLATILE)|A(MAY_TRAP)|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* syncm */
  {
    MEP_INSN_SYNCM, "syncm", "syncm", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* stcb $rn,$uimm16 */
  {
    MEP_INSN_STCB, "stcb", "stcb", 32,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ldcb $rn,$uimm16 */
  {
    MEP_INSN_LDCB, "ldcb", "ldcb", 32,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 3, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bsetm ($rma),$uimm3 */
  {
    MEP_INSN_BSETM, "bsetm", "bsetm", 16,
    { 0|A(OPTIONAL_BIT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bclrm ($rma),$uimm3 */
  {
    MEP_INSN_BCLRM, "bclrm", "bclrm", 16,
    { 0|A(OPTIONAL_BIT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bnotm ($rma),$uimm3 */
  {
    MEP_INSN_BNOTM, "bnotm", "bnotm", 16,
    { 0|A(OPTIONAL_BIT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* btstm \$0,($rma),$uimm3 */
  {
    MEP_INSN_BTSTM, "btstm", "btstm", 16,
    { 0|A(OPTIONAL_BIT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* tas $rn,($rma) */
  {
    MEP_INSN_TAS, "tas", "tas", 16,
    { 0|A(OPTIONAL_BIT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* cache $cimm4,($rma) */
  {
    MEP_INSN_CACHE, "cache", "cache", 16,
    { 0|A(VOLATILE), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* mul $rn,$rm */
  {
    MEP_INSN_MUL, "mul", "mul", 16,
    { 0|A(OPTIONAL_MUL_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* mulu $rn,$rm */
  {
    MEP_INSN_MULU, "mulu", "mulu", 16,
    { 0|A(OPTIONAL_MUL_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* mulr $rn,$rm */
  {
    MEP_INSN_MULR, "mulr", "mulr", 16,
    { 0|A(OPTIONAL_MUL_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 3, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* mulru $rn,$rm */
  {
    MEP_INSN_MULRU, "mulru", "mulru", 16,
    { 0|A(OPTIONAL_MUL_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 3, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* madd $rn,$rm */
  {
    MEP_INSN_MADD, "madd", "madd", 32,
    { 0|A(OPTIONAL_MUL_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* maddu $rn,$rm */
  {
    MEP_INSN_MADDU, "maddu", "maddu", 32,
    { 0|A(OPTIONAL_MUL_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* maddr $rn,$rm */
  {
    MEP_INSN_MADDR, "maddr", "maddr", 32,
    { 0|A(OPTIONAL_MUL_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 3, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* maddru $rn,$rm */
  {
    MEP_INSN_MADDRU, "maddru", "maddru", 32,
    { 0|A(OPTIONAL_MUL_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 3, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* div $rn,$rm */
  {
    MEP_INSN_DIV, "div", "div", 16,
    { 0|A(MAY_TRAP)|A(OPTIONAL_DIV_INSN)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 34, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* divu $rn,$rm */
  {
    MEP_INSN_DIVU, "divu", "divu", 16,
    { 0|A(MAY_TRAP)|A(OPTIONAL_DIV_INSN)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 34, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* dret */
  {
    MEP_INSN_DRET, "dret", "dret", 16,
    { 0|A(OPTIONAL_DEBUG_INSN)|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* dbreak */
  {
    MEP_INSN_DBREAK, "dbreak", "dbreak", 16,
    { 0|A(VOLATILE)|A(MAY_TRAP)|A(OPTIONAL_DEBUG_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ldz $rn,$rm */
  {
    MEP_INSN_LDZ, "ldz", "ldz", 32,
    { 0|A(OPTIONAL_LDZ_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* abs $rn,$rm */
  {
    MEP_INSN_ABS, "abs", "abs", 32,
    { 0|A(OPTIONAL_ABS_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ave $rn,$rm */
  {
    MEP_INSN_AVE, "ave", "ave", 32,
    { 0|A(OPTIONAL_AVE_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* min $rn,$rm */
  {
    MEP_INSN_MIN, "min", "min", 32,
    { 0|A(OPTIONAL_MINMAX_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* max $rn,$rm */
  {
    MEP_INSN_MAX, "max", "max", 32,
    { 0|A(OPTIONAL_MINMAX_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* minu $rn,$rm */
  {
    MEP_INSN_MINU, "minu", "minu", 32,
    { 0|A(OPTIONAL_MINMAX_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* maxu $rn,$rm */
  {
    MEP_INSN_MAXU, "maxu", "maxu", 32,
    { 0|A(OPTIONAL_MINMAX_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* clip $rn,$cimm5 */
  {
    MEP_INSN_CLIP, "clip", "clip", 32,
    { 0|A(OPTIONAL_CLIP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* clipu $rn,$cimm5 */
  {
    MEP_INSN_CLIPU, "clipu", "clipu", 32,
    { 0|A(OPTIONAL_CLIP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sadd $rn,$rm */
  {
    MEP_INSN_SADD, "sadd", "sadd", 32,
    { 0|A(OPTIONAL_SAT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ssub $rn,$rm */
  {
    MEP_INSN_SSUB, "ssub", "ssub", 32,
    { 0|A(OPTIONAL_SAT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* saddu $rn,$rm */
  {
    MEP_INSN_SADDU, "saddu", "saddu", 32,
    { 0|A(OPTIONAL_SAT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* ssubu $rn,$rm */
  {
    MEP_INSN_SSUBU, "ssubu", "ssubu", 32,
    { 0|A(OPTIONAL_SAT_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* swcp $crn,($rma) */
  {
    MEP_INSN_SWCP, "swcp", "swcp", 16,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lwcp $crn,($rma) */
  {
    MEP_INSN_LWCP, "lwcp", "lwcp", 16,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* smcp $crn64,($rma) */
  {
    MEP_INSN_SMCP, "smcp", "smcp", 16,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lmcp $crn64,($rma) */
  {
    MEP_INSN_LMCP, "lmcp", "lmcp", 16,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* swcpi $crn,($rma+) */
  {
    MEP_INSN_SWCPI, "swcpi", "swcpi", 16,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lwcpi $crn,($rma+) */
  {
    MEP_INSN_LWCPI, "lwcpi", "lwcpi", 16,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* smcpi $crn64,($rma+) */
  {
    MEP_INSN_SMCPI, "smcpi", "smcpi", 16,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lmcpi $crn64,($rma+) */
  {
    MEP_INSN_LMCPI, "lmcpi", "lmcpi", 16,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* swcp $crn,$sdisp16($rma) */
  {
    MEP_INSN_SWCP16, "swcp16", "swcp", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lwcp $crn,$sdisp16($rma) */
  {
    MEP_INSN_LWCP16, "lwcp16", "lwcp", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* smcp $crn64,$sdisp16($rma) */
  {
    MEP_INSN_SMCP16, "smcp16", "smcp", 32,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lmcp $crn64,$sdisp16($rma) */
  {
    MEP_INSN_LMCP16, "lmcp16", "lmcp", 32,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sbcpa $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_SBCPA, "sbcpa", "sbcpa", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbcpa $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_LBCPA, "lbcpa", "lbcpa", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* shcpa $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_SHCPA, "shcpa", "shcpa", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhcpa $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_LHCPA, "lhcpa", "lhcpa", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* swcpa $crn,($rma+),$cdisp10a4 */
  {
    MEP_INSN_SWCPA, "swcpa", "swcpa", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lwcpa $crn,($rma+),$cdisp10a4 */
  {
    MEP_INSN_LWCPA, "lwcpa", "lwcpa", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* smcpa $crn64,($rma+),$cdisp10a8 */
  {
    MEP_INSN_SMCPA, "smcpa", "smcpa", 32,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lmcpa $crn64,($rma+),$cdisp10a8 */
  {
    MEP_INSN_LMCPA, "lmcpa", "lmcpa", 32,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sbcpm0 $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_SBCPM0, "sbcpm0", "sbcpm0", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbcpm0 $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_LBCPM0, "lbcpm0", "lbcpm0", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* shcpm0 $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_SHCPM0, "shcpm0", "shcpm0", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhcpm0 $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_LHCPM0, "lhcpm0", "lhcpm0", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* swcpm0 $crn,($rma+),$cdisp10a4 */
  {
    MEP_INSN_SWCPM0, "swcpm0", "swcpm0", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lwcpm0 $crn,($rma+),$cdisp10a4 */
  {
    MEP_INSN_LWCPM0, "lwcpm0", "lwcpm0", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* smcpm0 $crn64,($rma+),$cdisp10a8 */
  {
    MEP_INSN_SMCPM0, "smcpm0", "smcpm0", 32,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lmcpm0 $crn64,($rma+),$cdisp10a8 */
  {
    MEP_INSN_LMCPM0, "lmcpm0", "lmcpm0", 32,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sbcpm1 $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_SBCPM1, "sbcpm1", "sbcpm1", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbcpm1 $crn,($rma+),$cdisp10 */
  {
    MEP_INSN_LBCPM1, "lbcpm1", "lbcpm1", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* shcpm1 $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_SHCPM1, "shcpm1", "shcpm1", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhcpm1 $crn,($rma+),$cdisp10a2 */
  {
    MEP_INSN_LHCPM1, "lhcpm1", "lhcpm1", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* swcpm1 $crn,($rma+),$cdisp10a4 */
  {
    MEP_INSN_SWCPM1, "swcpm1", "swcpm1", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lwcpm1 $crn,($rma+),$cdisp10a4 */
  {
    MEP_INSN_LWCPM1, "lwcpm1", "lwcpm1", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* smcpm1 $crn64,($rma+),$cdisp10a8 */
  {
    MEP_INSN_SMCPM1, "smcpm1", "smcpm1", 32,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lmcpm1 $crn64,($rma+),$cdisp10a8 */
  {
    MEP_INSN_LMCPM1, "lmcpm1", "lmcpm1", 32,
    { 0|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bcpeq $cccc,$pcrel17a2 */
  {
    MEP_INSN_BCPEQ, "bcpeq", "bcpeq", 32,
    { 0|A(RELAXABLE)|A(OPTIONAL_CP_INSN)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bcpne $cccc,$pcrel17a2 */
  {
    MEP_INSN_BCPNE, "bcpne", "bcpne", 32,
    { 0|A(RELAXABLE)|A(OPTIONAL_CP_INSN)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bcpat $cccc,$pcrel17a2 */
  {
    MEP_INSN_BCPAT, "bcpat", "bcpat", 32,
    { 0|A(RELAXABLE)|A(OPTIONAL_CP_INSN)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bcpaf $cccc,$pcrel17a2 */
  {
    MEP_INSN_BCPAF, "bcpaf", "bcpaf", 32,
    { 0|A(RELAXABLE)|A(OPTIONAL_CP_INSN)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* synccp */
  {
    MEP_INSN_SYNCCP, "synccp", "synccp", 16,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* jsrv $rm */
  {
    MEP_INSN_JSRV, "jsrv", "jsrv", 16,
    { 0|A(OPTIONAL_CP_INSN)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* bsrv $pcrel24a2 */
  {
    MEP_INSN_BSRV, "bsrv", "bsrv", 32,
    { 0|A(OPTIONAL_CP_INSN)|A(COND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --syscall-- */
  {
    MEP_INSN_SIM_SYSCALL, "sim-syscall", "--syscall--", 16,
    { 0, { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_0, "ri-0", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_1, "ri-1", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_2, "ri-2", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_3, "ri-3", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_4, "ri-4", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_5, "ri-5", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_6, "ri-6", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_7, "ri-7", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_8, "ri-8", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_9, "ri-9", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_10, "ri-10", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_11, "ri-11", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_12, "ri-12", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_13, "ri-13", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_14, "ri-14", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_15, "ri-15", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_17, "ri-17", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_20, "ri-20", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_21, "ri-21", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_22, "ri-22", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_23, "ri-23", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* --reserved-- */
  {
    MEP_INSN_RI_26, "ri-26", "--reserved--", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* cmov $crnx64,$rm */
  {
    MEP_INSN_CMOV_CRN_RM, "cmov-crn-rm", "cmov", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cmov $rm,$crnx64 */
  {
    MEP_INSN_CMOV_RN_CRM, "cmov-rn-crm", "cmov", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cmovc $ivc2c3ccrn,$rm */
  {
    MEP_INSN_CMOVC_CCRN_RM, "cmovc-ccrn-rm", "cmovc", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cmovc $rm,$ivc2c3ccrn */
  {
    MEP_INSN_CMOVC_RN_CCRM, "cmovc-rn-ccrm", "cmovc", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cmovh $crnx64,$rm */
  {
    MEP_INSN_CMOVH_CRN_RM, "cmovh-crn-rm", "cmovh", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cmovh $rm,$crnx64 */
  {
    MEP_INSN_CMOVH_RN_CRM, "cmovh-rn-crm", "cmovh", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cmov $ivc2crn,$ivc2rm */
  {
    MEP_INSN_CMOV_CRN_RM_P0, "cmov-crn-rm-p0", "cmov", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x8" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0), 0 } } } }
  },
/* cmov $ivc2rm,$ivc2crn */
  {
    MEP_INSN_CMOV_RN_CRM_P0, "cmov-rn-crm-p0", "cmov", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x8" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0), 0 } } } }
  },
/* cmovc $ivc2ccrn,$ivc2rm */
  {
    MEP_INSN_CMOVC_CCRN_RM_P0, "cmovc-ccrn-rm-p0", "cmovc", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x8" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0), 0 } } } }
  },
/* cmovc $ivc2rm,$ivc2ccrn */
  {
    MEP_INSN_CMOVC_RN_CCRM_P0, "cmovc-rn-ccrm-p0", "cmovc", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x8" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0), 0 } } } }
  },
/* cmovh $ivc2crn,$ivc2rm */
  {
    MEP_INSN_CMOVH_CRN_RM_P0, "cmovh-crn-rm-p0", "cmovh", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x8" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0), 0 } } } }
  },
/* cmovh $ivc2rm,$ivc2crn */
  {
    MEP_INSN_CMOVH_RN_CRM_P0, "cmovh-rn-crm-p0", "cmovh", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x8" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0), 0 } } } }
  },
/* cpadd3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPADD3_B_C3, "cpadd3_b_C3", "cpadd3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpadd3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPADD3_H_C3, "cpadd3_h_C3", "cpadd3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpadd3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPADD3_W_C3, "cpadd3_w_C3", "cpadd3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdadd3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CDADD3_C3, "cdadd3_C3", "cdadd3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsub3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSUB3_B_C3, "cpsub3_b_C3", "cpsub3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsub3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSUB3_H_C3, "cpsub3_h_C3", "cpsub3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsub3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSUB3_W_C3, "cpsub3_w_C3", "cpsub3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdsub3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CDSUB3_C3, "cdsub3_C3", "cdsub3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpand3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CPAND3_C3, "cpand3_C3", "cpand3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_VECT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpor3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CPOR3_C3, "cpor3_C3", "cpor3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_VECT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpnor3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CPNOR3_C3, "cpnor3_C3", "cpnor3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_VECT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpxor3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CPXOR3_C3, "cpxor3_C3", "cpxor3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_VECT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsel $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSEL_C3, "cpsel_C3", "cpsel", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpfsftbi $croc,$crqc,$crpc,$imm3p4 */
  {
    MEP_INSN_CPFSFTBI_C3, "cpfsftbi_C3", "cpfsftbi", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpfsftbs0 $croc,$crqc,$crpc */
  {
    MEP_INSN_CPFSFTBS0_C3, "cpfsftbs0_C3", "cpfsftbs0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpfsftbs1 $croc,$crqc,$crpc */
  {
    MEP_INSN_CPFSFTBS1_C3, "cpfsftbs1_C3", "cpfsftbs1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpunpacku.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPUNPACKU_B_C3, "cpunpacku_b_C3", "cpunpacku.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpunpacku.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPUNPACKU_H_C3, "cpunpacku_h_C3", "cpunpacku.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpunpacku.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPUNPACKU_W_C3, "cpunpacku_w_C3", "cpunpacku.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpunpackl.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPUNPACKL_B_C3, "cpunpackl_b_C3", "cpunpackl.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpunpackl.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPUNPACKL_H_C3, "cpunpackl_h_C3", "cpunpackl.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpunpackl.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPUNPACKL_W_C3, "cpunpackl_w_C3", "cpunpackl.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppacku.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPPACKU_B_C3, "cppacku_b_C3", "cppacku.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppack.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPPACK_B_C3, "cppack_b_C3", "cppack.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppack.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPPACK_H_C3, "cppack_h_C3", "cppack.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrl3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSRL3_B_C3, "cpsrl3_b_C3", "cpsrl3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssrl3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSRL3_B_C3, "cpssrl3_b_C3", "cpssrl3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrl3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSRL3_H_C3, "cpsrl3_h_C3", "cpsrl3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssrl3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSRL3_H_C3, "cpssrl3_h_C3", "cpssrl3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrl3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSRL3_W_C3, "cpsrl3_w_C3", "cpsrl3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssrl3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSRL3_W_C3, "cpssrl3_w_C3", "cpssrl3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdsrl3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CDSRL3_C3, "cdsrl3_C3", "cdsrl3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsra3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSRA3_B_C3, "cpsra3_b_C3", "cpsra3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssra3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSRA3_B_C3, "cpssra3_b_C3", "cpssra3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsra3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSRA3_H_C3, "cpsra3_h_C3", "cpsra3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssra3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSRA3_H_C3, "cpssra3_h_C3", "cpssra3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsra3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSRA3_W_C3, "cpsra3_w_C3", "cpsra3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssra3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSRA3_W_C3, "cpssra3_w_C3", "cpssra3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdsra3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CDSRA3_C3, "cdsra3_C3", "cdsra3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsll3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSLL3_B_C3, "cpsll3_b_C3", "cpsll3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssll3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSLL3_B_C3, "cpssll3_b_C3", "cpssll3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsll3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSLL3_H_C3, "cpsll3_h_C3", "cpsll3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssll3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSLL3_H_C3, "cpssll3_h_C3", "cpssll3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsll3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSLL3_W_C3, "cpsll3_w_C3", "cpsll3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssll3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSLL3_W_C3, "cpssll3_w_C3", "cpssll3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdsll3 $croc,$crqc,$crpc */
  {
    MEP_INSN_CDSLL3_C3, "cdsll3_C3", "cdsll3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsla3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSLA3_H_C3, "cpsla3_h_C3", "cpsla3.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsla3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSLA3_W_C3, "cpsla3_w_C3", "cpsla3.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsadd3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSADD3_H_C3, "cpsadd3_h_C3", "cpsadd3.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsadd3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSADD3_W_C3, "cpsadd3_w_C3", "cpsadd3.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssub3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSUB3_H_C3, "cpssub3_h_C3", "cpssub3.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssub3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPSSUB3_W_C3, "cpssub3_w_C3", "cpssub3.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextuaddu3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPEXTUADDU3_B_C3, "cpextuaddu3_b_C3", "cpextuaddu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextuadd3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPEXTUADD3_B_C3, "cpextuadd3_b_C3", "cpextuadd3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextladdu3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPEXTLADDU3_B_C3, "cpextladdu3_b_C3", "cpextladdu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextladd3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPEXTLADD3_B_C3, "cpextladd3_b_C3", "cpextladd3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextusubu3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPEXTUSUBU3_B_C3, "cpextusubu3_b_C3", "cpextusubu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextusub3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPEXTUSUB3_B_C3, "cpextusub3_b_C3", "cpextusub3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextlsubu3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPEXTLSUBU3_B_C3, "cpextlsubu3_b_C3", "cpextlsubu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextlsub3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPEXTLSUB3_B_C3, "cpextlsub3_b_C3", "cpextlsub3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaveu3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPAVEU3_B_C3, "cpaveu3_b_C3", "cpaveu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpave3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPAVE3_B_C3, "cpave3_b_C3", "cpave3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpave3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPAVE3_H_C3, "cpave3_h_C3", "cpave3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpave3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPAVE3_W_C3, "cpave3_w_C3", "cpave3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddsru3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPADDSRU3_B_C3, "cpaddsru3_b_C3", "cpaddsru3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddsr3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPADDSR3_B_C3, "cpaddsr3_b_C3", "cpaddsr3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddsr3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPADDSR3_H_C3, "cpaddsr3_h_C3", "cpaddsr3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddsr3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPADDSR3_W_C3, "cpaddsr3_w_C3", "cpaddsr3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabsu3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPABSU3_B_C3, "cpabsu3_b_C3", "cpabsu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabs3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPABS3_B_C3, "cpabs3_b_C3", "cpabs3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabs3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPABS3_H_C3, "cpabs3_h_C3", "cpabs3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmaxu3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMAXU3_B_C3, "cpmaxu3_b_C3", "cpmaxu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmax3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMAX3_B_C3, "cpmax3_b_C3", "cpmax3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmax3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMAX3_H_C3, "cpmax3_h_C3", "cpmax3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmaxu3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMAXU3_W_C3, "cpmaxu3_w_C3", "cpmaxu3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmax3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMAX3_W_C3, "cpmax3_w_C3", "cpmax3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpminu3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMINU3_B_C3, "cpminu3_b_C3", "cpminu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmin3.b $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMIN3_B_C3, "cpmin3_b_C3", "cpmin3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmin3.h $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMIN3_H_C3, "cpmin3_h_C3", "cpmin3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpminu3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMINU3_W_C3, "cpminu3_w_C3", "cpminu3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmin3.w $croc,$crqc,$crpc */
  {
    MEP_INSN_CPMIN3_W_C3, "cpmin3_w_C3", "cpmin3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovfrcsar0 $croc */
  {
    MEP_INSN_CPMOVFRCSAR0_C3, "cpmovfrcsar0_C3", "cpmovfrcsar0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovfrcsar1 $croc */
  {
    MEP_INSN_CPMOVFRCSAR1_C3, "cpmovfrcsar1_C3", "cpmovfrcsar1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovfrcc $croc */
  {
    MEP_INSN_CPMOVFRCC_C3, "cpmovfrcc_C3", "cpmovfrcc", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovtocsar0 $crqc */
  {
    MEP_INSN_CPMOVTOCSAR0_C3, "cpmovtocsar0_C3", "cpmovtocsar0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovtocsar1 $crqc */
  {
    MEP_INSN_CPMOVTOCSAR1_C3, "cpmovtocsar1_C3", "cpmovtocsar1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovtocc $crqc */
  {
    MEP_INSN_CPMOVTOCC_C3, "cpmovtocc_C3", "cpmovtocc", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmov $croc,$crqc */
  {
    MEP_INSN_CPMOV_C3, "cpmov_C3", "cpmov", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabsz.b $croc,$crqc */
  {
    MEP_INSN_CPABSZ_B_C3, "cpabsz_b_C3", "cpabsz.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabsz.h $croc,$crqc */
  {
    MEP_INSN_CPABSZ_H_C3, "cpabsz_h_C3", "cpabsz.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabsz.w $croc,$crqc */
  {
    MEP_INSN_CPABSZ_W_C3, "cpabsz_w_C3", "cpabsz.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpldz.h $croc,$crqc */
  {
    MEP_INSN_CPLDZ_H_C3, "cpldz_h_C3", "cpldz.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpldz.w $croc,$crqc */
  {
    MEP_INSN_CPLDZ_W_C3, "cpldz_w_C3", "cpldz.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpnorm.h $croc,$crqc */
  {
    MEP_INSN_CPNORM_H_C3, "cpnorm_h_C3", "cpnorm.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpnorm.w $croc,$crqc */
  {
    MEP_INSN_CPNORM_W_C3, "cpnorm_w_C3", "cpnorm.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cphaddu.b $croc,$crqc */
  {
    MEP_INSN_CPHADDU_B_C3, "cphaddu_b_C3", "cphaddu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cphadd.b $croc,$crqc */
  {
    MEP_INSN_CPHADD_B_C3, "cphadd_b_C3", "cphadd.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cphadd.h $croc,$crqc */
  {
    MEP_INSN_CPHADD_H_C3, "cphadd_h_C3", "cphadd.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cphadd.w $croc,$crqc */
  {
    MEP_INSN_CPHADD_W_C3, "cphadd_w_C3", "cphadd.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpccadd.b $crqc */
  {
    MEP_INSN_CPCCADD_B_C3, "cpccadd_b_C3", "cpccadd.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRSTCOPY, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpbcast.b $croc,$crqc */
  {
    MEP_INSN_CPBCAST_B_C3, "cpbcast_b_C3", "cpbcast.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpbcast.h $croc,$crqc */
  {
    MEP_INSN_CPBCAST_H_C3, "cpbcast_h_C3", "cpbcast.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpbcast.w $croc,$crqc */
  {
    MEP_INSN_CPBCAST_W_C3, "cpbcast_w_C3", "cpbcast.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextuu.b $croc,$crqc */
  {
    MEP_INSN_CPEXTUU_B_C3, "cpextuu_b_C3", "cpextuu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextu.b $croc,$crqc */
  {
    MEP_INSN_CPEXTU_B_C3, "cpextu_b_C3", "cpextu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextuu.h $croc,$crqc */
  {
    MEP_INSN_CPEXTUU_H_C3, "cpextuu_h_C3", "cpextuu.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextu.h $croc,$crqc */
  {
    MEP_INSN_CPEXTU_H_C3, "cpextu_h_C3", "cpextu.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextlu.b $croc,$crqc */
  {
    MEP_INSN_CPEXTLU_B_C3, "cpextlu_b_C3", "cpextlu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextl.b $croc,$crqc */
  {
    MEP_INSN_CPEXTL_B_C3, "cpextl_b_C3", "cpextl.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextlu.h $croc,$crqc */
  {
    MEP_INSN_CPEXTLU_H_C3, "cpextlu_h_C3", "cpextlu.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpextl.h $croc,$crqc */
  {
    MEP_INSN_CPEXTL_H_C3, "cpextl_h_C3", "cpextl.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcastub.h $croc,$crqc */
  {
    MEP_INSN_CPCASTUB_H_C3, "cpcastub_h_C3", "cpcastub.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcastb.h $croc,$crqc */
  {
    MEP_INSN_CPCASTB_H_C3, "cpcastb_h_C3", "cpcastb.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcastub.w $croc,$crqc */
  {
    MEP_INSN_CPCASTUB_W_C3, "cpcastub_w_C3", "cpcastub.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcastb.w $croc,$crqc */
  {
    MEP_INSN_CPCASTB_W_C3, "cpcastb_w_C3", "cpcastb.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcastuh.w $croc,$crqc */
  {
    MEP_INSN_CPCASTUH_W_C3, "cpcastuh_w_C3", "cpcastuh.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcasth.w $croc,$crqc */
  {
    MEP_INSN_CPCASTH_W_C3, "cpcasth_w_C3", "cpcasth.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdcastuw $croc,$crqc */
  {
    MEP_INSN_CDCASTUW_C3, "cdcastuw_C3", "cdcastuw", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdcastw $croc,$crqc */
  {
    MEP_INSN_CDCASTW_C3, "cdcastw_C3", "cdcastw", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpeqz.b $crqc,$crpc */
  {
    MEP_INSN_CPCMPEQZ_B_C3, "cpcmpeqz_b_C3", "cpcmpeqz.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpeq.b $crqc,$crpc */
  {
    MEP_INSN_CPCMPEQ_B_C3, "cpcmpeq_b_C3", "cpcmpeq.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpeq.h $crqc,$crpc */
  {
    MEP_INSN_CPCMPEQ_H_C3, "cpcmpeq_h_C3", "cpcmpeq.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpeq.w $crqc,$crpc */
  {
    MEP_INSN_CPCMPEQ_W_C3, "cpcmpeq_w_C3", "cpcmpeq.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpne.b $crqc,$crpc */
  {
    MEP_INSN_CPCMPNE_B_C3, "cpcmpne_b_C3", "cpcmpne.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpne.h $crqc,$crpc */
  {
    MEP_INSN_CPCMPNE_H_C3, "cpcmpne_h_C3", "cpcmpne.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpne.w $crqc,$crpc */
  {
    MEP_INSN_CPCMPNE_W_C3, "cpcmpne_w_C3", "cpcmpne.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpgtu.b $crqc,$crpc */
  {
    MEP_INSN_CPCMPGTU_B_C3, "cpcmpgtu_b_C3", "cpcmpgtu.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpgt.b $crqc,$crpc */
  {
    MEP_INSN_CPCMPGT_B_C3, "cpcmpgt_b_C3", "cpcmpgt.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpgt.h $crqc,$crpc */
  {
    MEP_INSN_CPCMPGT_H_C3, "cpcmpgt_h_C3", "cpcmpgt.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpgtu.w $crqc,$crpc */
  {
    MEP_INSN_CPCMPGTU_W_C3, "cpcmpgtu_w_C3", "cpcmpgtu.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpgt.w $crqc,$crpc */
  {
    MEP_INSN_CPCMPGT_W_C3, "cpcmpgt_w_C3", "cpcmpgt.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpgeu.b $crqc,$crpc */
  {
    MEP_INSN_CPCMPGEU_B_C3, "cpcmpgeu_b_C3", "cpcmpgeu.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpge.b $crqc,$crpc */
  {
    MEP_INSN_CPCMPGE_B_C3, "cpcmpge_b_C3", "cpcmpge.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpge.h $crqc,$crpc */
  {
    MEP_INSN_CPCMPGE_H_C3, "cpcmpge_h_C3", "cpcmpge.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpgeu.w $crqc,$crpc */
  {
    MEP_INSN_CPCMPGEU_W_C3, "cpcmpgeu_w_C3", "cpcmpgeu.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpcmpge.w $crqc,$crpc */
  {
    MEP_INSN_CPCMPGE_W_C3, "cpcmpge_w_C3", "cpcmpge.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpeq.b $crqc,$crpc */
  {
    MEP_INSN_CPACMPEQ_B_C3, "cpacmpeq_b_C3", "cpacmpeq.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpeq.h $crqc,$crpc */
  {
    MEP_INSN_CPACMPEQ_H_C3, "cpacmpeq_h_C3", "cpacmpeq.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpeq.w $crqc,$crpc */
  {
    MEP_INSN_CPACMPEQ_W_C3, "cpacmpeq_w_C3", "cpacmpeq.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpne.b $crqc,$crpc */
  {
    MEP_INSN_CPACMPNE_B_C3, "cpacmpne_b_C3", "cpacmpne.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpne.h $crqc,$crpc */
  {
    MEP_INSN_CPACMPNE_H_C3, "cpacmpne_h_C3", "cpacmpne.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpne.w $crqc,$crpc */
  {
    MEP_INSN_CPACMPNE_W_C3, "cpacmpne_w_C3", "cpacmpne.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpgtu.b $crqc,$crpc */
  {
    MEP_INSN_CPACMPGTU_B_C3, "cpacmpgtu_b_C3", "cpacmpgtu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpgt.b $crqc,$crpc */
  {
    MEP_INSN_CPACMPGT_B_C3, "cpacmpgt_b_C3", "cpacmpgt.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpgt.h $crqc,$crpc */
  {
    MEP_INSN_CPACMPGT_H_C3, "cpacmpgt_h_C3", "cpacmpgt.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpgtu.w $crqc,$crpc */
  {
    MEP_INSN_CPACMPGTU_W_C3, "cpacmpgtu_w_C3", "cpacmpgtu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpgt.w $crqc,$crpc */
  {
    MEP_INSN_CPACMPGT_W_C3, "cpacmpgt_w_C3", "cpacmpgt.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpgeu.b $crqc,$crpc */
  {
    MEP_INSN_CPACMPGEU_B_C3, "cpacmpgeu_b_C3", "cpacmpgeu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpge.b $crqc,$crpc */
  {
    MEP_INSN_CPACMPGE_B_C3, "cpacmpge_b_C3", "cpacmpge.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpge.h $crqc,$crpc */
  {
    MEP_INSN_CPACMPGE_H_C3, "cpacmpge_h_C3", "cpacmpge.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpgeu.w $crqc,$crpc */
  {
    MEP_INSN_CPACMPGEU_W_C3, "cpacmpgeu_w_C3", "cpacmpgeu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpacmpge.w $crqc,$crpc */
  {
    MEP_INSN_CPACMPGE_W_C3, "cpacmpge_w_C3", "cpacmpge.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpeq.b $crqc,$crpc */
  {
    MEP_INSN_CPOCMPEQ_B_C3, "cpocmpeq_b_C3", "cpocmpeq.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpeq.h $crqc,$crpc */
  {
    MEP_INSN_CPOCMPEQ_H_C3, "cpocmpeq_h_C3", "cpocmpeq.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpeq.w $crqc,$crpc */
  {
    MEP_INSN_CPOCMPEQ_W_C3, "cpocmpeq_w_C3", "cpocmpeq.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpne.b $crqc,$crpc */
  {
    MEP_INSN_CPOCMPNE_B_C3, "cpocmpne_b_C3", "cpocmpne.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpne.h $crqc,$crpc */
  {
    MEP_INSN_CPOCMPNE_H_C3, "cpocmpne_h_C3", "cpocmpne.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpne.w $crqc,$crpc */
  {
    MEP_INSN_CPOCMPNE_W_C3, "cpocmpne_w_C3", "cpocmpne.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpgtu.b $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGTU_B_C3, "cpocmpgtu_b_C3", "cpocmpgtu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpgt.b $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGT_B_C3, "cpocmpgt_b_C3", "cpocmpgt.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpgt.h $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGT_H_C3, "cpocmpgt_h_C3", "cpocmpgt.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpgtu.w $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGTU_W_C3, "cpocmpgtu_w_C3", "cpocmpgtu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpgt.w $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGT_W_C3, "cpocmpgt_w_C3", "cpocmpgt.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpgeu.b $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGEU_B_C3, "cpocmpgeu_b_C3", "cpocmpgeu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpge.b $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGE_B_C3, "cpocmpge_b_C3", "cpocmpge.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpge.h $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGE_H_C3, "cpocmpge_h_C3", "cpocmpge.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpgeu.w $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGEU_W_C3, "cpocmpgeu_w_C3", "cpocmpgeu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpocmpge.w $crqc,$crpc */
  {
    MEP_INSN_CPOCMPGE_W_C3, "cpocmpge_w_C3", "cpocmpge.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrli3.b $crqc,$crpc,$imm3p9 */
  {
    MEP_INSN_CPSRLI3_B_C3, "cpsrli3_b_C3", "cpsrli3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrli3.h $crqc,$crpc,$imm4p8 */
  {
    MEP_INSN_CPSRLI3_H_C3, "cpsrli3_h_C3", "cpsrli3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrli3.w $crqc,$crpc,$imm5p7 */
  {
    MEP_INSN_CPSRLI3_W_C3, "cpsrli3_w_C3", "cpsrli3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdsrli3 $crqc,$crpc,$imm6p6 */
  {
    MEP_INSN_CDSRLI3_C3, "cdsrli3_C3", "cdsrli3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrai3.b $crqc,$crpc,$imm3p9 */
  {
    MEP_INSN_CPSRAI3_B_C3, "cpsrai3_b_C3", "cpsrai3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrai3.h $crqc,$crpc,$imm4p8 */
  {
    MEP_INSN_CPSRAI3_H_C3, "cpsrai3_h_C3", "cpsrai3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrai3.w $crqc,$crpc,$imm5p7 */
  {
    MEP_INSN_CPSRAI3_W_C3, "cpsrai3_w_C3", "cpsrai3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdsrai3 $crqc,$crpc,$imm6p6 */
  {
    MEP_INSN_CDSRAI3_C3, "cdsrai3_C3", "cdsrai3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpslli3.b $crqc,$crpc,$imm3p9 */
  {
    MEP_INSN_CPSLLI3_B_C3, "cpslli3_b_C3", "cpslli3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpslli3.h $crqc,$crpc,$imm4p8 */
  {
    MEP_INSN_CPSLLI3_H_C3, "cpslli3_h_C3", "cpslli3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpslli3.w $crqc,$crpc,$imm5p7 */
  {
    MEP_INSN_CPSLLI3_W_C3, "cpslli3_w_C3", "cpslli3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdslli3 $crqc,$crpc,$imm6p6 */
  {
    MEP_INSN_CDSLLI3_C3, "cdslli3_C3", "cdslli3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpslai3.h $crqc,$crpc,$imm4p8 */
  {
    MEP_INSN_CPSLAI3_H_C3, "cpslai3_h_C3", "cpslai3.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpslai3.w $crqc,$crpc,$imm5p7 */
  {
    MEP_INSN_CPSLAI3_W_C3, "cpslai3_w_C3", "cpslai3.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpclipiu3.w $crqc,$crpc,$imm5p7 */
  {
    MEP_INSN_CPCLIPIU3_W_C3, "cpclipiu3_w_C3", "cpclipiu3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpclipi3.w $crqc,$crpc,$imm5p7 */
  {
    MEP_INSN_CPCLIPI3_W_C3, "cpclipi3_w_C3", "cpclipi3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdclipiu3 $crqc,$crpc,$imm6p6 */
  {
    MEP_INSN_CDCLIPIU3_C3, "cdclipiu3_C3", "cdclipiu3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdclipi3 $crqc,$crpc,$imm6p6 */
  {
    MEP_INSN_CDCLIPI3_C3, "cdclipi3_C3", "cdclipi3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovi.b $crqc,$simm8p4 */
  {
    MEP_INSN_CPMOVI_B_C3, "cpmovi_b_C3", "cpmovi.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmoviu.h $crqc,$imm8p4 */
  {
    MEP_INSN_CPMOVIU_H_C3, "cpmoviu_h_C3", "cpmoviu.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovi.h $crqc,$simm8p4 */
  {
    MEP_INSN_CPMOVI_H_C3, "cpmovi_h_C3", "cpmovi.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmoviu.w $crqc,$imm8p4 */
  {
    MEP_INSN_CPMOVIU_W_C3, "cpmoviu_w_C3", "cpmoviu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovi.w $crqc,$simm8p4 */
  {
    MEP_INSN_CPMOVI_W_C3, "cpmovi_w_C3", "cpmovi.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdmoviu $crqc,$imm8p4 */
  {
    MEP_INSN_CDMOVIU_C3, "cdmoviu_C3", "cdmoviu", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cdmovi $crqc,$simm8p4 */
  {
    MEP_INSN_CDMOVI_C3, "cdmovi_C3", "cdmovi", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpadda1u.b $crqc,$crpc */
  {
    MEP_INSN_CPADDA1U_B_C3, "cpadda1u_b_C3", "cpadda1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpadda1.b $crqc,$crpc */
  {
    MEP_INSN_CPADDA1_B_C3, "cpadda1_b_C3", "cpadda1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddua1.h $crqc,$crpc */
  {
    MEP_INSN_CPADDUA1_H_C3, "cpaddua1_h_C3", "cpaddua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddla1.h $crqc,$crpc */
  {
    MEP_INSN_CPADDLA1_H_C3, "cpaddla1_h_C3", "cpaddla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddaca1u.b $crqc,$crpc */
  {
    MEP_INSN_CPADDACA1U_B_C3, "cpaddaca1u_b_C3", "cpaddaca1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddaca1.b $crqc,$crpc */
  {
    MEP_INSN_CPADDACA1_B_C3, "cpaddaca1_b_C3", "cpaddaca1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddacua1.h $crqc,$crpc */
  {
    MEP_INSN_CPADDACUA1_H_C3, "cpaddacua1_h_C3", "cpaddacua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpaddacla1.h $crqc,$crpc */
  {
    MEP_INSN_CPADDACLA1_H_C3, "cpaddacla1_h_C3", "cpaddacla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsuba1u.b $crqc,$crpc */
  {
    MEP_INSN_CPSUBA1U_B_C3, "cpsuba1u_b_C3", "cpsuba1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsuba1.b $crqc,$crpc */
  {
    MEP_INSN_CPSUBA1_B_C3, "cpsuba1_b_C3", "cpsuba1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsubua1.h $crqc,$crpc */
  {
    MEP_INSN_CPSUBUA1_H_C3, "cpsubua1_h_C3", "cpsubua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsubla1.h $crqc,$crpc */
  {
    MEP_INSN_CPSUBLA1_H_C3, "cpsubla1_h_C3", "cpsubla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsubaca1u.b $crqc,$crpc */
  {
    MEP_INSN_CPSUBACA1U_B_C3, "cpsubaca1u_b_C3", "cpsubaca1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsubaca1.b $crqc,$crpc */
  {
    MEP_INSN_CPSUBACA1_B_C3, "cpsubaca1_b_C3", "cpsubaca1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsubacua1.h $crqc,$crpc */
  {
    MEP_INSN_CPSUBACUA1_H_C3, "cpsubacua1_h_C3", "cpsubacua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsubacla1.h $crqc,$crpc */
  {
    MEP_INSN_CPSUBACLA1_H_C3, "cpsubacla1_h_C3", "cpsubacla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabsa1u.b $crqc,$crpc */
  {
    MEP_INSN_CPABSA1U_B_C3, "cpabsa1u_b_C3", "cpabsa1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabsa1.b $crqc,$crpc */
  {
    MEP_INSN_CPABSA1_B_C3, "cpabsa1_b_C3", "cpabsa1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabsua1.h $crqc,$crpc */
  {
    MEP_INSN_CPABSUA1_H_C3, "cpabsua1_h_C3", "cpabsua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpabsla1.h $crqc,$crpc */
  {
    MEP_INSN_CPABSLA1_H_C3, "cpabsla1_h_C3", "cpabsla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsada1u.b $crqc,$crpc */
  {
    MEP_INSN_CPSADA1U_B_C3, "cpsada1u_b_C3", "cpsada1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsada1.b $crqc,$crpc */
  {
    MEP_INSN_CPSADA1_B_C3, "cpsada1_b_C3", "cpsada1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsadua1.h $crqc,$crpc */
  {
    MEP_INSN_CPSADUA1_H_C3, "cpsadua1_h_C3", "cpsadua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsadla1.h $crqc,$crpc */
  {
    MEP_INSN_CPSADLA1_H_C3, "cpsadla1_h_C3", "cpsadla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpseta1.h $crqc,$crpc */
  {
    MEP_INSN_CPSETA1_H_C3, "cpseta1_h_C3", "cpseta1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsetua1.w $crqc,$crpc */
  {
    MEP_INSN_CPSETUA1_W_C3, "cpsetua1_w_C3", "cpsetua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsetla1.w $crqc,$crpc */
  {
    MEP_INSN_CPSETLA1_W_C3, "cpsetla1_w_C3", "cpsetla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmova1.b $croc */
  {
    MEP_INSN_CPMOVA1_B_C3, "cpmova1_b_C3", "cpmova1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovua1.h $croc */
  {
    MEP_INSN_CPMOVUA1_H_C3, "cpmovua1_h_C3", "cpmovua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovla1.h $croc */
  {
    MEP_INSN_CPMOVLA1_H_C3, "cpmovla1_h_C3", "cpmovla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovuua1.w $croc */
  {
    MEP_INSN_CPMOVUUA1_W_C3, "cpmovuua1_w_C3", "cpmovuua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovula1.w $croc */
  {
    MEP_INSN_CPMOVULA1_W_C3, "cpmovula1_w_C3", "cpmovula1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovlua1.w $croc */
  {
    MEP_INSN_CPMOVLUA1_W_C3, "cpmovlua1_w_C3", "cpmovlua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovlla1.w $croc */
  {
    MEP_INSN_CPMOVLLA1_W_C3, "cpmovlla1_w_C3", "cpmovlla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppacka1u.b $croc */
  {
    MEP_INSN_CPPACKA1U_B_C3, "cppacka1u_b_C3", "cppacka1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppacka1.b $croc */
  {
    MEP_INSN_CPPACKA1_B_C3, "cppacka1_b_C3", "cppacka1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppackua1.h $croc */
  {
    MEP_INSN_CPPACKUA1_H_C3, "cppackua1_h_C3", "cppackua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppackla1.h $croc */
  {
    MEP_INSN_CPPACKLA1_H_C3, "cppackla1_h_C3", "cppackla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppackua1.w $croc */
  {
    MEP_INSN_CPPACKUA1_W_C3, "cppackua1_w_C3", "cppackua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cppackla1.w $croc */
  {
    MEP_INSN_CPPACKLA1_W_C3, "cppackla1_w_C3", "cppackla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovhua1.w $croc */
  {
    MEP_INSN_CPMOVHUA1_W_C3, "cpmovhua1_w_C3", "cpmovhua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmovhla1.w $croc */
  {
    MEP_INSN_CPMOVHLA1_W_C3, "cpmovhla1_w_C3", "cpmovhla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrla1 $crqc */
  {
    MEP_INSN_CPSRLA1_C3, "cpsrla1_C3", "cpsrla1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsraa1 $crqc */
  {
    MEP_INSN_CPSRAA1_C3, "cpsraa1_C3", "cpsraa1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpslla1 $crqc */
  {
    MEP_INSN_CPSLLA1_C3, "cpslla1_C3", "cpslla1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsrlia1 $imm5p7 */
  {
    MEP_INSN_CPSRLIA1_P1, "cpsrlia1_P1", "cpsrlia1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsraia1 $imm5p7 */
  {
    MEP_INSN_CPSRAIA1_P1, "cpsraia1_P1", "cpsraia1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsllia1 $imm5p7 */
  {
    MEP_INSN_CPSLLIA1_P1, "cpsllia1_P1", "cpsllia1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssqa1u.b $crqc,$crpc */
  {
    MEP_INSN_CPSSQA1U_B_C3, "cpssqa1u_b_C3", "cpssqa1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssqa1.b $crqc,$crpc */
  {
    MEP_INSN_CPSSQA1_B_C3, "cpssqa1_b_C3", "cpssqa1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssda1u.b $crqc,$crpc */
  {
    MEP_INSN_CPSSDA1U_B_C3, "cpssda1u_b_C3", "cpssda1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpssda1.b $crqc,$crpc */
  {
    MEP_INSN_CPSSDA1_B_C3, "cpssda1_b_C3", "cpssda1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmula1u.b $crqc,$crpc */
  {
    MEP_INSN_CPMULA1U_B_C3, "cpmula1u_b_C3", "cpmula1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmula1.b $crqc,$crpc */
  {
    MEP_INSN_CPMULA1_B_C3, "cpmula1_b_C3", "cpmula1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulua1.h $crqc,$crpc */
  {
    MEP_INSN_CPMULUA1_H_C3, "cpmulua1_h_C3", "cpmulua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulla1.h $crqc,$crpc */
  {
    MEP_INSN_CPMULLA1_H_C3, "cpmulla1_h_C3", "cpmulla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulua1u.w $crqc,$crpc */
  {
    MEP_INSN_CPMULUA1U_W_C3, "cpmulua1u_w_C3", "cpmulua1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulla1u.w $crqc,$crpc */
  {
    MEP_INSN_CPMULLA1U_W_C3, "cpmulla1u_w_C3", "cpmulla1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulua1.w $crqc,$crpc */
  {
    MEP_INSN_CPMULUA1_W_C3, "cpmulua1_w_C3", "cpmulua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulla1.w $crqc,$crpc */
  {
    MEP_INSN_CPMULLA1_W_C3, "cpmulla1_w_C3", "cpmulla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmada1u.b $crqc,$crpc */
  {
    MEP_INSN_CPMADA1U_B_C3, "cpmada1u_b_C3", "cpmada1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmada1.b $crqc,$crpc */
  {
    MEP_INSN_CPMADA1_B_C3, "cpmada1_b_C3", "cpmada1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmadua1.h $crqc,$crpc */
  {
    MEP_INSN_CPMADUA1_H_C3, "cpmadua1_h_C3", "cpmadua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmadla1.h $crqc,$crpc */
  {
    MEP_INSN_CPMADLA1_H_C3, "cpmadla1_h_C3", "cpmadla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmadua1u.w $crqc,$crpc */
  {
    MEP_INSN_CPMADUA1U_W_C3, "cpmadua1u_w_C3", "cpmadua1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmadla1u.w $crqc,$crpc */
  {
    MEP_INSN_CPMADLA1U_W_C3, "cpmadla1u_w_C3", "cpmadla1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmadua1.w $crqc,$crpc */
  {
    MEP_INSN_CPMADUA1_W_C3, "cpmadua1_w_C3", "cpmadua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmadla1.w $crqc,$crpc */
  {
    MEP_INSN_CPMADLA1_W_C3, "cpmadla1_w_C3", "cpmadla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmsbua1.h $crqc,$crpc */
  {
    MEP_INSN_CPMSBUA1_H_C3, "cpmsbua1_h_C3", "cpmsbua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmsbla1.h $crqc,$crpc */
  {
    MEP_INSN_CPMSBLA1_H_C3, "cpmsbla1_h_C3", "cpmsbla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmsbua1u.w $crqc,$crpc */
  {
    MEP_INSN_CPMSBUA1U_W_C3, "cpmsbua1u_w_C3", "cpmsbua1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmsbla1u.w $crqc,$crpc */
  {
    MEP_INSN_CPMSBLA1U_W_C3, "cpmsbla1u_w_C3", "cpmsbla1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmsbua1.w $crqc,$crpc */
  {
    MEP_INSN_CPMSBUA1_W_C3, "cpmsbua1_w_C3", "cpmsbua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmsbla1.w $crqc,$crpc */
  {
    MEP_INSN_CPMSBLA1_W_C3, "cpmsbla1_w_C3", "cpmsbla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmadua1.h $crqc,$crpc */
  {
    MEP_INSN_CPSMADUA1_H_C3, "cpsmadua1_h_C3", "cpsmadua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmadla1.h $crqc,$crpc */
  {
    MEP_INSN_CPSMADLA1_H_C3, "cpsmadla1_h_C3", "cpsmadla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmadua1.w $crqc,$crpc */
  {
    MEP_INSN_CPSMADUA1_W_C3, "cpsmadua1_w_C3", "cpsmadua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmadla1.w $crqc,$crpc */
  {
    MEP_INSN_CPSMADLA1_W_C3, "cpsmadla1_w_C3", "cpsmadla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmsbua1.h $crqc,$crpc */
  {
    MEP_INSN_CPSMSBUA1_H_C3, "cpsmsbua1_h_C3", "cpsmsbua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmsbla1.h $crqc,$crpc */
  {
    MEP_INSN_CPSMSBLA1_H_C3, "cpsmsbla1_h_C3", "cpsmsbla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmsbua1.w $crqc,$crpc */
  {
    MEP_INSN_CPSMSBUA1_W_C3, "cpsmsbua1_w_C3", "cpsmsbua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmsbla1.w $crqc,$crpc */
  {
    MEP_INSN_CPSMSBLA1_W_C3, "cpsmsbla1_w_C3", "cpsmsbla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulslua1.h $crqc,$crpc */
  {
    MEP_INSN_CPMULSLUA1_H_C3, "cpmulslua1_h_C3", "cpmulslua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulslla1.h $crqc,$crpc */
  {
    MEP_INSN_CPMULSLLA1_H_C3, "cpmulslla1_h_C3", "cpmulslla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulslua1.w $crqc,$crpc */
  {
    MEP_INSN_CPMULSLUA1_W_C3, "cpmulslua1_w_C3", "cpmulslua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpmulslla1.w $crqc,$crpc */
  {
    MEP_INSN_CPMULSLLA1_W_C3, "cpmulslla1_w_C3", "cpmulslla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmadslua1.h $crqc,$crpc */
  {
    MEP_INSN_CPSMADSLUA1_H_C3, "cpsmadslua1_h_C3", "cpsmadslua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmadslla1.h $crqc,$crpc */
  {
    MEP_INSN_CPSMADSLLA1_H_C3, "cpsmadslla1_h_C3", "cpsmadslla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmadslua1.w $crqc,$crpc */
  {
    MEP_INSN_CPSMADSLUA1_W_C3, "cpsmadslua1_w_C3", "cpsmadslua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmadslla1.w $crqc,$crpc */
  {
    MEP_INSN_CPSMADSLLA1_W_C3, "cpsmadslla1_w_C3", "cpsmadslla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmsbslua1.h $crqc,$crpc */
  {
    MEP_INSN_CPSMSBSLUA1_H_C3, "cpsmsbslua1_h_C3", "cpsmsbslua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmsbslla1.h $crqc,$crpc */
  {
    MEP_INSN_CPSMSBSLLA1_H_C3, "cpsmsbslla1_h_C3", "cpsmsbslla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmsbslua1.w $crqc,$crpc */
  {
    MEP_INSN_CPSMSBSLUA1_W_C3, "cpsmsbslua1_w_C3", "cpsmsbslua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* cpsmsbslla1.w $crqc,$crpc */
  {
    MEP_INSN_CPSMSBSLLA1_W_C3, "cpsmsbslla1_w_C3", "cpsmsbslla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x10" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_C3), 0 } } } }
  },
/* c0nop */
  {
    MEP_INSN_C0NOP_P0_P0S, "c0nop_P0_P0S", "c0nop", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x28" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P0S), 0 } } } }
  },
/* cpadd3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPADD3_B_P0S_P1, "cpadd3_b_P0S_P1", "cpadd3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpadd3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPADD3_H_P0S_P1, "cpadd3_h_P0S_P1", "cpadd3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpadd3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPADD3_W_P0S_P1, "cpadd3_w_P0S_P1", "cpadd3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpunpacku.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPUNPACKU_B_P0S_P1, "cpunpacku_b_P0S_P1", "cpunpacku.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpunpacku.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPUNPACKU_H_P0S_P1, "cpunpacku_h_P0S_P1", "cpunpacku.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpunpacku.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPUNPACKU_W_P0S_P1, "cpunpacku_w_P0S_P1", "cpunpacku.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpunpackl.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPUNPACKL_B_P0S_P1, "cpunpackl_b_P0S_P1", "cpunpackl.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpunpackl.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPUNPACKL_H_P0S_P1, "cpunpackl_h_P0S_P1", "cpunpackl.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpunpackl.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPUNPACKL_W_P0S_P1, "cpunpackl_w_P0S_P1", "cpunpackl.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsel $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSEL_P0S_P1, "cpsel_P0S_P1", "cpsel", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpfsftbs0 $crop,$crqp,$crpp */
  {
    MEP_INSN_CPFSFTBS0_P0S_P1, "cpfsftbs0_P0S_P1", "cpfsftbs0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpfsftbs1 $crop,$crqp,$crpp */
  {
    MEP_INSN_CPFSFTBS1_P0S_P1, "cpfsftbs1_P0S_P1", "cpfsftbs1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmov $crop,$crqp */
  {
    MEP_INSN_CPMOV_P0S_P1, "cpmov_P0S_P1", "cpmov", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpabsz.b $crop,$crqp */
  {
    MEP_INSN_CPABSZ_B_P0S_P1, "cpabsz_b_P0S_P1", "cpabsz.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpabsz.h $crop,$crqp */
  {
    MEP_INSN_CPABSZ_H_P0S_P1, "cpabsz_h_P0S_P1", "cpabsz.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpabsz.w $crop,$crqp */
  {
    MEP_INSN_CPABSZ_W_P0S_P1, "cpabsz_w_P0S_P1", "cpabsz.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpldz.h $crop,$crqp */
  {
    MEP_INSN_CPLDZ_H_P0S_P1, "cpldz_h_P0S_P1", "cpldz.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpldz.w $crop,$crqp */
  {
    MEP_INSN_CPLDZ_W_P0S_P1, "cpldz_w_P0S_P1", "cpldz.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpnorm.h $crop,$crqp */
  {
    MEP_INSN_CPNORM_H_P0S_P1, "cpnorm_h_P0S_P1", "cpnorm.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpnorm.w $crop,$crqp */
  {
    MEP_INSN_CPNORM_W_P0S_P1, "cpnorm_w_P0S_P1", "cpnorm.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cphaddu.b $crop,$crqp */
  {
    MEP_INSN_CPHADDU_B_P0S_P1, "cphaddu_b_P0S_P1", "cphaddu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cphadd.b $crop,$crqp */
  {
    MEP_INSN_CPHADD_B_P0S_P1, "cphadd_b_P0S_P1", "cphadd.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cphadd.h $crop,$crqp */
  {
    MEP_INSN_CPHADD_H_P0S_P1, "cphadd_h_P0S_P1", "cphadd.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cphadd.w $crop,$crqp */
  {
    MEP_INSN_CPHADD_W_P0S_P1, "cphadd_w_P0S_P1", "cphadd.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpccadd.b $crqp */
  {
    MEP_INSN_CPCCADD_B_P0S_P1, "cpccadd_b_P0S_P1", "cpccadd.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRSTCOPY, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpbcast.b $crop,$crqp */
  {
    MEP_INSN_CPBCAST_B_P0S_P1, "cpbcast_b_P0S_P1", "cpbcast.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpbcast.h $crop,$crqp */
  {
    MEP_INSN_CPBCAST_H_P0S_P1, "cpbcast_h_P0S_P1", "cpbcast.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpbcast.w $crop,$crqp */
  {
    MEP_INSN_CPBCAST_W_P0S_P1, "cpbcast_w_P0S_P1", "cpbcast.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextuu.b $crop,$crqp */
  {
    MEP_INSN_CPEXTUU_B_P0S_P1, "cpextuu_b_P0S_P1", "cpextuu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextu.b $crop,$crqp */
  {
    MEP_INSN_CPEXTU_B_P0S_P1, "cpextu_b_P0S_P1", "cpextu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextuu.h $crop,$crqp */
  {
    MEP_INSN_CPEXTUU_H_P0S_P1, "cpextuu_h_P0S_P1", "cpextuu.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextu.h $crop,$crqp */
  {
    MEP_INSN_CPEXTU_H_P0S_P1, "cpextu_h_P0S_P1", "cpextu.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextlu.b $crop,$crqp */
  {
    MEP_INSN_CPEXTLU_B_P0S_P1, "cpextlu_b_P0S_P1", "cpextlu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextl.b $crop,$crqp */
  {
    MEP_INSN_CPEXTL_B_P0S_P1, "cpextl_b_P0S_P1", "cpextl.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextlu.h $crop,$crqp */
  {
    MEP_INSN_CPEXTLU_H_P0S_P1, "cpextlu_h_P0S_P1", "cpextlu.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4UHI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextl.h $crop,$crqp */
  {
    MEP_INSN_CPEXTL_H_P0S_P1, "cpextl_h_P0S_P1", "cpextl.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcastub.h $crop,$crqp */
  {
    MEP_INSN_CPCASTUB_H_P0S_P1, "cpcastub_h_P0S_P1", "cpcastub.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcastb.h $crop,$crqp */
  {
    MEP_INSN_CPCASTB_H_P0S_P1, "cpcastb_h_P0S_P1", "cpcastb.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcastub.w $crop,$crqp */
  {
    MEP_INSN_CPCASTUB_W_P0S_P1, "cpcastub_w_P0S_P1", "cpcastub.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcastb.w $crop,$crqp */
  {
    MEP_INSN_CPCASTB_W_P0S_P1, "cpcastb_w_P0S_P1", "cpcastb.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcastuh.w $crop,$crqp */
  {
    MEP_INSN_CPCASTUH_W_P0S_P1, "cpcastuh_w_P0S_P1", "cpcastuh.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcasth.w $crop,$crqp */
  {
    MEP_INSN_CPCASTH_W_P0S_P1, "cpcasth_w_P0S_P1", "cpcasth.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdcastuw $crop,$crqp */
  {
    MEP_INSN_CDCASTUW_P0S_P1, "cdcastuw_P0S_P1", "cdcastuw", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdcastw $crop,$crqp */
  {
    MEP_INSN_CDCASTW_P0S_P1, "cdcastw_P0S_P1", "cdcastw", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmovfrcsar0 $crop */
  {
    MEP_INSN_CPMOVFRCSAR0_P0S_P1, "cpmovfrcsar0_P0S_P1", "cpmovfrcsar0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmovfrcsar1 $crop */
  {
    MEP_INSN_CPMOVFRCSAR1_P0S_P1, "cpmovfrcsar1_P0S_P1", "cpmovfrcsar1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmovfrcc $crop */
  {
    MEP_INSN_CPMOVFRCC_P0S_P1, "cpmovfrcc_P0S_P1", "cpmovfrcc", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmovtocsar0 $crqp */
  {
    MEP_INSN_CPMOVTOCSAR0_P0S_P1, "cpmovtocsar0_P0S_P1", "cpmovtocsar0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmovtocsar1 $crqp */
  {
    MEP_INSN_CPMOVTOCSAR1_P0S_P1, "cpmovtocsar1_P0S_P1", "cpmovtocsar1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmovtocc $crqp */
  {
    MEP_INSN_CPMOVTOCC_P0S_P1, "cpmovtocc_P0S_P1", "cpmovtocc", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpeqz.b $crqp,$crpp */
  {
    MEP_INSN_CPCMPEQZ_B_P0S_P1, "cpcmpeqz_b_P0S_P1", "cpcmpeqz.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpeq.b $crqp,$crpp */
  {
    MEP_INSN_CPCMPEQ_B_P0S_P1, "cpcmpeq_b_P0S_P1", "cpcmpeq.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpeq.h $crqp,$crpp */
  {
    MEP_INSN_CPCMPEQ_H_P0S_P1, "cpcmpeq_h_P0S_P1", "cpcmpeq.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpeq.w $crqp,$crpp */
  {
    MEP_INSN_CPCMPEQ_W_P0S_P1, "cpcmpeq_w_P0S_P1", "cpcmpeq.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpne.b $crqp,$crpp */
  {
    MEP_INSN_CPCMPNE_B_P0S_P1, "cpcmpne_b_P0S_P1", "cpcmpne.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpne.h $crqp,$crpp */
  {
    MEP_INSN_CPCMPNE_H_P0S_P1, "cpcmpne_h_P0S_P1", "cpcmpne.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpne.w $crqp,$crpp */
  {
    MEP_INSN_CPCMPNE_W_P0S_P1, "cpcmpne_w_P0S_P1", "cpcmpne.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpgtu.b $crqp,$crpp */
  {
    MEP_INSN_CPCMPGTU_B_P0S_P1, "cpcmpgtu_b_P0S_P1", "cpcmpgtu.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpgt.b $crqp,$crpp */
  {
    MEP_INSN_CPCMPGT_B_P0S_P1, "cpcmpgt_b_P0S_P1", "cpcmpgt.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpgt.h $crqp,$crpp */
  {
    MEP_INSN_CPCMPGT_H_P0S_P1, "cpcmpgt_h_P0S_P1", "cpcmpgt.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpgtu.w $crqp,$crpp */
  {
    MEP_INSN_CPCMPGTU_W_P0S_P1, "cpcmpgtu_w_P0S_P1", "cpcmpgtu.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpgt.w $crqp,$crpp */
  {
    MEP_INSN_CPCMPGT_W_P0S_P1, "cpcmpgt_w_P0S_P1", "cpcmpgt.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpgeu.b $crqp,$crpp */
  {
    MEP_INSN_CPCMPGEU_B_P0S_P1, "cpcmpgeu_b_P0S_P1", "cpcmpgeu.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpge.b $crqp,$crpp */
  {
    MEP_INSN_CPCMPGE_B_P0S_P1, "cpcmpge_b_P0S_P1", "cpcmpge.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpge.h $crqp,$crpp */
  {
    MEP_INSN_CPCMPGE_H_P0S_P1, "cpcmpge_h_P0S_P1", "cpcmpge.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpgeu.w $crqp,$crpp */
  {
    MEP_INSN_CPCMPGEU_W_P0S_P1, "cpcmpgeu_w_P0S_P1", "cpcmpgeu.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpcmpge.w $crqp,$crpp */
  {
    MEP_INSN_CPCMPGE_W_P0S_P1, "cpcmpge_w_P0S_P1", "cpcmpge.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpadda0u.b $crqp,$crpp */
  {
    MEP_INSN_CPADDA0U_B_P0S, "cpadda0u_b_P0S", "cpadda0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpadda0.b $crqp,$crpp */
  {
    MEP_INSN_CPADDA0_B_P0S, "cpadda0_b_P0S", "cpadda0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpaddua0.h $crqp,$crpp */
  {
    MEP_INSN_CPADDUA0_H_P0S, "cpaddua0_h_P0S", "cpaddua0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpaddla0.h $crqp,$crpp */
  {
    MEP_INSN_CPADDLA0_H_P0S, "cpaddla0_h_P0S", "cpaddla0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpaddaca0u.b $crqp,$crpp */
  {
    MEP_INSN_CPADDACA0U_B_P0S, "cpaddaca0u_b_P0S", "cpaddaca0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpaddaca0.b $crqp,$crpp */
  {
    MEP_INSN_CPADDACA0_B_P0S, "cpaddaca0_b_P0S", "cpaddaca0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpaddacua0.h $crqp,$crpp */
  {
    MEP_INSN_CPADDACUA0_H_P0S, "cpaddacua0_h_P0S", "cpaddacua0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpaddacla0.h $crqp,$crpp */
  {
    MEP_INSN_CPADDACLA0_H_P0S, "cpaddacla0_h_P0S", "cpaddacla0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsuba0u.b $crqp,$crpp */
  {
    MEP_INSN_CPSUBA0U_B_P0S, "cpsuba0u_b_P0S", "cpsuba0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsuba0.b $crqp,$crpp */
  {
    MEP_INSN_CPSUBA0_B_P0S, "cpsuba0_b_P0S", "cpsuba0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsubua0.h $crqp,$crpp */
  {
    MEP_INSN_CPSUBUA0_H_P0S, "cpsubua0_h_P0S", "cpsubua0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsubla0.h $crqp,$crpp */
  {
    MEP_INSN_CPSUBLA0_H_P0S, "cpsubla0_h_P0S", "cpsubla0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsubaca0u.b $crqp,$crpp */
  {
    MEP_INSN_CPSUBACA0U_B_P0S, "cpsubaca0u_b_P0S", "cpsubaca0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsubaca0.b $crqp,$crpp */
  {
    MEP_INSN_CPSUBACA0_B_P0S, "cpsubaca0_b_P0S", "cpsubaca0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsubacua0.h $crqp,$crpp */
  {
    MEP_INSN_CPSUBACUA0_H_P0S, "cpsubacua0_h_P0S", "cpsubacua0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsubacla0.h $crqp,$crpp */
  {
    MEP_INSN_CPSUBACLA0_H_P0S, "cpsubacla0_h_P0S", "cpsubacla0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpabsa0u.b $crqp,$crpp */
  {
    MEP_INSN_CPABSA0U_B_P0S, "cpabsa0u_b_P0S", "cpabsa0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpabsa0.b $crqp,$crpp */
  {
    MEP_INSN_CPABSA0_B_P0S, "cpabsa0_b_P0S", "cpabsa0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpabsua0.h $crqp,$crpp */
  {
    MEP_INSN_CPABSUA0_H_P0S, "cpabsua0_h_P0S", "cpabsua0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpabsla0.h $crqp,$crpp */
  {
    MEP_INSN_CPABSLA0_H_P0S, "cpabsla0_h_P0S", "cpabsla0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsada0u.b $crqp,$crpp */
  {
    MEP_INSN_CPSADA0U_B_P0S, "cpsada0u_b_P0S", "cpsada0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsada0.b $crqp,$crpp */
  {
    MEP_INSN_CPSADA0_B_P0S, "cpsada0_b_P0S", "cpsada0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsadua0.h $crqp,$crpp */
  {
    MEP_INSN_CPSADUA0_H_P0S, "cpsadua0_h_P0S", "cpsadua0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsadla0.h $crqp,$crpp */
  {
    MEP_INSN_CPSADLA0_H_P0S, "cpsadla0_h_P0S", "cpsadla0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpseta0.h $crqp,$crpp */
  {
    MEP_INSN_CPSETA0_H_P0S, "cpseta0_h_P0S", "cpseta0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsetua0.w $crqp,$crpp */
  {
    MEP_INSN_CPSETUA0_W_P0S, "cpsetua0_w_P0S", "cpsetua0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsetla0.w $crqp,$crpp */
  {
    MEP_INSN_CPSETLA0_W_P0S, "cpsetla0_w_P0S", "cpsetla0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmova0.b $crop */
  {
    MEP_INSN_CPMOVA0_B_P0S, "cpmova0_b_P0S", "cpmova0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmovua0.h $crop */
  {
    MEP_INSN_CPMOVUA0_H_P0S, "cpmovua0_h_P0S", "cpmovua0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmovla0.h $crop */
  {
    MEP_INSN_CPMOVLA0_H_P0S, "cpmovla0_h_P0S", "cpmovla0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmovuua0.w $crop */
  {
    MEP_INSN_CPMOVUUA0_W_P0S, "cpmovuua0_w_P0S", "cpmovuua0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmovula0.w $crop */
  {
    MEP_INSN_CPMOVULA0_W_P0S, "cpmovula0_w_P0S", "cpmovula0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmovlua0.w $crop */
  {
    MEP_INSN_CPMOVLUA0_W_P0S, "cpmovlua0_w_P0S", "cpmovlua0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmovlla0.w $crop */
  {
    MEP_INSN_CPMOVLLA0_W_P0S, "cpmovlla0_w_P0S", "cpmovlla0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cppacka0u.b $crop */
  {
    MEP_INSN_CPPACKA0U_B_P0S, "cppacka0u_b_P0S", "cppacka0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cppacka0.b $crop */
  {
    MEP_INSN_CPPACKA0_B_P0S, "cppacka0_b_P0S", "cppacka0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cppackua0.h $crop */
  {
    MEP_INSN_CPPACKUA0_H_P0S, "cppackua0_h_P0S", "cppackua0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cppackla0.h $crop */
  {
    MEP_INSN_CPPACKLA0_H_P0S, "cppackla0_h_P0S", "cppackla0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cppackua0.w $crop */
  {
    MEP_INSN_CPPACKUA0_W_P0S, "cppackua0_w_P0S", "cppackua0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cppackla0.w $crop */
  {
    MEP_INSN_CPPACKLA0_W_P0S, "cppackla0_w_P0S", "cppackla0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmovhua0.w $crop */
  {
    MEP_INSN_CPMOVHUA0_W_P0S, "cpmovhua0_w_P0S", "cpmovhua0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpmovhla0.w $crop */
  {
    MEP_INSN_CPMOVHLA0_W_P0S, "cpmovhla0_w_P0S", "cpmovhla0.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpacsuma0 */
  {
    MEP_INSN_CPACSUMA0_P0S, "cpacsuma0_P0S", "cpacsuma0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpaccpa0 */
  {
    MEP_INSN_CPACCPA0_P0S, "cpaccpa0_P0S", "cpaccpa0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsrla0 $crqp */
  {
    MEP_INSN_CPSRLA0_P0S, "cpsrla0_P0S", "cpsrla0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsraa0 $crqp */
  {
    MEP_INSN_CPSRAA0_P0S, "cpsraa0_P0S", "cpsraa0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpslla0 $crqp */
  {
    MEP_INSN_CPSLLA0_P0S, "cpslla0_P0S", "cpslla0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsrlia0 $imm5p23 */
  {
    MEP_INSN_CPSRLIA0_P0S, "cpsrlia0_P0S", "cpsrlia0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsraia0 $imm5p23 */
  {
    MEP_INSN_CPSRAIA0_P0S, "cpsraia0_P0S", "cpsraia0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpsllia0 $imm5p23 */
  {
    MEP_INSN_CPSLLIA0_P0S, "cpsllia0_P0S", "cpsllia0", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftba0s0u.b $crqp,$crpp */
  {
    MEP_INSN_CPFSFTBA0S0U_B_P0S, "cpfsftba0s0u_b_P0S", "cpfsftba0s0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftba0s0.b $crqp,$crpp */
  {
    MEP_INSN_CPFSFTBA0S0_B_P0S, "cpfsftba0s0_b_P0S", "cpfsftba0s0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftbua0s0.h $crqp,$crpp */
  {
    MEP_INSN_CPFSFTBUA0S0_H_P0S, "cpfsftbua0s0_h_P0S", "cpfsftbua0s0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftbla0s0.h $crqp,$crpp */
  {
    MEP_INSN_CPFSFTBLA0S0_H_P0S, "cpfsftbla0s0_h_P0S", "cpfsftbla0s0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfaca0s0u.b $crqp,$crpp */
  {
    MEP_INSN_CPFACA0S0U_B_P0S, "cpfaca0s0u_b_P0S", "cpfaca0s0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfaca0s0.b $crqp,$crpp */
  {
    MEP_INSN_CPFACA0S0_B_P0S, "cpfaca0s0_b_P0S", "cpfaca0s0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfacua0s0.h $crqp,$crpp */
  {
    MEP_INSN_CPFACUA0S0_H_P0S, "cpfacua0s0_h_P0S", "cpfacua0s0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfacla0s0.h $crqp,$crpp */
  {
    MEP_INSN_CPFACLA0S0_H_P0S, "cpfacla0s0_h_P0S", "cpfacla0s0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftba0s1u.b $crqp,$crpp */
  {
    MEP_INSN_CPFSFTBA0S1U_B_P0S, "cpfsftba0s1u_b_P0S", "cpfsftba0s1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftba0s1.b $crqp,$crpp */
  {
    MEP_INSN_CPFSFTBA0S1_B_P0S, "cpfsftba0s1_b_P0S", "cpfsftba0s1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftbua0s1.h $crqp,$crpp */
  {
    MEP_INSN_CPFSFTBUA0S1_H_P0S, "cpfsftbua0s1_h_P0S", "cpfsftbua0s1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftbla0s1.h $crqp,$crpp */
  {
    MEP_INSN_CPFSFTBLA0S1_H_P0S, "cpfsftbla0s1_h_P0S", "cpfsftbla0s1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfaca0s1u.b $crqp,$crpp */
  {
    MEP_INSN_CPFACA0S1U_B_P0S, "cpfaca0s1u_b_P0S", "cpfaca0s1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfaca0s1.b $crqp,$crpp */
  {
    MEP_INSN_CPFACA0S1_B_P0S, "cpfaca0s1_b_P0S", "cpfaca0s1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfacua0s1.h $crqp,$crpp */
  {
    MEP_INSN_CPFACUA0S1_H_P0S, "cpfacua0s1_h_P0S", "cpfacua0s1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfacla0s1.h $crqp,$crpp */
  {
    MEP_INSN_CPFACLA0S1_H_P0S, "cpfacla0s1_h_P0S", "cpfacla0s1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x20" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S), 0 } } } }
  },
/* cpfsftbi $crop,$crqp,$crpp,$imm3p5 */
  {
    MEP_INSN_CPFSFTBI_P0_P1, "cpfsftbi_P0_P1", "cpfsftbi", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpeq.b $crqp,$crpp */
  {
    MEP_INSN_CPACMPEQ_B_P0_P1, "cpacmpeq_b_P0_P1", "cpacmpeq.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpeq.h $crqp,$crpp */
  {
    MEP_INSN_CPACMPEQ_H_P0_P1, "cpacmpeq_h_P0_P1", "cpacmpeq.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpeq.w $crqp,$crpp */
  {
    MEP_INSN_CPACMPEQ_W_P0_P1, "cpacmpeq_w_P0_P1", "cpacmpeq.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpne.b $crqp,$crpp */
  {
    MEP_INSN_CPACMPNE_B_P0_P1, "cpacmpne_b_P0_P1", "cpacmpne.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpne.h $crqp,$crpp */
  {
    MEP_INSN_CPACMPNE_H_P0_P1, "cpacmpne_h_P0_P1", "cpacmpne.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpne.w $crqp,$crpp */
  {
    MEP_INSN_CPACMPNE_W_P0_P1, "cpacmpne_w_P0_P1", "cpacmpne.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpgtu.b $crqp,$crpp */
  {
    MEP_INSN_CPACMPGTU_B_P0_P1, "cpacmpgtu_b_P0_P1", "cpacmpgtu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpgt.b $crqp,$crpp */
  {
    MEP_INSN_CPACMPGT_B_P0_P1, "cpacmpgt_b_P0_P1", "cpacmpgt.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpgt.h $crqp,$crpp */
  {
    MEP_INSN_CPACMPGT_H_P0_P1, "cpacmpgt_h_P0_P1", "cpacmpgt.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpgtu.w $crqp,$crpp */
  {
    MEP_INSN_CPACMPGTU_W_P0_P1, "cpacmpgtu_w_P0_P1", "cpacmpgtu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpgt.w $crqp,$crpp */
  {
    MEP_INSN_CPACMPGT_W_P0_P1, "cpacmpgt_w_P0_P1", "cpacmpgt.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpgeu.b $crqp,$crpp */
  {
    MEP_INSN_CPACMPGEU_B_P0_P1, "cpacmpgeu_b_P0_P1", "cpacmpgeu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpge.b $crqp,$crpp */
  {
    MEP_INSN_CPACMPGE_B_P0_P1, "cpacmpge_b_P0_P1", "cpacmpge.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpge.h $crqp,$crpp */
  {
    MEP_INSN_CPACMPGE_H_P0_P1, "cpacmpge_h_P0_P1", "cpacmpge.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpgeu.w $crqp,$crpp */
  {
    MEP_INSN_CPACMPGEU_W_P0_P1, "cpacmpgeu_w_P0_P1", "cpacmpgeu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpacmpge.w $crqp,$crpp */
  {
    MEP_INSN_CPACMPGE_W_P0_P1, "cpacmpge_w_P0_P1", "cpacmpge.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpeq.b $crqp,$crpp */
  {
    MEP_INSN_CPOCMPEQ_B_P0_P1, "cpocmpeq_b_P0_P1", "cpocmpeq.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpeq.h $crqp,$crpp */
  {
    MEP_INSN_CPOCMPEQ_H_P0_P1, "cpocmpeq_h_P0_P1", "cpocmpeq.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpeq.w $crqp,$crpp */
  {
    MEP_INSN_CPOCMPEQ_W_P0_P1, "cpocmpeq_w_P0_P1", "cpocmpeq.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpne.b $crqp,$crpp */
  {
    MEP_INSN_CPOCMPNE_B_P0_P1, "cpocmpne_b_P0_P1", "cpocmpne.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpne.h $crqp,$crpp */
  {
    MEP_INSN_CPOCMPNE_H_P0_P1, "cpocmpne_h_P0_P1", "cpocmpne.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpne.w $crqp,$crpp */
  {
    MEP_INSN_CPOCMPNE_W_P0_P1, "cpocmpne_w_P0_P1", "cpocmpne.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpgtu.b $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGTU_B_P0_P1, "cpocmpgtu_b_P0_P1", "cpocmpgtu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpgt.b $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGT_B_P0_P1, "cpocmpgt_b_P0_P1", "cpocmpgt.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpgt.h $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGT_H_P0_P1, "cpocmpgt_h_P0_P1", "cpocmpgt.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpgtu.w $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGTU_W_P0_P1, "cpocmpgtu_w_P0_P1", "cpocmpgtu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpgt.w $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGT_W_P0_P1, "cpocmpgt_w_P0_P1", "cpocmpgt.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpgeu.b $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGEU_B_P0_P1, "cpocmpgeu_b_P0_P1", "cpocmpgeu.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpge.b $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGE_B_P0_P1, "cpocmpge_b_P0_P1", "cpocmpge.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpge.h $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGE_H_P0_P1, "cpocmpge_h_P0_P1", "cpocmpge.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpgeu.w $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGEU_W_P0_P1, "cpocmpgeu_w_P0_P1", "cpocmpgeu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpocmpge.w $crqp,$crpp */
  {
    MEP_INSN_CPOCMPGE_W_P0_P1, "cpocmpge_w_P0_P1", "cpocmpge.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdadd3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CDADD3_P0_P1, "cdadd3_P0_P1", "cdadd3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsub3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSUB3_B_P0_P1, "cpsub3_b_P0_P1", "cpsub3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsub3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSUB3_H_P0_P1, "cpsub3_h_P0_P1", "cpsub3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsub3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSUB3_W_P0_P1, "cpsub3_w_P0_P1", "cpsub3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdsub3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CDSUB3_P0_P1, "cdsub3_P0_P1", "cdsub3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsadd3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSADD3_H_P0_P1, "cpsadd3_h_P0_P1", "cpsadd3.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsadd3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSADD3_W_P0_P1, "cpsadd3_w_P0_P1", "cpsadd3.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssub3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSUB3_H_P0_P1, "cpssub3_h_P0_P1", "cpssub3.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssub3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSUB3_W_P0_P1, "cpssub3_w_P0_P1", "cpssub3.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextuaddu3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPEXTUADDU3_B_P0_P1, "cpextuaddu3_b_P0_P1", "cpextuaddu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextuadd3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPEXTUADD3_B_P0_P1, "cpextuadd3_b_P0_P1", "cpextuadd3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextladdu3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPEXTLADDU3_B_P0_P1, "cpextladdu3_b_P0_P1", "cpextladdu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextladd3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPEXTLADD3_B_P0_P1, "cpextladd3_b_P0_P1", "cpextladd3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextusubu3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPEXTUSUBU3_B_P0_P1, "cpextusubu3_b_P0_P1", "cpextusubu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextusub3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPEXTUSUB3_B_P0_P1, "cpextusub3_b_P0_P1", "cpextusub3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextlsubu3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPEXTLSUBU3_B_P0_P1, "cpextlsubu3_b_P0_P1", "cpextlsubu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpextlsub3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPEXTLSUB3_B_P0_P1, "cpextlsub3_b_P0_P1", "cpextlsub3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpaveu3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPAVEU3_B_P0_P1, "cpaveu3_b_P0_P1", "cpaveu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpave3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPAVE3_B_P0_P1, "cpave3_b_P0_P1", "cpave3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpave3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPAVE3_H_P0_P1, "cpave3_h_P0_P1", "cpave3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpave3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPAVE3_W_P0_P1, "cpave3_w_P0_P1", "cpave3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpaddsru3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPADDSRU3_B_P0_P1, "cpaddsru3_b_P0_P1", "cpaddsru3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpaddsr3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPADDSR3_B_P0_P1, "cpaddsr3_b_P0_P1", "cpaddsr3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpaddsr3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPADDSR3_H_P0_P1, "cpaddsr3_h_P0_P1", "cpaddsr3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpaddsr3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPADDSR3_W_P0_P1, "cpaddsr3_w_P0_P1", "cpaddsr3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpabsu3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPABSU3_B_P0_P1, "cpabsu3_b_P0_P1", "cpabsu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpabs3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPABS3_B_P0_P1, "cpabs3_b_P0_P1", "cpabs3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpabs3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPABS3_H_P0_P1, "cpabs3_h_P0_P1", "cpabs3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpand3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CPAND3_P0_P1, "cpand3_P0_P1", "cpand3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_VECT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpor3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CPOR3_P0_P1, "cpor3_P0_P1", "cpor3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_VECT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpnor3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CPNOR3_P0_P1, "cpnor3_P0_P1", "cpnor3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_VECT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpxor3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CPXOR3_P0_P1, "cpxor3_P0_P1", "cpxor3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_VECT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cppacku.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPPACKU_B_P0_P1, "cppacku_b_P0_P1", "cppacku.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cppack.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPPACK_B_P0_P1, "cppack_b_P0_P1", "cppack.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cppack.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPPACK_H_P0_P1, "cppack_h_P0_P1", "cppack.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmaxu3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMAXU3_B_P0_P1, "cpmaxu3_b_P0_P1", "cpmaxu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmax3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMAX3_B_P0_P1, "cpmax3_b_P0_P1", "cpmax3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmax3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMAX3_H_P0_P1, "cpmax3_h_P0_P1", "cpmax3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmaxu3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMAXU3_W_P0_P1, "cpmaxu3_w_P0_P1", "cpmaxu3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmax3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMAX3_W_P0_P1, "cpmax3_w_P0_P1", "cpmax3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpminu3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMINU3_B_P0_P1, "cpminu3_b_P0_P1", "cpminu3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmin3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMIN3_B_P0_P1, "cpmin3_b_P0_P1", "cpmin3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmin3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMIN3_H_P0_P1, "cpmin3_h_P0_P1", "cpmin3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpminu3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMINU3_W_P0_P1, "cpminu3_w_P0_P1", "cpminu3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmin3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPMIN3_W_P0_P1, "cpmin3_w_P0_P1", "cpmin3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrl3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSRL3_B_P0_P1, "cpsrl3_b_P0_P1", "cpsrl3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssrl3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSRL3_B_P0_P1, "cpssrl3_b_P0_P1", "cpssrl3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrl3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSRL3_H_P0_P1, "cpsrl3_h_P0_P1", "cpsrl3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssrl3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSRL3_H_P0_P1, "cpssrl3_h_P0_P1", "cpssrl3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrl3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSRL3_W_P0_P1, "cpsrl3_w_P0_P1", "cpsrl3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssrl3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSRL3_W_P0_P1, "cpssrl3_w_P0_P1", "cpssrl3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdsrl3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CDSRL3_P0_P1, "cdsrl3_P0_P1", "cdsrl3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsra3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSRA3_B_P0_P1, "cpsra3_b_P0_P1", "cpsra3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssra3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSRA3_B_P0_P1, "cpssra3_b_P0_P1", "cpssra3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsra3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSRA3_H_P0_P1, "cpsra3_h_P0_P1", "cpsra3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssra3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSRA3_H_P0_P1, "cpssra3_h_P0_P1", "cpssra3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsra3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSRA3_W_P0_P1, "cpsra3_w_P0_P1", "cpsra3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssra3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSRA3_W_P0_P1, "cpssra3_w_P0_P1", "cpssra3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdsra3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CDSRA3_P0_P1, "cdsra3_P0_P1", "cdsra3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsll3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSLL3_B_P0_P1, "cpsll3_b_P0_P1", "cpsll3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssll3.b $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSLL3_B_P0_P1, "cpssll3_b_P0_P1", "cpssll3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsll3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSLL3_H_P0_P1, "cpsll3_h_P0_P1", "cpsll3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssll3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSLL3_H_P0_P1, "cpssll3_h_P0_P1", "cpssll3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsll3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSLL3_W_P0_P1, "cpsll3_w_P0_P1", "cpsll3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpssll3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSSLL3_W_P0_P1, "cpssll3_w_P0_P1", "cpssll3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdsll3 $crop,$crqp,$crpp */
  {
    MEP_INSN_CDSLL3_P0_P1, "cdsll3_P0_P1", "cdsll3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsla3.h $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSLA3_H_P0_P1, "cpsla3_h_P0_P1", "cpsla3.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsla3.w $crop,$crqp,$crpp */
  {
    MEP_INSN_CPSLA3_W_P0_P1, "cpsla3_w_P0_P1", "cpsla3.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrli3.b $crop,$crqp,$imm3p5 */
  {
    MEP_INSN_CPSRLI3_B_P0_P1, "cpsrli3_b_P0_P1", "cpsrli3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrli3.h $crop,$crqp,$imm4p4 */
  {
    MEP_INSN_CPSRLI3_H_P0_P1, "cpsrli3_h_P0_P1", "cpsrli3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrli3.w $crop,$crqp,$imm5p3 */
  {
    MEP_INSN_CPSRLI3_W_P0_P1, "cpsrli3_w_P0_P1", "cpsrli3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdsrli3 $crop,$crqp,$imm6p2 */
  {
    MEP_INSN_CDSRLI3_P0_P1, "cdsrli3_P0_P1", "cdsrli3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrai3.b $crop,$crqp,$imm3p5 */
  {
    MEP_INSN_CPSRAI3_B_P0_P1, "cpsrai3_b_P0_P1", "cpsrai3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrai3.h $crop,$crqp,$imm4p4 */
  {
    MEP_INSN_CPSRAI3_H_P0_P1, "cpsrai3_h_P0_P1", "cpsrai3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpsrai3.w $crop,$crqp,$imm5p3 */
  {
    MEP_INSN_CPSRAI3_W_P0_P1, "cpsrai3_w_P0_P1", "cpsrai3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdsrai3 $crop,$crqp,$imm6p2 */
  {
    MEP_INSN_CDSRAI3_P0_P1, "cdsrai3_P0_P1", "cdsrai3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpslli3.b $crop,$crqp,$imm3p5 */
  {
    MEP_INSN_CPSLLI3_B_P0_P1, "cpslli3_b_P0_P1", "cpslli3.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpslli3.h $crop,$crqp,$imm4p4 */
  {
    MEP_INSN_CPSLLI3_H_P0_P1, "cpslli3_h_P0_P1", "cpslli3.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpslli3.w $crop,$crqp,$imm5p3 */
  {
    MEP_INSN_CPSLLI3_W_P0_P1, "cpslli3_w_P0_P1", "cpslli3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdslli3 $crop,$crqp,$imm6p2 */
  {
    MEP_INSN_CDSLLI3_P0_P1, "cdslli3_P0_P1", "cdslli3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpslai3.h $crop,$crqp,$imm4p4 */
  {
    MEP_INSN_CPSLAI3_H_P0_P1, "cpslai3_h_P0_P1", "cpslai3.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpslai3.w $crop,$crqp,$imm5p3 */
  {
    MEP_INSN_CPSLAI3_W_P0_P1, "cpslai3_w_P0_P1", "cpslai3.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpclipiu3.w $crop,$crqp,$imm5p3 */
  {
    MEP_INSN_CPCLIPIU3_W_P0_P1, "cpclipiu3_w_P0_P1", "cpclipiu3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpclipi3.w $crop,$crqp,$imm5p3 */
  {
    MEP_INSN_CPCLIPI3_W_P0_P1, "cpclipi3_w_P0_P1", "cpclipi3.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdclipiu3 $crop,$crqp,$imm6p2 */
  {
    MEP_INSN_CDCLIPIU3_P0_P1, "cdclipiu3_P0_P1", "cdclipiu3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdclipi3 $crop,$crqp,$imm6p2 */
  {
    MEP_INSN_CDCLIPI3_P0_P1, "cdclipi3_P0_P1", "cdclipi3", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmovi.h $crqp,$simm16p0 */
  {
    MEP_INSN_CPMOVI_H_P0_P1, "cpmovi_h_P0_P1", "cpmovi.h", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmoviu.w $crqp,$imm16p0 */
  {
    MEP_INSN_CPMOVIU_W_P0_P1, "cpmoviu_w_P0_P1", "cpmoviu.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpmovi.w $crqp,$simm16p0 */
  {
    MEP_INSN_CPMOVI_W_P0_P1, "cpmovi_w_P0_P1", "cpmovi.w", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdmoviu $crqp,$imm16p0 */
  {
    MEP_INSN_CDMOVIU_P0_P1, "cdmoviu_P0_P1", "cdmoviu", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* cdmovi $crqp,$simm16p0 */
  {
    MEP_INSN_CDMOVI_P0_P1, "cdmovi_P0_P1", "cdmovi", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0)|(1<<SLOTS_P1), 0 } } } }
  },
/* c1nop */
  {
    MEP_INSN_C1NOP_P1, "c1nop_P1", "c1nop", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovi.b $crqp,$simm8p20 */
  {
    MEP_INSN_CPMOVI_B_P0S_P1, "cpmovi_b_P0S_P1", "cpmovi.b", 32,
    { 0|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x24" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P0S)|(1<<SLOTS_P1), 0 } } } }
  },
/* cpadda1u.b $crqp,$crpp */
  {
    MEP_INSN_CPADDA1U_B_P1, "cpadda1u_b_P1", "cpadda1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpadda1.b $crqp,$crpp */
  {
    MEP_INSN_CPADDA1_B_P1, "cpadda1_b_P1", "cpadda1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpaddua1.h $crqp,$crpp */
  {
    MEP_INSN_CPADDUA1_H_P1, "cpaddua1_h_P1", "cpaddua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpaddla1.h $crqp,$crpp */
  {
    MEP_INSN_CPADDLA1_H_P1, "cpaddla1_h_P1", "cpaddla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpaddaca1u.b $crqp,$crpp */
  {
    MEP_INSN_CPADDACA1U_B_P1, "cpaddaca1u_b_P1", "cpaddaca1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpaddaca1.b $crqp,$crpp */
  {
    MEP_INSN_CPADDACA1_B_P1, "cpaddaca1_b_P1", "cpaddaca1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpaddacua1.h $crqp,$crpp */
  {
    MEP_INSN_CPADDACUA1_H_P1, "cpaddacua1_h_P1", "cpaddacua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpaddacla1.h $crqp,$crpp */
  {
    MEP_INSN_CPADDACLA1_H_P1, "cpaddacla1_h_P1", "cpaddacla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsuba1u.b $crqp,$crpp */
  {
    MEP_INSN_CPSUBA1U_B_P1, "cpsuba1u_b_P1", "cpsuba1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsuba1.b $crqp,$crpp */
  {
    MEP_INSN_CPSUBA1_B_P1, "cpsuba1_b_P1", "cpsuba1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsubua1.h $crqp,$crpp */
  {
    MEP_INSN_CPSUBUA1_H_P1, "cpsubua1_h_P1", "cpsubua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsubla1.h $crqp,$crpp */
  {
    MEP_INSN_CPSUBLA1_H_P1, "cpsubla1_h_P1", "cpsubla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsubaca1u.b $crqp,$crpp */
  {
    MEP_INSN_CPSUBACA1U_B_P1, "cpsubaca1u_b_P1", "cpsubaca1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsubaca1.b $crqp,$crpp */
  {
    MEP_INSN_CPSUBACA1_B_P1, "cpsubaca1_b_P1", "cpsubaca1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsubacua1.h $crqp,$crpp */
  {
    MEP_INSN_CPSUBACUA1_H_P1, "cpsubacua1_h_P1", "cpsubacua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsubacla1.h $crqp,$crpp */
  {
    MEP_INSN_CPSUBACLA1_H_P1, "cpsubacla1_h_P1", "cpsubacla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpabsa1u.b $crqp,$crpp */
  {
    MEP_INSN_CPABSA1U_B_P1, "cpabsa1u_b_P1", "cpabsa1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpabsa1.b $crqp,$crpp */
  {
    MEP_INSN_CPABSA1_B_P1, "cpabsa1_b_P1", "cpabsa1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpabsua1.h $crqp,$crpp */
  {
    MEP_INSN_CPABSUA1_H_P1, "cpabsua1_h_P1", "cpabsua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpabsla1.h $crqp,$crpp */
  {
    MEP_INSN_CPABSLA1_H_P1, "cpabsla1_h_P1", "cpabsla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsada1u.b $crqp,$crpp */
  {
    MEP_INSN_CPSADA1U_B_P1, "cpsada1u_b_P1", "cpsada1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsada1.b $crqp,$crpp */
  {
    MEP_INSN_CPSADA1_B_P1, "cpsada1_b_P1", "cpsada1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsadua1.h $crqp,$crpp */
  {
    MEP_INSN_CPSADUA1_H_P1, "cpsadua1_h_P1", "cpsadua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsadla1.h $crqp,$crpp */
  {
    MEP_INSN_CPSADLA1_H_P1, "cpsadla1_h_P1", "cpsadla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpseta1.h $crqp,$crpp */
  {
    MEP_INSN_CPSETA1_H_P1, "cpseta1_h_P1", "cpseta1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsetua1.w $crqp,$crpp */
  {
    MEP_INSN_CPSETUA1_W_P1, "cpsetua1_w_P1", "cpsetua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsetla1.w $crqp,$crpp */
  {
    MEP_INSN_CPSETLA1_W_P1, "cpsetla1_w_P1", "cpsetla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmova1.b $crop */
  {
    MEP_INSN_CPMOVA1_B_P1, "cpmova1_b_P1", "cpmova1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovua1.h $crop */
  {
    MEP_INSN_CPMOVUA1_H_P1, "cpmovua1_h_P1", "cpmovua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovla1.h $crop */
  {
    MEP_INSN_CPMOVLA1_H_P1, "cpmovla1_h_P1", "cpmovla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovuua1.w $crop */
  {
    MEP_INSN_CPMOVUUA1_W_P1, "cpmovuua1_w_P1", "cpmovuua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovula1.w $crop */
  {
    MEP_INSN_CPMOVULA1_W_P1, "cpmovula1_w_P1", "cpmovula1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovlua1.w $crop */
  {
    MEP_INSN_CPMOVLUA1_W_P1, "cpmovlua1_w_P1", "cpmovlua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovlla1.w $crop */
  {
    MEP_INSN_CPMOVLLA1_W_P1, "cpmovlla1_w_P1", "cpmovlla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cppacka1u.b $crop */
  {
    MEP_INSN_CPPACKA1U_B_P1, "cppacka1u_b_P1", "cppacka1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cppacka1.b $crop */
  {
    MEP_INSN_CPPACKA1_B_P1, "cppacka1_b_P1", "cppacka1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cppackua1.h $crop */
  {
    MEP_INSN_CPPACKUA1_H_P1, "cppackua1_h_P1", "cppackua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cppackla1.h $crop */
  {
    MEP_INSN_CPPACKLA1_H_P1, "cppackla1_h_P1", "cppackla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cppackua1.w $crop */
  {
    MEP_INSN_CPPACKUA1_W_P1, "cppackua1_w_P1", "cppackua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cppackla1.w $crop */
  {
    MEP_INSN_CPPACKLA1_W_P1, "cppackla1_w_P1", "cppackla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovhua1.w $crop */
  {
    MEP_INSN_CPMOVHUA1_W_P1, "cpmovhua1_w_P1", "cpmovhua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmovhla1.w $crop */
  {
    MEP_INSN_CPMOVHLA1_W_P1, "cpmovhla1_w_P1", "cpmovhla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_FIRST, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpacsuma1 */
  {
    MEP_INSN_CPACSUMA1_P1, "cpacsuma1_P1", "cpacsuma1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpaccpa1 */
  {
    MEP_INSN_CPACCPA1_P1, "cpaccpa1_P1", "cpaccpa1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpacswp */
  {
    MEP_INSN_CPACSWP_P1, "cpacswp_P1", "cpacswp", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsrla1 $crqp */
  {
    MEP_INSN_CPSRLA1_P1, "cpsrla1_P1", "cpsrla1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsraa1 $crqp */
  {
    MEP_INSN_CPSRAA1_P1, "cpsraa1_P1", "cpsraa1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpslla1 $crqp */
  {
    MEP_INSN_CPSLLA1_P1, "cpslla1_P1", "cpslla1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsrlia1 $imm5p23 */
  {
    MEP_INSN_CPSRLIA1_1_P1, "cpsrlia1_1_p1", "cpsrlia1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsraia1 $imm5p23 */
  {
    MEP_INSN_CPSRAIA1_1_P1, "cpsraia1_1_p1", "cpsraia1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsllia1 $imm5p23 */
  {
    MEP_INSN_CPSLLIA1_1_P1, "cpsllia1_1_p1", "cpsllia1", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulia1s0u.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMULIA1S0U_B_P1, "cpfmulia1s0u_b_P1", "cpfmulia1s0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulia1s0.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMULIA1S0_B_P1, "cpfmulia1s0_b_P1", "cpfmulia1s0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmuliua1s0.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMULIUA1S0_H_P1, "cpfmuliua1s0_h_P1", "cpfmuliua1s0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulila1s0.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMULILA1S0_H_P1, "cpfmulila1s0_h_P1", "cpfmulila1s0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadia1s0u.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMADIA1S0U_B_P1, "cpfmadia1s0u_b_P1", "cpfmadia1s0u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadia1s0.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMADIA1S0_B_P1, "cpfmadia1s0_b_P1", "cpfmadia1s0.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadiua1s0.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMADIUA1S0_H_P1, "cpfmadiua1s0_h_P1", "cpfmadiua1s0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadila1s0.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMADILA1S0_H_P1, "cpfmadila1s0_h_P1", "cpfmadila1s0.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulia1s1u.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMULIA1S1U_B_P1, "cpfmulia1s1u_b_P1", "cpfmulia1s1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulia1s1.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMULIA1S1_B_P1, "cpfmulia1s1_b_P1", "cpfmulia1s1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmuliua1s1.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMULIUA1S1_H_P1, "cpfmuliua1s1_h_P1", "cpfmuliua1s1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulila1s1.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMULILA1S1_H_P1, "cpfmulila1s1_h_P1", "cpfmulila1s1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadia1s1u.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMADIA1S1U_B_P1, "cpfmadia1s1u_b_P1", "cpfmadia1s1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadia1s1.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMADIA1S1_B_P1, "cpfmadia1s1_b_P1", "cpfmadia1s1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadiua1s1.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMADIUA1S1_H_P1, "cpfmadiua1s1_h_P1", "cpfmadiua1s1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadila1s1.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPFMADILA1S1_H_P1, "cpfmadila1s1_h_P1", "cpfmadila1s1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpamulia1u.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPAMULIA1U_B_P1, "cpamulia1u_b_P1", "cpamulia1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpamulia1.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPAMULIA1_B_P1, "cpamulia1_b_P1", "cpamulia1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpamuliua1.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPAMULIUA1_H_P1, "cpamuliua1_h_P1", "cpamuliua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpamulila1.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPAMULILA1_H_P1, "cpamulila1_h_P1", "cpamulila1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpamadia1u.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPAMADIA1U_B_P1, "cpamadia1u_b_P1", "cpamadia1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpamadia1.b $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPAMADIA1_B_P1, "cpamadia1_b_P1", "cpamadia1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpamadiua1.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPAMADIUA1_H_P1, "cpamadiua1_h_P1", "cpamadiua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpamadila1.h $crqp,$crpp,$simm8p0 */
  {
    MEP_INSN_CPAMADILA1_H_P1, "cpamadila1_h_P1", "cpamadila1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulia1u.b $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    MEP_INSN_CPFMULIA1U_B_P1, "cpfmulia1u_b_P1", "cpfmulia1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulia1.b $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    MEP_INSN_CPFMULIA1_B_P1, "cpfmulia1_b_P1", "cpfmulia1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmuliua1.h $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    MEP_INSN_CPFMULIUA1_H_P1, "cpfmuliua1_h_P1", "cpfmuliua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmulila1.h $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    MEP_INSN_CPFMULILA1_H_P1, "cpfmulila1_h_P1", "cpfmulila1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadia1u.b $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    MEP_INSN_CPFMADIA1U_B_P1, "cpfmadia1u_b_P1", "cpfmadia1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadia1.b $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    MEP_INSN_CPFMADIA1_B_P1, "cpfmadia1_b_P1", "cpfmadia1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadiua1.h $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    MEP_INSN_CPFMADIUA1_H_P1, "cpfmadiua1_h_P1", "cpfmadiua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpfmadila1.h $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    MEP_INSN_CPFMADILA1_H_P1, "cpfmadila1_h_P1", "cpfmadila1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpssqa1u.b $crqp,$crpp */
  {
    MEP_INSN_CPSSQA1U_B_P1, "cpssqa1u_b_P1", "cpssqa1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpssqa1.b $crqp,$crpp */
  {
    MEP_INSN_CPSSQA1_B_P1, "cpssqa1_b_P1", "cpssqa1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpssda1u.b $crqp,$crpp */
  {
    MEP_INSN_CPSSDA1U_B_P1, "cpssda1u_b_P1", "cpssda1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpssda1.b $crqp,$crpp */
  {
    MEP_INSN_CPSSDA1_B_P1, "cpssda1_b_P1", "cpssda1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmula1u.b $crqp,$crpp */
  {
    MEP_INSN_CPMULA1U_B_P1, "cpmula1u_b_P1", "cpmula1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmula1.b $crqp,$crpp */
  {
    MEP_INSN_CPMULA1_B_P1, "cpmula1_b_P1", "cpmula1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulua1.h $crqp,$crpp */
  {
    MEP_INSN_CPMULUA1_H_P1, "cpmulua1_h_P1", "cpmulua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulla1.h $crqp,$crpp */
  {
    MEP_INSN_CPMULLA1_H_P1, "cpmulla1_h_P1", "cpmulla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulua1u.w $crqp,$crpp */
  {
    MEP_INSN_CPMULUA1U_W_P1, "cpmulua1u_w_P1", "cpmulua1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulla1u.w $crqp,$crpp */
  {
    MEP_INSN_CPMULLA1U_W_P1, "cpmulla1u_w_P1", "cpmulla1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulua1.w $crqp,$crpp */
  {
    MEP_INSN_CPMULUA1_W_P1, "cpmulua1_w_P1", "cpmulua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulla1.w $crqp,$crpp */
  {
    MEP_INSN_CPMULLA1_W_P1, "cpmulla1_w_P1", "cpmulla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmada1u.b $crqp,$crpp */
  {
    MEP_INSN_CPMADA1U_B_P1, "cpmada1u_b_P1", "cpmada1u.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8UQI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmada1.b $crqp,$crpp */
  {
    MEP_INSN_CPMADA1_B_P1, "cpmada1_b_P1", "cpmada1.b", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V8QI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmadua1.h $crqp,$crpp */
  {
    MEP_INSN_CPMADUA1_H_P1, "cpmadua1_h_P1", "cpmadua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmadla1.h $crqp,$crpp */
  {
    MEP_INSN_CPMADLA1_H_P1, "cpmadla1_h_P1", "cpmadla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmadua1u.w $crqp,$crpp */
  {
    MEP_INSN_CPMADUA1U_W_P1, "cpmadua1u_w_P1", "cpmadua1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmadla1u.w $crqp,$crpp */
  {
    MEP_INSN_CPMADLA1U_W_P1, "cpmadla1u_w_P1", "cpmadla1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmadua1.w $crqp,$crpp */
  {
    MEP_INSN_CPMADUA1_W_P1, "cpmadua1_w_P1", "cpmadua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmadla1.w $crqp,$crpp */
  {
    MEP_INSN_CPMADLA1_W_P1, "cpmadla1_w_P1", "cpmadla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmsbua1.h $crqp,$crpp */
  {
    MEP_INSN_CPMSBUA1_H_P1, "cpmsbua1_h_P1", "cpmsbua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmsbla1.h $crqp,$crpp */
  {
    MEP_INSN_CPMSBLA1_H_P1, "cpmsbla1_h_P1", "cpmsbla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmsbua1u.w $crqp,$crpp */
  {
    MEP_INSN_CPMSBUA1U_W_P1, "cpmsbua1u_w_P1", "cpmsbua1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmsbla1u.w $crqp,$crpp */
  {
    MEP_INSN_CPMSBLA1U_W_P1, "cpmsbla1u_w_P1", "cpmsbla1u.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2USI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmsbua1.w $crqp,$crpp */
  {
    MEP_INSN_CPMSBUA1_W_P1, "cpmsbua1_w_P1", "cpmsbua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmsbla1.w $crqp,$crpp */
  {
    MEP_INSN_CPMSBLA1_W_P1, "cpmsbla1_w_P1", "cpmsbla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmadua1.h $crqp,$crpp */
  {
    MEP_INSN_CPSMADUA1_H_P1, "cpsmadua1_h_P1", "cpsmadua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmadla1.h $crqp,$crpp */
  {
    MEP_INSN_CPSMADLA1_H_P1, "cpsmadla1_h_P1", "cpsmadla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmadua1.w $crqp,$crpp */
  {
    MEP_INSN_CPSMADUA1_W_P1, "cpsmadua1_w_P1", "cpsmadua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmadla1.w $crqp,$crpp */
  {
    MEP_INSN_CPSMADLA1_W_P1, "cpsmadla1_w_P1", "cpsmadla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmsbua1.h $crqp,$crpp */
  {
    MEP_INSN_CPSMSBUA1_H_P1, "cpsmsbua1_h_P1", "cpsmsbua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmsbla1.h $crqp,$crpp */
  {
    MEP_INSN_CPSMSBLA1_H_P1, "cpsmsbla1_h_P1", "cpsmsbla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmsbua1.w $crqp,$crpp */
  {
    MEP_INSN_CPSMSBUA1_W_P1, "cpsmsbua1_w_P1", "cpsmsbua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmsbla1.w $crqp,$crpp */
  {
    MEP_INSN_CPSMSBLA1_W_P1, "cpsmsbla1_w_P1", "cpsmsbla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulslua1.h $crqp,$crpp */
  {
    MEP_INSN_CPMULSLUA1_H_P1, "cpmulslua1_h_P1", "cpmulslua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulslla1.h $crqp,$crpp */
  {
    MEP_INSN_CPMULSLLA1_H_P1, "cpmulslla1_h_P1", "cpmulslla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulslua1.w $crqp,$crpp */
  {
    MEP_INSN_CPMULSLUA1_W_P1, "cpmulslua1_w_P1", "cpmulslua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpmulslla1.w $crqp,$crpp */
  {
    MEP_INSN_CPMULSLLA1_W_P1, "cpmulslla1_w_P1", "cpmulslla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmadslua1.h $crqp,$crpp */
  {
    MEP_INSN_CPSMADSLUA1_H_P1, "cpsmadslua1_h_P1", "cpsmadslua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmadslla1.h $crqp,$crpp */
  {
    MEP_INSN_CPSMADSLLA1_H_P1, "cpsmadslla1_h_P1", "cpsmadslla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmadslua1.w $crqp,$crpp */
  {
    MEP_INSN_CPSMADSLUA1_W_P1, "cpsmadslua1_w_P1", "cpsmadslua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmadslla1.w $crqp,$crpp */
  {
    MEP_INSN_CPSMADSLLA1_W_P1, "cpsmadslla1_w_P1", "cpsmadslla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmsbslua1.h $crqp,$crpp */
  {
    MEP_INSN_CPSMSBSLUA1_H_P1, "cpsmsbslua1_h_P1", "cpsmsbslua1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmsbslla1.h $crqp,$crpp */
  {
    MEP_INSN_CPSMSBSLLA1_H_P1, "cpsmsbslla1_h_P1", "cpsmsbslla1.h", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V4HI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmsbslua1.w $crqp,$crpp */
  {
    MEP_INSN_CPSMSBSLUA1_W_P1, "cpsmsbslua1_w_P1", "cpsmsbslua1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
  },
/* cpsmsbslla1.w $crqp,$crpp */
  {
    MEP_INSN_CPSMSBSLLA1_W_P1, "cpsmsbslla1_w_P1", "cpsmsbslla1.w", 32,
    { 0|A(VOLATILE)|A(OPTIONAL_CP_INSN), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x4" } }, { { CPTYPE_V2SI, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_P1), 0 } } } }
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
static void mep_cgen_rebuild_tables (CGEN_CPU_TABLE *);

/* Subroutine of mep_cgen_cpu_open to look up a mach via its bfd name.  */

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

/* Subroutine of mep_cgen_cpu_open to build the hardware table.  */

static void
build_hw_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_HW_ENTRY *init = & mep_cgen_hw_table[0];
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

/* Subroutine of mep_cgen_cpu_open to build the hardware table.  */

static void
build_ifield_table (CGEN_CPU_TABLE *cd)
{
  cd->ifld_table = & mep_cgen_ifld_table[0];
}

/* Subroutine of mep_cgen_cpu_open to build the hardware table.  */

static void
build_operand_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_OPERAND *init = & mep_cgen_operand_table[0];
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

/* Subroutine of mep_cgen_cpu_open to build the hardware table.
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
  const CGEN_IBASE *ib = & mep_cgen_insn_table[0];
  CGEN_INSN *insns = xmalloc (MAX_INSNS * sizeof (CGEN_INSN));

  memset (insns, 0, MAX_INSNS * sizeof (CGEN_INSN));
  for (i = 0; i < MAX_INSNS; ++i)
    insns[i].base = &ib[i];
  cd->insn_table.init_entries = insns;
  cd->insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->insn_table.num_init_entries = MAX_INSNS;
}

/* Subroutine of mep_cgen_cpu_open to rebuild the tables.  */

static void
mep_cgen_rebuild_tables (CGEN_CPU_TABLE *cd)
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
	const CGEN_ISA *isa = & mep_cgen_isa_table[i];

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
	const CGEN_MACH *mach = & mep_cgen_mach_table[i];

	if (mach->insn_chunk_bitsize != 0)
	{
	  if (cd->insn_chunk_bitsize != 0 && cd->insn_chunk_bitsize != mach->insn_chunk_bitsize)
	    {
	      opcodes_error_handler
		(/* xgettext:c-format */
		 _("internal error: mep_cgen_rebuild_tables: "
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
mep_cgen_cpu_open (enum cgen_cpu_open_arg arg_type, ...)
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
	      lookup_mach_via_bfd_name (mep_cgen_mach_table, name);

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
	     _("internal error: mep_cgen_cpu_open: "
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
	 _("internal error: mep_cgen_cpu_open: no endianness specified"));
      abort ();
    }

  cd->isas = cgen_bitset_copy (isas);
  cd->machs = machs;
  cd->endian = endian;
  cd->insn_endian
    = (insn_endian == CGEN_ENDIAN_UNKNOWN ? endian : insn_endian);

  /* Table (re)builder.  */
  cd->rebuild_tables = mep_cgen_rebuild_tables;
  mep_cgen_rebuild_tables (cd);

  /* Default to not allowing signed overflow.  */
  cd->signed_overflow_ok_p = 0;

  return (CGEN_CPU_DESC) cd;
}

/* Cover fn to mep_cgen_cpu_open to handle the simple case of 1 isa, 1 mach.
   MACH_NAME is the bfd name of the mach.  */

CGEN_CPU_DESC
mep_cgen_cpu_open_1 (const char *mach_name, enum cgen_endian endian)
{
  return mep_cgen_cpu_open (CGEN_CPU_OPEN_BFDMACH, mach_name,
			       CGEN_CPU_OPEN_ENDIAN, endian,
			       CGEN_CPU_OPEN_END);
}

/* Close a cpu table.
   ??? This can live in a machine independent file, but there's currently
   no place to put this file (there's no libcgen).  libopcodes is the wrong
   place as some simulator ports use this but they don't use libopcodes.  */

void
mep_cgen_cpu_close (CGEN_CPU_DESC cd)
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

