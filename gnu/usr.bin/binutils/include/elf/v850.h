/* V850 ELF support for BFD.
   Copyright (C) 1997-2023 Free Software Foundation, Inc.
   Created by Michael Meissner, Cygnus Support <meissner@cygnus.com>

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

/* This file holds definitions specific to the MIPS ELF ABI.  Note
   that most of this is not actually implemented by BFD.  */

#ifndef _ELF_V850_H
#define _ELF_V850_H

/* Processor specific flags for the ELF header e_flags field.  */

/* Four bit V850 architecture field.  */
#define EF_V850_ARCH		0xf0000000

/* v850 code.  */
#define E_V850_ARCH		0x00000000

/* v850e code.  */
#define E_V850E_ARCH		0x10000000

/* v850e1 code.  */
#define E_V850E1_ARCH		0x20000000

/* v850e2 code.  */
#define E_V850E2_ARCH		0x30000000

/* v850e2v3 code.  */
#define E_V850E2V3_ARCH		0x40000000

/* v850e3v5 code.  */
#define E_V850E3V5_ARCH		0x60000000

/* Flags for the st_other field.  */
#define V850_OTHER_SDA		0x10	/* Symbol had SDA relocations.  */
#define V850_OTHER_ZDA		0x20	/* Symbol had ZDA relocations.  */
#define V850_OTHER_TDA		0x40	/* Symbol had TDA relocations.  */
#define V850_OTHER_ERROR	0x80	/* Symbol had an error reported.  */

/* V850 relocations.  */
#include "elf/reloc-macros.h"

START_RELOC_NUMBERS (v850_reloc_type)
     RELOC_NUMBER (R_V850_NONE, 0)
     RELOC_NUMBER (R_V850_9_PCREL, 1)
     RELOC_NUMBER (R_V850_22_PCREL, 2)
     RELOC_NUMBER (R_V850_HI16_S, 3)
     RELOC_NUMBER (R_V850_HI16, 4)
     RELOC_NUMBER (R_V850_LO16, 5)
     RELOC_NUMBER (R_V850_ABS32, 6)
     RELOC_NUMBER (R_V850_16, 7)
     RELOC_NUMBER (R_V850_8, 8)
     RELOC_NUMBER( R_V850_SDA_16_16_OFFSET, 9)		/* For ld.b, st.b, set1, clr1, not1, tst1, movea, movhi */
     RELOC_NUMBER( R_V850_SDA_15_16_OFFSET, 10)		/* For ld.w, ld.h, ld.hu, st.w, st.h */
     RELOC_NUMBER( R_V850_ZDA_16_16_OFFSET, 11)		/* For ld.b, st.b, set1, clr1, not1, tst1, movea, movhi */
     RELOC_NUMBER( R_V850_ZDA_15_16_OFFSET, 12)		/* For ld.w, ld.h, ld.hu, st.w, st.h */
     RELOC_NUMBER( R_V850_TDA_6_8_OFFSET, 13)		/* For sst.w, sld.w */
     RELOC_NUMBER( R_V850_TDA_7_8_OFFSET, 14)		/* For sst.h, sld.h */
     RELOC_NUMBER( R_V850_TDA_7_7_OFFSET, 15)		/* For sst.b, sld.b */
     RELOC_NUMBER( R_V850_TDA_16_16_OFFSET, 16)		/* For set1, clr1, not1, tst1, movea, movhi */
     RELOC_NUMBER( R_V850_TDA_4_5_OFFSET, 17)		/* For sld.hu */
     RELOC_NUMBER( R_V850_TDA_4_4_OFFSET, 18)		/* For sld.bu */
     RELOC_NUMBER( R_V850_SDA_16_16_SPLIT_OFFSET, 19)	/* For ld.bu */
     RELOC_NUMBER( R_V850_ZDA_16_16_SPLIT_OFFSET, 20)	/* For ld.bu */
     RELOC_NUMBER( R_V850_CALLT_6_7_OFFSET, 21)		/* For callt */
     RELOC_NUMBER( R_V850_CALLT_16_16_OFFSET, 22)	/* For callt */
     RELOC_NUMBER (R_V850_GNU_VTINHERIT, 23)
     RELOC_NUMBER (R_V850_GNU_VTENTRY, 24)
     RELOC_NUMBER (R_V850_LONGCALL, 25)
     RELOC_NUMBER (R_V850_LONGJUMP, 26)
     RELOC_NUMBER (R_V850_ALIGN, 27)
     RELOC_NUMBER (R_V850_REL32, 28)
     RELOC_NUMBER (R_V850_LO16_SPLIT_OFFSET, 29)	/* For ld.bu */
     RELOC_NUMBER (R_V850_16_PCREL, 30)      		/* For loop */
     RELOC_NUMBER (R_V850_17_PCREL, 31)      		/* For br */
     RELOC_NUMBER (R_V850_23, 32)			/* For 23bit ld.[w,h,hu,b,bu],st.[w,h,b] */
     RELOC_NUMBER (R_V850_32_PCREL, 33)      		/* For jr32, jarl32 */
     RELOC_NUMBER (R_V850_32_ABS, 34)      		/* For jmp32 */
     RELOC_NUMBER (R_V850_16_SPLIT_OFFSET, 35)      	/* For ld.bu */
     RELOC_NUMBER (R_V850_16_S1, 36)      		/* For ld.w, ld.h st.w st.h */
     RELOC_NUMBER (R_V850_LO16_S1, 37)      		/* For ld.w, ld.h st.w st.h */
     RELOC_NUMBER (R_V850_CALLT_15_16_OFFSET, 38)	/* For ld.w, ld.h, ld.hu, st.w, st.h */
     RELOC_NUMBER (R_V850_32_GOTPCREL, 39)		/* GLOBAL_OFFSET_TABLE from pc */
     RELOC_NUMBER (R_V850_16_GOT, 40)      		/* GOT ENTRY from gp */
     RELOC_NUMBER (R_V850_32_GOT, 41)      		
     RELOC_NUMBER (R_V850_22_PLT, 42)			/* For jr */
     RELOC_NUMBER (R_V850_32_PLT, 43)			/* For jr32 */
     RELOC_NUMBER (R_V850_COPY, 44)      		
     RELOC_NUMBER (R_V850_GLOB_DAT, 45)      		
     RELOC_NUMBER (R_V850_JMP_SLOT, 46)      		
     RELOC_NUMBER (R_V850_RELATIVE, 47)      		
     RELOC_NUMBER (R_V850_16_GOTOFF, 48)      		/* From gp */
     RELOC_NUMBER (R_V850_32_GOTOFF, 49)      		
     RELOC_NUMBER (R_V850_CODE, 50)      		
     RELOC_NUMBER (R_V850_DATA, 51)      		/* For loop */

END_RELOC_NUMBERS (R_V850_max)


/* Processor specific section indices.  These sections do not actually
   exist.  Symbols with a st_shndx field corresponding to one of these
   values have a special meaning.  */

/* Small data area common symbol.  */
#define SHN_V850_SCOMMON	SHN_LORESERVE

/* Tiny data area common symbol.  */
#define SHN_V850_TCOMMON	(SHN_LORESERVE + 1)

/* Zero data area common symbol.  */
#define SHN_V850_ZCOMMON	(SHN_LORESERVE + 2)


/* Processor specific section types.  */

/* Section contains the .scommon data.  */
#define SHT_V850_SCOMMON	0x70000000

/* Section contains the .scommon data.  */
#define SHT_V850_TCOMMON	0x70000001

/* Section contains the .scommon data.  */
#define SHT_V850_ZCOMMON	0x70000002

/* Processor specific section flags.  */

/* This section must be in the small data area (pointed to by GP).  */
#define SHF_V850_GPREL		0x10000000

/* This section must be in the tiny data area (pointed to by EP).  */
#define SHF_V850_EPREL		0x20000000

/* This section must be in the zero data area (pointed to by R0).  */
#define SHF_V850_R0REL		0x40000000

/* Alternative versions of the above definitions, as specified by the RH850 ABI.  */

#define EF_RH850_ABI		0xF0000000

#define EF_V800_850E3		0x00100000

#define EF_RH850_FPU_DOUBLE	0x00000001	/* sizeof(double) == 8.  */
#define EF_RH850_FPU_SINGLE	0x00000002	/* sizeof(double) == 4.  */
#define EF_RH850_REGMODE22	0x00000020	/* Registers r15-r24 (inclusive) are not used.  */
#define EF_RH850_REGMODE32	0x00000040
#define EF_RH850_GP_FIX		0x00000100	/* r4 is fixed.  */
#define EF_RH850_GP_NOFIX	0x00000200	/* r4 is callee save.  */
#define EF_RH850_EP_FIX		0x00000400	/* r30 is fixed.  */
#define EF_RH850_EP_NOFIX	0x00000800	/* r30 is callee save.  */
#define EF_RH850_TP_FIX		0x00001000	/* r5 is fixed.  */
#define EF_RH850_TP_NOFIX	0x00002000	/* r5 is callee save.  */
#define EF_RH850_REG2_RESERVE	0x00004000	/* r2 is fixed.  */
#define EF_RH850_REG2_NORESERVE 0x00008000	/* r2 is callee saved.  */

#define SHT_RENESAS_IOP		SHT_LOUSER	/* Used by Renesas linker.  */

#define SHF_RENESAS_ABS		0x80000000	/* Absolute section.  */
#define SHF_GHS_ABS		0x00000400	/* Use unknown.  */

#define STT_RENESAS_ENTRY	14		/* Set for functions called at reset time.  */

START_RELOC_NUMBERS (v800_reloc_type)

     RELOC_NUMBER (R_V800_NONE,      0x00)
     RELOC_NUMBER (R_V810_NONE,      0x30)
     RELOC_NUMBER (R_V810_BYTE,      0x31)
     RELOC_NUMBER (R_V810_HWORD,     0x32)
     RELOC_NUMBER (R_V810_WORD,      0x33)
     RELOC_NUMBER (R_V810_WLO,       0x34)
     RELOC_NUMBER (R_V810_WHI,       0x35)
     RELOC_NUMBER (R_V810_WHI1,      0x36)
     RELOC_NUMBER (R_V810_GPBYTE,    0x37)
     RELOC_NUMBER (R_V810_GPHWORD,   0x38)
     RELOC_NUMBER (R_V810_GPWORD,    0x39)
     RELOC_NUMBER (R_V810_GPWLO,     0x3a)
     RELOC_NUMBER (R_V810_GPWHI,     0x3b)
     RELOC_NUMBER (R_V810_GPWHI1,    0x3c)
     RELOC_NUMBER (R_V850_HWLO,      0x3d)
     FAKE_RELOC   (R_V810_reserved1, 0x3e)
     RELOC_NUMBER (R_V850_EP7BIT,    0x3f)
     RELOC_NUMBER (R_V850_EPHBYTE,   0x40)
     RELOC_NUMBER (R_V850_EPWBYTE,   0x41)
     RELOC_NUMBER (R_V850_REGHWLO,   0x42)
     FAKE_RELOC   (R_V810_reserved2, 0x43)
     RELOC_NUMBER (R_V850_GPHWLO,    0x44)
     FAKE_RELOC   (R_V810_reserved3, 0x45)
     RELOC_NUMBER (R_V850_PCR22,     0x46)
     RELOC_NUMBER (R_V850_BLO,       0x47)
     RELOC_NUMBER (R_V850_EP4BIT,    0x48)
     RELOC_NUMBER (R_V850_EP5BIT,    0x49)
     RELOC_NUMBER (R_V850_REGBLO,    0x4a)
     RELOC_NUMBER (R_V850_GPBLO,     0x4b)
     RELOC_NUMBER (R_V810_WLO_1,     0x4c)
     RELOC_NUMBER (R_V810_GPWLO_1,   0x4d)
     RELOC_NUMBER (R_V850_BLO_1,     0x4e)
     RELOC_NUMBER (R_V850_HWLO_1,    0x4f)
     FAKE_RELOC   (R_V810_reserved4, 0x50)
     RELOC_NUMBER (R_V850_GPBLO_1,   0x51)
     RELOC_NUMBER (R_V850_GPHWLO_1,  0x52)
     FAKE_RELOC   (R_V810_reserved5, 0x53)
     RELOC_NUMBER (R_V850_EPBLO,     0x54)
     RELOC_NUMBER (R_V850_EPHWLO,    0x55)
     FAKE_RELOC   (R_V810_reserved6, 0x56)
     RELOC_NUMBER (R_V850_EPWLO_N,   0x57)
     RELOC_NUMBER (R_V850_PC32,      0x58)
     RELOC_NUMBER (R_V850_W23BIT,    0x59)
     RELOC_NUMBER (R_V850_GPW23BIT,  0x5a)
     RELOC_NUMBER (R_V850_EPW23BIT,  0x5b)
     RELOC_NUMBER (R_V850_B23BIT,    0x5c)
     RELOC_NUMBER (R_V850_GPB23BIT,  0x5d)
     RELOC_NUMBER (R_V850_EPB23BIT,  0x5e)
     RELOC_NUMBER (R_V850_PC16U,     0x5f)
     RELOC_NUMBER (R_V850_PC17,      0x60)
     RELOC_NUMBER (R_V850_DW8,       0x61)
     RELOC_NUMBER (R_V850_GPDW8,     0x62)
     RELOC_NUMBER (R_V850_EPDW8,     0x63)
     RELOC_NUMBER (R_V850_PC9,       0x64)
     RELOC_NUMBER (R_V810_REGBYTE,   0x65)
     RELOC_NUMBER (R_V810_REGHWORD,  0x66)
     RELOC_NUMBER (R_V810_REGWORD,   0x67)
     RELOC_NUMBER (R_V810_REGWLO,    0x68)
     RELOC_NUMBER (R_V810_REGWHI,    0x69)
     RELOC_NUMBER (R_V810_REGWHI1,   0x6a)
     RELOC_NUMBER (R_V850_REGW23BIT, 0x6b)
     RELOC_NUMBER (R_V850_REGB23BIT, 0x6c)
     RELOC_NUMBER (R_V850_REGDW8,    0x6d)
     RELOC_NUMBER (R_V810_EPBYTE,    0x6e)
     RELOC_NUMBER (R_V810_EPHWORD,   0x6f)
     RELOC_NUMBER (R_V810_EPWORD,    0x70)
     RELOC_NUMBER (R_V850_WLO23,     0x71)
     RELOC_NUMBER (R_V850_WORD_E,    0x72)
     RELOC_NUMBER (R_V850_REGWORD_E, 0x73)
     RELOC_NUMBER (R_V850_WORD,      0x74)
     RELOC_NUMBER (R_V850_GPWORD,    0x75)
     RELOC_NUMBER (R_V850_REGWORD,   0x76)
     RELOC_NUMBER (R_V850_EPWORD,    0x77)
     RELOC_NUMBER (R_V810_TPBYTE,    0x78)
     RELOC_NUMBER (R_V810_TPHWORD,   0x79)
     RELOC_NUMBER (R_V810_TPWORD,    0x7a)
     RELOC_NUMBER (R_V810_TPWLO,     0x7b)
     RELOC_NUMBER (R_V810_TPWHI,     0x7c)
     RELOC_NUMBER (R_V810_TPWHI1,    0x7d)
     RELOC_NUMBER (R_V850_TPHWLO,    0x7e)
     RELOC_NUMBER (R_V850_TPBLO,     0x7f)
     RELOC_NUMBER (R_V810_TPWLO_1,   0x80)
     RELOC_NUMBER (R_V850_TPBLO_1,   0x81)
     RELOC_NUMBER (R_V850_TPHWLO_1,  0x82)
     RELOC_NUMBER (R_V850_TP23BIT,   0x83)
     RELOC_NUMBER (R_V850_TPW23BIT,  0x84)
     RELOC_NUMBER (R_V850_TPDW8,     0x85)

/* These are defined by the RH850 ABI, but not used.  */
     RELOC_NUMBER (R_V810_ABS32,     0xa0)
     RELOC_NUMBER (R_V850_SYM,       0xe0)
     RELOC_NUMBER (R_V850_OPadd,     0xe1)
     RELOC_NUMBER (R_V850_OPsub,     0xe2)
     RELOC_NUMBER (R_V850_OPsctsize, 0xe3)
     RELOC_NUMBER (R_V850_OPscttop,  0xe4)

END_RELOC_NUMBERS (R_V800_max)

/* Type for Renesas note sections.  NB/ This is in application space
   rather than processor space as it refers to the requirements of the
   binary concerned.  A given processor may be able to handle multiple
   different types of application.  */
#define SHT_RENESAS_INFO	0xa0000000

/* Contents of a Renesas note entry:

     namesz +------------------+
            |    4             |  "REL\0"
     descsz +------------------+
            |    4             |  Currently 4byte only
     type   +------------------+
            |    ID            |
     name   +------------------+
            |    REL\0         |
     desc   +------------------+
            |    Value         |
            +------------------+  */

#define V850_NOTE_SECNAME	".note.renesas"
#define SIZEOF_V850_NOTE	20
#define V850_NOTE_NAME		"REL"

enum v850_notes
{
  V850_NOTE_ALIGNMENT =	1,	/* Alignment of 8-byte entities.  */
#define EF_RH850_DATA_ALIGN4	0x0001	/* Aligned to 4-byte bounadries.  */
#define EF_RH850_DATA_ALIGN8	0x0002	/* Aligned to 8-byte bounadries.  */

  V850_NOTE_DATA_SIZE =	2,	/* Sizeof double and long double.  */
#define EF_RH850_DOUBLE32	0x0001	/* 32-bits in size.  */
#define EF_RH850_DOUBLE64	0x0002	/* 64-bits in size.  */

  V850_NOTE_FPU_INFO = 3,	/* Defined if extended floating point insns are used.  */
#define EF_RH850_FPU20		0x0001	/* Set if [N]]M{ADD|SUB}F.S are used.  */
#define EF_RH850_FPU30		0x0002	/* Set if ADSF.D or ADDF.D is used.  */

  V850_NOTE_SIMD_INFO = 4,
#define EF_RH850_SIMD		0x0001

  V850_NOTE_CACHE_INFO = 5,
#define EF_RH850_CACHE		0x0001

  V850_NOTE_MMU_INFO = 6
#define EF_RH850_MMU		0x0001
};

#define NUM_V850_NOTES	V850_NOTE_MMU_INFO

#endif /* _ELF_V850_H */
