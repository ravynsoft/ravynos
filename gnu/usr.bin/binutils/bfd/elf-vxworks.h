/* VxWorks support for ELF
   Copyright (C) 2005-2023 Free Software Foundation, Inc.

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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "elf/common.h"
#include "elf/internal.h"

bool elf_vxworks_add_symbol_hook
  (bfd *, struct bfd_link_info *, Elf_Internal_Sym *, const char **,
   flagword *, asection **, bfd_vma *);
int elf_vxworks_link_output_symbol_hook
  (struct bfd_link_info *, const char *name, Elf_Internal_Sym *,
   asection *, struct elf_link_hash_entry *);
bool elf_vxworks_emit_relocs
  (bfd *, asection *, Elf_Internal_Shdr *, Elf_Internal_Rela *,
   struct elf_link_hash_entry **);
bool elf_vxworks_final_write_processing (bfd *);
bool elf_vxworks_create_dynamic_sections
  (bfd *, struct bfd_link_info *, asection **);
bool elf_vxworks_add_dynamic_entries (bfd *, struct bfd_link_info *);
bool elf_vxworks_finish_dynamic_entry (bfd *, Elf_Internal_Dyn *);
bool _bfd_elf_maybe_vxworks_add_dynamic_tags
  (bfd *, struct bfd_link_info *, bool);
