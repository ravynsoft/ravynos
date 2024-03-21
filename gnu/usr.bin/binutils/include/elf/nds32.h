/* NDS32 ELF support for BFD.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Andes Technology Corporation.

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

#ifndef _ELF_NDS32_H
#define _ELF_NDS32_H

#include "elf/reloc-macros.h"

START_RELOC_NUMBERS (elf_nds32_reloc_type)
  /* These used for relocations.  */
  RELOC_NUMBER (R_NDS32_NONE, 0)
  /* REL relocations.  */
  RELOC_NUMBER (R_NDS32_16, 1)
  RELOC_NUMBER (R_NDS32_32, 2)
  RELOC_NUMBER (R_NDS32_20, 3)
  RELOC_NUMBER (R_NDS32_9_PCREL, 4)
  RELOC_NUMBER (R_NDS32_15_PCREL, 5)
  RELOC_NUMBER (R_NDS32_17_PCREL, 6)
  RELOC_NUMBER (R_NDS32_25_PCREL, 7)
  RELOC_NUMBER (R_NDS32_HI20, 8)
  RELOC_NUMBER (R_NDS32_LO12S3, 9)
  RELOC_NUMBER (R_NDS32_LO12S2, 10)
  RELOC_NUMBER (R_NDS32_LO12S1, 11)
  RELOC_NUMBER (R_NDS32_LO12S0, 12)
  RELOC_NUMBER (R_NDS32_SDA15S3, 13)
  RELOC_NUMBER (R_NDS32_SDA15S2, 14)
  RELOC_NUMBER (R_NDS32_SDA15S1, 15)
  RELOC_NUMBER (R_NDS32_SDA15S0, 16)
  RELOC_NUMBER (R_NDS32_GNU_VTINHERIT, 17)
  RELOC_NUMBER (R_NDS32_GNU_VTENTRY, 18)
  /* RELA relocations.  */
  RELOC_NUMBER (R_NDS32_16_RELA, 19)
  RELOC_NUMBER (R_NDS32_32_RELA, 20)
  RELOC_NUMBER (R_NDS32_20_RELA, 21)
  RELOC_NUMBER (R_NDS32_9_PCREL_RELA, 22)
  RELOC_NUMBER (R_NDS32_15_PCREL_RELA, 23)
  RELOC_NUMBER (R_NDS32_17_PCREL_RELA, 24)
  RELOC_NUMBER (R_NDS32_25_PCREL_RELA, 25)
  RELOC_NUMBER (R_NDS32_HI20_RELA, 26)
  RELOC_NUMBER (R_NDS32_LO12S3_RELA, 27)
  RELOC_NUMBER (R_NDS32_LO12S2_RELA, 28)
  RELOC_NUMBER (R_NDS32_LO12S1_RELA, 29)
  RELOC_NUMBER (R_NDS32_LO12S0_RELA, 30)
  RELOC_NUMBER (R_NDS32_SDA15S3_RELA, 31)
  RELOC_NUMBER (R_NDS32_SDA15S2_RELA, 32)
  RELOC_NUMBER (R_NDS32_SDA15S1_RELA, 33)
  RELOC_NUMBER (R_NDS32_SDA15S0_RELA, 34)
  RELOC_NUMBER (R_NDS32_RELA_GNU_VTINHERIT, 35)
  RELOC_NUMBER (R_NDS32_RELA_GNU_VTENTRY, 36)
  /* GOT and PLT.  */
  RELOC_NUMBER (R_NDS32_GOT20, 37)
  RELOC_NUMBER (R_NDS32_25_PLTREL, 38)
  RELOC_NUMBER (R_NDS32_COPY, 39)
  RELOC_NUMBER (R_NDS32_GLOB_DAT, 40)
  RELOC_NUMBER (R_NDS32_JMP_SLOT, 41)
  RELOC_NUMBER (R_NDS32_RELATIVE, 42)
  RELOC_NUMBER (R_NDS32_GOTOFF, 43)
  RELOC_NUMBER (R_NDS32_GOTPC20, 44)
  RELOC_NUMBER (R_NDS32_GOT_HI20, 45)
  RELOC_NUMBER (R_NDS32_GOT_LO12, 46)
  RELOC_NUMBER (R_NDS32_GOTPC_HI20, 47)
  RELOC_NUMBER (R_NDS32_GOTPC_LO12, 48)
  RELOC_NUMBER (R_NDS32_GOTOFF_HI20, 49)
  RELOC_NUMBER (R_NDS32_GOTOFF_LO12, 50)
  /* 32_to_16 relaxations.  */
  RELOC_NUMBER (R_NDS32_INSN16, 51)
  /* Alignment tag.  */
  RELOC_NUMBER (R_NDS32_LABEL, 52)
  RELOC_NUMBER (R_NDS32_LONGCALL1, 53)		/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_LONGCALL2, 54)		/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_LONGCALL3, 55)		/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_LONGJUMP1, 56)		/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_LONGJUMP2, 57)		/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_LONGJUMP3, 58)		/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_LOADSTORE, 59)		/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_9_FIXED_RELA, 60)
  RELOC_NUMBER (R_NDS32_15_FIXED_RELA, 61)
  RELOC_NUMBER (R_NDS32_17_FIXED_RELA, 62)
  RELOC_NUMBER (R_NDS32_25_FIXED_RELA, 63)
  RELOC_NUMBER (R_NDS32_PLTREL_HI20, 64)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_PLTREL_LO12, 65)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_PLT_GOTREL_HI20, 66)
  RELOC_NUMBER (R_NDS32_PLT_GOTREL_LO12, 67)
  RELOC_NUMBER (R_NDS32_SDA12S2_DP_RELA, 68)
  RELOC_NUMBER (R_NDS32_SDA12S2_SP_RELA, 69)
  RELOC_NUMBER (R_NDS32_LO12S2_DP_RELA, 70)
  RELOC_NUMBER (R_NDS32_LO12S2_SP_RELA, 71)
  RELOC_NUMBER (R_NDS32_LO12S0_ORI_RELA, 72)
  RELOC_NUMBER (R_NDS32_SDA16S3_RELA, 73)
  RELOC_NUMBER (R_NDS32_SDA17S2_RELA, 74)
  RELOC_NUMBER (R_NDS32_SDA18S1_RELA, 75)
  RELOC_NUMBER (R_NDS32_SDA19S0_RELA, 76)
  RELOC_NUMBER (R_NDS32_DWARF2_OP1_RELA, 77)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_DWARF2_OP2_RELA, 78)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_DWARF2_LEB_RELA, 79)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_UPDATE_TA_RELA, 80)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_9_PLTREL, 81)
  RELOC_NUMBER (R_NDS32_PLT_GOTREL_LO20, 82)
  RELOC_NUMBER (R_NDS32_PLT_GOTREL_LO15, 83)
  RELOC_NUMBER (R_NDS32_PLT_GOTREL_LO19, 84)
  RELOC_NUMBER (R_NDS32_GOT_LO15, 85)
  RELOC_NUMBER (R_NDS32_GOT_LO19, 86)
  RELOC_NUMBER (R_NDS32_GOTOFF_LO15, 87)
  RELOC_NUMBER (R_NDS32_GOTOFF_LO19, 88)
  RELOC_NUMBER (R_NDS32_GOT15S2_RELA, 89)
  RELOC_NUMBER (R_NDS32_GOT17S2_RELA, 90)
  RELOC_NUMBER (R_NDS32_5_RELA, 91)
  RELOC_NUMBER (R_NDS32_10_UPCREL_RELA, 92)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_SDA_FP7U2_RELA, 93)
  RELOC_NUMBER (R_NDS32_WORD_9_PCREL_RELA, 94)
  RELOC_NUMBER (R_NDS32_25_ABS_RELA, 95)
  RELOC_NUMBER (R_NDS32_17IFC_PCREL_RELA, 96)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_10IFCU_PCREL_RELA, 97)	/* This is obsoleted.  */
  /* TLS support.  */
  RELOC_NUMBER (R_NDS32_TLS_LE_HI20, 98)
  RELOC_NUMBER (R_NDS32_TLS_LE_LO12, 99)
  RELOC_NUMBER (R_NDS32_TLS_IE_HI20, 100)
  RELOC_NUMBER (R_NDS32_TLS_IE_LO12S2, 101)
  RELOC_NUMBER (R_NDS32_TLS_TPOFF, 102)
  RELOC_NUMBER (R_NDS32_TLS_LE_20, 103)
  RELOC_NUMBER (R_NDS32_TLS_LE_15S0, 104)
  RELOC_NUMBER (R_NDS32_TLS_LE_15S1, 105)
  RELOC_NUMBER (R_NDS32_TLS_LE_15S2, 106)
  RELOC_NUMBER (R_NDS32_LONGCALL4, 107)
  RELOC_NUMBER (R_NDS32_LONGCALL5, 108)
  RELOC_NUMBER (R_NDS32_LONGCALL6, 109)
  RELOC_NUMBER (R_NDS32_LONGJUMP4, 110)
  RELOC_NUMBER (R_NDS32_LONGJUMP5, 111)
  RELOC_NUMBER (R_NDS32_LONGJUMP6, 112)
  RELOC_NUMBER (R_NDS32_LONGJUMP7, 113)
  /* Reserved numbers: 114.  */
  /* TLS support */
  RELOC_NUMBER (R_NDS32_TLS_IE_LO12, 115)
  RELOC_NUMBER (R_NDS32_TLS_IEGP_HI20, 116)
  RELOC_NUMBER (R_NDS32_TLS_IEGP_LO12, 117)
  RELOC_NUMBER (R_NDS32_TLS_IEGP_LO12S2, 118)
  RELOC_NUMBER (R_NDS32_TLS_DESC, 119)
  RELOC_NUMBER (R_NDS32_TLS_DESC_HI20, 120)
  RELOC_NUMBER (R_NDS32_TLS_DESC_LO12, 121)
  RELOC_NUMBER (R_NDS32_TLS_DESC_20, 122)
  RELOC_NUMBER (R_NDS32_TLS_DESC_SDA17S2, 123)
  /* Reserved numbers: 124-191.  */

  /* These used only for relaxations  */
  RELOC_NUMBER (R_NDS32_RELAX_ENTRY, 192)
  RELOC_NUMBER (R_NDS32_GOT_SUFF, 193)
  RELOC_NUMBER (R_NDS32_GOTOFF_SUFF, 194)
  RELOC_NUMBER (R_NDS32_PLT_GOT_SUFF, 195)
  RELOC_NUMBER (R_NDS32_MULCALL_SUFF, 196)	/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_PTR, 197)
  RELOC_NUMBER (R_NDS32_PTR_COUNT, 198)
  RELOC_NUMBER (R_NDS32_PTR_RESOLVED, 199)
  RELOC_NUMBER (R_NDS32_PLTBLOCK, 200)		/* This is obsoleted.  */
  RELOC_NUMBER (R_NDS32_RELAX_REGION_BEGIN, 201)
  RELOC_NUMBER (R_NDS32_RELAX_REGION_END, 202)
  RELOC_NUMBER (R_NDS32_MINUEND, 203)
  RELOC_NUMBER (R_NDS32_SUBTRAHEND, 204)
  RELOC_NUMBER (R_NDS32_DIFF8, 205)
  RELOC_NUMBER (R_NDS32_DIFF16, 206)
  RELOC_NUMBER (R_NDS32_DIFF32, 207)
  RELOC_NUMBER (R_NDS32_DIFF_ULEB128, 208)
  RELOC_NUMBER (R_NDS32_DATA, 209)
  RELOC_NUMBER (R_NDS32_TRAN, 210)
  /* TLS support */
  RELOC_NUMBER (R_NDS32_TLS_LE_ADD, 211)
  RELOC_NUMBER (R_NDS32_TLS_LE_LS, 212)
  RELOC_NUMBER (R_NDS32_EMPTY, 213)
  RELOC_NUMBER (R_NDS32_TLS_DESC_ADD, 214)
  RELOC_NUMBER (R_NDS32_TLS_DESC_FUNC, 215)
  RELOC_NUMBER (R_NDS32_TLS_DESC_CALL, 216)
  RELOC_NUMBER (R_NDS32_TLS_DESC_MEM, 217)
  RELOC_NUMBER (R_NDS32_RELAX_REMOVE, 218)
  RELOC_NUMBER (R_NDS32_RELAX_GROUP, 219)
  RELOC_NUMBER (R_NDS32_TLS_IEGP_LW, 220)
  RELOC_NUMBER (R_NDS32_LSI, 221)
  /* Reserved numbers: 222-255.  */

END_RELOC_NUMBERS (R_NDS32_max)

/* Processor specific section indices.  These sections do not actually
   exist.  Symbols with a st_shndx field corresponding to one of these
   values have a special meaning.  */

/* Processor specific flags for the ELF header e_flags field.

   31   28 27		      8 7   4 3       0
   ---------------------------------------------
   | ARCH | CONFUGURAION FIELD | ABI | ELF_VER |
   ---------------------------------------------  */

/* Architechure definition.  */

/* 4-bit (b31-b28) nds32 architecture field.
   We can have up to 15 architectures; 0000 is for unknown.  */
#define EF_NDS_ARCH				0xF0000000
#define EF_NDS_ARCH_SHIFT			28
/* There could be more architectures. For now, only n1 and n1h.  */
#define E_NDS_ARCH_STAR_RESERVED		0x00000000
#define E_NDS_ARCH_STAR_V1_0			0x10000000
#define E_NDS_ARCH_STAR_V2_0			0x20000000
#define E_NDS_ARCH_STAR_V3_0			0x30000000
#define E_NDS_ARCH_STAR_V3_M			0x40000000
#define E_NDS_ARCH_STAR_V0_9			0x90000000	/* Obsoleted.  */
/* n1 code.  */
#define E_N1_ARCH			E_NDS_ARCH_STAR_V0_9
/* n1h code.  */
#define E_N1H_ARCH			E_NDS_ARCH_STAR_V1_0


/* Configuration field definitioans.  */
#define EF_NDS_INST				0x0FFFFF00

/* E_NDS_ARCH_STAR_V1_0 configuration fields.

   E_NDS_ARCH_STAR_V2_0 configuration fields.
   These are discarded in v2.
     * E_NDS32_HAS_MFUSR_PC_INST	0x00000100
     * E_NDS32_HAS_DIV_INST		0x00002000
     * E_NDS32_HAS_NO_MAC_INST		0x00100000
   These are added in v2.
     * E_NDS32_HAS_DIV_DX_INST		0x00002000
     * E_NDS32_HAS_MAC_DX_INST		0x00100000  */

/* MFUSR rt, PC and correct ISYNC, MSYNC instructions.
   Old N1213HC has no such instructions.  */
#define E_NDS32_HAS_MFUSR_PC_INST		0x00000100 /* Reclaimed.  */
/* C/C++ performance extension instructions.  */
#define E_NDS32_HAS_EXT_INST			0x00000200
/* Performance extension set II instructions.  */
#define E_NDS32_HAS_EXT2_INST			0x00000400
/* Single precision Floating point processor instructions.  */
#define E_NDS32_HAS_FPU_INST			0x00000800
/* Audio instructions with 32-bit audio dx.lo register.  */
#define E_NDS32_HAS_AUDIO_INST			0x00001000
/* DIV instructions.  */
#define E_NDS32_HAS_DIV_INST			0x00002000 /* Reclaimed.  */
/* DIV instructions using d0/d1.  */
#define E_NDS32_HAS_DIV_DX_INST			0x00002000 /* v2.  */
/* 16-bit instructions.  */
#define E_NDS32_HAS_16BIT_INST			0x00004000 /* Reclaimed.  */
/* String operation instructions.  */
#define E_NDS32_HAS_STRING_INST			0x00008000
/* Reduced register file.  */
#define E_NDS32_HAS_REDUCED_REGS		0x00010000
/* Video instructions.  */
#define E_NDS32_HAS_VIDEO_INST			0x00020000 /* Reclaimed.  */
#define E_NDS32_HAS_SATURATION_INST		0x00020000 /* v3, ELF 1.4.  */
/* Encription instructions.  */
#define E_NDS32_HAS_ENCRIPT_INST		0x00040000
/* Doulbe Precision Floating point processor instructions.  */
#define E_NDS32_HAS_FPU_DP_INST			0x00080000
/* No MAC instruction used.  */
#define E_NDS32_HAS_NO_MAC_INST			0x00100000 /* Reclaimed when V2/V3.  */
/* MAC instruction using d0/d1.  */
#define E_NDS32_HAS_MAC_DX_INST			0x00100000 /* v2.  */
/* L2 cache instruction.  */
#define E_NDS32_HAS_L2C_INST			0x00200000
/* FPU registers configuration when FPU SP/DP presents; 0x00c00000.  */
#define E_NDS32_FPU_REG_CONF_SHIFT		22
#define E_NDS32_FPU_REG_CONF			(0x3 << E_NDS32_FPU_REG_CONF_SHIFT)
#define E_NDS32_FPU_REG_8SP_4DP			0x0
#define E_NDS32_FPU_REG_16SP_8DP		0x1
#define E_NDS32_FPU_REG_32SP_16DP		0x2
#define E_NDS32_FPU_REG_32SP_32DP		0x3
/* FPU MAC instruction used.  */
#define E_NDS32_HAS_FPU_MAC_INST		0x01000000
/* DSP extension.  */
#define E_NDS32_HAS_DSP_INST			0x02000000
/* PIC enabled.  */
#define E_NDS32_HAS_PIC				0x04000000
/* Use custom section.  */
#define E_NDS32_HAS_CUSTOM_SEC			0x08000000
/* Hardware zero-overhead loop enabled.  */
#define E_NDS32_HAS_ZOL				(1 << 26)

/* 4-bit for ABI signature, allow up to 16 ABIs
   0: for OLD ABI V0, phase out
   1: for V1 , starting with V0 toolchain
   2: for V2
   3: for V2FP (fs0, fs1 as function parameter)
   4: for AABI  */
/* Only old N1213HC use V0.
   New ABI is used due to return register is changed to r0 from r5.  */
#define EF_NDS_ABI				0x000000F0
#define EF_NDS_ABI_SHIFT			4
#define E_NDS_ABI_V0				0x00000000
#define E_NDS_ABI_V1				0x00000010
#define E_NDS_ABI_V2				0x00000020
#define E_NDS_ABI_V2FP				0x00000030
#define E_NDS_ABI_AABI				0x00000040
#define E_NDS_ABI_V2FP_PLUS			0x00000050

/* This flag signifies the version of Andes ELF.
   Some more information may exist somewhere which is TBD.  */
#define EF_NDS32_ELF_VERSION			0x0000000F
#define EF_NDS32_ELF_VERSION_SHIFT		0

/* Andes ELF Version 1.3 and before.  */
#define E_NDS32_ELF_VER_1_2			0x0
/* Andes ELF Version 1.31.  */
#define E_NDS32_ELF_VER_1_3			0x1
/* Andes ELF Version 1.4. Change the way we fix .debug_* and .gcc_except_table.
   Change three bit for SAT.  */
#define E_NDS32_ELF_VER_1_4			0x2

#endif
