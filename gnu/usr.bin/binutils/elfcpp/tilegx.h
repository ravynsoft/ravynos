// tilegx.h -- ELF definitions specific to EM_TILEGX  -*- C++ -*-

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
// Written by Jiong Wang (jiwang@tilera.com)

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

#ifndef ELFCPP_TILEGX_H
#define ELFCPP_TILEGX_H

namespace elfcpp
{

// Documentation is taken from
//   http://www.tilera.com/scm/docs/index.html

enum
{
  R_TILEGX_NONE = 0,
  R_TILEGX_64 = 1,
  R_TILEGX_32 = 2,
  R_TILEGX_16 = 3,
  R_TILEGX_8 = 4,
  R_TILEGX_64_PCREL = 5,
  R_TILEGX_32_PCREL = 6,
  R_TILEGX_16_PCREL = 7,
  R_TILEGX_8_PCREL = 8,
  R_TILEGX_HW0 = 9,
  R_TILEGX_HW1 = 10,
  R_TILEGX_HW2 = 11,
  R_TILEGX_HW3 = 12,
  R_TILEGX_HW0_LAST = 13,
  R_TILEGX_HW1_LAST = 14,
  R_TILEGX_HW2_LAST = 15,
  R_TILEGX_COPY = 16,
  R_TILEGX_GLOB_DAT = 17,
  R_TILEGX_JMP_SLOT = 18,
  R_TILEGX_RELATIVE = 19,
  R_TILEGX_BROFF_X1 = 20,
  R_TILEGX_JUMPOFF_X1 = 21,
  R_TILEGX_JUMPOFF_X1_PLT = 22,
  R_TILEGX_IMM8_X0 = 23,
  R_TILEGX_IMM8_Y0 = 24,
  R_TILEGX_IMM8_X1 = 25,
  R_TILEGX_IMM8_Y1 = 26,
  R_TILEGX_DEST_IMM8_X1 = 27,
  R_TILEGX_MT_IMM14_X1 = 28,
  R_TILEGX_MF_IMM14_X1 = 29,
  R_TILEGX_MMSTART_X0 = 30,
  R_TILEGX_MMEND_X0 = 31,
  R_TILEGX_SHAMT_X0 = 32,
  R_TILEGX_SHAMT_X1 = 33,
  R_TILEGX_SHAMT_Y0 = 34,
  R_TILEGX_SHAMT_Y1 = 35,
  R_TILEGX_IMM16_X0_HW0 = 36,
  R_TILEGX_IMM16_X1_HW0 = 37,
  R_TILEGX_IMM16_X0_HW1 = 38,
  R_TILEGX_IMM16_X1_HW1 = 39,
  R_TILEGX_IMM16_X0_HW2 = 40,
  R_TILEGX_IMM16_X1_HW2 = 41,
  R_TILEGX_IMM16_X0_HW3 = 42,
  R_TILEGX_IMM16_X1_HW3 = 43,
  R_TILEGX_IMM16_X0_HW0_LAST = 44,
  R_TILEGX_IMM16_X1_HW0_LAST = 45,
  R_TILEGX_IMM16_X0_HW1_LAST = 46,
  R_TILEGX_IMM16_X1_HW1_LAST = 47,
  R_TILEGX_IMM16_X0_HW2_LAST = 48,
  R_TILEGX_IMM16_X1_HW2_LAST = 49,
  R_TILEGX_IMM16_X0_HW0_PCREL = 50,
  R_TILEGX_IMM16_X1_HW0_PCREL = 51,
  R_TILEGX_IMM16_X0_HW1_PCREL = 52,
  R_TILEGX_IMM16_X1_HW1_PCREL = 53,
  R_TILEGX_IMM16_X0_HW2_PCREL = 54,
  R_TILEGX_IMM16_X1_HW2_PCREL = 55,
  R_TILEGX_IMM16_X0_HW3_PCREL = 56,
  R_TILEGX_IMM16_X1_HW3_PCREL = 57,
  R_TILEGX_IMM16_X0_HW0_LAST_PCREL = 58,
  R_TILEGX_IMM16_X1_HW0_LAST_PCREL = 59,
  R_TILEGX_IMM16_X0_HW1_LAST_PCREL = 60,
  R_TILEGX_IMM16_X1_HW1_LAST_PCREL = 61,
  R_TILEGX_IMM16_X0_HW2_LAST_PCREL = 62,
  R_TILEGX_IMM16_X1_HW2_LAST_PCREL = 63,
  R_TILEGX_IMM16_X0_HW0_GOT = 64,
  R_TILEGX_IMM16_X1_HW0_GOT = 65,

  R_TILEGX_IMM16_X0_HW0_PLT_PCREL = 66,
  R_TILEGX_IMM16_X1_HW0_PLT_PCREL = 67,
  R_TILEGX_IMM16_X0_HW1_PLT_PCREL = 68,
  R_TILEGX_IMM16_X1_HW1_PLT_PCREL = 69,
  R_TILEGX_IMM16_X0_HW2_PLT_PCREL = 70,
  R_TILEGX_IMM16_X1_HW2_PLT_PCREL = 71,

  R_TILEGX_IMM16_X0_HW0_LAST_GOT = 72,
  R_TILEGX_IMM16_X1_HW0_LAST_GOT = 73,
  R_TILEGX_IMM16_X0_HW1_LAST_GOT = 74,
  R_TILEGX_IMM16_X1_HW1_LAST_GOT = 75,
  R_TILEGX_IMM16_X0_HW0_TLS_GD = 78,
  R_TILEGX_IMM16_X1_HW0_TLS_GD = 79,
  R_TILEGX_IMM16_X0_HW0_TLS_LE = 80,
  R_TILEGX_IMM16_X1_HW0_TLS_LE = 81,
  R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE = 82,
  R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE = 83,
  R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE = 84,
  R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE = 85,
  R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD = 86,
  R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD = 87,
  R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD = 88,
  R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD = 89,
  R_TILEGX_IRELATIVE = 90,
  R_TILEGX_IMM16_X0_HW0_TLS_IE = 92,
  R_TILEGX_IMM16_X1_HW0_TLS_IE = 93,

  R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL = 94,
  R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL = 95,
  R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL = 96,
  R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL = 97,
  R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL = 98,
  R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL = 99,

  R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE = 100,
  R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE = 101,
  R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE = 102,
  R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE = 103,
  R_TILEGX_TLS_DTPMOD64 = 106,
  R_TILEGX_TLS_DTPOFF64 = 107,
  R_TILEGX_TLS_TPOFF64 = 108,
  R_TILEGX_TLS_DTPMOD32 = 109,
  R_TILEGX_TLS_DTPOFF32 = 110,
  R_TILEGX_TLS_TPOFF32 = 111,
  R_TILEGX_TLS_GD_CALL = 112,
  R_TILEGX_IMM8_X0_TLS_GD_ADD = 113,
  R_TILEGX_IMM8_X1_TLS_GD_ADD = 114,
  R_TILEGX_IMM8_Y0_TLS_GD_ADD = 115,
  R_TILEGX_IMM8_Y1_TLS_GD_ADD = 116,
  R_TILEGX_TLS_IE_LOAD = 117,
  R_TILEGX_IMM8_X0_TLS_ADD = 118,
  R_TILEGX_IMM8_X1_TLS_ADD = 119,
  R_TILEGX_IMM8_Y0_TLS_ADD = 120,
  R_TILEGX_IMM8_Y1_TLS_ADD = 121,
  R_TILEGX_GNU_VTINHERIT = 128,
  R_TILEGX_GNU_VTENTRY = 129,
  R_TILEGX_NUM = 130
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_TILEGX_H)
