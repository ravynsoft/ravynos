// i386.h -- ELF definitions specific to EM_386  -*- C++ -*-

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

#ifndef ELFCPP_I386_H
#define ELFCPP_I386_H

// Documentation for the TLS relocs is taken from
//   http://people.redhat.com/drepper/tls.pdf
//   http://www.lsd.ic.unicamp.br/~oliva/writeups/TLS/RFC-TLSDESC-x86.txt

namespace elfcpp
{

enum
{
  R_386_NONE = 0,        // No reloc
  R_386_32 = 1,          // Direct 32 bit zero extended
  R_386_PC32 = 2,        // PC relative 32 bit signed
  R_386_GOT32 = 3,       // 32 bit GOT entry  
  R_386_PLT32 = 4,       // 32 bit PLT address
  R_386_COPY = 5,        // Copy symbol at runtime
  R_386_GLOB_DAT = 6,    // Create GOT entry      
  R_386_JUMP_SLOT = 7,   // Create PLT entry      
  R_386_RELATIVE = 8,    // Adjust by program base
  R_386_GOTOFF = 9,      // 32-bit GOT offset
  R_386_GOTPC = 10,      // 32-bit PC relative offset to GOT
  // Used by Sun.
  R_386_32PLT = 11,
  // TLS extensions.
  R_386_TLS_TPOFF = 14,  // Outstanding Initial Exec reloc, gnu-style (both)
  R_386_TLS_IE = 15,     // Initial Initial Exec reloc, gnu-style (no-PIC)
  R_386_TLS_GOTIE = 16,  // Initial Initial Exec reloc, gnu-style (for PIC)
  R_386_TLS_LE = 17,     // Initial Local Exec reloc, gnu-style
  R_386_TLS_GD = 18,     // Initial General Dynamic reloc, gnu-style
  R_386_TLS_LDM = 19,    // Initial Local Dynamic reloc, gnu-style
  // GNU extensions.
  R_386_16 = 20,         // Direct 16 bit zero extended
  R_386_PC16 = 21,       // 16 bit sign extended pc relative
  R_386_8 = 22,          // Direct 8 bit sign extended
  R_386_PC8 = 23,        // 8 bit sign extended pc relative
  // More TLS relocs.
  R_386_TLS_GD_32 = 24,     // Initial General Dynamic reloc, sun-style
  R_386_TLS_GD_PUSH = 25,   // Initial General Dynamic reloc, sun-style
  R_386_TLS_GD_CALL = 26,   // Initial General Dynamic reloc, sun-style
  R_386_TLS_GD_POP = 27,    // Initial General Dynamic reloc, sun-style
  R_386_TLS_LDM_32 = 28,    // Initial Local Dynamic reloc, sun-style
  R_386_TLS_LDM_PUSH = 29,  // Initial Local Dynamic reloc, sun-style
  R_386_TLS_LDM_CALL = 30,  // Initial Local Dynamic reloc, sun-style
  R_386_TLS_LDM_POP = 31,   // Initial Local Dynamic reloc, sun-style
  R_386_TLS_LDO_32 = 32,    // Initial Local Dynamic reloc, sun+gnu styles
  R_386_TLS_IE_32 = 33,     // Initial Initial Exec reloc, sun-style
  R_386_TLS_LE_32 = 34,     // Initial Local Exec reloc, sun-style
  R_386_TLS_DTPMOD32 = 35,  // Outstanding General/Local Dynamic reloc, sun+gnu
  R_386_TLS_DTPOFF32 = 36,  // Outstanding General Dynamic reloc, sun+gnu
  R_386_TLS_TPOFF32 = 37,   // Outstanding Initial Exec reloc, sun-style
  R_386_TLS_GOTDESC = 39,   // GOT offset for TLS descriptor
  R_386_TLS_DESC_CALL = 40, // Marker of call through TLS desc for relaxation
  R_386_TLS_DESC = 41,      // TLS descriptor containing pointer to code and
                            // to argument, returning TLS offset for symbol
  R_386_IRELATIVE = 42,     // Adjust indirectly by program base
  R_386_GOT32X = 43,        // 32 bit GOT entry, relaxable
  // Used by Intel.
  R_386_USED_BY_INTEL_200 = 200,
  // GNU vtable garbage collection extensions.
  R_386_GNU_VTINHERIT = 250,
  R_386_GNU_VTENTRY = 251
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_I386_H)
