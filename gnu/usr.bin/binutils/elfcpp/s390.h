// s390.h -- ELF definitions specific to EM_S390  -*- C++ -*-

// Copyright (C) 2015-2023 Free Software Foundation, Inc.
// Written by Marcin Kościelnicki <koriakin@0x04.net>.

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

#ifndef ELFCPP_S390_H
#define ELFCPP_S390_H

namespace elfcpp
{

enum
{
  // Original 31-bit ABI.
  R_390_NONE = 0,           // No reloc.
  R_390_8 = 1,              // Direct 8 bit.
  R_390_12 = 2,             // Direct 12 bit.
  R_390_16 = 3,             // Direct 16 bit.
  R_390_32 = 4,             // Direct 32 bit.
  R_390_PC32 = 5,           // PC relative 32 bit.
  R_390_GOT12 = 6,          // 12 bit GOT offset.
  R_390_GOT32 = 7,          // 32 bit GOT offset.
  R_390_PLT32 = 8,          // 32 bit PC relative PLT address.
  R_390_COPY = 9,           // Copy symbol at runtime.
  R_390_GLOB_DAT = 10,      // Create GOT entry.
  R_390_JMP_SLOT = 11,      // Create PLT entry.
  R_390_RELATIVE = 12,      // Adjust by program base.
  R_390_GOTOFF32 = 13,      // 32 bit offset to GOT.
  R_390_GOTPC = 14,         // 32 bit PC relative offset to GOT.
  R_390_GOT16 = 15,         // 16 bit GOT offset.
  R_390_PC16 = 16,          // PC relative 16 bit.
  R_390_PC16DBL = 17,       // PC relative 16 bit shifted by 1.
  R_390_PLT16DBL = 18,      // 16 bit PC rel. PLT shifted by 1.
  // New 64-bit ABI.
  R_390_PC32DBL = 19,       // PC relative 32 bit shifted by 1.
  R_390_PLT32DBL = 20,      // 32 bit PC rel. PLT shifted by 1.
  R_390_GOTPCDBL = 21,      // 32 bit PC rel. GOT shifted by 1.
  R_390_64 = 22,            // Direct 64 bit.
  R_390_PC64 = 23,          // PC relative 64 bit.
  R_390_GOT64 = 24,         // 64 bit GOT offset.
  R_390_PLT64 = 25,         // 64 bit PC relative PLT address.
  R_390_GOTENT = 26,        // 32 bit PC rel. to GOT entry >> 1.
  // Extensions.
  R_390_GOTOFF16 = 27,      // 16 bit offset to GOT.
  R_390_GOTOFF64 = 28,      // 64 bit offset to GOT.
  R_390_GOTPLT12 = 29,      // 12 bit offset to jump slot.
  R_390_GOTPLT16 = 30,      // 16 bit offset to jump slot.
  R_390_GOTPLT32 = 31,      // 32 bit offset to jump slot.
  R_390_GOTPLT64 = 32,      // 64 bit offset to jump slot.
  R_390_GOTPLTENT = 33,     // 32 bit rel. offset to jump slot.
  R_390_PLTOFF16 = 34,      // 16 bit offset from GOT to PLT.
  R_390_PLTOFF32 = 35,      // 32 bit offset from GOT to PLT.
  R_390_PLTOFF64 = 36,      // 16 bit offset from GOT to PLT.
  // TLS extensions.
  R_390_TLS_LOAD = 37,      // Tag for load insn in TLS code.
  R_390_TLS_GDCALL = 38,    // Tag for function call in general dynamic TLS code.
  R_390_TLS_LDCALL = 39,    // Tag for function call in local dynamic TLS code.
  R_390_TLS_GD32 = 40,      // Direct 32 bit for general dynamic thread local data.
  R_390_TLS_GD64 = 41,      // Direct 64 bit for general dynamic thread local data.
  R_390_TLS_GOTIE12 = 42,   // 12 bit GOT offset for static TLS block offset.
  R_390_TLS_GOTIE32 = 43,   // 32 bit GOT offset for static TLS block offset.
  R_390_TLS_GOTIE64 = 44,   // 64 bit GOT offset for static TLS block offset.
  R_390_TLS_LDM32 = 45,     // Direct 32 bit for local dynamic thread local data in LD code.
  R_390_TLS_LDM64 = 46,     // Direct 64 bit for local dynamic thread local data in LD code.
  R_390_TLS_IE32 = 47,      // 32 bit address of GOT entry for negated static TLS block offset.
  R_390_TLS_IE64 = 48,      // 64 bit address of GOT entry for negated static TLS block offset.
  R_390_TLS_IEENT = 49,     // 32 bit rel. offset to GOT entry for negated static TLS block offset.
  R_390_TLS_LE32 = 50,      // 32 bit negated offset relative to static TLS block.
  R_390_TLS_LE64 = 51,      // 64 bit negated offset relative to static TLS block.
  R_390_TLS_LDO32 = 52,     // 32 bit offset relative to TLS block.
  R_390_TLS_LDO64 = 53,     // 64 bit offset relative to TLS block.
  R_390_TLS_DTPMOD = 54,    // ID of module containing symbol.
  R_390_TLS_DTPOFF = 55,    // Offset in TLS block.
  R_390_TLS_TPOFF = 56,     // Negate offset in static TLS block.
  // Yet more misc extensions.
  R_390_20 = 57,            // Direct 20 bit.
  R_390_GOT20 = 58,         // 20 bit GOT offset.
  R_390_GOTPLT20 = 59,      // 20 bit offset to jump slot.
  R_390_TLS_GOTIE20 = 60,   // 20 bit GOT offset for static TLS block offset.
  R_390_IRELATIVE = 61,     // IFUNC relocation.
  R_390_PC12DBL = 62,       // PC relative 12 bit shifted by 1.
  R_390_PLT12DBL = 63,      // 12 bit PC rel. PLT shifted by 1.
  R_390_PC24DBL = 64,       // PC relative 24 bit shifted by 1.
  R_390_PLT24DBL = 65,      // 24 bit PC rel. PLT shifted by 1.
  // GNU vtable garbage collection extensions.
  R_390_GNU_VTINHERIT = 250,
  R_390_GNU_VTENTRY = 251,
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_S390_H)
