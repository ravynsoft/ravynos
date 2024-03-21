/* Adapteva EPIPHANY ELF support for BFD.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by Embecosm on behalf of Adapteva, Inc.

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

#ifndef _ELF_EPIPHANY_H
#define _ELF_EPIPHANY_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_epiphany_reloc_type)
  RELOC_NUMBER (R_EPIPHANY_NONE,    0)

  /* Absolute address relocations.  */
  RELOC_NUMBER (R_EPIPHANY_8,       1)
  RELOC_NUMBER (R_EPIPHANY_16,      2)
  RELOC_NUMBER (R_EPIPHANY_32,      3)

  /* PC-relative relocations.  */
  RELOC_NUMBER (R_EPIPHANY_8_PCREL, 4)
  RELOC_NUMBER (R_EPIPHANY_16_PCREL,5)
  RELOC_NUMBER (R_EPIPHANY_32_PCREL,6)

  /* special forms for 8/24 bit branch displacements.  */
  RELOC_NUMBER (R_EPIPHANY_SIMM8,   7)
  RELOC_NUMBER (R_EPIPHANY_SIMM24,  8)

  /* HIGH and LOW relocations taking part of a 32 bit address and
     depositing it into the IMM16 field of a destination.  */
  RELOC_NUMBER (R_EPIPHANY_HIGH, 9)
  RELOC_NUMBER (R_EPIPHANY_LOW,10)

  /* 11 bit signed immediate value.  */
  RELOC_NUMBER (R_EPIPHANY_SIMM11, 11)
  /* 11 bit magnitude addressing displacement.  */
  RELOC_NUMBER (R_EPIPHANY_IMM11, 12)

  /* 8 bit immediate for MOV.S R,IMM8.  */
  RELOC_NUMBER (R_EPIPHANY_IMM8, 13)

END_RELOC_NUMBERS(R_EPIPHANY_max)

#endif /* _ELF_EPIPHANY_H */
