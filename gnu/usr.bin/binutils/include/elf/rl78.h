/* RL78 ELF support for BFD.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#ifndef _ELF_RL78_H
#define _ELF_RL78_H

#include "elf/reloc-macros.h"

/* Note that there are a few internal relocation types used by the
   linker to do link-time relaxation.  If you update this file, please
   check elf32-rl78.c to see if any of the internal relocations need to
   be, er, relocated.  */

/* Preliminary relocations.  */
START_RELOC_NUMBERS (elf_rl78_reloc_type)

  RELOC_NUMBER (R_RL78_NONE,         0x00)
  /* These are for data, and are bi-endian.  */
  RELOC_NUMBER (R_RL78_DIR32,        0x01) /* Was: R_RL78_32.  */
  RELOC_NUMBER (R_RL78_DIR24S,       0x02) /* Was: R_RL78_24.  */
  RELOC_NUMBER (R_RL78_DIR16,        0x03)
  RELOC_NUMBER (R_RL78_DIR16U,       0x04) /* Was: R_RL78_16_UNS.  */
  RELOC_NUMBER (R_RL78_DIR16S,       0x05) /* Was: R_RL78_16.  */
  RELOC_NUMBER (R_RL78_DIR8,         0x06)
  RELOC_NUMBER (R_RL78_DIR8U,        0x07) /* Was: R_RL78_8_UNS.  */
  RELOC_NUMBER (R_RL78_DIR8S,        0x08) /* Was: R_RL78_8.  */

  /* Signed pc-relative values.  */
  RELOC_NUMBER (R_RL78_DIR24S_PCREL, 0x09) /* Was: R_RL78_24_PCREL.  */
  RELOC_NUMBER (R_RL78_DIR16S_PCREL, 0x0a) /* Was: R_RL78_16_PCREL.  */
  RELOC_NUMBER (R_RL78_DIR8S_PCREL,  0x0b) /* Was: R_RL78_8_PCREL.  */

  /* These are for fields in the instructions.  */
  RELOC_NUMBER (R_RL78_DIR16UL,      0x0c)
  RELOC_NUMBER (R_RL78_DIR16UW,      0x0d)
  RELOC_NUMBER (R_RL78_DIR8UL,       0x0e)
  RELOC_NUMBER (R_RL78_DIR8UW,       0x0f)
  RELOC_NUMBER (R_RL78_DIR32_REV,    0x10)
  RELOC_NUMBER (R_RL78_DIR16_REV,    0x11)
  RELOC_NUMBER (R_RL78_DIR3U_PCREL,  0x12)

  /* These are extensions added by Red Hat.  */
  RELOC_NUMBER (R_RL78_RH_RELAX,     0x2d) /* Marks opcodes suitable for linker relaxation.  */
  RELOC_NUMBER (R_RL78_RH_SFR,       0x2e) /* SFR addresses - internal use only.  */
  RELOC_NUMBER (R_RL78_RH_SADDR,     0x2f) /* SADDR addresses - internal use only..  */

  /* These are for complex relocs.  */
  RELOC_NUMBER (R_RL78_ABS32,        0x41)
  RELOC_NUMBER (R_RL78_ABS24S,       0x42)
  RELOC_NUMBER (R_RL78_ABS16,        0x43)
  RELOC_NUMBER (R_RL78_ABS16U,       0x44)
  RELOC_NUMBER (R_RL78_ABS16S,       0x45)
  RELOC_NUMBER (R_RL78_ABS8,         0x46)
  RELOC_NUMBER (R_RL78_ABS8U,        0x47)
  RELOC_NUMBER (R_RL78_ABS8S,        0x48)
  RELOC_NUMBER (R_RL78_ABS24S_PCREL, 0x49)
  RELOC_NUMBER (R_RL78_ABS16S_PCREL, 0x4a)
  RELOC_NUMBER (R_RL78_ABS8S_PCREL,  0x4b)
  RELOC_NUMBER (R_RL78_ABS16UL,      0x4c)
  RELOC_NUMBER (R_RL78_ABS16UW,      0x4d)
  RELOC_NUMBER (R_RL78_ABS8UL,       0x4e)
  RELOC_NUMBER (R_RL78_ABS8UW,       0x4f)
  RELOC_NUMBER (R_RL78_ABS32_REV,    0x50)
  RELOC_NUMBER (R_RL78_ABS16_REV,    0x51)

  RELOC_NUMBER (R_RL78_SYM,          0x80)
  RELOC_NUMBER (R_RL78_OPneg,        0x81)
  RELOC_NUMBER (R_RL78_OPadd,        0x82)
  RELOC_NUMBER (R_RL78_OPsub,        0x83)
  RELOC_NUMBER (R_RL78_OPmul,        0x84)
  RELOC_NUMBER (R_RL78_OPdiv,        0x85)
  RELOC_NUMBER (R_RL78_OPshla,       0x86)
  RELOC_NUMBER (R_RL78_OPshra,       0x87)
  RELOC_NUMBER (R_RL78_OPsctsize,    0x88)
  RELOC_NUMBER (R_RL78_OPscttop,     0x8d)
  RELOC_NUMBER (R_RL78_OPand,        0x90)
  RELOC_NUMBER (R_RL78_OPor,         0x91)
  RELOC_NUMBER (R_RL78_OPxor,        0x92)
  RELOC_NUMBER (R_RL78_OPnot,        0x93)
  RELOC_NUMBER (R_RL78_OPmod,        0x94)
  RELOC_NUMBER (R_RL78_OPromtop,     0x95)
  RELOC_NUMBER (R_RL78_OPramtop,     0x96)

END_RELOC_NUMBERS (R_RL78_max)

#define EF_RL78_CPU_RL78	0x00000079      /* FIXME: correct value?  */
#define EF_RL78_CPU_MASK	0x0000007F	/* specific cpu bits.  */
#define EF_RL78_ALL_FLAGS	(EF_RL78_CPU_MASK)

/* Values for the e_flags field in the ELF header.  */
#define E_FLAG_RL78_64BIT_DOUBLES	(1 << 0)
#define E_FLAG_RL78_DSP			(1 << 1) /* Defined in the RL78 CPU Object file specification, but not explained.  */
#define E_FLAG_RL78_CPU_MASK            0x0c
#define E_FLAG_RL78_ANY_CPU             0x00     /* CPU not specified.  Assume no CPU specific code usage.  */
#define E_FLAG_RL78_G10			0x04     /* CPU is missing register banks 1-3, so uses different ABI.  */
#define E_FLAG_RL78_G13			0x08     /* CPU uses a peripheral for multiply/divide.  */
#define E_FLAG_RL78_G14			0x0c     /* CPU has multiply/divide instructions.  */

/* These define the addend field of R_RL78_RH_RELAX relocations.  */
#define RL78_RELAXA_MASK	0x000000f0	/* Mask for relax types */
#define	RL78_RELAXA_BRA		0x00000010	/* Any type of branch (must be decoded).  */
#define	RL78_RELAXA_ADDR16	0x00000020	/* addr16->sfr/saddr opportunity  */
#define RL78_RELAXA_RNUM	0x0000000f	/* Number of associated relocations.  */
/* These mark the place where alignment is requested, and the place where the filler bytes end.  */
#define	RL78_RELAXA_ALIGN	0x10000000	/* Start alignment; the remaining bits are the alignment value.  */
#define	RL78_RELAXA_ELIGN	0x20000000	/* End alignment; the remaining bits are the alignment value.  */
#define	RL78_RELAXA_ANUM	0x00ffffff	/* Alignment amount, in bytes (i.e. .balign).  */

#endif /* _ELF_RL78_H */
