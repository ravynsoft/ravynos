/* LoongArch-specific support for ELF.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Loongson Ltd.

   Based on RISC-V target.

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
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/loongarch.h"
#include "elfxx-loongarch.h"

#define ALL_ONES (~ (bfd_vma) 0)

typedef struct loongarch_reloc_howto_type_struct
{
  /* The first must be reloc_howto_type!  */
  reloc_howto_type howto;
  bfd_reloc_code_real_type bfd_type;
  bool (*adjust_reloc_bits)(bfd *, reloc_howto_type *, bfd_vma *);
  const char *larch_reloc_type_name;
} loongarch_reloc_howto_type;

#define LOONGARCH_DEFAULT_HOWTO(r_name)					    \
  { HOWTO (R_LARCH_##r_name, 0, 4, 32, false, 0, complain_overflow_signed,  \
	bfd_elf_generic_reloc, "R_LARCH_" #r_name, false, 0, ALL_ONES,	    \
	false), BFD_RELOC_LARCH_##r_name, NULL, NULL }

#define LOONGARCH_HOWTO(type, right, size, bits, pcrel, left, ovf, func,  \
	    name, inplace, src_mask, dst_mask, pcrel_off, btype, afunc,lname) \
  { HOWTO(type, right, size, bits, pcrel, left, ovf, func, name,	  \
	  inplace, src_mask, dst_mask, pcrel_off), btype, afunc, lname }

#define LOONGARCH_EMPTY_HOWTO(C) \
  { EMPTY_HOWTO (C), BFD_RELOC_NONE, NULL, NULL }

static bool
reloc_bits (bfd *abfd, reloc_howto_type *howto, bfd_vma *val);
static bool
reloc_sign_bits (bfd *abfd, reloc_howto_type *howto, bfd_vma *fix_val);

static bfd_reloc_status_type
loongarch_elf_add_sub_reloc (bfd *, arelent *, asymbol *, void *,
			      asection *, bfd *, char **);

static bfd_reloc_status_type
loongarch_elf_add_sub_reloc_uleb128 (bfd *, arelent *, asymbol *, void *,
				      asection *, bfd *, char **);

/* This does not include any relocation information, but should be
   good enough for GDB or objdump to read the file.  */
static loongarch_reloc_howto_type loongarch_howto_table[] =
{
  /* No relocation.  */
    LOONGARCH_HOWTO (R_LARCH_NONE,	  /* type (0).  */
	 0,				  /* rightshift */
	 0,				  /* size */
	 0,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_NONE",		  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 0,				  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_NONE,		  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  /* 32 bit relocation.  */
  LOONGARCH_HOWTO (R_LARCH_32,		  /* type (1).  */
	 0,				  /* rightshift */
	 4,				  /* size */
	 32,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_32",			  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_32,			  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  /* 64 bit relocation.  */
  LOONGARCH_HOWTO (R_LARCH_64,		  /* type (2).  */
	 0,				  /* rightshift */
	 8,				  /* size */
	 64,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_64",			  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_64,			  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_RELATIVE,	  /* type (3).  */
	 0,				  /* rightshift */
	 4,				  /* size */
	 32,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_RELATIVE",		  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_NONE,		  /* undefined?  */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_COPY,	  /* type (4).  */
	 0,				  /* rightshift */
	 0,				  /* this one is variable size */
	 0,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_bitfield,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_COPY",		  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 0,				  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_NONE,		  /* undefined?  */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_JUMP_SLOT,	  /* type (5).  */
	 0,				  /* rightshift */
	 8,				  /* size */
	 64,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_bitfield,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_JUMP_SLOT",		  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 0,				  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_NONE,		  /* undefined?  */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  /* Dynamic TLS relocations.  */
  LOONGARCH_HOWTO (R_LARCH_TLS_DTPMOD32,  /* type (6).  */
	 0,				  /* rightshift */
	 4,				  /* size */
	 32,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_TLS_DTPMOD32",	  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_DTPMOD32,	  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_DTPMOD64,  /* type (7).  */
	 0,				  /* rightshift */
	 8,				  /* size */
	 64,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_TLS_DTPMOD64",	  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_DTPMOD64,	  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_DTPREL32,  /* type (8). */
	 0,				  /* rightshift */
	 4,				  /* size */
	 32,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_TLS_DTPREL32",	  /* name */
	 true,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_DTPREL32,	  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_DTPREL64,  /* type (9).  */
	 0,				  /* rightshift */
	 8,				  /* size */
	 64,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_TLS_DTPREL64",	  /* name */
	 true,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_DTPREL64,	  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_TPREL32,	  /* type (10).  */
	 0,				  /* rightshift */
	 4,				  /* size */
	 32,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_TLS_TPREL32",		  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_TPREL32,	  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_TPREL64,	  /* type (11).  */
	 0,				  /* rightshift */
	 8,				  /* size */
	 64,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_TLS_TPREL64",		  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_TPREL64,	  /* bfd_reloc_code_real_type */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_IRELATIVE,	  /* type (12).  */
	 0,				  /* rightshift */
	 4,				  /* size */
	 32,				  /* bitsize */
	 false,				  /* pc_relative */
	 0,				  /* bitpos */
	 complain_overflow_dont,	  /* complain_on_overflow */
	 bfd_elf_generic_reloc,		  /* special_function */
	 "R_LARCH_IRELATIVE",		  /* name */
	 false,				  /* partial_inplace */
	 0,				  /* src_mask */
	 ALL_ONES,			  /* dst_mask */
	 false,				  /* pcrel_offset */
	 BFD_RELOC_NONE,		  /* undefined?  */
	 NULL,				  /* adjust_reloc_bits */
	 NULL),				  /* larch_reloc_type_name */

  LOONGARCH_EMPTY_HOWTO (13),
  LOONGARCH_EMPTY_HOWTO (14),
  LOONGARCH_EMPTY_HOWTO (15),
  LOONGARCH_EMPTY_HOWTO (16),
  LOONGARCH_EMPTY_HOWTO (17),
  LOONGARCH_EMPTY_HOWTO (18),
  LOONGARCH_EMPTY_HOWTO (19),

  LOONGARCH_HOWTO (R_LARCH_MARK_LA,		/* type (20).  */
	 0,					/* rightshift.  */
	 0,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_MARK_LA",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0,					/* dst_mask.  */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_MARK_LA,		/* bfd_reloc_code_real_type */
	 NULL,					/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_MARK_PCREL,		/* type (21).  */
	 0,					/* rightshift.  */
	 0,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_MARK_PCREL",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0,					/* dst_mask.  */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_MARK_PCREL,		/* bfd_reloc_code_real_type */
	 NULL,					/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_PUSH_PCREL,	/* type (22).  */
	 2,					/* rightshift.  */
	 4,					/* size.  */
	 32,					/* bitsize.  */
	 true /* FIXME: somewhat use this.  */,	/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_SOP_PUSH_PCREL",		/* name.  */
	 false,					/* partial_inplace.  */
	 0x03ffffff,				/* src_mask.  */
	 0x03ffffff,				/* dst_mask.  */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_PUSH_PCREL,	/* bfd_reloc_code_real_type */
	 NULL,					/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  /* type 23-37.  */
  LOONGARCH_DEFAULT_HOWTO (SOP_PUSH_ABSOLUTE),
  LOONGARCH_DEFAULT_HOWTO (SOP_PUSH_DUP),
  LOONGARCH_DEFAULT_HOWTO (SOP_PUSH_GPREL),
  LOONGARCH_DEFAULT_HOWTO (SOP_PUSH_TLS_TPREL),
  LOONGARCH_DEFAULT_HOWTO (SOP_PUSH_TLS_GOT),
  LOONGARCH_DEFAULT_HOWTO (SOP_PUSH_TLS_GD),
  LOONGARCH_DEFAULT_HOWTO (SOP_PUSH_PLT_PCREL),
  LOONGARCH_DEFAULT_HOWTO (SOP_ASSERT),
  LOONGARCH_DEFAULT_HOWTO (SOP_NOT),
  LOONGARCH_DEFAULT_HOWTO (SOP_SUB),
  LOONGARCH_DEFAULT_HOWTO (SOP_SL),
  LOONGARCH_DEFAULT_HOWTO (SOP_SR),
  LOONGARCH_DEFAULT_HOWTO (SOP_ADD),
  LOONGARCH_DEFAULT_HOWTO (SOP_AND),
  LOONGARCH_DEFAULT_HOWTO (SOP_IF_ELSE),

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_S_10_5,	  /* type (38).  */
	 0,					  /* rightshift.  */
	 4,					  /* size.  */
	 5,					  /* bitsize.  */
	 false,					  /* pc_relative.  */
	 10,					  /* bitpos.  */
	 complain_overflow_signed,		  /* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			  /* special_function.  */
	 "R_LARCH_SOP_POP_32_S_10_5",		  /* name.  */
	 false,					  /* partial_inplace.  */
	 0,					  /* src_mask */
	 0x7c00,				  /* dst_mask */
	 false,					  /* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_S_10_5,	  /* bfd_reloc_code_real_type */
	 reloc_bits,				  /* adjust_reloc_bits */
	 NULL),					  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_U_10_12,	  /* type (39).  */
	 0,					  /* rightshift.  */
	 4,					  /* size.  */
	 12,					  /* bitsize.  */
	 false,					  /* pc_relative.  */
	 10,					  /* bitpos.  */
	 complain_overflow_unsigned,		  /* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			  /* special_function.  */
	 "R_LARCH_SOP_POP_32_U_10_12",		  /* name.  */
	 false,					  /* partial_inplace.  */
	 0,					  /* src_mask */
	 0x3ffc00,				  /* dst_mask */
	 false,					  /* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_U_10_12,	  /* bfd_reloc_code_real_type */
	 reloc_bits,				  /* adjust_reloc_bits */
	 NULL),					  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_S_10_12,	  /* type (40).  */
	 0,					  /* rightshift.  */
	 4,					  /* size.  */
	 12,					  /* bitsize.  */
	 false,					  /* pc_relative.  */
	 10,					  /* bitpos.  */
	 complain_overflow_signed,		  /* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			  /* special_function.  */
	 "R_LARCH_SOP_POP_32_S_10_12",		  /* name.  */
	 false,					  /* partial_inplace.  */
	 0,					  /* src_mask */
	 0x3ffc00,				  /* dst_mask */
	 false,					  /* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_S_10_12,	  /* bfd_reloc_code_real_type */
	 reloc_bits,				  /* adjust_reloc_bits */
	 NULL),					  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_S_10_16,	  /* type (41).  */
	 0,					  /* rightshift.  */
	 4,					  /* size.  */
	 16,					  /* bitsize.  */
	 false,					  /* pc_relative.  */
	 10,					  /* bitpos.  */
	 complain_overflow_signed,		  /* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			  /* special_function.  */
	 "R_LARCH_SOP_POP_32_S_10_16",		  /* name.  */
	 false,					  /* partial_inplace.  */
	 0,					  /* src_mask */
	 0x3fffc00,				  /* dst_mask */
	 false,					  /* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_S_10_16,	  /* bfd_reloc_code_real_type */
	 reloc_bits,				  /* adjust_reloc_bits */
	 NULL),					  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_S_10_16_S2, /* type (42).  */
	 2,					  /* rightshift.  */
	 4,					  /* size.  */
	 16,					  /* bitsize.  */
	 false,					  /* pc_relative.  */
	 10,					  /* bitpos.  */
	 complain_overflow_signed,		  /* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			  /* special_function.  */
	 "R_LARCH_SOP_POP_32_S_10_16_S2",	  /* name.  */
	 false,					  /* partial_inplace.  */
	 0,					  /* src_mask */
	 0x3fffc00,				  /* dst_mask */
	 false,					  /* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_S_10_16_S2,	  /* bfd_reloc_code_real_type */
	 reloc_sign_bits,			  /* adjust_reloc_bits */
	 NULL),					  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_S_5_20,	  /* type (43).  */
	 0,					  /* rightshift.  */
	 4,					  /* size.  */
	 20,					  /* bitsize.  */
	 false,					  /* pc_relative.  */
	 5,					  /* bitpos.  */
	 complain_overflow_signed,		  /* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			  /* special_function.  */
	 "R_LARCH_SOP_POP_32_S_5_20",		  /* name.  */
	 false,					  /* partial_inplace.  */
	 0,					  /* src_mask */
	 0x1ffffe0,				  /* dst_mask */
	 false,					  /* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_S_5_20,	  /* bfd_reloc_code_real_type */
	 reloc_bits,				  /* adjust_reloc_bits */
	 NULL),					  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_S_0_5_10_16_S2,
						  /* type (44).  */
	 2,					  /* rightshift.  */
	 4,					  /* size.  */
	 21,					  /* bitsize.  */
	 false,					  /* pc_relative.  */
	 0,					  /* bitpos.  */
	 complain_overflow_signed,		  /* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			  /* special_function.  */
	 "R_LARCH_SOP_POP_32_S_0_5_10_16_S2",	  /* name.  */
	 false,					  /* partial_inplace.  */
	 0xfc0003e0,				  /* src_mask */
	 0xfc0003e0,				  /* dst_mask */
	 false,					  /* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_S_0_5_10_16_S2,
						  /* bfd_reloc_code_real_type */
	 reloc_sign_bits,			  /* adjust_reloc_bits */
	 NULL),					  /* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_S_0_10_10_16_S2,	/* type (45).  */
	 2,					/* rightshift.  */
	 4,					/* size.  */
	 26,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_SOP_POP_32_S_0_10_10_16_S2",	/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x03ffffff,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_S_0_10_10_16_S2,
						/* bfd_reloc_code_real_type */
	 reloc_sign_bits,			/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_SOP_POP_32_U,	/* type (46).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 32,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_unsigned,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_SOP_POP_32_S_U",		/* name.  */
	 false,					/* partial_inplace.  */
	 0xffffffff00000000,			/* src_mask */
	 0x00000000ffffffff,			/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_SOP_POP_32_U,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  /* 8-bit in-place addition, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_ADD8,		/* type (47).  */
	 0,					/* rightshift.  */
	 1,					/* size.  */
	 8,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_ADD8",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0xff,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_ADD8,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 16-bit in-place addition, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_ADD16,		/* type (48).  */
	 0,					/* rightshift.  */
	 2,					/* size.  */
	 16,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_ADD16",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0xffff,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_ADD16,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 24-bit in-place addition, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_ADD24,		/* type (49).  */
	 0,					/* rightshift.  */
	 3,					/* size.  */
	 24,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_ADD24",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0xffffff,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_ADD24,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 32-bit in-place addition, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_ADD32,		/* type (50).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 32,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_ADD32",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0xffffffff,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_ADD32,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 64-bit in-place addition, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_ADD64,		/* type (51).  */
	 0,					/* rightshift.  */
	 8,					/* size.  */
	 64,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_ADD64",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 ALL_ONES,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_ADD64,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 8-bit in-place subtraction, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_SUB8,		/* type (52).  */
	 0,					/* rightshift.  */
	 1,					/* size.  */
	 8,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_SUB8",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0xff,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_SUB8,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 16-bit in-place subtraction, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_SUB16,		/* type (53).  */
	 0,					/* rightshift.  */
	 2,					/* size.  */
	 16,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_SUB16",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0xffff,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_SUB16,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 24-bit in-place subtraction, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_SUB24,		/* type (54).  */
	 0,					/* rightshift.  */
	 3,					/* size.  */
	 24,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_SUB24",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0xffffff,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_SUB24,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 32-bit in-place subtraction, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_SUB32,		/* type (55).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 32,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_SUB32",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0xffffffff,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_SUB32,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 64-bit in-place subtraction, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_SUB64,		/* type (56).  */
	 0,					/* rightshift.  */
	 8,					/* size.  */
	 64,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_SUB64",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 ALL_ONES,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_SUB64,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  LOONGARCH_HOWTO (R_LARCH_GNU_VTINHERIT,	/* type (57).  */
	 0,					/* rightshift.  */
	 0,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GNU_VTINHERIT",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0,					/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_NONE,			/* bfd_reloc_code_real_type */
	 NULL,					/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GNU_VTENTRY,		/* type (58).  */
	 0,					/* rightshift.  */
	 0,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 NULL,					/* special_function.  */
	 "R_LARCH_GNU_VTENTRY",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0,					/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_NONE,			/* bfd_reloc_code_real_type */
	 NULL,					/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  LOONGARCH_EMPTY_HOWTO (59),
  LOONGARCH_EMPTY_HOWTO (60),
  LOONGARCH_EMPTY_HOWTO (61),
  LOONGARCH_EMPTY_HOWTO (62),
  LOONGARCH_EMPTY_HOWTO (63),

  /* New reloc types.  */
  LOONGARCH_HOWTO (R_LARCH_B16,			/* type (64).  */
	 2,					/* rightshift.  */
	 4,					/* size.  */
	 16,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_B16",				/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0x3fffc00,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_B16,			/* bfd_reloc_code_real_type.  */
	 reloc_sign_bits,			/* adjust_reloc_bits.  */
	 "b16"),				/* larch_reloc_type_name.  */

  LOONGARCH_HOWTO (R_LARCH_B21,			/* type (65).  */
	 2,					/* rightshift.  */
	 4,					/* size.  */
	 21,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_B21",				/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0x3fffc1f,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_B21,			/* bfd_reloc_code_real_type.  */
	 reloc_sign_bits,			/* adjust_reloc_bits.  */
	 "b21"),				/* larch_reloc_type_name.  */

  LOONGARCH_HOWTO (R_LARCH_B26,			/* type (66).  */
	 2,					/* rightshift.  */
	 4,					/* size.  */
	 26,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_B26",				/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0x03ffffff,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_B26,			/* bfd_reloc_code_real_type.  */
	 reloc_sign_bits,			/* adjust_reloc_bits.  */
	 "b26"),				/* larch_reloc_type_name.  */

  LOONGARCH_HOWTO (R_LARCH_ABS_HI20,		/* type (67).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_ABS_HI20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_ABS_HI20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "abs_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_ABS_LO12,		/* type (68).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_unsigned,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_ABS_LO12",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_ABS_LO12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "abs_lo12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_ABS64_LO20,		/* type (69).  */
	 32,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_ABS64_LO20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_ABS64_LO20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "abs64_lo20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_ABS64_HI12,		/* type (70).  */
	 52,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_ABS64_HI12",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_ABS64_HI12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "abs64_hi12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_PCALA_HI20,		/* type (71).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_PCALA_HI20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_PCALA_HI20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "pc_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_PCALA_LO12,		/* type (72).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_PCALA_LO12",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_PCALA_LO12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "pc_lo12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_PCALA64_LO20,	/* type (73).  */
	 32,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_PCALA64_LO20",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_PCALA64_LO20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "pc64_lo20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_PCALA64_HI12,	/* type (74).  */
	 52,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_PCALA64_HI12",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_PCALA64_HI12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "pc64_hi12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GOT_PC_HI20,		/* type (75).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GOT_PC_HI20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_GOT_PC_HI20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "got_pc_hi20"),			/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GOT_PC_LO12,		/* type (76).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GOT_PC_LO12",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_GOT_PC_LO12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "got_pc_lo12"),			/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GOT64_PC_LO20,	/* type (77).  */
	 32,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GOT64_PC_LO20",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_GOT64_PC_LO20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "got64_pc_lo20"),			/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GOT64_PC_HI12,	/* type (78).  */
	 52,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GOT64_PC_HI12",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_GOT64_PC_HI12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "got64_pc_hi12"),			/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GOT_HI20,		/* type (79).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GOT_HI20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_GOT_HI20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "got_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GOT_LO12,		/* type (80).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GOT_LO12",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_GOT_LO12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "got_lo12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GOT64_LO20,		/* type (81).  */
	 32,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GOT64_LO20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_GOT64_LO20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "got64_lo20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_GOT64_HI12,		/* type (82).  */
	 52,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_GOT64_HI12",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_GOT64_HI12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "got64_hi12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_LE_HI20,		/* type (83).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_LE_HI20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_LE_HI20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "le_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_LE_LO12,		/* type (84).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_unsigned,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_LE_LO12",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_LE_LO12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "le_lo12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_LE64_LO20,	/* type (85).  */
	 32,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_LE64_LO20",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_LE64_LO20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "le64_lo20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_LE64_HI12,	/* type (86).  */
	 52,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_LE64_HI12",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_LE64_HI12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "le64_hi12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_IE_PC_HI20,	/* type (87).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_IE_PC_HI20",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_IE_PC_HI20,	/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ie_pc_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_IE_PC_LO12,	/* type (88).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_IE_PC_LO12",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_IE_PC_LO12,	/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ie_pc_lo12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_IE64_PC_LO20,	/* type (89).  */
	 32,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_IE64_PC_LO20",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_IE64_PC_LO20,	/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ie64_pc_lo20"),			/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_IE64_PC_HI12,	/* type (90).  */
	 52,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_IE64_PC_HI12",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_IE64_PC_HI12,	/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ie64_pc_hi12"),			/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_IE_HI20,		/* type (91).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_IE_HI20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_IE_HI20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ie_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_IE_LO12,		/* type (92).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_IE_LO12",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_IE_LO12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ie_lo12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_IE64_LO20,	/* type (93).  */
	 32,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_IE64_LO20",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_IE64_LO20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ie64_lo20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_IE64_HI12,	/* type (94).  */
	 52,					/* rightshift.  */
	 4,					/* size.  */
	 12,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 10,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_IE64_HI12",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x3ffc00,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_IE64_HI12,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ie64_hi12"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_LD_PC_HI20,	/* type (95).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_LD_PC_HI20",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_LD_PC_HI20,	/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ld_pc_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_LD_HI20,		/* type (96).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_LD_HI20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_LD_HI20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "ld_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_GD_PC_HI20,	/* type (97).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_GD_PC_HI20",		/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_GD_PC_HI20,	/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "gd_pc_hi20"),				/* larch_reloc_type_name */

  LOONGARCH_HOWTO (R_LARCH_TLS_GD_HI20,		/* type (98).  */
	 12,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_TLS_GD_HI20",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0x1ffffe0,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_TLS_GD_HI20,		/* bfd_reloc_code_real_type */
	 reloc_bits,				/* adjust_reloc_bits */
	 "gd_hi20"),				/* larch_reloc_type_name */

  /* 32-bit PC relative.  */
  LOONGARCH_HOWTO (R_LARCH_32_PCREL,		/* type (99).  */
	 0,					/* rightshift.  */
	 4,					/* size.  */
	 32,					/* bitsize.  */
	 true,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_32_PCREL",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0xffffffff,				/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_32_PCREL,		/* bfd_reloc_code_real_type */
	 NULL,					/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  /* The paired relocation may be relaxed.  */
  LOONGARCH_HOWTO (R_LARCH_RELAX,		/* type (100).  */
	 0,					/* rightshift */
	 1,					/* size */
	 0,					/* bitsize */
	 false,					/* pc_relative */
	 0,					/* bitpos */
	 complain_overflow_dont,		/* complain_on_overflow */
	 bfd_elf_generic_reloc,			/* special_function */
	 "R_LARCH_RELAX",			/* name */
	 false,					/* partial_inplace */
	 0,					/* src_mask */
	 0,					/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_RELAX,			/* bfd_reloc_code_real_type */
	 NULL,					/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

  /* Delete relaxed instruction.  */
  LOONGARCH_HOWTO (R_LARCH_DELETE,		/* type (101).  */
	 0,					/* rightshift.  */
	 0,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_DELETE",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_DELETE,		/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* Indicates an alignment statement.  The addend field encodes how many
     bytes of NOPs follow the statement.  The desired alignment is the
     addend rounded up to the next power of two.  */
  LOONGARCH_HOWTO (R_LARCH_ALIGN,		/* type (102).  */
	 0,					/* rightshift.  */
	 0,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_ALIGN",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_ALIGN,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* pcala_hi20 + pcala_lo12 relaxed to pcrel20_s2.  */
  LOONGARCH_HOWTO (R_LARCH_PCREL20_S2,		/* type (103).  */
	 2,					/* rightshift.  */
	 4,					/* size.  */
	 20,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 5,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_PCREL20_S2",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0x1ffffe0,				/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_PCREL20_S2,		/* bfd_reloc_code_real_type.  */
	 reloc_sign_bits,			/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* Canonical Frame Address.  */
  LOONGARCH_HOWTO (R_LARCH_CFA,			/* type (104).  */
	 0,					/* rightshift.  */
	 0,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_CFA",				/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_CFA,			/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 6-bit in-place addition, for local label subtraction
     to calculate DW_CFA_advance_loc.  */
  LOONGARCH_HOWTO (R_LARCH_ADD6,		/* type (105).  */
	 0,					/* rightshift.  */
	 1,					/* size.  */
	 8,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_ADD6",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0x3f,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_ADD6,			/* bfd_reloc_code_real_type.  */
	 reloc_bits,				/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 6-bit in-place subtraction, for local label subtraction
     to calculate DW_CFA_advance_loc.  */
  LOONGARCH_HOWTO (R_LARCH_SUB6,		/* type (106).  */
	 0,					/* rightshift.  */
	 1,					/* size.  */
	 8,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc,		/* special_function.  */
	 "R_LARCH_SUB6",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0x3f,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_SUB6,			/* bfd_reloc_code_real_type.  */
	 reloc_bits,				/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* The length of unsigned-leb128 is variable, just assume the
     size is one byte here.
     uleb128 in-place addition, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_ADD_ULEB128,		/* type (107).  */
	 0,					/* rightshift.  */
	 1,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc_uleb128,	/* special_function.  */
	 "R_LARCH_ADD_ULEB128",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_ADD_ULEB128,		/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* The length of unsigned-leb128 is variable, just assume the
     size is one byte here.
     uleb128 in-place subtraction, for local label subtraction.  */
  LOONGARCH_HOWTO (R_LARCH_SUB_ULEB128,		/* type (108).  */
	 0,					/* rightshift.  */
	 1,					/* size.  */
	 0,					/* bitsize.  */
	 false,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_dont,		/* complain_on_overflow.  */
	 loongarch_elf_add_sub_reloc_uleb128,	/* special_function.  */
	 "R_LARCH_SUB_ULEB128",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask.  */
	 0,					/* dst_mask.  */
	 false,					/* pcrel_offset.  */
	 BFD_RELOC_LARCH_SUB_ULEB128,		/* bfd_reloc_code_real_type.  */
	 NULL,					/* adjust_reloc_bits.  */
	 NULL),					/* larch_reloc_type_name.  */

  /* 64-bit PC relative.  */
  LOONGARCH_HOWTO (R_LARCH_64_PCREL,		/* type (109).  */
	 0,					/* rightshift.  */
	 8,					/* size.  */
	 64,					/* bitsize.  */
	 true,					/* pc_relative.  */
	 0,					/* bitpos.  */
	 complain_overflow_signed,		/* complain_on_overflow.  */
	 bfd_elf_generic_reloc,			/* special_function.  */
	 "R_LARCH_64_PCREL",			/* name.  */
	 false,					/* partial_inplace.  */
	 0,					/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 false,					/* pcrel_offset */
	 BFD_RELOC_LARCH_64_PCREL,		/* bfd_reloc_code_real_type */
	 NULL,					/* adjust_reloc_bits */
	 NULL),					/* larch_reloc_type_name */

};

reloc_howto_type *
loongarch_elf_rtype_to_howto (bfd *abfd, unsigned int r_type)
{
  if(r_type < R_LARCH_count)
    {
      /* For search table fast.  */
      BFD_ASSERT (ARRAY_SIZE (loongarch_howto_table) == R_LARCH_count);

      if (loongarch_howto_table[r_type].howto.type == r_type)
	return (reloc_howto_type *)&loongarch_howto_table[r_type];

      for (size_t i = 0; i < ARRAY_SIZE (loongarch_howto_table); i++)
	if (loongarch_howto_table[i].howto.type == r_type)
	  return (reloc_howto_type *)&loongarch_howto_table[i];
    }

  (*_bfd_error_handler) (_("%pB: unsupported relocation type %#x"),
			 abfd, r_type);
  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

reloc_howto_type *
loongarch_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  BFD_ASSERT (ARRAY_SIZE (loongarch_howto_table) == R_LARCH_count);

  for (size_t i = 0; i < ARRAY_SIZE (loongarch_howto_table); i++)
    if (loongarch_howto_table[i].howto.name
	&& strcasecmp (loongarch_howto_table[i].howto.name, r_name) == 0)
      return (reloc_howto_type *)&loongarch_howto_table[i];

  (*_bfd_error_handler) (_("%pB: unsupported relocation type %s"),
			 abfd, r_name);
  bfd_set_error (bfd_error_bad_value);

  return NULL;
}

/* Cost so much.  */
reloc_howto_type *
loongarch_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			     bfd_reloc_code_real_type code)
{
  BFD_ASSERT (ARRAY_SIZE (loongarch_howto_table) == R_LARCH_count);

  /* Fast search for new reloc types.  */
  if (BFD_RELOC_LARCH_B16 <= code && code < BFD_RELOC_LARCH_RELAX)
    {
      BFD_ASSERT (BFD_RELOC_LARCH_RELAX - BFD_RELOC_LARCH_B16
		  == R_LARCH_RELAX - R_LARCH_B16);
      loongarch_reloc_howto_type *ht = NULL;
      ht = &loongarch_howto_table[code - BFD_RELOC_LARCH_B16 + R_LARCH_B16];
      BFD_ASSERT (ht->bfd_type == code);
      return (reloc_howto_type *)ht;
    }

  for (size_t i = 0; i < ARRAY_SIZE (loongarch_howto_table); i++)
    if (loongarch_howto_table[i].bfd_type == code)
      return (reloc_howto_type *)&loongarch_howto_table[i];

  (*_bfd_error_handler) (_("%pB: unsupported bfd relocation type %#x"),
			 abfd, code);
  bfd_set_error (bfd_error_bad_value);

  return NULL;
}

bfd_reloc_code_real_type
loongarch_larch_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				   const char *l_r_name)
{
  for (size_t i = 0; i < ARRAY_SIZE (loongarch_howto_table); i++)
    {
      loongarch_reloc_howto_type *lht = &loongarch_howto_table[i];
      if ((NULL != lht->larch_reloc_type_name)
	  && (0 == strcmp (lht->larch_reloc_type_name, l_r_name)))
	return lht->bfd_type;
    }

  (*_bfd_error_handler) (_("%pB: unsupported relocation type name %s"),
			 abfd, l_r_name);
  bfd_set_error (bfd_error_bad_value);
  return BFD_RELOC_NONE;
}


/* Functions for reloc bits field.
   1.  Signed extend *fix_val.
   2.  Return false if overflow.  */

#define LARCH_RELOC_BFD_VMA_BIT_MASK(bitsize) \
  (~((((bfd_vma)0x1) << (bitsize)) - 1))

/* Adjust val to perform insn
   BFD_RELOC_LARCH_SOP_POP_32_S_10_5
   BFD_RELOC_LARCH_SOP_POP_32_S_10_12
   BFD_RELOC_LARCH_SOP_POP_32_U_10_12
   BFD_RELOC_LARCH_SOP_POP_32_S_10_16
   BFD_RELOC_LARCH_SOP_POP_32_S_5_20
   BFD_RELOC_LARCH_SOP_POP_32_U.  */

static bool
reloc_bits (bfd *abfd ATTRIBUTE_UNUSED,
	    reloc_howto_type *howto,
	    bfd_vma *fix_val)
{
  bfd_signed_vma val = (bfd_signed_vma)(*fix_val);
  bfd_signed_vma mask = ((bfd_signed_vma)0x1 << howto->bitsize) - 1;

  val = val >> howto->rightshift;

  /* Perform insn bits field.  */
  val = val & mask;
  val <<= howto->bitpos;

  *fix_val = (bfd_vma)val;

  return true;
}

static bool
reloc_sign_bits (bfd *abfd, reloc_howto_type *howto, bfd_vma *fix_val)
{
  if (howto->complain_on_overflow != complain_overflow_signed)
    return false;

  bfd_signed_vma val = (bfd_signed_vma)(*fix_val);

  /* Check alignment. FIXME: if rightshift is not alingment.  */
  if (howto->rightshift
      && (val & ((((bfd_signed_vma) 1) << howto->rightshift) - 1)))
    {
      (*_bfd_error_handler) (_("%pB: relocation %s right shift %d error 0x%lx"),
			     abfd, howto->name, howto->rightshift, (long) val);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  bfd_signed_vma mask = ((bfd_signed_vma)0x1 << (howto->bitsize
			  + howto->rightshift - 1)) - 1;

  /* Positive number: high part is all 0;
     Negative number: if high part is not all 0, high part must be all 1.
     high part: from sign bit to highest bit.  */
  if ((val & ~mask) && ((val & ~mask) != ~mask))
    {
      (*_bfd_error_handler) (_("%pB: relocation %s overflow 0x%lx"),
			     abfd, howto->name, (long) val);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  val = val >> howto->rightshift;
  /* can delete? */
  mask = ((bfd_signed_vma)0x1 << howto->bitsize) - 1;
  val = val & mask;

  switch (howto->type)
    {
    case R_LARCH_SOP_POP_32_S_0_10_10_16_S2:
    case R_LARCH_B26:
      /* Perform insn bits field.  25:16>>16, 15:0<<10.  */
      val = ((val & 0xffff) << 10) | ((val >> 16) & 0x3ff);
      break;
    case R_LARCH_B21:
      val = ((val & 0xffff) << 10) | ((val >> 16) & 0x1f);
      break;
    default:
      val <<= howto->bitpos;
      break;
    }

  *fix_val = val;
  return true;
}

bool
loongarch_adjust_reloc_bitsfield (bfd *abfd, reloc_howto_type *howto,
				  bfd_vma *fix_val)
{
  BFD_ASSERT (((loongarch_reloc_howto_type *)howto)->adjust_reloc_bits);
  return ((loongarch_reloc_howto_type *)
	  howto)->adjust_reloc_bits (abfd, howto, fix_val);
}

static bfd_reloc_status_type
loongarch_elf_add_sub_reloc (bfd *abfd,
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
    case R_LARCH_ADD6:
    case R_LARCH_ADD8:
    case R_LARCH_ADD16:
    case R_LARCH_ADD32:
    case R_LARCH_ADD64:
      relocation = old_value + relocation;
      break;

    case R_LARCH_SUB6:
    case R_LARCH_SUB8:
    case R_LARCH_SUB16:
    case R_LARCH_SUB32:
    case R_LARCH_SUB64:
      relocation = old_value - relocation;
      break;
    }

  bfd_put (howto->bitsize, abfd, relocation, data + reloc_entry->address);

  return bfd_reloc_ok;
}

static bfd_reloc_status_type
loongarch_elf_add_sub_reloc_uleb128 (bfd *abfd,
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

  unsigned int len = 0;
  bfd_byte *p = data + reloc_entry->address;
  bfd_vma old_value = _bfd_read_unsigned_leb128 (abfd, p, &len);

  switch (howto->type)
    {
    case R_LARCH_ADD_ULEB128:
      relocation = old_value + relocation;
      break;

    case R_LARCH_SUB_ULEB128:
      relocation = old_value - relocation;
      break;
    }

  bfd_vma mask = (1 << (7 * len)) - 1;
  relocation = relocation & mask;
  loongarch_write_unsigned_leb128 (p, len, relocation);
  return bfd_reloc_ok;
}

/* Write VALUE in uleb128 format to P.
   LEN is the uleb128 value length.
   Return a pointer to the byte following the last byte that was written.  */
bfd_byte *
loongarch_write_unsigned_leb128 (bfd_byte *p, unsigned int len, bfd_vma value)
{
  bfd_byte c;
  do
    {
      c = value & 0x7f;
      if (len > 1)
	c |= 0x80;
      *(p++) = c;
      value >>= 7;
      len--;
    }
  while (len);
  return p;
}

int loongarch_get_uleb128_length (bfd_byte *buf)
{
  unsigned int len = 0;
  _bfd_read_unsigned_leb128 (NULL, buf, &len);
  return len;
}
