/* 32-bit ELF support for S+core.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by
   Brain.lin (brain.lin@sunplusct.com)
   Mei Ligang (ligang@sunnorth.com.cn)
   Pei-Lin Tsai (pltsai@sunplus.com)

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

#include "elf/common.h"
#include "elf/internal.h"

extern void
s7_bfd_score_elf_hide_symbol (struct bfd_link_info *,
			      struct elf_link_hash_entry *,
			      bool);

extern bool
s7_bfd_score_info_to_howto (bfd *, arelent *, Elf_Internal_Rela *);

extern int
s7_bfd_score_elf_relocate_section (bfd *,
				   struct bfd_link_info *,
				   bfd *,
				   asection *,
				   bfd_byte *,
				   Elf_Internal_Rela *,
				   Elf_Internal_Sym *,
				   asection **);

extern bool
s7_bfd_score_elf_check_relocs (bfd *,
			       struct bfd_link_info *,
			       asection *,
			       const Elf_Internal_Rela *);

extern bool
s7_bfd_score_elf_add_symbol_hook (bfd *,
				  struct bfd_link_info *,
				  Elf_Internal_Sym *,
				  const char **,
				  flagword *,
				  asection **,
				  bfd_vma *);

extern void
s7_bfd_score_elf_symbol_processing (bfd *, asymbol *);

extern int
s7_bfd_score_elf_link_output_symbol_hook (struct bfd_link_info *,
					  const char *,
					  Elf_Internal_Sym *,
					  asection *,
					  struct elf_link_hash_entry *);

extern bool
s7_bfd_score_elf_section_from_bfd_section (bfd *,
					   asection *,
					   int *);

extern bool
s7_bfd_score_elf_adjust_dynamic_symbol (struct bfd_link_info *,
					struct elf_link_hash_entry *);

extern bool
s7_bfd_score_elf_always_size_sections (bfd *, struct bfd_link_info *);

extern bool
s7_bfd_score_elf_size_dynamic_sections (bfd *, struct bfd_link_info *);

extern bool
s7_bfd_score_elf_create_dynamic_sections (bfd *, struct bfd_link_info *);

extern bool
s7_bfd_score_elf_finish_dynamic_symbol (bfd *,
					struct bfd_link_info *,
					struct elf_link_hash_entry *,
					Elf_Internal_Sym *);

extern bool
s7_bfd_score_elf_finish_dynamic_sections (bfd *, struct bfd_link_info *);

extern bool
s7_bfd_score_elf_fake_sections (bfd *,
				Elf_Internal_Shdr *,
				asection *);

extern bool
s7_bfd_score_elf_section_processing (bfd *, Elf_Internal_Shdr *);

extern bool
s7_bfd_score_elf_write_section (bfd *, asection *, bfd_byte *);

extern void
s7_bfd_score_elf_copy_indirect_symbol (struct bfd_link_info *,
				       struct elf_link_hash_entry *,
				       struct elf_link_hash_entry *);

extern bool
s7_bfd_score_elf_discard_info (bfd *, struct elf_reloc_cookie *,
			       struct bfd_link_info *);

extern bool
s7_bfd_score_elf_ignore_discarded_relocs (asection *);

extern asection *
s7_bfd_score_elf_gc_mark_hook (asection *,
			       struct bfd_link_info *,
			       Elf_Internal_Rela *,
			       struct elf_link_hash_entry *,
			       Elf_Internal_Sym *);

extern bool
s7_bfd_score_elf_grok_prstatus (bfd *, Elf_Internal_Note *);

extern bool
s7_bfd_score_elf_grok_psinfo (bfd *, Elf_Internal_Note *);

extern reloc_howto_type *
s7_elf32_score_reloc_type_lookup (bfd *, bfd_reloc_code_real_type);

extern struct bfd_link_hash_table *
s7_elf32_score_link_hash_table_create (bfd *);

extern bool
s7_elf32_score_print_private_bfd_data (bfd *, void *);

extern bool
s7_elf32_score_merge_private_bfd_data (bfd *, struct bfd_link_info *);

extern bool
s7_elf32_score_new_section_hook (bfd *, asection *);

extern bool
_bfd_score_elf_common_definition (Elf_Internal_Sym *);

#define elf_backend_common_definition   _bfd_score_elf_common_definition
