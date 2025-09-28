/* Toshiba MeP ELF support for BFD.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.

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

#ifndef _ELF_MEP_H
#define _ELF_MEP_H

/* Bits in the sh_flags field of Elf32_Shdr:  */

#define SHF_MEP_VLIW		0x10000000	/* contains vliw code */

#include "elf/reloc-macros.h"

/* Note: The comments in this file are used by bfd/mep-relocs.pl to
   build parts of bfd/elf32-mep.c.  */

/* Relocations.  */
START_RELOC_NUMBERS (elf_mep_reloc_type)

  /* These two must appear first so that they are not processed by bfd/mep-relocs.pl.  */
  RELOC_NUMBER (R_MEP_NONE, 0)
  RELOC_NUMBER (R_RELC, 1)

  RELOC_NUMBER (R_MEP_8, 2)		/* 7654 3210                               U */
  RELOC_NUMBER (R_MEP_16, 3)		/* fedc ba98 7654 3210                     U */
  RELOC_NUMBER (R_MEP_32, 4)		/* vuts rqpo nmlk jihg fedc ba98 7654 3210 U */

  RELOC_NUMBER (R_MEP_PCREL8A2, 5)	/* ---- ---- 7654 321-                     S PC-REL */
  RELOC_NUMBER (R_MEP_PCREL12A2, 6)	/* ---- ba98 7654 321-                     S PC-REL */
  RELOC_NUMBER (R_MEP_PCREL17A2, 7)	/* ---- ---- ---- ---- gfed cba9 8765 4321 S PC-REL */
  RELOC_NUMBER (R_MEP_PCREL24A2, 8)	/* ---- -765 4321 ---- nmlk jihg fedc ba98 S PC-REL */
  RELOC_NUMBER (R_MEP_PCABS24A2, 9)	/* ---- -765 4321 ---- nmlk jihg fedc ba98 U */

  RELOC_NUMBER (R_MEP_LOW16, 10)	/* ---- ---- ---- ---- fedc ba98 7654 3210 U no-overflow */
  RELOC_NUMBER (R_MEP_HI16U, 11)	/* ---- ---- ---- ---- vuts rqpo nmlk jihg U no-overflow */
  RELOC_NUMBER (R_MEP_HI16S, 12)	/* ---- ---- ---- ---- vuts rqpo nmlk jihg S no-overflow */
  RELOC_NUMBER (R_MEP_GPREL, 13)	/* ---- ---- ---- ---- fedc ba98 7654 3210 S GP-REL*/
  RELOC_NUMBER (R_MEP_TPREL, 14)	/* ---- ---- ---- ---- fedc ba98 7654 3210 S TP-REL*/

  RELOC_NUMBER (R_MEP_TPREL7, 15)	/* ---- ---- -654 3210                     U TP-REL */
  RELOC_NUMBER (R_MEP_TPREL7A2, 16)	/* ---- ---- -654 321-                     U TP-REL */
  RELOC_NUMBER (R_MEP_TPREL7A4, 17)	/* ---- ---- -654 32--                     U TP-REL */

  RELOC_NUMBER (R_MEP_UIMM24, 18)	/* ---- ---- 7654 3210 nmlk jihg fedc ba98 U */
  RELOC_NUMBER (R_MEP_ADDR24A4, 19)	/* ---- ---- 7654 32-- nmlk jihg fedc ba98 U */

  RELOC_NUMBER (R_MEP_GNU_VTINHERIT, 20) /* ---- ---- ---- ----                     U no-overflow */
  RELOC_NUMBER (R_MEP_GNU_VTENTRY, 21)   /* ---- ---- ---- ----                     U no-overflow */

END_RELOC_NUMBERS(R_MEP_max)

#define	EF_MEP_CPU_MASK		0xff000000	/* specific cpu bits */
#define EF_MEP_CPU_MEP          0x00000000	/* generic MEP */
#define EF_MEP_CPU_C2   	0x01000000	/* MEP c2 */
#define EF_MEP_CPU_C3   	0x02000000	/* MEP c3 */
#define EF_MEP_CPU_C4   	0x04000000	/* MEP c4 */
/* 5..7 are reseved */
#define EF_MEP_CPU_C5   	0x08000000	/* MEP c5 */
#define EF_MEP_CPU_H1   	0x10000000	/* MEP h1 */

#define EF_MEP_COP_MASK		0x00ff0000
#define EF_MEP_COP_NONE		0x00000000
#define EF_MEP_COP_AVC		0x00010000
#define EF_MEP_COP_AVC2		0x00020000
#define EF_MEP_COP_FMAX		0x00030000
/* 4..5 are reserved.  */
#define EF_MEP_COP_IVC2		0x00060000

#define EF_MEP_LIBRARY		0x00000100	/* Built as a library */

#define EF_MEP_INDEX_MASK       0x000000ff      /* Configuration index */

#define EF_MEP_ALL_FLAGS	0xffff01ff

#endif /* _ELF_MEP_H */
