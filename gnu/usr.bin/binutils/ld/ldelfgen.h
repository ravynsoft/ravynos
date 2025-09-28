/* Emulation code used by all ELF targets.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

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

struct elf_internal_sym;
struct elf_strtab_hash;
struct ctf_dict;

extern void ldelf_map_segments (bool);
extern int ldelf_emit_ctf_early (void);
extern void ldelf_acquire_strings_for_ctf
  (struct ctf_dict *ctf_output, struct elf_strtab_hash *strtab);
extern void ldelf_new_dynsym_for_ctf
  (struct ctf_dict *ctf_output, int symidx, struct elf_internal_sym *sym);
