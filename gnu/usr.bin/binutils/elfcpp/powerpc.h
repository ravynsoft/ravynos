// powerpc.h -- ELF definitions specific to EM_PPC and EM_PPC64  -*- C++ -*-

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by David S. Miller <davem@davemloft.net>.

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

#ifndef ELFCPP_POWERPC_H
#define ELFCPP_POWERPC_H

namespace elfcpp
{

// The relocation numbers for 32-bit and 64-bit powerpc are nearly
// identical.  Therefore I've adopted the convention of using
// R_POWERPC_foo for values which are the same in R_PPC_* and R_PPC64_*.
// For relocations which are specific to the word size I will use
// R_PPC_foo or R_PPC64_foo.
enum
{
  R_POWERPC_NONE = 0,
  R_POWERPC_ADDR32 = 1,
  R_POWERPC_ADDR24 = 2,
  R_POWERPC_ADDR16 = 3,
  R_POWERPC_ADDR16_LO = 4,
  R_POWERPC_ADDR16_HI = 5,
  R_POWERPC_ADDR16_HA = 6,
  R_POWERPC_ADDR14 = 7,
  R_POWERPC_ADDR14_BRTAKEN = 8,
  R_POWERPC_ADDR14_BRNTAKEN = 9,
  R_POWERPC_REL24 = 10,
  R_POWERPC_REL14 = 11,
  R_POWERPC_REL14_BRTAKEN = 12,
  R_POWERPC_REL14_BRNTAKEN = 13,
  R_POWERPC_GOT16 = 14,
  R_POWERPC_GOT16_LO = 15,
  R_POWERPC_GOT16_HI = 16,
  R_POWERPC_GOT16_HA = 17,
  R_PPC_PLTREL24 = 18,
  R_POWERPC_COPY = 19,
  R_POWERPC_GLOB_DAT = 20,
  R_POWERPC_JMP_SLOT = 21,
  R_POWERPC_RELATIVE = 22,
  R_PPC_LOCAL24PC = 23,
  R_POWERPC_UADDR32 = 24,
  R_POWERPC_UADDR16 = 25,
  R_POWERPC_REL32 = 26,
  R_POWERPC_PLT32 = 27,
  R_POWERPC_PLTREL32 = 28,
  R_POWERPC_PLT16_LO = 29,
  R_POWERPC_PLT16_HI = 30,
  R_POWERPC_PLT16_HA = 31,
  R_PPC_SDAREL16 = 32,
  R_POWERPC_SECTOFF = 33,
  R_POWERPC_SECTOFF_LO = 34,
  R_POWERPC_SECTOFF_HI = 35,
  R_POWERPC_SECTOFF_HA = 36,
  R_POWERPC_ADDR30 = 37,
  R_PPC64_ADDR64 = 38,
  R_PPC64_ADDR16_HIGHER = 39,
  R_PPC64_ADDR16_HIGHERA = 40,
  R_PPC64_ADDR16_HIGHEST = 41,
  R_PPC64_ADDR16_HIGHESTA = 42,
  R_PPC64_UADDR64 = 43,
  R_PPC64_REL64 = 44,
  R_PPC64_PLT64 = 45,
  R_PPC64_PLTREL64 = 46,
  R_PPC64_TOC16 = 47,
  R_PPC64_TOC16_LO = 48,
  R_PPC64_TOC16_HI = 49,
  R_PPC64_TOC16_HA = 50,
  R_PPC64_TOC = 51,
  R_PPC64_PLTGOT16 = 52,
  R_PPC64_PLTGOT16_LO = 53,
  R_PPC64_PLTGOT16_HI = 54,
  R_PPC64_PLTGOT16_HA = 55,
  R_PPC64_ADDR16_DS = 56,
  R_PPC64_ADDR16_LO_DS = 57,
  R_PPC64_GOT16_DS = 58,
  R_PPC64_GOT16_LO_DS = 59,
  R_PPC64_PLT16_LO_DS = 60,
  R_PPC64_SECTOFF_DS = 61,
  R_PPC64_SECTOFF_LO_DS = 62,
  R_PPC64_TOC16_DS = 63,
  R_PPC64_TOC16_LO_DS = 64,
  R_PPC64_PLTGOT16_DS = 65,
  R_PPC64_PLTGOT16_LO_DS = 66,
  R_POWERPC_TLS = 67,
  R_POWERPC_DTPMOD = 68,
  R_POWERPC_TPREL16 = 69,
  R_POWERPC_TPREL16_LO = 70,
  R_POWERPC_TPREL16_HI = 71,
  R_POWERPC_TPREL16_HA = 72,
  R_POWERPC_TPREL = 73,
  R_POWERPC_DTPREL16 = 74,
  R_POWERPC_DTPREL16_LO = 75,
  R_POWERPC_DTPREL16_HI = 76,
  R_POWERPC_DTPREL16_HA = 77,
  R_POWERPC_DTPREL = 78,
  R_POWERPC_GOT_TLSGD16 = 79,
  R_POWERPC_GOT_TLSGD16_LO = 80,
  R_POWERPC_GOT_TLSGD16_HI = 81,
  R_POWERPC_GOT_TLSGD16_HA = 82,
  R_POWERPC_GOT_TLSLD16 = 83,
  R_POWERPC_GOT_TLSLD16_LO = 84,
  R_POWERPC_GOT_TLSLD16_HI = 85,
  R_POWERPC_GOT_TLSLD16_HA = 86,
  R_POWERPC_GOT_TPREL16 = 87,
  R_POWERPC_GOT_TPREL16_LO = 88,
  R_POWERPC_GOT_TPREL16_HI = 89,
  R_POWERPC_GOT_TPREL16_HA = 90,
  R_POWERPC_GOT_DTPREL16 = 91,
  R_POWERPC_GOT_DTPREL16_LO = 92,
  R_POWERPC_GOT_DTPREL16_HI = 93,
  R_POWERPC_GOT_DTPREL16_HA = 94,
  R_PPC_TLSGD = 95,
  R_PPC64_TPREL16_DS = 95,
  R_PPC_TLSLD = 96,
  R_PPC64_TPREL16_LO_DS = 96,
  R_PPC64_TPREL16_HIGHER = 97,
  R_PPC64_TPREL16_HIGHERA = 98,
  R_PPC64_TPREL16_HIGHEST = 99,
  R_PPC64_TPREL16_HIGHESTA = 100,
  R_PPC_EMB_NADDR32 = 101,
  R_PPC64_DTPREL16_DS = 101,
  R_PPC_EMB_NADDR16 = 102,
  R_PPC64_DTPREL16_LO_DS = 102,
  R_PPC_EMB_NADDR16_LO = 103,
  R_PPC64_DTPREL16_HIGHER = 103,
  R_PPC_EMB_NADDR16_HI = 104,
  R_PPC64_DTPREL16_HIGHERA = 104,
  R_PPC_EMB_NADDR16_HA = 105,
  R_PPC64_DTPREL16_HIGHEST = 105,
  R_PPC_EMB_SDAI16 = 106,
  R_PPC64_DTPREL16_HIGHESTA = 106,
  R_PPC_EMB_SDA2I16 = 107,
  R_PPC64_TLSGD = 107,
  R_PPC_EMB_SDA2REL = 108,
  R_PPC64_TLSLD = 108,
  R_PPC_EMB_SDA21 = 109,
  R_PPC64_TOCSAVE = 109,
  R_PPC_EMB_MRKREF = 110,
  R_PPC64_ADDR16_HIGH = 110,
  R_PPC_EMB_RELSEC16 = 111,
  R_PPC64_ADDR16_HIGHA = 111,
  R_PPC_EMB_RELST_LO = 112,
  R_PPC64_TPREL16_HIGH = 112,
  R_PPC_EMB_RELST_HI = 113,
  R_PPC64_TPREL16_HIGHA = 113,
  R_PPC_EMB_RELST_HA = 114,
  R_PPC64_DTPREL16_HIGH = 114,
  R_PPC_EMB_BIT_FLD = 115,
  R_PPC64_DTPREL16_HIGHA = 115,
  R_PPC_EMB_RELSDA = 116,
  R_PPC64_REL24_NOTOC = 116,
  R_PPC64_ADDR64_LOCAL = 117,
  R_PPC64_ENTRY = 118,
  R_POWERPC_PLTSEQ = 119,
  R_POWERPC_PLTCALL = 120,
  R_PPC64_PLTSEQ_NOTOC = 121,
  R_PPC64_PLTCALL_NOTOC = 122,
  R_PPC64_PCREL_OPT = 123,
  R_PPC64_REL24_P9NOTOC = 124,

  R_PPC64_D34 = 128,
  R_PPC64_D34_LO = 129,
  R_PPC64_D34_HI30 = 130,
  R_PPC64_D34_HA30 = 131,
  R_PPC64_PCREL34 = 132,
  R_PPC64_GOT_PCREL34 = 133,
  R_PPC64_PLT_PCREL34 = 134,
  R_PPC64_PLT_PCREL34_NOTOC = 135,
  R_PPC64_ADDR16_HIGHER34 = 136,
  R_PPC64_ADDR16_HIGHERA34 = 137,
  R_PPC64_ADDR16_HIGHEST34 = 138,
  R_PPC64_ADDR16_HIGHESTA34 = 139,
  R_PPC64_REL16_HIGHER34 = 140,
  R_PPC64_REL16_HIGHERA34 = 141,
  R_PPC64_REL16_HIGHEST34 = 142,
  R_PPC64_REL16_HIGHESTA34 = 143,
  R_PPC64_D28 = 144,
  R_PPC64_PCREL28 = 145,
  R_PPC64_TPREL34 = 146,
  R_PPC64_DTPREL34 = 147,
  R_PPC64_GOT_TLSGD_PCREL34 = 148,
  R_PPC64_GOT_TLSLD_PCREL34 = 149,
  R_PPC64_GOT_TPREL_PCREL34 = 150,
  R_PPC64_GOT_DTPREL_PCREL34 = 151,

  R_PPC_VLE_REL8 = 216,
  R_PPC_VLE_REL15 = 217,
  R_PPC_VLE_REL24 = 218,
  R_PPC_VLE_LO16A = 219,
  R_PPC_VLE_LO16D = 220,
  R_PPC_VLE_HI16A = 221,
  R_PPC_VLE_HI16D = 222,
  R_PPC_VLE_HA16A = 223,
  R_PPC_VLE_HA16D = 224,
  R_PPC_VLE_SDA21 = 225,
  R_PPC_VLE_SDA21_LO = 226,
  R_PPC_VLE_SDAREL_LO16A = 227,
  R_PPC_VLE_SDAREL_LO16D = 228,
  R_PPC_VLE_SDAREL_HI16A = 229,
  R_PPC_VLE_SDAREL_HI16D = 230,
  R_PPC_VLE_SDAREL_HA16A = 231,
  R_PPC_VLE_SDAREL_HA16D = 232,

  R_PPC64_REL16_HIGH = 240,
  R_PPC64_REL16_HIGHA = 241,
  R_PPC64_REL16_HIGHER = 242,
  R_PPC64_REL16_HIGHERA = 243,
  R_PPC64_REL16_HIGHEST = 244,
  R_PPC64_REL16_HIGHESTA = 245,

  R_POWERPC_REL16DX_HA = 246,
  R_PPC64_JMP_IREL = 247,
  R_POWERPC_IRELATIVE = 248,
  R_POWERPC_REL16 = 249,
  R_POWERPC_REL16_LO = 250,
  R_POWERPC_REL16_HI = 251,
  R_POWERPC_REL16_HA = 252,
  R_POWERPC_GNU_VTINHERIT = 253,
  R_POWERPC_GNU_VTENTRY = 254,
  R_PPC_TOC16 = 255,
};

// e_flags values defined for powerpc
enum
{
  EF_PPC_EMB = 0x80000000,             // PowerPC embedded flag.
  EF_PPC_RELOCATABLE = 0x00010000,     // PowerPC -mrelocatable flag.  */
  EF_PPC_RELOCATABLE_LIB = 0x00008000, // PowerPC -mrelocatable-lib flag.  */
};

// e_flags values defined for powerpc64
enum
{
  // ABI version
  // 1 for original function descriptor using ABI,
  // 2 for revised ABI without function descriptors,
  // 0 for unspecified or not using any features affected by the differences.
  EF_PPC64_ABI = 3
};

// Object attribute tags.  0-3 are generic.
enum
{
  // FP ABI, low 2 bits:
  // 1 for double precision hard-float,
  // 2 for soft-float,
  // 3 for single precision hard-float.
  // 0 for not tagged or not using any ABIs affected by the differences.
  // Next 2 bits:
  // 1 for ibm long double
  // 2 for 64-bit long double
  // 3 for IEEE long double.
  // 0 for not tagged or not using any ABIs affected by the differences.
  Tag_GNU_Power_ABI_FP = 4,

  // Value 1 for general purpose registers only, 2 for AltiVec
  // registers, 3 for SPE registers; 0 for not tagged or not using any
  // ABIs affected by the differences.
  Tag_GNU_Power_ABI_Vector = 8,

  // Value 1 for ABIs using r3/r4 for returning structures <= 8 bytes,
  // 2 for ABIs using memory; 0 for not tagged or not using any ABIs
  // affected by the differences.
  Tag_GNU_Power_ABI_Struct_Return = 12
};

// DT_PPC_OPT bits
enum
{
  PPC_OPT_TLS = 1
};

// DT_PPC64_OPT bits
enum
{
  PPC64_OPT_TLS = 1,
  PPC64_OPT_MULTI_TOC = 2,
  PPC64_OPT_LOCALENTRY = 4
};

enum
{
  // The ELFv2 ABI uses three bits in the symbol st_other field of a
  // function definition to specify the number of instructions between a
  // function's global entry point and local entry point.
  // The global entry point is used when it is necessary to set up the
  // toc pointer (r2) for the function.  Callers must enter the global
  // entry point with r12 set to the global entry point address.  On
  // return from the function, r2 may have a different value to that
  // which it had on entry.
  // The local entry point is used when r2 is known to already be valid
  // for the function.  There is no requirement on r12 when using the
  // local entry point, and on return r2 will contain the same value as
  // at entry.
  // A value of zero in these bits means that the function has a single
  // entry point with no requirement on r12 or r2, and that on return r2
  // will contain the same value as at entry.
  // Values of one and seven are reserved.

  STO_PPC64_LOCAL_BIT = 5,
  STO_PPC64_LOCAL_MASK = 0xE0
};

// 3 bit other field to bytes.
static inline unsigned int
ppc64_decode_local_entry(unsigned int other)
{
  return ((1 << other) >> 2) << 2;
}

// bytes to field value.
static inline unsigned int
ppc64_encode_local_entry(unsigned int val)
{
  return (val >= 4 * 4
	  ? (val >= 8 * 4
	     ? (val >= 16 * 4 ? 6 : 5)
	     : 4)
	  : (val >= 2 * 4
	     ? 3
	     : (val >= 1 * 4 ? 2 : 0)));
}

} // End namespace elfcpp.

#endif // !defined(ELFCPP_POWERPC_H)
