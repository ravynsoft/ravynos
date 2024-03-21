/* 32-bit ELF support for TI C6X
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

#ifdef __cplusplus
extern "C" {
#endif

extern int elf32_tic6x_merge_arch_attributes (int, int);

/* This function is provided for use from the assembler.  */

extern void elf32_tic6x_set_use_rela_p (bfd *, bool);

struct elf32_tic6x_params
{
  int dsbt_index;
  int dsbt_size;
};

extern void elf32_tic6x_setup (struct bfd_link_info *,
			       struct elf32_tic6x_params *);

/* C6x unwind section editing support.  */
extern bool elf32_tic6x_fix_exidx_coverage (struct bfd_section **,
					    unsigned int,
					    struct bfd_link_info *,
					    bool);
#ifdef __cplusplus
}
#endif
