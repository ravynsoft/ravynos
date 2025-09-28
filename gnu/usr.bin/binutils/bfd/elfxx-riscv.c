/* RISC-V-specific support for ELF.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   Contributed by Andrew Waterman (andrew@sifive.com).
   Based on TILE-Gx and MIPS targets.

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
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/riscv.h"
#include "opcode/riscv.h"
#include "libiberty.h"
#include "elfxx-riscv.h"
#include "safe-ctype.h"

#define MINUS_ONE ((bfd_vma)0 - 1)

/* Special handler for ADD/SUB relocations that allows them to be filled out
   both in the pre-linked and post-linked file.  This is necessary to make
   pre-linked debug info work, as due to linker relaxations we need to emit
   relocations for the debug info.  */
static bfd_reloc_status_type riscv_elf_add_sub_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type riscv_elf_ignore_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);

/* The relocation table used for SHT_RELA sections.  */

static reloc_howto_type howto_table[] =
{
  /* No relocation.  */
  HOWTO (R_RISCV_NONE,			/* type */
	 0,				/* rightshift */
	 0,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_NONE",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 32 bit relocation.  */
  HOWTO (R_RISCV_32,			/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_32",			/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 64 bit relocation.  */
  HOWTO (R_RISCV_64,			/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_64",			/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 MINUS_ONE,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* Relocation against a local symbol in a shared object.  */
  HOWTO (R_RISCV_RELATIVE,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_RELATIVE",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  HOWTO (R_RISCV_COPY,			/* type */
	 0,				/* rightshift */
	 0,				/* this one is variable size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_COPY",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  HOWTO (R_RISCV_JUMP_SLOT,		/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_JUMP_SLOT",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* Dynamic TLS relocations.  */
  HOWTO (R_RISCV_TLS_DTPMOD32,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TLS_DTPMOD32",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  HOWTO (R_RISCV_TLS_DTPMOD64,		/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TLS_DTPMOD64",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 MINUS_ONE,			/* dst_mask */
	 false),			/* pcrel_offset */

  HOWTO (R_RISCV_TLS_DTPREL32,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TLS_DTPREL32",	/* name */
	 true,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  HOWTO (R_RISCV_TLS_DTPREL64,		/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TLS_DTPREL64",	/* name */
	 true,				/* partial_inplace */
	 0,				/* src_mask */
	 MINUS_ONE,			/* dst_mask */
	 false),			/* pcrel_offset */

  HOWTO (R_RISCV_TLS_TPREL32,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TLS_TPREL32",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  HOWTO (R_RISCV_TLS_TPREL64,		/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TLS_TPREL64",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 MINUS_ONE,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* Reserved for future relocs that the dynamic linker must understand.  */
  EMPTY_HOWTO (12),
  EMPTY_HOWTO (13),
  EMPTY_HOWTO (14),
  EMPTY_HOWTO (15),

  /* 12-bit PC-relative branch offset.  */
  HOWTO (R_RISCV_BRANCH,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_BRANCH",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_BTYPE_IMM (-1U),	/* dst_mask */
	 true),				/* pcrel_offset */

  /* 20-bit PC-relative jump offset.  */
  HOWTO (R_RISCV_JAL,			/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_JAL",			/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_JTYPE_IMM (-1U),	/* dst_mask */
	 true),				/* pcrel_offset */

  /* 32-bit PC-relative function call (AUIPC/JALR).  */
  HOWTO (R_RISCV_CALL,			/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_CALL",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_UTYPE_IMM (-1U) | ((bfd_vma) ENCODE_ITYPE_IMM (-1U) << 32),
					/* dst_mask */
	 true),				/* pcrel_offset */

  /* Like R_RISCV_CALL, but not locally binding.  */
  HOWTO (R_RISCV_CALL_PLT,		/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_CALL_PLT",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_UTYPE_IMM (-1U) | ((bfd_vma) ENCODE_ITYPE_IMM (-1U) << 32),
					/* dst_mask */
	 true),				/* pcrel_offset */

  /* High 20 bits of 32-bit PC-relative GOT access.  */
  HOWTO (R_RISCV_GOT_HI20,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_GOT_HI20",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_UTYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* High 20 bits of 32-bit PC-relative TLS IE GOT access.  */
  HOWTO (R_RISCV_TLS_GOT_HI20,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TLS_GOT_HI20",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_UTYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* High 20 bits of 32-bit PC-relative TLS GD GOT reference.  */
  HOWTO (R_RISCV_TLS_GD_HI20,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TLS_GD_HI20",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_UTYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* High 20 bits of 32-bit PC-relative reference.  */
  HOWTO (R_RISCV_PCREL_HI20,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_PCREL_HI20",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_UTYPE_IMM (-1U),	/* dst_mask */
	 true),				/* pcrel_offset */

  /* Low 12 bits of a 32-bit PC-relative load or add.  */
  HOWTO (R_RISCV_PCREL_LO12_I,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_PCREL_LO12_I",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_ITYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* Low 12 bits of a 32-bit PC-relative store.  */
  HOWTO (R_RISCV_PCREL_LO12_S,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_PCREL_LO12_S",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_STYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* High 20 bits of 32-bit absolute address.  */
  HOWTO (R_RISCV_HI20,			/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_HI20",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_UTYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* High 12 bits of 32-bit load or add.  */
  HOWTO (R_RISCV_LO12_I,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_LO12_I",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_ITYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* High 12 bits of 32-bit store.  */
  HOWTO (R_RISCV_LO12_S,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_LO12_S",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_STYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* High 20 bits of TLS LE thread pointer offset.  */
  HOWTO (R_RISCV_TPREL_HI20,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TPREL_HI20",		/* name */
	 true,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_UTYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* Low 12 bits of TLS LE thread pointer offset for loads and adds.  */
  HOWTO (R_RISCV_TPREL_LO12_I,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TPREL_LO12_I",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_ITYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* Low 12 bits of TLS LE thread pointer offset for stores.  */
  HOWTO (R_RISCV_TPREL_LO12_S,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TPREL_LO12_S",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_STYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* TLS LE thread pointer usage.  May be relaxed.  */
  HOWTO (R_RISCV_TPREL_ADD,		/* type */
	 0,				/* rightshift */
	 0,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TPREL_ADD",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 8-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_ADD8,			/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 8,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_ADD8",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xff,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 16-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_ADD16,			/* type */
	 0,				/* rightshift */
	 2,				/* size */
	 16,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_ADD16",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 32-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_ADD32,			/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_ADD32",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 64-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_ADD64,			/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_ADD64",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 MINUS_ONE,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 8-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_SUB8,			/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 8,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_SUB8",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xff,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 16-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_SUB16,			/* type */
	 0,				/* rightshift */
	 2,				/* size */
	 16,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_SUB16",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 32-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_SUB32,			/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_SUB32",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 64-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_SUB64,			/* type */
	 0,				/* rightshift */
	 8,				/* size */
	 64,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_SUB64",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 MINUS_ONE,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 41 and 42 are reserved.  */
  EMPTY_HOWTO (0),
  EMPTY_HOWTO (0),

  /* Indicates an alignment statement.  The addend field encodes how many
     bytes of NOPs follow the statement.  The desired alignment is the
     addend rounded up to the next power of two.  */
  HOWTO (R_RISCV_ALIGN,			/* type */
	 0,				/* rightshift */
	 0,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_ALIGN",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 8-bit PC-relative branch offset.  */
  HOWTO (R_RISCV_RVC_BRANCH,		/* type */
	 0,				/* rightshift */
	 2,				/* size */
	 16,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_RVC_BRANCH",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_CBTYPE_IMM (-1U),	/* dst_mask */
	 true),				/* pcrel_offset */

  /* 11-bit PC-relative jump offset.  */
  HOWTO (R_RISCV_RVC_JUMP,		/* type */
	 0,				/* rightshift */
	 2,				/* size */
	 16,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_RVC_JUMP",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_CJTYPE_IMM (-1U),	/* dst_mask */
	 true),				/* pcrel_offset */

  /* High 6 bits of 18-bit absolute address.  */
  HOWTO (R_RISCV_RVC_LUI,		/* type */
	 0,				/* rightshift */
	 2,				/* size */
	 16,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_RVC_LUI",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_CITYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* GP-relative load.  */
  HOWTO (R_RISCV_GPREL_I,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_GPREL_I",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_ITYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* GP-relative store.  */
  HOWTO (R_RISCV_GPREL_S,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_GPREL_S",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_STYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* TP-relative TLS LE load.  */
  HOWTO (R_RISCV_TPREL_I,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TPREL_I",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_ITYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* TP-relative TLS LE store.  */
  HOWTO (R_RISCV_TPREL_S,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_TPREL_S",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 ENCODE_STYPE_IMM (-1U),	/* dst_mask */
	 false),			/* pcrel_offset */

  /* The paired relocation may be relaxed.  */
  HOWTO (R_RISCV_RELAX,			/* type */
	 0,				/* rightshift */
	 0,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_RELAX",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 6-bit in-place addition, for local label subtraction.  */
  HOWTO (R_RISCV_SUB6,			/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 8,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_add_sub_reloc,	/* special_function */
	 "R_RISCV_SUB6",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0x3f,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 6-bit in-place setting, for local label subtraction.  */
  HOWTO (R_RISCV_SET6,			/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 8,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_SET6",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0x3f,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 8-bit in-place setting, for local label subtraction.  */
  HOWTO (R_RISCV_SET8,			/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 8,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_SET8",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xff,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* 16-bit in-place setting, for local label subtraction.  */
  HOWTO (R_RISCV_SET16,			/* type */
	 0,				/* rightshift */
	 2,				/* size */
	 16,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_SET16",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 32-bit in-place setting, for local label subtraction.  */
  HOWTO (R_RISCV_SET32,			/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_SET32",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* 32-bit PC relative.  */
  HOWTO (R_RISCV_32_PCREL,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 true,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_32_PCREL",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* Relocation against a local ifunc symbol in a shared object.  */
  HOWTO (R_RISCV_IRELATIVE,		/* type */
	 0,				/* rightshift */
	 4,				/* size */
	 32,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_RISCV_IRELATIVE",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 false),			/* pcrel_offset */

  /* Reserved for R_RISCV_PLT32.  */
  EMPTY_HOWTO (59),

  /* N-bit in-place setting, for unsigned-leb128 local label subtraction.  */
  HOWTO (R_RISCV_SET_ULEB128,		/* type */
	 0,				/* rightshift */
	 0,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_ignore_reloc,	/* special_function */
	 "R_RISCV_SET_ULEB128",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* N-bit in-place addition, for unsigned-leb128 local label subtraction.  */
  HOWTO (R_RISCV_SUB_ULEB128,		/* type */
	 0,				/* rightshift */
	 0,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 riscv_elf_ignore_reloc,	/* special_function */
	 "R_RISCV_SUB_ULEB128",		/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */
};

/* A mapping from BFD reloc types to RISC-V ELF reloc types.  */
struct elf_reloc_map
{
  bfd_reloc_code_real_type bfd_val;
  enum elf_riscv_reloc_type elf_val;
};

static const struct elf_reloc_map riscv_reloc_map[] =
{
  { BFD_RELOC_NONE, R_RISCV_NONE },
  { BFD_RELOC_32, R_RISCV_32 },
  { BFD_RELOC_64, R_RISCV_64 },
  { BFD_RELOC_RISCV_ADD8, R_RISCV_ADD8 },
  { BFD_RELOC_RISCV_ADD16, R_RISCV_ADD16 },
  { BFD_RELOC_RISCV_ADD32, R_RISCV_ADD32 },
  { BFD_RELOC_RISCV_ADD64, R_RISCV_ADD64 },
  { BFD_RELOC_RISCV_SUB8, R_RISCV_SUB8 },
  { BFD_RELOC_RISCV_SUB16, R_RISCV_SUB16 },
  { BFD_RELOC_RISCV_SUB32, R_RISCV_SUB32 },
  { BFD_RELOC_RISCV_SUB64, R_RISCV_SUB64 },
  { BFD_RELOC_CTOR, R_RISCV_64 },
  { BFD_RELOC_12_PCREL, R_RISCV_BRANCH },
  { BFD_RELOC_RISCV_HI20, R_RISCV_HI20 },
  { BFD_RELOC_RISCV_LO12_I, R_RISCV_LO12_I },
  { BFD_RELOC_RISCV_LO12_S, R_RISCV_LO12_S },
  { BFD_RELOC_RISCV_PCREL_LO12_I, R_RISCV_PCREL_LO12_I },
  { BFD_RELOC_RISCV_PCREL_LO12_S, R_RISCV_PCREL_LO12_S },
  { BFD_RELOC_RISCV_CALL, R_RISCV_CALL },
  { BFD_RELOC_RISCV_CALL_PLT, R_RISCV_CALL_PLT },
  { BFD_RELOC_RISCV_PCREL_HI20, R_RISCV_PCREL_HI20 },
  { BFD_RELOC_RISCV_JMP, R_RISCV_JAL },
  { BFD_RELOC_RISCV_GOT_HI20, R_RISCV_GOT_HI20 },
  { BFD_RELOC_RISCV_TLS_DTPMOD32, R_RISCV_TLS_DTPMOD32 },
  { BFD_RELOC_RISCV_TLS_DTPREL32, R_RISCV_TLS_DTPREL32 },
  { BFD_RELOC_RISCV_TLS_DTPMOD64, R_RISCV_TLS_DTPMOD64 },
  { BFD_RELOC_RISCV_TLS_DTPREL64, R_RISCV_TLS_DTPREL64 },
  { BFD_RELOC_RISCV_TLS_TPREL32, R_RISCV_TLS_TPREL32 },
  { BFD_RELOC_RISCV_TLS_TPREL64, R_RISCV_TLS_TPREL64 },
  { BFD_RELOC_RISCV_TPREL_HI20, R_RISCV_TPREL_HI20 },
  { BFD_RELOC_RISCV_TPREL_ADD, R_RISCV_TPREL_ADD },
  { BFD_RELOC_RISCV_TPREL_LO12_S, R_RISCV_TPREL_LO12_S },
  { BFD_RELOC_RISCV_TPREL_LO12_I, R_RISCV_TPREL_LO12_I },
  { BFD_RELOC_RISCV_TLS_GOT_HI20, R_RISCV_TLS_GOT_HI20 },
  { BFD_RELOC_RISCV_TLS_GD_HI20, R_RISCV_TLS_GD_HI20 },
  { BFD_RELOC_RISCV_ALIGN, R_RISCV_ALIGN },
  { BFD_RELOC_RISCV_RVC_BRANCH, R_RISCV_RVC_BRANCH },
  { BFD_RELOC_RISCV_RVC_JUMP, R_RISCV_RVC_JUMP },
  { BFD_RELOC_RISCV_RVC_LUI, R_RISCV_RVC_LUI },
  { BFD_RELOC_RISCV_GPREL_I, R_RISCV_GPREL_I },
  { BFD_RELOC_RISCV_GPREL_S, R_RISCV_GPREL_S },
  { BFD_RELOC_RISCV_TPREL_I, R_RISCV_TPREL_I },
  { BFD_RELOC_RISCV_TPREL_S, R_RISCV_TPREL_S },
  { BFD_RELOC_RISCV_RELAX, R_RISCV_RELAX },
  { BFD_RELOC_RISCV_SUB6, R_RISCV_SUB6 },
  { BFD_RELOC_RISCV_SET6, R_RISCV_SET6 },
  { BFD_RELOC_RISCV_SET8, R_RISCV_SET8 },
  { BFD_RELOC_RISCV_SET16, R_RISCV_SET16 },
  { BFD_RELOC_RISCV_SET32, R_RISCV_SET32 },
  { BFD_RELOC_RISCV_32_PCREL, R_RISCV_32_PCREL },
  { BFD_RELOC_RISCV_SET_ULEB128, R_RISCV_SET_ULEB128 },
  { BFD_RELOC_RISCV_SUB_ULEB128, R_RISCV_SUB_ULEB128 },
};

/* Given a BFD reloc type, return a howto structure.  */

reloc_howto_type *
riscv_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (riscv_reloc_map); i++)
    if (riscv_reloc_map[i].bfd_val == code)
      return &howto_table[(int) riscv_reloc_map[i].elf_val];

  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

reloc_howto_type *
riscv_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (howto_table); i++)
    if (howto_table[i].name && strcasecmp (howto_table[i].name, r_name) == 0)
      return &howto_table[i];

  return NULL;
}

reloc_howto_type *
riscv_elf_rtype_to_howto (bfd *abfd, unsigned int r_type)
{
  if (r_type >= ARRAY_SIZE (howto_table))
    {
      (*_bfd_error_handler) (_("%pB: unsupported relocation type %#x"),
			     abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
  return &howto_table[r_type];
}

/* Special_function of RISCV_ADD and RISCV_SUB relocations.  */

static bfd_reloc_status_type
riscv_elf_add_sub_reloc (bfd *abfd,
			 arelent *reloc_entry,
			 asymbol *symbol,
			 void *data,
			 asection *input_section,
			 bfd *output_bfd,
			 char **error_message ATTRIBUTE_UNUSED)
{
  reloc_howto_type *howto = reloc_entry->howto;
  bfd_vma relocation;

  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    return bfd_reloc_continue;

  relocation = symbol->value + symbol->section->output_section->vma
    + symbol->section->output_offset + reloc_entry->addend;

  bfd_size_type octets = reloc_entry->address
    * bfd_octets_per_byte (abfd, input_section);
  if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd,
				  input_section, octets))
    return bfd_reloc_outofrange;

  bfd_vma old_value = bfd_get (howto->bitsize, abfd,
			       data + reloc_entry->address);

  switch (howto->type)
    {
    case R_RISCV_ADD8:
    case R_RISCV_ADD16:
    case R_RISCV_ADD32:
    case R_RISCV_ADD64:
      relocation = old_value + relocation;
      break;
    case R_RISCV_SUB6:
      relocation = (old_value & ~howto->dst_mask)
		   | (((old_value & howto->dst_mask) - relocation)
		      & howto->dst_mask);
      break;
    case R_RISCV_SUB8:
    case R_RISCV_SUB16:
    case R_RISCV_SUB32:
    case R_RISCV_SUB64:
      relocation = old_value - relocation;
      break;
    }
  bfd_put (howto->bitsize, abfd, relocation, data + reloc_entry->address);

  return bfd_reloc_ok;
}

/* Special handler for relocations which don't have to be relocated.
   This function just simply return bfd_reloc_ok.  */

static bfd_reloc_status_type
riscv_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			arelent *reloc_entry,
			asymbol *symbol ATTRIBUTE_UNUSED,
			void *data ATTRIBUTE_UNUSED,
			asection *input_section,
			bfd *output_bfd,
			char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

/* Always add the IMPLICIT for the SUBSET.  */

static bool
check_implicit_always (const char *implicit ATTRIBUTE_UNUSED,
		       riscv_subset_t *subset ATTRIBUTE_UNUSED)
{
  return true;
}

/* Add the IMPLICIT only when the version of SUBSET less than 2.1.  */

static bool
check_implicit_for_i (const char *implicit ATTRIBUTE_UNUSED,
		      riscv_subset_t *subset)
{
  return (subset->major_version < 2
	  || (subset->major_version == 2
	      && subset->minor_version < 1));
}

/* Record all implicit information for the subsets.  */
struct riscv_implicit_subset
{
  const char *subset_name;
  const char *implicit_name;
  /* A function to determine if we need to add the implicit subset.  */
  bool (*check_func) (const char *, riscv_subset_t *);
};
static struct riscv_implicit_subset riscv_implicit_subsets[] =
{
  {"e", "i",		check_implicit_always},
  {"i", "zicsr",	check_implicit_for_i},
  {"i", "zifencei",	check_implicit_for_i},
  {"g", "i",		check_implicit_always},
  {"g", "m",		check_implicit_always},
  {"g", "a",		check_implicit_always},
  {"g", "f",		check_implicit_always},
  {"g", "d",		check_implicit_always},
  {"g", "zicsr",	check_implicit_always},
  {"g", "zifencei",	check_implicit_always},
  {"m", "zmmul",	check_implicit_always},
  {"h", "zicsr",	check_implicit_always},
  {"q", "d",		check_implicit_always},
  {"v", "d",		check_implicit_always},
  {"v", "zve64d",	check_implicit_always},
  {"v", "zvl128b",	check_implicit_always},
  {"zve64d", "d",	check_implicit_always},
  {"zve64d", "zve64f",	check_implicit_always},
  {"zve64f", "zve32f",	check_implicit_always},
  {"zve64f", "zve64x",	check_implicit_always},
  {"zve64f", "zvl64b",	check_implicit_always},
  {"zve32f", "f",	check_implicit_always},
  {"zve32f", "zvl32b",	check_implicit_always},
  {"zve32f", "zve32x",	check_implicit_always},
  {"zve64x", "zve32x",	check_implicit_always},
  {"zve64x", "zvl64b",	check_implicit_always},
  {"zve32x", "zvl32b",	check_implicit_always},
  {"zvl65536b", "zvl32768b",	check_implicit_always},
  {"zvl32768b", "zvl16384b",	check_implicit_always},
  {"zvl16384b", "zvl8192b",	check_implicit_always},
  {"zvl8192b", "zvl4096b",	check_implicit_always},
  {"zvl4096b", "zvl2048b",	check_implicit_always},
  {"zvl2048b", "zvl1024b",	check_implicit_always},
  {"zvl1024b", "zvl512b",	check_implicit_always},
  {"zvl512b", "zvl256b",	check_implicit_always},
  {"zvl256b", "zvl128b",	check_implicit_always},
  {"zvl128b", "zvl64b",		check_implicit_always},
  {"zvl64b", "zvl32b",		check_implicit_always},
  {"zfa", "f",		check_implicit_always},
  {"d", "f",		check_implicit_always},
  {"zfh", "zfhmin",	check_implicit_always},
  {"zfhmin", "f",	check_implicit_always},
  {"f", "zicsr",	check_implicit_always},
  {"zqinx", "zdinx",	check_implicit_always},
  {"zdinx", "zfinx",	check_implicit_always},
  {"zhinx", "zhinxmin",	check_implicit_always},
  {"zhinxmin", "zfinx",	check_implicit_always},
  {"zfinx", "zicsr",	check_implicit_always},
  {"zk", "zkn",		check_implicit_always},
  {"zk", "zkr",		check_implicit_always},
  {"zk", "zkt",		check_implicit_always},
  {"zkn", "zbkb",	check_implicit_always},
  {"zkn", "zbkc",	check_implicit_always},
  {"zkn", "zbkx",	check_implicit_always},
  {"zkn", "zkne",	check_implicit_always},
  {"zkn", "zknd",	check_implicit_always},
  {"zkn", "zknh",	check_implicit_always},
  {"zks", "zbkb",	check_implicit_always},
  {"zks", "zbkc",	check_implicit_always},
  {"zks", "zbkx",	check_implicit_always},
  {"zks", "zksed",	check_implicit_always},
  {"zks", "zksh",	check_implicit_always},
  {"zvkn", "zvkned",	check_implicit_always},
  {"zvkn", "zvknha",	check_implicit_always},
  {"zvkn", "zvknhb",	check_implicit_always},
  {"zvkn", "zvbb",	check_implicit_always},
  {"zvkn", "zvkt",	check_implicit_always},
  {"zvkng", "zvkn",	check_implicit_always},
  {"zvkng", "zvkg",	check_implicit_always},
  {"zvknc", "zvkn",	check_implicit_always},
  {"zvknc", "zvbc",	check_implicit_always},
  {"zvks", "zvksed",	check_implicit_always},
  {"zvks", "zvksh",	check_implicit_always},
  {"zvks", "zvbb",	check_implicit_always},
  {"zvks", "zvkt",	check_implicit_always},
  {"zvksg", "zvks",	check_implicit_always},
  {"zvksg", "zvkg",	check_implicit_always},
  {"zvksc", "zvks",	check_implicit_always},
  {"zvksc", "zvbc",	check_implicit_always},
  {"smaia", "ssaia",		check_implicit_always},
  {"smstateen", "ssstateen",	check_implicit_always},
  {"smepmp", "zicsr",		check_implicit_always},
  {"ssaia", "zicsr",		check_implicit_always},
  {"sscofpmf", "zicsr",		check_implicit_always},
  {"ssstateen", "zicsr",	check_implicit_always},
  {"sstc", "zicsr",		check_implicit_always},
  {NULL, NULL, NULL}
};

/* For default_enable field, decide if the extension should
   be enbaled by default.  */

#define EXT_DEFAULT   0x1

/* List all extensions that binutils should know about.  */

struct riscv_supported_ext
{
  const char *name;
  enum riscv_spec_class isa_spec_class;
  int major_version;
  int minor_version;
  unsigned long default_enable;
};

/* The standard extensions must be added in canonical order.  */

static struct riscv_supported_ext riscv_supported_std_ext[] =
{
  {"e",		ISA_SPEC_CLASS_20191213,	1, 9, 0 },
  {"e",		ISA_SPEC_CLASS_20190608,	1, 9, 0 },
  {"e",		ISA_SPEC_CLASS_2P2,		1, 9, 0 },
  {"i",		ISA_SPEC_CLASS_20191213,	2, 1, 0 },
  {"i",		ISA_SPEC_CLASS_20190608,	2, 1, 0 },
  {"i",		ISA_SPEC_CLASS_2P2,		2, 0, 0 },
  /* The g is a special case which we don't want to output it,
     but still need it when adding implicit extensions.  */
  {"g",		ISA_SPEC_CLASS_NONE, RISCV_UNKNOWN_VERSION, RISCV_UNKNOWN_VERSION, EXT_DEFAULT },
  {"m",		ISA_SPEC_CLASS_20191213,	2, 0, 0 },
  {"m",		ISA_SPEC_CLASS_20190608,	2, 0, 0 },
  {"m",		ISA_SPEC_CLASS_2P2,		2, 0, 0 },
  {"a",		ISA_SPEC_CLASS_20191213,	2, 1, 0 },
  {"a",		ISA_SPEC_CLASS_20190608,	2, 0, 0 },
  {"a",		ISA_SPEC_CLASS_2P2,		2, 0, 0 },
  {"f",		ISA_SPEC_CLASS_20191213,	2, 2, 0 },
  {"f",		ISA_SPEC_CLASS_20190608,	2, 2, 0 },
  {"f",		ISA_SPEC_CLASS_2P2,		2, 0, 0 },
  {"d",		ISA_SPEC_CLASS_20191213,	2, 2, 0 },
  {"d",		ISA_SPEC_CLASS_20190608,	2, 2, 0 },
  {"d",		ISA_SPEC_CLASS_2P2,		2, 0, 0 },
  {"q",		ISA_SPEC_CLASS_20191213,	2, 2, 0 },
  {"q",		ISA_SPEC_CLASS_20190608,	2, 2, 0 },
  {"q",		ISA_SPEC_CLASS_2P2,		2, 0, 0 },
  {"c",		ISA_SPEC_CLASS_20191213,	2, 0, 0 },
  {"c",		ISA_SPEC_CLASS_20190608,	2, 0, 0 },
  {"c",		ISA_SPEC_CLASS_2P2,		2, 0, 0 },
  {"v",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"h",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {NULL, 0, 0, 0, 0}
};

static struct riscv_supported_ext riscv_supported_std_z_ext[] =
{
  {"zicbom",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zicbop",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zicboz",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zicond",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zicsr",		ISA_SPEC_CLASS_20191213,	2, 0,  0 },
  {"zicsr",		ISA_SPEC_CLASS_20190608,	2, 0,  0 },
  {"zifencei",		ISA_SPEC_CLASS_20191213,	2, 0,  0 },
  {"zifencei",		ISA_SPEC_CLASS_20190608,	2, 0,  0 },
  {"zihintpause",	ISA_SPEC_CLASS_DRAFT,		2, 0,  0 },
  {"zmmul",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zawrs",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zfa",		ISA_SPEC_CLASS_DRAFT,		0, 1,  0 },
  {"zfh",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zfhmin",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zfinx",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zdinx",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zqinx",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zhinx",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zhinxmin",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zbb",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zba",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zbc",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zbs",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zbkb",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zbkc",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zbkx",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zk",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zkn",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zknd",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zkne",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zknh",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zkr",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zks",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zksed",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zksh",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zkt",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zve32x",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zve32f",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zve32d",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zve64x",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zve64f",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zve64d",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvbb",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvbc",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvkg",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvkn",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvkng",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvknc",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvkned",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvknha",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvknhb",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvksed",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvksh",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvks",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvksg",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvksc",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvkt",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl32b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl64b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl128b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl256b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl512b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl1024b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl2048b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl4096b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl8192b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl16384b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl32768b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"zvl65536b",		ISA_SPEC_CLASS_DRAFT,		1, 0,  0 },
  {"ztso",		ISA_SPEC_CLASS_DRAFT,		0, 1,  0 },
  {NULL, 0, 0, 0, 0}
};

static struct riscv_supported_ext riscv_supported_std_s_ext[] =
{
  {"smaia",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"smepmp",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"smstateen",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"ssaia",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"sscofpmf",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"ssstateen",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"sstc",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"svinval",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"svnapot",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {"svpbmt",		ISA_SPEC_CLASS_DRAFT,		1, 0, 0 },
  {NULL, 0, 0, 0, 0}
};

static struct riscv_supported_ext riscv_supported_std_zxm_ext[] =
{
  {NULL, 0, 0, 0, 0}
};

static struct riscv_supported_ext riscv_supported_vendor_x_ext[] =
{
  {"xtheadba",		ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadbb",		ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadbs",		ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadcmo",		ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadcondmov",	ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadfmemidx",	ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadfmv",		ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadint",		ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadmac",		ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadmemidx",	ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadmempair",	ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {"xtheadsync",	ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  /* XVentanaCondOps: https://github.com/ventanamicro/ventana-custom-extensions/releases/download/v1.0.0/ventana-custom-extensions-v1.0.0.pdf */
  {"xventanacondops",	ISA_SPEC_CLASS_DRAFT,	1, 0, 0 },
  {NULL, 0, 0, 0, 0}
};

const struct riscv_supported_ext *riscv_all_supported_ext[] =
{
  riscv_supported_std_ext,
  riscv_supported_std_z_ext,
  riscv_supported_std_s_ext,
  riscv_supported_std_zxm_ext,
  riscv_supported_vendor_x_ext,
  NULL
};

/* ISA extension prefixed name class.  Must define them in parsing order.  */
enum riscv_prefix_ext_class
{
  RV_ISA_CLASS_Z = 1,
  RV_ISA_CLASS_S,
  RV_ISA_CLASS_ZXM,
  RV_ISA_CLASS_X,
  RV_ISA_CLASS_SINGLE
};

/* Record the strings of the prefixed extensions, and their corresponding
   classes.  The more letters of the prefix string, the more forward it must
   be defined.  Otherwise, the riscv_get_prefix_class will map it to the
   wrong classes.  */
struct riscv_parse_prefix_config
{
  /* Class of the extension. */
  enum riscv_prefix_ext_class class;

  /* Prefix string for error printing and internal parser usage.  */
  const char *prefix;
};
static const struct riscv_parse_prefix_config parse_config[] =
{
  {RV_ISA_CLASS_ZXM, "zxm"},
  {RV_ISA_CLASS_Z, "z"},
  {RV_ISA_CLASS_S, "s"},
  {RV_ISA_CLASS_X, "x"},
  {RV_ISA_CLASS_SINGLE, NULL}
};

/* Get the prefixed name class for the extensions, the class also
   means the order of the prefixed extensions.  */

static enum riscv_prefix_ext_class
riscv_get_prefix_class (const char *arch)
{
  int i = 0;
  while (parse_config[i].class != RV_ISA_CLASS_SINGLE)
    {
      if (strncmp (arch, parse_config[i].prefix,
		   strlen (parse_config[i].prefix)) == 0)
	return parse_config[i].class;
      i++;
    }
  return RV_ISA_CLASS_SINGLE;
}

/* Check KNOWN_EXTS to see if the EXT is supported.  */

static bool
riscv_known_prefixed_ext (const char *ext,
			  struct riscv_supported_ext *known_exts)
{
  size_t i;
  for (i = 0; known_exts[i].name != NULL; ++i)
    if (strcmp (ext, known_exts[i].name) == 0)
      return true;
  return false;
}

/* Check whether the prefixed extension is recognized or not.  Return
   true if recognized, otehrwise return false.  */

static bool
riscv_recognized_prefixed_ext (const char *ext)
{
  enum riscv_prefix_ext_class class = riscv_get_prefix_class (ext);
  switch (class)
  {
  case RV_ISA_CLASS_Z:
    return riscv_known_prefixed_ext (ext, riscv_supported_std_z_ext);
  case RV_ISA_CLASS_ZXM:
    return riscv_known_prefixed_ext (ext, riscv_supported_std_zxm_ext);
  case RV_ISA_CLASS_S:
    return riscv_known_prefixed_ext (ext, riscv_supported_std_s_ext);
  case RV_ISA_CLASS_X:
    /* Only the single x is unrecognized.  */
    if (strcmp (ext, "x") != 0)
      return true;
  default:
    break;
  }
  return false;
}

/* Canonical order for single letter extensions.  */
static const char riscv_ext_canonical_order[] = "eigmafdqlcbkjtpvnh";

/* Array is used to compare the orders of standard extensions quickly.  */
static int riscv_ext_order[26] = {0};

/* Init the riscv_ext_order array.  */

static void
riscv_init_ext_order (void)
{
  static bool inited = false;
  if (inited)
    return;

  /* The orders of all standard extensions are positive.  */
  int order = 1;

  for (const char *ext = &riscv_ext_canonical_order[0]; *ext; ++ext)
    riscv_ext_order[(*ext - 'a')] = order++;

  /* Some of the prefixed keyword are not single letter, so we set
     their prefixed orders in the riscv_compare_subsets directly,
     not through the riscv_ext_order.  */

  inited = true;
}

/* Similar to the strcmp.  It returns an integer less than, equal to,
   or greater than zero if `subset2` is found, respectively, to be less
   than, to match, or be greater than `subset1`.

   The order values,
   Zero: Preserved keywords.
   Positive number: Standard extensions.
   Negative number: Prefixed keywords.  */

int
riscv_compare_subsets (const char *subset1, const char *subset2)
{
  int order1 = riscv_ext_order[(*subset1 - 'a')];
  int order2 = riscv_ext_order[(*subset2 - 'a')];

  /* Compare the standard extension first.  */
  if (order1 > 0 && order2 > 0)
    return order1 - order2;

  /* Set the prefixed orders to negative numbers.  */
  enum riscv_prefix_ext_class class1 = riscv_get_prefix_class (subset1);
  enum riscv_prefix_ext_class class2 = riscv_get_prefix_class (subset2);

  if (class1 != RV_ISA_CLASS_SINGLE)
    order1 = - (int) class1;
  if (class2 != RV_ISA_CLASS_SINGLE)
    order2 = - (int) class2;

  if (order1 == order2)
    {
      /* Compare the standard addition z extensions.  */
      if (class1 == RV_ISA_CLASS_Z)
	{
	  order1 = riscv_ext_order[(*++subset1 - 'a')];
	  order2 = riscv_ext_order[(*++subset2 - 'a')];
	  if (order1 != order2)
	    return order1 - order2;
	}
      return strcasecmp (++subset1, ++subset2);
    }

  return order2 - order1;
}

/* Find subset in the list.  Return TRUE and set `current` to the subset
   if it is found.  Otherwise, return FALSE and set `current` to the place
   where we should insert the subset.  However, return FALSE with the NULL
   `current` means we should insert the subset at the head of subset list,
   if needed.  */

bool
riscv_lookup_subset (const riscv_subset_list_t *subset_list,
		     const char *subset,
		     riscv_subset_t **current)
{
  riscv_subset_t *s, *pre_s = NULL;

  /* If the subset is added in order, then just add it at the tail.  */
  if (subset_list->tail != NULL
      && riscv_compare_subsets (subset_list->tail->name, subset) < 0)
    {
      *current = subset_list->tail;
      return false;
    }

  for (s = subset_list->head;
       s != NULL;
       pre_s = s, s = s->next)
    {
      int cmp = riscv_compare_subsets (s->name, subset);
      if (cmp == 0)
	{
	  *current = s;
	  return true;
	}
      else if (cmp > 0)
	break;
    }
  *current = pre_s;

  return false;
}

/* Add the extension to the subset list.  Search the
   list first, and then find the right place to add.  */

void
riscv_add_subset (riscv_subset_list_t *subset_list,
		  const char *subset,
		  int major,
		  int minor)
{
  riscv_subset_t *current, *new;

  if (riscv_lookup_subset (subset_list, subset, &current))
    return;

  new = xmalloc (sizeof *new);
  new->name = xstrdup (subset);
  new->major_version = major;
  new->minor_version = minor;
  new->next = NULL;

  if (current != NULL)
    {
      new->next = current->next;
      current->next = new;
    }
  else
    {
      new->next = subset_list->head;
      subset_list->head = new;
    }

  if (new->next == NULL)
    subset_list->tail = new;
}

/* Get the default versions from the riscv_supported_*ext tables.  */

static void
riscv_get_default_ext_version (enum riscv_spec_class *default_isa_spec,
			       const char *name,
			       int *major_version,
			       int *minor_version)
{
  if (name == NULL
      || default_isa_spec == NULL
      || *default_isa_spec == ISA_SPEC_CLASS_NONE)
    return;

  struct riscv_supported_ext *table = NULL;
  enum riscv_prefix_ext_class class = riscv_get_prefix_class (name);
  switch (class)
    {
    case RV_ISA_CLASS_ZXM: table = riscv_supported_std_zxm_ext; break;
    case RV_ISA_CLASS_Z: table = riscv_supported_std_z_ext; break;
    case RV_ISA_CLASS_S: table = riscv_supported_std_s_ext; break;
    case RV_ISA_CLASS_X: table = riscv_supported_vendor_x_ext; break;
    default:
      table = riscv_supported_std_ext;
    }

  int i = 0;
  while (table != NULL && table[i].name != NULL)
    {
      if (strcmp (table[i].name, name) == 0
	  && (table[i].isa_spec_class == ISA_SPEC_CLASS_DRAFT
	      || table[i].isa_spec_class == *default_isa_spec))
	{
	  *major_version = table[i].major_version;
	  *minor_version = table[i].minor_version;
	  return;
	}
      i++;
    }
}

/* Find the default versions for the extension before adding them to
   the subset list, if their versions are RISCV_UNKNOWN_VERSION.
   Afterwards, report errors if we can not find their default versions.  */

static void
riscv_parse_add_subset (riscv_parse_subset_t *rps,
			const char *subset,
			int major,
			int minor,
			bool implicit)
{
  int major_version = major;
  int minor_version = minor;

  if (major_version == RISCV_UNKNOWN_VERSION
       || minor_version == RISCV_UNKNOWN_VERSION)
    riscv_get_default_ext_version (rps->isa_spec, subset,
				   &major_version, &minor_version);

  /* We don't care the versions of the implicit extensions.  */
  if (!implicit
      && (major_version == RISCV_UNKNOWN_VERSION
	  || minor_version == RISCV_UNKNOWN_VERSION))
    {
      if (subset[0] == 'x')
	rps->error_handler
	  (_("x ISA extension `%s' must be set with the versions"),
	   subset);
      /* Allow old ISA spec can recognize zicsr and zifencei.  */
      else if (strcmp (subset, "zicsr") != 0
	       && strcmp (subset, "zifencei") != 0)
	rps->error_handler
	  (_("cannot find default versions of the ISA extension `%s'"),
	   subset);
      return;
    }

  riscv_add_subset (rps->subset_list, subset,
		    major_version, minor_version);
}

/* Release subset list.  */

void
riscv_release_subset_list (riscv_subset_list_t *subset_list)
{
   while (subset_list->head != NULL)
    {
      riscv_subset_t *next = subset_list->head->next;
      free ((void *)subset_list->head->name);
      free (subset_list->head);
      subset_list->head = next;
    }

  subset_list->tail = NULL;

  if (subset_list->arch_str != NULL)
    {
      free ((void*) subset_list->arch_str);
      subset_list->arch_str = NULL;
    }
}

/* Parsing extension version.

   Return Value:
     Points to the end of version

   Arguments:
     `p`: Curent parsing position.
     `major_version`: Parsed major version.
     `minor_version`: Parsed minor version.  */

static const char *
riscv_parsing_subset_version (const char *p,
			      int *major_version,
			      int *minor_version)
{
  bool major_p = true;
  int version = 0;
  char np;

  *major_version = 0;
  *minor_version = 0;
  for (; *p; ++p)
    {
      if (*p == 'p')
	{
	  np = *(p + 1);

	  /* Might be beginning of `p` extension.  */
	  if (!ISDIGIT (np))
	    break;

	  *major_version = version;
	  major_p = false;
	  version = 0;
	}
      else if (ISDIGIT (*p))
	version = (version * 10) + (*p - '0');
      else
	break;
    }

  if (major_p)
    *major_version = version;
  else
    *minor_version = version;

  /* We can not find any version in string.  */
  if (*major_version == 0 && *minor_version == 0)
    {
      *major_version = RISCV_UNKNOWN_VERSION;
      *minor_version = RISCV_UNKNOWN_VERSION;
    }

  return p;
}

/* Parsing function for both standard and prefixed extensions.

   Return Value:
     Points to the end of extensions.

   Arguments:
     `rps`: Hooks and status for parsing extensions.
     `arch`: Full ISA string.
     `p`: Curent parsing position.  */

static const char *
riscv_parse_extensions (riscv_parse_subset_t *rps,
			const char *arch,
			const char *p)
{
  /* First letter must start with i, e or g.  */
  if (*p != 'e' && *p != 'i' && *p != 'g')
    {
      rps->error_handler
	(_("%s: first ISA extension must be `e', `i' or `g'"),
	 arch);
      return NULL;
    }

  while (*p != '\0')
    {
      if (*p == '_')
	{
	  p++;
	  continue;
	}

      char *subset = xstrdup (p);
      char *q = subset;	/* Start of version.  */
      const char *end_of_version;
      bool implicit = false;

      enum riscv_prefix_ext_class class = riscv_get_prefix_class (p);
      if (class == RV_ISA_CLASS_SINGLE)
	{
	  if (riscv_ext_order[(*subset - 'a')] == 0)
	    {
	      rps->error_handler
		(_("%s: unknown standard ISA extension or prefix class `%c'"),
		 arch, *subset);
	      free (subset);
	      return NULL;
	    }
	  q++;
	}
      else
	{
	  /* Extract the whole prefixed extension by '_'.  */
	  while (*++q != '\0' && *q != '_')
	    ;
	  /* Look forward to the first letter which is not <major>p<minor>.  */
	  bool find_any_version = false;
	  bool find_minor_version = false;
	  while (1)
	    {
	      q--;
	      if (ISDIGIT (*q))
		find_any_version = true;
	      else if (find_any_version
		       && !find_minor_version
		       && *q == 'p'
		       && ISDIGIT (*(q - 1)))
	      find_minor_version = true;
	      else
		break;
	    }
	  q++;

	  /* Check if the end of extension is 'p' or not.  If yes, then
	     the second letter from the end cannot be number.  */
	  if (*(q - 1) == 'p' && ISDIGIT (*(q - 2)))
	    {
	      *q = '\0';
	      rps->error_handler
		(_("%s: invalid prefixed ISA extension `%s' ends with <number>p"),
		 arch, subset);
	      free (subset);
	      return NULL;
	    }
	}

      int major_version = RISCV_UNKNOWN_VERSION;
      int minor_version = RISCV_UNKNOWN_VERSION;
      end_of_version =
	riscv_parsing_subset_version (q, &major_version, &minor_version);
      *q = '\0';
      if (end_of_version == NULL)
	{
	  free (subset);
	  return NULL;
	}

      /* Check if the prefixed extension name is well-formed.  */
      if (class != RV_ISA_CLASS_SINGLE
	  && rps->check_unknown_prefixed_ext
	  && !riscv_recognized_prefixed_ext (subset))
	{
	  rps->error_handler
	    (_("%s: unknown prefixed ISA extension `%s'"),
	     arch, subset);
	  free (subset);
	  return NULL;
	}

      /* Added g as an implicit extension.  */
      if (class == RV_ISA_CLASS_SINGLE
	  && strcmp (subset, "g") == 0)
	{
	  implicit = true;
	  major_version = RISCV_UNKNOWN_VERSION;
	  minor_version = RISCV_UNKNOWN_VERSION;
	}
      riscv_parse_add_subset (rps, subset,
			      major_version,
			      minor_version, implicit);
      p += end_of_version - subset;
      free (subset);

      if (class != RV_ISA_CLASS_SINGLE
	  && *p != '\0' && *p != '_')
	{
	  rps->error_handler
	    (_("%s: prefixed ISA extension must separate with _"),
	     arch);
	  return NULL;
	}
    }

  return p;
}

/* Add the implicit extensions.  */

static void
riscv_parse_add_implicit_subsets (riscv_parse_subset_t *rps)
{
  struct riscv_implicit_subset *t = riscv_implicit_subsets;
  bool finished = false;
  while (!finished)
    {
      finished = true;
      for (; t->subset_name; t++)
	{
	  riscv_subset_t *subset = NULL;
	  riscv_subset_t *implicit_subset = NULL;
	  if (riscv_lookup_subset (rps->subset_list, t->subset_name, &subset)
	      && !riscv_lookup_subset (rps->subset_list, t->implicit_name,
				       &implicit_subset)
	      && t->check_func (t->implicit_name, subset))
	    {
	      riscv_parse_add_subset (rps, t->implicit_name,
				      RISCV_UNKNOWN_VERSION,
				      RISCV_UNKNOWN_VERSION, true);

	      /* Restart the loop and pick up any new implications.  */
	      finished = false;
	      t = riscv_implicit_subsets;
	      break;
	    }
	}
    }
}

/* Check extensions conflicts.  */

static bool
riscv_parse_check_conflicts (riscv_parse_subset_t *rps)
{
  riscv_subset_t *subset = NULL;
  int xlen = *rps->xlen;
  bool no_conflict = true;

  if (riscv_lookup_subset (rps->subset_list, "e", &subset)
      && xlen > 32)
    {
      rps->error_handler
	(_("rv%d does not support the `e' extension"), xlen);
      no_conflict = false;
    }
  if (riscv_lookup_subset (rps->subset_list, "q", &subset)
      && (subset->major_version < 2 || (subset->major_version == 2
					&& subset->minor_version < 2))
      && xlen < 64)
    {
      rps->error_handler (_("rv%d does not support the `q' extension"), xlen);
      no_conflict = false;
    }
  if (riscv_lookup_subset (rps->subset_list, "zfinx", &subset)
      && riscv_lookup_subset (rps->subset_list, "f", &subset))
    {
      rps->error_handler
	(_("`zfinx' is conflict with the `f/d/q/zfh/zfhmin' extension"));
      no_conflict = false;
    }

  bool support_zve = false;
  bool support_zvl = false;
  riscv_subset_t *s = rps->subset_list->head;
  for (; s != NULL; s = s->next)
    {
      if (!support_zve
	  && strncmp (s->name, "zve", 3) == 0)
	support_zve = true;
      if (!support_zvl
	  && strncmp (s->name, "zvl", 3) == 0)
	support_zvl = true;
      if (support_zve && support_zvl)
	break;
    }
  if (support_zvl && !support_zve)
    {
      rps->error_handler
	(_("zvl*b extensions need to enable either `v' or `zve' extension"));
      no_conflict = false;
    }

  return no_conflict;
}

/* Set the default subset list according to the default_enable field
   of riscv_supported_*ext tables.  */

static void
riscv_set_default_arch (riscv_parse_subset_t *rps)
{
  unsigned long enable = EXT_DEFAULT;
  int i, j;
  for (i = 0; riscv_all_supported_ext[i] != NULL; i++)
    {
      const struct riscv_supported_ext *table = riscv_all_supported_ext[i];
      for (j = 0; table[j].name != NULL; j++)
	{
	  bool implicit = false;
	  if (strcmp (table[j].name, "g") == 0)
	    implicit = true;
	  if (table[j].default_enable & enable)
	    riscv_parse_add_subset (rps, table[j].name,
				    RISCV_UNKNOWN_VERSION,
				    RISCV_UNKNOWN_VERSION, implicit);
	}
    }
}

/* Function for parsing ISA string.

   Return Value:
     Return TRUE on success.

   Arguments:
     `rps`: Hooks and status for parsing extensions.
     `arch`: Full ISA string.  */

bool
riscv_parse_subset (riscv_parse_subset_t *rps,
		    const char *arch)
{
  const char *p;

  /* Init the riscv_ext_order array to compare the order of extensions
     quickly.  */
  riscv_init_ext_order ();

  if (arch == NULL)
    {
      riscv_set_default_arch (rps);
      riscv_parse_add_implicit_subsets (rps);
      return riscv_parse_check_conflicts (rps);
    }

  for (p = arch; *p != '\0'; p++)
    {
      if (ISUPPER (*p))
	{
	  rps->error_handler
	    (_("%s: ISA string cannot contain uppercase letters"),
	     arch);
	  return false;
	}
    }

  p = arch;
  if (startswith (p, "rv32"))
    {
      *rps->xlen = 32;
      p += 4;
    }
  else if (startswith (p, "rv64"))
    {
      *rps->xlen = 64;
      p += 4;
    }
  else
    {
      /* ISA string shouldn't be NULL or empty here.  For linker,
	 it might be empty when we failed to merge the ISA string
	 in the riscv_merge_attributes.  For assembler, we might
	 give an empty string by .attribute arch, "" or -march=.
	 However, We have already issued the correct error message
	 in another side, so do not issue this error when the ISA
	 string is empty.  */
      if (strlen (arch))
	rps->error_handler (
	  _("%s: ISA string must begin with rv32 or rv64"),
	  arch);
      return false;
    }

  /* Parse single standard and prefixed extensions.  */
  if (riscv_parse_extensions (rps, arch, p) == NULL)
    return false;

  /* Finally add implicit extensions according to the current
     extensions.  */
  riscv_parse_add_implicit_subsets (rps);

  /* Check the conflicts.  */
  return riscv_parse_check_conflicts (rps);
}

/* Return the number of digits for the input.  */

size_t
riscv_estimate_digit (unsigned num)
{
  size_t digit = 0;
  if (num == 0)
    return 1;

  for (digit = 0; num ; num /= 10)
    digit++;

  return digit;
}

/* Auxiliary function to estimate string length of subset list.  */

static size_t
riscv_estimate_arch_strlen1 (const riscv_subset_t *subset)
{
  if (subset == NULL)
    return 6; /* For rv32/rv64/rv128 and string terminator.  */

  return riscv_estimate_arch_strlen1 (subset->next)
	 + strlen (subset->name)
	 + riscv_estimate_digit (subset->major_version)
	 + 1 /* For version seperator 'p'.  */
	 + riscv_estimate_digit (subset->minor_version)
	 + 1 /* For underscore.  */;
}

/* Estimate the string length of this subset list.  */

static size_t
riscv_estimate_arch_strlen (const riscv_subset_list_t *subset_list)
{
  return riscv_estimate_arch_strlen1 (subset_list->head);
}

/* Auxiliary function to convert subset info to string.  */

static void
riscv_arch_str1 (riscv_subset_t *subset,
		 char *attr_str, char *buf, size_t bufsz)
{
  const char *underline = "_";
  riscv_subset_t *subset_t = subset;

  if (subset_t == NULL)
    return;

  /* No underline between rvXX and i/e.  */
  if ((strcasecmp (subset_t->name, "i") == 0)
      || (strcasecmp (subset_t->name, "e") == 0))
    underline = "";

  snprintf (buf, bufsz, "%s%s%dp%d",
	    underline,
	    subset_t->name,
	    subset_t->major_version,
	    subset_t->minor_version);

  strncat (attr_str, buf, bufsz);

  /* Skip 'i' extension after 'e', or skip extensions which
     versions are unknown.  */
  while (subset_t->next
	 && ((strcmp (subset_t->name, "e") == 0
	      && strcmp (subset_t->next->name, "i") == 0)
	     || subset_t->next->major_version == RISCV_UNKNOWN_VERSION
	     || subset_t->next->minor_version == RISCV_UNKNOWN_VERSION))
    subset_t = subset_t->next;

  riscv_arch_str1 (subset_t->next, attr_str, buf, bufsz);
}

/* Convert subset information into string with explicit versions.  */

char *
riscv_arch_str (unsigned xlen, const riscv_subset_list_t *subset)
{
  size_t arch_str_len = riscv_estimate_arch_strlen (subset);
  char *attr_str = xmalloc (arch_str_len);
  char *buf = xmalloc (arch_str_len);

  snprintf (attr_str, arch_str_len, "rv%u", xlen);

  riscv_arch_str1 (subset->head, attr_str, buf, arch_str_len);
  free (buf);

  return attr_str;
}

/* Copy the subset in the subset list.  */

static struct riscv_subset_t *
riscv_copy_subset (riscv_subset_list_t *subset_list,
		   riscv_subset_t *subset)
{
  if (subset == NULL)
    return NULL;

  riscv_subset_t *new = xmalloc (sizeof *new);
  new->name = xstrdup (subset->name);
  new->major_version = subset->major_version;
  new->minor_version = subset->minor_version;
  new->next = riscv_copy_subset (subset_list, subset->next);

  if (subset->next == NULL)
    subset_list->tail = new;

  return new;
}

/* Copy the subset list.  */

riscv_subset_list_t *
riscv_copy_subset_list (riscv_subset_list_t *subset_list)
{
  riscv_subset_list_t *new = xmalloc (sizeof *new);
  new->head = riscv_copy_subset (new, subset_list->head);
  new->arch_str = strdup (subset_list->arch_str);
  return new;
}

/* Remove the SUBSET from the subset list.  */

static void
riscv_remove_subset (riscv_subset_list_t *subset_list,
		     const char *subset)
{
  riscv_subset_t *current = subset_list->head;
  riscv_subset_t *pre = NULL;
  for (; current != NULL; pre = current, current = current->next)
    {
      if (strcmp (current->name, subset) == 0)
	{
	  if (pre == NULL)
	    subset_list->head = current->next;
	  else
	    pre->next = current->next;
	  if (current->next == NULL)
	    subset_list->tail = pre;
	  free ((void *) current->name);
	  free (current);
	  break;
	}
    }
}

/* Add/Remove an extension to/from the subset list.  This is used for
   the .option rvc or norvc, and .option arch directives.  */

bool
riscv_update_subset (riscv_parse_subset_t *rps,
		     const char *str)
{
  const char *p = str;

  do
    {
      int major_version = RISCV_UNKNOWN_VERSION;
      int minor_version = RISCV_UNKNOWN_VERSION;

      bool removed = false;
      switch (*p)
	{
	case '+': removed = false; break;
	case '-': removed = true; break;
	default:
	  riscv_release_subset_list (rps->subset_list);
	  return riscv_parse_subset (rps, p);
	}
      ++p;

      char *subset = xstrdup (p);
      char *q = subset;
      const char *end_of_version;
      /* Extract the whole prefixed extension by ','.  */
      while (*q != '\0' && *q != ',')
        q++;

      /* Look forward to the first letter which is not <major>p<minor>.  */
      bool find_any_version = false;
      bool find_minor_version = false;
      size_t len = q - subset;
      size_t i;
      for (i = len; i > 0; i--)
        {
	  q--;
	  if (ISDIGIT (*q))
	    find_any_version = true;
	  else if (find_any_version
		   && !find_minor_version
		   && *q == 'p'
		   && ISDIGIT (*(q - 1)))
	    find_minor_version = true;
	  else
	    break;
	}
      if (len > 0)
	q++;

      /* Check if the end of extension is 'p' or not.  If yes, then
	 the second letter from the end cannot be number.  */
      if (len > 1 && *(q - 1) == 'p' && ISDIGIT (*(q - 2)))
	{
	  *q = '\0';
	  rps->error_handler
	    (_("invalid ISA extension ends with <number>p "
	       "in .option arch `%s'"), str);
	  free (subset);
	  return false;
	}

      end_of_version =
	riscv_parsing_subset_version (q, &major_version, &minor_version);
      *q = '\0';
      if (end_of_version == NULL)
	{
	  free (subset);
	  return false;
	}

      if (strlen (subset) == 0
	  || (strlen (subset) == 1
	      && riscv_ext_order[(*subset - 'a')] == 0)
	  || (strlen (subset) > 1
	      && rps->check_unknown_prefixed_ext
	      && !riscv_recognized_prefixed_ext (subset)))
	{
	  rps->error_handler
	    (_("unknown ISA extension `%s' in .option arch `%s'"),
	     subset, str);
	  free (subset);
	  return false;
	}

      if (strcmp (subset, "i") == 0
	  || strcmp (subset, "e") == 0
	  || strcmp (subset, "g") == 0)
	{
	  rps->error_handler
	    (_("cannot + or - base extension `%s' in .option "
	       "arch `%s'"), subset, str);
	  free (subset);
	  return false;
	}

      if (removed)
	riscv_remove_subset (rps->subset_list, subset);
      else
	riscv_parse_add_subset (rps, subset, major_version, minor_version, true);
      p += end_of_version - subset;
      free (subset);
    }
  while (*p++ == ',');

  riscv_parse_add_implicit_subsets (rps);
  return riscv_parse_check_conflicts (rps);
}

/* Check if the FEATURE subset is supported or not in the subset list.
   Return true if it is supported; Otherwise, return false.  */

bool
riscv_subset_supports (riscv_parse_subset_t *rps,
		       const char *feature)
{
  struct riscv_subset_t *subset;
  return riscv_lookup_subset (rps->subset_list, feature, &subset);
}

/* Each instuction is belonged to an instruction class INSN_CLASS_*.
   Call riscv_subset_supports to make sure if the instuction is valid.  */

bool
riscv_multi_subset_supports (riscv_parse_subset_t *rps,
			     enum riscv_insn_class insn_class)
{
  switch (insn_class)
    {
    case INSN_CLASS_I:
      return riscv_subset_supports (rps, "i");
    case INSN_CLASS_ZICBOM:
      return riscv_subset_supports (rps, "zicbom");
    case INSN_CLASS_ZICBOP:
      return riscv_subset_supports (rps, "zicbop");
    case INSN_CLASS_ZICBOZ:
      return riscv_subset_supports (rps, "zicboz");
    case INSN_CLASS_ZICOND:
      return riscv_subset_supports (rps, "zicond");
    case INSN_CLASS_ZICSR:
      return riscv_subset_supports (rps, "zicsr");
    case INSN_CLASS_ZIFENCEI:
      return riscv_subset_supports (rps, "zifencei");
    case INSN_CLASS_ZIHINTPAUSE:
      return riscv_subset_supports (rps, "zihintpause");
    case INSN_CLASS_M:
      return riscv_subset_supports (rps, "m");
    case INSN_CLASS_ZMMUL:
      return riscv_subset_supports (rps, "zmmul");
    case INSN_CLASS_A:
      return riscv_subset_supports (rps, "a");
    case INSN_CLASS_ZAWRS:
      return riscv_subset_supports (rps, "zawrs");
    case INSN_CLASS_F:
      return riscv_subset_supports (rps, "f");
    case INSN_CLASS_D:
      return riscv_subset_supports (rps, "d");
    case INSN_CLASS_Q:
      return riscv_subset_supports (rps, "q");
    case INSN_CLASS_C:
      return riscv_subset_supports (rps, "c");
    case INSN_CLASS_F_AND_C:
      return (riscv_subset_supports (rps, "f")
	      && riscv_subset_supports (rps, "c"));
    case INSN_CLASS_D_AND_C:
      return (riscv_subset_supports (rps, "d")
	      && riscv_subset_supports (rps, "c"));
    case INSN_CLASS_F_INX:
      return (riscv_subset_supports (rps, "f")
	      || riscv_subset_supports (rps, "zfinx"));
    case INSN_CLASS_D_INX:
      return (riscv_subset_supports (rps, "d")
	      || riscv_subset_supports (rps, "zdinx"));
    case INSN_CLASS_Q_INX:
      return (riscv_subset_supports (rps, "q")
	      || riscv_subset_supports (rps, "zqinx"));
    case INSN_CLASS_ZFH_INX:
      return (riscv_subset_supports (rps, "zfh")
	      || riscv_subset_supports (rps, "zhinx"));
    case INSN_CLASS_ZFHMIN:
      return riscv_subset_supports (rps, "zfhmin");
    case INSN_CLASS_ZFHMIN_INX:
      return (riscv_subset_supports (rps, "zfhmin")
	      || riscv_subset_supports (rps, "zhinxmin"));
    case INSN_CLASS_ZFHMIN_AND_D_INX:
      return ((riscv_subset_supports (rps, "zfhmin")
	       && riscv_subset_supports (rps, "d"))
	      || (riscv_subset_supports (rps, "zhinxmin")
		  && riscv_subset_supports (rps, "zdinx")));
    case INSN_CLASS_ZFHMIN_AND_Q_INX:
      return ((riscv_subset_supports (rps, "zfhmin")
	       && riscv_subset_supports (rps, "q"))
	      || (riscv_subset_supports (rps, "zhinxmin")
		  && riscv_subset_supports (rps, "zqinx")));
    case INSN_CLASS_ZFA:
      return riscv_subset_supports (rps, "zfa");
    case INSN_CLASS_D_AND_ZFA:
      return riscv_subset_supports (rps, "d")
	     && riscv_subset_supports (rps, "zfa");
    case INSN_CLASS_Q_AND_ZFA:
      return riscv_subset_supports (rps, "q")
	     && riscv_subset_supports (rps, "zfa");
    case INSN_CLASS_ZFH_AND_ZFA:
      return riscv_subset_supports (rps, "zfh")
	     && riscv_subset_supports (rps, "zfa");
    case INSN_CLASS_ZBA:
      return riscv_subset_supports (rps, "zba");
    case INSN_CLASS_ZBB:
      return riscv_subset_supports (rps, "zbb");
    case INSN_CLASS_ZBC:
      return riscv_subset_supports (rps, "zbc");
    case INSN_CLASS_ZBS:
      return riscv_subset_supports (rps, "zbs");
    case INSN_CLASS_ZBKB:
      return riscv_subset_supports (rps, "zbkb");
    case INSN_CLASS_ZBKC:
      return riscv_subset_supports (rps, "zbkc");
    case INSN_CLASS_ZBKX:
      return riscv_subset_supports (rps, "zbkx");
    case INSN_CLASS_ZBB_OR_ZBKB:
      return (riscv_subset_supports (rps, "zbb")
	      || riscv_subset_supports (rps, "zbkb"));
    case INSN_CLASS_ZBC_OR_ZBKC:
      return (riscv_subset_supports (rps, "zbc")
	      || riscv_subset_supports (rps, "zbkc"));
    case INSN_CLASS_ZKND:
      return riscv_subset_supports (rps, "zknd");
    case INSN_CLASS_ZKNE:
      return riscv_subset_supports (rps, "zkne");
    case INSN_CLASS_ZKNH:
      return riscv_subset_supports (rps, "zknh");
    case INSN_CLASS_ZKND_OR_ZKNE:
      return (riscv_subset_supports (rps, "zknd")
	      || riscv_subset_supports (rps, "zkne"));
    case INSN_CLASS_ZKSED:
      return riscv_subset_supports (rps, "zksed");
    case INSN_CLASS_ZKSH:
      return riscv_subset_supports (rps, "zksh");
    case INSN_CLASS_V:
      return (riscv_subset_supports (rps, "v")
	      || riscv_subset_supports (rps, "zve64x")
	      || riscv_subset_supports (rps, "zve32x"));
    case INSN_CLASS_ZVEF:
      return (riscv_subset_supports (rps, "v")
	      || riscv_subset_supports (rps, "zve64d")
	      || riscv_subset_supports (rps, "zve64f")
	      || riscv_subset_supports (rps, "zve32f"));
    case INSN_CLASS_ZVBB:
      return riscv_subset_supports (rps, "zvbb");
    case INSN_CLASS_ZVBC:
      return riscv_subset_supports (rps, "zvbc");
    case INSN_CLASS_ZVKG:
      return riscv_subset_supports (rps, "zvkg");
    case INSN_CLASS_ZVKNED:
      return riscv_subset_supports (rps, "zvkned");
    case INSN_CLASS_ZVKNHA:
      return riscv_subset_supports (rps, "zvknha");
    case INSN_CLASS_ZVKNHB:
      return riscv_subset_supports (rps, "zvknhb");
    case INSN_CLASS_ZVKNHA_OR_ZVKNHB:
      return (riscv_subset_supports (rps, "zvknha")
	      || riscv_subset_supports (rps, "zvknhb"));
    case INSN_CLASS_ZVKSED:
      return riscv_subset_supports (rps, "zvksed");
    case INSN_CLASS_ZVKSH:
      return riscv_subset_supports (rps, "zvksh");
    case INSN_CLASS_SVINVAL:
      return riscv_subset_supports (rps, "svinval");
    case INSN_CLASS_H:
      return riscv_subset_supports (rps, "h");
    case INSN_CLASS_XTHEADBA:
      return riscv_subset_supports (rps, "xtheadba");
    case INSN_CLASS_XTHEADBB:
      return riscv_subset_supports (rps, "xtheadbb");
    case INSN_CLASS_XTHEADBS:
      return riscv_subset_supports (rps, "xtheadbs");
    case INSN_CLASS_XTHEADCMO:
      return riscv_subset_supports (rps, "xtheadcmo");
    case INSN_CLASS_XTHEADCONDMOV:
      return riscv_subset_supports (rps, "xtheadcondmov");
    case INSN_CLASS_XTHEADFMEMIDX:
      return riscv_subset_supports (rps, "xtheadfmemidx");
    case INSN_CLASS_XTHEADFMV:
      return riscv_subset_supports (rps, "xtheadfmv");
    case INSN_CLASS_XTHEADINT:
      return riscv_subset_supports (rps, "xtheadint");
    case INSN_CLASS_XTHEADMAC:
      return riscv_subset_supports (rps, "xtheadmac");
    case INSN_CLASS_XTHEADMEMIDX:
      return riscv_subset_supports (rps, "xtheadmemidx");
    case INSN_CLASS_XTHEADMEMPAIR:
      return riscv_subset_supports (rps, "xtheadmempair");
    case INSN_CLASS_XTHEADSYNC:
      return riscv_subset_supports (rps, "xtheadsync");
    case INSN_CLASS_XVENTANACONDOPS:
      return riscv_subset_supports (rps, "xventanacondops");
    default:
      rps->error_handler
        (_("internal: unreachable INSN_CLASS_*"));
      return false;
    }
}

/* Each instuction is belonged to an instruction class INSN_CLASS_*.
   Call riscv_subset_supports_ext to determine the missing extension.  */

const char *
riscv_multi_subset_supports_ext (riscv_parse_subset_t *rps,
				 enum riscv_insn_class insn_class)
{
  switch (insn_class)
    {
    case INSN_CLASS_I:
      return "i";
    case INSN_CLASS_ZICBOM:
      return "zicbom";
    case INSN_CLASS_ZICBOP:
      return "zicbop";
    case INSN_CLASS_ZICBOZ:
      return "zicboz";
    case INSN_CLASS_ZICOND:
      return "zicond";
    case INSN_CLASS_ZICSR:
      return "zicsr";
    case INSN_CLASS_ZIFENCEI:
      return "zifencei";
    case INSN_CLASS_ZIHINTPAUSE:
      return "zihintpause";
    case INSN_CLASS_M:
      return "m";
    case INSN_CLASS_ZMMUL:
      return _ ("m' or `zmmul");
    case INSN_CLASS_A:
      return "a";
    case INSN_CLASS_ZAWRS:
      return "zawrs";
    case INSN_CLASS_F:
      return "f";
    case INSN_CLASS_D:
      return "d";
    case INSN_CLASS_Q:
      return "q";
    case INSN_CLASS_C:
      return "c";
    case INSN_CLASS_F_AND_C:
      if (!riscv_subset_supports (rps, "f")
	  && !riscv_subset_supports (rps, "c"))
	return _("f' and `c");
      else if (!riscv_subset_supports (rps, "f"))
	return "f";
      else
	return "c";
    case INSN_CLASS_D_AND_C:
      if (!riscv_subset_supports (rps, "d")
	  && !riscv_subset_supports (rps, "c"))
	return _("d' and `c");
      else if (!riscv_subset_supports (rps, "d"))
	return "d";
      else
	return "c";
    case INSN_CLASS_F_INX:
      return _("f' or `zfinx");
    case INSN_CLASS_D_INX:
      return _("d' or `zdinx");
    case INSN_CLASS_Q_INX:
      return _("q' or `zqinx");
    case INSN_CLASS_ZFH_INX:
      return _("zfh' or `zhinx");
    case INSN_CLASS_ZFHMIN:
      return "zfhmin";
    case INSN_CLASS_ZFHMIN_INX:
      return _("zfhmin' or `zhinxmin");
    case INSN_CLASS_ZFHMIN_AND_D_INX:
      if (riscv_subset_supports (rps, "zfhmin"))
	return "d";
      else if (riscv_subset_supports (rps, "d"))
	return "zfhmin";
      else if (riscv_subset_supports (rps, "zhinxmin"))
	return "zdinx";
      else if (riscv_subset_supports (rps, "zdinx"))
	return "zhinxmin";
      else
	return _("zfhmin' and `d', or `zhinxmin' and `zdinx");
    case INSN_CLASS_ZFHMIN_AND_Q_INX:
      if (riscv_subset_supports (rps, "zfhmin"))
	return "q";
      else if (riscv_subset_supports (rps, "q"))
	return "zfhmin";
      else if (riscv_subset_supports (rps, "zhinxmin"))
	return "zqinx";
      else if (riscv_subset_supports (rps, "zqinx"))
	return "zhinxmin";
      else
	return _("zfhmin' and `q', or `zhinxmin' and `zqinx");
    case INSN_CLASS_ZFA:
      return "zfa";
    case INSN_CLASS_D_AND_ZFA:
      if (!riscv_subset_supports (rps, "d")
	  && !riscv_subset_supports (rps, "zfa"))
	return _("d' and `zfa");
      else if (!riscv_subset_supports (rps, "d"))
	return "d";
      else
	return "zfa";
    case INSN_CLASS_Q_AND_ZFA:
      if (!riscv_subset_supports (rps, "q")
	  && !riscv_subset_supports (rps, "zfa"))
	return _("q' and `zfa");
      else if (!riscv_subset_supports (rps, "q"))
	return "q";
      else
	return "zfa";
    case INSN_CLASS_ZFH_AND_ZFA:
      if (!riscv_subset_supports (rps, "zfh")
	  && !riscv_subset_supports (rps, "zfa"))
	return _("zfh' and `zfa");
      else if (!riscv_subset_supports (rps, "zfh"))
	return "zfh";
      else
	return "zfa";
    case INSN_CLASS_ZBA:
      return "zba";
    case INSN_CLASS_ZBB:
      return "zbb";
    case INSN_CLASS_ZBC:
      return "zbc";
    case INSN_CLASS_ZBS:
      return "zbs";
    case INSN_CLASS_ZBKB:
      return "zbkb";
    case INSN_CLASS_ZBKC:
      return "zbkc";
    case INSN_CLASS_ZBKX:
      return "zbkx";
    case INSN_CLASS_ZBB_OR_ZBKB:
      return _("zbb' or `zbkb");
    case INSN_CLASS_ZBC_OR_ZBKC:
      return _("zbc' or `zbkc");
    case INSN_CLASS_ZKND:
      return "zknd";
    case INSN_CLASS_ZKNE:
      return "zkne";
    case INSN_CLASS_ZKNH:
      return "zknh";
    case INSN_CLASS_ZKND_OR_ZKNE:
      return _("zknd' or `zkne");
    case INSN_CLASS_ZKSED:
      return "zksed";
    case INSN_CLASS_ZKSH:
      return "zksh";
    case INSN_CLASS_V:
      return _("v' or `zve64x' or `zve32x");
    case INSN_CLASS_ZVEF:
      return _("v' or `zve64d' or `zve64f' or `zve32f");
    case INSN_CLASS_ZVBB:
      return _("zvbb");
    case INSN_CLASS_ZVBC:
      return _("zvbc");
    case INSN_CLASS_ZVKG:
      return _("zvkg");
    case INSN_CLASS_ZVKNED:
      return _("zvkned");
    case INSN_CLASS_ZVKNHA:
      return _("zvknha");
    case INSN_CLASS_ZVKNHB:
      return _("zvknhb");
    case INSN_CLASS_ZVKSED:
      return _("zvksed");
    case INSN_CLASS_ZVKSH:
      return _("zvksh");
    case INSN_CLASS_SVINVAL:
      return "svinval";
    case INSN_CLASS_H:
      return _("h");
    case INSN_CLASS_XTHEADBA:
      return "xtheadba";
    case INSN_CLASS_XTHEADBB:
      return "xtheadbb";
    case INSN_CLASS_XTHEADBS:
      return "xtheadbs";
    case INSN_CLASS_XTHEADCMO:
      return "xtheadcmo";
    case INSN_CLASS_XTHEADCONDMOV:
      return "xtheadcondmov";
    case INSN_CLASS_XTHEADFMEMIDX:
      return "xtheadfmemidx";
    case INSN_CLASS_XTHEADFMV:
      return "xtheadfmv";
    case INSN_CLASS_XTHEADINT:
      return "xtheadint";
    case INSN_CLASS_XTHEADMAC:
      return "xtheadmac";
    case INSN_CLASS_XTHEADMEMIDX:
      return "xtheadmemidx";
    case INSN_CLASS_XTHEADMEMPAIR:
      return "xtheadmempair";
    case INSN_CLASS_XTHEADSYNC:
      return "xtheadsync";
    default:
      rps->error_handler
        (_("internal: unreachable INSN_CLASS_*"));
      return NULL;
    }
}
