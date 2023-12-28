// dwarf.h -- DWARF2 constants  -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of elfcpp.

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// In addition to the permissions in the GNU Library General Public
// License, the Free Software Foundation gives you unlimited
// permission to link the compiled version of this file into
// combinations with other programs, and to distribute those
// combinations without any restriction coming from the use of this
// file.  (The Library Public License restrictions do apply in other
// respects; for example, they cover modification of the file, and
/// distribution when not linked into a combined executable.)

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef ELFCPP_DWARF_H
#define ELFCPP_DWARF_H

namespace elfcpp
{

// DWARF2 codes.

// Include DW_TAG, DW_FORM, DW_AT, DW_OP, DW_ATE, and DW_CFA
// codes from dwarf2.def.

#define DW_FIRST_TAG(name, value) enum DW_TAG { \
  name = value
#define DW_TAG(name, value) , name = value
#define DW_TAG_DUP(name, value) , name = value
#define DW_END_TAG };

#define DW_FIRST_FORM(name, value) enum DW_FORM { \
  name = value
#define DW_FORM(name, value) , name = value
#define DW_END_FORM };

#define DW_FIRST_AT(name, value) enum DW_AT { \
  name = value
#define DW_AT(name, value) , name = value
#define DW_AT_DUP(name, value) , name = value
#define DW_END_AT };

#define DW_FIRST_OP(name, value) enum DW_OP { \
  name = value
#define DW_OP(name, value) , name = value
#define DW_OP_DUP(name, value) , name = value
#define DW_END_OP };

#define DW_FIRST_ATE(name, value) enum DW_ENCODING { \
  name = value
#define DW_ATE(name, value) , name = value
#define DW_ATE_DUP(name, value) , name = value
#define DW_END_ATE };

#define DW_FIRST_CFA(name, value) enum DW_CFA { \
  name = value
#define DW_CFA(name, value) , name = value
#define DW_CFA_DUP(name, value) , name = value
#define DW_END_CFA };

#define DW_FIRST_IDX(name, value) enum dwarf_name_index_attribute { \
  name = value
#define DW_IDX(name, value) , name = value
#define DW_IDX_DUP(name, value) , name = value
#define DW_END_IDX };

#define DW_FIRST_UT(name, value) enum dwarf_unit_type { \
  name = value
#define DW_UT(name, value) , name = value
#define DW_END_UT };

#include "dwarf2.def"

#undef DW_FIRST_TAG
#undef DW_TAG
#undef DW_TAG_DUP
#undef DW_END_TAG

#undef DW_FIRST_FORM
#undef DW_FORM
#undef DW_END_FORM

#undef DW_FIRST_AT
#undef DW_AT
#undef DW_AT_DUP
#undef DW_END_AT

#undef DW_FIRST_OP
#undef DW_OP
#undef DW_OP_DUP
#undef DW_END_OP

#undef DW_FIRST_ATE
#undef DW_ATE
#undef DW_ATE_DUP
#undef DW_END_ATE

#undef DW_FIRST_CFA
#undef DW_CFA_DUP
#undef DW_CFA
#undef DW_END_CFA

#undef DW_FIRST_IDX
#undef DW_IDX
#undef DW_IDX_DUP
#undef DW_END_IDX

#undef DW_FIRST_UT
#undef DW_UT
#undef DW_END_UT

// Frame unwind information.

enum DW_EH_PE
{
  DW_EH_PE_absptr = 0x00,
  DW_EH_PE_omit = 0xff,

  DW_EH_PE_uleb128 = 0x01,
  DW_EH_PE_udata2 = 0x02,
  DW_EH_PE_udata4 = 0x03,
  DW_EH_PE_udata8 = 0x04,
  DW_EH_PE_signed = 0x08,
  DW_EH_PE_sleb128 = 0x09,
  DW_EH_PE_sdata2 = 0x0a,
  DW_EH_PE_sdata4 = 0x0b,
  DW_EH_PE_sdata8 = 0x0c,

  DW_EH_PE_pcrel = 0x10,
  DW_EH_PE_textrel = 0x20,
  DW_EH_PE_datarel = 0x30,
  DW_EH_PE_funcrel = 0x40,
  DW_EH_PE_aligned = 0x50,

  DW_EH_PE_indirect = 0x80
};

// Line number table content type codes.

enum DW_LNCT
{
  DW_LNCT_path            = 0x1,
  DW_LNCT_directory_index = 0x2,
  DW_LNCT_timestamp       = 0x3,
  DW_LNCT_size            = 0x4,
  DW_LNCT_MD5             = 0x5,
  DW_LNCT_lo_user         = 0x2000,
  DW_LNCT_hi_user         = 0x3fff
};

// Line number opcodes.

enum DW_LINE_OPS
{
  DW_LNS_extended_op        = 0x00,
  DW_LNS_copy               = 0x01,
  DW_LNS_advance_pc         = 0x02,
  DW_LNS_advance_line       = 0x03,
  DW_LNS_set_file           = 0x04,
  DW_LNS_set_column         = 0x05,
  DW_LNS_negate_stmt        = 0x06,
  DW_LNS_set_basic_block    = 0x07,
  DW_LNS_const_add_pc       = 0x08,
  DW_LNS_fixed_advance_pc   = 0x09,
  // DWARF 3.
  DW_LNS_set_prologue_end   = 0x0a,
  DW_LNS_set_epilogue_begin = 0x0b,
  DW_LNS_set_isa            = 0x0c
};

// Line number extended opcodes.

enum DW_LINE_EXTENDED_OPS
{
  DW_LNE_end_sequence                = 0x01,
  DW_LNE_set_address                 = 0x02,
  DW_LNE_define_file                 = 0x03,
  // DWARF4.
  DW_LNE_set_discriminator           = 0x04,
  // HP extensions.
  DW_LNE_HP_negate_is_UV_update      = 0x11,
  DW_LNE_HP_push_context             = 0x12,
  DW_LNE_HP_pop_context              = 0x13,
  DW_LNE_HP_set_file_line_column     = 0x14,
  DW_LNE_HP_set_routine_name         = 0x15,
  DW_LNE_HP_set_sequence             = 0x16,
  DW_LNE_HP_negate_post_semantics    = 0x17,
  DW_LNE_HP_negate_function_exit     = 0x18,
  DW_LNE_HP_negate_front_end_logical = 0x19,
  DW_LNE_HP_define_proc              = 0x20,
  DW_LNE_lo_user                     = 0x80,
  DW_LNE_hi_user                     = 0xff
};

enum DW_CHILDREN
{
  DW_CHILDREN_no  = 0,
  DW_CHILDREN_yes = 1
};

// Source language names and codes.
enum DW_LANG
{
  DW_LANG_C89 = 0x0001,
  DW_LANG_C = 0x0002,
  DW_LANG_Ada83 = 0x0003,
  DW_LANG_C_plus_plus = 0x0004,
  DW_LANG_Cobol74 = 0x0005,
  DW_LANG_Cobol85 = 0x0006,
  DW_LANG_Fortran77 = 0x0007,
  DW_LANG_Fortran90 = 0x0008,
  DW_LANG_Pascal83 = 0x0009,
  DW_LANG_Modula2 = 0x000a,
  // DWARF 3.
  DW_LANG_Java = 0x000b,
  DW_LANG_C99 = 0x000c,
  DW_LANG_Ada95 = 0x000d,
  DW_LANG_Fortran95 = 0x000e,
  DW_LANG_PLI = 0x000f,
  DW_LANG_ObjC = 0x0010,
  DW_LANG_ObjC_plus_plus = 0x0011,
  DW_LANG_UPC = 0x0012,
  DW_LANG_D = 0x0013,
  // DWARF 4.
  DW_LANG_Python = 0x0014,
  // DWARF 5.
  DW_LANG_Go = 0x0016,
  DW_LANG_C_plus_plus_11 = 0x001a,
  DW_LANG_C11 = 0x001d,
  DW_LANG_C_plus_plus_14 = 0x0021,
  DW_LANG_Fortran03 = 0x0022,
  DW_LANG_Fortran08 = 0x0023,

  DW_LANG_lo_user = 0x8000,	// Implementation-defined range start.
  DW_LANG_hi_user = 0xffff,	// Implementation-defined range start.
  // MIPS.
  DW_LANG_Mips_Assembler = 0x8001,
  // UPC.
  DW_LANG_Upc = 0x8765,
  // HP extensions.
  DW_LANG_HP_Bliss     = 0x8003,
  DW_LANG_HP_Basic91   = 0x8004,
  DW_LANG_HP_Pascal91  = 0x8005,
  DW_LANG_HP_IMacro    = 0x8006,
  DW_LANG_HP_Assembler = 0x8007
};

// Range list entry kinds in .debug_rnglists* section.

enum DW_RLE
{
  DW_RLE_end_of_list   = 0x00,
  DW_RLE_base_addressx = 0x01,
  DW_RLE_startx_endx   = 0x02,
  DW_RLE_startx_length = 0x03,
  DW_RLE_offset_pair   = 0x04,
  DW_RLE_base_address  = 0x05,
  DW_RLE_start_end     = 0x06,
  DW_RLE_start_length  = 0x07
};

// DWARF section identifiers used in the package format.
// Extensions for Fission.  See http://gcc.gnu.org/wiki/DebugFissionDWP.
// Added (with changes) in DWARF 5.

enum DW_SECT
{
  DW_SECT_INFO        = 1,
  DW_SECT_ABBREV      = 3,
  DW_SECT_LINE        = 4,
  DW_SECT_LOCLISTS    = 5,
  DW_SECT_STR_OFFSETS = 6,
  DW_SECT_MACINFO     = 7,
  DW_SECT_RNGLISTS    = 8,
  DW_SECT_MAX = DW_SECT_RNGLISTS,
  // These were used only for the experimental Fission support in DWARF 4.
  DW_SECT_TYPES       = 2,
  DW_SECT_LOC         = 5,
  DW_SECT_MACRO       = 8
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_DWARF_H)
