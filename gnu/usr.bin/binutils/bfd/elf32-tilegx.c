/* TILE-Gx-specific support for 32-bit ELF.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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
#include "elfxx-tilegx.h"
#include "elf32-tilegx.h"


/* Support for core dump NOTE sections.  */

static bool
tilegx_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  if (note->descsz != TILEGX_PRSTATUS_SIZEOF)
    return false;

  /* pr_cursig */
  elf_tdata (abfd)->core->signal =
    bfd_get_16 (abfd, note->descdata + TILEGX_PRSTATUS_OFFSET_PR_CURSIG);

  /* pr_pid */
  elf_tdata (abfd)->core->pid =
    bfd_get_32 (abfd, note->descdata + TILEGX_PRSTATUS_OFFSET_PR_PID);

  /* pr_reg */
  offset = TILEGX_PRSTATUS_OFFSET_PR_REG;
  size   = TILEGX_GREGSET_T_SIZE;

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
tilegx_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz != TILEGX_PRPSINFO_SIZEOF)
    return false;

  elf_tdata (abfd)->core->program
    = _bfd_elfcore_strndup (abfd, note->descdata + TILEGX_PRPSINFO_OFFSET_PR_FNAME, 16);
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + TILEGX_PRPSINFO_OFFSET_PR_PSARGS, ELF_PR_PSARGS_SIZE);


  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */
  {
    char *command = elf_tdata (abfd)->core->command;
    int n = strlen (command);

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return true;
}


#define ELF_ARCH		bfd_arch_tilegx
#define ELF_TARGET_ID		TILEGX_ELF_DATA
#define ELF_MACHINE_CODE	EM_TILEGX
#define ELF_MAXPAGESIZE		0x10000
#define ELF_COMMONPAGESIZE	0x10000

#define TARGET_BIG_SYM		tilegx_elf32_be_vec
#define TARGET_BIG_NAME		"elf32-tilegx-be"
#define TARGET_LITTLE_SYM	tilegx_elf32_le_vec
#define TARGET_LITTLE_NAME	"elf32-tilegx-le"

#define elf_backend_reloc_type_class	     tilegx_reloc_type_class

#define bfd_elf32_bfd_reloc_name_lookup	     tilegx_reloc_name_lookup
#define bfd_elf32_bfd_link_hash_table_create tilegx_elf_link_hash_table_create
#define bfd_elf32_bfd_reloc_type_lookup	     tilegx_reloc_type_lookup
#define bfd_elf32_bfd_merge_private_bfd_data \
  _bfd_tilegx_elf_merge_private_bfd_data

#define elf_backend_copy_indirect_symbol     tilegx_elf_copy_indirect_symbol
#define elf_backend_create_dynamic_sections  tilegx_elf_create_dynamic_sections
#define elf_backend_check_relocs	     tilegx_elf_check_relocs
#define elf_backend_adjust_dynamic_symbol    tilegx_elf_adjust_dynamic_symbol
#define elf_backend_omit_section_dynsym	     tilegx_elf_omit_section_dynsym
#define elf_backend_size_dynamic_sections    tilegx_elf_size_dynamic_sections
#define elf_backend_relocate_section	     tilegx_elf_relocate_section
#define elf_backend_finish_dynamic_symbol    tilegx_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections  tilegx_elf_finish_dynamic_sections
#define elf_backend_gc_mark_hook	     tilegx_elf_gc_mark_hook
#define elf_backend_plt_sym_val		     tilegx_elf_plt_sym_val
#define elf_info_to_howto_rel		     NULL
#define elf_info_to_howto		     tilegx_info_to_howto_rela
#define elf_backend_grok_prstatus	     tilegx_elf_grok_prstatus
#define elf_backend_grok_psinfo		     tilegx_elf_grok_psinfo
#define elf_backend_additional_program_headers tilegx_additional_program_headers

#define elf_backend_init_index_section	_bfd_elf_init_1_index_section

#define elf_backend_can_gc_sections 1
#define elf_backend_can_refcount 1
#define elf_backend_want_got_plt 1
#define elf_backend_plt_readonly 1
/* Align PLT mod 64 byte L2 line size. */
#define elf_backend_plt_alignment 6
#define elf_backend_want_plt_sym 1
#define elf_backend_got_header_size 4
#define elf_backend_want_dynrelro 1
#define elf_backend_rela_normal 1
#define elf_backend_default_execstack 0

#include "elf32-target.h"
