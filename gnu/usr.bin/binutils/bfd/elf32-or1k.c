/* Or1k-specific support for 32-bit ELF.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
   Contributed for OR32 by Johan Rydberg, jrydberg@opencores.org

   PIC parts added by Stefan Kristiansson, stefan.kristiansson@saunalahti.fi,
   largely based on elf32-m32r.c and elf32-microblaze.c.

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
   along with this program; if not, see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/or1k.h"
#include "libiberty.h"

#define N_ONES(X)	(((bfd_vma)2 << (X)) - 1)

#define PLT_ENTRY_SIZE 16
#define PLT_ENTRY_SIZE_LARGE (6*4)
#define PLT_MAX_INSN_COUNT 6

#define OR1K_MOVHI(D)		(0x18000000 | (D << 21))
#define OR1K_ADRP(D)		(0x08000000 | (D << 21))
#define OR1K_LWZ(D,A)		(0x84000000 | (D << 21) | (A << 16))
#define OR1K_ADD(D,A,B)		(0xE0000000 | (D << 21) | (A << 16) | (B << 11))
#define OR1K_ORI(D,A)		(0xA8000000 | (D << 21) | (A << 16))
#define OR1K_ORI0(D)		(0xA8000000 | (D << 21))
#define OR1K_JR(B)		(0x44000000 | (B << 11))
#define OR1K_NOP		0x15000000

#define ELF_DYNAMIC_INTERPRETER "/usr/lib/ld.so.1"

static reloc_howto_type or1k_elf_howto_table[] =
{
  /* This reloc does nothing.  */
  HOWTO (R_OR1K_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_NONE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_32",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_16,
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_16",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_8,
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_8",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_LO_16_IN_INSN, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_LO_16_IN_INSN", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_HI_16_IN_INSN, /* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_HI_16_IN_INSN", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A PC relative 26 bit relocation, right shifted by 2.  */
  HOWTO (R_OR1K_INSN_REL_26, /* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_INSN_REL_26", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x03ffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_OR1K_GNU_VTINHERIT, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_OR1K_GNU_VTINHERIT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_OR1K_GNU_VTENTRY, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_OR1K_GNU_VTENTRY", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_32_PCREL,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_32_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_OR1K_16_PCREL,
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_16_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_OR1K_8_PCREL,
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_8_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

   HOWTO (R_OR1K_GOTPC_HI16,	/* Type.  */
	 16,			/* Rightshift.  */
	 4,			/* Size.  */
	 16,			/* Bitsize.  */
	 true,			/* PC_relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont, /* Complain on overflow.  */
	 bfd_elf_generic_reloc, /* Special Function.  */
	 "R_OR1K_GOTPC_HI16",	/* Name.  */
	 false,		/* Partial Inplace.  */
	 0,			/* Source Mask.  */
	 0xffff,		/* Dest Mask.  */
	 true),			/* PC relative offset?  */

   HOWTO (R_OR1K_GOTPC_LO16,	/* Type.  */
	 0,			/* Rightshift.  */
	 4,			/* Size.  */
	 16,			/* Bitsize.  */
	 true,			/* PC_relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont, /* Complain on overflow.  */
	 bfd_elf_generic_reloc, /* Special Function.  */
	 "R_OR1K_GOTPC_LO16",	/* Name.  */
	 false,		/* Partial Inplace.  */
	 0,			/* Source Mask.  */
	 0xffff,		/* Dest Mask.  */
	 true),			/* PC relative offset?  */

  HOWTO (R_OR1K_GOT16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOT16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 26 bit PLT relocation.  Shifted by 2.  */
  HOWTO (R_OR1K_PLT26,		/* Type.  */
	 2,			/* Rightshift.  */
	 4,			/* Size.  */
	 26,			/* Bitsize.  */
	 true,			/* pc_relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_signed, /* Complain on overflow.  */
	 bfd_elf_generic_reloc, /* Special Function.  */
	 "R_OR1K_PLT26",	/* Name.  */
	 false,			/* Partial Inplace.  */
	 0,			/* Source Mask.  */
	 0x03ffffff,		/* Dest Mask.  */
	 true),			/* PC relative offset?  */

  HOWTO (R_OR1K_GOTOFF_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOTOFF_HI16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_GOTOFF_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOTOFF_LO16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_COPY,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_COPY",		/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GLOB_DAT",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_JMP_SLOT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_JMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_RELATIVE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_RELATIVE",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_GD_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_GD_HI16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_GD_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_GD_LO16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDM_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LDM_HI16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDM_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LDM_LO16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDO_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LDO_HI16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDO_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LDO_LO16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_IE_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_IE_HI16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_IE_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_IE_LO16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LE_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LE_HI16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LE_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LE_LO16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_TPOFF,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_TPOFF",    /* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_DTPOFF,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_DTPOFF",   /* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_DTPMOD,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_DTPMOD",   /* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_AHI16,		/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_AHI16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_GOTOFF_AHI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOTOFF_AHI16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_IE_AHI16,   /* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_IE_AHI16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LE_AHI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LE_AHI16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_SLO16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_SLO16",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_GOTOFF_SLO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOTOFF_SLO16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LE_SLO16,   /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LE_SLO16", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A page relative 21 bit relocation, right shifted by 13, aligned.
     Note that this is *page* relative, not pc relative.  The idea is
     similar, but normally the section alignment is not such that the
     assembler can infer a final value, which it attempts to do with
     pc-relative relocations to local symbols.  */
  HOWTO (R_OR1K_PCREL_PG21,    /* type */
	 13,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_PCREL_PG21",   /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x001fffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_OR1K_GOT_PG21,       /* type */
	 13,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOT_PG21",     /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x001fffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_OR1K_TLS_GD_PG21,    /* type */
	 13,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_GD_PG21",  /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x001fffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDM_PG21,   /* type */
	 13,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_LDM_PG21", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x001fffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_OR1K_TLS_IE_PG21,    /* type */
	 13,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_IE_PG21",  /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x001fffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_OR1K_LO13,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_LO13",		/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_GOT_LO13,       /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOT_LO13",     /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_GD_LO13,    /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_GD_LO13",  /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDM_LO13,   /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLD_LDM_LO13", /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_TLS_IE_LO13,    /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_TLS_IE_LO13",  /* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_OR1K_SLO13,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_SLO13",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 26 bit PLT relocation, using ADRP.  Shifted by 2.  */
  HOWTO (R_OR1K_PLTA26,		/* Type.  */
	 2,			/* Rightshift.  */
	 4,			/* Size.  */
	 26,			/* Bitsize.  */
	 true,			/* pc_relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_signed, /* Complain on overflow.  */
	 bfd_elf_generic_reloc,	/* Special Function.  */
	 "R_OR1K_PLTA26",	/* Name.  */
	 false,			/* Partial Inplace.  */
	 0,			/* Source Mask.  */
	 0x03ffffff,		/* Dest Mask.  */
	 true),			/* PC relative offset?  */

  HOWTO (R_OR1K_GOT_AHI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOT_AHI16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */
};

/* A copy of the R_OR1K_GOT16 used in the presense of R_OR1K_GOT_AHI16
   relocations when we know we can ignore overflows.  */
static reloc_howto_type or1k_elf_got16_no_overflow_howto =
  HOWTO (R_OR1K_GOT16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_OR1K_GOT16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false);		/* pcrel_offset */

/* Map BFD reloc types to Or1k ELF reloc types.  */

struct or1k_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int or1k_reloc_val;
};

static const struct or1k_reloc_map or1k_reloc_map[] =
{
  { BFD_RELOC_NONE,		R_OR1K_NONE },
  { BFD_RELOC_32,		R_OR1K_32 },
  { BFD_RELOC_16,		R_OR1K_16 },
  { BFD_RELOC_8,		R_OR1K_8 },
  { BFD_RELOC_LO16,		R_OR1K_LO_16_IN_INSN },
  { BFD_RELOC_HI16,		R_OR1K_HI_16_IN_INSN },
  { BFD_RELOC_HI16_S,		R_OR1K_AHI16 },
  { BFD_RELOC_OR1K_REL_26,	R_OR1K_INSN_REL_26 },
  { BFD_RELOC_VTABLE_ENTRY,	R_OR1K_GNU_VTENTRY },
  { BFD_RELOC_VTABLE_INHERIT,	R_OR1K_GNU_VTINHERIT },
  { BFD_RELOC_32_PCREL,		R_OR1K_32_PCREL },
  { BFD_RELOC_16_PCREL,		R_OR1K_16_PCREL },
  { BFD_RELOC_8_PCREL,		R_OR1K_8_PCREL },
  { BFD_RELOC_LO16_GOTOFF,	R_OR1K_GOTOFF_LO16 },
  { BFD_RELOC_HI16_GOTOFF,	R_OR1K_GOTOFF_HI16 },
  { BFD_RELOC_HI16_S_GOTOFF,	R_OR1K_GOTOFF_AHI16 },
  { BFD_RELOC_OR1K_GOTPC_HI16,	R_OR1K_GOTPC_HI16 },
  { BFD_RELOC_OR1K_GOTPC_LO16,	R_OR1K_GOTPC_LO16 },
  { BFD_RELOC_OR1K_GOT16,	R_OR1K_GOT16 },
  { BFD_RELOC_OR1K_PLT26,	R_OR1K_PLT26 },
  { BFD_RELOC_OR1K_GLOB_DAT,	R_OR1K_GLOB_DAT },
  { BFD_RELOC_OR1K_COPY,	R_OR1K_COPY },
  { BFD_RELOC_OR1K_JMP_SLOT,	R_OR1K_JMP_SLOT },
  { BFD_RELOC_OR1K_RELATIVE,	R_OR1K_RELATIVE },
  { BFD_RELOC_OR1K_TLS_GD_HI16, R_OR1K_TLS_GD_HI16 },
  { BFD_RELOC_OR1K_TLS_GD_LO16, R_OR1K_TLS_GD_LO16 },
  { BFD_RELOC_OR1K_TLS_LDM_HI16,	R_OR1K_TLS_LDM_HI16 },
  { BFD_RELOC_OR1K_TLS_LDM_LO16,	R_OR1K_TLS_LDM_LO16 },
  { BFD_RELOC_OR1K_TLS_LDO_HI16,	R_OR1K_TLS_LDO_HI16 },
  { BFD_RELOC_OR1K_TLS_LDO_LO16,	R_OR1K_TLS_LDO_LO16 },
  { BFD_RELOC_OR1K_TLS_IE_HI16, R_OR1K_TLS_IE_HI16 },
  { BFD_RELOC_OR1K_TLS_IE_LO16, R_OR1K_TLS_IE_LO16 },
  { BFD_RELOC_OR1K_TLS_IE_AHI16, R_OR1K_TLS_IE_AHI16 },
  { BFD_RELOC_OR1K_TLS_LE_HI16, R_OR1K_TLS_LE_HI16 },
  { BFD_RELOC_OR1K_TLS_LE_LO16, R_OR1K_TLS_LE_LO16 },
  { BFD_RELOC_OR1K_TLS_LE_AHI16, R_OR1K_TLS_LE_AHI16 },
  { BFD_RELOC_OR1K_SLO16,	R_OR1K_SLO16 },
  { BFD_RELOC_OR1K_GOTOFF_SLO16, R_OR1K_GOTOFF_SLO16 },
  { BFD_RELOC_OR1K_TLS_LE_SLO16, R_OR1K_TLS_LE_SLO16 },
  { BFD_RELOC_OR1K_PCREL_PG21,	R_OR1K_PCREL_PG21 },
  { BFD_RELOC_OR1K_GOT_PG21,	R_OR1K_GOT_PG21 },
  { BFD_RELOC_OR1K_TLS_GD_PG21,	R_OR1K_TLS_GD_PG21 },
  { BFD_RELOC_OR1K_TLS_LDM_PG21, R_OR1K_TLS_LDM_PG21 },
  { BFD_RELOC_OR1K_TLS_IE_PG21,	R_OR1K_TLS_IE_PG21 },
  { BFD_RELOC_OR1K_LO13,	R_OR1K_LO13 },
  { BFD_RELOC_OR1K_GOT_LO13,	R_OR1K_GOT_LO13 },
  { BFD_RELOC_OR1K_TLS_GD_LO13,	R_OR1K_TLS_GD_LO13 },
  { BFD_RELOC_OR1K_TLS_LDM_LO13, R_OR1K_TLS_LDM_LO13 },
  { BFD_RELOC_OR1K_TLS_IE_LO13,	R_OR1K_TLS_IE_LO13 },
  { BFD_RELOC_OR1K_SLO13,	R_OR1K_SLO13 },
  { BFD_RELOC_OR1K_PLTA26,	R_OR1K_PLTA26 },
  { BFD_RELOC_OR1K_GOT_AHI16,	R_OR1K_GOT_AHI16 },
};

/* tls_type is a mask used to track how each symbol is accessed,
   it may be accessed via multiple types of TLS access methods.
   We track this for sizing (allocating got + relocation section space) and
   for how to process relocations.  */
#define TLS_UNKNOWN    0
#define TLS_NONE       1
#define TLS_GD	       2
#define TLS_LD	       4
#define TLS_IE	       8
#define TLS_LE	      16

/* The size of the TLS thread control block, used to offset LE access.  */
#define TCB_SIZE      16

/* ELF linker hash entry.  */
struct elf_or1k_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* For calculating PLT size.  */
  bfd_vma plt_index;
  /* Track type of TLS access.  */
  unsigned char tls_type;
};

/* ELF object data.  */
struct elf_or1k_obj_tdata
{
  struct elf_obj_tdata root;

  /* tls_type for each local got entry.  */
  unsigned char *local_tls_type;
};

#define elf_or1k_tdata(abfd) \
  ((struct elf_or1k_obj_tdata *) (abfd)->tdata.any)

#define elf_or1k_local_tls_type(abfd) \
  (elf_or1k_tdata (abfd)->local_tls_type)

/* ELF linker hash table.  */
struct elf_or1k_link_hash_table
{
  struct elf_link_hash_table root;

  bfd_vma plt_count;
  bool saw_plta;
};

static size_t
elf_or1k_plt_entry_size (bfd_vma plt_index)
{
  bfd_vma plt_reloc;

  plt_reloc = plt_index * sizeof (Elf32_External_Rela);

  return (plt_reloc > 0xffff) ? PLT_ENTRY_SIZE_LARGE : PLT_ENTRY_SIZE;
}

/* Get the ELF linker hash table from a link_info structure.  */
#define or1k_elf_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == OR1K_ELF_DATA)		\
   ? (struct elf_or1k_link_hash_table *) (p)->hash : NULL)

static bool
elf_or1k_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_or1k_obj_tdata),
				  OR1K_ELF_DATA);
}

/* Create an entry in an or1k ELF linker hash table.  */

static struct bfd_hash_entry *
or1k_elf_link_hash_newfunc (struct bfd_hash_entry *entry,
			    struct bfd_hash_table *table,
			    const char *string)
{
  struct elf_or1k_link_hash_entry *ret =
    (struct elf_or1k_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table,
			     sizeof (struct elf_or1k_link_hash_entry));
  if (ret == NULL)
    return NULL;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_or1k_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != NULL)
    {
      struct elf_or1k_link_hash_entry *eh;

      eh = (struct elf_or1k_link_hash_entry *) ret;
      eh->tls_type = TLS_UNKNOWN;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Create an or1k ELF linker hash table.  */

static struct bfd_link_hash_table *
or1k_elf_link_hash_table_create (bfd *abfd)
{
  struct elf_or1k_link_hash_table *ret;
  size_t amt = sizeof (struct elf_or1k_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      or1k_elf_link_hash_newfunc,
				      sizeof (struct elf_or1k_link_hash_entry),
				      OR1K_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->root.root;
}

static reloc_howto_type *
or1k_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
			bfd_reloc_code_real_type bcode)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (or1k_reloc_map); i++)
    if (or1k_reloc_map[i].bfd_reloc_val == bcode)
      {
	unsigned int ocode = or1k_reloc_map[i].or1k_reloc_val;
	if (ocode < (unsigned int) R_OR1K_max)
	  return &or1k_elf_howto_table[ocode];
	else
	  break;
      }

  return NULL;
}

static reloc_howto_type *
or1k_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			const char *r_name)
{
  unsigned int i;

  for (i = 0; i < R_OR1K_max; i++)
    if (or1k_elf_howto_table[i].name != NULL
	&& strcasecmp (or1k_elf_howto_table[i].name, r_name) == 0)
      return &or1k_elf_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an Or1k ELF reloc.  */

static bool
or1k_info_to_howto_rela (bfd * abfd,
			 arelent * cache_ptr,
			 Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_OR1K_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = & or1k_elf_howto_table[r_type];
  return true;
}

/* Return the relocation value for @tpoff relocations..  */
static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address, bool dynamic)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);
  bfd_vma base;

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;

  if (dynamic)
    return address - htab->tls_sec->vma;
  else
    {
      /* On or1k, the tp points to just after the tcb, if we have an alignment
	 greater than the tcb size we need to offset by the alignment difference.  */
      base = align_power ((bfd_vma) TCB_SIZE, htab->tls_sec->alignment_power)
	     - TCB_SIZE;

      /* The thread pointer on or1k stores the address after the TCB where
	 the data is, just compute the difference. No need to compensate
	 for the size of TCB.  */
      return address - htab->tls_sec->vma + base;
    }
}

/* If we have both IE and GD accesses to a symbol the IE relocations should be
   offset by 8 bytes because the got contains both GD and IE entries.  */
static bfd_vma
or1k_initial_exec_offset (reloc_howto_type *howto, unsigned char tls_type_mask)
{
   switch (howto->type)
     {
     case R_OR1K_TLS_IE_HI16:
     case R_OR1K_TLS_IE_LO16:
     case R_OR1K_TLS_IE_PG21:
     case R_OR1K_TLS_IE_LO13:
     case R_OR1K_TLS_IE_AHI16:
       return (tls_type_mask & TLS_GD) != 0 ? 8 : 0;
     default:
       return 0;
     }
}

/* Like _bfd_final_link_relocate, but handles non-contiguous fields.  */

static bfd_reloc_status_type
or1k_final_link_relocate (reloc_howto_type *howto, bfd *input_bfd,
			  asection *input_section, bfd_byte *contents,
			  bfd_vma offset, bfd_vma value)
{
  bfd_reloc_status_type status = bfd_reloc_ok;
  int size = bfd_get_reloc_size (howto);
  bfd_vma x, place;

  /* Sanity check the address.  */
  if (offset + size > bfd_get_section_limit_octets (input_bfd, input_section))
    return bfd_reloc_outofrange;

  place = (input_section->output_section->vma
	   + input_section->output_offset
	   + (howto->pcrel_offset ? offset : 0));

  switch (howto->type)
    {
    case R_OR1K_AHI16:
    case R_OR1K_GOT_AHI16:
    case R_OR1K_GOTOFF_AHI16:
    case R_OR1K_TLS_IE_AHI16:
    case R_OR1K_TLS_LE_AHI16:
      /* Adjust the operand to match with a signed LO16.  */
      value += 0x8000;
      break;

    case R_OR1K_INSN_REL_26:
      value -= place;
      /* Diagnose mis-aligned branch targets.  */
      if (value & 3)
	status = bfd_reloc_dangerous;
      break;

    case R_OR1K_PCREL_PG21:
    case R_OR1K_GOT_PG21:
    case R_OR1K_TLS_GD_PG21:
    case R_OR1K_TLS_LDM_PG21:
    case R_OR1K_TLS_IE_PG21:
      value = (value & -8192) - (place & -8192);
      break;

    case R_OR1K_LO13:
    case R_OR1K_GOT_LO13:
    case R_OR1K_TLS_GD_LO13:
    case R_OR1K_TLS_LDM_LO13:
    case R_OR1K_TLS_IE_LO13:
    case R_OR1K_SLO13:
      value &= 8191;
      break;

    default:
      if (howto->pc_relative)
	value -= place;
      break;
    }

  status = bfd_check_overflow (howto->complain_on_overflow,
			       howto->bitsize,
			       howto->rightshift,
			       bfd_arch_bits_per_address (input_bfd),
			       value);
  value >>= howto->rightshift;

  /* If we're overwriting the entire destination,
     then no need to read the current contents.  */
  if (size == 0 || howto->dst_mask == N_ONES (size))
    x = 0;
  else
    {
      BFD_ASSERT (size == 4);
      x = bfd_get_32 (input_bfd, contents + offset);
    }

  switch (howto->type)
    {
    case R_OR1K_SLO16:
    case R_OR1K_GOTOFF_SLO16:
    case R_OR1K_TLS_LE_SLO16:
    case R_OR1K_SLO13:
      /* The split imm16 field used for stores.  */
      x = (x & ~0x3e007ff) | ((value & 0xf800) << 10) | (value & 0x7ff);
      break;

    default:
      {
	bfd_vma fieldmask = howto->dst_mask;
	value <<= howto->bitpos;
	x = (x & ~fieldmask) | (value & fieldmask);
      }
      break;
    }

  /* Put the relocated value back in the object file.  */
  switch (size)
    {
    case 0:
      break;
    case 1:
      bfd_put_8 (input_bfd, x, contents + offset);
      break;
    case 2:
      bfd_put_16 (input_bfd, x, contents + offset);
      break;
    case 4:
      bfd_put_32 (input_bfd, x, contents + offset);
      break;
#ifdef BFD64
    case 8:
      bfd_put_64 (input_bfd, x, contents + offset);
      break;
#endif
    default:
      _bfd_error_handler
	(_("%pB: Cannot handle relocation value size of %d"),
	 input_bfd, size);
      abort ();
    }
  return status;
}

/* Relocate an Or1k ELF section.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjusting the section contents as
   necessary, and (if using Rela relocs and generating a relocatable
   output file) adjusting the reloc addend as necessary.

   This function does not have to worry about setting the reloc
   address or the reloc symbol index.

   LOCAL_SYMS is a pointer to the swapped in local symbols.

   LOCAL_SECTIONS is an array giving the section in the input file
   corresponding to the st_shndx field of each local symbol.

   The global hash table entry for the global symbols can be found
   via elf_sym_hashes (input_bfd).

   When generating relocatable output, this function must handle
   STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
   going to be the section symbol corresponding to the output
   section, which means that the addend must be adjusted
   accordingly.  */

static int
or1k_elf_relocate_section (bfd *output_bfd,
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
  struct elf_or1k_link_hash_table *htab = or1k_elf_hash_table (info);
  asection *sreloc;
  bfd_vma *local_got_offsets;
  asection *sgot, *splt;
  bfd_vma plt_base, got_base, got_sym_value;
  bool ret_val = true;
  bool saw_gotha = false;

  if (htab == NULL)
    return false;

  local_got_offsets = elf_local_got_offsets (input_bfd);

  sreloc = elf_section_data (input_section)->sreloc;

  splt = htab->root.splt;
  plt_base = 0;
  if (splt != NULL)
    plt_base = splt->output_section->vma + splt->output_offset;

  sgot = htab->root.sgot;
  got_sym_value = got_base = 0;
  if (sgot != NULL)
    {
      struct elf_link_hash_entry *hgot = htab->root.hgot;
      got_sym_value = (hgot->root.u.def.value
		       + hgot->root.u.def.section->output_section->vma
		       + hgot->root.u.def.section->output_offset);
      got_base = sgot->output_section->vma + sgot->output_offset;
    }

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

  /* Make a full scan for R_OR1K_GOT_AHI16, since it could be AFTER R_OR1K_GOT16.  */
  for (rel = relocs; rel < relend; rel++)
    {
      int r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type==R_OR1K_GOT_AHI16)
        {
	  saw_gotha = true;
	  break;
        }
    }

  for (rel = relocs; rel < relend; rel++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name = NULL;
      int r_type;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      if (r_type == R_OR1K_GNU_VTINHERIT
	  || r_type == R_OR1K_GNU_VTENTRY)
	continue;

      if (r_type < 0 || r_type >= (int) R_OR1K_max)
	{
	  _bfd_error_handler
	    (_("%pB: unknown relocation type %d"),
	     input_bfd, (int) r_type);
	  bfd_set_error (bfd_error_bad_value);
	  ret_val = false;
	  continue;
	}

      howto = or1k_elf_howto_table + ELF32_R_TYPE (rel->r_info);
      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = name == NULL ? bfd_section_name (sec) : name;
	}
      else
	{
	  bool unresolved_reloc, warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	  name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      switch (howto->type)
	{
	case R_OR1K_PLT26:
	case R_OR1K_PLTA26:
	  /* If the call is not local, redirect the branch to the PLT.
	     Otherwise do nothing to send the branch to the symbol direct.  */
	  if (!SYMBOL_CALLS_LOCAL (info, h)
	      && h->plt.offset != (bfd_vma) -1)
	    relocation = plt_base + h->plt.offset;

	  /* Addend should be zero.  */
	  if (rel->r_addend != 0)
	    {
	      _bfd_error_handler
		(_("%pB: addend should be zero for plt relocations"),
		 input_bfd);
	      bfd_set_error (bfd_error_bad_value);
	      ret_val = false;
	    }
	  break;

	case R_OR1K_GOT_AHI16:
	case R_OR1K_GOT16:
	case R_OR1K_GOT_PG21:
	case R_OR1K_GOT_LO13:
	  {
	    bfd_vma off;

	    /* Relocation is to the entry for this symbol
	       in the global offset table.  */
	  BFD_ASSERT (sgot != NULL);
	  if (h != NULL)
	    {
	      bool dyn;

	      off = h->got.offset;
	      BFD_ASSERT (off != (bfd_vma) -1);

	      dyn = htab->root.dynamic_sections_created;
	      if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						     bfd_link_pic (info),
						     h)
		  || (bfd_link_pic (info)
		      && SYMBOL_REFERENCES_LOCAL (info, h)))
		{
		    /* This is actually a static link, or it is a -Bsymbolic
		       link and the symbol is defined locally, or the symbol
		       was forced to be local because of a version file.
		       We must initialize this entry in the GOT.  Since the
		       offset must always be a multiple of 4, we use the least
		       significant bit to record whether we have initialized
		       it already.

		     When doing a dynamic link, we create a .rela.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      /* Write entry in GOT.  */
		      bfd_put_32 (output_bfd, relocation,
				  sgot->contents + off);
		      /* Mark GOT entry as having been written.  */
		      h->got.offset |= 1;
		    }
		}
	    }
	  else
	    {
	      bfd_byte *loc;

	      BFD_ASSERT (local_got_offsets != NULL
			  && local_got_offsets[r_symndx] != (bfd_vma) -1);

	      /* Get offset into GOT table.  */
	      off = local_got_offsets[r_symndx];

	      /* The offset must always be a multiple of 4.  We use
		 the least significant bit to record whether we have
		 already processed this entry.  */
	      if ((off & 1) != 0)
		off &= ~1;
	      else
		{
		  /* Write entry in GOT.  */
		  bfd_put_32 (output_bfd, relocation, sgot->contents + off);
		  if (bfd_link_pic (info))
		    {
		      asection *srelgot;
		      Elf_Internal_Rela outrel;

		      /* We need to generate a R_OR1K_RELATIVE reloc
			 for the dynamic linker.  */
		      srelgot = htab->root.srelgot;
		      BFD_ASSERT (srelgot != NULL);

		      outrel.r_offset = got_base + off;
		      outrel.r_info = ELF32_R_INFO (0, R_OR1K_RELATIVE);
		      outrel.r_addend = relocation;
		      loc = srelgot->contents;
		      loc += (srelgot->reloc_count
			      * sizeof (Elf32_External_Rela));
		      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		      ++srelgot->reloc_count;
		    }
		  local_got_offsets[r_symndx] |= 1;
		}
	    }

	    /* The GOT_PG21 and GOT_LO13 relocs are pc-relative,
	       while the GOT16 reloc is GOT relative.  */
	    relocation = got_base + off;
	    if (r_type == R_OR1K_GOT16
		|| r_type == R_OR1K_GOT_AHI16)
	      relocation -= got_sym_value;

	    /* If we have a R_OR1K_GOT16 following a R_OR1K_GOT_AHI16
	       relocation we assume the code is doing the right thing to avoid
	       overflows.  */
	    if (r_type == R_OR1K_GOT16 && saw_gotha)
	      howto = &or1k_elf_got16_no_overflow_howto;

	  /* Addend should be zero.  */
	  if (rel->r_addend != 0)
	    {
	      _bfd_error_handler
		(_("%pB: addend should be zero for got relocations"),
		 input_bfd);
	      bfd_set_error (bfd_error_bad_value);
	      ret_val = false;
	    }
	  }
	  break;

	case R_OR1K_GOTOFF_LO16:
	case R_OR1K_GOTOFF_HI16:
	case R_OR1K_GOTOFF_AHI16:
	case R_OR1K_GOTOFF_SLO16:
	  /* Relocation is offset from GOT.  */
	  BFD_ASSERT (sgot != NULL);
	  if (!SYMBOL_REFERENCES_LOCAL (info, h))
	    {
	      _bfd_error_handler
		(_("%pB: gotoff relocation against dynamic symbol %s"),
		 input_bfd, h->root.root.string);
	      ret_val = false;
	      bfd_set_error (bfd_error_bad_value);
	    }
	  relocation -= got_sym_value;
	  break;

	case R_OR1K_INSN_REL_26:
	  /* For a non-shared link, these will reference plt or call the
	     version of actual object.  */
	  if (bfd_link_pic (info) && !SYMBOL_CALLS_LOCAL (info, h))
	    {
	      _bfd_error_handler
		(_("%pB: pc-relative relocation against dynamic symbol %s"),
		 input_bfd, name);
	      ret_val = false;
	      bfd_set_error (bfd_error_bad_value);
	    }
	  break;

	case R_OR1K_PCREL_PG21:
	case R_OR1K_LO13:
	case R_OR1K_SLO13:
	  /* For a non-shared link, these will reference either the plt
	     or a .dynbss copy of the symbol.  */
	  if (bfd_link_pic (info) && !SYMBOL_REFERENCES_LOCAL (info, h))
	    {
	      _bfd_error_handler
		(_("%pB: pc-relative relocation against dynamic symbol %s"),
		 input_bfd, name);
	      ret_val = false;
	      bfd_set_error (bfd_error_bad_value);
	    }
	  break;

	case R_OR1K_HI_16_IN_INSN:
	case R_OR1K_LO_16_IN_INSN:
	case R_OR1K_AHI16:
	case R_OR1K_SLO16:
	  if (bfd_link_pic (info))
	    {
	      _bfd_error_handler
		(_("%pB: non-pic relocation against symbol %s"),
		 input_bfd, name);
	      ret_val = false;
	      bfd_set_error (bfd_error_bad_value);
	    }
	  break;

	case R_OR1K_32:
	  /* R_OR1K_16? */
	  {
	    /* r_symndx will be STN_UNDEF (zero) only for relocs against symbols
	       from removed linkonce sections, or sections discarded by
	       a linker script.  */
	    if (r_symndx == STN_UNDEF
		|| (input_section->flags & SEC_ALLOC) == 0)
	      break;

	    /* Emit a direct relocation if the symbol is dynamic,
	       or a RELATIVE reloc for shared objects.  We can omit
	       RELATIVE relocs to local undefweak symbols.  */
	    if (bfd_link_pic (info)
		? (h == NULL
		     || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		     || h->root.type != bfd_link_hash_undefweak)
		: (h != NULL
		    && h->dynindx != -1
		    && !h->non_got_ref
		   && ((h->def_dynamic && !h->def_regular)
			|| h->root.type == bfd_link_hash_undefweak
			|| h->root.type == bfd_link_hash_undefined)))
	      {
		Elf_Internal_Rela outrel;
		bfd_byte *loc;
		bool skip;

		/* When generating a shared object, these relocations
		   are copied into the output file to be resolved at run
		   time.  */

		BFD_ASSERT (sreloc != NULL);

		skip = false;

		outrel.r_offset =
		  _bfd_elf_section_offset (output_bfd, info, input_section,
					   rel->r_offset);
		if (outrel.r_offset == (bfd_vma) -1)
		  skip = true;
		else if (outrel.r_offset == (bfd_vma) -2)
		  skip = true;
		outrel.r_offset += (input_section->output_section->vma
				    + input_section->output_offset);

		if (skip)
		  memset (&outrel, 0, sizeof outrel);
		else if (SYMBOL_REFERENCES_LOCAL (info, h))
		  {
		    outrel.r_info = ELF32_R_INFO (0, R_OR1K_RELATIVE);
		    outrel.r_addend = relocation + rel->r_addend;
		  }
		else
		  {
		    BFD_ASSERT (h->dynindx != -1);
		    outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		    outrel.r_addend = rel->r_addend;
		  }

		loc = sreloc->contents;
		loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
		bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		break;
	      }
	    break;
	  }

	case R_OR1K_TLS_LDM_HI16:
	case R_OR1K_TLS_LDM_LO16:
	case R_OR1K_TLS_LDM_PG21:
	case R_OR1K_TLS_LDM_LO13:
	case R_OR1K_TLS_LDO_HI16:
	case R_OR1K_TLS_LDO_LO16:
	  /* TODO: implement support for local dynamic.  */
	  BFD_FAIL ();
	  _bfd_error_handler
	    (_("%pB: support for local dynamic not implemented"),
	     input_bfd);
	  bfd_set_error (bfd_error_bad_value);
	  return false;

	case R_OR1K_TLS_GD_HI16:
	case R_OR1K_TLS_GD_LO16:
	case R_OR1K_TLS_GD_PG21:
	case R_OR1K_TLS_GD_LO13:
	case R_OR1K_TLS_IE_HI16:
	case R_OR1K_TLS_IE_LO16:
	case R_OR1K_TLS_IE_PG21:
	case R_OR1K_TLS_IE_LO13:
	case R_OR1K_TLS_IE_AHI16:
	  {
	    bfd_vma gotoff;
	    Elf_Internal_Rela rela;
	    asection *srelgot;
	    bfd_byte *loc;
	    bool dynamic;
	    int indx = 0;
	    unsigned char tls_type;

	    srelgot = htab->root.srelgot;

	    /* Mark as TLS related GOT entry by setting
	       bit 2 to indcate TLS and bit 1 to indicate GOT.  */
	    if (h != NULL)
	      {
		gotoff = h->got.offset;
		tls_type = ((struct elf_or1k_link_hash_entry *) h)->tls_type;
		h->got.offset |= 3;
	      }
	    else
	      {
		unsigned char *local_tls_type;

		gotoff = local_got_offsets[r_symndx];
		local_tls_type = (unsigned char *) elf_or1k_local_tls_type (input_bfd);
		tls_type = local_tls_type == NULL ? TLS_NONE
						  : local_tls_type[r_symndx];
		local_got_offsets[r_symndx] |= 3;
	      }

	    /* Only process the relocation once.  */
	    if ((gotoff & 1) != 0)
	      {
		gotoff += or1k_initial_exec_offset (howto, tls_type);

		/* The PG21 and LO13 relocs are pc-relative, while the
		   rest are GOT relative.  */
		relocation = got_base + (gotoff & ~3);
		if (!(r_type == R_OR1K_TLS_GD_PG21
		    || r_type == R_OR1K_TLS_GD_LO13
		    || r_type == R_OR1K_TLS_IE_PG21
		    || r_type == R_OR1K_TLS_IE_LO13))
		  relocation -= got_sym_value;
		break;
	      }

	    BFD_ASSERT (elf_hash_table (info)->hgot == NULL
			|| elf_hash_table (info)->hgot->root.u.def.value == 0);

	    if (h != NULL)
	      {
		bool dyn = htab->root.dynamic_sections_created;
		bool pic = bfd_link_pic (info);

		if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, pic, h)
		    && (!pic || !SYMBOL_REFERENCES_LOCAL (info, h)))
		  indx = h->dynindx;
	      }

	    /* Dynamic entries will require relocations.  If we do not need
	       them we will just use the default R_OR1K_NONE and
	       not set anything.  */
	    dynamic = (bfd_link_pic (info) || indx != 0)
		       && (h == NULL
			   || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			   || h->root.type != bfd_link_hash_undefweak);

	    /* Shared GD.  */
	    if (dynamic && ((tls_type & TLS_GD) != 0))
	      {
		int i;

		/* Add DTPMOD and DTPOFF GOT and rela entries.  */
		for (i = 0; i < 2; ++i)
		  {
		    BFD_ASSERT (srelgot->contents != NULL);

		    rela.r_offset = got_base + gotoff + i*4;
		    if (h != NULL && h->dynindx != -1)
		      {
			rela.r_info = ELF32_R_INFO (h->dynindx,
			    (i == 0 ? R_OR1K_TLS_DTPMOD : R_OR1K_TLS_DTPOFF));
			rela.r_addend = 0;
		      }
		    else
		      {
			rela.r_info = ELF32_R_INFO (0,
			    (i == 0 ? R_OR1K_TLS_DTPMOD : R_OR1K_TLS_DTPOFF));
			rela.r_addend =
			    (i == 0 ? 0 : tpoff (info, relocation, dynamic));
		      }

		    loc = srelgot->contents;
		    loc += (srelgot->reloc_count++
			    * sizeof (Elf32_External_Rela));

		    bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
		    bfd_put_32 (output_bfd, 0, sgot->contents + gotoff + i*4);
		  }
	      }
	    /* Static GD.  */
	    else if ((tls_type & TLS_GD) != 0)
	      {
		bfd_put_32 (output_bfd, 1, sgot->contents + gotoff);
		bfd_put_32 (output_bfd, tpoff (info, relocation, dynamic),
		    sgot->contents + gotoff + 4);
	      }

	    gotoff += or1k_initial_exec_offset (howto, tls_type);

	    /* Shared IE.  */
	    if (dynamic && ((tls_type & TLS_IE) != 0))
	      {
		BFD_ASSERT (srelgot->contents != NULL);

		/* Add TPOFF GOT and rela entries.  */
		rela.r_offset = got_base + gotoff;
		if (h != NULL && h->dynindx != -1)
		  {
		    rela.r_info = ELF32_R_INFO (h->dynindx, R_OR1K_TLS_TPOFF);
		    rela.r_addend = 0;
		  }
		else
		  {
		    rela.r_info = ELF32_R_INFO (0, R_OR1K_TLS_TPOFF);
		    rela.r_addend = tpoff (info, relocation, dynamic);
		  }

		loc = srelgot->contents;
		loc += srelgot->reloc_count++ * sizeof (Elf32_External_Rela);

		bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
		bfd_put_32 (output_bfd, 0, sgot->contents + gotoff);
	      }
	    /* Static IE.  */
	    else if ((tls_type & TLS_IE) != 0)
	      bfd_put_32 (output_bfd, tpoff (info, relocation, dynamic),
			  sgot->contents + gotoff);

	    /* The PG21 and LO13 relocs are pc-relative, while the
	       rest are GOT relative.  */
	    relocation = got_base + gotoff;
	    if (!(r_type == R_OR1K_TLS_GD_PG21
		  || r_type == R_OR1K_TLS_GD_LO13
		  || r_type == R_OR1K_TLS_IE_PG21
		  || r_type == R_OR1K_TLS_IE_LO13))
	      relocation -= got_sym_value;
	  }
	  break;

	case R_OR1K_TLS_LE_HI16:
	case R_OR1K_TLS_LE_LO16:
	case R_OR1K_TLS_LE_AHI16:
	case R_OR1K_TLS_LE_SLO16:
	  /* Relocation is offset from TP.  */
	  relocation = tpoff (info, relocation, 0);
	  break;

	case R_OR1K_TLS_DTPMOD:
	case R_OR1K_TLS_DTPOFF:
	case R_OR1K_TLS_TPOFF:
	  /* These are resolved dynamically on load and shouldn't
	     be used as linker input.  */
	  BFD_FAIL ();
	  _bfd_error_handler
	    (_("%pB: will not resolve runtime TLS relocation"),
	     input_bfd);
	  bfd_set_error (bfd_error_bad_value);
	  return false;

	default:
	  break;
	}

      r = or1k_final_link_relocate (howto, input_bfd, input_section, contents,
				    rel->r_offset, relocation + rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  const char *msg = NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, (h ? &h->root : NULL), name, howto->name,
		 (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      msg = _("internal error: out of range error");
	      break;

	    case bfd_reloc_notsupported:
	      msg = _("internal error: unsupported relocation error");
	      break;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous relocation");
	      break;

	    default:
	      msg = _("internal error: unknown error");
	      break;
	    }

	  if (msg)
	    (*info->callbacks->warning) (info, msg, name, input_bfd,
					 input_section, rel->r_offset);
	}
    }

  return ret_val;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
or1k_elf_gc_mark_hook (asection *sec,
		       struct bfd_link_info *info,
		       Elf_Internal_Rela *rel,
		       struct elf_link_hash_entry *h,
		       Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_OR1K_GNU_VTINHERIT:
      case R_OR1K_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Look through the relocs for a section during the first phase.  */

static bool
or1k_elf_check_relocs (bfd *abfd,
		       struct bfd_link_info *info,
		       asection *sec,
		       const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;

  const Elf_Internal_Rela *rel_end;
  struct elf_or1k_link_hash_table *htab;
  bfd *dynobj;
  asection *sreloc = NULL;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->root.dynobj;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;
      unsigned char tls_type;
      int r_type;

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
	case R_OR1K_TLS_GD_HI16:
	case R_OR1K_TLS_GD_LO16:
	case R_OR1K_TLS_GD_PG21:
	case R_OR1K_TLS_GD_LO13:
	  tls_type = TLS_GD;
	  break;
	case R_OR1K_TLS_LDM_HI16:
	case R_OR1K_TLS_LDM_LO16:
	case R_OR1K_TLS_LDM_PG21:
	case R_OR1K_TLS_LDM_LO13:
	case R_OR1K_TLS_LDO_HI16:
	case R_OR1K_TLS_LDO_LO16:
	  tls_type = TLS_LD;
	  break;
	case R_OR1K_TLS_IE_HI16:
	case R_OR1K_TLS_IE_LO16:
	case R_OR1K_TLS_IE_PG21:
	case R_OR1K_TLS_IE_LO13:
	case R_OR1K_TLS_IE_AHI16:
	  tls_type = TLS_IE;
	  break;
	case R_OR1K_TLS_LE_HI16:
	case R_OR1K_TLS_LE_LO16:
	case R_OR1K_TLS_LE_AHI16:
	case R_OR1K_TLS_LE_SLO16:
	  tls_type = TLS_LE;
	  break;
	default:
	  tls_type = TLS_NONE;
	}

      /* Record TLS type.  */
      if (h != NULL)
	  ((struct elf_or1k_link_hash_entry *) h)->tls_type |= tls_type;
      else
	{
	  unsigned char *local_tls_type;

	  /* This is a TLS type record for a local symbol.  */
	  local_tls_type = (unsigned char *) elf_or1k_local_tls_type (abfd);
	  if (local_tls_type == NULL)
	    {
	      bfd_size_type size;

	      size = symtab_hdr->sh_info;
	      local_tls_type = bfd_zalloc (abfd, size);
	      if (local_tls_type == NULL)
		return false;
	      elf_or1k_local_tls_type (abfd) = local_tls_type;
	    }
	  local_tls_type[r_symndx] |= tls_type;
	}

      switch (r_type)
	{
	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_OR1K_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_OR1K_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	  /* This relocation requires .plt entry.  */
	case R_OR1K_PLTA26:
	  htab->saw_plta = true;
	  /* FALLTHRU */
	case R_OR1K_PLT26:
	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      h->plt.refcount += 1;
	    }
	  break;

	case R_OR1K_GOT_AHI16:
	case R_OR1K_GOT16:
	case R_OR1K_GOT_PG21:
	case R_OR1K_GOT_LO13:
	case R_OR1K_TLS_GD_HI16:
	case R_OR1K_TLS_GD_LO16:
	case R_OR1K_TLS_GD_PG21:
	case R_OR1K_TLS_GD_LO13:
	case R_OR1K_TLS_IE_HI16:
	case R_OR1K_TLS_IE_LO16:
	case R_OR1K_TLS_IE_PG21:
	case R_OR1K_TLS_IE_LO13:
	case R_OR1K_TLS_IE_AHI16:
	      if (h != NULL)
		h->got.refcount += 1;
	      else
		{
		  bfd_signed_vma *local_got_refcounts;

		  /* This is a global offset table entry for a local symbol.  */
		  local_got_refcounts = elf_local_got_refcounts (abfd);
		  if (local_got_refcounts == NULL)
		    {
		      bfd_size_type size;

		      size = symtab_hdr->sh_info;
		      size *= sizeof (bfd_signed_vma);
		      local_got_refcounts = bfd_zalloc (abfd, size);
		      if (local_got_refcounts == NULL)
			return false;
		      elf_local_got_refcounts (abfd) = local_got_refcounts;
		    }
		  local_got_refcounts[r_symndx] += 1;
		}
	  /* FALLTHRU */

	case R_OR1K_GOTOFF_HI16:
	case R_OR1K_GOTOFF_LO16:
	case R_OR1K_GOTOFF_AHI16:
	case R_OR1K_GOTOFF_SLO16:
	  if (htab->root.sgot == NULL)
	    {
	      if (dynobj == NULL)
		htab->root.dynobj = dynobj = abfd;
	      if (!_bfd_elf_create_got_section (dynobj, info))
		return false;
	    }
	  break;

	case R_OR1K_INSN_REL_26:
	case R_OR1K_HI_16_IN_INSN:
	case R_OR1K_LO_16_IN_INSN:
	case R_OR1K_AHI16:
	case R_OR1K_SLO16:
	case R_OR1K_32:
	case R_OR1K_PCREL_PG21:
	case R_OR1K_LO13:
	case R_OR1K_SLO13:
	  {
	    if (h != NULL && !bfd_link_pic (info))
	      {
		/* We may need a copy reloc.  */
		h->non_got_ref = 1;

		/* We may also need a .plt entry.  */
		h->plt.refcount += 1;
		if (r_type != R_OR1K_INSN_REL_26)
		  h->pointer_equality_needed = 1;
	      }

	    /* If we are creating a shared library, and this is a reloc
	       against a global symbol, or a non PC relative reloc
	       against a local symbol, then we need to copy the reloc
	       into the shared library.  However, if we are linking with
	       -Bsymbolic, we do not need to copy a reloc against a
	       global symbol which is defined in an object we are
	       including in the link (i.e., DEF_REGULAR is set).  At
	       this point we have not seen all the input files, so it is
	       possible that DEF_REGULAR is not set now but will be set
	       later (it is never cleared).  In case of a weak definition,
	       DEF_REGULAR may be cleared later by a strong definition in
	       a shared library.  We account for that possibility below by
	       storing information in the relocs_copied field of the hash
	       table entry.  A similar situation occurs when creating
	       shared libraries and symbol visibility changes render the
	       symbol local.

	       If on the other hand, we are creating an executable, we
	       may need to keep relocations for symbols satisfied by a
	       dynamic library if we manage to avoid copy relocs for the
	       symbol.  */

	    if ((bfd_link_pic (info)
		 && (sec->flags & SEC_ALLOC) != 0
		 && (r_type != R_OR1K_INSN_REL_26
		     || (h != NULL
			 && (!SYMBOLIC_BIND (info, h)
			     || h->root.type == bfd_link_hash_defweak
			     || !h->def_regular))))
		|| (!bfd_link_pic (info)
		    && (sec->flags & SEC_ALLOC) != 0
		    && h != NULL
		    && (h->root.type == bfd_link_hash_defweak
			|| !h->def_regular)))
	      {
		struct elf_dyn_relocs *sec_relocs;
		struct elf_dyn_relocs **head;

		/* When creating a shared object, we must copy these
		   relocs into the output file.  We create a reloc
		   section in dynobj and make room for the reloc.  */
		if (sreloc == NULL)
		  {
		    const char *name;
		    unsigned int strndx = elf_elfheader (abfd)->e_shstrndx;
		    unsigned int shnam = _bfd_elf_single_rel_hdr (sec)->sh_name;

		    name = bfd_elf_string_from_elf_section (abfd, strndx, shnam);
		    if (name == NULL)
		      return false;

		    if (!startswith (name, ".rela")
			|| strcmp (bfd_section_name (sec), name + 5) != 0)
		      {
			_bfd_error_handler
			  /* xgettext:c-format */
			  (_("%pB: bad relocation section name `%s\'"),
			   abfd, name);
		      }

		    if (htab->root.dynobj == NULL)
		      htab->root.dynobj = abfd;
		    dynobj = htab->root.dynobj;

		    sreloc = bfd_get_section_by_name (dynobj, name);
		    if (sreloc == NULL)
		      {
			sreloc = _bfd_elf_make_dynamic_reloc_section
			  (sec, dynobj, 2, abfd, /*rela?*/ true);

			if (sreloc == NULL)
			  return false;
		      }
		    elf_section_data (sec)->sreloc = sreloc;
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
		    Elf_Internal_Sym *isym;
		    void *vpp;

		    isym = bfd_sym_from_r_symndx (&htab->root.sym_cache,
						  abfd, r_symndx);
		    if (isym == NULL)
		      return false;

		    s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		    if (s == NULL)
		      return false;

		    vpp = &elf_section_data (s)->local_dynrel;
		    head = (struct elf_dyn_relocs **) vpp;
		  }

		sec_relocs = *head;
		/* Allocate this sections dynamic reolcations structure if this
		   is a new section.  */
		if (sec_relocs == NULL || sec_relocs->sec != sec)
		  {
		    size_t amt = sizeof *sec_relocs;
		    sec_relocs = ((struct elf_dyn_relocs *)
				  bfd_alloc (htab->root.dynobj, amt));
		    if (sec_relocs == NULL)
		      return false;
		    sec_relocs->next = *head;
		    *head = sec_relocs;
		    sec_relocs->sec = sec;
		    sec_relocs->count = 0;
		    sec_relocs->pc_count = 0;
		  }

		sec_relocs->count += 1;
		if (r_type == R_OR1K_INSN_REL_26)
		  sec_relocs->pc_count += 1;
	      }
	  }
	  break;
	}
    }

  return true;
}

static void
or1k_write_plt_entry (bfd *output_bfd, bfd_byte *contents, unsigned insnj,
		      unsigned insns[], size_t insn_count)
{
  unsigned nodelay = elf_elfheader (output_bfd)->e_flags & EF_OR1K_NODELAY;
  unsigned output_insns[PLT_MAX_INSN_COUNT];

  /* Copy instructions into the output buffer.  */
  for (size_t i = 0; i < insn_count; i++)
    output_insns[i] = insns[i];

  /* Honor the no-delay-slot setting.  */
  if (insns[insn_count-1] == OR1K_NOP)
    {
      unsigned slot1, slot2;

      if (nodelay)
	slot1 = insns[insn_count-2], slot2 = insnj;
      else
	slot1 = insnj, slot2 = insns[insn_count-2];

      output_insns[insn_count-2] = slot1;
      output_insns[insn_count-1] = slot2;
      output_insns[insn_count]   = OR1K_NOP;
    }
  else
    {
      unsigned slot1, slot2;

      if (nodelay)
	slot1 = insns[insn_count-1], slot2 = insnj;
      else
	slot1 = insnj, slot2 = insns[insn_count-1];

      output_insns[insn_count-1] = slot1;
      output_insns[insn_count]   = slot2;
    }

  /* Write out the output buffer.  */
  for (size_t i = 0; i < (insn_count+1); i++)
    bfd_put_32 (output_bfd, output_insns[i], contents + (i*4));
}

/* Finish up the dynamic sections.  */

static bool
or1k_elf_finish_dynamic_sections (bfd *output_bfd,
				  struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn, *sgot;
  struct elf_or1k_link_hash_table *htab;

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->root.dynobj;

  sgot = htab->root.sgotplt;
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (htab->root.dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (sgot != NULL && sdyn != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);

      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      continue;

	    case DT_PLTGOT:
	      s = htab->root.sgotplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_JMPREL:
	      s = htab->root.srelplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_PLTRELSZ:
	      s = htab->root.srelplt;
	      dyn.d_un.d_val = s->size;
	      break;
	    }
	  bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	}


      /* Fill in the first entry in the procedure linkage table.  */
      splt = htab->root.splt;
      if (splt && splt->size > 0)
	{
	  unsigned plt[PLT_MAX_INSN_COUNT];
	  size_t plt_insn_count = 3;
	  bfd_vma got_addr = sgot->output_section->vma + sgot->output_offset;

	  /* Note we force 16 byte alignment on the .got, so that
	     the movhi/adrp can be shared between the two loads.  */

	  if (htab->saw_plta)
	    {
	      bfd_vma pc = splt->output_section->vma + splt->output_offset;
	      unsigned pa = ((got_addr >> 13) - (pc >> 13)) & 0x1fffff;
	      unsigned po = got_addr & 0x1fff;
	      plt[0] = OR1K_ADRP(12) | pa;
	      plt[1] = OR1K_LWZ(15,12) | (po + 8);
	      plt[2] = OR1K_LWZ(12,12) | (po + 4);
	    }
	  else if (bfd_link_pic (info))
	    {
	      plt[0] = OR1K_LWZ(15, 16) | 8;	/* .got+8 */
	      plt[1] = OR1K_LWZ(12, 16) | 4;	/* .got+4 */
	      plt[2] = OR1K_NOP;
	    }
	  else
	    {
	      unsigned ha = ((got_addr + 0x8000) >> 16) & 0xffff;
	      unsigned lo = got_addr & 0xffff;
	      plt[0] = OR1K_MOVHI(12) | ha;
	      plt[1] = OR1K_LWZ(15,12) | (lo + 8);
	      plt[2] = OR1K_LWZ(12,12) | (lo + 4);
	    }

	  or1k_write_plt_entry (output_bfd, splt->contents, OR1K_JR(15),
				plt, plt_insn_count);

	  elf_section_data (splt->output_section)->this_hdr.sh_entsize = 4;
	}
    }

  /* Set the first entry in the global offset table to the address of
     the dynamic section.  */
  if (sgot && sgot->size > 0)
    {
      if (sdyn == NULL)
	bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents);
      else
	bfd_put_32 (output_bfd,
		    sdyn->output_section->vma + sdyn->output_offset,
		    sgot->contents);
      elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;
    }

  if (htab->root.sgot && htab->root.sgot->size > 0)
    elf_section_data (htab->root.sgot->output_section)->this_hdr.sh_entsize = 4;

  return true;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
or1k_elf_finish_dynamic_symbol (bfd *output_bfd,
				struct bfd_link_info *info,
				struct elf_link_hash_entry *h,
				Elf_Internal_Sym *sym)
{
  struct elf_or1k_link_hash_table *htab;
  bfd_byte *loc;

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return false;

  if (h->plt.offset != (bfd_vma) -1)
    {
      unsigned int plt[PLT_MAX_INSN_COUNT];
      size_t plt_insn_count = 3;
      asection *splt;
      asection *sgot;
      asection *srela;
      bfd_vma plt_base_addr;
      bfd_vma plt_addr;
      bfd_vma plt_index;
      bfd_vma plt_reloc;
      bfd_vma got_base_addr;
      bfd_vma got_offset;
      bfd_vma got_addr;
      Elf_Internal_Rela rela;
      bool large_plt_entry;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */
      BFD_ASSERT (h->dynindx != -1);

      splt = htab->root.splt;
      sgot = htab->root.sgotplt;
      srela = htab->root.srelplt;
      BFD_ASSERT (splt != NULL && sgot != NULL && srela != NULL);

      plt_base_addr = splt->output_section->vma + splt->output_offset;
      got_base_addr = sgot->output_section->vma + sgot->output_offset;

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.  */
      plt_index = ((struct elf_or1k_link_hash_entry *) h)->plt_index;
      plt_addr = plt_base_addr + h->plt.offset;
      plt_reloc = plt_index * sizeof (Elf32_External_Rela);

      large_plt_entry = (elf_or1k_plt_entry_size (plt_index)
			 == PLT_ENTRY_SIZE_LARGE);

      /* Get the offset into the .got table of the entry that
	corresponds to this function.  Each .got entry is 4 bytes.
	The first three are reserved.  */
      got_offset = (plt_index + 3) * 4;
      got_addr = got_base_addr + got_offset;

      /* Fill in the entry in the procedure linkage table.  */
      if (htab->saw_plta)
	{
	  unsigned pa = ((got_addr >> 13) - (plt_addr >> 13)) & 0x1fffff;
	  unsigned po = (got_addr & 0x1fff);
	  plt[0] = OR1K_ADRP(12) | pa;
	  plt[1] = OR1K_LWZ(12,12) | po;
	  plt[2] = OR1K_ORI0(11) | plt_reloc;
	}
      else if (bfd_link_pic (info))
	{
	  if (large_plt_entry)
	    {
	      unsigned gotha = ((got_offset + 0x8000) >> 16) & 0xffff;
	      unsigned got = got_offset & 0xffff;
	      unsigned pltrelhi = (plt_reloc >> 16) & 0xffff;
	      unsigned pltrello = plt_reloc & 0xffff;

	      plt[0] = OR1K_MOVHI(12) | gotha;
	      plt[1] = OR1K_ADD(12,12,16);
	      plt[2] = OR1K_LWZ(12,12) | got;
	      plt[3] = OR1K_MOVHI(11) | pltrelhi;
	      plt[4] = OR1K_ORI(11,11) | pltrello;
	      plt_insn_count = 5;
	    }
	  else
	    {
	      plt[0] = OR1K_LWZ(12,16) | got_offset;
	      plt[1] = OR1K_ORI0(11) | plt_reloc;
	      plt[2] = OR1K_NOP;
	    }
	}
      else
	{
	  unsigned ha = ((got_addr + 0x8000) >> 16) & 0xffff;
	  unsigned lo = got_addr & 0xffff;
	  plt[0] = OR1K_MOVHI(12) | ha;
	  plt[1] = OR1K_LWZ(12,12) | lo;
	  plt[2] = OR1K_ORI0(11) | plt_reloc;
	}

      /* For large code model we fixup the non-PIC PLT relocation instructions
	 here.  */
      if (large_plt_entry && !bfd_link_pic (info))
	{
	  unsigned pltrelhi = (plt_reloc >> 16) & 0xffff;
	  unsigned pltrello = plt_reloc & 0xffff;

	  plt[2] = OR1K_MOVHI(11) | pltrelhi;
	  plt[3] = OR1K_ORI(11,11) | pltrello;
	  plt[4] = OR1K_NOP;
	  plt_insn_count = 5;
	}

      or1k_write_plt_entry (output_bfd, splt->contents + h->plt.offset,
			    OR1K_JR(12), plt, plt_insn_count);

      /* Fill in the entry in the global offset table.  We initialize it to
	 point to the top of the plt.  This is done to lazy lookup the actual
	 symbol as the first plt entry will be setup by libc to call the
	 runtime dynamic linker.  */
      bfd_put_32 (output_bfd, plt_base_addr, sgot->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = got_addr;
      rela.r_info = ELF32_R_INFO (h->dynindx, R_OR1K_JMP_SLOT);
      rela.r_addend = 0;
      loc = srela->contents;
      loc += plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	}
    }

  if (h->got.offset != (bfd_vma) -1
      && (h->got.offset & 2) == 0) /* Homemade TLS check.  */
    {
      asection *sgot;
      asection *srelgot;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */
      sgot = htab->root.sgot;
      srelgot = htab->root.srelgot;
      BFD_ASSERT (sgot != NULL && srelgot != NULL);

      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset
		       + (h->got.offset &~ 1));

      /* If this is a -Bsymbolic link, and the symbol is defined
	 locally, we just want to emit a RELATIVE reloc.  Likewise if
	 the symbol was forced to be local because of a version file.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (bfd_link_pic (info) && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  rela.r_info = ELF32_R_INFO (0, R_OR1K_RELATIVE);
	  rela.r_addend = (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset);
	}
      else
	{
	  BFD_ASSERT ((h->got.offset & 1) == 0);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + h->got.offset);
	  rela.r_info = ELF32_R_INFO (h->dynindx, R_OR1K_GLOB_DAT);
	  rela.r_addend = 0;
	}

      loc = srelgot->contents;
      loc += srelgot->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
      ++srelgot->reloc_count;
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;

      /* This symbols needs a copy reloc.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_OR1K_COPY);
      rela.r_addend = 0;
      if (h->root.u.def.section == htab->root.sdynrelro)
	s = htab->root.sreldynrelro;
      else
	s = htab->root.srelbss;
      loc = s->contents + s->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
      ++s->reloc_count;
    }

  /* Mark some specially defined symbols as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || h == htab->root.hgot)
    sym->st_shndx = SHN_ABS;

  return true;
}

static enum elf_reloc_type_class
or1k_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   const asection *rel_sec ATTRIBUTE_UNUSED,
			   const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_OR1K_RELATIVE:  return reloc_class_relative;
    case R_OR1K_JMP_SLOT:  return reloc_class_plt;
    case R_OR1K_COPY:	   return reloc_class_copy;
    default:		   return reloc_class_normal;
    }
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
or1k_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *h)
{
  struct elf_or1k_link_hash_table *htab;
  bfd *dynobj;
  asection *s, *srel;

  dynobj = elf_hash_table (info)->dynobj;

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
  if (h->type == STT_FUNC
      || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || (SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak)))
	{
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a PCREL
	     reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}

      return true;
    }
  else
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

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (bfd_link_pic (info))
    return true;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return true;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return true;
    }

  /* If we don't find any dynamic relocs in read-only sections, then
     we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
  if (!_bfd_elf_readonly_dynrelocs (h))
    {
      h->non_got_ref = 0;
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

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* We must generate a R_OR1K_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
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
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Caclulate an update the sizes required for a symbol in the GOT and
   RELA relocation section based on the TLS_TYPE and whether or not the symbol
   is DYNAMIC.

   Symbols with TLS_GD access require 8 bytes in the GOT and, if dynamic,
   require two relocation entries.  Symbols with TLS_IE access require 4 bytes
   in the GOT and, if dynamic, require one relocation entry.  Symbols may have
   both TLS_GD and TLS_IE access to be accounted for.

   Other symbols require 4 bytes in the GOT table and, if dynamic, require one
   relocation entry.  */

static void
or1k_set_got_and_rela_sizes (const unsigned char tls_type,
			     const bool dynamic,
			     bfd_vma *got_size,
			     bfd_vma *rela_size)
{
  bool is_tls_entry = false;

  /* TLS GD requires two GOT entries and two relocs.  */
  if ((tls_type & TLS_GD) != 0)
    {
      *got_size += 8;
      is_tls_entry = true;
    }

  if ((tls_type & TLS_IE) != 0)
    {
      *got_size += 4;
      is_tls_entry = true;
    }

  if (!is_tls_entry)
    *got_size += 4;

  if (dynamic)
    {
      if ((tls_type & TLS_GD) != 0)
	*rela_size += 2 * sizeof (Elf32_External_Rela);

      if ((tls_type & TLS_IE) != 0)
	*rela_size += sizeof (Elf32_External_Rela);

      if (!is_tls_entry)
	*rela_size += sizeof (Elf32_External_Rela);
    }
}


/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void * inf)
{
  struct bfd_link_info *info;
  struct elf_or1k_link_hash_table *htab;
  struct elf_dyn_relocs *sec_relocs;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return false;

  if (htab->root.dynamic_sections_created
      && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, bfd_link_pic (info), h))
	{
	  asection *splt = htab->root.splt;
	  bfd_vma plt_index;

	  /* Track the index of our plt entry for use in calculating size.  */
	  plt_index = htab->plt_count++;
	  ((struct elf_or1k_link_hash_entry *) h)->plt_index = plt_index;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (splt->size == 0)
	    splt->size = elf_or1k_plt_entry_size (plt_index);

	  h->plt.offset = splt->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (! bfd_link_pic (info)
	      && !h->def_regular)
	    {
	      h->root.u.def.section = splt;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  splt->size += elf_or1k_plt_entry_size (plt_index);

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->root.sgotplt->size += 4;

	  /* We also need to make an entry in the .rel.plt section.  */
	  htab->root.srelplt->size += sizeof (Elf32_External_Rela);
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

  if (h->got.refcount > 0)
    {
      asection *sgot;
      bool dyn;
      unsigned char tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      sgot = htab->root.sgot;

      h->got.offset = sgot->size;

      tls_type = ((struct elf_or1k_link_hash_entry *) h)->tls_type;

      dyn = htab->root.dynamic_sections_created;
      dyn = WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h);
      or1k_set_got_and_rela_sizes (tls_type, dyn,
				   &sgot->size, &htab->root.srelgot->size);
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
      if (SYMBOL_CALLS_LOCAL (info, h))
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &h->dyn_relocs; (sec_relocs = *pp) != NULL;)
	    {
	      sec_relocs->count -= sec_relocs->pc_count;
	      sec_relocs->pc_count = 0;
	      if (sec_relocs->count == 0)
		*pp = sec_relocs->next;
	      else
		pp = &sec_relocs->next;
	    }
	}

      /* Also discard relocs on undefined weak syms with non-default
	 visibility.  */
      if (h->dyn_relocs != NULL
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	    h->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1
		   && !h->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }
	}
    }
  else
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic
	       && !h->def_regular)
	      || (htab->root.dynamic_sections_created
		  && (h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined))))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1
	      && !h->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      h->dyn_relocs = NULL;

    keep: ;
    }

  /* Finally, allocate space.  */
  for (sec_relocs = h->dyn_relocs;
       sec_relocs != NULL;
       sec_relocs = sec_relocs->next)
    {
      asection *sreloc = elf_section_data (sec_relocs->sec)->sreloc;
      sreloc->size += sec_relocs->count * sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
or1k_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				struct bfd_link_info *info)
{
  struct elf_or1k_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->root.dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (htab->root.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_section_by_name (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      unsigned char *local_tls_type;
      asection *srel;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *sec_relocs;

	  for (sec_relocs = ((struct elf_dyn_relocs *)
			     elf_section_data (s)->local_dynrel);
	       sec_relocs != NULL;
	       sec_relocs = sec_relocs->next)
	    {
	      if (! bfd_is_abs_section (sec_relocs->sec)
		  && bfd_is_abs_section (sec_relocs->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (sec_relocs->count != 0)
		{
		  srel = elf_section_data (sec_relocs->sec)->sreloc;
		  srel->size += sec_relocs->count
				* sizeof (Elf32_External_Rela);
		  if ((sec_relocs->sec->output_section->flags & SEC_READONLY)
		      != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      s = htab->root.sgot;
      srel = htab->root.srelgot;
      local_tls_type = (unsigned char *) elf_or1k_local_tls_type (ibfd);
      for (; local_got < end_local_got; ++local_got)
	{
	  if (*local_got > 0)
	    {
	      unsigned char tls_type = (local_tls_type == NULL)
					? TLS_UNKNOWN
					: *local_tls_type;

	      *local_got = s->size;
	      or1k_set_got_and_rela_sizes (tls_type, bfd_link_pic (info),
					   &s->size, &srel->size);
	    }
	  else

	    *local_got = (bfd_vma) -1;

	  if (local_tls_type)
	    ++local_tls_type;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, allocate_dynrelocs, info);

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->root.splt
	  || s == htab->root.sgot
	  || s == htab->root.sgotplt
	  || s == htab->root.sdynbss
	  || s == htab->root.sdynrelro)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (startswith (bfd_section_name (s), ".rela"))
	{
	  if (s->size != 0 && s != htab->root.srelplt)
	    relocs = true;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  s->reloc_count = 0;
	}
      else
	/* It's not one of our sections, so don't allocate space.  */
	continue;

      if (s->size == 0)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is mostly to handle .rela.bss and
	     .rela.plt.  We must create both sections in
	     create_dynamic_sections, because they must be created
	     before the linker maps input sections to output
	     sections.  The linker does that before
	     adjust_dynamic_symbol is called, and it is that
	     function which decides whether anything needs to go
	     into these sections.  */
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  We use bfd_zalloc
	 here in case unused entries are not reclaimed before the
	 section's contents are written out.  This should not happen,
	 but this way if it does, we get a R_OR1K_NONE reloc instead
	 of garbage.  */
      s->contents = bfd_zalloc (dynobj, s->size);

      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
or1k_elf_copy_indirect_symbol (struct bfd_link_info *info,
			       struct elf_link_hash_entry *dir,
			       struct elf_link_hash_entry *ind)
{
  struct elf_or1k_link_hash_entry * edir;
  struct elf_or1k_link_hash_entry * eind;

  edir = (struct elf_or1k_link_hash_entry *) dir;
  eind = (struct elf_or1k_link_hash_entry *) ind;

  if (ind->root.type == bfd_link_hash_indirect)
    {
      if (dir->got.refcount <= 0)
	{
	  edir->tls_type = eind->tls_type;
	  eind->tls_type = TLS_UNKNOWN;
	}
    }

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Set the right machine number.  */

static bool
or1k_elf_object_p (bfd *abfd)
{
  unsigned long mach = bfd_mach_or1k;

  if (elf_elfheader (abfd)->e_flags & EF_OR1K_NODELAY)
    mach = bfd_mach_or1knd;

  return bfd_default_set_arch_mach (abfd, bfd_arch_or1k, mach);
}

/* Store the machine number in the flags field.  */

static bool
or1k_elf_final_write_processing (bfd *abfd)
{
  switch (bfd_get_mach (abfd))
    {
    default:
    case bfd_mach_or1k:
      break;
    case bfd_mach_or1knd:
      elf_elfheader (abfd)->e_flags |= EF_OR1K_NODELAY;
      break;
    }
  return _bfd_elf_final_write_processing (abfd);
}

static bool
or1k_elf_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

/* Make sure all input files are consistent with respect to
   EF_OR1K_NODELAY flag setting.  */

static bool
elf32_or1k_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword out_flags;
  flagword in_flags;

  in_flags  = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  if (!elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = in_flags;

      return true;
    }

  if (in_flags == out_flags)
    return true;

  if ((in_flags & EF_OR1K_NODELAY) != (out_flags & EF_OR1K_NODELAY))
    {
      _bfd_error_handler
	(_("%pB: %s flag mismatch with previous modules"),
	 ibfd, "EF_OR1K_NODELAY");

      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;

}

/* Implement elf_backend_grok_prstatus:
   Support for core dump NOTE sections.  */
static bool
or1k_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
    default:
      return false;

    case 212:	      /* Linux/OpenRISC */
      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

      /* pr_pid */
      elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, note->descdata + 24);

      /* pr_reg */
      offset = 72;
      size = 132;

      break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

/* Implement elf_backend_grok_psinfo.  */
static bool
or1k_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

    case 128:	      /* Linux/OpenRISC elf_prpsinfo */
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 32, 16);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 48, 80);
    }

  return true;
}


#define ELF_ARCH			bfd_arch_or1k
#define ELF_MACHINE_CODE		EM_OR1K
#define ELF_TARGET_ID			OR1K_ELF_DATA
#define ELF_MAXPAGESIZE			0x2000

#define TARGET_BIG_SYM			or1k_elf32_vec
#define TARGET_BIG_NAME			"elf32-or1k"

#define elf_info_to_howto_rel		NULL
#define elf_info_to_howto		or1k_info_to_howto_rela
#define elf_backend_relocate_section	or1k_elf_relocate_section
#define elf_backend_gc_mark_hook	or1k_elf_gc_mark_hook
#define elf_backend_check_relocs	or1k_elf_check_relocs
#define elf_backend_reloc_type_class	or1k_elf_reloc_type_class
#define elf_backend_can_gc_sections	1
#define elf_backend_rela_normal		1

#define bfd_elf32_mkobject		     elf_or1k_mkobject

#define bfd_elf32_bfd_merge_private_bfd_data elf32_or1k_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags or1k_elf_set_private_flags
#define bfd_elf32_bfd_reloc_type_lookup or1k_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup or1k_reloc_name_lookup

#define elf_backend_object_p		    or1k_elf_object_p
#define elf_backend_final_write_processing  or1k_elf_final_write_processing
#define elf_backend_can_refcount		1

#define elf_backend_plt_readonly		1
#define elf_backend_want_got_plt		1
#define elf_backend_want_plt_sym		0
#define elf_backend_got_header_size		12
#define elf_backend_dtrel_excludes_plt		1
#define elf_backend_want_dynrelro		1

#define bfd_elf32_bfd_link_hash_table_create	or1k_elf_link_hash_table_create
#define elf_backend_copy_indirect_symbol	or1k_elf_copy_indirect_symbol
#define elf_backend_create_dynamic_sections	_bfd_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections	or1k_elf_finish_dynamic_sections
#define elf_backend_size_dynamic_sections	or1k_elf_size_dynamic_sections
#define elf_backend_adjust_dynamic_symbol	or1k_elf_adjust_dynamic_symbol
#define elf_backend_finish_dynamic_symbol	or1k_elf_finish_dynamic_symbol

#define elf_backend_grok_prstatus	  or1k_grok_prstatus
#define elf_backend_grok_psinfo		  or1k_grok_psinfo

#include "elf32-target.h"
