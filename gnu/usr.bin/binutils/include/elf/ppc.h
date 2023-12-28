/* PPC ELF support for BFD.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.

   By Michael Meissner, Cygnus Support, <meissner@cygnus.com>,
   from information in the System V Application Binary Interface,
   PowerPC Processor Supplement and the PowerPC Embedded Application
   Binary Interface (eabi).

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

/* This file holds definitions specific to the PPC ELF ABI.  Note
   that most of this is not actually implemented by BFD.  */

#ifndef _ELF_PPC_H
#define _ELF_PPC_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_ppc_reloc_type)
  RELOC_NUMBER (R_PPC_NONE,		  0)
  RELOC_NUMBER (R_PPC_ADDR32,		  1)
  RELOC_NUMBER (R_PPC_ADDR24,		  2)
  RELOC_NUMBER (R_PPC_ADDR16,		  3)
  RELOC_NUMBER (R_PPC_ADDR16_LO,	  4)
  RELOC_NUMBER (R_PPC_ADDR16_HI,	  5)
  RELOC_NUMBER (R_PPC_ADDR16_HA,	  6)
  RELOC_NUMBER (R_PPC_ADDR14,		  7)
  RELOC_NUMBER (R_PPC_ADDR14_BRTAKEN,	  8)
  RELOC_NUMBER (R_PPC_ADDR14_BRNTAKEN,	  9)
  RELOC_NUMBER (R_PPC_REL24,		 10)
  RELOC_NUMBER (R_PPC_REL14,		 11)
  RELOC_NUMBER (R_PPC_REL14_BRTAKEN,	 12)
  RELOC_NUMBER (R_PPC_REL14_BRNTAKEN,	 13)
  RELOC_NUMBER (R_PPC_GOT16,		 14)
  RELOC_NUMBER (R_PPC_GOT16_LO,		 15)
  RELOC_NUMBER (R_PPC_GOT16_HI,		 16)
  RELOC_NUMBER (R_PPC_GOT16_HA,		 17)
  RELOC_NUMBER (R_PPC_PLTREL24,		 18)
  RELOC_NUMBER (R_PPC_COPY,		 19)
  RELOC_NUMBER (R_PPC_GLOB_DAT,		 20)
  RELOC_NUMBER (R_PPC_JMP_SLOT,		 21)
  RELOC_NUMBER (R_PPC_RELATIVE,		 22)
  RELOC_NUMBER (R_PPC_LOCAL24PC,	 23)
  RELOC_NUMBER (R_PPC_UADDR32,		 24)
  RELOC_NUMBER (R_PPC_UADDR16,		 25)
  RELOC_NUMBER (R_PPC_REL32,		 26)
  RELOC_NUMBER (R_PPC_PLT32,		 27)
  RELOC_NUMBER (R_PPC_PLTREL32,		 28)
  RELOC_NUMBER (R_PPC_PLT16_LO,		 29)
  RELOC_NUMBER (R_PPC_PLT16_HI,		 30)
  RELOC_NUMBER (R_PPC_PLT16_HA,		 31)
  RELOC_NUMBER (R_PPC_SDAREL16,		 32)
  RELOC_NUMBER (R_PPC_SECTOFF,		 33)
  RELOC_NUMBER (R_PPC_SECTOFF_LO,	 34)
  RELOC_NUMBER (R_PPC_SECTOFF_HI,	 35)
  RELOC_NUMBER (R_PPC_SECTOFF_HA,	 36)
  RELOC_NUMBER (R_PPC_ADDR30,		 37)

#ifndef RELOC_MACROS_GEN_FUNC
/* Relocations only used internally by ld.  If you need to use these
   reloc numbers, you can change them to some other unused value
   without affecting the ABI.  They will never appear in object files.  */
  RELOC_NUMBER (R_PPC_RELAX,		 48)
  RELOC_NUMBER (R_PPC_RELAX_PLT,	 49)
  RELOC_NUMBER (R_PPC_RELAX_PLTREL24,	 50)
/* Reloc only used internally by gas.  As above, value is unimportant.  */
  RELOC_NUMBER (R_PPC_16DX_HA,		 51)
#endif

  /* Relocs added to support TLS.  */
  RELOC_NUMBER (R_PPC_TLS,		 67)
  RELOC_NUMBER (R_PPC_DTPMOD32,		 68)
  RELOC_NUMBER (R_PPC_TPREL16,		 69)
  RELOC_NUMBER (R_PPC_TPREL16_LO,	 70)
  RELOC_NUMBER (R_PPC_TPREL16_HI,	 71)
  RELOC_NUMBER (R_PPC_TPREL16_HA,	 72)
  RELOC_NUMBER (R_PPC_TPREL32,		 73)
  RELOC_NUMBER (R_PPC_DTPREL16,		 74)
  RELOC_NUMBER (R_PPC_DTPREL16_LO,	 75)
  RELOC_NUMBER (R_PPC_DTPREL16_HI,	 76)
  RELOC_NUMBER (R_PPC_DTPREL16_HA,	 77)
  RELOC_NUMBER (R_PPC_DTPREL32,		 78)
  RELOC_NUMBER (R_PPC_GOT_TLSGD16,	 79)
  RELOC_NUMBER (R_PPC_GOT_TLSGD16_LO,	 80)
  RELOC_NUMBER (R_PPC_GOT_TLSGD16_HI,	 81)
  RELOC_NUMBER (R_PPC_GOT_TLSGD16_HA,	 82)
  RELOC_NUMBER (R_PPC_GOT_TLSLD16,	 83)
  RELOC_NUMBER (R_PPC_GOT_TLSLD16_LO,	 84)
  RELOC_NUMBER (R_PPC_GOT_TLSLD16_HI,	 85)
  RELOC_NUMBER (R_PPC_GOT_TLSLD16_HA,	 86)
  RELOC_NUMBER (R_PPC_GOT_TPREL16,	 87)
  RELOC_NUMBER (R_PPC_GOT_TPREL16_LO,	 88)
  RELOC_NUMBER (R_PPC_GOT_TPREL16_HI,	 89)
  RELOC_NUMBER (R_PPC_GOT_TPREL16_HA,	 90)
  RELOC_NUMBER (R_PPC_GOT_DTPREL16,	 91)
  RELOC_NUMBER (R_PPC_GOT_DTPREL16_LO,	 92)
  RELOC_NUMBER (R_PPC_GOT_DTPREL16_HI,	 93)
  RELOC_NUMBER (R_PPC_GOT_DTPREL16_HA,	 94)
  RELOC_NUMBER (R_PPC_TLSGD,		 95)
  RELOC_NUMBER (R_PPC_TLSLD,		 96)

/* The remaining relocs are from the Embedded ELF ABI, and are not
   in the SVR4 ELF ABI.  */
  RELOC_NUMBER (R_PPC_EMB_NADDR32,	101)
  RELOC_NUMBER (R_PPC_EMB_NADDR16,	102)
  RELOC_NUMBER (R_PPC_EMB_NADDR16_LO,	103)
  RELOC_NUMBER (R_PPC_EMB_NADDR16_HI,	104)
  RELOC_NUMBER (R_PPC_EMB_NADDR16_HA,	105)
  RELOC_NUMBER (R_PPC_EMB_SDAI16,	106)
  RELOC_NUMBER (R_PPC_EMB_SDA2I16,	107)
  RELOC_NUMBER (R_PPC_EMB_SDA2REL,	108)
  RELOC_NUMBER (R_PPC_EMB_SDA21,	109)
  RELOC_NUMBER (R_PPC_EMB_MRKREF,	110)
  RELOC_NUMBER (R_PPC_EMB_RELSEC16,	111)
  RELOC_NUMBER (R_PPC_EMB_RELST_LO,	112)
  RELOC_NUMBER (R_PPC_EMB_RELST_HI,	113)
  RELOC_NUMBER (R_PPC_EMB_RELST_HA,	114)
  RELOC_NUMBER (R_PPC_EMB_BIT_FLD,	115)
  RELOC_NUMBER (R_PPC_EMB_RELSDA,	116)

/* Marker reloc for inline plt call insns.  */
  RELOC_NUMBER (R_PPC_PLTSEQ,		119)
  RELOC_NUMBER (R_PPC_PLTCALL,		120)

/* PowerPC VLE relocations.  */
  RELOC_NUMBER (R_PPC_VLE_REL8,		216)
  RELOC_NUMBER (R_PPC_VLE_REL15,	217)
  RELOC_NUMBER (R_PPC_VLE_REL24,	218)
  RELOC_NUMBER (R_PPC_VLE_LO16A,	219)
  RELOC_NUMBER (R_PPC_VLE_LO16D,	220)
  RELOC_NUMBER (R_PPC_VLE_HI16A,	221)
  RELOC_NUMBER (R_PPC_VLE_HI16D,	222)
  RELOC_NUMBER (R_PPC_VLE_HA16A,	223)
  RELOC_NUMBER (R_PPC_VLE_HA16D,	224)
  RELOC_NUMBER (R_PPC_VLE_SDA21,	225)
  RELOC_NUMBER (R_PPC_VLE_SDA21_LO,	226)
  RELOC_NUMBER (R_PPC_VLE_SDAREL_LO16A,	227)
  RELOC_NUMBER (R_PPC_VLE_SDAREL_LO16D,	228)
  RELOC_NUMBER (R_PPC_VLE_SDAREL_HI16A,	229)
  RELOC_NUMBER (R_PPC_VLE_SDAREL_HI16D,	230)
  RELOC_NUMBER (R_PPC_VLE_SDAREL_HA16A,	231)
  RELOC_NUMBER (R_PPC_VLE_SDAREL_HA16D,	232)
  RELOC_NUMBER (R_PPC_VLE_ADDR20,	233)

/* Power9 split rel16 for addpcis.  */
  RELOC_NUMBER (R_PPC_REL16DX_HA,	246)

/* Support STT_GNU_IFUNC plt calls.  */
  RELOC_NUMBER (R_PPC_IRELATIVE,	248)

/* These are GNU extensions used in PIC code sequences.  */
  RELOC_NUMBER (R_PPC_REL16,		249)
  RELOC_NUMBER (R_PPC_REL16_LO,		250)
  RELOC_NUMBER (R_PPC_REL16_HI,		251)
  RELOC_NUMBER (R_PPC_REL16_HA,		252)

/* These are GNU extensions to enable C++ vtable garbage collection.  */
  RELOC_NUMBER (R_PPC_GNU_VTINHERIT,	253)
  RELOC_NUMBER (R_PPC_GNU_VTENTRY,	254)

/* This is a phony reloc to handle any old fashioned TOC16 references
   that may still be in object files.  */
  RELOC_NUMBER (R_PPC_TOC16,		255)

END_RELOC_NUMBERS (R_PPC_max)

#define IS_PPC_TLS_RELOC(R) \
  ((R) >= R_PPC_TLS && (R) <= R_PPC_GOT_DTPREL16_HA)

/* Specify the value of _GLOBAL_OFFSET_TABLE_.  */
#define DT_PPC_GOT		(DT_LOPROC)

/* Specify that tls descriptors should be optimized.  */
#define DT_PPC_OPT		(DT_LOPROC + 1)
#define PPC_OPT_TLS		1

/* Processor specific flags for the ELF header e_flags field.  */

#define	EF_PPC_EMB		0x80000000	/* PowerPC embedded flag.  */

#define	EF_PPC_RELOCATABLE	0x00010000	/* PowerPC -mrelocatable flag.  */
#define	EF_PPC_RELOCATABLE_LIB	0x00008000	/* PowerPC -mrelocatable-lib flag.  */

/* Processor specific program headers, p_flags field.  */
#define PF_PPC_VLE		0x10000000	/* PowerPC VLE.  */

/* Processor specific section headers, sh_flags field.  */
#define SHF_PPC_VLE		0x10000000	/* PowerPC VLE text section.  */

/* Processor specific section headers, sh_type field.  */

#define SHT_ORDERED		SHT_HIPROC	/* Link editor is to sort the \
						   entries in this section \
						   based on the address \
						   specified in the associated \
						   symbol table entry.  */

/* APUinfo note section.  */
#define APUINFO_SECTION_NAME	".PPC.EMB.apuinfo"
#define APUINFO_LABEL		"APUinfo"

#define PPC_APUINFO_ISEL	0x40
#define PPC_APUINFO_PMR		0x41
#define PPC_APUINFO_RFMCI	0x42
#define PPC_APUINFO_CACHELCK	0x43
#define PPC_APUINFO_SPE		0x100
#define PPC_APUINFO_EFS		0x101
#define PPC_APUINFO_BRLOCK	0x102
#define PPC_APUINFO_VLE		0x104

/* Object attribute tags.  */
enum
{
  /* 0-3 are generic.  */

  /* FP ABI, low 2 bits:
     1 for double precision hard-float,
     2 for soft-float,
     3 for single precision hard-float.
     0 for not tagged or not using any ABIs affected by the differences.
     Next 2 bits:
     1 for ibm long double
     2 for 64-bit long double
     3 for IEEE long double.
     0 for not tagged or not using any ABIs affected by the differences.  */
  Tag_GNU_Power_ABI_FP = 4,

  /* Value 1 for general purpose registers only, 2 for AltiVec
     registers, 3 for SPE registers; 0 for not tagged or not using any
     ABIs affected by the differences.  */
  Tag_GNU_Power_ABI_Vector = 8,

  /* Value 1 for ABIs using r3/r4 for returning structures <= 8 bytes,
     2 for ABIs using memory; 0 for not tagged or not using any ABIs
     affected by the differences.  */
  Tag_GNU_Power_ABI_Struct_Return = 12
};

#endif /* _ELF_PPC_H */
