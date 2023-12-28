/* Lattice Mico32 ELF support for BFD.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Contributed by Jon Beniston <jon@beniston.com>

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

#ifndef _ELF_LM32_H
#define _ELF_LM32_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_lm32_reloc_type)
     RELOC_NUMBER (R_LM32_NONE,                      0)
     RELOC_NUMBER (R_LM32_8,                         1)
     RELOC_NUMBER (R_LM32_16,                        2)
     RELOC_NUMBER (R_LM32_32,                        3)
     RELOC_NUMBER (R_LM32_HI16,                      4)
     RELOC_NUMBER (R_LM32_LO16,                      5)
     RELOC_NUMBER (R_LM32_GPREL16,                   6)
     RELOC_NUMBER (R_LM32_CALL,                      7)
     RELOC_NUMBER (R_LM32_BRANCH,                    8)
     RELOC_NUMBER (R_LM32_GNU_VTINHERIT,             9)
     RELOC_NUMBER (R_LM32_GNU_VTENTRY,               10)
     RELOC_NUMBER (R_LM32_16_GOT,                    11)
     RELOC_NUMBER (R_LM32_GOTOFF_HI16,               12)
     RELOC_NUMBER (R_LM32_GOTOFF_LO16,               13)
     RELOC_NUMBER (R_LM32_COPY,                      14)
     RELOC_NUMBER (R_LM32_GLOB_DAT,                  15)
     RELOC_NUMBER (R_LM32_JMP_SLOT,                  16)
     RELOC_NUMBER (R_LM32_RELATIVE,                  17)
END_RELOC_NUMBERS (R_LM32_max)

/* Processor specific flags for the ELF header e_flags field.  */

#define EF_LM32_MACH                 0x00000001

/* Various CPU types.  */

#define E_LM32_MACH                  0x1

#endif /* _ELF_LM32_H */
