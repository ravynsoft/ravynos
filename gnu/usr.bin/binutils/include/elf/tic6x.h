/* TI C6X ELF support for BFD.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

#ifndef _ELF_TIC6X_H
#define _ELF_TIC6X_H

#include "elf/reloc-macros.h"

/* Relocation types.  */
START_RELOC_NUMBERS (elf_tic6x_reloc_type)
  RELOC_NUMBER (R_C6000_NONE, 0)
  RELOC_NUMBER (R_C6000_ABS32, 1)
  RELOC_NUMBER (R_C6000_ABS16, 2)
  RELOC_NUMBER (R_C6000_ABS8, 3)
  RELOC_NUMBER (R_C6000_PCR_S21, 4)
  RELOC_NUMBER (R_C6000_PCR_S12, 5)
  RELOC_NUMBER (R_C6000_PCR_S10, 6)
  RELOC_NUMBER (R_C6000_PCR_S7, 7)
  RELOC_NUMBER (R_C6000_ABS_S16, 8)
  RELOC_NUMBER (R_C6000_ABS_L16, 9)
  RELOC_NUMBER (R_C6000_ABS_H16, 10)
  RELOC_NUMBER (R_C6000_SBR_U15_B, 11)
  RELOC_NUMBER (R_C6000_SBR_U15_H, 12)
  RELOC_NUMBER (R_C6000_SBR_U15_W, 13)
  RELOC_NUMBER (R_C6000_SBR_S16, 14)
  RELOC_NUMBER (R_C6000_SBR_L16_B, 15)
  RELOC_NUMBER (R_C6000_SBR_L16_H, 16)
  RELOC_NUMBER (R_C6000_SBR_L16_W, 17)
  RELOC_NUMBER (R_C6000_SBR_H16_B, 18)
  RELOC_NUMBER (R_C6000_SBR_H16_H, 19)
  RELOC_NUMBER (R_C6000_SBR_H16_W, 20)
  RELOC_NUMBER (R_C6000_SBR_GOT_U15_W, 21)
  RELOC_NUMBER (R_C6000_SBR_GOT_L16_W, 22)
  RELOC_NUMBER (R_C6000_SBR_GOT_H16_W, 23)
  RELOC_NUMBER (R_C6000_DSBT_INDEX, 24)
  RELOC_NUMBER (R_C6000_PREL31, 25)
  RELOC_NUMBER (R_C6000_COPY, 26)
  RELOC_NUMBER (R_C6000_JUMP_SLOT, 27)
  RELOC_NUMBER (R_C6000_EHTYPE, 28)
  RELOC_NUMBER (R_C6000_PCR_H16, 29)
  RELOC_NUMBER (R_C6000_PCR_L16, 30)
  RELOC_NUMBER (R_C6000_ALIGN, 253)
  RELOC_NUMBER (R_C6000_FPHEAD, 254)
  RELOC_NUMBER (R_C6000_NOCMP, 255)
END_RELOC_NUMBERS (R_TIC6X_max)

/* Processor-specific flags.  */

/* File contains static relocation information.  */
#define EF_C6000_REL		0x1

/* Processor-specific section types.  */

/* Unwind function table for stack unwinding.  */
#define SHT_C6000_UNWIND	0x70000001

/* DLL dynamic linking pre-emption map.  */
#define SHT_C6000_PREEMPTMAP	0x70000002

/* Object file compatibility attributes.  */
#define SHT_C6000_ATTRIBUTES	0x70000003

/* Intermediate code for link-time optimization.  */
#define SHT_TI_ICODE		0x7F000000

/* Symbolic cross reference information.  */
#define SHT_TI_XREF		0x7F000001

/* Reserved.  */
#define SHT_TI_HANDLER		0x7F000002

/* Compressed data for initializing C variables.  */
#define SHT_TI_INITINFO		0x7F000003

/* Extended program header attributes.  */
#define SHT_TI_PHATTRS		0x7F000004

/* Processor specific section indices.  These sections do not actually
   exist.  Symbols with a st_shndx field corresponding to one of these
   values have a special meaning.  */

/* Small data area common symbol.  */
#define SHN_TIC6X_SCOMMON	SHN_LORESERVE

/* Processor-specific segment types.  */

/* Extended Segment Attributes.  */
#define PT_C6000_PHATTR		0x70000000

/* Processor-specific dynamic tags.  */

/* Undocumented.  */
#define DT_C6000_GSYM_OFFSET	0x6000000D

/* Undocumented.  */
#define DT_C6000_GSTR_OFFSET	0x6000000F

/* Statically linked base address of data segment.  */
#define DT_C6000_DSBT_BASE	0x70000000

/* Number of entries in this module's DSBT.  */
#define DT_C6000_DSBT_SIZE	0x70000001

/* Undocumented.  */
#define DT_C6000_PREEMPTMAP	0x70000002

/* The hard-coded DSBT index for this module, if any.  */
#define DT_C6000_DSBT_INDEX	0x70000003

/* Extended program header attributes.  */

/* Terminate a segment.  */
#define PHA_NULL		0x0

/* Segment's address bound to the final address.  */
#define PHA_BOUND		0x1

/* Segment cannot be further relocated.  */
#define PHA_READONLY		0x2

/* Build attributes.  */
enum
  {
#define TAG(tag, value) tag = value,
#include "elf/tic6x-attrs.h"
#undef TAG
    Tag_C6XABI_last
  };

/* Values for Tag_ISA.  GNU-specific names; the ABI does not specify
   names for these values.  */
enum
  {
    C6XABI_Tag_ISA_none = 0,
    C6XABI_Tag_ISA_C62X = 1,
    C6XABI_Tag_ISA_C67X = 3,
    C6XABI_Tag_ISA_C67XP = 4,
    C6XABI_Tag_ISA_C64X = 6,
    C6XABI_Tag_ISA_C64XP = 7,
    C6XABI_Tag_ISA_C674X = 8
  };

/* Special section names.  */
#define ELF_STRING_C6000_unwind           ".c6xabi.exidx"
#define ELF_STRING_C6000_unwind_info      ".c6xabi.extab"
#define ELF_STRING_C6000_unwind_once      ".gnu.linkonce.c6xabi.exidx."
#define ELF_STRING_C6000_unwind_info_once ".gnu.linkonce.c6xabi.extab."

#endif /* _ELF_TIC6X_H */
