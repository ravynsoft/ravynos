/* Altera Nios II ELF support for BFD.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Nigel Gray (ngray@altera.com).
   Contributed by Mentor Graphics, Inc.

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


/* This file holds definitions specific to the Altera Nios II ELF ABI.  Note
   that most of this is not actually implemented by BFD.  */

#ifndef _ELF_NIOS2_H
#define _ELF_NIOS2_H

#include "elf/reloc-macros.h"

/* The order of these numbers must match the order in
   the elf_nios2_howto_table_rel table for the lookup
   function to work properly.  */

START_RELOC_NUMBERS (elf_nios2_reloc_type)
  /* Relocs used by both R1 and R2, with different howtos to match
     the respective encodings.  */
  RELOC_NUMBER (R_NIOS2_NONE,	  0)
  RELOC_NUMBER (R_NIOS2_S16,	  1)
  RELOC_NUMBER (R_NIOS2_U16,	  2)
  RELOC_NUMBER (R_NIOS2_PCREL16, 3)
  RELOC_NUMBER (R_NIOS2_CALL26,  4)
  RELOC_NUMBER (R_NIOS2_IMM5,	  5)
  RELOC_NUMBER (R_NIOS2_CACHE_OPX, 6)
  RELOC_NUMBER (R_NIOS2_IMM6,	  7)
  RELOC_NUMBER (R_NIOS2_IMM8,	  8)
  RELOC_NUMBER (R_NIOS2_HI16,	  9)
  RELOC_NUMBER (R_NIOS2_LO16,	  10)
  RELOC_NUMBER (R_NIOS2_HIADJ16, 11)
  RELOC_NUMBER (R_NIOS2_BFD_RELOC_32, 12)
  RELOC_NUMBER (R_NIOS2_BFD_RELOC_16, 13)
  RELOC_NUMBER (R_NIOS2_BFD_RELOC_8, 14)
  RELOC_NUMBER (R_NIOS2_GPREL, 15)
  RELOC_NUMBER (R_NIOS2_GNU_VTINHERIT, 16)
  RELOC_NUMBER (R_NIOS2_GNU_VTENTRY, 17)
  RELOC_NUMBER (R_NIOS2_UJMP, 18)
  RELOC_NUMBER (R_NIOS2_CJMP, 19)
  RELOC_NUMBER (R_NIOS2_CALLR, 20)
  RELOC_NUMBER (R_NIOS2_ALIGN, 21)
  RELOC_NUMBER (R_NIOS2_GOT16, 22)
  RELOC_NUMBER (R_NIOS2_CALL16, 23)
  RELOC_NUMBER (R_NIOS2_GOTOFF_LO, 24)
  RELOC_NUMBER (R_NIOS2_GOTOFF_HA, 25)
  RELOC_NUMBER (R_NIOS2_PCREL_LO, 26)
  RELOC_NUMBER (R_NIOS2_PCREL_HA, 27)
  RELOC_NUMBER (R_NIOS2_TLS_GD16, 28)
  RELOC_NUMBER (R_NIOS2_TLS_LDM16, 29)
  RELOC_NUMBER (R_NIOS2_TLS_LDO16, 30)
  RELOC_NUMBER (R_NIOS2_TLS_IE16, 31)
  RELOC_NUMBER (R_NIOS2_TLS_LE16, 32)
  RELOC_NUMBER (R_NIOS2_TLS_DTPMOD, 33)
  RELOC_NUMBER (R_NIOS2_TLS_DTPREL, 34)
  RELOC_NUMBER (R_NIOS2_TLS_TPREL, 35)
  RELOC_NUMBER (R_NIOS2_COPY, 36)
  RELOC_NUMBER (R_NIOS2_GLOB_DAT, 37)
  RELOC_NUMBER (R_NIOS2_JUMP_SLOT, 38)
  RELOC_NUMBER (R_NIOS2_RELATIVE, 39)
  RELOC_NUMBER (R_NIOS2_GOTOFF, 40)
  RELOC_NUMBER (R_NIOS2_CALL26_NOAT,  41)
  RELOC_NUMBER (R_NIOS2_GOT_LO, 42)
  RELOC_NUMBER (R_NIOS2_GOT_HA, 43)
  RELOC_NUMBER (R_NIOS2_CALL_LO, 44)
  RELOC_NUMBER (R_NIOS2_CALL_HA, 45)

  /* Relocs specific to R2.  */
  RELOC_NUMBER (R_NIOS2_R2_S12, 64)
  RELOC_NUMBER (R_NIOS2_R2_I10_1_PCREL, 65)
  RELOC_NUMBER (R_NIOS2_R2_T1I7_1_PCREL, 66)
  RELOC_NUMBER (R_NIOS2_R2_T1I7_2, 67)
  RELOC_NUMBER (R_NIOS2_R2_T2I4, 68)
  RELOC_NUMBER (R_NIOS2_R2_T2I4_1, 69)
  RELOC_NUMBER (R_NIOS2_R2_T2I4_2, 70)
  RELOC_NUMBER (R_NIOS2_R2_X1I7_2, 71)
  RELOC_NUMBER (R_NIOS2_R2_X2L5, 72)
  RELOC_NUMBER (R_NIOS2_R2_F1I5_2, 73)
  RELOC_NUMBER (R_NIOS2_R2_L5I4X1, 74)
  RELOC_NUMBER (R_NIOS2_R2_T1X1I6, 75)
  RELOC_NUMBER (R_NIOS2_R2_T1X1I6_2, 76)

  /* Last reloc.  */
  RELOC_NUMBER (R_NIOS2_ILLEGAL, 77)
END_RELOC_NUMBERS (R_NIOS2_maxext)

/* Processor-specific section flags.  */

/* This is used to mark gp-relative sections.  */
#define SHF_NIOS2_GPREL	0x10000000

/* Processor-specific dynamic array tags.  */

/* Address of _gp.  */
#define DT_NIOS2_GP 0x70000002

/* Processor specific flags for the Elf header e_flags field.  */

#define EF_NIOS2_ARCH_R1 0x00000000
#define EF_NIOS2_ARCH_R2 0x00000001

#endif /* _ELF_NIOS2_H */
