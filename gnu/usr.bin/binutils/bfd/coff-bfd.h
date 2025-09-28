/* BFD COFF interfaces used outside of BFD.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Cygnus Support.

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

/* This structure is used for a comdat section, as in PE.  A comdat
   section is associated with a particular symbol.  When the linker
   sees a comdat section, it keeps only one of the sections with a
   given name and associated with a given symbol.  */

struct coff_comdat_info
{
  /* The name of the symbol associated with a comdat section.  */
  const char *name;

  /* The local symbol table index of the symbol associated with a
     comdat section.  This is only meaningful to the object file format
     specific code; it is not an index into the list returned by
     bfd_canonicalize_symtab.  */
  long symbol;
};

/* The used_by_bfd field of a section may be set to a pointer to this
   structure.  */

struct coff_section_tdata
{
  /* The relocs, swapped into COFF internal form.  This may be NULL.  */
  struct internal_reloc *relocs;
  /* The section contents.  This may be NULL.  */
  bfd_byte *contents;
  /* Information cached by coff_find_nearest_line.  */
  bool saved_bias;
  bfd_signed_vma bias;
  bfd_vma offset;
  unsigned int i;
  const char *function;
  /* Optional information about a COMDAT entry; NULL if not COMDAT. */
  struct coff_comdat_info *comdat;
  int line_base;
  /* A pointer used for .stab linking optimizations.  */
  void * stab_info;
  /* Available for individual backends.  */
  void * tdata;
};

/* An accessor macro for the coff_section_tdata structure.  */
#define coff_section_data(abfd, sec) \
  ((struct coff_section_tdata *) (sec)->used_by_bfd)

#define bfd_coff_get_comdat_section(abfd, sec)		\
  ((bfd_get_flavour (abfd) == bfd_target_coff_flavour	\
    && coff_section_data (abfd, sec) != NULL)		\
   ? coff_section_data (abfd, sec)->comdat : NULL)

#define coff_symbol_from(symbol)			\
  ((bfd_family_coff (bfd_asymbol_bfd (symbol))		\
    && bfd_asymbol_bfd (symbol)->tdata.coff_obj_data)	\
   ? (coff_symbol_type *) (symbol) : NULL)

struct internal_syment;
union internal_auxent;

extern bool bfd_coff_get_syment
  (bfd *, struct bfd_symbol *, struct internal_syment *);

extern bool bfd_coff_get_auxent
  (bfd *, struct bfd_symbol *, int, union internal_auxent *);

extern bool bfd_coff_set_symbol_class
  (bfd *, struct bfd_symbol *, unsigned int);
