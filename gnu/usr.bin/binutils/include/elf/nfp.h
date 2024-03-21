/* NFP ELF support for BFD.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   Contributed by Francois H. Theron <francois.theron@netronome.com>

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

#ifndef _ELF_NFP_H
#define _ELF_NFP_H

#include "bfd.h"
#include "elf/common.h"
#include "elf/reloc-macros.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ET_NFP_PARTIAL_REL (ET_LOPROC + ET_REL)
#define ET_NFP_PARTIAL_EXEC (ET_LOPROC + ET_EXEC)

/* NFP e_flags - chip family
   Valid values for FAMILY are:
   0x3200 - NFP-32xx
   0x6000 - NFP-6xxx/NFP-4xxx.  */
#define EF_NFP_MACH(ef_nfp)        (((ef_nfp) >> 8) & 0xFFFF)
#define EF_NFP_SET_MACH(nfp_fam)   (((nfp_fam) & 0xFFFF) << 8)

#define E_NFP_MACH_3200	0x3200
#define E_NFP_MACH_6000	0x6000

#define NFP_3200_CPPTGT_MSF0     1
#define NFP_3200_CPPTGT_QDR      2
#define NFP_3200_CPPTGT_MSF1     3
#define NFP_3200_CPPTGT_HASH     4
#define NFP_3200_CPPTGT_MU       7
#define NFP_3200_CPPTGT_GS       8
#define NFP_3200_CPPTGT_PCIE     9
#define NFP_3200_CPPTGT_ARM     10
#define NFP_3200_CPPTGT_CRYPTO  12
#define NFP_3200_CPPTGT_CAP     13
#define NFP_3200_CPPTGT_CT      14
#define NFP_3200_CPPTGT_CLS     15

#define NFP_6000_CPPTGT_NBI      1
#define NFP_6000_CPPTGT_VQDR     2
#define NFP_6000_CPPTGT_ILA      6
#define NFP_6000_CPPTGT_MU       7
#define NFP_6000_CPPTGT_PCIE     9
#define NFP_6000_CPPTGT_ARM     10
#define NFP_6000_CPPTGT_CRYPTO  12
#define NFP_6000_CPPTGT_CTXPB   14
#define NFP_6000_CPPTGT_CLS     15

/* NFP Section types
   MECONFIG - NFP-32xx only, ME CSR configurations
   INITREG - A generic register initialisation section (chip or ME CSRs/GPRs)
   UDEBUG - Legacy-style debug data section.  */
#define SHT_NFP_MECONFIG	(SHT_LOPROC + 1)
#define SHT_NFP_INITREG		(SHT_LOPROC + 2)
#define SHT_NFP_UDEBUG		SHT_LOUSER

/* NFP SECTION flags
     ELF-64 sh_flags is 64-bit, but there is no info on what the upper 32 bits
     are expected to be used for, it is not marked reserved either.
     We'll use them for NFP-specific flags since we don't use ELF-32.

   INIT - Sections that are loaded and executed before the final text
	  microcode.  Non-code INIT sections are loaded first, then other
	  memory secions, then INIT2 sections, then INIT-code sections.
   INIT2 - Sections that are loaded before INIT-code sections, used for
	   transient configuration before executing INIT-code section
	   microcode.
   SCS - The number of additional ME codestores being shared with the group's
	 base ME of the section, e.g. 0 for no SCS, 1 for dual and 3 for
	 quad.  If this is 0 it is possible that stagger-style SCS codestore
	 sections are being used.  For stagger-style each section is simply
	 loaded directly to the ME it is assigned to.  If these flags are
	 used, virtual address space loading will be used - one large section
	 loaded to the group's base ME will be packed across shared MEs by
	 hardware.  This is not available on all ME versions.

    NFP_ELF_SHF_GET_SCS (val) returns the number of additional codestores
    being shared with the group's base ME, e.g. 0 for no SCS,
    1 for dual SCS, 3 for quad SCS.  */

#define SHF_NFP_INIT		0x80000000
#define SHF_NFP_INIT2		0x40000000
#define SHF_NFP_SCS(shf)	(((shf) >> 32) & 0xFF)
#define SHF_NFP_SET_SCS(v)	((uint64_t) ((v) & 0xFF) << 32)

/* NFP Section Info
   For PROGBITS and NOBITS sections:
     MEMTYPE - the memory type
     DOMAIN - The island ID and ME number where the data will be loaded.
	      For NFP-32xx, this is an island number or linear ME number.
	      For NFP-6xxx, DOMAIN<15:8> == island ID, DOMAIN<7:0> is 0 based
	      ME number (if applicable).
   For INITREG sections:
     ISLAND - island ID (if it's a ME target, ME numbers are in the
	      section data)
     CPPTGT - CPP Target ID
     CPPACTRD - CPP Read Action
     CPPTOKRD - CPP Read Token
     CPPACTWR - CPP Write Action
     CPPTOKWR - CPP Write Token
     ORDER - Controls the order in which the loader processes sections with
	     the same info fields.  */

#define SHI_NFP_DOMAIN(shi)		(((shi) >> 16) & 0xFFFF)
#define SHI_NFP_MEMTYPE(shi)		( (shi) & 0xFFFF)
#define SHI_NFP_SET_DOMAIN(v)		(((v) & 0xFFFF) << 16)
#define SHI_NFP_SET_MEMTYPE(v)		( (v) & 0xFFFF)

#define SHI_NFP_IREG_ISLAND(shi)	(((shi) >> 26) & 0x3F)
#define SHI_NFP_IREG_CPPTGT(shi)	(((shi) >> 22) &  0xF)
#define SHI_NFP_IREG_CPPACTRD(shi)	(((shi) >> 17) & 0x1F)
#define SHI_NFP_IREG_CPPTOKRD(shi)	(((shi) >> 15) &  0x3)
#define SHI_NFP_IREG_CPPACTWR(shi)	(((shi) >> 10) & 0x1F)
#define SHI_NFP_IREG_CPPTOKWR(shi)	(((shi) >> 8)  &  0x3)
#define SHI_NFP_IREG_ORDER(shi)		( (shi) & 0xFF)
#define SHI_NFP_SET_IREG_ISLAND(v)	(((v) & 0x3F) << 26)
#define SHI_NFP_SET_IREG_CPPTGT(v)	(((v) &  0xF) << 22)
#define SHI_NFP_SET_IREG_CPPACTRD(v)	(((v) & 0x1F) << 17)
#define SHI_NFP_SET_IREG_CPPTOKRD(v)	(((v) &  0x3) << 15)
#define SHI_NFP_SET_IREG_CPPACTWR(v)	(((v) & 0x1F) << 10)
#define SHI_NFP_SET_IREG_CPPTOKWR(v)	(((v) &  0x3) << 8)
#define SHI_NFP_SET_IREG_ORDER(v)	( (v) & 0xFF)

/* CtXpb/reflect_read_sig_init/reflect_write_sig_init
   identifies Init-CSR sections for ME CSRs.  */
#define SHI_NFP_6000_IS_IREG_MECSR(shi) ( \
  SHI_NFP_IREG_CPPTGT (shi) == NFP_6000_CPPTGT_CTXPB \
  && SHI_NFP_IREG_CPPACTRD (shi) == 2 \
  && SHI_NFP_IREG_CPPTOKRD (shi) == 1 \
  && SHI_NFP_IREG_CPPACTWR (shi) == 3 \
  && SHI_NFP_IREG_CPPTOKWR (shi) == 1 \
)

/* Transient INITREG sections will be validated against the target
   but will not be kept - validate, write or read and discard.
   They will still be handled last (in order).  */
#define SHI_NFP_IREG_ORDER_TRANSIENT	0xFF

/* Below are some extra macros to translate SHI fields in more specific
   contexts.

   For NFP-32xx, DOMAIN is set to a global linear ME number (0 to 39).
   An NFP-32xx has 8 MEs per island and up to 5 islands.  */

#define SHI_NFP_3200_ISLAND(shi)	((SHI_NFP_DOMAIN (shi) >> 3) & 0x7)
#define SHI_NFP_3200_MENUM(shi)		( SHI_NFP_DOMAIN (shi)       & 0x7)
#define SHI_NFP_SET_3200_ISLAND(v)	SHI_NFP_SET_DOMAIN (((v) & 0x7) << 3)
#define SHI_NFP_SET_3200_MENUM(v)	SHI_NFP_SET_DOMAIN ( (v) & 0x7)

#define SHI_NFP_ISLAND(shi)		((SHI_NFP_DOMAIN (shi) >> 8) & 0xFF)
#define SHI_NFP_MENUM(shi)		( SHI_NFP_DOMAIN (shi)       & 0xFF)
#define SHI_NFP_SET_ISLAND(shi)		SHI_NFP_SET_DOMAIN (((shi) & 0xFF) << 8)
#define SHI_NFP_SET_MENUM(shi)		SHI_NFP_SET_DOMAIN ( (shi) & 0xFF)

#define SHI_NFP_MEMTYPE_NONE 		0
#define SHI_NFP_MEMTYPE_USTORE 		1
#define SHI_NFP_MEMTYPE_LMEM 		2
#define SHI_NFP_MEMTYPE_CLS 		3
#define SHI_NFP_MEMTYPE_DRAM 		4
#define SHI_NFP_MEMTYPE_MU 		4
#define SHI_NFP_MEMTYPE_SRAM 		5
#define SHI_NFP_MEMTYPE_GS 		6
#define SHI_NFP_MEMTYPE_PPC_LMEM 	7
#define SHI_NFP_MEMTYPE_PPC_SMEM 	8
#define SHI_NFP_MEMTYPE_EMU_CACHE 	9

/* VTP_FORCE is for use by the NFP Linker+Loader only.  */
#define NFP_IREG_VTP_FORCE		0
#define NFP_IREG_VTP_CONST		1
#define NFP_IREG_VTP_REQUIRED		2
#define NFP_IREG_VTP_VOLATILE_INIT	3
#define NFP_IREG_VTP_VOLATILE_NOINIT	4
#define NFP_IREG_VTP_INVALID		5

/* Init-CSR entry w0 fields:
   NLW - Not Last Word
   CTX - ME context number (if applicable)
   VTP - Value type
   COH - CPP Offset High 8 bits.  */
#define NFP_IREG_ENTRY_WO_NLW(w0) (((w0) >> 31) & 0x1)
#define NFP_IREG_ENTRY_WO_CTX(w0) (((w0) >> 28) & 0x7)
#define NFP_IREG_ENTRY_WO_VTP(w0) (((w0) >> 25) & 0x7)
#define NFP_IREG_ENTRY_WO_COH(w0) (((w0) >> 0) & 0xFF)

typedef struct
{
  uint32_t w0;
  uint32_t cpp_offset_lo;
  uint32_t val;
  uint32_t mask;
} Elf_Nfp_InitRegEntry;

typedef struct
{
  uint32_t ctx_enables;
  uint32_t entry;
  uint32_t misc_control;
  uint32_t reserved;
} Elf_Nfp_MeConfig;

/* Relocations.  */
START_RELOC_NUMBERS (elf_nfp3200_reloc_type)
    RELOC_NUMBER (R_NFP3200_NOTYPE, 0)
    RELOC_NUMBER (R_NFP3200_W32LE, 1)
    RELOC_NUMBER (R_NFP3200_SRC8_A, 2)
    RELOC_NUMBER (R_NFP3200_SRC8_B, 3)
    RELOC_NUMBER (R_NFP3200_IMMED8_I, 4)
    RELOC_NUMBER (R_NFP3200_SC, 5)
    RELOC_NUMBER (R_NFP3200_IMMED_LO16_I_A, 6)
    RELOC_NUMBER (R_NFP3200_IMMED_LO16_I_B, 7)
    RELOC_NUMBER (R_NFP3200_SRC7_B, 8)
    RELOC_NUMBER (R_NFP3200_SRC7_A, 9)
    RELOC_NUMBER (R_NFP3200_SRC8_I_B, 10)
    RELOC_NUMBER (R_NFP3200_SRC8_I_A, 11)
    RELOC_NUMBER (R_NFP3200_IMMED_HI16_I_A, 12)
    RELOC_NUMBER (R_NFP3200_IMMED_HI16_I_B, 13)
    RELOC_NUMBER (R_NFP3200_RSVD_0, 14)
    RELOC_NUMBER (R_NFP3200_RSVD_1, 15)
    RELOC_NUMBER (R_NFP3200_RSVD_2, 16)
    RELOC_NUMBER (R_NFP3200_RSVD_3, 17)
    RELOC_NUMBER (R_NFP3200_RSVD_4, 18)
    RELOC_NUMBER (R_NFP3200_RSVD_5, 19)
    RELOC_NUMBER (R_NFP3200_RSVD_6, 20)
    RELOC_NUMBER (R_NFP3200_W64LE, 21)
    RELOC_NUMBER (R_NFP3200_W32BE, 22)
    RELOC_NUMBER (R_NFP3200_W64BE, 23)
    RELOC_NUMBER (R_NFP3200_W32LE_AND, 24)
    RELOC_NUMBER (R_NFP3200_W32BE_AND, 25)
    RELOC_NUMBER (R_NFP3200_W32LE_OR, 26)
    RELOC_NUMBER (R_NFP3200_W32BE_OR, 27)
    RELOC_NUMBER (R_NFP3200_W64LE_AND, 28)
    RELOC_NUMBER (R_NFP3200_W64BE_AND, 29)
    RELOC_NUMBER (R_NFP3200_W64LE_OR, 30)
    RELOC_NUMBER (R_NFP3200_W64BE_OR, 31)
END_RELOC_NUMBERS (R_NFP3200_MAX)

START_RELOC_NUMBERS (elf_nfp_reloc_type)
    RELOC_NUMBER (R_NFP_NOTYPE, 0)
    RELOC_NUMBER (R_NFP_W32LE, 1)
    RELOC_NUMBER (R_NFP_SRC8_A, 2)
    RELOC_NUMBER (R_NFP_SRC8_B, 3)
    RELOC_NUMBER (R_NFP_IMMED8_I, 4)
    RELOC_NUMBER (R_NFP_SC, 5)
    RELOC_NUMBER (R_NFP_IMMED_LO16_I_A, 6)
    RELOC_NUMBER (R_NFP_IMMED_LO16_I_B, 7)
    RELOC_NUMBER (R_NFP_SRC7_B, 8)
    RELOC_NUMBER (R_NFP_SRC7_A, 9)
    RELOC_NUMBER (R_NFP_SRC8_I_B, 10)
    RELOC_NUMBER (R_NFP_SRC8_I_A, 11)
    RELOC_NUMBER (R_NFP_IMMED_HI16_I_A, 12)
    RELOC_NUMBER (R_NFP_IMMED_HI16_I_B, 13)
    RELOC_NUMBER (R_NFP_W64LE, 14)
    RELOC_NUMBER (R_NFP_SH_INFO, 15)
    RELOC_NUMBER (R_NFP_W32BE, 16)
    RELOC_NUMBER (R_NFP_W64BE, 17)
    RELOC_NUMBER (R_NFP_W32_29_24, 18)
    RELOC_NUMBER (R_NFP_W32LE_AND, 19)
    RELOC_NUMBER (R_NFP_W32BE_AND, 20)
    RELOC_NUMBER (R_NFP_W32LE_OR, 21)
    RELOC_NUMBER (R_NFP_W32BE_OR, 22)
    RELOC_NUMBER (R_NFP_W64LE_AND, 23)
    RELOC_NUMBER (R_NFP_W64BE_AND, 24)
    RELOC_NUMBER (R_NFP_W64LE_OR, 25)
    RELOC_NUMBER (R_NFP_W64BE_OR, 26)
END_RELOC_NUMBERS (R_NFP_MAX)

#ifdef __cplusplus
}
#endif

#endif /* _ELF_NFP_H */
