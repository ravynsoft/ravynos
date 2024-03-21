/* BFD back-end for Intel/AMD x86_64 PECOFF files.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

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
   MA 02110-1301, USA.

   Written by Kai Tietz, OneVision Software GmbH&CoKg.  */

#define PEI_HEADERS
#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "libiberty.h"
#include "coff/x86_64.h"
#include "coff/internal.h"
#include "coff/pe.h"
#include "libcoff.h"

#define TARGET_SYM		x86_64_pe_vec
#define TARGET_NAME		"pe-x86-64"
#define TARGET_SYM_BIG		x86_64_pe_big_vec
#define TARGET_NAME_BIG		"pe-bigobj-x86-64"
#define COFF_WITH_PE
#define COFF_WITH_pex64
#define COFF_WITH_PE_BIGOBJ
#define PCRELOFFSET		true
#if defined (USE_MINGW64_LEADING_UNDERSCORES)
#define TARGET_UNDERSCORE	'_'
#else
#define TARGET_UNDERSCORE	0
#endif
#define COFF_LONG_SECTION_NAMES
#define COFF_SUPPORT_GNU_LINKONCE
#define COFF_LONG_FILENAMES

#define COFF_SECTION_ALIGNMENT_ENTRIES \
{ COFF_SECTION_NAME_EXACT_MATCH (".bss"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".data"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".rdata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".text"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".idata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 2 }, \
{ COFF_SECTION_NAME_EXACT_MATCH (".pdata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 2 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".debug"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".zdebug"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.wi."), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }

/* The function pex64_bfd_print_pdata is implemented in pei-x86_64.c
   source, but has be extended to also handle pe objects.  */
extern bool pex64_bfd_print_pdata (bfd *, void *);

#define bfd_pe_print_pdata   pex64_bfd_print_pdata

static bool
pex64_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  if (bfd_link_pde (info)
      && bfd_get_flavour (info->output_bfd) == bfd_target_elf_flavour)
    {
      /* NB: When linking Windows x86-64 relocatable object files to
	 generate ELF executable, create an indirect reference to
	 __executable_start for __ImageBase to support R_AMD64_IMAGEBASE
	 relocation which is relative to __ImageBase.  */
      struct bfd_link_hash_entry *h, *hi;
      hi = bfd_link_hash_lookup (info->hash, "__ImageBase", true, false,
				 false);
      if (hi->type == bfd_link_hash_new
	  || hi->type == bfd_link_hash_undefined
	  || hi->type == bfd_link_hash_undefweak)
	{
	  h = bfd_link_hash_lookup (info->hash, "__executable_start",
				    true, false, true);
	  hi->type = bfd_link_hash_indirect;
	  hi->u.i.link = h;
	}
    }

  return _bfd_coff_link_add_symbols (abfd, info);
}

#define coff_bfd_link_add_symbols pex64_link_add_symbols

#include "coff-x86_64.c"
