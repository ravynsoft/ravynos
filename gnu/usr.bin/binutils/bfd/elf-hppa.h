/* Common code for PA ELF implementations.
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

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

#define ELF_HOWTO_TABLE_SIZE       R_PARISC_UNIMPLEMENTED + 1

/* This file is included by multiple PA ELF BFD backends with different
   sizes.

   Most of the routines are written to be size independent, but sometimes
   external constraints require 32 or 64 bit specific code.  We remap
   the definitions/functions as necessary here.  */
#if ARCH_SIZE == 64
#define ELF_R_TYPE(X)		      ELF64_R_TYPE(X)
#define ELF_R_SYM(X)		      ELF64_R_SYM(X)
#define elf_hppa_reloc_final_type     elf64_hppa_reloc_final_type
#define _bfd_elf_hppa_gen_reloc_type  _bfd_elf64_hppa_gen_reloc_type
#define elf_hppa_relocate_section     elf64_hppa_relocate_section
#define elf_hppa_final_link	      elf64_hppa_final_link
#endif
#if ARCH_SIZE == 32
#define ELF_R_TYPE(X)		      ELF32_R_TYPE(X)
#define ELF_R_SYM(X)		      ELF32_R_SYM(X)
#define elf_hppa_reloc_final_type     elf32_hppa_reloc_final_type
#define _bfd_elf_hppa_gen_reloc_type  _bfd_elf32_hppa_gen_reloc_type
#define elf_hppa_relocate_section     elf32_hppa_relocate_section
#define elf_hppa_final_link	      elf32_hppa_final_link
#endif

/* ELF/PA relocation howto entries.  */

static reloc_howto_type elf_hppa_howto_table[ELF_HOWTO_TABLE_SIZE] =
{
#define HOW(type, size, bitsize, pc_rel, complain, mask) \
  HOWTO (type, 0, size, bitsize, pc_rel, 0, complain_overflow_ ## complain, \
	 bfd_elf_generic_reloc, #type, false, 0, mask, false)

  /* The values in DIR32 are to placate the check in
     _bfd_stab_section_find_nearest_line.  */
  HOW (R_PARISC_NONE,		0,  0, false,     dont, 0),
  HOW (R_PARISC_DIR32,		4, 32, false, bitfield, 0xffffffff),
  HOW (R_PARISC_DIR21L,		4, 21, false, bitfield, 0),
  HOW (R_PARISC_DIR17R,		4, 17, false, bitfield, 0),
  HOW (R_PARISC_DIR17F,		4, 17, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DIR14R,		4, 14, false, bitfield, 0),
  HOW (R_PARISC_DIR14F,		4, 14, false, bitfield, 0),
  /* 8 */
  HOW (R_PARISC_PCREL12F,	4, 12,  true, bitfield, 0),
  HOW (R_PARISC_PCREL32,	4, 32,  true, bitfield, 0),
  HOW (R_PARISC_PCREL21L,	4, 21,  true, bitfield, 0),
  HOW (R_PARISC_PCREL17R,	4, 17,  true, bitfield, 0),
  HOW (R_PARISC_PCREL17F,	4, 17,  true, bitfield, 0),
  HOW (R_PARISC_PCREL17C,	4, 17,  true, bitfield, 0),
  HOW (R_PARISC_PCREL14R,	4, 14,  true, bitfield, 0),
  HOW (R_PARISC_PCREL14F,	4, 14,  true, bitfield, 0),
  /* 16 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DPREL21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_DPREL14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_DPREL14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DPREL14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_DPREL14F,	4, 14, false, bitfield, 0),
  /* 24 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DLTREL21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DLTREL14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_DLTREL14F,	4, 14, false, bitfield, 0),
  /* 32 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DLTIND21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DLTIND14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_DLTIND14F,	4, 14, false, bitfield, 0),
  /* 40 */
  HOW (R_PARISC_SETBASE,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_SECREL32,	4, 32, false, bitfield, 0xffffffff),
  HOW (R_PARISC_BASEREL21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_BASEREL17R,	4, 17, false, bitfield, 0),
  HOW (R_PARISC_BASEREL17F,	4, 17, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_BASEREL14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_BASEREL14F,	4, 14, false, bitfield, 0),
  /* 48 */
  HOW (R_PARISC_SEGBASE,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_SEGREL32,	4, 32, false, bitfield, 0),
  HOW (R_PARISC_PLTOFF21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_PLTOFF14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_PLTOFF14F,	4, 14, false, bitfield, 0),
  /* 56 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_FPTR32,	4, 32, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_FPTR21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_FPTR14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 64 */
  HOW (R_PARISC_FPTR64,		8, 64, false, bitfield, 0),
  HOW (R_PARISC_PLABEL32,	4, 32, false, bitfield, 0),
  HOW (R_PARISC_PLABEL21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_PLABEL14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 72 */
  HOW (R_PARISC_PCREL64,	8, 64, false, bitfield, 0),
  HOW (R_PARISC_PCREL22C,	4, 22, false, bitfield, 0),
  HOW (R_PARISC_PCREL22F,	4, 22, false, bitfield, 0),
  HOW (R_PARISC_PCREL14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_PCREL14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_PCREL16F,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_PCREL16WF,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_PCREL16DF,	4, 16, false, bitfield, 0),
  /* 80 */
  HOW (R_PARISC_DIR64,		8, 64, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DIR14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_DIR14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_DIR16F,		4, 16, false, bitfield, 0),
  HOW (R_PARISC_DIR16WF,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_DIR16DF,	4, 16, false, bitfield, 0),
  /* 88 */
  HOW (R_PARISC_GPREL64,	8, 64, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DLTREL14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_DLTREL14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_GPREL16F,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_GPREL16WF,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_GPREL16DF,	4, 16, false, bitfield, 0),
  /* 96 */
  HOW (R_PARISC_LTOFF64,	8, 64, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_DLTIND14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_DLTIND14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_LTOFF16F,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_LTOFF16WF,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_LTOFF16DF,	4, 16, false, bitfield, 0),
  /* 104 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_BASEREL14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_BASEREL14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 112 */
  HOW (R_PARISC_SEGREL64,	8, 64, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_PLTOFF14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_PLTOFF14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_PLTOFF16F,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_PLTOFF16WF,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_PLTOFF16DF,	4, 16, false, bitfield, 0),
  /* 120 */
  HOW (R_PARISC_LTOFF_FPTR64,	8, 64, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_FPTR14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_FPTR14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_FPTR16F,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_FPTR16WF,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_FPTR16DF,	4, 16, false, bitfield, 0),
  /* 128 */
  HOW (R_PARISC_COPY,		0,  0, false, bitfield, 0),
  HOW (R_PARISC_IPLT,		0,  0, false, bitfield, 0),
  HOW (R_PARISC_EPLT,		0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 136 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 144 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  /* 152 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_TPREL32,	4, 32, false,     dont, 0),
  HOW (R_PARISC_TPREL21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_TPREL14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  /* 160 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_LTOFF_TP21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_TP14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_TP14F,	4, 14, false, bitfield, 0),
  /* 168 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 176 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 184 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  /* 192 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 200 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 208 */
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false,     dont, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  /* 216 */
  HOW (R_PARISC_TPREL64,	8, 64, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_TPREL14WR,	4, 14, false,     dont, 0),
  HOW (R_PARISC_TPREL14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_TPREL16F,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_TPREL16WF,	4, 16, false,     dont, 0),
  HOW (R_PARISC_TPREL16DF,	4, 16, false, bitfield, 0),
  /* 224 */
  HOW (R_PARISC_LTOFF_TP64,	8, 64, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_UNIMPLEMENTED,	0,  0, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_TP14WR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_TP14DR,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_TP16F,	4, 16, false,     dont, 0),
  HOW (R_PARISC_LTOFF_TP16WF,	4, 16, false, bitfield, 0),
  HOW (R_PARISC_LTOFF_TP16DF,	4, 16, false, bitfield, 0),
  /* 232 */
  HOW (R_PARISC_GNU_VTENTRY,	0,  0, false,     dont, 0),
  HOW (R_PARISC_GNU_VTINHERIT,	0,  0, false,     dont, 0),
  HOW (R_PARISC_TLS_GD21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_TLS_GD14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_TLS_GDCALL,	0,  0, false,     dont, 0),
  HOW (R_PARISC_TLS_LDM21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_TLS_LDM14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_TLS_LDMCALL,	0,  0, false,     dont, 0),
  /* 240 */
  HOW (R_PARISC_TLS_LDO21L,	4, 21, false, bitfield, 0),
  HOW (R_PARISC_TLS_LDO14R,	4, 14, false, bitfield, 0),
  HOW (R_PARISC_TLS_DTPMOD32,	4, 32, false, bitfield, 0),
  HOW (R_PARISC_TLS_DTPMOD64,	8, 64, false, bitfield, 0),
  HOW (R_PARISC_TLS_DTPOFF32,	4, 32, false, bitfield, 0),
  HOW (R_PARISC_TLS_DTPOFF64,	8, 64, false, bitfield, 0)
#undef HOW
};

#define OFFSET_14R_FROM_21L 4
#define OFFSET_14F_FROM_21L 5

/* Return the final relocation type for the given base type, instruction
   format, and field selector.  */

elf_hppa_reloc_type
elf_hppa_reloc_final_type (bfd *abfd,
			   elf_hppa_reloc_type base_type,
			   int format,
			   unsigned int field)
{
  elf_hppa_reloc_type final_type = base_type;

  /* Just a tangle of nested switch statements to deal with the braindamage
     that a different field selector means a completely different relocation
     for PA ELF.  */
  switch (base_type)
    {
      /* We have been using generic relocation types.  However, that may not
	 really make sense.  Anyway, we need to support both R_PARISC_DIR64
	 and R_PARISC_DIR32 here.  */
    case R_PARISC_DIR32:
    case R_PARISC_DIR64:
    case R_HPPA_ABS_CALL:
      switch (format)
	{
	case 14:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_DIR14F;
	      break;
	    case e_rsel:
	    case e_rrsel:
	    case e_rdsel:
	      final_type = R_PARISC_DIR14R;
	      break;
	    case e_rtsel:
	      final_type = R_PARISC_DLTIND14R;
	      break;
	    case e_rtpsel:
	      final_type = R_PARISC_LTOFF_FPTR14DR;
	      break;
	    case e_tsel:
	      final_type = R_PARISC_DLTIND14F;
	      break;
	    case e_rpsel:
	      final_type = R_PARISC_PLABEL14R;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 17:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_DIR17F;
	      break;
	    case e_rsel:
	    case e_rrsel:
	    case e_rdsel:
	      final_type = R_PARISC_DIR17R;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 21:
	  switch (field)
	    {
	    case e_lsel:
	    case e_lrsel:
	    case e_ldsel:
	    case e_nlsel:
	    case e_nlrsel:
	      final_type = R_PARISC_DIR21L;
	      break;
	    case e_ltsel:
	      final_type = R_PARISC_DLTIND21L;
	      break;
	    case e_ltpsel:
	      final_type = R_PARISC_LTOFF_FPTR21L;
	      break;
	    case e_lpsel:
	      final_type = R_PARISC_PLABEL21L;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 32:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_DIR32;
	      /* When in 64bit mode, a 32bit relocation is supposed to
		 be a section relative relocation.  Dwarf2 (for example)
		 uses 32bit section relative relocations.  */
	      if (bfd_arch_bits_per_address (abfd) != 32)
		final_type = R_PARISC_SECREL32;
	      break;
	    case e_psel:
	      final_type = R_PARISC_PLABEL32;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 64:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_DIR64;
	      break;
	    case e_psel:
	      final_type = R_PARISC_FPTR64;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	default:
	  return R_PARISC_NONE;
	}
      break;

    case R_HPPA_GOTOFF:
      switch (format)
	{
	case 14:
	  switch (field)
	    {
	    case e_rsel:
	    case e_rrsel:
	    case e_rdsel:
	      /* R_PARISC_DLTREL14R for elf64, R_PARISC_DPREL14R for elf32.  */
	      final_type = base_type + OFFSET_14R_FROM_21L;
	      break;
	    case e_fsel:
	      /* R_PARISC_DLTREL14F for elf64, R_PARISC_DPREL14F for elf32.  */
	      final_type = base_type + OFFSET_14F_FROM_21L;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 21:
	  switch (field)
	    {
	    case e_lsel:
	    case e_lrsel:
	    case e_ldsel:
	    case e_nlsel:
	    case e_nlrsel:
	      /* R_PARISC_DLTREL21L for elf64, R_PARISC_DPREL21L for elf32.  */
	      final_type = base_type;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 64:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_GPREL64;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	default:
	  return R_PARISC_NONE;
	}
      break;

    case R_HPPA_PCREL_CALL:
      switch (format)
	{
	case 12:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_PCREL12F;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 14:
	  /* Contrary to appearances, these are not calls of any sort.
	     Rather, they are loads/stores with a pcrel reloc.  */
	  switch (field)
	    {
	    case e_rsel:
	    case e_rrsel:
	    case e_rdsel:
	      final_type = R_PARISC_PCREL14R;
	      break;
	    case e_fsel:
	      if (bfd_get_mach (abfd) < 25)
		final_type = R_PARISC_PCREL14F;
	      else
		final_type = R_PARISC_PCREL16F;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 17:
	  switch (field)
	    {
	    case e_rsel:
	    case e_rrsel:
	    case e_rdsel:
	      final_type = R_PARISC_PCREL17R;
	      break;
	    case e_fsel:
	      final_type = R_PARISC_PCREL17F;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 21:
	  switch (field)
	    {
	    case e_lsel:
	    case e_lrsel:
	    case e_ldsel:
	    case e_nlsel:
	    case e_nlrsel:
	      final_type = R_PARISC_PCREL21L;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 22:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_PCREL22F;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 32:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_PCREL32;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 64:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_PCREL64;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	default:
	  return R_PARISC_NONE;
	}
      break;

    case R_PARISC_TLS_GD21L:
      switch (field)
	{
	  case e_ltsel:
	  case e_lrsel:
	    final_type = R_PARISC_TLS_GD21L;
	    break;
	  case e_rtsel:
	  case e_rrsel:
	    final_type = R_PARISC_TLS_GD14R;
	    break;
	  default:
	    return R_PARISC_NONE;
	}
      break;

    case R_PARISC_TLS_LDM21L:
      switch (field)
	{
	  case e_ltsel:
	  case e_lrsel:
	    final_type = R_PARISC_TLS_LDM21L;
	    break;
	  case e_rtsel:
	  case e_rrsel:
	    final_type = R_PARISC_TLS_LDM14R;
	    break;
	  default:
	    return R_PARISC_NONE;
	}
      break;

    case R_PARISC_TLS_LDO21L:
      switch (field)
	{
	  case e_lrsel:
	    final_type = R_PARISC_TLS_LDO21L;
	    break;
	  case e_rrsel:
	    final_type = R_PARISC_TLS_LDO14R;
	    break;
	  default:
	    return R_PARISC_NONE;
	}
      break;

    case R_PARISC_TLS_IE21L:
      switch (field)
	{
	  case e_ltsel:
	  case e_lrsel:
	    final_type = R_PARISC_TLS_IE21L;
	    break;
	  case e_rtsel:
	  case e_rrsel:
	    final_type = R_PARISC_TLS_IE14R;
	    break;
	  default:
	    return R_PARISC_NONE;
	}
      break;

    case R_PARISC_TLS_LE21L:
      switch (field)
	{
	  case e_lrsel:
	    final_type = R_PARISC_TLS_LE21L;
	    break;
	  case e_rrsel:
	    final_type = R_PARISC_TLS_LE14R;
	    break;
	  default:
	    return R_PARISC_NONE;
	}
      break;

    case R_PARISC_SEGREL32:
      switch (format)
	{
	case 32:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_SEGREL32;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	case 64:
	  switch (field)
	    {
	    case e_fsel:
	      final_type = R_PARISC_SEGREL64;
	      break;
	    default:
	      return R_PARISC_NONE;
	    }
	  break;

	default:
	  return R_PARISC_NONE;
	}
      break;

    case R_PARISC_GNU_VTENTRY:
    case R_PARISC_GNU_VTINHERIT:
    case R_PARISC_SEGBASE:
      /* The defaults are fine for these cases.  */
      break;

    default:
      return R_PARISC_NONE;
    }

  return final_type;
}

/* Return one (or more) BFD relocations which implement the base
   relocation with modifications based on format and field.  */

elf_hppa_reloc_type **
_bfd_elf_hppa_gen_reloc_type (bfd *abfd,
			      elf_hppa_reloc_type base_type,
			      int format,
			      unsigned int field,
			      int ignore ATTRIBUTE_UNUSED,
			      asymbol *sym ATTRIBUTE_UNUSED)
{
  elf_hppa_reloc_type *finaltype;
  elf_hppa_reloc_type **final_types;
  size_t amt = sizeof (elf_hppa_reloc_type *) * 2;

  /* Allocate slots for the BFD relocation.  */
  final_types = bfd_alloc (abfd, amt);
  if (final_types == NULL)
    return NULL;

  /* Allocate space for the relocation itself.  */
  amt = sizeof (elf_hppa_reloc_type);
  finaltype = bfd_alloc (abfd, amt);
  if (finaltype == NULL)
    return NULL;

  /* Some reasonable defaults.  */
  final_types[0] = finaltype;
  final_types[1] = NULL;

  *finaltype = elf_hppa_reloc_final_type (abfd, base_type, format, field);

  return final_types;
}

/* Translate from an elf into field into a howto relocation pointer.  */

static bool
elf_hppa_info_to_howto (bfd *abfd,
			arelent *bfd_reloc,
			Elf_Internal_Rela *elf_reloc)
{
  unsigned int r_type = ELF32_R_TYPE (elf_reloc->r_info);
  unsigned int type = r_type;
  reloc_howto_type *howto = NULL;

  if (r_type < (unsigned int) R_PARISC_UNIMPLEMENTED)
    {
      howto = &elf_hppa_howto_table[r_type];
      type = howto->type;
    }
  if (type >= (unsigned int) R_PARISC_UNIMPLEMENTED)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  bfd_reloc->howto = howto;
  return true;
}

/* Translate from an elf into field into a howto relocation pointer.  */

static bool
elf_hppa_info_to_howto_rel (bfd *abfd,
			    arelent *bfd_reloc,
			    Elf_Internal_Rela *elf_reloc)
{
  unsigned int r_type = ELF_R_TYPE (elf_reloc->r_info);
  unsigned int type = r_type;
  reloc_howto_type *howto = NULL;

  if (r_type < (unsigned int) R_PARISC_UNIMPLEMENTED)
    {
      howto = &elf_hppa_howto_table[r_type];
      type = howto->type;
    }
  if (type >= (unsigned int) R_PARISC_UNIMPLEMENTED)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  bfd_reloc->howto = howto;
  return true;
}

/* Return the address of the howto table entry to perform the CODE
   relocation for an ARCH machine.  */

static reloc_howto_type *
elf_hppa_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			    bfd_reloc_code_real_type code)
{
  if ((int) code < (int) R_PARISC_UNIMPLEMENTED)
    {
      BFD_ASSERT ((int) elf_hppa_howto_table[(int) code].type == (int) code);
      return &elf_hppa_howto_table[(int) code];
    }
  return NULL;
}

static reloc_howto_type *
elf_hppa_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			    const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (elf_hppa_howto_table) / sizeof (elf_hppa_howto_table[0]);
       i++)
    if (elf_hppa_howto_table[i].name != NULL
	&& strcasecmp (elf_hppa_howto_table[i].name, r_name) == 0)
      return &elf_hppa_howto_table[i];

  return NULL;
}

/* Return TRUE if SYM represents a local label symbol.  */

static bool
elf_hppa_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED, const char *name)
{
  if (name[0] == 'L' && name[1] == '$')
    return true;
  return _bfd_elf_is_local_label_name (abfd, name);
}

/* Set the correct type for an ELF section.  We do this by the
   section name, which is a hack, but ought to work.  */

static bool
elf_hppa_fake_sections (bfd *abfd, Elf_Internal_Shdr *hdr, asection *sec)
{
  const char *name;

  name = bfd_section_name (sec);

  if (strcmp (name, ".PARISC.unwind") == 0)
    {
      int indx;
      asection *asec;

#if ARCH_SIZE == 64
      hdr->sh_type = SHT_PARISC_UNWIND;
#else
      /* Note - it is not clear why this is not SHT_PARISC_UNWIND as well.
	 Presumably it is a historical constraint, so leave it as it is.  */
      hdr->sh_type = SHT_PROGBITS;
#endif
      /* ?!? How are unwinds supposed to work for symbols in arbitrary
	 sections?  Or what if we have multiple .text sections in a single
	 .o file?  HP really messed up on this one.

	 Ugh.  We can not use elf_section_data (sec)->this_idx at this
	 point because it is not initialized yet.

	 So we (gasp) recompute it here.  Hopefully nobody ever changes the
	 way sections are numbered in elf.c!  */
      for (asec = abfd->sections, indx = 1; asec; asec = asec->next, indx++)
	{
	  if (asec->name && strcmp (asec->name, ".text") == 0)
	    {
	      hdr->sh_info = indx;
	      hdr->sh_flags |= SHF_INFO_LINK;
	      break;
	    }
	}

      /* The unwind table entries are 16 bytes long, so it is not clear
	 why this field is set to 4.  (The ELF spec says that the sh_entsize
	 field is a byte quantity, but this is a processor specific section,
	 so it is allowed to change the rules).  Leave as it is for now.  */
      hdr->sh_entsize = 4;
    }
  return true;
}

static bool
elf_hppa_final_write_processing (bfd *abfd)
{
  int mach = bfd_get_mach (abfd);

  elf_elfheader (abfd)->e_flags &= ~(EF_PARISC_ARCH | EF_PARISC_TRAPNIL
				     | EF_PARISC_EXT | EF_PARISC_LSB
				     | EF_PARISC_WIDE | EF_PARISC_NO_KABP
				     | EF_PARISC_LAZYSWAP);

  if (mach == 10)
    elf_elfheader (abfd)->e_flags |= EFA_PARISC_1_0;
  else if (mach == 11)
    elf_elfheader (abfd)->e_flags |= EFA_PARISC_1_1;
  else if (mach == 20)
    elf_elfheader (abfd)->e_flags |= EFA_PARISC_2_0;
  else if (mach == 25)
    elf_elfheader (abfd)->e_flags |= (EF_PARISC_WIDE
				      | EFA_PARISC_2_0
				      /* The GNU tools have trapped without
					 option since 1993, so need to take
					 a step backwards with the ELF
					 based toolchains.  */
				      | EF_PARISC_TRAPNIL);
  return _bfd_elf_final_write_processing (abfd);
}

/* Comparison function for qsort to sort unwind section during a
   final link.  */

static int
hppa_unwind_entry_compare (const void *a, const void *b)
{
  const bfd_byte *ap, *bp;
  unsigned long av, bv;

  ap = a;
  av = (unsigned long) ap[0] << 24;
  av |= (unsigned long) ap[1] << 16;
  av |= (unsigned long) ap[2] << 8;
  av |= (unsigned long) ap[3];

  bp = b;
  bv = (unsigned long) bp[0] << 24;
  bv |= (unsigned long) bp[1] << 16;
  bv |= (unsigned long) bp[2] << 8;
  bv |= (unsigned long) bp[3];

  return av < bv ? -1 : av > bv ? 1 : 0;
}

static bool
elf_hppa_sort_unwind (bfd *abfd)
{
  asection *s;

  /* Magic section names, but this is much safer than having
     relocate_section remember where SEGREL32 relocs occurred.
     Consider what happens if someone inept creates a linker script
     that puts unwind information in .text.  */
  s = bfd_get_section_by_name (abfd, ".PARISC.unwind");
  if (s != NULL && (s->flags & SEC_HAS_CONTENTS) != 0)

    {
      bfd_size_type size;
      bfd_byte *contents;

      if (!bfd_malloc_and_get_section (abfd, s, &contents))
	return false;

      size = s->size;
      qsort (contents, (size_t) (size / 16), 16, hppa_unwind_entry_compare);

      if (! bfd_set_section_contents (abfd, s, contents, (file_ptr) 0, size))
	return false;
    }

  return true;
}

/* What to do when ld finds relocations against symbols defined in
   discarded sections.  */

static unsigned int
elf_hppa_action_discarded (asection *sec)
{
  /* Ignore relocations in .data.rel.ro.local.  This section can contain
     PLABEL32 relocations to functions in discarded COMDAT groups.  */
  if (strcmp (".data.rel.ro.local", sec->name) == 0)
    return 0;

  if (strcmp (".PARISC.unwind", sec->name) == 0)
    return 0;

  return _bfd_elf_default_action_discarded (sec);
}
