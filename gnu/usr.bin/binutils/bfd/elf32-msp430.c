/*  MSP430-specific support for 32-bit ELF
    Copyright (C) 2002-2023 Free Software Foundation, Inc.
    Contributed by Dmitry Diky <diwil@mail.ru>

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
#include "libiberty.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/msp430.h"

static bool debug_relocs = 0;

/* All users of this file have bfd_octets_per_byte (abfd, sec) == 1.  */
#define OCTETS_PER_BYTE(ABFD, SEC) 1

static bfd_reloc_status_type
rl78_sym_diff_handler (bfd * abfd,
		       arelent * reloc,
		       asymbol * sym ATTRIBUTE_UNUSED,
		       void * addr ATTRIBUTE_UNUSED,
		       asection * input_sec,
		       bfd * out_bfd ATTRIBUTE_UNUSED,
		       char ** error_message ATTRIBUTE_UNUSED)
{
  bfd_size_type octets;
  octets = reloc->address * OCTETS_PER_BYTE (abfd, input_sec);

  /* Catch the case where bfd_install_relocation would return
     bfd_reloc_outofrange because the SYM_DIFF reloc is being used in a very
     small section.  It does not actually matter if this happens because all
     that SYM_DIFF does is compute a (4-byte) value.  A second reloc then uses
     this value, and it is that reloc that must fit into the section.

     This happens in eg, gcc/testsuite/gcc.c-torture/compile/labels-3.c.  */
  if ((octets + bfd_get_reloc_size (reloc->howto))
      > bfd_get_section_limit_octets (abfd, input_sec))
    return bfd_reloc_ok;
  return bfd_reloc_continue;
}

/* Special handler for relocations which don't have to be relocated.
   This function just simply returns bfd_reloc_ok.  */
static bfd_reloc_status_type
msp430_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED, arelent *reloc_entry,
			asymbol *symbol ATTRIBUTE_UNUSED,
			void *data ATTRIBUTE_UNUSED, asection *input_section,
			bfd *output_bfd, char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}

static reloc_howto_type elf_msp430_howto_table[] =
{
  HOWTO (R_MSP430_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_MSP430_32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_32",		/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 10 bit PC relative relocation.  */
  HOWTO (R_MSP430_10_PCREL,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 10,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_10_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0x3ff,			/* src_mask */
	 0x3ff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_MSP430_16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_16",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit PC relative relocation for command address.  */
  HOWTO (R_MSP430_16_PCREL,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_16_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit absolute relocation, byte operations.  */
  HOWTO (R_MSP430_16_BYTE,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_16_BYTE",	/* name */
	 false,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit absolute relocation for command address.  */
  HOWTO (R_MSP430_16_PCREL_BYTE,/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_16_PCREL_BYTE",/* name */
	 false,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 10 bit PC relative relocation for complicated polymorphs.  */
  HOWTO (R_MSP430_2X_PCREL,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 10,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_2X_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0x3ff,			/* src_mask */
	 0x3ff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit relaxable relocation for command address.  */
  HOWTO (R_MSP430_RL_PCREL,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_RL_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true)			/* pcrel_offset */

  /* A 8-bit absolute relocation.  */
  , HOWTO (R_MSP430_8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_8",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Together with a following reloc, allows for the difference
     between two symbols to be the real addend of the second reloc.  */
  HOWTO (R_MSP430_SYM_DIFF,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 rl78_sym_diff_handler,	/* special handler.  */
	 "R_MSP430_SYM_DIFF",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* The length of unsigned-leb128 is variable, just assume the
     size is one byte here.  */
  HOWTO (R_MSP430_GNU_SET_ULEB128,	/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 msp430_elf_ignore_reloc,	/* special handler.  */
	 "R_MSP430_GNU_SET_ULEB128",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* The length of unsigned-leb128 is variable, just assume the
     size is one byte here.  */
  HOWTO (R_MSP430_GNU_SUB_ULEB128,	/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 msp430_elf_ignore_reloc,	/* special handler.  */
	 "R_MSP430_GNU_SUB_ULEB128",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

};

static reloc_howto_type elf_msp430x_howto_table[] =
{
  HOWTO (R_MSP430_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_MSP430_ABS32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_ABS32",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_MSP430_ABS16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_ABS16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_MSP430_ABS8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_ABS8",	/* name */
	 false,			/* partial_inplace */
	 0xff,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_MSP430_PCR16,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_PCR16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_PCR20_EXT_SRC,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_PCR20_EXT_SRC",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_PCR20_EXT_DST,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_PCR20_EXT_DST",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_PCR20_EXT_ODST,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_PCR20_EXT_ODST",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_ABS20_EXT_SRC,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_ABS20_EXT_SRC",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_ABS20_EXT_DST,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_ABS20_EXT_DST",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_ABS20_EXT_ODST,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_ABS20_EXT_ODST",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_ABS20_ADR_SRC,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_ABS20_ADR_SRC",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_ABS20_ADR_DST,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_ABS20_ADR_DST",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_PCR16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_PCR16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_PCR20_CALL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_PCR20_CALL",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430X_ABS16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_ABS16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430_ABS_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_ABS_HI16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MSP430_PREL31,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430_PREL31",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  EMPTY_HOWTO (R_MSP430_EHTYPE),

  /* A 10 bit PC relative relocation.  */
  HOWTO (R_MSP430X_10_PCREL,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 10,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_10_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0x3ff,			/* src_mask */
	 0x3ff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 10 bit PC relative relocation for complicated polymorphs.  */
  HOWTO (R_MSP430X_2X_PCREL,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 10,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MSP430X_2X_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0x3ff,			/* src_mask */
	 0x3ff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* Together with a following reloc, allows for the difference
     between two symbols to be the real addend of the second reloc.  */
  HOWTO (R_MSP430X_SYM_DIFF,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 rl78_sym_diff_handler,	/* special handler.  */
	 "R_MSP430X_SYM_DIFF",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* The length of unsigned-leb128 is variable, just assume the
     size is one byte here.  */
  HOWTO (R_MSP430X_GNU_SET_ULEB128,	/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 msp430_elf_ignore_reloc,	/* special handler.  */
	 "R_MSP430X_GNU_SET_ULEB128",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

  /* The length of unsigned-leb128 is variable, just assume the
     size is one byte here.  */
  HOWTO (R_MSP430X_GNU_SUB_ULEB128,	/* type */
	 0,				/* rightshift */
	 1,				/* size */
	 0,				/* bitsize */
	 false,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 msp430_elf_ignore_reloc,	/* special handler.  */
	 "R_MSP430X_GNU_SUB_ULEB128",	/* name */
	 false,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 false),			/* pcrel_offset */

};

/* Map BFD reloc types to MSP430 ELF reloc types.  */

struct msp430_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int elf_reloc_val;
};

static const struct msp430_reloc_map msp430_reloc_map[] =
{
  {BFD_RELOC_NONE,		   R_MSP430_NONE},
  {BFD_RELOC_32,		   R_MSP430_32},
  {BFD_RELOC_MSP430_10_PCREL,	   R_MSP430_10_PCREL},
  {BFD_RELOC_16,		   R_MSP430_16_BYTE},
  {BFD_RELOC_MSP430_16_PCREL,	   R_MSP430_16_PCREL},
  {BFD_RELOC_MSP430_16,		   R_MSP430_16},
  {BFD_RELOC_MSP430_16_PCREL_BYTE, R_MSP430_16_PCREL_BYTE},
  {BFD_RELOC_MSP430_16_BYTE,	   R_MSP430_16_BYTE},
  {BFD_RELOC_MSP430_2X_PCREL,	   R_MSP430_2X_PCREL},
  {BFD_RELOC_MSP430_RL_PCREL,	   R_MSP430_RL_PCREL},
  {BFD_RELOC_8,			   R_MSP430_8},
  {BFD_RELOC_MSP430_SYM_DIFF,	   R_MSP430_SYM_DIFF},
  {BFD_RELOC_MSP430_SET_ULEB128,   R_MSP430_GNU_SET_ULEB128 },
  {BFD_RELOC_MSP430_SUB_ULEB128,   R_MSP430_GNU_SUB_ULEB128 }
};

static const struct msp430_reloc_map msp430x_reloc_map[] =
{
  {BFD_RELOC_NONE,		      R_MSP430_NONE},
  {BFD_RELOC_32,		      R_MSP430_ABS32},
  {BFD_RELOC_16,		      R_MSP430_ABS16},
  {BFD_RELOC_8,			      R_MSP430_ABS8},
  {BFD_RELOC_MSP430_ABS8,	      R_MSP430_ABS8},
  {BFD_RELOC_MSP430X_PCR20_EXT_SRC,   R_MSP430X_PCR20_EXT_SRC},
  {BFD_RELOC_MSP430X_PCR20_EXT_DST,   R_MSP430X_PCR20_EXT_DST},
  {BFD_RELOC_MSP430X_PCR20_EXT_ODST,  R_MSP430X_PCR20_EXT_ODST},
  {BFD_RELOC_MSP430X_ABS20_EXT_SRC,   R_MSP430X_ABS20_EXT_SRC},
  {BFD_RELOC_MSP430X_ABS20_EXT_DST,   R_MSP430X_ABS20_EXT_DST},
  {BFD_RELOC_MSP430X_ABS20_EXT_ODST,  R_MSP430X_ABS20_EXT_ODST},
  {BFD_RELOC_MSP430X_ABS20_ADR_SRC,   R_MSP430X_ABS20_ADR_SRC},
  {BFD_RELOC_MSP430X_ABS20_ADR_DST,   R_MSP430X_ABS20_ADR_DST},
  {BFD_RELOC_MSP430X_PCR16,	      R_MSP430X_PCR16},
  {BFD_RELOC_MSP430X_PCR20_CALL,      R_MSP430X_PCR20_CALL},
  {BFD_RELOC_MSP430X_ABS16,	      R_MSP430X_ABS16},
  {BFD_RELOC_MSP430_ABS_HI16,	      R_MSP430_ABS_HI16},
  {BFD_RELOC_MSP430_PREL31,	      R_MSP430_PREL31},
  {BFD_RELOC_MSP430_10_PCREL,	      R_MSP430X_10_PCREL},
  {BFD_RELOC_MSP430_2X_PCREL,	      R_MSP430X_2X_PCREL},
  {BFD_RELOC_MSP430_RL_PCREL,	      R_MSP430X_PCR16},
  {BFD_RELOC_MSP430_SYM_DIFF,	      R_MSP430X_SYM_DIFF},
  {BFD_RELOC_MSP430_SET_ULEB128,      R_MSP430X_GNU_SET_ULEB128 },
  {BFD_RELOC_MSP430_SUB_ULEB128,      R_MSP430X_GNU_SUB_ULEB128 }
};

static inline bool
uses_msp430x_relocs (bfd * abfd)
{
  extern const bfd_target msp430_elf32_ti_vec;

  return bfd_get_mach (abfd) == bfd_mach_msp430x
    || abfd->xvec == & msp430_elf32_ti_vec;
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  unsigned int i;

  if (uses_msp430x_relocs (abfd))
    {
      for (i = ARRAY_SIZE (msp430x_reloc_map); i--;)
	if (msp430x_reloc_map[i].bfd_reloc_val == code)
	  return elf_msp430x_howto_table + msp430x_reloc_map[i].elf_reloc_val;
    }
  else
    {
      for (i = 0; i < ARRAY_SIZE (msp430_reloc_map); i++)
	if (msp430_reloc_map[i].bfd_reloc_val == code)
	  return &elf_msp430_howto_table[msp430_reloc_map[i].elf_reloc_val];
    }

  return NULL;
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  if (uses_msp430x_relocs (abfd))
    {
      for (i = ARRAY_SIZE (elf_msp430x_howto_table); i--;)
	if (elf_msp430x_howto_table[i].name != NULL
	    && strcasecmp (elf_msp430x_howto_table[i].name, r_name) == 0)
	  return elf_msp430x_howto_table + i;
    }
  else
    {
      for (i = 0;
	   i < (sizeof (elf_msp430_howto_table)
		/ sizeof (elf_msp430_howto_table[0]));
	   i++)
	if (elf_msp430_howto_table[i].name != NULL
	    && strcasecmp (elf_msp430_howto_table[i].name, r_name) == 0)
	  return &elf_msp430_howto_table[i];
    }

  return NULL;
}

/* Set the howto pointer for an MSP430 ELF reloc.  */

static bool
msp430_info_to_howto_rela (bfd * abfd,
			   arelent * cache_ptr,
			   Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);

  if (uses_msp430x_relocs (abfd))
    {
      if (r_type >= (unsigned int) R_MSP430x_max)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      abfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      cache_ptr->howto = elf_msp430x_howto_table + r_type;
    }
  else if (r_type >= (unsigned int) R_MSP430_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  else
    cache_ptr->howto = &elf_msp430_howto_table[r_type];

  return true;
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bool
elf32_msp430_check_relocs (bfd * abfd, struct bfd_link_info * info,
			   asection * sec, const Elf_Internal_Rela * relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
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
    }

  return true;
}

/* Perform a single relocation.  By default we use the standard BFD
   routines, but a few relocs, we have to do them ourselves.  */

static bfd_reloc_status_type
msp430_final_link_relocate (reloc_howto_type *	   howto,
			    bfd *		   input_bfd,
			    asection *		   input_section,
			    bfd_byte *		   contents,
			    Elf_Internal_Rela *	   rel,
			    bfd_vma		   relocation,
			    struct bfd_link_info * info)
{
  static asection *  sym_diff_section;
  static bfd_vma     sym_diff_value;

  struct bfd_elf_section_data * esd = elf_section_data (input_section);
  bfd_reloc_status_type r = bfd_reloc_ok;
  bfd_vma x;
  bfd_signed_vma srel;
  bool is_rel_reloc = false;

  if (uses_msp430x_relocs (input_bfd))
    {
      /* See if we have a REL type relocation.  */
      is_rel_reloc = (esd->rel.hdr != NULL);
      /* Sanity check - only one type of relocation per section.
	 FIXME: Theoretically it is possible to have both types,
	 but if that happens how can we distinguish between the two ?  */
      BFD_ASSERT (! is_rel_reloc || ! esd->rela.hdr);
      /* If we are using a REL relocation then the addend should be empty.  */
      BFD_ASSERT (! is_rel_reloc || rel->r_addend == 0);
    }

  if (debug_relocs)
    printf ("writing relocation (%p) at 0x%lx type: %d\n", rel,
	    (long) (input_section->output_section->vma + input_section->output_offset
		    + rel->r_offset), howto->type);
  if (sym_diff_section != NULL)
    {
      BFD_ASSERT (sym_diff_section == input_section);

     if (uses_msp430x_relocs (input_bfd))
       switch (howto->type)
	 {
	 case R_MSP430X_GNU_SET_ULEB128:
	   relocation += (!is_rel_reloc ? rel->r_addend : 0);
	   /* Fall through.  */
	 case R_MSP430_ABS32:
	  /* If we are computing a 32-bit value for the location lists
	     and the result is 0 then we add one to the value.  A zero
	     value can result because of linker relaxation deleteing
	     prologue instructions and using a value of 1 (for the begin
	     and end offsets in the location list entry) results in a
	     nul entry which does not prevent the following entries from
	     being parsed.  */
	   if (relocation == sym_diff_value
	       && strcmp (input_section->name, ".debug_loc") == 0)
	     ++ relocation;
	   /* Fall through.  */
	 case R_MSP430_ABS16:
	 case R_MSP430X_ABS16:
	 case R_MSP430_ABS8:
	   BFD_ASSERT (! is_rel_reloc);
	   relocation -= sym_diff_value;
	  break;

	 default:
	   return bfd_reloc_dangerous;
	 }
     else
       switch (howto->type)
	 {
	 case R_MSP430_GNU_SET_ULEB128:
	   relocation += (!is_rel_reloc ? rel->r_addend : 0);
	   /* Fall through.  */
	 case R_MSP430_32:
	 case R_MSP430_16:
	 case R_MSP430_16_BYTE:
	 case R_MSP430_8:
	   relocation -= sym_diff_value;
	  break;

	 default:
	   return bfd_reloc_dangerous;
	 }

      sym_diff_section = NULL;
    }

  if ((uses_msp430x_relocs (input_bfd)
       && howto->type == R_MSP430X_GNU_SET_ULEB128)
      || (!uses_msp430x_relocs (input_bfd)
	  && howto->type == R_MSP430_GNU_SET_ULEB128))
    {
      unsigned int len, new_len = 0;
      bfd_byte *endp, *p;
      unsigned int val = relocation;

      _bfd_read_unsigned_leb128 (input_bfd, contents + rel->r_offset, &len);

      /* Clean the contents value to zero.  Do not reduce the length.  */
      p = contents + rel->r_offset;
      endp = (p + len) - 1;
      memset (p, 0x80, len - 1);
      *(endp) = 0;

      /* Get the length of the new uleb128 value.  */
      do
	{
	  new_len++;
	  val >>= 7;
	} while (val);

      if (new_len > len)
	{
	  _bfd_error_handler
	    (_("error: final size of uleb128 value at offset 0x%lx in %pA "
	       "from %pB exceeds available space"),
	     (long) rel->r_offset, input_section, input_bfd);
	}
      else
	{
	  /* If the number of bytes required to store the new value has
	     decreased, "right align" the new value within the available space,
	     so the MSB side is padded with uleb128 zeros (0x80).  */
	  p = _bfd_write_unsigned_leb128 (p + (len - new_len), endp,
					  relocation);
	  /* We checked there is enough space for the new value above, so this
	     should never be NULL.  */
	  BFD_ASSERT (p);
	}

      return bfd_reloc_ok;
    }
  else if (uses_msp430x_relocs (input_bfd))
    switch (howto->type)
      {
      case R_MSP430X_SYM_DIFF:
      case R_MSP430X_GNU_SUB_ULEB128:
	/* Cache the input section and value.
	   The offset is unreliable, since relaxation may
	   have reduced the following reloc's offset.  */
	BFD_ASSERT (! is_rel_reloc);
	sym_diff_section = input_section;
	sym_diff_value = relocation + (howto->type == R_MSP430X_GNU_SUB_ULEB128
				       ? rel->r_addend : 0);
	return bfd_reloc_ok;

      case R_MSP430_ABS16:
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  srel += bfd_get_16 (input_bfd, contents);
	else
	  srel += rel->r_addend;
	bfd_put_16 (input_bfd, srel & 0xffff, contents);
	break;

      case R_MSP430X_10_PCREL:
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  srel += bfd_get_16 (input_bfd, contents) & 0x3ff;
	else
	  srel += rel->r_addend;
	srel -= rel->r_offset;
	srel -= 2;		/* Branch instructions add 2 to the PC...  */
	srel -= (input_section->output_section->vma +
		 input_section->output_offset);
	if (srel & 1)
	  return bfd_reloc_outofrange;

	/* MSP430 addresses commands as words.  */
	srel >>= 1;

	/* Check for an overflow.  */
	if (srel < -512 || srel > 511)
	  {
	    if (info->disable_target_specific_optimizations < 0)
	      {
		static bool warned = false;
		if (! warned)
		  {
		    info->callbacks->warning
		      (info,
		       _("try enabling relaxation to avoid relocation truncations"),
		       NULL, input_bfd, input_section, relocation);
		    warned = true;
		  }
	      }
	    return bfd_reloc_overflow;
	  }

	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xfc00) | (srel & 0x3ff);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430X_PCR20_EXT_ODST:
	/* [0,4]+[48,16] = ---F ---- ---- FFFF */
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;
	    addend = (bfd_get_16 (input_bfd, contents) & 0xf) << 16;
	    addend |= bfd_get_16 (input_bfd, contents + 6);
	    srel += addend;

	  }
	else
	  srel += rel->r_addend;
	srel -= rel->r_offset;
	srel -= (input_section->output_section->vma +
		 input_section->output_offset);
	bfd_put_16 (input_bfd, (srel & 0xffff), contents + 6);
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xfff0) | ((srel >> 16) & 0xf);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430X_ABS20_EXT_SRC:
	/* [7,4]+[32,16] = -78- ---- FFFF */
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;
	    addend = (bfd_get_16 (input_bfd, contents) & 0x0780) << 9;
	    addend |= bfd_get_16 (input_bfd, contents + 4);
	    srel += addend;
	  }
	else
	  srel += rel->r_addend;
	bfd_put_16 (input_bfd, (srel & 0xffff), contents + 4);
	srel >>= 16;
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xf87f) | ((srel << 7) & 0x0780);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430_16_PCREL:
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  srel += bfd_get_16 (input_bfd, contents);
	else
	  srel += rel->r_addend;
	srel -= rel->r_offset;
	/* Only branch instructions add 2 to the PC...  */
	srel -= (input_section->output_section->vma +
		 input_section->output_offset);
	if (srel & 1)
	  return bfd_reloc_outofrange;
	bfd_put_16 (input_bfd, srel & 0xffff, contents);
	break;

      case R_MSP430X_PCR20_EXT_DST:
	/* [0,4]+[32,16] = ---F ---- FFFF */
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;
	    addend = (bfd_get_16 (input_bfd, contents) & 0xf) << 16;
	    addend |= bfd_get_16 (input_bfd, contents + 4);
	    srel += addend;
	  }
	else
	  srel += rel->r_addend;
	srel -= rel->r_offset;
	srel -= (input_section->output_section->vma +
		 input_section->output_offset);
	bfd_put_16 (input_bfd, (srel & 0xffff), contents + 4);
	srel >>= 16;
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xfff0) | (srel & 0xf);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430X_PCR20_EXT_SRC:
	/* [7,4]+[32,16] = -78- ---- FFFF */
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;
	    addend = ((bfd_get_16 (input_bfd, contents) & 0x0780) << 9);
	    addend |= bfd_get_16 (input_bfd, contents + 4);
	    srel += addend;;
	  }
	else
	  srel += rel->r_addend;
	srel -= rel->r_offset;
	/* Only branch instructions add 2 to the PC...  */
	srel -= (input_section->output_section->vma +
		 input_section->output_offset);
	bfd_put_16 (input_bfd, (srel & 0xffff), contents + 4);
	srel >>= 16;
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xf87f) | ((srel << 7) & 0x0780);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430_ABS8:
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  srel += bfd_get_8 (input_bfd, contents);
	else
	  srel += rel->r_addend;
	bfd_put_8 (input_bfd, srel & 0xff, contents);
	break;

      case R_MSP430X_ABS20_EXT_DST:
	/* [0,4]+[32,16] = ---F ---- FFFF */
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;
	    addend = (bfd_get_16 (input_bfd, contents) & 0xf) << 16;
	    addend |= bfd_get_16 (input_bfd, contents + 4);
	    srel += addend;
	  }
	else
	  srel += rel->r_addend;
	bfd_put_16 (input_bfd, (srel & 0xffff), contents + 4);
	srel >>= 16;
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xfff0) | (srel & 0xf);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430X_ABS20_EXT_ODST:
	/* [0,4]+[48,16] = ---F ---- ---- FFFF */
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;
	    addend = (bfd_get_16 (input_bfd, contents) & 0xf) << 16;
	    addend |= bfd_get_16 (input_bfd, contents + 6);
	    srel += addend;
	  }
	else
	  srel += rel->r_addend;
	bfd_put_16 (input_bfd, (srel & 0xffff), contents + 6);
	srel >>= 16;
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xfff0) | (srel & 0xf);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430X_ABS20_ADR_SRC:
	/* [8,4]+[16,16] = -F-- FFFF */
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;

	    addend = ((bfd_get_16 (input_bfd, contents) & 0xf00) << 8);
	    addend |= bfd_get_16 (input_bfd, contents + 2);
	    srel += addend;
	  }
	else
	  srel += rel->r_addend;
	bfd_put_16 (input_bfd, (srel & 0xffff), contents + 2);
	srel >>= 16;
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xf0ff) | ((srel << 8) & 0x0f00);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430X_ABS20_ADR_DST:
	/* [0,4]+[16,16] = ---F FFFF */
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;
	    addend = ((bfd_get_16 (input_bfd, contents) & 0xf) << 16);
	    addend |= bfd_get_16 (input_bfd, contents + 2);
	    srel += addend;
	  }
	else
	  srel += rel->r_addend;
	bfd_put_16 (input_bfd, (srel & 0xffff), contents + 2);
	srel >>= 16;
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xfff0) | (srel & 0xf);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430X_ABS16:
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  srel += bfd_get_16 (input_bfd, contents);
	else
	  srel += rel->r_addend;
	x = srel;
	if (x > 0xffff)
	  return bfd_reloc_overflow;
	bfd_put_16 (input_bfd, srel & 0xffff, contents);
	break;

      case R_MSP430_ABS_HI16:
	/* The EABI specifies that this must be a RELA reloc.  */
	BFD_ASSERT (! is_rel_reloc);
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	srel += rel->r_addend;
	bfd_put_16 (input_bfd, (srel >> 16) & 0xffff, contents);
	break;

      case R_MSP430X_PCR20_CALL:
	/* [0,4]+[16,16] = ---F FFFF*/
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  {
	    bfd_vma addend;
	    addend = (bfd_get_16 (input_bfd, contents) & 0xf) << 16;
	    addend |= bfd_get_16 (input_bfd, contents + 2);
	    srel += addend;
	  }
	else
	  srel += rel->r_addend;
	srel -= rel->r_offset;
	srel -= (input_section->output_section->vma +
		 input_section->output_offset);
	bfd_put_16 (input_bfd, srel & 0xffff, contents + 2);
	srel >>= 16;
	x = bfd_get_16 (input_bfd, contents);
	x = (x & 0xfff0) | (srel & 0xf);
	bfd_put_16 (input_bfd, x, contents);
	break;

      case R_MSP430X_PCR16:
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  srel += bfd_get_16 (input_bfd, contents);
	else
	  srel += rel->r_addend;
	srel -= rel->r_offset;
	srel -= (input_section->output_section->vma +
		 input_section->output_offset);
	bfd_put_16 (input_bfd, srel & 0xffff, contents);
	break;

      case R_MSP430_PREL31:
	contents += rel->r_offset;
	srel = (bfd_signed_vma) relocation;
	if (is_rel_reloc)
	  srel += (bfd_get_32 (input_bfd, contents) & 0x7fffffff);
	else
	  srel += rel->r_addend;
	srel += rel->r_addend;
	x = bfd_get_32 (input_bfd, contents);
	x = (x & 0x80000000) | ((srel >> 31) & 0x7fffffff);
	bfd_put_32 (input_bfd, x, contents);
	break;

      default:
	r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				      contents, rel->r_offset,
				      relocation, rel->r_addend);
      }
  else
    switch (howto->type)
      {
    case R_MSP430_10_PCREL:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      srel -= rel->r_offset;
      srel -= 2;		/* Branch instructions add 2 to the PC...  */
      srel -= (input_section->output_section->vma +
	       input_section->output_offset);

      if (srel & 1)
	return bfd_reloc_outofrange;

      /* MSP430 addresses commands as words.  */
      srel >>= 1;

      /* Check for an overflow.  */
      if (srel < -512 || srel > 511)
	{
	  if (info->disable_target_specific_optimizations < 0)
	    {
	      static bool warned = false;
	      if (! warned)
		{
		  info->callbacks->warning
		    (info,
		     _("try enabling relaxation to avoid relocation truncations"),
		     NULL, input_bfd, input_section, relocation);
		  warned = true;
		}
	    }
	  return bfd_reloc_overflow;
	}

      x = bfd_get_16 (input_bfd, contents);
      x = (x & 0xfc00) | (srel & 0x3ff);
      bfd_put_16 (input_bfd, x, contents);
      break;

    case R_MSP430_2X_PCREL:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      srel -= rel->r_offset;
      srel -= 2;		/* Branch instructions add 2 to the PC...  */
      srel -= (input_section->output_section->vma +
	       input_section->output_offset);

      if (srel & 1)
	return bfd_reloc_outofrange;

      /* MSP430 addresses commands as words.  */
      srel >>= 1;

      /* Check for an overflow.  */
      if (srel < -512 || srel > 511)
	return bfd_reloc_overflow;

      x = bfd_get_16 (input_bfd, contents);
      x = (x & 0xfc00) | (srel & 0x3ff);
      bfd_put_16 (input_bfd, x, contents);
      /* Handle second jump instruction.  */
      x = bfd_get_16 (input_bfd, contents - 2);
      srel += 1;
      x = (x & 0xfc00) | (srel & 0x3ff);
      bfd_put_16 (input_bfd, x, contents - 2);
      break;

    case R_MSP430_RL_PCREL:
    case R_MSP430_16_PCREL:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      srel -= rel->r_offset;
      /* Only branch instructions add 2 to the PC...  */
      srel -= (input_section->output_section->vma +
	       input_section->output_offset);

      if (srel & 1)
	return bfd_reloc_outofrange;

      bfd_put_16 (input_bfd, srel & 0xffff, contents);
      break;

    case R_MSP430_16_PCREL_BYTE:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      srel -= rel->r_offset;
      /* Only branch instructions add 2 to the PC...  */
      srel -= (input_section->output_section->vma +
	       input_section->output_offset);

      bfd_put_16 (input_bfd, srel & 0xffff, contents);
      break;

    case R_MSP430_16_BYTE:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      bfd_put_16 (input_bfd, srel & 0xffff, contents);
      break;

    case R_MSP430_16:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;

      if (srel & 1)
	return bfd_reloc_notsupported;

      bfd_put_16 (input_bfd, srel & 0xffff, contents);
      break;

    case R_MSP430_8:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;

      bfd_put_8 (input_bfd, srel & 0xff, contents);
      break;

    case R_MSP430_SYM_DIFF:
    case R_MSP430_GNU_SUB_ULEB128:
      /* Cache the input section and value.
	 The offset is unreliable, since relaxation may
	 have reduced the following reloc's offset.  */
      sym_diff_section = input_section;
      sym_diff_value = relocation + (howto->type == R_MSP430_GNU_SUB_ULEB128
				     ? rel->r_addend : 0);
      return bfd_reloc_ok;

      default:
	r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				      contents, rel->r_offset,
				      relocation, rel->r_addend);
      }

  return r;
}

/* Relocate an MSP430 ELF section.  */

static int
elf32_msp430_relocate_section (bfd * output_bfd ATTRIBUTE_UNUSED,
			       struct bfd_link_info * info,
			       bfd * input_bfd,
			       asection * input_section,
			       bfd_byte * contents,
			       Elf_Internal_Rela * relocs,
			       Elf_Internal_Sym * local_syms,
			       asection ** local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

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

      if (uses_msp430x_relocs (input_bfd))
	howto = elf_msp430x_howto_table + r_type;
      else
	howto = elf_msp430_howto_table + r_type;

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
	  name = name == NULL || *name == 0 ? bfd_section_name (sec) : name;
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

      r = msp430_final_link_relocate (howto, input_bfd, input_section,
				      contents, rel, relocation, info);

      if (r != bfd_reloc_ok)
	{
	  const char *msg = (const char *) NULL;

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
	      msg = _("internal error: branch/jump to an odd address detected");
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

/* The final processing done just before writing out a MSP430 ELF object
   file.  This gets the MSP430 architecture right based on the machine
   number.  */

static bool
bfd_elf_msp430_final_write_processing (bfd *abfd)
{
  unsigned long val;

  switch (bfd_get_mach (abfd))
    {
    default:
    case bfd_mach_msp110: val = E_MSP430_MACH_MSP430x11x1; break;
    case bfd_mach_msp11: val = E_MSP430_MACH_MSP430x11; break;
    case bfd_mach_msp12: val = E_MSP430_MACH_MSP430x12; break;
    case bfd_mach_msp13: val = E_MSP430_MACH_MSP430x13; break;
    case bfd_mach_msp14: val = E_MSP430_MACH_MSP430x14; break;
    case bfd_mach_msp15: val = E_MSP430_MACH_MSP430x15; break;
    case bfd_mach_msp16: val = E_MSP430_MACH_MSP430x16; break;
    case bfd_mach_msp31: val = E_MSP430_MACH_MSP430x31; break;
    case bfd_mach_msp32: val = E_MSP430_MACH_MSP430x32; break;
    case bfd_mach_msp33: val = E_MSP430_MACH_MSP430x33; break;
    case bfd_mach_msp41: val = E_MSP430_MACH_MSP430x41; break;
    case bfd_mach_msp42: val = E_MSP430_MACH_MSP430x42; break;
    case bfd_mach_msp43: val = E_MSP430_MACH_MSP430x43; break;
    case bfd_mach_msp44: val = E_MSP430_MACH_MSP430x44; break;
    case bfd_mach_msp20: val = E_MSP430_MACH_MSP430x20; break;
    case bfd_mach_msp22: val = E_MSP430_MACH_MSP430x22; break;
    case bfd_mach_msp23: val = E_MSP430_MACH_MSP430x23; break;
    case bfd_mach_msp24: val = E_MSP430_MACH_MSP430x24; break;
    case bfd_mach_msp26: val = E_MSP430_MACH_MSP430x26; break;
    case bfd_mach_msp46: val = E_MSP430_MACH_MSP430x46; break;
    case bfd_mach_msp47: val = E_MSP430_MACH_MSP430x47; break;
    case bfd_mach_msp54: val = E_MSP430_MACH_MSP430x54; break;
    case bfd_mach_msp430x: val = E_MSP430_MACH_MSP430X; break;
    }

  elf_elfheader (abfd)->e_machine = EM_MSP430;
  elf_elfheader (abfd)->e_flags &= ~EF_MSP430_MACH;
  elf_elfheader (abfd)->e_flags |= val;
  return _bfd_elf_final_write_processing (abfd);
}

/* Set the right machine number.  */

static bool
elf32_msp430_object_p (bfd * abfd)
{
  int e_set = bfd_mach_msp14;

  if (elf_elfheader (abfd)->e_machine == EM_MSP430
      || elf_elfheader (abfd)->e_machine == EM_MSP430_OLD)
    {
      int e_mach = elf_elfheader (abfd)->e_flags & EF_MSP430_MACH;

      switch (e_mach)
	{
	default:
	case E_MSP430_MACH_MSP430x11: e_set = bfd_mach_msp11; break;
	case E_MSP430_MACH_MSP430x11x1: e_set = bfd_mach_msp110; break;
	case E_MSP430_MACH_MSP430x12: e_set = bfd_mach_msp12; break;
	case E_MSP430_MACH_MSP430x13: e_set = bfd_mach_msp13; break;
	case E_MSP430_MACH_MSP430x14: e_set = bfd_mach_msp14; break;
	case E_MSP430_MACH_MSP430x15: e_set = bfd_mach_msp15; break;
	case E_MSP430_MACH_MSP430x16: e_set = bfd_mach_msp16; break;
	case E_MSP430_MACH_MSP430x31: e_set = bfd_mach_msp31; break;
	case E_MSP430_MACH_MSP430x32: e_set = bfd_mach_msp32; break;
	case E_MSP430_MACH_MSP430x33: e_set = bfd_mach_msp33; break;
	case E_MSP430_MACH_MSP430x41: e_set = bfd_mach_msp41; break;
	case E_MSP430_MACH_MSP430x42: e_set = bfd_mach_msp42; break;
	case E_MSP430_MACH_MSP430x43: e_set = bfd_mach_msp43; break;
	case E_MSP430_MACH_MSP430x44: e_set = bfd_mach_msp44; break;
	case E_MSP430_MACH_MSP430x20: e_set = bfd_mach_msp20; break;
	case E_MSP430_MACH_MSP430x22: e_set = bfd_mach_msp22; break;
	case E_MSP430_MACH_MSP430x23: e_set = bfd_mach_msp23; break;
	case E_MSP430_MACH_MSP430x24: e_set = bfd_mach_msp24; break;
	case E_MSP430_MACH_MSP430x26: e_set = bfd_mach_msp26; break;
	case E_MSP430_MACH_MSP430x46: e_set = bfd_mach_msp46; break;
	case E_MSP430_MACH_MSP430x47: e_set = bfd_mach_msp47; break;
	case E_MSP430_MACH_MSP430x54: e_set = bfd_mach_msp54; break;
	case E_MSP430_MACH_MSP430X: e_set = bfd_mach_msp430x; break;
	}
    }

  return bfd_default_set_arch_mach (abfd, bfd_arch_msp430, e_set);
}

/* These functions handle relaxing for the msp430.
   Relaxation required only in two cases:
    - Bad hand coding like jumps from one section to another or
      from file to file.
    - Sibling calls. This will affect only 'jump label' polymorph. Without
      relaxing this enlarges code by 2 bytes. Sibcalls implemented but
      do not work in gcc's port by the reason I do not know.
    - To convert out of range conditional jump instructions (found inside
      a function) into inverted jumps over an unconditional branch instruction.
   Anyway, if a relaxation required, user should pass -relax option to the
   linker.

   There are quite a few relaxing opportunities available on the msp430:

   ================================================================

   1. 3 words -> 1 word

   eq	   ==	   jeq label			jne +4; br lab
   ne	   !=	   jne label			jeq +4; br lab
   lt	   <	   jl  label			jge +4; br lab
   ltu	   <	   jlo label			lhs +4; br lab
   ge	   >=	   jge label			jl  +4; br lab
   geu	   >=	   jhs label			jlo +4; br lab

   2. 4 words -> 1 word

   ltn	   <	   jn			   jn  +2; jmp +4; br lab

   3. 4 words -> 2 words

   gt	   >	   jeq +2; jge label	   jeq +6; jl  +4; br label
   gtu	   >	   jeq +2; jhs label	   jeq +6; jlo +4; br label

   4. 4 words -> 2 words and 2 labels

   leu	   <=	   jeq label; jlo label	   jeq +2; jhs +4; br label
   le	   <=	   jeq label; jl  label	   jeq +2; jge +4; br label
   =================================================================

   codemap for first cases is (labels masked ):
	      eq:	0x2002,0x4010,0x0000 -> 0x2400
	      ne:	0x2402,0x4010,0x0000 -> 0x2000
	      lt:	0x3402,0x4010,0x0000 -> 0x3800
	      ltu:	0x2c02,0x4010,0x0000 -> 0x2800
	      ge:	0x3802,0x4010,0x0000 -> 0x3400
	      geu:	0x2802,0x4010,0x0000 -> 0x2c00

  second case:
	      ltn:	0x3001,0x3c02,0x4010,0x0000 -> 0x3000

  third case:
	      gt:	0x2403,0x3802,0x4010,0x0000 -> 0x2401,0x3400
	      gtu:	0x2403,0x2802,0x4010,0x0000 -> 0x2401,0x2c00

  fourth case:
	      leu:	0x2401,0x2c02,0x4010,0x0000 -> 0x2400,0x2800
	      le:	0x2401,0x3402,0x4010,0x0000 -> 0x2400,0x3800

  Unspecified case :)
	      jump:	0x4010,0x0000 -> 0x3c00.  */

#define NUMB_RELAX_CODES	12
static struct rcodes_s
{
  int f0, f1;			/* From code.  */
  int t0, t1;			/* To code.  */
  int labels;			/* Position of labels: 1 - one label at first
				   word, 2 - one at second word, 3 - two
				   labels at both.  */
  int cdx;			/* Words to match.  */
  int bs;			/* Shrink bytes.  */
  int off;			/* Offset from old label for new code.  */
  int ncl;			/* New code length.  */
} rcode[] =
{/*				  lab,cdx,bs,off,ncl */
  { 0x0000, 0x0000, 0x3c00, 0x0000, 1, 0, 2, 2,	 2},	/* jump */
  { 0x0000, 0x2002, 0x2400, 0x0000, 1, 1, 4, 4,	 2},	/* eq */
  { 0x0000, 0x2402, 0x2000, 0x0000, 1, 1, 4, 4,	 2},	/* ne */
  { 0x0000, 0x3402, 0x3800, 0x0000, 1, 1, 4, 4,	 2},	/* lt */
  { 0x0000, 0x2c02, 0x2800, 0x0000, 1, 1, 4, 4,	 2},	/* ltu */
  { 0x0000, 0x3802, 0x3400, 0x0000, 1, 1, 4, 4,	 2},	/* ge */
  { 0x0000, 0x2802, 0x2c00, 0x0000, 1, 1, 4, 4,	 2},	/* geu */
  { 0x3001, 0x3c02, 0x3000, 0x0000, 1, 2, 6, 6,	 2},	/* ltn */
  { 0x2403, 0x3802, 0x2401, 0x3400, 2, 2, 4, 6,	 4},	/* gt */
  { 0x2403, 0x2802, 0x2401, 0x2c00, 2, 2, 4, 6,	 4},	/* gtu */
  { 0x2401, 0x2c02, 0x2400, 0x2800, 3, 2, 4, 6,	 4},	/* leu , 2 labels */
  { 0x2401, 0x2c02, 0x2400, 0x2800, 3, 2, 4, 6,	 4},	/* le  , 2 labels */
  { 0,	    0,	    0,	    0,	    0, 0, 0, 0,	 0}
};

/* Return TRUE if a symbol exists at the given address.  */

static bool
msp430_elf_symbol_address_p (bfd * abfd,
			     asection * sec,
			     Elf_Internal_Sym * isym,
			     bfd_vma addr)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int sec_shndx;
  Elf_Internal_Sym *isymend;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  unsigned int symcount;

  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);

  /* Examine all the local symbols.  */
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  for (isymend = isym + symtab_hdr->sh_info; isym < isymend; isym++)
    if (isym->st_shndx == sec_shndx && isym->st_value == addr)
      return true;

  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)
	      - symtab_hdr->sh_info);
  sym_hashes = elf_sym_hashes (abfd);
  end_hashes = sym_hashes + symcount;
  for (; sym_hashes < end_hashes; sym_hashes++)
    {
      struct elf_link_hash_entry *sym_hash = *sym_hashes;

      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec
	  && sym_hash->root.u.def.value == addr)
	return true;
    }

  return false;
}

/* Adjust all local symbols defined as '.section + 0xXXXX' (.section has
   sec_shndx) referenced from current and other sections.  */

static bool
msp430_elf_relax_adjust_locals (bfd * abfd, asection * sec, bfd_vma addr,
				int count, unsigned int sec_shndx,
				bfd_vma toaddr)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *irel;
  Elf_Internal_Rela *irelend;
  Elf_Internal_Sym *isym;

  irel = elf_section_data (sec)->relocs;
  if (irel == NULL)
    return true;

  irelend = irel + sec->reloc_count;
  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;

  for (;irel < irelend; irel++)
    {
      unsigned int sidx = ELF32_R_SYM(irel->r_info);
      Elf_Internal_Sym *lsym = isym + sidx;

      /* Adjust symbols referenced by .sec+0xXX.  */
      if (irel->r_addend > addr && irel->r_addend < toaddr
	  && sidx < symtab_hdr->sh_info
	  && lsym->st_shndx == sec_shndx)
	irel->r_addend -= count;
    }

  return true;
}

/* Delete some bytes from a section while relaxing.  */

static bool
msp430_elf_relax_delete_bytes (bfd * abfd, asection * sec, bfd_vma addr,
			       int count)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int sec_shndx;
  bfd_byte *contents;
  Elf_Internal_Rela *irel;
  Elf_Internal_Rela *irelend;
  bfd_vma toaddr;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  unsigned int symcount;
  asection *p;

  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);

  contents = elf_section_data (sec)->this_hdr.contents;

  toaddr = sec->size;
  if (debug_relocs)
    printf ("      deleting %d bytes between 0x%lx to 0x%lx\n",
	    count, (long) addr, (long) toaddr);

  irel = elf_section_data (sec)->relocs;
  irelend = irel + sec->reloc_count;

  /* Actually delete the bytes.  */
  memmove (contents + addr, contents + addr + count,
	   (size_t) (toaddr - addr - count));
  sec->size -= count;

  /* Adjust all the relocs.  */
  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;
  for (; irel < irelend; irel++)
    {
      /* Get the new reloc address.  */
      if ((irel->r_offset > addr && irel->r_offset < toaddr))
	irel->r_offset -= count;
    }

  for (p = abfd->sections; p != NULL; p = p->next)
    msp430_elf_relax_adjust_locals (abfd,p,addr,count,sec_shndx,toaddr);

  /* Adjust the local symbols defined in this section.  */
  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;
  for (isymend = isym + symtab_hdr->sh_info; isym < isymend; isym++)
    {
      const char * name;

      name = bfd_elf_string_from_elf_section
	(abfd, symtab_hdr->sh_link, isym->st_name);
      name = name == NULL || *name == 0 ? bfd_section_name (sec) : name;

      if (isym->st_shndx != sec_shndx)
	continue;

      if (isym->st_value > addr
	  && (isym->st_value < toaddr
	      /* We also adjust a symbol at the end of the section if its name is
		 on the list below.  These symbols are used for debug info
		 generation and they refer to the end of the current section, not
		 the start of the next section.  */
	      || (isym->st_value == toaddr
		  && name != NULL
		  && (startswith (name, ".Letext")
		      || startswith (name, ".LFE")))))
	{
	  if (debug_relocs)
	    printf ("      adjusting value of local symbol %s from 0x%lx ",
		    name, (long) isym->st_value);
	  if (isym->st_value < addr + count)
	    isym->st_value = addr;
	  else
	    isym->st_value -= count;
	  if (debug_relocs)
	    printf ("to 0x%lx\n", (long) isym->st_value);
	}
      /* Adjust the function symbol's size as well.  */
      else if (ELF_ST_TYPE (isym->st_info) == STT_FUNC
	       && isym->st_value + isym->st_size > addr
	       && isym->st_value + isym->st_size < toaddr)
	isym->st_size -= count;
    }

  /* Now adjust the global symbols defined in this section.  */
  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)
	      - symtab_hdr->sh_info);
  sym_hashes = elf_sym_hashes (abfd);
  end_hashes = sym_hashes + symcount;
  for (; sym_hashes < end_hashes; sym_hashes++)
    {
      struct elf_link_hash_entry *sym_hash = *sym_hashes;

      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec
	  && sym_hash->root.u.def.value > addr
	  && sym_hash->root.u.def.value < toaddr)
	{
	  if (sym_hash->root.u.def.value < addr + count)
	    sym_hash->root.u.def.value = addr;
	  else
	    sym_hash->root.u.def.value -= count;
	}
      /* Adjust the function symbol's size as well.  */
      else if (sym_hash->root.type == bfd_link_hash_defined
	       && sym_hash->root.u.def.section == sec
	       && sym_hash->type == STT_FUNC
	       && sym_hash->root.u.def.value + sym_hash->size > addr
	       && sym_hash->root.u.def.value + sym_hash->size < toaddr)
	sym_hash->size -= count;
    }

  return true;
}

/* Insert one or two words into a section whilst relaxing.  */

static bfd_byte *
msp430_elf_relax_add_words (bfd * abfd, asection * sec, bfd_vma addr,
			    int num_words, int word1, int word2)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int sec_shndx;
  bfd_byte *contents;
  Elf_Internal_Rela *irel;
  Elf_Internal_Rela *irelend;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  unsigned int symcount;
  bfd_vma sec_end;
  asection *p;
  if (debug_relocs)
    printf ("      adding %d words at 0x%lx\n", num_words,
	    (long) (sec->output_section->vma + sec->output_offset + addr));

  contents = elf_section_data (sec)->this_hdr.contents;
  sec_end = sec->size;
  int num_bytes = num_words * 2;

  /* Make space for the new words.  */
  contents = bfd_realloc (contents, sec_end + num_bytes);
  memmove (contents + addr + num_bytes, contents + addr, sec_end - addr);

  /* Insert the new words.  */
  bfd_put_16 (abfd, word1, contents + addr);
  if (num_words == 2)
    bfd_put_16 (abfd, word2, contents + addr + 2);

  /* Update the section information.  */
  sec->size += num_bytes;
  elf_section_data (sec)->this_hdr.contents = contents;

  /* Adjust all the relocs.  */
  irel = elf_section_data (sec)->relocs;
  irelend = irel + sec->reloc_count;

  for (; irel < irelend; irel++)
    if ((irel->r_offset >= addr && irel->r_offset < sec_end))
      irel->r_offset += num_bytes;

  /* Adjust the local symbols defined in this section.  */
  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);
  for (p = abfd->sections; p != NULL; p = p->next)
    msp430_elf_relax_adjust_locals (abfd, p, addr, -num_bytes,
				    sec_shndx, sec_end);

  /* Adjust the global symbols affected by the move.  */
  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;
  for (isymend = isym + symtab_hdr->sh_info; isym < isymend; isym++)
    if (isym->st_shndx == sec_shndx
	&& isym->st_value >= addr && isym->st_value < sec_end)
      {
	if (debug_relocs)
	  printf ("      adjusting value of local symbol %s from 0x%lx to "
		  "0x%lx\n", bfd_elf_string_from_elf_section
		  (abfd, symtab_hdr->sh_link, isym->st_name),
		  (long) isym->st_value, (long)(isym->st_value + num_bytes));
	isym->st_value += num_bytes;
      }

  /* Now adjust the global symbols defined in this section.  */
  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)
	      - symtab_hdr->sh_info);
  sym_hashes = elf_sym_hashes (abfd);
  end_hashes = sym_hashes + symcount;
  for (; sym_hashes < end_hashes; sym_hashes++)
    {
      struct elf_link_hash_entry *sym_hash = *sym_hashes;

      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec
	  && sym_hash->root.u.def.value >= addr
	  && sym_hash->root.u.def.value < sec_end)
	sym_hash->root.u.def.value += num_bytes;
    }

  return contents;
}

static bool
msp430_elf_relax_section (bfd * abfd, asection * sec,
			  struct bfd_link_info * link_info,
			  bool * again)
{
  Elf_Internal_Shdr * symtab_hdr;
  Elf_Internal_Rela * internal_relocs;
  Elf_Internal_Rela * irel;
  Elf_Internal_Rela * irelend;
  bfd_byte *	      contents = NULL;
  Elf_Internal_Sym *  isymbuf = NULL;

  /* Assume nothing changes.  */
  *again = false;

  /* We don't have to do anything for a relocatable link, if
     this section does not have relocs, or if this is not a
     code section.  */
  if (bfd_link_relocatable (link_info)
      || sec->reloc_count == 0
      || (sec->flags & SEC_RELOC) == 0
      || (sec->flags & SEC_HAS_CONTENTS) == 0
      || (sec->flags & SEC_CODE) == 0)
    return true;

  if (debug_relocs)
    printf ("Relaxing %s (%p), output_offset: 0x%lx sec size: 0x%lx\n",
	    sec->name, sec, (long) sec->output_offset, (long) sec->size);

  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;

  /* Get a copy of the native relocations.  */
  internal_relocs =
    _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL, link_info->keep_memory);
  if (internal_relocs == NULL)
    goto error_return;

  /* Walk through them looking for relaxing opportunities.  */
  irelend = internal_relocs + sec->reloc_count;

  if (debug_relocs)
    printf ("  trying code size growing relocs\n");
  /* Do code size growing relocs first.  */
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      bfd_vma symval;

      /* If this isn't something that can be relaxed, then ignore
	 this reloc.  */
      if (uses_msp430x_relocs (abfd)
	  && ELF32_R_TYPE (irel->r_info) == (int) R_MSP430X_10_PCREL)
	;
      else if (! uses_msp430x_relocs (abfd)
	       && ELF32_R_TYPE (irel->r_info) == (int) R_MSP430_10_PCREL)
	;
      else
	continue;

      /* Get the section contents if we haven't done so already.  */
      if (contents == NULL)
	{
	  /* Get cached copy if it exists.  */
	  if (elf_section_data (sec)->this_hdr.contents != NULL)
	    contents = elf_section_data (sec)->this_hdr.contents;
	  else if (! bfd_malloc_and_get_section (abfd, sec, &contents))
	    goto error_return;
	}

      /* Read this BFD's local symbols if we haven't done so already.  */
      if (isymbuf == NULL && symtab_hdr->sh_info != 0)
	{
	  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (isymbuf == NULL)
	    isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
					    symtab_hdr->sh_info, 0,
					    NULL, NULL, NULL);
	  if (isymbuf == NULL)
	    goto error_return;
	}

      /* Get the value of the symbol referred to by the reloc.  */
      if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  Elf_Internal_Sym *isym;
	  asection *sym_sec;

	  isym = isymbuf + ELF32_R_SYM (irel->r_info);
	  if (isym->st_shndx == SHN_UNDEF)
	    sym_sec = bfd_und_section_ptr;
	  else if (isym->st_shndx == SHN_ABS)
	    sym_sec = bfd_abs_section_ptr;
	  else if (isym->st_shndx == SHN_COMMON)
	    sym_sec = bfd_com_section_ptr;
	  else
	    sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
	  symval = (isym->st_value
		    + sym_sec->output_section->vma + sym_sec->output_offset);

	  if (debug_relocs)
	    printf ("    processing reloc at 0x%lx for local sym: %s "
		    "st_value: 0x%lx adj value: 0x%lx\n",
		    (long) (sec->output_offset + sec->output_section->vma
			    + irel->r_offset),
		    bfd_elf_string_from_elf_section (abfd, symtab_hdr->sh_link,
						     isym->st_name),
		    (long) isym->st_value, (long) symval);
	}
      else
	{
	  unsigned long indx;
	  struct elf_link_hash_entry *h;

	  /* An external symbol.  */
	  indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  BFD_ASSERT (h != NULL);

	  if (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	    /* This appears to be a reference to an undefined
	       symbol.  Just ignore it--it will be caught by the
	       regular reloc processing.  */
	    continue;

	  symval = (h->root.u.def.value
		    + h->root.u.def.section->output_section->vma
		    + h->root.u.def.section->output_offset);
	  if (debug_relocs)
	    printf ("    processing reloc at 0x%lx for global sym: %s "
		    "st_value: 0x%lx adj value: 0x%lx\n",
		    (long) (sec->output_offset + sec->output_section->vma
		    + irel->r_offset),
	      h->root.root.string, (long) h->root.u.def.value,
	      (long) symval);
	}

      /* For simplicity of coding, we are going to modify the section
	 contents, the section relocs, and the BFD symbol table.  We
	 must tell the rest of the code not to free up this
	 information.  It would be possible to instead create a table
	 of changes which have to be made, as is done in coff-mips.c;
	 that would be more work, but would require less memory when
	 the linker is run.  */

      bfd_signed_vma value = symval;
      int opcode;

      /* Compute the value that will be relocated.  */
      value += irel->r_addend;
      /* Convert to PC relative.  */
      value -= (sec->output_section->vma + sec->output_offset);
      value -= irel->r_offset;
      value -= 2;

      /* Scale.  */
      value >>= 1;

      /* If it is in range then no modifications are needed.  */
      if (value >= -512 && value <= 511)
	continue;

      /* Get the opcode.  */
      opcode = bfd_get_16 (abfd, contents + irel->r_offset);

      /* Compute the new opcode.  We are going to convert:
	 JMP label
	   into:
	 BR[A] label
	   or
	 J<cond> label
	   into:
	 J<inv-cond> 1f
	 BR[A] #label
	 1:			*/
      switch (opcode & 0xfc00)
	{
	case 0x3800: opcode = 0x3402; break; /* Jl  -> Jge +2 */
	case 0x3400: opcode = 0x3802; break; /* Jge -> Jl  +2 */
	case 0x2c00: opcode = 0x2802; break; /* Jhs -> Jlo +2 */
	case 0x2800: opcode = 0x2c02; break; /* Jlo -> Jhs +2 */
	case 0x2400: opcode = 0x2002; break; /* Jeq -> Jne +2 */
	case 0x2000: opcode = 0x2402; break; /* jne -> Jeq +2 */
	case 0x3000: /* jn    */
	  /* There is no direct inverse of the Jn insn.
	     FIXME: we could do this as:
		Jn 1f
		br 2f
	     1: br label
	     2:		       */
	  continue;
	case 0x3c00:
	  if (uses_msp430x_relocs (abfd))
	    opcode = 0x0080;	/* JMP -> BRA  */
	  else
	    opcode = 0x4030;	/* JMP -> BR  */
	  break;
	default:
	  /* Unhandled branch instruction.  */
	  /* fprintf (stderr, "unrecog: %x\n", opcode); */
	  continue;
	}

      /* Note that we've changed the relocs, section contents, etc.  */
      elf_section_data (sec)->relocs = internal_relocs;
      elf_section_data (sec)->this_hdr.contents = contents;
      symtab_hdr->contents = (unsigned char *) isymbuf;

      /* Install the new opcode.  */
      bfd_put_16 (abfd, opcode, contents + irel->r_offset);

      /* Insert the new branch instruction.  */
      if (uses_msp430x_relocs (abfd))
	{
	  if (debug_relocs)
	    printf ("      R_MSP430X_10_PCREL -> R_MSP430X_ABS20_ADR_SRC "
		    "(growing with new opcode 0x%x)\n", opcode);

	  /* Insert an absolute branch (aka MOVA) instruction.
	     Note that bits 19:16 of the address are stored in the first word
	     of the insn, so this is where r_offset will point to.  */
	  if (opcode == 0x0080)
	    {
	      /* If we're inserting a BRA because we are converting from a JMP,
		 then only add one word for destination address; the BRA opcode
		 has already been written.  */
	      contents = msp430_elf_relax_add_words
		(abfd, sec, irel->r_offset + 2, 1, 0x0000, 0);
	    }
	  else
	    {
	      contents = msp430_elf_relax_add_words
		(abfd, sec, irel->r_offset + 2, 2, 0x0080, 0x0000);
	      /* Update the relocation to point to the inserted branch
		 instruction.  Note - we are changing a PC-relative reloc
		 into an absolute reloc, but this is OK because we have
		 arranged with the assembler to have the reloc's value be
		 a (local) symbol, not a section+offset value.  */
	      irel->r_offset += 2;
	    }

	  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
				       R_MSP430X_ABS20_ADR_SRC);
	}
      else
	{
	  if (debug_relocs)
	    printf ("      R_MSP430_10_PCREL -> R_MSP430_16 "
		    "(growing with new opcode 0x%x)\n", opcode);
	  if (opcode == 0x4030)
	    {
	      /* If we're inserting a BR because we are converting from a JMP,
		 then only add one word for destination address; the BR opcode
		 has already been written.  */
	      contents = msp430_elf_relax_add_words
		(abfd, sec, irel->r_offset + 2, 1, 0x0000, 0);
	      irel->r_offset += 2;
	    }
	  else
	    {
	      contents = msp430_elf_relax_add_words
		(abfd, sec, irel->r_offset + 2, 2, 0x4030, 0x0000);
	      /* See comment above about converting a 10-bit PC-rel
		 relocation into a 16-bit absolute relocation.  */
	      irel->r_offset += 4;
	    }
	  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
				       R_MSP430_16);
	}

      /* Growing the section may mean that other
	 conditional branches need to be fixed.  */
      *again = true;
    }

    if (debug_relocs)
      printf ("  trying code size shrinking relocs\n");

    for (irel = internal_relocs; irel < irelend; irel++)
      {
	bfd_vma symval;

	/* Get the section contents if we haven't done so already.  */
	if (contents == NULL)
	  {
	    /* Get cached copy if it exists.  */
	    if (elf_section_data (sec)->this_hdr.contents != NULL)
	      contents = elf_section_data (sec)->this_hdr.contents;
	    else if (! bfd_malloc_and_get_section (abfd, sec, &contents))
	      goto error_return;
	  }

	/* Read this BFD's local symbols if we haven't done so already.  */
	if (isymbuf == NULL && symtab_hdr->sh_info != 0)
	  {
	    isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	    if (isymbuf == NULL)
	      isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
					      symtab_hdr->sh_info, 0,
					      NULL, NULL, NULL);
	    if (isymbuf == NULL)
	      goto error_return;
	  }

	/* Get the value of the symbol referred to by the reloc.  */
	if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	  {
	    /* A local symbol.  */
	    Elf_Internal_Sym *isym;
	    asection *sym_sec;

	    isym = isymbuf + ELF32_R_SYM (irel->r_info);
	    if (isym->st_shndx == SHN_UNDEF)
	      sym_sec = bfd_und_section_ptr;
	    else if (isym->st_shndx == SHN_ABS)
	      sym_sec = bfd_abs_section_ptr;
	    else if (isym->st_shndx == SHN_COMMON)
	      sym_sec = bfd_com_section_ptr;
	    else
	      sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
	    symval = (isym->st_value
		      + sym_sec->output_section->vma + sym_sec->output_offset);

	    if (debug_relocs)
	      printf ("    processing reloc at 0x%lx for local sym: %s "
		      "st_value: 0x%lx adj value: 0x%lx\n",
		      (long) (sec->output_offset + sec->output_section->vma
			      + irel->r_offset),
		      bfd_elf_string_from_elf_section
		      (abfd, symtab_hdr->sh_link, isym->st_name),
		      (long) isym->st_value, (long) symval);
	  }
	else
	  {
	    unsigned long indx;
	    struct elf_link_hash_entry *h;

	    /* An external symbol.  */
	    indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
	    h = elf_sym_hashes (abfd)[indx];
	    BFD_ASSERT (h != NULL);

	    if (h->root.type != bfd_link_hash_defined
		&& h->root.type != bfd_link_hash_defweak)
	      /* This appears to be a reference to an undefined
		 symbol.  Just ignore it--it will be caught by the
		 regular reloc processing.  */
	      continue;

	    symval = (h->root.u.def.value
		      + h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset);
	    if (debug_relocs)
	      printf ("    processing reloc at 0x%lx for global sym: %s "
		      "st_value: 0x%lx adj value: 0x%lx\n", (long)
		      (sec->output_offset + sec->output_section->vma
		       + irel->r_offset),
		      h->root.root.string, (long) h->root.u.def.value,
		      (long) symval);
	  }

	/* For simplicity of coding, we are going to modify the section
	   contents, the section relocs, and the BFD symbol table.  We
	   must tell the rest of the code not to free up this
	   information.  It would be possible to instead create a table
	   of changes which have to be made, as is done in coff-mips.c;
	   that would be more work, but would require less memory when
	   the linker is run.  */

	/* Try to turn a 16bit pc-relative branch into a 10bit pc-relative
	   branch.  */
	/* Paranoia? paranoia...  */
	if (! uses_msp430x_relocs (abfd)
	    && ELF32_R_TYPE (irel->r_info) == (int) R_MSP430_RL_PCREL)
	  {
	    bfd_vma value = symval;

	    /* Deal with pc-relative gunk.  */
	    value -= (sec->output_section->vma + sec->output_offset);
	    value -= irel->r_offset;
	    value += irel->r_addend;

	    /* See if the value will fit in 10 bits, note the high value is
	       1016 as the target will be two bytes closer if we are
	       able to relax.  */
	    if ((long) value < 1016 && (long) value > -1016)
	      {
		int code0 = 0, code1 = 0, code2 = 0;
		int i;
		struct rcodes_s *rx;

		/* Get the opcode.  */
		if (irel->r_offset >= 6)
		  code0 = bfd_get_16 (abfd, contents + irel->r_offset - 6);

		if (irel->r_offset >= 4)
		  code1 = bfd_get_16 (abfd, contents + irel->r_offset - 4);

		code2 = bfd_get_16 (abfd, contents + irel->r_offset - 2);

		if (code2 != 0x4010)
		  continue;

		/* Check r4 and r3.  */
		for (i = NUMB_RELAX_CODES - 1; i >= 0; i--)
		  {
		    rx = &rcode[i];
		    if (rx->cdx == 2 && rx->f0 == code0 && rx->f1 == code1)
		      break;
		    else if (rx->cdx == 1 && rx->f1 == code1)
		      break;
		    else if (rx->cdx == 0)	/* This is an unconditional jump.  */
		      break;
		  }

		/* Check labels:
		   .Label0:       ; we do not care about this label
		   jeq    +6
		   .Label1:       ; make sure there is no label here
		   jl     +4
		   .Label2:       ; make sure there is no label here
		   br .Label_dst

		   So, if there is .Label1 or .Label2 we cannot relax this code.
		   This actually should not happen, cause for relaxable
		   instructions we use RL_PCREL reloc instead of 16_PCREL.
		   Will change this in the future. */

		if (rx->cdx > 0
		    && msp430_elf_symbol_address_p (abfd, sec, isymbuf,
						    irel->r_offset - 2))
		  continue;
		if (rx->cdx > 1
		    && msp430_elf_symbol_address_p (abfd, sec, isymbuf,
						    irel->r_offset - 4))
		  continue;

		/* Note that we've changed the relocs, section contents, etc.  */
		elf_section_data (sec)->relocs = internal_relocs;
		elf_section_data (sec)->this_hdr.contents = contents;
		symtab_hdr->contents = (unsigned char *) isymbuf;

		if (debug_relocs)
		  printf ("      R_MSP430_RL_PCREL -> ");
		/* Fix the relocation's type.  */
		if (uses_msp430x_relocs (abfd))
		  {
		    if (rx->labels == 3)	/* Handle special cases.  */
		      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
						   R_MSP430X_2X_PCREL);
		    else
		      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
						   R_MSP430X_10_PCREL);
		  }
		else
		  {
		    if (rx->labels == 3)	/* Handle special cases.  */
		      {
			irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
						     R_MSP430_2X_PCREL);
			if (debug_relocs)
			  printf ("R_MSP430_2X_PCREL (shrinking with new opcode"
				  " 0x%x)\n", rx->t0);
		      }
		    else
		      {
			irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
						     R_MSP430_10_PCREL);
			if (debug_relocs)
			  printf ("R_MSP430_10_PCREL (shrinking with new opcode"
				  " 0x%x)\n", rx->t0);
		      }
		  }

		/* Fix the opcode right way.  */
		bfd_put_16 (abfd, rx->t0, contents + irel->r_offset - rx->off);
		if (rx->t1)
		  bfd_put_16 (abfd, rx->t1,
			      contents + irel->r_offset - rx->off + 2);

		/* Delete bytes. */
		if (!msp430_elf_relax_delete_bytes (abfd, sec,
						    irel->r_offset - rx->off +
						    rx->ncl, rx->bs))
		  goto error_return;

		/* Handle unconditional jumps.  */
		if (rx->cdx == 0)
		  irel->r_offset -= 2;

		/* That will change things, so, we should relax again.
		   Note that this is not required, and it may be slow.  */
		*again = true;
	      }
	  }

	/* Try to turn a 16-bit absolute branch into a 10-bit pc-relative
	   branch.  */
	if ((uses_msp430x_relocs (abfd)
	     && ELF32_R_TYPE (irel->r_info) == R_MSP430X_ABS16)
	    || (! uses_msp430x_relocs (abfd)
		&& ELF32_R_TYPE (irel->r_info) == R_MSP430_16))
	  {
	    bfd_vma value = symval;

	    value -= (sec->output_section->vma + sec->output_offset);
	    value -= irel->r_offset;
	    value += irel->r_addend;

	    /* See if the value will fit in 10 bits, note the high value is
	       1016 as the target will be two bytes closer if we are
	       able to relax.  */
	    if ((long) value < 1016 && (long) value > -1016)
	      {
		int code1, code2, opcode;

		/* Get the opcode.  */
		code2 = bfd_get_16 (abfd, contents + irel->r_offset - 2);
		if (code2 != 0x4030) /* BR -> JMP */
		  continue;
		/* FIXME: check r4 and r3 ? */
		/* FIXME: Handle 0x4010 as well ?  */

		/* Note that we've changed the relocs, section contents, etc.  */
		elf_section_data (sec)->relocs = internal_relocs;
		elf_section_data (sec)->this_hdr.contents = contents;
		symtab_hdr->contents = (unsigned char *) isymbuf;

		/* Fix the relocation's type.  */
		if (uses_msp430x_relocs (abfd))
		  {
		    irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
						 R_MSP430X_10_PCREL);
		    if (debug_relocs)
		      printf ("      R_MSP430X_16 -> R_MSP430X_10_PCREL ");
		  }
		else
		  {
		    irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
						 R_MSP430_10_PCREL);
		    if (debug_relocs)
		      printf ("      R_MSP430_16 -> R_MSP430_10_PCREL ");
		  }
		/* If we're trying to shrink a BR[A] after previously having
		   grown a JMP for this reloc, then we have a sequence like
		   this:
		     J<cond> 1f
		     BR[A]
		     1:
		   The opcode for J<cond> has the target hard-coded as 2 words
		   ahead of the insn, instead of using a reloc.
		   This means we cannot rely on any of the helper functions to
		   update this hard-coded jump destination if we remove the
		   BR[A] insn, so we must explicitly update it here.
		   This does mean that we can remove the entire branch
		   instruction, and invert the conditional jump, saving us 4
		   bytes rather than only 2 if we detected this in the normal
		   way.  */
		code1 = bfd_get_16 (abfd, contents + irel->r_offset - 4);
		switch (code1)
		  {
		    case 0x3802: opcode = 0x3401; break; /* Jl  +2 -> Jge +1 */
		    case 0x3402: opcode = 0x3801; break; /* Jge +2 -> Jl  +1 */
		    case 0x2c02: opcode = 0x2801; break; /* Jhs +2 -> Jlo +1 */
		    case 0x2802: opcode = 0x2c01; break; /* Jlo +2 -> Jhs +1 */
		    case 0x2402: opcode = 0x2001; break; /* Jeq +2 -> Jne +1 */
		    case 0x2002: opcode = 0x2401; break; /* jne +2 -> Jeq +1 */
		    case 0x3002: /* jn +2   */
		      /* FIXME: There is no direct inverse of the Jn insn.  */
		      continue;
		    default:
		      /* The previous opcode does not have a hard-coded jump
			 that we added when previously relaxing, so relax the
			 current branch as normal.  */
		      opcode = 0x3c00;
		      break;
		    }
		if (debug_relocs)
		  printf ("(shrinking with new opcode 0x%x)\n", opcode);

		if (opcode != 0x3c00)
		  {
		    /* Invert the opcode of the conditional jump.  */
		    bfd_put_16 (abfd, opcode, contents + irel->r_offset - 4);
		    irel->r_offset -= 4;

		    /* Delete 4 bytes - the full BR insn.  */
		    if (!msp430_elf_relax_delete_bytes (abfd, sec,
							irel->r_offset + 2, 4))
		      goto error_return;
		  }
		else
		  {
		    /* Fix the opcode right way.  */
		    bfd_put_16 (abfd, opcode, contents + irel->r_offset - 2);
		    irel->r_offset -= 2;

		    /* Delete bytes.  */
		    if (!msp430_elf_relax_delete_bytes (abfd, sec,
							irel->r_offset + 2, 2))
		      goto error_return;
		  }

		/* That will change things, so, we should relax again.
		   Note that this is not required, and it may be slow.  */
		*again = true;
	      }
	  }
      }

  if (isymbuf != NULL && symtab_hdr->contents != (unsigned char *) isymbuf)
    {
      if (!link_info->keep_memory)
	free (isymbuf);
      else
	{
	  /* Cache the symbols for elf_link_input_bfd.  */
	  symtab_hdr->contents = (unsigned char *) isymbuf;
	}
    }

  if (contents != NULL
      && elf_section_data (sec)->this_hdr.contents != contents)
    {
      if (!link_info->keep_memory)
	free (contents);
      else
	{
	  /* Cache the section contents for elf_link_input_bfd.  */
	  elf_section_data (sec)->this_hdr.contents = contents;
	}
    }

  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return true;

 error_return:
  if (symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);
  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return false;
}

/* Handle an MSP430 specific section when reading an object file.
   This is called when bfd_section_from_shdr finds a section with
   an unknown type.  */

static bool
elf32_msp430_section_from_shdr (bfd *abfd,
				Elf_Internal_Shdr * hdr,
				const char *name,
				int shindex)
{
  switch (hdr->sh_type)
    {
    case SHT_MSP430_SEC_FLAGS:
    case SHT_MSP430_SYM_ALIASES:
    case SHT_MSP430_ATTRIBUTES:
      return _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
    default:
      return false;
    }
}

static bool
elf32_msp430_obj_attrs_handle_unknown (bfd *abfd, int tag)
{
  _bfd_error_handler
    /* xgettext:c-format */
    (_("warning: %pB: unknown MSPABI object attribute %d"),
     abfd, tag);
  return true;
}

/* Determine whether an object attribute tag takes an integer, a
   string or both.  */

static int
elf32_msp430_obj_attrs_arg_type (int tag)
{
  if (tag == Tag_compatibility)
    return ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL;

  if (tag < 32)
    return ATTR_TYPE_FLAG_INT_VAL;

  return (tag & 1) != 0 ? ATTR_TYPE_FLAG_STR_VAL : ATTR_TYPE_FLAG_INT_VAL;
}

static inline const char *
isa_type (int isa)
{
  switch (isa)
    {
    case 1: return "MSP430";
    case 2: return "MSP430X";
    default: return "unknown";
    }
}

static inline const char *
code_model (int model)
{
  switch (model)
    {
    case 1: return "small";
    case 2: return "large";
    default: return "unknown";
    }
}

static inline const char *
data_model (int model)
{
  switch (model)
    {
    case 1: return "small";
    case 2: return "large";
    case 3: return "restricted large";
    default: return "unknown";
    }
}

/* Merge MSPABI and GNU object attributes from IBFD into OBFD.
   Raise an error if there are conflicting attributes.  */

static bool
elf32_msp430_merge_msp430_attributes (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  obj_attribute *in_msp_attr, *in_gnu_attr;
  obj_attribute *out_msp_attr, *out_gnu_attr;
  bool result = true;
  static bfd * first_input_bfd = NULL;

  /* Skip linker created files.  */
  if (ibfd->flags & BFD_LINKER_CREATED)
    return true;

  /* LTO can create temporary files for linking which may not have an attribute
     section.  */
  if (ibfd->lto_output
      && bfd_get_section_by_name (ibfd, ".MSP430.attributes") == NULL)
    return true;

  /* If this is the first real object just copy the attributes.  */
  if (!elf_known_obj_attributes_proc (obfd)[0].i)
    {
      _bfd_elf_copy_obj_attributes (ibfd, obfd);

      out_msp_attr = elf_known_obj_attributes_proc (obfd);

      /* Use the Tag_null value to indicate that
	 the attributes have been initialized.  */
      out_msp_attr[0].i = 1;

      first_input_bfd = ibfd;
      return true;
    }

  in_msp_attr = elf_known_obj_attributes_proc (ibfd);
  out_msp_attr = elf_known_obj_attributes_proc (obfd);
  in_gnu_attr = elf_known_obj_attributes (ibfd) [OBJ_ATTR_GNU];
  out_gnu_attr = elf_known_obj_attributes (obfd) [OBJ_ATTR_GNU];

  /* The ISAs must be the same.  */
  if (in_msp_attr[OFBA_MSPABI_Tag_ISA].i != out_msp_attr[OFBA_MSPABI_Tag_ISA].i)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB uses %s instructions but %pB uses %s"),
	 ibfd, isa_type (in_msp_attr[OFBA_MSPABI_Tag_ISA].i),
	 first_input_bfd, isa_type (out_msp_attr[OFBA_MSPABI_Tag_ISA].i));
      result = false;
    }

  /* The code models must be the same.  */
  if (in_msp_attr[OFBA_MSPABI_Tag_Code_Model].i
      != out_msp_attr[OFBA_MSPABI_Tag_Code_Model].i)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB uses the %s code model whereas %pB uses the %s code model"),
	 ibfd, code_model (in_msp_attr[OFBA_MSPABI_Tag_Code_Model].i),
	 first_input_bfd,
	 code_model (out_msp_attr[OFBA_MSPABI_Tag_Code_Model].i));
      result = false;
    }

  /* The large code model is only supported by the MSP430X.  */
  if (in_msp_attr[OFBA_MSPABI_Tag_Code_Model].i == 2
      && out_msp_attr[OFBA_MSPABI_Tag_ISA].i != 2)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB uses the large code model but %pB uses MSP430 instructions"),
	 ibfd, first_input_bfd);
      result = false;
    }

  /* The data models must be the same.  */
  if (in_msp_attr[OFBA_MSPABI_Tag_Data_Model].i
      != out_msp_attr[OFBA_MSPABI_Tag_Data_Model].i)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB uses the %s data model whereas %pB uses the %s data model"),
	 ibfd, data_model (in_msp_attr[OFBA_MSPABI_Tag_Data_Model].i),
	 first_input_bfd,
	 data_model (out_msp_attr[OFBA_MSPABI_Tag_Data_Model].i));
      result = false;
    }

  /* The small code model requires the use of the small data model.  */
  if (in_msp_attr[OFBA_MSPABI_Tag_Code_Model].i == 1
      && out_msp_attr[OFBA_MSPABI_Tag_Data_Model].i != 1)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB uses the small code model but %pB uses the %s data model"),
	 ibfd, first_input_bfd,
	 data_model (out_msp_attr[OFBA_MSPABI_Tag_Data_Model].i));
      result = false;
    }

  /* The large data models are only supported by the MSP430X.  */
  if (in_msp_attr[OFBA_MSPABI_Tag_Data_Model].i > 1
      && out_msp_attr[OFBA_MSPABI_Tag_ISA].i != 2)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB uses the %s data model but %pB only uses MSP430 instructions"),
	 ibfd, data_model (in_msp_attr[OFBA_MSPABI_Tag_Data_Model].i),
	 first_input_bfd);
      result = false;
    }

  /* Just ignore the data region unless the large memory model is in use.
     We have already checked that ibfd and obfd use the same memory model.  */
  if ((in_msp_attr[OFBA_MSPABI_Tag_Code_Model].i
       == OFBA_MSPABI_Val_Code_Model_LARGE)
      && (in_msp_attr[OFBA_MSPABI_Tag_Data_Model].i
	  == OFBA_MSPABI_Val_Data_Model_LARGE))
    {
      /* We cannot allow "lower region only" to be linked with any other
	 values (i.e. ANY or NONE).
	 Before this attribute existed, "ANY" region was the default.  */
      bool ibfd_lower_region_used
	= (in_gnu_attr[Tag_GNU_MSP430_Data_Region].i
	   == Val_GNU_MSP430_Data_Region_Lower);
      bool obfd_lower_region_used
	= (out_gnu_attr[Tag_GNU_MSP430_Data_Region].i
	   == Val_GNU_MSP430_Data_Region_Lower);
      if (ibfd_lower_region_used != obfd_lower_region_used)
	{
	  _bfd_error_handler
	    (_("error: %pB can use the upper region for data, "
	       "but %pB assumes data is exclusively in lower memory"),
	     ibfd_lower_region_used ? obfd : ibfd,
	     ibfd_lower_region_used ? ibfd : obfd);
	  result = false;
	}
    }

  return result;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
elf32_msp430_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  /* Make sure that the machine number reflects the most
     advanced version of the MSP architecture required.  */
#define max(a,b) ((a) > (b) ? (a) : (b))
  if (bfd_get_mach (ibfd) != bfd_get_mach (obfd))
    bfd_default_set_arch_mach (obfd, bfd_get_arch (obfd),
			       max (bfd_get_mach (ibfd), bfd_get_mach (obfd)));
#undef max

  return elf32_msp430_merge_msp430_attributes (ibfd, info);
}

static bool
msp430_elf_is_target_special_symbol (bfd *abfd, asymbol *sym)
{
  return _bfd_elf_is_local_label_name (abfd, sym->name);
}

static bool
uses_large_model (bfd *abfd)
{
  obj_attribute * attr;

  if (abfd->flags & BFD_LINKER_CREATED)
    return false;

  attr = elf_known_obj_attributes_proc (abfd);
  if (attr == NULL)
    return false;

  return attr[OFBA_MSPABI_Tag_Code_Model].i == 2;
}

static unsigned int
elf32_msp430_eh_frame_address_size (bfd *abfd,
				    const asection *sec ATTRIBUTE_UNUSED)
{
  return uses_large_model (abfd) ? 4 : 2;
}

/* This is gross.  The MSP430 EABI says that (sec 11.5):

     "An implementation may choose to use Rel or Rela
      type relocations for other relocations."

   But it also says that:

     "Certain relocations are identified as Rela only. [snip]
      Where Rela is specified, an implementation must honor
      this requirement."

  There is one relocation marked as requiring RELA - R_MSP430_ABS_HI16 - but
  to keep things simple we choose to use RELA relocations throughout.  The
  problem is that the TI compiler generates REL relocations, so we have to
  be able to accept those as well.  */

#define elf_backend_may_use_rel_p  1
#define elf_backend_may_use_rela_p 1
#define elf_backend_default_use_rela_p 1

#undef	elf_backend_obj_attrs_vendor
#define elf_backend_obj_attrs_vendor		"mspabi"
#undef	elf_backend_obj_attrs_section
#define elf_backend_obj_attrs_section		".MSP430.attributes"
#undef	elf_backend_obj_attrs_section_type
#define elf_backend_obj_attrs_section_type	SHT_MSP430_ATTRIBUTES
#define elf_backend_section_from_shdr		elf32_msp430_section_from_shdr
#define elf_backend_obj_attrs_handle_unknown	elf32_msp430_obj_attrs_handle_unknown
#undef	elf_backend_obj_attrs_arg_type
#define elf_backend_obj_attrs_arg_type		elf32_msp430_obj_attrs_arg_type
#define bfd_elf32_bfd_merge_private_bfd_data	elf32_msp430_merge_private_bfd_data
#define elf_backend_eh_frame_address_size	elf32_msp430_eh_frame_address_size

#define ELF_ARCH		bfd_arch_msp430
#define ELF_MACHINE_CODE	EM_MSP430
#define ELF_MACHINE_ALT1	EM_MSP430_OLD
#define ELF_MAXPAGESIZE		4
#define	ELF_OSABI		ELFOSABI_STANDALONE

#define TARGET_LITTLE_SYM	msp430_elf32_vec
#define TARGET_LITTLE_NAME	"elf32-msp430"

#define elf_info_to_howto		     msp430_info_to_howto_rela
#define elf_info_to_howto_rel		     NULL
#define elf_backend_relocate_section	     elf32_msp430_relocate_section
#define elf_backend_check_relocs	     elf32_msp430_check_relocs
#define elf_backend_can_gc_sections	     1
#define elf_backend_final_write_processing   bfd_elf_msp430_final_write_processing
#define elf_backend_object_p		     elf32_msp430_object_p
#define bfd_elf32_bfd_relax_section	     msp430_elf_relax_section
#define bfd_elf32_bfd_is_target_special_symbol	msp430_elf_is_target_special_symbol

#undef	elf32_bed
#define elf32_bed		elf32_msp430_bed

#include "elf32-target.h"

/* The TI compiler sets the OSABI field to ELFOSABI_NONE.  */
#undef	TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM	msp430_elf32_ti_vec

#undef	elf32_bed
#define elf32_bed		elf32_msp430_ti_bed

#undef	ELF_OSABI
#define	ELF_OSABI		ELFOSABI_NONE

static const struct bfd_elf_special_section msp430_ti_elf_special_sections[] =
{
  /* prefix, prefix_length,	   suffix_len, type,		   attributes.  */
  { STRING_COMMA_LEN (".TI.symbol.alias"),  0, SHT_MSP430_SYM_ALIASES, 0 },
  { STRING_COMMA_LEN (".TI.section.flags"), 0, SHT_MSP430_SEC_FLAGS,   0 },
  { STRING_COMMA_LEN ("_TI_build_attrib"),  0, SHT_MSP430_ATTRIBUTES,  0 },
  { NULL, 0,				    0, 0,		       0 }
};

#undef	elf_backend_special_sections
#define elf_backend_special_sections		msp430_ti_elf_special_sections

#include "elf32-target.h"
