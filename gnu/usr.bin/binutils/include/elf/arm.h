/* ARM ELF support for BFD.
   Copyright (C) 1998-2023 Free Software Foundation, Inc.

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

#ifndef _ELF_ARM_H
#define _ELF_ARM_H

#include "elf/reloc-macros.h"

/* Processor specific flags for the ELF header e_flags field.  */

/* Old ABI (ie GNU pre EABI).  These are deprecated.  */
#define EF_ARM_RELEXEC     0x01
#define EF_ARM_INTERWORK   0x04
#define EF_ARM_APCS_26     0x08
#define EF_ARM_APCS_FLOAT  0x10
#define EF_ARM_PIC         0x20
#define EF_ARM_ALIGN8	   0x40		/* 8-bit structure alignment is in use.  */
#define EF_ARM_NEW_ABI     0x80
#define EF_ARM_OLD_ABI     0x100
#define EF_ARM_SOFT_FLOAT  0x200
#define EF_ARM_VFP_FLOAT   0x400
#define EF_ARM_MAVERICK_FLOAT 0x800

/* Frame unwind information */
#define PT_ARM_EXIDX (PT_LOPROC + 1)

/* Old ARM ELF spec. version B-01.  Mostly deprecated.  */
#define EF_ARM_SYMSARESORTED 0x04	/* NB conflicts with EF_INTERWORK.  */
#define EF_ARM_DYNSYMSUSESEGIDX 0x08	/* NB conflicts with EF_APCS26.  */
#define EF_ARM_MAPSYMSFIRST 0x10	/* NB conflicts with EF_APCS_FLOAT.  */

/* New constants defined in the ARM ELF spec. version XXX (AAELF).
   Only valid in conjunction with EF_ARM_EABI_VER5. */
#define EF_ARM_ABI_FLOAT_SOFT 0x200	/* NB conflicts with EF_ARM_SOFT_FLOAT.  */
#define EF_ARM_ABI_FLOAT_HARD 0x400	/* NB conflicts with EF_ARM_VFP_FLOAT.  */

/* Constants defined in AAELF.  */
#define EF_ARM_BE8	    0x00800000
#define EF_ARM_LE8	    0x00400000

#define EF_ARM_EABIMASK      0xFF000000
#define EF_ARM_EABI_VERSION(flags) ((flags) & EF_ARM_EABIMASK)
#define EF_ARM_EABI_UNKNOWN  0x00000000
#define EF_ARM_EABI_VER1     0x01000000
#define EF_ARM_EABI_VER2     0x02000000
#define EF_ARM_EABI_VER3     0x03000000
#define EF_ARM_EABI_VER4     0x04000000
#define EF_ARM_EABI_VER5     0x05000000

/* Local aliases for some flags to match names used by COFF port.  */
#define F_INTERWORK	   EF_ARM_INTERWORK
#define F_APCS26	   EF_ARM_APCS_26
#define F_APCS_FLOAT	   EF_ARM_APCS_FLOAT
#define F_PIC              EF_ARM_PIC
#define F_SOFT_FLOAT	   EF_ARM_SOFT_FLOAT
#define F_VFP_FLOAT	   EF_ARM_VFP_FLOAT

/* Additional symbol types for Thumb.  */
#define STT_ARM_TFUNC      STT_LOPROC   /* A Thumb function.  */
#define STT_ARM_16BIT      STT_HIPROC   /* A Thumb label.  */

/* Additional section types.  */
#define SHT_ARM_EXIDX	       0x70000001	/* Section holds ARM unwind info.  */
#define SHT_ARM_PREEMPTMAP     0x70000002	/* Section pre-emption details.  */
#define SHT_ARM_ATTRIBUTES     0x70000003	/* Section holds attributes.  */
#define SHT_ARM_DEBUGOVERLAY   0x70000004	/* Section holds overlay debug info.  */
#define SHT_ARM_OVERLAYSECTION 0x70000005	/* Section holds GDB and overlay integration info.  */

/* ARM-specific values for sh_flags.  */
#define SHF_ENTRYSECT      0x10000000   /* Section contains an entry point.  */
#define SHF_ARM_PURECODE   0x20000000   /* Section contains only code and no data.  */
#define SHF_COMDEF         0x80000000   /* Section may be multiply defined in the input to a link step.  */

/* ARM-specific program header flags.  */
#define PF_ARM_SB          0x10000000   /* Segment contains the location addressed by the static base.  */
#define PF_ARM_PI          0x20000000   /* Segment is position-independent.  */
#define PF_ARM_ABS         0x40000000   /* Segment must be loaded at its base address.  */

/* Values for the Tag_CPU_arch EABI attribute.  */
#define TAG_CPU_ARCH_PRE_V4	0
#define TAG_CPU_ARCH_V4		1
#define TAG_CPU_ARCH_V4T	2
#define TAG_CPU_ARCH_V5T	3
#define TAG_CPU_ARCH_V5TE	4
#define TAG_CPU_ARCH_V5TEJ	5
#define TAG_CPU_ARCH_V6		6
#define TAG_CPU_ARCH_V6KZ	7
#define TAG_CPU_ARCH_V6T2	8
#define TAG_CPU_ARCH_V6K	9
#define TAG_CPU_ARCH_V7		10
#define TAG_CPU_ARCH_V6_M	11
#define TAG_CPU_ARCH_V6S_M	12
#define TAG_CPU_ARCH_V7E_M	13
#define TAG_CPU_ARCH_V8		14
#define TAG_CPU_ARCH_V8R	15
#define TAG_CPU_ARCH_V8M_BASE	16
#define TAG_CPU_ARCH_V8M_MAIN	17
#define TAG_CPU_ARCH_8_1A       18
#define TAG_CPU_ARCH_8_2A       19
#define TAG_CPU_ARCH_8_3A       20
#define TAG_CPU_ARCH_V8_1M_MAIN 21
#define TAG_CPU_ARCH_V9         22
#define MAX_TAG_CPU_ARCH	TAG_CPU_ARCH_V9
/* Pseudo-architecture to allow objects to be compatible with the subset of
   armv4t and armv6-m.  This value should never be stored in object files.  */
#define TAG_CPU_ARCH_V4T_PLUS_V6_M (MAX_TAG_CPU_ARCH + 1)

/* Relocation types.  */

START_RELOC_NUMBERS (elf_arm_reloc_type)
/* AAELF official names and numbers.  */
  RELOC_NUMBER (R_ARM_NONE,            	  0)
  RELOC_NUMBER (R_ARM_PC24,            	  1)   /* deprecated */
  RELOC_NUMBER (R_ARM_ABS32,           	  2)
  RELOC_NUMBER (R_ARM_REL32,           	  3)
  RELOC_NUMBER (R_ARM_LDR_PC_G0,       	  4)
  RELOC_NUMBER (R_ARM_ABS16,           	  5)
  RELOC_NUMBER (R_ARM_ABS12,           	  6)
  RELOC_NUMBER (R_ARM_THM_ABS5,        	  7)
  RELOC_NUMBER (R_ARM_ABS8,            	  8)
  RELOC_NUMBER (R_ARM_SBREL32,         	  9)
  RELOC_NUMBER (R_ARM_THM_CALL,        	 10)
  RELOC_NUMBER (R_ARM_THM_PC8,         	 11)
  RELOC_NUMBER (R_ARM_BREL_ADJ,	       	 12)
  RELOC_NUMBER (R_ARM_TLS_DESC,          13)
  RELOC_NUMBER (R_ARM_THM_SWI8,        	 14)   /* obsolete */
  RELOC_NUMBER (R_ARM_XPC25,           	 15)   /* obsolete */
  RELOC_NUMBER (R_ARM_THM_XPC22,       	 16)   /* obsolete */
  RELOC_NUMBER (R_ARM_TLS_DTPMOD32,    	 17)
  RELOC_NUMBER (R_ARM_TLS_DTPOFF32,    	 18)
  RELOC_NUMBER (R_ARM_TLS_TPOFF32,     	 19)
  RELOC_NUMBER (R_ARM_COPY,            	 20)   /* Copy symbol at runtime.  */
  RELOC_NUMBER (R_ARM_GLOB_DAT,        	 21)   /* Create GOT entry.  */
  RELOC_NUMBER (R_ARM_JUMP_SLOT,       	 22)   /* Create PLT entry.  */
  RELOC_NUMBER (R_ARM_RELATIVE,        	 23)   /* Adjust by program base.  */
  RELOC_NUMBER (R_ARM_GOTOFF32,        	 24)   /* 32 bit offset to GOT.  */
  RELOC_NUMBER (R_ARM_BASE_PREL,       	 25)   /* 32 bit PC relative offset to GOT.  */
  RELOC_NUMBER (R_ARM_GOT_BREL,        	 26)   /* 32 bit GOT entry.  */
  RELOC_NUMBER (R_ARM_PLT32,           	 27)   /* deprecated - 32 bit PLT address.  */
  RELOC_NUMBER (R_ARM_CALL,            	 28)
  RELOC_NUMBER (R_ARM_JUMP24,          	 29)
  RELOC_NUMBER (R_ARM_THM_JUMP24,      	 30)
  RELOC_NUMBER (R_ARM_BASE_ABS,        	 31)
  RELOC_NUMBER (R_ARM_ALU_PCREL7_0,    	 32)   /* obsolete */
  RELOC_NUMBER (R_ARM_ALU_PCREL15_8,   	 33)   /* obsolete */
  RELOC_NUMBER (R_ARM_ALU_PCREL23_15,  	 34)   /* obsolete */
  RELOC_NUMBER (R_ARM_LDR_SBREL_11_0,  	 35)   /* deprecated, should have _NC suffix */
  RELOC_NUMBER (R_ARM_ALU_SBREL_19_12, 	 36)   /* deprecated, should have _NC suffix */
  RELOC_NUMBER (R_ARM_ALU_SBREL_27_20, 	 37)   /* deprecated, should have _CK suffix */
  RELOC_NUMBER (R_ARM_TARGET1,         	 38)
  RELOC_NUMBER (R_ARM_SBREL31,         	 39)   /* deprecated */
  RELOC_NUMBER (R_ARM_V4BX,            	 40)
  RELOC_NUMBER (R_ARM_TARGET2,	       	 41)
  RELOC_NUMBER (R_ARM_PREL31,	       	 42)
  RELOC_NUMBER (R_ARM_MOVW_ABS_NC,     	 43)
  RELOC_NUMBER (R_ARM_MOVT_ABS,        	 44)
  RELOC_NUMBER (R_ARM_MOVW_PREL_NC,    	 45)
  RELOC_NUMBER (R_ARM_MOVT_PREL,       	 46)
  RELOC_NUMBER (R_ARM_THM_MOVW_ABS_NC, 	 47)
  RELOC_NUMBER (R_ARM_THM_MOVT_ABS,    	 48)
  RELOC_NUMBER (R_ARM_THM_MOVW_PREL_NC,	 49)
  RELOC_NUMBER (R_ARM_THM_MOVT_PREL,   	 50)
  RELOC_NUMBER (R_ARM_THM_JUMP19,      	 51)
  RELOC_NUMBER (R_ARM_THM_JUMP6,       	 52)
  RELOC_NUMBER (R_ARM_THM_ALU_PREL_11_0, 53)
  RELOC_NUMBER (R_ARM_THM_PC12,        	 54)
  RELOC_NUMBER (R_ARM_ABS32_NOI,       	 55)
  RELOC_NUMBER (R_ARM_REL32_NOI,       	 56)
  RELOC_NUMBER (R_ARM_ALU_PC_G0_NC,      57)
  RELOC_NUMBER (R_ARM_ALU_PC_G0,         58)
  RELOC_NUMBER (R_ARM_ALU_PC_G1_NC,      59)
  RELOC_NUMBER (R_ARM_ALU_PC_G1,         60)
  RELOC_NUMBER (R_ARM_ALU_PC_G2,         61)
  RELOC_NUMBER (R_ARM_LDR_PC_G1,         62)
  RELOC_NUMBER (R_ARM_LDR_PC_G2,         63)
  RELOC_NUMBER (R_ARM_LDRS_PC_G0,        64)
  RELOC_NUMBER (R_ARM_LDRS_PC_G1,        65)
  RELOC_NUMBER (R_ARM_LDRS_PC_G2,        66)
  RELOC_NUMBER (R_ARM_LDC_PC_G0,         67)
  RELOC_NUMBER (R_ARM_LDC_PC_G1,         68)
  RELOC_NUMBER (R_ARM_LDC_PC_G2,         69)
  RELOC_NUMBER (R_ARM_ALU_SB_G0_NC,      70)
  RELOC_NUMBER (R_ARM_ALU_SB_G0,         71)
  RELOC_NUMBER (R_ARM_ALU_SB_G1_NC,      72)
  RELOC_NUMBER (R_ARM_ALU_SB_G1,         73)
  RELOC_NUMBER (R_ARM_ALU_SB_G2,         74)
  RELOC_NUMBER (R_ARM_LDR_SB_G0,         75)
  RELOC_NUMBER (R_ARM_LDR_SB_G1,         76)
  RELOC_NUMBER (R_ARM_LDR_SB_G2,         77)
  RELOC_NUMBER (R_ARM_LDRS_SB_G0,        78)
  RELOC_NUMBER (R_ARM_LDRS_SB_G1,        79)
  RELOC_NUMBER (R_ARM_LDRS_SB_G2,        80)
  RELOC_NUMBER (R_ARM_LDC_SB_G0,         81)
  RELOC_NUMBER (R_ARM_LDC_SB_G1,         82)
  RELOC_NUMBER (R_ARM_LDC_SB_G2,         83)
  RELOC_NUMBER (R_ARM_MOVW_BREL_NC,    	 84)
  RELOC_NUMBER (R_ARM_MOVT_BREL,       	 85)
  RELOC_NUMBER (R_ARM_MOVW_BREL,       	 86)
  RELOC_NUMBER (R_ARM_THM_MOVW_BREL_NC,	 87)
  RELOC_NUMBER (R_ARM_THM_MOVT_BREL,   	 88)
  RELOC_NUMBER (R_ARM_THM_MOVW_BREL,   	 89)
  RELOC_NUMBER (R_ARM_TLS_GOTDESC,       90)
  RELOC_NUMBER (R_ARM_TLS_CALL,          91)
  RELOC_NUMBER (R_ARM_TLS_DESCSEQ,       92)
  RELOC_NUMBER (R_ARM_THM_TLS_CALL,      93)
  RELOC_NUMBER (R_ARM_PLT32_ABS,       	 94)
  RELOC_NUMBER (R_ARM_GOT_ABS,	       	 95)
  RELOC_NUMBER (R_ARM_GOT_PREL,	       	 96)
  RELOC_NUMBER (R_ARM_GOT_BREL12,      	 97)
  RELOC_NUMBER (R_ARM_GOTOFF12,	       	 98)
  RELOC_NUMBER (R_ARM_GOTRELAX,	       	 99)
  RELOC_NUMBER (R_ARM_GNU_VTENTRY,    	100)   /* deprecated - old C++ abi */
  RELOC_NUMBER (R_ARM_GNU_VTINHERIT,  	101)   /* deprecated - old C++ abi */
  RELOC_NUMBER (R_ARM_THM_JUMP11,     	102)
  RELOC_NUMBER (R_ARM_THM_JUMP8,      	103)
  RELOC_NUMBER (R_ARM_TLS_GD32,	      	104)
  RELOC_NUMBER (R_ARM_TLS_LDM32,      	105)
  RELOC_NUMBER (R_ARM_TLS_LDO32,      	106)
  RELOC_NUMBER (R_ARM_TLS_IE32,       	107)
  RELOC_NUMBER (R_ARM_TLS_LE32,	      	108)
  RELOC_NUMBER (R_ARM_TLS_LDO12,      	109)
  RELOC_NUMBER (R_ARM_TLS_LE12,	      	110)
  RELOC_NUMBER (R_ARM_TLS_IE12GP,     	111)
  /* 112 - 127 private range */
  RELOC_NUMBER (R_ARM_ME_TOO,	        128)   /* obsolete */
  RELOC_NUMBER (R_ARM_THM_TLS_DESCSEQ  ,129)

  RELOC_NUMBER (R_ARM_THM_ALU_ABS_G0_NC,132)
  RELOC_NUMBER (R_ARM_THM_ALU_ABS_G1_NC,133)
  RELOC_NUMBER (R_ARM_THM_ALU_ABS_G2_NC,134)
  RELOC_NUMBER (R_ARM_THM_ALU_ABS_G3_NC,135)
  RELOC_NUMBER (R_ARM_THM_BF16,		136)
  RELOC_NUMBER (R_ARM_THM_BF12,		137)
  RELOC_NUMBER (R_ARM_THM_BF18,		138)

  RELOC_NUMBER (R_ARM_IRELATIVE,      	160)
  RELOC_NUMBER (R_ARM_GOTFUNCDESC,    	161)
  RELOC_NUMBER (R_ARM_GOTOFFFUNCDESC, 	162)
  RELOC_NUMBER (R_ARM_FUNCDESC,       	163)
  RELOC_NUMBER (R_ARM_FUNCDESC_VALUE, 	164)
  RELOC_NUMBER (R_ARM_TLS_GD32_FDPIC,   165)
  RELOC_NUMBER (R_ARM_TLS_LDM32_FDPIC,  166)
  RELOC_NUMBER (R_ARM_TLS_IE32_FDPIC,   167)

  /* Extensions?  R=read-only?  */
  RELOC_NUMBER (R_ARM_RXPC25,         	249)
  RELOC_NUMBER (R_ARM_RSBREL32,       	250)
  RELOC_NUMBER (R_ARM_THM_RPC22,      	251)
  RELOC_NUMBER (R_ARM_RREL32,         	252)
  RELOC_NUMBER (R_ARM_RABS32,         	253)
  RELOC_NUMBER (R_ARM_RPC24,          	254)
  RELOC_NUMBER (R_ARM_RBASE,          	255)

  /* Unofficial names for some of the relocs.  */
  FAKE_RELOC (R_ARM_GOTOFF,             R_ARM_GOTOFF32)   /* 32 bit offset to GOT.  */
  FAKE_RELOC (R_ARM_THM_PC22,           R_ARM_THM_CALL)
  FAKE_RELOC (R_ARM_THM_PC11,           R_ARM_THM_JUMP11)
  FAKE_RELOC (R_ARM_THM_PC9,            R_ARM_THM_JUMP8)

  /* Relocs with both a different name, and (apparently) different meaning in
     GNU usage.  */
  FAKE_RELOC (R_ARM_GOTPC,              R_ARM_BASE_PREL)  /* 32 bit PC relative offset to GOT.  */
  FAKE_RELOC (R_ARM_GOT32,              R_ARM_GOT_BREL)   /* 32 bit GOT entry.  */
  FAKE_RELOC (R_ARM_ROSEGREL32,         R_ARM_SBREL31)    /* ??? */
  FAKE_RELOC (R_ARM_AMP_VCALL9,         R_ARM_BREL_ADJ)   /* Thumb-something.  Not used.  */

END_RELOC_NUMBERS (R_ARM_max = 256)

#ifdef BFD_ARCH_SIZE
/* EABI object attributes.  */

enum
{
  /* 0-3 are generic.  */
  Tag_CPU_raw_name = 4,
  Tag_CPU_name,
  Tag_CPU_arch,
  Tag_CPU_arch_profile,
  Tag_ARM_ISA_use,
  Tag_THUMB_ISA_use,
  Tag_FP_arch,
  Tag_WMMX_arch,
  Tag_Advanced_SIMD_arch,
  Tag_PCS_config,
  Tag_ABI_PCS_R9_use,
  Tag_ABI_PCS_RW_data,
  Tag_ABI_PCS_RO_data,
  Tag_ABI_PCS_GOT_use,
  Tag_ABI_PCS_wchar_t,
  Tag_ABI_FP_rounding,
  Tag_ABI_FP_denormal,
  Tag_ABI_FP_exceptions,
  Tag_ABI_FP_user_exceptions,
  Tag_ABI_FP_number_model,
  Tag_ABI_align_needed,
  Tag_ABI_align_preserved,
  Tag_ABI_enum_size,
  Tag_ABI_HardFP_use,
  Tag_ABI_VFP_args,
  Tag_ABI_WMMX_args,
  Tag_ABI_optimization_goals,
  Tag_ABI_FP_optimization_goals,
  /* 32 is generic (Tag_compatibility).  */
  Tag_undefined33 = 33,
  Tag_CPU_unaligned_access,
  Tag_undefined35,
  Tag_FP_HP_extension,
  Tag_undefined37,
  Tag_ABI_FP_16bit_format,
  Tag_undefined39,
  Tag_undefined40,
  Tag_undefined41,
  Tag_MPextension_use,
  Tag_undefined_43,
  Tag_DIV_use,
  Tag_DSP_extension = 46,
  Tag_MVE_arch = 48,
  Tag_PAC_extension = 50,
  Tag_BTI_extension = 52,
  Tag_BTI_use = 74,
  Tag_PACRET_use = 76,
  Tag_nodefaults = 64,
  Tag_also_compatible_with,
  Tag_T2EE_use,
  Tag_conformance,
  Tag_Virtualization_use,
  Tag_undefined69,
  Tag_MPextension_use_legacy,

  /* The following tags are legacy names for other tags.  */
  Tag_VFP_arch = Tag_FP_arch,
  Tag_ABI_align8_needed = Tag_ABI_align_needed,
  Tag_ABI_align8_preserved = Tag_ABI_align_preserved,
  Tag_VFP_HP_extension = Tag_FP_HP_extension
};

/* Values for Tag_ABI_FP_number_model.  */
enum
{
  AEABI_FP_number_model_none = 0,
  AEABI_FP_number_model_ieee754_number = 1,
  AEABI_FP_number_model_rtabi = 2,
  AEABI_FP_number_model_ieee754_all = 3
};

/* Values for Tag_ABI_VFP_args.  */
enum
{
  AEABI_VFP_args_base = 0,
  AEABI_VFP_args_vfp = 1,
  AEABI_VFP_args_toolchain = 2,
  AEABI_VFP_args_compatible = 3
};
#endif

/* The name of the note section used to identify arm variants.  */
#define ARM_NOTE_SECTION ".note.gnu.arm.ident"

/* Special section names.  */
#define ELF_STRING_ARM_unwind           ".ARM.exidx"
#define ELF_STRING_ARM_unwind_info      ".ARM.extab"
#define ELF_STRING_ARM_unwind_once      ".gnu.linkonce.armexidx."
#define ELF_STRING_ARM_unwind_info_once ".gnu.linkonce.armextab."

enum arm_st_branch_type {
  ST_BRANCH_TO_ARM,
  ST_BRANCH_TO_THUMB,
  ST_BRANCH_LONG,
  ST_BRANCH_UNKNOWN,
  ST_BRANCH_ENUM_SIZE
};

#define NUM_ENUM_ARM_ST_BRANCH_TYPE_BITS 2
#define ENUM_ARM_ST_BRANCH_TYPE_BITMASK	\
  ((1 << NUM_ENUM_ARM_ST_BRANCH_TYPE_BITS) - 1)

#define ARM_GET_SYM_BRANCH_TYPE(STI) \
  ((enum arm_st_branch_type) ((STI) & ENUM_ARM_ST_BRANCH_TYPE_BITMASK))
#ifdef BFD_ASSERT
#define ARM_SET_SYM_BRANCH_TYPE(STI, TYPE)			\
  do {								\
    BFD_ASSERT (TYPE <= ST_BRANCH_ENUM_SIZE);			\
    BFD_ASSERT ((1 << NUM_ENUM_ARM_ST_BRANCH_TYPE_BITS)		\
		>= ST_BRANCH_ENUM_SIZE);			\
    (STI) = (((STI) & ~ENUM_ARM_ST_BRANCH_TYPE_BITMASK)		\
	     | ((TYPE) & ENUM_ARM_ST_BRANCH_TYPE_BITMASK));	\
  } while (0)
#else
#define ARM_SET_SYM_BRANCH_TYPE(STI, TYPE)			\
  (STI) = (((STI) & ~ENUM_ARM_ST_BRANCH_TYPE_BITMASK)		\
	   | ((TYPE) & ENUM_ARM_ST_BRANCH_TYPE_BITMASK))
#endif

#endif /* _ELF_ARM_H */
