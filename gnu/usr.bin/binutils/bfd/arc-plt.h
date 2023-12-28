/* ARC-specific header file for PLT support.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   Contributed by Cupertino Miranda (cmiranda@synopsys.com).

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef ARC_PLT_H
#define ARC_PLT_H

/* Instructions appear in memory as a sequence of half-words (16 bit);
   individual half-words are represented on the target in target byte order.
   We use 'unsigned short' on the host to represent the PLT templates,
   and translate to target byte order as we copy to the target.  */
typedef uint16_t insn_hword;

enum plt_reloc_symbol
{
  LAST_RELOC = 0,

  SGOT = 1,

  RELATIVE = (1 << 8),
  RELATIVE_INSN_32 = (1 << 9),
  RELATIVE_INSN_24 = (1 << 10),

  MIDDLE_ENDIAN = (1 << 11)
};

#define IS_RELATIVE(S)	    ((S & (RELATIVE | RELATIVE_INSN_24 | RELATIVE_INSN_32)) != 0)
#define IS_INSN_32(S)	    ((S & RELATIVE_INSN_32) != 0)
#define IS_INSN_24(S)	    ((S & RELATIVE_INSN_24) != 0)
#define IS_MIDDLE_ENDIAN(S) ((S & MIDDLE_ENDIAN) != 0)
#define SYM_ONLY(S)	    (S & 0xFF)

struct plt_reloc
{
  bfd_vma	  offset;
  bfd_vma	  size;
  bfd_vma	  mask;
  enum plt_reloc_symbol symbol;
  bfd_vma	  addend;
};


#define PLT_TYPE_START(NAME) NAME,
#define PLT_TYPE_END(NAME)
#define PLT_ENTRY(...)
#define PLT_ELEM(...)
#define ENTRY_RELOC(...)
#define ELEM_RELOC(...)

enum plt_types_enum
{
  PLT_START = -1,
#include "arc-plt.def"
  PLT_MAX
};

#undef PLT_TYPE_START
#undef PLT_TYPE_END
#undef PLT_ENTRY
#undef PLT_ELEM
#undef ENTRY_RELOC
#undef ELEM_RELOC

typedef insn_hword insn_hword_array[];

struct plt_version_t
{
  const insn_hword_array *entry;
  const bfd_vma		  entry_size;
  const insn_hword_array *elem;
  const bfd_vma		  elem_size;

  const struct plt_reloc *entry_relocs;
  const struct plt_reloc *elem_relocs;
};

#define PLT_TYPE_START(NAME) \
  const insn_hword NAME##_plt_entry[] = {
#define PLT_TYPE_END(NAME) };
#define PLT_ENTRY(...) __VA_ARGS__,
#define PLT_ELEM(...)
#define ENTRY_RELOC(...)
#define ELEM_RELOC(...)

#include "arc-plt.def"

#undef PLT_TYPE_START
#undef PLT_TYPE_END
#undef PLT_ENTRY
#undef PLT_ELEM
#undef ENTRY_RELOC
#undef ELEM_RELOC

#define PLT_TYPE_START(NAME) \
  const struct plt_reloc NAME##_plt_entry_relocs[] = {
#define PLT_TYPE_END(NAME) \
    {0, 0, 0, LAST_RELOC, 0} \
  };
#define PLT_ENTRY(...)
#define PLT_ELEM(...)
#define ENTRY_RELOC(...) { __VA_ARGS__ },
#define ELEM_RELOC(...)

#include "arc-plt.def"

#undef PLT_TYPE_START
#undef PLT_TYPE_END
#undef PLT_ENTRY
#undef PLT_ELEM
#undef ENTRY_RELOC
#undef ELEM_RELOC


#define PLT_TYPE_START(NAME) \
  const insn_hword NAME##_plt_elem[] = {
#define PLT_TYPE_END(NAME) };
#define PLT_ENTRY(...)
#define PLT_ELEM(...) __VA_ARGS__,
#define ENTRY_RELOC(...)
#define ELEM_RELOC(...)

#include "arc-plt.def"

#undef PLT_TYPE_START
#undef PLT_TYPE_END
#undef PLT_ENTRY
#undef PLT_ELEM
#undef ENTRY_RELOC
#undef ELEM_RELOC

#define PLT_TYPE_START(NAME) \
  const struct plt_reloc NAME##_plt_elem_relocs[] = {
#define PLT_TYPE_END(NAME) \
    {0, 0, 0, LAST_RELOC, 0} \
  };
#define PLT_ENTRY(...)
#define PLT_ELEM(...)
#define ENTRY_RELOC(...)
#define ELEM_RELOC(...) { __VA_ARGS__ },

#include "arc-plt.def"

#undef PLT_TYPE_START
#undef PLT_TYPE_END
#undef PLT_ENTRY
#undef PLT_ELEM
#undef ENTRY_RELOC
#undef ELEM_RELOC


#define PLT_TYPE_START(NAME) \
  { \
    .entry = &NAME##_plt_entry, \
    .entry_size = sizeof (NAME##_plt_entry), \
    .elem = &NAME##_plt_elem, \
    .elem_size = sizeof (NAME##_plt_elem),  \
    .entry_relocs = NAME##_plt_entry_relocs, \
    .elem_relocs = NAME##_plt_elem_relocs
#define PLT_TYPE_END(NAME) },
#define PLT_ENTRY(...)
#define PLT_ELEM(...)
#define ENTRY_RELOC(...)
#define ELEM_RELOC(...)
const struct plt_version_t plt_versions[PLT_MAX] = {

#include "arc-plt.def"

};
#undef PLT_TYPE_START
#undef PLT_TYPE_END
#undef PLT_ENTRY
#undef PLT_ELEM
#undef ENTRY_RELOC
#undef ELEM_RELOC


#endif /* ARC_PLT_H */
