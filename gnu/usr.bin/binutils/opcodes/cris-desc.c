/* CPU data for cris.

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
#include "cris-desc.h"
#include "cris-opc.h"
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
  { "crisv0", MACH_CRISV0 },
  { "crisv3", MACH_CRISV3 },
  { "crisv8", MACH_CRISV8 },
  { "crisv10", MACH_CRISV10 },
  { "crisv32", MACH_CRISV32 },
  { "max", MACH_MAX },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY ISA_attr[] ATTRIBUTE_UNUSED =
{
  { "cris", ISA_CRIS },
  { "max", ISA_MAX },
  { 0, 0 }
};

const CGEN_ATTR_TABLE cris_cgen_ifield_attr_table[] =
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

const CGEN_ATTR_TABLE cris_cgen_hardware_attr_table[] =
{
  { "MACH", & MACH_attr[0], & MACH_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "CACHE-ADDR", &bool_attr[0], &bool_attr[0] },
  { "PC", &bool_attr[0], &bool_attr[0] },
  { "PROFILE", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE cris_cgen_operand_attr_table[] =
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

const CGEN_ATTR_TABLE cris_cgen_insn_attr_table[] =
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

static const CGEN_ISA cris_cgen_isa_table[] = {
  { "cris", 16, 16, 16, 48 },
  { 0, 0, 0, 0, 0 }
};

/* Machine variants.  */

static const CGEN_MACH cris_cgen_mach_table[] = {
  { "crisv0", "cris", MACH_CRISV0, 0 },
  { "crisv3", "cris", MACH_CRISV3, 0 },
  { "crisv8", "cris", MACH_CRISV8, 0 },
  { "crisv10", "cris", MACH_CRISV10, 0 },
  { "crisv32", "crisv32", MACH_CRISV32, 0 },
  { 0, 0, 0, 0 }
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_gr_names_pcreg_entries[] =
{
  { "PC", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "SP", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "R0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "R1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "R2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "R3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "R4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "R5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "R6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "R7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "R8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "R9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "R10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "R11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "R12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "R13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "R14", 14, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_gr_names_pcreg =
{
  & cris_cgen_opval_gr_names_pcreg_entries[0],
  17,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_gr_names_acr_entries[] =
{
  { "ACR", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "SP", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "R0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "R1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "R2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "R3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "R4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "R5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "R6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "R7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "R8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "R9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "R10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "R11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "R12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "R13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "R14", 14, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_gr_names_acr =
{
  & cris_cgen_opval_gr_names_acr_entries[0],
  17,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_gr_names_v32_entries[] =
{
  { "ACR", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "SP", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "R0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "R1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "R2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "R3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "R4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "R5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "R6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "R7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "R8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "R9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "R10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "R11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "R12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "R13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "R14", 14, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_gr_names_v32 =
{
  & cris_cgen_opval_gr_names_v32_entries[0],
  17,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_p_names_v10_entries[] =
{
  { "CCR", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "MOF", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "IBR", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "IRP", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "BAR", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "DCCR", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "BRP", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "USP", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "VR", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "SRP", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "P0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "P1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "P2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "P3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "P4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "P5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "P6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "P7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "P8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "P9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "P10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "P11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "P12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "P13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "P14", 14, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_p_names_v10 =
{
  & cris_cgen_opval_p_names_v10_entries[0],
  25,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_p_names_v32_entries[] =
{
  { "BZ", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "PID", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "SRS", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "WZ", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "EXS", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "EDA", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "MOF", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "DZ", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "EBP", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "ERP", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "NRP", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "CCS", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "USP", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "SPC", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "VR", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "SRP", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "P0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "P1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "P2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "P3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "P4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "P5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "P6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "P7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "P8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "P9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "P10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "P11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "P12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "P13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "P14", 14, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_p_names_v32 =
{
  & cris_cgen_opval_p_names_v32_entries[0],
  31,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_p_names_v32_x_entries[] =
{
  { "BZ", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "PID", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "SRS", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "WZ", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "EXS", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "EDA", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "MOF", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "DZ", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "EBP", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "ERP", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "NRP", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "CCS", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "USP", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "SPC", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "VR", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "SRP", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "P0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "P1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "P2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "P3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "P4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "P5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "P6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "P7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "P8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "P9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "P10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "P11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "P12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "P13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "P14", 14, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_p_names_v32_x =
{
  & cris_cgen_opval_p_names_v32_x_entries[0],
  31,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_h_inc_entries[] =
{
  { "", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "+", 1, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_h_inc =
{
  & cris_cgen_opval_h_inc_entries[0],
  2,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_h_ccode_entries[] =
{
  { "cc", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "cs", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "ne", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "eq", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "vc", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "vs", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "pl", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "mi", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "ls", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "hi", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "ge", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "lt", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "gt", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "le", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "a", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "wf", 15, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_h_ccode =
{
  & cris_cgen_opval_h_ccode_entries[0],
  16,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_h_swap_entries[] =
{
  { " ", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "r", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "b", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "br", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "w", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "wr", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "wb", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "wbr", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "n", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "nr", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "nb", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "nbr", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "nw", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "nwr", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "nwb", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "nwbr", 15, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_h_swap =
{
  & cris_cgen_opval_h_swap_entries[0],
  16,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_h_flagbits_entries[] =
{
  { "_", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "c", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "v", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "cv", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "z", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "cz", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "vz", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "cvz", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "n", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "cn", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "vn", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "cvn", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "zn", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "czn", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "vzn", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzn", 15, {0, {{{0, 0}}}}, 0, 0 },
  { "x", 16, {0, {{{0, 0}}}}, 0, 0 },
  { "cx", 17, {0, {{{0, 0}}}}, 0, 0 },
  { "vx", 18, {0, {{{0, 0}}}}, 0, 0 },
  { "cvx", 19, {0, {{{0, 0}}}}, 0, 0 },
  { "zx", 20, {0, {{{0, 0}}}}, 0, 0 },
  { "czx", 21, {0, {{{0, 0}}}}, 0, 0 },
  { "vzx", 22, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzx", 23, {0, {{{0, 0}}}}, 0, 0 },
  { "nx", 24, {0, {{{0, 0}}}}, 0, 0 },
  { "cnx", 25, {0, {{{0, 0}}}}, 0, 0 },
  { "vnx", 26, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnx", 27, {0, {{{0, 0}}}}, 0, 0 },
  { "znx", 28, {0, {{{0, 0}}}}, 0, 0 },
  { "cznx", 29, {0, {{{0, 0}}}}, 0, 0 },
  { "vznx", 30, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznx", 31, {0, {{{0, 0}}}}, 0, 0 },
  { "i", 32, {0, {{{0, 0}}}}, 0, 0 },
  { "ci", 33, {0, {{{0, 0}}}}, 0, 0 },
  { "vi", 34, {0, {{{0, 0}}}}, 0, 0 },
  { "cvi", 35, {0, {{{0, 0}}}}, 0, 0 },
  { "zi", 36, {0, {{{0, 0}}}}, 0, 0 },
  { "czi", 37, {0, {{{0, 0}}}}, 0, 0 },
  { "vzi", 38, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzi", 39, {0, {{{0, 0}}}}, 0, 0 },
  { "ni", 40, {0, {{{0, 0}}}}, 0, 0 },
  { "cni", 41, {0, {{{0, 0}}}}, 0, 0 },
  { "vni", 42, {0, {{{0, 0}}}}, 0, 0 },
  { "cvni", 43, {0, {{{0, 0}}}}, 0, 0 },
  { "zni", 44, {0, {{{0, 0}}}}, 0, 0 },
  { "czni", 45, {0, {{{0, 0}}}}, 0, 0 },
  { "vzni", 46, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzni", 47, {0, {{{0, 0}}}}, 0, 0 },
  { "xi", 48, {0, {{{0, 0}}}}, 0, 0 },
  { "cxi", 49, {0, {{{0, 0}}}}, 0, 0 },
  { "vxi", 50, {0, {{{0, 0}}}}, 0, 0 },
  { "cvxi", 51, {0, {{{0, 0}}}}, 0, 0 },
  { "zxi", 52, {0, {{{0, 0}}}}, 0, 0 },
  { "czxi", 53, {0, {{{0, 0}}}}, 0, 0 },
  { "vzxi", 54, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzxi", 55, {0, {{{0, 0}}}}, 0, 0 },
  { "nxi", 56, {0, {{{0, 0}}}}, 0, 0 },
  { "cnxi", 57, {0, {{{0, 0}}}}, 0, 0 },
  { "vnxi", 58, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnxi", 59, {0, {{{0, 0}}}}, 0, 0 },
  { "znxi", 60, {0, {{{0, 0}}}}, 0, 0 },
  { "cznxi", 61, {0, {{{0, 0}}}}, 0, 0 },
  { "vznxi", 62, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznxi", 63, {0, {{{0, 0}}}}, 0, 0 },
  { "u", 64, {0, {{{0, 0}}}}, 0, 0 },
  { "cu", 65, {0, {{{0, 0}}}}, 0, 0 },
  { "vu", 66, {0, {{{0, 0}}}}, 0, 0 },
  { "cvu", 67, {0, {{{0, 0}}}}, 0, 0 },
  { "zu", 68, {0, {{{0, 0}}}}, 0, 0 },
  { "czu", 69, {0, {{{0, 0}}}}, 0, 0 },
  { "vzu", 70, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzu", 71, {0, {{{0, 0}}}}, 0, 0 },
  { "nu", 72, {0, {{{0, 0}}}}, 0, 0 },
  { "cnu", 73, {0, {{{0, 0}}}}, 0, 0 },
  { "vnu", 74, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnu", 75, {0, {{{0, 0}}}}, 0, 0 },
  { "znu", 76, {0, {{{0, 0}}}}, 0, 0 },
  { "cznu", 77, {0, {{{0, 0}}}}, 0, 0 },
  { "vznu", 78, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznu", 79, {0, {{{0, 0}}}}, 0, 0 },
  { "xu", 80, {0, {{{0, 0}}}}, 0, 0 },
  { "cxu", 81, {0, {{{0, 0}}}}, 0, 0 },
  { "vxu", 82, {0, {{{0, 0}}}}, 0, 0 },
  { "cvxu", 83, {0, {{{0, 0}}}}, 0, 0 },
  { "zxu", 84, {0, {{{0, 0}}}}, 0, 0 },
  { "czxu", 85, {0, {{{0, 0}}}}, 0, 0 },
  { "vzxu", 86, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzxu", 87, {0, {{{0, 0}}}}, 0, 0 },
  { "nxu", 88, {0, {{{0, 0}}}}, 0, 0 },
  { "cnxu", 89, {0, {{{0, 0}}}}, 0, 0 },
  { "vnxu", 90, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnxu", 91, {0, {{{0, 0}}}}, 0, 0 },
  { "znxu", 92, {0, {{{0, 0}}}}, 0, 0 },
  { "cznxu", 93, {0, {{{0, 0}}}}, 0, 0 },
  { "vznxu", 94, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznxu", 95, {0, {{{0, 0}}}}, 0, 0 },
  { "iu", 96, {0, {{{0, 0}}}}, 0, 0 },
  { "ciu", 97, {0, {{{0, 0}}}}, 0, 0 },
  { "viu", 98, {0, {{{0, 0}}}}, 0, 0 },
  { "cviu", 99, {0, {{{0, 0}}}}, 0, 0 },
  { "ziu", 100, {0, {{{0, 0}}}}, 0, 0 },
  { "cziu", 101, {0, {{{0, 0}}}}, 0, 0 },
  { "vziu", 102, {0, {{{0, 0}}}}, 0, 0 },
  { "cvziu", 103, {0, {{{0, 0}}}}, 0, 0 },
  { "niu", 104, {0, {{{0, 0}}}}, 0, 0 },
  { "cniu", 105, {0, {{{0, 0}}}}, 0, 0 },
  { "vniu", 106, {0, {{{0, 0}}}}, 0, 0 },
  { "cvniu", 107, {0, {{{0, 0}}}}, 0, 0 },
  { "zniu", 108, {0, {{{0, 0}}}}, 0, 0 },
  { "czniu", 109, {0, {{{0, 0}}}}, 0, 0 },
  { "vzniu", 110, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzniu", 111, {0, {{{0, 0}}}}, 0, 0 },
  { "xiu", 112, {0, {{{0, 0}}}}, 0, 0 },
  { "cxiu", 113, {0, {{{0, 0}}}}, 0, 0 },
  { "vxiu", 114, {0, {{{0, 0}}}}, 0, 0 },
  { "cvxiu", 115, {0, {{{0, 0}}}}, 0, 0 },
  { "zxiu", 116, {0, {{{0, 0}}}}, 0, 0 },
  { "czxiu", 117, {0, {{{0, 0}}}}, 0, 0 },
  { "vzxiu", 118, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzxiu", 119, {0, {{{0, 0}}}}, 0, 0 },
  { "nxiu", 120, {0, {{{0, 0}}}}, 0, 0 },
  { "cnxiu", 121, {0, {{{0, 0}}}}, 0, 0 },
  { "vnxiu", 122, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnxiu", 123, {0, {{{0, 0}}}}, 0, 0 },
  { "znxiu", 124, {0, {{{0, 0}}}}, 0, 0 },
  { "cznxiu", 125, {0, {{{0, 0}}}}, 0, 0 },
  { "vznxiu", 126, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznxiu", 127, {0, {{{0, 0}}}}, 0, 0 },
  { "p", 128, {0, {{{0, 0}}}}, 0, 0 },
  { "cp", 129, {0, {{{0, 0}}}}, 0, 0 },
  { "vp", 130, {0, {{{0, 0}}}}, 0, 0 },
  { "cvp", 131, {0, {{{0, 0}}}}, 0, 0 },
  { "zp", 132, {0, {{{0, 0}}}}, 0, 0 },
  { "czp", 133, {0, {{{0, 0}}}}, 0, 0 },
  { "vzp", 134, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzp", 135, {0, {{{0, 0}}}}, 0, 0 },
  { "np", 136, {0, {{{0, 0}}}}, 0, 0 },
  { "cnp", 137, {0, {{{0, 0}}}}, 0, 0 },
  { "vnp", 138, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnp", 139, {0, {{{0, 0}}}}, 0, 0 },
  { "znp", 140, {0, {{{0, 0}}}}, 0, 0 },
  { "cznp", 141, {0, {{{0, 0}}}}, 0, 0 },
  { "vznp", 142, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznp", 143, {0, {{{0, 0}}}}, 0, 0 },
  { "xp", 144, {0, {{{0, 0}}}}, 0, 0 },
  { "cxp", 145, {0, {{{0, 0}}}}, 0, 0 },
  { "vxp", 146, {0, {{{0, 0}}}}, 0, 0 },
  { "cvxp", 147, {0, {{{0, 0}}}}, 0, 0 },
  { "zxp", 148, {0, {{{0, 0}}}}, 0, 0 },
  { "czxp", 149, {0, {{{0, 0}}}}, 0, 0 },
  { "vzxp", 150, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzxp", 151, {0, {{{0, 0}}}}, 0, 0 },
  { "nxp", 152, {0, {{{0, 0}}}}, 0, 0 },
  { "cnxp", 153, {0, {{{0, 0}}}}, 0, 0 },
  { "vnxp", 154, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnxp", 155, {0, {{{0, 0}}}}, 0, 0 },
  { "znxp", 156, {0, {{{0, 0}}}}, 0, 0 },
  { "cznxp", 157, {0, {{{0, 0}}}}, 0, 0 },
  { "vznxp", 158, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznxp", 159, {0, {{{0, 0}}}}, 0, 0 },
  { "ip", 160, {0, {{{0, 0}}}}, 0, 0 },
  { "cip", 161, {0, {{{0, 0}}}}, 0, 0 },
  { "vip", 162, {0, {{{0, 0}}}}, 0, 0 },
  { "cvip", 163, {0, {{{0, 0}}}}, 0, 0 },
  { "zip", 164, {0, {{{0, 0}}}}, 0, 0 },
  { "czip", 165, {0, {{{0, 0}}}}, 0, 0 },
  { "vzip", 166, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzip", 167, {0, {{{0, 0}}}}, 0, 0 },
  { "nip", 168, {0, {{{0, 0}}}}, 0, 0 },
  { "cnip", 169, {0, {{{0, 0}}}}, 0, 0 },
  { "vnip", 170, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnip", 171, {0, {{{0, 0}}}}, 0, 0 },
  { "znip", 172, {0, {{{0, 0}}}}, 0, 0 },
  { "cznip", 173, {0, {{{0, 0}}}}, 0, 0 },
  { "vznip", 174, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznip", 175, {0, {{{0, 0}}}}, 0, 0 },
  { "xip", 176, {0, {{{0, 0}}}}, 0, 0 },
  { "cxip", 177, {0, {{{0, 0}}}}, 0, 0 },
  { "vxip", 178, {0, {{{0, 0}}}}, 0, 0 },
  { "cvxip", 179, {0, {{{0, 0}}}}, 0, 0 },
  { "zxip", 180, {0, {{{0, 0}}}}, 0, 0 },
  { "czxip", 181, {0, {{{0, 0}}}}, 0, 0 },
  { "vzxip", 182, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzxip", 183, {0, {{{0, 0}}}}, 0, 0 },
  { "nxip", 184, {0, {{{0, 0}}}}, 0, 0 },
  { "cnxip", 185, {0, {{{0, 0}}}}, 0, 0 },
  { "vnxip", 186, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnxip", 187, {0, {{{0, 0}}}}, 0, 0 },
  { "znxip", 188, {0, {{{0, 0}}}}, 0, 0 },
  { "cznxip", 189, {0, {{{0, 0}}}}, 0, 0 },
  { "vznxip", 190, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznxip", 191, {0, {{{0, 0}}}}, 0, 0 },
  { "up", 192, {0, {{{0, 0}}}}, 0, 0 },
  { "cup", 193, {0, {{{0, 0}}}}, 0, 0 },
  { "vup", 194, {0, {{{0, 0}}}}, 0, 0 },
  { "cvup", 195, {0, {{{0, 0}}}}, 0, 0 },
  { "zup", 196, {0, {{{0, 0}}}}, 0, 0 },
  { "czup", 197, {0, {{{0, 0}}}}, 0, 0 },
  { "vzup", 198, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzup", 199, {0, {{{0, 0}}}}, 0, 0 },
  { "nup", 200, {0, {{{0, 0}}}}, 0, 0 },
  { "cnup", 201, {0, {{{0, 0}}}}, 0, 0 },
  { "vnup", 202, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnup", 203, {0, {{{0, 0}}}}, 0, 0 },
  { "znup", 204, {0, {{{0, 0}}}}, 0, 0 },
  { "cznup", 205, {0, {{{0, 0}}}}, 0, 0 },
  { "vznup", 206, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznup", 207, {0, {{{0, 0}}}}, 0, 0 },
  { "xup", 208, {0, {{{0, 0}}}}, 0, 0 },
  { "cxup", 209, {0, {{{0, 0}}}}, 0, 0 },
  { "vxup", 210, {0, {{{0, 0}}}}, 0, 0 },
  { "cvxup", 211, {0, {{{0, 0}}}}, 0, 0 },
  { "zxup", 212, {0, {{{0, 0}}}}, 0, 0 },
  { "czxup", 213, {0, {{{0, 0}}}}, 0, 0 },
  { "vzxup", 214, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzxup", 215, {0, {{{0, 0}}}}, 0, 0 },
  { "nxup", 216, {0, {{{0, 0}}}}, 0, 0 },
  { "cnxup", 217, {0, {{{0, 0}}}}, 0, 0 },
  { "vnxup", 218, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnxup", 219, {0, {{{0, 0}}}}, 0, 0 },
  { "znxup", 220, {0, {{{0, 0}}}}, 0, 0 },
  { "cznxup", 221, {0, {{{0, 0}}}}, 0, 0 },
  { "vznxup", 222, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznxup", 223, {0, {{{0, 0}}}}, 0, 0 },
  { "iup", 224, {0, {{{0, 0}}}}, 0, 0 },
  { "ciup", 225, {0, {{{0, 0}}}}, 0, 0 },
  { "viup", 226, {0, {{{0, 0}}}}, 0, 0 },
  { "cviup", 227, {0, {{{0, 0}}}}, 0, 0 },
  { "ziup", 228, {0, {{{0, 0}}}}, 0, 0 },
  { "cziup", 229, {0, {{{0, 0}}}}, 0, 0 },
  { "vziup", 230, {0, {{{0, 0}}}}, 0, 0 },
  { "cvziup", 231, {0, {{{0, 0}}}}, 0, 0 },
  { "niup", 232, {0, {{{0, 0}}}}, 0, 0 },
  { "cniup", 233, {0, {{{0, 0}}}}, 0, 0 },
  { "vniup", 234, {0, {{{0, 0}}}}, 0, 0 },
  { "cvniup", 235, {0, {{{0, 0}}}}, 0, 0 },
  { "zniup", 236, {0, {{{0, 0}}}}, 0, 0 },
  { "czniup", 237, {0, {{{0, 0}}}}, 0, 0 },
  { "vzniup", 238, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzniup", 239, {0, {{{0, 0}}}}, 0, 0 },
  { "xiup", 240, {0, {{{0, 0}}}}, 0, 0 },
  { "cxiup", 241, {0, {{{0, 0}}}}, 0, 0 },
  { "vxiup", 242, {0, {{{0, 0}}}}, 0, 0 },
  { "cvxiup", 243, {0, {{{0, 0}}}}, 0, 0 },
  { "zxiup", 244, {0, {{{0, 0}}}}, 0, 0 },
  { "czxiup", 245, {0, {{{0, 0}}}}, 0, 0 },
  { "vzxiup", 246, {0, {{{0, 0}}}}, 0, 0 },
  { "cvzxiup", 247, {0, {{{0, 0}}}}, 0, 0 },
  { "nxiup", 248, {0, {{{0, 0}}}}, 0, 0 },
  { "cnxiup", 249, {0, {{{0, 0}}}}, 0, 0 },
  { "vnxiup", 250, {0, {{{0, 0}}}}, 0, 0 },
  { "cvnxiup", 251, {0, {{{0, 0}}}}, 0, 0 },
  { "znxiup", 252, {0, {{{0, 0}}}}, 0, 0 },
  { "cznxiup", 253, {0, {{{0, 0}}}}, 0, 0 },
  { "vznxiup", 254, {0, {{{0, 0}}}}, 0, 0 },
  { "cvznxiup", 255, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_h_flagbits =
{
  & cris_cgen_opval_h_flagbits_entries[0],
  256,
  0, 0, 0, 0, ""
};

static CGEN_KEYWORD_ENTRY cris_cgen_opval_h_supr_entries[] =
{
  { "S0", 0, {0, {{{0, 0}}}}, 0, 0 },
  { "S1", 1, {0, {{{0, 0}}}}, 0, 0 },
  { "S2", 2, {0, {{{0, 0}}}}, 0, 0 },
  { "S3", 3, {0, {{{0, 0}}}}, 0, 0 },
  { "S4", 4, {0, {{{0, 0}}}}, 0, 0 },
  { "S5", 5, {0, {{{0, 0}}}}, 0, 0 },
  { "S6", 6, {0, {{{0, 0}}}}, 0, 0 },
  { "S7", 7, {0, {{{0, 0}}}}, 0, 0 },
  { "S8", 8, {0, {{{0, 0}}}}, 0, 0 },
  { "S9", 9, {0, {{{0, 0}}}}, 0, 0 },
  { "S10", 10, {0, {{{0, 0}}}}, 0, 0 },
  { "S11", 11, {0, {{{0, 0}}}}, 0, 0 },
  { "S12", 12, {0, {{{0, 0}}}}, 0, 0 },
  { "S13", 13, {0, {{{0, 0}}}}, 0, 0 },
  { "S14", 14, {0, {{{0, 0}}}}, 0, 0 },
  { "S15", 15, {0, {{{0, 0}}}}, 0, 0 }
};

CGEN_KEYWORD cris_cgen_opval_h_supr =
{
  & cris_cgen_opval_h_supr_entries[0],
  16,
  0, 0, 0, 0, ""
};


/* The hardware table.  */

#define A(a) (1 << CGEN_HW_##a)

const CGEN_HW_ENTRY cris_cgen_hw_table[] =
{
  { "h-memory", HW_H_MEMORY, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-sint", HW_H_SINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-uint", HW_H_UINT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-addr", HW_H_ADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-iaddr", HW_H_IADDR, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-inc", HW_H_INC, CGEN_ASM_KEYWORD, & cris_cgen_opval_h_inc, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-ccode", HW_H_CCODE, CGEN_ASM_KEYWORD, & cris_cgen_opval_h_ccode, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-swap", HW_H_SWAP, CGEN_ASM_KEYWORD, & cris_cgen_opval_h_swap, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-flagbits", HW_H_FLAGBITS, CGEN_ASM_KEYWORD, & cris_cgen_opval_h_flagbits, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-v32-v32", HW_H_V32, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-v32-non-v32", HW_H_V32, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-pc", HW_H_PC, CGEN_ASM_NONE, 0, { 0|A(PROFILE)|A(PC), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-gr", HW_H_GR, CGEN_ASM_NONE, 0, { 0|A(PROFILE)|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-gr-pc", HW_H_GR_X, CGEN_ASM_KEYWORD, & cris_cgen_opval_gr_names_pcreg, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-gr-real-pc", HW_H_GR_REAL_PC, CGEN_ASM_KEYWORD, & cris_cgen_opval_gr_names_pcreg, { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-raw-gr-pc", HW_H_RAW_GR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-gr-acr", HW_H_GR_X, CGEN_ASM_KEYWORD, & cris_cgen_opval_gr_names_acr, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-raw-gr-acr", HW_H_RAW_GR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-sr", HW_H_SR, CGEN_ASM_NONE, 0, { 0|A(PROFILE)|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-sr-v0", HW_H_SR_X, CGEN_ASM_KEYWORD, & cris_cgen_opval_p_names_v10, { 0, { { { (1<<MACH_CRISV0), 0 } } } } },
  { "h-sr-v3", HW_H_SR_X, CGEN_ASM_KEYWORD, & cris_cgen_opval_p_names_v10, { 0, { { { (1<<MACH_CRISV3), 0 } } } } },
  { "h-sr-v8", HW_H_SR_X, CGEN_ASM_KEYWORD, & cris_cgen_opval_p_names_v10, { 0, { { { (1<<MACH_CRISV8), 0 } } } } },
  { "h-sr-v10", HW_H_SR_X, CGEN_ASM_KEYWORD, & cris_cgen_opval_p_names_v10, { 0, { { { (1<<MACH_CRISV10), 0 } } } } },
  { "h-sr-v32", HW_H_SR_X, CGEN_ASM_KEYWORD, & cris_cgen_opval_p_names_v32, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-supr", HW_H_SUPR, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-cbit", HW_H_CBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-cbit-move", HW_H_CBIT_MOVE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-cbit-move-v32", HW_H_CBIT_MOVE_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-cbit-move-pre-v32", HW_H_CBIT_MOVE_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-vbit", HW_H_VBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-vbit-move", HW_H_VBIT_MOVE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-vbit-move-v32", HW_H_VBIT_MOVE_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-vbit-move-pre-v32", HW_H_VBIT_MOVE_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-zbit", HW_H_ZBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-zbit-move", HW_H_ZBIT_MOVE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-zbit-move-v32", HW_H_ZBIT_MOVE_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-zbit-move-pre-v32", HW_H_ZBIT_MOVE_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-nbit", HW_H_NBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-nbit-move", HW_H_NBIT_MOVE, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-nbit-move-v32", HW_H_NBIT_MOVE_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-nbit-move-pre-v32", HW_H_NBIT_MOVE_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-xbit", HW_H_XBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
  { "h-ibit", HW_H_IBIT, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-ibit-pre-v32", HW_H_IBIT_X, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-pbit", HW_H_PBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } } },
  { "h-rbit", HW_H_RBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-ubit", HW_H_UBIT, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-ubit-pre-v32", HW_H_UBIT_X, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV10), 0 } } } } },
  { "h-gbit", HW_H_GBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-kernel-sp", HW_H_KERNEL_SP, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-ubit-v32", HW_H_UBIT_X, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-ibit-v32", HW_H_IBIT_X, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-mbit", HW_H_MBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-qbit", HW_H_QBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-sbit", HW_H_SBIT, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-insn-prefixed-p", HW_H_INSN_PREFIXED_P, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } } },
  { "h-insn-prefixed-p-pre-v32", HW_H_INSN_PREFIXED_P_X, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-insn-prefixed-p-v32", HW_H_INSN_PREFIXED_P_X, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { "h-prefixreg-pre-v32", HW_H_PREFIXREG, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } } },
  { "h-prefixreg-v32", HW_H_PREFIXREG, CGEN_ASM_NONE, 0, { 0|A(VIRTUAL), { { { (1<<MACH_CRISV32), 0 } } } } },
  { 0, 0, CGEN_ASM_NONE, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A


/* The instruction field table.  */

#define A(a) (1 << CGEN_IFLD_##a)

const CGEN_IFLD cris_cgen_ifld_table[] =
{
  { CRIS_F_NIL, "f-nil", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_ANYOF, "f-anyof", 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_OPERAND1, "f-operand1", 0, 16, 3, 4, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_SIZE, "f-size", 0, 16, 5, 2, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_OPCODE, "f-opcode", 0, 16, 9, 4, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_MODE, "f-mode", 0, 16, 11, 2, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_OPERAND2, "f-operand2", 0, 16, 15, 4, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_MEMMODE, "f-memmode", 0, 16, 10, 1, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_MEMBIT, "f-membit", 0, 16, 11, 1, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_B5, "f-b5", 0, 16, 5, 1, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_OPCODE_HI, "f-opcode-hi", 0, 16, 9, 2, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_DSTSRC, "f-dstsrc", 0, 0, 0, 0,{ 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_U6, "f-u6", 0, 16, 5, 6, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_S6, "f-s6", 0, 16, 5, 6, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_U5, "f-u5", 0, 16, 4, 5, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_U4, "f-u4", 0, 16, 3, 4, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_S8, "f-s8", 0, 16, 7, 8, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_DISP9_HI, "f-disp9-hi", 0, 16, 0, 1, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_DISP9_LO, "f-disp9-lo", 0, 16, 7, 7, { 0, { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_DISP9, "f-disp9", 0, 0, 0, 0,{ 0|A(PCREL_ADDR)|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_QO, "f-qo", 0, 16, 3, 4, { 0|A(PCREL_ADDR), { { { (1<<MACH_CRISV32), 0 } } } }  },
  { CRIS_F_INDIR_PC__BYTE, "f-indir-pc+-byte", 16, 16, 15, 16, { 0|A(SIGN_OPT), { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_INDIR_PC__WORD, "f-indir-pc+-word", 16, 16, 15, 16, { 0|A(SIGN_OPT), { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_INDIR_PC__WORD_PCREL, "f-indir-pc+-word-pcrel", 16, 16, 15, 16, { 0|A(SIGN_OPT)|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_INDIR_PC__DWORD, "f-indir-pc+-dword", 16, 32, 31, 32, { 0|A(SIGN_OPT), { { { (1<<MACH_BASE), 0 } } } }  },
  { CRIS_F_INDIR_PC__DWORD_PCREL, "f-indir-pc+-dword-pcrel", 16, 32, 31, 32, { 0|A(PCREL_ADDR)|A(SIGN_OPT), { { { (1<<MACH_CRISV32), 0 } } } }  },
  { 0, 0, 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A



/* multi ifield declarations */

const CGEN_MAYBE_MULTI_IFLD CRIS_F_DSTSRC_MULTI_IFIELD [];
const CGEN_MAYBE_MULTI_IFLD CRIS_F_DISP9_MULTI_IFIELD [];


/* multi ifield definitions */

const CGEN_MAYBE_MULTI_IFLD CRIS_F_DSTSRC_MULTI_IFIELD [] =
{
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND2] } },
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND1] } },
    { 0, { 0 } }
};
const CGEN_MAYBE_MULTI_IFLD CRIS_F_DISP9_MULTI_IFIELD [] =
{
    { 0, { &cris_cgen_ifld_table[CRIS_F_DISP9_HI] } },
    { 0, { &cris_cgen_ifld_table[CRIS_F_DISP9_LO] } },
    { 0, { 0 } }
};

/* The operand table.  */

#define A(a) (1 << CGEN_OPERAND_##a)
#define OPERAND(op) CRIS_OPERAND_##op

const CGEN_OPERAND cris_cgen_operand_table[] =
{
/* pc: program counter */
  { "pc", CRIS_OPERAND_PC, HW_H_PC, 0, 0,
    { 0, { &cris_cgen_ifld_table[CRIS_F_NIL] } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* cbit:  */
  { "cbit", CRIS_OPERAND_CBIT, HW_H_CBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* cbit-move: cbit for pre-V32, nothing for newer */
  { "cbit-move", CRIS_OPERAND_CBIT_MOVE, HW_H_CBIT_MOVE, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* vbit:  */
  { "vbit", CRIS_OPERAND_VBIT, HW_H_VBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* vbit-move: vbit for pre-V32, nothing for newer */
  { "vbit-move", CRIS_OPERAND_VBIT_MOVE, HW_H_VBIT_MOVE, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* zbit:  */
  { "zbit", CRIS_OPERAND_ZBIT, HW_H_ZBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* zbit-move: zbit for pre-V32, nothing for newer */
  { "zbit-move", CRIS_OPERAND_ZBIT_MOVE, HW_H_ZBIT_MOVE, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* nbit:  */
  { "nbit", CRIS_OPERAND_NBIT, HW_H_NBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* nbit-move: nbit for pre-V32, nothing for newer */
  { "nbit-move", CRIS_OPERAND_NBIT_MOVE, HW_H_NBIT_MOVE, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* xbit:  */
  { "xbit", CRIS_OPERAND_XBIT, HW_H_XBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* ibit:  */
  { "ibit", CRIS_OPERAND_IBIT, HW_H_IBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* ubit:  */
  { "ubit", CRIS_OPERAND_UBIT, HW_H_UBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }  },
/* pbit:  */
  { "pbit", CRIS_OPERAND_PBIT, HW_H_PBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }  },
/* rbit: carry bit for MCP+restore-P flag bit */
  { "rbit", CRIS_OPERAND_RBIT, HW_H_RBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_CRISV32), 0 } } } }  },
/* sbit:  */
  { "sbit", CRIS_OPERAND_SBIT, HW_H_SBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_CRISV32), 0 } } } }  },
/* mbit:  */
  { "mbit", CRIS_OPERAND_MBIT, HW_H_MBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_CRISV32), 0 } } } }  },
/* qbit:  */
  { "qbit", CRIS_OPERAND_QBIT, HW_H_QBIT, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_CRISV32), 0 } } } }  },
/* prefix-set: Instruction-prefixed flag */
  { "prefix-set", CRIS_OPERAND_PREFIX_SET, HW_H_INSN_PREFIXED_P, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* prefixreg: Prefix address */
  { "prefixreg", CRIS_OPERAND_PREFIXREG, HW_H_PREFIXREG, 0, 0,
    { 0, { 0 } }, 
    { 0|A(SEM_ONLY), { { { (1<<MACH_BASE), 0 } } } }  },
/* Rs: Source general register */
  { "Rs", CRIS_OPERAND_RS, HW_H_GR, 3, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND1] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* inc: Incrementness of indirect operand */
  { "inc", CRIS_OPERAND_INC, HW_H_INC, 10, 1,
    { 0, { &cris_cgen_ifld_table[CRIS_F_MEMMODE] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* Ps: Source special register */
  { "Ps", CRIS_OPERAND_PS, HW_H_SR, 15, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND2] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* Ss: Source support register */
  { "Ss", CRIS_OPERAND_SS, HW_H_SUPR, 15, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND2] } }, 
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }  },
/* Sd: Destination support register */
  { "Sd", CRIS_OPERAND_SD, HW_H_SUPR, 15, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND2] } }, 
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }  },
/* i: Quick signed 6-bit */
  { "i", CRIS_OPERAND_I, HW_H_SINT, 5, 6,
    { 0, { &cris_cgen_ifld_table[CRIS_F_S6] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* j: Quick unsigned 6-bit */
  { "j", CRIS_OPERAND_J, HW_H_UINT, 5, 6,
    { 0, { &cris_cgen_ifld_table[CRIS_F_U6] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* c: Quick unsigned 5-bit */
  { "c", CRIS_OPERAND_C, HW_H_UINT, 4, 5,
    { 0, { &cris_cgen_ifld_table[CRIS_F_U5] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* qo: Quick unsigned 4-bit, PC-relative */
  { "qo", CRIS_OPERAND_QO, HW_H_ADDR, 3, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_QO] } }, 
    { 0|A(PCREL_ADDR), { { { (1<<MACH_CRISV32), 0 } } } }  },
/* Rd: Destination general register */
  { "Rd", CRIS_OPERAND_RD, HW_H_GR, 15, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND2] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* sconst8: Signed byte [PC+] */
  { "sconst8", CRIS_OPERAND_SCONST8, HW_H_SINT, 15, 16,
    { 0, { &cris_cgen_ifld_table[CRIS_F_INDIR_PC__BYTE] } }, 
    { 0|A(SIGN_OPT), { { { (1<<MACH_BASE), 0 } } } }  },
/* uconst8: Unsigned byte [PC+] */
  { "uconst8", CRIS_OPERAND_UCONST8, HW_H_UINT, 15, 16,
    { 0, { &cris_cgen_ifld_table[CRIS_F_INDIR_PC__BYTE] } }, 
    { 0|A(SIGN_OPT), { { { (1<<MACH_BASE), 0 } } } }  },
/* sconst16: Signed word [PC+] */
  { "sconst16", CRIS_OPERAND_SCONST16, HW_H_SINT, 15, 16,
    { 0, { &cris_cgen_ifld_table[CRIS_F_INDIR_PC__WORD] } }, 
    { 0|A(SIGN_OPT), { { { (1<<MACH_BASE), 0 } } } }  },
/* uconst16: Unsigned word [PC+] */
  { "uconst16", CRIS_OPERAND_UCONST16, HW_H_UINT, 15, 16,
    { 0, { &cris_cgen_ifld_table[CRIS_F_INDIR_PC__WORD] } }, 
    { 0|A(SIGN_OPT), { { { (1<<MACH_BASE), 0 } } } }  },
/* const32: Dword [PC+] */
  { "const32", CRIS_OPERAND_CONST32, HW_H_UINT, 31, 32,
    { 0, { &cris_cgen_ifld_table[CRIS_F_INDIR_PC__DWORD] } }, 
    { 0|A(SIGN_OPT), { { { (1<<MACH_BASE), 0 } } } }  },
/* const32-pcrel: Dword [PC+] */
  { "const32-pcrel", CRIS_OPERAND_CONST32_PCREL, HW_H_ADDR, 31, 32,
    { 0, { &cris_cgen_ifld_table[CRIS_F_INDIR_PC__DWORD_PCREL] } }, 
    { 0|A(PCREL_ADDR)|A(SIGN_OPT), { { { (1<<MACH_CRISV32), 0 } } } }  },
/* Pd: Destination special register */
  { "Pd", CRIS_OPERAND_PD, HW_H_SR, 15, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND2] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* o: Signed 8-bit */
  { "o", CRIS_OPERAND_O, HW_H_SINT, 7, 8,
    { 0, { &cris_cgen_ifld_table[CRIS_F_S8] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* o-pcrel: 9-bit signed immediate PC-rel */
  { "o-pcrel", CRIS_OPERAND_O_PCREL, HW_H_IADDR, 0, 8,
    { 2, { &CRIS_F_DISP9_MULTI_IFIELD[0] } }, 
    { 0|A(PCREL_ADDR)|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } }  },
/* o-word-pcrel: 16-bit signed immediate PC-rel */
  { "o-word-pcrel", CRIS_OPERAND_O_WORD_PCREL, HW_H_IADDR, 15, 16,
    { 0, { &cris_cgen_ifld_table[CRIS_F_INDIR_PC__WORD_PCREL] } }, 
    { 0|A(SIGN_OPT)|A(PCREL_ADDR), { { { (1<<MACH_BASE), 0 } } } }  },
/* cc: Condition codes */
  { "cc", CRIS_OPERAND_CC, HW_H_CCODE, 15, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND2] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* n: Quick unsigned 4-bit */
  { "n", CRIS_OPERAND_N, HW_H_UINT, 3, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_U4] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* swapoption: Swap option */
  { "swapoption", CRIS_OPERAND_SWAPOPTION, HW_H_SWAP, 15, 4,
    { 0, { &cris_cgen_ifld_table[CRIS_F_OPERAND2] } }, 
    { 0, { { { (1<<MACH_BASE), 0 } } } }  },
/* list-of-flags: Flag bits as operand */
  { "list-of-flags", CRIS_OPERAND_LIST_OF_FLAGS, HW_H_FLAGBITS, 3, 8,
    { 2, { &CRIS_F_DSTSRC_MULTI_IFIELD[0] } }, 
    { 0|A(VIRTUAL), { { { (1<<MACH_BASE), 0 } } } }  },
/* sentinel */
  { 0, 0, 0, 0, 0,
    { 0, { 0 } },
    { 0, { { { (1<<MACH_BASE), 0 } } } } }
};

#undef A


/* The instruction table.  */

#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))
#define A(a) (1 << CGEN_INSN_##a)

static const CGEN_IBASE cris_cgen_insn_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { 0, 0, 0, 0, { 0, { { { (1<<MACH_BASE), 0 } } } } },
/* nop */
  {
    CRIS_INSN_NOP, "nop", "nop", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* move.b move.m ${Rs},${Rd} */
  {
    CRIS_INSN_MOVE_B_R, "move.b-r", "move.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move.w move.m ${Rs},${Rd} */
  {
    CRIS_INSN_MOVE_W_R, "move.w-r", "move.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move.d move.m ${Rs},${Rd} */
  {
    CRIS_INSN_MOVE_D_R, "move.d-r", "move.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move.d PC,${Rd} */
  {
    CRIS_INSN_MOVEPCR, "movepcr", "move.d", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* moveq $i,$Rd */
  {
    CRIS_INSN_MOVEQ, "moveq", "moveq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movs.b movs.m ${Rs},${Rd} */
  {
    CRIS_INSN_MOVS_B_R, "movs.b-r", "movs.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movs.w movs.m ${Rs},${Rd} */
  {
    CRIS_INSN_MOVS_W_R, "movs.w-r", "movs.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movu.b movu.m ${Rs},${Rd} */
  {
    CRIS_INSN_MOVU_B_R, "movu.b-r", "movu.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movu.w movu.m ${Rs},${Rd} */
  {
    CRIS_INSN_MOVU_W_R, "movu.w-r", "movu.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move.b ${sconst8},${Rd} */
  {
    CRIS_INSN_MOVECBR, "movecbr", "move.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move.w ${sconst16},${Rd} */
  {
    CRIS_INSN_MOVECWR, "movecwr", "move.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move.d ${const32},${Rd} */
  {
    CRIS_INSN_MOVECDR, "movecdr", "move.d", 48,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movs.b ${sconst8},${Rd} */
  {
    CRIS_INSN_MOVSCBR, "movscbr", "movs.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movs.w ${sconst16},${Rd} */
  {
    CRIS_INSN_MOVSCWR, "movscwr", "movs.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movu.b ${uconst8},${Rd} */
  {
    CRIS_INSN_MOVUCBR, "movucbr", "movu.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movu.w ${uconst16},${Rd} */
  {
    CRIS_INSN_MOVUCWR, "movucwr", "movu.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addq $j,$Rd */
  {
    CRIS_INSN_ADDQ, "addq", "addq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subq $j,$Rd */
  {
    CRIS_INSN_SUBQ, "subq", "subq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp-r.b $Rs,$Rd */
  {
    CRIS_INSN_CMP_R_B_R, "cmp-r.b-r", "cmp-r.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp-r.w $Rs,$Rd */
  {
    CRIS_INSN_CMP_R_W_R, "cmp-r.w-r", "cmp-r.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp-r.d $Rs,$Rd */
  {
    CRIS_INSN_CMP_R_D_R, "cmp-r.d-r", "cmp-r.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_CMP_M_B_M, "cmp-m.b-m", "cmp-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_CMP_M_W_M, "cmp-m.w-m", "cmp-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp-m.d [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_CMP_M_D_M, "cmp-m.d-m", "cmp-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp.b $sconst8,$Rd */
  {
    CRIS_INSN_CMPCBR, "cmpcbr", "cmp.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp.w $sconst16,$Rd */
  {
    CRIS_INSN_CMPCWR, "cmpcwr", "cmp.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmp.d $const32,$Rd */
  {
    CRIS_INSN_CMPCDR, "cmpcdr", "cmp.d", 48,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpq $i,$Rd */
  {
    CRIS_INSN_CMPQ, "cmpq", "cmpq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmps-m.b [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_CMPS_M_B_M, "cmps-m.b-m", "cmps-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmps-m.w [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_CMPS_M_W_M, "cmps-m.w-m", "cmps-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_CMPSCBR, "cmpscbr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_CMPSCWR, "cmpscwr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpu-m.b [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_CMPU_M_B_M, "cmpu-m.b-m", "cmpu-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* cmpu-m.w [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_CMPU_M_W_M, "cmpu-m.w-m", "cmpu-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_CMPUCBR, "cmpucbr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_CMPUCWR, "cmpucwr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVE_M_B_M, "move-m.b-m", "move-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVE_M_W_M, "move-m.w-m", "move-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move-m.d [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVE_M_D_M, "move-m.d-m", "move-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movs-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVS_M_B_M, "movs-m.b-m", "movs-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movs-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVS_M_W_M, "movs-m.w-m", "movs-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movu-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVU_M_B_M, "movu-m.b-m", "movu-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* movu-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVU_M_W_M, "movu-m.w-m", "movu-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move ${Rs},${Pd} */
  {
    CRIS_INSN_MOVE_R_SPRV0, "move-r-sprv0", "move", 16,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${Rs},${Pd} */
  {
    CRIS_INSN_MOVE_R_SPRV3, "move-r-sprv3", "move", 16,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${Rs},${Pd} */
  {
    CRIS_INSN_MOVE_R_SPRV8, "move-r-sprv8", "move", 16,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${Rs},${Pd} */
  {
    CRIS_INSN_MOVE_R_SPRV10, "move-r-sprv10", "move", 16,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${Rs},${Pd} */
  {
    CRIS_INSN_MOVE_R_SPRV32, "move-r-sprv32", "move", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${Ps},${Rd-sfield} */
  {
    CRIS_INSN_MOVE_SPR_RV0, "move-spr-rv0", "move", 16,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${Ps},${Rd-sfield} */
  {
    CRIS_INSN_MOVE_SPR_RV3, "move-spr-rv3", "move", 16,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${Ps},${Rd-sfield} */
  {
    CRIS_INSN_MOVE_SPR_RV8, "move-spr-rv8", "move", 16,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${Ps},${Rd-sfield} */
  {
    CRIS_INSN_MOVE_SPR_RV10, "move-spr-rv10", "move", 16,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${Ps},${Rd-sfield} */
  {
    CRIS_INSN_MOVE_SPR_RV32, "move-spr-rv32", "move", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* ret/reti/retb */
  {
    CRIS_INSN_RET_TYPE, "ret-type", "ret/reti/retb", 16,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* move [${Rs}${inc}],${Pd} */
  {
    CRIS_INSN_MOVE_M_SPRV0, "move-m-sprv0", "move", 16,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move [${Rs}${inc}],${Pd} */
  {
    CRIS_INSN_MOVE_M_SPRV3, "move-m-sprv3", "move", 16,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move [${Rs}${inc}],${Pd} */
  {
    CRIS_INSN_MOVE_M_SPRV8, "move-m-sprv8", "move", 16,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move [${Rs}${inc}],${Pd} */
  {
    CRIS_INSN_MOVE_M_SPRV10, "move-m-sprv10", "move", 16,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move [${Rs}${inc}],${Pd} */
  {
    CRIS_INSN_MOVE_M_SPRV32, "move-m-sprv32", "move", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${sconst16},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV0_P5, "move-c-sprv0-p5", "move", 32,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV0_P9, "move-c-sprv0-p9", "move", 48,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV0_P10, "move-c-sprv0-p10", "move", 48,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV0_P11, "move-c-sprv0-p11", "move", 48,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV0_P12, "move-c-sprv0-p12", "move", 48,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV0_P13, "move-c-sprv0-p13", "move", 48,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${sconst16},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV0_P6, "move-c-sprv0-p6", "move", 32,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${sconst16},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV0_P7, "move-c-sprv0-p7", "move", 32,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${sconst16},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P5, "move-c-sprv3-p5", "move", 32,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P9, "move-c-sprv3-p9", "move", 48,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P10, "move-c-sprv3-p10", "move", 48,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P11, "move-c-sprv3-p11", "move", 48,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P12, "move-c-sprv3-p12", "move", 48,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P13, "move-c-sprv3-p13", "move", 48,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${sconst16},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P6, "move-c-sprv3-p6", "move", 32,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${sconst16},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P7, "move-c-sprv3-p7", "move", 32,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV3_P14, "move-c-sprv3-p14", "move", 48,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${sconst16},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV8_P5, "move-c-sprv8-p5", "move", 32,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV8_P9, "move-c-sprv8-p9", "move", 48,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV8_P10, "move-c-sprv8-p10", "move", 48,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV8_P11, "move-c-sprv8-p11", "move", 48,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV8_P12, "move-c-sprv8-p12", "move", 48,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV8_P13, "move-c-sprv8-p13", "move", 48,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV8_P14, "move-c-sprv8-p14", "move", 48,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${sconst16},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P5, "move-c-sprv10-p5", "move", 32,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P9, "move-c-sprv10-p9", "move", 48,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P10, "move-c-sprv10-p10", "move", 48,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P11, "move-c-sprv10-p11", "move", 48,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P12, "move-c-sprv10-p12", "move", 48,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P13, "move-c-sprv10-p13", "move", 48,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P7, "move-c-sprv10-p7", "move", 48,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P14, "move-c-sprv10-p14", "move", 48,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV10_P15, "move-c-sprv10-p15", "move", 48,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P2, "move-c-sprv32-p2", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P3, "move-c-sprv32-p3", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P5, "move-c-sprv32-p5", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P6, "move-c-sprv32-p6", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P7, "move-c-sprv32-p7", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P9, "move-c-sprv32-p9", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P10, "move-c-sprv32-p10", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P11, "move-c-sprv32-p11", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P12, "move-c-sprv32-p12", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P13, "move-c-sprv32-p13", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P14, "move-c-sprv32-p14", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${const32},${Pd} */
  {
    CRIS_INSN_MOVE_C_SPRV32_P15, "move-c-sprv32-p15", "move", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${Ps},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVE_SPR_MV0, "move-spr-mv0", "move", 16,
    { 0, { { { (1<<MACH_CRISV0), 0 } } } }
  },
/* move ${Ps},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVE_SPR_MV3, "move-spr-mv3", "move", 16,
    { 0, { { { (1<<MACH_CRISV3), 0 } } } }
  },
/* move ${Ps},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVE_SPR_MV8, "move-spr-mv8", "move", 16,
    { 0, { { { (1<<MACH_CRISV8), 0 } } } }
  },
/* move ${Ps},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVE_SPR_MV10, "move-spr-mv10", "move", 16,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${Ps},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVE_SPR_MV32, "move-spr-mv32", "move", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* sbfs [${Rd-sfield}${inc}] */
  {
    CRIS_INSN_SBFS, "sbfs", "sbfs", 16,
    { 0, { { { (1<<MACH_CRISV10), 0 } } } }
  },
/* move ${Ss},${Rd-sfield} */
  {
    CRIS_INSN_MOVE_SS_R, "move-ss-r", "move", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* move ${Rs},${Sd} */
  {
    CRIS_INSN_MOVE_R_SS, "move-r-ss", "move", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* movem ${Rs-dfield},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVEM_R_M, "movem-r-m", "movem", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* movem ${Rs-dfield},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVEM_R_M_V32, "movem-r-m-v32", "movem", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* movem [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVEM_M_R, "movem-m-r", "movem", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* movem [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVEM_M_PC, "movem-m-pc", "movem", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* movem [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_MOVEM_M_R_V32, "movem-m-r-v32", "movem", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* add.b $Rs,$Rd */
  {
    CRIS_INSN_ADD_B_R, "add.b-r", "add.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add.w $Rs,$Rd */
  {
    CRIS_INSN_ADD_W_R, "add.w-r", "add.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add.d $Rs,$Rd */
  {
    CRIS_INSN_ADD_D_R, "add.d-r", "add.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_ADD_M_B_M, "add-m.b-m", "add-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_ADD_M_W_M, "add-m.w-m", "add-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add-m.d [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_ADD_M_D_M, "add-m.d-m", "add-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add.b ${sconst8}],${Rd} */
  {
    CRIS_INSN_ADDCBR, "addcbr", "add.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add.w ${sconst16}],${Rd} */
  {
    CRIS_INSN_ADDCWR, "addcwr", "add.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add.d ${const32}],${Rd} */
  {
    CRIS_INSN_ADDCDR, "addcdr", "add.d", 48,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* add.d ${sconst32},PC */
  {
    CRIS_INSN_ADDCPC, "addcpc", "add.d", 48,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* adds.b $Rs,$Rd */
  {
    CRIS_INSN_ADDS_B_R, "adds.b-r", "adds.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* adds.w $Rs,$Rd */
  {
    CRIS_INSN_ADDS_W_R, "adds.w-r", "adds.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* adds-m.b [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_ADDS_M_B_M, "adds-m.b-m", "adds-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* adds-m.w [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_ADDS_M_W_M, "adds-m.w-m", "adds-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_ADDSCBR, "addscbr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_ADDSCWR, "addscwr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* adds.w [PC],PC */
  {
    CRIS_INSN_ADDSPCPC, "addspcpc", "adds.w", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* addu.b $Rs,$Rd */
  {
    CRIS_INSN_ADDU_B_R, "addu.b-r", "addu.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addu.w $Rs,$Rd */
  {
    CRIS_INSN_ADDU_W_R, "addu.w-r", "addu.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addu-m.b [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_ADDU_M_B_M, "addu-m.b-m", "addu-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addu-m.w [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_ADDU_M_W_M, "addu-m.w-m", "addu-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_ADDUCBR, "adducbr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_ADDUCWR, "adducwr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub.b $Rs,$Rd */
  {
    CRIS_INSN_SUB_B_R, "sub.b-r", "sub.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub.w $Rs,$Rd */
  {
    CRIS_INSN_SUB_W_R, "sub.w-r", "sub.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub.d $Rs,$Rd */
  {
    CRIS_INSN_SUB_D_R, "sub.d-r", "sub.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_SUB_M_B_M, "sub-m.b-m", "sub-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_SUB_M_W_M, "sub-m.w-m", "sub-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub-m.d [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_SUB_M_D_M, "sub-m.d-m", "sub-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub.b ${sconst8}],${Rd} */
  {
    CRIS_INSN_SUBCBR, "subcbr", "sub.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub.w ${sconst16}],${Rd} */
  {
    CRIS_INSN_SUBCWR, "subcwr", "sub.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* sub.d ${const32}],${Rd} */
  {
    CRIS_INSN_SUBCDR, "subcdr", "sub.d", 48,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subs.b $Rs,$Rd */
  {
    CRIS_INSN_SUBS_B_R, "subs.b-r", "subs.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subs.w $Rs,$Rd */
  {
    CRIS_INSN_SUBS_W_R, "subs.w-r", "subs.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subs-m.b [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_SUBS_M_B_M, "subs-m.b-m", "subs-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subs-m.w [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_SUBS_M_W_M, "subs-m.w-m", "subs-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_SUBSCBR, "subscbr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_SUBSCWR, "subscwr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subu.b $Rs,$Rd */
  {
    CRIS_INSN_SUBU_B_R, "subu.b-r", "subu.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subu.w $Rs,$Rd */
  {
    CRIS_INSN_SUBU_W_R, "subu.w-r", "subu.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subu-m.b [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_SUBU_M_B_M, "subu-m.b-m", "subu-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* subu-m.w [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_SUBU_M_W_M, "subu-m.w-m", "subu-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_SUBUCBR, "subucbr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* [${Rs}${inc}],$Rd */
  {
    CRIS_INSN_SUBUCWR, "subucwr", "[", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addc $Rs,$Rd */
  {
    CRIS_INSN_ADDC_R, "addc-r", "addc", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* addc [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_ADDC_M, "addc-m", "addc", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* addc ${const32},${Rd} */
  {
    CRIS_INSN_ADDC_C, "addc-c", "addc", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* lapc.d ${const32-pcrel},${Rd} */
  {
    CRIS_INSN_LAPC_D, "lapc-d", "lapc.d", 48,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* lapcq ${qo},${Rd} */
  {
    CRIS_INSN_LAPCQ, "lapcq", "lapcq", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* addi.b ${Rs-dfield}.m,${Rd-sfield} */
  {
    CRIS_INSN_ADDI_B_R, "addi.b-r", "addi.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addi.w ${Rs-dfield}.m,${Rd-sfield} */
  {
    CRIS_INSN_ADDI_W_R, "addi.w-r", "addi.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addi.d ${Rs-dfield}.m,${Rd-sfield} */
  {
    CRIS_INSN_ADDI_D_R, "addi.d-r", "addi.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* neg.b $Rs,$Rd */
  {
    CRIS_INSN_NEG_B_R, "neg.b-r", "neg.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* neg.w $Rs,$Rd */
  {
    CRIS_INSN_NEG_W_R, "neg.w-r", "neg.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* neg.d $Rs,$Rd */
  {
    CRIS_INSN_NEG_D_R, "neg.d-r", "neg.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* test-m.b [${Rs}${inc}] */
  {
    CRIS_INSN_TEST_M_B_M, "test-m.b-m", "test-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* test-m.w [${Rs}${inc}] */
  {
    CRIS_INSN_TEST_M_W_M, "test-m.w-m", "test-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* test-m.d [${Rs}${inc}] */
  {
    CRIS_INSN_TEST_M_D_M, "test-m.d-m", "test-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move-r-m.b ${Rs-dfield},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVE_R_M_B_M, "move-r-m.b-m", "move-r-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move-r-m.w ${Rs-dfield},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVE_R_M_W_M, "move-r-m.w-m", "move-r-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* move-r-m.d ${Rs-dfield},[${Rd-sfield}${inc}] */
  {
    CRIS_INSN_MOVE_R_M_D_M, "move-r-m.d-m", "move-r-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* muls.b $Rs,$Rd */
  {
    CRIS_INSN_MULS_B, "muls.b", "muls.b", 16,
    { 0, { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }
  },
/* muls.w $Rs,$Rd */
  {
    CRIS_INSN_MULS_W, "muls.w", "muls.w", 16,
    { 0, { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }
  },
/* muls.d $Rs,$Rd */
  {
    CRIS_INSN_MULS_D, "muls.d", "muls.d", 16,
    { 0, { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }
  },
/* mulu.b $Rs,$Rd */
  {
    CRIS_INSN_MULU_B, "mulu.b", "mulu.b", 16,
    { 0, { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }
  },
/* mulu.w $Rs,$Rd */
  {
    CRIS_INSN_MULU_W, "mulu.w", "mulu.w", 16,
    { 0, { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }
  },
/* mulu.d $Rs,$Rd */
  {
    CRIS_INSN_MULU_D, "mulu.d", "mulu.d", 16,
    { 0, { { { (1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }
  },
/* mcp $Ps,$Rd */
  {
    CRIS_INSN_MCP, "mcp", "mcp", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* mstep $Rs,$Rd */
  {
    CRIS_INSN_MSTEP, "mstep", "mstep", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* dstep $Rs,$Rd */
  {
    CRIS_INSN_DSTEP, "dstep", "dstep", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* abs $Rs,$Rd */
  {
    CRIS_INSN_ABS, "abs", "abs", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and.b $Rs,$Rd */
  {
    CRIS_INSN_AND_B_R, "and.b-r", "and.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and.w $Rs,$Rd */
  {
    CRIS_INSN_AND_W_R, "and.w-r", "and.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and.d $Rs,$Rd */
  {
    CRIS_INSN_AND_D_R, "and.d-r", "and.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_AND_M_B_M, "and-m.b-m", "and-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_AND_M_W_M, "and-m.w-m", "and-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and-m.d [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_AND_M_D_M, "and-m.d-m", "and-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and.b ${sconst8}],${Rd} */
  {
    CRIS_INSN_ANDCBR, "andcbr", "and.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and.w ${sconst16}],${Rd} */
  {
    CRIS_INSN_ANDCWR, "andcwr", "and.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* and.d ${const32}],${Rd} */
  {
    CRIS_INSN_ANDCDR, "andcdr", "and.d", 48,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* andq $i,$Rd */
  {
    CRIS_INSN_ANDQ, "andq", "andq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* orr.b $Rs,$Rd */
  {
    CRIS_INSN_ORR_B_R, "orr.b-r", "orr.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* orr.w $Rs,$Rd */
  {
    CRIS_INSN_ORR_W_R, "orr.w-r", "orr.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* orr.d $Rs,$Rd */
  {
    CRIS_INSN_ORR_D_R, "orr.d-r", "orr.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* or-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_OR_M_B_M, "or-m.b-m", "or-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* or-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_OR_M_W_M, "or-m.w-m", "or-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* or-m.d [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_OR_M_D_M, "or-m.d-m", "or-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* or.b ${sconst8}],${Rd} */
  {
    CRIS_INSN_ORCBR, "orcbr", "or.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* or.w ${sconst16}],${Rd} */
  {
    CRIS_INSN_ORCWR, "orcwr", "or.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* or.d ${const32}],${Rd} */
  {
    CRIS_INSN_ORCDR, "orcdr", "or.d", 48,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* orq $i,$Rd */
  {
    CRIS_INSN_ORQ, "orq", "orq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* xor $Rs,$Rd */
  {
    CRIS_INSN_XOR, "xor", "xor", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* not ${Rs} */
  {
    CRIS_INSN_NOT, "not", "not", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3), 0 } } } }
  },
/* swap${swapoption} ${Rs} */
  {
    CRIS_INSN_SWAP, "swap", "swap", 16,
    { 0, { { { (1<<MACH_CRISV8)|(1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }
  },
/* asrr.b $Rs,$Rd */
  {
    CRIS_INSN_ASRR_B_R, "asrr.b-r", "asrr.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* asrr.w $Rs,$Rd */
  {
    CRIS_INSN_ASRR_W_R, "asrr.w-r", "asrr.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* asrr.d $Rs,$Rd */
  {
    CRIS_INSN_ASRR_D_R, "asrr.d-r", "asrr.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* asrq $c,${Rd} */
  {
    CRIS_INSN_ASRQ, "asrq", "asrq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lsrr.b $Rs,$Rd */
  {
    CRIS_INSN_LSRR_B_R, "lsrr.b-r", "lsrr.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lsrr.w $Rs,$Rd */
  {
    CRIS_INSN_LSRR_W_R, "lsrr.w-r", "lsrr.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lsrr.d $Rs,$Rd */
  {
    CRIS_INSN_LSRR_D_R, "lsrr.d-r", "lsrr.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lsrq $c,${Rd} */
  {
    CRIS_INSN_LSRQ, "lsrq", "lsrq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lslr.b $Rs,$Rd */
  {
    CRIS_INSN_LSLR_B_R, "lslr.b-r", "lslr.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lslr.w $Rs,$Rd */
  {
    CRIS_INSN_LSLR_W_R, "lslr.w-r", "lslr.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lslr.d $Rs,$Rd */
  {
    CRIS_INSN_LSLR_D_R, "lslr.d-r", "lslr.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lslq $c,${Rd} */
  {
    CRIS_INSN_LSLQ, "lslq", "lslq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* $Rs,$Rd */
  {
    CRIS_INSN_BTST, "btst", "", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* btstq $c,${Rd} */
  {
    CRIS_INSN_BTSTQ, "btstq", "btstq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* setf ${list-of-flags} */
  {
    CRIS_INSN_SETF, "setf", "setf", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* clearf ${list-of-flags} */
  {
    CRIS_INSN_CLEARF, "clearf", "clearf", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* rfe */
  {
    CRIS_INSN_RFE, "rfe", "rfe", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* sfe */
  {
    CRIS_INSN_SFE, "sfe", "sfe", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* rfg */
  {
    CRIS_INSN_RFG, "rfg", "rfg", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* rfn */
  {
    CRIS_INSN_RFN, "rfn", "rfn", 16,
    { 0, { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* halt */
  {
    CRIS_INSN_HALT, "halt", "halt", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* b${cc} ${o-pcrel} */
  {
    CRIS_INSN_BCC_B, "bcc-b", "b", 16,
    { 0|A(COND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ba ${o-pcrel} */
  {
    CRIS_INSN_BA_B, "ba-b", "ba", 16,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_BASE), 0 } } } }
  },
/* b${cc} ${o-word-pcrel} */
  {
    CRIS_INSN_BCC_W, "bcc-w", "b", 32,
    { 0|A(COND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_BASE), 0 } } } }
  },
/* ba ${o-word-pcrel} */
  {
    CRIS_INSN_BA_W, "ba-w", "ba", 32,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_BASE), 0 } } } }
  },
/* jas ${Rs},${Pd} */
  {
    CRIS_INSN_JAS_R, "jas-r", "jas", 16,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* jump/jsr/jir ${Rs} */
  {
    CRIS_INSN_JUMP_R, "jump-r", "jump/jsr/jir", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* jas ${const32},${Pd} */
  {
    CRIS_INSN_JAS_C, "jas-c", "jas", 48,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* jump/jsr/jir [${Rs}${inc}] */
  {
    CRIS_INSN_JUMP_M, "jump-m", "jump/jsr/jir", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* jump/jsr/jir ${const32} */
  {
    CRIS_INSN_JUMP_C, "jump-c", "jump/jsr/jir", 48,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* jump ${Ps} */
  {
    CRIS_INSN_JUMP_P, "jump-p", "jump", 16,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* bas ${const32},${Pd} */
  {
    CRIS_INSN_BAS_C, "bas-c", "bas", 48,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* jasc ${Rs},${Pd} */
  {
    CRIS_INSN_JASC_R, "jasc-r", "jasc", 16,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* jasc ${const32},${Pd} */
  {
    CRIS_INSN_JASC_C, "jasc-c", "jasc", 48,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* basc ${const32},${Pd} */
  {
    CRIS_INSN_BASC_C, "basc-c", "basc", 48,
    { 0|A(UNCOND_CTI)|A(DELAY_SLOT), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* break $n */
  {
    CRIS_INSN_BREAK, "break", "break", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_BASE), 0 } } } }
  },
/* bound-r.b ${Rs},${Rd} */
  {
    CRIS_INSN_BOUND_R_B_R, "bound-r.b-r", "bound-r.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* bound-r.w ${Rs},${Rd} */
  {
    CRIS_INSN_BOUND_R_W_R, "bound-r.w-r", "bound-r.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* bound-r.d ${Rs},${Rd} */
  {
    CRIS_INSN_BOUND_R_D_R, "bound-r.d-r", "bound-r.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* bound-m.b [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_BOUND_M_B_M, "bound-m.b-m", "bound-m.b", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* bound-m.w [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_BOUND_M_W_M, "bound-m.w-m", "bound-m.w", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* bound-m.d [${Rs}${inc}],${Rd} */
  {
    CRIS_INSN_BOUND_M_D_M, "bound-m.d-m", "bound-m.d", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* bound.b [PC+],${Rd} */
  {
    CRIS_INSN_BOUND_CB, "bound-cb", "bound.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* bound.w [PC+],${Rd} */
  {
    CRIS_INSN_BOUND_CW, "bound-cw", "bound.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* bound.d [PC+],${Rd} */
  {
    CRIS_INSN_BOUND_CD, "bound-cd", "bound.d", 48,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* s${cc} ${Rd-sfield} */
  {
    CRIS_INSN_SCC, "scc", "s", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* lz ${Rs},${Rd} */
  {
    CRIS_INSN_LZ, "lz", "lz", 16,
    { 0, { { { (1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10)|(1<<MACH_CRISV32), 0 } } } }
  },
/* addoq $o,$Rs,ACR */
  {
    CRIS_INSN_ADDOQ, "addoq", "addoq", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* bdapq $o,PC */
  {
    CRIS_INSN_BDAPQPC, "bdapqpc", "bdapq", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* bdap ${sconst32},PC */
  {
    CRIS_INSN_BDAP_32_PC, "bdap-32-pc", "bdap", 48,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* move [PC+],P0 */
  {
    CRIS_INSN_MOVE_M_PCPLUS_P0, "move-m-pcplus-p0", "move", 16,
    { 0|A(COND_CTI), { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* move [SP+],P8 */
  {
    CRIS_INSN_MOVE_M_SPPLUS_P8, "move-m-spplus-p8", "move", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* addo-m.b [${Rs}${inc}],$Rd,ACR */
  {
    CRIS_INSN_ADDO_M_B_M, "addo-m.b-m", "addo-m.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addo-m.w [${Rs}${inc}],$Rd,ACR */
  {
    CRIS_INSN_ADDO_M_W_M, "addo-m.w-m", "addo-m.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addo-m.d [${Rs}${inc}],$Rd,ACR */
  {
    CRIS_INSN_ADDO_M_D_M, "addo-m.d-m", "addo-m.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addo.b [PC+],$Rd,ACR */
  {
    CRIS_INSN_ADDO_CB, "addo-cb", "addo.b", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addo.w [PC+],$Rd,ACR */
  {
    CRIS_INSN_ADDO_CW, "addo-cw", "addo.w", 32,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addo.d [PC+],$Rd,ACR */
  {
    CRIS_INSN_ADDO_CD, "addo-cd", "addo.d", 48,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* dip [${Rs}${inc}] */
  {
    CRIS_INSN_DIP_M, "dip-m", "dip", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* dip [PC+] */
  {
    CRIS_INSN_DIP_C, "dip-c", "dip", 48,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* addi-acr.b ${Rs-dfield}.m,${Rd-sfield},ACR */
  {
    CRIS_INSN_ADDI_ACR_B_R, "addi-acr.b-r", "addi-acr.b", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addi-acr.w ${Rs-dfield}.m,${Rd-sfield},ACR */
  {
    CRIS_INSN_ADDI_ACR_W_R, "addi-acr.w-r", "addi-acr.w", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* addi-acr.d ${Rs-dfield}.m,${Rd-sfield},ACR */
  {
    CRIS_INSN_ADDI_ACR_D_R, "addi-acr.d-r", "addi-acr.d", 16,
    { 0, { { { (1<<MACH_BASE), 0 } } } }
  },
/* biap-pc.b ${Rs-dfield}.m,PC */
  {
    CRIS_INSN_BIAP_PC_B_R, "biap-pc.b-r", "biap-pc.b", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* biap-pc.w ${Rs-dfield}.m,PC */
  {
    CRIS_INSN_BIAP_PC_W_R, "biap-pc.w-r", "biap-pc.w", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* biap-pc.d ${Rs-dfield}.m,PC */
  {
    CRIS_INSN_BIAP_PC_D_R, "biap-pc.d-r", "biap-pc.d", 16,
    { 0, { { { (1<<MACH_CRISV0)|(1<<MACH_CRISV3)|(1<<MACH_CRISV8)|(1<<MACH_CRISV10), 0 } } } }
  },
/* fidxi [$Rs] */
  {
    CRIS_INSN_FIDXI, "fidxi", "fidxi", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* fidxi [$Rs] */
  {
    CRIS_INSN_FTAGI, "ftagi", "fidxi", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* fidxd [$Rs] */
  {
    CRIS_INSN_FIDXD, "fidxd", "fidxd", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV32), 0 } } } }
  },
/* ftagd [$Rs] */
  {
    CRIS_INSN_FTAGD, "ftagd", "ftagd", 16,
    { 0|A(UNCOND_CTI), { { { (1<<MACH_CRISV32), 0 } } } }
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
static void cris_cgen_rebuild_tables (CGEN_CPU_TABLE *);

/* Subroutine of cris_cgen_cpu_open to look up a mach via its bfd name.  */

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

/* Subroutine of cris_cgen_cpu_open to build the hardware table.  */

static void
build_hw_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_HW_ENTRY *init = & cris_cgen_hw_table[0];
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

/* Subroutine of cris_cgen_cpu_open to build the hardware table.  */

static void
build_ifield_table (CGEN_CPU_TABLE *cd)
{
  cd->ifld_table = & cris_cgen_ifld_table[0];
}

/* Subroutine of cris_cgen_cpu_open to build the hardware table.  */

static void
build_operand_table (CGEN_CPU_TABLE *cd)
{
  int i;
  int machs = cd->machs;
  const CGEN_OPERAND *init = & cris_cgen_operand_table[0];
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

/* Subroutine of cris_cgen_cpu_open to build the hardware table.
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
  const CGEN_IBASE *ib = & cris_cgen_insn_table[0];
  CGEN_INSN *insns = xmalloc (MAX_INSNS * sizeof (CGEN_INSN));

  memset (insns, 0, MAX_INSNS * sizeof (CGEN_INSN));
  for (i = 0; i < MAX_INSNS; ++i)
    insns[i].base = &ib[i];
  cd->insn_table.init_entries = insns;
  cd->insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->insn_table.num_init_entries = MAX_INSNS;
}

/* Subroutine of cris_cgen_cpu_open to rebuild the tables.  */

static void
cris_cgen_rebuild_tables (CGEN_CPU_TABLE *cd)
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
	const CGEN_ISA *isa = & cris_cgen_isa_table[i];

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
	const CGEN_MACH *mach = & cris_cgen_mach_table[i];

	if (mach->insn_chunk_bitsize != 0)
	{
	  if (cd->insn_chunk_bitsize != 0 && cd->insn_chunk_bitsize != mach->insn_chunk_bitsize)
	    {
	      opcodes_error_handler
		(/* xgettext:c-format */
		 _("internal error: cris_cgen_rebuild_tables: "
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
cris_cgen_cpu_open (enum cgen_cpu_open_arg arg_type, ...)
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
	      lookup_mach_via_bfd_name (cris_cgen_mach_table, name);

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
	     _("internal error: cris_cgen_cpu_open: "
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
	 _("internal error: cris_cgen_cpu_open: no endianness specified"));
      abort ();
    }

  cd->isas = cgen_bitset_copy (isas);
  cd->machs = machs;
  cd->endian = endian;
  cd->insn_endian
    = (insn_endian == CGEN_ENDIAN_UNKNOWN ? endian : insn_endian);

  /* Table (re)builder.  */
  cd->rebuild_tables = cris_cgen_rebuild_tables;
  cris_cgen_rebuild_tables (cd);

  /* Default to not allowing signed overflow.  */
  cd->signed_overflow_ok_p = 0;
  
  return (CGEN_CPU_DESC) cd;
}

/* Cover fn to cris_cgen_cpu_open to handle the simple case of 1 isa, 1 mach.
   MACH_NAME is the bfd name of the mach.  */

CGEN_CPU_DESC
cris_cgen_cpu_open_1 (const char *mach_name, enum cgen_endian endian)
{
  return cris_cgen_cpu_open (CGEN_CPU_OPEN_BFDMACH, mach_name,
			       CGEN_CPU_OPEN_ENDIAN, endian,
			       CGEN_CPU_OPEN_END);
}

/* Close a cpu table.
   ??? This can live in a machine independent file, but there's currently
   no place to put this file (there's no libcgen).  libopcodes is the wrong
   place as some simulator ports use this but they don't use libopcodes.  */

void
cris_cgen_cpu_close (CGEN_CPU_DESC cd)
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

