/* Visium ELF support for BFD.

   Copyright (C) 2002-2023 Free Software Foundation, Inc.

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

#ifndef _ELF_VISIUM_H
#define _ELF_VISIUM_H

#include "elf/reloc-macros.h"

/* Processor specific flags for the ELF header e_flags field.  */
#define EF_VISIUM_ARCH_MCM   0x01
#define EF_VISIUM_ARCH_MCM24 0x02
#define EF_VISIUM_ARCH_GR6   0x04

/* Relocations.  */
START_RELOC_NUMBERS (elf_visium_reloc_type)
  RELOC_NUMBER (R_VISIUM_NONE, 0)
  RELOC_NUMBER (R_VISIUM_8, 1)
  RELOC_NUMBER (R_VISIUM_16, 2)
  RELOC_NUMBER (R_VISIUM_32, 3)
  RELOC_NUMBER (R_VISIUM_8_PCREL, 4)
  RELOC_NUMBER (R_VISIUM_16_PCREL, 5)
  RELOC_NUMBER (R_VISIUM_32_PCREL, 6)
  RELOC_NUMBER (R_VISIUM_PC16, 7)
  RELOC_NUMBER (R_VISIUM_HI16, 8)
  RELOC_NUMBER (R_VISIUM_LO16, 9)
  RELOC_NUMBER (R_VISIUM_IM16, 10)
  RELOC_NUMBER (R_VISIUM_HI16_PCREL, 11)
  RELOC_NUMBER (R_VISIUM_LO16_PCREL, 12)
  RELOC_NUMBER (R_VISIUM_IM16_PCREL, 13)
  RELOC_NUMBER (R_VISIUM_GNU_VTINHERIT, 200)
  RELOC_NUMBER (R_VISIUM_GNU_VTENTRY, 201)
END_RELOC_NUMBERS(R_VISIUM_max)

#endif /* _ELF_VISIUM_H */
