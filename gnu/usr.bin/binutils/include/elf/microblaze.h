/* Xilinx MicroBlaze support for BFD.
 
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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 
   02110-1301, USA.  */

/* This file holds definitions specific to the MICROBLAZE ELF ABI.  */

#ifndef _ELF_MICROBLAZE_H
#define _ELF_MICROBLAZE_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_microblaze_reloc_type)
  RELOC_NUMBER (R_MICROBLAZE_NONE, 0)
  RELOC_NUMBER (R_MICROBLAZE_32, 1)
  RELOC_NUMBER (R_MICROBLAZE_32_PCREL, 2)
  RELOC_NUMBER (R_MICROBLAZE_64_PCREL, 3)
  RELOC_NUMBER (R_MICROBLAZE_32_PCREL_LO, 4)
  RELOC_NUMBER (R_MICROBLAZE_64, 5)
  RELOC_NUMBER (R_MICROBLAZE_32_LO, 6)
  RELOC_NUMBER (R_MICROBLAZE_SRO32, 7)
  RELOC_NUMBER (R_MICROBLAZE_SRW32, 8)
  RELOC_NUMBER (R_MICROBLAZE_64_NONE, 9)
  RELOC_NUMBER (R_MICROBLAZE_32_SYM_OP_SYM, 10)
  RELOC_NUMBER (R_MICROBLAZE_GNU_VTINHERIT, 11)
  RELOC_NUMBER (R_MICROBLAZE_GNU_VTENTRY, 12)
  RELOC_NUMBER (R_MICROBLAZE_GOTPC_64, 13)  /* PC-relative GOT offset.  */
  RELOC_NUMBER (R_MICROBLAZE_GOT_64, 14)    /* GOT entry offset.  */
  RELOC_NUMBER (R_MICROBLAZE_PLT_64, 15)    /* PLT offset (PC-relative).  */
  RELOC_NUMBER (R_MICROBLAZE_REL, 16)       /* Adjust by program base.  */
  RELOC_NUMBER (R_MICROBLAZE_JUMP_SLOT, 17) /* Create PLT entry.  */
  RELOC_NUMBER (R_MICROBLAZE_GLOB_DAT, 18)  /* Create GOT entry.  */
  RELOC_NUMBER (R_MICROBLAZE_GOTOFF_64, 19) /* Offset relative to GOT.  */
  RELOC_NUMBER (R_MICROBLAZE_GOTOFF_32, 20) /* Offset relative to GOT.  */
  RELOC_NUMBER (R_MICROBLAZE_COPY, 21)      /* Runtime copy.  */
  RELOC_NUMBER (R_MICROBLAZE_TLS, 22)           /* TLS Reloc */
  RELOC_NUMBER (R_MICROBLAZE_TLSGD, 23)         /* TLS General Dynamic */
  RELOC_NUMBER (R_MICROBLAZE_TLSLD, 24)         /* TLS Local Dynamic */
  RELOC_NUMBER (R_MICROBLAZE_TLSDTPMOD32, 25)   /* TLS Module ID */
  RELOC_NUMBER (R_MICROBLAZE_TLSDTPREL32, 26)   /* TLS Offset Within TLS Block */
  RELOC_NUMBER (R_MICROBLAZE_TLSDTPREL64, 27)   /* TLS Offset Within TLS Block */
  RELOC_NUMBER (R_MICROBLAZE_TLSGOTTPREL32, 28) /* TLS Offset From Thread Pointer */
  RELOC_NUMBER (R_MICROBLAZE_TLSTPREL32, 29)    /* TLS Offset From Thread Pointer */
  RELOC_NUMBER (R_MICROBLAZE_TEXTPCREL_64, 30)  /* PC-relative TEXT offset.  */
  RELOC_NUMBER (R_MICROBLAZE_TEXTREL_64, 31)    /* TEXT Entry offset 64-bit.  */
  RELOC_NUMBER (R_MICROBLAZE_TEXTREL_32_LO, 32) /* TEXT Entry offset 32-bit.  */
END_RELOC_NUMBERS (R_MICROBLAZE_max)

/* Global base address names.  */
#define RO_SDA_ANCHOR_NAME "_SDA2_BASE_"
#define RW_SDA_ANCHOR_NAME "_SDA_BASE_"

/* Section Attributes.  */
#define SHF_MICROBLAZE_NOREAD	0x80000000

#endif /* _ELF_MICROBLAZE_H */
