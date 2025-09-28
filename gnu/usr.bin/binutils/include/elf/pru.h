/* TI PRU ELF support for BFD.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   Contributed by Dimitar Dimitrov <dimitar@dinux.eu>

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


/* This file holds definitions specific to the TI PRU ELF ABI.  Note
   that most of this is not actually implemented by BFD.  */

#ifndef _ELF_PRU_H
#define _ELF_PRU_H

#include "elf/reloc-macros.h"

START_RELOC_NUMBERS (elf_pru_reloc_type)
  RELOC_NUMBER (R_PRU_NONE,		    0)
  RELOC_NUMBER (R_PRU_16_PMEM,		    5)
  RELOC_NUMBER (R_PRU_U16_PMEMIMM,	    6)
  RELOC_NUMBER (R_PRU_BFD_RELOC_16,	    8)
  RELOC_NUMBER (R_PRU_U16,		    9)
  RELOC_NUMBER (R_PRU_32_PMEM,		    10)
  RELOC_NUMBER (R_PRU_BFD_RELOC_32,	    11)
  RELOC_NUMBER (R_PRU_S10_PCREL,	    14)
  RELOC_NUMBER (R_PRU_U8_PCREL,		    15)
  RELOC_NUMBER (R_PRU_LDI32,		    18)

  /* Extensions required by GCC, or simply nice to have.  */
  RELOC_NUMBER (R_PRU_GNU_BFD_RELOC_8,	    64)
  RELOC_NUMBER (R_PRU_GNU_DIFF8,	    65)
  RELOC_NUMBER (R_PRU_GNU_DIFF16,	    66)
  RELOC_NUMBER (R_PRU_GNU_DIFF32,	    67)
  RELOC_NUMBER (R_PRU_GNU_DIFF16_PMEM,	    68)
  RELOC_NUMBER (R_PRU_GNU_DIFF32_PMEM,	    69)
  RELOC_NUMBER (R_PRU_ILLEGAL,		    70)
END_RELOC_NUMBERS (R_PRU_maxext)

/* Processor-specific section flags.  */

#endif /* _ELF_PRU_H */
