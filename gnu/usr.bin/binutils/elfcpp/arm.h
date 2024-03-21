// arm.h -- ELF definitions specific to EM_ARM  -*- C++ -*-

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Doug Kwan <dougkwan@google.com>.

// This file is part of elfcpp.
   
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// In addition to the permissions in the GNU Library General Public
// License, the Free Software Foundation gives you unlimited
// permission to link the compiled version of this file into
// combinations with other programs, and to distribute those
// combinations without any restriction coming from the use of this
// file.  (The Library Public License restrictions do apply in other
// respects; for example, they cover modification of the file, and
// distribution when not linked into a combined executable.)

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef ELFCPP_ARM_H
#define ELFCPP_ARM_H

// The relocation type information is taken from:
//
//   ELF for the ARM Architecture
//   Document number: ARM IHI 0044C, current through ABI release 2.07
//   Date of Issue: 10th October, 2008
//

namespace elfcpp
{

//
// ARM Relocations Codes
//

// Operation notes: 
//   S: Address of the symbol.
//   A: Addend for relocation.
//   P: Address of the place being relocated.
//   Pa: Adjusted address of the place being relocated (P & 0xfffffffc)
//   T: is 1 if S has type STT_FUNC and the symbol addresses a Thumb
//      instruction.Thumb-bit; it is 0 otherwise.
//   B(S): Addressing origin of the output segment defining S.
//   GOT_ORG: Addressing origin of the Global Offset Table.
//   GOT(S): Address of the GOT entry for S.
//

enum
{
				// Type		Class	Operation		
				// ------------------------------
  R_ARM_NONE = 0,		// Static	Misc
  R_ARM_PC24 = 1,		// Deprecated	ARM	((S + A) | T) - P
  R_ARM_ABS32 = 2,		// Static	Data	(S + A) | T
  R_ARM_REL32 = 3,		// Static	Data	((S + A) | T) - P
  R_ARM_LDR_PC_G0 = 4,		// Static	ARM	S + A - P
  R_ARM_ABS16 = 5,		// Static	Data	S + A
  R_ARM_ABS12 = 6,		// Static	ARM	S + A
  R_ARM_THM_ABS5 = 7,		// Static	Thumb16	S + A
  R_ARM_ABS8 = 8,		// Static	Data	S + A
  R_ARM_SBREL32 = 9,		// Static	Data	((S + A) | T) - B(S)
  R_ARM_THM_CALL = 10,		// Static	Thumb32	((S + A) | T) - P
  R_ARM_THM_PC8 = 11,		// Static	Thumb16
  R_ARM_BREL_ADJ = 12,		// Dynamic	Data	DeltaB(S) + A
  R_ARM_TLS_DESC = 13,		// Dynamic	Data
  R_ARM_THM_SWI8 = 14,		// Obsolete
  R_ARM_XPC25 = 15,		// Obsolete
  R_ARM_THM_XPC22 = 16,		// Obsolete
  R_ARM_TLS_DTPMOD32 = 17,	// Dynamic	Data	Module(S)
  R_ARM_TLS_DTPOFF32 = 18,	// Dynamic	Data	S + A - TLS
  R_ARM_TLS_TPOFF32 = 19,	// Dynamic	Data	S + A - tp
  R_ARM_COPY = 20,		// Dynamic	Misc
  R_ARM_GLOB_DAT = 21,		// Dynamic	Data	(S + A) | T
  R_ARM_JUMP_SLOT = 22,		// Dynamic	Data	(S + A) | T
  R_ARM_RELATIVE = 23,		// Dynamic	Data	B(S) + A
  R_ARM_GOTOFF32 = 24,		// Static	Data	(((S + A) | T) - GOT_ORG
  R_ARM_BASE_PREL = 25,		// Static	Data	B(S) + A - P
  R_ARM_GOT_BREL = 26,		// Static	Data	GOT(S) + A - GOT_ORG
  R_ARM_PLT32 = 27,		// Deprecated	ARM	((S + A) | T) - P
  R_ARM_CALL = 28,		// Static	ARM	((S + A) | T) - P
  R_ARM_JUMP24 = 29,		// Static	ARM	((S + A) | T) - P
  R_ARM_THM_JUMP24 = 30,	// Static	Thumb32	((S + A) | T) - P
  R_ARM_BASE_ABS = 31,		// Static	Data	B(S) + A
  R_ARM_ALU_PCREL_7_0 = 32,	// Obsolete
  R_ARM_ALU_PCREL_15_8 = 33,	// Obsolete
  R_ARM_ALU_PCREL_23_15 = 34,	// Obsolete
  R_ARM_LDR_SBREL_11_0_NC = 35,	// Deprecated	ARM	S + A - B(S)
  R_ARM_ALU_SBREL_19_12_NC = 36,// Deprecated	ARM	S + A - B(S)
  R_ARM_ALU_SBREL_27_20_CK = 37,// Deprecated	ARM	S + A - B(S)
  R_ARM_TARGET1 = 38,		// Data		Misc	(S + A) | T or
				//			((S + A) | T) - P
  R_ARM_SBREL31 = 39,		// Deprecated	Data	((S + A) | T) - B(S)
  R_ARM_V4BX = 40,		// Static	Misc 
  R_ARM_TARGET2 = 41,		// Static	Misc
  R_ARM_PREL31 = 42,		// Static	Data	((S + A) | T) - P
  R_ARM_MOVW_ABS_NC = 43,	// Static	ARM	(S + A) | T
  R_ARM_MOVT_ABS = 44,		// Static	ARM	S + A
  R_ARM_MOVW_PREL_NC = 45,	// Static	ARM	((S + A) | T) - P
  R_ARM_MOVT_PREL = 46,		// Static	ARM	S + A - P
  R_ARM_THM_MOVW_ABS_NC = 47,	// Static	Thumb32	(S + A) | T
  R_ARM_THM_MOVT_ABS = 48,	// Static	Thumb32	S + A - P
  R_ARM_THM_MOVW_PREL_NC =  49,	// Static	Thumb32	((S + A) | T) - P
  R_ARM_THM_MOVT_PREL = 50,	// Static	Thumb32	S + A - P
  R_ARM_THM_JUMP19 = 51,	// Static	Thumb32	((S + A) | T) - P
  R_ARM_THM_JUMP6 = 52,		// Static	Thumb16	S + A - P
  R_ARM_THM_ALU_PREL_11_0 = 53,	// Static	Thumb32	((S + A) | T) - Pa
  R_ARM_THM_PC12 = 54,		// Static	Thumb32	S + A - Pa
  R_ARM_ABS32_NOI = 55,		// Static	Data	S + A
  R_ARM_REL32_NOI = 56,		// Static	Data	S + A - P
  R_ARM_ALU_PC_G0_NC = 57,	// Static	ARM	((S + A) | T) - P
  R_ARM_ALU_PC_G0 = 58,		// Static	ARM	((S + A) | T) - P
  R_ARM_ALU_PC_G1_NC = 59,	// Static	ARM	((S + A) | T) - P
  R_ARM_ALU_PC_G1 = 60,		// Static	ARM	((S + A) | T) - P
  R_ARM_ALU_PC_G2 = 61,		// Static	ARM	((S + A) | T) - P
  R_ARM_LDR_PC_G1 = 62,		// Static	ARM	S + A - P
  R_ARM_LDR_PC_G2 = 63,		// Static	ARM	S + A - P
  R_ARM_LDRS_PC_G0 = 64,	// Static	ARM	S + A - P
  R_ARM_LDRS_PC_G1 = 65,	// Static	ARM	S + A - P
  R_ARM_LDRS_PC_G2 = 66,	// Static	ARM	S + A - P
  R_ARM_LDC_PC_G0 = 67,		// Static	ARM	S + A - P
  R_ARM_LDC_PC_G1 = 68,		// Static	ARM	S + A - P
  R_ARM_LDC_PC_G2 = 69,		// Static	ARM	S + A - P
  R_ARM_ALU_SB_G0_NC = 70,	// Static	ARM	((S + A) | T) - B(S)
  R_ARM_ALU_SB_G0 = 71,		// Static	ARM	((S + A) | T) - B(S)
  R_ARM_ALU_SB_G1_NC = 72,	// Static	ARM	((S + A) | T) - B(S)
  R_ARM_ALU_SB_G1 = 73,		// Static	ARM	((S + A) | T) - B(S)
  R_ARM_ALU_SB_G2 = 74,		// Static	ARM	((S + A) | T) - B(S)
  R_ARM_LDR_SB_G0 = 75,		// Static	ARM	S + A - B(S)
  R_ARM_LDR_SB_G1 = 76,		// Static	ARM	S + A - B(S)
  R_ARM_LDR_SB_G2 = 77,		// Static	ARM	S + A - B(S)
  R_ARM_LDRS_SB_G0 = 78,	// Static	ARM	S + A - B(S)
  R_ARM_LDRS_SB_G1 = 79,	// Static	ARM	S + A - B(S)
  R_ARM_LDRS_SB_G2 = 80,	// Static	ARM	S + A - B(S)
  R_ARM_LDC_SB_G0 = 81,		// Static	ARM	S + A - B(S)
  R_ARM_LDC_SB_G1 = 82,		// Static	ARM	S + A - B(S)
  R_ARM_LDC_SB_G2 = 83,		// Static	ARM	S + A - B(S)
  R_ARM_MOVW_BREL_NC = 84,	// Static	ARM	((S + A) | T) - B(S)
  R_ARM_MOVT_BREL = 85,		// Static	ARM	S + A - B(S)
  R_ARM_MOVW_BREL = 86,		// Static	ARM	((S + A) | T) - B(S)
  R_ARM_THM_MOVW_BREL_NC = 87,	// Static	Thumb32	((S + A) | T) - B(S)
  R_ARM_THM_MOVT_BREL = 88,	// Static	Thumb32	S + A - B(S)
  R_ARM_THM_MOVW_BREL = 89,	// Static	Thumb32	((S + A) | T) - B(S)
  R_ARM_TLS_GOTDESC = 90,	// Static	Data
  R_ARM_TLS_CALL = 91,		// Static	ARM
  R_ARM_TLS_DESCSEQ = 92,	// Static	ARM	TLS relaxation
  R_ARM_THM_TLS_CALL = 93,	// Static	Thumb32
  R_ARM_PLT32_ABS =  94,	// Static	Data	PLT(S) + A
  R_ARM_GOT_ABS =  95,		// Static	Data	GOT(S) + A
  R_ARM_GOT_PREL = 96,		// Static	Data	GOT(S) + A - P
  R_ARM_GOT_BREL12 =  97,	// Static	ARM	GOT(S) + A - GOT_ORG
  R_ARM_GOTOFF12 =  98,		// Static	ARM	S + A - GOT_ROG
  R_ARM_GOTRELAX =  99,		// Static	Misc
  R_ARM_GNU_VTENTRY = 100,	// Deprecated	Data
  R_ARM_GNU_VTINHERIT = 101,	// Deprecated	Data
  R_ARM_THM_JUMP11 = 102,	// Static	Thumb16	S + A - P
  R_ARM_THM_JUMP8 = 103,	// Static	Thumb16	S + A - P
  R_ARM_TLS_GD32 = 104,		// Static	Data	GOT(S) + A - P
  R_ARM_TLS_LDM32 = 105,	// Static	Data	GOT(S) + A - P
  R_ARM_TLS_LDO32 = 106,	// Static	Data	S + A - TLS
  R_ARM_TLS_IE32 = 107,		// Static	Data	GOT(S) + A - P
  R_ARM_TLS_LE32 = 108,		// Static	Data	S + A - tp
  R_ARM_TLS_LDO12 = 109,	// Static	ARM	S + A - TLS
  R_ARM_TLS_LE12 = 110,		// Static	ARM	S + A - tp
  R_ARM_TLS_IE12GP = 111,	// Static	ARM	GOT(S) + A - GOT_ORG
  R_ARM_PRIVATE_0 = 112,	// Private (n = 0, 1, ... 15)
  R_ARM_PRIVATE_1 = 113,
  R_ARM_PRIVATE_2 = 114,
  R_ARM_PRIVATE_3 = 115,
  R_ARM_PRIVATE_4 = 116,
  R_ARM_PRIVATE_5 = 117,
  R_ARM_PRIVATE_6 = 118,
  R_ARM_PRIVATE_7 = 119,
  R_ARM_PRIVATE_8 = 120,
  R_ARM_PRIVATE_9 = 121,
  R_ARM_PRIVATE_10 = 122,
  R_ARM_PRIVATE_11 = 123,
  R_ARM_PRIVATE_12 = 124,
  R_ARM_PRIVATE_13 = 125,
  R_ARM_PRIVATE_14 = 126,
  R_ARM_PRIVATE_15 = 127,
  R_ARM_ME_TOO = 128,		// Obsolete
  R_ARM_THM_TLS_DESCSEQ16 = 129,// Static	Thumb16
  R_ARM_THM_TLS_DESCSEQ32 = 130,// Static	Thumb32
  // 131 - 135			Unallocated
  // Relocations for Armv8.1-M Mainline (BF/BFL)
  R_ARM_THM_BF16 = 136,		// Static       Thumb32 ((S + A) | T) – P
  R_ARM_THM_BF12 = 137,		// Static       Thumb32 ((S + A) | T) – P
  R_ARM_THM_BF18 = 138,		// Static       Thumb32 ((S + A) | T) – P
  // 139			Unallocated
  // 140 - 159			Dynamic		Reserved for future allocation
  R_ARM_IRELATIVE = 160,	// Dynamic
  // 161 - 255			Unallocated
};

// e_flags values used for ARM.  We only support flags defined in AAELF.

enum
{
  EF_ARM_BE8 = 0x00800000,

  // Mask to extract EABI version, not really a flag value.
  EF_ARM_EABIMASK = 0xFF000000,

  EF_ARM_EABI_UNKNOWN = 0x00000000,
  EF_ARM_EABI_VER1 = 0x01000000,
  EF_ARM_EABI_VER2 = 0x02000000,
  EF_ARM_EABI_VER3 = 0x03000000,
  EF_ARM_EABI_VER4 = 0x04000000,
  EF_ARM_EABI_VER5 = 0x05000000,
};

// Extract EABI version from flags.

inline Elf_Word
arm_eabi_version(Elf_Word flags)
{ return flags & EF_ARM_EABIMASK; }

// EABI_VER5 e_flags values for identifying soft- and hard-float ABI
// choice.
enum
{
  EF_ARM_ABI_FLOAT_SOFT = 0x200,
  EF_ARM_ABI_FLOAT_HARD = 0x400,
};

// Values for the Tag_CPU_arch EABI attribute.
enum
{
  TAG_CPU_ARCH_PRE_V4,
  TAG_CPU_ARCH_V4,
  TAG_CPU_ARCH_V4T,
  TAG_CPU_ARCH_V5T,
  TAG_CPU_ARCH_V5TE,
  TAG_CPU_ARCH_V5TEJ,
  TAG_CPU_ARCH_V6,
  TAG_CPU_ARCH_V6KZ,
  TAG_CPU_ARCH_V6T2,
  TAG_CPU_ARCH_V6K,
  TAG_CPU_ARCH_V7,
  TAG_CPU_ARCH_V6_M,
  TAG_CPU_ARCH_V6S_M,
  TAG_CPU_ARCH_V7E_M,
  TAG_CPU_ARCH_V8,
  TAG_CPU_ARCH_V8R,
  TAG_CPU_ARCH_V8M_BASE,
  TAG_CPU_ARCH_V8M_MAIN,
  TAG_CPU_ARCH_8_1A,
  TAG_CPU_ARCH_8_2A,
  TAG_CPU_ARCH_8_3A,
  TAG_CPU_ARCH_V8_1M_MAIN,
  TAG_CPU_ARCH_V9,
  MAX_TAG_CPU_ARCH = TAG_CPU_ARCH_V9,
  // Pseudo-architecture to allow objects to be compatible with the subset of
  // armv4t and armv6-m.  This value should never be stored in object files.
  TAG_CPU_ARCH_V4T_PLUS_V6_M = (MAX_TAG_CPU_ARCH + 1)
};

// EABI object attributes.
enum
{
  // 0-3 are generic.
  Tag_CPU_raw_name = 4,
  Tag_CPU_name = 5,
  Tag_CPU_arch = 6,
  Tag_CPU_arch_profile = 7,
  Tag_ARM_ISA_use = 8,
  Tag_THUMB_ISA_use = 9,
  Tag_FP_arch = 10,
  Tag_WMMX_arch = 11,
  Tag_Advanced_SIMD_arch = 12,
  Tag_PCS_config = 13,
  Tag_ABI_PCS_R9_use = 14,
  Tag_ABI_PCS_RW_data = 15,
  Tag_ABI_PCS_RO_data = 16,
  Tag_ABI_PCS_GOT_use = 17,
  Tag_ABI_PCS_wchar_t = 18,
  Tag_ABI_FP_rounding = 19,
  Tag_ABI_FP_denormal = 20,
  Tag_ABI_FP_exceptions = 21,
  Tag_ABI_FP_user_exceptions = 22,
  Tag_ABI_FP_number_model = 23,
  Tag_ABI_align_needed = 24,
  Tag_ABI_align_preserved = 25,
  Tag_ABI_enum_size = 26,
  Tag_ABI_HardFP_use = 27,
  Tag_ABI_VFP_args = 28,
  Tag_ABI_WMMX_args = 29,
  Tag_ABI_optimization_goals = 30,
  Tag_ABI_FP_optimization_goals = 31,
  // 32 is generic (Tag_compatibility).
  Tag_undefined33 = 33,
  Tag_CPU_unaligned_access = 34,
  Tag_undefined35 = 35,
  Tag_FP_HP_extension = 36,
  Tag_undefined37 = 37,
  Tag_ABI_FP_16bit_format = 38,
  Tag_undefined39 = 39,
  Tag_undefined40 = 40,
  Tag_undefined41 = 41,
  Tag_MPextension_use = 42,
  Tag_undefined43 = 43,
  Tag_DIV_use = 44,
  Tag_MVE_arch = 48,
  Tag_PAC_extension = 50,
  Tag_BTI_extension = 52,
  Tag_BTI_use = 74,
  Tag_PACRET_use = 76,
  Tag_nodefaults = 64,
  Tag_also_compatible_with = 65,
  Tag_T2EE_use = 66,
  Tag_conformance = 67,
  Tag_Virtualization_use = 68,
  Tag_undefined69 = 69,
  Tag_MPextension_use_legacy = 70,

  // The following tags are legacy names for other tags.
  Tag_VFP_arch = Tag_FP_arch,
  Tag_ABI_align8_needed = Tag_ABI_align_needed,
  Tag_ABI_align8_preserved = Tag_ABI_align_preserved,
  Tag_VFP_HP_extension = Tag_FP_HP_extension
};

// Values for Tag_ABI_PCS_R9_use.
enum
{
  AEABI_R9_V6 = 0,
  AEABI_R9_SB = 1,
  AEABI_R9_TLS = 2,
  AEABI_R9_unused = 3
};

// Values for Tag_ABI_PCS_RW_data.
enum
{
  AEABI_PCS_RW_data_absolute = 0,
  AEABI_PCS_RW_data_PCrel = 1,
  AEABI_PCS_RW_data_SBrel = 2,
  AEABI_PCS_RW_data_unused = 3
};

// Values for Tag_ABI_enum_size.
enum
{
  AEABI_enum_unused = 0,
  AEABI_enum_short = 1,
  AEABI_enum_wide = 2,
  AEABI_enum_forced_wide = 3
};

// Values for Tag_ABI_FP_number_model.
enum
{
  AEABI_FP_number_model_none = 0,
  AEABI_FP_number_model_ieee754_number = 1,
  AEABI_FP_number_model_rtabi = 2,
  AEABI_FP_number_model_ieee754_all = 3
};

// Values for Tag_ABI_VFP_args.
enum
{
  AEABI_VFP_args_base = 0,
  AEABI_VFP_args_vfp = 1,
  AEABI_VFP_args_toolchain = 2,
  AEABI_VFP_args_compatible = 3
};

// For Exception Index Table. (Exception handling ABI for the ARM
// architectue, Section 5)
enum
{
  EXIDX_CANTUNWIND = 1,
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_ARM_H)
