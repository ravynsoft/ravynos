/* ft32 ELF support for BFD.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ELF_FT32_H
#define _ELF_FT32_H

#include "elf/reloc-macros.h"

/* Relocation types.  */
START_RELOC_NUMBERS (elf_ft32_reloc_type)
  RELOC_NUMBER (R_FT32_NONE, 0)
  RELOC_NUMBER (R_FT32_32, 1)
  RELOC_NUMBER (R_FT32_16, 2)
  RELOC_NUMBER (R_FT32_8,  3)
  RELOC_NUMBER (R_FT32_10, 4)
  RELOC_NUMBER (R_FT32_20, 5)
  RELOC_NUMBER (R_FT32_17, 6)
  RELOC_NUMBER (R_FT32_18, 7)
  RELOC_NUMBER (R_FT32_RELAX, 8)
  RELOC_NUMBER (R_FT32_SC0, 9)
  RELOC_NUMBER (R_FT32_SC1, 10)
  RELOC_NUMBER (R_FT32_15, 11)
  RELOC_NUMBER (R_FT32_DIFF32, 12)
END_RELOC_NUMBERS (R_FT32_max)

#endif /* _ELF_FT32_H */
