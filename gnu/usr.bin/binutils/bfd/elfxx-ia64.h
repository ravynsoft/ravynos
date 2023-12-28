/* IA-64 support for 64-bit ELF
   Copyright (C) 1998-2023 Free Software Foundation, Inc.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

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

reloc_howto_type *ia64_elf_reloc_type_lookup (bfd *, bfd_reloc_code_real_type);

reloc_howto_type *ia64_elf_reloc_name_lookup (bfd *, const char *);

reloc_howto_type *ia64_elf_lookup_howto (unsigned int rtype);

bool ia64_elf_relax_br (bfd_byte *contents, bfd_vma off);
void ia64_elf_relax_brl (bfd_byte *contents, bfd_vma off);
void ia64_elf_relax_ldxmov (bfd_byte *contents, bfd_vma off);

bfd_reloc_status_type ia64_elf_install_value (bfd_byte *hit_addr, bfd_vma v,
					      unsigned int r_type);

/* IA64 Itanium code generation.  Called from linker.  */
extern void bfd_elf32_ia64_after_parse
  (int);

extern void bfd_elf64_ia64_after_parse
  (int);
