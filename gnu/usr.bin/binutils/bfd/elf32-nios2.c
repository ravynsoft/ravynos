/* 32-bit ELF support for Nios II.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Nigel Gray (ngray@altera.com).
   Contributed by Mentor Graphics, Inc.

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

/* This file handles Altera Nios II ELF targets.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "bfdlink.h"
#include "genlink.h"
#include "elf-bfd.h"
#include "elf/nios2.h"
#include "opcode/nios2.h"
#include "elf32-nios2.h"
#include "libiberty.h"

/* Use RELA relocations.  */
#ifndef USE_RELA
#define USE_RELA
#endif

#ifdef USE_REL
#undef USE_REL
#endif

/* Forward declarations.  */
static bfd_reloc_status_type nios2_elf32_ignore_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_hi16_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_lo16_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_hiadj16_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_pcrel_lo16_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_pcrel_hiadj16_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_pcrel16_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_call26_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_gprel_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_ujmp_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_cjmp_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nios2_elf32_callr_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);

/* Target vector.  */
extern const bfd_target nios2_elf32_le_vec;
extern const bfd_target nios2_elf32_be_vec;

/* Offset of tp and dtp pointers from start of TLS block.  */
#define TP_OFFSET	0x7000
#define DTP_OFFSET	0x8000

/* The relocation tables used for SHT_REL sections.  There are separate
   tables for R1 and R2 encodings.  */
static reloc_howto_type elf_nios2_r1_howto_table_rel[] = {
  /* No relocation.  */
  HOWTO (R_NIOS2_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_NIOS2_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* 16-bit signed immediate relocation.  */
  HOWTO (R_NIOS2_S16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_signed,	/* complain on overflow */
	 bfd_elf_generic_reloc,	/* special function */
	 "R_NIOS2_S16",		/* name */
	 false,			/* partial_inplace */
	 0x003fffc0,		/* src_mask */
	 0x003fffc0,		/* dest_mask */
	 false),		/* pcrel_offset */

  /* 16-bit unsigned immediate relocation.  */
  HOWTO (R_NIOS2_U16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_unsigned,	/* complain on overflow */
	 bfd_elf_generic_reloc,	/* special function */
	 "R_NIOS2_U16",		/* name */
	 false,			/* partial_inplace */
	 0x003fffc0,		/* src_mask */
	 0x003fffc0,		/* dest_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_NIOS2_PCREL16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_signed,	/* complain on overflow */
	 nios2_elf32_pcrel16_relocate,	/* special function */
	 "R_NIOS2_PCREL16",	/* name */
	 false,			/* partial_inplace */
	 0x003fffc0,		/* src_mask */
	 0x003fffc0,		/* dest_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_NIOS2_CALL26,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_dont,	/* complain on overflow */
	 nios2_elf32_call26_relocate,	/* special function */
	 "R_NIOS2_CALL26",	/* name */
	 false,			/* partial_inplace */
	 0xffffffc0,		/* src_mask */
	 0xffffffc0,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_NIOS2_IMM5,
	 0,
	 4,
	 5,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_IMM5",
	 false,
	 0x000007c0,
	 0x000007c0,
	 false),

  HOWTO (R_NIOS2_CACHE_OPX,
	 0,
	 4,
	 5,
	 false,
	 22,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_CACHE_OPX",
	 false,
	 0x07c00000,
	 0x07c00000,
	 false),

  HOWTO (R_NIOS2_IMM6,
	 0,
	 4,
	 6,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_IMM6",
	 false,
	 0x00000fc0,
	 0x00000fc0,
	 false),

  HOWTO (R_NIOS2_IMM8,
	 0,
	 4,
	 8,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_IMM8",
	 false,
	 0x00003fc0,
	 0x00003fc0,
	 false),

  HOWTO (R_NIOS2_HI16,
	 0,
	 4,
	 32,
	 false,
	 6,
	 complain_overflow_dont,
	 nios2_elf32_hi16_relocate,
	 "R_NIOS2_HI16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_LO16,
	 0,
	 4,
	 32,
	 false,
	 6,
	 complain_overflow_dont,
	 nios2_elf32_lo16_relocate,
	 "R_NIOS2_LO16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_HIADJ16,
	 0,
	 4,
	 32,
	 false,
	 6,
	 complain_overflow_dont,
	 nios2_elf32_hiadj16_relocate,
	 "R_NIOS2_HIADJ16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_BFD_RELOC_32,
	 0,
	 4,			/* long */
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_BFD_RELOC32",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_BFD_RELOC_16,
	 0,
	 2,			/* short */
	 16,
	 false,
	 0,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_BFD_RELOC16",
	 false,
	 0x0000ffff,
	 0x0000ffff,
	 false),

  HOWTO (R_NIOS2_BFD_RELOC_8,
	 0,
	 1,			/* byte */
	 8,
	 false,
	 0,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_BFD_RELOC8",
	 false,
	 0x000000ff,
	 0x000000ff,
	 false),

  HOWTO (R_NIOS2_GPREL,
	 0,
	 4,
	 32,
	 false,
	 6,
	 complain_overflow_dont,
	 nios2_elf32_gprel_relocate,
	 "R_NIOS2_GPREL",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_GNU_VTINHERIT,
	 0,
	 4,
	 0,
	 false,
	 0,
	 complain_overflow_dont,
	 NULL,
	 "R_NIOS2_GNU_VTINHERIT",
	 false,
	 0,
	 0,
	 false),

  HOWTO (R_NIOS2_GNU_VTENTRY,
	 0,
	 4,
	 0,
	 false,
	 0,
	 complain_overflow_dont,
	 _bfd_elf_rel_vtable_reloc_fn,
	 "R_NIOS2_GNU_VTENTRY",
	 false,
	 0,
	 0,
	 false),

  HOWTO (R_NIOS2_UJMP,
	 0,
	 4,
	 32,
	 false,
	 6,
	 complain_overflow_dont,
	 nios2_elf32_ujmp_relocate,
	 "R_NIOS2_UJMP",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_CJMP,
	 0,
	 4,
	 32,
	 false,
	 6,
	 complain_overflow_dont,
	 nios2_elf32_cjmp_relocate,
	 "R_NIOS2_CJMP",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_CALLR,
	 0,
	 4,
	 32,
	 false,
	 6,
	 complain_overflow_dont,
	 nios2_elf32_callr_relocate,
	 "R_NIOS2_CALLR",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_ALIGN,
	 0,
	 4,
	 0,
	 false,
	 0,
	 complain_overflow_dont,
	 nios2_elf32_ignore_reloc,
	 "R_NIOS2_ALIGN",
	 false,
	 0,
	 0,
	 true),


  HOWTO (R_NIOS2_GOT16,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOT16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_CALL16,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_CALL16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_GOTOFF_LO,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOTOFF_LO",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_GOTOFF_HA,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOTOFF_HA",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_PCREL_LO,
	 0,
	 4,
	 16,
	 true,
	 6,
	 complain_overflow_dont,
	 nios2_elf32_pcrel_lo16_relocate,
	 "R_NIOS2_PCREL_LO",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 true),

  HOWTO (R_NIOS2_PCREL_HA,
	 0,
	 4,
	 16,
	 false, /* This is a PC-relative relocation, but we need to subtract
		   PC ourselves before the HIADJ.  */
	 6,
	 complain_overflow_dont,
	 nios2_elf32_pcrel_hiadj16_relocate,
	 "R_NIOS2_PCREL_HA",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 true),

  HOWTO (R_NIOS2_TLS_GD16,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_GD16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_TLS_LDM16,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_LDM16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_TLS_LDO16,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_LDO16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_TLS_IE16,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_IE16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_TLS_LE16,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_LE16",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_TLS_DTPMOD,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_DTPMOD",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_TLS_DTPREL,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_DTPREL",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_TLS_TPREL,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_TPREL",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_COPY,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_COPY",
	 false,
	 0,
	 0,
	 false),

  HOWTO (R_NIOS2_GLOB_DAT,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GLOB_DAT",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_JUMP_SLOT,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_JUMP_SLOT",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_RELATIVE,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_RELATIVE",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_GOTOFF,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOTOFF",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_CALL26_NOAT,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_dont,	/* complain on overflow */
	 nios2_elf32_call26_relocate,	/* special function */
	 "R_NIOS2_CALL26_NOAT",	/* name */
	 false,			/* partial_inplace */
	 0xffffffc0,		/* src_mask */
	 0xffffffc0,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_NIOS2_GOT_LO,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOT_LO",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_GOT_HA,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOT_HA",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_CALL_LO,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_CALL_LO",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

  HOWTO (R_NIOS2_CALL_HA,
	 0,
	 4,
	 16,
	 false,
	 6,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_CALL_HA",
	 false,
	 0x003fffc0,
	 0x003fffc0,
	 false),

/* Add other relocations here.  */
};

static reloc_howto_type elf_nios2_r2_howto_table_rel[] = {
  /* No relocation.  */
  HOWTO (R_NIOS2_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_NIOS2_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* 16-bit signed immediate relocation.  */
  HOWTO (R_NIOS2_S16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed,	/* complain on overflow */
	 bfd_elf_generic_reloc,	/* special function */
	 "R_NIOS2_S16",		/* name */
	 false,			/* partial_inplace */
	 0xffff0000,		/* src_mask */
	 0xffff0000,		/* dest_mask */
	 false),		/* pcrel_offset */

  /* 16-bit unsigned immediate relocation.  */
  HOWTO (R_NIOS2_U16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_unsigned,	/* complain on overflow */
	 bfd_elf_generic_reloc,	/* special function */
	 "R_NIOS2_U16",		/* name */
	 false,			/* partial_inplace */
	 0xffff0000,		/* src_mask */
	 0xffff0000,		/* dest_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_NIOS2_PCREL16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed,	/* complain on overflow */
	 nios2_elf32_pcrel16_relocate,	/* special function */
	 "R_NIOS2_PCREL16",	/* name */
	 false,			/* partial_inplace */
	 0xffff0000,		/* src_mask */
	 0xffff0000,		/* dest_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_NIOS2_CALL26,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_dont,	/* complain on overflow */
	 nios2_elf32_call26_relocate,	/* special function */
	 "R_NIOS2_CALL26",	/* name */
	 false,			/* partial_inplace */
	 0xffffffc0,		/* src_mask */
	 0xffffffc0,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_NIOS2_IMM5,
	 0,
	 4,
	 5,
	 false,
	 21,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_IMM5",
	 false,
	 0x03e00000,
	 0x03e00000,
	 false),

  HOWTO (R_NIOS2_CACHE_OPX,
	 0,
	 4,
	 5,
	 false,
	 11,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_CACHE_OPX",
	 false,
	 0x0000f800,
	 0x0000f800,
	 false),

  HOWTO (R_NIOS2_IMM6,
	 0,
	 4,
	 6,
	 false,
	 26,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_IMM6",
	 false,
	 0xfc000000,
	 0xfc000000,
	 false),

  HOWTO (R_NIOS2_IMM8,
	 0,
	 4,
	 8,
	 false,
	 24,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_IMM8",
	 false,
	 0xff000000,
	 0xff000000,
	 false),

  HOWTO (R_NIOS2_HI16,
	 0,
	 4,
	 32,
	 false,
	 16,
	 complain_overflow_dont,
	 nios2_elf32_hi16_relocate,
	 "R_NIOS2_HI16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_LO16,
	 0,
	 4,
	 32,
	 false,
	 16,
	 complain_overflow_dont,
	 nios2_elf32_lo16_relocate,
	 "R_NIOS2_LO16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_HIADJ16,
	 0,
	 4,
	 32,
	 false,
	 16,
	 complain_overflow_dont,
	 nios2_elf32_hiadj16_relocate,
	 "R_NIOS2_HIADJ16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_BFD_RELOC_32,
	 0,
	 4,			/* long */
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_BFD_RELOC32",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_BFD_RELOC_16,
	 0,
	 2,			/* short */
	 16,
	 false,
	 0,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_BFD_RELOC16",
	 false,
	 0x0000ffff,
	 0x0000ffff,
	 false),

  HOWTO (R_NIOS2_BFD_RELOC_8,
	 0,
	 1,			/* byte */
	 8,
	 false,
	 0,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_BFD_RELOC8",
	 false,
	 0x000000ff,
	 0x000000ff,
	 false),

  HOWTO (R_NIOS2_GPREL,
	 0,
	 4,
	 32,
	 false,
	 16,
	 complain_overflow_dont,
	 nios2_elf32_gprel_relocate,
	 "R_NIOS2_GPREL",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_GNU_VTINHERIT,
	 0,
	 4,
	 0,
	 false,
	 0,
	 complain_overflow_dont,
	 NULL,
	 "R_NIOS2_GNU_VTINHERIT",
	 false,
	 0,
	 0,
	 false),

  HOWTO (R_NIOS2_GNU_VTENTRY,
	 0,
	 4,
	 0,
	 false,
	 0,
	 complain_overflow_dont,
	 _bfd_elf_rel_vtable_reloc_fn,
	 "R_NIOS2_GNU_VTENTRY",
	 false,
	 0,
	 0,
	 false),

  HOWTO (R_NIOS2_UJMP,
	 0,
	 4,
	 32,
	 false,
	 16,
	 complain_overflow_dont,
	 nios2_elf32_ujmp_relocate,
	 "R_NIOS2_UJMP",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_CJMP,
	 0,
	 4,
	 32,
	 false,
	 16,
	 complain_overflow_dont,
	 nios2_elf32_cjmp_relocate,
	 "R_NIOS2_CJMP",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_CALLR,
	 0,
	 4,
	 32,
	 false,
	 16,
	 complain_overflow_dont,
	 nios2_elf32_callr_relocate,
	 "R_NIOS2_CALLR",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_ALIGN,
	 0,
	 4,
	 0,
	 false,
	 0,
	 complain_overflow_dont,
	 nios2_elf32_ignore_reloc,
	 "R_NIOS2_ALIGN",
	 false,
	 0,
	 0,
	 true),

  HOWTO (R_NIOS2_GOT16,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOT16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_CALL16,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_CALL16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_GOTOFF_LO,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOTOFF_LO",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_GOTOFF_HA,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOTOFF_HA",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_PCREL_LO,
	 0,
	 4,
	 16,
	 true,
	 16,
	 complain_overflow_dont,
	 nios2_elf32_pcrel_lo16_relocate,
	 "R_NIOS2_PCREL_LO",
	 false,
	 0xffff0000,
	 0xffff0000,
	 true),

  HOWTO (R_NIOS2_PCREL_HA,
	 0,
	 4,
	 16,
	 false, /* This is a PC-relative relocation, but we need to subtract
		   PC ourselves before the HIADJ.  */
	 16,
	 complain_overflow_dont,
	 nios2_elf32_pcrel_hiadj16_relocate,
	 "R_NIOS2_PCREL_HA",
	 false,
	 0xffff0000,
	 0xffff0000,
	 true),

  HOWTO (R_NIOS2_TLS_GD16,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_GD16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_TLS_LDM16,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_LDM16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_TLS_LDO16,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_LDO16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_TLS_IE16,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_IE16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_TLS_LE16,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_LE16",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_TLS_DTPMOD,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_DTPMOD",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_TLS_DTPREL,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_DTPREL",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_TLS_TPREL,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_TLS_TPREL",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_COPY,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_COPY",
	 false,
	 0,
	 0,
	 false),

  HOWTO (R_NIOS2_GLOB_DAT,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GLOB_DAT",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_JUMP_SLOT,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_JUMP_SLOT",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_RELATIVE,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_RELATIVE",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_GOTOFF,
	 0,
	 4,
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOTOFF",
	 false,
	 0xffffffff,
	 0xffffffff,
	 false),

  HOWTO (R_NIOS2_CALL26_NOAT,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_dont,	/* complain on overflow */
	 nios2_elf32_call26_relocate,	/* special function */
	 "R_NIOS2_CALL26_NOAT",	/* name */
	 false,			/* partial_inplace */
	 0xffffffc0,		/* src_mask */
	 0xffffffc0,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_NIOS2_GOT_LO,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOT_LO",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_GOT_HA,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_GOT_HA",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_CALL_LO,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_CALL_LO",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_CALL_HA,
	 0,
	 4,
	 16,
	 false,
	 16,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_CALL_HA",
	 false,
	 0xffff0000,
	 0xffff0000,
	 false),

  HOWTO (R_NIOS2_R2_S12,
	 0,
	 4,
	 12,
	 false,
	 16,
	 complain_overflow_signed,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_S12",
	 false,
	 0x0fff0000,
	 0x0fff0000,
	 false),

  HOWTO (R_NIOS2_R2_I10_1_PCREL,
	 1,
	 2,
	 10,
	 true,
	 6,
	 complain_overflow_signed,
	 bfd_elf_generic_reloc,		/* FIXME? */
	 "R_NIOS2_R2_I10_1_PCREL",
	 false,
	 0xffc0,
	 0xffc0,
	 true),

  HOWTO (R_NIOS2_R2_T1I7_1_PCREL,
	 1,
	 2,
	 7,
	 true,
	 9,
	 complain_overflow_signed,
	 bfd_elf_generic_reloc,		/* FIXME? */
	 "R_NIOS2_R2_T1I7_1_PCREL",
	 false,
	 0xfe00,
	 0xfe00,
	 true),

  HOWTO (R_NIOS2_R2_T1I7_2,
	 2,
	 2,
	 7,
	 false,
	 9,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_T1I7_2",
	 false,
	 0xfe00,
	 0xfe00,
	 false),

  HOWTO (R_NIOS2_R2_T2I4,
	 0,
	 2,
	 4,
	 false,
	 12,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_T2I4",
	 false,
	 0xf000,
	 0xf000,
	 false),

  HOWTO (R_NIOS2_R2_T2I4_1,
	 1,
	 2,
	 4,
	 false,
	 12,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_T2I4_1",
	 false,
	 0xf000,
	 0xf000,
	 false),

  HOWTO (R_NIOS2_R2_T2I4_2,
	 2,
	 2,
	 4,
	 false,
	 12,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_T2I4_2",
	 false,
	 0xf000,
	 0xf000,
	 false),

  HOWTO (R_NIOS2_R2_X1I7_2,
	 2,
	 2,
	 7,
	 false,
	 6,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_X1I7_2",
	 false,
	 0x1fc0,
	 0x1fc0,
	 false),

  HOWTO (R_NIOS2_R2_X2L5,
	 0,
	 2,
	 5,
	 false,
	 6,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_X2L5",
	 false,
	 0x07c0,
	 0x07c0,
	 false),

  HOWTO (R_NIOS2_R2_F1I5_2,
	 2,
	 2,
	 5,
	 false,
	 6,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_F1L5_2",
	 false,
	 0x07c0,
	 0x07c0,
	 false),

  HOWTO (R_NIOS2_R2_L5I4X1,
	 2,
	 2,
	 4,
	 false,
	 6,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_L5I4X1",
	 false,
	 0x03c0,
	 0x03c0,
	 false),

  HOWTO (R_NIOS2_R2_T1X1I6,
	 0,
	 2,
	 6,
	 false,
	 9,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_T1X1I6",
	 false,
	 0x7e00,
	 0x7e00,
	 false),

  HOWTO (R_NIOS2_R2_T1X1I6_2,
	 2,
	 4,
	 6,
	 false,
	 9,
	 complain_overflow_unsigned,
	 bfd_elf_generic_reloc,
	 "R_NIOS2_R2_T1I1X6_2",
	 false,
	 0x7e00,
	 0x7e00,
	 false),

/* Add other relocations here.  */
};

static unsigned char elf_code_to_howto_index[R_NIOS2_ILLEGAL + 1];


/* Return true if producing output for a R2 BFD.  */
#define BFD_IS_R2(abfd) (bfd_get_mach (abfd) == bfd_mach_nios2r2)

/* Return the howto for relocation RTYPE.  */
static reloc_howto_type *
lookup_howto (unsigned int rtype, bfd *abfd)
{
  static int initialized = 0;
  int i;
  /* R2 relocations are a superset of R1, so use that for the lookup
     table.  */
  int r1_howto_tbl_size = (int) ARRAY_SIZE (elf_nios2_r1_howto_table_rel);
  int r2_howto_tbl_size = (int) ARRAY_SIZE (elf_nios2_r2_howto_table_rel);

  if (!initialized)
    {
      initialized = 1;
      memset (elf_code_to_howto_index, 0xff,
	      sizeof (elf_code_to_howto_index));
      for (i = 0; i < r2_howto_tbl_size; i++)
	{
	  elf_code_to_howto_index[elf_nios2_r2_howto_table_rel[i].type] = i;
	  if (i < r1_howto_tbl_size)
	    BFD_ASSERT (elf_nios2_r2_howto_table_rel[i].type
			== elf_nios2_r1_howto_table_rel[i].type);
	}
    }

  if (rtype > R_NIOS2_ILLEGAL)
    return NULL;
  i = elf_code_to_howto_index[rtype];
  if (BFD_IS_R2 (abfd))
    {
      if (i >= r2_howto_tbl_size)
	return NULL;
      return elf_nios2_r2_howto_table_rel + i;
    }
  else
    {
      if (i >= r1_howto_tbl_size)
	return NULL;
      return elf_nios2_r1_howto_table_rel + i;
    }
}

/* Map for converting BFD reloc types to Nios II reloc types.  */
struct elf_reloc_map
{
  bfd_reloc_code_real_type bfd_val;
  enum elf_nios2_reloc_type elf_val;
};

static const struct elf_reloc_map nios2_reloc_map[] =
{
  {BFD_RELOC_NONE, R_NIOS2_NONE},
  {BFD_RELOC_NIOS2_S16, R_NIOS2_S16},
  {BFD_RELOC_NIOS2_U16, R_NIOS2_U16},
  {BFD_RELOC_16_PCREL, R_NIOS2_PCREL16},
  {BFD_RELOC_NIOS2_CALL26, R_NIOS2_CALL26},
  {BFD_RELOC_NIOS2_IMM5, R_NIOS2_IMM5},
  {BFD_RELOC_NIOS2_CACHE_OPX, R_NIOS2_CACHE_OPX},
  {BFD_RELOC_NIOS2_IMM6, R_NIOS2_IMM6},
  {BFD_RELOC_NIOS2_IMM8, R_NIOS2_IMM8},
  {BFD_RELOC_NIOS2_HI16, R_NIOS2_HI16},
  {BFD_RELOC_NIOS2_LO16, R_NIOS2_LO16},
  {BFD_RELOC_NIOS2_HIADJ16, R_NIOS2_HIADJ16},
  {BFD_RELOC_32, R_NIOS2_BFD_RELOC_32},
  {BFD_RELOC_16, R_NIOS2_BFD_RELOC_16},
  {BFD_RELOC_8, R_NIOS2_BFD_RELOC_8},
  {BFD_RELOC_NIOS2_GPREL, R_NIOS2_GPREL},
  {BFD_RELOC_VTABLE_INHERIT, R_NIOS2_GNU_VTINHERIT},
  {BFD_RELOC_VTABLE_ENTRY, R_NIOS2_GNU_VTENTRY},
  {BFD_RELOC_NIOS2_UJMP, R_NIOS2_UJMP},
  {BFD_RELOC_NIOS2_CJMP, R_NIOS2_CJMP},
  {BFD_RELOC_NIOS2_CALLR, R_NIOS2_CALLR},
  {BFD_RELOC_NIOS2_ALIGN, R_NIOS2_ALIGN},
  {BFD_RELOC_NIOS2_GOT16, R_NIOS2_GOT16},
  {BFD_RELOC_NIOS2_CALL16, R_NIOS2_CALL16},
  {BFD_RELOC_NIOS2_GOTOFF_LO, R_NIOS2_GOTOFF_LO},
  {BFD_RELOC_NIOS2_GOTOFF_HA, R_NIOS2_GOTOFF_HA},
  {BFD_RELOC_NIOS2_PCREL_LO, R_NIOS2_PCREL_LO},
  {BFD_RELOC_NIOS2_PCREL_HA, R_NIOS2_PCREL_HA},
  {BFD_RELOC_NIOS2_TLS_GD16, R_NIOS2_TLS_GD16},
  {BFD_RELOC_NIOS2_TLS_LDM16, R_NIOS2_TLS_LDM16},
  {BFD_RELOC_NIOS2_TLS_LDO16, R_NIOS2_TLS_LDO16},
  {BFD_RELOC_NIOS2_TLS_IE16, R_NIOS2_TLS_IE16},
  {BFD_RELOC_NIOS2_TLS_LE16, R_NIOS2_TLS_LE16},
  {BFD_RELOC_NIOS2_TLS_DTPMOD, R_NIOS2_TLS_DTPMOD},
  {BFD_RELOC_NIOS2_TLS_DTPREL, R_NIOS2_TLS_DTPREL},
  {BFD_RELOC_NIOS2_TLS_TPREL, R_NIOS2_TLS_TPREL},
  {BFD_RELOC_NIOS2_COPY, R_NIOS2_COPY},
  {BFD_RELOC_NIOS2_GLOB_DAT, R_NIOS2_GLOB_DAT},
  {BFD_RELOC_NIOS2_JUMP_SLOT, R_NIOS2_JUMP_SLOT},
  {BFD_RELOC_NIOS2_RELATIVE, R_NIOS2_RELATIVE},
  {BFD_RELOC_NIOS2_GOTOFF, R_NIOS2_GOTOFF},
  {BFD_RELOC_NIOS2_CALL26_NOAT, R_NIOS2_CALL26_NOAT},
  {BFD_RELOC_NIOS2_GOT_LO, R_NIOS2_GOT_LO},
  {BFD_RELOC_NIOS2_GOT_HA, R_NIOS2_GOT_HA},
  {BFD_RELOC_NIOS2_CALL_LO, R_NIOS2_CALL_LO},
  {BFD_RELOC_NIOS2_CALL_HA, R_NIOS2_CALL_HA},
  {BFD_RELOC_NIOS2_R2_S12, R_NIOS2_R2_S12},
  {BFD_RELOC_NIOS2_R2_I10_1_PCREL, R_NIOS2_R2_I10_1_PCREL},
  {BFD_RELOC_NIOS2_R2_T1I7_1_PCREL, R_NIOS2_R2_T1I7_1_PCREL},
  {BFD_RELOC_NIOS2_R2_T1I7_2, R_NIOS2_R2_T1I7_2},
  {BFD_RELOC_NIOS2_R2_T2I4, R_NIOS2_R2_T2I4},
  {BFD_RELOC_NIOS2_R2_T2I4_1, R_NIOS2_R2_T2I4_1},
  {BFD_RELOC_NIOS2_R2_T2I4_2, R_NIOS2_R2_T2I4_2},
  {BFD_RELOC_NIOS2_R2_X1I7_2, R_NIOS2_R2_X1I7_2},
  {BFD_RELOC_NIOS2_R2_X2L5, R_NIOS2_R2_X2L5},
  {BFD_RELOC_NIOS2_R2_F1I5_2, R_NIOS2_R2_F1I5_2},
  {BFD_RELOC_NIOS2_R2_L5I4X1, R_NIOS2_R2_L5I4X1},
  {BFD_RELOC_NIOS2_R2_T1X1I6, R_NIOS2_R2_T1X1I6},
  {BFD_RELOC_NIOS2_R2_T1X1I6_2, R_NIOS2_R2_T1X1I6_2},
};

enum elf32_nios2_stub_type
{
  nios2_stub_call26_before,
  nios2_stub_call26_after,
  nios2_stub_none
};

struct elf32_nios2_stub_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry bh_root;

  /* The stub section.  */
  asection *stub_sec;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump.  */
  bfd_vma target_value;
  asection *target_section;

  enum elf32_nios2_stub_type stub_type;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf32_nios2_link_hash_entry *hh;

  /* And the reloc addend that this was derived from.  */
  bfd_vma addend;

  /* Where this stub is being called from, or, in the case of combined
     stub sections, the first input section in the group.  */
  asection *id_sec;
};

#define nios2_stub_hash_entry(ent) \
  ((struct elf32_nios2_stub_hash_entry *)(ent))

#define nios2_stub_hash_lookup(table, string, create, copy) \
  ((struct elf32_nios2_stub_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))


/* Nios II ELF linker hash entry.  */

struct elf32_nios2_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* A pointer to the most recently used stub hash entry against this
     symbol.  */
  struct elf32_nios2_stub_hash_entry *hsh_cache;

#define GOT_UNKNOWN	0
#define GOT_NORMAL	1
#define GOT_TLS_GD	2
#define GOT_TLS_IE	4
  unsigned char tls_type;

  /* We need to detect and take special action for symbols which are only
     referenced with %call() and not with %got().  Such symbols do not need
     a dynamic GOT reloc in shared objects, only a dynamic PLT reloc.  Lazy
     linking will not work if the dynamic GOT reloc exists.
     To check for this condition efficiently, we compare got_types_used against
     CALL_USED, meaning
     (got_types_used & (GOT_USED | CALL_USED)) == CALL_USED.
  */
#define GOT_USED	1
#define CALL_USED	2
  unsigned char got_types_used;
};

#define elf32_nios2_hash_entry(ent) \
  ((struct elf32_nios2_link_hash_entry *) (ent))

/* Get the Nios II elf linker hash table from a link_info structure.  */
#define elf32_nios2_hash_table(info) \
  ((struct elf32_nios2_link_hash_table *) ((info)->hash))

/* Nios II ELF linker hash table.  */
struct elf32_nios2_link_hash_table
  {
    /* The main hash table.  */
    struct elf_link_hash_table root;

    /* The stub hash table.  */
    struct bfd_hash_table bstab;

    /* Linker stub bfd.  */
    bfd *stub_bfd;

    /* Linker call-backs.  */
    asection * (*add_stub_section) (const char *, asection *, bool);
    void (*layout_sections_again) (void);

    /* Array to keep track of which stub sections have been created, and
       information on stub grouping.  */
    struct map_stub
    {
      /* These are the section to which stubs in the group will be
	 attached.  */
      asection *first_sec, *last_sec;
      /* The stub sections.  There might be stubs inserted either before
	 or after the real section.*/
      asection *first_stub_sec, *last_stub_sec;
    } *stub_group;

    /* Assorted information used by nios2_elf32_size_stubs.  */
    unsigned int bfd_count;
    unsigned int top_index;
    asection **input_list;
    Elf_Internal_Sym **all_local_syms;

    /* Short-cuts to get to dynamic linker sections.  */
    asection *sbss;

    /* GOT pointer symbol _gp_got.  */
    struct elf_link_hash_entry *h_gp_got;

    union {
      bfd_signed_vma refcount;
      bfd_vma offset;
    } tls_ldm_got;

    bfd_vma res_n_size;
  };

struct nios2_elf32_obj_tdata
{
  struct elf_obj_tdata root;

  /* tls_type for each local got entry.  */
  char *local_got_tls_type;

  /* TRUE if TLS GD relocs have been seen for this object.  */
  bool has_tlsgd;
};

#define elf32_nios2_tdata(abfd) \
  ((struct nios2_elf32_obj_tdata *) (abfd)->tdata.any)

#define elf32_nios2_local_got_tls_type(abfd) \
  (elf32_nios2_tdata (abfd)->local_got_tls_type)

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER "/lib/ld.so.1"

/* PLT implementation for position-dependent code.  */
static const bfd_vma nios2_plt_entry[] = { /* .PLTn: */
  0x03c00034,	/* movhi r15, %hiadj(plt_got_slot_address) */
  0x7bc00017,	/* ldw r15, %lo(plt_got_slot_address)(r15) */
  0x7800683a	/* jmp r15 */
};

static const bfd_vma nios2_plt0_entry[] = { /* .PLTresolve */
  0x03800034,	/* movhi r14, %hiadj(res_0) */
  0x73800004,	/* addi r14, r14, %lo(res_0) */
  0x7b9fc83a,	/* sub r15, r15, r14 */
  0x03400034,	/* movhi r13, %hiadj(_GLOBAL_OFFSET_TABLE_) */
  0x6b800017,	/* ldw r14, %lo(_GLOBAL_OFFSET_TABLE_+4)(r13) */
  0x6b400017,	/* ldw r13, %lo(_GLOBAL_OFFSET_TABLE_+8)(r13) */
  0x6800683a	/* jmp r13 */
};

/* PLT implementation for position-independent code.  */
static const bfd_vma nios2_so_plt_entry[] = { /* .PLTn */
  0x03c00034,	/* movhi r15, %hiadj(index * 4) */
  0x7bc00004,	/* addi r15, r15, %lo(index * 4) */
  0x00000006	/* br .PLTresolve */
};

static const bfd_vma nios2_so_plt0_entry[] = { /* .PLTresolve */
  0x001ce03a,	/* nextpc r14 */
  0x03400034,	/* movhi r13, %hiadj(_GLOBAL_OFFSET_TABLE_) */
  0x6b9b883a,	/* add r13, r13, r14 */
  0x6b800017,	/* ldw r14, %lo(_GLOBAL_OFFSET_TABLE_+4)(r13) */
  0x6b400017,	/* ldw r13, %lo(_GLOBAL_OFFSET_TABLE_+8)(r13) */
  0x6800683a	/* jmp r13 */
};

/* CALL26 stub.  */
static const bfd_vma nios2_call26_stub_entry[] = {
  0x00400034,	/* orhi at, r0, %hiadj(dest) */
  0x08400004,	/* addi at, at, %lo(dest) */
  0x0800683a	/* jmp at */
};

/* Install 16-bit immediate value VALUE at offset OFFSET into section SEC.  */
static void
nios2_elf32_install_imm16 (asection *sec, bfd_vma offset, bfd_vma value)
{
  bfd_vma word = bfd_get_32 (sec->owner, sec->contents + offset);

  BFD_ASSERT (value <= 0xffff || ((bfd_signed_vma) value) >= -0xffff);

  bfd_put_32 (sec->owner, word | ((value & 0xffff) << 6),
	      sec->contents + offset);
}

/* Install COUNT 32-bit values DATA starting at offset OFFSET into
   section SEC. */
static void
nios2_elf32_install_data (asection *sec, const bfd_vma *data, bfd_vma offset,
			  int count)
{
  while (count--)
    {
      bfd_put_32 (sec->owner, *data, sec->contents + offset);
      offset += 4;
      ++data;
    }
}

/* The usual way of loading a 32-bit constant into a Nios II register is to
   load the high 16 bits in one instruction and then add the low 16 bits with
   a signed add. This means that the high halfword needs to be adjusted to
   compensate for the sign bit of the low halfword. This function returns the
   adjusted high halfword for a given 32-bit constant.  */
static
bfd_vma hiadj (bfd_vma symbol_value)
{
  return ((symbol_value + 0x8000) >> 16) & 0xffff;
}

/* Implement elf_backend_grok_prstatus:
   Support for core dump NOTE sections.  */
static bool
nios2_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
    default:
      return false;

    case 212:	      /* Linux/Nios II */
      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

      /* pr_pid */
      elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, note->descdata + 24);

      /* pr_reg */
      offset = 72;
      size = 136;

      break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

/* Implement elf_backend_grok_psinfo.  */
static bool
nios2_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

    case 124:	      /* Linux/Nios II elf_prpsinfo */
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 28, 16);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 44, 80);
    }

  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */

  {
    char *command = elf_tdata (abfd)->core->command;
    int n = strlen (command);

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return true;
}

/* Assorted hash table functions.  */

/* Initialize an entry in the stub hash table.  */
static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table,
		   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf32_nios2_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf32_nios2_stub_hash_entry *hsh;

      /* Initialize the local fields.  */
      hsh = (struct elf32_nios2_stub_hash_entry *) entry;
      hsh->stub_sec = NULL;
      hsh->stub_offset = 0;
      hsh->target_value = 0;
      hsh->target_section = NULL;
      hsh->stub_type = nios2_stub_none;
      hsh->hh = NULL;
      hsh->id_sec = NULL;
    }

  return entry;
}

/* Create an entry in a Nios II ELF linker hash table.  */
static struct bfd_hash_entry *
link_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table, const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf32_nios2_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry)
    {
      struct elf32_nios2_link_hash_entry *eh;

      eh = (struct elf32_nios2_link_hash_entry *) entry;
      eh->hsh_cache = NULL;
      eh->tls_type = GOT_UNKNOWN;
      eh->got_types_used = 0;
    }

  return entry;
}

/* Section name for stubs is the associated section name plus this
   string.  */
#define STUB_SUFFIX ".stub"

/* Build a name for an entry in the stub hash table.  */
static char *
nios2_stub_name (const asection *input_section,
		 const asection *sym_sec,
		 const struct elf32_nios2_link_hash_entry *hh,
		 const Elf_Internal_Rela *rel,
		 enum elf32_nios2_stub_type stub_type)
{
  char *stub_name;
  bfd_size_type len;
  char stubpos = (stub_type == nios2_stub_call26_before) ? 'b' : 'a';

  if (hh)
    {
      len = 8 + 1 + 1 + 1+ strlen (hh->root.root.root.string) + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	{
	  sprintf (stub_name, "%08x_%c_%s+%x",
		   input_section->id & 0xffffffff,
		   stubpos,
		   hh->root.root.root.string,
		   (int) rel->r_addend & 0xffffffff);
	}
    }
  else
    {
      len = 8 + 1 + 1 + 1+ 8 + 1 + 8 + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	{
	  sprintf (stub_name, "%08x_%c_%x:%x+%x",
		   input_section->id & 0xffffffff,
		   stubpos,
		   sym_sec->id & 0xffffffff,
		   (int) ELF32_R_SYM (rel->r_info) & 0xffffffff,
		   (int) rel->r_addend & 0xffffffff);
	}
    }
  return stub_name;
}

/* Look up an entry in the stub hash.  Stub entries are cached because
   creating the stub name takes a bit of time.  */
static struct elf32_nios2_stub_hash_entry *
nios2_get_stub_entry (const asection *input_section,
		      const asection *sym_sec,
		      struct elf32_nios2_link_hash_entry *hh,
		      const Elf_Internal_Rela *rel,
		      struct elf32_nios2_link_hash_table *htab,
		      enum elf32_nios2_stub_type stub_type)
{
  struct elf32_nios2_stub_hash_entry *hsh;
  const asection *id_sec;

  /* If this input section is part of a group of sections sharing one
     stub section, then use the id of the first/last section in the group,
     depending on the stub section placement relative to the group.
     Stub names need to include a section id, as there may well be
     more than one stub used to reach say, printf, and we need to
     distinguish between them.  */
  if (stub_type == nios2_stub_call26_before)
    id_sec = htab->stub_group[input_section->id].first_sec;
  else
    id_sec = htab->stub_group[input_section->id].last_sec;

  if (hh != NULL && hh->hsh_cache != NULL
      && hh->hsh_cache->hh == hh
      && hh->hsh_cache->id_sec == id_sec
      && hh->hsh_cache->stub_type == stub_type)
    {
      hsh = hh->hsh_cache;
    }
  else
    {
      char *stub_name;

      stub_name = nios2_stub_name (id_sec, sym_sec, hh, rel, stub_type);
      if (stub_name == NULL)
	return NULL;

      hsh = nios2_stub_hash_lookup (&htab->bstab,
				    stub_name, false, false);

      if (hh != NULL)
	hh->hsh_cache = hsh;

      free (stub_name);
    }

  return hsh;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */
static struct elf32_nios2_stub_hash_entry *
nios2_add_stub (const char *stub_name,
		asection *section,
		struct elf32_nios2_link_hash_table *htab,
		enum elf32_nios2_stub_type stub_type)
{
  asection *link_sec;
  asection *stub_sec;
  asection **secptr, **linkptr;
  struct elf32_nios2_stub_hash_entry *hsh;
  bool afterp;

  if (stub_type == nios2_stub_call26_before)
    {
      link_sec = htab->stub_group[section->id].first_sec;
      secptr = &(htab->stub_group[section->id].first_stub_sec);
      linkptr = &(htab->stub_group[link_sec->id].first_stub_sec);
      afterp = false;
    }
  else
    {
      link_sec = htab->stub_group[section->id].last_sec;
      secptr = &(htab->stub_group[section->id].last_stub_sec);
      linkptr = &(htab->stub_group[link_sec->id].last_stub_sec);
      afterp = true;
    }
  stub_sec = *secptr;
  if (stub_sec == NULL)
    {
      stub_sec = *linkptr;
      if (stub_sec == NULL)
	{
	  size_t namelen;
	  bfd_size_type len;
	  char *s_name;

	  namelen = strlen (link_sec->name);
	  len = namelen + sizeof (STUB_SUFFIX);
	  s_name = bfd_alloc (htab->stub_bfd, len);
	  if (s_name == NULL)
	    return NULL;

	  memcpy (s_name, link_sec->name, namelen);
	  memcpy (s_name + namelen, STUB_SUFFIX, sizeof (STUB_SUFFIX));

	  stub_sec = (*htab->add_stub_section) (s_name, link_sec, afterp);
	  if (stub_sec == NULL)
	    return NULL;
	  *linkptr = stub_sec;
	}
      *secptr = stub_sec;
    }

  /* Enter this entry into the linker stub hash table.  */
  hsh = nios2_stub_hash_lookup (&htab->bstab, stub_name,
				true, false);
  if (hsh == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: cannot create stub entry %s"),
			  section->owner,
			  stub_name);
      return NULL;
    }

  hsh->stub_sec = stub_sec;
  hsh->stub_offset = 0;
  hsh->id_sec = link_sec;
  return hsh;
}

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */
int
nios2_elf32_setup_section_lists (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *input_bfd;
  unsigned int bfd_count;
  unsigned int top_id, top_index;
  asection *section;
  asection **input_list, **list;
  size_t amt;
  struct elf32_nios2_link_hash_table *htab = elf32_nios2_hash_table (info);

  /* Count the number of input BFDs and find the top input section id.  */
  for (input_bfd = info->input_bfds, bfd_count = 0, top_id = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next)
    {
      bfd_count += 1;
      for (section = input_bfd->sections;
	   section != NULL;
	   section = section->next)
	{
	  if (top_id < section->id)
	    top_id = section->id;
	}
    }

  htab->bfd_count = bfd_count;

  amt = sizeof (struct map_stub) * (top_id + 1);
  htab->stub_group = bfd_zmalloc (amt);
  if (htab->stub_group == NULL)
    return -1;

  /* We can't use output_bfd->section_count here to find the top output
     section index as some sections may have been removed, and
     strip_excluded_output_sections doesn't renumber the indices.  */
  for (section = output_bfd->sections, top_index = 0;
       section != NULL;
       section = section->next)
    {
      if (top_index < section->index)
	top_index = section->index;
    }

  htab->top_index = top_index;
  amt = sizeof (asection *) * (top_index + 1);
  input_list = bfd_malloc (amt);
  htab->input_list = input_list;
  if (input_list == NULL)
    return -1;

  /* For sections we aren't interested in, mark their entries with a
     value we can check later.  */
  list = input_list + top_index;
  do
    *list = bfd_abs_section_ptr;
  while (list-- != input_list);

  for (section = output_bfd->sections;
       section != NULL;
       section = section->next)
    {
      /* FIXME: This is a bit of hack. Currently our .ctors and .dtors
       * have PC relative relocs in them but no code flag set.  */
      if (((section->flags & SEC_CODE) != 0) ||
	  strcmp(".ctors", section->name) ||
	  strcmp(".dtors", section->name))
	input_list[section->index] = NULL;
    }

  return 1;
}

/* The linker repeatedly calls this function for each input section,
   in the order that input sections are linked into output sections.
   Build lists of input sections to determine groupings between which
   we may insert linker stubs.  */
void
nios2_elf32_next_input_section (struct bfd_link_info *info, asection *isec)
{
  struct elf32_nios2_link_hash_table *htab = elf32_nios2_hash_table (info);

  if (isec->output_section->index <= htab->top_index)
    {
      asection **list = htab->input_list + isec->output_section->index;
      if (*list != bfd_abs_section_ptr)
	{
	  /* Steal the last_sec pointer for our list.
	     This happens to make the list in reverse order,
	     which is what we want.  */
	  htab->stub_group[isec->id].last_sec = *list;
	  *list = isec;
	}
    }
}

/* Segment mask for CALL26 relocation relaxation.  */
#define CALL26_SEGMENT(x) ((x) & 0xf0000000)

/* Fudge factor for approximate maximum size of all stubs that might
   be inserted by the linker.  This does not actually limit the number
   of stubs that might be inserted, and only affects strategy for grouping
   and placement of stubs.  Perhaps this should be computed based on number
   of relocations seen, or be specifiable on the command line.  */
#define MAX_STUB_SECTION_SIZE 0xffff

/* See whether we can group stub sections together.  Grouping stub
   sections may result in fewer stubs.  More importantly, we need to
   put all .init* and .fini* stubs at the end of the .init or
   .fini output sections respectively, because glibc splits the
   _init and _fini functions into multiple parts.  Putting a stub in
   the middle of a function is not a good idea.
   Rather than computing groups of a maximum fixed size, for Nios II
   CALL26 relaxation it makes more sense to compute the groups based on
   sections that fit within a 256MB address segment.  Also do not allow
   a group to span more than one output section, since different output
   sections might correspond to different memory banks on a bare-metal
   target, etc.  */
static void
group_sections (struct elf32_nios2_link_hash_table *htab)
{
  asection **list = htab->input_list + htab->top_index;
  do
    {
      /* The list is in reverse order so we'll search backwards looking
	 for the first section that begins in the same memory segment,
	 marking sections along the way to point at the tail for this
	 group.  */
      asection *tail = *list;
      if (tail == bfd_abs_section_ptr)
	continue;
      while (tail != NULL)
	{
	  bfd_vma start = tail->output_section->vma + tail->output_offset;
	  bfd_vma end = start + tail->size;
	  bfd_vma segment = CALL26_SEGMENT (end);
	  asection *prev;

	  if (segment != CALL26_SEGMENT (start)
	      || segment != CALL26_SEGMENT (end + MAX_STUB_SECTION_SIZE))
	    /* This section spans more than one memory segment, or is
	       close enough to the end of the segment that adding stub
	       sections before it might cause it to move so that it
	       spans memory segments, or that stubs added at the end of
	       this group might overflow into the next memory segment.
	       Put it in a group by itself to localize the effects.  */
	    {
	      prev = htab->stub_group[tail->id].last_sec;
	      htab->stub_group[tail->id].last_sec = tail;
	      htab->stub_group[tail->id].first_sec = tail;
	    }
	  else
	    /* Collect more sections for this group.  */
	    {
	      asection *curr, *first;
	      for (curr = tail; ; curr = prev)
		{
		  prev = htab->stub_group[curr->id].last_sec;
		  if (!prev
		      || tail->output_section != prev->output_section
		      || (CALL26_SEGMENT (prev->output_section->vma
					  + prev->output_offset)
			  != segment))
		    break;
		}
	      first = curr;
	      for (curr = tail; ; curr = prev)
		{
		  prev = htab->stub_group[curr->id].last_sec;
		  htab->stub_group[curr->id].last_sec = tail;
		  htab->stub_group[curr->id].first_sec = first;
		  if (curr == first)
		    break;
		}
	    }

	  /* Reset tail for the next group.  */
	  tail = prev;
	}
    }
  while (list-- != htab->input_list);
  free (htab->input_list);
}

/* Determine the type of stub needed, if any, for a call.  */
static enum elf32_nios2_stub_type
nios2_type_of_stub (asection *input_sec,
		    const Elf_Internal_Rela *rel,
		    struct elf32_nios2_link_hash_entry *hh,
		    struct elf32_nios2_link_hash_table *htab,
		    bfd_vma destination,
		    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  bfd_vma location, segment, start, end;
  asection *s0, *s1, *s;

  if (hh != NULL &&
      !(hh->root.root.type == bfd_link_hash_defined
	|| hh->root.root.type == bfd_link_hash_defweak))
    return nios2_stub_none;

  /* Determine where the call point is.  */
  location = (input_sec->output_section->vma
	      + input_sec->output_offset + rel->r_offset);
  segment = CALL26_SEGMENT (location);

  /* Nios II CALL and JMPI instructions can transfer control to addresses
     within the same 256MB segment as the PC.  */
  if (segment == CALL26_SEGMENT (destination))
    return nios2_stub_none;

  /* Find the start and end addresses of the stub group.  Also account for
     any already-created stub sections for this group.  Note that for stubs
     in the end section, only the first instruction of the last stub
     (12 bytes long) needs to be within range.  */
  s0 = htab->stub_group[input_sec->id].first_sec;
  s = htab->stub_group[s0->id].first_stub_sec;
  if (s != NULL && s->size > 0)
    start = s->output_section->vma + s->output_offset;
  else
    start = s0->output_section->vma + s0->output_offset;

  s1 = htab->stub_group[input_sec->id].last_sec;
  s = htab->stub_group[s1->id].last_stub_sec;
  if (s != NULL && s->size > 0)
    end = s->output_section->vma + s->output_offset + s->size - 8;
  else
    end = s1->output_section->vma + s1->output_offset + s1->size;

  BFD_ASSERT (start < end);
  BFD_ASSERT (start <= location);
  BFD_ASSERT (location < end);

  /* Put stubs at the end of the group unless that is not a valid
     location and the beginning of the group is.  It might be that
     neither the beginning nor end works if we have an input section
     so large that it spans multiple segment boundaries.  In that
     case, punt; the end result will be a relocation overflow error no
     matter what we do here.

     Note that adding stubs pushes up the addresses of all subsequent
     sections, so that stubs allocated on one pass through the
     relaxation loop may not be valid on the next pass.  (E.g., we may
     allocate a stub at the beginning of the section on one pass and
     find that the call site has been bumped into the next memory
     segment on the next pass.)  The important thing to note is that
     we never try to reclaim the space allocated to such unused stubs,
     so code size and section addresses can only increase with each
     iteration.  Accounting for the start and end addresses of the
     already-created stub sections ensures that when the algorithm
     converges, it converges accurately, with the entire appropriate
     stub section accessible from the call site and not just the
     address at the start or end of the stub group proper.  */

  if (segment == CALL26_SEGMENT (end))
    return nios2_stub_call26_after;
  else if (segment == CALL26_SEGMENT (start))
    return nios2_stub_call26_before;
  else
    /* Perhaps this should be a dedicated error code.  */
    return nios2_stub_none;
}

static bool
nios2_build_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg ATTRIBUTE_UNUSED)
{
  struct elf32_nios2_stub_hash_entry *hsh
    = (struct elf32_nios2_stub_hash_entry *) gen_entry;
  asection *stub_sec = hsh->stub_sec;
  bfd_vma sym_value;
  struct bfd_link_info *info;

  info = (struct bfd_link_info *) in_arg;

  /* Fail if the target section could not be assigned to an output
     section.  The user should fix his linker script.  */
  if (hsh->target_section->output_section == NULL
      && info->non_contiguous_regions)
    info->callbacks->einfo (_("%F%P: Could not assign `%pA' to an output section. "
			      "Retry without --enable-non-contiguous-regions.\n"),
			    hsh->target_section);

  /* Make a note of the offset within the stubs for this entry.  */
  hsh->stub_offset = stub_sec->size;

  switch (hsh->stub_type)
    {
    case nios2_stub_call26_before:
    case nios2_stub_call26_after:
      /* A call26 stub looks like:
	   orhi at, %hiadj(dest)
	   addi at, at, %lo(dest)
	   jmp at
	 Note that call/jmpi instructions can't be used in PIC code
	 so there is no reason for the stub to be PIC, either.  */
      sym_value = (hsh->target_value
		   + hsh->target_section->output_offset
		   + hsh->target_section->output_section->vma
		   + hsh->addend);

      nios2_elf32_install_data (stub_sec, nios2_call26_stub_entry,
				hsh->stub_offset, 3);
      nios2_elf32_install_imm16 (stub_sec, hsh->stub_offset,
				 hiadj (sym_value));
      nios2_elf32_install_imm16 (stub_sec, hsh->stub_offset + 4,
				 (sym_value & 0xffff));
      stub_sec->size += 12;
      break;
    default:
      BFD_FAIL ();
      return false;
    }

  return true;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */
static bool
nios2_size_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg ATTRIBUTE_UNUSED)
{
  struct elf32_nios2_stub_hash_entry *hsh
    = (struct elf32_nios2_stub_hash_entry *) gen_entry;

  switch (hsh->stub_type)
    {
    case nios2_stub_call26_before:
    case nios2_stub_call26_after:
      hsh->stub_sec->size += 12;
      break;
    default:
      BFD_FAIL ();
      return false;
    }
  return true;
}

/* Read in all local syms for all input bfds.
   Returns -1 on error, 0 otherwise.  */

static int
get_local_syms (bfd *output_bfd ATTRIBUTE_UNUSED, bfd *input_bfd,
		struct bfd_link_info *info)
{
  unsigned int bfd_indx;
  Elf_Internal_Sym *local_syms, **all_local_syms;
  struct elf32_nios2_link_hash_table *htab = elf32_nios2_hash_table (info);

  /* We want to read in symbol extension records only once.  To do this
     we need to read in the local symbols in parallel and save them for
     later use; so hold pointers to the local symbols in an array.  */
  size_t amt = sizeof (Elf_Internal_Sym *) * htab->bfd_count;
  all_local_syms = bfd_zmalloc (amt);
  htab->all_local_syms = all_local_syms;
  if (all_local_syms == NULL)
    return -1;

  /* Walk over all the input BFDs, swapping in local symbols.  */
  for (bfd_indx = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next, bfd_indx++)
    {
      Elf_Internal_Shdr *symtab_hdr;

      /* We'll need the symbol table in a second.  */
      symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
      if (symtab_hdr->sh_info == 0)
	continue;

      /* We need an array of the local symbols attached to the input bfd.  */
      local_syms = (Elf_Internal_Sym *) symtab_hdr->contents;
      if (local_syms == NULL)
	{
	  local_syms = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
					     symtab_hdr->sh_info, 0,
					     NULL, NULL, NULL);
	  /* Cache them for elf_link_input_bfd.  */
	  symtab_hdr->contents = (unsigned char *) local_syms;
	}
      if (local_syms == NULL)
	return -1;

      all_local_syms[bfd_indx] = local_syms;
    }

  return 0;
}

/* Determine and set the size of the stub section for a final link.  */
bool
nios2_elf32_size_stubs (bfd *output_bfd, bfd *stub_bfd,
			struct bfd_link_info *info,
			asection *(*add_stub_section) (const char *,
						       asection *, bool),
			void (*layout_sections_again) (void))
{
  bool stub_changed = false;
  struct elf32_nios2_link_hash_table *htab = elf32_nios2_hash_table (info);

  /* Stash our params away.  */
  htab->stub_bfd = stub_bfd;
  htab->add_stub_section = add_stub_section;
  htab->layout_sections_again = layout_sections_again;

  /* FIXME: We only compute the section groups once.  This could cause
     problems if adding a large stub section causes following sections,
     or parts of them, to move into another segment.  However, this seems
     to be consistent with the way other back ends handle this....  */
  group_sections (htab);

  if (get_local_syms (output_bfd, info->input_bfds, info))
    {
      if (htab->all_local_syms)
	goto error_ret_free_local;
      return false;
    }

  while (1)
    {
      bfd *input_bfd;
      unsigned int bfd_indx;
      asection *stub_sec;

      for (input_bfd = info->input_bfds, bfd_indx = 0;
	   input_bfd != NULL;
	   input_bfd = input_bfd->link.next, bfd_indx++)
	{
	  Elf_Internal_Shdr *symtab_hdr;
	  asection *section;
	  Elf_Internal_Sym *local_syms;

	  /* We'll need the symbol table in a second.  */
	  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
	  if (symtab_hdr->sh_info == 0)
	    continue;

	  local_syms = htab->all_local_syms[bfd_indx];

	  /* Walk over each section attached to the input bfd.  */
	  for (section = input_bfd->sections;
	       section != NULL;
	       section = section->next)
	    {
	      Elf_Internal_Rela *internal_relocs, *irelaend, *irela;

	      /* If there aren't any relocs, then there's nothing more
		 to do.  */
	      if ((section->flags & SEC_RELOC) == 0
		  || section->reloc_count == 0)
		continue;

	      /* If this section is a link-once section that will be
		 discarded, then don't create any stubs.  */
	      if (section->output_section == NULL
		  || section->output_section->owner != output_bfd)
		continue;

	      /* Get the relocs.  */
	      internal_relocs
		= _bfd_elf_link_read_relocs (input_bfd, section, NULL, NULL,
					     info->keep_memory);
	      if (internal_relocs == NULL)
		goto error_ret_free_local;

	      /* Now examine each relocation.  */
	      irela = internal_relocs;
	      irelaend = irela + section->reloc_count;
	      for (; irela < irelaend; irela++)
		{
		  unsigned int r_type, r_indx;
		  enum elf32_nios2_stub_type stub_type;
		  struct elf32_nios2_stub_hash_entry *hsh;
		  asection *sym_sec;
		  bfd_vma sym_value;
		  bfd_vma destination;
		  struct elf32_nios2_link_hash_entry *hh;
		  char *stub_name;
		  const asection *id_sec;

		  r_type = ELF32_R_TYPE (irela->r_info);
		  r_indx = ELF32_R_SYM (irela->r_info);

		  if (r_type >= (unsigned int) R_NIOS2_ILLEGAL)
		    {
		      bfd_set_error (bfd_error_bad_value);
		    error_ret_free_internal:
		      if (elf_section_data (section)->relocs == NULL)
			free (internal_relocs);
		      goto error_ret_free_local;
		    }

		  /* Only look for stubs on CALL and JMPI instructions.  */
		  if (r_type != (unsigned int) R_NIOS2_CALL26)
		    continue;

		  /* Now determine the call target, its name, value,
		     section.  */
		  sym_sec = NULL;
		  sym_value = 0;
		  destination = 0;
		  hh = NULL;
		  if (r_indx < symtab_hdr->sh_info)
		    {
		      /* It's a local symbol.  */
		      Elf_Internal_Sym *sym;
		      Elf_Internal_Shdr *hdr;
		      unsigned int shndx;

		      sym = local_syms + r_indx;
		      if (ELF_ST_TYPE (sym->st_info) != STT_SECTION)
			sym_value = sym->st_value;
		      shndx = sym->st_shndx;
		      if (shndx < elf_numsections (input_bfd))
			{
			  hdr = elf_elfsections (input_bfd)[shndx];
			  sym_sec = hdr->bfd_section;
			  destination = (sym_value + irela->r_addend
					 + sym_sec->output_offset
					 + sym_sec->output_section->vma);
			}
		    }
		  else
		    {
		      /* It's an external symbol.  */
		      int e_indx;

		      e_indx = r_indx - symtab_hdr->sh_info;
		      hh = ((struct elf32_nios2_link_hash_entry *)
			    elf_sym_hashes (input_bfd)[e_indx]);

		      while (hh->root.root.type == bfd_link_hash_indirect
			     || hh->root.root.type == bfd_link_hash_warning)
			hh = ((struct elf32_nios2_link_hash_entry *)
			      hh->root.root.u.i.link);

		      if (hh->root.root.type == bfd_link_hash_defined
			  || hh->root.root.type == bfd_link_hash_defweak)
			{
			  sym_sec = hh->root.root.u.def.section;
			  sym_value = hh->root.root.u.def.value;

			  if (sym_sec->output_section != NULL)
			    destination = (sym_value + irela->r_addend
					   + sym_sec->output_offset
					   + sym_sec->output_section->vma);
			  else
			    continue;
			}
		      else if (hh->root.root.type == bfd_link_hash_undefweak)
			{
			  if (! bfd_link_pic (info))
			    continue;
			}
		      else if (hh->root.root.type == bfd_link_hash_undefined)
			{
			  if (! (info->unresolved_syms_in_objects == RM_IGNORE
				 && (ELF_ST_VISIBILITY (hh->root.other)
				     == STV_DEFAULT)))
			    continue;
			}
		      else
			{
			  bfd_set_error (bfd_error_bad_value);
			  goto error_ret_free_internal;
			}
		    }

		  /* Determine what (if any) linker stub is needed.  */
		  stub_type = nios2_type_of_stub (section, irela, hh, htab,
						  destination, info);
		  if (stub_type == nios2_stub_none)
		    continue;

		  /* Support for grouping stub sections.  */
		  if (stub_type == nios2_stub_call26_before)
		    id_sec = htab->stub_group[section->id].first_sec;
		  else
		    id_sec = htab->stub_group[section->id].last_sec;

		  /* Get the name of this stub.  */
		  stub_name = nios2_stub_name (id_sec, sym_sec, hh, irela,
					       stub_type);
		  if (!stub_name)
		    goto error_ret_free_internal;

		  hsh = nios2_stub_hash_lookup (&htab->bstab,
						stub_name,
						false, false);
		  if (hsh != NULL)
		    {
		      /* The proper stub has already been created.  */
		      free (stub_name);
		      continue;
		    }

		  hsh = nios2_add_stub (stub_name, section, htab, stub_type);
		  if (hsh == NULL)
		    {
		      free (stub_name);
		      goto error_ret_free_internal;
		    }
		  hsh->target_value = sym_value;
		  hsh->target_section = sym_sec;
		  hsh->stub_type = stub_type;
		  hsh->hh = hh;
		  hsh->addend = irela->r_addend;
		  stub_changed = true;
		}

	      /* We're done with the internal relocs, free them.  */
	      if (elf_section_data (section)->relocs == NULL)
		free (internal_relocs);
	    }
	}

      if (!stub_changed)
	break;

      /* OK, we've added some stubs.  Find out the new size of the
	 stub sections.  */
      for (stub_sec = htab->stub_bfd->sections;
	   stub_sec != NULL;
	   stub_sec = stub_sec->next)
	stub_sec->size = 0;

      bfd_hash_traverse (&htab->bstab, nios2_size_one_stub, htab);

      /* Ask the linker to do its stuff.  */
      (*htab->layout_sections_again) ();
      stub_changed = false;
    }

  free (htab->all_local_syms);
  return true;

 error_ret_free_local:
  free (htab->all_local_syms);
  return false;
}

/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  This function is called via nios2elf_finish in the linker.  */
bool
nios2_elf32_build_stubs (struct bfd_link_info *info)
{
  asection *stub_sec;
  struct bfd_hash_table *table;
  struct elf32_nios2_link_hash_table *htab;

  htab = elf32_nios2_hash_table (info);

  for (stub_sec = htab->stub_bfd->sections;
       stub_sec != NULL;
       stub_sec = stub_sec->next)
    /* The stub_bfd may contain non-stub sections if it is also the
       dynobj.  Any such non-stub sections are created with the
       SEC_LINKER_CREATED flag set, while stub sections do not
       have that flag.  Ignore any non-stub sections here.  */
    if ((stub_sec->flags & SEC_LINKER_CREATED) == 0)
      {
	bfd_size_type size;

	/* Allocate memory to hold the linker stubs.  */
	size = stub_sec->size;
	stub_sec->contents = bfd_zalloc (htab->stub_bfd, size);
	if (stub_sec->contents == NULL && size != 0)
	  return false;
	stub_sec->size = 0;
      }

  /* Build the stubs as directed by the stub hash table.  */
  table = &htab->bstab;
  bfd_hash_traverse (table, nios2_build_one_stub, info);

  return true;
}


#define is_nios2_elf(bfd) \
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour \
   && elf_object_id (bfd) == NIOS2_ELF_DATA)

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
nios2_elf32_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword old_flags;
  flagword new_flags;

  if (!is_nios2_elf (ibfd) || !is_nios2_elf (obfd))
    return true;

  /* Check if we have the same endianness.  */
  if (! _bfd_generic_verify_endian_match (ibfd, info))
    return false;

  new_flags = elf_elfheader (ibfd)->e_flags;
  old_flags = elf_elfheader (obfd)->e_flags;
  if (!elf_flags_init (obfd))
    {
      /* First call, no flags set.  */
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = new_flags;

      switch (new_flags)
	{
	default:
	case EF_NIOS2_ARCH_R1:
	  bfd_default_set_arch_mach (obfd, bfd_arch_nios2, bfd_mach_nios2r1);
	  break;
	case EF_NIOS2_ARCH_R2:
	  if (bfd_big_endian (ibfd))
	    {
	      _bfd_error_handler
		(_("error: %pB: big-endian R2 is not supported"), ibfd);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  bfd_default_set_arch_mach (obfd, bfd_arch_nios2, bfd_mach_nios2r2);
	  break;
	}
    }

  /* Incompatible flags.  */
  else if (new_flags != old_flags)
    {
      /* So far, the only incompatible flags denote incompatible
	 architectures.  */
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB: conflicting CPU architectures %d/%d"),
	 ibfd, new_flags, old_flags);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* Merge Tag_compatibility attributes and any common GNU ones.  */
  _bfd_elf_merge_object_attributes (ibfd, info);

  return true;
}

/* Implement bfd_elf32_bfd_reloc_type_lookup:
   Given a BFD reloc type, return a howto structure.  */

static reloc_howto_type *
nios2_elf32_bfd_reloc_type_lookup (bfd *abfd,
				   bfd_reloc_code_real_type code)
{
  int i;

  for (i = 0; i < (int) ARRAY_SIZE (nios2_reloc_map); ++i)
    if (nios2_reloc_map[i].bfd_val == code)
      return lookup_howto (nios2_reloc_map[i].elf_val, abfd);
  return NULL;
}

/* Implement bfd_elf32_bfd_reloc_name_lookup:
   Given a reloc name, return a howto structure.  */

static reloc_howto_type *
nios2_elf32_bfd_reloc_name_lookup (bfd *abfd,
				   const char *r_name)
{
  int i;
  reloc_howto_type *howto_tbl;
  int howto_tbl_size;

  if (BFD_IS_R2 (abfd))
    {
      howto_tbl = elf_nios2_r2_howto_table_rel;
      howto_tbl_size = (int) ARRAY_SIZE (elf_nios2_r2_howto_table_rel);
    }
  else
    {
      howto_tbl = elf_nios2_r1_howto_table_rel;
      howto_tbl_size = (int) ARRAY_SIZE (elf_nios2_r1_howto_table_rel);
    }

  for (i = 0; i < howto_tbl_size; i++)
    if (howto_tbl[i].name && strcasecmp (howto_tbl[i].name, r_name) == 0)
      return howto_tbl + i;

  return NULL;
}

/* Implement elf_info_to_howto:
   Given a ELF32 relocation, fill in a arelent structure.  */

static bool
nios2_elf32_info_to_howto (bfd *abfd, arelent *cache_ptr,
			   Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if ((cache_ptr->howto = lookup_howto (r_type, abfd)) == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  return true;
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving @dtpoff relocation.
   This is PT_TLS segment p_vaddr.  */
static bfd_vma
dtpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma;
}

/* Return the relocation value for @tpoff relocation
   if STT_TLS virtual address is ADDRESS.  */
static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;
  return address - htab->tls_sec->vma;
}

/* Set the GP value for OUTPUT_BFD.  Returns FALSE if this is a
   dangerous relocation.  */
static bool
nios2_elf_assign_gp (bfd *output_bfd, bfd_vma *pgp, struct bfd_link_info *info)
{

  bool gp_found;
  struct bfd_hash_entry *h;
  struct bfd_link_hash_entry *lh;

  /* If we've already figured out what GP will be, just return it. */
  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp)
    return true;

  h = bfd_hash_lookup (&info->hash->table, "_gp", false, false);
  lh = (struct bfd_link_hash_entry *) h;
 lookup:
  if (lh)
    {
      switch (lh->type)
	{
	case bfd_link_hash_undefined:
	case bfd_link_hash_undefweak:
	case bfd_link_hash_common:
	  gp_found = false;
	  break;
	case bfd_link_hash_defined:
	case bfd_link_hash_defweak:
	  gp_found = true;
	  {
	    asection *sym_sec = lh->u.def.section;
	    bfd_vma sym_value = lh->u.def.value;

	    if (sym_sec->output_section)
	      sym_value = (sym_value + sym_sec->output_offset
			   + sym_sec->output_section->vma);
	    *pgp = sym_value;
	  }
	  break;
	case bfd_link_hash_indirect:
	case bfd_link_hash_warning:
	  lh = lh->u.i.link;
	  /* @@FIXME  ignoring warning for now */
	  goto lookup;
	case bfd_link_hash_new:
	default:
	  abort ();
	}
    }
  else
    gp_found = false;

  if (!gp_found)
    {
      /* Only get the error once. */
      *pgp = 4;
      _bfd_set_gp_value (output_bfd, *pgp);
      return false;
    }

  _bfd_set_gp_value (output_bfd, *pgp);

  return true;
}

/* Retrieve the previously cached _gp pointer, returning bfd_reloc_dangerous
   if it's not available as we don't have a link_info pointer available here
   to look it up in the output symbol table.  We don't need to adjust the
   symbol value for an external symbol if we are producing relocatable
   output.  */
static bfd_reloc_status_type
nios2_elf_final_gp (bfd *output_bfd, asymbol *symbol, bool relocatable,
		    char **error_message, bfd_vma *pgp)
{
  if (bfd_is_und_section (symbol->section) && !relocatable)
    {
      *pgp = 0;
      return bfd_reloc_undefined;
    }

  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp == 0 && (!relocatable || (symbol->flags & BSF_SECTION_SYM) != 0))
    {
      if (relocatable)
	{
	  /* Make up a value.  */
	  *pgp = symbol->section->output_section->vma + 0x4000;
	  _bfd_set_gp_value (output_bfd, *pgp);
	}
      else
	{
	  *error_message
	    = (char *) _("global pointer relative relocation when _gp not defined");
	  return bfd_reloc_dangerous;
	}
    }

  return bfd_reloc_ok;
}

/* Do the relocations that require special handling.  */
static bfd_reloc_status_type
nios2_elf32_do_hi16_relocate (bfd *abfd, reloc_howto_type *howto,
			      asection *input_section,
			      bfd_byte *data, bfd_vma offset,
			      bfd_vma symbol_value, bfd_vma addend)
{
  symbol_value = symbol_value + addend;
  addend = 0;
  symbol_value = (symbol_value >> 16) & 0xffff;
  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

static bfd_reloc_status_type
nios2_elf32_do_lo16_relocate (bfd *abfd, reloc_howto_type *howto,
			      asection *input_section,
			      bfd_byte *data, bfd_vma offset,
			      bfd_vma symbol_value, bfd_vma addend)
{
  symbol_value = symbol_value + addend;
  addend = 0;
  symbol_value = symbol_value & 0xffff;
  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

static bfd_reloc_status_type
nios2_elf32_do_hiadj16_relocate (bfd *abfd, reloc_howto_type *howto,
				 asection *input_section,
				 bfd_byte *data, bfd_vma offset,
				 bfd_vma symbol_value, bfd_vma addend)
{
  symbol_value = symbol_value + addend;
  addend = 0;
  symbol_value = hiadj(symbol_value);
  return _bfd_final_link_relocate (howto, abfd, input_section, data, offset,
				   symbol_value, addend);
}

static bfd_reloc_status_type
nios2_elf32_do_pcrel_lo16_relocate (bfd *abfd, reloc_howto_type *howto,
				    asection *input_section,
				    bfd_byte *data, bfd_vma offset,
				    bfd_vma symbol_value, bfd_vma addend)
{
  symbol_value = symbol_value + addend;
  addend = 0;
  symbol_value = symbol_value & 0xffff;
  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

static bfd_reloc_status_type
nios2_elf32_do_pcrel_hiadj16_relocate (bfd *abfd, reloc_howto_type *howto,
				       asection *input_section,
				       bfd_byte *data, bfd_vma offset,
				       bfd_vma symbol_value, bfd_vma addend)
{
  symbol_value = symbol_value + addend;
  symbol_value -= (input_section->output_section->vma
		   + input_section->output_offset);
  symbol_value -= offset;
  addend = 0;
  symbol_value = hiadj(symbol_value);
  return _bfd_final_link_relocate (howto, abfd, input_section, data, offset,
				   symbol_value, addend);
}

static bfd_reloc_status_type
nios2_elf32_do_pcrel16_relocate (bfd *abfd, reloc_howto_type *howto,
				 asection *input_section,
				 bfd_byte *data, bfd_vma offset,
				 bfd_vma symbol_value, bfd_vma addend)
{
  /* NIOS2 pc relative relocations are relative to the next 32-bit instruction
     so we need to subtract 4 before doing a final_link_relocate. */
  symbol_value = symbol_value + addend - 4;
  addend = 0;
  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

static bfd_reloc_status_type
nios2_elf32_do_call26_relocate (bfd *abfd, reloc_howto_type *howto,
				asection *input_section,
				bfd_byte *data, bfd_vma offset,
				bfd_vma symbol_value, bfd_vma addend)
{
  /* Check that the relocation is in the same page as the current address.  */
  if (CALL26_SEGMENT (symbol_value + addend)
      != CALL26_SEGMENT (input_section->output_section->vma
			 + input_section->output_offset
			 + offset))
    return bfd_reloc_overflow;

  /* Check that the target address is correctly aligned on a 4-byte
     boundary.  */
  if ((symbol_value + addend) & 0x3)
    return bfd_reloc_overflow;

  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

static bfd_reloc_status_type
nios2_elf32_do_gprel_relocate (bfd *abfd, reloc_howto_type *howto,
			       asection *input_section,
			       bfd_byte *data, bfd_vma offset,
			       bfd_vma symbol_value, bfd_vma addend)
{
  /* Because we need the output_bfd, the special handling is done
     in nios2_elf32_relocate_section or in nios2_elf32_gprel_relocate.  */
  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

static bfd_reloc_status_type
nios2_elf32_do_ujmp_relocate (bfd *abfd, reloc_howto_type *howto,
			      asection *input_section,
			      bfd_byte *data, bfd_vma offset,
			      bfd_vma symbol_value, bfd_vma addend)
{
  bfd_vma symbol_lo16, symbol_hi16;
  bfd_reloc_status_type r;
  symbol_value = symbol_value + addend;
  addend = 0;
  symbol_hi16 = (symbol_value >> 16) & 0xffff;
  symbol_lo16 = symbol_value & 0xffff;

  r = _bfd_final_link_relocate (howto, abfd, input_section,
				data, offset, symbol_hi16, addend);

  if (r == bfd_reloc_ok)
    return _bfd_final_link_relocate (howto, abfd, input_section,
				     data, offset + 4, symbol_lo16, addend);

  return r;
}

static bfd_reloc_status_type
nios2_elf32_do_cjmp_relocate (bfd *abfd, reloc_howto_type *howto,
			      asection *input_section,
			      bfd_byte *data, bfd_vma offset,
			      bfd_vma symbol_value, bfd_vma addend)
{
  bfd_vma symbol_lo16, symbol_hi16;
  bfd_reloc_status_type r;
  symbol_value = symbol_value + addend;
  addend = 0;
  symbol_hi16 = (symbol_value >> 16) & 0xffff;
  symbol_lo16 = symbol_value & 0xffff;

  r = _bfd_final_link_relocate (howto, abfd, input_section,
				data, offset, symbol_hi16, addend);

  if (r == bfd_reloc_ok)
    return _bfd_final_link_relocate (howto, abfd, input_section,
				     data, offset + 4, symbol_lo16, addend);

  return r;
}

static bfd_reloc_status_type
nios2_elf32_do_callr_relocate (bfd *abfd, reloc_howto_type *howto,
			       asection *input_section,
			       bfd_byte *data, bfd_vma offset,
			       bfd_vma symbol_value, bfd_vma addend)
{
  bfd_vma symbol_lo16, symbol_hi16;
  bfd_reloc_status_type r;
  symbol_value = symbol_value + addend;
  addend = 0;
  symbol_hi16 = (symbol_value >> 16) & 0xffff;
  symbol_lo16 = symbol_value & 0xffff;

  r = _bfd_final_link_relocate (howto, abfd, input_section,
				data, offset, symbol_hi16, addend);

  if (r == bfd_reloc_ok)
    return _bfd_final_link_relocate (howto, abfd, input_section,
				     data, offset + 4, symbol_lo16, addend);

  return r;
}

/* HOWTO handlers for relocations that require special handling.  */

/* This is for relocations used only when relaxing to ensure
   changes in size of section don't screw up .align.  */
static bfd_reloc_status_type
nios2_elf32_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED, arelent *reloc_entry,
			  asymbol *symbol ATTRIBUTE_UNUSED,
			  void *data ATTRIBUTE_UNUSED, asection *input_section,
			  bfd *output_bfd,
			  char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
nios2_elf32_hi16_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			   void *data, asection *input_section,
			   bfd *output_bfd,
			   char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_hi16_relocate (abfd, reloc_entry->howto,
				       input_section,
				       data, reloc_entry->address,
				       (symbol->value
					+ symbol->section->output_section->vma
					+ symbol->section->output_offset),
				       reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_lo16_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			   void *data, asection *input_section,
			   bfd *output_bfd,
			   char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_lo16_relocate (abfd, reloc_entry->howto,
				       input_section,
				       data, reloc_entry->address,
				       (symbol->value
					+ symbol->section->output_section->vma
					+ symbol->section->output_offset),
				       reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_hiadj16_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			      void *data, asection *input_section,
			      bfd *output_bfd,
			      char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_hiadj16_relocate (abfd, reloc_entry->howto,
					  input_section,
					  data, reloc_entry->address,
					  (symbol->value
					   + symbol->section->output_section->vma
					   + symbol->section->output_offset),
					  reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_pcrel_lo16_relocate (bfd *abfd, arelent *reloc_entry,
				 asymbol *symbol, void *data,
				 asection *input_section, bfd *output_bfd,
				 char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_pcrel_lo16_relocate (
    abfd, reloc_entry->howto, input_section, data, reloc_entry->address,
    (symbol->value + symbol->section->output_section->vma
     + symbol->section->output_offset),
    reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_pcrel_hiadj16_relocate (bfd *abfd, arelent *reloc_entry,
				    asymbol *symbol, void *data,
				    asection *input_section, bfd *output_bfd,
				    char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_pcrel_hiadj16_relocate (
    abfd, reloc_entry->howto, input_section, data, reloc_entry->address,
    (symbol->value + symbol->section->output_section->vma
     + symbol->section->output_offset),
    reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_pcrel16_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			      void *data, asection *input_section,
			      bfd *output_bfd,
			      char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_pcrel16_relocate (abfd, reloc_entry->howto,
					  input_section,
					  data, reloc_entry->address,
					  (symbol->value
					   + symbol->section->output_section->vma
					   + symbol->section->output_offset),
					  reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_call26_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			     void *data, asection *input_section,
			     bfd *output_bfd,
			     char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_call26_relocate (abfd, reloc_entry->howto,
					 input_section,
					 data, reloc_entry->address,
					 (symbol->value
					  + symbol->section->output_section->vma
					  + symbol->section->output_offset),
					 reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_gprel_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			    void *data, asection *input_section,
			    bfd *output_bfd, char **msg)
{
  bfd_vma relocation;
  bfd_vma gp;
  bfd_reloc_status_type r;


  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  relocation = (symbol->value
		+ symbol->section->output_section->vma
		+ symbol->section->output_offset);

  /* This assumes we've already cached the _gp symbol.  */
  r = nios2_elf_final_gp (abfd, symbol, false, msg, &gp);
  if (r == bfd_reloc_ok)
    {
      relocation = relocation + reloc_entry->addend - gp;
      reloc_entry->addend = 0;
      if ((signed) relocation < -32768 || (signed) relocation > 32767)
	{
	  *msg = _("global pointer relative address out of range");
	  r = bfd_reloc_outofrange;
	}
      else
	r = nios2_elf32_do_gprel_relocate (abfd, reloc_entry->howto,
					   input_section,
					   data, reloc_entry->address,
					   relocation, reloc_entry->addend);
    }

  return r;
}

static bfd_reloc_status_type
nios2_elf32_ujmp_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			   void *data, asection *input_section,
			   bfd *output_bfd, char **msg ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_ujmp_relocate (abfd, reloc_entry->howto,
				       input_section,
				       data, reloc_entry->address,
				       (symbol->value
					+ symbol->section->output_section->vma
					+ symbol->section->output_offset),
				       reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_cjmp_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			   void *data, asection *input_section,
			   bfd *output_bfd, char **msg ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_cjmp_relocate (abfd, reloc_entry->howto,
				       input_section,
				       data, reloc_entry->address,
				       (symbol->value
					+ symbol->section->output_section->vma
					+ symbol->section->output_offset),
				       reloc_entry->addend);
}

static bfd_reloc_status_type
nios2_elf32_callr_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			    void *data, asection *input_section,
			    bfd *output_bfd, char **msg ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  return nios2_elf32_do_callr_relocate (abfd, reloc_entry->howto,
					input_section,
					data, reloc_entry->address,
					(symbol->value
					 + symbol->section->output_section->vma
					 + symbol->section->output_offset),
					reloc_entry->addend);
}


/* Implement elf_backend_relocate_section.  */
static int
nios2_elf32_relocate_section (bfd *output_bfd,
			      struct bfd_link_info *info,
			      bfd *input_bfd,
			      asection *input_section,
			      bfd_byte *contents,
			      Elf_Internal_Rela *relocs,
			      Elf_Internal_Sym *local_syms,
			      asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  struct elf32_nios2_link_hash_table *htab;
  asection *sgot;
  asection *splt;
  asection *sreloc = NULL;
  bfd_vma *local_got_offsets;
  bfd_vma got_base;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

  htab = elf32_nios2_hash_table (info);
  sgot = htab->root.sgot;
  splt = htab->root.splt;
  local_got_offsets = elf_local_got_offsets (input_bfd);

  if (htab->h_gp_got == NULL)
    got_base = 0;
  else
    got_base = htab->h_gp_got->root.u.def.value;

  for (rel = relocs; rel < relend; rel++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      struct elf32_nios2_link_hash_entry *eh;
      bfd_vma relocation;
      bfd_vma gp;
      bfd_reloc_status_type r = bfd_reloc_ok;
      const char *name = NULL;
      int r_type;
      const char *format;
      char *msg = NULL;
      bool unresolved_reloc;
      bfd_vma off;
      int use_plt;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      howto = lookup_howto ((unsigned) ELF32_R_TYPE (rel->r_info), output_bfd);
      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	}

      if (sec && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      /* Nothing more to do unless this is a final link.  */
      if (bfd_link_relocatable (info))
	continue;

      if (howto)
	{
	  bool resolved_to_zero;

	  resolved_to_zero = (h != NULL
			      && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));
	  switch (howto->type)
	    {
	    case R_NIOS2_HI16:
	      r = nios2_elf32_do_hi16_relocate (input_bfd, howto,
						input_section,
						contents, rel->r_offset,
						relocation, rel->r_addend);
	      break;
	    case R_NIOS2_LO16:
	      r = nios2_elf32_do_lo16_relocate (input_bfd, howto,
						input_section,
						contents, rel->r_offset,
						relocation, rel->r_addend);
	      break;
	    case R_NIOS2_PCREL_LO:
	      r = nios2_elf32_do_pcrel_lo16_relocate (input_bfd, howto,
						      input_section,
						      contents,
						      rel->r_offset,
						      relocation,
						      rel->r_addend);
	      break;
	    case R_NIOS2_HIADJ16:
	      r = nios2_elf32_do_hiadj16_relocate (input_bfd, howto,
						   input_section, contents,
						   rel->r_offset, relocation,
						   rel->r_addend);
	      break;
	    case R_NIOS2_PCREL_HA:
	      r = nios2_elf32_do_pcrel_hiadj16_relocate (input_bfd, howto,
							 input_section,
							 contents,
							 rel->r_offset,
							 relocation,
							 rel->r_addend);
	      break;
	    case R_NIOS2_PCREL16:
	      r = nios2_elf32_do_pcrel16_relocate (input_bfd, howto,
						   input_section, contents,
						   rel->r_offset, relocation,
						   rel->r_addend);
	      break;
	    case R_NIOS2_GPREL:
	      /* Turns an absolute address into a gp-relative address.  */
	      if (!nios2_elf_assign_gp (output_bfd, &gp, info))
		{
		  bfd_vma reloc_address;

		  if (sec && sec->output_section)
		    reloc_address = (sec->output_section->vma
				     + sec->output_offset
				     + rel->r_offset);
		  else
		    reloc_address = 0;

		  format = _("global pointer relative relocation at address "
			     "%#" PRIx64 " when _gp not defined\n");
		  msg = bfd_asprintf (format, (uint64_t) reloc_address);
		  r = bfd_reloc_dangerous;
		}
	      else
		{
		  bfd_vma symbol_address = rel->r_addend + relocation;
		  relocation = symbol_address - gp;
		  rel->r_addend = 0;
		  if (((signed) relocation < -32768
		       || (signed) relocation > 32767)
		      && (!h
			  || h->root.type == bfd_link_hash_defined
			  || h->root.type == bfd_link_hash_defweak))
		    {
		      if (h)
			name = h->root.root.string;
		      else
			{
			  name = (bfd_elf_string_from_elf_section
				  (input_bfd, symtab_hdr->sh_link,
				   sym->st_name));
			  if (name == NULL || *name == '\0')
			    name = bfd_section_name (sec);
			}
		      /* xgettext:c-format */
		      format = _("unable to reach %s (at %#" PRIx64 ") from "
				 "the global pointer (at %#" PRIx64 ") "
				 "because the offset (%" PRId64 ") is out of "
				 "the allowed range, -32678 to 32767\n" );
		      msg = bfd_asprintf (format, name,
					  (uint64_t) symbol_address,
					  (uint64_t) gp,
					  (int64_t) relocation);
		      r = bfd_reloc_outofrange;
		    }
		  else
		    r =	_bfd_final_link_relocate (howto, input_bfd,
						  input_section, contents,
						  rel->r_offset, relocation,
						  rel->r_addend);
		}
	      break;
	    case R_NIOS2_UJMP:
	      r = nios2_elf32_do_ujmp_relocate (input_bfd, howto,
						input_section,
						contents, rel->r_offset,
						relocation, rel->r_addend);
	      break;
	    case R_NIOS2_CJMP:
	      r = nios2_elf32_do_cjmp_relocate (input_bfd, howto,
						input_section,
						contents, rel->r_offset,
						relocation, rel->r_addend);
	      break;
	    case R_NIOS2_CALLR:
	      r = nios2_elf32_do_callr_relocate (input_bfd, howto,
						 input_section, contents,
						 rel->r_offset, relocation,
						 rel->r_addend);
	      break;
	    case R_NIOS2_CALL26:
	    case R_NIOS2_CALL26_NOAT:
	      /* If we have a call to an undefined weak symbol, we just want
		 to stuff a zero in the bits of the call instruction and
		 bypass the normal call26 relocation handling, because it'll
		 diagnose an overflow error if address 0 isn't in the same
		 256MB segment as the call site.  Presumably the call
		 should be guarded by a null check anyway.  */
	      if (h != NULL && h->root.type == bfd_link_hash_undefweak)
		{
		  BFD_ASSERT (relocation == 0 && rel->r_addend == 0);
		  r = _bfd_final_link_relocate (howto, input_bfd,
						input_section, contents,
						rel->r_offset, relocation,
						rel->r_addend);
		  break;
		}
	      /* Handle relocations which should use the PLT entry.
		 NIOS2_BFD_RELOC_32 relocations will use the symbol's value,
		 which may point to a PLT entry, but we don't need to handle
		 that here.  If we created a PLT entry, all branches in this
		 object should go to it.  */
	      if (h != NULL && splt != NULL && h->plt.offset != (bfd_vma) -1)
		{
		  /* If we've created a .plt section, and assigned a PLT entry
		     to this function, it should not be known to bind locally.
		     If it were, we would have cleared the PLT entry.  */
		  BFD_ASSERT (!SYMBOL_CALLS_LOCAL (info, h));

		  relocation = (splt->output_section->vma
				+ splt->output_offset
				+ h->plt.offset);

		  unresolved_reloc = false;
		}
	      /* Detect R_NIOS2_CALL26 relocations that would overflow the
		 256MB segment.  Replace the target with a reference to a
		 trampoline instead.
		 Note that htab->stub_group is null if relaxation has been
		 disabled by the --no-relax linker command-line option, so
		 we can use that to skip this processing entirely.  */
	      if (howto->type == R_NIOS2_CALL26 && htab->stub_group)
		{
		  bfd_vma dest = relocation + rel->r_addend;
		  enum elf32_nios2_stub_type stub_type;

		  eh = (struct elf32_nios2_link_hash_entry *)h;
		  stub_type = nios2_type_of_stub (input_section, rel, eh,
						  htab, dest, NULL);

		  if (stub_type != nios2_stub_none)
		    {
		      struct elf32_nios2_stub_hash_entry *hsh;

		      hsh = nios2_get_stub_entry (input_section, sec,
						  eh, rel, htab, stub_type);
		      if (hsh == NULL)
			{
			  r = bfd_reloc_undefined;
			  break;
			}

		      dest = (hsh->stub_offset
			      + hsh->stub_sec->output_offset
			      + hsh->stub_sec->output_section->vma);
		      r = nios2_elf32_do_call26_relocate (input_bfd, howto,
							  input_section,
							  contents,
							  rel->r_offset,
							  dest, 0);
		      break;
		    }
		}

	      /* Normal case.  */
	      r = nios2_elf32_do_call26_relocate (input_bfd, howto,
						  input_section, contents,
						  rel->r_offset, relocation,
						  rel->r_addend);
	      break;
	    case R_NIOS2_ALIGN:
	      r = bfd_reloc_ok;
	      /* For symmetry this would be
		 r = nios2_elf32_do_ignore_reloc (input_bfd, howto,
						  input_section, contents,
						  rel->r_offset, relocation,
						  rel->r_addend);
		but do_ignore_reloc would do no more than return
		bfd_reloc_ok. */
	      break;

	    case R_NIOS2_GOT16:
	    case R_NIOS2_CALL16:
	    case R_NIOS2_GOT_LO:
	    case R_NIOS2_GOT_HA:
	    case R_NIOS2_CALL_LO:
	    case R_NIOS2_CALL_HA:
	      /* Relocation is to the entry for this symbol in the
		 global offset table.  */
	      if (sgot == NULL)
		{
		  r = bfd_reloc_notsupported;
		  break;
		}

	      use_plt = 0;

	      if (h != NULL)
		{
		  bool dyn;

		  eh = (struct elf32_nios2_link_hash_entry *)h;
		  use_plt = (eh->got_types_used == CALL_USED
			     && h->plt.offset != (bfd_vma) -1);

		  off = h->got.offset;
		  BFD_ASSERT (off != (bfd_vma) -1);
		  dyn = htab->root.dynamic_sections_created;
		  if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							 bfd_link_pic (info),
							 h)
		      || (bfd_link_pic (info)
			  && SYMBOL_REFERENCES_LOCAL (info, h))
		      || ((ELF_ST_VISIBILITY (h->other)
			   || resolved_to_zero)
			  && h->root.type == bfd_link_hash_undefweak))
		    {
		      /* This is actually a static link, or it is a -Bsymbolic
			 link and the symbol is defined locally.  We must
			 initialize this entry in the global offset table.
			 Since the offset must always be a multiple of 4, we
			 use the least significant bit to record whether we
			 have initialized it already.

			 When doing a dynamic link, we create a .rela.got
			 relocation entry to initialize the value.  This is
			 done in the finish_dynamic_symbol routine.  */
		      if ((off & 1) != 0)
			off &= ~1;
		      else
			{
			  bfd_put_32 (output_bfd, relocation,
				      sgot->contents + off);
			  h->got.offset |= 1;
			}
		    }
		  else
		    unresolved_reloc = false;
		}
	      else
		{
		  BFD_ASSERT (local_got_offsets != NULL
			      && local_got_offsets[r_symndx] != (bfd_vma) -1);

		  off = local_got_offsets[r_symndx];

		  /* The offset must always be a multiple of 4.  We use the
		     least significant bit to record whether we have already
		     generated the necessary reloc.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation,
				  sgot->contents + off);

		      if (bfd_link_pic (info))
			{
			  asection *srelgot;
			  Elf_Internal_Rela outrel;
			  bfd_byte *loc;

			  srelgot = htab->root.srelgot;
			  BFD_ASSERT (srelgot != NULL);

			  outrel.r_addend = relocation;
			  outrel.r_offset = (sgot->output_section->vma
					     + sgot->output_offset
					     + off);
			  outrel.r_info = ELF32_R_INFO (0, R_NIOS2_RELATIVE);
			  loc = srelgot->contents;
			  loc += (srelgot->reloc_count++ *
				  sizeof (Elf32_External_Rela));
			  bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
			}

		      local_got_offsets[r_symndx] |= 1;
		    }
		}

	      if (use_plt && bfd_link_pic (info))
		{
		  off = ((h->plt.offset - 24) / 12 + 3) * 4;
		  relocation = (htab->root.sgotplt->output_offset + off
				- got_base);
		}
	      else
		relocation = sgot->output_offset + off - got_base;

	      /* This relocation does not use the addend.  */
	      rel->r_addend = 0;

	      switch (howto->type)
		{
		case R_NIOS2_GOT_LO:
		case R_NIOS2_CALL_LO:
		  r = nios2_elf32_do_lo16_relocate (input_bfd, howto,
						    input_section, contents,
						    rel->r_offset, relocation,
						    rel->r_addend);
		  break;
		case R_NIOS2_GOT_HA:
		case R_NIOS2_CALL_HA:
		  r = nios2_elf32_do_hiadj16_relocate (input_bfd, howto,
						       input_section, contents,
						       rel->r_offset,
						       relocation,
						       rel->r_addend);
		  break;
		default:
		  r = _bfd_final_link_relocate (howto, input_bfd,
						input_section, contents,
						rel->r_offset, relocation,
						rel->r_addend);
		  break;
		}
	      break;

	    case R_NIOS2_GOTOFF_LO:
	    case R_NIOS2_GOTOFF_HA:
	    case R_NIOS2_GOTOFF:
	      /* Relocation is relative to the global offset table pointer.  */

	      BFD_ASSERT (sgot != NULL);
	      if (sgot == NULL)
		{
		  r = bfd_reloc_notsupported;
		  break;
		}

	      /* Note that sgot->output_offset is not involved in this
		 calculation.  We always want the start of .got.  */
	      relocation -= sgot->output_section->vma;

	      /* Now we adjust the relocation to be relative to the GOT pointer
		 (the _gp_got symbol), which possibly contains the 0x8000 bias.  */
	      relocation -= got_base;

	      switch (howto->type)
		{
		case R_NIOS2_GOTOFF_LO:
		  r = nios2_elf32_do_lo16_relocate (input_bfd, howto,
						    input_section, contents,
						    rel->r_offset, relocation,
						    rel->r_addend);
		  break;
		case R_NIOS2_GOTOFF_HA:
		  r = nios2_elf32_do_hiadj16_relocate (input_bfd, howto,
						       input_section, contents,
						       rel->r_offset,
						       relocation,
						       rel->r_addend);
		  break;
		default:
		  r = _bfd_final_link_relocate (howto, input_bfd,
						input_section, contents,
						rel->r_offset, relocation,
						rel->r_addend);
		  break;
		}
	      break;

	    case R_NIOS2_TLS_LDO16:
	      relocation -= dtpoff_base (info) + DTP_OFFSET;

	      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					    contents, rel->r_offset,
					    relocation, rel->r_addend);
	      break;
	    case R_NIOS2_TLS_LDM16:
	      if (htab->root.sgot == NULL)
		abort ();

	      off = htab->tls_ldm_got.offset;

	      if ((off & 1) != 0)
		off &= ~1;
	      else
		{
		  /* If we don't know the module number, create a relocation
		     for it.  */
		  if (bfd_link_pic (info))
		    {
		      Elf_Internal_Rela outrel;
		      bfd_byte *loc;

		      if (htab->root.srelgot == NULL)
			abort ();

		      outrel.r_addend = 0;
		      outrel.r_offset = (htab->root.sgot->output_section->vma
					 + htab->root.sgot->output_offset
					 + off);
		      outrel.r_info = ELF32_R_INFO (0, R_NIOS2_TLS_DTPMOD);

		      loc = htab->root.srelgot->contents;
		      loc += (htab->root.srelgot->reloc_count++
			      * sizeof (Elf32_External_Rela));
		      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		    }
		  else
		    bfd_put_32 (output_bfd, 1,
				htab->root.sgot->contents + off);

		  htab->tls_ldm_got.offset |= 1;
		}

	      relocation = htab->root.sgot->output_offset + off - got_base;

	      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					    contents, rel->r_offset,
					    relocation, rel->r_addend);

	      break;
	    case R_NIOS2_TLS_GD16:
	    case R_NIOS2_TLS_IE16:
	      {
		int indx;
		char tls_type;

		if (htab->root.sgot == NULL)
		  abort ();

		indx = 0;
		if (h != NULL)
		  {
		    bool dyn;
		    dyn = htab->root.dynamic_sections_created;
		    if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							 bfd_link_pic (info),
							 h)
			&& (!bfd_link_pic (info)
			    || !SYMBOL_REFERENCES_LOCAL (info, h)))
		      {
			unresolved_reloc = false;
			indx = h->dynindx;
		      }
		    off = h->got.offset;
		    tls_type = (((struct elf32_nios2_link_hash_entry *) h)
				->tls_type);
		  }
		else
		  {
		    if (local_got_offsets == NULL)
		      abort ();
		    off = local_got_offsets[r_symndx];
		    tls_type = (elf32_nios2_local_got_tls_type (input_bfd)
				[r_symndx]);
		  }

		if (tls_type == GOT_UNKNOWN)
		  abort ();

		if ((off & 1) != 0)
		  off &= ~1;
		else
		  {
		    bool need_relocs = false;
		    Elf_Internal_Rela outrel;
		    bfd_byte *loc = NULL;
		    int cur_off = off;

		    /* The GOT entries have not been initialized yet.  Do it
		       now, and emit any relocations.  If both an IE GOT and a
		       GD GOT are necessary, we emit the GD first.  */

		    if ((bfd_link_pic (info) || indx != 0)
			&& (h == NULL
			    || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
				&& !resolved_to_zero)
			    || h->root.type != bfd_link_hash_undefweak))
		      {
			need_relocs = true;
			if (htab->root.srelgot == NULL)
			  abort ();
			loc = htab->root.srelgot->contents;
			loc += (htab->root.srelgot->reloc_count *
				sizeof (Elf32_External_Rela));
		      }

		    if (tls_type & GOT_TLS_GD)
		      {
			if (need_relocs)
			  {
			    outrel.r_addend = 0;
			    outrel.r_offset = (htab->root.sgot->output_section->vma
					       + htab->root.sgot->output_offset
					       + cur_off);
			    outrel.r_info = ELF32_R_INFO (indx,
							  R_NIOS2_TLS_DTPMOD);

			    bfd_elf32_swap_reloca_out (output_bfd, &outrel,
						       loc);
			    htab->root.srelgot->reloc_count++;
			    loc += sizeof (Elf32_External_Rela);

			    if (indx == 0)
			      bfd_put_32 (output_bfd,
					  (relocation - dtpoff_base (info) -
					   DTP_OFFSET),
					  htab->root.sgot->contents + cur_off + 4);
			    else
			      {
				outrel.r_addend = 0;
				outrel.r_info = ELF32_R_INFO (indx,
				  R_NIOS2_TLS_DTPREL);
				outrel.r_offset += 4;

				bfd_elf32_swap_reloca_out (output_bfd, &outrel,
							   loc);
				htab->root.srelgot->reloc_count++;
				loc += sizeof (Elf32_External_Rela);
			      }
			  }
			else
			  {
			    /* If we are not emitting relocations for a
			       general dynamic reference, then we must be in a
			       static link or an executable link with the
			       symbol binding locally.  Mark it as belonging
			       to module 1, the executable.  */
			    bfd_put_32 (output_bfd, 1,
					htab->root.sgot->contents + cur_off);
			    bfd_put_32 (output_bfd, (relocation -
						     dtpoff_base (info) -
						     DTP_OFFSET),
					htab->root.sgot->contents + cur_off + 4);
			  }

			cur_off += 8;
		      }

		    if (tls_type & GOT_TLS_IE)
		      {
			if (need_relocs)
			  {
			    if (indx == 0)
			      outrel.r_addend = (relocation -
						 dtpoff_base (info));
			    else
			      outrel.r_addend = 0;
			    outrel.r_offset = (htab->root.sgot->output_section->vma
					       + htab->root.sgot->output_offset
					       + cur_off);
			    outrel.r_info = ELF32_R_INFO (indx,
							  R_NIOS2_TLS_TPREL);

			    bfd_elf32_swap_reloca_out (output_bfd, &outrel,
						       loc);
			    htab->root.srelgot->reloc_count++;
			    loc += sizeof (Elf32_External_Rela);
			  }
			else
			  bfd_put_32 (output_bfd, (tpoff (info, relocation)
						   - TP_OFFSET),
				      htab->root.sgot->contents + cur_off);
			cur_off += 4;
		      }

		    if (h != NULL)
		      h->got.offset |= 1;
		    else
		      local_got_offsets[r_symndx] |= 1;
		  }

		if ((tls_type & GOT_TLS_GD) && r_type != R_NIOS2_TLS_GD16)
		  off += 8;
		relocation = htab->root.sgot->output_offset + off - got_base;

		r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					      contents, rel->r_offset,
					      relocation, rel->r_addend);
	      }

	      break;
	    case R_NIOS2_TLS_LE16:
	      if (bfd_link_dll (info))
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB(%pA+%#" PRIx64 "): %s relocation not "
		       "permitted in shared object"),
		     input_bfd, input_section,
		     (uint64_t) rel->r_offset, howto->name);
		  return false;
		}
	      else
		relocation = tpoff (info, relocation) - TP_OFFSET;

	      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					    contents, rel->r_offset,
					    relocation, rel->r_addend);
	      break;

	    case R_NIOS2_BFD_RELOC_32:
	      if (bfd_link_pic (info)
		  && (input_section->flags & SEC_ALLOC) != 0
		  && (h == NULL
		      || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			  && !resolved_to_zero)
		      || h->root.type != bfd_link_hash_undefweak))
		{
		  Elf_Internal_Rela outrel;
		  bfd_byte *loc;
		  bool skip, relocate;

		  /* When generating a shared object, these relocations
		     are copied into the output file to be resolved at run
		     time.  */

		  skip = false;
		  relocate = false;

		  outrel.r_offset
		    = _bfd_elf_section_offset (output_bfd, info,
					       input_section, rel->r_offset);
		  if (outrel.r_offset == (bfd_vma) -1)
		    skip = true;
		  else if (outrel.r_offset == (bfd_vma) -2)
		    skip = true, relocate = true;
		  outrel.r_offset += (input_section->output_section->vma
				      + input_section->output_offset);

		  if (skip)
		    memset (&outrel, 0, sizeof outrel);
		  else if (h != NULL
			   && h->dynindx != -1
			   && (!bfd_link_pic (info)
			       || !SYMBOLIC_BIND (info, h)
			       || !h->def_regular))
		    {
		      outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		      outrel.r_addend = rel->r_addend;
		    }
		  else
		    {
		      /* This symbol is local, or marked to become local.  */
		      outrel.r_addend = relocation + rel->r_addend;
		      relocate = true;
		      outrel.r_info = ELF32_R_INFO (0, R_NIOS2_RELATIVE);
		    }

		  sreloc = elf_section_data (input_section)->sreloc;
		  if (sreloc == NULL)
		    abort ();

		  loc = sreloc->contents;
		  loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
		  bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

		  /* This reloc will be computed at runtime, so there's no
		     need to do anything now, except for R_NIOS2_BFD_RELOC_32
		     relocations that have been turned into
		     R_NIOS2_RELATIVE.  */
		  if (!relocate)
		    break;
		}

	      r = _bfd_final_link_relocate (howto, input_bfd,
					    input_section, contents,
					    rel->r_offset, relocation,
					    rel->r_addend);
	      break;

	    case R_NIOS2_TLS_DTPREL:
	      relocation -= dtpoff_base (info);
	      /* Fall through.  */

	    default:
	      r = _bfd_final_link_relocate (howto, input_bfd,
					    input_section, contents,
					    rel->r_offset, relocation,
					    rel->r_addend);
	      break;
	    }
	}
      else
	r = bfd_reloc_notsupported;

      if (r != bfd_reloc_ok)
	{
	  if (h != NULL)
	    name = h->root.root.string;
	  else
	    {
	      name = bfd_elf_string_from_elf_section (input_bfd,
						      symtab_hdr->sh_link,
						      sym->st_name);
	      if (name == NULL || *name == '\0')
		name = bfd_section_name (sec);
	    }

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow) (info, NULL, name,
						  howto->name, (bfd_vma) 0,
						  input_bfd, input_section,
						  rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol) (info, name, input_bfd,
						    input_section,
						    rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      if (msg == NULL)
		msg = _("relocation out of range");
	      break;

	    case bfd_reloc_notsupported:
	      if (msg == NULL)
		msg = _("unsupported relocation");
	      break;

	    case bfd_reloc_dangerous:
	      if (msg == NULL)
		msg = _("dangerous relocation");
	      break;

	    default:
	      if (msg == NULL)
		msg = _("unknown error");
	      break;
	    }

	  if (msg)
	    {
	      (*info->callbacks->warning) (info, msg, name, input_bfd,
					   input_section, rel->r_offset);
	      return false;
	    }
	}
    }
  return true;
}

/* Implement elf-backend_section_flags:
   Convert NIOS2 specific section flags to bfd internal section flags.  */
static bool
nios2_elf32_section_flags (const Elf_Internal_Shdr *hdr)
{
  if (hdr->sh_flags & SHF_NIOS2_GPREL)
    hdr->bfd_section->flags |= SEC_SMALL_DATA;

  return true;
}

/* Implement elf_backend_fake_sections:
   Set the correct type for an NIOS2 ELF section.  We do this by the
   section name, which is a hack, but ought to work.  */
static bool
nios2_elf32_fake_sections (bfd *abfd ATTRIBUTE_UNUSED,
			   Elf_Internal_Shdr *hdr, asection *sec)
{
  const char *name = bfd_section_name (sec);

  if ((sec->flags & SEC_SMALL_DATA)
      || strcmp (name, ".sdata") == 0
      || strcmp (name, ".sbss") == 0
      || strcmp (name, ".lit4") == 0 || strcmp (name, ".lit8") == 0)
    hdr->sh_flags |= SHF_NIOS2_GPREL;

  return true;
}

/* Create .got, .gotplt, and .rela.got sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */
static bool
create_got_section (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf32_nios2_link_hash_table *htab;
  struct elf_link_hash_entry *h;

  htab = elf32_nios2_hash_table (info);

  if (! _bfd_elf_create_got_section (dynobj, info))
    return false;

  /* In order for the two loads in .PLTresolve to share the same %hiadj,
     _GLOBAL_OFFSET_TABLE_ must be aligned to a 16-byte boundary.  */
  if (!bfd_set_section_alignment (htab->root.sgotplt, 4))
    return false;

  /* The Nios II ABI specifies that GOT-relative relocations are relative
     to the linker-created symbol _gp_got, rather than using
     _GLOBAL_OFFSET_TABLE_ directly.  In particular, the latter always
     points to the base of the GOT while _gp_got may include a bias.  */
  h = _bfd_elf_define_linkage_sym (dynobj, info, htab->root.sgotplt,
				   "_gp_got");
  htab->h_gp_got = h;
  if (h == NULL)
    return false;

  return true;
}

/* Implement elf_backend_create_dynamic_sections:
   Create .plt, .rela.plt, .got, .got.plt, .rela.got, .dynbss, and
   .rela.bss sections in DYNOBJ, and set up shortcuts to them in our
   hash table.  */
static bool
nios2_elf32_create_dynamic_sections (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf32_nios2_link_hash_table *htab;

  htab = elf32_nios2_hash_table (info);
  if (!htab->root.sgot && !create_got_section (dynobj, info))
    return false;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return false;

  /* In order for the two loads in a shared object .PLTresolve to share the
     same %hiadj, the start of the PLT (as well as the GOT) must be aligned
     to a 16-byte boundary.  This is because the addresses for these loads
     include the -(.plt+4) PIC correction.  */
  return bfd_set_section_alignment (htab->root.splt, 4);
}

/* Implement elf_backend_copy_indirect_symbol:
   Copy the extra info we tack onto an elf_link_hash_entry.  */
static void
nios2_elf32_copy_indirect_symbol (struct bfd_link_info *info,
				  struct elf_link_hash_entry *dir,
				  struct elf_link_hash_entry *ind)
{
  struct elf32_nios2_link_hash_entry *edir, *eind;

  edir = (struct elf32_nios2_link_hash_entry *) dir;
  eind = (struct elf32_nios2_link_hash_entry *) ind;

  if (ind->root.type == bfd_link_hash_indirect
      && dir->got.refcount <= 0)
    {
      edir->tls_type = eind->tls_type;
      eind->tls_type = GOT_UNKNOWN;
    }

  edir->got_types_used |= eind->got_types_used;

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Set the right machine number for a NIOS2 ELF file.  */

static bool
nios2_elf32_object_p (bfd *abfd)
{
  unsigned long mach;

  mach = elf_elfheader (abfd)->e_flags;

  switch (mach)
    {
    default:
    case EF_NIOS2_ARCH_R1:
      bfd_default_set_arch_mach (abfd, bfd_arch_nios2, bfd_mach_nios2r1);
      break;
    case EF_NIOS2_ARCH_R2:
      bfd_default_set_arch_mach (abfd, bfd_arch_nios2, bfd_mach_nios2r2);
      break;
    }

  return true;
}

/* Implement elf_backend_check_relocs:
   Look through the relocs for a section during the first phase.  */
static bool
nios2_elf32_check_relocs (bfd *abfd, struct bfd_link_info *info,
			  asection *sec, const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  struct elf32_nios2_link_hash_table *htab;
  asection *sreloc = NULL;
  bfd_signed_vma *local_got_refcounts;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  local_got_refcounts = elf_local_got_refcounts (abfd);

  htab = elf32_nios2_hash_table (info);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      r_type = ELF32_R_TYPE (rel->r_info);

      switch (r_type)
	{
	case R_NIOS2_GOT16:
	case R_NIOS2_GOT_LO:
	case R_NIOS2_GOT_HA:
	case R_NIOS2_CALL16:
	case R_NIOS2_CALL_LO:
	case R_NIOS2_CALL_HA:
	case R_NIOS2_TLS_GD16:
	case R_NIOS2_TLS_IE16:
	  /* This symbol requires a global offset table entry.  */
	  {
	    int tls_type, old_tls_type;

	    switch (r_type)
	      {
	      default:
	      case R_NIOS2_GOT16:
	      case R_NIOS2_GOT_LO:
	      case R_NIOS2_GOT_HA:
	      case R_NIOS2_CALL16:
	      case R_NIOS2_CALL_LO:
	      case R_NIOS2_CALL_HA:
		tls_type = GOT_NORMAL;
		break;
	      case R_NIOS2_TLS_GD16:
		tls_type = GOT_TLS_GD;
		break;
	      case R_NIOS2_TLS_IE16:
		tls_type = GOT_TLS_IE;
		break;
	      }

	    if (h != NULL)
	      {
		struct elf32_nios2_link_hash_entry *eh
		  = (struct elf32_nios2_link_hash_entry *)h;
		h->got.refcount++;
		old_tls_type = elf32_nios2_hash_entry(h)->tls_type;
		if (r_type == R_NIOS2_CALL16
		    || r_type == R_NIOS2_CALL_LO
		    || r_type == R_NIOS2_CALL_HA)
		  {
		    /* Make sure a plt entry is created for this symbol if
		       it turns out to be a function defined by a dynamic
		       object.  */
		    h->plt.refcount++;
		    h->needs_plt = 1;
		    h->type = STT_FUNC;
		    eh->got_types_used |= CALL_USED;
		  }
		else
		  eh->got_types_used |= GOT_USED;
	      }
	    else
	      {
		/* This is a global offset table entry for a local symbol.  */
		if (local_got_refcounts == NULL)
		  {
		    bfd_size_type size;

		    size = symtab_hdr->sh_info;
		    size *= (sizeof (bfd_signed_vma) + sizeof (char));
		    local_got_refcounts
		      = ((bfd_signed_vma *) bfd_zalloc (abfd, size));
		    if (local_got_refcounts == NULL)
		      return false;
		    elf_local_got_refcounts (abfd) = local_got_refcounts;
		    elf32_nios2_local_got_tls_type (abfd)
		      = (char *) (local_got_refcounts + symtab_hdr->sh_info);
		  }
		local_got_refcounts[r_symndx]++;
		old_tls_type = elf32_nios2_local_got_tls_type (abfd) [r_symndx];
	      }

	    /* We will already have issued an error message if there is a
	       TLS / non-TLS mismatch, based on the symbol type.  We don't
	       support any linker relaxations.  So just combine any TLS
	       types needed.  */
	    if (old_tls_type != GOT_UNKNOWN && old_tls_type != GOT_NORMAL
		&& tls_type != GOT_NORMAL)
	      tls_type |= old_tls_type;

	    if (old_tls_type != tls_type)
	      {
		if (h != NULL)
		  elf32_nios2_hash_entry (h)->tls_type = tls_type;
		else
		  elf32_nios2_local_got_tls_type (abfd) [r_symndx] = tls_type;
	      }
	  }
	make_got:
	  if (htab->root.sgot == NULL)
	    {
	      if (htab->root.dynobj == NULL)
		htab->root.dynobj = abfd;
	      if (!create_got_section (htab->root.dynobj, info))
		return false;
	    }
	  break;

	case R_NIOS2_TLS_LDM16:
	  htab->tls_ldm_got.refcount++;
	  goto make_got;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_NIOS2_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_NIOS2_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	case R_NIOS2_BFD_RELOC_32:
	case R_NIOS2_CALL26:
	case R_NIOS2_CALL26_NOAT:
	case R_NIOS2_HIADJ16:
	case R_NIOS2_LO16:

	  if (h != NULL)
	    {
	      /* If this reloc is in a read-only section, we might
		   need a copy reloc.  We can't check reliably at this
		   stage whether the section is read-only, as input
		   sections have not yet been mapped to output sections.
		   Tentatively set the flag for now, and correct in
		   adjust_dynamic_symbol.  */
	      if (!bfd_link_pic (info))
		h->non_got_ref = 1;

	      /* Make sure a plt entry is created for this symbol if it
		 turns out to be a function defined by a dynamic object.  */
	      h->plt.refcount++;

	      if (r_type == R_NIOS2_CALL26 || r_type == R_NIOS2_CALL26_NOAT)
		h->needs_plt = 1;
	    }

	  /* If we are creating a shared library, we need to copy the
	     reloc into the shared library.  */
	  if (bfd_link_pic (info)
	      && (sec->flags & SEC_ALLOC) != 0
	      && (r_type == R_NIOS2_BFD_RELOC_32
		  || (h != NULL && ! h->needs_plt
		      && (! SYMBOLIC_BIND (info, h) || ! h->def_regular))))
	    {
	      struct elf_dyn_relocs *p;
	      struct elf_dyn_relocs **head;

	      /* When creating a shared object, we must copy these
		 reloc types into the output file.  We create a reloc
		 section in dynobj and make room for this reloc.  */
	      if (sreloc == NULL)
		{
		  if (htab->root.dynobj == NULL)
		    htab->root.dynobj = abfd;

		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->root.dynobj, 2, abfd, true);
		  if (sreloc == NULL)
		    return false;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		head = &h->dyn_relocs;
	      else
		{
		  /* Track dynamic relocs needed for local syms too.
		     We really need local syms available to do this
		     easily.  Oh well.  */

		  asection *s;
		  void *vpp;
		  Elf_Internal_Sym *isym;

		  isym = bfd_sym_from_r_symndx (&htab->root.sym_cache,
						abfd, r_symndx);
		  if (isym == NULL)
		    return false;

		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (s == NULL)
		    s = sec;

		  vpp = &elf_section_data (s)->local_dynrel;
		  head = (struct elf_dyn_relocs **) vpp;
		}

	      p = *head;
	      if (p == NULL || p->sec != sec)
		{
		  size_t amt = sizeof *p;
		  p = ((struct elf_dyn_relocs *)
		       bfd_alloc (htab->root.dynobj, amt));
		  if (p == NULL)
		    return false;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;

	    }
	  break;
	}
    }

  return true;
}


/* Implement elf_backend_gc_mark_hook:
   Return the section that should be marked against GC for a given
   relocation.  */
static asection *
nios2_elf32_gc_mark_hook (asection *sec,
			  struct bfd_link_info *info,
			  Elf_Internal_Rela *rel,
			  struct elf_link_hash_entry *h,
			  Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_NIOS2_GNU_VTINHERIT:
      case R_NIOS2_GNU_VTENTRY:
	return NULL;
      }
  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Implement elf_backend_finish_dynamic_symbols:
   Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */
static bool
nios2_elf32_finish_dynamic_symbol (bfd *output_bfd,
				   struct bfd_link_info *info,
				   struct elf_link_hash_entry *h,
				   Elf_Internal_Sym *sym)
{
  struct elf32_nios2_link_hash_table *htab;
  struct elf32_nios2_link_hash_entry *eh
    = (struct elf32_nios2_link_hash_entry *)h;
  int use_plt;

  htab = elf32_nios2_hash_table (info);

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *sgotplt;
      asection *srela;
      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      bfd_vma got_address;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */
      BFD_ASSERT (h->dynindx != -1);
      splt = htab->root.splt;
      sgotplt = htab->root.sgotplt;
      srela = htab->root.srelplt;
      BFD_ASSERT (splt != NULL && sgotplt != NULL && srela != NULL);

      /* Emit the PLT entry.  */
      if (bfd_link_pic (info))
	{
	  nios2_elf32_install_data (splt, nios2_so_plt_entry, h->plt.offset,
				    3);
	  plt_index = (h->plt.offset - 24) / 12;
	  got_offset = (plt_index + 3) * 4;
	  nios2_elf32_install_imm16 (splt, h->plt.offset,
				     hiadj(plt_index * 4));
	  nios2_elf32_install_imm16 (splt, h->plt.offset + 4,
				     (plt_index * 4) & 0xffff);
	  nios2_elf32_install_imm16 (splt, h->plt.offset + 8,
				     0xfff4 - h->plt.offset);
	  got_address = (sgotplt->output_section->vma + sgotplt->output_offset
			 + got_offset);

	  /* Fill in the entry in the global offset table.  There are no
	     res_n slots for a shared object PLT, instead the .got.plt entries
	     point to the PLT entries.  */
	  bfd_put_32 (output_bfd,
		      splt->output_section->vma + splt->output_offset
		      + h->plt.offset, sgotplt->contents + got_offset);
	}
      else
	{
	  plt_index = (h->plt.offset - 28 - htab->res_n_size) / 12;
	  got_offset = (plt_index + 3) * 4;

	  nios2_elf32_install_data (splt, nios2_plt_entry, h->plt.offset, 3);
	  got_address = (sgotplt->output_section->vma + sgotplt->output_offset
			 + got_offset);
	  nios2_elf32_install_imm16 (splt, h->plt.offset, hiadj(got_address));
	  nios2_elf32_install_imm16 (splt, h->plt.offset + 4,
				     got_address & 0xffff);

	  /* Fill in the entry in the global offset table.  */
	  bfd_put_32 (output_bfd,
		      splt->output_section->vma + splt->output_offset
		      + plt_index * 4, sgotplt->contents + got_offset);
	}

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = got_address;
      rela.r_info = ELF32_R_INFO (h->dynindx, R_NIOS2_JUMP_SLOT);
      rela.r_addend = 0;
      loc = srela->contents + plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	  /* If the symbol is weak, we do need to clear the value.
	     Otherwise, the PLT entry would provide a definition for
	     the symbol even if the symbol wasn't defined anywhere,
	     and so the symbol would never be NULL.  */
	  if (!h->ref_regular_nonweak)
	    sym->st_value = 0;
	}
    }

  use_plt = (eh->got_types_used == CALL_USED
	     && h->plt.offset != (bfd_vma) -1);

  if (!use_plt && h->got.offset != (bfd_vma) -1
      && (elf32_nios2_hash_entry (h)->tls_type & GOT_TLS_GD) == 0
      && (elf32_nios2_hash_entry (h)->tls_type & GOT_TLS_IE) == 0)
    {
      asection *sgot;
      asection *srela;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      bfd_vma offset;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */
      sgot = htab->root.sgot;
      srela = htab->root.srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      offset = (h->got.offset & ~(bfd_vma) 1);
      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset + offset);

      /* If this is a -Bsymbolic link, and the symbol is defined
	 locally, we just want to emit a RELATIVE reloc.  Likewise if
	 the symbol was forced to be local because of a version file.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */

      if (bfd_link_pic (info) && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  rela.r_info = ELF32_R_INFO (0, R_NIOS2_RELATIVE);
	  rela.r_addend = bfd_get_signed_32 (output_bfd,
					     (sgot->contents + offset));
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + offset);
	}
      else
	{
	  bfd_put_32 (output_bfd, (bfd_vma) 0,
		      sgot->contents + offset);
	  rela.r_info = ELF32_R_INFO (h->dynindx, R_NIOS2_GLOB_DAT);
	  rela.r_addend = 0;
	}

      loc = srela->contents;
      loc += srela->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  if (use_plt && h->got.offset != (bfd_vma) -1)
    {
      bfd_vma offset = (h->got.offset & ~(bfd_vma) 1);
      asection *sgot = htab->root.sgot;
      asection *splt = htab->root.splt;
      bfd_put_32 (output_bfd, (splt->output_section->vma + splt->output_offset
			       + h->plt.offset),
		  sgot->contents + offset);
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol needs a copy reloc.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_NIOS2_COPY);
      rela.r_addend = 0;
      if (h->root.u.def.section == htab->root.sdynrelro)
	s = htab->root.sreldynrelro;
      else
	s = htab->root.srelbss;
      BFD_ASSERT (s != NULL);
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  /* Mark _DYNAMIC, _GLOBAL_OFFSET_TABLE_, and _gp_got as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || h == htab->root.hgot
      || h == htab->h_gp_got)
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Implement elf_backend_finish_dynamic_sections.  */
static bool
nios2_elf32_finish_dynamic_sections (bfd *output_bfd,
				     struct bfd_link_info *info)
{
  asection *sgotplt;
  asection *sdyn;
  struct elf32_nios2_link_hash_table *htab;

  htab = elf32_nios2_hash_table (info);
  sgotplt = htab->root.sgotplt;
  sdyn = NULL;

  if (htab->root.dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      splt = htab->root.splt;
      sdyn = bfd_get_linker_section (htab->root.dynobj, ".dynamic");
      BFD_ASSERT (splt != NULL && sdyn != NULL && sgotplt != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elf32_swap_dyn_in (htab->root.dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      break;

	    case DT_PLTGOT:
	      s = htab->root.sgotplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_JMPREL:
	      s = htab->root.srelplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = htab->root.srelplt;
	      dyn.d_un.d_val = s->size;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_NIOS2_GP:
	      s = htab->root.sgotplt;
	      dyn.d_un.d_ptr
		= s->output_section->vma + s->output_offset + 0x7ff0;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;
	    }
	}

      /* Fill in the first entry in the procedure linkage table.  */
      if (splt->size > 0)
	{
	  bfd_vma got_address = (sgotplt->output_section->vma
				 + sgotplt->output_offset);
	  if (bfd_link_pic (info))
	    {
	      bfd_vma got_pcrel = got_address - (splt->output_section->vma
						 + splt->output_offset);
	      /* Both GOT and PLT must be aligned to a 16-byte boundary
		 for the two loads to share the %hiadj part.  The 4-byte
		 offset for nextpc is accounted for in the %lo offsets
		 on the loads.  */
	      BFD_ASSERT ((got_pcrel & 0xf) == 0);
	      nios2_elf32_install_data (splt, nios2_so_plt0_entry, 0, 6);
	      nios2_elf32_install_imm16 (splt, 4, hiadj (got_pcrel));
	      nios2_elf32_install_imm16 (splt, 12, got_pcrel & 0xffff);
	      nios2_elf32_install_imm16 (splt, 16, (got_pcrel + 4) & 0xffff);
	    }
	  else
	    {
	      /* Divide by 4 here, not 3 because we already corrected for the
		 res_N branches.  */
	      bfd_vma res_size = (splt->size - 28) / 4;
	      bfd_vma res_start = (splt->output_section->vma
				   + splt->output_offset);
	      bfd_vma res_offset;

	      for (res_offset = 0; res_offset < res_size; res_offset += 4)
		bfd_put_32 (output_bfd,
			    6 | ((res_size - (res_offset + 4)) << 6),
			    splt->contents + res_offset);

	      /* The GOT must be aligned to a 16-byte boundary for the
		 two loads to share the same %hiadj part.  */
	      BFD_ASSERT ((got_address & 0xf) == 0);

	      nios2_elf32_install_data (splt, nios2_plt0_entry, res_size, 7);
	      nios2_elf32_install_imm16 (splt, res_size, hiadj (res_start));
	      nios2_elf32_install_imm16 (splt, res_size + 4,
					 res_start & 0xffff);
	      nios2_elf32_install_imm16 (splt, res_size + 12,
					 hiadj (got_address));
	      nios2_elf32_install_imm16 (splt, res_size + 16,
					 (got_address + 4) & 0xffff);
	      nios2_elf32_install_imm16 (splt, res_size + 20,
					 (got_address + 8) & 0xffff);
	    }
	}
    }

  /* Fill in the first three entries in the global offset table.  */
  if (sgotplt != NULL && sgotplt->size > 0)
    {
      if (sdyn == NULL)
	bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents);
      else
	bfd_put_32 (output_bfd,
		    sdyn->output_section->vma + sdyn->output_offset,
		    sgotplt->contents);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents + 4);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents + 8);

      if (sgotplt->output_section != bfd_abs_section_ptr)
	elf_section_data (sgotplt->output_section)->this_hdr.sh_entsize = 4;
    }

  return true;
}

/* Implement elf_backend_adjust_dynamic_symbol:
   Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */
static bool
nios2_elf32_adjust_dynamic_symbol (struct bfd_link_info *info,
				   struct elf_link_hash_entry *h)
{
  struct elf32_nios2_link_hash_table *htab;
  bfd *dynobj;
  asection *s, *srel;
  unsigned align2;

  htab = elf32_nios2_hash_table (info);
  dynobj = htab->root.dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
	      && (h->needs_plt
		  || h->is_weakalias
		  || (h->def_dynamic
		      && h->ref_regular
		      && !h->def_regular)));

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PCREL reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}

      return true;
    }

  /* Reinitialize the plt offset now that it is not used as a reference
     count any more.  */
  h->plt.offset = (bfd_vma) -1;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      h->root.u.def.section = def->root.u.def.section;
      h->root.u.def.value = def->root.u.def.value;
      return true;
    }

  /* If there are no non-GOT references, we do not need a copy
     relocation.  */
  if (!h->non_got_ref)
    return true;

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.
     If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (bfd_link_pic (info))
    return true;

  if (h->size == 0)
    {
      _bfd_error_handler (_("dynamic variable `%s' is zero size"),
			  h->root.root.string);
      return true;
    }

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */
  /* We must generate a R_NIOS2_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = htab->root.sdynrelro;
      srel = htab->root.sreldynrelro;
    }
  else
    {
      s = htab->root.sdynbss;
      srel = htab->root.srelbss;
    }
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  align2 = bfd_log2 (h->size);
  if (align2 > h->root.u.def.section->alignment_power)
    align2 = h->root.u.def.section->alignment_power;

  /* Align dynbss.  */
  s->size = BFD_ALIGN (s->size, (bfd_size_type)1 << align2);
  if (align2 > bfd_section_alignment (s)
      && !bfd_set_section_alignment (s, align2))
    return false;

  /* Define the symbol as being at this point in the section.  */
  h->root.u.def.section = s;
  h->root.u.def.value = s->size;

  /* Increment the section size to make room for the symbol.  */
  s->size += h->size;

  return true;
}

/* Worker function for nios2_elf32_size_dynamic_sections.  */
static bool
adjust_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct elf32_nios2_link_hash_table *htab;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) inf;
  htab = elf32_nios2_hash_table (info);

  if (h->plt.offset != (bfd_vma)-1)
    h->plt.offset += htab->res_n_size;
  if (htab->root.splt == h->root.u.def.section)
    h->root.u.def.value += htab->res_n_size;

  return true;
}

/* Another worker function for nios2_elf32_size_dynamic_sections.
   Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */
static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct elf32_nios2_link_hash_table *htab;
  struct elf32_nios2_link_hash_entry *eh;
  struct elf_dyn_relocs *p;
  int use_plt;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) inf;
  htab = elf32_nios2_hash_table (info);

  if (htab->root.dynamic_sections_created
      && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local
	  && !bfd_elf_link_record_dynamic_symbol (info, h))
	return false;

      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, bfd_link_pic (info), h))
	{
	  asection *s = htab->root.splt;

	  /* Allocate room for the header.  */
	  if (s->size == 0)
	    {
	      if (bfd_link_pic (info))
		s->size = 24;
	      else
		s->size = 28;
	    }

	  h->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (! bfd_link_pic (info)
	      && !h->def_regular)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  s->size += 12;

	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->root.srelplt->size += sizeof (Elf32_External_Rela);

	  /* And the .got.plt section.  */
	  htab->root.sgotplt->size += 4;
	}
      else
	{
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
    }

  eh = (struct elf32_nios2_link_hash_entry *) h;
  use_plt = (eh->got_types_used == CALL_USED
	     && h->plt.offset != (bfd_vma) -1);

  if (h->got.refcount > 0)
    {
      asection *s;
      bool dyn;
      int tls_type = eh->tls_type;
      int indx;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local
	  && !bfd_elf_link_record_dynamic_symbol (info, h))
	return false;

      s = htab->root.sgot;
      h->got.offset = s->size;

      if (tls_type == GOT_UNKNOWN)
	abort ();

      if (tls_type == GOT_NORMAL)
	/* Non-TLS symbols need one GOT slot.  */
	s->size += 4;
      else
	{
	  if (tls_type & GOT_TLS_GD)
	    /* R_NIOS2_TLS_GD16 needs 2 consecutive GOT slots.  */
	    s->size += 8;
	  if (tls_type & GOT_TLS_IE)
	    /* R_NIOS2_TLS_IE16 needs one GOT slot.  */
	    s->size += 4;
	}

      dyn = htab->root.dynamic_sections_created;

      indx = 0;
      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h)
	  && (!bfd_link_pic (info)
	      || !SYMBOL_REFERENCES_LOCAL (info, h)))
	indx = h->dynindx;

      if (tls_type != GOT_NORMAL
	  && (bfd_link_pic (info) || indx != 0)
	  && (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	      || h->root.type != bfd_link_hash_undefweak))
	{
	  if (tls_type & GOT_TLS_IE)
	    htab->root.srelgot->size += sizeof (Elf32_External_Rela);

	  if (tls_type & GOT_TLS_GD)
	    htab->root.srelgot->size += sizeof (Elf32_External_Rela);

	  if ((tls_type & GOT_TLS_GD) && indx != 0)
	    htab->root.srelgot->size += sizeof (Elf32_External_Rela);
	}
      else if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		|| h->root.type != bfd_link_hash_undefweak)
	       && !use_plt
	       && (bfd_link_pic (info)
		   || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	htab->root.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    h->got.offset = (bfd_vma) -1;

  if (h->dyn_relocs == NULL)
    return true;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (bfd_link_pic (info))
    {
      if (h->def_regular
	  && (h->forced_local || SYMBOLIC_BIND (info, h)))
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &h->dyn_relocs; (p = *pp) != NULL; )
	    {
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}

      /* Also discard relocs on undefined weak syms with non-default
	 visibility.  */
      if (h->dyn_relocs != NULL
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      || UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	    h->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1
		   && !h->forced_local
		   && !bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}
    }
  else
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic && !h->def_regular)
	      || (htab->root.dynamic_sections_created
		  && (h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined))))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1
	      && !h->forced_local
	      && !bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      h->dyn_relocs = NULL;

    keep: ;
    }

  /* Finally, allocate space.  */
  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Implement elf_backend_size_dynamic_sections:
   Set the sizes of the dynamic sections.  */
static bool
nios2_elf32_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				   struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;
  struct elf32_nios2_link_hash_table *htab;

  htab = elf32_nios2_hash_table (info);
  dynobj = htab->root.dynobj;
  BFD_ASSERT (dynobj != NULL);

  htab->res_n_size = 0;
  if (htab->root.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }
  else
    {
      /* We may have created entries in the .rela.got section.
	 However, if we are not creating the dynamic sections, we will
	 not actually use these entries.  Reset the size of .rela.got,
	 which will cause it to get stripped from the output file
	 below.  */
      s = htab->root.srelgot;
      if (s != NULL)
	s->size = 0;
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      char *local_tls_type;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = elf_section_data (s)->local_dynrel; p != NULL; p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (p->count != 0)
		{
		  srel = elf_section_data (p->sec)->sreloc;
		  srel->size += p->count * sizeof (Elf32_External_Rela);
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      local_tls_type = elf32_nios2_local_got_tls_type (ibfd);
      s = htab->root.sgot;
      srel = htab->root.srelgot;
      for (; local_got < end_local_got; ++local_got, ++local_tls_type)
	{
	  if (*local_got > 0)
	    {
	      *local_got = s->size;
	      if (*local_tls_type & GOT_TLS_GD)
		/* TLS_GD relocs need an 8-byte structure in the GOT.  */
		s->size += 8;
	      if (*local_tls_type & GOT_TLS_IE)
		s->size += 4;
	      if (*local_tls_type == GOT_NORMAL)
		s->size += 4;

	      if (bfd_link_pic (info) || *local_tls_type == GOT_TLS_GD)
		srel->size += sizeof (Elf32_External_Rela);
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  if (htab->tls_ldm_got.refcount > 0)
    {
      /* Allocate two GOT entries and one dynamic relocation (if necessary)
	 for R_NIOS2_TLS_LDM16 relocations.  */
      htab->tls_ldm_got.offset = htab->root.sgot->size;
      htab->root.sgot->size += 8;
      if (bfd_link_pic (info))
	htab->root.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    htab->tls_ldm_got.offset = -1;

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (& htab->root, allocate_dynrelocs, info);

  if (htab->root.dynamic_sections_created)
    {
      /* If the .got section is more than 0x8000 bytes, we add
	 0x8000 to the value of _gp_got, so that 16-bit relocations
	 have a greater chance of working. */
      if (htab->root.sgot->size >= 0x8000
	  && htab->h_gp_got->root.u.def.value == 0)
	htab->h_gp_got->root.u.def.value = 0x8000;
    }

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char *name;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      /* It's OK to base decisions on the section name, because none
	 of the dynobj section names depend upon the input files.  */
      name = bfd_section_name (s);

      if (startswith (name, ".rela"))
	{
	  if (s->size != 0)
	    {
	      if (s != htab->root.srelplt)
		relocs = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else if (s == htab->root.splt)
	{
	  /* Correct for the number of res_N branches.  */
	  if (s->size != 0 && !bfd_link_pic (info))
	    {
	      htab->res_n_size = (s->size - 28) / 3;
	      s->size += htab->res_n_size;
	    }
	}
      else if (s != htab->sbss
	       && s != htab->root.sgot
	       && s != htab->root.sgotplt
	       && s != htab->root.sdynbss
	       && s != htab->root.sdynrelro)
	/* It's not one of our sections, so don't allocate space.  */
	continue;

      if (s->size == 0)
	{
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  /* Adjust dynamic symbols that point to the plt to account for the
     now-known number of resN slots.  */
  if (htab->res_n_size)
    elf_link_hash_traverse (& htab->root, adjust_dynrelocs, info);

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* Free the derived linker hash table.  */
static void
nios2_elf32_link_hash_table_free (bfd *obfd)
{
  struct elf32_nios2_link_hash_table *htab
    = (struct elf32_nios2_link_hash_table *) obfd->link.hash;

  bfd_hash_table_free (&htab->bstab);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Implement bfd_elf32_bfd_link_hash_table_create.  */
static struct bfd_link_hash_table *
nios2_elf32_link_hash_table_create (bfd *abfd)
{
  struct elf32_nios2_link_hash_table *ret;
  size_t amt = sizeof (struct elf32_nios2_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      link_hash_newfunc,
				      sizeof (struct
					      elf32_nios2_link_hash_entry),
				      NIOS2_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  /* Init the stub hash table too.  */
  if (!bfd_hash_table_init (&ret->bstab, stub_hash_newfunc,
			    sizeof (struct elf32_nios2_stub_hash_entry)))
    {
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }
  ret->root.root.hash_table_free = nios2_elf32_link_hash_table_free;

  return &ret->root.root;
}

/* Implement elf_backend_reloc_type_class.  */
static enum elf_reloc_type_class
nios2_elf32_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			      const asection *rel_sec ATTRIBUTE_UNUSED,
			      const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_NIOS2_RELATIVE:
      return reloc_class_relative;
    case R_NIOS2_JUMP_SLOT:
      return reloc_class_plt;
    case R_NIOS2_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Return 1 if target is one of ours.  */
static bool
is_nios2_elf_target (const struct bfd_target *targ)
{
  return (targ == &nios2_elf32_le_vec
	  || targ == &nios2_elf32_be_vec);
}

/* Implement elf_backend_add_symbol_hook.
   This hook is called by the linker when adding symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */
static bool
nios2_elf_add_symbol_hook (bfd *abfd,
			   struct bfd_link_info *info,
			   Elf_Internal_Sym *sym,
			   const char **namep ATTRIBUTE_UNUSED,
			   flagword *flagsp ATTRIBUTE_UNUSED,
			   asection **secp,
			   bfd_vma *valp)
{
  if (sym->st_shndx == SHN_COMMON
      && !bfd_link_relocatable (info)
      && sym->st_size <= elf_gp_size (abfd)
      && is_nios2_elf_target (info->output_bfd->xvec))
    {
      /* Common symbols less than or equal to -G nn bytes are automatically
	 put into .sbss.  */
      struct elf32_nios2_link_hash_table *htab;

      htab = elf32_nios2_hash_table (info);
      if (htab->sbss == NULL)
	{
	  flagword flags = SEC_IS_COMMON | SEC_SMALL_DATA | SEC_LINKER_CREATED;

	  if (htab->root.dynobj == NULL)
	    htab->root.dynobj = abfd;

	  htab->sbss = bfd_make_section_anyway_with_flags (htab->root.dynobj,
							   ".sbss", flags);
	  if (htab->sbss == NULL)
	    return false;
	}

      *secp = htab->sbss;
      *valp = sym->st_size;
    }

  return true;
}

/* Implement elf_backend_can_make_relative_eh_frame:
   Decide whether to attempt to turn absptr or lsda encodings in
   shared libraries into pcrel within the given input section.  */
static bool
nios2_elf32_can_make_relative_eh_frame (bfd *input_bfd ATTRIBUTE_UNUSED,
					struct bfd_link_info *info
					ATTRIBUTE_UNUSED,
					asection *eh_frame_section
					ATTRIBUTE_UNUSED)
{
  /* We can't use PC-relative encodings in the .eh_frame section.  */
  return false;
}

/* Implement elf_backend_special_sections.  */
const struct bfd_elf_special_section elf32_nios2_special_sections[] =
{
  { STRING_COMMA_LEN (".sbss"),	 -2, SHT_NOBITS,
    SHF_ALLOC + SHF_WRITE + SHF_NIOS2_GPREL },
  { STRING_COMMA_LEN (".sdata"), -2, SHT_PROGBITS,
    SHF_ALLOC + SHF_WRITE + SHF_NIOS2_GPREL },
  { NULL,		      0,  0, 0,		     0 }
};

#define ELF_ARCH			bfd_arch_nios2
#define ELF_TARGET_ID			NIOS2_ELF_DATA
#define ELF_MACHINE_CODE		EM_ALTERA_NIOS2

/* The Nios II MMU uses a 4K page size.  */

#define ELF_MAXPAGESIZE			0x1000

#define bfd_elf32_bfd_link_hash_table_create \
					  nios2_elf32_link_hash_table_create

#define bfd_elf32_bfd_merge_private_bfd_data \
					  nios2_elf32_merge_private_bfd_data

/* Relocation table lookup macros.  */

#define bfd_elf32_bfd_reloc_type_lookup	  nios2_elf32_bfd_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup	  nios2_elf32_bfd_reloc_name_lookup

/* JUMP_TABLE_LINK macros.  */

/* elf_info_to_howto (using RELA relocations).  */

#define elf_info_to_howto		  nios2_elf32_info_to_howto

/* elf backend functions.  */

#define elf_backend_can_gc_sections	1
#define elf_backend_can_refcount	1
#define elf_backend_plt_readonly	1
#define elf_backend_want_got_plt	1
#define elf_backend_want_dynrelro	1
#define elf_backend_rela_normal		1
#define elf_backend_dtrel_excludes_plt	1

#define elf_backend_relocate_section	  nios2_elf32_relocate_section
#define elf_backend_section_flags	  nios2_elf32_section_flags
#define elf_backend_fake_sections	  nios2_elf32_fake_sections
#define elf_backend_check_relocs	  nios2_elf32_check_relocs

#define elf_backend_gc_mark_hook	  nios2_elf32_gc_mark_hook
#define elf_backend_create_dynamic_sections \
					  nios2_elf32_create_dynamic_sections
#define elf_backend_finish_dynamic_symbol nios2_elf32_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
					  nios2_elf32_finish_dynamic_sections
#define elf_backend_adjust_dynamic_symbol nios2_elf32_adjust_dynamic_symbol
#define elf_backend_reloc_type_class	  nios2_elf32_reloc_type_class
#define elf_backend_size_dynamic_sections nios2_elf32_size_dynamic_sections
#define elf_backend_add_symbol_hook	  nios2_elf_add_symbol_hook
#define elf_backend_copy_indirect_symbol  nios2_elf32_copy_indirect_symbol
#define elf_backend_object_p		  nios2_elf32_object_p

#define elf_backend_grok_prstatus	  nios2_grok_prstatus
#define elf_backend_grok_psinfo		  nios2_grok_psinfo

#undef elf_backend_can_make_relative_eh_frame
#define elf_backend_can_make_relative_eh_frame \
					  nios2_elf32_can_make_relative_eh_frame

#define elf_backend_special_sections	  elf32_nios2_special_sections

#define TARGET_LITTLE_SYM		nios2_elf32_le_vec
#define TARGET_LITTLE_NAME		"elf32-littlenios2"
#define TARGET_BIG_SYM			nios2_elf32_be_vec
#define TARGET_BIG_NAME			"elf32-bignios2"

#define elf_backend_got_header_size	12
#define elf_backend_default_execstack	0

#include "elf32-target.h"
