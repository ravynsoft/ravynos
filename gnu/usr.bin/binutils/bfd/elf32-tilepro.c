/* TILEPro-specific support for 32-bit ELF.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/tilepro.h"
#include "opcode/tilepro.h"
#include "libiberty.h"
#include "elf32-tilepro.h"

#define TILEPRO_BYTES_PER_WORD 4

static reloc_howto_type tilepro_elf_howto_table [] =
{
  /* This reloc does nothing.  */
  HOWTO (R_TILEPRO_NONE,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 32 bit absolute relocation.  */
  HOWTO (R_TILEPRO_32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_TILEPRO_16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* An 8 bit absolute relocation.  */
  HOWTO (R_TILEPRO_8,	/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 32 bit pc-relative relocation.  */
  HOWTO (R_TILEPRO_32_PCREL,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_32_PCREL", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit pc-relative relocation.  */
  HOWTO (R_TILEPRO_16_PCREL,/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_16_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* An 8 bit pc-relative relocation.  */
  HOWTO (R_TILEPRO_8_PCREL,	/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_8_PCREL",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit relocation without overflow.  */
  HOWTO (R_TILEPRO_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_LO16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* The high order 16 bits of an address.  */
  HOWTO (R_TILEPRO_HI16,	/* type */
	 16,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_HI16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* The high order 16 bits of an address, plus 1 if the contents of
     the low 16 bits, treated as a signed number, is negative.  */
  HOWTO (R_TILEPRO_HA16,	/* type */
	 16,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEPRO_HA16",  /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_TILEPRO_COPY,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_COPY",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_GLOB_DAT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_JMP_SLOT,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_JMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_RELATIVE,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_RELATIVE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_BROFF_X1, /* type */
	 TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES, /* rightshift */
	 4,			/* size */
	 17,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEPRO_BROFF_X1", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_JOFFLONG_X1, /* type */
	 TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES, /* rightshift */
	 4,			/* size */
	 29,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEPRO_JOFFLONG_X1", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_JOFFLONG_X1_PLT, /* type */
	 TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES, /* rightshift */
	 4,			/* size */
	 29,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEPRO_JOFFLONG_X1_PLT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

#define TILEPRO_IMM_HOWTO(name, size, bitsize) \
  HOWTO (name, 0, size, bitsize, false, 0, \
	 complain_overflow_signed, bfd_elf_generic_reloc, \
	 #name, false, 0, -1, false)

#define TILEPRO_UIMM_HOWTO(name, size, bitsize) \
  HOWTO (name, 0, size, bitsize, false, 0, \
	 complain_overflow_unsigned, bfd_elf_generic_reloc, \
	 #name, false, 0, -1, false)

  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM8_X0,  1,  8),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM8_Y0,  1,  8),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM8_X1,  1,  8),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM8_Y1,  1,  8),
  TILEPRO_UIMM_HOWTO(R_TILEPRO_MT_IMM15_X1, 2,  15),
  TILEPRO_UIMM_HOWTO(R_TILEPRO_MF_IMM15_X1, 2,  15),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM16_X0, 2, 16),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM16_X1, 2, 16),

#define TILEPRO_IMM16_HOWTO(name, rshift) \
  HOWTO (name, rshift, 2, 16, false, 0, \
	 complain_overflow_dont, bfd_elf_generic_reloc, \
	 #name, false, 0, 0xffff, false)

  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_HA, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_HA, 16),

  /* PC-relative offsets. */

  HOWTO (R_TILEPRO_IMM16_X0_PCREL,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_IMM16_X0_PCREL",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_IMM16_X1_PCREL,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_IMM16_X1_PCREL",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

#define TILEPRO_IMM16_HOWTO_PCREL(name, rshift) \
  HOWTO (name, rshift, 2, 16, true, 0, \
	 complain_overflow_dont, bfd_elf_generic_reloc, \
	 #name, false, 0, 0xffff, true)

  TILEPRO_IMM16_HOWTO_PCREL (R_TILEPRO_IMM16_X0_LO_PCREL,  0),
  TILEPRO_IMM16_HOWTO_PCREL (R_TILEPRO_IMM16_X1_LO_PCREL,  0),
  TILEPRO_IMM16_HOWTO_PCREL (R_TILEPRO_IMM16_X0_HI_PCREL, 16),
  TILEPRO_IMM16_HOWTO_PCREL (R_TILEPRO_IMM16_X1_HI_PCREL, 16),
  TILEPRO_IMM16_HOWTO_PCREL (R_TILEPRO_IMM16_X0_HA_PCREL, 16),
  TILEPRO_IMM16_HOWTO_PCREL (R_TILEPRO_IMM16_X1_HA_PCREL, 16),

  /* Byte offset into GOT for a particular symbol. */
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM16_X0_GOT, 2, 16),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM16_X1_GOT, 2, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_GOT_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_GOT_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_GOT_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_GOT_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_GOT_HA, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_GOT_HA, 16),

  TILEPRO_UIMM_HOWTO(R_TILEPRO_MMSTART_X0, 1, 5),
  TILEPRO_UIMM_HOWTO(R_TILEPRO_MMEND_X0,   1, 5),
  TILEPRO_UIMM_HOWTO(R_TILEPRO_MMSTART_X1, 1, 5),
  TILEPRO_UIMM_HOWTO(R_TILEPRO_MMEND_X1,   1, 5),

  TILEPRO_UIMM_HOWTO(R_TILEPRO_SHAMT_X0, 1, 5),
  TILEPRO_UIMM_HOWTO(R_TILEPRO_SHAMT_X1, 1, 5),
  TILEPRO_UIMM_HOWTO(R_TILEPRO_SHAMT_Y0, 1, 5),
  TILEPRO_UIMM_HOWTO(R_TILEPRO_SHAMT_Y1, 1, 5),

  TILEPRO_IMM_HOWTO(R_TILEPRO_DEST_IMM8_X1, 1, 8),

  /* These relocs are currently not defined.  */
  EMPTY_HOWTO (56),
  EMPTY_HOWTO (57),
  EMPTY_HOWTO (58),
  EMPTY_HOWTO (59),

  HOWTO (R_TILEPRO_TLS_GD_CALL, /* type */
	 TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES, /* rightshift */
	 4,			/* size */
	 29,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEPRO_TLS_GD_CALL", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM8_X0_TLS_GD_ADD,  1,  8),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM8_X1_TLS_GD_ADD,  1,  8),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM8_Y0_TLS_GD_ADD,  1,  8),
  TILEPRO_IMM_HOWTO(R_TILEPRO_IMM8_Y1_TLS_GD_ADD,  1,  8),
  TILEPRO_IMM_HOWTO(R_TILEPRO_TLS_IE_LOAD,  1,  8),

  /* Offsets into the GOT of TLS Descriptors. */

  HOWTO (R_TILEPRO_IMM16_X0_TLS_GD,/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_IMM16_X0_TLS_GD",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_TILEPRO_IMM16_X1_TLS_GD,/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_IMM16_X1_TLS_GD",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_GD_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_GD_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_GD_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_GD_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_GD_HA, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_GD_HA, 16),

  /* Offsets into the GOT of TLS Descriptors. */

  HOWTO (R_TILEPRO_IMM16_X0_TLS_IE,/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_IMM16_X0_TLS_IE",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_IMM16_X1_TLS_IE,/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_IMM16_X1_TLS_IE",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_IE_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_IE_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_IE_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_IE_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_IE_HA, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_IE_HA, 16),

  /* These are common with the Solaris TLS implementation. */
  HOWTO(R_TILEPRO_TLS_DTPMOD32, 0, 0, 0, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_TILEPRO_TLS_DTPMOD32",
	false, 0, 0, true),
  HOWTO(R_TILEPRO_TLS_DTPOFF32, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_TILEPRO_TLS_DTPOFF32",
	false, 0, 0xFFFFFFFF, true),
  HOWTO(R_TILEPRO_TLS_TPOFF32, 0, 0, 0, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_TILEPRO_TLS_TPOFF32",
	false, 0, 0, true),

  HOWTO (R_TILEPRO_IMM16_X0_TLS_LE,/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_IMM16_X0_TLS_LE",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEPRO_IMM16_X1_TLS_LE,/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEPRO_IMM16_X1_TLS_LE",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_LE_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_LE_LO,  0),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_LE_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_LE_HI, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X0_TLS_LE_HA, 16),
  TILEPRO_IMM16_HOWTO (R_TILEPRO_IMM16_X1_TLS_LE_HA, 16),
};

static reloc_howto_type tilepro_elf_howto_table2 [] =
{
  /* GNU extension to record C++ vtable hierarchy */
  HOWTO (R_TILEPRO_GNU_VTINHERIT, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_TILEPRO_GNU_VTINHERIT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable member usage */
  HOWTO (R_TILEPRO_GNU_VTENTRY,	    /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn,	/* special_function */
	 "R_TILEPRO_GNU_VTENTRY",   /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

};

/* Map BFD reloc types to TILEPRO ELF reloc types.  */

typedef struct tilepro_reloc_map
{
  bfd_reloc_code_real_type  bfd_reloc_val;
  unsigned int		    tilepro_reloc_val;
  reloc_howto_type *	    table;
} reloc_map;

static const reloc_map tilepro_reloc_map [] =
{
#define TH_REMAP(bfd, tilepro) \
  { bfd, tilepro, tilepro_elf_howto_table },

  /* Standard relocations. */
  TH_REMAP (BFD_RELOC_NONE,		       R_TILEPRO_NONE)
  TH_REMAP (BFD_RELOC_32,		       R_TILEPRO_32)
  TH_REMAP (BFD_RELOC_16,		       R_TILEPRO_16)
  TH_REMAP (BFD_RELOC_8,		       R_TILEPRO_8)
  TH_REMAP (BFD_RELOC_32_PCREL,		       R_TILEPRO_32_PCREL)
  TH_REMAP (BFD_RELOC_16_PCREL,		       R_TILEPRO_16_PCREL)
  TH_REMAP (BFD_RELOC_8_PCREL,		       R_TILEPRO_8_PCREL)
  TH_REMAP (BFD_RELOC_LO16,		       R_TILEPRO_LO16)
  TH_REMAP (BFD_RELOC_HI16,		       R_TILEPRO_HI16)
  TH_REMAP (BFD_RELOC_HI16_S,		       R_TILEPRO_HA16)

  /* Custom relocations. */
  TH_REMAP (BFD_RELOC_TILEPRO_COPY,	       R_TILEPRO_COPY)
  TH_REMAP (BFD_RELOC_TILEPRO_GLOB_DAT,	       R_TILEPRO_GLOB_DAT)
  TH_REMAP (BFD_RELOC_TILEPRO_JMP_SLOT,	       R_TILEPRO_JMP_SLOT)
  TH_REMAP (BFD_RELOC_TILEPRO_RELATIVE,	       R_TILEPRO_RELATIVE)
  TH_REMAP (BFD_RELOC_TILEPRO_BROFF_X1,	       R_TILEPRO_BROFF_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_JOFFLONG_X1,     R_TILEPRO_JOFFLONG_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_JOFFLONG_X1_PLT, R_TILEPRO_JOFFLONG_X1_PLT)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM8_X0,	       R_TILEPRO_IMM8_X0)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM8_Y0,	       R_TILEPRO_IMM8_Y0)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM8_X1,	       R_TILEPRO_IMM8_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM8_Y1,	       R_TILEPRO_IMM8_Y1)
  TH_REMAP (BFD_RELOC_TILEPRO_DEST_IMM8_X1,    R_TILEPRO_DEST_IMM8_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_MT_IMM15_X1,     R_TILEPRO_MT_IMM15_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_MF_IMM15_X1,     R_TILEPRO_MF_IMM15_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0,	       R_TILEPRO_IMM16_X0)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1,	       R_TILEPRO_IMM16_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_LO,     R_TILEPRO_IMM16_X0_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_LO,     R_TILEPRO_IMM16_X1_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_HI,     R_TILEPRO_IMM16_X0_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_HI,     R_TILEPRO_IMM16_X1_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_HA,     R_TILEPRO_IMM16_X0_HA)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_HA,     R_TILEPRO_IMM16_X1_HA)

  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_PCREL,	 R_TILEPRO_IMM16_X0_PCREL)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_PCREL,	 R_TILEPRO_IMM16_X1_PCREL)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_LO_PCREL, R_TILEPRO_IMM16_X0_LO_PCREL)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_LO_PCREL, R_TILEPRO_IMM16_X1_LO_PCREL)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_HI_PCREL, R_TILEPRO_IMM16_X0_HI_PCREL)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_HI_PCREL, R_TILEPRO_IMM16_X1_HI_PCREL)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_HA_PCREL, R_TILEPRO_IMM16_X0_HA_PCREL)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_HA_PCREL, R_TILEPRO_IMM16_X1_HA_PCREL)

  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_GOT, R_TILEPRO_IMM16_X0_GOT)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_GOT, R_TILEPRO_IMM16_X1_GOT)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_GOT_LO, R_TILEPRO_IMM16_X0_GOT_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_GOT_LO, R_TILEPRO_IMM16_X1_GOT_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_GOT_HI, R_TILEPRO_IMM16_X0_GOT_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_GOT_HI, R_TILEPRO_IMM16_X1_GOT_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_GOT_HA, R_TILEPRO_IMM16_X0_GOT_HA)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_GOT_HA, R_TILEPRO_IMM16_X1_GOT_HA)

  TH_REMAP (BFD_RELOC_TILEPRO_MMSTART_X0,  R_TILEPRO_MMSTART_X0)
  TH_REMAP (BFD_RELOC_TILEPRO_MMEND_X0,	   R_TILEPRO_MMEND_X0)
  TH_REMAP (BFD_RELOC_TILEPRO_MMSTART_X1,  R_TILEPRO_MMSTART_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_MMEND_X1,	   R_TILEPRO_MMEND_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_SHAMT_X0,	   R_TILEPRO_SHAMT_X0)
  TH_REMAP (BFD_RELOC_TILEPRO_SHAMT_X1,	   R_TILEPRO_SHAMT_X1)
  TH_REMAP (BFD_RELOC_TILEPRO_SHAMT_Y0,	   R_TILEPRO_SHAMT_Y0)
  TH_REMAP (BFD_RELOC_TILEPRO_SHAMT_Y1,	   R_TILEPRO_SHAMT_Y1)

  TH_REMAP (BFD_RELOC_TILEPRO_TLS_GD_CALL,	  R_TILEPRO_TLS_GD_CALL)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM8_X0_TLS_GD_ADD, R_TILEPRO_IMM8_X0_TLS_GD_ADD)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM8_X1_TLS_GD_ADD, R_TILEPRO_IMM8_X1_TLS_GD_ADD)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM8_Y0_TLS_GD_ADD, R_TILEPRO_IMM8_Y0_TLS_GD_ADD)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM8_Y1_TLS_GD_ADD, R_TILEPRO_IMM8_Y1_TLS_GD_ADD)
  TH_REMAP (BFD_RELOC_TILEPRO_TLS_IE_LOAD,	  R_TILEPRO_TLS_IE_LOAD)

  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD,	  R_TILEPRO_IMM16_X0_TLS_GD)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD,	  R_TILEPRO_IMM16_X1_TLS_GD)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_LO, R_TILEPRO_IMM16_X0_TLS_GD_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_LO, R_TILEPRO_IMM16_X1_TLS_GD_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_HI, R_TILEPRO_IMM16_X0_TLS_GD_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_HI, R_TILEPRO_IMM16_X1_TLS_GD_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_HA, R_TILEPRO_IMM16_X0_TLS_GD_HA)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_HA, R_TILEPRO_IMM16_X1_TLS_GD_HA)

  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE,	  R_TILEPRO_IMM16_X0_TLS_IE)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE,	  R_TILEPRO_IMM16_X1_TLS_IE)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_LO, R_TILEPRO_IMM16_X0_TLS_IE_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_LO, R_TILEPRO_IMM16_X1_TLS_IE_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_HI, R_TILEPRO_IMM16_X0_TLS_IE_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_HI, R_TILEPRO_IMM16_X1_TLS_IE_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_HA, R_TILEPRO_IMM16_X0_TLS_IE_HA)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_HA, R_TILEPRO_IMM16_X1_TLS_IE_HA)

  TH_REMAP (BFD_RELOC_TILEPRO_TLS_DTPMOD32, R_TILEPRO_TLS_DTPMOD32)
  TH_REMAP (BFD_RELOC_TILEPRO_TLS_DTPOFF32, R_TILEPRO_TLS_DTPOFF32)
  TH_REMAP (BFD_RELOC_TILEPRO_TLS_TPOFF32,  R_TILEPRO_TLS_TPOFF32)

  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE,	  R_TILEPRO_IMM16_X0_TLS_LE)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE,	  R_TILEPRO_IMM16_X1_TLS_LE)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_LO, R_TILEPRO_IMM16_X0_TLS_LE_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_LO, R_TILEPRO_IMM16_X1_TLS_LE_LO)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_HI, R_TILEPRO_IMM16_X0_TLS_LE_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_HI, R_TILEPRO_IMM16_X1_TLS_LE_HI)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_HA, R_TILEPRO_IMM16_X0_TLS_LE_HA)
  TH_REMAP (BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_HA, R_TILEPRO_IMM16_X1_TLS_LE_HA)

#undef TH_REMAP

  { BFD_RELOC_VTABLE_INHERIT, R_TILEPRO_GNU_VTINHERIT, tilepro_elf_howto_table2 },
  { BFD_RELOC_VTABLE_ENTRY,   R_TILEPRO_GNU_VTENTRY,   tilepro_elf_howto_table2 },
};



/* TILEPRO ELF linker hash entry.  */

struct tilepro_elf_link_hash_entry
{
  struct elf_link_hash_entry elf;

#define GOT_UNKNOWN     0
#define GOT_NORMAL      1
#define GOT_TLS_GD      2
#define GOT_TLS_IE      4
  unsigned char tls_type;
};

#define tilepro_elf_hash_entry(ent) \
  ((struct tilepro_elf_link_hash_entry *)(ent))

struct _bfd_tilepro_elf_obj_tdata
{
  struct elf_obj_tdata root;

  /* tls_type for each local got entry.  */
  char *local_got_tls_type;
};

#define _bfd_tilepro_elf_tdata(abfd) \
  ((struct _bfd_tilepro_elf_obj_tdata *) (abfd)->tdata.any)

#define _bfd_tilepro_elf_local_got_tls_type(abfd) \
  (_bfd_tilepro_elf_tdata (abfd)->local_got_tls_type)

#define is_tilepro_elf(bfd)				\
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour	\
   && elf_tdata (bfd) != NULL				\
   && elf_object_id (bfd) == TILEPRO_ELF_DATA)

/* Allocate TILEPro ELF private object data.  */

static bool
tilepro_elf_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd,
				  sizeof (struct _bfd_tilepro_elf_obj_tdata),
				  TILEPRO_ELF_DATA);
}

#include "elf/common.h"
#include "elf/internal.h"

/* Get the Tilepro ELF linker hash table from a link_info structure.  */
#define tilepro_elf_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == TILEPRO_ELF_DATA)	\
   ? (struct elf_link_hash_table *) (p)->hash : NULL)

static reloc_howto_type *
tilepro_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
			   bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = ARRAY_SIZE (tilepro_reloc_map); i--;)
    {
      const reloc_map * entry;

      entry = tilepro_reloc_map + i;

      if (entry->bfd_reloc_val == code)
	return entry->table + (entry->tilepro_reloc_val
			       - entry->table[0].type);
    }

  return NULL;
}

static reloc_howto_type *
tilepro_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			   const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (tilepro_elf_howto_table)
	    / sizeof (tilepro_elf_howto_table[0]));
       i++)
    if (tilepro_elf_howto_table[i].name != NULL
	&& strcasecmp (tilepro_elf_howto_table[i].name, r_name) == 0)
      return &tilepro_elf_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an TILEPro ELF reloc.  */

static bool
tilepro_info_to_howto_rela (bfd * abfd ATTRIBUTE_UNUSED,
			    arelent * cache_ptr,
			    Elf_Internal_Rela * dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  if (r_type <= (unsigned int) R_TILEPRO_IMM16_X1_TLS_LE_HA)
    cache_ptr->howto = &tilepro_elf_howto_table [r_type];
  else if (r_type - R_TILEPRO_GNU_VTINHERIT
	   <= ((unsigned int) R_TILEPRO_GNU_VTENTRY
	       - (unsigned int) R_TILEPRO_GNU_VTINHERIT))
    cache_ptr->howto
      = &tilepro_elf_howto_table2 [r_type - R_TILEPRO_GNU_VTINHERIT];
  else
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  return true;
}

typedef tilepro_bundle_bits (*tilepro_create_func)(int);

static const tilepro_create_func reloc_to_create_func[] =
{
  /* The first fourteen relocation types don't correspond to operands */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  /* The remaining relocations are used for immediate operands */
  create_BrOff_X1,
  create_JOffLong_X1,
  create_JOffLong_X1,
  create_Imm8_X0,
  create_Imm8_Y0,
  create_Imm8_X1,
  create_Imm8_Y1,
  create_MT_Imm15_X1,
  create_MF_Imm15_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_MMStart_X0,
  create_MMEnd_X0,
  create_MMStart_X1,
  create_MMEnd_X1,
  create_ShAmt_X0,
  create_ShAmt_X1,
  create_ShAmt_Y0,
  create_ShAmt_Y1,

  create_Dest_Imm8_X1,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,

  NULL,
  NULL,
  NULL,

  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
};

#define NELEMS(a)	((int) (sizeof (a) / sizeof ((a)[0])))

/* Support for core dump NOTE sections.  */

static bool
tilepro_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  if (note->descsz != TILEPRO_PRSTATUS_SIZEOF)
    return false;

  /* pr_cursig */
  elf_tdata (abfd)->core->signal =
    bfd_get_16 (abfd, note->descdata + TILEPRO_PRSTATUS_OFFSET_PR_CURSIG);

  /* pr_pid */
  elf_tdata (abfd)->core->pid =
    bfd_get_32 (abfd, note->descdata + TILEPRO_PRSTATUS_OFFSET_PR_PID);

  /* pr_reg */
  offset = TILEPRO_PRSTATUS_OFFSET_PR_REG;
  size   = TILEPRO_GREGSET_T_SIZE;

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
tilepro_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz != TILEPRO_PRPSINFO_SIZEOF)
    return false;

  elf_tdata (abfd)->core->program
    = _bfd_elfcore_strndup (abfd,
			    note->descdata + TILEPRO_PRPSINFO_OFFSET_PR_FNAME,
			    16);
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd,
			    note->descdata + TILEPRO_PRPSINFO_OFFSET_PR_PSARGS,
			    ELF_PR_PSARGS_SIZE);


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


static void
tilepro_elf_append_rela_32 (bfd *abfd, asection *s, Elf_Internal_Rela *rel)
{
  Elf32_External_Rela *loc32;

  loc32 = (Elf32_External_Rela *) s->contents;
  loc32 += s->reloc_count++;
  bfd_elf32_swap_reloca_out (abfd, rel, (bfd_byte *) loc32);
}

/* PLT/GOT stuff */

/* The procedure linkage table starts with the following header:

   {
    rli	    r29, r29, 16
    lwadd   r28, r27, 4
   }
    lw	    r27, r27
   {
    info    10		  ## SP not offset, return PC in LR
    jr	    r27
   }

   Subsequent entries are the following, jumping to the header at the end:

   lnk	   r28
1:
   {
    auli    r28, r28, <_GLOBAL_OFFSET_TABLE_ - 1b + MY_GOT_OFFSET>
    auli    r27, r28, <_GLOBAL_OFFSET_TABLE_ - 1b>
   }
   {
    addli   r28, r28, <_GLOBAL_OFFSET_TABLE_ - 1b + MY_GOT_OFFSET>
    addli   r27, r27, <_GLOBAL_OFFSET_TABLE_ - 1b>
   }
   {
    auli    r29, zero, MY_PLT_INDEX
    lw	    r28, r28
   }
   {
    info    10		  ## SP not offset, return PC in LR
    jr	    r28
   }

   We initially store MY_PLT_INDEX in the high bits so that we can use the all
   16 bits as an unsigned offset; if we use the low bits we would get an
   unwanted sign extension.  The PLT header then rotates the index to get the
   right value, before calling the resolution routine.  This computation can
   fit in unused bundle slots so it's free.

   This code sequence lets the code at at the start of the PLT determine
   which PLT entry was executed by examining 'r29'.

   Note that MY_PLT_INDEX skips over the header entries, so the first
   actual jump table entry has index zero.
*/

#define PLT_HEADER_SIZE_IN_BUNDLES 3
#define PLT_ENTRY_SIZE_IN_BUNDLES  5

#define PLT_HEADER_SIZE \
  (PLT_HEADER_SIZE_IN_BUNDLES * TILEPRO_BUNDLE_SIZE_IN_BYTES)
#define PLT_ENTRY_SIZE \
  (PLT_ENTRY_SIZE_IN_BUNDLES * TILEPRO_BUNDLE_SIZE_IN_BYTES)

/* The size in bytes of an entry in the global offset table.  */

#define GOT_ENTRY_SIZE TILEPRO_BYTES_PER_WORD

#define GOTPLT_HEADER_SIZE (2 * GOT_ENTRY_SIZE)


static const bfd_byte
tilepro_plt0_entry[PLT_HEADER_SIZE] =
{
  0x5d, 0x07, 0x03, 0x70,
  0x6e, 0x23, 0xd0, 0x30, /* { rli r29, r29, 16 ; lwadd r28, r27, 4 } */
  0x00, 0x50, 0xba, 0x6d,
  0x00, 0x08, 0x6d, 0xdc, /* { lw r27, r27 } */
  0xff, 0xaf, 0x10, 0x50,
  0x60, 0x03, 0x18, 0x08, /* { info 10 ; jr r27 } */
};

static const bfd_byte
tilepro_short_plt_entry[PLT_ENTRY_SIZE] =
{
  0x00, 0x50, 0x16, 0x70,
  0x0e, 0x00, 0x1a, 0x08, /* { lnk r28 } */
  0x1c, 0x07, 0x00, 0xa0,
  0x8d, 0x03, 0x00, 0x18, /* { addli r28, r28, 0 ; addli r27, r28, 0 } */
  0xdd, 0x0f, 0x00, 0x30,
  0x8e, 0x73, 0x0b, 0x40, /* { auli r29, zero, 0 ; lw r28, r28 } */
  0xff, 0xaf, 0x10, 0x50,
  0x80, 0x03, 0x18, 0x08, /* { info 10 ; jr r28 } */
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
};

static const bfd_byte
tilepro_long_plt_entry[PLT_ENTRY_SIZE] =
{
  0x00, 0x50, 0x16, 0x70,
  0x0e, 0x00, 0x1a, 0x08, /* { lnk r28 } */
  0x1c, 0x07, 0x00, 0xb0,
  0x8d, 0x03, 0x00, 0x20, /* { auli r28, r28, 0 ; auli r27, r28, 0 } */
  0x1c, 0x07, 0x00, 0xa0,
  0x6d, 0x03, 0x00, 0x18, /* { addli r28, r28, 0 ; addli r27, r27, 0 } */
  0xdd, 0x0f, 0x00, 0x30,
  0x8e, 0x73, 0x0b, 0x40, /* { auli r29, zero, 0 ; lw r28, r28 } */
  0xff, 0xaf, 0x10, 0x50,
  0x80, 0x03, 0x18, 0x08, /* { info 10 ; jr r28 } */
};

static bfd_vma
tilepro_ha16(bfd_vma vma)
{
  return ((vma >> 16) + ((vma >> 15) & 1)) & 0xffff;
}

static int
tilepro_plt_entry_build (asection *splt, asection *sgotplt, bfd_vma offset,
		      bfd_vma *r_offset)
{
  int plt_index = (offset - PLT_ENTRY_SIZE) / PLT_ENTRY_SIZE;
  int got_offset = plt_index * GOT_ENTRY_SIZE + GOTPLT_HEADER_SIZE;
  tilepro_bundle_bits *pc;

  /* Compute the distance from the got entry to the lnk.  */
  bfd_signed_vma dist_got_entry = sgotplt->output_section->vma
    + sgotplt->output_offset
    + got_offset
    - splt->output_section->vma
    - splt->output_offset
    - offset
    - TILEPRO_BUNDLE_SIZE_IN_BYTES;

  /* Compute the distance to GOTPLT[0].  */
  bfd_signed_vma dist_got0 = dist_got_entry - got_offset;

  /* Check whether we can use the short plt entry with 16-bit offset.  */
  bool short_plt_entry =
    (dist_got_entry <= 0x7fff && dist_got0 >= -0x8000);

  /* Copy the plt entry template.  */
  memcpy (splt->contents + offset,
	  short_plt_entry ? tilepro_short_plt_entry : tilepro_long_plt_entry,
	  PLT_ENTRY_SIZE);

  /* Write the immediate offsets.  */
  pc = (tilepro_bundle_bits *)(splt->contents + offset);
  pc++;

  if (!short_plt_entry)
    {
      /* { auli r28, r28, &GOTPLT[MY_GOT_INDEX] ; auli r27, r28, &GOTPLT[0] } */
      *pc++ |= create_Imm16_X0 (tilepro_ha16 (dist_got_entry))
	| create_Imm16_X1 (tilepro_ha16 (dist_got0));
    }

  /* { addli r28, r28, &GOTPLT[MY_GOT_INDEX] ; addli r27, r28, &GOTPLT[0] } or
     { addli r28, r28, &GOTPLT[MY_GOT_INDEX] ; addli r27, r27, &GOTPLT[0] } */
  *pc++ |= create_Imm16_X0 (dist_got_entry)
    | create_Imm16_X1 (dist_got0);

  /* { auli r29, zero, MY_PLT_INDEX ; lw r28, r28 } */
  *pc |= create_Imm16_X0 (plt_index);

  /* Set the relocation offset.  */
  *r_offset = got_offset;

  return plt_index;
}

#define TILEPRO_ELF_RELA_BYTES (sizeof(Elf32_External_Rela))


/* Create an entry in an TILEPro ELF linker hash table.  */

static struct bfd_hash_entry *
link_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table, const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry =
	bfd_hash_allocate (table,
			   sizeof (struct tilepro_elf_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct tilepro_elf_link_hash_entry *eh;

      eh = (struct tilepro_elf_link_hash_entry *) entry;
      eh->tls_type = GOT_UNKNOWN;
    }

  return entry;
}

/* Create a TILEPRO ELF linker hash table.  */

static struct bfd_link_hash_table *
tilepro_elf_link_hash_table_create (bfd *abfd)
{
  struct elf_link_hash_table *ret;
  size_t amt = sizeof (struct elf_link_hash_table);

  ret = (struct elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (ret, abfd, link_hash_newfunc,
				      sizeof (struct tilepro_elf_link_hash_entry),
				      TILEPRO_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->root;
}

/* Create the .got section.  */

static bool
tilepro_elf_create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags;
  asection *s, *s_got;
  struct elf_link_hash_entry *h;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* This function may be called more than once.  */
  if (htab->sgot != NULL)
    return true;

  flags = bed->dynamic_sec_flags;

  s = bfd_make_section_with_flags (abfd,
				   (bed->rela_plts_and_copies_p
				    ? ".rela.got" : ".rel.got"),
				   (bed->dynamic_sec_flags
				    | SEC_READONLY));
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->srelgot = s;

  s = s_got = bfd_make_section_with_flags (abfd, ".got", flags);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->sgot = s;

  /* The first bit of the global offset table is the header.  */
  s->size += bed->got_header_size;

  if (bed->want_got_plt)
    {
      s = bfd_make_section_with_flags (abfd, ".got.plt", flags);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->sgotplt = s;

      /* Reserve room for the header.  */
      s->size += GOTPLT_HEADER_SIZE;
    }

  if (bed->want_got_sym)
    {
      /* Define the symbol _GLOBAL_OFFSET_TABLE_ at the start of the .got
	 section.  We don't do this in the linker script because we don't want
	 to define the symbol if we are not creating a global offset
	 table.  */
      h = _bfd_elf_define_linkage_sym (abfd, info, s_got,
				       "_GLOBAL_OFFSET_TABLE_");
      elf_hash_table (info)->hgot = h;
      if (h == NULL)
	return false;
    }

  return true;
}

/* Create .plt, .rela.plt, .got, .got.plt, .rela.got, .dynbss, and
   .rela.bss sections in DYNOBJ, and set up shortcuts to them in our
   hash table.  */

static bool
tilepro_elf_create_dynamic_sections (bfd *dynobj,
				     struct bfd_link_info *info)
{
  if (!tilepro_elf_create_got_section (dynobj, info))
    return false;

  return _bfd_elf_create_dynamic_sections (dynobj, info);
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
tilepro_elf_copy_indirect_symbol (struct bfd_link_info *info,
				  struct elf_link_hash_entry *dir,
				  struct elf_link_hash_entry *ind)
{
  struct tilepro_elf_link_hash_entry *edir, *eind;

  edir = (struct tilepro_elf_link_hash_entry *) dir;
  eind = (struct tilepro_elf_link_hash_entry *) ind;

  if (ind->root.type == bfd_link_hash_indirect
      && dir->got.refcount <= 0)
    {
      edir->tls_type = eind->tls_type;
      eind->tls_type = GOT_UNKNOWN;
    }
  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

static int
tilepro_tls_translate_to_le (int r_type)
{
  switch (r_type)
    {
    case R_TILEPRO_IMM16_X0_TLS_GD:
    case R_TILEPRO_IMM16_X0_TLS_IE:
      return R_TILEPRO_IMM16_X0_TLS_LE;

    case R_TILEPRO_IMM16_X1_TLS_GD:
    case R_TILEPRO_IMM16_X1_TLS_IE:
      return R_TILEPRO_IMM16_X1_TLS_LE;

    case R_TILEPRO_IMM16_X0_TLS_GD_LO:
    case R_TILEPRO_IMM16_X0_TLS_IE_LO:
      return R_TILEPRO_IMM16_X0_TLS_LE_LO;

    case R_TILEPRO_IMM16_X1_TLS_GD_LO:
    case R_TILEPRO_IMM16_X1_TLS_IE_LO:
      return R_TILEPRO_IMM16_X1_TLS_LE_LO;

    case R_TILEPRO_IMM16_X0_TLS_GD_HI:
    case R_TILEPRO_IMM16_X0_TLS_IE_HI:
      return R_TILEPRO_IMM16_X0_TLS_LE_HI;

    case R_TILEPRO_IMM16_X1_TLS_GD_HI:
    case R_TILEPRO_IMM16_X1_TLS_IE_HI:
      return R_TILEPRO_IMM16_X1_TLS_LE_HI;

    case R_TILEPRO_IMM16_X0_TLS_GD_HA:
    case R_TILEPRO_IMM16_X0_TLS_IE_HA:
      return R_TILEPRO_IMM16_X0_TLS_LE_HA;

    case R_TILEPRO_IMM16_X1_TLS_GD_HA:
    case R_TILEPRO_IMM16_X1_TLS_IE_HA:
      return R_TILEPRO_IMM16_X1_TLS_LE_HA;
    }
  return r_type;
}

static int
tilepro_tls_translate_to_ie (int r_type)
{
  switch (r_type)
    {
    case R_TILEPRO_IMM16_X0_TLS_GD:
    case R_TILEPRO_IMM16_X0_TLS_IE:
      return R_TILEPRO_IMM16_X0_TLS_IE;

    case R_TILEPRO_IMM16_X1_TLS_GD:
    case R_TILEPRO_IMM16_X1_TLS_IE:
      return R_TILEPRO_IMM16_X1_TLS_IE;

    case R_TILEPRO_IMM16_X0_TLS_GD_LO:
    case R_TILEPRO_IMM16_X0_TLS_IE_LO:
      return R_TILEPRO_IMM16_X0_TLS_IE_LO;

    case R_TILEPRO_IMM16_X1_TLS_GD_LO:
    case R_TILEPRO_IMM16_X1_TLS_IE_LO:
      return R_TILEPRO_IMM16_X1_TLS_IE_LO;

    case R_TILEPRO_IMM16_X0_TLS_GD_HI:
    case R_TILEPRO_IMM16_X0_TLS_IE_HI:
      return R_TILEPRO_IMM16_X0_TLS_IE_HI;

    case R_TILEPRO_IMM16_X1_TLS_GD_HI:
    case R_TILEPRO_IMM16_X1_TLS_IE_HI:
      return R_TILEPRO_IMM16_X1_TLS_IE_HI;

    case R_TILEPRO_IMM16_X0_TLS_GD_HA:
    case R_TILEPRO_IMM16_X0_TLS_IE_HA:
      return R_TILEPRO_IMM16_X0_TLS_IE_HA;

    case R_TILEPRO_IMM16_X1_TLS_GD_HA:
    case R_TILEPRO_IMM16_X1_TLS_IE_HA:
      return R_TILEPRO_IMM16_X1_TLS_IE_HA;
    }
  return r_type;
}

static int
tilepro_elf_tls_transition (struct bfd_link_info *info, int r_type,
			    int is_local)
{
  if (!bfd_link_executable (info))
    return r_type;

  if (is_local)
    return tilepro_tls_translate_to_le (r_type);
  else
    return tilepro_tls_translate_to_ie (r_type);
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */

static bool
tilepro_elf_check_relocs (bfd *abfd, struct bfd_link_info *info,
			  asection *sec, const Elf_Internal_Rela *relocs)
{
  struct elf_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;
  int num_relocs;

  if (bfd_link_relocatable (info))
    return true;

  htab = tilepro_elf_hash_table (info);
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  sreloc = NULL;

  num_relocs = sec->reloc_count;

  BFD_ASSERT (is_tilepro_elf (abfd) || num_relocs == 0);

  if (htab->dynobj == NULL)
    htab->dynobj = abfd;

  rel_end = relocs + num_relocs;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      unsigned int r_symndx;
      struct elf_link_hash_entry *h;
      int tls_type;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_symndx >= NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: bad symbol index: %d"),
			      abfd, r_symndx);
	  return false;
	}

      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      r_type = tilepro_elf_tls_transition (info, r_type, h == NULL);
      switch (r_type)
	{
	case R_TILEPRO_IMM16_X0_TLS_LE:
	case R_TILEPRO_IMM16_X1_TLS_LE:
	case R_TILEPRO_IMM16_X0_TLS_LE_LO:
	case R_TILEPRO_IMM16_X1_TLS_LE_LO:
	case R_TILEPRO_IMM16_X0_TLS_LE_HI:
	case R_TILEPRO_IMM16_X1_TLS_LE_HI:
	case R_TILEPRO_IMM16_X0_TLS_LE_HA:
	case R_TILEPRO_IMM16_X1_TLS_LE_HA:
	  if (!bfd_link_executable (info))
	    goto r_tilepro_plt32;
	  break;

	case R_TILEPRO_IMM16_X0_TLS_GD:
	case R_TILEPRO_IMM16_X1_TLS_GD:
	case R_TILEPRO_IMM16_X0_TLS_GD_LO:
	case R_TILEPRO_IMM16_X1_TLS_GD_LO:
	case R_TILEPRO_IMM16_X0_TLS_GD_HI:
	case R_TILEPRO_IMM16_X1_TLS_GD_HI:
	case R_TILEPRO_IMM16_X0_TLS_GD_HA:
	case R_TILEPRO_IMM16_X1_TLS_GD_HA:
	  BFD_ASSERT (bfd_link_pic (info));
	  tls_type = GOT_TLS_GD;
	  goto have_got_reference;

	case R_TILEPRO_IMM16_X0_TLS_IE:
	case R_TILEPRO_IMM16_X1_TLS_IE:
	case R_TILEPRO_IMM16_X0_TLS_IE_LO:
	case R_TILEPRO_IMM16_X1_TLS_IE_LO:
	case R_TILEPRO_IMM16_X0_TLS_IE_HI:
	case R_TILEPRO_IMM16_X1_TLS_IE_HI:
	case R_TILEPRO_IMM16_X0_TLS_IE_HA:
	case R_TILEPRO_IMM16_X1_TLS_IE_HA:
	  tls_type = GOT_TLS_IE;
	  if (!bfd_link_executable (info))
	    info->flags |= DF_STATIC_TLS;
	  goto have_got_reference;

	case R_TILEPRO_IMM16_X0_GOT:
	case R_TILEPRO_IMM16_X1_GOT:
	case R_TILEPRO_IMM16_X0_GOT_LO:
	case R_TILEPRO_IMM16_X1_GOT_LO:
	case R_TILEPRO_IMM16_X0_GOT_HI:
	case R_TILEPRO_IMM16_X1_GOT_HI:
	case R_TILEPRO_IMM16_X0_GOT_HA:
	case R_TILEPRO_IMM16_X1_GOT_HA:
	   tls_type = GOT_NORMAL;
	   /* Fall Through */

	have_got_reference:
	  /* This symbol requires a global offset table entry.  */
	  {
	    int old_tls_type;

	    if (h != NULL)
	      {
		h->got.refcount += 1;
		old_tls_type = tilepro_elf_hash_entry(h)->tls_type;
	      }
	    else
	      {
		bfd_signed_vma *local_got_refcounts;

		/* This is a global offset table entry for a local symbol.  */
		local_got_refcounts = elf_local_got_refcounts (abfd);
		if (local_got_refcounts == NULL)
		  {
		    bfd_size_type size;

		    size = symtab_hdr->sh_info;
		    size *= (sizeof (bfd_signed_vma) + sizeof(char));
		    local_got_refcounts = ((bfd_signed_vma *)
					   bfd_zalloc (abfd, size));
		    if (local_got_refcounts == NULL)
		      return false;
		    elf_local_got_refcounts (abfd) = local_got_refcounts;
		    _bfd_tilepro_elf_local_got_tls_type (abfd)
		      = (char *) (local_got_refcounts + symtab_hdr->sh_info);
		  }
		local_got_refcounts[r_symndx] += 1;
		old_tls_type =
		  _bfd_tilepro_elf_local_got_tls_type (abfd) [r_symndx];
	      }

	    /* If a TLS symbol is accessed using IE at least once,
	       there is no point to use dynamic model for it.  */
	    if (old_tls_type != tls_type && old_tls_type != GOT_UNKNOWN
		&& (old_tls_type != GOT_TLS_GD
		    || tls_type != GOT_TLS_IE))
	      {
		if (old_tls_type == GOT_TLS_IE && tls_type == GOT_TLS_GD)
		  tls_type = old_tls_type;
		else
		  {
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: `%s' accessed both as normal and thread local symbol"),
		       abfd, h ? h->root.root.string : "<local>");
		    return false;
		  }
	      }

	    if (old_tls_type != tls_type)
	      {
		if (h != NULL)
		  tilepro_elf_hash_entry (h)->tls_type = tls_type;
		else
		  _bfd_tilepro_elf_local_got_tls_type (abfd) [r_symndx] =
		    tls_type;
	      }
	  }

	  if (htab->sgot == NULL)
	    {
	      if (!tilepro_elf_create_got_section (htab->dynobj, info))
		return false;
	    }
	  break;

	case R_TILEPRO_TLS_GD_CALL:
	  if (!bfd_link_executable (info))
	    {
	      /* These are basically R_TILEPRO_JOFFLONG_X1_PLT relocs
		 against __tls_get_addr.  */
	      struct bfd_link_hash_entry *bh = NULL;
	      if (! _bfd_generic_link_add_one_symbol (info, abfd,
						      "__tls_get_addr", 0,
						      bfd_und_section_ptr, 0,
						      NULL, false, false,
						      &bh))
		return false;
	      h = (struct elf_link_hash_entry *) bh;
	    }
	  else
	    break;
	  /* Fall through */

	case R_TILEPRO_JOFFLONG_X1_PLT:
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code without
	     linking in any dynamic objects, in which case we don't
	     need to generate a procedure linkage table after all.  */

	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      h->plt.refcount += 1;
	    }
	  break;

	case R_TILEPRO_32_PCREL:
	case R_TILEPRO_16_PCREL:
	case R_TILEPRO_8_PCREL:
	case R_TILEPRO_IMM16_X0_PCREL:
	case R_TILEPRO_IMM16_X1_PCREL:
	case R_TILEPRO_IMM16_X0_LO_PCREL:
	case R_TILEPRO_IMM16_X1_LO_PCREL:
	case R_TILEPRO_IMM16_X0_HI_PCREL:
	case R_TILEPRO_IMM16_X1_HI_PCREL:
	case R_TILEPRO_IMM16_X0_HA_PCREL:
	case R_TILEPRO_IMM16_X1_HA_PCREL:
	  if (h != NULL)
	    h->non_got_ref = 1;

	  if (h != NULL
	      && strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
	    break;
	  /* Fall through.  */

	case R_TILEPRO_32:
	case R_TILEPRO_16:
	case R_TILEPRO_8:
	case R_TILEPRO_LO16:
	case R_TILEPRO_HI16:
	case R_TILEPRO_HA16:
	case R_TILEPRO_COPY:
	case R_TILEPRO_GLOB_DAT:
	case R_TILEPRO_JMP_SLOT:
	case R_TILEPRO_RELATIVE:
	case R_TILEPRO_BROFF_X1:
	case R_TILEPRO_JOFFLONG_X1:
	case R_TILEPRO_IMM8_X0:
	case R_TILEPRO_IMM8_Y0:
	case R_TILEPRO_IMM8_X1:
	case R_TILEPRO_IMM8_Y1:
	case R_TILEPRO_DEST_IMM8_X1:
	case R_TILEPRO_MT_IMM15_X1:
	case R_TILEPRO_MF_IMM15_X1:
	case R_TILEPRO_IMM16_X0:
	case R_TILEPRO_IMM16_X1:
	case R_TILEPRO_IMM16_X0_LO:
	case R_TILEPRO_IMM16_X1_LO:
	case R_TILEPRO_IMM16_X0_HI:
	case R_TILEPRO_IMM16_X1_HI:
	case R_TILEPRO_IMM16_X0_HA:
	case R_TILEPRO_IMM16_X1_HA:
	case R_TILEPRO_MMSTART_X0:
	case R_TILEPRO_MMEND_X0:
	case R_TILEPRO_MMSTART_X1:
	case R_TILEPRO_MMEND_X1:
	case R_TILEPRO_SHAMT_X0:
	case R_TILEPRO_SHAMT_X1:
	case R_TILEPRO_SHAMT_Y0:
	case R_TILEPRO_SHAMT_Y1:
	  if (h != NULL)
	      h->non_got_ref = 1;

	r_tilepro_plt32:
	  if (h != NULL && !bfd_link_pic (info))
	    {
	      /* We may need a .plt entry if the function this reloc
		 refers to is in a shared lib.  */
	      h->plt.refcount += 1;
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
	       && (! tilepro_elf_howto_table[r_type].pc_relative
		   || (h != NULL
		       && (! info->symbolic
			   || h->root.type == bfd_link_hash_defweak
			   || !h->def_regular))))
	      || (!bfd_link_pic (info)
		  && (sec->flags & SEC_ALLOC) != 0
		  && h != NULL
		  && (h->root.type == bfd_link_hash_defweak
		      || !h->def_regular)))
	    {
	      struct elf_dyn_relocs *p;
	      struct elf_dyn_relocs **head;

	      /* When creating a shared object, we must copy these
		 relocs into the output file.  We create a reloc
		 section in dynobj and make room for the reloc.  */
	      if (sreloc == NULL)
		{
		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->dynobj, 2, abfd, /*rela?*/ true);

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

		  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
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
		       bfd_alloc (htab->dynobj, amt));
		  if (p == NULL)
		    return false;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;
	      if (tilepro_elf_howto_table[r_type].pc_relative)
		p->pc_count += 1;
	    }

	  break;

	case R_TILEPRO_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	case R_TILEPRO_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	default:
	  break;
	}
    }

  return true;
}


static asection *
tilepro_elf_gc_mark_hook (asection *sec,
			  struct bfd_link_info *info,
			  Elf_Internal_Rela *rel,
			  struct elf_link_hash_entry *h,
			  Elf_Internal_Sym *sym)
{
  if (h != NULL)
    {
      switch (ELF32_R_TYPE (rel->r_info))
	{
	case R_TILEPRO_GNU_VTINHERIT:
	case R_TILEPRO_GNU_VTENTRY:
	  return NULL;
	}
    }

  /* FIXME: The test here, in check_relocs and in relocate_section
     dealing with TLS optimization, ought to be !bfd_link_executable (info).  */
  if (bfd_link_pic (info))
    {
      struct bfd_link_hash_entry *bh;

      switch (ELF32_R_TYPE (rel->r_info))
	{
	case R_TILEPRO_TLS_GD_CALL:
	  /* This reloc implicitly references __tls_get_addr.  We know
	     another reloc will reference the same symbol as the one
	     on this reloc, so the real symbol and section will be
	     gc marked when processing the other reloc.  That lets
	     us handle __tls_get_addr here.  */
	  bh = NULL;
	  if (! _bfd_generic_link_add_one_symbol (info, sec->owner,
						  "__tls_get_addr", 0,
						  bfd_und_section_ptr,
						  0, NULL, false,
						  false, &bh))
	    return NULL;
	  h = (struct elf_link_hash_entry *) bh;
	  BFD_ASSERT (h != NULL);
	  h->mark = 1;
	  if (h->is_weakalias)
	    weakdef (h)->mark = 1;
	  sym = NULL;
	}
    }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
tilepro_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				   struct elf_link_hash_entry *h)
{
  struct elf_link_hash_table *htab;
  asection *s, *srel;

  htab = tilepro_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (htab->dynobj != NULL
	      && (h->needs_plt
		  || h->is_weakalias
		  || (h->def_dynamic
		      && h->ref_regular
		      && !h->def_regular)));

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later
     (although we could actually do it here). */
  if (h->type == STT_FUNC || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a R_TILEPRO_JOFFLONG_X1_PLT
	     reloc in an input file, but the symbol was never referred
	     to by a dynamic object, or if all references were garbage
	     collected.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a
	     R_TILEPRO_JOFFLONG_X1 relocation instead. */
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

  /* We must generate a R_TILEPRO_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rel.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = htab->sdynrelro;
      srel = htab->sreldynrelro;
    }
  else
    {
      s = htab->sdynbss;
      srel = htab->srelbss;
    }
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      srel->size += TILEPRO_ELF_RELA_BYTES;
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct elf_link_hash_table *htab;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = tilepro_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (htab->dynamic_sections_created
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
	  asection *s = htab->splt;

	  /* Allocate room for the header.  */
	  if (s->size == 0)
	    {
	      s->size = PLT_ENTRY_SIZE;
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
	  s->size += PLT_ENTRY_SIZE;

	  /* We also need to make an entry in the .got.plt section.  */
	  htab->sgotplt->size += GOT_ENTRY_SIZE;

	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->srelplt->size += TILEPRO_ELF_RELA_BYTES;
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

  /* If a TLS_IE symbol is now local to the binary, make it a TLS_LE
     requiring no TLS entry.  */
  if (h->got.refcount > 0
      && bfd_link_executable (info)
      && h->dynindx == -1
      && tilepro_elf_hash_entry(h)->tls_type == GOT_TLS_IE)
    h->got.offset = (bfd_vma) -1;
  else if (h->got.refcount > 0)
    {
      asection *s;
      bool dyn;
      int tls_type = tilepro_elf_hash_entry(h)->tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      s = htab->sgot;
      h->got.offset = s->size;
      s->size += TILEPRO_BYTES_PER_WORD;
      /* R_TILEPRO_IMM16_Xn_TLS_GD entries need 2 consecutive GOT slots. */
      if (tls_type == GOT_TLS_GD)
	s->size += TILEPRO_BYTES_PER_WORD;
      dyn = htab->dynamic_sections_created;
      /* R_TILEPRO_IMM16_Xn_TLS_IE_xxx needs one dynamic relocation,
	 R_TILEPRO_IMM16_Xn_TLS_GD_xxx needs two if local symbol and two if
	 global.  */
      if (tls_type == GOT_TLS_GD || tls_type == GOT_TLS_IE)
	htab->srelgot->size += 2 * TILEPRO_ELF_RELA_BYTES;
      else if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						bfd_link_pic (info),
						h))
	htab->srelgot->size += TILEPRO_ELF_RELA_BYTES;
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
	      || (htab->dynamic_sections_created
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
  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;
      sreloc->size += p->count * TILEPRO_ELF_RELA_BYTES;
    }

  return true;
}

/* Return true if the dynamic symbol for a given section should be
   omitted when creating a shared library.  */

static bool
tilepro_elf_omit_section_dynsym (bfd *output_bfd,
				    struct bfd_link_info *info,
				    asection *p)
{
  /* We keep the .got section symbol so that explicit relocations
     against the _GLOBAL_OFFSET_TABLE_ symbol emitted in PIC mode
     can be turned into relocations against the .got symbol.  */
  if (strcmp (p->name, ".got") == 0)
    return false;

  return _bfd_elf_omit_section_dynsym_default (output_bfd, info, p);
}

/* Set the sizes of the dynamic sections.  */

#define ELF32_DYNAMIC_INTERPRETER "/lib/ld.so.1"

static bool
tilepro_elf_size_dynamic_sections (bfd *output_bfd,
				      struct bfd_link_info *info)
{
  (void)output_bfd;

  struct elf_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd *ibfd;

  htab = tilepro_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dynobj = htab->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = sizeof ELF32_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF32_DYNAMIC_INTERPRETER;
	}
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

      if (! is_tilepro_elf (ibfd))
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
		  srel->size += p->count * TILEPRO_ELF_RELA_BYTES;
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    {
		      info->flags |= DF_TEXTREL;

		      info->callbacks->minfo (_("%pB: dynamic relocation in read-only section `%pA'\n"),
					      p->sec->owner, p->sec);
		    }
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      local_tls_type = _bfd_tilepro_elf_local_got_tls_type (ibfd);
      s = htab->sgot;
      srel = htab->srelgot;
      for (; local_got < end_local_got; ++local_got, ++local_tls_type)
	{
	  if (*local_got > 0)
	    {
	      *local_got = s->size;
	      s->size += TILEPRO_BYTES_PER_WORD;
	      if (*local_tls_type == GOT_TLS_GD)
		s->size += TILEPRO_BYTES_PER_WORD;
	      if (bfd_link_pic (info)
		  || *local_tls_type == GOT_TLS_GD
		  || *local_tls_type == GOT_TLS_IE)
		srel->size += TILEPRO_ELF_RELA_BYTES;
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (htab, allocate_dynrelocs, info);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* If the .got section is more than 0x8000 bytes, we add
	 0x8000 to the value of _GLOBAL_OFFSET_TABLE_, so that 16
	 bit relocations have a greater chance of working. */
      if (htab->sgot->size >= 0x8000
	  && elf_hash_table (info)->hgot->root.u.def.value == 0)
	elf_hash_table (info)->hgot->root.u.def.value = 0x8000;
    }

  if (htab->sgotplt)
    {
      struct elf_link_hash_entry *got;
      got = elf_link_hash_lookup (elf_hash_table (info),
				  "_GLOBAL_OFFSET_TABLE_",
				  false, false, false);

      /* Don't allocate .got.plt section if there are no GOT nor PLT
	 entries and there is no refeence to _GLOBAL_OFFSET_TABLE_.  */
      if ((got == NULL
	   || !got->ref_regular_nonweak)
	  && (htab->sgotplt->size
	      == GOTPLT_HEADER_SIZE)
	  && (htab->splt == NULL
	      || htab->splt->size == 0)
	  && (htab->sgot == NULL
	      || (htab->sgot->size
		  == get_elf_backend_data (output_bfd)->got_header_size)))
	htab->sgotplt->size = 0;
    }

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->splt
	  || s == htab->sgot
	  || s == htab->sgotplt
	  || s == htab->sdynbss
	  || s == htab->sdynrelro)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (startswith (s->name, ".rela"))
	{
	  if (s->size != 0)
	    {
	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else
	{
	  /* It's not one of our sections.  */
	  continue;
	}

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

      /* Allocate memory for the section contents.  Zero the memory
	 for the benefit of .rela.plt, which has 4 unused entries
	 at the beginning, and we don't want garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, true);
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

/* Return the relocation value for R_TILEPRO_TLS_TPOFF32. */

static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;

  return (address - htab->tls_sec->vma);
}

/* Replace the MASK bits in ADDR with those in INSN, for the next
   TILEPRO_BUNDLE_SIZE_IN_BYTES bytes.  */

static void
tilepro_replace_insn (bfd_byte *addr, const bfd_byte *mask,
		      const bfd_byte *insn)
{
  int i;
  for (i = 0; i < TILEPRO_BUNDLE_SIZE_IN_BYTES; i++)
    {
      addr[i] = (addr[i] & ~mask[i]) | (insn[i] & mask[i]);
    }
}

/* Mask to extract the bits corresponding to an instruction in a
   specific pipe of a bundle.  */
static const bfd_byte insn_mask_X1[] = {
  0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0x7f
};

/* Mask to extract the bits corresponding to an instruction in a
   specific pipe of a bundle, minus the destination operand and the
   first source operand.  */
static const bfd_byte insn_mask_X0_no_dest_no_srca[] = {
  0x00, 0xf0, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00
};

static const bfd_byte insn_mask_X1_no_dest_no_srca[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x7f
};

static const bfd_byte insn_mask_Y0_no_dest_no_srca[] = {
  0x00, 0xf0, 0x0f, 0x78, 0x00, 0x00, 0x00, 0x00
};

static const bfd_byte insn_mask_Y1_no_dest_no_srca[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0x78
};

/* Mask to extract the first source operand of an instruction.  */
static const bfd_byte srca_mask_X0[] = {
  0xc0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const bfd_byte srca_mask_X1[] = {
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00
};

/* Various instructions synthesized to support tls references.  */

/* move r0, r0 in the X1 pipe, used for tls le.  */
static const bfd_byte insn_tls_le_move_X1[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x33, 0x08
};

/* move r0, zero in the X0 and X1 pipe, used for tls le.  */
static const bfd_byte insn_tls_le_move_zero_X0X1[] = {
  0xc0, 0xff, 0xcf, 0x00, 0xe0, 0xff, 0x33, 0x08
};

/* lw r0, r0 in the X1 pipe, used for tls ie.  */
static const bfd_byte insn_tls_ie_lw_X1[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x0b, 0x40
};

/* add r0, r0, tp in various pipes, used for tls ie.  */
static const bfd_byte insn_tls_ie_add_X0X1[] = {
  0x00, 0x50, 0x0f, 0x00, 0x00, 0xa8, 0x07, 0x08
};
static const bfd_byte insn_tls_ie_add_Y0Y1[] = {
  0x00, 0x50, 0x03, 0x08, 0x00, 0xa8, 0x01, 0x8c
};

/* move r0, r0 in various pipes, used for tls gd.  */
static const bfd_byte insn_tls_gd_add_X0X1[] = {
  0x00, 0xf0, 0xcf, 0x00, 0x00, 0xf8, 0x33, 0x08
};
static const bfd_byte insn_tls_gd_add_Y0Y1[] = {
  0x00, 0xf0, 0x0b, 0x18, 0x00, 0xf8, 0x05, 0x9c
};

/* Relocate an TILEPRO ELF section.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures.

   This function is responsible for adjusting the section contents as
   necessary, and (if generating a relocatable output file) adjusting
   the reloc addend as necessary.

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
tilepro_elf_relocate_section (bfd *output_bfd, struct bfd_link_info *info,
			      bfd *input_bfd, asection *input_section,
			      bfd_byte *contents, Elf_Internal_Rela *relocs,
			      Elf_Internal_Sym *local_syms,
			      asection **local_sections)
{
  struct elf_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_vma *local_got_offsets;
  bfd_vma got_base;
  asection *sreloc;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  int num_relocs;

  htab = tilepro_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);
  local_got_offsets = elf_local_got_offsets (input_bfd);

  if (elf_hash_table (info)->hgot == NULL)
    got_base = 0;
  else
    got_base = elf_hash_table (info)->hgot->root.u.def.value;

  sreloc = elf_section_data (input_section)->sreloc;

  rel = relocs;
  num_relocs = input_section->reloc_count;
  relend = relocs + num_relocs;
  for (; rel < relend; rel++)
    {
      int r_type, tls_type;
      bool is_tls_iele, is_tls_le;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *sym;
      tilepro_create_func create_func;
      asection *sec;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name;
      bfd_vma off;
      bool is_plt = false;
      bool resolved_to_zero;
      bool unresolved_reloc;

      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type == R_TILEPRO_GNU_VTINHERIT
	  || r_type == R_TILEPRO_GNU_VTENTRY)
	continue;

      if ((unsigned int)r_type >= NELEMS(tilepro_elf_howto_table))
	return _bfd_unrecognized_reloc (input_bfd, input_section, r_type);

      howto = tilepro_elf_howto_table + r_type;

      /* This is a final link.  */
      r_symndx = ELF32_R_SYM (rel->r_info);
      h = NULL;
      sym = NULL;
      sec = NULL;
      unresolved_reloc = false;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
	  bool warned ATTRIBUTE_UNUSED;
	  bool ignored ATTRIBUTE_UNUSED;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	  if (warned)
	    {
	      /* To avoid generating warning messages about truncated
		 relocations, set the relocation's address to be the same as
		 the start of this section.  */
	      if (input_section->output_section != NULL)
		relocation = input_section->output_section->vma;
	      else
		relocation = 0;
	    }
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      if (h != NULL)
	name = h->root.root.string;
      else
	{
	  name = (bfd_elf_string_from_elf_section
		  (input_bfd, symtab_hdr->sh_link, sym->st_name));
	  if (name == NULL || *name == '\0')
	    name = bfd_section_name (sec);
	}

      switch (r_type)
	{
	case R_TILEPRO_TLS_GD_CALL:
	case R_TILEPRO_IMM8_X0_TLS_GD_ADD:
	case R_TILEPRO_IMM8_Y0_TLS_GD_ADD:
	case R_TILEPRO_IMM8_X1_TLS_GD_ADD:
	case R_TILEPRO_IMM8_Y1_TLS_GD_ADD:
	case R_TILEPRO_IMM16_X0_TLS_GD_HA:
	case R_TILEPRO_IMM16_X1_TLS_GD_HA:
	case R_TILEPRO_IMM16_X0_TLS_IE_HA:
	case R_TILEPRO_IMM16_X1_TLS_IE_HA:
	  tls_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    tls_type =
	      _bfd_tilepro_elf_local_got_tls_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    tls_type = tilepro_elf_hash_entry(h)->tls_type;

	  is_tls_iele = (bfd_link_executable (info) || tls_type == GOT_TLS_IE);
	  is_tls_le = is_tls_iele && (bfd_link_executable (info)
				      && (h == NULL || h->dynindx == -1));

	  if (r_type == R_TILEPRO_TLS_GD_CALL)
	    {
	      if (is_tls_le)
		{
		  /* GD -> LE */
		  tilepro_replace_insn (contents + rel->r_offset,
					insn_mask_X1, insn_tls_le_move_X1);
		  continue;
		}
	      else if (is_tls_iele)
		{
		  /* GD -> IE */
		  tilepro_replace_insn (contents + rel->r_offset,
					insn_mask_X1, insn_tls_ie_lw_X1);
		  continue;
		}

	      /* GD -> GD */
	      h = (struct elf_link_hash_entry *)
		bfd_link_hash_lookup (info->hash, "__tls_get_addr", false,
				      false, true);
	      BFD_ASSERT (h != NULL);
	      r_type = R_TILEPRO_JOFFLONG_X1_PLT;
	      howto = tilepro_elf_howto_table + r_type;
	    }
	  else if (r_type == R_TILEPRO_IMM16_X0_TLS_GD_HA
		   || r_type == R_TILEPRO_IMM16_X0_TLS_IE_HA)
	    {
	      if (is_tls_le)
		tilepro_replace_insn (contents + rel->r_offset, srca_mask_X0,
				      insn_tls_le_move_zero_X0X1);
	    }
	  else if (r_type == R_TILEPRO_IMM16_X1_TLS_GD_HA
		   || r_type == R_TILEPRO_IMM16_X1_TLS_IE_HA)
	    {
	      if (is_tls_le)
		tilepro_replace_insn (contents + rel->r_offset, srca_mask_X1,
				      insn_tls_le_move_zero_X0X1);
	    }
	  else
	    {
	      const bfd_byte *mask = NULL;
	      const bfd_byte *add_insn = NULL;

	      switch (r_type)
		{
		case R_TILEPRO_IMM8_X0_TLS_GD_ADD:
		  add_insn = is_tls_iele ? insn_tls_ie_add_X0X1
		    : insn_tls_gd_add_X0X1;
		  mask = insn_mask_X0_no_dest_no_srca;
		  break;
		case R_TILEPRO_IMM8_X1_TLS_GD_ADD:
		  add_insn = is_tls_iele ? insn_tls_ie_add_X0X1
		    : insn_tls_gd_add_X0X1;
		  mask = insn_mask_X1_no_dest_no_srca;
		  break;
		case R_TILEPRO_IMM8_Y0_TLS_GD_ADD:
		  add_insn = is_tls_iele ? insn_tls_ie_add_Y0Y1
		    : insn_tls_gd_add_Y0Y1;
		  mask = insn_mask_Y0_no_dest_no_srca;
		  break;
		case R_TILEPRO_IMM8_Y1_TLS_GD_ADD:
		  add_insn = is_tls_iele ? insn_tls_ie_add_Y0Y1
		    : insn_tls_gd_add_Y0Y1;
		  mask = insn_mask_Y1_no_dest_no_srca;
		  break;
		}

	      tilepro_replace_insn (contents + rel->r_offset, mask, add_insn);

	      continue;
	    }
	  break;
	case R_TILEPRO_TLS_IE_LOAD:
	  if (bfd_link_executable (info) && (h == NULL || h->dynindx == -1))
	    /* IE -> LE */
	    tilepro_replace_insn (contents + rel->r_offset,
				  insn_mask_X1_no_dest_no_srca,
				  insn_tls_le_move_X1);
	  else
	    /* IE -> IE */
	    tilepro_replace_insn (contents + rel->r_offset,
				  insn_mask_X1_no_dest_no_srca,
				  insn_tls_ie_lw_X1);
	  continue;
	  break;
	default:
	  break;
	}

      resolved_to_zero = (h != NULL
			  && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));

      switch (r_type)
	{
	case R_TILEPRO_IMM16_X0_GOT:
	case R_TILEPRO_IMM16_X1_GOT:
	case R_TILEPRO_IMM16_X0_GOT_LO:
	case R_TILEPRO_IMM16_X1_GOT_LO:
	case R_TILEPRO_IMM16_X0_GOT_HI:
	case R_TILEPRO_IMM16_X1_GOT_HI:
	case R_TILEPRO_IMM16_X0_GOT_HA:
	case R_TILEPRO_IMM16_X1_GOT_HA:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */
	  if (htab->sgot == NULL)
	    abort ();

	  if (h != NULL)
	    {
	      bool dyn;

	      off = h->got.offset;
	      BFD_ASSERT (off != (bfd_vma) -1);
	      dyn = elf_hash_table (info)->dynamic_sections_created;

	      if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						     bfd_link_pic (info),
						     h)
		  || (bfd_link_pic (info)
		      && SYMBOL_REFERENCES_LOCAL (info, h)))
		{
		  /* This is actually a static link, or it is a
		     -Bsymbolic link and the symbol is defined
		     locally, or the symbol was forced to be local
		     because of a version file.  We must initialize
		     this entry in the global offset table.  Since the
		     offset must always be a multiple
		     of 4 for 32-bit, we use the least significant bit
		     to record whether we have initialized it already.

		     When doing a dynamic link, we create a .rela.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation,
					  htab->sgot->contents + off);
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

	      /* The offset must always be a multiple of 4 on 32-bit.
		 We use the least significant bit to record
		 whether we have already processed this entry.  */
	      if ((off & 1) != 0)
		off &= ~1;
	      else
		{
		  if (bfd_link_pic (info))
		    {
		      asection *s;
		      Elf_Internal_Rela outrel;

		      /* We need to generate a R_TILEPRO_RELATIVE reloc
			 for the dynamic linker.  */
		      s = htab->srelgot;
		      BFD_ASSERT (s != NULL);

		      outrel.r_offset = (htab->sgot->output_section->vma
					 + htab->sgot->output_offset
					 + off);
		      outrel.r_info = ELF32_R_INFO (0, R_TILEPRO_RELATIVE);
		      outrel.r_addend = relocation;
		      relocation = 0;
		      tilepro_elf_append_rela_32 (output_bfd, s, &outrel);
		    }

		  bfd_put_32 (output_bfd, relocation,
				      htab->sgot->contents + off);
		  local_got_offsets[r_symndx] |= 1;
		}
	    }
	  relocation = off - got_base;
	  break;

	case R_TILEPRO_JOFFLONG_X1_PLT:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */
	  BFD_ASSERT (h != NULL);

	  if (h->plt.offset == (bfd_vma) -1 || htab->splt == NULL)
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      break;
	    }

	  relocation = (htab->splt->output_section->vma
			+ htab->splt->output_offset
			+ h->plt.offset);
	  unresolved_reloc = false;
	  break;

	case R_TILEPRO_32_PCREL:
	case R_TILEPRO_16_PCREL:
	case R_TILEPRO_8_PCREL:
	case R_TILEPRO_IMM16_X0_PCREL:
	case R_TILEPRO_IMM16_X1_PCREL:
	case R_TILEPRO_IMM16_X0_LO_PCREL:
	case R_TILEPRO_IMM16_X1_LO_PCREL:
	case R_TILEPRO_IMM16_X0_HI_PCREL:
	case R_TILEPRO_IMM16_X1_HI_PCREL:
	case R_TILEPRO_IMM16_X0_HA_PCREL:
	case R_TILEPRO_IMM16_X1_HA_PCREL:
	  if (h != NULL
	      && strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
	    break;
	  /* Fall through.  */
	case R_TILEPRO_32:
	case R_TILEPRO_16:
	case R_TILEPRO_8:
	case R_TILEPRO_LO16:
	case R_TILEPRO_HI16:
	case R_TILEPRO_HA16:
	case R_TILEPRO_COPY:
	case R_TILEPRO_GLOB_DAT:
	case R_TILEPRO_JMP_SLOT:
	case R_TILEPRO_RELATIVE:
	case R_TILEPRO_BROFF_X1:
	case R_TILEPRO_JOFFLONG_X1:
	case R_TILEPRO_IMM8_X0:
	case R_TILEPRO_IMM8_Y0:
	case R_TILEPRO_IMM8_X1:
	case R_TILEPRO_IMM8_Y1:
	case R_TILEPRO_DEST_IMM8_X1:
	case R_TILEPRO_MT_IMM15_X1:
	case R_TILEPRO_MF_IMM15_X1:
	case R_TILEPRO_IMM16_X0:
	case R_TILEPRO_IMM16_X1:
	case R_TILEPRO_IMM16_X0_LO:
	case R_TILEPRO_IMM16_X1_LO:
	case R_TILEPRO_IMM16_X0_HI:
	case R_TILEPRO_IMM16_X1_HI:
	case R_TILEPRO_IMM16_X0_HA:
	case R_TILEPRO_IMM16_X1_HA:
	case R_TILEPRO_MMSTART_X0:
	case R_TILEPRO_MMEND_X0:
	case R_TILEPRO_MMSTART_X1:
	case R_TILEPRO_MMEND_X1:
	case R_TILEPRO_SHAMT_X0:
	case R_TILEPRO_SHAMT_X1:
	case R_TILEPRO_SHAMT_Y0:
	case R_TILEPRO_SHAMT_Y1:
	  if ((input_section->flags & SEC_ALLOC) == 0)
	    break;

	  if ((bfd_link_pic (info)
	       && (h == NULL
		   || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		       && !resolved_to_zero)
		   || h->root.type != bfd_link_hash_undefweak)
	       && (! howto->pc_relative
		   || !SYMBOL_CALLS_LOCAL (info, h)))
	      || (!bfd_link_pic (info)
		  && h != NULL
		  && h->dynindx != -1
		  && !h->non_got_ref
		  && ((h->def_dynamic
		       && !h->def_regular)
		      || h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined)))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip, relocate = false;

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
		skip = true, relocate = true;
	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);

	      switch (r_type)
		{
		case R_TILEPRO_32_PCREL:
		case R_TILEPRO_16_PCREL:
		case R_TILEPRO_8_PCREL:
		  /* If the symbol is not dynamic, we should not keep
		     a dynamic relocation.  But an .rela.* slot has been
		     allocated for it, output R_TILEPRO_NONE.
		     FIXME: Add code tracking needed dynamic relocs as
		     e.g. i386 has.  */
		  if (h->dynindx == -1)
		    skip = true, relocate = true;
		  break;
		}

	      if (skip)
		memset (&outrel, 0, sizeof outrel);
	      /* h->dynindx may be -1 if the symbol was marked to
		 become local.  */
	      else if (h != NULL &&
		       h->dynindx != -1
		       && (! is_plt
			   || !bfd_link_pic (info)
			   || !SYMBOLIC_BIND (info, h)
			   || !h->def_regular))
		{
		  BFD_ASSERT (h->dynindx != -1);
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  if (r_type == R_TILEPRO_32)
		    {
		      outrel.r_info = ELF32_R_INFO (0, R_TILEPRO_RELATIVE);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		  else
		    {
		      long indx;

		      outrel.r_addend = relocation + rel->r_addend;

		      if (is_plt)
			sec = htab->splt;

		      if (bfd_is_abs_section (sec))
			indx = 0;
		      else if (sec == NULL || sec->owner == NULL)
			{
			  bfd_set_error (bfd_error_bad_value);
			  return false;
			}
		      else
			{
			  asection *osec;

			  /* We are turning this relocation into one
			     against a section symbol.  It would be
			     proper to subtract the symbol's value,
			     osec->vma, from the emitted reloc addend,
			     but ld.so expects buggy relocs.  */
			  osec = sec->output_section;
			  indx = elf_section_data (osec)->dynindx;

			  if (indx == 0)
			    {
			      osec = htab->text_index_section;
			      indx = elf_section_data (osec)->dynindx;
			    }

			  /* FIXME: we really should be able to link non-pic
			     shared libraries.  */
			  if (indx == 0)
			    {
			      BFD_FAIL ();
			      _bfd_error_handler
				(_("%pB: probably compiled without -fPIC?"),
				 input_bfd);
			      bfd_set_error (bfd_error_bad_value);
			      return false;
			    }
			}

		      outrel.r_info = ELF32_R_INFO (indx, r_type);
		    }
		}

	      tilepro_elf_append_rela_32 (output_bfd, sreloc, &outrel);

	      /* This reloc will be computed at runtime, so there's no
		 need to do anything now.  */
	      if (! relocate)
		continue;
	    }
	  break;

	case R_TILEPRO_IMM16_X0_TLS_LE:
	case R_TILEPRO_IMM16_X1_TLS_LE:
	case R_TILEPRO_IMM16_X0_TLS_LE_LO:
	case R_TILEPRO_IMM16_X1_TLS_LE_LO:
	case R_TILEPRO_IMM16_X0_TLS_LE_HI:
	case R_TILEPRO_IMM16_X1_TLS_LE_HI:
	case R_TILEPRO_IMM16_X0_TLS_LE_HA:
	case R_TILEPRO_IMM16_X1_TLS_LE_HA:
	  if (!bfd_link_executable (info))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip;

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
	      else
		{
		  outrel.r_info = ELF32_R_INFO (0, r_type);
		  outrel.r_addend = relocation - dtpoff_base (info)
				    + rel->r_addend;
		}

	      tilepro_elf_append_rela_32 (output_bfd, sreloc, &outrel);
	      continue;
	    }
	  relocation = tpoff (info, relocation);
	  break;

	case R_TILEPRO_IMM16_X0_TLS_GD:
	case R_TILEPRO_IMM16_X1_TLS_GD:
	case R_TILEPRO_IMM16_X0_TLS_GD_LO:
	case R_TILEPRO_IMM16_X1_TLS_GD_LO:
	case R_TILEPRO_IMM16_X0_TLS_GD_HI:
	case R_TILEPRO_IMM16_X1_TLS_GD_HI:
	case R_TILEPRO_IMM16_X0_TLS_GD_HA:
	case R_TILEPRO_IMM16_X1_TLS_GD_HA:
	case R_TILEPRO_IMM16_X0_TLS_IE:
	case R_TILEPRO_IMM16_X1_TLS_IE:
	case R_TILEPRO_IMM16_X0_TLS_IE_LO:
	case R_TILEPRO_IMM16_X1_TLS_IE_LO:
	case R_TILEPRO_IMM16_X0_TLS_IE_HI:
	case R_TILEPRO_IMM16_X1_TLS_IE_HI:
	case R_TILEPRO_IMM16_X0_TLS_IE_HA:
	case R_TILEPRO_IMM16_X1_TLS_IE_HA:
	  r_type = tilepro_elf_tls_transition (info, r_type, h == NULL);
	  tls_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    tls_type
	      = _bfd_tilepro_elf_local_got_tls_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    {
	      tls_type = tilepro_elf_hash_entry(h)->tls_type;
	      if (bfd_link_executable (info)
		  && h->dynindx == -1
		  && tls_type == GOT_TLS_IE)
		r_type = tilepro_tls_translate_to_le (r_type);
	    }
	  if (tls_type == GOT_TLS_IE)
	    r_type = tilepro_tls_translate_to_ie (r_type);

	  if (r_type == R_TILEPRO_IMM16_X0_TLS_LE
	      || r_type == R_TILEPRO_IMM16_X1_TLS_LE
	      || r_type == R_TILEPRO_IMM16_X0_TLS_LE_LO
	      || r_type == R_TILEPRO_IMM16_X1_TLS_LE_LO
	      || r_type == R_TILEPRO_IMM16_X0_TLS_LE_HI
	      || r_type == R_TILEPRO_IMM16_X1_TLS_LE_HI
	      || r_type == R_TILEPRO_IMM16_X0_TLS_LE_HA
	      || r_type == R_TILEPRO_IMM16_X1_TLS_LE_HA)
	    {
	      relocation = tpoff (info, relocation);
	      break;
	    }

	  if (h != NULL)
	    {
	      off = h->got.offset;
	      h->got.offset |= 1;
	    }
	  else
	    {
	      BFD_ASSERT (local_got_offsets != NULL);
	      off = local_got_offsets[r_symndx];
	      local_got_offsets[r_symndx] |= 1;
	    }

	  if (htab->sgot == NULL)
	    abort ();

	  if ((off & 1) != 0)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;
	      int indx = 0;
	      bool need_relocs = false;

	      if (htab->srelgot == NULL)
		abort ();

	      if (h != NULL)
	      {
		bool dyn;
		dyn = htab->dynamic_sections_created;

		if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						     bfd_link_pic (info),
						     h)
		    && (!bfd_link_pic (info)
			|| !SYMBOL_REFERENCES_LOCAL (info, h)))
		  {
		    indx = h->dynindx;
		  }
	      }

	      /* The GOT entries have not been initialized yet.  Do it
		 now, and emit any relocations. */
	      if ((bfd_link_pic (info) || indx != 0)
		  && (h == NULL
		      || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		      || h->root.type != bfd_link_hash_undefweak))
		    need_relocs = true;

	      switch (r_type)
		{
		  case R_TILEPRO_IMM16_X0_TLS_IE:
		  case R_TILEPRO_IMM16_X1_TLS_IE:
		  case R_TILEPRO_IMM16_X0_TLS_IE_LO:
		  case R_TILEPRO_IMM16_X1_TLS_IE_LO:
		  case R_TILEPRO_IMM16_X0_TLS_IE_HI:
		  case R_TILEPRO_IMM16_X1_TLS_IE_HI:
		  case R_TILEPRO_IMM16_X0_TLS_IE_HA:
		  case R_TILEPRO_IMM16_X1_TLS_IE_HA:
		    if (need_relocs) {
		      bfd_put_32 (output_bfd, 0, htab->sgot->contents + off);
		      outrel.r_offset = (htab->sgot->output_section->vma
				       + htab->sgot->output_offset + off);
		      outrel.r_addend = 0;
		      if (indx == 0)
			outrel.r_addend = relocation - dtpoff_base (info);
		      outrel.r_info = ELF32_R_INFO (indx, R_TILEPRO_TLS_TPOFF32);
		      tilepro_elf_append_rela_32 (output_bfd, htab->srelgot,
						  &outrel);
		    } else {
		      bfd_put_32 (output_bfd, tpoff (info, relocation),
				  htab->sgot->contents + off);
		    }
		    break;

		  case R_TILEPRO_IMM16_X0_TLS_GD:
		  case R_TILEPRO_IMM16_X1_TLS_GD:
		  case R_TILEPRO_IMM16_X0_TLS_GD_LO:
		  case R_TILEPRO_IMM16_X1_TLS_GD_LO:
		  case R_TILEPRO_IMM16_X0_TLS_GD_HI:
		  case R_TILEPRO_IMM16_X1_TLS_GD_HI:
		  case R_TILEPRO_IMM16_X0_TLS_GD_HA:
		  case R_TILEPRO_IMM16_X1_TLS_GD_HA:
		    if (need_relocs) {
		      outrel.r_offset = (htab->sgot->output_section->vma
				       + htab->sgot->output_offset + off);
		      outrel.r_addend = 0;
		      outrel.r_info = ELF32_R_INFO (indx, R_TILEPRO_TLS_DTPMOD32);
		      bfd_put_32 (output_bfd, 0, htab->sgot->contents + off);
		      tilepro_elf_append_rela_32 (output_bfd, htab->srelgot,
						  &outrel);
		      if (indx == 0)
			{
			  BFD_ASSERT (! unresolved_reloc);
			  bfd_put_32 (output_bfd,
				      relocation - dtpoff_base (info),
				      (htab->sgot->contents + off +
				       TILEPRO_BYTES_PER_WORD));
			}
		      else
			{
			  bfd_put_32 (output_bfd, 0,
				      (htab->sgot->contents + off +
				       TILEPRO_BYTES_PER_WORD));
			  outrel.r_info = ELF32_R_INFO (indx,
							R_TILEPRO_TLS_DTPOFF32);
			  outrel.r_offset += TILEPRO_BYTES_PER_WORD;
			  tilepro_elf_append_rela_32 (output_bfd,
						      htab->srelgot, &outrel);
			}
		    }

		    else {
		      /* If we are not emitting relocations for a
			 general dynamic reference, then we must be in a
			 static link or an executable link with the
			 symbol binding locally.  Mark it as belonging
			 to module 1, the executable.  */
		      bfd_put_32 (output_bfd, 1,
				  htab->sgot->contents + off );
		      bfd_put_32 (output_bfd, relocation - dtpoff_base (info),
				  htab->sgot->contents + off +
				  TILEPRO_BYTES_PER_WORD);
		   }
		   break;
		}
	    }

	  if (off >= (bfd_vma) -2)
	    abort ();

	  relocation = off - got_base;
	  unresolved_reloc = false;
	  howto = tilepro_elf_howto_table + r_type;
	  break;

	default:
	  break;
	}

      /* Dynamic relocs are not propagated for SEC_DEBUGGING sections
	 because such sections are not SEC_ALLOC and thus ld.so will
	 not process them.  */
      if (unresolved_reloc
	  && !((input_section->flags & SEC_DEBUGGING) != 0
	       && h->def_dynamic)
	  && _bfd_elf_section_offset (output_bfd, info, input_section,
				      rel->r_offset) != (bfd_vma) -1)
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB(%pA+%#" PRIx64 "): "
	     "unresolvable %s relocation against symbol `%s'"),
	   input_bfd,
	   input_section,
	   (uint64_t) rel->r_offset,
	   howto->name,
	   h->root.root.string);

      r = bfd_reloc_continue;

      /* For the _HA types, we add 0x8000 so that if bit 15 is set,
       * we will increment bit 16.  The howto->rightshift takes care
       * of the rest for us. */
      switch (r_type)
      {
      case R_TILEPRO_HA16:
      case R_TILEPRO_IMM16_X0_HA:
      case R_TILEPRO_IMM16_X1_HA:
      case R_TILEPRO_IMM16_X0_HA_PCREL:
      case R_TILEPRO_IMM16_X1_HA_PCREL:
      case R_TILEPRO_IMM16_X0_GOT_HA:
      case R_TILEPRO_IMM16_X1_GOT_HA:
      case R_TILEPRO_IMM16_X0_TLS_GD_HA:
      case R_TILEPRO_IMM16_X1_TLS_GD_HA:
      case R_TILEPRO_IMM16_X0_TLS_IE_HA:
      case R_TILEPRO_IMM16_X1_TLS_IE_HA:
	relocation += 0x8000;
	break;
      }

      /* Get the operand creation function, if any. */
      create_func = reloc_to_create_func[r_type];
      if (create_func == NULL)
      {
	r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				      contents, rel->r_offset,
				      relocation, rel->r_addend);
      }
      else
      {
	if (howto->pc_relative)
	{
	  relocation -=
	    input_section->output_section->vma + input_section->output_offset;
	  if (howto->pcrel_offset)
	    relocation -= rel->r_offset;
	}

	bfd_byte *data;

	/* Add the relocation addend if any to the final target value */
	relocation += rel->r_addend;

	/* Do basic range checking */
	r = bfd_check_overflow (howto->complain_on_overflow,
				howto->bitsize,
				howto->rightshift,
				32,
				relocation);

	/*
	 * Write the relocated value out into the raw section data.
	 * Don't put a relocation out in the .rela section.
	 */
	tilepro_bundle_bits mask = create_func(-1);
	tilepro_bundle_bits value = create_func(relocation >> howto->rightshift);

	/* Only touch bytes while the mask is not 0, so we
	   don't write to out of bounds memory if this is actually
	   a 16-bit switch instruction. */
	for (data = contents + rel->r_offset; mask != 0; data++)
	  {
	    bfd_byte byte_mask = (bfd_byte)mask;
	    *data = (*data & ~byte_mask) | ((bfd_byte)value & byte_mask);
	    mask >>= 8;
	    value >>= 8;
	  }
      }

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

  return true;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
tilepro_elf_finish_dynamic_symbol (bfd *output_bfd,
				   struct bfd_link_info *info,
				   struct elf_link_hash_entry *h,
				   Elf_Internal_Sym *sym)
{
  struct elf_link_hash_table *htab;

  htab = tilepro_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *srela;
      asection *sgotplt;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      bfd_vma r_offset;

      int rela_index;

      /* This symbol has an entry in the PLT.  Set it up.  */

      BFD_ASSERT (h->dynindx != -1);

      splt = htab->splt;
      srela = htab->srelplt;
      sgotplt = htab->sgotplt;

      if (splt == NULL || srela == NULL)
       abort ();

      /* Fill in the entry in the procedure linkage table.  */
      rela_index = tilepro_plt_entry_build (splt, sgotplt, h->plt.offset,
					    &r_offset);

      /* Fill in the entry in the global offset table, which initially points
	 to the beginning of the plt.  */
      bfd_put_32 (output_bfd, splt->output_section->vma + splt->output_offset,
		  sgotplt->contents + r_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgotplt->output_section->vma
		       + sgotplt->output_offset
		       + r_offset);
      rela.r_addend = 0;
      rela.r_info = ELF32_R_INFO (h->dynindx, R_TILEPRO_JMP_SLOT);

      loc = srela->contents + rela_index * sizeof (Elf32_External_Rela);
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

  if (h->got.offset != (bfd_vma) -1
      && tilepro_elf_hash_entry(h)->tls_type != GOT_TLS_GD
      && tilepro_elf_hash_entry(h)->tls_type != GOT_TLS_IE)
    {
      asection *sgot;
      asection *srela;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the GOT.  Set it up.  */

      sgot = htab->sgot;
      srela = htab->srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset
		       + (h->got.offset &~ (bfd_vma) 1));

      /* If this is a -Bsymbolic link, and the symbol is defined
	 locally, we just want to emit a RELATIVE reloc.  Likewise if
	 the symbol was forced to be local because of a version file.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (bfd_link_pic (info)
	  && (info->symbolic || h->dynindx == -1)
	  && h->def_regular)
	{
	  asection *sec = h->root.u.def.section;
	  rela.r_info = ELF32_R_INFO (0, R_TILEPRO_RELATIVE);
	  rela.r_addend = (h->root.u.def.value
			   + sec->output_section->vma
			   + sec->output_offset);
	}
      else
	{
	  rela.r_info = ELF32_R_INFO (h->dynindx, R_TILEPRO_GLOB_DAT);
	  rela.r_addend = 0;
	}

      bfd_put_32 (output_bfd, 0,
			  sgot->contents + (h->got.offset & ~(bfd_vma) 1));
      tilepro_elf_append_rela_32 (output_bfd, srela, &rela);
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;

      /* This symbols needs a copy reloc.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1);

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_TILEPRO_COPY);
      rela.r_addend = 0;
      if (h->root.u.def.section == htab->sdynrelro)
	s = htab->sreldynrelro;
      else
	s = htab->srelbss;
      tilepro_elf_append_rela_32 (output_bfd, s, &rela);
    }

  /* Mark some specially defined symbols as absolute. */
  if (h == htab->hdynamic
      || (h == htab->hgot || h == htab->hplt))
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Finish up the dynamic sections.  */

static bool
tilepro_finish_dyn (bfd *output_bfd, struct bfd_link_info *info,
		    bfd *dynobj, asection *sdyn,
		    asection *splt ATTRIBUTE_UNUSED)
{
  Elf32_External_Dyn *dyncon, *dynconend;
  struct elf_link_hash_table *htab;

  htab = tilepro_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dyncon = (Elf32_External_Dyn *) sdyn->contents;
  dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
  for (; dyncon < dynconend; dyncon++)
    {
      Elf_Internal_Dyn dyn;
      asection *s;

      bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

      switch (dyn.d_tag)
	{
	case DT_PLTGOT:
	  s = htab->sgotplt;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	  break;
	case DT_JMPREL:
	  s = htab->srelplt;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	  break;
	case DT_PLTRELSZ:
	  s = htab->srelplt;
	  dyn.d_un.d_val = s->size;
	  break;
	default:
	  continue;
	}

      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
    }
  return true;
}

static bool
tilepro_elf_finish_dynamic_sections (bfd *output_bfd,
				     struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn;
  struct elf_link_hash_table *htab;

  htab = tilepro_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dynobj = htab->dynobj;

  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      asection *splt;
      bool ret;

      splt = htab->splt;
      BFD_ASSERT (splt != NULL && sdyn != NULL);

      ret = tilepro_finish_dyn (output_bfd, info, dynobj, sdyn, splt);

      if (!ret)
	return ret;

      /* Fill in the first entry in the procedure linkage table.  */
      if (splt->size > 0)
	{
	  memcpy (splt->contents, tilepro_plt0_entry, PLT_HEADER_SIZE);
	  memset (splt->contents + PLT_HEADER_SIZE, 0,
		  PLT_ENTRY_SIZE - PLT_HEADER_SIZE);
	}

      if (elf_section_data (splt->output_section) != NULL)
	elf_section_data (splt->output_section)->this_hdr.sh_entsize
	  = PLT_ENTRY_SIZE;
    }

  if (htab->sgotplt)
    {
      if (bfd_is_abs_section (htab->sgotplt->output_section))
	{
	  _bfd_error_handler
	    (_("discarded output section: `%pA'"), htab->sgotplt);
	  return false;
	}

      if (htab->sgotplt->size > 0)
	{
	  /* Write the first two entries in .got.plt, needed for the dynamic
	     linker.  */
	  bfd_put_32 (output_bfd, (bfd_vma) -1,
		      htab->sgotplt->contents);
	  bfd_put_32 (output_bfd, (bfd_vma) 0,
		      htab->sgotplt->contents + GOT_ENTRY_SIZE);
	}

      elf_section_data (htab->sgotplt->output_section)->this_hdr.sh_entsize
	= GOT_ENTRY_SIZE;
    }

  if (htab->sgot)
    {
      if (htab->sgot->size > 0)
	{
	  /* Set the first entry in the global offset table to the address of
	     the dynamic section.  */
	  bfd_vma val = (sdyn ?
			 sdyn->output_section->vma + sdyn->output_offset :
			 0);
	  bfd_put_32 (output_bfd, val, htab->sgot->contents);
	}

      elf_section_data (htab->sgot->output_section)->this_hdr.sh_entsize
	= GOT_ENTRY_SIZE;
    }

  return true;
}



/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
tilepro_elf_plt_sym_val (bfd_vma i, const asection *plt,
		      const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + (i + 1) * PLT_ENTRY_SIZE;
}

static enum elf_reloc_type_class
tilepro_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			  const asection *rel_sec ATTRIBUTE_UNUSED,
			  const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_TILEPRO_RELATIVE:
      return reloc_class_relative;
    case R_TILEPRO_JMP_SLOT:
      return reloc_class_plt;
    case R_TILEPRO_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

static int
tilepro_additional_program_headers (bfd *abfd,
				    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  /* Each .intrpt section specified by the user adds another PT_LOAD
     header since the sections are discontiguous. */
  static const char intrpt_sections[4][9] =
    {
      ".intrpt0", ".intrpt1", ".intrpt2", ".intrpt3"
    };
  int count = 0;
  int i;

  for (i = 0; i < 4; i++)
    {
      asection *sec = bfd_get_section_by_name (abfd, intrpt_sections[i]);
      if (sec != NULL && (sec->flags & SEC_LOAD) != 0)
	++count;
    }

  /* Add four "padding" headers in to leave room in case a custom linker
     script does something fancy. Otherwise ld complains that it ran
     out of program headers and refuses to link. */
  count += 4;

  return count;
}

#define ELF_ARCH		bfd_arch_tilepro
#define ELF_TARGET_ID		TILEPRO_ELF_DATA
#define ELF_MACHINE_CODE	EM_TILEPRO
#define ELF_MAXPAGESIZE		0x10000
#define ELF_COMMONPAGESIZE	0x10000

#define TARGET_LITTLE_SYM	tilepro_elf32_vec
#define TARGET_LITTLE_NAME	"elf32-tilepro"

#define elf_backend_reloc_type_class	     tilepro_reloc_type_class

#define bfd_elf32_bfd_reloc_name_lookup	     tilepro_reloc_name_lookup
#define bfd_elf32_bfd_link_hash_table_create tilepro_elf_link_hash_table_create
#define bfd_elf32_bfd_reloc_type_lookup	     tilepro_reloc_type_lookup

#define elf_backend_copy_indirect_symbol     tilepro_elf_copy_indirect_symbol
#define elf_backend_create_dynamic_sections  tilepro_elf_create_dynamic_sections
#define elf_backend_check_relocs	     tilepro_elf_check_relocs
#define elf_backend_adjust_dynamic_symbol    tilepro_elf_adjust_dynamic_symbol
#define elf_backend_omit_section_dynsym	     tilepro_elf_omit_section_dynsym
#define elf_backend_size_dynamic_sections    tilepro_elf_size_dynamic_sections
#define elf_backend_relocate_section	     tilepro_elf_relocate_section
#define elf_backend_finish_dynamic_symbol    tilepro_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections  tilepro_elf_finish_dynamic_sections
#define elf_backend_gc_mark_hook	     tilepro_elf_gc_mark_hook
#define elf_backend_plt_sym_val		     tilepro_elf_plt_sym_val
#define elf_info_to_howto_rel		     NULL
#define elf_info_to_howto		     tilepro_info_to_howto_rela
#define elf_backend_grok_prstatus	     tilepro_elf_grok_prstatus
#define elf_backend_grok_psinfo		     tilepro_elf_grok_psinfo
#define elf_backend_additional_program_headers tilepro_additional_program_headers

#define bfd_elf32_mkobject		     tilepro_elf_mkobject

#define elf_backend_init_index_section	_bfd_elf_init_1_index_section

#define elf_backend_can_gc_sections 1
#define elf_backend_can_refcount 1
#define elf_backend_want_got_plt 1
#define elf_backend_plt_readonly 1
/* Align PLT mod 64 byte L2 line size. */
#define elf_backend_plt_alignment 6
#define elf_backend_want_plt_sym 1
#define elf_backend_got_header_size GOT_ENTRY_SIZE
#define elf_backend_want_dynrelro 1
#define elf_backend_rela_normal 1
#define elf_backend_default_execstack 0

#include "elf32-target.h"
