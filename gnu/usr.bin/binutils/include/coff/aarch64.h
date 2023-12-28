/* AArch64 COFF support for BFD.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.

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

#define COFFAARCH64 1

#define L_LNNO_SIZE 2
#define INCLUDE_COMDAT_FIELDS_IN_AUXENT
#include "coff/external.h"

#define F_AARCH64_ARCHITECTURE_MASK	(0x4000)

#define	AARCH64MAGIC	0xaa64  /* From Microsoft specification. */

#undef  BADMAG
#define BADMAG(x) ((x).f_magic != AARCH64MAGIC)
#define AARCH64         1                 /* Customize coffcode.h.  */

#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b

#define OMAGIC          0404    /* Object files, eg as output.  */
#define ZMAGIC          IMAGE_NT_OPTIONAL_HDR64_MAGIC    /* Demand load format, eg normal ld output 0x10b.  */
#define STMAGIC		0401	/* Target shlib.  */
#define SHMAGIC		0443	/* Host   shlib.  */

/* define some NT default values */
/*  #define NT_IMAGE_BASE        0x400000 moved to internal.h */
#define NT_SECTION_ALIGNMENT 0x1000
#define NT_FILE_ALIGNMENT    0x200
#define NT_DEF_RESERVE       0x100000
#define NT_DEF_COMMIT        0x1000

/* We use the .rdata section to hold read only data.  */
#define _LIT	".rdata"

/********************** RELOCATION DIRECTIVES **********************/
struct external_reloc
{
  char r_vaddr[4];
  char r_symndx[4];
  char r_type[2];
};

#define RELOC struct external_reloc
#define RELSZ 10

/* ARM64 relocations types. */


#define IMAGE_REL_ARM64_ABSOLUTE        0x0000  /* No relocation required */
#define IMAGE_REL_ARM64_ADDR32          0x0001  /* The 32-bit VA of the target. */
#define IMAGE_REL_ARM64_ADDR32NB        0x0002  /* The 32-bit RVA of the target. */
#define IMAGE_REL_ARM64_BRANCH26        0x0003  /* The 26-bit relative displacement to the target, for B and BL instructions. */
#define IMAGE_REL_ARM64_PAGEBASE_REL21  0x0004  /* The page base of the target, for ADRP instruction. */
#define IMAGE_REL_ARM64_REL21           0x0005  /* The 12-bit relative displacement to the target, for instruction ADR */
#define IMAGE_REL_ARM64_PAGEOFFSET_12A  0x0006  /* The 12-bit page offset of the target, for instructions ADD/ADDS (immediate) with zero shift. */
#define IMAGE_REL_ARM64_PAGEOFFSET_12L  0x0007  /* The 12-bit page offset of the target, for instruction LDR (indexed, unsigned immediate). */
#define IMAGE_REL_ARM64_SECREL          0x0008  /* The 32-bit offset of the target from the beginning of its section. This is used to support debugging information and static thread local storage. */
#define IMAGE_REL_ARM64_SECREL_LOW12A   0x0009  /* Bit 0:11 of section offset of the target, for instructions ADD/ADDS (immediate) with zero shift. */
#define IMAGE_REL_ARM64_SECREL_HIGH12A  0x000A  /* Bit 12:23 of section offset of the target, for instructions ADD/ADDS (immediate) with zero shift. */
#define IMAGE_REL_ARM64_SECREL_LOW12L   0x000B  /* Bit 0:11 of section offset of the target, for instruction LDR (indexed, unsigned immediate). */
#define IMAGE_REL_ARM64_TOKEN           0x000C  /* CLR token */
#define IMAGE_REL_ARM64_SECTION         0x000D  /* The 16-bit section index of the section that contains the target. This is used to support debugging information. */
#define IMAGE_REL_ARM64_ADDR64          0x000E  /* The 64-bit VA of the relocation target. */
#define IMAGE_REL_ARM64_BRANCH19        0x000F  /* 19 bit offset << 2 & sign ext. for conditional B */
#define IMAGE_REL_ARM64_BRANCH14        0x0010  /* The 14-bit offset to the relocation target, for instructions TBZ and TBNZ. */
#define IMAGE_REL_ARM64_REL32           0x0011  /* The 32-bit relative address from the byte following the relocation. */

#define ARM_NOTE_SECTION ".note"
