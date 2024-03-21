/* C-SKY ELF support for BFD.
   Copyright (C) 1998-2023 Free Software Foundation, Inc.
   Contributed by C-SKY Microsystems and Mentor Graphics.

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

#ifndef _ELF_CSKY_H
#define _ELF_CSKY_H

#include "elf/reloc-macros.h"

/* Values of relocation types according to the ABI doc.
   The order should be consistent with csky bfd reloc type
   table in bfd-in2.h.  */
START_RELOC_NUMBERS (elf_csky_reloc_type)
    RELOC_NUMBER (R_CKCORE_NONE,0)
    RELOC_NUMBER (R_CKCORE_ADDR32,1)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM8BY4,2)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM11BY2,3)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM4BY2,4)
    RELOC_NUMBER (R_CKCORE_PCREL32,5)
    RELOC_NUMBER (R_CKCORE_PCREL_JSR_IMM11BY2,6)
    RELOC_NUMBER (R_CKCORE_GNU_VTINHERIT,7)
    RELOC_NUMBER (R_CKCORE_GNU_VTENTRY,8)
    RELOC_NUMBER (R_CKCORE_RELATIVE,9)
    RELOC_NUMBER (R_CKCORE_COPY,10)
    RELOC_NUMBER (R_CKCORE_GLOB_DAT,11)
    RELOC_NUMBER (R_CKCORE_JUMP_SLOT,12)
    RELOC_NUMBER (R_CKCORE_GOTOFF,13)
    RELOC_NUMBER (R_CKCORE_GOTPC,14)
    RELOC_NUMBER (R_CKCORE_GOT32,15)
    RELOC_NUMBER (R_CKCORE_PLT32,16)
    RELOC_NUMBER (R_CKCORE_ADDRGOT,17)
    RELOC_NUMBER (R_CKCORE_ADDRPLT,18)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM26BY2,19)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM16BY2,20)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM16BY4,21)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM10BY2,22)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM10BY4,23)
    RELOC_NUMBER (R_CKCORE_ADDR_HI16,24)
    RELOC_NUMBER (R_CKCORE_ADDR_LO16,25)
    RELOC_NUMBER (R_CKCORE_GOTPC_HI16,26)
    RELOC_NUMBER (R_CKCORE_GOTPC_LO16,27)
    RELOC_NUMBER (R_CKCORE_GOTOFF_HI16,28)
    RELOC_NUMBER (R_CKCORE_GOTOFF_LO16,29)
    RELOC_NUMBER (R_CKCORE_GOT12,30)
    RELOC_NUMBER (R_CKCORE_GOT_HI16,31)
    RELOC_NUMBER (R_CKCORE_GOT_LO16,32)
    RELOC_NUMBER (R_CKCORE_PLT12,33)
    RELOC_NUMBER (R_CKCORE_PLT_HI16,34)
    RELOC_NUMBER (R_CKCORE_PLT_LO16,35)
    RELOC_NUMBER (R_CKCORE_ADDRGOT_HI16,36)
    RELOC_NUMBER (R_CKCORE_ADDRGOT_LO16,37)
    RELOC_NUMBER (R_CKCORE_ADDRPLT_HI16,38)
    RELOC_NUMBER (R_CKCORE_ADDRPLT_LO16,39)
    RELOC_NUMBER (R_CKCORE_PCREL_JSR_IMM26BY2,40)
    RELOC_NUMBER (R_CKCORE_TOFFSET_LO16, 41)
    RELOC_NUMBER (R_CKCORE_DOFFSET_LO16, 42)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM18BY2, 43)
    RELOC_NUMBER (R_CKCORE_DOFFSET_IMM18, 44)
    RELOC_NUMBER (R_CKCORE_DOFFSET_IMM18BY2, 45)
    RELOC_NUMBER (R_CKCORE_DOFFSET_IMM18BY4, 46)
    RELOC_NUMBER (R_CKCORE_GOTOFF_IMM18, 47)
    RELOC_NUMBER (R_CKCORE_GOT_IMM18BY4, 48)
    RELOC_NUMBER (R_CKCORE_PLT_IMM18BY4, 49)
    RELOC_NUMBER (R_CKCORE_PCREL_IMM7BY4, 50)
    RELOC_NUMBER (R_CKCORE_TLS_LE32, 51)
    RELOC_NUMBER (R_CKCORE_TLS_IE32, 52)
    RELOC_NUMBER (R_CKCORE_TLS_GD32, 53)
    RELOC_NUMBER (R_CKCORE_TLS_LDM32, 54)
    RELOC_NUMBER (R_CKCORE_TLS_LDO32, 55)
    RELOC_NUMBER (R_CKCORE_TLS_DTPMOD32, 56)
    RELOC_NUMBER (R_CKCORE_TLS_DTPOFF32, 57)
    RELOC_NUMBER (R_CKCORE_TLS_TPOFF32, 58)
    RELOC_NUMBER (R_CKCORE_PCREL_FLRW_IMM8BY4, 59)
    RELOC_NUMBER (R_CKCORE_NOJSRI, 60)
    RELOC_NUMBER (R_CKCORE_CALLGRAPH, 61)
    RELOC_NUMBER (R_CKCORE_IRELATIVE, 62)
    RELOC_NUMBER (R_CKCORE_PCREL_BLOOP_IMM4BY4, 63)
    RELOC_NUMBER (R_CKCORE_PCREL_BLOOP_IMM12BY4, 64)
END_RELOC_NUMBERS (R_CKCORE_MAX)

/* Additional section types.  */
#define SHT_CSKY_ATTRIBUTES       0x70000001 /* Section holds attributes.  */

/* Object attribute tags.  */
enum
{
  /* 0-3 are generic. */
  /* Arch name for this object file.  */
  Tag_CSKY_ARCH_NAME = 4,
  Tag_CSKY_CPU_NAME = 5,

  /* ISA flags for this object file.  */
  Tag_CSKY_ISA_FLAGS,
  Tag_CSKY_ISA_EXT_FLAGS,

  /* CSKY DSP version used by this object file.  */
  Tag_CSKY_DSP_VERSION,

  /* CSKY VDSP version used by this object file.  */
  Tag_CSKY_VDSP_VERSION,

  /* CSKY FPU version used by this object file.  */
  Tag_CSKY_FPU_VERSION = 0x10,
  /* FPU ABI.  params: Soft GR/Hard GR/Hard FR. */
  Tag_CSKY_FPU_ABI,
  /* Rounding Support.  */
  Tag_CSKY_FPU_ROUNDING,
  /* Denormal Support.  */
  Tag_CSKY_FPU_DENORMAL,
  /* Exeception Support.  */
  Tag_CSKY_FPU_Exception,
  /* Number Module Support("IEEE 754").  */
  Tag_CSKY_FPU_NUMBER_MODULE,
  /* Half/Single/Double.  */
  Tag_CSKY_FPU_HARDFP,

  Tag_CSKY_MAX,
};

/* Object attribute values.  */
enum
{
  /* Values defined for Tag_CSKY_DSP_VERSION.  */
  VAL_CSKY_DSP_VERSION_EXTENSION = 1,	/* hi-lo DSP extension.  */
  VAL_CSKY_DSP_VERSION_2 = 2,		/* CK803s EDSP.  */
};

enum
{
  /* Values defined for Tag_CSKY_VDSP_VERSION.  */
  VAL_CSKY_VDSP_VERSION_1 = 1,	/* VDSP version 1.  */
  VAL_CSKY_VDSP_VERSION_2	/* VDSP version 1.  */
};

enum
{
  /* Values defined for Tag_CSKY_FPU_VERSION.  */
  VAL_CSKY_FPU_VERSION_1 = 1,	/* ABIV1 FPU.  */
  VAL_CSKY_FPU_VERSION_2,	/* ABIV2 FPU.  */
};

enum
{
  VAL_CSKY_FPU_ABI_SOFT = 1,
  VAL_CSKY_FPU_ABI_SOFTFP,
  VAL_CSKY_FPU_ABI_HARD,
};

enum
{
  VAL_CSKY_FPU_HARDFP_HALF = 1,
  VAL_CSKY_FPU_HARDFP_SINGLE = 2,
  VAL_CSKY_FPU_HARDFP_DOUBLE = 4,
};

#endif /* _ELF_CSKY_H  */
