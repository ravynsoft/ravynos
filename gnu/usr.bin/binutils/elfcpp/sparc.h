// sparc.h -- ELF definitions specific to EM_SPARC  -*- C++ -*-

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

#ifndef ELFCPP_SPARC_H
#define ELFCPP_SPARC_H

// Documentation for the TLS relocs is taken from
//   http://people.redhat.com/drepper/tls.pdf
//
// More full documentation on sparc specific ELF file
// format details can be found at
//
//   http://docs.sun.com/app/docs/doc/819/0690/
//   "Linker and Libraries Guide"
//
// specifically Chapter 7 "Object File Format" and
// Chapter 8 "Thread-Local Storage"

namespace elfcpp
{

enum
{
  R_SPARC_NONE = 0,           // No reloc
  R_SPARC_8 = 1,              // Direct 8 bit
  R_SPARC_16 = 2,             // Direct 16 bit
  R_SPARC_32 = 3,             // Direct 32 bit
  R_SPARC_DISP8 = 4,          // PC relative 8 bit
  R_SPARC_DISP16 = 5,         // PC relative 16 bit
  R_SPARC_DISP32 = 6,         // PC relative 32 bit
  R_SPARC_WDISP30 = 7,        // PC relative 30 bit shifted
  R_SPARC_WDISP22 = 8,        // PC relative 22 bit shifted
  R_SPARC_HI22 = 9,           // High 22 bit
  R_SPARC_22 = 10,            // Direct 22 bit
  R_SPARC_13 = 11,            // Direct 13 bit
  R_SPARC_LO10 = 12,          // Truncated 10 bit
  R_SPARC_GOT10 = 13,         // Truncated 10 bit GOT entry
  R_SPARC_GOT13 = 14,         // 13 bit GOT entry
  R_SPARC_GOT22 = 15,         // 22 bit GOT entry shifted
  R_SPARC_PC10 = 16,          // PC relative 10 bit truncated
  R_SPARC_PC22 = 17,          // PC relative 22 bit shifted
  R_SPARC_WPLT30 = 18,        // 30 bit PC relative PLT address
  R_SPARC_COPY = 19,          // Copy symbol at runtime
  R_SPARC_GLOB_DAT = 20,      // Create GOT entry
  R_SPARC_JMP_SLOT = 21,      // Create PLT entry
  R_SPARC_RELATIVE = 22,      // Adjust by program base
  R_SPARC_UA32 = 23,          // Direct 32 bit unaligned
  R_SPARC_PLT32 = 24,         // Direct 32 bit ref to PLT entry
  R_SPARC_HIPLT22 = 25,       // High 22 bit PLT entry
  R_SPARC_LOPLT10 = 26,       // Truncated 10 bit PLT entry
  R_SPARC_PCPLT32 = 27,       // PC rel 32 bit ref to PLT entry
  R_SPARC_PCPLT22 = 28,       // PC rel high 22 bit PLT entry
  R_SPARC_PCPLT10 = 29,       // PC rel trunc 10 bit PLT entry
  R_SPARC_10 = 30,            // Direct 10 bit
  R_SPARC_11 = 31,            // Direct 11 bit
  R_SPARC_64 = 32,            // Direct 64 bit
  R_SPARC_OLO10 = 33,         // 10bit with secondary 13bit addend
  R_SPARC_HH22 = 34,          // Top 22 bits of direct 64 bit
  R_SPARC_HM10 = 35,          // High middle 10 bits of ...
  R_SPARC_LM22 = 36,          // Low middle 22 bits of ...
  R_SPARC_PC_HH22 = 37,       // Top 22 bits of pc rel 64 bit
  R_SPARC_PC_HM10 = 38,       // High middle 10 bit of ...
  R_SPARC_PC_LM22 = 39,       // Low miggle 22 bits of ...
  R_SPARC_WDISP16 = 40,       // PC relative 16 bit shifted
  R_SPARC_WDISP19 = 41,       // PC relative 19 bit shifted
  R_SPARC_GLOB_JMP = 42,      // was part of v9 ABI but was removed
  R_SPARC_7 = 43,             // Direct 7 bit
  R_SPARC_5 = 44,             // Direct 5 bit
  R_SPARC_6 = 45,             // Direct 6 bit
  R_SPARC_DISP64 = 46,        // PC relative 64 bit
  R_SPARC_PLT64 = 47,         // Direct 64 bit ref to PLT entry
  R_SPARC_HIX22 = 48,         // High 22 bit complemented
  R_SPARC_LOX10 = 49,         // Truncated 11 bit complemented
  R_SPARC_H44 = 50,           // Direct high 12 of 44 bit
  R_SPARC_M44 = 51,           // Direct mid 22 of 44 bit
  R_SPARC_L44 = 52,           // Direct low 10 of 44 bit
  R_SPARC_REGISTER = 53,      // Global register usage
  R_SPARC_UA64 = 54,          // Direct 64 bit unaligned
  R_SPARC_UA16 = 55,          // Direct 16 bit unaligned
  R_SPARC_TLS_GD_HI22 = 56,   // Initial General Dynamic reloc, high 22-bit
  R_SPARC_TLS_GD_LO10 = 57,   // Initial General Dynamic reloc, low 10-bit
  R_SPARC_TLS_GD_ADD = 58,    // Initial General Dynamic reloc, add
  R_SPARC_TLS_GD_CALL = 59,   // Initial General Dynamic reloc, call
  R_SPARC_TLS_LDM_HI22 = 60,  // Initial Local Dynamic reloc, high 22-bit
  R_SPARC_TLS_LDM_LO10 = 61,  // Initial Local Dynamic reloc, low 10-bit
  R_SPARC_TLS_LDM_ADD = 62,   // Initial Local Dynamic reloc, add
  R_SPARC_TLS_LDM_CALL = 63,  // Initial Local Dynamic reloc, call
  R_SPARC_TLS_LDO_HIX22 = 64, // Initial Local Dynamic, high extended 22-bit
  R_SPARC_TLS_LDO_LOX10 = 65, // Initial Local Dynamic, low extended 10-bit
  R_SPARC_TLS_LDO_ADD = 66,   // Initial Local Dynamic, add extended
  R_SPARC_TLS_IE_HI22 = 67,   // Initial Initial Exec reloc, high 22-bit
  R_SPARC_TLS_IE_LO10 = 68,   // Initial Initial Exec reloc, low 10-bit
  R_SPARC_TLS_IE_LD = 69,     // Initial Initial Exec reloc, load 32-bit
  R_SPARC_TLS_IE_LDX = 70,    // Initial Initial Exec reloc, load 64-bit
  R_SPARC_TLS_IE_ADD = 71,    // Initial Initial Exec reloc, add
  R_SPARC_TLS_LE_HIX22 = 72,  // Initial Local Exec reloc, high extended 22-bit
  R_SPARC_TLS_LE_LOX10 = 73,  // Initial Local Exec reloc, low extended 10-bit
  R_SPARC_TLS_DTPMOD32 = 74,  // Outstanding General/Local Dynamic reloc, 32-bit
  R_SPARC_TLS_DTPMOD64 = 75,  // Outstanding General/Local Dynamic reloc, 64-bit
  R_SPARC_TLS_DTPOFF32 = 76,  // Outstanding General Dynamic reloc, 32-bit
  R_SPARC_TLS_DTPOFF64 = 77,  // Outstanding General Dynamic reloc, 64-bit
  R_SPARC_TLS_TPOFF32 = 78,   // Outstanding Initial Exec reloc, 32-bit
  R_SPARC_TLS_TPOFF64 = 79,   // Outstanding Initial Exec reloc, 64-bit

  // GOT data code transformations
  R_SPARC_GOTDATA_HIX22 = 80,
  R_SPARC_GOTDATA_LOX10 = 81,
  R_SPARC_GOTDATA_OP_HIX22 = 82,
  R_SPARC_GOTDATA_OP_LOX10 = 83,
  R_SPARC_GOTDATA_OP = 84,

  R_SPARC_H34 = 85,           // Direct high 12 of 34 bit
  R_SPARC_SIZE32 = 86,        // size of symbol, 32-bit
  R_SPARC_SIZE64 = 87,        // size of symbol, 64-bit
  R_SPARC_WDISP10 = 88,       // PC relative 10 bit shifted

  R_SPARC_JMP_IREL = 248,     // Create PLT slot to IFUNC function
  R_SPARC_IRELATIVE = 249,    // Adjust indirectly by program base

  // GNU vtable garbage collection extensions.
  R_SPARC_GNU_VTINHERIT = 250,
  R_SPARC_GNU_VTENTRY = 251,

  R_SPARC_REV32 = 252,
};

// e_flags values defined for sparc
enum
{
  EF_SPARC_EXT_MASK = 0xffff00,    // reserved for vendor extensions
  EF_SPARC_32PLUS_MASK = 0xffff00, // bits indicating V8+ type
  EF_SPARC_32PLUS = 0x000100,      // generic V8+ features
  EF_SPARC_SUN_US1 = 0x000200,     // Sun UltraSPARC-I extensions
  EF_SPARC_HAL_R1 = 0x000400,      // HAL R1 extensions
  EF_SPARC_SUN_US3 = 0x000800,     // Sun UltraSPARC-III extensions
  EF_SPARC_LEDATA = 0x800000,      // little endian data
  EF_SPARCV9_MM = 0x3,             // memory model mask
  EF_SPARCV9_TSO = 0x0,            // total store ordering
  EF_SPARCV9_PSO = 0x1,            // partial store ordering
  EF_SPARCV9_RMO = 0x2,            // relaxed store ordering
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_SPARC_H)
