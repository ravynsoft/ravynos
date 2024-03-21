/* SPARC ELF support for BFD.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   By Doug Evans, Cygnus Support, <dje@cygnus.com>.

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

#ifndef _ELF_SPARC_H
#define _ELF_SPARC_H

/* Processor specific flags for the ELF header e_flags field.  */

/* These are defined by Sun.  */

#define EF_SPARC_32PLUS_MASK	0xffff00	/* bits indicating V8+ type */
#define EF_SPARC_32PLUS		0x000100	/* generic V8+ features */
#define EF_SPARC_SUN_US1	0x000200	/* Sun UltraSPARC1 extensions */
#define EF_SPARC_HAL_R1		0x000400	/* HAL R1 extensions */
#define EF_SPARC_SUN_US3	0x000800	/* Sun UltraSPARCIII extensions */

#define EF_SPARC_LEDATA         0x800000	/* little endian data */

/* This name is used in the V9 ABI.  */
#define EF_SPARC_EXT_MASK	0xffff00	/* reserved for vendor extensions */

/* V9 memory models */
#define EF_SPARCV9_MM		0x3		/* memory model mask */
#define EF_SPARCV9_TSO		0x0		/* total store ordering */
#define EF_SPARCV9_PSO		0x1		/* partial store ordering */
#define EF_SPARCV9_RMO		0x2		/* relaxed store ordering */

/* Section indices.  */

#define SHN_BEFORE	SHN_LORESERVE		/* Used with SHF_ORDERED and...  */
#define SHN_AFTER	(SHN_LORESERVE + 1)	/* SHF_LINK_ORDER section flags. */

/* Section flags.  */

#define SHF_ORDERED		0x40000000	/* treat sh_link,sh_info specially */

/* Symbol types.  */

#define STT_REGISTER		13		/* global reg reserved to app. */

#include "elf/reloc-macros.h"

/* Relocation types.  */
START_RELOC_NUMBERS (elf_sparc_reloc_type)
  RELOC_NUMBER (R_SPARC_NONE, 0)
  RELOC_NUMBER (R_SPARC_8, 1)
  RELOC_NUMBER (R_SPARC_16, 2)
  RELOC_NUMBER (R_SPARC_32, 3)
  RELOC_NUMBER (R_SPARC_DISP8, 4)
  RELOC_NUMBER (R_SPARC_DISP16, 5)
  RELOC_NUMBER (R_SPARC_DISP32, 6)
  RELOC_NUMBER (R_SPARC_WDISP30, 7)
  RELOC_NUMBER (R_SPARC_WDISP22, 8)
  RELOC_NUMBER (R_SPARC_HI22, 9)
  RELOC_NUMBER (R_SPARC_22, 10)
  RELOC_NUMBER (R_SPARC_13, 11)
  RELOC_NUMBER (R_SPARC_LO10, 12)
  RELOC_NUMBER (R_SPARC_GOT10, 13)
  RELOC_NUMBER (R_SPARC_GOT13, 14)
  RELOC_NUMBER (R_SPARC_GOT22, 15)
  RELOC_NUMBER (R_SPARC_PC10, 16)
  RELOC_NUMBER (R_SPARC_PC22, 17)
  RELOC_NUMBER (R_SPARC_WPLT30, 18)
  RELOC_NUMBER (R_SPARC_COPY, 19)
  RELOC_NUMBER (R_SPARC_GLOB_DAT, 20)
  RELOC_NUMBER (R_SPARC_JMP_SLOT, 21)
  RELOC_NUMBER (R_SPARC_RELATIVE, 22)
  RELOC_NUMBER (R_SPARC_UA32, 23)

  /* ??? These 6 relocs are new but not currently used.  For binary
     compatibility in the sparc64-elf toolchain, we leave them out.
     A non-binary upward compatible change is expected for sparc64-elf.  */
#ifndef SPARC64_OLD_RELOCS
  /* ??? New relocs on the UltraSPARC.  Not sure what they're for yet.  */
  RELOC_NUMBER (R_SPARC_PLT32, 24)
  RELOC_NUMBER (R_SPARC_HIPLT22, 25)
  RELOC_NUMBER (R_SPARC_LOPLT10, 26)
  RELOC_NUMBER (R_SPARC_PCPLT32, 27)
  RELOC_NUMBER (R_SPARC_PCPLT22, 28)
  RELOC_NUMBER (R_SPARC_PCPLT10, 29)
#endif

  /* v9 relocs */
  RELOC_NUMBER (R_SPARC_10, 30)
  RELOC_NUMBER (R_SPARC_11, 31)
  RELOC_NUMBER (R_SPARC_64, 32)
  RELOC_NUMBER (R_SPARC_OLO10, 33)
  RELOC_NUMBER (R_SPARC_HH22, 34)
  RELOC_NUMBER (R_SPARC_HM10, 35)
  RELOC_NUMBER (R_SPARC_LM22, 36)
  RELOC_NUMBER (R_SPARC_PC_HH22, 37)
  RELOC_NUMBER (R_SPARC_PC_HM10, 38)
  RELOC_NUMBER (R_SPARC_PC_LM22, 39)
  RELOC_NUMBER (R_SPARC_WDISP16, 40)
  RELOC_NUMBER (R_SPARC_WDISP19, 41)
  RELOC_NUMBER (R_SPARC_UNUSED_42, 42)
  RELOC_NUMBER (R_SPARC_7, 43)
  RELOC_NUMBER (R_SPARC_5, 44)
  RELOC_NUMBER (R_SPARC_6, 45)
  RELOC_NUMBER (R_SPARC_DISP64, 46)
  RELOC_NUMBER (R_SPARC_PLT64, 47)
  RELOC_NUMBER (R_SPARC_HIX22, 48)
  RELOC_NUMBER (R_SPARC_LOX10, 49)
  RELOC_NUMBER (R_SPARC_H44, 50)
  RELOC_NUMBER (R_SPARC_M44, 51)
  RELOC_NUMBER (R_SPARC_L44, 52)
  RELOC_NUMBER (R_SPARC_REGISTER, 53)
  RELOC_NUMBER (R_SPARC_UA64, 54)
  RELOC_NUMBER (R_SPARC_UA16, 55)

  RELOC_NUMBER (R_SPARC_TLS_GD_HI22, 56)
  RELOC_NUMBER (R_SPARC_TLS_GD_LO10, 57)
  RELOC_NUMBER (R_SPARC_TLS_GD_ADD, 58)
  RELOC_NUMBER (R_SPARC_TLS_GD_CALL, 59)
  RELOC_NUMBER (R_SPARC_TLS_LDM_HI22, 60)
  RELOC_NUMBER (R_SPARC_TLS_LDM_LO10, 61)
  RELOC_NUMBER (R_SPARC_TLS_LDM_ADD, 62)
  RELOC_NUMBER (R_SPARC_TLS_LDM_CALL, 63)
  RELOC_NUMBER (R_SPARC_TLS_LDO_HIX22, 64)
  RELOC_NUMBER (R_SPARC_TLS_LDO_LOX10, 65)
  RELOC_NUMBER (R_SPARC_TLS_LDO_ADD, 66)
  RELOC_NUMBER (R_SPARC_TLS_IE_HI22, 67)
  RELOC_NUMBER (R_SPARC_TLS_IE_LO10, 68)
  RELOC_NUMBER (R_SPARC_TLS_IE_LD, 69)
  RELOC_NUMBER (R_SPARC_TLS_IE_LDX, 70)
  RELOC_NUMBER (R_SPARC_TLS_IE_ADD, 71)
  RELOC_NUMBER (R_SPARC_TLS_LE_HIX22, 72)
  RELOC_NUMBER (R_SPARC_TLS_LE_LOX10, 73)
  RELOC_NUMBER (R_SPARC_TLS_DTPMOD32, 74)
  RELOC_NUMBER (R_SPARC_TLS_DTPMOD64, 75)
  RELOC_NUMBER (R_SPARC_TLS_DTPOFF32, 76)
  RELOC_NUMBER (R_SPARC_TLS_DTPOFF64, 77)
  RELOC_NUMBER (R_SPARC_TLS_TPOFF32, 78)
  RELOC_NUMBER (R_SPARC_TLS_TPOFF64, 79)

  RELOC_NUMBER (R_SPARC_GOTDATA_HIX22, 80)
  RELOC_NUMBER (R_SPARC_GOTDATA_LOX10, 81)
  RELOC_NUMBER (R_SPARC_GOTDATA_OP_HIX22, 82)
  RELOC_NUMBER (R_SPARC_GOTDATA_OP_LOX10, 83)
  RELOC_NUMBER (R_SPARC_GOTDATA_OP, 84)

  RELOC_NUMBER (R_SPARC_H34, 85)
  RELOC_NUMBER (R_SPARC_SIZE32, 86)
  RELOC_NUMBER (R_SPARC_SIZE64, 87)
  RELOC_NUMBER (R_SPARC_WDISP10, 88)
  
  EMPTY_RELOC  (R_SPARC_max_std)

  RELOC_NUMBER (R_SPARC_JMP_IREL, 248)
  RELOC_NUMBER (R_SPARC_IRELATIVE, 249)
  RELOC_NUMBER (R_SPARC_GNU_VTINHERIT, 250)
  RELOC_NUMBER (R_SPARC_GNU_VTENTRY, 251)
  RELOC_NUMBER (R_SPARC_REV32, 252)

END_RELOC_NUMBERS (R_SPARC_max)

/* Relocation macros.  */

#define ELF64_R_TYPE_DATA(info) \
  (((bfd_signed_vma)(ELF64_R_TYPE(info) >> 8) ^ 0x800000) - 0x800000)
#define ELF64_R_TYPE_ID(info) \
  ((info) & 0xff)
#define ELF64_R_TYPE_INFO(data, type) \
  (((bfd_vma) ((data) & 0xffffff) << 8) | (bfd_vma) (type))

/* Values for Elf64_Dyn.d_tag.  */

#define DT_SPARC_REGISTER	0x70000001

/* Object attribute tags.  */
enum
{
  /* 0-3 are generic.  */
  Tag_GNU_Sparc_HWCAPS = 4,
  Tag_GNU_Sparc_HWCAPS2 = 8
};

/* Generally speaking the ELF_SPARC_HWCAP_* and ELF_SPARC_HWCAP2_*
   values match the AV_SPARC_* and AV2_SPARC_* bits respectively.

   However Solaris 11 introduced a backwards-incompatible change
   deprecating the RANDOM, TRANS and ASI_CACHE_SPARING bits in the
   AT_SUNW_CAP_HW1 flags, reusing the bits for the unrelated hwcaps
   FJATHHPC, FJDES and FJAES respectively.  In GNU/Linux we opted to
   keep the old hwcaps in Tag_GNU_Sparc_HWCAPS and allocate bits for
   FJATHHPC, FJDES and JFAES in Tag_GNU_Sparc_HWCAPS2.  */

#define ELF_SPARC_HWCAP_MUL32	0x00000001 /* umul/umulcc/smul/smulcc insns */
#define ELF_SPARC_HWCAP_DIV32	0x00000002 /* udiv/udivcc/sdiv/sdivcc insns */
#define ELF_SPARC_HWCAP_FSMULD	0x00000004 /* 'fsmuld' insn */
#define ELF_SPARC_HWCAP_V8PLUS	0x00000008 /* v9 insns available to 32bit */
#define ELF_SPARC_HWCAP_POPC	0x00000010 /* 'popc' insn */
#define ELF_SPARC_HWCAP_VIS	0x00000020 /* VIS insns */
#define ELF_SPARC_HWCAP_VIS2	0x00000040 /* VIS2 insns */
#define ELF_SPARC_HWCAP_ASI_BLK_INIT	\
				0x00000080 /* block init ASIs */
#define ELF_SPARC_HWCAP_FMAF	0x00000100 /* fused multiply-add */
#define ELF_SPARC_HWCAP_VIS3	0x00000400 /* VIS3 insns */
#define ELF_SPARC_HWCAP_HPC	0x00000800 /* HPC insns */
#define ELF_SPARC_HWCAP_RANDOM	0x00001000 /* 'random' insn */
#define ELF_SPARC_HWCAP_TRANS	0x00002000 /* transaction insns */
#define ELF_SPARC_HWCAP_FJFMAU	0x00004000 /* unfused multiply-add */
#define ELF_SPARC_HWCAP_IMA	0x00008000 /* integer multiply-add */
#define ELF_SPARC_HWCAP_ASI_CACHE_SPARING \
				0x00010000 /* cache sparing ASIs */
#define ELF_SPARC_HWCAP_AES	0x00020000 /* AES crypto insns */
#define ELF_SPARC_HWCAP_DES	0x00040000 /* DES crypto insns */
#define ELF_SPARC_HWCAP_KASUMI	0x00080000 /* KASUMI crypto insns */
#define ELF_SPARC_HWCAP_CAMELLIA \
				0x00100000 /* CAMELLIA crypto insns */
#define ELF_SPARC_HWCAP_MD5	0x00200000 /* MD5 hashing insns */
#define ELF_SPARC_HWCAP_SHA1	0x00400000 /* SHA1 hashing insns */
#define ELF_SPARC_HWCAP_SHA256	0x00800000 /* SHA256 hashing insns */
#define ELF_SPARC_HWCAP_SHA512	0x01000000 /* SHA512 hashing insns */
#define ELF_SPARC_HWCAP_MPMUL	0x02000000 /* Multiple Precision Multiply */
#define ELF_SPARC_HWCAP_MONT	0x04000000 /* Montgomery Mult/Sqrt */
#define ELF_SPARC_HWCAP_PAUSE	0x08000000 /* Pause insn */
#define ELF_SPARC_HWCAP_CBCOND	0x10000000 /* Compare and Branch insns */
#define ELF_SPARC_HWCAP_CRC32C	0x20000000 /* CRC32C insn */

#define ELF_SPARC_HWCAP2_FJATHPLUS 0x00000001 /* Fujitsu Athena+ */
#define ELF_SPARC_HWCAP2_VIS3B     0x00000002 /* Subset of VIS3 present on sparc64 X+ */
#define ELF_SPARC_HWCAP2_ADP       0x00000004 /* Application Data Protection */
#define ELF_SPARC_HWCAP2_SPARC5    0x00000008 /* The 29 new fp and sub instructions */
#define ELF_SPARC_HWCAP2_MWAIT     0x00000010 /* mwait instruction and load/monitor ASIs */
#define ELF_SPARC_HWCAP2_XMPMUL    0x00000020 /* XOR multiple precision multiply */
#define ELF_SPARC_HWCAP2_XMONT     0x00000040 /* XOR Montgomery mult/sqr instructions */
#define ELF_SPARC_HWCAP2_NSEC      \
                                   0x00000080 /* pause insn with support for nsec timings */
#define ELF_SPARC_HWCAP2_FJATHHPC  0x00001000 /* Fujitsu HPC instrs */
#define ELF_SPARC_HWCAP2_FJDES     0x00002000 /* Fujitsu DES instrs */
#define ELF_SPARC_HWCAP2_FJAES     0x00010000 /* Fujitsu AES instrs */

#define ELF_SPARC_HWCAP2_SPARC6    0x00020000 /* OSA2017 new instructions */
#define ELF_SPARC_HWCAP2_ONADDSUB  0x00040000 /* Oracle Number add/subtract */
#define ELF_SPARC_HWCAP2_ONMUL     0x00080000 /* Oracle Number multiply */
#define ELF_SPARC_HWCAP2_ONDIV     0x00100000 /* Oracle Number divide */
#define ELF_SPARC_HWCAP2_DICTUNP   0x00200000 /* Dictionary unpack instruction */
#define ELF_SPARC_HWCAP2_FPCMPSHL  0x00400000 /* Partition compare with shifted result */
#define ELF_SPARC_HWCAP2_RLE       0x00800000 /* Run-length encoded burst and length */
#define ELF_SPARC_HWCAP2_SHA3      0x01000000 /* SHA3 instruction */

#endif /* _ELF_SPARC_H */
