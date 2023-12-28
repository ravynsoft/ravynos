/* 32-bit ELF support for ARM
   Copyright (C) 1998-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include <limits.h>

#include "bfd.h"
#include "libiberty.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf-nacl.h"
#include "elf-vxworks.h"
#include "elf/arm.h"
#include "elf32-arm.h"
#include "cpu-arm.h"

/* Return the relocation section associated with NAME.  HTAB is the
   bfd's elf32_arm_link_hash_entry.  */
#define RELOC_SECTION(HTAB, NAME) \
  ((HTAB)->use_rel ? ".rel" NAME : ".rela" NAME)

/* Return size of a relocation entry.  HTAB is the bfd's
   elf32_arm_link_hash_entry.  */
#define RELOC_SIZE(HTAB) \
  ((HTAB)->use_rel \
   ? sizeof (Elf32_External_Rel) \
   : sizeof (Elf32_External_Rela))

/* Return function to swap relocations in.  HTAB is the bfd's
   elf32_arm_link_hash_entry.  */
#define SWAP_RELOC_IN(HTAB) \
  ((HTAB)->use_rel \
   ? bfd_elf32_swap_reloc_in \
   : bfd_elf32_swap_reloca_in)

/* Return function to swap relocations out.  HTAB is the bfd's
   elf32_arm_link_hash_entry.  */
#define SWAP_RELOC_OUT(HTAB) \
  ((HTAB)->use_rel \
   ? bfd_elf32_swap_reloc_out \
   : bfd_elf32_swap_reloca_out)

#define elf_info_to_howto		NULL
#define elf_info_to_howto_rel		elf32_arm_info_to_howto

#define ARM_ELF_ABI_VERSION		0
#define ARM_ELF_OS_ABI_VERSION		ELFOSABI_ARM

/* The Adjusted Place, as defined by AAELF.  */
#define Pa(X) ((X) & 0xfffffffc)

static bool elf32_arm_write_section (bfd *output_bfd,
				     struct bfd_link_info *link_info,
				     asection *sec,
				     bfd_byte *contents);

/* Note: code such as elf32_arm_reloc_type_lookup expect to use e.g.
   R_ARM_PC24 as an index into this, and find the R_ARM_PC24 HOWTO
   in that slot.  */

static reloc_howto_type elf32_arm_howto_table_1[] =
{
  /* No relocation.  */
  HOWTO (R_ARM_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_NONE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_PC24,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_PC24",		/* name */
	 false,			/* partial_inplace */
	 0x00ffffff,		/* src_mask */
	 0x00ffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* 32 bit absolute */
  HOWTO (R_ARM_ABS32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ABS32",		/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* standard 32bit pc-relative reloc */
  HOWTO (R_ARM_REL32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_REL32",		/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit absolute - R_ARM_LDR_PC_G0 in AAELF */
  HOWTO (R_ARM_LDR_PC_G0,	/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDR_PC_G0",     /* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

   /* 16 bit absolute */
  HOWTO (R_ARM_ABS16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ABS16",		/* name */
	 false,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 12 bit absolute */
  HOWTO (R_ARM_ABS12,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ABS12",		/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_ABS5,	/* type */
	 6,			/* rightshift */
	 2,			/* size */
	 5,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_ABS5",	/* name */
	 false,			/* partial_inplace */
	 0x000007e0,		/* src_mask */
	 0x000007e0,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 8 bit absolute */
  HOWTO (R_ARM_ABS8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ABS8",		/* name */
	 false,			/* partial_inplace */
	 0x000000ff,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_SBREL32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_SBREL32",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_CALL,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_CALL",	/* name */
	 false,			/* partial_inplace */
	 0x07ff2fff,		/* src_mask */
	 0x07ff2fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_THM_PC8,		/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_PC8",	/* name */
	 false,			/* partial_inplace */
	 0x000000ff,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_BREL_ADJ,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_BREL_ADJ",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_DESC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_DESC",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_SWI8,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_SWI8",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00000000,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* BLX instruction for the ARM.  */
  HOWTO (R_ARM_XPC25,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_XPC25",		/* name */
	 false,			/* partial_inplace */
	 0x00ffffff,		/* src_mask */
	 0x00ffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* BLX instruction for the Thumb.  */
  HOWTO (R_ARM_THM_XPC22,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_XPC22",	/* name */
	 false,			/* partial_inplace */
	 0x07ff2fff,		/* src_mask */
	 0x07ff2fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* Dynamic TLS relocations.  */

  HOWTO (R_ARM_TLS_DTPMOD32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_TLS_DTPMOD32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_DTPOFF32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_TLS_DTPOFF32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_TPOFF32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_TLS_TPOFF32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Relocs used in ARM Linux */

  HOWTO (R_ARM_COPY,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_COPY",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_GLOB_DAT",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_JUMP_SLOT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_JUMP_SLOT",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_RELATIVE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_RELATIVE",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_GOTOFF32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_GOTOFF32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_GOTPC,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_GOTPC",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_GOT32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_GOT32",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_PLT32,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_PLT32",		/* name */
	 false,			/* partial_inplace */
	 0x00ffffff,		/* src_mask */
	 0x00ffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_CALL,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_CALL",		/* name */
	 false,			/* partial_inplace */
	 0x00ffffff,		/* src_mask */
	 0x00ffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_JUMP24,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_JUMP24",	/* name */
	 false,			/* partial_inplace */
	 0x00ffffff,		/* src_mask */
	 0x00ffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_THM_JUMP24,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_JUMP24",	/* name */
	 false,			/* partial_inplace */
	 0x07ff2fff,		/* src_mask */
	 0x07ff2fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_BASE_ABS,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_BASE_ABS",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_ALU_PCREL7_0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_PCREL_7_0",	/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_PCREL15_8,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 true,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_PCREL_15_8",/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_PCREL23_15,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 true,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_PCREL_23_15",/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDR_SBREL_11_0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDR_SBREL_11_0",/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_ALU_SBREL_19_12,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_SBREL_19_12",/* name */
	 false,			/* partial_inplace */
	 0x000ff000,		/* src_mask */
	 0x000ff000,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_ALU_SBREL_27_20,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 20,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_SBREL_27_20",/* name */
	 false,			/* partial_inplace */
	 0x0ff00000,		/* src_mask */
	 0x0ff00000,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TARGET1,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TARGET1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_ROSEGREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ROSEGREL32",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_V4BX,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_V4BX",		/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TARGET2,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TARGET2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_PREL31,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 31,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_PREL31",	/* name */
	 false,			/* partial_inplace */
	 0x7fffffff,		/* src_mask */
	 0x7fffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_MOVW_ABS_NC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_MOVW_ABS_NC",	/* name */
	 false,			/* partial_inplace */
	 0x000f0fff,		/* src_mask */
	 0x000f0fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_MOVT_ABS,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_MOVT_ABS",	/* name */
	 false,			/* partial_inplace */
	 0x000f0fff,		/* src_mask */
	 0x000f0fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_MOVW_PREL_NC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_MOVW_PREL_NC",	/* name */
	 false,			/* partial_inplace */
	 0x000f0fff,		/* src_mask */
	 0x000f0fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_MOVT_PREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_MOVT_PREL",	/* name */
	 false,			/* partial_inplace */
	 0x000f0fff,		/* src_mask */
	 0x000f0fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_THM_MOVW_ABS_NC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_MOVW_ABS_NC",/* name */
	 false,			/* partial_inplace */
	 0x040f70ff,		/* src_mask */
	 0x040f70ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_MOVT_ABS,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_MOVT_ABS",	/* name */
	 false,			/* partial_inplace */
	 0x040f70ff,		/* src_mask */
	 0x040f70ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_MOVW_PREL_NC,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_MOVW_PREL_NC",/* name */
	 false,			/* partial_inplace */
	 0x040f70ff,		/* src_mask */
	 0x040f70ff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_THM_MOVT_PREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_MOVT_PREL",	/* name */
	 false,			/* partial_inplace */
	 0x040f70ff,		/* src_mask */
	 0x040f70ff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_THM_JUMP19,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 19,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_THM_JUMP19",	/* name */
	 false,			/* partial_inplace */
	 0x043f2fff,		/* src_mask */
	 0x043f2fff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_THM_JUMP6,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 6,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_JUMP6",	/* name */
	 false,			/* partial_inplace */
	 0x02f8,		/* src_mask */
	 0x02f8,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* These are declared as 13-bit signed relocations because we can
     address -4095 .. 4095(base) by altering ADDW to SUBW or vice
     versa.  */
  HOWTO (R_ARM_THM_ALU_PREL_11_0,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 13,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_ALU_PREL_11_0",/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_THM_PC12,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 13,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_PC12",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ABS32_NOI,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ABS32_NOI",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_REL32_NOI,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_REL32_NOI",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Group relocations.  */

  HOWTO (R_ARM_ALU_PC_G0_NC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_PC_G0_NC",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_PC_G0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_PC_G0",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_PC_G1_NC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_PC_G1_NC",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_PC_G1,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_PC_G1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_PC_G2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_PC_G2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDR_PC_G1,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDR_PC_G1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDR_PC_G2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDR_PC_G2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDRS_PC_G0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDRS_PC_G0",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDRS_PC_G1,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDRS_PC_G1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDRS_PC_G2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDRS_PC_G2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDC_PC_G0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDC_PC_G0",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDC_PC_G1,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDC_PC_G1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDC_PC_G2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDC_PC_G2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_SB_G0_NC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_SB_G0_NC",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_SB_G0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_SB_G0",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_SB_G1_NC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_SB_G1_NC",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_SB_G1,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_SB_G1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_ALU_SB_G2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_ALU_SB_G2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDR_SB_G0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDR_SB_G0",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDR_SB_G1,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDR_SB_G1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDR_SB_G2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDR_SB_G2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDRS_SB_G0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDRS_SB_G0",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDRS_SB_G1,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDRS_SB_G1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDRS_SB_G2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDRS_SB_G2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDC_SB_G0,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDC_SB_G0",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDC_SB_G1,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDC_SB_G1",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_LDC_SB_G2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_LDC_SB_G2",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* End of group relocations.  */

  HOWTO (R_ARM_MOVW_BREL_NC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_MOVW_BREL_NC",	/* name */
	 false,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_MOVT_BREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_MOVT_BREL",	/* name */
	 false,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_MOVW_BREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_MOVW_BREL",	/* name */
	 false,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_MOVW_BREL_NC,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_MOVW_BREL_NC",/* name */
	 false,			/* partial_inplace */
	 0x040f70ff,		/* src_mask */
	 0x040f70ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_MOVT_BREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_MOVT_BREL",	/* name */
	 false,			/* partial_inplace */
	 0x040f70ff,		/* src_mask */
	 0x040f70ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_MOVW_BREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_MOVW_BREL",	/* name */
	 false,			/* partial_inplace */
	 0x040f70ff,		/* src_mask */
	 0x040f70ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_GOTDESC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_ARM_TLS_GOTDESC",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_CALL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_CALL",	/* name */
	 false,			/* partial_inplace */
	 0x00ffffff,		/* src_mask */
	 0x00ffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_DESCSEQ,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_DESCSEQ",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00000000,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_TLS_CALL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_TLS_CALL",	/* name */
	 false,			/* partial_inplace */
	 0x07ff07ff,		/* src_mask */
	 0x07ff07ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_PLT32_ABS,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_PLT32_ABS",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_GOT_ABS,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_GOT_ABS",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),			/* pcrel_offset */

  HOWTO (R_ARM_GOT_PREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_GOT_PREL",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_GOT_BREL12,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_GOT_BREL12",	/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_GOTOFF12,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_GOTOFF12",	/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (R_ARM_GOTRELAX),	 /* reserved for future GOT-load optimizations */

  /* GNU extension to record C++ vtable member usage */
  HOWTO (R_ARM_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn,	/* special_function */
	 "R_ARM_GNU_VTENTRY",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy */
  HOWTO (R_ARM_GNU_VTINHERIT, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_ARM_GNU_VTINHERIT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_THM_JUMP11,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 11,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_JUMP11",	/* name */
	 false,			/* partial_inplace */
	 0x000007ff,		/* src_mask */
	 0x000007ff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_ARM_THM_JUMP8,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_JUMP8",	/* name */
	 false,			/* partial_inplace */
	 0x000000ff,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* TLS relocations */
  HOWTO (R_ARM_TLS_GD32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_ARM_TLS_GD32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_LDM32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_TLS_LDM32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_LDO32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_TLS_LDO32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_IE32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			 /* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_ARM_TLS_IE32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_LE32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_ARM_TLS_LE32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_LDO12,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_LDO12",	/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_LE12,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_LE12",	/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_TLS_IE12GP,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_IE12GP",	/* name */
	 false,			/* partial_inplace */
	 0x00000fff,		/* src_mask */
	 0x00000fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 112-127 private relocations.  */
  EMPTY_HOWTO (112),
  EMPTY_HOWTO (113),
  EMPTY_HOWTO (114),
  EMPTY_HOWTO (115),
  EMPTY_HOWTO (116),
  EMPTY_HOWTO (117),
  EMPTY_HOWTO (118),
  EMPTY_HOWTO (119),
  EMPTY_HOWTO (120),
  EMPTY_HOWTO (121),
  EMPTY_HOWTO (122),
  EMPTY_HOWTO (123),
  EMPTY_HOWTO (124),
  EMPTY_HOWTO (125),
  EMPTY_HOWTO (126),
  EMPTY_HOWTO (127),

  /* R_ARM_ME_TOO, obsolete.  */
  EMPTY_HOWTO (128),

  HOWTO (R_ARM_THM_TLS_DESCSEQ,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_THM_TLS_DESCSEQ",/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00000000,		/* dst_mask */
	 false),		/* pcrel_offset */
  EMPTY_HOWTO (130),
  EMPTY_HOWTO (131),
  HOWTO (R_ARM_THM_ALU_ABS_G0_NC,/* type.  */
	 0,			/* rightshift.  */
	 2,			/* size.  */
	 16,			/* bitsize.  */
	 false,			/* pc_relative.  */
	 0,			/* bitpos.  */
	 complain_overflow_bitfield,/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,	/* special_function.  */
	 "R_ARM_THM_ALU_ABS_G0_NC",/* name.  */
	 false,			/* partial_inplace.  */
	 0x00000000,		/* src_mask.  */
	 0x00000000,		/* dst_mask.  */
	 false),		/* pcrel_offset.  */
  HOWTO (R_ARM_THM_ALU_ABS_G1_NC,/* type.  */
	 0,			/* rightshift.  */
	 2,			/* size.  */
	 16,			/* bitsize.  */
	 false,			/* pc_relative.  */
	 0,			/* bitpos.  */
	 complain_overflow_bitfield,/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,	/* special_function.  */
	 "R_ARM_THM_ALU_ABS_G1_NC",/* name.  */
	 false,			/* partial_inplace.  */
	 0x00000000,		/* src_mask.  */
	 0x00000000,		/* dst_mask.  */
	 false),		/* pcrel_offset.  */
  HOWTO (R_ARM_THM_ALU_ABS_G2_NC,/* type.  */
	 0,			/* rightshift.  */
	 2,			/* size.  */
	 16,			/* bitsize.  */
	 false,			/* pc_relative.  */
	 0,			/* bitpos.  */
	 complain_overflow_bitfield,/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,	/* special_function.  */
	 "R_ARM_THM_ALU_ABS_G2_NC",/* name.  */
	 false,			/* partial_inplace.  */
	 0x00000000,		/* src_mask.  */
	 0x00000000,		/* dst_mask.  */
	 false),		/* pcrel_offset.  */
  HOWTO (R_ARM_THM_ALU_ABS_G3_NC,/* type.  */
	 0,			/* rightshift.  */
	 2,			/* size.  */
	 16,			/* bitsize.  */
	 false,			/* pc_relative.  */
	 0,			/* bitpos.  */
	 complain_overflow_bitfield,/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,	/* special_function.  */
	 "R_ARM_THM_ALU_ABS_G3_NC",/* name.  */
	 false,			/* partial_inplace.  */
	 0x00000000,		/* src_mask.  */
	 0x00000000,		/* dst_mask.  */
	 false),		/* pcrel_offset.  */
  /* Relocations for Armv8.1-M Mainline.  */
  HOWTO (R_ARM_THM_BF16,	/* type.  */
	 0,			/* rightshift.  */
	 2,			/* size.  */
	 16,			/* bitsize.  */
	 true,			/* pc_relative.  */
	 0,			/* bitpos.  */
	 complain_overflow_dont,/* do not complain_on_overflow.  */
	 bfd_elf_generic_reloc,	/* special_function.  */
	 "R_ARM_THM_BF16",	/* name.  */
	 false,			/* partial_inplace.  */
	 0x001f0ffe,		/* src_mask.  */
	 0x001f0ffe,		/* dst_mask.  */
	 true),			/* pcrel_offset.  */
  HOWTO (R_ARM_THM_BF12,	/* type.  */
	 0,			/* rightshift.  */
	 2,			/* size.  */
	 12,			/* bitsize.  */
	 true,			/* pc_relative.  */
	 0,			/* bitpos.  */
	 complain_overflow_dont,/* do not complain_on_overflow.  */
	 bfd_elf_generic_reloc,	/* special_function.  */
	 "R_ARM_THM_BF12",	/* name.  */
	 false,			/* partial_inplace.  */
	 0x00010ffe,		/* src_mask.  */
	 0x00010ffe,		/* dst_mask.  */
	 true),			/* pcrel_offset.  */
  HOWTO (R_ARM_THM_BF18,	/* type.  */
	 0,			/* rightshift.  */
	 2,			/* size.  */
	 18,			/* bitsize.  */
	 true,			/* pc_relative.  */
	 0,			/* bitpos.  */
	 complain_overflow_dont,/* do not complain_on_overflow.  */
	 bfd_elf_generic_reloc,	/* special_function.  */
	 "R_ARM_THM_BF18",	/* name.  */
	 false,			/* partial_inplace.  */
	 0x007f0ffe,		/* src_mask.  */
	 0x007f0ffe,		/* dst_mask.  */
	 true),			/* pcrel_offset.  */
};

/* 160 onwards: */
static reloc_howto_type elf32_arm_howto_table_2[8] =
{
  HOWTO (R_ARM_IRELATIVE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_ARM_IRELATIVE",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_ARM_GOTFUNCDESC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_GOTFUNCDESC",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_ARM_GOTOFFFUNCDESC, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_GOTOFFFUNCDESC",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_ARM_FUNCDESC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_FUNCDESC",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_ARM_FUNCDESC_VALUE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_FUNCDESC_VALUE",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_ARM_TLS_GD32_FDPIC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_GD32_FDPIC",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_ARM_TLS_LDM32_FDPIC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_LDM32_FDPIC",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_ARM_TLS_IE32_FDPIC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_TLS_IE32_FDPIC",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
};

/* 249-255 extended, currently unused, relocations:  */
static reloc_howto_type elf32_arm_howto_table_3[4] =
{
  HOWTO (R_ARM_RREL32,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_RREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_RABS32,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_RABS32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_RPC24,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_RPC24",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_ARM_RBASE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ARM_RBASE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false)			/* pcrel_offset */
};

static reloc_howto_type *
elf32_arm_howto_from_type (unsigned int r_type)
{
  if (r_type < ARRAY_SIZE (elf32_arm_howto_table_1))
    return &elf32_arm_howto_table_1[r_type];

  if (r_type >= R_ARM_IRELATIVE
      && r_type < R_ARM_IRELATIVE + ARRAY_SIZE (elf32_arm_howto_table_2))
    return &elf32_arm_howto_table_2[r_type - R_ARM_IRELATIVE];

  if (r_type >= R_ARM_RREL32
      && r_type < R_ARM_RREL32 + ARRAY_SIZE (elf32_arm_howto_table_3))
    return &elf32_arm_howto_table_3[r_type - R_ARM_RREL32];

  return NULL;
}

static bool
elf32_arm_info_to_howto (bfd * abfd, arelent * bfd_reloc,
			 Elf_Internal_Rela * elf_reloc)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (elf_reloc->r_info);
  if ((bfd_reloc->howto = elf32_arm_howto_from_type (r_type)) == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  return true;
}

struct elf32_arm_reloc_map
  {
    bfd_reloc_code_real_type  bfd_reloc_val;
    unsigned char	      elf_reloc_val;
  };

/* All entries in this list must also be present in elf32_arm_howto_table.  */
static const struct elf32_arm_reloc_map elf32_arm_reloc_map[] =
  {
    {BFD_RELOC_NONE,		     R_ARM_NONE},
    {BFD_RELOC_ARM_PCREL_BRANCH,     R_ARM_PC24},
    {BFD_RELOC_ARM_PCREL_CALL,	     R_ARM_CALL},
    {BFD_RELOC_ARM_PCREL_JUMP,	     R_ARM_JUMP24},
    {BFD_RELOC_ARM_PCREL_BLX,	     R_ARM_XPC25},
    {BFD_RELOC_THUMB_PCREL_BLX,	     R_ARM_THM_XPC22},
    {BFD_RELOC_32,		     R_ARM_ABS32},
    {BFD_RELOC_32_PCREL,	     R_ARM_REL32},
    {BFD_RELOC_8,		     R_ARM_ABS8},
    {BFD_RELOC_16,		     R_ARM_ABS16},
    {BFD_RELOC_ARM_OFFSET_IMM,	     R_ARM_ABS12},
    {BFD_RELOC_ARM_THUMB_OFFSET,     R_ARM_THM_ABS5},
    {BFD_RELOC_THUMB_PCREL_BRANCH25, R_ARM_THM_JUMP24},
    {BFD_RELOC_THUMB_PCREL_BRANCH23, R_ARM_THM_CALL},
    {BFD_RELOC_THUMB_PCREL_BRANCH12, R_ARM_THM_JUMP11},
    {BFD_RELOC_THUMB_PCREL_BRANCH20, R_ARM_THM_JUMP19},
    {BFD_RELOC_THUMB_PCREL_BRANCH9,  R_ARM_THM_JUMP8},
    {BFD_RELOC_THUMB_PCREL_BRANCH7,  R_ARM_THM_JUMP6},
    {BFD_RELOC_ARM_GLOB_DAT,	     R_ARM_GLOB_DAT},
    {BFD_RELOC_ARM_JUMP_SLOT,	     R_ARM_JUMP_SLOT},
    {BFD_RELOC_ARM_RELATIVE,	     R_ARM_RELATIVE},
    {BFD_RELOC_ARM_GOTOFF,	     R_ARM_GOTOFF32},
    {BFD_RELOC_ARM_GOTPC,	     R_ARM_GOTPC},
    {BFD_RELOC_ARM_GOT_PREL,	     R_ARM_GOT_PREL},
    {BFD_RELOC_ARM_GOT32,	     R_ARM_GOT32},
    {BFD_RELOC_ARM_PLT32,	     R_ARM_PLT32},
    {BFD_RELOC_ARM_TARGET1,	     R_ARM_TARGET1},
    {BFD_RELOC_ARM_ROSEGREL32,	     R_ARM_ROSEGREL32},
    {BFD_RELOC_ARM_SBREL32,	     R_ARM_SBREL32},
    {BFD_RELOC_ARM_PREL31,	     R_ARM_PREL31},
    {BFD_RELOC_ARM_TARGET2,	     R_ARM_TARGET2},
    {BFD_RELOC_ARM_PLT32,	     R_ARM_PLT32},
    {BFD_RELOC_ARM_TLS_GOTDESC,	     R_ARM_TLS_GOTDESC},
    {BFD_RELOC_ARM_TLS_CALL,	     R_ARM_TLS_CALL},
    {BFD_RELOC_ARM_THM_TLS_CALL,     R_ARM_THM_TLS_CALL},
    {BFD_RELOC_ARM_TLS_DESCSEQ,	     R_ARM_TLS_DESCSEQ},
    {BFD_RELOC_ARM_THM_TLS_DESCSEQ,  R_ARM_THM_TLS_DESCSEQ},
    {BFD_RELOC_ARM_TLS_DESC,	     R_ARM_TLS_DESC},
    {BFD_RELOC_ARM_TLS_GD32,	     R_ARM_TLS_GD32},
    {BFD_RELOC_ARM_TLS_LDO32,	     R_ARM_TLS_LDO32},
    {BFD_RELOC_ARM_TLS_LDM32,	     R_ARM_TLS_LDM32},
    {BFD_RELOC_ARM_TLS_DTPMOD32,     R_ARM_TLS_DTPMOD32},
    {BFD_RELOC_ARM_TLS_DTPOFF32,     R_ARM_TLS_DTPOFF32},
    {BFD_RELOC_ARM_TLS_TPOFF32,	     R_ARM_TLS_TPOFF32},
    {BFD_RELOC_ARM_TLS_IE32,	     R_ARM_TLS_IE32},
    {BFD_RELOC_ARM_TLS_LE32,	     R_ARM_TLS_LE32},
    {BFD_RELOC_ARM_IRELATIVE,	     R_ARM_IRELATIVE},
    {BFD_RELOC_ARM_GOTFUNCDESC,      R_ARM_GOTFUNCDESC},
    {BFD_RELOC_ARM_GOTOFFFUNCDESC,   R_ARM_GOTOFFFUNCDESC},
    {BFD_RELOC_ARM_FUNCDESC,         R_ARM_FUNCDESC},
    {BFD_RELOC_ARM_FUNCDESC_VALUE,   R_ARM_FUNCDESC_VALUE},
    {BFD_RELOC_ARM_TLS_GD32_FDPIC,   R_ARM_TLS_GD32_FDPIC},
    {BFD_RELOC_ARM_TLS_LDM32_FDPIC,  R_ARM_TLS_LDM32_FDPIC},
    {BFD_RELOC_ARM_TLS_IE32_FDPIC,   R_ARM_TLS_IE32_FDPIC},
    {BFD_RELOC_VTABLE_INHERIT,	     R_ARM_GNU_VTINHERIT},
    {BFD_RELOC_VTABLE_ENTRY,	     R_ARM_GNU_VTENTRY},
    {BFD_RELOC_ARM_MOVW,	     R_ARM_MOVW_ABS_NC},
    {BFD_RELOC_ARM_MOVT,	     R_ARM_MOVT_ABS},
    {BFD_RELOC_ARM_MOVW_PCREL,	     R_ARM_MOVW_PREL_NC},
    {BFD_RELOC_ARM_MOVT_PCREL,	     R_ARM_MOVT_PREL},
    {BFD_RELOC_ARM_THUMB_MOVW,	     R_ARM_THM_MOVW_ABS_NC},
    {BFD_RELOC_ARM_THUMB_MOVT,	     R_ARM_THM_MOVT_ABS},
    {BFD_RELOC_ARM_THUMB_MOVW_PCREL, R_ARM_THM_MOVW_PREL_NC},
    {BFD_RELOC_ARM_THUMB_MOVT_PCREL, R_ARM_THM_MOVT_PREL},
    {BFD_RELOC_ARM_ALU_PC_G0_NC, R_ARM_ALU_PC_G0_NC},
    {BFD_RELOC_ARM_ALU_PC_G0, R_ARM_ALU_PC_G0},
    {BFD_RELOC_ARM_ALU_PC_G1_NC, R_ARM_ALU_PC_G1_NC},
    {BFD_RELOC_ARM_ALU_PC_G1, R_ARM_ALU_PC_G1},
    {BFD_RELOC_ARM_ALU_PC_G2, R_ARM_ALU_PC_G2},
    {BFD_RELOC_ARM_LDR_PC_G0, R_ARM_LDR_PC_G0},
    {BFD_RELOC_ARM_LDR_PC_G1, R_ARM_LDR_PC_G1},
    {BFD_RELOC_ARM_LDR_PC_G2, R_ARM_LDR_PC_G2},
    {BFD_RELOC_ARM_LDRS_PC_G0, R_ARM_LDRS_PC_G0},
    {BFD_RELOC_ARM_LDRS_PC_G1, R_ARM_LDRS_PC_G1},
    {BFD_RELOC_ARM_LDRS_PC_G2, R_ARM_LDRS_PC_G2},
    {BFD_RELOC_ARM_LDC_PC_G0, R_ARM_LDC_PC_G0},
    {BFD_RELOC_ARM_LDC_PC_G1, R_ARM_LDC_PC_G1},
    {BFD_RELOC_ARM_LDC_PC_G2, R_ARM_LDC_PC_G2},
    {BFD_RELOC_ARM_ALU_SB_G0_NC, R_ARM_ALU_SB_G0_NC},
    {BFD_RELOC_ARM_ALU_SB_G0, R_ARM_ALU_SB_G0},
    {BFD_RELOC_ARM_ALU_SB_G1_NC, R_ARM_ALU_SB_G1_NC},
    {BFD_RELOC_ARM_ALU_SB_G1, R_ARM_ALU_SB_G1},
    {BFD_RELOC_ARM_ALU_SB_G2, R_ARM_ALU_SB_G2},
    {BFD_RELOC_ARM_LDR_SB_G0, R_ARM_LDR_SB_G0},
    {BFD_RELOC_ARM_LDR_SB_G1, R_ARM_LDR_SB_G1},
    {BFD_RELOC_ARM_LDR_SB_G2, R_ARM_LDR_SB_G2},
    {BFD_RELOC_ARM_LDRS_SB_G0, R_ARM_LDRS_SB_G0},
    {BFD_RELOC_ARM_LDRS_SB_G1, R_ARM_LDRS_SB_G1},
    {BFD_RELOC_ARM_LDRS_SB_G2, R_ARM_LDRS_SB_G2},
    {BFD_RELOC_ARM_LDC_SB_G0, R_ARM_LDC_SB_G0},
    {BFD_RELOC_ARM_LDC_SB_G1, R_ARM_LDC_SB_G1},
    {BFD_RELOC_ARM_LDC_SB_G2, R_ARM_LDC_SB_G2},
    {BFD_RELOC_ARM_V4BX,	     R_ARM_V4BX},
    {BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC, R_ARM_THM_ALU_ABS_G3_NC},
    {BFD_RELOC_ARM_THUMB_ALU_ABS_G2_NC, R_ARM_THM_ALU_ABS_G2_NC},
    {BFD_RELOC_ARM_THUMB_ALU_ABS_G1_NC, R_ARM_THM_ALU_ABS_G1_NC},
    {BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC, R_ARM_THM_ALU_ABS_G0_NC},
    {BFD_RELOC_ARM_THUMB_BF17, R_ARM_THM_BF16},
    {BFD_RELOC_ARM_THUMB_BF13, R_ARM_THM_BF12},
    {BFD_RELOC_ARM_THUMB_BF19, R_ARM_THM_BF18}
  };

static reloc_howto_type *
elf32_arm_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			     bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf32_arm_reloc_map); i ++)
    if (elf32_arm_reloc_map[i].bfd_reloc_val == code)
      return elf32_arm_howto_from_type (elf32_arm_reloc_map[i].elf_reloc_val);

  return NULL;
}

static reloc_howto_type *
elf32_arm_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			     const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf32_arm_howto_table_1); i++)
    if (elf32_arm_howto_table_1[i].name != NULL
	&& strcasecmp (elf32_arm_howto_table_1[i].name, r_name) == 0)
      return &elf32_arm_howto_table_1[i];

  for (i = 0; i < ARRAY_SIZE (elf32_arm_howto_table_2); i++)
    if (elf32_arm_howto_table_2[i].name != NULL
	&& strcasecmp (elf32_arm_howto_table_2[i].name, r_name) == 0)
      return &elf32_arm_howto_table_2[i];

  for (i = 0; i < ARRAY_SIZE (elf32_arm_howto_table_3); i++)
    if (elf32_arm_howto_table_3[i].name != NULL
	&& strcasecmp (elf32_arm_howto_table_3[i].name, r_name) == 0)
      return &elf32_arm_howto_table_3[i];

  return NULL;
}

/* Support for core dump NOTE sections.  */

static bool
elf32_arm_nabi_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
      default:
	return false;

      case 148:		/* Linux/ARM 32-bit.  */
	/* pr_cursig */
	elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);

	/* pr_reg */
	offset = 72;
	size = 72;

	break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
elf32_arm_nabi_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
      default:
	return false;

      case 124:		/* Linux/ARM elf_prpsinfo.  */
	elf_tdata (abfd)->core->pid
	 = bfd_get_32 (abfd, note->descdata + 12);
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

static char *
elf32_arm_nabi_write_core_note (bfd *abfd, char *buf, int *bufsiz,
				int note_type, ...)
{
  switch (note_type)
    {
    default:
      return NULL;

    case NT_PRPSINFO:
      {
	char data[124] ATTRIBUTE_NONSTRING;
	va_list ap;

	va_start (ap, note_type);
	memset (data, 0, sizeof (data));
	strncpy (data + 28, va_arg (ap, const char *), 16);
#if GCC_VERSION == 8000 || GCC_VERSION == 8001
	DIAGNOSTIC_PUSH;
	/* GCC 8.0 and 8.1 warn about 80 equals destination size with
	   -Wstringop-truncation:
	   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85643
	 */
	DIAGNOSTIC_IGNORE_STRINGOP_TRUNCATION;
#endif
	strncpy (data + 44, va_arg (ap, const char *), 80);
#if GCC_VERSION == 8000 || GCC_VERSION == 8001
	DIAGNOSTIC_POP;
#endif
	va_end (ap);

	return elfcore_write_note (abfd, buf, bufsiz,
				   "CORE", note_type, data, sizeof (data));
      }

    case NT_PRSTATUS:
      {
	char data[148];
	va_list ap;
	long pid;
	int cursig;
	const void *greg;

	va_start (ap, note_type);
	memset (data, 0, sizeof (data));
	pid = va_arg (ap, long);
	bfd_put_32 (abfd, pid, data + 24);
	cursig = va_arg (ap, int);
	bfd_put_16 (abfd, cursig, data + 12);
	greg = va_arg (ap, const void *);
	memcpy (data + 72, greg, 72);
	va_end (ap);

	return elfcore_write_note (abfd, buf, bufsiz,
				   "CORE", note_type, data, sizeof (data));
      }
    }
}

#define TARGET_LITTLE_SYM		arm_elf32_le_vec
#define TARGET_LITTLE_NAME		"elf32-littlearm"
#define TARGET_BIG_SYM			arm_elf32_be_vec
#define TARGET_BIG_NAME			"elf32-bigarm"

#define elf_backend_grok_prstatus	elf32_arm_nabi_grok_prstatus
#define elf_backend_grok_psinfo		elf32_arm_nabi_grok_psinfo
#define elf_backend_write_core_note	elf32_arm_nabi_write_core_note

typedef unsigned long int insn32;
typedef unsigned short int insn16;

/* In lieu of proper flags, assume all EABIv4 or later objects are
   interworkable.  */
#define INTERWORK_FLAG(abfd)  \
  (EF_ARM_EABI_VERSION (elf_elfheader (abfd)->e_flags) >= EF_ARM_EABI_VER4 \
  || (elf_elfheader (abfd)->e_flags & EF_ARM_INTERWORK) \
  || ((abfd)->flags & BFD_LINKER_CREATED))

/* The linker script knows the section names for placement.
   The entry_names are used to do simple name mangling on the stubs.
   Given a function name, and its type, the stub can be found. The
   name can be changed. The only requirement is the %s be present.  */
#define THUMB2ARM_GLUE_SECTION_NAME ".glue_7t"
#define THUMB2ARM_GLUE_ENTRY_NAME   "__%s_from_thumb"

#define ARM2THUMB_GLUE_SECTION_NAME ".glue_7"
#define ARM2THUMB_GLUE_ENTRY_NAME   "__%s_from_arm"

#define VFP11_ERRATUM_VENEER_SECTION_NAME ".vfp11_veneer"
#define VFP11_ERRATUM_VENEER_ENTRY_NAME   "__vfp11_veneer_%x"

#define STM32L4XX_ERRATUM_VENEER_SECTION_NAME ".text.stm32l4xx_veneer"
#define STM32L4XX_ERRATUM_VENEER_ENTRY_NAME   "__stm32l4xx_veneer_%x"

#define ARM_BX_GLUE_SECTION_NAME ".v4_bx"
#define ARM_BX_GLUE_ENTRY_NAME   "__bx_r%d"

#define STUB_ENTRY_NAME   "__%s_veneer"

#define CMSE_PREFIX "__acle_se_"

#define CMSE_STUB_NAME ".gnu.sgstubs"

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER     "/usr/lib/ld.so.1"

/* FDPIC default stack size.  */
#define DEFAULT_STACK_SIZE 0x8000

static const unsigned long tls_trampoline [] =
{
  0xe08e0000,		/* add r0, lr, r0 */
  0xe5901004,		/* ldr r1, [r0,#4] */
  0xe12fff11,		/* bx  r1 */
};

static const unsigned long dl_tlsdesc_lazy_trampoline [] =
{
  0xe52d2004, /*	push    {r2}			*/
  0xe59f200c, /*      ldr     r2, [pc, #3f - . - 8]	*/
  0xe59f100c, /*      ldr     r1, [pc, #4f - . - 8]	*/
  0xe79f2002, /* 1:   ldr     r2, [pc, r2]		*/
  0xe081100f, /* 2:   add     r1, pc			*/
  0xe12fff12, /*      bx      r2			*/
  0x00000014, /* 3:   .word  _GLOBAL_OFFSET_TABLE_ - 1b - 8
				+ dl_tlsdesc_lazy_resolver(GOT)   */
  0x00000018, /* 4:   .word  _GLOBAL_OFFSET_TABLE_ - 2b - 8 */
};

/* NOTE: [Thumb nop sequence]
   When adding code that transitions from Thumb to Arm the instruction that
   should be used for the alignment padding should be 0xe7fd (b .-2) instead of
   a nop for performance reasons.  */

/* ARM FDPIC PLT entry.  */
/* The last 5 words contain PLT lazy fragment code and data.  */
static const bfd_vma elf32_arm_fdpic_plt_entry [] =
  {
    0xe59fc008,    /* ldr     r12, .L1 */
    0xe08cc009,    /* add     r12, r12, r9 */
    0xe59c9004,    /* ldr     r9, [r12, #4] */
    0xe59cf000,    /* ldr     pc, [r12] */
    0x00000000,    /* L1.     .word   foo(GOTOFFFUNCDESC) */
    0x00000000,    /* L1.     .word   foo(funcdesc_value_reloc_offset) */
    0xe51fc00c,    /* ldr     r12, [pc, #-12] */
    0xe92d1000,    /* push    {r12} */
    0xe599c004,    /* ldr     r12, [r9, #4] */
    0xe599f000,    /* ldr     pc, [r9] */
  };

/* Thumb FDPIC PLT entry.  */
/* The last 5 words contain PLT lazy fragment code and data.  */
static const bfd_vma elf32_arm_fdpic_thumb_plt_entry [] =
  {
    0xc00cf8df,    /* ldr.w   r12, .L1 */
    0x0c09eb0c,    /* add.w   r12, r12, r9 */
    0x9004f8dc,    /* ldr.w   r9, [r12, #4] */
    0xf000f8dc,    /* ldr.w   pc, [r12] */
    0x00000000,    /* .L1     .word   foo(GOTOFFFUNCDESC) */
    0x00000000,    /* .L2     .word   foo(funcdesc_value_reloc_offset) */
    0xc008f85f,    /* ldr.w   r12, .L2 */
    0xcd04f84d,    /* push    {r12} */
    0xc004f8d9,    /* ldr.w   r12, [r9, #4] */
    0xf000f8d9,    /* ldr.w   pc, [r9] */
  };

#ifdef FOUR_WORD_PLT

/* The first entry in a procedure linkage table looks like
   this.  It is set up so that any shared library function that is
   called before the relocation has been set up calls the dynamic
   linker first.  */
static const bfd_vma elf32_arm_plt0_entry [] =
{
  0xe52de004,		/* str   lr, [sp, #-4]! */
  0xe59fe010,		/* ldr   lr, [pc, #16]  */
  0xe08fe00e,		/* add   lr, pc, lr     */
  0xe5bef008,		/* ldr   pc, [lr, #8]!  */
};

/* Subsequent entries in a procedure linkage table look like
   this.  */
static const bfd_vma elf32_arm_plt_entry [] =
{
  0xe28fc600,		/* add   ip, pc, #NN	*/
  0xe28cca00,		/* add	 ip, ip, #NN	*/
  0xe5bcf000,		/* ldr	 pc, [ip, #NN]! */
  0x00000000,		/* unused		*/
};

#else /* not FOUR_WORD_PLT */

/* The first entry in a procedure linkage table looks like
   this.  It is set up so that any shared library function that is
   called before the relocation has been set up calls the dynamic
   linker first.  */
static const bfd_vma elf32_arm_plt0_entry [] =
{
  0xe52de004,		/* str	 lr, [sp, #-4]! */
  0xe59fe004,		/* ldr	 lr, [pc, #4]	*/
  0xe08fe00e,		/* add	 lr, pc, lr	*/
  0xe5bef008,		/* ldr	 pc, [lr, #8]!	*/
  0x00000000,		/* &GOT[0] - .		*/
};

/* By default subsequent entries in a procedure linkage table look like
   this. Offsets that don't fit into 28 bits will cause link error.  */
static const bfd_vma elf32_arm_plt_entry_short [] =
{
  0xe28fc600,		/* add   ip, pc, #0xNN00000 */
  0xe28cca00,		/* add	 ip, ip, #0xNN000   */
  0xe5bcf000,		/* ldr	 pc, [ip, #0xNNN]!  */
};

/* When explicitly asked, we'll use this "long" entry format
   which can cope with arbitrary displacements.  */
static const bfd_vma elf32_arm_plt_entry_long [] =
{
  0xe28fc200,		/* add	 ip, pc, #0xN0000000 */
  0xe28cc600,		/* add	 ip, ip, #0xNN00000  */
  0xe28cca00,		/* add	 ip, ip, #0xNN000    */
  0xe5bcf000,		/* ldr	 pc, [ip, #0xNNN]!   */
};

static bool elf32_arm_use_long_plt_entry = false;

#endif /* not FOUR_WORD_PLT */

/* The first entry in a procedure linkage table looks like this.
   It is set up so that any shared library function that is called before the
   relocation has been set up calls the dynamic linker first.  */
static const bfd_vma elf32_thumb2_plt0_entry [] =
{
  /* NOTE: As this is a mixture of 16-bit and 32-bit instructions,
     an instruction maybe encoded to one or two array elements.  */
  0xf8dfb500,		/* push	   {lr}		 */
  0x44fee008,		/* ldr.w   lr, [pc, #8]	 */
			/* add	   lr, pc	 */
  0xff08f85e,		/* ldr.w   pc, [lr, #8]! */
  0x00000000,		/* &GOT[0] - .		 */
};

/* Subsequent entries in a procedure linkage table for thumb only target
   look like this.  */
static const bfd_vma elf32_thumb2_plt_entry [] =
{
  /* NOTE: As this is a mixture of 16-bit and 32-bit instructions,
     an instruction maybe encoded to one or two array elements.  */
  0x0c00f240,		/* movw	   ip, #0xNNNN	  */
  0x0c00f2c0,		/* movt	   ip, #0xNNNN	  */
  0xf8dc44fc,		/* add	   ip, pc	  */
  0xe7fcf000		/* ldr.w   pc, [ip]	  */
			/* b      .-4		  */
};

/* The format of the first entry in the procedure linkage table
   for a VxWorks executable.  */
static const bfd_vma elf32_arm_vxworks_exec_plt0_entry[] =
{
  0xe52dc008,		/* str	  ip,[sp,#-8]!			*/
  0xe59fc000,		/* ldr	  ip,[pc]			*/
  0xe59cf008,		/* ldr	  pc,[ip,#8]			*/
  0x00000000,		/* .long  _GLOBAL_OFFSET_TABLE_		*/
};

/* The format of subsequent entries in a VxWorks executable.  */
static const bfd_vma elf32_arm_vxworks_exec_plt_entry[] =
{
  0xe59fc000,	      /* ldr	ip,[pc]			*/
  0xe59cf000,	      /* ldr	pc,[ip]			*/
  0x00000000,	      /* .long	@got				*/
  0xe59fc000,	      /* ldr	ip,[pc]			*/
  0xea000000,	      /* b	_PLT				*/
  0x00000000,	      /* .long	@pltindex*sizeof(Elf32_Rela)	*/
};

/* The format of entries in a VxWorks shared library.  */
static const bfd_vma elf32_arm_vxworks_shared_plt_entry[] =
{
  0xe59fc000,	      /* ldr	ip,[pc]			*/
  0xe79cf009,	      /* ldr	pc,[ip,r9]			*/
  0x00000000,	      /* .long	@got				*/
  0xe59fc000,	      /* ldr	ip,[pc]			*/
  0xe599f008,	      /* ldr	pc,[r9,#8]			*/
  0x00000000,	      /* .long	@pltindex*sizeof(Elf32_Rela)	*/
};

/* An initial stub used if the PLT entry is referenced from Thumb code.  */
#define PLT_THUMB_STUB_SIZE 4
static const bfd_vma elf32_arm_plt_thumb_stub [] =
{
  0x4778,		/* bx pc */
  0xe7fd		/* b .-2 */
};

/* The first entry in a procedure linkage table looks like
   this.  It is set up so that any shared library function that is
   called before the relocation has been set up calls the dynamic
   linker first.  */
static const bfd_vma elf32_arm_nacl_plt0_entry [] =
{
  /* First bundle: */
  0xe300c000,		/* movw	ip, #:lower16:&GOT[2]-.+8	*/
  0xe340c000,		/* movt	ip, #:upper16:&GOT[2]-.+8	*/
  0xe08cc00f,		/* add	ip, ip, pc			*/
  0xe52dc008,		/* str	ip, [sp, #-8]!			*/
  /* Second bundle: */
  0xe3ccc103,		/* bic	ip, ip, #0xc0000000		*/
  0xe59cc000,		/* ldr	ip, [ip]			*/
  0xe3ccc13f,		/* bic	ip, ip, #0xc000000f		*/
  0xe12fff1c,		/* bx	ip				*/
  /* Third bundle: */
  0xe320f000,		/* nop					*/
  0xe320f000,		/* nop					*/
  0xe320f000,		/* nop					*/
  /* .Lplt_tail: */
  0xe50dc004,		/* str	ip, [sp, #-4]			*/
  /* Fourth bundle: */
  0xe3ccc103,		/* bic	ip, ip, #0xc0000000		*/
  0xe59cc000,		/* ldr	ip, [ip]			*/
  0xe3ccc13f,		/* bic	ip, ip, #0xc000000f		*/
  0xe12fff1c,		/* bx	ip				*/
};
#define ARM_NACL_PLT_TAIL_OFFSET	(11 * 4)

/* Subsequent entries in a procedure linkage table look like this.  */
static const bfd_vma elf32_arm_nacl_plt_entry [] =
{
  0xe300c000,		/* movw	ip, #:lower16:&GOT[n]-.+8	*/
  0xe340c000,		/* movt	ip, #:upper16:&GOT[n]-.+8	*/
  0xe08cc00f,		/* add	ip, ip, pc			*/
  0xea000000,		/* b	.Lplt_tail			*/
};

/* PR 28924:
   There was a bug due to too high values of THM_MAX_FWD_BRANCH_OFFSET and
   THM2_MAX_FWD_BRANCH_OFFSET.  The first macro concerns the case when Thumb-2
   is not available, and second macro when Thumb-2 is available.  Among other
   things, they affect the range of branches represented as BLX instructions
   in Encoding T2 defined in Section A8.8.25 of the ARM Architecture
   Reference Manual ARMv7-A and ARMv7-R edition issue C.d.  Such branches are
   specified there to have a maximum forward offset that is a multiple of 4.
   Previously, the respective values defined here were multiples of 2 but not
   4 and they are included in comments for reference.  */
#define ARM_MAX_FWD_BRANCH_OFFSET  ((((1 << 23) - 1) << 2) + 8)
#define ARM_MAX_BWD_BRANCH_OFFSET ((-((1 << 23) << 2)) + 8)
#define THM_MAX_FWD_BRANCH_OFFSET   ((1 << 22) - 4 + 4)
/* #def THM_MAX_FWD_BRANCH_OFFSET   ((1 << 22) - 2 + 4) */
#define THM_MAX_BWD_BRANCH_OFFSET  (-(1 << 22) + 4)
#define THM2_MAX_FWD_BRANCH_OFFSET (((1 << 24) - 4) + 4)
/* #def THM2_MAX_FWD_BRANCH_OFFSET (((1 << 24) - 2) + 4) */
#define THM2_MAX_BWD_BRANCH_OFFSET (-(1 << 24) + 4)
#define THM2_MAX_FWD_COND_BRANCH_OFFSET (((1 << 20) -2) + 4)
#define THM2_MAX_BWD_COND_BRANCH_OFFSET (-(1 << 20) + 4)

enum stub_insn_type
{
  THUMB16_TYPE = 1,
  THUMB32_TYPE,
  ARM_TYPE,
  DATA_TYPE
};

#define THUMB16_INSN(X)		{(X), THUMB16_TYPE, R_ARM_NONE, 0}
/* A bit of a hack.  A Thumb conditional branch, in which the proper condition
   is inserted in arm_build_one_stub().  */
#define THUMB16_BCOND_INSN(X)	{(X), THUMB16_TYPE, R_ARM_NONE, 1}
#define THUMB32_INSN(X)		{(X), THUMB32_TYPE, R_ARM_NONE, 0}
#define THUMB32_MOVT(X)		{(X), THUMB32_TYPE, R_ARM_THM_MOVT_ABS, 0}
#define THUMB32_MOVW(X)		{(X), THUMB32_TYPE, R_ARM_THM_MOVW_ABS_NC, 0}
#define THUMB32_B_INSN(X, Z)	{(X), THUMB32_TYPE, R_ARM_THM_JUMP24, (Z)}
#define ARM_INSN(X)		{(X), ARM_TYPE, R_ARM_NONE, 0}
#define ARM_REL_INSN(X, Z)	{(X), ARM_TYPE, R_ARM_JUMP24, (Z)}
#define DATA_WORD(X,Y,Z)	{(X), DATA_TYPE, (Y), (Z)}

typedef struct
{
  bfd_vma	       data;
  enum stub_insn_type  type;
  unsigned int	       r_type;
  int		       reloc_addend;
}  insn_sequence;

/* See note [Thumb nop sequence] when adding a veneer.  */

/* Arm/Thumb -> Arm/Thumb long branch stub. On V5T and above, use blx
   to reach the stub if necessary.  */
static const insn_sequence elf32_arm_stub_long_branch_any_any[] =
{
  ARM_INSN (0xe51ff004),	    /* ldr   pc, [pc, #-4] */
  DATA_WORD (0, R_ARM_ABS32, 0),    /* dcd   R_ARM_ABS32(X) */
};

/* V4T Arm -> Thumb long branch stub. Used on V4T where blx is not
   available.  */
static const insn_sequence elf32_arm_stub_long_branch_v4t_arm_thumb[] =
{
  ARM_INSN (0xe59fc000),	    /* ldr   ip, [pc, #0] */
  ARM_INSN (0xe12fff1c),	    /* bx    ip */
  DATA_WORD (0, R_ARM_ABS32, 0),    /* dcd   R_ARM_ABS32(X) */
};

/* Thumb -> Thumb long branch stub. Used on M-profile architectures.  */
static const insn_sequence elf32_arm_stub_long_branch_thumb_only[] =
{
  THUMB16_INSN (0xb401),	     /* push {r0} */
  THUMB16_INSN (0x4802),	     /* ldr  r0, [pc, #8] */
  THUMB16_INSN (0x4684),	     /* mov  ip, r0 */
  THUMB16_INSN (0xbc01),	     /* pop  {r0} */
  THUMB16_INSN (0x4760),	     /* bx   ip */
  THUMB16_INSN (0xbf00),	     /* nop */
  DATA_WORD (0, R_ARM_ABS32, 0),     /* dcd  R_ARM_ABS32(X) */
};

/* Thumb -> Thumb long branch stub in thumb2 encoding.  Used on armv7.  */
static const insn_sequence elf32_arm_stub_long_branch_thumb2_only[] =
{
  THUMB32_INSN (0xf85ff000),	     /* ldr.w  pc, [pc, #-0] */
  DATA_WORD (0, R_ARM_ABS32, 0),     /* dcd  R_ARM_ABS32(x) */
};

/* Thumb -> Thumb long branch stub. Used for PureCode sections on Thumb2
   M-profile architectures.  */
static const insn_sequence elf32_arm_stub_long_branch_thumb2_only_pure[] =
{
  THUMB32_MOVW (0xf2400c00),	     /* mov.w ip, R_ARM_MOVW_ABS_NC */
  THUMB32_MOVT (0xf2c00c00),	     /* movt  ip, R_ARM_MOVT_ABS << 16 */
  THUMB16_INSN (0x4760),	     /* bx   ip */
};

/* V4T Thumb -> Thumb long branch stub. Using the stack is not
   allowed.  */
static const insn_sequence elf32_arm_stub_long_branch_v4t_thumb_thumb[] =
{
  THUMB16_INSN (0x4778),	     /* bx   pc */
  THUMB16_INSN (0xe7fd),	     /* b   .-2 */
  ARM_INSN (0xe59fc000),	     /* ldr  ip, [pc, #0] */
  ARM_INSN (0xe12fff1c),	     /* bx   ip */
  DATA_WORD (0, R_ARM_ABS32, 0),     /* dcd  R_ARM_ABS32(X) */
};

/* V4T Thumb -> ARM long branch stub. Used on V4T where blx is not
   available.  */
static const insn_sequence elf32_arm_stub_long_branch_v4t_thumb_arm[] =
{
  THUMB16_INSN (0x4778),	     /* bx   pc */
  THUMB16_INSN (0xe7fd),	     /* b   .-2 */
  ARM_INSN (0xe51ff004),	     /* ldr   pc, [pc, #-4] */
  DATA_WORD (0, R_ARM_ABS32, 0),     /* dcd   R_ARM_ABS32(X) */
};

/* V4T Thumb -> ARM short branch stub. Shorter variant of the above
   one, when the destination is close enough.  */
static const insn_sequence elf32_arm_stub_short_branch_v4t_thumb_arm[] =
{
  THUMB16_INSN (0x4778),	     /* bx   pc */
  THUMB16_INSN (0xe7fd),	     /* b   .-2 */
  ARM_REL_INSN (0xea000000, -8),     /* b    (X-8) */
};

/* ARM/Thumb -> ARM long branch stub, PIC.  On V5T and above, use
   blx to reach the stub if necessary.  */
static const insn_sequence elf32_arm_stub_long_branch_any_arm_pic[] =
{
  ARM_INSN (0xe59fc000),	     /* ldr   ip, [pc] */
  ARM_INSN (0xe08ff00c),	     /* add   pc, pc, ip */
  DATA_WORD (0, R_ARM_REL32, -4),    /* dcd   R_ARM_REL32(X-4) */
};

/* ARM/Thumb -> Thumb long branch stub, PIC.  On V5T and above, use
   blx to reach the stub if necessary.  We can not add into pc;
   it is not guaranteed to mode switch (different in ARMv6 and
   ARMv7).  */
static const insn_sequence elf32_arm_stub_long_branch_any_thumb_pic[] =
{
  ARM_INSN (0xe59fc004),	     /* ldr   ip, [pc, #4] */
  ARM_INSN (0xe08fc00c),	     /* add   ip, pc, ip */
  ARM_INSN (0xe12fff1c),	     /* bx    ip */
  DATA_WORD (0, R_ARM_REL32, 0),     /* dcd   R_ARM_REL32(X) */
};

/* V4T ARM -> ARM long branch stub, PIC.  */
static const insn_sequence elf32_arm_stub_long_branch_v4t_arm_thumb_pic[] =
{
  ARM_INSN (0xe59fc004),	     /* ldr   ip, [pc, #4] */
  ARM_INSN (0xe08fc00c),	     /* add   ip, pc, ip */
  ARM_INSN (0xe12fff1c),	     /* bx    ip */
  DATA_WORD (0, R_ARM_REL32, 0),     /* dcd   R_ARM_REL32(X) */
};

/* V4T Thumb -> ARM long branch stub, PIC.  */
static const insn_sequence elf32_arm_stub_long_branch_v4t_thumb_arm_pic[] =
{
  THUMB16_INSN (0x4778),	     /* bx   pc */
  THUMB16_INSN (0xe7fd),	     /* b   .-2 */
  ARM_INSN (0xe59fc000),	     /* ldr  ip, [pc, #0] */
  ARM_INSN (0xe08cf00f),	     /* add  pc, ip, pc */
  DATA_WORD (0, R_ARM_REL32, -4),     /* dcd  R_ARM_REL32(X) */
};

/* Thumb -> Thumb long branch stub, PIC. Used on M-profile
   architectures.  */
static const insn_sequence elf32_arm_stub_long_branch_thumb_only_pic[] =
{
  THUMB16_INSN (0xb401),	     /* push {r0} */
  THUMB16_INSN (0x4802),	     /* ldr  r0, [pc, #8] */
  THUMB16_INSN (0x46fc),	     /* mov  ip, pc */
  THUMB16_INSN (0x4484),	     /* add  ip, r0 */
  THUMB16_INSN (0xbc01),	     /* pop  {r0} */
  THUMB16_INSN (0x4760),	     /* bx   ip */
  DATA_WORD (0, R_ARM_REL32, 4),     /* dcd  R_ARM_REL32(X) */
};

/* V4T Thumb -> Thumb long branch stub, PIC. Using the stack is not
   allowed.  */
static const insn_sequence elf32_arm_stub_long_branch_v4t_thumb_thumb_pic[] =
{
  THUMB16_INSN (0x4778),	     /* bx   pc */
  THUMB16_INSN (0xe7fd),	     /* b   .-2 */
  ARM_INSN (0xe59fc004),	     /* ldr  ip, [pc, #4] */
  ARM_INSN (0xe08fc00c),	     /* add   ip, pc, ip */
  ARM_INSN (0xe12fff1c),	     /* bx   ip */
  DATA_WORD (0, R_ARM_REL32, 0),     /* dcd  R_ARM_REL32(X) */
};

/* Thumb2/ARM -> TLS trampoline.  Lowest common denominator, which is a
   long PIC stub.  We can use r1 as a scratch -- and cannot use ip.  */
static const insn_sequence elf32_arm_stub_long_branch_any_tls_pic[] =
{
  ARM_INSN (0xe59f1000),	     /* ldr   r1, [pc] */
  ARM_INSN (0xe08ff001),	     /* add   pc, pc, r1 */
  DATA_WORD (0, R_ARM_REL32, -4),    /* dcd   R_ARM_REL32(X-4) */
};

/* V4T Thumb -> TLS trampoline.  lowest common denominator, which is a
   long PIC stub.  We can use r1 as a scratch -- and cannot use ip.  */
static const insn_sequence elf32_arm_stub_long_branch_v4t_thumb_tls_pic[] =
{
  THUMB16_INSN (0x4778),	     /* bx   pc */
  THUMB16_INSN (0xe7fd),	     /* b   .-2 */
  ARM_INSN (0xe59f1000),	     /* ldr  r1, [pc, #0] */
  ARM_INSN (0xe081f00f),	     /* add  pc, r1, pc */
  DATA_WORD (0, R_ARM_REL32, -4),    /* dcd  R_ARM_REL32(X) */
};

/* NaCl ARM -> ARM long branch stub.  */
static const insn_sequence elf32_arm_stub_long_branch_arm_nacl[] =
{
  ARM_INSN (0xe59fc00c),		/* ldr	ip, [pc, #12] */
  ARM_INSN (0xe3ccc13f),		/* bic	ip, ip, #0xc000000f */
  ARM_INSN (0xe12fff1c),		/* bx	ip */
  ARM_INSN (0xe320f000),		/* nop */
  ARM_INSN (0xe125be70),		/* bkpt	0x5be0 */
  DATA_WORD (0, R_ARM_ABS32, 0),	/* dcd	R_ARM_ABS32(X) */
  DATA_WORD (0, R_ARM_NONE, 0),		/* .word 0 */
  DATA_WORD (0, R_ARM_NONE, 0),		/* .word 0 */
};

/* NaCl ARM -> ARM long branch stub, PIC.  */
static const insn_sequence elf32_arm_stub_long_branch_arm_nacl_pic[] =
{
  ARM_INSN (0xe59fc00c),		/* ldr	ip, [pc, #12] */
  ARM_INSN (0xe08cc00f),		/* add	ip, ip, pc */
  ARM_INSN (0xe3ccc13f),		/* bic	ip, ip, #0xc000000f */
  ARM_INSN (0xe12fff1c),		/* bx	ip */
  ARM_INSN (0xe125be70),		/* bkpt	0x5be0 */
  DATA_WORD (0, R_ARM_REL32, 8),	/* dcd	R_ARM_REL32(X+8) */
  DATA_WORD (0, R_ARM_NONE, 0),		/* .word 0 */
  DATA_WORD (0, R_ARM_NONE, 0),		/* .word 0 */
};

/* Stub used for transition to secure state (aka SG veneer).  */
static const insn_sequence elf32_arm_stub_cmse_branch_thumb_only[] =
{
  THUMB32_INSN (0xe97fe97f),		/* sg.  */
  THUMB32_B_INSN (0xf000b800, -4),	/* b.w original_branch_dest.  */
};


/* Cortex-A8 erratum-workaround stubs.  */

/* Stub used for conditional branches (which may be beyond +/-1MB away, so we
   can't use a conditional branch to reach this stub).  */

static const insn_sequence elf32_arm_stub_a8_veneer_b_cond[] =
{
  THUMB16_BCOND_INSN (0xd001),	       /* b<cond>.n true.  */
  THUMB32_B_INSN (0xf000b800, -4),     /* b.w insn_after_original_branch.  */
  THUMB32_B_INSN (0xf000b800, -4)      /* true: b.w original_branch_dest.  */
};

/* Stub used for b.w and bl.w instructions.  */

static const insn_sequence elf32_arm_stub_a8_veneer_b[] =
{
  THUMB32_B_INSN (0xf000b800, -4)	/* b.w original_branch_dest.  */
};

static const insn_sequence elf32_arm_stub_a8_veneer_bl[] =
{
  THUMB32_B_INSN (0xf000b800, -4)	/* b.w original_branch_dest.  */
};

/* Stub used for Thumb-2 blx.w instructions.  We modified the original blx.w
   instruction (which switches to ARM mode) to point to this stub.  Jump to the
   real destination using an ARM-mode branch.  */

static const insn_sequence elf32_arm_stub_a8_veneer_blx[] =
{
  ARM_REL_INSN (0xea000000, -8)	/* b original_branch_dest.  */
};

/* For each section group there can be a specially created linker section
   to hold the stubs for that group.  The name of the stub section is based
   upon the name of another section within that group with the suffix below
   applied.

   PR 13049: STUB_SUFFIX used to be ".stub", but this allowed the user to
   create what appeared to be a linker stub section when it actually
   contained user code/data.  For example, consider this fragment:

     const char * stubborn_problems[] = { "np" };

   If this is compiled with "-fPIC -fdata-sections" then gcc produces a
   section called:

     .data.rel.local.stubborn_problems

   This then causes problems in arm32_arm_build_stubs() as it triggers:

      // Ignore non-stub sections.
      if (!strstr (stub_sec->name, STUB_SUFFIX))
	continue;

   And so the section would be ignored instead of being processed.  Hence
   the change in definition of STUB_SUFFIX to a name that cannot be a valid
   C identifier.  */
#define STUB_SUFFIX ".__stub"

/* One entry per long/short branch stub defined above.  */
#define DEF_STUBS \
  DEF_STUB (long_branch_any_any)	\
  DEF_STUB (long_branch_v4t_arm_thumb) \
  DEF_STUB (long_branch_thumb_only) \
  DEF_STUB (long_branch_v4t_thumb_thumb)	\
  DEF_STUB (long_branch_v4t_thumb_arm) \
  DEF_STUB (short_branch_v4t_thumb_arm) \
  DEF_STUB (long_branch_any_arm_pic) \
  DEF_STUB (long_branch_any_thumb_pic) \
  DEF_STUB (long_branch_v4t_thumb_thumb_pic) \
  DEF_STUB (long_branch_v4t_arm_thumb_pic) \
  DEF_STUB (long_branch_v4t_thumb_arm_pic) \
  DEF_STUB (long_branch_thumb_only_pic) \
  DEF_STUB (long_branch_any_tls_pic) \
  DEF_STUB (long_branch_v4t_thumb_tls_pic) \
  DEF_STUB (long_branch_arm_nacl) \
  DEF_STUB (long_branch_arm_nacl_pic) \
  DEF_STUB (cmse_branch_thumb_only) \
  DEF_STUB (a8_veneer_b_cond) \
  DEF_STUB (a8_veneer_b) \
  DEF_STUB (a8_veneer_bl) \
  DEF_STUB (a8_veneer_blx) \
  DEF_STUB (long_branch_thumb2_only) \
  DEF_STUB (long_branch_thumb2_only_pure)

#define DEF_STUB(x) arm_stub_##x,
enum elf32_arm_stub_type
{
  arm_stub_none,
  DEF_STUBS
  max_stub_type
};
#undef DEF_STUB

/* Note the first a8_veneer type.  */
const unsigned arm_stub_a8_veneer_lwm = arm_stub_a8_veneer_b_cond;

typedef struct
{
  const insn_sequence* template_sequence;
  int template_size;
} stub_def;

#define DEF_STUB(x) {elf32_arm_stub_##x, ARRAY_SIZE(elf32_arm_stub_##x)},
static const stub_def stub_definitions[] =
{
  {NULL, 0},
  DEF_STUBS
};

struct elf32_arm_stub_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  /* The stub section.  */
  asection *stub_sec;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump).  */
  bfd_vma target_value;
  asection *target_section;

  /* Same as above but for the source of the branch to the stub.  Used for
     Cortex-A8 erratum workaround to patch it to branch to the stub.  As
     such, source section does not need to be recorded since Cortex-A8 erratum
     workaround stubs are only generated when both source and target are in the
     same section.  */
  bfd_vma source_value;

  /* The instruction which caused this stub to be generated (only valid for
     Cortex-A8 erratum workaround stubs at present).  */
  unsigned long orig_insn;

  /* The stub type.  */
  enum elf32_arm_stub_type stub_type;
  /* Its encoding size in bytes.  */
  int stub_size;
  /* Its template.  */
  const insn_sequence *stub_template;
  /* The size of the template (number of entries).  */
  int stub_template_size;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf32_arm_link_hash_entry *h;

  /* Type of branch.  */
  enum arm_st_branch_type branch_type;

  /* Where this stub is being called from, or, in the case of combined
     stub sections, the first input section in the group.  */
  asection *id_sec;

  /* The name for the local symbol at the start of this stub.  The
     stub name in the hash table has to be unique; this does not, so
     it can be friendlier.  */
  char *output_name;
};

/* Used to build a map of a section.  This is required for mixed-endian
   code/data.  */

typedef struct elf32_elf_section_map
{
  bfd_vma vma;
  char type;
}
elf32_arm_section_map;

/* Information about a VFP11 erratum veneer, or a branch to such a veneer.  */

typedef enum
{
  VFP11_ERRATUM_BRANCH_TO_ARM_VENEER,
  VFP11_ERRATUM_BRANCH_TO_THUMB_VENEER,
  VFP11_ERRATUM_ARM_VENEER,
  VFP11_ERRATUM_THUMB_VENEER
}
elf32_vfp11_erratum_type;

typedef struct elf32_vfp11_erratum_list
{
  struct elf32_vfp11_erratum_list *next;
  bfd_vma vma;
  union
  {
    struct
    {
      struct elf32_vfp11_erratum_list *veneer;
      unsigned int vfp_insn;
    } b;
    struct
    {
      struct elf32_vfp11_erratum_list *branch;
      unsigned int id;
    } v;
  } u;
  elf32_vfp11_erratum_type type;
}
elf32_vfp11_erratum_list;

/* Information about a STM32L4XX erratum veneer, or a branch to such a
   veneer.  */
typedef enum
{
  STM32L4XX_ERRATUM_BRANCH_TO_VENEER,
  STM32L4XX_ERRATUM_VENEER
}
elf32_stm32l4xx_erratum_type;

typedef struct elf32_stm32l4xx_erratum_list
{
  struct elf32_stm32l4xx_erratum_list *next;
  bfd_vma vma;
  union
  {
    struct
    {
      struct elf32_stm32l4xx_erratum_list *veneer;
      unsigned int insn;
    } b;
    struct
    {
      struct elf32_stm32l4xx_erratum_list *branch;
      unsigned int id;
    } v;
  } u;
  elf32_stm32l4xx_erratum_type type;
}
elf32_stm32l4xx_erratum_list;

typedef enum
{
  DELETE_EXIDX_ENTRY,
  INSERT_EXIDX_CANTUNWIND_AT_END
}
arm_unwind_edit_type;

/* A (sorted) list of edits to apply to an unwind table.  */
typedef struct arm_unwind_table_edit
{
  arm_unwind_edit_type type;
  /* Note: we sometimes want to insert an unwind entry corresponding to a
     section different from the one we're currently writing out, so record the
     (text) section this edit relates to here.  */
  asection *linked_section;
  unsigned int index;
  struct arm_unwind_table_edit *next;
}
arm_unwind_table_edit;

typedef struct _arm_elf_section_data
{
  /* Information about mapping symbols.  */
  struct bfd_elf_section_data elf;
  unsigned int mapcount;
  unsigned int mapsize;
  elf32_arm_section_map *map;
  /* Information about CPU errata.  */
  unsigned int erratumcount;
  elf32_vfp11_erratum_list *erratumlist;
  unsigned int stm32l4xx_erratumcount;
  elf32_stm32l4xx_erratum_list *stm32l4xx_erratumlist;
  unsigned int additional_reloc_count;
  /* Information about unwind tables.  */
  union
  {
    /* Unwind info attached to a text section.  */
    struct
    {
      asection *arm_exidx_sec;
    } text;

    /* Unwind info attached to an .ARM.exidx section.  */
    struct
    {
      arm_unwind_table_edit *unwind_edit_list;
      arm_unwind_table_edit *unwind_edit_tail;
    } exidx;
  } u;
}
_arm_elf_section_data;

#define elf32_arm_section_data(sec) \
  ((_arm_elf_section_data *) elf_section_data (sec))

/* A fix which might be required for Cortex-A8 Thumb-2 branch/TLB erratum.
   These fixes are subject to a relaxation procedure (in elf32_arm_size_stubs),
   so may be created multiple times: we use an array of these entries whilst
   relaxing which we can refresh easily, then create stubs for each potentially
   erratum-triggering instruction once we've settled on a solution.  */

struct a8_erratum_fix
{
  bfd *input_bfd;
  asection *section;
  bfd_vma offset;
  bfd_vma target_offset;
  unsigned long orig_insn;
  char *stub_name;
  enum elf32_arm_stub_type stub_type;
  enum arm_st_branch_type branch_type;
};

/* A table of relocs applied to branches which might trigger Cortex-A8
   erratum.  */

struct a8_erratum_reloc
{
  bfd_vma from;
  bfd_vma destination;
  struct elf32_arm_link_hash_entry *hash;
  const char *sym_name;
  unsigned int r_type;
  enum arm_st_branch_type branch_type;
  bool non_a8_stub;
};

/* The size of the thread control block.  */
#define TCB_SIZE	8

/* ARM-specific information about a PLT entry, over and above the usual
   gotplt_union.  */
struct arm_plt_info
{
  /* We reference count Thumb references to a PLT entry separately,
     so that we can emit the Thumb trampoline only if needed.  */
  bfd_signed_vma thumb_refcount;

  /* Some references from Thumb code may be eliminated by BL->BLX
     conversion, so record them separately.  */
  bfd_signed_vma maybe_thumb_refcount;

  /* How many of the recorded PLT accesses were from non-call relocations.
     This information is useful when deciding whether anything takes the
     address of an STT_GNU_IFUNC PLT.  A value of 0 means that all
     non-call references to the function should resolve directly to the
     real runtime target.  */
  unsigned int noncall_refcount;

  /* Since PLT entries have variable size if the Thumb prologue is
     used, we need to record the index into .got.plt instead of
     recomputing it from the PLT offset.  */
  bfd_signed_vma got_offset;
};

/* Information about an .iplt entry for a local STT_GNU_IFUNC symbol.  */
struct arm_local_iplt_info
{
  /* The information that is usually found in the generic ELF part of
     the hash table entry.  */
  union gotplt_union root;

  /* The information that is usually found in the ARM-specific part of
     the hash table entry.  */
  struct arm_plt_info arm;

  /* A list of all potential dynamic relocations against this symbol.  */
  struct elf_dyn_relocs *dyn_relocs;
};

/* Structure to handle FDPIC support for local functions.  */
struct fdpic_local
{
  unsigned int funcdesc_cnt;
  unsigned int gotofffuncdesc_cnt;
  int funcdesc_offset;
};

struct elf_arm_obj_tdata
{
  struct elf_obj_tdata root;

  /* Zero to warn when linking objects with incompatible enum sizes.  */
  int no_enum_size_warning;

  /* Zero to warn when linking objects with incompatible wchar_t sizes.  */
  int no_wchar_size_warning;

  /* The number of entries in each of the arrays in this strcuture.
     Used to avoid buffer overruns.  */
  bfd_size_type num_entries;

  /* tls_type for each local got entry.  */
  char *local_got_tls_type;

  /* GOTPLT entries for TLS descriptors.  */
  bfd_vma *local_tlsdesc_gotent;

  /* Information for local symbols that need entries in .iplt.  */
  struct arm_local_iplt_info **local_iplt;

  /* Maintains FDPIC counters and funcdesc info.  */
  struct fdpic_local *local_fdpic_cnts;
};

#define elf_arm_tdata(bfd) \
  ((struct elf_arm_obj_tdata *) (bfd)->tdata.any)

#define elf32_arm_num_entries(bfd) \
  (elf_arm_tdata (bfd)->num_entries)

#define elf32_arm_local_got_tls_type(bfd) \
  (elf_arm_tdata (bfd)->local_got_tls_type)

#define elf32_arm_local_tlsdesc_gotent(bfd) \
  (elf_arm_tdata (bfd)->local_tlsdesc_gotent)

#define elf32_arm_local_iplt(bfd) \
  (elf_arm_tdata (bfd)->local_iplt)

#define elf32_arm_local_fdpic_cnts(bfd) \
  (elf_arm_tdata (bfd)->local_fdpic_cnts)

#define is_arm_elf(bfd) \
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour \
   && elf_tdata (bfd) != NULL \
   && elf_object_id (bfd) == ARM_ELF_DATA)

static bool
elf32_arm_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_arm_obj_tdata),
				  ARM_ELF_DATA);
}

#define elf32_arm_hash_entry(ent) ((struct elf32_arm_link_hash_entry *)(ent))

/* Structure to handle FDPIC support for extern functions.  */
struct fdpic_global {
  unsigned int gotofffuncdesc_cnt;
  unsigned int gotfuncdesc_cnt;
  unsigned int funcdesc_cnt;
  int funcdesc_offset;
  int gotfuncdesc_offset;
};

/* Arm ELF linker hash entry.  */
struct elf32_arm_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* ARM-specific PLT information.  */
  struct arm_plt_info plt;

#define GOT_UNKNOWN	0
#define GOT_NORMAL	1
#define GOT_TLS_GD	2
#define GOT_TLS_IE	4
#define GOT_TLS_GDESC	8
#define GOT_TLS_GD_ANY_P(type)	((type & GOT_TLS_GD) || (type & GOT_TLS_GDESC))
  unsigned int tls_type : 8;

  /* True if the symbol's PLT entry is in .iplt rather than .plt.  */
  unsigned int is_iplt : 1;

  unsigned int unused : 23;

  /* Offset of the GOTPLT entry reserved for the TLS descriptor,
     starting at the end of the jump table.  */
  bfd_vma tlsdesc_got;

  /* The symbol marking the real symbol location for exported thumb
     symbols with Arm stubs.  */
  struct elf_link_hash_entry *export_glue;

  /* A pointer to the most recently used stub hash entry against this
     symbol.  */
  struct elf32_arm_stub_hash_entry *stub_cache;

  /* Counter for FDPIC relocations against this symbol.  */
  struct fdpic_global fdpic_cnts;
};

/* Traverse an arm ELF linker hash table.  */
#define elf32_arm_link_hash_traverse(table, func, info)			\
  (elf_link_hash_traverse						\
   (&(table)->root,							\
    (bool (*) (struct elf_link_hash_entry *, void *)) (func),		\
    (info)))

/* Get the ARM elf linker hash table from a link_info structure.  */
#define elf32_arm_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == ARM_ELF_DATA)		\
   ? (struct elf32_arm_link_hash_table *) (p)->hash : NULL)

#define arm_stub_hash_lookup(table, string, create, copy) \
  ((struct elf32_arm_stub_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

/* Array to keep track of which stub sections have been created, and
   information on stub grouping.  */
struct map_stub
{
  /* This is the section to which stubs in the group will be
     attached.  */
  asection *link_sec;
  /* The stub section.  */
  asection *stub_sec;
};

#define elf32_arm_compute_jump_table_size(htab) \
  ((htab)->next_tls_desc_index * 4)

/* ARM ELF linker hash table.  */
struct elf32_arm_link_hash_table
{
  /* The main hash table.  */
  struct elf_link_hash_table root;

  /* The size in bytes of the section containing the Thumb-to-ARM glue.  */
  bfd_size_type thumb_glue_size;

  /* The size in bytes of the section containing the ARM-to-Thumb glue.  */
  bfd_size_type arm_glue_size;

  /* The size in bytes of section containing the ARMv4 BX veneers.  */
  bfd_size_type bx_glue_size;

  /* Offsets of ARMv4 BX veneers.  Bit1 set if present, and Bit0 set when
     veneer has been populated.  */
  bfd_vma bx_glue_offset[15];

  /* The size in bytes of the section containing glue for VFP11 erratum
     veneers.  */
  bfd_size_type vfp11_erratum_glue_size;

 /* The size in bytes of the section containing glue for STM32L4XX erratum
     veneers.  */
  bfd_size_type stm32l4xx_erratum_glue_size;

  /* A table of fix locations for Cortex-A8 Thumb-2 branch/TLB erratum.  This
     holds Cortex-A8 erratum fix locations between elf32_arm_size_stubs() and
     elf32_arm_write_section().  */
  struct a8_erratum_fix *a8_erratum_fixes;
  unsigned int num_a8_erratum_fixes;

  /* An arbitrary input BFD chosen to hold the glue sections.  */
  bfd * bfd_of_glue_owner;

  /* Nonzero to output a BE8 image.  */
  int byteswap_code;

  /* Zero if R_ARM_TARGET1 means R_ARM_ABS32.
     Nonzero if R_ARM_TARGET1 means R_ARM_REL32.  */
  int target1_is_rel;

  /* The relocation to use for R_ARM_TARGET2 relocations.  */
  int target2_reloc;

  /* 0 = Ignore R_ARM_V4BX.
     1 = Convert BX to MOV PC.
     2 = Generate v4 interworing stubs.  */
  int fix_v4bx;

  /* Whether we should fix the Cortex-A8 Thumb-2 branch/TLB erratum.  */
  int fix_cortex_a8;

  /* Whether we should fix the ARM1176 BLX immediate issue.  */
  int fix_arm1176;

  /* Nonzero if the ARM/Thumb BLX instructions are available for use.  */
  int use_blx;

  /* What sort of code sequences we should look for which may trigger the
     VFP11 denorm erratum.  */
  bfd_arm_vfp11_fix vfp11_fix;

  /* Global counter for the number of fixes we have emitted.  */
  int num_vfp11_fixes;

  /* What sort of code sequences we should look for which may trigger the
     STM32L4XX erratum.  */
  bfd_arm_stm32l4xx_fix stm32l4xx_fix;

  /* Global counter for the number of fixes we have emitted.  */
  int num_stm32l4xx_fixes;

  /* Nonzero to force PIC branch veneers.  */
  int pic_veneer;

  /* The number of bytes in the initial entry in the PLT.  */
  bfd_size_type plt_header_size;

  /* The number of bytes in the subsequent PLT etries.  */
  bfd_size_type plt_entry_size;

  /* True if the target uses REL relocations.  */
  bool use_rel;

  /* Nonzero if import library must be a secure gateway import library
     as per ARMv8-M Security Extensions.  */
  int cmse_implib;

  /* The import library whose symbols' address must remain stable in
     the import library generated.  */
  bfd *in_implib_bfd;

  /* The index of the next unused R_ARM_TLS_DESC slot in .rel.plt.  */
  bfd_vma next_tls_desc_index;

  /* How many R_ARM_TLS_DESC relocations were generated so far.  */
  bfd_vma num_tls_desc;

  /* The (unloaded but important) VxWorks .rela.plt.unloaded section.  */
  asection *srelplt2;

  /* Offset in .plt section of tls_arm_trampoline.  */
  bfd_vma tls_trampoline;

  /* Data for R_ARM_TLS_LDM32/R_ARM_TLS_LDM32_FDPIC relocations.  */
  union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } tls_ldm_got;

  /* For convenience in allocate_dynrelocs.  */
  bfd * obfd;

  /* The amount of space used by the reserved portion of the sgotplt
     section, plus whatever space is used by the jump slots.  */
  bfd_vma sgotplt_jump_table_size;

  /* The stub hash table.  */
  struct bfd_hash_table stub_hash_table;

  /* Linker stub bfd.  */
  bfd *stub_bfd;

  /* Linker call-backs.  */
  asection * (*add_stub_section) (const char *, asection *, asection *,
				  unsigned int);
  void (*layout_sections_again) (void);

  /* Array to keep track of which stub sections have been created, and
     information on stub grouping.  */
  struct map_stub *stub_group;

  /* Input stub section holding secure gateway veneers.  */
  asection *cmse_stub_sec;

  /* Offset in cmse_stub_sec where new SG veneers (not in input import library)
     start to be allocated.  */
  bfd_vma new_cmse_stub_offset;

  /* Number of elements in stub_group.  */
  unsigned int top_id;

  /* Assorted information used by elf32_arm_size_stubs.  */
  unsigned int bfd_count;
  unsigned int top_index;
  asection **input_list;

  /* True if the target system uses FDPIC. */
  int fdpic_p;

  /* Fixup section. Used for FDPIC.  */
  asection *srofixup;
};

/* Add an FDPIC read-only fixup.  */
static void
arm_elf_add_rofixup (bfd *output_bfd, asection *srofixup, bfd_vma offset)
{
  bfd_vma fixup_offset;

  fixup_offset = srofixup->reloc_count++ * 4;
  BFD_ASSERT (fixup_offset < srofixup->size);
  bfd_put_32 (output_bfd, offset, srofixup->contents + fixup_offset);
}

static inline int
ctz (unsigned int mask)
{
#if GCC_VERSION >= 3004
  return __builtin_ctz (mask);
#else
  unsigned int i;

  for (i = 0; i < 8 * sizeof (mask); i++)
    {
      if (mask & 0x1)
	break;
      mask = (mask >> 1);
    }
  return i;
#endif
}

static inline int
elf32_arm_popcount (unsigned int mask)
{
#if GCC_VERSION >= 3004
  return __builtin_popcount (mask);
#else
  unsigned int i;
  int sum = 0;

  for (i = 0; i < 8 * sizeof (mask); i++)
    {
      if (mask & 0x1)
	sum++;
      mask = (mask >> 1);
    }
  return sum;
#endif
}

static void elf32_arm_add_dynreloc (bfd *output_bfd, struct bfd_link_info *info,
				    asection *sreloc, Elf_Internal_Rela *rel);

static void
arm_elf_fill_funcdesc (bfd *output_bfd,
		       struct bfd_link_info *info,
		       int *funcdesc_offset,
		       int dynindx,
		       int offset,
		       bfd_vma addr,
		       bfd_vma dynreloc_value,
		       bfd_vma seg)
{
  if ((*funcdesc_offset & 1) == 0)
    {
      struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (info);
      asection *sgot = globals->root.sgot;

      if (bfd_link_pic (info))
	{
	  asection *srelgot = globals->root.srelgot;
	  Elf_Internal_Rela outrel;

	  outrel.r_info = ELF32_R_INFO (dynindx, R_ARM_FUNCDESC_VALUE);
	  outrel.r_offset = sgot->output_section->vma + sgot->output_offset + offset;
	  outrel.r_addend = 0;

	  elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
	  bfd_put_32 (output_bfd, addr, sgot->contents + offset);
	  bfd_put_32 (output_bfd, seg, sgot->contents + offset + 4);
	}
      else
	{
	  struct elf_link_hash_entry *hgot = globals->root.hgot;
	  bfd_vma got_value = hgot->root.u.def.value
	    + hgot->root.u.def.section->output_section->vma
	    + hgot->root.u.def.section->output_offset;

	  arm_elf_add_rofixup (output_bfd, globals->srofixup,
			       sgot->output_section->vma + sgot->output_offset
			       + offset);
	  arm_elf_add_rofixup (output_bfd, globals->srofixup,
			       sgot->output_section->vma + sgot->output_offset
			       + offset + 4);
	  bfd_put_32 (output_bfd, dynreloc_value, sgot->contents + offset);
	  bfd_put_32 (output_bfd, got_value, sgot->contents + offset + 4);
	}
      *funcdesc_offset |= 1;
    }
}

/* Create an entry in an ARM ELF linker hash table.  */

static struct bfd_hash_entry *
elf32_arm_link_hash_newfunc (struct bfd_hash_entry * entry,
			     struct bfd_hash_table * table,
			     const char * string)
{
  struct elf32_arm_link_hash_entry * ret =
    (struct elf32_arm_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = (struct elf32_arm_link_hash_entry *)
	bfd_hash_allocate (table, sizeof (struct elf32_arm_link_hash_entry));
  if (ret == NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf32_arm_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != NULL)
    {
      ret->tls_type = GOT_UNKNOWN;
      ret->tlsdesc_got = (bfd_vma) -1;
      ret->plt.thumb_refcount = 0;
      ret->plt.maybe_thumb_refcount = 0;
      ret->plt.noncall_refcount = 0;
      ret->plt.got_offset = -1;
      ret->is_iplt = false;
      ret->export_glue = NULL;

      ret->stub_cache = NULL;

      ret->fdpic_cnts.gotofffuncdesc_cnt = 0;
      ret->fdpic_cnts.gotfuncdesc_cnt = 0;
      ret->fdpic_cnts.funcdesc_cnt = 0;
      ret->fdpic_cnts.funcdesc_offset = -1;
      ret->fdpic_cnts.gotfuncdesc_offset = -1;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Ensure that we have allocated bookkeeping structures for ABFD's local
   symbols.  */

static bool
elf32_arm_allocate_local_sym_info (bfd *abfd)
{
  if (elf_local_got_refcounts (abfd) == NULL)
    {
      bfd_size_type num_syms;

      elf32_arm_num_entries (abfd) = 0;

      /* Whilst it might be tempting to allocate a single block of memory and
	 then divide it up amoungst the arrays in the elf_arm_obj_tdata
	 structure, this interferes with the work of memory checkers looking
	 for buffer overruns.  So allocate each array individually.  */

      num_syms = elf_tdata (abfd)->symtab_hdr.sh_info;

      elf_local_got_refcounts (abfd) = bfd_zalloc
	(abfd, num_syms * sizeof (* elf_local_got_refcounts (abfd)));

      if (elf_local_got_refcounts (abfd) == NULL)
	return false;

      elf32_arm_local_tlsdesc_gotent (abfd) = bfd_zalloc
	(abfd, num_syms * sizeof (* elf32_arm_local_tlsdesc_gotent (abfd)));

      if (elf32_arm_local_tlsdesc_gotent (abfd) == NULL)
	return false;

      elf32_arm_local_iplt (abfd) = bfd_zalloc
	(abfd, num_syms * sizeof (* elf32_arm_local_iplt (abfd)));

      if (elf32_arm_local_iplt (abfd) == NULL)
	return false;

      elf32_arm_local_fdpic_cnts (abfd) = bfd_zalloc
	(abfd, num_syms * sizeof (* elf32_arm_local_fdpic_cnts (abfd)));

      if (elf32_arm_local_fdpic_cnts (abfd) == NULL)
	return false;

      elf32_arm_local_got_tls_type (abfd) = bfd_zalloc
	(abfd, num_syms * sizeof (* elf32_arm_local_got_tls_type (abfd)));

      if (elf32_arm_local_got_tls_type (abfd) == NULL)
	return false;

      elf32_arm_num_entries (abfd) = num_syms;

#if GCC_VERSION >= 3000
      BFD_ASSERT (__alignof__ (*elf32_arm_local_tlsdesc_gotent (abfd))
		  <= __alignof__ (*elf_local_got_refcounts (abfd)));
      BFD_ASSERT (__alignof__ (*elf32_arm_local_iplt (abfd))
		  <= __alignof__ (*elf32_arm_local_tlsdesc_gotent (abfd)));
      BFD_ASSERT (__alignof__ (*elf32_arm_local_fdpic_cnts (abfd))
		  <= __alignof__ (*elf32_arm_local_iplt (abfd)));
      BFD_ASSERT (__alignof__ (*elf32_arm_local_got_tls_type (abfd))
		  <= __alignof__ (*elf32_arm_local_fdpic_cnts (abfd)));
#endif
    }
  return true;
}

/* Return the .iplt information for local symbol R_SYMNDX, which belongs
   to input bfd ABFD.  Create the information if it doesn't already exist.
   Return null if an allocation fails.  */

static struct arm_local_iplt_info *
elf32_arm_create_local_iplt (bfd *abfd, unsigned long r_symndx)
{
  struct arm_local_iplt_info **ptr;

  if (!elf32_arm_allocate_local_sym_info (abfd))
    return NULL;

  BFD_ASSERT (r_symndx < elf_tdata (abfd)->symtab_hdr.sh_info);
  BFD_ASSERT (r_symndx < elf32_arm_num_entries (abfd));
  ptr = &elf32_arm_local_iplt (abfd)[r_symndx];
  if (*ptr == NULL)
    *ptr = bfd_zalloc (abfd, sizeof (**ptr));
  return *ptr;
}

/* Try to obtain PLT information for the symbol with index R_SYMNDX
   in ABFD's symbol table.  If the symbol is global, H points to its
   hash table entry, otherwise H is null.

   Return true if the symbol does have PLT information.  When returning
   true, point *ROOT_PLT at the target-independent reference count/offset
   union and *ARM_PLT at the ARM-specific information.  */

static bool
elf32_arm_get_plt_info (bfd *abfd, struct elf32_arm_link_hash_table *globals,
			struct elf32_arm_link_hash_entry *h,
			unsigned long r_symndx, union gotplt_union **root_plt,
			struct arm_plt_info **arm_plt)
{
  struct arm_local_iplt_info *local_iplt;

  if (globals->root.splt == NULL && globals->root.iplt == NULL)
    return false;

  if (h != NULL)
    {
      *root_plt = &h->root.plt;
      *arm_plt = &h->plt;
      return true;
    }

  if (elf32_arm_local_iplt (abfd) == NULL)
    return false;

  if (r_symndx >= elf32_arm_num_entries (abfd))
    return false;

  local_iplt = elf32_arm_local_iplt (abfd)[r_symndx];
  if (local_iplt == NULL)
    return false;

  *root_plt = &local_iplt->root;
  *arm_plt = &local_iplt->arm;
  return true;
}

static bool using_thumb_only (struct elf32_arm_link_hash_table *globals);

/* Return true if the PLT described by ARM_PLT requires a Thumb stub
   before it.  */

static bool
elf32_arm_plt_needs_thumb_stub_p (struct bfd_link_info *info,
				  struct arm_plt_info *arm_plt)
{
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);

  return (!using_thumb_only (htab) && (arm_plt->thumb_refcount != 0
	  || (!htab->use_blx && arm_plt->maybe_thumb_refcount != 0)));
}

/* Return a pointer to the head of the dynamic reloc list that should
   be used for local symbol ISYM, which is symbol number R_SYMNDX in
   ABFD's symbol table.  Return null if an error occurs.  */

static struct elf_dyn_relocs **
elf32_arm_get_local_dynreloc_list (bfd *abfd, unsigned long r_symndx,
				   Elf_Internal_Sym *isym)
{
  if (ELF32_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
    {
      struct arm_local_iplt_info *local_iplt;

      local_iplt = elf32_arm_create_local_iplt (abfd, r_symndx);
      if (local_iplt == NULL)
	return NULL;
      return &local_iplt->dyn_relocs;
    }
  else
    {
      /* Track dynamic relocs needed for local syms too.
	 We really need local syms available to do this
	 easily.  Oh well.  */
      asection *s;
      void *vpp;

      s = bfd_section_from_elf_index (abfd, isym->st_shndx);
      if (s == NULL)
	return NULL;

      vpp = &elf_section_data (s)->local_dynrel;
      return (struct elf_dyn_relocs **) vpp;
    }
}

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
      entry = (struct bfd_hash_entry *)
	  bfd_hash_allocate (table, sizeof (struct elf32_arm_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf32_arm_stub_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct elf32_arm_stub_hash_entry *) entry;
      eh->stub_sec = NULL;
      eh->stub_offset = (bfd_vma) -1;
      eh->source_value = 0;
      eh->target_value = 0;
      eh->target_section = NULL;
      eh->orig_insn = 0;
      eh->stub_type = arm_stub_none;
      eh->stub_size = 0;
      eh->stub_template = NULL;
      eh->stub_template_size = -1;
      eh->h = NULL;
      eh->id_sec = NULL;
      eh->output_name = NULL;
    }

  return entry;
}

/* Create .got, .gotplt, and .rel(a).got sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */

static bool
create_got_section (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  if (! _bfd_elf_create_got_section (dynobj, info))
    return false;

  /* Also create .rofixup.  */
  if (htab->fdpic_p)
    {
      htab->srofixup = bfd_make_section_with_flags (dynobj, ".rofixup",
						    (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
						     | SEC_IN_MEMORY | SEC_LINKER_CREATED | SEC_READONLY));
      if (htab->srofixup == NULL
	  || !bfd_set_section_alignment (htab->srofixup, 2))
	return false;
    }

  return true;
}

/* Create the .iplt, .rel(a).iplt and .igot.plt sections.  */

static bool
create_ifunc_sections (struct bfd_link_info *info)
{
  struct elf32_arm_link_hash_table *htab;
  const struct elf_backend_data *bed;
  bfd *dynobj;
  asection *s;
  flagword flags;

  htab = elf32_arm_hash_table (info);
  dynobj = htab->root.dynobj;
  bed = get_elf_backend_data (dynobj);
  flags = bed->dynamic_sec_flags;

  if (htab->root.iplt == NULL)
    {
      s = bfd_make_section_anyway_with_flags (dynobj, ".iplt",
					      flags | SEC_READONLY | SEC_CODE);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->plt_alignment))
	return false;
      htab->root.iplt = s;
    }

  if (htab->root.irelplt == NULL)
    {
      s = bfd_make_section_anyway_with_flags (dynobj,
					      RELOC_SECTION (htab, ".iplt"),
					      flags | SEC_READONLY);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->root.irelplt = s;
    }

  if (htab->root.igotplt == NULL)
    {
      s = bfd_make_section_anyway_with_flags (dynobj, ".igot.plt", flags);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->root.igotplt = s;
    }
  return true;
}

/* Determine if we're dealing with a Thumb only architecture.  */

static bool
using_thumb_only (struct elf32_arm_link_hash_table *globals)
{
  int arch;
  int profile = bfd_elf_get_obj_attr_int (globals->obfd, OBJ_ATTR_PROC,
					  Tag_CPU_arch_profile);

  if (profile)
    return profile == 'M';

  arch = bfd_elf_get_obj_attr_int (globals->obfd, OBJ_ATTR_PROC, Tag_CPU_arch);

  /* Force return logic to be reviewed for each new architecture.  */
  BFD_ASSERT (arch <= TAG_CPU_ARCH_V8_1M_MAIN);

  if (arch == TAG_CPU_ARCH_V6_M
      || arch == TAG_CPU_ARCH_V6S_M
      || arch == TAG_CPU_ARCH_V7E_M
      || arch == TAG_CPU_ARCH_V8M_BASE
      || arch == TAG_CPU_ARCH_V8M_MAIN
      || arch == TAG_CPU_ARCH_V8_1M_MAIN)
    return true;

  return false;
}

/* Determine if we're dealing with a Thumb-2 object.  */

static bool
using_thumb2 (struct elf32_arm_link_hash_table *globals)
{
  int arch;
  int thumb_isa = bfd_elf_get_obj_attr_int (globals->obfd, OBJ_ATTR_PROC,
					    Tag_THUMB_ISA_use);

  /* No use of thumb permitted, or a legacy thumb-1/2 definition.  */
  if (thumb_isa < 3)
    return thumb_isa == 2;

  /* Variant of thumb is described by the architecture tag.  */
  arch = bfd_elf_get_obj_attr_int (globals->obfd, OBJ_ATTR_PROC, Tag_CPU_arch);

  /* Force return logic to be reviewed for each new architecture.  */
  BFD_ASSERT (arch <= TAG_CPU_ARCH_V8_1M_MAIN);

  return (arch == TAG_CPU_ARCH_V6T2
	  || arch == TAG_CPU_ARCH_V7
	  || arch == TAG_CPU_ARCH_V7E_M
	  || arch == TAG_CPU_ARCH_V8
	  || arch == TAG_CPU_ARCH_V8R
	  || arch == TAG_CPU_ARCH_V8M_MAIN
	  || arch == TAG_CPU_ARCH_V8_1M_MAIN);
}

/* Determine whether Thumb-2 BL instruction is available.  */

static bool
using_thumb2_bl (struct elf32_arm_link_hash_table *globals)
{
  int arch =
    bfd_elf_get_obj_attr_int (globals->obfd, OBJ_ATTR_PROC, Tag_CPU_arch);

  /* Force return logic to be reviewed for each new architecture.  */
  BFD_ASSERT (arch <= TAG_CPU_ARCH_V9);

  /* Architecture was introduced after ARMv6T2 (eg. ARMv6-M).  */
  return (arch == TAG_CPU_ARCH_V6T2
	  || arch >= TAG_CPU_ARCH_V7);
}

/* Create .plt, .rel(a).plt, .got, .got.plt, .rel(a).got, .dynbss, and
   .rel(a).bss sections in DYNOBJ, and set up shortcuts to them in our
   hash table.  */

static bool
elf32_arm_create_dynamic_sections (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  if (!htab->root.sgot && !create_got_section (dynobj, info))
    return false;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return false;

  if (htab->root.target_os == is_vxworks)
    {
      if (!elf_vxworks_create_dynamic_sections (dynobj, info, &htab->srelplt2))
	return false;

      if (bfd_link_pic (info))
	{
	  htab->plt_header_size = 0;
	  htab->plt_entry_size
	    = 4 * ARRAY_SIZE (elf32_arm_vxworks_shared_plt_entry);
	}
      else
	{
	  htab->plt_header_size
	    = 4 * ARRAY_SIZE (elf32_arm_vxworks_exec_plt0_entry);
	  htab->plt_entry_size
	    = 4 * ARRAY_SIZE (elf32_arm_vxworks_exec_plt_entry);
	}

      if (elf_elfheader (dynobj))
	elf_elfheader (dynobj)->e_ident[EI_CLASS] = ELFCLASS32;
    }
  else
    {
      /* PR ld/16017
	 Test for thumb only architectures.  Note - we cannot just call
	 using_thumb_only() as the attributes in the output bfd have not been
	 initialised at this point, so instead we use the input bfd.  */
      bfd * saved_obfd = htab->obfd;

      htab->obfd = dynobj;
      if (using_thumb_only (htab))
	{
	  htab->plt_header_size = 4 * ARRAY_SIZE (elf32_thumb2_plt0_entry);
	  htab->plt_entry_size  = 4 * ARRAY_SIZE (elf32_thumb2_plt_entry);
	}
      htab->obfd = saved_obfd;
    }

  if (htab->fdpic_p) {
    htab->plt_header_size = 0;
    if (info->flags & DF_BIND_NOW)
      htab->plt_entry_size = 4 * (ARRAY_SIZE (elf32_arm_fdpic_plt_entry) - 5);
    else
      htab->plt_entry_size = 4 * ARRAY_SIZE (elf32_arm_fdpic_plt_entry);
  }

  if (!htab->root.splt
      || !htab->root.srelplt
      || !htab->root.sdynbss
      || (!bfd_link_pic (info) && !htab->root.srelbss))
    abort ();

  return true;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
elf32_arm_copy_indirect_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *dir,
				struct elf_link_hash_entry *ind)
{
  struct elf32_arm_link_hash_entry *edir, *eind;

  edir = (struct elf32_arm_link_hash_entry *) dir;
  eind = (struct elf32_arm_link_hash_entry *) ind;

  if (ind->root.type == bfd_link_hash_indirect)
    {
      /* Copy over PLT info.  */
      edir->plt.thumb_refcount += eind->plt.thumb_refcount;
      eind->plt.thumb_refcount = 0;
      edir->plt.maybe_thumb_refcount += eind->plt.maybe_thumb_refcount;
      eind->plt.maybe_thumb_refcount = 0;
      edir->plt.noncall_refcount += eind->plt.noncall_refcount;
      eind->plt.noncall_refcount = 0;

      /* Copy FDPIC counters.  */
      edir->fdpic_cnts.gotofffuncdesc_cnt += eind->fdpic_cnts.gotofffuncdesc_cnt;
      edir->fdpic_cnts.gotfuncdesc_cnt += eind->fdpic_cnts.gotfuncdesc_cnt;
      edir->fdpic_cnts.funcdesc_cnt += eind->fdpic_cnts.funcdesc_cnt;

      /* We should only allocate a function to .iplt once the final
	 symbol information is known.  */
      BFD_ASSERT (!eind->is_iplt);

      if (dir->got.refcount <= 0)
	{
	  edir->tls_type = eind->tls_type;
	  eind->tls_type = GOT_UNKNOWN;
	}
    }

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Destroy an ARM elf linker hash table.  */

static void
elf32_arm_link_hash_table_free (bfd *obfd)
{
  struct elf32_arm_link_hash_table *ret
    = (struct elf32_arm_link_hash_table *) obfd->link.hash;

  bfd_hash_table_free (&ret->stub_hash_table);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create an ARM elf linker hash table.  */

static struct bfd_link_hash_table *
elf32_arm_link_hash_table_create (bfd *abfd)
{
  struct elf32_arm_link_hash_table *ret;
  size_t amt = sizeof (struct elf32_arm_link_hash_table);

  ret = (struct elf32_arm_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (& ret->root, abfd,
				      elf32_arm_link_hash_newfunc,
				      sizeof (struct elf32_arm_link_hash_entry),
				      ARM_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->vfp11_fix = BFD_ARM_VFP11_FIX_NONE;
  ret->stm32l4xx_fix = BFD_ARM_STM32L4XX_FIX_NONE;
#ifdef FOUR_WORD_PLT
  ret->plt_header_size = 16;
  ret->plt_entry_size = 16;
#else
  ret->plt_header_size = 20;
  ret->plt_entry_size = elf32_arm_use_long_plt_entry ? 16 : 12;
#endif
  ret->use_rel = true;
  ret->obfd = abfd;
  ret->fdpic_p = 0;

  if (!bfd_hash_table_init (&ret->stub_hash_table, stub_hash_newfunc,
			    sizeof (struct elf32_arm_stub_hash_entry)))
    {
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }
  ret->root.root.hash_table_free = elf32_arm_link_hash_table_free;

  return &ret->root.root;
}

/* Determine what kind of NOPs are available.  */

static bool
arch_has_arm_nop (struct elf32_arm_link_hash_table *globals)
{
  const int arch = bfd_elf_get_obj_attr_int (globals->obfd, OBJ_ATTR_PROC,
					     Tag_CPU_arch);

  /* Force return logic to be reviewed for each new architecture.  */
  BFD_ASSERT (arch <= TAG_CPU_ARCH_V9);

  return (arch == TAG_CPU_ARCH_V6T2
	  || arch == TAG_CPU_ARCH_V6K
	  || arch == TAG_CPU_ARCH_V7
	  || arch == TAG_CPU_ARCH_V8
	  || arch == TAG_CPU_ARCH_V8R
	  || arch == TAG_CPU_ARCH_V9);
}

static bool
arm_stub_is_thumb (enum elf32_arm_stub_type stub_type)
{
  switch (stub_type)
    {
    case arm_stub_long_branch_thumb_only:
    case arm_stub_long_branch_thumb2_only:
    case arm_stub_long_branch_thumb2_only_pure:
    case arm_stub_long_branch_v4t_thumb_arm:
    case arm_stub_short_branch_v4t_thumb_arm:
    case arm_stub_long_branch_v4t_thumb_arm_pic:
    case arm_stub_long_branch_v4t_thumb_tls_pic:
    case arm_stub_long_branch_thumb_only_pic:
    case arm_stub_cmse_branch_thumb_only:
      return true;
    case arm_stub_none:
      BFD_FAIL ();
      return false;
      break;
    default:
      return false;
    }
}

/* Determine the type of stub needed, if any, for a call.  */

static enum elf32_arm_stub_type
arm_type_of_stub (struct bfd_link_info *info,
		  asection *input_sec,
		  const Elf_Internal_Rela *rel,
		  unsigned char st_type,
		  enum arm_st_branch_type *actual_branch_type,
		  struct elf32_arm_link_hash_entry *hash,
		  bfd_vma destination,
		  asection *sym_sec,
		  bfd *input_bfd,
		  const char *name)
{
  bfd_vma location;
  bfd_signed_vma branch_offset;
  unsigned int r_type;
  struct elf32_arm_link_hash_table * globals;
  bool thumb2, thumb2_bl, thumb_only;
  enum elf32_arm_stub_type stub_type = arm_stub_none;
  int use_plt = 0;
  enum arm_st_branch_type branch_type = *actual_branch_type;
  union gotplt_union *root_plt;
  struct arm_plt_info *arm_plt;
  int arch;
  int thumb2_movw;

  if (branch_type == ST_BRANCH_LONG)
    return stub_type;

  globals = elf32_arm_hash_table (info);
  if (globals == NULL)
    return stub_type;

  thumb_only = using_thumb_only (globals);
  thumb2 = using_thumb2 (globals);
  thumb2_bl = using_thumb2_bl (globals);

  arch = bfd_elf_get_obj_attr_int (globals->obfd, OBJ_ATTR_PROC, Tag_CPU_arch);

  /* True for architectures that implement the thumb2 movw instruction.  */
  thumb2_movw = thumb2 || (arch  == TAG_CPU_ARCH_V8M_BASE);

  /* Determine where the call point is.  */
  location = (input_sec->output_offset
	      + input_sec->output_section->vma
	      + rel->r_offset);

  r_type = ELF32_R_TYPE (rel->r_info);

  /* ST_BRANCH_TO_ARM is nonsense to thumb-only targets when we
     are considering a function call relocation.  */
  if (thumb_only && (r_type == R_ARM_THM_CALL || r_type == R_ARM_THM_JUMP24
		     || r_type == R_ARM_THM_JUMP19)
      && branch_type == ST_BRANCH_TO_ARM)
    branch_type = ST_BRANCH_TO_THUMB;

  /* For TLS call relocs, it is the caller's responsibility to provide
     the address of the appropriate trampoline.  */
  if (r_type != R_ARM_TLS_CALL
      && r_type != R_ARM_THM_TLS_CALL
      && elf32_arm_get_plt_info (input_bfd, globals, hash,
				 ELF32_R_SYM (rel->r_info), &root_plt,
				 &arm_plt)
      && root_plt->offset != (bfd_vma) -1)
    {
      asection *splt;

      if (hash == NULL || hash->is_iplt)
	splt = globals->root.iplt;
      else
	splt = globals->root.splt;
      if (splt != NULL)
	{
	  use_plt = 1;

	  /* Note when dealing with PLT entries: the main PLT stub is in
	     ARM mode, so if the branch is in Thumb mode, another
	     Thumb->ARM stub will be inserted later just before the ARM
	     PLT stub. If a long branch stub is needed, we'll add a
	     Thumb->Arm one and branch directly to the ARM PLT entry.
	     Here, we have to check if a pre-PLT Thumb->ARM stub
	     is needed and if it will be close enough.  */

	  destination = (splt->output_section->vma
			 + splt->output_offset
			 + root_plt->offset);
	  st_type = STT_FUNC;

	  /* Thumb branch/call to PLT: it can become a branch to ARM
	     or to Thumb. We must perform the same checks and
	     corrections as in elf32_arm_final_link_relocate.  */
	  if ((r_type == R_ARM_THM_CALL)
	      || (r_type == R_ARM_THM_JUMP24))
	    {
	      if (globals->use_blx
		  && r_type == R_ARM_THM_CALL
		  && !thumb_only)
		{
		  /* If the Thumb BLX instruction is available, convert
		     the BL to a BLX instruction to call the ARM-mode
		     PLT entry.  */
		  branch_type = ST_BRANCH_TO_ARM;
		}
	      else
		{
		  if (!thumb_only)
		    /* Target the Thumb stub before the ARM PLT entry.  */
		    destination -= PLT_THUMB_STUB_SIZE;
		  branch_type = ST_BRANCH_TO_THUMB;
		}
	    }
	  else
	    {
	      branch_type = ST_BRANCH_TO_ARM;
	    }
	}
    }
  /* Calls to STT_GNU_IFUNC symbols should go through a PLT.  */
  BFD_ASSERT (st_type != STT_GNU_IFUNC);

  branch_offset = (bfd_signed_vma)(destination - location);

  if (r_type == R_ARM_THM_CALL || r_type == R_ARM_THM_JUMP24
      || r_type == R_ARM_THM_TLS_CALL || r_type == R_ARM_THM_JUMP19)
    {
      /* Handle cases where:
	 - this call goes too far (different Thumb/Thumb2 max
	   distance)
	 - it's a Thumb->Arm call and blx is not available, or it's a
	   Thumb->Arm branch (not bl). A stub is needed in this case,
	   but only if this call is not through a PLT entry. Indeed,
	   PLT stubs handle mode switching already.  */
      if ((!thumb2_bl
	    && (branch_offset > THM_MAX_FWD_BRANCH_OFFSET
		|| (branch_offset < THM_MAX_BWD_BRANCH_OFFSET)))
	  || (thumb2_bl
	      && (branch_offset > THM2_MAX_FWD_BRANCH_OFFSET
		  || (branch_offset < THM2_MAX_BWD_BRANCH_OFFSET)))
	  || (thumb2
	      && (branch_offset > THM2_MAX_FWD_COND_BRANCH_OFFSET
		  || (branch_offset < THM2_MAX_BWD_COND_BRANCH_OFFSET))
	      && (r_type == R_ARM_THM_JUMP19))
	  || (branch_type == ST_BRANCH_TO_ARM
	      && (((r_type == R_ARM_THM_CALL
		    || r_type == R_ARM_THM_TLS_CALL) && !globals->use_blx)
		  || (r_type == R_ARM_THM_JUMP24)
		  || (r_type == R_ARM_THM_JUMP19))
	      && !use_plt))
	{
	  /* If we need to insert a Thumb-Thumb long branch stub to a
	     PLT, use one that branches directly to the ARM PLT
	     stub. If we pretended we'd use the pre-PLT Thumb->ARM
	     stub, undo this now.  */
	  if ((branch_type == ST_BRANCH_TO_THUMB) && use_plt && !thumb_only)
	    {
	      branch_type = ST_BRANCH_TO_ARM;
	      branch_offset += PLT_THUMB_STUB_SIZE;
	    }

	  if (branch_type == ST_BRANCH_TO_THUMB)
	    {
	      /* Thumb to thumb.  */
	      if (!thumb_only)
		{
		  if (input_sec->flags & SEC_ELF_PURECODE)
		    _bfd_error_handler
		      (_("%pB(%pA): warning: long branch veneers used in"
			 " section with SHF_ARM_PURECODE section"
			 " attribute is only supported for M-profile"
			 " targets that implement the movw instruction"),
		       input_bfd, input_sec);

		  stub_type = (bfd_link_pic (info) | globals->pic_veneer)
		    /* PIC stubs.  */
		    ? ((globals->use_blx
			&& (r_type == R_ARM_THM_CALL))
		       /* V5T and above. Stub starts with ARM code, so
			  we must be able to switch mode before
			  reaching it, which is only possible for 'bl'
			  (ie R_ARM_THM_CALL relocation).  */
		       ? arm_stub_long_branch_any_thumb_pic
		       /* On V4T, use Thumb code only.  */
		       : arm_stub_long_branch_v4t_thumb_thumb_pic)

		    /* non-PIC stubs.  */
		    : ((globals->use_blx
			&& (r_type == R_ARM_THM_CALL))
		       /* V5T and above.  */
		       ? arm_stub_long_branch_any_any
		       /* V4T.  */
		       : arm_stub_long_branch_v4t_thumb_thumb);
		}
	      else
		{
		  if (thumb2_movw && (input_sec->flags & SEC_ELF_PURECODE))
		      stub_type = arm_stub_long_branch_thumb2_only_pure;
		  else
		    {
		      if (input_sec->flags & SEC_ELF_PURECODE)
			_bfd_error_handler
			  (_("%pB(%pA): warning: long branch veneers used in"
			     " section with SHF_ARM_PURECODE section"
			     " attribute is only supported for M-profile"
			     " targets that implement the movw instruction"),
			   input_bfd, input_sec);

		      stub_type = (bfd_link_pic (info) | globals->pic_veneer)
			/* PIC stub.  */
			? arm_stub_long_branch_thumb_only_pic
			/* non-PIC stub.  */
			: (thumb2 ? arm_stub_long_branch_thumb2_only
				  : arm_stub_long_branch_thumb_only);
		    }
		}
	    }
	  else
	    {
	      if (input_sec->flags & SEC_ELF_PURECODE)
		_bfd_error_handler
		  (_("%pB(%pA): warning: long branch veneers used in"
		     " section with SHF_ARM_PURECODE section"
		     " attribute is only supported" " for M-profile"
		     " targets that implement the movw instruction"),
		   input_bfd, input_sec);

	      /* Thumb to arm.  */
	      if (sym_sec != NULL
		  && sym_sec->owner != NULL
		  && !INTERWORK_FLAG (sym_sec->owner))
		{
		  _bfd_error_handler
		    (_("%pB(%s): warning: interworking not enabled;"
		       " first occurrence: %pB: %s call to %s"),
		     sym_sec->owner, name, input_bfd, "Thumb", "ARM");
		}

	      stub_type =
		(bfd_link_pic (info) | globals->pic_veneer)
		/* PIC stubs.  */
		? (r_type == R_ARM_THM_TLS_CALL
		   /* TLS PIC stubs.  */
		   ? (globals->use_blx ? arm_stub_long_branch_any_tls_pic
		      : arm_stub_long_branch_v4t_thumb_tls_pic)
		   : ((globals->use_blx && r_type == R_ARM_THM_CALL)
		      /* V5T PIC and above.  */
		      ? arm_stub_long_branch_any_arm_pic
		      /* V4T PIC stub.  */
		      : arm_stub_long_branch_v4t_thumb_arm_pic))

		/* non-PIC stubs.  */
		: ((globals->use_blx && r_type == R_ARM_THM_CALL)
		   /* V5T and above.  */
		   ? arm_stub_long_branch_any_any
		   /* V4T.  */
		   : arm_stub_long_branch_v4t_thumb_arm);

	      /* Handle v4t short branches.  */
	      if ((stub_type == arm_stub_long_branch_v4t_thumb_arm)
		  && (branch_offset <= THM_MAX_FWD_BRANCH_OFFSET)
		  && (branch_offset >= THM_MAX_BWD_BRANCH_OFFSET))
		stub_type = arm_stub_short_branch_v4t_thumb_arm;
	    }
	}
    }
  else if (r_type == R_ARM_CALL
	   || r_type == R_ARM_JUMP24
	   || r_type == R_ARM_PLT32
	   || r_type == R_ARM_TLS_CALL)
    {
      if (input_sec->flags & SEC_ELF_PURECODE)
	_bfd_error_handler
	  (_("%pB(%pA): warning: long branch veneers used in"
	     " section with SHF_ARM_PURECODE section"
	     " attribute is only supported for M-profile"
	     " targets that implement the movw instruction"),
	   input_bfd, input_sec);
      if (branch_type == ST_BRANCH_TO_THUMB)
	{
	  /* Arm to thumb.  */

	  if (sym_sec != NULL
	      && sym_sec->owner != NULL
	      && !INTERWORK_FLAG (sym_sec->owner))
	    {
	      _bfd_error_handler
		(_("%pB(%s): warning: interworking not enabled;"
		   " first occurrence: %pB: %s call to %s"),
		 sym_sec->owner, name, input_bfd, "ARM", "Thumb");
	    }

	  /* We have an extra 2-bytes reach because of
	     the mode change (bit 24 (H) of BLX encoding).  */
	  if (branch_offset > (ARM_MAX_FWD_BRANCH_OFFSET + 2)
	      || (branch_offset < ARM_MAX_BWD_BRANCH_OFFSET)
	      || (r_type == R_ARM_CALL && !globals->use_blx)
	      || (r_type == R_ARM_JUMP24)
	      || (r_type == R_ARM_PLT32))
	    {
	      stub_type = (bfd_link_pic (info) | globals->pic_veneer)
		/* PIC stubs.  */
		? ((globals->use_blx)
		   /* V5T and above.  */
		   ? arm_stub_long_branch_any_thumb_pic
		   /* V4T stub.  */
		   : arm_stub_long_branch_v4t_arm_thumb_pic)

		/* non-PIC stubs.  */
		: ((globals->use_blx)
		   /* V5T and above.  */
		   ? arm_stub_long_branch_any_any
		   /* V4T.  */
		   : arm_stub_long_branch_v4t_arm_thumb);
	    }
	}
      else
	{
	  /* Arm to arm.  */
	  if (branch_offset > ARM_MAX_FWD_BRANCH_OFFSET
	      || (branch_offset < ARM_MAX_BWD_BRANCH_OFFSET))
	    {
	      stub_type =
		(bfd_link_pic (info) | globals->pic_veneer)
		/* PIC stubs.  */
		? (r_type == R_ARM_TLS_CALL
		   /* TLS PIC Stub.  */
		   ? arm_stub_long_branch_any_tls_pic
		   : (globals->root.target_os == is_nacl
		      ? arm_stub_long_branch_arm_nacl_pic
		      : arm_stub_long_branch_any_arm_pic))
		/* non-PIC stubs.  */
		: (globals->root.target_os == is_nacl
		   ? arm_stub_long_branch_arm_nacl
		   : arm_stub_long_branch_any_any);
	    }
	}
    }

  /* If a stub is needed, record the actual destination type.  */
  if (stub_type != arm_stub_none)
    *actual_branch_type = branch_type;

  return stub_type;
}

/* Build a name for an entry in the stub hash table.  */

static char *
elf32_arm_stub_name (const asection *input_section,
		     const asection *sym_sec,
		     const struct elf32_arm_link_hash_entry *hash,
		     const Elf_Internal_Rela *rel,
		     enum elf32_arm_stub_type stub_type)
{
  char *stub_name;
  bfd_size_type len;

  if (hash)
    {
      len = 8 + 1 + strlen (hash->root.root.root.string) + 1 + 8 + 1 + 2 + 1;
      stub_name = (char *) bfd_malloc (len);
      if (stub_name != NULL)
	sprintf (stub_name, "%08x_%s+%x_%d",
		 input_section->id & 0xffffffff,
		 hash->root.root.root.string,
		 (int) rel->r_addend & 0xffffffff,
		 (int) stub_type);
    }
  else
    {
      len = 8 + 1 + 8 + 1 + 8 + 1 + 8 + 1 + 2 + 1;
      stub_name = (char *) bfd_malloc (len);
      if (stub_name != NULL)
	sprintf (stub_name, "%08x_%x:%x+%x_%d",
		 input_section->id & 0xffffffff,
		 sym_sec->id & 0xffffffff,
		 ELF32_R_TYPE (rel->r_info) == R_ARM_TLS_CALL
		 || ELF32_R_TYPE (rel->r_info) == R_ARM_THM_TLS_CALL
		 ? 0 : (int) ELF32_R_SYM (rel->r_info) & 0xffffffff,
		 (int) rel->r_addend & 0xffffffff,
		 (int) stub_type);
    }

  return stub_name;
}

/* Look up an entry in the stub hash.  Stub entries are cached because
   creating the stub name takes a bit of time.  */

static struct elf32_arm_stub_hash_entry *
elf32_arm_get_stub_entry (const asection *input_section,
			  const asection *sym_sec,
			  struct elf_link_hash_entry *hash,
			  const Elf_Internal_Rela *rel,
			  struct elf32_arm_link_hash_table *htab,
			  enum elf32_arm_stub_type stub_type)
{
  struct elf32_arm_stub_hash_entry *stub_entry;
  struct elf32_arm_link_hash_entry *h = (struct elf32_arm_link_hash_entry *) hash;
  const asection *id_sec;

  if ((input_section->flags & SEC_CODE) == 0)
    return NULL;

  /* If the input section is the CMSE stubs one and it needs a long
     branch stub to reach it's final destination, give up with an
     error message: this is not supported.  See PR ld/24709.  */
  if (!strncmp (input_section->name, CMSE_STUB_NAME, strlen (CMSE_STUB_NAME)))
    {
      bfd *output_bfd = htab->obfd;
      asection *out_sec = bfd_get_section_by_name (output_bfd, CMSE_STUB_NAME);

      _bfd_error_handler (_("ERROR: CMSE stub (%s section) too far "
			    "(%#" PRIx64 ") from destination (%#" PRIx64 ")"),
			  CMSE_STUB_NAME,
			  (uint64_t)out_sec->output_section->vma
			    + out_sec->output_offset,
			  (uint64_t)sym_sec->output_section->vma
			    + sym_sec->output_offset
			    + h->root.root.u.def.value);
      /* Exit, rather than leave incompletely processed
	 relocations.  */
      xexit (1);
    }

  /* If this input section is part of a group of sections sharing one
     stub section, then use the id of the first section in the group.
     Stub names need to include a section id, as there may well be
     more than one stub used to reach say, printf, and we need to
     distinguish between them.  */
  BFD_ASSERT (input_section->id <= htab->top_id);
  id_sec = htab->stub_group[input_section->id].link_sec;

  if (h != NULL && h->stub_cache != NULL
      && h->stub_cache->h == h
      && h->stub_cache->id_sec == id_sec
      && h->stub_cache->stub_type == stub_type)
    {
      stub_entry = h->stub_cache;
    }
  else
    {
      char *stub_name;

      stub_name = elf32_arm_stub_name (id_sec, sym_sec, h, rel, stub_type);
      if (stub_name == NULL)
	return NULL;

      stub_entry = arm_stub_hash_lookup (&htab->stub_hash_table,
					stub_name, false, false);
      if (h != NULL)
	h->stub_cache = stub_entry;

      free (stub_name);
    }

  return stub_entry;
}

/* Whether veneers of type STUB_TYPE require to be in a dedicated output
   section.  */

static bool
arm_dedicated_stub_output_section_required (enum elf32_arm_stub_type stub_type)
{
  if (stub_type >= max_stub_type)
    abort ();  /* Should be unreachable.  */

  switch (stub_type)
    {
    case arm_stub_cmse_branch_thumb_only:
      return true;

    default:
      return false;
    }

  abort ();  /* Should be unreachable.  */
}

/* Required alignment (as a power of 2) for the dedicated section holding
   veneers of type STUB_TYPE, or 0 if veneers of this type are interspersed
   with input sections.  */

static int
arm_dedicated_stub_output_section_required_alignment
  (enum elf32_arm_stub_type stub_type)
{
  if (stub_type >= max_stub_type)
    abort ();  /* Should be unreachable.  */

  switch (stub_type)
    {
    /* Vectors of Secure Gateway veneers must be aligned on 32byte
       boundary.  */
    case arm_stub_cmse_branch_thumb_only:
      return 5;

    default:
      BFD_ASSERT (!arm_dedicated_stub_output_section_required (stub_type));
      return 0;
    }

  abort ();  /* Should be unreachable.  */
}

/* Name of the dedicated output section to put veneers of type STUB_TYPE, or
   NULL if veneers of this type are interspersed with input sections.  */

static const char *
arm_dedicated_stub_output_section_name (enum elf32_arm_stub_type stub_type)
{
  if (stub_type >= max_stub_type)
    abort ();  /* Should be unreachable.  */

  switch (stub_type)
    {
    case arm_stub_cmse_branch_thumb_only:
      return CMSE_STUB_NAME;

    default:
      BFD_ASSERT (!arm_dedicated_stub_output_section_required (stub_type));
      return NULL;
    }

  abort ();  /* Should be unreachable.  */
}

/* If veneers of type STUB_TYPE should go in a dedicated output section,
   returns the address of the hash table field in HTAB holding a pointer to the
   corresponding input section.  Otherwise, returns NULL.  */

static asection **
arm_dedicated_stub_input_section_ptr (struct elf32_arm_link_hash_table *htab,
				      enum elf32_arm_stub_type stub_type)
{
  if (stub_type >= max_stub_type)
    abort ();  /* Should be unreachable.  */

  switch (stub_type)
    {
    case arm_stub_cmse_branch_thumb_only:
      return &htab->cmse_stub_sec;

    default:
      BFD_ASSERT (!arm_dedicated_stub_output_section_required (stub_type));
      return NULL;
    }

  abort ();  /* Should be unreachable.  */
}

/* Find or create a stub section to contain a stub of type STUB_TYPE.  SECTION
   is the section that branch into veneer and can be NULL if stub should go in
   a dedicated output section.  Returns a pointer to the stub section, and the
   section to which the stub section will be attached (in *LINK_SEC_P).
   LINK_SEC_P may be NULL.  */

static asection *
elf32_arm_create_or_find_stub_sec (asection **link_sec_p, asection *section,
				   struct elf32_arm_link_hash_table *htab,
				   enum elf32_arm_stub_type stub_type)
{
  asection *link_sec, *out_sec, **stub_sec_p;
  const char *stub_sec_prefix;
  bool dedicated_output_section =
    arm_dedicated_stub_output_section_required (stub_type);
  int align;

  if (dedicated_output_section)
    {
      bfd *output_bfd = htab->obfd;
      const char *out_sec_name =
	arm_dedicated_stub_output_section_name (stub_type);
      link_sec = NULL;
      stub_sec_p = arm_dedicated_stub_input_section_ptr (htab, stub_type);
      stub_sec_prefix = out_sec_name;
      align = arm_dedicated_stub_output_section_required_alignment (stub_type);
      out_sec = bfd_get_section_by_name (output_bfd, out_sec_name);
      if (out_sec == NULL)
	{
	  _bfd_error_handler (_("no address assigned to the veneers output "
				"section %s"), out_sec_name);
	  return NULL;
	}
    }
  else
    {
      BFD_ASSERT (section->id <= htab->top_id);
      link_sec = htab->stub_group[section->id].link_sec;
      BFD_ASSERT (link_sec != NULL);
      stub_sec_p = &htab->stub_group[section->id].stub_sec;
      if (*stub_sec_p == NULL)
	stub_sec_p = &htab->stub_group[link_sec->id].stub_sec;
      stub_sec_prefix = link_sec->name;
      out_sec = link_sec->output_section;
      align = htab->root.target_os == is_nacl ? 4 : 3;
    }

  if (*stub_sec_p == NULL)
    {
      size_t namelen;
      bfd_size_type len;
      char *s_name;

      namelen = strlen (stub_sec_prefix);
      len = namelen + sizeof (STUB_SUFFIX);
      s_name = (char *) bfd_alloc (htab->stub_bfd, len);
      if (s_name == NULL)
	return NULL;

      memcpy (s_name, stub_sec_prefix, namelen);
      memcpy (s_name + namelen, STUB_SUFFIX, sizeof (STUB_SUFFIX));
      *stub_sec_p = (*htab->add_stub_section) (s_name, out_sec, link_sec,
					       align);
      if (*stub_sec_p == NULL)
	return NULL;

      out_sec->flags |= SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE
			| SEC_HAS_CONTENTS | SEC_RELOC | SEC_IN_MEMORY
			| SEC_KEEP;
    }

  if (!dedicated_output_section)
    htab->stub_group[section->id].stub_sec = *stub_sec_p;

  if (link_sec_p)
    *link_sec_p = link_sec;

  return *stub_sec_p;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct elf32_arm_stub_hash_entry *
elf32_arm_add_stub (const char *stub_name, asection *section,
		    struct elf32_arm_link_hash_table *htab,
		    enum elf32_arm_stub_type stub_type)
{
  asection *link_sec;
  asection *stub_sec;
  struct elf32_arm_stub_hash_entry *stub_entry;

  stub_sec = elf32_arm_create_or_find_stub_sec (&link_sec, section, htab,
						stub_type);
  if (stub_sec == NULL)
    return NULL;

  /* Enter this entry into the linker stub hash table.  */
  stub_entry = arm_stub_hash_lookup (&htab->stub_hash_table, stub_name,
				     true, false);
  if (stub_entry == NULL)
    {
      if (section == NULL)
	section = stub_sec;
      _bfd_error_handler (_("%pB: cannot create stub entry %s"),
			  section->owner, stub_name);
      return NULL;
    }

  stub_entry->stub_sec = stub_sec;
  stub_entry->stub_offset = (bfd_vma) -1;
  stub_entry->id_sec = link_sec;

  return stub_entry;
}

/* Store an Arm insn into an output section not processed by
   elf32_arm_write_section.  */

static void
put_arm_insn (struct elf32_arm_link_hash_table * htab,
	      bfd * output_bfd, bfd_vma val, void * ptr)
{
  if (htab->byteswap_code != bfd_little_endian (output_bfd))
    bfd_putl32 (val, ptr);
  else
    bfd_putb32 (val, ptr);
}

/* Store a 16-bit Thumb insn into an output section not processed by
   elf32_arm_write_section.  */

static void
put_thumb_insn (struct elf32_arm_link_hash_table * htab,
		bfd * output_bfd, bfd_vma val, void * ptr)
{
  if (htab->byteswap_code != bfd_little_endian (output_bfd))
    bfd_putl16 (val, ptr);
  else
    bfd_putb16 (val, ptr);
}

/* Store a Thumb2 insn into an output section not processed by
   elf32_arm_write_section.  */

static void
put_thumb2_insn (struct elf32_arm_link_hash_table * htab,
		 bfd * output_bfd, bfd_vma val, bfd_byte * ptr)
{
  /* T2 instructions are 16-bit streamed.  */
  if (htab->byteswap_code != bfd_little_endian (output_bfd))
    {
      bfd_putl16 ((val >> 16) & 0xffff, ptr);
      bfd_putl16 ((val & 0xffff), ptr + 2);
    }
  else
    {
      bfd_putb16 ((val >> 16) & 0xffff, ptr);
      bfd_putb16 ((val & 0xffff), ptr + 2);
    }
}

/* If it's possible to change R_TYPE to a more efficient access
   model, return the new reloc type.  */

static unsigned
elf32_arm_tls_transition (struct bfd_link_info *info, int r_type,
			  struct elf_link_hash_entry *h)
{
  int is_local = (h == NULL);

  if (bfd_link_dll (info)
      || (h && h->root.type == bfd_link_hash_undefweak))
    return r_type;

  /* We do not support relaxations for Old TLS models.  */
  switch (r_type)
    {
    case R_ARM_TLS_GOTDESC:
    case R_ARM_TLS_CALL:
    case R_ARM_THM_TLS_CALL:
    case R_ARM_TLS_DESCSEQ:
    case R_ARM_THM_TLS_DESCSEQ:
      return is_local ? R_ARM_TLS_LE32 : R_ARM_TLS_IE32;
    }

  return r_type;
}

static bfd_reloc_status_type elf32_arm_final_link_relocate
  (reloc_howto_type *, bfd *, bfd *, asection *, bfd_byte *,
   Elf_Internal_Rela *, bfd_vma, struct bfd_link_info *, asection *,
   const char *, unsigned char, enum arm_st_branch_type,
   struct elf_link_hash_entry *, bool *, char **);

static unsigned int
arm_stub_required_alignment (enum elf32_arm_stub_type stub_type)
{
  switch (stub_type)
    {
    case arm_stub_a8_veneer_b_cond:
    case arm_stub_a8_veneer_b:
    case arm_stub_a8_veneer_bl:
      return 2;

    case arm_stub_long_branch_any_any:
    case arm_stub_long_branch_v4t_arm_thumb:
    case arm_stub_long_branch_thumb_only:
    case arm_stub_long_branch_thumb2_only:
    case arm_stub_long_branch_thumb2_only_pure:
    case arm_stub_long_branch_v4t_thumb_thumb:
    case arm_stub_long_branch_v4t_thumb_arm:
    case arm_stub_short_branch_v4t_thumb_arm:
    case arm_stub_long_branch_any_arm_pic:
    case arm_stub_long_branch_any_thumb_pic:
    case arm_stub_long_branch_v4t_thumb_thumb_pic:
    case arm_stub_long_branch_v4t_arm_thumb_pic:
    case arm_stub_long_branch_v4t_thumb_arm_pic:
    case arm_stub_long_branch_thumb_only_pic:
    case arm_stub_long_branch_any_tls_pic:
    case arm_stub_long_branch_v4t_thumb_tls_pic:
    case arm_stub_cmse_branch_thumb_only:
    case arm_stub_a8_veneer_blx:
      return 4;

    case arm_stub_long_branch_arm_nacl:
    case arm_stub_long_branch_arm_nacl_pic:
      return 16;

    default:
      abort ();  /* Should be unreachable.  */
    }
}

/* Returns whether stubs of type STUB_TYPE take over the symbol they are
   veneering (TRUE) or have their own symbol (FALSE).  */

static bool
arm_stub_sym_claimed (enum elf32_arm_stub_type stub_type)
{
  if (stub_type >= max_stub_type)
    abort ();  /* Should be unreachable.  */

  switch (stub_type)
    {
    case arm_stub_cmse_branch_thumb_only:
      return true;

    default:
      return false;
    }

  abort ();  /* Should be unreachable.  */
}

/* Returns the padding needed for the dedicated section used stubs of type
   STUB_TYPE.  */

static int
arm_dedicated_stub_section_padding (enum elf32_arm_stub_type stub_type)
{
  if (stub_type >= max_stub_type)
    abort ();  /* Should be unreachable.  */

  switch (stub_type)
    {
    case arm_stub_cmse_branch_thumb_only:
      return 32;

    default:
      return 0;
    }

  abort ();  /* Should be unreachable.  */
}

/* If veneers of type STUB_TYPE should go in a dedicated output section,
   returns the address of the hash table field in HTAB holding the offset at
   which new veneers should be layed out in the stub section.  */

static bfd_vma*
arm_new_stubs_start_offset_ptr (struct elf32_arm_link_hash_table *htab,
				enum elf32_arm_stub_type stub_type)
{
  switch (stub_type)
    {
    case arm_stub_cmse_branch_thumb_only:
      return &htab->new_cmse_stub_offset;

    default:
      BFD_ASSERT (!arm_dedicated_stub_output_section_required (stub_type));
      return NULL;
    }
}

static bool
arm_build_one_stub (struct bfd_hash_entry *gen_entry,
		    void * in_arg)
{
#define MAXRELOCS 3
  bool removed_sg_veneer;
  struct elf32_arm_stub_hash_entry *stub_entry;
  struct elf32_arm_link_hash_table *globals;
  struct bfd_link_info *info;
  asection *stub_sec;
  bfd *stub_bfd;
  bfd_byte *loc;
  bfd_vma sym_value;
  int template_size;
  int size;
  const insn_sequence *template_sequence;
  int i;
  int stub_reloc_idx[MAXRELOCS] = {-1, -1};
  int stub_reloc_offset[MAXRELOCS] = {0, 0};
  int nrelocs = 0;
  int just_allocated = 0;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_arm_stub_hash_entry *) gen_entry;
  info = (struct bfd_link_info *) in_arg;

  /* Fail if the target section could not be assigned to an output
     section.  The user should fix his linker script.  */
  if (stub_entry->target_section->output_section == NULL
      && info->non_contiguous_regions)
    info->callbacks->einfo (_("%F%P: Could not assign `%pA' to an output section. "
			      "Retry without --enable-non-contiguous-regions.\n"),
			    stub_entry->target_section);

  globals = elf32_arm_hash_table (info);
  if (globals == NULL)
    return false;

  stub_sec = stub_entry->stub_sec;

  if ((globals->fix_cortex_a8 < 0)
      != (arm_stub_required_alignment (stub_entry->stub_type) == 2))
    /* We have to do less-strictly-aligned fixes last.  */
    return true;

  /* Assign a slot at the end of section if none assigned yet.  */
  if (stub_entry->stub_offset == (bfd_vma) -1)
    {
      stub_entry->stub_offset = stub_sec->size;
      just_allocated = 1;
    }
  loc = stub_sec->contents + stub_entry->stub_offset;

  stub_bfd = stub_sec->owner;

  /* This is the address of the stub destination.  */
  sym_value = (stub_entry->target_value
	       + stub_entry->target_section->output_offset
	       + stub_entry->target_section->output_section->vma);

  template_sequence = stub_entry->stub_template;
  template_size = stub_entry->stub_template_size;

  size = 0;
  for (i = 0; i < template_size; i++)
    {
      switch (template_sequence[i].type)
	{
	case THUMB16_TYPE:
	  {
	    bfd_vma data = (bfd_vma) template_sequence[i].data;
	    if (template_sequence[i].reloc_addend != 0)
	      {
		/* We've borrowed the reloc_addend field to mean we should
		   insert a condition code into this (Thumb-1 branch)
		   instruction.  See THUMB16_BCOND_INSN.  */
		BFD_ASSERT ((data & 0xff00) == 0xd000);
		data |= ((stub_entry->orig_insn >> 22) & 0xf) << 8;
	      }
	    bfd_put_16 (stub_bfd, data, loc + size);
	    size += 2;
	  }
	  break;

	case THUMB32_TYPE:
	  bfd_put_16 (stub_bfd,
		      (template_sequence[i].data >> 16) & 0xffff,
		      loc + size);
	  bfd_put_16 (stub_bfd, template_sequence[i].data & 0xffff,
		      loc + size + 2);
	  if (template_sequence[i].r_type != R_ARM_NONE)
	    {
	      stub_reloc_idx[nrelocs] = i;
	      stub_reloc_offset[nrelocs++] = size;
	    }
	  size += 4;
	  break;

	case ARM_TYPE:
	  bfd_put_32 (stub_bfd, template_sequence[i].data,
		      loc + size);
	  /* Handle cases where the target is encoded within the
	     instruction.  */
	  if (template_sequence[i].r_type == R_ARM_JUMP24)
	    {
	      stub_reloc_idx[nrelocs] = i;
	      stub_reloc_offset[nrelocs++] = size;
	    }
	  size += 4;
	  break;

	case DATA_TYPE:
	  bfd_put_32 (stub_bfd, template_sequence[i].data, loc + size);
	  stub_reloc_idx[nrelocs] = i;
	  stub_reloc_offset[nrelocs++] = size;
	  size += 4;
	  break;

	default:
	  BFD_FAIL ();
	  return false;
	}
    }

  if (just_allocated)
    stub_sec->size += size;

  /* Stub size has already been computed in arm_size_one_stub. Check
     consistency.  */
  BFD_ASSERT (size == stub_entry->stub_size);

  /* Destination is Thumb. Force bit 0 to 1 to reflect this.  */
  if (stub_entry->branch_type == ST_BRANCH_TO_THUMB)
    sym_value |= 1;

  /* Assume non empty slots have at least one and at most MAXRELOCS entries
     to relocate in each stub.  */
  removed_sg_veneer =
    (size == 0 && stub_entry->stub_type == arm_stub_cmse_branch_thumb_only);
  BFD_ASSERT (removed_sg_veneer || (nrelocs != 0 && nrelocs <= MAXRELOCS));

  for (i = 0; i < nrelocs; i++)
    {
      Elf_Internal_Rela rel;
      bool unresolved_reloc;
      char *error_message;
      bfd_vma points_to =
	sym_value + template_sequence[stub_reloc_idx[i]].reloc_addend;

      rel.r_offset = stub_entry->stub_offset + stub_reloc_offset[i];
      rel.r_info = ELF32_R_INFO (0,
				 template_sequence[stub_reloc_idx[i]].r_type);
      rel.r_addend = 0;

      if (stub_entry->stub_type == arm_stub_a8_veneer_b_cond && i == 0)
	/* The first relocation in the elf32_arm_stub_a8_veneer_b_cond[]
	   template should refer back to the instruction after the original
	   branch.  We use target_section as Cortex-A8 erratum workaround stubs
	   are only generated when both source and target are in the same
	   section.  */
	points_to = stub_entry->target_section->output_section->vma
		    + stub_entry->target_section->output_offset
		    + stub_entry->source_value;

      elf32_arm_final_link_relocate (elf32_arm_howto_from_type
	  (template_sequence[stub_reloc_idx[i]].r_type),
	   stub_bfd, info->output_bfd, stub_sec, stub_sec->contents, &rel,
	   points_to, info, stub_entry->target_section, "", STT_FUNC,
	   stub_entry->branch_type,
	   (struct elf_link_hash_entry *) stub_entry->h, &unresolved_reloc,
	   &error_message);
    }

  return true;
#undef MAXRELOCS
}

/* Calculate the template, template size and instruction size for a stub.
   Return value is the instruction size.  */

static unsigned int
find_stub_size_and_template (enum elf32_arm_stub_type stub_type,
			     const insn_sequence **stub_template,
			     int *stub_template_size)
{
  const insn_sequence *template_sequence = NULL;
  int template_size = 0, i;
  unsigned int size;

  template_sequence = stub_definitions[stub_type].template_sequence;
  if (stub_template)
    *stub_template = template_sequence;

  template_size = stub_definitions[stub_type].template_size;
  if (stub_template_size)
    *stub_template_size = template_size;

  size = 0;
  for (i = 0; i < template_size; i++)
    {
      switch (template_sequence[i].type)
	{
	case THUMB16_TYPE:
	  size += 2;
	  break;

	case ARM_TYPE:
	case THUMB32_TYPE:
	case DATA_TYPE:
	  size += 4;
	  break;

	default:
	  BFD_FAIL ();
	  return 0;
	}
    }

  return size;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */

static bool
arm_size_one_stub (struct bfd_hash_entry *gen_entry,
		   void *in_arg ATTRIBUTE_UNUSED)
{
  struct elf32_arm_stub_hash_entry *stub_entry;
  const insn_sequence *template_sequence;
  int template_size, size;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_arm_stub_hash_entry *) gen_entry;

  BFD_ASSERT ((stub_entry->stub_type > arm_stub_none)
	      && stub_entry->stub_type < ARRAY_SIZE (stub_definitions));

  size = find_stub_size_and_template (stub_entry->stub_type, &template_sequence,
				      &template_size);

  /* Initialized to -1.  Null size indicates an empty slot full of zeros.  */
  if (stub_entry->stub_template_size)
    {
      stub_entry->stub_size = size;
      stub_entry->stub_template = template_sequence;
      stub_entry->stub_template_size = template_size;
    }

  /* Already accounted for.  */
  if (stub_entry->stub_offset != (bfd_vma) -1)
    return true;

  size = (size + 7) & ~7;
  stub_entry->stub_sec->size += size;

  return true;
}

/* External entry points for sizing and building linker stubs.  */

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elf32_arm_setup_section_lists (bfd *output_bfd,
			       struct bfd_link_info *info)
{
  bfd *input_bfd;
  unsigned int bfd_count;
  unsigned int top_id, top_index;
  asection *section;
  asection **input_list, **list;
  size_t amt;
  struct elf32_arm_link_hash_table *htab = elf32_arm_hash_table (info);

  if (htab == NULL)
    return 0;

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
  htab->stub_group = (struct map_stub *) bfd_zmalloc (amt);
  if (htab->stub_group == NULL)
    return -1;
  htab->top_id = top_id;

  /* We can't use output_bfd->section_count here to find the top output
     section index as some sections may have been removed, and
     _bfd_strip_section_from_output doesn't renumber the indices.  */
  for (section = output_bfd->sections, top_index = 0;
       section != NULL;
       section = section->next)
    {
      if (top_index < section->index)
	top_index = section->index;
    }

  htab->top_index = top_index;
  amt = sizeof (asection *) * (top_index + 1);
  input_list = (asection **) bfd_malloc (amt);
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
      if ((section->flags & SEC_CODE) != 0)
	input_list[section->index] = NULL;
    }

  return 1;
}

/* The linker repeatedly calls this function for each input section,
   in the order that input sections are linked into output sections.
   Build lists of input sections to determine groupings between which
   we may insert linker stubs.  */

void
elf32_arm_next_input_section (struct bfd_link_info *info,
			      asection *isec)
{
  struct elf32_arm_link_hash_table *htab = elf32_arm_hash_table (info);

  if (htab == NULL)
    return;

  if (isec->output_section->index <= htab->top_index)
    {
      asection **list = htab->input_list + isec->output_section->index;

      if (*list != bfd_abs_section_ptr && (isec->flags & SEC_CODE) != 0)
	{
	  /* Steal the link_sec pointer for our list.  */
#define PREV_SEC(sec) (htab->stub_group[(sec)->id].link_sec)
	  /* This happens to make the list in reverse order,
	     which we reverse later.  */
	  PREV_SEC (isec) = *list;
	  *list = isec;
	}
    }
}

/* See whether we can group stub sections together.  Grouping stub
   sections may result in fewer stubs.  More importantly, we need to
   put all .init* and .fini* stubs at the end of the .init or
   .fini output sections respectively, because glibc splits the
   _init and _fini functions into multiple parts.  Putting a stub in
   the middle of a function is not a good idea.  */

static void
group_sections (struct elf32_arm_link_hash_table *htab,
		bfd_size_type stub_group_size,
		bool stubs_always_after_branch)
{
  asection **list = htab->input_list;

  do
    {
      asection *tail = *list;
      asection *head;

      if (tail == bfd_abs_section_ptr)
	continue;

      /* Reverse the list: we must avoid placing stubs at the
	 beginning of the section because the beginning of the text
	 section may be required for an interrupt vector in bare metal
	 code.  */
#define NEXT_SEC PREV_SEC
      head = NULL;
      while (tail != NULL)
	{
	  /* Pop from tail.  */
	  asection *item = tail;
	  tail = PREV_SEC (item);

	  /* Push on head.  */
	  NEXT_SEC (item) = head;
	  head = item;
	}

      while (head != NULL)
	{
	  asection *curr;
	  asection *next;
	  bfd_vma stub_group_start = head->output_offset;
	  bfd_vma end_of_next;

	  curr = head;
	  while (NEXT_SEC (curr) != NULL)
	    {
	      next = NEXT_SEC (curr);
	      end_of_next = next->output_offset + next->size;
	      if (end_of_next - stub_group_start >= stub_group_size)
		/* End of NEXT is too far from start, so stop.  */
		break;
	      /* Add NEXT to the group.  */
	      curr = next;
	    }

	  /* OK, the size from the start to the start of CURR is less
	     than stub_group_size and thus can be handled by one stub
	     section.  (Or the head section is itself larger than
	     stub_group_size, in which case we may be toast.)
	     We should really be keeping track of the total size of
	     stubs added here, as stubs contribute to the final output
	     section size.  */
	  do
	    {
	      next = NEXT_SEC (head);
	      /* Set up this stub group.  */
	      htab->stub_group[head->id].link_sec = curr;
	    }
	  while (head != curr && (head = next) != NULL);

	  /* But wait, there's more!  Input sections up to stub_group_size
	     bytes after the stub section can be handled by it too.  */
	  if (!stubs_always_after_branch)
	    {
	      stub_group_start = curr->output_offset + curr->size;

	      while (next != NULL)
		{
		  end_of_next = next->output_offset + next->size;
		  if (end_of_next - stub_group_start >= stub_group_size)
		    /* End of NEXT is too far from stubs, so stop.  */
		    break;
		  /* Add NEXT to the stub group.  */
		  head = next;
		  next = NEXT_SEC (head);
		  htab->stub_group[head->id].link_sec = curr;
		}
	    }
	  head = next;
	}
    }
  while (list++ != htab->input_list + htab->top_index);

  free (htab->input_list);
#undef PREV_SEC
#undef NEXT_SEC
}

/* Comparison function for sorting/searching relocations relating to Cortex-A8
   erratum fix.  */

static int
a8_reloc_compare (const void *a, const void *b)
{
  const struct a8_erratum_reloc *ra = (const struct a8_erratum_reloc *) a;
  const struct a8_erratum_reloc *rb = (const struct a8_erratum_reloc *) b;

  if (ra->from < rb->from)
    return -1;
  else if (ra->from > rb->from)
    return 1;
  else
    return 0;
}

static struct elf_link_hash_entry *find_thumb_glue (struct bfd_link_info *,
						    const char *, char **);

/* Helper function to scan code for sequences which might trigger the Cortex-A8
   branch/TLB erratum.  Fill in the table described by A8_FIXES_P,
   NUM_A8_FIXES_P, A8_FIX_TABLE_SIZE_P.  Returns true if an error occurs, false
   otherwise.  */

static bool
cortex_a8_erratum_scan (bfd *input_bfd,
			struct bfd_link_info *info,
			struct a8_erratum_fix **a8_fixes_p,
			unsigned int *num_a8_fixes_p,
			unsigned int *a8_fix_table_size_p,
			struct a8_erratum_reloc *a8_relocs,
			unsigned int num_a8_relocs,
			unsigned prev_num_a8_fixes,
			bool *stub_changed_p)
{
  asection *section;
  struct elf32_arm_link_hash_table *htab = elf32_arm_hash_table (info);
  struct a8_erratum_fix *a8_fixes = *a8_fixes_p;
  unsigned int num_a8_fixes = *num_a8_fixes_p;
  unsigned int a8_fix_table_size = *a8_fix_table_size_p;

  if (htab == NULL)
    return false;

  for (section = input_bfd->sections;
       section != NULL;
       section = section->next)
    {
      bfd_byte *contents = NULL;
      struct _arm_elf_section_data *sec_data;
      unsigned int span;
      bfd_vma base_vma;

      if (elf_section_type (section) != SHT_PROGBITS
	  || (elf_section_flags (section) & SHF_EXECINSTR) == 0
	  || (section->flags & SEC_EXCLUDE) != 0
	  || (section->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	  || (section->output_section == bfd_abs_section_ptr))
	continue;

      base_vma = section->output_section->vma + section->output_offset;

      if (elf_section_data (section)->this_hdr.contents != NULL)
	contents = elf_section_data (section)->this_hdr.contents;
      else if (! bfd_malloc_and_get_section (input_bfd, section, &contents))
	return true;

      sec_data = elf32_arm_section_data (section);

      for (span = 0; span < sec_data->mapcount; span++)
	{
	  unsigned int span_start = sec_data->map[span].vma;
	  unsigned int span_end = (span == sec_data->mapcount - 1)
	    ? section->size : sec_data->map[span + 1].vma;
	  unsigned int i;
	  char span_type = sec_data->map[span].type;
	  bool last_was_32bit = false, last_was_branch = false;

	  if (span_type != 't')
	    continue;

	  /* Span is entirely within a single 4KB region: skip scanning.  */
	  if (((base_vma + span_start) & ~0xfff)
	      == ((base_vma + span_end) & ~0xfff))
	    continue;

	  /* Scan for 32-bit Thumb-2 branches which span two 4K regions, where:

	       * The opcode is BLX.W, BL.W, B.W, Bcc.W
	       * The branch target is in the same 4KB region as the
		 first half of the branch.
	       * The instruction before the branch is a 32-bit
		 length non-branch instruction.  */
	  for (i = span_start; i < span_end;)
	    {
	      unsigned int insn = bfd_getl16 (&contents[i]);
	      bool insn_32bit = false, is_blx = false, is_b = false;
	      bool is_bl = false, is_bcc = false, is_32bit_branch;

	      if ((insn & 0xe000) == 0xe000 && (insn & 0x1800) != 0x0000)
		insn_32bit = true;

	      if (insn_32bit)
		{
		  /* Load the rest of the insn (in manual-friendly order).  */
		  insn = (insn << 16) | bfd_getl16 (&contents[i + 2]);

		  /* Encoding T4: B<c>.W.  */
		  is_b = (insn & 0xf800d000) == 0xf0009000;
		  /* Encoding T1: BL<c>.W.  */
		  is_bl = (insn & 0xf800d000) == 0xf000d000;
		  /* Encoding T2: BLX<c>.W.  */
		  is_blx = (insn & 0xf800d000) == 0xf000c000;
		  /* Encoding T3: B<c>.W (not permitted in IT block).  */
		  is_bcc = (insn & 0xf800d000) == 0xf0008000
			   && (insn & 0x07f00000) != 0x03800000;
		}

	      is_32bit_branch = is_b || is_bl || is_blx || is_bcc;

	      if (((base_vma + i) & 0xfff) == 0xffe
		  && insn_32bit
		  && is_32bit_branch
		  && last_was_32bit
		  && ! last_was_branch)
		{
		  bfd_signed_vma offset = 0;
		  bool force_target_arm = false;
		  bool force_target_thumb = false;
		  bfd_vma target;
		  enum elf32_arm_stub_type stub_type = arm_stub_none;
		  struct a8_erratum_reloc key, *found;
		  bool use_plt = false;

		  key.from = base_vma + i;
		  found = (struct a8_erratum_reloc *)
		      bsearch (&key, a8_relocs, num_a8_relocs,
			       sizeof (struct a8_erratum_reloc),
			       &a8_reloc_compare);

		  if (found)
		    {
		      char *error_message = NULL;
		      struct elf_link_hash_entry *entry;

		      /* We don't care about the error returned from this
			 function, only if there is glue or not.  */
		      entry = find_thumb_glue (info, found->sym_name,
					       &error_message);

		      if (entry)
			found->non_a8_stub = true;

		      /* Keep a simpler condition, for the sake of clarity.  */
		      if (htab->root.splt != NULL && found->hash != NULL
			  && found->hash->root.plt.offset != (bfd_vma) -1)
			use_plt = true;

		      if (found->r_type == R_ARM_THM_CALL)
			{
			  if (found->branch_type == ST_BRANCH_TO_ARM
			      || use_plt)
			    force_target_arm = true;
			  else
			    force_target_thumb = true;
			}
		    }

		  /* Check if we have an offending branch instruction.  */

		  if (found && found->non_a8_stub)
		    /* We've already made a stub for this instruction, e.g.
		       it's a long branch or a Thumb->ARM stub.  Assume that
		       stub will suffice to work around the A8 erratum (see
		       setting of always_after_branch above).  */
		    ;
		  else if (is_bcc)
		    {
		      offset = (insn & 0x7ff) << 1;
		      offset |= (insn & 0x3f0000) >> 4;
		      offset |= (insn & 0x2000) ? 0x40000 : 0;
		      offset |= (insn & 0x800) ? 0x80000 : 0;
		      offset |= (insn & 0x4000000) ? 0x100000 : 0;
		      if (offset & 0x100000)
			offset |= ~ ((bfd_signed_vma) 0xfffff);
		      stub_type = arm_stub_a8_veneer_b_cond;
		    }
		  else if (is_b || is_bl || is_blx)
		    {
		      int s = (insn & 0x4000000) != 0;
		      int j1 = (insn & 0x2000) != 0;
		      int j2 = (insn & 0x800) != 0;
		      int i1 = !(j1 ^ s);
		      int i2 = !(j2 ^ s);

		      offset = (insn & 0x7ff) << 1;
		      offset |= (insn & 0x3ff0000) >> 4;
		      offset |= i2 << 22;
		      offset |= i1 << 23;
		      offset |= s << 24;
		      if (offset & 0x1000000)
			offset |= ~ ((bfd_signed_vma) 0xffffff);

		      if (is_blx)
			offset &= ~ ((bfd_signed_vma) 3);

		      stub_type = is_blx ? arm_stub_a8_veneer_blx :
			is_bl ? arm_stub_a8_veneer_bl : arm_stub_a8_veneer_b;
		    }

		  if (stub_type != arm_stub_none)
		    {
		      bfd_vma pc_for_insn = base_vma + i + 4;

		      /* The original instruction is a BL, but the target is
			 an ARM instruction.  If we were not making a stub,
			 the BL would have been converted to a BLX.  Use the
			 BLX stub instead in that case.  */
		      if (htab->use_blx && force_target_arm
			  && stub_type == arm_stub_a8_veneer_bl)
			{
			  stub_type = arm_stub_a8_veneer_blx;
			  is_blx = true;
			  is_bl = false;
			}
		      /* Conversely, if the original instruction was
			 BLX but the target is Thumb mode, use the BL
			 stub.  */
		      else if (force_target_thumb
			       && stub_type == arm_stub_a8_veneer_blx)
			{
			  stub_type = arm_stub_a8_veneer_bl;
			  is_blx = false;
			  is_bl = true;
			}

		      if (is_blx)
			pc_for_insn &= ~ ((bfd_vma) 3);

		      /* If we found a relocation, use the proper destination,
			 not the offset in the (unrelocated) instruction.
			 Note this is always done if we switched the stub type
			 above.  */
		      if (found)
			offset =
			  (bfd_signed_vma) (found->destination - pc_for_insn);

		      /* If the stub will use a Thumb-mode branch to a
			 PLT target, redirect it to the preceding Thumb
			 entry point.  */
		      if (stub_type != arm_stub_a8_veneer_blx && use_plt)
			offset -= PLT_THUMB_STUB_SIZE;

		      target = pc_for_insn + offset;

		      /* The BLX stub is ARM-mode code.  Adjust the offset to
			 take the different PC value (+8 instead of +4) into
			 account.  */
		      if (stub_type == arm_stub_a8_veneer_blx)
			offset += 4;

		      if (((base_vma + i) & ~0xfff) == (target & ~0xfff))
			{
			  char *stub_name = NULL;

			  if (num_a8_fixes == a8_fix_table_size)
			    {
			      a8_fix_table_size *= 2;
			      a8_fixes = (struct a8_erratum_fix *)
				  bfd_realloc (a8_fixes,
					       sizeof (struct a8_erratum_fix)
					       * a8_fix_table_size);
			    }

			  if (num_a8_fixes < prev_num_a8_fixes)
			    {
			      /* If we're doing a subsequent scan,
				 check if we've found the same fix as
				 before, and try and reuse the stub
				 name.  */
			      stub_name = a8_fixes[num_a8_fixes].stub_name;
			      if ((a8_fixes[num_a8_fixes].section != section)
				  || (a8_fixes[num_a8_fixes].offset != i))
				{
				  free (stub_name);
				  stub_name = NULL;
				  *stub_changed_p = true;
				}
			    }

			  if (!stub_name)
			    {
			      stub_name = (char *) bfd_malloc (8 + 1 + 8 + 1);
			      if (stub_name != NULL)
				sprintf (stub_name, "%x:%x", section->id, i);
			    }

			  a8_fixes[num_a8_fixes].input_bfd = input_bfd;
			  a8_fixes[num_a8_fixes].section = section;
			  a8_fixes[num_a8_fixes].offset = i;
			  a8_fixes[num_a8_fixes].target_offset =
			    target - base_vma;
			  a8_fixes[num_a8_fixes].orig_insn = insn;
			  a8_fixes[num_a8_fixes].stub_name = stub_name;
			  a8_fixes[num_a8_fixes].stub_type = stub_type;
			  a8_fixes[num_a8_fixes].branch_type =
			    is_blx ? ST_BRANCH_TO_ARM : ST_BRANCH_TO_THUMB;

			  num_a8_fixes++;
			}
		    }
		}

	      i += insn_32bit ? 4 : 2;
	      last_was_32bit = insn_32bit;
	      last_was_branch = is_32bit_branch;
	    }
	}

      if (elf_section_data (section)->this_hdr.contents == NULL)
	free (contents);
    }

  *a8_fixes_p = a8_fixes;
  *num_a8_fixes_p = num_a8_fixes;
  *a8_fix_table_size_p = a8_fix_table_size;

  return false;
}

/* Create or update a stub entry depending on whether the stub can already be
   found in HTAB.  The stub is identified by:
   - its type STUB_TYPE
   - its source branch (note that several can share the same stub) whose
     section and relocation (if any) are given by SECTION and IRELA
     respectively
   - its target symbol whose input section, hash, name, value and branch type
     are given in SYM_SEC, HASH, SYM_NAME, SYM_VALUE and BRANCH_TYPE
     respectively

   If found, the value of the stub's target symbol is updated from SYM_VALUE
   and *NEW_STUB is set to FALSE.  Otherwise, *NEW_STUB is set to
   TRUE and the stub entry is initialized.

   Returns the stub that was created or updated, or NULL if an error
   occurred.  */

static struct elf32_arm_stub_hash_entry *
elf32_arm_create_stub (struct elf32_arm_link_hash_table *htab,
		       enum elf32_arm_stub_type stub_type, asection *section,
		       Elf_Internal_Rela *irela, asection *sym_sec,
		       struct elf32_arm_link_hash_entry *hash, char *sym_name,
		       bfd_vma sym_value, enum arm_st_branch_type branch_type,
		       bool *new_stub)
{
  const asection *id_sec;
  char *stub_name;
  struct elf32_arm_stub_hash_entry *stub_entry;
  unsigned int r_type;
  bool sym_claimed = arm_stub_sym_claimed (stub_type);

  BFD_ASSERT (stub_type != arm_stub_none);
  *new_stub = false;

  if (sym_claimed)
    stub_name = sym_name;
  else
    {
      BFD_ASSERT (irela);
      BFD_ASSERT (section);
      BFD_ASSERT (section->id <= htab->top_id);

      /* Support for grouping stub sections.  */
      id_sec = htab->stub_group[section->id].link_sec;

      /* Get the name of this stub.  */
      stub_name = elf32_arm_stub_name (id_sec, sym_sec, hash, irela,
				       stub_type);
      if (!stub_name)
	return NULL;
    }

  stub_entry = arm_stub_hash_lookup (&htab->stub_hash_table, stub_name, false,
				     false);
  /* The proper stub has already been created, just update its value.  */
  if (stub_entry != NULL)
    {
      if (!sym_claimed)
	free (stub_name);
      stub_entry->target_value = sym_value;
      return stub_entry;
    }

  stub_entry = elf32_arm_add_stub (stub_name, section, htab, stub_type);
  if (stub_entry == NULL)
    {
      if (!sym_claimed)
	free (stub_name);
      return NULL;
    }

  stub_entry->target_value = sym_value;
  stub_entry->target_section = sym_sec;
  stub_entry->stub_type = stub_type;
  stub_entry->h = hash;
  stub_entry->branch_type = branch_type;

  if (sym_claimed)
    stub_entry->output_name = sym_name;
  else
    {
      if (sym_name == NULL)
	sym_name = "unnamed";
      stub_entry->output_name = (char *)
	bfd_alloc (htab->stub_bfd, sizeof (THUMB2ARM_GLUE_ENTRY_NAME)
				   + strlen (sym_name));
      if (stub_entry->output_name == NULL)
	{
	  free (stub_name);
	  return NULL;
	}

      /* For historical reasons, use the existing names for ARM-to-Thumb and
	 Thumb-to-ARM stubs.  */
      r_type = ELF32_R_TYPE (irela->r_info);
      if ((r_type == (unsigned int) R_ARM_THM_CALL
	   || r_type == (unsigned int) R_ARM_THM_JUMP24
	   || r_type == (unsigned int) R_ARM_THM_JUMP19)
	  && branch_type == ST_BRANCH_TO_ARM)
	sprintf (stub_entry->output_name, THUMB2ARM_GLUE_ENTRY_NAME, sym_name);
      else if ((r_type == (unsigned int) R_ARM_CALL
		|| r_type == (unsigned int) R_ARM_JUMP24)
	       && branch_type == ST_BRANCH_TO_THUMB)
	sprintf (stub_entry->output_name, ARM2THUMB_GLUE_ENTRY_NAME, sym_name);
      else
	sprintf (stub_entry->output_name, STUB_ENTRY_NAME, sym_name);
    }

  *new_stub = true;
  return stub_entry;
}

/* Scan symbols in INPUT_BFD to identify secure entry functions needing a
   gateway veneer to transition from non secure to secure state and create them
   accordingly.

   "ARMv8-M Security Extensions: Requirements on Development Tools" document
   defines the conditions that govern Secure Gateway veneer creation for a
   given symbol <SYM> as follows:
   - it has function type
   - it has non local binding
   - a symbol named __acle_se_<SYM> (called special symbol) exists with the
     same type, binding and value as <SYM> (called normal symbol).
   An entry function can handle secure state transition itself in which case
   its special symbol would have a different value from the normal symbol.

   OUT_ATTR gives the output attributes, SYM_HASHES the symbol index to hash
   entry mapping while HTAB gives the name to hash entry mapping.
   *CMSE_STUB_CREATED is increased by the number of secure gateway veneer
   created.

   The return value gives whether a stub failed to be allocated.  */

static bool
cmse_scan (bfd *input_bfd, struct elf32_arm_link_hash_table *htab,
	   obj_attribute *out_attr, struct elf_link_hash_entry **sym_hashes,
	   int *cmse_stub_created)
{
  const struct elf_backend_data *bed;
  Elf_Internal_Shdr *symtab_hdr;
  unsigned i, j, sym_count, ext_start;
  Elf_Internal_Sym *cmse_sym, *local_syms;
  struct elf32_arm_link_hash_entry *hash, *cmse_hash = NULL;
  enum arm_st_branch_type branch_type;
  char *sym_name, *lsym_name;
  bfd_vma sym_value;
  asection *section;
  struct elf32_arm_stub_hash_entry *stub_entry;
  bool is_v8m, new_stub, cmse_invalid, ret = true;

  bed = get_elf_backend_data (input_bfd);
  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_count = symtab_hdr->sh_size / bed->s->sizeof_sym;
  ext_start = symtab_hdr->sh_info;
  is_v8m = (out_attr[Tag_CPU_arch].i >= TAG_CPU_ARCH_V8M_BASE
	    && out_attr[Tag_CPU_arch_profile].i == 'M');

  local_syms = (Elf_Internal_Sym *) symtab_hdr->contents;
  if (local_syms == NULL)
    local_syms = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
				       symtab_hdr->sh_info, 0, NULL, NULL,
				       NULL);
  if (symtab_hdr->sh_info && local_syms == NULL)
    return false;

  /* Scan symbols.  */
  for (i = 0; i < sym_count; i++)
    {
      cmse_invalid = false;

      if (i < ext_start)
	{
	  cmse_sym = &local_syms[i];
	  sym_name = bfd_elf_string_from_elf_section (input_bfd,
						      symtab_hdr->sh_link,
						      cmse_sym->st_name);
	  if (!sym_name || !startswith (sym_name, CMSE_PREFIX))
	    continue;

	  /* Special symbol with local binding.  */
	  cmse_invalid = true;
	}
      else
	{
	  cmse_hash = elf32_arm_hash_entry (sym_hashes[i - ext_start]);
	  if (cmse_hash == NULL)
	    continue;

	  sym_name = (char *) cmse_hash->root.root.root.string;
	  if (!startswith (sym_name, CMSE_PREFIX))
	    continue;

	  /* Special symbol has incorrect binding or type.  */
	  if ((cmse_hash->root.root.type != bfd_link_hash_defined
	       && cmse_hash->root.root.type != bfd_link_hash_defweak)
	      || cmse_hash->root.type != STT_FUNC)
	    cmse_invalid = true;
	}

      if (!is_v8m)
	{
	  _bfd_error_handler (_("%pB: special symbol `%s' only allowed for "
				"ARMv8-M architecture or later"),
			      input_bfd, sym_name);
	  is_v8m = true; /* Avoid multiple warning.  */
	  ret = false;
	}

      if (cmse_invalid)
	{
	  _bfd_error_handler (_("%pB: invalid special symbol `%s'; it must be"
				" a global or weak function symbol"),
			      input_bfd, sym_name);
	  ret = false;
	  if (i < ext_start)
	    continue;
	}

      sym_name += strlen (CMSE_PREFIX);
      hash = (struct elf32_arm_link_hash_entry *)
	elf_link_hash_lookup (&(htab)->root, sym_name, false, false, true);

      /* No associated normal symbol or it is neither global nor weak.  */
      if (!hash
	  || (hash->root.root.type != bfd_link_hash_defined
	      && hash->root.root.type != bfd_link_hash_defweak)
	  || hash->root.type != STT_FUNC)
	{
	  /* Initialize here to avoid warning about use of possibly
	     uninitialized variable.  */
	  j = 0;

	  if (!hash)
	    {
	      /* Searching for a normal symbol with local binding.  */
	      for (; j < ext_start; j++)
		{
		  lsym_name =
		    bfd_elf_string_from_elf_section (input_bfd,
						     symtab_hdr->sh_link,
						     local_syms[j].st_name);
		  if (!strcmp (sym_name, lsym_name))
		    break;
		}
	    }

	  if (hash || j < ext_start)
	    {
	      _bfd_error_handler
		(_("%pB: invalid standard symbol `%s'; it must be "
		   "a global or weak function symbol"),
		 input_bfd, sym_name);
	    }
	  else
	    _bfd_error_handler
	      (_("%pB: absent standard symbol `%s'"), input_bfd, sym_name);
	  ret = false;
	  if (!hash)
	    continue;
	}

      sym_value = hash->root.root.u.def.value;
      section = hash->root.root.u.def.section;

      if (cmse_hash->root.root.u.def.section != section)
	{
	  _bfd_error_handler
	    (_("%pB: `%s' and its special symbol are in different sections"),
	     input_bfd, sym_name);
	  ret = false;
	}
      if (cmse_hash->root.root.u.def.value != sym_value)
	continue; /* Ignore: could be an entry function starting with SG.  */

	/* If this section is a link-once section that will be discarded, then
	   don't create any stubs.  */
      if (section->output_section == NULL)
	{
	  _bfd_error_handler
	    (_("%pB: entry function `%s' not output"), input_bfd, sym_name);
	  continue;
	}

      if (hash->root.size == 0)
	{
	  _bfd_error_handler
	    (_("%pB: entry function `%s' is empty"), input_bfd, sym_name);
	  ret = false;
	}

      if (!ret)
	continue;
      branch_type = ARM_GET_SYM_BRANCH_TYPE (hash->root.target_internal);
      stub_entry
	= elf32_arm_create_stub (htab, arm_stub_cmse_branch_thumb_only,
				 NULL, NULL, section, hash, sym_name,
				 sym_value, branch_type, &new_stub);

      if (stub_entry == NULL)
	 ret = false;
      else
	{
	  BFD_ASSERT (new_stub);
	  (*cmse_stub_created)++;
	}
    }

  if (!symtab_hdr->contents)
    free (local_syms);
  return ret;
}

/* Return TRUE iff a symbol identified by its linker HASH entry is a secure
   code entry function, ie can be called from non secure code without using a
   veneer.  */

static bool
cmse_entry_fct_p (struct elf32_arm_link_hash_entry *hash)
{
  bfd_byte contents[4];
  uint32_t first_insn;
  asection *section;
  file_ptr offset;
  bfd *abfd;

  /* Defined symbol of function type.  */
  if (hash->root.root.type != bfd_link_hash_defined
      && hash->root.root.type != bfd_link_hash_defweak)
    return false;
  if (hash->root.type != STT_FUNC)
    return false;

  /* Read first instruction.  */
  section = hash->root.root.u.def.section;
  abfd = section->owner;
  offset = hash->root.root.u.def.value - section->vma;
  if (!bfd_get_section_contents (abfd, section, contents, offset,
				 sizeof (contents)))
    return false;

  first_insn = bfd_get_32 (abfd, contents);

  /* Starts by SG instruction.  */
  return first_insn == 0xe97fe97f;
}

/* Output the name (in symbol table) of the veneer GEN_ENTRY if it is a new
   secure gateway veneers (ie. the veneers was not in the input import library)
   and there is no output import library (GEN_INFO->out_implib_bfd is NULL.  */

static bool
arm_list_new_cmse_stub (struct bfd_hash_entry *gen_entry, void *gen_info)
{
  struct elf32_arm_stub_hash_entry *stub_entry;
  struct bfd_link_info *info;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_arm_stub_hash_entry *) gen_entry;
  info = (struct bfd_link_info *) gen_info;

  if (info->out_implib_bfd)
    return true;

  if (stub_entry->stub_type != arm_stub_cmse_branch_thumb_only)
    return true;

  if (stub_entry->stub_offset == (bfd_vma) -1)
    _bfd_error_handler ("  %s", stub_entry->output_name);

  return true;
}

/* Set offset of each secure gateway veneers so that its address remain
   identical to the one in the input import library referred by
   HTAB->in_implib_bfd.  A warning is issued for veneers that disappeared
   (present in input import library but absent from the executable being
   linked) or if new veneers appeared and there is no output import library
   (INFO->out_implib_bfd is NULL and *CMSE_STUB_CREATED is bigger than the
   number of secure gateway veneers found in the input import library.

   The function returns whether an error occurred.  If no error occurred,
   *CMSE_STUB_CREATED gives the number of SG veneers created by both cmse_scan
   and this function and HTAB->new_cmse_stub_offset is set to the biggest
   veneer observed set for new veneers to be layed out after.  */

static bool
set_cmse_veneer_addr_from_implib (struct bfd_link_info *info,
				  struct elf32_arm_link_hash_table *htab,
				  int *cmse_stub_created)
{
  long symsize;
  char *sym_name;
  flagword flags;
  long i, symcount;
  bfd *in_implib_bfd;
  asection *stub_out_sec;
  bool ret = true;
  Elf_Internal_Sym *intsym;
  const char *out_sec_name;
  bfd_size_type cmse_stub_size;
  asymbol **sympp = NULL, *sym;
  struct elf32_arm_link_hash_entry *hash;
  const insn_sequence *cmse_stub_template;
  struct elf32_arm_stub_hash_entry *stub_entry;
  int cmse_stub_template_size, new_cmse_stubs_created = *cmse_stub_created;
  bfd_vma veneer_value, stub_offset, next_cmse_stub_offset;
  bfd_vma cmse_stub_array_start = (bfd_vma) -1, cmse_stub_sec_vma = 0;

  /* No input secure gateway import library.  */
  if (!htab->in_implib_bfd)
    return true;

  in_implib_bfd = htab->in_implib_bfd;
  if (!htab->cmse_implib)
    {
      _bfd_error_handler (_("%pB: --in-implib only supported for Secure "
			    "Gateway import libraries"), in_implib_bfd);
      return false;
    }

  /* Get symbol table size.  */
  symsize = bfd_get_symtab_upper_bound (in_implib_bfd);
  if (symsize < 0)
    return false;

  /* Read in the input secure gateway import library's symbol table.  */
  sympp = (asymbol **) bfd_malloc (symsize);
  if (sympp == NULL)
    return false;

  symcount = bfd_canonicalize_symtab (in_implib_bfd, sympp);
  if (symcount < 0)
    {
      ret = false;
      goto free_sym_buf;
    }

  htab->new_cmse_stub_offset = 0;
  cmse_stub_size =
    find_stub_size_and_template (arm_stub_cmse_branch_thumb_only,
				 &cmse_stub_template,
				 &cmse_stub_template_size);
  out_sec_name =
    arm_dedicated_stub_output_section_name (arm_stub_cmse_branch_thumb_only);
  stub_out_sec =
    bfd_get_section_by_name (htab->obfd, out_sec_name);
  if (stub_out_sec != NULL)
    cmse_stub_sec_vma = stub_out_sec->vma;

  /* Set addresses of veneers mentionned in input secure gateway import
     library's symbol table.  */
  for (i = 0; i < symcount; i++)
    {
      sym = sympp[i];
      flags = sym->flags;
      sym_name = (char *) bfd_asymbol_name (sym);
      intsym = &((elf_symbol_type *) sym)->internal_elf_sym;

      if (sym->section != bfd_abs_section_ptr
	  || !(flags & (BSF_GLOBAL | BSF_WEAK))
	  || (flags & BSF_FUNCTION) != BSF_FUNCTION
	  || (ARM_GET_SYM_BRANCH_TYPE (intsym->st_target_internal)
	      != ST_BRANCH_TO_THUMB))
	{
	  _bfd_error_handler (_("%pB: invalid import library entry: `%s'; "
				"symbol should be absolute, global and "
				"refer to Thumb functions"),
			      in_implib_bfd, sym_name);
	  ret = false;
	  continue;
	}

      veneer_value = bfd_asymbol_value (sym);
      stub_offset = veneer_value - cmse_stub_sec_vma;
      stub_entry = arm_stub_hash_lookup (&htab->stub_hash_table, sym_name,
					 false, false);
      hash = (struct elf32_arm_link_hash_entry *)
	elf_link_hash_lookup (&(htab)->root, sym_name, false, false, true);

      /* Stub entry should have been created by cmse_scan or the symbol be of
	 a secure function callable from non secure code.  */
      if (!stub_entry && !hash)
	{
	  bool new_stub;

	  _bfd_error_handler
	    (_("entry function `%s' disappeared from secure code"), sym_name);
	  hash = (struct elf32_arm_link_hash_entry *)
	    elf_link_hash_lookup (&(htab)->root, sym_name, true, true, true);
	  stub_entry
	    = elf32_arm_create_stub (htab, arm_stub_cmse_branch_thumb_only,
				     NULL, NULL, bfd_abs_section_ptr, hash,
				     sym_name, veneer_value,
				     ST_BRANCH_TO_THUMB, &new_stub);
	  if (stub_entry == NULL)
	    ret = false;
	  else
	  {
	    BFD_ASSERT (new_stub);
	    new_cmse_stubs_created++;
	    (*cmse_stub_created)++;
	  }
	  stub_entry->stub_template_size = stub_entry->stub_size = 0;
	  stub_entry->stub_offset = stub_offset;
	}
      /* Symbol found is not callable from non secure code.  */
      else if (!stub_entry)
	{
	  if (!cmse_entry_fct_p (hash))
	    {
	      _bfd_error_handler (_("`%s' refers to a non entry function"),
				  sym_name);
	      ret = false;
	    }
	  continue;
	}
      else
	{
	  /* Only stubs for SG veneers should have been created.  */
	  BFD_ASSERT (stub_entry->stub_type == arm_stub_cmse_branch_thumb_only);

	  /* Check visibility hasn't changed.  */
	  if (!!(flags & BSF_GLOBAL)
	      != (hash->root.root.type == bfd_link_hash_defined))
	    _bfd_error_handler
	      (_("%pB: visibility of symbol `%s' has changed"), in_implib_bfd,
	       sym_name);

	  stub_entry->stub_offset = stub_offset;
	}

      /* Size should match that of a SG veneer.  */
      if (intsym->st_size != cmse_stub_size)
	{
	  _bfd_error_handler (_("%pB: incorrect size for symbol `%s'"),
			      in_implib_bfd, sym_name);
	  ret = false;
	}

      /* Previous veneer address is before current SG veneer section.  */
      if (veneer_value < cmse_stub_sec_vma)
	{
	  /* Avoid offset underflow.  */
	  if (stub_entry)
	    stub_entry->stub_offset = 0;
	  stub_offset = 0;
	  ret = false;
	}

      /* Complain if stub offset not a multiple of stub size.  */
      if (stub_offset % cmse_stub_size)
	{
	  _bfd_error_handler
	    (_("offset of veneer for entry function `%s' not a multiple of "
	       "its size"), sym_name);
	  ret = false;
	}

      if (!ret)
	continue;

      new_cmse_stubs_created--;
      if (veneer_value < cmse_stub_array_start)
	cmse_stub_array_start = veneer_value;
      next_cmse_stub_offset = stub_offset + ((cmse_stub_size + 7) & ~7);
      if (next_cmse_stub_offset > htab->new_cmse_stub_offset)
	htab->new_cmse_stub_offset = next_cmse_stub_offset;
    }

  if (!info->out_implib_bfd && new_cmse_stubs_created != 0)
    {
      BFD_ASSERT (new_cmse_stubs_created > 0);
      _bfd_error_handler
	(_("new entry function(s) introduced but no output import library "
	   "specified:"));
      bfd_hash_traverse (&htab->stub_hash_table, arm_list_new_cmse_stub, info);
    }

  if (cmse_stub_array_start != cmse_stub_sec_vma)
    {
      _bfd_error_handler
	(_("start address of `%s' is different from previous link"),
	 out_sec_name);
      ret = false;
    }

 free_sym_buf:
  free (sympp);
  return ret;
}

/* Determine and set the size of the stub section for a final link.

   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with a "bl"
   instruction.  */

bool
elf32_arm_size_stubs (bfd *output_bfd,
		      bfd *stub_bfd,
		      struct bfd_link_info *info,
		      bfd_signed_vma group_size,
		      asection * (*add_stub_section) (const char *, asection *,
						      asection *,
						      unsigned int),
		      void (*layout_sections_again) (void))
{
  bool ret = true;
  obj_attribute *out_attr;
  int cmse_stub_created = 0;
  bfd_size_type stub_group_size;
  bool m_profile, stubs_always_after_branch, first_veneer_scan = true;
  struct elf32_arm_link_hash_table *htab = elf32_arm_hash_table (info);
  struct a8_erratum_fix *a8_fixes = NULL;
  unsigned int num_a8_fixes = 0, a8_fix_table_size = 10;
  struct a8_erratum_reloc *a8_relocs = NULL;
  unsigned int num_a8_relocs = 0, a8_reloc_table_size = 10, i;

  if (htab == NULL)
    return false;

  if (htab->fix_cortex_a8)
    {
      a8_fixes = (struct a8_erratum_fix *)
	  bfd_zmalloc (sizeof (struct a8_erratum_fix) * a8_fix_table_size);
      a8_relocs = (struct a8_erratum_reloc *)
	  bfd_zmalloc (sizeof (struct a8_erratum_reloc) * a8_reloc_table_size);
    }

  /* Propagate mach to stub bfd, because it may not have been
     finalized when we created stub_bfd.  */
  bfd_set_arch_mach (stub_bfd, bfd_get_arch (output_bfd),
		     bfd_get_mach (output_bfd));

  /* Stash our params away.  */
  htab->stub_bfd = stub_bfd;
  htab->add_stub_section = add_stub_section;
  htab->layout_sections_again = layout_sections_again;
  stubs_always_after_branch = group_size < 0;

  out_attr = elf_known_obj_attributes_proc (output_bfd);
  m_profile = out_attr[Tag_CPU_arch_profile].i == 'M';

  /* The Cortex-A8 erratum fix depends on stubs not being in the same 4K page
     as the first half of a 32-bit branch straddling two 4K pages.  This is a
     crude way of enforcing that.  */
  if (htab->fix_cortex_a8)
    stubs_always_after_branch = 1;

  if (group_size < 0)
    stub_group_size = -group_size;
  else
    stub_group_size = group_size;

  if (stub_group_size == 1)
    {
      /* Default values.  */
      /* Thumb branch range is +-4MB has to be used as the default
	 maximum size (a given section can contain both ARM and Thumb
	 code, so the worst case has to be taken into account).

	 This value is 24K less than that, which allows for 2025
	 12-byte stubs.  If we exceed that, then we will fail to link.
	 The user will have to relink with an explicit group size
	 option.  */
      stub_group_size = 4170000;
    }

  group_sections (htab, stub_group_size, stubs_always_after_branch);

  /* If we're applying the cortex A8 fix, we need to determine the
     program header size now, because we cannot change it later --
     that could alter section placements.  Notice the A8 erratum fix
     ends up requiring the section addresses to remain unchanged
     modulo the page size.  That's something we cannot represent
     inside BFD, and we don't want to force the section alignment to
     be the page size.  */
  if (htab->fix_cortex_a8)
    (*htab->layout_sections_again) ();

  while (1)
    {
      bfd *input_bfd;
      unsigned int bfd_indx;
      asection *stub_sec;
      enum elf32_arm_stub_type stub_type;
      bool stub_changed = false;
      unsigned prev_num_a8_fixes = num_a8_fixes;

      num_a8_fixes = 0;
      for (input_bfd = info->input_bfds, bfd_indx = 0;
	   input_bfd != NULL;
	   input_bfd = input_bfd->link.next, bfd_indx++)
	{
	  Elf_Internal_Shdr *symtab_hdr;
	  asection *section;
	  Elf_Internal_Sym *local_syms = NULL;

	  if (!is_arm_elf (input_bfd))
	    continue;
	  if ((input_bfd->flags & DYNAMIC) != 0
	      && (elf_sym_hashes (input_bfd) == NULL
		  || (elf_dyn_lib_class (input_bfd) & DYN_AS_NEEDED) != 0))
	    continue;

	  num_a8_relocs = 0;

	  /* We'll need the symbol table in a second.  */
	  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
	  if (symtab_hdr->sh_info == 0)
	    continue;

	  /* Limit scan of symbols to object file whose profile is
	     Microcontroller to not hinder performance in the general case.  */
	  if (m_profile && first_veneer_scan)
	    {
	      struct elf_link_hash_entry **sym_hashes;

	      sym_hashes = elf_sym_hashes (input_bfd);
	      if (!cmse_scan (input_bfd, htab, out_attr, sym_hashes,
			      &cmse_stub_created))
		goto error_ret_free_local;

	      if (cmse_stub_created != 0)
		stub_changed = true;
	    }

	  /* Walk over each section attached to the input bfd.  */
	  for (section = input_bfd->sections;
	       section != NULL;
	       section = section->next)
	    {
	      Elf_Internal_Rela *internal_relocs, *irelaend, *irela;

	      /* If there aren't any relocs, then there's nothing more
		 to do.  */
	      if ((section->flags & SEC_RELOC) == 0
		  || section->reloc_count == 0
		  || (section->flags & SEC_CODE) == 0)
		continue;

	      /* If this section is a link-once section that will be
		 discarded, then don't create any stubs.  */
	      if (section->output_section == NULL
		  || section->output_section->owner != output_bfd)
		continue;

	      /* Get the relocs.  */
	      internal_relocs
		= _bfd_elf_link_read_relocs (input_bfd, section, NULL,
					     NULL, info->keep_memory);
	      if (internal_relocs == NULL)
		goto error_ret_free_local;

	      /* Now examine each relocation.  */
	      irela = internal_relocs;
	      irelaend = irela + section->reloc_count;
	      for (; irela < irelaend; irela++)
		{
		  unsigned int r_type, r_indx;
		  asection *sym_sec;
		  bfd_vma sym_value;
		  bfd_vma destination;
		  struct elf32_arm_link_hash_entry *hash;
		  const char *sym_name;
		  unsigned char st_type;
		  enum arm_st_branch_type branch_type;
		  bool created_stub = false;

		  r_type = ELF32_R_TYPE (irela->r_info);
		  r_indx = ELF32_R_SYM (irela->r_info);

		  if (r_type >= (unsigned int) R_ARM_max)
		    {
		      bfd_set_error (bfd_error_bad_value);
		    error_ret_free_internal:
		      if (elf_section_data (section)->relocs == NULL)
			free (internal_relocs);
		    /* Fall through.  */
		    error_ret_free_local:
		      if (symtab_hdr->contents != (unsigned char *) local_syms)
			free (local_syms);
		      return false;
		    }

		  hash = NULL;
		  if (r_indx >= symtab_hdr->sh_info)
		    hash = elf32_arm_hash_entry
		      (elf_sym_hashes (input_bfd)
		       [r_indx - symtab_hdr->sh_info]);

		  /* Only look for stubs on branch instructions, or
		     non-relaxed TLSCALL  */
		  if ((r_type != (unsigned int) R_ARM_CALL)
		      && (r_type != (unsigned int) R_ARM_THM_CALL)
		      && (r_type != (unsigned int) R_ARM_JUMP24)
		      && (r_type != (unsigned int) R_ARM_THM_JUMP19)
		      && (r_type != (unsigned int) R_ARM_THM_XPC22)
		      && (r_type != (unsigned int) R_ARM_THM_JUMP24)
		      && (r_type != (unsigned int) R_ARM_PLT32)
		      && !((r_type == (unsigned int) R_ARM_TLS_CALL
			    || r_type == (unsigned int) R_ARM_THM_TLS_CALL)
			   && r_type == (elf32_arm_tls_transition
					 (info, r_type,
					  (struct elf_link_hash_entry *) hash))
			   && ((hash ? hash->tls_type
				: (elf32_arm_local_got_tls_type
				   (input_bfd)[r_indx]))
			       & GOT_TLS_GDESC) != 0))
		    continue;

		  /* Now determine the call target, its name, value,
		     section.  */
		  sym_sec = NULL;
		  sym_value = 0;
		  destination = 0;
		  sym_name = NULL;

		  if (r_type == (unsigned int) R_ARM_TLS_CALL
		      || r_type == (unsigned int) R_ARM_THM_TLS_CALL)
		    {
		      /* A non-relaxed TLS call.  The target is the
			 plt-resident trampoline and nothing to do
			 with the symbol.  */
		      BFD_ASSERT (htab->tls_trampoline > 0);
		      sym_sec = htab->root.splt;
		      sym_value = htab->tls_trampoline;
		      hash = 0;
		      st_type = STT_FUNC;
		      branch_type = ST_BRANCH_TO_ARM;
		    }
		  else if (!hash)
		    {
		      /* It's a local symbol.  */
		      Elf_Internal_Sym *sym;

		      if (local_syms == NULL)
			{
			  local_syms
			    = (Elf_Internal_Sym *) symtab_hdr->contents;
			  if (local_syms == NULL)
			    local_syms
			      = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
						      symtab_hdr->sh_info, 0,
						      NULL, NULL, NULL);
			  if (local_syms == NULL)
			    goto error_ret_free_internal;
			}

		      sym = local_syms + r_indx;
		      if (sym->st_shndx == SHN_UNDEF)
			sym_sec = bfd_und_section_ptr;
		      else if (sym->st_shndx == SHN_ABS)
			sym_sec = bfd_abs_section_ptr;
		      else if (sym->st_shndx == SHN_COMMON)
			sym_sec = bfd_com_section_ptr;
		      else
			sym_sec =
			  bfd_section_from_elf_index (input_bfd, sym->st_shndx);

		      if (!sym_sec)
			/* This is an undefined symbol.  It can never
			   be resolved.  */
			continue;

		      if (ELF_ST_TYPE (sym->st_info) != STT_SECTION)
			sym_value = sym->st_value;
		      destination = (sym_value + irela->r_addend
				     + sym_sec->output_offset
				     + sym_sec->output_section->vma);
		      st_type = ELF_ST_TYPE (sym->st_info);
		      branch_type =
			ARM_GET_SYM_BRANCH_TYPE (sym->st_target_internal);
		      sym_name
			= bfd_elf_string_from_elf_section (input_bfd,
							   symtab_hdr->sh_link,
							   sym->st_name);
		    }
		  else
		    {
		      /* It's an external symbol.  */
		      while (hash->root.root.type == bfd_link_hash_indirect
			     || hash->root.root.type == bfd_link_hash_warning)
			hash = ((struct elf32_arm_link_hash_entry *)
				hash->root.root.u.i.link);

		      if (hash->root.root.type == bfd_link_hash_defined
			  || hash->root.root.type == bfd_link_hash_defweak)
			{
			  sym_sec = hash->root.root.u.def.section;
			  sym_value = hash->root.root.u.def.value;

			  struct elf32_arm_link_hash_table *globals =
						  elf32_arm_hash_table (info);

			  /* For a destination in a shared library,
			     use the PLT stub as target address to
			     decide whether a branch stub is
			     needed.  */
			  if (globals != NULL
			      && globals->root.splt != NULL
			      && hash != NULL
			      && hash->root.plt.offset != (bfd_vma) -1)
			    {
			      sym_sec = globals->root.splt;
			      sym_value = hash->root.plt.offset;
			      if (sym_sec->output_section != NULL)
				destination = (sym_value
					       + sym_sec->output_offset
					       + sym_sec->output_section->vma);
			    }
			  else if (sym_sec->output_section != NULL)
			    destination = (sym_value + irela->r_addend
					   + sym_sec->output_offset
					   + sym_sec->output_section->vma);
			}
		      else if ((hash->root.root.type == bfd_link_hash_undefined)
			       || (hash->root.root.type == bfd_link_hash_undefweak))
			{
			  /* For a shared library, use the PLT stub as
			     target address to decide whether a long
			     branch stub is needed.
			     For absolute code, they cannot be handled.  */
			  struct elf32_arm_link_hash_table *globals =
			    elf32_arm_hash_table (info);

			  if (globals != NULL
			      && globals->root.splt != NULL
			      && hash != NULL
			      && hash->root.plt.offset != (bfd_vma) -1)
			    {
			      sym_sec = globals->root.splt;
			      sym_value = hash->root.plt.offset;
			      if (sym_sec->output_section != NULL)
				destination = (sym_value
					       + sym_sec->output_offset
					       + sym_sec->output_section->vma);
			    }
			  else
			    continue;
			}
		      else
			{
			  bfd_set_error (bfd_error_bad_value);
			  goto error_ret_free_internal;
			}
		      st_type = hash->root.type;
		      branch_type =
			ARM_GET_SYM_BRANCH_TYPE (hash->root.target_internal);
		      sym_name = hash->root.root.root.string;
		    }

		  do
		    {
		      bool new_stub;
		      struct elf32_arm_stub_hash_entry *stub_entry;

		      /* Determine what (if any) linker stub is needed.  */
		      stub_type = arm_type_of_stub (info, section, irela,
						    st_type, &branch_type,
						    hash, destination, sym_sec,
						    input_bfd, sym_name);
		      if (stub_type == arm_stub_none)
			break;

		      /* We've either created a stub for this reloc already,
			 or we are about to.  */
		      stub_entry =
			elf32_arm_create_stub (htab, stub_type, section, irela,
					       sym_sec, hash,
					       (char *) sym_name, sym_value,
					       branch_type, &new_stub);

		      created_stub = stub_entry != NULL;
		      if (!created_stub)
			goto error_ret_free_internal;
		      else if (!new_stub)
			break;
		      else
			stub_changed = true;
		    }
		  while (0);

		  /* Look for relocations which might trigger Cortex-A8
		     erratum.  */
		  if (htab->fix_cortex_a8
		      && (r_type == (unsigned int) R_ARM_THM_JUMP24
			  || r_type == (unsigned int) R_ARM_THM_JUMP19
			  || r_type == (unsigned int) R_ARM_THM_CALL
			  || r_type == (unsigned int) R_ARM_THM_XPC22))
		    {
		      bfd_vma from = section->output_section->vma
				     + section->output_offset
				     + irela->r_offset;

		      if ((from & 0xfff) == 0xffe)
			{
			  /* Found a candidate.  Note we haven't checked the
			     destination is within 4K here: if we do so (and
			     don't create an entry in a8_relocs) we can't tell
			     that a branch should have been relocated when
			     scanning later.  */
			  if (num_a8_relocs == a8_reloc_table_size)
			    {
			      a8_reloc_table_size *= 2;
			      a8_relocs = (struct a8_erratum_reloc *)
				  bfd_realloc (a8_relocs,
					       sizeof (struct a8_erratum_reloc)
					       * a8_reloc_table_size);
			    }

			  a8_relocs[num_a8_relocs].from = from;
			  a8_relocs[num_a8_relocs].destination = destination;
			  a8_relocs[num_a8_relocs].r_type = r_type;
			  a8_relocs[num_a8_relocs].branch_type = branch_type;
			  a8_relocs[num_a8_relocs].sym_name = sym_name;
			  a8_relocs[num_a8_relocs].non_a8_stub = created_stub;
			  a8_relocs[num_a8_relocs].hash = hash;

			  num_a8_relocs++;
			}
		    }
		}

	      /* We're done with the internal relocs, free them.  */
	      if (elf_section_data (section)->relocs == NULL)
		free (internal_relocs);
	    }

	  if (htab->fix_cortex_a8)
	    {
	      /* Sort relocs which might apply to Cortex-A8 erratum.  */
	      qsort (a8_relocs, num_a8_relocs,
		     sizeof (struct a8_erratum_reloc),
		     &a8_reloc_compare);

	      /* Scan for branches which might trigger Cortex-A8 erratum.  */
	      if (cortex_a8_erratum_scan (input_bfd, info, &a8_fixes,
					  &num_a8_fixes, &a8_fix_table_size,
					  a8_relocs, num_a8_relocs,
					  prev_num_a8_fixes, &stub_changed)
		  != 0)
		goto error_ret_free_local;
	    }

	  if (local_syms != NULL
	      && symtab_hdr->contents != (unsigned char *) local_syms)
	    {
	      if (!info->keep_memory)
		free (local_syms);
	      else
		symtab_hdr->contents = (unsigned char *) local_syms;
	    }
	}

      if (first_veneer_scan
	  && !set_cmse_veneer_addr_from_implib (info, htab,
						&cmse_stub_created))
	ret = false;

      if (prev_num_a8_fixes != num_a8_fixes)
	stub_changed = true;

      if (!stub_changed)
	break;

      /* OK, we've added some stubs.  Find out the new size of the
	 stub sections.  */
      for (stub_sec = htab->stub_bfd->sections;
	   stub_sec != NULL;
	   stub_sec = stub_sec->next)
	{
	  /* Ignore non-stub sections.  */
	  if (!strstr (stub_sec->name, STUB_SUFFIX))
	    continue;

	  stub_sec->size = 0;
	}

      /* Add new SG veneers after those already in the input import
	 library.  */
      for (stub_type = arm_stub_none + 1; stub_type < max_stub_type;
	   stub_type++)
	{
	  bfd_vma *start_offset_p;
	  asection **stub_sec_p;

	  start_offset_p = arm_new_stubs_start_offset_ptr (htab, stub_type);
	  stub_sec_p = arm_dedicated_stub_input_section_ptr (htab, stub_type);
	  if (start_offset_p == NULL)
	    continue;

	  BFD_ASSERT (stub_sec_p != NULL);
	  if (*stub_sec_p != NULL)
	    (*stub_sec_p)->size = *start_offset_p;
	}

      /* Compute stub section size, considering padding.  */
      bfd_hash_traverse (&htab->stub_hash_table, arm_size_one_stub, htab);
      for (stub_type = arm_stub_none + 1; stub_type < max_stub_type;
	   stub_type++)
	{
	  int size, padding;
	  asection **stub_sec_p;

	  padding = arm_dedicated_stub_section_padding (stub_type);
	  stub_sec_p = arm_dedicated_stub_input_section_ptr (htab, stub_type);
	  /* Skip if no stub input section or no stub section padding
	     required.  */
	  if ((stub_sec_p != NULL && *stub_sec_p == NULL) || padding == 0)
	    continue;
	  /* Stub section padding required but no dedicated section.  */
	  BFD_ASSERT (stub_sec_p);

	  size = (*stub_sec_p)->size;
	  size = (size + padding - 1) & ~(padding - 1);
	  (*stub_sec_p)->size = size;
	}

      /* Add Cortex-A8 erratum veneers to stub section sizes too.  */
      if (htab->fix_cortex_a8)
	for (i = 0; i < num_a8_fixes; i++)
	  {
	    stub_sec = elf32_arm_create_or_find_stub_sec (NULL,
			 a8_fixes[i].section, htab, a8_fixes[i].stub_type);

	    if (stub_sec == NULL)
	      return false;

	    stub_sec->size
	      += find_stub_size_and_template (a8_fixes[i].stub_type, NULL,
					      NULL);
	  }


      /* Ask the linker to do its stuff.  */
      (*htab->layout_sections_again) ();
      first_veneer_scan = false;
    }

  /* Add stubs for Cortex-A8 erratum fixes now.  */
  if (htab->fix_cortex_a8)
    {
      for (i = 0; i < num_a8_fixes; i++)
	{
	  struct elf32_arm_stub_hash_entry *stub_entry;
	  char *stub_name = a8_fixes[i].stub_name;
	  asection *section = a8_fixes[i].section;
	  unsigned int section_id = a8_fixes[i].section->id;
	  asection *link_sec = htab->stub_group[section_id].link_sec;
	  asection *stub_sec = htab->stub_group[section_id].stub_sec;
	  const insn_sequence *template_sequence;
	  int template_size, size = 0;

	  stub_entry = arm_stub_hash_lookup (&htab->stub_hash_table, stub_name,
					     true, false);
	  if (stub_entry == NULL)
	    {
	      _bfd_error_handler (_("%pB: cannot create stub entry %s"),
				  section->owner, stub_name);
	      return false;
	    }

	  stub_entry->stub_sec = stub_sec;
	  stub_entry->stub_offset = (bfd_vma) -1;
	  stub_entry->id_sec = link_sec;
	  stub_entry->stub_type = a8_fixes[i].stub_type;
	  stub_entry->source_value = a8_fixes[i].offset;
	  stub_entry->target_section = a8_fixes[i].section;
	  stub_entry->target_value = a8_fixes[i].target_offset;
	  stub_entry->orig_insn = a8_fixes[i].orig_insn;
	  stub_entry->branch_type = a8_fixes[i].branch_type;

	  size = find_stub_size_and_template (a8_fixes[i].stub_type,
					      &template_sequence,
					      &template_size);

	  stub_entry->stub_size = size;
	  stub_entry->stub_template = template_sequence;
	  stub_entry->stub_template_size = template_size;
	}

      /* Stash the Cortex-A8 erratum fix array for use later in
	 elf32_arm_write_section().  */
      htab->a8_erratum_fixes = a8_fixes;
      htab->num_a8_erratum_fixes = num_a8_fixes;
    }
  else
    {
      htab->a8_erratum_fixes = NULL;
      htab->num_a8_erratum_fixes = 0;
    }
  return ret;
}

/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  We also set up the .plt entries for statically linked PIC
   functions here.  This function is called via arm_elf_finish in the
   linker.  */

bool
elf32_arm_build_stubs (struct bfd_link_info *info)
{
  asection *stub_sec;
  struct bfd_hash_table *table;
  enum elf32_arm_stub_type stub_type;
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  for (stub_sec = htab->stub_bfd->sections;
       stub_sec != NULL;
       stub_sec = stub_sec->next)
    {
      bfd_size_type size;

      /* Ignore non-stub sections.  */
      if (!strstr (stub_sec->name, STUB_SUFFIX))
	continue;

      /* Allocate memory to hold the linker stubs.  Zeroing the stub sections
	 must at least be done for stub section requiring padding and for SG
	 veneers to ensure that a non secure code branching to a removed SG
	 veneer causes an error.  */
      size = stub_sec->size;
      stub_sec->contents = (unsigned char *) bfd_zalloc (htab->stub_bfd, size);
      if (stub_sec->contents == NULL && size != 0)
	return false;

      stub_sec->size = 0;
    }

  /* Add new SG veneers after those already in the input import library.  */
  for (stub_type = arm_stub_none + 1; stub_type < max_stub_type; stub_type++)
    {
      bfd_vma *start_offset_p;
      asection **stub_sec_p;

      start_offset_p = arm_new_stubs_start_offset_ptr (htab, stub_type);
      stub_sec_p = arm_dedicated_stub_input_section_ptr (htab, stub_type);
      if (start_offset_p == NULL)
	continue;

      BFD_ASSERT (stub_sec_p != NULL);
      if (*stub_sec_p != NULL)
	(*stub_sec_p)->size = *start_offset_p;
    }

  /* Build the stubs as directed by the stub hash table.  */
  table = &htab->stub_hash_table;
  bfd_hash_traverse (table, arm_build_one_stub, info);
  if (htab->fix_cortex_a8)
    {
      /* Place the cortex a8 stubs last.  */
      htab->fix_cortex_a8 = -1;
      bfd_hash_traverse (table, arm_build_one_stub, info);
    }

  return true;
}

/* Locate the Thumb encoded calling stub for NAME.  */

static struct elf_link_hash_entry *
find_thumb_glue (struct bfd_link_info *link_info,
		 const char *name,
		 char **error_message)
{
  char *tmp_name;
  struct elf_link_hash_entry *hash;
  struct elf32_arm_link_hash_table *hash_table;

  /* We need a pointer to the armelf specific hash table.  */
  hash_table = elf32_arm_hash_table (link_info);
  if (hash_table == NULL)
    return NULL;

  tmp_name = (char *) bfd_malloc ((bfd_size_type) strlen (name)
				  + strlen (THUMB2ARM_GLUE_ENTRY_NAME) + 1);

  BFD_ASSERT (tmp_name);

  sprintf (tmp_name, THUMB2ARM_GLUE_ENTRY_NAME, name);

  hash = elf_link_hash_lookup
    (&(hash_table)->root, tmp_name, false, false, true);

  if (hash == NULL)
    {
      *error_message = bfd_asprintf (_("unable to find %s glue '%s' for '%s'"),
				     "Thumb", tmp_name, name);
      if (*error_message == NULL)
	*error_message = (char *) bfd_errmsg (bfd_error_system_call);
    }

  free (tmp_name);

  return hash;
}

/* Locate the ARM encoded calling stub for NAME.  */

static struct elf_link_hash_entry *
find_arm_glue (struct bfd_link_info *link_info,
	       const char *name,
	       char **error_message)
{
  char *tmp_name;
  struct elf_link_hash_entry *myh;
  struct elf32_arm_link_hash_table *hash_table;

  /* We need a pointer to the elfarm specific hash table.  */
  hash_table = elf32_arm_hash_table (link_info);
  if (hash_table == NULL)
    return NULL;

  tmp_name = (char *) bfd_malloc ((bfd_size_type) strlen (name)
				  + strlen (ARM2THUMB_GLUE_ENTRY_NAME) + 1);
  BFD_ASSERT (tmp_name);

  sprintf (tmp_name, ARM2THUMB_GLUE_ENTRY_NAME, name);

  myh = elf_link_hash_lookup
    (&(hash_table)->root, tmp_name, false, false, true);

  if (myh == NULL)
    {
      *error_message = bfd_asprintf (_("unable to find %s glue '%s' for '%s'"),
				     "ARM", tmp_name, name);
      if (*error_message == NULL)
	*error_message = (char *) bfd_errmsg (bfd_error_system_call);
    }
  free (tmp_name);

  return myh;
}

/* ARM->Thumb glue (static images):

   .arm
   __func_from_arm:
   ldr r12, __func_addr
   bx  r12
   __func_addr:
   .word func    @ behave as if you saw a ARM_32 reloc.

   (v5t static images)
   .arm
   __func_from_arm:
   ldr pc, __func_addr
   __func_addr:
   .word func    @ behave as if you saw a ARM_32 reloc.

   (relocatable images)
   .arm
   __func_from_arm:
   ldr r12, __func_offset
   add r12, r12, pc
   bx  r12
   __func_offset:
   .word func - .   */

#define ARM2THUMB_STATIC_GLUE_SIZE 12
static const insn32 a2t1_ldr_insn = 0xe59fc000;
static const insn32 a2t2_bx_r12_insn = 0xe12fff1c;
static const insn32 a2t3_func_addr_insn = 0x00000001;

#define ARM2THUMB_V5_STATIC_GLUE_SIZE 8
static const insn32 a2t1v5_ldr_insn = 0xe51ff004;
static const insn32 a2t2v5_func_addr_insn = 0x00000001;

#define ARM2THUMB_PIC_GLUE_SIZE 16
static const insn32 a2t1p_ldr_insn = 0xe59fc004;
static const insn32 a2t2p_add_pc_insn = 0xe08cc00f;
static const insn32 a2t3p_bx_r12_insn = 0xe12fff1c;

/* Thumb->ARM:				Thumb->(non-interworking aware) ARM

     .thumb				.thumb
     .align 2				.align 2
 __func_from_thumb:		    __func_from_thumb:
     bx pc				push {r6, lr}
     nop				ldr  r6, __func_addr
     .arm				mov  lr, pc
     b func				bx   r6
					.arm
				    ;; back_to_thumb
					ldmia r13! {r6, lr}
					bx    lr
				    __func_addr:
					.word	     func  */

#define THUMB2ARM_GLUE_SIZE 8
static const insn16 t2a1_bx_pc_insn = 0x4778;
static const insn16 t2a2_noop_insn = 0x46c0;
static const insn32 t2a3_b_insn = 0xea000000;

#define VFP11_ERRATUM_VENEER_SIZE 8
#define STM32L4XX_ERRATUM_LDM_VENEER_SIZE 16
#define STM32L4XX_ERRATUM_VLDM_VENEER_SIZE 24

#define ARM_BX_VENEER_SIZE 12
static const insn32 armbx1_tst_insn = 0xe3100001;
static const insn32 armbx2_moveq_insn = 0x01a0f000;
static const insn32 armbx3_bx_insn = 0xe12fff10;

#ifndef ELFARM_NABI_C_INCLUDED
static void
arm_allocate_glue_section_space (bfd * abfd, bfd_size_type size, const char * name)
{
  asection * s;
  bfd_byte * contents;

  if (size == 0)
    {
      /* Do not include empty glue sections in the output.  */
      if (abfd != NULL)
	{
	  s = bfd_get_linker_section (abfd, name);
	  if (s != NULL)
	    s->flags |= SEC_EXCLUDE;
	}
      return;
    }

  BFD_ASSERT (abfd != NULL);

  s = bfd_get_linker_section (abfd, name);
  BFD_ASSERT (s != NULL);

  contents = (bfd_byte *) bfd_zalloc (abfd, size);

  BFD_ASSERT (s->size == size);
  s->contents = contents;
}

bool
bfd_elf32_arm_allocate_interworking_sections (struct bfd_link_info * info)
{
  struct elf32_arm_link_hash_table * globals;

  globals = elf32_arm_hash_table (info);
  BFD_ASSERT (globals != NULL);

  arm_allocate_glue_section_space (globals->bfd_of_glue_owner,
				   globals->arm_glue_size,
				   ARM2THUMB_GLUE_SECTION_NAME);

  arm_allocate_glue_section_space (globals->bfd_of_glue_owner,
				   globals->thumb_glue_size,
				   THUMB2ARM_GLUE_SECTION_NAME);

  arm_allocate_glue_section_space (globals->bfd_of_glue_owner,
				   globals->vfp11_erratum_glue_size,
				   VFP11_ERRATUM_VENEER_SECTION_NAME);

  arm_allocate_glue_section_space (globals->bfd_of_glue_owner,
				   globals->stm32l4xx_erratum_glue_size,
				   STM32L4XX_ERRATUM_VENEER_SECTION_NAME);

  arm_allocate_glue_section_space (globals->bfd_of_glue_owner,
				   globals->bx_glue_size,
				   ARM_BX_GLUE_SECTION_NAME);

  return true;
}

/* Allocate space and symbols for calling a Thumb function from Arm mode.
   returns the symbol identifying the stub.  */

static struct elf_link_hash_entry *
record_arm_to_thumb_glue (struct bfd_link_info * link_info,
			  struct elf_link_hash_entry * h)
{
  const char * name = h->root.root.string;
  asection * s;
  char * tmp_name;
  struct elf_link_hash_entry * myh;
  struct bfd_link_hash_entry * bh;
  struct elf32_arm_link_hash_table * globals;
  bfd_vma val;
  bfd_size_type size;

  globals = elf32_arm_hash_table (link_info);
  BFD_ASSERT (globals != NULL);
  BFD_ASSERT (globals->bfd_of_glue_owner != NULL);

  s = bfd_get_linker_section
    (globals->bfd_of_glue_owner, ARM2THUMB_GLUE_SECTION_NAME);

  BFD_ASSERT (s != NULL);

  tmp_name = (char *) bfd_malloc ((bfd_size_type) strlen (name)
				  + strlen (ARM2THUMB_GLUE_ENTRY_NAME) + 1);
  BFD_ASSERT (tmp_name);

  sprintf (tmp_name, ARM2THUMB_GLUE_ENTRY_NAME, name);

  myh = elf_link_hash_lookup
    (&(globals)->root, tmp_name, false, false, true);

  if (myh != NULL)
    {
      /* We've already seen this guy.  */
      free (tmp_name);
      return myh;
    }

  /* The only trick here is using hash_table->arm_glue_size as the value.
     Even though the section isn't allocated yet, this is where we will be
     putting it.  The +1 on the value marks that the stub has not been
     output yet - not that it is a Thumb function.  */
  bh = NULL;
  val = globals->arm_glue_size + 1;
  _bfd_generic_link_add_one_symbol (link_info, globals->bfd_of_glue_owner,
				    tmp_name, BSF_GLOBAL, s, val,
				    NULL, true, false, &bh);

  myh = (struct elf_link_hash_entry *) bh;
  myh->type = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  myh->forced_local = 1;

  free (tmp_name);

  if (bfd_link_pic (link_info)
      || globals->root.is_relocatable_executable
      || globals->pic_veneer)
    size = ARM2THUMB_PIC_GLUE_SIZE;
  else if (globals->use_blx)
    size = ARM2THUMB_V5_STATIC_GLUE_SIZE;
  else
    size = ARM2THUMB_STATIC_GLUE_SIZE;

  s->size += size;
  globals->arm_glue_size += size;

  return myh;
}

/* Allocate space for ARMv4 BX veneers.  */

static void
record_arm_bx_glue (struct bfd_link_info * link_info, int reg)
{
  asection * s;
  struct elf32_arm_link_hash_table *globals;
  char *tmp_name;
  struct elf_link_hash_entry *myh;
  struct bfd_link_hash_entry *bh;
  bfd_vma val;

  /* BX PC does not need a veneer.  */
  if (reg == 15)
    return;

  globals = elf32_arm_hash_table (link_info);
  BFD_ASSERT (globals != NULL);
  BFD_ASSERT (globals->bfd_of_glue_owner != NULL);

  /* Check if this veneer has already been allocated.  */
  if (globals->bx_glue_offset[reg])
    return;

  s = bfd_get_linker_section
    (globals->bfd_of_glue_owner, ARM_BX_GLUE_SECTION_NAME);

  BFD_ASSERT (s != NULL);

  /* Add symbol for veneer.  */
  tmp_name = (char *)
      bfd_malloc ((bfd_size_type) strlen (ARM_BX_GLUE_ENTRY_NAME) + 1);
  BFD_ASSERT (tmp_name);

  sprintf (tmp_name, ARM_BX_GLUE_ENTRY_NAME, reg);

  myh = elf_link_hash_lookup
    (&(globals)->root, tmp_name, false, false, false);

  BFD_ASSERT (myh == NULL);

  bh = NULL;
  val = globals->bx_glue_size;
  _bfd_generic_link_add_one_symbol (link_info, globals->bfd_of_glue_owner,
				    tmp_name, BSF_FUNCTION | BSF_LOCAL, s, val,
				    NULL, true, false, &bh);

  myh = (struct elf_link_hash_entry *) bh;
  myh->type = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  myh->forced_local = 1;

  s->size += ARM_BX_VENEER_SIZE;
  globals->bx_glue_offset[reg] = globals->bx_glue_size | 2;
  globals->bx_glue_size += ARM_BX_VENEER_SIZE;
}


/* Add an entry to the code/data map for section SEC.  */

static void
elf32_arm_section_map_add (asection *sec, char type, bfd_vma vma)
{
  struct _arm_elf_section_data *sec_data = elf32_arm_section_data (sec);
  unsigned int newidx;

  if (sec_data->map == NULL)
    {
      sec_data->map = (elf32_arm_section_map *)
	  bfd_malloc (sizeof (elf32_arm_section_map));
      sec_data->mapcount = 0;
      sec_data->mapsize = 1;
    }

  newidx = sec_data->mapcount++;

  if (sec_data->mapcount > sec_data->mapsize)
    {
      sec_data->mapsize *= 2;
      sec_data->map = (elf32_arm_section_map *)
	  bfd_realloc_or_free (sec_data->map, sec_data->mapsize
			       * sizeof (elf32_arm_section_map));
    }

  if (sec_data->map)
    {
      sec_data->map[newidx].vma = vma;
      sec_data->map[newidx].type = type;
    }
}


/* Record information about a VFP11 denorm-erratum veneer.  Only ARM-mode
   veneers are handled for now.  */

static bfd_vma
record_vfp11_erratum_veneer (struct bfd_link_info *link_info,
			     elf32_vfp11_erratum_list *branch,
			     bfd *branch_bfd,
			     asection *branch_sec,
			     unsigned int offset)
{
  asection *s;
  struct elf32_arm_link_hash_table *hash_table;
  char *tmp_name;
  struct elf_link_hash_entry *myh;
  struct bfd_link_hash_entry *bh;
  bfd_vma val;
  struct _arm_elf_section_data *sec_data;
  elf32_vfp11_erratum_list *newerr;

  hash_table = elf32_arm_hash_table (link_info);
  BFD_ASSERT (hash_table != NULL);
  BFD_ASSERT (hash_table->bfd_of_glue_owner != NULL);

  s = bfd_get_linker_section
    (hash_table->bfd_of_glue_owner, VFP11_ERRATUM_VENEER_SECTION_NAME);

  sec_data = elf32_arm_section_data (s);

  BFD_ASSERT (s != NULL);

  tmp_name = (char *) bfd_malloc ((bfd_size_type) strlen
				  (VFP11_ERRATUM_VENEER_ENTRY_NAME) + 10);
  BFD_ASSERT (tmp_name);

  sprintf (tmp_name, VFP11_ERRATUM_VENEER_ENTRY_NAME,
	   hash_table->num_vfp11_fixes);

  myh = elf_link_hash_lookup
    (&(hash_table)->root, tmp_name, false, false, false);

  BFD_ASSERT (myh == NULL);

  bh = NULL;
  val = hash_table->vfp11_erratum_glue_size;
  _bfd_generic_link_add_one_symbol (link_info, hash_table->bfd_of_glue_owner,
				    tmp_name, BSF_FUNCTION | BSF_LOCAL, s, val,
				    NULL, true, false, &bh);

  myh = (struct elf_link_hash_entry *) bh;
  myh->type = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  myh->forced_local = 1;

  /* Link veneer back to calling location.  */
  sec_data->erratumcount += 1;
  newerr = (elf32_vfp11_erratum_list *)
      bfd_zmalloc (sizeof (elf32_vfp11_erratum_list));

  newerr->type = VFP11_ERRATUM_ARM_VENEER;
  newerr->vma = -1;
  newerr->u.v.branch = branch;
  newerr->u.v.id = hash_table->num_vfp11_fixes;
  branch->u.b.veneer = newerr;

  newerr->next = sec_data->erratumlist;
  sec_data->erratumlist = newerr;

  /* A symbol for the return from the veneer.  */
  sprintf (tmp_name, VFP11_ERRATUM_VENEER_ENTRY_NAME "_r",
	   hash_table->num_vfp11_fixes);

  myh = elf_link_hash_lookup
    (&(hash_table)->root, tmp_name, false, false, false);

  if (myh != NULL)
    abort ();

  bh = NULL;
  val = offset + 4;
  _bfd_generic_link_add_one_symbol (link_info, branch_bfd, tmp_name, BSF_LOCAL,
				    branch_sec, val, NULL, true, false, &bh);

  myh = (struct elf_link_hash_entry *) bh;
  myh->type = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  myh->forced_local = 1;

  free (tmp_name);

  /* Generate a mapping symbol for the veneer section, and explicitly add an
     entry for that symbol to the code/data map for the section.  */
  if (hash_table->vfp11_erratum_glue_size == 0)
    {
      bh = NULL;
      /* FIXME: Creates an ARM symbol.  Thumb mode will need attention if it
	 ever requires this erratum fix.  */
      _bfd_generic_link_add_one_symbol (link_info,
					hash_table->bfd_of_glue_owner, "$a",
					BSF_LOCAL, s, 0, NULL,
					true, false, &bh);

      myh = (struct elf_link_hash_entry *) bh;
      myh->type = ELF_ST_INFO (STB_LOCAL, STT_NOTYPE);
      myh->forced_local = 1;

      /* The elf32_arm_init_maps function only cares about symbols from input
	 BFDs.  We must make a note of this generated mapping symbol
	 ourselves so that code byteswapping works properly in
	 elf32_arm_write_section.  */
      elf32_arm_section_map_add (s, 'a', 0);
    }

  s->size += VFP11_ERRATUM_VENEER_SIZE;
  hash_table->vfp11_erratum_glue_size += VFP11_ERRATUM_VENEER_SIZE;
  hash_table->num_vfp11_fixes++;

  /* The offset of the veneer.  */
  return val;
}

/* Record information about a STM32L4XX STM erratum veneer.  Only THUMB-mode
   veneers need to be handled because used only in Cortex-M.  */

static bfd_vma
record_stm32l4xx_erratum_veneer (struct bfd_link_info *link_info,
				 elf32_stm32l4xx_erratum_list *branch,
				 bfd *branch_bfd,
				 asection *branch_sec,
				 unsigned int offset,
				 bfd_size_type veneer_size)
{
  asection *s;
  struct elf32_arm_link_hash_table *hash_table;
  char *tmp_name;
  struct elf_link_hash_entry *myh;
  struct bfd_link_hash_entry *bh;
  bfd_vma val;
  struct _arm_elf_section_data *sec_data;
  elf32_stm32l4xx_erratum_list *newerr;

  hash_table = elf32_arm_hash_table (link_info);
  BFD_ASSERT (hash_table != NULL);
  BFD_ASSERT (hash_table->bfd_of_glue_owner != NULL);

  s = bfd_get_linker_section
    (hash_table->bfd_of_glue_owner, STM32L4XX_ERRATUM_VENEER_SECTION_NAME);

  BFD_ASSERT (s != NULL);

  sec_data = elf32_arm_section_data (s);

  tmp_name = (char *) bfd_malloc ((bfd_size_type) strlen
				  (STM32L4XX_ERRATUM_VENEER_ENTRY_NAME) + 10);
  BFD_ASSERT (tmp_name);

  sprintf (tmp_name, STM32L4XX_ERRATUM_VENEER_ENTRY_NAME,
	   hash_table->num_stm32l4xx_fixes);

  myh = elf_link_hash_lookup
    (&(hash_table)->root, tmp_name, false, false, false);

  BFD_ASSERT (myh == NULL);

  bh = NULL;
  val = hash_table->stm32l4xx_erratum_glue_size;
  _bfd_generic_link_add_one_symbol (link_info, hash_table->bfd_of_glue_owner,
				    tmp_name, BSF_FUNCTION | BSF_LOCAL, s, val,
				    NULL, true, false, &bh);

  myh = (struct elf_link_hash_entry *) bh;
  myh->type = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  myh->forced_local = 1;

  /* Link veneer back to calling location.  */
  sec_data->stm32l4xx_erratumcount += 1;
  newerr = (elf32_stm32l4xx_erratum_list *)
      bfd_zmalloc (sizeof (elf32_stm32l4xx_erratum_list));

  newerr->type = STM32L4XX_ERRATUM_VENEER;
  newerr->vma = -1;
  newerr->u.v.branch = branch;
  newerr->u.v.id = hash_table->num_stm32l4xx_fixes;
  branch->u.b.veneer = newerr;

  newerr->next = sec_data->stm32l4xx_erratumlist;
  sec_data->stm32l4xx_erratumlist = newerr;

  /* A symbol for the return from the veneer.  */
  sprintf (tmp_name, STM32L4XX_ERRATUM_VENEER_ENTRY_NAME "_r",
	   hash_table->num_stm32l4xx_fixes);

  myh = elf_link_hash_lookup
    (&(hash_table)->root, tmp_name, false, false, false);

  if (myh != NULL)
    abort ();

  bh = NULL;
  val = offset + 4;
  _bfd_generic_link_add_one_symbol (link_info, branch_bfd, tmp_name, BSF_LOCAL,
				    branch_sec, val, NULL, true, false, &bh);

  myh = (struct elf_link_hash_entry *) bh;
  myh->type = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  myh->forced_local = 1;

  free (tmp_name);

  /* Generate a mapping symbol for the veneer section, and explicitly add an
     entry for that symbol to the code/data map for the section.  */
  if (hash_table->stm32l4xx_erratum_glue_size == 0)
    {
      bh = NULL;
      /* Creates a THUMB symbol since there is no other choice.  */
      _bfd_generic_link_add_one_symbol (link_info,
					hash_table->bfd_of_glue_owner, "$t",
					BSF_LOCAL, s, 0, NULL,
					true, false, &bh);

      myh = (struct elf_link_hash_entry *) bh;
      myh->type = ELF_ST_INFO (STB_LOCAL, STT_NOTYPE);
      myh->forced_local = 1;

      /* The elf32_arm_init_maps function only cares about symbols from input
	 BFDs.  We must make a note of this generated mapping symbol
	 ourselves so that code byteswapping works properly in
	 elf32_arm_write_section.  */
      elf32_arm_section_map_add (s, 't', 0);
    }

  s->size += veneer_size;
  hash_table->stm32l4xx_erratum_glue_size += veneer_size;
  hash_table->num_stm32l4xx_fixes++;

  /* The offset of the veneer.  */
  return val;
}

#define ARM_GLUE_SECTION_FLAGS \
  (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_CODE \
   | SEC_READONLY | SEC_LINKER_CREATED)

/* Create a fake section for use by the ARM backend of the linker.  */

static bool
arm_make_glue_section (bfd * abfd, const char * name)
{
  asection * sec;

  sec = bfd_get_linker_section (abfd, name);
  if (sec != NULL)
    /* Already made.  */
    return true;

  sec = bfd_make_section_anyway_with_flags (abfd, name, ARM_GLUE_SECTION_FLAGS);

  if (sec == NULL
      || !bfd_set_section_alignment (sec, 2))
    return false;

  /* Set the gc mark to prevent the section from being removed by garbage
     collection, despite the fact that no relocs refer to this section.  */
  sec->gc_mark = 1;

  return true;
}

/* Set size of .plt entries.  This function is called from the
   linker scripts in ld/emultempl/{armelf}.em.  */

void
bfd_elf32_arm_use_long_plt (void)
{
  elf32_arm_use_long_plt_entry = true;
}

/* Add the glue sections to ABFD.  This function is called from the
   linker scripts in ld/emultempl/{armelf}.em.  */

bool
bfd_elf32_arm_add_glue_sections_to_bfd (bfd *abfd,
					struct bfd_link_info *info)
{
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (info);
  bool dostm32l4xx = globals
    && globals->stm32l4xx_fix != BFD_ARM_STM32L4XX_FIX_NONE;
  bool addglue;

  /* If we are only performing a partial
     link do not bother adding the glue.  */
  if (bfd_link_relocatable (info))
    return true;

  addglue = arm_make_glue_section (abfd, ARM2THUMB_GLUE_SECTION_NAME)
    && arm_make_glue_section (abfd, THUMB2ARM_GLUE_SECTION_NAME)
    && arm_make_glue_section (abfd, VFP11_ERRATUM_VENEER_SECTION_NAME)
    && arm_make_glue_section (abfd, ARM_BX_GLUE_SECTION_NAME);

  if (!dostm32l4xx)
    return addglue;

  return addglue
    && arm_make_glue_section (abfd, STM32L4XX_ERRATUM_VENEER_SECTION_NAME);
}

/* Mark output sections of veneers needing a dedicated one with SEC_KEEP.  This
   ensures they are not marked for deletion by
   strip_excluded_output_sections () when veneers are going to be created
   later.  Not doing so would trigger assert on empty section size in
   lang_size_sections_1 ().  */

void
bfd_elf32_arm_keep_private_stub_output_sections (struct bfd_link_info *info)
{
  enum elf32_arm_stub_type stub_type;

  /* If we are only performing a partial
     link do not bother adding the glue.  */
  if (bfd_link_relocatable (info))
    return;

  for (stub_type = arm_stub_none + 1; stub_type < max_stub_type; stub_type++)
    {
      asection *out_sec;
      const char *out_sec_name;

      if (!arm_dedicated_stub_output_section_required (stub_type))
	continue;

     out_sec_name = arm_dedicated_stub_output_section_name (stub_type);
     out_sec = bfd_get_section_by_name (info->output_bfd, out_sec_name);
     if (out_sec != NULL)
	out_sec->flags |= SEC_KEEP;
    }
}

/* Select a BFD to be used to hold the sections used by the glue code.
   This function is called from the linker scripts in ld/emultempl/
   {armelf/pe}.em.  */

bool
bfd_elf32_arm_get_bfd_for_interworking (bfd *abfd, struct bfd_link_info *info)
{
  struct elf32_arm_link_hash_table *globals;

  /* If we are only performing a partial link
     do not bother getting a bfd to hold the glue.  */
  if (bfd_link_relocatable (info))
    return true;

  /* Make sure we don't attach the glue sections to a dynamic object.  */
  BFD_ASSERT (!(abfd->flags & DYNAMIC));

  globals = elf32_arm_hash_table (info);
  BFD_ASSERT (globals != NULL);

  if (globals->bfd_of_glue_owner != NULL)
    return true;

  /* Save the bfd for later use.  */
  globals->bfd_of_glue_owner = abfd;

  return true;
}

static void
check_use_blx (struct elf32_arm_link_hash_table *globals)
{
  int cpu_arch;

  cpu_arch = bfd_elf_get_obj_attr_int (globals->obfd, OBJ_ATTR_PROC,
				       Tag_CPU_arch);

  if (globals->fix_arm1176)
    {
      if (cpu_arch == TAG_CPU_ARCH_V6T2 || cpu_arch > TAG_CPU_ARCH_V6K)
	globals->use_blx = 1;
    }
  else
    {
      if (cpu_arch > TAG_CPU_ARCH_V4T)
	globals->use_blx = 1;
    }
}

bool
bfd_elf32_arm_process_before_allocation (bfd *abfd,
					 struct bfd_link_info *link_info)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs = NULL;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *contents = NULL;

  asection *sec;
  struct elf32_arm_link_hash_table *globals;

  /* If we are only performing a partial link do not bother
     to construct any glue.  */
  if (bfd_link_relocatable (link_info))
    return true;

  /* Here we have a bfd that is to be included on the link.  We have a
     hook to do reloc rummaging, before section sizes are nailed down.  */
  globals = elf32_arm_hash_table (link_info);
  BFD_ASSERT (globals != NULL);

  check_use_blx (globals);

  if (globals->byteswap_code && !bfd_big_endian (abfd))
    {
      _bfd_error_handler (_("%pB: BE8 images only valid in big-endian mode"),
			  abfd);
      return false;
    }

  /* PR 5398: If we have not decided to include any loadable sections in
     the output then we will not have a glue owner bfd.  This is OK, it
     just means that there is nothing else for us to do here.  */
  if (globals->bfd_of_glue_owner == NULL)
    return true;

  /* Rummage around all the relocs and map the glue vectors.  */
  sec = abfd->sections;

  if (sec == NULL)
    return true;

  for (; sec != NULL; sec = sec->next)
    {
      if (sec->reloc_count == 0)
	continue;

      if ((sec->flags & SEC_EXCLUDE) != 0
	  || (sec->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      symtab_hdr = & elf_symtab_hdr (abfd);

      /* Load the relocs.  */
      internal_relocs
	= _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL, false);

      if (internal_relocs == NULL)
	goto error_return;

      irelend = internal_relocs + sec->reloc_count;
      for (irel = internal_relocs; irel < irelend; irel++)
	{
	  long r_type;
	  unsigned long r_index;

	  struct elf_link_hash_entry *h;

	  r_type = ELF32_R_TYPE (irel->r_info);
	  r_index = ELF32_R_SYM (irel->r_info);

	  /* These are the only relocation types we care about.  */
	  if (   r_type != R_ARM_PC24
	      && (r_type != R_ARM_V4BX || globals->fix_v4bx < 2))
	    continue;

	  /* Get the section contents if we haven't done so already.  */
	  if (contents == NULL)
	    {
	      /* Get cached copy if it exists.  */
	      if (elf_section_data (sec)->this_hdr.contents != NULL)
		contents = elf_section_data (sec)->this_hdr.contents;
	      else
		{
		  /* Go get them off disk.  */
		  if (! bfd_malloc_and_get_section (abfd, sec, &contents))
		    goto error_return;
		}
	    }

	  if (r_type == R_ARM_V4BX)
	    {
	      int reg;

	      reg = bfd_get_32 (abfd, contents + irel->r_offset) & 0xf;
	      record_arm_bx_glue (link_info, reg);
	      continue;
	    }

	  /* If the relocation is not against a symbol it cannot concern us.  */
	  h = NULL;

	  /* We don't care about local symbols.  */
	  if (r_index < symtab_hdr->sh_info)
	    continue;

	  /* This is an external symbol.  */
	  r_index -= symtab_hdr->sh_info;
	  h = (struct elf_link_hash_entry *)
	    elf_sym_hashes (abfd)[r_index];

	  /* If the relocation is against a static symbol it must be within
	     the current section and so cannot be a cross ARM/Thumb relocation.  */
	  if (h == NULL)
	    continue;

	  /* If the call will go through a PLT entry then we do not need
	     glue.  */
	  if (globals->root.splt != NULL && h->plt.offset != (bfd_vma) -1)
	    continue;

	  switch (r_type)
	    {
	    case R_ARM_PC24:
	      /* This one is a call from arm code.  We need to look up
		 the target of the call.  If it is a thumb target, we
		 insert glue.  */
	      if (ARM_GET_SYM_BRANCH_TYPE (h->target_internal)
		  == ST_BRANCH_TO_THUMB)
		record_arm_to_thumb_glue (link_info, h);
	      break;

	    default:
	      abort ();
	    }
	}

      if (elf_section_data (sec)->this_hdr.contents != contents)
	free (contents);
      contents = NULL;

      if (elf_section_data (sec)->relocs != internal_relocs)
	free (internal_relocs);
      internal_relocs = NULL;
    }

  return true;

 error_return:
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);
  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return false;
}
#endif


/* Initialise maps of ARM/Thumb/data for input BFDs.  */

void
bfd_elf32_arm_init_maps (bfd *abfd)
{
  Elf_Internal_Sym *isymbuf;
  Elf_Internal_Shdr *hdr;
  unsigned int i, localsyms;

  /* PR 7093: Make sure that we are dealing with an arm elf binary.  */
  if (! is_arm_elf (abfd))
    return;

  if ((abfd->flags & DYNAMIC) != 0)
    return;

  hdr = & elf_symtab_hdr (abfd);
  localsyms = hdr->sh_info;

  /* Obtain a buffer full of symbols for this BFD. The hdr->sh_info field
     should contain the number of local symbols, which should come before any
     global symbols.  Mapping symbols are always local.  */
  isymbuf = bfd_elf_get_elf_syms (abfd, hdr, localsyms, 0, NULL, NULL,
				  NULL);

  /* No internal symbols read?  Skip this BFD.  */
  if (isymbuf == NULL)
    return;

  for (i = 0; i < localsyms; i++)
    {
      Elf_Internal_Sym *isym = &isymbuf[i];
      asection *sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
      const char *name;

      if (sec != NULL
	  && ELF_ST_BIND (isym->st_info) == STB_LOCAL)
	{
	  name = bfd_elf_string_from_elf_section (abfd,
	    hdr->sh_link, isym->st_name);

	  if (bfd_is_arm_special_symbol_name (name,
					      BFD_ARM_SPECIAL_SYM_TYPE_MAP))
	    elf32_arm_section_map_add (sec, name[1], isym->st_value);
	}
    }
}


/* Auto-select enabling of Cortex-A8 erratum fix if the user didn't explicitly
   say what they wanted.  */

void
bfd_elf32_arm_set_cortex_a8_fix (bfd *obfd, struct bfd_link_info *link_info)
{
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (link_info);
  obj_attribute *out_attr = elf_known_obj_attributes_proc (obfd);

  if (globals == NULL)
    return;

  if (globals->fix_cortex_a8 == -1)
    {
      /* Turn on Cortex-A8 erratum workaround for ARMv7-A.  */
      if (out_attr[Tag_CPU_arch].i == TAG_CPU_ARCH_V7
	  && (out_attr[Tag_CPU_arch_profile].i == 'A'
	      || out_attr[Tag_CPU_arch_profile].i == 0))
	globals->fix_cortex_a8 = 1;
      else
	globals->fix_cortex_a8 = 0;
    }
}


void
bfd_elf32_arm_set_vfp11_fix (bfd *obfd, struct bfd_link_info *link_info)
{
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (link_info);
  obj_attribute *out_attr = elf_known_obj_attributes_proc (obfd);

  if (globals == NULL)
    return;
  /* We assume that ARMv7+ does not need the VFP11 denorm erratum fix.  */
  if (out_attr[Tag_CPU_arch].i >= TAG_CPU_ARCH_V7)
    {
      switch (globals->vfp11_fix)
	{
	case BFD_ARM_VFP11_FIX_DEFAULT:
	case BFD_ARM_VFP11_FIX_NONE:
	  globals->vfp11_fix = BFD_ARM_VFP11_FIX_NONE;
	  break;

	default:
	  /* Give a warning, but do as the user requests anyway.  */
	  _bfd_error_handler (_("%pB: warning: selected VFP11 erratum "
	    "workaround is not necessary for target architecture"), obfd);
	}
    }
  else if (globals->vfp11_fix == BFD_ARM_VFP11_FIX_DEFAULT)
    /* For earlier architectures, we might need the workaround, but do not
       enable it by default.  If users is running with broken hardware, they
       must enable the erratum fix explicitly.  */
    globals->vfp11_fix = BFD_ARM_VFP11_FIX_NONE;
}

void
bfd_elf32_arm_set_stm32l4xx_fix (bfd *obfd, struct bfd_link_info *link_info)
{
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (link_info);
  obj_attribute *out_attr = elf_known_obj_attributes_proc (obfd);

  if (globals == NULL)
    return;

  /* We assume only Cortex-M4 may require the fix.  */
  if (out_attr[Tag_CPU_arch].i != TAG_CPU_ARCH_V7E_M
      || out_attr[Tag_CPU_arch_profile].i != 'M')
    {
      if (globals->stm32l4xx_fix != BFD_ARM_STM32L4XX_FIX_NONE)
	/* Give a warning, but do as the user requests anyway.  */
	_bfd_error_handler
	  (_("%pB: warning: selected STM32L4XX erratum "
	     "workaround is not necessary for target architecture"), obfd);
    }
}

enum bfd_arm_vfp11_pipe
{
  VFP11_FMAC,
  VFP11_LS,
  VFP11_DS,
  VFP11_BAD
};

/* Return a VFP register number.  This is encoded as RX:X for single-precision
   registers, or X:RX for double-precision registers, where RX is the group of
   four bits in the instruction encoding and X is the single extension bit.
   RX and X fields are specified using their lowest (starting) bit.  The return
   value is:

     0...31: single-precision registers s0...s31
     32...63: double-precision registers d0...d31.

   Although X should be zero for VFP11 (encoding d0...d15 only), we might
   encounter VFP3 instructions, so we allow the full range for DP registers.  */

static unsigned int
bfd_arm_vfp11_regno (unsigned int insn, bool is_double, unsigned int rx,
		     unsigned int x)
{
  if (is_double)
    return (((insn >> rx) & 0xf) | (((insn >> x) & 1) << 4)) + 32;
  else
    return (((insn >> rx) & 0xf) << 1) | ((insn >> x) & 1);
}

/* Set bits in *WMASK according to a register number REG as encoded by
   bfd_arm_vfp11_regno().  Ignore d16-d31.  */

static void
bfd_arm_vfp11_write_mask (unsigned int *wmask, unsigned int reg)
{
  if (reg < 32)
    *wmask |= 1 << reg;
  else if (reg < 48)
    *wmask |= 3 << ((reg - 32) * 2);
}

/* Return TRUE if WMASK overwrites anything in REGS.  */

static bool
bfd_arm_vfp11_antidependency (unsigned int wmask, int *regs, int numregs)
{
  int i;

  for (i = 0; i < numregs; i++)
    {
      unsigned int reg = regs[i];

      if (reg < 32 && (wmask & (1 << reg)) != 0)
	return true;

      reg -= 32;

      if (reg >= 16)
	continue;

      if ((wmask & (3 << (reg * 2))) != 0)
	return true;
    }

  return false;
}

/* In this function, we're interested in two things: finding input registers
   for VFP data-processing instructions, and finding the set of registers which
   arbitrary VFP instructions may write to.  We use a 32-bit unsigned int to
   hold the written set, so FLDM etc. are easy to deal with (we're only
   interested in 32 SP registers or 16 dp registers, due to the VFP version
   implemented by the chip in question).  DP registers are marked by setting
   both SP registers in the write mask).  */

static enum bfd_arm_vfp11_pipe
bfd_arm_vfp11_insn_decode (unsigned int insn, unsigned int *destmask, int *regs,
			   int *numregs)
{
  enum bfd_arm_vfp11_pipe vpipe = VFP11_BAD;
  bool is_double = ((insn & 0xf00) == 0xb00) ? 1 : 0;

  if ((insn & 0x0f000e10) == 0x0e000a00)  /* A data-processing insn.  */
    {
      unsigned int pqrs;
      unsigned int fd = bfd_arm_vfp11_regno (insn, is_double, 12, 22);
      unsigned int fm = bfd_arm_vfp11_regno (insn, is_double, 0, 5);

      pqrs = ((insn & 0x00800000) >> 20)
	   | ((insn & 0x00300000) >> 19)
	   | ((insn & 0x00000040) >> 6);

      switch (pqrs)
	{
	case 0: /* fmac[sd].  */
	case 1: /* fnmac[sd].  */
	case 2: /* fmsc[sd].  */
	case 3: /* fnmsc[sd].  */
	  vpipe = VFP11_FMAC;
	  bfd_arm_vfp11_write_mask (destmask, fd);
	  regs[0] = fd;
	  regs[1] = bfd_arm_vfp11_regno (insn, is_double, 16, 7);  /* Fn.  */
	  regs[2] = fm;
	  *numregs = 3;
	  break;

	case 4: /* fmul[sd].  */
	case 5: /* fnmul[sd].  */
	case 6: /* fadd[sd].  */
	case 7: /* fsub[sd].  */
	  vpipe = VFP11_FMAC;
	  goto vfp_binop;

	case 8: /* fdiv[sd].  */
	  vpipe = VFP11_DS;
	  vfp_binop:
	  bfd_arm_vfp11_write_mask (destmask, fd);
	  regs[0] = bfd_arm_vfp11_regno (insn, is_double, 16, 7);   /* Fn.  */
	  regs[1] = fm;
	  *numregs = 2;
	  break;

	case 15: /* extended opcode.  */
	  {
	    unsigned int extn = ((insn >> 15) & 0x1e)
			      | ((insn >> 7) & 1);

	    switch (extn)
	      {
	      case 0: /* fcpy[sd].  */
	      case 1: /* fabs[sd].  */
	      case 2: /* fneg[sd].  */
	      case 8: /* fcmp[sd].  */
	      case 9: /* fcmpe[sd].  */
	      case 10: /* fcmpz[sd].  */
	      case 11: /* fcmpez[sd].  */
	      case 16: /* fuito[sd].  */
	      case 17: /* fsito[sd].  */
	      case 24: /* ftoui[sd].  */
	      case 25: /* ftouiz[sd].  */
	      case 26: /* ftosi[sd].  */
	      case 27: /* ftosiz[sd].  */
		/* These instructions will not bounce due to underflow.  */
		*numregs = 0;
		vpipe = VFP11_FMAC;
		break;

	      case 3: /* fsqrt[sd].  */
		/* fsqrt cannot underflow, but it can (perhaps) overwrite
		   registers to cause the erratum in previous instructions.  */
		bfd_arm_vfp11_write_mask (destmask, fd);
		vpipe = VFP11_DS;
		break;

	      case 15: /* fcvt{ds,sd}.  */
		{
		  int rnum = 0;

		  bfd_arm_vfp11_write_mask (destmask, fd);

		  /* Only FCVTSD can underflow.  */
		  if ((insn & 0x100) != 0)
		    regs[rnum++] = fm;

		  *numregs = rnum;

		  vpipe = VFP11_FMAC;
		}
		break;

	      default:
		return VFP11_BAD;
	      }
	  }
	  break;

	default:
	  return VFP11_BAD;
	}
    }
  /* Two-register transfer.  */
  else if ((insn & 0x0fe00ed0) == 0x0c400a10)
    {
      unsigned int fm = bfd_arm_vfp11_regno (insn, is_double, 0, 5);

      if ((insn & 0x100000) == 0)
	{
	  if (is_double)
	    bfd_arm_vfp11_write_mask (destmask, fm);
	  else
	    {
	      bfd_arm_vfp11_write_mask (destmask, fm);
	      bfd_arm_vfp11_write_mask (destmask, fm + 1);
	    }
	}

      vpipe = VFP11_LS;
    }
  else if ((insn & 0x0e100e00) == 0x0c100a00)  /* A load insn.  */
    {
      int fd = bfd_arm_vfp11_regno (insn, is_double, 12, 22);
      unsigned int puw = ((insn >> 21) & 0x1) | (((insn >> 23) & 3) << 1);

      switch (puw)
	{
	case 0: /* Two-reg transfer.  We should catch these above.  */
	  abort ();

	case 2: /* fldm[sdx].  */
	case 3:
	case 5:
	  {
	    unsigned int i, offset = insn & 0xff;

	    if (is_double)
	      offset >>= 1;

	    for (i = fd; i < fd + offset; i++)
	      bfd_arm_vfp11_write_mask (destmask, i);
	  }
	  break;

	case 4: /* fld[sd].  */
	case 6:
	  bfd_arm_vfp11_write_mask (destmask, fd);
	  break;

	default:
	  return VFP11_BAD;
	}

      vpipe = VFP11_LS;
    }
  /* Single-register transfer. Note L==0.  */
  else if ((insn & 0x0f100e10) == 0x0e000a10)
    {
      unsigned int opcode = (insn >> 21) & 7;
      unsigned int fn = bfd_arm_vfp11_regno (insn, is_double, 16, 7);

      switch (opcode)
	{
	case 0: /* fmsr/fmdlr.  */
	case 1: /* fmdhr.  */
	  /* Mark fmdhr and fmdlr as writing to the whole of the DP
	     destination register.  I don't know if this is exactly right,
	     but it is the conservative choice.  */
	  bfd_arm_vfp11_write_mask (destmask, fn);
	  break;

	case 7: /* fmxr.  */
	  break;
	}

      vpipe = VFP11_LS;
    }

  return vpipe;
}


static int elf32_arm_compare_mapping (const void * a, const void * b);


/* Look for potentially-troublesome code sequences which might trigger the
   VFP11 denormal/antidependency erratum.  See, e.g., the ARM1136 errata sheet
   (available from ARM) for details of the erratum.  A short version is
   described in ld.texinfo.  */

bool
bfd_elf32_arm_vfp11_erratum_scan (bfd *abfd, struct bfd_link_info *link_info)
{
  asection *sec;
  bfd_byte *contents = NULL;
  int state = 0;
  int regs[3], numregs = 0;
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (link_info);
  int use_vector = (globals->vfp11_fix == BFD_ARM_VFP11_FIX_VECTOR);

  if (globals == NULL)
    return false;

  /* We use a simple FSM to match troublesome VFP11 instruction sequences.
     The states transition as follows:

       0 -> 1 (vector) or 0 -> 2 (scalar)
	   A VFP FMAC-pipeline instruction has been seen. Fill
	   regs[0]..regs[numregs-1] with its input operands. Remember this
	   instruction in 'first_fmac'.

       1 -> 2
	   Any instruction, except for a VFP instruction which overwrites
	   regs[*].

       1 -> 3 [ -> 0 ]  or
       2 -> 3 [ -> 0 ]
	   A VFP instruction has been seen which overwrites any of regs[*].
	   We must make a veneer!  Reset state to 0 before examining next
	   instruction.

       2 -> 0
	   If we fail to match anything in state 2, reset to state 0 and reset
	   the instruction pointer to the instruction after 'first_fmac'.

     If the VFP11 vector mode is in use, there must be at least two unrelated
     instructions between anti-dependent VFP11 instructions to properly avoid
     triggering the erratum, hence the use of the extra state 1.  */

  /* If we are only performing a partial link do not bother
     to construct any glue.  */
  if (bfd_link_relocatable (link_info))
    return true;

  /* Skip if this bfd does not correspond to an ELF image.  */
  if (! is_arm_elf (abfd))
    return true;

  /* We should have chosen a fix type by the time we get here.  */
  BFD_ASSERT (globals->vfp11_fix != BFD_ARM_VFP11_FIX_DEFAULT);

  if (globals->vfp11_fix == BFD_ARM_VFP11_FIX_NONE)
    return true;

  /* Skip this BFD if it corresponds to an executable or dynamic object.  */
  if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
    return true;

  for (sec = abfd->sections; sec != NULL; sec = sec->next)
    {
      unsigned int i, span, first_fmac = 0, veneer_of_insn = 0;
      struct _arm_elf_section_data *sec_data;

      /* If we don't have executable progbits, we're not interested in this
	 section.  Also skip if section is to be excluded.  */
      if (elf_section_type (sec) != SHT_PROGBITS
	  || (elf_section_flags (sec) & SHF_EXECINSTR) == 0
	  || (sec->flags & SEC_EXCLUDE) != 0
	  || sec->sec_info_type == SEC_INFO_TYPE_JUST_SYMS
	  || sec->output_section == bfd_abs_section_ptr
	  || strcmp (sec->name, VFP11_ERRATUM_VENEER_SECTION_NAME) == 0)
	continue;

      sec_data = elf32_arm_section_data (sec);

      if (sec_data->mapcount == 0)
	continue;

      if (elf_section_data (sec)->this_hdr.contents != NULL)
	contents = elf_section_data (sec)->this_hdr.contents;
      else if (! bfd_malloc_and_get_section (abfd, sec, &contents))
	goto error_return;

      qsort (sec_data->map, sec_data->mapcount, sizeof (elf32_arm_section_map),
	     elf32_arm_compare_mapping);

      for (span = 0; span < sec_data->mapcount; span++)
	{
	  unsigned int span_start = sec_data->map[span].vma;
	  unsigned int span_end = (span == sec_data->mapcount - 1)
				  ? sec->size : sec_data->map[span + 1].vma;
	  char span_type = sec_data->map[span].type;

	  /* FIXME: Only ARM mode is supported at present.  We may need to
	     support Thumb-2 mode also at some point.  */
	  if (span_type != 'a')
	    continue;

	  for (i = span_start; i < span_end;)
	    {
	      unsigned int next_i = i + 4;
	      unsigned int insn = bfd_big_endian (abfd)
		? (((unsigned) contents[i] << 24)
		   | (contents[i + 1] << 16)
		   | (contents[i + 2] << 8)
		   | contents[i + 3])
		: (((unsigned) contents[i + 3] << 24)
		   | (contents[i + 2] << 16)
		   | (contents[i + 1] << 8)
		   | contents[i]);
	      unsigned int writemask = 0;
	      enum bfd_arm_vfp11_pipe vpipe;

	      switch (state)
		{
		case 0:
		  vpipe = bfd_arm_vfp11_insn_decode (insn, &writemask, regs,
						    &numregs);
		  /* I'm assuming the VFP11 erratum can trigger with denorm
		     operands on either the FMAC or the DS pipeline. This might
		     lead to slightly overenthusiastic veneer insertion.  */
		  if (vpipe == VFP11_FMAC || vpipe == VFP11_DS)
		    {
		      state = use_vector ? 1 : 2;
		      first_fmac = i;
		      veneer_of_insn = insn;
		    }
		  break;

		case 1:
		  {
		    int other_regs[3], other_numregs;
		    vpipe = bfd_arm_vfp11_insn_decode (insn, &writemask,
						      other_regs,
						      &other_numregs);
		    if (vpipe != VFP11_BAD
			&& bfd_arm_vfp11_antidependency (writemask, regs,
							 numregs))
		      state = 3;
		    else
		      state = 2;
		  }
		  break;

		case 2:
		  {
		    int other_regs[3], other_numregs;
		    vpipe = bfd_arm_vfp11_insn_decode (insn, &writemask,
						      other_regs,
						      &other_numregs);
		    if (vpipe != VFP11_BAD
			&& bfd_arm_vfp11_antidependency (writemask, regs,
							 numregs))
		      state = 3;
		    else
		      {
			state = 0;
			next_i = first_fmac + 4;
		      }
		  }
		  break;

		case 3:
		  abort ();  /* Should be unreachable.  */
		}

	      if (state == 3)
		{
		  elf32_vfp11_erratum_list *newerr =(elf32_vfp11_erratum_list *)
		      bfd_zmalloc (sizeof (elf32_vfp11_erratum_list));

		  elf32_arm_section_data (sec)->erratumcount += 1;

		  newerr->u.b.vfp_insn = veneer_of_insn;

		  switch (span_type)
		    {
		    case 'a':
		      newerr->type = VFP11_ERRATUM_BRANCH_TO_ARM_VENEER;
		      break;

		    default:
		      abort ();
		    }

		  record_vfp11_erratum_veneer (link_info, newerr, abfd, sec,
					       first_fmac);

		  newerr->vma = -1;

		  newerr->next = sec_data->erratumlist;
		  sec_data->erratumlist = newerr;

		  state = 0;
		}

	      i = next_i;
	    }
	}

      if (elf_section_data (sec)->this_hdr.contents != contents)
	free (contents);
      contents = NULL;
    }

  return true;

 error_return:
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);

  return false;
}

/* Find virtual-memory addresses for VFP11 erratum veneers and return locations
   after sections have been laid out, using specially-named symbols.  */

void
bfd_elf32_arm_vfp11_fix_veneer_locations (bfd *abfd,
					  struct bfd_link_info *link_info)
{
  asection *sec;
  struct elf32_arm_link_hash_table *globals;
  char *tmp_name;

  if (bfd_link_relocatable (link_info))
    return;

  /* Skip if this bfd does not correspond to an ELF image.  */
  if (! is_arm_elf (abfd))
    return;

  globals = elf32_arm_hash_table (link_info);
  if (globals == NULL)
    return;

  tmp_name = (char *) bfd_malloc ((bfd_size_type) strlen
				  (VFP11_ERRATUM_VENEER_ENTRY_NAME) + 10);
  BFD_ASSERT (tmp_name);

  for (sec = abfd->sections; sec != NULL; sec = sec->next)
    {
      struct _arm_elf_section_data *sec_data = elf32_arm_section_data (sec);
      elf32_vfp11_erratum_list *errnode = sec_data->erratumlist;

      for (; errnode != NULL; errnode = errnode->next)
	{
	  struct elf_link_hash_entry *myh;
	  bfd_vma vma;

	  switch (errnode->type)
	    {
	    case VFP11_ERRATUM_BRANCH_TO_ARM_VENEER:
	    case VFP11_ERRATUM_BRANCH_TO_THUMB_VENEER:
	      /* Find veneer symbol.  */
	      sprintf (tmp_name, VFP11_ERRATUM_VENEER_ENTRY_NAME,
		       errnode->u.b.veneer->u.v.id);

	      myh = elf_link_hash_lookup
		(&(globals)->root, tmp_name, false, false, true);

	      if (myh == NULL)
		_bfd_error_handler (_("%pB: unable to find %s veneer `%s'"),
				    abfd, "VFP11", tmp_name);

	      vma = myh->root.u.def.section->output_section->vma
		    + myh->root.u.def.section->output_offset
		    + myh->root.u.def.value;

	      errnode->u.b.veneer->vma = vma;
	      break;

	    case VFP11_ERRATUM_ARM_VENEER:
	    case VFP11_ERRATUM_THUMB_VENEER:
	      /* Find return location.  */
	      sprintf (tmp_name, VFP11_ERRATUM_VENEER_ENTRY_NAME "_r",
		       errnode->u.v.id);

	      myh = elf_link_hash_lookup
		(&(globals)->root, tmp_name, false, false, true);

	      if (myh == NULL)
		_bfd_error_handler (_("%pB: unable to find %s veneer `%s'"),
				    abfd, "VFP11", tmp_name);

	      vma = myh->root.u.def.section->output_section->vma
		    + myh->root.u.def.section->output_offset
		    + myh->root.u.def.value;

	      errnode->u.v.branch->vma = vma;
	      break;

	    default:
	      abort ();
	    }
	}
    }

  free (tmp_name);
}

/* Find virtual-memory addresses for STM32L4XX erratum veneers and
   return locations after sections have been laid out, using
   specially-named symbols.  */

void
bfd_elf32_arm_stm32l4xx_fix_veneer_locations (bfd *abfd,
					      struct bfd_link_info *link_info)
{
  asection *sec;
  struct elf32_arm_link_hash_table *globals;
  char *tmp_name;

  if (bfd_link_relocatable (link_info))
    return;

  /* Skip if this bfd does not correspond to an ELF image.  */
  if (! is_arm_elf (abfd))
    return;

  globals = elf32_arm_hash_table (link_info);
  if (globals == NULL)
    return;

  tmp_name = (char *) bfd_malloc ((bfd_size_type) strlen
				  (STM32L4XX_ERRATUM_VENEER_ENTRY_NAME) + 10);
  BFD_ASSERT (tmp_name);

  for (sec = abfd->sections; sec != NULL; sec = sec->next)
    {
      struct _arm_elf_section_data *sec_data = elf32_arm_section_data (sec);
      elf32_stm32l4xx_erratum_list *errnode = sec_data->stm32l4xx_erratumlist;

      for (; errnode != NULL; errnode = errnode->next)
	{
	  struct elf_link_hash_entry *myh;
	  bfd_vma vma;

	  switch (errnode->type)
	    {
	    case STM32L4XX_ERRATUM_BRANCH_TO_VENEER:
	      /* Find veneer symbol.  */
	      sprintf (tmp_name, STM32L4XX_ERRATUM_VENEER_ENTRY_NAME,
		       errnode->u.b.veneer->u.v.id);

	      myh = elf_link_hash_lookup
		(&(globals)->root, tmp_name, false, false, true);

	      if (myh == NULL)
		_bfd_error_handler (_("%pB: unable to find %s veneer `%s'"),
				    abfd, "STM32L4XX", tmp_name);

	      vma = myh->root.u.def.section->output_section->vma
		+ myh->root.u.def.section->output_offset
		+ myh->root.u.def.value;

	      errnode->u.b.veneer->vma = vma;
	      break;

	    case STM32L4XX_ERRATUM_VENEER:
	      /* Find return location.  */
	      sprintf (tmp_name, STM32L4XX_ERRATUM_VENEER_ENTRY_NAME "_r",
		       errnode->u.v.id);

	      myh = elf_link_hash_lookup
		(&(globals)->root, tmp_name, false, false, true);

	      if (myh == NULL)
		_bfd_error_handler (_("%pB: unable to find %s veneer `%s'"),
				    abfd, "STM32L4XX", tmp_name);

	      vma = myh->root.u.def.section->output_section->vma
		+ myh->root.u.def.section->output_offset
		+ myh->root.u.def.value;

	      errnode->u.v.branch->vma = vma;
	      break;

	    default:
	      abort ();
	    }
	}
    }

  free (tmp_name);
}

static inline bool
is_thumb2_ldmia (const insn32 insn)
{
  /* Encoding T2: LDM<c>.W <Rn>{!},<registers>
     1110 - 1000 - 10W1 - rrrr - PM (0) l - llll - llll - llll.  */
  return (insn & 0xffd02000) == 0xe8900000;
}

static inline bool
is_thumb2_ldmdb (const insn32 insn)
{
  /* Encoding T1: LDMDB<c> <Rn>{!},<registers>
     1110 - 1001 - 00W1 - rrrr - PM (0) l - llll - llll - llll.  */
  return (insn & 0xffd02000) == 0xe9100000;
}

static inline bool
is_thumb2_vldm (const insn32 insn)
{
  /* A6.5 Extension register load or store instruction
     A7.7.229
     We look for SP 32-bit and DP 64-bit registers.
     Encoding T1 VLDM{mode}<c> <Rn>{!}, <list>
     <list> is consecutive 64-bit registers
     1110 - 110P - UDW1 - rrrr - vvvv - 1011 - iiii - iiii
     Encoding T2 VLDM{mode}<c> <Rn>{!}, <list>
     <list> is consecutive 32-bit registers
     1110 - 110P - UDW1 - rrrr - vvvv - 1010 - iiii - iiii
     if P==0 && U==1 && W==1 && Rn=1101 VPOP
     if PUW=010 || PUW=011 || PUW=101 VLDM.  */
  return
    (((insn & 0xfe100f00) == 0xec100b00) ||
     ((insn & 0xfe100f00) == 0xec100a00))
    && /* (IA without !).  */
    (((((insn << 7) >> 28) & 0xd) == 0x4)
     /* (IA with !), includes VPOP (when reg number is SP).  */
     || ((((insn << 7) >> 28) & 0xd) == 0x5)
     /* (DB with !).  */
     || ((((insn << 7) >> 28) & 0xd) == 0x9));
}

/* STM STM32L4XX erratum : This function assumes that it receives an LDM or
   VLDM opcode and:
 - computes the number and the mode of memory accesses
 - decides if the replacement should be done:
   . replaces only if > 8-word accesses
   . or (testing purposes only) replaces all accesses.  */

static bool
stm32l4xx_need_create_replacing_stub (const insn32 insn,
				      bfd_arm_stm32l4xx_fix stm32l4xx_fix)
{
  int nb_words = 0;

  /* The field encoding the register list is the same for both LDMIA
     and LDMDB encodings.  */
  if (is_thumb2_ldmia (insn) || is_thumb2_ldmdb (insn))
    nb_words = elf32_arm_popcount (insn & 0x0000ffff);
  else if (is_thumb2_vldm (insn))
   nb_words = (insn & 0xff);

  /* DEFAULT mode accounts for the real bug condition situation,
     ALL mode inserts stubs for each LDM/VLDM instruction (testing).  */
  return (stm32l4xx_fix == BFD_ARM_STM32L4XX_FIX_DEFAULT
	  ? nb_words > 8
	  : stm32l4xx_fix == BFD_ARM_STM32L4XX_FIX_ALL);
}

/* Look for potentially-troublesome code sequences which might trigger
   the STM STM32L4XX erratum.  */

bool
bfd_elf32_arm_stm32l4xx_erratum_scan (bfd *abfd,
				      struct bfd_link_info *link_info)
{
  asection *sec;
  bfd_byte *contents = NULL;
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (link_info);

  if (globals == NULL)
    return false;

  /* If we are only performing a partial link do not bother
     to construct any glue.  */
  if (bfd_link_relocatable (link_info))
    return true;

  /* Skip if this bfd does not correspond to an ELF image.  */
  if (! is_arm_elf (abfd))
    return true;

  if (globals->stm32l4xx_fix == BFD_ARM_STM32L4XX_FIX_NONE)
    return true;

  /* Skip this BFD if it corresponds to an executable or dynamic object.  */
  if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
    return true;

  for (sec = abfd->sections; sec != NULL; sec = sec->next)
    {
      unsigned int i, span;
      struct _arm_elf_section_data *sec_data;

      /* If we don't have executable progbits, we're not interested in this
	 section.  Also skip if section is to be excluded.  */
      if (elf_section_type (sec) != SHT_PROGBITS
	  || (elf_section_flags (sec) & SHF_EXECINSTR) == 0
	  || (sec->flags & SEC_EXCLUDE) != 0
	  || sec->sec_info_type == SEC_INFO_TYPE_JUST_SYMS
	  || sec->output_section == bfd_abs_section_ptr
	  || strcmp (sec->name, STM32L4XX_ERRATUM_VENEER_SECTION_NAME) == 0)
	continue;

      sec_data = elf32_arm_section_data (sec);

      if (sec_data->mapcount == 0)
	continue;

      if (elf_section_data (sec)->this_hdr.contents != NULL)
	contents = elf_section_data (sec)->this_hdr.contents;
      else if (! bfd_malloc_and_get_section (abfd, sec, &contents))
	goto error_return;

      qsort (sec_data->map, sec_data->mapcount, sizeof (elf32_arm_section_map),
	     elf32_arm_compare_mapping);

      for (span = 0; span < sec_data->mapcount; span++)
	{
	  unsigned int span_start = sec_data->map[span].vma;
	  unsigned int span_end = (span == sec_data->mapcount - 1)
	    ? sec->size : sec_data->map[span + 1].vma;
	  char span_type = sec_data->map[span].type;
	  int itblock_current_pos = 0;

	  /* Only Thumb2 mode need be supported with this CM4 specific
	     code, we should not encounter any arm mode eg span_type
	     != 'a'.  */
	  if (span_type != 't')
	    continue;

	  for (i = span_start; i < span_end;)
	    {
	      unsigned int insn = bfd_get_16 (abfd, &contents[i]);
	      bool insn_32bit = false;
	      bool is_ldm = false;
	      bool is_vldm = false;
	      bool is_not_last_in_it_block = false;

	      /* The first 16-bits of all 32-bit thumb2 instructions start
		 with opcode[15..13]=0b111 and the encoded op1 can be anything
		 except opcode[12..11]!=0b00.
		 See 32-bit Thumb instruction encoding.  */
	      if ((insn & 0xe000) == 0xe000 && (insn & 0x1800) != 0x0000)
		insn_32bit = true;

	      /* Compute the predicate that tells if the instruction
		 is concerned by the IT block
		 - Creates an error if there is a ldm that is not
		   last in the IT block thus cannot be replaced
		 - Otherwise we can create a branch at the end of the
		   IT block, it will be controlled naturally by IT
		   with the proper pseudo-predicate
		 - So the only interesting predicate is the one that
		   tells that we are not on the last item of an IT
		   block.  */
	      if (itblock_current_pos != 0)
		  is_not_last_in_it_block = !!--itblock_current_pos;

	      if (insn_32bit)
		{
		  /* Load the rest of the insn (in manual-friendly order).  */
		  insn = (insn << 16) | bfd_get_16 (abfd, &contents[i + 2]);
		  is_ldm = is_thumb2_ldmia (insn) || is_thumb2_ldmdb (insn);
		  is_vldm = is_thumb2_vldm (insn);

		  /* Veneers are created for (v)ldm depending on
		     option flags and memory accesses conditions; but
		     if the instruction is not the last instruction of
		     an IT block, we cannot create a jump there, so we
		     bail out.  */
		    if ((is_ldm || is_vldm)
			&& stm32l4xx_need_create_replacing_stub
			(insn, globals->stm32l4xx_fix))
		      {
			if (is_not_last_in_it_block)
			  {
			    _bfd_error_handler
			      /* xgettext:c-format */
			      (_("%pB(%pA+%#x): error: multiple load detected"
				 " in non-last IT block instruction:"
				 " STM32L4XX veneer cannot be generated; "
				 "use gcc option -mrestrict-it to generate"
				 " only one instruction per IT block"),
			       abfd, sec, i);
			  }
			else
			  {
			    elf32_stm32l4xx_erratum_list *newerr =
			      (elf32_stm32l4xx_erratum_list *)
			      bfd_zmalloc
			      (sizeof (elf32_stm32l4xx_erratum_list));

			    elf32_arm_section_data (sec)
			      ->stm32l4xx_erratumcount += 1;
			    newerr->u.b.insn = insn;
			    /* We create only thumb branches.  */
			    newerr->type =
			      STM32L4XX_ERRATUM_BRANCH_TO_VENEER;
			    record_stm32l4xx_erratum_veneer
			      (link_info, newerr, abfd, sec,
			       i,
			       is_ldm ?
			       STM32L4XX_ERRATUM_LDM_VENEER_SIZE:
			       STM32L4XX_ERRATUM_VLDM_VENEER_SIZE);
			    newerr->vma = -1;
			    newerr->next = sec_data->stm32l4xx_erratumlist;
			    sec_data->stm32l4xx_erratumlist = newerr;
			  }
		      }
		}
	      else
		{
		  /* A7.7.37 IT p208
		     IT blocks are only encoded in T1
		     Encoding T1: IT{x{y{z}}} <firstcond>
		     1 0 1 1 - 1 1 1 1 - firstcond - mask
		     if mask = '0000' then see 'related encodings'
		     We don't deal with UNPREDICTABLE, just ignore these.
		     There can be no nested IT blocks so an IT block
		     is naturally a new one for which it is worth
		     computing its size.  */
		  bool is_newitblock = ((insn & 0xff00) == 0xbf00)
		    && ((insn & 0x000f) != 0x0000);
		  /* If we have a new IT block we compute its size.  */
		  if (is_newitblock)
		    {
		      /* Compute the number of instructions controlled
			 by the IT block, it will be used to decide
			 whether we are inside an IT block or not.  */
		      unsigned int mask = insn & 0x000f;
		      itblock_current_pos = 4 - ctz (mask);
		    }
		}

	      i += insn_32bit ? 4 : 2;
	    }
	}

      if (elf_section_data (sec)->this_hdr.contents != contents)
	free (contents);
      contents = NULL;
    }

  return true;

 error_return:
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);

  return false;
}

/* Set target relocation values needed during linking.  */

void
bfd_elf32_arm_set_target_params (struct bfd *output_bfd,
				 struct bfd_link_info *link_info,
				 struct elf32_arm_params *params)
{
  struct elf32_arm_link_hash_table *globals;

  globals = elf32_arm_hash_table (link_info);
  if (globals == NULL)
    return;

  globals->target1_is_rel = params->target1_is_rel;
  if (globals->fdpic_p)
    globals->target2_reloc = R_ARM_GOT32;
  else if (strcmp (params->target2_type, "rel") == 0)
    globals->target2_reloc = R_ARM_REL32;
  else if (strcmp (params->target2_type, "abs") == 0)
    globals->target2_reloc = R_ARM_ABS32;
  else if (strcmp (params->target2_type, "got-rel") == 0)
    globals->target2_reloc = R_ARM_GOT_PREL;
  else
    {
      _bfd_error_handler (_("invalid TARGET2 relocation type '%s'"),
			  params->target2_type);
    }
  globals->fix_v4bx = params->fix_v4bx;
  globals->use_blx |= params->use_blx;
  globals->vfp11_fix = params->vfp11_denorm_fix;
  globals->stm32l4xx_fix = params->stm32l4xx_fix;
  if (globals->fdpic_p)
    globals->pic_veneer = 1;
  else
    globals->pic_veneer = params->pic_veneer;
  globals->fix_cortex_a8 = params->fix_cortex_a8;
  globals->fix_arm1176 = params->fix_arm1176;
  globals->cmse_implib = params->cmse_implib;
  globals->in_implib_bfd = params->in_implib_bfd;

  BFD_ASSERT (is_arm_elf (output_bfd));
  elf_arm_tdata (output_bfd)->no_enum_size_warning
    = params->no_enum_size_warning;
  elf_arm_tdata (output_bfd)->no_wchar_size_warning
    = params->no_wchar_size_warning;
}

/* Replace the target offset of a Thumb bl or b.w instruction.  */

static void
insert_thumb_branch (bfd *abfd, long int offset, bfd_byte *insn)
{
  bfd_vma upper;
  bfd_vma lower;
  int reloc_sign;

  BFD_ASSERT ((offset & 1) == 0);

  upper = bfd_get_16 (abfd, insn);
  lower = bfd_get_16 (abfd, insn + 2);
  reloc_sign = (offset < 0) ? 1 : 0;
  upper = (upper & ~(bfd_vma) 0x7ff)
	  | ((offset >> 12) & 0x3ff)
	  | (reloc_sign << 10);
  lower = (lower & ~(bfd_vma) 0x2fff)
	  | (((!((offset >> 23) & 1)) ^ reloc_sign) << 13)
	  | (((!((offset >> 22) & 1)) ^ reloc_sign) << 11)
	  | ((offset >> 1) & 0x7ff);
  bfd_put_16 (abfd, upper, insn);
  bfd_put_16 (abfd, lower, insn + 2);
}

/* Thumb code calling an ARM function.  */

static int
elf32_thumb_to_arm_stub (struct bfd_link_info * info,
			 const char *		name,
			 bfd *			input_bfd,
			 bfd *			output_bfd,
			 asection *		input_section,
			 bfd_byte *		hit_data,
			 asection *		sym_sec,
			 bfd_vma		offset,
			 bfd_signed_vma		addend,
			 bfd_vma		val,
			 char **error_message)
{
  asection * s = 0;
  bfd_vma my_offset;
  long int ret_offset;
  struct elf_link_hash_entry * myh;
  struct elf32_arm_link_hash_table * globals;

  myh = find_thumb_glue (info, name, error_message);
  if (myh == NULL)
    return false;

  globals = elf32_arm_hash_table (info);
  BFD_ASSERT (globals != NULL);
  BFD_ASSERT (globals->bfd_of_glue_owner != NULL);

  my_offset = myh->root.u.def.value;

  s = bfd_get_linker_section (globals->bfd_of_glue_owner,
			      THUMB2ARM_GLUE_SECTION_NAME);

  BFD_ASSERT (s != NULL);
  BFD_ASSERT (s->contents != NULL);
  BFD_ASSERT (s->output_section != NULL);

  if ((my_offset & 0x01) == 0x01)
    {
      if (sym_sec != NULL
	  && sym_sec->owner != NULL
	  && !INTERWORK_FLAG (sym_sec->owner))
	{
	  _bfd_error_handler
	    (_("%pB(%s): warning: interworking not enabled;"
	       " first occurrence: %pB: %s call to %s"),
	     sym_sec->owner, name, input_bfd, "Thumb", "ARM");

	  return false;
	}

      --my_offset;
      myh->root.u.def.value = my_offset;

      put_thumb_insn (globals, output_bfd, (bfd_vma) t2a1_bx_pc_insn,
		      s->contents + my_offset);

      put_thumb_insn (globals, output_bfd, (bfd_vma) t2a2_noop_insn,
		      s->contents + my_offset + 2);

      ret_offset =
	/* Address of destination of the stub.  */
	((bfd_signed_vma) val)
	- ((bfd_signed_vma)
	   /* Offset from the start of the current section
	      to the start of the stubs.  */
	   (s->output_offset
	    /* Offset of the start of this stub from the start of the stubs.  */
	    + my_offset
	    /* Address of the start of the current section.  */
	    + s->output_section->vma)
	   /* The branch instruction is 4 bytes into the stub.  */
	   + 4
	   /* ARM branches work from the pc of the instruction + 8.  */
	   + 8);

      put_arm_insn (globals, output_bfd,
		    (bfd_vma) t2a3_b_insn | ((ret_offset >> 2) & 0x00FFFFFF),
		    s->contents + my_offset + 4);
    }

  BFD_ASSERT (my_offset <= globals->thumb_glue_size);

  /* Now go back and fix up the original BL insn to point to here.  */
  ret_offset =
    /* Address of where the stub is located.  */
    (s->output_section->vma + s->output_offset + my_offset)
     /* Address of where the BL is located.  */
    - (input_section->output_section->vma + input_section->output_offset
       + offset)
    /* Addend in the relocation.  */
    - addend
    /* Biassing for PC-relative addressing.  */
    - 8;

  insert_thumb_branch (input_bfd, ret_offset, hit_data - input_section->vma);

  return true;
}

/* Populate an Arm to Thumb stub.  Returns the stub symbol.  */

static struct elf_link_hash_entry *
elf32_arm_create_thumb_stub (struct bfd_link_info * info,
			     const char *	    name,
			     bfd *		    input_bfd,
			     bfd *		    output_bfd,
			     asection *		    sym_sec,
			     bfd_vma		    val,
			     asection *		    s,
			     char **		    error_message)
{
  bfd_vma my_offset;
  long int ret_offset;
  struct elf_link_hash_entry * myh;
  struct elf32_arm_link_hash_table * globals;

  myh = find_arm_glue (info, name, error_message);
  if (myh == NULL)
    return NULL;

  globals = elf32_arm_hash_table (info);
  BFD_ASSERT (globals != NULL);
  BFD_ASSERT (globals->bfd_of_glue_owner != NULL);

  my_offset = myh->root.u.def.value;

  if ((my_offset & 0x01) == 0x01)
    {
      if (sym_sec != NULL
	  && sym_sec->owner != NULL
	  && !INTERWORK_FLAG (sym_sec->owner))
	{
	  _bfd_error_handler
	    (_("%pB(%s): warning: interworking not enabled;"
	       " first occurrence: %pB: %s call to %s"),
	     sym_sec->owner, name, input_bfd, "ARM", "Thumb");
	}

      --my_offset;
      myh->root.u.def.value = my_offset;

      if (bfd_link_pic (info)
	  || globals->root.is_relocatable_executable
	  || globals->pic_veneer)
	{
	  /* For relocatable objects we can't use absolute addresses,
	     so construct the address from a relative offset.  */
	  /* TODO: If the offset is small it's probably worth
	     constructing the address with adds.  */
	  put_arm_insn (globals, output_bfd, (bfd_vma) a2t1p_ldr_insn,
			s->contents + my_offset);
	  put_arm_insn (globals, output_bfd, (bfd_vma) a2t2p_add_pc_insn,
			s->contents + my_offset + 4);
	  put_arm_insn (globals, output_bfd, (bfd_vma) a2t3p_bx_r12_insn,
			s->contents + my_offset + 8);
	  /* Adjust the offset by 4 for the position of the add,
	     and 8 for the pipeline offset.  */
	  ret_offset = (val - (s->output_offset
			       + s->output_section->vma
			       + my_offset + 12))
		       | 1;
	  bfd_put_32 (output_bfd, ret_offset,
		      s->contents + my_offset + 12);
	}
      else if (globals->use_blx)
	{
	  put_arm_insn (globals, output_bfd, (bfd_vma) a2t1v5_ldr_insn,
			s->contents + my_offset);

	  /* It's a thumb address.  Add the low order bit.  */
	  bfd_put_32 (output_bfd, val | a2t2v5_func_addr_insn,
		      s->contents + my_offset + 4);
	}
      else
	{
	  put_arm_insn (globals, output_bfd, (bfd_vma) a2t1_ldr_insn,
			s->contents + my_offset);

	  put_arm_insn (globals, output_bfd, (bfd_vma) a2t2_bx_r12_insn,
			s->contents + my_offset + 4);

	  /* It's a thumb address.  Add the low order bit.  */
	  bfd_put_32 (output_bfd, val | a2t3_func_addr_insn,
		      s->contents + my_offset + 8);

	  my_offset += 12;
	}
    }

  BFD_ASSERT (my_offset <= globals->arm_glue_size);

  return myh;
}

/* Arm code calling a Thumb function.  */

static int
elf32_arm_to_thumb_stub (struct bfd_link_info * info,
			 const char *		name,
			 bfd *			input_bfd,
			 bfd *			output_bfd,
			 asection *		input_section,
			 bfd_byte *		hit_data,
			 asection *		sym_sec,
			 bfd_vma		offset,
			 bfd_signed_vma		addend,
			 bfd_vma		val,
			 char **error_message)
{
  unsigned long int tmp;
  bfd_vma my_offset;
  asection * s;
  long int ret_offset;
  struct elf_link_hash_entry * myh;
  struct elf32_arm_link_hash_table * globals;

  globals = elf32_arm_hash_table (info);
  BFD_ASSERT (globals != NULL);
  BFD_ASSERT (globals->bfd_of_glue_owner != NULL);

  s = bfd_get_linker_section (globals->bfd_of_glue_owner,
			      ARM2THUMB_GLUE_SECTION_NAME);
  BFD_ASSERT (s != NULL);
  BFD_ASSERT (s->contents != NULL);
  BFD_ASSERT (s->output_section != NULL);

  myh = elf32_arm_create_thumb_stub (info, name, input_bfd, output_bfd,
				     sym_sec, val, s, error_message);
  if (!myh)
    return false;

  my_offset = myh->root.u.def.value;
  tmp = bfd_get_32 (input_bfd, hit_data);
  tmp = tmp & 0xFF000000;

  /* Somehow these are both 4 too far, so subtract 8.  */
  ret_offset = (s->output_offset
		+ my_offset
		+ s->output_section->vma
		- (input_section->output_offset
		   + input_section->output_section->vma
		   + offset + addend)
		- 8);

  tmp = tmp | ((ret_offset >> 2) & 0x00FFFFFF);

  bfd_put_32 (output_bfd, (bfd_vma) tmp, hit_data - input_section->vma);

  return true;
}

/* Populate Arm stub for an exported Thumb function.  */

static bool
elf32_arm_to_thumb_export_stub (struct elf_link_hash_entry *h, void * inf)
{
  struct bfd_link_info * info = (struct bfd_link_info *) inf;
  asection * s;
  struct elf_link_hash_entry * myh;
  struct elf32_arm_link_hash_entry *eh;
  struct elf32_arm_link_hash_table * globals;
  asection *sec;
  bfd_vma val;
  char *error_message;

  eh = elf32_arm_hash_entry (h);
  /* Allocate stubs for exported Thumb functions on v4t.  */
  if (eh->export_glue == NULL)
    return true;

  globals = elf32_arm_hash_table (info);
  BFD_ASSERT (globals != NULL);
  BFD_ASSERT (globals->bfd_of_glue_owner != NULL);

  s = bfd_get_linker_section (globals->bfd_of_glue_owner,
			      ARM2THUMB_GLUE_SECTION_NAME);
  BFD_ASSERT (s != NULL);
  BFD_ASSERT (s->contents != NULL);
  BFD_ASSERT (s->output_section != NULL);

  sec = eh->export_glue->root.u.def.section;

  BFD_ASSERT (sec->output_section != NULL);

  val = eh->export_glue->root.u.def.value + sec->output_offset
	+ sec->output_section->vma;

  myh = elf32_arm_create_thumb_stub (info, h->root.root.string,
				     h->root.u.def.section->owner,
				     globals->obfd, sec, val, s,
				     &error_message);
  BFD_ASSERT (myh);
  return true;
}

/* Populate ARMv4 BX veneers.  Returns the absolute adress of the veneer.  */

static bfd_vma
elf32_arm_bx_glue (struct bfd_link_info * info, int reg)
{
  bfd_byte *p;
  bfd_vma glue_addr;
  asection *s;
  struct elf32_arm_link_hash_table *globals;

  globals = elf32_arm_hash_table (info);
  BFD_ASSERT (globals != NULL);
  BFD_ASSERT (globals->bfd_of_glue_owner != NULL);

  s = bfd_get_linker_section (globals->bfd_of_glue_owner,
			      ARM_BX_GLUE_SECTION_NAME);
  BFD_ASSERT (s != NULL);
  BFD_ASSERT (s->contents != NULL);
  BFD_ASSERT (s->output_section != NULL);

  BFD_ASSERT (globals->bx_glue_offset[reg] & 2);

  glue_addr = globals->bx_glue_offset[reg] & ~(bfd_vma)3;

  if ((globals->bx_glue_offset[reg] & 1) == 0)
    {
      p = s->contents + glue_addr;
      bfd_put_32 (globals->obfd, armbx1_tst_insn + (reg << 16), p);
      bfd_put_32 (globals->obfd, armbx2_moveq_insn + reg, p + 4);
      bfd_put_32 (globals->obfd, armbx3_bx_insn + reg, p + 8);
      globals->bx_glue_offset[reg] |= 1;
    }

  return glue_addr + s->output_section->vma + s->output_offset;
}

/* Generate Arm stubs for exported Thumb symbols.  */
static void
elf32_arm_begin_write_processing (bfd *abfd ATTRIBUTE_UNUSED,
				  struct bfd_link_info *link_info)
{
  struct elf32_arm_link_hash_table * globals;

  if (link_info == NULL)
    /* Ignore this if we are not called by the ELF backend linker.  */
    return;

  globals = elf32_arm_hash_table (link_info);
  if (globals == NULL)
    return;

  /* If blx is available then exported Thumb symbols are OK and there is
     nothing to do.  */
  if (globals->use_blx)
    return;

  elf_link_hash_traverse (&globals->root, elf32_arm_to_thumb_export_stub,
			  link_info);
}

/* Reserve space for COUNT dynamic relocations in relocation selection
   SRELOC.  */

static void
elf32_arm_allocate_dynrelocs (struct bfd_link_info *info, asection *sreloc,
			      bfd_size_type count)
{
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  BFD_ASSERT (htab->root.dynamic_sections_created);
  if (sreloc == NULL)
    abort ();
  sreloc->size += RELOC_SIZE (htab) * count;
}

/* Reserve space for COUNT R_ARM_IRELATIVE relocations.  If the link is
   dynamic, the relocations should go in SRELOC, otherwise they should
   go in the special .rel.iplt section.  */

static void
elf32_arm_allocate_irelocs (struct bfd_link_info *info, asection *sreloc,
			    bfd_size_type count)
{
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  if (!htab->root.dynamic_sections_created)
    htab->root.irelplt->size += RELOC_SIZE (htab) * count;
  else
    {
      BFD_ASSERT (sreloc != NULL);
      sreloc->size += RELOC_SIZE (htab) * count;
    }
}

/* Add relocation REL to the end of relocation section SRELOC.  */

static void
elf32_arm_add_dynreloc (bfd *output_bfd, struct bfd_link_info *info,
			asection *sreloc, Elf_Internal_Rela *rel)
{
  bfd_byte *loc;
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  if (!htab->root.dynamic_sections_created
      && ELF32_R_TYPE (rel->r_info) == R_ARM_IRELATIVE)
    sreloc = htab->root.irelplt;
  if (sreloc == NULL)
    abort ();
  loc = sreloc->contents;
  loc += sreloc->reloc_count++ * RELOC_SIZE (htab);
  if (sreloc->reloc_count * RELOC_SIZE (htab) > sreloc->size)
    abort ();
  SWAP_RELOC_OUT (htab) (output_bfd, rel, loc);
}

/* Allocate room for a PLT entry described by ROOT_PLT and ARM_PLT.
   IS_IPLT_ENTRY says whether the entry belongs to .iplt rather than
   to .plt.  */

static void
elf32_arm_allocate_plt_entry (struct bfd_link_info *info,
			      bool is_iplt_entry,
			      union gotplt_union *root_plt,
			      struct arm_plt_info *arm_plt)
{
  struct elf32_arm_link_hash_table *htab;
  asection *splt;
  asection *sgotplt;

  htab = elf32_arm_hash_table (info);

  if (is_iplt_entry)
    {
      splt = htab->root.iplt;
      sgotplt = htab->root.igotplt;

      /* NaCl uses a special first entry in .iplt too.  */
      if (htab->root.target_os == is_nacl && splt->size == 0)
	splt->size += htab->plt_header_size;

      /* Allocate room for an R_ARM_IRELATIVE relocation in .rel.iplt.  */
      elf32_arm_allocate_irelocs (info, htab->root.irelplt, 1);
    }
  else
    {
      splt = htab->root.splt;
      sgotplt = htab->root.sgotplt;

    if (htab->fdpic_p)
      {
	/* Allocate room for R_ARM_FUNCDESC_VALUE.  */
	/* For lazy binding, relocations will be put into .rel.plt, in
	   .rel.got otherwise.  */
	/* FIXME: today we don't support lazy binding so put it in .rel.got */
	if (info->flags & DF_BIND_NOW)
	  elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
	else
	  elf32_arm_allocate_dynrelocs (info, htab->root.srelplt, 1);
      }
    else
      {
	/* Allocate room for an R_JUMP_SLOT relocation in .rel.plt.  */
	elf32_arm_allocate_dynrelocs (info, htab->root.srelplt, 1);
      }

      /* If this is the first .plt entry, make room for the special
	 first entry.  */
      if (splt->size == 0)
	splt->size += htab->plt_header_size;

      htab->next_tls_desc_index++;
    }

  /* Allocate the PLT entry itself, including any leading Thumb stub.  */
  if (elf32_arm_plt_needs_thumb_stub_p (info, arm_plt))
    splt->size += PLT_THUMB_STUB_SIZE;
  root_plt->offset = splt->size;
  splt->size += htab->plt_entry_size;

  /* We also need to make an entry in the .got.plt section, which
     will be placed in the .got section by the linker script.  */
  if (is_iplt_entry)
    arm_plt->got_offset = sgotplt->size;
  else
    arm_plt->got_offset = sgotplt->size - 8 * htab->num_tls_desc;
  if (htab->fdpic_p)
    /* Function descriptor takes 64 bits in GOT.  */
    sgotplt->size += 8;
  else
    sgotplt->size += 4;
}

static bfd_vma
arm_movw_immediate (bfd_vma value)
{
  return (value & 0x00000fff) | ((value & 0x0000f000) << 4);
}

static bfd_vma
arm_movt_immediate (bfd_vma value)
{
  return ((value & 0x0fff0000) >> 16) | ((value & 0xf0000000) >> 12);
}

/* Fill in a PLT entry and its associated GOT slot.  If DYNINDX == -1,
   the entry lives in .iplt and resolves to (*SYM_VALUE)().
   Otherwise, DYNINDX is the index of the symbol in the dynamic
   symbol table and SYM_VALUE is undefined.

   ROOT_PLT points to the offset of the PLT entry from the start of its
   section (.iplt or .plt).  ARM_PLT points to the symbol's ARM-specific
   bookkeeping information.

   Returns FALSE if there was a problem.  */

static bool
elf32_arm_populate_plt_entry (bfd *output_bfd, struct bfd_link_info *info,
			      union gotplt_union *root_plt,
			      struct arm_plt_info *arm_plt,
			      int dynindx, bfd_vma sym_value)
{
  struct elf32_arm_link_hash_table *htab;
  asection *sgot;
  asection *splt;
  asection *srel;
  bfd_byte *loc;
  bfd_vma plt_index;
  Elf_Internal_Rela rel;
  bfd_vma got_header_size;

  htab = elf32_arm_hash_table (info);

  /* Pick the appropriate sections and sizes.  */
  if (dynindx == -1)
    {
      splt = htab->root.iplt;
      sgot = htab->root.igotplt;
      srel = htab->root.irelplt;

      /* There are no reserved entries in .igot.plt, and no special
	 first entry in .iplt.  */
      got_header_size = 0;
    }
  else
    {
      splt = htab->root.splt;
      sgot = htab->root.sgotplt;
      srel = htab->root.srelplt;

      got_header_size = get_elf_backend_data (output_bfd)->got_header_size;
    }
  BFD_ASSERT (splt != NULL && srel != NULL);

  bfd_vma got_offset, got_address, plt_address;
  bfd_vma got_displacement, initial_got_entry;
  bfd_byte * ptr;

  BFD_ASSERT (sgot != NULL);

  /* Get the offset into the .(i)got.plt table of the entry that
     corresponds to this function.  */
  got_offset = (arm_plt->got_offset & -2);

  /* Get the index in the procedure linkage table which
     corresponds to this symbol.  This is the index of this symbol
     in all the symbols for which we are making plt entries.
     After the reserved .got.plt entries, all symbols appear in
     the same order as in .plt.  */
  if (htab->fdpic_p)
    /* Function descriptor takes 8 bytes.  */
    plt_index = (got_offset - got_header_size) / 8;
  else
    plt_index = (got_offset - got_header_size) / 4;

  /* Calculate the address of the GOT entry.  */
  got_address = (sgot->output_section->vma
		 + sgot->output_offset
		 + got_offset);

  /* ...and the address of the PLT entry.  */
  plt_address = (splt->output_section->vma
		 + splt->output_offset
		 + root_plt->offset);

  ptr = splt->contents + root_plt->offset;
  if (htab->root.target_os == is_vxworks && bfd_link_pic (info))
    {
      unsigned int i;
      bfd_vma val;

      for (i = 0; i != htab->plt_entry_size / 4; i++, ptr += 4)
	{
	  val = elf32_arm_vxworks_shared_plt_entry[i];
	  if (i == 2)
	    val |= got_address - sgot->output_section->vma;
	  if (i == 5)
	    val |= plt_index * RELOC_SIZE (htab);
	  if (i == 2 || i == 5)
	    bfd_put_32 (output_bfd, val, ptr);
	  else
	    put_arm_insn (htab, output_bfd, val, ptr);
	}
    }
  else if (htab->root.target_os == is_vxworks)
    {
      unsigned int i;
      bfd_vma val;

      for (i = 0; i != htab->plt_entry_size / 4; i++, ptr += 4)
	{
	  val = elf32_arm_vxworks_exec_plt_entry[i];
	  if (i == 2)
	    val |= got_address;
	  if (i == 4)
	    val |= 0xffffff & -((root_plt->offset + i * 4 + 8) >> 2);
	  if (i == 5)
	    val |= plt_index * RELOC_SIZE (htab);
	  if (i == 2 || i == 5)
	    bfd_put_32 (output_bfd, val, ptr);
	  else
	    put_arm_insn (htab, output_bfd, val, ptr);
	}

      loc = (htab->srelplt2->contents
	     + (plt_index * 2 + 1) * RELOC_SIZE (htab));

      /* Create the .rela.plt.unloaded R_ARM_ABS32 relocation
	 referencing the GOT for this PLT entry.  */
      rel.r_offset = plt_address + 8;
      rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_ARM_ABS32);
      rel.r_addend = got_offset;
      SWAP_RELOC_OUT (htab) (output_bfd, &rel, loc);
      loc += RELOC_SIZE (htab);

      /* Create the R_ARM_ABS32 relocation referencing the
	 beginning of the PLT for this GOT entry.  */
      rel.r_offset = got_address;
      rel.r_info = ELF32_R_INFO (htab->root.hplt->indx, R_ARM_ABS32);
      rel.r_addend = 0;
      SWAP_RELOC_OUT (htab) (output_bfd, &rel, loc);
    }
  else if (htab->root.target_os == is_nacl)
    {
      /* Calculate the displacement between the PLT slot and the
	 common tail that's part of the special initial PLT slot.  */
      int32_t tail_displacement
	= ((splt->output_section->vma + splt->output_offset
	    + ARM_NACL_PLT_TAIL_OFFSET)
	   - (plt_address + htab->plt_entry_size + 4));
      BFD_ASSERT ((tail_displacement & 3) == 0);
      tail_displacement >>= 2;

      BFD_ASSERT ((tail_displacement & 0xff000000) == 0
		  || (-tail_displacement & 0xff000000) == 0);

      /* Calculate the displacement between the PLT slot and the entry
	 in the GOT.  The offset accounts for the value produced by
	 adding to pc in the penultimate instruction of the PLT stub.  */
      got_displacement = (got_address
			  - (plt_address + htab->plt_entry_size));

      /* NaCl does not support interworking at all.  */
      BFD_ASSERT (!elf32_arm_plt_needs_thumb_stub_p (info, arm_plt));

      put_arm_insn (htab, output_bfd,
		    elf32_arm_nacl_plt_entry[0]
		    | arm_movw_immediate (got_displacement),
		    ptr + 0);
      put_arm_insn (htab, output_bfd,
		    elf32_arm_nacl_plt_entry[1]
		    | arm_movt_immediate (got_displacement),
		    ptr + 4);
      put_arm_insn (htab, output_bfd,
		    elf32_arm_nacl_plt_entry[2],
		    ptr + 8);
      put_arm_insn (htab, output_bfd,
		    elf32_arm_nacl_plt_entry[3]
		    | (tail_displacement & 0x00ffffff),
		    ptr + 12);
    }
  else if (htab->fdpic_p)
    {
      const bfd_vma *plt_entry = using_thumb_only (htab)
	? elf32_arm_fdpic_thumb_plt_entry
	: elf32_arm_fdpic_plt_entry;

      /* Fill-up Thumb stub if needed.  */
      if (elf32_arm_plt_needs_thumb_stub_p (info, arm_plt))
	{
	  put_thumb_insn (htab, output_bfd,
			  elf32_arm_plt_thumb_stub[0], ptr - 4);
	  put_thumb_insn (htab, output_bfd,
			  elf32_arm_plt_thumb_stub[1], ptr - 2);
	}
      /* As we are using 32 bit instructions even for the Thumb
	 version, we have to use 'put_arm_insn' instead of
	 'put_thumb_insn'.  */
      put_arm_insn (htab, output_bfd, plt_entry[0], ptr + 0);
      put_arm_insn (htab, output_bfd, plt_entry[1], ptr + 4);
      put_arm_insn (htab, output_bfd, plt_entry[2], ptr + 8);
      put_arm_insn (htab, output_bfd, plt_entry[3], ptr + 12);
      bfd_put_32 (output_bfd, got_offset, ptr + 16);

      if (!(info->flags & DF_BIND_NOW))
	{
	  /* funcdesc_value_reloc_offset.  */
	  bfd_put_32 (output_bfd,
		      htab->root.srelplt->reloc_count * RELOC_SIZE (htab),
		      ptr + 20);
	  put_arm_insn (htab, output_bfd, plt_entry[6], ptr + 24);
	  put_arm_insn (htab, output_bfd, plt_entry[7], ptr + 28);
	  put_arm_insn (htab, output_bfd, plt_entry[8], ptr + 32);
	  put_arm_insn (htab, output_bfd, plt_entry[9], ptr + 36);
	}
    }
  else if (using_thumb_only (htab))
    {
      /* PR ld/16017: Generate thumb only PLT entries.  */
      if (!using_thumb2 (htab))
	{
	  /* FIXME: We ought to be able to generate thumb-1 PLT
	     instructions...  */
	  _bfd_error_handler (_("%pB: warning: thumb-1 mode PLT generation not currently supported"),
			      output_bfd);
	  return false;
	}

      /* Calculate the displacement between the PLT slot and the entry in
	 the GOT.  The 12-byte offset accounts for the value produced by
	 adding to pc in the 3rd instruction of the PLT stub.  */
      got_displacement = got_address - (plt_address + 12);

      /* As we are using 32 bit instructions we have to use 'put_arm_insn'
	 instead of 'put_thumb_insn'.  */
      put_arm_insn (htab, output_bfd,
		    elf32_thumb2_plt_entry[0]
		    | ((got_displacement & 0x000000ff) << 16)
		    | ((got_displacement & 0x00000700) << 20)
		    | ((got_displacement & 0x00000800) >>  1)
		    | ((got_displacement & 0x0000f000) >> 12),
		    ptr + 0);
      put_arm_insn (htab, output_bfd,
		    elf32_thumb2_plt_entry[1]
		    | ((got_displacement & 0x00ff0000)      )
		    | ((got_displacement & 0x07000000) <<  4)
		    | ((got_displacement & 0x08000000) >> 17)
		    | ((got_displacement & 0xf0000000) >> 28),
		    ptr + 4);
      put_arm_insn (htab, output_bfd,
		    elf32_thumb2_plt_entry[2],
		    ptr + 8);
      put_arm_insn (htab, output_bfd,
		    elf32_thumb2_plt_entry[3],
		    ptr + 12);
    }
  else
    {
      /* Calculate the displacement between the PLT slot and the
	 entry in the GOT.  The eight-byte offset accounts for the
	 value produced by adding to pc in the first instruction
	 of the PLT stub.  */
      got_displacement = got_address - (plt_address + 8);

      if (elf32_arm_plt_needs_thumb_stub_p (info, arm_plt))
	{
	  put_thumb_insn (htab, output_bfd,
			  elf32_arm_plt_thumb_stub[0], ptr - 4);
	  put_thumb_insn (htab, output_bfd,
			  elf32_arm_plt_thumb_stub[1], ptr - 2);
	}

      if (!elf32_arm_use_long_plt_entry)
	{
	  BFD_ASSERT ((got_displacement & 0xf0000000) == 0);

	  put_arm_insn (htab, output_bfd,
			elf32_arm_plt_entry_short[0]
			| ((got_displacement & 0x0ff00000) >> 20),
			ptr + 0);
	  put_arm_insn (htab, output_bfd,
			elf32_arm_plt_entry_short[1]
			| ((got_displacement & 0x000ff000) >> 12),
			ptr+ 4);
	  put_arm_insn (htab, output_bfd,
			elf32_arm_plt_entry_short[2]
			| (got_displacement & 0x00000fff),
			ptr + 8);
#ifdef FOUR_WORD_PLT
	  bfd_put_32 (output_bfd, elf32_arm_plt_entry_short[3], ptr + 12);
#endif
	}
      else
	{
	  put_arm_insn (htab, output_bfd,
			elf32_arm_plt_entry_long[0]
			| ((got_displacement & 0xf0000000) >> 28),
			ptr + 0);
	  put_arm_insn (htab, output_bfd,
			elf32_arm_plt_entry_long[1]
			| ((got_displacement & 0x0ff00000) >> 20),
			ptr + 4);
	  put_arm_insn (htab, output_bfd,
			elf32_arm_plt_entry_long[2]
			| ((got_displacement & 0x000ff000) >> 12),
			ptr+ 8);
	  put_arm_insn (htab, output_bfd,
			elf32_arm_plt_entry_long[3]
			| (got_displacement & 0x00000fff),
			ptr + 12);
	}
    }

  /* Fill in the entry in the .rel(a).(i)plt section.  */
  rel.r_offset = got_address;
  rel.r_addend = 0;
  if (dynindx == -1)
    {
      /* .igot.plt entries use IRELATIVE relocations against SYM_VALUE.
	 The dynamic linker or static executable then calls SYM_VALUE
	 to determine the correct run-time value of the .igot.plt entry.  */
      rel.r_info = ELF32_R_INFO (0, R_ARM_IRELATIVE);
      initial_got_entry = sym_value;
    }
  else
    {
      /* For FDPIC we will have to resolve a R_ARM_FUNCDESC_VALUE
	 used by PLT entry.  */
      if (htab->fdpic_p)
	{
	  rel.r_info = ELF32_R_INFO (dynindx, R_ARM_FUNCDESC_VALUE);
	  initial_got_entry = 0;
	}
      else
	{
	  rel.r_info = ELF32_R_INFO (dynindx, R_ARM_JUMP_SLOT);
	  initial_got_entry = (splt->output_section->vma
			       + splt->output_offset);

	  /* PR ld/16017
	     When thumb only we need to set the LSB for any address that
	     will be used with an interworking branch instruction.  */
	  if (using_thumb_only (htab))
	    initial_got_entry |= 1;
	}
    }

  /* Fill in the entry in the global offset table.  */
  bfd_put_32 (output_bfd, initial_got_entry,
	      sgot->contents + got_offset);

  if (htab->fdpic_p && !(info->flags & DF_BIND_NOW))
    {
      /* Setup initial funcdesc value.  */
      /* FIXME: we don't support lazy binding because there is a
	 race condition between both words getting written and
	 some other thread attempting to read them. The ARM
	 architecture does not have an atomic 64 bit load/store
	 instruction that could be used to prevent it; it is
	 recommended that threaded FDPIC applications run with the
	 LD_BIND_NOW environment variable set.  */
      bfd_put_32 (output_bfd, plt_address + 0x18,
		  sgot->contents + got_offset);
      bfd_put_32 (output_bfd, -1 /*TODO*/,
		  sgot->contents + got_offset + 4);
    }

  if (dynindx == -1)
    elf32_arm_add_dynreloc (output_bfd, info, srel, &rel);
  else
    {
      if (htab->fdpic_p)
	{
	  /* For FDPIC we put PLT relocationss into .rel.got when not
	     lazy binding otherwise we put them in .rel.plt.  For now,
	     we don't support lazy binding so put it in .rel.got.  */
	  if (info->flags & DF_BIND_NOW)
	    elf32_arm_add_dynreloc (output_bfd, info, htab->root.srelgot, &rel);
	  else
	    elf32_arm_add_dynreloc (output_bfd, info, htab->root.srelplt, &rel);
	}
      else
	{
	  loc = srel->contents + plt_index * RELOC_SIZE (htab);
	  SWAP_RELOC_OUT (htab) (output_bfd, &rel, loc);
	}
    }

  return true;
}

/* Some relocations map to different relocations depending on the
   target.  Return the real relocation.  */

static int
arm_real_reloc_type (struct elf32_arm_link_hash_table * globals,
		     int r_type)
{
  switch (r_type)
    {
    case R_ARM_TARGET1:
      if (globals->target1_is_rel)
	return R_ARM_REL32;
      else
	return R_ARM_ABS32;

    case R_ARM_TARGET2:
      return globals->target2_reloc;

    default:
      return r_type;
    }
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
  bfd_vma base;

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;
  base = align_power ((bfd_vma) TCB_SIZE, htab->tls_sec->alignment_power);
  return address - htab->tls_sec->vma + base;
}

/* Perform an R_ARM_ABS12 relocation on the field pointed to by DATA.
   VALUE is the relocation value.  */

static bfd_reloc_status_type
elf32_arm_abs12_reloc (bfd *abfd, void *data, bfd_vma value)
{
  if (value > 0xfff)
    return bfd_reloc_overflow;

  value |= bfd_get_32 (abfd, data) & 0xfffff000;
  bfd_put_32 (abfd, value, data);
  return bfd_reloc_ok;
}

/* Handle TLS relaxations.  Relaxing is possible for symbols that use
   R_ARM_GOTDESC, R_ARM_{,THM_}TLS_CALL or
   R_ARM_{,THM_}TLS_DESCSEQ relocations, during a static link.

   Return bfd_reloc_ok if we're done, bfd_reloc_continue if the caller
   is to then call final_link_relocate.  Return other values in the
   case of error.

   FIXME:When --emit-relocs is in effect, we'll emit relocs describing
   the pre-relaxed code.  It would be nice if the relocs were updated
   to match the optimization.   */

static bfd_reloc_status_type
elf32_arm_tls_relax (struct elf32_arm_link_hash_table *globals,
		     bfd *input_bfd, asection *input_sec, bfd_byte *contents,
		     Elf_Internal_Rela *rel, unsigned long is_local)
{
  unsigned long insn;

  switch (ELF32_R_TYPE (rel->r_info))
    {
    default:
      return bfd_reloc_notsupported;

    case R_ARM_TLS_GOTDESC:
      if (is_local)
	insn = 0;
      else
	{
	  insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	  if (insn & 1)
	    insn -= 5; /* THUMB */
	  else
	    insn -= 8; /* ARM */
	}
      bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
      return bfd_reloc_continue;

    case R_ARM_THM_TLS_DESCSEQ:
      /* Thumb insn.  */
      insn = bfd_get_16 (input_bfd, contents + rel->r_offset);
      if ((insn & 0xff78) == 0x4478)	  /* add rx, pc */
	{
	  if (is_local)
	    /* nop */
	    bfd_put_16 (input_bfd, 0x46c0, contents + rel->r_offset);
	}
      else if ((insn & 0xffc0) == 0x6840)  /* ldr rx,[ry,#4] */
	{
	  if (is_local)
	    /* nop */
	    bfd_put_16 (input_bfd, 0x46c0, contents + rel->r_offset);
	  else
	    /* ldr rx,[ry] */
	    bfd_put_16 (input_bfd, insn & 0xf83f, contents + rel->r_offset);
	}
      else if ((insn & 0xff87) == 0x4780)  /* blx rx */
	{
	  if (is_local)
	    /* nop */
	    bfd_put_16 (input_bfd, 0x46c0, contents + rel->r_offset);
	  else
	    /* mov r0, rx */
	    bfd_put_16 (input_bfd, 0x4600 | (insn & 0x78),
			contents + rel->r_offset);
	}
      else
	{
	  if ((insn & 0xf000) == 0xf000 || (insn & 0xf800) == 0xe800)
	    /* It's a 32 bit instruction, fetch the rest of it for
	       error generation.  */
	    insn = (insn << 16)
	      | bfd_get_16 (input_bfd, contents + rel->r_offset + 2);
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB(%pA+%#" PRIx64 "): "
	       "unexpected %s instruction '%#lx' in TLS trampoline"),
	     input_bfd, input_sec, (uint64_t) rel->r_offset,
	     "Thumb", insn);
	  return bfd_reloc_notsupported;
	}
      break;

    case R_ARM_TLS_DESCSEQ:
      /* arm insn.  */
      insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
      if ((insn & 0xffff0ff0) == 0xe08f0000) /* add rx,pc,ry */
	{
	  if (is_local)
	    /* mov rx, ry */
	    bfd_put_32 (input_bfd, 0xe1a00000 | (insn & 0xffff),
			contents + rel->r_offset);
	}
      else if ((insn & 0xfff00fff) == 0xe5900004) /* ldr rx,[ry,#4]*/
	{
	  if (is_local)
	    /* nop */
	    bfd_put_32 (input_bfd, 0xe1a00000, contents + rel->r_offset);
	  else
	    /* ldr rx,[ry] */
	    bfd_put_32 (input_bfd, insn & 0xfffff000,
			contents + rel->r_offset);
	}
      else if ((insn & 0xfffffff0) == 0xe12fff30) /* blx rx */
	{
	  if (is_local)
	    /* nop */
	    bfd_put_32 (input_bfd, 0xe1a00000, contents + rel->r_offset);
	  else
	    /* mov r0, rx */
	    bfd_put_32 (input_bfd, 0xe1a00000 | (insn & 0xf),
			contents + rel->r_offset);
	}
      else
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB(%pA+%#" PRIx64 "): "
	       "unexpected %s instruction '%#lx' in TLS trampoline"),
	     input_bfd, input_sec, (uint64_t) rel->r_offset,
	     "ARM", insn);
	  return bfd_reloc_notsupported;
	}
      break;

    case R_ARM_TLS_CALL:
      /* GD->IE relaxation, turn the instruction into 'nop' or
	 'ldr r0, [pc,r0]'  */
      insn = is_local ? 0xe1a00000 : 0xe79f0000;
      bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
      break;

    case R_ARM_THM_TLS_CALL:
      /* GD->IE relaxation.  */
      if (!is_local)
	/* add r0,pc; ldr r0, [r0]  */
	insn = 0x44786800;
      else if (using_thumb2 (globals))
	/* nop.w */
	insn = 0xf3af8000;
      else
	/* nop; nop */
	insn = 0xbf00bf00;

      bfd_put_16 (input_bfd, insn >> 16, contents + rel->r_offset);
      bfd_put_16 (input_bfd, insn & 0xffff, contents + rel->r_offset + 2);
      break;
    }
  return bfd_reloc_ok;
}

/* For a given value of n, calculate the value of G_n as required to
   deal with group relocations.  We return it in the form of an
   encoded constant-and-rotation, together with the final residual.  If n is
   specified as less than zero, then final_residual is filled with the
   input value and no further action is performed.  */

static bfd_vma
calculate_group_reloc_mask (bfd_vma value, int n, bfd_vma *final_residual)
{
  int current_n;
  bfd_vma g_n;
  bfd_vma encoded_g_n = 0;
  bfd_vma residual = value; /* Also known as Y_n.  */

  for (current_n = 0; current_n <= n; current_n++)
    {
      int shift;

      /* Calculate which part of the value to mask.  */
      if (residual == 0)
	shift = 0;
      else
	{
	  int msb;

	  /* Determine the most significant bit in the residual and
	     align the resulting value to a 2-bit boundary.  */
	  for (msb = 30; msb >= 0; msb -= 2)
	    if (residual & (3u << msb))
	      break;

	  /* The desired shift is now (msb - 6), or zero, whichever
	     is the greater.  */
	  shift = msb - 6;
	  if (shift < 0)
	    shift = 0;
	}

      /* Calculate g_n in 32-bit as well as encoded constant+rotation form.  */
      g_n = residual & (0xff << shift);
      encoded_g_n = (g_n >> shift)
		    | ((g_n <= 0xff ? 0 : (32 - shift) / 2) << 8);

      /* Calculate the residual for the next time around.  */
      residual &= ~g_n;
    }

  *final_residual = residual;

  return encoded_g_n;
}

/* Given an ARM instruction, determine whether it is an ADD or a SUB.
   Returns 1 if it is an ADD, -1 if it is a SUB, and 0 otherwise.  */

static int
identify_add_or_sub (bfd_vma insn)
{
  int opcode = insn & 0x1e00000;

  if (opcode == 1 << 23) /* ADD */
    return 1;

  if (opcode == 1 << 22) /* SUB */
    return -1;

  return 0;
}

/* Perform a relocation as part of a final link.  */

static bfd_reloc_status_type
elf32_arm_final_link_relocate (reloc_howto_type *	    howto,
			       bfd *			    input_bfd,
			       bfd *			    output_bfd,
			       asection *		    input_section,
			       bfd_byte *		    contents,
			       Elf_Internal_Rela *	    rel,
			       bfd_vma			    value,
			       struct bfd_link_info *	    info,
			       asection *		    sym_sec,
			       const char *		    sym_name,
			       unsigned char		    st_type,
			       enum arm_st_branch_type	    branch_type,
			       struct elf_link_hash_entry * h,
			       bool *			    unresolved_reloc_p,
			       char **			    error_message)
{
  unsigned long			r_type = howto->type;
  unsigned long			r_symndx;
  bfd_byte *			hit_data = contents + rel->r_offset;
  bfd_vma *			local_got_offsets;
  bfd_vma *			local_tlsdesc_gotents;
  asection *			sgot;
  asection *			splt;
  asection *			sreloc = NULL;
  asection *			srelgot;
  bfd_vma			addend;
  bfd_signed_vma		signed_addend;
  unsigned char			dynreloc_st_type;
  bfd_vma			dynreloc_value;
  struct elf32_arm_link_hash_table * globals;
  struct elf32_arm_link_hash_entry *eh;
  union gotplt_union	       *root_plt;
  struct arm_plt_info	       *arm_plt;
  bfd_vma			plt_offset;
  bfd_vma			gotplt_offset;
  bool				has_iplt_entry;
  bool				resolved_to_zero;

  globals = elf32_arm_hash_table (info);
  if (globals == NULL)
    return bfd_reloc_notsupported;

  BFD_ASSERT (is_arm_elf (input_bfd));
  BFD_ASSERT (howto != NULL);

  /* Some relocation types map to different relocations depending on the
     target.  We pick the right one here.  */
  r_type = arm_real_reloc_type (globals, r_type);

  /* It is possible to have linker relaxations on some TLS access
     models.  Update our information here.  */
  r_type = elf32_arm_tls_transition (info, r_type, h);

  if (r_type != howto->type)
    howto = elf32_arm_howto_from_type (r_type);

  eh = (struct elf32_arm_link_hash_entry *) h;
  sgot = globals->root.sgot;
  local_got_offsets = elf_local_got_offsets (input_bfd);
  local_tlsdesc_gotents = elf32_arm_local_tlsdesc_gotent (input_bfd);

  if (globals->root.dynamic_sections_created)
    srelgot = globals->root.srelgot;
  else
    srelgot = NULL;

  r_symndx = ELF32_R_SYM (rel->r_info);

  if (globals->use_rel)
    {
      bfd_vma sign;

      switch (bfd_get_reloc_size (howto))
	{
	case 1: addend = bfd_get_8 (input_bfd, hit_data); break;
	case 2: addend = bfd_get_16 (input_bfd, hit_data); break;
	case 4: addend = bfd_get_32 (input_bfd, hit_data); break;
	default: addend = 0; break;
	}
      /* Note: the addend and signed_addend calculated here are
	 incorrect for any split field.  */
      addend &= howto->src_mask;
      sign = howto->src_mask & ~(howto->src_mask >> 1);
      signed_addend = (addend ^ sign) - sign;
      signed_addend = (bfd_vma) signed_addend << howto->rightshift;
      addend <<= howto->rightshift;
    }
  else
    addend = signed_addend = rel->r_addend;

  /* ST_BRANCH_TO_ARM is nonsense to thumb-only targets when we
     are resolving a function call relocation.  */
  if (using_thumb_only (globals)
      && (r_type == R_ARM_THM_CALL
	  || r_type == R_ARM_THM_JUMP24)
      && branch_type == ST_BRANCH_TO_ARM)
    branch_type = ST_BRANCH_TO_THUMB;

  /* Record the symbol information that should be used in dynamic
     relocations.  */
  dynreloc_st_type = st_type;
  dynreloc_value = value;
  if (branch_type == ST_BRANCH_TO_THUMB)
    dynreloc_value |= 1;

  /* Find out whether the symbol has a PLT.  Set ST_VALUE, BRANCH_TYPE and
     VALUE appropriately for relocations that we resolve at link time.  */
  has_iplt_entry = false;
  if (elf32_arm_get_plt_info (input_bfd, globals, eh, r_symndx, &root_plt,
			      &arm_plt)
      && root_plt->offset != (bfd_vma) -1)
    {
      plt_offset = root_plt->offset;
      gotplt_offset = arm_plt->got_offset;

      if (h == NULL || eh->is_iplt)
	{
	  has_iplt_entry = true;
	  splt = globals->root.iplt;

	  /* Populate .iplt entries here, because not all of them will
	     be seen by finish_dynamic_symbol.  The lower bit is set if
	     we have already populated the entry.  */
	  if (plt_offset & 1)
	    plt_offset--;
	  else
	    {
	      if (elf32_arm_populate_plt_entry (output_bfd, info, root_plt, arm_plt,
						-1, dynreloc_value))
		root_plt->offset |= 1;
	      else
		return bfd_reloc_notsupported;
	    }

	  /* Static relocations always resolve to the .iplt entry.  */
	  st_type = STT_FUNC;
	  value = (splt->output_section->vma
		   + splt->output_offset
		   + plt_offset);
	  branch_type = ST_BRANCH_TO_ARM;

	  /* If there are non-call relocations that resolve to the .iplt
	     entry, then all dynamic ones must too.  */
	  if (arm_plt->noncall_refcount != 0)
	    {
	      dynreloc_st_type = st_type;
	      dynreloc_value = value;
	    }
	}
      else
	/* We populate the .plt entry in finish_dynamic_symbol.  */
	splt = globals->root.splt;
    }
  else
    {
      splt = NULL;
      plt_offset = (bfd_vma) -1;
      gotplt_offset = (bfd_vma) -1;
    }

  resolved_to_zero = (h != NULL
		      && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));

  switch (r_type)
    {
    case R_ARM_NONE:
      /* We don't need to find a value for this symbol.  It's just a
	 marker.  */
      *unresolved_reloc_p = false;
      return bfd_reloc_ok;

    case R_ARM_ABS12:
      if (globals->root.target_os != is_vxworks)
	return elf32_arm_abs12_reloc (input_bfd, hit_data, value + addend);
      /* Fall through.  */

    case R_ARM_PC24:
    case R_ARM_ABS32:
    case R_ARM_ABS32_NOI:
    case R_ARM_REL32:
    case R_ARM_REL32_NOI:
    case R_ARM_CALL:
    case R_ARM_JUMP24:
    case R_ARM_XPC25:
    case R_ARM_PREL31:
    case R_ARM_PLT32:
      /* Handle relocations which should use the PLT entry.  ABS32/REL32
	 will use the symbol's value, which may point to a PLT entry, but we
	 don't need to handle that here.  If we created a PLT entry, all
	 branches in this object should go to it, except if the PLT is too
	 far away, in which case a long branch stub should be inserted.  */
      if ((r_type != R_ARM_ABS32 && r_type != R_ARM_REL32
	   && r_type != R_ARM_ABS32_NOI && r_type != R_ARM_REL32_NOI
	   && r_type != R_ARM_CALL
	   && r_type != R_ARM_JUMP24
	   && r_type != R_ARM_PLT32)
	  && plt_offset != (bfd_vma) -1)
	{
	  /* If we've created a .plt section, and assigned a PLT entry
	     to this function, it must either be a STT_GNU_IFUNC reference
	     or not be known to bind locally.  In other cases, we should
	     have cleared the PLT entry by now.  */
	  BFD_ASSERT (has_iplt_entry || !SYMBOL_CALLS_LOCAL (info, h));

	  value = (splt->output_section->vma
		   + splt->output_offset
		   + plt_offset);
	  *unresolved_reloc_p = false;
	  return _bfd_final_link_relocate (howto, input_bfd, input_section,
					   contents, rel->r_offset, value,
					   rel->r_addend);
	}

      /* When generating a shared object or relocatable executable, these
	 relocations are copied into the output file to be resolved at
	 run time.  */
      if ((bfd_link_pic (info)
	   || globals->root.is_relocatable_executable
	   || globals->fdpic_p)
	  && (input_section->flags & SEC_ALLOC)
	  && !(globals->root.target_os == is_vxworks
	       && strcmp (input_section->output_section->name,
			  ".tls_vars") == 0)
	  && ((r_type != R_ARM_REL32 && r_type != R_ARM_REL32_NOI)
	      || !SYMBOL_CALLS_LOCAL (info, h))
	  && !(input_bfd == globals->stub_bfd
	       && strstr (input_section->name, STUB_SUFFIX))
	  && (h == NULL
	      || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		  && !resolved_to_zero)
	      || h->root.type != bfd_link_hash_undefweak)
	  && r_type != R_ARM_PC24
	  && r_type != R_ARM_CALL
	  && r_type != R_ARM_JUMP24
	  && r_type != R_ARM_PREL31
	  && r_type != R_ARM_PLT32)
	{
	  Elf_Internal_Rela outrel;
	  bool skip, relocate;
	  int isrofixup = 0;

	  if ((r_type == R_ARM_REL32 || r_type == R_ARM_REL32_NOI)
	      && !h->def_regular)
	    {
	      char *v = _("shared object");

	      if (bfd_link_executable (info))
		v = _("PIE executable");

	      _bfd_error_handler
		(_("%pB: relocation %s against external or undefined symbol `%s'"
		   " can not be used when making a %s; recompile with -fPIC"), input_bfd,
		 elf32_arm_howto_table_1[r_type].name, h->root.root.string, v);
	      return bfd_reloc_notsupported;
	    }

	  *unresolved_reloc_p = false;

	  if (sreloc == NULL && globals->root.dynamic_sections_created)
	    {
	      sreloc = _bfd_elf_get_dynamic_reloc_section (input_bfd, input_section,
							   ! globals->use_rel);

	      if (sreloc == NULL)
		return bfd_reloc_notsupported;
	    }

	  skip = false;
	  relocate = false;

	  outrel.r_addend = addend;
	  outrel.r_offset =
	    _bfd_elf_section_offset (output_bfd, info, input_section,
				     rel->r_offset);
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
		       || !(bfd_link_pie (info)
			    || SYMBOLIC_BIND (info, h))
		       || !h->def_regular))
	    outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
	  else
	    {
	      int symbol;

	      /* This symbol is local, or marked to become local.  */
	      BFD_ASSERT (r_type == R_ARM_ABS32 || r_type == R_ARM_ABS32_NOI
			  || (globals->fdpic_p && !bfd_link_pic (info)));
	      /* On SVR4-ish systems, the dynamic loader cannot
		 relocate the text and data segments independently,
		 so the symbol does not matter.  */
	      symbol = 0;
	      if (dynreloc_st_type == STT_GNU_IFUNC)
		/* We have an STT_GNU_IFUNC symbol that doesn't resolve
		   to the .iplt entry.  Instead, every non-call reference
		   must use an R_ARM_IRELATIVE relocation to obtain the
		   correct run-time address.  */
		outrel.r_info = ELF32_R_INFO (symbol, R_ARM_IRELATIVE);
	      else if (globals->fdpic_p && !bfd_link_pic (info))
		isrofixup = 1;
	      else
		outrel.r_info = ELF32_R_INFO (symbol, R_ARM_RELATIVE);
	      if (globals->use_rel)
		relocate = true;
	      else
		outrel.r_addend += dynreloc_value;
	    }

	  if (isrofixup)
	    arm_elf_add_rofixup (output_bfd, globals->srofixup, outrel.r_offset);
	  else
	    elf32_arm_add_dynreloc (output_bfd, info, sreloc, &outrel);

	  /* If this reloc is against an external symbol, we do not want to
	     fiddle with the addend.  Otherwise, we need to include the symbol
	     value so that it becomes an addend for the dynamic reloc.  */
	  if (! relocate)
	    return bfd_reloc_ok;

	  return _bfd_final_link_relocate (howto, input_bfd, input_section,
					   contents, rel->r_offset,
					   dynreloc_value, (bfd_vma) 0);
	}
      else switch (r_type)
	{
	case R_ARM_ABS12:
	  return elf32_arm_abs12_reloc (input_bfd, hit_data, value + addend);

	case R_ARM_XPC25:	  /* Arm BLX instruction.  */
	case R_ARM_CALL:
	case R_ARM_JUMP24:
	case R_ARM_PC24:	  /* Arm B/BL instruction.  */
	case R_ARM_PLT32:
	  {
	  struct elf32_arm_stub_hash_entry *stub_entry = NULL;

	  if (r_type == R_ARM_XPC25)
	    {
	      /* Check for Arm calling Arm function.  */
	      /* FIXME: Should we translate the instruction into a BL
		 instruction instead ?  */
	      if (branch_type != ST_BRANCH_TO_THUMB)
		_bfd_error_handler
		  (_("\%pB: warning: %s BLX instruction targets"
		     " %s function '%s'"),
		   input_bfd, "ARM",
		   "ARM", h ? h->root.root.string : "(local)");
	    }
	  else if (r_type == R_ARM_PC24)
	    {
	      /* Check for Arm calling Thumb function.  */
	      if (branch_type == ST_BRANCH_TO_THUMB)
		{
		  if (elf32_arm_to_thumb_stub (info, sym_name, input_bfd,
					       output_bfd, input_section,
					       hit_data, sym_sec, rel->r_offset,
					       signed_addend, value,
					       error_message))
		    return bfd_reloc_ok;
		  else
		    return bfd_reloc_dangerous;
		}
	    }

	  /* Check if a stub has to be inserted because the
	     destination is too far or we are changing mode.  */
	  if (   r_type == R_ARM_CALL
	      || r_type == R_ARM_JUMP24
	      || r_type == R_ARM_PLT32)
	    {
	      enum elf32_arm_stub_type stub_type = arm_stub_none;
	      struct elf32_arm_link_hash_entry *hash;

	      hash = (struct elf32_arm_link_hash_entry *) h;
	      stub_type = arm_type_of_stub (info, input_section, rel,
					    st_type, &branch_type,
					    hash, value, sym_sec,
					    input_bfd, sym_name);

	      if (stub_type != arm_stub_none)
		{
		  /* The target is out of reach, so redirect the
		     branch to the local stub for this function.  */
		  stub_entry = elf32_arm_get_stub_entry (input_section,
							 sym_sec, h,
							 rel, globals,
							 stub_type);
		  {
		    if (stub_entry != NULL)
		      value = (stub_entry->stub_offset
			       + stub_entry->stub_sec->output_offset
			       + stub_entry->stub_sec->output_section->vma);

		    if (plt_offset != (bfd_vma) -1)
		      *unresolved_reloc_p = false;
		  }
		}
	      else
		{
		  /* If the call goes through a PLT entry, make sure to
		     check distance to the right destination address.  */
		  if (plt_offset != (bfd_vma) -1)
		    {
		      value = (splt->output_section->vma
			       + splt->output_offset
			       + plt_offset);
		      *unresolved_reloc_p = false;
		      /* The PLT entry is in ARM mode, regardless of the
			 target function.  */
		      branch_type = ST_BRANCH_TO_ARM;
		    }
		}
	    }

	  /* The ARM ELF ABI says that this reloc is computed as: S - P + A
	     where:
	      S is the address of the symbol in the relocation.
	      P is address of the instruction being relocated.
	      A is the addend (extracted from the instruction) in bytes.

	     S is held in 'value'.
	     P is the base address of the section containing the
	       instruction plus the offset of the reloc into that
	       section, ie:
		 (input_section->output_section->vma +
		  input_section->output_offset +
		  rel->r_offset).
	     A is the addend, converted into bytes, ie:
		 (signed_addend * 4)

	     Note: None of these operations have knowledge of the pipeline
	     size of the processor, thus it is up to the assembler to
	     encode this information into the addend.  */
	  value -= (input_section->output_section->vma
		    + input_section->output_offset);
	  value -= rel->r_offset;
	  value += signed_addend;

	  signed_addend = value;
	  signed_addend >>= howto->rightshift;

	  /* A branch to an undefined weak symbol is turned into a jump to
	     the next instruction unless a PLT entry will be created.
	     Do the same for local undefined symbols (but not for STN_UNDEF).
	     The jump to the next instruction is optimized as a NOP depending
	     on the architecture.  */
	  if (h ? (h->root.type == bfd_link_hash_undefweak
		   && plt_offset == (bfd_vma) -1)
	      : r_symndx != STN_UNDEF && bfd_is_und_section (sym_sec))
	    {
	      value = (bfd_get_32 (input_bfd, hit_data) & 0xf0000000);

	      if (arch_has_arm_nop (globals))
		value |= 0x0320f000;
	      else
		value |= 0x01a00000; /* Using pre-UAL nop: mov r0, r0.  */
	    }
	  else
	    {
	      /* Perform a signed range check.  */
	      if (   signed_addend >   ((bfd_signed_vma)  (howto->dst_mask >> 1))
		  || signed_addend < - ((bfd_signed_vma) ((howto->dst_mask + 1) >> 1)))
		return bfd_reloc_overflow;

	      addend = (value & 2);

	      value = (signed_addend & howto->dst_mask)
		| (bfd_get_32 (input_bfd, hit_data) & (~ howto->dst_mask));

	      if (r_type == R_ARM_CALL)
		{
		  /* Set the H bit in the BLX instruction.  */
		  if (branch_type == ST_BRANCH_TO_THUMB)
		    {
		      if (addend)
			value |= (1 << 24);
		      else
			value &= ~(bfd_vma)(1 << 24);
		    }

		  /* Select the correct instruction (BL or BLX).  */
		  /* Only if we are not handling a BL to a stub. In this
		     case, mode switching is performed by the stub.  */
		  if (branch_type == ST_BRANCH_TO_THUMB && !stub_entry)
		    value |= (1 << 28);
		  else if (stub_entry || branch_type != ST_BRANCH_UNKNOWN)
		    {
		      value &= ~(bfd_vma)(1 << 28);
		      value |= (1 << 24);
		    }
		}
	    }
	  }
	  break;

	case R_ARM_ABS32:
	  value += addend;
	  if (branch_type == ST_BRANCH_TO_THUMB)
	    value |= 1;
	  break;

	case R_ARM_ABS32_NOI:
	  value += addend;
	  break;

	case R_ARM_REL32:
	  value += addend;
	  if (branch_type == ST_BRANCH_TO_THUMB)
	    value |= 1;
	  value -= (input_section->output_section->vma
		    + input_section->output_offset + rel->r_offset);
	  break;

	case R_ARM_REL32_NOI:
	  value += addend;
	  value -= (input_section->output_section->vma
		    + input_section->output_offset + rel->r_offset);
	  break;

	case R_ARM_PREL31:
	  value -= (input_section->output_section->vma
		    + input_section->output_offset + rel->r_offset);
	  value += signed_addend;
	  if (! h || h->root.type != bfd_link_hash_undefweak)
	    {
	      /* Check for overflow.  */
	      if ((value ^ (value >> 1)) & (1 << 30))
		return bfd_reloc_overflow;
	    }
	  value &= 0x7fffffff;
	  value |= (bfd_get_32 (input_bfd, hit_data) & 0x80000000);
	  if (branch_type == ST_BRANCH_TO_THUMB)
	    value |= 1;
	  break;
	}

      bfd_put_32 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_ARM_ABS8:
      value += addend;

      /* There is no way to tell whether the user intended to use a signed or
	 unsigned addend.  When checking for overflow we accept either,
	 as specified by the AAELF.  */
      if ((long) value > 0xff || (long) value < -0x80)
	return bfd_reloc_overflow;

      bfd_put_8 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_ARM_ABS16:
      value += addend;

      /* See comment for R_ARM_ABS8.  */
      if ((long) value > 0xffff || (long) value < -0x8000)
	return bfd_reloc_overflow;

      bfd_put_16 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_ARM_THM_ABS5:
      /* Support ldr and str instructions for the thumb.  */
      if (globals->use_rel)
	{
	  /* Need to refetch addend.  */
	  addend = bfd_get_16 (input_bfd, hit_data) & howto->src_mask;
	  /* ??? Need to determine shift amount from operand size.  */
	  addend >>= howto->rightshift;
	}
      value += addend;

      /* ??? Isn't value unsigned?  */
      if ((long) value > 0x1f || (long) value < -0x10)
	return bfd_reloc_overflow;

      /* ??? Value needs to be properly shifted into place first.  */
      value |= bfd_get_16 (input_bfd, hit_data) & 0xf83f;
      bfd_put_16 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_ARM_THM_ALU_PREL_11_0:
      /* Corresponds to: addw.w reg, pc, #offset (and similarly for subw).  */
      {
	bfd_vma insn;
	bfd_signed_vma relocation;

	insn = (bfd_get_16 (input_bfd, hit_data) << 16)
	     | bfd_get_16 (input_bfd, hit_data + 2);

	if (globals->use_rel)
	  {
	    signed_addend = (insn & 0xff) | ((insn & 0x7000) >> 4)
			  | ((insn & (1 << 26)) >> 15);
	    if (insn & 0xf00000)
	      signed_addend = -signed_addend;
	  }

	relocation = value + signed_addend;
	relocation -= Pa (input_section->output_section->vma
			  + input_section->output_offset
			  + rel->r_offset);

	/* PR 21523: Use an absolute value.  The user of this reloc will
	   have already selected an ADD or SUB insn appropriately.  */
	value = llabs (relocation);

	if (value >= 0x1000)
	  return bfd_reloc_overflow;

	/* Destination is Thumb.  Force bit 0 to 1 to reflect this.  */
	if (branch_type == ST_BRANCH_TO_THUMB)
	  value |= 1;

	insn = (insn & 0xfb0f8f00) | (value & 0xff)
	     | ((value & 0x700) << 4)
	     | ((value & 0x800) << 15);
	if (relocation < 0)
	  insn |= 0xa00000;

	bfd_put_16 (input_bfd, insn >> 16, hit_data);
	bfd_put_16 (input_bfd, insn & 0xffff, hit_data + 2);

	return bfd_reloc_ok;
      }

    case R_ARM_THM_PC8:
      /* PR 10073:  This reloc is not generated by the GNU toolchain,
	 but it is supported for compatibility with third party libraries
	 generated by other compilers, specifically the ARM/IAR.  */
      {
	bfd_vma insn;
	bfd_signed_vma relocation;

	insn = bfd_get_16 (input_bfd, hit_data);

	if (globals->use_rel)
	  addend = ((((insn & 0x00ff) << 2) + 4) & 0x3ff) -4;

	relocation = value + addend;
	relocation -= Pa (input_section->output_section->vma
			  + input_section->output_offset
			  + rel->r_offset);

	value = relocation;

	/* We do not check for overflow of this reloc.  Although strictly
	   speaking this is incorrect, it appears to be necessary in order
	   to work with IAR generated relocs.  Since GCC and GAS do not
	   generate R_ARM_THM_PC8 relocs, the lack of a check should not be
	   a problem for them.  */
	value &= 0x3fc;

	insn = (insn & 0xff00) | (value >> 2);

	bfd_put_16 (input_bfd, insn, hit_data);

	return bfd_reloc_ok;
      }

    case R_ARM_THM_PC12:
      /* Corresponds to: ldr.w reg, [pc, #offset].  */
      {
	bfd_vma insn;
	bfd_signed_vma relocation;

	insn = (bfd_get_16 (input_bfd, hit_data) << 16)
	     | bfd_get_16 (input_bfd, hit_data + 2);

	if (globals->use_rel)
	  {
	    signed_addend = insn & 0xfff;
	    if (!(insn & (1 << 23)))
	      signed_addend = -signed_addend;
	  }

	relocation = value + signed_addend;
	relocation -= Pa (input_section->output_section->vma
			  + input_section->output_offset
			  + rel->r_offset);

	value = relocation;

	if (value >= 0x1000)
	  return bfd_reloc_overflow;

	insn = (insn & 0xff7ff000) | value;
	if (relocation >= 0)
	  insn |= (1 << 23);

	bfd_put_16 (input_bfd, insn >> 16, hit_data);
	bfd_put_16 (input_bfd, insn & 0xffff, hit_data + 2);

	return bfd_reloc_ok;
      }

    case R_ARM_THM_XPC22:
    case R_ARM_THM_CALL:
    case R_ARM_THM_JUMP24:
      /* Thumb BL (branch long instruction).  */
      {
	bfd_vma relocation;
	bfd_vma reloc_sign;
	bool overflow = false;
	bfd_vma upper_insn = bfd_get_16 (input_bfd, hit_data);
	bfd_vma lower_insn = bfd_get_16 (input_bfd, hit_data + 2);
	bfd_signed_vma reloc_signed_max;
	bfd_signed_vma reloc_signed_min;
	bfd_vma check;
	bfd_signed_vma signed_check;
	int bitsize;
	const int thumb2 = using_thumb2 (globals);
	const int thumb2_bl = using_thumb2_bl (globals);

	/* A branch to an undefined weak symbol is turned into a jump to
	   the next instruction unless a PLT entry will be created.
	   The jump to the next instruction is optimized as a NOP.W for
	   Thumb-2 enabled architectures.  */
	if (h && h->root.type == bfd_link_hash_undefweak
	    && plt_offset == (bfd_vma) -1)
	  {
	    if (thumb2)
	      {
		bfd_put_16 (input_bfd, 0xf3af, hit_data);
		bfd_put_16 (input_bfd, 0x8000, hit_data + 2);
	      }
	    else
	      {
		bfd_put_16 (input_bfd, 0xe000, hit_data);
		bfd_put_16 (input_bfd, 0xbf00, hit_data + 2);
	      }
	    return bfd_reloc_ok;
	  }

	/* Fetch the addend.  We use the Thumb-2 encoding (backwards compatible
	   with Thumb-1) involving the J1 and J2 bits.  */
	if (globals->use_rel)
	  {
	    bfd_vma s = (upper_insn & (1 << 10)) >> 10;
	    bfd_vma upper = upper_insn & 0x3ff;
	    bfd_vma lower = lower_insn & 0x7ff;
	    bfd_vma j1 = (lower_insn & (1 << 13)) >> 13;
	    bfd_vma j2 = (lower_insn & (1 << 11)) >> 11;
	    bfd_vma i1 = j1 ^ s ? 0 : 1;
	    bfd_vma i2 = j2 ^ s ? 0 : 1;

	    addend = (i1 << 23) | (i2 << 22) | (upper << 12) | (lower << 1);
	    /* Sign extend.  */
	    addend = (addend | ((s ? 0 : 1) << 24)) - (1 << 24);

	    signed_addend = addend;
	  }

	if (r_type == R_ARM_THM_XPC22)
	  {
	    /* Check for Thumb to Thumb call.  */
	    /* FIXME: Should we translate the instruction into a BL
	       instruction instead ?  */
	    if (branch_type == ST_BRANCH_TO_THUMB)
	      _bfd_error_handler
		(_("%pB: warning: %s BLX instruction targets"
		   " %s function '%s'"),
		 input_bfd, "Thumb",
		 "Thumb", h ? h->root.root.string : "(local)");
	  }
	else
	  {
	    /* If it is not a call to Thumb, assume call to Arm.
	       If it is a call relative to a section name, then it is not a
	       function call at all, but rather a long jump.  Calls through
	       the PLT do not require stubs.  */
	    if (branch_type == ST_BRANCH_TO_ARM && plt_offset == (bfd_vma) -1)
	      {
		if (globals->use_blx && r_type == R_ARM_THM_CALL)
		  {
		    /* Convert BL to BLX.  */
		    lower_insn = (lower_insn & ~0x1000) | 0x0800;
		  }
		else if ((   r_type != R_ARM_THM_CALL)
			 && (r_type != R_ARM_THM_JUMP24))
		  {
		    if (elf32_thumb_to_arm_stub
			(info, sym_name, input_bfd, output_bfd, input_section,
			 hit_data, sym_sec, rel->r_offset, signed_addend, value,
			 error_message))
		      return bfd_reloc_ok;
		    else
		      return bfd_reloc_dangerous;
		  }
	      }
	    else if (branch_type == ST_BRANCH_TO_THUMB
		     && globals->use_blx
		     && r_type == R_ARM_THM_CALL)
	      {
		/* Make sure this is a BL.  */
		lower_insn |= 0x1800;
	      }
	  }

	enum elf32_arm_stub_type stub_type = arm_stub_none;
	if (r_type == R_ARM_THM_CALL || r_type == R_ARM_THM_JUMP24)
	  {
	    /* Check if a stub has to be inserted because the destination
	       is too far.  */
	    struct elf32_arm_stub_hash_entry *stub_entry;
	    struct elf32_arm_link_hash_entry *hash;

	    hash = (struct elf32_arm_link_hash_entry *) h;

	    stub_type = arm_type_of_stub (info, input_section, rel,
					  st_type, &branch_type,
					  hash, value, sym_sec,
					  input_bfd, sym_name);

	    if (stub_type != arm_stub_none)
	      {
		/* The target is out of reach or we are changing modes, so
		   redirect the branch to the local stub for this
		   function.  */
		stub_entry = elf32_arm_get_stub_entry (input_section,
						       sym_sec, h,
						       rel, globals,
						       stub_type);
		if (stub_entry != NULL)
		  {
		    value = (stub_entry->stub_offset
			     + stub_entry->stub_sec->output_offset
			     + stub_entry->stub_sec->output_section->vma);

		    if (plt_offset != (bfd_vma) -1)
		      *unresolved_reloc_p = false;
		  }

		/* If this call becomes a call to Arm, force BLX.  */
		if (globals->use_blx && (r_type == R_ARM_THM_CALL))
		  {
		    if ((stub_entry
			 && !arm_stub_is_thumb (stub_entry->stub_type))
			|| branch_type != ST_BRANCH_TO_THUMB)
		      lower_insn = (lower_insn & ~0x1000) | 0x0800;
		  }
	      }
	  }

	/* Handle calls via the PLT.  */
	if (stub_type == arm_stub_none && plt_offset != (bfd_vma) -1)
	  {
	    value = (splt->output_section->vma
		     + splt->output_offset
		     + plt_offset);

	    if (globals->use_blx
		&& r_type == R_ARM_THM_CALL
		&& ! using_thumb_only (globals))
	      {
		/* If the Thumb BLX instruction is available, convert
		   the BL to a BLX instruction to call the ARM-mode
		   PLT entry.  */
		lower_insn = (lower_insn & ~0x1000) | 0x0800;
		branch_type = ST_BRANCH_TO_ARM;
	      }
	    else
	      {
		if (! using_thumb_only (globals))
		  /* Target the Thumb stub before the ARM PLT entry.  */
		  value -= PLT_THUMB_STUB_SIZE;
		branch_type = ST_BRANCH_TO_THUMB;
	      }
	    *unresolved_reloc_p = false;
	  }

	relocation = value + signed_addend;

	relocation -= (input_section->output_section->vma
		       + input_section->output_offset
		       + rel->r_offset);

	check = relocation >> howto->rightshift;

	/* If this is a signed value, the rightshift just dropped
	   leading 1 bits (assuming twos complement).  */
	if ((bfd_signed_vma) relocation >= 0)
	  signed_check = check;
	else
	  signed_check = check | ~((bfd_vma) -1 >> howto->rightshift);

	/* Calculate the permissable maximum and minimum values for
	   this relocation according to whether we're relocating for
	   Thumb-2 or not.  */
	bitsize = howto->bitsize;
	if (!thumb2_bl)
	  bitsize -= 2;
	reloc_signed_max = (1 << (bitsize - 1)) - 1;
	reloc_signed_min = ~reloc_signed_max;

	/* Assumes two's complement.  */
	if (signed_check > reloc_signed_max || signed_check < reloc_signed_min)
	  overflow = true;

	if ((lower_insn & 0x5000) == 0x4000)
	  /* For a BLX instruction, make sure that the relocation is rounded up
	     to a word boundary.  This follows the semantics of the instruction
	     which specifies that bit 1 of the target address will come from bit
	     1 of the base address.  */
	  relocation = (relocation + 2) & ~ 3;

	/* Put RELOCATION back into the insn.  Assumes two's complement.
	   We use the Thumb-2 encoding, which is safe even if dealing with
	   a Thumb-1 instruction by virtue of our overflow check above.  */
	reloc_sign = (signed_check < 0) ? 1 : 0;
	upper_insn = (upper_insn & ~(bfd_vma) 0x7ff)
		     | ((relocation >> 12) & 0x3ff)
		     | (reloc_sign << 10);
	lower_insn = (lower_insn & ~(bfd_vma) 0x2fff)
		     | (((!((relocation >> 23) & 1)) ^ reloc_sign) << 13)
		     | (((!((relocation >> 22) & 1)) ^ reloc_sign) << 11)
		     | ((relocation >> 1) & 0x7ff);

	/* Put the relocated value back in the object file:  */
	bfd_put_16 (input_bfd, upper_insn, hit_data);
	bfd_put_16 (input_bfd, lower_insn, hit_data + 2);

	return (overflow ? bfd_reloc_overflow : bfd_reloc_ok);
      }
      break;

    case R_ARM_THM_JUMP19:
      /* Thumb32 conditional branch instruction.  */
      {
	bfd_vma relocation;
	bool overflow = false;
	bfd_vma upper_insn = bfd_get_16 (input_bfd, hit_data);
	bfd_vma lower_insn = bfd_get_16 (input_bfd, hit_data + 2);
	bfd_signed_vma reloc_signed_max = 0xffffe;
	bfd_signed_vma reloc_signed_min = -0x100000;
	bfd_signed_vma signed_check;
	enum elf32_arm_stub_type stub_type = arm_stub_none;
	struct elf32_arm_stub_hash_entry *stub_entry;
	struct elf32_arm_link_hash_entry *hash;

	/* Need to refetch the addend, reconstruct the top three bits,
	   and squish the two 11 bit pieces together.  */
	if (globals->use_rel)
	  {
	    bfd_vma S     = (upper_insn & 0x0400) >> 10;
	    bfd_vma upper = (upper_insn & 0x003f);
	    bfd_vma J1    = (lower_insn & 0x2000) >> 13;
	    bfd_vma J2    = (lower_insn & 0x0800) >> 11;
	    bfd_vma lower = (lower_insn & 0x07ff);

	    upper |= J1 << 6;
	    upper |= J2 << 7;
	    upper |= (!S) << 8;
	    upper -= 0x0100; /* Sign extend.  */

	    addend = (upper << 12) | (lower << 1);
	    signed_addend = addend;
	  }

	/* Handle calls via the PLT.  */
	if (plt_offset != (bfd_vma) -1)
	  {
	    value = (splt->output_section->vma
		     + splt->output_offset
		     + plt_offset);
	    /* Target the Thumb stub before the ARM PLT entry.  */
	    value -= PLT_THUMB_STUB_SIZE;
	    *unresolved_reloc_p = false;
	  }

	hash = (struct elf32_arm_link_hash_entry *)h;

	stub_type = arm_type_of_stub (info, input_section, rel,
				      st_type, &branch_type,
				      hash, value, sym_sec,
				      input_bfd, sym_name);
	if (stub_type != arm_stub_none)
	  {
	    stub_entry = elf32_arm_get_stub_entry (input_section,
						   sym_sec, h,
						   rel, globals,
						   stub_type);
	    if (stub_entry != NULL)
	      {
		value = (stub_entry->stub_offset
			+ stub_entry->stub_sec->output_offset
			+ stub_entry->stub_sec->output_section->vma);
	      }
	  }

	relocation = value + signed_addend;
	relocation -= (input_section->output_section->vma
		       + input_section->output_offset
		       + rel->r_offset);
	signed_check = (bfd_signed_vma) relocation;

	if (signed_check > reloc_signed_max || signed_check < reloc_signed_min)
	  overflow = true;

	/* Put RELOCATION back into the insn.  */
	{
	  bfd_vma S  = (relocation & 0x00100000) >> 20;
	  bfd_vma J2 = (relocation & 0x00080000) >> 19;
	  bfd_vma J1 = (relocation & 0x00040000) >> 18;
	  bfd_vma hi = (relocation & 0x0003f000) >> 12;
	  bfd_vma lo = (relocation & 0x00000ffe) >>  1;

	  upper_insn = (upper_insn & 0xfbc0) | (S << 10) | hi;
	  lower_insn = (lower_insn & 0xd000) | (J1 << 13) | (J2 << 11) | lo;
	}

	/* Put the relocated value back in the object file:  */
	bfd_put_16 (input_bfd, upper_insn, hit_data);
	bfd_put_16 (input_bfd, lower_insn, hit_data + 2);

	return (overflow ? bfd_reloc_overflow : bfd_reloc_ok);
      }

    case R_ARM_THM_JUMP11:
    case R_ARM_THM_JUMP8:
    case R_ARM_THM_JUMP6:
      /* Thumb B (branch) instruction).  */
      {
	bfd_signed_vma relocation;
	bfd_signed_vma reloc_signed_max = (1 << (howto->bitsize - 1)) - 1;
	bfd_signed_vma reloc_signed_min = ~ reloc_signed_max;
	bfd_signed_vma signed_check;

	/* CZB cannot jump backward.  */
	if (r_type == R_ARM_THM_JUMP6)
	  {
	    reloc_signed_min = 0;
	    if (globals->use_rel)
	      signed_addend = ((addend & 0x200) >> 3) | ((addend & 0xf8) >> 2);
	  }

	relocation = value + signed_addend;

	relocation -= (input_section->output_section->vma
		       + input_section->output_offset
		       + rel->r_offset);

	relocation >>= howto->rightshift;
	signed_check = relocation;

	if (r_type == R_ARM_THM_JUMP6)
	  relocation = ((relocation & 0x0020) << 4) | ((relocation & 0x001f) << 3);
	else
	  relocation &= howto->dst_mask;
	relocation |= (bfd_get_16 (input_bfd, hit_data) & (~ howto->dst_mask));

	bfd_put_16 (input_bfd, relocation, hit_data);

	/* Assumes two's complement.  */
	if (signed_check > reloc_signed_max || signed_check < reloc_signed_min)
	  return bfd_reloc_overflow;

	return bfd_reloc_ok;
      }

    case R_ARM_ALU_PCREL7_0:
    case R_ARM_ALU_PCREL15_8:
    case R_ARM_ALU_PCREL23_15:
      {
	bfd_vma insn;
	bfd_vma relocation;

	insn = bfd_get_32 (input_bfd, hit_data);
	if (globals->use_rel)
	  {
	    /* Extract the addend.  */
	    addend = (insn & 0xff) << ((insn & 0xf00) >> 7);
	    signed_addend = addend;
	  }
	relocation = value + signed_addend;

	relocation -= (input_section->output_section->vma
		       + input_section->output_offset
		       + rel->r_offset);
	insn = (insn & ~0xfff)
	       | ((howto->bitpos << 7) & 0xf00)
	       | ((relocation >> howto->bitpos) & 0xff);
	bfd_put_32 (input_bfd, value, hit_data);
      }
      return bfd_reloc_ok;

    case R_ARM_GNU_VTINHERIT:
    case R_ARM_GNU_VTENTRY:
      return bfd_reloc_ok;

    case R_ARM_GOTOFF32:
      /* Relocation is relative to the start of the
	 global offset table.  */

      BFD_ASSERT (sgot != NULL);
      if (sgot == NULL)
	return bfd_reloc_notsupported;

      /* If we are addressing a Thumb function, we need to adjust the
	 address by one, so that attempts to call the function pointer will
	 correctly interpret it as Thumb code.  */
      if (branch_type == ST_BRANCH_TO_THUMB)
	value += 1;

      /* Note that sgot->output_offset is not involved in this
	 calculation.  We always want the start of .got.  If we
	 define _GLOBAL_OFFSET_TABLE in a different way, as is
	 permitted by the ABI, we might have to change this
	 calculation.  */
      value -= sgot->output_section->vma;
      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, rel->r_offset, value,
				       rel->r_addend);

    case R_ARM_GOTPC:
      /* Use global offset table as symbol value.  */
      BFD_ASSERT (sgot != NULL);

      if (sgot == NULL)
	return bfd_reloc_notsupported;

      *unresolved_reloc_p = false;
      value = sgot->output_section->vma;
      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, rel->r_offset, value,
				       rel->r_addend);

    case R_ARM_GOT32:
    case R_ARM_GOT_PREL:
      /* Relocation is to the entry for this symbol in the
	 global offset table.  */
      if (sgot == NULL)
	return bfd_reloc_notsupported;

      if (dynreloc_st_type == STT_GNU_IFUNC
	  && plt_offset != (bfd_vma) -1
	  && (h == NULL || SYMBOL_REFERENCES_LOCAL (info, h)))
	{
	  /* We have a relocation against a locally-binding STT_GNU_IFUNC
	     symbol, and the relocation resolves directly to the runtime
	     target rather than to the .iplt entry.  This means that any
	     .got entry would be the same value as the .igot.plt entry,
	     so there's no point creating both.  */
	  sgot = globals->root.igotplt;
	  value = sgot->output_offset + gotplt_offset;
	}
      else if (h != NULL)
	{
	  bfd_vma off;

	  off = h->got.offset;
	  BFD_ASSERT (off != (bfd_vma) -1);
	  if ((off & 1) != 0)
	    {
	      /* We have already processsed one GOT relocation against
		 this symbol.  */
	      off &= ~1;
	      if (globals->root.dynamic_sections_created
		  && !SYMBOL_REFERENCES_LOCAL (info, h))
		*unresolved_reloc_p = false;
	    }
	  else
	    {
	      Elf_Internal_Rela outrel;
	      int isrofixup = 0;

	      if (((h->dynindx != -1) || globals->fdpic_p)
		  && !SYMBOL_REFERENCES_LOCAL (info, h))
		{
		  /* If the symbol doesn't resolve locally in a static
		     object, we have an undefined reference.  If the
		     symbol doesn't resolve locally in a dynamic object,
		     it should be resolved by the dynamic linker.  */
		  if (globals->root.dynamic_sections_created)
		    {
		      outrel.r_info = ELF32_R_INFO (h->dynindx, R_ARM_GLOB_DAT);
		      *unresolved_reloc_p = false;
		    }
		  else
		    outrel.r_info = 0;
		  outrel.r_addend = 0;
		}
	      else
		{
		  if (dynreloc_st_type == STT_GNU_IFUNC)
		    outrel.r_info = ELF32_R_INFO (0, R_ARM_IRELATIVE);
		  else if (bfd_link_pic (info)
			   && !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
		    outrel.r_info = ELF32_R_INFO (0, R_ARM_RELATIVE);
		  else
		    {
		      outrel.r_info = 0;
		      if (globals->fdpic_p)
			isrofixup = 1;
		    }
		  outrel.r_addend = dynreloc_value;
		}

	      /* The GOT entry is initialized to zero by default.
		 See if we should install a different value.  */
	      if (outrel.r_addend != 0
		  && (globals->use_rel || outrel.r_info == 0))
		{
		  bfd_put_32 (output_bfd, outrel.r_addend,
			      sgot->contents + off);
		  outrel.r_addend = 0;
		}

	      if (isrofixup)
		arm_elf_add_rofixup (output_bfd,
				     elf32_arm_hash_table (info)->srofixup,
				     sgot->output_section->vma
				     + sgot->output_offset + off);

	      else if (outrel.r_info != 0)
		{
		  outrel.r_offset = (sgot->output_section->vma
				     + sgot->output_offset
				     + off);
		  elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
		}

	      h->got.offset |= 1;
	    }
	  value = sgot->output_offset + off;
	}
      else
	{
	  bfd_vma off;

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
	      Elf_Internal_Rela outrel;
	      int isrofixup = 0;

	      if (dynreloc_st_type == STT_GNU_IFUNC)
		outrel.r_info = ELF32_R_INFO (0, R_ARM_IRELATIVE);
	      else if (bfd_link_pic (info))
		outrel.r_info = ELF32_R_INFO (0, R_ARM_RELATIVE);
	      else
		{
		  outrel.r_info = 0;
		  if (globals->fdpic_p)
		    isrofixup = 1;
		}

	      /* The GOT entry is initialized to zero by default.
		 See if we should install a different value.  */
	      if (globals->use_rel || outrel.r_info == 0)
		bfd_put_32 (output_bfd, dynreloc_value, sgot->contents + off);

	      if (isrofixup)
		arm_elf_add_rofixup (output_bfd,
				     globals->srofixup,
				     sgot->output_section->vma
				     + sgot->output_offset + off);

	      else if (outrel.r_info != 0)
		{
		  outrel.r_addend = addend + dynreloc_value;
		  outrel.r_offset = (sgot->output_section->vma
				     + sgot->output_offset
				     + off);
		  elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
		}

	      local_got_offsets[r_symndx] |= 1;
	    }

	  value = sgot->output_offset + off;
	}
      if (r_type != R_ARM_GOT32)
	value += sgot->output_section->vma;

      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, rel->r_offset, value,
				       rel->r_addend);

    case R_ARM_TLS_LDO32:
      value = value - dtpoff_base (info);

      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, rel->r_offset, value,
				       rel->r_addend);

    case R_ARM_TLS_LDM32:
    case R_ARM_TLS_LDM32_FDPIC:
      {
	bfd_vma off;

	if (sgot == NULL)
	  abort ();

	off = globals->tls_ldm_got.offset;

	if ((off & 1) != 0)
	  off &= ~1;
	else
	  {
	    /* If we don't know the module number, create a relocation
	       for it.  */
	    if (bfd_link_dll (info))
	      {
		Elf_Internal_Rela outrel;

		if (srelgot == NULL)
		  abort ();

		outrel.r_addend = 0;
		outrel.r_offset = (sgot->output_section->vma
				   + sgot->output_offset + off);
		outrel.r_info = ELF32_R_INFO (0, R_ARM_TLS_DTPMOD32);

		if (globals->use_rel)
		  bfd_put_32 (output_bfd, outrel.r_addend,
			      sgot->contents + off);

		elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
	      }
	    else
	      bfd_put_32 (output_bfd, 1, sgot->contents + off);

	    globals->tls_ldm_got.offset |= 1;
	  }

	if (r_type == R_ARM_TLS_LDM32_FDPIC)
	  {
	    bfd_put_32 (output_bfd,
			globals->root.sgot->output_offset + off,
			contents + rel->r_offset);

	    return bfd_reloc_ok;
	  }
	else
	  {
	    value = sgot->output_section->vma + sgot->output_offset + off
	      - (input_section->output_section->vma
		 + input_section->output_offset + rel->r_offset);

	    return _bfd_final_link_relocate (howto, input_bfd, input_section,
					     contents, rel->r_offset, value,
					     rel->r_addend);
	  }
      }

    case R_ARM_TLS_CALL:
    case R_ARM_THM_TLS_CALL:
    case R_ARM_TLS_GD32:
    case R_ARM_TLS_GD32_FDPIC:
    case R_ARM_TLS_IE32:
    case R_ARM_TLS_IE32_FDPIC:
    case R_ARM_TLS_GOTDESC:
    case R_ARM_TLS_DESCSEQ:
    case R_ARM_THM_TLS_DESCSEQ:
      {
	bfd_vma off, offplt;
	int indx = 0;
	char tls_type;

	BFD_ASSERT (sgot != NULL);

	if (h != NULL)
	  {
	    bool dyn;
	    dyn = globals->root.dynamic_sections_created;
	    if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						 bfd_link_pic (info),
						 h)
		&& (!bfd_link_pic (info)
		    || !SYMBOL_REFERENCES_LOCAL (info, h)))
	      {
		*unresolved_reloc_p = false;
		indx = h->dynindx;
	      }
	    off = h->got.offset;
	    offplt = elf32_arm_hash_entry (h)->tlsdesc_got;
	    tls_type = ((struct elf32_arm_link_hash_entry *) h)->tls_type;
	  }
	else
	  {
	    BFD_ASSERT (local_got_offsets != NULL);

	    if (r_symndx >= elf32_arm_num_entries (input_bfd))
	      {
		_bfd_error_handler (_("\
%pB: expected symbol index in range 0..%lu but found local symbol with index %lu"),
				    input_bfd,
				    (unsigned long) elf32_arm_num_entries (input_bfd),
				    r_symndx);
		return false;
	      }
	    off = local_got_offsets[r_symndx];
	    offplt = local_tlsdesc_gotents[r_symndx];
	    tls_type = elf32_arm_local_got_tls_type (input_bfd)[r_symndx];
	  }

	/* Linker relaxations happens from one of the
	   R_ARM_{GOTDESC,CALL,DESCSEQ} relocations to IE or LE.  */
	if (ELF32_R_TYPE (rel->r_info) != r_type)
	  tls_type = GOT_TLS_IE;

	BFD_ASSERT (tls_type != GOT_UNKNOWN);

	if ((off & 1) != 0)
	  off &= ~1;
	else
	  {
	    bool need_relocs = false;
	    Elf_Internal_Rela outrel;
	    int cur_off = off;

	    /* The GOT entries have not been initialized yet.  Do it
	       now, and emit any relocations.  If both an IE GOT and a
	       GD GOT are necessary, we emit the GD first.  */

	    if ((bfd_link_dll (info) || indx != 0)
		&& (h == NULL
		    || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			&& !resolved_to_zero)
		    || h->root.type != bfd_link_hash_undefweak))
	      {
		need_relocs = true;
		BFD_ASSERT (srelgot != NULL);
	      }

	    if (tls_type & GOT_TLS_GDESC)
	      {
		bfd_byte *loc;

		/* We should have relaxed, unless this is an undefined
		   weak symbol.  */
		BFD_ASSERT ((h && (h->root.type == bfd_link_hash_undefweak))
			    || bfd_link_dll (info));
		BFD_ASSERT (globals->sgotplt_jump_table_size + offplt + 8
			    <= globals->root.sgotplt->size);

		outrel.r_addend = 0;
		outrel.r_offset = (globals->root.sgotplt->output_section->vma
				   + globals->root.sgotplt->output_offset
				   + offplt
				   + globals->sgotplt_jump_table_size);

		outrel.r_info = ELF32_R_INFO (indx, R_ARM_TLS_DESC);
		sreloc = globals->root.srelplt;
		loc = sreloc->contents;
		loc += globals->next_tls_desc_index++ * RELOC_SIZE (globals);
		BFD_ASSERT (loc + RELOC_SIZE (globals)
			   <= sreloc->contents + sreloc->size);

		SWAP_RELOC_OUT (globals) (output_bfd, &outrel, loc);

		/* For globals, the first word in the relocation gets
		   the relocation index and the top bit set, or zero,
		   if we're binding now.  For locals, it gets the
		   symbol's offset in the tls section.  */
		bfd_put_32 (output_bfd,
			    !h ? value - elf_hash_table (info)->tls_sec->vma
			    : info->flags & DF_BIND_NOW ? 0
			    : 0x80000000 | ELF32_R_SYM (outrel.r_info),
			    globals->root.sgotplt->contents + offplt
			    + globals->sgotplt_jump_table_size);

		/* Second word in the relocation is always zero.  */
		bfd_put_32 (output_bfd, 0,
			    globals->root.sgotplt->contents + offplt
			    + globals->sgotplt_jump_table_size + 4);
	      }
	    if (tls_type & GOT_TLS_GD)
	      {
		if (need_relocs)
		  {
		    outrel.r_addend = 0;
		    outrel.r_offset = (sgot->output_section->vma
				       + sgot->output_offset
				       + cur_off);
		    outrel.r_info = ELF32_R_INFO (indx, R_ARM_TLS_DTPMOD32);

		    if (globals->use_rel)
		      bfd_put_32 (output_bfd, outrel.r_addend,
				  sgot->contents + cur_off);

		    elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);

		    if (indx == 0)
		      bfd_put_32 (output_bfd, value - dtpoff_base (info),
				  sgot->contents + cur_off + 4);
		    else
		      {
			outrel.r_addend = 0;
			outrel.r_info = ELF32_R_INFO (indx,
						      R_ARM_TLS_DTPOFF32);
			outrel.r_offset += 4;

			if (globals->use_rel)
			  bfd_put_32 (output_bfd, outrel.r_addend,
				      sgot->contents + cur_off + 4);

			elf32_arm_add_dynreloc (output_bfd, info,
						srelgot, &outrel);
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
				sgot->contents + cur_off);
		    bfd_put_32 (output_bfd, value - dtpoff_base (info),
				sgot->contents + cur_off + 4);
		  }

		cur_off += 8;
	      }

	    if (tls_type & GOT_TLS_IE)
	      {
		if (need_relocs)
		  {
		    if (indx == 0)
		      outrel.r_addend = value - dtpoff_base (info);
		    else
		      outrel.r_addend = 0;
		    outrel.r_offset = (sgot->output_section->vma
				       + sgot->output_offset
				       + cur_off);
		    outrel.r_info = ELF32_R_INFO (indx, R_ARM_TLS_TPOFF32);

		    if (globals->use_rel)
		      bfd_put_32 (output_bfd, outrel.r_addend,
				  sgot->contents + cur_off);

		    elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
		  }
		else
		  bfd_put_32 (output_bfd, tpoff (info, value),
			      sgot->contents + cur_off);
		cur_off += 4;
	      }

	    if (h != NULL)
	      h->got.offset |= 1;
	    else
	      local_got_offsets[r_symndx] |= 1;
	  }

	if ((tls_type & GOT_TLS_GD) && r_type != R_ARM_TLS_GD32 && r_type != R_ARM_TLS_GD32_FDPIC)
	  off += 8;
	else if (tls_type & GOT_TLS_GDESC)
	  off = offplt;

	if (ELF32_R_TYPE (rel->r_info) == R_ARM_TLS_CALL
	    || ELF32_R_TYPE (rel->r_info) == R_ARM_THM_TLS_CALL)
	  {
	    bfd_signed_vma offset;
	    /* TLS stubs are arm mode.  The original symbol is a
	       data object, so branch_type is bogus.  */
	    branch_type = ST_BRANCH_TO_ARM;
	    enum elf32_arm_stub_type stub_type
	      = arm_type_of_stub (info, input_section, rel,
				  st_type, &branch_type,
				  (struct elf32_arm_link_hash_entry *)h,
				  globals->tls_trampoline, globals->root.splt,
				  input_bfd, sym_name);

	    if (stub_type != arm_stub_none)
	      {
		struct elf32_arm_stub_hash_entry *stub_entry
		  = elf32_arm_get_stub_entry
		  (input_section, globals->root.splt, 0, rel,
		   globals, stub_type);
		offset = (stub_entry->stub_offset
			  + stub_entry->stub_sec->output_offset
			  + stub_entry->stub_sec->output_section->vma);
	      }
	    else
	      offset = (globals->root.splt->output_section->vma
			+ globals->root.splt->output_offset
			+ globals->tls_trampoline);

	    if (ELF32_R_TYPE (rel->r_info) == R_ARM_TLS_CALL)
	      {
		unsigned long inst;

		offset -= (input_section->output_section->vma
			   + input_section->output_offset
			   + rel->r_offset + 8);

		inst = offset >> 2;
		inst &= 0x00ffffff;
		value = inst | (globals->use_blx ? 0xfa000000 : 0xeb000000);
	      }
	    else
	      {
		/* Thumb blx encodes the offset in a complicated
		   fashion.  */
		unsigned upper_insn, lower_insn;
		unsigned neg;

		offset -= (input_section->output_section->vma
			   + input_section->output_offset
			   + rel->r_offset + 4);

		if (stub_type != arm_stub_none
		    && arm_stub_is_thumb (stub_type))
		  {
		    lower_insn = 0xd000;
		  }
		else
		  {
		    lower_insn = 0xc000;
		    /* Round up the offset to a word boundary.  */
		    offset = (offset + 2) & ~2;
		  }

		neg = offset < 0;
		upper_insn = (0xf000
			      | ((offset >> 12) & 0x3ff)
			      | (neg << 10));
		lower_insn |= (((!((offset >> 23) & 1)) ^ neg) << 13)
			      | (((!((offset >> 22) & 1)) ^ neg) << 11)
			      | ((offset >> 1) & 0x7ff);
		bfd_put_16 (input_bfd, upper_insn, hit_data);
		bfd_put_16 (input_bfd, lower_insn, hit_data + 2);
		return bfd_reloc_ok;
	      }
	  }
	/* These relocations needs special care, as besides the fact
	   they point somewhere in .gotplt, the addend must be
	   adjusted accordingly depending on the type of instruction
	   we refer to.  */
	else if ((r_type == R_ARM_TLS_GOTDESC) && (tls_type & GOT_TLS_GDESC))
	  {
	    unsigned long data, insn;
	    unsigned thumb;

	    data = bfd_get_signed_32 (input_bfd, hit_data);
	    thumb = data & 1;
	    data &= ~1ul;

	    if (thumb)
	      {
		insn = bfd_get_16 (input_bfd, contents + rel->r_offset - data);
		if ((insn & 0xf000) == 0xf000 || (insn & 0xf800) == 0xe800)
		  insn = (insn << 16)
		    | bfd_get_16 (input_bfd,
				  contents + rel->r_offset - data + 2);
		if ((insn & 0xf800c000) == 0xf000c000)
		  /* bl/blx */
		  value = -6;
		else if ((insn & 0xffffff00) == 0x4400)
		  /* add */
		  value = -5;
		else
		  {
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): "
			 "unexpected %s instruction '%#lx' "
			 "referenced by TLS_GOTDESC"),
		       input_bfd, input_section, (uint64_t) rel->r_offset,
		       "Thumb", insn);
		    return bfd_reloc_notsupported;
		  }
	      }
	    else
	      {
		insn = bfd_get_32 (input_bfd, contents + rel->r_offset - data);

		switch (insn >> 24)
		  {
		  case 0xeb:  /* bl */
		  case 0xfa:  /* blx */
		    value = -4;
		    break;

		  case 0xe0:	/* add */
		    value = -8;
		    break;

		  default:
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): "
			 "unexpected %s instruction '%#lx' "
			 "referenced by TLS_GOTDESC"),
		       input_bfd, input_section, (uint64_t) rel->r_offset,
		       "ARM", insn);
		    return bfd_reloc_notsupported;
		  }
	      }

	    value += ((globals->root.sgotplt->output_section->vma
		       + globals->root.sgotplt->output_offset + off)
		      - (input_section->output_section->vma
			 + input_section->output_offset
			 + rel->r_offset)
		      + globals->sgotplt_jump_table_size);
	  }
	else
	  value = ((globals->root.sgot->output_section->vma
		    + globals->root.sgot->output_offset + off)
		   - (input_section->output_section->vma
		      + input_section->output_offset + rel->r_offset));

	if (globals->fdpic_p && (r_type == R_ARM_TLS_GD32_FDPIC ||
				 r_type == R_ARM_TLS_IE32_FDPIC))
	  {
	    /* For FDPIC relocations, resolve to the offset of the GOT
	       entry from the start of GOT.  */
	    bfd_put_32 (output_bfd,
			globals->root.sgot->output_offset + off,
			contents + rel->r_offset);

	    return bfd_reloc_ok;
	  }
	else
	  {
	    return _bfd_final_link_relocate (howto, input_bfd, input_section,
					     contents, rel->r_offset, value,
					     rel->r_addend);
	  }
      }

    case R_ARM_TLS_LE32:
      if (bfd_link_dll (info))
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB(%pA+%#" PRIx64 "): %s relocation not permitted "
	       "in shared object"),
	     input_bfd, input_section, (uint64_t) rel->r_offset, howto->name);
	  return bfd_reloc_notsupported;
	}
      else
	value = tpoff (info, value);

      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, rel->r_offset, value,
				       rel->r_addend);

    case R_ARM_V4BX:
      if (globals->fix_v4bx)
	{
	  bfd_vma insn = bfd_get_32 (input_bfd, hit_data);

	  /* Ensure that we have a BX instruction.  */
	  BFD_ASSERT ((insn & 0x0ffffff0) == 0x012fff10);

	  if (globals->fix_v4bx == 2 && (insn & 0xf) != 0xf)
	    {
	      /* Branch to veneer.  */
	      bfd_vma glue_addr;
	      glue_addr = elf32_arm_bx_glue (info, insn & 0xf);
	      glue_addr -= input_section->output_section->vma
			   + input_section->output_offset
			   + rel->r_offset + 8;
	      insn = (insn & 0xf0000000) | 0x0a000000
		     | ((glue_addr >> 2) & 0x00ffffff);
	    }
	  else
	    {
	      /* Preserve Rm (lowest four bits) and the condition code
		 (highest four bits). Other bits encode MOV PC,Rm.  */
	      insn = (insn & 0xf000000f) | 0x01a0f000;
	    }

	  bfd_put_32 (input_bfd, insn, hit_data);
	}
      return bfd_reloc_ok;

    case R_ARM_MOVW_ABS_NC:
    case R_ARM_MOVT_ABS:
    case R_ARM_MOVW_PREL_NC:
    case R_ARM_MOVT_PREL:
    /* Until we properly support segment-base-relative addressing then
       we assume the segment base to be zero, as for the group relocations.
       Thus R_ARM_MOVW_BREL_NC has the same semantics as R_ARM_MOVW_ABS_NC
       and R_ARM_MOVT_BREL has the same semantics as R_ARM_MOVT_ABS.  */
    case R_ARM_MOVW_BREL_NC:
    case R_ARM_MOVW_BREL:
    case R_ARM_MOVT_BREL:
      {
	bfd_vma insn = bfd_get_32 (input_bfd, hit_data);

	if (globals->use_rel)
	  {
	    addend = ((insn >> 4) & 0xf000) | (insn & 0xfff);
	    signed_addend = (addend ^ 0x8000) - 0x8000;
	  }

	value += signed_addend;

	if (r_type == R_ARM_MOVW_PREL_NC || r_type == R_ARM_MOVT_PREL)
	  value -= (input_section->output_section->vma
		    + input_section->output_offset + rel->r_offset);

	if (r_type == R_ARM_MOVW_BREL && value >= 0x10000)
	  return bfd_reloc_overflow;

	if (branch_type == ST_BRANCH_TO_THUMB)
	  value |= 1;

	if (r_type == R_ARM_MOVT_ABS || r_type == R_ARM_MOVT_PREL
	    || r_type == R_ARM_MOVT_BREL)
	  value >>= 16;

	insn &= 0xfff0f000;
	insn |= value & 0xfff;
	insn |= (value & 0xf000) << 4;
	bfd_put_32 (input_bfd, insn, hit_data);
      }
      return bfd_reloc_ok;

    case R_ARM_THM_MOVW_ABS_NC:
    case R_ARM_THM_MOVT_ABS:
    case R_ARM_THM_MOVW_PREL_NC:
    case R_ARM_THM_MOVT_PREL:
    /* Until we properly support segment-base-relative addressing then
       we assume the segment base to be zero, as for the above relocations.
       Thus R_ARM_THM_MOVW_BREL_NC has the same semantics as
       R_ARM_THM_MOVW_ABS_NC and R_ARM_THM_MOVT_BREL has the same semantics
       as R_ARM_THM_MOVT_ABS.  */
    case R_ARM_THM_MOVW_BREL_NC:
    case R_ARM_THM_MOVW_BREL:
    case R_ARM_THM_MOVT_BREL:
      {
	bfd_vma insn;

	insn = bfd_get_16 (input_bfd, hit_data) << 16;
	insn |= bfd_get_16 (input_bfd, hit_data + 2);

	if (globals->use_rel)
	  {
	    addend = ((insn >> 4)  & 0xf000)
		   | ((insn >> 15) & 0x0800)
		   | ((insn >> 4)  & 0x0700)
		   | (insn	   & 0x00ff);
	    signed_addend = (addend ^ 0x8000) - 0x8000;
	  }

	value += signed_addend;

	if (r_type == R_ARM_THM_MOVW_PREL_NC || r_type == R_ARM_THM_MOVT_PREL)
	  value -= (input_section->output_section->vma
		    + input_section->output_offset + rel->r_offset);

	if (r_type == R_ARM_THM_MOVW_BREL && value >= 0x10000)
	  return bfd_reloc_overflow;

	if (branch_type == ST_BRANCH_TO_THUMB)
	  value |= 1;

	if (r_type == R_ARM_THM_MOVT_ABS || r_type == R_ARM_THM_MOVT_PREL
	    || r_type == R_ARM_THM_MOVT_BREL)
	  value >>= 16;

	insn &= 0xfbf08f00;
	insn |= (value & 0xf000) << 4;
	insn |= (value & 0x0800) << 15;
	insn |= (value & 0x0700) << 4;
	insn |= (value & 0x00ff);

	bfd_put_16 (input_bfd, insn >> 16, hit_data);
	bfd_put_16 (input_bfd, insn & 0xffff, hit_data + 2);
      }
      return bfd_reloc_ok;

    case R_ARM_ALU_PC_G0_NC:
    case R_ARM_ALU_PC_G1_NC:
    case R_ARM_ALU_PC_G0:
    case R_ARM_ALU_PC_G1:
    case R_ARM_ALU_PC_G2:
    case R_ARM_ALU_SB_G0_NC:
    case R_ARM_ALU_SB_G1_NC:
    case R_ARM_ALU_SB_G0:
    case R_ARM_ALU_SB_G1:
    case R_ARM_ALU_SB_G2:
      {
	bfd_vma insn = bfd_get_32 (input_bfd, hit_data);
	bfd_vma pc = input_section->output_section->vma
		     + input_section->output_offset + rel->r_offset;
	/* sb is the origin of the *segment* containing the symbol.  */
	bfd_vma sb = sym_sec ? sym_sec->output_section->vma : 0;
	bfd_vma residual;
	bfd_vma g_n;
	bfd_signed_vma signed_value;
	int group = 0;

	/* Determine which group of bits to select.  */
	switch (r_type)
	  {
	  case R_ARM_ALU_PC_G0_NC:
	  case R_ARM_ALU_PC_G0:
	  case R_ARM_ALU_SB_G0_NC:
	  case R_ARM_ALU_SB_G0:
	    group = 0;
	    break;

	  case R_ARM_ALU_PC_G1_NC:
	  case R_ARM_ALU_PC_G1:
	  case R_ARM_ALU_SB_G1_NC:
	  case R_ARM_ALU_SB_G1:
	    group = 1;
	    break;

	  case R_ARM_ALU_PC_G2:
	  case R_ARM_ALU_SB_G2:
	    group = 2;
	    break;

	  default:
	    abort ();
	  }

	/* If REL, extract the addend from the insn.  If RELA, it will
	   have already been fetched for us.  */
	if (globals->use_rel)
	  {
	    int negative;
	    bfd_vma constant = insn & 0xff;
	    bfd_vma rotation = (insn & 0xf00) >> 8;

	    if (rotation == 0)
	      signed_addend = constant;
	    else
	      {
		/* Compensate for the fact that in the instruction, the
		   rotation is stored in multiples of 2 bits.  */
		rotation *= 2;

		/* Rotate "constant" right by "rotation" bits.  */
		signed_addend = (constant >> rotation) |
				(constant << (8 * sizeof (bfd_vma) - rotation));
	      }

	    /* Determine if the instruction is an ADD or a SUB.
	       (For REL, this determines the sign of the addend.)  */
	    negative = identify_add_or_sub (insn);
	    if (negative == 0)
	      {
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB(%pA+%#" PRIx64 "): only ADD or SUB instructions "
		     "are allowed for ALU group relocations"),
		  input_bfd, input_section, (uint64_t) rel->r_offset);
		return bfd_reloc_overflow;
	      }

	    signed_addend *= negative;
	  }

	/* Compute the value (X) to go in the place.  */
	if (r_type == R_ARM_ALU_PC_G0_NC
	    || r_type == R_ARM_ALU_PC_G1_NC
	    || r_type == R_ARM_ALU_PC_G0
	    || r_type == R_ARM_ALU_PC_G1
	    || r_type == R_ARM_ALU_PC_G2)
	  /* PC relative.  */
	  signed_value = value - pc + signed_addend;
	else
	  /* Section base relative.  */
	  signed_value = value - sb + signed_addend;

	/* If the target symbol is a Thumb function, then set the
	   Thumb bit in the address.  */
	if (branch_type == ST_BRANCH_TO_THUMB)
	  signed_value |= 1;

	/* Calculate the value of the relevant G_n, in encoded
	   constant-with-rotation format.  */
	g_n = calculate_group_reloc_mask (signed_value < 0 ? - signed_value : signed_value,
					  group, &residual);

	/* Check for overflow if required.  */
	if ((r_type == R_ARM_ALU_PC_G0
	     || r_type == R_ARM_ALU_PC_G1
	     || r_type == R_ARM_ALU_PC_G2
	     || r_type == R_ARM_ALU_SB_G0
	     || r_type == R_ARM_ALU_SB_G1
	     || r_type == R_ARM_ALU_SB_G2) && residual != 0)
	  {
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB(%pA+%#" PRIx64 "): overflow whilst "
		 "splitting %#" PRIx64 " for group relocation %s"),
	       input_bfd, input_section, (uint64_t) rel->r_offset,
	       (uint64_t) (signed_value < 0 ? -signed_value : signed_value),
	       howto->name);
	    return bfd_reloc_overflow;
	  }

	/* Mask out the value and the ADD/SUB part of the opcode; take care
	   not to destroy the S bit.  */
	insn &= 0xff1ff000;

	/* Set the opcode according to whether the value to go in the
	   place is negative.  */
	if (signed_value < 0)
	  insn |= 1 << 22;
	else
	  insn |= 1 << 23;

	/* Encode the offset.  */
	insn |= g_n;

	bfd_put_32 (input_bfd, insn, hit_data);
      }
      return bfd_reloc_ok;

    case R_ARM_LDR_PC_G0:
    case R_ARM_LDR_PC_G1:
    case R_ARM_LDR_PC_G2:
    case R_ARM_LDR_SB_G0:
    case R_ARM_LDR_SB_G1:
    case R_ARM_LDR_SB_G2:
      {
	bfd_vma insn = bfd_get_32 (input_bfd, hit_data);
	bfd_vma pc = input_section->output_section->vma
		     + input_section->output_offset + rel->r_offset;
	/* sb is the origin of the *segment* containing the symbol.  */
	bfd_vma sb = sym_sec ? sym_sec->output_section->vma : 0;
	bfd_vma residual;
	bfd_signed_vma signed_value;
	int group = 0;

	/* Determine which groups of bits to calculate.  */
	switch (r_type)
	  {
	  case R_ARM_LDR_PC_G0:
	  case R_ARM_LDR_SB_G0:
	    group = 0;
	    break;

	  case R_ARM_LDR_PC_G1:
	  case R_ARM_LDR_SB_G1:
	    group = 1;
	    break;

	  case R_ARM_LDR_PC_G2:
	  case R_ARM_LDR_SB_G2:
	    group = 2;
	    break;

	  default:
	    abort ();
	  }

	/* If REL, extract the addend from the insn.  If RELA, it will
	   have already been fetched for us.  */
	if (globals->use_rel)
	  {
	    int negative = (insn & (1 << 23)) ? 1 : -1;
	    signed_addend = negative * (insn & 0xfff);
	  }

	/* Compute the value (X) to go in the place.  */
	if (r_type == R_ARM_LDR_PC_G0
	    || r_type == R_ARM_LDR_PC_G1
	    || r_type == R_ARM_LDR_PC_G2)
	  /* PC relative.  */
	  signed_value = value - pc + signed_addend;
	else
	  /* Section base relative.  */
	  signed_value = value - sb + signed_addend;

	/* Calculate the value of the relevant G_{n-1} to obtain
	   the residual at that stage.  */
	calculate_group_reloc_mask (signed_value < 0 ? - signed_value : signed_value,
				    group - 1, &residual);

	/* Check for overflow.  */
	if (residual >= 0x1000)
	  {
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB(%pA+%#" PRIx64 "): overflow whilst "
		 "splitting %#" PRIx64 " for group relocation %s"),
	       input_bfd, input_section, (uint64_t) rel->r_offset,
	       (uint64_t) (signed_value < 0 ? -signed_value : signed_value),
	       howto->name);
	    return bfd_reloc_overflow;
	  }

	/* Mask out the value and U bit.  */
	insn &= 0xff7ff000;

	/* Set the U bit if the value to go in the place is non-negative.  */
	if (signed_value >= 0)
	  insn |= 1 << 23;

	/* Encode the offset.  */
	insn |= residual;

	bfd_put_32 (input_bfd, insn, hit_data);
      }
      return bfd_reloc_ok;

    case R_ARM_LDRS_PC_G0:
    case R_ARM_LDRS_PC_G1:
    case R_ARM_LDRS_PC_G2:
    case R_ARM_LDRS_SB_G0:
    case R_ARM_LDRS_SB_G1:
    case R_ARM_LDRS_SB_G2:
      {
	bfd_vma insn = bfd_get_32 (input_bfd, hit_data);
	bfd_vma pc = input_section->output_section->vma
		     + input_section->output_offset + rel->r_offset;
	/* sb is the origin of the *segment* containing the symbol.  */
	bfd_vma sb = sym_sec ? sym_sec->output_section->vma : 0;
	bfd_vma residual;
	bfd_signed_vma signed_value;
	int group = 0;

	/* Determine which groups of bits to calculate.  */
	switch (r_type)
	  {
	  case R_ARM_LDRS_PC_G0:
	  case R_ARM_LDRS_SB_G0:
	    group = 0;
	    break;

	  case R_ARM_LDRS_PC_G1:
	  case R_ARM_LDRS_SB_G1:
	    group = 1;
	    break;

	  case R_ARM_LDRS_PC_G2:
	  case R_ARM_LDRS_SB_G2:
	    group = 2;
	    break;

	  default:
	    abort ();
	  }

	/* If REL, extract the addend from the insn.  If RELA, it will
	   have already been fetched for us.  */
	if (globals->use_rel)
	  {
	    int negative = (insn & (1 << 23)) ? 1 : -1;
	    signed_addend = negative * (((insn & 0xf00) >> 4) + (insn & 0xf));
	  }

	/* Compute the value (X) to go in the place.  */
	if (r_type == R_ARM_LDRS_PC_G0
	    || r_type == R_ARM_LDRS_PC_G1
	    || r_type == R_ARM_LDRS_PC_G2)
	  /* PC relative.  */
	  signed_value = value - pc + signed_addend;
	else
	  /* Section base relative.  */
	  signed_value = value - sb + signed_addend;

	/* Calculate the value of the relevant G_{n-1} to obtain
	   the residual at that stage.  */
	calculate_group_reloc_mask (signed_value < 0 ? - signed_value : signed_value,
				    group - 1, &residual);

	/* Check for overflow.  */
	if (residual >= 0x100)
	  {
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB(%pA+%#" PRIx64 "): overflow whilst "
		 "splitting %#" PRIx64 " for group relocation %s"),
	       input_bfd, input_section, (uint64_t) rel->r_offset,
	       (uint64_t) (signed_value < 0 ? -signed_value : signed_value),
	       howto->name);
	    return bfd_reloc_overflow;
	  }

	/* Mask out the value and U bit.  */
	insn &= 0xff7ff0f0;

	/* Set the U bit if the value to go in the place is non-negative.  */
	if (signed_value >= 0)
	  insn |= 1 << 23;

	/* Encode the offset.  */
	insn |= ((residual & 0xf0) << 4) | (residual & 0xf);

	bfd_put_32 (input_bfd, insn, hit_data);
      }
      return bfd_reloc_ok;

    case R_ARM_LDC_PC_G0:
    case R_ARM_LDC_PC_G1:
    case R_ARM_LDC_PC_G2:
    case R_ARM_LDC_SB_G0:
    case R_ARM_LDC_SB_G1:
    case R_ARM_LDC_SB_G2:
      {
	bfd_vma insn = bfd_get_32 (input_bfd, hit_data);
	bfd_vma pc = input_section->output_section->vma
		     + input_section->output_offset + rel->r_offset;
	/* sb is the origin of the *segment* containing the symbol.  */
	bfd_vma sb = sym_sec ? sym_sec->output_section->vma : 0;
	bfd_vma residual;
	bfd_signed_vma signed_value;
	int group = 0;

	/* Determine which groups of bits to calculate.  */
	switch (r_type)
	  {
	  case R_ARM_LDC_PC_G0:
	  case R_ARM_LDC_SB_G0:
	    group = 0;
	    break;

	  case R_ARM_LDC_PC_G1:
	  case R_ARM_LDC_SB_G1:
	    group = 1;
	    break;

	  case R_ARM_LDC_PC_G2:
	  case R_ARM_LDC_SB_G2:
	    group = 2;
	    break;

	  default:
	    abort ();
	  }

	/* If REL, extract the addend from the insn.  If RELA, it will
	   have already been fetched for us.  */
	if (globals->use_rel)
	  {
	    int negative = (insn & (1 << 23)) ? 1 : -1;
	    signed_addend = negative * ((insn & 0xff) << 2);
	  }

	/* Compute the value (X) to go in the place.  */
	if (r_type == R_ARM_LDC_PC_G0
	    || r_type == R_ARM_LDC_PC_G1
	    || r_type == R_ARM_LDC_PC_G2)
	  /* PC relative.  */
	  signed_value = value - pc + signed_addend;
	else
	  /* Section base relative.  */
	  signed_value = value - sb + signed_addend;

	/* Calculate the value of the relevant G_{n-1} to obtain
	   the residual at that stage.  */
	calculate_group_reloc_mask (signed_value < 0 ? - signed_value : signed_value,
				    group - 1, &residual);

	/* Check for overflow.  (The absolute value to go in the place must be
	   divisible by four and, after having been divided by four, must
	   fit in eight bits.)  */
	if ((residual & 0x3) != 0 || residual >= 0x400)
	  {
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB(%pA+%#" PRIx64 "): overflow whilst "
		 "splitting %#" PRIx64 " for group relocation %s"),
	       input_bfd, input_section, (uint64_t) rel->r_offset,
	       (uint64_t) (signed_value < 0 ? -signed_value : signed_value),
	       howto->name);
	    return bfd_reloc_overflow;
	  }

	/* Mask out the value and U bit.  */
	insn &= 0xff7fff00;

	/* Set the U bit if the value to go in the place is non-negative.  */
	if (signed_value >= 0)
	  insn |= 1 << 23;

	/* Encode the offset.  */
	insn |= residual >> 2;

	bfd_put_32 (input_bfd, insn, hit_data);
      }
      return bfd_reloc_ok;

    case R_ARM_THM_ALU_ABS_G0_NC:
    case R_ARM_THM_ALU_ABS_G1_NC:
    case R_ARM_THM_ALU_ABS_G2_NC:
    case R_ARM_THM_ALU_ABS_G3_NC:
	{
	    const int shift_array[4] = {0, 8, 16, 24};
	    bfd_vma insn = bfd_get_16 (input_bfd, hit_data);
	    bfd_vma addr = value;
	    int shift = shift_array[r_type - R_ARM_THM_ALU_ABS_G0_NC];

	    /* Compute address.  */
	    if (globals->use_rel)
		signed_addend = insn & 0xff;
	    addr += signed_addend;
	    if (branch_type == ST_BRANCH_TO_THUMB)
		addr |= 1;
	    /* Clean imm8 insn.  */
	    insn &= 0xff00;
	    /* And update with correct part of address.  */
	    insn |= (addr >> shift) & 0xff;
	    /* Update insn.  */
	    bfd_put_16 (input_bfd, insn, hit_data);
	}

	*unresolved_reloc_p = false;
	return bfd_reloc_ok;

    case R_ARM_GOTOFFFUNCDESC:
      {
	if (h == NULL)
	  {
	    struct fdpic_local *local_fdpic_cnts = elf32_arm_local_fdpic_cnts (input_bfd);
	    int dynindx = elf_section_data (sym_sec->output_section)->dynindx;

	    if (r_symndx >= elf32_arm_num_entries (input_bfd))
	      {
		* error_message = _("local symbol index too big");
		return bfd_reloc_dangerous;
	      }

	    int offset = local_fdpic_cnts[r_symndx].funcdesc_offset & ~1;
	    bfd_vma addr = dynreloc_value - sym_sec->output_section->vma;
	    bfd_vma seg = -1;

	    if (bfd_link_pic (info) && dynindx == 0)
	      {
		* error_message = _("no dynamic index information available");
		return bfd_reloc_dangerous;
	      }

	    /* Resolve relocation.  */
	    bfd_put_32 (output_bfd, (offset + sgot->output_offset)
		       , contents + rel->r_offset);
	    /* Emit R_ARM_FUNCDESC_VALUE or two fixups on funcdesc if
	       not done yet.  */
	    arm_elf_fill_funcdesc (output_bfd, info,
				   &local_fdpic_cnts[r_symndx].funcdesc_offset,
				   dynindx, offset, addr, dynreloc_value, seg);
	  }
	else
	  {
	    int dynindx;
	    int offset = eh->fdpic_cnts.funcdesc_offset & ~1;
	    bfd_vma addr;
	    bfd_vma seg = -1;

	    /* For static binaries, sym_sec can be null.  */
	    if (sym_sec)
	      {
		dynindx = elf_section_data (sym_sec->output_section)->dynindx;
		addr = dynreloc_value - sym_sec->output_section->vma;
	      }
	    else
	      {
		dynindx = 0;
		addr = 0;
	      }

	    if (bfd_link_pic (info) && dynindx == 0)
	      {
		* error_message = _("no dynamic index information available");
		return bfd_reloc_dangerous;
	      }

	    /* This case cannot occur since funcdesc is allocated by
	       the dynamic loader so we cannot resolve the relocation.  */
	    if (h->dynindx != -1)
	      {
		* error_message = _("invalid dynamic index");
		return bfd_reloc_dangerous;
	      }

	    /* Resolve relocation.  */
	    bfd_put_32 (output_bfd, (offset + sgot->output_offset),
		        contents + rel->r_offset);
	    /* Emit R_ARM_FUNCDESC_VALUE on funcdesc if not done yet.  */
	    arm_elf_fill_funcdesc (output_bfd, info,
				   &eh->fdpic_cnts.funcdesc_offset,
				   dynindx, offset, addr, dynreloc_value, seg);
	  }
      }
      *unresolved_reloc_p = false;
      return bfd_reloc_ok;

    case R_ARM_GOTFUNCDESC:
      {
	if (h != NULL)
	  {
	    Elf_Internal_Rela outrel;

	    /* Resolve relocation.  */
	    bfd_put_32 (output_bfd, ((eh->fdpic_cnts.gotfuncdesc_offset & ~1)
				     + sgot->output_offset),
			contents + rel->r_offset);
	    /* Add funcdesc and associated R_ARM_FUNCDESC_VALUE.  */
	    if (h->dynindx == -1)
	      {
		int dynindx;
		int offset = eh->fdpic_cnts.funcdesc_offset & ~1;
		bfd_vma addr;
		bfd_vma seg = -1;

		/* For static binaries sym_sec can be null.  */
		if (sym_sec)
		  {
		    dynindx = elf_section_data (sym_sec->output_section)->dynindx;
		    addr = dynreloc_value - sym_sec->output_section->vma;
		  }
		else
		  {
		    dynindx = 0;
		    addr = 0;
		  }

		/* Emit R_ARM_FUNCDESC_VALUE on funcdesc if not done yet.  */
		arm_elf_fill_funcdesc (output_bfd, info,
				       &eh->fdpic_cnts.funcdesc_offset,
				       dynindx, offset, addr, dynreloc_value, seg);
	      }

	    /* Add a dynamic relocation on GOT entry if not already done.  */
	    if ((eh->fdpic_cnts.gotfuncdesc_offset & 1) == 0)
	      {
		if (h->dynindx == -1)
		  {
		    outrel.r_info = ELF32_R_INFO (0, R_ARM_RELATIVE);
		    if (h->root.type == bfd_link_hash_undefweak)
		      bfd_put_32 (output_bfd, 0, sgot->contents
				  + (eh->fdpic_cnts.gotfuncdesc_offset & ~1));
		    else
		      bfd_put_32 (output_bfd, sgot->output_section->vma
				  + sgot->output_offset
				  + (eh->fdpic_cnts.funcdesc_offset & ~1),
				  sgot->contents
				  + (eh->fdpic_cnts.gotfuncdesc_offset & ~1));
		  }
		else
		  {
		    outrel.r_info = ELF32_R_INFO (h->dynindx, R_ARM_FUNCDESC);
		  }
		outrel.r_offset = sgot->output_section->vma
		  + sgot->output_offset
		  + (eh->fdpic_cnts.gotfuncdesc_offset & ~1);
		outrel.r_addend = 0;
		if (h->dynindx == -1 && !bfd_link_pic (info))
		  if (h->root.type == bfd_link_hash_undefweak)
		    arm_elf_add_rofixup (output_bfd, globals->srofixup, -1);
		  else
		    arm_elf_add_rofixup (output_bfd, globals->srofixup,
					 outrel.r_offset);
		else
		  elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
		eh->fdpic_cnts.gotfuncdesc_offset |= 1;
	      }
	  }
	else
	  {
	    /* Such relocation on static function should not have been
	       emitted by the compiler.  */
	    return bfd_reloc_notsupported;
	  }
      }
      *unresolved_reloc_p = false;
      return bfd_reloc_ok;

    case R_ARM_FUNCDESC:
      {
	if (h == NULL)
	  {
	    struct fdpic_local *local_fdpic_cnts = elf32_arm_local_fdpic_cnts (input_bfd);
	    Elf_Internal_Rela outrel;
	    int dynindx = elf_section_data (sym_sec->output_section)->dynindx;

	    if (r_symndx >= elf32_arm_num_entries (input_bfd))
	      {
		* error_message = _("local symbol index too big");
		return bfd_reloc_dangerous;
	      }

	    int offset = local_fdpic_cnts[r_symndx].funcdesc_offset & ~1;
	    bfd_vma addr = dynreloc_value - sym_sec->output_section->vma;
	    bfd_vma seg = -1;

	    if (bfd_link_pic (info) && dynindx == 0)
	      {
		* error_message = _("dynamic index information not available");
		return bfd_reloc_dangerous;
	      }

	    /* Replace static FUNCDESC relocation with a
	       R_ARM_RELATIVE dynamic relocation or with a rofixup for
	       executable.  */
	    outrel.r_info = ELF32_R_INFO (0, R_ARM_RELATIVE);
	    outrel.r_offset = input_section->output_section->vma
	      + input_section->output_offset + rel->r_offset;
	    outrel.r_addend = 0;
	    if (bfd_link_pic (info))
	      elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
	    else
	      arm_elf_add_rofixup (output_bfd, globals->srofixup, outrel.r_offset);

	    bfd_put_32 (input_bfd, sgot->output_section->vma
			+ sgot->output_offset + offset, hit_data);

	    /* Emit R_ARM_FUNCDESC_VALUE on funcdesc if not done yet.  */
	    arm_elf_fill_funcdesc (output_bfd, info,
				   &local_fdpic_cnts[r_symndx].funcdesc_offset,
				   dynindx, offset, addr, dynreloc_value, seg);
	  }
	else
	  {
	    if (h->dynindx == -1)
	      {
		int dynindx;
		int offset = eh->fdpic_cnts.funcdesc_offset & ~1;
		bfd_vma addr;
		bfd_vma seg = -1;
		Elf_Internal_Rela outrel;

		/* For static binaries sym_sec can be null.  */
		if (sym_sec)
		  {
		    dynindx = elf_section_data (sym_sec->output_section)->dynindx;
		    addr = dynreloc_value - sym_sec->output_section->vma;
		  }
		else
		  {
		    dynindx = 0;
		    addr = 0;
		  }

		if (bfd_link_pic (info) && dynindx == 0)
		  abort ();

		/* Replace static FUNCDESC relocation with a
		   R_ARM_RELATIVE dynamic relocation.  */
		outrel.r_info = ELF32_R_INFO (0, R_ARM_RELATIVE);
		outrel.r_offset = input_section->output_section->vma
		  + input_section->output_offset + rel->r_offset;
		outrel.r_addend = 0;
		if (bfd_link_pic (info))
		  elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
		else
		  arm_elf_add_rofixup (output_bfd, globals->srofixup, outrel.r_offset);

		bfd_put_32 (input_bfd, sgot->output_section->vma
			    + sgot->output_offset + offset, hit_data);

		/* Emit R_ARM_FUNCDESC_VALUE on funcdesc if not done yet.  */
		arm_elf_fill_funcdesc (output_bfd, info,
				       &eh->fdpic_cnts.funcdesc_offset,
				       dynindx, offset, addr, dynreloc_value, seg);
	      }
	    else
	      {
		Elf_Internal_Rela outrel;

		/* Add a dynamic relocation.  */
		outrel.r_info = ELF32_R_INFO (h->dynindx, R_ARM_FUNCDESC);
		outrel.r_offset = input_section->output_section->vma
		  + input_section->output_offset + rel->r_offset;
		outrel.r_addend = 0;
		elf32_arm_add_dynreloc (output_bfd, info, srelgot, &outrel);
	      }
	  }
      }
      *unresolved_reloc_p = false;
      return bfd_reloc_ok;

    case R_ARM_THM_BF16:
      {
	bfd_vma relocation;
	bfd_vma upper_insn = bfd_get_16 (input_bfd, hit_data);
	bfd_vma lower_insn = bfd_get_16 (input_bfd, hit_data + 2);

	if (globals->use_rel)
	  {
	    bfd_vma immA  = (upper_insn & 0x001f);
	    bfd_vma immB  = (lower_insn & 0x07fe) >> 1;
	    bfd_vma immC  = (lower_insn & 0x0800) >> 11;
	    addend  = (immA << 12);
	    addend |= (immB << 2);
	    addend |= (immC << 1);
	    addend |= 1;
	    /* Sign extend.  */
	    signed_addend = (addend & 0x10000) ? addend - (1 << 17) : addend;
	  }

	relocation  = value + signed_addend;
	relocation -= (input_section->output_section->vma
		       + input_section->output_offset
		       + rel->r_offset);

	/* Put RELOCATION back into the insn.  */
	{
	  bfd_vma immA = (relocation & 0x0001f000) >> 12;
	  bfd_vma immB = (relocation & 0x00000ffc) >> 2;
	  bfd_vma immC = (relocation & 0x00000002) >> 1;

	  upper_insn = (upper_insn & 0xffe0) | immA;
	  lower_insn = (lower_insn & 0xf001) | (immC << 11) | (immB << 1);
	}

	/* Put the relocated value back in the object file:  */
	bfd_put_16 (input_bfd, upper_insn, hit_data);
	bfd_put_16 (input_bfd, lower_insn, hit_data + 2);

	return bfd_reloc_ok;
      }

    case R_ARM_THM_BF12:
      {
	bfd_vma relocation;
	bfd_vma upper_insn = bfd_get_16 (input_bfd, hit_data);
	bfd_vma lower_insn = bfd_get_16 (input_bfd, hit_data + 2);

	if (globals->use_rel)
	  {
	    bfd_vma immA  = (upper_insn & 0x0001);
	    bfd_vma immB  = (lower_insn & 0x07fe) >> 1;
	    bfd_vma immC  = (lower_insn & 0x0800) >> 11;
	    addend  = (immA << 12);
	    addend |= (immB << 2);
	    addend |= (immC << 1);
	    addend |= 1;
	    /* Sign extend.  */
	    addend = (addend & 0x1000) ? addend - (1 << 13) : addend;
	    signed_addend = addend;
	  }

	relocation  = value + signed_addend;
	relocation -= (input_section->output_section->vma
		       + input_section->output_offset
		       + rel->r_offset);

	/* Put RELOCATION back into the insn.  */
	{
	  bfd_vma immA = (relocation & 0x00001000) >> 12;
	  bfd_vma immB = (relocation & 0x00000ffc) >> 2;
	  bfd_vma immC = (relocation & 0x00000002) >> 1;

	  upper_insn = (upper_insn & 0xfffe) | immA;
	  lower_insn = (lower_insn & 0xf001) | (immC << 11) | (immB << 1);
	}

	/* Put the relocated value back in the object file:  */
	bfd_put_16 (input_bfd, upper_insn, hit_data);
	bfd_put_16 (input_bfd, lower_insn, hit_data + 2);

	return bfd_reloc_ok;
      }

    case R_ARM_THM_BF18:
      {
	bfd_vma relocation;
	bfd_vma upper_insn = bfd_get_16 (input_bfd, hit_data);
	bfd_vma lower_insn = bfd_get_16 (input_bfd, hit_data + 2);

	if (globals->use_rel)
	  {
	    bfd_vma immA  = (upper_insn & 0x007f);
	    bfd_vma immB  = (lower_insn & 0x07fe) >> 1;
	    bfd_vma immC  = (lower_insn & 0x0800) >> 11;
	    addend  = (immA << 12);
	    addend |= (immB << 2);
	    addend |= (immC << 1);
	    addend |= 1;
	    /* Sign extend.  */
	    addend = (addend & 0x40000) ? addend - (1 << 19) : addend;
	    signed_addend = addend;
	  }

	relocation  = value + signed_addend;
	relocation -= (input_section->output_section->vma
		       + input_section->output_offset
		       + rel->r_offset);

	/* Put RELOCATION back into the insn.  */
	{
	  bfd_vma immA = (relocation & 0x0007f000) >> 12;
	  bfd_vma immB = (relocation & 0x00000ffc) >> 2;
	  bfd_vma immC = (relocation & 0x00000002) >> 1;

	  upper_insn = (upper_insn & 0xff80) | immA;
	  lower_insn = (lower_insn & 0xf001) | (immC << 11) | (immB << 1);
	}

	/* Put the relocated value back in the object file:  */
	bfd_put_16 (input_bfd, upper_insn, hit_data);
	bfd_put_16 (input_bfd, lower_insn, hit_data + 2);

	return bfd_reloc_ok;
      }

    default:
      return bfd_reloc_notsupported;
    }
}

/* Add INCREMENT to the reloc (of type HOWTO) at ADDRESS.  */
static void
arm_add_to_rel (bfd *		   abfd,
		bfd_byte *	   address,
		reloc_howto_type * howto,
		bfd_signed_vma	   increment)
{
  bfd_signed_vma addend;

  if (howto->type == R_ARM_THM_CALL
      || howto->type == R_ARM_THM_JUMP24)
    {
      int upper_insn, lower_insn;
      int upper, lower;

      upper_insn = bfd_get_16 (abfd, address);
      lower_insn = bfd_get_16 (abfd, address + 2);
      upper = upper_insn & 0x7ff;
      lower = lower_insn & 0x7ff;

      addend = (upper << 12) | (lower << 1);
      addend += increment;
      addend >>= 1;

      upper_insn = (upper_insn & 0xf800) | ((addend >> 11) & 0x7ff);
      lower_insn = (lower_insn & 0xf800) | (addend & 0x7ff);

      bfd_put_16 (abfd, (bfd_vma) upper_insn, address);
      bfd_put_16 (abfd, (bfd_vma) lower_insn, address + 2);
    }
  else
    {
      bfd_vma	     contents;

      contents = bfd_get_32 (abfd, address);

      /* Get the (signed) value from the instruction.  */
      addend = contents & howto->src_mask;
      if (addend & ((howto->src_mask + 1) >> 1))
	{
	  bfd_signed_vma mask;

	  mask = -1;
	  mask &= ~ howto->src_mask;
	  addend |= mask;
	}

      /* Add in the increment, (which is a byte value).  */
      switch (howto->type)
	{
	default:
	  addend += increment;
	  break;

	case R_ARM_PC24:
	case R_ARM_PLT32:
	case R_ARM_CALL:
	case R_ARM_JUMP24:
	  addend *= bfd_get_reloc_size (howto);
	  addend += increment;

	  /* Should we check for overflow here ?  */

	  /* Drop any undesired bits.  */
	  addend >>= howto->rightshift;
	  break;
	}

      contents = (contents & ~ howto->dst_mask) | (addend & howto->dst_mask);

      bfd_put_32 (abfd, contents, address);
    }
}

#define IS_ARM_TLS_RELOC(R_TYPE)	\
  ((R_TYPE) == R_ARM_TLS_GD32		\
   || (R_TYPE) == R_ARM_TLS_GD32_FDPIC  \
   || (R_TYPE) == R_ARM_TLS_LDO32	\
   || (R_TYPE) == R_ARM_TLS_LDM32	\
   || (R_TYPE) == R_ARM_TLS_LDM32_FDPIC	\
   || (R_TYPE) == R_ARM_TLS_DTPOFF32	\
   || (R_TYPE) == R_ARM_TLS_DTPMOD32	\
   || (R_TYPE) == R_ARM_TLS_TPOFF32	\
   || (R_TYPE) == R_ARM_TLS_LE32	\
   || (R_TYPE) == R_ARM_TLS_IE32	\
   || (R_TYPE) == R_ARM_TLS_IE32_FDPIC	\
   || IS_ARM_TLS_GNU_RELOC (R_TYPE))

/* Specific set of relocations for the gnu tls dialect.  */
#define IS_ARM_TLS_GNU_RELOC(R_TYPE)	\
  ((R_TYPE) == R_ARM_TLS_GOTDESC	\
   || (R_TYPE) == R_ARM_TLS_CALL	\
   || (R_TYPE) == R_ARM_THM_TLS_CALL	\
   || (R_TYPE) == R_ARM_TLS_DESCSEQ	\
   || (R_TYPE) == R_ARM_THM_TLS_DESCSEQ)

/* Relocate an ARM ELF section.  */

static int
elf32_arm_relocate_section (bfd *		   output_bfd,
			    struct bfd_link_info * info,
			    bfd *		   input_bfd,
			    asection *		   input_section,
			    bfd_byte *		   contents,
			    Elf_Internal_Rela *	   relocs,
			    Elf_Internal_Sym *	   local_syms,
			    asection **		   local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  const char *name;
  struct elf32_arm_link_hash_table * globals;

  globals = elf32_arm_hash_table (info);
  if (globals == NULL)
    return false;

  symtab_hdr = & elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int			   r_type;
      reloc_howto_type *	   howto;
      unsigned long		   r_symndx;
      Elf_Internal_Sym *	   sym;
      asection *		   sec;
      struct elf_link_hash_entry * h;
      bfd_vma			   relocation;
      bfd_reloc_status_type	   r;
      arelent			   bfd_reloc;
      char			   sym_type;
      bool			   unresolved_reloc = false;
      char *error_message = NULL;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type   = ELF32_R_TYPE (rel->r_info);
      r_type   = arm_real_reloc_type (globals, r_type);

      if (   r_type == R_ARM_GNU_VTENTRY
	  || r_type == R_ARM_GNU_VTINHERIT)
	continue;

      howto = bfd_reloc.howto = elf32_arm_howto_from_type (r_type);

      if (howto == NULL)
	return _bfd_unrecognized_reloc (input_bfd, input_section, r_type);

      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sym_type = ELF32_ST_TYPE (sym->st_info);
	  sec = local_sections[r_symndx];

	  /* An object file might have a reference to a local
	     undefined symbol.  This is a daft object file, but we
	     should at least do something about it.  V4BX & NONE
	     relocations do not use the symbol and are explicitly
	     allowed to use the undefined symbol, so allow those.
	     Likewise for relocations against STN_UNDEF.  */
	  if (r_type != R_ARM_V4BX
	      && r_type != R_ARM_NONE
	      && r_symndx != STN_UNDEF
	      && bfd_is_und_section (sec)
	      && ELF_ST_BIND (sym->st_info) != STB_WEAK)
	    (*info->callbacks->undefined_symbol)
	      (info, bfd_elf_string_from_elf_section
	       (input_bfd, symtab_hdr->sh_link, sym->st_name),
	       input_bfd, input_section,
	       rel->r_offset, true);

	  if (globals->use_rel)
	    {
	      relocation = (sec->output_section->vma
			    + sec->output_offset
			    + sym->st_value);
	      if (!bfd_link_relocatable (info)
		  && (sec->flags & SEC_MERGE)
		  && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		{
		  asection *msec;
		  bfd_vma addend, value;

		  switch (r_type)
		    {
		    case R_ARM_MOVW_ABS_NC:
		    case R_ARM_MOVT_ABS:
		      value = bfd_get_32 (input_bfd, contents + rel->r_offset);
		      addend = ((value & 0xf0000) >> 4) | (value & 0xfff);
		      addend = (addend ^ 0x8000) - 0x8000;
		      break;

		    case R_ARM_THM_MOVW_ABS_NC:
		    case R_ARM_THM_MOVT_ABS:
		      value = bfd_get_16 (input_bfd, contents + rel->r_offset)
			      << 16;
		      value |= bfd_get_16 (input_bfd,
					   contents + rel->r_offset + 2);
		      addend = ((value & 0xf7000) >> 4) | (value & 0xff)
			       | ((value & 0x04000000) >> 15);
		      addend = (addend ^ 0x8000) - 0x8000;
		      break;

		    default:
		      if (howto->rightshift
			  || (howto->src_mask & (howto->src_mask + 1)))
			{
			  _bfd_error_handler
			    /* xgettext:c-format */
			    (_("%pB(%pA+%#" PRIx64 "): "
			       "%s relocation against SEC_MERGE section"),
			     input_bfd, input_section,
			     (uint64_t) rel->r_offset, howto->name);
			  return false;
			}

		      value = bfd_get_32 (input_bfd, contents + rel->r_offset);

		      /* Get the (signed) value from the instruction.  */
		      addend = value & howto->src_mask;
		      if (addend & ((howto->src_mask + 1) >> 1))
			{
			  bfd_signed_vma mask;

			  mask = -1;
			  mask &= ~ howto->src_mask;
			  addend |= mask;
			}
		      break;
		    }

		  msec = sec;
		  addend =
		    _bfd_elf_rel_local_sym (output_bfd, sym, &msec, addend)
		    - relocation;
		  addend += msec->output_section->vma + msec->output_offset;

		  /* Cases here must match those in the preceding
		     switch statement.  */
		  switch (r_type)
		    {
		    case R_ARM_MOVW_ABS_NC:
		    case R_ARM_MOVT_ABS:
		      value = (value & 0xfff0f000) | ((addend & 0xf000) << 4)
			      | (addend & 0xfff);
		      bfd_put_32 (input_bfd, value, contents + rel->r_offset);
		      break;

		    case R_ARM_THM_MOVW_ABS_NC:
		    case R_ARM_THM_MOVT_ABS:
		      value = (value & 0xfbf08f00) | ((addend & 0xf700) << 4)
			      | (addend & 0xff) | ((addend & 0x0800) << 15);
		      bfd_put_16 (input_bfd, value >> 16,
				  contents + rel->r_offset);
		      bfd_put_16 (input_bfd, value,
				  contents + rel->r_offset + 2);
		      break;

		    default:
		      value = (value & ~ howto->dst_mask)
			      | (addend & howto->dst_mask);
		      bfd_put_32 (input_bfd, value, contents + rel->r_offset);
		      break;
		    }
		}
	    }
	  else
	    relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  sym_type = h->type;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	{
	  /* This is a relocatable link.  We don't have to change
	     anything, unless the reloc is against a section symbol,
	     in which case we have to adjust according to where the
	     section symbol winds up in the output section.  */
	  if (sym != NULL && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    {
	      if (globals->use_rel)
		arm_add_to_rel (input_bfd, contents + rel->r_offset,
				howto, (bfd_signed_vma) sec->output_offset);
	      else
		rel->r_addend += sec->output_offset;
	    }
	  continue;
	}

      if (h != NULL)
	name = h->root.root.string;
      else
	{
	  name = (bfd_elf_string_from_elf_section
		  (input_bfd, symtab_hdr->sh_link, sym->st_name));
	  if (name == NULL || *name == '\0')
	    name = bfd_section_name (sec);
	}

      if (r_symndx != STN_UNDEF
	  && r_type != R_ARM_NONE
	  && (h == NULL
	      || h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	  && IS_ARM_TLS_RELOC (r_type) != (sym_type == STT_TLS))
	{
	  _bfd_error_handler
	    ((sym_type == STT_TLS
	      /* xgettext:c-format */
	      ? _("%pB(%pA+%#" PRIx64 "): %s used with TLS symbol %s")
	      /* xgettext:c-format */
	      : _("%pB(%pA+%#" PRIx64 "): %s used with non-TLS symbol %s")),
	     input_bfd,
	     input_section,
	     (uint64_t) rel->r_offset,
	     howto->name,
	     name);
	}

      /* We call elf32_arm_final_link_relocate unless we're completely
	 done, i.e., the relaxation produced the final output we want,
	 and we won't let anybody mess with it. Also, we have to do
	 addend adjustments in case of a R_ARM_TLS_GOTDESC relocation
	 both in relaxed and non-relaxed cases.  */
      if ((elf32_arm_tls_transition (info, r_type, h) != (unsigned)r_type)
	  || (IS_ARM_TLS_GNU_RELOC (r_type)
	      && !((h ? elf32_arm_hash_entry (h)->tls_type :
		    elf32_arm_local_got_tls_type (input_bfd)[r_symndx])
		   & GOT_TLS_GDESC)))
	{
	  r = elf32_arm_tls_relax (globals, input_bfd, input_section,
				   contents, rel, h == NULL);
	  /* This may have been marked unresolved because it came from
	     a shared library.  But we've just dealt with that.  */
	  unresolved_reloc = 0;
	}
      else
	r = bfd_reloc_continue;

      if (r == bfd_reloc_continue)
	{
	  unsigned char branch_type =
	    h ? ARM_GET_SYM_BRANCH_TYPE (h->target_internal)
	      : ARM_GET_SYM_BRANCH_TYPE (sym->st_target_internal);

	  r = elf32_arm_final_link_relocate (howto, input_bfd, output_bfd,
					     input_section, contents, rel,
					     relocation, info, sec, name,
					     sym_type, branch_type, h,
					     &unresolved_reloc,
					     &error_message);
	}

      /* Dynamic relocs are not propagated for SEC_DEBUGGING sections
	 because such sections are not SEC_ALLOC and thus ld.so will
	 not process them.  */
      if (unresolved_reloc
	  && !((input_section->flags & SEC_DEBUGGING) != 0
	       && h->def_dynamic)
	  && _bfd_elf_section_offset (output_bfd, info, input_section,
				      rel->r_offset) != (bfd_vma) -1)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB(%pA+%#" PRIx64 "): "
	       "unresolvable %s relocation against symbol `%s'"),
	     input_bfd,
	     input_section,
	     (uint64_t) rel->r_offset,
	     howto->name,
	     h->root.root.string);
	  return false;
	}

      if (r != bfd_reloc_ok)
	{
	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      /* If the overflowing reloc was to an undefined symbol,
		 we have already printed one error message and there
		 is no point complaining again.  */
	      if (!h || h->root.type != bfd_link_hash_undefined)
		(*info->callbacks->reloc_overflow)
		  (info, (h ? &h->root : NULL), name, howto->name,
		   (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      error_message = _("out of range");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      error_message = _("unsupported relocation");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      /* error_message should already be set.  */
	      goto common_error;

	    default:
	      error_message = _("unknown error");
	      /* Fall through.  */

	    common_error:
	      BFD_ASSERT (error_message != NULL);
	      (*info->callbacks->reloc_dangerous)
		(info, error_message, input_bfd, input_section, rel->r_offset);
	      break;
	    }
	}
    }

  return true;
}

/* Add a new unwind edit to the list described by HEAD, TAIL.  If TINDEX is zero,
   adds the edit to the start of the list.  (The list must be built in order of
   ascending TINDEX: the function's callers are primarily responsible for
   maintaining that condition).  */

static void
add_unwind_table_edit (arm_unwind_table_edit **head,
		       arm_unwind_table_edit **tail,
		       arm_unwind_edit_type type,
		       asection *linked_section,
		       unsigned int tindex)
{
  arm_unwind_table_edit *new_edit = (arm_unwind_table_edit *)
      xmalloc (sizeof (arm_unwind_table_edit));

  new_edit->type = type;
  new_edit->linked_section = linked_section;
  new_edit->index = tindex;

  if (tindex > 0)
    {
      new_edit->next = NULL;

      if (*tail)
	(*tail)->next = new_edit;

      (*tail) = new_edit;

      if (!*head)
	(*head) = new_edit;
    }
  else
    {
      new_edit->next = *head;

      if (!*tail)
	*tail = new_edit;

      *head = new_edit;
    }
}

static _arm_elf_section_data *get_arm_elf_section_data (asection *);

/* Increase the size of EXIDX_SEC by ADJUST bytes.  ADJUST mau be negative.  */

static void
adjust_exidx_size (asection *exidx_sec, int adjust)
{
  asection *out_sec;

  if (!exidx_sec->rawsize)
    exidx_sec->rawsize = exidx_sec->size;

  bfd_set_section_size (exidx_sec, exidx_sec->size + adjust);
  out_sec = exidx_sec->output_section;
  /* Adjust size of output section.  */
  bfd_set_section_size (out_sec, out_sec->size + adjust);
}

/* Insert an EXIDX_CANTUNWIND marker at the end of a section.  */

static void
insert_cantunwind_after (asection *text_sec, asection *exidx_sec)
{
  struct _arm_elf_section_data *exidx_arm_data;

  exidx_arm_data = get_arm_elf_section_data (exidx_sec);
  add_unwind_table_edit
    (&exidx_arm_data->u.exidx.unwind_edit_list,
     &exidx_arm_data->u.exidx.unwind_edit_tail,
     INSERT_EXIDX_CANTUNWIND_AT_END, text_sec, UINT_MAX);

  exidx_arm_data->additional_reloc_count++;

  adjust_exidx_size (exidx_sec, 8);
}

/* Scan .ARM.exidx tables, and create a list describing edits which should be
   made to those tables, such that:

     1. Regions without unwind data are marked with EXIDX_CANTUNWIND entries.
     2. Duplicate entries are merged together (EXIDX_CANTUNWIND, or unwind
	codes which have been inlined into the index).

   If MERGE_EXIDX_ENTRIES is false, duplicate entries are not merged.

   The edits are applied when the tables are written
   (in elf32_arm_write_section).  */

bool
elf32_arm_fix_exidx_coverage (asection **text_section_order,
			      unsigned int num_text_sections,
			      struct bfd_link_info *info,
			      bool merge_exidx_entries)
{
  bfd *inp;
  unsigned int last_second_word = 0, i;
  asection *last_exidx_sec = NULL;
  asection *last_text_sec = NULL;
  int last_unwind_type = -1;

  /* Walk over all EXIDX sections, and create backlinks from the corrsponding
     text sections.  */
  for (inp = info->input_bfds; inp != NULL; inp = inp->link.next)
    {
      asection *sec;

      for (sec = inp->sections; sec != NULL; sec = sec->next)
	{
	  struct bfd_elf_section_data *elf_sec = elf_section_data (sec);
	  Elf_Internal_Shdr *hdr = &elf_sec->this_hdr;

	  if (!hdr || hdr->sh_type != SHT_ARM_EXIDX)
	    continue;

	  if (elf_sec->linked_to)
	    {
	      Elf_Internal_Shdr *linked_hdr
		= &elf_section_data (elf_sec->linked_to)->this_hdr;
	      struct _arm_elf_section_data *linked_sec_arm_data
		= get_arm_elf_section_data (linked_hdr->bfd_section);

	      if (linked_sec_arm_data == NULL)
		continue;

	      /* Link this .ARM.exidx section back from the text section it
		 describes.  */
	      linked_sec_arm_data->u.text.arm_exidx_sec = sec;
	    }
	}
    }

  /* Walk all text sections in order of increasing VMA.  Eilminate duplicate
     index table entries (EXIDX_CANTUNWIND and inlined unwind opcodes),
     and add EXIDX_CANTUNWIND entries for sections with no unwind table data.  */

  for (i = 0; i < num_text_sections; i++)
    {
      asection *sec = text_section_order[i];
      asection *exidx_sec;
      struct _arm_elf_section_data *arm_data = get_arm_elf_section_data (sec);
      struct _arm_elf_section_data *exidx_arm_data;
      bfd_byte *contents = NULL;
      int deleted_exidx_bytes = 0;
      bfd_vma j;
      arm_unwind_table_edit *unwind_edit_head = NULL;
      arm_unwind_table_edit *unwind_edit_tail = NULL;
      Elf_Internal_Shdr *hdr;
      bfd *ibfd;

      if (arm_data == NULL)
	continue;

      exidx_sec = arm_data->u.text.arm_exidx_sec;
      if (exidx_sec == NULL)
	{
	  /* Section has no unwind data.  */
	  if (last_unwind_type == 0 || !last_exidx_sec)
	    continue;

	  /* Ignore zero sized sections.  */
	  if (sec->size == 0)
	    continue;

	  insert_cantunwind_after (last_text_sec, last_exidx_sec);
	  last_unwind_type = 0;
	  continue;
	}

      /* Skip /DISCARD/ sections.  */
      if (bfd_is_abs_section (exidx_sec->output_section))
	continue;

      hdr = &elf_section_data (exidx_sec)->this_hdr;
      if (hdr->sh_type != SHT_ARM_EXIDX)
	continue;

      exidx_arm_data = get_arm_elf_section_data (exidx_sec);
      if (exidx_arm_data == NULL)
	continue;

      ibfd = exidx_sec->owner;

      if (hdr->contents != NULL)
	contents = hdr->contents;
      else if (! bfd_malloc_and_get_section (ibfd, exidx_sec, &contents))
	/* An error?  */
	continue;

      if (last_unwind_type > 0)
	{
	  unsigned int first_word = bfd_get_32 (ibfd, contents);
	  /* Add cantunwind if first unwind item does not match section
	     start.  */
	  if (first_word != sec->vma)
	    {
	      insert_cantunwind_after (last_text_sec, last_exidx_sec);
	      last_unwind_type = 0;
	    }
	}

      for (j = 0; j < hdr->sh_size; j += 8)
	{
	  unsigned int second_word = bfd_get_32 (ibfd, contents + j + 4);
	  int unwind_type;
	  int elide = 0;

	  /* An EXIDX_CANTUNWIND entry.  */
	  if (second_word == 1)
	    {
	      if (last_unwind_type == 0)
		elide = 1;
	      unwind_type = 0;
	    }
	  /* Inlined unwinding data.  Merge if equal to previous.  */
	  else if ((second_word & 0x80000000) != 0)
	    {
	      if (merge_exidx_entries
		   && last_second_word == second_word && last_unwind_type == 1)
		elide = 1;
	      unwind_type = 1;
	      last_second_word = second_word;
	    }
	  /* Normal table entry.  In theory we could merge these too,
	     but duplicate entries are likely to be much less common.  */
	  else
	    unwind_type = 2;

	  if (elide && !bfd_link_relocatable (info))
	    {
	      add_unwind_table_edit (&unwind_edit_head, &unwind_edit_tail,
				     DELETE_EXIDX_ENTRY, NULL, j / 8);

	      deleted_exidx_bytes += 8;
	    }

	  last_unwind_type = unwind_type;
	}

      /* Free contents if we allocated it ourselves.  */
      if (contents != hdr->contents)
	free (contents);

      /* Record edits to be applied later (in elf32_arm_write_section).  */
      exidx_arm_data->u.exidx.unwind_edit_list = unwind_edit_head;
      exidx_arm_data->u.exidx.unwind_edit_tail = unwind_edit_tail;

      if (deleted_exidx_bytes > 0)
	adjust_exidx_size (exidx_sec, - deleted_exidx_bytes);

      last_exidx_sec = exidx_sec;
      last_text_sec = sec;
    }

  /* Add terminating CANTUNWIND entry.  */
  if (!bfd_link_relocatable (info) && last_exidx_sec
      && last_unwind_type != 0)
    insert_cantunwind_after (last_text_sec, last_exidx_sec);

  return true;
}

static bool
elf32_arm_output_glue_section (struct bfd_link_info *info, bfd *obfd,
			       bfd *ibfd, const char *name)
{
  asection *sec, *osec;

  sec = bfd_get_linker_section (ibfd, name);
  if (sec == NULL || (sec->flags & SEC_EXCLUDE) != 0)
    return true;

  osec = sec->output_section;
  if (elf32_arm_write_section (obfd, info, sec, sec->contents))
    return true;

  if (! bfd_set_section_contents (obfd, osec, sec->contents,
				  sec->output_offset, sec->size))
    return false;

  return true;
}

static bool
elf32_arm_final_link (bfd *abfd, struct bfd_link_info *info)
{
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (info);
  asection *sec, *osec;

  if (globals == NULL)
    return false;

  /* Invoke the regular ELF backend linker to do all the work.  */
  if (!bfd_elf_final_link (abfd, info))
    return false;

  /* Process stub sections (eg BE8 encoding, ...).  */
  struct elf32_arm_link_hash_table *htab = elf32_arm_hash_table (info);
  unsigned int i;
  for (i=0; i<htab->top_id; i++)
    {
      sec = htab->stub_group[i].stub_sec;
      /* Only process it once, in its link_sec slot.  */
      if (sec && i == htab->stub_group[i].link_sec->id)
	{
	  osec = sec->output_section;
	  elf32_arm_write_section (abfd, info, sec, sec->contents);
	  if (! bfd_set_section_contents (abfd, osec, sec->contents,
					  sec->output_offset, sec->size))
	    return false;
	}
    }

  /* Write out any glue sections now that we have created all the
     stubs.  */
  if (globals->bfd_of_glue_owner != NULL)
    {
      if (! elf32_arm_output_glue_section (info, abfd,
					   globals->bfd_of_glue_owner,
					   ARM2THUMB_GLUE_SECTION_NAME))
	return false;

      if (! elf32_arm_output_glue_section (info, abfd,
					   globals->bfd_of_glue_owner,
					   THUMB2ARM_GLUE_SECTION_NAME))
	return false;

      if (! elf32_arm_output_glue_section (info, abfd,
					   globals->bfd_of_glue_owner,
					   VFP11_ERRATUM_VENEER_SECTION_NAME))
	return false;

      if (! elf32_arm_output_glue_section (info, abfd,
					   globals->bfd_of_glue_owner,
					   STM32L4XX_ERRATUM_VENEER_SECTION_NAME))
	return false;

      if (! elf32_arm_output_glue_section (info, abfd,
					   globals->bfd_of_glue_owner,
					   ARM_BX_GLUE_SECTION_NAME))
	return false;
    }

  return true;
}

/* Return a best guess for the machine number based on the attributes.  */

static unsigned int
bfd_arm_get_mach_from_attributes (bfd * abfd)
{
  int arch = bfd_elf_get_obj_attr_int (abfd, OBJ_ATTR_PROC, Tag_CPU_arch);

  switch (arch)
    {
    case TAG_CPU_ARCH_PRE_V4: return bfd_mach_arm_3M;
    case TAG_CPU_ARCH_V4: return bfd_mach_arm_4;
    case TAG_CPU_ARCH_V4T: return bfd_mach_arm_4T;
    case TAG_CPU_ARCH_V5T: return bfd_mach_arm_5T;

    case TAG_CPU_ARCH_V5TE:
      {
	char * name;

	BFD_ASSERT (Tag_CPU_name < NUM_KNOWN_OBJ_ATTRIBUTES);
	name = elf_known_obj_attributes (abfd) [OBJ_ATTR_PROC][Tag_CPU_name].s;

	if (name)
	  {
	    if (strcmp (name, "IWMMXT2") == 0)
	      return bfd_mach_arm_iWMMXt2;

	    if (strcmp (name, "IWMMXT") == 0)
	      return bfd_mach_arm_iWMMXt;

	    if (strcmp (name, "XSCALE") == 0)
	      {
		int wmmx;

		BFD_ASSERT (Tag_WMMX_arch < NUM_KNOWN_OBJ_ATTRIBUTES);
		wmmx = elf_known_obj_attributes (abfd) [OBJ_ATTR_PROC][Tag_WMMX_arch].i;
		switch (wmmx)
		  {
		  case 1: return bfd_mach_arm_iWMMXt;
		  case 2: return bfd_mach_arm_iWMMXt2;
		  default: return bfd_mach_arm_XScale;
		  }
	      }
	  }

	return bfd_mach_arm_5TE;
      }

    case TAG_CPU_ARCH_V5TEJ:
	return bfd_mach_arm_5TEJ;
    case TAG_CPU_ARCH_V6:
	return bfd_mach_arm_6;
    case TAG_CPU_ARCH_V6KZ:
	return bfd_mach_arm_6KZ;
    case TAG_CPU_ARCH_V6T2:
	return bfd_mach_arm_6T2;
    case TAG_CPU_ARCH_V6K:
	return bfd_mach_arm_6K;
    case TAG_CPU_ARCH_V7:
	return bfd_mach_arm_7;
    case TAG_CPU_ARCH_V6_M:
	return bfd_mach_arm_6M;
    case TAG_CPU_ARCH_V6S_M:
	return bfd_mach_arm_6SM;
    case TAG_CPU_ARCH_V7E_M:
	return bfd_mach_arm_7EM;
    case TAG_CPU_ARCH_V8:
	return bfd_mach_arm_8;
    case TAG_CPU_ARCH_V8R:
	return bfd_mach_arm_8R;
    case TAG_CPU_ARCH_V8M_BASE:
	return bfd_mach_arm_8M_BASE;
    case TAG_CPU_ARCH_V8M_MAIN:
	return bfd_mach_arm_8M_MAIN;
    case TAG_CPU_ARCH_V8_1M_MAIN:
	return bfd_mach_arm_8_1M_MAIN;
    case TAG_CPU_ARCH_V9:
	return bfd_mach_arm_9;

    default:
      /* Force entry to be added for any new known Tag_CPU_arch value.  */
      BFD_ASSERT (arch > MAX_TAG_CPU_ARCH);

      /* Unknown Tag_CPU_arch value.  */
      return bfd_mach_arm_unknown;
    }
}

/* Set the right machine number.  */

static bool
elf32_arm_object_p (bfd *abfd)
{
  unsigned int mach;

  mach = bfd_arm_get_mach_from_notes (abfd, ARM_NOTE_SECTION);

  if (mach == bfd_mach_arm_unknown)
    {
      if (elf_elfheader (abfd)->e_flags & EF_ARM_MAVERICK_FLOAT)
	mach = bfd_mach_arm_ep9312;
      else
	mach = bfd_arm_get_mach_from_attributes (abfd);
    }

  bfd_default_set_arch_mach (abfd, bfd_arch_arm, mach);
  return true;
}

/* Function to keep ARM specific flags in the ELF header.  */

static bool
elf32_arm_set_private_flags (bfd *abfd, flagword flags)
{
  if (elf_flags_init (abfd)
      && elf_elfheader (abfd)->e_flags != flags)
    {
      if (EF_ARM_EABI_VERSION (flags) == EF_ARM_EABI_UNKNOWN)
	{
	  if (flags & EF_ARM_INTERWORK)
	    _bfd_error_handler
	      (_("warning: not setting interworking flag of %pB since it has already been specified as non-interworking"),
	       abfd);
	  else
	    _bfd_error_handler
	      (_("warning: clearing the interworking flag of %pB due to outside request"),
	       abfd);
	}
    }
  else
    {
      elf_elfheader (abfd)->e_flags = flags;
      elf_flags_init (abfd) = true;
    }

  return true;
}

/* Copy backend specific data from one object module to another.  */

static bool
elf32_arm_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  flagword in_flags;
  flagword out_flags;

  if (! is_arm_elf (ibfd) || ! is_arm_elf (obfd))
    return true;

  in_flags  = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  if (elf_flags_init (obfd)
      && EF_ARM_EABI_VERSION (out_flags) == EF_ARM_EABI_UNKNOWN
      && in_flags != out_flags)
    {
      /* Cannot mix APCS26 and APCS32 code.  */
      if ((in_flags & EF_ARM_APCS_26) != (out_flags & EF_ARM_APCS_26))
	return false;

      /* Cannot mix float APCS and non-float APCS code.  */
      if ((in_flags & EF_ARM_APCS_FLOAT) != (out_flags & EF_ARM_APCS_FLOAT))
	return false;

      /* If the src and dest have different interworking flags
	 then turn off the interworking bit.  */
      if ((in_flags & EF_ARM_INTERWORK) != (out_flags & EF_ARM_INTERWORK))
	{
	  if (out_flags & EF_ARM_INTERWORK)
	    _bfd_error_handler
	      (_("warning: clearing the interworking flag of %pB because non-interworking code in %pB has been linked with it"),
	       obfd, ibfd);

	  in_flags &= ~EF_ARM_INTERWORK;
	}

      /* Likewise for PIC, though don't warn for this case.  */
      if ((in_flags & EF_ARM_PIC) != (out_flags & EF_ARM_PIC))
	in_flags &= ~EF_ARM_PIC;
    }

  elf_elfheader (obfd)->e_flags = in_flags;
  elf_flags_init (obfd) = true;

  return _bfd_elf_copy_private_bfd_data (ibfd, obfd);
}

/* Values for Tag_ABI_PCS_R9_use.  */
enum
{
  AEABI_R9_V6,
  AEABI_R9_SB,
  AEABI_R9_TLS,
  AEABI_R9_unused
};

/* Values for Tag_ABI_PCS_RW_data.  */
enum
{
  AEABI_PCS_RW_data_absolute,
  AEABI_PCS_RW_data_PCrel,
  AEABI_PCS_RW_data_SBrel,
  AEABI_PCS_RW_data_unused
};

/* Values for Tag_ABI_enum_size.  */
enum
{
  AEABI_enum_unused,
  AEABI_enum_short,
  AEABI_enum_wide,
  AEABI_enum_forced_wide
};

/* Determine whether an object attribute tag takes an integer, a
   string or both.  */

static int
elf32_arm_obj_attrs_arg_type (int tag)
{
  if (tag == Tag_compatibility)
    return ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL;
  else if (tag == Tag_nodefaults)
    return ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_NO_DEFAULT;
  else if (tag == Tag_CPU_raw_name || tag == Tag_CPU_name)
    return ATTR_TYPE_FLAG_STR_VAL;
  else if (tag < 32)
    return ATTR_TYPE_FLAG_INT_VAL;
  else
    return (tag & 1) != 0 ? ATTR_TYPE_FLAG_STR_VAL : ATTR_TYPE_FLAG_INT_VAL;
}

/* The ABI defines that Tag_conformance should be emitted first, and that
   Tag_nodefaults should be second (if either is defined).  This sets those
   two positions, and bumps up the position of all the remaining tags to
   compensate.  */
static int
elf32_arm_obj_attrs_order (int num)
{
  if (num == LEAST_KNOWN_OBJ_ATTRIBUTE)
    return Tag_conformance;
  if (num == LEAST_KNOWN_OBJ_ATTRIBUTE + 1)
    return Tag_nodefaults;
  if ((num - 2) < Tag_nodefaults)
    return num - 2;
  if ((num - 1) < Tag_conformance)
    return num - 1;
  return num;
}

/* Attribute numbers >=64 (mod 128) can be safely ignored.  */
static bool
elf32_arm_obj_attrs_handle_unknown (bfd *abfd, int tag)
{
  if ((tag & 127) < 64)
    {
      _bfd_error_handler
	(_("%pB: unknown mandatory EABI object attribute %d"),
	 abfd, tag);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  else
    {
      _bfd_error_handler
	(_("warning: %pB: unknown EABI object attribute %d"),
	 abfd, tag);
      return true;
    }
}

/* Read the architecture from the Tag_also_compatible_with attribute, if any.
   Returns -1 if no architecture could be read.  */

static int
get_secondary_compatible_arch (bfd *abfd)
{
  obj_attribute *attr =
    &elf_known_obj_attributes_proc (abfd)[Tag_also_compatible_with];

  /* Note: the tag and its argument below are uleb128 values, though
     currently-defined values fit in one byte for each.  */
  if (attr->s
      && attr->s[0] == Tag_CPU_arch
      && (attr->s[1] & 128) != 128
      && attr->s[2] == 0)
   return attr->s[1];

  /* This tag is "safely ignorable", so don't complain if it looks funny.  */
  return -1;
}

/* Set, or unset, the architecture of the Tag_also_compatible_with attribute.
   The tag is removed if ARCH is -1.  */

static void
set_secondary_compatible_arch (bfd *abfd, int arch)
{
  obj_attribute *attr =
    &elf_known_obj_attributes_proc (abfd)[Tag_also_compatible_with];

  if (arch == -1)
    {
      attr->s = NULL;
      return;
    }

  /* Note: the tag and its argument below are uleb128 values, though
     currently-defined values fit in one byte for each.  */
  if (!attr->s)
    attr->s = (char *) bfd_alloc (abfd, 3);
  attr->s[0] = Tag_CPU_arch;
  attr->s[1] = arch;
  attr->s[2] = '\0';
}

/* Combine two values for Tag_CPU_arch, taking secondary compatibility tags
   into account.  */

static int
tag_cpu_arch_combine (bfd *ibfd, int oldtag, int *secondary_compat_out,
		      int newtag, int secondary_compat)
{
#define T(X) TAG_CPU_ARCH_##X
  int tagl, tagh, result;
  const int v6t2[] =
    {
      T(V6T2),   /* PRE_V4.  */
      T(V6T2),   /* V4.  */
      T(V6T2),   /* V4T.  */
      T(V6T2),   /* V5T.  */
      T(V6T2),   /* V5TE.  */
      T(V6T2),   /* V5TEJ.  */
      T(V6T2),   /* V6.  */
      T(V7),     /* V6KZ.  */
      T(V6T2)    /* V6T2.  */
    };
  const int v6k[] =
    {
      T(V6K),    /* PRE_V4.  */
      T(V6K),    /* V4.  */
      T(V6K),    /* V4T.  */
      T(V6K),    /* V5T.  */
      T(V6K),    /* V5TE.  */
      T(V6K),    /* V5TEJ.  */
      T(V6K),    /* V6.  */
      T(V6KZ),   /* V6KZ.  */
      T(V7),     /* V6T2.  */
      T(V6K)     /* V6K.  */
    };
  const int v7[] =
    {
      T(V7),     /* PRE_V4.  */
      T(V7),     /* V4.  */
      T(V7),     /* V4T.  */
      T(V7),     /* V5T.  */
      T(V7),     /* V5TE.  */
      T(V7),     /* V5TEJ.  */
      T(V7),     /* V6.  */
      T(V7),     /* V6KZ.  */
      T(V7),     /* V6T2.  */
      T(V7),     /* V6K.  */
      T(V7)      /* V7.  */
    };
  const int v6_m[] =
    {
      -1,	 /* PRE_V4.  */
      -1,	 /* V4.  */
      T(V6K),    /* V4T.  */
      T(V6K),    /* V5T.  */
      T(V6K),    /* V5TE.  */
      T(V6K),    /* V5TEJ.  */
      T(V6K),    /* V6.  */
      T(V6KZ),   /* V6KZ.  */
      T(V7),     /* V6T2.  */
      T(V6K),    /* V6K.  */
      T(V7),     /* V7.  */
      T(V6_M)    /* V6_M.  */
    };
  const int v6s_m[] =
    {
      -1,	 /* PRE_V4.  */
      -1,	 /* V4.  */
      T(V6K),    /* V4T.  */
      T(V6K),    /* V5T.  */
      T(V6K),    /* V5TE.  */
      T(V6K),    /* V5TEJ.  */
      T(V6K),    /* V6.  */
      T(V6KZ),   /* V6KZ.  */
      T(V7),     /* V6T2.  */
      T(V6K),    /* V6K.  */
      T(V7),     /* V7.  */
      T(V6S_M),  /* V6_M.  */
      T(V6S_M)   /* V6S_M.  */
    };
  const int v7e_m[] =
    {
      -1,	 /* PRE_V4.  */
      -1,	 /* V4.  */
      T(V7E_M),  /* V4T.  */
      T(V7E_M),  /* V5T.  */
      T(V7E_M),  /* V5TE.  */
      T(V7E_M),  /* V5TEJ.  */
      T(V7E_M),  /* V6.  */
      T(V7E_M),  /* V6KZ.  */
      T(V7E_M),  /* V6T2.  */
      T(V7E_M),  /* V6K.  */
      T(V7E_M),  /* V7.  */
      T(V7E_M),  /* V6_M.  */
      T(V7E_M),  /* V6S_M.  */
      T(V7E_M)   /* V7E_M.  */
    };
  const int v8[] =
    {
      T(V8),		/* PRE_V4.  */
      T(V8),		/* V4.  */
      T(V8),		/* V4T.  */
      T(V8),		/* V5T.  */
      T(V8),		/* V5TE.  */
      T(V8),		/* V5TEJ.  */
      T(V8),		/* V6.  */
      T(V8),		/* V6KZ.  */
      T(V8),		/* V6T2.  */
      T(V8),		/* V6K.  */
      T(V8),		/* V7.  */
      T(V8),		/* V6_M.  */
      T(V8),		/* V6S_M.  */
      T(V8),		/* V7E_M.  */
      T(V8),		/* V8.  */
      T(V8),		/* V8-R.  */
      T(V8),		/* V8-M.BASE.  */
      T(V8),		/* V8-M.MAIN.  */
      T(V8),		/* V8.1.  */
      T(V8),		/* V8.2.  */
      T(V8),		/* V8.3.  */
      T(V8),		/* V8.1-M.MAIN.  */
    };
  const int v8r[] =
    {
      T(V8R),		/* PRE_V4.  */
      T(V8R),		/* V4.  */
      T(V8R),		/* V4T.  */
      T(V8R),		/* V5T.  */
      T(V8R),		/* V5TE.  */
      T(V8R),		/* V5TEJ.  */
      T(V8R),		/* V6.  */
      T(V8R),		/* V6KZ.  */
      T(V8R),		/* V6T2.  */
      T(V8R),		/* V6K.  */
      T(V8R),		/* V7.  */
      T(V8R),		/* V6_M.  */
      T(V8R),		/* V6S_M.  */
      T(V8R),		/* V7E_M.  */
      T(V8),		/* V8.  */
      T(V8R),		/* V8R.  */
    };
  const int v8m_baseline[] =
    {
      -1,		/* PRE_V4.  */
      -1,		/* V4.  */
      -1,		/* V4T.  */
      -1,		/* V5T.  */
      -1,		/* V5TE.  */
      -1,		/* V5TEJ.  */
      -1,		/* V6.  */
      -1,		/* V6KZ.  */
      -1,		/* V6T2.  */
      -1,		/* V6K.  */
      -1,		/* V7.  */
      T(V8M_BASE),	/* V6_M.  */
      T(V8M_BASE),	/* V6S_M.  */
      -1,		/* V7E_M.  */
      -1,		/* V8.  */
      -1,		/* V8R.  */
      T(V8M_BASE)	/* V8-M BASELINE.  */
    };
  const int v8m_mainline[] =
    {
      -1,		/* PRE_V4.  */
      -1,		/* V4.  */
      -1,		/* V4T.  */
      -1,		/* V5T.  */
      -1,		/* V5TE.  */
      -1,		/* V5TEJ.  */
      -1,		/* V6.  */
      -1,		/* V6KZ.  */
      -1,		/* V6T2.  */
      -1,		/* V6K.  */
      T(V8M_MAIN),	/* V7.  */
      T(V8M_MAIN),	/* V6_M.  */
      T(V8M_MAIN),	/* V6S_M.  */
      T(V8M_MAIN),	/* V7E_M.  */
      -1,		/* V8.  */
      -1,		/* V8R.  */
      T(V8M_MAIN),	/* V8-M BASELINE.  */
      T(V8M_MAIN)	/* V8-M MAINLINE.  */
    };
  const int v8_1m_mainline[] =
    {
      -1,		/* PRE_V4.  */
      -1,		/* V4.  */
      -1,		/* V4T.  */
      -1,		/* V5T.  */
      -1,		/* V5TE.  */
      -1,		/* V5TEJ.  */
      -1,		/* V6.  */
      -1,		/* V6KZ.  */
      -1,		/* V6T2.  */
      -1,		/* V6K.  */
      T(V8_1M_MAIN),	/* V7.  */
      T(V8_1M_MAIN),	/* V6_M.  */
      T(V8_1M_MAIN),	/* V6S_M.  */
      T(V8_1M_MAIN),	/* V7E_M.  */
      -1,		/* V8.  */
      -1,		/* V8R.  */
      T(V8_1M_MAIN),	/* V8-M BASELINE.  */
      T(V8_1M_MAIN),	/* V8-M MAINLINE.  */
      -1,		/* Unused (18).  */
      -1,		/* Unused (19).  */
      -1,		/* Unused (20).  */
      T(V8_1M_MAIN)	/* V8.1-M MAINLINE.  */
    };
  const int v9[] =
    {
      T(V9),		/* PRE_V4.  */
      T(V9),		/* V4.  */
      T(V9),		/* V4T.  */
      T(V9),		/* V5T.  */
      T(V9),		/* V5TE.  */
      T(V9),		/* V5TEJ.  */
      T(V9),		/* V6.  */
      T(V9),		/* V6KZ.  */
      T(V9),		/* V6T2.  */
      T(V9),		/* V6K.  */
      T(V9),		/* V7.  */
      T(V9),		/* V6_M.  */
      T(V9),		/* V6S_M.  */
      T(V9),		/* V7E_M.  */
      T(V9),		/* V8.  */
      T(V9),		/* V8-R.  */
      T(V9),		/* V8-M.BASE.  */
      T(V9),		/* V8-M.MAIN.  */
      T(V9),		/* V8.1.  */
      T(V9),		/* V8.2.  */
      T(V9),		/* V8.3.  */
      T(V9),		/* V8.1-M.MAIN.  */
      T(V9),		/* V9.  */
     };
  const int v4t_plus_v6_m[] =
    {
      -1,		/* PRE_V4.  */
      -1,		/* V4.  */
      T(V4T),		/* V4T.  */
      T(V5T),		/* V5T.  */
      T(V5TE),		/* V5TE.  */
      T(V5TEJ),		/* V5TEJ.  */
      T(V6),		/* V6.  */
      T(V6KZ),		/* V6KZ.  */
      T(V6T2),		/* V6T2.  */
      T(V6K),		/* V6K.  */
      T(V7),		/* V7.  */
      T(V6_M),		/* V6_M.  */
      T(V6S_M),		/* V6S_M.  */
      T(V7E_M),		/* V7E_M.  */
      T(V8),		/* V8.  */
      -1,		/* V8R.  */
      T(V8M_BASE),	/* V8-M BASELINE.  */
      T(V8M_MAIN),	/* V8-M MAINLINE.  */
      -1,		/* Unused (18).  */
      -1,		/* Unused (19).  */
      -1,		/* Unused (20).  */
      T(V8_1M_MAIN),	/* V8.1-M MAINLINE.  */
      T(V9),		/* V9.  */
      T(V4T_PLUS_V6_M)	/* V4T plus V6_M.  */
    };
  const int *comb[] =
    {
      v6t2,
      v6k,
      v7,
      v6_m,
      v6s_m,
      v7e_m,
      v8,
      v8r,
      v8m_baseline,
      v8m_mainline,
      NULL,
      NULL,
      NULL,
      v8_1m_mainline,
      v9,
      /* Pseudo-architecture.  */
      v4t_plus_v6_m
    };

  /* Check we've not got a higher architecture than we know about.  */

  if (oldtag > MAX_TAG_CPU_ARCH || newtag > MAX_TAG_CPU_ARCH)
    {
      _bfd_error_handler (_("error: %pB: unknown CPU architecture"), ibfd);
      return -1;
    }

  /* Override old tag if we have a Tag_also_compatible_with on the output.  */

  if ((oldtag == T(V6_M) && *secondary_compat_out == T(V4T))
      || (oldtag == T(V4T) && *secondary_compat_out == T(V6_M)))
    oldtag = T(V4T_PLUS_V6_M);

  /* And override the new tag if we have a Tag_also_compatible_with on the
     input.  */

  if ((newtag == T(V6_M) && secondary_compat == T(V4T))
      || (newtag == T(V4T) && secondary_compat == T(V6_M)))
    newtag = T(V4T_PLUS_V6_M);

  tagl = (oldtag < newtag) ? oldtag : newtag;
  result = tagh = (oldtag > newtag) ? oldtag : newtag;

  /* Architectures before V6KZ add features monotonically.  */
  if (tagh <= TAG_CPU_ARCH_V6KZ)
    return result;

  result = comb[tagh - T(V6T2)] ? comb[tagh - T(V6T2)][tagl] : -1;

  /* Use Tag_CPU_arch == V4T and Tag_also_compatible_with (Tag_CPU_arch V6_M)
     as the canonical version.  */
  if (result == T(V4T_PLUS_V6_M))
    {
      result = T(V4T);
      *secondary_compat_out = T(V6_M);
    }
  else
    *secondary_compat_out = -1;

  if (result == -1)
    {
      _bfd_error_handler (_("error: %pB: conflicting CPU architectures %d/%d"),
			  ibfd, oldtag, newtag);
      return -1;
    }

  return result;
#undef T
}

/* Query attributes object to see if integer divide instructions may be
   present in an object.  */
static bool
elf32_arm_attributes_accept_div (const obj_attribute *attr)
{
  int arch = attr[Tag_CPU_arch].i;
  int profile = attr[Tag_CPU_arch_profile].i;

  switch (attr[Tag_DIV_use].i)
    {
    case 0:
      /* Integer divide allowed if instruction contained in archetecture.  */
      if (arch == TAG_CPU_ARCH_V7 && (profile == 'R' || profile == 'M'))
	return true;
      else if (arch >= TAG_CPU_ARCH_V7E_M)
	return true;
      else
	return false;

    case 1:
      /* Integer divide explicitly prohibited.  */
      return false;

    default:
      /* Unrecognised case - treat as allowing divide everywhere.  */
    case 2:
      /* Integer divide allowed in ARM state.  */
      return true;
    }
}

/* Query attributes object to see if integer divide instructions are
   forbidden to be in the object.  This is not the inverse of
   elf32_arm_attributes_accept_div.  */
static bool
elf32_arm_attributes_forbid_div (const obj_attribute *attr)
{
  return attr[Tag_DIV_use].i == 1;
}

/* Merge EABI object attributes from IBFD into OBFD.  Raise an error if there
   are conflicting attributes.  */

static bool
elf32_arm_merge_eabi_attributes (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  obj_attribute *in_attr;
  obj_attribute *out_attr;
  /* Some tags have 0 = don't care, 1 = strong requirement,
     2 = weak requirement.  */
  static const int order_021[3] = {0, 2, 1};
  int i;
  bool result = true;
  const char *sec_name = get_elf_backend_data (ibfd)->obj_attrs_section;

  /* Skip the linker stubs file.  This preserves previous behavior
     of accepting unknown attributes in the first input file - but
     is that a bug?  */
  if (ibfd->flags & BFD_LINKER_CREATED)
    return true;

  /* Skip any input that hasn't attribute section.
     This enables to link object files without attribute section with
     any others.  */
  if (bfd_get_section_by_name (ibfd, sec_name) == NULL)
    return true;

  if (!elf_known_obj_attributes_proc (obfd)[0].i)
    {
      /* This is the first object.  Copy the attributes.  */
      _bfd_elf_copy_obj_attributes (ibfd, obfd);

      out_attr = elf_known_obj_attributes_proc (obfd);

      /* Use the Tag_null value to indicate the attributes have been
	 initialized.  */
      out_attr[0].i = 1;

      /* We do not output objects with Tag_MPextension_use_legacy - we move
	 the attribute's value to Tag_MPextension_use.  */
      if (out_attr[Tag_MPextension_use_legacy].i != 0)
	{
	  if (out_attr[Tag_MPextension_use].i != 0
	      && out_attr[Tag_MPextension_use_legacy].i
		!= out_attr[Tag_MPextension_use].i)
	    {
	      _bfd_error_handler
		(_("Error: %pB has both the current and legacy "
		   "Tag_MPextension_use attributes"), ibfd);
	      result = false;
	    }

	  out_attr[Tag_MPextension_use] =
	    out_attr[Tag_MPextension_use_legacy];
	  out_attr[Tag_MPextension_use_legacy].type = 0;
	  out_attr[Tag_MPextension_use_legacy].i = 0;
	}

      /* PR 28859 and 28848:  Handle the case where the first input file,
	 eg crti.o, has a Tag_ABI_HardFP_use of 3 but no Tag_FP_arch set.
	 Using Tag_ABI_HardFP_use in this way is deprecated, so reset the
	 attribute to zero.
	 FIXME: Should we handle other non-zero values of Tag_ABI_HardFO_use ? */
      if (out_attr[Tag_ABI_HardFP_use].i == 3 && out_attr[Tag_FP_arch].i == 0)
	out_attr[Tag_ABI_HardFP_use].i = 0;

      return result;
    }

  in_attr = elf_known_obj_attributes_proc (ibfd);
  out_attr = elf_known_obj_attributes_proc (obfd);
  /* This needs to happen before Tag_ABI_FP_number_model is merged.  */
  if (in_attr[Tag_ABI_VFP_args].i != out_attr[Tag_ABI_VFP_args].i)
    {
      /* Ignore mismatches if the object doesn't use floating point or is
	 floating point ABI independent.  */
      if (out_attr[Tag_ABI_FP_number_model].i == AEABI_FP_number_model_none
	  || (in_attr[Tag_ABI_FP_number_model].i != AEABI_FP_number_model_none
	      && out_attr[Tag_ABI_VFP_args].i == AEABI_VFP_args_compatible))
	out_attr[Tag_ABI_VFP_args].i = in_attr[Tag_ABI_VFP_args].i;
      else if (in_attr[Tag_ABI_FP_number_model].i != AEABI_FP_number_model_none
	       && in_attr[Tag_ABI_VFP_args].i != AEABI_VFP_args_compatible)
	{
	  _bfd_error_handler
	    (_("error: %pB uses VFP register arguments, %pB does not"),
	     in_attr[Tag_ABI_VFP_args].i ? ibfd : obfd,
	     in_attr[Tag_ABI_VFP_args].i ? obfd : ibfd);
	  result = false;
	}
    }

  for (i = LEAST_KNOWN_OBJ_ATTRIBUTE; i < NUM_KNOWN_OBJ_ATTRIBUTES; i++)
    {
      /* Merge this attribute with existing attributes.  */
      switch (i)
	{
	case Tag_CPU_raw_name:
	case Tag_CPU_name:
	  /* These are merged after Tag_CPU_arch.  */
	  break;

	case Tag_ABI_optimization_goals:
	case Tag_ABI_FP_optimization_goals:
	  /* Use the first value seen.  */
	  break;

	case Tag_CPU_arch:
	  {
	    int secondary_compat = -1, secondary_compat_out = -1;
	    unsigned int saved_out_attr = out_attr[i].i;
	    int arch_attr;
	    static const char *name_table[] =
	      {
		/* These aren't real CPU names, but we can't guess
		   that from the architecture version alone.  */
		"Pre v4",
		"ARM v4",
		"ARM v4T",
		"ARM v5T",
		"ARM v5TE",
		"ARM v5TEJ",
		"ARM v6",
		"ARM v6KZ",
		"ARM v6T2",
		"ARM v6K",
		"ARM v7",
		"ARM v6-M",
		"ARM v6S-M",
		"ARM v7E-M",
		"ARM v8",
		"ARM v8-R",
		"ARM v8-M.baseline",
		"ARM v8-M.mainline",
		"ARM v8.1-A",
		"ARM v8.2-A",
		"ARM v8.3-A",
		"ARM v8.1-M.mainline",
		"ARM v9",
	    };

	    /* Merge Tag_CPU_arch and Tag_also_compatible_with.  */
	    secondary_compat = get_secondary_compatible_arch (ibfd);
	    secondary_compat_out = get_secondary_compatible_arch (obfd);
	    arch_attr = tag_cpu_arch_combine (ibfd, out_attr[i].i,
					      &secondary_compat_out,
					      in_attr[i].i,
					      secondary_compat);

	    /* Return with error if failed to merge.  */
	    if (arch_attr == -1)
	      return false;

	    out_attr[i].i = arch_attr;

	    set_secondary_compatible_arch (obfd, secondary_compat_out);

	    /* Merge Tag_CPU_name and Tag_CPU_raw_name.  */
	    if (out_attr[i].i == saved_out_attr)
	      ; /* Leave the names alone.  */
	    else if (out_attr[i].i == in_attr[i].i)
	      {
		/* The output architecture has been changed to match the
		   input architecture.  Use the input names.  */
		out_attr[Tag_CPU_name].s = in_attr[Tag_CPU_name].s
		  ? _bfd_elf_attr_strdup (obfd, in_attr[Tag_CPU_name].s)
		  : NULL;
		out_attr[Tag_CPU_raw_name].s = in_attr[Tag_CPU_raw_name].s
		  ? _bfd_elf_attr_strdup (obfd, in_attr[Tag_CPU_raw_name].s)
		  : NULL;
	      }
	    else
	      {
		out_attr[Tag_CPU_name].s = NULL;
		out_attr[Tag_CPU_raw_name].s = NULL;
	      }

	    /* If we still don't have a value for Tag_CPU_name,
	       make one up now.  Tag_CPU_raw_name remains blank.  */
	    if (out_attr[Tag_CPU_name].s == NULL
		&& out_attr[i].i < ARRAY_SIZE (name_table))
	      out_attr[Tag_CPU_name].s =
		_bfd_elf_attr_strdup (obfd, name_table[out_attr[i].i]);
	  }
	  break;

	case Tag_ARM_ISA_use:
	case Tag_THUMB_ISA_use:
	case Tag_WMMX_arch:
	case Tag_Advanced_SIMD_arch:
	  /* ??? Do Advanced_SIMD (NEON) and WMMX conflict?  */
	case Tag_ABI_FP_rounding:
	case Tag_ABI_FP_exceptions:
	case Tag_ABI_FP_user_exceptions:
	case Tag_ABI_FP_number_model:
	case Tag_FP_HP_extension:
	case Tag_CPU_unaligned_access:
	case Tag_T2EE_use:
	case Tag_MPextension_use:
	case Tag_MVE_arch:
	case Tag_PAC_extension:
	case Tag_BTI_extension:
	case Tag_BTI_use:
	case Tag_PACRET_use:
	  /* Use the largest value specified.  */
	  if (in_attr[i].i > out_attr[i].i)
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_ABI_align_preserved:
	case Tag_ABI_PCS_RO_data:
	  /* Use the smallest value specified.  */
	  if (in_attr[i].i < out_attr[i].i)
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_ABI_align_needed:
	  if ((in_attr[i].i > 0 || out_attr[i].i > 0)
	      && (in_attr[Tag_ABI_align_preserved].i == 0
		  || out_attr[Tag_ABI_align_preserved].i == 0))
	    {
	      /* This error message should be enabled once all non-conformant
		 binaries in the toolchain have had the attributes set
		 properly.
	      _bfd_error_handler
		(_("error: %pB: 8-byte data alignment conflicts with %pB"),
		 obfd, ibfd);
	      result = false; */
	    }
	  /* Fall through.  */
	case Tag_ABI_FP_denormal:
	case Tag_ABI_PCS_GOT_use:
	  /* Use the "greatest" from the sequence 0, 2, 1, or the largest
	     value if greater than 2 (for future-proofing).  */
	  if ((in_attr[i].i > 2 && in_attr[i].i > out_attr[i].i)
	      || (in_attr[i].i <= 2 && out_attr[i].i <= 2
		  && order_021[in_attr[i].i] > order_021[out_attr[i].i]))
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_Virtualization_use:
	  /* The virtualization tag effectively stores two bits of
	     information: the intended use of TrustZone (in bit 0), and the
	     intended use of Virtualization (in bit 1).  */
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  else if (in_attr[i].i != 0
		   && in_attr[i].i != out_attr[i].i)
	    {
	      if (in_attr[i].i <= 3 && out_attr[i].i <= 3)
		out_attr[i].i = 3;
	      else
		{
		  _bfd_error_handler
		    (_("error: %pB: unable to merge virtualization attributes "
		       "with %pB"),
		     obfd, ibfd);
		  result = false;
		}
	    }
	  break;

	case Tag_CPU_arch_profile:
	  if (out_attr[i].i != in_attr[i].i)
	    {
	      /* 0 will merge with anything.
		 'A' and 'S' merge to 'A'.
		 'R' and 'S' merge to 'R'.
		 'M' and 'A|R|S' is an error.  */
	      if (out_attr[i].i == 0
		  || (out_attr[i].i == 'S'
		      && (in_attr[i].i == 'A' || in_attr[i].i == 'R')))
		out_attr[i].i = in_attr[i].i;
	      else if (in_attr[i].i == 0
		       || (in_attr[i].i == 'S'
			   && (out_attr[i].i == 'A' || out_attr[i].i == 'R')))
		; /* Do nothing.  */
	      else
		{
		  _bfd_error_handler
		    (_("error: %pB: conflicting architecture profiles %c/%c"),
		     ibfd,
		     in_attr[i].i ? in_attr[i].i : '0',
		     out_attr[i].i ? out_attr[i].i : '0');
		  result = false;
		}
	    }
	  break;

	case Tag_DSP_extension:
	  /* No need to change output value if any of:
	     - pre (<=) ARMv5T input architecture (do not have DSP)
	     - M input profile not ARMv7E-M and do not have DSP.  */
	  if (in_attr[Tag_CPU_arch].i <= 3
	      || (in_attr[Tag_CPU_arch_profile].i == 'M'
		  && in_attr[Tag_CPU_arch].i != 13
		  && in_attr[i].i == 0))
	    ; /* Do nothing.  */
	  /* Output value should be 0 if DSP part of architecture, ie.
	     - post (>=) ARMv5te architecture output
	     - A, R or S profile output or ARMv7E-M output architecture.  */
	  else if (out_attr[Tag_CPU_arch].i >= 4
		   && (out_attr[Tag_CPU_arch_profile].i == 'A'
		       || out_attr[Tag_CPU_arch_profile].i == 'R'
		       || out_attr[Tag_CPU_arch_profile].i == 'S'
		       || out_attr[Tag_CPU_arch].i == 13))
	    out_attr[i].i = 0;
	  /* Otherwise, DSP instructions are added and not part of output
	     architecture.  */
	  else
	    out_attr[i].i = 1;
	  break;

	case Tag_FP_arch:
	    {
	      /* Tag_ABI_HardFP_use is handled along with Tag_FP_arch since
		 the meaning of Tag_ABI_HardFP_use depends on Tag_FP_arch
		 when it's 0.  It might mean absence of FP hardware if
		 Tag_FP_arch is zero.  */

#define VFP_VERSION_COUNT 9
	      static const struct
	      {
		  int ver;
		  int regs;
	      } vfp_versions[VFP_VERSION_COUNT] =
		{
		  {0, 0},
		  {1, 16},
		  {2, 16},
		  {3, 32},
		  {3, 16},
		  {4, 32},
		  {4, 16},
		  {8, 32},
		  {8, 16}
		};
	      int ver;
	      int regs;
	      int newval;

	      /* If the output has no requirement about FP hardware,
		 follow the requirement of the input.  */
	      if (out_attr[i].i == 0)
		{
		  /* This assert is still reasonable, we shouldn't
		     produce the suspicious build attribute
		     combination (See below for in_attr).  */
		  BFD_ASSERT (out_attr[Tag_ABI_HardFP_use].i == 0);
		  out_attr[i].i = in_attr[i].i;
		  out_attr[Tag_ABI_HardFP_use].i
		    = in_attr[Tag_ABI_HardFP_use].i;
		  break;
		}
	      /* If the input has no requirement about FP hardware, do
		 nothing.  */
	      else if (in_attr[i].i == 0)
		{
		  /* We used to assert that Tag_ABI_HardFP_use was
		     zero here, but we should never assert when
		     consuming an object file that has suspicious
		     build attributes.  The single precision variant
		     of 'no FP architecture' is still 'no FP
		     architecture', so we just ignore the tag in this
		     case.  */
		  break;
		}

	      /* Both the input and the output have nonzero Tag_FP_arch.
		 So Tag_ABI_HardFP_use is implied by Tag_FP_arch when it's zero.  */

	      /* If both the input and the output have zero Tag_ABI_HardFP_use,
		 do nothing.  */
	      if (in_attr[Tag_ABI_HardFP_use].i == 0
		  && out_attr[Tag_ABI_HardFP_use].i == 0)
		;
	      /* If the input and the output have different Tag_ABI_HardFP_use,
		 the combination of them is 0 (implied by Tag_FP_arch).  */
	      else if (in_attr[Tag_ABI_HardFP_use].i
		       != out_attr[Tag_ABI_HardFP_use].i)
		out_attr[Tag_ABI_HardFP_use].i = 0;

	      /* Now we can handle Tag_FP_arch.  */

	      /* Values of VFP_VERSION_COUNT or more aren't defined, so just
		 pick the biggest.  */
	      if (in_attr[i].i >= VFP_VERSION_COUNT
		  && in_attr[i].i > out_attr[i].i)
		{
		  out_attr[i] = in_attr[i];
		  break;
		}
	      /* The output uses the superset of input features
		 (ISA version) and registers.  */
	      ver = vfp_versions[in_attr[i].i].ver;
	      if (ver < vfp_versions[out_attr[i].i].ver)
		ver = vfp_versions[out_attr[i].i].ver;
	      regs = vfp_versions[in_attr[i].i].regs;
	      if (regs < vfp_versions[out_attr[i].i].regs)
		regs = vfp_versions[out_attr[i].i].regs;
	      /* This assumes all possible supersets are also a valid
		 options.  */
	      for (newval = VFP_VERSION_COUNT - 1; newval > 0; newval--)
		{
		  if (regs == vfp_versions[newval].regs
		      && ver == vfp_versions[newval].ver)
		    break;
		}
	      out_attr[i].i = newval;
	    }
	  break;
	case Tag_PCS_config:
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  else if (in_attr[i].i != 0 && out_attr[i].i != in_attr[i].i)
	    {
	      /* It's sometimes ok to mix different configs, so this is only
		 a warning.  */
	      _bfd_error_handler
		(_("warning: %pB: conflicting platform configuration"), ibfd);
	    }
	  break;
	case Tag_ABI_PCS_R9_use:
	  if (in_attr[i].i != out_attr[i].i
	      && out_attr[i].i != AEABI_R9_unused
	      && in_attr[i].i != AEABI_R9_unused)
	    {
	      _bfd_error_handler
		(_("error: %pB: conflicting use of R9"), ibfd);
	      result = false;
	    }
	  if (out_attr[i].i == AEABI_R9_unused)
	    out_attr[i].i = in_attr[i].i;
	  break;
	case Tag_ABI_PCS_RW_data:
	  if (in_attr[i].i == AEABI_PCS_RW_data_SBrel
	      && out_attr[Tag_ABI_PCS_R9_use].i != AEABI_R9_SB
	      && out_attr[Tag_ABI_PCS_R9_use].i != AEABI_R9_unused)
	    {
	      _bfd_error_handler
		(_("error: %pB: SB relative addressing conflicts with use of R9"),
		 ibfd);
	      result = false;
	    }
	  /* Use the smallest value specified.  */
	  if (in_attr[i].i < out_attr[i].i)
	    out_attr[i].i = in_attr[i].i;
	  break;
	case Tag_ABI_PCS_wchar_t:
	  if (out_attr[i].i && in_attr[i].i && out_attr[i].i != in_attr[i].i
	      && !elf_arm_tdata (obfd)->no_wchar_size_warning)
	    {
	      _bfd_error_handler
		(_("warning: %pB uses %u-byte wchar_t yet the output is to use %u-byte wchar_t; use of wchar_t values across objects may fail"),
		 ibfd, in_attr[i].i, out_attr[i].i);
	    }
	  else if (in_attr[i].i && !out_attr[i].i)
	    out_attr[i].i = in_attr[i].i;
	  break;
	case Tag_ABI_enum_size:
	  if (in_attr[i].i != AEABI_enum_unused)
	    {
	      if (out_attr[i].i == AEABI_enum_unused
		  || out_attr[i].i == AEABI_enum_forced_wide)
		{
		  /* The existing object is compatible with anything.
		     Use whatever requirements the new object has.  */
		  out_attr[i].i = in_attr[i].i;
		}
	      else if (in_attr[i].i != AEABI_enum_forced_wide
		       && out_attr[i].i != in_attr[i].i
		       && !elf_arm_tdata (obfd)->no_enum_size_warning)
		{
		  static const char *aeabi_enum_names[] =
		    { "", "variable-size", "32-bit", "" };
		  const char *in_name =
		    in_attr[i].i < ARRAY_SIZE (aeabi_enum_names)
		    ? aeabi_enum_names[in_attr[i].i]
		    : "<unknown>";
		  const char *out_name =
		    out_attr[i].i < ARRAY_SIZE (aeabi_enum_names)
		    ? aeabi_enum_names[out_attr[i].i]
		    : "<unknown>";
		  _bfd_error_handler
		    (_("warning: %pB uses %s enums yet the output is to use %s enums; use of enum values across objects may fail"),
		     ibfd, in_name, out_name);
		}
	    }
	  break;
	case Tag_ABI_VFP_args:
	  /* Aready done.  */
	  break;
	case Tag_ABI_WMMX_args:
	  if (in_attr[i].i != out_attr[i].i)
	    {
	      _bfd_error_handler
		(_("error: %pB uses iWMMXt register arguments, %pB does not"),
		 ibfd, obfd);
	      result = false;
	    }
	  break;
	case Tag_compatibility:
	  /* Merged in target-independent code.  */
	  break;
	case Tag_ABI_HardFP_use:
	  /* This is handled along with Tag_FP_arch.  */
	  break;
	case Tag_ABI_FP_16bit_format:
	  if (in_attr[i].i != 0 && out_attr[i].i != 0)
	    {
	      if (in_attr[i].i != out_attr[i].i)
		{
		  _bfd_error_handler
		    (_("error: fp16 format mismatch between %pB and %pB"),
		     ibfd, obfd);
		  result = false;
		}
	    }
	  if (in_attr[i].i != 0)
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_DIV_use:
	  /* A value of zero on input means that the divide instruction may
	     be used if available in the base architecture as specified via
	     Tag_CPU_arch and Tag_CPU_arch_profile.  A value of 1 means that
	     the user did not want divide instructions.  A value of 2
	     explicitly means that divide instructions were allowed in ARM
	     and Thumb state.  */
	  if (in_attr[i].i == out_attr[i].i)
	    /* Do nothing.  */ ;
	  else if (elf32_arm_attributes_forbid_div (in_attr)
		   && !elf32_arm_attributes_accept_div (out_attr))
	    out_attr[i].i = 1;
	  else if (elf32_arm_attributes_forbid_div (out_attr)
		   && elf32_arm_attributes_accept_div (in_attr))
	    out_attr[i].i = in_attr[i].i;
	  else if (in_attr[i].i == 2)
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_MPextension_use_legacy:
	  /* We don't output objects with Tag_MPextension_use_legacy - we
	     move the value to Tag_MPextension_use.  */
	  if (in_attr[i].i != 0 && in_attr[Tag_MPextension_use].i != 0)
	    {
	      if (in_attr[Tag_MPextension_use].i != in_attr[i].i)
		{
		  _bfd_error_handler
		    (_("%pB has both the current and legacy "
		       "Tag_MPextension_use attributes"),
		     ibfd);
		  result = false;
		}
	    }

	  if (in_attr[i].i > out_attr[Tag_MPextension_use].i)
	    out_attr[Tag_MPextension_use] = in_attr[i];

	  break;

	case Tag_nodefaults:
	  /* This tag is set if it exists, but the value is unused (and is
	     typically zero).  We don't actually need to do anything here -
	     the merge happens automatically when the type flags are merged
	     below.  */
	  break;
	case Tag_also_compatible_with:
	  /* Already done in Tag_CPU_arch.  */
	  break;
	case Tag_conformance:
	  /* Keep the attribute if it matches.  Throw it away otherwise.
	     No attribute means no claim to conform.  */
	  if (!in_attr[i].s || !out_attr[i].s
	      || strcmp (in_attr[i].s, out_attr[i].s) != 0)
	    out_attr[i].s = NULL;
	  break;

	default:
	  result
	    = result && _bfd_elf_merge_unknown_attribute_low (ibfd, obfd, i);
	}

      /* If out_attr was copied from in_attr then it won't have a type yet.  */
      if (in_attr[i].type && !out_attr[i].type)
	out_attr[i].type = in_attr[i].type;
    }

  /* Merge Tag_compatibility attributes and any common GNU ones.  */
  if (!_bfd_elf_merge_object_attributes (ibfd, info))
    return false;

  /* Check for any attributes not known on ARM.  */
  result &= _bfd_elf_merge_unknown_attribute_list (ibfd, obfd);

  return result;
}


/* Return TRUE if the two EABI versions are incompatible.  */

static bool
elf32_arm_versions_compatible (unsigned iver, unsigned over)
{
  /* v4 and v5 are the same spec before and after it was released,
     so allow mixing them.  */
  if ((iver == EF_ARM_EABI_VER4 && over == EF_ARM_EABI_VER5)
      || (iver == EF_ARM_EABI_VER5 && over == EF_ARM_EABI_VER4))
    return true;

  return (iver == over);
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
elf32_arm_merge_private_bfd_data (bfd *, struct bfd_link_info *);

/* Display the flags field.  */

static bool
elf32_arm_print_private_bfd_data (bfd *abfd, void * ptr)
{
  FILE * file = (FILE *) ptr;
  unsigned long flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  /* Ignore init flag - it may not be set, despite the flags field
     containing valid data.  */

  fprintf (file, _("private flags = 0x%lx:"), elf_elfheader (abfd)->e_flags);

  switch (EF_ARM_EABI_VERSION (flags))
    {
    case EF_ARM_EABI_UNKNOWN:
      /* The following flag bits are GNU extensions and not part of the
	 official ARM ELF extended ABI.  Hence they are only decoded if
	 the EABI version is not set.  */
      if (flags & EF_ARM_INTERWORK)
	fprintf (file, _(" [interworking enabled]"));

      if (flags & EF_ARM_APCS_26)
	fprintf (file, " [APCS-26]");
      else
	fprintf (file, " [APCS-32]");

      if (flags & EF_ARM_VFP_FLOAT)
	fprintf (file, _(" [VFP float format]"));
      else if (flags & EF_ARM_MAVERICK_FLOAT)
	fprintf (file, _(" [Maverick float format]"));
      else
	fprintf (file, _(" [FPA float format]"));

      if (flags & EF_ARM_APCS_FLOAT)
	fprintf (file, _(" [floats passed in float registers]"));

      if (flags & EF_ARM_PIC)
	fprintf (file, _(" [position independent]"));

      if (flags & EF_ARM_NEW_ABI)
	fprintf (file, _(" [new ABI]"));

      if (flags & EF_ARM_OLD_ABI)
	fprintf (file, _(" [old ABI]"));

      if (flags & EF_ARM_SOFT_FLOAT)
	fprintf (file, _(" [software FP]"));

      flags &= ~(EF_ARM_INTERWORK | EF_ARM_APCS_26 | EF_ARM_APCS_FLOAT
		 | EF_ARM_PIC | EF_ARM_NEW_ABI | EF_ARM_OLD_ABI
		 | EF_ARM_SOFT_FLOAT | EF_ARM_VFP_FLOAT
		 | EF_ARM_MAVERICK_FLOAT);
      break;

    case EF_ARM_EABI_VER1:
      fprintf (file, _(" [Version1 EABI]"));

      if (flags & EF_ARM_SYMSARESORTED)
	fprintf (file, _(" [sorted symbol table]"));
      else
	fprintf (file, _(" [unsorted symbol table]"));

      flags &= ~ EF_ARM_SYMSARESORTED;
      break;

    case EF_ARM_EABI_VER2:
      fprintf (file, _(" [Version2 EABI]"));

      if (flags & EF_ARM_SYMSARESORTED)
	fprintf (file, _(" [sorted symbol table]"));
      else
	fprintf (file, _(" [unsorted symbol table]"));

      if (flags & EF_ARM_DYNSYMSUSESEGIDX)
	fprintf (file, _(" [dynamic symbols use segment index]"));

      if (flags & EF_ARM_MAPSYMSFIRST)
	fprintf (file, _(" [mapping symbols precede others]"));

      flags &= ~(EF_ARM_SYMSARESORTED | EF_ARM_DYNSYMSUSESEGIDX
		 | EF_ARM_MAPSYMSFIRST);
      break;

    case EF_ARM_EABI_VER3:
      fprintf (file, _(" [Version3 EABI]"));
      break;

    case EF_ARM_EABI_VER4:
      fprintf (file, _(" [Version4 EABI]"));
      goto eabi;

    case EF_ARM_EABI_VER5:
      fprintf (file, _(" [Version5 EABI]"));

      if (flags & EF_ARM_ABI_FLOAT_SOFT)
	fprintf (file, _(" [soft-float ABI]"));

      if (flags & EF_ARM_ABI_FLOAT_HARD)
	fprintf (file, _(" [hard-float ABI]"));

      flags &= ~(EF_ARM_ABI_FLOAT_SOFT | EF_ARM_ABI_FLOAT_HARD);

    eabi:
      if (flags & EF_ARM_BE8)
	fprintf (file, _(" [BE8]"));

      if (flags & EF_ARM_LE8)
	fprintf (file, _(" [LE8]"));

      flags &= ~(EF_ARM_LE8 | EF_ARM_BE8);
      break;

    default:
      fprintf (file, _(" <EABI version unrecognised>"));
      break;
    }

  flags &= ~ EF_ARM_EABIMASK;

  if (flags & EF_ARM_RELEXEC)
    fprintf (file, _(" [relocatable executable]"));

  if (flags & EF_ARM_PIC)
    fprintf (file, _(" [position independent]"));

  if (elf_elfheader (abfd)->e_ident[EI_OSABI] == ELFOSABI_ARM_FDPIC)
    fprintf (file, _(" [FDPIC ABI supplement]"));

  flags &= ~ (EF_ARM_RELEXEC | EF_ARM_PIC);

  if (flags)
    fprintf (file, _(" <Unrecognised flag bits set>"));

  fputc ('\n', file);

  return true;
}

static int
elf32_arm_get_symbol_type (Elf_Internal_Sym * elf_sym, int type)
{
  switch (ELF_ST_TYPE (elf_sym->st_info))
    {
    case STT_ARM_TFUNC:
      return ELF_ST_TYPE (elf_sym->st_info);

    case STT_ARM_16BIT:
      /* If the symbol is not an object, return the STT_ARM_16BIT flag.
	 This allows us to distinguish between data used by Thumb instructions
	 and non-data (which is probably code) inside Thumb regions of an
	 executable.  */
      if (type != STT_OBJECT && type != STT_TLS)
	return ELF_ST_TYPE (elf_sym->st_info);
      break;

    default:
      break;
    }

  return type;
}

static asection *
elf32_arm_gc_mark_hook (asection *sec,
			struct bfd_link_info *info,
			Elf_Internal_Rela *rel,
			struct elf_link_hash_entry *h,
			Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_ARM_GNU_VTINHERIT:
      case R_ARM_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Look through the relocs for a section during the first phase.  */

static bool
elf32_arm_check_relocs (bfd *abfd, struct bfd_link_info *info,
			asection *sec, const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  bfd *dynobj;
  asection *sreloc;
  struct elf32_arm_link_hash_table *htab;
  bool call_reloc_p;
  bool may_become_dynamic_p;
  bool may_need_local_target_p;
  unsigned long nsyms;

  if (bfd_link_relocatable (info))
    return true;

  BFD_ASSERT (is_arm_elf (abfd));

  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  sreloc = NULL;

  /* Create dynamic sections for relocatable executables so that we can
     copy relocations.  */
  if (htab->root.is_relocatable_executable
      && ! htab->root.dynamic_sections_created)
    {
      if (! _bfd_elf_link_create_dynamic_sections (abfd, info))
	return false;
    }

  if (htab->root.dynobj == NULL)
    htab->root.dynobj = abfd;
  if (!create_ifunc_sections (info))
    return false;

  dynobj = htab->root.dynobj;

  symtab_hdr = & elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);
  nsyms = NUM_SHDR_ENTRIES (symtab_hdr);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      Elf_Internal_Sym *isym;
      struct elf_link_hash_entry *h;
      struct elf32_arm_link_hash_entry *eh;
      unsigned int r_symndx;
      int r_type;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);
      r_type = arm_real_reloc_type (htab, r_type);

      if (r_symndx >= nsyms
	  /* PR 9934: It is possible to have relocations that do not
	     refer to symbols, thus it is also possible to have an
	     object file containing relocations but no symbol table.  */
	  && (r_symndx > STN_UNDEF || nsyms > 0))
	{
	  _bfd_error_handler (_("%pB: bad symbol index: %d"), abfd,
			      r_symndx);
	  return false;
	}

      h = NULL;
      isym = NULL;
      if (nsyms > 0)
	{
	  if (r_symndx < symtab_hdr->sh_info)
	    {
	      /* A local symbol.  */
	      isym = bfd_sym_from_r_symndx (&htab->root.sym_cache,
					    abfd, r_symndx);
	      if (isym == NULL)
		return false;
	    }
	  else
	    {
	      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	      while (h->root.type == bfd_link_hash_indirect
		     || h->root.type == bfd_link_hash_warning)
		h = (struct elf_link_hash_entry *) h->root.u.i.link;
	    }
	}

      eh = (struct elf32_arm_link_hash_entry *) h;

      call_reloc_p = false;
      may_become_dynamic_p = false;
      may_need_local_target_p = false;

      /* Could be done earlier, if h were already available.  */
      r_type = elf32_arm_tls_transition (info, r_type, h);
      switch (r_type)
	{
	case R_ARM_GOTOFFFUNCDESC:
	  {
	    if (h == NULL)
	      {
		if (!elf32_arm_allocate_local_sym_info (abfd))
		  return false;
		if (r_symndx >= elf32_arm_num_entries (abfd))
		  return false;
		elf32_arm_local_fdpic_cnts (abfd) [r_symndx].gotofffuncdesc_cnt += 1;
		elf32_arm_local_fdpic_cnts (abfd) [r_symndx].funcdesc_offset = -1;
	      }
	    else
	      {
		eh->fdpic_cnts.gotofffuncdesc_cnt++;
	      }
	  }
	  break;

	case R_ARM_GOTFUNCDESC:
	  {
	    if (h == NULL)
	      {
		/* Such a relocation is not supposed to be generated
		   by gcc on a static function.  */
		/* Anyway if needed it could be handled.  */
		return false;
	      }
	    else
	      {
		eh->fdpic_cnts.gotfuncdesc_cnt++;
	      }
	  }
	  break;

	case R_ARM_FUNCDESC:
	  {
	    if (h == NULL)
	      {
		if (!elf32_arm_allocate_local_sym_info (abfd))
		  return false;
		if (r_symndx >= elf32_arm_num_entries (abfd))
		  return false;
		elf32_arm_local_fdpic_cnts (abfd) [r_symndx].funcdesc_cnt += 1;
		elf32_arm_local_fdpic_cnts (abfd) [r_symndx].funcdesc_offset = -1;
	      }
	    else
	      {
		eh->fdpic_cnts.funcdesc_cnt++;
	      }
	  }
	  break;

	  case R_ARM_GOT32:
	  case R_ARM_GOT_PREL:
	  case R_ARM_TLS_GD32:
	  case R_ARM_TLS_GD32_FDPIC:
	  case R_ARM_TLS_IE32:
	  case R_ARM_TLS_IE32_FDPIC:
	  case R_ARM_TLS_GOTDESC:
	  case R_ARM_TLS_DESCSEQ:
	  case R_ARM_THM_TLS_DESCSEQ:
	  case R_ARM_TLS_CALL:
	  case R_ARM_THM_TLS_CALL:
	    /* This symbol requires a global offset table entry.  */
	    {
	      int tls_type, old_tls_type;

	      switch (r_type)
		{
		case R_ARM_TLS_GD32: tls_type = GOT_TLS_GD; break;
		case R_ARM_TLS_GD32_FDPIC: tls_type = GOT_TLS_GD; break;

		case R_ARM_TLS_IE32: tls_type = GOT_TLS_IE; break;
		case R_ARM_TLS_IE32_FDPIC: tls_type = GOT_TLS_IE; break;

		case R_ARM_TLS_GOTDESC:
		case R_ARM_TLS_CALL: case R_ARM_THM_TLS_CALL:
		case R_ARM_TLS_DESCSEQ: case R_ARM_THM_TLS_DESCSEQ:
		  tls_type = GOT_TLS_GDESC; break;

		default: tls_type = GOT_NORMAL; break;
		}

	      if (!bfd_link_executable (info) && (tls_type & GOT_TLS_IE))
		info->flags |= DF_STATIC_TLS;

	      if (h != NULL)
		{
		  h->got.refcount++;
		  old_tls_type = elf32_arm_hash_entry (h)->tls_type;
		}
	      else
		{
		  /* This is a global offset table entry for a local symbol.  */
		  if (!elf32_arm_allocate_local_sym_info (abfd))
		    return false;
		  if (r_symndx >= elf32_arm_num_entries (abfd))
		    {
		      _bfd_error_handler (_("%pB: bad symbol index: %d"), abfd,
					  r_symndx);
		      return false;
		    }

		  elf_local_got_refcounts (abfd)[r_symndx] += 1;
		  old_tls_type = elf32_arm_local_got_tls_type (abfd) [r_symndx];
		}

	      /* If a variable is accessed with both tls methods, two
		 slots may be created.  */
	      if (GOT_TLS_GD_ANY_P (old_tls_type)
		  && GOT_TLS_GD_ANY_P (tls_type))
		tls_type |= old_tls_type;

	      /* We will already have issued an error message if there
		 is a TLS/non-TLS mismatch, based on the symbol
		 type.  So just combine any TLS types needed.  */
	      if (old_tls_type != GOT_UNKNOWN && old_tls_type != GOT_NORMAL
		  && tls_type != GOT_NORMAL)
		tls_type |= old_tls_type;

	      /* If the symbol is accessed in both IE and GDESC
		 method, we're able to relax. Turn off the GDESC flag,
		 without messing up with any other kind of tls types
		 that may be involved.  */
	      if ((tls_type & GOT_TLS_IE) && (tls_type & GOT_TLS_GDESC))
		tls_type &= ~GOT_TLS_GDESC;

	      if (old_tls_type != tls_type)
		{
		  if (h != NULL)
		    elf32_arm_hash_entry (h)->tls_type = tls_type;
		  else
		    elf32_arm_local_got_tls_type (abfd) [r_symndx] = tls_type;
		}
	    }
	    /* Fall through.  */

	  case R_ARM_TLS_LDM32:
	  case R_ARM_TLS_LDM32_FDPIC:
	    if (r_type == R_ARM_TLS_LDM32 || r_type == R_ARM_TLS_LDM32_FDPIC)
		htab->tls_ldm_got.refcount++;
	    /* Fall through.  */

	  case R_ARM_GOTOFF32:
	  case R_ARM_GOTPC:
	    if (htab->root.sgot == NULL
		&& !create_got_section (htab->root.dynobj, info))
	      return false;
	    break;

	  case R_ARM_PC24:
	  case R_ARM_PLT32:
	  case R_ARM_CALL:
	  case R_ARM_JUMP24:
	  case R_ARM_PREL31:
	  case R_ARM_THM_CALL:
	  case R_ARM_THM_JUMP24:
	  case R_ARM_THM_JUMP19:
	    call_reloc_p = true;
	    may_need_local_target_p = true;
	    break;

	  case R_ARM_ABS12:
	    /* VxWorks uses dynamic R_ARM_ABS12 relocations for
	       ldr __GOTT_INDEX__ offsets.  */
	    if (htab->root.target_os != is_vxworks)
	      {
		may_need_local_target_p = true;
		break;
	      }
	    else goto jump_over;

	    /* Fall through.  */

	  case R_ARM_MOVW_ABS_NC:
	  case R_ARM_MOVT_ABS:
	  case R_ARM_THM_MOVW_ABS_NC:
	  case R_ARM_THM_MOVT_ABS:
	    if (bfd_link_pic (info))
	      {
		_bfd_error_handler
		  (_("%pB: relocation %s against `%s' can not be used when making a shared object; recompile with -fPIC"),
		   abfd, elf32_arm_howto_table_1[r_type].name,
		   (h) ? h->root.root.string : "a local symbol");
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    /* Fall through.  */
	  case R_ARM_ABS32:
	  case R_ARM_ABS32_NOI:
	jump_over:
	    if (h != NULL && bfd_link_executable (info))
	      {
		h->pointer_equality_needed = 1;
	      }
	    /* Fall through.  */
	  case R_ARM_REL32:
	  case R_ARM_REL32_NOI:
	  case R_ARM_MOVW_PREL_NC:
	  case R_ARM_MOVT_PREL:
	  case R_ARM_THM_MOVW_PREL_NC:
	  case R_ARM_THM_MOVT_PREL:

	    /* Should the interworking branches be listed here?  */
	    if ((bfd_link_pic (info) || htab->root.is_relocatable_executable
		 || htab->fdpic_p)
		&& (sec->flags & SEC_ALLOC) != 0)
	      {
		if (h == NULL
		    && elf32_arm_howto_from_type (r_type)->pc_relative)
		  {
		    /* In shared libraries and relocatable executables,
		       we treat local relative references as calls;
		       see the related SYMBOL_CALLS_LOCAL code in
		       allocate_dynrelocs.  */
		    call_reloc_p = true;
		    may_need_local_target_p = true;
		  }
		else
		  /* We are creating a shared library or relocatable
		     executable, and this is a reloc against a global symbol,
		     or a non-PC-relative reloc against a local symbol.
		     We may need to copy the reloc into the output.  */
		  may_become_dynamic_p = true;
	      }
	    else
	      may_need_local_target_p = true;
	    break;

	/* This relocation describes the C++ object vtable hierarchy.
	   Reconstruct it for later use during GC.  */
	case R_ARM_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	/* This relocation describes which C++ vtable entries are actually
	   used.  Record for later use during GC.  */
	case R_ARM_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_offset))
	    return false;
	  break;
	}

      if (h != NULL)
	{
	  if (call_reloc_p)
	    /* We may need a .plt entry if the function this reloc
	       refers to is in a different object, regardless of the
	       symbol's type.  We can't tell for sure yet, because
	       something later might force the symbol local.  */
	    h->needs_plt = 1;
	  else if (may_need_local_target_p)
	    /* If this reloc is in a read-only section, we might
	       need a copy reloc.  We can't check reliably at this
	       stage whether the section is read-only, as input
	       sections have not yet been mapped to output sections.
	       Tentatively set the flag for now, and correct in
	       adjust_dynamic_symbol.  */
	    h->non_got_ref = 1;
	}

      if (may_need_local_target_p
	  && (h != NULL || ELF32_ST_TYPE (isym->st_info) == STT_GNU_IFUNC))
	{
	  union gotplt_union *root_plt;
	  struct arm_plt_info *arm_plt;
	  struct arm_local_iplt_info *local_iplt;

	  if (h != NULL)
	    {
	      root_plt = &h->plt;
	      arm_plt = &eh->plt;
	    }
	  else
	    {
	      local_iplt = elf32_arm_create_local_iplt (abfd, r_symndx);
	      if (local_iplt == NULL)
		return false;
	      root_plt = &local_iplt->root;
	      arm_plt = &local_iplt->arm;
	    }

	  /* If the symbol is a function that doesn't bind locally,
	     this relocation will need a PLT entry.  */
	  if (root_plt->refcount != -1)
	    root_plt->refcount += 1;

	  if (!call_reloc_p)
	    arm_plt->noncall_refcount++;

	  /* It's too early to use htab->use_blx here, so we have to
	     record possible blx references separately from
	     relocs that definitely need a thumb stub.  */

	  if (r_type == R_ARM_THM_CALL)
	    arm_plt->maybe_thumb_refcount += 1;

	  if (r_type == R_ARM_THM_JUMP24
	      || r_type == R_ARM_THM_JUMP19)
	    arm_plt->thumb_refcount += 1;
	}

      if (may_become_dynamic_p)
	{
	  struct elf_dyn_relocs *p, **head;

	  /* Create a reloc section in dynobj.  */
	  if (sreloc == NULL)
	    {
	      sreloc = _bfd_elf_make_dynamic_reloc_section
		(sec, dynobj, 2, abfd, ! htab->use_rel);

	      if (sreloc == NULL)
		return false;
	    }

	  /* If this is a global symbol, count the number of
	     relocations we need for this symbol.  */
	  if (h != NULL)
	    head = &h->dyn_relocs;
	  else
	    {
	      head = elf32_arm_get_local_dynreloc_list (abfd, r_symndx, isym);
	      if (head == NULL)
		return false;
	    }

	  p = *head;
	  if (p == NULL || p->sec != sec)
	    {
	      size_t amt = sizeof *p;

	      p = (struct elf_dyn_relocs *) bfd_alloc (htab->root.dynobj, amt);
	      if (p == NULL)
		return false;
	      p->next = *head;
	      *head = p;
	      p->sec = sec;
	      p->count = 0;
	      p->pc_count = 0;
	    }

	  if (elf32_arm_howto_from_type (r_type)->pc_relative)
	    p->pc_count += 1;
	  p->count += 1;
	  if (h == NULL && htab->fdpic_p && !bfd_link_pic (info)
	      && r_type != R_ARM_ABS32 && r_type != R_ARM_ABS32_NOI)
	    {
	      /* Here we only support R_ARM_ABS32 and R_ARM_ABS32_NOI
		 that will become rofixup.  */
	      /* This is due to the fact that we suppose all will become rofixup.  */
	      _bfd_error_handler
		(_("FDPIC does not yet support %s relocation"
		   " to become dynamic for executable"),
		 elf32_arm_howto_table_1[r_type].name);
	      abort ();
	    }
	}
    }

  return true;
}

static void
elf32_arm_update_relocs (asection *o,
			 struct bfd_elf_section_reloc_data *reldata)
{
  void (*swap_in) (bfd *, const bfd_byte *, Elf_Internal_Rela *);
  void (*swap_out) (bfd *, const Elf_Internal_Rela *, bfd_byte *);
  const struct elf_backend_data *bed;
  _arm_elf_section_data *eado;
  struct bfd_link_order *p;
  bfd_byte *erela_head, *erela;
  Elf_Internal_Rela *irela_head, *irela;
  Elf_Internal_Shdr *rel_hdr;
  bfd *abfd;
  unsigned int count;

  eado = get_arm_elf_section_data (o);

  if (!eado || eado->elf.this_hdr.sh_type != SHT_ARM_EXIDX)
    return;

  abfd = o->owner;
  bed = get_elf_backend_data (abfd);
  rel_hdr = reldata->hdr;

  if (rel_hdr->sh_entsize == bed->s->sizeof_rel)
    {
      swap_in = bed->s->swap_reloc_in;
      swap_out = bed->s->swap_reloc_out;
    }
  else if (rel_hdr->sh_entsize == bed->s->sizeof_rela)
    {
      swap_in = bed->s->swap_reloca_in;
      swap_out = bed->s->swap_reloca_out;
    }
  else
    abort ();

  erela_head = rel_hdr->contents;
  irela_head = (Elf_Internal_Rela *) bfd_zmalloc
    ((NUM_SHDR_ENTRIES (rel_hdr) + 1) * sizeof (*irela_head));

  erela = erela_head;
  irela = irela_head;
  count = 0;

  for (p = o->map_head.link_order; p; p = p->next)
    {
      if (p->type == bfd_section_reloc_link_order
	  || p->type == bfd_symbol_reloc_link_order)
	{
	  (*swap_in) (abfd, erela, irela);
	  erela += rel_hdr->sh_entsize;
	  irela++;
	  count++;
	}
      else if (p->type == bfd_indirect_link_order)
	{
	  struct bfd_elf_section_reloc_data *input_reldata;
	  arm_unwind_table_edit *edit_list, *edit_tail;
	  _arm_elf_section_data *eadi;
	  bfd_size_type j;
	  bfd_vma offset;
	  asection *i;

	  i = p->u.indirect.section;

	  eadi = get_arm_elf_section_data (i);
	  edit_list = eadi->u.exidx.unwind_edit_list;
	  edit_tail = eadi->u.exidx.unwind_edit_tail;
	  offset = i->output_offset;

	  if (eadi->elf.rel.hdr &&
	      eadi->elf.rel.hdr->sh_entsize == rel_hdr->sh_entsize)
	    input_reldata = &eadi->elf.rel;
	  else if (eadi->elf.rela.hdr &&
		   eadi->elf.rela.hdr->sh_entsize == rel_hdr->sh_entsize)
	    input_reldata = &eadi->elf.rela;
	  else
	    abort ();

	  if (edit_list)
	    {
	      for (j = 0; j < NUM_SHDR_ENTRIES (input_reldata->hdr); j++)
		{
		  arm_unwind_table_edit *edit_node, *edit_next;
		  bfd_vma bias;
		  bfd_vma reloc_index;

		  (*swap_in) (abfd, erela, irela);
		  reloc_index = (irela->r_offset - offset) / 8;

		  bias = 0;
		  edit_node = edit_list;
		  for (edit_next = edit_list;
		       edit_next && edit_next->index <= reloc_index;
		       edit_next = edit_node->next)
		    {
		      bias++;
		      edit_node = edit_next;
		    }

		  if (edit_node->type != DELETE_EXIDX_ENTRY
		      || edit_node->index != reloc_index)
		    {
		      irela->r_offset -= bias * 8;
		      irela++;
		      count++;
		    }

		  erela += rel_hdr->sh_entsize;
		}

	      if (edit_tail->type == INSERT_EXIDX_CANTUNWIND_AT_END)
		{
		  /* New relocation entity.  */
		  asection *text_sec = edit_tail->linked_section;
		  asection *text_out = text_sec->output_section;
		  bfd_vma exidx_offset = offset + i->size - 8;

		  irela->r_addend = 0;
		  irela->r_offset = exidx_offset;
		  irela->r_info = ELF32_R_INFO
		    (text_out->target_index, R_ARM_PREL31);
		  irela++;
		  count++;
		}
	    }
	  else
	    {
	      for (j = 0; j < NUM_SHDR_ENTRIES (input_reldata->hdr); j++)
		{
		  (*swap_in) (abfd, erela, irela);
		  erela += rel_hdr->sh_entsize;
		  irela++;
		}

	      count += NUM_SHDR_ENTRIES (input_reldata->hdr);
	    }
	}
    }

  reldata->count = count;
  rel_hdr->sh_size = count * rel_hdr->sh_entsize;

  erela = erela_head;
  irela = irela_head;
  while (count > 0)
    {
      (*swap_out) (abfd, irela, erela);
      erela += rel_hdr->sh_entsize;
      irela++;
      count--;
    }

  free (irela_head);

  /* Hashes are no longer valid.  */
  free (reldata->hashes);
  reldata->hashes = NULL;
}

/* Unwinding tables are not referenced directly.  This pass marks them as
   required if the corresponding code section is marked.  Similarly, ARMv8-M
   secure entry functions can only be referenced by SG veneers which are
   created after the GC process. They need to be marked in case they reside in
   their own section (as would be the case if code was compiled with
   -ffunction-sections).  */

static bool
elf32_arm_gc_mark_extra_sections (struct bfd_link_info *info,
				  elf_gc_mark_hook_fn gc_mark_hook)
{
  bfd *sub;
  Elf_Internal_Shdr **elf_shdrp;
  asection *cmse_sec;
  obj_attribute *out_attr;
  Elf_Internal_Shdr *symtab_hdr;
  unsigned i, sym_count, ext_start;
  const struct elf_backend_data *bed;
  struct elf_link_hash_entry **sym_hashes;
  struct elf32_arm_link_hash_entry *cmse_hash;
  bool again, is_v8m, first_bfd_browse = true;
  bool extra_marks_added = false;
  asection *isec;

  _bfd_elf_gc_mark_extra_sections (info, gc_mark_hook);

  out_attr = elf_known_obj_attributes_proc (info->output_bfd);
  is_v8m = out_attr[Tag_CPU_arch].i >= TAG_CPU_ARCH_V8M_BASE
	   && out_attr[Tag_CPU_arch_profile].i == 'M';

  /* Marking EH data may cause additional code sections to be marked,
     requiring multiple passes.  */
  again = true;
  while (again)
    {
      again = false;
      for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
	{
	  asection *o;

	  if (! is_arm_elf (sub))
	    continue;

	  elf_shdrp = elf_elfsections (sub);
	  for (o = sub->sections; o != NULL; o = o->next)
	    {
	      Elf_Internal_Shdr *hdr;

	      hdr = &elf_section_data (o)->this_hdr;
	      if (hdr->sh_type == SHT_ARM_EXIDX
		  && hdr->sh_link
		  && hdr->sh_link < elf_numsections (sub)
		  && !o->gc_mark
		  && elf_shdrp[hdr->sh_link]->bfd_section->gc_mark)
		{
		  again = true;
		  if (!_bfd_elf_gc_mark (info, o, gc_mark_hook))
		    return false;
		}
	    }

	  /* Mark section holding ARMv8-M secure entry functions.  We mark all
	     of them so no need for a second browsing.  */
	  if (is_v8m && first_bfd_browse)
	    {
	      bool debug_sec_need_to_be_marked = false;

	      sym_hashes = elf_sym_hashes (sub);
	      bed = get_elf_backend_data (sub);
	      symtab_hdr = &elf_tdata (sub)->symtab_hdr;
	      sym_count = symtab_hdr->sh_size / bed->s->sizeof_sym;
	      ext_start = symtab_hdr->sh_info;

	      /* Scan symbols.  */
	      for (i = ext_start; i < sym_count; i++)
		{
		  cmse_hash = elf32_arm_hash_entry (sym_hashes[i - ext_start]);
		  if (cmse_hash == NULL)
		    continue;

		  /* Assume it is a special symbol.  If not, cmse_scan will
		     warn about it and user can do something about it.  */
		  if (startswith (cmse_hash->root.root.root.string,
				  CMSE_PREFIX))
		    {
		      cmse_sec = cmse_hash->root.root.u.def.section;
		      if (!cmse_sec->gc_mark
			  && !_bfd_elf_gc_mark (info, cmse_sec, gc_mark_hook))
			return false;
		      /* The debug sections related to these secure entry
			 functions are marked on enabling below flag.  */
		      debug_sec_need_to_be_marked = true;
		    }
		}

	      if (debug_sec_need_to_be_marked)
		{
		  /* Looping over all the sections of the object file containing
		     Armv8-M secure entry functions and marking all the debug
		     sections.  */
		  for (isec = sub->sections; isec != NULL; isec = isec->next)
		    {
		      /* If not a debug sections, skip it.  */
		      if (!isec->gc_mark && (isec->flags & SEC_DEBUGGING))
			{
			  isec->gc_mark = 1;
			  extra_marks_added = true;
			}
		    }
		  debug_sec_need_to_be_marked = false;
		}
	    }
	}

      first_bfd_browse = false;
    }

  /* PR 30354: If we have added extra marks then make sure that any
     dependencies of the newly marked sections are also marked.  */
  if (extra_marks_added)
    _bfd_elf_gc_mark_extra_sections (info, gc_mark_hook);

  return true;
}

/* Treat mapping symbols as special target symbols.  */

static bool
elf32_arm_is_target_special_symbol (bfd * abfd ATTRIBUTE_UNUSED, asymbol * sym)
{
  return bfd_is_arm_special_symbol_name (sym->name,
					 BFD_ARM_SPECIAL_SYM_TYPE_ANY);
}

/* If the ELF symbol SYM might be a function in SEC, return the
   function size and set *CODE_OFF to the function's entry point,
   otherwise return zero.  */

static bfd_size_type
elf32_arm_maybe_function_sym (const asymbol *sym, asection *sec,
			      bfd_vma *code_off)
{
  bfd_size_type size;
  elf_symbol_type * elf_sym = (elf_symbol_type *) sym;

  if ((sym->flags & (BSF_SECTION_SYM | BSF_FILE | BSF_OBJECT
		     | BSF_THREAD_LOCAL | BSF_RELC | BSF_SRELC)) != 0
      || sym->section != sec)
    return 0;

  size = (sym->flags & BSF_SYNTHETIC) ? 0 : elf_sym->internal_elf_sym.st_size;

  if (!(sym->flags & BSF_SYNTHETIC))
    switch (ELF_ST_TYPE (elf_sym->internal_elf_sym.st_info))
      {
	case STT_NOTYPE:
	  /* Ignore symbols created by the annobin plugin for gcc and clang.
	     These symbols are hidden, local, notype and have a size of 0.  */
	  if (size == 0
	      && sym->flags & BSF_LOCAL
	      && ELF_ST_VISIBILITY (elf_sym->internal_elf_sym.st_other) == STV_HIDDEN)
	    return 0;
	  /* Fall through.  */
	case STT_FUNC:
	case STT_ARM_TFUNC:
	  /* FIXME: Allow STT_GNU_IFUNC as well ?  */
	  break;
	default:
	  return 0;
      }

  if ((sym->flags & BSF_LOCAL)
      && bfd_is_arm_special_symbol_name (sym->name,
					 BFD_ARM_SPECIAL_SYM_TYPE_ANY))
    return 0;

  *code_off = sym->value;

  /* Do not return 0 for the function's size.  */
  return size ? size : 1;

}

static bool
elf32_arm_find_inliner_info (bfd *	    abfd,
			     const char **  filename_ptr,
			     const char **  functionname_ptr,
			     unsigned int * line_ptr)
{
  bool found;
  found = _bfd_dwarf2_find_inliner_info (abfd, filename_ptr,
					 functionname_ptr, line_ptr,
					 & elf_tdata (abfd)->dwarf2_find_line_info);
  return found;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
elf32_arm_adjust_dynamic_symbol (struct bfd_link_info * info,
				 struct elf_link_hash_entry * h)
{
  bfd * dynobj;
  asection *s, *srel;
  struct elf32_arm_link_hash_entry * eh;
  struct elf32_arm_link_hash_table *globals;

  globals = elf32_arm_hash_table (info);
  if (globals == NULL)
    return false;

  dynobj = elf_hash_table (info)->dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
	      && (h->needs_plt
		  || h->type == STT_GNU_IFUNC
		  || h->is_weakalias
		  || (h->def_dynamic
		      && h->ref_regular
		      && !h->def_regular)));

  eh = (struct elf32_arm_link_hash_entry *) h;

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC || h->type == STT_GNU_IFUNC || h->needs_plt)
    {
      /* Calls to STT_GNU_IFUNC symbols always use a PLT, even if the
	 symbol binds locally.  */
      if (h->plt.refcount <= 0
	  || (h->type != STT_GNU_IFUNC
	      && (SYMBOL_CALLS_LOCAL (info, h)
		  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		      && h->root.type == bfd_link_hash_undefweak))))
	{
	  /* This case can occur if we saw a PLT32 reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PC24 reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  eh->plt.thumb_refcount = 0;
	  eh->plt.maybe_thumb_refcount = 0;
	  eh->plt.noncall_refcount = 0;
	  h->needs_plt = 0;
	}

      return true;
    }
  else
    {
      /* It's possible that we incorrectly decided a .plt reloc was
	 needed for an R_ARM_PC24 or similar reloc to a non-function sym
	 in check_relocs.  We can't decide accurately between function
	 and non-function syms in check-relocs; Objects loaded later in
	 the link may change h->type.  So fix it now.  */
      h->plt.offset = (bfd_vma) -1;
      eh->plt.thumb_refcount = 0;
      eh->plt.maybe_thumb_refcount = 0;
      eh->plt.noncall_refcount = 0;
    }

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
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  Relocatable executables
     can reference data in shared objects directly, so we don't need to
     do anything here.  */
  if (bfd_link_pic (info) || globals->root.is_relocatable_executable)
    return true;

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */
  /* If allowed, we must generate a R_ARM_COPY reloc to tell the dynamic
     linker to copy the initial value out of the dynamic object and into
     the runtime process image.  We need to remember the offset into the
     .rel(a).bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = globals->root.sdynrelro;
      srel = globals->root.sreldynrelro;
    }
  else
    {
      s = globals->root.sdynbss;
      srel = globals->root.srelbss;
    }
  if (info->nocopyreloc == 0
      && (h->root.u.def.section->flags & SEC_ALLOC) != 0
      && h->size != 0)
    {
      elf32_arm_allocate_dynrelocs (info, srel, 1);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs_for_symbol (struct elf_link_hash_entry *h, void * inf)
{
  struct bfd_link_info *info;
  struct elf32_arm_link_hash_table *htab;
  struct elf32_arm_link_hash_entry *eh;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  eh = (struct elf32_arm_link_hash_entry *) h;

  info = (struct bfd_link_info *) inf;
  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  if ((htab->root.dynamic_sections_created || h->type == STT_GNU_IFUNC)
      && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      /* If the call in the PLT entry binds locally, the associated
	 GOT entry should use an R_ARM_IRELATIVE relocation instead of
	 the usual R_ARM_JUMP_SLOT.  Put it in the .iplt section rather
	 than the .plt section.  */
      if (h->type == STT_GNU_IFUNC && SYMBOL_CALLS_LOCAL (info, h))
	{
	  eh->is_iplt = 1;
	  if (eh->plt.noncall_refcount == 0
	      && SYMBOL_REFERENCES_LOCAL (info, h))
	    /* All non-call references can be resolved directly.
	       This means that they can (and in some cases, must)
	       resolve directly to the run-time target, rather than
	       to the PLT.  That in turns means that any .got entry
	       would be equal to the .igot.plt entry, so there's
	       no point having both.  */
	    h->got.refcount = 0;
	}

      if (bfd_link_pic (info)
	  || eh->is_iplt
	  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  elf32_arm_allocate_plt_entry (info, eh->is_iplt, &h->plt, &eh->plt);

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (! bfd_link_pic (info)
	      && !h->def_regular)
	    {
	      h->root.u.def.section = htab->root.splt;
	      h->root.u.def.value = h->plt.offset;

	      /* Make sure the function is not marked as Thumb, in case
		 it is the target of an ABS32 relocation, which will
		 point to the PLT entry.  */
	      ARM_SET_SYM_BRANCH_TYPE (h->target_internal, ST_BRANCH_TO_ARM);
	    }

	  /* VxWorks executables have a second set of relocations for
	     each PLT entry.  They go in a separate relocation section,
	     which is processed by the kernel loader.  */
	  if (htab->root.target_os == is_vxworks && !bfd_link_pic (info))
	    {
	      /* There is a relocation for the initial PLT entry:
		 an R_ARM_32 relocation for _GLOBAL_OFFSET_TABLE_.  */
	      if (h->plt.offset == htab->plt_header_size)
		elf32_arm_allocate_dynrelocs (info, htab->srelplt2, 1);

	      /* There are two extra relocations for each subsequent
		 PLT entry: an R_ARM_32 relocation for the GOT entry,
		 and an R_ARM_32 relocation for the PLT entry.  */
	      elf32_arm_allocate_dynrelocs (info, htab->srelplt2, 2);
	    }
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

  eh = (struct elf32_arm_link_hash_entry *) h;
  eh->tlsdesc_got = (bfd_vma) -1;

  if (h->got.refcount > 0)
    {
      asection *s;
      bool dyn;
      int tls_type = elf32_arm_hash_entry (h)->tls_type;
      int indx;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (htab->root.dynamic_sections_created
	  && h->dynindx == -1
	  && !h->forced_local
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      s = htab->root.sgot;
      h->got.offset = s->size;

      if (tls_type == GOT_UNKNOWN)
	abort ();

      if (tls_type == GOT_NORMAL)
	/* Non-TLS symbols need one GOT slot.  */
	s->size += 4;
      else
	{
	  if (tls_type & GOT_TLS_GDESC)
	    {
	      /* R_ARM_TLS_DESC needs 2 GOT slots.  */
	      eh->tlsdesc_got
		= (htab->root.sgotplt->size
		   - elf32_arm_compute_jump_table_size (htab));
	      htab->root.sgotplt->size += 8;
	      h->got.offset = (bfd_vma) -2;
	      /* plt.got_offset needs to know there's a TLS_DESC
		 reloc in the middle of .got.plt.  */
	      htab->num_tls_desc++;
	    }

	  if (tls_type & GOT_TLS_GD)
	    {
	      /* R_ARM_TLS_GD32 and R_ARM_TLS_GD32_FDPIC need two
		 consecutive GOT slots.  If the symbol is both GD
		 and GDESC, got.offset may have been
		 overwritten.  */
	      h->got.offset = s->size;
	      s->size += 8;
	    }

	  if (tls_type & GOT_TLS_IE)
	    /* R_ARM_TLS_IE32/R_ARM_TLS_IE32_FDPIC need one GOT
	       slot.  */
	    s->size += 4;
	}

      dyn = htab->root.dynamic_sections_created;

      indx = 0;
      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h)
	  && (!bfd_link_pic (info)
	      || !SYMBOL_REFERENCES_LOCAL (info, h)))
	indx = h->dynindx;

      if (tls_type != GOT_NORMAL
	  && (bfd_link_dll (info) || indx != 0)
	  && (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	      || h->root.type != bfd_link_hash_undefweak))
	{
	  if (tls_type & GOT_TLS_IE)
	    elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);

	  if (tls_type & GOT_TLS_GD)
	    elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);

	  if (tls_type & GOT_TLS_GDESC)
	    {
	      elf32_arm_allocate_dynrelocs (info, htab->root.srelplt, 1);
	      /* GDESC needs a trampoline to jump to.  */
	      htab->tls_trampoline = -1;
	    }

	  /* Only GD needs it.  GDESC just emits one relocation per
	     2 entries.  */
	  if ((tls_type & GOT_TLS_GD) && indx != 0)
	    elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
	}
      else if (((indx != -1) || htab->fdpic_p)
	       && !SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  if (htab->root.dynamic_sections_created)
	    /* Reserve room for the GOT entry's R_ARM_GLOB_DAT relocation.  */
	    elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
	}
      else if (h->type == STT_GNU_IFUNC
	       && eh->plt.noncall_refcount == 0)
	/* No non-call references resolve the STT_GNU_IFUNC's PLT entry;
	   they all resolve dynamically instead.  Reserve room for the
	   GOT entry's R_ARM_IRELATIVE relocation.  */
	elf32_arm_allocate_irelocs (info, htab->root.srelgot, 1);
      else if (bfd_link_pic (info)
	       && !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	/* Reserve room for the GOT entry's R_ARM_RELATIVE relocation.  */
	elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
      else if (htab->fdpic_p && tls_type == GOT_NORMAL)
	/* Reserve room for rofixup for FDPIC executable.  */
	/* TLS relocs do not need space since they are completely
	   resolved.  */
	htab->srofixup->size += 4;
    }
  else
    h->got.offset = (bfd_vma) -1;

  /* FDPIC support.  */
  if (eh->fdpic_cnts.gotofffuncdesc_cnt > 0)
    {
      /* Symbol musn't be exported.  */
      if (h->dynindx != -1)
	abort ();

      /* We only allocate one function descriptor with its associated
	 relocation.  */
      if (eh->fdpic_cnts.funcdesc_offset == -1)
	{
	  asection *s = htab->root.sgot;

	  eh->fdpic_cnts.funcdesc_offset = s->size;
	  s->size += 8;
	  /* We will add an R_ARM_FUNCDESC_VALUE relocation or two rofixups.  */
	  if (bfd_link_pic (info))
	    elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
	  else
	    htab->srofixup->size += 8;
	}
    }

  if (eh->fdpic_cnts.gotfuncdesc_cnt > 0)
    {
      asection *s = htab->root.sgot;

      if (htab->root.dynamic_sections_created && h->dynindx == -1
	  && !h->forced_local)
	if (! bfd_elf_link_record_dynamic_symbol (info, h))
	  return false;

      if (h->dynindx == -1)
	{
	  /* We only allocate one function descriptor with its
	     associated relocation.  */
	  if (eh->fdpic_cnts.funcdesc_offset == -1)
	    {

	      eh->fdpic_cnts.funcdesc_offset = s->size;
	      s->size += 8;
	      /* We will add an R_ARM_FUNCDESC_VALUE relocation or two
		 rofixups.  */
	      if (bfd_link_pic (info))
		elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
	      else
		htab->srofixup->size += 8;
	    }
	}

      /* Add one entry into the GOT and a R_ARM_FUNCDESC or
	 R_ARM_RELATIVE/rofixup relocation on it.  */
      eh->fdpic_cnts.gotfuncdesc_offset = s->size;
      s->size += 4;
      if (h->dynindx == -1 && !bfd_link_pic (info))
	htab->srofixup->size += 4;
      else
	elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
    }

  if (eh->fdpic_cnts.funcdesc_cnt > 0)
    {
      if (htab->root.dynamic_sections_created && h->dynindx == -1
	  && !h->forced_local)
	if (! bfd_elf_link_record_dynamic_symbol (info, h))
	  return false;

      if (h->dynindx == -1)
	{
	  /* We only allocate one function descriptor with its
	     associated relocation.  */
	  if (eh->fdpic_cnts.funcdesc_offset == -1)
	    {
	      asection *s = htab->root.sgot;

	      eh->fdpic_cnts.funcdesc_offset = s->size;
	      s->size += 8;
	      /* We will add an R_ARM_FUNCDESC_VALUE relocation or two
		 rofixups.  */
	      if (bfd_link_pic (info))
		elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
	      else
		htab->srofixup->size += 8;
	    }
	}
      if (h->dynindx == -1 && !bfd_link_pic (info))
	{
	  /* For FDPIC executable we replace R_ARM_RELATIVE with a rofixup.  */
	  htab->srofixup->size += 4 * eh->fdpic_cnts.funcdesc_cnt;
	}
      else
	{
	  /* Will need one dynamic reloc per reference. will be either
	     R_ARM_FUNCDESC or R_ARM_RELATIVE for hidden symbols.  */
	  elf32_arm_allocate_dynrelocs (info, htab->root.srelgot,
					eh->fdpic_cnts.funcdesc_cnt);
	}
    }

  /* Allocate stubs for exported Thumb functions on v4t.  */
  if (!htab->use_blx && h->dynindx != -1
      && h->def_regular
      && ARM_GET_SYM_BRANCH_TYPE (h->target_internal) == ST_BRANCH_TO_THUMB
      && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
    {
      struct elf_link_hash_entry * th;
      struct bfd_link_hash_entry * bh;
      struct elf_link_hash_entry * myh;
      char name[1024];
      asection *s;
      bh = NULL;
      /* Create a new symbol to regist the real location of the function.  */
      s = h->root.u.def.section;
      sprintf (name, "__real_%s", h->root.root.string);
      _bfd_generic_link_add_one_symbol (info, s->owner,
					name, BSF_GLOBAL, s,
					h->root.u.def.value,
					NULL, true, false, &bh);

      myh = (struct elf_link_hash_entry *) bh;
      myh->type = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
      myh->forced_local = 1;
      ARM_SET_SYM_BRANCH_TYPE (myh->target_internal, ST_BRANCH_TO_THUMB);
      eh->export_glue = myh;
      th = record_arm_to_thumb_glue (info, h);
      /* Point the symbol at the stub.  */
      h->type = ELF_ST_INFO (ELF_ST_BIND (h->type), STT_FUNC);
      ARM_SET_SYM_BRANCH_TYPE (h->target_internal, ST_BRANCH_TO_ARM);
      h->root.u.def.section = th->root.u.def.section;
      h->root.u.def.value = th->root.u.def.value & ~1;
    }

  if (h->dyn_relocs == NULL)
    return true;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (bfd_link_pic (info)
      || htab->root.is_relocatable_executable
      || htab->fdpic_p)
    {
      /* Relocs that use pc_count are PC-relative forms, which will appear
	 on something like ".long foo - ." or "movw REG, foo - .".  We want
	 calls to protected symbols to resolve directly to the function
	 rather than going via the plt.  If people want function pointer
	 comparisons to work as expected then they should avoid writing
	 assembly like ".long foo - .".  */
      if (SYMBOL_CALLS_LOCAL (info, h))
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

      if (htab->root.target_os == is_vxworks)
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &h->dyn_relocs; (p = *pp) != NULL; )
	    {
	      if (strcmp (p->sec->output_section->name, ".tls_vars") == 0)
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
	  else if (htab->root.dynamic_sections_created && h->dynindx == -1
		   && !h->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }
	}

      else if (htab->root.is_relocatable_executable && h->dynindx == -1
	       && h->root.type == bfd_link_hash_new)
	{
	  /* Output absolute symbols so that we can create relocations
	     against them.  For normal symbols we output a relocation
	     against the section that contains them.  */
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
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
	  if (h->dynindx == -1 && !h->forced_local
	      && h->root.type == bfd_link_hash_undefweak)
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
  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;

      if (h->type == STT_GNU_IFUNC
	  && eh->plt.noncall_refcount == 0
	  && SYMBOL_REFERENCES_LOCAL (info, h))
	elf32_arm_allocate_irelocs (info, sreloc, p->count);
      else if (h->dynindx != -1
	       && (!bfd_link_pic (info) || !info->symbolic || !h->def_regular))
	elf32_arm_allocate_dynrelocs (info, sreloc, p->count);
      else if (htab->fdpic_p && !bfd_link_pic (info))
	htab->srofixup->size += 4 * p->count;
      else
	elf32_arm_allocate_dynrelocs (info, sreloc, p->count);
    }

  return true;
}

void
bfd_elf32_arm_set_byteswap_code (struct bfd_link_info *info,
				 int byteswap_code)
{
  struct elf32_arm_link_hash_table *globals;

  globals = elf32_arm_hash_table (info);
  if (globals == NULL)
    return;

  globals->byteswap_code = byteswap_code;
}

/* Set the sizes of the dynamic sections.  */

static bool
elf32_arm_size_dynamic_sections (bfd * output_bfd ATTRIBUTE_UNUSED,
				 struct bfd_link_info * info)
{
  bfd * dynobj;
  asection * s;
  bool relocs;
  bfd *ibfd;
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);
  check_use_blx (htab);

  if (elf_hash_table (info)->dynamic_sections_created)
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

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      struct arm_local_iplt_info **local_iplt_ptr, *local_iplt;
      char *local_tls_type;
      bfd_vma *local_tlsdesc_gotent;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;
      unsigned int symndx;
      struct fdpic_local *local_fdpic_cnts;

      if (! is_arm_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = (struct elf_dyn_relocs *)
		   elf_section_data (s)->local_dynrel; p != NULL; p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (htab->root.target_os == is_vxworks
		       && strcmp (p->sec->output_section->name,
				  ".tls_vars") == 0)
		{
		  /* Relocations in vxworks .tls_vars sections are
		     handled specially by the loader.  */
		}
	      else if (p->count != 0)
		{
		  srel = elf_section_data (p->sec)->sreloc;
		  if (htab->fdpic_p && !bfd_link_pic (info))
		    htab->srofixup->size += 4 * p->count;
		  else
		    elf32_arm_allocate_dynrelocs (info, srel, p->count);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (local_got == NULL)
	continue;

      symtab_hdr = & elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      local_iplt_ptr = elf32_arm_local_iplt (ibfd);
      local_tls_type = elf32_arm_local_got_tls_type (ibfd);
      local_tlsdesc_gotent = elf32_arm_local_tlsdesc_gotent (ibfd);
      local_fdpic_cnts = elf32_arm_local_fdpic_cnts (ibfd);
      symndx = 0;
      s = htab->root.sgot;
      srel = htab->root.srelgot;
      for (; local_got < end_local_got;
	   ++local_got, ++local_iplt_ptr, ++local_tls_type,
	   ++local_tlsdesc_gotent, ++symndx, ++local_fdpic_cnts)
	{
	  if (symndx >= elf32_arm_num_entries (ibfd))
	    return false;

	  *local_tlsdesc_gotent = (bfd_vma) -1;
	  local_iplt = *local_iplt_ptr;

	  /* FDPIC support.  */
	  if (local_fdpic_cnts->gotofffuncdesc_cnt > 0)
	    {
	      if (local_fdpic_cnts->funcdesc_offset == -1)
		{
		  local_fdpic_cnts->funcdesc_offset = s->size;
		  s->size += 8;

		  /* We will add an R_ARM_FUNCDESC_VALUE relocation or two rofixups.  */
		  if (bfd_link_pic (info))
		    elf32_arm_allocate_dynrelocs (info, srel, 1);
		  else
		    htab->srofixup->size += 8;
		}
	    }

	  if (local_fdpic_cnts->funcdesc_cnt > 0)
	    {
	      if (local_fdpic_cnts->funcdesc_offset == -1)
		{
		  local_fdpic_cnts->funcdesc_offset = s->size;
		  s->size += 8;

		  /* We will add an R_ARM_FUNCDESC_VALUE relocation or two rofixups.  */
		  if (bfd_link_pic (info))
		    elf32_arm_allocate_dynrelocs (info, srel, 1);
		  else
		    htab->srofixup->size += 8;
		}

	      /* We will add n R_ARM_RELATIVE relocations or n rofixups.  */
	      if (bfd_link_pic (info))
		elf32_arm_allocate_dynrelocs (info, srel, local_fdpic_cnts->funcdesc_cnt);
	      else
		htab->srofixup->size += 4 * local_fdpic_cnts->funcdesc_cnt;
	    }

	  if (local_iplt != NULL)
	    {
	      struct elf_dyn_relocs *p;

	      if (local_iplt->root.refcount > 0)
		{
		  elf32_arm_allocate_plt_entry (info, true,
						&local_iplt->root,
						&local_iplt->arm);
		  if (local_iplt->arm.noncall_refcount == 0)
		    /* All references to the PLT are calls, so all
		       non-call references can resolve directly to the
		       run-time target.  This means that the .got entry
		       would be the same as the .igot.plt entry, so there's
		       no point creating both.  */
		    *local_got = 0;
		}
	      else
		{
		  BFD_ASSERT (local_iplt->arm.noncall_refcount == 0);
		  local_iplt->root.offset = (bfd_vma) -1;
		}

	      for (p = local_iplt->dyn_relocs; p != NULL; p = p->next)
		{
		  asection *psrel;

		  psrel = elf_section_data (p->sec)->sreloc;
		  if (local_iplt->arm.noncall_refcount == 0)
		    elf32_arm_allocate_irelocs (info, psrel, p->count);
		  else
		    elf32_arm_allocate_dynrelocs (info, psrel, p->count);
		}
	    }
	  if (*local_got > 0)
	    {
	      Elf_Internal_Sym *isym;

	      *local_got = s->size;
	      if (*local_tls_type & GOT_TLS_GD)
		/* TLS_GD relocs need an 8-byte structure in the GOT.  */
		s->size += 8;
	      if (*local_tls_type & GOT_TLS_GDESC)
		{
		  *local_tlsdesc_gotent = htab->root.sgotplt->size
		    - elf32_arm_compute_jump_table_size (htab);
		  htab->root.sgotplt->size += 8;
		  *local_got = (bfd_vma) -2;
		  /* plt.got_offset needs to know there's a TLS_DESC
		     reloc in the middle of .got.plt.  */
		  htab->num_tls_desc++;
		}
	      if (*local_tls_type & GOT_TLS_IE)
		s->size += 4;

	      if (*local_tls_type & GOT_NORMAL)
		{
		  /* If the symbol is both GD and GDESC, *local_got
		     may have been overwritten.  */
		  *local_got = s->size;
		  s->size += 4;
		}

	      isym = bfd_sym_from_r_symndx (&htab->root.sym_cache, ibfd,
					    symndx);
	      if (isym == NULL)
		return false;

	      /* If all references to an STT_GNU_IFUNC PLT are calls,
		 then all non-call references, including this GOT entry,
		 resolve directly to the run-time target.  */
	      if (ELF32_ST_TYPE (isym->st_info) == STT_GNU_IFUNC
		  && (local_iplt == NULL
		      || local_iplt->arm.noncall_refcount == 0))
		elf32_arm_allocate_irelocs (info, srel, 1);
	      else if (bfd_link_pic (info) || output_bfd->flags & DYNAMIC || htab->fdpic_p)
		{
		  if ((bfd_link_pic (info) && !(*local_tls_type & GOT_TLS_GDESC)))
		    elf32_arm_allocate_dynrelocs (info, srel, 1);
		  else if (htab->fdpic_p && *local_tls_type & GOT_NORMAL)
		    htab->srofixup->size += 4;

		  if ((bfd_link_pic (info) || htab->fdpic_p)
		      && *local_tls_type & GOT_TLS_GDESC)
		    {
		      elf32_arm_allocate_dynrelocs (info,
						    htab->root.srelplt, 1);
		      htab->tls_trampoline = -1;
		    }
		}
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  if (htab->tls_ldm_got.refcount > 0)
    {
      /* Allocate two GOT entries and one dynamic relocation (if necessary)
	 for R_ARM_TLS_LDM32/R_ARM_TLS_LDM32_FDPIC relocations.  */
      htab->tls_ldm_got.offset = htab->root.sgot->size;
      htab->root.sgot->size += 8;
      if (bfd_link_pic (info))
	elf32_arm_allocate_dynrelocs (info, htab->root.srelgot, 1);
    }
  else
    htab->tls_ldm_got.offset = -1;

  /* At the very end of the .rofixup section is a pointer to the GOT,
     reserve space for it. */
  if (htab->fdpic_p && htab->srofixup != NULL)
    htab->srofixup->size += 4;

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (& htab->root, allocate_dynrelocs_for_symbol, info);

  /* Here we rummage through the found bfds to collect glue information.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      if (! is_arm_elf (ibfd))
	continue;

      /* Initialise mapping tables for code/data.  */
      bfd_elf32_arm_init_maps (ibfd);

      if (!bfd_elf32_arm_process_before_allocation (ibfd, info)
	  || !bfd_elf32_arm_vfp11_erratum_scan (ibfd, info)
	  || !bfd_elf32_arm_stm32l4xx_erratum_scan (ibfd, info))
	_bfd_error_handler (_("errors encountered processing file %pB"), ibfd);
    }

  /* Allocate space for the glue sections now that we've sized them.  */
  bfd_elf32_arm_allocate_interworking_sections (info);

  /* For every jump slot reserved in the sgotplt, reloc_count is
     incremented.  However, when we reserve space for TLS descriptors,
     it's not incremented, so in order to compute the space reserved
     for them, it suffices to multiply the reloc count by the jump
     slot size.  */
  if (htab->root.srelplt)
    htab->sgotplt_jump_table_size = elf32_arm_compute_jump_table_size (htab);

  if (htab->tls_trampoline)
    {
      if (htab->root.splt->size == 0)
	htab->root.splt->size += htab->plt_header_size;

      htab->tls_trampoline = htab->root.splt->size;
      htab->root.splt->size += htab->plt_entry_size;

      /* If we're not using lazy TLS relocations, don't generate the
	 PLT and GOT entries they require.  */
      if ((info->flags & DF_BIND_NOW))
	htab->root.tlsdesc_plt = 0;
      else
	{
	  htab->root.tlsdesc_got = htab->root.sgot->size;
	  htab->root.sgot->size += 4;

	  htab->root.tlsdesc_plt = htab->root.splt->size;
	  htab->root.splt->size += 4 * ARRAY_SIZE (dl_tlsdesc_lazy_trampoline);
	}
    }

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char * name;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      /* It's OK to base decisions on the section name, because none
	 of the dynobj section names depend upon the input files.  */
      name = bfd_section_name (s);

      if (s == htab->root.splt)
	{
	  /* Remember whether there is a PLT.  */
	  ;
	}
      else if (startswith (name, ".rel"))
	{
	  if (s->size != 0)
	    {
	      /* Remember whether there are any reloc sections other
		 than .rel(a).plt and .rela.plt.unloaded.  */
	      if (s != htab->root.srelplt && s != htab->srelplt2)
		relocs = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else if (s != htab->root.sgot
	       && s != htab->root.sgotplt
	       && s != htab->root.iplt
	       && s != htab->root.igotplt
	       && s != htab->root.sdynbss
	       && s != htab->root.sdynrelro
	       && s != htab->srofixup)
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is mostly to handle .rel(a).bss and
	     .rel(a).plt.  We must create both sections in
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

      /* Allocate memory for the section contents.  */
      s->contents = (unsigned char *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_maybe_vxworks_add_dynamic_tags (output_bfd, info,
						  relocs);
}

/* Size sections even though they're not dynamic.  We use it to setup
   _TLS_MODULE_BASE_, if needed.  */

static bool
elf32_arm_always_size_sections (bfd *output_bfd,
				struct bfd_link_info *info)
{
  asection *tls_sec;
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);

  if (bfd_link_relocatable (info))
    return true;

  tls_sec = elf_hash_table (info)->tls_sec;

  if (tls_sec)
    {
      struct elf_link_hash_entry *tlsbase;

      tlsbase = elf_link_hash_lookup
	(elf_hash_table (info), "_TLS_MODULE_BASE_", true, true, false);

      if (tlsbase)
	{
	  struct bfd_link_hash_entry *bh = NULL;
	  const struct elf_backend_data *bed
	    = get_elf_backend_data (output_bfd);

	  if (!(_bfd_generic_link_add_one_symbol
		(info, output_bfd, "_TLS_MODULE_BASE_", BSF_LOCAL,
		 tls_sec, 0, NULL, false,
		 bed->collect, &bh)))
	    return false;

	  tlsbase->type = STT_TLS;
	  tlsbase = (struct elf_link_hash_entry *)bh;
	  tlsbase->def_regular = 1;
	  tlsbase->other = STV_HIDDEN;
	  (*bed->elf_backend_hide_symbol) (info, tlsbase, true);
	}
    }

  if (htab->fdpic_p && !bfd_link_relocatable (info)
      && !bfd_elf_stack_segment_size (output_bfd, info,
				      "__stacksize", DEFAULT_STACK_SIZE))
    return false;

  return true;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
elf32_arm_finish_dynamic_symbol (bfd * output_bfd,
				 struct bfd_link_info * info,
				 struct elf_link_hash_entry * h,
				 Elf_Internal_Sym * sym)
{
  struct elf32_arm_link_hash_table *htab;
  struct elf32_arm_link_hash_entry *eh;

  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  eh = (struct elf32_arm_link_hash_entry *) h;

  if (h->plt.offset != (bfd_vma) -1)
    {
      if (!eh->is_iplt)
	{
	  BFD_ASSERT (h->dynindx != -1);
	  if (! elf32_arm_populate_plt_entry (output_bfd, info, &h->plt, &eh->plt,
					      h->dynindx, 0))
	    return false;
	}

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  */
	  sym->st_shndx = SHN_UNDEF;
	  /* If the symbol is weak we need to clear the value.
	     Otherwise, the PLT entry would provide a definition for
	     the symbol even if the symbol wasn't defined anywhere,
	     and so the symbol would never be NULL.  Leave the value if
	     there were any relocations where pointer equality matters
	     (this is a clue for the dynamic linker, to make function
	     pointer comparisons work between an application and shared
	     library).  */
	  if (!h->ref_regular_nonweak || !h->pointer_equality_needed)
	    sym->st_value = 0;
	}
      else if (eh->is_iplt && eh->plt.noncall_refcount != 0)
	{
	  /* At least one non-call relocation references this .iplt entry,
	     so the .iplt entry is the function's canonical address.  */
	  sym->st_info = ELF_ST_INFO (ELF_ST_BIND (sym->st_info), STT_FUNC);
	  ARM_SET_SYM_BRANCH_TYPE (sym->st_target_internal, ST_BRANCH_TO_ARM);
	  sym->st_shndx = (_bfd_elf_section_from_bfd_section
			   (output_bfd, htab->root.iplt->output_section));
	  sym->st_value = (h->plt.offset
			   + htab->root.iplt->output_section->vma
			   + htab->root.iplt->output_offset);
	}
    }

  if (h->needs_copy)
    {
      asection * s;
      Elf_Internal_Rela rel;

      /* This symbol needs a copy reloc.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      rel.r_addend = 0;
      rel.r_offset = (h->root.u.def.value
		      + h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset);
      rel.r_info = ELF32_R_INFO (h->dynindx, R_ARM_COPY);
      if (h->root.u.def.section == htab->root.sdynrelro)
	s = htab->root.sreldynrelro;
      else
	s = htab->root.srelbss;
      elf32_arm_add_dynreloc (output_bfd, info, s, &rel);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  On VxWorks,
     and for FDPIC, the _GLOBAL_OFFSET_TABLE_ symbol is not absolute:
     it is relative to the ".got" section.  */
  if (h == htab->root.hdynamic
      || (!htab->fdpic_p
	  && htab->root.target_os != is_vxworks
	  && h == htab->root.hgot))
    sym->st_shndx = SHN_ABS;

  return true;
}

static void
arm_put_trampoline (struct elf32_arm_link_hash_table *htab, bfd *output_bfd,
		    void *contents,
		    const unsigned long *template, unsigned count)
{
  unsigned ix;

  for (ix = 0; ix != count; ix++)
    {
      unsigned long insn = template[ix];

      /* Emit mov pc,rx if bx is not permitted.  */
      if (htab->fix_v4bx == 1 && (insn & 0x0ffffff0) == 0x012fff10)
	insn = (insn & 0xf000000f) | 0x01a0f000;
      put_arm_insn (htab, output_bfd, insn, (char *)contents + ix*4);
    }
}

/* Install the special first PLT entry for elf32-arm-nacl.  Unlike
   other variants, NaCl needs this entry in a static executable's
   .iplt too.  When we're handling that case, GOT_DISPLACEMENT is
   zero.  For .iplt really only the last bundle is useful, and .iplt
   could have a shorter first entry, with each individual PLT entry's
   relative branch calculated differently so it targets the last
   bundle instead of the instruction before it (labelled .Lplt_tail
   above).  But it's simpler to keep the size and layout of PLT0
   consistent with the dynamic case, at the cost of some dead code at
   the start of .iplt and the one dead store to the stack at the start
   of .Lplt_tail.  */
static void
arm_nacl_put_plt0 (struct elf32_arm_link_hash_table *htab, bfd *output_bfd,
		   asection *plt, bfd_vma got_displacement)
{
  unsigned int i;

  put_arm_insn (htab, output_bfd,
		elf32_arm_nacl_plt0_entry[0]
		| arm_movw_immediate (got_displacement),
		plt->contents + 0);
  put_arm_insn (htab, output_bfd,
		elf32_arm_nacl_plt0_entry[1]
		| arm_movt_immediate (got_displacement),
		plt->contents + 4);

  for (i = 2; i < ARRAY_SIZE (elf32_arm_nacl_plt0_entry); ++i)
    put_arm_insn (htab, output_bfd,
		  elf32_arm_nacl_plt0_entry[i],
		  plt->contents + (i * 4));
}

/* Finish up the dynamic sections.  */

static bool
elf32_arm_finish_dynamic_sections (bfd * output_bfd, struct bfd_link_info * info)
{
  bfd * dynobj;
  asection * sgot;
  asection * sdyn;
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = elf_hash_table (info)->dynobj;

  sgot = htab->root.sgotplt;
  /* A broken linker script might have discarded the dynamic sections.
     Catch this here so that we do not seg-fault later on.  */
  if (sgot != NULL && bfd_is_abs_section (sgot->output_section))
    return false;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      splt = htab->root.splt;
      BFD_ASSERT (splt != NULL && sdyn != NULL);
      BFD_ASSERT (sgot != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);

      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  const char * name;
	  asection * s;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      if (htab->root.target_os == is_vxworks
		  && elf_vxworks_finish_dynamic_entry (output_bfd, &dyn))
		bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_HASH:
	    case DT_STRTAB:
	    case DT_SYMTAB:
	    case DT_VERSYM:
	    case DT_VERDEF:
	    case DT_VERNEED:
	      break;

	    case DT_PLTGOT:
	      name = ".got.plt";
	      goto get_vma;
	    case DT_JMPREL:
	      name = RELOC_SECTION (htab, ".plt");
	    get_vma:
	      s = bfd_get_linker_section (dynobj, name);
	      if (s == NULL)
		{
		  _bfd_error_handler
		    (_("could not find section %s"), name);
		  bfd_set_error (bfd_error_invalid_operation);
		  return false;
		}
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = htab->root.srelplt;
	      BFD_ASSERT (s != NULL);
	      dyn.d_un.d_val = s->size;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_RELSZ:
	    case DT_RELASZ:
	    case DT_REL:
	    case DT_RELA:
	      break;

	    case DT_TLSDESC_PLT:
	      s = htab->root.splt;
	      dyn.d_un.d_ptr = (s->output_section->vma + s->output_offset
				+ htab->root.tlsdesc_plt);
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_TLSDESC_GOT:
	      s = htab->root.sgot;
	      dyn.d_un.d_ptr = (s->output_section->vma + s->output_offset
				+ htab->root.tlsdesc_got);
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	      /* Set the bottom bit of DT_INIT/FINI if the
		 corresponding function is Thumb.  */
	    case DT_INIT:
	      name = info->init_function;
	      goto get_sym;
	    case DT_FINI:
	      name = info->fini_function;
	    get_sym:
	      /* If it wasn't set by elf_bfd_final_link
		 then there is nothing to adjust.  */
	      if (dyn.d_un.d_val != 0)
		{
		  struct elf_link_hash_entry * eh;

		  eh = elf_link_hash_lookup (elf_hash_table (info), name,
					     false, false, true);
		  if (eh != NULL
		      && ARM_GET_SYM_BRANCH_TYPE (eh->target_internal)
			 == ST_BRANCH_TO_THUMB)
		    {
		      dyn.d_un.d_val |= 1;
		      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
		    }
		}
	      break;
	    }
	}

      /* Fill in the first entry in the procedure linkage table.  */
      if (splt->size > 0 && htab->plt_header_size)
	{
	  const bfd_vma *plt0_entry;
	  bfd_vma got_address, plt_address, got_displacement;

	  /* Calculate the addresses of the GOT and PLT.  */
	  got_address = sgot->output_section->vma + sgot->output_offset;
	  plt_address = splt->output_section->vma + splt->output_offset;

	  if (htab->root.target_os == is_vxworks)
	    {
	      /* The VxWorks GOT is relocated by the dynamic linker.
		 Therefore, we must emit relocations rather than simply
		 computing the values now.  */
	      Elf_Internal_Rela rel;

	      plt0_entry = elf32_arm_vxworks_exec_plt0_entry;
	      put_arm_insn (htab, output_bfd, plt0_entry[0],
			    splt->contents + 0);
	      put_arm_insn (htab, output_bfd, plt0_entry[1],
			    splt->contents + 4);
	      put_arm_insn (htab, output_bfd, plt0_entry[2],
			    splt->contents + 8);
	      bfd_put_32 (output_bfd, got_address, splt->contents + 12);

	      /* Generate a relocation for _GLOBAL_OFFSET_TABLE_.  */
	      rel.r_offset = plt_address + 12;
	      rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_ARM_ABS32);
	      rel.r_addend = 0;
	      SWAP_RELOC_OUT (htab) (output_bfd, &rel,
				     htab->srelplt2->contents);
	    }
	  else if (htab->root.target_os == is_nacl)
	    arm_nacl_put_plt0 (htab, output_bfd, splt,
			       got_address + 8 - (plt_address + 16));
	  else if (using_thumb_only (htab))
	    {
	      got_displacement = got_address - (plt_address + 12);

	      plt0_entry = elf32_thumb2_plt0_entry;
	      put_arm_insn (htab, output_bfd, plt0_entry[0],
			    splt->contents + 0);
	      put_arm_insn (htab, output_bfd, plt0_entry[1],
			    splt->contents + 4);
	      put_arm_insn (htab, output_bfd, plt0_entry[2],
			    splt->contents + 8);

	      bfd_put_32 (output_bfd, got_displacement, splt->contents + 12);
	    }
	  else
	    {
	      got_displacement = got_address - (plt_address + 16);

	      plt0_entry = elf32_arm_plt0_entry;
	      put_arm_insn (htab, output_bfd, plt0_entry[0],
			    splt->contents + 0);
	      put_arm_insn (htab, output_bfd, plt0_entry[1],
			    splt->contents + 4);
	      put_arm_insn (htab, output_bfd, plt0_entry[2],
			    splt->contents + 8);
	      put_arm_insn (htab, output_bfd, plt0_entry[3],
			    splt->contents + 12);

#ifdef FOUR_WORD_PLT
	      /* The displacement value goes in the otherwise-unused
		 last word of the second entry.  */
	      bfd_put_32 (output_bfd, got_displacement, splt->contents + 28);
#else
	      bfd_put_32 (output_bfd, got_displacement, splt->contents + 16);
#endif
	    }
	}

      /* UnixWare sets the entsize of .plt to 4, although that doesn't
	 really seem like the right value.  */
      if (splt->output_section->owner == output_bfd)
	elf_section_data (splt->output_section)->this_hdr.sh_entsize = 4;

      if (htab->root.tlsdesc_plt)
	{
	  bfd_vma got_address
	    = sgot->output_section->vma + sgot->output_offset;
	  bfd_vma gotplt_address = (htab->root.sgot->output_section->vma
				    + htab->root.sgot->output_offset);
	  bfd_vma plt_address
	    = splt->output_section->vma + splt->output_offset;

	  arm_put_trampoline (htab, output_bfd,
			      splt->contents + htab->root.tlsdesc_plt,
			      dl_tlsdesc_lazy_trampoline, 6);

	  bfd_put_32 (output_bfd,
		      gotplt_address + htab->root.tlsdesc_got
		      - (plt_address + htab->root.tlsdesc_plt)
		      - dl_tlsdesc_lazy_trampoline[6],
		      splt->contents + htab->root.tlsdesc_plt + 24);
	  bfd_put_32 (output_bfd,
		      got_address - (plt_address + htab->root.tlsdesc_plt)
		      - dl_tlsdesc_lazy_trampoline[7],
		      splt->contents + htab->root.tlsdesc_plt + 24 + 4);
	}

      if (htab->tls_trampoline)
	{
	  arm_put_trampoline (htab, output_bfd,
			      splt->contents + htab->tls_trampoline,
			      tls_trampoline, 3);
#ifdef FOUR_WORD_PLT
	  bfd_put_32 (output_bfd, 0x00000000,
		      splt->contents + htab->tls_trampoline + 12);
#endif
	}

      if (htab->root.target_os == is_vxworks
	  && !bfd_link_pic (info)
	  && htab->root.splt->size > 0)
	{
	  /* Correct the .rel(a).plt.unloaded relocations.  They will have
	     incorrect symbol indexes.  */
	  int num_plts;
	  unsigned char *p;

	  num_plts = ((htab->root.splt->size - htab->plt_header_size)
		      / htab->plt_entry_size);
	  p = htab->srelplt2->contents + RELOC_SIZE (htab);

	  for (; num_plts; num_plts--)
	    {
	      Elf_Internal_Rela rel;

	      SWAP_RELOC_IN (htab) (output_bfd, p, &rel);
	      rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_ARM_ABS32);
	      SWAP_RELOC_OUT (htab) (output_bfd, &rel, p);
	      p += RELOC_SIZE (htab);

	      SWAP_RELOC_IN (htab) (output_bfd, p, &rel);
	      rel.r_info = ELF32_R_INFO (htab->root.hplt->indx, R_ARM_ABS32);
	      SWAP_RELOC_OUT (htab) (output_bfd, &rel, p);
	      p += RELOC_SIZE (htab);
	    }
	}
    }

  if (htab->root.target_os == is_nacl
      && htab->root.iplt != NULL
      && htab->root.iplt->size > 0)
    /* NaCl uses a special first entry in .iplt too.  */
    arm_nacl_put_plt0 (htab, output_bfd, htab->root.iplt, 0);

  /* Fill in the first three entries in the global offset table.  */
  if (sgot)
    {
      if (sgot->size > 0)
	{
	  if (sdyn == NULL)
	    bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents);
	  else
	    bfd_put_32 (output_bfd,
			sdyn->output_section->vma + sdyn->output_offset,
			sgot->contents);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + 4);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + 8);
	}

      elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;
    }

  /* At the very end of the .rofixup section is a pointer to the GOT.  */
  if (htab->fdpic_p && htab->srofixup != NULL)
    {
      struct elf_link_hash_entry *hgot = htab->root.hgot;

      bfd_vma got_value = hgot->root.u.def.value
	+ hgot->root.u.def.section->output_section->vma
	+ hgot->root.u.def.section->output_offset;

      arm_elf_add_rofixup (output_bfd, htab->srofixup, got_value);

      /* Make sure we allocated and generated the same number of fixups.  */
      BFD_ASSERT (htab->srofixup->reloc_count * 4 == htab->srofixup->size);
    }

  return true;
}

static bool
elf32_arm_init_file_header (bfd *abfd, struct bfd_link_info *link_info)
{
  Elf_Internal_Ehdr * i_ehdrp;	/* ELF file header, internal form.  */
  struct elf32_arm_link_hash_table *globals;
  struct elf_segment_map *m;

  if (!_bfd_elf_init_file_header (abfd, link_info))
    return false;

  i_ehdrp = elf_elfheader (abfd);

  if (EF_ARM_EABI_VERSION (i_ehdrp->e_flags) == EF_ARM_EABI_UNKNOWN)
    i_ehdrp->e_ident[EI_OSABI] = ELFOSABI_ARM;
  i_ehdrp->e_ident[EI_ABIVERSION] = ARM_ELF_ABI_VERSION;

  if (link_info)
    {
      globals = elf32_arm_hash_table (link_info);
      if (globals != NULL && globals->byteswap_code)
	i_ehdrp->e_flags |= EF_ARM_BE8;

      if (globals->fdpic_p)
	i_ehdrp->e_ident[EI_OSABI] |= ELFOSABI_ARM_FDPIC;
    }

  if (EF_ARM_EABI_VERSION (i_ehdrp->e_flags) == EF_ARM_EABI_VER5
      && ((i_ehdrp->e_type == ET_DYN) || (i_ehdrp->e_type == ET_EXEC)))
    {
      int abi = bfd_elf_get_obj_attr_int (abfd, OBJ_ATTR_PROC, Tag_ABI_VFP_args);
      if (abi == AEABI_VFP_args_vfp)
	i_ehdrp->e_flags |= EF_ARM_ABI_FLOAT_HARD;
      else
	i_ehdrp->e_flags |= EF_ARM_ABI_FLOAT_SOFT;
    }

  /* Scan segment to set p_flags attribute if it contains only sections with
     SHF_ARM_PURECODE flag.  */
  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
    {
      unsigned int j;

      if (m->count == 0)
	continue;
      for (j = 0; j < m->count; j++)
	{
	  if (!(elf_section_flags (m->sections[j]) & SHF_ARM_PURECODE))
	    break;
	}
      if (j == m->count)
	{
	  m->p_flags = PF_X;
	  m->p_flags_valid = 1;
	}
    }
  return true;
}

static enum elf_reloc_type_class
elf32_arm_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			    const asection *rel_sec ATTRIBUTE_UNUSED,
			    const Elf_Internal_Rela *rela)
{
  struct elf32_arm_link_hash_table *htab = elf32_arm_hash_table (info);

  if (htab->root.dynsym != NULL
      && htab->root.dynsym->contents != NULL)
    {
      /* Check relocation against STT_GNU_IFUNC symbol if there are
	 dynamic symbols.  */
      bfd *abfd = info->output_bfd;
      const struct elf_backend_data *bed = get_elf_backend_data (abfd);
      unsigned long r_symndx = ELF32_R_SYM (rela->r_info);
      if (r_symndx != STN_UNDEF)
	{
	  Elf_Internal_Sym sym;
	  if (!bed->s->swap_symbol_in (abfd,
				       (htab->root.dynsym->contents
					+ r_symndx * bed->s->sizeof_sym),
				       0, &sym))
	    {
	      /* xgettext:c-format */
	      _bfd_error_handler (_("%pB symbol number %lu references"
				    " nonexistent SHT_SYMTAB_SHNDX section"),
				  abfd, r_symndx);
	      /* Ideally an error class should be returned here.  */
	    }
	  else if (ELF_ST_TYPE (sym.st_info) == STT_GNU_IFUNC)
	    return reloc_class_ifunc;
	}
    }

  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_ARM_RELATIVE:
      return reloc_class_relative;
    case R_ARM_JUMP_SLOT:
      return reloc_class_plt;
    case R_ARM_COPY:
      return reloc_class_copy;
    case R_ARM_IRELATIVE:
      return reloc_class_ifunc;
    default:
      return reloc_class_normal;
    }
}

static void
arm_final_write_processing (bfd *abfd)
{
  bfd_arm_update_notes (abfd, ARM_NOTE_SECTION);
}

static bool
elf32_arm_final_write_processing (bfd *abfd)
{
  arm_final_write_processing (abfd);
  return _bfd_elf_final_write_processing (abfd);
}

/* Return TRUE if this is an unwinding table entry.  */

static bool
is_arm_elf_unwind_section_name (bfd * abfd ATTRIBUTE_UNUSED, const char * name)
{
  return (startswith (name, ELF_STRING_ARM_unwind)
	  || startswith (name, ELF_STRING_ARM_unwind_once));
}


/* Set the type and flags for an ARM section.  We do this by
   the section name, which is a hack, but ought to work.  */

static bool
elf32_arm_fake_sections (bfd * abfd, Elf_Internal_Shdr * hdr, asection * sec)
{
  const char * name;

  name = bfd_section_name (sec);

  if (is_arm_elf_unwind_section_name (abfd, name))
    {
      hdr->sh_type = SHT_ARM_EXIDX;
      hdr->sh_flags |= SHF_LINK_ORDER;
    }

  if (sec->flags & SEC_ELF_PURECODE)
    hdr->sh_flags |= SHF_ARM_PURECODE;

  return true;
}

/* Handle an ARM specific section when reading an object file.  This is
   called when bfd_section_from_shdr finds a section with an unknown
   type.  */

static bool
elf32_arm_section_from_shdr (bfd *abfd,
			     Elf_Internal_Shdr * hdr,
			     const char *name,
			     int shindex)
{
  /* There ought to be a place to keep ELF backend specific flags, but
     at the moment there isn't one.  We just keep track of the
     sections by their name, instead.  Fortunately, the ABI gives
     names for all the ARM specific sections, so we will probably get
     away with this.  */
  switch (hdr->sh_type)
    {
    case SHT_ARM_EXIDX:
    case SHT_ARM_PREEMPTMAP:
    case SHT_ARM_ATTRIBUTES:
      break;

    default:
      return false;
    }

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return false;

  return true;
}

static _arm_elf_section_data *
get_arm_elf_section_data (asection * sec)
{
  if (sec && sec->owner && is_arm_elf (sec->owner))
    return elf32_arm_section_data (sec);
  else
    return NULL;
}

typedef struct
{
  void *flaginfo;
  struct bfd_link_info *info;
  asection *sec;
  int sec_shndx;
  int (*func) (void *, const char *, Elf_Internal_Sym *,
	       asection *, struct elf_link_hash_entry *);
} output_arch_syminfo;

enum map_symbol_type
{
  ARM_MAP_ARM,
  ARM_MAP_THUMB,
  ARM_MAP_DATA
};


/* Output a single mapping symbol.  */

static bool
elf32_arm_output_map_sym (output_arch_syminfo *osi,
			  enum map_symbol_type type,
			  bfd_vma offset)
{
  static const char *names[3] = {"$a", "$t", "$d"};
  Elf_Internal_Sym sym;

  sym.st_value = osi->sec->output_section->vma
		 + osi->sec->output_offset
		 + offset;
  sym.st_size = 0;
  sym.st_other = 0;
  sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_NOTYPE);
  sym.st_shndx = osi->sec_shndx;
  sym.st_target_internal = 0;
  elf32_arm_section_map_add (osi->sec, names[type][1], offset);
  return osi->func (osi->flaginfo, names[type], &sym, osi->sec, NULL) == 1;
}

/* Output mapping symbols for the PLT entry described by ROOT_PLT and ARM_PLT.
   IS_IPLT_ENTRY_P says whether the PLT is in .iplt rather than .plt.  */

static bool
elf32_arm_output_plt_map_1 (output_arch_syminfo *osi,
			    bool is_iplt_entry_p,
			    union gotplt_union *root_plt,
			    struct arm_plt_info *arm_plt)
{
  struct elf32_arm_link_hash_table *htab;
  bfd_vma addr, plt_header_size;

  if (root_plt->offset == (bfd_vma) -1)
    return true;

  htab = elf32_arm_hash_table (osi->info);
  if (htab == NULL)
    return false;

  if (is_iplt_entry_p)
    {
      osi->sec = htab->root.iplt;
      plt_header_size = 0;
    }
  else
    {
      osi->sec = htab->root.splt;
      plt_header_size = htab->plt_header_size;
    }
  osi->sec_shndx = (_bfd_elf_section_from_bfd_section
		    (osi->info->output_bfd, osi->sec->output_section));

  addr = root_plt->offset & -2;
  if (htab->root.target_os == is_vxworks)
    {
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_ARM, addr))
	return false;
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_DATA, addr + 8))
	return false;
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_ARM, addr + 12))
	return false;
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_DATA, addr + 20))
	return false;
    }
  else if (htab->root.target_os == is_nacl)
    {
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_ARM, addr))
	return false;
    }
  else if (htab->fdpic_p)
    {
      enum map_symbol_type type = using_thumb_only (htab)
	? ARM_MAP_THUMB
	: ARM_MAP_ARM;

      if (elf32_arm_plt_needs_thumb_stub_p (osi->info, arm_plt))
	if (!elf32_arm_output_map_sym (osi, ARM_MAP_THUMB, addr - 4))
	  return false;
      if (!elf32_arm_output_map_sym (osi, type, addr))
	return false;
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_DATA, addr + 16))
	return false;
      if (htab->plt_entry_size == 4 * ARRAY_SIZE (elf32_arm_fdpic_plt_entry))
	if (!elf32_arm_output_map_sym (osi, type, addr + 24))
	  return false;
    }
  else if (using_thumb_only (htab))
    {
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_THUMB, addr))
	return false;
    }
  else
    {
      bool thumb_stub_p;

      thumb_stub_p = elf32_arm_plt_needs_thumb_stub_p (osi->info, arm_plt);
      if (thumb_stub_p)
	{
	  if (!elf32_arm_output_map_sym (osi, ARM_MAP_THUMB, addr - 4))
	    return false;
	}
#ifdef FOUR_WORD_PLT
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_ARM, addr))
	return false;
      if (!elf32_arm_output_map_sym (osi, ARM_MAP_DATA, addr + 12))
	return false;
#else
      /* A three-word PLT with no Thumb thunk contains only Arm code,
	 so only need to output a mapping symbol for the first PLT entry and
	 entries with thumb thunks.  */
      if (thumb_stub_p || addr == plt_header_size)
	{
	  if (!elf32_arm_output_map_sym (osi, ARM_MAP_ARM, addr))
	    return false;
	}
#endif
    }

  return true;
}

/* Output mapping symbols for PLT entries associated with H.  */

static bool
elf32_arm_output_plt_map (struct elf_link_hash_entry *h, void *inf)
{
  output_arch_syminfo *osi = (output_arch_syminfo *) inf;
  struct elf32_arm_link_hash_entry *eh;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  eh = (struct elf32_arm_link_hash_entry *) h;
  return elf32_arm_output_plt_map_1 (osi, SYMBOL_CALLS_LOCAL (osi->info, h),
				     &h->plt, &eh->plt);
}

/* Bind a veneered symbol to its veneer identified by its hash entry
   STUB_ENTRY.  The veneered location thus loose its symbol.  */

static void
arm_stub_claim_sym (struct elf32_arm_stub_hash_entry *stub_entry)
{
  struct elf32_arm_link_hash_entry *hash = stub_entry->h;

  BFD_ASSERT (hash);
  hash->root.root.u.def.section = stub_entry->stub_sec;
  hash->root.root.u.def.value = stub_entry->stub_offset;
  hash->root.size = stub_entry->stub_size;
}

/* Output a single local symbol for a generated stub.  */

static bool
elf32_arm_output_stub_sym (output_arch_syminfo *osi, const char *name,
			   bfd_vma offset, bfd_vma size)
{
  Elf_Internal_Sym sym;

  sym.st_value = osi->sec->output_section->vma
		 + osi->sec->output_offset
		 + offset;
  sym.st_size = size;
  sym.st_other = 0;
  sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  sym.st_shndx = osi->sec_shndx;
  sym.st_target_internal = 0;
  return osi->func (osi->flaginfo, name, &sym, osi->sec, NULL) == 1;
}

static bool
arm_map_one_stub (struct bfd_hash_entry * gen_entry,
		  void * in_arg)
{
  struct elf32_arm_stub_hash_entry *stub_entry;
  asection *stub_sec;
  bfd_vma addr;
  char *stub_name;
  output_arch_syminfo *osi;
  const insn_sequence *template_sequence;
  enum stub_insn_type prev_type;
  int size;
  int i;
  enum map_symbol_type sym_type;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_arm_stub_hash_entry *) gen_entry;
  osi = (output_arch_syminfo *) in_arg;

  stub_sec = stub_entry->stub_sec;

  /* Ensure this stub is attached to the current section being
     processed.  */
  if (stub_sec != osi->sec)
    return true;

  addr = (bfd_vma) stub_entry->stub_offset;
  template_sequence = stub_entry->stub_template;

  if (arm_stub_sym_claimed (stub_entry->stub_type))
    arm_stub_claim_sym (stub_entry);
  else
    {
      stub_name = stub_entry->output_name;
      switch (template_sequence[0].type)
	{
	case ARM_TYPE:
	  if (!elf32_arm_output_stub_sym (osi, stub_name, addr,
					  stub_entry->stub_size))
	    return false;
	  break;
	case THUMB16_TYPE:
	case THUMB32_TYPE:
	  if (!elf32_arm_output_stub_sym (osi, stub_name, addr | 1,
					  stub_entry->stub_size))
	    return false;
	  break;
	default:
	  BFD_FAIL ();
	  return 0;
	}
    }

  prev_type = DATA_TYPE;
  size = 0;
  for (i = 0; i < stub_entry->stub_template_size; i++)
    {
      switch (template_sequence[i].type)
	{
	case ARM_TYPE:
	  sym_type = ARM_MAP_ARM;
	  break;

	case THUMB16_TYPE:
	case THUMB32_TYPE:
	  sym_type = ARM_MAP_THUMB;
	  break;

	case DATA_TYPE:
	  sym_type = ARM_MAP_DATA;
	  break;

	default:
	  BFD_FAIL ();
	  return false;
	}

      if (template_sequence[i].type != prev_type)
	{
	  prev_type = template_sequence[i].type;
	  if (!elf32_arm_output_map_sym (osi, sym_type, addr + size))
	    return false;
	}

      switch (template_sequence[i].type)
	{
	case ARM_TYPE:
	case THUMB32_TYPE:
	  size += 4;
	  break;

	case THUMB16_TYPE:
	  size += 2;
	  break;

	case DATA_TYPE:
	  size += 4;
	  break;

	default:
	  BFD_FAIL ();
	  return false;
	}
    }

  return true;
}

/* Output mapping symbols for linker generated sections,
   and for those data-only sections that do not have a
   $d.  */

static bool
elf32_arm_output_arch_local_syms (bfd *output_bfd,
				  struct bfd_link_info *info,
				  void *flaginfo,
				  int (*func) (void *, const char *,
					       Elf_Internal_Sym *,
					       asection *,
					       struct elf_link_hash_entry *))
{
  output_arch_syminfo osi;
  struct elf32_arm_link_hash_table *htab;
  bfd_vma offset;
  bfd_size_type size;
  bfd *input_bfd;

  if (info->strip == strip_all
      && !info->emitrelocations
      && !bfd_link_relocatable (info))
    return true;

  htab = elf32_arm_hash_table (info);
  if (htab == NULL)
    return false;

  check_use_blx (htab);

  osi.flaginfo = flaginfo;
  osi.info = info;
  osi.func = func;

  /* Add a $d mapping symbol to data-only sections that
     don't have any mapping symbol.  This may result in (harmless) redundant
     mapping symbols.  */
  for (input_bfd = info->input_bfds;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next)
    {
      if ((input_bfd->flags & (BFD_LINKER_CREATED | HAS_SYMS)) == HAS_SYMS)
	for (osi.sec = input_bfd->sections;
	     osi.sec != NULL;
	     osi.sec = osi.sec->next)
	  {
	    if (osi.sec->output_section != NULL
		&& ((osi.sec->output_section->flags & (SEC_ALLOC | SEC_CODE))
		    != 0)
		&& (osi.sec->flags & (SEC_HAS_CONTENTS | SEC_LINKER_CREATED))
		   == SEC_HAS_CONTENTS
		&& get_arm_elf_section_data (osi.sec) != NULL
		&& get_arm_elf_section_data (osi.sec)->mapcount == 0
		&& osi.sec->size > 0
		&& (osi.sec->flags & SEC_EXCLUDE) == 0)
	      {
		osi.sec_shndx = _bfd_elf_section_from_bfd_section
		  (output_bfd, osi.sec->output_section);
		if (osi.sec_shndx != (int)SHN_BAD)
		  elf32_arm_output_map_sym (&osi, ARM_MAP_DATA, 0);
	      }
	  }
    }

  /* ARM->Thumb glue.  */
  if (htab->arm_glue_size > 0)
    {
      osi.sec = bfd_get_linker_section (htab->bfd_of_glue_owner,
					ARM2THUMB_GLUE_SECTION_NAME);

      osi.sec_shndx = _bfd_elf_section_from_bfd_section
	  (output_bfd, osi.sec->output_section);
      if (bfd_link_pic (info) || htab->root.is_relocatable_executable
	  || htab->pic_veneer)
	size = ARM2THUMB_PIC_GLUE_SIZE;
      else if (htab->use_blx)
	size = ARM2THUMB_V5_STATIC_GLUE_SIZE;
      else
	size = ARM2THUMB_STATIC_GLUE_SIZE;

      for (offset = 0; offset < htab->arm_glue_size; offset += size)
	{
	  elf32_arm_output_map_sym (&osi, ARM_MAP_ARM, offset);
	  elf32_arm_output_map_sym (&osi, ARM_MAP_DATA, offset + size - 4);
	}
    }

  /* Thumb->ARM glue.  */
  if (htab->thumb_glue_size > 0)
    {
      osi.sec = bfd_get_linker_section (htab->bfd_of_glue_owner,
					THUMB2ARM_GLUE_SECTION_NAME);

      osi.sec_shndx = _bfd_elf_section_from_bfd_section
	  (output_bfd, osi.sec->output_section);
      size = THUMB2ARM_GLUE_SIZE;

      for (offset = 0; offset < htab->thumb_glue_size; offset += size)
	{
	  elf32_arm_output_map_sym (&osi, ARM_MAP_THUMB, offset);
	  elf32_arm_output_map_sym (&osi, ARM_MAP_ARM, offset + 4);
	}
    }

  /* ARMv4 BX veneers.  */
  if (htab->bx_glue_size > 0)
    {
      osi.sec = bfd_get_linker_section (htab->bfd_of_glue_owner,
					ARM_BX_GLUE_SECTION_NAME);

      osi.sec_shndx = _bfd_elf_section_from_bfd_section
	  (output_bfd, osi.sec->output_section);

      elf32_arm_output_map_sym (&osi, ARM_MAP_ARM, 0);
    }

  /* Long calls stubs.  */
  if (htab->stub_bfd && htab->stub_bfd->sections)
    {
      asection* stub_sec;

      for (stub_sec = htab->stub_bfd->sections;
	   stub_sec != NULL;
	   stub_sec = stub_sec->next)
	{
	  /* Ignore non-stub sections.  */
	  if (!strstr (stub_sec->name, STUB_SUFFIX))
	    continue;

	  osi.sec = stub_sec;

	  osi.sec_shndx = _bfd_elf_section_from_bfd_section
	    (output_bfd, osi.sec->output_section);

	  bfd_hash_traverse (&htab->stub_hash_table, arm_map_one_stub, &osi);
	}
    }

  /* Finally, output mapping symbols for the PLT.  */
  if (htab->root.splt && htab->root.splt->size > 0)
    {
      osi.sec = htab->root.splt;
      osi.sec_shndx = (_bfd_elf_section_from_bfd_section
		       (output_bfd, osi.sec->output_section));

      /* Output mapping symbols for the plt header.  */
      if (htab->root.target_os == is_vxworks)
	{
	  /* VxWorks shared libraries have no PLT header.  */
	  if (!bfd_link_pic (info))
	    {
	      if (!elf32_arm_output_map_sym (&osi, ARM_MAP_ARM, 0))
		return false;
	      if (!elf32_arm_output_map_sym (&osi, ARM_MAP_DATA, 12))
		return false;
	    }
	}
      else if (htab->root.target_os == is_nacl)
	{
	  if (!elf32_arm_output_map_sym (&osi, ARM_MAP_ARM, 0))
	    return false;
	}
      else if (using_thumb_only (htab) && !htab->fdpic_p)
	{
	  if (!elf32_arm_output_map_sym (&osi, ARM_MAP_THUMB, 0))
	    return false;
	  if (!elf32_arm_output_map_sym (&osi, ARM_MAP_DATA, 12))
	    return false;
	  if (!elf32_arm_output_map_sym (&osi, ARM_MAP_THUMB, 16))
	    return false;
	}
      else if (!htab->fdpic_p)
	{
	  if (!elf32_arm_output_map_sym (&osi, ARM_MAP_ARM, 0))
	    return false;
#ifndef FOUR_WORD_PLT
	  if (!elf32_arm_output_map_sym (&osi, ARM_MAP_DATA, 16))
	    return false;
#endif
	}
    }
  if (htab->root.target_os == is_nacl
      && htab->root.iplt
      && htab->root.iplt->size > 0)
    {
      /* NaCl uses a special first entry in .iplt too.  */
      osi.sec = htab->root.iplt;
      osi.sec_shndx = (_bfd_elf_section_from_bfd_section
		       (output_bfd, osi.sec->output_section));
      if (!elf32_arm_output_map_sym (&osi, ARM_MAP_ARM, 0))
	return false;
    }
  if ((htab->root.splt && htab->root.splt->size > 0)
      || (htab->root.iplt && htab->root.iplt->size > 0))
    {
      elf_link_hash_traverse (&htab->root, elf32_arm_output_plt_map, &osi);
      for (input_bfd = info->input_bfds;
	   input_bfd != NULL;
	   input_bfd = input_bfd->link.next)
	{
	  struct arm_local_iplt_info **local_iplt;
	  unsigned int i, num_syms;

	  local_iplt = elf32_arm_local_iplt (input_bfd);
	  if (local_iplt != NULL)
	    {
	      num_syms = elf_symtab_hdr (input_bfd).sh_info;
	      if (num_syms > elf32_arm_num_entries (input_bfd))
		{
		  _bfd_error_handler (_("\
%pB: Number of symbols in input file has increased from %lu to %u\n"),
				      input_bfd,
				      (unsigned long) elf32_arm_num_entries (input_bfd),
				      num_syms);
		  return false;
		}
	      for (i = 0; i < num_syms; i++)
		if (local_iplt[i] != NULL
		    && !elf32_arm_output_plt_map_1 (&osi, true,
						    &local_iplt[i]->root,
						    &local_iplt[i]->arm))
		  return false;
	    }
	}
    }
  if (htab->root.tlsdesc_plt != 0)
    {
      /* Mapping symbols for the lazy tls trampoline.  */
      if (!elf32_arm_output_map_sym (&osi, ARM_MAP_ARM,
				     htab->root.tlsdesc_plt))
	return false;

      if (!elf32_arm_output_map_sym (&osi, ARM_MAP_DATA,
				     htab->root.tlsdesc_plt + 24))
	return false;
    }
  if (htab->tls_trampoline != 0)
    {
      /* Mapping symbols for the tls trampoline.  */
      if (!elf32_arm_output_map_sym (&osi, ARM_MAP_ARM, htab->tls_trampoline))
	return false;
#ifdef FOUR_WORD_PLT
      if (!elf32_arm_output_map_sym (&osi, ARM_MAP_DATA,
				     htab->tls_trampoline + 12))
	return false;
#endif
    }

  return true;
}

/* Filter normal symbols of CMSE entry functions of ABFD to include in
   the import library.  All SYMCOUNT symbols of ABFD can be examined
   from their pointers in SYMS.  Pointers of symbols to keep should be
   stored continuously at the beginning of that array.

   Returns the number of symbols to keep.  */

static unsigned int
elf32_arm_filter_cmse_symbols (bfd *abfd ATTRIBUTE_UNUSED,
			       struct bfd_link_info *info,
			       asymbol **syms, long symcount)
{
  size_t maxnamelen;
  char *cmse_name;
  long src_count, dst_count = 0;
  struct elf32_arm_link_hash_table *htab;

  htab = elf32_arm_hash_table (info);
  if (!htab->stub_bfd || !htab->stub_bfd->sections)
    symcount = 0;

  maxnamelen = 128;
  cmse_name = (char *) bfd_malloc (maxnamelen);
  BFD_ASSERT (cmse_name);

  for (src_count = 0; src_count < symcount; src_count++)
    {
      struct elf32_arm_link_hash_entry *cmse_hash;
      asymbol *sym;
      flagword flags;
      char *name;
      size_t namelen;

      sym = syms[src_count];
      flags = sym->flags;
      name = (char *) bfd_asymbol_name (sym);

      if ((flags & BSF_FUNCTION) != BSF_FUNCTION)
	continue;
      if (!(flags & (BSF_GLOBAL | BSF_WEAK)))
	continue;

      namelen = strlen (name) + sizeof (CMSE_PREFIX) + 1;
      if (namelen > maxnamelen)
	{
	  cmse_name = (char *)
	    bfd_realloc (cmse_name, namelen);
	  maxnamelen = namelen;
	}
      snprintf (cmse_name, maxnamelen, "%s%s", CMSE_PREFIX, name);
      cmse_hash = (struct elf32_arm_link_hash_entry *)
	elf_link_hash_lookup (&(htab)->root, cmse_name, false, false, true);

      if (!cmse_hash
	  || (cmse_hash->root.root.type != bfd_link_hash_defined
	      && cmse_hash->root.root.type != bfd_link_hash_defweak)
	  || cmse_hash->root.type != STT_FUNC)
	continue;

      syms[dst_count++] = sym;
    }
  free (cmse_name);

  syms[dst_count] = NULL;

  return dst_count;
}

/* Filter symbols of ABFD to include in the import library.  All
   SYMCOUNT symbols of ABFD can be examined from their pointers in
   SYMS.  Pointers of symbols to keep should be stored continuously at
   the beginning of that array.

   Returns the number of symbols to keep.  */

static unsigned int
elf32_arm_filter_implib_symbols (bfd *abfd ATTRIBUTE_UNUSED,
				 struct bfd_link_info *info,
				 asymbol **syms, long symcount)
{
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (info);

  /* Requirement 8 of "ARM v8-M Security Extensions: Requirements on
     Development Tools" (ARM-ECM-0359818) mandates Secure Gateway import
     library to be a relocatable object file.  */
  BFD_ASSERT (!(bfd_get_file_flags (info->out_implib_bfd) & EXEC_P));
  if (globals->cmse_implib)
    return elf32_arm_filter_cmse_symbols (abfd, info, syms, symcount);
  else
    return _bfd_elf_filter_global_symbols (abfd, info, syms, symcount);
}

/* Allocate target specific section data.  */

static bool
elf32_arm_new_section_hook (bfd *abfd, asection *sec)
{
  if (!sec->used_by_bfd)
    {
      _arm_elf_section_data *sdata;
      size_t amt = sizeof (*sdata);

      sdata = (_arm_elf_section_data *) bfd_zalloc (abfd, amt);
      if (sdata == NULL)
	return false;
      sec->used_by_bfd = sdata;
    }

  return _bfd_elf_new_section_hook (abfd, sec);
}


/* Used to order a list of mapping symbols by address.  */

static int
elf32_arm_compare_mapping (const void * a, const void * b)
{
  const elf32_arm_section_map *amap = (const elf32_arm_section_map *) a;
  const elf32_arm_section_map *bmap = (const elf32_arm_section_map *) b;

  if (amap->vma > bmap->vma)
    return 1;
  else if (amap->vma < bmap->vma)
    return -1;
  else if (amap->type > bmap->type)
    /* Ensure results do not depend on the host qsort for objects with
       multiple mapping symbols at the same address by sorting on type
       after vma.  */
    return 1;
  else if (amap->type < bmap->type)
    return -1;
  else
    return 0;
}

/* Add OFFSET to lower 31 bits of ADDR, leaving other bits unmodified.  */

static unsigned long
offset_prel31 (unsigned long addr, bfd_vma offset)
{
  return (addr & ~0x7ffffffful) | ((addr + offset) & 0x7ffffffful);
}

/* Copy an .ARM.exidx table entry, adding OFFSET to (applied) PREL31
   relocations.  */

static void
copy_exidx_entry (bfd *output_bfd, bfd_byte *to, bfd_byte *from, bfd_vma offset)
{
  unsigned long first_word = bfd_get_32 (output_bfd, from);
  unsigned long second_word = bfd_get_32 (output_bfd, from + 4);

  /* High bit of first word is supposed to be zero.  */
  if ((first_word & 0x80000000ul) == 0)
    first_word = offset_prel31 (first_word, offset);

  /* If the high bit of the first word is clear, and the bit pattern is not 0x1
     (EXIDX_CANTUNWIND), this is an offset to an .ARM.extab entry.  */
  if ((second_word != 0x1) && ((second_word & 0x80000000ul) == 0))
    second_word = offset_prel31 (second_word, offset);

  bfd_put_32 (output_bfd, first_word, to);
  bfd_put_32 (output_bfd, second_word, to + 4);
}

/* Data for make_branch_to_a8_stub().  */

struct a8_branch_to_stub_data
{
  asection *writing_section;
  bfd_byte *contents;
};


/* Helper to insert branches to Cortex-A8 erratum stubs in the right
   places for a particular section.  */

static bool
make_branch_to_a8_stub (struct bfd_hash_entry *gen_entry,
		       void *in_arg)
{
  struct elf32_arm_stub_hash_entry *stub_entry;
  struct a8_branch_to_stub_data *data;
  bfd_byte *contents;
  unsigned long branch_insn;
  bfd_vma veneered_insn_loc, veneer_entry_loc;
  bfd_signed_vma branch_offset;
  bfd *abfd;
  unsigned int loc;

  stub_entry = (struct elf32_arm_stub_hash_entry *) gen_entry;
  data = (struct a8_branch_to_stub_data *) in_arg;

  if (stub_entry->target_section != data->writing_section
      || stub_entry->stub_type < arm_stub_a8_veneer_lwm)
    return true;

  contents = data->contents;

  /* We use target_section as Cortex-A8 erratum workaround stubs are only
     generated when both source and target are in the same section.  */
  veneered_insn_loc = stub_entry->target_section->output_section->vma
		      + stub_entry->target_section->output_offset
		      + stub_entry->source_value;

  veneer_entry_loc = stub_entry->stub_sec->output_section->vma
		     + stub_entry->stub_sec->output_offset
		     + stub_entry->stub_offset;

  if (stub_entry->stub_type == arm_stub_a8_veneer_blx)
    veneered_insn_loc &= ~3u;

  branch_offset = veneer_entry_loc - veneered_insn_loc - 4;

  abfd = stub_entry->target_section->owner;
  loc = stub_entry->source_value;

  /* We attempt to avoid this condition by setting stubs_always_after_branch
     in elf32_arm_size_stubs if we've enabled the Cortex-A8 erratum workaround.
     This check is just to be on the safe side...  */
  if ((veneered_insn_loc & ~0xfff) == (veneer_entry_loc & ~0xfff))
    {
      _bfd_error_handler (_("%pB: error: Cortex-A8 erratum stub is "
			    "allocated in unsafe location"), abfd);
      return false;
    }

  switch (stub_entry->stub_type)
    {
    case arm_stub_a8_veneer_b:
    case arm_stub_a8_veneer_b_cond:
      branch_insn = 0xf0009000;
      goto jump24;

    case arm_stub_a8_veneer_blx:
      branch_insn = 0xf000e800;
      goto jump24;

    case arm_stub_a8_veneer_bl:
      {
	unsigned int i1, j1, i2, j2, s;

	branch_insn = 0xf000d000;

      jump24:
	if (branch_offset < -16777216 || branch_offset > 16777214)
	  {
	    /* There's not much we can do apart from complain if this
	       happens.  */
	    _bfd_error_handler (_("%pB: error: Cortex-A8 erratum stub out "
				  "of range (input file too large)"), abfd);
	    return false;
	  }

	/* i1 = not(j1 eor s), so:
	   not i1 = j1 eor s
	   j1 = (not i1) eor s.  */

	branch_insn |= (branch_offset >> 1) & 0x7ff;
	branch_insn |= ((branch_offset >> 12) & 0x3ff) << 16;
	i2 = (branch_offset >> 22) & 1;
	i1 = (branch_offset >> 23) & 1;
	s = (branch_offset >> 24) & 1;
	j1 = (!i1) ^ s;
	j2 = (!i2) ^ s;
	branch_insn |= j2 << 11;
	branch_insn |= j1 << 13;
	branch_insn |= s << 26;
      }
      break;

    default:
      BFD_FAIL ();
      return false;
    }

  bfd_put_16 (abfd, (branch_insn >> 16) & 0xffff, &contents[loc]);
  bfd_put_16 (abfd, branch_insn & 0xffff, &contents[loc + 2]);

  return true;
}

/* Beginning of stm32l4xx work-around.  */

/* Functions encoding instructions necessary for the emission of the
   fix-stm32l4xx-629360.
   Encoding is extracted from the
   ARM (C) Architecture Reference Manual
   ARMv7-A and ARMv7-R edition
   ARM DDI 0406C.b (ID072512).  */

static inline bfd_vma
create_instruction_branch_absolute (int branch_offset)
{
  /* A8.8.18 B (A8-334)
     B target_address (Encoding T4).  */
  /* 1111 - 0Sii - iiii - iiii - 10J1 - Jiii - iiii - iiii.  */
  /* jump offset is:  S:I1:I2:imm10:imm11:0.  */
  /* with : I1 = NOT (J1 EOR S) I2 = NOT (J2 EOR S).  */

  int s = ((branch_offset & 0x1000000) >> 24);
  int j1 = s ^ !((branch_offset & 0x800000) >> 23);
  int j2 = s ^ !((branch_offset & 0x400000) >> 22);

  if (branch_offset < -(1 << 24) || branch_offset >= (1 << 24))
    BFD_ASSERT (0 && "Error: branch out of range.  Cannot create branch.");

  bfd_vma patched_inst = 0xf0009000
    | s << 26 /* S.  */
    | (((unsigned long) (branch_offset) >> 12) & 0x3ff) << 16 /* imm10.  */
    | j1 << 13 /* J1.  */
    | j2 << 11 /* J2.  */
    | (((unsigned long) (branch_offset) >> 1) & 0x7ff); /* imm11.  */

  return patched_inst;
}

static inline bfd_vma
create_instruction_ldmia (int base_reg, int wback, int reg_mask)
{
  /* A8.8.57 LDM/LDMIA/LDMFD (A8-396)
     LDMIA Rn!, {Ra, Rb, Rc, ...} (Encoding T2).  */
  bfd_vma patched_inst = 0xe8900000
    | (/*W=*/wback << 21)
    | (base_reg << 16)
    | (reg_mask & 0x0000ffff);

  return patched_inst;
}

static inline bfd_vma
create_instruction_ldmdb (int base_reg, int wback, int reg_mask)
{
  /* A8.8.60 LDMDB/LDMEA (A8-402)
     LDMDB Rn!, {Ra, Rb, Rc, ...} (Encoding T1).  */
  bfd_vma patched_inst = 0xe9100000
    | (/*W=*/wback << 21)
    | (base_reg << 16)
    | (reg_mask & 0x0000ffff);

  return patched_inst;
}

static inline bfd_vma
create_instruction_mov (int target_reg, int source_reg)
{
  /* A8.8.103 MOV (register) (A8-486)
     MOV Rd, Rm (Encoding T1).  */
  bfd_vma patched_inst = 0x4600
    | (target_reg & 0x7)
    | ((target_reg & 0x8) >> 3) << 7
    | (source_reg << 3);

  return patched_inst;
}

static inline bfd_vma
create_instruction_sub (int target_reg, int source_reg, int value)
{
  /* A8.8.221 SUB (immediate) (A8-708)
     SUB Rd, Rn, #value (Encoding T3).  */
  bfd_vma patched_inst = 0xf1a00000
    | (target_reg << 8)
    | (source_reg << 16)
    | (/*S=*/0 << 20)
    | ((value & 0x800) >> 11) << 26
    | ((value & 0x700) >>  8) << 12
    | (value & 0x0ff);

  return patched_inst;
}

static inline bfd_vma
create_instruction_vldmia (int base_reg, int is_dp, int wback, int num_words,
			   int first_reg)
{
  /* A8.8.332 VLDM (A8-922)
     VLMD{MODE} Rn{!}, {list} (Encoding T1 or T2).  */
  bfd_vma patched_inst = (is_dp ? 0xec900b00 : 0xec900a00)
    | (/*W=*/wback << 21)
    | (base_reg << 16)
    | (num_words & 0x000000ff)
    | (((unsigned)first_reg >> 1) & 0x0000000f) << 12
    | (first_reg & 0x00000001) << 22;

  return patched_inst;
}

static inline bfd_vma
create_instruction_vldmdb (int base_reg, int is_dp, int num_words,
			   int first_reg)
{
  /* A8.8.332 VLDM (A8-922)
     VLMD{MODE} Rn!, {} (Encoding T1 or T2).  */
  bfd_vma patched_inst = (is_dp ? 0xed300b00 : 0xed300a00)
    | (base_reg << 16)
    | (num_words & 0x000000ff)
    | (((unsigned)first_reg >>1 ) & 0x0000000f) << 12
    | (first_reg & 0x00000001) << 22;

  return patched_inst;
}

static inline bfd_vma
create_instruction_udf_w (int value)
{
  /* A8.8.247 UDF (A8-758)
     Undefined (Encoding T2).  */
  bfd_vma patched_inst = 0xf7f0a000
    | (value & 0x00000fff)
    | (value & 0x000f0000) << 16;

  return patched_inst;
}

static inline bfd_vma
create_instruction_udf (int value)
{
  /* A8.8.247 UDF (A8-758)
     Undefined (Encoding T1).  */
  bfd_vma patched_inst = 0xde00
    | (value & 0xff);

  return patched_inst;
}

/* Functions writing an instruction in memory, returning the next
   memory position to write to.  */

static inline bfd_byte *
push_thumb2_insn32 (struct elf32_arm_link_hash_table * htab,
		    bfd * output_bfd, bfd_byte *pt, insn32 insn)
{
  put_thumb2_insn (htab, output_bfd, insn, pt);
  return pt + 4;
}

static inline bfd_byte *
push_thumb2_insn16 (struct elf32_arm_link_hash_table * htab,
		    bfd * output_bfd, bfd_byte *pt, insn32 insn)
{
  put_thumb_insn (htab, output_bfd, insn, pt);
  return pt + 2;
}

/* Function filling up a region in memory with T1 and T2 UDFs taking
   care of alignment.  */

static bfd_byte *
stm32l4xx_fill_stub_udf (struct elf32_arm_link_hash_table * htab,
			 bfd *			 output_bfd,
			 const bfd_byte * const	 base_stub_contents,
			 bfd_byte * const	 from_stub_contents,
			 const bfd_byte * const	 end_stub_contents)
{
  bfd_byte *current_stub_contents = from_stub_contents;

  /* Fill the remaining of the stub with deterministic contents : UDF
     instructions.
     Check if realignment is needed on modulo 4 frontier using T1, to
     further use T2.  */
  if ((current_stub_contents < end_stub_contents)
      && !((current_stub_contents - base_stub_contents) % 2)
      && ((current_stub_contents - base_stub_contents) % 4))
    current_stub_contents =
      push_thumb2_insn16 (htab, output_bfd, current_stub_contents,
			  create_instruction_udf (0));

  for (; current_stub_contents < end_stub_contents;)
    current_stub_contents =
      push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			  create_instruction_udf_w (0));

  return current_stub_contents;
}

/* Functions writing the stream of instructions equivalent to the
   derived sequence for ldmia, ldmdb, vldm respectively.  */

static void
stm32l4xx_create_replacing_stub_ldmia (struct elf32_arm_link_hash_table * htab,
				       bfd * output_bfd,
				       const insn32 initial_insn,
				       const bfd_byte *const initial_insn_addr,
				       bfd_byte *const base_stub_contents)
{
  int wback = (initial_insn & 0x00200000) >> 21;
  int ri, rn = (initial_insn & 0x000F0000) >> 16;
  int insn_all_registers = initial_insn & 0x0000ffff;
  int insn_low_registers, insn_high_registers;
  int usable_register_mask;
  int nb_registers = elf32_arm_popcount (insn_all_registers);
  int restore_pc = (insn_all_registers & (1 << 15)) ? 1 : 0;
  int restore_rn = (insn_all_registers & (1 << rn)) ? 1 : 0;
  bfd_byte *current_stub_contents = base_stub_contents;

  BFD_ASSERT (is_thumb2_ldmia (initial_insn));

  /* In BFD_ARM_STM32L4XX_FIX_ALL mode we may have to deal with
     smaller than 8 registers load sequences that do not cause the
     hardware issue.  */
  if (nb_registers <= 8)
    {
      /* UNTOUCHED : LDMIA Rn{!}, {R-all-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    initial_insn);

      /* B initial_insn_addr+4.  */
      if (!restore_pc)
	current_stub_contents =
	  push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			      create_instruction_branch_absolute
			      (initial_insn_addr - current_stub_contents));

      /* Fill the remaining of the stub with deterministic contents.  */
      current_stub_contents =
	stm32l4xx_fill_stub_udf (htab, output_bfd,
				 base_stub_contents, current_stub_contents,
				 base_stub_contents +
				 STM32L4XX_ERRATUM_LDM_VENEER_SIZE);

      return;
    }

  /* - reg_list[13] == 0.  */
  BFD_ASSERT ((insn_all_registers & (1 << 13))==0);

  /* - reg_list[14] & reg_list[15] != 1.  */
  BFD_ASSERT ((insn_all_registers & 0xC000) != 0xC000);

  /* - if (wback==1) reg_list[rn] == 0.  */
  BFD_ASSERT (!wback || !restore_rn);

  /* - nb_registers > 8.  */
  BFD_ASSERT (elf32_arm_popcount (insn_all_registers) > 8);

  /* At this point, LDMxx initial insn loads between 9 and 14 registers.  */

  /* In the following algorithm, we split this wide LDM using 2 LDM insns:
    - One with the 7 lowest registers (register mask 0x007F)
      This LDM will finally contain between 2 and 7 registers
    - One with the 7 highest registers (register mask 0xDF80)
      This ldm will finally contain between 2 and 7 registers.  */
  insn_low_registers = insn_all_registers & 0x007F;
  insn_high_registers = insn_all_registers & 0xDF80;

  /* A spare register may be needed during this veneer to temporarily
     handle the base register.  This register will be restored with the
     last LDM operation.
     The usable register may be any general purpose register (that
     excludes PC, SP, LR : register mask is 0x1FFF).  */
  usable_register_mask = 0x1FFF;

  /* Generate the stub function.  */
  if (wback)
    {
      /* LDMIA Rn!, {R-low-register-list} : (Encoding T2).  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (rn, /*wback=*/1, insn_low_registers));

      /* LDMIA Rn!, {R-high-register-list} : (Encoding T2).  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (rn, /*wback=*/1, insn_high_registers));
      if (!restore_pc)
	{
	  /* B initial_insn_addr+4.  */
	  current_stub_contents =
	    push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
				create_instruction_branch_absolute
				(initial_insn_addr - current_stub_contents));
       }
    }
  else /* if (!wback).  */
    {
      ri = rn;

      /* If Rn is not part of the high-register-list, move it there.  */
      if (!(insn_high_registers & (1 << rn)))
	{
	  /* Choose a Ri in the high-register-list that will be restored.  */
	  ri = ctz (insn_high_registers & usable_register_mask & ~(1 << rn));

	  /* MOV Ri, Rn.  */
	  current_stub_contents =
	    push_thumb2_insn16 (htab, output_bfd, current_stub_contents,
				create_instruction_mov (ri, rn));
	}

      /* LDMIA Ri!, {R-low-register-list} : (Encoding T2).  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (ri, /*wback=*/1, insn_low_registers));

      /* LDMIA Ri, {R-high-register-list} : (Encoding T2).  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (ri, /*wback=*/0, insn_high_registers));

      if (!restore_pc)
	{
	  /* B initial_insn_addr+4.  */
	  current_stub_contents =
	    push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
				create_instruction_branch_absolute
				(initial_insn_addr - current_stub_contents));
	}
    }

  /* Fill the remaining of the stub with deterministic contents.  */
  current_stub_contents =
    stm32l4xx_fill_stub_udf (htab, output_bfd,
			     base_stub_contents, current_stub_contents,
			     base_stub_contents +
			     STM32L4XX_ERRATUM_LDM_VENEER_SIZE);
}

static void
stm32l4xx_create_replacing_stub_ldmdb (struct elf32_arm_link_hash_table * htab,
				       bfd * output_bfd,
				       const insn32 initial_insn,
				       const bfd_byte *const initial_insn_addr,
				       bfd_byte *const base_stub_contents)
{
  int wback = (initial_insn & 0x00200000) >> 21;
  int ri, rn = (initial_insn & 0x000f0000) >> 16;
  int insn_all_registers = initial_insn & 0x0000ffff;
  int insn_low_registers, insn_high_registers;
  int usable_register_mask;
  int restore_pc = (insn_all_registers & (1 << 15)) ? 1 : 0;
  int restore_rn = (insn_all_registers & (1 << rn)) ? 1 : 0;
  int nb_registers = elf32_arm_popcount (insn_all_registers);
  bfd_byte *current_stub_contents = base_stub_contents;

  BFD_ASSERT (is_thumb2_ldmdb (initial_insn));

  /* In BFD_ARM_STM32L4XX_FIX_ALL mode we may have to deal with
     smaller than 8 registers load sequences that do not cause the
     hardware issue.  */
  if (nb_registers <= 8)
    {
      /* UNTOUCHED : LDMIA Rn{!}, {R-all-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    initial_insn);

      /* B initial_insn_addr+4.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_branch_absolute
			    (initial_insn_addr - current_stub_contents));

      /* Fill the remaining of the stub with deterministic contents.  */
      current_stub_contents =
	stm32l4xx_fill_stub_udf (htab, output_bfd,
				 base_stub_contents, current_stub_contents,
				 base_stub_contents +
				 STM32L4XX_ERRATUM_LDM_VENEER_SIZE);

      return;
    }

  /* - reg_list[13] == 0.  */
  BFD_ASSERT ((insn_all_registers & (1 << 13)) == 0);

  /* - reg_list[14] & reg_list[15] != 1.  */
  BFD_ASSERT ((insn_all_registers & 0xC000) != 0xC000);

  /* - if (wback==1) reg_list[rn] == 0.  */
  BFD_ASSERT (!wback || !restore_rn);

  /* - nb_registers > 8.  */
  BFD_ASSERT (elf32_arm_popcount (insn_all_registers) > 8);

  /* At this point, LDMxx initial insn loads between 9 and 14 registers.  */

  /* In the following algorithm, we split this wide LDM using 2 LDM insn:
    - One with the 7 lowest registers (register mask 0x007F)
      This LDM will finally contain between 2 and 7 registers
    - One with the 7 highest registers (register mask 0xDF80)
      This ldm will finally contain between 2 and 7 registers.  */
  insn_low_registers = insn_all_registers & 0x007F;
  insn_high_registers = insn_all_registers & 0xDF80;

  /* A spare register may be needed during this veneer to temporarily
     handle the base register.  This register will be restored with
     the last LDM operation.
     The usable register may be any general purpose register (that excludes
     PC, SP, LR : register mask is 0x1FFF).  */
  usable_register_mask = 0x1FFF;

  /* Generate the stub function.  */
  if (!wback && !restore_pc && !restore_rn)
    {
      /* Choose a Ri in the low-register-list that will be restored.  */
      ri = ctz (insn_low_registers & usable_register_mask & ~(1 << rn));

      /* MOV Ri, Rn.  */
      current_stub_contents =
	push_thumb2_insn16 (htab, output_bfd, current_stub_contents,
			    create_instruction_mov (ri, rn));

      /* LDMDB Ri!, {R-high-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmdb
			    (ri, /*wback=*/1, insn_high_registers));

      /* LDMDB Ri, {R-low-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmdb
			    (ri, /*wback=*/0, insn_low_registers));

      /* B initial_insn_addr+4.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_branch_absolute
			    (initial_insn_addr - current_stub_contents));
    }
  else if (wback && !restore_pc && !restore_rn)
    {
      /* LDMDB Rn!, {R-high-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmdb
			    (rn, /*wback=*/1, insn_high_registers));

      /* LDMDB Rn!, {R-low-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmdb
			    (rn, /*wback=*/1, insn_low_registers));

      /* B initial_insn_addr+4.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_branch_absolute
			    (initial_insn_addr - current_stub_contents));
    }
  else if (!wback && restore_pc && !restore_rn)
    {
      /* Choose a Ri in the high-register-list that will be restored.  */
      ri = ctz (insn_high_registers & usable_register_mask & ~(1 << rn));

      /* SUB Ri, Rn, #(4*nb_registers).  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_sub (ri, rn, (4 * nb_registers)));

      /* LDMIA Ri!, {R-low-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (ri, /*wback=*/1, insn_low_registers));

      /* LDMIA Ri, {R-high-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (ri, /*wback=*/0, insn_high_registers));
    }
  else if (wback && restore_pc && !restore_rn)
    {
      /* Choose a Ri in the high-register-list that will be restored.  */
      ri = ctz (insn_high_registers & usable_register_mask & ~(1 << rn));

      /* SUB Rn, Rn, #(4*nb_registers)  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_sub (rn, rn, (4 * nb_registers)));

      /* MOV Ri, Rn.  */
      current_stub_contents =
	push_thumb2_insn16 (htab, output_bfd, current_stub_contents,
			    create_instruction_mov (ri, rn));

      /* LDMIA Ri!, {R-low-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (ri, /*wback=*/1, insn_low_registers));

      /* LDMIA Ri, {R-high-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (ri, /*wback=*/0, insn_high_registers));
    }
  else if (!wback && !restore_pc && restore_rn)
    {
      ri = rn;
      if (!(insn_low_registers & (1 << rn)))
	{
	  /* Choose a Ri in the low-register-list that will be restored.  */
	  ri = ctz (insn_low_registers & usable_register_mask & ~(1 << rn));

	  /* MOV Ri, Rn.  */
	  current_stub_contents =
	    push_thumb2_insn16 (htab, output_bfd, current_stub_contents,
				create_instruction_mov (ri, rn));
	}

      /* LDMDB Ri!, {R-high-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmdb
			    (ri, /*wback=*/1, insn_high_registers));

      /* LDMDB Ri, {R-low-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmdb
			    (ri, /*wback=*/0, insn_low_registers));

      /* B initial_insn_addr+4.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_branch_absolute
			    (initial_insn_addr - current_stub_contents));
    }
  else if (!wback && restore_pc && restore_rn)
    {
      ri = rn;
      if (!(insn_high_registers & (1 << rn)))
	{
	  /* Choose a Ri in the high-register-list that will be restored.  */
	  ri = ctz (insn_high_registers & usable_register_mask & ~(1 << rn));
	}

      /* SUB Ri, Rn, #(4*nb_registers).  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_sub (ri, rn, (4 * nb_registers)));

      /* LDMIA Ri!, {R-low-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (ri, /*wback=*/1, insn_low_registers));

      /* LDMIA Ri, {R-high-register-list}.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_ldmia
			    (ri, /*wback=*/0, insn_high_registers));
    }
  else if (wback && restore_rn)
    {
      /* The assembler should not have accepted to encode this.  */
      BFD_ASSERT (0 && "Cannot patch an instruction that has an "
	"undefined behavior.\n");
    }

  /* Fill the remaining of the stub with deterministic contents.  */
  current_stub_contents =
    stm32l4xx_fill_stub_udf (htab, output_bfd,
			     base_stub_contents, current_stub_contents,
			     base_stub_contents +
			     STM32L4XX_ERRATUM_LDM_VENEER_SIZE);

}

static void
stm32l4xx_create_replacing_stub_vldm (struct elf32_arm_link_hash_table * htab,
				      bfd * output_bfd,
				      const insn32 initial_insn,
				      const bfd_byte *const initial_insn_addr,
				      bfd_byte *const base_stub_contents)
{
  int num_words = initial_insn & 0xff;
  bfd_byte *current_stub_contents = base_stub_contents;

  BFD_ASSERT (is_thumb2_vldm (initial_insn));

  /* In BFD_ARM_STM32L4XX_FIX_ALL mode we may have to deal with
     smaller than 8 words load sequences that do not cause the
     hardware issue.  */
  if (num_words <= 8)
    {
      /* Untouched instruction.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    initial_insn);

      /* B initial_insn_addr+4.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_branch_absolute
			    (initial_insn_addr - current_stub_contents));
    }
  else
    {
      bool is_dp = /* DP encoding.  */
	(initial_insn & 0xfe100f00) == 0xec100b00;
      bool is_ia_nobang = /* (IA without !).  */
	(((initial_insn << 7) >> 28) & 0xd) == 0x4;
      bool is_ia_bang = /* (IA with !) - includes VPOP.  */
	(((initial_insn << 7) >> 28) & 0xd) == 0x5;
      bool is_db_bang = /* (DB with !).  */
	(((initial_insn << 7) >> 28) & 0xd) == 0x9;
      int base_reg = ((unsigned int) initial_insn << 12) >> 28;
      /* d = UInt (Vd:D);.  */
      int first_reg = ((((unsigned int) initial_insn << 16) >> 28) << 1)
	| (((unsigned int)initial_insn << 9) >> 31);

      /* Compute the number of 8-words chunks needed to split.  */
      int chunks = (num_words % 8) ? (num_words / 8 + 1) : (num_words / 8);
      int chunk;

      /* The test coverage has been done assuming the following
	 hypothesis that exactly one of the previous is_ predicates is
	 true.  */
      BFD_ASSERT (    (is_ia_nobang ^ is_ia_bang ^ is_db_bang)
		  && !(is_ia_nobang & is_ia_bang & is_db_bang));

      /* We treat the cutting of the words in one pass for all
	 cases, then we emit the adjustments:

	 vldm rx, {...}
	 -> vldm rx!, {8_words_or_less} for each needed 8_word
	 -> sub rx, rx, #size (list)

	 vldm rx!, {...}
	 -> vldm rx!, {8_words_or_less} for each needed 8_word
	 This also handles vpop instruction (when rx is sp)

	 vldmd rx!, {...}
	 -> vldmb rx!, {8_words_or_less} for each needed 8_word.  */
      for (chunk = 0; chunk < chunks; ++chunk)
	{
	  bfd_vma new_insn = 0;

	  if (is_ia_nobang || is_ia_bang)
	    {
	      new_insn = create_instruction_vldmia
		(base_reg,
		 is_dp,
		 /*wback= .  */1,
		 chunks - (chunk + 1) ?
		 8 : num_words - chunk * 8,
		 first_reg + chunk * 8);
	    }
	  else if (is_db_bang)
	    {
	      new_insn = create_instruction_vldmdb
		(base_reg,
		 is_dp,
		 chunks - (chunk + 1) ?
		 8 : num_words - chunk * 8,
		 first_reg + chunk * 8);
	    }

	  if (new_insn)
	    current_stub_contents =
	      push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
				  new_insn);
	}

      /* Only this case requires the base register compensation
	 subtract.  */
      if (is_ia_nobang)
	{
	  current_stub_contents =
	    push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
				create_instruction_sub
				(base_reg, base_reg, 4*num_words));
	}

      /* B initial_insn_addr+4.  */
      current_stub_contents =
	push_thumb2_insn32 (htab, output_bfd, current_stub_contents,
			    create_instruction_branch_absolute
			    (initial_insn_addr - current_stub_contents));
    }

  /* Fill the remaining of the stub with deterministic contents.  */
  current_stub_contents =
    stm32l4xx_fill_stub_udf (htab, output_bfd,
			     base_stub_contents, current_stub_contents,
			     base_stub_contents +
			     STM32L4XX_ERRATUM_VLDM_VENEER_SIZE);
}

static void
stm32l4xx_create_replacing_stub (struct elf32_arm_link_hash_table * htab,
				 bfd * output_bfd,
				 const insn32 wrong_insn,
				 const bfd_byte *const wrong_insn_addr,
				 bfd_byte *const stub_contents)
{
  if (is_thumb2_ldmia (wrong_insn))
    stm32l4xx_create_replacing_stub_ldmia (htab, output_bfd,
					   wrong_insn, wrong_insn_addr,
					   stub_contents);
  else if (is_thumb2_ldmdb (wrong_insn))
    stm32l4xx_create_replacing_stub_ldmdb (htab, output_bfd,
					   wrong_insn, wrong_insn_addr,
					   stub_contents);
  else if (is_thumb2_vldm (wrong_insn))
    stm32l4xx_create_replacing_stub_vldm (htab, output_bfd,
					  wrong_insn, wrong_insn_addr,
					  stub_contents);
}

/* End of stm32l4xx work-around.  */


/* Do code byteswapping.  Return FALSE afterwards so that the section is
   written out as normal.  */

static bool
elf32_arm_write_section (bfd *output_bfd,
			 struct bfd_link_info *link_info,
			 asection *sec,
			 bfd_byte *contents)
{
  unsigned int mapcount, errcount;
  _arm_elf_section_data *arm_data;
  struct elf32_arm_link_hash_table *globals = elf32_arm_hash_table (link_info);
  elf32_arm_section_map *map;
  elf32_vfp11_erratum_list *errnode;
  elf32_stm32l4xx_erratum_list *stm32l4xx_errnode;
  bfd_vma ptr;
  bfd_vma end;
  bfd_vma offset = sec->output_section->vma + sec->output_offset;
  bfd_byte tmp;
  unsigned int i;

  if (globals == NULL)
    return false;

  /* If this section has not been allocated an _arm_elf_section_data
     structure then we cannot record anything.  */
  arm_data = get_arm_elf_section_data (sec);
  if (arm_data == NULL)
    return false;

  mapcount = arm_data->mapcount;
  map = arm_data->map;
  errcount = arm_data->erratumcount;

  if (errcount != 0)
    {
      unsigned int endianflip = bfd_big_endian (output_bfd) ? 3 : 0;

      for (errnode = arm_data->erratumlist; errnode != 0;
	   errnode = errnode->next)
	{
	  bfd_vma target = errnode->vma - offset;

	  switch (errnode->type)
	    {
	    case VFP11_ERRATUM_BRANCH_TO_ARM_VENEER:
	      {
		bfd_vma branch_to_veneer;
		/* Original condition code of instruction, plus bit mask for
		   ARM B instruction.  */
		unsigned int insn = (errnode->u.b.vfp_insn & 0xf0000000)
				  | 0x0a000000;

		/* The instruction is before the label.  */
		target -= 4;

		/* Above offset included in -4 below.  */
		branch_to_veneer = errnode->u.b.veneer->vma
				   - errnode->vma - 4;

		if ((signed) branch_to_veneer < -(1 << 25)
		    || (signed) branch_to_veneer >= (1 << 25))
		  _bfd_error_handler (_("%pB: error: VFP11 veneer out of "
					"range"), output_bfd);

		insn |= (branch_to_veneer >> 2) & 0xffffff;
		contents[endianflip ^ target] = insn & 0xff;
		contents[endianflip ^ (target + 1)] = (insn >> 8) & 0xff;
		contents[endianflip ^ (target + 2)] = (insn >> 16) & 0xff;
		contents[endianflip ^ (target + 3)] = (insn >> 24) & 0xff;
	      }
	      break;

	    case VFP11_ERRATUM_ARM_VENEER:
	      {
		bfd_vma branch_from_veneer;
		unsigned int insn;

		/* Take size of veneer into account.  */
		branch_from_veneer = errnode->u.v.branch->vma
				     - errnode->vma - 12;

		if ((signed) branch_from_veneer < -(1 << 25)
		    || (signed) branch_from_veneer >= (1 << 25))
		  _bfd_error_handler (_("%pB: error: VFP11 veneer out of "
					"range"), output_bfd);

		/* Original instruction.  */
		insn = errnode->u.v.branch->u.b.vfp_insn;
		contents[endianflip ^ target] = insn & 0xff;
		contents[endianflip ^ (target + 1)] = (insn >> 8) & 0xff;
		contents[endianflip ^ (target + 2)] = (insn >> 16) & 0xff;
		contents[endianflip ^ (target + 3)] = (insn >> 24) & 0xff;

		/* Branch back to insn after original insn.  */
		insn = 0xea000000 | ((branch_from_veneer >> 2) & 0xffffff);
		contents[endianflip ^ (target + 4)] = insn & 0xff;
		contents[endianflip ^ (target + 5)] = (insn >> 8) & 0xff;
		contents[endianflip ^ (target + 6)] = (insn >> 16) & 0xff;
		contents[endianflip ^ (target + 7)] = (insn >> 24) & 0xff;
	      }
	      break;

	    default:
	      abort ();
	    }
	}
    }

  if (arm_data->stm32l4xx_erratumcount != 0)
    {
      for (stm32l4xx_errnode = arm_data->stm32l4xx_erratumlist;
	   stm32l4xx_errnode != 0;
	   stm32l4xx_errnode = stm32l4xx_errnode->next)
	{
	  bfd_vma target = stm32l4xx_errnode->vma - offset;

	  switch (stm32l4xx_errnode->type)
	    {
	    case STM32L4XX_ERRATUM_BRANCH_TO_VENEER:
	      {
		unsigned int insn;
		bfd_vma branch_to_veneer =
		  stm32l4xx_errnode->u.b.veneer->vma - stm32l4xx_errnode->vma;

		if ((signed) branch_to_veneer < -(1 << 24)
		    || (signed) branch_to_veneer >= (1 << 24))
		  {
		    bfd_vma out_of_range =
		      ((signed) branch_to_veneer < -(1 << 24)) ?
		      - branch_to_veneer - (1 << 24) :
		      ((signed) branch_to_veneer >= (1 << 24)) ?
		      branch_to_veneer - (1 << 24) : 0;

		    _bfd_error_handler
		      (_("%pB(%#" PRIx64 "): error: "
			 "cannot create STM32L4XX veneer; "
			 "jump out of range by %" PRId64 " bytes; "
			 "cannot encode branch instruction"),
		       output_bfd,
		       (uint64_t) (stm32l4xx_errnode->vma - 4),
		       (int64_t) out_of_range);
		    continue;
		  }

		insn = create_instruction_branch_absolute
		  (stm32l4xx_errnode->u.b.veneer->vma - stm32l4xx_errnode->vma);

		/* The instruction is before the label.  */
		target -= 4;

		put_thumb2_insn (globals, output_bfd,
				 (bfd_vma) insn, contents + target);
	      }
	      break;

	    case STM32L4XX_ERRATUM_VENEER:
	      {
		bfd_byte * veneer;
		bfd_byte * veneer_r;
		unsigned int insn;

		veneer = contents + target;
		veneer_r = veneer
		  + stm32l4xx_errnode->u.b.veneer->vma
		  - stm32l4xx_errnode->vma - 4;

		if ((signed) (veneer_r - veneer -
			      STM32L4XX_ERRATUM_VLDM_VENEER_SIZE >
			      STM32L4XX_ERRATUM_LDM_VENEER_SIZE ?
			      STM32L4XX_ERRATUM_VLDM_VENEER_SIZE :
			      STM32L4XX_ERRATUM_LDM_VENEER_SIZE) < -(1 << 24)
		    || (signed) (veneer_r - veneer) >= (1 << 24))
		  {
		    _bfd_error_handler (_("%pB: error: cannot create STM32L4XX "
					  "veneer"), output_bfd);
		     continue;
		  }

		/* Original instruction.  */
		insn = stm32l4xx_errnode->u.v.branch->u.b.insn;

		stm32l4xx_create_replacing_stub
		  (globals, output_bfd, insn, (void*)veneer_r, (void*)veneer);
	      }
	      break;

	    default:
	      abort ();
	    }
	}
    }

  if (arm_data->elf.this_hdr.sh_type == SHT_ARM_EXIDX)
    {
      arm_unwind_table_edit *edit_node
	= arm_data->u.exidx.unwind_edit_list;
      /* Now, sec->size is the size of the section we will write.  The original
	 size (before we merged duplicate entries and inserted EXIDX_CANTUNWIND
	 markers) was sec->rawsize.  (This isn't the case if we perform no
	 edits, then rawsize will be zero and we should use size).  */
      bfd_byte *edited_contents = (bfd_byte *) bfd_malloc (sec->size);
      unsigned int input_size = sec->rawsize ? sec->rawsize : sec->size;
      unsigned int in_index, out_index;
      bfd_vma add_to_offsets = 0;

      if (edited_contents == NULL)
	return false;
      for (in_index = 0, out_index = 0; in_index * 8 < input_size || edit_node;)
	{
	  if (edit_node)
	    {
	      unsigned int edit_index = edit_node->index;

	      if (in_index < edit_index && in_index * 8 < input_size)
		{
		  copy_exidx_entry (output_bfd, edited_contents + out_index * 8,
				    contents + in_index * 8, add_to_offsets);
		  out_index++;
		  in_index++;
		}
	      else if (in_index == edit_index
		       || (in_index * 8 >= input_size
			   && edit_index == UINT_MAX))
		{
		  switch (edit_node->type)
		    {
		    case DELETE_EXIDX_ENTRY:
		      in_index++;
		      add_to_offsets += 8;
		      break;

		    case INSERT_EXIDX_CANTUNWIND_AT_END:
		      {
			asection *text_sec = edit_node->linked_section;
			bfd_vma text_offset = text_sec->output_section->vma
					      + text_sec->output_offset
					      + text_sec->size;
			bfd_vma exidx_offset = offset + out_index * 8;
			unsigned long prel31_offset;

			/* Note: this is meant to be equivalent to an
			   R_ARM_PREL31 relocation.  These synthetic
			   EXIDX_CANTUNWIND markers are not relocated by the
			   usual BFD method.  */
			prel31_offset = (text_offset - exidx_offset)
					& 0x7ffffffful;
			if (bfd_link_relocatable (link_info))
			  {
			    /* Here relocation for new EXIDX_CANTUNWIND is
			       created, so there is no need to
			       adjust offset by hand.  */
			    prel31_offset = text_sec->output_offset
					    + text_sec->size;
			  }

			/* First address we can't unwind.  */
			bfd_put_32 (output_bfd, prel31_offset,
				    &edited_contents[out_index * 8]);

			/* Code for EXIDX_CANTUNWIND.  */
			bfd_put_32 (output_bfd, 0x1,
				    &edited_contents[out_index * 8 + 4]);

			out_index++;
			add_to_offsets -= 8;
		      }
		      break;
		    }

		  edit_node = edit_node->next;
		}
	    }
	  else
	    {
	      /* No more edits, copy remaining entries verbatim.  */
	      copy_exidx_entry (output_bfd, edited_contents + out_index * 8,
				contents + in_index * 8, add_to_offsets);
	      out_index++;
	      in_index++;
	    }
	}

      if (!(sec->flags & SEC_EXCLUDE) && !(sec->flags & SEC_NEVER_LOAD))
	bfd_set_section_contents (output_bfd, sec->output_section,
				  edited_contents,
				  (file_ptr) sec->output_offset, sec->size);

      return true;
    }

  /* Fix code to point to Cortex-A8 erratum stubs.  */
  if (globals->fix_cortex_a8)
    {
      struct a8_branch_to_stub_data data;

      data.writing_section = sec;
      data.contents = contents;

      bfd_hash_traverse (& globals->stub_hash_table, make_branch_to_a8_stub,
			 & data);
    }

  if (mapcount == 0)
    return false;

  if (globals->byteswap_code)
    {
      qsort (map, mapcount, sizeof (* map), elf32_arm_compare_mapping);

      ptr = map[0].vma;
      for (i = 0; i < mapcount; i++)
	{
	  if (i == mapcount - 1)
	    end = sec->size;
	  else
	    end = map[i + 1].vma;

	  switch (map[i].type)
	    {
	    case 'a':
	      /* Byte swap code words.  */
	      while (ptr + 3 < end)
		{
		  tmp = contents[ptr];
		  contents[ptr] = contents[ptr + 3];
		  contents[ptr + 3] = tmp;
		  tmp = contents[ptr + 1];
		  contents[ptr + 1] = contents[ptr + 2];
		  contents[ptr + 2] = tmp;
		  ptr += 4;
		}
	      break;

	    case 't':
	      /* Byte swap code halfwords.  */
	      while (ptr + 1 < end)
		{
		  tmp = contents[ptr];
		  contents[ptr] = contents[ptr + 1];
		  contents[ptr + 1] = tmp;
		  ptr += 2;
		}
	      break;

	    case 'd':
	      /* Leave data alone.  */
	      break;
	    }
	  ptr = end;
	}
    }

  free (map);
  arm_data->mapcount = -1;
  arm_data->mapsize = 0;
  arm_data->map = NULL;

  return false;
}

/* Mangle thumb function symbols as we read them in.  */

static bool
elf32_arm_swap_symbol_in (bfd * abfd,
			  const void *psrc,
			  const void *pshn,
			  Elf_Internal_Sym *dst)
{
  if (!bfd_elf32_swap_symbol_in (abfd, psrc, pshn, dst))
    return false;
  dst->st_target_internal = 0;

  /* New EABI objects mark thumb function symbols by setting the low bit of
     the address.  */
  if (ELF_ST_TYPE (dst->st_info) == STT_FUNC
      || ELF_ST_TYPE (dst->st_info) == STT_GNU_IFUNC)
    {
      if (dst->st_value & 1)
	{
	  dst->st_value &= ~(bfd_vma) 1;
	  ARM_SET_SYM_BRANCH_TYPE (dst->st_target_internal,
				   ST_BRANCH_TO_THUMB);
	}
      else
	ARM_SET_SYM_BRANCH_TYPE (dst->st_target_internal, ST_BRANCH_TO_ARM);
    }
  else if (ELF_ST_TYPE (dst->st_info) == STT_ARM_TFUNC)
    {
      dst->st_info = ELF_ST_INFO (ELF_ST_BIND (dst->st_info), STT_FUNC);
      ARM_SET_SYM_BRANCH_TYPE (dst->st_target_internal, ST_BRANCH_TO_THUMB);
    }
  else if (ELF_ST_TYPE (dst->st_info) == STT_SECTION)
    ARM_SET_SYM_BRANCH_TYPE (dst->st_target_internal, ST_BRANCH_LONG);
  else
    ARM_SET_SYM_BRANCH_TYPE (dst->st_target_internal, ST_BRANCH_UNKNOWN);

  return true;
}


/* Mangle thumb function symbols as we write them out.  */

static void
elf32_arm_swap_symbol_out (bfd *abfd,
			   const Elf_Internal_Sym *src,
			   void *cdst,
			   void *shndx)
{
  Elf_Internal_Sym newsym;

  /* We convert STT_ARM_TFUNC symbols into STT_FUNC with the low bit
     of the address set, as per the new EABI.  We do this unconditionally
     because objcopy does not set the elf header flags until after
     it writes out the symbol table.  */
  if (ARM_GET_SYM_BRANCH_TYPE (src->st_target_internal) == ST_BRANCH_TO_THUMB)
    {
      newsym = *src;
      if (ELF_ST_TYPE (src->st_info) != STT_GNU_IFUNC)
	newsym.st_info = ELF_ST_INFO (ELF_ST_BIND (src->st_info), STT_FUNC);
      if (newsym.st_shndx != SHN_UNDEF)
	{
	  /* Do this only for defined symbols. At link type, the static
	     linker will simulate the work of dynamic linker of resolving
	     symbols and will carry over the thumbness of found symbols to
	     the output symbol table. It's not clear how it happens, but
	     the thumbness of undefined symbols can well be different at
	     runtime, and writing '1' for them will be confusing for users
	     and possibly for dynamic linker itself.
	  */
	  newsym.st_value |= 1;
	}

      src = &newsym;
    }
  bfd_elf32_swap_symbol_out (abfd, src, cdst, shndx);
}

/* Add the PT_ARM_EXIDX program header.  */

static bool
elf32_arm_modify_segment_map (bfd *abfd,
			      struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  struct elf_segment_map *m;
  asection *sec;

  sec = bfd_get_section_by_name (abfd, ".ARM.exidx");
  if (sec != NULL && (sec->flags & SEC_LOAD) != 0)
    {
      /* If there is already a PT_ARM_EXIDX header, then we do not
	 want to add another one.  This situation arises when running
	 "strip"; the input binary already has the header.  */
      m = elf_seg_map (abfd);
      while (m && m->p_type != PT_ARM_EXIDX)
	m = m->next;
      if (!m)
	{
	  m = (struct elf_segment_map *)
	      bfd_zalloc (abfd, sizeof (struct elf_segment_map));
	  if (m == NULL)
	    return false;
	  m->p_type = PT_ARM_EXIDX;
	  m->count = 1;
	  m->sections[0] = sec;

	  m->next = elf_seg_map (abfd);
	  elf_seg_map (abfd) = m;
	}
    }

  return true;
}

/* We may add a PT_ARM_EXIDX program header.  */

static int
elf32_arm_additional_program_headers (bfd *abfd,
				      struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  asection *sec;

  sec = bfd_get_section_by_name (abfd, ".ARM.exidx");
  if (sec != NULL && (sec->flags & SEC_LOAD) != 0)
    return 1;
  else
    return 0;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  */

static bool
elf32_arm_add_symbol_hook (bfd *abfd, struct bfd_link_info *info,
			   Elf_Internal_Sym *sym, const char **namep,
			   flagword *flagsp, asection **secp, bfd_vma *valp)
{
  if (elf32_arm_hash_table (info) == NULL)
    return false;

  if (elf32_arm_hash_table (info)->root.target_os == is_vxworks
      && !elf_vxworks_add_symbol_hook (abfd, info, sym, namep,
				       flagsp, secp, valp))
    return false;

  return true;
}

/* We use this to override swap_symbol_in and swap_symbol_out.  */
const struct elf_size_info elf32_arm_size_info =
{
  sizeof (Elf32_External_Ehdr),
  sizeof (Elf32_External_Phdr),
  sizeof (Elf32_External_Shdr),
  sizeof (Elf32_External_Rel),
  sizeof (Elf32_External_Rela),
  sizeof (Elf32_External_Sym),
  sizeof (Elf32_External_Dyn),
  sizeof (Elf_External_Note),
  4,
  1,
  32, 2,
  ELFCLASS32, EV_CURRENT,
  bfd_elf32_write_out_phdrs,
  bfd_elf32_write_shdrs_and_ehdr,
  bfd_elf32_checksum_contents,
  bfd_elf32_write_relocs,
  elf32_arm_swap_symbol_in,
  elf32_arm_swap_symbol_out,
  bfd_elf32_slurp_reloc_table,
  bfd_elf32_slurp_symbol_table,
  bfd_elf32_swap_dyn_in,
  bfd_elf32_swap_dyn_out,
  bfd_elf32_swap_reloc_in,
  bfd_elf32_swap_reloc_out,
  bfd_elf32_swap_reloca_in,
  bfd_elf32_swap_reloca_out
};

static bfd_vma
read_code32 (const bfd *abfd, const bfd_byte *addr)
{
  /* V7 BE8 code is always little endian.  */
  if ((elf_elfheader (abfd)->e_flags & EF_ARM_BE8) != 0)
    return bfd_getl32 (addr);

  return bfd_get_32 (abfd, addr);
}

static bfd_vma
read_code16 (const bfd *abfd, const bfd_byte *addr)
{
  /* V7 BE8 code is always little endian.  */
  if ((elf_elfheader (abfd)->e_flags & EF_ARM_BE8) != 0)
    return bfd_getl16 (addr);

  return bfd_get_16 (abfd, addr);
}

/* Return size of plt0 entry starting at ADDR
   or (bfd_vma) -1 if size can not be determined.  */

static bfd_vma
elf32_arm_plt0_size (const bfd *abfd, const bfd_byte *addr)
{
  bfd_vma first_word;
  bfd_vma plt0_size;

  first_word = read_code32 (abfd, addr);

  if (first_word == elf32_arm_plt0_entry[0])
    plt0_size = 4 * ARRAY_SIZE (elf32_arm_plt0_entry);
  else if (first_word == elf32_thumb2_plt0_entry[0])
    plt0_size = 4 * ARRAY_SIZE (elf32_thumb2_plt0_entry);
  else
    /* We don't yet handle this PLT format.  */
    return (bfd_vma) -1;

  return plt0_size;
}

/* Return size of plt entry starting at offset OFFSET
   of plt section located at address START
   or (bfd_vma) -1 if size can not be determined.  */

static bfd_vma
elf32_arm_plt_size (const bfd *abfd, const bfd_byte *start, bfd_vma offset)
{
  bfd_vma first_insn;
  bfd_vma plt_size = 0;
  const bfd_byte *addr = start + offset;

  /* PLT entry size if fixed on Thumb-only platforms.  */
  if (read_code32 (abfd, start) == elf32_thumb2_plt0_entry[0])
      return 4 * ARRAY_SIZE (elf32_thumb2_plt_entry);

  /* Respect Thumb stub if necessary.  */
  if (read_code16 (abfd, addr) == elf32_arm_plt_thumb_stub[0])
    {
      plt_size += 2 * ARRAY_SIZE (elf32_arm_plt_thumb_stub);
    }

  /* Strip immediate from first add.  */
  first_insn = read_code32 (abfd, addr + plt_size) & 0xffffff00;

#ifdef FOUR_WORD_PLT
  if (first_insn == elf32_arm_plt_entry[0])
    plt_size += 4 * ARRAY_SIZE (elf32_arm_plt_entry);
#else
  if (first_insn == elf32_arm_plt_entry_long[0])
    plt_size += 4 * ARRAY_SIZE (elf32_arm_plt_entry_long);
  else if (first_insn == elf32_arm_plt_entry_short[0])
    plt_size += 4 * ARRAY_SIZE (elf32_arm_plt_entry_short);
#endif
  else
    /* We don't yet handle this PLT format.  */
    return (bfd_vma) -1;

  return plt_size;
}

/* Implementation is shamelessly borrowed from _bfd_elf_get_synthetic_symtab.  */

static long
elf32_arm_get_synthetic_symtab (bfd *abfd,
			       long symcount ATTRIBUTE_UNUSED,
			       asymbol **syms ATTRIBUTE_UNUSED,
			       long dynsymcount,
			       asymbol **dynsyms,
			       asymbol **ret)
{
  asection *relplt;
  asymbol *s;
  arelent *p;
  long count, i, n;
  size_t size;
  Elf_Internal_Shdr *hdr;
  char *names;
  asection *plt;
  bfd_vma offset;
  bfd_byte *data;

  *ret = NULL;

  if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0)
    return 0;

  if (dynsymcount <= 0)
    return 0;

  relplt = bfd_get_section_by_name (abfd, ".rel.plt");
  if (relplt == NULL)
    return 0;

  hdr = &elf_section_data (relplt)->this_hdr;
  if (hdr->sh_link != elf_dynsymtab (abfd)
      || (hdr->sh_type != SHT_REL && hdr->sh_type != SHT_RELA))
    return 0;

  plt = bfd_get_section_by_name (abfd, ".plt");
  if (plt == NULL)
    return 0;

  if (!elf32_arm_size_info.slurp_reloc_table (abfd, relplt, dynsyms, true))
    return -1;

  data = NULL;
  if (!bfd_get_full_section_contents (abfd, plt, &data))
    return -1;

  count = NUM_SHDR_ENTRIES (hdr);
  size = count * sizeof (asymbol);
  p = relplt->relocation;
  for (i = 0; i < count; i++, p += elf32_arm_size_info.int_rels_per_ext_rel)
    {
      size += strlen ((*p->sym_ptr_ptr)->name) + sizeof ("@plt");
      if (p->addend != 0)
	size += sizeof ("+0x") - 1 + 8;
    }

  offset = elf32_arm_plt0_size (abfd, data);
  if (offset == (bfd_vma) -1
      || (s = *ret = (asymbol *) bfd_malloc (size)) == NULL)
    {
      free (data);
      return -1;
    }

  names = (char *) (s + count);
  p = relplt->relocation;
  n = 0;
  for (i = 0; i < count; i++, p += elf32_arm_size_info.int_rels_per_ext_rel)
    {
      size_t len;

      bfd_vma plt_size = elf32_arm_plt_size (abfd, data, offset);
      if (plt_size == (bfd_vma) -1)
	break;

      *s = **p->sym_ptr_ptr;
      /* Undefined syms won't have BSF_LOCAL or BSF_GLOBAL set.  Since
	 we are defining a symbol, ensure one of them is set.  */
      if ((s->flags & BSF_LOCAL) == 0)
	s->flags |= BSF_GLOBAL;
      s->flags |= BSF_SYNTHETIC;
      s->section = plt;
      s->value = offset;
      s->name = names;
      s->udata.p = NULL;
      len = strlen ((*p->sym_ptr_ptr)->name);
      memcpy (names, (*p->sym_ptr_ptr)->name, len);
      names += len;
      if (p->addend != 0)
	{
	  char buf[30], *a;

	  memcpy (names, "+0x", sizeof ("+0x") - 1);
	  names += sizeof ("+0x") - 1;
	  bfd_sprintf_vma (abfd, buf, p->addend);
	  for (a = buf; *a == '0'; ++a)
	    ;
	  len = strlen (a);
	  memcpy (names, a, len);
	  names += len;
	}
      memcpy (names, "@plt", sizeof ("@plt"));
      names += sizeof ("@plt");
      ++s, ++n;
      offset += plt_size;
    }

  free (data);
  return n;
}

static bool
elf32_arm_section_flags (const Elf_Internal_Shdr *hdr)
{
  if (hdr->sh_flags & SHF_ARM_PURECODE)
    hdr->bfd_section->flags |= SEC_ELF_PURECODE;
  return true;
}

static flagword
elf32_arm_lookup_section_flags (char *flag_name)
{
  if (!strcmp (flag_name, "SHF_ARM_PURECODE"))
    return SHF_ARM_PURECODE;

  return SEC_NO_FLAGS;
}

static unsigned int
elf32_arm_count_additional_relocs (asection *sec)
{
  struct _arm_elf_section_data *arm_data;
  arm_data = get_arm_elf_section_data (sec);

  return arm_data == NULL ? 0 : arm_data->additional_reloc_count;
}

/* Called to set the sh_flags, sh_link and sh_info fields of OSECTION which
   has a type >= SHT_LOOS.  Returns TRUE if these fields were initialised
   FALSE otherwise.  ISECTION is the best guess matching section from the
   input bfd IBFD, but it might be NULL.  */

static bool
elf32_arm_copy_special_section_fields (const bfd *ibfd ATTRIBUTE_UNUSED,
				       bfd *obfd ATTRIBUTE_UNUSED,
				       const Elf_Internal_Shdr *isection ATTRIBUTE_UNUSED,
				       Elf_Internal_Shdr *osection)
{
  switch (osection->sh_type)
    {
    case SHT_ARM_EXIDX:
      {
	Elf_Internal_Shdr **oheaders = elf_elfsections (obfd);
	Elf_Internal_Shdr **iheaders = elf_elfsections (ibfd);
	unsigned i = 0;

	osection->sh_flags = SHF_ALLOC | SHF_LINK_ORDER;
	osection->sh_info = 0;

	/* The sh_link field must be set to the text section associated with
	   this index section.  Unfortunately the ARM EHABI does not specify
	   exactly how to determine this association.  Our caller does try
	   to match up OSECTION with its corresponding input section however
	   so that is a good first guess.  */
	if (isection != NULL
	    && osection->bfd_section != NULL
	    && isection->bfd_section != NULL
	    && isection->bfd_section->output_section != NULL
	    && isection->bfd_section->output_section == osection->bfd_section
	    && iheaders != NULL
	    && isection->sh_link > 0
	    && isection->sh_link < elf_numsections (ibfd)
	    && iheaders[isection->sh_link]->bfd_section != NULL
	    && iheaders[isection->sh_link]->bfd_section->output_section != NULL
	    )
	  {
	    for (i = elf_numsections (obfd); i-- > 0;)
	      if (oheaders[i]->bfd_section
		  == iheaders[isection->sh_link]->bfd_section->output_section)
		break;
	  }

	if (i == 0)
	  {
	    /* Failing that we have to find a matching section ourselves.  If
	       we had the output section name available we could compare that
	       with input section names.  Unfortunately we don't.  So instead
	       we use a simple heuristic and look for the nearest executable
	       section before this one.  */
	    for (i = elf_numsections (obfd); i-- > 0;)
	      if (oheaders[i] == osection)
		break;
	    if (i == 0)
	      break;

	    while (i-- > 0)
	      if (oheaders[i]->sh_type == SHT_PROGBITS
		  && (oheaders[i]->sh_flags & (SHF_ALLOC | SHF_EXECINSTR))
		  == (SHF_ALLOC | SHF_EXECINSTR))
		break;
	  }

	if (i)
	  {
	    osection->sh_link = i;
	    /* If the text section was part of a group
	       then the index section should be too.  */
	    if (oheaders[i]->sh_flags & SHF_GROUP)
	      osection->sh_flags |= SHF_GROUP;
	    return true;
	  }
      }
      break;

    case SHT_ARM_PREEMPTMAP:
      osection->sh_flags = SHF_ALLOC;
      break;

    case SHT_ARM_ATTRIBUTES:
    case SHT_ARM_DEBUGOVERLAY:
    case SHT_ARM_OVERLAYSECTION:
    default:
      break;
    }

  return false;
}

/* Returns TRUE if NAME is an ARM mapping symbol.
   Traditionally the symbols $a, $d and $t have been used.
   The ARM ELF standard also defines $x (for A64 code).  It also allows a
   period initiated suffix to be added to the symbol: "$[adtx]\.[:sym_char]+".
   Other tools might also produce $b (Thumb BL), $f, $p, $m and $v, but we do
   not support them here.  $t.x indicates the start of ThumbEE instructions.  */

static bool
is_arm_mapping_symbol (const char * name)
{
  return name != NULL /* Paranoia.  */
    && name[0] == '$' /* Note: if objcopy --prefix-symbols has been used then
			 the mapping symbols could have acquired a prefix.
			 We do not support this here, since such symbols no
			 longer conform to the ARM ELF ABI.  */
    && (name[1] == 'a' || name[1] == 'd' || name[1] == 't' || name[1] == 'x')
    && (name[2] == 0 || name[2] == '.');
  /* FIXME: Strictly speaking the symbol is only a valid mapping symbol if
     any characters that follow the period are legal characters for the body
     of a symbol's name.  For now we just assume that this is the case.  */
}

/* Make sure that mapping symbols in object files are not removed via the
   "strip --strip-unneeded" tool.  These symbols are needed in order to
   correctly generate interworking veneers, and for byte swapping code
   regions.  Once an object file has been linked, it is safe to remove the
   symbols as they will no longer be needed.  */

static void
elf32_arm_backend_symbol_processing (bfd *abfd, asymbol *sym)
{
  if (((abfd->flags & (EXEC_P | DYNAMIC)) == 0)
      && sym->section != bfd_abs_section_ptr
      && is_arm_mapping_symbol (sym->name))
    sym->flags |= BSF_KEEP;
}

#undef  elf_backend_copy_special_section_fields
#define elf_backend_copy_special_section_fields elf32_arm_copy_special_section_fields

#define ELF_ARCH			bfd_arch_arm
#define ELF_TARGET_ID			ARM_ELF_DATA
#define ELF_MACHINE_CODE		EM_ARM
#define ELF_MAXPAGESIZE			0x1000
#define ELF_COMMONPAGESIZE		0x1000

#define bfd_elf32_mkobject			elf32_arm_mkobject

#define bfd_elf32_bfd_copy_private_bfd_data	elf32_arm_copy_private_bfd_data
#define bfd_elf32_bfd_merge_private_bfd_data	elf32_arm_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags		elf32_arm_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data	elf32_arm_print_private_bfd_data
#define bfd_elf32_bfd_link_hash_table_create	elf32_arm_link_hash_table_create
#define bfd_elf32_bfd_reloc_type_lookup		elf32_arm_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		elf32_arm_reloc_name_lookup
#define bfd_elf32_find_inliner_info		elf32_arm_find_inliner_info
#define bfd_elf32_new_section_hook		elf32_arm_new_section_hook
#define bfd_elf32_bfd_is_target_special_symbol	elf32_arm_is_target_special_symbol
#define bfd_elf32_bfd_final_link		elf32_arm_final_link
#define bfd_elf32_get_synthetic_symtab	elf32_arm_get_synthetic_symtab

#define elf_backend_get_symbol_type		elf32_arm_get_symbol_type
#define elf_backend_maybe_function_sym		elf32_arm_maybe_function_sym
#define elf_backend_gc_mark_hook		elf32_arm_gc_mark_hook
#define elf_backend_gc_mark_extra_sections	elf32_arm_gc_mark_extra_sections
#define elf_backend_check_relocs		elf32_arm_check_relocs
#define elf_backend_update_relocs		elf32_arm_update_relocs
#define elf_backend_relocate_section		elf32_arm_relocate_section
#define elf_backend_write_section		elf32_arm_write_section
#define elf_backend_adjust_dynamic_symbol	elf32_arm_adjust_dynamic_symbol
#define elf_backend_create_dynamic_sections	elf32_arm_create_dynamic_sections
#define elf_backend_finish_dynamic_symbol	elf32_arm_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections	elf32_arm_finish_dynamic_sections
#define elf_backend_size_dynamic_sections	elf32_arm_size_dynamic_sections
#define elf_backend_always_size_sections	elf32_arm_always_size_sections
#define elf_backend_init_index_section		_bfd_elf_init_2_index_sections
#define elf_backend_init_file_header		elf32_arm_init_file_header
#define elf_backend_reloc_type_class		elf32_arm_reloc_type_class
#define elf_backend_object_p			elf32_arm_object_p
#define elf_backend_fake_sections		elf32_arm_fake_sections
#define elf_backend_section_from_shdr		elf32_arm_section_from_shdr
#define elf_backend_final_write_processing	elf32_arm_final_write_processing
#define elf_backend_copy_indirect_symbol	elf32_arm_copy_indirect_symbol
#define elf_backend_size_info			elf32_arm_size_info
#define elf_backend_modify_segment_map		elf32_arm_modify_segment_map
#define elf_backend_additional_program_headers	elf32_arm_additional_program_headers
#define elf_backend_output_arch_local_syms	elf32_arm_output_arch_local_syms
#define elf_backend_filter_implib_symbols	elf32_arm_filter_implib_symbols
#define elf_backend_begin_write_processing	elf32_arm_begin_write_processing
#define elf_backend_add_symbol_hook		elf32_arm_add_symbol_hook
#define elf_backend_count_additional_relocs	elf32_arm_count_additional_relocs
#define elf_backend_symbol_processing		elf32_arm_backend_symbol_processing

#define elf_backend_can_refcount       1
#define elf_backend_can_gc_sections    1
#define elf_backend_plt_readonly       1
#define elf_backend_want_got_plt       1
#define elf_backend_want_plt_sym       0
#define elf_backend_want_dynrelro      1
#define elf_backend_may_use_rel_p      1
#define elf_backend_may_use_rela_p     0
#define elf_backend_default_use_rela_p 0
#define elf_backend_dtrel_excludes_plt 1

#define elf_backend_got_header_size	12
#define elf_backend_extern_protected_data 0

#undef	elf_backend_obj_attrs_vendor
#define elf_backend_obj_attrs_vendor		"aeabi"
#undef	elf_backend_obj_attrs_section
#define elf_backend_obj_attrs_section		".ARM.attributes"
#undef	elf_backend_obj_attrs_arg_type
#define elf_backend_obj_attrs_arg_type		elf32_arm_obj_attrs_arg_type
#undef	elf_backend_obj_attrs_section_type
#define elf_backend_obj_attrs_section_type	SHT_ARM_ATTRIBUTES
#define elf_backend_obj_attrs_order		elf32_arm_obj_attrs_order
#define elf_backend_obj_attrs_handle_unknown	elf32_arm_obj_attrs_handle_unknown

#undef	elf_backend_section_flags
#define elf_backend_section_flags		elf32_arm_section_flags
#undef	elf_backend_lookup_section_flags_hook
#define elf_backend_lookup_section_flags_hook	elf32_arm_lookup_section_flags

#define elf_backend_linux_prpsinfo32_ugid16	true

#include "elf32-target.h"

/* Native Client targets.  */

#undef	TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		arm_elf32_nacl_le_vec
#undef	TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		"elf32-littlearm-nacl"
#undef	TARGET_BIG_SYM
#define TARGET_BIG_SYM			arm_elf32_nacl_be_vec
#undef	TARGET_BIG_NAME
#define TARGET_BIG_NAME			"elf32-bigarm-nacl"

/* Like elf32_arm_link_hash_table_create -- but overrides
   appropriately for NaCl.  */

static struct bfd_link_hash_table *
elf32_arm_nacl_link_hash_table_create (bfd *abfd)
{
  struct bfd_link_hash_table *ret;

  ret = elf32_arm_link_hash_table_create (abfd);
  if (ret)
    {
      struct elf32_arm_link_hash_table *htab
	= (struct elf32_arm_link_hash_table *) ret;

      htab->plt_header_size = 4 * ARRAY_SIZE (elf32_arm_nacl_plt0_entry);
      htab->plt_entry_size = 4 * ARRAY_SIZE (elf32_arm_nacl_plt_entry);
    }
  return ret;
}

/* Since NaCl doesn't use the ARM-specific unwind format, we don't
   really need to use elf32_arm_modify_segment_map.  But we do it
   anyway just to reduce gratuitous differences with the stock ARM backend.  */

static bool
elf32_arm_nacl_modify_segment_map (bfd *abfd, struct bfd_link_info *info)
{
  return (elf32_arm_modify_segment_map (abfd, info)
	  && nacl_modify_segment_map (abfd, info));
}

static bool
elf32_arm_nacl_final_write_processing (bfd *abfd)
{
  arm_final_write_processing (abfd);
  return nacl_final_write_processing (abfd);
}

static bfd_vma
elf32_arm_nacl_plt_sym_val (bfd_vma i, const asection *plt,
			    const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma
    + 4 * (ARRAY_SIZE (elf32_arm_nacl_plt0_entry) +
	   i * ARRAY_SIZE (elf32_arm_nacl_plt_entry));
}

#undef	elf32_bed
#define elf32_bed				elf32_arm_nacl_bed
#undef  bfd_elf32_bfd_link_hash_table_create
#define bfd_elf32_bfd_link_hash_table_create	\
  elf32_arm_nacl_link_hash_table_create
#undef	elf_backend_plt_alignment
#define elf_backend_plt_alignment		4
#undef	elf_backend_modify_segment_map
#define	elf_backend_modify_segment_map		elf32_arm_nacl_modify_segment_map
#undef	elf_backend_modify_headers
#define	elf_backend_modify_headers		nacl_modify_headers
#undef  elf_backend_final_write_processing
#define elf_backend_final_write_processing	elf32_arm_nacl_final_write_processing
#undef bfd_elf32_get_synthetic_symtab
#undef  elf_backend_plt_sym_val
#define elf_backend_plt_sym_val			elf32_arm_nacl_plt_sym_val
#undef  elf_backend_copy_special_section_fields

#undef	ELF_MINPAGESIZE
#undef	ELF_COMMONPAGESIZE

#undef ELF_TARGET_OS
#define ELF_TARGET_OS				is_nacl

#include "elf32-target.h"

/* Reset to defaults.  */
#undef	elf_backend_plt_alignment
#undef	elf_backend_modify_segment_map
#define elf_backend_modify_segment_map		elf32_arm_modify_segment_map
#undef	elf_backend_modify_headers
#undef  elf_backend_final_write_processing
#define elf_backend_final_write_processing	elf32_arm_final_write_processing
#undef	ELF_MINPAGESIZE
#undef	ELF_COMMONPAGESIZE
#define ELF_COMMONPAGESIZE		0x1000


/* FDPIC Targets.  */

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		arm_elf32_fdpic_le_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		"elf32-littlearm-fdpic"
#undef  TARGET_BIG_SYM
#define TARGET_BIG_SYM			arm_elf32_fdpic_be_vec
#undef  TARGET_BIG_NAME
#define TARGET_BIG_NAME			"elf32-bigarm-fdpic"
#undef elf_match_priority
#define elf_match_priority		128
#undef ELF_OSABI
#define ELF_OSABI		ELFOSABI_ARM_FDPIC

/* Like elf32_arm_link_hash_table_create -- but overrides
   appropriately for FDPIC.  */

static struct bfd_link_hash_table *
elf32_arm_fdpic_link_hash_table_create (bfd *abfd)
{
  struct bfd_link_hash_table *ret;

  ret = elf32_arm_link_hash_table_create (abfd);
  if (ret)
    {
      struct elf32_arm_link_hash_table *htab = (struct elf32_arm_link_hash_table *) ret;

      htab->fdpic_p = 1;
    }
  return ret;
}

/* We need dynamic symbols for every section, since segments can
   relocate independently.  */
static bool
elf32_arm_fdpic_omit_section_dynsym (bfd *output_bfd ATTRIBUTE_UNUSED,
				    struct bfd_link_info *info
				    ATTRIBUTE_UNUSED,
				    asection *p ATTRIBUTE_UNUSED)
{
  switch (elf_section_data (p)->this_hdr.sh_type)
    {
    case SHT_PROGBITS:
    case SHT_NOBITS:
      /* If sh_type is yet undecided, assume it could be
	 SHT_PROGBITS/SHT_NOBITS.  */
    case SHT_NULL:
      return false;

      /* There shouldn't be section relative relocations
	 against any other section.  */
    default:
      return true;
    }
}

#undef  elf32_bed
#define elf32_bed				elf32_arm_fdpic_bed

#undef  bfd_elf32_bfd_link_hash_table_create
#define bfd_elf32_bfd_link_hash_table_create	elf32_arm_fdpic_link_hash_table_create

#undef elf_backend_omit_section_dynsym
#define elf_backend_omit_section_dynsym		elf32_arm_fdpic_omit_section_dynsym

#undef ELF_TARGET_OS

#include "elf32-target.h"

#undef elf_match_priority
#undef ELF_OSABI
#undef elf_backend_omit_section_dynsym

/* VxWorks Targets.  */

#undef	TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		arm_elf32_vxworks_le_vec
#undef	TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		"elf32-littlearm-vxworks"
#undef	TARGET_BIG_SYM
#define TARGET_BIG_SYM			arm_elf32_vxworks_be_vec
#undef	TARGET_BIG_NAME
#define TARGET_BIG_NAME			"elf32-bigarm-vxworks"

/* Like elf32_arm_link_hash_table_create -- but overrides
   appropriately for VxWorks.  */

static struct bfd_link_hash_table *
elf32_arm_vxworks_link_hash_table_create (bfd *abfd)
{
  struct bfd_link_hash_table *ret;

  ret = elf32_arm_link_hash_table_create (abfd);
  if (ret)
    {
      struct elf32_arm_link_hash_table *htab
	= (struct elf32_arm_link_hash_table *) ret;
      htab->use_rel = 0;
    }
  return ret;
}

static bool
elf32_arm_vxworks_final_write_processing (bfd *abfd)
{
  arm_final_write_processing (abfd);
  return elf_vxworks_final_write_processing (abfd);
}

#undef  elf32_bed
#define elf32_bed elf32_arm_vxworks_bed

#undef  bfd_elf32_bfd_link_hash_table_create
#define bfd_elf32_bfd_link_hash_table_create	elf32_arm_vxworks_link_hash_table_create
#undef  elf_backend_final_write_processing
#define elf_backend_final_write_processing	elf32_arm_vxworks_final_write_processing
#undef  elf_backend_emit_relocs
#define elf_backend_emit_relocs			elf_vxworks_emit_relocs

#undef  elf_backend_may_use_rel_p
#define elf_backend_may_use_rel_p	0
#undef  elf_backend_may_use_rela_p
#define elf_backend_may_use_rela_p	1
#undef  elf_backend_default_use_rela_p
#define elf_backend_default_use_rela_p	1
#undef  elf_backend_want_plt_sym
#define elf_backend_want_plt_sym	1
#undef  ELF_MAXPAGESIZE
#define ELF_MAXPAGESIZE			0x1000
#undef ELF_TARGET_OS
#define ELF_TARGET_OS			is_vxworks

#include "elf32-target.h"


/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
elf32_arm_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword out_flags;
  flagword in_flags;
  bool flags_compatible = true;
  asection *sec;

  /* Check if we have the same endianness.  */
  if (! _bfd_generic_verify_endian_match (ibfd, info))
    return false;

  if (! is_arm_elf (ibfd) || ! is_arm_elf (obfd))
    return true;

  if (!elf32_arm_merge_eabi_attributes (ibfd, info))
    return false;

  /* The input BFD must have had its flags initialised.  */
  /* The following seems bogus to me -- The flags are initialized in
     the assembler but I don't think an elf_flags_init field is
     written into the object.  */
  /* BFD_ASSERT (elf_flags_init (ibfd)); */

  in_flags  = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  /* In theory there is no reason why we couldn't handle this.  However
     in practice it isn't even close to working and there is no real
     reason to want it.  */
  if (EF_ARM_EABI_VERSION (in_flags) >= EF_ARM_EABI_VER4
      && !(ibfd->flags & DYNAMIC)
      && (in_flags & EF_ARM_BE8))
    {
      _bfd_error_handler (_("error: %pB is already in final BE8 format"),
			  ibfd);
      return false;
    }

  if (!elf_flags_init (obfd))
    {
      /* If the input is the default architecture and had the default
	 flags then do not bother setting the flags for the output
	 architecture, instead allow future merges to do this.  If no
	 future merges ever set these flags then they will retain their
	 uninitialised values, which surprise surprise, correspond
	 to the default values.  */
      if (bfd_get_arch_info (ibfd)->the_default
	  && elf_elfheader (ibfd)->e_flags == 0)
	return true;

      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = in_flags;

      if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	  && bfd_get_arch_info (obfd)->the_default)
	return bfd_set_arch_mach (obfd, bfd_get_arch (ibfd), bfd_get_mach (ibfd));

      return true;
    }

  /* Determine what should happen if the input ARM architecture
     does not match the output ARM architecture.  */
  if (! bfd_arm_merge_machines (ibfd, obfd))
    return false;

  /* Identical flags must be compatible.  */
  if (in_flags == out_flags)
    return true;

  /* Check to see if the input BFD actually contains any sections.  If
     not, its flags may not have been initialised either, but it
     cannot actually cause any incompatiblity.  Do not short-circuit
     dynamic objects; their section list may be emptied by
    elf_link_add_object_symbols.

    Also check to see if there are no code sections in the input.
    In this case there is no need to check for code specific flags.
    XXX - do we need to worry about floating-point format compatability
    in data sections ?  */
  if (!(ibfd->flags & DYNAMIC))
    {
      bool null_input_bfd = true;
      bool only_data_sections = true;

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	{
	  /* Ignore synthetic glue sections.  */
	  if (strcmp (sec->name, ".glue_7")
	      && strcmp (sec->name, ".glue_7t"))
	    {
	      if ((bfd_section_flags (sec)
		   & (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
		  == (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
		only_data_sections = false;

	      null_input_bfd = false;
	      break;
	    }
	}

      if (null_input_bfd || only_data_sections)
	return true;
    }

  /* Complain about various flag mismatches.  */
  if (!elf32_arm_versions_compatible (EF_ARM_EABI_VERSION (in_flags),
				      EF_ARM_EABI_VERSION (out_flags)))
    {
      _bfd_error_handler
	(_("error: source object %pB has EABI version %d, but target %pB has EABI version %d"),
	 ibfd, (in_flags & EF_ARM_EABIMASK) >> 24,
	 obfd, (out_flags & EF_ARM_EABIMASK) >> 24);
      return false;
    }

  /* Not sure what needs to be checked for EABI versions >= 1.  */
  /* VxWorks libraries do not use these flags.  */
  if (get_elf_backend_data (obfd) != &elf32_arm_vxworks_bed
      && get_elf_backend_data (ibfd) != &elf32_arm_vxworks_bed
      && EF_ARM_EABI_VERSION (in_flags) == EF_ARM_EABI_UNKNOWN)
    {
      if ((in_flags & EF_ARM_APCS_26) != (out_flags & EF_ARM_APCS_26))
	{
	  _bfd_error_handler
	    (_("error: %pB is compiled for APCS-%d, whereas target %pB uses APCS-%d"),
	     ibfd, in_flags & EF_ARM_APCS_26 ? 26 : 32,
	     obfd, out_flags & EF_ARM_APCS_26 ? 26 : 32);
	  flags_compatible = false;
	}

      if ((in_flags & EF_ARM_APCS_FLOAT) != (out_flags & EF_ARM_APCS_FLOAT))
	{
	  if (in_flags & EF_ARM_APCS_FLOAT)
	    _bfd_error_handler
	      (_("error: %pB passes floats in float registers, whereas %pB passes them in integer registers"),
	       ibfd, obfd);
	  else
	    _bfd_error_handler
	      (_("error: %pB passes floats in integer registers, whereas %pB passes them in float registers"),
	       ibfd, obfd);

	  flags_compatible = false;
	}

      if ((in_flags & EF_ARM_VFP_FLOAT) != (out_flags & EF_ARM_VFP_FLOAT))
	{
	  if (in_flags & EF_ARM_VFP_FLOAT)
	    _bfd_error_handler
	      (_("error: %pB uses %s instructions, whereas %pB does not"),
	       ibfd, "VFP", obfd);
	  else
	    _bfd_error_handler
	      (_("error: %pB uses %s instructions, whereas %pB does not"),
	       ibfd, "FPA", obfd);

	  flags_compatible = false;
	}

      if ((in_flags & EF_ARM_MAVERICK_FLOAT) != (out_flags & EF_ARM_MAVERICK_FLOAT))
	{
	  if (in_flags & EF_ARM_MAVERICK_FLOAT)
	    _bfd_error_handler
	      (_("error: %pB uses %s instructions, whereas %pB does not"),
	       ibfd, "Maverick", obfd);
	  else
	    _bfd_error_handler
	      (_("error: %pB does not use %s instructions, whereas %pB does"),
	       ibfd, "Maverick", obfd);

	  flags_compatible = false;
	}

#ifdef EF_ARM_SOFT_FLOAT
      if ((in_flags & EF_ARM_SOFT_FLOAT) != (out_flags & EF_ARM_SOFT_FLOAT))
	{
	  /* We can allow interworking between code that is VFP format
	     layout, and uses either soft float or integer regs for
	     passing floating point arguments and results.  We already
	     know that the APCS_FLOAT flags match; similarly for VFP
	     flags.  */
	  if ((in_flags & EF_ARM_APCS_FLOAT) != 0
	      || (in_flags & EF_ARM_VFP_FLOAT) == 0)
	    {
	      if (in_flags & EF_ARM_SOFT_FLOAT)
		_bfd_error_handler
		  (_("error: %pB uses software FP, whereas %pB uses hardware FP"),
		   ibfd, obfd);
	      else
		_bfd_error_handler
		  (_("error: %pB uses hardware FP, whereas %pB uses software FP"),
		   ibfd, obfd);

	      flags_compatible = false;
	    }
	}
#endif

      /* Interworking mismatch is only a warning.  */
      if ((in_flags & EF_ARM_INTERWORK) != (out_flags & EF_ARM_INTERWORK))
	{
	  if (in_flags & EF_ARM_INTERWORK)
	    {
	      _bfd_error_handler
		(_("warning: %pB supports interworking, whereas %pB does not"),
		 ibfd, obfd);
	    }
	  else
	    {
	      _bfd_error_handler
		(_("warning: %pB does not support interworking, whereas %pB does"),
		 ibfd, obfd);
	    }
	}
    }

  return flags_compatible;
}
