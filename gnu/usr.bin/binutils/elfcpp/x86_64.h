// x86-64.h -- ELF definitions specific to EM_X86_64  -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Andrew Chatham.

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

#ifndef ELFCPP_X86_64_H
#define ELFCPP_X86_64_H

namespace elfcpp
{

// Documentation is taken from
//   http://www.x86-64.org/documentation/abi-0.98.pdf
//   elf.h
// Documentation for the TLS relocs is taken from
//   http://people.redhat.com/drepper/tls.pdf
//   http://www.lsd.ic.unicamp.br/~oliva/writeups/TLS/RFC-TLSDESC-x86.txt

enum
{
  R_X86_64_NONE = 0,       // No reloc
  R_X86_64_64 = 1,         // Direct 64 bit
  R_X86_64_PC32 = 2,       // PC relative 32 bit signed
  R_X86_64_GOT32 = 3,      // 32 bit GOT entry
  R_X86_64_PLT32 = 4,      // 32 bit PLT address
  R_X86_64_COPY = 5,       // Copy symbol at runtime
  R_X86_64_GLOB_DAT = 6,   // Create GOT entry
  R_X86_64_JUMP_SLOT = 7,  // Create PLT entry
  R_X86_64_RELATIVE = 8,   // Adjust by program base
  R_X86_64_GOTPCREL = 9,   // 32 bit signed PC relative offset to GOT
  R_X86_64_32 = 10,        // Direct 32 bit zero extended
  R_X86_64_32S = 11,       // Direct 32 bit sign extended
  R_X86_64_16 = 12,        // Direct 16 bit zero extended
  R_X86_64_PC16 = 13,      // 16 bit sign extended pc relative
  R_X86_64_8 = 14,         // Direct 8 bit sign extended
  R_X86_64_PC8 = 15,       // 8 bit sign extended pc relative

  // TLS relocations
  R_X86_64_DTPMOD64 = 16,  // ID of module containing symbol
  R_X86_64_DTPOFF64 = 17,  // Offset in module's TLS block
  R_X86_64_TPOFF64 = 18,   // Offset in initial TLS block
  R_X86_64_TLSGD = 19,     // 32 bit signed PC relative offset to two
                           // GOT entries for GD symbol
  R_X86_64_TLSLD = 20,     // 32 bit signed PC relative offset to two
                           // GOT entries for LD symbol
  R_X86_64_DTPOFF32 = 21,  // Offset in TLS block
  R_X86_64_GOTTPOFF = 22,  // 32 bit signed PC relative offset to GOT
                           // entry for IE symbol
  R_X86_64_TPOFF32 = 23,   // Offset in initial TLS block

  R_X86_64_PC64 = 24,      // 64-bit PC relative
  R_X86_64_GOTOFF64 = 25,  // 64-bit GOT offset
  R_X86_64_GOTPC32 = 26,   // 32-bit PC relative offset to GOT

  R_X86_64_GOT64 = 27,     // 64-bit GOT entry offset
  R_X86_64_GOTPCREL64 = 28, // 64-bit PC relative offset to GOT entry
  R_X86_64_GOTPC64 = 29,   // 64-bit PC relative offset to GOT
  R_X86_64_GOTPLT64 = 30,  // Like GOT64, indicates that PLT entry needed
  R_X86_64_PLTOFF64 = 31,  // 64-bit GOT relative offset to PLT entry

  R_X86_64_SIZE32 = 32,
  R_X86_64_SIZE64 = 33,

  R_X86_64_GOTPC32_TLSDESC = 34, // 32-bit PC relative to TLS descriptor in GOT
  R_X86_64_TLSDESC_CALL = 35,    // Relaxable call through TLS descriptor
  R_X86_64_TLSDESC = 36,         // 2 by 64-bit TLS descriptor
  R_X86_64_IRELATIVE = 37,          // Adjust indirectly by program base
  R_X86_64_RELATIVE64 = 38,      // 64-bit adjust by program base
  R_X86_64_PC32_BND = 39,  // PC relative 32 bit signed with BND prefix
  R_X86_64_PLT32_BND = 40, // 32 bit PLT address with BND prefix
  R_X86_64_GOTPCRELX = 41, // 32 bit signed PC relative offset to GOT
			   // without REX prefix, relaxable.
  R_X86_64_REX_GOTPCRELX = 42, // 32 bit signed PC relative offset to GOT
			       // with REX prefix, relaxable.
  // GNU vtable garbage collection extensions.
  R_X86_64_GNU_VTINHERIT = 250,
  R_X86_64_GNU_VTENTRY = 251
};

// The bit values that can appear in the GNU_PROPERTY_X86_FEATURE_1_AND
// program property.

const uint64_t GNU_PROPERTY_X86_FEATURE_1_IBT = 1ULL << 0;
const uint64_t GNU_PROPERTY_X86_FEATURE_1_SHSTK = 1ULL << 1;

} // End namespace elfcpp.

#endif // !defined(ELFCPP_X86_64_H)
