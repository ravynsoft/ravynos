/* Generic support for 64-bit ELF
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"

/* This does not include any relocation information, but should be
   good enough for GDB or objdump to read the file.  */

static reloc_howto_type dummy =
  HOWTO (0,			/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "UNKNOWN",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false);		/* pcrel_offset */

static bool
elf_generic_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
			   arelent *bfd_reloc,
			   Elf_Internal_Rela *elf_reloc ATTRIBUTE_UNUSED)
{
  bfd_reloc->howto = &dummy;
  return true;
}

static bool
elf_generic_info_to_howto_rel (bfd *abfd ATTRIBUTE_UNUSED,
			       arelent *bfd_reloc,
			       Elf_Internal_Rela *elf_reloc ATTRIBUTE_UNUSED)
{
  bfd_reloc->howto = &dummy;
  return true;
}

static void
check_for_relocs (bfd * abfd, asection * o, void * failed)
{
  if ((o->flags & SEC_RELOC) != 0)
    {
      Elf_Internal_Ehdr *ehdrp;

      ehdrp = elf_elfheader (abfd);
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: Relocations in generic ELF (EM: %d)"),
			  abfd, ehdrp->e_machine);

      bfd_set_error (bfd_error_wrong_format);
      * (bool *) failed = true;
    }
}

static bool
elf64_generic_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  bool failed = false;

  /* Check if there are any relocations.  */
  bfd_map_over_sections (abfd, check_for_relocs, & failed);

  if (failed)
    return false;
  return bfd_elf_link_add_symbols (abfd, info);
}

#define TARGET_LITTLE_SYM		elf64_le_vec
#define TARGET_LITTLE_NAME		"elf64-little"
#define TARGET_BIG_SYM			elf64_be_vec
#define TARGET_BIG_NAME			"elf64-big"
#define ELF_ARCH			bfd_arch_unknown
#define ELF_MACHINE_CODE		EM_NONE
#define ELF_MAXPAGESIZE			0x1
#define bfd_elf64_bfd_reloc_type_lookup bfd_default_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup _bfd_norelocs_bfd_reloc_name_lookup
#define bfd_elf64_bfd_link_add_symbols	elf64_generic_link_add_symbols
#define elf_info_to_howto		elf_generic_info_to_howto
#define elf_info_to_howto_rel		elf_generic_info_to_howto_rel

#include "elf64-target.h"
