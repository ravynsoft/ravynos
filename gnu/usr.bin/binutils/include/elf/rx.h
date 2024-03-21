/* RX ELF support for BFD.
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

#ifndef _ELF_RX_H
#define _ELF_RX_H

#include "elf/reloc-macros.h"

/* Note that there are a few internal relocation types used by the
   linker to do link-time relaxation.  If you update this file, please
   check elf32-rx.c to see if any of the internal relocations need to
   be, er, relocated.  */

/* Preliminary relocations.  */
START_RELOC_NUMBERS (elf_rx_reloc_type)

  RELOC_NUMBER (R_RX_NONE,         0x00)
  /* These are for data, and are bi-endian.  */
  RELOC_NUMBER (R_RX_DIR32,        0x01) /* Was: R_RX_32.  */
  RELOC_NUMBER (R_RX_DIR24S,       0x02) /* Was: R_RX_24.  */
  RELOC_NUMBER (R_RX_DIR16,        0x03)
  RELOC_NUMBER (R_RX_DIR16U,       0x04) /* Was: R_RX_16_UNS.  */
  RELOC_NUMBER (R_RX_DIR16S,       0x05) /* Was: R_RX_16.  */
  RELOC_NUMBER (R_RX_DIR8,         0x06)
  RELOC_NUMBER (R_RX_DIR8U,        0x07) /* Was: R_RX_8_UNS.  */
  RELOC_NUMBER (R_RX_DIR8S,        0x08) /* Was: R_RX_8.  */

  /* Signed pc-relative values.  */
  RELOC_NUMBER (R_RX_DIR24S_PCREL, 0x09) /* Was: R_RX_24_PCREL.  */
  RELOC_NUMBER (R_RX_DIR16S_PCREL, 0x0a) /* Was: R_RX_16_PCREL.  */
  RELOC_NUMBER (R_RX_DIR8S_PCREL,  0x0b) /* Was: R_RX_8_PCREL.  */

  /* These are for fields in the instructions.  */
  RELOC_NUMBER (R_RX_DIR16UL,      0x0c)
  RELOC_NUMBER (R_RX_DIR16UW,      0x0d)
  RELOC_NUMBER (R_RX_DIR8UL,       0x0e)
  RELOC_NUMBER (R_RX_DIR8UW,       0x0f)
  RELOC_NUMBER (R_RX_DIR32_REV,    0x10)
  RELOC_NUMBER (R_RX_DIR16_REV,    0x11)
  RELOC_NUMBER (R_RX_DIR3U_PCREL,  0x12)

  /* These are extensions added by Red Hat.  */
  RELOC_NUMBER (R_RX_RH_3_PCREL,   0x20) /* Like R_RX_DIR8S_PCREL but only 3-bits.  */
  RELOC_NUMBER (R_RX_RH_16_OP,     0x21) /* Like R_RX_DIR16 but for opcodes - always big endian.  */
  RELOC_NUMBER (R_RX_RH_24_OP,     0x22) /* Like R_RX_DIR24S but for opcodes - always big endian.  */
  RELOC_NUMBER (R_RX_RH_32_OP,     0x23) /* Like R_RX_DIR32 but for opcodes - always big endian.  */
  RELOC_NUMBER (R_RX_RH_24_UNS,    0x24) /* Like R_RX_DIR24S but for unsigned values.  */
  RELOC_NUMBER (R_RX_RH_8_NEG,     0x25) /* Like R_RX_DIR8 but -x is stored.  */
  RELOC_NUMBER (R_RX_RH_16_NEG,    0x26) /* Like R_RX_DIR16 but -x is stored.  */
  RELOC_NUMBER (R_RX_RH_24_NEG,    0x27) /* Like R_RX_DIR24S but -x is stored.  */
  RELOC_NUMBER (R_RX_RH_32_NEG,    0x28) /* Like R_RX_DIR32 but -x is stored.  */
  RELOC_NUMBER (R_RX_RH_DIFF,      0x29) /* Subtract from a previous relocation.  */
  RELOC_NUMBER (R_RX_RH_GPRELB,    0x2a) /* Byte value, relative to __gp.  */
  RELOC_NUMBER (R_RX_RH_GPRELW,    0x2b) /* Word value, relative to __gp.  */
  RELOC_NUMBER (R_RX_RH_GPRELL,    0x2c) /* Long value, relative to __gp.  */
  RELOC_NUMBER (R_RX_RH_RELAX,     0x2d) /* Marks opcodes suitable for linker relaxation.  */

  /* These are for complex relocs.  */
  RELOC_NUMBER (R_RX_ABS32,        0x41)
  RELOC_NUMBER (R_RX_ABS24S,       0x42)
  RELOC_NUMBER (R_RX_ABS16,        0x43)
  RELOC_NUMBER (R_RX_ABS16U,       0x44)
  RELOC_NUMBER (R_RX_ABS16S,       0x45)
  RELOC_NUMBER (R_RX_ABS8,         0x46)
  RELOC_NUMBER (R_RX_ABS8U,        0x47)
  RELOC_NUMBER (R_RX_ABS8S,        0x48)
  RELOC_NUMBER (R_RX_ABS24S_PCREL, 0x49)
  RELOC_NUMBER (R_RX_ABS16S_PCREL, 0x4a)
  RELOC_NUMBER (R_RX_ABS8S_PCREL,  0x4b)
  RELOC_NUMBER (R_RX_ABS16UL,      0x4c)
  RELOC_NUMBER (R_RX_ABS16UW,      0x4d)
  RELOC_NUMBER (R_RX_ABS8UL,       0x4e)
  RELOC_NUMBER (R_RX_ABS8UW,       0x4f)
  RELOC_NUMBER (R_RX_ABS32_REV,    0x50)
  RELOC_NUMBER (R_RX_ABS16_REV,    0x51)

  RELOC_NUMBER (R_RX_SYM,          0x80)
  RELOC_NUMBER (R_RX_OPneg,        0x81)
  RELOC_NUMBER (R_RX_OPadd,        0x82)
  RELOC_NUMBER (R_RX_OPsub,        0x83)
  RELOC_NUMBER (R_RX_OPmul,        0x84)
  RELOC_NUMBER (R_RX_OPdiv,        0x85)
  RELOC_NUMBER (R_RX_OPshla,       0x86)
  RELOC_NUMBER (R_RX_OPshra,       0x87)
  RELOC_NUMBER (R_RX_OPsctsize,    0x88)
  RELOC_NUMBER (R_RX_OPscttop,     0x8d)
  RELOC_NUMBER (R_RX_OPand,        0x90)
  RELOC_NUMBER (R_RX_OPor,         0x91)
  RELOC_NUMBER (R_RX_OPxor,        0x92)
  RELOC_NUMBER (R_RX_OPnot,        0x93)
  RELOC_NUMBER (R_RX_OPmod,        0x94)
  RELOC_NUMBER (R_RX_OPromtop,     0x95)
  RELOC_NUMBER (R_RX_OPramtop,     0x96)

END_RELOC_NUMBERS (R_RX_max)

#define EF_RX_CPU_RX	0x00000079      /* FIXME: this collides with the E_FLAG_RX_... values below.  */
#define EF_RX_CPU_MASK	0x000003FF	/* specific cpu bits.  */
#define EF_RX_ALL_FLAGS	(EF_RX_CPU_MASK)

/* Values for the e_flags field in the ELF header.  */
#define E_FLAG_RX_64BIT_DOUBLES		(1 << 0)
#define E_FLAG_RX_DSP			(1 << 1) /* Defined in the RX CPU Object file specification, but not explained. */
#define E_FLAG_RX_PID			(1 << 2) /* Unofficial - DJ */
#define E_FLAG_RX_ABI			(1 << 3) /* Binary passes stacked arguments using natural alignment.  Unofficial - NC.  */
#define E_FLAG_RX_SINSNS_SET		(1 << 6) /* Set if bit-5 is significant.  */
#define E_FLAG_RX_SINSNS_YES		(1 << 7) /* Set if string instructions are used in the binary.  */
#define E_FLAG_RX_SINSNS_NO		0        /* Bit-5 if this binary must not be linked with a string instruction using binary.  */
#define E_FLAG_RX_SINSNS_MASK		(3 << 6) /* Mask of bits used to determine string instruction use.  */
#define E_FLAG_RX_V2			(1 << 8) /* RX v2 instructions */
#define E_FLAG_RX_V3			(1 << 9) /* RX v3 instructions */

/* These define the addend field of R_RX_RH_RELAX relocations.  */
#define	RX_RELAXA_IMM6	0x00000010	/* Imm8/16/24/32 at bit offset 6.  */
#define	RX_RELAXA_IMM12	0x00000020	/* Imm8/16/24/32 at bit offset 12.  */
#define	RX_RELAXA_DSP4	0x00000040	/* Dsp0/8/16 at bit offset 4.  */
#define	RX_RELAXA_DSP6	0x00000080	/* Dsp0/8/16 at bit offset 6.  */
#define	RX_RELAXA_DSP14	0x00000100	/* Dsp0/8/16 at bit offset 14.  */
#define	RX_RELAXA_BRA	0x00000200	/* Any type of branch (must be decoded).  */
#define RX_RELAXA_RNUM	0x0000000f	/* Number of associated relocations.  */
/* These mark the place where alignment is requested, and the place where the filler bytes end.  */
#define	RX_RELAXA_ALIGN	0x10000000	/* Start alignment; the remaining bits are the alignment value.  */
#define	RX_RELAXA_ELIGN	0x20000000	/* End alignment; the remaining bits are the alignment value.  */
#define	RX_RELAXA_ANUM	0x00ffffff	/* Alignment amount, in bytes (i.e. .balign).  */

#endif /* _ELF_RX_H */
