/* AMDGPU ELF support for BFD.

   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _ELF_AMDGPU_H
#define _ELF_AMDGPU_H

#include "elf/reloc-macros.h"

/* e_ident[EI_ABIVERSION] values, when e_ident[EI_OSABI] is
   ELFOSABI_AMDGPU_HSA.  */

#define ELFABIVERSION_AMDGPU_HSA_V2 0
#define ELFABIVERSION_AMDGPU_HSA_V3 1
#define ELFABIVERSION_AMDGPU_HSA_V4 2
#define ELFABIVERSION_AMDGPU_HSA_V5 3

/* Processor selection mask for EF_AMDGPU_MACH_* values.  */

#define EF_AMDGPU_MACH 0x0ff
#define EF_AMDGPU_MACH_AMDGCN_MIN 0x020

#define EF_AMDGPU_MACH_AMDGCN_GFX600  0x020
#define EF_AMDGPU_MACH_AMDGCN_GFX601  0x021
#define EF_AMDGPU_MACH_AMDGCN_GFX700  0x022
#define EF_AMDGPU_MACH_AMDGCN_GFX701  0x023
#define EF_AMDGPU_MACH_AMDGCN_GFX702  0x024
#define EF_AMDGPU_MACH_AMDGCN_GFX703  0x025
#define EF_AMDGPU_MACH_AMDGCN_GFX704  0x026
#define EF_AMDGPU_MACH_AMDGCN_GFX801  0x028
#define EF_AMDGPU_MACH_AMDGCN_GFX802  0x029
#define EF_AMDGPU_MACH_AMDGCN_GFX803  0x02a
#define EF_AMDGPU_MACH_AMDGCN_GFX810  0x02b
#define EF_AMDGPU_MACH_AMDGCN_GFX900  0x02c
#define EF_AMDGPU_MACH_AMDGCN_GFX902  0x02d
#define EF_AMDGPU_MACH_AMDGCN_GFX904  0x02e
#define EF_AMDGPU_MACH_AMDGCN_GFX906  0x02f
#define EF_AMDGPU_MACH_AMDGCN_GFX908  0x030
#define EF_AMDGPU_MACH_AMDGCN_GFX909  0x031
#define EF_AMDGPU_MACH_AMDGCN_GFX90C  0x032
#define EF_AMDGPU_MACH_AMDGCN_GFX1010 0x033
#define EF_AMDGPU_MACH_AMDGCN_GFX1011 0x034
#define EF_AMDGPU_MACH_AMDGCN_GFX1012 0x035
#define EF_AMDGPU_MACH_AMDGCN_GFX1030 0x036
#define EF_AMDGPU_MACH_AMDGCN_GFX1031 0x037
#define EF_AMDGPU_MACH_AMDGCN_GFX1032 0x038
#define EF_AMDGPU_MACH_AMDGCN_GFX1033 0x039
#define EF_AMDGPU_MACH_AMDGCN_GFX602  0x03a
#define EF_AMDGPU_MACH_AMDGCN_GFX705  0x03b
#define EF_AMDGPU_MACH_AMDGCN_GFX805  0x03c
#define EF_AMDGPU_MACH_AMDGCN_GFX1035 0x03d
#define EF_AMDGPU_MACH_AMDGCN_GFX1034 0x03e
#define EF_AMDGPU_MACH_AMDGCN_GFX90A  0x03f
#define EF_AMDGPU_MACH_AMDGCN_GFX940  0x040
#define EF_AMDGPU_MACH_AMDGCN_GFX1013 0x042
#define EF_AMDGPU_MACH_AMDGCN_GFX1036 0x045

/* Code object v3 machine flags.  */

#define EF_AMDGPU_FEATURE_XNACK_V3   0x100
#define EF_AMDGPU_FEATURE_SRAMECC_V3 0x200

/* Code object v4 (and later) machine flags.  */

#define EF_AMDGPU_FEATURE_XNACK_V4             0x300
#define EF_AMDGPU_FEATURE_XNACK_UNSUPPORTED_V4 0x000
#define EF_AMDGPU_FEATURE_XNACK_ANY_V4         0x100
#define EF_AMDGPU_FEATURE_XNACK_OFF_V4         0x200
#define EF_AMDGPU_FEATURE_XNACK_ON_V4          0x300

#define EF_AMDGPU_FEATURE_SRAMECC_V4             0xc00
#define EF_AMDGPU_FEATURE_SRAMECC_UNSUPPORTED_V4 0x000
#define EF_AMDGPU_FEATURE_SRAMECC_ANY_V4         0x400
#define EF_AMDGPU_FEATURE_SRAMECC_OFF_V4         0x800
#define EF_AMDGPU_FEATURE_SRAMECC_ON_V4          0xc00

/* Notes. */

#define NT_AMDGPU_METADATA                32

/* Relocations.  */

START_RELOC_NUMBERS (elf_amdgpu_reloc_type)
 RELOC_NUMBER (R_AMDGPU_NONE,           0)
 RELOC_NUMBER (R_AMDGPU_ABS32_LO,       1)
 RELOC_NUMBER (R_AMDGPU_ABS32_HI,       2)
 RELOC_NUMBER (R_AMDGPU_ABS64,          3)
 RELOC_NUMBER (R_AMDGPU_REL32,          4)
 RELOC_NUMBER (R_AMDGPU_REL64,          5)
 RELOC_NUMBER (R_AMDGPU_ABS32,          6)
 RELOC_NUMBER (R_AMDGPU_GOTPCREL,       7)
 RELOC_NUMBER (R_AMDGPU_GOTPCREL32_LO,  8)
 RELOC_NUMBER (R_AMDGPU_GOTPCREL32_HI,  9)
 RELOC_NUMBER (R_AMDGPU_REL32_LO,      10)
 RELOC_NUMBER (R_AMDGPU_REL32_HI,      11)
 RELOC_NUMBER (R_AMDGPU_RELATIVE64,    13)
 RELOC_NUMBER (R_AMDGPU_REL16,         16)
END_RELOC_NUMBERS (R_AMDGPU_max)

#endif /* _ELF_AMDGPU_H */
