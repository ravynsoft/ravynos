/* TILE-Gx-specific support for ELF.
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
#include "elf/tilegx.h"
#include "opcode/tilegx.h"
#include "libiberty.h"
#include "elfxx-tilegx.h"

#define ABI_64_P(abfd) \
  (get_elf_backend_data (abfd)->s->elfclass == ELFCLASS64)

#define TILEGX_ELF_WORD_BYTES(htab) \
  ((htab)->bytes_per_word)

/* The size of an external RELA relocation.  */
#define TILEGX_ELF_RELA_BYTES(htab) \
  ((htab)->bytes_per_rela)

/* Both 32-bit and 64-bit tilegx encode this in an identical manner,
   so just take advantage of that.  */
#define TILEGX_ELF_R_TYPE(r_info) \
  ((r_info) & 0xFF)

#define TILEGX_ELF_R_INFO(htab, in_rel, index, type)	\
  ((htab)->r_info (in_rel, index, type))

#define TILEGX_ELF_R_SYMNDX(htab, r_info) \
  ((htab)->r_symndx(r_info))

#define TILEGX_ELF_DTPOFF_RELOC(htab) \
  ((htab)->dtpoff_reloc)

#define TILEGX_ELF_DTPMOD_RELOC(htab) \
  ((htab)->dtpmod_reloc)

#define TILEGX_ELF_TPOFF_RELOC(htab) \
  ((htab)->tpoff_reloc)

#define TILEGX_ELF_PUT_WORD(htab, bfd, val, ptr) \
  ((htab)->put_word (bfd, val, ptr))

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF64_DYNAMIC_INTERPRETER "/lib/ld.so.1"
#define ELF32_DYNAMIC_INTERPRETER "/lib32/ld.so.1"


static reloc_howto_type tilegx_elf_howto_table [] =
{
  /* This reloc does nothing.  */
  HOWTO (R_TILEGX_NONE,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */
#ifdef BFD64
  /* A 64 bit absolute relocation.  */
  HOWTO (R_TILEGX_64,	/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_64",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffffffffffffULL,	/* dst_mask */
	 false),		/* pcrel_offset */
#endif
  /* A 32 bit absolute relocation.  */
  HOWTO (R_TILEGX_32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_TILEGX_16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* An 8 bit absolute relocation.  */
  HOWTO (R_TILEGX_8,	/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */
#ifdef BFD64
  /* A 64 bit pc-relative relocation.  */
  HOWTO (R_TILEGX_64_PCREL,/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_32_PCREL", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffffffffffffULL,	/* dst_mask */
	 true),			/* pcrel_offset */
#endif
  /* A 32 bit pc-relative relocation.  */
  HOWTO (R_TILEGX_32_PCREL,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_32_PCREL", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit pc-relative relocation.  */
  HOWTO (R_TILEGX_16_PCREL,/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_16_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* An 8 bit pc-relative relocation.  */
  HOWTO (R_TILEGX_8_PCREL,	/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_8_PCREL",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit relocation without overflow.  */
  HOWTO (R_TILEGX_HW0,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_HW0",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit relocation without overflow.  */
  HOWTO (R_TILEGX_HW1,	/* type */
	 16,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_HW1",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit relocation without overflow.  */
  HOWTO (R_TILEGX_HW2,	/* type */
	 32,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_HW2",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit relocation without overflow.  */
  HOWTO (R_TILEGX_HW3,	/* type */
	 48,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_HW3",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit relocation with overflow.  */
  HOWTO (R_TILEGX_HW0_LAST,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_HW0_LAST",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit relocation with overflow.  */
  HOWTO (R_TILEGX_HW1_LAST,	/* type */
	 16,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_HW1_LAST",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit relocation with overflow.  */
  HOWTO (R_TILEGX_HW2_LAST,	/* type */
	 32,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_HW2_LAST",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_TILEGX_COPY,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_COPY",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEGX_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_GLOB_DAT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEGX_JMP_SLOT,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_JMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEGX_RELATIVE,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_TILEGX_RELATIVE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEGX_BROFF_X1, /* type */
	 TILEGX_LOG2_BUNDLE_ALIGNMENT_IN_BYTES, /* rightshift */
	 4,			/* size */
	 17,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEGX_BROFF_X1", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEGX_JUMPOFF_X1, /* type */
	 TILEGX_LOG2_BUNDLE_ALIGNMENT_IN_BYTES, /* rightshift */
	 4,			/* size */
	 27,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEGX_JUMPOFF_X1", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_TILEGX_JUMPOFF_X1_PLT, /* type */
	 TILEGX_LOG2_BUNDLE_ALIGNMENT_IN_BYTES, /* rightshift */
	 4,			/* size */
	 27,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEGX_JUMPOFF_X1_PLT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

#define TILEGX_IMM_HOWTO(name, size, bitsize) \
  HOWTO (name, 0, size, bitsize, false, 0, \
	 complain_overflow_signed, bfd_elf_generic_reloc, \
	 #name, false, 0, -1, false)

#define TILEGX_UIMM_HOWTO(name, size, bitsize) \
  HOWTO (name, 0, size, bitsize, false, 0, \
	 complain_overflow_unsigned, bfd_elf_generic_reloc, \
	 #name, false, 0, -1, false)

  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_X0, 1, 8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_Y0, 1, 8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_X1, 1, 8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_Y1, 1, 8),
  TILEGX_IMM_HOWTO(R_TILEGX_DEST_IMM8_X1, 1, 8),

  TILEGX_UIMM_HOWTO(R_TILEGX_MT_IMM14_X1, 2, 14),
  TILEGX_UIMM_HOWTO(R_TILEGX_MF_IMM14_X1, 2, 14),

  TILEGX_UIMM_HOWTO(R_TILEGX_MMSTART_X0, 1, 6),
  TILEGX_UIMM_HOWTO(R_TILEGX_MMEND_X0,   1, 6),

  TILEGX_UIMM_HOWTO(R_TILEGX_SHAMT_X0, 1, 6),
  TILEGX_UIMM_HOWTO(R_TILEGX_SHAMT_X1, 1, 6),
  TILEGX_UIMM_HOWTO(R_TILEGX_SHAMT_Y0, 1, 6),
  TILEGX_UIMM_HOWTO(R_TILEGX_SHAMT_Y1, 1, 6),

#define TILEGX_IMM16_HOWTO(name, rshift) \
  HOWTO (name, rshift, 2, 16, false, 0, \
	 complain_overflow_dont, bfd_elf_generic_reloc, \
	 #name, false, 0, 0xffff, false)

  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X0_HW0, 0),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X1_HW0, 0),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X0_HW1, 16),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X1_HW1, 16),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X0_HW2, 32),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X1_HW2, 32),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X0_HW3, 48),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X1_HW3, 48),

#define TILEGX_IMM16_HOWTO_LAST(name, rshift) \
  HOWTO (name, rshift, 2, 16, false, 0, \
	 complain_overflow_signed, bfd_elf_generic_reloc, \
	 #name, false, 0, 0xffff, false)

  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW0_LAST, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW0_LAST, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW1_LAST, 16),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW1_LAST, 16),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW2_LAST, 32),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW2_LAST, 32),

  /* PC-relative offsets. */

#define TILEGX_IMM16_HOWTO_PCREL(name, rshift) \
  HOWTO (name, rshift, 2, 16, true, 0, \
	 complain_overflow_dont, bfd_elf_generic_reloc, \
	 #name, false, 0, 0xffff, true)

  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X0_HW0_PCREL, 0),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X1_HW0_PCREL, 0),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X0_HW1_PCREL, 16),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X1_HW1_PCREL, 16),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X0_HW2_PCREL, 32),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X1_HW2_PCREL, 32),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X0_HW3_PCREL, 48),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X1_HW3_PCREL, 48),

#define TILEGX_IMM16_HOWTO_LAST_PCREL(name, rshift) \
  HOWTO (name, rshift, 2, 16, true, 0, \
	 complain_overflow_signed, bfd_elf_generic_reloc, \
	 #name, false, 0, 0xffff, true)

  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X0_HW0_LAST_PCREL,  0),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X1_HW0_LAST_PCREL,  0),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X0_HW1_LAST_PCREL, 16),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X1_HW1_LAST_PCREL, 16),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X0_HW2_LAST_PCREL, 32),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X1_HW2_LAST_PCREL, 32),

  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X0_HW0_GOT, 0),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X1_HW0_GOT, 0),

  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X0_HW0_PLT_PCREL, 0),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X1_HW0_PLT_PCREL, 0),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X0_HW1_PLT_PCREL, 16),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X1_HW1_PLT_PCREL, 16),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X0_HW2_PLT_PCREL, 32),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X1_HW2_PLT_PCREL, 32),

  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW0_LAST_GOT, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW0_LAST_GOT, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW1_LAST_GOT, 16),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW1_LAST_GOT, 16),

  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X0_HW3_PLT_PCREL, 48),
  TILEGX_IMM16_HOWTO_PCREL (R_TILEGX_IMM16_X1_HW3_PLT_PCREL, 48),

  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X0_HW0_TLS_GD, 0),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X1_HW0_TLS_GD, 0),

  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X0_HW0_TLS_LE, 0),
  TILEGX_IMM16_HOWTO (R_TILEGX_IMM16_X1_HW0_TLS_LE, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE, 16),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE, 16),

  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD, 0),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD, 16),
  TILEGX_IMM16_HOWTO_LAST (R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD, 16),
  EMPTY_HOWTO (90),
  EMPTY_HOWTO (91),

#define TILEGX_IMM16_HOWTO_TLS_IE(name, rshift) \
  HOWTO (name, rshift, 2, 16, false, 0, \
	 complain_overflow_dont, bfd_elf_generic_reloc, \
	 #name, false, 0, 0xffff, true)

  TILEGX_IMM16_HOWTO_TLS_IE (R_TILEGX_IMM16_X0_HW0_TLS_IE, 0),
  TILEGX_IMM16_HOWTO_TLS_IE (R_TILEGX_IMM16_X1_HW0_TLS_IE, 0),

  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL,  0),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL,  0),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL, 16),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL, 16),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL, 32),
  TILEGX_IMM16_HOWTO_LAST_PCREL (R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL, 32),

#define TILEGX_IMM16_HOWTO_LAST_TLS_IE(name, rshift) \
  HOWTO (name, rshift, 2, 16, false, 0, \
	 complain_overflow_signed, bfd_elf_generic_reloc, \
	 #name, false, 0, 0xffff, true)

  TILEGX_IMM16_HOWTO_LAST_TLS_IE (R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE, 0),
  TILEGX_IMM16_HOWTO_LAST_TLS_IE (R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE, 0),
  TILEGX_IMM16_HOWTO_LAST_TLS_IE (R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE, 16),
  TILEGX_IMM16_HOWTO_LAST_TLS_IE (R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE, 16),
  EMPTY_HOWTO (104),
  EMPTY_HOWTO (105),

  HOWTO(R_TILEGX_TLS_DTPMOD64, 0, 0, 0, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_TILEGX_TLS_DTPMOD64",
	false, 0, 0, true),
  HOWTO(R_TILEGX_TLS_DTPOFF64, 0, 8, 64, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_TILEGX_TLS_DTPOFF64",
	false, 0, -1, true),
  HOWTO(R_TILEGX_TLS_TPOFF64, 0, 0, 0, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_TILEGX_TLS_TPOFF64",
	false, 0, 0, true),

  HOWTO(R_TILEGX_TLS_DTPMOD32, 0, 0, 0, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_TILEGX_TLS_DTPMOD32",
	false, 0, 0, true),
  HOWTO(R_TILEGX_TLS_DTPOFF32, 0, 8, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_TILEGX_TLS_DTPOFF32",
	false, 0, -1, true),
  HOWTO(R_TILEGX_TLS_TPOFF32, 0, 0, 0, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_TILEGX_TLS_TPOFF32",
	false, 0, 0, true),

  HOWTO (R_TILEGX_TLS_GD_CALL, /* type */
	 TILEGX_LOG2_BUNDLE_ALIGNMENT_IN_BYTES, /* rightshift */
	 4,			/* size */
	 27,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_TILEGX_TLS_GD_CALL", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 -1,			/* dst_mask */
	 true),			/* pcrel_offset */

  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_X0_TLS_GD_ADD,  1,  8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_X1_TLS_GD_ADD,  1,  8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_Y0_TLS_GD_ADD,  1,  8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_Y1_TLS_GD_ADD,  1,  8),
  TILEGX_IMM_HOWTO(R_TILEGX_TLS_IE_LOAD, 1,  8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_X0_TLS_ADD,  1,  8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_X1_TLS_ADD,  1,  8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_Y0_TLS_ADD,  1,  8),
  TILEGX_IMM_HOWTO(R_TILEGX_IMM8_Y1_TLS_ADD,  1,  8),
};

static reloc_howto_type tilegx_elf_howto_table2 [] =
{
  /* GNU extension to record C++ vtable hierarchy */
  HOWTO (R_TILEGX_GNU_VTINHERIT, /* type */
	 0,			/* rightshift */
	 8,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_TILEGX_GNU_VTINHERIT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable member usage */
  HOWTO (R_TILEGX_GNU_VTENTRY,	   /* type */
	 0,			/* rightshift */
	 8,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn,	/* special_function */
	 "R_TILEGX_GNU_VTENTRY",   /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

};

/* Map BFD reloc types to TILEGX ELF reloc types.  */

typedef struct tilegx_reloc_map
{
  bfd_reloc_code_real_type  bfd_reloc_val;
  unsigned int		    tilegx_reloc_val;
  reloc_howto_type *	    table;
} reloc_map;

static const reloc_map tilegx_reloc_map [] =
{
#define TH_REMAP(bfd, tilegx) \
  { bfd, tilegx, tilegx_elf_howto_table },

  /* Standard relocations. */
  TH_REMAP (BFD_RELOC_NONE,		       R_TILEGX_NONE)
  TH_REMAP (BFD_RELOC_64,		       R_TILEGX_64)
  TH_REMAP (BFD_RELOC_32,		       R_TILEGX_32)
  TH_REMAP (BFD_RELOC_16,		       R_TILEGX_16)
  TH_REMAP (BFD_RELOC_8,		       R_TILEGX_8)
  TH_REMAP (BFD_RELOC_64_PCREL,		       R_TILEGX_64_PCREL)
  TH_REMAP (BFD_RELOC_32_PCREL,		       R_TILEGX_32_PCREL)
  TH_REMAP (BFD_RELOC_16_PCREL,		       R_TILEGX_16_PCREL)
  TH_REMAP (BFD_RELOC_8_PCREL,		       R_TILEGX_8_PCREL)

#define SIMPLE_REMAP(t) TH_REMAP (BFD_RELOC_##t, R_##t)

  /* Custom relocations. */
  SIMPLE_REMAP (TILEGX_HW0)
  SIMPLE_REMAP (TILEGX_HW1)
  SIMPLE_REMAP (TILEGX_HW2)
  SIMPLE_REMAP (TILEGX_HW3)
  SIMPLE_REMAP (TILEGX_HW0_LAST)
  SIMPLE_REMAP (TILEGX_HW1_LAST)
  SIMPLE_REMAP (TILEGX_HW2_LAST)
  SIMPLE_REMAP (TILEGX_COPY)
  SIMPLE_REMAP (TILEGX_GLOB_DAT)
  SIMPLE_REMAP (TILEGX_JMP_SLOT)
  SIMPLE_REMAP (TILEGX_RELATIVE)
  SIMPLE_REMAP (TILEGX_BROFF_X1)
  SIMPLE_REMAP (TILEGX_JUMPOFF_X1)
  SIMPLE_REMAP (TILEGX_JUMPOFF_X1_PLT)
  SIMPLE_REMAP (TILEGX_IMM8_X0)
  SIMPLE_REMAP (TILEGX_IMM8_Y0)
  SIMPLE_REMAP (TILEGX_IMM8_X1)
  SIMPLE_REMAP (TILEGX_IMM8_Y1)
  SIMPLE_REMAP (TILEGX_DEST_IMM8_X1)
  SIMPLE_REMAP (TILEGX_MT_IMM14_X1)
  SIMPLE_REMAP (TILEGX_MF_IMM14_X1)
  SIMPLE_REMAP (TILEGX_MMSTART_X0)
  SIMPLE_REMAP (TILEGX_MMEND_X0)
  SIMPLE_REMAP (TILEGX_SHAMT_X0)
  SIMPLE_REMAP (TILEGX_SHAMT_X1)
  SIMPLE_REMAP (TILEGX_SHAMT_Y0)
  SIMPLE_REMAP (TILEGX_SHAMT_Y1)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW2)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW2)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW3)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW3)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_LAST)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_LAST)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_LAST)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_LAST)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW2_LAST)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW2_LAST)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW2_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW2_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW3_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW3_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_LAST_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_LAST_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_LAST_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_LAST_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW2_LAST_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW2_LAST_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_GOT)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_GOT)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW2_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW2_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_LAST_GOT)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_LAST_GOT)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_LAST_GOT)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_LAST_GOT)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW3_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW3_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_TLS_GD)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_TLS_GD)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_TLS_LE)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_TLS_LE)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_LAST_TLS_LE)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_LAST_TLS_LE)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_LAST_TLS_LE)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_LAST_TLS_LE)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_LAST_TLS_GD)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_LAST_TLS_GD)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_LAST_TLS_GD)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_LAST_TLS_GD)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_TLS_IE)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_TLS_IE)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW0_LAST_TLS_IE)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW0_LAST_TLS_IE)
  SIMPLE_REMAP (TILEGX_IMM16_X0_HW1_LAST_TLS_IE)
  SIMPLE_REMAP (TILEGX_IMM16_X1_HW1_LAST_TLS_IE)

  SIMPLE_REMAP (TILEGX_TLS_DTPMOD64)
  SIMPLE_REMAP (TILEGX_TLS_DTPOFF64)
  SIMPLE_REMAP (TILEGX_TLS_TPOFF64)

  SIMPLE_REMAP (TILEGX_TLS_DTPMOD32)
  SIMPLE_REMAP (TILEGX_TLS_DTPOFF32)
  SIMPLE_REMAP (TILEGX_TLS_TPOFF32)

  SIMPLE_REMAP (TILEGX_TLS_GD_CALL)
  SIMPLE_REMAP (TILEGX_IMM8_X0_TLS_GD_ADD)
  SIMPLE_REMAP (TILEGX_IMM8_X1_TLS_GD_ADD)
  SIMPLE_REMAP (TILEGX_IMM8_Y0_TLS_GD_ADD)
  SIMPLE_REMAP (TILEGX_IMM8_Y1_TLS_GD_ADD)
  SIMPLE_REMAP (TILEGX_TLS_IE_LOAD)
  SIMPLE_REMAP (TILEGX_IMM8_X0_TLS_ADD)
  SIMPLE_REMAP (TILEGX_IMM8_X1_TLS_ADD)
  SIMPLE_REMAP (TILEGX_IMM8_Y0_TLS_ADD)
  SIMPLE_REMAP (TILEGX_IMM8_Y1_TLS_ADD)

#undef SIMPLE_REMAP
#undef TH_REMAP

  { BFD_RELOC_VTABLE_INHERIT,	    R_TILEGX_GNU_VTINHERIT, tilegx_elf_howto_table2 },
  { BFD_RELOC_VTABLE_ENTRY,	    R_TILEGX_GNU_VTENTRY,   tilegx_elf_howto_table2 },
};



/* TILEGX ELF linker hash entry.  */

struct tilegx_elf_link_hash_entry
{
  struct elf_link_hash_entry elf;

#define GOT_UNKNOWN     0
#define GOT_NORMAL      1
#define GOT_TLS_GD      2
#define GOT_TLS_IE      4
  unsigned char tls_type;
};

#define tilegx_elf_hash_entry(ent) \
  ((struct tilegx_elf_link_hash_entry *)(ent))

struct _bfd_tilegx_elf_obj_tdata
{
  struct elf_obj_tdata root;

  /* tls_type for each local got entry.  */
  char *local_got_tls_type;
};

#define _bfd_tilegx_elf_tdata(abfd) \
  ((struct _bfd_tilegx_elf_obj_tdata *) (abfd)->tdata.any)

#define _bfd_tilegx_elf_local_got_tls_type(abfd) \
  (_bfd_tilegx_elf_tdata (abfd)->local_got_tls_type)

#define is_tilegx_elf(bfd)				\
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour	\
   && elf_tdata (bfd) != NULL				\
   && elf_object_id (bfd) == TILEGX_ELF_DATA)

#include "elf/common.h"
#include "elf/internal.h"

struct tilegx_elf_link_hash_table
{
  struct elf_link_hash_table elf;

  int bytes_per_word;
  int word_align_power;
  int bytes_per_rela;
  int dtpmod_reloc;
  int dtpoff_reloc;
  int tpoff_reloc;
  bfd_vma (*r_info) (Elf_Internal_Rela *, bfd_vma, bfd_vma);
  bfd_vma (*r_symndx) (bfd_vma);
  void (*put_word) (bfd *, bfd_vma, void *);
  const char *dynamic_interpreter;

  /* Whether LE transition has been disabled for some of the
     sections.  */
  bool disable_le_transition;
};


/* Get the Tile ELF linker hash table from a link_info structure.  */
#define tilegx_elf_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == TILEGX_ELF_DATA)	\
   ? (struct tilegx_elf_link_hash_table *) (p)->hash : NULL)

#ifdef BFD64
static bfd_vma
tilegx_elf_r_info_64 (Elf_Internal_Rela *in_rel ATTRIBUTE_UNUSED,
		      bfd_vma rel_index,
		      bfd_vma type)
{
  return ELF64_R_INFO (rel_index, type);
}

static bfd_vma
tilegx_elf_r_symndx_64 (bfd_vma r_info)
{
  return ELF64_R_SYM (r_info);
}

static void
tilegx_put_word_64 (bfd *abfd, bfd_vma val, void *ptr)
{
  bfd_put_64 (abfd, val, ptr);
}
#endif /* BFD64 */

static bfd_vma
tilegx_elf_r_info_32 (Elf_Internal_Rela *in_rel ATTRIBUTE_UNUSED,
		      bfd_vma rel_index,
		      bfd_vma type)
{
  return ELF32_R_INFO (rel_index, type);
}

static bfd_vma
tilegx_elf_r_symndx_32 (bfd_vma r_info)
{
  return ELF32_R_SYM (r_info);
}

static void
tilegx_put_word_32 (bfd *abfd, bfd_vma val, void *ptr)
{
  bfd_put_32 (abfd, val, ptr);
}

reloc_howto_type *
tilegx_reloc_type_lookup (bfd * abfd,
			  bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = ARRAY_SIZE (tilegx_reloc_map); i--;)
    {
      const reloc_map * entry;

      entry = tilegx_reloc_map + i;

      if (entry->bfd_reloc_val == code)
	return entry->table + (entry->tilegx_reloc_val
			       - entry->table[0].type);
    }

  /* xgettext:c-format */
  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
		      abfd, (int) code);
  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

reloc_howto_type *
tilegx_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			  const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (tilegx_elf_howto_table)
	    / sizeof (tilegx_elf_howto_table[0]));
       i++)
    if (tilegx_elf_howto_table[i].name != NULL
	&& strcasecmp (tilegx_elf_howto_table[i].name, r_name) == 0)
      return &tilegx_elf_howto_table[i];

  return NULL;
}

bool
tilegx_info_to_howto_rela (bfd *abfd ATTRIBUTE_UNUSED,
			   arelent *cache_ptr,
			   Elf_Internal_Rela *dst)
{
  unsigned int r_type = TILEGX_ELF_R_TYPE (dst->r_info);

  if (r_type <= (unsigned int) R_TILEGX_IMM8_Y1_TLS_ADD)
    cache_ptr->howto = &tilegx_elf_howto_table [r_type];
  else if (r_type - R_TILEGX_GNU_VTINHERIT
	   <= ((unsigned int) R_TILEGX_GNU_VTENTRY
	       - (unsigned int) R_TILEGX_GNU_VTINHERIT))
    cache_ptr->howto
      = &tilegx_elf_howto_table2 [r_type - R_TILEGX_GNU_VTINHERIT];
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

typedef tilegx_bundle_bits (*tilegx_create_func)(int);

static const tilegx_create_func reloc_to_create_func[] =
{
  /* The first twenty relocation types don't correspond to operands */
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
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  /* The remaining relocations are used for immediate operands */
  create_BrOff_X1,
  create_JumpOff_X1,
  create_JumpOff_X1,
  create_Imm8_X0,
  create_Imm8_Y0,
  create_Imm8_X1,
  create_Imm8_Y1,
  create_Dest_Imm8_X1,
  create_MT_Imm14_X1,
  create_MF_Imm14_X1,
  create_BFStart_X0,
  create_BFEnd_X0,
  create_ShAmt_X0,
  create_ShAmt_X1,
  create_ShAmt_Y0,
  create_ShAmt_Y1,
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
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
  create_Imm16_X0,
  create_Imm16_X1,
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
};

static void
tilegx_elf_append_rela (bfd *abfd, asection *s, Elf_Internal_Rela *rel)
{
  const struct elf_backend_data *bed;
  bfd_byte *loc;

  bed = get_elf_backend_data (abfd);
  loc = s->contents + (s->reloc_count++ * bed->s->sizeof_rela);
  bed->s->swap_reloca_out (abfd, rel, loc);
}

/* PLT/GOT stuff */

/* The procedure linkage table starts with the following header:

     ld_add	  r28, r27, 8
     ld		  r27, r27
   {
     jr		  r27
     info	  10		## SP not offset, return PC in LR
   }

   Subsequent entries are the following, jumping to the header at the end:

   {
     moveli	  r28, <_GLOBAL_OFFSET_TABLE_ - 1f + MY_GOT_OFFSET>
     lnk	  r26
   }
1:
   {
     moveli	  r27, <_GLOBAL_OFFSET_TABLE_ - 1b>
     shl16insli	  r28, r28, <_GLOBAL_OFFSET_TABLE_ - 1b + MY_GOT_OFFSET>
   }
   {
     add	  r28, r26, r28
     shl16insli	  r27, r27, <_GLOBAL_OFFSET_TABLE_ - 1b>
   }
   {
     add	  r27, r26, r27
     ld		  r28, r28
     info	  10	   ## SP not offset, return PC in LR
   }
   {
     shl16insli	  r29, zero, MY_PLT_INDEX
     jr		  r28
   }

   This code sequence lets the code at at the start of the PLT determine
   which PLT entry was executed by examining 'r29'.

   Note that MY_PLT_INDEX skips over the header entries, so the first
   actual jump table entry has index zero.

   If the offset fits in 16 bits,

     lnk	  r26
1:
   {
     addli	  r28, r26, <_GLOBAL_OFFSET_TABLE_ - 1b + MY_GOT_OFFSET>
     moveli	  r27, <_GLOBAL_OFFSET_TABLE_ - 1b>
   }
   {
     shl16insli	  r29, zero, MY_PLT_INDEX
     ld		  r28, r28
   }
   {
     add	  r27, r26, r27
     jr		  r28
   }
     info	  10	   ## SP not offset, return PC in LR

   For the purpose of backtracing, the procedure linkage table ends with the
   following tail entry:

     info	  10	   ## SP not offset, return PC in LR

   The 32-bit versions are similar, with ld4s replacing ld, and offsets into
   the GOT being multiples of 4 instead of 8.

*/

#define PLT_HEADER_SIZE_IN_BUNDLES 3
#define PLT_ENTRY_SIZE_IN_BUNDLES 5
#define PLT_TAIL_SIZE_IN_BUNDLES 1

#define PLT_HEADER_SIZE \
  (PLT_HEADER_SIZE_IN_BUNDLES * TILEGX_BUNDLE_SIZE_IN_BYTES)
#define PLT_ENTRY_SIZE \
  (PLT_ENTRY_SIZE_IN_BUNDLES * TILEGX_BUNDLE_SIZE_IN_BYTES)
#define PLT_TAIL_SIZE \
  (PLT_TAIL_SIZE_IN_BUNDLES * TILEGX_BUNDLE_SIZE_IN_BYTES)

#define GOT_ENTRY_SIZE(htab) TILEGX_ELF_WORD_BYTES (htab)

#define GOTPLT_HEADER_SIZE(htab) (2 * GOT_ENTRY_SIZE (htab))

static const bfd_byte
tilegx64_plt0_entry[PLT_HEADER_SIZE] =
{
  0x00, 0x30, 0x48, 0x51,
  0x6e, 0x43, 0xa0, 0x18, /* { ld_add r28, r27, 8 } */
  0x00, 0x30, 0xbc, 0x35,
  0x00, 0x40, 0xde, 0x9e, /* { ld r27, r27 } */
  0xff, 0xaf, 0x30, 0x40,
  0x60, 0x73, 0x6a, 0x28, /* { info 10 ; jr r27 } */
};

static const bfd_byte
tilegx64_long_plt_entry[PLT_ENTRY_SIZE] =
{
  0xdc, 0x0f, 0x00, 0x10,
  0x0d, 0xf0, 0x6a, 0x28, /* { moveli r28, 0 ; lnk r26 } */
  0xdb, 0x0f, 0x00, 0x10,
  0x8e, 0x03, 0x00, 0x38, /* { moveli r27, 0 ; shl16insli r28, r28, 0 } */
  0x9c, 0xc6, 0x0d, 0xd0,
  0x6d, 0x03, 0x00, 0x38, /* { add r28, r26, r28 ; shl16insli r27, r27, 0 } */
  0x9b, 0xb6, 0xc5, 0xad,
  0xff, 0x57, 0xe0, 0x8e, /* { add r27, r26, r27 ; info 10 ; ld r28, r28 } */
  0xdd, 0x0f, 0x00, 0x70,
  0x80, 0x73, 0x6a, 0x28, /* { shl16insli r29, zero, 0 ; jr r28 } */
};

static const bfd_byte
tilegx64_short_plt_entry[PLT_ENTRY_SIZE] =
{
  0x00, 0x30, 0x48, 0x51,
  0x0d, 0xf0, 0x6a, 0x28, /* { lnk r26 } */
  0x9c, 0x06, 0x00, 0x90,
  0xed, 0x07, 0x00, 0x00, /* { addli r28, r26, 0 ; moveli r27, 0 } */
  0xdd, 0x0f, 0x00, 0x70,
  0x8e, 0xeb, 0x6a, 0x28, /* { shl16insli r29, zero, 0 ; ld r28, r28 } */
  0x9b, 0xb6, 0x0d, 0x50,
  0x80, 0x73, 0x6a, 0x28, /* { add r27, r26, r27 ; jr r28 } */
  0x00, 0x30, 0x48, 0xd1,
  0xff, 0x57, 0x18, 0x18, /* { info 10 } */
};

/* Reuse an existing info 10 bundle.  */
static const bfd_byte *const tilegx64_plt_tail_entry =
  &tilegx64_short_plt_entry[4 * TILEGX_BUNDLE_SIZE_IN_BYTES];

static const bfd_byte
tilegx32_plt0_entry[PLT_HEADER_SIZE] =
{
  0x00, 0x30, 0x48, 0x51,
  0x6e, 0x23, 0x58, 0x18, /* { ld4s_add r28, r27, 4 } */
  0x00, 0x30, 0xbc, 0x35,
  0x00, 0x40, 0xde, 0x9c, /* { ld4s r27, r27 } */
  0xff, 0xaf, 0x30, 0x40,
  0x60, 0x73, 0x6a, 0x28, /* { info 10 ; jr r27 } */
};

static const bfd_byte
tilegx32_long_plt_entry[PLT_ENTRY_SIZE] =
{
  0xdc, 0x0f, 0x00, 0x10,
  0x0d, 0xf0, 0x6a, 0x28, /* { moveli r28, 0 ; lnk r26 } */
  0xdb, 0x0f, 0x00, 0x10,
  0x8e, 0x03, 0x00, 0x38, /* { moveli r27, 0 ; shl16insli r28, r28, 0 } */
  0x9c, 0xc6, 0x0d, 0xd0,
  0x6d, 0x03, 0x00, 0x38, /* { add r28, r26, r28 ; shl16insli r27, r27, 0 } */
  0x9b, 0xb6, 0xc5, 0xad,
  0xff, 0x57, 0xe0, 0x8c, /* { add r27, r26, r27 ; info 10 ; ld4s r28, r28 } */
  0xdd, 0x0f, 0x00, 0x70,
  0x80, 0x73, 0x6a, 0x28, /* { shl16insli r29, zero, 0 ; jr r28 } */
};

static const bfd_byte
tilegx32_short_plt_entry[PLT_ENTRY_SIZE] =
{
  0x00, 0x30, 0x48, 0x51,
  0x0d, 0xf0, 0x6a, 0x28, /* { lnk r26 } */
  0x9c, 0x06, 0x00, 0x90,
  0xed, 0x07, 0x00, 0x00, /* { addli r28, r26, 0 ; moveli r27, 0 } */
  0xdd, 0x0f, 0x00, 0x70,
  0x8e, 0x9b, 0x6a, 0x28, /* { shl16insli r29, zero, 0 ; ld4s r28, r28 } */
  0x9b, 0xb6, 0x0d, 0x50,
  0x80, 0x73, 0x6a, 0x28, /* { add r27, r26, r27 ; jr r28 } */
  0x00, 0x30, 0x48, 0xd1,
  0xff, 0x57, 0x18, 0x18, /* { info 10 } */
};

/* Reuse an existing info 10 bundle.  */
static const bfd_byte *const tilegx32_plt_tail_entry =
  &tilegx64_short_plt_entry[4 * TILEGX_BUNDLE_SIZE_IN_BYTES];

static int
tilegx_plt_entry_build (bfd *output_bfd,
			struct tilegx_elf_link_hash_table *htab,
			asection *splt, asection *sgotplt,
			bfd_vma offset, bfd_vma *r_offset)
{
  int plt_index = (offset - PLT_HEADER_SIZE) / PLT_ENTRY_SIZE;
  int got_offset = (plt_index * GOT_ENTRY_SIZE (htab)
		    + GOTPLT_HEADER_SIZE (htab));
  tilegx_bundle_bits *pc;

  /* Compute the distance from the got entry to the lnk.  */
  bfd_signed_vma dist_got_entry = sgotplt->output_section->vma
    + sgotplt->output_offset
    + got_offset
    - splt->output_section->vma
    - splt->output_offset
    - offset
    - TILEGX_BUNDLE_SIZE_IN_BYTES;

  /* Compute the distance to GOTPLT[0].  */
  bfd_signed_vma dist_got0 = dist_got_entry - got_offset;

  /* Check whether we can use the short plt entry with 16-bit offset.  */
  bool short_plt_entry =
    (dist_got_entry <= 0x7fff && dist_got0 >= -0x8000);

  const tilegx_bundle_bits *plt_entry = (tilegx_bundle_bits *)
    (ABI_64_P (output_bfd) ?
     (short_plt_entry ? tilegx64_short_plt_entry : tilegx64_long_plt_entry) :
     (short_plt_entry ? tilegx32_short_plt_entry : tilegx32_long_plt_entry));

  /* Copy the plt entry template.  */
  memcpy (splt->contents + offset, plt_entry, PLT_ENTRY_SIZE);

  /* Write the immediate offsets.  */
  pc = (tilegx_bundle_bits *)(splt->contents + offset);

  if (short_plt_entry)
    {
      /* { lnk r28 }  */
      pc++;

      /* { addli r28, r28, &GOTPLT[MY_GOT_INDEX] ; moveli r27, &GOTPLT[0] }  */
      *pc++ |= create_Imm16_X0 (dist_got_entry)
	| create_Imm16_X1 (dist_got0);

      /* { shl16insli r29, zero, MY_PLT_INDEX ; ld r28, r28 }  */
      *pc++ |= create_Imm16_X0 (plt_index);
    }
  else
    {
      /* { moveli r28, &GOTPLT[MY_GOT_INDEX] ; lnk r26 }  */
      *pc++ |= create_Imm16_X0 (dist_got_entry >> 16);

      /* { moveli r27, &GOTPLT[0] ;
	   shl16insli r28, r28, &GOTPLT[MY_GOT_INDEX] }  */
      *pc++ |= create_Imm16_X0 (dist_got0 >> 16)
	| create_Imm16_X1 (dist_got_entry);

      /* { add r28, r26, r28 ; shl16insli r27, r27, &GOTPLT[0] }  */
      *pc++ |= create_Imm16_X1 (dist_got0);

      /* { add r27, r26, r27 ; info 10 ; ld r28, r28 } */
      pc++;

      /* { shl16insli r29, zero, MY_GOT_INDEX ; jr r28 } */
      *pc++ |= create_Imm16_X0 (plt_index);
   }

  /* Set the relocation offset.  */
  *r_offset = got_offset;

  return plt_index;
}

/* Create an entry in an TILEGX ELF linker hash table.  */

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
			   sizeof (struct tilegx_elf_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct tilegx_elf_link_hash_entry *eh;

      eh = (struct tilegx_elf_link_hash_entry *) entry;
      eh->tls_type = GOT_UNKNOWN;
    }

  return entry;
}

/* Create a TILEGX ELF linker hash table.  */

struct bfd_link_hash_table *
tilegx_elf_link_hash_table_create (bfd *abfd)
{
  struct tilegx_elf_link_hash_table *ret;
  size_t amt = sizeof (struct tilegx_elf_link_hash_table);

  ret = (struct tilegx_elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

#ifdef BFD64
  if (ABI_64_P (abfd))
    {
      ret->bytes_per_word = 8;
      ret->word_align_power = 3;
      ret->bytes_per_rela = sizeof (Elf64_External_Rela);
      ret->dtpoff_reloc = R_TILEGX_TLS_DTPOFF64;
      ret->dtpmod_reloc = R_TILEGX_TLS_DTPMOD64;
      ret->tpoff_reloc = R_TILEGX_TLS_TPOFF64;
      ret->r_info = tilegx_elf_r_info_64;
      ret->r_symndx = tilegx_elf_r_symndx_64;
      ret->dynamic_interpreter = ELF64_DYNAMIC_INTERPRETER;
      ret->put_word = tilegx_put_word_64;
    }
  else
#endif
    {
      ret->bytes_per_word = 4;
      ret->word_align_power = 2;
      ret->bytes_per_rela = sizeof (Elf32_External_Rela);
      ret->dtpoff_reloc = R_TILEGX_TLS_DTPOFF32;
      ret->dtpmod_reloc = R_TILEGX_TLS_DTPMOD32;
      ret->tpoff_reloc = R_TILEGX_TLS_TPOFF32;
      ret->r_info = tilegx_elf_r_info_32;
      ret->r_symndx = tilegx_elf_r_symndx_32;
      ret->dynamic_interpreter = ELF32_DYNAMIC_INTERPRETER;
      ret->put_word = tilegx_put_word_32;
    }

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd, link_hash_newfunc,
				      sizeof (struct tilegx_elf_link_hash_entry),
				      TILEGX_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->elf.root;
}

/* Create the .got section.  */

static bool
tilegx_elf_create_got_section (bfd *abfd, struct bfd_link_info *info)
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

  s = bfd_make_section_anyway_with_flags (abfd,
					  (bed->rela_plts_and_copies_p
					   ? ".rela.got" : ".rel.got"),
					  (bed->dynamic_sec_flags
					   | SEC_READONLY));
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->srelgot = s;

  s = s_got = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->sgot = s;

  /* The first bit of the global offset table is the header.  */
  s->size += bed->got_header_size;

  if (bed->want_got_plt)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".got.plt", flags);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->sgotplt = s;

      /* Reserve room for the header.  */
      s->size += GOTPLT_HEADER_SIZE (tilegx_elf_hash_table (info));
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

bool
tilegx_elf_create_dynamic_sections (bfd *dynobj,
				    struct bfd_link_info *info)
{
  if (!tilegx_elf_create_got_section (dynobj, info))
    return false;

  return _bfd_elf_create_dynamic_sections (dynobj, info);
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

void
tilegx_elf_copy_indirect_symbol (struct bfd_link_info *info,
				 struct elf_link_hash_entry *dir,
				 struct elf_link_hash_entry *ind)
{
  struct tilegx_elf_link_hash_entry *edir, *eind;

  edir = (struct tilegx_elf_link_hash_entry *) dir;
  eind = (struct tilegx_elf_link_hash_entry *) ind;

  if (ind->root.type == bfd_link_hash_indirect
      && dir->got.refcount <= 0)
    {
      edir->tls_type = eind->tls_type;
      eind->tls_type = GOT_UNKNOWN;
    }
  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

static int
tilegx_tls_translate_to_le (int r_type)
{
  switch (r_type)
    {
    case R_TILEGX_IMM16_X0_HW0_TLS_GD:
    case R_TILEGX_IMM16_X0_HW0_TLS_IE:
      return R_TILEGX_IMM16_X0_HW0_TLS_LE;

    case R_TILEGX_IMM16_X1_HW0_TLS_GD:
    case R_TILEGX_IMM16_X1_HW0_TLS_IE:
      return R_TILEGX_IMM16_X1_HW0_TLS_LE;

    case R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
    case R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
      return R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE;

    case R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
    case R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
      return R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE;

    case R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
    case R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
      return R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE;

    case R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
    case R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
      return R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE;
    }
  return r_type;
}

static int
tilegx_tls_translate_to_ie (int r_type)
{
  switch (r_type)
    {
    case R_TILEGX_IMM16_X0_HW0_TLS_GD:
    case R_TILEGX_IMM16_X0_HW0_TLS_IE:
      return R_TILEGX_IMM16_X0_HW0_TLS_IE;

    case R_TILEGX_IMM16_X1_HW0_TLS_GD:
    case R_TILEGX_IMM16_X1_HW0_TLS_IE:
      return R_TILEGX_IMM16_X1_HW0_TLS_IE;

    case R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
    case R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
      return R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE;

    case R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
    case R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
      return R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE;

    case R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
    case R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
      return R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE;

    case R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
    case R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
      return R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE;
    }
  return r_type;
}

static int
tilegx_elf_tls_transition (struct bfd_link_info *info, int r_type,
			   int is_local, bool disable_le_transition)
{
  if (!bfd_link_executable (info))
    return r_type;

  if (is_local && !disable_le_transition)
    return tilegx_tls_translate_to_le (r_type);
  else
    return tilegx_tls_translate_to_ie (r_type);
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */

bool
tilegx_elf_check_relocs (bfd *abfd, struct bfd_link_info *info,
			 asection *sec, const Elf_Internal_Rela *relocs)
{
  struct tilegx_elf_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;
  int num_relocs;
  bool has_tls_gd_or_ie = false, has_tls_add = false;

  if (bfd_link_relocatable (info))
    return true;

  htab = tilegx_elf_hash_table (info);
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  sreloc = NULL;

  num_relocs = sec->reloc_count;

  BFD_ASSERT (is_tilegx_elf (abfd) || num_relocs == 0);

  if (htab->elf.dynobj == NULL)
    htab->elf.dynobj = abfd;

  rel_end = relocs + num_relocs;

  /* Check whether to do optimization to transform TLS GD/IE
     referehces to TLS LE.  We disable it if we're linking with old
     TLS code sequences that do not support such optimization.  Old
     TLS code sequences have tls_gd_call/tls_ie_load relocations but
     no tls_add relocations.  */
  for (rel = relocs; rel < rel_end && !has_tls_add; rel++)
    {
      int r_type = TILEGX_ELF_R_TYPE (rel->r_info);
      switch (r_type)
	{
	case R_TILEGX_TLS_GD_CALL:
	case R_TILEGX_TLS_IE_LOAD:
	  has_tls_gd_or_ie = true;
	  break;
	case R_TILEGX_IMM8_X0_TLS_ADD:
	case R_TILEGX_IMM8_Y0_TLS_ADD:
	case R_TILEGX_IMM8_X1_TLS_ADD:
	case R_TILEGX_IMM8_Y1_TLS_ADD:
	  has_tls_add = true;
	  break;
	}
    }

  sec->sec_flg0 = (has_tls_gd_or_ie && !has_tls_add);
  htab->disable_le_transition |= sec->sec_flg0;

  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      unsigned int r_symndx;
      struct elf_link_hash_entry *h;
      int tls_type;

      r_symndx = TILEGX_ELF_R_SYMNDX (htab, rel->r_info);
      r_type = TILEGX_ELF_R_TYPE (rel->r_info);

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

      r_type = tilegx_elf_tls_transition (info, r_type, h == NULL,
					  sec->sec_flg0);
      switch (r_type)
	{
	case R_TILEGX_IMM16_X0_HW0_TLS_LE:
	case R_TILEGX_IMM16_X1_HW0_TLS_LE:
	case R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
	case R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
	case R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
	case R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
	  if (!bfd_link_executable (info))
	    goto r_tilegx_plt32;
	  break;

	case R_TILEGX_IMM16_X0_HW0_TLS_GD:
	case R_TILEGX_IMM16_X1_HW0_TLS_GD:
	case R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
	case R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
	case R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
	case R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
	  BFD_ASSERT (bfd_link_pic (info));
	  tls_type = GOT_TLS_GD;
	  goto have_got_reference;

	case R_TILEGX_IMM16_X0_HW0_TLS_IE:
	case R_TILEGX_IMM16_X1_HW0_TLS_IE:
	case R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
	case R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
	case R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
	case R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
	  tls_type = GOT_TLS_IE;
	  if (!bfd_link_executable (info))
	    info->flags |= DF_STATIC_TLS;
	  goto have_got_reference;

	case R_TILEGX_IMM16_X0_HW0_GOT:
	case R_TILEGX_IMM16_X1_HW0_GOT:
	case R_TILEGX_IMM16_X0_HW0_LAST_GOT:
	case R_TILEGX_IMM16_X1_HW0_LAST_GOT:
	case R_TILEGX_IMM16_X0_HW1_LAST_GOT:
	case R_TILEGX_IMM16_X1_HW1_LAST_GOT:
	  tls_type = GOT_NORMAL;
	  /* Fall Through */

	have_got_reference:
	  /* This symbol requires a global offset table entry.  */
	  {
	    int old_tls_type;

	    if (h != NULL)
	      {
		h->got.refcount += 1;
		old_tls_type = tilegx_elf_hash_entry(h)->tls_type;
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
		    _bfd_tilegx_elf_local_got_tls_type (abfd)
		      = (char *) (local_got_refcounts + symtab_hdr->sh_info);
		  }
		local_got_refcounts[r_symndx] += 1;
		old_tls_type = _bfd_tilegx_elf_local_got_tls_type (abfd) [r_symndx];
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
		  tilegx_elf_hash_entry (h)->tls_type = tls_type;
		else
		  _bfd_tilegx_elf_local_got_tls_type (abfd) [r_symndx] = tls_type;
	      }
	  }

	  if (htab->elf.sgot == NULL)
	    {
	      if (!tilegx_elf_create_got_section (htab->elf.dynobj, info))
		return false;
	    }
	  break;

	case R_TILEGX_TLS_GD_CALL:
	  if (!bfd_link_executable (info))
	    {
	      /* These are basically R_TILEGX_JUMPOFF_X1_PLT relocs
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

	case R_TILEGX_JUMPOFF_X1_PLT:
	case R_TILEGX_IMM16_X0_HW0_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW0_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW1_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW1_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW2_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW2_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW3_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW3_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL:
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

	case R_TILEGX_64_PCREL:
	case R_TILEGX_32_PCREL:
	case R_TILEGX_16_PCREL:
	case R_TILEGX_8_PCREL:
	case R_TILEGX_IMM16_X0_HW0_PCREL:
	case R_TILEGX_IMM16_X1_HW0_PCREL:
	case R_TILEGX_IMM16_X0_HW1_PCREL:
	case R_TILEGX_IMM16_X1_HW1_PCREL:
	case R_TILEGX_IMM16_X0_HW2_PCREL:
	case R_TILEGX_IMM16_X1_HW2_PCREL:
	case R_TILEGX_IMM16_X0_HW3_PCREL:
	case R_TILEGX_IMM16_X1_HW3_PCREL:
	case R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
	case R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
	case R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
	case R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
	case R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
	case R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
	  if (h != NULL)
	    h->non_got_ref = 1;

	  if (h != NULL
	      && strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
	    break;
	  /* Fall through.  */

	case R_TILEGX_64:
	case R_TILEGX_32:
	case R_TILEGX_16:
	case R_TILEGX_8:
	case R_TILEGX_HW0:
	case R_TILEGX_HW1:
	case R_TILEGX_HW2:
	case R_TILEGX_HW3:
	case R_TILEGX_HW0_LAST:
	case R_TILEGX_HW1_LAST:
	case R_TILEGX_HW2_LAST:
	case R_TILEGX_COPY:
	case R_TILEGX_GLOB_DAT:
	case R_TILEGX_JMP_SLOT:
	case R_TILEGX_RELATIVE:
	case R_TILEGX_BROFF_X1:
	case R_TILEGX_JUMPOFF_X1:
	case R_TILEGX_IMM8_X0:
	case R_TILEGX_IMM8_Y0:
	case R_TILEGX_IMM8_X1:
	case R_TILEGX_IMM8_Y1:
	case R_TILEGX_DEST_IMM8_X1:
	case R_TILEGX_MT_IMM14_X1:
	case R_TILEGX_MF_IMM14_X1:
	case R_TILEGX_MMSTART_X0:
	case R_TILEGX_MMEND_X0:
	case R_TILEGX_SHAMT_X0:
	case R_TILEGX_SHAMT_X1:
	case R_TILEGX_SHAMT_Y0:
	case R_TILEGX_SHAMT_Y1:
	case R_TILEGX_IMM16_X0_HW0:
	case R_TILEGX_IMM16_X1_HW0:
	case R_TILEGX_IMM16_X0_HW1:
	case R_TILEGX_IMM16_X1_HW1:
	case R_TILEGX_IMM16_X0_HW2:
	case R_TILEGX_IMM16_X1_HW2:
	case R_TILEGX_IMM16_X0_HW3:
	case R_TILEGX_IMM16_X1_HW3:
	case R_TILEGX_IMM16_X0_HW0_LAST:
	case R_TILEGX_IMM16_X1_HW0_LAST:
	case R_TILEGX_IMM16_X0_HW1_LAST:
	case R_TILEGX_IMM16_X1_HW1_LAST:
	case R_TILEGX_IMM16_X0_HW2_LAST:
	case R_TILEGX_IMM16_X1_HW2_LAST:
	  if (h != NULL)
	    h->non_got_ref = 1;

	r_tilegx_plt32:
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
	       && (! tilegx_elf_howto_table[r_type].pc_relative
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
		    (sec, htab->elf.dynobj, htab->word_align_power, abfd,
		     /*rela?*/ true);

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

		  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
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
		       bfd_alloc (htab->elf.dynobj, amt));
		  if (p == NULL)
		    return false;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;
	      if (tilegx_elf_howto_table[r_type].pc_relative)
		p->pc_count += 1;
	    }

	  break;

	case R_TILEGX_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	case R_TILEGX_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	default:
	  break;
	}
    }

  return true;
}


asection *
tilegx_elf_gc_mark_hook (asection *sec,
			 struct bfd_link_info *info,
			 Elf_Internal_Rela *rel,
			 struct elf_link_hash_entry *h,
			 Elf_Internal_Sym *sym)
{
  if (h != NULL)
    {
      switch (TILEGX_ELF_R_TYPE (rel->r_info))
	{
	case R_TILEGX_GNU_VTINHERIT:
	case R_TILEGX_GNU_VTENTRY:
	  return NULL;
	}
    }

  /* FIXME: The test here, in check_relocs and in relocate_section
     dealing with TLS optimization, ought to be !bfd_link_executable (info).  */
  if (bfd_link_pic (info))
    {
      struct bfd_link_hash_entry *bh;

      switch (TILEGX_ELF_R_TYPE (rel->r_info))
	{
	case R_TILEGX_TLS_GD_CALL:
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

bool
tilegx_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				  struct elf_link_hash_entry *h)
{
  struct tilegx_elf_link_hash_table *htab;
  bfd *dynobj;
  asection *s, *srel;

  htab = tilegx_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  dynobj = htab->elf.dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
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
	  /* This case can occur if we saw a R_TILEGX_JUMPOFF_X1_PLT
	     reloc in an input file, but the symbol was never referred
	     to by a dynamic object, or if all references were garbage
	     collected.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a
	     R_TILEGX_JUMPOFF_X1 relocation instead. */
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

  /* We must generate a R_TILEGX_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rel.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = htab->elf.sdynrelro;
      srel = htab->elf.sreldynrelro;
    }
  else
    {
      s = htab->elf.sdynbss;
      srel = htab->elf.srelbss;
    }
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      srel->size += TILEGX_ELF_RELA_BYTES (htab);
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
  struct tilegx_elf_link_hash_table *htab;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = tilegx_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (htab->elf.dynamic_sections_created
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
	  asection *s = htab->elf.splt;

	  /* Allocate room for the header and tail.  */
	  if (s->size == 0)
	    {
	      s->size = PLT_ENTRY_SIZE;
	    }

	  h->plt.offset = s->size - PLT_ENTRY_SIZE + PLT_HEADER_SIZE;

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
	  htab->elf.sgotplt->size += GOT_ENTRY_SIZE (htab);

	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->elf.srelplt->size += TILEGX_ELF_RELA_BYTES (htab);
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
      && !htab->disable_le_transition
      && bfd_link_executable (info)
      && h->dynindx == -1
      && tilegx_elf_hash_entry(h)->tls_type == GOT_TLS_IE)
    h->got.offset = (bfd_vma) -1;
  else if (h->got.refcount > 0)
    {
      asection *s;
      bool dyn;
      int tls_type = tilegx_elf_hash_entry(h)->tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      s = htab->elf.sgot;
      h->got.offset = s->size;
      s->size += TILEGX_ELF_WORD_BYTES (htab);
      /* TLS_GD entries need 2 consecutive GOT slots. */
      if (tls_type == GOT_TLS_GD)
	s->size += TILEGX_ELF_WORD_BYTES (htab);
      dyn = htab->elf.dynamic_sections_created;
      /* TLS_IE needs one dynamic relocation,
	 TLS_GD needs two if local symbol and two if global.  */
      if (tls_type == GOT_TLS_GD || tls_type == GOT_TLS_IE)
	htab->elf.srelgot->size += 2 * TILEGX_ELF_RELA_BYTES (htab);
      else if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						bfd_link_pic (info),
						h))
	htab->elf.srelgot->size += TILEGX_ELF_RELA_BYTES (htab);
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
	      || (htab->elf.dynamic_sections_created
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
      sreloc->size += p->count * TILEGX_ELF_RELA_BYTES (htab);
    }

  return true;
}

/* Return true if the dynamic symbol for a given section should be
   omitted when creating a shared library.  */

bool
tilegx_elf_omit_section_dynsym (bfd *output_bfd,
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

bool
tilegx_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				  struct bfd_link_info *info)
{
  struct tilegx_elf_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd *ibfd;

  htab = tilegx_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dynobj = htab->elf.dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = strlen (htab->dynamic_interpreter) + 1;
	  s->contents = (unsigned char *) htab->dynamic_interpreter;
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

      if (! is_tilegx_elf (ibfd))
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
		  srel->size += p->count * TILEGX_ELF_RELA_BYTES (htab);
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
      local_tls_type = _bfd_tilegx_elf_local_got_tls_type (ibfd);
      s = htab->elf.sgot;
      srel = htab->elf.srelgot;
      for (; local_got < end_local_got; ++local_got, ++local_tls_type)
	{
	  if (*local_got > 0)
	    {
	      *local_got = s->size;
	      s->size += TILEGX_ELF_WORD_BYTES (htab);
	      if (*local_tls_type == GOT_TLS_GD)
		s->size += TILEGX_ELF_WORD_BYTES (htab);
	      if (bfd_link_pic (info)
		  || *local_tls_type == GOT_TLS_GD
		  || *local_tls_type == GOT_TLS_IE)
		srel->size += TILEGX_ELF_RELA_BYTES (htab);
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, allocate_dynrelocs, info);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* If the .got section is more than 0x8000 bytes, we add
	 0x8000 to the value of _GLOBAL_OFFSET_TABLE_, so that 16
	 bit relocations have a greater chance of working. */
      if (htab->elf.sgot->size >= 0x8000
	  && elf_hash_table (info)->hgot->root.u.def.value == 0)
	elf_hash_table (info)->hgot->root.u.def.value = 0x8000;
    }

  if (htab->elf.sgotplt)
    {
      struct elf_link_hash_entry *got;
      got = elf_link_hash_lookup (elf_hash_table (info),
				  "_GLOBAL_OFFSET_TABLE_",
				  false, false, false);

      /* Don't allocate .got.plt section if there are no GOT nor PLT
	 entries and there is no refeence to _GLOBAL_OFFSET_TABLE_.  */
      if ((got == NULL
	   || !got->ref_regular_nonweak)
	  && (htab->elf.sgotplt->size
	      == (unsigned)GOTPLT_HEADER_SIZE (htab))
	  && (htab->elf.splt == NULL
	      || htab->elf.splt->size == 0)
	  && (htab->elf.sgot == NULL
	      || (htab->elf.sgot->size
		  == get_elf_backend_data (output_bfd)->got_header_size)))
	htab->elf.sgotplt->size = 0;
    }

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->elf.splt
	  || s == htab->elf.sgot
	  || s == htab->elf.sgotplt
	  || s == htab->elf.sdynbss
	  || s == htab->elf.sdynrelro)
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

/* Return the relocation value for @tpoff relocation. */

static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;

  return (address - htab->tls_sec->vma);
}

/* Copy SIZE bits from FROM to TO at address ADDR.  */

static void
tilegx_copy_bits (bfd_byte *addr, int from, int to, int size)
{
  int i;
  for (i = 0; i < size; i++)
    {
      int from_byte = (from + i) / 8;
      int from_bit = (from + i) % 8;
      int to_byte = (to + i) / 8;
      int to_bit = (to + i) % 8;
      bfd_byte to_mask = 1 << to_bit;
      addr[to_byte] = (addr[to_byte] & ~to_mask)
	| ((addr[from_byte] >> from_bit << to_bit) & to_mask);
    }
}

/* Replace the MASK bits in ADDR with those in INSN, for the next
   TILEGX_BUNDLE_SIZE_IN_BYTES bytes.  */

static void
tilegx_replace_insn (bfd_byte *addr, const bfd_byte *mask,
		     const bfd_byte *insn)
{
  int i;
  for (i = 0; i < TILEGX_BUNDLE_SIZE_IN_BYTES; i++)
    {
      addr[i] = (addr[i] & ~mask[i]) | (insn[i] & mask[i]);
    }
}

/* Mask to extract the bits corresponding to an instruction in a
   specific pipe of a bundle.  */
static const bfd_byte insn_mask_X1[] = {
  0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0x3f
};

/* Mask to extract the bits corresponding to an instruction in a
   specific pipe of a bundle, minus the destination operand and the
   first source operand.  */
static const bfd_byte insn_mask_X0_no_dest_no_srca[] = {
  0x00, 0xf0, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00
};

static const bfd_byte insn_mask_X1_no_dest_no_srca[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x3f
};

static const bfd_byte insn_mask_Y0_no_dest_no_srca[] = {
  0x00, 0xf0, 0x0f, 0x78, 0x00, 0x00, 0x00, 0x00
};
static const bfd_byte insn_mask_Y1_no_dest_no_srca[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0x3c
};

/* Mask to extract the bits corresponding to an instruction in a
   specific pipe of a bundle, minus the register operands.  */
static const bfd_byte insn_mask_X0_no_operand[] = {
  0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00
};

static const bfd_byte insn_mask_X1_no_operand[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x3f
};

static const bfd_byte insn_mask_Y0_no_operand[] = {
  0x00, 0x00, 0x0c, 0x78, 0x00, 0x00, 0x00, 0x00
};

static const bfd_byte insn_mask_Y1_no_operand[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x3c
};

/* Various instructions synthesized to support tls references.  */

/* ld r0, r0 in the X1 pipe, used for tls ie.  */
static const bfd_byte insn_tls_ie_ld_X1[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x6a, 0x28
};

/* ld4s r0, r0 in the X1 pipe, used for tls ie.  */
static const bfd_byte insn_tls_ie_ld4s_X1[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x6a, 0x28
};

/* add r0, r0, tp in various pipes, used for tls ie.  */
static const bfd_byte insn_tls_ie_add_X0X1[] = {
  0x00, 0x50, 0x0f, 0x50, 0x00, 0xa8, 0x07, 0x28
};
static const bfd_byte insn_tls_ie_add_Y0Y1[] = {
  0x00, 0x50, 0x27, 0x2c, 0x00, 0xa8, 0x13, 0x9a
};

/* addx r0, r0, tp in various pipes, used for tls ie.  */
static const bfd_byte insn_tls_ie_addx_X0X1[] = {
  0x00, 0x50, 0x0b, 0x50, 0x00, 0xa8, 0x05, 0x28
};
static const bfd_byte insn_tls_ie_addx_Y0Y1[] = {
  0x00, 0x50, 0x03, 0x2c, 0x00, 0xa8, 0x01, 0x9a
};

/* move r0, r0 in various pipes, used for tls gd.  */
static const bfd_byte insn_tls_gd_add_X0X1[] = {
  0x00, 0xf0, 0x07, 0x51, 0x00, 0xf8, 0x3b, 0x28
};
static const bfd_byte insn_tls_gd_add_Y0Y1[] = {
  0x00, 0xf0, 0x0b, 0x54, 0x00, 0xf8, 0x05, 0xae
};

static const bfd_byte *insn_move_X0X1 = insn_tls_gd_add_X0X1;
static const bfd_byte *insn_move_Y0Y1 = insn_tls_gd_add_Y0Y1;

static const bfd_byte *insn_add_X0X1 = insn_tls_ie_add_X0X1;
static const bfd_byte *insn_add_Y0Y1 = insn_tls_ie_add_Y0Y1;

static const bfd_byte *insn_addx_X0X1 = insn_tls_ie_addx_X0X1;
static const bfd_byte *insn_addx_Y0Y1 = insn_tls_ie_addx_Y0Y1;

/* Relocate an TILEGX ELF section.

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

int
tilegx_elf_relocate_section (bfd *output_bfd, struct bfd_link_info *info,
			     bfd *input_bfd, asection *input_section,
			     bfd_byte *contents, Elf_Internal_Rela *relocs,
			     Elf_Internal_Sym *local_syms,
			     asection **local_sections)
{
  struct tilegx_elf_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_vma *local_got_offsets;
  bfd_vma got_base;
  asection *sreloc;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  int num_relocs;

  htab = tilegx_elf_hash_table (info);
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
      tilegx_create_func create_func;
      asection *sec;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name;
      bfd_vma off;
      bool is_plt = false;
      bool resolved_to_zero;
      bool unresolved_reloc;

      r_type = TILEGX_ELF_R_TYPE (rel->r_info);
      if (r_type == R_TILEGX_GNU_VTINHERIT
	  || r_type == R_TILEGX_GNU_VTENTRY)
	continue;

      if ((unsigned int)r_type >= ARRAY_SIZE (tilegx_elf_howto_table))
	return _bfd_unrecognized_reloc (input_bfd, input_section, r_type);

      howto = tilegx_elf_howto_table + r_type;

      /* This is a final link.  */
      r_symndx = TILEGX_ELF_R_SYMNDX (htab, rel->r_info);
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
	case R_TILEGX_TLS_GD_CALL:
	case R_TILEGX_IMM8_X0_TLS_GD_ADD:
	case R_TILEGX_IMM8_Y0_TLS_GD_ADD:
	case R_TILEGX_IMM8_X1_TLS_GD_ADD:
	case R_TILEGX_IMM8_Y1_TLS_GD_ADD:
	case R_TILEGX_IMM8_X0_TLS_ADD:
	case R_TILEGX_IMM8_Y0_TLS_ADD:
	case R_TILEGX_IMM8_X1_TLS_ADD:
	case R_TILEGX_IMM8_Y1_TLS_ADD:
	  tls_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    tls_type =
	      _bfd_tilegx_elf_local_got_tls_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    tls_type = tilegx_elf_hash_entry(h)->tls_type;

	  is_tls_iele = (bfd_link_executable (info) || tls_type == GOT_TLS_IE);
	  is_tls_le = is_tls_iele && (!input_section->sec_flg0
				      && bfd_link_executable (info)
				      && (h == NULL || h->dynindx == -1));

	  if (r_type == R_TILEGX_TLS_GD_CALL)
	    {
	      if (is_tls_le)
		{
		  /* GD -> LE */
		  tilegx_replace_insn (contents + rel->r_offset,
				       insn_mask_X1, insn_move_X0X1);
		  continue;
		}
	      else if (is_tls_iele)
		{
		  /* GD -> IE */
		  if (ABI_64_P (output_bfd))
		    tilegx_replace_insn (contents + rel->r_offset,
					 insn_mask_X1, insn_tls_ie_ld_X1);
		  else
		    tilegx_replace_insn (contents + rel->r_offset,
					 insn_mask_X1, insn_tls_ie_ld4s_X1);
		  continue;
		}

	      /* GD -> GD */
	      h = (struct elf_link_hash_entry *)
		bfd_link_hash_lookup (info->hash, "__tls_get_addr", false,
				      false, true);
	      BFD_ASSERT (h != NULL);
	      r_type = R_TILEGX_JUMPOFF_X1_PLT;
	      howto = tilegx_elf_howto_table + r_type;
	    }
	  else if (r_type == R_TILEGX_IMM8_X0_TLS_ADD
		   || r_type ==  R_TILEGX_IMM8_X1_TLS_ADD
		   || r_type ==  R_TILEGX_IMM8_Y0_TLS_ADD
		   || r_type ==  R_TILEGX_IMM8_Y1_TLS_ADD)
	    {
	      bool is_pipe0 =
		(r_type == R_TILEGX_IMM8_X0_TLS_ADD
		 || r_type ==  R_TILEGX_IMM8_Y0_TLS_ADD);
	      bool is_X0X1 =
		(r_type == R_TILEGX_IMM8_X0_TLS_ADD
		 || r_type ==  R_TILEGX_IMM8_X1_TLS_ADD);
	      int dest_begin = is_pipe0 ? 0 : 31;
	      int src_begin;
	      const bfd_byte *insn;
	      const bfd_byte *mask = NULL;

	      if (is_tls_le)
		{
		  /* 1. copy dest operand into the first source operand.
		     2. change the opcode to "move".  */
		  src_begin = is_pipe0 ? 6 : 37;
		  insn = is_X0X1 ? insn_move_X0X1 : insn_move_Y0Y1;

		  switch (r_type)
		    {
		    case R_TILEGX_IMM8_X0_TLS_ADD:
		      mask = insn_mask_X0_no_dest_no_srca;
		      break;
		    case R_TILEGX_IMM8_X1_TLS_ADD:
		      mask = insn_mask_X1_no_dest_no_srca;
		      break;
		    case R_TILEGX_IMM8_Y0_TLS_ADD:
		      mask = insn_mask_Y0_no_dest_no_srca;
		      break;
		    case R_TILEGX_IMM8_Y1_TLS_ADD:
		      mask = insn_mask_Y1_no_dest_no_srca;
		      break;
		    }
		}
	      else
		{
		  /* 1. copy dest operand into the second source operand.
		     2. change the opcode to "add".  */
		  src_begin = is_pipe0 ? 12 : 43;
		  if (ABI_64_P (output_bfd))
		    insn = is_X0X1 ? insn_add_X0X1 : insn_add_Y0Y1;
		  else
		    insn = is_X0X1 ? insn_addx_X0X1 : insn_addx_Y0Y1;

		  switch (r_type)
		    {
		    case R_TILEGX_IMM8_X0_TLS_ADD:
		      mask = insn_mask_X0_no_operand;
		      break;
		    case R_TILEGX_IMM8_X1_TLS_ADD:
		      mask = insn_mask_X1_no_operand;
		      break;
		    case R_TILEGX_IMM8_Y0_TLS_ADD:
		      mask = insn_mask_Y0_no_operand;
		      break;
		    case R_TILEGX_IMM8_Y1_TLS_ADD:
		      mask = insn_mask_Y1_no_operand;
		      break;
		    }
		}

	      tilegx_copy_bits (contents + rel->r_offset, dest_begin,
				src_begin, 6);
	      tilegx_replace_insn (contents  + rel->r_offset, mask, insn);

	      continue;
	    }
	  else
	    {
	      const bfd_byte *mask = NULL;
	      const bfd_byte *add_insn = NULL;
	      bool is_64bit = ABI_64_P (output_bfd);

	      switch (r_type)
		{
		case R_TILEGX_IMM8_X0_TLS_GD_ADD:
		  add_insn = is_tls_iele
		    ? (is_64bit ? insn_tls_ie_add_X0X1 : insn_tls_ie_addx_X0X1)
		    : insn_tls_gd_add_X0X1;
		  mask = insn_mask_X0_no_dest_no_srca;
		  break;
		case R_TILEGX_IMM8_X1_TLS_GD_ADD:
		  add_insn = is_tls_iele
		    ? (is_64bit ? insn_tls_ie_add_X0X1 : insn_tls_ie_addx_X0X1)
		    : insn_tls_gd_add_X0X1;
		  mask = insn_mask_X1_no_dest_no_srca;
		  break;
		case R_TILEGX_IMM8_Y0_TLS_GD_ADD:
		  add_insn = is_tls_iele
		    ? (is_64bit ? insn_tls_ie_add_Y0Y1 : insn_tls_ie_addx_Y0Y1)
		    : insn_tls_gd_add_Y0Y1;
		  mask = insn_mask_Y0_no_dest_no_srca;
		  break;
		case R_TILEGX_IMM8_Y1_TLS_GD_ADD:
		  add_insn = is_tls_iele
		    ? (is_64bit ? insn_tls_ie_add_Y0Y1 : insn_tls_ie_addx_Y0Y1)
		    : insn_tls_gd_add_Y0Y1;
		  mask = insn_mask_Y1_no_dest_no_srca;
		  break;
		}

	      tilegx_replace_insn (contents + rel->r_offset, mask, add_insn);

	      continue;
	    }
	  break;
	case R_TILEGX_TLS_IE_LOAD:
	  if (!input_section->sec_flg0
	      && bfd_link_executable (info)
	      && (h == NULL || h->dynindx == -1))
	    {
	      /* IE -> LE */
	      tilegx_replace_insn (contents + rel->r_offset,
				   insn_mask_X1_no_dest_no_srca,
				   insn_move_X0X1);
	    }
	  else
	    {
	      /* IE -> IE */
	      if (ABI_64_P (output_bfd))
		tilegx_replace_insn (contents + rel->r_offset,
				     insn_mask_X1_no_dest_no_srca,
				     insn_tls_ie_ld_X1);
	      else
		tilegx_replace_insn (contents + rel->r_offset,
				     insn_mask_X1_no_dest_no_srca,
				     insn_tls_ie_ld4s_X1);
	    }
	  continue;
	  break;
	default:
	  break;
	}

      resolved_to_zero = (h != NULL
			  && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));

      switch (r_type)
	{
	case R_TILEGX_IMM16_X0_HW0_GOT:
	case R_TILEGX_IMM16_X1_HW0_GOT:
	case R_TILEGX_IMM16_X0_HW0_LAST_GOT:
	case R_TILEGX_IMM16_X1_HW0_LAST_GOT:
	case R_TILEGX_IMM16_X0_HW1_LAST_GOT:
	case R_TILEGX_IMM16_X1_HW1_LAST_GOT:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */
	  if (htab->elf.sgot == NULL)
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
		     of 8 for 64-bit, we use the least significant bit
		     to record whether we have initialized it already.

		     When doing a dynamic link, we create a .rela.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      TILEGX_ELF_PUT_WORD (htab, output_bfd, relocation,
					   htab->elf.sgot->contents + off);
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

	      /* The offset must always be a multiple of 8 on 64-bit.
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

		      /* We need to generate a R_TILEGX_RELATIVE reloc
			 for the dynamic linker.  */
		      s = htab->elf.srelgot;
		      BFD_ASSERT (s != NULL);

		      outrel.r_offset = (htab->elf.sgot->output_section->vma
					 + htab->elf.sgot->output_offset
					 + off);
		      outrel.r_info =
			TILEGX_ELF_R_INFO (htab, NULL, 0, R_TILEGX_RELATIVE);
		      outrel.r_addend = relocation;
		      relocation = 0;
		      tilegx_elf_append_rela (output_bfd, s, &outrel);
		    }

		  TILEGX_ELF_PUT_WORD (htab, output_bfd, relocation,
				       htab->elf.sgot->contents + off);
		  local_got_offsets[r_symndx] |= 1;
		}
	    }
	  relocation = off - got_base;
	  break;

	case R_TILEGX_JUMPOFF_X1_PLT:
	case R_TILEGX_IMM16_X0_HW0_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW0_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW1_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW1_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW2_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW2_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW3_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW3_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL:
	case R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */
	  BFD_ASSERT (h != NULL);

	  if (h->plt.offset == (bfd_vma) -1 || htab->elf.splt == NULL)
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      break;
	    }

	  relocation = (htab->elf.splt->output_section->vma
			+ htab->elf.splt->output_offset
			+ h->plt.offset);
	  unresolved_reloc = false;
	  break;

	case R_TILEGX_64_PCREL:
	case R_TILEGX_32_PCREL:
	case R_TILEGX_16_PCREL:
	case R_TILEGX_8_PCREL:
	case R_TILEGX_IMM16_X0_HW0_PCREL:
	case R_TILEGX_IMM16_X1_HW0_PCREL:
	case R_TILEGX_IMM16_X0_HW1_PCREL:
	case R_TILEGX_IMM16_X1_HW1_PCREL:
	case R_TILEGX_IMM16_X0_HW2_PCREL:
	case R_TILEGX_IMM16_X1_HW2_PCREL:
	case R_TILEGX_IMM16_X0_HW3_PCREL:
	case R_TILEGX_IMM16_X1_HW3_PCREL:
	case R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
	case R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
	case R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
	case R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
	case R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
	case R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
	  if (h != NULL
	      && strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
	    break;
	  /* Fall through.  */
	case R_TILEGX_64:
	case R_TILEGX_32:
	case R_TILEGX_16:
	case R_TILEGX_8:
	case R_TILEGX_HW0:
	case R_TILEGX_HW1:
	case R_TILEGX_HW2:
	case R_TILEGX_HW3:
	case R_TILEGX_HW0_LAST:
	case R_TILEGX_HW1_LAST:
	case R_TILEGX_HW2_LAST:
	case R_TILEGX_COPY:
	case R_TILEGX_GLOB_DAT:
	case R_TILEGX_JMP_SLOT:
	case R_TILEGX_RELATIVE:
	case R_TILEGX_BROFF_X1:
	case R_TILEGX_JUMPOFF_X1:
	case R_TILEGX_IMM8_X0:
	case R_TILEGX_IMM8_Y0:
	case R_TILEGX_IMM8_X1:
	case R_TILEGX_IMM8_Y1:
	case R_TILEGX_DEST_IMM8_X1:
	case R_TILEGX_MT_IMM14_X1:
	case R_TILEGX_MF_IMM14_X1:
	case R_TILEGX_MMSTART_X0:
	case R_TILEGX_MMEND_X0:
	case R_TILEGX_SHAMT_X0:
	case R_TILEGX_SHAMT_X1:
	case R_TILEGX_SHAMT_Y0:
	case R_TILEGX_SHAMT_Y1:
	case R_TILEGX_IMM16_X0_HW0:
	case R_TILEGX_IMM16_X1_HW0:
	case R_TILEGX_IMM16_X0_HW1:
	case R_TILEGX_IMM16_X1_HW1:
	case R_TILEGX_IMM16_X0_HW2:
	case R_TILEGX_IMM16_X1_HW2:
	case R_TILEGX_IMM16_X0_HW3:
	case R_TILEGX_IMM16_X1_HW3:
	case R_TILEGX_IMM16_X0_HW0_LAST:
	case R_TILEGX_IMM16_X1_HW0_LAST:
	case R_TILEGX_IMM16_X0_HW1_LAST:
	case R_TILEGX_IMM16_X1_HW1_LAST:
	case R_TILEGX_IMM16_X0_HW2_LAST:
	case R_TILEGX_IMM16_X1_HW2_LAST:
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
		case R_TILEGX_64_PCREL:
		case R_TILEGX_32_PCREL:
		case R_TILEGX_16_PCREL:
		case R_TILEGX_8_PCREL:
		  /* If the symbol is not dynamic, we should not keep
		     a dynamic relocation.  But an .rela.* slot has been
		     allocated for it, output R_TILEGX_NONE.
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
		  outrel.r_info = TILEGX_ELF_R_INFO (htab, rel, h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  if (r_type == R_TILEGX_32 || r_type == R_TILEGX_64)
		    {
		      outrel.r_info = TILEGX_ELF_R_INFO (htab, NULL, 0,
							 R_TILEGX_RELATIVE);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		  else
		    {
		      long indx;

		      outrel.r_addend = relocation + rel->r_addend;

		      if (is_plt)
			sec = htab->elf.splt;

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
			      osec = htab->elf.text_index_section;
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

		      outrel.r_info = TILEGX_ELF_R_INFO (htab, rel, indx,
							 r_type);
		    }
		}

	      tilegx_elf_append_rela (output_bfd, sreloc, &outrel);

	      /* This reloc will be computed at runtime, so there's no
		 need to do anything now.  */
	      if (! relocate)
		continue;
	    }
	  break;

	case R_TILEGX_IMM16_X0_HW0_TLS_LE:
	case R_TILEGX_IMM16_X1_HW0_TLS_LE:
	case R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE:
	case R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE:
	case R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE:
	case R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE:
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
		  outrel.r_info = TILEGX_ELF_R_INFO (htab, NULL, 0, r_type);
		  outrel.r_addend = relocation - dtpoff_base (info)
				    + rel->r_addend;
		}

	      tilegx_elf_append_rela (output_bfd, sreloc, &outrel);
	      continue;
	    }
	  relocation = tpoff (info, relocation);
	  break;

	case R_TILEGX_IMM16_X0_HW0_TLS_GD:
	case R_TILEGX_IMM16_X1_HW0_TLS_GD:
	case R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
	case R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
	case R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
	case R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
	case R_TILEGX_IMM16_X0_HW0_TLS_IE:
	case R_TILEGX_IMM16_X1_HW0_TLS_IE:
	case R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
	case R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
	case R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
	case R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
	  r_type = tilegx_elf_tls_transition (info, r_type, h == NULL,
					      input_section->sec_flg0);
	  tls_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    tls_type =
	      _bfd_tilegx_elf_local_got_tls_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    {
	      tls_type = tilegx_elf_hash_entry(h)->tls_type;
	      if (bfd_link_executable (info)
		  && h->dynindx == -1
		  && tls_type == GOT_TLS_IE)
		r_type = (!input_section->sec_flg0
			  ? tilegx_tls_translate_to_le (r_type)
			  : tilegx_tls_translate_to_ie (r_type));
	    }

	  if (tls_type == GOT_TLS_IE)
	    r_type = tilegx_tls_translate_to_ie (r_type);

	  if (r_type == R_TILEGX_IMM16_X0_HW0_TLS_LE
	      || r_type == R_TILEGX_IMM16_X1_HW0_TLS_LE
	      || r_type == R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE
	      || r_type == R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE
	      || r_type == R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE
	      || r_type == R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE)
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

	  if (htab->elf.sgot == NULL)
	    abort ();

	  if ((off & 1) != 0)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;
	      int indx = 0;
	      bool need_relocs = false;

	      if (htab->elf.srelgot == NULL)
		abort ();

	      if (h != NULL)
	      {
		bool dyn;
		dyn = htab->elf.dynamic_sections_created;

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
		  case R_TILEGX_IMM16_X0_HW0_TLS_IE:
		  case R_TILEGX_IMM16_X1_HW0_TLS_IE:
		  case R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
		  case R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
		  case R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
		  case R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
		    if (need_relocs) {
		      TILEGX_ELF_PUT_WORD (htab, output_bfd, 0,
					   htab->elf.sgot->contents + off);
		      outrel.r_offset = (htab->elf.sgot->output_section->vma
				       + htab->elf.sgot->output_offset + off);
		      outrel.r_addend = 0;
		      if (indx == 0)
			outrel.r_addend = relocation - dtpoff_base (info);
		      outrel.r_info = TILEGX_ELF_R_INFO (htab, NULL, indx,
							 TILEGX_ELF_TPOFF_RELOC (htab));
		      tilegx_elf_append_rela (output_bfd, htab->elf.srelgot, &outrel);
		    } else {
		      TILEGX_ELF_PUT_WORD (htab, output_bfd,
					   tpoff (info, relocation),
					   htab->elf.sgot->contents + off);
		    }
		    break;

		  case R_TILEGX_IMM16_X0_HW0_TLS_GD:
		  case R_TILEGX_IMM16_X1_HW0_TLS_GD:
		  case R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
		  case R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
		  case R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
		  case R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
		    if (need_relocs) {
		      outrel.r_offset = (htab->elf.sgot->output_section->vma
				       + htab->elf.sgot->output_offset + off);
		      outrel.r_addend = 0;
		      outrel.r_info = TILEGX_ELF_R_INFO (htab, NULL, indx,
							 TILEGX_ELF_DTPMOD_RELOC (htab));
		      TILEGX_ELF_PUT_WORD (htab, output_bfd, 0,
					   htab->elf.sgot->contents + off);
		      tilegx_elf_append_rela (output_bfd, htab->elf.srelgot, &outrel);
		      if (indx == 0)
			{
			  BFD_ASSERT (! unresolved_reloc);
			  TILEGX_ELF_PUT_WORD (htab, output_bfd,
					       relocation - dtpoff_base (info),
					       (htab->elf.sgot->contents + off +
						TILEGX_ELF_WORD_BYTES (htab)));
			}
		      else
			{
			  TILEGX_ELF_PUT_WORD (htab, output_bfd, 0,
					       (htab->elf.sgot->contents + off +
						TILEGX_ELF_WORD_BYTES (htab)));
			  outrel.r_info = TILEGX_ELF_R_INFO (htab, NULL, indx,
							     TILEGX_ELF_DTPOFF_RELOC (htab));
			  outrel.r_offset += TILEGX_ELF_WORD_BYTES (htab);
			  tilegx_elf_append_rela (output_bfd, htab->elf.srelgot, &outrel);
			}
		    }

		    else {
		      /* If we are not emitting relocations for a
			 general dynamic reference, then we must be in a
			 static link or an executable link with the
			 symbol binding locally.  Mark it as belonging
			 to module 1, the executable.  */
		      TILEGX_ELF_PUT_WORD (htab, output_bfd, 1,
					   htab->elf.sgot->contents + off );
		      TILEGX_ELF_PUT_WORD (htab, output_bfd,
					   relocation - dtpoff_base (info),
					   htab->elf.sgot->contents + off +
					   TILEGX_ELF_WORD_BYTES (htab));
		   }
		   break;
		}
	    }

	  if (off >= (bfd_vma) -2)
	    abort ();

	  relocation = off - got_base;
	  unresolved_reloc = false;
	  howto = tilegx_elf_howto_table + r_type;
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
				TILEGX_ELF_WORD_BYTES (htab) * 8,
				relocation);

	/*
	 * Write the relocated value out into the raw section data.
	 * Don't put a relocation out in the .rela section.
	 */
	tilegx_bundle_bits mask = create_func(-1);
	tilegx_bundle_bits value = create_func(relocation >> howto->rightshift);

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

bool
tilegx_elf_finish_dynamic_symbol (bfd *output_bfd,
				  struct bfd_link_info *info,
				  struct elf_link_hash_entry *h,
				  Elf_Internal_Sym *sym)
{
  struct tilegx_elf_link_hash_table *htab;

  htab = tilegx_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *srela;
      asection *sgotplt;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      bfd_vma r_offset;
      const struct elf_backend_data *bed = get_elf_backend_data (output_bfd);


      int rela_index;

      /* This symbol has an entry in the PLT.  Set it up.  */

      BFD_ASSERT (h->dynindx != -1);

      splt = htab->elf.splt;
      srela = htab->elf.srelplt;
      sgotplt = htab->elf.sgotplt;

      if (splt == NULL || srela == NULL)
       abort ();

      /* Fill in the entry in the procedure linkage table.  */
      rela_index = tilegx_plt_entry_build (output_bfd, htab, splt, sgotplt,
					   h->plt.offset, &r_offset);

      /* Fill in the entry in the global offset table, which initially points
	 to the beginning of the plt.  */
      TILEGX_ELF_PUT_WORD (htab, output_bfd,
			   splt->output_section->vma + splt->output_offset,
			   sgotplt->contents + r_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgotplt->output_section->vma
		       + sgotplt->output_offset
		       + r_offset);
      rela.r_addend = 0;
      rela.r_info = TILEGX_ELF_R_INFO (htab, NULL, h->dynindx, R_TILEGX_JMP_SLOT);

      loc = srela->contents + rela_index * TILEGX_ELF_RELA_BYTES (htab);
      bed->s->swap_reloca_out (output_bfd, &rela, loc);

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
      && tilegx_elf_hash_entry(h)->tls_type != GOT_TLS_GD
      && tilegx_elf_hash_entry(h)->tls_type != GOT_TLS_IE)
    {
      asection *sgot;
      asection *srela;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the GOT.  Set it up.  */

      sgot = htab->elf.sgot;
      srela = htab->elf.srelgot;
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
	  rela.r_info = TILEGX_ELF_R_INFO (htab, NULL, 0, R_TILEGX_RELATIVE);
	  rela.r_addend = (h->root.u.def.value
			   + sec->output_section->vma
			   + sec->output_offset);
	}
      else
	{
	  rela.r_info = TILEGX_ELF_R_INFO (htab, NULL, h->dynindx, R_TILEGX_GLOB_DAT);
	  rela.r_addend = 0;
	}

      TILEGX_ELF_PUT_WORD (htab, output_bfd, 0,
			   sgot->contents + (h->got.offset & ~(bfd_vma) 1));
      tilegx_elf_append_rela (output_bfd, srela, &rela);
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;

      /* This symbols needs a copy reloc.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1);

      if (h->root.u.def.section == htab->elf.sdynrelro)
	s = htab->elf.sreldynrelro;
      else
	s = htab->elf.srelbss;
      BFD_ASSERT (s != NULL);

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = TILEGX_ELF_R_INFO (htab, NULL, h->dynindx, R_TILEGX_COPY);
      rela.r_addend = 0;
      tilegx_elf_append_rela (output_bfd, s, &rela);
    }

  /* Mark some specially defined symbols as absolute. */
  if (h == htab->elf.hdynamic
      || (h == htab->elf.hgot || h == htab->elf.hplt))
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Finish up the dynamic sections.  */

static bool
tilegx_finish_dyn (bfd *output_bfd, struct bfd_link_info *info,
		   bfd *dynobj, asection *sdyn,
		   asection *splt ATTRIBUTE_UNUSED)
{
  struct tilegx_elf_link_hash_table *htab;
  const struct elf_backend_data *bed;
  bfd_byte *dyncon, *dynconend;
  size_t dynsize;

  htab = tilegx_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  bed = get_elf_backend_data (output_bfd);
  dynsize = bed->s->sizeof_dyn;
  dynconend = sdyn->contents + sdyn->size;

  for (dyncon = sdyn->contents; dyncon < dynconend; dyncon += dynsize)
    {
      Elf_Internal_Dyn dyn;
      asection *s;

      bed->s->swap_dyn_in (dynobj, dyncon, &dyn);

      switch (dyn.d_tag)
	{
	case DT_PLTGOT:
	  s = htab->elf.sgotplt;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	  break;
	case DT_JMPREL:
	  s = htab->elf.srelplt;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	  break;
	case DT_PLTRELSZ:
	  s = htab->elf.srelplt;
	  dyn.d_un.d_val = s->size;
	  break;
	default:
	  continue;
	}

      bed->s->swap_dyn_out (output_bfd, &dyn, dyncon);
    }
  return true;
}

bool
tilegx_elf_finish_dynamic_sections (bfd *output_bfd,
				    struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn;
  struct tilegx_elf_link_hash_table *htab;
  size_t pad_size;

  htab = tilegx_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dynobj = htab->elf.dynobj;

  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      asection *splt;
      bool ret;

      splt = htab->elf.splt;
      BFD_ASSERT (splt != NULL && sdyn != NULL);

      ret = tilegx_finish_dyn (output_bfd, info, dynobj, sdyn, splt);

      if (!ret)
	return ret;

      /* Fill in the head and tail entries in the procedure linkage table.  */
      if (splt->size > 0)
	{
	  memcpy (splt->contents,
		  ABI_64_P (output_bfd) ?
		    tilegx64_plt0_entry : tilegx32_plt0_entry,
		  PLT_HEADER_SIZE);

	  memcpy (splt->contents + splt->size
		  - PLT_ENTRY_SIZE + PLT_HEADER_SIZE,
		  ABI_64_P (output_bfd) ?
		    tilegx64_plt_tail_entry : tilegx32_plt_tail_entry,
		  PLT_TAIL_SIZE);
	  /* Add padding so that the plt section is a multiple of its
	     entry size.  */
	  pad_size = PLT_ENTRY_SIZE - PLT_HEADER_SIZE - PLT_TAIL_SIZE;
	  memset (splt->contents + splt->size - pad_size, 0, pad_size);

	  elf_section_data (splt->output_section)->this_hdr.sh_entsize
	    = PLT_ENTRY_SIZE;
	}
    }

  if (htab->elf.sgotplt)
    {
      if (bfd_is_abs_section (htab->elf.sgotplt->output_section))
	{
	  _bfd_error_handler
	    (_("discarded output section: `%pA'"), htab->elf.sgotplt);
	  return false;
	}

      if (htab->elf.sgotplt->size > 0)
	{
	  /* Write the first two entries in .got.plt, needed for the dynamic
	     linker.  */
	  TILEGX_ELF_PUT_WORD (htab, output_bfd, (bfd_vma) -1,
			       htab->elf.sgotplt->contents);
	  TILEGX_ELF_PUT_WORD (htab, output_bfd, (bfd_vma) 0,
			       htab->elf.sgotplt->contents
			       + GOT_ENTRY_SIZE (htab));

	  elf_section_data (htab->elf.sgotplt->output_section)->this_hdr.sh_entsize =
	    GOT_ENTRY_SIZE (htab);
	}
    }

  if (htab->elf.sgot)
    {
      if (htab->elf.sgot->size > 0)
	{
	  /* Set the first entry in the global offset table to the address of
	     the dynamic section.  */
	  bfd_vma val = (sdyn ?
			 sdyn->output_section->vma + sdyn->output_offset :
			 0);
	  TILEGX_ELF_PUT_WORD (htab, output_bfd, val,
			       htab->elf.sgot->contents);

	  elf_section_data (htab->elf.sgot->output_section)->this_hdr.sh_entsize =
	    GOT_ENTRY_SIZE (htab);
	}
    }

  return true;
}



/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

bfd_vma
tilegx_elf_plt_sym_val (bfd_vma i, const asection *plt,
			const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + PLT_HEADER_SIZE + i * PLT_ENTRY_SIZE;
}

enum elf_reloc_type_class
tilegx_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			 const asection *rel_sec ATTRIBUTE_UNUSED,
			 const Elf_Internal_Rela *rela)
{
  switch ((int) TILEGX_ELF_R_TYPE (rela->r_info))
    {
    case R_TILEGX_RELATIVE:
      return reloc_class_relative;
    case R_TILEGX_JMP_SLOT:
      return reloc_class_plt;
    case R_TILEGX_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

int
tilegx_additional_program_headers (bfd *abfd,
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


bool
_bfd_tilegx_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  const char *targ1 = bfd_get_target (ibfd);
  const char *targ2 = bfd_get_target (obfd);

  if (strcmp (targ1, targ2) != 0)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: cannot link together %s and %s objects"),
	 ibfd, targ1, targ2);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}
