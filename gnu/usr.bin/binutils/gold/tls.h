// tls.h -- Thread-Local Storage utility routines for gold   -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#ifndef GOLD_TLS_H
#define GOLD_TLS_H

#include "elfcpp.h"
#include "reloc.h"

namespace gold
{

namespace tls
{

// This is used for relocations that can be converted to a different,
// more efficient type of relocation.

enum Tls_optimization
{
  TLSOPT_NONE,    // Can not convert this relocation to a more efficient one.
  TLSOPT_TO_LD,   // Can convert General Dynamic to Local Dynamic.
  TLSOPT_TO_LE,   // Can convert GD or LD to Local-Exec.
  TLSOPT_TO_IE,   // Can convert GD or LD or LE to Initial-Exec.
};

// Check the range for a TLS relocation.  This is inlined for efficiency.

template<int size, bool big_endian>
inline void
check_range(const Relocate_info<size, big_endian>* relinfo,
            size_t relnum,
            typename elfcpp::Elf_types<size>::Elf_Addr rel_offset,
            section_size_type view_size, int off)
{
  typename elfcpp::Elf_types<size>::Elf_Addr offset = rel_offset + off;
  // Elf_Addr is unsigned, so this also tests for signed offset < 0.
  if (offset > view_size)
    gold_error_at_location(relinfo, relnum, rel_offset,
			   _("TLS relocation out of range"));
}

// Check the validity of a TLS relocation.  This is like assert.

template<int size, bool big_endian>
inline void
check_tls(const Relocate_info<size, big_endian>* relinfo,
          size_t relnum,
          typename elfcpp::Elf_types<size>::Elf_Addr rel_offset,
          bool valid)
{
  if (!valid)
    gold_error_at_location(relinfo, relnum, rel_offset,
			   _("TLS relocation against invalid instruction"));
}


} // End namespace tls.

} // End namespace gold.

#endif // !defined(GOLD_TLS_H)
